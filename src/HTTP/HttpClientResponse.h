#ifndef HTTPCLIENTRESPONSE_H
#define HTTPCLIENTRESPONSE_H
#include <string>
#include <map>
#include <sstream>
#include "../tools/stdext.h"
class HttpClientResponse
{
public:
  std::string vision;
  int code;
  std::string message;
  typedef std::map<std::string,std::string,string_less_nocase> HeadMap;
  HeadMap headmap;
private:
  bool done;
  std::ostringstream line;
public:
  HttpClientResponse();
  void reset();
  const char *newdata(const char* buf,size_t size);
  bool Done()
  {
    return done;
  }
};

#endif // HTTPCLIENTRESPONSE_H
