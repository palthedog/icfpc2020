#include <fstream>
#include <cstdlib>
#include <tuple>

#include <types.h>
#include <send.h>

#include "vm.h"

#include <plot.h>

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

Sexp send() {
  return Sexp(new UnaryFunc(
      "send",
      [](Sexp a) {
        Sexp s = eval(call(Mod(), a));
        cerr << "send: " << s << endl;
        string response = sendData("/aliens/send",s->mod());
        Sexp res = dem(response);
        cerr << "resp: " << res << endl;
        return res;
      }));
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
    return send();
  } else if (token == "c") {
    p = new TriFunc(
        "c",
        [](Sexp x0, Sexp x1, Sexp x2) {
          return ap(ap(x0, x2), x1);
        });
  } else if (token == "b") {
    p = new TriFunc(
        "b",
        [](Sexp x0, Sexp x1, Sexp x2) {
          return ap(x0, ap(x1, x2));
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
        [](Sexp a, Sexp b){
          return Sexp(new Num(to_int(a) * to_int(b)));
        });
  } else if (token == "div") {
    p = new BinaryFunc(
        "div",
        [](Sexp a, Sexp b){ return Sexp(new Num(to_int(a) / to_int(b))); });
  } else if (token == "if0") {
    return If0();
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
          Sexp e = eval(a);
          return e->isNil() ? CreateTrue() : CreateFalse() ;
        });
  } else if (token == "car") {
    return Car();
  } else if (token == "cdr") {
    return Cdr();
  } else if (token == "i") {
    p = new UnaryFunc(
        "i",
        [](Sexp a){ return eval(a) ; });
  } else if (token == "nil") {
    p = new Nil();
  } else if (token[0] == '-' || (token[0] >= '0' && token[0] <= '9')) {
    p = new Num(token);
  } else if (token == "mod") {
    return Mod();
  } else if (token == "dem") {
    return Dem();
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
    int p = line.find(" = ");
    const string l = line.substr(0, p);
    const string r = line.substr(p + 3);

    //cout << "LHS: " << l << endl;

    Sexp rhs = parse(this, r);
    //cout << "RHS: " << *rhs << endl;

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

Sexp modem() {
  return Sexp(new UnaryFunc(
      "modem",
      [](Sexp x0) {
        return ap(Dem(), eval(ap(Mod(), x0)));
      }));
}

Sexp draw() {
  return Sexp(new UnaryFunc(
      "draw",
      [](Sexp x0) {
        x0 = eval(x0);
         while (!x0->isNil()) {
           Sexp h = eval(call(Car(), x0));
           
           Sexp x = eval(call(Car(), h));
           Sexp y = eval(call(Cdr(), h));
           cerr << "DRAW(" << x << ", " << y << ")" << endl;
           if (plot) {
             plot->draw((int)to_int(x), (int)to_int(y));
           }
           
           x0 = eval(call(Cdr(), x0));
        }
        return nil();
      }));
}

Sexp multipledraw() {
  return Sexp(new UnaryFunc(
      "multipledraw",
      [](Sexp x0) {
        cerr << "multipledraw: " << x0 << endl;
        x0 = eval(x0);
        if (x0->isNil()) {
          return nil();
        }

        Sexp h = call(Car(), x0);
        Sexp drawH = call(draw(), h);

        Sexp tail = call(Cdr(), x0);
        Sexp multiDraw = eval(call(multipledraw(), tail));
        return eval(ap(ap(Cons(), drawH), multiDraw));
      }));
}

Sexp f38() {
  return Sexp(new BinaryFunc(
      "f38",
      /*
      [=](Sexp x2, Sexp x0) {
        x2 = eval(x2);
        x0 = eval(x0);
        return eval(ap(ap(ap(
            If0(),
            ap(Car(), x0)),
                          List(eval(ap(modem(), ap(Car(), ap(Cdr(), x0)))),
                          eval(ap(multipledraw(), ap(Car(), ap(Cdr(), ap(Cdr(), x0))))))),
                          ap(ap(ap(interact(), x2), ap(modem(), ap(Car(), ap(Cdr(), x0)))),
                          ap(send(), ap(Car(), ap(Cdr(), ap(Cdr(), x0)))))));
      }));
      /*/
      [=](Sexp p, Sexp a) {
        cerr << "Arg: " << a << endl;
        Sexp flag = call(Car(), a);
        Sexp newState = call(Car(), call(Cdr(), a));
        Sexp data = call(Car(), call(Cdr(), call(Cdr(), a)));

        bint intFlag = to_int(flag);
        cerr << "Flag: " << intFlag << endl;
        if (intFlag == 0) {
          // modem
          cerr << "f38 -> draw: " << data << endl;
          if (plot) {
            plot->startDraw();
          }
          Sexp drw = call(multipledraw(), data);
          if (plot) {
            plot->endDraw();
          }
          return List(eval(call(modem(), newState)), drw);
        } else {
          cerr << "f38 -> interact" << endl;
          cerr << "newState: " << str(newState) << endl;
          cerr << "data: " << str(data) << endl;
          Sexp m_newState = call(modem(), newState);
          cerr << "Newstate: " << m_newState << endl;
          Sexp response = call(send(), data);
          cerr << "Response: " << response << endl;
          return call(interact(), p, m_newState, response);
        }
      }));
  //*/
}

Sexp interact() {
  return Sexp(new TriFunc(
      "interact",
      [=](Sexp p, Sexp s, Sexp v) {
        cerr << "in interact" << endl;
        return ap(ap(f38(), p), ap(ap(p, s), v));
        //return call(f38(), p, call(p, s, v));
      }));
}

Sexp VM::interact(
    const string&name,
    Sexp state,
    Sexp vec) const {
  cerr << "vm.interact" << endl;
  Sexp p = eval(protocol(name));
  //return call(::interact(), p, state, vec);
  return ap(ap(ap(::interact(), p), state), vec);
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
  //cout << "func@: " << index << " " << str(ret) << endl;
  return ret;
}

Sexp Function::call_(Sexp _this, Sexp arg) const{
  Sexp f = vm_->function(index_);
  //cout << "func.call: " << str(f) << " " << str(arg) << endl;
  return call(f, arg);
}

Sexp Function::eval_(Sexp _this) const{
  Sexp f = vm_->function(index_);
  //cout << "func.eval: " << str(f) << endl;
  return eval(f);
}

std::string modNum(bint num) {
  // cerr << "modNum(" << num << ")" << endl;
  
  string s;
  if (num >= 0) {
    s = "01";
  } else {
    s = "10";
    num = -num;
  }

  int width = 0;  
  int bits = 0;
  if (num == 0) {
    width = 0;
  } else {
    breal num_r(num);
    breal bits_r = mp::floor(mp::log(num_r) / mp::log(breal(2.0))) + 1;
    breal width_r = (bits_r / 4) + 1;
    width = (int) width_r;
    bits = (int) bits;
    //cerr << "raw bits: " << bits_r << endl;
  }
  
  bits = width * 4;
  //cerr << "bits: " << bits << ", width: " << width << endl;
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

std::string modImpl(Sexp e) {
  e = eval(e);
  //cerr << "mod: " << e << endl;
  if (e->isNum()) {
    return modNum(to_int(e));
  }
  if (e->isNil()) {
    return "00";
  }
  string s;
  s += "11";  // cons
  s += modImpl(eval(call(Car(), e)));
  s += modImpl(eval(call(Cdr(), e)));
  return s;
}

tuple<Sexp, string> demElem(string s) {
  //cerr << "demElem: " << s << endl;

  // number
  string h = s.substr(0, 2);
  string tail = s.substr(2);

  if (h == "00") {
    // nil;
    //cerr << "nil" << endl;
    return forward_as_tuple(nil(), tail);
  }

  bool negative = h == "10";
  int width = 0;
  while (tail[width] == '1') {
    width++;
  }
  int bits = width * 4;
  //cerr << "width: " << width << ", bits: " << bits << endl;
  tail = tail.substr(width + 1); // +1 for last '0'
  //cerr << "tail: " << tail << endl;
  
  string numStr = tail.substr(0, bits);
  //cerr << "numStr: " << numStr << endl;
  
  bint n = 0;
  if (numStr != "") {
    n = decode(numStr);
  }
  if (negative) {
    n = -n;
  }
  return forward_as_tuple(num(n), tail.substr(bits));
}
  
tuple<Sexp, string> demImpl(string s) {
  //cerr << "dem: " << s << endl;

  string h = s.substr(0, 2);
  string tail;

  if (h == "11") {
    // cons
    Sexp hExp;
    tail = s.substr(2);
    tie(hExp, tail) = demImpl(tail);
    //cerr << "cons(" << hExp << " ..." << endl;

    Sexp tailExp;
    tie(tailExp, tail) = demImpl(tail);
    return forward_as_tuple(call(Cons(), hExp, tailExp), tail);
  }

  Sexp numExp;
  tie(numExp, tail) = demElem(s);
  return forward_as_tuple(numExp, tail);
}

Sexp dem(std::string s) {
  Sexp exp;
  string tail;
  tie(exp, tail) = demImpl(s);
  if (tail != "") {
    cerr << "Tail should be empty but: " << tail <<endl;
    exit(1);
  }
  //cerr << "dem result: " << exp << endl;
  return exp;
}

Sexp Dem() {
  return Sexp(new UnaryFunc(
      "dem",
      [](Sexp a) {
        Sexp exp = dem(eval(a)->mod());
        return exp;
      }));
}
