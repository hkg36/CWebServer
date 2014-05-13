#include <sys/epoll.h>
#include <map>
#include <vector>
#include "tools/IPtr.h"
#include "tools/CBuffer.h"
#ifndef _IOLOOP_H
#define _IOLOOP_H

class FileTask: public IPtrBase<FileTask>
{
protected:
    int fd;
    bool closed;
    int events;
public:
    inline int getFD() {
        return fd;
    }
    FileTask () : fd(0),closed ( false ),events(EPOLLIN){

    }
    inline void setClose() {
        closed = true;
    }
    inline int getEvent(){
      return events;
    }
    inline bool isClose() {
        return closed;
    }
    virtual int on_event(int event)=0;
    virtual int datainput ( LPCBUFFER data ) {}
    virtual int fail ( int code ) {}
    virtual void idel(){}
    virtual ~FileTask() {
    }
};

class IOLoop
{
private:
  int epfd;
  std::map<int,CIPtr<FileTask>> fdmap;
  IOLoop();
  ~IOLoop();
public:
  static IOLoop& instanse();
  int AddTask(CIPtr<FileTask> task);
  int DelTask(int fd);
  int run_once(int wait_time=-1);
};
#endif