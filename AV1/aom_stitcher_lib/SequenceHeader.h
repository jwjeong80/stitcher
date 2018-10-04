#include <stdio.h>
#include <assert.h>
//#include "av1/decoder/obu.h"
//#include "av1/common/enums.h"
//#include "bit_reader.h"
//#include "av1/common/timing.h"
#include "bit_reader_c.h"
#include "Define.h"
#include "bit_writer.h"
//#include "av1_common.h"
///////////////////////////////////////////////////////////////////////////////

// The following three profiles are currently defined.
// Profile 0.  8-bit and 10-bit 4:2:0 and 4:0:0 only.
// Profile 1.  8-bit and 10-bit 4:4:4
// Profile 2.  8-bit and 10-bit 4:2:2
//            12-bit  4:0:0, 4:2:2 and 4:4:4
// Since we have three bits for the profiles, it can be extended later.


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
	
	void ShInitialize();


	int is_valid_seq_level_idx(uint8_t seq_level_idx) {
		return seq_level_idx < 24 || seq_level_idx == 31;
	}

	// Sequence header parser 
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
	int ShParserSeqLevelIdx(int idx, CBitReader *rb);
	void ShParserTimingInfoHeader(CBitReader *rb);
	void ShParserDecoderModelInfo(CBitReader *rb);
	void ShParserOperatingParametersInfo(int op_num, CBitReader *rb);
	void ShParserSequenceInfo(CBitReader *rb);
	void ShParserColorConfig(CBitReader *rb);


	// Sequence header reader 
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

	int ShReadSeqLevelIdxMajor(int idx) {return m_seq_level_idx[idx].major;}	
	int ShReadSeqLevelIdxMinor(int idx) {return m_seq_level_idx[idx].minor;}
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
	int ShReadEnableRefFrameMvs() { return m_enable_ref_frame_mvs; }
	int ShReadUse128x128Superblock() { return m_use_128x128_superblock; }
	int ShReadSeparateUvDeltaQ() { return m_separate_uv_delta_q; }
	int ShReadNumPlanes() { return m_NumPlanes; }
	int ShReadEnableCdef() { return m_enable_cdef; }
	int ShReadEnableRestoration() { return m_enable_restoration; }
	int ShReadSubsamplingX() { return m_subsampling_x; }
	int ShReadSubsamplingY() { return m_subsampling_y; }
	int ShReadEnableWarpedMotion() { return m_enable_warped_motion; }

	int ShReadFrameWidth() { return m_FrameWidth; }
	int ShReadFrameHeight() { return m_FrameHeight; }
	
	// Sequence header writer 
	uint32_t write_sequence_header_obu(FrameSize_t *tileSizes, uint8_t *const dst, int bit_buffer_offset);
	void write_profile(BITSTREAM_PROFILE profile, CBitWriter *wb);
	void write_bitstream_level(BitstreamLevel bl, CBitWriter *wb);
	uint8_t major_minor_to_seq_level_idx(BitstreamLevel bl);
	void write_timing_info_header(CBitWriter *wb);
	void write_decoder_model_info(CBitWriter *wb);
	void write_dec_model_op_parameters(CBitWriter *wb, int op_num);
	void write_sequence_header(FrameSize_t *tileSizes, CBitWriter *wb);
	void write_color_config(CBitWriter *wb);
	
	uint32_t SequencHeaderCompare(CSequenceHeader *pSh);


	int m_ParserIdx;
	uint32_t m_uiNumTileRows;
	uint32_t m_uiNumTileCols;
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
	int m_FrameWidth;
	int m_FrameHeight;
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
