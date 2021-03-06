#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "cron.hpp"

class CronSuite : public testing::TestWithParam<std::tuple<std::string, std::string, std::string>> {
    protected:
    using time_point = std::chrono::system_clock::time_point;

    static auto TimePoint(const std::string& date, const std::string& fmt="%y-%m-%d %H:%M:%S") {
        std::tm tm = {};
        std::stringstream ss(date);
        ss >> std::get_time(&tm, fmt.data());
        if (ss.fail()) {
            throw std::runtime_error("invalid date ...");
        }
        tm.tm_isdst = -1;
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));

    }
    static auto Datatime(const time_point& time, const std::string& fmt="%y-%m-%d %H:%M:%S") {
        auto tt = std::chrono::system_clock::to_time_t(time);
        auto tm = *std::localtime(&tt);
        auto ss = std::stringstream{};
        ss << std::put_time(&tm, fmt.data());
        return ss.str();
    }
};


TEST_P(CronSuite, overview_test) {
    auto& [expr, beg, end] = GetParam();
    EXPECT_EQ(TimePoint(beg) + Cron::Build(expr), TimePoint(end));
}

INSTANTIATE_TEST_SUITE_P(Cron, CronSuite, testing::Values(
    std::tuple{"*/15 * 1-4 * * *",  "2012-07-01 09:53:50", "2012-07-02 01:00:00"},
    std::tuple{"*/15 * 1-4 * * *",  "2012-07-01 09:53:00", "2012-07-02 01:00:00"},
    std::tuple{"0 */2 1-4 * * *",   "2012-07-01 09:00:00", "2012-07-02 01:00:00"},
    std::tuple{"* * * * * *",       "2012-07-01 09:00:00", "2012-07-01 09:00:01"},
    std::tuple{"* * * * * *",       "2012-12-01 09:00:58", "2012-12-01 09:00:59"},
    std::tuple{"10 * * * * *",      "2012-12-01 09:42:09", "2012-12-01 09:42:10"},
    std::tuple{"11 * * * * *",      "2012-12-01 09:42:10", "2012-12-01 09:42:11"},
    std::tuple{"10 * * * * *",      "2012-12-01 09:42:10", "2012-12-01 09:43:10"},
    std::tuple{"10-15 * * * * *",   "2012-12-01 09:42:09", "2012-12-01 09:42:10"},
    std::tuple{"10-15 * * * * *",   "2012-12-01 21:42:14", "2012-12-01 21:42:15"},
    std::tuple{"0 * * * * *",       "2012-12-01 21:10:42", "2012-12-01 21:11:00"},
    std::tuple{"0 * * * * *",       "2012-12-01 21:11:00", "2012-12-01 21:12:00"},
    std::tuple{"0 11 * * * *",      "2012-12-01 21:10:42", "2012-12-01 21:11:00"},
    std::tuple{"0 10 * * * *",      "2012-12-01 21:11:00", "2012-12-01 22:10:00"},
    std::tuple{"0 0 * * * *",       "2012-09-30 11:01:00", "2012-09-30 12:00:00"},
    std::tuple{"0 0 * * * *",       "2012-09-30 12:00:00", "2012-09-30 13:00:00"},
    std::tuple{"0 0 * * * *",       "2012-09-10 23:01:00", "2012-09-11 00:00:00"},
    std::tuple{"0 0 * * * *",       "2012-09-11 00:00:00", "2012-09-11 01:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-09-01 14:42:43", "2012-09-02 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-09-02 00:00:00", "2012-09-03 00:00:00"},
    std::tuple{"* * * 10 * *",      "2012-10-09 15:12:42", "2012-10-10 00:00:00"},
    std::tuple{"* * * 10 * *",      "2012-10-11 15:12:42", "2012-11-10 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-09-30 15:12:42", "2012-10-01 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-10-01 00:00:00", "2012-10-02 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-08-30 15:12:42", "2012-08-31 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-08-31 00:00:00", "2012-09-01 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-10-30 15:12:42", "2012-10-31 00:00:00"},
    std::tuple{"0 0 0 * * *",       "2012-10-31 00:00:00", "2012-11-01 00:00:00"},
    std::tuple{"0 0 0 1 * *",       "2012-10-30 15:12:42", "2012-11-01 00:00:00"},
    std::tuple{"0 0 0 1 * *",       "2012-11-01 00:00:00", "2012-12-01 00:00:00"},
    std::tuple{"0 0 0 1 * *",       "2010-12-31 15:12:42", "2011-01-01 00:00:00"},
    std::tuple{"0 0 0 1 * *",       "2011-01-01 00:00:00", "2011-02-01 00:00:00"},
    std::tuple{"0 0 0 31 * *",      "2011-10-30 15:12:42", "2011-10-31 00:00:00"},
    std::tuple{"0 0 0 1 * *",       "2011-10-30 15:12:42", "2011-11-01 00:00:00"},
    std::tuple{"* * * * * 2",       "2010-10-25 15:12:42", "2010-10-26 00:00:00"},
    std::tuple{"* * * * * 2",       "2010-10-20 15:12:42", "2010-10-26 00:00:00"},
    std::tuple{"* * * * * 2",       "2010-10-27 15:12:42", "2010-11-02 00:00:00"},
    std::tuple{"55 5 * * * *",      "2010-10-27 15:04:54", "2010-10-27 15:05:55"},
    std::tuple{"55 5 * * * *",      "2010-10-27 15:05:55", "2010-10-27 16:05:55"},
    std::tuple{"55 * 10 * * *",     "2010-10-27 09:04:54", "2010-10-27 10:00:55"},
    std::tuple{"55 * 10 * * *",     "2010-10-27 10:00:55", "2010-10-27 10:01:55"},
    std::tuple{"* 5 10 * * *",      "2010-10-27 09:04:55", "2010-10-27 10:05:00"},
    std::tuple{"* 5 10 * * *",      "2010-10-27 10:05:00", "2010-10-27 10:05:01"},
    std::tuple{"55 * * 3 * *",      "2010-10-02 10:05:54", "2010-10-03 00:00:55"},
    std::tuple{"55 * * 3 * *",      "2010-10-03 00:00:55", "2010-10-03 00:01:55"},
    std::tuple{"* * * 3 11 *",      "2010-10-02 14:42:55", "2010-11-03 00:00:00"},
    std::tuple{"* * * 3 11 *",      "2010-11-03 00:00:00", "2010-11-03 00:00:01"},
    std::tuple{"0 0 0 29 2 *",      "2007-02-10 14:42:55", "2008-02-29 00:00:00"},
    std::tuple{"0 0 0 29 2 *",      "2008-02-29 00:00:00", "2012-02-29 00:00:00"},
    // std::tuple{"0 0 7 ? * MON-FRI", "2009-09-26 00:42:55", "2009-09-28 07:00:00"},
    // std::tuple{"0 0 7 ? * MON-FRI", "2009-09-28 07:00:00", "2009-09-29 07:00:00"},
    std::tuple{"0 30 23 30 1/3 ?",  "2010-12-30 00:00:00", "2011-01-30 23:30:00"},
    std::tuple{"0 30 23 30 1/3 ?",  "2011-01-30 23:30:00", "2011-04-30 23:30:00"},
    std::tuple{"0 30 23 30 1/3 ?",  "2011-04-30 23:30:00", "2011-07-30 23:30:00"}
    ));
