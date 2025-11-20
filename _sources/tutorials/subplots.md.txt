# Subplots

In `rallyplot`, a 'subplot' is a separate graph with independent x- and y-axis to all other 
existing plots. This is in contrast to a [linked-subplot](linked_subplots.md), in which the x-axis are shared between plots.

A new subplot can be created with the `add_subplot()` (Python) / `addSubplot()` (C++)
function. This function takes 4 arguments:

- *row*: The row (zero-indexed) position in the window where the subplot will be placed
- *column*: The zero-indexed column position in the window where the subplot will be placed
- *row stretch*: The number of rows the subplot spans
- *column stretch*: The number of columns the subplot spans

The most recently added subplot becomes the _active_ subplot. All subsequent plotting and setting function calls
will apply to that subplot only. When adding a new subplot, existing settings will be applied to the new subplot. 
To go back and edit a subplot, use `set_active_subplot(row, col)` (Python) / `setActiveSubplot(row, col)` (C++). 
See the example scripts below for practical demonstrations of these features.


```{image} /_static/img/subplots600.png
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

plotter = Plotter(anti_aliasing_samples=8)

open_high_low_close_df, volume, dates = get_toy_candlestick_data(N=100_000)

# Plot a candlestick chart
plotter.candlestick_from_df(
    open_high_low_close_df, dates
)

# add a linked subplot and plot onto it
plotter.add_linked_subplot(0.33)

y = open_high_low_close_df["open"].to_numpy()

plotter.line(y)

plotter.set_legend(["Subplot Data 1.1"], linked_subplot_idx=0)
plotter.set_legend(["Subplot Data 1.2"], linked_subplot_idx=1)

# Now, add a new subplot. This will be positioned to the
# right of the original subplot. As soon as we add a new
# subplot it becomes the 'active' subplot, and all 'plotter'
# calls affect that subplot
plotter.add_subplot(0, 1, 1, 1)

plotter.line(volume, color=(0.25, 0.22, 0.66), width=1.0)

plotter.set_legend(["Subplot Data 2"])

# We can change settings on this subplot. These settings will
# be remembered and used for all future subplots.
plotter.set_crosshair_settings(on=False)

# Add another subplot, underneath the two existing subplots.
# Set the stretch to '2' to span the entire width of the window.
plotter.add_subplot(1, 0, 1, 2)

plotter.candlestick_from_df(open_high_low_close_df, dates)


# We can add a linked subplot to a subplot as usual.
plotter.add_linked_subplot(0.33)

plotter.bar(volume)

plotter.set_legend(["Subplot Data 3.1"], linked_subplot_idx=0)
plotter.set_legend(["Subplot Data 3.2"], linked_subplot_idx=1)

# We can set the active subplot to operate on
# particular subplots as desired
for idx in ((0, 0), (0, 1), (1, 0)):
    plotter.set_active_subplot(*idx)
    plotter.set_title("My Plot Title")
    plotter.set_x_label("My X Label")
    plotter.set_y_label("My Y  Label")
    plotter.set_hover_value_settings(display_mode="off")


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
    PlotterArgs plotterArgs{};
    plotterArgs.antiAliasingSamples = 8;

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

    // add a linked subplot and plot onto it
    plotter.addLinkedSubplot(0.33);

    const std::vector<float>& y = candleData.open;

    plotter.line(y);

    // The second argument, linkedSubplotIdx, specified
    // the linekd subplot to set the legend on
    plotter.setLegend({"Subplot Data 1.1"}, std::nullopt, 0);
    plotter.setLegend({"Subplot Data 1.2"}, std::nullopt, 1);

    // Now, add a new subplot. This will be positioned to the
    // right of the original subplot. As soon as we add a new
    // subplot it becomes the 'active' subplot, and all 'plotter'
    // calls affect that subplot
    plotter.addSubplot(0, 1, 1, 1);

    LineSettings lineSettings{};
    lineSettings.color = {0.25, 0.22, 0.66};
    lineSettings.width = 1.0f;
    plotter.line(candleData.volume, lineSettings);

    plotter.setLegend({"Subplot Data 2"});

    // We can change settings on this subplot. These settings will
    // be remembered and used for all future subplots.
    CrosshairSettings crosshairSettings;
    crosshairSettings.on = false;
    plotter.setCrosshairSettings(crosshairSettings);

    // Add another subplot, underneath the two existing subplots.
    // Set the stretch to '2' to span the entire width of the window.
    plotter.addSubplot(1, 0, 1, 2);


    // Plot a candlestick chart
    plotter.candlestick(
        candleData.open,
        candleData.high,
        candleData.low,
        candleData.close,
        candleData.dates
        );

    // We can add a linked subplot to a subplot as usual.
    plotter.addLinkedSubplot(0.33);

    plotter.bar(candleData.volume);

    plotter.setLegend({"Subplot Data 3.1"}, std::nullopt, 0);
    plotter.setLegend({"Subplot Data 3.2"}, std::nullopt, 1);

    // We can set the active subplot to operate on
    // particular subplots as desired

    HoverValueSettings hoverValueSettings{};
    hoverValueSettings.displayMode = HoverValueDisplayMode::off;

    for (std::pair<int,int> idx : std::initializer_list<std::pair<int,int>>{{0, 0}, {0, 1}, {1, 0}})
    {
        plotter.setActiveSubplot(idx.first, idx.second);
        plotter.setTitle("My Plot Title");
        plotter.setXLabel("My X Label");
        plotter.setYLabel("My Y  Label");
        plotter.setHoverValueSettings(hoverValueSettings);
    }

    plotter.start();
}

```

:::::