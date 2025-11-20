# Plotting Buy / Sell Orders

This example shows how you can use a [scatter plot](scatter-tutorial) 
to show buy and sell orders on a [candlestick](candlestick-tutorial) chart.


```{image} /_static/img/example-scatter600.png
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
from datetime import datetime

# Let's create some sythnetic data, and pretend we have past buy
# and sell orders we wish to  visualise at the given indices.
dataset_df, volume, dates = get_toy_candlestick_data(N=10_000, seed=42)

buy_indices = np.array([100, 250, 5001, 7501, 9999], dtype=int)
sell_indices = np.array([151, 310, 5011, 7851], dtype=int)

# Index out the prices and dates to plot.
buy_prices = np.ascontiguousarray(
    dataset_df["close"][buy_indices]
)
buy_dates = np.ascontiguousarray(
    np.array(dates)[buy_indices]
)
sell_prices = np.ascontiguousarray(
    dataset_df["close"][sell_indices]
)

plotter = Plotter(anti_aliasing_samples=8)

plotter.candlestick_from_df(dataset_df, dates)

# We plot the buy orders at the given dates / prices.
plotter.scatter(buy_dates, buy_prices, shape="triangle_up", color=(0.0, 1.0))

# We can also just pass the indices directly for the
# x-axis positions of the scatter plot.
plotter.scatter(sell_indices, sell_prices, shape="triangle_down", color=(1.0))

plotter.start()


```

:::::

:::::{tab-item} C++
:sync: cpp

```cpp
    
    // Let's create some sythnetic data, and pretend we have past buy
    // and sell orders we wish to  visualise at the given indices.
    ToyCandleStickdataSettings toySettings{};
    toySettings.seed = 42;
    CandleData candleData = getToyCandlestickData(10000, toySettings);


    std::vector<int> buyIndices = {100, 250, 5001, 7501, 9999};
    std::vector<int> sellIndices = {151, 310, 5011, 7851};

    std::vector<float> buyPrices;
    std::vector<std::chrono::system_clock::time_point> buyDates;
    std::vector<float> sellPrices;

    // Index out the prices and dates to plot.
    for (const int& idx : buyIndices)
    {
        buyPrices.push_back(candleData.close[idx]);
        buyDates.push_back(candleData.dates[idx]);
    }
    for (const int& idx : sellIndices)
    {
        sellPrices.push_back(candleData.close[idx]);
    }

    PlotterArgs plotterArgs{};
    plotterArgs.antiAliasingSamples = 8;

    Plotter plotter = Plotter(plotterArgs);

    plotter.candlestick(
        candleData.open, candleData.high, candleData.low, candleData.close, candleData.dates
    );
    
    // We plot the buy orders at the given dates / prices.
    ScatterSettings scatterSettings{};
    scatterSettings.color = {0.0f, 1.0f};
    scatterSettings.shape = ScatterShape::triangleUp;
    plotter.scatter(buyDates, buyPrices, scatterSettings);
    
    // We can also just pass the indices directly for
    // the x-axis positions of the scatter plot.
    scatterSettings.color = {1.0f};
    scatterSettings.shape = ScatterShape::triangleDown;
    plotter.scatter(sellIndices, sellPrices, scatterSettings);

    plotter.start();

```

:::::

::::::