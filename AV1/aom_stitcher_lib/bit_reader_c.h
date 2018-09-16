#pragma once
#include <stdint.h>

class CBitReader
{

public:

	const uint8_t *m_bit_buffer;
	const uint8_t *m_bit_buffer_end;
	uint32_t m_bit_offset;

	CBitReader() : m_bit_buffer(NULL), m_bit_buffer_end(NULL), m_bit_offset(0) {}
	CBitReader(const uint8_t *bit_buffer, const uint8_t *bit_buffer_end, uint32_t bit_offset)
		: m_bit_buffer(bit_buffer), m_bit_buffer_end(bit_buffer_end), m_bit_offset(bit_offset) {}

	size_t AomRbBytesRead();

	// *InitReadBitBuffer(struct AomReadBitBuffer *rb, const uint8_t *data, const uint8_t *data_end);

	//size_t AomRbBytesRead(const struct AomReadBitBuffer *rb);

	int AomRbReadBit();

	int AomRbReadLiteral(int bits);

	uint32_t AomRbReadUnsignedLiteral(int bits);

	int AomRbReadInvSignedLiteral(int bits);


	uint32_t AomRbReadUvlc();

private:
};

