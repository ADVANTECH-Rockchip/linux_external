/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
 *     Author: Randy Li <randy.li@rock-chips.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>

#include "gstmppjpegenc.h"

#define GST_CAT_DEFAULT mppvideoenc_debug

#define parent_class gst_mpp_jpeg_enc_parent_class
G_DEFINE_TYPE (GstMppJpegEnc, gst_mpp_jpeg_enc, GST_TYPE_MPP_VIDEO_ENC);

#define DEFAULT_PROP_QUANT 10

enum
{
  PROP_0,
  PROP_QUANT,
  PROP_LAST,
};

static GstStaticPadTemplate gst_mpp_jpeg_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg, "
        "width  = (int) [ 96, 8192 ], " "height = (int) [ 32, 8192 ], "
        /* Up to 90 million pixels per second at the rk3399 */
        "framerate = (fraction) [0/1, 60/1], " "sof-marker = { 0 }")
    );

static void
gst_mpp_h264_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);

  switch (prop_id) {
    case PROP_QUANT:{
      guint quant = g_value_get_uint (value);
      if (self->quant == quant)
        return;

      self->quant = quant;
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }

  self->prop_dirty = TRUE;
}

static void
gst_mpp_h264_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);

  switch (prop_id) {
    case PROP_QUANT:
      g_value_set_uint (value, self->quant);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_mpp_jpeg_enc_open (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Opening");

  if (mpp_create (&self->mpp_ctx, &self->mpi))
    goto failure;
  if (mpp_init (self->mpp_ctx, MPP_CTX_ENC, MPP_VIDEO_CodingMJPEG))
    goto failure;

  return TRUE;

failure:
  return FALSE;
}

static void
gst_mpp_jpeg_enc_update_properties (GstVideoEncoder * encoder)
{
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);
  GstMppVideoEnc *mppenc = GST_MPP_VIDEO_ENC (encoder);
  MppEncCfg cfg;

  if (!self->prop_dirty)
    return;

  if (mpp_enc_cfg_init (&cfg)) {
    GST_WARNING_OBJECT (self, "Init enc cfg failed");
    return;
  }

  if (mppenc->mpi->control (mppenc->mpp_ctx, MPP_ENC_GET_CFG, cfg)) {
    GST_WARNING_OBJECT (self, "Get enc cfg failed");
    mpp_enc_cfg_deinit (cfg);
    return;
  }

  mpp_enc_cfg_set_s32 (cfg, "jpeg:quant", self->quant);

  if (mppenc->mpi->control (mppenc->mpp_ctx, MPP_ENC_SET_CFG, cfg))
    GST_WARNING_OBJECT (self, "Set enc cfg failed");

  mpp_enc_cfg_deinit (cfg);

  self->prop_dirty = FALSE;
}

static gboolean
gst_mpp_jpeg_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  gst_mpp_jpeg_enc_update_properties (encoder);

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->set_format (encoder, state);
}

static GstFlowReturn
gst_mpp_jpeg_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstCaps *outcaps;
  GstFlowReturn ret;

  outcaps = gst_caps_new_empty_simple ("image/jpeg");

  gst_mpp_jpeg_enc_update_properties (encoder);

  ret = GST_MPP_VIDEO_ENC_CLASS (parent_class)->handle_frame (encoder, frame,
      outcaps);
  gst_caps_unref (outcaps);
  return ret;
}

static void
gst_mpp_jpeg_enc_init (GstMppJpegEnc * self)
{
  self->quant = DEFAULT_PROP_QUANT;
  self->prop_dirty = TRUE;
}

static void
gst_mpp_jpeg_enc_class_init (GstMppJpegEncClass * klass)
{
  GstElementClass *element_class;
  GObjectClass *gobject_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp JPEG Encoder",
      "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>");

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_get_property);

  video_encoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_enc_open);
  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_enc_src_template));

  g_object_class_install_property (gobject_class, PROP_QUANT,
      g_param_spec_uint ("quant", "Quant",
          "JPEG Quantization", 0, 10, DEFAULT_PROP_QUANT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
