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

#ifndef __PACKETMACHINE_EXCEPTION_HPP
#define __PACKETMACHINE_EXCEPTION_HPP

#include <exception>
#include <sstream>
#include <string>

namespace pm {
namespace Exception {

// Exception::Error is base exception of packetmachine.
// Exception classes of packetmachine should inherit Exception::Error
// if there is no special reason.

class Error : public std::exception {
 private:
  std::string errmsg_;
 public:
  explicit Error(const std::string &errmsg) : errmsg_(errmsg) {}
  virtual ~Error() throw() {}
  virtual const char* what() const throw() {
    return this->errmsg_.c_str();
  }
};

// ConfigError for invalid preparation.

class ConfigError : public Error {
 public:
  explicit ConfigError(const std::string &errmsg) : Error(errmsg) {}
};

class TypeError : public Error {
 public:
  explicit TypeError(const std::string &errmsg) : Error(errmsg) {}
};

class IndexError : public Error {
 public:
  explicit IndexError(const std::string &errmsg) : Error(errmsg) {}
};

}   // namespace Exception
}   // namespace pm

#endif    // __PACKETMACHINE_EXCEPTION_HPP
