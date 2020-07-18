#include <iostream>
#include <sstream>

#include <plot.h>

using namespace std;

void Plot::draw(int x, int y) {
  ostringstream oss;
  oss << "DRAW " << x << " " << y << endl;
  asio::write(sock_, asio::buffer(oss.str()));  
}

void Plot::startDraw() {
  asio::write(sock_, asio::buffer("START\n"));
}

void Plot::endDraw() {
  asio::write(sock_, asio::buffer("END\n"));
}

std::string Plot::read() {
  asio::streambuf buffer;
  boost::system::error_code error;
  asio::read(sock_, buffer, asio::transfer_at_least(1), error);
  if (error && error != asio::error::eof) {
    cerr << "receive failed: " << error.message() << endl;
    exit(1);
  }

  return asio::buffer_cast<const char*>(buffer.data());
}
