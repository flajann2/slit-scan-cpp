// EntryEditor -- edit Entry fields, convert on the fly, handle
//                invalid character insertions.
#pragma once

namespace Gui {
  using namespace std;

  template<typename T>
  class EntryEditor : public Composition {
  public:
    EntryEditor(T& field_, shared_ptr<Gtk::Entry> entry_) : field(field_)
                                                          , entry(entry_)
                                                          , entry_buffer(entry_->get_buffer()) {
      cout << "field: " << field << endl;

      // set up the buffer
      entry->signal_changed().connect(sigc::mem_fun(*this, &EntryEditor::on_change), true);
      entry_buffer->signal_inserted_text().connect(sigc::mem_fun(*this, &EntryEditor::on_inserted), true);
      entry_buffer->signal_deleted_text().connect(sigc::mem_fun(*this, &EntryEditor::on_deleted), true);

      // initial value
      entry_buffer->set_text(format("{}", field));
    }
    
    // Show what the field holds. For when the field was changed
    // behind our back, or to throw away something half typed.
    void refresh() {
      entry_buffer->set_text(format("{}", field));
    }

  protected:
    // The whole of the text has to be a number: "12abc" is not 12.
    T from_string(const std::string_view& str) {
      T result;
      auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
      if (ec != std::errc{} || ptr != str.data() + str.size()) {
        throw std::runtime_error("Conversion failed");
      }
      return result;
    }

    // Every keystroke comes through here. What is typed takes hold
    // as soon as it is a number. While it is not one -- the box is
    // empty, or holds just a minus sign, on the way to a new value
    // -- the field keeps what it had and the box is marked, but the
    // text is left alone so that the typing can go on.
    void on_change() {
      const std::string text = entry->get_text();
      try {
        field = from_string(text);
        entry->remove_css_class("error");
      } catch (const exception& e) {
        entry->add_css_class("error");
      }
      cout << "on_change() -> " << field << endl;
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
    shared_ptr<Gtk::Entry> entry;
    shared_ptr<Gtk::EntryBuffer> entry_buffer;
    T& field;
  };
}
