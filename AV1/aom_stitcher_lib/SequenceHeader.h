﻿#include <stdio.h>
//#include "av1/decoder/obu.h"
//#include "av1/common/enums.h"
//#include "bit_reader.h"
//#include "av1/common/timing.h"
#include "bit_reader_c.h"
//#include "av1_common.h"
///////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_TEMPORAL_LAYERS 8
#define MAX_NUM_SPATIAL_LAYERS 4
/* clang-format off */
// clang-format seems to think this is a pointer dereference and not a
// multiplication.
#define MAX_NUM_OPERATING_POINTS \
  MAX_NUM_TEMPORAL_LAYERS * MAX_NUM_SPATIAL_LAYERS
#define PROFILE_BITS 3
// The following three profiles are currently defined.
// Profile 0.  8-bit and 10-bit 4:2:0 and 4:0:0 only.
// Profile 1.  8-bit and 10-bit 4:4:4
// Profile 2.  8-bit and 10-bit 4:2:2
//            12-bit  4:0:0, 4:2:2 and 4:4:4
// Since we have three bits for the profiles, it can be extended later.
typedef enum BITSTREAM_PROFILE {
	PROFILE_0,
	PROFILE_1,
	PROFILE_2,
	MAX_PROFILES,
} BITSTREAM_PROFILE;

typedef struct BitstreamLevel {
	uint8_t major;
	uint8_t minor;
} BitstreamLevel;

#define LEVEL_MAJOR_BITS 3
#define LEVEL_MINOR_BITS 2
#define LEVEL_BITS (LEVEL_MAJOR_BITS + LEVEL_MINOR_BITS)

#define LEVEL_MAJOR_MIN 2
#define LEVEL_MAJOR_MAX ((1 << LEVEL_MAJOR_BITS) - 1 + LEVEL_MAJOR_MIN)
#define LEVEL_MINOR_MIN 0
#define LEVEL_MINOR_MAX ((1 << LEVEL_MINOR_BITS) - 1)

#define OP_POINTS_CNT_MINUS_1_BITS 5
#define OP_POINTS_IDC_BITS 12

typedef struct timing_info {
	uint32_t num_units_in_display_tick;
	uint32_t time_scale;
	int equal_picture_interval;
	uint32_t num_ticks_per_picture_minus_1;
} timing_info_t;

typedef struct decoder_model_info {
	int buffer_delay_length_minus_1;
	uint32_t num_units_in_decoding_tick;
	int buffer_removal_time_length_minus_1;
	int frame_presentation_time_length_minus_1;
} decoder_model_info_t;

typedef struct aom_dec_model_op_parameters {
	int decoder_model_present_for_this_op;
	int initial_display_delay_present_for_this_op;
	int64_t bitrate;
	int64_t buffer_size;
	uint32_t decoder_buffer_delay;
	uint32_t encoder_buffer_delay;
	int low_delay_mode_flag;
	int display_model_param_present_flag;
} aom_dec_model_op_parameters_t;


class CSequenceHeader : CBitReader
{
public:
	CSequenceHeader();
	virtual ~CSequenceHeader();

	int is_valid_seq_level_idx(uint8_t seq_level_idx) {
		return seq_level_idx < 24 || seq_level_idx == 31;
	}


	void ShParserProfile(BITSTREAM_PROFILE seq_profile) { m_seq_profile = seq_profile; }
	void ShParserStillPicture(int still_picture) { m_still_picture = still_picture; }
	void ShParserReducedStillPictureHdr(int reduced_still_picture_header) { m_reduced_still_picture_header = reduced_still_picture_header; }
	void ShParserTimingInfoPresentFlag(int timing_info_present_flag) { m_timing_info_present_flag = timing_info_present_flag; }
	void ShParserDecoderModelInfoPresentFlag(int decoder_model_info_present_flag) { m_decoder_model_info_present_flag = decoder_model_info_present_flag; }
	void ShParserInitialDisplayDelayPresentFlag(int initial_display_delay_present_flag) { m_initial_display_delay_present_flag = initial_display_delay_present_flag; }
	void ShParserOperatingPointsCntMinus1(int operating_points_cnt_minus_1) { m_operating_points_cnt_minus_1 = operating_points_cnt_minus_1; }
	void ShParserOperatingPointIdc(int op_num, int operating_point_idc) { m_operating_point_idc[op_num] = operating_point_idc; }
	
	void ShParserSeqTier(int op_num, uint8_t seq_tier) { m_seq_tier[op_num] = seq_tier; }
	void ShParserDecoderModelPresentForThisOp(int op_num, int decoder_model_present_for_this_op) { 
		m_op_params[op_num].decoder_model_present_for_this_op = decoder_model_present_for_this_op; }
	void ShParserInitialDisplayDelayPresentForThisOp(int op_num, int initial_display_delay_present_for_this_op) { 
		m_op_params[op_num].initial_display_delay_present_for_this_op = initial_display_delay_present_for_this_op; }
	void ShParserInitialDisplayDelayMinus1(int op_num, int initial_display_delay_minus_1) {
		m_initial_display_delay_minus_1[op_num] = initial_display_delay_minus_1;
	}


	int ShParserSeqLevelIdx(int idx, CBitReader *rb) {
		const uint8_t seq_level_idx = rb->AomRbReadLiteral(LEVEL_BITS);
		if (!is_valid_seq_level_idx(seq_level_idx)) return 0;
		m_seq_level_idx[idx].major = (seq_level_idx >> LEVEL_MINOR_BITS) + LEVEL_MAJOR_MIN;
		m_seq_level_idx[idx].minor = seq_level_idx & ((1 << LEVEL_MINOR_BITS) - 1);
		return 1;
	}

	void ShParserTimingInfoHeader(CBitReader *rb) {
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
	void ShParserDecoderModelInfo(CBitReader *rb) {
		m_decoder_model_info.buffer_delay_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
		m_decoder_model_info.num_units_in_decoding_tick = rb->AomRbReadUnsignedLiteral(32);  // Number of units in a decoding tick
		m_decoder_model_info.buffer_removal_time_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
		m_decoder_model_info.frame_presentation_time_length_minus_1 = rb->AomRbReadLiteral(5) + 1;
	}

	void ShParserOperatingParametersInfo(int op_num, CBitReader *rb) {
		// The cm->op_params array has MAX_NUM_OPERATING_POINTS + 1 elements.
		if (op_num > MAX_NUM_OPERATING_POINTS) {
			printf("AV1 does not support %d decoder model operating points",op_num + 1);
		}

		m_op_params[op_num].decoder_buffer_delay = rb->AomRbReadUnsignedLiteral(m_decoder_model_info.buffer_delay_length_minus_1);
		m_op_params[op_num].encoder_buffer_delay = rb->AomRbReadUnsignedLiteral(m_decoder_model_info.buffer_delay_length_minus_1);
		m_op_params[op_num].low_delay_mode_flag = rb->AomRbReadBit();
	}

	void ShParserSequenceInfo(CBitReader *rb) {
		m_frame_width_bits_minus_1 = rb->AomRbReadLiteral(4);
		m_frame_height_bits_minus_1 = rb->AomRbReadLiteral(4);
		m_max_frame_width_minus_1 = rb->AomRbReadLiteral(m_frame_width_bits_minus_1+1);
		m_max_frame_height_minus_1 = rb->AomRbReadLiteral(m_frame_height_bits_minus_1 + 1);

		if(m_reduced_still_picture_header)
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
			m_order_hint_bits_minus_1 = -1;
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

			if (m_enable_order_hint)
				m_order_hint_bits_minus_1 = rb->AomRbReadLiteral(3);
			else
				m_order_hint_bits_minus_1 = -1;
		}

		m_enable_superres = rb->AomRbReadBit();
		m_enable_cdef = rb->AomRbReadBit();
		m_enable_restoration = rb->AomRbReadBit();
	}


	BITSTREAM_PROFILE ShReadProfile() { return m_seq_profile; }
	int AvReadStillPicture() { return m_still_picture; }
	int ShReadReducedStillPictureHdr() { return m_reduced_still_picture_header; }
	int ShReadTimingInfoPresentFlag() { return m_timing_info_present_flag; }
	int ShReadDecoderModelInfoPresentFlag() { return m_decoder_model_info_present_flag; }
	int ShReadInitialDisplayDelayPresentFlag() { return m_initial_display_delay_present_flag; }
	int ShReadOperatingPointsCntMinus1() { return m_operating_points_cnt_minus_1; }
	int ShReadOperatingPointIdc(int idx) { return m_operating_point_idc[idx]; }
	BitstreamLevel ShReadSeqLevelIdx(int idx) { return m_seq_level_idx[idx]; }
	uint8_t ShReadSeqTier(int idx) { return m_seq_tier[idx]; }
	int ShReadDecoderModelPresentForThisOp(int idx) { return m_op_params[idx].decoder_model_present_for_this_op; }
	int ShReadInitialDisplayDelayPresentForThisOp(int idx) { return m_op_params[idx].initial_display_delay_present_for_this_op; }

	int ShReadSeqLevelIdxMajor(int idx) {
		return m_seq_level_idx[idx].major;
	}	
	int ShReadSeqLevelIdxMinor(int idx) {
		return m_seq_level_idx[idx].minor;
	}


	

private:
	BITSTREAM_PROFILE m_seq_profile;
	int m_still_picture;
	int m_reduced_still_picture_header;
	int m_timing_info_present_flag;
	int m_decoder_model_info_present_flag;
	int m_initial_display_delay_present_flag;

	int m_operating_points_cnt_minus_1;
	int m_operating_point_idc[MAX_NUM_OPERATING_POINTS];
	BitstreamLevel m_seq_level_idx[MAX_NUM_OPERATING_POINTS];
	uint8_t m_seq_tier[MAX_NUM_OPERATING_POINTS];
	//decoder_model_info()
	aom_dec_model_op_parameters_t m_op_params[MAX_NUM_OPERATING_POINTS + 1];
	timing_info_t m_timing_info;
	decoder_model_info_t m_decoder_model_info;

	int m_initial_display_delay_minus_1[MAX_NUM_OPERATING_POINTS];

	//choose_operating_point()
	int m_frame_width_bits_minus_1;
	int m_frame_height_bits_minus_1;
	int m_max_frame_width_minus_1;
	int m_max_frame_height_minus_1;
	int m_frame_id_numbers_present_flag;
	int m_delta_frame_id_length_minus_2;
	int m_additional_frame_id_length_minus_1;
	int m_use_128x128_superblock;  // Size of the superblock used for this frame
	int m_enable_filter_intra;
	int m_enable_intra_edge_filter;

	int m_enable_interintra_compound;
	int m_enable_masked_compound;
	int m_enable_warped_motion;
	int m_enable_dual_filter;
	int m_enable_order_hint;
	int m_enable_jnt_comp;
	int m_enable_ref_frame_mvs;
	int m_seq_choose_screen_content_tools;
	int m_seq_force_screen_content_tools;
	int m_seq_choose_integer_mv;
	int m_seq_force_integer_mv;
	int m_order_hint_bits_minus_1;

	int m_enable_superres;
	int m_enable_cdef;
	int m_enable_restoration;

	//color_config()
	int m_HighBitdepth;
	int m_TwelveBit;
	int m_MonoChrome;
	int m_ColorDescriptionPresentFlag;
	int m_ColorPrimaries;
	int m_TransferCharacteristics;
	int m_MatrixCoefficients;
	int m_ColorRange;
	int m_SubsamplingX;
	int m_SubsamplingY;
	int m_ChromaSamplePosition;
	int m_SeparateUvDeltaQ;
	
	int m_FilmGrainParamsPresent;
};

class ShManager
{
public:
	ShManager();
	~ShManager();

	void		Init();
	void		Destroy();

	void		storeSequenceHeader(CSequenceHeader *seqHeader);

	CSequenceHeader*	getStoredVPS(int id) { return	m_ShBuffer[0]; }

	CSequenceHeader*	getSequenceHeader() { return	m_ShBuffer[0]; };

private:

	CSequenceHeader*	m_ShBuffer[1];

	int			m_ShId;
};
