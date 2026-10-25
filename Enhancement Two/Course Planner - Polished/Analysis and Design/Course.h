#ifndef COURSE_H
#define COURSE_H

#include <string>
#include <vector>

// Structure to hold course information such as title, number, and prereq

struct Course {
	std::string courseNum;
	std::string courseTitle;
	std::vector<std::string> prerequisites;
};

#endif