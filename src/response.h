#include <string>
#include<string>
#include<sstream>
#include <ctime>
class response{
public:
  
  std::string content;
  bool header;
  bool nocache;
  bool nostore; 
  std::string ETag;
  int max_age;
  int Last_modified;
  bool mustValidate;
  std::string ifnomatch;
  std::string ifmodifiedsince;
  time_t time1;
  std::string expires;
  bool isPrivate;
  std::string firstLine;
  bool valid;
  
  response(){
    content = "";
    nocache = false;
    nostore = false;
    ETag = "";
    max_age = -1;
    Last_modified = 0;
    ifnomatch="";
    ifmodifiedsince="";
    time1 = time(0);
    expires ="";
    valid = false;
  }
  
  bool hasHeader(){
    return header;
  }
   
  void setHeader(std::string first){
    header = true;
    firstLine = first;
    valid = true;
  }

  bool appendResp(std::string resp){
    std::istringstream iss(resp);
    std::string line;
    while(std::getline(iss, line)){
      if(line.length() >= 7){
	std::string end = line.substr(0, 7);
	if(end == "</html>" || end == "</HTML>"){
	  content += end;
	  content += "\n";
	  return true;
	}
      }
      content += line;
      content += "\n";
    }
    return false;
  }
};
