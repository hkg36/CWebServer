#include "CListenSocket.h"
#include <boost/regex.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include "Pages/CIndexPage.h"
#include "Pages/CStaticPath.h"
#include "Pages/CWebSocketPage.h"
#include <vector>

struct PathList
{
  const char *uri;
  HttpProcessor* (*create_class)();
};
struct PathList paths[]={
  "/",CIndexPage::create,
  "/static/.*",CStaticPath::create,
  "/ws",CWebSocketPage::create,
};

struct URIPath{
  boost::regex pathrex;
  HttpProcessor* (*create_class)();
};
std::vector<URIPath> urlpaths;
HttpProcessor* getHttpProcessor(const std::string &path)
{
  if(urlpaths.empty())
  {
    for(int i=0;i<sizeof(paths)/sizeof(PathList);i++)
    {
      URIPath onepath={
	.pathrex=boost::regex(paths[i].uri, boost::regex::perl|boost::regex::icase),
	.create_class=paths[i].create_class
      };
      urlpaths.push_back(onepath);
    }
  }
  std::string path2;
  int pos=path.find('?');
  if(pos>0)
  {
    path2=path.substr(0,pos);
  }
  else{
    path2=path;
  }
  
  for(auto i=urlpaths.begin();i!=urlpaths.end();i++)
  {
    boost::smatch match;
    if(boost::regex_match ( path2, match, i->pathrex ))
    {
      return i->create_class();
    }
  }
  return nullptr;
}