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

#include <string.h>
#include "./gtest/gtest.h"

#include "../src/packet.hpp"

TEST(Packet, store) {
  pm::byte_t a[] = {1, 2, 3, 4, 5};
  pm::Packet pkt;
  timeval tv = {100, 200};   // tv_sec, tv_usec

  // store
  pkt.store(a, 4);
  pkt.set_cap_len(10);
  pkt.set_tv(tv);

  // clear original data
  ::memset(a, 0, sizeof(a));
  ::memset(&tv, 0, sizeof(tv));
  EXPECT_EQ(0, a[0]);

  // test
  auto tv_s = pkt.tv();
  const pm::byte_t* p = pkt.buf();

  EXPECT_EQ(4, pkt.len());
  EXPECT_EQ(1, p[0]);
  EXPECT_EQ(4, p[3]);
  EXPECT_EQ(100, tv_s.tv_sec);
  EXPECT_EQ(200, tv_s.tv_usec);
}
