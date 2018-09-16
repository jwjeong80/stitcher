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

	const ptrdiff_t bit_buffer_byte_length = rb->m_bit_buffer_end - rb->m_bit_buffer;
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
		//const size_t bytes_available = data_end - data;
		const size_t bytes_available = uiBitstreamSize;

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
			//pbi->seen_frame_header = 0;
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
			//case OBU_FRAME_HEADER:
			//case OBU_REDUNDANT_FRAME_HEADER:
			//case OBU_FRAME:

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

	const uint32_t saved_bit_offset = rb->m_bit_offset;
	CSequenceHeader *pSh = &m_ShBuffer;

	pSh->ShWriteProfile(BITSTREAM_PROFILE(rb->AomRbReadLiteral(PROFILE_BITS)));

	if (pSh->ShReadProfile() > CONFIG_MAX_DECODE_PROFILE) {
		return 0;
	}

	// Still picture or not
	pSh->ShWriteStillPicture(rb->AomRbReadBit());
	pSh->ShWriteReducedStillPictureHdr(rb->AomRbReadBit());

	// Video must have reduced_still_picture_hdr = 0
	if (pSh->AvReadStillPicture() && pSh->ShReadReducedStillPictureHdr()) {
		return 0;
	}

	if (pSh->ShReadReducedStillPictureHdr()) {
		pSh->ShWriteTimingInfoPresentFlag(0);
		pSh->ShWriteDecoderModelInfoPresentFlag(0);
		pSh->ShWriteInitialDisplayDelayPresentFlag(0);
		pSh->ShWritemOperatingPointsCntMinus1(0);
		pSh->ShWritemOperatingPointIdc(0, 0);

		const uint8_t seq_level_idx = rb->AomRbReadLiteral(LEVEL_BITS);
		if (pSh->is_valid_seq_level_idx(seq_level_idx)) return 0;
		BitstreamLevel bl;
		bl.major = (seq_level_idx >> LEVEL_MINOR_BITS) + LEVEL_MAJOR_MIN;
		bl.minor = seq_level_idx & ((1 << LEVEL_MINOR_BITS) - 1);
		pSh->ShWriteSeqLevelIdx(0, bl);
		pSh->ShWriteSeqTier(0, 0);
		pSh->ShWriteDecoderModelPresentForThisOp(0, 0);
		pSh->ShWriteInitialDisplayDelayPresentForThisOp(0, 0);
	}
	else {
		pSh->ShWriteTimingInfoPresentFlag(rb->AomRbReadBit());
		if(pSh->ShReadTimingInfoPresentFlag()) {
			pSh->ShWriteTimingInfoHeader(rb); //decodeframe.c
			pSh->ShWriteDecoderModelInfoPresentFlag(rb->AomRbReadBit());

			if (pSh->ShReadDecoderModelInfoPresentFlag())
				pSh->ShWriteDecoderModelInfo(rb);
		}
		else {
			pSh->ShWriteDecoderModelInfoPresentFlag(0);
		}
	//	seq_params->display_model_info_present_flag = aom_rb_read_bit(rb);
	//	seq_params->operating_points_cnt_minus_1 =
	//		aom_rb_read_literal(rb, OP_POINTS_CNT_MINUS_1_BITS);
	//	for (int i = 0; i < seq_params->operating_points_cnt_minus_1 + 1; i++) {
	//		seq_params->operating_point_idc[i] =
	//			aom_rb_read_literal(rb, OP_POINTS_IDC_BITS);
	//		if (!read_bitstream_level(&seq_params->level[i], rb)) {
	//			cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
	//			return 0;
	//		}
	//		// This is the seq_level_idx[i] > 7 check in the spec. seq_level_idx 7
	//		// is equivalent to level 3.3.
	//		if (seq_params->level[i].major > 3)
	//			seq_params->tier[i] = aom_rb_read_bit(rb);
	//		else
	//			seq_params->tier[i] = 0;
	//		if (seq_params->decoder_model_info_present_flag) {
	//			cm->op_params[i].decoder_model_param_present_flag = aom_rb_read_bit(rb);
	//			if (cm->op_params[i].decoder_model_param_present_flag)
	//				av1_read_op_parameters_info(cm, rb, i); //decodeframe.c
	//		}
	//		else {
	//			cm->op_params[i].decoder_model_param_present_flag = 0;
	//		}
	//		if (cm->timing_info_present &&
	//			(cm->timing_info.equal_picture_interval ||
	//				cm->op_params[i].decoder_model_param_present_flag)) {
	//			cm->op_params[i].bitrate = max_level_bitrate(
	//				seq_params->profile,
	//				major_minor_to_seq_level_idx(seq_params->level[i]), //onyxc_int.h
	//				seq_params->tier[i]);
	//			// Level with seq_level_idx = 31 returns a high "dummy" bitrate to pass
	//			// the check
	//			if (cm->op_params[i].bitrate == 0)
	//				aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//					"AV1 does not support this combination of "
	//					"profile, level, and tier.");
	//			// Buffer size in bits/s is bitrate in bits/s * 1 s
	//			cm->op_params[i].buffer_size = cm->op_params[i].bitrate;
	//		}
	//		if (cm->timing_info_present && cm->timing_info.equal_picture_interval &&
	//			!cm->op_params[i].decoder_model_param_present_flag) {
	//			// When the decoder_model_parameters are not sent for this op, set
	//			// the default ones that can be used with the resource availability mode
	//			cm->op_params[i].decoder_buffer_delay = 70000;
	//			cm->op_params[i].encoder_buffer_delay = 20000;
	//			cm->op_params[i].low_delay_mode_flag = 0;
	//		}

	//		if (seq_params->display_model_info_present_flag) {
	//			cm->op_params[i].display_model_param_present_flag = aom_rb_read_bit(rb);
	//			if (cm->op_params[i].display_model_param_present_flag) {
	//				cm->op_params[i].initial_display_delay =
	//					aom_rb_read_literal(rb, 4) + 1;
	//				if (cm->op_params[i].initial_display_delay > 10)
	//					aom_internal_error(
	//						&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//						"AV1 does not support more than 10 decoded frames delay");
	//			}
	//			else {
	//				cm->op_params[i].initial_display_delay = 10;
	//			}
	//		}
	//		else {
	//			cm->op_params[i].display_model_param_present_flag = 0;
	//			cm->op_params[i].initial_display_delay = 10;
	//		}
	//	}
	//}
	//// This decoder supports all levels.  Choose operating point provided by
	//// external means
	//int operating_point = pbi->operating_point;
	//if (operating_point < 0 ||
	//	operating_point > seq_params->operating_points_cnt_minus_1)
	//	operating_point = 0;
	//pbi->current_operating_point =
	//	seq_params->operating_point_idc[operating_point];
	//if (aom_get_num_layers_from_operating_point_idc( //obu.c
	//	pbi->current_operating_point, &cm->number_spatial_layers,
	//	&cm->number_temporal_layers) != AOM_CODEC_OK) {
	//	cm->error.error_code = AOM_CODEC_ERROR;
	//	return 0;
	}

	//av1_read_sequence_header(cm, rb, seq_params); //decodeframe.c

	//av1_read_color_config(rb, pbi->allow_lowbitdepth, seq_params, &cm->error); //decodeframe.c
	//if (!(seq_params->subsampling_x == 0 && seq_params->subsampling_y == 0) &&
	//	!(seq_params->subsampling_x == 1 && seq_params->subsampling_y == 1) &&
	//	!(seq_params->subsampling_x == 1 && seq_params->subsampling_y == 0)) {
	//	aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//		"Only 4:4:4, 4:2:2 and 4:2:0 are currently supported, "
	//		"%d %d subsampling is not supported.\n",
	//		seq_params->subsampling_x, seq_params->subsampling_y);
	//}

	//seq_params->film_grain_params_present = aom_rb_read_bit(rb);

	//if (av1_check_trailing_bits(pbi, rb) != 0) { //decoderframe.c
	//  // cm->error.error_code is already set.
	//	return 0;
	//}

	//// If a sequence header has been decoded before, we check if the new
	//// one is consistent with the old one.
	//if (pbi->sequence_header_ready) {
	//	if (!are_seq_headers_consistent(&cm->seq_params, seq_params)) { //obu.c
	//		aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//			"Inconsistent sequence headers received.");
	//	}
	//}

	//cm->seq_params = *seq_params;
	//pbi->sequence_header_ready = 1;

	//return ((rb->bit_offset - saved_bit_offset + 7) >> 3);

	return 1;
}