#include "bot.h"

#include <map>
#include <iostream>

using namespace std;

V nextGravity(const Ship&s) {
  return -s.position().norm();
}

V targetV(GameResponse game, const Ship& ship, bool& close) {
  auto pos = ship.position();  
  
  bint dx = -pos.x;
  bint dy = -pos.y;

  bint cos90 = 0;
  bint sin90 = 1;
  bint cos_90 = 0;
  bint sin_90 = -1;

  bint tvx0 = cos90 * dx - sin90 * dy;
  bint tvy0 = sin90 * dx + cos90 * dy;
  bint tvx1 = cos_90 * dx - sin_90 * dy;
  bint tvy1 = sin_90 * dx + cos_90 * dy;
  V tv0(tvx0, tvy0);
  V tv1(tvx1, tvy1);


  V g = nextGravity(ship);
  
  V v = ship.velocity() + g;
  
  if (v.len() == 0) {
    vector<V> candidates = {
      V(1, 0),
      V(-1, 0),
      V(0, 1),
      V(0, -1),
    };
    int bextI = 0;
    double bestAngle = 2.0;
    for (int i = 0; i < 4; i++) {
      V c = candidates[i];
      if ((c + g).len() == 0) {
        continue;
      }
      double a0 = c.angle(tv0);
      double a1 = c.angle(tv1);
      if (bestAngle > a0) {
        bestAngle = a0;
        bextI = i;
      }
      if (bestAngle > a1) {
        bestAngle = a1;
        bextI = i;
      }
    }
    v = candidates[bextI];
    cout << "v == 0. -> " << v << endl;
  }

  cerr << "TargetV0: " << tv0 << endl;
  cerr << "TargetV1: " << tv1 << endl;
  double a0 = v.angle(tv0);
  double a1 = v.angle(tv1);
  cerr << "Ang0: " << a0 << endl;
  cerr << "Ang1: " << a1 << endl;

  double angleDiff = abs(a0 - a1);
  cerr << "Angle diff: " << angleDiff << endl;
  close = angleDiff < 0.4;
  if (close && ship.role() == 0) {
    if ((ship.shipId() % 2) == 1) {
      // reverse
      cerr << "Reverse target" << endl;
      if (a0 < a1) {
        return tv1;
      }
      return tv0;
    }
  }
  
  if (a0 < a1) {
    return tv0;
  }
  return tv1;
}

Ship Bot::getWeekShip(GameResponse game) const {
  int bestIndex = 0;
  bint highestDamage = -1;
  
  vector<Ship> ships = game.ships();
  for (int i = 0; i < ships.size(); i++) {
    const Ship&s = ships[i];
    if (s.role() == game.role()) {
      continue;
    }
    cout << "Enemy Ship: " << ships[i] << endl;

    bint damage = s.damage();
    if (highestDamage < damage) {
      bestIndex = i;
      highestDamage = damage;
    }
  }
  cout << "WeekShip: "  << bestIndex << endl;
  return ships[bestIndex];
}

bool approaching(const Ship&a, const Ship&b) {
  return false;
}

double requiredVel(const Ship&s) {
  // V＝（398600/(6378+H)）1/2
  double C = 4000.0;
  double r = s.position().len();
  return sqrt(C / r);
}


Sexp Bot::commands(GameResponse game, const Ship& myShip, Sexp cmds) const {
  Ship enemyShip = getWeekShip(game);
  Sexp shipId = num(myShip.shipId());

  V ePos = enemyShip.position();
  V eVel = enemyShip.velocity();
  if (game.role() == 0) {
    V shootTarget = ePos + eVel;
    Sexp sht = myShip.shoot(shootTarget.x, shootTarget.y, 2);
    cmds = Cons(sht, cmds);
    cout << "CMD Shoot: " << ePos << endl;
  }

  // Move
  bool close = false;
  V tv = targetV(game, myShip, close);
  cerr << "targetV: " << tv << endl;

  double vel = myShip.velocity().len();
  double reqV = requiredVel(myShip);
  cout << "Vel: " << vel << endl;
  cout << "ReqV: " << reqV << endl;
  
  double angle = myShip.velocity().angle(tv);
  double angleDegree = (180.0 * angle) / 3.14;
  cerr << "angleDegree: " << angleDegree << endl;
  if (angleDegree > 30.0 || vel < reqV) {
    V accel = (myShip.velocity() - tv).norm();
    Sexp mov = myShip.accel(accel.x, accel.y);
    cmds = Cons(mov, cmds);
    cout << "CMD Move: " << accel << endl;
  }

  bool far = myShip.position().len() > 50;
  //bool slow = myShip.velocity().len() < 10;
  if (myShip.numParts() > 1 && far && close) {
    cmds = Cons(myShip.split(), cmds);
    cout << "CMD split" << endl;
  }
  
  return cmds;
}
