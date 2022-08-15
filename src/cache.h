#include <map>
#include "parser.h"

class cache{
public:
  //  store request-response
  std::map<std::string, response> cache_map;
  
  response * get_resp(request req,int & m);
  int insertReq(request req, response resp);
   
};
