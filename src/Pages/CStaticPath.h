#include "../CListenSocket.h"
#ifndef CSTATICPATH_H
#define CSTATICPATH_H

class CStaticPath:public HttpProcessor
{
public:
CStaticPath();
void ProcessRequest(CHttpServerRequest & request);
static HttpProcessor* create();
};

#endif // CSTATICPATH_H
