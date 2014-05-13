#include "CWSSendFrame.h"

CWSSendFrame::CWSSendFrame(unsigned long long payloadlen,unsigned char fin,unsigned char opcode):
fin(fin),opcode(opcode),payloadlen(payloadlen)
{
}
int CWSSendFrame::FrameHeadSize()
{
  if(payloadlen<126)
    return 2;
  if(payloadlen<65536)
    return 4;
  return 10;
}

int CWSSendFrame::WriteHeadPart(unsigned char* buf,int size)
{
  int needsize=FrameHeadSize();
  if(size<needsize)
    return -1;
}