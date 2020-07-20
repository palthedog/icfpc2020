#include <iostream>
#include <regex>
#include <string>
#include <list>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 20
#define CPPHTTPLIB_READ_TIMEOUT_SECOND 20
#define CPPHTTPLIB_WRITE_TIMEOUT_SECOND 20
#include "httplib.h"

#include <vm/vm.h>
#include <bot/bot.h>

#include <game.h>
#include <types.h>
#include <plot.h>

using namespace std;

bool https;
string serverUrl;
string serverName;
int serverPort;

string apiKey;
string playerKeyStr;

bool localMode;

std::shared_ptr<Plot> plot;

bool parseArgv(int argc, char**argv) {
	const std::regex urlRegexp("(http|https)://([^/:]+)(:\\d+)?/?");
  const string serverUrl = argv[1];
  
  if (argc == 3) {
    // subbmission
    localMode = false;
    cerr << "Submission mode" << endl;
    playerKeyStr = argv[2];
  } else if (argc == 4) {
    // local bot test
    localMode = true;
    cerr << "local mode" << endl;
    apiKey = argv[2];
    playerKeyStr = argv[3];
  }

	std::smatch urlMatches;
	if (!std::regex_search(serverUrl, urlMatches, urlRegexp)) {
		std::cout << "Bad server URL" << std::endl;
		return false;
	}
  string protocol = urlMatches[1];
  https = protocol == "https";

	serverName = urlMatches[2];
  if (urlMatches[3] == "") {
    serverPort = https ? 443 : 80;
  } else {
    serverPort = std::stoi(urlMatches[3].str().substr(1));
  }

  cout << "ServerURL: " << serverUrl << endl;
  cout << "ServerPort: " << serverPort << endl;
	cout << "ServerName: " << serverName << "; APIKey: " << apiKey << std::endl;
	cout << "PlayerKey: " << playerKeyStr << endl;
  return true;
}

string post(const string&path, const string& body) {
	shared_ptr<httplib::Client> pclient;
  if (https) {
    pclient.reset((httplib::Client*) new httplib::SSLClient(serverName, serverPort));
  } else {
    pclient.reset(new httplib::Client(serverName, serverPort));
  }

  string pathWithKey = path;
  if (apiKey != "") {
    pathWithKey += "?apiKey=" + apiKey;
  }
  std::cout << "PathWithKey: " << pathWithKey << endl;
  std::cout << "body: " << body << std::endl;

  const std::shared_ptr<httplib::Response> serverResponse = 
      pclient->Post(pathWithKey.c_str(), body.c_str(), "text/plain");

	if (!serverResponse) {
		std::cout << "Unexpected server response:\nNo response from server" << std::endl;
    exit(1);
	}
	
	if (serverResponse->status != 200) {
		std::cout << "Unexpected server response:\nHTTP code: " << serverResponse->status
		          << "\nResponse body: " << serverResponse->body << std::endl;
    exit(1);
		return "";
	}

	//std::cout << "Server response: " << serverResponse->body << std::endl;
	return serverResponse->body;
}

std::string sendData(const std::string& path, const std::string& payload) {
  return post(path, payload);
}

void printNum(bint num) {
  cout << num << endl;
  cout << numToStr(num) << endl;
}

void printSexp(const string& str) {
  cout << "Input: " << str << endl;
  Sexp sexp = parse(nullptr, str);
  cout << "Parsed: " << sexp << endl;
  cout << "Eval: " << eval(sexp) << endl;
}

void printMod(const string& str) {
  cout << "Input: " << str << endl;
  Sexp sexp = parse(nullptr, str);
  cout << "Parsed: " << sexp << endl;
  auto e = eval(sexp);
  cout << "Eval: " << e << endl;
  cout << "Mod-str: " << e->mod() << endl;
}

int runTest() {
  /*
  printNum(1);
  printNum(7);
  printNum(8);
  printNum(508);
  printNum(65537);
  printNum(bint("8704918670857764736"));

  // From blog.
  printNum(decode("110110000111011111100001001111110101000000"));

  // From blog.
  printNum(decode("110110000111011111100001001111110100110000"));

  // Response for "1101000"
  printNum(decode("110110000111011111100001001110011010000100"));

  // Response
  cerr << decode("1101000") << endl;
  */

  printSexp("ap ap add 1 2");
  printSexp("ap ap ap cons 1 2 add");
  printSexp("ap ap ap c add 1 2");
  printSexp("ap ap ap b inc dec 3");
  printSexp("ap ap ap s add inc 1");

  printSexp("ap ap ap s mul ap add 1 6");
    
  printSexp("ap ap t 1 5");
  printSexp("ap ap t t i");
  printSexp("ap ap t t ap inc 5");
  printSexp("ap ap t ap inc 5 t");

  printSexp("ap car ap ap cons 3 1");
  printSexp("ap car i");  // t

  printSexp("ap ap eq 3 3");  // t
  printSexp("ap ap eq 3 2");  // f

  printSexp("ap ap t 42 ap inc 42");

  printSexp("ap ap f 42 0");

  printMod("ap mod 0");
  printMod("ap mod 1");
  printMod("ap mod -1");
  printMod("ap mod 2");
  printMod("ap mod -2");
  printMod("ap mod 256");
  printMod("ap mod -256");

  printSexp("ap ap ap cons 2 1 add");
  printSexp("ap car ap ap cons t f");
  printSexp("ap car i");
  printSexp("ap cdr i");

  printSexp("ap isnil nil");
  printSexp("ap isnil ap ap cons 1 2");

  printSexp("ap ap lt -19 -20");

  printSexp("ap ap div 4 2");
  printSexp("ap ap div 4 3");
  printSexp("ap ap div 4 5");
  printSexp("ap ap div 6 -2");
  printSexp("ap ap div -5 3");
  printSexp("ap ap div -5 -3");

  // mod
  printSexp("ap mod nil");
  printSexp("ap mod ap ap cons nil nil");
  printSexp("ap mod ap ap cons 0 nil");
  printSexp("ap mod ap ap cons 1 2");
  printSexp("ap mod ap ap cons 1 ap ap cons 2 nil");

  printSexp("ap dem ap mod nil");
  printSexp("ap dem ap mod ap ap cons nil nil");
  printSexp("ap dem ap mod ap ap cons 0 nil");
  printSexp("ap dem ap mod ap ap cons 1 2");
  printSexp("ap dem ap mod ap ap cons 1 ap ap cons 2 nil");

  printSexp("ap dem ap mod ap ap cons 1 ap ap cons 2 nil");
  return 0;
}

void visualizeGame(GameResponse game) {
  if (!plot) {
    return;
  }
  
  plot->clear();
  
  int earthSize = 16;
  plot->startDraw();
  for (int y = -earthSize; y <= earthSize; y++) {
    for (int x = -earthSize; x <= earthSize; x++) {
      plot->draw(x, y);      
    }
  }
  plot->endDraw();

  plot->startDraw();
  for (const Ship& s : game.ships()) {
    auto pos = s.position();
    plot->draw((int)pos.x, (int)pos.y);
  }
  plot->endDraw();
  
  plot->flush();
}

int runLocal(const string& path) {
  plot.reset(new Plot());

  int x = 0;
  int y = 0;
  Sexp state = nil();

  VM vm(path);
  list<pair<int, int>> bootstrap = {
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {8, 4},
    {2, -8},
    {3, 6},
    {0, -14},
    {-4, 10},
    {9, -3 },
    {-4, 10},
    {1, 4  }
  };

  bool gprof = false;
#ifdef GPROF
  gprof = true;
#endif
  
  while (true) {
    plot->clear();
    
    cout << "x: " << x << ", y: " << y << endl;

    //auto vec0 = 
    //Sexp p = vm.protocol("galaxy");
    Sexp p = vm.interact("galaxy", state, Vec(num(x), num(y)));
    // Sexp p = vm.function(1141);
    cout << "Start evaluating: " << str(p) << endl;
    
    Sexp result = eval(p);
    cout << "p = " << result << endl;

    state = call(Car(), result);

    plot->flush();
    if (!bootstrap.empty()) {
      auto it = bootstrap.begin();;
      x = it->first;
      y = it->second;
      bootstrap.pop_front();
    } else {
      if (gprof) {
        cout << "quitting for gprof." << endl;
        return 0;
      }

      cout << "listening" << endl;
      string response = plot->read();
      cout << "from plotter: " << response << endl;
      istringstream iss(response);
      string cmd;
      iss >> cmd >> x >> y;
    }
  }
  return 0;
}

Sexp joinGame(Sexp playerKey) {
  cout << "Join: " << playerKey << endl;
  Sexp joinRequest = List(num(2), playerKey, NIL);

  cout << "Join Request: " << joinRequest << endl;
  // send it to aliens and get the GameResponse
  Sexp gameResponse = call(SEND, joinRequest);
  cout << "Join respones: " << gameResponse << endl;
  return gameResponse;
}

Sexp startGame(Sexp playerKey, Sexp gameState) {
  GameResponse game(gameState);

  int fuel = 300;  // OK: 300, NG: 500
  int snd = 1;
  int third = 1;  // larger one consumes fewer fuel.
  int forth = 1;
  if (!localMode || game.role() == 0) {
    /*
    snd = 10;
    third = 7; // OK: 10
    forth = 5; // OK: 10
    */
    snd = 10;
    third = 11; // OK: 11, NG: 16
    forth = 5; // OK: 10
  }

  if (eval(game.staticGameInfo())->isNil()) {
    fuel = 100;
    snd = 8;
    third = 16;
    forth = 32;
  }
  
  // make valid START request using the provided playerKey and gameResponse returned from JOIN
  Sexp startParam = List(num(fuel),
                         num(snd),
                         num(third),
                         num(forth));
  Sexp startRequest = List(num(3), playerKey, startParam);

  cout << "Start Request: " << startRequest << endl;
  // send it to aliens and get the updated GameResponse
  Sexp startResponse = call(SEND, startRequest);
  cout << "Start response: " << startResponse << endl;
  return startResponse;
}

int runBot() {
  cout << "Run bot" << endl;
  cout << "player key:" << playerKeyStr << endl;
  Sexp playerKey = num(bint(playerKeyStr));

  Sexp gameResponse = joinGame(playerKey);
  if (!checkGame(gameResponse)) {
    cout << "Failed to join game." << endl;
    return 0;
  }

  Sexp info = GameResponse(gameResponse).staticGameInfo();
  cout << "***" << endl;
  cout << "Static Game Info: " << info << endl;
  cout << "***" << endl;
  
  
  gameResponse = startGame(playerKey, gameResponse);
  if (!checkGame(gameResponse)) {
    cout << "Failed to start game: " << gameResponse << endl;
    return 1;
  }

  cout << "Game Response: " << gameResponse << endl;


  if (apiKey != "" && GameResponse(gameResponse).role() == 0) {
    // local test.
    cout << "Enable plotter" << endl;
    plot.reset(new Plot());
  }

  Bot bot(playerKey);
  while (true) {
    GameResponse game(gameResponse);
    visualizeGame(game);

    Sexp info = game.staticGameInfo();
    Sexp state = game.gameState();
    cout << "OK: " << game.ok() << endl;
    cout << "Tick: " << game.gameTick() << endl;
    cout << "?State: " << nth(state, 1) << endl;

    for (const Ship& s : game.ships()) {
      cout << "    " << s.toString() << endl;
    }

    if (!game.ok()) {
      cerr << "Fail?" << endl;
      cerr << "  stage: " << game.gameState() << endl;
      break;
    }

    if (game.gameStage() != 1) {
      cerr << "Finished" << endl;
      break;
    }

    Sexp cmd = bot.command(game);
    Sexp commandRequest = List(num(4), playerKey, List(cmd));
    gameResponse = call(SEND, commandRequest);
    cout << "game respones: " << gameResponse << endl;
  }
  return 0;
}

int communicate() {
  post("/aliens/send", modImpl(num(0)));
  return 0;
}

int main(int argc, char* argv[]) {
  const string galaxy = "../data/galaxy.txt";
  if (argc < 3) {
    cerr << argv[0] << "<server> <key>" << endl;
    return 1;
  }

  parseArgv(argc, argv);
  if (argc == 3) {
    return runBot();
  }

  if (playerKeyStr == "galaxy") {
    return runLocal(galaxy);
  }
  return runBot();
}
