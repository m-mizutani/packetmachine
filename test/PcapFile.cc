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

#include "./gtest/gtest.h"
#include "../src/capture.hpp"
#include "../src/packet.hpp"

TEST(PcapFile, ok) {
  pm::PcapFile *pfile = new pm::PcapFile("./test/data1.pcap");
  ASSERT_TRUE(pfile->ready());

  pm::Packet pkt;
  int count = 0;
  while (0 <= (pfile->read(&pkt))) {
    count += 1;
  }

  EXPECT_TRUE(pfile->error().empty());
  EXPECT_EQ(count, 10000);
}

TEST(PcapFile, ng_no_such_file) {
  pm::PcapFile *pfile = new pm::PcapFile("./test/no_such_file.pcap");
  ASSERT_FALSE(pfile->ready());
  EXPECT_EQ(pfile->error(),
            "./test/no_such_file.pcap: No such file or directory");

  pm::Packet pkt;
  EXPECT_EQ(pfile->read(&pkt), -1);
  EXPECT_EQ(pfile->error(), "pcap is not ready");
}

TEST(PcapFile, ng_invalid_format) {
  // Open file that is not a pcap format file.
  pm::PcapFile *pfile = new pm::PcapFile("./test/main.cc");
  ASSERT_FALSE(pfile->ready());
  EXPECT_EQ(pfile->error(), "unknown file format");

  pm::Packet pkt;
  EXPECT_EQ(pfile->read(&pkt), -1);
  EXPECT_EQ(pfile->error(), "pcap is not ready");
}
