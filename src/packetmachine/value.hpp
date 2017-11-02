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

#ifndef __PACKETMACHINE_OBJECT_HPP__
#define __PACKETMACHINE_OBJECT_HPP__

#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <iostream>

#include "./common.hpp"
#include "./exception.hpp"

namespace pm {

//
// Value is abstruction class to present decoded items from packet(s).
// e.g. source TCP port number from TCP header.
//
// A decode module developer basically uses Value class or create a new
// class inheriting Value class. However some fields of decoded result of
// packet has complex structure. For example, DNS packet has any number of
// query records. Then, PacketMachine has not only Value class but also
// Array and Map structure. They inherit from Value class in order to have
// represent function and abstruct variable to store in memory.
//
// Value and Child classes must has 2 methods:
//
// void clear():
// Instance of child classes may be reused because of memory allocation/free
// performance. Then, they should have clear method to change status into
// "unavailable". The method is not needed to free internal memory.
// Additionally, clear must be recursive.
//
// void repr(std::ostream &os) const:
// In order to present internal data as printable format. It is inspired by
// __repr__() method in Python.
//


class Value {
 public:
  enum Endian {
    LITTLE,
    BIG,
  };

 private:
  bool active_;

  const byte_t* ptr_;
  size_t len_;

  byte_t* buf_;
  size_t buf_len_;
  Endian endian_;

  // DISALLOW COPY AND ASSIGN
  Value(const Value&);
  void operator=(const Value&);

 public:
  Value();
  virtual ~Value();

  virtual bool is_null() const { return false; }

  void set(const void* ptr, size_t len, Endian e = BIG);
  void cpy(const void* ptr, size_t len, Endian e = BIG);

  virtual inline void clear() { this->active_ = false; }
  virtual void repr(std::ostream &os) const;
  virtual std::string repr() const;

  bool active() const { return this->active_; }
  size_t len() const { return this->len_; }

  virtual bool hex(std::ostream &os) const;
  virtual bool ip4(std::ostream &os) const;
  virtual bool ip6(std::ostream &os) const;
  virtual bool mac(std::ostream &os) const;
  virtual bool uint64(uint64_t* d) const;     // NOLINT, cpplint missunderstand
  virtual bool uint(unsigned int* d) const;
  virtual bool uint(unsigned long* d) const;  // NOLINT, require long type

  inline bool is_hex() const { return this->active_; }
  inline bool is_ip4() const { return (this->active_ && this->len_ == 4); }
  inline bool is_ip6() const { return (this->active_ && this->len_ == 16); }
  inline bool is_mac() const { return (this->active_ && this->len_ == 6); }
  inline bool is_uint() const {
    // support unsigned int less than or equal to 64bit.
    return (this->active_ &&
            (this->len_ == 1 ||    // uint8_t
             this->len_ == 2 ||    // uint16_t
             this->len_ == 4 ||    // uint32_t
             this->len_ == 8));    // uint64_t
  }

  std::string hex() const;
  std::string ip4() const;
  std::string ip6() const;
  std::string mac() const;
  uint64_t uint64() const;
  unsigned int uint() const;
  const byte_t* raw(size_t* len = nullptr) const;

  virtual bool is_vector() const { return false; }

  // ------------
  // support Array and Map structure
  virtual size_t size() const { return 0; }

  // for Array
  virtual bool is_array() const { return false; }
  virtual const Value& get(size_t idx) const {
    throw Exception::TypeError("not Array");
  }
  virtual void push(Value* val) {
    throw Exception::TypeError("not Array");
  }

  // for Map
  virtual bool is_map() const { return false; }
  virtual const Value& find(const std::string& key) const {
    throw Exception::TypeError("not Map");
  }
  virtual void insert(const std::string& key, Value* val) {
    throw Exception::TypeError("not Map");
  }

  // Output stream
  friend std::ostream& operator<<(std::ostream& os, const Value& obj);
};


namespace value {

class NoneValue : public Value {
 public:
  NoneValue() {}
  ~NoneValue() {}
  bool is_null() const { return true; }
};

static const NoneValue NONE;

class Array : public Value {
 protected:
  std::vector<Value*> array_;

 public:
  Array() = default;
  virtual ~Array();
  virtual void clear();
  virtual void repr(std::ostream &os) const;

  virtual size_t size() const;
  virtual bool is_array() const { return true; }
  virtual const Value& get(size_t idx) const;
  virtual void push(Value* val);

  static Value* new_value() { return new Array(); }
};


class Map : public Value {
 protected:
  std::map<std::string, Value*> map_;

 public:
  Map() = default;
  virtual ~Map();
  virtual void clear();
  virtual void repr(std::ostream &os) const;

  virtual size_t size() const;
  virtual bool is_map() const { return true; }
  virtual const Value& find(const std::string& key) const;
  virtual void insert(const std::string& key, Value* val);

  static Value* new_value() { return new Map(); }
};



/*
 * General Value Type
 */
class IPv4Addr : public Value {
 public:
  IPv4Addr() = default;
  ~IPv4Addr() = default;
  void repr(std::ostream &os) const { this->ip4(os); }
  static Value* new_value() { return new IPv4Addr(); }
};

class IPv6Addr : public Value {
 public:
  IPv6Addr() = default;
  ~IPv6Addr() = default;
  void repr(std::ostream &os) const { this->ip6(os); }
  static Value* new_value() { return new IPv6Addr(); }
};

class PortNumber : public Value {
 public:
  PortNumber() = default;
  ~PortNumber() = default;
  void repr(std::ostream &os) const {
    uint64_t d;
    this->uint64(&d);  // NOLINT, cpplint missunderstand this line
    os << d;
  }
  static Value* new_value() { return new PortNumber(); }
};

}   // namespace value

}   // namespace pm

#endif   // __PACKETMACHINE_OBJECT_HPP__
