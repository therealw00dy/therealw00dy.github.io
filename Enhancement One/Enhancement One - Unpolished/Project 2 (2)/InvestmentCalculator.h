#ifndef INVESTMENT_CALCULATOR_H
#define INVESTMENT_CALCULATOR_H

#include <vector>
#include "YearRecord.h"

// Compounding interest math, replacing original Investment class two identical methods with a single method parameterized by the monthlyDeposit
// Also does not print details in the method, leaves it external

class InvestmentCalculator
{
public:
	InvestmentCalculator(double initialInvestment, double apy, int numYears);

	// Returns on YearRecord per year
	// Pass monthlyDeposit = 0.0 to keep original without monthly deposit behavior
	std::vector<YearRecord> calculateYearlyBalances(double monthlyDeposit) const;

private:
	double initialInvestment;
	double apy;
	int numYears;
};

#endif