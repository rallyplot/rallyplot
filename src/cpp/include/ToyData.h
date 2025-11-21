#include <vector>
#include <chrono>
#include <random>
#include <optional>

using namespace std::chrono;

struct CandleData {
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> volume;
    std::vector<system_clock::time_point> dates;
};


struct ToyCandleStickdataSettings {
    float phi = 0.999999;
    float startValue = 2000.0;
    std::optional<system_clock::time_point> startDate = std::nullopt;
    duration<float> timeResolution = seconds(1);
    float errorStd = 2.0;
    float highLowStd = 0.5;
    float volumeMean = 1000.0;
    float volumeStd = 250.0;
    float volumeMin = 10.0;
    std::optional<unsigned int> seed = std::nullopt;
};

inline CandleData getToyCandlestickData(
    int N,
    ToyCandleStickdataSettings cfg = ToyCandleStickdataSettings{}
)
{
    std::vector<float> open(N);
    std::vector<float> high(N);
    std::vector<float> low(N);
    std::vector<float> close(N);

    if (!cfg.seed.has_value())
    {
        cfg.seed = static_cast<unsigned int>(system_clock::now().time_since_epoch().count());
    }
    std::default_random_engine generator(cfg.seed.value());
    std::normal_distribution<float> norm_dist(0.0, cfg.errorStd);
    std::normal_distribution<float> highLowDist(0, cfg.highLowStd);
    std::normal_distribution<float> volumeDist(1000, 250);

    std::vector<float> volume(N);
    std::vector<system_clock::time_point> dates(N);

    if (!cfg.startDate.has_value())
    {
        std::tm tm = {};
        tm.tm_year = 2020 - 1900;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        std::time_t tt = std::mktime(&tm);
        cfg.startDate = system_clock::from_time_t(tt);
    }
    auto resolution = seconds(1);

    open[0] = cfg.startValue;
    close[0] = open[0] * cfg.phi + norm_dist(generator);

    for (int i = 1; i < N; i++)
    {
        open[i] = close[i - 1];
        close[i] = close[i - 1] * cfg.phi + norm_dist(generator);

        high[i] = std::max(close[i], open[i]) + std::abs(highLowDist(generator));
        low[i] = std::min(close[i], open[i]) - std::abs(highLowDist(generator));

        volume[i] = std::max(volumeDist(generator), 10.f);
        dates[i] = cfg.startDate.value() + i * resolution;
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

