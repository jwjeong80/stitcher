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




	int FhReadShowExistingFrame() { return m_show_existing_frame; }
	int FhReadShowFrame() { return m_show_frame; }
	FRAME_TYPE FhReadFrameType() { return m_frame_type; }
	int FhReadAllowScreenContentTools() { return m_allow_screen_content_tools; }
	int FhReadOrderHint() { return m_order_hint; }
	int FhReadErrorResilientMode() { return m_error_resilient_mode; }
	int FhReadRefreshFrameFlags() { return m_refresh_frame_flags; }

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
	int m_FrameWidthMinus1;
	int m_FrameHeightMinus1;

	//superres_params()
	int m_UseSuperres;
	int m_CodedDenom;

	//render_size( )
	int m_RenderAndFrameSizeDifferent;
	int m_RenderWidthMinus1;
	int m_RenderHeightMinus1;


	int m_FrameRefsShortSignaling;
	int m_LastFrameIdx;
	int m_GoldFrameIdx;
	int m_RefFrameIdx[INTER_REFS_PER_FRAME];
	int m_DeltaFrameIdMinus1;

	//frame_size_with_refs( )
	int m_FoundRef;


	//read_interpolation_filter( )
	int m_IsFilterSwitchable;
	int m_InterpolationFilter;

	int m_IsMotionModeSwitchable;


	int m_DisableFrameEndUpdateCdf;

	//tile_info ( )
	int m_UniformTileSpacingFlag;
	int m_IncrementTileColsLog2;
	int m_IncrementTileRowsLog2;
	int m_WidthInSbsMinus1;
	int m_HeightInSbsMinus1;
	int m_ContextUpdateTileId;
	int m_TileSizeBytesMinus1;

	//quantization_params()
	int m_BaseQIdx;
	int m_DiffUvDelta;
	int m_UsingQmatrix;
	int m_QmY;
	int m_QmV;
	int m_QmU;

	//read_delta_q()
	int m_DeltaCoded;
	int m_DeltaQ;

	//segmentation_params()
	int m_SegmentationEnabled;
	int m_SegmentationUpdateMap;
	int m_SegmentationTemporalUpdate;
	int m_SegmentationUpdateData;
	int m_FeatureEnabled;
	int m_FeatureValue;

	//delta_q_params()
	int m_DeltaQPresent;
	int m_DeltaQRes;

	//delta_lf_params()
	int m_DeltaLfPresent;
	int DeltaLfRes;
	int DeltaLfMulti;

	//loop_filter_params()
	int m_LoopFilterLevel[4];
	int m_LoopFilterSharpness;
	int m_LoopFilterDeltaEnabled;
	int m_LoopFilterDeltaUpdate;
	int m_UpdateRefDelta;
	int m_LoopFilterRefDeltas;
	int m_UpdateModeDelta;
	int m_LoopFilterModeDeltas;

	int m_RefValid[NUM_REF_FRAMES];
	int	m_RefOrderHint[NUM_REF_FRAMES];
	int m_RefFrameId[NUM_REF_FRAMES];

	int m_OrderHints[INTER_REFS_PER_FRAME];


	int m_PrevFrameID;
};