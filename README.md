NetDec
=====================


```cpp
#include <iostream>
#include <netdec/netdec.hpp>

class Printer : public netdec::Reciever {
  void recv(const netdec::Property &p) {
    if (p.has("dns")) {
      std::cout << p["dns"] << std::endl;
    }
  };
};

int main(int argc, char *argv[]) {
  netdec::Observer obs("eth0");
	std::shared_ptr<Reciever> r_ptr(new Printer());
  netdec::obj_id h_id = obs.install("dns.request", r_ptr);

  obs.start();
  obs.wait(10.0); // wait 10 second
  obs.stop();

  // also delete Printer instance
  obs.remove(h_id); 
}
```