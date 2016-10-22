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
#include "../packetmachine/exception.hpp"
#include "./buffer.hpp"
#include "./hash.hpp"
#include "../debug.hpp"

namespace pm {

class LRUHash {
  class Bucket;
  static const size_t DEFAULT_BUCKET_SIZE = 1031;
  static const uint32_t SEED = 0xc0ffee;

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

  static uint32_t murmur3(const void *key, size_t len);

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


template <typename T>
class LruHash {
 public:
  class Node {
   private:
    T data_;
    const Buffer key_;
    Node *next_, *prev_;  // double linked list for Bucket
    Node *link_;          // single linked list for TimeSlot
    int update_;

   public:
    Node() :
        next_(nullptr), prev_(nullptr), link_(nullptr), update_(0) {
    }
    explicit Node(T data, const Buffer& key)
        : data_(data), key_(key),
          next_(nullptr), prev_(nullptr), link_(nullptr), update_(0) {
    }
    virtual ~Node() {}

    T data() const { return this->data_; }
    virtual bool is_null() const {
      return (this->key_.len() == 0);
    }

    void attach(Node *node) {
      Node *prev = this;
      Node *next = this->next_;

      node->prev_ = prev;
      node->next_ = next;
      prev->next_ = node;
      if (next) {
        next->prev_ = node;
      }
    }
    void detach() {
      Node *next = this->next_;
      Node *prev = this->prev_;
      if (next) {
        next->prev_ = prev;
      }
      if (prev) {
        prev->next_ = next;
      }
      this->next_ = this->prev_ = nullptr;
    }
    void push_link(Node * node) {
      Node * next = this->link_;
      node->link_ = next;
      this->link_ = node;
    }
    Node* pop_link() {
      Node *node = this->link_;
      if (node) {
        Node *next = node->link_;
        this->link_ = next;
      }
      return node;
    }
    Node* pop_all() {
      Node * all = this->link_;
      this->link_ = nullptr;
      return all;
    }

    bool has_link() const {
      return (this->link_ != nullptr);
    }
    
    Node* search(const Buffer& key) {
      if (this->key_ == key) {
        return this;
      } else if (this->next_) {
        return this->next_->search(key);
      } else {
        return nullptr;
      }
    }

    const Buffer& key() const {
      return this->key_;
    }
  };
  
 private:
  // ------------------------------------------
  class Bucket {
   private:
    Node root_;

   public:
    Bucket() = default;
    ~Bucket() = default;
    void attach(Node *node) {
      this->root_.attach(node);      
    }
    Node* search(const Buffer& key) {
      return this->root_.search(key);
    }
  };

  // ------------------------------------------
  class TimeSlot {
   private:
    Node root_;
   public:
    TimeSlot() = default;
    ~TimeSlot() = default;
    void push(Node *node) {
      this->root_.push_link(node);
    }
    Node* pop() {
      return this->root_.pop_link();
    }
  };

  // ------------------------------------------
  std::vector<TimeSlot> timeslot_;
  std::vector<Bucket> bucket_;
  uint64_t curr_tick_;
  Node exp_node_;
  static const size_t DEFAULT_BUCKET_SIZE = 1031;
  Node null_node_;

 public:
  static inline uint32_t key2hv(const Buffer& key) {
    return hash32(key.get(), key.len());
  }

  explicit LruHash(size_t timeslot_size,
                   size_t bucket_size = DEFAULT_BUCKET_SIZE)
      : timeslot_(timeslot_size), bucket_(bucket_size), curr_tick_(0) {
    if (this->bucket_.size() == 0) {
      this->bucket_.resize(DEFAULT_BUCKET_SIZE);
    }
  }
  ~LruHash() {
  }
  
  bool put(uint64_t tick, const Buffer& key, T data) {
    if (tick >= this->timeslot_.size()) {
      return false;
    }

    size_t ptr = key2hv(key) % this->bucket_.size();
    Node *node = new Node(data, key);
    this->bucket_[ptr].attach(node);

    size_t tp = (tick + this->curr_tick_) % this->timeslot_.size();
    this->timeslot_[tp].push(node);

    return true;
  }
  
  const Node& get(const Buffer& key) {
    size_t ptr = key2hv(key) % this->bucket_.size();
    Node* node = this->bucket_[ptr].search(key);
    if (node == nullptr) {
      node = &(this->null_node_);
    }
    
    return *node;
  }
  
  void update(uint64_t tick = 1) {   // progress tick
    for (size_t i = 0; i < tick; i++) {
      size_t tp = (this->curr_tick_ + i) % this->timeslot_.size();

      Node *node;
      while (nullptr != (node = this->timeslot_[tp].pop())) {
        node->detach();
        this->exp_node_.push_link(node);
      }
    }
    this->curr_tick_ += tick;
    return;
  }

  bool has_expired() const {
    return this->exp_node_.has_link();
  }
  
  T pop() {
    Node* node = this->exp_node_.pop_link();
    if (node) {
      T data = node->data();
      delete node;
      return data;
    } else {
      throw Exception::NoDataError("no more expired data");
    }
  }  // pop expired node
};

}  // namespace pm

#endif  // PACKETMACHINE_UTILS_LRU_HPP__
