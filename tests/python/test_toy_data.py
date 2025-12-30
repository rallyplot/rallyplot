from rallyplot import get_toy_candlestick_data
import datetime

def test_toy_candledata():
    """
    Test the toy data generation function. This is used for many
    examples in the documentation so it is important it works.
    """
    dataframe, volumes, dates = get_toy_candlestick_data(1000)

    assert dataframe.shape == (1000, 4)
    assert all(dataframe.columns == ['open', 'high', 'low', 'close'])
    assert volumes.shape
    assert len(dates) == 1000
    assert isinstance(dates[0], datetime.datetime)

    print("Successfully run `test_toy_candledata`.")

test_toy_candledata()