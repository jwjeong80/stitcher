// HEVCFile.h
// Created by yonghwan@keti.re.kr, 2011.07.01

//#pragma once

//#include "LargeFile.h"
#include "av1/decoder/obu.h"
#include "aom/aom_codec.h"
#include "aom_dsp/bitreader_buffer.h"
#ifndef MAX_OBU_NUM
#define     MAX_OBU_NUM	(440 + 8) // = MAX_NUM_TILES + VPS/SPS/PPS...
struct OBU
{
	uint32_t    uiNumOfOBU;                    /* [in] */
	uint8_t*    pEachOBU[MAX_OBU_NUM];            /* [in]; only pointer (continuous memory) */
	uint32_t    uiEachOBUSize[MAX_OBU_NUM];       /* [in] */
};
#endif // MAX_NALU_NUM

enum eReturnStatusAV1
{
	RET_FALSE_AV1 = 0,
	RET_TRUE_AV1 = 1,
	RET_EOF_AV1 = 2
};

struct AvxInputContext {
	const char *filename;
	FILE *file;
	int64_t length;
	//struct FileTypeDetectionBuffer detect;
	//enum VideoFileType file_type;
	uint32_t width;
	uint32_t height;
	//struct AvxRational pixel_aspect_ratio;
	//aom_img_fmt_t fmt;
	//aom_bit_depth_t bit_depth;
	int only_i420;
	uint32_t fourcc;
//	struct AvxRational framerate;
//#if CONFIG_AV1_ENCODER
//	y4m_input y4m;
//#endif
};

struct ObuDecInputContext {
	struct AvxInputContext avx_ctx;
	uint8_t *buffer;
	size_t buffer_capacity;
	size_t bytes_buffered;
	int is_annexb;
};

class CAV1File
{
public:
	CAV1File(void);
	~CAV1File(void);

	bool		Reset();

	bool OpenOBU(const char* rcFilename);


	//int file_is_obu(struct ObuDecInputContext *obu_ctx);
	int file_is_obu(void);
	static aom_codec_err_t read_obu_header(struct aom_read_bit_buffer *rb,
		int is_annexb, ObuHeader *header);
	int obudec_read_temporal_unit( uint8_t **buffer, size_t *bytes_read, size_t *buffer_size);
	int obudec_read_leb128(FILE *f, uint8_t *value_buffer,
		size_t *value_length, uint64_t *value);
	int obudec_read_one_obu(FILE *f, uint8_t **obu_buffer,
		size_t obu_bytes_buffered,
		size_t *obu_buffer_capacity, size_t *obu_length,
		ObuHeader *obu_header, int is_annexb);
	int obudec_read_obu_payload(FILE *f, size_t payload_length,
		uint8_t *obu_data, size_t *bytes_read);
	int obudec_read_obu_header_and_size(FILE *f, size_t buffer_capacity,
		int is_annexb, uint8_t *buffer,
		size_t *bytes_read,
		size_t *payload_length,
		ObuHeader *obu_header);
	int obudec_read_obu_header(FILE *f, size_t buffer_capacity,
		int is_annexb, uint8_t *obu_data,
		ObuHeader *obu_header, size_t *bytes_read);

	aom_codec_err_t aom_read_obu_header(uint8_t *buffer, size_t buffer_length,
		size_t *consumed, ObuHeader *header,
		int is_annexb);

	//uint32_t	ExtractPacket3(/*[in]*/uint8_t *pInputBuf, /*[in]*/uint32_t dwBufSize, /*[out]*/uint8_t** pBufStartPos, /*[out]*/uint32_t& uiOutBufSize);
	//eReturnStatus	ExtractAU(/*[in]*/uint8_t* pBuf, /*[in]*/uint32_t dwBufSize, /*[out]*/AccessUnit* pAU);
	eReturnStatusAV1 ExtractOBU(/*[in]*/uint8_t* pBuf, /*[in]*/uint32_t dwBufSize, /*[out]*/OBU* pOBU);

	size_t AomRbBytesRead(const struct aom_read_bit_buffer *rb);
	static int AomRbReadBit(struct aom_read_bit_buffer *rb);
	static int AomRbReadLiteral(struct aom_read_bit_buffer *rb, int bits);

	//int64_t		getPosition();
	//bool		setPosition(int64_t iPos);

private:

	//const uint8_t*	FindStartCode2(const uint8_t* pStart, const uint8_t* pEnd,  /*[out]*/uint32_t& uiStartCodeSize);
	struct ObuDecInputContext m_OBUCtx;
	struct AvxInputContext    m_AomInputCtx;
};
