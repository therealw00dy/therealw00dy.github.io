#include "InvestmentTests.h"
#include "InvestmentCalculator.h"
#include "InputValidator.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>

// Testing suite added to follow enhancement guidelines and reinforce good coding practice

namespace
{
	// Two amounts are close enough if within a cent, exact equality is unreliable with floating points
	bool approxEqual(double a, double b, double epsilon = 0.01)
	{
		return std::fabs(a - b) < epsilon;
	}

	// Checks if the text has the part we are looking for
	bool contains(const std::string& text, const std::string& part)
	{
		return text.find(part) != std::string::npos;
	}

	// Runs one check, print PASS/FAIL with a message
	bool check(bool condition, const std::string& description)
	{
		std::cout << (condition ? " [PASS] " : " [FAIL] ") << description << std::endl;
		return condition;
	}

	// Fake keyboard and screen so input can be tested without typing
	class FakeConsole
	{
	public:
		// Swap in fake input and output
		explicit FakeConsole(const std::string& typed) : input(typed)
		{
			oldIn = std::cin.rdbuf(input.rdbuf());
			oldOut = std::cout.rdbuf(output.rdbuf());
		}
		// Put the real ones back
		~FakeConsole()
		{
			std::cin.rdbuf(oldIn);
			std::cout.rdbuf(oldOut);
			std::cin.clear();
		}
		std::string printed() const { return output.str(); }

	private:
		std::istringstream input;
		std::ostringstream output;
		std::streambuf* oldIn;
		std::streambuf* oldOut;
	};

	// Copies of the original two methods (no printing) to compare against the new calculator
	// Original without deposit
	double originalWithoutDeposit(double initialInvestment, double apy, int numYears)
	{
		double balance = initialInvestment;
		double monthlyInterest = (apy / 100) / 12;
		for (int year = 1; year <= numYears; year++) {
			for (int month = 1; month <= 12; month++) {
				balance += balance * monthlyInterest;
			}
		}
		return balance;
	}

	// Original with deposit
	double originalWithDeposit(double initialInvestment, double monthlyDeposit, double apy, int numYears)
	{
		double balance = initialInvestment;
		double monthlyInterest = (apy / 100) / 12;
		for (int year = 1; year <= numYears; year++) {
			for (int month = 1; month <= 12; month++) {
				balance += monthlyDeposit;
				balance += balance * monthlyInterest;
			}
		}
		return balance;
	}
}

bool runInvestmentTests()
{
	std::cout << "Running unit tests\n";
	bool allPassed = true;

	// Test 1: No monthly deposit, one year, 5% APY
	// Expected balance $1051.16, interest $51.16
	{
		InvestmentCalculator calculator(1000.0, 5.0, 1);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(0.0);

		allPassed &= check(results.size() == 1 && approxEqual(results[0].balance, 1051.16), "accuracy: year-end balance is $1051.16");
		allPassed &= check(results.size() == 1 && approxEqual(results[0].interestEarnedThisYear, 51.16), "accuracy: interest earned is $51.16");
		allPassed &= check(results.size() == 1 && results[0].year == 1, "accuracy: the record is labeled year 1");
	}

	// Test 2: $100 monthly deposit, five years, 5% APY
	// Expected year 5 balance $8112.30
	{
		InvestmentCalculator calculator(1000.0, 5.0, 5);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(100.0);

		allPassed &= check(results.size() == 5 && approxEqual(results[4].balance, 8112.30), "accuracy: with $100 deposits the year 5 balance is $8112.30");
	}

	// Test 3: 0% APY should show no growth
	{
		InvestmentCalculator calculator(1000.0, 0.0, 3);
		std::vector<YearRecord> results = calculator.calculateYearlyBalances(0.0);

		allPassed &= check(results.size() == 3 && approxEqual(results[2].balance, 1000.0), "zero values: 0% APY and no deposit leaves $1000.00 unchanged");
	}

	// Test 4: Compare to the original methods, 27 no deposit and 81 with deposit combinations
	{
		const double amounts[] = { 0.0, 1000.0, 25000.50 };
		const double deposits[] = { 0.0, 100.0, 250.75 };
		const double rates[] = { 0.0, 5.0, 12.5 };
		const int years[] = { 1, 10, 40 };

		// Last balance, -1 if no records came back
		auto finalBalance = [](const std::vector<YearRecord>& records) {
			return records.empty() ? -1.0 : records.back().balance;
			};

		// No deposit: 3 amounts x 3 rates x 3 periods = 27
		bool matchesNoDeposit = true;
		for (double amount : amounts) {
			for (double rate : rates) {
				for (int period : years) {
					InvestmentCalculator calculator(amount, rate, period);
					if (!approxEqual(finalBalance(calculator.calculateYearlyBalances(0.0)), originalWithoutDeposit(amount, rate, period), 1e-6)) {
						matchesNoDeposit = false;
					}
				}
			}
		}

		// With deposit: 3 amounts x 3 deposits x 3 rates x 3 periods = 81
		bool matchesWithDeposit = true;
		for (double amount : amounts) {
			for (double deposit : deposits) {
				for (double rate : rates) {
					for (int period : years) {
						InvestmentCalculator calculator(amount, rate, period);
						if (!approxEqual(finalBalance(calculator.calculateYearlyBalances(deposit)), originalWithDeposit(amount, deposit, rate, period), 1e-6)) {
							matchesWithDeposit = false;
						}
					}
				}
			}
		}

		allPassed &= check(matchesNoDeposit, "original behavior: no-deposit results match the original method in all 27 combinations");
		allPassed &= check(matchesWithDeposit, "original behavior: with-deposit results match the original method in all 81 combinations");
	}

	// Test 5: Unusual periods, 0 years and 50 years
	{
		InvestmentCalculator zeroYears(1000.0, 5.0, 0);
		allPassed &= check(zeroYears.calculateYearlyBalances(100.0).empty(), "unusual period: 0 years returns no records");

		InvestmentCalculator fiftyYears(1000.0, 5.0, 50);
		std::vector<YearRecord> results = fiftyYears.calculateYearlyBalances(100.0);
		allPassed &= check(results.size() == 50 && std::isfinite(results[49].balance) && results[49].balance > results[0].balance,
			"unusual period: 50 years returns 50 records with a finite, growing balance");
	}

	// Test 6: Calculator should not print anything
	{
		std::string printed;
		{
			FakeConsole console("");
			InvestmentCalculator calculator(1000.0, 5.0, 3);
			calculator.calculateYearlyBalances(100.0);
			printed = console.printed();
		}
		allPassed &= check(printed.empty(), "separation: the calculator prints nothing");
	}

	// Test 7: Bad input first, then good input
	{
		double apy = 0;
		int numYears = 0;
		char answer = 0;
		std::string printed;

		{ FakeConsole console("-5\n5\n"); apy = InputValidator::getValidatedDouble("APY: ", 0.0); printed = console.printed(); }
		allPassed &= check(approxEqual(apy, 5.0) && contains(printed, "at least"), "invalid input: a negative APY (-5) is rejected, then 5 is accepted");

		{ FakeConsole console("abc\n3\n"); apy = InputValidator::getValidatedDouble("APY: ", 0.0); }
		allPassed &= check(approxEqual(apy, 3.0), "invalid input: letters for the APY are rejected, then 3 is accepted");

		{ FakeConsole console("0\n1\n"); numYears = InputValidator::getValidatedInt("Years: ", 1); printed = console.printed(); }
		allPassed &= check(numYears == 1 && contains(printed, "at least"), "invalid input: 0 years is rejected, then 1 (the minimum) is accepted");

		{ FakeConsole console("Y\n"); answer = InputValidator::getValidatedYesNo("Again? "); }
		allPassed &= check(answer == 'y', "invalid input: a capital Y is accepted as yes");

		// Byte above 127 is a negative char, which is what broke tolower()
		{ FakeConsole console("\xE9\nn\n"); answer = InputValidator::getValidatedYesNo("Again? "); printed = console.printed(); }
		allPassed &= check(answer == 'n' && contains(printed, "'y' or 'n'"), "invalid input: a non-ASCII character is rejected without crashing");
	}

	std::cout << (allPassed ? "All tests passed\n\n" : "One or more tests failed\n\n");
	return allPassed;
}