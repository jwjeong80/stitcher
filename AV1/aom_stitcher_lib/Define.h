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
/*!\brief Decorator indicating that given struct/union/enum is packed */
#ifndef ATTRIBUTE_PACKED
#if defined(__GNUC__) && __GNUC__
#define ATTRIBUTE_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define ATTRIBUTE_PACKED
#else
#define ATTRIBUTE_PACKED
#endif
#endif /* ATTRIBUTE_PACKED */

// definitions
#define MAX_BIT_SIZE    (1024*1024)
#define MAX_STREAMS     (440) /* 20x22 */

#define DISABLE_LINES       0
#define STITCHER_FEATURES   1


#define MAX_NUM_TEMPORAL_LAYERS 8
#define MAX_NUM_SPATIAL_LAYERS 4
/* clang-format off */
// clang-format seems to think this is a pointer dereference and not a
// multiplication.
#define MAX_NUM_OPERATING_POINTS \
  MAX_NUM_TEMPORAL_LAYERS * MAX_NUM_SPATIAL_LAYERS
#define PROFILE_BITS 3


#define LEVEL_MAJOR_BITS 3
#define LEVEL_MINOR_BITS 2
#define LEVEL_BITS (LEVEL_MAJOR_BITS + LEVEL_MINOR_BITS)

#define LEVEL_MAJOR_MIN 2
#define LEVEL_MAJOR_MAX ((1 << LEVEL_MAJOR_BITS) - 1 + LEVEL_MAJOR_MIN)
#define LEVEL_MINOR_MIN 0
#define LEVEL_MINOR_MAX ((1 << LEVEL_MINOR_BITS) - 1)

#define OP_POINTS_CNT_MINUS_1_BITS 5
#define OP_POINTS_IDC_BITS 12

#define NUM_REF_FRAMES 8

#define PRIMARY_REF_BITS 3
#define PRIMARY_REF_NONE 7

#define SUPERRES_DENOM_BITS 3
#define SUPERRES_DENOM_MIN  9
#define SUPERRES_NUM 8

#define MAX_TILE_ROWS 64
#define MAX_TILE_COLS 64
#define MAX_TILE_WIDTH (4096)        // Max Tile width in pixels
#define MAX_TILE_AREA (4096 * 2304)  // Maximum tile area in pixels

#define AOMMIN(x, y) (((x) < (y)) ? (x) : (y))
#define AOMMAX(x, y) (((x) > (y)) ? (x) : (y))

#define MAX_SEGMENTS 8
#define TOTAL_REFS_PER_FRAME 8

#define RESTORATION_TILESIZE_MAX 256

typedef enum ATTRIBUTE_PACKED {
	IDENTITY = 0,      // identity transformation, 0-parameter
	TRANSLATION = 1,   // translational motion 2-parameter
	ROTZOOM = 2,       // simplified affine with rotation + zoom only, 4-parameter
	AFFINE = 3,        // affine, 6-parameter
	TRANS_TYPES,
} TransformationType;

typedef enum ATTRIBUTE_PACKED {
	RESTORE_NONE,
	RESTORE_WIENER,
	RESTORE_SGRPROJ,
	RESTORE_SWITCHABLE,
	RESTORE_SWITCHABLE_TYPES = RESTORE_SWITCHABLE,
	RESTORE_TYPES = 4,
} RestorationType;

const RestorationType Remap_Lr_Type[4] = {
	RESTORE_NONE, RESTORE_SWITCHABLE, RESTORE_WIENER, RESTORE_SGRPROJ
};

typedef enum ATTRIBUTE_PACKED {
	ONLY_4X4,         // use only 4x4 transform
	TX_MODE_LARGEST,  // transform size is the largest possible for pu size
	TX_MODE_SELECT,   // transform specified for each block
	TX_MODES,
} TX_MODE;

typedef enum ATTRIBUTE_PACKED {
	KEY_FRAME = 0,
	INTER_FRAME = 1,
	INTRA_ONLY_FRAME = 2,  // replaces intra-only
	SWITCH_FRAME = 3,
	FRAME_TYPES,
} FRAME_TYPE;

typedef enum ATTRIBUTE_PACKED {
	EIGHTTAP_REGULAR,
	EIGHTTAP_SMOOTH,
	MULTITAP_SHARP,
	BILINEAR,
	INTERP_FILTERS_ALL,
	SWITCHABLE_FILTERS = BILINEAR,
	SWITCHABLE = SWITCHABLE_FILTERS + 1, /* the last switchable one */
	EXTRA_FILTERS = INTERP_FILTERS_ALL - SWITCHABLE_FILTERS,
} InterpFilter;