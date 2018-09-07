#pragma once
/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: StitcherApi.h
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/
// Modified by yonghwan(2018.07.30)

#pragma once

#if defined (_WIN32)
#ifdef STATIC_LIB
#define STITCH_API
#else
#ifdef STITCH_EXPORTS
#define STITCH_API __declspec(dllexport)
#else
#define STITCH_API __declspec(dllimport)
#endif
#endif

#ifndef WINAPI 
#define WINAPI __stdcall
#endif

#elif defined(__linux__)
#define STITCH_API 
#define WINAPI
#endif

#include <stdint.h>

#ifndef MAX_OBU_NUM
// MAX_NUM_TILE_COLS = 20
// MAX_NUM_TILE_ROWS = 22
#define     MAX_OBU_NUM	(440 + 8) // = MAX_NUM_TILES + VPS/SPS/PPS...
struct OBU
{
	uint32_t    uiNumOfOBU;                    /* [in] */
	uint8_t*    pMemAddrOfOBU;            /* [in]; only pointer (continuous memory) */
	uint32_t    uiSizeOfOBUs;       /* [in] */
};
#endif // MAX_NALU_NUM

// input flags per picture or per sequence
enum eInputFlagsAV1
{
	NO_INPUT_FLAGS_AV1 = 0x00000000,
	WRITE_GLB_HDRS_AV1 = 0x00000001,
};

#ifdef __cplusplus
extern "C" {
#endif

	/*
	[params]
	- bAnnexBflag = 1: output is AnnexB format (4-byte start code + NALU)
	- bAnnexBflag = 0: output is MP4 length format (4-byte length + NALU) => Caution: 4-byte length is BIG-ENDIAN format
	[return]
	- 0: failure,  1: success
	*/
	STITCH_API int WINAPI Keti_AV1_Stitcher_Create(void** ppHandle/*[out]*/, uint32_t uiNumTileRows/*[in]*/, uint32_t uiNumTileCols/*[in]*/, bool bAnnexBflag/*[in]*/);
	/*
	[params]
	- pOutAUs.pNALU bitstream is AnnexB format or MP4 length format according to bAnnexBflag
	- pInpAUs.pNALU bitstream is 'AnnexB format only'
	[return]
	- 0: failure,  1: success
	*/
	STITCH_API int WINAPI Keti_AV1_Stitcher_StitchSingleOBU(void* pHandle/*[in]*/, const OBU *pInpOBUs/*[in]*/, uint32_t uiStitchFlags, OBU *pOutOBUs/*[out]*/);
	STITCH_API void WINAPI Keti_AV1_Stitcher_Destroy(void* pHandle/*[in]*/);

#ifdef __cplusplus
}
#endif