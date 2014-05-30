#include "../CListenSocket.h"
#ifndef CINDEXPAGE_H
#define CINDEXPAGE_H

class CIndexPage:public HttpProcessor
{
public:
CIndexPage();
void ProcessRequest(CHttpServerRequest & request);
static HttpProcessor* create();
};

#endif // CINDEXPAGE_H
