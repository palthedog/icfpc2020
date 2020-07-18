#ifndef PLOT_H
#define PLOT_H

#include <boost/asio.hpp>
#include <memory>

namespace asio = boost::asio;
namespace ip = asio::ip;

class Plot {
  asio::io_service io_service_;
  ip::tcp::socket sock_;

 public:

  Plot()
      : sock_(io_service_) {
    std::string server_name = "localhost";
    ip::tcp::resolver resolver(io_service_);
    
    ip::tcp::resolver::query query(server_name, "15151");

    boost::asio::connect(sock_, resolver.resolve(query));
  }

  void startDraw();
  void draw(int x, int y);
  void endDraw();
};

extern std::shared_ptr<Plot> plot;

#endif
