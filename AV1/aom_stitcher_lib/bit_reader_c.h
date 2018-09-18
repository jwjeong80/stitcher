#pragma once
#include <stdint.h>

class CBitReader
{

public:


	CBitReader() : m_bit_buffer(NULL), m_bit_buffer_end(NULL), m_bit_offset(0) {}
	CBitReader(const uint8_t *bit_buffer, const uint8_t *bit_buffer_end, uint32_t bit_offset)
		: m_bit_buffer(bit_buffer), m_bit_buffer_end(bit_buffer_end), m_bit_offset(bit_offset) {}

	size_t AomRbBytesRead();
	int AomRbReadBit();
	int AomRbReadLiteral(int bits);
	uint32_t AomRbReadUnsignedLiteral(int bits);
	int AomRbReadInvSignedLiteral(int bits);
	uint32_t AomRbReadUvlc();
	int Av1CheckTrailingBits();

	const uint8_t* AomRbReadBitBuffer() { return m_bit_buffer; }
	const uint8_t* AomRbReadBitBufferEnd() { return m_bit_buffer_end; }
	uint32_t AomRbReadBitOffset() { return m_bit_offset; }

private:

	const uint8_t *m_bit_buffer;
	const uint8_t *m_bit_buffer_end;
	uint32_t m_bit_offset;
};

