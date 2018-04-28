# libfuzzer-binary

using libfuzzer to fuzz binary without source code

## dependency

[radare2](https://github.com/radare/radare2)

```
git clone https://github.com/radare/radare2.git
cd radare2
sys/install.sh
```

[radare2-r2pipe - c](https://github.com/radare/radare2-r2pipe/tree/master/c)

```
sudo apt-get install pkg-config
git clone https://github.com/radare/radare2-r2pipe.git
cd radare2-r2pipe/c
make
```

[cJSON](https://github.com/DaveGamble/cJSON)

we use cJSON as submodule

[ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages)

```
sudo apt-get install ninja-build
```

## build

```
git clone --recurse-submodules https://github.com/OAlienO/libfuzzer-binary.git
cd libfuzzer-binary
ninja
```

## get started

use `FUZZBIN` environment variable to pass the target binary

```
FUZZBIN=$(pwd)/testing/test ./fuzzer
```

## docker support

```
docker pull oalieno/libfuzzer-binary
docker run --rm -it --privileged oalieno/libfuzzer-binary /bin/bash
```
