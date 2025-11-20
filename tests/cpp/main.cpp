// TODOs
// ---------------------

// For documentation:
// - make clear notes that the lifetime of data must exceed the lifetime of plots.

// - dates are always hashed (for scatterplot)


// - document scatterplot, other textgures "// TODO: document this! 0 is here, 1 is scatterplot, 2 is" or centralise all enums in configs

// - make sure config responsibilities (central, etc) are clear.
// review std::string vs. string view, consts, namespaces

/*

// Roadmap / small things to look into
// -----------------------------------
// Make reset charts reset axis!
// - Subplots (x dimension)
// - connect to websocket for live plots
// - improve performance for 20 million+ points
// - smarter way to pin y axis
// - texture are created for every single subplot!! not necesarry...
// - an option to switch between keys moving all suplots vs. a specific subplot
// - Fast mouse protection: fast mouse move protection is not good...  it would be nice to remove it and find a better way to handle the issue with axis, while loop etc!
// - axis ticks slow to update for ~10m
// - scaler the miter limit by zoom
// - zoom out, at edges it goes weirdly lagging...
// - different subplot y-axis labels for linked subplots
// - gridlinewidth does not work on all platforms (GPU dependent)
// - More windows! probably dont use separate processes (maybe) but instead use new widgets!
// - need to refactor plotter so it can act like a widget
// - fix the double-draw on the candlesticks which is really hurting performance.
// - automate the
// - plot names! (1) add plot name argument, 2) interacive with legend (add legend name to plot) 3) use name for hover
// - extend draw mode
// - currently not possible to set line color from python, read directly from plot...
// - Precision issues: Maximum precision is around 11 million (add to docs!). Different dragging behaviour between axis vs. plot. Note that scatter and line go out of alignment
//   (reduce range to -0.5, 0.5 or whatever. m_firstXTick will be key for redoing these calculations. Check what is happening at the shader level!
//   ALONG WITH FLOAT, CHECK ALL METHODS OF COMPUTING POSITION FOR PLOT OVERLAYS (E.G. CROSSHAIR, HOVER) USE THE SAME METHOD AND MATCH EXACTLY!
// - const is used so intermiddetly
// scale line padding for hover mouse my pproportion! i.e. when u hover your mouse over a line and it shows the popup, this width is not scaled by proprortion

// NOTES
// -----
// - Line Plots
//       https://mattdesl.svbtle.com/drawing-lines-is-hard
// for dashed lines - much harder!
//       https://almarklein.org/triangletricks.html
//       https://github.com/pygfx/pygfx/pull/628
//       https://almarklein.org/line_rendering.html
//       https://decovar.dev/blog/2021/08/29/qt-webassembly-custom-opengl/
//       https://doc.qt.io/qt-6/wasm.html
//       use vertex shader expansion!

// it would be nice to be able to return the plot objects (subplot, linked subplot)

/*
 KNOWN ISSUES
-------------
- Zooming is weird on Linux (at least, WSL, test elsewhere)
- trackpad works but is not 100% optimal
- the ticks update slwoly and weirdly, need to fix this, then can remove timer on mouseReleaseEvent.

DONT FORGET TO FIX .DLL PATH ISSUES CURRENT WAY IS A HACK!!
and check my c++ approach makes sense
then activation key
then website

TODO: check / add datetime tests properly to cover all options
*/


#include <Plotter.h>
#include <ToyData.h>
#include <vector>

struct BollingerOutput
{
    std::vector<float> rollingMean;
    std::vector<float> stdUpper;
    std::vector<float> stdLower;
};

BollingerOutput computeBollinger(const std::vector<float>& data)
/*
    A quick function to compute rolling mean and 2x std with
    pre-determined window size (20).
 */
{
    int n = data.size();
    std::vector<float> rollingMean(n);
    std::vector<float> stdUpper(n);
    std::vector<float> stdLower(n);

    int nTimesStd = 2;
    int W = 20;
    double mean = 0;
    double S1 = 0;
    for (int i = 0; i < W; i++)
    {
        mean += data[i];
        S1 += data[i]*data[i];

        rollingMean[i] = std::numeric_limits<float>::quiet_NaN();
        stdUpper[i] = std::numeric_limits<float>::quiet_NaN();
        stdLower[i] = std::numeric_limits<float>::quiet_NaN();
    }
    mean = mean / W;
    S1 = S1 / W;

    double var;
    for (int i = W; i < n; i++)
    {
        mean = mean - data[i - W] / W + data[i] / W;
        S1 = S1 - data[i - W]*data[i - W] / W + data[i]*data[i] / W;

        var = S1 - mean*mean;
        if (var < 0.0) var = 0.0;

        rollingMean[i] = mean;
        stdUpper[i] = mean + std::sqrt(var) * nTimesStd;
        stdLower[i] = mean - std::sqrt(var) * nTimesStd;
    }

    return BollingerOutput{
        rollingMean, stdUpper, stdLower
    };
}


int main(int argc, char* argv[])
{

    CandleData candleData = getToyCandlestickData(100000);

    BollingerOutput bollingerOutput = computeBollinger(candleData.close);

    PlotterArgs plotterArgs{};
    plotterArgs.antiAliasingSamples = 8;

    Plotter plotter = Plotter(plotterArgs);

    plotter.candlestick(
        candleData.open, candleData.high, candleData.low, candleData.close, candleData.dates
    );


    LineSettings lineSettings{};
    lineSettings.color = {0.1216, 0.4667, 0.7059};
    lineSettings.width = 1.5f;

    plotter.line(
        bollingerOutput.rollingMean,
        candleData.dates,
        lineSettings
        );

    lineSettings.color = {0.6824, 0.7804, 0.9098};
    lineSettings.width = 0.75f;

    plotter.line(bollingerOutput.stdLower, lineSettings);
    plotter.line(bollingerOutput.stdUpper, lineSettings);

    plotter.start();

}



/*

    if (false)
    {

    PlotterArgs plotArgs;
    plotArgs.colorMode = ColorMode::light;
    plotArgs.antiAliasingSamples = 0;
    // plotArgs.axisRight = false;
  //  plotArgs.height = 300;
   // plotArgs.width = 1200;
    // plotArgs.heightMarginSize = 0;
    // plotArgs.widthMarginSize = 0;

    // plotArgs.axisRight = false;
    Plotter plot = Plotter(plotArgs);

    // TODO: check filepath for better error.
    std::string path;

    CandleData candleData = getToyCandlestickData(1'000'000);   //  plot._readDataFromCSV(path);  // plot without dates

    std::vector<int> scatterXData;
    scatterXData.reserve(int(candleData.open.size() / 1000.0));

    std::vector<float> scatterYData;
    scatterYData.reserve(int(candleData.open.size() / 1000.0));



    for (int i = 0; i < candleData.open.size(); i++)
    {
        if (i % 1000 == 0)
        {
            scatterXData.push_back(i);
            scatterYData.push_back(candleData.open[i]);
        }
    }

    std::vector<std::string> x_data;
    x_data.reserve(candleData.open.size());


    for (int i = 0; i < candleData.open.size(); i++)
    {
        x_data.push_back(std::to_string(i));
    }

    std::vector<float> y_data = candleData.open;

    for (int i = 0; i < y_data.size(); i++)
    {
        y_data[i] = y_data[i] + 2.5f;
    }

    std::vector<float> datesFloat;
    datesFloat.reserve(y_data.size());

    for (int i = 0; i < y_data.size(); i++)
    {
        datesFloat.push_back(float(i + 123));
    }

    std::vector<std::chrono::system_clock::time_point> datetimeVector;

    datetimeVector.reserve(1'000'000);  // Prevent reallocations

    auto start = std::chrono::system_clock::from_time_t(0);  // 1970-01-01 00:00:00 UTC
    std::chrono::minutes step(15);

    for (int i = 0; i < 1'000'000; ++i)
    {
        datetimeVector.push_back(start + i * step);
    }

    std::vector<std::chrono::system_clock::time_point> datetimeVector2;

    datetimeVector2.reserve(30000);  // Prevent reallocations

    auto start2 = std::chrono::system_clock::from_time_t(0);  // 1970-01-01 00:00:00 UTC
    std::chrono::minutes step2(15);

    for (int i = 0; i < 30000; ++i)
    {
        datetimeVector2.push_back(start2 + 2*i * step2);
    }



    CandlestickSettings candlestickSettings;
    candlestickSettings.lineModeLinewidth = 0.50;
    candlestickSettings.lineModeBasicLine = false;
    candlestickSettings.lineModeLinewidth = 2.0;

    LineSettings lineSettings;  // TODO: should swap order on linesettings
    lineSettings.basicLine = false;
    lineSettings.width = 2.0;

    std::cout << "SIZE:: " << candleData.open.size() << std::endl;

    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close, candlestickSettings);

    plot.addLinkedSubplot(0.075);
    plot.addLinkedSubplot(0.25);  // TODO: doc this
    plot.bar(candleData.volume);


    CameraSettings cameraSettings;
    cameraSettings.mouseZoomSpeed = 0.012;
    plot.setCameraSettings(cameraSettings);

    // plot.pinYAxis(false);

    plot.setTitle("Plot 1");
    plot.setXLabel("xLabel1");

    std::vector<std::string> label6 = {"H1", "H2", "H3", "H4"};
    plot.setLegend(label6);


    CrosshairSettings crosshairSetings;
    crosshairSetings.on = true;
    plot.setCrosshairSettings(crosshairSetings);

    HoverValueSettings hoverValueSettings;
    hoverValueSettings.displayMode = HoverValueDisplayMode::onlyUnderMouse;
    plot.setHoverValueSettings(hoverValueSettings);

    plot.addSubplot(0, 1, 1, 1);

    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close, candlestickSettings);

    // plot.setCameraSettings(cameraSettings);
    plot.pinYAxis(false);

    // 1) hold zoom left button while go off left edge in linux causes seg fault

    plot.setActiveSubplot(0, 0);

    plot.bar(candleData.open);




    std::vector<std::chrono::system_clock::time_point> scatterXDataLabels;
    scatterXDataLabels.reserve(scatterXData.size());
    for (int i = 0; i < scatterXData.size(); i++)
    {
        scatterXDataLabels.push_back(datetimeVector[scatterXData[i]]);
    }

    ScatterSettings scatterSettings;
    scatterSettings.shape = ScatterShape::triangleUp;
   // scatterSettings.color = {0, 0, 1};
    plot.scatter(scatterXDataLabels, scatterYData, scatterSettings);

    LineSettings lineSettings2;
    lineSettings2.color = {0.937, 0.686, 0.016};
    lineSettings2.width = 0.52f;
    plot.line(y_data, lineSettings2);


    std::cout << "Have 1." << std::endl;

    plot.addLinkedSubplot(0.25);

    lineSettings.color = {0.50, 0.50, 0.50};
    lineSettings.width = 0.50;
    plot.line(y_data);

    //ScatterSettings scatterSettings;
    scatterSettings.shape = ScatterShape::triangleUp;
    // scatterSettings.color = {0, 0, 1};
    plot.scatter(scatterXDataLabels, scatterYData, scatterSettings);

    BarSettings barSettings;
    barSettings.color = {0, 1, 1};
    plot.bar(candleData.low);
    plot.line(candleData.high, lineSettings);

    plot.addLinkedSubplot(0.10);
    plot.bar(candleData.low);

    plot.addLinkedSubplot(0.25);
    plot.bar(candleData.low);


    // plot.setYLimits(140, 186);  // TODO: note this doesn't work when linking zoom... document.
//plot.pinYAxis(true);
    //plot.resizeSubplots({0.5, 0.5});

   // plot.setXLimits(candleData.dates[1], candleData.dates[20000]);

    XAxisSettings xAxisSettings;
    xAxisSettings.minNumTicks = 1;
    xAxisSettings.maxNumTicks = 4;
    plot.setXAxisSettings(xAxisSettings);

    std::cout << "Have 2." << std::endl;

    plot.setTitle("Plot 1");
    plot.setXLabel("xLabel1");

    std::vector<std::string> label = {"H1", "H2", "H3", "H4"};
    plot.setLegend(label);


    std::vector<std::string> label2 = {"A1", "A2"};
    plot.setLegend(label2);


    std::vector<std::string> label3 = {"B1"};
    plot.setLegend(label3);

    std::vector<std::string> label4 = {"B1"};
    plot.setLegend(label4);

    plot.addSubplot(0, 1, 1, 1); // , 1, 1);
    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close);

    plot.setTitle("Plot 2");
    plot.setYLabel("Price ($)");

    std::cout << "Have 3." << std::endl;

    plot.addSubplot(1, 1, 1, 1);
    plot.setXLabel("xLabel2");
    plot.setYLabel("Price");
    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close);

//    plot.pinYAxis(true);
 //   plot.addLinkedSubplot(0.15);
  //  plot.line(candleData.open, 1, lineSettings);
   // plot.pinYAxis(true);

    std::cout << "Have 3.1" << std::endl;

    plot.addSubplot(1, 0, 1, 1);
    plot.setXLabel("xLabel2");
    plot.setYLabel("Price");
    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close);

    std::cout << "Have 3.2" << std::endl;

    plot.addSubplot(0, 2, 1, 1);
    plot.setXLabel("xLabel2");
    plot.setYLabel("Price (£)");
    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close);

    std::cout << "Have 3.3" << std::endl;

    plot.addSubplot(1, 2, 1, 2);
    plot.setXLabel("xLabel2");
    plot.setYLabel("Price (£)");
    plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close);

    std::cout << "Have 4." << std::endl;

  //  plot.addSubplot(2, 2, 1, 1);
  //  plot.setXLabel("xLabel2");
 //  plot.setYLabel("Price");
 //   plot.candlestick(candleData.open, candleData.high, candleData.low, candleData.close, 0);

    std::vector<float> col = {0.1, 0.5, 1.0};
    plot.setDrawLineSettings({0.5, col});

    std::vector<std::string> label5 = {"APPL"};
    plot.setLegend(label5);

//    plot.addSubplot(0, 0, 2, 1);

  //  std::cout << status.exceptionMessage << " DONE." << std::endl;

    plot.start();

    return 0;

    }
}
*/
