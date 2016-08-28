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

#include "./perf.hpp"

PerfTestList global_pt_list_ __attribute__((init_priority(101)));

PerfTest::PerfTest() {
  global_pt_list_.add_perf_test(this);
}


void PerfTestList::add_perf_test(PerfTest *pt) {
  this->pt_list_.push_back(pt);
}

void PerfTestList::run_perf_test() {
  for (auto& pt : this->pt_list_) {
    pt->run();
  }
}

void PerfTestList::run_all_test() {
  global_pt_list_.run_perf_test();
}


void StopWatch::start() {
  gettimeofday(&this->tv_begin, nullptr);
}

void StopWatch::stop() {
  gettimeofday(&this->tv_end, nullptr);
}

float StopWatch::delta() const {
  struct timeval tv;
  timersub(&(this->tv_end), &(this->tv_begin), &tv);
  float usec = static_cast<float>(tv.tv_usec);
  float sec = static_cast<float>(tv.tv_sec);
  return sec + (usec / 1000000);
}
