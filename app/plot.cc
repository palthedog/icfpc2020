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
