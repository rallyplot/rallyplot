# Bar Plot

The bar plot displays bars whose heights correspond to the values in the provided data array.
By default, each bar starts at 0. The `min_value` (Python) / `minValue` (C++) argument can be used to
set the starting point (min value) of the bars. This acts as an offset — the heights of the bars are always
determined by the input data, while the baseline shifts their vertical starting point.

The spacing between the bars can be controlled with the `width_ratio` (Python) / `widthRatio` (C++) argument.
This value should be between `0` and `1`, representing the ratio of bar width to the total space between
consecutive bars (e.g., `0.5` gives equal bar and gap widths, `1` results in adjacent bars with no gaps).


```{image} /_static/img/bar-plot600.png
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
import numpy as np

plotter = Plotter()

_, volume, dates = get_toy_candlestick_data(N=100_000)

min_y = 100
max_y = np.max(volume) + min_y

plotter.bar(
    volume,
    dates=dates,
    width_ratio=0.9,
    min_value=min_y,  # add some padding at the bottom
)

# Add some padding around the plot top and bottom
plotter.set_y_limits(min_y - 100, max_y + 100)

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
    const std::vector<float>& volume = candleData.volume;

    Plotter plotter = Plotter();

    float minY = 100.0f;
    float maxY = *std::max_element(volume.begin(), volume.end()) + minY;

    BarSettings barSettings{};
    barSettings.widthRatio = 0.9f;
    barSettings.minValue = minY;  // add some padding at the bottom

    // Let's fix the y-limits with some padding for nicer visualisation
    plotter.bar(
        candleData.volume,
        candleData.dates,
        barSettings
        );

    // Add some padding around the plot top and bottom
    plotter.setYLimits(minY - 100.0f, maxY + 100.0f);  

    plotter.start();

}
```

:::::

::::::