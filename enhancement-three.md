---
title: Enhancement Three
---

[Home](index.html)

# Enhancement Three: Databases

[Enhancement Three Files](https://github.com/therealw00dy/therealw00dy.github.io/tree/main/Enhancement%20Three)

## [Narrative](https://github.com/therealw00dy/therealw00dy.github.io/blob/main/Enhancement%20Three/Enhancement%203.docx)

### Briefly describe the artifact. What is it? When was it created?

  This artifact is a course planner created for CS300 Analysis and Design around August 2025. It reads course information such as title, number, and any prerequisites from a CSV into a hash table, and the user gets to navigate a menu to get the information they are looking for.

### Justify the inclusion of the artifact in your ePortfolio.

  I chose this artifact for the database category because the data is relational. A course can have multiple prerequisites, and every prerequisite is also a course (or supposed to be).

  My enhancement replaced the regular CSV loading with a CSV import into a SQLite database that was made from two tables. Course holds one row per course, and Prerequisite holds one row per “Course X requires course Y,” which models the relationships better than a plain comma-separated list.

  I built the program to support all of the CRUD operations, allowing the user full control over the database list. Using SQLite allowed me to use a LEFT JOIN and ORDER BY so that SQLite would do the sorting in my read function. Updates allow the user to change a title, and the delete function is blocked if other courses depend on that course as a prerequisite. The hash table that was enhanced in Enhancement 2 is still in the program as a fast in-memory copy that is rebuilt from the database information after each change, allowing the program to keep its speed.

### Did you meet the course outcomes you planned to meet with this enhancement in Module One? Do you have any updates to your outcome-coverage plans?

  Yes, my plan targeted outcome 3 (trade-offs), outcome 4 (using well-founded techniques), and outcome 5 (security mindset). Outcome 3 is apparent in my schema design, where I normalized prerequisites into their own table, which trades a simpler structure for joins and more rigid integrity. Outcome 4 is shown in the constraints, index, transactions, and use of a real database, all utilizing well-documented and implemented techniques. Outcome 5 showed up more in Enhancement 2, but also in the parameterized queries assisted in this one.

  I have two updates to my plan, the foreign keys and index. The foreign key and transaction outcome are what add to outcome 5, since they help protect data integrity, and parameterized queries protect against injection. The second, I planned an index on the course number, but using a composite primary key makes that lookup quick, so I used the index on the prerequisite lookup since it allows the reverse lookup to be fast.

  There are some limits to the program, especially when it comes to the addition or updating of courses and prerequisites. The database makes sure the prerequisites exist, but it does not prevent the (unlikely) possibility of circular prerequisites; it is still handled by the graph from Enhancement 2. Another large issue is that prerequisites cannot be added after a course is created; you would have to delete the course, then re-add the course with the prerequisites listed.

### Reflect on the process of enhancing and modifying the artifact. What did you learn as you were creating it and improving it? What challenges did you face?

  I learned some positive things while creating and improving this program. This was my first time deep diving into SQLite, and I applied what I knew from other database formats like NoSQL and MySQL. Because SQL is only seen as text by the C++ compiler, typos in the queries were not caught until the program ran. I had a few times when I misspelled a word, name, or just missed a parenthesis, but I was not informed until I ran the program and it just failed. While this was my big learning point, that also caused it to be my biggest challenge. I have worked with other database formats like NoSQL and MySQL, but I had very limited time with SQLite before implementing it here and reading through pages of people’s iterations and documentation online.
