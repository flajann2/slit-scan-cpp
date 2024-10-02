// EntryEditor -- edit Entry fields, convert on the fly, handle
//                invalid character insertions.
#pragma once

namespace Gui {
  using namespace std;
  
  template<typename T>
  class EntryEditor : public Composition {
  public:
    EntryEditor(T& field, shared_ptr<Gtk::Entry> entry_) : entry(entry_)
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
    void on_change() {
      cout << "on_change() -> " << entry->get_text()  << endl;
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
  };
}
