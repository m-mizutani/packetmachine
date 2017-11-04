PacketMachine
=====================

A high-performance and simplified network traffic decoding library in C++.


```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char* argv[]) {
  pm::Machine m;

  m.on("TCP", [](const pm::Property& p) {
      std::cout << "TCP: " <<
          p["IPv4.src"] << ":" << p["TCP.src_port"] << " > " <<
          p["IPv4.dst"] << ":" << p["TCP.dst_port"] << std::endl;
    });

  m.add_pcapdev("eth0");
  m.loop();
}
```

**PacketMachine is ...**

- High performance packet decoding library
- Able to capture network traffic via existing mechanism (e.g. libpcap)

**PacketMachine is not ...**

- Packet capturing mechanism for higher performance than netmap, DPDK, etc.
- Packet crafting library.

Features
------------

* Event callback model for network traffic decoding.
* Easy access to parameter of network traffic packet, such as `p["IPv4.src"]`
* Multi-threading (read/capture thread + decoding thread)

Performance
------------

![benchmark-task1](./docs/images/benchmark_task1.png)

Packet processing performance depends on kinds of task and data set. The chart shows a result of benchmark with simple packet counting task and 3 data set. In the task, PacketMachine marked over 2,000,000 pps (Packet Per Second). This is about three times faster than [libtins](https://github.com/mfontanini/libtins) that is a C++ library for packet decoding and crafting.

See a [benchmark](docs/benchmark.md) document for more detail.

How to use
------------

### Install

Note: Install libpcap before setup of PacketMachine.

```sh
$ git clone --recurse-submodules  https://github.com/m-mizutani/packetmachine.git
$ cd packetmachine
$ cmake . && make
$ sudo make install
```

See [install.md](docs/install.md) for more details.

### Compile

Prepare `example.cc` as below:

```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char* argv[]) {
  pm::Machine m;

  m.on("UDP", [](const pm::Property& p) {
      std::cout << "UDP: " <<
          p["IPv4.src"] << ":" << p["UDP.src_port"] << " > " <<
          p["IPv4.dst"] << ":" << p["UDP.dst_port"] << std::endl;
    });

  m.add_pcapdev(argv[1]);
  m.loop();

  return 0;
}
```

Compile `example.cc` and run it.

```sh
$ clang++ -std=c++11 example.cc -o example -lpacketmachine
$ sudo ./example eth0
UDP: 199.59.148.241:443 > 172.20.10.2:53227
UDP: 172.20.10.2:53227 > 199.59.148.241:443
...
```

Documents
------------

- [Installation guide](docs/install.md)
- [Tutorial](docs/tutorial.md)
- References
    - [Parameter List](docs/parameters.md)
    - [Event List](docs/events.md)
    - [Config List](docs/configs.md)

	
License
------------

- main part: [2-clause BSD license](LICENSE.md)
- `test/gtest*`: from **Google Test**, Google Inc. The BSD 3-Clause License
- `src/utils/hash.cc`: from **Chromium**, Google Inc. The BSD 3-Clause License

Author
------------

- Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
