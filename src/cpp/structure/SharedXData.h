#ifndef SHAREDXDATA_H
#define SHAREDXDATA_H

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <variant>
#include "../include/Plotter.h"


enum class DateType
{
    NoDate,
    String,
    Timepoint
};


struct TimePointHash {
    std::size_t operator()(const std::chrono::system_clock::time_point& tp) const {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      tp.time_since_epoch()).count();
        return std::hash<std::int64_t>{}(ms);
    }
};


struct TimePointEqual {
    bool operator()(const std::chrono::system_clock::time_point& a,
                    const std::chrono::system_clock::time_point& b) const {
        return a == b;
    }
};


using TimepointMap = std::unordered_map<std::chrono::system_clock::time_point, int, TimePointHash, TimePointEqual>;


class SharedXData
{

public:
    SharedXData();

    SharedXData(const SharedXData&) = delete;
    SharedXData& operator=(const SharedXData&) = delete;
    SharedXData(SharedXData&&) = delete;
    SharedXData& operator=(SharedXData&&) = delete;

    std::string getXTickLabelStr(int tickIndex);
    std::vector<std::string> getXTickLabelDatetime(std::vector<int> allTickLabels);
    std::string getSingleFormattedLabel(int tickIndex);

    std::vector<int> convertDateToIndex(const std::vector<std::string>& stringVector) const;
    std::vector<int> convertDateToIndex(const std::vector<std::chrono::system_clock::time_point>& timeVector) const;

    DateType getDateType();

    void handleNewXDataVector(DateVector xData);

private:

    // The user can pass m_xData (string labels) by reference or it can be
    // generated on the fly from an int array of indicies. In this case, we
    // store the array and then reference it in m_xData.
//    std::vector<std::string> m_xTickLabels{};
    std::optional<
        std::variant<
            StringVectorRef,
            TimepointVectorRef
        >
    > m_xData = std::nullopt;

    // Hash for quick comparison of new dates and quick conversion of
    // scatterplot date to index. This requires a copy of all date +
    // holding the integer, but it is a necessary evil. When combining
    // multiple plots with different x-axis, this hashing will be required.
    std::variant<
        std::unordered_map<std::string, int>,
        TimepointMap
    > m_dateIndex;
};

#endif // SHAREDXDATA_H
