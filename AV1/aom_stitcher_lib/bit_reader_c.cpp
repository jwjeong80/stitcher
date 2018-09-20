#include "bit_reader_c.h"
#include <assert.h>

size_t CBitReader::AomRbBytesRead() {
	return (m_bit_offset + 7) >> 3;
}

int CBitReader::AomRbReadBit() {
	const uint32_t off = m_bit_offset;
	const uint32_t p = off >> 3;
	const int q = 7 - (int)(off & 0x7);
	if (m_bit_buffer + p < m_bit_buffer_end) {
		const int bit = (m_bit_buffer[p] >> q) & 1;
		m_bit_offset = off + 1;
		return bit;
	}
	else {
		return 0;
	}
}


int CBitReader::AomRbReadLiteral(int bits) {
	assert(bits <= 31);
	int value = 0, bit;
	for (bit = bits - 1; bit >= 0; bit--) value |= AomRbReadBit() << bit;
	return value;
}

uint32_t CBitReader::AomRbReadUnsignedLiteral(int bits) {
	assert(bits <= 32);
	uint32_t value = 0;
	int bit;
	for (bit = bits - 1; bit >= 0; bit--)
		value |= (uint32_t)AomRbReadBit() << bit;
	return value;
}

int CBitReader::AomRbReadInvSignedLiteral(int bits) {
	const int nbits = sizeof(unsigned) * 8 - bits - 1;
	const unsigned value = (unsigned)AomRbReadLiteral(bits + 1) << nbits;
	return ((int)value) >> nbits;
}

uint32_t CBitReader::AomRbReadUvlc() {
	int leading_zeros = 0;
	while (!AomRbReadBit()) ++leading_zeros;
	// Maximum 32 bits.
	if (leading_zeros >= 32) return UINT32_MAX;
	const uint32_t base = (1u << leading_zeros) - 1;
	const uint32_t value = AomRbReadLiteral(leading_zeros);
	return base + value;
}


int CBitReader::Av1CheckTrailingBits() {
	// bit_offset is set to 0 (mod 8) when the reader is already byte aligned
	int bits_before_alignment = 8 - m_bit_offset % 8;
	int trailing = AomRbReadLiteral(bits_before_alignment);
	if (trailing != (1 << (bits_before_alignment - 1))) {
		return -1;
	}
	return 0;
}

int CBitReader::AomRbReadUniform(int n)
{
	const int l = get_unsigned_bits(n);
	const int m = (1 << l) - n;
	const int v = AomRbReadLiteral(l - 1);
	assert(l != 0);
	if (v < m)
		return v;
	else
		return (v << 1) - m + AomRbReadBit();
}