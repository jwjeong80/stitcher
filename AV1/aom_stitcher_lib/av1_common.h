#include <stdint.h>

int aom_uleb_decode(const uint8_t *buffer, size_t available, uint64_t *value, size_t *length);
size_t aom_uleb_size_in_bytes(uint64_t value);
int aom_uleb_encode(uint64_t value, size_t available, uint8_t *coded_value, size_t *coded_size);



/*!\brief Algorithm return codes */
typedef enum {
	/*!\brief Operation completed without error */
	AOM_CODEC_OK,

	/*!\brief Unspecified error */
	AOM_CODEC_ERROR,

	/*!\brief Memory operation failed */
	AOM_CODEC_MEM_ERROR,

	/*!\brief ABI version mismatch */
	AOM_CODEC_ABI_MISMATCH,

	/*!\brief Algorithm does not have required capability */
	AOM_CODEC_INCAPABLE,

	/*!\brief The given bitstream is not supported.
	 *
	 * The bitstream was unable to be parsed at the highest level. The decoder
	 * is unable to proceed. This error \ref SHOULD be treated as fatal to the
	 * stream. */
	AOM_CODEC_UNSUP_BITSTREAM,

	/*!\brief Encoded bitstream uses an unsupported feature
	 *
	 * The decoder does not implement a feature required by the encoder. This
	 * return code should only be used for features that prevent future
	 * pictures from being properly decoded. This error \ref MAY be treated as
	 * fatal to the stream or \ref MAY be treated as fatal to the current GOP.
	 */
	AOM_CODEC_UNSUP_FEATURE,

	/*!\brief The coded data for this stream is corrupt or incomplete
	 *
	 * There was a problem decoding the current frame.  This return code
	 * should only be used for failures that prevent future pictures from
	 * being properly decoded. This error \ref MAY be treated as fatal to the
	 * stream or \ref MAY be treated as fatal to the current GOP. If decoding
	 * is continued for the current GOP, artifacts may be present.
	 */
	AOM_CODEC_CORRUPT_FRAME,

	/*!\brief An application-supplied parameter is not valid.
	 *
	 */
	AOM_CODEC_INVALID_PARAM,

	/*!\brief An iterator reached the end of list.
	 *
	 */
	AOM_CODEC_LIST_END

} aom_codec_err_t;