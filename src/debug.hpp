/*-
 * Copyright (c) 2013 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
 * All rights reserved.
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

#ifndef SRC_DEBUG_H__
#define SRC_DEBUG_H__

#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <libgen.h>

#define debug(X, ...)                                       \
  do {                                                      \
    struct tm td;                                           \
    struct timeval tv;                                      \
    struct timezone tz;                                     \
    FILE * out = stdout;                                    \
    char *fpath = const_cast<char*>(__FILE__);              \
    if (X) {                                                \
      gettimeofday(&tv, &tz);                               \
      localtime_r(&tv.tv_sec, &td);                         \
      fprintf(out, "(DEBUG %02d:%02d:%02d.%03d %s:%d %s) ", \
              td.tm_hour, td.tm_min, td.tm_sec,             \
              static_cast <int> (tv.tv_usec / 1000),        \
              basename(fpath), __LINE__, __FUNCTION__);     \
      fprintf(out, __VA_ARGS__);                            \
      fprintf(out, "\n");                                   \
      fflush(out);                                          \
    }                                                       \
  } while (0);

#endif  // SRC_DEBUG_H__
