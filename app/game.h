#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>

#include <sstream>
#include <iostream>

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

  bint damage() const {
    return to_int(nth(ship_, 5));
  }

  bint maxHp() const {
    return to_int(nth(ship_, 6));
  }

  bint numParts() const {
    return to_int(nth(spec(), 3));
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
        "  spec:" << spec() <<
        ", damage:" << damage() << '/' << maxHp() <<
        ", d:" << eval(nth(ship_, 7)) <<
        ")";
    return oss.str();
  }

  // Command
  Sexp accel(bint x, bint y) const {
    Sexp v = Vec(num(x), num(y));
    return List(num(0), num(shipId()), v);
  }

  Sexp shoot(bint x, bint y, bint size)const {
    Sexp v = Vec(num(x), num(y));
    return List(num(2), num(shipId()), v, num(size));
  }

  Sexp split()const {
    Sexp s = spec();
    return List(num(3),
                num(shipId()),
                List(num(to_int(nth(s, 0)) / 2),
                     num(to_int(nth(s, 1)) / 2),
                     num(to_int(nth(s, 2)) / 2),
                     num(to_int(nth(s, 3)) / 2)));
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

inline std::ostream& operator<<(std::ostream&os, const Ship&s) {
  os << s.toString();
  return os;
}

inline bool checkGame(Sexp state) {
  GameResponse game(state);
  if (!game.ok()) {
    return false;
  }
  return true;
}

#endif
