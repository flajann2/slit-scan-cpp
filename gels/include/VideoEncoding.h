// VideoEncoding.h -- the video codecs we can record with, which
// file formats can hold which, and how to ask for each.
//
// This header stands alone (nothing but the standard library), so
// that both the video configuration dialog and the recorder can
// use it.
#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Gel {

  // The first four are the ones offered in the Video Setup dialog.
  // The rest are what a file format falls back to when it can hold
  // none of those, or when no encoder for them is to be had.
  enum class VideoCodec { H264, H265, VP9, AV1
                          , Theora, MPEG4, MJPEG, VP8 };

  inline const char* codec_name(VideoCodec codec) {
    switch (codec) {
    case VideoCodec::H264:   return "H.264";
    case VideoCodec::H265:   return "H.265";
    case VideoCodec::VP9:    return "VP9";
    case VideoCodec::AV1:    return "AV1";
    case VideoCodec::Theora: return "Theora";
    case VideoCodec::MPEG4:  return "MPEG-4";
    case VideoCodec::MJPEG:  return "Motion JPEG";
    case VideoCodec::VP8:    return "VP8";
    }
    return "?";
  }

  // Can a file of this format (its extension: "mp4", "mkv", ...)
  // hold video of this codec? This is what FFmpeg's muxers accept;
  // the odd pairings that technically work in avi are left out.
  inline bool container_holds(const std::string& format, VideoCodec codec) {
    using C = VideoCodec;
    if (format == "mp4")  return codec == C::H264 || codec == C::H265 || codec == C::VP9 || codec == C::AV1 || codec == C::MPEG4;
    if (format == "mkv")  return codec != C::MJPEG;
    if (format == "mov")  return codec == C::H264 || codec == C::H265 || codec == C::MPEG4;
    if (format == "webm") return codec == C::VP9 || codec == C::AV1 || codec == C::VP8;
    if (format == "avi")  return codec == C::H264 || codec == C::MPEG4 || codec == C::MJPEG;
    if (format == "ogg")  return codec == C::Theora;
    return false;
  }

  // The codec a file format gets when it cannot hold the one asked for.
  inline VideoCodec default_codec(const std::string& format) {
    if (format == "webm") return VideoCodec::VP9;
    if (format == "ogg")  return VideoCodec::Theora;
    return VideoCodec::H264;
  }

  // The file format that best suits a codec.
  inline std::string natural_container(VideoCodec codec) {
    switch (codec) {
    case VideoCodec::VP9:
    case VideoCodec::VP8:    return "webm";
    case VideoCodec::Theora: return "ogg";
    case VideoCodec::MJPEG:  return "avi";
    default:                 return "mp4";
    }
  }

  // How to ask the ffmpeg program for a codec, given the encoders
  // it has (by name, as `ffmpeg -encoders` lists them). The
  // settings are chosen to keep up with the scan as it happens:
  // the encoders' own defaults are meant for unhurried encoding,
  // and some of them manage less than a frame a second that way.
  // Empty if this ffmpeg has no encoder for the codec.
  inline std::vector<std::string> ffmpeg_codec_args(VideoCodec codec
                                                    , const std::set<std::string>& encoders) {
    auto has = [&](const char* name) { return encoders.count(name) > 0; };

    switch (codec) {
    case VideoCodec::H264:
      if (has("libx264"))     return {"-c:v", "libx264", "-preset", "veryfast", "-crf", "20"};
      if (has("libopenh264")) return {"-c:v", "libopenh264", "-b:v", "8M"};
      break;

    case VideoCodec::H265:
      if (has("libx265"))     return {"-c:v", "libx265", "-preset", "veryfast", "-crf", "24"
                                      , "-x265-params", "log-level=error"};
      break;

    case VideoCodec::VP9:
      if (has("libvpx-vp9"))  return {"-c:v", "libvpx-vp9", "-deadline", "realtime", "-cpu-used", "8"
                                      , "-row-mt", "1", "-crf", "32", "-b:v", "0"};
      break;

    case VideoCodec::AV1:
      if (has("libsvtav1"))   return {"-c:v", "libsvtav1", "-preset", "10", "-crf", "34"};
      if (has("libaom-av1"))  return {"-c:v", "libaom-av1", "-usage", "realtime", "-cpu-used", "8"
                                      , "-row-mt", "1", "-crf", "34", "-b:v", "0"};
      if (has("librav1e"))    return {"-c:v", "librav1e", "-speed", "10", "-qp", "100"};
      break;

    case VideoCodec::Theora:
      if (has("libtheora"))   return {"-c:v", "libtheora", "-q:v", "7"};
      break;

    case VideoCodec::MPEG4:
      if (has("mpeg4"))       return {"-c:v", "mpeg4", "-q:v", "4"};
      break;

    case VideoCodec::MJPEG:
      if (has("mjpeg"))       return {"-c:v", "mjpeg", "-q:v", "3"};
      break;

    case VideoCodec::VP8:
      if (has("libvpx"))      return {"-c:v", "libvpx", "-deadline", "realtime", "-cpu-used", "8"
                                      , "-crf", "12", "-b:v", "4M"};
      break;
    }
    return {};
  }

  // The same question put to OpenCV's own video writer, which is
  // what we fall back on without the ffmpeg program: the four
  // character codes to try for a codec, in order.
  inline std::vector<std::string> opencv_fourccs(VideoCodec codec) {
    switch (codec) {
    case VideoCodec::H264:   return {"avc1", "H264"};
    case VideoCodec::H265:   return {"hvc1", "HEVC"};
    case VideoCodec::VP9:    return {"vp09", "VP90"};
    case VideoCodec::AV1:    return {"av01", "AV01"};
    case VideoCodec::Theora: return {"THEO"};
    case VideoCodec::MPEG4:  return {"mp4v"};
    case VideoCodec::MJPEG:  return {"MJPG"};
    case VideoCodec::VP8:    return {"VP80"};
    }
    return {};
  }

  // The codec OpenCV can be counted on to have for a file format,
  // for when it has nothing better.
  inline VideoCodec last_resort_codec(const std::string& format) {
    if (format == "avi")  return VideoCodec::MJPEG;
    if (format == "webm") return VideoCodec::VP8;
    if (format == "ogg")  return VideoCodec::Theora;
    return VideoCodec::MPEG4;
  }
}
