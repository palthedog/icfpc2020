#include <fstream>
#include <cstdlib>

#include "vm.h"

using namespace std;

Sexp parse(istringstream& iss, bool lhs) {
  string token;
  if (!getline(iss, token, ' ')) {
    return Sexp();
  }

  Exp* p;
  if (token[0] == ':') {
    p = new Line(token);
  } else if (token == "ap") {
    Sexp ap(new Ap());

    Sexp f = parse(iss);
    if (!f) {
      return ap;
    }
    ap = ap->bind(f);

    Sexp arg = parse(iss);
    if (!arg) {
      return ap;
    }
    return ap->bind(arg);
  } else if (token == "c") {
    p = new CComb();
  } else if (token == "b") {
    p = new BComb();
  } else if (token == "s") {
    p = new SComb();
  } else if (token == "cons") {
    p = new Cons();
  } else if (token == "add") {
    p = new BinaryFunc(
        "sum",
        [](Sexp a, Sexp b){ return Sexp(new Num(a->to_int() + b->to_int())); });
  } else if (token == "mul") {
    p = new BinaryFunc(
        "mul",
        [](Sexp a, Sexp b){ return Sexp(new Num(a->to_int() * b->to_int())); });
  } else if (token == "inc") {
    p = new UnaryFunc(
        "inc",
        [](Sexp a){ return Sexp(new Num(a->to_int() + 1)); });
  } else if (token == "dec") {
    p = new UnaryFunc(
        "dec",
        [](Sexp a){ return Sexp(new Num(a->to_int() - 1)); });
  } else if (token == "neg") {
    p = new UnaryFunc(
        "neg",
        [](Sexp a){ return Sexp(new Num(-a->to_int())); });
  } else if (token == "nil") {
    p = new Nil();
  } else if (token[0] >= '0' && token[0] <= '9') {
    p = new Num(token);
  } else if (lhs) {
    p = new Protocol(token);
  } else {
    cerr << "Unknown: " << token << endl;
    cerr.flush();
    exit(1);
  }
  return Sexp(p);
}


VM::VM(const string&path) {
  ifstream ifs(path);

  if (!ifs) {
    cerr << "Failed to open file: " << path << endl;
  }

  int i = 0;
  string line;
  while (getline(ifs, line)) {
    cout << (i++) << " line: " << line << endl;

    int p = line.find(" = ");
    const string l = line.substr(0, p);
    const string r = line.substr(p + 3);
    
    Sexp lhs = parse(l);
    cout << "LHS: " << *lhs << endl;

    Sexp rhs = parse(r);
    cout << "RHS: " << *rhs << endl;    
  }
}
