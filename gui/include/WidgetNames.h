// WidgetNames.h of SlitScan -- all the widget names as specified
// in Camblanche.
#pragma once

namespace ss {
  const std::string resource = "/de/atomlogik/gui/";
  const std::string main_window = "ss_main_window";

  // Gel Selection
  const std::string b_main_load     = "ssb_main_load";
  const std::string b_main_save     = "ssb_main_save";
  const std::string b_main_save_as  = "ssb_main_save_as";
  const std::string b_main_new      = "ssb_main_new";
  const std::string b_main_settings = "ssb_main_settings";
  
  const std::string main_gel_get_left = "ssb_main_gel_get_left";
  const std::string main_gel_get_right = "ssb_main_gel_get_right";
  const std::string main_gel_list_left = "ss_main_gel_list_left";
  const std::string main_gel_list_right = "ss_main_gel_list_right";
  const std::string main_canvas = "ss_main_canvas";

  const std::string file_gel_chooser = "ss_file_gel_chooser";

  //// the buttons at the bottom
  const std::string main_toggle_play   = "ss_main_toggle_play";
  const std::string main_video_setup   = "ss_main_video_setup";
  const std::string main_toggle_record = "ss_main_toggle_record";
  
  // dialog global settings
  const std::string main_settings                 = "ss_main_settings";
  const std::string main_settings_note            = "ss_main_settings_note";
  const std::string main_settings_note_p1_general = "ss_main_settings_note_p1_general";
  const std::string main_settings_p1_width_e      = "ss_main_settings_p1_width_e";
  const std::string main_settings_p1_spacing_e    = "ss_main_settings_p1_spacing_e";
  const std::string main_settings_p1_factor_e     = "ss_main_settings_p1_factor_e";

  const std::string main_settings_note_p2_video   = "ss_main_settings_note_p2_video";

  // dialog configuration of individual gels -- images, camera, video
  const std::string gel_config           = "ss_gel_config";
  const std::string gel_config_note      = "ss_gel_config_note";
  const std::string gel_config_note_p1   = "ss_gel_config_note_p1";
  const std::string gel_config_p1_canvas = "ss_gel_config_p1_canvas";
  const std::string gel_config_p1_rotate = "ss_gel_config_p1_rotate";
  const std::string gel_config_p1_rate   = "ss_gel_config_p1_rate";
  const std::string gel_config_p1_flip   = "ss_gel_config_p1_flip";
  const std::string gel_config_note_p2   = "ss_gel_config_note_p2";
  const std::string gel_config_p2_canvas = "ss_gel_config_p2_canvas";
  const std::string gel_config_note_p3   = "ss_gel_config_note_p3";
  const std::string gel_config_p3_canvas = "ss_gel_config_p3_canvas";

  // Video Config
  const std::string video_config             = "ss_video_config";
  const std::string video_config_note        = "ss_video_config_note";
  const std::string video_config_p1          = "ss_video_config_p1";
  const std::string video_p1_resolution      = "ss_video_p1_resolution";
  const std::string video_p1_resolution_list = "ss_video_p1_resolution_list";
  const std::string video_p1_fps             = "ss_video_p1_fps";
  const std::string video_p1_container       = "ss_video_p1_container";

  const std::string video_config_p2          = "ss_video_config_p2";
}
