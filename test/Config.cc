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

#include <unistd.h>
#include "./gtest/gtest.h"
#include "../src/packetmachine/config.hpp"

TEST(Config, set_and_get) {
  pm::Config conf;
  conf.set("test.int", 12);
  conf.set_true("test.bool");
  conf.set("test.str", "hoge");
  
  EXPECT_TRUE(conf.has("test.int"));
  EXPECT_TRUE(conf.has("test.str"));
  EXPECT_TRUE(conf.has("test.bool"));
  EXPECT_FALSE(conf.has("test.false"));

  EXPECT_EQ(12, conf.get("test.int").as_int());
  EXPECT_EQ(true, conf.get("test.bool").as_bool());
  EXPECT_EQ("hoge", conf.get("test.str").as_str());
}

TEST(Config, overwrite) {
  pm::Config conf;
  conf.set("test.int", 12);
  conf.set("test.int", 24);
  EXPECT_EQ(24, conf.get("test.int").as_int());

  conf.set("test.str", "aa");
  conf.set("test.str", "bb");
  EXPECT_EQ("bb", conf.get("test.str").as_str());
  
  conf.set_true("test.bool");
  conf.set_false("test.bool");
  EXPECT_EQ(false, conf.get("test.bool").as_bool());
  
}

