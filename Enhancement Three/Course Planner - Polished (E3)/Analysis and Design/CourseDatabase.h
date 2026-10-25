#ifndef COURSE_DATABASE_H
#define COURSE_DATABASE_H

#include <string>
#include <vector>
#include "Course.h"

struct sqlite3;

// Enhancement 3
// Original program read a CSV into its memory on each run and kept the prereqs as a list inside each course
// This now replaces it with a SQLite database that will persist between runs and store the data relationally

class CourseDatabase {
public:
	// Summary of bulk import
	struct ImportResult {
		int coursesInserted = 0;
		int prerequisitesInserted = 0;
		std::vector<std::string> warnings;
	};

	CourseDatabase();
	~CourseDatabase();
	CourseDatabase(const CourseDatabase&) = delete;
	CourseDatabase& operator = (const CourseDatabase&) = delete;

	// Opens / creates database file with foreign key enforcement
	bool open(const std::string& path, std::string& error);
	void close();

	// CRUD - Create, read, update, delete

	// CREATE
	// Adds course and its prereqs in a single transaction
	bool addCourse(const Course& course, std::string& error);
	// Replace everything in database with given courses for CSV import, reports and skips prereqs that are not a name of a course, may change in the coming weeks
	bool replaceAllCourses(const std::vector<Course>& courses, ImportResult& result, std::string& error);

	// READ
	// Retrieves courses
	bool getCourse(const std::string& courseNum, Course& out);
	std::vector<Course> getAllCourses();
	std::vector<Course> getCourseRequiring(const std::string& courseNum);

	// UPDATE
	// Change course information
	bool updateCourseTitle(const std::string& courseNum, const std::string& newTitle, std::string& error);
	// Add or remove a prereq for a course that already exists
	bool addPrerequisite(const std::string& courseNum, const std::string& prerequisiteNum, std::string& error);
	bool removePrerequisite(const std::string& courseNum, const std::string& prerequisiteNum, std::string& error);

	// DELETE
	// Delete course, will fail if another course lists it as a prereq still
	bool deleteCourse(const std::string& courseNum, std::string& error);

	bool courseExists(const std::string& courseNum);

private:
	sqlite3* db;

	bool exec(const char* sql, std::string& error);
	bool createSchema(std::string& error);
};

#endif