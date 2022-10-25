//
// Created by eugene on 25.10.22.
//

#include <iostream>
#include <mutex>
#include <condition_variable>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "dispersion.grpc.pb.h"

#include "dispersion_types.h"
#include "dispersion_results.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using dispersion::Dispersion;
using dispersion::DispersionReply;
using dispersion::DispersionRequest;

// Logic and data behind the server's behavior.
class DispersionServiceImpl final: public Dispersion::Service
{

    Status GetDispersionCalculations(ServerContext *context, const DispersionRequest *request,
                                     DispersionReply *reply) override
    {
        if (context->IsCancelled()) {
            return Status(grpc::StatusCode::CANCELLED, "deadline exceeded");
        }
        const auto shard = request->shard();
        std::cout << "Got request from shard " << shard << '\n';
        if (shard > 0 && shard <= NumShards) {
            if (data_.isShardReady(shard)) {
                std::cout << "Duplicate results from shard " << shard << ", ignore it.\n";
                reply->set_result(static_cast<size_t>(ReturnCode::DUPLICATE));
                return Status(grpc::StatusCode::ALREADY_EXISTS, "duplicate results");
            }
            reply->set_result(static_cast<size_t>(ReturnCode::OK));
            data_.addShardResults(shard, Results{std::optional{std::tuple{
                request->mean(), request->dispersion(), request->sum(), request->count()}}});
            results_ready_cv_.notify_one();
            return Status::OK;
        }
        else {
            std::cout << "Wrong shard: " << shard << '\n';
            reply->set_result(static_cast<size_t>(ReturnCode::WRONG_SHARD));
            return Status(grpc::StatusCode::INVALID_ARGUMENT, "wrong shard");
        }
    }
public:
    void waitForAllShards() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        results_ready_cv_.wait(lock,
                               [this]
                               { return data_.areReady(); });
    }

    Results calculateWith(Results calculate_func(const std::vector<Results> &data)) const
    {
        return data_.calculateWith(calculate_func);
    }

private:
    mutable std::mutex mutex_;
    mutable std::condition_variable results_ready_cv_;
    DistributedResults data_{NumShards};
};
