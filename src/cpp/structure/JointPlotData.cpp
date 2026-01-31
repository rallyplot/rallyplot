#include "JointPlotData.h"
#include "LinkedSubplot.h"
#include "../charts/plots/BasePlot.h"
#include "../charts/plots/CandlestickPlot.h"
#include "../charts/Camera.h"
#include "../charts/plots/ScatterPlot.h"


JointPlotData::JointPlotData(const LinkedSubplot& subplot)
    : m_sp(subplot),
    m_minVector(m_computedMinVector.data(), m_computedMinVector.size()),
    m_maxVector(m_computedMaxVector.data(), m_computedMaxVector.size())
{}


void JointPlotData::addPlot(std::unique_ptr<BasePlot> plot)
/*
    Add a plot. As it is enforced all x-axis
    must be exactly the same as the axis is shared
    on all plots, this is checked here.
 */
{
    if (!isEmpty())
    {
        if (!dynamic_cast<ScatterPlot*>(plot.get()) && plot->getNumDatapoints() != getNumDatapoints())
        {
            std::cerr << "CRITICAL ERROR: The added plot has " << plot->getNumDatapoints()
                      << " datapoints, but all plots that are not scatterplots must have the"
                         " same number of datapoints. The current number of datapoints on the"
                         "plot is " << plot->getNumDatapoints() << "." << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    m_plotVector.push_back(std::move(plot));

    recomputeMinMax();
}


MinMaxVectorType JointPlotData::getMinMaxVector(const std::unique_ptr<BasePlot>& plot)
{
    if (const auto& castPlot = dynamic_cast<OneValuePlot*>(plot.get()))
    {
        return { castPlot->getMinVector(), castPlot->getMaxVector()} ;
    }
    else if (const auto& castPlot = dynamic_cast<FourValuePlot*>(plot.get()))
    {
        return {castPlot->getMinVector(), castPlot->getMaxVector()};
    }
    else
    {
        throw std::invalid_argument("Only OneValuePlot or FourValuePlot are valid types.");
    }
};



void JointPlotData::recomputeMinMax()
/*
    The min / max value across all shared plots as separate vectors `m_computedMinVector`, `m_computedMaxVector`.
    This is used to pin the y-axis to the min / max value shown on the graph.
    When a new plot is added, we need to recompute these vectors.
 */
{
    MinMaxVectorType firstPlotMinMax = getMinMaxVector(m_plotVector[0]);

    if (m_plotVector.size() == 1)
    {
        m_minVector = firstPlotMinMax.first;
        m_maxVector = firstPlotMinMax.second;
    }
    else
    {
        int numDataPoints = m_plotVector[0]->getNumDatapoints();

        m_computedMinVector.resize(numDataPoints);
        m_computedMaxVector.resize(numDataPoints);


        if (dynamic_cast<ScatterPlot*>(m_plotVector[0].get()))
        {
            std::cerr << "CRITICAL ERROR: first plot in m_plotVector cannot be a scatterlot." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        const StdPtrVector<float>& firstMin = firstPlotMinMax.first;
        const StdPtrVector<float>& firstMax =  firstPlotMinMax.second;

        for (int i = 0; i < numDataPoints; i++)
        {
            m_computedMinVector[i] = firstMin[i];
            m_computedMaxVector[i] = firstMax[i];
        }

        // Update computedMin/Max vectors based on the values of all other plots.
        for (int plotIdx = 1; plotIdx < m_plotVector.size(); plotIdx++)
        {
            // If we have a scatterplot, just iterate through the values of the scatterplot
            // and match the index to the value-to-update as required
            if (ScatterPlot* scatterPlot = dynamic_cast<ScatterPlot*>(m_plotVector[plotIdx].get()))
            {
                const StdPtrVector<int>& xData = scatterPlot->getPlotData().getXData();
                const StdPtrVector<float>& yData = scatterPlot->getPlotData().getYData();

                for (int i = 0; i < xData.size(); i++)
                {
                    int scatterXVal = xData[i];

                    if (yData[i] < m_computedMinVector[scatterXVal] || std::isnan(m_computedMinVector[scatterXVal]))
                    {
                        m_computedMinVector[scatterXVal] = yData[i];
                    }
                    if (yData[i] > m_computedMaxVector[scatterXVal]|| std::isnan(m_computedMaxVector[scatterXVal]))
                    {
                        m_computedMaxVector[scatterXVal] = yData[i];
                    }
                }
            }
            // Otherwise, iterate and check every datapoint in the graph
            else
            {
                MinMaxVectorType minMaxVectors = getMinMaxVector(m_plotVector[plotIdx]);

                const StdPtrVector<float>& minVector = minMaxVectors.first;
                const StdPtrVector<float>& maxVector = minMaxVectors.second;

                for (int i = 0; i < numDataPoints; i++)
                {
                    if (minVector[i] < m_computedMinVector[i] || std::isnan(m_computedMinVector[i]))
                    {
                        m_computedMinVector[i] = minVector[i];
                    }
                    if (maxVector[i] > m_computedMaxVector[i] || std::isnan(m_computedMaxVector[i]))
                    {
                        m_computedMaxVector[i] = maxVector[i];
                    }
                }
            }
        }

        m_minVector =  StdPtrVector<float>(m_computedMinVector.data(), m_computedMinVector.size());
        m_maxVector = StdPtrVector<float>(m_computedMaxVector.data(), m_computedMaxVector.size());
    }

    m_minValue = (double)*std::min_element(m_minVector.begin(), m_minVector.end());
    m_maxValue = (double)*std::max_element(m_maxVector.begin(), m_maxVector.end());
}


void JointPlotData::draw(glm::mat4& NDCMatrix, Camera& camera)
/*
    Draw every plot attached to the JointPlotData.
 */
{
    if (m_plotVector.empty())
    {
        return;
    }

    // For some reason (I think related to GL_DEPTH_TEST) the plot
    // order must be reversed for plots at the end of the vector
    // to appear on top of plots at the start.
    for (int i = m_plotVector.size() - 1; i >= 0; i--)
    {
        m_plotVector[i]->draw(NDCMatrix, camera);
    }
}


/* --------------------------------------------------------------
    Getters
 --------------------------------------------------------------*/

double JointPlotData::getDelta() const
{
    if (isEmpty())
    {
        return 0.0;
    }
    else
    {
        return m_plotVector[0]->getDelta();
    }
}


int JointPlotData::getNumDatapoints() const
{
    if (isEmpty())
    {
        return 0;
    }
    else
    {
        return m_plotVector[0]->getNumDatapoints();
    }
}

void JointPlotData::cycleCandlestickPlotType()
/*
    Cycle between full candles, no-cap candles, etc.
 */
{
    for (const std::unique_ptr<BasePlot>& plot : m_plotVector)
    {
        if (CandlestickPlot* candlestickPlot = dynamic_cast<CandlestickPlot*>(plot.get()))
        {
            candlestickPlot->cyclePlotType();
        }
    }
}


double JointPlotData::getDataMinY() const
{
    return m_minValue;
}


double JointPlotData::getDataMaxY() const
{
    return m_maxValue;
}


double JointPlotData::getDataMinX() const
/*
    This is always 0.0 now, as the x-axis
    representation is forced between 0 and 1.
 */
{
    return 0.0;
}


double JointPlotData::getDataMaxX() const
/*
    x-axis data goes from 0 to 1 upper
    exclusive (so 0 to 1-delta).
 */
{
    if (m_plotVector.empty())
    {
        return 0.0;
    }
    return 1.0 - getDelta();

}


std::tuple<double, double> JointPlotData::getMinMaxInViewRange(double leftBorder, double rightBorder) const
{
    int startIdx = (int)std::floor(leftBorder / getDelta());
    int endIdx = (int)std::floor(rightBorder / getDelta());

    if (startIdx < 0)
        startIdx = 0;

    if (startIdx >= endIdx)
    {
        return {0.0, 0.0};
    }

    if (endIdx > m_plotVector[0]->getNumDatapoints())
    {
        endIdx = m_plotVector[0]->getNumDatapoints();
    }

    double min, max;

    // This is not very well optimised, if the first plot is not candlestick then
    // m_minVector == m_maxVector (it is just the data) and we could use std::min_max_element
    auto minValue = std::min_element(m_minVector.begin() + startIdx, m_minVector.begin() + endIdx);
    auto maxValue = std::max_element(m_maxVector.begin() + startIdx, m_maxVector.begin() + endIdx);

    min = (double)*minValue;
    max = (double)*maxValue;

    return {min, max};
}
