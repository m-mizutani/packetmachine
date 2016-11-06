PacketMachine
=====================

A high-performance and simplified network traffic decoding library in C++.


```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char* argv[]) {
  pm::Machine m;

  m.on("TCP", [&](const pm::Property& p) {
      std::cout << "TCP: " <<
          p["IPv4.src"] << ":" << p["TCP.src_port"] << " > " <<
          p["IPv4.dst"] << ":" << p["TCP.dst_port"] << std::endl;
    });

  m.add_pcapdev("eth0");
  m.loop();
}
```

Features
------------

* Event callback model for network traffic decoding
* C++11 friendly
