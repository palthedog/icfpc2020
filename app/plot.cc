#include <iostream>
#include <sstream>

#include <plot.h>

using namespace std;

void Plot::sendMessage(const std::string& mes) {
  if (online_) {
    asio::write(sock_, asio::buffer(mes));  
  } else {
    cerr << "OFFLINE: " << mes << endl;
  }
}

void Plot::draw(int x, int y) {
  dots_.insert(make_pair(x, y));
}

void Plot::startDraw() {
  dots_.clear();
}

void Plot::endDraw() {
  sendMessage("START\n");
  send();
  sendMessage("END\n");
}

void Plot::clear() {
  sendMessage("CLEAR\n");
}

void Plot::flush() {
  sendMessage("FLUSH\n");
}

void Plot::send() {
  ostringstream oss;
  for (auto dot : dots_) {
    int x = dot.first;
    int y = dot.second;
    oss << "DRAW " << x << " " << y << endl;  
  }
  cout << "Sending dots" << endl;
  sendMessage(oss.str());
  cout << "Sent" << endl;
}

std::string Plot::read() {
  if (!online_) {
    return "click 0 0";
  }
  
  asio::streambuf buffer;
  boost::system::error_code error;

  sendMessage("READ\n");
  
  cout << "read" << endl;
  asio::read(sock_, buffer, asio::transfer_at_least(1), error);
  cout << "read done" << endl;

  if (error && error != asio::error::eof) {
    cerr << "receive failed: " << error.message() << endl;
    exit(1);
  }

  return asio::buffer_cast<const char*>(buffer.data());
}
