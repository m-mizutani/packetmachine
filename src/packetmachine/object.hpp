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
// Object is abstruction class to present decoded items from packet(s).
// e.g. source TCP port number from TCP header.
//
// A decode module developer basically uses Value class or create a new
// class inheriting Value class. However some fields of decoded result of
// packet has complex structure. For example, DNS packet has any number of
// query records. Then, PacketMachine has not only Value class but also
// Array and Map structure. They inherit from Object class in order to have
// represent function and abstruct variable to store in memory.
//
// Child classes of Object class must has 2 methods:
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


class Object {
 public:
  Object() = default;
  virtual ~Object() {}

  template <typename T> const T& as() const {
    const T* ptr = dynamic_cast<const T*>(this);
    if (ptr) {
      return *ptr;
    } else {
      std::string msg = "Can not convert to ";
      msg += typeid(T).name();
      throw Exception::TypeError(msg);
    }
  }

  template <typename T> bool is() const {
    const T* ptr = dynamic_cast<const T*>(this);
    return (ptr != nullptr);
  }

  virtual void clear() = 0;
  virtual void repr(std::ostream &os) const = 0;
  std::string repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Object& obj);
};


class Array : public Object {
 private:
  std::vector<Object*> array_;

 public:
  Array();
  virtual ~Array();
  void clear();
  void repr(std::ostream &os) const;
  void push(Object *obj);
};


class Map : public Object {
 private:
  std::map<std::string, Object*> map_;

 public:
  Map();
  virtual ~Map();
  void clear();
  void repr(std::ostream &os) const;
  void insert(const std::string& key, Object* obj);
};


class Value : public Object {
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

 public:
  Value();
  virtual ~Value();

  void set(const void* ptr, size_t len, Endian e = BIG);
  void cpy(const void* ptr, size_t len, Endian e = BIG);

  virtual void clear();
  virtual void repr(std::ostream &os) const;

  bool active() const { return this->active_; }
  size_t len() const { return this->len_; }

  virtual bool hex(std::ostream &os) const;
  virtual bool ip4(std::ostream &os) const;
  virtual bool ip6(std::ostream &os) const;
  virtual bool mac(std::ostream &os) const;
  virtual bool uint(uint64_t* d) const;

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
  uint64_t uint() const;
  const byte_t* raw(size_t* len = nullptr) const;
};


/*
 * General Value Type
 */
class IPv4Addr : public Value {
 public:
  IPv4Addr() = default;
  ~IPv4Addr() = default;
  void repr(std::ostream &os) const { this->ip4(os); }
  static Object* new_value() { return new IPv4Addr(); }
};

class PortNumber : public Value {
 public:
  PortNumber() = default;
  ~PortNumber() = default;
  void repr(std::ostream &os) const {
    uint64_t d;
    this->uint(&d);
    os << d;
  }
  static Object* new_value() { return new PortNumber(); }
};


}   // namespace pm

#endif   // __PACKETMACHINE_OBJECT_HPP__
