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

class TestNode : public pm::LRUHash::Node {
private:
  uint64_t hv_;
  void *key_;
  size_t len_;

public:
  TestNode(uint64_t hv, const std::string &key) : hv_(hv) {
    this->len_ = key.length() * sizeof(char);
    this->key_ = malloc(this->len_);
    memcpy(this->key_, key.c_str(), this->len_);
  }
  ~TestNode() {}
  uint64_t hash() { return this->hv_; }
  bool match(const void *key, size_t len) {
    return (len == this->len_ && memcmp(this->key_, key, len) == 0);
  }
  void *key() { return this->key_; }
  size_t len() { return this->len_; }
};

TEST(LRUHash, basic_test) {
  pm::LRUHash *lru = new pm::LRUHash(10);
  TestNode *node = new TestNode(100, "100");

  // put node
  EXPECT_TRUE(lru->put(1, node));
  EXPECT_EQ(node, lru->get(node->hash(), node->key(), node->len()));

  // still alive
  lru->update(1);
  EXPECT_EQ(node, lru->get(node->hash(), node->key(), node->len()));

  // expired
  lru->update(1);
  EXPECT_EQ(NULL, lru->get(node->hash(), node->key(), node->len()));
  EXPECT_EQ(node, lru->pop());
}

TEST(LRUHash, expire_test) {
  pm::LRUHash *lru = new pm::LRUHash(10);
  TestNode *n1 = new TestNode(100, "100");
  TestNode *n2 = new TestNode(200, "200");
  TestNode *n3 = new TestNode(300, "300");
  TestNode *n4 = new TestNode(400, "400");

  ASSERT_TRUE(lru->put(1, n1));
  ASSERT_TRUE(lru->put(2, n2));
  ASSERT_TRUE(lru->put(4, n3));
  ASSERT_TRUE(lru->put(4, n4));

#define __TEST(r1,r2,r3,r4)                                             \
  {                                                                     \
    if (r1) { EXPECT_EQ(n1,   lru->get(n1->hash(), n1->key(), n1->len())); } \
    else    { EXPECT_EQ(NULL, lru->get(n1->hash(), n1->key(), n1->len())); } \
    if (r2) { EXPECT_EQ(n2,   lru->get(n2->hash(), n2->key(), n2->len())); } \
    else    { EXPECT_EQ(NULL, lru->get(n2->hash(), n2->key(), n2->len())); } \
    if (r3) { EXPECT_EQ(n3,   lru->get(n3->hash(), n3->key(), n3->len())); } \
    else    { EXPECT_EQ(NULL, lru->get(n3->hash(), n3->key(), n3->len())); } \
    if (r4) { EXPECT_EQ(n4,   lru->get(n4->hash(), n4->key(), n4->len())); } \
    else    { EXPECT_EQ(NULL, lru->get(n4->hash(), n4->key(), n4->len())); } \
  }

  __TEST(true, true, true, true);
  EXPECT_EQ(NULL,  lru->pop());  // no expired node

  lru->update(1);

  __TEST(true, true, true, true);
  EXPECT_EQ(NULL, lru->pop());  // no expired node

  lru->update(1);
  __TEST(false, true, true, true);
  EXPECT_EQ(n1, lru->pop()); 

  lru->update(1);
  __TEST(false, false, true, true);
  EXPECT_EQ(n2, lru->pop()); 

  lru->update(3);
  __TEST(false, false, false, false);
  EXPECT_EQ(n3, lru->pop()); 
  EXPECT_EQ(n4, lru->pop()); 

#undef __TEST

}

TEST(LRUHash, tick_test) {
  pm::LRUHash *lru = new pm::LRUHash(10);
  TestNode *n1 = new TestNode(100, "100");
  TestNode *n2 = new TestNode(200, "200");
  
  ASSERT_TRUE(lru->put(1, n1));
  lru->update(1);
  EXPECT_EQ(n1, lru->get(n1->hash(), n1->key(), n1->len()));

  ASSERT_TRUE(lru->put(2, n2));
  lru->update(1);
  EXPECT_EQ(NULL, lru->get(n1->hash(), n1->key(), n1->len()));
  EXPECT_EQ(n2,   lru->get(n2->hash(), n2->key(), n2->len()));
  lru->update(1);
  EXPECT_EQ(NULL, lru->get(n1->hash(), n1->key(), n1->len()));
  EXPECT_EQ(n2,   lru->get(n2->hash(), n2->key(), n2->len()));
  lru->update(1);
  EXPECT_EQ(NULL, lru->get(n1->hash(), n1->key(), n1->len()));
  EXPECT_EQ(NULL, lru->get(n2->hash(), n2->key(), n2->len())); 
}

TEST(LRUHash, error_handle) {
  pm::LRUHash *lru = new pm::LRUHash(10);
  TestNode *n1 = new TestNode(100, "100");
  TestNode *n2 = new TestNode(200, "200");
  
  ASSERT_TRUE (lru->put( 9, n1));
  ASSERT_FALSE(lru->put(10, n2));
  EXPECT_EQ(n1, lru->get(n1->hash(), n1->key(), n1->len()));
  EXPECT_EQ(NULL,  lru->get(n2->hash(), n2->key(), n2->len()));
  lru->update(15);
  EXPECT_EQ(n1, lru->pop());
  EXPECT_EQ(NULL,  lru->pop());
}

