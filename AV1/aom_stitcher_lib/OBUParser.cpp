/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: HEVCParser.cpp
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#include "stdafx.h"
#include <stdio.h>
#include <string.h>  // for memcpy()

//#include "../Common/NAL.h"
//#include "../Decoder/TDecCavlc.h"
#include "Define.h"
//#include "aom/aom_codec.h"
//#include "av1/decoder/decoder.h"
#include "OBUparser.h"
//#include "aom_dsp/bitreader_buffer.h"
//#include "aom_ports/mem_ops.h"

//#include "av1/common/common.h"
//#include "av1/common/timing.h"
//#include "av1/decoder/decoder.h"
//#include "av1/decoder/decodeframe.h"

//#include "bit_reader.h"
#include "bit_reader_c.h"
#include "config.h"

COBUInfo::COBUInfo()
	//: m_nPrevSlicePOC(INT_MAX)
	//, m_bFirstPicInSeq(0)
	//, m_iSliceIdx(0)
	//, m_prevTid0POC(0)
	//, m_prevAssociatedIRAPType(NAL_UNIT_INVALID)
	//, m_prevPocCRA(0)
	//, m_pocRandomAccess(INT_MAX)
	//, m_prevSliceSkipped(false)
	//, m_skippedPOC(0)
	//, m_nLastPOCNoOutputPriorPics(-1)
	//, m_isNoOutputPriorPicsFlag(false)
	//, m_craNoRaslOutputFlag(false)
	//, m_nActiveVpsId(0)
	//, m_nActiveSpsId(0)
	//, m_nActivePpsId(0)
{
	//m_pSlicePilot = new TComSlice;
}

COBUInfo::~COBUInfo()
{
	//SAFE_DELETES(m_pSlicePilot);
}

///////////////////////////////////////////////////////////

COBUParser::COBUParser(void)
	//: m_bFirstSliceInBitstream(true)
	//, m_bShowLogsFlag(0)
{
}

COBUParser::~COBUParser(void)
{
	Destroy();
}

void COBUParser::Create()
{
	//m_ShManager.Init();
}

void COBUParser::Destroy()
{
	//m_ShManager.Destroy();
}

// Picture prediction structures (0-12 are predefined) in scalability metadata.
typedef enum {
	SCALABILITY_L1T2 = 0,
	SCALABILITY_L1T3 = 1,
	SCALABILITY_L2T1 = 2,
	SCALABILITY_L2T2 = 3,
	SCALABILITY_L2T3 = 4,
	SCALABILITY_S2T1 = 5,
	SCALABILITY_S2T2 = 6,
	SCALABILITY_S2T3 = 7,
	SCALABILITY_L2T1h = 8,
	SCALABILITY_L2T2h = 9,
	SCALABILITY_L2T3h = 10,
	SCALABILITY_S2T1h = 11,
	SCALABILITY_S2T2h = 12,
	SCALABILITY_S2T3h = 13,
	SCALABILITY_SS = 14
} SCALABILITY_STRUCTURES;

bool COBUParser::ValidObuType(int obu_type) {
	switch (obu_type) {
	case OBU_SEQUENCE_HEADER:
	case OBU_TEMPORAL_DELIMITER:
	case OBU_FRAME_HEADER:
	case OBU_TILE_GROUP:
	case OBU_METADATA:
	case OBU_FRAME:
	case OBU_REDUNDANT_FRAME_HEADER:
	case OBU_TILE_LIST:
	case OBU_PADDING: return true;
	}
	return false;
}

aom_codec_err_t COBUParser::ReadObuSize(const uint8_t *data,
	size_t bytes_available,
	size_t *const obu_size,
	size_t *const length_field_size) {
	uint64_t u_obu_size = 0;
	if (aom_uleb_decode(data, bytes_available, &u_obu_size, length_field_size) !=
		0) {
		return AOM_CODEC_CORRUPT_FRAME;
	}

	if (u_obu_size > UINT32_MAX) return AOM_CODEC_CORRUPT_FRAME;
	*obu_size = (size_t)u_obu_size;
	return AOM_CODEC_OK;
}


// Parses OBU header and stores values in 'header'.
aom_codec_err_t COBUParser::ReadObuHeaderC(CBitReader *rb,
	int is_annexb, ObuHeader *header) {
	if (!rb || !header) return AOM_CODEC_INVALID_PARAM;

	const ptrdiff_t bit_buffer_byte_length = rb->AomRbReadBitBufferEnd() - rb->AomRbReadBitBuffer();
	if (bit_buffer_byte_length < 1) return AOM_CODEC_CORRUPT_FRAME;

	header->size = 1;

	if (rb->AomRbReadBit() != 0) {
		// Forbidden bit. Must not be set.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	header->type = (OBU_TYPE)rb->AomRbReadLiteral(4);

	if (!ValidObuType(header->type)) return AOM_CODEC_CORRUPT_FRAME;

	header->has_extension = rb->AomRbReadBit();
	header->has_size_field = rb->AomRbReadBit();

	if (!header->has_size_field && !is_annexb) {
		// section 5 obu streams must have obu_size field set.
		return AOM_CODEC_UNSUP_BITSTREAM;
	}

	if (rb->AomRbReadBit() != 0) {
		// obu_reserved_1bit must be set to 0.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	if (header->has_extension) {
		if (bit_buffer_byte_length == 1) return AOM_CODEC_CORRUPT_FRAME;

		header->size += 1;
		header->temporal_layer_id = rb->AomRbReadLiteral(3);
		header->spatial_layer_id = rb->AomRbReadLiteral(2);
		if (rb->AomRbReadLiteral(3) != 0) {
			// extension_header_reserved_3bits must be set to 0.
			return AOM_CODEC_CORRUPT_FRAME;
		}
	}

	return AOM_CODEC_OK;
}

// Parses OBU header and stores values in 'header'.
//aom_codec_err_t COBUParser::ReadObuHeader(struct AomReadBitBuffer *rb,
//	int is_annexb, ObuHeader *header) {
//	if (!rb || !header) return AOM_CODEC_INVALID_PARAM;
//
//	const ptrdiff_t bit_buffer_byte_length = rb->bit_buffer_end - rb->bit_buffer;
//	if (bit_buffer_byte_length < 1) return AOM_CODEC_CORRUPT_FRAME;
//
//	header->size = 1;
//
//	if (AomRbReadBit(rb) != 0) {
//		// Forbidden bit. Must not be set.
//		return AOM_CODEC_CORRUPT_FRAME;
//	}
//
//	header->type = (OBU_TYPE)AomRbReadLiteral(rb, 4);
//
//	if (!ValidObuType(header->type)) return AOM_CODEC_CORRUPT_FRAME;
//
//	header->has_extension = AomRbReadBit(rb);
//	header->has_size_field = AomRbReadBit(rb);
//
//	if (!header->has_size_field && !is_annexb) {
//		// section 5 obu streams must have obu_size field set.
//		return AOM_CODEC_UNSUP_BITSTREAM;
//	}
//
//	if (AomRbReadBit(rb) != 0) {
//		// obu_reserved_1bit must be set to 0.
//		return AOM_CODEC_CORRUPT_FRAME;
//	}
//
//	if (header->has_extension) {
//		if (bit_buffer_byte_length == 1) return AOM_CODEC_CORRUPT_FRAME;
//
//		header->size += 1;
//		header->temporal_layer_id = AomRbReadLiteral(rb, 3);
//		header->spatial_layer_id = AomRbReadLiteral(rb, 2);
//		if (AomRbReadLiteral(rb, 3) != 0) {
//			// extension_header_reserved_3bits must be set to 0.
//			return AOM_CODEC_CORRUPT_FRAME;
//		}
//	}
//
//	return AOM_CODEC_OK;
//}

aom_codec_err_t COBUParser::AomReadObuHeaderC(uint8_t *buffer, size_t buffer_length,
	size_t *consumed, ObuHeader *header, int is_annexb) {
	if (buffer_length < 1 || !consumed || !header) return AOM_CODEC_INVALID_PARAM;

	// TODO(tomfinegan): Set the error handler here and throughout this file, and
	// confirm parsing work done via aom_read_bit_buffer is successful.
	CBitReader rb(buffer, buffer + buffer_length, 0);
	aom_codec_err_t parse_result = ReadObuHeaderC(&rb, is_annexb, header);
	if (parse_result == AOM_CODEC_OK) *consumed = header->size;
	return parse_result;
}

//aom_codec_err_t COBUParser::AomReadObuHeader(uint8_t *buffer, size_t buffer_length,
//	size_t *consumed, ObuHeader *header,	int is_annexb) {
//	if (buffer_length < 1 || !consumed || !header) return AOM_CODEC_INVALID_PARAM;
//
//	// TODO(tomfinegan): Set the error handler here and throughout this file, and
//	// confirm parsing work done via aom_read_bit_buffer is successful.
//	struct AomReadBitBuffer rb = { buffer, buffer + buffer_length, 0};
//	aom_codec_err_t parse_result = ReadObuHeader(&rb, is_annexb, header);
//	if (parse_result == AOM_CODEC_OK) *consumed = header->size;
//	return parse_result;
//}

aom_codec_err_t COBUParser::AomReadObuHeaderAndSize(const uint8_t *data,
	size_t bytes_available,
	int is_annexb,
	ObuHeader *obu_header,
	size_t *const payload_size,
	size_t *const bytes_read) {
	size_t length_field_size = 0, obu_size = 0;
	aom_codec_err_t status;

	if (is_annexb) {
		// Size field comes before the OBU header, and includes the OBU header
		status =
			ReadObuSize(data, bytes_available, &obu_size, &length_field_size);

		if (status != AOM_CODEC_OK) return status;
	}

	CBitReader rb(data + length_field_size, data + bytes_available, 0);
	//struct AomReadBitBuffer rb = { data + length_field_size, data + bytes_available, 0};

	status = ReadObuHeaderC(&rb, is_annexb, obu_header); //obu.c
	if (status != AOM_CODEC_OK) return status;

	if (is_annexb) {
		// Derive the payload size from the data we've already read
		if (obu_size < obu_header->size) return AOM_CODEC_CORRUPT_FRAME;

		*payload_size = obu_size - obu_header->size;
	}
	else {
		// Size field comes after the OBU header, and is just the payload size
		status = ReadObuSize(data + obu_header->size,
			bytes_available - obu_header->size, payload_size,  //obu.c
			&length_field_size);
		if (status != AOM_CODEC_OK) return status;
	}

	*bytes_read = length_field_size + obu_header->size;
	return AOM_CODEC_OK;
}
//int COBUParser::DecodeOneOBU(const uint8_t *data, const uint8_t *data_end, const uint8_t **p_data_end)
//bool COBUParser::DecodeOneOBU(uint8_t *pBitStream, uint32_t uiBitstreamSize, bool bAnnexB) {
//	//AV1_COMMON *const cm = &pbi->common;
//	int frame_decoding_finished = 0;
//	int is_first_tg_obu_received = 1;
//	uint32_t frame_header_size = 0;
//	int seq_header_received = 0;
//	size_t seq_header_size = 0;
//	ObuHeader obu_header;
//	memset(&obu_header, 0, sizeof(obu_header));
//
//	const uint8_t *data = pBitStream;
//	const uint8_t *data_end = pBitStream + uiBitstreamSize;
//
//	//pbi->seen_frame_header = 0;
//
//	//if (data_end < data) {
//	//	cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
//	//	return -1;
//	//}
//
//	// Reset pbi->camera_frame_header_ready to 0 if cm->large_scale_tile = 0.
//	//if (!cm->large_scale_tile) pbi->camera_frame_header_ready = 0;
//
//	// decode frame as a series of OBUs
//	while (!frame_decoding_finished) {
//		struct AomReadBitBuffer rb;
//		size_t payload_size = 0;
//		size_t decoded_payload_size = 0;
//		size_t obu_payload_offset = 0;
//		size_t bytes_read = 0;
//		//const size_t bytes_available = data_end - data;
//		const size_t bytes_available = uiBitstreamSize;
//
//		aom_codec_err_t status =
//			AomReadObuHeaderAndSize(data, bytes_available, bAnnexB,
//				&obu_header, &payload_size, &bytes_read);
//
//		if (status != AOM_CODEC_OK) {
//			return -1;
//		}
//
//		// Record obu size header information.
//		//pbi->obu_size_hdr.data = data + obu_header.size;
//		//pbi->obu_size_hdr.size = bytes_read - obu_header.size;
//
//		// Note: aom_read_obu_header_and_size() takes care of checking that this
//		// doesn't cause 'data' to advance past 'data_end'.
//		data += bytes_read;
//
//		if ((size_t)(data_end - data) < payload_size) {
//			return -1;
//		}
//		
//		//if (obu_header.type != OBU_TEMPORAL_DELIMITER &&
//		//	obu_header.type != OBU_SEQUENCE_HEADER &&
//		//	obu_header.type != OBU_PADDING) {
//		//	// don't decode obu if it's not in current operating mode
//		//	if (!IsObuInCurrentOperatingPoint(pbi, obu_header)) {
//		//		data += payload_size;
//		//		continue;
//		//	}
//		//}
//		InitReadBitBuffer(&rb, data, data + payload_size);
//		//av1_init_read_bit_buffer(pbi, &rb, data, data + payload_size);
//
//		switch (obu_header.type) {
//		case OBU_TEMPORAL_DELIMITER:
//			decoded_payload_size = ReadTemporalDelimiterObu();
//			//pbi->seen_frame_header = 0;
//			break;
//		//case OBU_SEQUENCE_HEADER:
//		//	if (!seq_header_received) {
//		//		decoded_payload_size = ReadSequenceHeaderObu(&rb);
//		//		//if (cm->error.error_code != AOM_CODEC_OK) return -1;
//
//		//		seq_header_size = decoded_payload_size;
//		//		seq_header_received = 1;
//		//	}
//		//	else {
//		//		// Seeing another sequence header, skip as all sequence headers are
//		//		// required to be identical except for the contents of
//		//		// operating_parameters_info and the amount of trailing bits.
//		//		// TODO(yaowu): verifying redundant sequence headers are identical.
//		//		decoded_payload_size = seq_header_size;
//		//	}
//		//	break;
//		//case OBU_FRAME_HEADER:
//		//case OBU_REDUNDANT_FRAME_HEADER:
//		//case OBU_FRAME:
//		
//		default:
//			// Skip unrecognized OBUs
//			decoded_payload_size = payload_size;
//			break;
//		}
//
//		// Check that the signalled OBU size matches the actual amount of data read
//		if (decoded_payload_size > payload_size) {
//			//cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
//			return -1;
//		}
//
//		// If there are extra padding bytes, they should all be zero
//		while (decoded_payload_size < payload_size) {
//			uint8_t padding_byte = data[decoded_payload_size++];
//			if (padding_byte != 0) {
//				//cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
//				return -1;
//			}
//		}
//
//		data += payload_size;
//	}
//
//	return frame_decoding_finished;
//}

bool COBUParser::DecodeOneOBUC(uint8_t *pBitStream, uint32_t uiBitstreamSize, bool bAnnexB) {
	//AV1_COMMON *const cm = &pbi->common;
	int frame_decoding_finished = 0;
	int is_first_tg_obu_received = 1;
	uint32_t frame_header_size = 0;
	int seq_header_received = 0;
	size_t seq_header_size = 0;
	ObuHeader obu_header;
	memset(&obu_header, 0, sizeof(obu_header));

	const uint8_t *data = pBitStream;
	const uint8_t *data_end = pBitStream + uiBitstreamSize;

	//pbi->seen_frame_header = 0;

	//if (data_end < data) {
	//	cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
	//	return -1;
	//}

	// Reset pbi->camera_frame_header_ready to 0 if cm->large_scale_tile = 0.
	//if (!cm->large_scale_tile) pbi->camera_frame_header_ready = 0;

	// decode frame as a series of OBUs
	while (!frame_decoding_finished) {
		//struct AomReadBitBuffer rb;
		size_t payload_size = 0;
		size_t decoded_payload_size = 0;
		size_t obu_payload_offset = 0;
		size_t bytes_read = 0;
		const size_t bytes_available = data_end - data;
		//const size_t bytes_available = uiBitstreamSize;

		aom_codec_err_t status =
			AomReadObuHeaderAndSize(data, bytes_available, bAnnexB,
				&obu_header, &payload_size, &bytes_read);

		if (status != AOM_CODEC_OK) {
			return -1;
		}

		// Record obu size header information.
		//pbi->obu_size_hdr.data = data + obu_header.size;
		//pbi->obu_size_hdr.size = bytes_read - obu_header.size;

		// Note: aom_read_obu_header_and_size() takes care of checking that this
		// doesn't cause 'data' to advance past 'data_end'.
		data += bytes_read;

		if ((size_t)(data_end - data) < payload_size) {
			return -1;
		}

		//if (obu_header.type != OBU_TEMPORAL_DELIMITER &&
		//	obu_header.type != OBU_SEQUENCE_HEADER &&
		//	obu_header.type != OBU_PADDING) {
		//	// don't decode obu if it's not in current operating mode
		//	if (!IsObuInCurrentOperatingPoint(pbi, obu_header)) {
		//		data += payload_size;
		//		continue;
		//	}
		//}
		CBitReader rb(data, data + payload_size, 0);
		//InitReadBitBuffer(&rb, data, data + payload_size);
		//av1_init_read_bit_buffer(pbi, &rb, data, data + payload_size);

		switch (obu_header.type) {
		case OBU_TEMPORAL_DELIMITER:
			decoded_payload_size = ReadTemporalDelimiterObu();
			m_SeenFrameHeader = 0;
			break;
			case OBU_SEQUENCE_HEADER:
				if (!seq_header_received) {
					decoded_payload_size = ReadSequenceHeaderObu(&rb);
					//if (cm->error.error_code != AOM_CODEC_OK) return -1;

					seq_header_size = decoded_payload_size;
					seq_header_received = 1;
				}
				else {
					// Seeing another sequence header, skip as all sequence headers are
					// required to be identical except for the contents of
					// operating_parameters_info and the amount of trailing bits.
					// TODO(yaowu): verifying redundant sequence headers are identical.
					decoded_payload_size = seq_header_size;
				}
				break;
			case OBU_FRAME_HEADER:
			case OBU_REDUNDANT_FRAME_HEADER:
			case OBU_FRAME:
				// Only decode first frame header received
				if (!m_SeenFrameHeader /*||
					(cm->large_scale_tile && !pbi->camera_frame_header_ready)*/) {
					frame_header_size = ReadFrameHeaderObu(               //obu.c
						&rb, data, obu_header.type != OBU_FRAME);
					m_SeenFrameHeader = 1;
					//if (!pbi->ext_tile_debug && cm->large_scale_tile)
					//	pbi->camera_frame_header_ready = 1;
				}
				else {
					//// TODO(wtc): Verify that the frame_header_obu is identical to the
					//// original frame_header_obu. For now just skip frame_header_size
					//// bytes in the bit buffer.
					//if (frame_header_size > payload_size) {
					//	cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
					//	return -1;
					//}
					//assert(rb.bit_offset == 0);
					//rb.bit_offset = 8 * frame_header_size;
				}
				//decoded_payload_size = frame_header_size;
				//pbi->frame_header_size = frame_header_size;

				//if (cm->show_existing_frame) {
				//	if (obu_header.type == OBU_FRAME) {
				//		cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
				//		return -1;
				//	}
				//	frame_decoding_finished = 1;
				//	pbi->seen_frame_header = 0;
				//	break;
				//}

		default:
			// Skip unrecognized OBUs
			decoded_payload_size = payload_size;
			break;
		}

		// Check that the signalled OBU size matches the actual amount of data read
		if (decoded_payload_size > payload_size) {
			//cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return -1;
		}

		// If there are extra padding bytes, they should all be zero
		while (decoded_payload_size < payload_size) {
			uint8_t padding_byte = data[decoded_payload_size++];
			if (padding_byte != 0) {
				//cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
				return -1;
			}
		}

		data += payload_size;
	}

	return frame_decoding_finished;
}


uint32_t COBUParser::ReadSequenceHeaderObu(CBitReader *rb)
{
	// Use a local variable to store the information as we decode. At the end,
	// if no errors have occurred, cm->seq_params is updated.
	//SequenceHeader sh = cm->seq_params;
	//SequenceHeader *const seq_params = &sh;

	const uint32_t saved_bit_offset = rb->AomRbReadBitOffset();
	CSequenceHeader *pSh = &m_ShBuffer;

	pSh->ShParserProfile(BITSTREAM_PROFILE(rb->AomRbReadLiteral(PROFILE_BITS)));

	if (pSh->ShReadProfile() > CONFIG_MAX_DECODE_PROFILE) {
		return 0;
	}

	// Still picture or not
	pSh->ShParserStillPicture(rb->AomRbReadBit());
	pSh->ShParserReducedStillPictureHdr(rb->AomRbReadBit());

	// Video must have reduced_still_picture_hdr = 0
	if (pSh->AvReadStillPicture() && pSh->ShReadReducedStillPictureHdr()) {
		return 0;
	}

	if (pSh->ShReadReducedStillPictureHdr()) {
		pSh->ShParserTimingInfoPresentFlag(0);
		pSh->ShParserDecoderModelInfoPresentFlag(0);
		pSh->ShParserInitialDisplayDelayPresentFlag(0);
		pSh->ShParserOperatingPointsCntMinus1(0);
		pSh->ShParserOperatingPointIdc(0, 0);
		pSh->ShParserSeqLevelIdx(0, rb);
		pSh->ShParserSeqTier(0, 0);
		pSh->ShParserDecoderModelPresentForThisOp(0, 0);
		pSh->ShParserInitialDisplayDelayPresentForThisOp(0, 0);
	}
	else {
		pSh->ShParserTimingInfoPresentFlag(rb->AomRbReadBit()); // timing_info_present_flag
		if(pSh->ShReadTimingInfoPresentFlag()) {
			pSh->ShParserTimingInfoHeader(rb);
			pSh->ShParserDecoderModelInfoPresentFlag(rb->AomRbReadBit());

			if (pSh->ShReadDecoderModelInfoPresentFlag())
				pSh->ShParserDecoderModelInfo(rb);
		}
		else {
			pSh->ShParserDecoderModelInfoPresentFlag(0);
		}
		pSh->ShParserInitialDisplayDelayPresentFlag(rb->AomRbReadBit());
		pSh->ShParserOperatingPointsCntMinus1(rb->AomRbReadLiteral(OP_POINTS_CNT_MINUS_1_BITS));
		
		for (int i = 0; i < pSh->ShReadOperatingPointsCntMinus1() + 1; i++) {
			pSh->ShParserOperatingPointIdc(i, rb->AomRbReadLiteral(OP_POINTS_IDC_BITS));			

			if (!pSh->ShParserSeqLevelIdx(i, rb)) {
				return 0;
			}
			// This is the seq_level_idx[i] > 7 check in the spec. seq_level_idx 7
			// is equivalent to level 3.3.
			if(pSh->ShReadSeqLevelIdxMajor(i) > 3)
				pSh->ShParserSeqTier(i, rb->AomRbReadBit());
			else
				pSh->ShParserSeqTier(i, 0);

			if (pSh->ShReadDecoderModelInfoPresentFlag())
			{
				pSh->ShParserDecoderModelPresentForThisOp(i, rb->AomRbReadBit());
				if (pSh->ShReadDecoderModelPresentForThisOp(i))
					pSh->ShParserOperatingParametersInfo(i, rb);
			}
			else {
				pSh->ShParserDecoderModelPresentForThisOp(i, 0);
			}

			if (pSh->ShReadInitialDisplayDelayPresentFlag())
			{
				pSh->ShParserInitialDisplayDelayPresentForThisOp(i, rb->AomRbReadBit());
				if (pSh->ShReadInitialDisplayDelayPresentForThisOp(i))
					pSh->ShParserInitialDisplayDelayMinus1(i, rb->AomRbReadLiteral(4));
				else
					pSh->ShParserInitialDisplayDelayMinus1(i, 9);
			}
			else
			{
				pSh->ShParserInitialDisplayDelayPresentForThisOp(i, 0);
				pSh->ShParserInitialDisplayDelayMinus1(i, 9);
			}
		}
	}

	pSh->ShParserSequenceInfo(rb);
	pSh->ShParserColorConfig(rb);

	pSh->ShParserFilmGrainParamsPresent(rb->AomRbReadBit());

	if (rb->Av1CheckTrailingBits() != 0) {
		return 0;
	}

	return ((rb->AomRbReadBitOffset() - saved_bit_offset + 7) >> 3);
}


uint32_t COBUParser::ReadFrameHeaderObu(CBitReader *rb, const uint8_t *data, int trainiling_bits_present)
{

	const uint32_t saved_bit_offset = rb->AomRbReadBitOffset();
	CSequenceHeader *pSh = &m_ShBuffer;
	CFrameHeader *pFh = &m_FhBuffer;
	ObuHeader obu_header;

	int idLen = 0;
	int FrameIsIntra = 1;
	FRAME_TYPE frame_type;
	int show_frame;

	if (pSh->ShReadFrameIdNumbersPresentFlag()) {
		idLen = pSh->ShReadAdditionalFrameIdLengthMinus1() +
			pSh->ShReadDeltaFrameIdLengthMinus2() + 3;
	}
	int allFrames = (1 << NUM_REF_FRAMES) - 1;

	if (pSh->ShReadReducedStillPictureHdr()) {
		pFh->FhParserShowExistingFrame(0);
		frame_type = KEY_FRAME;
		pFh->FhParserFrameType(frame_type);
		show_frame = 1;
		pFh->FhParserShowFrame(1);		
		pFh->FhParserShowableFrame(0);
	}
	else
	{
		pFh->FhParserShowExistingFrame(rb->AomRbReadBit());

		if (pFh->FhReadShowExistingFrame() == 1)
		{
			pFh->FhParserFrameToShowMapIdx(rb->AomRbReadLiteral(3));

			if (pSh->ShReadDecoderModelInfoPresentFlag() && !pSh->ShReadEqualPictureInterval()) {
				pFh->FhParserTemporalPointInfo(pSh->ShReadDecoderModelInfo().frame_presentation_time_length_minus_1, rb);
			}
			pFh->FhParserRefreshFrameFlags(0);

			if (pSh->ShReadFrameIdNumbersPresentFlag()) {
				pFh->FhParserDisplayFrameId(rb->AomRbReadLiteral(idLen));
			}
			frame_type = KEY_FRAME; /*frame_type = RefFrameType[ frame_to_show_map_idx ]*/
			pFh->FhParserFrameType(frame_type);

			if (frame_type == KEY_FRAME) {
				pFh->FhParserRefreshFrameFlags(allFrames);
			}

			//if(pSh->ShReadFilmGrainParamsPresent())
			//	load_grain_params(frame_to_show_map_idx)
			return 0;
		}

		frame_type = (FRAME_TYPE)rb->AomRbReadLiteral(2);
		pFh->FhParserFrameType(frame_type);
		int FrameIsIntra = (frame_type == INTRA_ONLY_FRAME || frame_type == KEY_FRAME);

		show_frame = rb->AomRbReadBit();
		pFh->FhParserShowFrame(show_frame);

		if (show_frame && pSh->ShReadDecoderModelInfoPresentFlag() && !pSh->ShReadEqualPictureInterval()) {
			pFh->FhParserTemporalPointInfo(pSh->ShReadDecoderModelInfo().frame_presentation_time_length_minus_1, rb);
		}

		if (show_frame) {
			pFh->FhParserShowableFrame(frame_type != KEY_FRAME);
		}
		else {
			pFh->FhParserShowableFrame(rb->AomRbReadBit());
		}

		if (frame_type == SWITCH_FRAME || (frame_type == KEY_FRAME && show_frame)) {
			pFh->FhParserErrorResilientMode(1);
		}
		else {
			pFh->FhParserErrorResilientMode(rb->AomRbReadBit());
		}
	}
	
	if (frame_type == KEY_FRAME && show_frame) {
		pFh->FhSetRefValandOrderHint();
	}

	pFh->FhParserDisableCdfUpdate(rb->AomRbReadBit());

	if (pSh->ShReadSeqForceScreenContentTools() == 2) {//SELECT_SCREEN_CONTENT_TOOLS
		pFh->FhParserAllowScreenContentTools(rb->AomRbReadBit());
	}
	else {
		pFh->FhParserAllowScreenContentTools(pSh->ShReadSeqForceScreenContentTools());
	}

	if (pFh->FhReadAllowScreenContentTools()) {
		int seq_force_integer_mv = pSh->ShReadSeqForceIntegerMv();
		if (seq_force_integer_mv == 2) {// SELECT_INTEGER_MV
			pFh->FhParserForceIntegerMv(rb->AomRbReadBit());
		}
		else {
			pFh->FhParserForceIntegerMv(seq_force_integer_mv);
		}
	}
	else {
		pFh->FhParserForceIntegerMv(0);
	}

	if(FrameIsIntra)
		pFh->FhParserForceIntegerMv(1);


	if (pSh->ShReadFrameIdNumbersPresentFlag()) {
		int prev_frame_id = 0;
		int have_prev_frame_id = /* !pbi->decoding_first_frame &&*/  !(frame_type == KEY_FRAME && show_frame);
		if (have_prev_frame_id) {
			pFh->FhSetPrevFrameID();
		}
		pFh->FhParserCurrentFrameId(rb->AomRbReadLiteral(idLen));
		pFh->FhSetMarkRefFrames(idLen, pSh->ShReadDeltaFrameIdLengthMinus2());
	}
	else {
		pFh->FhParserCurrentFrameId(0);
	}

	if (frame_type == SWITCH_FRAME)
		pFh->FhParserFrameSizeOverrideFlag(1);
	else if(pSh->ShReadReducedStillPictureHdr())
		pFh->FhParserFrameSizeOverrideFlag(0);
	else
		pFh->FhParserFrameSizeOverrideFlag(rb->AomRbReadBit());

	int OrderHintBits = pSh->ShReadOrderHintBits();
	pFh->FhParserOrderHint(rb->AomRbReadLiteral(OrderHintBits));
	int OrderHint = pFh->FhReadOrderHint();

	if (FrameIsIntra || pFh->FhReadErrorResilientMode()) {
		pFh->FhParserPrimaryRefFrame(PRIMARY_REF_NONE);
	}
	else{
		pFh->FhParserPrimaryRefFrame(rb->AomRbReadLiteral(3));
	}

	if (pSh->ShReadDecoderModelInfoPresentFlag())
	{
		int buffer_removal_time_present_flag = rb->AomRbReadBit();
		pFh->FhParserBufferRemovalTimePresentFlag(buffer_removal_time_present_flag);

		if (buffer_removal_time_present_flag) {
			int operating_points_cnt_minus_1 = pSh->ShReadOperatingPointsCntMinus1();

			int n = pSh->ShReadDecoderModelInfo().buffer_removal_time_length_minus_1 + 1;
			for (int opNum = 0; opNum <= operating_points_cnt_minus_1; opNum++) {
				if (pSh->ShReadDecoderModelPresentForThisOp(opNum)) {
					int opPtIdc = pSh->ShReadOperatingPointIdc(opNum);

					int inTemporalLayer = (opPtIdc >> obu_header.temporal_layer_id) & 1;
					int	inSpatialLayer = (opPtIdc >> (obu_header.spatial_layer_id + 8)) & 1;
						
					if (opPtIdc == 0 || (inTemporalLayer && inSpatialLayer)) {
						pFh->FhParserBufferRemovalTime(opNum, rb->AomRbReadLiteral(n));
					}
				}
			}
		}
	}
	pFh->FhParserAllowHighPrecisionMv(0);
	pFh->FhParserUseRefFrameMvs(0);
	pFh->FhParserAllowIntrabc(0);

	if (frame_type == SWITCH_FRAME || (frame_type == KEY_FRAME && show_frame)) {
		pFh->FhParserRefreshFrameFlags(allFrames);
	}
	else {
		pFh->FhParserRefreshFrameFlags(rb->AomRbReadLiteral(8));
	}

	if (!FrameIsIntra || pFh->FhReadRefreshFrameFlags() != allFrames) {
		if (pFh->FhReadErrorResilientMode() && pSh->ShReadEnableOrderHint()){
			pFh->FhParseRefOrderHint(OrderHintBits, rb);
		}
	}

	if (frame_type == KEY_FRAME) {
		pFh->FhParserFrameSize(
			pSh->ShReadFrameWidthBitsMinus1(),
			pSh->ShReadFrameHeightBitsMinus1(),
			pSh->ShReadMaxFrameWidthMinus1(),
			pSh->ShReadMaxFrameHeightMinus1(),
			pSh->ShReadEnableSuperres(), rb);
		pFh->FhRenderSize(rb);

		if (pFh->FhReadAllowScreenContentTools() &&
			pFh->FhReadUpscaledWidth() == pFh->FhReadFrameWidth()) {
			pFh->FhParserAllowIntrabc(rb->AomRbReadBit());
		}
	}
	else {
		if (frame_type == INTRA_ONLY_FRAME) {
			pFh->FhParserFrameSize(
				pSh->ShReadFrameWidthBitsMinus1(),
				pSh->ShReadFrameHeightBitsMinus1(),
				pSh->ShReadMaxFrameWidthMinus1(),
				pSh->ShReadMaxFrameHeightMinus1(),
				pSh->ShReadEnableSuperres(), rb);
			pFh->FhRenderSize(rb);

			if (pFh->FhReadAllowScreenContentTools() &&
				pFh->FhReadUpscaledWidth() == pFh->FhReadFrameWidth()) {
				pFh->FhParserAllowIntrabc(rb->AomRbReadBit());
			}
		}
		else {
			int frame_refs_short_signaling;
			if (!pSh->ShReadEnableOrderHint()) {
				frame_refs_short_signaling = 0;
				pFh->FhParserFrameRefsShortSignaling(frame_refs_short_signaling);
			}
			else{
				frame_refs_short_signaling = rb->AomRbReadBit();
				pFh->FhParserFrameRefsShortSignaling(frame_refs_short_signaling);

				if (frame_refs_short_signaling) {
					pFh->FhParserLastFrameIdx(rb->AomRbReadLiteral(3));
					pFh->FhParserGoldFrameIdx(rb->AomRbReadLiteral(3));
					//set_frame_refs()
				}
			}

			for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
				if (!frame_refs_short_signaling)
					pFh->FhParserRefFramesIdx(i, rb->AomRbReadLiteral(3));
				if (pSh->ShReadFrameIdNumbersPresentFlag()) {
					int n = pSh->ShReadDeltaFrameIdLengthMinus2() + 2;
					int delta_frame_id_minus_1 = rb->AomRbReadLiteral(n);
					pFh->FhParserDeltaFrameIdMinus1(delta_frame_id_minus_1);
					int DeltaFrameId = delta_frame_id_minus_1 + 1;
					int expectedFrameId = ((pFh->FhReadCurrentFrameId() + (1 << idLen) -
						DeltaFrameId) % (1 << idLen));
					pFh->FhParserExpectedFrameId(i, expectedFrameId);
				}
			}

			if (pFh->FhReadFrameSizeOverrideFlag() && !pFh->FhReadErrorResilientMode()) {
				pFh->FhParserFrameSizeWithRefs(
					pSh->ShReadFrameWidthBitsMinus1(),
					pSh->ShReadFrameHeightBitsMinus1(),
					pSh->ShReadMaxFrameWidthMinus1(),
					pSh->ShReadMaxFrameHeightMinus1(),
					pSh->ShReadEnableSuperres(), rb);
			}
			else {
				pFh->FhParserFrameSize(
					pSh->ShReadFrameWidthBitsMinus1(),
					pSh->ShReadFrameHeightBitsMinus1(),
					pSh->ShReadMaxFrameWidthMinus1(),
					pSh->ShReadMaxFrameHeightMinus1(),
					pSh->ShReadEnableSuperres(), rb);
				pFh->FhRenderSize(rb);
			}

			pFh->FhParserForceIntegerMv(rb);
			pFh->FhParserInterpolationFilter(rb);
			pFh->FhParserIsMotionModeSwitchable(rb->AomRbReadBit());

			if (pFh->FhReadErrorResilientMode() || pSh->ShReadEnableRefFrameMvs()) {
				pFh->FhParserUseRefFrameMvs(0);
			}
			else {
				pFh->FhParserUseRefFrameMvs(rb->AomRbReadBit());
			}
		}
	}

	if (!FrameIsIntra){
		//for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
		//	int refFrame = LAST_FRAME + i;

		//	int	hint = RefOrderHint[ref_frame_idx[i]]
		//		OrderHints[refFrame] = hint
		//		if (!enable_order_hint) {
		//			RefFrameSignBias[refFrame] = 0
		//		}
		//		else {
		//			RefFrameSignBias[refFrame] = get_relative_dist(hint, OrderHint) > 0
		//		}
		//}
	}

	if (pSh->ShReadReducedStillPictureHdr() || pFh->FhReadDisableCdfUpdate())
		pFh->FhParserDisableFrameEndUpdateCdf(1);
	else
		pFh->FhParserDisableFrameEndUpdateCdf(rb->AomRbReadBit());

	if (pFh->FhReadPrimaryRefFrame() == PRIMARY_REF_NONE) {
		//init_non_coeff_cdfs()
		//setup_past_independence()
	}
	else{
		//load_cdfs( ref_frame_idx[ primary_ref_frame ] )
		//load_previous( )
	}

	if (pFh->FhReadUseRefFrameMvs() == 1) {
		//motion_field_estimation();
	}

	pFh->FhParserTileInfo(pSh->ShReadUse128x128Superblock(), rb);
	pFh->FhParserQuantizationParams(pSh->ShReadNumPlanes(), pSh->ShReadSeparateUvDeltaQ(), rb);
	pFh->FhParserSegmentationParams(rb);
	pFh->FhParserDeltaQParams(rb);
	pFh->FhParserDeltaLfParams(rb);

	if (pFh->FhReadPrimaryRefFrame() == PRIMARY_REF_NONE) {
		//init_coeff_cdfs(); //unimplemented
	}
	else {
		//load_previous_segment_ids(); //unimplemented
	}

	pFh->FhSetCodedLossless(1);
	for (int segmentId = 0; segmentId < MAX_SEGMENTS; segmentId++)
	{
		//unimplemented
	}

	int AllLossless = pFh->FhReadCodedLossless() && (pFh->FhReadFrameWidth() == pFh->FhReadUpscaledWidth());
	pFh->FhSetAllLossless(AllLossless);
	pFh->FhParserLoopFilterParams(pSh->ShReadNumPlanes(), rb);
	pFh->FhParserCdefParams(pSh->ShReadNumPlanes(), pSh->ShReadEnableCdef(), rb);


	return 1;
}