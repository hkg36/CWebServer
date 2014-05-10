#include "IOLoop.h"
#include "CListenSocket.h"
int main(int argc, char **argv) {
  CIPtr<FileTask> listener=new CListenSocket("localhost",8082);
  IOLoop::instanse().AddTask(listener);
  while(1)
    IOLoop::instanse().run_once(200);
  
  return 0;
}
