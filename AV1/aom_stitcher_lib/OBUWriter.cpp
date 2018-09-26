#include "OBUWriter.h"
#include "CommonDef.h"

COBUWriter::COBUWriter(void)
	: m_bAnnexB(true)
	, m_uiNumTileRows(0)
	, m_uiNumTileCols(0)
	, m_uiNumParsers(0)
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		m_pSequenceHdrs[i] = NULL;
		m_pTileHdrs[i] = NULL;
		m_pTileDatas[i] = NULL;
	}
}

COBUWriter::~COBUWriter(void)
{
	Destroy();
}

void COBUWriter::Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexB)
{
	m_uiNumTileRows = uiNumTileRows;
	m_uiNumTileCols = uiNumTileCols;
	m_uiNumParsers = uiNumTileRows * uiNumTileCols;
	m_bAnnexB = bAnnexB;

	//for (int i = 0; i < m_uiNumParsers; i++)
	//{
	//	m_pNewSliceSegHdrs[i] = new TComSlice;
	//}

	m_OBUOutBuf = new uint8_t[8192];
	m_OBUOutSize = 0;

	memcpy(m_OBUOutBuf, m_TemporalDelimiter, 2);
	m_OBUOutSize = 2;
	//m_PsManager.Init();
	//m_CavlcEncoder.setBitstream(&(m_OutputNALU.m_Bitstream));
}

void COBUWriter::Destroy()
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		m_pSequenceHdrs[i] = NULL;
		m_pTileHdrs[i] = NULL;
		m_pTileDatas[i] = NULL;

		//SAFE_DELETES(m_pNewSliceSegHdrs[i]);
	}
	SAFE_DELETES(m_OBUOutBuf);
	//m_PsManager.Destroy();
}


void COBUWriter::SetTileData(const uint8_t *pTileHeader, const uint8_t* pTileData, int32_t iParserIdx) {
	// get only pointers
	m_pTileHdrs[iParserIdx] = pTileHeader;
	m_pTileDatas[iParserIdx] = pTileData;
}