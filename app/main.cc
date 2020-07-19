#include <iostream>
#include <regex>
#include <string>
#include <list>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <vm/vm.h>
#include <types.h>
#include <plot.h>

using namespace std;

bool https;
string serverUrl;
string serverName;
int serverPort;
string playerKey;

std::shared_ptr<Plot> plot;

bool parseArgv(int argc, char**argv) {
	const std::regex urlRegexp("(http|https)://([^/:]+)(:\\d+)?/?");
  const string serverUrl = argv[1];

  playerKey = argv[2];

	std::smatch urlMatches;
	if (!std::regex_search(serverUrl, urlMatches, urlRegexp)) {
		std::cout << "Unexpected server response:\nBad server URL" << std::endl;
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
	std::cout << "ServerName: " << serverName << "; PlayerKey: " << playerKey << std::endl;
  return true;
}

string post(const string&path, const string& body) {
	shared_ptr<httplib::Client> pclient;
  if (https) {
    pclient.reset((httplib::Client*) new httplib::SSLClient(serverName, serverPort));
  } else {
    pclient.reset(new httplib::Client(serverName, serverPort));
  }

  string pathWithKey = path + "?apiKey=" + playerKey;
  std::cout << "PathWithKey: " << pathWithKey << "; body: " << body << std::endl;

  const std::shared_ptr<httplib::Response> serverResponse = 
      pclient->Post(pathWithKey.c_str(), body.c_str(), "text/plain");

	if (!serverResponse) {
		std::cout << "Unexpected server response:\nNo response from server" << std::endl;
    exit(1);
		return "";
	}
	
	if (serverResponse->status != 200) {
		std::cout << "Unexpected server response:\nHTTP code: " << serverResponse->status
		          << "\nResponse body: " << serverResponse->body << std::endl;
    exit(1);
		return "";
	}

	std::cout << "Server response: " << serverResponse->body << std::endl;
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
}

int runLocal(const string& path) {
  int x = 0;
  int y = 0;
  Sexp state = nil();

  int cx = 0;
  int cy = 0;

  VM vm(path);
  plot.reset(new Plot());

  int numDots = 0;

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

int runBot() {
  cout << "Run bot" << endl;
  bool gprof = false;
#ifdef GPROF
  gprof = true;
#endif
  cout << "player key:" << playerKey << endl;
  Sexp playerKeyExp = num(bint(playerKey));
  cout << "player key (enc):" << playerKeyExp << endl;

  Sexp gameResponse;

  /* CREATE
  Sexp createRequest = List(num(1), num(0));
  gameResponse = call(SEND, createRequest);
  cout << "game respones: " << gameResponse << endl;
  Sexp attackPlayer = num(bint("922338937212915124"));
  Sexp defenderPlayer = num(bint("6458753618228656357"));
  */
  
  Sexp joinRequest = List(num(2), playerKeyExp, NIL);

  // send it to aliens and get the GameResponse
  gameResponse = call(SEND, joinRequest);
  cout << "game respones: " << gameResponse << endl;

  // make valid START request using the provided playerKey and gameResponse returned from JOIN
  Sexp startParam = List(num(4), num(8), num(16), num(32));
  Sexp startRequest = List(num(3), playerKeyExp, startParam);

  // send it to aliens and get the updated GameResponse
  gameResponse = call(SEND, startRequest);
  cout << "game respones: " << gameResponse << endl;

  while (true) {
    Sexp shipId = num(0);
    Sexp v = Vec(num(1), num(1));
    Sexp cmd = List(num(0), shipId, v);
    Sexp commandRequest = List(num(4), playerKeyExp, cmd);
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
  return runLocal(galaxy);
}
