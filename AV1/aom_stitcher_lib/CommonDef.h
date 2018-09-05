/** \file     CommonDef.h
    \brief    Defines constants, macros and tool parameters
*/

#ifndef __COMMONDEF__
#define __COMMONDEF__

#include <algorithm>

#if _MSC_VER > 1000
// disable "signed and unsigned mismatch"
#pragma warning( disable : 4018 )
// disable bool coercion "performance warning"
#pragma warning( disable : 4800 )
#endif // _MSC_VER > 1000
#include "TypeDef.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Version information
// ====================================================================================================================

#define NV_VERSION        "12.0"                 ///< Current software version

// ====================================================================================================================
// Platform information
// ====================================================================================================================

#ifdef __GNUC__
#define NVM_COMPILEDBY  "[GCC %d.%d.%d]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#ifdef __IA64__
#define NVM_ONARCH    "[on 64-bit] "
#else
#define NVM_ONARCH    "[on 32-bit] "
#endif
#endif

#ifdef __INTEL_COMPILER
#define NVM_COMPILEDBY  "[ICC %d]", __INTEL_COMPILER
#elif  _MSC_VER
#define NVM_COMPILEDBY  "[VS %d]", _MSC_VER
#endif

#ifndef NVM_COMPILEDBY
#define NVM_COMPILEDBY "[Unk-CXX]"
#endif

#ifdef _WIN32
#define NVM_ONOS        "[Windows]"
#elif  __linux__
#define NVM_ONOS        "[Linux]"
#elif  __CYGWIN__
#define NVM_ONOS        "[Cygwin]"
#elif __APPLE__
#define NVM_ONOS        "[Mac OS X]"
#else
#define NVM_ONOS		"[Unk-OS]"
#endif

#define NVM_BITS          "[%d bit] ", (sizeof(void*) == 8 ? 64 : 32) ///< used for checking 64-bit O/S

#if defined(__LP64__) // LP64 machine, OS X or Linux
  #define ARCH_X64		1
#elif defined(_WIN64) // LLP64 machine, Windows
  #define ARCH_X64		1
#else	// 32-bit machine (Windows, Linux, or OS X)
  #define ARCH_X64		0
#endif


#ifndef NULL
#define NULL              0
#endif

// ====================================================================================================================
// Common constants
// ====================================================================================================================

#define MAX_GOP                     64          ///< max. value of hierarchical GOP size

#define MAX_NUM_REF_PICS            16          ///< max. number of pictures used for reference
#define MAX_NUM_REF                 16          ///< max. number of entries in picture reference list

#define MAX_UINT                    0xFFFFFFFFU ///< max. value of unsigned 32-bit integer
//#define MAX_INT                     2147483647  ///< max. value of signed 32-bit integer

#define MIN_QP                      0
#define MAX_QP                      51

#define NOT_VALID                   -1

// ====================================================================================================================
// Macro functions
// ====================================================================================================================


// Clip a signed integer value into the [0, 255] range
inline uint8_t	clip_uint8(int a)
{
	if (a & (~0xFF))	return (-a)>>31;
	else				return a; 
}

// Clip a signed integer value into the [-128, 127] range
inline int8_t	clip_int8(int a)
{
	if ((a + 0x80) & ~0xFF)		return (a>>31) ^ 0x7F;
	else						return a; 
}

// Clip a signed integer value into the [0, 65535] range
inline uint16_t	clip_uint16(int a)
{
	if (a & (~0xFFFF))	return (-a)>>31;
	else				return a; 
}

// Clip a signed integer value into the [-32768, 32767] range
inline int16_t	clip_int16(int a)
{
	if ((a + 0x8000) & ~0xFFFF)		return (a>>31) ^ 0x7FFF;
	else							return a; 
}

// Clip a signed integer value into the unsigned [0, ((2^b)-1)] range
inline uint32_t	clip_uintN(int a, int b/*bit depth*/)
{
	if (a & ~((1<<b) - 1)) return -a >> 31 & ((1<<b) - 1);
	else                   return  a;
}


#define		HEVC_MIN(x, y)	((x) > (y) ? (y) : (x))
#define		HEVC_MAX(x, y)	((x) > (y) ? (x) : (y))
#define		HEVC_ABS(x)		((x) >= 0 ? (x) : (-(x)))
#define		HEVC_SWAP(type, x, y)	{	\
	type tmp	=	y;					\
	y			=	x;					\
	x			=	tmp;				\
}

/** clip a, such that minVal <= a <= maxVal */
template <typename T> inline T Clip3( T minVal, T maxVal, T a) 
{
	if( a < minVal)			return minVal;
	else if (a > maxVal)	return maxVal;
	else					return a;
}

inline uint32_t ClipY(int x, int bitDepthY) 
{ 
	return clip_uintN(x, bitDepthY); 
}
inline uint32_t ClipZeroMax(int a, int maxVal)		// unsigned max value only
{ 
	if (a & (~maxVal))	return -a >> 31 & maxVal;
	else				return a; 
}

// yonghwan (2012.07.20)
#define SAFE_DELETES(x)	if(x)	{	delete (x);	x = NULL;		}	// 단수
#define	SAFE_DELETE(x)	if(x)	{	delete [] (x);	x = NULL;	}	// 복수

#ifdef _MSC_VER 
#define		DECL_ALIGNED(n, type, var)		__declspec(align(n)) type var
#define		DECL_ASM_CONST(n, type, var)	__declspec(align(n)) static const type var
#elif defined(__GNUC__) // gcc
#define		DECL_ALIGNED(n, type, var)		type __attribute__ ((aligned(n))) var
#define		DECL_ASM_CONST(n, type, var)	static const type __attribute__((used)) __attribute__((aligned(n))) var
#else
#define		DECL_ALIGNED(n, type, var)		type var
#define		DECL_ASM_CONST(n, type, var)	static const type var
#endif

#pragma intrinsic(memcpy, memset, abs)


// ====================================================================================================================
// Coding tool configuration
// ====================================================================================================================

// AMVP: advanced motion vector prediction
#define AMVP_MAX_NUM_CANDS          2           ///< max number of final candidates
#define AMVP_MAX_NUM_CANDS_MEM      3           ///< max number of candidates
// MERGE
#define MRG_MAX_NUM_CANDS           5

// Explicit temporal layer QP offset
#define MAX_TLAYER                  7           ///< max number of temporal layer

#define MAX_CHROMA_FORMAT_IDC       3

#define MAX_TIMECODE_SEI_SETS       3            ///< Maximum number of time sets

// TODO: Existing names used for the different NAL unit types can be altered to better reflect the names in the spec.
//       However, the names in the spec are not yet stable at this point. Once the names are stable, a cleanup 
//       effort can be done without use of macros to alter the names used to indicate the different NAL unit types.
enum NalUnitType
{
	NAL_UNIT_CODED_SLICE_TRAIL_N = 0, // 0
	NAL_UNIT_CODED_SLICE_TRAIL_R,     // 1
  
	NAL_UNIT_CODED_SLICE_TSA_N,       // 2
	NAL_UNIT_CODED_SLICE_TSA_R,       // 3
  
	NAL_UNIT_CODED_SLICE_STSA_N,      // 4
	NAL_UNIT_CODED_SLICE_STSA_R,      // 5

	NAL_UNIT_CODED_SLICE_RADL_N,      // 6
	NAL_UNIT_CODED_SLICE_RADL_R,      // 7
  
	NAL_UNIT_CODED_SLICE_RASL_N,      // 8
	NAL_UNIT_CODED_SLICE_RASL_R,      // 9

	NAL_UNIT_RESERVED_VCL_N10,
	NAL_UNIT_RESERVED_VCL_R11,
	NAL_UNIT_RESERVED_VCL_N12,
	NAL_UNIT_RESERVED_VCL_R13,
	NAL_UNIT_RESERVED_VCL_N14,
	NAL_UNIT_RESERVED_VCL_R15,

	NAL_UNIT_CODED_SLICE_BLA_W_LP,    // 16
	NAL_UNIT_CODED_SLICE_BLA_W_RADL,  // 17
	NAL_UNIT_CODED_SLICE_BLA_N_LP,    // 18
	NAL_UNIT_CODED_SLICE_IDR_W_RADL,  // 19
	NAL_UNIT_CODED_SLICE_IDR_N_LP,    // 20
	NAL_UNIT_CODED_SLICE_CRA,         // 21
	NAL_UNIT_RESERVED_IRAP_VCL22,
	NAL_UNIT_RESERVED_IRAP_VCL23,

	NAL_UNIT_RESERVED_VCL24,
	NAL_UNIT_RESERVED_VCL25,
	NAL_UNIT_RESERVED_VCL26,
	NAL_UNIT_RESERVED_VCL27,
	NAL_UNIT_RESERVED_VCL28,
	NAL_UNIT_RESERVED_VCL29,
	NAL_UNIT_RESERVED_VCL30,
	NAL_UNIT_RESERVED_VCL31,

	NAL_UNIT_VPS,                     // 32
	NAL_UNIT_SPS,                     // 33
	NAL_UNIT_PPS,                     // 34
	NAL_UNIT_ACCESS_UNIT_DELIMITER,   // 35
	NAL_UNIT_EOS,                     // 36 (End of Sequence)
	NAL_UNIT_EOB,                     // 37 (End of Bitstream)
	NAL_UNIT_FILLER_DATA,             // 38
	NAL_UNIT_PREFIX_SEI,              // 39
	NAL_UNIT_SUFFIX_SEI,              // 40
	NAL_UNIT_RESERVED_NVCL41,
	NAL_UNIT_RESERVED_NVCL42,
	NAL_UNIT_RESERVED_NVCL43,
	NAL_UNIT_RESERVED_NVCL44,
	NAL_UNIT_RESERVED_NVCL45,
	NAL_UNIT_RESERVED_NVCL46,
	NAL_UNIT_RESERVED_NVCL47,
	NAL_UNIT_UNSPECIFIED_48,
	NAL_UNIT_UNSPECIFIED_49,
	NAL_UNIT_UNSPECIFIED_50,
	NAL_UNIT_UNSPECIFIED_51,
	NAL_UNIT_UNSPECIFIED_52,
	NAL_UNIT_UNSPECIFIED_53,
	NAL_UNIT_UNSPECIFIED_54,
	NAL_UNIT_UNSPECIFIED_55,
	NAL_UNIT_UNSPECIFIED_56,
	NAL_UNIT_UNSPECIFIED_57,
	NAL_UNIT_UNSPECIFIED_58,
	NAL_UNIT_UNSPECIFIED_59,
	NAL_UNIT_UNSPECIFIED_60,
	NAL_UNIT_UNSPECIFIED_61,
	NAL_UNIT_UNSPECIFIED_62,
	NAL_UNIT_UNSPECIFIED_63,
	NAL_UNIT_INVALID,
};

//! \}

#endif // end of #ifndef  __COMMONDEF__

