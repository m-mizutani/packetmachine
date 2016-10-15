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

#include "./fixtures.hpp"

TEST_F(ModuleTesterData1, ARP_packet) {
  const pm::Property* p;
  p = get_property(1296);   // # packet #1297

  EXPECT_TRUE(p->has_value("ARP.hw_type"));   // Ethernet
  EXPECT_TRUE(p->has_value("ARP.pr_type"));   // IPv4
  EXPECT_TRUE(p->has_value("ARP.hw_size"));   // MAC address (6 byte)
  EXPECT_TRUE(p->has_value("ARP.pr_size"));   // IPv4 address (4 byte)

  EXPECT_EQ(0x001, p->value("ARP.hw_type").uint());   // Ethernet
  EXPECT_EQ(0x800, p->value("ARP.pr_type").uint());   // IPv4
  EXPECT_EQ(6,     p->value("ARP.hw_size").uint());   // MAC address (6 byte)
  EXPECT_EQ(4,     p->value("ARP.pr_size").uint());   // IPv4 address (4 byte)
}

