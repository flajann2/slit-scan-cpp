
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
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
    auto builder = Gtk::Builder::create_from_file("uitest");
    main_window = builder->get_object<Gtk::Window>("main_window");
    main_window->show();
    add_window(*main_window);
    cout << "window title: " << main_window->get_title() << endl;

    // get what we need to attache event handlers to.
    main_save = builder->get_object<Gtk::Button>("main_save");
    main_save_as = builder->get_object<Gtk::Button>("main_save_as");
    main_quit = builder->get_object<Gtk::Button>("main_quit");
    
    // Connect signals to handlers (slots?)
    main_save->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_save));
    main_save_as->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_save_as));
    main_quit->signal_clicked().connect(sigc::mem_fun(*this, &UITestApplication::on_main_quit));
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
private:
  shared_ptr<Gtk::Window> main_window;  
  shared_ptr<Gtk::Button> main_save;
  shared_ptr<Gtk::Button> main_save_as;
  shared_ptr<Gtk::Button> main_quit;
};

int main(int argc, char* argv[]) {
  UITestApplication app;
  return app.run(argc, argv);
}

