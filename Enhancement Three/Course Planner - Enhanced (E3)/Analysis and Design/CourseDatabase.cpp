#include "CourseDatabase.h"
#include "sqlite3.h"

// Used for many references: https://www.sqlite.org/cintro.html

namespace {

	// Wrapper around statement so it finalizes. Values attached with bind(), sending them separately to SQLite

	class Statement {
	public:
		Statement(sqlite3* db, const char* sql) : stmt(nullptr), ok(false) {
			ok = (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK);
		}
		~Statement() {
			if (stmt) {
				sqlite3_finalize(stmt);
			}
		}
		Statement(const Statement&) = delete;
		Statement& operator = (const Statement&) = delete;

		bool valid() const { return ok; }

		void bind(int index, const std::string& value) {
			sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
		}
		int step() { return sqlite3_step(stmt); }
		void reset() {
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
		}
		// Return column text
		std::string text(int column) {
			const unsigned char* value = sqlite3_column_text(stmt, column);
			return value ? reinterpret_cast<const char*>(value) : "";
		}

	private:
		sqlite3_stmt* stmt;
		bool ok;
	};

}

CourseDatabase::CourseDatabase() : db(nullptr) {}

CourseDatabase::~CourseDatabase() {
	close();
}

bool CourseDatabase::open(const std::string& path, std::string& error) {
	close();

	if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
		error = db ? sqlite3_errmsg(db) : "Couldn't open database";
		close();
		return false;
	}

	// SQLite enforces foreign keys if asked, per connection https://sqlite.org/foreignkeys.html
	if (!exec("PRAGMA foreign_keys = ON", error)) {
		close();
		return false;
	}
	if (!createSchema(error)) {
		close();
		return false;
	}
	return true;
}

void CourseDatabase::close() {
	if (db) {
		sqlite3_close(db);
		db = nullptr;
	}
}

bool CourseDatabase::exec(const char* sql, std::string& error) {
	char* message = nullptr;
	int result = sqlite3_exec(db, sql, nullptr, nullptr, &message);
	if (result != SQLITE_OK) {
		error = message ? message : "unknown database error";
		sqlite3_free(message);
		return false;
	}
	return true;
}

bool CourseDatabase::createSchema(std::string& error) {
	// Table resource https://www.sqlite.org/lang_createtable.html

	const char* schema =
		"CREATE TABLE IF NOT EXISTS Course ("
		"	course_num TEXT PRIMARY KEY,"
		"	course_title TEXT NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS Prerequisite ("
		"	course_num	TEXT NOT NULL,"
		"	prerequisite_num TEXT NOT NULL,"
		"	PRIMARY KEY (course_num, prerequisite_num),"
		"	FOREIGN KEY (course_num) REFERENCES Course(course_num) ON DELETE CASCADE,"
		"	FOREIGN KEY (prerequisite_num) REFERENCES Course(course_num),"
		"	CHECK (course_num <> prerequisite_num)"
		");"
		"CREATE INDEX IF NOT EXISTS idx_prerequisite_prerequisite_num"
		"	ON Prerequisite(prerequisite_num);";
	return exec(schema, error);
}

bool CourseDatabase::courseExists(const std::string& courseNum) {
	Statement statement(db, "SELECT 1 FROM Course WHERE course_num = ?;");
	if (!statement.valid()) {
		return false;
	}
	statement.bind(1, courseNum);
	return statement.step() == SQLITE_ROW;
}

// CREATE
// Add course to table
bool CourseDatabase::addCourse(const Course& course, std::string& error) {
	if (course.courseNum.empty()) {
		error = "Course number cannot be empty";
		return false;
	}

	if (!exec("BEGIN;", error)) {
		return false;
	}
	// Undo amd report why
	auto fail = [&](const std::string& message) {
		std::string ignored;
		exec("ROLLBACK;", ignored);
		error = message;
		return false;
	};

	{
		Statement insertCourse(db, "INSERT INTO Course (course_num, course_title) VALUES (?,?);");
		if (!insertCourse.valid()) {
			return fail(sqlite3_errmsg(db));
		}
		insertCourse.bind(1, course.courseNum);
		insertCourse.bind(2, course.courseTitle);
		int result = insertCourse.step();
		if (result == SQLITE_CONSTRAINT) {
			return fail("Course " + course.courseNum + " is already input");
		}
		if (result != SQLITE_DONE) {
			return fail(sqlite3_errmsg(db));
		}
	}

	for (const std::string& prereq : course.prerequisites) {
		if (!courseExists(prereq)) {
			return fail("Prerequisite " + prereq + " is not in the list");
		}

		Statement insertPrereq(db, "INSERT INTO Prerequisite (course_num, prerequisite_num) VALUES (?,?);");
		if (!insertPrereq.valid()) {
			return fail(sqlite3_errmsg(db));
		}
		insertPrereq.bind(1, course.courseNum);
		insertPrereq.bind(2, prereq);
		int result = insertPrereq.step();
		if (result == SQLITE_CONSTRAINT) {
			return fail("Prerequisite " + prereq + " could not be added");
		}
		if (result != SQLITE_DONE) {
			return fail(sqlite3_errmsg(db));
		}
	}

	if (!exec("COMMIT;", error)) {
		return fail(error);
	}
	return true;
}

bool CourseDatabase::replaceAllCourses(const std::vector<Course>& courses, ImportResult& result, std::string& error) {
	result = ImportResult();

	if (!exec("BEGIN;", error)) {
		return false;
	}
	auto fail = [&](const std::string& message) {
		std::string ignored;
		exec("ROLLBACK;", ignored);
		error = message;
		return false;
		};

	// Prereq rows first
	if (!exec("DELETE FROM Prerequisite;", error) || !exec("DELETE FROM Course;", error)) {
		return fail(error);
	}

	// Pass 1
	// Insert every course
	std::vector<bool> inserted(courses.size(), false);
	{
		Statement insertCourse(db, "INSERT INTO Course (course_num, course_title) VALUES (?,?);");
		if (!insertCourse.valid()) {
			return fail(sqlite3_errmsg(db));
		}
		for (size_t i = 0; i < courses.size(); i++) {
			insertCourse.reset();
			insertCourse.bind(1, courses[i].courseNum);
			insertCourse.bind(2, courses[i].courseTitle);
			int step = insertCourse.step();
			if (step == SQLITE_DONE) {
				inserted[i] = true;
				result.coursesInserted++;
			} else if (step == SQLITE_CONSTRAINT) {
				result.warnings.push_back("Skipped duplicated course " + courses[i].courseNum + ".");
			} else {
				return fail(sqlite3_errmsg(db));
			}
		}
	}

	// Pass 2
	// Insert prereqs
	{
		Statement insertPrereq(db, "INSERT INTO Prerequisite (course_num, prerequisite_num) VALUES (?,?);");
		if (!insertPrereq.valid()) {
			return fail(sqlite3_errmsg(db));
		}
		for (size_t i = 0; i < courses.size(); i++) {
			if (!inserted[i]) {
				continue;
			}
			for (const std::string& prereq : courses[i].prerequisites) {
				if (!courseExists(prereq)) {
					result.warnings.push_back(courses[i].courseNum + " lists prerequisite " + prereq + ", which isn't in the course list, skipped for now");
					continue;
				} 
				insertPrereq.reset();
				insertPrereq.bind(1, courses[i].courseNum);
				insertPrereq.bind(2, prereq);
				int step = insertPrereq.step();
				if (step == SQLITE_DONE) {
					result.prerequisitesInserted++;
				} else if (step == SQLITE_CONSTRAINT) {
					result.warnings.push_back(courses[i].courseNum + " lists prerequisite" + prereq + " twice or itself, skipped for now");
				} else {
					return fail(sqlite3_errmsg(db));
				}
			} 
		}
	}

	if (!exec("COMMIT;", error)) {
		return fail(error);
	}
	return true;
}

// READ
bool CourseDatabase::getCourse(const std::string& courseNum, Course& out) {
	Statement findCourse(db, "SELECT course_title FROM Course WHERE course_num = ?;");
	if (!findCourse.valid()) {
		return false;
	}
	findCourse.bind(1, courseNum);
	if (findCourse.step() != SQLITE_ROW) {
		return false;
	}

	out = Course();
	out.courseNum = courseNum;
	out.courseTitle = findCourse.text(0);

	Statement findPrereqs(db, "SELECT prerequisite_num FROM Prerequisite WHERE course_num = ? ORDER BY prerequisite_num;");
	if (findPrereqs.valid()) {
		findPrereqs.bind(1, courseNum);
		while (findPrereqs.step() == SQLITE_ROW) {
			out.prerequisites.push_back(findPrereqs.text(0));
		}
	}
	return true;
}

std::vector<Course> CourseDatabase::getAllCourses() {
	std::vector <Course> courses;

	// One query instead of per course
	// LEFT JOIN to keep courses with no prereqs
	// ORDER BY for SQLite to sort
	Statement query(db,
		"SELECT c.course_num, c.course_title, p.prerequisite_num "
		"FROM Course c LEFT JOIN Prerequisite p ON p.course_num = c.course_num "
		"ORDER BY c.course_num, p.prerequisite_num;");
	if (!query.valid()) {
		return courses;
	}

	while (query.step() == SQLITE_ROW) {
		std::string courseNum = query.text(0);
		if (courses.empty() || courses.back().courseNum != courseNum) {
			Course course;
			course.courseNum = courseNum;
			course.courseTitle = query.text(1);
			courses.push_back(course);
		}
		std::string prereq = query.text(2);
		if (!prereq.empty()) {
			courses.back().prerequisites.push_back(prereq);
		}
	}
	return courses;
}

std::vector<Course> CourseDatabase::getCourseRequiring(const std::string& courseNum) {
	std::vector<Course> courses;

	Statement query(db,
		"SELECT c.course_num, c.course_title "
		"FROM Prerequisite p JOIN Course c ON c.course_num = p.course_num "
		"WHERE p.prerequisite_num = ? "
		"ORDER BY c.course_num;");
	if (!query.valid()) {
		return courses;
	}
	query.bind(1, courseNum);

	while (query.step() == SQLITE_ROW) {
		Course course;
		course.courseNum = query.text(0);
		course.courseTitle = query.text(1);
		courses.push_back(course);
	}
	return courses;
}

// UPDATE
bool CourseDatabase::updateCourseTitle(const std::string& courseNum, const std::string& newTitle, std::string& error) {
	Statement update(db, "UPDATE Course SET course_title = ? WHERE course_num = ?;");
	if (!update.valid()) {
		error = sqlite3_errmsg(db);
		return false;
	}
	update.bind(1, newTitle);
	update.bind(2, courseNum);

	if (update.step() != SQLITE_DONE) {
		error = sqlite3_errmsg(db);
		return false;
	}
	if (sqlite3_changes(db) == 0) {
		error = "Course " + courseNum + " not found";
		return false;
	}
	return true;
}

// DELETE
bool CourseDatabase::deleteCourse(const std::string& courseNum, std::string& error) {
	Statement remove(db, "DELETE FROM Course WHERE course_num = ?;");
	if (!remove.valid()) {
		error = sqlite3_errmsg(db);
		return false;
	}
	remove.bind(1, courseNum);

	int result = remove.step();
	if (result == SQLITE_CONSTRAINT) {
		// Foreign key blocked delete
		error = "Cannot delete " + courseNum + ", listed as a prerequisite for other courses";
		return false;
	}
	if (result != SQLITE_DONE) {
		error = sqlite3_errmsg(db);
		return false;
	}
	if (sqlite3_changes(db) == 0) {
		error = "Course " + courseNum + " not found";
		return false;
	}
	return true;
}