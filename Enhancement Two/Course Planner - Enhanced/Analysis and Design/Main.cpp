#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

#include "Course.h"
#include "HashTable.h"
#include "PrerequisiteGraph.h"

using namespace std;

// File loading function for CSV 
void loadFile(HashTable& courseHash) {
	ifstream file("SNHU_CS_COURSE_LIST.csv");
	if (!file.is_open()) {
		cout << "Error, cannot open file";
		return;
	}

	// Clear hash table
	courseHash.clear();

	// Separate and partition CSV information
	string line;
	while (getline(file, line)) {
		stringstream ss(line);
		string part;
		vector<string> parts;

		// Split lines with commas
		while (getline(ss, part, ',')) {
			parts.push_back(part);
		}

		// Error if lines do not have number and title
		if (parts.size() < 2) {
			cout << "Error, invalid format";
			continue;
		}

		// Assign number and title
		Course c;
		c.courseNum = parts[0];
		c.courseTitle = parts[1];

		// Prereq check and insert to other parts
		for (size_t i = 2; i < parts.size(); i++) {
			c.prerequisites.push_back(parts[i]);
		}

		// Insert course into table
		courseHash.insert(c);
	}
}

// Sort for print all
bool sortCourses(const Course& a, const Course& b) {
	return a.courseNum < b.courseNum;
}

// Function to print all courses
void printAll(HashTable& courseHash) {
	vector<Course> courses = courseHash.getCourses();

	if (courses.size() == 0) {
		cout << "No courses" << endl;
		return;
	}
	sort(courses.begin(), courses.end(), sortCourses);

	for (size_t i = 0; i < courses.size(); i++) {
		cout << courses[i].courseNum << ": " << courses[i].courseTitle << endl;
	}
}

// Function to print specific course
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

// Enhancement
// New menu option that build prereq graph from currently loaded class
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

// Menu display
void displayMenu() {
	cout << "\nWelcome to the course planner" << endl;
	cout << "1: Load courses" << endl;
	cout << "2: Print all courses" << endl;
	cout << "3: Search and print course information" << endl;
	cout << "4: Validate prerequisites and print course order" << endl;
	cout << "9: Exit" << endl;
}

// Main function, run display menu choices
int main() {
	cout << endl;

	HashTable courseHash;
	int choice = 0;

	while (choice != 9) {
		displayMenu();
		cout << "Enter choice: ";
		cin >> choice;

		// Original program never checked for fail here

		if (cin.fail()) {
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << "Invalid Choice";
			choice = 0;
			continue;
		}

		if (choice == 1) {
			loadFile(courseHash);
		} else if (choice == 2) {
			printAll(courseHash);
		} else if (choice == 3) {
			string courseNum;
			cout << "Enter course number: ";
			cin >> courseNum;
			printCourse(courseHash, courseNum);
		} else if (choice == 4) {
			validatePrerequisites(courseHash);
		} else if (choice == 9) {
			cout << "Exiting";
		} else {
			cout << "Invalid choice";
		}
	}

	return 0;
}
