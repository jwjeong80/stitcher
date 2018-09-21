#pragma once
#include <stdint.h>
#include <assert.h>
#include <intrin.h>

#pragma intrinsic(_BitScanReverse)

class CBitReader
{

public:
	inline int get_msb(unsigned int n) {
		unsigned long first_set_bit;
		assert(n != 0);
		_BitScanReverse(&first_set_bit, n);
		return first_set_bit;
	}

	inline int get_unsigned_bits(unsigned int num_values) {
		return num_values > 0 ? get_msb(num_values) + 1 : 0;
	}

	CBitReader() : m_bit_buffer(NULL), m_bit_buffer_end(NULL), m_bit_offset(0) {}
	CBitReader(const uint8_t *bit_buffer, const uint8_t *bit_buffer_end, uint32_t bit_offset)
		: m_bit_buffer(bit_buffer), m_bit_buffer_end(bit_buffer_end), m_bit_offset(bit_offset) {}

	size_t AomRbBytesRead();
	int AomRbReadBit();
	int AomRbReadLiteral(int bits);
	uint32_t AomRbReadUnsignedLiteral(int bits);
	int AomRbReadInvSignedLiteral(int bits);
	uint32_t AomRbReadUvlc();
	int AomRbReadUniform(int n);
	int Av1CheckTrailingBits();
	const int ByteAlignment();

	const uint8_t* AomRbReadBitBuffer() { return m_bit_buffer; }
	const uint8_t* AomRbReadBitBufferEnd() { return m_bit_buffer_end; }
	uint32_t AomRbReadBitOffset() { return m_bit_offset; }


private:

	const uint8_t *m_bit_buffer;
	const uint8_t *m_bit_buffer_end;
	uint32_t m_bit_offset;
};

