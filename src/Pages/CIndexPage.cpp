#include "CIndexPage.h"
#include "../HTTP/HttpServerResponse.h"

HttpProcessor* CIndexPage::create()
{
  return new CIndexPage();
}
CIndexPage::CIndexPage()
{

}

void CIndexPage::ProcessRequest(CHttpServerRequest & request)
{
  CHttpServerResponse response;
  response.Vision ( request.Vision() );
  response.Message(200);
  std::string okstr="Hello world";
  response.setContentLength(okstr.size());
  std::string maintype ( "text" );
  std::string subtype ( "plain" );
  response.setContentType ( maintype, subtype );
  std::string headstr=response.SaveHead();
  LPCBUFFER buffer=CBuffer::getBuffer(1024);
  memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
  buffer->datalen=headstr.size();
  baseio->buffer_write(buffer);
  buffer=CBuffer::getBuffer(1024);
  memcpy(buffer->Buffer(),okstr.c_str(),okstr.size());
  buffer->datalen=okstr.size();
  baseio->buffer_write(buffer);
  setClose();
}