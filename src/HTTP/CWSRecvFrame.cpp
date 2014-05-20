#include "CWSRecvFrame.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <algorithm>

#define ntohl64(p) \
((((uint64_t)((p)[7])) << 0) + (((uint64_t)((p)[6])) << 8) +\
(((uint64_t)((p)[5])) << 16) + (((uint64_t)((p)[4])) << 24) +\
(((uint64_t)((p)[3])) << 32) + (((uint64_t)((p)[2])) << 40) +\
(((uint64_t)((p)[1])) << 48) + (((uint64_t)((p)[0])) << 56))

CWSRecvFrame::CWSRecvFrame()
{
  bodystream=ByteStream::CreateInstanse();
}
void CWSRecvFrame::Init()
{
  step=0;
  to_recv=2;
  framedone=false;
  bodystream->SetSize(0);
}
bool CWSRecvFrame::inputchars(unsigned char *w,int size,int *proced)
{
  unsigned long long can_proc=std::min((unsigned long long)size,to_recv);
  bodystream->Write(w,can_proc);
  *proced=can_proc;
  to_recv-=can_proc;
  if(to_recv==0)
  {
    if(step==0)
    {
      unsigned char* buf= (unsigned char*)bodystream->GetBuffer();
      framehead.fin=(buf[0]>>7)&0x1;
      framehead.rsv1=(buf[0]>>6)&0x1;
      framehead.rsv2=(buf[0]>>5)&0x1;
      framehead.rsv3=(buf[0]>>4)&0x1;
      framehead.opcode=buf[0]&0xF;
      framehead.mask=(buf[1]>>7)&0x1;
      framehead.payloadlen=buf[1]&127;
      if(framehead.payloadlen==126)
      {
	step=1;
	to_recv=2;
	bodystream->SetSize(0);
      }
      else if(framehead.payloadlen==127)
      {
	step=1;
	to_recv=8;
	bodystream->SetSize(0);
      }
      else{
	bodysize=framehead.payloadlen;
	step=2;
	if(framehead.mask)
	  to_recv=bodysize+4;
	else
	  to_recv=bodysize;
	bodystream->SetSize(0);
      }
      return true;
    }
    else if(step==1)
    {
      if(framehead.payloadlen==126)
      {
	bodysize=ntohs(*(uint16_t*)bodystream->GetBuffer());
      }
      else if(framehead.payloadlen==127)
      {
	unsigned char* tmp=(unsigned char*)bodystream->GetBuffer();
	bodysize=ntohl64(tmp);
      }
      step=2;
      if(framehead.mask)
	to_recv=bodysize+4;
      else
	to_recv=bodysize;
      bodystream->SetSize(0);
      return true;
    }
    else if(step==2)
    {
      if(framehead.mask)
      {
	unsigned char* mask=(unsigned char*)bodystream->GetBuffer();
	bodypoint=(unsigned char*)bodystream->GetBuffer()+4;
	for(unsigned long long p=0;p<bodysize;p++)
	{
	  *(bodypoint+p)=*(bodypoint+p)^mask[p%4];
	}
      }
      else{
	bodypoint=(unsigned char*)bodystream->GetBuffer();
      }
      framedone=true;
      return false;
    }
    return false;
  }
  return true;
}
bool CWSRecvFrame::inputbuffer(unsigned char* buf,int size,int *proced)
{
  int proc=0;
  bool res=false;
  while(proc<size)
  {
    int proced=0;
    res=inputchars(buf+proc,size-proc,&proced);
    proc+=proced;
    if(res==false)
      break;
  }
  *proced=proc;
  return res;
}