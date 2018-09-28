#pragma once
#include <stdint.h>
#include "obu_header.h"

	// Writes only the OBU Sequence Header payload, and returns the size of the
	// payload written to 'dst'. This function does not write the OBU header, the
	// optional extension, or the OBU size to 'dst'.
uint32_t write_sequence_header_obu(uint8_t *const dst);

	// Writes the OBU header byte, and the OBU header extension byte when
	// 'obu_extension' is non-zero. Returns number of bytes written to 'dst'.
uint32_t write_obu_header(OBU_TYPE obu_type, int obu_extension,	uint8_t *const dst);

int write_uleb_obu_size(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *dest);

int av1_pack_bitstream(uint8_t *dest, size_t *size);

	//static INLINE int av1_preserve_existing_gf(AV1_COMP *cpi) {
	//	// Do not swap gf and arf indices for internal overlay frames
	//	return cpi->rc.is_src_frame_alt_ref && !cpi->rc.is_src_frame_ext_arf;
	//}
