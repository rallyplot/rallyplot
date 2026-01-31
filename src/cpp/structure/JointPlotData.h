#ifndef JOINTPLOTDATA_H
#define JOINTPLOTDATA_H

#include "../charts/plots/BasePlot.h"
#include "../charts/Camera.h"

using MinMaxVectorType = const std::pair<const StdPtrVector<float>&, const StdPtrVector<float>&>;

class LinkedSubplot;  // forward declaration

class JointPlotData
/*
    This class coordinates the all plots shown on a subplot.
    It maintains a vector m_plotVector of currently displayed plots.
    All plots share an x-axis data and are required to have matching
    x-axes when passed.

    Getters operate over all plots associated with the JointPlotData.
    For example, if there are 5 plots displayed, getDataMaxY will get
    the maximum Y value out of all 5 plots.
 */
{

public:
    JointPlotData(const LinkedSubplot& m_sp);

    JointPlotData(const JointPlotData&) = delete;
    JointPlotData& operator=(const JointPlotData&) = delete;
    JointPlotData(JointPlotData&&) = delete;
    JointPlotData& operator=(JointPlotData&&) = delete;

    const std::vector<std::unique_ptr<BasePlot>>& plotVector() const { return m_plotVector; };
    void addPlot(std::unique_ptr<BasePlot> plot);

    bool isEmpty() const { return m_plotVector.empty(); }
    int numPlots() const { return m_plotVector.size(); }

    void draw(glm::mat4& NDCMatrix, Camera& camera);

    int getNumDatapoints() const;
    double getDelta() const;

    double getDataMinY() const;
    double getDataMaxY() const;
    double getDataMinX() const;
    double getDataMaxX() const;

    std::tuple<double, double> getMinMaxInViewRange(double leftBorder, double rightBorder) const;

    void cycleCandlestickPlotType();


private:

    std::vector<std::unique_ptr<BasePlot>> m_plotVector;
    const LinkedSubplot& m_sp;
    // If we have only one plot, the min / max vector is just the
    // plot data itself. Otherwise, we need to compute this if we
    // are pinning the y-axis.
    std::vector<float> m_computedMinVector{};
    std::vector<float> m_computedMaxVector{};

    StdPtrVector<float> m_minVector;
    StdPtrVector<float> m_maxVector;

    double m_minValue = 0;
    double m_maxValue = 0;

    void recomputeMinMax();
    MinMaxVectorType getMinMaxVector(const std::unique_ptr<BasePlot>& plot);
};

#endif
