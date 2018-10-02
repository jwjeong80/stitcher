#include "OBUWriter.h"
#include "CommonDef.h"
#include "av1_common.h"

COBUWriter::COBUWriter(void)
	: m_bAnnexB(true)
	, m_uiNumTileRows(0)
	, m_uiNumTileCols(0)
	, m_uiNumParsers(0)
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		m_pSequenceHdrs[i] = NULL;

		for (int j = 0; j < MAX_OBUS_IN_TU; j++)		
		{
			m_pTileHdrs[i][j] = NULL;
			m_pTileDatas[i][j] = NULL;
		}
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

	m_OBUOutBuf = new uint8_t[INI_BUF_SIZE];
	m_OBUBufSize = INI_BUF_SIZE;
	m_OBUOutIdx = 0;
	m_pOBUOutBufStart = m_OBUOutBuf;

	//m_PsManager.Init();
	//m_CavlcEncoder.setBitstream(&(m_OutputNALU.m_Bitstream));
}

void COBUWriter::Destroy()
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		m_pSequenceHdrs[i] = NULL;
		for (int j = 0; j < MAX_OBUS_IN_TU; j++)
		{
			m_pTileHdrs[i][j] = NULL;
			m_pTileDatas[i][j] = NULL;

		}

		//SAFE_DELETES(m_pNewSliceSegHdrs[i]);
	}
	m_pOBUOutBufStart = NULL;
	printf("Destroy\n");

	SAFE_DELETES(m_OBUOutBuf);
	//m_PsManager.Destroy();
}


void COBUWriter::SetTileData(const uint8_t *pTileHeader, const uint8_t* pTileData, int32_t iParserIdx, int iObuIdx) {
	// get only pointers
	m_pTileHdrs[iParserIdx][iObuIdx] = pTileHeader;
	m_pTileDatas[iParserIdx][iObuIdx] = pTileData;
}

uint32_t COBUWriter::write_obu_header(int obu_type, int obu_extension, uint8_t *const dst, int bit_buffer_offset) {
	CBitWriter wb(dst, bit_buffer_offset);
	uint32_t size = 0;
	uint32_t before_size = bit_buffer_offset >> 3;
	assert(bit_buffer_offset % 8 == 0);

	wb.aom_wb_write_literal(0, 1);  // forbidden bit.
	wb.aom_wb_write_literal((int)obu_type, 4);
	wb.aom_wb_write_literal(obu_extension ? 1 : 0, 1);
	wb.aom_wb_write_literal(1, 1);  // obu_has_payload_length_field
	wb.aom_wb_write_literal(0, 1);  // reserved

	if (obu_extension) {
		wb.aom_wb_write_literal(obu_extension & 0xFF, 8);
	}

	size = wb.aom_wb_bytes_written();
	return size - before_size;
}

size_t COBUWriter::obu_memmove(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *data, uint32_t byte_offset) {
	const size_t length_field_size = aom_uleb_size_in_bytes(obu_payload_size);
	const uint32_t move_dst_offset = (uint32_t)length_field_size + obu_header_size + byte_offset;
	const uint32_t move_src_offset = obu_header_size + byte_offset;
	const uint32_t move_size = obu_payload_size;
	memmove(data + move_dst_offset, data + move_src_offset, move_size);
	return length_field_size;
}

int COBUWriter::write_uleb_obu_size(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *dest) {
	const uint32_t obu_size = obu_payload_size;
	const uint32_t offset = obu_header_size;
	size_t coded_obu_size = 0;

	if (aom_uleb_encode(obu_size, sizeof(obu_size), dest + offset,
		&coded_obu_size) != 0) {
		return AOM_CODEC_ERROR;
	}

	return AOM_CODEC_OK;
}


uint32_t COBUWriter::write_tile_group_header(uint8_t *const dst, int bit_buffer_offset) {
	
	CBitWriter wb(dst, bit_buffer_offset);
	uint32_t size = 0;
	uint32_t before_size = bit_buffer_offset >> 3;
	assert(bit_buffer_offset % 8 == 0);

	if(m_uiNumTileRows*m_uiNumTileCols == 1)
	   return size;

	//tile_start_and_end_present_flag = 0 
	wb.aom_wb_write_bit(0);

	size = wb.aom_wb_bytes_written();
	return size - before_size;
}

int COBUWriter::choose_size_bytes(uint32_t size, int spare_msbs) {
	// Choose the number of bytes required to represent size, without
	// using the 'spare_msbs' number of most significant bits.

	// Make sure we will fit in 4 bytes to start with..
	if (spare_msbs > 0 && size >> (32 - spare_msbs) != 0) return -1;

	// Normalise to 32 bits
	size <<= spare_msbs;

	if (size >> 24 != 0)
		return 4;
	else if (size >> 16 != 0)
		return 3;
	else if (size >> 8 != 0)
		return 2;
	else
		return 1;
}

int	 COBUWriter::reallocBuf(uint32_t uiNewBufSize)
{
	if (uiNewBufSize > m_OBUBufSize)
	{
		uint8_t* pTemp = new uint8_t[uiNewBufSize];
		if (pTemp)
		{
			if (m_OBUOutIdx)
			{
				memcpy(pTemp, m_OBUOutBuf, m_OBUOutIdx);
			}
			delete[] m_OBUOutBuf;
			m_OBUOutBuf = pTemp;
			m_OBUBufSize = uiNewBufSize;
			return 1;
		}
		else
		{
			// TRACE(L"ERROR: Unable to realloc output bitstream buffer\n");
			return 0;
		}
	}
	return 1;
}

int COBUWriter::reallocBufAdd(uint32_t uiAddBufSize)
{
	if (m_OBUBufSize - m_OBUOutIdx < uiAddBufSize)
	{
		uint32_t uiNewBufSize = (m_OBUBufSize + uiAddBufSize) << 1;	// doubling
		return reallocBuf(uiNewBufSize);
	}
	return 1;
}
