(line-tutorial)=
# Line Plot

This will plot a simple line chart. There are two rendering modes for the line plot set with `basic_line` (Python)
/ `basicLine` (C++) argument. When True, a simple line plot will be used which is significantly
faster, however the line width will not be adjustable. When false, the line width will be adjustable, but
the plot performance will be slower.

For larger line widths (when basic mode is False), visual artefacts may occur at the joints between
line segments, in particular when zooming. This can be mitigated by the miter limit argument,
`miter_limit` (Python) /  `miterLimit` (C++), which controls how far the corner of a line segment extends.
argument. Note this is not a perfect solution for very thick lines, and there is still
[room for improvement in the current implementation](roadmap).

See below for an example demonstrating how both options can be used.

```{image} /_static/img/line-plot600.png
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
from rallyplot import get_toy_candlestick_data

plotter = Plotter(
    color_mode="dark"
)


open_high_low_close_df, volume, dates = get_toy_candlestick_data(N=100_000)

data = open_high_low_close_df["open"]

# The 'basic' lineplot has a fixed line width, but is
# significantly based that `basic=False`. It looks best
# with no antialiasing.
plotter.line(
    y=data,
    dates=dates,
    basic_line=True,
    color=(0.94, 0.96, 0.26)
)

plotter.add_linked_subplot(0.5)

# For thick lines, the `miter_limit` may need increasing. This can
# lead to some visual artefacts and should be adjusted carefully.
plotter.line(
    y=data,
    width=1.5,
    miter_limit=3.75,
    color=(0.75, 0.75, 1.0)
)

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
    CandleData candleData = getToyCandlestickData(100000);

    const std::vector<float>& data = candleData.open;

    PlotterArgs plotterArgs{};
    plotterArgs.colorMode = ColorMode::dark;

    Plotter plotter = Plotter(plotterArgs);

    // The 'basic' lineplot has a fixed line width, but is
    // significantly based that `basic=False`. It looks best
    // with no antialiasing.
    LineSettings lineSettings{};

    lineSettings.basicLine = true;
    lineSettings.color = {0.94, 0.96, 0.26};

    plotter.line(
        candleData.open,
        candleData.dates,
        lineSettings
        );

    plotter.addLinkedSubplot(0.5f);

    // For thick lines, the `miter_limit` may need increasing. This can
    // lead to some visual artefacts and should be adjusted carefully.
    lineSettings.basicLine = false;
    lineSettings.width = 1.5f;
    lineSettings.miterLimit = 4.25f;
    lineSettings.color = {0.75, 0.75, 1.0};

    plotter.line(
        data,
        lineSettings
        );

    plotter.start();

}


```

:::::

::::::