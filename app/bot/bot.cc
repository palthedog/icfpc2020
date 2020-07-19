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
  
  if (d0 < d1) {
    return tv0;
  }
  return tv1;
}

Sexp Bot::command(GameResponse game) {
  Ship myShip = game.myShip();
  Ship enemyShip = game.enemyShip();
  
  Sexp shipId = num(myShip.shipId());

  V dp = enemyShip.position() - myShip.position();
  if (game.gameTick() % 5 == 0) {
    return myShip.shoot(dp.x, dp.y);
  }

  // Move
  V tv = targetV(game, myShip);
  cerr << "targetV: " << tv << endl;
  V normalizedTargetV = tv.norm();
  cerr << "norm TargetV: " << normalizedTargetV << endl;
  return myShip.accel(-normalizedTargetV.x, -normalizedTargetV.y);
}
