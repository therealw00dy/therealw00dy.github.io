---
title: Enhancement One
---

[Home](index.html)

# Enhancement One: Software Design and Engineering

[Enhancement One Files](https://github.com/therealw00dy/therealw00dy.github.io/blob/main/Enhancement%20One)

## Narrative

### Briefly describe the artifact. What is it? When was it created?

The artifact is an investment calculator made in C++. It was created for CS 210: Programming Languages in February 2025. This takes the user’s initial investment, monthly deposit, annual percentage yield (APY), and number of years, then calculates and prints a yearly table of the balance on the account and the interest earned, showing both with and without a monthly deposit.

### Justify the inclusion of the artifact in your ePortfolio.

 I selected this artifact for the Software Design and Engineering category since it had clear and flexible structural problems that let me demonstrate my progression as a software developer over nearly two years. The original program was condensed into a few files, combining multiple parts and functions together when it would be a better practice to separate for testing, troubleshooting, and expandability purposes. One of the bigger changes in the code is the inclusion of defensive coding. Throughout the past two years of classes, defensive coding has been pushed harder and is now something that is noticeable to me when it used to not be.

The enhancement restructured the program into four components: the calculation logic, the print logic, the validation logic and a testing suite to run a test on the calculations when you run the program. Together these show object-oriented decomposition, separation of concerns, and defensive programming.

### Did you meet the course outcomes you planned to meet with this enhancement in Module One? Do you have any updates to your outcome-coverage plans?

Yes, my plan targeted Outcome 3 (design trade-offs), outcome 4 (well-founded software engineering techniques), and outcome 5 (a security mindset with input validation) and I think the finished enhancement hits all three. My outcome 5 went a little deeper than I expected since it was one of my first times learning about string validations, I found that tolower() has a bug with undefined behavior in C++ when using non ASCII keys. I learned of a fix for that by casting an unsigned char before the tolower(). This extra bit was not in my original plan, but upon learning this issue, it was a cool topic to look up and learn something new about.

### Reflect on the process of enhancing and modifying the artifact. What did you learn as you were creating it and improving it? What challenges did you face?

One of the biggest lessons found in this assignment is that refactoring code is not just moving it around, it has to be verified through created tests and actually usage tests. I compiled and ran the program against different cases I could think of, checking things like capital letters or incorrect characters, which is where I found the issue in tolower(). This was the most insightful part of the enhancement, learning something new that was completely unexpected and sent me on a journey to find out something new about a system I have more vaguely implemented before.
