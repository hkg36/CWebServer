#include "IOLoop.h"
#include "CListenSocket.h"
#include <limits.h>
#include <stdlib.h>

#include "HTTP/CWSRecvFrame.h"
int main(int argc, char **argv) {
  //char current_absolute_path[PATH_MAX];
  //realpath("./", current_absolute_path);
  //printf("current absolute path:%s\n", current_absolute_path);
  
  CIPtr<FileTask> listener=new CListenSocket("localhost",8082);
  IOLoop::instanse().AddTask(listener);
  while(1)
    IOLoop::instanse().run_once(200);
  
  return 0;
}
