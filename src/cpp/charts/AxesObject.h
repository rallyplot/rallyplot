#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <QOpenGLFunctions_3_3_Core>

#include "../opengl/VertexArrayObject.h"
#include "shaders/Program.h"
#include "Camera.h"
#include "../Configs.h"
#include "../structure/SharedXData.h"

class LinkedSubplot;  // forward declaration

struct tickFinderResult
{
	double lmin;
	double lmax;
	double lstep;
	double score;
	int numTicks;
	bool badFlag;
};


class AxesObject
	/*
		Class to coordinate the drawing of axes, axes ticks and gridlines.

		The ticks are initialised based on the data, and expressed
		as a start position and tick separation distance (tick delta)
		in world coordinates. As ticks are zoomed / panned, the start position
		and tick delta is adjusted accordingly. 
	*/
{
public:
    // JointPlotData& jointPlotData, Camera& camera, AxisTickLabels& axisTickLabels,
    AxesObject(Configs& configs, SharedXData& sharedXData, LinkedSubplot& subplot, QOpenGLFunctions_3_3_Core& glFunctions);
	~AxesObject();

    AxesObject(const AxesObject&) = delete;
    AxesObject& operator=(const AxesObject&) = delete;
    AxesObject(AxesObject&&) = delete;
    AxesObject& operator=(AxesObject&&) = delete;

	void setupAxesVAO();
	void bindAxesVAO();

	void setupTickVAO();
	void bindTickVAO();

	void initXTicks(int numDataPoints);
	void initYTicks();

    void drawAxes(glm::mat4& viewportTransform, std::string toDraw);
    void drawXAxesAndTicks(glm::mat4& viewportTransform);
    void drawYAxesAndTicks(glm::mat4& viewportTransform, glm::mat4 &NDCMatrix, double yHeightProportion);

    double roundToCandleWidth(double value) const;

    tickFinderResult tickFinder(double dmin, double dmax, int numTicks, int minTicks, int maxTicks);

    int getNumTicksShown(std::string axisName);

private:

    Configs& m_configs;
    SharedXData& m_sharedXData;
    LinkedSubplot& m_linkedSubplot;
    QOpenGLFunctions_3_3_Core& m_gl;

    Program m_axesProgram;
	Program m_axesTickProgram;
	VertexArrayObject m_axesVertexArray;
	VertexArrayObject m_axesTickVertexArray;

    float m_axesData[12];
	float m_genericTicks[120];

	unsigned int m_axesVBO;
	unsigned int m_tickVBO;

    // double m_numXTicks;
    double m_firstXTick;
    double m_XTickDelta;

    // double m_numYTicks;
	double m_firstYTick;
	double m_YTickDelta;

	std::vector<std::string> m_yTickLabels;
	int m_numYDigits;

	void updateXTicksPan();
	void updateXTicksZoom();

	void updateYTicksPan();
	void updateYTicksZoom();

	void drawXTicks(glm::mat4& viewportTransform);
    void drawYTicks(glm::mat4& viewportTransform, glm::mat4 NDCMatrix, double yHeightProportion);

	void drawTicksAndGridlines(
		glm::mat4& viewportTransform,
		float tickStartView,
		float tickStartWorld,
		int startAtLowestIdx,
		int isX,
		float axisPos,
        const BackendAxisSettings& axisSettings
	);

	void drawYTickLabels(
		glm::mat4& viewportTransform,
		float tickStartView,
		float tickDeltaView,
		int startAtLowestIdx,
        float axisPos,
        double yHeightProportion
	);

	void drawXTickLabels(
		glm::mat4& viewportTransform,
		float tickStartView,
		float tickDeltaView,
		int startAtLowestIdx,
		float axisPos
	);
};



