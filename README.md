PacketMachine
=====================


```cpp
#include <iostream>
#include <packetmachine.hpp>

class Printer : public pm::Process {
  void recv(const netdec::Property &p) {
    if (p.has("dns")) {
      std::cout << p["dns"] << std::endl;
    }
  };
};

int main(int argc, char *argv[]) {
  pm::Machine m("eth0");
	pm::ProcPtr ptr(new Printer());
	m.bind("dns.request", ptr);

	m.start();
	m.stop();
}
```