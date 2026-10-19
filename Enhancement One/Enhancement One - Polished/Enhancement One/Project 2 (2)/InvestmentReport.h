#ifndef INVESTMENT_REPORT_H
#define INVESTMENT_REPORT_H

#include <string>
#include <vector>
#include "YearRecord.h"

// Original code had calculation methods call printDetails directly from the loops, which limited the usability of the methods including printing

class InvestmentReport
{
public:

	// Prints titled, formatted table
	static void printYearlyTable(const std::string& title, const std::vector<YearRecord>& records);
};

#endif