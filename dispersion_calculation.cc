//
// Created by eugene on 23.10.22.
//

#include <optional>
#include <cmath>
#include <tuple>

#include "dispersion_types.h"
#include "dispersion_results.h"
#include "dispersion_calculation.h"

Results calculateMeanAndDispersionAsWhole(const DataContainer &data)
{
    const size_t n = data.size();
    if (n == 0) {
        return Results(std::nullopt);
    }
    ValueType sum{};
    ValueType sumOfSquares{};
    for (const auto &value : data) {
        sum += value;
        sumOfSquares += std::pow(value, 2);
    }
    ValueType mean = sum / data.size();
    ValueType dispersion = sumOfSquares / n - std::pow(mean, 2);
    return Results(std::tuple{mean, dispersion, sum, n});
}

Results calculateMeanAndDispersionAsWhole(const std::vector<DataContainer> &data)
{
    ValueType sum{};
    ValueType sumOfSquares{};
    size_t n = 0;
    for (const auto &container : data) {
        n += container.size();
        for (const auto &value : container) {
            sum += value;
            sumOfSquares += std::pow(value, 2);
        }
    }
    if (n == 0) {
        return Results(std::nullopt);
    }
    ValueType mean = sum / n;
    ValueType dispersion = sumOfSquares / n - std::pow(mean, 2);
    return Results(std::tuple{mean, dispersion, sum, n});
}

Results calculateMeanAndDispersionFromParts(const std::vector<Results> &data)
{
    ValueType weightedDispersion{};
    ValueType weightedMean{};
    size_t n = 0;
    ValueType sum{};
    for (const auto &part : data) {
        if (part.has_values()) {
            weightedDispersion += part.count() * part.dispersion();
            weightedMean += part.count() * part.mean();
            n += part.count();
            sum += part.sum();
        }
    }
    weightedMean /= n;
    ValueType mean = sum / n;
    ValueType weightedVariance{};
    for (const auto &part : data) {
        if (part.has_values()) {
            weightedVariance += part.count() * std::pow(weightedMean - part.mean(), 2);
        }
    }
    ValueType dispersion = (weightedDispersion + weightedVariance) / n;
    return Results(std::tuple{mean, dispersion, sum, n});
}
