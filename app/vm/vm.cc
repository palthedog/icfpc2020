#include <fstream>
#include <cstdlib>
#include <tuple>

#include <types.h>
#include <send.h>

#include "vm.h"

#include <plot.h>

namespace mp = boost::multiprecision;

using namespace std;

std::map<std::pair<Exp*, Exp*>, Sexp> globalCache_;

map<bint, Sexp> numMap;
Sexp num(bint num) {
  if (num < -1024 && num > 1024) {
    return Sexp(new Num(num));
  }
  auto f = numMap.find(num);
  if (f != numMap.end()) {
    return f->second;
  }

  Sexp n(new Num(num));  
  numMap[num] = n;
  return n;
}

Sexp Nil::INSTANCE(new Nil());

Sexp NIL(Nil::INSTANCE);
Sexp CONS(new TriFunc(
    "cons",
    [](Sexp x0, Sexp x1, Sexp x2) {
      x0 = eval(x0);
      x1 = eval(x1);
      x2 = eval(x2);
      return ap(ap(x2, x0), x1);
    }));

Sexp CAR(new UnaryFunc(
    "car",
    [](Sexp a){
      Sexp e = eval(a);
      return ap(e, TRUE);
    }));
Sexp CDR(new UnaryFunc(
      "cdr",
      [](Sexp a){
        Sexp e = eval(a);
        return ap(e, FALSE);
      }));
Sexp IF0(new TriFunc(
      "if0",
      [](Sexp c, Sexp a, Sexp b){
        bint e = to_int(c);
        if (e == 0) {
          return eval(a);
        } else {
          return eval(b);
        }
      }));
Sexp C(new TriFunc(
    "c",
    [](Sexp x0, Sexp x1, Sexp x2) {
      return ap(ap(x0, x2), x1);
    }));
Sexp B(new TriFunc(
    "b",
    [](Sexp x0, Sexp x1, Sexp x2) {
      return ap(x0, ap(x1, x2));
    }));
Sexp ADD(new BinaryFunc(
        "add",
        [](Sexp a, Sexp b){
          return Sexp(num(to_int(a) + to_int(b)));
        }));
Sexp EQ(new BinaryFunc(
    "eq",
    [](Sexp a, Sexp b){
      //cout << "eq: " << a << ", " << b << endl;
      return (to_int(a) == to_int(b)) ? TRUE : FALSE;
    }));
Sexp LT(new BinaryFunc(
    "lt",
    [](Sexp a, Sexp b){
      return (to_int(a) < to_int(b)) ? TRUE : FALSE;
    }));
Sexp MUL(new BinaryFunc(
    "mul",
    [](Sexp a, Sexp b){
      return Sexp(num(to_int(a) * to_int(b)));
    }));
Sexp DIV(new BinaryFunc(
    "div",
    [](Sexp a, Sexp b){ return Sexp(num(to_int(a) / to_int(b))); }));

Sexp INC(new UnaryFunc(
    "inc",
    [](Sexp a){ return Sexp(num(to_int(a) + 1)); }));;

Sexp DEC(new UnaryFunc(
        "dec",
        [](Sexp a){ return Sexp(num(to_int(a) - 1)); }));

Sexp NEG(new UnaryFunc(
        "neg",
        [](Sexp a){ return Sexp(num(-to_int(a))); }));

Sexp ISNIL(new UnaryFunc(
    "isnil",
    [](Sexp a){
      Sexp e = eval(a);
      return e->isNil() ? TRUE : FALSE ;
    }));

Sexp I(new UnaryFunc(
    "i",
    [](Sexp a){ return eval(a) ; }));
Sexp S(new TriFunc(
    "s",
    [](Sexp a, Sexp b, Sexp c) {
      Sexp ec = eval(c);
      return ap(ap(a, ec), ap(b, ec));
    }));

Sexp MODEM(new UnaryFunc(
    "modem",
    [](Sexp x0) {
      return ap(Dem(), eval(ap(Mod(), x0)));
    }));

Sexp MOD(new UnaryFunc(
    "mod",
    [](Sexp a){
      return Sexp(new ModResult(modImpl(a)));
    }));
Sexp DEM(new UnaryFunc(
    "dem",
    [](Sexp a) {
      Sexp exp = dem(eval(a)->mod());
      return exp;
    }));

Sexp TRUE(new BinaryFunc(
    "t",
    [](Sexp a, Sexp b){ return eval(a); }));

//Sexp FALSE(ap(S, TRUE));
Sexp FALSE(new BinaryFunc(
    "f",
    [](Sexp a, Sexp b){ return eval(b); }));

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

Sexp SEND(new UnaryFunc(
      "send",
      [](Sexp a) {
        Sexp s = eval(call(Mod(), a));
        cerr << "send: " << s << endl;
        string response = sendData("/aliens/send",s->mod());
        Sexp res = dem(response);
        cerr << "resp: " << res << endl;
        return res;
      }));
Sexp send() {
  return SEND;
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
    Sexp arg = parse(vm, iss);
    if (!f || !arg) {
      cerr << "Invalid format. ap is used sololy" << endl;
      exit(1);
    }
    return ap(f, arg);
  } else if (token == "send") {
    return send();
  } else if (token == "c") {
    return C;
  } else if (token == "b") {
    return B;
  } else if (token == "s") {
    return S;
  } else if (token == "cons") {
    return CONS;
  } else if (token == "add") {
    return ADD;
  } else if (token == "eq") {
    return EQ;
  } else if (token == "lt") {
    return LT;
  } else if (token == "t") {
    return TRUE;
  } else if (token == "f") {
    return FALSE;
  } else if (token == "mul") {
    return MUL;
  } else if (token == "div") {
    return DIV;
  } else if (token == "if0") {
    return If0();
  } else if (token == "inc") {
    return INC;
  } else if (token == "dec") {
    return DEC;
  } else if (token == "neg") {
    return NEG;
  } else if (token == "isnil") {
    return ISNIL;
  } else if (token == "car") {
    return CAR;
  } else if (token == "cdr") {
    return CDR;
  } else if (token == "i") {
    return I;
  } else if (token == "nil") {
    return NIL;
  } else if (token[0] == '-' || (token[0] >= '0' && token[0] <= '9')) {
    return num(bint(token));
  } else if (token == "mod") {
    return MOD;
  } else if (token == "dem") {
    return Dem();
  } else {
    cerr << "Unknown: " << token << endl;
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

Sexp DRAW(new UnaryFunc(
      "draw",
      [](Sexp x0) {
        if (plot) {
          plot->startDraw();
        }

        x0 = eval(x0);
        while (!x0->isNil()) {
          Sexp h = eval(call(Car(), x0));
          
          Sexp x = eval(call(Car(), h));
          Sexp y = eval(call(Cdr(), h));
          if (plot) {
            plot->draw((int)to_int(x), (int)to_int(y));
          }
          
          x0 = eval(call(Cdr(), x0));
        }
        if (plot) {
          plot->endDraw();
        }
        return nil();
      }));
inline Sexp draw() {
  return DRAW;
}

Sexp MULTIPLEDRAW(new UnaryFunc(
    "multipledraw",
    [](Sexp x0) {
      //cerr << "multipledraw: " << x0 << endl;
      x0 = eval(x0);
      //cerr << "md evaled: " << x0 << endl;
      
      if (x0->isNil()) {
        return nil();
      }
      return eval(ap(ap(Cons(), call(draw(), ap(Car(), x0))), call(multipledraw(), ap(Cdr(), x0))));
    }));

inline Sexp multipledraw() {
  return MULTIPLEDRAW;
}

Sexp F38(new BinaryFunc(
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
      [](Sexp p, Sexp a) {
        //cerr << "Arg: " << a << endl;
        Sexp flag = call(Car(), a);
        Sexp newState = call(Car(), call(Cdr(), a));
        Sexp data = call(Car(), call(Cdr(), call(Cdr(), a)));

        bint intFlag = to_int(flag);
        cerr << "Flag: " << intFlag << endl;
        if (intFlag == 0) {
          // modem
          cerr << "f38 -> draw: " << data << endl;
          Sexp drw = call(multipledraw(), data);
          //cerr << "Eval New State: " << newState << endl;          
          Sexp ns = eval(call(modem(), newState));
          cerr << "New State: " << ns << endl;
          return List(ns, drw);
        } else {
          //cerr << "f38 -> interact" << endl;
          cerr << "NewState: " << str(newState) << endl;
          //cerr << "data: " << str(data) << endl;
          Sexp m_newState = call(modem(), newState);
          //cerr << "Newstate: " << m_newState << endl;
          Sexp response = call(send(), data);
          //cerr << "Response: " << response << endl;
          return call(interact(), p, m_newState, response);
        }
      }));

inline Sexp f38() {
  return F38;
}

Sexp INTERACT(new TriFunc(
      "interact",
      [](Sexp p, Sexp s, Sexp v) {
        //cerr << "in interact" << endl;
        return ap(ap(f38(), p), ap(ap(p, s), v));
        //return call(f38(), p, call(p, s, v));
      }));

inline Sexp interact() {
  return INTERACT;
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
