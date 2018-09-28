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

//#include "av1/decoder/obu.h"

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
	m_OBUWriter.Create(uiNumTileRows, uiNumTileCols, bAnnexB);
	
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

	m_OBUWriter.Destroy();
}

int CAV1BStrStitcher::StitchSingleOBU(const OBU *pInpOBUs, uint32_t uiAnnexBFlags, OBU *pOutOBUs)
{
	uint8_t *pBitstream;
	uint32_t uiBitstreamSize;

	m_OBUWriter.initialize();

	//bool bRwGlbHdrsFlag = uiStitchFlags & WRITE_GLB_HDRS;
	bool bAnnexB = uiAnnexBFlags;
	bool bRwSeqHdrsFlag;
	bool bSeqHeaderSame = true;
	 //decode all OBUs
	for (int i = 0; i < m_uiNumParsers; i++)
	{
		m_pOBUParser[i]->OBUInfoInitilize();
		pBitstream = pInpOBUs[i].pMemAddrOfOBU;
		uiBitstreamSize = pInpOBUs[i].uiSizeOfOBUs;

		m_pOBUParser[i]->DecodeOneOBUC(pBitstream, uiBitstreamSize, bAnnexB);

		bRwSeqHdrsFlag = m_pOBUParser[i]->getSeqHeader(1) != NULL;

		//if (bRwSeqHdrsFlag && i > 1) {
		//	if (m_pOBUParser[0]->getSeqHeaderSize() == m_pOBUParser[i]->getSeqHeaderSize()) {
		//		int same = memcmp(m_pOBUParser[0]->getSeqHeader(), m_pOBUParser[i]->getSeqHeader(), m_pOBUParser[i]->getSeqHeaderSize());
		//		bool bRwSeqHdrsSame = same == 0 ? true : false;
		//		bRwSeqHdrsFlag = bRwSeqHdrsFlag && bRwSeqHdrsSame;
		//		if(!bRwSeqHdrsSame)
		//			printf("Sequence memory datas are different at %d \n", i);
		//	}
		//	else {
		//		bRwSeqHdrsFlag = false;
		//		printf("Sequence memory sizes are different \n");
		//	}

		//}
		//// load pointers of header information from HevcParser
		int j = bRwSeqHdrsFlag ? 2 : 1;
		for(; j < m_pOBUParser[i]->getNumberObu(); j++)
			m_OBUWriter.SetTileData(m_pOBUParser[i]->getTlieHeader(0), m_pOBUParser[i]->getTlieData(0), i, j);
		//m_HevcWriter.SetSlice(m_pHevcParser[i]->GetSliceHeader(), m_pHevcParser[i]->GetSliceSegData(), i);
	}

	printf("");

	m_OBUWriter.WriteTemporalDelimiter();

	if(bRwSeqHdrsFlag)
	    m_OBUWriter.SetOBUOutBuf(m_pOBUParser[0]->getSeqHeader(1), m_pOBUParser[0]->getSeqHeaderSize(1));

	int j = bRwSeqHdrsFlag ? 2 : 1;
	for (; j < m_pOBUParser[0]->getNumberObu(); j++)
		m_OBUWriter.SetOBUOutBuf(m_pOBUParser[0]->getFrameObu(j), m_pOBUParser[0]->getFrameObuSize(j));

	pOutOBUs->pMemAddrOfOBU = m_OBUWriter.getOBUOutBuf();
	pOutOBUs->uiSizeOfOBUs = m_OBUWriter.getOBUOutBufSize();

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
