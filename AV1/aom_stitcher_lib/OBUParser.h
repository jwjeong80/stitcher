/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: HEVCParser.h
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#pragma once

#include "Define.h"

//#include "../Common/NAL.h"
//#include "../Decoder/SEIread.h"
//#include "../Decoder/TDecCavlc.h"
//#include "av1/decoder/obu.h"
//#include "bit_reader.h"
#include "obu_header.h"
#include "av1_common.h"
#include "bit_reader_c.h"
#include "SequenceHeader.h"
#include "FrameHeader.h"
//typedef struct {
//	size_t size;  // Size (1 or 2 bytes) of the OBU header (including the
//				  // optional OBU extension header) in the bitstream.
//	OBU_TYPE type;
//	int has_size_field;
//	int has_extension;
//	// The following fields come from the OBU extension header and therefore are
//	// only used if has_extension is true.
//	int temporal_layer_id;
//	int spatial_layer_id;
//} ObuHeader;

const uint32_t kObuForbiddenBitMask = 0x1;
const uint32_t kObuForbiddenBitShift = 7;
const uint32_t kObuTypeBitsMask = 0xF;
const uint32_t kObuTypeBitsShift = 3;
const uint32_t kObuExtensionFlagBitMask = 0x1;
const uint32_t kObuExtensionFlagBitShift = 2;
const uint32_t kObuHasSizeFieldBitMask = 0x1;
const uint32_t kObuHasSizeFieldBitShift = 1;

// When extension flag bit is set:
// 8 bits: extension header
// 7,6,5
//   temporal ID
// 4,3
//   spatial ID
// 2,1,0
//   reserved bits
const uint32_t kObuExtTemporalIdBitsMask = 0x7;
const uint32_t kObuExtTemporalIdBitsShift = 5;
const uint32_t kObuExtSpatialIdBitsMask = 0x3;
const uint32_t kObuExtSpatialIdBitsShift = 3;

class	COBUInfo	// Private structure
{
public:

	COBUInfo() : 
		m_pSeqHdrObu(NULL),
		m_pFrameObu(NULL),
		m_pFrameHeader(NULL),
		m_pTileHeader(NULL),
		m_pTilePayload(NULL),
		m_SeqHdrSize(-1),
		m_SeqHdrObuHdrSize(-1),
		m_SeqHdrPayloadSize(-1),
		m_FrameObuSize(-1),
		m_FrameObuHdrSize(-1),
		m_FrameHdrSize(-1),
		m_TileHdrSize(-1),
		m_TilePayloadSize(-1) {}
	
	~COBUInfo();
	
	void initilize() {
		m_pSeqHdrObu = NULL;
		m_pFrameObu = NULL;
		m_pFrameHeader = NULL;
		m_pTileHeader = NULL;
		m_pTilePayload = NULL;
		m_SeqHdrSize = -1;
		m_SeqHdrObuHdrSize = -1;
		m_SeqHdrPayloadSize = -1;
		m_FrameObuSize = -1;
		m_FrameObuHdrSize = -1;
		m_FrameHdrSize = -1;
		m_TileHdrSize = -1;
		m_TilePayloadSize = -1;
	}

	ObuHeader m_obu_header;

	const uint8_t* m_pSeqHdrObu;  //sequence header OBU start address  
	const uint8_t* m_pFrameObu;   //frame OBU start address
	const uint8_t* m_pFrameHeader; // = m_pFrameObu + m_FrameObuSize;
	const uint8_t* m_pTileHeader;  // = m_pFrameHeader + m_FrameHeaderSize;
	const uint8_t* m_pTilePayload;    // = m_pTileHeaderStartAddr + m_TileHeaderSize;
	                                             
	
	//Sequence Header OBU:  
	// m_pSeqHeaderStartAddr
	//         |
	//         |m_SeqHeaderOBUSize|m_SeqHeaderDataSize|
	int m_SeqHdrSize;      // m_SeqHdrObuHdrSize+ m_SeqHeaderDataSize
	int m_SeqHdrObuHdrSize;        
	int m_SeqHdrPayloadSize;

	//Frame Header OBU:  
	// m_pFrameObuStartAddr                 m_pTileHeaderStartAddr
	//       |         m_pFrameHeaderStartAddr      |      m_pTileDataStartAddr            
	//       |                  |                   |                  |
	//       |m_FrameObuHdrSize | m_FrameHeaderSize | m_TileHeaderSize | m_TileDataSize |
	int m_FrameObuSize; // m_FrameObuSize + m_FrameObuHdrSize + m_TileHeaderSize + m_TileDataSize
	int m_FrameObuHdrSize;      
	int m_FrameHdrSize;   
	int m_TileHdrSize;   
	int m_TilePayloadSize;     



	//void                InitSkipVars();

	//TComSlice*          m_pSlicePilot;              // storage for slice header
	//NALUnit             m_Nalu;
	//int                 m_iSliceIdx;                // �� ��ó���� 0�� �ʱ�ȭ�ؾ� ��.
	//Int                 m_nPrevSlicePOC;
	//int                 m_bFirstPicInSeq;           // ������ ������ ù��° ��ó(AU)������ ��Ÿ���� �÷���

	//												// �� �������� ���� �������� �����̽� ��� �Ľ� ���� ��, ���� ������ ���۽� �����ؾ� ��!!!
	//Int                 m_prevTid0POC;
	//NalUnitType         m_prevAssociatedIRAPType;   ///< NAL unit type of the associated IRAP picture
	//Int                 m_prevPocCRA;               ///< POC number of the latest CRA picture
	//Int                 m_pocRandomAccess;          ///< POC number of the random access point (the first IDR or CRA picture)
	//Bool                m_prevSliceSkipped;
	//Int                 m_skippedPOC;

	//Int                 m_nLastPOCNoOutputPriorPics;
	//Bool                m_isNoOutputPriorPicsFlag;
	//Bool                m_craNoRaslOutputFlag;      //value of variable NoRaslOutputFlag of the last CRA pic

	//int                 m_iPOCLastDisplay;          // last POC in display order

	//int                 m_nActiveVpsId;
	//int                 m_nActiveSpsId;
	//int                 m_nActivePpsId;

	//Bool                m_bFirstSliceInSequence;    // EOS������ �ϳ��� ������
	//int                 m_bEosFlag;                 // ���� ������ �������� AU�� EOS�� �����Ѵٸ�, (1) ���� ������ ���� ���� m_bFirstSliceInSequence=true ���� �ʿ�
	//												// (2) ���� InitSkipVars() ���� �ʿ�
};


class COBUParser
{
public:
	COBUParser(void);
	~COBUParser(void);

	void                Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols);
	void                Destroy();

	bool                DecodeOneOBU(uint8_t *pBitStream, uint32_t uiBitstreamSize, bool AnnexB);

	void OBUInfoInitilize() {
		for(int i=0; i< MAX_OBUS_IN_TU; i++)
			m_ObuInfo[i].initilize();
	}

	bool ValidObuType(int obu_type);

	void SequenceHeaderCopy(CSequenceHeader *SourceSh) {
		memcpy(&m_ShBuffer, SourceSh, sizeof(CSequenceHeader));
	}

	void FrameHeaderCopy(CFrameHeader *SourceFh, int idx) {
		memcpy(&m_FhBuffer[idx], SourceFh, sizeof(CFrameHeader));
	}

	uint32_t SequenceHdrCompare(CSequenceHeader *srcSH) {
		return m_ShBuffer.SequencHeaderCompare(srcSH);
	}

	uint32_t FrameHdrCompare(CFrameHeader *srcFH, int idx) {
		return m_FhBuffer[idx].FrameHeaderCompare(srcFH);
	}
	void FrameHeaderInit(int idx) {
		return m_FhBuffer[idx].InitZeroParameterSet();
	}
	
	COBUParser& operator=(const COBUParser& rhs) {
		memcpy(&this->m_ShBuffer, &rhs.m_ShBuffer, sizeof(CSequenceHeader));
		return *this;
	}

	aom_codec_err_t ReadObuHeader(struct AomReadBitBuffer *rb,
		int is_annexb, ObuHeader *header);
	aom_codec_err_t AomReadObuHeader(uint8_t *buffer, size_t buffer_length,
		size_t *consumed, ObuHeader *header, int is_annexb);

	aom_codec_err_t ReadObuSize(const uint8_t *data,
		size_t bytes_available,
		size_t *const obu_size,
		size_t *const length_field_size);

	aom_codec_err_t AomReadObuHeaderAndSize(const uint8_t *data,
		size_t bytes_available,
		int is_annexb,
		ObuHeader *obu_header,
		size_t *const payload_size,
		size_t *const bytes_read);

	 uint32_t ReadTemporalDelimiterObu() { return 0; }
	 uint32_t ReadSequenceHeaderObu(CBitReader *rb);


	 bool DecodeOneOBUC(uint8_t *pBitStream, uint32_t uiBitstreamSize, bool bAnnexB, int iParserIdx);
	 aom_codec_err_t ReadObuHeaderC(CBitReader *rb, int is_annexb, ObuHeader *header);
	 aom_codec_err_t AomReadObuHeaderC(uint8_t *buffer, size_t buffer_length,
		 size_t *consumed, ObuHeader *header, int is_annexb);

	 uint32_t ReadFrameHeaderObu(CBitReader *rb, const uint8_t *data, int trainiling_bits_present, uint32_t num_frame_obu);
	 int32_t ReadTileGroupHeader(CBitReader *rb, int *start_tile, int *end_tile, int tile_start_implicit, uint32_t num_frame_obu);

	 uint32_t RewriteSequenceHeaderObu(FrameSize_t *tileSizes, uint8_t *const dst, int bit_buffer_offset) {
		 CSequenceHeader *pSh = &m_ShBuffer;
		 return pSh->write_sequence_header_obu(tileSizes, dst, bit_buffer_offset);
	 }
	 uint32_t RewriteFrameHeaderObu(FrameSize_t *tile_sizes, uint8_t *const dst, int bit_buffer_offset, uint32_t num_frame_obu);

	 //OBU Infos
	 const uint8_t *getSeqHdr(int idx) { return m_ObuInfo[idx].m_pSeqHdrObu; }
	 int getSeqHeaderSize(int idx) { return m_ObuInfo[idx].m_SeqHdrSize; }

	 const uint8_t *getFrameObu(int idx) { return m_ObuInfo[idx].m_pFrameObu; }
	 const uint8_t *getTlieHeader(int idx) { return m_ObuInfo[idx].m_pTileHeader; }
	 const uint8_t *getTlieData(int idx) { return m_ObuInfo[idx].m_pTilePayload; }

	 int getFrameObuSize(int idx) { return m_ObuInfo[idx].m_FrameObuSize; }
	 int getTileSize(int idx) { return m_ObuInfo[idx].m_TilePayloadSize; }
	 OBU_TYPE getObuType(int idx) { return m_ObuInfo[idx].m_obu_header.type; }



	 int getNumberObu() { return m_NumObu; }
	 int getNumberFrameObu() { return m_numFrameOBU; }

	 int getShFrameWidth() { return m_ShBuffer.ShReadFrameWidth(); }
	 int getShFrameHeight() { return m_ShBuffer.ShReadFrameHeight(); }
	 
	 int getFhFrameWidth(int idx) { return m_FhBuffer[idx].FhReadFrameWidth(); }
	 int getFhFrameHeight(int idx) { return m_FhBuffer[idx].FhReadFrameHeight(); }
	 
	 CSequenceHeader getSeqHeaderBuffer() { return m_ShBuffer; }
	 CFrameHeader getFrameHeaderBuffer(int idx) { return m_FhBuffer[idx]; }



private:
	CSequenceHeader      m_ShBuffer;
	CFrameHeader         m_FhBuffer[OBUS_IN_TU];
	COBUInfo             m_ObuInfo[MAX_OBUS_IN_TU];                   // storage for slice header & segment data
	int                  m_NumObu;
	int                  m_ParserIdx;
	uint32_t            m_uiNumTileRows;
	uint32_t            m_uiNumTileCols;

	uint32_t            m_numFrameOBU;
	//TComInputBitstream	m_SliceSegData;             // storage for slice segment data

	//Bool                m_bShowLogsFlag;

	//TDecCavlc           m_CavlcDecoder;
	//SEIReader           m_SeiReader;

	//TComInputBitstream  m_InputBitstream;
	//CSEIMessages        m_SEIs;                     ///< List of SEI messages that have been received before the first slice and between slices

	//Bool                m_bFirstSliceInBitstream;   // ��Ʈ��Ʈ�� �߰��� EOS�� ������ ���� �ִ� ��ü ��Ʈ��Ʈ��
	int                   m_SeenFrameHeader;
protected:
};