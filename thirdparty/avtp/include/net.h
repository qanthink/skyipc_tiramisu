#include "public.h"
#include "mind_package.h"


class NetWork
{
    friend void recvThread(NetWork* pNetWork, Recv_cb recv_cb);
    int sock = 0;
    int serverPort = 0;
    char serverIp[30] = { 0 };
    struct sockaddr_in addr_serv;
    int addrLen = 0;
    void createSock();
public:
    NetWork();
    void setServerAddr(const char* ip, int port);
    int sendData(void* data, int len);
    void closeSock();
};

void recvThread(NetWork* pNetWork, Recv_cb recv_cb);//参数是接收到数据的回调函数