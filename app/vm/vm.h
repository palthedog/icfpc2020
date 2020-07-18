#ifndef VM_H
#define VM_H

#include <string>
#include <sstream>
#include <map>

#include <memory>

#include <types.h>

class VM;

class Exp;
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
    Exp* ori = e.get();
    e = e->eval_(e);
    //std::cerr << "after: " << (long) e.get() << std::endl;
    if (ori == e.get()) {
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
  Ap()
      : f_() {
  }
  
  Ap(Sexp f)
      : f_(f) {
  }

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
  
  Sexp eval_(Sexp _this) const override{
    if (f_ && arg_) {
      Sexp e = eval(call(eval(f_), arg_));
      return e;
    }

    return Exp::eval_(_this);
  }
  
  Sexp call_(Sexp _this, const Sexp arg) const override {
    if (!f_) {
      return Sexp(new Ap(arg));
    }
    //std::cerr << "ap call: " << str(_this) << std::endl;
    return Sexp(new Ap(f_, arg));
  }
};

inline Sexp ap(Sexp f, Sexp a) {
  return Sexp(new Ap(f, a));
}

inline Sexp ap(Sexp f) {
  return Sexp(new Ap(f));
}

inline Sexp ap() {
  return Sexp(new Ap());
}

class Num : public Exp {
  bint num_;

 public:
  Num(const std::string& token)
      : num_(token) {
  }

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

inline Sexp num(int n) {
  return Sexp(new Num(n));
}

/*
class Cons : public Exp {
  Sexp a_;
  Sexp b_;

 public:
  Cons() {
  }

  Cons(Sexp a)
      : a_(a) {
  }

  Cons(Sexp a, Sexp b)
      : a_(a)
      , b_(b) {
  }

  virtual ~Cons() {}

  virtual Sexp call_(Sexp _this, const Sexp arg) const override {
    if (!a_) {
      return Sexp(new Cons(arg));
    } else if (!b_) {
      return Sexp(new Cons(a_, arg));
    } else {
      return ap(ap(arg, a_), b_);
    }
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << "cons(?, ?)";
    } else if (!b_) {
      os << "cons(" << *a_ << ", ?)";
    } else {
      os << "cons(" << *a_ << ", " << *b_ << ")";
    }
  }

  virtual bool isCons() const {
    return true;
  }

  Sexp car() const override {
    return a_;
  }

  Sexp cdr() const override {
    return b_;
  }
};
*/

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

class DemResult : public Exp {
  std::string mod_;
 public:
  DemResult(std::string mod_str): mod_(mod_str) {
  }

  virtual ~DemResult() {
  }

  virtual void print(std::ostream&os) const {
    os << "dem(" << mod_ << ")";
  }

  Sexp dem() const override {
    std::cerr << "TODO: parse " << mod_ << std::endl;
    exit(1);
    return num(0);
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

inline Sexp SComb() {
  return Sexp(new TriFunc(
      "s",
      [](Sexp a, Sexp b, Sexp c) {
        Sexp ec = eval(c);
        return ap(ap(a, ec), ap(b, ec));
      }));
}

inline Sexp CreateTrue() {
  return Sexp(new BinaryFunc(
      "t",
      [](Sexp a, Sexp b){ return eval(a); }));
}

inline Sexp CreateFalse() {
  return ap(SComb(), CreateTrue());
}

class Nil : public Exp {
 public:
  Nil() {
  }

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
    return CreateTrue();
  }
};

inline Sexp nil() {
  return Sexp(new Nil());
}

inline Sexp Cons() {
  return Sexp(new TriFunc(
        "cons",
        [](Sexp x0, Sexp x1, Sexp x2) {
          return ap(ap(x2, x0), x1);
        }));
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
  return ap(ap(Cons(), x0), nil());
}

inline Sexp List(Sexp x0, Sexp x1) {
  return ap(ap(Cons(), x0), List(x1));
}

inline Sexp Car() {
  return Sexp(new UnaryFunc(
      "car",
      [](Sexp a){
        Sexp e = eval(a);
        return ap(e, CreateTrue());
      }));
}

inline Sexp Cdr() {
  return Sexp(new UnaryFunc(
      "cdr",
      [](Sexp a){
        Sexp e = eval(a);
        return ap(e, CreateFalse());
      }));
}

inline Sexp If0() {
  return Sexp(new TriFunc(
      "if0",
      [](Sexp c, Sexp a, Sexp b){
        bint e = to_int(c);
        return e == 0 ? a : b;
      }));
}

std::string modImpl(Sexp a);

inline Sexp Mod() {
  return Sexp(new UnaryFunc(
      "mod",
      [](Sexp a){
        return Sexp(new ModResult(modImpl(a)));
      }));
}

Sexp Dem();

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
