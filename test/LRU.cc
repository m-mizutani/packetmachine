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

TEST(LruHash, basic_test) {
  pm::LruHash<MyData*> lru(10);
  MyData d1(1), d2(2), d3(3);
  pm::Buffer k1(&d1.a_, sizeof(d1.a_));
  pm::Buffer k2(&d2.a_, sizeof(d2.a_));
  pm::Buffer k3(&d3.a_, sizeof(d3.a_));

  // put node
  EXPECT_TRUE(lru.put(1, k1, &d1));
  const auto n1 = lru.get(k1);
  EXPECT_FALSE(n1.is_null());
  EXPECT_EQ(&d1, n1.data());
  EXPECT_FALSE(lru.has_expired());

  // still alive
  lru.update(1);
  const auto n2 = lru.get(k1);
  EXPECT_FALSE(n2.is_null());
  EXPECT_EQ(&d1, n2.data());
  EXPECT_FALSE(lru.has_expired());

  // expired
  lru.update(1);
  const auto n3 = lru.get(k1);
  EXPECT_TRUE(n3.is_null());
  EXPECT_TRUE(lru.has_expired());
  EXPECT_EQ(&d1, lru.pop());
}

}  // namespace lru_test
