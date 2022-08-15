#include <cstdlib>
#include <iostream>  
#include "parser.h"
#include <cstring>
#include <ctime>
#include <vector>
#include <stdexcept>
//#include <boost/algorithm/string/trim.hpp>

int getType(std::string s) 
{
     if (s.compare("GET")==0)
      {
	 return 0;
      }
     else if (s.compare("POST")==0)
     {
	return 1;
     }
     else if (s.compare("CONNECT")==0)
     {
	return 2;
     }
     else
      {
	 return 3;
      }
 }

int request::id = 0;
response parser::parseResponse1(std::string m)
{
  response newRes;
  if (m.find("no-cache")!=-1)
    {
      newRes.nocache = true;
    }

  if (m.find("no-stroe")!=-1)
    {
      newRes.nostore = true;
    }

  if (m.find("must-revalidate")!=-1)
    {
      newRes.mustValidate = true;
    }

  if (m.find("ETag")!=-1)
    {
      std::string sub  =  m.substr(m.find("ETag:"));
      newRes.ETag = sub.substr(sub.find("ETag:")+5,sub.find_first_of("\r\n")-sub.find("ETag:")-5);
    }

 if (m.find("If-None-Match")!=-1)
    {
      std::string sub  =  m.substr(m.find("If-None-Match:"));
      newRes.ifnomatch = sub.substr(sub.find("If-None-Match:")+14,sub.find_first_of("\r\n")-sub.find("If-None-Match")-14);
    }

 if (m.find("Last-Modified")!=-1)
   {
     std::string sub  =  m.substr(m.find("Last-Modified:"));
      newRes.ifmodifiedsince = sub.substr(sub.find("Last-Modified:")+14,sub.find_first_of("\r\n")-sub.find("Last-Modified:")-14);
   }

 if (m.find("Exipres:")!=-1)
   {
     std::string sub  =  m.substr(m.find("Expires:"));
      newRes.expires= sub.substr(sub.find("Expires:")+8,sub.find_first_of("\r\n")-sub.find("Expires:")-8);
   }
 
 if (m.find("private")!=-1)
   {
     newRes.isPrivate = true;
   }
 newRes.content = m;
 return newRes;
}

std::string parser::getFirstLine(std::string req)
{
  size_t lineEnd = req.find_first_of("\r\n");
  std::string firstLine = req.substr(0,lineEnd);
  return firstLine;
}

bool parser::checkHead(std::string req, response &r){
  if(r.hasHeader()){
    return true;
  }
  return req.find("\r\n\r\n") == std::string::npos;
}

bool parser::parseResponse(std::string resp, response &r){
  std::string firstLine = getFirstLine(resp);
  
  int word = firstLine.find(" ");
  std::string status = firstLine.substr(word + 1, 3); 
   
    
  if(!r.hasHeader()){
    if(status.compare("200") != 0){
      return false;
    }
    r.setHeader(firstLine);
  }

  bool out = r.appendResp(resp);
  return out;
}

request parser::parseRequest(std::string request1)
{
  try{
  size_t a = request1.find_first_of("\r\n");
  std::string firstLine = request1.substr(0,a);
  std::string restLine = request1.substr(a+2);
  int method = firstLine.find(" ");
  
  int type = getType(firstLine.substr(0,method));
  int res = firstLine.find(" ",method+1);
  std::string next1 = firstLine.substr(method+1,res-method-1);
  int timeCur =time(0);
  //  std::cout<<restLine.find("\r\n")<<std::endl;
  std::string ipLine =restLine.substr(restLine.find("Host:"),restLine.find("\r\n")-restLine.find("Host:"));
  std::string ipFrom =ipLine.substr(ipLine.find("Host:")+6,ipLine.find_last_of(":")-ipLine.find("Host:")-6);
  //  std::cout<<"ipFrom"<<ipLine.find(":")<<std::endl;
  std::string port;
  // std::cout<<"nLin  "<<ipLine<<std::endl;
  // std::cout<<"m    "<<ipFrom<<std::endl;
  // std::cout<<ipLine.find(":")<<"+"<<ipLine.find(":")<<std::endl;
  if (ipLine.find_last_of(":") != ipLine.find(":"))
    {
      //port = ipLine.substr(ipLine.find_last_of(":")+1, 3);
      port = "443";
      //port = ipLine.substr(ipLine.find_last_of(":")+1,ipLine.find("\r\n")-ipLine.find_last_of(":")
    }
  else
    {
      port = "80";
    }
  std::string m =firstLine+ipLine;
  bool noCache = false;
  bool noStore = false;
  // std::string maxAge = "";
  // std::cout<<"b"<<port<<std::endl;
  //  std::vector<std::string> cacheControl;
   if (ipLine.find("Cache-Control:")!=-1)
    {
      // cacheControl =ipLine.substr(ipLine.find("Cache-Control:")+14,ipLine.find("\r\n")-ipLine.find("Cache-Control:")-14);
      if (ipLine.find("no-cache")!=-1)
	{
	  noCache = true;
	}

      if (ipLine.find("no-store")!=-1)
	{
	  noStore = true;
	}
     	
    }
      // cacheControl = "No specification";
   return request(type,next1,ipFrom,timeCur,port,noCache,noStore,m);
  }
  catch(...){
    throw std::invalid_argument("request invalid");
  }
}


