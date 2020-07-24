/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MODULE_TAG "mpi_enc_utils"

#include <string.h>

#include "mpp_mem.h"
#include "mpp_log.h"

#include "rk_mpi.h"
#include "utils.h"
#include "mpi_enc_utils.h"

#define MAX_FILE_NAME_LENGTH        256

MpiEncTestArgs *mpi_enc_test_cmd_get(void)
{
    MpiEncTestArgs *args = mpp_calloc(MpiEncTestArgs, 1);

    return args;
}

MPP_RET mpi_enc_test_cmd_update_by_args(MpiEncTestArgs* cmd, int argc, char **argv)
{
    const char *opt;
    const char *next;
    RK_S32 optindex = 1;
    RK_S32 handleoptions = 1;
    MPP_RET ret = MPP_NOK;

    if ((argc < 2) || (cmd == NULL))
        return ret;

    /* parse options */
    while (optindex < argc) {
        opt  = (const char*)argv[optindex++];
        next = (const char*)argv[optindex];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
            if (opt[1] == '-') {
                if (opt[2] != '\0') {
                    opt++;
                } else {
                    handleoptions = 0;
                    continue;
                }
            }

            opt++;

            switch (*opt) {
            case 'i':
                if (next) {
                    size_t len = strnlen(next, MAX_FILE_NAME_LENGTH);
                    if (len) {
                        cmd->file_input = mpp_calloc(char, len + 1);
                        strncpy(cmd->file_input, next, len);
                        name_to_frame_format(cmd->file_input, &cmd->format);
                    }
                } else {
                    mpp_err("input file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'o':
                if (next) {
                    size_t len = strnlen(next, MAX_FILE_NAME_LENGTH);
                    if (len) {
                        cmd->file_output = mpp_calloc(char, len + 1);
                        strncpy(cmd->file_output, next, len);
                        name_to_coding_type(cmd->file_output, &cmd->type);
                    }
                } else {
                    mpp_log("output file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'w':
                if (next) {
                    cmd->width = atoi(next);
                    if (!cmd->hor_stride)
                        cmd->hor_stride = cmd->width;
                } else {
                    mpp_err("invalid input width\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'h':
                if (!next)
                    goto PARSE_OPINIONS_OUT;

                if ((*(opt + 1) != '\0') && !strncmp(opt, "help", 4)) {
                    goto PARSE_OPINIONS_OUT;
                } else if (next) {
                    cmd->height = atoi(next);
                    if (!cmd->ver_stride)
                        cmd->ver_stride = cmd->height;
                }
                break;
            case 'u':
                if (next) {
                    cmd->hor_stride = atoi(next);
                } else {
                    mpp_err("invalid input width\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'v':
                if (next) {
                    cmd->ver_stride = atoi(next);
                } else {
                    mpp_log("input height is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'f':
                if (next) {
                    cmd->format = (MppFrameFormat)atoi(next);
                    ret = ((cmd->format >= MPP_FMT_YUV_BUTT && cmd->format < MPP_FRAME_FMT_RGB) ||
                           cmd->format >= MPP_FMT_RGB_BUTT);
                }

                if (!next || ret) {
                    mpp_err("invalid input format %d\n", cmd->format);
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 't':
                if (next) {
                    cmd->type = (MppCodingType)atoi(next);
                    ret = mpp_check_support_format(MPP_CTX_ENC, cmd->type);
                }

                if (!next || ret) {
                    mpp_err("invalid input coding type\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'n':
                if (next) {
                    cmd->num_frames = atoi(next);
                } else {
                    mpp_err("invalid input max number of frames\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'g':
                if (next) {
                    cmd->gop_mode = atoi(next);
                } else {
                    mpp_err("invalid gop mode\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'b':
                if (next) {
                    cmd->bps_target = atoi(next);
                } else {
                    mpp_err("invalid bit rate\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'r':
                if (next) {
                    cmd->fps_out = atoi(next);
                } else {
                    mpp_err("invalid output frame rate\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'l':
                if (next) {
                    cmd->loop_cnt = atoi(next);
                } else {
                    mpp_err("invalid loop count\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            default:
                mpp_err("skip invalid opt %c\n", *opt);
                break;
            }

            optindex++;
        }
    }

    ret = MPP_OK;

    /* check essential parameter */
    if (cmd->type <= MPP_VIDEO_CodingAutoDetect) {
        mpp_err("invalid type %d\n", cmd->type);
        ret = MPP_NOK;
    }
    if (cmd->width <= 0 || cmd->height <= 0 ||
        cmd->hor_stride <= 0 || cmd->ver_stride <= 0) {
        mpp_err("invalid w:h [%d:%d] stride [%d:%d]\n",
                cmd->width, cmd->height, cmd->hor_stride, cmd->ver_stride);
        ret = MPP_NOK;
    }

PARSE_OPINIONS_OUT:
    return ret;
}

MPP_RET mpi_enc_test_cmd_put(MpiEncTestArgs* cmd)
{
    if (NULL == cmd)
        return MPP_OK;

    MPP_FREE(cmd->file_input);
    MPP_FREE(cmd->file_output);
    MPP_FREE(cmd);

    return MPP_OK;
}

MPP_RET mpi_enc_gen_gop_ref(MppEncGopRef *ref, RK_S32 gop_mode)
{
    ref->change = 0;
    ref->gop_cfg_enable = 0;

    /* error gop and tsvc config */
    if (gop_mode < 0 || gop_mode > 3)
        return MPP_NOK;

    // no tsvc config
    if (gop_mode == 0)
        return MPP_OK;

    ref->change = 1;
    ref->gop_cfg_enable = 1;

    // default no LTR
    ref->lt_ref_interval = 0;
    ref->max_lt_ref_cnt = 0;

    // rockchip tsvc config
    MppGopRefInfo *gop = &ref->gop_info[0];

    if (gop_mode == 3) {
        // tsvc4
        //      /-> P1      /-> P3        /-> P5      /-> P7
        //     /           /             /           /
        //    //--------> P2            //--------> P6
        //   //                        //
        //  ///---------------------> P4
        // ///
        // P0 ------------------------------------------------> P8
        ref->ref_gop_len    = 8;
        ref->layer_weight[0] = 800;
        ref->layer_weight[1] = 400;
        ref->layer_weight[2] = 400;
        ref->layer_weight[3] = 400;

        gop[0].temporal_id  = 0;
        gop[0].ref_idx      = 0;
        gop[0].is_non_ref   = 0;
        gop[0].is_lt_ref    = 1;
        gop[0].lt_idx       = 0;

        gop[1].temporal_id  = 3;
        gop[1].ref_idx      = 0;
        gop[1].is_non_ref   = 1;
        gop[1].is_lt_ref    = 0;
        gop[1].lt_idx       = 0;

        gop[2].temporal_id  = 2;
        gop[2].ref_idx      = 0;
        gop[2].is_non_ref   = 0;
        gop[2].is_lt_ref    = 0;
        gop[2].lt_idx       = 0;

        gop[3].temporal_id  = 3;
        gop[3].ref_idx      = 2;
        gop[3].is_non_ref   = 1;
        gop[3].is_lt_ref    = 0;
        gop[3].lt_idx       = 0;

        gop[4].temporal_id  = 1;
        gop[4].ref_idx      = 0;
        gop[4].is_non_ref   = 0;
        gop[4].is_lt_ref    = 1;
        gop[4].lt_idx       = 1;

        gop[5].temporal_id  = 3;
        gop[5].ref_idx      = 4;
        gop[5].is_non_ref   = 1;
        gop[5].is_lt_ref    = 0;
        gop[5].lt_idx       = 0;

        gop[6].temporal_id  = 2;
        gop[6].ref_idx      = 4;
        gop[6].is_non_ref   = 0;
        gop[6].is_lt_ref    = 0;
        gop[6].lt_idx       = 0;

        gop[7].temporal_id  = 3;
        gop[7].ref_idx      = 6;
        gop[7].is_non_ref   = 1;
        gop[7].is_lt_ref    = 0;
        gop[7].lt_idx       = 0;

        gop[8].temporal_id  = 0;
        gop[8].ref_idx      = 0;
        gop[8].is_non_ref   = 0;
        gop[8].is_lt_ref    = 1;
        gop[8].lt_idx       = 0;

        ref->max_lt_ref_cnt = 2;
    } else if (gop_mode == 2) {
        // tsvc3
        //     /-> P1      /-> P3
        //    /           /
        //   //--------> P2
        //  //
        // P0/---------------------> P4
        ref->ref_gop_len    = 4;
        ref->layer_weight[0] = 1000;
        ref->layer_weight[1] = 500;
        ref->layer_weight[2] = 500;
        ref->layer_weight[3] = 0;

        gop[0].temporal_id  = 0;
        gop[0].ref_idx      = 0;
        gop[0].is_non_ref   = 0;
        gop[0].is_lt_ref    = 0;
        gop[0].lt_idx       = 0;

        gop[1].temporal_id  = 2;
        gop[1].ref_idx      = 0;
        gop[1].is_non_ref   = 1;
        gop[1].is_lt_ref    = 0;
        gop[1].lt_idx       = 0;

        gop[2].temporal_id  = 1;
        gop[2].ref_idx      = 0;
        gop[2].is_non_ref   = 0;
        gop[2].is_lt_ref    = 0;
        gop[2].lt_idx       = 0;

        gop[3].temporal_id  = 2;
        gop[3].ref_idx      = 2;
        gop[3].is_non_ref   = 1;
        gop[3].is_lt_ref    = 0;
        gop[3].lt_idx       = 0;

        gop[4].temporal_id  = 0;
        gop[4].ref_idx      = 0;
        gop[4].is_non_ref   = 0;
        gop[4].is_lt_ref    = 0;
        gop[4].lt_idx       = 0;

        // set to lt_ref interval with looping LTR idx
        ref->lt_ref_interval = 10;
        ref->max_lt_ref_cnt = 3;
    } else if (gop_mode == 1) {
        // tsvc2
        //   /-> P1
        //  /
        // P0--------> P2
        ref->ref_gop_len    = 2;
        ref->layer_weight[0] = 1400;
        ref->layer_weight[1] = 600;
        ref->layer_weight[2] = 0;
        ref->layer_weight[3] = 0;

        gop[0].temporal_id  = 0;
        gop[0].ref_idx      = 0;
        gop[0].is_non_ref   = 0;
        gop[0].is_lt_ref    = 0;
        gop[0].lt_idx       = 0;

        gop[1].temporal_id  = 1;
        gop[1].ref_idx      = 0;
        gop[1].is_non_ref   = 1;
        gop[1].is_lt_ref    = 0;
        gop[1].lt_idx       = 0;

        gop[2].temporal_id  = 0;
        gop[2].ref_idx      = 0;
        gop[2].is_non_ref   = 0;
        gop[2].is_lt_ref    = 0;
        gop[2].lt_idx       = 0;
    }

    return MPP_OK;
}

MPP_RET mpi_enc_gen_osd_data(MppEncOSDData *osd_data, MppBuffer osd_buf, RK_U32 frame_cnt)
{
    RK_U32 k = 0;
    RK_U32 buf_size = 0;
    RK_U32 buf_offset = 0;
    RK_U8 *buf = mpp_buffer_get_ptr(osd_buf);

    osd_data->num_region = 8;
    osd_data->buf = osd_buf;

    for (k = 0; k < osd_data->num_region; k++) {
        MppEncOSDRegion *region = &osd_data->region[k];
        RK_U8 idx = k;

        region->enable = 1;
        region->inverse = frame_cnt & 1;
        region->start_mb_x = k * 3;
        region->start_mb_y = k * 2;
        region->num_mb_x = 2;
        region->num_mb_y = 2;

        buf_size = region->num_mb_x * region->num_mb_y * 256;
        buf_offset = k * buf_size;
        osd_data->region[k].buf_offset = buf_offset;

        memset(buf + buf_offset, idx, buf_size);
    }

    return MPP_OK;
}

MPP_RET mpi_enc_gen_osd_plt(MppEncOSDPlt *osd_plt, RK_U32 *table)
{
    RK_U32 k = 0;

    if (osd_plt->data && table) {
        for (k = 0; k < 256; k++)
            osd_plt->data[k].val = table[k % 8];
    }
    return MPP_OK;
}

static OptionInfo mpi_enc_cmd[] = {
    {"i",               "input_file",           "input bitstream file"},
    {"o",               "output_file",          "output bitstream file, "},
    {"w",               "width",                "the width of input picture"},
    {"h",               "height",               "the height of input picture"},
    {"f",               "format",               "the format of input picture"},
    {"t",               "type",                 "output stream coding type"},
    {"n",               "max frame number",     "max encoding frame number"},
    {"g",               "gop_mode",             "gop reference mode"},
    {"d",               "debug",                "debug flag"},
    {"b",               "target bps",           "set tareget bps"},
    {"r",               "output frame rate",    "set output frame rate"},
    {"l",               "loop count",           "loop encoding times for each frame"},
};

MPP_RET mpi_enc_test_cmd_show_opt(MpiEncTestArgs* cmd)
{
    mpp_log("cmd parse result:\n");
    mpp_log("input  file name: %s\n", cmd->file_input);
    mpp_log("output file name: %s\n", cmd->file_output);
    mpp_log("width      : %d\n", cmd->width);
    mpp_log("height     : %d\n", cmd->height);
    mpp_log("format     : %d\n", cmd->format);
    mpp_log("type       : %d\n", cmd->type);

    return MPP_OK;
}

void mpi_enc_test_help(void)
{
    mpp_log("usage: mpi_enc_test [options]\n");
    show_options(mpi_enc_cmd);
    mpp_show_support_format();
    mpp_show_color_format();
}
