#include "CWSSendMaskFrame.h"
#include <stdlib.h>
#include <string.h>
CWSSendMaskFrame::CWSSendMaskFrame():fin(1),payloadlen(0),opcode(0)
{
  for(int i=0;i<4;i++)
    mask[i]=rand();
}
int CWSSendMaskFrame::FrameHeadSize()
{
  if ( payloadlen<126 )
    return 2+4;
  if ( payloadlen<65536 )
    return 4+4;
  return 10+4;
}

int CWSSendMaskFrame::WriteHeadPart ( unsigned char* buf,int size )
{
  int needsize=FrameHeadSize();
  if ( size<needsize )
    return -1;
  buf[0]= ( fin?0x80:0 ) |opcode&0xf;
  if ( payloadlen<126 )
    {
      buf[1]=payloadlen&127|0x80;
      memcpy(buf+2,mask,4);
      return 2+4;
    }
  if ( payloadlen<65536 )
    {
      buf[1]=126|0x80;
      buf[2]= ( payloadlen>>8 ) &0xff;
      buf[3]=payloadlen&0xff;
      memcpy(buf+4,mask,4);
      return 4+4;
    }
  buf[1]=127|0x80;

  buf[2]= ( payloadlen>>56 ) &0xff;
  buf[3]= ( payloadlen>>48 ) &0xff;
  buf[4]= ( payloadlen>>40 ) &0xff;
  buf[5]= ( payloadlen>>32 ) &0xff;
  buf[6]= ( payloadlen>>24 ) &0xff;
  buf[7]= ( payloadlen>>16 ) &0xff;
  buf[8]= ( payloadlen>>8 ) &0xff;
  buf[9]=payloadlen&0xff;
  memcpy(buf+10,mask,4);
  return 10+4;
}
void CWSSendMaskFrame::maskbuffer(char *buf,int size)
{
  for(auto i=0;i<size;i++)
  {
    buf[i]^=mask[i%4];
  }
}