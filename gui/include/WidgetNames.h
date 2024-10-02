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

  // global settings
  const std::string main_settings                 = "ss_main_settings";
  const std::string main_settings_note            = "ss_main_settings_note";
  const std::string main_settings_note_p1_general = "ss_main_settings_note_p1_general";
  const std::string main_settings_p1_width_e      = "ss_main_settings_p1_width_e";
  const std::string main_settings_p1_spacing_e    = "ss_main_settings_p1_spacing_e";
  const std::string main_settings_p1_factor_e     = "ss_main_settings_p1_factor_e";

  const std::string main_settings_note_p2_video   = "ss_main_settings_note_p2_video";

  // configuration of individual gels
  const std::string gel_config = "ss_gel_config";
  const std::string gel_config_note = "ss_gel_config_note";
  const std::string gel_config_p1_gel = "ss_gel_config_p1_gel";
  const std::string gel_config_p2_camera = "ss_gel_config_p2_camera";
  const std::string gel_config_gel_p1_clock = "ss_gel_config_gel_p1_clock";
  const std::string gel_config_gel_p1_anticlock = "ss_gel_config_gel_p1_anticlock";


}
