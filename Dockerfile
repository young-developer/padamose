FROM ubuntu:20.04

WORKDIR /app

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
  && apt-get install -y build-essential \
      gcc \
      g++ \
      gdb \
      clang \
      make \
      ninja-build \
      cmake \
      autoconf \
      automake \
      locales-all \
      dos2unix \
      rsync \
      tar \
      python \
      python-dev \
  && apt-get clean \

COPY . /app

RUN apt-get install -y libgflags-dev
RUN apt-get install -y libsnappy-dev
RUN apt-get install -y zlib1g-dev
RUN apt-get install -y libbz2-dev
RUN apt-get install -y liblz4-dev
RUN apt-get install -y libzstd-dev
RUN apt-get install -y librocksdb-dev
RUN apt-get install -y libboost-all-dev

EXPOSE 22
CMD ["/bin/bash"]
