//#include "av1/decoder/obu.h"

#include "BitReader.h"
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

typedef struct aom_dec_model_op_parameters {
	int decoder_model_param_present_flag;
	int64_t bitrate;
	int64_t buffer_size;
	uint32_t decoder_buffer_delay;
	uint32_t encoder_buffer_delay;
	int low_delay_mode_flag;
	int display_model_param_present_flag;
	int initial_display_delay;
} aom_dec_model_op_parameters_t;


class CSequenceHeader
{
public:
	CSequenceHeader();
	virtual ~CSequenceHeader();


	BITSTREAM_PROFILE Av1ReadProfile(struct AomReadBitBuffer *rb); 
	

private:
	BITSTREAM_PROFILE m_SeqProfile;
	int m_StillPicture;
	int m_ReducedStillPictureHeader;
	int m_TimingInfoPresentFlag;
	int m_DecoderModelInfoPresentFlag;
	int m_InitialDisplayDelayPresentFlag;

	int m_OperatingPointsCntMinus1;
	int m_OperatingPointIdc[MAX_NUM_OPERATING_POINTS];
	int m_SeqLevelIdx[MAX_NUM_OPERATING_POINTS];
	int m_SeqTier[MAX_NUM_OPERATING_POINTS];
	int m_DecoderModelPresentForThisOp[MAX_NUM_OPERATING_POINTS];
	//decoder_model_info()
	aom_dec_model_op_parameters_t m_OpParams[MAX_NUM_OPERATING_POINTS + 1];
	int m_InitialDisplayDelayPresentForThisOp[MAX_NUM_OPERATING_POINTS];
	int m_InitialDisplayDelayMinus1[MAX_NUM_OPERATING_POINTS];

	//choose_operating_point()
	int m_FrameWidthBitsMinus1;
	int m_FrameHeightBitsMinus1;
	int m_MaxFrameWidthMinus1;
	int m_MaxFrameHeightMinus1;
	int m_FrameIdNumbersPresentFlag;
	int m_DeltaFrameIdLengthMinus2;
	int m_AdditionalFrameIdLengthMinus1;
	int m_Use128x128Superblock;  // Size of the superblock used for this frame
	int m_EnableFilterIntra;
	int m_EnableIntraEdgeFilter;

	int m_EnableInterintraCompound;
	int m_EnableMaskedCompound;
	int m_EnableWarpedMotion;
	int m_EnableDualFilter;
	int m_EnableOrderHint;
	int m_EnableJntComp;
	int m_EnableRefFrameMvs;
	int m_SeqChooseScreenContentTools;
	int m_SeqForceScreenContentTools;
	int m_SeqChooseIntegerMv;
	int m_SeqForceIntegerMv;
	int m_OrderHintBitsMinus1;

	int m_EnableSuperres;
	int m_EnableCdef;
	int m_EnableRestoration;

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
