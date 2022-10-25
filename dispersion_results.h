//
// Created by eugene on 23.10.22.
//

#ifndef DISPERSION_DISPERSION_RESULTS_H
#define DISPERSION_DISPERSION_RESULTS_H

#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <optional>
#include <algorithm>
#include <utility>
#include <tuple>
#include <cmath>
#include <string>

using ResultType = std::optional<std::tuple<ValueType, ValueType, ValueType, size_t>>;

class Results
{
public:
    Results()
        : results_(std::nullopt)
    {}
    explicit Results(ResultType results)
        : results_(std::move(results))
    {}

    Results(const Results &other)
        : results_(other.results_)
    {} // rule of 5
    Results(Results &&other) noexcept
        : results_(std::move(other.results_))
    {}
    Results &operator=(const Results &other)
    {
        if (this != &other) {
            results_ = other.results_;
        }
        return *this;
    }
    Results &operator=(Results &&other) noexcept
    {
        std::swap(results_, other.results_);
        return *this;
    }

    ValueType mean() const
    { return std::get<0>(*results_); }
    ValueType dispersion() const
    { return std::get<1>(*results_); }
    ValueType sum() const
    { return std::get<2>(*results_); }
    size_t count() const
    { return std::get<3>(*results_); }
    bool has_values() const
    { return results_.has_value(); }

    friend bool operator==(const Results &lhs, const Results &rhs)
    {
        constexpr ValueType eps{1.0e-6};
        return lhs.count() == rhs.count() &&
            std::fabs(lhs.mean() - rhs.mean()) < eps &&
            std::fabs(lhs.sum() - rhs.sum()) < eps &&
            std::fabs(lhs.dispersion() - rhs.dispersion()) < eps;
    }
    friend bool operator!=(const Results &lhs, const Results &rhs)
    {
        return !(lhs == rhs);
    }
private:
    ResultType results_;
};

static void printResults(const Results &results)
{
    if (results.has_values()) {
        std::cout << "Count: " << results.count() << " Sum: " << results.sum()
                  << " Mean: " << results.mean() << " Dispersion: " << results.dispersion() << '\n';
    }
    else {
        std::cout << "No data\n";
    }
}

class DistributedResults
{
public:
    explicit DistributedResults(size_t shards)
        : shards_(shards)
    {
        results_.resize(shards_);
    }
    void addShardResults(size_t shard, Results &&results)
    {
        if (--shard < shards_) {
            results_[shard] = results;
        }
        else throw std::runtime_error("Wrong shard number " + std::to_string(shard));
    }
    bool isShardReady(size_t shard) const
    {
        return results_.at(shard - 1).has_values();
    }
    bool areReady() const
    {
        return std::all_of(std::begin(results_), std::end(results_),
                           [](const auto &result)
                           { return result.has_values(); });
    }
    Results calculateWith(Results calculate_func(const std::vector<Results> &data)) const
    {
        return calculate_func(results_);
    }
private:
    std::vector<Results> results_;
    size_t shards_;
};

static DataContainer readFile(const std::string &filename)
{
    using IteratorType = std::istream_iterator<ValueType>;
    DataContainer result;
    std::ifstream stream(filename);
    std::copy(IteratorType(stream), IteratorType(), std::back_inserter(result));
    return result;
}

#endif //DISPERSION_DISPERSION_RESULTS_H
