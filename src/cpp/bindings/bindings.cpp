#define PYBIND11_DETAILED_ERROR_MESSAGES
#include <Plotter.h>
#include <ToyData.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>


namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<std::chrono::system_clock::time_point>);

ToyCandleStickdataSettings defaultToyCandleStickdataSettings;
PlotterArgs defaultPlotterArgs;
XAxisSettings defaultXAxisSettings;
YAxisSettings defaultYAxisSettings;
CandlestickSettings defaultCandlestickSettings;
LineSettings defaultLineSettings;
BarSettings defaultBarSettings;
ScatterSettings defaultScatterSettings;
CameraSettings defaultCameraSettings;
CrosshairSettings defaultCrosshairSettings;
HoverValueSettings defaultHoverValueSettings;
DrawLineSettings defaultDrawLineSettings;
LegendSettings defaultLegendSettings;
AxisLabelSettings defaultAxisLabelSettings;
TitleLabelSettings defaultTitleLabelSettings;


// Converters
// -----------------------------------------------------------------------------
// Convert from python input to c++-side enums. Very verbose, needs improvement.

Font fontStrToEnum(std::string font)
{
    if (font == "arial")
    {
        return Font::arial;
    }
    else if (font == "consola")
    {
        return Font::consola;
    }
    else
    {
        throw std::invalid_argument("Invalid font.");
    }
}


CandlestickMode candlestickModeStrToEnum(std::string mode)
{
    if (mode == "full")
    {
        return CandlestickMode::full;
    }
    else if (mode == "body_only")
    {
        return CandlestickMode::bodyOnly;
    }
    else if (mode == "no_caps")
    {
        return CandlestickMode::noCaps;
    }
    else if (mode == "line_open")
    {
        return CandlestickMode::lineOpen;
    }
    else if (mode == "line_close")
    {
        return CandlestickMode::lineClose;
    }
    else
    {
        throw std::invalid_argument("Invalid candle stick mode.'");
    }
}


HoverValueDisplayMode hoverValueDisplayModeStrToEnum(std::string displayMode)
{
    if (displayMode == "always_show")
    {
        return HoverValueDisplayMode::alwaysShow;
    }
    else if (displayMode == "only_under_mouse")
    {
        return HoverValueDisplayMode::onlyUnderMouse;
    }
    else if (displayMode == "off")
    {
        return HoverValueDisplayMode::off;
    }
    else
    {
        throw std::invalid_argument("Invalid display mode.");
    }
}


ScatterShape scatterShapeStrToEnum(std::string shape)
{
    if (shape == "circle")
    {
        return ScatterShape::circle;
    }
    else if (shape == "triangle_up")
    {
        return ScatterShape::triangleUp;
    }
    else if (shape == "triangle_down")
    {
        return ScatterShape::triangleDown;
    }
    else if (shape == "cross")
    {
        return ScatterShape::cross;
    }
    else
    {
        throw std::invalid_argument("Invalid scatter shape.");
    }
}

auto arr_copy = [](const std::vector<float>& v) {
    // Allocate a new NumPy array of the same length (owned by Python)
    py::array_t<float> a(v.size());
    // Copy the bytes from the C++ vector into the NumPy buffer
    std::memcpy(a.mutable_data(), v.data(), v.size() * sizeof(float));
    return a;  // No base/capsule -> Python owns memory
};

// Central Plotting Functions
// -----------------------------------------------------------------------------
// Unfortunately std::optional<std::variant< ref wrapper const str vector, ref wrapper const datetime vector>> does not work
// due to the nesting of optional variant ref wrapper. The only workable solution that I could think of is to
// overload and have central functions here. Obviously this is not ideal at al!

template <typename T>
void callCandlestickPlot(
    Plotter& self,
    py::array_t<float> open,
    py::array_t<float> high,
    py::array_t<float> low,
    py::array_t<float> close,
    std::optional<std::reference_wrapper<const std::vector<T>>> dates,
    int linkedSubplotIdx,
    std::vector<float> upColor,
    std::vector<float> downColor,
    std::string mode,
    double candleWidthRatio,
    double capWidthRatio,
    double lineModeLinewidth,
    double lineModeMiterLimit,
    bool lineModeBasicLine
)
{
    py::buffer_info bufferOpen = open.request();
    py::buffer_info bufferHigh = high.request();
    py::buffer_info bufferLow = low.request();
    py::buffer_info bufferClose = close.request();

    float* openPtr = static_cast<float*>(bufferOpen.ptr);
    float* highPtr = static_cast<float*>(bufferHigh.ptr);
    float* lowPtr = static_cast<float*>(bufferLow.ptr);
    float* closePtr = static_cast<float*>(bufferClose.ptr);

    std::size_t openSize = bufferOpen.shape[0];
    std::size_t highSize = bufferHigh.shape[0];
    std::size_t lowSize = bufferLow.shape[0];
    std::size_t closeSize = bufferClose.shape[0];

    CandlestickMode candlestickMode = candlestickModeStrToEnum(mode);

    CandlestickSettings settings{
        upColor,
        downColor,
        candlestickMode,
        candleWidthRatio,
        capWidthRatio,
        lineModeLinewidth,
        lineModeMiterLimit,
        lineModeBasicLine
    };

    self.candlestick(
        openPtr, openSize,
        highPtr, highSize,
        lowPtr, lowSize,
        closePtr, closeSize,
        dates,
        settings,
        linkedSubplotIdx
        );
}


template <typename T>
void callBarPlot(
    Plotter& self,
    py::array_t<float> yData,
    std::optional<std::reference_wrapper<const std::vector<T>>> dates,
    int linkedSubplotIdx,
    std::vector<float> color,
    double widthRatio,
    std::optional<float> minValue
)
{
    py::buffer_info bufferY = yData.request();
    float* yPtr = static_cast<float*>(bufferY.ptr);
    std::size_t ySize = bufferY.shape[0];

    BarSettings settings {
        color,
        widthRatio,
        minValue
    };

    self.bar(yPtr, ySize, dates, settings, linkedSubplotIdx);
}


template <typename T>
void callLinePlot(
    Plotter& self,
    py::array_t<float> yData,
    std::optional<std::reference_wrapper<const std::vector<T>>> dates,
    int linkedSubplotIdx,
    std::vector<float> color,
    double width,
    double miterLimit,
    bool basicLine
)
{
    py::buffer_info bufferY = yData.request();
    float* yPtr = static_cast<float*>(bufferY.ptr);
    std::size_t ySize = bufferY.shape[0];

    LineSettings settings{ color, width, miterLimit, basicLine};

    self.line(yPtr, ySize, dates, settings, linkedSubplotIdx);
}

// -----------------------------------------------------------------------------
// Plotter Class
// -----------------------------------------------------------------------------

PYBIND11_MODULE(pythonBindings, m)
{
    // TODOC: all are wrapped in lambda because we dontw ant to return plotter status

    // Bind std::vector<std::string> with custom constructor
    // this still copies but cannot get using std::vector<std::string> directly to work
    py::bind_vector<std::vector<std::string>>(m, "StringVector")
        .def(py::init([](py::list list) {
            std::vector<std::string> vec;
            vec.reserve(list.size());
            for (auto item : list) {
                vec.push_back(item.cast<std::string>());
            }
            return vec;
        }));

    py::bind_vector<std::vector<std::chrono::system_clock::time_point>>(m, "DatetimeVector")
        .def(py::init([](py::list list) {
            std::vector<std::chrono::system_clock::time_point> vec;
            vec.reserve(list.size());
            for (auto item : list) {
                vec.push_back(item.cast<std::chrono::system_clock::time_point>());
            }
            return vec;
        }));

    py::class_<Plotter>(m, "Plotter")
        .def(
            py::init(
                [](
                    int width,
                    int height,
                    std::string colorMode,
                    int antiAliasingSamples,
                    std::string axisTickLabelFont,
                    int axisTickLabelFontSize,
                    bool axisRight,
                    int widthMarginSize,
                    int heightMarginSize
                   )
                {
                    // TODO: centralise this conversion properly
                    ColorMode colorModeEnum = colorMode == "light" ? ColorMode::light : ColorMode::dark;
                    Font fontEnum = axisTickLabelFont == "arial" ? Font::arial : Font::consola;

                    PlotterArgs plotterArgs{
                        width,
                        height,
                        colorModeEnum,
                        antiAliasingSamples,
                        fontEnum,
                        axisTickLabelFontSize,
                        axisRight,
                        widthMarginSize,
                        heightMarginSize,
                    };

                    return std::make_unique<Plotter>(plotterArgs);
                }
            ),
            py::arg("width") = defaultPlotterArgs.width,
            py::arg("height") = defaultPlotterArgs.height,
            py::arg("color_mode") = "light",
            py::arg("anti_aliasing_samples") = defaultPlotterArgs.antiAliasingSamples,
            py::arg("axis_tick_label_font") = "arial",
            py::arg("axis_tick_label_font_size") = defaultPlotterArgs.axisTickLabelFontSize,
            py::arg("axis_right") = defaultPlotterArgs.axisRight,
            py::arg("width_margin_size") = defaultPlotterArgs.widthMarginSize,
            py::arg("height_margin_size") = defaultPlotterArgs.heightMarginSize
        )
        .def("start", [](Plotter& self){ self.start(); })
        .def("set_background_color",
             [](Plotter& self,
                std::vector<float> backgroundColor)
             {
                 self.setBackgroundColor(backgroundColor);
             },
             py::arg("color")
             )
        .def("set_background_color", py::overload_cast<const std::vector<float>>(&Plotter::setBackgroundColor), py::arg("color"))
        .def("set_camera_settings",
            [](Plotter& self,
              double keyZoomSpeed,
              double mouseZoomSpeed,
              double keyPanSpeed,
              double mousePanSpeed,
              double wheelSpeed,
              bool lockMostRecentDate,
              bool fixZoomAtEdge
            )
            {
                CameraSettings cameraSettings
                {
                    keyZoomSpeed,
                    mouseZoomSpeed,
                    keyPanSpeed,
                    mousePanSpeed,
                    wheelSpeed,
                    lockMostRecentDate,
                    fixZoomAtEdge
                };
                self.setCameraSettings(cameraSettings);
            },
            py::arg("key_zoom_speed") = defaultCameraSettings.keyZoomSpeed,
            py::arg("mouse_zoom_speed") = defaultCameraSettings.mouseZoomSpeed,
            py::arg("key_pan_speed") = defaultCameraSettings.keyPanSpeed,
            py::arg("mouse_pan_speed") = defaultCameraSettings.mousePanSpeed,
            py::arg("wheel_speed") = defaultCameraSettings.wheelSpeed,
            py::arg("lock_most_recent_date") = defaultCameraSettings.lockMostRecentDate,
            py::arg("fix_zoom_at_edge") = defaultCameraSettings.fixZoomAtEdge
        )
        .def("set_crosshair_settings",
            [](
               Plotter& self,
               bool on,
               std::string font,
               int fontSize,
               double linewidth,
               std::optional<std::vector<float>> lineColor,
               std::optional<std::vector<float>> backgroundColor,
               std::optional<std::vector<float>> fontColor
            )
            {
                CrosshairSettings crosshairSettings
                {
                    on,
                    fontStrToEnum(font),
                    fontSize,
                    linewidth,
                    lineColor,
                    backgroundColor,
                    fontColor
                };
                self.setCrosshairSettings(crosshairSettings);
            },
            py::arg("on") = defaultCrosshairSettings.on,
            py::arg("font") = "arial",
            py::arg("font_size") = defaultCrosshairSettings.fontSize,
            py::arg("linewidth") = defaultCrosshairSettings.linewidth,
            py::arg("line_color") = defaultCrosshairSettings.lineColor,
            py::arg("background_color") = defaultCrosshairSettings.backgroundColor,
            py::arg("font_color") = defaultCrosshairSettings.fontColor
        )
        .def("set_draw_line_settings",
            [](Plotter& self, double linewidth, std::optional<std::vector<float>> color)
            {
               DrawLineSettings drawLineSettings {static_cast<float>(linewidth), color};
               self.setDrawLineSettings(drawLineSettings);
            },
            py::arg("linewidth") = defaultDrawLineSettings.linewidth,
            py::arg("color") = defaultDrawLineSettings.color
        )
        .def("set_hover_value_settings",
             [](
                 Plotter& self,
                 std::string displayMode,
                 std::string font,
                 int fontSize,
                 std::optional<std::vector<float>> fontColor,
                 std::optional<std::vector<float>> backgroundColor,
                 std::optional<std::vector<float>> borderColor
                 )
             {
                HoverValueDisplayMode displayModeType = hoverValueDisplayModeStrToEnum(displayMode);

                HoverValueSettings hoverValueSettings{};
                hoverValueSettings.displayMode = displayModeType;
                hoverValueSettings.font = fontStrToEnum(font);
                hoverValueSettings.fontSize = fontSize;
                hoverValueSettings.fontColor = fontColor;
                hoverValueSettings.backgroundColor = backgroundColor;
                hoverValueSettings.borderColor = borderColor;

                self.setHoverValueSettings(hoverValueSettings);
             },

             py::arg("display_mode") = "always_show",
             py::arg("font") = "arial",
             py::arg("font_size") = defaultHoverValueSettings.fontSize,
             py::arg("font_color") = defaultHoverValueSettings.fontColor,
             py::arg("background_color") = defaultHoverValueSettings.backgroundColor,
             py::arg("border_color") = defaultHoverValueSettings.borderColor
        )
        .def("set_x_axis_settings",
             [](
                 Plotter& self,
                 int minNumTicks,
                 int maxNumTicks,
                 int initNumTicks,
                 bool showTicks,
                 float tickLinewidth,
                 float tickSize,
                 bool showGridline,
                 float gridlineWidth,
                 float axisLinewidth,
                 std::optional<std::vector<float>> gridlineColor,
                 std::optional<std::vector<float>> axisColor,
                 std::optional<std::vector<float>> fontColor,
                 std::optional<int> linkedSubplotIdx
                 )
             {
                XAxisSettings axisSettings;

                axisSettings.minNumTicks = minNumTicks;
                axisSettings.maxNumTicks = maxNumTicks;
                axisSettings.initNumTicks = initNumTicks;
                axisSettings.showTicks = showTicks;
                axisSettings.tickLinewidth = tickLinewidth;
                axisSettings.tickSize = tickSize;
                axisSettings.showGridline = showGridline;
                axisSettings.gridlineWidth = gridlineWidth;
                axisSettings.axisLinewidth = axisLinewidth;
                axisSettings.tickLabelDecimalPlaces = std::nullopt;
                axisSettings.gridlineColor = gridlineColor;
                axisSettings.axisColor = axisColor;
                axisSettings.fontColor = fontColor;

                self.setXAxisSettings(axisSettings, linkedSubplotIdx);
             },
             py::arg("min_num_ticks") = defaultXAxisSettings.minNumTicks,
             py::arg("max_num_ticks") = defaultXAxisSettings.maxNumTicks,
             py::arg("init_num_ticks") = defaultXAxisSettings.initNumTicks,
             py::arg("show_ticks") = defaultXAxisSettings.showTicks,
             py::arg("tick_linewidth") = defaultXAxisSettings.tickLinewidth,
             py::arg("tick_size") = defaultXAxisSettings.tickSize,
             py::arg("show_gridline") = defaultXAxisSettings.showGridline,
             py::arg("gridline_width") = defaultXAxisSettings.gridlineWidth,
             py::arg("axis_linewidth") = defaultXAxisSettings.axisLinewidth,
             py::arg("gridline_color") = py::none(),
             py::arg("axis_color") = py::none(),
             py::arg("font_color") = py::none(),
             py::arg("linked_subplot_idx") = py::none()
             )
        .def("set_y_axis_settings",  // TODO: try and merge with above
             [](
                 Plotter& self,
                 int minNumTicks,
                 int maxNumTicks,
                 int initNumTicks,
                 bool showTicks,
                 float tickLinewidth,
                 float tickSize,
                 bool showGridline,
                 float gridlineWidth,
                 float axisLinewidth,
                 int tickLabelDecimalPlaces,
                 std::optional<std::vector<float>> gridlineColor,
                 std::optional<std::vector<float>> axisColor,
                 std::optional<std::vector<float>> fontColor,
                 std::optional<int> linkedSubplotIdx
                 )
             {
                YAxisSettings axisSettings;
                axisSettings.minNumTicks = minNumTicks;
                axisSettings.maxNumTicks = maxNumTicks;
                axisSettings.initNumTicks = initNumTicks;
                axisSettings.showTicks = showTicks;
                axisSettings.tickLinewidth = tickLinewidth;
                axisSettings.tickSize = tickSize;
                axisSettings.showGridline = showGridline;
                axisSettings.gridlineWidth = gridlineWidth;
                axisSettings.axisLinewidth = axisLinewidth;
                axisSettings.tickLabelDecimalPlaces = tickLabelDecimalPlaces;
                axisSettings.gridlineColor = gridlineColor;
                axisSettings.axisColor = axisColor;
                axisSettings.fontColor = fontColor;

                self.setYAxisSettings(axisSettings, linkedSubplotIdx);
             },
             py::arg("min_num_ticks") = defaultYAxisSettings.minNumTicks,
             py::arg("max_num_ticks") = defaultYAxisSettings.maxNumTicks,
             py::arg("init_num_ticks") = defaultYAxisSettings.initNumTicks,
             py::arg("show_ticks") = defaultYAxisSettings.showTicks,
             py::arg("tick_linewidth") = defaultYAxisSettings.tickLinewidth,
             py::arg("tick_size") = defaultYAxisSettings.tickSize,
             py::arg("show_gridline") = defaultYAxisSettings.showGridline,
             py::arg("gridline_width") = defaultYAxisSettings.gridlineWidth,
             py::arg("axis_linewidth") = defaultYAxisSettings.axisLinewidth,
             py::arg("tick_label_decimal_places") = defaultYAxisSettings.tickLabelDecimalPlaces,
             py::arg("gridline_color") = py::none(),
             py::arg("axis_color") = py::none(),
             py::arg("font_color") = py::none(),
             py::arg("linked_subplot_idx") = py::none()
        )

        .def("pin_y_axis", [](Plotter& self, bool on, std::optional<int> linkedSubplotIdx){ self.pinYAxis(on, linkedSubplotIdx ); },py::arg("on") = true, py::arg("linked_subplot_idx") = std::nullopt)
        .def("set_y_limits",
             [](
                 Plotter& self,
                std::optional<float> min,
                std::optional<float> max,
                std::optional<int> linkedSubplotIdx
            ) { self.setYLimits(min, max, linkedSubplotIdx); },
             py::arg("min") = py::none(),
             py::arg("max") = py::none(),
             py::arg("linked_subplot_idx") = py::none()
        )
        .def("set_x_limits",
             [](
                 Plotter& self,
                 std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> min,
                 std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> max
               ) { self.setXLimits(min, max); },
             py::arg("min") = py::none(),
             py::arg("max") = py::none()
        )
        .def("link_y_axes", [](Plotter& self, bool on) { self.linkYAxis(on); }, py::arg("on") = true)
        .def("set_y_label",
            []
            (Plotter& self, std::string text, std::string font, std::string weight, int font_size, std::optional<std::vector<float>> color)
            {
                AxisLabelSettings axisLabelSettings
                {
                    fontStrToEnum(font), weight, font_size, color
                };

                self.setYLabel(text, axisLabelSettings);
            },
            py::arg("text"),
            py::arg("font") = "arial",
            py::arg("weight") = defaultAxisLabelSettings.weight,
            py::arg("font_size") = defaultAxisLabelSettings.fontSize,
            py::arg("color") = py::none()
        )
        .def("set_x_label",
             []
             (Plotter& self, std::string text, std::string font, std::string weight, int font_size, std::optional<std::vector<float>> color)
             {
                 AxisLabelSettings axisLabelSettings
                     {
                         fontStrToEnum(font), weight, font_size, color
                     };
                 self.setXLabel(text, axisLabelSettings);
            },
            py::arg("text"),
            py::arg("font") = "arial",
            py::arg("weight") = defaultAxisLabelSettings.weight,
            py::arg("font_size") = defaultAxisLabelSettings.fontSize,
            py::arg("color") = py::none()
        )
        .def("set_title",
             []
             (Plotter& self, std::string text, std::string font, std::string weight, int font_size, std::optional<std::vector<float>> color)
             {
                 TitleLabelSettings titleLabelSettings
                     {
                         fontStrToEnum(font), weight, font_size, color
                     };
                 self.setTitle(text, titleLabelSettings);
             },
             py::arg("text"),
             py::arg("font") = "arial",
             py::arg("weight") = defaultTitleLabelSettings.weight,
             py::arg("font_size") = defaultTitleLabelSettings.fontSize,
             py::arg("color") = py::none()
            )
        .def("set_legend",
             [] (
            Plotter& self,
            std::vector<std::string> labelNames,
            int linkedSubplotIdx,
            float legendSizeScaler,
            float xBoxPad,
            float yBoxPad,
            float lineWidth,
            float lineHeight,
            float xLinePadLeft,
            float xLinePadRight,
            float xTextPadRight,
            float yInterItemPad,
            float yItemPad,
            std::string font,
            int fontSize,
            std::optional<std::vector<float>> fontColor,
            std::vector<float> boxColor
            )
            {
                LegendSettings legendSettings
                {
                    legendSizeScaler,
                    xBoxPad,
                    yBoxPad,
                    lineWidth,
                    lineHeight,
                    xLinePadLeft,
                    xLinePadRight,
                    xTextPadRight,
                    yInterItemPad,
                    yItemPad,
                    fontStrToEnum(font),
                    fontSize,
                    fontColor,
                    boxColor
                };
                self.setLegend(labelNames, legendSettings, linkedSubplotIdx);
            },


            py::arg("label_names"),
            py::arg("linked_subplot_idx") = -1,
            py::arg("legend_size_scalar") = defaultLegendSettings.legendSizeScaler,
            py::arg("x_box_pad") = defaultLegendSettings.xBoxPad,
            py::arg("y_box_pad") = defaultLegendSettings.yBoxPad,
            py::arg("line_width") = defaultLegendSettings.lineWidth,
            py::arg("line_height") = defaultLegendSettings.lineHeight,
            py::arg("x_line_pad_left") = defaultLegendSettings.xLinePadLeft,
            py::arg("x_line_pad_right") = defaultLegendSettings.xLinePadRight,
            py::arg("x_text_pad_right") = defaultLegendSettings.xTextPadRight,
            py::arg("y_inter_item_pad") = defaultLegendSettings.yInterItemPad,
            py::arg("y_item_pad") = defaultLegendSettings.yItemPad,
            py::arg("font") = "arial",
            py::arg("font_size") = defaultLegendSettings.fontSize,
            py::arg("font_color") = defaultLegendSettings.fontColor,
            py::arg("box_color") = defaultLegendSettings.boxColor
        )
        .def("add_subplot",
             [](Plotter& self,
                int row,
                int col,
                int rowSpan,
                int colSpan
             ){ self.addSubplot(row, col, rowSpan, colSpan); },
             py::arg("row"),
             py::arg("col"),
             py::arg("row_span"),
             py::arg("col_span"))
        .def("set_active_subplot", [](Plotter& self, int row, int col){ self.setActiveSubplot(row, col); }, py::arg("row"), py::arg("col"))
        .def("resize_linked_subplots", [](Plotter& self, std::vector<double> yHeights){ self.resizeLinkedSubplots(yHeights); }, py::arg("y_heights"))
        .def("add_linked_subplot", [](Plotter& self, float heightAsProportion) { self.addLinkedSubplot(heightAsProportion); }, py::arg("height_as_proportion"))
        .def("_grab_frame_buffer",
             [](Plotter& self,
                std::optional<int> row,
                std::optional<int> col
            ){

               std::tuple<std::vector<std::uint8_t>, int, int> frameBufferOutput = self._grabFrameBuffer(row, col);
               std::vector<std::uint8_t> buffer = std::get<0>(frameBufferOutput);
               int width = std::get<1>(frameBufferOutput);
               int height = std::get<2>(frameBufferOutput);

               py::array_t<std::uint8_t> pyBuffer(buffer.size());
               std::memcpy(pyBuffer.mutable_data(), buffer.data(), buffer.size());
               std::tuple<py::array_t<std::uint8_t>, int, int> output = {pyBuffer, width, height};

               return output;
        },
        py::arg("row") = py::none(),
        py::arg("col") = py::none()
        )

        /* ----------------------------------------------------------------------------------------------------------------
            Plots
        ----------------------------------------------------------------------------------------------------------------
        Note all these ploting function signatures are duplicated twice to handle datetime vs. string, as pybind
        cannot handle nested optional variant reference wrappers.
        */

        .def("candlestick",
        [](Plotter& self,
            py::array_t<float> open,
            py::array_t<float> high,
            py::array_t<float> low,
            py::array_t<float> close,
            std::optional<StringVectorRef> dates,
            int linkedSubplotIdx,
            std::vector<float> upColor,
            std::vector<float> downColor,
            std::string mode,
            double candleWidthRatio,
            double capWidthRatio,
            double lineModeLinewidth,
            double lineModeMiterLimit,
            bool lineModeBasicLine
            )
            {
                callCandlestickPlot(
                    self, open, high, low, close, dates, linkedSubplotIdx, upColor, downColor, mode, candleWidthRatio, capWidthRatio, lineModeLinewidth,  lineModeMiterLimit, lineModeBasicLine
                );
            },
            py::arg("open"),
            py::arg("high"),
            py::arg("low"),
            py::arg("close"),
            py::arg("dates") = py::none(),
            py::arg("linked_subplot_idx") = 0,
            py::arg("up_color") = defaultCandlestickSettings.upColor,
            py::arg("down_color") = defaultCandlestickSettings.downColor,
            py::arg("mode") = "full",
            py::arg("candle_width_ratio") = defaultCandlestickSettings.candleWidthRatio,
            py::arg("cap_width_ratio") = defaultCandlestickSettings.capWidthRatio,
            py::arg("line_mode_linewidth") = defaultCandlestickSettings.lineModeLinewidth,
            py::arg("line_mode_miter_limit") = defaultCandlestickSettings.lineModeMiterLimit,
            py::arg("line_mode_basic_line") = defaultCandlestickSettings.lineModeBasicLine,
            py::keep_alive<1, 2>(),  // self keeps open
            py::keep_alive<1, 3>(),  // self keeps high
            py::keep_alive<1, 4>(),  // self keeps low
            py::keep_alive<1, 5>(),   // self keeps close
            py::keep_alive<1, 6>()   // self keeps dates
        )
        .def("candlestick",
             [](Plotter& self,
                py::array_t<float> open,
                py::array_t<float> high,
                py::array_t<float> low,
                py::array_t<float> close,
                std::optional<TimepointVectorRef> dates,
                int linkedSubplotIdx,
                std::vector<float> upColor,
                std::vector<float> downColor,
                std::string mode,
                double candleWidthRatio,
                double capWidthRatio,
                double lineModeLinewidth,
                double lineModeMiterLimit,
                bool lineModeBasicLine
                )
             {
                callCandlestickPlot(
                    self, open, high, low, close, dates, linkedSubplotIdx, upColor, downColor, mode, candleWidthRatio, capWidthRatio, lineModeLinewidth, lineModeMiterLimit, lineModeBasicLine
                );
             },
             py::arg("open"),
             py::arg("high"),
             py::arg("low"),
             py::arg("close"),
             py::arg("dates") = py::none(),
             py::arg("linked_subplot_idx") = 0,
             py::arg("up_color") = defaultCandlestickSettings.upColor,
             py::arg("down_color") = defaultCandlestickSettings.downColor,
             py::arg("mode") = "full",
             py::arg("candle_width_ratio") = defaultCandlestickSettings.candleWidthRatio,
             py::arg("cap_width_ratio") = defaultCandlestickSettings.capWidthRatio,
             py::arg("line_mode_linewidth") = defaultCandlestickSettings.lineModeLinewidth,
             py::arg("line_mode_miter_limit") = defaultCandlestickSettings.lineModeMiterLimit,
             py::arg("line_mode_basic_line") = defaultCandlestickSettings.lineModeBasicLine,
             py::keep_alive<1, 2>(),  // self keeps open
             py::keep_alive<1, 3>(),  // self keeps high
             py::keep_alive<1, 4>(),  // self keeps low
             py::keep_alive<1, 5>(),  // self keeps close
             py::keep_alive<1, 6>()   // self keeps dates
        )

        .def("line",
            [](Plotter& self,
                py::array_t<float> yData,
                std::optional<StringVectorRef> dates,
                int linkedSubplotIdx,
                std::vector<float> color,
                double width,
                double miterLimit,
                bool basicLine
            )
            {
                callLinePlot(self, yData, dates, linkedSubplotIdx, color, width, miterLimit, basicLine);
            },
            py::arg("y"),
            py::arg("dates") = py::none(),
            py::arg("linked_subplot_idx") = 0,
            py::arg("color") = defaultLineSettings.color,
            py::arg("width") = defaultLineSettings.width,
            py::arg("miter_limit") = defaultLineSettings.miterLimit,
            py::arg("basic_line") = defaultLineSettings.basicLine,
            py::keep_alive<1, 2>(),  // self keeps yData
            py::keep_alive<1, 3>()   // self keeps dates
        )
        .def("line",
            [](Plotter& self,
               py::array_t<float> yData,
               std::optional<TimepointVectorRef> dates,
               int linkedSubplotIdx,
               std::vector<float> color,
               double width,
               double miterLimit,
               bool basicLine
               )
            {
               callLinePlot(self, yData, dates, linkedSubplotIdx, color, width, miterLimit, basicLine);
            },
            py::arg("y"),
            py::arg("dates") = py::none(),
            py::arg("linked_subplot_idx") = 0,
            py::arg("color") = defaultLineSettings.color,
            py::arg("width") = defaultLineSettings.width,
            py::arg("miter_limit") = defaultLineSettings.miterLimit,
            py::arg("basic_line") = defaultLineSettings.basicLine,
            py::keep_alive<1, 2>(),  // self keeps yData
            py::keep_alive<1, 3>()
        )

        .def("bar",
             [](Plotter& self,
                py::array_t<float> yData,
                std::optional<StringVectorRef> dates,
                int linkedSubplotIdx,
                std::vector<float> color,
                double widthRatio,
                std::optional<float> minValue
             )
            {
                callBarPlot(self, yData, dates, linkedSubplotIdx, color, widthRatio, minValue);
            },
             py::arg("y"),
             py::arg("dates") = py::none(),
             py::arg("linked_subplot_idx") = 0,
             py::arg("color") = defaultBarSettings.color,
             py::arg("width_ratio") = defaultBarSettings.widthRatio,
             py::arg("min_value") = py::none(),
             py::keep_alive<1, 2>(),  // self keeps yData
             py::keep_alive<1, 3>()   // self keeps dates
        )
        .def("bar",
             [](Plotter& self,
                py::array_t<float> yData,
                std::optional<TimepointVectorRef> dates,
                int linkedSubplotIdx,
                std::vector<float> color,
                double widthRatio,
                std::optional<float> minValue
                )
             {
                 callBarPlot(self, yData, dates, linkedSubplotIdx, color, widthRatio, minValue);
             },
             py::arg("y"),
             py::arg("dates") = py::none(),
             py::arg("linked_subplot_idx") = 0,
             py::arg("color") = defaultBarSettings.color,
             py::arg("width_ratio") = defaultBarSettings.widthRatio,
             py::arg("min_value") = py::none(),
             py::keep_alive<1, 2>(),  // self keeps yData
             py::keep_alive<1, 3>()
        )

        // Scatter variant is okay because its not within an optional
        .def("scatter",
             [](Plotter& self,
                std::variant<py::array_t<int>, StringVectorRef, TimepointVectorRef> xData,
                py::array_t<float> yData,
                int linkedSubplotIdx,
                std::string shape,
                std::vector<float> color,
                bool fixedSize,
                double markerSizeFixed,
                double markerSizeFree
             )
             {
                 py::buffer_info bufferY = yData.request();
                 float* yPtr = static_cast<float*>(bufferY.ptr);
                 std::size_t ySize = bufferY.shape[0];

                 ScatterSettings settings {
                     scatterShapeStrToEnum(shape),
                     color,
                     fixedSize,
                     markerSizeFixed,
                     markerSizeFree
                 };

                 if (std::holds_alternative<StringVectorRef>(xData))
                 {
                     StringVectorRef x = std::get<StringVectorRef>(xData);
                     self.scatter(x, yPtr, ySize, settings, linkedSubplotIdx);
                 }
                 else if (std::holds_alternative<TimepointVectorRef>(xData))
                 {
                     TimepointVectorRef x = std::get<TimepointVectorRef>(xData);
                    self.scatter(x, yPtr, ySize, settings, linkedSubplotIdx);
                 }
                 else
                 {
                     py::array_t<int> x = std::get<py::array_t<int>>(xData);

                     py::buffer_info bufferX = x.request();
                     int* xPtr = static_cast<int*>(bufferX.ptr);
                     std::size_t xSize = bufferX.shape[0];

                     self.scatter(xPtr, xSize, yPtr, ySize, settings, linkedSubplotIdx);

                 }
             },
            py::arg("x"),
            py::arg("y"),
            py::arg("linked_subplot_idx") = 0,
            py::arg("shape") = "cross",
            py::arg("color") = defaultScatterSettings.color,
            py::arg("fixed_size") = defaultScatterSettings.fixedSize,
            py::arg("marker_size_fixed") = defaultScatterSettings.markerSizeFixed,
            py::arg("marker_size_free") = defaultScatterSettings.markerSizeFree,
            py::keep_alive<1, 2>(),    // self keeps x
            py::keep_alive<1, 3>()    // self keeps y
        );




    /* Toy Data
   -------------------------------------------------------------------------*/

    m.def("get_toy_candlestick_data",
          [](int N, float phi, float startValue,
             std::optional<std::chrono::system_clock::time_point> startDate,
             std::chrono::duration<float> timeResolution,
             float errorStd, float highLowStd,
             float volumeMean, float volumeStd, float volumeMin,
             std::optional<unsigned int> seed)
          {

              ToyCandleStickdataSettings settings{};
              settings.phi = phi;
              settings.startValue = startValue;
              settings.timeResolution = timeResolution;
              settings.errorStd = errorStd;
              settings.highLowStd = highLowStd;
              settings.volumeMean = volumeMean;
              settings.volumeStd = volumeStd;
              settings.volumeMin = volumeMin;
              settings.seed = seed;

              CandleData cd = getToyCandlestickData(N, settings);

              py::dict out;
              out["open"]   = arr_copy(cd.open);
              out["high"]   = arr_copy(cd.high);
              out["low"]    = arr_copy(cd.low);
              out["close"]  = arr_copy(cd.close);
              out["volume"] = arr_copy(cd.volume);

              // dates -> Python datetime list (copy; small)
              py::list dts;
              for (auto& tp : cd.dates)
                {

                  dts.append(tp);
              }
              out["dates"] = std::move(dts);

              return out;
          },
          py::arg("N"),
          py::arg("phi") = defaultToyCandleStickdataSettings.phi,
          py::arg("start_value") = defaultToyCandleStickdataSettings.startValue,
          py::arg("start_date") = defaultToyCandleStickdataSettings.startDate,
          py::arg("time_resolution") = defaultToyCandleStickdataSettings.timeResolution,   // timedelta accepted
          py::arg("error_std") = defaultToyCandleStickdataSettings.errorStd,
          py::arg("high_low_std") = defaultToyCandleStickdataSettings.highLowStd,
          py::arg("volume_mean") = defaultToyCandleStickdataSettings.volumeMean,
          py::arg("volume_std") = defaultToyCandleStickdataSettings.volumeStd,
          py::arg("volume_min") = defaultToyCandleStickdataSettings.volumeMin,
          py::arg("seed") = defaultToyCandleStickdataSettings.seed
        );
};

