#include "bot.h"

#include <map>
#include <iostream>

using namespace std;

V nextGravity(const Ship&s) {
  return -s.position().norm();
}

pair<V, V> targetV(GameResponse game, const Ship& ship, bool& close) {
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
  
  vector<V> candidates = {
    V(1, 0),
    V(-1, 0),
    V(0, 1),
    V(0, -1),
    V(1, 1),
    V(-1, -1),
    V(-1, 1),
    V(1, -1),
  };
  int bestI = 0;
  int secondI = 0;
  double bestCross = 0;
  double secondCross = 0;
  cout << "pos: " << pos << endl;
  cout << "G: " << g << endl;
  for (int i = 0; i < 8; i++) {
    V cand = candidates[i] + v;
    double crs = cand.cross(-pos);
    cout << candidates[i] << ", cross: " << crs << endl;
    crs = abs(crs);
    
    if (bestCross < crs) {
      secondCross = bestCross;
      secondI = bestI;
      
      bestI = i;
      bestCross = crs;
    }
  }
  cout << "Best second I " << bestI << " " << secondI << endl;
  return make_pair(candidates[bestI], candidates[secondI]);
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
  /*
  // V＝（398600/(6378+H)）1/2
  double C = 4000.0;
  double r = s.position().len();
  return sqrt(C / r);
  */
  return 7.0;
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
  auto bestScnd = targetV(game, myShip, close);
  cout << "best second: " << bestScnd.first << " " << bestScnd.second << endl;

  V vel = myShip.velocity();
  double velLen = vel.len();
  double reqV = requiredVel(myShip);
  cout << "Vel: " << velLen << endl;
  cout << "ReqV: " << reqV << endl;

  V pos = myShip.position();
  V g = nextGravity(myShip);
  double cr0 = (vel + bestScnd.first + g).cross(-pos);
  double cr1 = (vel + bestScnd.second + g).cross(-pos);
  
  cout << "Cross0: " << cr0 << endl;
  cout << "Cross1: " << cr1 << endl;
  bool oppositeDirection = (cr0 * cr1 < 0);
  
  if (abs(cr0) < 200) {  // velLen < reqV
    V best = bestScnd.first;
    if (oppositeDirection &&
        (myShip.shipId() % 2 == 0) &&
        cr0 < 0) {
      cout << "prefer positive" << endl;
      best = bestScnd.second;
    }
    if (oppositeDirection &&
        (myShip.shipId() % 2 == 1) &&
        cr0 > 0) {
      cout << "prefer negative" << endl;
      best = bestScnd.second;
    }
    Sexp mov = myShip.accel(-best.x, -best.y);
    cmds = Cons(mov, cmds);
    cout << "CMD Move: " << -best << endl;
  }

  if (myShip.numParts() > 1 && oppositeDirection) {
    cmds = Cons(myShip.split(), cmds);
    cout << "CMD split" << endl;
  }
  
  return cmds;
}
