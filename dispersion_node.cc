//
// Created by eugene on 24.10.22.
//

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "dispersion_types.h"
#include "dispersion_results.h"
#include "dispersion_calculation.h"
#include "dispersion_service.h"
#include "dispersion_client.h"

Results calculateLocal()
{
    const std::string file_name{"dispersion?.txt"};
    std::vector<DataContainer> all_data;
    std::vector<Results> partial_data;
    for (int i = 1; i <= NumShards; ++i) {
        std::string current_file_name{file_name};
        current_file_name.replace(file_name.find_first_of('?'), 1, std::to_string(i));
        std::cout << "Read file " << current_file_name << '\n';
        DataContainer data = readFile(current_file_name);
        auto file_results = calculateMeanAndDispersionAsWhole(data);
        printResults(file_results);
        partial_data.emplace_back(file_results);
        all_data.emplace_back(std::move(data));
    }
    std::cout << "\nResults for all data:\n";
    auto results = calculateMeanAndDispersionAsWhole(all_data);
    printResults(results);

    std::cout << "\nResults for data from parts:\n";
    const auto results_from_parts = calculateMeanAndDispersionFromParts(partial_data);
    printResults(results_from_parts);

    const bool is_equal = results_from_parts == results;
    std::cout << "\nResults from all data and from parts are" << (is_equal ? "" : " not") << " equal.\n";
    return results;
}

void shutdownThread(const DispersionServiceImpl *service)
{
    service->waitForAllShards();
    std::cout << "All shards replied, calculate final results.\n";
    const auto distributed_results = service->calculateWith(calculateMeanAndDispersionFromParts);
    const Results local_results = calculateLocal();
    const bool is_equal = distributed_results == local_results;
    printResults(local_results);
    printResults(distributed_results);
    std::cout << "\nLocal and distributed results are" << (is_equal ? "" : " not") << " equal.\n";
    exit(0);
}

int RunServerRole(const std::string &server_address)
{
    std::cout << "Enter 'r' to run distributed dispersion calculations: ";
    while (std::getchar() != 'r') {}

    DispersionServiceImpl service;
    std::thread exit_thread(shutdownThread, &service);
    exit_thread.detach();

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "\n\nServer listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
    return 0;
}

int RunShardRole(const std::string &master_address)
{
    DispersionClient dispersion_client(grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials()));

    const int shard = [](const std::string &variable_name)
    {
        const char *variable_p = std::getenv(variable_name.c_str());
        if (variable_p != nullptr) {
            const std::string variable_value{variable_p};
            try {
                std::size_t pos{};
                int shard = std::stoi(variable_value, &pos);
                if (shard > 0 && shard <= NumShards) return shard;
                throw std::invalid_argument(variable_value);
            }
            catch (const std::invalid_argument &ex) {
                std::cerr << "Wrong shard number: " << ex.what() << '\n';
                std::exit(-1);
            }
        }
        std::cerr << "Have to set shard number.\n";
        exit(1);
    }("SHARD_NUM");

    return static_cast<int>(dispersion_client.CalculateDispersion(shard));
}

int main(int argc, char **argv)
{
    const std::string node_role = [](const std::string &variable_name)
    {
        const char *variable_p = std::getenv(variable_name.c_str());
        if (variable_p != nullptr) {
            std::string variable_value{variable_p};
            if (variable_value == "master" || variable_value == "shard") {
                return variable_value;
            }
        }
        std::cerr << "Have to set node role: (master|shard)\n";
        exit(1);
    }("NODE_ROLE");
    const std::string master_address{"0.0.0.0:50051"};
    if (node_role == "master") {
        return RunServerRole(master_address);
    }
    else if (node_role == "shard") {
        return RunShardRole(master_address);
    }
}
