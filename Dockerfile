FROM ubuntu:16.04

WORKDIR /root

RUN apt-get update && apt-get -y install vim git ninja-build pkg-config clang clang-4.0 qemu

RUN git clone https://github.com/radare/radare2.git && cd radare2 && sys/install.sh

RUN git clone https://github.com/radare/radare2-r2pipe.git && cd radare2-r2pipe/c/ && make
