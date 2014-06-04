#include "CWSSendFrame.h"

CWSSendFrame::CWSSendFrame ( unsigned long long payloadlen,unsigned char fin,unsigned char opcode ) :
  fin ( fin ),opcode ( opcode ),payloadlen ( payloadlen )
{
}
int CWSSendFrame::FrameHeadSize()
{
  if ( payloadlen<126 )
    return 2;
  if ( payloadlen<65536 )
    return 4;
  return 10;
}

int CWSSendFrame::WriteHeadPart ( unsigned char* buf,int size )
{
  int needsize=FrameHeadSize();
  if ( size<needsize )
    return -1;
  buf[0]= ( fin?0x80:0 ) |opcode&0xf;
  if ( payloadlen<126 )
    {
      buf[1]=payloadlen&127;
      return 2;
    }
  if ( payloadlen<65536 )
    {
      buf[1]=126;
      buf[2]= ( payloadlen>>8 ) &0xff;
      buf[3]=payloadlen&0xff;
      return 4;
    }
  buf[1]=127;

  buf[2]= ( payloadlen>>56 ) &0xff;
  buf[3]= ( payloadlen>>48 ) &0xff;
  buf[4]= ( payloadlen>>40 ) &0xff;
  buf[5]= ( payloadlen>>32 ) &0xff;
  buf[6]= ( payloadlen>>24 ) &0xff;
  buf[7]= ( payloadlen>>16 ) &0xff;
  buf[8]= ( payloadlen>>8 ) &0xff;
  buf[9]=payloadlen&0xff;
  return 10;
}
