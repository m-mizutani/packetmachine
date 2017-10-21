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

#include "./packetmachine/config.hpp"
#include "./packetmachine/exception.hpp"
#include <type_traits>

namespace pm {

bool ConfigValue::as_bool() const {
  throw Exception::TypeError("bool is not supported");
}
int ConfigValue::as_int() const {
  throw Exception::TypeError("int is not supported");
}
const std::string& ConfigValue::as_str() const {
  throw Exception::TypeError("string is not supported");
}

namespace config {

class Int : public ConfigValue {
 private:
  int val_;
 public:
  explicit Int(int val) : val_(val) {}
  int as_int() const { return this->val_; }
};

class Str : public ConfigValue {
 private:
  std::string val_;
 public:
  explicit Str(const std::string& val) : val_(val) {}
  const std::string& as_str() const { return this->val_; }
};

class Bool : public ConfigValue {
 private:
  bool val_;
 public:
  explicit Bool(bool val) : val_(val) {}
  bool as_bool() const { return this->val_; }
};

}   // namespace config


Config::ConfMap::iterator Config::fetch(const std::string& key,
                                        Config::ConfMap* kv_map) {
  auto it = kv_map->find(key);
  if (it == kv_map->end()) {
    kv_map->insert(std::make_pair(key, nullptr));
    return kv_map->find(key);
  } else {
    delete it->second;
    return it;
  }  
}


Config::Config() {
};

Config::~Config() {
  for (auto& it : this->kv_map_) {
    delete it.second;
  }
}

Config& Config::set(const std::string& key, int val) {
  auto it = fetch(key, &this->kv_map_);
  auto v = new config::Int(val);
  it->second = v;
  return *this;
}

Config& Config::set(const std::string& key, const std::string& val) {
  auto it = fetch(key, &this->kv_map_);
  auto v = new config::Str(val);
  it->second = v;
  return *this;
}

Config& Config::set_true(const std::string& key) {
  auto it = fetch(key, &this->kv_map_);
  auto v = new config::Bool(true);
  it->second = v;
  return *this;
}

Config& Config::set_false(const std::string& key) {
  auto it = fetch(key, &this->kv_map_);
  auto v = new config::Bool(false);
  it->second = v;
  return *this;
}


bool Config::has(const std::string& key) const {
  return (this->kv_map_.find(key) != this->kv_map_.end());
}

const ConfigValue& Config::get(const std::string& key) const {
  auto it = this->kv_map_.find(key);
  if (it == this->kv_map_.end()) {
    throw Exception::KeyError(key + " does not exist");
  }

  return *(it->second);  
}

}   // namespace pm
