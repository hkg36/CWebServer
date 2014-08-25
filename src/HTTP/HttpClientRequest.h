#ifndef HTTPCLIENTREQUEST_H
#define HTTPCLIENTREQUEST_H
#include <string>
#include <map>
#include <vector>
#include "../tools/stdext.h"
class HttpClientRequest
{
public:
  std::string act;
  std::string uri;
  std::string vision;

  typedef std::map<std::string,std::string,string_less_nocase> HeadMap;
  HeadMap headmap;
public:
  HttpClientRequest();
  std::string ToHttpHead();
};

#endif // HTTPCLIENTREQUEST_H
