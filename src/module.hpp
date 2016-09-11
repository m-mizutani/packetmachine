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

#ifndef __PACKETMACHINE_MODULE_HPP__
#define __PACKETMACHINE_MODULE_HPP__

#include <map>
#include <vector>
#include <string>

#include "./packetmachine/object.hpp"
#include "./packetmachine/property.hpp"

namespace pm {

class Decoder;

class Module {
 private:
  std::map<std::string, param_id> param_map_;
  std::vector<Object*(*)()> param_new_;
  Decoder *dec_;

 protected:
  static Object* new_value();
  static Object* new_array();
  static Object* new_map();

  param_id define_param(const std::string& name,
                        Object*(*new_object)() = new_value);
  event_id define_event(const std::string& name);
  mod_id lookup_module(const std::string& name);

 public:
  static const mod_id NONE = -1;

  Module();
  virtual ~Module() = default;
  virtual void setup() = 0;
  virtual mod_id decode(Payload* pd, Property* prop) = 0;
  void set_decoder(Decoder* dec);
  Object* new_param(param_id id);
};


// Interface for factory template.
class ModuleFactory {
 public:
  ModuleFactory() = default;
  ~ModuleFactory() = default;
  virtual Module* create() = 0;
};

// Factory set that should be used as only one global variable.
class ModuleBuilder {
 private:
  std::map<std::string, ModuleFactory*> factory_;

 public:
  ModuleBuilder() = default;
  ~ModuleBuilder() = default;
  void add(const std::string& name, ModuleFactory *factory);
  void build(std::map<std::string, Module*> *mod_map);
};

// Global variable and function.
extern ModuleBuilder __global_module_builder;
void build_module_map(std::map<std::string, Module*> *mod_map);

// Actual class of module factory.
template<typename T>
class ModuleFactoryEntry : public ModuleFactory {
 public:
  Module* create() { return new T; }
  ModuleFactoryEntry(const std::string& name,
                     ModuleBuilder* mfs = &__global_module_builder) {
    mfs->add(name, this);
  }
};


#define INIT_MODULE(CNAME)                                      \
  ModuleFactoryEntry<CNAME> __module_##CNAME##_factory(#CNAME)

}   // namespace pm

#endif   // __PACKETMACHINE_MODULE_HPP__
