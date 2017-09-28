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

#include "../src/debug.hpp"
#include "./fixtures.hpp"

TEST_F(ModuleTesterData1, NameService_DNS_query) {
  const pm::Property* p;
  p = get_property(73);   // packet #74

  EXPECT_TRUE(p->has_value("DNS.tx_id"));
  EXPECT_TRUE(p->has_value("DNS.question"));
  EXPECT_FALSE(p->has_value("DNS.answer"));
  EXPECT_FALSE(p->has_value("DNS.authority"));
  EXPECT_FALSE(p->has_value("DNS.additional"));

  const auto& q = p->value("DNS.question");
  EXPECT_TRUE(q.is_array());
  EXPECT_EQ(1u, q.size());

  for (size_t i = 0; i < q.size(); i++) {
    const auto& r = q.get(i);
    EXPECT_FALSE(r.is_null());
    EXPECT_EQ("www.nicovideo.jp.", r.find("name").repr());
    EXPECT_EQ("A",               r.find("type").repr());
  }
}

TEST_F(ModuleTesterData1, NameService_DNS_reply) {
  const pm::Property* p;
  p = get_property(1838);   // packet #77

  EXPECT_TRUE(p->has_value("DNS.tx_id"));
  EXPECT_TRUE(p->has_value("DNS.question"));
  EXPECT_TRUE(p->has_value("DNS.answer"));
  EXPECT_FALSE(p->has_value("DNS.authority"));
  EXPECT_FALSE(p->has_value("DNS.additional"));

  {
    const auto& q = p->value("DNS.question");
    EXPECT_TRUE(q.is_array());
    EXPECT_EQ(1u, q.size());

    const auto& r = q.get(0);
    EXPECT_FALSE(r.is_null());
    EXPECT_EQ("y.one.impact-ad.jp.", r.find("name").repr());
    EXPECT_EQ("A", r.find("type").repr());
  }

  {
    const auto& a = p->value("DNS.answer");
    EXPECT_FALSE(a.is_map());
    EXPECT_TRUE(a.is_array());
    EXPECT_EQ(8u, a.size());

    const auto& r0 = a.get(0);
    EXPECT_TRUE(r0.is_map());
    EXPECT_FALSE(r0.is_array());
    EXPECT_EQ("y.one.impact-ad.jp.", r0.find("name").repr());
    EXPECT_EQ("A",                   r0.find("type").repr());
    EXPECT_EQ("54.65.207.193",       r0.find("data").repr());

    const auto& r6 = a.get(7);
    EXPECT_EQ("y.one.impact-ad.jp.", r6.find("name").repr());
    EXPECT_EQ("A",                   r6.find("type").repr());
    EXPECT_EQ("54.65.247.186",       r6.find("data").repr());
  }
}
