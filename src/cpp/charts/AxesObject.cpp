#define FMT_HEADER_ONLY
#define FMT_UNICODE 0
#include <vector>
#include <cmath>
#include "../structure/LinkedSubplot.h"
#include "../structure/JointPlotData.h"
#include "../../vendor/fmt/include/fmt/core.h"


AxesObject::AxesObject(Configs& configs, SharedXData& sharedXData, LinkedSubplot& subplot, QOpenGLFunctions_3_3_Core& glFunctions)
    :  m_configs(configs),
       m_sharedXData(sharedXData),
       m_linkedSubplot(subplot),
       m_gl(glFunctions),
       m_axesProgram(
          "axes_vertex.shader",
          "axes_fragment.shader",
          m_gl
      ),
      m_axesTickProgram(
          "axes_tick_vertex.shader",
          "axes_fragment.shader",
          m_gl
      ),
      m_axesVertexArray(m_gl),
      m_axesTickVertexArray(m_gl)
{
    m_axesProgram.setupAndBindProgram();
    m_axesTickProgram.setupAndBindProgram();

    // Set up axes vertices. 4 vertices (x, y) for two axis lines. 
    // The axis are defined in NDC coordinates.
    // -------------------------------------------------------

    m_axesData[0] = -1.0f;  m_axesData[1] = -1.0f;   // Left Axis
    m_axesData[2] = -1.0f;  m_axesData[3] = 1.0f;
    
    m_axesData[4] = -1.0f;  m_axesData[5] = -1.0f;   // Bottom Axis
    m_axesData[6] = 1.0f;   m_axesData[7] = -1.0f;

    m_axesData[8] = 1.0f;   m_axesData[9] = -1.0f;   // Right Axis
    m_axesData[10] = 1.0f;  m_axesData[11] = 1.0f;
    
    // Set up generic ticks that are used only as indices that
    // are multiplied by start tick position and tick delta
    // in the shader.
    // -------------------------------------------------------

    int idxCounter = 0;
    for (int i = 0; i < 30; i++) 
    {
        m_genericTicks[idxCounter] = i;       // tick index
        m_genericTicks[idxCounter + 1] = 1;   // top vertex indicator
        
        m_genericTicks[idxCounter + 2] = i;   // tick index
        m_genericTicks[idxCounter + 3] = -1;  // bottom vertex indicator

        idxCounter += 4;
    }

    setupAxesVAO();
    setupTickVAO();

}


AxesObject::~AxesObject()
{
}

/* ----------------------------------------------------------------------------------------------------------
  Setup axes and tick buffers
 ----------------------------------------------------------------------------------------------------------*/


void AxesObject::setupAxesVAO()
/*
    Setup VAO and associated buffers for the axesData vertices.

    The axes buffer has 4 vertices (x, y), each one the corner 
    of the axes (vertices are connected by lines).
*/
{
    m_axesVertexArray.setup();

    m_gl.glGenBuffers(1, &m_axesVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_axesVBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(m_axesData), m_axesData, GL_STATIC_DRAW);

    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    m_gl.glEnableVertexAttribArray(0);

    m_axesVertexArray.unBind();
}


void AxesObject::setupTickVAO()
/*
    Setup VAO and associated buffers for the generic tick vertices.

    The tick buffer has 2 vertices per tick (e.g. for x axis, top vertex
    and bottom vertex of the tick line).For each vertex, we have the index
    (as the final position is defined with tick start and tick delta) and
    {-1, 1} that indicates it's position (top/bottom) or (left/right).
*/
{
    m_axesTickVertexArray.setup();

    m_gl.glGenBuffers(1, &m_tickVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_tickVBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(m_genericTicks), m_genericTicks, GL_STATIC_DRAW);

    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);

    m_axesTickVertexArray.unBind();
}


void AxesObject::bindAxesVAO()
{
    m_axesVertexArray.bind();
}


void AxesObject::bindTickVAO()
{
    m_axesTickVertexArray.bind();
}


/* ----------------------------------------------------------------------------------------------------------
  Init tick positions
 ----------------------------------------------------------------------------------------------------------*/


void AxesObject::initXTicks(int numDataPoints) 
/*
    Initialise the x-axis ticks (which are tied to the center of the candlestick.
    Initialise the first tick as the center of the first candle in the view.

    When the axis is on the left, the first tick is the leftmost and everything
    is relative to that. When the axis is on the right, the first tick is the
    rightmost and everything is relevant to that.
*/
{
    double delta = m_linkedSubplot.jointPlotData().getDelta();

    int minNumTicks = m_linkedSubplot.xAxisSettings().minNumTicks;
    int maxNumTicks = m_linkedSubplot.xAxisSettings().maxNumTicks;
    int initNumTicks = m_linkedSubplot.xAxisSettings().initNumTicks;

    if (m_configs.m_plotOptions.axisRight)
    {
        m_firstXTick = numDataPoints *  delta;
    }
    else
    {
        m_firstXTick = 0.0;
    }
    double range = m_linkedSubplot.camera().getViewWidth() / initNumTicks;

    m_XTickDelta = roundToCandleWidth(range);
}


void AxesObject::initYTicks()
/*
  Initialise the y ticks, which are not directly tied to the candlestick
  parameters. Select y-tick algorithmically to show nice values.
*/
{
    int minNumTicks = m_linkedSubplot.yAxisSettings().minNumTicks;
    int maxNumTicks = m_linkedSubplot.yAxisSettings().maxNumTicks;
    int initNumTicks = m_linkedSubplot.yAxisSettings().initNumTicks;

    tickFinderResult yTicks = tickFinder(
        m_linkedSubplot.camera().getBottom(),
        m_linkedSubplot.camera().getTop(),
        initNumTicks,
        minNumTicks,
        maxNumTicks
    );

    m_firstYTick = yTicks.lmin;
    m_YTickDelta = yTicks.lstep;

}


/* ----------------------------------------------------------------------------------------------------------
  Drawers
 ----------------------------------------------------------------------------------------------------------*/

void AxesObject::drawYAxesAndTicks(glm::mat4& viewportTransform, glm::mat4& NDCMatrix, double yHeightProportion)
/*
    Coordinate drawing of all axes, ticks, labels and gridlines.
*/
{
    updateYTicksZoom();
    updateYTicksPan();
    drawYTicks(viewportTransform, NDCMatrix, yHeightProportion);
}


void AxesObject::drawXAxesAndTicks(glm::mat4& viewportTransform)
{
    updateXTicksZoom();
    updateXTicksPan();
    drawXTicks(viewportTransform);
}


void AxesObject::drawAxes(glm::mat4& viewportTransform, std::string toDraw)
/*
    Draw the axes, simple as just two lines (4 vertices).

    The buffer is organised is memory with the first line (two vertices)
    the bottom left y-axis, the second line the x-axis, and the third line
    the right y-axis. So depending on whether the axis show is left or right,
    we take vertices 0:4 or 2:6.
*/
{
    bindAxesVAO();
    m_axesProgram.bind();
    m_axesProgram.setUniformMatrix4fc("axes_viewport_transform", viewportTransform);

    if (toDraw == "x")
    {
        m_axesProgram.setUniform4f("lineColor", m_linkedSubplot.xAxisSettings().axisColor);
        m_gl.glLineWidth(m_linkedSubplot.xAxisSettings().axisLinewidth);
        m_gl.glDrawArrays(GL_LINES, 2, 2);
    }
    else if (toDraw == "y")
    {
        m_axesProgram.setUniform4f("lineColor", m_linkedSubplot.yAxisSettings().axisColor);
        m_gl.glLineWidth(m_linkedSubplot.yAxisSettings().axisLinewidth);

        if (m_configs.m_plotOptions.axisRight)
        {
            m_gl.glDrawArrays(GL_LINES, 4, 2);
        }
        else
        {
            m_gl.glDrawArrays(GL_LINES, 0, 2);
        }
    }
    else
    {
        std::cerr << "CRITICAL ERROR: `toDraw` value not recognised." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}


void AxesObject::drawXTicks(glm::mat4& viewportTransform)
/*
    Coordinate drawing of the x ticks, associated gridlines and
    x tick labels.

    The start tick position and gap between ticks (tick delta) are first converted to NDC.

    Key parameters:

    viewportTransform : transform from window edge to viewport edge (in NDC)
    tickStartView : first tick to draw, position in NDC
    tickDeltaView : offset between conseceutive ticks, in NDC
    startAtLowestIdx : if 1 then we are starting at the leftmost tick and increasing. 
                      Otherwise, we start at the rightmost tick and decrease. 
                      Used depending on whether left or right y-axis is shown.
    isX :            1 if is the x-axis, 0 otherwise.
    axisPos :        Position of the axis in NDC coordinates (before viewport transformation)
    xAxisSettings :  see Configs.h for list of changable axis configurations.
*/
{
    float tickStartView = (2.0 * (m_firstXTick - m_linkedSubplot.camera().getLeft())) / (m_linkedSubplot.camera().getViewWidth()) - 1.0;
    float tickDeltaView = (2.0 * m_XTickDelta) / m_linkedSubplot.camera().getViewWidth();

    int startAtLowestIdx = (m_configs.m_plotOptions.axisRight) ? 0 : 1;

    float axisPos = -1.0f;

    drawTicksAndGridlines(
        viewportTransform,
        tickStartView,
        tickDeltaView,
        startAtLowestIdx,
        1,  // isX,
        axisPos,
        m_linkedSubplot.xAxisSettings()
    );

    axisPos -= m_linkedSubplot.yAxisSettings().tickSize;

    drawXTickLabels(
        viewportTransform,
        tickStartView,
        tickDeltaView,
        startAtLowestIdx,
        axisPos
    );
}


void AxesObject::drawYTicks(glm::mat4& viewportTransform, glm::mat4 NDCMatrix, double yHeightProportion)
/*
    See DrawXTicks.
*/
{
    // glm::vec4 tmpTickStartView = NDCMatrix * glm::vec4(0.0f, m_firstYTick, 0.0f, 0.0f);
    float tickStartView = (2.0 * (m_firstYTick - m_linkedSubplot.camera().getBottom())) / (m_linkedSubplot.camera().getViewHeight()) - 1.0;

    // glm::vec4 tmpTickDeltaView = NDCMatrix * glm::vec4(0.0f, m_YTickDelta, 0.0f, 0.0f);
    float tickDeltaView = (2.0 * m_YTickDelta) / m_linkedSubplot.camera().getViewHeight();

    float axisPos = (m_configs.m_plotOptions.axisRight) ? 1.0f : -1.0f;
    
    int startAtLowestIdx = 1;
    int isX = 0;

    drawTicksAndGridlines(
        viewportTransform,
        tickStartView,
        tickDeltaView,
        startAtLowestIdx,   
        isX,                 
        axisPos,
        m_linkedSubplot.yAxisSettings()
    );

    // Make a small offset to move the label away from the axis (i.e. in positive
    // or negative direction depending on the axis x-position (-1 or +1)
    axisPos += axisPos * m_linkedSubplot.yAxisSettings().tickSize / 2;

    drawYTickLabels(
        viewportTransform,
        tickStartView,
        tickDeltaView,
        startAtLowestIdx,
        axisPos,
        yHeightProportion
    );
}


void AxesObject::drawXTickLabels(
    glm::mat4& viewportTransform,
    float tickStartView,
    float tickDeltaView,
    int startAtLowestIdx,
    float axisPos
)
/*
    Handle the drawing of the x-axis tick labels. See drawXTicks() for arguments.

    First, we need to generate the tick labels to show. We do this based on
    the current first tick to display, and tick delta (we need to increase or decrease
    from first tick depending on left or right axis). 

    One the list of tick labels is generated, we write them to the 
    vertex array buffer draw them. `drawXTickLabels` handles the
    low-level shader uniform setup and drawing.
*/
{
    float direction = (startAtLowestIdx) ? 1.0f : -1.0f;

    std::string leftOrRight = m_configs.m_plotOptions.axisRight ? "right" : "left";
    int numTicksShown = getNumTicksShown("x" + leftOrRight);

    std::vector<int> tickIndicesVector;

    for (int i = 0; i < numTicksShown; i++)
    {
        double delta = m_linkedSubplot.jointPlotData().getDelta();

        double tickIndexCalc = (m_firstXTick / delta) + i * (m_XTickDelta / delta) * direction;
        int tickIndex = static_cast<int>(std::round(tickIndexCalc));

        if (tickIndex < 0)
        {
            continue;
        }
        if (tickIndex > m_linkedSubplot.jointPlotData().getNumDatapoints() - 1)
        {
            continue;
        }

        tickIndicesVector.push_back(tickIndex);
    }

    std::vector<std::string> allTickLabels;
    int numChars = 0;

    bool isDatetime = m_sharedXData.getDateType() == DateType::Timepoint;

    if (!isDatetime)
    {
        for (int i = 0; i < tickIndicesVector.size(); i++)
        {
            std::string label = m_sharedXData.getXTickLabelStr(tickIndicesVector[i]);
            allTickLabels.push_back(label);
            numChars += label.size();
        }
    }
    else
    {
        allTickLabels = m_sharedXData.getXTickLabelDatetime(tickIndicesVector);
        for (int i = 0; i < allTickLabels.size(); i++)
        {
            numChars += allTickLabels[i].size();  // TODO: sort this out!
        }
    }

    m_linkedSubplot.axisTickLabels().xWriteTextToBuffer(allTickLabels, numChars);

    m_linkedSubplot.axisTickLabels().drawXTickLabels(viewportTransform, tickStartView, tickDeltaView, axisPos, numChars, 25.0f, 25.0f, startAtLowestIdx);

    // Allow time_point as a vector and hold it on SharesXAxis
    // Show it with the label as above
    // figure out the looping and introduce the different modes depending on the displayed date range.
    // Handle on the python side, and the public-facing API on the c++ side.
    // make sure scatterplot is handled properly!
}


void AxesObject::drawYTickLabels(
    glm::mat4& viewportTransform,
    float tickStartView,
    float tickDeltaView,
    int startAtLowestIdx,
    float axisPos,
    double yHeightProportion
)
/*
    See drawXTickLabels().
*/
{
    int numTicksShown = getNumTicksShown("y");

    int numChars = 0;
    std::vector<std::string> allTickLabels;
    for (int i = 0; i < numTicksShown; i++)
    {
        std::string tickValue = fmt::format("{:.{}f}", (float)m_firstYTick + i * (float)m_YTickDelta, m_linkedSubplot.yAxisSettings().tickLabelDecimalPlaces.value());
        numChars += tickValue.length();
        allTickLabels.push_back(tickValue);
    }

    m_linkedSubplot.axisTickLabels().yWriteTextToBuffer(allTickLabels, numChars);
    m_linkedSubplot.axisTickLabels().drawYTickLabels(
        viewportTransform,
        tickStartView,
        tickDeltaView,
        axisPos,
        numChars,
        25.0f,
        25.0f,
        startAtLowestIdx,
        yHeightProportion
    );
}


void AxesObject::drawTicksAndGridlines(
    glm::mat4& viewportTransform,
    float tickStartView,
    float tickDeltaView,
    int startAtLowestIdx,
    int isX,
    float axisPos,
    const BackendAxisSettings& axisSettings
)
/*
    Handle shader setup and draw calls for the axis ticks and gridlines.
    
    Handles drawing of both x and y-axis ticks depending on arguments.

    See drawXTicks() for argument documentation.

    Note `maxPossibleTicks` just puts an upper bound on the number of ticks
    to draw, it is the number of visible ticks + some padding just in case.
    Anything out of the view will be clipped anyways.
*/
{
    m_gl.glDisable(GL_LINE_SMOOTH);
    m_gl.glDisable(GL_MULTISAMPLE);           // important
    m_gl.glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    m_gl.glDisable(GL_POLYGON_SMOOTH);

    // Setup buffers and uniforms
    bindTickVAO();
    m_axesTickProgram.bind();

    m_axesTickProgram.setUniformMatrix4fc("axes_viewport_transform", viewportTransform);

    if (isX)
    {
        m_axesTickProgram.setUniform1f("tickHeight", axisSettings.tickSize);
    }
    else
    {
        float direction = m_configs.m_plotOptions.axisRight ? -1 : 1;
        m_axesTickProgram.setUniform1f("tickHeight", axisSettings.tickSize * direction);
    }
    m_axesTickProgram.setUniform1f("startPos", tickStartView);
    m_axesTickProgram.setUniform1f("tickDelta", tickDeltaView);
    m_axesTickProgram.setUniform1i("startAtLowestIdx", startAtLowestIdx);
    m_axesTickProgram.setUniform1i("isX", isX);
    m_axesTickProgram.setUniform1i("axisLeft", isX);
    m_axesTickProgram.setUniform1f("axisPos", axisPos);

    int maxPossibleTicks = axisSettings.maxNumTicks + 5;

    // Draw Gridlines (first, so they are drawn over by axis ticks)
    if (axisSettings.showGridline)
    {
        m_axesTickProgram.setUniform4f("lineColor", axisSettings.gridlineColor);
        m_axesTickProgram.setUniform1i("isGridline", 1);

        m_gl.glLineWidth(axisSettings.gridlineWidth);
        m_gl.glDrawArrays(GL_LINES, 0, maxPossibleTicks * 2);
    }

    // Draw ticks
    if (axisSettings.showTicks)
    {
        m_axesTickProgram.bind();
        m_axesTickProgram.setUniform4f("lineColor", axisSettings.axisColor);
        m_axesTickProgram.setUniform1i("isGridline", 0);

        m_gl.glLineWidth(axisSettings.tickLinewidth);
        m_gl.glDrawArrays(GL_LINES, 0, maxPossibleTicks * 2);
    }

    m_gl.glEnable(GL_LINE_SMOOTH);
    m_gl.glEnable(GL_MULTISAMPLE);           // important
    m_gl.glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    m_gl.glEnable(GL_POLYGON_SMOOTH);
}


/* ----------------------------------------------------------------------------------------------------------
    Zoom and pan updaters
 ------------------------------------------------------------------------------------------------------------
 
 When the camera is zoomed or panned, the ticks must be updated. After much experimentation, this organisation
 worked, and was the most flexible to maintain. It would be nicer to set the ticks directly from the camera
 when zoom or pan occurs, but because the callbacks are triggered in the poll call it makes coordination 
 difficult.

 Instead, tick positions are adjusted for panning every loop. For zooming, it is necessary to only
 chang number of ticks when explicitly zooming, otherwise weird artefacts can occur when panning
 if the number of ticks is in between the upper limit, because we will dip over/under the threshold
 when panning. Flags are set in the camera to signal zooming and are handled in the main loop that calls 
 these functions. Its not ideal but after much experimentation this was the simplest way.

 */


void AxesObject::updateXTicksPan()
/*
    If the first tick is not within one tick delta from
    the left edge, we will add new ticks at tickDelta spacing
    until we are within one tickDelta of the left edge.
*/
{
    // Clip the ticks if they go over the edge of the camera, or the underlying data
    if (m_configs.m_plotOptions.axisRight)
    {
        if (m_firstXTick + m_XTickDelta < m_linkedSubplot.camera().getRight())
        {
            m_firstXTick += m_XTickDelta * std::floor<int>((m_linkedSubplot.camera().getRight() - m_firstXTick) / m_XTickDelta);
        }
        else if (m_firstXTick > m_linkedSubplot.camera().getRight())
        {
            m_firstXTick -= m_XTickDelta;
        }
    }
    else
    {
        if (m_firstXTick - m_XTickDelta > m_linkedSubplot.camera().getLeft())
        {
            m_firstXTick -= m_XTickDelta * std::floor<int>((m_firstXTick - m_linkedSubplot.camera().getLeft()) / m_XTickDelta);
        }
        else if ((m_firstXTick < m_linkedSubplot.camera().getLeft()))
        {
            m_firstXTick += m_XTickDelta;
        }
    }
}



void AxesObject::updateXTicksZoom()
/*
   Update number of ticks on x axis after zooming. 
   The x ticks must be in the center of the candles, so when
   we update the tickdelta we must ensure it is a multiple
   of the true handle width, so we do some rounding.
*/
{
    std::string leftOrRight = m_configs.m_plotOptions.axisRight ? "right" : "left";
    int numTicksShown = getNumTicksShown("x" + leftOrRight);

    if (numTicksShown > m_linkedSubplot.xAxisSettings().maxNumTicks)
    {
        m_XTickDelta = roundToCandleWidth(m_XTickDelta *= 2.0);
    }
    else if (numTicksShown < m_linkedSubplot.xAxisSettings().minNumTicks)
    {
        if (m_XTickDelta / 2.0 < m_linkedSubplot.jointPlotData().getDelta())
        // cannot have tick delta less than delta, or strange things happen,
        // and we never need more than 1 tick per candle.
        {
            return;
        }
        m_XTickDelta = roundToCandleWidth(m_XTickDelta /= 2.0);
    }
}


void AxesObject::updateYTicksPan()
/*
    See updateXTicksPan(), it is exactly the same
    logic now with the bottom edge.
*/
{
    if (m_firstYTick - m_YTickDelta > m_linkedSubplot.camera().getBottom())
    {
        m_firstYTick -= m_YTickDelta * std::floor<int>((m_firstYTick - m_linkedSubplot.camera().getBottom()) / m_YTickDelta);
    }
    if (m_firstYTick < m_linkedSubplot.camera().getBottom())
    {
        m_firstYTick += m_YTickDelta;
    }
}


void AxesObject::updateYTicksZoom()
/*
    See updateXTicksZoom(), it is similar logic but with
    the top bound. Also, as we are not tied to the exact 
    candlestick positions, we can just increase / decrease
    by double exactly.
*/
{
    int numTicksShown = getNumTicksShown("y");

    if (numTicksShown > m_linkedSubplot.yAxisSettings().maxNumTicks)
    {
        m_YTickDelta *= 2;
    }
    else if (numTicksShown < m_linkedSubplot.yAxisSettings().minNumTicks)
    {
        m_YTickDelta /= 2;
    }
}


/* ----------------------------------------------------------------------------------------------------------
  Helpers   
 ----------------------------------------------------------------------------------------------------------*/


int AxesObject::getNumTicksShown(std::string axisName)
/*
    "xleft", "xright", or "y".
*/
{
    int numTicksShown;
    if (axisName == "xleft")
    {
        numTicksShown = (int)std::ceil((m_linkedSubplot.camera().getRight() - m_firstXTick) / m_XTickDelta);
    }
    else if (axisName == "xright")
    {
        numTicksShown = (int)std::ceil((m_firstXTick - m_linkedSubplot.camera().getLeft()) / m_XTickDelta);
    }
    else if (axisName == "y")
    {
        numTicksShown = (int)std::ceil((m_linkedSubplot.camera().getTop() - m_firstYTick) / m_YTickDelta);
    }
    else
    {
        std::cerr << "CRITICAL ERROR: axisName " << axisName << " not recognised." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return numTicksShown;
}


double AxesObject::roundToCandleWidth(double value) const
/*
*/
{
    double delta = m_linkedSubplot.jointPlotData().getDelta();
    double tickDelta = std::floor(value / delta) * delta;

    return tickDelta;
}


tickFinderResult AxesObject::tickFinder(double dmin, double dmax, int numTicks, int minTicks, int maxTicks)
{
    std::vector<double> Q = {0.01, 0.025, 0.05, 0.1, 0.2, 0.25, 0.4, 0.5, 0.75, 1, 2, 4, 5, 3, 7, 6, 8, 9 };

    double range = dmax - dmin;

    if (range <= 0 || numTicks <= 1)
    {
        std::cerr << "CRITCAL ERROR: Invalid input: dmax must be greater than dmin, and numTicks must be > 1" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int intervals = numTicks - 1;

    double minCoverage = 0.0;
    bool firstInnerLoop = true;

    tickFinderResult result;
    result.badFlag = true;

    for (int i = 0; i < Q.size(); ++i) {
        double delta = range / intervals;
        double base = std::floor(std::log10(delta));
        double dbase = std::pow(10.0, base);

        double tdelta = Q[i] * dbase;
        int thisNumTicks = std::ceil(range / tdelta);
        if (thisNumTicks < minTicks || thisNumTicks > maxTicks)
        {
            continue;
        }
        if (tdelta < range / intervals / 10) {
            continue; // Skip overly fine step sizes
        }




        double tmin = std::floor(dmin / tdelta) * tdelta;
        double tmax = tmin + intervals * tdelta;

        if (tmax < dmax) {
            tmax = tmin + std::ceil((dmax - tmin) / tdelta) * tdelta;
        }

        if (tmin <= dmin && tmax >= dmax) {
            double coverage = (dmax - dmin) / (tmax - tmin);
            double granularity = std::abs((tmax - tmin) - range) / range;
            double tnice = coverage - granularity;

            if (firstInnerLoop || tnice > result.score) {
                result.lmin = tmin;
                result.lmax = tmax;
                result.lstep = tdelta;
                result.numTicks = numTicks;
                result.score = tnice;
                result.badFlag = false;

                firstInnerLoop = false;
            }
        }
    }

    return result;
}
