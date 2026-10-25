#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <vector>
#include <list>
#include "Course.h"

// Enhancement 2, changes from original hash table
// Hash function was sum of character, which does not differentiate location. Replaced with polynomial rolling hash.
// Table size was fixed with no resizing, performance degradation will occur as more courses are loaded than it was designed for
// table will now resize

class HashTable {
private:
	std::vector<std::list<Course>> table;
	int tableSize;
	int numElements;

	static constexpr double LOAD_FACTOR_THRESHOLD = 0.7;

	// Polynomial rolling hash
	// Using current character position instead of just summing codes

	int hash(const std::string& key) const {
		unsigned long h = 0;
		for (unsigned char c : key) {
			h = h * 31 + c;
		}
		return static_cast<int>(h % static_cast<unsigned long>(tableSize));
	}

	// Rebuilds table at new size and re inputs existing courses, recalculating buckets under new tableSize
	// Every entry rehashed

	void resize(int newSize) {
		std::vector<std::list<Course>> oldTable = table;
		tableSize = newSize;
		table.clear();
		table.resize(newSize);

		for (std::list<Course>& bucket : oldTable) {
			for (Course& c : bucket) {
				int index = hash(c.courseNum);
				table[index].push_back(c);
			}
		}
	}

public:
	// Constructor initializing hash table
	HashTable(int size = 20) {
		tableSize = size;
		numElements = 0;
		table.resize(size);
	}

	// Insert function to insert information
	void insert(Course course) {
		// Check load factor before insert so table does not exceed threshold
		double projectedLoadFactor = static_cast<double>(numElements + 1) / tableSize;
		if (projectedLoadFactor > LOAD_FACTOR_THRESHOLD) {
			resize(tableSize * 2);
		}

		int index = hash(course.courseNum);
		table[index].push_back(course);
		numElements++;
	}

	// Search function to find course using number
	Course* search(const std::string& courseNum) {
		int index = hash(courseNum);

		for (Course& c : table[index]) {
			if (c.courseNum == courseNum) {
				return &c;
			}
		}
		return nullptr;
	}

	// Vector of all courses in table
	std::vector<Course> getCourses() const {
		std::vector<Course> result;

		for (size_t i = 0; i < table.size(); i++) {
			for (const Course& c : table[i]) {
				result.push_back(c);
			}
		}
		return result;
	}

	// Clear function
	void clear() {
		for (size_t i = 0; i < table.size(); i++) {
			table[i].clear();
		}
		numElements = 0;
	}

	// Used by the tests
	int getTableSize() const { return tableSize; }
	int getNumElements() const { return numElements; }

	// Which bucket a course number goes in
	int bucketIndex(const std::string& key) const { return hash(key); }

	// Size of the biggest bucket, longer means more collisions
	int longestChain() const {
		size_t longest = 0;
		for (const std::list<Course>& bucket : table) {
			if (bucket.size() > longest) {
				longest = bucket.size();
			}
		}
		return static_cast<int>(longest);
	}

};

#endif