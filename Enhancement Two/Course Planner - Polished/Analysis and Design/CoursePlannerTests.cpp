#include "CoursePlannerTests.h"
#include "HashTable.h"
#include "PrerequisiteGraph.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// Testing suite for the hash table and prereq graph, checks normal and invalid data

namespace
{
	// Runs one check, print PASS/FAIL with a message
	bool check(bool condition, const std::string& description)
	{
		std::cout << (condition ? " [PASS] " : " [FAIL] ") << description << std::endl;
		return condition;
	}

	// Quick way to build a course for a test
	Course makeCourse(const std::string& number, const std::vector<std::string>& prereqs = {})
	{
		Course c;
		c.courseNum = number;
		c.courseTitle = "Course " + number;
		c.prerequisites = prereqs;
		return c;
	}

	// The original sum of characters hash, copied so the new hash can be compared to it
	int oldHash(const std::string& key, int tableSize)
	{
		int sum = 0;
		for (char c : key) {
			sum += c;
		}
		return sum % tableSize;
	}

	// Longest chain the old hash would make for these keys in a table of this size
	int oldLongestChain(const std::vector<std::string>& keys, int tableSize)
	{
		std::unordered_map<int, int> counts;
		int longest = 0;
		for (const std::string& key : keys) {
			int count = ++counts[oldHash(key, tableSize)];
			if (count > longest) {
				longest = count;
			}
		}
		return longest;
	}

	// True if every prereq comes before the course that needs it, and every course is in the order
	bool orderIsValid(const std::vector<std::string>& order, const std::vector<Course>& courses)
	{
		std::unordered_map<std::string, size_t> position;
		for (size_t i = 0; i < order.size(); i++) {
			position[order[i]] = i;
		}
		for (const Course& c : courses) {
			if (position.count(c.courseNum) == 0) {
				return false;
			}
			for (const std::string& prereq : c.prerequisites) {
				if (position.count(prereq) == 0 || position[prereq] >= position[c.courseNum]) {
					return false;
				}
			}
		}
		return true;
	}
}

bool runCoursePlannerTests()
{
	std::cout << "Running unit tests\n";
	bool allPassed = true;

	// Test 1: Resizing, table starts at 20 buckets and resizes when load factor would pass 0.7
	// 14 courses = exactly 0.7 so no resize, the 15th would be 0.75 so it doubles to 40
	{
		HashTable table(20);
		for (int i = 0; i < 14; i++) {
			table.insert(makeCourse("CS" + std::to_string(100 + i)));
		}
		allPassed &= check(table.getTableSize() == 20, "resize: 14 courses (load factor 0.7) does not resize the table");

		table.insert(makeCourse("CS200"));
		allPassed &= check(table.getTableSize() == 40, "resize: the 15th course doubles the table to 40 buckets");
	}

	// Test 2: Load factor never passes 0.7, even after 200 inserts
	{
		HashTable table(20);
		bool stayedUnder = true;
		for (int i = 0; i < 200; i++) {
			table.insert(makeCourse("CS" + std::to_string(100 + i)));
			if (static_cast<double>(table.getNumElements()) / table.getTableSize() > 0.7) {
				stayedUnder = false;
			}
		}
		allPassed &= check(stayedUnder, "resize: load factor stays at or under 0.7 across 200 inserts");
	}

	// Test 3: Rehashing keeps every course and its data after several resizes
	{
		HashTable table(20);
		for (int i = 0; i < 100; i++) {
			table.insert(makeCourse("CS" + std::to_string(100 + i), { "CS100" }));
		}
		bool allFound = true;
		for (int i = 0; i < 100; i++) {
			Course* found = table.search("CS" + std::to_string(100 + i));
			if (found == nullptr || found->courseTitle != "Course CS" + std::to_string(100 + i) ||
				found->prerequisites.size() != 1 || found->prerequisites[0] != "CS100") {
				allFound = false;
			}
		}
		allPassed &= check(allFound && table.getNumElements() == 100, "lookup: all 100 courses are found with their data intact after resizing");
		allPassed &= check(table.search("CS999") == nullptr, "lookup: a course that was never added returns nullptr");
	}

	// Test 4: Collisions
	// Anagram course numbers used to land in the same bucket with the sum hash
	{
		HashTable table(40);
		allPassed &= check(table.bucketIndex("CS101") != table.bucketIndex("CS110") && table.bucketIndex("CS201") != table.bucketIndex("CS210"),
			"collision: anagram course numbers (CS101 and CS110, CS201 and CS210) land in different buckets");
	}

	// Two different courses in the same bucket are both kept (chaining)
	{
		HashTable table(20);
		// 21 keys and 20 buckets, so at least two have to share a bucket
		std::string first, second;
		for (int i = 0; i < 21 && second.empty(); i++) {
			std::string key = "A" + std::to_string(i);
			for (int j = 0; j < i; j++) {
				std::string other = "A" + std::to_string(j);
				if (table.bucketIndex(key) == table.bucketIndex(other)) {
					first = other;
					second = key;
					break;
				}
			}
		}
		table.insert(makeCourse(first));
		table.insert(makeCourse(second));
		allPassed &= check(!second.empty() && table.longestChain() == 2 && table.search(first) != nullptr && table.search(second) != nullptr,
			"collision: two courses in the same bucket are both stored and found");
	}

	// The new hash makes shorter chains than the old one on the same course numbers and table size
	{
		std::vector<std::string> keys;
		HashTable table(20);
		for (int i = 100; i < 200; i++) {
			keys.push_back("CS" + std::to_string(i));
			table.insert(makeCourse(keys.back()));
		}
		int oldChain = oldLongestChain(keys, table.getTableSize());
		allPassed &= check(table.longestChain() < oldChain,
			"collision: CS100 to CS199 longest chain is " + std::to_string(table.longestChain()) + " now, it was " + std::to_string(oldChain) + " with the old hash");
	}

	// Test 5: Cycle detection and ordering on normal data
	// CS100 -> CS101 -> CS201 -> CS300, each course needs the one before it
	{
		std::vector<Course> courses = { makeCourse("CS100"), makeCourse("CS101", { "CS100" }), makeCourse("CS201", { "CS101" }), makeCourse("CS300", { "CS201" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(!graph.hasCycle(cycleNode), "cycle: a normal chain of prereqs has no cycle");
		allPassed &= check(orderIsValid(graph.topologicalSort(), courses), "order: every prereq comes before the course that needs it");
	}

	// Diamond: CS201 and CS202 both need CS101, CS300 needs both. A shared prereq is not a cycle
	{
		std::vector<Course> courses = { makeCourse("CS101"), makeCourse("CS201", { "CS101" }), makeCourse("CS202", { "CS101" }), makeCourse("CS300", { "CS201", "CS202" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(!graph.hasCycle(cycleNode) && orderIsValid(graph.topologicalSort(), courses) && graph.topologicalSort().size() == 4,
			"order: shared prereqs (a diamond) are not flagged as a cycle and still order correctly");
	}

	// Test 6: Invalid data, circular prereqs
	{
		std::vector<Course> courses = { makeCourse("CS100", { "CS200" }), makeCourse("CS200", { "CS100" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(graph.hasCycle(cycleNode) && (cycleNode == "CS100" || cycleNode == "CS200"), "cycle: two courses that need each other are detected");
		allPassed &= check(graph.topologicalSort().size() < graph.nodeCount(), "order: the sort leaves out courses that are stuck in a cycle");
	}

	{
		std::vector<Course> courses = { makeCourse("CS100", { "CS300" }), makeCourse("CS200", { "CS100" }), makeCourse("CS300", { "CS200" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(graph.hasCycle(cycleNode), "cycle: a loop through three courses is detected");
	}

	{
		std::vector<Course> courses = { makeCourse("CS100", { "CS100" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(graph.hasCycle(cycleNode), "cycle: a course that requires itself is detected");
	}

	// Cycle is off to the side of valid courses
	{
		std::vector<Course> courses = { makeCourse("CS100"), makeCourse("CS101", { "CS100" }), makeCourse("CS500", { "CS501" }), makeCourse("CS501", { "CS500" }) };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::string cycleNode;
		allPassed &= check(graph.hasCycle(cycleNode), "cycle: a loop is still found when other courses are valid");
	}

	// Test 7: Invalid data, a prereq that is not a course
	{
		std::vector<Course> courses = { makeCourse("MAT225", { "MAT140" }), makeCourse("PHY150", { "MAT140" }), makeCourse("CS110") };
		PrerequisiteGraph graph;
		graph.build(courses);

		std::vector<std::string> missing = graph.undefinedPrerequisites();
		std::vector<std::string> users = graph.requiredBy("MAT140");
		allPassed &= check(missing.size() == 1 && missing[0] == "MAT140" && users.size() == 2 && users[0] == "MAT225" && users[1] == "PHY150",
			"invalid data: MAT140 is reported as not a course, with the courses that require it");
	}

	{
		std::vector<Course> courses = { makeCourse("CS100"), makeCourse("CS101", { "CS100" }) };
		PrerequisiteGraph graph;
		graph.build(courses);
		allPassed &= check(graph.undefinedPrerequisites().empty(), "invalid data: nothing is reported when every prereq is a real course");
	}

	// Test 8: No courses at all
	{
		PrerequisiteGraph graph;
		graph.build({});

		std::string cycleNode;
		allPassed &= check(!graph.hasCycle(cycleNode) && graph.topologicalSort().empty() && graph.nodeCount() == 0, "invalid data: an empty course list is handled without errors");
	}

	std::cout << (allPassed ? "All tests passed\n\n" : "One or more tests failed\n\n");
	return allPassed;
}