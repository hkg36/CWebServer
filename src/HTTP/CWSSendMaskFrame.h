#ifndef CWSSENDMASKFRAME_H
#define CWSSENDMASKFRAME_H

class CWSSendMaskFrame
{
public:
  unsigned char fin;
  unsigned char opcode;
  unsigned long long payloadlen;
  char mask[4];
public:
  CWSSendMaskFrame();
  int FrameHeadSize();
  int WriteHeadPart ( unsigned char* buf,int size );
  void maskbuffer(char *buf,int size);
};

#endif // CWSSENDMASKFRAME_H
