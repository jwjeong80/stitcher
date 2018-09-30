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

	m_OBUOutBuf = new uint8_t[8192];
	m_OBUOutSize = 0;
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

	wb.aom_wb_write_literal(0, 1);  // forbidden bit.
	wb.aom_wb_write_literal((int)obu_type, 4);
	wb.aom_wb_write_literal(obu_extension ? 1 : 0, 1);
	wb.aom_wb_write_literal(1, 1);  // obu_has_payload_length_field
	wb.aom_wb_write_literal(0, 1);  // reserved

	if (obu_extension) {
		wb.aom_wb_write_literal(obu_extension & 0xFF, 8);
	}

	size = wb.aom_wb_bytes_written();
	return size;
}

size_t COBUWriter::obu_memmove(uint32_t obu_header_size, uint32_t obu_payload_size, uint8_t *data, int bit_buffer_offset) {
	const size_t length_field_size = aom_uleb_size_in_bytes(obu_payload_size);
	const uint32_t move_dst_offset = (uint32_t)length_field_size + obu_header_size + bit_buffer_offset;
	const uint32_t move_src_offset = obu_header_size + bit_buffer_offset;
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
