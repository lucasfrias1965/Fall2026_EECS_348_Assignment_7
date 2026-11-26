// Assignment7.cpp
// Object-oriented C++ program that creates five MariaDB/MySQL tables (suffix "7"),
// loads their data, runs 12 SQL queries, and prints the results in table format.
//
// Compile (Linux, MariaDB Connector/C or libmysqlclient):
//   g++ -std=c++17 Assignment7.cpp -o Assignment7 $(mariadb_config --cflags --libs)
//   -- or --
//   g++ -std=c++17 Assignment7.cpp -o Assignment7 $(mysql_config --cflags --libs)
//   -- or, if headers are in /usr/include/mysql --
//   g++ -std=c++17 Assignment7.cpp -o Assignment7 -I/usr/include/mysql -lmysqlclient
//
// Run:
//   ./Assignment7

#include <mysql/mysql.h>   // if your system uses <mariadb/mysql.h>, change this line

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Connection settings (macros)
// ---------------------------------------------------------------------------
#define DB_HOST     "mysql.eecs.ku.edu"
#define DB_USER     "348f26_l382f965"
#define DB_PASSWORD "gaumanp6rQxG"   // <-- put your real password here
#define DB_NAME     "348f26_l382f965"              // default schema = username on KU EECS; change if different
#define DB_PORT     3306

// ---------------------------------------------------------------------------
// ResultSet: owns a MYSQL_RES* and knows how to print itself as a table.
// ---------------------------------------------------------------------------
class ResultSet {
public:
    explicit ResultSet(MYSQL_RES* res) : res_(res) {}
    ~ResultSet() { if (res_) mysql_free_result(res_); }

    ResultSet(const ResultSet&) = delete;
    ResultSet& operator=(const ResultSet&) = delete;
    ResultSet(ResultSet&& o) noexcept : res_(o.res_) { o.res_ = nullptr; }
    ResultSet& operator=(ResultSet&& o) noexcept {
        if (this != &o) {
            if (res_) mysql_free_result(res_);
            res_ = o.res_;
            o.res_ = nullptr;
        }
        return *this;
    }

    void print(std::ostream& out = std::cout) const {
        if (!res_) {
            out << "(no result set)\n";
            return;
        }

        const unsigned int nCols = mysql_num_fields(res_);
        MYSQL_FIELD* fields = mysql_fetch_fields(res_);

        // Pull every row into memory so column widths can be computed.
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
        std::vector<size_t> widths(nCols, 0);

        for (unsigned int i = 0; i < nCols; ++i) {
            headers.emplace_back(fields[i].name);
            widths[i] = headers[i].size();
        }

        mysql_data_seek(res_, 0);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res_)) != nullptr) {
            std::vector<std::string> r;
            for (unsigned int i = 0; i < nCols; ++i) {
                std::string v = row[i] ? row[i] : "NULL";
                widths[i] = std::max(widths[i], v.size());
                r.push_back(std::move(v));
            }
            rows.push_back(std::move(r));
        }

        auto border = [&]() {
            out << '+';
            for (size_t w : widths) out << std::string(w + 2, '-') << '+';
            out << '\n';
        };
        auto line = [&](const std::vector<std::string>& cells) {
            out << '|';
            for (unsigned int i = 0; i < nCols; ++i)
                out << ' ' << std::left << std::setw(static_cast<int>(widths[i])) << cells[i] << " |";
            out << '\n';
        };

        border();
        line(headers);
        border();
        for (const auto& r : rows) line(r);
        border();
        out << rows.size() << (rows.size() == 1 ? " row" : " rows") << "\n";
    }

private:
    MYSQL_RES* res_;
};

// ---------------------------------------------------------------------------
// Database: wraps the MYSQL connection.
// ---------------------------------------------------------------------------
class Database {
public:
    Database(std::string host, std::string user, std::string password,
             std::string dbName, unsigned int port)
        : host_(std::move(host)), user_(std::move(user)), password_(std::move(password)),
          dbName_(std::move(dbName)), port_(port), conn_(nullptr) {}

    ~Database() { disconnect(); }

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void connect() {
        conn_ = mysql_init(nullptr);
        if (!conn_) throw std::runtime_error("mysql_init failed");

        if (!mysql_real_connect(conn_, host_.c_str(), user_.c_str(), password_.c_str(),
                                dbName_.c_str(), port_, nullptr, 0)) {
            std::string msg = std::string("Connection failed: ") + mysql_error(conn_);
            mysql_close(conn_);
            conn_ = nullptr;
            throw std::runtime_error(msg);
        }
    }

    void disconnect() {
        if (conn_) {
            mysql_close(conn_);
            conn_ = nullptr;
        }
    }

    // For statements that return no rows (CREATE, DROP, INSERT, UPDATE).
    void execute(const std::string& sql) {
        if (mysql_query(conn_, sql.c_str()) != 0)
            throw std::runtime_error("SQL error: " + std::string(mysql_error(conn_)) +
                                     "\n  in: " + sql);
    }

    // For SELECT statements.
    ResultSet query(const std::string& sql) {
        execute(sql);
        MYSQL_RES* res = mysql_store_result(conn_);
        if (!res && mysql_field_count(conn_) != 0)
            throw std::runtime_error("Could not store result: " + std::string(mysql_error(conn_)));
        return ResultSet(res);
    }

private:
    std::string host_, user_, password_, dbName_;
    unsigned int port_;
    MYSQL* conn_;
};

// ---------------------------------------------------------------------------
// SchemaBuilder: drops/creates the five "7" tables and loads their data.
// Data is taken from the five .SQL files supplied in Assignment7SQLTables.zip.
// ---------------------------------------------------------------------------
class SchemaBuilder {
public:
    explicit SchemaBuilder(Database& db) : db_(db) {}

    void build() {
        dropTables();
        createTables();
        loadCourse();
        loadStudent();
        loadFaculty();
        loadOffering();
        loadEnrollment();
    }

private:
    Database& db_;

    void dropTables() {
        // Child tables first so any foreign-key-like ordering is respected.
        db_.execute("DROP TABLE IF EXISTS Enrollment7");
        db_.execute("DROP TABLE IF EXISTS Offering7");
        db_.execute("DROP TABLE IF EXISTS Course7");
        db_.execute("DROP TABLE IF EXISTS Faculty7");
        db_.execute("DROP TABLE IF EXISTS Student7");
    }

    void createTables() {
        db_.execute(
            "CREATE TABLE Course7 ("
            "CourseNo CHAR(6), CrsDesc VARCHAR(50), CrsUnits INTEGER, "
            "PRIMARY KEY (CourseNo))");

        db_.execute(
            "CREATE TABLE Student7 ("
            "StdNo CHAR(11), StdFirstName VARCHAR(50), StdLastName VARCHAR(50), "
            "StdCity VARCHAR(50), StdState CHAR(2), StdZip CHAR(10), "
            "StdMajor CHAR(6), StdClass CHAR(6), StdGPA DECIMAL(3,2) DEFAULT 0, "
            "PRIMARY KEY (StdNo))");

        db_.execute(
            "CREATE TABLE Faculty7 ("
            "FacNo CHAR(11), FacFirstName VARCHAR(30), FacLastName VARCHAR(30), "
            "FacCity VARCHAR(30), FacState CHAR(2), FacDept CHAR(10), FacRank CHAR(4), "
            "FacSalary DECIMAL(10,2), FacSupervisor CHAR(11), FacHireDate DATE, "
            "FacZipCode CHAR(10), PRIMARY KEY (FacNo))");

        db_.execute(
            "CREATE TABLE Offering7 ("
            "OfferNo INTEGER, CourseNo CHAR(6), OffTerm CHAR(6), OffYear INTEGER, "
            "OffLocation VARCHAR(30), OffTime VARCHAR(10), FacNo CHAR(11), "
            "OffDays CHAR(6) DEFAULT 'MW', PRIMARY KEY (OfferNo))");

        db_.execute(
            "CREATE TABLE Enrollment7 ("
            "OfferNo INTEGER, StdNo CHAR(11), EnrGrade DECIMAL(3,2), "
            "PRIMARY KEY (OfferNo, StdNo))");
    }

    void loadCourse() {
        const std::string pre = "INSERT INTO Course7 (CourseNo, CrsDesc, CrsUnits) VALUES ";
        const char* rows[] = {
            "('FIN300', 'FUNDAMENTALS OF FINANCE', 4)",
            "('FIN450', 'PRINCIPLES OF INVESTMENTS', 4)",
            "('FIN480', 'CORPORATE FINANCE', 4)",
            "('IS320', 'FUNDAMENTALS OF BUSINESS PROGRAMMING', 4)",
            "('IS460', 'SYSTEMS ANALYSIS', 4)",
            "('IS470', 'BUSINESS DATA COMMUNICATIONS', 4)",
            "('IS480', 'FUNDAMENTALS OF DATABASE MANAGEMENT', 4)"};
        for (const char* r : rows) db_.execute(pre + r);
    }

    void loadStudent() {
        const std::string pre =
            "INSERT INTO Student7 (StdNo, StdFirstName, StdLastName, StdCity, StdState, "
            "StdMajor, StdClass, StdGPA, StdZip) VALUES ";
        const char* rows[] = {
            "('123-45-6789', 'HOMER', 'WELLS', 'SEATTLE', 'WA', 'IS', 'FR', 3.00, '98121-1111')",
            "('124-56-7890', 'BOB', 'NORBERT', 'BOTHELL', 'WA', 'FIN', 'JR', 2.70, '98011-2121')",
            "('234-56-7890', 'CANDY', 'KENDALL', 'TACOMA', 'WA', 'ACCT', 'JR', 3.50, '99042-3321')",
            "('345-67-8901', 'WALLY', 'KENDALL', 'SEATTLE', 'WA', 'IS', 'SR', 2.80, '98123-1141')",
            "('456-78-9012', 'JOE', 'ESTRADA', 'SEATTLE', 'WA', 'FIN', 'SR', 3.20, '98121-2333')",
            "('567-89-0123', 'MARIAH', 'DODGE', 'SEATTLE', 'WA', 'IS', 'JR', 3.60, '98114-0021')",
            "('678-90-1234', 'TESS', 'DODGE', 'REDMOND', 'WA', 'ACCT', 'SO', 3.30, '98116-2344')",
            "('789-01-2345', 'ROBERTO', 'MORALES', 'SEATTLE', 'WA', 'FIN', 'JR', 2.50, '98121-2212')",
            "('876-54-3210', 'CRISTOPHER', 'COLAN', 'SEATTLE', 'WA', 'IS', 'SR', 4.00, '98114-1332')",
            "('890-12-3456', 'LUKE', 'BRAZZI', 'SEATTLE', 'WA', 'IS', 'SR', 2.20, '98116-0021')",
            "('901-23-4567', 'WILLIAM', 'PILGRIM', 'BOTHELL', 'WA', 'IS', 'SO', 3.80, '98113-1885')"};
        for (const char* r : rows) db_.execute(pre + r);
    }

    void loadFaculty() {
        const std::string pre =
            "INSERT INTO Faculty7 (FacNo, FacFirstName, FacLastName, FacCity, FacState, "
            "FacDept, FacRank, FacSalary, FacSupervisor, FacHireDate, FacZipCode) VALUES ";
        const char* rows[] = {
            "('543-21-0987', 'VICTORIA', 'EMMANUEL', 'BOTHELL', 'WA', 'MS', 'PROF', 120000.00, NULL, '2008-04-15', '98011-2242')",
            "('765-43-2109', 'NICKI', 'MACON', 'BELLEVUE', 'WA', 'FIN', 'PROF', 65000.00, NULL, '2009-04-11', '98015-9945')",
            "('654-32-1098', 'LEONARD', 'FIBON', 'SEATTLE', 'WA', 'MS', 'ASSC', 70000.00, '543-21-0987', '2006-05-01', '98121-0094')",
            "('098-76-5432', 'LEONARD', 'VINCE', 'SEATTLE', 'WA', 'MS', 'ASST', 35000.00, '654-32-1098', '2007-04-10', '98111-9921')",
            "('876-54-3210', 'CRISTOPHER', 'COLAN', 'SEATTLE', 'WA', 'MS', 'ASST', 40000.00, '654-32-1098', '2011-03-01', '98114-1332')",
            "('987-65-4321', 'JULIA', 'MILLS', 'SEATTLE', 'WA', 'FIN', 'ASSC', 75000.00, '765-43-2109', '2012-03-15', '98114-9954')"};
        for (const char* r : rows) db_.execute(pre + r);
    }

    void loadOffering() {
        const std::string pre =
            "INSERT INTO Offering7 (OfferNo, CourseNo, OffTerm, OffYear, OffLocation, "
            "OffTime, FacNo, OffDays) VALUES ";
        const char* rows[] = {
            "(1111, 'IS320', 'SUMMER', 2020, 'BLM302', '10:30:00', NULL, 'MW')",
            "(1234, 'IS320', 'FALL', 2019, 'BLM302', '10:30:00', '098-76-5432', 'MW')",
            "(2222, 'IS460', 'SUMMER', 2019, 'BLM412', '13:30:00', NULL, 'TTH')",
            "(3333, 'IS320', 'SPRING', 2020, 'BLM214', '08:30:00', '098-76-5432', 'MW')",
            "(4321, 'IS320', 'FALL', 2019, 'BLM214', '15:30:00', '098-76-5432', 'TTH')",
            "(4444, 'IS320', 'WINTER', 2020, 'BLM302', '15:30:00', '543-21-0987', 'TTH')",
            "(5555, 'FIN300', 'WINTER', 2020, 'BLM207', '08:30:00', '765-43-2109', 'MW')",
            "(5678, 'IS480', 'WINTER', 2020, 'BLM302', '10:30:00', '987-65-4321', 'MW')",
            "(5679, 'IS480', 'SPRING', 2020, 'BLM412', '15:30:00', '876-54-3210', 'TTH')",
            "(6666, 'FIN450', 'WINTER', 2020, 'BLM212', '10:30:00', '987-65-4321', 'TTH')",
            "(7777, 'FIN480', 'SPRING', 2020, 'BLM305', '13:30:00', '765-43-2109', 'MW')",
            "(8888, 'IS320', 'SUMMER', 2020, 'BLM405', '13:30:00', '654-32-1098', 'MW')",
            "(9876, 'IS460', 'SPRING', 2020, 'BLM307', '13:30:00', '654-32-1098', 'TTH')"};
        for (const char* r : rows) db_.execute(pre + r);
    }

    void loadEnrollment() {
        const std::string pre = "INSERT INTO Enrollment7 (OfferNo, StdNo, EnrGrade) VALUES ";
        const char* rows[] = {
            "(1234, '123-45-6789', 3.30)", "(1234, '234-56-7890', 3.50)",
            "(1234, '345-67-8901', 3.20)", "(1234, '456-78-9012', 3.10)",
            "(1234, '567-89-0123', 3.80)", "(1234, '678-90-1234', 3.40)",
            "(4321, '123-45-6789', 3.50)", "(4321, '124-56-7890', 3.20)",
            "(4321, '789-01-2345', 3.50)", "(4321, '876-54-3210', 3.10)",
            "(4321, '890-12-3456', 3.40)", "(4321, '901-23-4567', 3.10)",
            "(5555, '123-45-6789', 3.20)", "(5555, '124-56-7890', 2.70)",
            "(5678, '123-45-6789', 3.20)", "(5678, '234-56-7890', 2.80)",
            "(5678, '345-67-8901', 3.30)", "(5678, '456-78-9012', 3.40)",
            "(5678, '567-89-0123', 2.60)", "(5679, '123-45-6789', 2.00)",
            "(5679, '124-56-7890', 3.70)", "(5679, '678-90-1234', 3.30)",
            "(5679, '789-01-2345', 3.80)", "(5679, '890-12-3456', 2.90)",
            "(5679, '901-23-4567', 3.10)", "(6666, '234-56-7890', 3.10)",
            "(6666, '567-89-0123', 3.60)", "(7777, '876-54-3210', 3.40)",
            "(7777, '890-12-3456', 3.70)", "(7777, '901-23-4567', 3.40)",
            "(9876, '124-56-7890', 3.50)", "(9876, '234-56-7890', 3.20)",
            "(9876, '345-67-8901', 3.20)", "(9876, '456-78-9012', 3.40)",
            "(9876, '567-89-0123', 2.60)", "(9876, '678-90-1234', 3.30)",
            "(9876, '901-23-4567', 4.00)", "(1111, '901-23-4567', 0.00)"};
        for (const char* r : rows) db_.execute(pre + r);
    }
};

// ---------------------------------------------------------------------------
// Query: one numbered assignment query. Holds its description, SQL, and kind.
// ---------------------------------------------------------------------------
class Query {
public:
    enum class Kind { Select, Modify };

    Query(int number, std::string description, std::string sql, Kind kind = Kind::Select)
        : number_(number), description_(std::move(description)),
          sql_(std::move(sql)), kind_(kind) {}

    int number() const { return number_; }
    const std::string& description() const { return description_; }
    const std::string& sql() const { return sql_; }
    Kind kind() const { return kind_; }

private:
    int number_;
    std::string description_;
    std::string sql_;
    Kind kind_;
};

// ---------------------------------------------------------------------------
// QueryRunner: executes Query objects and prints "number. description" + results.
// A Modify query may carry a follow-up SELECT (queries 10 and 12).
// ---------------------------------------------------------------------------
class QueryRunner {
public:
    explicit QueryRunner(Database& db) : db_(db) {}

    void add(Query q) { queries_.push_back(std::move(q)); }

    // For Modify queries: SELECT to display afterwards.
    void addFollowUp(int number, std::string selectSql) {
        followUps_.emplace_back(number, std::move(selectSql));
    }

    void runAll() {
        for (const Query& q : queries_) {
            std::cout << "\n" << q.number() << ". " << q.description() << "\n";
            std::cout << "   SQL: " << q.sql() << "\n\n";

            if (q.kind() == Query::Kind::Select) {
                db_.query(q.sql()).print();
            } else {
                db_.execute(q.sql());
                std::cout << "(statement executed successfully)\n";
                for (const auto& f : followUps_) {
                    if (f.first == q.number()) {
                        std::cout << "\n   SQL: " << f.second << "\n\n";
                        db_.query(f.second).print();
                    }
                }
            }
        }
    }

private:
    Database& db_;
    std::vector<Query> queries_;
    std::vector<std::pair<int, std::string>> followUps_;
};

// ---------------------------------------------------------------------------
// Assignment7App: ties everything together.
// ---------------------------------------------------------------------------
class Assignment7App {
public:
    Assignment7App()
        : db_(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, DB_PORT), runner_(db_) {}

    int run() {
        try {
            db_.connect();
            std::cout << "Connected to " << DB_HOST << " as " << DB_USER << "\n";

            SchemaBuilder(db_).build();
            std::cout << "Tables Course7, Student7, Faculty7, Offering7, Enrollment7 created and loaded.\n";

            registerQueries();
            runner_.runAll();
        } catch (const std::exception& e) {
            std::cerr << "\nERROR: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

private:
    Database db_;
    QueryRunner runner_;

    void registerQueries() {
        using K = Query::Kind;

        runner_.add(Query(1,
            "Retrieve all students majoring in 'IS'.",
            "SELECT * FROM Student7 WHERE StdMajor = 'IS'"));

        runner_.add(Query(2,
            "Find the student numbers of students who have enrolled in more than two courses.",
            "SELECT StdNo FROM Enrollment7 GROUP BY StdNo HAVING COUNT(*) > 2"));

        // More than 15 years as of 2025  =>  hired in 2010 or earlier.
        runner_.add(Query(3,
            "List the first and last names of professors employed more than 15 years as of 2025 "
            "(hired in 2010 or earlier).",
            "SELECT FacFirstName, FacLastName FROM Faculty7 WHERE 2025 - YEAR(FacHireDate) > 15 "
            "OR YEAR(FacHireDate) <= 2010"));

        runner_.add(Query(4,
            "Retrieve all course numbers offered in Summer 2020.",
            "SELECT CourseNo FROM Offering7 WHERE OffTerm = 'SUMMER' AND OffYear = 2020"));

        runner_.add(Query(5,
            "List the first and last names of professors who live in Zip code 98114.",
            "SELECT FacFirstName, FacLastName FROM Faculty7 WHERE FacZipCode LIKE '98114%'"));

        runner_.add(Query(6,
            "Retrieve the second-highest GPA from the Student table.",
            "SELECT MAX(StdGPA) AS SecondHighestGPA FROM Student7 "
            "WHERE StdGPA < (SELECT MAX(StdGPA) FROM Student7)"));

        runner_.add(Query(7,
            "Retrieve the first and last names of individuals who appear in both the Student and Faculty tables.",
            "SELECT DISTINCT s.StdFirstName, s.StdLastName FROM Student7 s "
            "JOIN Faculty7 f ON s.StdFirstName = f.FacFirstName AND s.StdLastName = f.FacLastName"));

        runner_.add(Query(8,
            "Retrieve each student's number, first and last names, and the number of selected courses.",
            "SELECT s.StdNo, s.StdFirstName, s.StdLastName, COUNT(e.OfferNo) AS NumCourses "
            "FROM Student7 s LEFT JOIN Enrollment7 e ON s.StdNo = e.StdNo "
            "GROUP BY s.StdNo, s.StdFirstName, s.StdLastName ORDER BY s.StdNo"));

        runner_.add(Query(9,
            "Retrieve the names and salaries of the top three highest-paid professors.",
            "SELECT FacFirstName, FacLastName, FacSalary FROM Faculty7 "
            "ORDER BY FacSalary DESC LIMIT 3"));

        runner_.add(Query(10,
            "Insert junior CS student ALICE SMITH from TOPEKA, KS, Zip 66610-1234, "
            "StdNo 888-88-8888, GPA 3.85, then display the updated Student table.",
            "INSERT INTO Student7 (StdNo, StdFirstName, StdLastName, StdCity, StdState, "
            "StdZip, StdMajor, StdClass, StdGPA) VALUES "
            "('888-88-8888', 'ALICE', 'SMITH', 'TOPEKA', 'KS', '66610-1234', 'CS', 'JR', 3.85)",
            K::Modify));
        runner_.addFollowUp(10, "SELECT * FROM Student7");

        runner_.add(Query(11,
            "Retrieve all student information for students with no enrollment records.",
            "SELECT * FROM Student7 s WHERE NOT EXISTS "
            "(SELECT 1 FROM Enrollment7 e WHERE e.StdNo = s.StdNo)"));

        runner_.add(Query(12,
            "BOB NORBERT moved to Overland Park, KS. Update StdCity and StdZip, "
            "then display the updated Student table.",
            "UPDATE Student7 SET StdCity = 'OVERLAND PARK', StdState = 'KS', StdZip = '66210' "
            "WHERE StdFirstName = 'BOB' AND StdLastName = 'NORBERT'",
            K::Modify));
        runner_.addFollowUp(12, "SELECT * FROM Student7");
    }
};

// ---------------------------------------------------------------------------
int main() {
    Assignment7App app;
    return app.run();
}
