/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: HEVCParser.cpp
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#pragma once

// definitions
#define MAX_BIT_SIZE    (1024*1024)
#define MAX_STREAMS     (440) /* 20x22 */
#define MAX_STREAMS_PS	(MAX_STREAMS + 8/*VPS/SPS/PPS...*/)

#define DISABLE_LINES       0
#define STITCHER_FEATURES   1
