#include "IOLoop.h"
#include <unistd.h>
IOLoop& IOLoop::instanse()
{
  static IOLoop looper;
  return looper;
}
IOLoop::IOLoop()
{
  epfd = epoll_create ( 256 );
}
IOLoop::~IOLoop()
{
  close(epfd);
}

int IOLoop::AddTask(CIPtr<FileTask> task)
{
  if(fdmap.find(task->getFD())!=fdmap.end())
    return 100;
  epoll_event ev;
  ev.data.fd = task->getFD();
  ev.events = task->getEvent() | EPOLLET;
  int res = epoll_ctl ( epfd, EPOLL_CTL_ADD, task->getFD(), &ev );
  if(res==0)
  {
    fdmap.insert(std::map<int,CIPtr<FileTask>>::value_type(task->getFD(),task));
    return 0;
  }
  else
    return res;
}
int IOLoop::DelTask(int fd)
{
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = 0;
  epoll_ctl ( epfd, EPOLL_CTL_DEL, fd, &ev );
  fdmap.erase(fd);
}

int IOLoop::run_once(int wait_time)
{
  epoll_event events[20];
  int epollres = epoll_wait ( epfd, events, 20, wait_time );
  if ( epollres == -1 )
  {
    auto pos=fdmap.begin();
    while(pos!=fdmap.end())
    {
      pos->second->idel();
      if(pos->second->isClose()){
	pos=fdmap.erase(pos);
      }else{
	pos++;
      }
    }
  }
  else
  {
    for ( int i = 0; i < epollres; i++ ) {
      epoll_event nowev = events[i];
      auto pos= fdmap.find(nowev.data.fd);
      if(pos!=fdmap.end())
      {
	pos->second->on_event(nowev.events);
	if(pos->second->isClose())
	{
	  epoll_event ev;
	  ev.data.fd = pos->second->getFD();
	  ev.events = 0;
	  epoll_ctl ( epfd, EPOLL_CTL_DEL, pos->second->getFD(), &ev );
	  printf("%d closed\n",pos->second->getFD());
	  fdmap.erase(pos);
	}
      }
    }
  }
}