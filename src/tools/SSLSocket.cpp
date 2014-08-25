#include "SSLSocket.h"
#include <unistd.h>
SSL_CTX *GetSSLContext()
{
  static SSL_CTX *ssl_ctx=nullptr;
  if(ssl_ctx==nullptr)
  {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if(ssl_ctx==nullptr)
    {
      exit(5);
    }
  }
  return ssl_ctx;
}
void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("certificate info:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("author: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("not certificate info！\n");
}
SSLSocket::SSLSocket():sock(0),ssl_session(nullptr)
{

}

int SSLSocket::Connect(const char* address,int port,bool usessl)
{
  SSL_CTX *ctx=nullptr;
  if(usessl)
  {
    ctx=GetSSLContext();
  }
  struct hostent *host;
  struct sockaddr_in server;
  if ((host = gethostbyname(address)) == NULL) {
    exit(1);
  }
  server.sin_addr.s_addr = *((unsigned long *) host-> h_addr_list[0]);
  server.sin_family=AF_INET;
  server.sin_port=htons(port);
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(connect(sock,(struct sockaddr *) &server,
                        sizeof(server))<0)
  {
    printf("Failed to connect with server");
  }
  
  if(usessl)
  {
    ssl_session = SSL_new(ctx);
    SSL_set_fd(ssl_session, sock);
    if (SSL_connect(ssl_session) == -1)
	ERR_print_errors_fp(stderr);
    else {
	printf("Connected with %s encryption\n", SSL_get_cipher(ssl_session));
	ShowCerts(ssl_session);
    }
  }
}
void SSLSocket::Close()
{
  if(ssl_session)
  {
    SSL_shutdown(ssl_session);
    SSL_free(ssl_session);
    ssl_session=nullptr;
  }
  if(sock)
  {
    close(sock);
    sock=0;
  }
}
int SSLSocket::Send(const char* buffer,int size)
{
  if(ssl_session)
  {
    return SSL_write(ssl_session,buffer,size);
  }
  else{
    return send(sock,buffer,size,0);
  }
}
int SSLSocket::Recv(char* buffer,int size)
{
  if(ssl_session)
  {
    return SSL_read(ssl_session,buffer,size);
  }
  else
    return recv(sock,buffer,size,0);
}