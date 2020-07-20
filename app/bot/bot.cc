#include "bot.h"

#include <map>
#include <iostream>

using namespace std;

V targetV(GameResponse game, const Ship& ship) {
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

  V v = ship.velocity();
  V tv0(tvx0, tvy0);
  V tv1(tvx1, tvy1);

  cerr << "TargetV0: " << tv0 << endl;
  cerr << "TargetV1: " << tv1 << endl;
  
  bint d0 = v.dist(tv0);
  bint d1 = v.dist(tv1);
  cerr << "D0: " << d0 << endl;
  cerr << "D1: " << d1 << endl;
  double a0 = v.angle(tv0);
  double a1 = v.angle(tv1);
  cerr << "Ang0: " << a0 << endl;
  cerr << "Ang1: " << a1 << endl;

  if (a0 < a1) {
    return tv0;
  }
  return tv1;
}

Sexp Bot::commands(GameResponse game) {
  Sexp cmds = nil();

  Ship myShip = game.myShip();
  Ship enemyShip = game.enemyShip();
  
  Sexp shipId = num(myShip.shipId());

  V ePos = enemyShip.position();
  V eVel = enemyShip.velocity();
  if (game.role() == 0) {
    V shootTarget = ePos + eVel;
    cmds = Cons(myShip.shoot(shootTarget.x, shootTarget.y, 2), cmds);
    cout << "CMD Shoot: " << ePos << endl;
  }

  // Move
  V tv = targetV(game, myShip);
  cerr << "targetV: " << tv << endl;

  double angle = myShip.velocity().angle(tv);
  double angleDegree = (180.0 * angle) / 3.14;
  cerr << "angleDegree: " << angleDegree << endl;
  if (angleDegree > 30.0) {
    V accel = (myShip.velocity() - tv).norm();
    Sexp mov = myShip.accel(accel.x, accel.y);
    cmds = Cons(mov, cmds);
    cout << "CMD Move: " << accel << endl;
  }
  return cmds;
}
