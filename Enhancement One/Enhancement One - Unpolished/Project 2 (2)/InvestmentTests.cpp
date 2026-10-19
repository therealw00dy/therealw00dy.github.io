#include "InvestmentTests.h"
#include "InvestmentCalculator.h"
#include <iostream>
#include <cmath>

// Testing suite added to follow enhancement guidelines and reinforce good coding practice

namespace
{
	// Two dollar amounts "close enough if within a cent, exact equality can be unreliable for compounding math with floating points
	bool approxEqual(double a, double b, double epsilon = 0.01)
	{
		return std::fabs(a - b) < epsilon;
	}

	// Runs one check, print PASS/FAIL with a message
	bool check(bool condition, const std::string& description)
	{
		std::cout << (condition ? " [PASS] " : " [FAIL] ") << description << std::endl;
		return condition;
	}
}

bool runInvestmentTests()
{
	std::cout << "Running unit tests\n";
	bool allPassed = true;

	// Test 1: No monthly deposit, one year, 5% APY
	// Expected balance $1051.16
	{
		InvestmentCalculator calculator(1000.0, 5.0, 1);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(0.0);

		allPassed &= check(results.size() == 1, "testOneYearNoDeposit: returns exactly one YearRecord");
		allPassed &= check(approxEqual(results[0].balance, 1051.16), "testOneYearNoDeposit: year-end balance is $1051.16");
		allPassed &= check(approxEqual(results[0].interestEarnedThisYear, 51.16), "testOneYearNoDeposit: interest earned is $51.16");
	}

	// Test 2: $100 monthly deposit, five years, 5% APY
	
	{
		InvestmentCalculator calculator(1000.0, 5.0, 5);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(100.0);

		allPassed &= check(results.size() == 5, "testFiveYearsWithDeposit: returns five YearRecords");
		allPassed &= check(approxEqual(results[0].balance, 2284.16), "testFiveYearsWithDeposit: year 1 balance is $2284.16");
		allPassed &= check(approxEqual(results[4].balance, 8112.30), "testFiveYearsWithDeposit: year 5 balance is $8112.30");
	}

	// Test 3: 0% APY should show no interest at all

	{
		InvestmentCalculator calculator(1000.0, 0.0, 3);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(0.0);

		allPassed &= check(approxEqual(results[2].balance, 1000.0), "testZeroInterestNoGrowth: balance is unchanged after 3 years at 0% APY");
		allPassed &= check(approxEqual(results[2].interestEarnedThisYear, 0.00), "testZeroInterestNoGrowth: interest earned is $0.00 at 0% APY");
	}

	std::cout << (allPassed ? "All tests passed\n\n" : "One or more tests failed\n\n");
	return allPassed;

}