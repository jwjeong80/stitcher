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

#include "stdafx.h"

#define STITCH_EXPORTS

#include "AV1BStrStitcher.h"
#include "AV1StitcherApi.h"

#ifdef __cplusplus
extern "C" {
#endif

	STITCH_API int WINAPI Keti_AV1_Stitcher_Create(void** ppHandle, uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexBflag)
	{
		if (ppHandle)
		{
			if (!uiNumTileRows || !uiNumTileCols)
			{
				fprintf(stdout, "Error: please check the tile structure, %d x %d!!\n", uiNumTileRows, uiNumTileCols);
				return 0;
			}

			//if (uiNumTileRows * uiNumTileCols > MAX_STREAMS)
			if (uiNumTileRows * uiNumTileCols > 100)
			{
				fprintf(stdout, "Error: the number of input streams are over the specification of stitcher!!\n");
				return 0;
			}

			CAV1BStrStitcher *pcAV1BStrStitcher = new CAV1BStrStitcher;
			*ppHandle = pcAV1BStrStitcher;
			return pcAV1BStrStitcher->Create(uiNumTileRows, uiNumTileCols, bAnnexBflag);
		}

		return 0;
	}

	STITCH_API int WINAPI Keti_AV1_Stitcher_StitchSingleOBU(void* pHandle/*[in]*/, const OBU *pInpOBUs/*[in]*/, uint32_t uiStitchFlags, OBU *pOutOBUs/*[out]*/)
	{
		if (pHandle)
		{
			CAV1BStrStitcher *pcAV1BStrStitcher = (CAV1BStrStitcher *)pHandle;
			for (int i = 0; i < pcAV1BStrStitcher->m_uiNumParsers; i++)
			{
				if (!pInpOBUs[i].uiNumOfOBU)
				{
					fprintf(stdout, "Error: one of input streams is empty, @%2d\n", i);
					return 0;
				}
			}
			return pcAV1BStrStitcher->StitchSingleOBU(pInpOBUs, uiStitchFlags, pOutOBUs);
		}

		return 0;
	}

	STITCH_API void WINAPI Keti_AV1_Stitcher_Destroy(void* pHandle)
	{
		if (pHandle)
		{
			CAV1BStrStitcher *pcAV1BStrStitcher = (CAV1BStrStitcher *)pHandle;
			pcAV1BStrStitcher->Destroy();
			delete pcAV1BStrStitcher;
		}
	}

#ifdef __cplusplus
}
#endif //extern "C"