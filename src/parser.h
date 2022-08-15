#include "request.h"
#include "response.h"
#include <cstring>

class parser{
  
 public:
  parser(){
  }
  bool parseResponse(std::string resp, response &r);
  request parseRequest(std::string request1);
  response parseResponse1(std::string m);
  std::string getFirstLine(std::string req);
  bool checkHead(std::string req, response &r);
  //int getType(std::string a);
 };
