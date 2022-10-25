//
// Created by eugene on 23.10.22.
//

#ifndef DISPERSION_DISPERSION_CALCULATION_H
#define DISPERSION_DISPERSION_CALCULATION_H

#include <vector>

#include "dispersion_types.h"
#include "dispersion_results.h"

Results calculateMeanAndDispersionAsWhole(const DataContainer &data);
Results calculateMeanAndDispersionAsWhole(const std::vector<DataContainer> &data);
Results calculateMeanAndDispersionFromParts(const std::vector<Results> &data);

#endif //DISPERSION_DISPERSION_CALCULATION_H
