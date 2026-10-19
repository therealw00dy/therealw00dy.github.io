/*
* Preston Woodrum
* Project 2
* Taking a series of user inputs and calculating how the account grows over years
* 08FEB2025
*/
#include <iostream>
#include <iomanip>
#include "Investment.h"

int main()
{
	char choice;

	do {
		double initialInvestment, apy, monthlyDeposit;
		int numYears;

		// Take inputs for investment numbers from user
		std::cout << "Initial Investment Amount: $";
		std::cin >> initialInvestment;
		std::cout << "Monthly Deposit: $";
		std::cin >> monthlyDeposit;
		std::cout << "Annual Interest: %";
		std::cin >> apy;
		std::cout << "Number of Years: ";
		std::cin >> numYears;

		Investment investment(initialInvestment, apy, numYears);

		// Calculate and display formatted balance without monthly deposit
		std::cout << "\nBalance and Interest Without Monthly Deposit:\n";
		std::cout << "===================================================================\n";
		std::cout << std::setw(10) << "Year" << std::setw(15) << "\tYear End Balance" << std::setw(20) << "\tYear End Earned Interest\n";
		std::cout << "===================================================================\n";

		investment.calculateBalanceWithoutMonthlyDeposit();

		// Calculate and displat formatted balance with monthly deposit
		std::cout << "\nBalance and Interest With Monthly Deposit:\n";
		std::cout << "===================================================================\n";
		std::cout << std::setw(10) << "Year" << std::setw(15) << "\tYear End Balance" << std::setw(20) << "\tYear End Earned Interest\n";
		std::cout << "===================================================================\n";

		investment.calculateBalanceWithMonthlyDeposit(monthlyDeposit);

		// Ask if user wants to continue testing information
		std::cout << "Would you like to change any information?";
		std::cin >> choice;
	
	} while (choice == 'y');


	return 0;
}