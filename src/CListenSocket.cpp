#include "CListenSocket.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "tools/CBuffer.h"
#include "HTTP/HttpServerResponse.h"

void CListenSocket::setnonblock ( int fd )
{
    int opts;
    opts = fcntl ( fd, F_GETFL );
    if ( opts < 0 ) {
        printf ( "fcntl error %s\n", strerror ( errno ) );
        return;
    }
    opts = opts | O_NONBLOCK;
    if ( fcntl ( fd, F_SETFL, opts ) < 0 ) {
        printf ( "fcntl error %s\n", strerror ( errno ) );
    }
}
CListenSocket::CListenSocket(const char* host,unsigned short port)
{
    struct sockaddr_in serveraddr;
    bzero ( &serveraddr, sizeof ( serveraddr ) );
    serveraddr.sin_family = AF_INET;
    int res = inet_pton ( serveraddr.sin_family, host, &serveraddr.sin_addr );
    serveraddr.sin_port = htons ( port );

    fd = socket ( serveraddr.sin_family, SOCK_STREAM, 0 );
    setnonblock ( fd );
    int opt = 1;
    setsockopt ( fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof ( opt ) );
    events=EPOLLIN;
    res = bind ( fd, ( sockaddr* ) & serveraddr, sizeof ( serveraddr ) );
    if ( res == -1 ) {
        printf ( "bind error %s\n", strerror ( errno ) );
        return;
    }
    res = listen ( fd, 20 );
    if ( res == -1 ) {
        printf ( "listen error %s\n", strerror ( errno ) );
        return;
    }
}

CListenSocket::~CListenSocket()
{
  if(fd)
    close(fd);
}
int CListenSocket::on_event(int event)
{
  if(event&EPOLLIN)
  {
    for ( ;; ) {
	struct sockaddr clientaddr;
	socklen_t caddrlen = sizeof ( clientaddr );
	int connfd = accept ( fd, &clientaddr,
			      &caddrlen );
	if ( connfd == -1 ) {
	    if ( errno != EAGAIN )
		printf ( "accept error %s\n",
			strerror ( errno ) );
	    break;
	} else {
	    setnonblock ( connfd );
	    CIPtr<FileTask> runingsocket=new CRuningSocket(connfd,&clientaddr);
	    int res = IOLoop::instanse().AddTask(runingsocket);
	}
    }
  }
}

CRuningSocket::CRuningSocket(int fd,struct sockaddr* addr):write_blocked(false),writepos(0)
{
  events=EPOLLIN | EPOLLOUT;
  this->fd=fd;
  memcpy(addr,&(this->addr),sizeof(sockaddr));
  httprequest.Init();
}
CRuningSocket::~CRuningSocket()
{
  if(fd)
    close(fd);
}
int CRuningSocket::on_event(int event)
{
  if ( event & EPOLLIN  )
  {
    while ( true ) {
      LPCBUFFER buffer = CBuffer::getBuffer ( 1024 );
      int res = read ( fd, buffer->Buffer (),
			buffer->BufLen () );
      if ( res == -1 && errno == EAGAIN ) {
	  break;
      } else if ( res == 0 ) {
	  setClose();
	  break;
      }
      /*if ( state ) {
	  state->readcount += res;
      }*/
      buffer->datalen = res;
      this->datainput (buffer);
   }
  }
  if (events & EPOLLOUT ) {
    write_blocked=false;
    TryWriteOut();
  }
  if ( events & EPOLLPRI ) {
  }
  if ( events & EPOLLERR ) {
      setClose();
  }
  if ( events & EPOLLHUP ) {
      // hand up
      setClose();
  }
}
void CRuningSocket::TryWriteOut()
{
  if(write_blocked)
    return;
  while ( !write_queue.empty () ) {
    LPCBUFFER nowbuf = write_queue.front ();
    if ( writepos >= nowbuf->datalen ) {
	write_queue.pop();
	writepos=0;
    } else {
	int towrite = nowbuf->datalen - writepos;
	int writeres = write ( fd,
				nowbuf->Buffer () +
				writepos,
				towrite );
	if ( writeres == -1 ) {
	    if ( errno != EAGAIN ) {
		setClose();
	    }
	    else{
	      write_blocked=true;
	    }
	    break;
	} else {
	    writepos += writeres;
	}
    }
  }
}
int CRuningSocket::datainput ( LPCBUFFER data )
{
  if(now_processor)
  {
    int res=now_processor->datainput(data);
    if(now_processor->isClose())
      now_processor=nullptr;
    return res;
  }
  size_t proced = 0;
  char* buf = ( char* ) data->Buffer();
  size_t datalen = data->datalen;
  bool procres = true;
  while ( datalen > 0 ) {
      procres = httprequest.InputBuffer ( buf, datalen, proced );
      buf += proced;
      datalen -= proced;
      if ( procres == false ) {
	now_processor=getHttpProcessor(httprequest.Uri());
	if(now_processor)
	{
	  now_processor->baseio=this;
	  now_processor->ProcessRequest(httprequest);
	  if(now_processor->isClose())
	    now_processor=nullptr;
	}
	else{
	  CHttpServerResponse response;
	  response.Vision ( httprequest.Vision() );
	  response.Message(404);
	  std::string headstr=response.SaveHead();
	  LPCBUFFER buffer=CBuffer::getBuffer(1024);
	  memcpy(buffer->Buffer(),headstr.c_str(),headstr.size());
	  buffer->datalen=headstr.size();
	  buffer_write(buffer);
	}
	httprequest.Init();
      }
  }
}
void CRuningSocket::buffer_write(LPCBUFFER buffer)
{
  if(buffer->datalen)
  {
    write_queue.push(buffer);
    TryWriteOut();
  }
}