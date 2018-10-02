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
		m_pOBUParser[i]->OBUInfoInitilize();
		pBitstream = pInpOBUs[i].pMemAddrOfOBU;
		uiBitstreamSize = pInpOBUs[i].uiSizeOfOBUs;

		m_pOBUParser[i]->DecodeOneOBUC(pBitstream, uiBitstreamSize, bAnnexB, i);

		bRwSeqHdrsFlag = m_pOBUParser[i]->getSeqHeader(1) != NULL;

		m_tileSizes[i].frame_width = m_pOBUParser[i]->getFhFrameWidth();
		m_tileSizes[i].frame_height = m_pOBUParser[i]->getFhFrameHeight();

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
		assert(m_pOBUParser[i]->getNumberObu() == 3);

		int j = bRwSeqHdrsFlag ? 2 : 1;
		for (; j < m_pOBUParser[i]->getNumberObu(); j++) {
			m_OBUWriter.SetTileData(m_pOBUParser[i]->getTlieHeader(0), m_pOBUParser[i]->getTlieData(0), i, j);
			uint32_t tile_size = m_pOBUParser[i]->getTileSize(j);
			max_tile_size = AOMMAX(tile_size, max_tile_size);
		}

		//m_HevcWriter.SetSlice(m_pHevcParser[i]->GetSliceHeader(), m_pHevcParser[i]->GetSliceSegData(), i);
		
	}

	int bit_offset = 0;
	uint32_t prev_obu_written_byte = 0;
	uint32_t total_obu_written_byte = 0;
	uint32_t cur_obu_written_byte = 0;
	m_OBUWriter.WriteTemporalDelimiter();
	bit_offset = 16; //temporal delimiter: 2byte * 8 = 16bit
	total_obu_written_byte = 2;
	
	prev_obu_written_byte = total_obu_written_byte;
	if (bRwSeqHdrsFlag) {
		m_pOBUParser[m_uiNumParsers]->SequenceHeaderCopy(&m_pOBUParser[0]->getSeqHeaderBuffer());
		
		//write sequence header obu
		uint32_t obu_header_size = m_OBUWriter.write_obu_header(int(OBU_SEQUENCE_HEADER), 0 /*obu_extension*/, m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += obu_header_size;
		uint32_t obu_header_offset = total_obu_written_byte;
		uint32_t cur_obu_written_byte = obu_header_size;
		bit_offset = total_obu_written_byte << 3;
		//uint32_t obu_header_offset = written_byte

		uint32_t obu_payload_size = m_pOBUParser[m_uiNumParsers]->RewriteSequenceHeaderObu(m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += obu_payload_size;
		cur_obu_written_byte += obu_payload_size;

		const size_t length_field_size = m_OBUWriter.obu_memmove(obu_header_size, obu_payload_size, m_OBUWriter.getOBUOutBuf(), obu_header_offset);
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

	//OBU FRAME HEADER
	prev_obu_written_byte = total_obu_written_byte;
	{
		m_pOBUParser[m_uiNumParsers]->FrameHeaderCopy(&m_pOBUParser[0]->getFrameHeaderBuffer());
	
		uint32_t obu_header_size = m_OBUWriter.write_obu_header(int(OBU_FRAME), 0 /*obu_extension*/, m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += obu_header_size;
		uint32_t obu_header_offset = total_obu_written_byte;
		uint32_t cur_obu_written_byte = obu_header_size;
		bit_offset = total_obu_written_byte << 3;

		uint32_t frame_header_payload_size 
			= m_pOBUParser[m_uiNumParsers]->RewriteFrameHeaderObu(m_tileSizes, m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += frame_header_payload_size;
		cur_obu_written_byte += frame_header_payload_size;
		bit_offset = total_obu_written_byte << 3;

		uint32_t tile_header_payload_size = m_OBUWriter.write_tile_group_header(m_OBUWriter.getOBUOutBuf(), bit_offset);
		total_obu_written_byte += tile_header_payload_size;
		cur_obu_written_byte += tile_header_payload_size;
		bit_offset = total_obu_written_byte << 3;

		m_OBUWriter.addOBUOutBufIdx(obu_header_size + frame_header_payload_size + tile_header_payload_size);
		assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());

		//search remux tile
		 int max_tile_size_byte = m_OBUWriter.choose_size_bytes(max_tile_size, 0);
		//tcsb = 4;  // This is ignored

		int obu_num = bRwSeqHdrsFlag ? 2 : 1;
		int tile_data_payload_size = 0;
		for (int i = 0; i < m_uiNumParsers; i++)
		{
			int TileSizeBytes = 4; //fixed in this code

			if(i < m_uiNumParsers - 1){
				uint32_t tileSize = m_pOBUParser[i]->getTileSize(obu_num);
				m_OBUWriter.mem_put_le32(m_OBUWriter.getOBUOutBuf(total_obu_written_byte), tileSize); //TileSizeBytes = 4; 
				total_obu_written_byte += TileSizeBytes;
				cur_obu_written_byte += TileSizeBytes;
				bit_offset = total_obu_written_byte << 3;

				m_OBUWriter.addOBUOutBufIdx(TileSizeBytes);
				m_OBUWriter.reallocBufAdd((tileSize << 1) + 2 + 4);
				m_OBUWriter.SetOBUOutBuf(m_pOBUParser[i]->getFrameObu(obu_num), tileSize);

				total_obu_written_byte += tileSize;
				cur_obu_written_byte += tileSize;
				tile_data_payload_size += (tileSize + TileSizeBytes);
			}
			else {
				uint32_t tileSize = m_pOBUParser[i]->getTileSize(obu_num);
				m_OBUWriter.reallocBufAdd((tileSize << 1) + 2 + 4);

				m_OBUWriter.SetOBUOutBuf(m_pOBUParser[i]->getFrameObu(obu_num), tileSize);
				total_obu_written_byte += tileSize;
				cur_obu_written_byte += tileSize;
				tile_data_payload_size += (tileSize);
			}

		}
		assert(total_obu_written_byte == m_OBUWriter.getOBUOutBufIdx());
		
		uint32_t obu_payload_size = frame_header_payload_size + tile_header_payload_size + tile_data_payload_size;
		const size_t length_field_size = m_OBUWriter.obu_memmove(obu_header_size, obu_payload_size, m_OBUWriter.getOBUOutBuf(), obu_header_offset);
		if (m_OBUWriter.write_uleb_obu_size(obu_header_offset, obu_payload_size, m_OBUWriter.getOBUOutBuf()) != AOM_CODEC_OK) {
			return AOM_CODEC_ERROR;
		}
		//length_field

	}

	//int j = bRwSeqHdrsFlag ? 2 : 1;
	//for (; j < m_pOBUParser[0]->getNumberObu(); j++)
	//	m_OBUWriter.SetOBUOutBuf(m_pOBUParser[0]->getFrameObu(j), m_pOBUParser[0]->getFrameObuSize(j));

	pOutOBUs->pMemAddrOfOBU = m_OBUWriter.getOBUOutBuf();
	pOutOBUs->uiSizeOfOBUs = m_OBUWriter.getOBUOutBufIdx();

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
