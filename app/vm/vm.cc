#include <fstream>
#include <cstdlib>

#include <types.h>

#include "vm.h"

namespace mp = boost::multiprecision;

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

Sexp send_impl(Sexp l) {
  cerr << "send_impl: " << l << endl;
  // TODO
  exit(1);
  return Sexp(new Nil());;
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
  } else if (token == "send") {
    p = new UnaryFunc(
        "send",
        send_impl);
  } else if (token == "c") {
    p = new TriFunc(
        "c",
        [](Sexp x0, Sexp x1, Sexp x2) {
          return eval(ap(ap(eval(x0), eval(x2)), eval(x1)));
        });
  } else if (token == "b") {
    p = new TriFunc(
        "b",
        [](Sexp x0, Sexp x1, Sexp x2) {
          return eval(ap(eval(x0), ap(eval(x1), eval(x2))));
        });
  } else if (token == "s") {
    return SComb();
  } else if (token == "cons") {
    //p = new Cons();
    return Cons();
  } else if (token == "add") {
    p = new BinaryFunc(
        "add",
        [](Sexp a, Sexp b){ return Sexp(new Num(to_int(a) + to_int(b))); });
  } else if (token == "eq") {
    p = new BinaryFunc(
        "eq",
        [](Sexp a, Sexp b){
          //cout << "eq: " << a << ", " << b << endl;
          return (to_int(a) == to_int(b)) ? CreateTrue() : CreateFalse();
        });
  } else if (token == "lt") {
    p = new BinaryFunc(
        "lt",
        [](Sexp a, Sexp b){
          //cout << "lt: " << a << ", " << b << endl;
          return (to_int(a) < to_int(b)) ? CreateTrue() : CreateFalse();
        });
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
    /*
    p = new UnaryFunc(
        "car",
        [](Sexp a){
          auto e = eval(a);
          return e->isCons() ? e->car() : ap(e, CreateTrue());
        });
    */
    p = new UnaryFunc(
        "car",
        [](Sexp a){
          auto e = eval(a);
          return ap(a, CreateTrue());
        });
  } else if (token == "cdr") {
    /*
    p = new UnaryFunc(
        "cdr",
        [](Sexp a){
          auto e = eval(a);
          return e->isCons() ? e->cdr() : ap(e, CreateFalse()) ;
        });
    */
    p = new UnaryFunc(
        "cdr",
        [](Sexp a){
          auto e = eval(a);
          return ap(a, CreateFalse());
        });
  } else if (token == "i") {
    p = new UnaryFunc(
        "i",
        [](Sexp a){ return a ; });
  } else if (token == "nil") {
    p = new Nil();
  } else if (token[0] == '-' || (token[0] >= '0' && token[0] <= '9')) {
    p = new Num(token);
  } else if (token == "mod") {
    p = new UnaryFunc(
        "mod",
        [](Sexp a){
          auto e = to_int(a);
          return Sexp(new ModResult(e));
        });
    
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

Sexp interact();

Sexp f38() {
  return Sexp(new BinaryFunc(
      "f38",
      [=](Sexp p, Sexp a) {
        auto e = eval(a);
        cerr << "Arg: " << e << endl;
        auto flag = e->car();
        auto newState = e->cdr()->car();
        auto data = e->cdr()->cdr()->car();
        if (to_int(a->car()) == 0) {
          // modem
          cerr << "f38 -> modem" << endl;
        } else {
          //interact(p, 
          cerr << "i38 -> nteract" << endl;
        }
        return Sexp(new Nil());
      }));
}

Sexp interact() {
  return Sexp(new TriFunc(
      "interact",
      [=](Sexp p, Sexp s, Sexp v) {
        cerr << "in interact" << endl;
        return call(f38(), p, call(p, s, v));
      }));
}

Sexp VM::interact(
    const string&name,
    Sexp state,
    Sexp vec) const {
  cerr << "vm.interact" << endl;
  Sexp p = eval(protocol(name));
  return call(::interact(), p, state, vec);
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
  auto ret = f->second;
  cout << "func@: " << index << << " " << f << endl;
  return ret;
}

Sexp Function::call_(Sexp _this, Sexp arg) const{
  auto f = vm_->function(index_);
  return call(f, eval(arg));
}

Sexp Function::eval_(Sexp _this) const{
  auto f = vm_->function(index_);
  return eval(f);
}

std::string ModResult::mod() const {
  bint num = num_;
  cerr << "mod(" << num << ")" << endl;
  string s;
  if (num >= 0) {
    s = "01";
  } else {
    s = "10";
    num = -num;
  }

  int width;  
  int bits;
  if (num == 0) {
    width = 0;
  } else {
    breal num_r(num);
    breal bits_r = mp::floor(mp::log(num_r) / mp::log(breal(2.0))) + 1;
    breal width_r = (bits_r / 4) + 1;
    width = (int) width_r;
    bits = (int) bits;
    cerr << "raw bits: " << bits_r << endl;
  }

  bits = width * 4;
  cerr << "bits: " << bits << ", width: " << width << endl;
  for (int i = 0; i < width; i++) {
    s += '1';
  }
  s += '0';

  for (int i = bits - 1; i >= 0; i--) {
    if (mp::bit_test(num, i)) {
      s += '1';
    } else {
      s += '0';
    }
  }
  return s;
}
