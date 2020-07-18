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

class Exp {
 public:
  virtual ~Exp() {}
  
  virtual Sexp call(Sexp _this, const Sexp a) const {
    std::cerr << "call not supported: " << *this << "; arg: " << *a << std::endl;
    exit(1);
  }

  virtual bint to_int() const {
    std::cerr << "to_int not supported: " << *this << std::endl;
    exit(1);
  }

  virtual bool isNil() const {
    return false;
  }

  virtual bool isCons() const {
    return false;
  }

  virtual Sexp car() const {
    std::cerr << "car not supported: " << *this << std::endl;
    exit(1);
  }
  
  virtual Sexp cdr() const {
    std::cerr << "cdr not supported: " << *this << std::endl;
    exit(1);
  }
  
  virtual void print(std::ostream&os) const = 0;

  friend std::ostream& operator<<(std::ostream&os, const Exp&e);
};

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

  Sexp call(Sexp _this, Sexp arg) const override;
};

class Ap : public Exp {
  Sexp f_;
  
 public:
  Ap()
      : f_() {
  }
  
  Ap(Sexp f)
      : f_(f) {
  }
  
  virtual ~Ap() {
    std::cerr << "ap destroyed: " << str(this) << std::endl;
  }

  void print(std::ostream&os) const override {
    os << "ap";
    if (f_) {
      os << "(" << f_ << ", )";
    }
  }

  Sexp call(Sexp _this, const Sexp arg) const override {
    if (!f_) {
      return Sexp(new Ap(arg));
    }
    std::cerr << "call: " << f_ << std::endl;
    return f_->call(f_, arg);
  }
};

inline Sexp ap(Sexp f, Sexp a) {
  std::cout << "ap: " << str(f) << ", " << str(a) << std::endl;
  return f->call(f, a);
}

inline Sexp ap(Sexp f) {
  return Sexp(new Ap(f));
}

inline Sexp ap() {
  return Sexp(new Ap());
}

class Nil : public Exp {
 public:
  Nil() {
  }

  virtual void print(std::ostream&os) const {
    os << "nil";
  }

  virtual bool isNil() const {
    return true;
  }
};

class Num : public Exp {
  bint num_;

 public:
  Num(const std::string& token)
      : num_(token) {
  }

  Num(bint num)
      : num_(num) {
  }

  virtual void print(std::ostream&os) const {
    os << num_;
  }

  virtual bint to_int() const {
    return num_;
  }
};

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

  virtual Sexp call(Sexp _this, const Sexp arg) const override {
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
      os << "Cons(?, ?)";
    } else if (!b_) {
      os << "Cons(" << *a_ << ", ?)";
    } else {
      os << "Cons(" << *a_ << ", " << *b_ << ")";
    }
  }

  virtual bool isCons() const {
    return true;
  }

  virtual Sexp car() const {
    return a_;
  }

  virtual Sexp cdr() const {
    return b_;
  }
};

/*
class BComb : public Exp {
  Sexp a_;
  Sexp b_;
  Sexp c_;

 public:
  BComb() {
  }

  BComb(Sexp a)
      : a_(a) {
  }

  BComb(Sexp a, Sexp b)
      : a_(a)
      , b_(b) {
  }

  BComb(Sexp a, Sexp b, Sexp c)
      : a_(a)
      , b_(b)
      , c_(c){
  }

  virtual ~BComb() {}

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new BComb(arg));
    } else if (!b_) {
      return Sexp(new BComb(a_, arg));
    } else if (!c_) {
      return Sexp(new BComb(a_, b_, arg));
    } else {
      return Exp::bind(arg);
    }
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << "BComb";
    } else if (!b_) {
      os << "BComb(" << *a_ << ")";
    } else if (!b_) {
      os << "BComb(" << *a_ << b_ << ")";
    } else {
      os << "BComb(" << *a_ << ", " << *b_ << ", " << *c_ << ")";
    }
  }

  virtual Sexp eval() const {
    Sexp a, b, c;
    if (a_) {
      a = a_->eval();
    }
    if (b_) {
      b = b_->eval();
    }
    if (c_) {
      c = c_->eval();
    }

    if (a && b && c) {
      return ap(a_->eval(), ap(b_->eval(), c_->eval()))->eval();
    }

    return Sexp(new BComb(a, b, c));
  }
};

class SComb : public Exp {
  Sexp a_;
  Sexp b_;
  Sexp c_;

 public:
  SComb() {
  }

  SComb(Sexp a)
      : a_(a) {
  }

  SComb(Sexp a, Sexp b)
      : a_(a)
      , b_(b) {
  }

  SComb(Sexp a, Sexp b, Sexp c)
      : a_(a)
      , b_(b)
      , c_(c){
  }

  virtual ~SComb() {}

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new SComb(arg));
    } else if (!b_) {
      return Sexp(new SComb(a_, arg));
    } else if (!c_) {
      return Sexp(new SComb(a_, b_, arg));
    } else {
      return Exp::bind(arg);
    }
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << "SComb";
    } else if (!b_) {
      os << "SComb(" << *a_ << ")";
    } else if (!b_) {
      os << "SComb(" << *a_ << b_ << ")";
    } else {
      os << "SComb(" << *a_ << ", " << *b_ << ", " << *c_ << ")";
    }
  }

  virtual Sexp eval() const {
    if (a_ && b_ && c_) {
      Sexp l = ap(a_, c_);
      Sexp r = ap(b_, c_);
      return ap(l, r);
    }
    return Sexp(new SComb(a_, b_, c_));
  }
};
*/

class UnaryFunc : public Exp {
 public:
  std::string name_;
  std::function<Sexp(Sexp)> body_;

  UnaryFunc(std::string name, std::function<Sexp(Sexp)> body)
      : name_(name)
      , body_(body){}

  virtual ~UnaryFunc() {
    std::cerr << "uni destroyed: " << name_ << std::endl;
  }

  virtual void print(std::ostream&os) const {
    os << name_;
  }

  Sexp call(Sexp _this, const Sexp arg) const override {
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
    std::cerr << "bin destroyed: " << name_ << std::endl;
  }

  virtual void print(std::ostream&os) const {
    os << name_;
  }

  Sexp call(Sexp _this, const Sexp arg) const override {
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

  Sexp call(Sexp _this, const Sexp arg) const override {
    std::string name = name_;
    return Sexp(
        new BinaryFunc(
            name_ + "(" + str(arg) + ")",
            [=](Sexp a, Sexp b) {
              *&_this; // capture this
              auto ret = body_(arg, a, b);
              return ret;
            }));
  }
};

inline Sexp SComb() {
  return Sexp(new TriFunc(
      "s",
      [](Sexp a, Sexp b, Sexp c) {
        return ap(ap(a, c), ap(b, c));
      }));
}

inline Sexp CreateTrue() {
  return Sexp(new BinaryFunc(
      "t",
      [](Sexp a, Sexp b){ return a; }));
}

inline Sexp CreateFalse() {
  return ap(SComb(), CreateTrue());
}

Sexp parse(VM*vm, std::istringstream& iss);

inline Sexp parse(VM*vm, std::string str) {
  std::istringstream iss(str);
  return parse(vm, iss);
}

class VM {
 public:
  VM(const std::string&path);

  Sexp protocol(const std::string&name) const;
  Sexp function(int index) const;
 private:
  std::map<int, Sexp> functions_;
  std::map<std::string, Sexp> protocols_;
};

#endif
