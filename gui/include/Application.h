// application.h of Slit Scan
#pragma once

#include <giomm/resource.h>
#include <gio/gio.h>

using namespace std;

class SlitScanApp : public Gtk::Application {
public:
  // NON_UNIQUE: every launch is an application of its own.
  //
  // Without it there can only be one SlitScan per login session:
  // a second one finds the first over D-Bus, asks it to come to
  // the front, and exits at once with no window and no message.
  // That is a fine thing in a mail program. Here it means that one
  // copy left running out of sight, or stuck, silently stops every
  // later launch from starting.
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname, Gio::Application::Flags::NON_UNIQUE) {
    // the command line: slitscan --no-splash (and --help lists it)
    add_main_option_entry(Gio::Application::OptionType::BOOL, "no-splash", '\0'
                          , "Start without the splash screen");
    signal_handle_local_options().connect(
      sigc::mem_fun(*this, &SlitScanApp::on_local_options), false);
  }

  static SlitScanApp& get_ss_app() { return ss_app; }

  friend int main(int argc, char* argv[]);
  
protected:
  
  // Override the startup method to create the window and ball
  virtual void on_startup() override {
    Gtk::Application::on_startup();
    
    main_builder = Gui::obtain_builder();
    main_window = main_builder->get_object<Gtk::Window>(ss::main_window); 
    main_window->show();
    add_window(*main_window);

    // functionality
    gel_selection = Gui::GelSelection::create();

    // and, unless told otherwise, the splash screen over it all
    if (show_splash) {
      splash = make_unique<Gui::Splash>();
      splash->show();
    }
  }

  // The options given on the command line. This comes before
  // on_startup().
  int on_local_options(const Glib::RefPtr<Glib::VariantDict>& options) {
    if (options && options->contains("no-splash"))
      show_splash = false;
    return -1; // carry on and start
  }

private:
  shared_ptr<Gtk::Builder> main_builder;
  shared_ptr<Gtk::Window> main_window;

  unique_ptr<Gui::GelSelection> gel_selection;
  unique_ptr<Gui::Splash> splash;
  bool show_splash = true;

  static SlitScanApp ss_app;
};

using SSA = class SlitScanApp;
