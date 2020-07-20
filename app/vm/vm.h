#ifndef VM_H
#define VM_H

#include <string>
#include <sstream>
#include <map>
#include <tuple>
#include <vector>
#include <cmath>

#include <memory>

#include <types.h>

class VM;

class Exp;
// typedef Exp* Sexp;
typedef std::shared_ptr<Exp> Sexp;

std::string str(const Sexp e);
std::string str(const Exp* e);

Sexp ap(Sexp f, Sexp a);

class Exp {
 public:
  virtual ~Exp() {}
  
  virtual Sexp call_(Sexp _this, const Sexp a) const {
    std::cerr << "call not supported: " << *this << "; arg: " << *a << std::endl;
    exit(1);
  }
  
  virtual Sexp eval_(Sexp _this) const {
    return _this;
  }

  virtual bint to_int_() const {
    std::cerr << "to_int not supported: " << *this << std::endl;
    exit(1);
  }

  virtual bool isNum() const {
    return false;
  }

  virtual bool isNil() const {
    return false;
  }

  virtual std::string mod() const {
    std::cerr << "mod not supported: " << *this << std::endl;
    exit(1);
  }

  virtual Sexp dem() const {
    std::cerr << "dem not supported: " << *this << std::endl;
    exit(1);
  }
  
  virtual void print(std::ostream&os) const = 0;

  friend std::ostream& operator<<(std::ostream&os, const Exp&e);
};

inline Sexp eval(Sexp e) {
  //std::cerr << "eval: " << str(e) << std::endl;
  while (!e->isNum()) {
    Exp* ori = &*e;
    e = e->eval_(e);
    //std::cerr << "after: " << (long) e.get() << std::endl;
    if (ori == &*e) {
      // not updated
      break;
    }
  }
  e = e->eval_(e);
  return e;
}

inline Sexp call(Sexp e, Sexp arg) {
  //std::cout << "call: " << str(e) << std::endl;
  return e->call_(e, arg);
}

inline Sexp call(Sexp e, Sexp a0, Sexp a1) {
  return call(call(e, a0), a1);
}

inline Sexp call(Sexp e, Sexp a0, Sexp a1, Sexp a2) {
  return call(call(call(e, a0), a1), a2);
}

inline bint to_int(Sexp e) {
  e = eval(e);
  return e->to_int_();
}

inline std::ostream& operator<<(std::ostream&os, const Exp&e) {
  e.print(os);
  return os;
}
inline std::ostream& operator<<(std::ostream&os, const Sexp& e) {
  (*e).print(os);
  return os;
}

class Function : public Exp {
 private:
  int index_;
  VM* vm_;

 public:
  Function(VM*vm, const std::string& token) {
    vm_ = vm;
    index_ = std::stoi(token.substr(1));
  }

  void print(std::ostream&os) const override {
    os << ":" << index_;
  }

  Sexp call_(Sexp _this, Sexp arg) const override;
  Sexp eval_(Sexp _this) const override;
};

class Ap : public Exp {
  Sexp f_;
  Sexp arg_;
  
 public:
  Ap(Sexp f, Sexp arg)
      : f_(f)
      , arg_(arg) {
  }
  
  virtual ~Ap() {
    //std::cerr << "ap destroyed: " << str(this) << std::endl;
  }

  void print(std::ostream&os) const override {
    os << "(ap ";
    if (f_) {
      os << f_ << ",";
    }
    if (arg_) {
      os << arg_;
    }
    os  << ")";
  }

  mutable Sexp cache_;
  
  Sexp eval_(Sexp _this) const override {
    if (cache_) {
      return cache_;
    }

    cache_ = eval(call(eval(f_), arg_));
    return cache_;
    return eval(call(eval(f_), arg_));
  }
};

extern std::map<std::pair<Exp*, Exp*>, Sexp> globalCache_;

inline Sexp ap(Sexp func, Sexp a) {
  auto key = std::make_pair(func.get(), a.get());
  auto f = globalCache_.find(key);
  if (f != globalCache_.end()) {
    return f->second;
  }

  if (globalCache_.size() > 100000) {
    globalCache_.clear();
  }
  
  Sexp node(new Ap(func, a));
  globalCache_[key] = node;
  return node;
}

class Num : public Exp {
  bint num_;

 public:
  Num(bint num)
      : num_(num) {
  }

  virtual ~Num() {
  }
  
  virtual void print(std::ostream&os) const {
    os << num_;
  }

  bint to_int_() const override {
    return num_;
  }

  bool isNum() const override {
    return true;
  }
};

extern Sexp num(bint n);

class ModResult : public Exp {
  std::string mod_;
 public:
  ModResult(std::string mod): mod_(mod) {
  }

  virtual ~ModResult() {
  }

  virtual void print(std::ostream&os) const {
    os << "mod(" << mod_ << ")";
  }

  std::string mod() const override {
    return mod_;
  }
};

class UnaryFunc : public Exp {
 public:
  std::string name_;
  std::function<Sexp(Sexp)> body_;

  UnaryFunc(std::string name, std::function<Sexp(Sexp)> body)
      : name_(name)
      , body_(body){}

  virtual ~UnaryFunc() {
    //std::cerr << "uni destroyed: " << name_ << std::endl;
  }

  virtual void print(std::ostream&os) const {
    os << name_;
  }

  Sexp call_(Sexp _this, const Sexp arg) const override {
    //std::cerr << "uni call: " << str(this) << std::endl;
    return body_(arg);
  }
};

class BinaryFunc : public Exp {
 public:
  std::string name_;
  std::function<Sexp(Sexp, Sexp)> body_;

  BinaryFunc(std::string name, std::function<Sexp(Sexp, Sexp)> body)
      : name_(name)
      , body_(body){}

  virtual ~BinaryFunc() {
    //std::cerr << "bin destroyed: " << name_ << std::endl;
  }

  virtual void print(std::ostream&os) const {
    os << name_;
  }

  Sexp call_(Sexp _this, const Sexp arg) const override {
    //std::cerr << "bin call: " << str(this) << "(" << str(arg) << ")" << std::endl;
    
    std::string name = name_;
    return Sexp(
        new UnaryFunc(
            name_ + "(" + str(arg) + ")",
            [=](Sexp a) {
              *&_this; // capture this
              return body_(arg, a);
            }));
  }
};

class TriFunc : public Exp {
  friend class TriToBin;
 public:
  std::string name_;
  std::function<Sexp(Sexp, Sexp, Sexp)> body_;

  TriFunc(std::string name,
          std::function<Sexp(Sexp, Sexp, Sexp)> body)
      : name_(name)
      , body_(body){
  }

  virtual ~TriFunc() {
  }

  void print(std::ostream&os) const override {
    os << name_;
  }

  Sexp call_(Sexp _this, const Sexp arg) const override {
    //std::cerr << "tri call: " << str(this) << std::endl;
    
    std::string name = name_;
    return Sexp(
        new BinaryFunc(
            name_ + "(" + str(arg) + ")",
            [=](Sexp a, Sexp b) {
              *&_this; // capture this
              Sexp ret = body_(arg, a, b);
              return ret;
            }));
  }
};

extern Sexp TRUE;
extern Sexp FALSE;

class Nil : public Exp {
  Nil() {
  }

 public:
  static Sexp INSTANCE;
  
  virtual ~Nil() {
  }

  virtual void print(std::ostream&os) const {
    os << "nil";
  }

  virtual bool isNil() const {
    return true;
  }

  Sexp call_(Sexp _this, const Sexp arg) const override {  
    //std::cerr << "nil(x0)" << std::endl;
    return TRUE;
  }
};

extern Sexp NIL;
extern Sexp CONS;
extern Sexp SEND;
  
inline Sexp nil() {
  return NIL;
}
inline Sexp Cons() {
  return CONS;
}

inline Sexp Cons(Sexp x0, Sexp x1) {
  return ap(ap(Cons(), x0), x1);
}

inline Sexp Vec(Sexp x0, Sexp x1) {
  return Cons(x0, x1);
}

inline Sexp List() {
  return nil();
}

inline Sexp List(Sexp x0) {
  return Cons(x0, nil());
}

inline Sexp List(Sexp x0, Sexp x1) {
  return ap(ap(Cons(), x0), List(x1));
}

inline Sexp List(Sexp x0, Sexp x1, Sexp x2) {
  return ap(ap(Cons(), x0), List(x1, x2));
}

inline Sexp List(Sexp x0, Sexp x1, Sexp x2, Sexp x3) {
  return ap(ap(Cons(), x0), List(x1, x2, x3));
}

inline Sexp List(Sexp x0, Sexp x1, Sexp x2, Sexp x3, Sexp x4) {
  return Cons(x0, List(x1, x2, x3, x4));
}

inline Sexp List(Sexp x0, Sexp x1, Sexp x2, Sexp x3, Sexp x4, Sexp x5) {
  return Cons(x0, List(x1, x2, x3, x4, x5));
}


extern Sexp CAR;
inline Sexp Car() {
  return CAR;
}
extern Sexp CDR;
inline Sexp Cdr() {
  return CDR;
}


inline Sexp car(Sexp a) {
  return call(CAR, a);
}

inline Sexp cdr(Sexp a) {
  return call(CDR, a);
}

inline Sexp nth(Sexp a, int n) {
  if (n == 0) {
    return eval(car(a));
  }
  return nth(cdr(a), n-1);
}

inline Sexp cadr(Sexp a) {
  return car(cdr(a));
}

inline Sexp caddr(Sexp a) {
  return car(cdr(cdr(a)));
}

inline std::pair<bint, bint> VecToPair(Sexp v) {
  return make_pair(to_int(car(v)), to_int(cdr(v)));
}

struct V {
  bint x;
  bint y;
  V(bint a, bint b) : x(a), y(b) {}

  bint dist(const V& other) const {
    bint dx = x - other.x;
    bint dy = x - other.x;
    return abs(dx) + abs(dy);
  }

  double angle(const V& other) const {
    bint a = (x * other.x + y * other.y);
    double length = len() * other.len();
    if (length == 0) {
      return 0.0;
    }
    double cs = ((double) a) / (double) length;
    return acos(cs);
  }

  double len() const {
    bint n = (x * x + y * y);
    return std::sqrt((double) n);
  }

  double cross(const V& other) {
    return x * other.y - y * other.x;
  }
  
  V norm() const {
    if (abs(x) > abs(y)) {
      return V(x > 0 ? 1 : -1, 0);
    }
    return V(0, y > 0 ? 1 : -1);
  }

  V norm2() const {
    double ln = len();
    return V((bint) round(((double) x) / ln), (bint) round(((double) y) / ln));
  }

  friend std::ostream& operator<<(std::ostream&os, const V&e);

  V operator-() const {
    return V(-x, -y);
  }
};
inline std::ostream& operator<<(std::ostream&os, const V&e) {
  os << "[" << e.x << "," << e.y << "]";
  return os;
}

inline V operator-(const V&a, const V&b) {
  return V(a.x - b.x, a.y - b.y);
}
inline V operator+(const V&a, const V&b) {
  return V(a.x + b.x, a.y + b.y);
}


inline V toV(Sexp v) {
  return V(to_int(car(v)), to_int(cdr(v)));
}

extern Sexp IF0;
inline Sexp If0() {
  return IF0;
}

std::string modImpl(Sexp a);
std::string modNum(bint num);
std::string modNumOld(bint num);

extern Sexp MOD;
inline Sexp Mod() {
  return MOD;
}

extern Sexp DEM;
inline Sexp Dem() {
  return DEM;
}

Sexp dem(std::string s);
extern Sexp MODEM;
inline Sexp modem() {
  return MODEM;
}

std::tuple<Sexp, std::string> demImpl(std::string s);
Sexp multipledraw();

Sexp parse(VM*vm, std::istringstream& iss);

inline Sexp parse(VM*vm, std::string str) {
  std::istringstream iss(str);
  return parse(vm, iss);
}

class VM {
 public:
  VM(const std::string&path);

  Sexp interact(const std::string&name, Sexp state, Sexp vec) const;  
  Sexp protocol(const std::string&name) const;
  Sexp function(int index) const;

 private:

  std::map<int, Sexp> functions_;
  std::map<std::string, Sexp> protocols_;
};

#endif
