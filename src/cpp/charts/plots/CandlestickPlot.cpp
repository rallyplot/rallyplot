
#include "../../include/UserVector.h"
#include "CandlestickPlot.h"
#include "../../include/UserVector.h"
#include "../../structure/LinkedSubplot.h"


CandlestickPlot::CandlestickPlot(
    Configs& configs,
    LinkedSubplot& subplot,
    BackendCandlestickSettings candlestickSettings,
    QOpenGLFunctions_3_3_Core& glFunctions,
    const float* openPtr, std::size_t openSize,
    const float* highPtr, std::size_t highSize,
    const float* lowPtr, std::size_t lowSize,
    const float* closePtr, std::size_t closeSize
)
	: 
	  m_configs(configs),
      m_linkedSubplot(subplot),
      m_candlestickSettings(candlestickSettings),
      m_gl(glFunctions),
      m_instanceProgram(
          "candlestick_vertex.shader",
          "candlestick_fragment.shader",
          glFunctions
      ),
      m_lineProgram(
          "candlestick_vertex.shader",
          "candlestick_line_fragment.shader",
          "line_geometry.shader",
          glFunctions
      ),
      m_oldPlotStyleProgram(
          "candlestick_vertex.shader",
          "candlestick_fragment.shader",
          glFunctions
      ),
      m_plotData(configs, openPtr, openSize, highPtr, highSize, lowPtr, lowSize, closePtr, closeSize),
      m_bodyVAO(glFunctions),
      m_candleVAO(glFunctions),
      m_lineVAO(glFunctions),
      m_linePlotVAO(glFunctions)
{
    initializeAllBuffers();
	m_instanceProgram.setupAndBindProgram();
    m_lineProgram.setupAndBindProgram();
    m_oldPlotStyleProgram.setupAndBindProgram();
}


CandlestickPlot::~CandlestickPlot()
{
    m_gl.glDeleteBuffers(1, &m_instanceVBO);
    m_instanceVBO = 0;

    m_gl.glDeleteBuffers(1, &m_bodyBasisVBO);
    m_bodyBasisVBO = 0;

    m_gl.glDeleteBuffers(1, &m_candleBasisVBO);
    m_candleBasisVBO = 0;

    m_gl.glDeleteBuffers(1, &m_lineBasisVBO);
    m_lineBasisVBO = 0;
}


/* -----------------------------------------------------------
   Drawers
   ---------------------------------------------------------*/


void CandlestickPlot::draw(glm::mat4& NDCMatrix, Camera& camera)
/*
	Here we coordinate the 5 different plot types for candlestick data.

    "full" : The full candlestick plot. First, we draw the body, then all the candlestick lines
	
    "bodyOnly" : First draw the body, then the shadow connectiong low-high
	
    "noCaps" : First draw the body, then a single vertical line connecting Open-Close.
							 This is required to avoid the candles dissapearing when zooming our far.
						     Maybe there is a better way to acheive this!
	
	"lineOpen" : A simple line plot, instancing is not used for this. Plot the Open positions as a line.
	
	"lineClose" :Simila to "lineOpen", but plot the Close positions as a line.


	The uniforms are:
		
	NDCMatrix : matrix from the camera to convert from view coordinates (the windowed 
			    world coordinates) to NDC (+- 1).

	offset : due to numerical issues, it is better to compute the NDCMatrix transformation
	         centered at zero. The offset is simply the left camera edge, and we pass
			 this to the shader so the NDC transformation incorproates this offset, 
			 see m_camera.getNDCMatrix().

	lampMode : currently unused mode for lighting effect.

	upColor : Color for price rise.

	downColor : Color for price fall

    candleWidth  TODODODOODO

	capWidth

    Draw Modes for Vertex Shader
    ----------------------------

    0 : The candle body
    1 : The full candlestick lines (vertical, two horizontal)
    2 : Line extending vertically through the body and extending to low / high (no cap tops)
    3 : Line extending vertically through the body to open / close (no caps)
    4 : Line only (open)
    5 : Line only (close)
*/
{
    m_gl.glEnable(GL_DEPTH_TEST);  // dont draw overlapping points (e.g. zoomed out)

    m_instanceProgram.bind();
    m_instanceProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());
	m_instanceProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
    m_instanceProgram.setUniform1f("offset", (float)camera.getLeft());
    m_instanceProgram.setUniform1i("lampMode", 0);

    m_instanceProgram.setUniform4f("upColor", m_candlestickSettings.upColor);
    m_instanceProgram.setUniform4f("downColor", m_candlestickSettings.downColor);

    m_instanceProgram.setUniform1f("candleWidthRatio", m_candlestickSettings.candleWidthRatio);
    m_instanceProgram.setUniform1f("capWidthRatio", m_candlestickSettings.capWidthRatio);

    bool isCandleStickPlot = (m_candlestickSettings.mode != CandlestickMode::lineOpen && m_candlestickSettings.mode != CandlestickMode::lineClose);

	if (isCandleStickPlot)
	{
		// Draw the candle body
		m_instanceProgram.setUniform1i("drawMode", 0);
        m_bodyVAO.bind();
        m_gl.glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_plotData.getNumDatapoints());

		// Next draw lines, either full candles, candles without caps, 
		// or line behind the body to stop it dissapearing when zoomed out.
        // TODO: this is a slow hack to avoid candles dissapearing when zoomed out.
        if (m_candlestickSettings.mode == CandlestickMode::full)
		{
            m_instanceProgram.setUniform1i("drawMode", 1);
			m_candleVAO.bind();
            m_gl.glDrawArraysInstanced(GL_LINES, 0, 6, m_plotData.getNumDatapoints());
		}
        else if (m_candlestickSettings.mode == CandlestickMode::noCaps)
        {
            m_instanceProgram.setUniform1i("drawMode", 2);
            m_lineVAO.bind();
            m_gl.glDrawArraysInstanced(GL_LINES, 0, 2, m_plotData.getNumDatapoints());
        }
        else
        {
            m_instanceProgram.setUniform1i("drawMode", 3);
            m_lineVAO.bind();
            m_gl.glDrawArraysInstanced(GL_LINES, 0, 2, m_plotData.getNumDatapoints());
        }
    }    
    else
    {
        if (!m_candlestickSettings.lineModeBasicLine)
        {
            m_lineProgram.bind();
            m_lineProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());
            m_lineProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
            m_lineProgram.setUniform1f("offset", (float)camera.getLeft());
            m_lineProgram.setUniform1i("lampMode", 0);

            m_lineProgram.setUniform4f("upColor", m_candlestickSettings.upColor);
            m_lineProgram.setUniform4f("downColor", m_candlestickSettings.downColor);

            m_lineProgram.setUniform1f("candleWidthRatio", m_candlestickSettings.candleWidthRatio);
            m_lineProgram.setUniform1f("capWidthRatio", m_candlestickSettings.capWidthRatio);

            m_lineProgram.setUniform1f("aspectRatio", camera.getAspectRatio());
            m_lineProgram.setUniform1f("yHeightProportion", m_linkedSubplot.m_yHeightProportion);
            m_lineProgram.setUniform1f("subplotHeightProportion", m_linkedSubplot.windowViewport().subplotSizePercent().second); // TODO: this naming is super confusing, this is the high level subplot
            m_lineProgram.setUniform1i("useColor", 1);

            int drawMode = (m_candlestickSettings.mode == CandlestickMode::lineOpen) ? 4 : 5;
            m_lineProgram.setUniform1i("drawMode", drawMode);
            m_lineProgram.setUniform1f("width", m_candlestickSettings.lineModeLinewidth / 100.0);
            m_lineProgram.setUniform1f("miterLimit", m_candlestickSettings.lineModeMiterLimit);

            m_linePlotVAO.bind();
            m_gl.glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, m_plotData.getNumDatapoints() + 1);
        }
        else
        {
            m_oldPlotStyleProgram.bind();
            m_oldPlotStyleProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());
            m_oldPlotStyleProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
            m_oldPlotStyleProgram.setUniform1f("offset", (float)camera.getLeft());
            m_oldPlotStyleProgram.setUniform1i("lampMode", 0);

            m_oldPlotStyleProgram.setUniform4f("upColor", m_candlestickSettings.upColor);
            m_oldPlotStyleProgram.setUniform4f("downColor", m_candlestickSettings.downColor);

            m_oldPlotStyleProgram.setUniform1f("candleWidthRatio", m_candlestickSettings.candleWidthRatio);
            m_oldPlotStyleProgram.setUniform1f("capWidthRatio", m_candlestickSettings.capWidthRatio);

            int drawMode = (m_candlestickSettings.mode == CandlestickMode::lineOpen) ? 4 : 5;
            m_lineProgram.setUniform1i("drawMode", drawMode);


            m_linePlotVAO.bind();

            m_gl.glDrawArrays(GL_LINE_STRIP, 0, m_plotData.getNumDatapoints());
        }
    }

    m_gl.glDisable(GL_DEPTH_TEST);
}


void CandlestickPlot::cyclePlotType()
{
    if (m_candlestickSettings.mode == CandlestickMode::full)
        m_candlestickSettings.mode = CandlestickMode::noCaps;

    else if (m_candlestickSettings.mode == CandlestickMode::noCaps)
        m_candlestickSettings.mode = CandlestickMode::bodyOnly;

    else if (m_candlestickSettings.mode == CandlestickMode::bodyOnly)
        m_candlestickSettings.mode = CandlestickMode::lineOpen;

    else if (m_candlestickSettings.mode == CandlestickMode::lineOpen)
        m_candlestickSettings.mode = CandlestickMode::lineClose;

    else if (m_candlestickSettings.mode == CandlestickMode::lineClose)
        m_candlestickSettings.mode = CandlestickMode::full;
}

/* -----------------------------------------------------------
   Setup Buffers
------------------------------------------------------------*/


void CandlestickPlot::initializeAllBuffers()
/*
    We have an instance array (xPos, Open, Close, Low, High) and a set
	of basis shapes (full candle, or verticle line placed between 
    Low-High price of Open-Close price) as well as a normal vertex
	line plot.

	For convenient plotting, we create 4 VAO that share the same
	instance VBO but have (optional) basis shape attached in the 
	first position. As the instance VBO is set only once, it is
	not duplicated across VAO.

	The instance attributes are:
        (xPos), (Open, Close), (Low, High).

	m_bodyVAO : VAO for the square body. The data is (x, y) pairs
			    indicating the 4 edges of the square.

	m_candleVAO : VAO for the candle lines. This is (x, y) pairs
				  for the vertices for the 3 lines that make the candle.

	m_lineVAO : A single verticle line (x, y), two points. 

	m_linePlotVAO : This is just the instance buffer with all attributes bound.
	                No basis shape is required because it is used for a simple line plot.
*/
{
	// Setup the instance buffer (shared between all VAO)
    m_gl.glGenBuffers(1, &m_instanceVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);


    // Interleave the data for fast access on the GPU.
    // This is a copy operation
    std::vector<float> tmp;
    tmp.resize(m_plotData.m_open.size() * 4);

    int j = 0;
    for (int i = 0; i < m_plotData.m_open.size(); i ++)
    {
        // Note the reorder to (open, close, high, low)
        tmp[j] = m_plotData.m_open[i];
        tmp[j + 1] = m_plotData.m_close[i];
        tmp[j + 2] = m_plotData.m_low[i];
        tmp[j + 3] = m_plotData.m_high[i];

        j += 4;
    }
    m_gl.glBufferData(GL_ARRAY_BUFFER, tmp.size() * sizeof(float), tmp.data(), GL_STATIC_DRAW);


	// Setup the body buffer and bind the associated instance VAO
	m_bodyVAO.setup();
    m_gl.glGenBuffers(1, &m_bodyBasisVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_bodyBasisVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getBodyBasis().size() * sizeof(float), m_plotData.getBodyBasis().data(), GL_STATIC_DRAW);
    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);

	rebindInstanceBuffer(true);

	// Setup the candle buffer and bind the associated instance VAO
	m_candleVAO.setup();
    m_gl.glGenBuffers(1, &m_candleBasisVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_candleBasisVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getCandleBasis().size() * sizeof(float), m_plotData.getCandleBasis().data(), GL_STATIC_DRAW);
    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);

	rebindInstanceBuffer(true);

	// Setup the line buffer and bind the associated instance VAO
	m_lineVAO.setup();
    m_gl.glGenBuffers(1, &m_lineBasisVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_lineBasisVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getLineBasis().size() * sizeof(float), m_plotData.getLineBasis().data(), GL_STATIC_DRAW);
    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);

	rebindInstanceBuffer(true);

	
	m_linePlotVAO.setup();
	rebindInstanceBuffer(false);
}

void CandlestickPlot::rebindInstanceBuffer(bool setAttributeDivisor)
/*
    Setup the attributes for the (xPos), (Open, Close), (Low, High) on the instance array.
*/
{
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0 * sizeof(float)));  // (Open, Close)
    m_gl.glEnableVertexAttribArray(1);

    m_gl.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));  // (Low, High)
    m_gl.glEnableVertexAttribArray(2);

	if (setAttributeDivisor)  // We do not want to do this for line plot which is not an instance
	{
        m_gl.glVertexAttribDivisor(1, 1);
        m_gl.glVertexAttribDivisor(2, 1);
	}
}

