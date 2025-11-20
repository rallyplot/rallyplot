from rallyplot import Plotter
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def test_all_dataframe_functions():

    x = np.random.normal(size=10_000)

    open = x
    close = x + 5
    low = open.copy()
    high = close.copy()

    df = pd.DataFrame({
        "Open": open,
        "Close": close,
        "Low": low,
        "High": high,
    })

    # Generate 10k datetimes starting from 2023-01-01, one minute apart
    dates = pd.date_range(start="2023-01-01", periods=10_000, freq="T", tz="UTC")
    dates_str = dates.astype(str).tolist()

    # just test every pandas input and check there are no crashes.

    plotter = Plotter()

    plotter.candlestick_from_df(df)
    plotter.start()

    plotter.candlestick_from_df(df, dates)
    plotter.start()

    plotter.candlestick_from_df(df, dates_str)
    plotter.start()

    plotter.candlestick(open, high, low, close, dates)
    plotter.start()

    plotter.candlestick(open, high, low, close, dates_str)
    plotter.start()

    plotter.line(df["Open"], dates)
    plotter.start()

    plotter.line(df["Open"], dates_str)
    plotter.start()

    plotter.bar(df["Open"], dates)
    plotter.start()

    plotter.bar(df["Open"], dates_str)
    plotter.start()

    plotter.line(df["Open"], dates)
    plotter.scatter(dates, df["Open"])
    plotter.start()

    plotter.line(df["Open"], dates_str)
    plotter.scatter(dates_str, df["Open"])
    plotter.start()

    plotter.line(df["Open"])
    plotter.scatter(np.arange(dates.size), df["Open"])
    print(type(df["Open"]))
    plotter.start()


test_all_dataframe_functions()