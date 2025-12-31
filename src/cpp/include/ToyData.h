#include <vector>
#include <chrono>
#include <random>
#include <optional>
#include <ctime>
#include <algorithm>
#include <cmath>

using namespace std::chrono;

struct CandleData
/*
    Data for a single candle.
*/
{
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> volume;
    std::vector<system_clock::time_point> dates;
};


struct ToyCandleStickdataSettings
/*
    Settings for generation of candlestick data by an AR1 process
    and high / low values drawn from a zero mean gaussian (with `highLowStd`).
    Volumes are drawn from a gaussian (`volumeMean`, `volumeStd`) with minimum value `volumeMin`.
    Dates start at `startDate` and increment with time `timeResolution`.
*/
{
    float phi = 0.999999f;
    float startValue = 2000.0f;
    std::optional<system_clock::time_point> startDate = std::nullopt;
    duration<double> timeResolution = seconds(1);
    float errorStd = 2.0f;
    float highLowStd = 0.5f;
    float volumeMean = 1000.0f;
    float volumeStd = 250.0f;
    float volumeMin = 10.0f;
    std::optional<unsigned int> seed = std::nullopt;
};


inline CandleData getToyCandlestickData(
    int N,
    ToyCandleStickdataSettings cfg = ToyCandleStickdataSettings{}
)
/*
    Create some toy data according to an AR1 process:
        close = open * Φ + ε

    `N` is the number of datapoints to generate.
    See `ToyCandleStickdataSettings` for other arguments.
*/
{
    std::vector<float> open(N);
    std::vector<float> high(N);
    std::vector<float> low(N);
    std::vector<float> close(N);

    // set up distributions to draw from
    if (!cfg.seed.has_value())
    {
        cfg.seed = static_cast<unsigned int>(system_clock::now().time_since_epoch().count());
    }
    std::default_random_engine generator(cfg.seed.value());
    std::normal_distribution<float> norm_dist(0.0, cfg.errorStd);
    std::normal_distribution<float> highLowDist(0, cfg.highLowStd);
    std::normal_distribution<float> volumeDist(cfg.volumeMean, cfg.volumeStd);

    std::vector<float> volume(N);
    std::vector<system_clock::time_point> dates(N);

    // Set up the start date for `dates`
    if (!cfg.startDate.has_value())
    {
        std::tm tm = {};
        tm.tm_year = 2020 - 1900;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        std::time_t tt = std::mktime(&tm);
        cfg.startDate = system_clock::from_time_t(tt);
    }
    auto timeStep = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        cfg.timeResolution
    );

    // Simulate candlestick data according to an AR1 process
    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            open[0] = cfg.startValue;
            close[0] = open[0] * cfg.phi + norm_dist(generator);
        }
        else
        {
            open[i] = close[i - 1];
            close[i] = close[i - 1] * cfg.phi + norm_dist(generator);
        }

        high[i] = std::max(close[i], open[i]) + std::abs(highLowDist(generator));
        low[i] = std::min(close[i], open[i]) - std::abs(highLowDist(generator));

        volume[i] = std::max(volumeDist(generator), cfg.volumeMin);
        dates[i] = cfg.startDate.value() + i * timeStep;
    }

    CandleData candleData{
        open,
        high,
        low,
        close,
        volume,
        dates
    };

    return candleData;
};

