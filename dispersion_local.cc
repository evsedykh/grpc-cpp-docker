//
// Created by eugene on 21.10.22.
//

#include <iostream>
#include <string>
#include <vector>

#include "dispersion_types.h"
#include "dispersion_results.h"
#include "dispersion_calculation.h"

int main()
{
    std::cout << "Calculate mean and dispersion for all data and parts of data:\n";
    constexpr int numParts = 4;
    const std::string file_name{"dispersion?.txt"};
    std::vector<DataContainer> allData;
    std::vector<Results> partialData;
    for (int i = 1; i <= numParts; ++i) {
        std::string current_file_name{file_name};
        current_file_name.replace(file_name.find_first_of('?'), 1, std::to_string(i));
        std::cout << "Read file " << current_file_name << '\n';
        DataContainer data = readFile(current_file_name);
        auto results = calculateMeanAndDispersionAsWhole(data);
        printResults(results);
        partialData.emplace_back(results);
        allData.emplace_back(std::move(data));
    }
    std::cout << "\nResults for all data:\n";
    const auto calculationResults = calculateMeanAndDispersionAsWhole(allData);
    printResults(calculationResults);

    std::cout << "\nResults for data from parts:\n";
    const auto calculationResultsFromParts = calculateMeanAndDispersionFromParts(partialData);
    printResults(calculationResultsFromParts);

    const bool is_equal = calculationResultsFromParts == calculationResults;
    std::cout << "\nResults from all data and from parts are" << (is_equal ? "" : " not") << " equal.\n";
}
