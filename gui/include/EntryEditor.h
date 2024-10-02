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

  protected:
    T from_string(const std::string_view& str) {
      T result;
      auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
      if (ec != std::errc{}) {
        throw std::runtime_error("Conversion failed");
      }
      return result;
    }
    
    void on_change() {
      try {
        field = from_string(entry->get_text().c_str());
      } catch (const exception& e) {
        cout << e.what() << endl;
        // TODO: this simply reverts the field
        // TODO: back to the original value. We
        // TODO: do a better job of handling this.
        entry_buffer->set_text(format("{}", field));
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
