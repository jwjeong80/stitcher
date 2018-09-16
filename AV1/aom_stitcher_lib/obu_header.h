#pragma once
/*!\brief OBU types. */
typedef enum ATTRIBUTE_PACKED {
	OBU_SEQUENCE_HEADER = 1,
	OBU_TEMPORAL_DELIMITER = 2,
	OBU_FRAME_HEADER = 3,
	OBU_TILE_GROUP = 4,
	OBU_METADATA = 5,
	OBU_FRAME = 6,
	OBU_REDUNDANT_FRAME_HEADER = 7,
	OBU_TILE_LIST = 8,
	OBU_PADDING = 15,
} OBU_TYPE;

/*!\brief OBU metadata types. */
typedef enum {
	OBU_METADATA_TYPE_AOM_RESERVED_0 = 0,
	OBU_METADATA_TYPE_HDR_CLL = 1,
	OBU_METADATA_TYPE_HDR_MDCV = 2,
	OBU_METADATA_TYPE_SCALABILITY = 3,
	OBU_METADATA_TYPE_ITUT_T35 = 4,
	OBU_METADATA_TYPE_TIMECODE = 5,
} OBU_METADATA_TYPE;

typedef struct {
	size_t size;  // Size (1 or 2 bytes) of the OBU header (including the
				  // optional OBU extension header) in the bitstream.
	OBU_TYPE type;
	int has_size_field;
	int has_extension;
	// The following fields come from the OBU extension header and therefore are
	// only used if has_extension is true.
	int temporal_layer_id;
	int spatial_layer_id;
} ObuHeader;