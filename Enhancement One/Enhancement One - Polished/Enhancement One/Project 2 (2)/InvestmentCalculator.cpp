#include "InvestmentCalculator.h"

InvestmentCalculator::InvestmentCalculator(double initialInvestment, double apy, int numYears)
	: initialInvestment(initialInvestment), apy(apy), numYears(numYears)
{
}

// Single replacement for original calculations which were the same equations but with and without a monthly deposit.
std::vector<YearRecord> InvestmentCalculator::calculateYearlyBalances(double monthlyDeposit) const
{
	std::vector<YearRecord> results;
	double balance = initialInvestment;
	// Annual rate -> monthly decimal rate: divide by 100 for percent then by 12 months
	double monthlyInterest = (apy / 100) / 12;

	for (int year = 1; year <= numYears; year++) {
		double interestEarnedThisYear = 0;

		for (int month = 1; month <= 12; month++) {
			// Deposit is added before intrest is calculated
			balance += monthlyDeposit;
			double interestEarnedThisMonth = balance * monthlyInterest;
			balance += interestEarnedThisMonth;
			interestEarnedThisYear += interestEarnedThisMonth;
		}

		results.push_back(YearRecord{ year, balance, interestEarnedThisYear });
	}

	return results;
}