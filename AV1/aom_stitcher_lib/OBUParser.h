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
		m_pSeqHdrOBuStartAddr(NULL),
		m_pFrameObuStartAddr(NULL), 
		m_pFrameHeaderStartAddr(NULL), 
		m_pTileHeaderStartAddr(NULL),
		m_pTileDataStartAddr(NULL),
		m_SeqHdrSize(-1),
		m_SeqHdrObuHdrSize(-1),
		m_SeqHeaderDataSize(-1),
		m_FrameObuSize(-1),
		m_FrameObuHdrSize(-1),
		m_FrameHeaderSize(-1),
	    m_TileHeaderSize(-1),
	    m_TileDataSize(-1) {}
	
	~COBUInfo();
	
	void initilize() {
		m_pSeqHdrOBuStartAddr = NULL;
		m_pFrameObuStartAddr = NULL;
		m_pFrameHeaderStartAddr = NULL;
		m_pTileHeaderStartAddr = NULL;
		m_pTileDataStartAddr = NULL;
		m_SeqHdrSize = -1;
		m_SeqHdrObuHdrSize = -1;
		m_SeqHeaderDataSize = -1;
		m_FrameObuSize = -1;
		m_FrameObuHdrSize = -1;
		m_FrameHeaderSize = -1;
		m_TileHeaderSize = -1;
		m_TileDataSize = -1;
	}

	ObuHeader obu_header[10];

	const uint8_t* m_pSeqHdrOBuStartAddr;  //sequence header OBU start address  
	const uint8_t* m_pFrameObuStartAddr;   //frame OBU start address
	const uint8_t* m_pFrameHeaderStartAddr; // = m_pFrameObuStartAddr + m_FrameObuSize;
	const uint8_t* m_pTileHeaderStartAddr;  // = m_pFrameHeaderStartAddr + m_FrameHeaderSize;
	const uint8_t* m_pTileDataStartAddr;    // = m_pTileHeaderStartAddr + m_TileHeaderSize;
	                                             
	
	//Sequence Header OBU:  
	// m_pSeqHeaderStartAddr
	//         |
	//         |m_SeqHeaderOBUSize|m_SeqHeaderDataSize|
	int m_SeqHdrSize;      // m_SeqHdrObuHdrSize+ m_SeqHeaderDataSize
	int m_SeqHdrObuHdrSize;        
	int m_SeqHeaderDataSize;

	//Frame Header OBU:  
	// m_pFrameObuStartAddr                 m_pTileHeaderStartAddr
	//       |         m_pFrameHeaderStartAddr      |      m_pTileDataStartAddr            
	//       |                  |                   |                  |
	//       |m_FrameObuHdrSize | m_FrameHeaderSize | m_TileHeaderSize | m_TileDataSize |
	int m_FrameObuSize; // m_FrameObuSize + m_FrameObuHdrSize + m_TileHeaderSize + m_TileDataSize
	int m_FrameObuHdrSize;      
	int m_FrameHeaderSize;   
	int m_TileHeaderSize;   
	int m_TileDataSize;     



	//void                InitSkipVars();

	//TComSlice*          m_pSlicePilot;              // storage for slice header
	//NALUnit             m_Nalu;
	//int                 m_iSliceIdx;                // 매 픽처마다 0로 초기화해야 함.
	//Int                 m_nPrevSlicePOC;
	//int                 m_bFirstPicInSeq;           // 시퀀스 내에서 첫번째 픽처(AU)인지를 나타내는 플래그

	//												// 이 변수들은 이전 쓰레드의 슬라이스 헤더 파싱 종료 후, 현재 쓰레드 시작시 복사해야 함!!!
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

	//Bool                m_bFirstSliceInSequence;    // EOS까지의 하나의 시퀀스
	//int                 m_bEosFlag;                 // 이전 프레임 쓰레드의 AU에 EOS가 존재한다면, (1) 현재 쓰레드 시작 전에 m_bFirstSliceInSequence=true 설정 필요
	//												// (2) 또한 InitSkipVars() 실행 필요
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
		for(int i=0; i< 10; i++)
			m_ObuInfo[i].initilize();
	}

	bool ValidObuType(int obu_type);

	void SequenceHeaderCopy(CSequenceHeader *SourceSh) {
		memcpy(&m_ShBuffer, SourceSh, sizeof(CSequenceHeader));
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

	 uint32_t ReadFrameHeaderObu(CBitReader *rb, const uint8_t *data, int trainiling_bits_present);
	 int32_t ReadTileGroupHeader(CBitReader *rb, int *start_tile, int *end_tile, int tile_start_implicit);

	 uint32_t RewriteSequenceHeaderObu(uint8_t *const dst, int bit_buffer_offset) {
		 CSequenceHeader *pSh = &m_ShBuffer;
		 return pSh->write_sequence_header_obu(dst, bit_buffer_offset);
	 }
	 void RewriteFrameHeaderObu(uint8_t *const dst, int bit_buffer_offset); 

	 
	 const uint8_t *getSeqHeader(int idx) { return m_ObuInfo[idx].m_pSeqHdrOBuStartAddr; }
	 int getSeqHeaderSize(int idx) { return m_ObuInfo[idx].m_SeqHdrSize; }

	 const uint8_t *getFrameObu(int idx) { return m_ObuInfo[idx].m_pFrameObuStartAddr; }
	 const uint8_t *getTlieHeader(int idx) { return m_ObuInfo[idx].m_pTileHeaderStartAddr; }
	 const uint8_t *getTlieData(int idx) { return m_ObuInfo[idx].m_pTileDataStartAddr; }

	 int getFrameObuSize(int idx) { return m_ObuInfo[idx].m_FrameObuSize; }
	 int getNumberObu() { return m_NumObu; }

	 int getFrameWidth() { return m_ShBuffer.ShReadFrameWidth(); }
	 int getFrameHeight() { return m_ShBuffer.ShReadFrameHeight(); }

	 CSequenceHeader getSeqHeaderBuffer() { return m_ShBuffer; }

private:
	CSequenceHeader      m_ShBuffer;
	CFrameHeader         m_FhBuffer;
	//ShManager            m_ShManager;                // Parameter Set Manager
	COBUInfo             m_ObuInfo[10];                   // storage for slice header & segment data
	ObuHeader            m_ObuHeader[10];
	int                  m_NumObu;
	int                  m_ParserIdx;
	uint32_t            m_uiNumTileRows;
	uint32_t            m_uiNumTileCols;
	//TComInputBitstream	m_SliceSegData;             // storage for slice segment data

	//Bool                m_bShowLogsFlag;

	//TDecCavlc           m_CavlcDecoder;
	//SEIReader           m_SeiReader;

	//TComInputBitstream  m_InputBitstream;
	//CSEIMessages        m_SEIs;                     ///< List of SEI messages that have been received before the first slice and between slices

	//Bool                m_bFirstSliceInBitstream;   // 비트스트림 중간에 EOS를 포함할 수도 있는 전체 비트스트림
	int                   m_SeenFrameHeader;
protected:
};