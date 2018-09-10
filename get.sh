#!/bin/bash

DIR=$(pwd)
echo $PWD

# get libfuzzer source code
cd $DIR && git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer libfuzzer/libfuzzer-source
cd $DIR/libfuzzer/libfuzzer-source && git reset --hard HEAD~1
cd $DIR/libfuzzer && patch -p0 < libfuzzer.diff

# get libfuzzer source code ( latest )
#cd $DIR && svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/fuzzer libfuzzer/libfuzzer-source

# get qemu source code
cd $DIR/qemu-2.12.0 && wget https://github.com/qemu/qemu/archive/v2.12.0.zip
cd $DIR/qemu-2.12.0 && unzip v2.12.0.zip && rm v2.12.0.zip
cd $DIR/qemu-2.12.0 && patch -p0 < qemu-2.12.0.diff
