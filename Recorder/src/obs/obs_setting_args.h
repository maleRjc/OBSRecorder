#pragma once
#ifndef RECORDER_SRC_OBS_OBS_SETTING_ARGS_H_
#define RECORDER_SRC_OBS_OBS_SETTING_ARGS_H_

/* OBS output types. */
// Encoding and encapsulation by FFMPEG.
#define OBS_OUTPUT_TYPE_ALLFFMPEG       "ffmpeg_output"
// Encoding by libx264 and libaac. Encapsulation by FFMPEG.
#define OBS_OUTPUT_TYPE_FFMPEGMUX       "ffmpeg_muxer"


/* OBS output properties. */
// "ffmpeg_output" setting args:
#define OBS_OUTPUT_ATTRNAME_URL         "url"     // Output path(file name, only used by ffmpeg_output)
#define OBS_OUTPUT_ATTRNAME_FORMAT      "format_name" // Suffix, eg: gif/mp4/avi...
#define OBS_OUTPUT_ATTRNAME_FMT         "format_mime_type" 
#define OBS_OUTPUT_ATTRNAME_MS          "muxer_settings"
#define OBS_OUTPUT_ATTRNAME_ENCNAME_V   "video_encoder"  // Video encoder's format(associate AVCodec::name)
#define OBS_OUTPUT_ATTRNAME_ENCID_V     "video_encoder_id" // Video encoder's id(associate AVCodecID)
#define OBS_OUTPUT_ATTRNAME_SETTING_V   "video_settings"
#define OBS_OUTPUT_ATTRNAME_BIRATE_V    "video_bitrate"
#define OBS_OUTPUT_ATTRNAME_ENCNAME_A   "audio_encoder" // Audio encoder's format(associate AVCodec::name)
#define OBS_OUTPUT_ATTRNAME_ENCID_A     "audio_encoder_id" //Audio encoder's id(associate AVCodecID)
#define OBS_OUTPUT_ATTRNAME_SETTING_A   "audio_settings"
#define OBS_OUTPUT_ATTRNAME_BIRATE_A    "audio_bitrate"
#define OBS_OUTPUT_ATTRNAME_GOPSIZE     "gop_size" // Keyframe intervals(associate AVCodecContext::gop_size)
#define OBS_OUTPUT_ATTRNAME_SWIDTH      "scale_width" // width
#define OBS_OUTPUT_ATTRNAME_SHEIGHT     "scale_height" //height
// "ffmpeg_muxer" setting args:
#define OBS_OUTPUT_ATTRNAME_PATH        "path"   // Output path(file name, only used byffmpeg_muxer)
// Plug flow setting args:
#define OBS_OUTPUT_ATTRNAME_BINDID      "bind_ip"  
#define OBS_OUTPUT_ATTRNAME_NSLE        "new_socket_loop_enabled" 
#define OBS_OUTPUT_ATTRNAME_LLME        "low_latency_mode_enabled"
#define OBS_OUTPUT_ATTRNAME_VBR         "dyn_bitrate"
// Replay cache setting args:
#define OBS_OUTPUT_ATTRNAME_DIR         "directory" 
#define OBS_OUTPUT_ATTRNAME_OUTFORMAT   "format" 
#define OBS_OUTPUT_ATTRNAME_EXTENSION   "extension" 
#define OBS_OUTPUT_ATTRNAME_ALLOWSPACE  "allow_spaces" 
#define OBS_OUTPUT_ATTRNAME_MTS         "max_time_sec" 
#define OBS_OUTPUT_ATTRNAME_MAXSIZE     "max_size_mb" 



/* OBS source types. */
// Video source:
#define OBS_SOURCE_TYPE_PICTURE         "image_source"     // A image.
#define OBS_SOURCE_TYPE_DSHOW           "dshow_input"      // A camera(video device).
#define OBS_SOURCE_TYPE_SCREEN          "monitor_capture"  // A screen.
#define OBS_SOURCE_TYPE_WINDOW          "window_capture"   // A specify window.
#define OBS_SOURCE_TYPE_COLOR           "color_source"     // A color rect.
#define OBS_SOURCE_TYPE_SLIDE           "slideshow"        // A ppt(many image).
#define OBS_SOURCE_TYPE_GAME            "game_capture"     // A game window.
// Audio source:
#define OBS_SOURCE_TYPE_INDEVICE_A      "wasapi_input_capture"   // A microphone.
#define OBS_SOURCE_TYPE_OUTDEVICE_A     "wasapi_output_capture"  // A loudspeaker.


/* OBS source properties. */
// "image_source":
#define OBS_SOURCE_TYPE_PICTURE_FILE                "file"
#define OBS_SOURCE_TYPE_PICTURE_UNLOAD              "unload"
// "dshow_input":
#define OBS_SOURCE_TYPE_DSHOW_VIDEO_DEVICE_ID       "video_device_id"
#define OBS_SOURCE_TYPE_DSHOW_ACTIVE                "active"
#define OBS_SOURCE_TYPE_DSHOW_ACTIVATE              "activate"
#define OBS_SOURCE_TYPE_DSHOW_DEACTIVATE            "deactivate_when_not_showing"
#define OBS_SOURCE_TYPE_DSHOW_RES_TYPE              "res_type"
#define OBS_SOURCE_TYPE_DSHOW_RESOLUTION            "resolution"
#define OBS_SOURCE_TYPE_DSHOW_FRAME_INTERVAL        "frame_interval"
// "monitor_capture":
#define OBS_SOURCE_TYPE_SCREEN_MONITOR              "monitor"
#define OBS_SOURCE_TYPE_SCREEN_CAPTURE_CURSOR       "capture_cursor"
// "color_source":
#define OBS_SOURCE_TYPE_COLOR_COLOR                 "color"
#define OBS_SOURCE_TYPE_COLOR_WIDTH                 "width"
#define OBS_SOURCE_TYPE_COLOR_HEIGHT                "height"
// "window_capture":
#define OBS_SOURCE_TYPE_WINDOW_WINDOW               "window"
#define OBS_SOURCE_TYPE_WINDOW_PRIORITY             "priority"
#define OBS_SOURCE_TYPE_WINDOW_CURSOR               "cursor"
#define OBS_SOURCE_TYPE_WINDOW_COMPATIBILITY        "compatibility"
// "wasapi_input_capture"/"wasapi_output_capture":
#define OBS_SOURCE_TYPE_DEVICE_A_DEVICE_ID          "device_id"
#define OBS_SOURCE_TYPE_DEVICE_A_USE_DEVICE_TIMING  "use_device_timing"

#endif  // RECORDER_SRC_OBS_OBS_SETTING_ARGS_H_