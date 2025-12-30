from rallyplot import Plotter
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


CHECK = False


def start_if_required(plotter):
    if CHECK:
        plotter.start()


def test_all_dataframe_functions():
    """
    Smoke test all functions that use dataframes as input.
    These are lightweight wrappers around the plotting subfunctions
    and so only smoke tests are currently performed.
    """
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
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.candlestick_from_df(df, dates)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.candlestick_from_df(df, dates_str)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.candlestick(open, high, low, close, dates)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.candlestick(open, high, low, close, dates_str)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.line(df["Open"], dates)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.line(df["Open"], dates_str)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.bar(df["Open"], dates)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.bar(df["Open"], dates_str)
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.line(df["Open"], dates)
    plotter.scatter(dates, df["Open"])
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.line(df["Open"], dates_str)
    plotter.scatter(dates_str, df["Open"])
    start_if_required(plotter)
    plotter.finish()

    plotter = Plotter()
    plotter.line(df["Open"])
    plotter.scatter(np.arange(dates.size), df["Open"])
    start_if_required(plotter)
    plotter.finish()

test_all_dataframe_functions()