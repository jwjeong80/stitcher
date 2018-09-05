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
#include "aom/aom_codec.h"
#include "av1/decoder/decoder.h"
#include "OBUparser.h"
#include "aom_dsp/bitreader_buffer.h"
#include "aom_ports/mem_ops.h"

#include "av1/common/common.h"
#include "av1/common/timing.h"
#include "av1/decoder/decoder.h"
#include "av1/decoder/decodeframe.h"

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
	//m_PsManager.Init();
}

void COBUParser::Destroy()
{
	//m_PsManager.Destroy();
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



// Parses OBU header and stores values in 'header'.
aom_codec_err_t COBUParser::ReadObuHeader(struct aom_read_bit_buffer *rb,
	int is_annexb, ObuHeader *header) {
	if (!rb || !header) return AOM_CODEC_INVALID_PARAM;

	const ptrdiff_t bit_buffer_byte_length = rb->bit_buffer_end - rb->bit_buffer;
	if (bit_buffer_byte_length < 1) return AOM_CODEC_CORRUPT_FRAME;

	header->size = 1;

	if (aom_rb_read_bit(rb) != 0) {
		// Forbidden bit. Must not be set.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	header->type = (OBU_TYPE)aom_rb_read_literal(rb, 4);

	if (!valid_obu_type(header->type)) return AOM_CODEC_CORRUPT_FRAME;

	header->has_extension = aom_rb_read_bit(rb);
	header->has_size_field = aom_rb_read_bit(rb);

	if (!header->has_size_field && !is_annexb) {
		// section 5 obu streams must have obu_size field set.
		return AOM_CODEC_UNSUP_BITSTREAM;
	}

	if (aom_rb_read_bit(rb) != 0) {
		// obu_reserved_1bit must be set to 0.
		return AOM_CODEC_CORRUPT_FRAME;
	}

	if (header->has_extension) {
		if (bit_buffer_byte_length == 1) return AOM_CODEC_CORRUPT_FRAME;

		header->size += 1;
		header->temporal_layer_id = aom_rb_read_literal(rb, 3);
		header->spatial_layer_id = aom_rb_read_literal(rb, 2);
		if (aom_rb_read_literal(rb, 3) != 0) {
			// extension_header_reserved_3bits must be set to 0.
			return AOM_CODEC_CORRUPT_FRAME;
		}
	}

	return AOM_CODEC_OK;
}

aom_codec_err_t COBUParser::AomReadObuHeader(uint8_t *buffer, size_t buffer_length,
	size_t *consumed, ObuHeader *header,	int is_annexb) {
	if (buffer_length < 1 || !consumed || !header) return AOM_CODEC_INVALID_PARAM;

	// TODO(tomfinegan): Set the error handler here and throughout this file, and
	// confirm parsing work done via aom_read_bit_buffer is successful.
	struct aom_read_bit_buffer rb = { buffer, buffer + buffer_length, 0, NULL,
		NULL };
	aom_codec_err_t parse_result = ReadObuHeader(&rb, is_annexb, header);
	if (parse_result == AOM_CODEC_OK) *consumed = header->size;
	return parse_result;
}

aom_codec_err_t COBUParser::AomGetNumLayersFromOperatingPointIdc(
	int operating_point_idc, unsigned int *number_spatial_layers,
	unsigned int *number_temporal_layers) {
	// derive number of spatial/temporal layers from operating_point_idc

	if (!number_spatial_layers || !number_temporal_layers)
		return AOM_CODEC_INVALID_PARAM;

	if (operating_point_idc == 0) {
		*number_temporal_layers = 1;
		*number_spatial_layers = 1;
	}
	else {
		*number_spatial_layers = 0;
		*number_temporal_layers = 0;
		for (int j = 0; j < MAX_NUM_SPATIAL_LAYERS; j++) {
			*number_spatial_layers +=
				(operating_point_idc >> (j + MAX_NUM_TEMPORAL_LAYERS)) & 0x1;
		}
		for (int j = 0; j < MAX_NUM_TEMPORAL_LAYERS; j++) {
			*number_temporal_layers += (operating_point_idc >> j) & 0x1;
		}
	}

	return AOM_CODEC_OK;
}

int COBUParser::IsObuInCurrentOperatingPoint(AV1Decoder *pbi,
	ObuHeader obu_header) {
	if (!pbi->current_operating_point) {
		return 1;
	}

	if ((pbi->current_operating_point >> obu_header.temporal_layer_id) & 0x1 &&
		(pbi->current_operating_point >> (obu_header.spatial_layer_id + 8)) &
		0x1) {
		return 1;
	}
	return 0;
}

int COBUParser::ByteAlignment(AV1_COMMON *const cm,
	struct aom_read_bit_buffer *const rb) {
	while (rb->bit_offset & 7) {
		if (aom_rb_read_bit(rb)) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return -1;
		}
	}
	return 0;
}



// Returns a boolean that indicates success.
int COBUParser::ReadBitstreamLevel(BitstreamLevel *bl,
	struct aom_read_bit_buffer *rb) {
	const uint8_t seq_level_idx = aom_rb_read_literal(rb, LEVEL_BITS);
	if (!is_valid_seq_level_idx(seq_level_idx)) return 0;
	bl->major = (seq_level_idx >> LEVEL_MINOR_BITS) + LEVEL_MAJOR_MIN;
	bl->minor = seq_level_idx & ((1 << LEVEL_MINOR_BITS) - 1);
	return 1;
}

// Returns whether two sequence headers are consistent with each other.
// TODO(huisu,wtc@google.com): make sure the code matches the spec exactly.
int COBUParser::AreSeqHeadersConsistent(const SequenceHeader *seq_params_old,
	const SequenceHeader *seq_params_new) {
	return !memcmp(seq_params_old, seq_params_new, sizeof(SequenceHeader));
}

// On success, sets pbi->sequence_header_ready to 1 and returns the number of
// bytes read from 'rb'.
// On failure, sets pbi->common.error.error_code and returns 0.
uint32_t COBUParser::ReadSequenceHeaderObu(AV1Decoder *pbi,
	struct aom_read_bit_buffer *rb) {
	AV1_COMMON *const cm = &pbi->common;
	const uint32_t saved_bit_offset = rb->bit_offset;

	// Verify rb has been configured to report errors.
	assert(rb->error_handler);

	// Use a local variable to store the information as we decode. At the end,
	// if no errors have occurred, cm->seq_params is updated.
	SequenceHeader sh = cm->seq_params;
	SequenceHeader *const seq_params = &sh;

	seq_params->profile = av1_read_profile(rb);
	if (seq_params->profile > CONFIG_MAX_DECODE_PROFILE) {
		cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
		return 0;
	}

	// Still picture or not
	seq_params->still_picture = aom_rb_read_bit(rb);
	seq_params->reduced_still_picture_hdr = aom_rb_read_bit(rb);
	// Video must have reduced_still_picture_hdr = 0
	if (!seq_params->still_picture && seq_params->reduced_still_picture_hdr) {
		cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
		return 0;
	}

	if (seq_params->reduced_still_picture_hdr) {
		cm->timing_info_present = 0;
		seq_params->decoder_model_info_present_flag = 0;
		seq_params->display_model_info_present_flag = 0;
		seq_params->operating_points_cnt_minus_1 = 0;
		seq_params->operating_point_idc[0] = 0;
		if (!ReadBitstreamLevel(&seq_params->level[0], rb)) {
			cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
			return 0;
		}
		seq_params->tier[0] = 0;
		cm->op_params[0].decoder_model_param_present_flag = 0;
		cm->op_params[0].display_model_param_present_flag = 0;
	}
	else {
		cm->timing_info_present = aom_rb_read_bit(rb);  // timing_info_present_flag
		if (cm->timing_info_present) {
			av1_read_timing_info_header(cm, rb);

			seq_params->decoder_model_info_present_flag = aom_rb_read_bit(rb);
			if (seq_params->decoder_model_info_present_flag)
				av1_read_decoder_model_info(cm, rb);
		}
		else {
			seq_params->decoder_model_info_present_flag = 0;
		}
		seq_params->display_model_info_present_flag = aom_rb_read_bit(rb);
		seq_params->operating_points_cnt_minus_1 =
			aom_rb_read_literal(rb, OP_POINTS_CNT_MINUS_1_BITS);
		for (int i = 0; i < seq_params->operating_points_cnt_minus_1 + 1; i++) {
			seq_params->operating_point_idc[i] =
				aom_rb_read_literal(rb, OP_POINTS_IDC_BITS);
			if (!ReadBitstreamLevel(&seq_params->level[i], rb)) {
				cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
				return 0;
			}
			// This is the seq_level_idx[i] > 7 check in the spec. seq_level_idx 7
			// is equivalent to level 3.3.
			if (seq_params->level[i].major > 3)
				seq_params->tier[i] = aom_rb_read_bit(rb);
			else
				seq_params->tier[i] = 0;
			if (seq_params->decoder_model_info_present_flag) {
				cm->op_params[i].decoder_model_param_present_flag = aom_rb_read_bit(rb);
				if (cm->op_params[i].decoder_model_param_present_flag)
					av1_read_op_parameters_info(cm, rb, i);
			}
			else {
				cm->op_params[i].decoder_model_param_present_flag = 0;
			}
			if (cm->timing_info_present &&
				(cm->timing_info.equal_picture_interval ||
					cm->op_params[i].decoder_model_param_present_flag)) {
				cm->op_params[i].bitrate = max_level_bitrate(
					seq_params->profile,
					major_minor_to_seq_level_idx(seq_params->level[i]),
					seq_params->tier[i]);
				// Level with seq_level_idx = 31 returns a high "dummy" bitrate to pass
				// the check
				if (cm->op_params[i].bitrate == 0)
					aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
						"AV1 does not support this combination of "
						"profile, level, and tier.");
				// Buffer size in bits/s is bitrate in bits/s * 1 s
				cm->op_params[i].buffer_size = cm->op_params[i].bitrate;
			}
			if (cm->timing_info_present && cm->timing_info.equal_picture_interval &&
				!cm->op_params[i].decoder_model_param_present_flag) {
				// When the decoder_model_parameters are not sent for this op, set
				// the default ones that can be used with the resource availability mode
				cm->op_params[i].decoder_buffer_delay = 70000;
				cm->op_params[i].encoder_buffer_delay = 20000;
				cm->op_params[i].low_delay_mode_flag = 0;
			}

			if (seq_params->display_model_info_present_flag) {
				cm->op_params[i].display_model_param_present_flag = aom_rb_read_bit(rb);
				if (cm->op_params[i].display_model_param_present_flag) {
					cm->op_params[i].initial_display_delay =
						aom_rb_read_literal(rb, 4) + 1;
					if (cm->op_params[i].initial_display_delay > 10)
						aom_internal_error(
							&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
							"AV1 does not support more than 10 decoded frames delay");
				}
				else {
					cm->op_params[i].initial_display_delay = 10;
				}
			}
			else {
				cm->op_params[i].display_model_param_present_flag = 0;
				cm->op_params[i].initial_display_delay = 10;
			}
		}
	}
	// This decoder supports all levels.  Choose operating point provided by
	// external means
	int operating_point = pbi->operating_point;
	if (operating_point < 0 ||
		operating_point > seq_params->operating_points_cnt_minus_1)
		operating_point = 0;
	pbi->current_operating_point =
		seq_params->operating_point_idc[operating_point];
	if (AomGetNumLayersFromOperatingPointIdc(
		pbi->current_operating_point, &cm->number_spatial_layers,
		&cm->number_temporal_layers) != AOM_CODEC_OK) {
		cm->error.error_code = AOM_CODEC_ERROR;
		return 0;
	}

	av1_read_sequence_header(cm, rb, seq_params);

	av1_read_color_config(rb, pbi->allow_lowbitdepth, seq_params, &cm->error);
	if (!(seq_params->subsampling_x == 0 && seq_params->subsampling_y == 0) &&
		!(seq_params->subsampling_x == 1 && seq_params->subsampling_y == 1) &&
		!(seq_params->subsampling_x == 1 && seq_params->subsampling_y == 0)) {
		aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
			"Only 4:4:4, 4:2:2 and 4:2:0 are currently supported, "
			"%d %d subsampling is not supported.\n",
			seq_params->subsampling_x, seq_params->subsampling_y);
	}

	seq_params->film_grain_params_present = aom_rb_read_bit(rb);

	if (av1_check_trailing_bits(pbi, rb) != 0) {
		// cm->error.error_code is already set.
		return 0;
	}

	// If a sequence header has been decoded before, we check if the new
	// one is consistent with the old one.
	if (pbi->sequence_header_ready) {
		if (!AreSeqHeadersConsistent(&cm->seq_params, seq_params)) {
			aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
				"Inconsistent sequence headers received.");
		}
	}

	cm->seq_params = *seq_params;
	pbi->sequence_header_ready = 1;

	return ((rb->bit_offset - saved_bit_offset + 7) >> 3);
}

// On success, returns the frame header size. On failure, calls
// aom_internal_error and does not return.
uint32_t COBUParser::ReadFrameHeaderObu(AV1Decoder *pbi,
	struct aom_read_bit_buffer *rb,
	const uint8_t *data,
	const uint8_t **p_data_end,
	int trailing_bits_present) {
	return av1_decode_frame_headers_and_setup(pbi, rb, data, p_data_end,
		trailing_bits_present);
}

//int32_t COBUParser::ReadTileGroupHeader(AV1Decoder *pbi,
//	struct aom_read_bit_buffer *rb,
//	int *start_tile, int *end_tile,
//	int tile_start_implicit) {
//	AV1_COMMON *const cm = &pbi->common;
//	uint32_t saved_bit_offset = rb->bit_offset;
//	int tile_start_and_end_present_flag = 0;
//	const int num_tiles = pbi->common.tile_rows * pbi->common.tile_cols;
//
//	if (!pbi->common.large_scale_tile && num_tiles > 1) {
//		tile_start_and_end_present_flag = aom_rb_read_bit(rb);
//	}
//	if (pbi->common.large_scale_tile || num_tiles == 1 ||
//		!tile_start_and_end_present_flag) {
//		*start_tile = 0;
//		*end_tile = num_tiles - 1;
//		return ((rb->bit_offset - saved_bit_offset + 7) >> 3);
//	}
//	if (tile_start_implicit && tile_start_and_end_present_flag) {
//		aom_internal_error(
//			&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
//			"For OBU_FRAME type obu tile_start_and_end_present_flag must be 0");
//		return -1;
//	}
//	*start_tile =
//		aom_rb_read_literal(rb, cm->log2_tile_rows + cm->log2_tile_cols);
//	*end_tile = aom_rb_read_literal(rb, cm->log2_tile_rows + cm->log2_tile_cols);
//
//	return ((rb->bit_offset - saved_bit_offset + 7) >> 3);
//}
//
//uint32_t COBUParser::ReadOneTileGroupObu(
//	AV1Decoder *pbi, struct aom_read_bit_buffer *rb, int is_first_tg,
//	const uint8_t *data, const uint8_t *data_end, const uint8_t **p_data_end,
//	int *is_last_tg, int tile_start_implicit) {
//	AV1_COMMON *const cm = &pbi->common;
//	int start_tile, end_tile;
//	int32_t header_size, tg_payload_size;
//
//	assert((rb->bit_offset & 7) == 0);
//	assert(rb->bit_buffer + aom_rb_bytes_read(rb) == data);
//
//	//header_size = ReadTileGroupHeader(pbi, rb, &start_tile, &end_tile,
//		tile_start_implicit);
//	if (header_size == -1 || ByteAlignment(cm, rb)) return 0;
//	if (start_tile > end_tile) return header_size;
//	data += header_size;
//	av1_decode_tg_tiles_and_wrapup(pbi, data, data_end, p_data_end, start_tile,
//		end_tile, is_first_tg);
//
//	tg_payload_size = (uint32_t)(*p_data_end - data);
//
//	// TODO(shan):  For now, assume all tile groups received in order
//	*is_last_tg = end_tile == cm->tile_rows * cm->tile_cols - 1;
//	return header_size + tg_payload_size;
//}

void COBUParser::AllocTileListBuffer(AV1Decoder *pbi) {
	// TODO(yunqing): for now, copy each tile's decoded YUV data directly to the
	// output buffer. This needs to be modified according to the application
	// requirement.
	AV1_COMMON *const cm = &pbi->common;
	const int tile_width_in_pixels = cm->tile_width * MI_SIZE;
	const int tile_height_in_pixels = cm->tile_height * MI_SIZE;
	const int ssy = cm->seq_params.subsampling_y;
	const int ssx = cm->seq_params.subsampling_x;
	const int num_planes = av1_num_planes(cm);
	const size_t yplane_tile_size = tile_height_in_pixels * tile_width_in_pixels;
	const size_t uvplane_tile_size =
		(num_planes > 1)
		? (tile_height_in_pixels >> ssy) * (tile_width_in_pixels >> ssx)
		: 0;
	const size_t tile_size = (cm->seq_params.use_highbitdepth ? 2 : 1) *
		(yplane_tile_size + 2 * uvplane_tile_size);
	pbi->tile_list_size = tile_size * (pbi->tile_count_minus_1 + 1);

	if (pbi->tile_list_size > pbi->buffer_sz) {
		if (pbi->tile_list_output != NULL) aom_free(pbi->tile_list_output);
		pbi->tile_list_output = NULL;

		pbi->tile_list_output = (uint8_t *)aom_memalign(32, pbi->tile_list_size);
		if (pbi->tile_list_output == NULL)
			aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR,
				"Failed to allocate the tile list output buffer");
		pbi->buffer_sz = pbi->tile_list_size;
	}
}

void COBUParser::CopyDecodedTileToTileListBuffer(AV1Decoder *pbi,
	uint8_t **output) {
	AV1_COMMON *const cm = &pbi->common;
	const int tile_width_in_pixels = cm->tile_width * MI_SIZE;
	const int tile_height_in_pixels = cm->tile_height * MI_SIZE;
	const int ssy = cm->seq_params.subsampling_y;
	const int ssx = cm->seq_params.subsampling_x;
	const int num_planes = av1_num_planes(cm);

	// Copy decoded tile to the tile list output buffer.
	YV12_BUFFER_CONFIG *cur_frame = get_frame_new_buffer(cm);
	const int mi_row = pbi->dec_tile_row * cm->tile_height;
	const int mi_col = pbi->dec_tile_col * cm->tile_width;
	const int is_hbd = (cur_frame->flags & YV12_FLAG_HIGHBITDEPTH) ? 1 : 0;
	uint8_t *bufs[MAX_MB_PLANE] = { NULL, NULL, NULL };
	int strides[MAX_MB_PLANE] = { 0, 0, 0 };
	int plane;

	for (plane = 0; plane < num_planes; ++plane) {
		int shift_x = plane > 0 ? ssx : 0;
		int shift_y = plane > 0 ? ssy : 0;

		bufs[plane] = cur_frame->buffers[plane];
		strides[plane] =
			(plane > 0) ? cur_frame->strides[1] : cur_frame->strides[0];

		bufs[plane] += mi_row * (MI_SIZE >> shift_y) * strides[plane] +
			mi_col * (MI_SIZE >> shift_x);

		if (is_hbd) {
			bufs[plane] = (uint8_t *)CONVERT_TO_SHORTPTR(bufs[plane]);
			strides[plane] *= 2;
		}

		int w, h;
		w = (plane > 0 && shift_x > 0) ? ((tile_width_in_pixels + 1) >> shift_x)
			: tile_width_in_pixels;
		w *= (1 + is_hbd);
		h = (plane > 0 && shift_y > 0) ? ((tile_height_in_pixels + 1) >> shift_y)
			: tile_height_in_pixels;
		int j;

		for (j = 0; j < h; ++j) {
			memcpy(*output, bufs[plane], w);
			bufs[plane] += strides[plane];
			*output += w;
		}
	}
}

// Only called while large_scale_tile = 1.
uint32_t COBUParser::ReadAndDecodeOneTileList(AV1Decoder *pbi,
	struct aom_read_bit_buffer *rb,
	const uint8_t *data,
	const uint8_t *data_end,
	const uint8_t **p_data_end,
	int *frame_decoding_finished) {
	AV1_COMMON *const cm = &pbi->common;
	uint32_t tile_list_payload_size = 0;
	const int num_tiles = cm->tile_cols * cm->tile_rows;
	const int start_tile = 0;
	const int end_tile = num_tiles - 1;
	int i = 0;

	// Process the tile list info.
	pbi->output_frame_width_in_tiles_minus_1 = aom_rb_read_literal(rb, 8);
	pbi->output_frame_height_in_tiles_minus_1 = aom_rb_read_literal(rb, 8);
	pbi->tile_count_minus_1 = aom_rb_read_literal(rb, 16);
	if (pbi->tile_count_minus_1 > MAX_TILES - 1) {
		cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
		return 0;
	}

	// Allocate output frame buffer for the tile list.
	AllocTileListBuffer(pbi);

	uint32_t tile_list_info_bytes = 4;
	tile_list_payload_size += tile_list_info_bytes;
	data += tile_list_info_bytes;
	uint8_t *output = pbi->tile_list_output;

	for (i = 0; i <= pbi->tile_count_minus_1; i++) {
		// Process 1 tile.
		// Reset the bit reader.
		rb->bit_offset = 0;
		rb->bit_buffer = data;

		// Read out the tile info.
		uint32_t tile_info_bytes = 5;
		// Set reference for each tile.
		int ref_idx = aom_rb_read_literal(rb, 8);
		if (ref_idx >= MAX_EXTERNAL_REFERENCES) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return 0;
		}
		av1_set_reference_dec(cm, 0, 1, &pbi->ext_refs.refs[ref_idx]);

		pbi->dec_tile_row = aom_rb_read_literal(rb, 8);
		pbi->dec_tile_col = aom_rb_read_literal(rb, 8);
		if (pbi->dec_tile_row < 0 || pbi->dec_tile_col < 0 ||
			pbi->dec_tile_row >= cm->tile_rows ||
			pbi->dec_tile_col >= cm->tile_cols) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return 0;
		}

		pbi->coded_tile_data_size = aom_rb_read_literal(rb, 16) + 1;
		data += tile_info_bytes;
		if ((size_t)(data_end - data) < pbi->coded_tile_data_size) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return 0;
		}

		av1_decode_tg_tiles_and_wrapup(pbi, data, data + pbi->coded_tile_data_size,
			p_data_end, start_tile, end_tile, 0);
		uint32_t tile_payload_size = (uint32_t)(*p_data_end - data);

		tile_list_payload_size += tile_info_bytes + tile_payload_size;

		// Update data ptr for next tile decoding.
		data = *p_data_end;
		assert(data <= data_end);

		// Copy the decoded tile to the tile list output buffer.
		CopyDecodedTileToTileListBuffer(pbi, &output);
	}

	*frame_decoding_finished = 1;
	return tile_list_payload_size;
}

void COBUParser::ReadMetadataItutT35(const uint8_t *data, size_t sz) {
	struct aom_read_bit_buffer rb = { data, data + sz, 0, NULL, NULL };
	for (size_t i = 0; i < sz; i++) {
		aom_rb_read_literal(&rb, 8);
	}
}

void COBUParser::ReadMetadataHdrCll(const uint8_t *data, size_t sz) {
	struct aom_read_bit_buffer rb = { data, data + sz, 0, NULL, NULL };
	aom_rb_read_literal(&rb, 16);  // max_cll
	aom_rb_read_literal(&rb, 16);  // max_fall
}

void COBUParser::ReadMetadataHdrMdcv(const uint8_t *data, size_t sz) {
	struct aom_read_bit_buffer rb = { data, data + sz, 0, NULL, NULL };
	for (int i = 0; i < 3; i++) {
		aom_rb_read_literal(&rb, 16);  // primary_i_chromaticity_x
		aom_rb_read_literal(&rb, 16);  // primary_i_chromaticity_y
	}

	aom_rb_read_literal(&rb, 16);  // white_point_chromaticity_x
	aom_rb_read_literal(&rb, 16);  // white_point_chromaticity_y

	aom_rb_read_unsigned_literal(&rb, 32);  // luminance_max
	aom_rb_read_unsigned_literal(&rb, 32);  // luminance_min
}

void COBUParser::ScalabilityStructure(struct aom_read_bit_buffer *rb) {
	int spatial_layers_cnt = aom_rb_read_literal(rb, 2);
	int spatial_layer_dimensions_present_flag = aom_rb_read_bit(rb);
	int spatial_layer_description_present_flag = aom_rb_read_bit(rb);
	int temporal_group_description_present_flag = aom_rb_read_bit(rb);
	aom_rb_read_literal(rb, 3);  // reserved

	if (spatial_layer_dimensions_present_flag) {
		int i;
		for (i = 0; i < spatial_layers_cnt + 1; i++) {
			aom_rb_read_literal(rb, 16);
			aom_rb_read_literal(rb, 16);
		}
	}
	if (spatial_layer_description_present_flag) {
		int i;
		for (i = 0; i < spatial_layers_cnt + 1; i++) {
			aom_rb_read_literal(rb, 8);
		}
	}
	if (temporal_group_description_present_flag) {
		int i, j, temporal_group_size;
		temporal_group_size = aom_rb_read_literal(rb, 8);
		for (i = 0; i < temporal_group_size; i++) {
			aom_rb_read_literal(rb, 3);
			aom_rb_read_bit(rb);
			aom_rb_read_bit(rb);
			int temporal_group_ref_cnt = aom_rb_read_literal(rb, 3);
			for (j = 0; j < temporal_group_ref_cnt; j++) {
				aom_rb_read_literal(rb, 8);
			}
		}
	}
}

void COBUParser::ReadMetadataScalability(const uint8_t *data, size_t sz) {
	struct aom_read_bit_buffer rb = { data, data + sz, 0, NULL, NULL };
	int scalability_mode_idc = aom_rb_read_literal(&rb, 8);
	if (scalability_mode_idc == SCALABILITY_SS) {
		ScalabilityStructure(&rb);
	}
}

void COBUParser::ReadMetadataTimecode(const uint8_t *data, size_t sz) {
	struct aom_read_bit_buffer rb = { data, data + sz, 0, NULL, NULL };
	aom_rb_read_literal(&rb, 5);                     // counting_type f(5)
	int full_timestamp_flag = aom_rb_read_bit(&rb);  // full_timestamp_flag f(1)
	aom_rb_read_bit(&rb);                            // discontinuity_flag (f1)
	aom_rb_read_bit(&rb);                            // cnt_dropped_flag f(1)
	aom_rb_read_literal(&rb, 9);                     // n_frames f(9)
	if (full_timestamp_flag) {
		aom_rb_read_literal(&rb, 6);  // seconds_value f(6)
		aom_rb_read_literal(&rb, 6);  // minutes_value f(6)
		aom_rb_read_literal(&rb, 5);  // hours_value f(5)
	}
	else {
		int seconds_flag = aom_rb_read_bit(&rb);  // seconds_flag f(1)
		if (seconds_flag) {
			aom_rb_read_literal(&rb, 6);              // seconds_value f(6)
			int minutes_flag = aom_rb_read_bit(&rb);  // minutes_flag f(1)
			if (minutes_flag) {
				aom_rb_read_literal(&rb, 6);            // minutes_value f(6)
				int hours_flag = aom_rb_read_bit(&rb);  // hours_flag f(1)
				if (hours_flag) {
					aom_rb_read_literal(&rb, 5);  // hours_value f(5)
				}
			}
		}
	}
	// time_offset_length f(5)
	int time_offset_length = aom_rb_read_literal(&rb, 5);
	if (time_offset_length) {
		aom_rb_read_literal(&rb, time_offset_length);  // f(time_offset_length)
	}
}

size_t COBUParser::ReadMetadata(const uint8_t *data, size_t sz) {
	size_t type_length;
	uint64_t type_value;
	OBU_METADATA_TYPE metadata_type;
	if (aom_uleb_decode(data, sz, &type_value, &type_length) < 0) {
		return sz;
	}
	metadata_type = (OBU_METADATA_TYPE)type_value;
	if (metadata_type == OBU_METADATA_TYPE_ITUT_T35) {
		ReadMetadataItutT35(data + type_length, sz - type_length);
	}
	else if (metadata_type == OBU_METADATA_TYPE_HDR_CLL) {
		ReadMetadataHdrCll(data + type_length, sz - type_length);
	}
	else if (metadata_type == OBU_METADATA_TYPE_HDR_MDCV) {
		ReadMetadataHdrMdcv(data + type_length, sz - type_length);
	}
	else if (metadata_type == OBU_METADATA_TYPE_SCALABILITY) {
		ReadMetadataScalability(data + type_length, sz - type_length);
	}
	else if (metadata_type == OBU_METADATA_TYPE_TIMECODE) {
		ReadMetadataTimecode(data + type_length, sz - type_length);
	}

	return sz;
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

bool COBUParser::ParseObuHeader(uint8_t obu_header_byte, ObuHeader *obu_header) {
	const int forbidden_bit =
		(obu_header_byte >> kObuForbiddenBitShift) & kObuForbiddenBitMask;
	if (forbidden_bit) {
		fprintf(stderr, "Invalid OBU, forbidden bit set.\n");
		return false;
	}

	obu_header->type = static_cast<OBU_TYPE>(
		(obu_header_byte >> kObuTypeBitsShift) & kObuTypeBitsMask);
	if (!ValidObuType(obu_header->type)) {
		fprintf(stderr, "Invalid OBU type: %d.\n", obu_header->type);
		return false;
	}

	obu_header->has_extension =
		(obu_header_byte >> kObuExtensionFlagBitShift) & kObuExtensionFlagBitMask;
	obu_header->has_size_field =
		(obu_header_byte >> kObuHasSizeFieldBitShift) & kObuHasSizeFieldBitMask;
	return true;
}

bool COBUParser::ParseObuExtensionHeader(uint8_t ext_header_byte, ObuHeader *obu_header) {
	obu_header->temporal_layer_id =
		(ext_header_byte >> kObuExtTemporalIdBitsShift) &
		kObuExtTemporalIdBitsMask;
	obu_header->spatial_layer_id =
		(ext_header_byte >> kObuExtSpatialIdBitsShift) & kObuExtSpatialIdBitsMask;

	return true;
}

//void COBUParser::PrintObuHeader(const ObuHeader *header) {
//	printf(
//		"  OBU type:        %s\n"
//		"      extension:   %s\n",
//		aom_obu_type_to_string(static_cast<OBU_TYPE>(header->type)),
//		header->has_extension ? "yes" : "no");
//	if (header->has_extension) {
//		printf(
//			"      temporal_id: %d\n"
//			"      spatial_id:  %d\n",
//			header->temporal_layer_id, header->temporal_layer_id);
//	}
//}
//
//bool COBUParser::DumpObu(const uint8_t *data, int length, int *obu_overhead_bytes) {
//	const int kObuHeaderSizeBytes = 1;
//	const int kMinimumBytesRequired = 1 + kObuHeaderSizeBytes;
//	int consumed = 0;
//	int obu_overhead = 0;
//	ObuHeader obu_header;
//	while (consumed < length) {
//		const int remaining = length - consumed;
//		if (remaining < kMinimumBytesRequired) {
//			fprintf(stderr,
//				"OBU parse error. Did not consume all data, %d bytes remain.\n",
//				remaining);
//			return false;
//		}
//
//		int obu_header_size = 0;
//
//		memset(&obu_header, 0, sizeof(obu_header));
//		const uint8_t obu_header_byte = *(data + consumed);
//		if (!ParseObuHeader(obu_header_byte, &obu_header)) {
//			fprintf(stderr, "OBU parsing failed at offset %d.\n", consumed);
//			return false;
//		}
//
//		++obu_overhead;
//		++obu_header_size;
//
//		if (obu_header.has_extension) {
//			const uint8_t obu_ext_header_byte =
//				*(data + consumed + kObuHeaderSizeBytes);
//			if (!ParseObuExtensionHeader(obu_ext_header_byte, &obu_header)) {
//				fprintf(stderr, "OBU extension parsing failed at offset %d.\n",
//					consumed + kObuHeaderSizeBytes);
//				return false;
//			}
//
//			++obu_overhead;
//			++obu_header_size;
//		}
//
//		PrintObuHeader(&obu_header);
//
//		uint64_t obu_size = 0;
//		size_t length_field_size = 0;
//		if (aom_uleb_decode(data + consumed + obu_header_size,
//			remaining - obu_header_size, &obu_size,
//			&length_field_size) != 0) {
//			fprintf(stderr, "OBU size parsing failed at offset %d.\n",
//				consumed + obu_header_size);
//			return false;
//		}
//		int current_obu_length = static_cast<int>(obu_size);
//		if (obu_header_size + static_cast<int>(length_field_size) +
//			current_obu_length >
//			remaining) {
//			fprintf(stderr, "OBU parsing failed: not enough OBU data.\n");
//			return false;
//		}
//		consumed += obu_header_size + static_cast<int>(length_field_size) +
//			current_obu_length;
//		printf("      length:      %d\n",
//			static_cast<int>(obu_header_size + length_field_size +
//				current_obu_length));
//	}
//
//	if (obu_overhead_bytes != nullptr) *obu_overhead_bytes = obu_overhead;
//	printf("  TU size: %d\n", consumed);
//
//	return true;
//}


// On success, returns a boolean that indicates whether the decoding of the
// current frame is finished. On failure, sets cm->error.error_code and
// returns -1.
int COBUParser::AomDecodeFrameFromObus(struct AV1Decoder *pbi, const uint8_t *data,
	const uint8_t *data_end,
	const uint8_t **p_data_end) {
	AV1_COMMON *const cm = &pbi->common;
	int frame_decoding_finished = 0;
	int is_first_tg_obu_received = 1;
	uint32_t frame_header_size = 0;
	int seq_header_received = 0;
	size_t seq_header_size = 0;
	ObuHeader obu_header;
	memset(&obu_header, 0, sizeof(obu_header));
	pbi->seen_frame_header = 0;

	if (data_end < data) {
		cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
		return -1;
	}

	// Reset pbi->camera_frame_header_ready to 0 if cm->large_scale_tile = 0.
	if (!cm->large_scale_tile) pbi->camera_frame_header_ready = 0;

	// decode frame as a series of OBUs
	while (!frame_decoding_finished && !cm->error.error_code) {
		struct aom_read_bit_buffer rb;
		size_t payload_size = 0;
		size_t decoded_payload_size = 0;
		size_t obu_payload_offset = 0;
		size_t bytes_read = 0;
		const size_t bytes_available = data_end - data;

		if (bytes_available == 0 && !pbi->seen_frame_header) {
			*p_data_end = data;
			cm->error.error_code = AOM_CODEC_OK;
			break;
		}

		aom_codec_err_t status =
			aom_read_obu_header_and_size(data, bytes_available, cm->is_annexb,
				&obu_header, &payload_size, &bytes_read);

		if (status != AOM_CODEC_OK) {
			cm->error.error_code = status;
			return -1;
		}

		// Record obu size header information.
		pbi->obu_size_hdr.data = data + obu_header.size;
		pbi->obu_size_hdr.size = bytes_read - obu_header.size;

		// Note: aom_read_obu_header_and_size() takes care of checking that this
		// doesn't cause 'data' to advance past 'data_end'.
		data += bytes_read;

		if ((size_t)(data_end - data) < payload_size) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return -1;
		}

		cm->temporal_layer_id = obu_header.temporal_layer_id;
		cm->spatial_layer_id = obu_header.spatial_layer_id;

		if (obu_header.type != OBU_TEMPORAL_DELIMITER &&
			obu_header.type != OBU_SEQUENCE_HEADER &&
			obu_header.type != OBU_PADDING) {
			// don't decode obu if it's not in current operating mode
			if (!IsObuInCurrentOperatingPoint(pbi, obu_header)) {
				data += payload_size;
				continue;
			}
		}

		av1_init_read_bit_buffer(pbi, &rb, data, data + payload_size);

		switch (obu_header.type) {
		case OBU_TEMPORAL_DELIMITER:
			decoded_payload_size = ReadTemporalDelimiterObu();
			pbi->seen_frame_header = 0;
			break;
		case OBU_SEQUENCE_HEADER:
			if (!seq_header_received) {
				decoded_payload_size = ReadSequenceHeaderObu(pbi, &rb);
				if (cm->error.error_code != AOM_CODEC_OK) return -1;

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
			if (!pbi->seen_frame_header ||
				(cm->large_scale_tile && !pbi->camera_frame_header_ready)) {
				frame_header_size = ReadFrameHeaderObu(
					pbi, &rb, data, p_data_end, obu_header.type != OBU_FRAME);
				pbi->seen_frame_header = 1;
				if (!pbi->ext_tile_debug && cm->large_scale_tile)
					pbi->camera_frame_header_ready = 1;
			}
			else {
				// TODO(wtc): Verify that the frame_header_obu is identical to the
				// original frame_header_obu. For now just skip frame_header_size
				// bytes in the bit buffer.
				if (frame_header_size > payload_size) {
					cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
					return -1;
				}
				assert(rb.bit_offset == 0);
				rb.bit_offset = 8 * frame_header_size;
			}
			decoded_payload_size = frame_header_size;
			pbi->frame_header_size = frame_header_size;

			if (cm->show_existing_frame) {
				if (obu_header.type == OBU_FRAME) {
					cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
					return -1;
				}
				frame_decoding_finished = 1;
				pbi->seen_frame_header = 0;
				break;
			}

			// In large scale tile coding, decode the common camera frame header
			// before any tile list OBU.
			if (!pbi->ext_tile_debug && pbi->camera_frame_header_ready) {
				frame_decoding_finished = 1;
				// Skip the rest of the frame data.
				decoded_payload_size = payload_size;
				// Update data_end.
				*p_data_end = data_end;
				break;
			}

			if (obu_header.type != OBU_FRAME) break;
			obu_payload_offset = frame_header_size;
			// Byte align the reader before reading the tile group.
			if (ByteAlignment(cm, &rb)) return -1;
			AOM_FALLTHROUGH_INTENDED;  // fall through to read tile group.
		case OBU_TILE_GROUP:
			if (!pbi->seen_frame_header) {
				cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
				return -1;
			}
			if (obu_payload_offset > payload_size) {
				cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
				return -1;
			}
			//decoded_payload_size += ReadOneTileGroupObu(
			//	pbi, &rb, is_first_tg_obu_received, data + obu_payload_offset,
			//	data + payload_size, p_data_end, &frame_decoding_finished,
			//	obu_header.type == OBU_FRAME);
			//is_first_tg_obu_received = 0;
			if (frame_decoding_finished) pbi->seen_frame_header = 0;
			break;
		case OBU_METADATA:
			decoded_payload_size = ReadMetadata(data, payload_size);
			break;
		case OBU_TILE_LIST:
			if (CONFIG_NORMAL_TILE_MODE) {
				cm->error.error_code = AOM_CODEC_UNSUP_BITSTREAM;
				return -1;
			}

			// This OBU type is purely for the large scale tile coding mode.
			// The common camera frame header has to be already decoded.
			if (!pbi->camera_frame_header_ready) {
				cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
				return -1;
			}

			cm->large_scale_tile = 1;
			av1_set_single_tile_decoding_mode(cm);
			decoded_payload_size =
				ReadAndDecodeOneTileList(pbi, &rb, data, data + payload_size,
					p_data_end, &frame_decoding_finished);
			if (cm->error.error_code != AOM_CODEC_OK) return -1;
			break;
		case OBU_PADDING:
		default:
			// Skip unrecognized OBUs
			decoded_payload_size = payload_size;
			break;
		}

		// Check that the signalled OBU size matches the actual amount of data read
		if (decoded_payload_size > payload_size) {
			cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
			return -1;
		}

		// If there are extra padding bytes, they should all be zero
		while (decoded_payload_size < payload_size) {
			uint8_t padding_byte = data[decoded_payload_size++];
			if (padding_byte != 0) {
				cm->error.error_code = AOM_CODEC_CORRUPT_FRAME;
				return -1;
			}
		}

		data += payload_size;
	}

	return frame_decoding_finished;
}