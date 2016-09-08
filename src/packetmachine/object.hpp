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
//
// void repr(std::ostream &os) const:
// In order to present internal data as printable format. It is inspired by
// __repr__() method in Python.
//

class Object {
 public:
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
};


class Array : public Object {
 private:
  std::vector<Object*> array_;

 public:
  Array();
  ~Array();
  void clear();
};


class Map : public Object {
 private:
  std::map<std::string, Object*> map_;

 public:
  Map();
  ~Map();
  void clear();
};


class Value : public Object {
 public:
  enum Endian {
    LITTLE,
    BIG,
  };

 private:
  byte_t* ptr_;
  size_t len_;

  byte_t* buf_;
  size_t buf_len_;
  Endian endian_;

 public:
  Value();
  ~Value();

  void set(const byte_t* ptr, size_t len, Endian e = BIG);
  void store(const byte_t* ptr, size_t len);

  virtual void clear();
  virtual void repr(std::ostream &os) const;

  virtual bool hex(std::ostream &os) const;
  virtual bool ip4(std::ostream &os) const;
  virtual bool ip6(std::ostream &os) const;
  virtual bool mac(std::ostream &os) const;
  virtual bool uint(uint64_t* d) const;

  virtual bool is_hex() const;
  virtual bool is_ip4() const;
  virtual bool is_ip6() const;
  virtual bool is_mac() const;
  virtual bool is_uint() const;

  std::string hex() const;
  std::string ip4() const;
  std::string ip6() const;
  std::string mac() const;
  uint64_t uint() const;
  const byte_t* raw(size_t* len) const;
};

}   // namespace pm

#endif   // __PACKETMACHINE_OBJECT_HPP__
