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

  V position() const {
    return toV(nth(ship_, 2));
  }

  V velocity() const {
    return toV(nth(ship_, 3));
  }

  Sexp spec() const {
    return nth(ship_, 4);
  }

  bint fuel() const {
    return to_int(car(spec()));
  }

  std::string toString() const {
    std::ostringstream oss;
    auto pos = position();
    auto vel = velocity();
    oss << "ship(" <<
        "r:" << role() <<
        ", id:" << shipId() <<
        ", pos:" << pos <<
        ", vel:" << vel <<
        ", fuel:" << fuel() <<
        "   spec:" << spec() <<
        ", b:" << eval(nth(ship_, 5)) <<
        ", c:" << eval(nth(ship_, 6)) <<
        ", d:" << eval(nth(ship_, 7)) <<
        ")";
    return oss.str();
  }


  // Command
  Sexp accel(bint x, bint y) {
    Sexp v = Vec(num(x), num(y));
    return List(num(0), num(shipId()), v);
  }

  Sexp shoot(bint x, bint y, bint size) {
    Sexp v = Vec(num(x), num(y));
    Sexp x3 = num(size);
    return List(num(2), num(shipId()), v, x3);
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

  int gameTick() const {
    return (int)to_int(nth(gameState(), 0));
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

  Ship enemyShip() const {
    for (const auto& s : ships()) {
      if (s.role() != role()) {
        return s;
      }
    }
    std::cerr << "enemy ship not found" << std::endl;
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

inline bool checkGame(Sexp state) {
  GameResponse game(state);
  if (!game.ok()) {
    return false;
  }
  return true;
}

#endif

