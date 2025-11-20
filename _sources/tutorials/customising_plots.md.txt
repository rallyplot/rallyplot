# Customising the Plots

The appearance of nearly every element of a plot can be customised. The below example highlight the 
main ways to change the appearance of the plots; refer to the [API documentation](api-reference-page) 
for the full range of options.

To create a bold neon-style plot:

```{image} /_static/img/customising-plots600.png
:alt: A Neon-style plot
:class: sd-m-auto sd-rounded-lg sd-shadow-md
:align: center
:width: 60%
```

:::{warning}

Axis and grid line widths may not change depending on your GPU.

:::

::::::{tab-set}

:::::{tab-item} Python
:sync: python


```python

from rallyplot import Plotter
from rallyplot.plotter import get_toy_candlestick_data

open_high_low_close_df, volume, dates = get_toy_candlestick_data(N=100_000)


# The Plotter() class initialisation exposes a number of arguments that control
# the plot appearance. Axis font and margin widths will apply to all subplots.
plotter = Plotter(
    anti_aliasing_samples=8,
    width=1200,
    height=600,
    axis_tick_label_font="consola",
    axis_tick_label_font_size=15,
    width_margin_size=80,  # spacing between the axis and window
    height_margin_size=30,
    axis_right=True  # set the axis on the left

)

# Set the background color for the subplot
plotter.set_background_color((0.05, 0.0, 0.1, 1.0))

y = open_high_low_close_df["open"]

# We can change the color of plots with `color`
# arguments that the plotting functions expose
plotter.candlestick_from_df(
    open_high_low_close_df,
    dates,
    up_color=(0.0, 1.0, 0.8, 1.0),
    down_color=(1.0, 0.0, 0.6, 0.8),
)

plotter.line(
    y + 10,
    color=(1.0, 0.88, 0.1, 0.7),
    width=0.75,
)

# The appearance of the title and x / y  labels can be customized when they are set.
plotter.set_title("My Neon Plot", font="consola", weight="bold", font_size=25, color=(1.0, 0.2, 0.9))

plotter.set_x_label(
    "X Label", font="consola", weight="bold", font_size=15, color=(1.0, 0.2, 0.9)
)

plotter.set_y_label(
    "Y Label", font="consola", weight="bold", font_size=15, color=(1.0, 0.2, 0.9)
)

# We can adjust the axis label color and axis gridlines, as well as many other
# options, with the axis setting functions
plotter.set_y_axis_settings(
    font_color=(1.0, 0.2, 0.9),
    show_gridline=False
)

plotter.set_x_axis_settings(
    font_color=(1.0, 0.2, 0.9),
    show_gridline=False
)

plotter.set_crosshair_settings(
    linewidth=2,
    line_color=(0.9, 0.2, 1.0, 0.8),
    font="consola",
    font_size=9,
    font_color=(0.0, 1.0, 0.5, 1.0),
    background_color=(0.2, 0.0, 0.2, 0.9)
)

# We can also adjust the appearance of drawn lines
# (hold M click left-clicking on the plot) and the
# pop-up box that displays the plot values at the cursor tip
plotter.set_draw_line_settings(linewidth=2)

plotter.set_hover_value_settings(
    display_mode="always_show",  # press CTRL+S to switch display between plots
    font="consola",
    font_size=9,
    font_color=(0.0, 1.0, 0.5, 1.0),
    background_color=(0.1, 0.0, 0.1, 0.9),
    border_color=(1.0, 0.0, 0.6, 1.0)
)

plotter.start()

```

:::::

:::::{tab-item} C++
:sync: cpp

```cpp

#include <Plotter.h>
#include <ToyData.h>

int main(int argc, char* argv[])
{

 CandleData candleData = getToyCandlestickData(100000);
 
 std::vector<float> y_with_offset(candleData.open.size());
 
 for (int i = 0; i < y_with_offset.size(); i++)
 {
     y_with_offset[i] = candleData.open[i] + 10.0f;
 }
 
 // The Plotter() class initialisation exposes a number of arguments that control
 // the plot appearance. Axis font and margin widths will apply to all subplots.
 PlotterArgs plotterArgs{};
 plotterArgs.antiAliasingSamples = 8;
 plotterArgs.width = 1200;
 plotterArgs.height = 600;
 plotterArgs.axisTickLabelFont = Font::consola;  
 plotterArgs.axisTickLabelFontSize = 15;
 plotterArgs.widthMarginSize = 80;  // spacing between the axis and window
 plotterArgs.heightMarginSize = 30;
 plotterArgs.axisRight = false;  // set the axis on the left
     
 Plotter plotter = Plotter(
     plotterArgs
 );
 
 // Set the background color for the subplot
 plotter.setBackgroundColor({0.05, 0.0, 0.1, 1.0});
 
 // We can change the color of plots with `color`
 // arguments that the plotting functions expose
 CandlestickSettings candlestickSettings;
 candlestickSettings.upColor = {0.0, 1.0, 0.8, 1.0};
 candlestickSettings.downColor = {1.0, 0.0, 0.6, 0.8};
 
 plotter.candlestick(
     candleData.open,
     candleData.high,
     candleData.low,
     candleData.close,
     candleData.dates,
     candlestickSettings
 );
 
 
 LineSettings lineSettings{};
 lineSettings.color = {1.0, 0.88, 0.1, 0.7};
 lineSettings.width = 0.75f;
 plotter.line(
     y_with_offset,
     lineSettings
 );
 
 // The appearance of the title and x / y  labels can be customized when they are set.
 // Set the title
 TitleLabelSettings titleSettings{};
 titleSettings.font = Font::consola;
 titleSettings.weight = "bold";
 titleSettings.fontSize = 25;
 titleSettings.color = {1.0, 0.2, 0.9};
 
 plotter.setTitle("My Neon Plot", titleSettings);
 
 // Set the axis labels
 AxisLabelSettings axisLabelSettings{};
 axisLabelSettings.font = Font::consola;
 axisLabelSettings.weight = "bold";
 axisLabelSettings.fontSize = 15;
 axisLabelSettings.color = {1.0, 0.2, 0.9};
 
 plotter.setXLabel("X Label", axisLabelSettings);
 plotter.setYLabel("Y Label", axisLabelSettings);
 
 
 // We can adjust the axis label color and axis gridlines, as well as many other
 // options, with the axis setting functions
 XAxisSettings xAxisSettings{};
 xAxisSettings.fontColor = {1.0, 0.2, 0.9};
 xAxisSettings.showGridline = false;
 
 YAxisSettings yAxisSettings;
 yAxisSettings.fontColor = {1.0, 0.2, 0.9};
 yAxisSettings.showGridline = false;
 
 plotter.setXAxisSettings(xAxisSettings);
 plotter.setYAxisSettings(yAxisSettings); 
 
 // We can also adjust the appearance of drawn lines
 // (hold M click left-clicking on the plot) and the
 // pop-up box that displays the plot values at the cursor tip
 CrosshairSettings crosshairSetting;
 crosshairSetting.linewidth = 2;
 crosshairSetting.lineColor = {0.9, 0.2, 1.0, 0.8};
 crosshairSetting.font = Font::consola;
 crosshairSetting.fontSize = 9;
 crosshairSetting.fontColor = {0.0, 1.0, 0.5, 1.0};
 crosshairSetting.backgroundColor = {0.2, 0.0, 0.2, 0.9};
 plotter.setCrosshairSettings(crosshairSetting);
 
 HoverValueSettings hoverValueSettings;
 hoverValueSettings.displayMode = HoverValueDisplayMode::alwaysShow; // press CTRL+S to switch display between plots
 hoverValueSettings.font = Font::consola;
 hoverValueSettings.fontSize = 9;
 hoverValueSettings.fontColor = {0.0, 1.0, 0.5, 1.0};
 hoverValueSettings.backgroundColor = {0.1, 0.0, 0.1, 0.9};
 hoverValueSettings.borderColor = {1.0, 0.0, 0.6, 1.0};
 
 plotter.setHoverValueSettings(hoverValueSettings);
 
 plotter.start();

}
```

:::::

::::::