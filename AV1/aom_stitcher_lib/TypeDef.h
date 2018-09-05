/** \file     TypeDef.h
    \brief    Define basic types, new types and enumerations
*/

#ifndef _TYPEDEF__
#define _TYPEDEF__

#include <stdint.h>
#include <vector>

//! \ingroup TLibCommon
//! \{

// yonghwan
#define		TEMP_DISABLE	0	// 개발 및 디버깅을 위해 임시로 disable시키는 코드 (실제 코드에서는 1이 되어야 함)

#define		FAST_DEC		1	// Fast decoding mode is not compliant to standard

#define		OPT_SH_WRITE	1	// Slice header writing optimization

///////////////////////////////////////////////////////////////////////////////

// Multi-layer extension macros
#define		MAX_LAYERS							(8)      ///< max number of layers the codec is supposed to handle
#define		MAX_NUM_LAYER_IDS					(63)
#define		MAX_VPS_OP_LAYER_SETS_PLUS1			(MAX_LAYERS+1)
#define		MAX_VPS_LAYER_SETS_PLUS1			1024
#define		MAX_VPS_OUTPUT_LAYER_SETS_PLUS1		1024
#define		MAX_VPS_LAYER_IDX_PLUS1				MAX_LAYERS
#define		MAX_NUM_ADD_LAYER_SETS				1023
#define		MAX_VPS_NUM_SCALABILITY_TYPES		16
#define		MAX_REF_LAYERS						7

/// scalability types
enum ScalabilityType
{
	VIEW_ORDER_INDEX = 1,	// MV-HEVC
	SCALABILITY_ID = 2,		// SHVC
	AUX_ID = 3,
};


// Tile constants
#define		MAX_NUM_TILE_COLS	(20)
#define		MAX_NUM_TILE_ROWS	(22)
#define		MAX_NUM_TILES	(MAX_NUM_TILE_COLS * MAX_NUM_TILE_ROWS)

/// supported prediction type
enum PredMode
{
	MODE_INTER,           ///< inter-prediction mode
	MODE_INTRA,           ///< intra-prediction mode
	MODE_NONE = 2
};

// Various CU Flags (16-bit bit-wise)
#define		CU_FLAG_MODE		0x00000003	// 2-bit: (0: MODE_INTER, 1: MODE_INTRA, 2: MODE_NONE)	
#define		CU_FLAG_TQ_BYPASS	0x00000004
#define		CU_FLAG_SKIP		0x00000008
#define		CU_FLAG_PCM			0x00000010
#define		CU_FLAG_BSLICE		0x00000020	// B-slice

#define		CU_TYPE_INTRA_SPLIT	0x01000000	// IntraSplitFlag (NxN INTRA) (휘발성 값 => 즉, 단일 CU 내에서만 사용하는 값)
#define		CU_FLAG_SDH_VALID	0x02000000	// Sign Data Hiding valid (휘발성 값)

#define		IS_CU_TQ_BYPASS(x)		((x) & CU_FLAG_TQ_BYPASS)
#define		IS_CU_SKIP(x)			((x) & CU_FLAG_SKIP)
#define		IS_CU_INTRA(x)			(((x) & CU_FLAG_MODE) == MODE_INTRA)
#define		IS_CU_INTER(x)			(((x) & CU_FLAG_MODE) == MODE_INTER)
#define		IS_CU_PCM(x)			((x) & CU_FLAG_PCM)
#define		IS_CU_BSLICE(x)			((x) & CU_FLAG_BSLICE)
#define		IS_CU_SKIP_PCM_TQB(x)	((x) & (CU_FLAG_TQ_BYPASS | CU_FLAG_SKIP | CU_FLAG_PCM))
#define		IS_CU_PCM_OR_BYPASS(x)	((x) & (CU_FLAG_PCM | CU_FLAG_TQ_BYPASS))

#define		SET_CU_INVALID(x)		((x) = (MODE_NONE))
#define		IS_CU_AVAIL(x)			(((x) & CU_FLAG_MODE) != MODE_NONE)

#define		IS_CU_INTRA_SPLIT(x)	((x) & CU_TYPE_INTRA_SPLIT)
#define		IS_CU_SDH_VALID(x)		((x) & CU_FLAG_SDH_VALID)

#define		PB_CACHE_WIDTH			(20)	// 4/*left*/ + 16 [[4x4 block 단위]]
#define		CB_CACHE_WIDTH			(12)	// 4/*left*/ + 8  [[8x8 block 단위]]

#define     MAX_VPS_NUM_HRD_PARAMETERS                  1
#define		MAX_VPS_OP_SETS_PLUS1						1024
#define		MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1	1

#define		MAX_CPB_CNT				32  ///< Upper bound of (cpb_cnt_minus1 + 1)

#define		COEF_REMAIN_BIN_REDUCTION	3 ///< indicates the level at which the VLC 
                                           ///< transitions from Golomb-Rice to TU+EG(k)
#define		CU_DQP_TU_CMAX			5                   ///< max number bins for truncated unary
#define		CU_DQP_EG_k				0                      ///< expgolomb order

#define		SBH_THRESHOLD			4  ///< I0156: value of the fixed SBH controlling threshold
  
#define		C1FLAG_NUMBER			8 // maximum number of largerThan1 flag coded in one chunk :  16 in HM5
#define		C2FLAG_NUMBER			1 // maximum number of largerThan2 flag coded in one chunk:  16 in HM5 

#define		MAX_NUM_VPS				16
#define		MAX_NUM_SPS				16
#define		MAX_NUM_PPS				64

#define		MLS_GRP_NUM				64     ///< G644 : Max number of coefficient groups, max(16, 64)
#define		MLS_CG_SIZE				4      ///< G644 : Coefficient group size of 4x4

#define		SCAN_SET_SIZE			16
#define		LOG2_SCAN_SET_SIZE		4


#define		PRINT_RPS_INFO			0           ///< Enable/disable the printing of bits used to send the RPS.
                                                    // using one nearest frame as reference frame, and the other frames are high quality (POC%4==0) frames (1+X)
                                                    // this should be done with encoder only decision
                                                    // but because of the absence of reference frame management, the related code was hard coded currently

#define		PLANAR_IDX				0
#define		DC_IDX					1                     // index for intra DC mode
#define		HOR_IDX					10                    // index for intra HORIZONTAL mode
#define		VER_IDX					26                    // index for intra VERTICAL   mode
//#define		DM_CHROMA_IDX			36                    // chroma mode index for derived from luma intra mode
//#define		NUM_INTRA_MODE			36
//#define		NUM_CHROMA_MODE			5                     // total number of chroma modes

#define     NUMBER_OF_PREDICTION_MODES  2

// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================

typedef       void                Void;
typedef       bool                Bool;
typedef       char                TChar; // Used for text/characters
typedef       short               Short;
typedef       unsigned short      UShort;
typedef       int                 Int;
typedef       unsigned int        UInt;
typedef       double              Double;
typedef       float               Float;

// ====================================================================================================================
// 64-bit integer type
// ====================================================================================================================

#ifdef _MSC_VER
typedef       __int64             Int64;
typedef       unsigned __int64    UInt64;

#else	// _MSC_VER

typedef       long long           Int64;
typedef       unsigned long long  UInt64;

#endif	// _MSC_VER

// ====================================================================================================================
// Type definition
// ====================================================================================================================

enum	CtbNeighbour
{
	CTB_LEFT	=	0,
	CTB_UP,
	CTB_UP_RIGHT,
	CTB_UP_LEFT,
};


// ====================================================================================================================
// Enumeration
// ====================================================================================================================

/// supported slice type
enum SliceType
{
	B_SLICE,
	P_SLICE,
	I_SLICE
};

/// chroma formats (according to semantics of chroma_format_idc)
enum ChromaFormat
{
	CHROMA_400  = 0,
	CHROMA_420  = 1,
	CHROMA_422  = 2,
	CHROMA_444  = 3,
	NUM_CHROMA_FORMAT = 4
};

/// supported partition shape
enum PartSize
{
	SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
	SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
	SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
	SIZE_NxN,             ///< symmetric motion partition,   Nx N
	SIZE_2NxnU,           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
	SIZE_2NxnD,           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
	SIZE_nLx2N,           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
	SIZE_nRx2N,           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N
	SIZE_NONE = 15
};

/// texture component type
enum TextType
{
	TEXT_LUMA,            ///< luma
	TEXT_CHROMA,          ///< chroma (U+V)
	TEXT_CHROMA_U,        ///< chroma U
	TEXT_CHROMA_V,        ///< chroma V
	TEXT_ALL,             ///< Y+U+V
	TEXT_NONE = 15
};

// Inter-prediction direction
enum InterPredDir	
{
	PRED_INTRA	= 0,
	PRED_L0		= 1,
	PRED_L1		= 2,
	PRED_BI		= 3,
};

/// reference list index
enum RefPicList
{
	REF_PIC_LIST_0 = 0,   ///< reference list 0
	REF_PIC_LIST_1 = 1,   ///< reference list 1
	REF_PIC_LIST_X = 100  ///< special mark
};

/// motion vector predictor direction used in AMVP
enum MVP_DIR
{
	MD_LEFT = 0,          ///< MVP of left block
	MD_ABOVE,             ///< MVP of above block
	MD_ABOVE_RIGHT,       ///< MVP of above right block
	MD_BELOW_LEFT,        ///< MVP of below left block
	MD_ABOVE_LEFT         ///< MVP of above left block
};

/// coefficient scanning type used in ACS
enum COEFF_SCAN_TYPE
{
	SCAN_DIAG = 0,         ///< up-right diagonal scan
	SCAN_HOR,              ///< horizontal first scan
	SCAN_VER               ///< vertical first scan
};

namespace Profile
{
	enum Name
	{
		NONE = 0,
		MAIN = 1,
		MAIN10 = 2,
		MAINSTILLPICTURE = 3,
		MAINREXT = 4,
		HIGHTHROUGHPUTREXT = 5,
		MULTIVIEWMAIN = 6,
		SCALABLEMAIN = 7,
		SCALABLEMAIN10 = 8,
	};
}

namespace Level
{
	enum Tier
	{
		MAIN = 0,
		HIGH = 1,
	};

	enum Name
	{
		NONE     = 0,
		LEVEL1   = 30,
		LEVEL2   = 60,
		LEVEL2_1 = 63,
		LEVEL3   = 90,
		LEVEL3_1 = 93,
		LEVEL4   = 120,
		LEVEL4_1 = 123,
		LEVEL5   = 150,
		LEVEL5_1 = 153,
		LEVEL5_2 = 156,
		LEVEL6   = 180,
		LEVEL6_1 = 183,
		LEVEL6_2 = 186,
		LEVEL8_5 = 255,
	};
}

enum ComponentID
{
    COMPONENT_Y = 0,
    COMPONENT_Cb = 1,
    COMPONENT_Cr = 2,
    MAX_NUM_COMPONENT = 3
};

enum SPSExtensionFlagIndex
{
    SPS_EXT__REXT = 0,
    //SPS_EXT__MVHEVC         = 1, //for use in future versions
    //SPS_EXT__SHVC           = 2, //for use in future versions
    NUM_SPS_EXTENSION_FLAGS = 8
};

enum PPSExtensionFlagIndex
{
    PPS_EXT__REXT = 0,
    //PPS_EXT__MVHEVC         = 1, //for use in future versions
    //PPS_EXT__SHVC           = 2, //for use in future versions
    NUM_PPS_EXTENSION_FLAGS = 8
};

struct TComPictureHash
{
    std::vector<uint8_t> hash;

    Bool operator==(const TComPictureHash &other) const
    {
        if (other.hash.size() != hash.size())
        {
            return false;
        }
        for (UInt i = 0; i < UInt(hash.size()); i++)
        {
            if (other.hash[i] != hash[i])
            {
                return false;
            }
        }
        return true;
    }

    Bool operator!=(const TComPictureHash &other) const
    {
        return !(*this == other);
    }
};

struct TComSEITimeSet
{
    TComSEITimeSet() : clockTimeStampFlag(false),
        numUnitFieldBasedFlag(false),
        countingType(0),
        fullTimeStampFlag(false),
        discontinuityFlag(false),
        cntDroppedFlag(false),
        numberOfFrames(0),
        secondsValue(0),
        minutesValue(0),
        hoursValue(0),
        secondsFlag(false),
        minutesFlag(false),
        hoursFlag(false),
        timeOffsetLength(0),
        timeOffsetValue(0)
    { }
    Bool clockTimeStampFlag;
    Bool numUnitFieldBasedFlag;
    Int  countingType;
    Bool fullTimeStampFlag;
    Bool discontinuityFlag;
    Bool cntDroppedFlag;
    Int  numberOfFrames;
    Int  secondsValue;
    Int  minutesValue;
    Int  hoursValue;
    Bool secondsFlag;
    Bool minutesFlag;
    Bool hoursFlag;
    Int  timeOffsetLength;
    Int  timeOffsetValue;
};

struct TComSEIMasteringDisplay
{
    Bool      colourVolumeSEIEnabled;
    UInt      maxLuminance;
    UInt      minLuminance;
    UShort    primaries[3][2];
    UShort    whitePoint[2];
};
//! \}

#endif
