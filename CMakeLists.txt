# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.5.1)

project(Dispersion C CXX)

set(CMAKE_CXX_STANDARD 17)

include(/var/local/git/grpc/examples/cpp/cmake/common.cmake)

# Proto file
get_filename_component(dispersion_proto "dispersion.proto" ABSOLUTE)
get_filename_component(dispersion_proto_path "${dispersion_proto}" PATH)

# Generated sources
set(dispersion_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/dispersion.pb.cc")
set(dispersion_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/dispersion.pb.h")
set(dispersion_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/dispersion.grpc.pb.cc")
set(dispersion_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/dispersion.grpc.pb.h")
add_custom_command(
      OUTPUT "${dispersion_proto_srcs}" "${dispersion_proto_hdrs}" "${dispersion_grpc_srcs}" "${dispersion_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${dispersion_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${dispersion_proto}"
      DEPENDS "${dispersion_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# dispersion_grpc_proto
add_library(dispersion_grpc_proto
  ${dispersion_grpc_srcs}
  ${dispersion_grpc_hdrs}
  ${dispersion_proto_srcs}
  ${dispersion_proto_hdrs})
target_link_libraries(dispersion_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets dispersion_](client|server|local)
foreach(_target
  dispersion_node dispersion_local)
  add_executable(${_target} "${_target}.cc"
                 "dispersion_types.h" "dispersion_results.h"
                 "dispersion_calculation.h" "dispersion_calculation.cc")
  target_link_libraries(${_target}
    dispersion_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

add_custom_command(TARGET dispersion_local POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/dispersion?.txt $<TARGET_FILE_DIR:dispersion_local>)
