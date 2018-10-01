#include "FrameHeader.h"
#include <stdio.h>

void CFrameHeader::FhSetRefValandOrderHint()
{
	for (int i = 0; i < NUM_REF_FRAMES; i++) {
		m_RefValid[i] = 0;
		m_RefOrderHint[i] = 0;
	}
	for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
		m_OrderHints[LAST_FRAME + i] = 0;
	}
}

void CFrameHeader::FhSetMarkRefFrames(int idLen, int delta_frame_id_length_minus_2) {
	int diffLen = delta_frame_id_length_minus_2 + 2;

	for (int i = 0; i < NUM_REF_FRAMES; i++) {
		if (m_current_frame_id > (1 << diffLen)) {
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

void CFrameHeader::FhParseRefOrderHint(int OrderHintBits, CBitReader *rb) {

	for (int i = 0; i < NUM_REF_FRAMES; i++) {
		m_ref_order_hint[i] = rb->AomRbReadLiteral(OrderHintBits);
		if (m_ref_order_hint[i] != m_RefOrderHint[i]) {
			m_RefValid[i] = 0;
		}
	}
}

void CFrameHeader::FhParserFrameSize(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
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

void CFrameHeader::FhParserSuperresParams(int enable_superres, CBitReader *rb) {
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

void CFrameHeader::FhRenderSize(CBitReader *rb) {
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

void CFrameHeader::FhComputeImageSize(void) {
	m_MiCols = 2 * ((m_FrameWidth + 7) >> 3);
	m_MiRows = 2 * ((m_FrameHeight + 7) >> 3);
}

void CFrameHeader::FhParserFrameSizeWithRefs(int frame_width_bits_minus_1, int frame_height_bits_minus_1,
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

void CFrameHeader::FhParserForceIntegerMv(CBitReader *rb) {
	if (m_force_integer_mv) {
		m_allow_high_precision_mv = 0;
	}
	else {
		m_allow_high_precision_mv = rb->AomRbReadBit();
	}
}

void CFrameHeader::FhParserInterpolationFilter(CBitReader *rb) {
	m_is_filter_switchable = rb->AomRbReadBit();

	if (m_is_filter_switchable == 1) {
		m_interpolation_filter = SWITCHABLE;
	}
	else {
		m_interpolation_filter = rb->AomRbReadLiteral(2);
	}
}

void CFrameHeader::FhParserTileInfo(int use_128x128_superblock, CBitReader *rb) {
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
		i = 0;
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

void CFrameHeader::FhParserQuantizationParams(int NumPlanes, int separate_uv_delta_q, CBitReader *rb) {
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

int CFrameHeader::FhReadDeltaQ(CBitReader *rb) {
	m_delta_coded = rb->AomRbReadBit();

	int delta_q = 0;
	if (m_delta_coded) {
		delta_q = AomRbReadInvSignedLiteral(6);
	}
	return delta_q;
}

void CFrameHeader::FhParserSegmentationParams(CBitReader *rb) {
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

void CFrameHeader::FhParserDeltaQParams(CBitReader *rb) {
	m_delta_q_res = 0;
	m_delta_q_present = 0;

	if (m_base_q_idx > 0) {
		m_delta_q_present = rb->AomRbReadBit();
	}
	if (m_delta_q_present) {
		m_delta_q_res = AomRbReadInvSignedLiteral(2);
	}
}

void CFrameHeader::FhParserDeltaLfParams(CBitReader *rb) {
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

void CFrameHeader::FhParserLoopFilterParams(int NumPlanes, CBitReader *rb) {

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
				m_update_ref_delta[i] = rb->AomRbReadBit();
				if (m_update_ref_delta[i] == 1)
					m_loop_filter_ref_deltas[i] = rb->AomRbReadInvSignedLiteral(6);
			}
			for (int i = 0; i < 2; i++) {
				m_update_mode_delta[i] = rb->AomRbReadBit();
				if (m_update_mode_delta[i] == 1)
					m_loop_filter_mode_deltas[i] = rb->AomRbReadInvSignedLiteral(6);
			}
		}
	}
}

void CFrameHeader::FhParserCdefParams(int NumPlanes, int enable_cdef, CBitReader *rb) {
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
		//if (m_cdef_y_sec_strength[i] == 3)
		//	m_cdef_y_sec_strength[i] += 1;

		if (NumPlanes > 1) {
			m_cdef_uv_pri_strength[i] = rb->AomRbReadLiteral(4);
			m_cdef_uv_sec_strength[i] = rb->AomRbReadLiteral(2);
			//if (m_cdef_uv_sec_strength[i] == 3)
			//	m_cdef_uv_sec_strength[i] += 1;
		}
	}
}

void CFrameHeader::FhParserLrParams(int NumPlanes, int enable_restoration, int use_128x128_superblock,
	int subsampling_x, int subsampling_y, CBitReader *rb) {

	if (m_AllLossless || m_allow_intrabc || !enable_restoration) {
		m_FrameRestorationType[0] = RESTORE_NONE;
		m_FrameRestorationType[1] = RESTORE_NONE;
		m_FrameRestorationType[2] = RESTORE_NONE;
		m_UsesLr = 0;
		return;
	}
	m_UsesLr = 0;
	m_usesChromaLr = 0;

	for (int i = 0; i < NumPlanes; i++) {
		m_lr_type[i] = rb->AomRbReadLiteral(2);
		m_FrameRestorationType[i] = Remap_Lr_Type[m_lr_type[i]];
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

void CFrameHeader::FhParserTxMode(CBitReader *rb) {
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

void CFrameHeader::FhParserFrameReferenceMode(int FrameIsIntra, CBitReader *rb) {
	if (FrameIsIntra) {
		m_reference_select = 0;
	}
	else {
		m_reference_select = rb->AomRbReadBit();
	}
}

void CFrameHeader::FhParserSkipModeParams(int FrameIsIntra, int enable_order_hint, CBitReader *rb) {
	if (FrameIsIntra || !m_reference_select || !enable_order_hint) {
		m_skipModeAllowed = 0;
	}
	else {
		m_forwardIdx = -1;
		m_backwardIdx = -1;

		//for (int i = 0; i < INTER_REFS_PER_FRAME; i++) {
		//	refHint = RefOrderHint[ref_frame_idx[i]]
		//		if (get_relative_dist(refHint, OrderHint) < 0) {
		//			if (forwardIdx < 0 ||
		//				get_relative_dist(refHint, forwardHint) > 0) {
		//				forwardIdx = i
		//					forwardHint = refHint
		//			}
		//		}
		//		else if (get_relative_dist(refHint, OrderHint) > 0) {
		//			if (backwardIdx < 0 ||
		//				get_relative_dist(refHint, backwardHint) < 0) {
		//				backwardIdx = i
		//					backwardHint = refHint
		//			}
		//		}
		//}
	}

	if (m_skipModeAllowed) {
		m_skip_mode_present = rb->AomRbReadBit();
	}
	else {
		m_skip_mode_present = 0;
	}
}

void  CFrameHeader::FhParserTemporalPointInfo(int frame_presentation_time_length_minus_1, CBitReader *rb) {
	int n = frame_presentation_time_length_minus_1 + 1;
	m_frame_presentation_time = rb->AomRbReadLiteral(n);
}

void CFrameHeader::FhParserGlobalMotionParams(int FrameIsIntra, CBitReader *rb) {
	for (int ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
		m_GmType[ref] = IDENTITY;
		for (int i = 0; i < 6; i++) {
			//gm_params[ref][i] = ((i % 3 == 2) ? 1 << WARPEDMODEL_PREC_BITS : 0);
		}
	}
	if (FrameIsIntra)
		return;

	for (int ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
		m_is_global = rb->AomRbReadBit();
		TransformationType type;
		if (m_is_global) {
			m_is_rot_zoom = rb->AomRbReadBit();
			if (m_is_rot_zoom) {
				type = ROTZOOM;
			}
			else {
				m_is_translation = rb->AomRbReadBit();
				type = m_is_translation ? TRANSLATION : AFFINE;
			}
		}
		else {
			type = IDENTITY;
		}
		m_GmType[ref] = type;

		if (type >= ROTZOOM) {
			//read_global_param(type, ref, 2);
			//read_global_param(type, ref, 3);
			if (type == AFFINE) {
				//read_global_param(type, ref, 4);
				//read_global_param(type, ref, 5)//
			}
			else {
				//gm_params[ref][4] = -gm_params[ref][3];
				//gm_params[ref][5] = gm_params[ref][2];
			}
		}
		if (type >= TRANSLATION) {
			//read_global_param(type, ref, 0);
			//read_global_param(type, ref, 1);
		}
	}

}

//write uncompressed header
uint32_t CFrameHeader::write_uncompressed_header_obu(uint8_t *const dst, int bit_buffer_offset) {
	CBitWriter wb(dst, bit_buffer_offset);
	uint32_t size = 0;



	return 0;
}


void  CFrameHeader::write_temporal_point_info(int frame_presentation_time_length_minus_1, CBitWriter *wb) {
	int n = frame_presentation_time_length_minus_1 + 1;
	wb->aom_wb_write_unsigned_literal(m_frame_presentation_time, n);
}

void CFrameHeader::write_frame_size(int num_bits_width, int num_bits_height, int enable_superres, CBitWriter *wb) {
	
	if (m_frame_size_override_flag) {
		wb->aom_wb_write_literal(m_frame_width_minus_1, num_bits_width + 1);
		wb->aom_wb_write_literal(m_frame_height_minus_1, num_bits_height + 1);
	}

	write_superres_scale(enable_superres, wb);
	write_render_size(wb);
}

void CFrameHeader::write_superres_scale(int enable_superres, CBitWriter *wb) {

	if (!enable_superres) {
		assert(m_SuperresDenom == SUPERRES_NUM);
		return;
	}

	// First bit is whether to to scale or not
	if (m_SuperresDenom == SUPERRES_NUM) {
		wb->aom_wb_write_bit(0);  // no scaling
	}
	else {
		wb->aom_wb_write_bit(1);  // scaling, write scale factor
		assert(m_SuperresDenom >= SUPERRES_DENOM_MIN);
		assert(m_SuperresDenom < SUPERRES_DENOM_MIN + (1 << SUPERRES_DENOM_BITS));
		wb->aom_wb_write_literal(m_SuperresDenom - SUPERRES_DENOM_MIN, SUPERRES_DENOM_BITS);
	}
}

void CFrameHeader::write_render_size(CBitWriter *wb) {

	// render_and_frame_size_different is always 0 
	assert(m_render_and_frame_size_different == 0);
	wb->aom_wb_write_bit(0);
	//if (scaling_active) {
	//	aom_wb_write_literal(wb, cm->render_width - 1, 16);
	//	aom_wb_write_literal(wb, cm->render_height - 1, 16);
	//}
}


void CFrameHeader::write_frame_size_with_refs(CBitWriter *wb) {
	//unimplmented
	//AV1_COMMON *const cm = &cpi->common;
	//int found = 0;

	//MV_REFERENCE_FRAME ref_frame;
	//for (ref_frame = LAST_FRAME; ref_frame <= ALTREF_FRAME; ++ref_frame) {
	//	YV12_BUFFER_CONFIG *cfg = get_ref_frame_buffer(cpi, ref_frame);

	//	if (cfg != NULL) {
	//		found = cm->superres_upscaled_width == cfg->y_crop_width &&
	//			cm->superres_upscaled_height == cfg->y_crop_height;
	//		found &= cm->render_width == cfg->render_width &&
	//			cm->render_height == cfg->render_height;
	//	}
	//	aom_wb_write_bit(wb, found);
	//	if (found) {
	//		write_superres_scale(cm, wb);
	//		break;
	//	}
	//}

	//if (!found) {
	//	int frame_size_override = 1;  // Always equal to 1 in this function
	//	write_frame_size(cm, frame_size_override, wb);
	//}
}

void CFrameHeader::write_frame_interp_filter(CBitWriter *wb) {
	wb->aom_wb_write_bit(m_is_filter_switchable);
	if (!m_is_filter_switchable) {
		wb->aom_wb_write_literal(m_interpolation_filter, 2);
	}
}
//void CFrameHeader::write_tile_info(struct aom_write_bit_buffer *saved_wb,
//	struct aom_write_bit_buffer *wb) {
//	write_tile_info_max_tile(cm, wb);
//
//	*saved_wb = *wb;
//	if (cm->tile_rows * cm->tile_cols > 1) {
//		// tile id used for cdf update
//		aom_wb_write_literal(wb, 0, cm->log2_tile_cols + cm->log2_tile_rows);
//		// Number of bytes in tile size - 1
//		aom_wb_write_literal(wb, 3, 2);
//	}
//}

void CFrameHeader::write_delta_q(int delta_q, CBitWriter *wb) {
	
	if (delta_q != 0) {
		wb->aom_wb_write_bit(1);
		wb->aom_wb_write_inv_signed_literal(delta_q, 6);
	}
	else {
		wb->aom_wb_write_bit(0);
	}
}


void CFrameHeader::encode_quantization(int NumPlanes, int separate_uv_delta_q, CBitWriter *wb) {

	wb->aom_wb_write_literal(m_base_q_idx, QINDEX_BITS);
	write_delta_q(m_DeltaQYDc, wb);
	if (NumPlanes > 1) {
		if (separate_uv_delta_q) 
			wb->aom_wb_write_bit(m_diff_uv_delta);
		
		write_delta_q(m_DeltaQUDc, wb);
		write_delta_q(m_DeltaQUAc, wb);
		if (m_diff_uv_delta) {
			write_delta_q(m_DeltaQVDc, wb);
			write_delta_q(m_DeltaQVAc, wb);
		}
	}
	wb->aom_wb_write_bit(m_using_qmatrix);

	if (m_using_qmatrix) {
		wb->aom_wb_write_literal(m_qm_y, QM_LEVEL_BITS);
		wb->aom_wb_write_literal(m_qm_u, QM_LEVEL_BITS);
		if (!separate_uv_delta_q)
			assert(m_qm_u == m_qm_v);
		else
			wb->aom_wb_write_literal(m_qm_v, QM_LEVEL_BITS);
	}
}


void CFrameHeader::encode_segmentation(CBitWriter *wb) {

	wb->aom_wb_write_bit(m_segmentation_enabled);
	assert(m_segmentation_enabled == 0);
	printf("Segmentation is not allowed!!\n");
	if (!m_segmentation_enabled)
		return;
}

void CFrameHeader::encode_delta_q_and_lf_params(CBitWriter *wb) {

	if (m_delta_q_present)
		assert(m_base_q_idx > 0);
	if (m_base_q_idx > 0) {
		wb->aom_wb_write_bit(m_delta_q_present);
		if (m_delta_q_present) {
			//wb->aom_wb_write_literal(get_msb(m_delta_q_res), 2);  //?
			wb->aom_wb_write_literal(m_delta_q_res, 2);
			//xd->current_qindex = cm->base_qindex;
			if (m_allow_intrabc)
				assert(m_delta_lf_present == 0);
			else
				wb->aom_wb_write_bit(m_delta_lf_present);
			if (m_delta_lf_present) {
				//wb->aom_wb_write_literal(get_msb(m_delta_lf_res), 2);?
				wb->aom_wb_write_literal(m_delta_lf_res, 2);
				wb->aom_wb_write_bit(m_delta_lf_multi);
				//av1_reset_loop_filter_delta(xd, av1_num_planes(cm));
			}
		}
	}
}


void CFrameHeader::encode_loopfilter(int NumPlanes, CBitWriter *wb) {
	assert(!m_CodedLossless);
	if (m_allow_intrabc) 
		return;

	// Encode the loop filter level and type
	wb->aom_wb_write_literal(m_loop_filter_level[0], 6);
	wb->aom_wb_write_literal(m_loop_filter_level[1], 6);
	if (NumPlanes > 1) {
		if (m_loop_filter_level[0] || m_loop_filter_level[1]) {
			wb->aom_wb_write_literal(m_loop_filter_level[2], 6);
			wb->aom_wb_write_literal(m_loop_filter_level[3], 6);
		}
	}
	wb->aom_wb_write_literal(m_loop_filter_sharpness, 3);

	// Write out loop filter deltas applied at the MB level based on mode or
	// ref frame (if they are enabled).
	wb->aom_wb_write_bit(m_loop_filter_delta_enabled);

	if (m_loop_filter_delta_enabled) {
		wb->aom_wb_write_bit(m_loop_filter_delta_update);

		if (m_loop_filter_delta_update) {
			int i;
			for (i = 0; i < REF_FRAMES; i++) {
				wb->aom_wb_write_bit(m_update_ref_delta[i]);
				if (m_update_ref_delta[i])
					wb->aom_wb_write_inv_signed_literal(m_loop_filter_ref_deltas[i], 6);
			}

			for (i = 0; i < 2; i++) {
				wb->aom_wb_write_bit(m_update_mode_delta[i]);
				if (m_update_mode_delta[i]) 
					wb->aom_wb_write_inv_signed_literal(m_loop_filter_mode_deltas[i], 6);
			}
		}
	}
}


void CFrameHeader::encode_cdef(int NumPlanes, int enable_cdef, CBitWriter *wb) {
	assert(!m_CodedLossless);
	if (enable_cdef) 
		return;
	if (m_allow_intrabc) 
		return;

	wb->aom_wb_write_literal(m_cdef_damping_minus_3, 2);
	//assert(cm->cdef_pri_damping == cm->cdef_sec_damping);
	wb->aom_wb_write_literal(m_cdef_bits, 2);

	for (int i = 0; i < (1 << m_cdef_bits); i++) {
		wb->aom_wb_write_literal(m_cdef_y_pri_strength[i], 4);
		wb->aom_wb_write_literal(m_cdef_y_sec_strength[i], 2);
		
		if (NumPlanes > 1) {
			wb->aom_wb_write_literal(m_cdef_uv_pri_strength[i], 4);
			wb->aom_wb_write_literal(m_cdef_uv_sec_strength[i], 2);
		}
	}
}


void CFrameHeader::encode_restoration_mode(int NumPlanes, int enable_restoration, int use_128x128_superblock,
	int subsampling_x, int subsampling_y, CBitWriter *wb) {
	
	if (enable_restoration) 
		return;
	if (m_allow_intrabc) 
		return;

	int all_none = 1, chroma_none = 1;
	for (int p = 0; p < NumPlanes; ++p) {
		wb->aom_wb_write_literal(m_lr_type[p], 2);
	}
	if (m_UsesLr) {
		const int sb_size =  use_128x128_superblock ? 128 : 64;
		assert(m_LoopRestorationSize[0] >= sb_size);
	
		if (sb_size == 64) {
			wb->aom_wb_write_bit(m_LoopRestorationSize[0] > 64);
		}
		if (m_LoopRestorationSize[0] > 64) {
			wb->aom_wb_write_bit(m_LoopRestorationSize[0] > 128);
		}
	}

	if (NumPlanes > 1) {
		int s = AOMMIN(subsampling_x, subsampling_y);
		if (s && m_usesChromaLr) {
			wb->aom_wb_write_bit(m_LoopRestorationSize[1] != m_LoopRestorationSize[0]);

			assert(m_LoopRestorationSize[1] == m_LoopRestorationSize[0] ||
				m_LoopRestorationSize[1] == (m_LoopRestorationSize[0] >> s));
			assert(m_LoopRestorationSize[2] == m_LoopRestorationSize[1]);
		}
	}
}


 void CFrameHeader::write_tx_mode(CBitWriter *wb) {
	if (m_CodedLossless) {
		m_TxMode = ONLY_4X4;
		return;
	}
	wb->aom_wb_write_bit(m_TxMode == TX_MODE_SELECT);
}