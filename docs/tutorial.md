Tutorial of PacketMachine Programming
===========================

Getting Started
------------

Welcome to PacketMachine. If you did not install PacketMachine library, please see a [installation guide](install.md) before writing code.

The PacketMachine allows you to write packet capture & analysis programs in C++. A simple example packet capture program is following.

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

  m.add_pcapdev(argv[1]);
  m.loop();

  return 0;
}
```

This program captures packets from a network interface that is given as 1st argument, and output IPv4 source address, TCP source port, destination addresss and TCP destination port of each TCP packet.




Basic steps of PacketMachine
-------------


### 1) Create PacketMachine instance and register input source

### 2) Add callback

A callback receives const [`Property`](api.md#property) reference.

### 3) Start capture
