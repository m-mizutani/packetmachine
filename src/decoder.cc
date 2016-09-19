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

#include <assert.h>
#include "./decoder.hpp"
#include "./packetmachine/property.hpp"

namespace pm {

Decoder::Decoder() : mod_ethernet_(nullptr) {
  std::map<std::string, Module*> mod_map;
  build_module_map(&mod_map);

  for (auto& m : mod_map) {
    Module* mod = m.second;
    mod->set_decoder(this);

    const mod_id id = this->modules_.size();
    this->modules_.push_back(mod);
    this->mod_map_.insert(std::make_pair(m.first, id));

    if (m.first == "Ethernet") {
      this->mod_ethernet_ = mod;
    }
  }

  assert(this->mod_ethernet_);
}

Decoder::~Decoder() {
  for (auto& m : this->modules_) {
    delete m;
  }
}

void Decoder::decode(Payload* pd, Property* prop) {
  Module* mod = this->mod_ethernet_;
  mod_id next;
  while (mod) {
    next = mod->decode(pd, prop);

    if (next == Module::NONE) {
      mod = nullptr;
    } else {
      assert(0 <= next && next < this->modules_.size());
      mod = this->modules_[next];
    }
  }
}

Object* Decoder::new_param(mod_id mid, param_id pid) {
  assert(0 <= mid && mid < this->modules_.size());
  return this->modules_[mid]->new_param(pid);
}

mod_id Decoder::lookup_module(const std::string& name) const {
  auto it = this->mod_map_.find(name);
  if (it == this->mod_map_.end()) {
    return Module::NONE;
  } else {
    return it->second;
  }
}

}   // namespace pm
