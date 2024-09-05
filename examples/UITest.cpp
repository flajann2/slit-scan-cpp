
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <giomm/menuitem.h>
#include <glibmm.h>
#include <giomm.h>
#include <giomm/menuitem.h>
#include <giomm/simpleaction.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.example.menuapp");

    // Load the UI from the Glade file
    auto builder = Gtk::Builder::create();
    builder->add_from_file("uitest");

    // Get references to the window and menu bar
    auto window = builder->get_object<Gtk::Window>("main_window");
    auto menubar = builder->get_object<Gio::Menu>("menubar");

    // Create menu items using the builder
    auto file_menu_item = builder->get_object<Gio::MenuItem>("file_item");
    auto quit_menu_item = builder->get_object<Gio::MenuItem>("quit_item");

    // Connect the quit menu item to the application's quit function
    auto action = Gio::SimpleAction::create("quit-action");
    /// action->signal_activate().connect([&app]() {
    ///     // Handle the action here
    ///     std::cout << "Quit Action activated!" << std::endl;
    ///     app->quit();
    /// });
    /// quit_menu_item->set_action_and_target(action->get_name()
    ///                                       , Glib::Variant<std::string>("quit_target_value"));

    /// quit_menu_item->signal_activate().connect(
    ///     [&app]() { app->quit(); }
    /// );

    // Add the menu items to the menu bar
    // file_menu_item->set_submenu(menubar->get_submenu());

    window->set_default_size(300, 200);
    window->show();

    return app->run();
}

