#include "CWebSocketPage.h"
#include "../HTTP/HttpServerResponse.h"
#include "../tools/base64.h"
#include <boost/uuid/sha1.hpp>
#include "../HTTP/CWSSendFrame.h"

const char fix_part[]="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
void CWebSocketPage::ProcessRequest(CHttpServerRequest & request)
{
  request.DebugPrint();
  std::string seckey= request.GetHead("Sec-WebSocket-Key");
  std::string sockversion=request.GetHead("Sec-WebSocket-Version");
  int versioncode=std::stoi(sockversion);
  boost::uuids::detail::sha1 sha1;
  sha1.process_bytes(seckey.c_str(),seckey.size());
  sha1.process_bytes(fix_part,sizeof(fix_part)-1);
  unsigned int hash[5];
  sha1.get_digest(hash);
  unsigned char hashres[20];
  for(int i=0;i<5;i++)
  {
    hashres[i*4+3]=(0xff)&hash[i];
    hashres[i*4+2]=(0xff)&(hash[i]>>8);
    hashres[i*4+1]=(0xff)&(hash[i]>>16);
    hashres[i*4+0]=(0xff)&(hash[i]>>24);
  }
  std::string result_check=base64_encode(hashres,sizeof(hashres));
  
  CHttpServerResponse response;
  response.Vision ( request.Vision() );
  response.Message(101);
  response.AddHead("Upgrade","websocket");
  response.AddHead("Connection","Upgrade");
  response.AddHead("Sec-WebSocket-Accept",result_check);
  
  std::string headstr=response.SaveHead();
  LPCBUFFER buffer=CBuffer::getBuffer(1024);
  memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
  buffer->datalen=headstr.size();
  baseio->buffer_write(buffer);
  
  recvframe.Init();
  onOpen();
}
void CWebSocketPage::onOpen()
{
}
int CWebSocketPage::datainput(LPCBUFFER data)
{
  int proced=0;
  while(true)
  {
    int data_run;
    bool res=recvframe.inputbuffer(data->Buffer()+proced,data->datalen-proced,&data_run);
    if(res==false)
    {
      if(recvframe.getFrameDone())
      {
	onFrame(&recvframe.framehead,recvframe.getBody(),(size_t)recvframe.getBodySize());
	recvframe.Init();
      }
      else{
	setClose();
	return 1;
      }
    }
    proced+=data_run;
    if(proced==data->datalen)
      break;
  }
  return 0;
}
void CWebSocketPage::onFrame(CWSRecvFrame::Head* head,const unsigned char* data,unsigned long long size)
{
  /*
解释PayloadData，如果接收到未知的opcode，接收端必须关闭连接。
0x0表示附加数据帧
0x1表示文本数据帧
0x2表示二进制数据帧
0x3-7暂时无定义，为以后的非控制帧保留
0x8表示连接关闭
0x9表示ping
0xA表示pong
0xB-F暂时无定义，为以后的控制帧保留
   */
  switch(head->opcode)
  {
  case 0x9:
  {
    CWSSendFrame sendframe(size,1,0xA);
    LPCBUFFER buffer=CBuffer::getBuffer(10);
    buffer->datalen = sendframe.WriteHeadPart(buffer->Buffer(),buffer->BufLen());
    //buffer->PrintByte();
    baseio->buffer_write(buffer);
    buffer=CBuffer::getBuffer(size);
    memcpy(buffer->Buffer(),data,size);
    buffer->datalen=size;
    baseio->buffer_write(buffer);
    return;
  }
  case 0xA:
    onPong(data,size);
    return;
  }
  
  std::string databody((const char*)data,(size_t)size);
  CWSSendFrame sendframe(databody.size());
  LPCBUFFER buffer=CBuffer::getBuffer(10);
  buffer->datalen = sendframe.WriteHeadPart(buffer->Buffer(),buffer->BufLen());
  //buffer->PrintByte();
  baseio->buffer_write(buffer);
  buffer=CBuffer::getBuffer(databody.size());
  memcpy(buffer->Buffer(),databody.c_str(),databody.size());
  buffer->datalen=databody.size();
  baseio->buffer_write(buffer);
}
void CWebSocketPage::onPong(const unsigned char* data,unsigned long long size)
{
  printf("read pong\n");
}
void CWebSocketPage::SendPing(const std::string data)
{
  CWSSendFrame sendframe(data.size(),1,0x9);
  LPCBUFFER buffer=CBuffer::getBuffer(10);
  buffer->datalen = sendframe.WriteHeadPart(buffer->Buffer(),buffer->BufLen());
  //buffer->PrintByte();
  baseio->buffer_write(buffer);
  buffer=CBuffer::getBuffer(data.size());
  memcpy(buffer->Buffer(),data.c_str(),data.size());
  buffer->datalen=data.size();
  baseio->buffer_write(buffer);
}
HttpProcessor* CWebSocketPage::create()
{
  return new CWebSocketPage();
}