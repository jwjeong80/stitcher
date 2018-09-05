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
#include "av1/decoder/obu.h"

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
	COBUInfo();
	~COBUInfo();

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

	void                Create();
	void                Destroy();

	bool                DecodeOneOBU(uint8_t *pBuf, uint32_t uiBufSize);

	static aom_codec_err_t ReadObuHeader(struct aom_read_bit_buffer *rb,
		int is_annexb, ObuHeader *header);
	aom_codec_err_t AomReadObuHeader(uint8_t *buffer, size_t buffer_length,
		size_t *consumed, ObuHeader *header, int is_annexb);

	int AomDecodeFrameFromObus(struct AV1Decoder *pbi, const uint8_t *data,
                               const uint8_t *data_end,
                               const uint8_t **p_data_end);
	static aom_codec_err_t AomGetNumLayersFromOperatingPointIdc(
		int operating_point_idc, unsigned int *number_spatial_layers,
		unsigned int *number_temporal_layers);
	static int IsObuInCurrentOperatingPoint(AV1Decoder *pbi,
		ObuHeader obu_header);
	static int ByteAlignment(AV1_COMMON *const cm,
		struct aom_read_bit_buffer *const rb);
	static int ReadBitstreamLevel(BitstreamLevel *bl,
		struct aom_read_bit_buffer *rb);
	static int AreSeqHeadersConsistent(const SequenceHeader *seq_params_old,
		const SequenceHeader *seq_params_new);
	static uint32_t ReadSequenceHeaderObu(AV1Decoder *pbi,
		struct aom_read_bit_buffer *rb);
	static uint32_t ReadFrameHeaderObu(AV1Decoder *pbi,
		struct aom_read_bit_buffer *rb,
		const uint8_t *data,
		const uint8_t **p_data_end,
		int trailing_bits_present);
	//static int32_t COBUParser::ReadTileGroupHeader(AV1Decoder *pbi,
	//	struct aom_read_bit_buffer *rb,
	//	int *start_tile, int *end_tile,
	//	int tile_start_implicit);
	//static uint32_t COBUParser::ReadOneTileGroupObu(
	//	AV1Decoder *pbi, struct aom_read_bit_buffer *rb, int is_first_tg,
	//	const uint8_t *data, const uint8_t *data_end, const uint8_t **p_data_end,
	//	int *is_last_tg, int tile_start_implicit);
	static void AllocTileListBuffer(AV1Decoder *pbi);
	static void CopyDecodedTileToTileListBuffer(AV1Decoder *pbi,
		uint8_t **output);
	static uint32_t ReadAndDecodeOneTileList(AV1Decoder *pbi,
		struct aom_read_bit_buffer *rb,
		const uint8_t *data,
		const uint8_t *data_end,
		const uint8_t **p_data_end,
		int *frame_decoding_finished);
	static void ReadMetadataItutT35(const uint8_t *data, size_t sz);
	static void ReadMetadataHdrCll(const uint8_t *data, size_t sz);
	static void ReadMetadataHdrMdcv(const uint8_t *data, size_t sz);
	static void ScalabilityStructure(struct aom_read_bit_buffer *rb);
	static void ReadMetadataScalability(const uint8_t *data, size_t sz);
	static void ReadMetadataTimecode(const uint8_t *data, size_t sz);
	static size_t ReadMetadata(const uint8_t *data, size_t sz);
	static aom_codec_err_t ReadObuSize(const uint8_t *data,
		size_t bytes_available,
		size_t *const obu_size,
		size_t *const length_field_size);

	static uint32_t ReadTemporalDelimiterObu() { return 0; }
	//uint8_t*            FindStartCode2(uint8_t* pStart, uint8_t* pEnd);
	//uint32_t            ExtractOneNALU(uint8_t* pBuf, uint32_t dwBufSize, Bool bLast);	// return : slice size, 0 => incomplete slice

	//TComSlice*          GetSliceHeader() { return m_AuInfo.m_pSlicePilot; }
	//TComInputBitstream* GetSliceSegData() { return &m_SliceSegData; }

private:
	//PsManager           m_PsManager;                // Parameter Set Manager
	COBUInfo             m_AuInfo;                   // storage for slice header & segment data
	//TComInputBitstream	m_SliceSegData;             // storage for slice segment data

	//Bool                m_bShowLogsFlag;

	//TDecCavlc           m_CavlcDecoder;
	//SEIReader           m_SeiReader;

	//TComInputBitstream  m_InputBitstream;
	//CSEIMessages        m_SEIs;                     ///< List of SEI messages that have been received before the first slice and between slices

	//Bool                m_bFirstSliceInBitstream;   // 비트스트림 중간에 EOS를 포함할 수도 있는 전체 비트스트림

protected:
	//Bool                xParseNonVclNal(NALUnit& nalu, int iThreadIdx);
	//Void                xParseVPS(TDecCavlc* pcCavlcDecoder, PsManager* pPsManager);
	//Void                xParseSPS(TDecCavlc* pcCavlcDecoder, PsManager* pPsManager);
	//Void                xParsePPS(TDecCavlc* pcCavlcDecoder, PsManager* pPsManager);
	//Bool                xParseSliceHeader(CAuInfo* pAuInfo, TDecCavlc* pcCavlcDecoder, int bFirstSliceInPicture, int& bRapPicFlag);
	//Bool                xParseSliceSegData(CAuInfo* pAuInfo, TComInputBitstream* pcBitstreamVector, int bFirstSliceInPicture, int bRapPicFlag, CSEIMessages* pSEIs);

	//void                InferNoOutputPriorPicsFlag(bool bRapPicFlag, TComSlice* pcSlicePilot, CAuInfo* pAuInfo);
	//Bool                isSkipPictureForBLA(NalUnitType associatedIRAPType, Int pocCRA, Int& iPOCLastDisplay, TComSlice* pcSlice);
	//Bool                isRandomAccessSkipPicture(Int& pocRandomAccess, Int& iPOCLastDisplay, TComSlice* pcSlice);
	bool ParseObuHeader(uint8_t obu_header_byte, ObuHeader *obu_header);
	bool ParseObuExtensionHeader(uint8_t ext_header_byte, ObuHeader *obu_header);
	//void PrintObuHeader(const ObuHeader *header);
	//bool DumpObu(const uint8_t *data, int length, int *obu_overhead_bytes);
	bool ValidObuType(int obu_type);


};
#pragma once
