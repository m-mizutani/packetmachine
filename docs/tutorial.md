Tutorial of PacketMachine Programming
===========================

Getting Started
------------

```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char* argv[]) {
  // Create an instance
  pm::Machine m;

  // Register callback function that shows TCP flow
  m.on("TCP", [](const pm::Property& p) {
      // This function is called in each packet including TCP
      std::cout << "TCP: " <<
	      // p["xxx"] gets parameter from the packet
          p["IPv4.src"] << ":" << p["TCP.src_port"] << " > " <<
          p["IPv4.dst"] << ":" << p["TCP.dst_port"] << std::endl;
    });

  // Add a network device specified by CLI argument as input source
  m.add_pcapdev(argv[1]);
  
  // Start packet capture loop
  m.loop();

  return 0;
}
```

```sh
$ clang++ -std=c++11 example.cc -o example -lpacketmachine
$ sudo ./example eth0
```

Welcome to PacketMachine. If you did not install PacketMachine library, please see a [installation guide](install.md) before writing code.
The PacketMachine allows you to write packet capture & analysis programs in C++. A simple example packet capture program is following.

Above program captures packets from a network interface that is given as 1st argument, and output IPv4 source address, TCP source port, destination addresss and TCP destination port of each TCP packet.

This part of the documentation, which is mostly prose, begins with some background information about PacketMachine, then focuses on step-by-step instructions for getting the most out of PacketMachine.


- [Quick start](tutorial.md#quick-start)
	- [Running in the background](tutorial.md#run-background)
	- [Use data as various format](tutorial.md#use-data-format)
	- [Access a structured parameter](tutorial.md#use-struct-parameter)
- [Advanced Usage](tutorial.md#advanced-usage)
    - [Faster parameter access](tutorial.md#faster-parameter-access)

[Quick Start](#quick-start)
----------------

### [Running in the background](#run-background)

```cpp
#include <iostream>
#include <packetmachine.hpp>
#include <unistd.h>

int main(int argc, char* argv[]) {
  pm::Machine m;
  m.on("IPv4", [](const pm::Property& p) {
      std::cout << p["IPv4.src"] << " -> " << p["IPv4.dst"] << std::endl;
  });

  // Add a network device specified by CLI argument as input source
  m.add_pcapdev(argv[1]);
  
  // Starting thread in the background
  m.start();
  
  // Waiting 10 seconds
  for (int i = 0; i < 10; i++) {
    std::cout << i << " sec" << std::endl;
    sleep(1);
  }
  
  // Shutdown
  m.halt();

  std::cout << "exit" << std::endl;
  return 0;
}
```

`pm::Machine::start()` starts threads in the background for capturing packets, decoding packets and call functions registered as callback. If you need to wait exiting input after `pm::Machine::start()`, `pm::Machine::join()` allowsthe main thread to join to the background threads.

Please note the callback functions should be called in the background thread and you need lock like `pthread_mutex_lock` to share variable(s) among threads in order to avoid race condition.

### [Use a parameter as various format](#use-data-format)

```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char* argv[]) {
  pm::Machine m;
  m.on("IPv4", [](const pm::Property& p) {
    // repr() shows value as recommended format
    std::cout << p.value("IPv4.src").repr() << std::endl; // "10.0.0.1"
	// hex() shows value as hex number sequence
    std::cout << p.value("IPv4.src").hex() << std::endl; // "0100000A"
	// uint64() returns integer
    std::cout << p.value("IPv4.src").uint64() << std::endl; // 167772161

  });

  m.add_pcapdev(argv[1]);
  m.loop()
  return 0;
}
```

### [Access a structured parameter](#use-struct-parameter)


[Advanced Usage](#advanced-usage)
-------------

### [Faster parameter access](tutorial.md#faster-parameter-access)

