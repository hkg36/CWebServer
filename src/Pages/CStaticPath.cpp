#include "CStaticPath.h"
#include "../HTTP/HttpServerResponse.h"
#include "stdio.h"
#include <sys/stat.h> 
#include <unistd.h>
#include <string>
#include <time.h>

HttpProcessor* CStaticPath::create()
{
  return new CStaticPath();
}
CStaticPath::CStaticPath()
{

}

void CStaticPath::ProcessRequest(CHttpServerRequest & request)
{
  std::string fpath = std::string("..") + request.Uri();
  struct   stat filestat;
  if(stat(fpath.c_str(),&filestat)==0)
  {
    if(request.HasHead("If-Modified-Since"))
    {
      std::string since_time=request.GetHead("If-Modified-Since");
      struct tm tm;
      strptime(since_time.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
      time_t s_time=timegm(&tm);
      if(filestat.st_mtime<=s_time)
      {
	CHttpServerResponse response;
	response.Vision ( request.Vision() );
	response.Message(304);
	char buf[64];
	time_t now = time(0);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
	response.AddHead("Date",buf);
	now+=60*60*200;
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
	response.AddHead("Expires",buf);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&filestat.st_mtime));
      response.AddHead("Modified-time",buf);
	std::string headstr=response.SaveHead();
	LPCBUFFER buffer=CBuffer::getBuffer(1024);
	memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
	buffer->datalen=headstr.size();
	baseio->buffer_write(buffer);
	setClose();
	return;
      }
    }
    FILE *fp=fopen(fpath.c_str(),"r");
    if(fp)
    {
      CHttpServerResponse response;
      response.Vision ( request.Vision() );
      response.Message(200);
      if(request.GetHead("Connection")=="keep-alive")
	response.AddHead("Connection","keep-alive");
      response.setContentLength(filestat.st_size);
      response.setContentType ( "text", "html" );
      
      char buf[64];
      time_t now = time(0);
      strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
      response.AddHead("Date",buf);
      now+=60*60*200;
      strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
      response.AddHead("Expires",buf);
      strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&filestat.st_mtime));
      response.AddHead("Last-Modified",buf);
      
      std::string headstr=response.SaveHead();
      LPCBUFFER buffer=CBuffer::getBuffer(1024);
      memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
      buffer->datalen=headstr.size();
      baseio->buffer_write(buffer);
      
      while(true)
      {
	LPCBUFFER buffer=CBuffer::getBuffer(1024);
	int read=fread(buffer->Buffer(),1,buffer->BufLen(),fp);
	if(read==0)
	  break;
	buffer->datalen=read;
	baseio->buffer_write(buffer);
      }
  
      fclose(fp);
      setClose();
      return;
    }
  }
  CHttpServerResponse response;
  response.Vision ( request.Vision() );
  response.Message(404);
  std::string headstr=response.SaveHead();
  LPCBUFFER buffer=CBuffer::getBuffer(1024);
  memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
  buffer->datalen=headstr.size();
  baseio->buffer_write(buffer);
  setClose();
}