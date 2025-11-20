(scatter-tutorial)=
# Scatterplot

The scatter plot can be used to overlay individual points (x, y) on top of an existing plot. Currently,
the scatterplot cannot be the first plot on a subplot, it must be overlaid onto an existing plot.

The main use case of the scatter plot is to plot trade entry and exit points over a chart of prices.

In `rallyplot`, x-axis labels may be omitted (in which case the integer positions are used by default) or explicitly
provided as an array of strings or UTC datetimes (see [here](passing-dates) for more details). When using the scatter plot, x-positions 
can be specified as integers (which always work), or as an array of strings or datetimes. 
If strings or datetimes are used, their types must match the x-axis label type already used in the chart.


```{image} /_static/img/scatter-plot600.png
:alt: GPU accelerated plotting demo
:class: sd-m-auto sd-rounded-lg sd-shadow-md
:align: center
:width: 60%
```


::::::{tab-set}

:::::{tab-item} Python
:sync: python

```python

import numpy as np
from rallyplot import Plotter
from rallyplot import get_toy_candlestick_data

candle_df, _, dates = get_toy_candlestick_data(N=100_000)
prices = candle_df["open"].to_numpy()

plotter = Plotter(
    anti_aliasing_samples=8,
)

plotter.line(
    prices,
    dates=dates,
)

scatter_dates = np.ascontiguousarray(dates[::1000])
scatter_prices = np.ascontiguousarray(prices[::1000])

plotter.scatter(
    scatter_dates,
    scatter_prices,
    shape="cross",
    color=(0.5, 0.0, 0.5, 1.0),
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

    std::vector<float> prices;
    std::vector<std::chrono::system_clock::time_point> dates;

    for (int i = 0; i < 100; i++)
    {
        prices.push_back(candleData.open[i * 1000]);
        dates.push_back(candleData.dates[i * 1000]);
    }

    Plotter plotter = Plotter();

    plotter.line(
        candleData.open,
        candleData.dates
        );

    ScatterSettings scatterSettings;
    scatterSettings.shape = ScatterShape::cross;
    scatterSettings.color = {0.5, 0.0, 0.5, 1.0};

    plotter.scatter(
        dates,
        prices,
        scatterSettings
        );

    plotter.start();

}

```

:::::

::::::