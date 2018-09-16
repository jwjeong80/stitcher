#include <assert.h>
#include "bit_reader.h"

struct AomReadBitBuffer* CBitReader::InitReadBitBuffer(
	struct AomReadBitBuffer *rb, const uint8_t *data,
	const uint8_t *data_end) {
	rb->bit_offset = 0;
	rb->bit_buffer = data;
	rb->bit_buffer_end = data_end;
	return rb;
}

size_t CBitReader::AomRbBytesRead(const struct AomReadBitBuffer *rb) {
	return (rb->bit_offset + 7) >> 3;
}

int CBitReader::AomRbReadBit(struct AomReadBitBuffer *rb) {
	const uint32_t off = rb->bit_offset;
	const uint32_t p = off >> 3;
	const int q = 7 - (int)(off & 0x7);
	if (rb->bit_buffer + p < rb->bit_buffer_end) {
		const int bit = (rb->bit_buffer[p] >> q) & 1;
		rb->bit_offset = off + 1;
		return bit;
	}
	else {
		return 0;
	}
}

int CBitReader::AomRbReadLiteral(struct AomReadBitBuffer *rb, int bits) {
	assert(bits <= 31);
	int value = 0, bit;
	for (bit = bits - 1; bit >= 0; bit--) value |= AomRbReadBit(rb) << bit;
	return value;
}

uint32_t CBitReader::AomRbReadUnsignedLiteral(struct AomReadBitBuffer *rb,
	int bits) {
	assert(bits <= 32);
	uint32_t value = 0;
	int bit;
	for (bit = bits - 1; bit >= 0; bit--)
		value |= (uint32_t)AomRbReadBit(rb) << bit;
	return value;
}

int CBitReader::AomRbReadInvSignedLiteral(struct AomReadBitBuffer *rb, int bits) {
	const int nbits = sizeof(unsigned) * 8 - bits - 1;
	const unsigned value = (unsigned)AomRbReadLiteral(rb, bits + 1) << nbits;
	return ((int)value) >> nbits;
}

