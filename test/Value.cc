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
#include "../src/packetmachine/value.hpp"

namespace machine_test {

TEST(Value, basic) {
  pm::byte_t a[] = {1, 2, 3, 4, 5};
  pm::Value v;

  EXPECT_FALSE(v.active());

  // set value
  v.set(a, 4);
  EXPECT_TRUE(v.active());
  EXPECT_EQ(4u, v.len());
  EXPECT_EQ("1.2.3.4", v.ip4());

  v.clear();
  EXPECT_FALSE(v.active());
  EXPECT_TRUE(v.ip4().empty());
}


TEST(Value, set_cpy) {
  pm::byte_t a[] = {10, 20, 30, 40};
  pm::Value v1, v2;
  v1.set(a, 4);
  v2.cpy(a, 4);

  EXPECT_EQ(40u, v1.raw()[3]);
  EXPECT_EQ(40u, v2.raw()[3]);

  // replace original data
  a[3] = 13;
  EXPECT_EQ(13u, v1.raw()[3]);
  EXPECT_EQ(40u, v2.raw()[3]);
}


TEST(Value, ip4) {
  pm::byte_t a[] = {1, 2, 3, 4, 5};
  pm::Value v;

  v.set(a, 4);
  EXPECT_TRUE(v.is_ip4());
  EXPECT_EQ("1.2.3.4", v.ip4());

  v.set(a, 5);
  EXPECT_FALSE(v.is_ip4());
  EXPECT_EQ("", v.ip4());

  v.set(a, 3);
  EXPECT_FALSE(v.is_ip4());
  EXPECT_EQ("", v.ip4());
}


TEST(Value, ip6) {
  pm::byte_t a[] = {
    32,  1,  0,  2,
     0,   3,  0,  4,
     0,   0,  0,  0,
     0,   0,  0,  1,
    99, 99, 99, 99,
  };
  pm::Value v;

  v.set(a, 16);
  EXPECT_TRUE(v.is_ip6());
  EXPECT_EQ("2001:2:3:4::1", v.ip6());

  v.set(a, 15);
  EXPECT_FALSE(v.is_ip6());
  EXPECT_EQ("", v.ip6());

  v.set(a, 17);
  EXPECT_FALSE(v.is_ip6());
  EXPECT_EQ("", v.ip6());
}


TEST(Value, mac) {
  pm::byte_t a[] = {0xff, 1, 2, 3, 4, 5, 6};
  pm::Value v;

  v.set(a, 6);
  EXPECT_TRUE(v.is_mac());
  EXPECT_EQ("ff:01:02:03:04:05", v.mac());

  v.set(a, 5);
  EXPECT_FALSE(v.is_mac());
  EXPECT_TRUE(v.mac().empty());

  v.set(a, 7);
  EXPECT_FALSE(v.is_mac());
  EXPECT_TRUE(v.mac().empty());
}


TEST(Value, uint) {
  pm::byte_t a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  pm::Value v;

  v.set(a, 1);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(1u, v.uint64());

  v.set(a, 2);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x102u, v.uint64());

  v.set(a, 4);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x01020304u, v.uint64());

  v.set(a, 8);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x0102030405060708u, v.uint64());

  v.set(a, 0);
  EXPECT_FALSE(v.is_uint());
  v.set(a, 3);
  EXPECT_FALSE(v.is_uint());
  v.set(a, 5);
  EXPECT_FALSE(v.is_uint());
  v.set(a, 7);
  EXPECT_FALSE(v.is_uint());
  v.set(a, 9);
  EXPECT_FALSE(v.is_uint());
}


TEST(Value, endian) {
  pm::byte_t a[] = {1, 2, 3, 4, 5, 6, 7, 8};
  pm::Value v;

  v.set(a, 2, pm::Value::BIG);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x102u, v.uint64());

  v.set(a, 2, pm::Value::LITTLE);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x201u, v.uint64());


  v.set(a, 4, pm::Value::BIG);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x01020304u, v.uint64());

  v.set(a, 4, pm::Value::LITTLE);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x04030201u, v.uint64());


  v.set(a, 8, pm::Value::BIG);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x0102030405060708u, v.uint64());

  v.set(a, 8, pm::Value::LITTLE);
  EXPECT_TRUE(v.is_uint());
  EXPECT_EQ(0x0807060504030201u, v.uint64());
}


}   // namespace machine_test
