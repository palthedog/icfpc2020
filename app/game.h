#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>

#include <sstream>

#include <vm/vm.h>
#include <game.h>


class Ship {
  Sexp ship_;
 public:
  Ship(Sexp ship) : ship_(ship) {
  }
  
  bint role() const {
    return to_int(nth(ship_, 0));
  }

  bint shipId() const {
    return to_int(nth(ship_, 1));
  }

  Sexp position() const {
    return nth(ship_, 2);
  }

  Sexp velocity() const {
    return nth(ship_, 3);
  }

  std::string toString() const {
    std::ostringstream oss;
    Sexp pos = position();
    Sexp vel = velocity();
    oss << "ship(" <<
        "r:" << role() <<
        ", id:" << shipId() <<
        ", pos:" << to_int(car(pos)) << "," << to_int(cdr(pos)) <<
        ", vel:" << to_int(car(vel)) << "," << to_int(cdr(vel));
    return oss.str();
  }


  // Command
  Sexp accel(bint x, bint y) {
    Sexp v = Vec(num(x), num(y));
    return List(num(0), num(shipId()), v);
  }
};

class GameResponse {
  Sexp response_;

 public:
  GameResponse(Sexp response) : response_(response) {
  }

  bool ok() const {
    return to_int(nth(response_, 0)) == 1;
  }

  bint gameStage() const {
    return to_int(nth(response_, 1));
  }
  
  Sexp staticGameInfo() const {
    return nth(response_, 2);
  }
  
  Sexp gameState() const {
    return nth(response_, 3);
  }

  bint role() const {
    return to_int(nth(staticGameInfo(), 1));
  }

  Sexp gameTick() const {
    return nth(gameState(), 0);
  }

  Sexp shipsAndCommands() const {
    return nth(gameState(), 2);
  }

  Ship myShip() const {
    for (const auto& s : ships()) {
      if (s.role() == role()) {
        return s;
      }
    }
    std::cerr << "my ship not found" << std::endl;
    exit(1);
    return Ship(NIL);
  }
  
  std::vector<Ship> ships() const {
    Sexp l = eval(shipsAndCommands());
    std::vector<Ship> ships;
    while (!eval(l)->isNil()) {    
      Sexp shipAndCommand = car(l);
      ships.push_back(Ship(nth(shipAndCommand, 0)));
      l = cdr(l);
    }
    return ships;
  }
};

bool checkGame(Sexp state) {
  GameResponse game(state);
  if (!game.ok()) {
    return false;
  }
  return true;
}

#endif

