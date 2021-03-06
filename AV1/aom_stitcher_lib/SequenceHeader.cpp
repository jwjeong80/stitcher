﻿#include "SequenceHeader.h"
#include "CommonDef.h"
//#include "bit_reader.h"

CSequenceHeader::CSequenceHeader()
{
	printf("call intit %d", m_ParserIdx);
	m_seq_profile = PROFILE_0;
	m_still_picture = 0;
	m_reduced_still_picture_header = 0;
	m_timing_info_present_flag = 0;
	m_decoder_model_info_present_flag = 0;
	m_initial_display_delay_present_flag = 0;

	m_operating_points_cnt_minus_1 = 0;

	for (int i = 0; i < MAX_NUM_OPERATING_POINTS; i++) {
		m_operating_point_idc[i] = 0;
		m_seq_level_idx[i].major = 0;
		m_seq_level_idx[i].minor = 0;
		m_seq_tier[i] = 0;
		m_initial_display_delay_minus_1[i] = 0;
	}

	for (int i = 0; i < MAX_NUM_OPERATING_POINTS + 1; i++) {
		m_op_params[i].bitrate = 0;
		m_op_params[i].buffer_size = 0;
		m_op_params[i].decoder_buffer_delay = 0;
		m_op_params[i].decoder_model_present_for_this_op = 0;
		m_op_params[i].display_model_param_present_flag = 0;
		m_op_params[i].encoder_buffer_delay = 0;
		m_op_params[i].initial_display_delay_present_for_this_op = 0;
		m_op_params[i].low_delay_mode_flag = 0;
	}

	m_timing_info.equal_picture_interval = 0;
	m_timing_info.num_ticks_per_picture_minus_1 = 0;
	m_timing_info.num_units_in_display_tick = 0;
	m_timing_info.time_scale = 0;

	m_decoder_model_info.buffer_delay_length_minus_1 = 0;
	m_decoder_model_info.buffer_removal_time_length_minus_1 = 0;
	m_decoder_model_info.frame_presentation_time_length_minus_1 = 0;
	m_decoder_model_info.num_units_in_decoding_tick = 0;


	m_frame_width_bits_minus_1 = 0;
	m_frame_height_bits_minus_1 = 0;
	m_max_frame_width_minus_1 = 0;
	m_max_frame_height_minus_1 = 0;
	m_FrameWidth = 0;
	m_FrameHeight = 0;
	m_frame_id_numbers_present_flag = 0;
	m_delta_frame_id_length_minus_2 = 0;
	m_additional_frame_id_length_minus_1 = 0;
	m_use_128x128_superblock = 0;  // Size of the superblock used for this frame
	m_enable_filter_intra = 0;
	m_enable_intra_edge_filter = 0;

	m_enable_interintra_compound = 0;
	m_enable_masked_compound = 0;
	m_enable_warped_motion = 0;
	m_enable_dual_filter = 0;
	m_enable_order_hint = 0;
	m_enable_jnt_comp = 0;
	m_enable_ref_frame_mvs = 0;
	m_seq_choose_screen_content_tools = 0;
	m_seq_force_screen_content_tools = 0;
	m_seq_choose_integer_mv = 0;
	m_seq_force_integer_mv = 0;
	m_order_hint_bits_minus_1 = 0;

	m_enable_superres;
	m_enable_cdef = 0;
	m_enable_restoration = 0;

	m_high_bitdepth = 0;
	m_twelve_bit = 0;
	m_mono_chrome = 0;
	m_color_description_present_flag = 0;
	m_color_primaries = 0;
	m_transfer_characteristics = 0;
	m_matrix_coefficients = 0;
	m_color_range = 0;
	m_subsampling_x = 0;
	m_subsampling_y = 0;
	m_chroma_sample_position = 0;
	m_separate_uv_delta_q = 0;
	m_film_grain_params_present = 0;

	m_BitDepth = 0;
	m_NumPlanes = 0;
	m_OrderHintBits = 0;
}
CSequenceHeader::~CSequenceHeader()
{

}
void CSequenceHeader::ShInitialize() {

}

int CSequenceHeader::ShParserSeqLevelIdx(int idx, CBitReader *rb) {
	const uint8_t seq_level_idx = rb->AomRbReadLiteral(LEVEL_BITS);
	if (!is_valid_seq_level_idx(seq_level_idx)) return 0;
	m_seq_level_idx[idx].major = (seq_level_idx >> LEVEL_MINOR_BITS) + LEVEL_MAJOR_MIN;
	m_seq_level_idx[idx].minor = seq_level_idx & ((1 << LEVEL_MINOR_BITS) - 1);
	return 1;
}

void CSequenceHeader::ShParserTimingInfoHeader(CBitReader *rb) {
	m_timing_info.num_units_in_display_tick = rb->AomRbReadUnsignedLiteral(32);  // Number of units in a display tick
	m_timing_info.time_scale = rb->AomRbReadUnsignedLiteral(32);  // Time scale
	if (m_timing_info.num_units_in_display_tick == 0 || m_timing_info.time_scale == 0) {
		printf("num_units_in_display_tick and time_scale must be greater than 0. \n");
	}
	m_timing_info.equal_picture_interval = rb->AomRbReadBit();  // Equal picture interval bit
	if (m_timing_info.equal_picture_interval) {
		m_timing_info.num_ticks_per_picture_minus_1 = rb->AomRbReadUvlc() + 1;  // ticks per picture
		if (m_timing_info.num_ticks_per_picture_minus_1 == 0) {
			printf("num_ticks_per_picture_minus_1 cannot be (1 << 32) − 1.");
		}
	}
}

void CSequenceHeader::ShParserDecoderModelInfo(CBitReader *rb) {
	m_decoder_model_info.buffer_delay_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
	m_decoder_model_info.num_units_in_decoding_tick = rb->AomRbReadUnsignedLiteral(32);  // Number of units in a decoding tick
	m_decoder_model_info.buffer_removal_time_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
	m_decoder_model_info.frame_presentation_time_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
}

void CSequenceHeader::ShParserOperatingParametersInfo(int op_num, CBitReader *rb) {
	// The cm->op_params array has MAX_NUM_OPERATING_POINTS + 1 elements.
	if (op_num > MAX_NUM_OPERATING_POINTS) {
		printf("AV1 does not support %d decoder model operating points", op_num + 1);
	}

	m_op_params[op_num].decoder_buffer_delay = rb->AomRbReadUnsignedLiteral(m_decoder_model_info.buffer_delay_length_minus_1);
	m_op_params[op_num].encoder_buffer_delay = rb->AomRbReadUnsignedLiteral(m_decoder_model_info.buffer_delay_length_minus_1);
	m_op_params[op_num].low_delay_mode_flag = rb->AomRbReadBit();
}

void CSequenceHeader::ShParserSequenceInfo(CBitReader *rb) {
	m_frame_width_bits_minus_1 = rb->AomRbReadLiteral(4);
	m_frame_height_bits_minus_1 = rb->AomRbReadLiteral(4);
	m_max_frame_width_minus_1 = rb->AomRbReadLiteral(m_frame_width_bits_minus_1 + 1);
	m_max_frame_height_minus_1 = rb->AomRbReadLiteral(m_frame_height_bits_minus_1 + 1);

	m_FrameWidth = m_max_frame_width_minus_1 + 1;
	m_FrameHeight = m_max_frame_height_minus_1 + 1;

	if (m_reduced_still_picture_header)
		m_frame_id_numbers_present_flag = 0;
	else
		m_frame_id_numbers_present_flag = rb->AomRbReadBit();

	if (m_frame_id_numbers_present_flag)
	{
		m_delta_frame_id_length_minus_2 = rb->AomRbReadLiteral(4);
		m_additional_frame_id_length_minus_1 = rb->AomRbReadLiteral(3);
	}
	m_use_128x128_superblock = rb->AomRbReadBit();
	m_enable_filter_intra = rb->AomRbReadBit();
	m_enable_intra_edge_filter = rb->AomRbReadBit();

	if (m_reduced_still_picture_header) {
		m_enable_interintra_compound = 0;
		m_enable_masked_compound = 0;
		m_enable_warped_motion = 0;
		m_enable_dual_filter = 0;
		m_enable_order_hint = 0;
		m_enable_jnt_comp = 0;
		m_enable_ref_frame_mvs = 0;
		m_seq_force_screen_content_tools = 2;  // SELECT_SCREEN_CONTENT_TOOLS
		m_seq_force_integer_mv = 2;            // SELECT_INTEGER_MV
		m_OrderHintBits = 0;
	}
	else {
		m_enable_interintra_compound = rb->AomRbReadBit();
		m_enable_masked_compound = rb->AomRbReadBit();
		m_enable_warped_motion = rb->AomRbReadBit();
		m_enable_dual_filter = rb->AomRbReadBit();
		m_enable_order_hint = rb->AomRbReadBit();

		if (m_enable_order_hint) {
			m_enable_jnt_comp = rb->AomRbReadBit();
			m_enable_ref_frame_mvs = rb->AomRbReadBit();
		}
		else {
			m_enable_jnt_comp = 0;
			m_enable_ref_frame_mvs = 0;
		}
		m_seq_choose_screen_content_tools = rb->AomRbReadBit();

		if (m_seq_choose_screen_content_tools)
			m_seq_force_screen_content_tools = 2; // SELECT_SCREEN_CONTENT_TOOLS
		else
			m_seq_force_screen_content_tools = rb->AomRbReadBit();

		if (m_seq_force_screen_content_tools > 0) {
			m_seq_choose_integer_mv = rb->AomRbReadBit();

			if (m_seq_choose_integer_mv)
				m_seq_force_integer_mv = 2;            // SELECT_INTEGER_MV
			else
				m_seq_force_integer_mv = rb->AomRbReadBit();
		}
		else {
			m_seq_force_integer_mv = 2;            // SELECT_INTEGER_MV
		}

		if (m_enable_order_hint) {
			m_order_hint_bits_minus_1 = rb->AomRbReadLiteral(3);
			m_OrderHintBits = m_order_hint_bits_minus_1 + 1;
		}
		else {
			m_OrderHintBits = 0;
		}
	}

	m_enable_superres = rb->AomRbReadBit();
	m_enable_cdef = rb->AomRbReadBit();
	m_enable_restoration = rb->AomRbReadBit();
}

void CSequenceHeader::ShParserColorConfig(CBitReader *rb) {

	m_high_bitdepth = rb->AomRbReadBit();

	if (m_seq_profile == PROFILE_2 && m_high_bitdepth) {
		m_twelve_bit = rb->AomRbReadBit();
		m_BitDepth = m_twelve_bit ? AOM_BITS_12 : AOM_BITS_10;
	}
	else if (m_seq_profile <= PROFILE_2) {
		m_BitDepth = m_twelve_bit ? AOM_BITS_10 : AOM_BITS_8;
	}
	else {
		printf("Unsupported profile/bit-depth combination");
	}

	if (m_seq_profile == PROFILE_1)
		m_mono_chrome = 0;
	else
		m_mono_chrome = rb->AomRbReadBit();

	m_NumPlanes = m_mono_chrome ? 1 : 3;
	m_color_description_present_flag = rb->AomRbReadBit();

	if (m_color_description_present_flag) {
		m_color_primaries = rb->AomRbReadLiteral(8);
		m_transfer_characteristics = rb->AomRbReadLiteral(8);
		m_matrix_coefficients = rb->AomRbReadLiteral(8);
	}
	else {
		m_color_primaries = AOM_CICP_CP_UNSPECIFIED;
		m_transfer_characteristics = AOM_CICP_TC_UNSPECIFIED;
		m_matrix_coefficients = AOM_CICP_MC_UNSPECIFIED;
	}

	if (m_mono_chrome) {
		// [16,235] (including xvycc) vs [0,255] range
		m_color_range = rb->AomRbReadBit();
		m_subsampling_y = m_subsampling_x = 1;
		m_chroma_sample_position = AOM_CSP_UNKNOWN;
		m_separate_uv_delta_q = 0;
		return;
	}
	else if (m_color_primaries == AOM_CICP_CP_BT_709 &&
		m_transfer_characteristics == AOM_CICP_TC_SRGB &&
		m_matrix_coefficients == AOM_CICP_MC_IDENTITY) {
		// It would be good to remove this dependency.
		m_subsampling_y = m_subsampling_x = 0;
		m_color_range = 1;  // assume full color-range
		if (!(m_seq_profile == PROFILE_1 ||
			(m_seq_profile == PROFILE_2 && m_seq_profile == AOM_BITS_12))) {
			printf("sRGB colorspace not compatible with specified profile \n");
		}
	}
	else {
		// [16,235] (including xvycc) vs [0,255] range
		m_color_range = rb->AomRbReadBit();
		if (m_seq_profile == PROFILE_0) {
			// 420 only
			m_subsampling_x = m_subsampling_y = 1;
		}
		else if (m_seq_profile == PROFILE_1) {
			// 444 only
			m_subsampling_x = m_subsampling_y = 0;
		}
		else {
			assert(m_seq_profile == PROFILE_2);

			if (m_BitDepth == AOM_BITS_12) {
				m_subsampling_x = rb->AomRbReadBit();
				if (m_subsampling_x)
					m_subsampling_y = rb->AomRbReadBit(); // 422 or 420
				else
					m_subsampling_y = 0;  // 444
			}
			else {
				// 422
				m_subsampling_x = 1;
				m_subsampling_y = 0;
			}
		}
		if (m_matrix_coefficients == AOM_CICP_MC_IDENTITY &&
			(m_subsampling_x || m_subsampling_y)) {
			printf("Identity CICP Matrix incompatible with non 4:4:4 color sampling\n");
		}
		if (m_subsampling_x && m_subsampling_y) {
			m_chroma_sample_position = rb->AomRbReadLiteral(2);
		}
	}
	m_separate_uv_delta_q = rb->AomRbReadBit();


	if (!(m_subsampling_x == 0 && m_subsampling_y == 0) &&
		!(m_subsampling_x == 1 && m_subsampling_y == 1) &&
		!(m_subsampling_x == 1 && m_subsampling_y == 0)) {
		printf("Only 4:4:4, 4:2:2 and 4:2:0 are currently supported, "
			"%d %d subsampling is not supported.\n",
			m_subsampling_x, m_subsampling_y);
	}

}


uint32_t CSequenceHeader::write_sequence_header_obu(FrameSize_t *tileSizes, uint8_t *const dst, int bit_buffer_offset) {
	CBitWriter wb(dst, bit_buffer_offset);
	uint32_t size = 0;
	uint32_t before_size = bit_buffer_offset >> 3;
	assert(bit_buffer_offset % 8 == 0);

	write_profile(m_seq_profile, &wb);

	// Still picture or not
	wb.aom_wb_write_bit(m_still_picture);
	assert(IMPLIES(!m_still_picture, !m_reduced_still_picture_header));
	// whether to use reduced still picture header
	wb.aom_wb_write_bit(m_reduced_still_picture_header);

	if (m_reduced_still_picture_header) {
		assert(m_timing_info_present_flag == 0);
		assert(m_decoder_model_info_present_flag == 0);
		assert(m_initial_display_delay_present_flag == 0);
		write_bitstream_level(m_seq_level_idx[0], &wb); //need update
	}
	else {
		wb.aom_wb_write_bit(m_timing_info_present_flag);  // timing info present flag

		if (m_timing_info_present_flag) {
			// timing_info
			write_timing_info_header(&wb);
			wb.aom_wb_write_bit(m_decoder_model_info_present_flag);
			if (m_decoder_model_info_present_flag) {
				write_decoder_model_info(&wb);
			}
		}
		wb.aom_wb_write_bit(m_initial_display_delay_present_flag);
		wb.aom_wb_write_literal(m_operating_points_cnt_minus_1, OP_POINTS_CNT_MINUS_1_BITS);
		int i;
		for (i = 0; i < m_operating_points_cnt_minus_1 + 1; i++) {
			wb.aom_wb_write_literal(m_operating_point_idc[i], OP_POINTS_IDC_BITS);
			//modified level 6.0
			m_seq_level_idx[i].major = 6;
			m_seq_level_idx[i].minor = 0;
			write_bitstream_level(m_seq_level_idx[i], &wb);
			if (m_seq_level_idx[i].major > 3)
				wb.aom_wb_write_bit(m_seq_tier[i]);
			if (m_decoder_model_info_present_flag) {
				wb.aom_wb_write_bit(m_op_params[i].decoder_model_present_for_this_op);

				if (m_op_params[i].decoder_model_present_for_this_op)
					write_dec_model_op_parameters(&wb, i);
			}
			if (m_initial_display_delay_present_flag) {
				wb.aom_wb_write_bit(m_op_params[i].initial_display_delay_present_for_this_op);
				if (m_op_params[i].initial_display_delay_present_for_this_op) {
					assert(m_op_params[i].initial_display_delay_present_for_this_op <= 10);
					wb.aom_wb_write_literal(m_initial_display_delay_minus_1[i], 4);
				}
			}
		}
	}
	write_sequence_header(tileSizes, &wb);

	write_color_config(&wb);

	wb.aom_wb_write_bit(m_film_grain_params_present);

	wb.add_trailing_bits();

	size = wb.aom_wb_bytes_written();
	return size - before_size;
}

void CSequenceHeader::write_profile(BITSTREAM_PROFILE profile, CBitWriter *wb) {
	assert(profile >= PROFILE_0 && profile < MAX_PROFILES);
	wb->aom_wb_write_literal(profile, PROFILE_BITS);
}

void CSequenceHeader::write_bitstream_level(BitstreamLevel bl, CBitWriter *wb) {
	uint8_t seq_level_idx = major_minor_to_seq_level_idx(bl);
	assert(is_valid_seq_level_idx(seq_level_idx));
	wb->aom_wb_write_literal(seq_level_idx, LEVEL_BITS);
}

uint8_t CSequenceHeader::major_minor_to_seq_level_idx(BitstreamLevel bl) {
	assert(bl.major >= LEVEL_MAJOR_MIN && bl.major <= LEVEL_MAJOR_MAX);
	// Since bl.minor is unsigned a comparison will return a warning:
	// comparison is always true due to limited range of data type
	assert(LEVEL_MINOR_MIN == 0);
	assert(bl.minor <= LEVEL_MINOR_MAX);
	return ((bl.major - LEVEL_MAJOR_MIN) << LEVEL_MINOR_BITS) + bl.minor;
}


void CSequenceHeader::write_timing_info_header(CBitWriter *wb) {
	wb->aom_wb_write_unsigned_literal(m_timing_info.num_units_in_display_tick, 32);  // Number of units in tick
	wb->aom_wb_write_unsigned_literal(m_timing_info.time_scale,	32);  // Time scale
	wb->aom_wb_write_bit(m_timing_info.equal_picture_interval);  // Equal picture interval bit
	if (m_timing_info.equal_picture_interval) {
		wb->aom_wb_write_uvlc(m_timing_info.num_ticks_per_picture_minus_1); // ticks per picture
	}
}

void CSequenceHeader::write_decoder_model_info(CBitWriter *wb) {
	wb->aom_wb_write_literal(m_decoder_model_info.buffer_delay_length_minus_1, 5);
	wb->aom_wb_write_unsigned_literal(m_decoder_model_info.num_units_in_decoding_tick, 32);
	wb->aom_wb_write_literal(m_decoder_model_info.buffer_removal_time_length_minus_1, 5);
	wb->aom_wb_write_literal(m_decoder_model_info.frame_presentation_time_length_minus_1, 5);
}

void CSequenceHeader::write_dec_model_op_parameters(CBitWriter *wb,	int op_num) {
	if (op_num > MAX_NUM_OPERATING_POINTS)
		printf("Encoder does not support %d decoder model operating points", op_num);
	int n = m_decoder_model_info.buffer_delay_length_minus_1 + 1;
	//  aom_wb_write_bit(wb, cm->op_params[op_num].has_parameters);
	//  if (!cm->op_params[op_num].has_parameters) return;

	wb->aom_wb_write_unsigned_literal(m_op_params[op_num].decoder_buffer_delay, n);
	wb->aom_wb_write_unsigned_literal(m_op_params[op_num].encoder_buffer_delay, n);
	wb->aom_wb_write_bit(m_op_params[op_num].low_delay_mode_flag);

//	m_buffer_removal_time[op_num] = 0; //FrameHeader.h

}


void CSequenceHeader::write_sequence_header(FrameSize_t *tileSizes, CBitWriter *wb) {

	m_FrameWidth = 0;
	m_FrameHeight = 0;
	//calculate total size
	for (int i = 0; i < m_uiNumTileCols; i++)
		m_FrameWidth += tileSizes[i].frame_width;

	for (int i = 0; i < m_uiNumTileRows; i++)
		m_FrameHeight += tileSizes[i*m_uiNumTileCols].frame_height;

	const int num_bits_width =
		(m_FrameWidth > 1) ? get_msb(m_FrameWidth - 1) + 1 : 1;
	// max((int)ceil(log2(max_frame_height)), 1)
	const int num_bits_height =
		(m_FrameHeight > 1) ? get_msb(m_FrameHeight - 1) + 1 : 1;
	assert(num_bits_width <= 16);
	assert(num_bits_height <= 16);

	m_frame_width_bits_minus_1 = num_bits_width - 1;
	m_frame_height_bits_minus_1 = num_bits_height - 1;

	m_max_frame_width_minus_1 = m_FrameWidth - 1;
	m_max_frame_height_minus_1 = m_FrameHeight - 1;

	wb->aom_wb_write_literal(m_frame_width_bits_minus_1, 4);
	wb->aom_wb_write_literal(m_frame_height_bits_minus_1, 4);
	wb->aom_wb_write_literal(m_max_frame_width_minus_1, num_bits_width);
	wb->aom_wb_write_literal(m_max_frame_height_minus_1, num_bits_height);

	///* Placeholder for actually writing to the bitstream */
	if (!m_reduced_still_picture_header) {
		wb->aom_wb_write_bit(m_frame_id_numbers_present_flag);
		if (m_frame_id_numbers_present_flag) {
			// We must always have delta_frame_id_length < frame_id_length,
			// in order for a frame to be referenced with a unique delta.
			// Avoid wasting bits by using a coding that enforces this restriction.
			wb->aom_wb_write_literal(m_delta_frame_id_length_minus_2, 4);
			wb->aom_wb_write_literal(m_additional_frame_id_length_minus_1, 3);
		}
	}
	wb->aom_wb_write_bit(m_use_128x128_superblock);
	wb->aom_wb_write_bit(m_enable_filter_intra);
	wb->aom_wb_write_bit(m_enable_intra_edge_filter);

	if (!m_reduced_still_picture_header) {
		wb->aom_wb_write_bit(m_enable_interintra_compound);
		wb->aom_wb_write_bit(m_enable_masked_compound);
		wb->aom_wb_write_bit(m_enable_warped_motion);
		wb->aom_wb_write_bit(m_enable_dual_filter);

		wb->aom_wb_write_bit(m_enable_order_hint);

		if (m_enable_order_hint) {
			wb->aom_wb_write_bit(m_enable_jnt_comp);
			wb->aom_wb_write_bit(m_enable_ref_frame_mvs);
		}

		wb->aom_wb_write_bit(m_seq_choose_screen_content_tools);

		if (m_seq_choose_screen_content_tools) {
			m_seq_force_screen_content_tools = 2; //SELECT_SCREEN_CONTENT_TOOLS =2
		}
		else {
			wb->aom_wb_write_bit(m_seq_force_screen_content_tools);
		}

		if (m_seq_force_screen_content_tools > 0) {
			if (m_seq_force_integer_mv == 2) {
				wb->aom_wb_write_bit(1);  // m_seq_choose_integer_mv
			}
			else {
				wb->aom_wb_write_bit(0); // m_seq_choose_integer_mv
				wb->aom_wb_write_bit(m_seq_force_integer_mv);
			}
		}
		else {
			assert(m_seq_force_integer_mv == 2);
		}
		if (m_enable_order_hint)
			wb->aom_wb_write_literal(m_order_hint_bits_minus_1, 3);
	}

	wb->aom_wb_write_bit(m_enable_superres);
	wb->aom_wb_write_bit(m_enable_cdef);
	wb->aom_wb_write_bit(m_enable_restoration);
}

void CSequenceHeader::write_color_config(CBitWriter *wb) {
	wb->aom_wb_write_bit(m_high_bitdepth);
	
	if (m_seq_profile == PROFILE_2 && m_high_bitdepth) {
		wb->aom_wb_write_bit(m_twelve_bit);
		m_BitDepth = m_twelve_bit ? 12:10;
	}
	else if (m_seq_profile <= PROFILE_2) {
		m_BitDepth = m_high_bitdepth ? 10 : 8;
	}
	if (m_seq_profile == PROFILE_1)
		m_mono_chrome = 0;
	else
		wb->aom_wb_write_bit(m_mono_chrome);


	if (m_color_primaries == AOM_CICP_CP_UNSPECIFIED &&
		m_transfer_characteristics == AOM_CICP_TC_UNSPECIFIED &&
		m_matrix_coefficients == AOM_CICP_MC_UNSPECIFIED) {
		wb->aom_wb_write_bit(0);  // No color description present
	}
	else {
		wb->aom_wb_write_bit(1);  // Color description present
		wb->aom_wb_write_literal(m_color_primaries, 8);
		wb->aom_wb_write_literal(m_transfer_characteristics, 8);
		wb->aom_wb_write_literal(m_matrix_coefficients, 8);
	}

	if (m_mono_chrome) {
		// 0: [16, 235] (i.e. xvYCC), 1: [0, 255]
		wb->aom_wb_write_bit(m_color_range);
		return;
	}
	if (m_color_primaries == AOM_CICP_CP_BT_709 &&
		m_transfer_characteristics == AOM_CICP_TC_SRGB &&
		m_matrix_coefficients == AOM_CICP_MC_IDENTITY) {  
		// it would be better to remove this dependency too
		assert(m_subsampling_x == 0 && m_subsampling_y == 0);
		assert(m_seq_profile == PROFILE_1 ||
			(m_seq_profile == PROFILE_2 && m_BitDepth == AOM_BITS_12));
	}
	else {
		// 0: [16, 235] (i.e. xvYCC), 1: [0, 255]
		wb->aom_wb_write_bit(m_color_range);
		if (m_seq_profile == PROFILE_0) {
			// 420 only
			assert(m_subsampling_x == 1 && m_subsampling_y == 1);
		}
		else if (m_seq_profile == PROFILE_1) {
			// 444 only
			assert(m_subsampling_x == 0 && m_subsampling_y == 0);
		}
		else if (m_seq_profile == PROFILE_2) {
			if (m_BitDepth == AOM_BITS_12) {
				// 420, 444 or 422
				wb->aom_wb_write_bit(m_subsampling_x);
				if (m_subsampling_x == 0) {
					assert(m_subsampling_y == 0 &&
						"4:4:0 subsampling not allowed in AV1");
				}
				else {
					wb->aom_wb_write_bit(m_subsampling_y);
				}
			}
			else {
				// 422 only
				assert(m_subsampling_x == 1 && m_subsampling_y == 0);
			}
		}
		if (m_matrix_coefficients == AOM_CICP_MC_IDENTITY) {
			assert(m_subsampling_x == 0 && m_subsampling_y == 0);
		}
		if (m_subsampling_x == 1 && m_subsampling_y == 1) {
			wb->aom_wb_write_literal(m_chroma_sample_position, 2);
		}
	}
	wb->aom_wb_write_bit(m_separate_uv_delta_q);
}

uint32_t CSequenceHeader::SequencHeaderCompare(CSequenceHeader *pSh) {

	assert(m_seq_profile == pSh->m_seq_profile);
	assert(m_still_picture == pSh->m_still_picture);
	assert(m_reduced_still_picture_header == pSh->m_reduced_still_picture_header);
	assert(m_timing_info_present_flag == pSh->m_timing_info_present_flag);
	assert(m_decoder_model_info_present_flag == pSh->m_decoder_model_info_present_flag);
	assert(m_initial_display_delay_present_flag == pSh->m_initial_display_delay_present_flag);
	assert(m_operating_points_cnt_minus_1 == pSh->m_operating_points_cnt_minus_1);

	for (int i = 0; i < m_operating_points_cnt_minus_1+1; i++) {
		assert(m_operating_point_idc[i] == pSh->m_operating_point_idc[i]);
		assert(m_seq_level_idx[i].major == pSh->m_seq_level_idx[i].major);
		assert(m_seq_level_idx[i].minor == pSh->m_seq_level_idx[i].minor);
		assert(m_seq_tier[i] == pSh->m_seq_tier[i]);
	}

	for (int i = 0; i < MAX_NUM_OPERATING_POINTS + 1; i++) {
		assert(m_op_params[i].bitrate == pSh->m_op_params[i].bitrate);
		assert(m_op_params[i].buffer_size == pSh->m_op_params[i].buffer_size);
		assert(m_op_params[i].decoder_buffer_delay == pSh->m_op_params[i].decoder_buffer_delay);
		assert(m_op_params[i].decoder_model_present_for_this_op == pSh->m_op_params[i].decoder_model_present_for_this_op);
		assert(m_op_params[i].display_model_param_present_flag == pSh->m_op_params[i].display_model_param_present_flag);
		assert(m_op_params[i].encoder_buffer_delay == pSh->m_op_params[i].encoder_buffer_delay);
		assert(m_op_params[i].initial_display_delay_present_for_this_op == pSh->m_op_params[i].initial_display_delay_present_for_this_op);
		assert(m_op_params[i].low_delay_mode_flag == pSh->m_op_params[i].low_delay_mode_flag);
	}
	//
	assert(m_timing_info.equal_picture_interval == pSh->m_timing_info.equal_picture_interval);
	assert(m_timing_info.num_ticks_per_picture_minus_1 == pSh->m_timing_info.num_ticks_per_picture_minus_1);
	assert(m_timing_info.num_units_in_display_tick == pSh->m_timing_info.num_units_in_display_tick);
	assert(m_timing_info.time_scale == pSh->m_timing_info.time_scale);

	//
	assert(m_decoder_model_info.buffer_delay_length_minus_1 == pSh->m_decoder_model_info.buffer_delay_length_minus_1);
	assert(m_decoder_model_info.buffer_removal_time_length_minus_1 == pSh->m_decoder_model_info.buffer_removal_time_length_minus_1);
	assert(m_decoder_model_info.frame_presentation_time_length_minus_1 == pSh->m_decoder_model_info.frame_presentation_time_length_minus_1);
	assert(m_decoder_model_info.num_units_in_decoding_tick == pSh->m_decoder_model_info.num_units_in_decoding_tick);
	
	for (int i = 0; i < MAX_NUM_OPERATING_POINTS; i++) {
		assert(m_initial_display_delay_minus_1[i] == pSh->m_initial_display_delay_minus_1[i]);
	}

	//no check
	//int m_frame_width_bits_minus_1;
	//int m_frame_height_bits_minus_1;
	//int m_max_frame_width_minus_1;
	//int m_max_frame_height_minus_1;
	//int m_FrameWidth;
	//int m_FrameHeight;
	assert(m_frame_id_numbers_present_flag == pSh->m_frame_id_numbers_present_flag);
	assert(m_delta_frame_id_length_minus_2 == pSh->m_delta_frame_id_length_minus_2);
	assert(m_additional_frame_id_length_minus_1 == pSh->m_additional_frame_id_length_minus_1);
	assert(m_use_128x128_superblock == pSh->m_use_128x128_superblock);
	assert(m_enable_filter_intra == pSh->m_enable_filter_intra);
	assert(m_enable_intra_edge_filter == pSh->m_enable_intra_edge_filter);

	assert(m_enable_interintra_compound == pSh->m_enable_interintra_compound);
	assert(m_enable_masked_compound == pSh->m_enable_masked_compound);
	assert(m_enable_warped_motion == pSh->m_enable_warped_motion);
	assert(m_enable_dual_filter == pSh->m_enable_dual_filter);
	assert(m_enable_order_hint == pSh->m_enable_order_hint);
	assert(m_enable_jnt_comp == pSh->m_enable_jnt_comp);
	assert(m_enable_ref_frame_mvs == pSh->m_enable_ref_frame_mvs);
	assert(m_seq_choose_screen_content_tools == pSh->m_seq_choose_screen_content_tools);
	assert(m_seq_force_integer_mv == pSh->m_seq_force_integer_mv);
	assert(m_seq_choose_integer_mv == pSh->m_seq_choose_integer_mv);
	assert(m_order_hint_bits_minus_1 == pSh->m_order_hint_bits_minus_1);

	assert(m_enable_superres == pSh->m_enable_superres);
	assert(m_enable_cdef == pSh->m_enable_cdef);
	assert(m_enable_restoration == pSh->m_enable_restoration);

	assert(m_high_bitdepth == pSh->m_high_bitdepth);
	assert(m_twelve_bit == pSh->m_twelve_bit);
	assert(m_mono_chrome == pSh->m_mono_chrome);
	assert(m_color_description_present_flag == pSh->m_color_description_present_flag);
	assert(m_color_primaries == pSh->m_color_primaries);
	assert(m_transfer_characteristics == pSh->m_transfer_characteristics);
	assert(m_matrix_coefficients == pSh->m_matrix_coefficients);
	assert(m_color_range == pSh->m_color_range);
	assert(m_subsampling_x == pSh->m_subsampling_x);
	assert(m_subsampling_y == pSh->m_subsampling_y);
	assert(m_chroma_sample_position == pSh->m_chroma_sample_position);
	assert(m_separate_uv_delta_q == pSh->m_separate_uv_delta_q);
	assert(m_film_grain_params_present == pSh->m_film_grain_params_present);

	assert(m_BitDepth == pSh->m_BitDepth);
	assert(m_NumPlanes == pSh->m_NumPlanes);
	assert(m_OrderHintBits == pSh->m_OrderHintBits);
	return 1;
}