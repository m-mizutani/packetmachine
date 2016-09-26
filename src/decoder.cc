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
#include "./debug.hpp"


namespace pm {

Decoder::Decoder() : mod_ethernet_(Module::NONE) {
  std::map<std::string, Module*> mod_map;
  build_module_map(&mod_map);

  // Building module map.
  for (auto& m : mod_map) {
    Module* mod = m.second;
    const mod_id id = this->modules_.size();
    mod->set_decoder(this);
    mod->set_mod_id(id);
    mod->set_name(m.first);

    this->modules_.push_back(mod);
    this->mod_map_.insert(std::make_pair(m.first, id));

    if (m.first == "Ethernet") {
      this->mod_ethernet_ = id;
    }
  }

  // Building parameter map.
  for (auto& m : this->modules_) {
    for (auto& p : *(m->param_map())) {
      const param_id global_id = this->params_.size();
      ParamDef *def = p.second;
      def->set_module_id(m->id());
      def->set_id(global_id);
      def->set_name(m->name() + "." + p.first);
      this->params_.push_back(def);
      this->param_map_.insert(std::make_pair(def->name(), def));
    }
  }

  // Setup modules.
  for (auto &m : this->modules_) {
    m->setup();
  }
  assert(this->mod_ethernet_ != Module::NONE);
}


Decoder::~Decoder() {
  for (auto& m : this->modules_) {
    delete m;
  }

  for (auto& pi : this->params_) {
    delete pi;
  }
}


void Decoder::decode(Payload* pd, Property* prop) {
  Module* mod;
  mod_id next = this->mod_ethernet_;

  // debug(true, "decoding");

  while (next != Module::NONE) {
    // debug(true, "next = %lld", next);
    mod = this->modules_[next];

    next = mod->decode(pd, prop);

    assert(next == Module::NONE || 
           (0 <= next && next < static_cast<mod_id>(this->modules_.size())));
  }
}

mod_id Decoder::lookup_module(const std::string& name) const {
  auto it = this->mod_map_.find(name);
  if (it == this->mod_map_.end()) {
    return Module::NONE;
  } else {
    return it->second;
  }
}

param_id Decoder::lookup_param_id(const std::string& name) const {
  const auto& it = this->param_map_.find(name);
  if (it == this->param_map_.end()) {
    return Param::NONE;
  } else {
    return it->second->id();
  }
}

const std::string& Decoder::lookup_param_name(param_id pid) const {
  if (pid < 0 || static_cast<param_id>(this->params_.size()) <= pid) {
    throw Exception::IndexError("No such parameter");
  } else {
    return this->params_[pid]->name();
  }
}

}   // namespace pm
