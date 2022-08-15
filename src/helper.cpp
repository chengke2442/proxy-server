#include "helper.h"
#include <iostream>

using namespace std; 

int get_sock(const char * addr, const char * port, int opt){
  struct addrinfo * list = init_addr(addr, port);
  int sock_rec = init_sock(list, opt);
  return sock_rec;
}



struct addrinfo * init_addr(const char * addr, const char * port){
  //own address info
  struct addrinfo own, *list;

  //clear out own addrinfo
  memset(&own, 0, sizeof(own));

  //IPv4/IPv6 address both
  own.ai_family = AF_UNSPEC;

  //Connected socket
  own.ai_socktype = SOCK_STREAM;

  //own IP
  if(addr == NULL){
    own.ai_flags = AI_PASSIVE;
  }

  
  //getaddrinfo fxn
  if(getaddrinfo(addr, port, &own, &list) != 0){
    cerr<<"Error: cannot set up proxy"<<endl;
    return NULL;
  }
  return list;
}

int init_sock(struct addrinfo * list, int opt){
  int yes = 1;
  int sock_rec;

  struct addrinfo * ptr;
  //traverse list and bind to a socket
  for(ptr = list; ptr != NULL; ptr = ptr->ai_next){

    //try to create socket
    if((sock_rec = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
      perror("socket-error");
      return -1;
    }

    //set socket properties
    if (setsockopt(sock_rec, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("sock-2");
      return -1;
    }

    if(opt == 0){
      //bind to the socket
      if (bind(sock_rec, ptr->ai_addr, ptr->ai_addrlen) == -1) {
	close(sock_rec);
	continue;
      }
    }
    else{
      if (connect(sock_rec, ptr->ai_addr, ptr->ai_addrlen) == -1) {
      close(sock_rec);
      perror("client: connect");
      continue;
    }
    
    }

    //succesful
    break;
  }
  
  freeaddrinfo(list); // all done with this structure

  return sock_rec;
}

void rec_data(int sock, void * data, int len){
  if(recv(sock, data, len, 0) == -1){
    perror("recv");
  }
}

void send_data(int sock, void * data, int len){
   if (send(sock, data, len, 0) == -1){
      perror("send");
    }
}  

int sock_accept(int sock,std::string *ip){
  struct sockaddr_storage socket_addr;
  socklen_t socket_len = sizeof(socket_addr);
  int client_connect;
  client_connect = accept(sock, (struct sockaddr *)&socket_addr, &socket_len);
  if (client_connect == -1)
    {
      std::cerr<<"Error:cannot accept"<<std::endl;
    }
  struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
  *ip = inet_ntoa(addr->sin_addr);
  return client_connect; 
}
