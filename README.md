# Using Docker to build and deploy a C++ gRPC service
# gRPC Dispersion calculation

This sample demonstrates how to create a basic gRPC service in C++ and build it
it using GCC and CMake in a Docker container. It also shows how to create a
Docker image that contains only the service implementation, without the tools
needed to build it. It is based on the Dockerfile used to build the [official
gRPC CXX image](https://hub.docker.com/r/grpc/cxx/~/dockerfile/).

## Build and run the sample

```sh
git clone https://github.com/evsedykh/grpc-cpp-docker.git
cd grpc-cpp-docker
docker build -t calculator .
```

To run deployment script with one master and 4 shards you can use:

```sh
docker run --rm -it --entrypoint=bash calculator
```
and then inside container:
```
./run.sh
```
type 'r' to run calculations on master node.

