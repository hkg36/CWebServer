#ifndef CWSSENDFRAME_H
#define CWSSENDFRAME_H

class CWSSendFrame
{
public:
  unsigned char fin;
  unsigned char opcode;
  unsigned long long payloadlen;
  CWSSendFrame(unsigned long long payloadlen,unsigned char fin=1,unsigned char opcode=0x1);
  int FrameHeadSize();
  int WriteHeadPart(unsigned char* buf,int size);
};

#endif // CWSSENDFRAME_H
