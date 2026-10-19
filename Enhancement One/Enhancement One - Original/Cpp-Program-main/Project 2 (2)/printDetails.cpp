#include "printDetails.h"
#include <iostream>
#include <iomanip>

void printDetails::printDetails2(int year, double balance, double interestEarnedThisYear)
{
    // Each line is a print section for a variable
    std::cout << std::setw(10) << year // Print for the year values and partially formatted
        << std::setw(15) << std::fixed << std::setprecision(2) << balance // Print for the balance values and partially formatted
        << std::setw(20) << std::fixed << std::setprecision(2) << interestEarnedThisYear // Print for the interest values and partially formatted
        << std::endl;
}
