// SlitScan -- does the vertical or horizontal slit scaning from left and right
//             image sources. Will do an output frame as a signal.
#pragma once

namespace Gel {

  class SlitScan {
  public:
    SlitScan() {
      tick_server = IMasterTickServer::create();
    }

  protected:
    void on_tick(time_stamp tick) {
      cout << "SlitScan tick: " << tick << endl;      
    }

  private:
    unique_ptr<IMasterTickServer> tick_server;
    
  };
}
