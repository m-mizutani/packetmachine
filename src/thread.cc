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

#include "./thread.hpp"
#include "./packetmachine/exception.hpp"
#include <assert.h>

namespace pm {

Thread::Thread() {
}

Thread::~Thread() {
}

void Thread::start() {
  pthread_create(&this->th_, nullptr, Thread::run_thread, this);
}

void Thread::join() {
  pthread_join(this->th_, nullptr);
}

void Thread::join(const struct timespec& timeout) {
#ifdef __linux__
  static const long BILLION = 1e9;  // NOLINT, according to timespec.tv_nsec
  struct timespec tv;
  ::clock_gettime(CLOCK_REALTIME, &tv);
  tv.tv_sec += timeout.tv_sec;
  tv.tv_nsec += timeout.tv_nsec;
  if (tv.tv_nsec > BILLION) {
    tv.tv_sec += static_cast<time_t>(tv.tv_nsec / BILLION);
    tv.tv_nsec = tv.tv_nsec % BILLION;
  }
  int r = pthread_timedjoin_np(this->th_, nullptr, &tv);
  assert(r == 0 || r == ETIMEDOUT);
  
#else
  throw Exception::RunTimeError("join() timeout is supported in only LInux");
#endif
}


void Thread::stop() {
  pthread_cancel(this->th_);
  pthread_join(this->th_, nullptr);  
}

void* Thread::run_thread(void* obj) {
  Thread *th = static_cast<Thread*>(obj);
  th->thread_main();
  return nullptr;
}

}   // namespace pm
