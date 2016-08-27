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

#ifndef __PACKETMACHINE_HPP__
#define __PACKETMACHINE_HPP__

#include <memory>
#include <string>

#include "./packetmachine/exception.hpp"
#include "./packetmachine/property.hpp"

namespace pm {

class Capture;

class Process {
 public:
  Process();
  ~Process();

  virtual void recieve(const Property& prop) = 0;
};


typedef std::shared_ptr<Process> ProcPtr;
typedef int eid;

class Machine {
 private:
  Capture *cap_;
  static void reciever(Capture *cap, int i);

 public:
  Machine();
  ~Machine();

  // Configure data source.
  void add_device(const std::string& dev_name);
  void add_pcapfile(const std::string& file_path);

  // Contorl capture & packet decodeing.
  void start();
  void shutdown();

  bool bind(const std::string& event_name, ProcPtr ptr);
};

}   // namespace pm

#endif   // __PACKETMACHINE_HPP__
