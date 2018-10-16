/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "bit_writer.h"

#include <stdio.h>
int CBitWriter::aom_wb_is_byte_aligned() {
  return (m_wb_bit_offset % CHAR_BIT == 0);
}

uint32_t CBitWriter::aom_wb_bytes_written() {
  return m_wb_bit_offset / CHAR_BIT + (m_wb_bit_offset % CHAR_BIT > 0);
}

void CBitWriter::aom_wb_write_bit(int bit) {
  const int off = m_wb_bit_offset;
  const int p = off / CHAR_BIT;
  const int q = CHAR_BIT - 1 - off % CHAR_BIT;
  if (q == CHAR_BIT - 1) {
    // Zero next char and write bit
	  m_wb_bit_buffer[p] = bit << q;
  } else {
    m_wb_bit_buffer[p] &= ~(1 << q);
    m_wb_bit_buffer[p] |= bit << q;
  }
  m_wb_bit_offset = off + 1;

#ifdef 0
  FILE *fp = fopen("wb_debug.txt", "a");
  fprintf(fp, "%d\t%d\n", m_wb_bit_offset, bit);
  fclose(fp);
#endif
}

void CBitWriter::aom_wb_overwrite_bit(int bit) {
  // Do not zero bytes but overwrite exisiting values
  const int off = (int)m_wb_bit_offset;
  const int p = off / CHAR_BIT;
  const int q = CHAR_BIT - 1 - off % CHAR_BIT;
  m_wb_bit_buffer[p] &= ~(1 << q);
  m_wb_bit_buffer[p] |= bit << q;
  m_wb_bit_offset = off + 1;
}

void CBitWriter::aom_wb_write_literal(int data, int bits) {
  assert(bits <= 31);
  int bit;
  for (bit = bits - 1; bit >= 0; bit--) aom_wb_write_bit((data >> bit) & 1);
}

void CBitWriter::aom_wb_write_unsigned_literal(uint32_t data, int bits) {
  assert(bits <= 32);
  int bit;
  for (bit = bits - 1; bit >= 0; bit--) aom_wb_write_bit((data >> bit) & 1);
}

void CBitWriter::aom_wb_overwrite_literal(int data, int bits) {
  int bit;
  for (bit = bits - 1; bit >= 0; bit--)
    aom_wb_overwrite_bit((data >> bit) & 1);
}

void CBitWriter::aom_wb_write_inv_signed_literal(int data, int bits) {
  aom_wb_write_literal(data, bits + 1);
}

void CBitWriter::aom_wb_write_uvlc(uint32_t v) {
  int64_t shift_val = ++v;
  int leading_zeroes = 1;

  assert(shift_val > 0);

  while (shift_val >>= 1) leading_zeroes += 2;

  aom_wb_write_literal(0, leading_zeroes >> 1);
  aom_wb_write_unsigned_literal(v, (leading_zeroes + 1) >> 1);
}

void CBitWriter::add_trailing_bits() {
	if (aom_wb_is_byte_aligned()) {
		aom_wb_write_literal(0x80, 8);
	}
	else {
		// assumes that the other bits are already 0s
		aom_wb_write_bit(1);
	}
}


// Same function as write_uniform but writing to uncompresses header wb
void CBitWriter::wb_write_uniform(int n, int v) {
	const int l = get_unsigned_bits(n);
	const int m = (1 << l) - n;
	if (l == 0) return;
	if (v < m) {
		aom_wb_write_literal(v, l - 1);
	}
	else {
		aom_wb_write_literal(m + ((v - m) >> 1), l - 1);
		aom_wb_write_literal((v - m) & 1, 1);
	}
}