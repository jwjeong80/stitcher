#include "FrameHeader.h"

uint32_t CFrameHeader::FhParserUncompressedHeader(CBitReader *rb) {
	//av1_common *const cm = &pbi->common;
	//const sequenceheader *const seq_params = &cm->seq_params;
	//macroblockd *const xd = &pbi->mb;
	//bufferpool *const pool = cm->buffer_pool;
	//refcntbuffer *const frame_bufs = pool->frame_bufs;

	
	//if (!pbi->sequence_header_ready) {
	//	aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//		"No sequence header");
	//}

	//cm->last_frame_type = cm->frame_type;
	//cm->last_intra_only = cm->intra_only;

	//// NOTE: By default all coded frames to be used as a reference
	//cm->is_reference_frame = 1;

	//if (seq_params->reduced_still_picture_hdr) {
	//	cm->show_existing_frame = 0;
	//	cm->show_frame = 1;
	//	cm->frame_type = KEY_FRAME; //blockd.h
	//	cm->error_resilient_mode = 1;
	//}
	//else {
	//	cm->show_existing_frame = aom_rb_read_bit(rb);
	//	cm->reset_decoder_state = 0;

	//	if (cm->show_existing_frame) {
	//		// Show an existing frame directly.
	//		const int existing_frame_idx = aom_rb_read_literal(rb, 3);
	//		const int frame_to_show = cm->ref_frame_map[existing_frame_idx];
	//		if (seq_params->decoder_model_info_present_flag &&
	//			cm->timing_info.equal_picture_interval == 0) {
	//			av1_read_temporal_point_info(cm, rb); //decodeframe.c
	//		}
	//		if (seq_params->frame_id_numbers_present_flag) {
	//			int frame_id_length = seq_params->frame_id_length;
	//			int display_frame_id = aom_rb_read_literal(rb, frame_id_length);
	//			/* Compare display_frame_id with ref_frame_id and check valid for
	//			* referencing */
	//			if (display_frame_id != cm->ref_frame_id[existing_frame_idx] ||
	//				cm->valid_for_referencing[existing_frame_idx] == 0)
	//				aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//					"Reference buffer frame ID mismatch");
	//		}
	//		lock_buffer_pool(pool); //onyxc_int.h
	//		if (frame_to_show < 0 || frame_bufs[frame_to_show].ref_count < 1) {
	//			unlock_buffer_pool(pool); //onyxc_int.h
	//			aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//				"Buffer %d does not contain a decoded frame",
	//				frame_to_show);
	//		}
	//		ref_cnt_fb(frame_bufs, &cm->new_fb_idx, frame_to_show); //onyxc_int.h
	//		cm->reset_decoder_state =
	//			frame_bufs[frame_to_show].frame_type == KEY_FRAME;
	//		unlock_buffer_pool(pool);

	//		cm->lf.filter_level[0] = 0;
	//		cm->lf.filter_level[1] = 0;
	//		cm->show_frame = 1;

	//		if (!frame_bufs[frame_to_show].showable_frame) {
	//			aom_merge_corrupted_flag(&xd->corrupted, 1); //aom_codec.c
	//		}
	//		if (cm->reset_decoder_state) frame_bufs[frame_to_show].showable_frame = 0;

	//		cm->film_grain_params = frame_bufs[frame_to_show].film_grain_params;

	//		if (cm->reset_decoder_state) {
	//			show_existing_frame_reset(pbi, existing_frame_idx); //decodeframe.c
	//		}
	//		else {
	//			pbi->refresh_frame_flags = 0;
	//		}

	//		return 0;
	//	}

	//	cm->frame_type = (FRAME_TYPE)aom_rb_read_literal(rb, 2);  // 2 bits
	//	cm->show_frame = aom_rb_read_bit(rb);
	//	if (seq_params->still_picture &&
	//		(cm->frame_type != KEY_FRAME || !cm->show_frame)) {
	//		aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//			"Still pictures must be coded as shown keyframes");
	//	}
	//	cm->showable_frame = cm->frame_type != KEY_FRAME;
	//	if (cm->show_frame) {
	//		if (seq_params->decoder_model_info_present_flag &&
	//			cm->timing_info.equal_picture_interval == 0)
	//			av1_read_temporal_point_info(cm, rb); //decodeframe.c
	//	}
	//	else {
	//		// See if this frame can be used as show_existing_frame in future
	//		cm->showable_frame = aom_rb_read_bit(rb);
	//	}
	//	cm->cur_frame->showable_frame = cm->showable_frame;
	//	cm->intra_only = cm->frame_type == INTRA_ONLY_FRAME;
	//	cm->error_resilient_mode =
	//		frame_is_sframe(cm) || (cm->frame_type == KEY_FRAME && cm->show_frame)
	//		? 1
	//		: aom_rb_read_bit(rb);
	//}

	//cm->disable_cdf_update = aom_rb_read_bit(rb);
	//if (seq_params->force_screen_content_tools == 2) {
	//	cm->allow_screen_content_tools = aom_rb_read_bit(rb);
	//}
	//else {
	//	cm->allow_screen_content_tools = seq_params->force_screen_content_tools;
	//}

	//if (cm->allow_screen_content_tools) {
	//	if (seq_params->force_integer_mv == 2) {
	//		cm->cur_frame_force_integer_mv = aom_rb_read_bit(rb);
	//	}
	//	else {
	//		cm->cur_frame_force_integer_mv = seq_params->force_integer_mv;
	//	}
	//}
	//else {
	//	cm->cur_frame_force_integer_mv = 0;
	//}

	//cm->frame_refs_short_signaling = 0;
	//int frame_size_override_flag = 0;
	//cm->allow_intrabc = 0;
	//cm->primary_ref_frame = PRIMARY_REF_NONE;

	//if (!seq_params->reduced_still_picture_hdr) {
	//	if (seq_params->frame_id_numbers_present_flag) {
	//		int frame_id_length = seq_params->frame_id_length;
	//		int diff_len = seq_params->delta_frame_id_length;
	//		int prev_frame_id = 0;
	//		int have_prev_frame_id = !pbi->decoding_first_frame &&
	//			!(cm->frame_type == KEY_FRAME && cm->show_frame);
	//		if (have_prev_frame_id) {
	//			prev_frame_id = cm->current_frame_id;
	//		}
	//		cm->current_frame_id = aom_rb_read_literal(rb, frame_id_length);

	//		if (have_prev_frame_id) {
	//			int diff_frame_id;
	//			if (cm->current_frame_id > prev_frame_id) {
	//				diff_frame_id = cm->current_frame_id - prev_frame_id;
	//			}
	//			else {
	//				diff_frame_id =
	//					(1 << frame_id_length) + cm->current_frame_id - prev_frame_id;
	//			}
	//			/* Check current_frame_id for conformance */
	//			if (prev_frame_id == cm->current_frame_id ||
	//				diff_frame_id >= (1 << (frame_id_length - 1))) {
	//				aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//					"Invalid value of current_frame_id");
	//			}
	//		}
	//		/* Check if some frames need to be marked as not valid for referencing */
	//		for (int i = 0; i < REF_FRAMES; i++) {
	//			if (cm->frame_type == KEY_FRAME && cm->show_frame) {
	//				cm->valid_for_referencing[i] = 0;
	//			}
	//			else if (cm->current_frame_id - (1 << diff_len) > 0) {
	//				if (cm->ref_frame_id[i] > cm->current_frame_id ||
	//					cm->ref_frame_id[i] < cm->current_frame_id - (1 << diff_len))
	//					cm->valid_for_referencing[i] = 0;
	//			}
	//			else {
	//				if (cm->ref_frame_id[i] > cm->current_frame_id &&
	//					cm->ref_frame_id[i] < (1 << frame_id_length) +
	//					cm->current_frame_id - (1 << diff_len))
	//					cm->valid_for_referencing[i] = 0;
	//			}
	//		}
	//	}

	//	frame_size_override_flag = frame_is_sframe(cm) ? 1 : aom_rb_read_bit(rb); //onyxc_int.h 

	//	cm->frame_offset =
	//		aom_rb_read_literal(rb, seq_params->order_hint_bits_minus_1 + 1);
	//	cm->current_video_frame = cm->frame_offset;

	//	if (!cm->error_resilient_mode && !frame_is_intra_only(cm)) {
	//		cm->primary_ref_frame = aom_rb_read_literal(rb, PRIMARY_REF_BITS);
	//	}
	//}

	//if (seq_params->decoder_model_info_present_flag) {
	//	cm->buffer_removal_time_present = aom_rb_read_bit(rb);
	//	if (cm->buffer_removal_time_present) {
	//		for (int op_num = 0;
	//			op_num < seq_params->operating_points_cnt_minus_1 + 1; op_num++) {
	//			if (cm->op_params[op_num].decoder_model_param_present_flag) {
	//				if ((((seq_params->operating_point_idc[op_num] >>
	//					cm->temporal_layer_id) &
	//					0x1) &&
	//					((seq_params->operating_point_idc[op_num] >>
	//					(cm->spatial_layer_id + 8)) &
	//						0x1)) ||
	//					seq_params->operating_point_idc[op_num] == 0) {
	//					cm->op_frame_timing[op_num].buffer_removal_time =
	//						aom_rb_read_unsigned_literal(
	//							rb, cm->buffer_model.buffer_removal_time_length);
	//				}
	//				else {
	//					cm->op_frame_timing[op_num].buffer_removal_time = 0;
	//				}
	//			}
	//			else {
	//				cm->op_frame_timing[op_num].buffer_removal_time = 0;
	//			}
	//		}
	//	}
	//}
	//if (cm->frame_type == KEY_FRAME) {
	//	if (!cm->show_frame)  // unshown keyframe (forward keyframe)
	//		pbi->refresh_frame_flags = aom_rb_read_literal(rb, REF_FRAMES);
	//	else  // shown keyframe
	//		pbi->refresh_frame_flags = (1 << REF_FRAMES) - 1;

	//	for (int i = 0; i < INTER_REFS_PER_FRAME; ++i) {
	//		cm->frame_refs[i].idx = INVALID_IDX;
	//		cm->frame_refs[i].buf = NULL;
	//	}
	//	if (pbi->need_resync) {
	//		memset(&cm->ref_frame_map, -1, sizeof(cm->ref_frame_map));
	//		pbi->need_resync = 0;
	//	}
	//}
	//else {
	//	if (cm->intra_only) {
	//		pbi->refresh_frame_flags = aom_rb_read_literal(rb, REF_FRAMES);
	//		if (pbi->refresh_frame_flags == 0xFF) {
	//			aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//				"Intra only frames cannot have refresh flags 0xFF");
	//		}
	//		if (pbi->need_resync) {
	//			memset(&cm->ref_frame_map, -1, sizeof(cm->ref_frame_map));
	//			pbi->need_resync = 0;
	//		}
	//	}
	//	else if (pbi->need_resync != 1) { /* Skip if need resync */
	//		pbi->refresh_frame_flags =
	//			frame_is_sframe(cm) ? 0xFF : aom_rb_read_literal(rb, REF_FRAMES);
	//		if (!pbi->refresh_frame_flags) {
	//			// NOTE: "pbi->refresh_frame_flags == 0" indicates that the coded frame
	//			//       will not be used as a reference
	//			cm->is_reference_frame = 0;
	//		}
	//	}
	//}

	//if (!frame_is_intra_only(cm) || pbi->refresh_frame_flags != 0xFF) {
	//	// Read all ref frame order hints if error_resilient_mode == 1
	//	if (cm->error_resilient_mode && seq_params->enable_order_hint) {
	//		for (int ref_idx = 0; ref_idx < REF_FRAMES; ref_idx++) {
	//			// Read order hint from bit stream
	//			unsigned int frame_offset =
	//				aom_rb_read_literal(rb, seq_params->order_hint_bits_minus_1 + 1);
	//			// Get buffer index
	//			int buf_idx = cm->ref_frame_map[ref_idx];
	//			assert(buf_idx < FRAME_BUFFERS);
	//			if (buf_idx == -1 ||
	//				frame_offset != frame_bufs[buf_idx].cur_frame_offset) {
	//				if (buf_idx >= 0) {
	//					lock_buffer_pool(pool);
	//					decrease_ref_count(buf_idx, frame_bufs, pool); //decoder.h
	//					unlock_buffer_pool(pool);
	//				}
	//				// If no corresponding buffer exists, allocate a new buffer with all
	//				// pixels set to neutral grey.
	//				buf_idx = get_free_fb(cm);
	//				if (buf_idx == INVALID_IDX) {
	//					aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR,
	//						"Unable to find free frame buffer");
	//				}
	//				lock_buffer_pool(pool);
	//				if (aom_realloc_frame_buffer( //yv12config.c
	//					&frame_bufs[buf_idx].buf, seq_params->max_frame_width,
	//					seq_params->max_frame_height, seq_params->subsampling_x,
	//					seq_params->subsampling_y, seq_params->use_highbitdepth,
	//					AOM_BORDER_IN_PIXELS, cm->byte_alignment,
	//					&pool->frame_bufs[buf_idx].raw_frame_buffer, pool->get_fb_cb,
	//					pool->cb_priv)) {
	//					unlock_buffer_pool(pool);
	//					aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR,
	//						"Failed to allocate frame buffer");
	//				}
	//				unlock_buffer_pool(pool);
	//				set_planes_to_neutral_grey(seq_params, &frame_bufs[buf_idx].buf, 0); //decodeframe.c

	//				cm->ref_frame_map[ref_idx] = buf_idx;
	//				frame_bufs[buf_idx].cur_frame_offset = frame_offset;
	//			}
	//		}
	//	}
	//}

	//if (cm->frame_type == KEY_FRAME) {
	//	setup_frame_size(cm, frame_size_override_flag, rb); //decodeframe.c

	//	if (cm->allow_screen_content_tools && !av1_superres_scaled(cm)) //resize.h
	//		cm->allow_intrabc = aom_rb_read_bit(rb);
	//	cm->allow_ref_frame_mvs = 0;
	//	cm->prev_frame = NULL;
	//}
	//else {
	//	cm->allow_ref_frame_mvs = 0;

	//	if (cm->intra_only) {
	//		cm->cur_frame->film_grain_params_present =
	//			seq_params->film_grain_params_present;
	//		setup_frame_size(cm, frame_size_override_flag, rb); //decodeframe.c
	//		if (cm->allow_screen_content_tools && !av1_superres_scaled(cm)) //resize.h
	//			cm->allow_intrabc = aom_rb_read_bit(rb);

	//	}
	//	else if (pbi->need_resync != 1) { /* Skip if need resync */

	//									  // Frame refs short signaling is off when error resilient mode is on.
	//		if (seq_params->enable_order_hint)
	//			cm->frame_refs_short_signaling = aom_rb_read_bit(rb);

	//		if (cm->frame_refs_short_signaling) {
	//			// == LAST_FRAME ==
	//			const int lst_ref = aom_rb_read_literal(rb, REF_FRAMES_LOG2);
	//			const int lst_idx = cm->ref_frame_map[lst_ref];

	//			// == GOLDEN_FRAME ==
	//			const int gld_ref = aom_rb_read_literal(rb, REF_FRAMES_LOG2);
	//			const int gld_idx = cm->ref_frame_map[gld_ref];

	//			// Most of the time, streams start with a keyframe. In that case,
	//			// ref_frame_map will have been filled in at that point and will not
	//			// contain any -1's. However, streams are explicitly allowed to start
	//			// with an intra-only frame, so long as they don't then signal a
	//			// reference to a slot that hasn't been set yet. That's what we are
	//			// checking here.
	//			if (lst_idx == -1)
	//				aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//					"Inter frame requests nonexistent reference");
	//			if (gld_idx == -1)
	//				aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//					"Inter frame requests nonexistent reference");

	//			av1_set_frame_refs(cm, lst_ref, gld_ref); //mvref_common.c
	//		}

	//		for (int i = 0; i < INTER_REFS_PER_FRAME; ++i) {
	//			int ref = 0;
	//			if (!cm->frame_refs_short_signaling) {
	//				ref = aom_rb_read_literal(rb, REF_FRAMES_LOG2);
	//				const int idx = cm->ref_frame_map[ref];

	//				// Most of the time, streams start with a keyframe. In that case,
	//				// ref_frame_map will have been filled in at that point and will not
	//				// contain any -1's. However, streams are explicitly allowed to start
	//				// with an intra-only frame, so long as they don't then signal a
	//				// reference to a slot that hasn't been set yet. That's what we are
	//				// checking here.
	//				if (idx == -1)
	//					aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//						"Inter frame requests nonexistent reference");

	//				RefBuffer *const ref_frame = &cm->frame_refs[i]; //RedBuffer in blockd.h
	//				ref_frame->idx = idx;
	//				ref_frame->buf = &frame_bufs[idx].buf;
	//				ref_frame->map_idx = ref;
	//			}
	//			else {
	//				ref = cm->frame_refs[i].map_idx;
	//			}

	//			cm->ref_frame_sign_bias[LAST_FRAME + i] = 0;

	//			if (seq_params->frame_id_numbers_present_flag) {
	//				int frame_id_length = seq_params->frame_id_length;
	//				int diff_len = seq_params->delta_frame_id_length;
	//				int delta_frame_id_minus_1 = aom_rb_read_literal(rb, diff_len);
	//				int ref_frame_id =
	//					((cm->current_frame_id - (delta_frame_id_minus_1 + 1) +
	//					(1 << frame_id_length)) %
	//						(1 << frame_id_length));
	//				// Compare values derived from delta_frame_id_minus_1 and
	//				// refresh_frame_flags. Also, check valid for referencing
	//				if (ref_frame_id != cm->ref_frame_id[ref] ||
	//					cm->valid_for_referencing[ref] == 0)
	//					aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//						"Reference buffer frame ID mismatch");
	//			}
	//		}

	//		if (!cm->error_resilient_mode && frame_size_override_flag) {
	//			setup_frame_size_with_refs(cm, rb); //decodeframe.c
	//		}
	//		else {
	//			setup_frame_size(cm, frame_size_override_flag, rb);  //decodeframe.c
	//		}

	//		if (cm->cur_frame_force_integer_mv) {
	//			cm->allow_high_precision_mv = 0;
	//		}
	//		else {
	//			cm->allow_high_precision_mv = aom_rb_read_bit(rb);
	//		}
	//		cm->interp_filter = read_frame_interp_filter(rb);
	//		cm->switchable_motion_mode = aom_rb_read_bit(rb);
	//	}

	//	cm->prev_frame = get_prev_frame(cm); //onyxc_int.h
	//	if (cm->primary_ref_frame != PRIMARY_REF_NONE &&
	//		cm->frame_refs[cm->primary_ref_frame].idx < 0) {
	//		aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//			"Reference frame containing this frame's initial "
	//			"frame context is unavailable.");
	//	}

	//	if (!cm->intra_only && pbi->need_resync != 1) {
	//		if (frame_might_allow_ref_frame_mvs(cm)) //onyxc_int.h
	//			cm->allow_ref_frame_mvs = aom_rb_read_bit(rb);
	//		else
	//			cm->allow_ref_frame_mvs = 0;

	//		for (int i = 0; i < INTER_REFS_PER_FRAME; ++i) {
	//			RefBuffer *const ref_buf = &cm->frame_refs[i];
	//			av1_setup_scale_factors_for_frame(  //scale.c
	//				&ref_buf->sf, ref_buf->buf->y_crop_width,
	//				ref_buf->buf->y_crop_height, cm->width, cm->height);
	//			if ((!av1_is_valid_scale(&ref_buf->sf))) //scale.h
	//				aom_internal_error(&cm->error, AOM_CODEC_UNSUP_BITSTREAM,
	//					"Reference frame has invalid dimensions");
	//		}
	//	}
	//}

	//av1_setup_frame_buf_refs(cm); //mvref_common.c

	//av1_setup_frame_sign_bias(cm); //mvref_common.c

	//cm->cur_frame->intra_only = cm->frame_type == KEY_FRAME || cm->intra_only;
	//cm->cur_frame->frame_type = cm->frame_type;

	//if (seq_params->frame_id_numbers_present_flag) {
	//	/* If bitmask is set, update reference frame id values and
	//	mark frames as valid for reference */
	//	int refresh_frame_flags = pbi->refresh_frame_flags;
	//	for (int i = 0; i < REF_FRAMES; i++) {
	//		if ((refresh_frame_flags >> i) & 1) {
	//			cm->ref_frame_id[i] = cm->current_frame_id;
	//			cm->valid_for_referencing[i] = 1;
	//		}
	//	}
	//}

	//const int might_bwd_adapt =
	//	!(seq_params->reduced_still_picture_hdr) && !(cm->disable_cdf_update);
	//if (might_bwd_adapt) {
	//	cm->refresh_frame_context = aom_rb_read_bit(rb)
	//		? REFRESH_FRAME_CONTEXT_DISABLED
	//		: REFRESH_FRAME_CONTEXT_BACKWARD;
	//}
	//else {
	//	cm->refresh_frame_context = REFRESH_FRAME_CONTEXT_DISABLED;
	//}

	//get_frame_new_buffer(cm)->bit_depth = seq_params->bit_depth;
	//get_frame_new_buffer(cm)->color_primaries = seq_params->color_primaries;
	//get_frame_new_buffer(cm)->transfer_characteristics =
	//	seq_params->transfer_characteristics;
	//get_frame_new_buffer(cm)->matrix_coefficients =
	//	seq_params->matrix_coefficients;
	//get_frame_new_buffer(cm)->monochrome = seq_params->monochrome;
	//get_frame_new_buffer(cm)->chroma_sample_position =
	//	seq_params->chroma_sample_position;
	//get_frame_new_buffer(cm)->color_range = seq_params->color_range;
	//get_frame_new_buffer(cm)->render_width = cm->render_width;
	//get_frame_new_buffer(cm)->render_height = cm->render_height;

	//if (pbi->need_resync) {
	//	aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//		"Keyframe / intra-only frame required to reset decoder"
	//		" state");
	//}

	//// Generate next_ref_frame_map.
	//lock_buffer_pool(pool);
	//int ref_index = 0;
	//for (int mask = pbi->refresh_frame_flags; mask; mask >>= 1) {
	//	if (mask & 1) {
	//		cm->next_ref_frame_map[ref_index] = cm->new_fb_idx;
	//		++frame_bufs[cm->new_fb_idx].ref_count;
	//	}
	//	else {
	//		cm->next_ref_frame_map[ref_index] = cm->ref_frame_map[ref_index];
	//	}
	//	// Current thread holds the reference frame.
	//	if (cm->ref_frame_map[ref_index] >= 0)
	//		++frame_bufs[cm->ref_frame_map[ref_index]].ref_count;
	//	++ref_index;
	//}

	//for (; ref_index < REF_FRAMES; ++ref_index) {
	//	cm->next_ref_frame_map[ref_index] = cm->ref_frame_map[ref_index];

	//	// Current thread holds the reference frame.
	//	if (cm->ref_frame_map[ref_index] >= 0)
	//		++frame_bufs[cm->ref_frame_map[ref_index]].ref_count;
	//}
	//unlock_buffer_pool(pool);
	//pbi->hold_ref_buf = 1;

	//if (cm->allow_intrabc) {
	//	// Set parameters corresponding to no filtering.
	//	struct loopfilter *lf = &cm->lf;
	//	lf->filter_level[0] = 0;
	//	lf->filter_level[1] = 0;
	//	cm->cdef_bits = 0;
	//	cm->cdef_strengths[0] = 0;
	//	cm->nb_cdef_strengths = 1;
	//	cm->cdef_uv_strengths[0] = 0;
	//	cm->rst_info[0].frame_restoration_type = RESTORE_NONE;
	//	cm->rst_info[1].frame_restoration_type = RESTORE_NONE;
	//	cm->rst_info[2].frame_restoration_type = RESTORE_NONE;
	//}

	//read_tile_info(pbi, rb); //decodeframe.c
	//setup_quantization(cm, rb); //decodeframe.c
	//xd->bd = (int)seq_params->bit_depth;

	//if (cm->num_allocated_above_context_planes < av1_num_planes(cm) ||
	//	cm->num_allocated_above_context_mi_col < cm->mi_cols ||
	//	cm->num_allocated_above_contexts < cm->tile_rows) {
	//	av1_free_above_context_buffers(cm, cm->num_allocated_above_contexts);  //alloccommon.c
	//	if (av1_alloc_above_context_buffers(cm, cm->tile_rows))
	//		aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR,
	//			"Failed to allocate context buffers");
	//}

	//if (cm->primary_ref_frame == PRIMARY_REF_NONE) {
	//	av1_setup_past_independence(cm); //entropymode.c
	//}

	//setup_segmentation(cm, rb); //decodeframe.c

	//cm->delta_q_res = 1;
	//cm->delta_lf_res = 1;
	//cm->delta_lf_present_flag = 0;
	//cm->delta_lf_multi = 0;
	//cm->delta_q_present_flag = cm->base_qindex > 0 ? aom_rb_read_bit(rb) : 0;
	//if (cm->delta_q_present_flag) {
	//	xd->current_qindex = cm->base_qindex;
	//	cm->delta_q_res = 1 << aom_rb_read_literal(rb, 2);
	//	if (!cm->allow_intrabc) cm->delta_lf_present_flag = aom_rb_read_bit(rb);
	//	if (cm->delta_lf_present_flag) {
	//		cm->delta_lf_res = 1 << aom_rb_read_literal(rb, 2);
	//		cm->delta_lf_multi = aom_rb_read_bit(rb);
	//		av1_reset_loop_filter_delta(xd, av1_num_planes(cm)); //av1_reset_loop_filter_delta: blockd.c //av1_num_planes: onyxc_int.h
	//	}
	//}

	//xd->cur_frame_force_integer_mv = cm->cur_frame_force_integer_mv;

	//for (int i = 0; i < MAX_SEGMENTS; ++i) {
	//	const int qindex = cm->seg.enabled
	//		? av1_get_qindex(&cm->seg, i, cm->base_qindex) //quant_common.c
	//		: cm->base_qindex;
	//	xd->lossless[i] = qindex == 0 && cm->y_dc_delta_q == 0 &&
	//		cm->u_dc_delta_q == 0 && cm->u_ac_delta_q == 0 &&
	//		cm->v_dc_delta_q == 0 && cm->v_ac_delta_q == 0;
	//	xd->qindex[i] = qindex;
	//}
	//cm->coded_lossless = is_coded_lossless(cm, xd); //onyxc_int.h
	//cm->all_lossless = cm->coded_lossless && !av1_superres_scaled(cm); //resize.h
	//setup_segmentation_dequant(cm); //decodeframe.c
	//if (cm->coded_lossless) {
	//	cm->lf.filter_level[0] = 0;
	//	cm->lf.filter_level[1] = 0;
	//}
	//if (cm->coded_lossless || !seq_params->enable_cdef) {
	//	cm->cdef_bits = 0;
	//	cm->cdef_strengths[0] = 0;
	//	cm->cdef_uv_strengths[0] = 0;
	//}
	//if (cm->all_lossless || !seq_params->enable_restoration) {
	//	cm->rst_info[0].frame_restoration_type = RESTORE_NONE;
	//	cm->rst_info[1].frame_restoration_type = RESTORE_NONE;
	//	cm->rst_info[2].frame_restoration_type = RESTORE_NONE;
	//}
	//setup_loopfilter(cm, rb); //decodeframe.c

	//if (!cm->coded_lossless && seq_params->enable_cdef) {
	//	setup_cdef(cm, rb); //decodeframe.c
	//}
	//if (!cm->all_lossless && seq_params->enable_restoration) {
	//	decode_restoration_mode(cm, rb); //decodeframe.c
	//}

	//cm->tx_mode = read_tx_mode(cm, rb); //decodeframe.c
	//cm->reference_mode = read_frame_reference_mode(cm, rb); //decodeframe.c
	//if (cm->reference_mode != SINGLE_REFERENCE) setup_compound_reference_mode(cm); //decodeframe.c

	//av1_setup_skip_mode_allowed(cm); //mvref_common.c
	//cm->skip_mode_flag = cm->is_skip_mode_allowed ? aom_rb_read_bit(rb) : 0;

	//if (frame_might_allow_warped_motion(cm)) //onyxc_int.h
	//	cm->allow_warped_motion = aom_rb_read_bit(rb);
	//else
	//	cm->allow_warped_motion = 0;

	//cm->reduced_tx_set_used = aom_rb_read_bit(rb);

	//if (cm->allow_ref_frame_mvs && !frame_might_allow_ref_frame_mvs(cm)) { //onyxc_int.h
	//	aom_internal_error(&cm->error, AOM_CODEC_CORRUPT_FRAME,
	//		"Frame wrongly requests reference frame MVs");
	//}

	//if (!frame_is_intra_only(cm)) read_global_motion(cm, rb); //frame_is_intra_only(): onyxc_int.h, read_global_motion(): decodeframe.c

	//cm->cur_frame->film_grain_params_present =
	//	seq_params->film_grain_params_present;
	//read_film_grain(cm, rb);

#if EXT_TILE_DEBUG
	if (pbi->ext_tile_debug && cm->large_scale_tile) {
		read_ext_tile_info(pbi, rb);
		av1_set_single_tile_decoding_mode(cm);
	}
#endif  // EXT_TILE_DEBUG
	return 0;
}