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

class Exp {
 public:
  virtual ~Exp() {}
  
  virtual Sexp bind(std::shared_ptr<Exp> a) {
    std::cerr << "Bind not supported: " << *this << ".bind(" << *a <<  ")" << std::endl;
    exit(1);
  }

  virtual bint to_int() const {
    return this->eval()->to_int();
  }

  virtual Sexp eval() const {
    std::cerr << "eval not supported: " << *this << std::endl;
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
  int x_;
  VM* vm_;

 public:
  Function(VM*vm, const std::string& token) {
    vm_ = vm;
    x_ = std::stoi(token.substr(1));
  }

  virtual void print(std::ostream&os) const {
    os << ":" << x_;
  }

  virtual Sexp eval() const;
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

  virtual ~Ap() {}

  virtual void print(std::ostream&os) const {
    os << "ap";
    if (f_) {
      os << "(" << f_ << ")";
    }
    if (arg_) {
      os << "(" << arg_ << ")";
    }
  }

  virtual Sexp eval() const {
    if (f_ && arg_) {
      return f_->bind(arg_)->eval();
    }
    if (f_) {
      return Sexp(new Ap(f_));
    }
    return Sexp(new Ap());
  }

  virtual Sexp bind(std::shared_ptr<Exp> arg) {
    if (!f_) {
      return Sexp(new Ap(arg));
    }
    if (!arg_) {
      return Sexp(new Ap(f_, arg));
    }
    return Exp::bind(arg);
  }
};

inline Sexp ap(Sexp f, Sexp a) {
  return Sexp(new Ap(f, a));
}

class Nil : public Exp {
 public:
  Nil() {
  }

  virtual void print(std::ostream&os) const {
    os << "nil";
  }

  virtual Sexp eval() const {
    return Sexp(new Nil());
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

  virtual Sexp eval() const {
    return Sexp(new Num(num_));
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

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new Cons(arg));
    } else if (!b_) {
      return Sexp(new Cons(a_, arg));
    } else {
      Sexp s(new Ap(arg, a_));
      return Sexp(new Ap(s, b_));
    }
  }

  virtual Sexp eval() const {
    Sexp a, b;
    if (a_) {
      a = a_->eval();
    }
    if (b_) {
      b = b_->eval();
    }
    return Sexp(new Cons(a, b));
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

class CComb : public Exp {
  Sexp a_;
  Sexp b_;
  Sexp c_;

 public:
  CComb() {
  }

  CComb(Sexp a)
      : a_(a) {
  }

  CComb(Sexp a, Sexp b)
      : a_(a)
      , b_(b) {
  }

  CComb(Sexp a, Sexp b, Sexp c)
      : a_(a)
      , b_(b)
      , c_(c){
  }

  virtual ~CComb() {}

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new CComb(arg));
    } else if (!b_) {
      return Sexp(new CComb(a_, arg));
    } else if (!c_) {
      return Sexp(new CComb(a_, b_, arg));
    } else {
      return Exp::bind(arg);
    }
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << "CComb";
    } else if (!b_) {
      os << "CComb(" << *a_ << ")";
    } else if (!b_) {
      os << "CComb(" << *a_ << b_ << ")";
    } else {
      os << "CComb(" << *a_ << ", " << *b_ << ", " << *c_ << ")";
    }
  }

  virtual Sexp eval() const {
    if (a_ && b_ && c_) {
      return Sexp(new Ap(Sexp(new Ap(a_, c_)), b_))->eval();
    }
    return Sexp(new CComb(a_, b_, c_));
  }
};

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

class BinaryFunc : public Exp {
 public:
  std::string name_;
  std::function<Sexp(Sexp, Sexp)> body_;
  Sexp a_;
  Sexp b_;

  BinaryFunc(std::string name, std::function<Sexp(Sexp, Sexp)> body)
      : name_(name)
      , body_(body){}

  BinaryFunc(std::string name,
             std::function<Sexp(Sexp, Sexp)> body,
             Sexp a)
      : name_(name)
      , body_(body)
      , a_(a) {
  }

  BinaryFunc(std::string name,
             std::function<Sexp(Sexp, Sexp)> body,
             Sexp a,
             Sexp b)
      : name_(name)
      , body_(body)
      , a_(a)
      , b_(b){
  }

  virtual ~BinaryFunc() {}

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new BinaryFunc(name_, body_, arg));
    } else if (!b_) {
      return Sexp(new BinaryFunc(name_, body_, a_, arg));
    }
    return Exp::bind(arg);
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << name_ << "()";
    } else if (!b_) {
      os << name_ << "(" << a_ << ")";
    } else {
      os << name_ << "(" << a_ << ", " << b_ << ")";
    }
  }

  virtual Sexp eval() const {
    if (a_ && b_) {
      return body_(a_, b_)->eval();
    }
    if (a_) {
      return Sexp(new BinaryFunc(name_, body_, a_));
    }
    return Sexp(new BinaryFunc(name_, body_));
  }

};

inline Sexp CreateTrue() {
  return Sexp(new BinaryFunc(
      "t",
      [](Sexp a, Sexp b){ return a->eval(); }));
}

inline Sexp CreateFalse() {
  return ap(Sexp(new SComb()), CreateTrue());
}

class UnaryFunc : public Exp {
 public:
  std::string name_;
  std::function<Sexp(Sexp)> body_;
  Sexp a_;

  UnaryFunc(std::string name, std::function<Sexp(Sexp)> body)
      : name_(name)
      , body_(body){}

  UnaryFunc(std::string name,
            std::function<Sexp(Sexp)> body,
            Sexp a)
      : name_(name)
      , body_(body)
      , a_(a) {
  }

  virtual ~UnaryFunc() {}

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new UnaryFunc(name_, body_, arg));
    }
    return Exp::bind(arg);
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << name_ << "(?)";
    } else {
      os << name_ << "(" << a_ << ")";
    }
  }

  virtual Sexp eval() const {
    if (a_) {
      Sexp a = a_->eval();
      return body_(a)->eval();
    }
    return Sexp(new UnaryFunc(name_, body_));
  }
};


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
