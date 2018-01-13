/*
 * Copyright (c) 2018 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
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

#include <unistd.h>
#include "./gtest/gtest.h"
#include "../src/packetmachine.hpp"

namespace machine_test {

const long TIMER_BLOCKING_DURATION = 1000 * 1000 * 1000;

TEST(Timer, set_timeout) {
  pm::Machine *m = new pm::Machine();
  int count = 0;

  m->set_timeout([&]() {
      count++;
    }, 1);
  m->add_blocker(TIMER_BLOCKING_DURATION);
  m->loop();

  EXPECT_EQ(1, count);
  delete m;
}

TEST(Timer, set_interval) {
  pm::Machine *m = new pm::Machine();
  int count = 0;

  m->set_interval([&]() {
      count++;
    }, 1);
  m->add_blocker(TIMER_BLOCKING_DURATION);
  m->loop();

  EXPECT_GT(count, 1);
  delete m;
}

TEST(Timer, set_two_interval) {
  pm::Machine *m = new pm::Machine();
  int count1 = 0;
  int count2 = 0;

  m->set_interval([&]() { count1++; }, 1);
  m->set_interval([&]() { count2++; }, 2);

  m->add_blocker(TIMER_BLOCKING_DURATION);
  m->loop();

  EXPECT_GT(count1, 1);
  EXPECT_GT(count2, 1);
  EXPECT_GE(count1, count2);

  delete m;
}

}   // namespace machine_test
