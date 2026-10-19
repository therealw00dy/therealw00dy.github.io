#include "Investment.h"
#include "printDetails.h"
#include <iomanip>

Investment::Investment(double initialInvestment, double apy, int numYears)
{
	this->initialInvestment = initialInvestment;
	this->apy = apy;
	this->numYears = numYears;
}

// Calculations to be called for inputs given not using the monthly deposit input
double Investment::calculateBalanceWithoutMonthlyDeposit()
{
	double balance = initialInvestment;
	double monthlyInterest = (apy / 100) / 12; // Interest calculation to determine monthly compounded interest

    // Nested for loops to run through each cycle of a year and the months and calculate balance
    for (int year = 1; year <= numYears; year++) {

        double interestEarnedThisYear = 0;

        for (int month = 1; month <= 12; month++) {
            double interestEarnedThisMonth = balance * monthlyInterest;
            balance += interestEarnedThisMonth;
            interestEarnedThisYear += interestEarnedThisMonth;
        }

        printDetails::printDetails2(year, balance, interestEarnedThisYear); // Print function made to attempt formatting the calculated items
    }
    return balance;
}

// Calculations to be called for inputs given using the monthly deposit input
double Investment::calculateBalanceWithMonthlyDeposit(double monthlyDeposit)
{
    double balance = initialInvestment;
    double monthlyInterest = (apy / 100) / 12; // Interest calculation to determine monthly compounded interest

    // Nested for loops to run through each cycle of a year and the months and calculate balance
    for (int year = 1; year <= numYears; year++) {

        double interestEarnedThisYear = 0;

        for (int month = 1; month <= 12; month++) {
            balance += monthlyDeposit;
            double interestEarnedThisMonth = balance * monthlyInterest;
            balance += interestEarnedThisMonth;
            interestEarnedThisYear += interestEarnedThisMonth;
        }

        printDetails::printDetails2(year, balance, interestEarnedThisYear); // Print function made to attempt formatting the calculated items
    }

    return balance;
}