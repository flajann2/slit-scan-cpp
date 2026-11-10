// Recorder.h -- writes frames to a video file, in a thread of its own.
//
// Writing a frame to a video file, and above all closing the file,
// are things that take as long as the video encoder cares to take.
// Sometimes that is a long time, and with some encoders closing
// has been known never to come back at all. So none of it happens
// in the GUI thread, which must stay free to answer the buttons,
// nor in the scanner's worker, which has frames to make.
//
// Each recording gets a thread. Frames go to it through a queue;
// stop() tells it there will be no more, and returns at once. The
// thread writes what is still queued, closes the file, and says so
// by way of signal_finished(), in the GUI thread.
#pragma once

#include <cstdio>
#include <list>
#include <memory>

namespace Gel {

  class Recorder : public sigc::trackable {
  public:
    // what became of a recording
    struct Result {
      std::string pathname;
      long frames = 0;     // 0 means nothing was recorded, and no file was kept
      std::string backend; // which of OpenCV's video backends wrote it
    };

    // makes the video writer. Only tests have a reason to supply one.
    using WriterFactory = std::function<std::shared_ptr<cv::VideoWriter>()>;

    explicit Recorder(WriterFactory factory_ = {})
      : factory(std::move(factory_))
      , shared(std::make_shared<Shared>()) {
      m_dispatcher.connect(sigc::mem_fun(*this, &Recorder::on_finished));
      shared->dispatcher = &m_dispatcher;
    }

    // Finishes whatever is being recorded. Gives the files a few
    // seconds to close, and then leaves without them: an encoder
    // that will not let go must not keep the application from
    // quitting.
    ~Recorder() {
      stop();
      {
        lockguard lock(shared->mutex);
        shared->dispatcher = nullptr; // no more news to the GUI
      }
      for (auto& session : closing) {
        std::unique_lock<std::mutex> lock(session->done_mutex);
        const bool done = session->done_cv.wait_for(lock, std::chrono::seconds(3)
                                                    , [&](){ return session->done; });
        lock.unlock();
        if (done)
          session->thread.join();
        else {
          cerr << "SlitScan: gave up waiting for " << session->pathname
               << " to close." << endl;
          session->thread.detach();
        }
      }
    }

    Recorder(const Recorder&) = delete;
    Recorder& operator=(const Recorder&) = delete;

    // GUI thread. Starts a new video file. `fourcc` is the codec,
    // as from cv::VideoWriter::fourcc(); frames of another size are
    // scaled to `size`. Returns false, and records nothing, if the
    // file cannot be opened for writing.
    bool start(const std::string& pathname, int fourcc, double fps, cv::Size size) {
      stop(); // one recording at a time

      auto session = std::make_shared<Session>();
      session->writer = factory ? factory() : std::make_shared<cv::VideoWriter>();
      session->pathname = pathname;
      session->size = size;

      // FFmpeg first, by name: it is the backend that writes and
      // closes files dependably. Only if it will not have the file
      // do we take whatever else OpenCV offers.
      try {
        if (!session->writer->open(pathname, cv::CAP_FFMPEG, fourcc, fps, size, true)
            || !session->writer->isOpened())
          session->writer->open(pathname, cv::CAP_ANY, fourcc, fps, size, true);
        if (!session->writer->isOpened())
          return false;
        session->backend = session->writer->getBackendName();
      } catch (const cv::Exception& e) {
        cerr << "SlitScan: " << e.what() << endl;
        if (!session->writer->isOpened())
          return false;
      }

      session->thread = std::thread(&Recorder::run, session, shared);
      lockguard lock(current_mutex);
      current = session;
      return true;
    }

    // GUI thread. No more frames for the file; returns at once. The
    // file is closed in the background, and signal_finished() tells
    // when. Returns false if nothing was being recorded.
    bool stop() {
      std::shared_ptr<Session> session;
      {
        lockguard lock(current_mutex);
        session.swap(current);
      }
      if (!session)
        return false;

      session->frames.finish();
      closing.push_back(session);
      return true;
    }

    bool is_recording() {
      lockguard lock(current_mutex);
      return bool(current);
    }

    // which backend the current recording is written with
    std::string backend() {
      lockguard lock(current_mutex);
      return current ? current->backend : std::string();
    }

    // Any thread. One more frame, RGB, for the file being recorded;
    // nothing happens if there is none. If the encoder is behind,
    // this waits for it, so that no frame of the video is lost.
    void write(const cv::Mat& rgb) {
      std::shared_ptr<Session> session;
      {
        lockguard lock(current_mutex);
        session = current;
      }
      if (session)
        session->frames.enqueue(rgb);
    }

    // Emitted in the GUI thread when a file has been closed.
    sigc::signal<void(const Result&)>& signal_finished() { return m_sig_finished; }

  private:
    // one recording
    struct Session {
      std::shared_ptr<cv::VideoWriter> writer;
      std::string pathname;
      std::string backend;
      cv::Size size;
      GelQueue<cv::Mat> frames{16};
      std::thread thread;

      std::mutex done_mutex; // guards done
      std::condition_variable done_cv;
      bool done = false;
    };

    // what the recording threads share with the Recorder. They may
    // outlive it, which is why it is held by shared_ptr.
    struct Shared {
      std::mutex mutex; // guards the rest
      std::list<Result> results;
      Glib::Dispatcher* dispatcher = nullptr; // null once the Recorder is gone
    };

    // The thread of one recording.
    static void run(std::shared_ptr<Session> session, std::shared_ptr<Shared> shared) {
      Result result;
      result.pathname = session->pathname;
      result.backend = session->backend;

      // the video file wants BGR, at the size it was opened with.
      while (auto rgb = session->frames.dequeue()) {
        try {
          cv::Mat bgr;
          cv::cvtColor(*rgb, bgr, cv::COLOR_RGB2BGR);
          if (bgr.size() != session->size)
            cv::resize(bgr, bgr, session->size, 0, 0, cv::INTER_AREA);
          session->writer->write(bgr);
          ++result.frames;
        } catch (const cv::Exception& e) {
          cerr << "SlitScan: " << e.what() << endl;
        }
      }

      try {
        session->writer->release(); // writes out the end of the file
      } catch (const cv::Exception& e) {
        cerr << "SlitScan: " << e.what() << endl;
      }

      // a video file with no frames in it is of no use to anyone,
      // and players take it for a broken one.
      if (result.frames == 0)
        std::remove(result.pathname.c_str());

      {
        lockguard lock(shared->mutex);
        shared->results.push_back(result);
        if (shared->dispatcher)
          shared->dispatcher->emit();
      }
      {
        lockguard lock(session->done_mutex);
        session->done = true;
      }
      session->done_cv.notify_all();
    }

    // GUI thread, by way of the dispatcher.
    void on_finished() {
      std::list<Result> results;
      {
        lockguard lock(shared->mutex);
        results.swap(shared->results);
      }

      // the threads that are done can be put away
      for (auto it = closing.begin(); it != closing.end(); ) {
        bool done;
        {
          lockguard lock((*it)->done_mutex);
          done = (*it)->done;
        }
        if (done) {
          (*it)->thread.join();
          it = closing.erase(it);
        } else {
          ++it;
        }
      }

      for (const auto& result : results)
        m_sig_finished.emit(result);
    }

    WriterFactory factory;
    std::shared_ptr<Shared> shared;
    Glib::Dispatcher m_dispatcher;
    sigc::signal<void(const Result&)> m_sig_finished;

    std::mutex current_mutex;         // guards current
    std::shared_ptr<Session> current; // the recording in progress

    // GUI thread only: recordings that have been stopped and are
    // being closed.
    std::list<std::shared_ptr<Session>> closing;
  };
}
