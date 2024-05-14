#include "GrblParser.h"

#include <string>
#include <tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class GrblParserParameterizedTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, int>>
{
protected:
    std::string getInputString() const
    {
        return std::get<0>(GetParam());
    }

    std::string getExpectedOutputString() const
    {
        return std::get<1>(GetParam());
    }

    int getExpectedCalls() const
    {
        return std::get<2>(GetParam());
    }
};

class MockGrblParser : public GrblParser
{
public:
    MOCK_METHOD(void, processData, (), (override));
    MOCK_METHOD(uint16_t, available, (), (override));
    MOCK_METHOD(char, read, (), (override));
    MOCK_METHOD(void, write, (char c), (override));
};

TEST_P(GrblParserParameterizedTest, data_is_processed_when_newline_is_received)
{
    // ARRANGE
    MockGrblParser grblParser;
    EXPECT_CALL(grblParser, processData()).Times(getExpectedCalls());

    // ACT
    grblParser.encode(getInputString());

    // ASSERT
    ASSERT_EQ(grblParser.data(), getExpectedOutputString());
}

INSTANTIATE_TEST_SUITE_P(encode, GrblParserParameterizedTest,
                         ::testing::Values(
                             std::make_tuple("foo", "foo", 0),
                             std::make_tuple("foo\r\n", "foo\r\n", 1),
                             std::make_tuple("   TEST   \r", "   TEST   \r", 0),
                             std::make_tuple("\r  \n   TEST   \n\r", "\r  \n   TEST   \n\r", 2)));

TEST(encode, accepts_both_lvalue_and_rvalue_string)
{
    // ARRANGE
    MockGrblParser grblParser;
    std::string myFirstData = "My first data\r\n";

    // ACT
    grblParser.encode(myFirstData);
    grblParser.encode("My second data\r\n");

    // ASSERT
    ASSERT_EQ(grblParser.data(), "My first data\r\nMy second data\r\n");
}

TEST(encode, accepts_characters)
{
    // ARRANGE
    MockGrblParser grblParser;
    std::string myData = "My data\r\n";

    // ACT
    for (const auto c : myData)
    {
        grblParser.encode(c);
    }

    // ASSERT
    ASSERT_EQ(grblParser.data(), myData);
}

TEST(processData, processOkAndErrorResponse)
{
    // ARRANGE
    // ACT
    // ASSERT
}
