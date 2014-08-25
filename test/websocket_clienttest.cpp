#include <stdlib.h>
#include <stdio.h>
#include "../src/HTTP/HttpClientRequest.h"
#include "../src/HTTP/HttpClientResponse.h"
#include "../src/tools/base64.h"
#include <boost/uuid/sha1.hpp>
#include "../src/HTTP/CWSSendMaskFrame.h"
#include "../src/HTTP/CWSRecvFrame.h"
#include "../src/tools/SSLSocket.h"

int main ( int argc, char **argv )
{
  unsigned char seckey_src[10];
  for(int i=0;i<sizeof(seckey_src);i++)
  {
    seckey_src[i]=rand();
  }
  std::string seckey=base64_encode(seckey_src,sizeof(seckey_src));
  const char fix_part[]="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  boost::uuids::detail::sha1 sha1;
  sha1.process_bytes ( seckey.c_str(),seckey.size() );
  sha1.process_bytes ( fix_part,sizeof ( fix_part )-1 );
  unsigned int hash[5];
  sha1.get_digest ( hash );
  unsigned char hashres[20];
  for ( int i=0; i<5; i++ )
    {
      hashres[i*4+3]= ( 0xff ) &hash[i];
      hashres[i*4+2]= ( 0xff ) & ( hash[i]>>8 );
      hashres[i*4+1]= ( 0xff ) & ( hash[i]>>16 );
      hashres[i*4+0]= ( 0xff ) & ( hash[i]>>24 );
    }
  std::string result_check=base64_encode ( hashres,sizeof ( hashres ) );
  
  HttpClientRequest req;
  req.uri="/ws?sessionid=SCK_700F86682D751A13DA05AF425F5BB12AC30A1007ACD7328B833C5669&cdata=57skc0a2c8";
  req.headmap["User-Agent"]="Mozilla/5.0 (Windows NT 6.1; WOW64; rv:31.0) Gecko/20100101 Firefox/31.0";
  req.headmap["Host"]="test.laixinle.com:8001";
  req.headmap["Sec-WebSocket-Version"]="13";
  req.headmap["Sec-WebSocket-Key"]=seckey;
  req.headmap["Connection"]="keep-alive, Upgrade";
  req.headmap["Upgrade"]="websocket";
  
  SSLSocket sock;
  sock.Connect("test.laixinle.com",8001,true);
  std::string headstr=req.ToHttpHead();
  sock.Send(headstr.c_str(),headstr.size());
  char buffer[2048];
  const char* last=nullptr;
  HttpClientResponse respose;
  while(true)
  {
    int recvd=sock.Recv(buffer,sizeof(buffer));
    std::string data(buffer,recvd);
    last=respose.newdata(data.c_str(),data.size());
    if(respose.Done())
      break;
  }
  if(respose.headmap["Sec-WebSocket-Accept"]==result_check &&
    respose.headmap["Upgrade"]=="websocket" &&
    respose.headmap["Connection"]=="Upgrade")
  {
    printf("connected\n");
    
    const char testbody[]="{\"func\":\"subtest.test\",\"parm\":{\"data\":\"ok\"}}";
    int tblen=strlen(testbody);
    CWSSendMaskFrame sendframe;
    sendframe.opcode=1;
    sendframe.payloadlen=tblen;
    int headsz=sendframe.WriteHeadPart((unsigned char*)buffer,sizeof(buffer));
    sock.Send(buffer,headsz);
    memcpy(buffer,testbody,tblen);
    sendframe.maskbuffer(buffer,tblen);
    sock.Send(buffer,tblen);
    
    CWSRecvFrame recvframe;
    recvframe.Init();
    while(true)
    {
      int recvd=sock.Recv(buffer,sizeof(buffer));
      if(recvd==0)
	break;
      while(recvd)
      {
	unsigned char* bufferpoint=(unsigned char*)buffer;
	size_t proced;
	recvframe.inputbuffer(bufferpoint,recvd,&proced);
	if(recvframe.getFrameDone())
	{
	  std::string data((const char*)recvframe.getBody(),(size_t)recvframe.getBodySize());
	  printf("%s\n",data.c_str());
	  recvframe.Init();
	}
	bufferpoint+=proced;
	recvd-=proced;
      }
    }
  }
  
  sock.Close();
  return 0;
}