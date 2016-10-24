PacketMachine
=====================


```cpp
#include <iostream>
#include <packetmachine.hpp>

int main(int argc, char *argv[]) {
  pm::Machine m;
	m.add_device("eth0");
	m.on("TCPSession.new", [&](const Property& p) {
	  std::cout << "TCP: " <<
		  p.value("IPv4.src") << ":" << p.value("TCP.src_port") << " -> " <<
		  p.value("IPv4.dst") << ":" << p.value("TCP.dst_port") << std::endl;
	});

	m.start();
	m.join();
	return 0;
}
```