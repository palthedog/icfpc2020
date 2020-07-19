#include <iostream>
#include <regex>
#include <string>
#include <list>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <vm/vm.h>
#include <game.h>
#include <types.h>

using namespace std;

bool https;
string serverUrl;
string serverName;
int serverPort;

string apiKey;

bool parseArgv(int argc, char**argv) {
	const std::regex urlRegexp("(http|https)://([^/:]+)(:\\d+)?/?");
  const string serverUrl = argv[1];

  apiKey = argv[2];

	std::smatch urlMatches;
	if (!std::regex_search(serverUrl, urlMatches, urlRegexp)) {
		std::cerr << "Unexpected server response:\nBad server URL" << std::endl;
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

  cerr << "ServerURL: " << serverUrl << endl;
	std::cerr << "ServerName: " << serverName << "; APIKey: " << apiKey << std::endl;
  return true;
}

string post(const string&path, const string& body) {
	shared_ptr<httplib::Client> pclient;
  if (https) {
    pclient.reset((httplib::Client*) new httplib::SSLClient(serverName, serverPort));
  } else {
    pclient.reset(new httplib::Client(serverName, serverPort));
  }

  string pathWithKey = path + "?apiKey=" + apiKey;
  std::cerr << "PathWithKey: " << pathWithKey << "; body: " << body << std::endl;

  const std::shared_ptr<httplib::Response> serverResponse = 
      pclient->Post(pathWithKey.c_str(), body.c_str(), "text/plain");

	if (!serverResponse) {
		std::cerr << "Unexpected server response:\nNo response from server" << std::endl;
    exit(1);
		return "";
	}
	
	if (serverResponse->status != 200) {
		std::cerr << "Unexpected server response:\nHTTP code: " << serverResponse->status
		          << "\nResponse body: " << serverResponse->body << std::endl;
    exit(1);
		return "";
	}

	std::cerr << "Server response: " << serverResponse->body << std::endl;
	return serverResponse->body;
}

std::string sendData(const std::string& path, const std::string& payload) {
  return post(path, payload);
}

int createGame() {
  Sexp gameResponse;
  // CREATE
  Sexp createRequest = List(num(1), num(0));
  Sexp createResponse = call(SEND, createRequest);
  cerr << "Create respones: " << createResponse << endl;
  if (to_int(car(createResponse)) == 0) {
    cerr << "Failed to create" << endl;
    return 1;
  }
  Sexp body = nth(createResponse, 1);
  Sexp attackerPlayer = nth(nth(body, 0), 1);
  Sexp defenderPlayer = nth(nth(body, 1), 1);
  cout << eval(attackerPlayer) << endl;
  cout << eval(defenderPlayer) << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  const string galaxy = "../data/galaxy.txt";
  if (argc < 3) {
    cerr << argv[0] << "<server> <key>" << endl;
    return 1;
  }
  parseArgv(argc, argv);  
  return createGame();
}
