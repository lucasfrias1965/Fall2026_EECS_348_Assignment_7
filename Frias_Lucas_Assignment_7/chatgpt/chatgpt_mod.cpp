/*
 * Assignment 7 - Object-oriented C++ / MariaDB
 *
 * Requirements:
 *   - Extract Assignment7SQLTables.zip first.
 *   - The five .SQL files should be in ./Assignment7_SQL_tables/
 *   - Install MariaDB Connector/C++ and compile with its include/library flags.
 *
 * Example compile command (adjust for your installation):
 *   g++ -std=c++17 Assignment7.cpp -o Assignment7 \
 *       $(mariadb_config --cflags --libs) -lmariadbcpp
 *
 * Some installations provide a different pkg-config / linker command for
 * MariaDB Connector/C++; use the command supplied with your installation.
 */

#include <mariadb/conncpp.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Database connection settings.
// Replace the password placeholder with your actual MariaDB password.
// Do not submit a source file containing your real password.
#define DB_HOST "mysql.eecs.ku.edu"
#define DB_USER "348f26_l382f965"
#define DB_PASSWORD "gaumanp6rQxG"
#define DB_NAME "348f26_l382f965"

// Overland Park has multiple ZIP codes. Replace this placeholder with
// BOB NORBERT'S actual new ZIP code before running the program.
#define BOB_NEW_ZIP "66214"

class Assignment7 {
private:
    sql::Driver* driver;
    std::unique_ptr<sql::Connection> connection;

    const std::string scriptDirectory = "Assignment7_SQL_tables/";

    // Read one supplied SQL script and rename every table identifier.
    std::string readAndRenameScript(const std::string& fileName,
                                    const std::string& tableName) {
        std::ifstream input(scriptDirectory + fileName);
        if (!input) {
            throw std::runtime_error(
                "Cannot open " + scriptDirectory + fileName +
                ". Extract the ZIP and check the folder name.");
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::string contents = buffer.str();

        // Match whole words, case-insensitively, so Student becomes Student7,
        // without accidentally changing a longer identifier.
        const std::regex tablePattern("\\b" + tableName + "\\b",
                                      std::regex_constants::icase);
        return std::regex_replace(contents, tablePattern, tableName + "7");
    }

    // Execute each semicolon-terminated statement in a supplied SQL file.
    void createAndLoadTable(const std::string& fileName,
                            const std::string& tableName) {
        const std::string script = readAndRenameScript(fileName, tableName);
        std::stringstream statements(script);
        std::string statementText;

        std::unique_ptr<sql::Statement> statement(
            connection->createStatement());

        while (std::getline(statements, statementText, ';')) {
            // Ignore blank statements.
            if (statementText.find_first_not_of(" \t\r\n") ==
                std::string::npos) {
                continue;
            }
            statement->execute(statementText);
        }

        std::cout << "Created and loaded " << tableName << "7.\n";
    }

    // Print a result set in a simple table format.
    void printResult(sql::ResultSet* result) {
        std::unique_ptr<sql::ResultSet> rows(result);
        sql::ResultSetMetaData* metadata = rows->getMetaData();
        const unsigned int columnCount = metadata->getColumnCount();

        std::vector<std::string> headings;
        std::vector<std::size_t> widths(columnCount, 3);

        for (unsigned int column = 1; column <= columnCount; ++column) {
            headings.push_back(metadata->getColumnLabel(column).c_str() );
            widths[column - 1] = std::max(
                widths[column - 1], headings.back().length());
        }

        // Buffer the rows so column widths can be sized to their contents.
        std::vector<std::vector<std::string>> data;
        while (rows->next()) {
            std::vector<std::string> row;
            for (unsigned int column = 1; column <= columnCount; ++column) {
                row.push_back(rows->isNull(column)
                        ? std::string("NULL")
                        : std::string(rows->getString(column).c_str()));

                widths[column - 1] =
                    std::max(widths[column - 1], row.back().length());
            }
            data.push_back(row);
        }

        const auto printSeparator = [&]() {
            for (std::size_t width : widths) {
                std::cout << '+' << std::string(width + 2, '-');
            }
            std::cout << "+\n";
        };

        printSeparator();
        for (unsigned int column = 0; column < columnCount; ++column) {
            std::cout << "| " << std::left
                      << std::setw(static_cast<int>(widths[column]))
                      << headings[column] << ' ';
        }
        std::cout << "|\n";
        printSeparator();

        for (const auto& row : data) {
            for (unsigned int column = 0; column < columnCount; ++column) {
                std::cout << "| " << std::left
                          << std::setw(static_cast<int>(widths[column]))
                          << row[column] << ' ';
            }
            std::cout << "|\n";
        }
        printSeparator();

        if (data.empty()) {
            std::cout << "(No rows returned.)\n";
        } else {
            std::cout << data.size() << " row(s) returned.\n";
        }
    }

    // Display the query number, assignment wording, and returned table.
    void runQuery(int number, const std::string& description,
                  const std::string& query) {
        std::cout << "\n" << number << ". " << description << "\n";
        std::unique_ptr<sql::Statement> statement(
            connection->createStatement());
        printResult(statement->executeQuery(query));
    }

    // Execute an INSERT or UPDATE statement that does not return rows.
    void runUpdate(const std::string& sqlText) {
        std::unique_ptr<sql::Statement> statement(
            connection->createStatement());
        statement->executeUpdate(sqlText);
    }

public:
    Assignment7() : driver(nullptr) {
        driver = sql::mariadb::get_driver_instance();

        const std::string url =
            "jdbc:mariadb://" DB_HOST ":3306/" DB_NAME;

        sql::Properties properties;
        properties["user"] = DB_USER;
        properties["password"] = DB_PASSWORD;

        connection.reset(driver->connect(url, properties));
        connection->setAutoCommit(true);
    }

    void run() {
        std::cout << "Connected to MariaDB.\n\n";
        std::cout << "Creating and loading the five Assignment 7 tables...\n";

        // The supplied SQL files contain DROP, CREATE, and INSERT statements.
        // Only the table names are changed; the supplied data is preserved.
        createAndLoadTable("student.SQL", "Student");
        createAndLoadTable("faculty.SQL", "Faculty");
        createAndLoadTable("course.SQL", "Course");
        createAndLoadTable("enrollment.SQL", "Enrollment");
        createAndLoadTable("offering.SQL", "Offering");

        runQuery(
            1,
            "Retrieve all student information for those whose major is 'IS'.",
            "SELECT * FROM Student7 WHERE StdMajor = 'IS'");

        runQuery(
            2,
            "Retrieve the student numbers (StdNo) of students who have "
            "enrolled in more than two courses.",
            "SELECT StdNo "
            "FROM Enrollment7 "
            "GROUP BY StdNo "
            "HAVING COUNT(DISTINCT OfferNo) > 2");

        runQuery(
            3,
            "List the first and last names of all professors from the Faculty "
            "table who have been employed for more than 15 years as of 2025, "
            "including those hired in 2010 or earlier.",
            "SELECT FacFirstName, FacLastName "
            "FROM Faculty7 "
            "WHERE FacHireDate <= '2010-12-31'");

        runQuery(
            4,
            "Retrieve all course numbers from the Offering table that were "
            "offered in Summer 2020.",
            "SELECT DISTINCT CourseNo "
            "FROM Offering7 "
            "WHERE UPPER(OffTerm) = 'SUMMER' AND OffYear = 2020");

        runQuery(
            5,
            "List the first and last names of all professors from the Faculty "
            "table who live in Zip code 98114.",
            "SELECT FacFirstName, FacLastName "
            "FROM Faculty7 "
            "WHERE FacZipCode LIKE '98114%'");

        runQuery(
            6,
            "Retrieve the second-highest GPA from the Student table.",
            "SELECT DISTINCT StdGPA "
            "FROM Student7 "
            "ORDER BY StdGPA DESC "
            "LIMIT 1 OFFSET 1");

        runQuery(
            7,
            "Retrieve the first and last names of individuals who appear in "
            "both the Student and Faculty tables.",
            "SELECT DISTINCT s.StdFirstName AS FirstName, "
            "s.StdLastName AS LastName "
            "FROM Student7 s "
            "INNER JOIN Faculty7 f "
            "ON s.StdFirstName = f.FacFirstName "
            "AND s.StdLastName = f.FacLastName");

        runQuery(
            8,
            "Retrieve the student number (StdNo), first and last names, and "
            "the number of selected courses from the Student and Enrollment "
            "tables.",
            "SELECT s.StdNo, s.StdFirstName, s.StdLastName, "
            "COUNT(DISTINCT e.OfferNo) AS SelectedCourses "
            "FROM Student7 s "
            "LEFT JOIN Enrollment7 e ON s.StdNo = e.StdNo "
            "GROUP BY s.StdNo, s.StdFirstName, s.StdLastName "
            "ORDER BY s.StdNo");

        runQuery(
            9,
            "Retrieve the first and last names of professors, along with "
            "their salaries, for the top three highest-paid professors "
            "from the Faculty table.",
            "SELECT FacFirstName, FacLastName, FacSalary "
            "FROM Faculty7 "
            "ORDER BY FacSalary DESC "
            "LIMIT 3");

        std::cout
            << "\n10. Insert a junior CS student named ALICE SMITH from "
               "TOPEKA, KS, Zip 66610-1234, with student number "
               "888-88-8888 and a GPA of 3.85 into the Student table. "
               "Then display the updated table using SELECT * FROM Student.\n";

        // ON DUPLICATE KEY UPDATE makes the program safe to run again.
        runUpdate(
            "INSERT INTO Student7 "
            "(StdNo, StdFirstName, StdLastName, StdCity, StdState, StdZip, "
            "StdMajor, StdClass, StdGPA) "
            "VALUES ('888-88-8888', 'ALICE', 'SMITH', 'TOPEKA', 'KS', "
            "'66610-1234', 'CS', 'JR', 3.85) "
            "ON DUPLICATE KEY UPDATE "
            "StdFirstName = VALUES(StdFirstName), "
            "StdLastName = VALUES(StdLastName), "
            "StdCity = VALUES(StdCity), "
            "StdState = VALUES(StdState), "
            "StdZip = VALUES(StdZip), "
            "StdMajor = VALUES(StdMajor), "
            "StdClass = VALUES(StdClass), "
            "StdGPA = VALUES(StdGPA)");

        runQuery(10, "Updated Student7 table:",
                 "SELECT * FROM Student7");

        runQuery(
            11,
            "Retrieve all student information from the Student table for "
            "students who do not have any enrollment records in the "
            "Enrollment table.",
            "SELECT s.* "
            "FROM Student7 s "
            "WHERE NOT EXISTS "
            "(SELECT 1 FROM Enrollment7 e WHERE e.StdNo = s.StdNo)");

        std::cout
            << "\n12. Student BOB NORBERT has moved to Overland Park, Kansas "
               "(KS). Update the StdCity and StdZip fields in the Student "
               "table to reflect this change, and then display the updated "
               "table using SELECT * FROM Student.\n";

        const std::string newZip = BOB_NEW_ZIP;
        if (newZip == "REPLACE_WITH_BOBS_NEW_ZIP") {
            throw std::runtime_error(
                "Before running Query 12, replace BOB_NEW_ZIP at the top "
                "of the source code with Bob Norbert's actual ZIP code. "
                "Overland Park has multiple ZIP codes.");
        }

        // The assignment gives the new city and state, but not the ZIP code.
        // The ZIP must be supplied rather than guessed.
        const std::string updateBob =
            "UPDATE Student7 "
            "SET StdCity = 'OVERLAND PARK', StdState = 'KS', StdZip = '" +
            newZip +
            "' WHERE StdFirstName = 'BOB' AND StdLastName = 'NORBERT'";

        runUpdate(updateBob);
        runQuery(12, "Updated Student7 table:",
                 "SELECT * FROM Student7");

        std::cout << "\nAssignment 7 completed successfully.\n";
    }
};

int main() {
    try {
        Assignment7 assignment;
        assignment.run();
    } catch (sql::SQLException& error) {
        std::cerr << "MariaDB error: " << error.what()
                  << "\nError code: " << error.getErrorCode()
                  << "\nSQL state: " << error.getSQLState() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Program error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
