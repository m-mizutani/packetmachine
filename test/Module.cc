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

#include "./gtest/gtest.h"
#include "../src/module.hpp"

namespace module_test {

class TestMod : public pm::Module {
 public:
  void setup() {
  }
  pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
    return pm::Module::NONE;
  }
};

TEST(Module, basic) {
  pm::ModuleBuilder builder;
  pm::ModuleFactoryEntry<TestMod> tf("TestMod", &builder);

  std::map<std::string, pm::Module*> mod_map;
  builder.build(&mod_map);

  EXPECT_EQ(1, mod_map.size());
}

TEST(Module, use_global_variable) {
  std::map<std::string, pm::Module*> mod_map;
  build_module_map(&mod_map);

  EXPECT_EQ(7, mod_map.size());
  EXPECT_NE(mod_map.end(), mod_map.find("Ethernet"));
  EXPECT_NE(mod_map.end(), mod_map.find("ARP"));
  EXPECT_NE(mod_map.end(), mod_map.find("IPv4"));
  EXPECT_NE(mod_map.end(), mod_map.find("UDP"));
  EXPECT_NE(mod_map.end(), mod_map.find("ICMP"));
  EXPECT_NE(mod_map.end(), mod_map.find("TCP"));
  EXPECT_NE(mod_map.end(), mod_map.find("TCPSession"));
}

}   // namespace module_test
