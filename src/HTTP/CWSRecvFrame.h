#include "../tools/ByteStream.h"
#ifndef CWSRECVFRAME_H
#define CWSRECVFRAME_H

class CWSRecvFrame
{
public:
  struct Head{
    unsigned char fin;
    unsigned char rsv1;
    unsigned char rsv2;
    unsigned char rsv3;
    unsigned char opcode;
    unsigned char mask;
    unsigned char payloadlen;
  }framehead;
  unsigned long long bodysize;
  ByteStream::LPByteStream bodystream;
  int step;
  unsigned long long to_recv;
  unsigned char* bodypoint;
  bool framedone;
public:
CWSRecvFrame();
inline bool getFrameDone(){return framedone;}
inline unsigned char* getBody(){return bodypoint;}
inline unsigned long long getBodySize(){return bodysize;}
void Init();
bool inputchars(unsigned char *w,size_t size,size_t *proced);
bool inputbuffer(unsigned char* buf,size_t size,size_t *proced);
};

#endif // CWSRECVFRAME_H
