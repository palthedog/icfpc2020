#ifndef VM_H
#define VM_H

#include <string>
#include <sstream>
#include <map>

#include <memory>

#include <types.h>

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
    std::cerr << "to_int not supported: " << *this << std::endl;
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


class Protocol : public Exp {
 private:
  std::string name_;

 public:
  Protocol(const std::string& token) {
    name_ = token;
  }

  virtual ~Protocol() {}
  
  virtual void print(std::ostream&os) const {
    os << name_;
  }
};

class Line : public Exp {
 private:
  int x_;

 public:
  Line(const std::string& token) {
    x_ = std::stoi(token.substr(1));
  }

  virtual void print(std::ostream&os) const {
    os << ":" << x_;
  }
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

  virtual ~Ap() {}

  virtual void print(std::ostream&os) const {
    os << "ap";
    if (f_) {
      os << " " << f_;
    }
  }

  virtual Sexp bind(std::shared_ptr<Exp> arg) {
    if (!f_) {
      return Sexp(new Ap(arg));
    }
    return f_->bind(arg);
  }
};

class Nil : public Exp {
 public:
  Nil() {
  }

  virtual void print(std::ostream&os) const {
    os << "nil";
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

  virtual Sexp bind(Sexp arg) {
    if (!a_) {
      return Sexp(new Cons(arg));
    } else if (!b_) {
      return Sexp(new Cons(a_, arg));
    } else {
      Sexp s(new Ap(arg));
      return s->bind(a_)->bind(b_);
    }
  }

  virtual void print(std::ostream&os) const {
    if (!a_) {
      os << "Cons(?, ?)";
    } else if (!b_) {
      os << "Cons(" << *a_ << ", ?)" << std::endl;
    } else {
      os << "Cons(" << *a_ << ", " << *b_ << ")" << std::endl;
    }
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
      os << name_ << "(?, ?)";
    } else if (!b_) {
      os << name_ << "(" << a_ << ", ?)";
    } else {
      os << name_ << "(" << a_ << ", " << b_ << ")";
    }
  }

  virtual bint to_int() const {
    return body_(a_, b_)->to_int();
  }
};


Sexp parse(std::istringstream& iss, bool lhs = false);

inline Sexp parse(std::string str, bool lhs = false) {
  std::istringstream iss(str);
  return parse(iss, lhs);
}

class VM {
 public:
  VM(const std::string&path);

 private:
  std::map<int, bint> vars;
};

#endif
