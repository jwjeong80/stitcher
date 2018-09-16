#include <stdint.h>

struct AomReadBitBuffer {
	const uint8_t *bit_buffer;
	const uint8_t *bit_buffer_end;
	uint32_t bit_offset;
};

class CBitReader
{
public:
	struct AomReadBitBuffer *InitReadBitBuffer(struct AomReadBitBuffer *rb, const uint8_t *data, const uint8_t *data_end);

	size_t AomRbBytesRead(const struct AomReadBitBuffer *rb);

	int AomRbReadBit(struct AomReadBitBuffer *rb);

	int AomRbReadLiteral(struct AomReadBitBuffer *rb, int bits);

	uint32_t AomRbReadUnsignedLiteral(struct AomReadBitBuffer *rb, int bits);

	int AomRbReadInvSignedLiteral(struct AomReadBitBuffer *rb, int bits);

private:


};

