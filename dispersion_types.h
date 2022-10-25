//
// Created by eugene on 23.10.22.
//

#ifndef DISPERSION_DISPERSION_TYPES_H
#define DISPERSION_DISPERSION_TYPES_H

#include <vector>

using ValueType = float;
using DataContainer = std::vector<ValueType>;

constexpr size_t NumShards = 4;

enum class ReturnCode
{
    OK = 0,
    DUPLICATE = 1,
    WRONG_SHARD = 2
};

#endif //DISPERSION_DISPERSION_TYPES_H
