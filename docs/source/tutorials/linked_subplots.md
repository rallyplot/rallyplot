# Linked Subplots

There are two types of subplots in `rallyplot`: 'linked subplots' in which the x-axis is shared between plots,
(and y-axis can be optionally linked) or  'subplots', which refers to completely independent subplots.
In this tutorial we will cover _linked subplots_ (see [here](subplots.md) for a tutorial on subplots).

The `add_linked_subplot(height_as_proportion)` (Python) / `addLinkedSubplot(heightAsProportion)` (C++) 
function is used to create a new linked subplot. This will create the new linked subplot underneath the current
plots, and its argument controls the proportion of space the new linked subplot will take.
See the example script below for more details.

Plots will allways be added to the most recently created linked subplot by default.
To add the plot to a specific linked subplot, use the 
`linked_subplot_idx` (Python) / `linkedSubplotIdx` (C++) arguments to the plotting functions. 
Similarly, settings will be applied to all linked subplots unless the above argument is provided:


```{image} /_static/img/linked-subplots600.png
:alt: GPU accelerated plotting demo
:class: sd-m-auto sd-rounded-lg sd-shadow-md
:align: center
:width: 60%
```

::::::{tab-set}

:::::{tab-item} Python
:sync: python


```python

from rallyplot import Plotter
from rallyplot.plotter import get_toy_candlestick_data

plotter = Plotter()

open_high_low_close_df, volume, dates = get_toy_candlestick_data(N=100_000)

# Plot a candlestick chart
plotter.candlestick_from_df(
    open_high_low_close_df, dates
)

# add a linked subplot and display line and bar plots on it
plotter.add_linked_subplot(0.33)

y = open_high_low_close_df["open"]
plotter.line(y + 25, width=0.75, color=(1.0, 0.5, 0.5))
plotter.line(y - 25, width=0.75, color=(1.0, 0.5, 0.5))

# Set the legend on the individual subplots
plotter.set_legend(["AAPL"], linked_subplot_idx=0)

# Set some camera settings on the subplots. We can apply
# only to the first linked subplot...
plotter.pin_y_axis(False, linked_subplot_idx=0)

# ... or apply to all linked subplots the argument not provided
plotter.set_crosshair_settings(on=False)

# Resize the linked subplots (expressed as a proportion of the window)
plotter.resize_linked_subplots([0.50, 0.50])

# By default, the y-axis of linked subplots is not linked.
# This can be linked if desired. Axis limits will become unpinned.
plotter.link_y_axes(on=True)

# Add another linked subplot because... why not?
plotter.add_linked_subplot(0.15)
plotter.bar(volume)

# Plot onto the non-active linked subplot
plotter.line(y, linked_subplot_idx=1, width=1.5, color=(1.0, 0.25, 0.25))
plotter.set_legend(["+0.5", "-0.5", "Bar"], linked_subplot_idx=1)
plotter.start()

```

:::::

:::::{tab-item} C++
:sync: cpp

```cpp

#include <Plotter.h>
#include <ToyData.h>
#include <vector>

int main(int argc, char* argv[])
{

    Plotter plotter = Plotter();

    CandleData candleData = getToyCandlestickData(100000);

    // Plot a candlestick chart
    plotter.candlestick(
        candleData.open,
        candleData.high,
        candleData.low,
        candleData.close,
        candleData.dates
        );

    // Create some data to plot, the open price data
    // with some secondary lines above and below it.
    const std::vector<float>& y = candleData.open;
    std::vector<float> y_offset_pos(y.size());
    std::vector<float> y_offset_neg(y.size());

    for (int i = 0; i < y_offset_pos.size(); i++)
    {
        y_offset_pos[i] = y[i] + 25.0f;
        y_offset_neg[i] = y[i] - 25.0f;
    }
    // add a linked subplot and display line and bar plots on it
    plotter.addLinkedSubplot(0.33);

    LineSettings lineSettings{};
    lineSettings.width = 0.75f;
    lineSettings.color = {1.0f, 0.5f, 0.5f};

    plotter.line(y_offset_pos, lineSettings);
    plotter.line(y_offset_neg, lineSettings);

    // Set the legend on the first linked subplot
    plotter.setLegend({"AAPL"}, std::nullopt, 0);

    // Set some camera settings on the subplots. We can apply
    // only to the first linked subplot...
    plotter.pinYAxis(false, 0);

    // ... or apply to all linked subplots the argument not provided
    CrosshairSettings crosshairSettings{};
    crosshairSettings.on = false;
    plotter.setCrosshairSettings(crosshairSettings);

    // Resize the linked subplots (expressed as a proportion of the window)
    plotter.resizeLinkedSubplots({0.50, 0.50});

    // By default, the y-axis of linked subplots is not linked.
    // This can be linked if desired. Axis limits will become unpinned.
    plotter.linkYAxis(true);

    // Add another linked subplot because... why not?
    plotter.addLinkedSubplot(0.15);
    plotter.bar(candleData.volume);

    // Plot onto the non-active linked subplot
    lineSettings.width = 1.5f;
    lineSettings.color = {1.0, 0.25, 0.25};

    plotter.line(y, lineSettings, 1);
    plotter.setLegend({"+0.5", "-0.5", "Bar"},  std::nullopt, 1);

    plotter.start();
}

```

:::::

::::::