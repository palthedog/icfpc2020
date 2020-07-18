#include <fstream>
#include <cstdlib>

#include "vm.h"

using namespace std;

std::string str(const Sexp e) {
  ostringstream oss;
  e->print(oss);
  return oss.str();
}

std::string str(const Exp* e) {
  ostringstream oss;
  e->print(oss);
  return oss.str();
}

Sexp ccomb_impl(Sexp x0, Sexp x1, Sexp x2) {
  cerr << "ccomb_impl" << endl;
  return ap(ap(x0, x2), x1);
}

Sexp parse(VM*vm, istringstream& iss) {
  string token;
  if (!getline(iss, token, ' ')) {
    return Sexp();
  }

  Exp* p;
  if (token[0] == ':') {
    p = new Function(vm, token);
  } else if (token == "ap") {
    Sexp f = parse(vm, iss);
    if (!f) {
      return ap();
    }

    Sexp arg = parse(vm, iss);
    if (!arg) {
      return ap(f);
    }
    return ap(f, arg);
  } else if (token == "c") {
    p = new TriFunc(
        "c",
        ccomb_impl);
  } else if (token == "b") {
    p = new TriFunc(
        "b",
        [](Sexp a, Sexp b, Sexp c) {
          return ap(a, ap(b, c));
        });
  } else if (token == "s") {
    return SComb();
  } else if (token == "cons") {
    p = new Cons();
  } else if (token == "add") {
    p = new BinaryFunc(
        "add",
        [](Sexp a, Sexp b){ return Sexp(new Num(to_int(a) + to_int(b))); });
  } else if (token == "eq") {
    p = new BinaryFunc(
        "eq",
        [](Sexp a, Sexp b){ return (to_int(a) == to_int(b)) ? CreateTrue() : CreateFalse(); });
  } else if (token == "lt") {
    p = new BinaryFunc(
        "lt",
        [](Sexp a, Sexp b){ return (to_int(a) < to_int(b)) ? CreateTrue() : CreateFalse(); });
  } else if (token == "t") {
    return CreateTrue();
  } else if (token == "f") {
    return CreateFalse();
  } else if (token == "mul") {
    p = new BinaryFunc(
        "mul",
        [](Sexp a, Sexp b){ return Sexp(new Num(to_int(a) * to_int(b))); });
  } else if (token == "div") {
    p = new BinaryFunc(
        "div",
        [](Sexp a, Sexp b){ return Sexp(new Num(to_int(a) / to_int(b))); });
  } else if (token == "inc") {
    p = new UnaryFunc(
        "inc",
        [](Sexp a){ return Sexp(new Num(to_int(a) + 1)); });
  } else if (token == "dec") {
    p = new UnaryFunc(
        "dec",
        [](Sexp a){ return Sexp(new Num(to_int(a) - 1)); });
  } else if (token == "neg") {
    p = new UnaryFunc(
        "neg",
        [](Sexp a){ return Sexp(new Num(-to_int(a))); });
  } else if (token == "isnil") {
    p = new UnaryFunc(
        "isnil",
        [](Sexp a){
          auto e = eval(a);
          return e->isNil() ? CreateTrue() : CreateFalse() ;
        });
  } else if (token == "car") {
    p = new UnaryFunc(
        "car",
        [](Sexp a){
          auto e = eval(a);
          return e->isCons() ? e->car() : ap(e, CreateTrue());
        });
  } else if (token == "cdr") {
    p = new UnaryFunc(
        "cdr",
        [](Sexp a){
          auto e = eval(a);
          return e->isCons() ? e->cdr() : ap(e, CreateFalse()) ;
        });
  } else if (token == "i") {
    p = new UnaryFunc(
        "i",
        [](Sexp a){ return a ; });
  } else if (token == "nil") {
    p = new Nil();
  } else if (token[0] == '-' || (token[0] >= '0' && token[0] <= '9')) {
    p = new Num(token);
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

    cout << "LHS: " << l << endl;
    

    Sexp rhs = parse(this, r);
    cout << "RHS: " << *rhs << endl;

    if (l[0] == ':') {
      // function
      int funcNum = stoi(l.substr(1));
      functions_[funcNum] = rhs;
    } else {
      // protocol
      protocols_[l] = rhs;
    }
  }
}

Sexp VM::protocol(const string&name) const {
  auto f = protocols_.find(name);
  if (f == protocols_.end()) {
    cerr << "Invalid protocol name: " << name << endl;
    exit(1);
  }
  return f->second;
}

Sexp VM::function(int index) const {
  auto f = functions_.find(index);
  if (f == functions_.end()) {
    cerr << "Invalid function id: " << index << endl;
    exit(1);
  }
  return f->second;
}

Sexp Function::call_(Sexp _this, Sexp arg) const{
  auto f = vm_->function(index_);
  //cout << "func.call: " << f << ", arg: " << arg << endl;
  return call(f, arg);
}

Sexp Function::eval_(Sexp _this) const{
  auto f = vm_->function(index_);
  //cout << "func.eval: " << f << endl;
  return eval(f);
}
