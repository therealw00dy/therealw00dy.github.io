#ifndef INVESTMENT_H
#define INVESTMENT_H

// Header file to declare calculation of investment calculations
class Investment
{
public:

	Investment(double initialInvestment, double apy, int numYears);

	double calculateBalanceWithoutMonthlyDeposit();
	double calculateBalanceWithMonthlyDeposit(double monthlyDeposit);

private:
	double initialInvestment, apy;
	int numYears;

};

#endif // INVESTMENT_H