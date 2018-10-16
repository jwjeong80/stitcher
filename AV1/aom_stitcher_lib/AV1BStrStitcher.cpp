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

	for (int i = 0; i < m_uiNumParsers + 1; i++)
	{
		m_pOBUParser[i] = new COBUParser;
		m_pOBUParser[i]->Create(uiNumTileRows, uiNumTileCols);
	}
	m_OBUWriter.Create(uiNumTileRows, uiNumTileCols, bAnnexB);


	//m_tileSizes = new FrameSize[m_uiNumParsers];
	m_uiWidths = new uint32_t[m_uiNumParsers];
	m_uiHeights = new uint32_t[m_uiNumParsers];
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

	//SAFE_DELETES(m_tileSizes);

	SAFE_DELETES(m_uiWidths);
	SAFE_DELETES(m_uiHeights);

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
	//CSequenceHeader::InitilizeFrameSize();
	uint32_t max_tile_size = 0;
	for (int i = 0; i < m_uiNumParsers; i++)
	{
		for (int k = 0; k < OBUS_IN_TU; k++)
			m_pOBUParser[i]->FrameHeaderInit(k);

		m_pOBUParser[i]->OBUInfoInitilize();
		pBitstream = pInpOBUs[i].pMemAddrOfOBU;
		uiBitstreamSize = pInpOBUs[i].uiSizeOfOBUs;

		m_pOBUParser[i]->DecodeOneOBUC(pBitstream, uiBitstreamSize, bAnnexB, i);

		bRwSeqHdrsFlag = m_pOBUParser[i]->getSeqHdr(1) != NULL;

		m_tileSizes[i].frame_width = m_pOBUParser[i]->getFhFrameWidth(0);
		m_tileSizes[i].frame_height = m_pOBUParser[i]->getFhFrameHeight(0);

		if (m_pOBUParser[i]->getNumberFrameObu() == 2)
			assert((m_pOBUParser[i]->getFhFrameWidth(0) == m_pOBUParser[i]->getFhFrameWidth(1)) &&
			(m_pOBUParser[i]->getFhFrameHeight(0) == m_pOBUParser[i]->getFhFrameHeight(1)));

		
		//// load pointers of header information from HevcParser
		//assert(m_pOBUParser[i]->getNumberObu() == 3);
		int frame_obu_num = 0;
		for (int obu_num = 1; obu_num < m_pOBUParser[i]->getNumberObu(); obu_num++) {
	
			if (m_pOBUParser[i]->getObuType(obu_num) == OBU_FRAME) {
				m_OBUWriter.SetTileData(m_pOBUParser[i]->getTlieHeader(obu_num), m_pOBUParser[i]->getTlieData(obu_num), 
					i /*parser_idx*/, frame_obu_num /*tile data storage idx*/);

				uint32_t tile_size = m_pOBUParser[i]->getTileSize(obu_num);
				max_tile_size = AOMMAX(tile_size, max_tile_size);
				frame_obu_num++;
			}
		}

	}

	int bit_offset = 0;
	uint32_t prev_obu_written_byte = 0;
	uint32_t total_obu_written_byte = 0;
	uint32_t cur_obu_written_byte = 0;

	////////////////////////////////////////////////////////////////
	////////////// Write Temporal Delimiter .
	////////////////////////////////////////////////////////////////
	m_OBUWriter.WriteTemporalDelimiter();
	bit_offset = 16; //temporal delimiter: 2byte * 8 = 16bit
	total_obu_written_byte = 2;
	
	////////////////////////////////////////////////////////////////
	/////////////// Write Sequence Header
	////////////////////////////////////////////////////////////////
	prev_obu_written_byte = total_obu_written_byte;
	if (bRwSeqHdrsFlag) {
		//Check
		for (int i = 1; i < m_uiNumParsers; i++) {	
			m_pOBUParser[0]->SequenceHdrCompare(&m_pOBUParser[i]->getSeqHeaderBuffer());
		}

		m_pOBUParser[m_uiNumParsers]->SequenceHeaderCopy(&m_pOBUParser[0]->getSeqHeaderBuffer());
		
		//write sequence header obu
		uint32_t obu_header_size = m_OBUWriter.write_obu_header(int(OBU_SEQUENCE_HEADER), 0 /*obu_extension*/, m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += obu_header_size;
		uint32_t obu_header_offset = total_obu_written_byte;
		uint32_t cur_obu_written_byte = obu_header_size;
		bit_offset = total_obu_written_byte << 3;
		//uint32_t obu_header_offset = written_byte

		uint32_t obu_payload_size = m_pOBUParser[m_uiNumParsers]->RewriteSequenceHeaderObu(m_tileSizes, m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += obu_payload_size;
		cur_obu_written_byte += obu_payload_size;

		const size_t length_field_size = m_OBUWriter.obu_memmove(obu_header_size, obu_payload_size, m_OBUWriter.getOBUOutBuf(), prev_obu_written_byte);
		if (m_OBUWriter.write_uleb_obu_size(obu_header_offset, obu_payload_size, m_OBUWriter.getOBUOutBuf()) != AOM_CODEC_OK) {
			return AOM_CODEC_ERROR;
		}
		int seq_header_size = obu_header_size + length_field_size + obu_payload_size;
		total_obu_written_byte += length_field_size;
		cur_obu_written_byte += length_field_size;
		bit_offset = total_obu_written_byte << 3;

		m_OBUWriter.addOBUOutBufIdx(cur_obu_written_byte);
		assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());
		//m_OBUWriter.SetOBUOutBuf(m_pOBUParser[0]->getSeqHeader(1), m_pOBUParser[0]->getSeqHeaderSize(1));
	}

	//Frame header 정보는 frame_obu_num index를 따름
	//Tile data는 obu_num을 따름 
	////////////////////////////////////////////////////////
	////////////////////  Write OBU FRAME 
	/////////////////////////////////////////////////////////
	int frame_obu_num = 0;
	for (int obu_num = 1; obu_num < m_pOBUParser[0]->getNumberObu(); obu_num++) {

		prev_obu_written_byte = total_obu_written_byte;

		if (m_pOBUParser[0]->getObuType(obu_num) == OBU_FRAME) {
			//Copy frame buffer m_pOBUParser[m_uiNumParsers] from &m_pOBUParser[0]
			m_pOBUParser[m_uiNumParsers]->FrameHeaderCopy(&m_pOBUParser[0]->getFrameHeaderBuffer(frame_obu_num), frame_obu_num);

			for (int parseIdx = 1; parseIdx < m_uiNumParsers; parseIdx++) {
				m_pOBUParser[0]->FrameHdrCompare(&m_pOBUParser[parseIdx]->getFrameHeaderBuffer(frame_obu_num), frame_obu_num);
			}

			//Write OBU Header
			uint32_t obu_header_size = m_OBUWriter.write_obu_header(int(OBU_FRAME), 0 /*obu_extension*/, m_OBUWriter.getOBUOutBuf(), bit_offset);
			total_obu_written_byte += obu_header_size;
			uint32_t obu_header_offset = total_obu_written_byte; //이 파라미터는 뒤에 length_field_size를 추가하기 위해 필요함 
			uint32_t cur_obu_written_byte = obu_header_size;
			bit_offset = total_obu_written_byte << 3;

			//Rewrite Frame Header
			uint32_t frame_header_payload_size
				= m_pOBUParser[m_uiNumParsers]->RewriteFrameHeaderObu(m_tileSizes, m_OBUWriter.getOBUOutBuf(), bit_offset, frame_obu_num);
			total_obu_written_byte += frame_header_payload_size;
			cur_obu_written_byte += frame_header_payload_size;
			bit_offset = total_obu_written_byte << 3;

			//Rewrite Tile Header
			uint32_t tile_header_payload_size = m_OBUWriter.write_tile_group_header(m_OBUWriter.getOBUOutBuf(), bit_offset);
			total_obu_written_byte += tile_header_payload_size;
			cur_obu_written_byte += tile_header_payload_size;
			bit_offset = total_obu_written_byte << 3;
			
			//Syncronise buffer index
			m_OBUWriter.addOBUOutBufIdx(obu_header_size + frame_header_payload_size + tile_header_payload_size);
			assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());


			//Rewrite Tile data payload

			int tile_data_payload_size = 0;
			for (int i = 0; i < m_uiNumParsers; i++)
			{
				int TileSizeBytes = 4; //fixed in this code

				if (i < m_uiNumParsers - 1) {
					uint32_t tileSize = m_pOBUParser[i]->getTileSize(obu_num);
					uint32_t tile_size_minus_1 = tileSize - 1;
					//write tile size
					m_OBUWriter.mem_put_le32(m_OBUWriter.getOBUOutBuf(total_obu_written_byte), tile_size_minus_1); //TileSizeBytes = 4; 
					total_obu_written_byte += TileSizeBytes;
					cur_obu_written_byte += TileSizeBytes;
					m_OBUWriter.addOBUOutBufIdx(TileSizeBytes);
					//bit_offset = total_obu_written_byte << 3;

					//realloc memory
					m_OBUWriter.reallocBufAdd((tile_size_minus_1 << 1) + 2 + 4);

					//write tile data
					m_OBUWriter.SetOBUOutBuf(m_pOBUParser[i]->getTlieData(obu_num), tileSize);

					total_obu_written_byte += tileSize;
					cur_obu_written_byte += tileSize;
					tile_data_payload_size += (tileSize + TileSizeBytes);
				}
				else {
					uint32_t tileSize = m_pOBUParser[i]->getTileSize(obu_num);
					m_OBUWriter.reallocBufAdd((tileSize << 1) + 2 + 4);

					m_OBUWriter.SetOBUOutBuf(m_pOBUParser[i]->getTlieData(obu_num), tileSize);
					total_obu_written_byte += tileSize;
					cur_obu_written_byte += tileSize;
					tile_data_payload_size += (tileSize);
				}
			}
			assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());

			uint32_t obu_payload_size = frame_header_payload_size + tile_header_payload_size + tile_data_payload_size;
			const size_t length_field_size = m_OBUWriter.obu_memmove(obu_header_size, obu_payload_size, m_OBUWriter.getOBUOutBuf(), prev_obu_written_byte);
			if (m_OBUWriter.write_uleb_obu_size(obu_header_offset, obu_payload_size, m_OBUWriter.getOBUOutBuf()) != AOM_CODEC_OK) {
				return AOM_CODEC_ERROR;
			}
			total_obu_written_byte += length_field_size;
			cur_obu_written_byte += length_field_size;
			m_OBUWriter.addOBUOutBufIdx(length_field_size);
			assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());

			frame_obu_num++;
		}
	}

	pOutOBUs->pMemAddrOfOBU = m_OBUWriter.getOBUOutBuf();
	pOutOBUs->uiSizeOfOBUs = m_OBUWriter.getOBUOutBufIdx();
	assert(total_obu_written_byte == pOutOBUs->uiSizeOfOBUs);
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
