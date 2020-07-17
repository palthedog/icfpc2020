#ifndef VM_H
#define VM_H

#include <map>

#include <types.h>

class VM {
 private:
  std::map<int, bint> vars;
};

#endif
