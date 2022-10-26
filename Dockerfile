# Copyright 2018 gRPC authors.
# Copyright 2018 Claudiu Nedelcu.
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
# limitations under the License.#
#
# Based on https://hub.docker.com/r/grpc/cxx.

FROM debian:latest as build

RUN apt-get update && apt-get install -y \
  autoconf \
  automake \
  build-essential \
  curl \
  git \
  libtool \
  wget \
  make \
  pkg-config \
  unzip \
  && apt-get clean

ENV MY_INSTALL_DIR /var/local/
ENV CALCULATOR_BUILD_PATH /usr/local/dispersion

RUN git clone --recurse-submodules -b v1.50.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc /var/local/git/grpc && \
    cd /var/local/git/grpc && \
    git submodule update --init --recursive

RUN wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6-Linux-x86_64.sh && \
    sh cmake-linux.sh -- --skip-license --prefix=$MY_INSTALL_DIR && \
    rm cmake-linux.sh

RUN export PATH="$MY_INSTALL_DIR/bin:$PATH" && \
    cmake --version

RUN echo "-- installing grpc" && \
    cd /var/local/git/grpc && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    export PATH="$MY_INSTALL_DIR/bin:$PATH" && \
    cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../.. && \
    make -j$(nproc) && make install && cd -

COPY . $CALCULATOR_BUILD_PATH/src/dispersion/

RUN echo "-- building dispersion calculator" && \
    export PATH="$MY_INSTALL_DIR/bin:$PATH" && \
    mkdir -p $CALCULATOR_BUILD_PATH/out/dispersion && \
    cd $CALCULATOR_BUILD_PATH/out/dispersion && \
    cmake -DCMAKE_BUILD_TYPE=Release $CALCULATOR_BUILD_PATH/src/dispersion && g++ -v && \
    make && \
    mkdir -p bin && \
    ldd dispersion_local | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' bin/ && \
    mv dispersion* bin/ && \
    echo "LD_LIBRARY_PATH=/opt/dispersion/:\$LD_LIBRARY_PATH ./dispersion_local" > bin/start.sh && \
    chmod +x bin/start.sh 

WORKDIR $CALCULATOR_BUILD_PATH
ENTRYPOINT ["/bin/bash"]
CMD ["-s"]

FROM debian:latest as runtime
COPY --from=build /usr/local/dispersion/out/dispersion/bin/dispersion* /usr/local/dispersion/src/dispersion/run.sh /opt/dispersion/
EXPOSE 8080
WORKDIR /opt/dispersion/
ENTRYPOINT ["/bin/bash", "start.sh"]
