#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "cache.h"
#include "helper.h"
#include <fstream>
#include <ctime>
#include <thread>
#include <mutex>
#include <regex>
#include <stdexcept>

//using namespace std;
#include "server.h"

#define PORT "12345"

#define BACKLOG 10

std::ofstream logFile;

cache cacheRequest;
std::mutex mtx;

void reqConnect(request & req,int sock_send,char *buf, const char* port, char * ip, int sock_s, parser * p)
{
  send(sock_send,"HTTP/1.1 200 OK\r\n\r\n",19,0);
  int nfds = 0; 
  if (sock_send>=sock_s)
    { 
     nfds = sock_send+1;
   }
  else
   {
      nfds = sock_s+1;
   }

  fd_set readfds;
  
  std::string firstLine = p->getFirstLine(buf);
  mtx.lock();
  logFile << req._id << ": Requesting " << firstLine << " from " << req.host << std::endl;
  mtx.unlock();
  
 while(1)
 {
  FD_ZERO(&readfds);
  FD_SET(sock_s,&readfds);
  FD_SET(sock_send,&readfds);

      
  if(select(nfds,&readfds,NULL,NULL,NULL)<0)
    {
      mtx.lock();
      logFile<<req._id<<"ERROR cannot set up connection" <<std::endl;
      mtx.unlock();
	
      return;
    }
  int fd[2] = {sock_s,sock_send};
  int len = 0;
  for (int i =0;i<2;i++)
  {
      char message[65536]={0};
      if (FD_ISSET(fd[i],&readfds))
	{
	  len = recv(fd[i],message,sizeof(message),0);
	  if (len<=0)
	    {
	      return;
	    }
	  else
	    {
	        if (send(fd[1-i],message,len,0)<=0)
		{
		  return;
		}
	    }
	}
    }
  }
}

void revalidation(int sock_send,int sock_s,request & req,response * cache_resp,char * resp)
{
  std::string req1 = req.validationinfo+"If-Modified-Since:"+cache_resp->ifmodifiedsince+"\r\n";
  if (cache_resp->ETag != "")
  {
      req1 = req1+"If-None-Match:"+cache_resp->ETag+"\r\n";
  }
  char m[req1.length()+1];
  strcpy(m,req1.c_str());

  send(sock_s,m,req1.length()+1,0);
  recv(sock_s, resp, 65535, 0);

}

std::string calculateTime(response* m)
{
  time_t now = m->time1+m->max_age;
  tm * ltm = gmtime(&now);
  char * n = asctime(ltm);
  std::string expire(n);
  return expire;
}

void send502Resp(int sock_send, request & req){
  const char * err = "HTTP/1.1 502 Bad Gateway";
  send(sock_send, err, sizeof(err), 0);
  mtx.lock();
  logFile << req._id << ": Responding " << err << std::endl;
  mtx.unlock();
}

response rec_req_send(int sock_s,int sock_send,char *resp,parser * p, request & req)
{
  response out_resp; 
 
  int finished = 1;
  
  while(1)
  {
      if (finished<=0){
	  printf("finish receiving!\n");
	  break;
      }
     
      finished = recv(sock_s, resp, 65535, 0);

      //parse response, get </html> tag
      std::string response = resp;
      std::cout << response;

      bool out = p->parseResponse(response, out_resp);
      
      //check if response is valid
      if(p->checkHead(response, out_resp) == false){
        send502Resp(sock_send, req);
	out_resp.valid = false;
	break;
      }
      
      send(sock_send,resp,finished,0);

      //found </html>, break from loop
      if(out){
	break;
      }
    }
  return out_resp;
}

void whole_validation(int sock_send,int sock_s,request & req,response * cache_resp,char *resp,parser*p)
{
  char mess[65536]={0};  
    revalidation(sock_send,sock_s,req,cache_resp,resp);
    send(sock_send,resp,65535,0);
    recv(sock_s,mess,65535,0);
    std::string vresp(mess);
    response m11 = p->parseResponse1(vresp);
	
    if (vresp.find("200 OK")!=-1)
     {
        std::string flag = req.host+req.resource;
	mtx.lock();
        cacheRequest.cache_map.erase(flag);
	cacheRequest.insertReq(req,m11);
	mtx.unlock();
	send(sock_s,mess,65535,0);
      }
    else
      {
	const char* m = cache_resp->content.c_str(); 
	send(sock_s,m,65535,0);
      }
}

void req_Get_Post(request & req,int sock_send,char *buf, const char * port, char * ip, int sock_s, parser * p)
{
  send(sock_s, buf, 1000,0);
  char resp[65535] = {0};
  char mess[65535] = {0};
  response out_resp;
   if (req.request_type==0)
   {
     // std::string respInCache;
     int type= -1; 
     response * cache_resp = cacheRequest.get_resp(req,type);
     if(cache_resp != NULL){
       if (type == 0)
	 {
	    mtx.lock();
	    std::string expireTime =calculateTime(cache_resp);
	    logFile<<req._id<<": in cache, but expired at "<<expireTime <<std::endl;
	    mtx.unlock();
	    whole_validation(sock_send,sock_s,req,cache_resp,resp,p);
	     
         }
	else if(type == 1)
	{
	    mtx.lock();
	    logFile<<req._id<<": in cache, requires validation"<<std::endl;
	    mtx.unlock();
	    whole_validation(sock_send,sock_s,req,cache_resp,resp,p);
	    
	}
	else  
	{
	    mtx.lock();
	    logFile<<req._id<<": in cache, valid"<<std::endl;
	    mtx.unlock();
	    send(sock_send, cache_resp->content.c_str(), 65535, 0);
	      
	 }
	    
      }
      else
      {
	int h =  recv(sock_s,resp,65535,0);
	send(sock_send,resp,h,0);
	
	std::string res(resp);
	if (res.find("200 OK")!=-1)
	  {
	  response newRes =p->parseResponse1(res);
	  if (req.noStore == 1 || newRes.nostore ==1)
	   {
	      mtx.lock();
	      logFile<<req._id<<": not cacheable because cache control " <<std::endl;
	      mtx.unlock();
	   }
	  
	  if (req.noStore != 1 && newRes.nostore!=1)
	   {
	       mtx.lock();
	       cacheRequest.insertReq(req,newRes);
	       mtx.unlock();
	   }

	  //Log cache
	  mtx.lock();
          logFile << req._id << ": " << "not in cache" << std::endl; 
	  mtx.unlock();
	  
	  out_resp = rec_req_send(sock_s,sock_send,resp,p,req);
	  }
	else
	  {
	    out_resp = rec_req_send(sock_s,sock_send,resp,p,req);
	  }
      }
      }

  
  if (req.request_type ==1)
  {
  out_resp = rec_req_send(sock_s,sock_send,resp,p, req);
  }

    //log request, response
  std::string firstLine = p->getFirstLine(buf);
  mtx.lock();
  logFile << req._id << ": Requesting " << firstLine << " from " << req.host << std::endl;
  if(out_resp.valid == true){
  logFile << req._id << ": Recieved " << out_resp.firstLine << " from " << req.host << std::endl;
  logFile << req._id << ": Responding " << out_resp.firstLine << std::endl;
  }
  mtx.unlock();

}


void send400Resp(int sock_send, int id){
  const char * err = "HTTP/1.1 400 Bad Code";
  send(sock_send, err, sizeof(err), 0);
  mtx.lock();
  logFile << id << ": Responding " << err << std::endl;
  mtx.unlock();
      
}

void handleRequest(int sock_rec, cache cache_get, int sock_send){
    
  //recieve the data
     char buf[65536]={0};
  // rec_data(sock_send, buf, 1000);
     int len =  recv(sock_send,buf,sizeof(buf),0);
     if (len<=0)
       {
	 mtx.lock();
	 logFile<<"ERROR NULL REQUEST" <<std::endl;
	 mtx.unlock();
	 return; 
       }
     // buf[1000] = 0;
     std::cout << buf << std::endl;

    //parse the request
      parser * p = new parser();
      std::string r = buf;
      request req;
      try{
	 req = p->parseRequest(buf);
      }
      catch(...){
	send400Resp(sock_send, -1);
	return;
      }
 
      //check if request is valid
      /*   if(req.port.compare("443") != 0 && req.port.compare("80") != 0){
	send400Resp(sock_send, req._id);
	return;
	}*/
   
      

      //Log request
      std::string firstLine = p->getFirstLine(r);
      mtx.lock();
      time_t now = time(0);

      // Convert now to tm struct for local timezone
      tm* localtm = localtime(&now);
      logFile << req._id << ": '" << firstLine << "' from " << req.host  << " @ " << asctime(localtm);  
      mtx.unlock();
      
      struct hostent * dest;
      const char * host  = req.host.c_str();
      // std::cout<<"host:"<<host<<std::endl;

      //get ip address
      if((dest = gethostbyname(host)) == NULL){
	send502Resp(sock_send, req);
	return;
      }

      char ip[100];
      struct in_addr **addr_list = (struct in_addr **) dest->h_addr_list;

      for(int i = 0; addr_list[i] != NULL; i++){
	  //Return the first one;
	  strcpy(ip , inet_ntoa(*addr_list[i]) );
      }

      std::cout << ip;

      //get port name
      const char * port = req.port.c_str();
      int sock_s = get_sock(ip, port, 1); 
      //   std::cout<<"portaaa"<<port<<std::endl;

      //process request aptly
      if (req.request_type==3)
      {
	//log error
        	mtx.lock();
	  logFile<<req._id<<": HTTP/1.1 400 Bad Request"<<std::endl;// our proxy will not handle other types of requests
	  mtx.unlock();
	  return;
      }
      //connect 
      else if (req.request_type==2){
	reqConnect(req,sock_send,buf, port, ip, sock_s, p);
	  mtx.lock();
	  logFile << req._id << ": Tunnel closed" << std::endl;
	  mtx.unlock();
      }
      //Get post
      else if(req.request_type == 0 || req.request_type == 1){
	req_Get_Post(req,sock_send,buf, port ,ip, sock_s,p);
	}
      else{
	send400Resp(sock_send, req._id);
      }

      //close socket
      close(sock_send);
      close(sock_s);
}


int main(void){
  logFile.open("abc.txt");
  std::string * s = new std::string(PORT);
  // This socket is used for receiving response
  int sock_rec = get_sock(NULL, s->c_str(), 0);

  if (sock_rec == -1){
      logFile<<"ERROR creating socket to accept"<<std::endl;
      free(s);
      return -1;
  }

  //Init CACHE
  cache cache_get;
  
  //wait for incoming connections
  if(listen(sock_rec, BACKLOG) == -1){
    free(s);
    return -1;
  }

   while(1){
     std::string ip1;
     int sock_send = sock_accept(sock_rec,&ip1);
     if (sock_send == -1)
       {
	 logFile<<"ERROR connecting client" <<std::endl;
	 continue;
       }
      
     std::thread th (&handleRequest, sock_rec, cache_get, sock_send);
     th.detach();
  }
  free(s);
  close(sock_rec);
  return 0;
}
