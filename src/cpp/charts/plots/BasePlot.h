#ifndef BASEPLOT_H
#define BASEPLOT_H

#include <variant>
#include <QOpenGLFunctions_3_3_Core>
#include "../Camera.h"
#include "CandlestickData.h"
#include "LineData.h"
#include "BasePlotData.h"


struct PlotColor
{
    glm::vec4 color;
};


struct CandlestickColor
{
    glm::vec4 upColor;
    glm::vec4 downColor;
};


using PlotData = std::variant<CandlestickData, LineData>;

class BasePlot
{
public:

    virtual ~BasePlot()  {};

    BasePlot(const BasePlot&) = delete;
    BasePlot& operator=(const BasePlot&) = delete;
    BasePlot(BasePlot&&) = delete;
    BasePlot& operator=(BasePlot&&) = delete;

    virtual void draw(glm::mat4& NDCMatrix, Camera& camera) = 0;

    double getDelta() const { return getPlotData().getDelta(); };
    int getNumDatapoints() const { return getPlotData().getNumDatapoints(); };

    virtual const BasePlotData& getPlotData() const = 0;

protected:

    BasePlot() = default;

};


class OneValuePlot : public BasePlot
{

public:

    virtual const OneValueData& getPlotData() const = 0;

    const StdPtrVector<float>& getYData() const { return getPlotData().getYData(); };

    virtual PlotColor getPlotColor() const = 0;

    const StdPtrVector<float>& getMinVector() const { return getPlotData().getMinVector(); };
    const StdPtrVector<float>& getMaxVector() const { return getPlotData().getMaxVector(); };
};


class FourValuePlot : public BasePlot
{

public:

    virtual const FourValueData& getPlotData() const = 0;

    const StdPtrVector<float>& getMinVector() const { return getPlotData().getMinVector(); };
    const StdPtrVector<float>& getMaxVector() const { return getPlotData().getMaxVector(); };

    virtual CandlestickColor getPlotColor() const = 0;

};


#endif
