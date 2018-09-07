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

	void                Create();
	void                Destroy();

	bool                DecodeOneOBU(uint8_t *pBitStream, uint32_t uiBitstreamSize);

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

private:
	//PsManager           m_PsManager;                // Parameter Set Manager
	COBUInfo             m_AuInfo;                   // storage for slice header & segment data
	//TComInputBitstream	m_SliceSegData;             // storage for slice segment data

	//Bool                m_bShowLogsFlag;

	//TDecCavlc           m_CavlcDecoder;
	//SEIReader           m_SeiReader;

	//TComInputBitstream  m_InputBitstream;
	//CSEIMessages        m_SEIs;                     ///< List of SEI messages that have been received before the first slice and between slices

	//Bool                m_bFirstSliceInBitstream;   // ��Ʈ��Ʈ�� �߰��� EOS�� ������ ���� �ִ� ��ü ��Ʈ��Ʈ��

protected:

	bool ParseObuHeader(uint8_t obu_header_byte, ObuHeader *obu_header);
	bool ParseObuExtensionHeader(uint8_t ext_header_byte, ObuHeader *obu_header);
	//void PrintObuHeader(const ObuHeader *header);
	//bool DumpObu(const uint8_t *data, int length, int *obu_overhead_bytes);
	bool ValidObuType(int obu_type);


};
#pragma once
