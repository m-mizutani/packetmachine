/*-
 * Copyright (c) 2013 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
 * All rights reserved.
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

#include <string.h>
#include "./gtest/gtest.h"
#include "../src/utils/lru.hpp"
#include "../src/debug.hpp"


namespace lru_test {

class MyData {
 public:
  explicit MyData(int a) : a_(a) {}
  int a_;
};

TEST(LruHash, basic) {
  pm::LruHash<MyData*> lru(10);
  MyData d1(1);
  pm::Buffer k1(&d1.a_, sizeof(d1.a_));

  // put node
  EXPECT_TRUE(lru.put(2, k1, &d1));
  const auto n1 = lru.get(k1);
  EXPECT_FALSE(n1.is_null());
  EXPECT_EQ(&d1, n1.data());
  EXPECT_FALSE(lru.has_expired());

  // still alive
  lru.update(1);
  EXPECT_TRUE(lru.has(k1));
  EXPECT_FALSE(lru.has_expired());

  // expired
  lru.update(1);
  EXPECT_FALSE(lru.has(k1));
  EXPECT_TRUE(lru.has_expired());
  EXPECT_EQ(&d1, lru.pop());
}

TEST(LruHash, multiple_data) {
  pm::LruHash<MyData*> lru(10);
  MyData d1(1), d2(2);
  pm::Buffer k1(&d1.a_, sizeof(d1.a_));
  pm::Buffer k2(&d2.a_, sizeof(d2.a_));

  // put node
  EXPECT_TRUE(lru.put(2, k1, &d1));
  EXPECT_TRUE(lru.put(3, k2, &d2));

  auto n1 = lru.get(k1);
  EXPECT_FALSE(n1.is_null());
  EXPECT_EQ(&d1, n1.data());
  auto n2 = lru.get(k2);
  EXPECT_FALSE(n2.is_null());
  EXPECT_EQ(&d2, n2.data());

  lru.update(2);
  EXPECT_FALSE(lru.has(k1));  // expired
  EXPECT_TRUE(lru.has(k2));   // still alive

  lru.update(1);
  EXPECT_FALSE(lru.has(k1));  // expired
  EXPECT_FALSE(lru.has(k2));  // expired
}


TEST(LruHash, update) {
  pm::LruHash<MyData*> lru(10);
  MyData d1(1), d2(2), d3(3);
  pm::Buffer k1(&d1.a_, sizeof(d1.a_));
  pm::Buffer k2(&d2.a_, sizeof(d2.a_));
  pm::Buffer k3(&d3.a_, sizeof(d3.a_));

  // start, tick: 0

  // put node
  EXPECT_TRUE(lru.put(3, k1, &d1));
  EXPECT_TRUE(lru.put(3, k2, &d2));
  EXPECT_TRUE(lru.put(3, k3, &d3));

  lru.update(2);  // tick: 2

  auto n1 = lru.get(k1);   // k1 hit cache, and updated.
  // next expiring tick is 2 (current tick) + 3 (original TTL)  = 5
  EXPECT_FALSE(n1.is_null());
  EXPECT_EQ(&d1, n1.data());

  auto n2 = lru.get(k2);   // k2 hit cache, and updated.
  // next expiring tick is 2 (current tick) + 3 (original TTL)  = 5
  EXPECT_FALSE(n2.is_null());
  EXPECT_EQ(&d2, n2.data());


  EXPECT_TRUE(lru.has(k1));   // k1 is still alive
  EXPECT_TRUE(lru.has(k2));   // k2 is still alive
  EXPECT_TRUE(lru.has(k3));   // k3 is still alive

  lru.update(1);  // tick: 3

  EXPECT_TRUE(lru.has(k1));   // k1 is still alive
  EXPECT_TRUE(lru.has(k2));   // k2 is still alive
  EXPECT_FALSE(lru.has(k3));  // k3 has been expired

  lru.update(1);  // tick: 4

  EXPECT_TRUE(lru.has(k1));   // k1 is still alive
  EXPECT_TRUE(lru.has(k2));   // k2 is still alive
  EXPECT_FALSE(lru.has(k3));  // k3 has been expired

  auto n3 = lru.get(k2);   // k2 hit cache, and updated.
  // next expiring tick is 4 (current tick) + 3 (original TTL)  = 7
  EXPECT_FALSE(n2.is_null());
  EXPECT_EQ(&d2, n2.data());

  lru.update(1);  // tick: 5

  EXPECT_FALSE(lru.has(k1));  // k1 has been expired
  EXPECT_TRUE(lru.has(k2));   // k2 is still alive
  EXPECT_FALSE(lru.has(k3));  // k3 has been expired

  lru.update(1);  // tick: 6

  EXPECT_FALSE(lru.has(k1));  // k1 has been expired
  EXPECT_TRUE(lru.has(k2));   // k2 is still alive
  EXPECT_FALSE(lru.has(k3));  // k3 has been expired

  lru.update(1);  // tick: 7

  EXPECT_FALSE(lru.has(k1));  // k1 has been expired
  EXPECT_FALSE(lru.has(k2));  // k2 has been expired
  EXPECT_FALSE(lru.has(k3));  // k3 has been expired
}

}  // namespace lru_test
