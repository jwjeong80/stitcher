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

	void SetTileData(const uint8_t *pTileHeader, const uint8_t* pTileData, int32_t iParserIdx, int iFrameObuIdx);

	void initialize() {
		m_OBUOutIdx = 0;
	}

	void SetOBUOutBuf(const uint8_t* pMemData, uint32_t MemSize) {
		memcpy(m_OBUOutBuf + m_OBUOutIdx, pMemData, MemSize);
		m_OBUOutIdx += MemSize;
	}

	void WriteTemporalDelimiter() {
		memcpy(m_OBUOutBuf + m_OBUOutIdx, m_TemporalDelimiter, 2);
		m_OBUOutIdx += 2;
	}

	uint8_t* getOBUOutBuf() { return m_OBUOutBuf;}
	uint8_t* getOBUOutBuf(int byte_offset) { return m_OBUOutBuf + byte_offset; }
	uint32_t getOBUOutBufIdx() { return m_OBUOutIdx; }

	void addOBUOutBufIdx(uint32_t written_byte) { m_OBUOutIdx += written_byte; }
	void setOBUOutBufIdx(uint32_t written_byte) { m_OBUOutIdx = written_byte; }

	int	reallocBuf(uint32_t uiNewBufSize);
	int	reallocBufAdd(uint32_t uiAddBufSize);

	uint32_t write_obu_header(int obu_type, int obu_extension, uint8_t *const dst, int bit_buffer_offset);
	size_t obu_memmove(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *data, uint32_t byte_offset);
	int write_uleb_obu_size(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *dest);

	uint32_t write_tile_group_header(uint8_t *const dst, int bit_buffer_offset);

	int choose_size_bytes(uint32_t size, int spare_msbs);

	void inline mem_put_le32(void *vmem, int val) {
		unsigned char *mem = (unsigned char *)vmem;

		mem[0] = (unsigned char)((val >> 0) & 0xff);
		mem[1] = (unsigned char)((val >> 8) & 0xff);
		mem[2] = (unsigned char)((val >> 16) & 0xff);
		mem[3] = (unsigned char)((val >> 24) & 0xff);
	}



private:
	bool                m_bAnnexB;
	uint32_t            m_uiNumTileRows;
	uint32_t            m_uiNumTileCols;

	uint32_t            m_uiNumParsers;                     // check number of parsers

	const uint8_t*      m_pSequenceHdrs[MAX_STREAMS];       // only pointer for tile headers

	const uint8_t*      m_pTileHdrs[MAX_STREAMS][OBUS_IN_TU];       // only pointer for tile headers
	const uint8_t*   	m_pTileDatas[MAX_STREAMS][OBUS_IN_TU];       // only pointer for tile datas
	const uint8_t*     m_pNewFrameHeader;    // storage for merged slice header
	const uint8_t      m_TemporalDelimiter[2] = { 0x12, 0x00 };

	uint8_t* m_OBUOutBuf;
	uint32_t m_OBUOutIdx;
	uint32_t m_OBUBufSize;
};