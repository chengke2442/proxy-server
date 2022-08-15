#include <iostream>
#include <cstring>
#include "parser.h"

using namespace std;

int main(int argc, char *argv[])
{
  parser* m = new parser();
  // const std::string request1 = "GET /hello.txt HTTP/1.1 zlib/1.2.3 \r\n   Host: www.example.com \r\n  Accept-Language: en, mi\r\n";
  request result = m->parseRequest("GET /hello.txt HTTP/1.1\r\n User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n Host: www.example.com \r\n  Accept-Language: en, mi\r\n Cache-Control:no-cache\r\n");
  std::cout<<result._id<<std::endl;
  std::cout<<result.resource<<std::endl;
  std::cout<<result.host<<std::endl;
  std::cout<<"bb:"<<result.noStore<<std::endl;
   delete m;
  return 0;
}
