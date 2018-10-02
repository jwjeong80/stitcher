#pragma once
/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: BStrStitcher.h
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#pragma once

#include "AV1File.h"

#include "OBUParser.h"
#include "OBUWriter.h"

class CAV1BStrStitcher
{
public:
	CAV1BStrStitcher(void);
	~CAV1BStrStitcher(void);

	int         Create(uint32_t uiNumTileRows, uint32_t uiNumTileCols, bool bAnnexB);
	int			StitchSingleOBU(const OBU *pInpOBUs, uint32_t uiStitchFlags, OBU *pOutOBUs);
	void        Destroy();

	bool		m_bAnnexBFlag;
	bool		m_bShowLogsFlag;

	uint32_t	m_uiNumParsers;
	OBU	m_OBU;
	
private:
	COBUParser * m_pOBUParser[MAX_STREAMS];
	COBUWriter  m_OBUWriter;

	uint32_t    *m_uiWidths;
	uint32_t    *m_uiHeights;

	FrameSize_t m_tileSizes[MAX_STREAMS];

protected:

};
