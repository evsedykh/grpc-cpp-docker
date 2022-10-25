//
// Created by eugene on 25.10.22.
//

#include <memory>
#include <string>
#include <iostream>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "dispersion.grpc.pb.h"

#include "dispersion_types.h"
#include "dispersion_results.h"
#include "dispersion_calculation.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using dispersion::Dispersion;
using dispersion::DispersionReply;
using dispersion::DispersionRequest;

class DispersionClient
{
public:
    explicit DispersionClient(std::shared_ptr<Channel> channel)
        : stub_(Dispersion::NewStub(channel))
    {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    ReturnCode CalculateDispersion(size_t shard)
    {
        const std::string file_name{"dispersion?.txt"};
        std::string current_file_name{file_name};
        current_file_name.replace(file_name.find_first_of('?'), 1, std::to_string(shard));
        std::cout << "Read file " << current_file_name << '\n';
        DataContainer data = readFile(current_file_name);
        auto results = calculateMeanAndDispersionAsWhole(data);
        printResults(results);

        // Data we are sending to the server.
        DispersionRequest request;
        request.set_mean(results.mean());
        request.set_dispersion(results.dispersion());
        request.set_sum(results.sum());
        request.set_count(results.count());
        request.set_shard(shard);

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;
        context.set_wait_for_ready(true);

        // Container for the data we expect from the server.
        DispersionReply reply;
        // The actual RPC.
        Status status = stub_->GetDispersionCalculations(&context, request, &reply);

        // Act upon its status.
        std::cout << "Server replied " << reply.result();
        if (!status.ok()) {
            std::cout << ", error " << status.error_code() << ": ";
            if (status.error_code() != grpc::StatusCode::OK) {
                std::cout << status.error_message();
            }
        }
        std::cout << ".\n";
        return static_cast<ReturnCode>(reply.result());
    }

private:
    std::unique_ptr<Dispersion::Stub> stub_;
};
