#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>

int get_sock(const char * addr, const char * port, int opt);
struct addrinfo * init_addr(const char * addr, const char * port);
int init_sock(struct addrinfo * list, int opt);
void rec_data(int sock, void * data, int len);
void send_data(int sock, void * data, int len);
int sock_accept(int sock,std::string *ip);
