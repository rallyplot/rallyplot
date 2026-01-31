#include "SharedXData.h"
#include <iostream>
#include "../../vendor/fmt/include/fmt/core.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <string>
#include "../../vendor/date-master/date/date.h"  // Howard Hinnant's date library


SharedXData::SharedXData() {}


std::string SharedXData::getSingleFormattedLabel(int tickIndex)
/*

*/
{
    using namespace std::chrono;
    using namespace date;

    if (!m_xData.has_value()) {
        return std::to_string(tickIndex);
    }

    if (std::holds_alternative<StringVectorRef>(*m_xData)) {
        const auto& vector = std::get<StringVectorRef>(*m_xData).get();
        if (tickIndex < 0 || tickIndex >= static_cast<int>(vector.size())) {
            return "";
        }
        return vector[tickIndex];
    }

    if (std::holds_alternative<TimepointVectorRef>(*m_xData)) {
        const auto& vector = std::get<TimepointVectorRef>(*m_xData).get();
        if (tickIndex < 0 || tickIndex >= static_cast<int>(vector.size())) {
            return "";
        }

        auto tickTime = vector[tickIndex];
        auto dayPoint = floor<days>(tickTime);
        year_month_day ymd = dayPoint;

        // Day suffix logic (same as before)
        unsigned day = static_cast<unsigned>(ymd.day());
        const char* daySuffix;
        if (day >= 11 && day <= 13) {
            daySuffix = "th";
        } else {
            switch (day % 10) {
            case 1: daySuffix = "st"; break;
            case 2: daySuffix = "nd"; break;
            case 3: daySuffix = "rd"; break;
            default: daySuffix = "th"; break;
            }
        }

        // Format: "July 2024, 14:30:00"
        auto truncated = std::chrono::time_point_cast<std::chrono::seconds>(tickTime);

        std::ostringstream oss;
        oss << day << daySuffix << " "
            << format("%b %Y, %H:%M:%S", truncated);  // %T = HH:MM:SS

        return oss.str();
    }

    return std::to_string(tickIndex);
}


std::vector<std::string> SharedXData::getXTickLabelDatetime(std::vector<int> allTickLabels)
/*

*/
{
    using namespace std::chrono;
    using namespace date;

    std::vector<std::string> labels;

    if (!m_xData.has_value() ||
        !std::holds_alternative<TimepointVectorRef>(*m_xData)) {
        return labels;
    }

    const auto& labelVector = std::get<TimepointVectorRef>(*m_xData).get();
    if (allTickLabels.empty() || allTickLabels.front() < 0 || allTickLabels.back() >= static_cast<int>(labelVector.size())) {
        return labels;
    }

    bool showSeconds = false;
    std::string timeFormat = showSeconds ? "%H:%M:%S" : "%H:%M";
    std::string dateTimeFormat = "%b %d " + timeFormat;

    system_clock::time_point startTime = labelVector[allTickLabels.front()];
    system_clock::time_point endTime = labelVector[allTickLabels.back()];
    auto totalMinutes = std::abs(
        duration_cast<duration<int, std::ratio<60>>>(endTime - startTime).count()
        );
    auto totalDays = std::abs(duration_cast<duration<int, std::ratio<86400>>>(endTime - startTime).count());
    int totalYears = totalDays / 365;
    int tickCount = static_cast<int>(allTickLabels.size());

    enum class Mode { TimeOnly, TimeWithDay, DayWithMonth, MonthWithYear, YearsOnly, subMinute };
    Mode mode;

    if (totalYears > tickCount) {
        mode = Mode::YearsOnly;
    } else if (totalMinutes < tickCount) {
        mode = Mode::subMinute;
    } else if (totalDays < 1) {
        mode = Mode::TimeOnly;
    } else if (totalDays < 7) {
        mode = Mode::TimeWithDay;
    } else if (totalDays < 365) {
        mode = Mode::DayWithMonth;
    } else {
        mode = Mode::MonthWithYear;
    }

    // Protect against invalid access
    if (allTickLabels.size() < 2) return labels;
    auto timeDiff = labelVector[allTickLabels[1]] - labelVector[allTickLabels[0]];

    for (size_t j = 0; j < allTickLabels.size(); ++j)
    {
        int tickIndex = allTickLabels[j];
        if (tickIndex < 0 || tickIndex >= static_cast<int>(labelVector.size())) {
            labels.push_back("");  // Or skip/throw
            continue;
        }

        auto tickTime = labelVector[tickIndex];
        auto nextTickTime = tickTime + timeDiff;

        std::ostringstream oss;

        switch (mode)
        {
        case Mode::subMinute:
            oss << format("%H:%M:%S", floor<seconds>(tickTime));
            break;
        case Mode::TimeOnly:
        case Mode::TimeWithDay:
            if (floor<days>(tickTime) != floor<days>(nextTickTime)) {
                oss << format(dateTimeFormat, floor<seconds>(tickTime));
            } else {
                oss << format(timeFormat, floor<seconds>(tickTime));
            }
            break;

        case Mode::DayWithMonth:
            if (year_month_day{floor<days>(tickTime)}.month() != year_month_day{floor<days>(nextTickTime)}.month() ||
                year_month_day{floor<days>(tickTime)}.year() != year_month_day{floor<days>(nextTickTime)}.year()) {
                oss << format("%b %d", floor<seconds>(tickTime));
            } else {
                oss << format("%d", floor<seconds>(tickTime));
            }
            break;

        case Mode::MonthWithYear:
            if (year_month_day{floor<days>(tickTime)}.year() != year_month_day{floor<days>(nextTickTime)}.year()) {
                oss << format("%Y %b", floor<seconds>(tickTime));
            } else {
                oss << format("%b", floor<seconds>(tickTime));
            }
            break;

        case Mode::YearsOnly:
            oss << format("%Y", floor<seconds>(tickTime));
            break;
        }

        labels.push_back(oss.str());
    }

    return labels;
}


std::string SharedXData::getXTickLabelStr(int tickIndex)
/*

*/
{
    if (!m_xData.has_value())
    {
        return std::to_string(tickIndex);
    }
    else
    {
        StringVectorRef labelVector = std::get<StringVectorRef>(m_xData.value());
        return labelVector.get()[tickIndex];
    }
}


std::vector<int> SharedXData::convertDateToIndex(const std::vector<std::string>& stringVector) const
/*

*/
{
    if (stringVector.empty())
        return {};

    std::vector<int> indexes;
    indexes.reserve(stringVector.size());

    const auto& dateMap = std::get<std::unordered_map<std::string, int>>(m_dateIndex);

    for (const auto& label : stringVector) {

        auto it = dateMap.find(label);

        if (it == dateMap.end())
        {
            std::cerr << "CRITICAL ERROR: Scatterplot date string \""
                      << label << "\" not found in x-axis labels."<< std::endl;
            std::exit(EXIT_FAILURE);
        }
        indexes.push_back(it->second);
    }

    return indexes;
}


std::vector<int> SharedXData::convertDateToIndex(const std::vector<std::chrono::system_clock::time_point>& timeVector) const
/*

*/
{
    if (timeVector.empty())
        return {};

    std::vector<int> indexes;
    indexes.reserve(timeVector.size());

    const auto& dateMap = std::get<TimepointMap>(m_dateIndex);
    for (const auto& tp : timeVector) {
        auto it = dateMap.find(tp);

        if (it == dateMap.end())
        {
            std::cerr << "CRITICAL ERROR: scatterplot datetime Datetime not found in x-axis labels." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        indexes.push_back(it->second);
    }

    return indexes;
}


DateType SharedXData::getDateType()
/*

*/
{
    if (!m_xData.has_value())
    {
        return DateType::NoDate;
    }
    if (std::holds_alternative<TimepointVectorRef>(m_xData.value()))
    {
        return DateType::Timepoint;
    }
    else if (std::holds_alternative<StringVectorRef>(m_xData.value()))
    {
        return DateType::String;
    }

    std::cerr << "CRITICAL ERROR: m_xDAta type not recognised." << std::endl;
    std::exit(EXIT_FAILURE);
}


void SharedXData::handleNewXDataVector(DateVector xData)
/*
 *     Replace existing. This is checked up front.
 */
{
    if (std::holds_alternative<StringVectorRef>(xData))
    {
        if (m_xData.has_value() && std::holds_alternative<TimepointVectorRef>(m_xData.value()))
        {
            std::cerr << "CRITIAL ERROR: plot contains timepoint dates but we are trying to set string. "
                         "This should be caught further up." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        const StringVectorRef data = std::get<StringVectorRef>(xData);

        m_xData = data;

        std::unordered_map<std::string, int> dateIndex;

        for (int i = 0; i < data.get().size(); i++)
        {
            dateIndex[data.get()[i]] = i;
        }
        m_dateIndex = dateIndex;
    }
    else
    {
        if (m_xData.has_value() && std::holds_alternative<StringVectorRef>(m_xData.value()))
        {
            std::cerr << "CRITIAL ERROR: plot contains string dates but we are trying to set timepoint. "
                         "This should be caught further up." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        const TimepointVectorRef data = std::get<TimepointVectorRef>(xData);

        m_xData = data;

        TimepointMap dateIndex;

        for (int i = 0; i < data.get().size(); i++)
        {
            dateIndex[data.get()[i]] = i;
        }
        m_dateIndex = dateIndex;
    }
};
