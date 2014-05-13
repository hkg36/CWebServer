#include "../CListenSocket.h"
#include "../HTTP/CWSRecvFrame.h"

#ifndef CWEBSOCKETPAGE_H
#define CWEBSOCKETPAGE_H

class CWebSocketPage:public HttpProcessor
{
protected:
  CWSRecvFrame recvframe;
public:
  void ProcessRequest(CHttpServerRequest & request);
  int datainput(LPCBUFFER data);
  static HttpProcessor* create();
  virtual void onFrame(CWSRecvFrame::Head* head,const unsigned char* data,unsigned long long size);
};

#endif // CWEBSOCKETPAGE_H
