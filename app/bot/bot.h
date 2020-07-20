#ifndef BOT_H
#define BOT_H

#include <vm/vm.h>
#include <game.h>

class Bot {
  Sexp playerKey_;

 public:
  Bot(Sexp playerKey)
      : playerKey_(playerKey) {
  }

  Sexp commands(GameResponse game);
};

#endif
