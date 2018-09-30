#pragma once
#include "bit_reader_c.h"
#include "Define.h"
#include "bit_writer.h"

#define NONE_FRAME -1
#define INTRA_FRAME 0
#define LAST_FRAME 1
#define LAST2_FRAME 2
#define LAST3_FRAME 3
#define GOLDEN_FRAME 4
#define BWDREF_FRAME 5
#define ALTREF2_FRAME 6
#define ALTREF_FRAME 7
#define EXTREF_FRAME REF_FRAMES
#define LAST_REF_FRAMES (LAST3_FRAME - LAST_FRAME + 1)

#define INTER_REFS_PER_FRAME (ALTREF_FRAME - LAST_FRAME + 1)

static int tile_log2(int blk_size, int target) {
	int k;
	for (k = 0; (blk_size << k) < target; k++) {
	}
	return k;
}

class CFrameHeader : CBitReader
{
public:

	void FhParserShowExistingFrame(int show_existing_frame) { m_show_existing_frame = show_existing_frame; }
	void FhParserFrameToShowMapIdx(int frame_to_show_map_idx) { m_frame_to_show_map_idx = frame_to_show_map_idx; }
	void FhParserDisplayFrameId(int display_frame_id) { m_display_frame_id = display_frame_id; }
	void FhParserFrameType(FRAME_TYPE frame_type) { m_frame_type = frame_type; }
	void FhParserShowFrame(int show_frame) { m_show_frame = show_frame; }
	void FhParserShowableFrame(int showable_frame) { m_showable_frame = showable_frame; }
	void FhParserErrorResilientMode(int error_resilient_mode) { m_error_resilient_mode = error_resilient_mode; }

	void FhParserTemporalPointInfo(int frame_presentation_time_length_minus_1, CBitReader *rb);
	void FhParserRefreshFrameFlags(int refresh_frame_flags) { m_refresh_frame_flags = refresh_frame_flags; }

	void FhSetRefValandOrderHint();

	void FhParserDisableCdfUpdate(int disable_cdf_update) { m_disable_cdf_update = disable_cdf_update; }
	void FhParserAllowScreenContentTools(int allow_screen_content_tools) { m_allow_screen_content_tools = allow_screen_content_tools; }
	void FhParserForceIntegerMv(int force_integer_mv) { m_force_integer_mv = force_integer_mv; }
	
	void FhSetPrevFrameID() { m_PrevFrameID = m_current_frame_id; }
	void FhParserCurrentFrameId(int current_frame_id) { m_current_frame_id = current_frame_id; }

	void FhSetMarkRefFrames(int idLen, int delta_frame_id_length_minus_2);

	void FhParserFrameSizeOverrideFlag(int frame_size_override_flag) { m_frame_size_override_flag = frame_size_override_flag; }
	void FhParserOrderHint(int order_hint) { m_order_hint = order_hint; }
	void FhParserPrimaryRefFrame(int primary_ref_frame) { m_primary_ref_frame = primary_ref_frame; }
	void FhParserBufferRemovalTimePresentFlag(int buffer_removal_time_present_flag) { 
		m_buffer_removal_time_present_flag = buffer_removal_time_present_flag; }
	
	void FhParserBufferRemovalTime(int op_num, int buffer_removal_time) {
		m_buffer_removal_time[op_num] = buffer_removal_time;
	}

	void FhParserAllowHighPrecisionMv(int allow_high_precision_mv) { m_allow_high_precision_mv = allow_high_precision_mv; }
	void FhParserUseRefFrameMvs(int use_ref_frame_mvs) { m_use_ref_frame_mvs = use_ref_frame_mvs; }
	void FhParserAllowIntrabc(int allow_intrabc) { m_allow_intrabc = allow_intrabc; }

	void FhParseRefOrderHint(int OrderHintBits, CBitReader *rb);

	void FhParserFrameSize(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
		int max_frame_width_minus_1, int max_frame_height_minus_1,
		int enable_superres, CBitReader *rb);

	void FhParserSuperresParams(int enable_superres, CBitReader *rb);

	void FhComputeImageSize(void);
	void FhRenderSize(CBitReader *rb);
		
	void FhParserFrameRefsShortSignaling(int frame_refs_short_signaling) { m_frame_refs_short_signaling = frame_refs_short_signaling; }
	void FhParserLastFrameIdx(int last_frame_idx) { m_last_frame_idx = last_frame_idx; }
	void FhParserGoldFrameIdx(int gold_frame_idx) { m_gold_frame_idx = gold_frame_idx; }
	void FhParserRefFramesIdx(int idx, int ref_frame_idx) { m_ref_frame_idx[idx] = ref_frame_idx; }
	void FhParserDeltaFrameIdMinus1(int delta_frame_id_minus_1) { m_delta_frame_id_minus_1 = delta_frame_id_minus_1; }
	void FhParserExpectedFrameId(int idx, int expectedFrameId) { m_expectedFrameId[idx] = expectedFrameId; }

	void FhParserFrameSizeWithRefs(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
		int max_frame_width_minus_1, int max_frame_height_minus_1,
		int enable_superres, CBitReader *rb);

	void FhParserForceIntegerMv(CBitReader *rb);
	void FhParserInterpolationFilter(CBitReader *rb);

	void FhParserIsMotionModeSwitchable(int is_motion_mode_switchable) { m_is_motion_mode_switchable = is_motion_mode_switchable; }
	void FhParserDisableFrameEndUpdateCdf(int disable_frame_end_update_cdf) { m_disable_frame_end_update_cdf = disable_frame_end_update_cdf; }

	void FhParserTileInfo(int use_128x128_superblock, CBitReader *rb);
	void FhParserQuantizationParams(int NumPlanes, int separate_uv_delta_q, CBitReader *rb);
	int FhReadDeltaQ(CBitReader *rb);

	void FhParserSegmentationParams(CBitReader *rb);
	void FhParserDeltaQParams(CBitReader *rb);
	void FhParserDeltaLfParams(CBitReader *rb);

	void FhSetCodedLossless(int CodedLossless) { m_CodedLossless = CodedLossless; }
	void FhSetAllLossless(int AllLossless) { m_AllLossless = AllLossless; }

	void FhParserLoopFilterParams(int NumPlanes, CBitReader *rb);
	void FhParserCdefParams(int NumPlanes, int enable_cdef, CBitReader *rb);
	void FhParserLrParams(int NumPlanes, int enable_restoration, int use_128x128_superblock,
		int subsampling_x, int subsampling_y, CBitReader *rb);

	void FhParserTxMode(CBitReader *rb);

	void FhParserFrameReferenceMode(int FrameIsIntra, CBitReader *rb);
	void FhParserSkipModeParams(int FrameIsIntra, int enable_order_hint, CBitReader *rb);

	void FhParserAllowWarpedMotion(int allow_warped_motion) { m_allow_warped_motion = allow_warped_motion; }
	void FhParserReducedTxSet(int reduced_tx_set) { m_reduced_tx_set = reduced_tx_set; }
	void FhParserGlobalMotionParams(int FrameIsIntra, CBitReader *rb);
	//tile_group_obu()
	void FhParserTileStartAndEndPresentFlag(int tile_start_and_end_present_flag) { m_tile_start_and_end_present_flag = tile_start_and_end_present_flag;	}

	void FhSetIdLen(int idLen) { m_idLen = idLen; }

	//Read uncompressed header 
	int FhReadShowExistingFrame() { return m_show_existing_frame; }
	int FhReadShowFrame() { return m_show_frame; }
	int FhReadShowableFrame() { return m_showable_frame; }
	int FhReadFrameToShowMapIdx() { return m_frame_to_show_map_idx; }
	FRAME_TYPE FhReadFrameType() { return m_frame_type; }
	int FhReadAllowScreenContentTools() { return m_allow_screen_content_tools; }
	int FhReadOrderHint() { return m_order_hint; }
	int FhReadErrorResilientMode() { return m_error_resilient_mode; }
	int FhReadRefreshFrameFlags() { return m_refresh_frame_flags; }

	int FhReadUpscaledWidth() { return m_UpscaledWidth; }
	int FhReadFrameWidth() { return m_FrameWidth; }

	int FhReadCurrentFrameId() { return m_current_frame_id; }
	int FhReadFrameSizeOverrideFlag() { return m_frame_size_override_flag;}
	int FhReadDisableCdfUpdate() { return m_disable_cdf_update;}

	int FhReadPrimaryRefFrame() { return m_primary_ref_frame; }
	int FhReadUseRefFrameMvs() { return m_use_ref_frame_mvs; }

	int FhReadCodedLossless() { return m_CodedLossless; }
	int FhReadAllLossless() { return m_AllLossless; }

	int FhReadTileCols() { return m_TileCols; }
	int FhReadTileRows() { return m_TileRows; }

	int FhReadTileColsLog2() { return m_TileColsLog2; }
	int FhReadTileRowsLog2() { return m_TileRowsLog2; }
	
	int FhReadIdLen() { return m_idLen; }
	int FhReadDisplayFrameId() { return m_display_frame_id; }
	int FhReadForceIntegerMv() { return m_force_integer_mv; }
	int FhReadBufferRemovalTimePresentFlag() { return m_buffer_removal_time_present_flag;}
	int FhReadBufferRemovalTime(int op_num) { return m_buffer_removal_time[op_num]; }
	int FhReadRefOrderHint(int idx) { return m_ref_order_hint[idx]; }


	//write uncompressed header
	uint32_t write_uncompressed_header_obu(uint8_t *const dst, int bit_buffer_offset);
	void write_temporal_point_info(int frame_presentation_time_length_minus_1, CBitWriter *wb);
private:
	int m_idLen;

	int m_show_existing_frame;
	int m_frame_to_show_map_idx;
	int m_display_frame_id;
	FRAME_TYPE m_frame_type;
	int m_show_frame;
	int m_showable_frame;
	int m_error_resilient_mode;

	//temporal_point_info( )
	int m_frame_presentation_time;

	int m_refresh_frame_flags;

	int m_disable_cdf_update;
	int m_allow_screen_content_tools;
	int m_force_integer_mv;
	int m_current_frame_id;
	int m_frame_size_override_flag;
	int m_order_hint;
	int m_primary_ref_frame;

	int m_buffer_removal_time_present_flag;
	int m_buffer_removal_time[MAX_NUM_OPERATING_POINTS];
	int m_refresh_frame_flagss;
	int m_ref_order_hint[INTER_REFS_PER_FRAME];

	int m_allow_high_precision_mv;
	int m_use_ref_frame_mvs;
	int m_allow_intrabc;

	//frame_size()
	int m_frame_width_minus_1;
	int m_frame_height_minus_1;

	//superres_params()
	int m_use_superres;
	int m_coded_denom;

	//render_size( )
	int m_render_and_frame_size_different;
	int m_render_width_minus_1;
	int m_render_height_minus_1;

	int m_frame_refs_short_signaling;
	int m_last_frame_idx;
	int m_gold_frame_idx;
	int m_ref_frame_idx[INTER_REFS_PER_FRAME];
	int m_delta_frame_id_minus_1;

	//frame_size_with_refs( )
	int m_found_ref;

	//read_interpolation_filter( )
	int m_is_filter_switchable;
	int m_interpolation_filter;

	int m_is_motion_mode_switchable;

	int m_disable_frame_end_update_cdf;

	//tile_info ( )
	int m_uniform_tile_spacing_flag;
	int m_increment_tile_cols_log2;
	int m_increment_tile_rows_log2;
	int m_width_in_sbs_minus_1;
	int m_height_in_sbs_minus_1;
	int m_context_update_tile_id;
	int m_tile_size_bytes_minus_1;

	//quantization_params()
	int m_base_q_idx;
	int m_diff_uv_delta;
	int m_using_qmatrix;
	int m_qm_y;
	int m_qm_u;
	int m_qm_v;
	//read_delta_q()
	int m_delta_coded;
	int m_delta_q;

	int m_DeltaQYDc;
	int m_DeltaQUDc;
	int m_DeltaQUAc;
	int m_DeltaQVDc;
	int m_DeltaQVAc;


	//segmentation_params()
	int m_segmentation_enabled;
	int m_segmentation_update_map;
	int m_segmentation_temporal_update;
	int m_segmentation_update_data;
	int m_feature_enabled;
	int m_feature_value;

	//delta_q_params()
	int m_delta_q_present;
	int m_delta_q_res;

	//delta_lf_params()
	int m_delta_lf_present;
	int m_delta_lf_res;
	int m_delta_lf_multi;

	//loop_filter_params()
	int m_loop_filter_level[4];
	int m_loop_filter_sharpness;
	int m_loop_filter_delta_enabled;
	int m_loop_filter_delta_update;
	int m_update_ref_delta;
	int m_loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME];
	int m_update_mode_delta;
	int m_loop_filter_mode_deltas[2];

	int m_cdef_damping_minus_3;
	int m_cdef_bits;
	int m_cdef_y_pri_strength[8];
	int m_cdef_y_sec_strength[8];
	int m_cdef_uv_pri_strength[8];
	int m_cdef_uv_sec_strength[8];
	int m_CdefDamping;

	int m_lr_type;
	int m_lr_unit_shift;
	int m_lr_unit_extra_shift;
	int m_lr_uv_shift;
	RestorationType m_FrameRestorationType[3];
	int m_LoopRestorationSize[3];
	int m_UsesLr;
	int m_usesChromaLr;

	//read_tx_mode()
	int m_tx_mode_select;
	TX_MODE m_TxMode;

	//frame_reference_mode( )
	int m_reference_select;

	//skip_mode_params()
	int m_skipModeAllowed;
	int m_forwardIdx;
	int m_backwardIdx;
	int m_forwardHint;
	int m_SkipModeFrame[2];
	int m_secondForwardIdx;
	int m_secondForwardHint;
	int m_skip_mode_present;

	int m_allow_warped_motion;
	int m_reduced_tx_set;

	//global_motion_params( )
	int m_is_global;
	int m_is_rot_zoom;
	int m_is_translation;
	TransformationType m_GmType[INTER_REFS_PER_FRAME];

	int m_RefValid[NUM_REF_FRAMES];
	int	m_RefOrderHint[NUM_REF_FRAMES];
	int m_RefFrameId[NUM_REF_FRAMES];

	int m_OrderHints[INTER_REFS_PER_FRAME];

	int m_PrevFrameID;
	int m_FrameWidth;
	int m_FrameHeight;
	int m_SuperresDenom;
	int m_UpscaledWidth;
	int m_RenderWidth;
	int m_RenderHeight;
	int m_expectedFrameId[INTER_REFS_PER_FRAME];

	int m_MiCols;
	int m_MiRows;

	int m_sbCols;
	int m_sbRows;
	int m_sbShift;
	int	m_sbSize;
	int	m_maxTileWidthSb;
	int	m_maxTileAreaSb;
	int	m_minLog2TileCols;
	int m_minLog2TileRows;
	int	m_maxLog2TileCols;
	int m_maxLog2TileRows;
	int	m_minLog2Tiles;
	int m_TileColsLog2; 
	int m_TileRowsLog2;
	int m_maxLog2TileCol;
	int m_tileWidthSb;

	int m_MiColStarts[MAX_TILE_COLS + 1];  // valid for 0 <= i <= tile_cols
	int m_MiRowStarts[MAX_TILE_ROWS + 1];  // valid for 0 <= i <= tile_rows
	int m_TileCols;
	int m_TileRows;

	int m_tileHeightSb;
	int m_widestTileSb;
	int m_maxTileHeightSb;
	int m_TileSizeBytes;

	int m_CodedLossless;
	int m_AllLossless;

	//tile_group_obu(sz)
	int m_tile_start_and_end_present_flag;
};
