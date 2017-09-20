Install PacketMachine
====================

Prerequisites
---------------

- Support OS: Linux and macOSX
    - Linux >= 4.4.0
    - macOSX >= 10.12.6
- C++ compiler (Recommend: clang++ >= 3.9)
- CMake >= 3.5.1
- libpcap >= 1.7.4

Build
------------------

```
$ git clone --recurse-submodules https://github.com/m-mizutani/packetmachine.git
$ cd packetmachine
$ cmake .
$ make
$ sudo make install
```

If you would like to debug

```
$ cmake cmake -DCMAKE_BUILD_TYPE=Debug .
```

Test
-------------------

After building the library

```
# with shared library (.so)
$ ./bin/packetmachine-shared-test
# with static library (.a)
$ ./bin/packetmachine-static-test
```
