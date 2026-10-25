#include "DatabaseTests.h"
#include "CourseDatabase.h"
#include "sqlite3.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

// Testing suite for the database, checks CRUD, foreign keys, transactions, parameterized queries, and blocked deletes
// Uses its own test database file so the real courses.db is never touched

namespace
{
	const char* TEST_DB = "test_courses.db";

	// Runs one check, print PASS/FAIL with a message
	bool check(bool condition, const std::string& description)
	{
		std::cout << (condition ? " [PASS] " : " [FAIL] ") << description << std::endl;
		return condition;
	}

	Course makeCourse(const std::string& number, const std::vector<std::string>& prereqs = {})
	{
		Course c;
		c.courseNum = number;
		c.courseTitle = "Course " + number;
		c.prerequisites = prereqs;
		return c;
	}

	// Runs SQL on its own connection (foreign keys on), skipping all of our program's checks
	// Used to see if the database itself rejects bad data, returns SQLite's result code
	int rawSql(const std::string& sql)
	{
		sqlite3* raw = nullptr;
		if (sqlite3_open(TEST_DB, &raw) != SQLITE_OK) {
			sqlite3_close(raw);
			return -1;
		}
		sqlite3_exec(raw, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
		int result = sqlite3_exec(raw, sql.c_str(), nullptr, nullptr, nullptr);
		sqlite3_close(raw);
		return result;
	}

	bool sameList(const std::vector<std::string>& a, const std::vector<std::string>& b)
	{
		return a == b;
	}
}

bool runDatabaseTests()
{
	std::cout << "Running database tests\n";
	bool allPassed = true;

	// Start from a fresh test database
	std::remove(TEST_DB);
	CourseDatabase db;
	std::string error;
	if (!db.open(TEST_DB, error)) {
		check(false, "setup: test database could not be opened: " + error);
		return false;
	}

	// Test 1: Create and read
	// CS100 has no prereq, CS101 needs CS100, CS201 needs CS101
	{
		bool added = db.addCourse(makeCourse("CS100"), error) &&
			db.addCourse(makeCourse("CS101", { "CS100" }), error) &&
			db.addCourse(makeCourse("CS201", { "CS101" }), error);

		Course read;
		allPassed &= check(added && db.getCourse("CS101", read) && read.courseTitle == "Course CS101" && sameList(read.prerequisites, { "CS100" }),
			"CRUD: a course is saved and read back with its title and prerequisite");
		allPassed &= check(!db.addCourse(makeCourse("CS100"), error) && db.getAllCourses().size() == 3,
			"CRUD: adding a course number that already exists is rejected");
	}

	// Test 2: Update title
	{
		Course read;
		bool changed = db.updateCourseTitle("CS100", "Intro to Computing", error);
		allPassed &= check(changed && db.getCourse("CS100", read) && read.courseTitle == "Intro to Computing", "CRUD: update changes a course title");
		allPassed &= check(!db.updateCourseTitle("ZZ999", "Ghost", error) && error.find("not found") != std::string::npos,
			"CRUD: updating a course that does not exist is reported, not accepted");
	}

	// Test 3: Add and remove prereqs on a course that already exists
	{
		Course read;
		bool added = db.addPrerequisite("CS201", "CS100", error);
		allPassed &= check(added && db.getCourse("CS201", read) && sameList(read.prerequisites, { "CS100", "CS101" }), "update: a prerequisite is added to an existing course");

		allPassed &= check(!db.addPrerequisite("ZZ999", "CS100", error) && !db.addPrerequisite("CS201", "NOPE1", error) &&
			!db.addPrerequisite("CS201", "CS100", error) && !db.addPrerequisite("CS201", "CS201", error),
			"update: a missing course, missing prerequisite, repeat, or a course requiring itself is all rejected");

		bool removed = db.removePrerequisite("CS201", "CS100", error);
		allPassed &= check(removed && db.getCourse("CS201", read) && sameList(read.prerequisites, { "CS101" }), "update: a prerequisite is removed");
		allPassed &= check(!db.removePrerequisite("CS201", "CS100", error), "update: removing a prerequisite that is not listed is reported");
	}

	// Test 4: Foreign keys, the database itself rejects bad rows, not just our code
	allPassed &= check(rawSql("INSERT INTO Prerequisite VALUES ('CS101', 'NOPE999');") == SQLITE_CONSTRAINT &&
		rawSql("INSERT INTO Prerequisite VALUES ('NOPE999', 'CS100');") == SQLITE_CONSTRAINT,
		"foreign key: a prerequisite row pointing at a course that does not exist is rejected by the database");
	allPassed &= check(rawSql("INSERT INTO Prerequisite VALUES ('CS100', 'CS100');") == SQLITE_CONSTRAINT,
		"constraint: a course requiring itself is rejected by the database");
	allPassed &= check(rawSql("INSERT INTO Prerequisite VALUES ('CS101', 'CS100');") == SQLITE_CONSTRAINT,
		"constraint: the same prerequisite pair twice is rejected by the database");

	// Test 5: Blocked deletes
	// CS100 is needed by CS101 so it cannot be deleted
	{
		Course read;
		allPassed &= check(!db.deleteCourse("CS100", error) && db.getCourse("CS100", read), "delete: a course that other courses require is blocked and stays in the database");

		// CS201 is not needed by anything, deleting it also removes its own prereq row
		bool deleted = db.deleteCourse("CS201", error);
		allPassed &= check(deleted && !db.courseExists("CS201") && db.getCourseRequiring("CS101").empty(),
			"delete: a course nothing requires is deleted along with its own prerequisite rows");
	}

	// Test 6: Transactions
	// CS300 has one real prereq and one fake one, nothing should be saved
	{
		bool added = db.addCourse(makeCourse("CS300", { "CS100", "NOPE1" }), error);
		bool leftBehind = db.courseExists("CS300");
		bool leftInPrereqs = false;
		for (const Course& c : db.getCourseRequiring("CS100")) {
			if (c.courseNum == "CS300") {
				leftInPrereqs = true;
			}
		}
		allPassed &= check(!added && !leftBehind && !leftInPrereqs, "transaction: an add that fails partway is rolled back, nothing is left behind");
	}

	// Data is still there after closing and opening again
	{
		db.close();
		Course read;
		bool reopened = db.open(TEST_DB, error);
		allPassed &= check(reopened && db.getCourse("CS101", read) && sameList(read.prerequisites, { "CS100" }), "transaction: saved data is still there after closing and reopening the database");
	}

	// Test 7: Parameterized queries, SQL typed in as data should be stored as text
	{
		std::string attack = "x'); DROP TABLE Course;--";
		Course read;
		bool added = db.addCourse([&]() { Course c = makeCourse("CS400"); c.courseTitle = attack; return c; }(), error);
		allPassed &= check(added && db.getCourse("CS400", read) && read.courseTitle == attack && db.getAllCourses().size() >= 3,
			"parameterized: SQL typed in as a title is stored as plain text and the table is still there");

		allPassed &= check(!db.getCourse("' OR '1'='1", read) && !db.courseExists("' OR '1'='1"),
			"parameterized: SQL typed in as a course number matches nothing");
	}

	db.close();
	std::remove(TEST_DB);

	std::cout << (allPassed ? "All database tests passed\n\n" : "One or more database tests failed\n\n");
	return allPassed;
}