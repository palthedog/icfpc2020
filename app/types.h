#ifndef TYPES_H
#define TYPES_H

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace mp = boost::multiprecision;
using bint = mp::cpp_int;
using breal = mp::cpp_dec_float_100;

std::string numToStr(bint num);
bint decode(const std::string&bstr);

#endif
