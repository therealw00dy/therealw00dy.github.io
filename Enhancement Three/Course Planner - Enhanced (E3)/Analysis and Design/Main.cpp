#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>

#include "Course.h"
#include "HashTable.h"
#include "PrerequisiteGraph.h"
#include "CourseDatabase.h"

using namespace std;

const string DATABASE_FILE = "courses.db";
const string CSV_FILE = "SNHU_CS_COURSE_LIST.csv";

// Clean up strings
string trim(const string& text) {
	const string whitespace = "\t\r\n";
	size_t start = text.find_first_not_of(whitespace);
	if (start == string::npos) {
		return "";
	}
	size_t end = text.find_last_not_of(whitespace);
	return text.substr(start, end - start + 1);
}

// Course numbers trimed and uppercase
string normalizeCourseNum(const string& text) {
	string result = trim(text);
	for (char& c : result) {
		c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
	}
	return result;
}

// Prompt and read whole line
string readLine(const string& prompt) {
	cout << prompt;
	string line;
	getline(cin, line);
	return trim(line);
}

// Read CSV into Course objects, false if cannot be opened
bool parseCSV(const string& path, vector<Course>& courses, vector<string>& problems) {
	ifstream file(path);
	if (!file.is_open()) {
		return false;
	}

	string line;
	int lineNumber = 0;
	while (getline(file, line)) {
		lineNumber++;
		if (trim(line).empty()) {
			continue;
		}

		stringstream ss(line);
		string part;
		vector<string> parts;
		while (getline(ss, part, ',')) {
			parts.push_back(trim(part));
		}

		if (parts.size() < 2 || parts[0].empty()) {
			problems.push_back("Line " + to_string(lineNumber) + " skipped, no course number / title");
			continue;
		}

		Course c;
		c.courseNum = normalizeCourseNum(parts[0]);
		c.courseTitle = parts[1];
		for (size_t i = 2; i < parts.size(); i++) {
			string prereq = normalizeCourseNum(parts[i]);
			if (!prereq.empty()) {
				c.prerequisites.push_back(prereq);
			}
		}
		courses.push_back(c);
	}
	return true;
}

// Database is permanent copy of data
// Hash table is left as a quick in-memory copy for lookups and where the graph is built
void reloadCache(CourseDatabase& db, HashTable& courseHash) {
	courseHash.clear();
	for (const Course& c : db.getAllCourses()) {
		courseHash.insert(c);
	}
}

// Menu option 1
// CSV mass import
void importCourses(CourseDatabase& db, HashTable& courseHash) {
	vector<Course> courses;
	vector<string> problems;
	if (!parseCSV(CSV_FILE, courses, problems)) {
		cout << "Error, cannot open" << CSV_FILE << endl;
		return;
	}
	for (const string& problem : problems) {
		cout << problem << endl;
	}

	CourseDatabase::ImportResult result;
	string error;
	if (!db.replaceAllCourses(courses, result, error)) {
		cout << "Import failed " << error << endl;
		return;
	}

	reloadCache(db, courseHash);
	cout << "Imported " << result.coursesInserted << " courses and " << result.prerequisitesInserted << " prerequisites" << endl;

	if (!result.warnings.empty()) {
		cout << result.warnings.size() << " warnings: " << endl;
		for (const string& warning : result.warnings) {
			cout << warning << endl;
		}
	}
}

// Menu option 2
// Print all courses, SQLite sorts

void printAll(CourseDatabase& db) {
	vector<Course> courses = db.getAllCourses();
	if (courses.empty()) {
		cout << "No courses" << endl;
		return;
	}
	for (const Course& c : courses) {
		cout << c.courseNum << ": " << c.courseTitle << endl;
	}
}

// Menu option 3
// Lookup course in the table
void printCourse(HashTable& courseHash, const string& courseNum) {
	Course* c = courseHash.search(courseNum);
	if (c == nullptr) {
		cout << "No course found" << endl;
		return;
	}

	cout << c->courseNum << ": " << c->courseTitle << endl;
	cout << "Prerequisites: ";

	for (size_t i = 0; i < c->prerequisites.size(); i++) {
		cout << c->prerequisites[i];
		if (i != c->prerequisites.size() - 1) {
			cout << ", ";
		}
	}
	cout << endl;
}

// Menu option 4
// Print valid order
void validatePrerequisites(HashTable& courseHash) {
	vector<Course> courses = courseHash.getCourses();
	if (courses.empty()) {
		cout << "No courses loaded." << endl;
		return;
	}

	PrerequisiteGraph graph;
	graph.build(courses);

	string cycleNode;
	if (graph.hasCycle(cycleNode)) {
		cout << "Error: a circular prerequisite was found involving course " << cycleNode << ". Course data is invalid until fixed" << endl;
		return;
	}

	vector<string> order = graph.topologicalSort();
	if (order.size() != graph.nodeCount()) {
		// Safeguard for topological sort
		cout << "Error: could not produce course order" << endl;
		return;
	}

	cout << "Valid order" << endl;
	for (size_t i = 0; i < order.size(); i++) {
		cout << (i + 1) << ". " << order[i] << endl;
	}
}

// Menu option 5
// Add a course - CREATE
void addCourseMenu(CourseDatabase& db, HashTable& courseHash) {
	Course course;
	course.courseNum = normalizeCourseNum(readLine("Course number: "));
	if (course.courseNum.empty()) {
		cout << "Course number cannot be empty" << endl;
		return;
	}
	course.courseTitle = readLine("Course title: ");
	if (course.courseTitle.empty()) {
		cout << "Course title cannot be empty" << endl;
		return;
	}

	string prereqLine = readLine("Prerequisites (course numbers separated by commas, or hit enter for none): ");
	stringstream ss(prereqLine);
	string part;
	while (getline(ss, part, ',')) {
		string prereq = normalizeCourseNum(part);
		if (!prereq.empty()) {
			course.prerequisites.push_back(prereq);
		}
	}

	string error;
	if (!db.addCourse(course, error)) {
		cout << "Could not add course " << error << endl;
		return;
	}
	reloadCache(db, courseHash);
	cout << "Added course" << endl;
}

// Menu option 6
// Change a course - UPDATE
void updateTitleMenu(CourseDatabase& db, HashTable& courseHash) {
	string courseNum = normalizeCourseNum(readLine("Course number to update: "));
	string newTitle = readLine("New title: ");
	if (newTitle.empty()) {
		cout << "Title cannot be empty" << endl;
		return;
	}

	string error;
	if (!db.updateCourseTitle(courseNum, newTitle, error)) {
		cout << "Could not update course" << error << endl;
		return;
	}
	reloadCache(db, courseHash);
	cout << "Updated " << courseNum << endl;
}

// Menu option 7
// Delete course - DELETE
void deleteCourseMenu(CourseDatabase& db, HashTable& courseHash) {
	string courseNum = normalizeCourseNum(readLine("Course number to delete: "));

	string error;
	if (!db.deleteCourse(courseNum, error)) {
		cout << error << endl;
		return;
	}
	reloadCache(db, courseHash);
	cout << "Deleted" << endl;
}

// Menu option 8
// Check what courses have this as a prereq
void requiredByMenu(CourseDatabase& db) {
	string courseNum = normalizeCourseNum(readLine("Course number: "));
	if (!db.courseExists(courseNum)) {
		cout << "No course found" << endl;
		return;
	}

	vector<Course> courses = db.getCourseRequiring(courseNum);
	if (courses.empty()) {
		cout << "No courses require this course" << endl;
		return;
	}
	cout << "Courses that require " << courseNum << ":" << endl;
	for (const Course& c : courses) {
		cout << c.courseNum << ": " << c.courseTitle << endl;
	}
}


// Menu display
void displayMenu() {
	cout << "\nWelcome to the course planner" << endl;
	cout << "1: Import CSV into database" << endl;
	cout << "2: Print all courses" << endl;
	cout << "3: Search and print course information" << endl;
	cout << "4: Validate prerequisites and print course order" << endl;
	cout << "5: Add course" << endl;
	cout << "6: Update course title" << endl;
	cout << "7: Delete course" << endl;
	cout << "8: Show courses that require course" << endl;
	cout << "9: Exit" << endl;
}

// Main function, run display menu choices
int main() {
	cout << endl;

	CourseDatabase db;
	string error;
	if (!db.open(DATABASE_FILE, error)) {
		cout << "Could not open database " << error << endl;
		return 1;
	}

	HashTable courseHash;
	reloadCache(db, courseHash);
	size_t loaded = courseHash.getCourses().size();
	if (loaded > 0) {
		cout << "Loaded " << loaded << " courses from " << DATABASE_FILE << endl;
	}

	int choice = 0;
	while (choice != 9) {
		displayMenu();
		cout << "Enter choice: ";
		cin >> choice;

		// Original program never checked for fail here

		if (cin.fail()) {
			if (cin.eof()) {
				break; // End of input check
			}
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << "Invalid Choice";
			choice = 0;
			continue;
		}
		// Discard rest of line so readLine() is clean
		cin.ignore((numeric_limits<streamsize>::max)(), '\n');
		cout << "\n";

		if (choice == 1) {
			importCourses(db, courseHash);
		} else if (choice == 2) {
			printAll(db);
		} else if (choice == 3) {
			printCourse(courseHash, normalizeCourseNum(readLine("Enter course number: ")));
		} else if (choice == 4) {
			validatePrerequisites(courseHash);
		} else if (choice == 5) {
			addCourseMenu(db, courseHash);
		} else if (choice == 6) {
			updateTitleMenu(db, courseHash);
		} else if (choice == 7) {
			deleteCourseMenu(db, courseHash);
		} else if (choice == 8) {
			requiredByMenu(db);
		} else if (choice == 9) {
			cout << "Exiting";
		} else {
			cout << "Invalid choice";
		}
	}

	return 0;
}
