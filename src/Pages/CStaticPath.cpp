#include "CStaticPath.h"
#include "../HTTP/HttpServerResponse.h"
#include "stdio.h"
#include <sys/stat.h> 
#include <unistd.h>

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
    FILE *fp=fopen(fpath.c_str(),"r");
    if(fp)
    {
      CHttpServerResponse response;
      response.Vision ( request.Vision() );
      response.Message(200);
      response.setContentLength(filestat.st_size);
      response.setContentType ( "text", "html" );
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