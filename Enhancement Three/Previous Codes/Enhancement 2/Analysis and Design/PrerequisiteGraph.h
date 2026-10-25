#ifndef PREREQUISITE_GRAPH_H
#define PREREQUISITE_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "Course.h"

// Original program stored prereqs as a flat vector<string> inside the course, with no checks for real prereqs
// or any way to find a possible chain, such as two classes being more side-by-side and being "prereqs" for each other
// Now builds directed graph for validation rather than just storing text

class PrerequisiteGraph {
private:
	// adjacency[x] - List of courses that require x as a prereq
	std::unordered_map<std::string, std::vector<std::string>> adjacency;
	std::unordered_set<std::string> allNodes;

	enum class VisitState { U, S, V};

	// Depth traversal for cycle detection (DFS) https://www.geeksforgeeks.org/cpp/cpp-program-for-dfs-traversal/
	// U = unvisited
	// S = on the DFS stack
	// V = visited and processed
	bool dfsHasCycle(const std::string& node, std::unordered_map<std::string, VisitState>& state, std::string& cycleNodeOut) const
	{
		state[node] = VisitState::S;

		auto it = adjacency.find(node);
		if (it != adjacency.end()) {
			for (const std::string& neighbor : it->second) {
				if (state[neighbor] == VisitState::S) {
					cycleNodeOut = neighbor;
					return true;
				}
				if (state[neighbor] == VisitState::U) {
					if (dfsHasCycle(neighbor, state, cycleNodeOut)) {
						return true;
					}
				}
			}
		}

		state[node] = VisitState::V;
		return false;
	}

public:
	// Rebuilds graph on given prereqs listed
	void build(const std::vector<Course>& courses) {
		adjacency.clear();
		allNodes.clear();

		for (const Course& c : courses) {
			allNodes.insert(c.courseNum);
			for (const std::string& prereq : c.prerequisites) {
				allNodes.insert(prereq);
				adjacency[prereq].push_back(c.courseNum);
			}
		}
	}

	// Return true if prereq graph contains cycle (a -> b, b -> a). Can point to cycle so have a location for it
	bool hasCycle(std::string& cycleNodeOut) const {
		std::unordered_map<std::string, VisitState> state;
		for (const std::string& node : allNodes) {
			state[node] = VisitState::U;
		}

		for (const std::string& node : allNodes) {
			if (state[node] == VisitState::U) {
				if (dfsHasCycle(node, state, cycleNodeOut)) {
					return true;
				}
			}
		}
		return false;
	}

	// Kahn's algorithm, repeatedly remove nodes with no un-met prereqs https://www.geeksforgeeks.org/cpp/kahns-algorithm-in-cpp/
	std::vector<std::string> topologicalSort() const {
		std::unordered_map<std::string, int> inDegree;
		for (const std::string& node : allNodes) {
			inDegree[node] = 0;
		}
		for (const auto& entry : adjacency) {
			for (const std::string& dependent : entry.second) {
				inDegree[dependent]++;
			}
		}

		std::queue<std::string> ready;
		for (const auto& entry : inDegree) {
			if (entry.second == 0) {
				ready.push(entry.first);
			}
		}

		std::vector<std::string> order;
		while (!ready.empty()) {
			std::string node = ready.front();
			ready.pop();
			order.push_back(node);

			auto it = adjacency.find(node);
			if (it != adjacency.end()) {
				for (const std::string& dependent : it->second) {
					inDegree[dependent]--;
					if (inDegree[dependent] == 0) {
						ready.push(dependent);
					}
				}
			}
		}

		return order;
	}
	size_t nodeCount() const { return allNodes.size(); }
};

#endif