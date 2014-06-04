#include "tools/IPtr.h"
#include "IOLoop.h"
#include <sys/socket.h>
#include <queue>
#include <string>
#include "HTTP/HttpServerRequest.h"
#ifndef CLISTENSOCKET_H
#define CLISTENSOCKET_H

class CListenSocket:public FileTask
{
private:
public:
  CListenSocket ( const char* host,unsigned short port );
  ~CListenSocket();
  int on_event ( int event );
  static void setnonblock ( int fd );
};

struct BaseSocketIO
{
  virtual void buffer_write ( LPCBUFFER buffer ) =0;
};
class HttpProcessor:public IPtrBase<HttpProcessor>
{
  //这个类实例化以后，当前连接只有第一次Http请求走ProcessRequest，之后的数据全部走datainput.要停止这个动作只有close
private:
  bool closed;
public:
  BaseSocketIO *baseio;
  HttpProcessor() :closed ( false ) {}
  virtual void ProcessRequest ( CHttpServerRequest & request ) =0;
  virtual int datainput ( unsigned char *data,size_t datalen,size_t *proced ) {};
  inline void setClose()
  {
    closed=true;
  }
  inline bool isClose()
  {
    return closed;
  }
};
class CRuningSocket:public FileTask,public BaseSocketIO
{
protected:
  struct sockaddr addr;
  bool write_blocked;
  std::queue<LPCBUFFER> write_queue;
  int writepos;
  CHttpServerRequest httprequest;
  CIPtr<HttpProcessor> now_processor;
public:
  CRuningSocket ( int fd,struct sockaddr* addr );
  ~CRuningSocket();
  int on_event ( int event );
  void TryWriteOut();
  int datainput ( LPCBUFFER data );
  void buffer_write ( LPCBUFFER buffer );
};

HttpProcessor* getHttpProcessor ( const std::string &path );

#endif // CLISTENSOCKET_H
