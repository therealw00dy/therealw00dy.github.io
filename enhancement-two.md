---
title: Enhancement Two
---

[Home](index.html)

# Enhancement Two: Algorithms and Data Structures

[Enhancement Two Files](https://github.com/therealw00dy/therealw00dy.github.io/tree/main/Enhancement%20Two)

## Narrative

### Briefly describe the artifact. What is it? When was it created?

This artifact is a course planner created for CS300 Analysis and Design around August 2025. It reads course information such as title, number, and any prerequisites from a CSV into a hash table, and the user gets to navigate a menu to get the information they are looking for.

### Justify the inclusion of the artifact in your ePortfolio.

I chose this artifact for the Algorithms and Data Structures enhancement because the hash table was the focus of the original assignment, but since it was my first time doing it, I found several flaws to point out. The hash function added up character codes of a course number, so any two courses with the same characters regardless of position would land in the same bucket. The table also stayed at 20 buckets no matter how many courses were loaded, and the prerequisites were just made of plain text, so the program could not actually check whether they made sense or not.

I created a better hash function, so now it reads the position of characters and will not put two different courses with the same numbers in different locations together. I added dynamic resizing, since it was locked to 20 for the original assignment, which, when using it in real applications, would not hold up. I also built a directed graph as an adjacency list, using depth-first search to spot any circular relations and Kahn’s algorithm to produce a topological sort.

I also addressed some security flaws. The original menu never checked if the input read had failed, which meant it could be put into a broken state and have unpredictable behavior; now it clears the input and asks again. The original also accepted any prerequisite data without question, which could include loops for courses that were impossible to take, so now the program can notice that issue and reject the data.

I used actual courses and prerequisites from the SNHU Computer Science degree catalog to have real-life data, making the CSV myself from that information.

### Did you meet the course outcomes you planned to meet with this enhancement in Module One? Do you have any updates to your outcome-coverage plans?

I did meet the outcomes that I had planned, such as Outcome 3 (designing and evaluating while managing trade-offs) and Outcome 4 (using well-founded techniques to deliver a working solution). Outcome 3 shows in the resizing decision, with a lower factor threshold leading to fewer collisions but more memory and more frequent rehashing. Outcome 4 appears in the cycle detection and topological sort, which gave a useful feature that the program did not previously have. Outcome 5 was also hit due to the input check and validation of prerequisite data. While there is largely increased functionality, I made the CSV by hand and did not code for every case, such as courses being a prerequisite but not in the CSV as their own course; this can easily be fixed with more time and more work, showing an area for more improvement.

### Reflect on the process of enhancing and modifying the artifact. What did you learn as you were creating it and improving it? What challenges did you face?

One useful thing to learn was why resizing must rehash every entry instead of just copying a table over. The course’s bucket depends on table size, so changing the size means that every course needs to be placed again.

My main challenge was understanding the graphing algorithms well enough to write them, but I did use information from online resources (cited in the code) to help me understand the data structures, since I had not worked with all of them fully yet. While reworking the program, I merged the courses ready to process and the final order list into one variable (Kahn’s algorithm) which had broken the sort. After fixing it, it made more sense why the algorithm needs both of the variables that are included in the program, so one can be used on a finished result and the other can be used for a work in progress.
