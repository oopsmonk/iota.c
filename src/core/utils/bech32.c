// Copyright 2020 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0
// Based on reference implementation https://github.com/sipa/bech32/blob/master/ref/c/segwit_addr.c

/* Copyright (c) 2017 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "string.h"

#include "core/utils/bech32.h"

// A Bech32[2] string is at most 90 characters long and consists of: [HRP, "1", data]
#define BECH32_MAX_STRING_LEN 90

static uint32_t bech32_polymod_step(uint32_t pre) {
  uint8_t b = pre >> 25;
  return ((pre & 0x1FFFFFF) << 5) ^ (-((b >> 0) & 1) & 0x3b6a57b2UL) ^ (-((b >> 1) & 1) & 0x26508e6dUL) ^
         (-((b >> 2) & 1) & 0x1ea119faUL) ^ (-((b >> 3) & 1) & 0x3d4233ddUL) ^ (-((b >> 4) & 1) & 0x2a1462b3UL);
}

static const char *charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static const int8_t charset_rev[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, 10, 17,
    21, 20, 26, 30, 7,  5,  -1, -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,  23, -1, 18, 22, 31, 27,
    19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,
    23, -1, 18, 22, 31, 27, 19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1};

static int convert_bits(uint8_t *out, size_t *outlen, int outbits, const uint8_t *in, size_t inlen, int inbits,
                        int pad) {
  uint32_t val = 0;
  int bits = 0;
  uint32_t maxv = (((uint32_t)1) << outbits) - 1;
  while (inlen--) {
    val = (val << inbits) | *(in++);
    bits += inbits;
    while (bits >= outbits) {
      bits -= outbits;
      out[(*outlen)++] = (val >> bits) & maxv;
    }
  }
  if (pad) {
    if (bits) {
      out[(*outlen)++] = (val << (outbits - bits)) & maxv;
    }
  } else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
    return 0;
  }
  return 1;
}

int bech32_encode(char *output, const char *hrp, const uint8_t *data, size_t data_len) {
  uint32_t chk = 1;
  size_t i = 0;
  while (hrp[i] != 0) {
    int ch = hrp[i];
    if (ch < 33 || ch > 126) {
      return 0;
    }

    if (ch >= 'A' && ch <= 'Z') return 0;
    chk = bech32_polymod_step(chk) ^ (ch >> 5);
    ++i;
  }
  if (i + 7 + data_len > BECH32_MAX_STRING_LEN) return 0;
  chk = bech32_polymod_step(chk);
  while (*hrp != 0) {
    chk = bech32_polymod_step(chk) ^ (*hrp & 0x1f);
    *(output++) = *(hrp++);
  }
  *(output++) = '1';
  for (i = 0; i < data_len; ++i) {
    if (*data >> 5) return 0;
    chk = bech32_polymod_step(chk) ^ (*data);
    *(output++) = charset[*(data++)];
  }
  for (i = 0; i < 6; ++i) {
    chk = bech32_polymod_step(chk);
  }
  chk ^= 1;
  for (i = 0; i < 6; ++i) {
    *(output++) = charset[(chk >> ((5 - i) * 5)) & 0x1f];
  }
  *output = 0;
  return 1;
}

int bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input) {
  uint32_t chk = 1;
  size_t i;
  size_t input_len = strlen(input);
  size_t hrp_len;
  int have_lower = 0, have_upper = 0;
  if (input_len < 8 || input_len > BECH32_MAX_STRING_LEN) {
    return 0;
  }
  *data_len = 0;
  while (*data_len < input_len && input[(input_len - 1) - *data_len] != '1') {
    ++(*data_len);
  }
  if (1 + *data_len >= input_len || *data_len < 6) {
    return 0;
  }
  hrp_len = input_len - (1 + *data_len);
  *(data_len) -= 6;
  for (i = 0; i < hrp_len; ++i) {
    int ch = input[i];
    if (ch < 33 || ch > 126) {
      return 0;
    }
    if (ch >= 'a' && ch <= 'z') {
      have_lower = 1;
    } else if (ch >= 'A' && ch <= 'Z') {
      have_upper = 1;
      ch = (ch - 'A') + 'a';
    }
    hrp[i] = ch;
    chk = bech32_polymod_step(chk) ^ (ch >> 5);
  }
  hrp[i] = 0;
  chk = bech32_polymod_step(chk);
  for (i = 0; i < hrp_len; ++i) {
    chk = bech32_polymod_step(chk) ^ (input[i] & 0x1f);
  }
  ++i;
  while (i < input_len) {
    int v = (input[i] & 0x80) ? -1 : charset_rev[(int)input[i]];
    if (input[i] >= 'a' && input[i] <= 'z') have_lower = 1;
    if (input[i] >= 'A' && input[i] <= 'Z') have_upper = 1;
    if (v == -1) {
      return 0;
    }
    chk = bech32_polymod_step(chk) ^ v;
    if (i + 6 < input_len) {
      data[i - (1 + hrp_len)] = v;
    }
    ++i;
  }
  if (have_lower && have_upper) {
    return 0;
  }
  return chk == 1;
}

int iota_addr_bech32_encode(char *output, const char *hrp, const uint8_t *addr, size_t addr_len) {
  uint8_t data[64];
  size_t datalen = 0;
  convert_bits(data, &datalen, 5, addr, addr_len, 8, 1);
  return bech32_encode(output, hrp, data, datalen);
}

int iota_addr_bech32_decode(uint8_t *addr_data, size_t *addr_len, const char *hrp, const char *addr_str) {
  uint8_t data[64];
  char hrp_actual[84];
  size_t data_len = 0;

  if (!bech32_decode(hrp_actual, data, &data_len, addr_str)) {
    return 0;
  }

  if (data_len == 0 || data_len > 64) {
    return 0;
  }

  if (strncmp(hrp, hrp_actual, 84) != 0) {
    return 0;
  }

  // 0 is denoted as ED25519 address type
  if (data[0] != 0) {
    return 0;
  }

  *addr_len = 0;
  if (!convert_bits(addr_data, addr_len, 8, data, data_len, 5, 0)) {
    return 0;
  }
  return 1;
}