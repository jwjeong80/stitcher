#pragma once
#include <stdint.h>
#include "Define.h"
#include <string.h>

class COBUWriter
{
public:
	COBUWriter(void);
	~COBUWriter(void);
	void                Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexB);
	void                Destroy();

	void SetTileData(const uint8_t *pTileHeader, const uint8_t* pTileData, int32_t iParserIdx);

	void setOBUOutBuf(const uint8_t* pMemData, uint32_t MemSize) {
		memcpy(m_OBUOutBuf, pMemData, MemSize);
		m_OBUOutSize += MemSize;
	}

private:
	bool                m_bAnnexB;
	uint32_t            m_uiNumTileRows;
	uint32_t            m_uiNumTileCols;
	uint32_t            m_uiNumParsers;                     // check number of parsers

	const uint8_t*      m_pSequenceHdrs[MAX_STREAMS];       // only pointer for tile headers

	const uint8_t*      m_pTileHdrs[MAX_STREAMS];       // only pointer for tile headers
	const uint8_t*   	m_pTileDatas[MAX_STREAMS];       // only pointer for tile datas
	const uint8_t*     m_pNewFrameHeader;    // storage for merged slice header
	const uint8_t      m_TemporalDelimiter[2] = { 0x12, 0x00 };

	uint8_t* m_OBUOutBuf;
	uint32_t m_OBUOutSize;
};