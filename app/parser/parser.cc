#include "parser.h"

//#include <cmath>
#include <iostream>

using namespace std;
namespace mp = boost::multiprecision;

string numToStr(bint num) {
  string s;

  breal num_r(num);
  breal bw = mp::floor(mp::sqrt(mp::log(num_r) / mp::log(breal(2.0)))) + 1;

  cerr << "num: " << num << endl;
  cerr << "bw: " << bw << endl;
  int w = (int) bw;
  cerr << "w: " << w << endl;
  
  int index = 0;
  for (int y = 0; y < w + 1; y++) {
    for (int x = 0; x < w + 1; x++) {
      if (x == 0 && y == 0) {
        s += ' ';
      } else if(x == 0 || y == 0) {
        s += '#';
      } else {
        s += (mp::bit_test(num, index)? '#' : ' ');
        index++;
      }
    }
    s += '\n';
  }
  return s;
}
