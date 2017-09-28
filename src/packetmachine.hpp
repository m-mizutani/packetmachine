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

#include <pthread.h>
#include <memory>
#include <string>
#include <functional>

#include "./packetmachine/common.hpp"
#include "./packetmachine/exception.hpp"
#include "./packetmachine/property.hpp"
#include "./packetmachine/object.hpp"

namespace pm {

class Capture;
class Input;
class Kernel;
class Handler;

class HandlerPtr {
 private:
  std::weak_ptr<Handler> ptr_;
 public:
  HandlerPtr(std::shared_ptr<Handler> ptr);
  ~HandlerPtr();
};

class Machine {
 private:
  Capture *cap_;
  Input *input_;
  Kernel *kernel_;
  pthread_t input_th_, kernel_th_;

 public:
  Machine();
  ~Machine();

  // Configure data source.
  void add_pcapdev(const std::string& dev_name);
  void add_pcapfile(const std::string& file_path);

  // Run capture & packet decodeing and wait
  void loop();

  //
  void start();
  bool join(struct timespec* timeout = nullptr);
  void halt();

  hdlr_id on(const std::string& event_name,
             std::function<void(const Property&)>&& callback);
  bool clear(hdlr_id hid);

  uint64_t recv_pkt() const;
  uint64_t recv_size() const;

  param_id lookup_param_id(const std::string& name) const;
  const std::string& lookup_param_name(param_id pid) const;
  event_id lookup_event_id(const std::string& name) const;
  const std::string& lookup_event_name(event_id eid) const;
};

}   // namespace pm

#endif   // __PACKETMACHINE_HPP__
