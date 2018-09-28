///*
// * Copyright (c) 2016, Alliance for Open Media. All rights reserved
// *
// * This source code is subject to the terms of the BSD 2 Clause License and
// * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
// * was not distributed with this source code in the LICENSE file, you can
// * obtain it at www.aomedia.org/license/software. If the Alliance for Open
// * Media Patent License 1.0 was not distributed with this source code in the
// * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
// */
//
//#include <assert.h>
//#include <limits.h>
//#include <stdio.h>
//
//#include "bitstream_writer.h"
//
//uint32_t write_sequence_header_obu(uint8_t *const dst) {
//	AV1_COMMON *const cm = &cpi->common;
//	struct aom_write_bit_buffer wb = { dst, 0 };
//	uint32_t size = 0;
//
//	write_profile(cm->seq_params.profile, &wb);
//
//	// Still picture or not
//	aom_wb_write_bit(&wb, cm->seq_params.still_picture);
//	assert(IMPLIES(!cm->seq_params.still_picture,
//		!cm->seq_params.reduced_still_picture_hdr));
//	// whether to use reduced still picture header
//	aom_wb_write_bit(&wb, cm->seq_params.reduced_still_picture_hdr);
//
//	if (cm->seq_params.reduced_still_picture_hdr) {
//		assert(cm->timing_info_present == 0);
//		assert(cm->seq_params.decoder_model_info_present_flag == 0);
//		assert(cm->seq_params.display_model_info_present_flag == 0);
//		write_bitstream_level(cm->seq_params.level[0], &wb);
//	}
//	else {
//		aom_wb_write_bit(&wb, cm->timing_info_present);  // timing info present flag
//
//		if (cm->timing_info_present) {
//			// timing_info
//			write_timing_info_header(cm, &wb);
//			aom_wb_write_bit(&wb, cm->seq_params.decoder_model_info_present_flag);
//			if (cm->seq_params.decoder_model_info_present_flag) {
//				write_decoder_model_info(cm, &wb);
//			}
//		}
//		aom_wb_write_bit(&wb, cm->seq_params.display_model_info_present_flag);
//		aom_wb_write_literal(&wb, cm->seq_params.operating_points_cnt_minus_1,
//			OP_POINTS_CNT_MINUS_1_BITS);
//		int i;
//		for (i = 0; i < cm->seq_params.operating_points_cnt_minus_1 + 1; i++) {
//			aom_wb_write_literal(&wb, cm->seq_params.operating_point_idc[i],
//				OP_POINTS_IDC_BITS);
//			write_bitstream_level(cm->seq_params.level[i], &wb);
//			if (cm->seq_params.level[i].major > 3)
//				aom_wb_write_bit(&wb, cm->seq_params.tier[i]);
//			if (cm->seq_params.decoder_model_info_present_flag) {
//				aom_wb_write_bit(&wb,
//					cm->op_params[i].decoder_model_param_present_flag);
//				if (cm->op_params[i].decoder_model_param_present_flag)
//					write_dec_model_op_parameters(cm, &wb, i);
//			}
//			if (cm->seq_params.display_model_info_present_flag) {
//				aom_wb_write_bit(&wb,
//					cm->op_params[i].display_model_param_present_flag);
//				if (cm->op_params[i].display_model_param_present_flag) {
//					assert(cm->op_params[i].initial_display_delay <= 10);
//					aom_wb_write_literal(&wb, cm->op_params[i].initial_display_delay - 1,
//						4);
//				}
//			}
//		}
//	}
//	write_sequence_header(cpi, &wb);
//
//	write_color_config(&cm->seq_params, &wb);
//
//	aom_wb_write_bit(&wb, cm->seq_params.film_grain_params_present);
//
//	add_trailing_bits(&wb);
//
//	size = aom_wb_bytes_written(&wb);
//	return size;
//}
//
//
//static void write_sequence_header(AV1_COMP *cpi,
//	struct aom_write_bit_buffer *wb) {
//	AV1_COMMON *const cm = &cpi->common;
//	SequenceHeader *seq_params = &cm->seq_params;
//
//	int max_frame_width = cpi->oxcf.forced_max_frame_width
//		? cpi->oxcf.forced_max_frame_width
//		: cpi->oxcf.width;
//	int max_frame_height = cpi->oxcf.forced_max_frame_height
//		? cpi->oxcf.forced_max_frame_height
//		: cpi->oxcf.height;
//	// max((int)ceil(log2(max_frame_width)), 1)
//	const int num_bits_width =
//		(max_frame_width > 1) ? get_msb(max_frame_width - 1) + 1 : 1;
//	// max((int)ceil(log2(max_frame_height)), 1)
//	const int num_bits_height =
//		(max_frame_height > 1) ? get_msb(max_frame_height - 1) + 1 : 1;
//	assert(num_bits_width <= 16);
//	assert(num_bits_height <= 16);
//
//	seq_params->num_bits_width = num_bits_width;
//	seq_params->num_bits_height = num_bits_height;
//	seq_params->max_frame_width = max_frame_width;
//	seq_params->max_frame_height = max_frame_height;
//
//	aom_wb_write_literal(wb, num_bits_width - 1, 4);
//	aom_wb_write_literal(wb, num_bits_height - 1, 4);
//	aom_wb_write_literal(wb, max_frame_width - 1, num_bits_width);
//	aom_wb_write_literal(wb, max_frame_height - 1, num_bits_height);
//
//	/* Placeholder for actually writing to the bitstream */
//	if (!seq_params->reduced_still_picture_hdr) {
//		seq_params->frame_id_numbers_present_flag =
//			cm->large_scale_tile ? 0 : cm->error_resilient_mode;
//		seq_params->frame_id_length = FRAME_ID_LENGTH;
//		seq_params->delta_frame_id_length = DELTA_FRAME_ID_LENGTH;
//
//		aom_wb_write_bit(wb, seq_params->frame_id_numbers_present_flag);
//		if (seq_params->frame_id_numbers_present_flag) {
//			// We must always have delta_frame_id_length < frame_id_length,
//			// in order for a frame to be referenced with a unique delta.
//			// Avoid wasting bits by using a coding that enforces this restriction.
//			aom_wb_write_literal(wb, seq_params->delta_frame_id_length - 2, 4);
//			aom_wb_write_literal(
//				wb,
//				seq_params->frame_id_length - seq_params->delta_frame_id_length - 1,
//				3);
//		}
//	}
//
//	write_sb_size(seq_params, wb);
//
//	aom_wb_write_bit(wb, seq_params->enable_filter_intra);
//	aom_wb_write_bit(wb, seq_params->enable_intra_edge_filter);
//
//	if (!seq_params->reduced_still_picture_hdr) {
//		aom_wb_write_bit(wb, seq_params->enable_interintra_compound);
//		aom_wb_write_bit(wb, seq_params->enable_masked_compound);
//		aom_wb_write_bit(wb, seq_params->enable_warped_motion);
//		aom_wb_write_bit(wb, seq_params->enable_dual_filter);
//
//		aom_wb_write_bit(wb, seq_params->enable_order_hint);
//
//		if (seq_params->enable_order_hint) {
//			aom_wb_write_bit(wb, seq_params->enable_jnt_comp);
//			aom_wb_write_bit(wb, seq_params->enable_ref_frame_mvs);
//		}
//		if (seq_params->force_screen_content_tools == 2) {
//			aom_wb_write_bit(wb, 1);
//		}
//		else {
//			aom_wb_write_bit(wb, 0);
//			aom_wb_write_bit(wb, seq_params->force_screen_content_tools);
//		}
//		if (seq_params->force_screen_content_tools > 0) {
//			if (seq_params->force_integer_mv == 2) {
//				aom_wb_write_bit(wb, 1);
//			}
//			else {
//				aom_wb_write_bit(wb, 0);
//				aom_wb_write_bit(wb, seq_params->force_integer_mv);
//			}
//		}
//		else {
//			assert(seq_params->force_integer_mv == 2);
//		}
//		if (seq_params->enable_order_hint)
//			aom_wb_write_literal(wb, seq_params->order_hint_bits_minus_1, 3);
//	}
//
//	aom_wb_write_bit(wb, seq_params->enable_superres);
//	aom_wb_write_bit(wb, seq_params->enable_cdef);
//	aom_wb_write_bit(wb, seq_params->enable_restoration);
//}
//
//
//static void write_color_config(const SequenceHeader *const seq_params,
//	struct aom_write_bit_buffer *wb) {
//	write_bitdepth(seq_params, wb);
//	const int is_monochrome = seq_params->monochrome;
//	// monochrome bit
//	if (seq_params->profile != PROFILE_1)
//		aom_wb_write_bit(wb, is_monochrome);
//	else
//		assert(!is_monochrome);
//	if (seq_params->color_primaries == AOM_CICP_CP_UNSPECIFIED &&
//		seq_params->transfer_characteristics == AOM_CICP_TC_UNSPECIFIED &&
//		seq_params->matrix_coefficients == AOM_CICP_MC_UNSPECIFIED) {
//		aom_wb_write_bit(wb, 0);  // No color description present
//	}
//	else {
//		aom_wb_write_bit(wb, 1);  // Color description present
//		aom_wb_write_literal(wb, seq_params->color_primaries, 8);
//		aom_wb_write_literal(wb, seq_params->transfer_characteristics, 8);
//		aom_wb_write_literal(wb, seq_params->matrix_coefficients, 8);
//	}
//	if (is_monochrome) {
//		// 0: [16, 235] (i.e. xvYCC), 1: [0, 255]
//		aom_wb_write_bit(wb, seq_params->color_range);
//		return;
//	}
//	if (seq_params->color_primaries == AOM_CICP_CP_BT_709 &&
//		seq_params->transfer_characteristics == AOM_CICP_TC_SRGB &&
//		seq_params->matrix_coefficients ==
//		AOM_CICP_MC_IDENTITY) {  // it would be better to remove this
//								 // dependency too
//		assert(seq_params->subsampling_x == 0 && seq_params->subsampling_y == 0);
//		assert(seq_params->profile == PROFILE_1 ||
//			(seq_params->profile == PROFILE_2 &&
//				seq_params->bit_depth == AOM_BITS_12));
//	}
//	else {
//		// 0: [16, 235] (i.e. xvYCC), 1: [0, 255]
//		aom_wb_write_bit(wb, seq_params->color_range);
//		if (seq_params->profile == PROFILE_0) {
//			// 420 only
//			assert(seq_params->subsampling_x == 1 && seq_params->subsampling_y == 1);
//		}
//		else if (seq_params->profile == PROFILE_1) {
//			// 444 only
//			assert(seq_params->subsampling_x == 0 && seq_params->subsampling_y == 0);
//		}
//		else if (seq_params->profile == PROFILE_2) {
//			if (seq_params->bit_depth == AOM_BITS_12) {
//				// 420, 444 or 422
//				aom_wb_write_bit(wb, seq_params->subsampling_x);
//				if (seq_params->subsampling_x == 0) {
//					assert(seq_params->subsampling_y == 0 &&
//						"4:4:0 subsampling not allowed in AV1");
//				}
//				else {
//					aom_wb_write_bit(wb, seq_params->subsampling_y);
//				}
//			}
//			else {
//				// 422 only
//				assert(seq_params->subsampling_x == 1 &&
//					seq_params->subsampling_y == 0);
//			}
//		}
//		if (seq_params->matrix_coefficients == AOM_CICP_MC_IDENTITY) {
//			assert(seq_params->subsampling_x == 0 && seq_params->subsampling_y == 0);
//		}
//		if (seq_params->subsampling_x == 1 && seq_params->subsampling_y == 1) {
//			aom_wb_write_literal(wb, seq_params->chroma_sample_position, 2);
//		}
//	}
//	aom_wb_write_bit(wb, seq_params->separate_uv_delta_q);
//}
//
//
//static void add_trailing_bits(struct aom_write_bit_buffer *wb) {
//	if (aom_wb_is_byte_aligned(wb)) {
//		aom_wb_write_literal(wb, 0x80, 8);
//	}
//	else {
//		// assumes that the other bits are already 0s
//		aom_wb_write_bit(wb, 1);
//	}
//}