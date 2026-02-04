#include <gtest/gtest.h>
#include <sstream>
#include "TinyP/SqlParser.h"
#include "TinyP/SqlEval.h"
#include "TinyP/Table.h"

using namespace TinyP;

// Helper to create a test table with employees
TablePtr createEmployeeTable() {
    auto table = std::make_shared<Table>("employees");
    table->addColumn("id", true);
    table->addColumn("name", false);
    table->addColumn("age", true);
    table->addColumn("salary", true);
    table->addColumn("department", false);

    // Add some rows
    table->addRow({Value("1", true), Value("Alice", false), Value("30", true), Value("50000", true), Value("Engineering", false)});
    table->addRow({Value("2", true), Value("Bob", false), Value("25", true), Value("45000", true), Value("Marketing", false)});
    table->addRow({Value("3", true), Value("Charlie", false), Value("35", true), Value("60000", true), Value("Engineering", false)});
    table->addRow({Value("4", true), Value("Diana", false), Value("28", true), Value("55000", true), Value("Sales", false)});
    table->addRow({Value("5", true), Value("Eve", false), Value("32", true), Value("52000", true), Value("Marketing", false)});

    return table;
}

// ============ Parser Tests ============

class SqlParserTest : public ::testing::Test {
protected:
    Result<SqlSelectStmtPtr> parse(const std::string& sql) {
        std::istringstream is(sql);
        SqlParser parser(is);
        return parser.parse();
    }
};

TEST_F(SqlParserTest, ParseSelectAll) {
    auto result = parse("SELECT * FROM employees");
    ASSERT_FALSE(result.err());
    EXPECT_TRUE(result.value()->selectAll());
    EXPECT_EQ(result.value()->tableName(), "employees");
    EXPECT_EQ(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectColumns) {
    auto result = parse("SELECT name, age FROM employees");
    ASSERT_FALSE(result.err());
    EXPECT_FALSE(result.value()->selectAll());
    EXPECT_EQ(result.value()->tableName(), "employees");
    ASSERT_EQ(result.value()->columns().size(), 2u);
    EXPECT_EQ(result.value()->columns()[0], "name");
    EXPECT_EQ(result.value()->columns()[1], "age");
}

TEST_F(SqlParserTest, ParseSelectWithWhere) {
    auto result = parse("SELECT * FROM employees WHERE age > 30");
    ASSERT_FALSE(result.err());
    EXPECT_TRUE(result.value()->selectAll());
    EXPECT_EQ(result.value()->tableName(), "employees");
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithAndCondition) {
    auto result = parse("SELECT name FROM employees WHERE age > 25 AND salary < 55000");
    ASSERT_FALSE(result.err());
    EXPECT_FALSE(result.value()->selectAll());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithOrCondition) {
    auto result = parse("SELECT * FROM employees WHERE age < 26 OR age > 34");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectCaseInsensitive) {
    auto result = parse("select * from employees where age > 30");
    ASSERT_FALSE(result.err());
    EXPECT_TRUE(result.value()->selectAll());
    EXPECT_EQ(result.value()->tableName(), "employees");
}

TEST_F(SqlParserTest, ParseSelectWithStringLiteral) {
    auto result = parse("SELECT * FROM employees WHERE name = \"Alice\"");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithParentheses) {
    auto result = parse("SELECT * FROM employees WHERE (age > 30 AND salary > 50000) OR department = \"Sales\"");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithArithmetic) {
    auto result = parse("SELECT * FROM employees WHERE age > (20 + 5)");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithComplexArithmetic) {
    auto result = parse("SELECT * FROM employees WHERE salary > (age * 1000 + 10000)");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseSelectWithArithmeticBothSides) {
    auto result = parse("SELECT * FROM employees WHERE (age + 5) > (id * 10)");
    ASSERT_FALSE(result.err());
    EXPECT_NE(result.value()->whereClause(), nullptr);
}

TEST_F(SqlParserTest, ParseError_MissingFrom) {
    auto result = parse("SELECT * employees");
    EXPECT_TRUE(result.err());
}

TEST_F(SqlParserTest, ParseError_MissingTable) {
    auto result = parse("SELECT * FROM");
    EXPECT_TRUE(result.err());
}

TEST_F(SqlParserTest, ParseError_InvalidColumn) {
    auto result = parse("SELECT 123 FROM employees");
    EXPECT_TRUE(result.err());
}

// ============ Eval Tests ============

class SqlEvalTest : public ::testing::Test {
protected:
    void SetUp() override {
        table_ = createEmployeeTable();
    }

    Result<QueryResultPtr> execute(const std::string& sql) {
        std::istringstream is(sql);
        SqlParser parser(is);
        auto parseResult = parser.parse();
        if (parseResult.err()) {
            return parseResult.err();
        }
        SqlEval eval(table_);
        parseResult.value()->accept(eval);
        return eval.result();
    }

    TablePtr table_;
};

TEST_F(SqlEvalTest, SelectAll) {
    auto result = execute("SELECT * FROM employees");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->columnCount(), 5u);
    EXPECT_EQ(result.value()->rowCount(), 5u);
}

TEST_F(SqlEvalTest, SelectSpecificColumns) {
    auto result = execute("SELECT name, age FROM employees");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->columnCount(), 2u);
    EXPECT_EQ(result.value()->rowCount(), 5u);
    EXPECT_EQ(result.value()->columns()[0].name, "name");
    EXPECT_EQ(result.value()->columns()[1].name, "age");
}

TEST_F(SqlEvalTest, SelectWithWhereEquals) {
    auto result = execute("SELECT * FROM employees WHERE name = \"Alice\"");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 1u);
    EXPECT_EQ(result.value()->rows()[0][1].data, "Alice");
}

TEST_F(SqlEvalTest, SelectWithWhereGreaterThan) {
    auto result = execute("SELECT name FROM employees WHERE age > 30");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u); // Charlie (35) and Eve (32)
}

TEST_F(SqlEvalTest, SelectWithWhereLessThan) {
    auto result = execute("SELECT name FROM employees WHERE age < 30");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u); // Bob (25) and Diana (28)
}

TEST_F(SqlEvalTest, SelectWithWhereAnd) {
    auto result = execute("SELECT name FROM employees WHERE age > 25 AND salary < 55000");
    ASSERT_FALSE(result.err());
    // Alice (30, 50000), Diana (28, 55000 - not included), Eve (32, 52000)
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

TEST_F(SqlEvalTest, SelectWithWhereOr) {
    auto result = execute("SELECT name FROM employees WHERE age < 26 OR age > 34");
    ASSERT_FALSE(result.err());
    // Bob (25), Charlie (35)
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

TEST_F(SqlEvalTest, SelectWithWhereStringEquals) {
    auto result = execute("SELECT name FROM employees WHERE department = \"Engineering\"");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u); // Alice and Charlie
}

TEST_F(SqlEvalTest, SelectWithWhereNotEquals) {
    auto result = execute("SELECT name FROM employees WHERE department <> \"Engineering\"");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 3u); // Bob, Diana, Eve
}

TEST_F(SqlEvalTest, SelectWithComplexWhere) {
    auto result = execute("SELECT name FROM employees WHERE (age > 30 AND salary > 50000) OR department = \"Sales\"");
    ASSERT_FALSE(result.err());
    // Charlie (35, 60000), Eve (32, 52000), Diana (Sales)
    EXPECT_EQ(result.value()->rowCount(), 3u);
}

TEST_F(SqlEvalTest, SelectNoMatchingRows) {
    auto result = execute("SELECT * FROM employees WHERE age > 100");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 0u);
}

TEST_F(SqlEvalTest, ErrorTableNotFound) {
    auto result = execute("SELECT * FROM nonexistent");
    EXPECT_TRUE(result.err());
}

TEST_F(SqlEvalTest, ErrorColumnNotFound) {
    auto result = execute("SELECT nonexistent FROM employees");
    EXPECT_TRUE(result.err());
}

TEST_F(SqlEvalTest, SelectWithNegativeNumber) {
    auto result = execute("SELECT * FROM employees WHERE salary > -1");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 5u);
}

TEST_F(SqlEvalTest, SelectWithGreaterEquals) {
    auto result = execute("SELECT name FROM employees WHERE age >= 30");
    ASSERT_FALSE(result.err());
    // Alice (30), Charlie (35), Eve (32)
    EXPECT_EQ(result.value()->rowCount(), 3u);
}

TEST_F(SqlEvalTest, SelectWithLessEquals) {
    auto result = execute("SELECT name FROM employees WHERE age <= 28");
    ASSERT_FALSE(result.err());
    // Bob (25), Diana (28)
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

// ============ Complex Arithmetic Expression Tests ============

TEST_F(SqlEvalTest, ArithmeticInWhere_Addition) {
    // age > 20 + 5 means age > 25, so Bob (25) is not included
    auto result = execute("SELECT name FROM employees WHERE age > 20 + 5");
    ASSERT_FALSE(result.err());
    // Alice (30), Charlie (35), Diana (28), Eve (32)
    EXPECT_EQ(result.value()->rowCount(), 4u);
}

TEST_F(SqlEvalTest, ArithmeticInWhere_Subtraction) {
    // age < 35 - 5 means age < 30
    auto result = execute("SELECT name FROM employees WHERE age < 35 - 5");
    ASSERT_FALSE(result.err());
    // Bob (25), Diana (28)
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

TEST_F(SqlEvalTest, ArithmeticInWhere_Multiplication) {
    // salary > 50 * 1000 means salary > 50000
    auto result = execute("SELECT name FROM employees WHERE salary > 50 * 1000");
    ASSERT_FALSE(result.err());
    // Charlie (60000), Diana (55000), Eve (52000)
    EXPECT_EQ(result.value()->rowCount(), 3u);
}

TEST_F(SqlEvalTest, ArithmeticInWhere_Division) {
    // salary / 1000 > 55 means salary > 55000
    auto result = execute("SELECT name FROM employees WHERE salary / 1000 > 55");
    ASSERT_FALSE(result.err());
    // Charlie (60000)
    EXPECT_EQ(result.value()->rowCount(), 1u);
}

TEST_F(SqlEvalTest, ArithmeticInWhere_Parentheses) {
    // (age + 5) * 2 > 70 means age + 5 > 35, so age > 30
    auto result = execute("SELECT name FROM employees WHERE (age + 5) * 2 > 70");
    ASSERT_FALSE(result.err());
    // Charlie (35), Eve (32)
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

TEST_F(SqlEvalTest, ArithmeticInWhere_ColumnOnBothSides) {
    // salary > age * 1500 + 5000
    // Alice: 50000 > 30*1500+5000 = 50000 > 50000 = false
    // Bob: 45000 > 25*1500+5000 = 45000 > 42500 = true
    // Charlie: 60000 > 35*1500+5000 = 60000 > 57500 = true
    // Diana: 55000 > 28*1500+5000 = 55000 > 47000 = true
    // Eve: 52000 > 32*1500+5000 = 52000 > 53000 = false
    auto result = execute("SELECT name FROM employees WHERE salary > age * 1500 + 5000");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 3u); // Bob, Charlie, Diana
}

TEST_F(SqlEvalTest, ArithmeticInWhere_ComplexExpression) {
    // (salary - 40000) / 10000 >= age / 10
    // Alice: (50000-40000)/10000 >= 30/10 -> 1 >= 3 = false
    // Bob: (45000-40000)/10000 >= 25/10 -> 0.5 >= 2.5 = false
    // Charlie: (60000-40000)/10000 >= 35/10 -> 2 >= 3.5 = false
    // Diana: (55000-40000)/10000 >= 28/10 -> 1.5 >= 2.8 = false
    // Eve: (52000-40000)/10000 >= 32/10 -> 1.2 >= 3.2 = false
    auto result = execute("SELECT name FROM employees WHERE (salary - 40000) / 10000 >= age / 10");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 0u);
}

TEST_F(SqlEvalTest, ArithmeticWithLogicalOperators) {
    // (age > 25 + 3) AND (salary < 55 * 1000)
    // age > 28 AND salary < 55000
    // Alice: 30 > 28 AND 50000 < 55000 = true
    // Bob: 25 > 28 = false
    // Charlie: 35 > 28 AND 60000 < 55000 = false
    // Diana: 28 > 28 = false
    // Eve: 32 > 28 AND 52000 < 55000 = true
    auto result = execute("SELECT name FROM employees WHERE age > 25 + 3 AND salary < 55 * 1000");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u); // Alice, Eve
}

TEST_F(SqlEvalTest, ArithmeticNestedParentheses) {
    // ((age - 20) * 2 + 10) > 30
    // Alice: ((30-20)*2+10) = 30 > 30 = false
    // Bob: ((25-20)*2+10) = 20 > 30 = false
    // Charlie: ((35-20)*2+10) = 40 > 30 = true
    // Diana: ((28-20)*2+10) = 26 > 30 = false
    // Eve: ((32-20)*2+10) = 34 > 30 = true
    auto result = execute("SELECT name FROM employees WHERE ((age - 20) * 2 + 10) > 30");
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u); // Charlie, Eve
}

// ============ Integration Tests ============

TEST(SqlIntegrationTest, EmptyTable) {
    auto empty = std::make_shared<Table>("empty");
    empty->addColumn("id", true);
    empty->addColumn("name", false);

    std::istringstream is("SELECT * FROM empty");
    SqlParser parser(is);
    auto stmt = parser.parse();
    ASSERT_FALSE(stmt.err());

    SqlEval eval(empty);
    stmt.value()->accept(eval);
    auto result = eval.result();
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->columnCount(), 2u);
    EXPECT_EQ(result.value()->rowCount(), 0u);
}

TEST(SqlIntegrationTest, SingleColumnTable) {
    auto single = std::make_shared<Table>("single");
    single->addColumn("value", true);
    single->addRow({Value("1", true)});
    single->addRow({Value("2", true)});
    single->addRow({Value("3", true)});

    std::istringstream is("SELECT value FROM single WHERE value > 1");
    SqlParser parser(is);
    auto stmt = parser.parse();
    ASSERT_FALSE(stmt.err());

    SqlEval eval(single);
    stmt.value()->accept(eval);
    auto result = eval.result();
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u);
}

TEST(SqlIntegrationTest, ArithmeticWithSingleColumn) {
    auto single = std::make_shared<Table>("numbers");
    single->addColumn("x", true);
    single->addRow({Value("10", true)});
    single->addRow({Value("20", true)});
    single->addRow({Value("30", true)});

    // x * 2 > 35, so x > 17.5 -> only 20 and 30 match
    std::istringstream is("SELECT x FROM numbers WHERE x * 2 > 35");
    SqlParser parser(is);
    auto stmt = parser.parse();
    ASSERT_FALSE(stmt.err());

    SqlEval eval(single);
    stmt.value()->accept(eval);
    auto result = eval.result();
    ASSERT_FALSE(result.err());
    EXPECT_EQ(result.value()->rowCount(), 2u);
}
