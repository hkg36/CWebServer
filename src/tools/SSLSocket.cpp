#include "SSLSocket.h"
#include <unistd.h>
#include <pthread.h>
static pthread_mutex_t* ssl_locks=nullptr;
void ssl_threadid_function_callback(CRYPTO_THREADID* id) {
    CRYPTO_THREADID_set_numeric(id, (unsigned long)pthread_self());
}
void ssl_lock_callback(int mode, int type,const char *file, int line) {
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(ssl_locks[type]));
	//printf("ssl locked: [type] %d, [file] %s, [line] %d\n", type, file, line);
    } else {
        pthread_mutex_unlock(&(ssl_locks[type]));
	//printf("ssl unlock: [type] %d, [file] %s, [line] %d\n", type, file, line);
    }
}
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
    SSL_CTX_set_timeout(ssl_ctx,5);
    
    ssl_locks = (pthread_mutex_t*)calloc(CRYPTO_num_locks(), sizeof(pthread_mutex_t));
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_setpshared ( &mutexattr, PTHREAD_PROCESS_PRIVATE );
    pthread_mutexattr_settype ( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
    for (int i=0; i< CRYPTO_num_locks(); i++) {
      pthread_mutex_init(&(ssl_locks[i]),&mutexattr);
    }
    pthread_mutexattr_destroy ( &mutexattr );
    CRYPTO_THREADID_set_callback(ssl_threadid_function_callback);
    CRYPTO_set_locking_callback(ssl_lock_callback);
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
  
  
  struct timeval timeo;
  socklen_t len = sizeof(timeo);
  timeo.tv_sec = 5;
  timeo.tv_usec=0;
  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
  {
    printf("set send time out fail\n");
  }
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
  {
    printf("set recv time out fail\n");
  }
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
bool SSLSocket::IsRecvTimeOut(int errret)
{
  if(ssl_session)
    return SSL_get_error(ssl_session,errret)==SSL_ERROR_WANT_READ;
  else
    return errno==EAGAIN;
}
bool SSLSocket::IsSendTimeOut(int errret)
{
  if(ssl_session)
    return SSL_get_error(ssl_session,errret)==SSL_ERROR_WANT_WRITE;
  else
    return errno==EAGAIN;
}