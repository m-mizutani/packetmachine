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

#include "./packetmachine/object.hpp"

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include "./debug.hpp"

namespace pm {

std::string Object::repr() const {
  std::stringstream ss;
  this->repr(ss);
  return ss.str();
}


std::ostream& operator<<(std::ostream& os, const Object& obj) {
  os << obj.repr();
  return os;
}


void Array::clear() {
  // TODO(m-mizutani): implement
}

void Array::repr(std::ostream &os) const {
  // TODO(m-mizutani): implement
}

void Array::push(Object *obj) {
  // TODO(m-mizutani): implement
}


Map::Map() {
  // TODO(m-mizutani): implement
}

Map::~Map() {
  // TODO(m-mizutani): implement
}

void Map::clear() {
  // TODO(m-mizutani): implement
}

void Map::repr(std::ostream &os) const {
  // TODO(m-mizutani): implement
}

void Map::insert(const std::string& key, Object* obj) {
  // TODO(m-mizutani): implement
}



Value::Value() :
    active_(false), ptr_(nullptr), len_(0), buf_(nullptr),
    buf_len_(0) {
}

Value::~Value() {
  if (this->buf_) {
    free(this->buf_);
  }
}


void Value::set(const void* ptr, size_t len, Endian e) {
  this->active_ = true;
  this->endian_ = e;
  this->ptr_ = static_cast<const byte_t*>(ptr);
  this->len_ = len;
}


void Value::cpy(const void* ptr, size_t len, Endian e) {
  if (this->buf_len_ < len) {
    // TODO(m-mizutani): handling memory allocation error
    this->buf_ = static_cast<byte_t*>(::realloc(this->buf_, len));
    this->buf_len_ = len;
  }

  this->active_ = true;
  this->endian_ = e;
  ::memcpy(this->buf_, ptr, len);

  this->ptr_ = this->buf_;
  this->len_ = len;
}


void Value::clear() {
  this->active_ = false;
}


void Value::repr(std::ostream &os) const {
  if (this->active_) {
    for (size_t i = 0; i < this->len_; i++) {
      byte_t c = this->ptr_[i];
      os << (isprint(c) ? static_cast<char>(c) : '.');
    }
  }
}

std::string Value::repr() const {
  std::stringstream ss;
  this->repr(ss);
  return ss.str();
}


bool Value::hex(std::ostream &os) const {
  return false;
}


bool Value::ip4(std::ostream &os) const {
  if (this->is_ip4()) {
    char addr[INET_ADDRSTRLEN];
    
    /*
      implement original inet_ntop below because of performance instead of:
      const char* p = inet_ntop(AF_INET, this->ptr_, addr, sizeof(addr));
    */
    
    char* p = &addr[0];
    for (int i = 0; i < 4; i++) {
      int n = this->ptr_[i];
      if (n >= 100) {
        *p = (n / 100) + '0';
        p++;
      }
      if (n >= 10) {
        *p = ((n / 10) % 10) + '0';
        p++;
      }
      p[0] = (n % 10) + '0';
      p[1] = '.';
      p += 2;
    }

    p[-1] = '\0';
    
    // assert(p);
    os << addr;
    return true;
  } else {
    return false;
  }
}


bool Value::ip6(std::ostream &os) const {
  if (this->is_ip6()) {
    char addr[INET6_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET6, this->ptr_, addr, sizeof(addr));
    assert(p);
    os << addr;
    return true;
  } else {
    return false;
  }
}


bool Value::mac(std::ostream &os) const {
  if (this->is_mac()) {
    for (size_t i = 0; i < 6; i++) {
      os << std::hex << std::setw(2) << std::setfill('0')
         << static_cast<int>(this->ptr_[i]);
      if (i < 5) {
        os << ":";
      }
    }
    return true;
  } else {
    return false;
  }
}


bool Value::uint(uint64_t* d) const {
  if (this->is_uint()) {
    switch (this->len_) {
      // uint8_t
      case 1: {
        uint8_t t = this->ptr_[0];
        *d = static_cast<uint64_t>(t);
      } break;

      // uint16_t
      case 2: {
        uint16_t t = *(reinterpret_cast<const uint16_t*>(this->ptr_));
        if (this->endian_ == BIG) {
          t = ntohs(t);
        }
        *d = static_cast<uint64_t>(t);
      } break;

      // uint32_t
      case 4: {
        uint32_t t = *(reinterpret_cast<const uint32_t*>(this->ptr_));
        if (this->endian_ == BIG) {
          t = ntohl(t);
        }
        *d = static_cast<uint64_t>(t);
      } break;

      // uint64_t
      case 8: {
        uint64_t r = *(reinterpret_cast<const uint64_t*>(this->ptr_));
#if __BYTE_ORDER == __LITTLE_ENDIAN
        if (this->endian_ == BIG) {
          r = ((r & 0xFF00000000000000ull) >> 56) |
              ((r & 0x00FF000000000000ull) >> 40) |
              ((r & 0x0000FF0000000000ull) >> 24) |
              ((r & 0x000000FF00000000ull) >>  8) |
              ((r & 0x00000000FF000000ull) <<  8) |
              ((r & 0x0000000000FF0000ull) << 24) |
              ((r & 0x000000000000FF00ull) << 40) |
              ((r & 0x00000000000000FFull) << 56);
        }
#endif
        *d = r;
      } break;

      default:
        assert(0);
        break;
    }
    return true;
  } else {
    return false;
  }
}



std::string Value::hex() const {
  std::stringstream ss;
  this->hex(ss);
  return ss.str();
}

std::string Value::ip4() const {
  std::stringstream ss;
  this->ip4(ss);
  return ss.str();
}

std::string Value::ip6() const {
  std::stringstream ss;
  this->ip6(ss);
  return ss.str();
}

std::string Value::mac() const {
  std::stringstream ss;
  this->mac(ss);
  return ss.str();
}

uint64_t Value::uint() const {
  if (this->active_ && this->is_uint()) {
    uint64_t d;
    this->uint(&d);
    return d;
  } else {
    return 0;
  }
}

const byte_t* Value::raw(size_t* len) const {
  if (this->active_) {
    if (len) {
      *len = this->len_;
    }
    return this->ptr_;
  } else {
    return nullptr;
  }
}


}   // namespace pm
