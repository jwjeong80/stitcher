#include <stdio.h>
#include <assert.h>
//#include "av1/decoder/obu.h"
//#include "av1/common/enums.h"
//#include "bit_reader.h"
//#include "av1/common/timing.h"
#include "bit_reader_c.h"
#include "Define.h"
//#include "av1_common.h"
///////////////////////////////////////////////////////////////////////////////

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


/*!\brief List of supported color primaries */
typedef enum aom_color_primaries {
	AOM_CICP_CP_RESERVED_0 = 0,  /**< For future use */
	AOM_CICP_CP_BT_709 = 1,      /**< BT.709 */
	AOM_CICP_CP_UNSPECIFIED = 2, /**< Unspecified */
	AOM_CICP_CP_RESERVED_3 = 3,  /**< For future use */
	AOM_CICP_CP_BT_470_M = 4,    /**< BT.470 System M (historical) */
	AOM_CICP_CP_BT_470_B_G = 5,  /**< BT.470 System B, G (historical) */
	AOM_CICP_CP_BT_601 = 6,      /**< BT.601 */
	AOM_CICP_CP_SMPTE_240 = 7,   /**< SMPTE 240 */
	AOM_CICP_CP_GENERIC_FILM =
	8, /**< Generic film (color filters using illuminant C) */
	AOM_CICP_CP_BT_2020 = 9,      /**< BT.2020, BT.2100 */
	AOM_CICP_CP_XYZ = 10,         /**< SMPTE 428 (CIE 1921 XYZ) */
	AOM_CICP_CP_SMPTE_431 = 11,   /**< SMPTE RP 431-2 */
	AOM_CICP_CP_SMPTE_432 = 12,   /**< SMPTE EG 432-1  */
	AOM_CICP_CP_RESERVED_13 = 13, /**< For future use (values 13 - 21)  */
	AOM_CICP_CP_EBU_3213 = 22,    /**< EBU Tech. 3213-E  */
	AOM_CICP_CP_RESERVED_23 = 23  /**< For future use (values 23 - 255)  */
} aom_color_primaries_t;        /**< alias for enum aom_color_primaries */

								/*!\brief List of supported transfer functions */
typedef enum aom_transfer_characteristics {
	AOM_CICP_TC_RESERVED_0 = 0,  /**< For future use */
	AOM_CICP_TC_BT_709 = 1,      /**< BT.709 */
	AOM_CICP_TC_UNSPECIFIED = 2, /**< Unspecified */
	AOM_CICP_TC_RESERVED_3 = 3,  /**< For future use */
	AOM_CICP_TC_BT_470_M = 4,    /**< BT.470 System M (historical)  */
	AOM_CICP_TC_BT_470_B_G = 5,  /**< BT.470 System B, G (historical) */
	AOM_CICP_TC_BT_601 = 6,      /**< BT.601 */
	AOM_CICP_TC_SMPTE_240 = 7,   /**< SMPTE 240 M */
	AOM_CICP_TC_LINEAR = 8,      /**< Linear */
	AOM_CICP_TC_LOG_100 = 9,     /**< Logarithmic (100 : 1 range) */
	AOM_CICP_TC_LOG_100_SQRT10 =
	10,                     /**< Logarithmic (100 * Sqrt(10) : 1 range) */
	AOM_CICP_TC_IEC_61966 = 11, /**< IEC 61966-2-4 */
	AOM_CICP_TC_BT_1361 = 12,   /**< BT.1361 */
	AOM_CICP_TC_SRGB = 13,      /**< sRGB or sYCC*/
	AOM_CICP_TC_BT_2020_10_BIT = 14, /**< BT.2020 10-bit systems */
	AOM_CICP_TC_BT_2020_12_BIT = 15, /**< BT.2020 12-bit systems */
	AOM_CICP_TC_SMPTE_2084 = 16,     /**< SMPTE ST 2084, ITU BT.2100 PQ */
	AOM_CICP_TC_SMPTE_428 = 17,      /**< SMPTE ST 428 */
	AOM_CICP_TC_HLG = 18,            /**< BT.2100 HLG, ARIB STD-B67 */
	AOM_CICP_TC_RESERVED_19 = 19     /**< For future use (values 19-255) */
} aom_transfer_characteristics_t;  /**< alias for enum aom_transfer_function */

								   /*!\brief List of supported matrix coefficients */
typedef enum aom_matrix_coefficients {
	AOM_CICP_MC_IDENTITY = 0,    /**< Identity matrix */
	AOM_CICP_MC_BT_709 = 1,      /**< BT.709 */
	AOM_CICP_MC_UNSPECIFIED = 2, /**< Unspecified */
	AOM_CICP_MC_RESERVED_3 = 3,  /**< For future use */
	AOM_CICP_MC_FCC = 4,         /**< US FCC 73.628 */
	AOM_CICP_MC_BT_470_B_G = 5,  /**< BT.470 System B, G (historical) */
	AOM_CICP_MC_BT_601 = 6,      /**< BT.601 */
	AOM_CICP_MC_SMPTE_240 = 7,   /**< SMPTE 240 M */
	AOM_CICP_MC_SMPTE_YCGCO = 8, /**< YCgCo */
	AOM_CICP_MC_BT_2020_NCL =
	9, /**< BT.2020 non-constant luminance, BT.2100 YCbCr  */
	AOM_CICP_MC_BT_2020_CL = 10, /**< BT.2020 constant luminance */
	AOM_CICP_MC_SMPTE_2085 = 11, /**< SMPTE ST 2085 YDzDx */
	AOM_CICP_MC_CHROMAT_NCL =
	12, /**< Chromaticity-derived non-constant luminance */
	AOM_CICP_MC_CHROMAT_CL = 13, /**< Chromaticity-derived constant luminance */
	AOM_CICP_MC_ICTCP = 14,      /**< BT.2100 ICtCp */
	AOM_CICP_MC_RESERVED_15 = 15 /**< For future use (values 15-255)  */
} aom_matrix_coefficients_t;

typedef enum aom_bit_depth {
	AOM_BITS_8 = 8,   /**<  8 bits */
	AOM_BITS_10 = 10, /**< 10 bits */
	AOM_BITS_12 = 12, /**< 12 bits */
} aom_bit_depth_t;

typedef enum aom_chroma_sample_position {
	AOM_CSP_UNKNOWN = 0,          /**< Unknown */
	AOM_CSP_VERTICAL = 1,         /**< Horizontally co-located with luma(0, 0)*/
								  /**< sample, between two vertical samples */
	AOM_CSP_COLOCATED = 2,        /**< Co-located with luma(0, 0) sample */
	AOM_CSP_RESERVED = 3          /**< Reserved value */
} aom_chroma_sample_position_t; /**< alias for enum aom_transfer_function */

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
	void ShParserFilmGrainParamsPresent(int film_grain_params_present) { m_film_grain_params_present = film_grain_params_present; }

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

	void ShParserColorConfig(CBitReader *rb) {
		
		m_high_bitdepth = rb->AomRbReadBit();

		if (m_seq_profile == PROFILE_2 && m_high_bitdepth) {
			m_twelve_bit = rb->AomRbReadBit();
			m_BitDepth = m_twelve_bit ? AOM_BITS_12 : AOM_BITS_10;
		}
		else if(m_seq_profile <= PROFILE_2){
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



	//int AreSeqHeadersConsistent(const SequenceHeader *seq_params_old,
	//	const SequenceHeader *seq_params_new) {
	//	return !memcmp(seq_params_old, seq_params_new, sizeof(CSequenceHeader));
	//}


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

	int ShReadFrameIdNumbersPresentFlag() { return m_frame_id_numbers_present_flag; }
	int ShReadAdditionalFrameIdLengthMinus1() { return m_additional_frame_id_length_minus_1; }
	int ShReadDeltaFrameIdLengthMinus2() { return m_delta_frame_id_length_minus_2; }
	int ShReadEqualPictureInterval() { return m_timing_info.equal_picture_interval; }
	int ShReadSeqForceScreenContentTools() { return m_seq_force_screen_content_tools; }

	int ShReadSeqLevelIdxMajor(int idx) {
		return m_seq_level_idx[idx].major;
	}	
	int ShReadSeqLevelIdxMinor(int idx) {
		return m_seq_level_idx[idx].minor;
	}

	decoder_model_info_t ShReadDecoderModelInfo() { return m_decoder_model_info; }
	
	int ShReadFilmGrainParamsPresent() { return m_film_grain_params_present; }
	int ShReadSeqForceIntegerMv() { return m_seq_force_integer_mv; }
	int ShReadOrderHintBits() { return m_OrderHintBits; }
	int ShReadEnableOrderHint() { return m_enable_order_hint; }

	int ShReadFrameWidthBitsMinus1() { return m_frame_width_bits_minus_1; }
	int ShReadFrameHeightBitsMinus1() { return m_frame_height_bits_minus_1; }
	int ShReadMaxFrameWidthMinus1() { return m_max_frame_width_minus_1; }
	int ShReadMaxFrameHeightMinus1() { return m_max_frame_height_minus_1; }

	int ShReadEnableSuperres() { return m_enable_superres; }

	
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
	int m_high_bitdepth;
	int m_twelve_bit;
	int m_mono_chrome;
	int m_color_description_present_flag;
	int m_color_primaries;
	int m_transfer_characteristics;
	int m_matrix_coefficients;
	int m_color_range;
	int m_subsampling_x;
	int m_subsampling_y;
	int m_chroma_sample_position;
	int m_separate_uv_delta_q;
	int m_film_grain_params_present;

	int m_BitDepth;
	int m_NumPlanes;
	int m_OrderHintBits;

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
