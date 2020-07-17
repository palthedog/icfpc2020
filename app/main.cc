#include <iostream>
#include <regex>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <types.h>

using namespace std;

bool https;
string serverUrl;
string serverName;
int serverPort;
string playerKey;

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
}

bool post(const string&path, const string& body) {
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
		return false;
	}
	
	if (serverResponse->status != 200) {
		std::cout << "Unexpected server response:\nHTTP code: " << serverResponse->status
		          << "\nResponse body: " << serverResponse->body << std::endl;
		return false;
	}

	std::cout << "Server response: " << serverResponse->body << std::endl;
	return true;
}

void printNum(bint num) {
  cout << num << endl;
  cout << numToStr(num) << endl;
}

int runLocal(int argc, char** argv) {
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
  return 0;
}

int communicate() {
  post("/aliens/send", "110110000111011111100001001111110101000000");
  return 0;
}

int main(int argc, char* argv[]) {
#ifdef LOCAL
  return runLocal(argc, argv);
#else
  serverUrl = argv[1];
  parseArgv(argc, argv);
  return communicate();
#endif
  
}
