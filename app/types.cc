#include "types.h"

#include <cstdlib>

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

bint decode(const string&bstr) {
  bint n;
  for (int i = 0; i < bstr.size(); i++) {
    n <<= 1;
    if (bstr[i] == '1') {
      n |= 1;
    } else if (bstr[i] == '0') {
    } else {
      cerr << "Not a binary: " << bstr << endl;
      exit(1);
    }
  }
  return n;
}
