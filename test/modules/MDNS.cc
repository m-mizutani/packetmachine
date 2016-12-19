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

#include "./fixtures.hpp"

TEST_F(ModuleTesterData2, MDNS_packet) {
  const pm::Property* p;
  p = get_property(27);   // # packet #28

  EXPECT_TRUE(p->has_value("MDNS.tx_id"));
  EXPECT_TRUE(p->has_value("MDNS.question"));
  EXPECT_FALSE(p->has_value("MDNS.answer"));
  EXPECT_TRUE(p->has_value("MDNS.authority"));
  EXPECT_FALSE(p->has_value("MDNS.additional"));

  EXPECT_EQ(0, p->value("MDNS.tx_id").uint());

  const pm::value::Array& q_arr =
      dynamic_cast<const pm::value::Array&>(p->value("MDNS.question"));
  EXPECT_EQ(3, q_arr.size());

  const pm::value::Map& rec1 =
      dynamic_cast<const pm::value::Map&>(q_arr.get(1));
  EXPECT_EQ(255,                  rec1.find("type").uint());
  EXPECT_EQ("Interceptor.local.", rec1.find("name").repr());

  const pm::value::Array& a_arr =
      dynamic_cast<const pm::value::Array&>(p->value("MDNS.authority"));
  EXPECT_EQ(3, a_arr.size());

  const pm::value::Map& rec2 =
      dynamic_cast<const pm::value::Map&>(a_arr.get(2));
  EXPECT_EQ("A",                  rec2.find("type").repr());
  EXPECT_EQ("Interceptor.local.", rec2.find("name").repr());
  EXPECT_EQ("10.0.0.130",         rec2.find("data").repr());
}

