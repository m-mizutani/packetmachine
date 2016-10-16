/*-
 * Copyright (c) 2016 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
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

#ifndef PACKETMACHINE_UTILS_LRU_HPP__
#define PACKETMACHINE_UTILS_LRU_HPP__

#include <map>
#include <vector>
#include <deque>
#include <string>

namespace pm {

class LRUHash {
  class Bucket;
  static const size_t DEFAULT_BUCKET_SIZE = 1031;

 public:
  class Node {
   private:
    Node *next_, *prev_;  // double linked list for Bucket
    Node *link_;          // single linked list for TimeSlot
    int update_;

   public:
    Node();
    virtual ~Node();
    virtual uint64_t hash() = 0;
    virtual bool match(const void *key, size_t len) = 0;
    void attach(Node *node);
    void detach();
    Node *pop_all();
    void push_link(Node * prev);
    Node *pop_link();
    Node *search(uint64_t hv, const void *key, size_t len);
  };

 private:
  class NodeRoot : public Node {
    uint64_t hash() { return 0; }
    bool match(const void *key, size_t len) { return false; }
  };
  class Bucket {
   private:
    NodeRoot root_;
   public:
    Bucket();
    ~Bucket();
    void attach(Node *node);
    Node* search(uint64_t hv, const void *key, size_t len);
  };

  class TimeSlot {
   private:
    NodeRoot root_;
   public:
    TimeSlot();
    ~TimeSlot();
    void push(Node *node);
    Node* pop();
  };

  std::vector<TimeSlot> timeslot_;
  std::vector<Bucket> bucket_;
  size_t curr_tick_;
  NodeRoot exp_node_;

 public:
  explicit LRUHash(size_t timeslot_size,
                   size_t bucket_size = DEFAULT_BUCKET_SIZE);
  ~LRUHash();
  bool put(size_t tick, Node *node);
  Node *get(uint64_t hv, const void *key, size_t len);
  void update(size_t tick = 1);  // progress tick
  Node *pop();  // pop expired node
};

}  // namespace pm

#endif  // PACKETMACHINE_UTILS_LRU_HPP__
