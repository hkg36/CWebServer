#include "HttpClientResponse.h"
#include <boost/algorithm/string.hpp>
HttpClientResponse::HttpClientResponse():done(false),code(0)
{

}
std::string trim(std::string const& str)
{
    std::size_t first = str.find_first_not_of(" \r\t\n");
    std::size_t last  = str.find_last_not_of(" \r\t\n");
    if(first==last)
      return std::string();
    return str.substr(first, last-first+1);
}
const char *HttpClientResponse::newdata(const char* buf,size_t size)
{
  const char *point=buf;
  for(auto i=0;i<size;i++)
  {
    
    line<<*point;
    if(*point=='\n')
    {
      std::string tmp=trim(line.str());
      line.str("");
      if(tmp.empty())
      {
	point++;
	done=true;
	return point;
      }
      if(code==0)
      {
	size_t pos1=tmp.find(' ');
	size_t pos2=tmp.find(' ',pos1+1);
	vision=tmp.substr(0,pos1);
	code=atoi(tmp.substr(pos1+1,pos2-pos1).c_str());
	message=tmp.substr(pos2+1);
      }
      else{
	size_t pos1=tmp.find(':');
	std::string key=tmp.substr(0,pos1);
	std::string value=tmp.substr(pos1+1);
	boost::algorithm::trim(key);
	boost::algorithm::trim(value);
	headmap[std::move(key)]=std::move(value);
      }
    }
    point++;
  }
  return nullptr;
}
void HttpClientResponse::reset()
{
  line.clear();
  headmap.clear();
  vision.clear();
  code=0;
  message.clear();
  done=false;
}