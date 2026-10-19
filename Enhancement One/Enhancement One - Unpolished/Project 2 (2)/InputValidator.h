#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include <string>

// Original main() read values straight from cin without any validation checks, leaving vulnerable fail states
// InputValidator incorporates checks for these issues

class InputValidator
{
public:
	// Prompts until user enters value that is within our requirements
	static double getValidatedDouble(const std::string& prompt, double minValue);
	static int getValidatedInt(const std::string& prompt, int minValue);
	static char getValidatedYesNo(const std::string& prompt);
};

#endif