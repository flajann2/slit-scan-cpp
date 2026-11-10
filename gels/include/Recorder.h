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
//
// The encoding itself is done by the ffmpeg program when there is
// one: the frames are piped to it raw. That gives us every codec
// ffmpeg has, at settings that keep up with the scan, and files
// that phones and messengers will play. Without ffmpeg we fall
// back on OpenCV's own video writer, which has fewer codecs and no
// way to set their speed.
#pragma once

#include <VideoEncoding.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <format>
#include <list>
#include <memory>
#include <sstream>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Gel {

  class Recorder : public sigc::trackable {
  public:
    // what to record
    struct Request {
      std::string pathname;
      std::string format;  // the kind of file: "mp4", "mkv", ...
      VideoCodec codec = VideoCodec::H264;
      double fps = 30.0;
      cv::Size size;       // frames of another size are scaled to this
    };

    // what became of a recording
    struct Result {
      std::string pathname;
      long frames = 0;  // 0 means nothing was recorded, and no file was kept
      bool ok = true;   // false if the encoder failed along the way
      std::string how;  // codec and encoder, for the user to read
    };

    // makes the OpenCV video writer. Only tests have a reason to supply one.
    using WriterFactory = std::function<std::shared_ptr<cv::VideoWriter>()>;

    explicit Recorder(WriterFactory factory_ = {})
      : factory(std::move(factory_))
      , shared(std::make_shared<Shared>()) {
      m_dispatcher.connect(sigc::mem_fun(*this, &Recorder::on_finished));
      shared->dispatcher = &m_dispatcher;

      // Should ffmpeg die on us, writing to its pipe must fail with
      // an error we can report, not kill the whole application.
      std::signal(SIGPIPE, SIG_IGN);
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
        const bool done = session->done_cv.wait_for(lock, std::chrono::seconds(5)
                                                    , [&](){ return session->done; });
        bool done2 = done;
        if (!done) {
          // an encoder that is stuck part way is put out of its
          // misery; one that is finishing the file is left to it.
          session->sink->abandon();
          done2 = session->done_cv.wait_for(lock, std::chrono::seconds(2)
                                            , [&](){ return session->done; });
        }
        lock.unlock();
        if (done2)
          session->thread.join();
        else {
          cerr << "SlitScan: not waiting any longer for " << session->pathname
               << " to close." << endl;
          session->thread.detach();
        }
      }
    }

    Recorder(const Recorder&) = delete;
    Recorder& operator=(const Recorder&) = delete;

    // Tests switch this off to exercise the OpenCV writer.
    void set_use_ffmpeg(bool use) { use_ffmpeg = use; }

    // GUI thread. Starts a new video file. Returns false, and
    // records nothing, if the file cannot be written or no encoder
    // can be had for it.
    //
    // If the file format cannot hold the codec asked for, or there
    // is no encoder for it, the nearest thing is recorded instead;
    // description() says what was settled on.
    bool start(const Request& request) {
      stop(); // one recording at a time

      // can we write there at all? Better to hear it now than
      // from the encoder later.
      if (FILE* probe = std::fopen(request.pathname.c_str(), "wb")) {
        std::fclose(probe);
        std::remove(request.pathname.c_str());
      } else {
        return false;
      }

      // the codecs to try, best first
      std::vector<VideoCodec> wanted;
      if (container_holds(request.format, request.codec))
        wanted.push_back(request.codec);
      if (default_codec(request.format) != request.codec)
        wanted.push_back(default_codec(request.format));

      auto session = std::make_shared<Session>();
      session->pathname = request.pathname;

      if (use_ffmpeg && FfmpegSink::available()) {
        for (VideoCodec codec : wanted) {
          session->sink = FfmpegSink::open(request, codec);
          if (session->sink)
            break;
        }
      }
      if (!session->sink) {
        wanted.push_back(last_resort_codec(request.format));
        for (VideoCodec codec : wanted) {
          session->sink = OpenCvSink::open(request, codec, factory);
          if (session->sink)
            break;
        }
      }
      if (!session->sink)
        return false;

      session->how = session->sink->how;
      if (session->sink->codec != request.codec)
        session->how += std::string(", instead of ") + codec_name(request.codec);

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

    // The codec and encoder of the current recording, for the user
    // to read: "H.264 (ffmpeg libx264)", say.
    std::string description() {
      lockguard lock(current_mutex);
      return current ? current->how : std::string();
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
    // Where the frames of a recording go. Made in the GUI thread;
    // write() and close() are called in the recording's own thread.
    struct Sink {
      virtual ~Sink() = default;
      virtual bool write(const cv::Mat& rgb) = 0; // false once it has failed
      virtual bool close() = 0;                   // true if the file is good

      // GUI thread, when quitting and the recording's thread has
      // not finished: stop an encoder that is stuck.
      virtual void abandon() {}
      VideoCodec codec = VideoCodec::H264;        // what it really encodes
      std::string how;
    };

    // The frame at the size the file wants, in one unbroken block.
    static cv::Mat fitted(const cv::Mat& frame, cv::Size size) {
      cv::Mat out = frame;
      if (out.size() != size)
        cv::resize(frame, out, size, 0, 0, cv::INTER_AREA);
      if (!out.isContinuous())
        out = out.clone();
      return out;
    }

    // ---- the ffmpeg program, fed raw frames through a pipe
    struct FfmpegSink : Sink {
      GPid pid = 0;
      int fd = -1; // ffmpeg's standard input
      cv::Size size;
      bool failed = false;

      // abandon() comes from another thread: these two say whether
      // there is still a process to stop, and whether it should be.
      std::mutex pid_mutex;                // guards pid once the thread runs
      std::atomic<bool> finishing{false};  // close() has been reached

      static bool available() {
        gchar* path = g_find_program_in_path("ffmpeg");
        const bool found = path != nullptr;
        g_free(path);
        return found;
      }

      // The encoders this ffmpeg has. Asked once, and remembered.
      // GUI thread only.
      static const std::set<std::string>& encoders() {
        static std::optional<std::set<std::string>> known;
        if (known)
          return *known;
        known.emplace();

        const gchar* argv[] = {"ffmpeg", "-hide_banner", "-encoders", nullptr};
        gchar* out = nullptr;
        if (g_spawn_sync(nullptr, const_cast<gchar**>(argv), nullptr
                         , GSpawnFlags(G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL)
                         , nullptr, nullptr, &out, nullptr, nullptr, nullptr) && out) {
          // lines read " V....D libx264   libx264 H.264 / AVC ...":
          // flags, then the name. Only video encoders interest us.
          std::istringstream lines(out);
          std::string line;
          while (std::getline(lines, line)) {
            std::istringstream words(line);
            std::string flags, name;
            if ((words >> flags >> name) && flags.size() == 6 && flags[0] == 'V')
              known->insert(name);
          }
        }
        g_free(out);
        return *known;
      }

      // Starts ffmpeg for this request and codec. Nothing comes
      // back if this ffmpeg has no encoder for the codec, or if it
      // cannot be started.
      static std::unique_ptr<Sink> open(const Request& request, VideoCodec codec) {
        const auto codec_args = ffmpeg_codec_args(codec, encoders());
        if (codec_args.empty())
          return {};

        std::vector<std::string> args = {
          "ffmpeg", "-nostdin", "-hide_banner", "-loglevel", "error", "-y"
          // what comes down the pipe: raw RGB frames
          , "-f", "rawvideo", "-pix_fmt", "rgb24"
          , "-video_size", std::format("{}x{}", request.size.width, request.size.height)
          , "-framerate", std::format("{}", request.fps)
          , "-i", "pipe:0"
        };
        args.insert(args.end(), codec_args.begin(), codec_args.end());

        // the pixel format every player can decode
        args.insert(args.end(), {"-pix_fmt", "yuv420p"});

        if (request.format == "mp4" || request.format == "mov") {
          // the index at the front, so that the video can start
          // playing before it has all arrived
          args.insert(args.end(), {"-movflags", "+faststart"});
          // and the tag Apple's players want on H.265
          if (codec == VideoCodec::H265)
            args.insert(args.end(), {"-tag:v", "hvc1"});
        }
        args.push_back(request.pathname);

        std::vector<gchar*> argv;
        for (auto& arg : args)
          argv.push_back(arg.data());
        argv.push_back(nullptr);

        auto sink = std::make_unique<FfmpegSink>();
        GError* error = nullptr;
        // ffmpeg's errors go to our own standard error, where the
        // user can read them.
        if (!g_spawn_async_with_pipes(nullptr, argv.data(), nullptr
                                      , GSpawnFlags(G_SPAWN_SEARCH_PATH
                                                    | G_SPAWN_DO_NOT_REAP_CHILD
                                                    | G_SPAWN_STDOUT_TO_DEV_NULL)
                                      , nullptr, nullptr
                                      , &sink->pid, &sink->fd, nullptr, nullptr
                                      , &error)) {
          cerr << "SlitScan: cannot start ffmpeg: " << (error ? error->message : "?") << endl;
          g_clear_error(&error);
          return {};
        }

        // never to be inherited by another ffmpeg, which would keep
        // the pipe open and this one waiting for more; and never
        // to block, so that a stuck ffmpeg cannot hold us for ever.
        fcntl(sink->fd, F_SETFD, fcntl(sink->fd, F_GETFD) | FD_CLOEXEC);
        fcntl(sink->fd, F_SETFL, fcntl(sink->fd, F_GETFL) | O_NONBLOCK);

        sink->size = request.size;
        sink->codec = codec;
        sink->how = std::string(codec_name(codec)) + " (ffmpeg " + codec_args[1] + ")";
        return sink;
      }

      ~FfmpegSink() override {
        if (fd >= 0)
          ::close(fd);
        if (pid) {
          // close() was never reached: do not leave it running
          kill(pid, SIGKILL);
          waitpid(pid, nullptr, 0);
          g_spawn_close_pid(pid);
        }
      }

      bool write(const cv::Mat& rgb) override {
        if (failed)
          return false;

        const cv::Mat frame = fitted(rgb, size);
        const uchar* data = frame.data;
        size_t left = frame.total() * frame.elemSize();
        int stalled_ms = 0;

        while (left > 0) {
          const ssize_t n = ::write(fd, data, left);
          if (n > 0) {
            data += n;
            left -= size_t(n);
            stalled_ms = 0;
            continue;
          }
          if (n < 0 && errno == EINTR)
            continue;
          if (n < 0 && errno == EAGAIN) {
            // the pipe is full: wait for ffmpeg to take some. An
            // ffmpeg that takes nothing for this long is stuck.
            pollfd p{fd, POLLOUT, 0};
            poll(&p, 1, 250);
            stalled_ms += 250;
            if (stalled_ms < 15000)
              continue;
            cerr << "SlitScan: ffmpeg has stopped taking frames." << endl;
          } else {
            // most likely ffmpeg gave up; it will have said why
            cerr << "SlitScan: ffmpeg stopped (" << std::strerror(errno) << ")." << endl;
          }
          failed = true;
          return false;
        }
        return true;
      }

      // Stuck while still taking frames, it is stopped. Once it has
      // been told there are no more it is left alone: it is a
      // program of its own, and finishes the file whether or not
      // we are still here.
      void abandon() override {
        lockguard lock(pid_mutex);
        if (pid && !finishing)
          kill(pid, SIGKILL);
      }

      // Tells ffmpeg there is no more, and waits for it to finish
      // the file.
      bool close() override {
        finishing = true;
        if (fd >= 0) {
          ::close(fd);
          fd = -1;
        }
        if (!pid)
          return false;

        // Finishing can take a while on a long recording, but not
        // for ever: an ffmpeg still at it after two minutes is stuck.
        int status = 0;
        bool exited = false;
        for (int waited_ms = 0; waited_ms < 120000; waited_ms += 50) {
          const pid_t r = waitpid(pid, &status, WNOHANG);
          if (r == pid || (r < 0 && errno != EINTR)) {
            exited = (r == pid);
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (!exited) {
          cerr << "SlitScan: ffmpeg did not finish; stopping it." << endl;
          kill(pid, SIGKILL);
          waitpid(pid, &status, 0);
        }
        {
          lockguard lock(pid_mutex);
          g_spawn_close_pid(pid);
          pid = 0;
        }

        return exited && !failed && WIFEXITED(status) && WEXITSTATUS(status) == 0;
      }
    };

    // ---- OpenCV's own video writer
    struct OpenCvSink : Sink {
      std::shared_ptr<cv::VideoWriter> writer;
      cv::Size size;
      bool failed = false;

      static std::unique_ptr<Sink> open(const Request& request, VideoCodec codec
                                        , const WriterFactory& factory) {
        auto sink = std::make_unique<OpenCvSink>();
        sink->writer = factory ? factory() : std::make_shared<cv::VideoWriter>();

        // FFmpeg first, by name: of OpenCV's backends it is the one
        // that writes and closes files dependably. Only if it will
        // not have the file do we take whatever else there is.
        for (const auto& cc : opencv_fourccs(codec)) {
          const int fourcc = cv::VideoWriter::fourcc(cc[0], cc[1], cc[2], cc[3]);
          for (int backend : {int(cv::CAP_FFMPEG), int(cv::CAP_ANY)}) {
            try {
              if (sink->writer->open(request.pathname, backend, fourcc
                                     , request.fps, request.size, true)
                  && sink->writer->isOpened()) {
                sink->size = request.size;
                sink->codec = codec;
                sink->how = std::string(codec_name(codec))
                  + " (OpenCV " + sink->writer->getBackendName() + ")";
                return sink;
              }
            } catch (const cv::Exception& e) {
              cerr << "SlitScan: " << e.what() << endl;
            }
          }
        }
        std::remove(request.pathname.c_str()); // what a failed try left behind
        return {};
      }

      bool write(const cv::Mat& rgb) override {
        if (failed)
          return false;
        try {
          // the video file wants BGR
          cv::Mat bgr;
          cv::cvtColor(fitted(rgb, size), bgr, cv::COLOR_RGB2BGR);
          writer->write(bgr);
        } catch (const cv::Exception& e) {
          cerr << "SlitScan: " << e.what() << endl;
          failed = true;
        }
        return !failed;
      }

      bool close() override {
        try {
          writer->release(); // writes out the end of the file
        } catch (const cv::Exception& e) {
          cerr << "SlitScan: " << e.what() << endl;
          failed = true;
        }
        return !failed;
      }
    };

    // one recording
    struct Session {
      std::shared_ptr<Sink> sink;
      std::string pathname;
      std::string how;
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
      result.how = session->how;

      // If the encoder fails we still take the frames off the
      // queue, so that the scanner is not left waiting on us.
      while (auto rgb = session->frames.dequeue()) {
        if (session->sink->write(*rgb))
          ++result.frames;
        else
          result.ok = false;
      }
      const bool closed_well = session->sink->close();

      if (result.frames == 0) {
        // a video file with no frames in it is of no use to anyone,
        // and players take it for a broken one. An encoder that
        // was given nothing may well call that an error; we do not.
        std::remove(result.pathname.c_str());
      } else if (!closed_well) {
        result.ok = false;
      }

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
    bool use_ffmpeg = true;
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
