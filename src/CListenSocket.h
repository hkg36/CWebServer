#include "tools/IPtr.h"
#include "IOLoop.h"
#include <sys/socket.h>
#include <queue>
#include "HTTP/HttpServerRequest.h"
#ifndef CLISTENSOCKET_H
#define CLISTENSOCKET_H

class CListenSocket:public FileTask
{
private:
public:
  CListenSocket(const char* host,unsigned short port);
  ~CListenSocket();
  int on_event(int event);
  static void setnonblock ( int fd );
};

class CRuningSocket:public FileTask
{
protected:
  struct sockaddr addr;
  bool write_blocked;
  std::queue<LPCBUFFER> write_queue;
  int writepos;
  CHttpServerRequest httprequest;
public:
  CRuningSocket(int fd,struct sockaddr* addr);
  ~CRuningSocket();
  int on_event(int event);
  void TryWriteOut();
  int datainput ( LPCBUFFER data );
};
#endif // CLISTENSOCKET_H
