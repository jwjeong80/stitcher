#include <stdint.h>


static const size_t kMaximumLeb128Size = 8;
static const uint8_t kLeb128ByteMask = 0x7f;  // Binary: 01111111
static const uint64_t kMaximumLeb128Value = UINT32_MAX;

int aom_uleb_decode(const uint8_t *buffer, size_t available, uint64_t *value,
	size_t *length) {
	if (buffer && value) {
		*value = 0;
		for (size_t i = 0; i < kMaximumLeb128Size && i < available; ++i) {
			const uint8_t decoded_byte = *(buffer + i) & kLeb128ByteMask;
			*value |= ((uint64_t)decoded_byte) << (i * 7);
			if ((*(buffer + i) >> 7) == 0) {
				if (length) {
					*length = i + 1;
				}

				// Fail on values larger than 32-bits to ensure consistent behavior on
				// 32 and 64 bit targets: value is typically used to determine buffer
				// allocation size.
				if (*value > UINT32_MAX) return -1;

				return 0;
			}
		}
	}
	// If we get here, either the buffer/value pointers were invalid,
// or we ran over the available space
	return -1;
}

size_t aom_uleb_size_in_bytes(uint64_t value) {
	size_t size = 0;
	do {
		++size;
	} while ((value >>= 7) != 0);
	return size;
}

int aom_uleb_encode(uint64_t value, size_t available, uint8_t *coded_value, size_t *coded_size) {
	const size_t leb_size = aom_uleb_size_in_bytes(value);
	if (value > kMaximumLeb128Value || leb_size > kMaximumLeb128Size ||
		leb_size > available || !coded_value || !coded_size) {
		return -1;
	}

	for (size_t i = 0; i < leb_size; ++i) {
		uint8_t byte = value & 0x7f;
		value >>= 7;

		if (value != 0) byte |= 0x80;  // Signal that more bytes follow.

		*(coded_value + i) = byte;
	}

	*coded_size = leb_size;
	return 0;
}