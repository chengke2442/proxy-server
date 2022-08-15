#include <unordered_map>
#include <vector>
class request{
   protected:
    static int id;
 public:

  int request_type;// 0 GET 1 POST 2 CONNECT 3 not included 
  std::string resource;
  std::string host;
  std::string port;
  std::string validationinfo;
  //  bool private1;
  int time;
  int _id;
  // std::vector<std::string> cacheControl;
  // unordered_map<std::string> cacheControl;
  bool noCache;
  bool noStore;

  request(){}
  
  request(int requestType,std::string httpRequests,std::string ipForm,int time,std::string port1,bool noCache1,bool noStore1,std::string validationinfo1){
    request_type = requestType;
    resource = httpRequests;
    host = ipForm;
    port = port1;
    time = time;
    noCache = noCache1;
    noStore = noStore1;
    validationinfo = validationinfo1;
    // private1 = private2;
    //cacheControl = m;
    //cacheControl = cacheControl;
     _id =++id; 
  }
};
