/*---------------------------------------------------------------- 
sigma star版权所有。
作者：lison.guo
时间：2020.8.14
----------------------------------------------------------------*/

#include "wifi.h"
#include "wifilink.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

/*
					第一部分：自组网程序的基础代码
					照搬过来就行，利于will.zhou 维护。
*/

/*-----------------------------------------------------------------------------
功 能：保存对码信息
参 数：data 字符串数据；len 字符串长度。
返回值：成功，返回实际写入的字节数；文件打开失败，返回-1.
注 意：
-----------------------------------------------------------------------------*/
int cb_Save(char *data, size_t len)
{
    // 对码成功后文件的保存路径。
    FILE *fp = fopen("/customer/wifilink.cfg", "wb");
    if(!fp)
    {
        cerr << "cb_save: -1" << endl;
        return -1;
    }
    
    int ret = fwrite(data, 1, len, fp);
    fclose(fp);
    cerr << "cb_save:" << ret << endl;
    
    // lison.guo 新增：手动加入sync() 同步文件缓存。
    sync();    
    return ret;
}

/*-----------------------------------------------------------------------------
功 能：读取对码信息
参 数：data 字符串缓存；len 缓存长度。
返回值：成功，返回实际读到的字节数；文件打开失败，返回-1.
注 意：
-----------------------------------------------------------------------------*/
int cb_Load(char *data, size_t len)
{
	// 对码文件路径。
    FILE *fp = fopen("/customer/wifilink.cfg", "rb");
    if(!fp)
    {
        cerr << "cb_Load: -1" << endl;
        return -1;
    }
    
    int ret = fread(data, 1, len, fp);
    fclose(fp);
    cerr << "cb_load:" << ret << endl;
    
    return ret;
}

/*-----------------------------------------------------------------------------
功 能：设置IP
参 数：ifname, 网口名称。
返回值：成功，返回实际读到的字节数；文件打开失败，返回-1.
注 意：该函数不可阻塞,如需长时间占用请设置超时机制(不超过15秒为宜)
-----------------------------------------------------------------------------*/
void cb_setip(const char *ifname)
{
    cout << "###################################### setip #########################################\n";
    cout << "123:" << ifname << endl;
    
    if(0 == strcmp(ifname, "br0"))
    {
    	// 杀死所有自动分配IP 的进程。
        system("busybox killall udhcpc");
        
        //system("udhcpc -i br0 &");
        //system("sh /ext/demo/setip.sh");

        // 通过脚本为网口设置IP.
        system("sh /customer/setip.sh");
        
        //system("ifconfig br0 mtu 1400");
        //system("sh /customer/setip.sh");
        //ifconfig br0 192.168.1.10
        //route add default dev br0
    }
    else if(0 == strcmp(ifname, "eth0"))
    {
        system("busybox killall udhcpc");
        //system("udhcpc -i eth0 &");
        //system("sh /ext/demo/setethip.sh");
        system("sh /customer/setip.sh");
    }

    return;
}

/*-----------------------------------------------------------------------------
功 能：接收监视信息。
参 数：buf, 接收到的数据的缓存；length, 接收到的数据的长度。
返回值：返回0.
注 意：参数的具体含义，will.zhou 没有给出。
-----------------------------------------------------------------------------*/
int recv_monitor_package(char *buf, int length)
{
    printf("Callback %s here\n", __func__);
    printf("Receive buf(%d):<%s>\n", length, buf);
    
    return 0;
}

/*-----------------------------------------------------------------------------
功 能：设置AP 的IP.
参 数：ifname, 网口名称。
返回值：无.
注 意：参数没有用到。
-----------------------------------------------------------------------------*/
void cb_setapip(const char *ifname)
{
   printf("start udhpcd\n");
   system("ifconfig p2p0 192.168.1.1");
   system("route add default gw 192.168.1.1 ");
}

/*-----------------------------------------------------------------------------
功 能：
参 数：
返回值：
注 意：函数的含义不明，可请教will.zhou.
-----------------------------------------------------------------------------*/
void cb_pair()
{
    printf("pair success\n");
}

/*-----------------------------------------------------------------------------
功 能：未知
参 数：未知
返回值：无
注 意：函数功能未知，可请教will.zhou.
-----------------------------------------------------------------------------*/
void cb_wifistatus(int status)
{
	cout << "Call cb_wifistatus(), status = " << status << endl;
	cout << "Call cb_wifistatus() end." << endl;
}

/*-----------------------------------------------------------------------------
功 能：wifilink 的循环线程。
参 数：
返回值：
注 意：函数功能未知，可请教will.zhou.
-----------------------------------------------------------------------------*/
void *creat1(void *arg)
{
#if (1 == __IPC__)
    WifiLinkIPCLoop();
#else
    WifiLinkNVRLoop();
#endif

    while(1)
    {
        sleep(1);
    }

    return (void *)0;
}

/*-----------------------------------------------------------------------------
功 能：读取连接状态的线程。
参 数：
返回值：
注 意：will.zhou 设置休眠间隔为200ms, lison.guo 修改为3s.
-----------------------------------------------------------------------------*/
void *creat2(void *arg)
{
	WifiRssi stWifiRssi;
	memset(&stWifiRssi, 0, sizeof(WifiRssi));

    while(1)
    {
        printf("GetLinkStatus ===== %d\n", GetLinkStatus());
		GetWifiRssi(&stWifiRssi);
        usleep(3 * 1000 * 1000);	// N * 1000 * 1000 = Ns
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
功 能：初始化自组网模块的入口函数
参 数：
返回值：
注 意：当用于单独测试自组网功能时，可更改为main 函数
-----------------------------------------------------------------------------*/
//int main(int argc, char const *argv[])
int wifiMainEntry(int argc, char const *argv[])
{
    WifilinkCfg cfg;
    
    memset(&cfg, 0, sizeof(WifilinkCfg));
    cfg.load_cb = cb_Load;
    cfg.save_cb = cb_Save;
    cfg.setip_cb = cb_setip;
    cfg.monitor_cb = recv_monitor_package;
    cfg.setapip_cb = cb_setapip;
    cfg.pair_cb = cb_pair;
	cfg.wifistatus_cb = cb_wifistatus;
    cfg.ApDelayTimes = 30;
    cfg.a = 1.7;
    cfg.b = 0.03;
    cfg.c = 0.7;
    cfg.d = 0.08;
    cfg.e = 0.1;

	// IPC's AP name = p2p0, NVR's AP name = wlan0
	#if (1 == (__IPC__))
    strcpy(cfg.ApIfName, "p2p0");
	#else
	strcpy(cfg.ApIfName, "wlan0");
    #endif
    cfg.TxPower = true;
    cfg.isSpecialFreq = true;
    cfg.WifiDbg = false;
    cfg.CustomGroup = false;
    cfg.EthFirst = true;
    cfg.DelayTimes = 4;
    cfg.MaxLinkLevel = 8;
    cfg.MaxChildNode = 3;
    cfg.LogLevel = 2;
    cfg.IsRepeater = false;
    cfg.IsConnectRepeater = false;
	cfg.broadcast_interval = 1;

    memset(cfg.LogFile, 0, sizeof(cfg.LogFile));
    memset(cfg.ko_path, 0, sizeof(cfg.ko_path));
    memset(cfg.conf_path, 0, sizeof(cfg.conf_path));

    // wifi 驱动的存放路径。
    strcpy(cfg.ko_path, "/customer/wifi");
    strcpy(cfg.conf_path, "/customer/wifi");

    // strcpy(cfg.apcfg.Ssid, "hellowillop");
    cfg.apcfg.Channel = 1;
    strcpy(cfg.apcfg.Passwd, "12345678");
    cfg.apcfg.Is40Mhz = false;
    cfg.apcfg.IsOpen = true;

    // new
    cfg.NoPasswdLink = true;
    uint8_t Key[16] = { 0x00, 0x01, 0x02, 0x03, 
    					0x04, 0x05, 0x06, 0x07,
    					0x08, 0x09, 0x0a, 0x0b,
    					0x0c, 0x0d, 0x0e, 0x0f };    					
    memcpy(cfg.EncKey, Key, sizeof(Key));
    
    StaCfg stStacfg;
    memset(&stStacfg, 0, sizeof(StaCfg));
    //strcpy(stStacfg.Ssid, "sstar");
    //strcpy(stStacfg.Passwd, "12345678");

	int ret = 0;
    ret = WifiLinkInit(&cfg);		// 初始化wifi link 模块。
    if(0 != ret)
    {
		cerr << "Fail to call WifiLinkInit(), ret = " << ret << endl;
		return ret;
    }

    #if 1	// 如果只是初始化自组网模块，不启动热点、组网、对码等功能时，则不需要调用WifiLinkStart().
    WifiLinkStart();
    #endif
    // UpdateStaConfig(&stacfg);

	int i = 0;
    uint8_t mac_db[8][6] = {0};
    ret = WifiGetMacArray((uint8_t **)mac_db);
    for(i = 0; i < 8; i++)
    {
        printf("num:%d,%02X:%02X:%02X:%02X:%02X:%02X\n", 
			ret,
			mac_db[i][0], mac_db[i][1], mac_db[i][2], 
			mac_db[i][3], mac_db[i][4], mac_db[i][5]);
    }

    #if 1
    pthread_t tid;
    printf("creat thread \n");
    ret = pthread_create(&tid, NULL, creat1, (void *)NULL);
    if(ret)
    {
        printf("pthread_creat fail \n");
        exit(-1);
    }
    #endif

	#if 0		// 如果不对码，则不调用。
	WifiNetPairing();
	#endif

	#if 0
	// 创建按键对码的对象，并启用该功能。
	wifiKeyPair *pWifiKeyPair = pWifiKeyPair->GetInstance();
	#if (1 == (__IPC__))
	pWifiKeyPair->Enable(10);		// NVR 10
	#else
	pWifiKeyPair->Enable(66);		// IPC 66
	#endif
	#endif

	#if 0
    /* code */
    while(1)
    {
        sleep(1);
    }
    #endif

    return 0;
}

/*
					第二部分：基于自组网封装的类
*/

Wifi::Wifi()
{
	//enable();

	bEnable = true;
}

Wifi::~Wifi()
{
	bEnable = false;
	
	disable();
}

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Wifi* Wifi::getInstance()
{
	static Wifi wifi;
	return &wifi;
}

/* ---------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Wifi::enable()
{
	cout << "Call Wifi::enable()." << endl;

	wifiMainEntry(0, NULL);

	bEnable = true;	
	cout << "Call Wifi::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Wifi::disable()
{
	cout << "Call Wifi::disable()." << endl;
	bEnable = false;
	
	cout << "Call Wifi::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：获取当前IPC 到上级的信号强度。
参--数：无
返回值：成功，返回信号强度，失败返回1.
注--意：信号强度为负值，单位为Db
-----------------------------------------------------------------------------*/
int Wifi::getApRssi()
{
	cout << "Call Wifi::getApRssi()." << endl;
	
	WifiRssi stWifiRssi;
	memset(&stWifiRssi, 0, sizeof(WifiRssi));
	
	int ret = 0;
	ret = GetWifiRssi(&stWifiRssi);
	if(0 != ret)
	{
		cerr << "Fail to call Wifi::getApRssi(), ret = " << ret << endl;
		ret = 1;
	}
	else
	{
		ret = stWifiRssi.aprssi;
	}

	cout << "Call Wifi::getApRssi() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：获取上一级的MAC 地址。
参--数：macBuf MAC地址缓存区；macBufSize 缓冲区长度 不得小于18字节
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void Wifi::getPreMac(char *macBuf, unsigned macBufSize)
{
	cout << "Call Wifi::getPreMac()." << endl;

	const unsigned minBufSize = 18;	// 00:00:00:00:00:00
	if(NULL == macBuf || macBufSize < minBufSize)
	{
		cerr << "Fail to call Wifi::getPreMac(), bad argument!" << endl;
	}

	LinkInfo stLinkInfo;
	memset(&stLinkInfo, 0, sizeof(LinkInfo));
	WifiGetStaLinkInfo(&stLinkInfo);
	snprintf(macBuf, macBufSize, "%02x:%02x:%02x:%02x:%02x:%02x", \
		stLinkInfo.bssid[0], stLinkInfo.bssid[1], stLinkInfo.bssid[2],
		stLinkInfo.bssid[3], stLinkInfo.bssid[4], stLinkInfo.bssid[5]);

	cout << "Call Wifi::getPreMac() end." << endl;
	return;
}

/*-----------------------------------------------------------------------------
描--述：获取本地MAC 地址。
参--数：macBuf MAC地址缓存区；macBufSize 缓冲区长度 不得小于18字节; interfaceName 网口名称
返回值：成功，返回0; 失败，返回linux 错误码; 参数非法，返回-1;
注--意：使用的是linux 标准函数，自组网无关。获取的是wlan0 的MAC 地址
-----------------------------------------------------------------------------*/
int Wifi::getLocalMac(char *macBuf, unsigned macBufSize, const char *interfaceName)
{
	cout << "Call Wifi::getLocalMac()." << endl;

	const unsigned minBufSize = 18;	// 00:00:00:00:00:00
	if(NULL == macBuf || macBufSize < minBufSize)
	{
		cerr << "Fail to call Wifi::getLocalMac(), bad argument!" << endl;
		return -1;
	}

	if(NULL == interfaceName)
	{
		cout << "Argument 3 has null value, use default interface." << endl;
		interfaceName = "wlan0";
	}

	// step1: 创建socket 文件描述符。
	int socketFd = 0;
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == socketFd)
	{
		cerr << "Fail to call socket(2), " << strerror(errno) << endl;
		return socketFd;
	}

	// step2: ioctl 获取MAC 地址。
	struct ifreq stIfreq;
	memset(&stIfreq, 0, sizeof(ifreq));	
	strcpy(stIfreq.ifr_name, interfaceName);
	int ret = 0;
	ret = ioctl(socketFd, SIOCGIFHWADDR, &stIfreq);
	if(0 != ret)
	{
		cerr << "Fail to call ioctl(2), " << strerror(errno) << endl;
		return ret;
	}

	// step3: 销毁socket 文件描述符，避免内存泄漏。
	if(-1 != socketFd)
	{
		close(socketFd);
		socketFd = -1;
	}

	// step4: 拷贝MAC 地址到用户空间。
    snprintf(macBuf, macBufSize, "%02x:%02x:%02x:%02x:%02x:%02x", \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[0], (unsigned char)stIfreq.ifr_hwaddr.sa_data[1], \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[2], (unsigned char)stIfreq.ifr_hwaddr.sa_data[3], \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[4], (unsigned char)stIfreq.ifr_hwaddr.sa_data[5]);

	cout << "Call Wifi::getLocalMac() end." << endl;
	return 0;
}

