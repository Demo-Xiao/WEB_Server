#include <cstdio>
#include"WebServer.h"
int main()
{
    printf("hell0 from %s!\n", "WebServer_v1_0");
    WebServer myWeb(8888);
    myWeb.Start();
    return 0;
}