/// ===============================================================================================
/// @file      : cron.cpp                                                    __|   __|  _ \   __ \ 
/// @copyright : 2019 LCMonteiro                                            (     |    (   |  |   |
///                                                                        \___| _|   \___/  _|  _|
/// @author    : Luis Monteiro                                                                     
/// ===============================================================================================

#include "cron.hpp"

#include <regex>

namespace Cron {
    ///
    /// WordDelimitedBy
    /// @brief
    ///
    template <char delimiter>
    class WordDelimitedBy : public std::string {};
    template <char delimiter>
    std::istream& operator>>(std::istream& is, WordDelimitedBy<delimiter>& output) {
        while (std::getline(is, output, delimiter) && output.empty()) {}
        return is;
    }

    ///
    /// Build cron space
    /// @param exp cron expression
    /// @return cron space
     ///
    Space Build(const std::string& exp) {
        // settings: <offset, minimum, maximum>
        using Settings = std::tuple<int, int, int>;
        // space settings
        const std::array<Settings, 6> settings{
            Settings{0, 0, 59},
            Settings{0, 0, 59},
            Settings{0, 0, 23},
            Settings{0, 1, 31},
            Settings{1, 0, 11},
            Settings{0, 0, 6}};
        // output space
        Space space;
        // split expression string
        std::istringstream iss(exp);
        std::vector<std::string> fields;
        std::copy(
            std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(fields));
        if (fields.size() > settings.size())
            throw BadExpression(
                "[check cron expression] expression must have less than six fields");
        // create cron table
        std::transform(
            std::begin(fields),
            std::end(fields),
            std::begin(settings),
            std::back_inserter(space),
            [](const auto& field, const auto& settings) {
                // helper for checking and correcting the value
                auto assert_value = [&field, &settings](auto val) {
                    val -= std::get<0>(settings);
                    if (val < std::get<1>(settings) || val > std::get<2>(settings))
                        throw BadExpression(
                            "[check cron expression] field out of bounds('" + field + "')");
                    return val;
                };
                // output value set
                std::set<int> set;
                // parse sequencies
                std::istringstream iss(field);
                std::vector<std::string> parts;
                std::copy(
                    std::istream_iterator<WordDelimitedBy<','>>(iss),
                    std::istream_iterator<WordDelimitedBy<','>>(),
                    std::back_inserter(parts));
                for (auto& part : parts) {
                    std::smatch sm;
                    if (std::regex_match(part, sm, std::regex("([0-9]+)"))) {
                        set.emplace(assert_value(std::stoi(sm.str(1))));
                        continue;
                    }
                    if (std::regex_match(part, sm, std::regex("(\\*+|\\?+)"))) {
                        auto beg = std::get<1>(settings);
                        auto end = std::get<2>(settings);
                        for (auto it = beg; it <= end; ++it)
                            set.emplace(it);
                        continue;
                    }
                    if (std::regex_match(part, sm, std::regex("([0-9]+)-([0-9]+)"))) {
                        auto beg = assert_value(std::stoi(sm.str(1)));
                        auto end = assert_value(std::stoi(sm.str(2)));
                        for (auto it = beg; it <= end; ++it)
                            set.emplace(it);
                        continue;
                    }
                    if (std::regex_match(part, sm, std::regex("(\\*|[0-9]+)/([0-9]+)"))) {
                        auto beg = (sm.str(1) == "*") ? std::get<1>(settings)
                                                      : assert_value(std::stoi(sm.str(1)));
                        auto end = std::get<2>(settings);
                        auto stp = std::stoi(sm.str(2));
                        for (auto it = beg; it < end; it += stp)
                            set.emplace(it);
                        continue;
                    }
                    throw BadExpression(
                        "[check cron expression] wrong field format ('" + field + "')");
                }
                return Field(std::move(set));
            });
        return space;
    }

    ///
    /// Move time through a cron space
    ///
    namespace tool {
        template <typename container_t>
        struct view {
            using const_iterator = typename container_t::const_iterator;

            view(const_iterator& beg, const_iterator& end) : beg_{beg}, end_{end} {}
            view(const container_t& container, size_t beg, size_t end)
              : beg_{std::min(std::next(std::begin(container), beg), std::end(container))},
                end_{std::min(std::next(std::begin(container), end), std::end(container))} {}

            auto begin() const { return beg_; }
            auto end() const { return end_; }
            auto begin(container_t& edit) const {
                return std::next(edit.begin(), std::distance(edit.cbegin(), beg_));
            }
            auto end(container_t& edit) const {
                return std::next(edit.begin(), std::distance(edit.cbegin(), end_));
            }

          private:
            const_iterator beg_;
            const_iterator end_;
        };
        template <typename iterator_t>
        static void reset(iterator_t beg, iterator_t end) {
            for (; beg != end; ++beg)
                beg->point = beg->begin();
        }
        template <typename iterator_t>
        static bool next(iterator_t beg, iterator_t end) {
            for (; beg != end; ++beg) {
                if (++(beg->point) != beg->end())
                    return false;
                beg->point = beg->begin();
            }
            return true;
        }
        template <typename view_t>
        static void reset(const view_t& space) {
            reset(space.begin(), space.end());
        }
        template <typename view_t>
        static bool next(const view_t& space) {
            return next(space.begin(), space.end());
        }
        template <typename cview_t, typename sview_t>
        static bool begin(const cview_t& calendar, const sview_t& space) {
            auto beg = std::make_reverse_iterator(space.end());
            auto end = std::make_reverse_iterator(space.begin());
            auto cit = std::make_reverse_iterator(
                std::next(calendar.begin(), std::distance(space.begin(), space.end())));
            for (auto it = beg; it != end; ++it, ++cit) {
                it->point = it->lower_bound(*cit);
                if (it->point == it->end()) {
                    it->point = it->begin();
                    return next(it.base(), beg.base());
                }
                if (*it->point != *cit) {
                    reset(std::next(it), end);
                    return false;
                }
            }
            return false;
        }
        template <typename cview_t, typename sview_t>
        static bool check(const cview_t& calendar, const sview_t& space) {
            auto cit = calendar.begin();
            auto sit = space.begin();
            for (; sit < space.end(); ++sit, ++cit)
                if (sit->find(*cit) == sit->end())
                    return false;
            return true;
        }
    } // namespace tool

    ///
    /// Calendar
    /// @brief this structure ensures that we always have a valid datetime
    ///
    struct Calendar : std::vector<std::reference_wrapper<int>> {
        explicit Calendar(const std::time_t& time) : tm_{}, tt_{time} {
            std::memcpy(&tm_, std::localtime(&tt_), sizeof(tm_));
            emplace_back(tm_.tm_sec);
            emplace_back(tm_.tm_min);
            emplace_back(tm_.tm_hour);
            emplace_back(tm_.tm_mday);
            emplace_back(tm_.tm_mon);
            emplace_back(tm_.tm_wday);
            tm_.tm_isdst = -1;
        }
        template <typename cview_t, typename sview_t>
        void update(cview_t calendar, sview_t space, bool carrier) {
            auto cit = calendar.begin(*this);
            auto sit = space.begin();
            for (; sit < space.end(); ++sit, ++cit)
                cit->get() = *sit->point;
            if (carrier) {
                if (cit != calendar.end(*this))
                    ++cit->get();
                else
                    ++tm_.tm_year;
            }
            if (tt_ = std::mktime(&tm_); tt_ < 0)
                throw BadExpression("[check cron expression] something went wrong");
        }
        auto time() const { return tt_; }
      private:
        std::tm tm_;
        std::time_t tt_;
    };

    ///
    /// Next
    /// @brief this function privides a next time point based on expression
    /// @param point time
    /// @param location space
    /// @return next time
    ///
    Time Next(const Time& point, const Space& location) {
        // fallback: maximum iterations
        constexpr int MAX_ITERATIONS = 366;
        // create candendar
        auto calendar = Calendar(Clock::to_time_t(point + std::chrono::seconds(1)));
        // search process
        auto calendar_datetime = tool::view<Calendar>(calendar, 0, 5);
        auto location_datetime = tool::view<Space>(location, 0, 5);
        calendar.update(
            calendar_datetime, 
            location_datetime, tool::begin(calendar_datetime, location_datetime));

        if(!tool::check(calendar, location)) {
            auto location_time = tool::view<Space>(location, 0, 3);
            auto calendar_time = tool::view<Calendar>(calendar, 0, 3);
            auto location_date = tool::view<Space>(location, 3, 5);
            auto calendar_date = tool::view<Calendar>(calendar, 3, 5);
            
            calendar.update(calendar_date, location_date, tool::next(location_date));

            for (auto i = 1; !tool::check(calendar, location); ++i) {
                if (i > MAX_ITERATIONS)
                    throw BadExpression("[check cron expression] maximum search iterations reached");
                calendar.update(calendar_date, location_date, tool::next(location_date));
            }
            
            calendar.update(calendar_time, location_time, (tool::reset(location_time), false));
        }
        return Clock::from_time_t(calendar.time());
    }

} // namespace Cron
