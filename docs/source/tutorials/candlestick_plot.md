(candlestick-tutorial)=
# Candlestick Plot

The candlestick function (`candlestick()`) generates a candlestick plot from the
passed `open`, `high`, `low`, `close` prices, which should be equal-length vectors.

In Python, `candelstick_from_df()` can be used to plot a candlestick chart
given a Pandas DataFrame with columns "open", "close", "high", "low" (case-insensitive).

Pressing 'Enter' on the interactive plot will cycle between different candle-stick plot
modes, including:

- *Full*: standard candlesticks with upper and lower shadows (wicks).
- *No caps*: candlesticks without shadow caps.
- *Body only*: only the candle body is drawn, without shadows.
- *Line (open)*: a gradient-colored line drawn through the open prices.
- *Line (close)*: a gradient-colored line drawn through the close prices.

The mode can be passed directly to the plotting function, see the [API docs](api-reference-page) for
the full argument list.

```{image} /_static/img/candlestick-plot600.png
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
    anti_aliasing_samples=8,
)


open_high_low_close_df, volume, dates = get_toy_candlestick_data(N=100_000)

# Split out the difference prices to highlight the `candlestick() API.
# In reality we could use candlestick_from_df() here.
open_ = open_high_low_close_df["open"]
high_ = open_high_low_close_df["high"]
low_ = open_high_low_close_df["low"]
close_ = open_high_low_close_df["close"]

line_mode_linewidth = 1.25
candle_width_ratio = 1.0
cap_width_ratio = 1.0

plotter.candlestick(
    open=open_,
    high=high_,
    low=low_,
    close=close_,
    dates=dates,
    mode="full",
    candle_width_ratio=0.8,
    cap_width_ratio=0.8,
    line_mode_linewidth=1.5
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

    Plotter plotter = Plotter();

    CandlestickSettings candlestickSettings;

    // Start the plot in line mode (open prices)
    candlestickSettings.mode = CandlestickMode::full;

    // Set the width of the line when in line mode
    candlestickSettings.lineModeLinewidth = 1.25f;

    // Set the width of the candle (as a ratio of space between
    // candles, 1.0f is no space between candles, 0.5f is the space
    // between candles equals the candle width). Similar for the
    // shadow cap width.
    candlestickSettings.candleWidthRatio = 0.8f;
    candlestickSettings.capWidthRatio = 0.8f;

    plotter.candlestick(
        candleData.open,
        candleData.high,
        candleData.low,
        candleData.close,
        candlestickSettings
        );

    plotter.start();

}

```

:::::

::::::