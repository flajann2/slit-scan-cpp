// ScanEffect.h -- the effects the scanner can produce.
//
// This header stands alone (nothing but the standard library), so
// that both the video configuration dialog and the scanner can use
// it. To add an effect: add it here and to scan_effect_name(), add
// it to create_effect_list() and to the Effect box in the Video
// Setup dialog (in the same order), and teach SlitScan to make it.
#pragma once

namespace Gel {

  enum class ScanEffect {
    // The slit stands upright. The picture has a left half and a
    // right half, which meet in the middle and stream outwards.
    VerticalSlit

    // The slit lies flat. The picture has a top half and a bottom
    // half, which meet in the middle and stream up and down. The
    // left gels make the top half and the right gels the bottom.
    , HorizontalSlit
  };

  inline const char* scan_effect_name(ScanEffect effect) {
    switch (effect) {
    case ScanEffect::VerticalSlit:   return "vertical slit scan";
    case ScanEffect::HorizontalSlit: return "horizontal slit scan";
    }
    return "?";
  }
}
