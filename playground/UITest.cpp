
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <giomm/menuitem.h>
#include <glibmm.h>
#include <giomm.h>
#include <giomm/menuitem.h>
#include <giomm/simpleaction.h>
#include <iostream>

using namespace std;

extern "C" void main_save() {
  cout << "main_save called in global scope" << endl;
}

class UITestApplication : public Gtk::Application {
public:
  UITestApplication(string appname = "de.atomlogik.uitest")
    : Gtk::Application(appname) {
  }
   
protected:

  virtual void on_startup() override {
    Gtk::Application::on_startup();
    auto builder = Gtk::Builder::create_from_file("uitest.ui");
    main_window = builder->get_object<Gtk::Window>("main_window");
    main_window->show();
    add_window(*main_window);
    cout << "window title: " << main_window->get_title() << endl;

    // get what we need to attache event handlers to.
    main_save = builder->get_object<Gtk::Button>("main_save");
    main_save_as = builder->get_object<Gtk::Button>("main_save_as");
    main_quit = builder->get_object<Gtk::Button>("main_quit");
    main_entry = builder->get_object<Gtk::Entry>("main_entry");
    main_entry_buffer = builder->get_object<Gtk::EntryBuffer>("main_entry_buffer");
 
    // Connect signals to handlers (slots?)
    main_save->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_save));
    main_save_as->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_save_as));
    main_quit->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_quit));
    main_entry->signal_insert_text().connect(sigc::mem_fun(*this, &UITestApplication::on_insert), true);
    main_entry->signal_delete_text().connect(sigc::mem_fun(*this, &UITestApplication::on_delete), true);
    main_entry->signal_changed().connect(sigc::mem_fun(*this, &UITestApplication::on_change), true);
    main_entry_buffer->signal_inserted_text().connect(sigc::mem_fun(*this, &UITestApplication::on_inserted), true);
    main_entry_buffer->signal_deleted_text().connect(sigc::mem_fun(*this, &UITestApplication::on_deleted), true);
  }
  
  void on_main_save() {
    cout << "main_save called" << endl;
  }

  void on_main_save_as() {
    cout << "main_save_as called" << endl;
  }

  void on_main_quit() {
    cout << "main_quit called" << endl;
    quit();
  }

  void on_insert(const Glib::ustring& text, int* position) {
    cout << "on_insert_text: " << text << "at " << * position << endl;
    std::string filtered_text;
    for (char c : text) {
      if (std::isdigit(c) || (c == '-' && *position == 0)) {
        filtered_text += c;
      }
    }
    
    if (filtered_text != text.raw()) {
      main_entry->insert_text(filtered_text
                                , filtered_text.length()
                                , *position);
      main_entry->signal_insert_text().emission_stop();
      }
    }

  void on_delete(int dstart, int dend) {
    cout << "on_delete(" << dstart << ", " << dend << ")" << endl;
  }

  void on_change() {
    cout << "on_change() -> " << main_entry->get_text()  << endl;
  }

  void on_inserted(guint position, const char* chars, guint n_chars) {
    cout << "on_inserted("
         << position << ", "
         << chars << ", "
         << n_chars << ")" << endl;
  }
  
  void on_deleted(guint position, guint n_chars) {
    cout << "on_deleted("
         << position << ", "
         << n_chars << ")" << endl;
  }
  
private:
  shared_ptr<Gtk::Window> main_window;  
  shared_ptr<Gtk::Button> main_save;
  shared_ptr<Gtk::Button> main_save_as;
  shared_ptr<Gtk::Button> main_quit;
  shared_ptr<Gtk::Entry> main_entry;
  shared_ptr<Gtk::EntryBuffer> main_entry_buffer;
};

int main(int argc, char* argv[]) {
  UITestApplication app;
  return app.run(argc, argv);
}

