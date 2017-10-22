/*
 * Copyright (c) 2017 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
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

#ifndef __PACKETMACHINE_CONFIG_HPP__
#define __PACKETMACHINE_CONFIG_HPP__

#include "./exception.hpp"
#include <string>
#include <map>
#include <memory>

namespace pm {


class ConfigValue {
 private:
  std::string key_;
 public:
  explicit ConfigValue(const std::string& key) : key_(key) {}
  const std::string& key() const { return this->key_; }
  virtual ~ConfigValue () {}
  virtual bool as_bool() const;
  virtual int as_int() const;
  virtual const std::string& as_str() const;
  void rename(const std::string& new_name);
};

typedef std::shared_ptr<ConfigValue> ConfigPtr;


class Config {
 private:
  typedef std::map<std::string, ConfigPtr> ConfMap;
  ConfMap kv_map_;

 public:
  Config();
  ~Config();
  Config& set(const std::string& key, int val);
  Config& set(const std::string& key, const std::string& val);
  Config& set_true(const std::string& key);
  Config& set_false(const std::string& key);
  Config& set(const std::string& key, ConfigPtr val);

  static ConfigPtr make_value(const std::string& key, int val);
  static ConfigPtr make_value(const std::string& key, bool val);
  static ConfigPtr make_value(const std::string& key, const std::string& val);
  
  const ConfMap& map() const { return this->kv_map_; }
  bool has(const std::string& key) const;
  const ConfigValue& get(const std::string& key) const;
  ConfigPtr ptr(const std::string& key) const;
};



}   // namespace pm

#endif   // __PACKETMACHINE_OBJECT_HPP__
