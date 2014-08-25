#ifndef SSLSOCKET_H
#define SSLSOCKET_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>          /* stderr, stdout */
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */
class SSLSocket
{
private:
  int sock;
  struct sockaddr_in server;
  SSL *ssl_session;
public:
  SSLSocket();
  int Connect(const char* address,int port,bool usessl=false);
  void Close();
  int Send(const char* buffer,int size);
  int Recv(char* buffer,int size);
};

#endif // SSLSOCKET_H
