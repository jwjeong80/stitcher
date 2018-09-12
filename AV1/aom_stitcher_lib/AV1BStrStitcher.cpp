/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: BStrStitcher.cpp
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#include "stdafx.h"

#include "CommonDef.h"
#include "AV1BStrStitcher.h"

#include "av1/decoder/obu.h"

CAV1BStrStitcher::CAV1BStrStitcher(void)
	: m_uiNumParsers(0)
	, m_bShowLogsFlag(0)
	, m_bAnnexBFlag(1)
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		m_pOBUParser[i] = NULL;
	}
}

CAV1BStrStitcher::~CAV1BStrStitcher(void)
{
	Destroy();
}

int CAV1BStrStitcher::Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexB)
{
	m_uiNumParsers = uiNumTileRows * uiNumTileCols;

	for (int i = 0; i < m_uiNumParsers; i++)
	{
		m_pOBUParser[i] = new COBUParser;
		m_pOBUParser[i]->Create();
	}
	//m_HevcWriter.Create(uiNumTileRows, uiNumTileCols, bAnnexB);

	return 1;
}

void CAV1BStrStitcher::Destroy()
{
	for (int i = 0; i < m_uiNumParsers; i++)
	{
		if (m_pOBUParser[i])
		{
			m_pOBUParser[i]->Destroy();
			SAFE_DELETES(m_pOBUParser[i]);
		}
	}

	//m_HevcWriter.Destroy();
}

int CAV1BStrStitcher::StitchSingleOBU(const OBU *pInpOBUs, uint32_t uiAnnexBFlags, OBU *pOutOBUs)
{
	uint8_t *pBitstream;
	uint32_t uiBitstreamSize;

	//bool bRwGlbHdrsFlag = uiStitchFlags & WRITE_GLB_HDRS;
	bool bAnnexB = uiAnnexBFlags;

	AV1Decoder pbi;

	 //decode all OBUs
	for (int i = 0; i < m_uiNumParsers; i++)
	{
		pBitstream = pInpOBUs[i].pMemAddrOfOBU;
		uiBitstreamSize = pInpOBUs[i].uiSizeOfOBUs;

		m_pOBUParser[i]->DecodeOneOBU(pBitstream, uiBitstreamSize, bAnnexB);

		//const uint8_t **ppSource = &pInpOBUs[i].pEachOBU[0];
		//const uint8_t *pSource = *ppSource;

		//const uint8_t *pSource = pInpOBUs[i].pEachOBU[0];
		//const uint8_t **ppSource = &pSource;

		//aom_decode_frame_from_obus(&pbi, pSource, pSource + pInpOBUs[i].uiEachOBUSize[0], ppSource);
		//aom_decode_frame_from_obus(&pbi, pInpOBUs[i].pEachOBU[0], pInpOBUs[i].pEachOBU[0] + pInpOBUs[i].uiEachOBUSize[0], &pInpOBUs[i].pEachOBU[0]);


		//pBitstream = pInpAUs[i].pNALU[0];
		//uiBitstreamSize = (uint32_t)(pInpAUs[i].pNALU[pInpAUs[i].uiNumOfNALU - 1] - pInpAUs[i].pNALU[0]) + pInpAUs[i].uiNALUSize[pInpAUs[i].uiNumOfNALU - 1];

		//m_pHevcParser[i]->DecodeOneAU(pBitstream, uiBitstreamSize);

		//// load pointers of header information from HevcParser
		//m_HevcWriter.SetSlice(m_pHevcParser[i]->GetSliceHeader(), m_pHevcParser[i]->GetSliceSegData(), i);
	}

	//// error handling .....
	//if (!m_HevcWriter.ValidateParameters())
	//{
	//	fprintf(stdout, "ERROR: Invalid parameters!\n");
	//	return 0;
	//}
	//else if (m_bShowLogsFlag)
	//{
	//	fprintf(stdout, "All parameters are validated!\n");
	//}

	//// rewriting header information
	//m_HevcWriter.RewriteHeaders(bRwGlbHdrsFlag);

	//// merge into single stream
	//m_HevcWriter.EncodeSingleAU(pOutAUs, bRwGlbHdrsFlag);

	return 1;
}
