#ifndef BASEPLOTDATA_H
#define BASEPLOTDATA_H

#include "../../include/UserVector.h"
#include <optional>
#include <variant>


struct CandleInfo
{
    float open;
    float high;
    float low;
    float close;
};


struct UnderMouseData
{

    std::variant<CandleInfo, double> yData;
    bool m_isUnderMouse;

    bool isCandle() const { return std::holds_alternative<CandleInfo>(yData); }
    const CandleInfo& getCandleInfo() const { return std::get<CandleInfo>(yData); };
    const double& getYData() const { return std::get<double>(yData); };
    bool isUnderMouse() const { return m_isUnderMouse; };
};


class BasePlotData
{
public:

    ~BasePlotData() {};

    BasePlotData(const BasePlotData&) = delete;
    BasePlotData& operator=(const BasePlotData&) = delete;
    BasePlotData(BasePlotData&&) = delete;
    BasePlotData& operator=(BasePlotData&&) = delete;

    double getDelta() const {return m_delta; };
    int getNumDatapoints() const { return m_numDataPoints; };

    virtual std::optional<UnderMouseData> getDataUnderMouse(
        int xIdx, double yMousePos, double yPadding, bool alwaysShow, std::optional<double> xMousePos = std::nullopt
    ) const = 0;

protected:
    BasePlotData() = default;

    int m_numDataPoints;
    double m_delta;
};


class OneValueData : public BasePlotData
{
public:
    virtual const StdPtrVector<float>& getYData() const = 0;
    virtual const StdPtrVector<float>& getMinVector() const = 0;
    virtual const StdPtrVector<float>& getMaxVector() const = 0;
};


class FourValueData : public BasePlotData
{

public:
    virtual const StdPtrVector<float>& getMinVector() const = 0;
    virtual const StdPtrVector<float>& getMaxVector() const = 0;
};



#endif
