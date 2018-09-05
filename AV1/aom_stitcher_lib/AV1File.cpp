// HEVCFile.cpp
// Created by yonghwan@keti.re.kr, 2011.07.01


#if _MSC_VER
#pragma warning(disable:4996) //#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stdafx.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>  // for memcpy()
#include "AV1File.h"
#include "aom/aom_integer.h"
//#include "av1/decoder/obu.h"

#include <assert.h>
#include <io.h>
#include <fcntl.h>

#define OBU_BUFFER_SIZE (500 * 1024)

#define OBU_HEADER_SIZE 1
#define OBU_EXTENSION_SIZE 1
#define OBU_MAX_LENGTH_FIELD_SIZE 8
#define OBU_DETECTION_SIZE \
  (OBU_HEADER_SIZE + OBU_EXTENSION_SIZE + 3 * OBU_MAX_LENGTH_FIELD_SIZE)

FILE *set_binary_mode(FILE *stream) {
	(void)stream;
#if defined(_WIN32) || defined(__OS2__)
	_setmode(_fileno(stream), _O_BINARY);
#endif
	return stream;
}


// Returns 1 when OBU type is valid, and 0 otherwise.
static int valid_obu_type(int obu_type) {
	int valid_type = 0;
	switch (obu_type) {
	case OBU_SEQUENCE_HEADER:
	case OBU_TEMPORAL_DELIMITER:
	case OBU_FRAME_HEADER:
	case OBU_TILE_GROUP:
	case OBU_METADATA:
	case OBU_FRAME:
	case OBU_REDUNDANT_FRAME_HEADER:
	case OBU_TILE_LIST:
	case OBU_PADDING: valid_type = 1; break;
	default: break;
	}
	return valid_type;
}

size_t CAV1File::AomRbBytesRead(const struct aom_read_bit_buffer *rb) {
	return (rb->bit_offset + 7) >> 3;
}

int CAV1File::AomRbReadBit(struct aom_read_bit_buffer *rb) {
	const uint32_t off = rb->bit_offset;
	const uint32_t p = off >> 3;
	const int q = 7 - (int)(off & 0x7);
	if (rb->bit_buffer + p < rb->bit_buffer_end) {
		const int bit = (rb->bit_buffer[p] >> q) & 1;
		rb->bit_offset = off + 1;
		return bit;
	}
	else {
		if (rb->error_handler) rb->error_handler(rb->error_handler_data);
		return 0;
	}
}

int CAV1File::AomRbReadLiteral(struct aom_read_bit_buffer *rb, int bits) {
	assert(bits <= 31);
	int value = 0, bit;
	for (bit = bits - 1; bit >= 0; bit--) value |= AomRbReadBit(rb) << bit;
	return value;
}

CAV1File::CAV1File(void)
{
	//m_OBUCtx.avx_ctx = NULL;
	memset(&m_OBUCtx.avx_ctx, 0, sizeof(AvxInputContext));
	m_OBUCtx.buffer = NULL;
	m_OBUCtx.buffer_capacity = 0;
	m_OBUCtx.bytes_buffered = 0;
	m_OBUCtx.is_annexb = 0;

	m_AomInputCtx.file = NULL;
	m_AomInputCtx.filename = NULL;
	m_AomInputCtx.height = 0;
	m_AomInputCtx.width = 0;

}

CAV1File::~CAV1File(void)
{
}

bool CAV1File::OpenOBU(const char* rcFilename)
{
	m_OBUCtx.avx_ctx.filename = rcFilename;
	if (!m_OBUCtx.avx_ctx.filename)
		return false;

	m_OBUCtx.avx_ctx.file = strcmp(m_OBUCtx.avx_ctx.filename, "-") ? fopen(m_OBUCtx.avx_ctx.filename, "rb") : set_binary_mode(stdin);

	if (!m_OBUCtx.avx_ctx.file)
		return false;

	return true;
}

#define		INIT_READ	(4*1024)	// SEI, SPS, PPS, Prefix_NALU는 모두 여기에 걸림.
#define		NEXT_READ	(8*1024)


#if 0
inline bool 	IsSuffixNAL(int nalUnitType)
{
	if (nalUnitType == 38 || nalUnitType == 40)	//Filler, Suffix_SEI, 
		return true;
	else if ((nalUnitType >= 45 && nalUnitType <= 47) || (nalUnitType >= 56 && nalUnitType <= 63))
		return true;
	else
		return false;
}
#endif

#define SET_BUFFER()		\
{							\
	pAU->pNALU[i]		= pOutBuf;		\
	pAU->uiNALUSize[i]	= uiOutBufSize;	\
	pAU->uiNumOfNALU++;					\
}
#define SHVC_EXT	1

// 7.4.2.4.4 Order of NAL units and coded pictures and their association to access units
// An access unit consists of one coded picture and zero or more non-VCL NAL units


// Reads unsigned LEB128 integer and returns 0 upon successful read and decode.
// Stores raw bytes in 'value_buffer', length of the number in 'value_length',
// and decoded value in 'value'.
//int CAV1File::obudec_read_leb128(uint8_t *value_buffer,
//	size_t *value_length, uint64_t *value) {
//	//if (!m_File || !value_buffer || !value_length || !value) return -1;
//	if (!value_buffer || !value_length || !value) return -1;
//
//	size_t len;
//	for (len = 0; len < OBU_MAX_LENGTH_FIELD_SIZE; ++len) {
//		//const size_t num_read = fread(&value_buffer[len], 1, 1, f);
//		const size_t num_read = 1;
//		if (num_read == 0) {
//			//if (len == 0 && feof(f)) {
//			//	*value_length = 0;
//			//	return 0;
//			//}
//			// Ran out of data before completing read of value.
//			return -1;
//		}
//		if ((value_buffer[len] >> 7) == 0) {
//			++len;
//			*value_length = len;
//			break;
//		}
//	}
//
//	return aom_uleb_decode(value_buffer, len, value, NULL);
//}
int CAV1File::obudec_read_leb128(FILE *f, uint8_t *value_buffer,
	size_t *value_length, uint64_t *value) {
	if (!f || !value_buffer || !value_length || !value) return -1;
	size_t len;
	for (len = 0; len < OBU_MAX_LENGTH_FIELD_SIZE; ++len) {
		const size_t num_read = fread(&value_buffer[len], 1, 1, f);
		if (num_read == 0) {
			if (len == 0 && feof(f)) {
				*value_length = 0;
				return 0;
			}
			// Ran out of data before completing read of value.
			return -1;
		}
		if ((value_buffer[len] >> 7) == 0) {
			++len;
			*value_length = len;
			break;
		}
	}

	return aom_uleb_decode(value_buffer, len, value, NULL);
}

// Reads OBU payload from 'f' and returns 0 for success when all payload bytes
// are read from the file. Payload data is written to 'obu_data', and actual
// bytes read added to 'bytes_read'.
int CAV1File::obudec_read_obu_payload(FILE *f, size_t payload_length,
	uint8_t *obu_data, size_t *bytes_read) {
	if (!f || payload_length == 0 || !obu_data || !bytes_read) return -1;

	if (fread(obu_data, 1, payload_length, f) != payload_length) {
		fprintf(stderr, "obudec: Failure reading OBU payload.\n");
		return -1;
	}

	*bytes_read += payload_length;
	return 0;
}

// Reads OBU header from 'f'. The 'buffer_capacity' passed in must be large
// enough to store an OBU header with extension (2 bytes). Raw OBU data is
// written to 'obu_data', parsed OBU header values are written to 'obu_header',
// and total bytes read from file are written to 'bytes_read'. Returns 0 for
// success, and non-zero on failure. When end of file is reached, the return
// value is 0 and the 'bytes_read' value is set to 0.
int CAV1File::obudec_read_obu_header(FILE *f, size_t buffer_capacity,
	int is_annexb, uint8_t *obu_data,
	ObuHeader *obu_header, size_t *bytes_read) {
	if (!f || buffer_capacity < (OBU_HEADER_SIZE + OBU_EXTENSION_SIZE) ||
		!obu_data || !obu_header || !bytes_read) {
		return -1;
	}
	*bytes_read = fread(obu_data, 1, 1, f);

	if (feof(f) && *bytes_read == 0) {
		return 0;
	}
	else if (*bytes_read != 1) {
		fprintf(stderr, "obudec: Failure reading OBU header.\n");
		return -1;
	}

	const int has_extension = (obu_data[0] >> 2) & 0x1;
	if (has_extension) {
		if (fread(&obu_data[1], 1, 1, f) != 1) {
			fprintf(stderr, "obudec: Failure reading OBU extension.");
			return -1;
		}
		++*bytes_read;
	}

	size_t obu_bytes_parsed = 0;
	const aom_codec_err_t parse_result = aom_read_obu_header(
		obu_data, *bytes_read, &obu_bytes_parsed, obu_header, is_annexb);
	if (parse_result != AOM_CODEC_OK || *bytes_read != obu_bytes_parsed) {
		fprintf(stderr, "obudec: Error parsing OBU header.\n");
		return -1;
	}

	return 0;
}

int CAV1File::obudec_read_obu_header_and_size(FILE *f, size_t buffer_capacity,
	int is_annexb, uint8_t *buffer,
	size_t *bytes_read,
	size_t *payload_length,
	ObuHeader *obu_header) {
	const size_t kMinimumBufferSize =
		(OBU_HEADER_SIZE + OBU_EXTENSION_SIZE + OBU_MAX_LENGTH_FIELD_SIZE);
	if (!f || !buffer || !bytes_read || !payload_length || !obu_header ||
		buffer_capacity < kMinimumBufferSize) {
		return -1;
	}

	size_t leb128_length = 0;
	uint64_t obu_size = 0;
	if (is_annexb) {
		if (obudec_read_leb128(f, &buffer[0], &leb128_length, &obu_size) != 0) {
			fprintf(stderr, "obudec: Failure reading OBU size length.\n");
			return -1;
		}
		else if (leb128_length == 0) {
			*payload_length = 0;
			return 0;
		}
		if (obu_size > UINT32_MAX) {
			fprintf(stderr, "obudec: OBU payload length too large.\n");
			return -1;
		}
	}

	size_t header_size = 0;
	if (obudec_read_obu_header(f, buffer_capacity - leb128_length, is_annexb,
		buffer + leb128_length, obu_header,
		&header_size) != 0) {
		return -1;
	}
	else if (header_size == 0) {
		*payload_length = 0;
		return 0;
	}

	if (is_annexb) {
		if (obu_size < header_size) {
			fprintf(stderr, "obudec: OBU size is too small.\n");
			return -1;
		}
		*payload_length = (size_t)obu_size - header_size;
	}
	else {
		uint64_t u64_payload_length = 0;
		if (obudec_read_leb128(f, &buffer[header_size], &leb128_length,
			&u64_payload_length) != 0) {
			fprintf(stderr, "obudec: Failure reading OBU payload length.\n");
			return -1;
		}
		if (u64_payload_length > UINT32_MAX) {
			fprintf(stderr, "obudec: OBU payload length too large.\n");
			return -1;
		}

		*payload_length = (size_t)u64_payload_length;
	}

	*bytes_read = leb128_length + header_size;
	return 0;
}


int CAV1File::obudec_read_one_obu(FILE *f, uint8_t **obu_buffer,
	size_t obu_bytes_buffered,
	size_t *obu_buffer_capacity, size_t *obu_length,
	ObuHeader *obu_header, int is_annexb) {
	size_t available_buffer_capacity = *obu_buffer_capacity - obu_bytes_buffered;

	if (!(*obu_buffer)) return -1;

	size_t bytes_read = 0;
	size_t obu_payload_length = 0;
	const int status = obudec_read_obu_header_and_size(
		f, available_buffer_capacity, is_annexb, *obu_buffer + obu_bytes_buffered,
		&bytes_read, &obu_payload_length, obu_header);
	if (status < 0) return status;

	if (obu_payload_length > SIZE_MAX - bytes_read) return -1;

	if (obu_payload_length > 256 * 1024 * 1024) {
		fprintf(stderr, "obudec: Read invalid OBU size (%u)\n",
			(unsigned int)obu_payload_length);
		*obu_length = bytes_read + obu_payload_length;
		return -1;
	}

	if (bytes_read + obu_payload_length > available_buffer_capacity) {
		// TODO(tomfinegan): Add overflow check.
		const size_t new_capacity =
			obu_bytes_buffered + bytes_read + 2 * obu_payload_length;

#if defined AOM_MAX_ALLOCABLE_MEMORY
		if (new_capacity > AOM_MAX_ALLOCABLE_MEMORY) {
			fprintf(stderr, "obudec: OBU size exceeds max alloc size.\n");
			return -1;
		}
#endif

		uint8_t *new_buffer = (uint8_t *)realloc(*obu_buffer, new_capacity);

		if (new_buffer) {
			*obu_buffer = new_buffer;
			*obu_buffer_capacity = new_capacity;
		}
		else {
			fprintf(stderr, "obudec: Failed to allocate compressed data buffer\n");
			*obu_length = bytes_read + obu_payload_length;
			return -1;
		}
	}

	if (obu_payload_length > 0 &&
		obudec_read_obu_payload(f, obu_payload_length,
			*obu_buffer + obu_bytes_buffered + bytes_read,
			&bytes_read) != 0) {
		return -1;
	}

	*obu_length = bytes_read;
	return 0;
}


// Parses OBU header and stores values in 'header'.
 aom_codec_err_t CAV1File::read_obu_header(struct aom_read_bit_buffer *rb,
	int is_annexb, ObuHeader *header) {
	if (!rb || !header) return AOM_CODEC_INVALID_PARAM;

	const ptrdiff_t bit_buffer_byte_length = rb->bit_buffer_end - rb->bit_buffer;
	if (bit_buffer_byte_length < 1) return AOM_CODEC_CORRUPT_FRAME;

	header->size = 1;

	if (AomRbReadBit(rb) != 0) {
		// Forbidden bit. Must not be set.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	header->type = (OBU_TYPE)AomRbReadLiteral(rb, 4);

	if (!valid_obu_type(header->type)) return AOM_CODEC_CORRUPT_FRAME;

	header->has_extension = AomRbReadBit(rb);
	header->has_size_field = AomRbReadBit(rb);

	if (!header->has_size_field && !is_annexb) {
		// section 5 obu streams must have obu_size field set.
		return AOM_CODEC_UNSUP_BITSTREAM;
	}

	if (AomRbReadBit(rb) != 0) {
		// obu_reserved_1bit must be set to 0.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	if (header->has_extension) {
		if (bit_buffer_byte_length == 1) return AOM_CODEC_CORRUPT_FRAME;

		header->size += 1;
		header->temporal_layer_id = AomRbReadLiteral(rb, 3);
		header->spatial_layer_id = AomRbReadLiteral(rb, 2);
		if (AomRbReadLiteral(rb, 3) != 0) {
			// extension_header_reserved_3bits must be set to 0.
			return AOM_CODEC_CORRUPT_FRAME;
		}
	}

	return AOM_CODEC_OK;
}

aom_codec_err_t CAV1File::aom_read_obu_header(uint8_t *buffer, size_t buffer_length,
	size_t *consumed, ObuHeader *header,
	int is_annexb) {
	if (buffer_length < 1 || !consumed || !header) return AOM_CODEC_INVALID_PARAM;

	// TODO(tomfinegan): Set the error handler here and throughout this file, and
	// confirm parsing work done via aom_read_bit_buffer is successful.
	struct aom_read_bit_buffer rb = { buffer, buffer + buffer_length, 0, NULL,
		NULL };
	aom_codec_err_t parse_result = read_obu_header(&rb, is_annexb, header);
	if (parse_result == AOM_CODEC_OK) *consumed = header->size;
	return parse_result;
}

int CAV1File::obudec_read_temporal_unit(	uint8_t **buffer, size_t *bytes_read, size_t *buffer_size) {
	struct ObuDecInputContext *obu_ctx = &m_OBUCtx;

	FILE *f = obu_ctx->avx_ctx.file;
	if (!f) return -1;

	*buffer_size = 0;
	*bytes_read = 0;

	if (feof(f)) {
		return 1;
	}

	size_t tu_size;
	size_t obu_size = 0;
	size_t length_of_temporal_unit_size = 0;
	uint8_t tuheader[OBU_MAX_LENGTH_FIELD_SIZE] = { 0 };

	if (obu_ctx->is_annexb) {
		uint64_t size = 0;

		if (obu_ctx->bytes_buffered == 0) {
			if (obudec_read_leb128(f, &tuheader[0], &length_of_temporal_unit_size,
				&size) != 0) {
				fprintf(stderr, "obudec: Failure reading temporal unit header\n");
				return -1;
			}
			if (size == 0 && feof(f)) {
				return 1;
			}
		}
		else {
			// temporal unit size was already stored in buffer
			if (aom_uleb_decode(obu_ctx->buffer, obu_ctx->bytes_buffered, &size,
				&length_of_temporal_unit_size) != 0) {
				fprintf(stderr, "obudec: Failure reading temporal unit header\n");
				return -1;
			}
		}

		if (size > UINT32_MAX || size + length_of_temporal_unit_size > UINT32_MAX) {
			fprintf(stderr, "obudec: TU too large.\n");
			return -1;
		}

		size += length_of_temporal_unit_size;
		tu_size = (size_t)size;
	}
	else {
		while (1) {
			ObuHeader obu_header;
			memset(&obu_header, 0, sizeof(obu_header));

			if (obudec_read_one_obu(f, &obu_ctx->buffer, obu_ctx->bytes_buffered,
				&obu_ctx->buffer_capacity, &obu_size, &obu_header,
				0) != 0) {
				fprintf(stderr, "obudec: read_one_obu failed in TU loop\n");
				return -1;
			}

			if (obu_header.type == OBU_TEMPORAL_DELIMITER || obu_size == 0) {
				tu_size = obu_ctx->bytes_buffered;
				break;
			}
			else {
				obu_ctx->bytes_buffered += obu_size;
				//uiNumOfOBU = 
			}
		}
	}

#if defined AOM_MAX_ALLOCABLE_MEMORY
	if (tu_size > AOM_MAX_ALLOCABLE_MEMORY) {
		fprintf(stderr, "obudec: Temporal Unit size exceeds max alloc size.\n");
		return -1;
	}
#endif
	uint8_t *new_buffer = (uint8_t *)realloc(*buffer, tu_size);
	if (!new_buffer) {
		free(*buffer);
		fprintf(stderr, "obudec: Out of memory.\n");
		return -1;
	}
	*buffer = new_buffer;
	*bytes_read = tu_size;
	*buffer_size = tu_size;

	if (!obu_ctx->is_annexb) {
		memcpy(*buffer, obu_ctx->buffer, tu_size);

		// At this point, (obu_ctx->buffer + obu_ctx->bytes_buffered + obu_size)
		// points to the end of the buffer.
		memmove(obu_ctx->buffer, obu_ctx->buffer + obu_ctx->bytes_buffered,
			obu_size);
		obu_ctx->bytes_buffered = obu_size;
	}
	else {
		if (!feof(f)) {
			size_t data_size;
			size_t offset;
			if (!obu_ctx->bytes_buffered) {
				data_size = tu_size - length_of_temporal_unit_size;
				memcpy(*buffer, &tuheader[0], length_of_temporal_unit_size);
				offset = length_of_temporal_unit_size;
			}
			else {
				memcpy(*buffer, obu_ctx->buffer, obu_ctx->bytes_buffered);
				offset = obu_ctx->bytes_buffered;
				data_size = tu_size - obu_ctx->bytes_buffered;
				obu_ctx->bytes_buffered = 0;
			}

			if (fread(*buffer + offset, 1, data_size, f) != data_size) {
				fprintf(stderr, "obudec: Failed to read full temporal unit\n");
				return -1;
			}
		}
	}
	return 0;
}


//int CAV1File::file_is_obu(struct ObuDecInputContext *obu_ctx) {
int CAV1File::file_is_obu(void) {
	struct ObuDecInputContext *obu_ctx = &m_OBUCtx;
	if (!obu_ctx || !(&obu_ctx->avx_ctx)) return 0;

	struct AvxInputContext *avx_ctx = &obu_ctx->avx_ctx;
	uint8_t detect_buf[OBU_DETECTION_SIZE] = { 0 };
	const int is_annexb = obu_ctx->is_annexb;
	FILE *f = avx_ctx->file;
	size_t payload_length = 0;
	ObuHeader obu_header;
	memset(&obu_header, 0, sizeof(obu_header));
	size_t length_of_unit_size = 0;
	size_t annexb_header_length = 0;
	uint64_t unit_size = 0;

	if (is_annexb) {
		// read the size of first temporal unit
		if (obudec_read_leb128(f, &detect_buf[0], &length_of_unit_size,
			&unit_size) != 0) {
			fprintf(stderr, "obudec: Failure reading temporal unit header\n");
			return 0;
		}

		// read the size of first frame unit
		if (obudec_read_leb128(f, &detect_buf[length_of_unit_size],
			&annexb_header_length, &unit_size) != 0) {
			fprintf(stderr, "obudec: Failure reading frame unit header\n");
			return 0;
		}
		annexb_header_length += length_of_unit_size;
	}

	size_t bytes_read = 0;
	if (obudec_read_obu_header_and_size(
		f, OBU_DETECTION_SIZE - annexb_header_length, is_annexb,
		&detect_buf[annexb_header_length], &bytes_read, &payload_length,
		&obu_header) != 0) {
		fprintf(stderr, "obudec: Failure reading first OBU.\n");
		rewind(f);
		return 0;
	}

	if (is_annexb) {
		bytes_read += annexb_header_length;
	}

	if (obu_header.type != OBU_TEMPORAL_DELIMITER &&
		obu_header.type != OBU_SEQUENCE_HEADER) {
		return 0;
	}

	if (obu_header.has_size_field) {
		if (obu_header.type == OBU_TEMPORAL_DELIMITER && payload_length != 0) {
			fprintf(
				stderr,
				"obudec: Invalid OBU_TEMPORAL_DELIMITER payload length (non-zero).");
			rewind(f);
			return 0;
		}
	}
	else if (!is_annexb) {
		fprintf(stderr, "obudec: OBU size fields required, cannot decode input.\n");
		rewind(f);
		return 0;
	}

	// Appears that input is valid Section 5 AV1 stream.
	obu_ctx->buffer = (uint8_t *)malloc(OBU_BUFFER_SIZE);
	if (!obu_ctx->buffer) {
		fprintf(stderr, "Out of memory.\n");
		rewind(f);
		return 0;
	}
	obu_ctx->buffer_capacity = OBU_BUFFER_SIZE;

	memcpy(obu_ctx->buffer, &detect_buf[0], bytes_read);
	obu_ctx->bytes_buffered = bytes_read;
	// If the first OBU is a SEQUENCE_HEADER, then it will have a payload.
	// We need to read this in so that our buffer only contains complete OBUs.
	if (payload_length > 0) {
		if (payload_length > (obu_ctx->buffer_capacity - bytes_read)) {
			fprintf(stderr, "obudec: First OBU's payload is too large\n");
			rewind(f);
			return 0;
		}

		size_t payload_bytes = 0;
		const int status = obudec_read_obu_payload(
			f, payload_length, &obu_ctx->buffer[bytes_read], &payload_bytes);
		if (status < 0) {
			rewind(f);
			return 0;
		}
		obu_ctx->bytes_buffered += payload_bytes;
	}
	return 1;
}

eReturnStatusAV1 CAV1File::ExtractOBU(/*[in]*/uint8_t* pBuf, /*[in]*/uint32_t dwBufSize, /*[out]*/OBU* pOBU)
{
	uint8_t*	pOrgBuf = pBuf;
	uint8_t*	pOutBuf = NULL;	// start position of buffer
	size_t	uiOutBufSize = 0;
	uint32_t	uiInputBufSize = dwBufSize;
	int		nalUnitType = -1;
	int		layerId = 0;
	bool		bExistLastVclNal = false;


	m_OBUCtx = { NULL, NULL, 0, 0 ,0 };
	
	pOBU->uiNumOfOBU = 0;

	for (int i = 0; i < MAX_OBU_NUM; i++)
	{
		size_t uiReadByte;
		//obudec_read_temporal_unit(&m_OBUCtx, &pOrgBuf, &uiReadByte, &uiOutBufSize);
		
		//uint32_t uiReadByte = ExtractPacket3(pOrgBuf, uiInputBufSize, &pOutBuf, uiOutBufSize);

		//if (!uiReadByte || !uiOutBufSize)
		//{
		//	return RET_EOF_AV1;	// EOF
		//}
		//else
		//{
		//	printf("%d\n", uiReadByte);
		//}
	}

	
//	for (int i = 0; i < MAX_NALU_NUM; i++)
//	{
//		uint32_t uiReadByte = ExtractPacket3(pOrgBuf, uiInputBufSize, &pOutBuf, uiOutBufSize);
//		if (!uiReadByte || !uiOutBufSize)
//		{
//			return RET_EOF;	// EOF
//		}
//		else
//		{
//			int nStartCodeLen = 4 - pOutBuf[2];
//			uint8_t* pInputBuf = pOutBuf + nStartCodeLen; //start code prefix 이후부터 nal unit 시작 
//
//														  // NALU header parsing
//														  //int		forbidden_zero_bit	= (pInputBuf[0] & 0x80) >> 7;		// 1-bit (forbidden_zero_bit)
//			nalUnitType = (pInputBuf[0] & 0x7E) >> 1;		// 6-bit (nal_unit_type)
//			layerId = ((pInputBuf[0] & 0x01) << 5) | ((pInputBuf[1] & 0xF8) >> 3);	// 6-bit (nuh_layer_id)
//																					//int	temporalId	= (pInputBuf[1] & 0x07) - 1;		// 3-bit (nuh_temporal_id_plus1)
//
//			if (IsVclNal(nalUnitType))	// Slices (VCL NALs)
//			{
//				int		first_slice_segment_in_pic_flag = (pInputBuf[2] >> 7);	// Slice header
//				if (first_slice_segment_in_pic_flag == 0)
//				{
//					SET_BUFFER();	// AU 내에 위치함
//				}
//				else	// first VCL NALU
//				{
//					if (pAU->uiNumOfNALU == 0)
//					{
//						SET_BUFFER();	// 첫번째 AU 시작
//					}
//					else if (bExistLastVclNal && (!layerId))	// AU내의 이전 NAL들 중에 VCL NAL이 존재하면 본 NAL이 New AU 시작점이 됨...
//					{	// next AU 시작
//						m_File.seek(-(int)uiReadByte, SEEK_CUR);
//						break;
//					}
//#if 0
//					else if (IsSuffixNAL(nalUnitTypePrev))
//					{	// next AU 시작
//						m_File.seek(-(int)uiReadByte, SEEK_CUR);
//						break;
//					}
//#endif
//					else
//					{
//						SET_BUFFER();	// AU 내에 위치함 (VPS/SPS/PPS, Prefix NALU 등 때문에)
//					}
//				}
//				bExistLastVclNal = true;
//			}
//			else if (nalUnitType == 35)	// AccessUnitDelimiter (always first NALU within an AU)
//			{
//				if (pAU->uiNumOfNALU == 0)
//				{
//					SET_BUFFER();	// 첫번째 AU 시작
//				}
//				else
//				{
//					// next AU 시작
//					m_File.seek(-(int)uiReadByte, SEEK_CUR);
//					break;
//				}
//			}
//			else if ((nalUnitType >= 32 && nalUnitType < 35)	// VPS, SPS, PPS
//				|| (nalUnitType == 39)						// Prefix SEI
//				|| (nalUnitType >= 41 && nalUnitType <= 44)
//				|| (nalUnitType >= 48 && nalUnitType <= 55))
//			{
//				if (pAU->uiNumOfNALU == 0)
//				{
//					SET_BUFFER();		// 첫번째 AU 시작
//				}
//				else
//				{
//					if (bExistLastVclNal && (!layerId))	// AU내의 이전 NAL들 중에 VCL NAL이 존재하면 본 NAL이 New AU 시작점이 됨...
//					{	// next AU 시작
//						m_File.seek(-(int)uiReadByte, SEEK_CUR);
//						break;
//					}
//					else
//					{
//						SET_BUFFER();	// AU 내에 위치함
//					}
//				}
//			}
//			else if (nalUnitType == 36)	// End of Sequence (EOS) (the last NALU in the AU)
//			{
//				SET_BUFFER();	// AU 내에서 마지막 NALU (만약, 바로 뒤에 EOB NALU도 같이 존재한다면, EOB가 마지막 NALU가 된다)
//								// 편의상 여기에서는 뒤에 존재할 수도 있는 EOB를 확인하지 않고, 바로 AU를 종료함.
//				break;
//			}
//			else if (nalUnitType == 37)	// End of Bitstream (EOB) (the last NALU in the AU)
//			{
//				SET_BUFFER();	// AU 내에서 마지막 NALU (항상)
//				break;
//			}
//			else	// 38(Filler Data), 40(Suffix_SEI), ...
//			{
//				SET_BUFFER();	// AU 내에 위치하고, 절대로 AU 내에서 첫번째 NALU가 될 수 없음
//			}
//
//			if (IsEOF())			break;
//		}
//		pOrgBuf += uiReadByte;
//		uiInputBufSize -= uiReadByte;
//		//nalUnitTypePrev	= nalUnitType;
//	}

	return pOBU->uiNumOfOBU ? RET_TRUE_AV1 : RET_FALSE_AV1;
}


