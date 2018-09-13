#pragma once


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

class CFrameHeader
{
private:
	int m_ShowExistingFrame;
	int m_FrameToShowMapIdx;
	int m_DisplayFrameId;
	int m_FrameType;
	int m_ShowFrame;
	int m_ShowableFrame;
	int m_ErrorResilientMode;

	int m_DisableCdfUpdate;
	int m_AllowScreenContentTools;
	int m_ForceIntegerMv;
	int m_CurrentFrameId;
	int m_FrameSizeOverrideFlag;
	int m_OrderHint;
	int m_PrimaryRefFrame;

	int m_BufferRemovalTimePresentFlag;
	int m_BufferRemovalTime[MAX_NUM_OPERATING_POINTS];
	int m_RefreshFrameFlags;
	int m_RefOrderHint[INTER_REFS_PER_FRAME];

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

	int m_AllowIntrabc;

	int m_FrameRefsShortSignaling;
	int m_LastFrameIdx;
	int m_GoldFrameIdx;
	int m_RefFrameIdx[INTER_REFS_PER_FRAME];
	int m_DeltaFrameIdMinus1;

	//frame_size_with_refs( )
	int m_FoundRef;

	int m_AllowHighPrecisionMv;

	//read_interpolation_filter( )
	int m_IsFilterSwitchable;
	int m_InterpolationFilter;

	int m_IsMotionModeSwitchable;
	int m_UseRefFrameMvs;

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


};