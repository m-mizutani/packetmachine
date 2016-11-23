/*
 * Copyright (c) 2016 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/time.h>
#include <iostream>
#include "../src/packetmachine.hpp"


int main(int argc, char* argv[]) {
  pm::Machine m;

  if (argc != 2) {
    std::cout << "usage) pmfile <file_path>" << std::endl;
    return -1;
  }

  uint64_t tcp_pkt_count = 0;
  uint64_t tcp_data_size = 0;
  uint64_t tcp_ssn_count = 0;
  uint64_t udp_pkt_count = 0;
  uint64_t udp_data_size = 0;
  uint64_t total_pkt_count = 0;
  uint64_t total_data_size = 0;

  try {
    m.on("Ethernet", [&](const pm::Property& p) {
        total_pkt_count += 1;
        total_data_size += p.pkt_size();
      });

    m.on("TCPSession.new", [&](const pm::Property& p) {
        tcp_ssn_count += 1;
      });

    m.on("TCP", [&](const pm::Property& p) {
        tcp_pkt_count += 1;
        tcp_data_size += p.pkt_size();
      });

    m.on("UDP", [&](const pm::Property& p) {
        udp_pkt_count += 1;
        udp_data_size += p.pkt_size();
      });

    struct timeval tv_begin, tv_end, tv_sub;
    gettimeofday(&tv_begin, nullptr);
    m.add_pcapfile(argv[1]);
    m.loop();
    gettimeofday(&tv_end, nullptr);
    timersub(&tv_end, &tv_begin, &tv_sub);


    double period = static_cast<double>(tv_sub.tv_sec) +
                    static_cast<double>(tv_sub.tv_usec) /
                    (1000.0 * 1000.0);

    std::cout << "[total]" << std::endl
              << "\tpacket count: " << total_pkt_count << std::endl
              << "\tdata size:    " << total_data_size << std::endl;

    std::cout << "[tcp]" << std::endl
              << "\tpacket count:  " << tcp_pkt_count << std::endl
              << "\tdata size:     " << tcp_data_size << std::endl
              << "\tsession count: " << tcp_ssn_count << std::endl;

    std::cout << "[performance]" << std::endl
              << "\t" << period << " second" << std::endl
              << "\t" << total_pkt_count / period << " pps" << std::endl
              << "\t" << total_data_size * 8 / 1000000 / period
              << " Mbps" << std::endl;
  } catch (pm::Exception::Error &e) {
    std::cerr << "PacketMachine Error: " << e.what() << std::endl;
  }
}
