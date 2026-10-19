/*
* Preston Woodrum
* Project 2
* Taking a series of user inputs and calculating how the account grows over years
* 08FEB2025
*/
#include <iostream>
#include <cctype>
#include "InvestmentTests.h"
#include "InvestmentCalculator.h"
#include "InvestmentReport.h"
#include "InputValidator.h"

int main()
{	
	// Run unit test once at startup so you can see calculation logic is functional before inputting numbers
	runInvestmentTests();

	char choice;

	do {
		double initialInvestment = InputValidator::getValidatedDouble("Initial Investment Amount: $", 0.0);
		double monthlyDeposit = InputValidator::getValidatedDouble("Monthly Deposit: $", 0.0);
		double apy = InputValidator::getValidatedDouble("Annual Interest: %", 0.0);
		int numYears = InputValidator::getValidatedInt("Number of Years: ", 1);

		InvestmentCalculator calculator(initialInvestment, apy, numYears);

		std::vector<YearRecord> withoutDeposit = calculator.calculateYearlyBalances(0.0);
		InvestmentReport::printYearlyTable("Balance and Interest Without Monthly Deposit", withoutDeposit);

		std::vector<YearRecord> withDeposit = calculator.calculateYearlyBalances(monthlyDeposit);
		InvestmentReport::printYearlyTable("Balance and Interest With Monthly Deposit", withDeposit);

		// Ask if want to continue
		choice = InputValidator::getValidatedYesNo("\nWould you like to change any information (y/n): ");
	
	} while (choice == 'y');


	return 0;
}