#include "InvestmentReport.h"
#include <iostream>
#include <iomanip>

void InvestmentReport::printYearlyTable(const std::string& title, const std::vector<YearRecord>& records)
{

	// Same header and column design from original design, no longer scattered around in other methods

	std::cout << "\n" << title << ":\n";

	std::cout << "=================================================================\n";
	std::cout << std::setw(10) << "Year" << std::setw(15) << "\tYear End Balance"
		<< std::setw(20) << "\tYear End Earned Interest\n";
	std::cout << "=================================================================\n";

	for (const YearRecord& record : records) {
		std::cout << std::setw(10) << record.year
			<< std::setw(15) << std::fixed << std::setprecision(2) << record.balance
			<< std::setw(20) << std::fixed << std::setprecision(2) << record.interestEarnedThisYear
			<< std::endl;
	}
}