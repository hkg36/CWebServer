#include "HttpClientRequest.h"
#include <sstream>
HttpClientRequest::HttpClientRequest():act("GET"),vision("1.1")
{

}
std::string HttpClientRequest::ToHttpHead()
{
  std::ostringstream full;
  full<<act<<" "<<uri<<" HTTP/"<<vision<<"\r\n";
  for(HeadMap::iterator i=headmap.begin();i!=headmap.end();i++)
  {
    full<<i->first<<":"<<i->second<<"\r\n";
  }
  full<<"\r\n";
  return full.str();
}