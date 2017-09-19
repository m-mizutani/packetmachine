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

TEST_F(ModuleTesterData1, DHCP_packet) {
  const pm::Property* p;
  p = get_property(266);   // # packet #267

  EXPECT_TRUE(p->has_value("DHCP.msg_type"));
  EXPECT_TRUE(p->has_value("DHCP.hw_type"));
  EXPECT_TRUE(p->has_value("DHCP.hw_addr_len"));
  EXPECT_TRUE(p->has_value("DHCP.hops"));
  EXPECT_TRUE(p->has_value("DHCP.trans_id"));
  EXPECT_TRUE(p->has_value("DHCP.seconds"));
  EXPECT_TRUE(p->has_value("DHCP.flags"));
  EXPECT_TRUE(p->has_value("DHCP.client_addr"));
  EXPECT_TRUE(p->has_value("DHCP.your_client_addr"));
  EXPECT_TRUE(p->has_value("DHCP.next_server_addr"));
  EXPECT_TRUE(p->has_value("DHCP.relay_agent_addr"));
  EXPECT_TRUE(p->has_value("DHCP.client_hw_addr"));
  EXPECT_FALSE(p->has_value("DHCP.server_host_name"));
  EXPECT_FALSE(p->has_value("DHCP.boot_file_name"));
  EXPECT_TRUE(p->has_value("DHCP.magic_cookie"));
  EXPECT_TRUE(p->has_value("DHCP.options"));

  EXPECT_EQ(2,                   p->value("DHCP.msg_type").uint());
  EXPECT_EQ(1,                   p->value("DHCP.hw_type").uint());
  EXPECT_EQ(6,                   p->value("DHCP.hw_addr_len").uint());
  EXPECT_EQ(1,                   p->value("DHCP.hops").uint());
  EXPECT_EQ(0x5aaca6d2,          p->value("DHCP.trans_id").uint());
  EXPECT_EQ(12,                  p->value("DHCP.seconds").uint());
  EXPECT_EQ(0x8000,              p->value("DHCP.flags").uint());
  EXPECT_EQ("0.0.0.0",           p->value("DHCP.your_client_addr").ip4());
  EXPECT_EQ("182.248.219.15",    p->value("DHCP.next_server_addr").ip4());
  EXPECT_EQ("10.139.96.6",       p->value("DHCP.relay_agent_addr").ip4());
  EXPECT_EQ("00:ac:ca:83:df:9e", p->value("DHCP.client_hw_addr").mac());
  size_t mc_len;
  auto mc_ptr = p->value("DHCP.magic_cookie").raw(&mc_len);
  EXPECT_EQ(4, mc_len);
  EXPECT_TRUE(memcmp(mc_ptr, "\x63\x82\x53\x63", 4) == 0);  
}

