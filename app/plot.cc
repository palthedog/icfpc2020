#include <iostream>
#include <sstream>

#include <plot.h>

using namespace std;

void Plot::draw(int x, int y) {
  dots_.insert(make_pair(x, y));
}

void Plot::startDraw() {
  dots_.clear();
  //asio::write(sock_, asio::buffer("START\n"));
  asio::write(sock_, asio::buffer("START\n"));  
}

void Plot::endDraw() {
  send();

  asio::write(sock_, asio::buffer("END\n"));
}

void Plot::send() {
  ostringstream oss;
  for (auto dot : dots_) {
    int x = dot.first;
    int y = dot.second;
    cout << "DRAW " << x << " " << y << endl;  
    oss << "DRAW " << x << " " << y << endl;  
  }
  cout << "Sending dots" << endl;
  asio::write(sock_, asio::buffer(oss.str()));  
  cout << "Sent" << endl;
}

std::string Plot::read() {
  asio::streambuf buffer;
  boost::system::error_code error;

  asio::write(sock_, asio::buffer("READ\n"));
  
  cout << "read" << endl;
  asio::read(sock_, buffer, asio::transfer_at_least(1), error);
  cout << "read done" << endl;

  if (error && error != asio::error::eof) {
    cerr << "receive failed: " << error.message() << endl;
    exit(1);
  }

  return asio::buffer_cast<const char*>(buffer.data());
}
