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
#include <functional>

#include "./packetmachine/common.hpp"
#include "./packetmachine/exception.hpp"
#include "./packetmachine/property.hpp"
#include "./packetmachine/value.hpp"
#include "./packetmachine/config.hpp"

namespace pm {

class Capture;
class Input;
class Kernel;
class HandlerEntity;


class Handler {
 private:
  std::weak_ptr<HandlerEntity> ptr_;
  std::weak_ptr<Kernel> kernel_;
 public:
  Handler(std::shared_ptr<HandlerEntity> ptr,
          std::shared_ptr<Kernel> kernel);
  ~Handler();
  bool is_active() const;
  bool activate();
  bool deactivate();
  bool destroy();
};


class Machine {
 private:
  Capture *cap_;
  Input *input_;
  std::shared_ptr<Kernel> kernel_;

 public:
  Machine();
  explicit Machine(const Config& config);
  ~Machine();

  // Configure data source.
  void add_pcapdev(const std::string& dev_name);
  void add_pcapfile(const std::string& file_path);
  const std::string& data_source_name() const;

  // Run capture & packet decodeing and wait
  void loop();

  // Run as background thread
  void start();
  bool join(struct timespec* timeout = nullptr);
  void halt();

  Handler on(const std::string& event_name,
             std::function<void(const Property&)>&& callback);

  uint64_t recv_pkt() const;
  uint64_t recv_size() const;

  const ParamKey& lookup_param_key(const std::string& name) const;
  const std::string& lookup_param_name(const ParamKey& key) const;

  event_id lookup_event_id(const std::string& name) const;
  const std::string& lookup_event_name(event_id eid) const;
};

}   // namespace pm

#endif   // __PACKETMACHINE_HPP__
