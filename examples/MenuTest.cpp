
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <giomm/menuitem.h>

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.example.menuapp");

    // Load the UI from the Glade file
    auto builder = Gtk::Builder::create();
    builder->add_from_file("menubar.glade");

    // Get references to the window and menu bar
    auto window = builder->get_object<Gtk::Window>("main_window");
    auto menubar = builder->get_object<Gio::Menu>("menubar");

    // Create menu items using the builder
    auto file_menu_item = builder->get_object<Gio::MenuItem>("file_menu_item");
    auto quit_menu_item = builder->get_object<Gio::MenuItem>("quit_menu_item");

    // Connect the quit menu item to the application's quit function
    // quit_menu_item->signal_activate().connect(
    //     [&app]() { app->quit(); }
    // );

    // Add the menu items to the menu bar
    // file_menu_item->set_submenu(menubar->get_submenu());

    window->set_default_size(300, 200);
    window->show();

    return app->run();
}

/// #include <gtkmm.h>
/// #include <iostream>
/// 
/// class MyWindow : public Gtk::ApplicationWindow {
/// public:
///     MyWindow() {
///         // Load the UI from the .ui file
///         auto builder = Gtk::Builder::create_from_file("menu_example.ui");
/// 
///         // Get the main window from the builder
///         auto bw = builder->get_widget("main_window");
///         add(*m_MainWindow);
/// 
///         // Connect signals for menu items
///         builder->signal_connect("on_menu_file_new", sigc::ptr_fun(&MyWindow::on_menu_file_new));
///         builder->signal_connect("on_menu_file_open", sigc::ptr_fun(&MyWindow::on_menu_file_open));
///         builder->signal_connect("on_menu_file_quit", sigc::ptr_fun(&MyWindow::on_menu_file_quit));
/// 
///         // Show all widgets
///         m_MainWindow->show_all();
///     }
/// 
/// protected:
///     void on_menu_file_new() {
///         std::cout << "New menu item selected." << std::endl;
///     }
/// 
///     void on_menu_file_open() {
///         std::cout << "Open menu item selected." << std::endl;
///     }
/// 
///     void on_menu_file_quit() {
///         hide(); // Close the window
///     }
/// 
/// private:
///     Gtk::ApplicationWindow* m_MainWindow = nullptr;
/// };
/// 
/// class MyApp : public Gtk::Application {
/// protected:
///     MyApp() : Gtk::Application("org.gtkmm.menu_example") {}
/// 
///     void on_activate() override {
///         auto window = Gtk::make_managed<MyWindow>();
///         window->set_application(*this);
///         window->show();
///     }
/// };
/// 
/// int main(int argc, char* argv[]) {
///     auto app = Gtk::make_refptr_for_instance<MyApp>(new MyApp());
///     return app->run(argc, argv);
/// }
