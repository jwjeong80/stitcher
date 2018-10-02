#pragma once
#include <stdint.h>
#include <assert.h>
#include <intrin.h>
class CBitWriter
{
public:

	CBitWriter() : m_wb_bit_buffer(NULL), m_wb_bit_offset(0) {}
	CBitWriter(uint8_t *bit_buffer, uint32_t bit_offset): m_wb_bit_buffer(bit_buffer), m_wb_bit_offset(bit_offset) {}

	inline int get_msb(unsigned int n) {
		unsigned long first_set_bit;
		assert(n != 0);
		_BitScanReverse(&first_set_bit, n);
		return first_set_bit;
	}

	inline int get_unsigned_bits(unsigned int num_values) {
		return num_values > 0 ? get_msb(num_values) + 1 : 0;
	}

	void add_trailing_bits();
	
	int aom_wb_is_byte_aligned();

	uint32_t aom_wb_bytes_written();

	void aom_wb_write_bit(int bit);

	void aom_wb_overwrite_bit(int bit);

	void aom_wb_write_literal(int data, int bits);

	void aom_wb_write_unsigned_literal(uint32_t data, int bits);

	void aom_wb_overwrite_literal(int data,	int bits);

	void aom_wb_write_inv_signed_literal(int data, int bits);

	void aom_wb_write_uvlc(uint32_t v);

	void wb_write_uniform(int n, int v);

private:
	uint8_t *m_wb_bit_buffer;
	uint32_t m_wb_bit_offset;
}; 



