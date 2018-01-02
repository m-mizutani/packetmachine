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
#include "./module.hpp"
#include "./decoder.hpp"
#include "./packetmachine/config.hpp"

namespace pm {

void build_module_map(std::map<std::string, Module*> *mod_map) {
#define INSTALL_MOD(CNAME)                                      \
  {                                                             \
    extern Module* __new_module_##CNAME##_factory();            \
    mod_map->insert(std::make_pair(#CNAME, NEW_MODULE(CNAME))); \
  }

  INSTALL_MOD(Ethernet);
  INSTALL_MOD(Dot1Q);
  INSTALL_MOD(PPPoE);
  INSTALL_MOD(ARP);
  INSTALL_MOD(IPv4);
  INSTALL_MOD(IPv6);
  INSTALL_MOD(ICMP);
  INSTALL_MOD(UDP);
  INSTALL_MOD(TCP);
  INSTALL_MOD(DNS);
  INSTALL_MOD(MDNS);
  INSTALL_MOD(DHCP);
}



ParamDef::ParamDef(const std::string& local_name, Value*(*constructor)()) :
    local_name_(local_name), key_(this), constructor_(constructor) {    
}
ParamDef::~ParamDef() {
}


void ParamDef::finalize(mod_id module_id, param_id pid,
                        const std::string &prefix) {
  this->module_id_ = module_id;
  this->name_ = prefix + "." + this->local_name_;

  this->id_ = pid;
  this->key_.set_key(pid);
}



// -------------------------------------
// ValueStorage
//

ValueStorage::ValueStorage() {
  
}
ValueStorage::~ValueStorage() {
  for(auto ptr : this->storage_) {
    delete ptr;
  }
}

void ValueStorage::resize(size_t s, Value*(*constructor)()) {
  size_t current = this->storage_.size();
  if (s > current) {
    this->storage_.resize(s);
    for (size_t idx = current; idx < this->storage_.size(); idx++) {
      this->storage_[idx] = constructor();
    }
  }
}

const Value& ValueStorage::get(size_t idx) const {
  assert(this->storage_.size() > idx);
  return *(this->storage_[idx]);
}


// -------------------------------------
// MajorParamDef
//

MajorParamDef::MajorParamDef(const std::string& local_name, Value*(*constructor)()) :
    ParamDef(local_name, new_storage), minor_constructor_(constructor) {
}
MajorParamDef::~MajorParamDef() {
  for (auto& m : this->def_map_) {
    delete m.second;
  }
}

void MajorParamDef::define_minor(const std::string& minor_name, Defer&& defer) {
  auto def = new MinorParamDef(this, std::move(defer), minor_name,
                               this->minor_constructor_);  
  assert(this->def_map_.find(minor_name) == this->def_map_.end());
  this->def_map_.insert(std::make_pair(minor_name, def));
}

void MajorParamDef::finalize(mod_id mid, param_id pid, const std::string& prefix) {
  this->ParamDef::finalize(mid, pid, prefix);
  
  param_id minor_id = 0;
  for (auto& m : this->def_map_) {
    m.second->finalize(mid, pid, this->name());
    m.second->set_minor_id(minor_id);
    minor_id++;
  }
}

Value* MajorParamDef::new_storage() {
  return new ValueStorage();
}

void MajorParamDef::copy_sub_def(std::map<std::string, ParamDef*> *def_map) {
  for (auto& d : this->def_map_) {
    def_map->insert(std::make_pair(d.second->name(), d.second));
  }
}

// -------------------------------------
// MinorParamDef
//

MinorParamDef::MinorParamDef(MajorParamDef* parent, Defer&& defer,
                             const std::string& local_name, Value*(*constructor)()) :
    ParamDef(local_name, constructor),
    minor_id_(Param::NONE), defer_(defer), parent_(parent) {
}
MinorParamDef::~MinorParamDef() {
}

void MinorParamDef::set_minor_id(param_id pid) {
  this->minor_id_ = pid;
}


// -------------------------------------
// ConfigDef
//

void ConfigDef::set_default(ConfigPtr default_value) {
  this->default_value_ = default_value;
}

void ConfigDef::finalize(mod_id module_id, const std::string& prefix) {
  this->module_id_ = module_id;
  this->name_ = prefix + "." + this->local_name_;
}


// -------------------------------------
// Module
//

Module::Module() : dec_(nullptr), id_(Module::NONE) {
}

Module::~Module() {
  for (auto it : this->config_map_) {
    delete it.second;
  }
}

Value* Module::new_value() {
  return new Value();
}

Value* Module::new_array() {
  return new value::Array();
}

Value* Module::new_map() {
  return new value::Map();
}


ParamDef* Module::define_param(const std::string& name,
                               Value*(*new_object)()) {
  ParamDef *def = new ParamDef(name, new_object);
  this->param_map_.insert(std::make_pair(name, def));
  return def;
}

MajorParamDef* Module::define_major_param(const std::string& name,
                                          Value*(*new_object)()) {
  MajorParamDef *def = new MajorParamDef(name, new_object);
  assert(this->param_map_.find(name) == this->param_map_.end());
  this->param_map_.insert(std::make_pair(name, def));
  return def;
}


const EventDef* Module::define_event(const std::string& name) {
  EventDef *def = new EventDef(name);
  assert(this->event_map_.find(name) == this->event_map_.end());
  this->event_map_.insert(std::make_pair(name, def));
  return def;
}


void Module::define_config(const std::string& name) {
  ConfigDef *def = new ConfigDef(name);
  assert(this->config_map_.find(name) == this->config_map_.end());
  this->config_map_.insert(std::make_pair(name, def));  
}

void Module::define_config(const std::string& name, int dflt_val) {
  ConfigDef *def = new ConfigDef(name);
  def->set_default(Config::make_value(name, dflt_val));
  assert(this->config_map_.find(name) == this->config_map_.end());
  this->config_map_.insert(std::make_pair(name, def));  
}

void Module::define_config(const std::string& name, bool dflt_val) {
  ConfigDef *def = new ConfigDef(name);
  def->set_default(Config::make_value(name, dflt_val));
  assert(this->config_map_.find(name) == this->config_map_.end());
  this->config_map_.insert(std::make_pair(name, def));  
}

void Module::define_config(const std::string& name,
                           const std::string& dflt_val) {
  ConfigDef *def = new ConfigDef(name);
  def->set_default(Config::make_value(name, dflt_val));
  assert(this->config_map_.find(name) == this->config_map_.end());
  this->config_map_.insert(std::make_pair(name, def));  
}

mod_id Module::lookup_module(const std::string& name) {
  // lookup_module must not be called before set decoder.
  assert(this->dec_);
  return this->dec_->lookup_module(name);
}

param_id Module::lookup_param_id(const std::string& name) {
  assert(this->dec_);
  return this->dec_->lookup_param_id(name);
}

void Module::set_decoder(Decoder* dec) {
  this->dec_ = dec;
}

void Module::set_mod_id(mod_id id) {
  this->id_ = id;
}

void Module::set_name(const std::string& name) {
  this->name_ = name;
}



}   // namespace pm
