#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

// Structure to hold onto course information like class number, title, and prerequisites
struct Course {
    string courseNum;
    string courseTitle;
    vector<string> prerequisites;
};

// Hash table class to store the Course objects
class HashTable {
private:
    // Buckets for chaining and size for number
    vector<list<Course>> table;
    int tableSize;

    // Hash function, calculates index based on the course number
    int hash(string key) {
        int sum = 0;
        for (char c : key) sum += c;
        return sum % tableSize;
    }

public:
    // Constructor initializing hash table
    HashTable(int size = 20) {
        tableSize = size;
        table.resize(size);
    }

    // Insert function to insert information when loading the file
    void insert(Course course) {
        int index = hash(course.courseNum);
        table[index].push_back(course);
    }

    // Search function to find a course using the course number
    Course* search(string courseNum) {
        int index = hash(courseNum);

        for (Course& c : table[index]) {
            if (c.courseNum == courseNum) {
                return &c;
            }
        }
        return NULL;
    }

    // Vector of all courses in table
    vector<Course> getCourses() {
        vector<Course> result;

        // Loop buckets
        for (int i = 0; i < table.size(); i++) {
            for (Course& c : table[i]) {
                result.push_back(c);
            }
        }
        return result;
    }

    // Clear function so it can be used before loading data
    void clear() {
        for (int i = 0; i < table.size(); i++) {
            table[i].clear();
        }
    }
};

// File loading function for the CSV that was provided
void loadFile(HashTable& courseHash) {
    ifstream file("CS 300 ABCU_Advising_Program_Input.csv");
    if (!file.is_open()) {
        cout << "Error: Cannot open file.";
        return;
    }
    
    // Clear hash table
    courseHash.clear();

    // Separate and partition the information in the CSV so we can easily access it when needed throughout the program
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string part;
        vector<string> parts;

        // Splitting each line by commas
        while (getline(ss, part, ',')) {
            parts.push_back(part);
        }

        // Error checking for lines not having at least a course number and title
        if (parts.size() < 2) {
            cout << "Error: Invalid format";
            continue;
        }
        // Assigning number and title to their relative location
        Course c;
        c.courseNum = parts[0];
        c.courseTitle = parts[1];

        // Prerequisite check and inserting to the rest of our parts
        for (int i = 2; i < parts.size(); i++) {
            c.prerequisites.push_back(parts[i]);
        }

        // Insert course into table
        courseHash.insert(c);
    }
}

// Sort for print all function using a comparison
bool sortCourses(Course a, Course b) {
    return a.courseNum < b.courseNum;
}

// Function to print all courses with number and title
void printAll(HashTable& courseHash) {
    vector<Course> courses = courseHash.getCourses();

    // Check for course loading and handle 
    if (courses.size() == 0) {
        cout << "No courses";
        return;
    }
    // Sort courses alphabetically and numerically 
    sort(courses.begin(), courses.end(), sortCourses);

    for (int i = 0; i < courses.size(); i++) {
        cout << courses[i].courseNum << ": " << courses[i].courseTitle << endl;
    }
}

// Function to print specific course using created search function
void printCourse(HashTable& courseHash, string courseNum) {
    Course* c = courseHash.search(courseNum);
    // Invalid choice handling
    if (c == NULL) {
        cout << "No course found" << endl;
        return;
    }
    
    cout << c->courseNum << ": " << c->courseTitle << endl;
    cout << "Prerequisites: ";
    
    // Print prerequisites if they exist
        for (int i = 0; i < c->prerequisites.size(); i++) {
            cout << c->prerequisites[i];

            if (i != c->prerequisites.size() - 1) {
                cout << ", ";
            }
        }
        cout << endl;
    
}

// Menu display, gives input choices for user
void displayMenu() {
    cout << "\nWelcome to the course planner" << endl;
    cout << "1: Load courses" << endl;
    cout << "2: Print all courses" << endl;
    cout << "3: Search and print course information" << endl;
    cout << "9: Exit" << endl;
}

// Main function, runs the choices in the menu display
int main() {
    HashTable courseHash;
    int choice = 0;
    // Looping for choices so the user can keep selecting different items if they please
    while (choice != 9) {
        displayMenu();
        cout << "Enter choice: ";
        cin >> choice;

        // Each function choice for each choice we have listed
        if (choice == 1) {
            loadFile(courseHash);
        } else if (choice == 2) {
            printAll(courseHash);
        } else if (choice == 3) {
            string courseNum;
            cout << "Enter course number: ";
            cin >> courseNum;
            printCourse(courseHash, courseNum);
        }
        else if (choice == 9) {
            cout << "Exiting";
        // Invalid choice handling
        } else {
            cout << "Invalid choice";
        }
    }
    
}