#pragma once
#include "bit_reader_c.h"
#include "Define.h"

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
	uint32_t FhParserUncompressedHeader(CBitReader *rb);

	void FhParserShowExistingFrame(int show_existing_frame) { m_show_existing_frame = show_existing_frame; }
	void FhParserFrameToShowMapIdx(int frame_to_show_map_idx) { m_frame_to_show_map_idx = frame_to_show_map_idx; }
	void FhParserDisplayFrameId(int display_frame_id) { m_display_frame_id = display_frame_id; }
	void FhParserFrameType(FRAME_TYPE frame_type) { m_frame_type = frame_type; }
	void FhParserShowFrame(int show_frame) { m_show_frame = show_frame; }
	void FhParserShowableFrame(int showable_frame) { m_showable_frame = showable_frame; }
	void FhParserErrorResilientMode(int error_resilient_mode) { m_error_resilient_mode = error_resilient_mode; }

	void FhParserTemporalPointInfo(int frame_presentation_time_length_minus_1, CBitReader *rb) {
		int n = frame_presentation_time_length_minus_1 + 1;
		m_frame_presentation_time = rb->AomRbReadLiteral(n);
	}
	void FhParserRefreshFrameFlags(int refresh_frame_flags) { m_refresh_frame_flags = refresh_frame_flags; }

	void FhSetRefValandOrderHint()
	{
		for (int i = 0; i < NUM_REF_FRAMES; i++) {
			m_RefValid[i] = 0;
			m_RefOrderHint[i] = 0;
		}
		for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
			m_OrderHints[LAST_FRAME + i] = 0;
		}
	}

	void FhParserDisableCdfUpdate(int disable_cdf_update) { m_disable_cdf_update = disable_cdf_update; }
	void FhParserAllowScreenContentTools(int allow_screen_content_tools) { m_allow_screen_content_tools = allow_screen_content_tools; }
	void FhParserForceIntegerMv(int force_integer_mv) { m_force_integer_mv = force_integer_mv; }
	
	void FhSetPrevFrameID() { m_PrevFrameID = m_current_frame_id; }
	void FhParserCurrentFrameId(int current_frame_id) { m_current_frame_id = current_frame_id; }

	void FhSetMarkRefFrames(int idLen, int delta_frame_id_length_minus_2) {
		int diffLen = delta_frame_id_length_minus_2 + 2;

		for (int i = 0; i < NUM_REF_FRAMES; i++) {
			if (m_current_frame_id >(1 << diffLen)) {
				if (m_RefFrameId[i] > m_current_frame_id ||
					m_RefFrameId[i] < (m_current_frame_id - (1 << diffLen)))
					m_RefValid[i] = 0;
			}
			else {
				if (m_RefFrameId[i] > m_current_frame_id &&
					m_RefFrameId[i] < ((1 << idLen) + m_current_frame_id - (1 << diffLen)))
					m_RefValid[i] = 0;
			}
		}
	}

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

	void FhParseRefOrderHint(int OrderHintBits, CBitReader *rb) {
		
		for (int i = 0; i < NUM_REF_FRAMES; i++) {
			m_ref_order_hint[i] = rb->AomRbReadLiteral(OrderHintBits);
			if (m_ref_order_hint[i] != m_RefOrderHint[i]) {
				m_RefValid[i] = 0;
			}
		}
	}

	void FhParserFrameSize(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
		int max_frame_width_minus_1, int max_frame_height_minus_1, 
		int enable_superres, CBitReader *rb) {
		
		if (m_frame_size_override_flag) {
			int n = frame_width_bits_minus_1 + 1;
			m_frame_width_minus_1 = rb->AomRbReadLiteral(n);
			n = frame_height_bits_minus_1 + 1;
			m_frame_height_minus_1 = rb->AomRbReadLiteral(n);
			m_FrameWidth = m_frame_width_minus_1 + 1;
			m_FrameHeight = m_frame_height_minus_1 + 1;
		}
		else {
			m_FrameWidth = max_frame_width_minus_1 + 1;
			m_FrameHeight = max_frame_height_minus_1 + 1;
		}

		FhParserSuperresParams(enable_superres, rb);
		FhComputeImageSize();
	}

	void FhParserSuperresParams(int enable_superres, CBitReader *rb) {
		if (enable_superres)
			m_use_superres = rb->AomRbReadBit();
		else
			m_use_superres = 0;

		if (m_use_superres) {
			m_coded_denom = rb->AomRbReadLiteral(SUPERRES_DENOM_BITS);
			m_SuperresDenom = m_coded_denom + SUPERRES_DENOM_MIN;
		}
		else {
			m_SuperresDenom = SUPERRES_NUM;
		}
		m_UpscaledWidth = m_FrameWidth;
		m_FrameWidth = (m_UpscaledWidth * SUPERRES_NUM + (m_SuperresDenom / 2)) / m_SuperresDenom;
	}

	void FhComputeImageSize(void) {
		m_MiCols = 2 * ((m_FrameWidth + 7) >> 3);
		m_MiRows = 2 * ((m_FrameHeight + 7) >> 3);
	}

	void FhRenderSize(CBitReader *rb) {
		m_render_and_frame_size_different = rb->AomRbReadBit();

		if (m_render_and_frame_size_different == 1) {
			m_render_width_minus_1 = rb->AomRbReadLiteral(16);
			m_render_height_minus_1 = rb->AomRbReadLiteral(16);
			m_RenderWidth = m_render_width_minus_1;
			m_RenderHeight = m_render_height_minus_1;
		}
		else {
			m_RenderWidth = m_UpscaledWidth;
			m_RenderHeight = m_FrameHeight;
		}
	}
		
	void FhParserFrameRefsShortSignaling(int frame_refs_short_signaling) {
		m_frame_refs_short_signaling = frame_refs_short_signaling;
	}
	void FhParserLastFrameIdx(int last_frame_idx) {
		m_last_frame_idx = last_frame_idx;
	}
	void FhParserGoldFrameIdx(int gold_frame_idx) {
		m_gold_frame_idx = gold_frame_idx;
	}
	void FhParserRefFramesIdx(int idx, int ref_frame_idx) { 
		m_ref_frame_idx[idx] = ref_frame_idx; }
	void FhParserDeltaFrameIdMinus1(int delta_frame_id_minus_1) {
		m_delta_frame_id_minus_1 = delta_frame_id_minus_1;
	}
	void FhParserExpectedFrameId(int idx, int expectedFrameId) {
		m_expectedFrameId[idx] = expectedFrameId;
	}

	void FhParserFrameSizeWithRefs(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
		int max_frame_width_minus_1, int max_frame_height_minus_1,
		int enable_superres, CBitReader *rb) {
		for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
		
			m_found_ref = rb->AomRbReadBit();

			if (m_found_ref == 1) {
				//UpscaledWidth = RefUpscaledWidth[ref_frame_idx[i]];
				//FrameWidth = UpscaledWidth;
				//FrameHeight = RefFrameHeight[ref_frame_idx[i]];
				//RenderWidth = RefRenderWidth[ref_frame_idx[i]];
				//RenderHeight = RefRenderHeight[ref_frame_idx[i]];
				break;
			}
		}
		if (m_found_ref == 0) {
			FhParserFrameSize(frame_width_bits_minus_1, frame_height_bits_minus_1,
				max_frame_width_minus_1, max_frame_height_minus_1,
				enable_superres, rb);
			FhRenderSize(rb);
		}
		else {
			FhParserSuperresParams(enable_superres, rb);
			FhComputeImageSize();
		}
	}

	void FhParserForceIntegerMv(CBitReader *rb) {
		if (m_force_integer_mv) {
			m_allow_high_precision_mv = 0;
		}
		else {
			m_allow_high_precision_mv = rb->AomRbReadBit();
		}
	}

	void FhParserInterpolationFilter(CBitReader *rb) {
		m_is_filter_switchable = rb->AomRbReadBit();

		if (m_is_filter_switchable == 1) {
			m_interpolation_filter = SWITCHABLE;
		}
		else {
			m_interpolation_filter = rb->AomRbReadLiteral(2);
		}
	}

	void FhParserIsMotionModeSwitchable(int is_motion_mode_switchable) {
		m_is_motion_mode_switchable = is_motion_mode_switchable;
	}
	void FhParserDisableFrameEndUpdateCdf(int disable_frame_end_update_cdf) {
		m_disable_frame_end_update_cdf = disable_frame_end_update_cdf;
	}

	void FhParserTileInfo(int use_128x128_superblock, CBitReader *rb) {
		m_sbCols = use_128x128_superblock ? ((m_MiCols + 31) >> 5) : ((m_MiCols + 15) >> 4);
		m_sbRows = use_128x128_superblock ? ((m_MiRows + 31) >> 5) : ((m_MiRows + 15) >> 4);
		m_sbShift = use_128x128_superblock ? 5 : 4;
		m_sbSize = m_sbShift + 2;
		m_maxTileWidthSb = MAX_TILE_WIDTH >> m_sbSize;
		m_maxTileAreaSb = MAX_TILE_AREA >> (2 * m_sbSize);
		m_minLog2TileCols = tile_log2(m_maxTileWidthSb, m_sbCols);
		m_maxLog2TileCols = tile_log2(1, AOMMIN(m_sbCols, MAX_TILE_COLS));
		m_maxLog2TileRows = tile_log2(1, AOMMIN(m_sbRows, MAX_TILE_ROWS));
		m_minLog2Tiles = AOMMAX(m_minLog2TileCols, tile_log2(m_maxTileAreaSb, m_sbRows * m_sbCols));

		m_uniform_tile_spacing_flag = rb->AomRbReadBit();
		if (m_uniform_tile_spacing_flag) {
			m_TileColsLog2 = m_minLog2TileCols;
				
			while (m_TileColsLog2 < m_maxLog2TileCols) {
				m_increment_tile_cols_log2 = rb->AomRbReadBit();
						
				if (m_increment_tile_cols_log2 == 1)
					m_TileColsLog2++;
				else
					break;
			}
			m_tileWidthSb = (m_sbCols + (1 << m_TileColsLog2) - 1) >> m_TileColsLog2;

			int i = 0;
			for (int startSb = 0; startSb < m_sbCols; startSb += m_tileWidthSb) {
				m_MiColStarts[i] = startSb << m_sbShift;
				i += 1;
			}
			m_MiColStarts[i] = m_MiCols;
			m_TileCols = i;

			m_minLog2TileRows = AOMMAX(m_minLog2Tiles - m_TileColsLog2, 0);
			m_TileRowsLog2 = m_minLog2TileRows;

			while (m_TileRowsLog2 < m_maxLog2TileRows) {
				m_increment_tile_rows_log2 = rb->AomRbReadBit();
				if (m_increment_tile_rows_log2 == 1)
					m_TileRowsLog2++;
				else
					break;
			}
			m_tileHeightSb = (m_sbRows + (1 << m_TileRowsLog2) - 1) >> m_TileRowsLog2;
			int i = 0; 
			for (int startSb = 0; startSb < m_sbRows; startSb += m_tileHeightSb) {
				m_MiRowStarts[i] = startSb << m_sbShift;
				i += 1;
			}
			m_MiRowStarts[i] = m_MiRows;
			m_TileRows = i;
		}
		else {
			m_widestTileSb = 0;
			int startSb = 0, i = 0;
			for (i = 0; startSb < m_sbCols; i++) {
				m_MiColStarts[i] = startSb << m_sbShift;
				int maxWidth = AOMMIN(m_sbCols - startSb, m_maxTileWidthSb);
				m_width_in_sbs_minus_1 = rb->AomRbReadUniform(maxWidth);
				int sizeSb = m_width_in_sbs_minus_1 + 1;
				m_widestTileSb = AOMMAX(sizeSb, m_widestTileSb);
				startSb += sizeSb;
			}
			m_MiColStarts[i] = m_MiCols;
			m_TileCols = i;
			m_TileColsLog2 = tile_log2(1, m_TileCols);

			if (m_minLog2Tiles > 0)
				m_maxTileAreaSb = (m_sbRows * m_sbCols) >> (m_minLog2Tiles + 1);
			else
				m_maxTileAreaSb = m_sbRows * m_sbCols;
			m_maxTileHeightSb = AOMMAX(m_maxTileAreaSb / m_widestTileSb, 1);

			startSb = 0;				
			for (i = 0; startSb < m_sbRows; i++) {
				m_MiRowStarts[i] = startSb << m_sbShift;
				int maxHeight = AOMMIN(m_sbRows - startSb, m_maxTileHeightSb);
				m_height_in_sbs_minus_1 = rb->AomRbReadUniform(maxHeight);
				int sizeSb = m_height_in_sbs_minus_1 + 1;
				startSb += sizeSb;
			}
			m_MiRowStarts[i] = m_MiRows;
			m_TileRows = i;
			m_TileRowsLog2 = tile_log2(1, m_TileRows);
		}
		if (m_TileColsLog2 > 0 || m_TileRowsLog2 > 0) {
			m_context_update_tile_id = rb->AomRbReadLiteral(m_TileRowsLog2 + m_TileColsLog2);
			m_tile_size_bytes_minus_1 = rb->AomRbReadLiteral(2);
			m_TileSizeBytes = m_tile_size_bytes_minus_1 + 1;
		}
		else {
			m_context_update_tile_id = 0;
		}
	}

	void FhParserQuantizationParams(int NumPlanes, int separate_uv_delta_q, CBitReader *rb) {
		m_base_q_idx = rb->AomRbReadLiteral(8);
		m_DeltaQYDc = FhReadDeltaQ(rb);

		if (NumPlanes > 1) {
			if (separate_uv_delta_q)
				m_diff_uv_delta = rb->AomRbReadBit();
			else
				m_diff_uv_delta = 0;

			m_DeltaQUDc = FhReadDeltaQ(rb);
			m_DeltaQUAc = FhReadDeltaQ(rb);

			if (m_diff_uv_delta) {
				m_DeltaQVDc = FhReadDeltaQ(rb);
				m_DeltaQVAc = FhReadDeltaQ(rb);
			}
			else {
				m_DeltaQVDc = m_DeltaQUDc;
				m_DeltaQVAc = m_DeltaQUAc;
			}
		}
		else {
			m_DeltaQUDc = 0;
			m_DeltaQUAc = 0;
			m_DeltaQVDc = 0;
			m_DeltaQVAc = 0;
		}
		m_using_qmatrix = rb->AomRbReadBit();

		if (m_using_qmatrix) {
			m_qm_y = rb->AomRbReadLiteral(4);
			m_qm_u = rb->AomRbReadLiteral(4);

			if (!separate_uv_delta_q)
				m_qm_v = m_qm_u;
			else
				m_qm_y = rb->AomRbReadLiteral(4);
		}
	}

	int FhReadDeltaQ(CBitReader *rb) {
		m_delta_coded = rb->AomRbReadBit();

		int delta_q = 0;
		if (m_delta_coded) {
			delta_q = AomRbReadInvSignedLiteral(6);
		}
		return delta_q;		
	}

	void FhParserSegmentationParams(CBitReader *rb) {
		m_segmentation_enabled = rb->AomRbReadBit();

		if (m_segmentation_enabled == 1) {
			//unimplemented
		}
		else {
			//unimplmented
		}
	/*	SegIdPreSkip = 0
			LastActiveSegId = 0
			for (i = 0; i < MAX_SEGMENTS; i++) {
				for (j = 0; j < SEG_LVL_MAX; j++) {
					if (FeatureEnabled[i][j]) {
						LastActiveSegId = i
							if (j >= SEG_LVL_REF_FRAME) {
								SegIdPreSkip = 1
							}
					}
				}*/
	}

	void FhParserDeltaQParams(CBitReader *rb) {
		m_delta_q_res = 0;
		m_delta_q_present = 0;

		if (m_base_q_idx > 0) {
			m_delta_q_present = rb->AomRbReadBit();
		}
		if (m_delta_q_present) {
			m_delta_q_res = AomRbReadInvSignedLiteral(2);
		}
	}

	void FhParserDeltaLfParams(CBitReader *rb) {
		m_delta_lf_present = 0;
		m_delta_lf_res = 0;
		m_delta_lf_multi = 0;

		if (m_delta_q_present) {
			if (!m_allow_intrabc)
				m_delta_lf_present = rb->AomRbReadBit();
			if (m_delta_lf_present) {
				m_delta_lf_res = rb->AomRbReadLiteral(2);
				m_delta_lf_multi = rb->AomRbReadBit();
			}
		}
	}

	void FhSetCodedLossless(int CodedLossless) {
		m_CodedLossless = CodedLossless;
	}
	void FhSetAllLossless(int AllLossless) {
		m_AllLossless = AllLossless;
	}

	

	void FhParserLoopFilterParams(int NumPlanes, CBitReader *rb) {
		
		if (m_CodedLossless || m_allow_intrabc) {
			m_loop_filter_level[0] = 0;
			m_loop_filter_level[1] = 0;
			m_loop_filter_ref_deltas[INTRA_FRAME] = 1;
			m_loop_filter_ref_deltas[LAST_FRAME] = 0;
			m_loop_filter_ref_deltas[LAST2_FRAME] = 0;
			m_loop_filter_ref_deltas[LAST3_FRAME] = 0;
			m_loop_filter_ref_deltas[BWDREF_FRAME] = 0;
			m_loop_filter_ref_deltas[GOLDEN_FRAME] = -1;
			m_loop_filter_ref_deltas[ALTREF_FRAME] = -1;
			m_loop_filter_ref_deltas[ALTREF2_FRAME] = -1;
			
			for (int i = 0; i < 2; i++) {
				m_loop_filter_mode_deltas[i] = 0;
			}
			return;
		}
		m_loop_filter_level[0] = rb->AomRbReadLiteral(6);
		m_loop_filter_level[1] = rb->AomRbReadLiteral(6);

		if (NumPlanes > 1) {
			if (m_loop_filter_level[0] || m_loop_filter_level[1]) {
				m_loop_filter_level[2] = rb->AomRbReadLiteral(6);
				m_loop_filter_level[3] = rb->AomRbReadLiteral(6);
			}
		}
		m_loop_filter_sharpness = rb->AomRbReadLiteral(3);
		m_loop_filter_delta_enabled = rb->AomRbReadBit();
		if (m_loop_filter_delta_enabled == 1) {
			m_loop_filter_delta_update = rb->AomRbReadBit();

			if (m_loop_filter_delta_update == 1) {
				for (int i = 0; i < TOTAL_REFS_PER_FRAME; i++) {
					m_update_ref_delta = rb->AomRbReadBit();
					if(m_update_ref_delta == 1)
						m_loop_filter_ref_deltas[i] = rb->AomRbReadInvSignedLiteral(6);
				}
			}
		}
	}

	void FhParserCdefParams(int NumPlanes, int enable_cdef, CBitReader *rb) {
		if (m_CodedLossless || m_allow_intrabc || !enable_cdef) {
			m_cdef_bits = 0;
			m_cdef_y_pri_strength[0] = 0;
			m_cdef_y_sec_strength[0] = 0;
			m_cdef_uv_pri_strength[0] = 0;
			m_cdef_uv_sec_strength[0] = 0;
			m_CdefDamping = 3;
			return;
		}
		m_cdef_damping_minus_3 = rb->AomRbReadLiteral(2);
		m_CdefDamping = m_cdef_damping_minus_3 + 3;
		m_cdef_bits = rb->AomRbReadLiteral(2);

		for (int i = 0; i < (1 << m_cdef_bits); i++) {
			m_cdef_y_pri_strength[i] = rb->AomRbReadLiteral(4);
			m_cdef_y_sec_strength[i] = rb->AomRbReadLiteral(2);
			if (m_cdef_y_sec_strength[i] == 3)
				m_cdef_y_sec_strength[i] += 1;

			if (NumPlanes > 1) {
				m_cdef_uv_pri_strength[i] = rb->AomRbReadLiteral(4);
				m_cdef_uv_sec_strength[i] = rb->AomRbReadLiteral(2);
				if (m_cdef_uv_sec_strength[i] == 3)
					m_cdef_uv_sec_strength[i] += 1;
			}
		}
	}

	void FhParserLrParams(int NumPlanes, int enable_restoration, int use_128x128_superblock,
		int subsampling_x, int subsampling_y, CBitReader *rb) {
		
		if (m_AllLossless || m_allow_intrabc ||	!enable_restoration) {
			m_FrameRestorationType[0] = RESTORE_NONE;
			m_FrameRestorationType[1] = RESTORE_NONE;
			m_FrameRestorationType[2] = RESTORE_NONE;
			m_UsesLr = 0;
			return;
		}
		m_UsesLr = 0;
		m_usesChromaLr = 0;

		for (int i = 0; i < NumPlanes; i++) {
			m_lr_type = rb->AomRbReadLiteral(2);
			m_FrameRestorationType[i] = Remap_Lr_Type[m_lr_type];
			if (m_FrameRestorationType[i] != RESTORE_NONE) {
				m_UsesLr = 1;
				if (i > 0) {
					m_usesChromaLr = 1;
				}
			}
		}

		if (m_UsesLr) {
			if (use_128x128_superblock) {
				m_lr_unit_shift = rb->AomRbReadBit();
				m_lr_unit_shift++;
			}
			else {
				m_lr_unit_shift = rb->AomRbReadBit();
				if (m_lr_unit_shift) {
					m_lr_unit_extra_shift = rb->AomRbReadBit();
					m_lr_unit_shift += m_lr_unit_extra_shift;
				}
			}
			m_LoopRestorationSize[0] = RESTORATION_TILESIZE_MAX >> (2 - m_lr_unit_shift);
			
			if (subsampling_x && subsampling_y && m_usesChromaLr) {
				m_lr_uv_shift = rb->AomRbReadBit();
			}
			else {
				m_lr_uv_shift = 0;
			}
			m_LoopRestorationSize[1] = m_LoopRestorationSize[0] >> m_lr_uv_shift;
			m_LoopRestorationSize[2] = m_LoopRestorationSize[0] >> m_lr_uv_shift;
		}
	}

	void FhParserTxModeSelect(CBitReader *rb) {
		if (m_CodedLossless == 1) {
			m_TxMode = ONLY_4X4;
		}
		else {
			m_tx_mode_select = rb->AomRbReadBit();
			if (m_tx_mode_select) {
				m_TxMode = TX_MODE_SELECT;
			}
			else {
				m_TxMode = TX_MODE_LARGEST;
			}

		}
	}

	void FhParserFrameReferenceMode(int FrameIsIntra, CBitReader *rb) {
		if (FrameIsIntra) {
			m_reference_select = 0;
		}
		else {
			m_reference_select = rb->AomRbReadBit();
		}
	}

	int FhReadShowExistingFrame() { return m_show_existing_frame; }
	int FhReadShowFrame() { return m_show_frame; }
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
	
private:
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
	int	m_maxLog2TileCols;
	int m_maxLog2TileRows;
	int	m_minLog2Tiles;
	int m_TileColsLog2; 
	int m_maxLog2TileCol;
	int m_tileWidthSb;

	int m_MiColStarts[MAX_TILE_COLS + 1];  // valid for 0 <= i <= tile_cols
	int m_MiRowStarts[MAX_TILE_ROWS + 1];  // valid for 0 <= i <= tile_rows
	int m_TileCols;
	int m_TileRows;

	int m_minLog2TileRows;
	int m_TileRowsLog2;
	int m_tileHeightSb;

	int m_widestTileSb;
	int m_maxTileHeightSb;
	int m_TileSizeBytes;

	int m_CodedLossless;
	int m_AllLossless;
};
