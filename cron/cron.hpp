/// ===============================================================================================
/// @file      : cron.hpp                                                    __|   __|  _ \   __ \ 
/// @copyright : 2019 LCMonteiro                                            (     |    (   |  |   |
///                                                                        \___| _|   \___/  _|  _|
/// @author    : Luis Monteiro                                                                     
/// ===============================================================================================
#pragma once
                     
#include <chrono>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Cron {
    ///
    /// Object Definitions
    ///
    using Clock = std::chrono::system_clock;
    using Time  = std::chrono::system_clock::time_point;

    ///
    /// Exception for wrong expression
    ///
    struct BadExpression : public std::runtime_error {
      public:
        explicit BadExpression(const std::string& message) : std::runtime_error(message.data()) {}
    };

    ///
    /// Cron space elements
    ///
    struct Field : std::set<int> {
        using base = std::set<int>;
        explicit Field(base&& set) : base(std::forward<base>(set)), point{begin()} {}
        mutable base::iterator point;
    };
    struct Space : std::vector<Field> {};

    ///
    /// Build
    /// @brief this function build a cron space
    ///                             ┌──────────────── second           [0 - 59]
    ///                             │ ┌────────────── minute           [0 - 59]
    ///                             │ │ ┌──────────── hour             [0 - 23]
    ///                             │ │ │ ┌────────── day of the month [1 - 31]
    ///                             │ │ │ │ ┌──────── month            [1 - 12]
    ///                             │ │ │ │ │ ┌────── day of the week  [0 -  6]
    ///                             │ │ │ │ │ │
    /// @param exp cron expression "*/// */// * *"
    /// @return cron space
    ///
    Space Build(const std::string& exp);

    ///
    /// Next
    /// @brief this function privides a next time point based on expression
    /// @param time reference
    /// @param location possible time locations
    /// @return next time
    ///
    Time Next(const Time& point, const Space& location);

    ///
    /// sum operator - wrapper for "Next" function
    ///
    inline Time operator+(const Time& point, const Space& location) {
        return Next(point, location);
    }

} // namespace Cron
