/*
 * Copyright (c) 2017 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
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

#include "./gtest/gtest.h"
#include "../src/packetmachine.hpp"

namespace machine_test {

TEST(Handler, ok) {
  pm::Machine *m = new pm::Machine();
  int count = 0;

  pm::hdlr_id hid = m->on("UDP", [&](const pm::Property& p) {
      count++;
    });

  m->add_pcapfile("./test/data1.pcap");
  m->loop();

  EXPECT_NE(pm::Handler::NONE, hid);
  EXPECT_EQ(624, count);
}

TEST(Handler, remove) {
  pm::Machine *m = new pm::Machine();
  int count = 0;

  pm::hdlr_id hid = m->on("UDP", [&](const pm::Property& p) {
      count++;
    });

  EXPECT_TRUE(m->clear(hid));
  EXPECT_FALSE(m->clear(hid)); // double clear
  
  m->add_pcapfile("./test/data1.pcap");
  m->loop();

  EXPECT_NE(pm::Handler::NONE, hid);
  EXPECT_EQ(0, count);
}

}   // namespace machine_test
