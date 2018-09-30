#pragma once
#include <stdint.h>
#include "Define.h"
#include <string.h>
#include "bitstream_writer.h"

class COBUWriter
{
public:
	COBUWriter(void);
	~COBUWriter(void);
	void                Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexB);
	void                Destroy();

	void SetTileData(const uint8_t *pTileHeader, const uint8_t* pTileData, int32_t iParserIdx, int iObuIdx);

	void initialize() {
		m_OBUOutSize = 0;
	}

	void SetOBUOutBuf(const uint8_t* pMemData, uint32_t MemSize) {
		memcpy(m_OBUOutBuf + m_OBUOutSize, pMemData, MemSize);
		m_OBUOutSize += MemSize;
	}

	void WriteTemporalDelimiter() {
		memcpy(m_OBUOutBuf + m_OBUOutSize, m_TemporalDelimiter, 2);
		m_OBUOutSize += 2;
	}

	uint8_t* getOBUOutBuf() { return m_OBUOutBuf;}
	uint8_t* getOBUOutBufStart() { return m_pOBUOutBufStart; }
	uint32_t getOBUOutBufSize() { return m_OBUOutSize; }

	uint32_t write_obu_header(int obu_type, int obu_extension, uint8_t *const dst, int bit_buffer_offset);
	size_t obu_memmove(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *data, int bit_buffer_offset);
	int write_uleb_obu_size(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *dest);

private:
	bool                m_bAnnexB;
	uint32_t            m_uiNumTileRows;
	uint32_t            m_uiNumTileCols;

	uint32_t            m_uiNumParsers;                     // check number of parsers

	const uint8_t*      m_pSequenceHdrs[MAX_STREAMS];       // only pointer for tile headers

	const uint8_t*      m_pTileHdrs[MAX_STREAMS][MAX_OBUS_IN_TU];       // only pointer for tile headers
	const uint8_t*   	m_pTileDatas[MAX_STREAMS][MAX_OBUS_IN_TU];       // only pointer for tile datas
	const uint8_t*     m_pNewFrameHeader;    // storage for merged slice header
	const uint8_t      m_TemporalDelimiter[2] = { 0x12, 0x00 };

	uint8_t* m_OBUOutBuf;
	uint8_t* m_pOBUOutBufStart;
	uint32_t m_OBUOutSize;
};