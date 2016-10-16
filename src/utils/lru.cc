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
o * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "./lru.hpp"
#include "../debug.hpp"

namespace pm {
  LRUHash::LRUHash(size_t timeslot_size, size_t bucket_size) :
    timeslot_(timeslot_size), bucket_(bucket_size), curr_tick_(0) {
    if (this->bucket_.size() == 0) {
      this->bucket_.resize(LRUHash::DEFAULT_BUCKET_SIZE);
    }
  }
  LRUHash::~LRUHash() {
  }
  bool LRUHash::put(size_t tick, LRUHash::Node *node) {
    if (tick >= this->timeslot_.size()) {
      return false;
    }

    size_t ptr = node->hash() % this->bucket_.size();
    this->bucket_[ptr].attach(node);

    size_t tp = (tick + this->curr_tick_) % this->timeslot_.size();
    this->timeslot_[tp].push(node);

    return true;
  }
  LRUHash::Node *LRUHash::get(uint64_t hv, const void *key, size_t len) {
    size_t ptr = hv % this->bucket_.size();
    return this->bucket_[ptr].search(hv, key, len);
  }
  void LRUHash::update(size_t tick) {
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
  LRUHash::Node *LRUHash::pop() {
    return this->exp_node_.pop_link();
  }

  // class LRUHash::Timeslot
  LRUHash::TimeSlot::TimeSlot() {
  }
  LRUHash::TimeSlot::~TimeSlot() {
  }

  void LRUHash::TimeSlot::push(LRUHash::Node *node) {
    this->root_.push_link(node);
  }

  LRUHash::Node* LRUHash::TimeSlot::pop() {
    return this->root_.pop_link();
  }


  // class LRUHash::Bucket
  LRUHash::Bucket::Bucket() {
  }
  LRUHash::Bucket::~Bucket() {
  }

  void LRUHash::Bucket::attach(Node* node) {
    this->root_.attach(node);
  }

  LRUHash::Node* LRUHash::Bucket::search(uint64_t hv, const void *key,
                                         size_t len) {
    return this->root_.search(hv, key, len);
  }

  // class LRUHash::Node
  LRUHash::Node::Node() : next_(nullptr), prev_(nullptr), link_(nullptr),
                          update_(0) {
  }
  LRUHash::Node::~Node() {
  }

  void LRUHash::Node::attach(Node *node) {
    Node *prev = this;
    Node *next = this->next_;

    node->prev_ = prev;
    node->next_ = next;
    prev->next_ = node;
    if (next) {
      next->prev_ = node;
    }
  }
  void LRUHash::Node::detach() {
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
  void LRUHash::Node::push_link(Node * node) {
    Node * next = this->link_;
    node->link_ = next;
    this->link_ = node;
  }
  LRUHash::Node* LRUHash::Node::pop_link() {
    Node *node = this->link_;
    if (node) {
      Node *next = node->link_;
      this->link_ = next;
    }
    return node;
  }
  LRUHash::Node* LRUHash::Node::pop_all() {
    Node * all = this->link_;
    this->link_ = nullptr;
    return all;
  }

  LRUHash::Node* LRUHash::Node::search(uint64_t hv, const void *key,
                                       size_t len) {
    for (Node *node = this->next_; node != nullptr; node = node->next_) {
      if (node->hash() == hv && node->match(key, len)) {
        return node;
      }
    }
    return nullptr;
  }

}  // namespace pm
