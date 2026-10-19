#include "InputValidator.h"
#include <iostream>
#include <limits>
#include <cctype>

double InputValidator::getValidatedDouble(const std::string& prompt, double minValue)
{
	double value;
	while (true) {
		std::cout << prompt;
		std::cin >> value;

		if (std::cin.fail()) {
			std::cin.clear(); // Reset fail state
			std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n'); // Discard bad input
			std::cout << "Please enter valid number.\n";
			continue;
		}

		if (value < minValue) {
			std::cout << "Please enter a value of at least " << minValue << "\n";
			continue;
		}

		return value;
	}
}

int InputValidator::getValidatedInt(const std::string& prompt, int minValue)
{
	int value;
	while (true) {
		std::cout << prompt;
		std::cin >> value;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
			std::cout << "Please enter a whole number.\n";
			continue;
		}

		if (value < minValue) {
			std::cout << "Please enter a value of at least " << minValue << "\n";
			continue;
		}

		return value;
	}
}

char InputValidator::getValidatedYesNo(const std::string& prompt)
{
	char choice;
	while (true) {
		std::cout << prompt;
		std::cin >> choice;

		// tolower is only defined for values representable as unsigned charm using unsigned char first avoids undefined behavior

		choice = static_cast<char>(std::tolower(static_cast<unsigned char>(choice)));

		if (choice == 'y' || choice == 'n') {
			return choice;
		}

		std::cout << "Enter y or n\n";
	}
}