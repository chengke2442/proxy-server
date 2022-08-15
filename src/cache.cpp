#include "cache.h"
#include <iostream>
#include <ctime>

bool checkExpire(std::string timeStr)
{
  size_t m = timeStr.find("GMT");
  if (m !=-1 )
    {
      timeStr.erase(m-1,4);
    }
  tm time1;
  strptime(timeStr.c_str(),"%a,%d %b %Y %H:%M:%S",&time1);
  time_t tm1 = mktime(&time1);
  time_t now = time(0)-28800;
  return (now < tm1);
}
response * cache::get_resp(request req,int & type){
  std::map<std::string, response>::iterator it;
  std::string request1 = req.host + req.resource;
  std::cout << "cache : "<< request1 << std::endl;
  
  it = cache_map.find(request1);
   if(it == cache_map.end()){
    return NULL;
  }
 int timeCur = time(0);
 if ((it->second.max_age+it->second.time1)<=timeCur)
   {
     type = 0;
   }
 if ((it->second.mustValidate==true) ||(it->second.nocache==true))
   {
     type = 1;
   }
 if (((it->second.max_age+it->second.time1)>=timeCur) && (it->second.mustValidate==false) && (checkExpire(it->second.expires) == true))
 {
     type = 2;
 }
  if(it == cache_map.end()){
    return NULL;
  }
  return &cache_map[request1];
}

int cache::insertReq(request req, response resp){

  std::string m = req.host+req.resource;
  if (cache_map.find(m)==cache_map.end())
    {
    
     cache_map.insert(std::pair<std::string, response>(m, resp));

    for (std::map<std::string, response>::iterator it= cache_map.begin(); it!=cache_map.end(); ++it)
    {
      std::cout << "---------------------------------------\n";
      std::cout << it->first << " => " << it->second.content << '\n';
      std::cout << "---------------------------------------\n"; 
    }

      return 0; 
    }
  else
    {
      return 1; 
    }
}
  
