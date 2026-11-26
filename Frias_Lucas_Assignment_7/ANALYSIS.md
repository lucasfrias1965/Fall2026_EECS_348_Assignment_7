# EECS 348 Assginment 7

Happy holiday season, Mr. Zhe. Thanksgiving should have passed by the time you read this, but I'm excited for this holiday and for Christmas, it's my favorite time of the year. And there's a lot more than just that to be thankful for. I'm happy to get a brand new and exciting Assignment 7 that is **not** about Sudoku. 

Today, we'll be telling AI to make SQL prompts in C++ and judge their validity. While, of course, there is some time complexity that can be analyzed within the C++ code, we'll also be analyzing the time and space complexity of the SQL commands andwhat they do.

Let's get into it:

# LLMs And Prompt

I chose the age old ChatGPT and Anthropic for this. Nothing fancy, I'm curious about the changees that continue to happen with their newer models.

To access ChatGPT from OpenAI, go to https://chatgpt.com and then put in the prompt. As of 2026 November, the default is GPT-6.

Same with Claude, go to Clade.AI, and log in with your Google, Apple, or email address (in case you still have an AOL email address) and then you can talk with Claude 5.5, or Sonnet. 

P.S, apparently, the small country of Anguilla has the domain .AI, and now currently is 10% of the gross domestic product of the territory, and is one of the most expensive domain name suffixes of 2026.

Here's the prompt I used for both GPT-6 and Sonnet 5.5:

```
 Generate C++ code using objects, not functions, to implement
the following requirements (Note: You may use either ChatGPT or CoPilot as one
of your choices, but not both. They are essentially the same GenAI.):
• Creates the five MySQL tables given in the Assignment7SQLTables.zip
file.
• These tables may not be the same ones you are using for Lab 12.
• To make sure these tables don’t conflict with those you are using for
Lab 12, add a 7 to the end of the table name, e.g., Course7
• Performs the following SQL queries and outputs the results:
1. Retrieve all student information for those whose major is 'IS' from the
Student table.
2. Retrieve the student numbers (StdNo) of students who have enrolled in
more than two courses from the Enrollment table.
3. List the first and last names of all professors from the Faculty table
who have been employed for more than 15 years as of 2025, including
those hired in 2010 or earlier.
4. Retrieve all course numbers from the Offering table that were offered
in Summer 2020.
5. List the first and last names of all professors from the Faculty table
who live in Zip code 98114.
6. Retrieve the second-highest GPA from the Student table.
7. Retrieve the first and last names of individuals who appear in both the
Student and Faculty tables.
8. Retrieve the student number (StdNo), first and last names, and the
number of selected courses from the Student and Enrollment tables.
9. Retrieve the first and last names of professors, along with their
salaries, for the top three highest-paid professors from the Faculty
table.
10. Insert a junior CS student named ALICE SMITH from TOPEKA, KS,
Zip 66610-1234, with student number 888-88-8888 and a GPA of 3.85
into the Student table. Then display the updated table using SELECT *
FROM Student.
11. Retrieve all student information from the Student table for students
who do not have any enrollment records in the Enrollment table.
12. Student BOB NORBERT has moved to Overland Park, Kansas (KS).
Update the StdCity and StdZip fields in the Student table to reflect
this change, and then display the updated table using SELECT * FROM
Student.
• The output for each query should display the query number, the query
given above, and the results. For example:
1. Retrieve all students majoring in 'IS'.
<results of Query 1 in table format>
2. Find the names of students who have enrolled in more than two
courses <results of QUery 2 in table format> Etc.

Here are all the SQL files you have:
$ ls -a *.SQL
course.SQL  enrollment.SQL  faculty.SQL  offering.SQL  student.SQL

You will be using MariaDB to connect to this. MariaDB is already installed. Here is the command to connect, with the password redacted. Please make a macro to define this password too:

mysql -h mysql.eecs.ku.edu -u 348f26_l382f965 -***********


You will have a zipped version of the tables for your pondering.

Good luck!
```

Both Sonnet and GPT-6 had access to the databases as I included the zipped version. I didn't provide my password out of security, and instead made them define it as a macro.

# Correctness

Correctness in this case is verifying the authenticity of the output towards the SQL database. I have to make my own queries to verify this, because using an AI to query it is a bit of a chicken and an egg problem.

Even worse, though, is the amount of work I had to spend making both AI outputs run first go. I'll chronicle it here:


## GPT 6

Unlike Claude's, firstly we were missing the mariadb header file:

```
chatgpt.cpp:17:10: fatal error: mariadb/conncpp.hpp: No such file or directory
   17 | #include <mariadb/conncpp.hpp>
      |          ^~~~~~~~~~~~~~~~~~~~~
compilation terminated
```

Previously, I used the KU cycle server as an enviornment. But using it in this case to install and run the MariaDB connector would be very hard without sudo. Instead, I opted to use WSL on my family's home computer as I was back home for the Thanksgiving break.

Installing it was still a pain. One of the many evil heads of the hydra known as C++ is that there is no package manager, you must figure it out yourself. I downloaded the latest Debian connector version of MariaDB, and then copied into my directory on my Ubuntu WSL enviornment. I then added it to my path after tar -xvzf ing it:

```
sudo cp -a include/. /usr/local/include/
sudo cp -a lib/. /usr/local/lib/
sudo cp -a share/. /usr/local/share/
sudo ldconfig
```

This added it to my path with Bash as my shell and using Debian 13 (Ubuntu). However, after linking correctly, there were still errors:

```
a7.cpp: In member function ‘void Assignment7::printResult(sql::ResultSet*)’:
a7.cpp:102:31: error: no matching function for call to ‘std::vector<std::__cxx11::basic_string<char> >::push_back(sql::SQLString)’
  102 |             headings.push_back(metadata->getColumnLabel(column));
      |             ~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In file included from /usr/include/c++/13/vector:66,
                 from /usr/local/include/mariadb/conncpp/CArray.hpp:27,
                 from /usr/local/include/mariadb/conncpp/jdbccompat.hpp:28,
                 from /usr/local/include/mariadb/conncpp/Connection.hpp:28,
                 from /usr/local/include/mariadb/conncpp/Driver.hpp:26,
                 from /usr/local/include/mariadb/conncpp.hpp:24,
                 from a7.cpp:17:
/usr/include/c++/13/bits/stl_vector.h:1281:7: note: candidate: ‘void std::vector<_Tp, _Alloc>::push_back(const value_type&) [with _Tp = std::__cxx11::basic_string<char>; _Alloc = std::allocator<std::__cxx11::basic_string<char> >; value_type = std::__cxx11::basic_string<char>]’
 1281 |       push_back(const value_type& __x)
      |       ^~~~~~~~~
/usr/include/c++/13/bits/stl_vector.h:1281:35: note:   no known conversion for argument 1 from ‘sql::SQLString’ to ‘const std::vector<std::__cxx11::basic_string<char> >::value_type&’ {aka ‘const std::__cxx11::basic_string<char>&’}
 1281 |       push_back(const value_type& __x)
      |                 ~~~~~~~~~~~~~~~~~~^~~
/usr/include/c++/13/bits/stl_vector.h:1298:7: note: candidate: ‘void std::vector<_Tp, _Alloc>::push_back(value_type&&) [with _Tp = std::__cxx11::basic_string<char>; _Alloc = std::allocator<std::__cxx11::basic_string<char> >; value_type = std::__cxx11::basic_string<char>]’
 1298 |       push_back(value_type&& __x)
      |       ^~~~~~~~~
/usr/include/c++/13/bits/stl_vector.h:1298:30: note:   no known conversion for argument 1 from ‘sql::SQLString’ to ‘std::vector<std::__cxx11::basic_string<char> >::value_type&&’ {aka ‘std::__cxx11::basic_string<char>&&’}
 1298 |       push_back(value_type&& __x)
      |                 ~~~~~~~~~~~~~^~~
a7.cpp:112:30: error: no matching function for call to ‘std::vector<std::__cxx11::basic_string<char> >::push_back(sql::SQLString)’
  112 |                 row.push_back(rows->isNull(column)
      |                 ~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~
  113 |                                   ? "NULL"
      |                                   ~~~~~~~~
  114 |                                   : rows->getString(column));
      |                                   ~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/stl_vector.h:1281:7: note: candidate: ‘void std::vector<_Tp, _Alloc>::push_back(const value_type&) [with _Tp = std::__cxx11::basic_string<char>; _Alloc = std::allocator<std::__cxx11::basic_string<char> >; value_type = std::__cxx11::basic_string<char>]’
 1281 |       push_back(const value_type& __x)
      |       ^~~~~~~~~
/usr/include/c++/13/bits/stl_vector.h:1281:35: note:   no known conversion for argument 1 from ‘sql::SQLString’ to ‘const std::vector<std::__cxx11::basic_string<char> >::value_type&’ {aka ‘const std::__cxx11::basic_string<char>&’}
 1281 |       push_back(const value_type& __x)
      |                 ~~~~~~~~~~~~~~~~~~^~~
/usr/include/c++/13/bits/stl_vector.h:1298:7: note: candidate: ‘void std::vector<_Tp, _Alloc>::push_back(value_type&&) [with _Tp = std::__cxx11::basic_string<char>; _Alloc = std::allocator<std::__cxx11::basic_string<char> >; value_type = std::__cxx11::basic_string<char>]’
 1298 |       push_back(value_type&& __x)
      |       ^~~~~~~~~
/usr/include/c++/13/bits/stl_vector.h:1298:30: note:   no known conversion for argument 1 from ‘sql::SQLString’ to ‘std::vector<std::__cxx11::basic_string<char> >::value_type&&’ {aka ‘std::__cxx11::basic_string<char>&&’}
 1298 |       push_back(value_type&& __x)
      |                 ~~~~~~~~~~~~~^~~
a7.cpp: In function ‘int main()’:
a7.cpp:349:60: error: passing ‘const sql::SQLException’ as ‘this’ argument discards qualifiers [-fpermissive]
  349 |                   << "\nError code: " << error.getErrorCode()
      |                                          ~~~~~~~~~~~~~~~~~~^~
In file included from /usr/local/include/mariadb/conncpp.hpp:39:
/usr/local/include/mariadb/conncpp/Exception.hpp:52:11: note:   in call to ‘int32_t sql::SQLException::getErrorCode()’
   52 |   int32_t getErrorCode() { return ErrorCode; }
      |           ^~~~~~~~~~~~
a7.cpp:350:58: error: passing ‘const sql::SQLException’ as ‘this’ argument discards qualifiers [-fpermissive]
  350 |                   << "\nSQL state: " << error.getSQLState() << '\n';
      |                                         ~~~~~~~~~~~~~~~~~^~
/usr/local/include/mariadb/conncpp/Exception.hpp:50:13: note:   in call to ‘sql::SQLString sql::SQLException::getSQLState()’
   50 |   SQLString getSQLState() { return SqlState.c_str(); }
      |             ^~~~~~~~~~~
```

The errors were silly but understandable: MariaDB uses its own custom string that isn't std::string and can't be promoted to it, causing me to have to correct these lines:

```
 (Line 102, chatgpt.cpp) headings.push_back(metadata->getColumnLabel(column));

(Line 112, chatgpt.cpp)
row.push_back(rows->isNull(column)
                                  ? "NULL"
                                  : rows->getString(column));
```

to this:

```
headings.push_back(metadata->getColumnLabel(column).c_str()); 
```
Fix here was to make it convert itself explicitly to cstr. Same type of explicit casting done here:

```
row.push_back(rows->isNull(column)
    ? std::string("NULL")
    : std::string(rows->getString(column).c_str()));
```

This made sure that everything was a string. Additionally, there was an error with this being const. The compilier advised me to not make it const for some internal library reason, so I did it and it worked and that's pretty much all I know.

```
catch (sql::SQLException& error)
```

Now, we move to the defintely more fun linking errors I had to go through:

```
 g++ -std=c++17 a7.cpp -o a7
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `main':
a7.cpp:(.text+0x15d): undefined reference to `sql::operator<<(std::ostream&, sql::SQLString const&)'
/usr/bin/ld: a7.cpp:(.text+0x176): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text+0x1fe): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `sql::SQLString::SQLString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)':
a7.cpp:(.text._ZN3sql9SQLStringC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN3sql9SQLStringC5ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x41): undefined reference to `sql::SQLString::SQLString(char const*, unsigned long)'
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `sql::SQLException::getSQLState()':
a7.cpp:(.text._ZN3sql12SQLException11getSQLStateEv[_ZN3sql12SQLException11getSQLStateEv]+0x20): undefined reference to `sql::SQLString::c_str() const'
/usr/bin/ld: a7.cpp:(.text._ZN3sql12SQLException11getSQLStateEv[_ZN3sql12SQLException11getSQLStateEv]+0x32): undefined reference to `sql::SQLString::SQLString(char const*)'
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `Assignment7::createAndLoadTable(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)':
a7.cpp:(.text._ZN11Assignment718createAndLoadTableERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_[_ZN11Assignment718createAndLoadTableERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_]+0x148): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment718createAndLoadTableERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_[_ZN11Assignment718createAndLoadTableERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_]+0x233): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `Assignment7::printResult(sql::ResultSet*)':
a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x140): undefined reference to `sql::SQLString::c_str() const'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x187): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x31b): undefined reference to `sql::SQLString::c_str() const'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x367): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x7b9): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment711printResultEPN3sql9ResultSetE[_ZN11Assignment711printResultEPN3sql9ResultSetE]+0x807): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `Assignment7::runQuery(int, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)':
a7.cpp:(.text._ZN11Assignment78runQueryEiRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_[_ZN11Assignment78runQueryEiRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_]+0x116): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment78runQueryEiRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_[_ZN11Assignment78runQueryEiRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_]+0x146): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: /tmp/ccXN3m0O.o:a7.cpp:(.text._ZN11Assignment79runUpdateERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN11Assignment79runUpdateERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x98): more undefined references to `sql::SQLString::~SQLString()' follow
/usr/bin/ld: /tmp/ccXN3m0O.o: in function `Assignment7::Assignment7()':
a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x78): undefined reference to `sql::mariadb::get_driver_instance()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0xbc): undefined reference to `sql::Properties::Properties()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0xd2): undefined reference to `sql::SQLString::SQLString(char const*)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0xe5): undefined reference to `sql::Properties::operator[](sql::SQLString const&)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0xfa): undefined reference to `sql::SQLString::operator=(char const*)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x106): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x11c): undefined reference to `sql::SQLString::SQLString(char const*)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x12f): undefined reference to `sql::Properties::operator[](sql::SQLString const&)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x144): undefined reference to `sql::SQLString::operator=(char const*)'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x150): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x1ab): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x1db): undefined reference to `sql::Properties::~Properties()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x242): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x257): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x26c): undefined reference to `sql::SQLString::~SQLString()'
/usr/bin/ld: a7.cpp:(.text._ZN11Assignment7C2Ev[_ZN11Assignment7C5Ev]+0x281): undefined reference to `sql::Properties::~Properties()'
/usr/bin/ld: /tmp/ccXN3m0O.o:(.data.rel.local.DW.ref._ZTIN3sql12SQLExceptionE[DW.ref._ZTIN3sql12SQLExceptionE]+0x0): undefined reference to `typeinfo for sql::SQLException'
collect2: error: ld returned 1 exit status
lucasfrias@FriasFamily:~/Code/SoftEng$
```

Super fun! It linked some parts wrong. This is kind of a problem with doing third party libraries in C++, but who needs a package manager when we can do it ourself, right?

I used Duck.AI using GPT 5 as a backend and quickly learned that I stupidly assumed my computer was amd64 instead of x86, which I should have known by just checking my uname. This time I reinstalled the correct x86 architecture for my Debian 13 distribution.

I then proceeded to compile the entire library and add it to g++. Duck.AI was very useful for this:

lucasfrias@FriasFamily:~/Code/SoftEng$ LIBDIR="$HOME/Code/SoftEng/mariadb-connector-cpp-1.1.9-deb13/lib/mariadb"

```
g++ -std=c++17 a7.cpp -o a7 \
  -L"$LIBDIR" \
  -Wl,-rpath,"$LIBDIR" \
  -lmariadbcpp
```

Now, we finally have a compilied program. It doesn't do anything without the SQL files so I had to throw those in too. I also had to fill in the password with the correct password as i defined in my instructions to be a macro. However, there was also this funny line

```
// Overland Park has multiple ZIP codes. Replace this placeholder with
// BOB NORBERT'S actual new ZIP code before running the program.
#define BOB_NEW_ZIP "REPLACE_WITH_BOBS_NEW_ZIP```

Just funny that AI checked the ZIP code but couldn't understand the string passing for Maria DB. I included 66214 as my zip since that's mine where I live in OP.

Recompiling with the new macro changes, we hit another fun error, this time with KU:

```
MariaDB error: Could not connect to address=(host=mysql.eecs.ku.edu)(port=3306)(type=master) : Can't connect to server on 'mysql.eecs.ku.edu' (110)
Error code: 2002
SQL state: HY000
```

Yikes! What on the world could be happening?

Well, as it turns out, KU IT closes off connections that are not within the network towards their databases, including this Assignment 7 one. 

This took me to download Cisco's VPN Service for KU, just to realize that it didn't work like that.

I then decided to install and build it MariaDB's connector from source on KU's cycle server to try to see if that works. It did, except that I had to also install Cmake without sudo, which took a long while. Then, after all of that, I had to debug the password prompt including stray characters causing MariaDB not to work, and fitting the SQL files specifically where they need to go in the directory. Eventually, I got it to run.

```
l382f965@cycle4:~/Code/SoftwareEng/Fall2026_EECS_348_Assignment_7/Frias_Lucas_Assignment_7/chatgpt$ ./out
Connected to MariaDB.

Creating and loading the five Assignment 7 tables...
Created and loaded Student7.
Created and loaded Faculty7.
Created and loaded Course7.
Created and loaded Enrollment7.
Created and loaded Offering7.

1. Retrieve all student information for those whose major is 'IS'.
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
6 row(s) returned.

2. Retrieve the student numbers (StdNo) of students who have enrolled in more than two courses.
+-------------+
| StdNo       |
+-------------+
| 123-45-6789 |
| 124-56-7890 |
| 234-56-7890 |
| 345-67-8901 |
| 456-78-9012 |
| 567-89-0123 |
| 678-90-1234 |
| 890-12-3456 |
| 901-23-4567 |
+-------------+
9 row(s) returned.

3. List the first and last names of all professors from the Faculty table who have been employed for more than 15 years as of 2025, including those hired in 2010 or earlier.
+--------------+-------------+
| FacFirstName | FacLastName |
+--------------+-------------+
| LEONARD      | VINCE       |
| VICTORIA     | EMMANUEL    |
| LEONARD      | FIBON       |
| NICKI        | MACON       |
+--------------+-------------+
4 row(s) returned.

4. Retrieve all course numbers from the Offering table that were offered in Summer 2020.
+----------+
| CourseNo |
+----------+
| IS320    |
+----------+
1 row(s) returned.

5. List the first and last names of all professors from the Faculty table who live in Zip code 98114.
+--------------+-------------+
| FacFirstName | FacLastName |
+--------------+-------------+
| CRISTOPHER   | COLAN       |
| JULIA        | MILLS       |
+--------------+-------------+
2 row(s) returned.

6. Retrieve the second-highest GPA from the Student table.
+--------+
| StdGPA |
+--------+
| 3.80   |
+--------+
1 row(s) returned.

7. Retrieve the first and last names of individuals who appear in both the Student and Faculty tables.
+------------+----------+
| FirstName  | LastName |
+------------+----------+
| CRISTOPHER | COLAN    |
+------------+----------+
1 row(s) returned.

8. Retrieve the student number (StdNo), first and last names, and the number of selected courses from the Student and Enrollment tables.
+-------------+--------------+-------------+-----------------+
| StdNo       | StdFirstName | StdLastName | SelectedCourses |
+-------------+--------------+-------------+-----------------+
| 123-45-6789 | HOMER        | WELLS       | 5               |
| 124-56-7890 | BOB          | NORBERT     | 4               |
| 234-56-7890 | CANDY        | KENDALL     | 4               |
| 345-67-8901 | WALLY        | KENDALL     | 3               |
| 456-78-9012 | JOE          | ESTRADA     | 3               |
| 567-89-0123 | MARIAH       | DODGE       | 4               |
| 678-90-1234 | TESS         | DODGE       | 3               |
| 789-01-2345 | ROBERTO      | MORALES     | 2               |
| 876-54-3210 | CRISTOPHER   | COLAN       | 2               |
| 890-12-3456 | LUKE         | BRAZZI      | 3               |
| 901-23-4567 | WILLIAM      | PILGRIM     | 5               |
+-------------+--------------+-------------+-----------------+
11 row(s) returned.

9. Retrieve the first and last names of professors, along with their salaries, for the top three highest-paid professors from the Faculty table.
+--------------+-------------+-----------+
| FacFirstName | FacLastName | FacSalary |
+--------------+-------------+-----------+
| VICTORIA     | EMMANUEL    | 120000.00 |
| JULIA        | MILLS       | 75000.00  |
| LEONARD      | FIBON       | 70000.00  |
+--------------+-------------+-----------+
3 row(s) returned.

10. Insert a junior CS student named ALICE SMITH from TOPEKA, KS, Zip 66610-1234, with student number 888-88-8888 and a GPA of 3.85 into the Student table. Then display the updated table using SELECT * FROM Student.

10. Updated Student7 table:
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 124-56-7890 | BOB          | NORBERT     | BOTHELL | WA       | 98011-2121 | FIN      | JR       | 2.70   |
| 234-56-7890 | CANDY        | KENDALL     | TACOMA  | WA       | 99042-3321 | ACCT     | JR       | 3.50   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 456-78-9012 | JOE          | ESTRADA     | SEATTLE | WA       | 98121-2333 | FIN      | SR       | 3.20   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 678-90-1234 | TESS         | DODGE       | REDMOND | WA       | 98116-2344 | ACCT     | SO       | 3.30   |
| 789-01-2345 | ROBERTO      | MORALES     | SEATTLE | WA       | 98121-2212 | FIN      | JR       | 2.50   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 888-88-8888 | ALICE        | SMITH       | TOPEKA  | KS       | 66610-1234 | CS       | JR       | 3.85   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
12 row(s) returned.

11. Retrieve all student information from the Student table for students who do not have any enrollment records in the Enrollment table.
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 888-88-8888 | ALICE        | SMITH       | TOPEKA  | KS       | 66610-1234 | CS       | JR       | 3.85   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
1 row(s) returned.

12. Student BOB NORBERT has moved to Overland Park, Kansas (KS). Update the StdCity and StdZip fields in the Student table to reflect this change, and then display the updated table using SELECT * FROM Student.

12. Updated Student7 table:
+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity       | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE       | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 124-56-7890 | BOB          | NORBERT     | OVERLAND PARK | KS       | 66214      | FIN      | JR       | 2.70   |
| 234-56-7890 | CANDY        | KENDALL     | TACOMA        | WA       | 99042-3321 | ACCT     | JR       | 3.50   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE       | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 456-78-9012 | JOE          | ESTRADA     | SEATTLE       | WA       | 98121-2333 | FIN      | SR       | 3.20   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE       | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 678-90-1234 | TESS         | DODGE       | REDMOND       | WA       | 98116-2344 | ACCT     | SO       | 3.30   |
| 789-01-2345 | ROBERTO      | MORALES     | SEATTLE       | WA       | 98121-2212 | FIN      | JR       | 2.50   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE       | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 888-88-8888 | ALICE        | SMITH       | TOPEKA        | KS       | 66610-1234 | CS       | JR       | 3.85   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE       | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL       | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
12 row(s) returned.

Assignment 7 completed successfully.
```

The output is correct


## Claude

Claude was a lot easier. That's only because it used lmysqlclient instead of mariadb connector directly. I just included it when compiling to avoid the linker error:

```
g++ claude.cpp -o claude -lmysqlclient
```

And then finally got my output:

```
Connected to mysql.eecs.ku.edu as 348f26_l382f965
Tables Course7, Student7, Faculty7, Offering7, Enrollment7 created and loaded.

1. Retrieve all students majoring in 'IS'.
   SQL: SELECT * FROM Student7 WHERE StdMajor = 'IS'

+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
6 rows

2. Find the student numbers of students who have enrolled in more than two courses.
   SQL: SELECT StdNo FROM Enrollment7 GROUP BY StdNo HAVING COUNT(*) > 2

+-------------+
| StdNo       |
+-------------+
| 123-45-6789 |
| 124-56-7890 |
| 234-56-7890 |
| 345-67-8901 |
| 456-78-9012 |
| 567-89-0123 |
| 678-90-1234 |
| 890-12-3456 |
| 901-23-4567 |
+-------------+
9 rows

3. List the first and last names of professors employed more than 15 years as of 2025 (hired in 2010 or earlier).
   SQL: SELECT FacFirstName, FacLastName FROM Faculty7 WHERE 2025 - YEAR(FacHireDate) > 15 OR YEAR(FacHireDate) <= 2010

+--------------+-------------+
| FacFirstName | FacLastName |
+--------------+-------------+
| LEONARD      | VINCE       |
| VICTORIA     | EMMANUEL    |
| LEONARD      | FIBON       |
| NICKI        | MACON       |
+--------------+-------------+
4 rows

4. Retrieve all course numbers offered in Summer 2020.
   SQL: SELECT CourseNo FROM Offering7 WHERE OffTerm = 'SUMMER' AND OffYear = 2020

+----------+
| CourseNo |
+----------+
| IS320    |
| IS320    |
+----------+
2 rows

5. List the first and last names of professors who live in Zip code 98114.
   SQL: SELECT FacFirstName, FacLastName FROM Faculty7 WHERE FacZipCode LIKE '98114%'

+--------------+-------------+
| FacFirstName | FacLastName |
+--------------+-------------+
| CRISTOPHER   | COLAN       |
| JULIA        | MILLS       |
+--------------+-------------+
2 rows

6. Retrieve the second-highest GPA from the Student table.
   SQL: SELECT MAX(StdGPA) AS SecondHighestGPA FROM Student7 WHERE StdGPA < (SELECT MAX(StdGPA) FROM Student7)

+------------------+
| SecondHighestGPA |
+------------------+
| 3.80             |
+------------------+
1 row

7. Retrieve the first and last names of individuals who appear in both the Student and Faculty tables.
   SQL: SELECT DISTINCT s.StdFirstName, s.StdLastName FROM Student7 s JOIN Faculty7 f ON s.StdFirstName = f.FacFirstName AND s.StdLastName = f.FacLastName

+--------------+-------------+
| StdFirstName | StdLastName |
+--------------+-------------+
| CRISTOPHER   | COLAN       |
+--------------+-------------+
1 row

8. Retrieve each student's number, first and last names, and the number of selected courses.
   SQL: SELECT s.StdNo, s.StdFirstName, s.StdLastName, COUNT(e.OfferNo) AS NumCourses FROM Student7 s LEFT JOIN Enrollment7 e ON s.StdNo = e.StdNo GROUP BY s.StdNo, s.StdFirstName, s.StdLastName ORDER BY s.StdNo

+-------------+--------------+-------------+------------+
| StdNo       | StdFirstName | StdLastName | NumCourses |
+-------------+--------------+-------------+------------+
| 123-45-6789 | HOMER        | WELLS       | 5          |
| 124-56-7890 | BOB          | NORBERT     | 4          |
| 234-56-7890 | CANDY        | KENDALL     | 4          |
| 345-67-8901 | WALLY        | KENDALL     | 3          |
| 456-78-9012 | JOE          | ESTRADA     | 3          |
| 567-89-0123 | MARIAH       | DODGE       | 4          |
| 678-90-1234 | TESS         | DODGE       | 3          |
| 789-01-2345 | ROBERTO      | MORALES     | 2          |
| 876-54-3210 | CRISTOPHER   | COLAN       | 2          |
| 890-12-3456 | LUKE         | BRAZZI      | 3          |
| 901-23-4567 | WILLIAM      | PILGRIM     | 5          |
+-------------+--------------+-------------+------------+
11 rows

9. Retrieve the names and salaries of the top three highest-paid professors.
   SQL: SELECT FacFirstName, FacLastName, FacSalary FROM Faculty7 ORDER BY FacSalary DESC LIMIT 3

+--------------+-------------+-----------+
| FacFirstName | FacLastName | FacSalary |
+--------------+-------------+-----------+
| VICTORIA     | EMMANUEL    | 120000.00 |
| JULIA        | MILLS       | 75000.00  |
| LEONARD      | FIBON       | 70000.00  |
+--------------+-------------+-----------+
3 rows

10. Insert junior CS student ALICE SMITH from TOPEKA, KS, Zip 66610-1234, StdNo 888-88-8888, GPA 3.85, then display the updated Student table.
   SQL: INSERT INTO Student7 (StdNo, StdFirstName, StdLastName, StdCity, StdState, StdZip, StdMajor, StdClass, StdGPA) VALUES ('888-88-8888', 'ALICE', 'SMITH', 'TOPEKA', 'KS', '66610-1234', 'CS', 'JR', 3.85)

(statement executed successfully)

   SQL: SELECT * FROM Student7

+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 124-56-7890 | BOB          | NORBERT     | BOTHELL | WA       | 98011-2121 | FIN      | JR       | 2.70   |
| 234-56-7890 | CANDY        | KENDALL     | TACOMA  | WA       | 99042-3321 | ACCT     | JR       | 3.50   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 456-78-9012 | JOE          | ESTRADA     | SEATTLE | WA       | 98121-2333 | FIN      | SR       | 3.20   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 678-90-1234 | TESS         | DODGE       | REDMOND | WA       | 98116-2344 | ACCT     | SO       | 3.30   |
| 789-01-2345 | ROBERTO      | MORALES     | SEATTLE | WA       | 98121-2212 | FIN      | JR       | 2.50   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 888-88-8888 | ALICE        | SMITH       | TOPEKA  | KS       | 66610-1234 | CS       | JR       | 3.85   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
12 rows

11. Retrieve all student information for students with no enrollment records.
   SQL: SELECT * FROM Student7 s WHERE NOT EXISTS (SELECT 1 FROM Enrollment7 e WHERE e.StdNo = s.StdNo)

+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
| 888-88-8888 | ALICE        | SMITH       | TOPEKA  | KS       | 66610-1234 | CS       | JR       | 3.85   |
+-------------+--------------+-------------+---------+----------+------------+----------+----------+--------+
1 row

12. BOB NORBERT moved to Overland Park, KS. Update StdCity and StdZip, then display the updated Student table.
   SQL: UPDATE Student7 SET StdCity = 'OVERLAND PARK', StdState = 'KS', StdZip = '66210' WHERE StdFirstName = 'BOB' AND StdLastName = 'NORBERT'

(statement executed successfully)

   SQL: SELECT * FROM Student7

+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
| StdNo       | StdFirstName | StdLastName | StdCity       | StdState | StdZip     | StdMajor | StdClass | StdGPA |
+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
| 123-45-6789 | HOMER        | WELLS       | SEATTLE       | WA       | 98121-1111 | IS       | FR       | 3.00   |
| 124-56-7890 | BOB          | NORBERT     | OVERLAND PARK | KS       | 66210      | FIN      | JR       | 2.70   |
| 234-56-7890 | CANDY        | KENDALL     | TACOMA        | WA       | 99042-3321 | ACCT     | JR       | 3.50   |
| 345-67-8901 | WALLY        | KENDALL     | SEATTLE       | WA       | 98123-1141 | IS       | SR       | 2.80   |
| 456-78-9012 | JOE          | ESTRADA     | SEATTLE       | WA       | 98121-2333 | FIN      | SR       | 3.20   |
| 567-89-0123 | MARIAH       | DODGE       | SEATTLE       | WA       | 98114-0021 | IS       | JR       | 3.60   |
| 678-90-1234 | TESS         | DODGE       | REDMOND       | WA       | 98116-2344 | ACCT     | SO       | 3.30   |
| 789-01-2345 | ROBERTO      | MORALES     | SEATTLE       | WA       | 98121-2212 | FIN      | JR       | 2.50   |
| 876-54-3210 | CRISTOPHER   | COLAN       | SEATTLE       | WA       | 98114-1332 | IS       | SR       | 4.00   |
| 888-88-8888 | ALICE        | SMITH       | TOPEKA        | KS       | 66610-1234 | CS       | JR       | 3.85   |
| 890-12-3456 | LUKE         | BRAZZI      | SEATTLE       | WA       | 98116-0021 | IS       | SR       | 2.20   |
| 901-23-4567 | WILLIAM      | PILGRIM     | BOTHELL       | WA       | 98113-1885 | IS       | SO       | 3.80   |
+-------------+--------------+-------------+---------------+----------+------------+----------+----------+--------+
```

## Output Correctness

Both program's output is correct. They output the correct changes on the table, and there is no doubt that the output is from the SQL tables themselves. Because of the lack of modularity that the program has (it is "dumb" because it is acting upon predefined tables doing predefined things), it is through a black box correct, despite the fact it was a pain to compile.

However, ChatGPT required editing of the source code to fix syntax errors. Therefore, Claude produced more correct working output that GPT did.

##Adherence

Both adhered to the object not function parameter, which is mostly what I tested for. However, I will note that Claude's adherence is a little silly. It creates a main runner class that inits the connection, and has the function run, which does everything, instead of the more robust OOP implementation chatgpt_mod.cpp does, I think that's much better. But we'll talk about that soon.


# Time Complexity

## Issues in Measuring Time Complexity

It would be boring if I analyze just the C++ time complexity of the code because they are largely the same. We'll talk more on the actual and discuss why it is slower, but simply put, the C++ is not the main driver. We instead should compare the time complexity. To do that, we have to understand what makes SQL commands better or worse for time complexity. This specifically is called Query Complexity.

This can get very complicated quickly. A good example of this is cache queries. If our previous query used the same identifier, it can still be in the cache, causing constant time lookup query complexity. On the other hand, by using the wildcard operator, we can have exponential or logarithmic time complexity. Clearly this matters.

This portion will have its own seperate theoretical, but will be discussed in conjunction with the actual time complexity since the two become merged within the problem statement.

## Theoretical

## C++ Code
Both of these are just creating some sort of facade to interact with the SQL table. Because our input size is nothing, as there is no input given except for the pre-existing SQL tables which the program needs to run, there should be no extra time complexity depending on the weight of the C++. The time complexity is non-existent because it cannot change for input size, there is no input size. 

There is nothing for me to give feedback on this route except for actually running the code and seeing what specific C++ features cause slowdowns.

The more interesting question is whether the abstraction chosen actually is good for the speed of execution. But as per the requirements, we're not just sending shell commands and calling it a day. Instead, we're creating an OOP facade that we interface with.

## SQL Time Complexity

Most of these queries fall under linear or logarithimic query time if cached. This is because we don't have any limits and we're just querying attributes, not relationships between two tables really. This makes it easier to filter in.

Theoretically, assuming our cache holds post query results for a bit, we can assume that the query of a specific attribute on a specific table starts of linear O(n) and then proceeds to become O( log (n) ) for each subsequent part.

Inserts are a constant time operation because they require no previous context into the table. 

## Actual
