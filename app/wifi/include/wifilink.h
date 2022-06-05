#pragma once
#include <string.h>
#include <stdint.h>

/*
    * libwifilink.h @Sigmastar
    * date:2019-05-23-1513
*/

#if defined(__cplusplus)||defined(c_plusplus) 

extern "C"{

#endif

#ifndef _WIFISTATUS
#define _WIFISTATUS
enum
{
    WIFILINK_NET_STATUS_OFF_LINE = 0x00,
    WIFILINK_NET_STATUS_ETH0_CHANGE = 0x01,
    WIFILINK_NET_STATUS_ETH0_ONLINE = 0x02,
    WIFILINK_NET_STATUS_AP_CHANGE = 0x03,
    WIFILINK_NET_STATUS_AP_ONLINK = 0x04,
    WIFILINK_NET_STATUS_WLAN0_CHANGE = 0x05,
    WIFILINK_NET_STATUS_WLAN0_ONLINE = 0x06,
};
#endif

typedef struct ApCfg
{
    //ssid
    char Ssid[32];
    int Channel;
    char Passwd[32];
    //开启40MHZ设置
    bool Is40Mhz;
    bool IsOpen; //false  wpa =2 true wpa = 0
} ApCfg;

typedef struct StaCfg
{
    //ssid
    char Ssid[32];
    uint8_t bssid[6];
    int Channel;
    char Passwd[32];
} StaCfg;

typedef struct LinkInfo
{
    //ssid
    char Ssid[32];
    uint8_t bssid[6];
    char Passwd[32];
    int mode; // 0 IPC 1 中继 2 路由器
} LinkInfo;

typedef enum _WIFILINK_DEL_CFG_MODE
{
	WIFILINK_DEL_CFG_ALL = 0, //全部删除
	WIFILINK_DEL_CFG_IPC,     //删除IPC配置信息
	WIFILINK_DEL_CFG_REP,      //删除基站配置信息
	WIFILINK_DEL_CFG_STA,      //删除路由器配置信息
}E_WIFILINK_DEL_CFG_MODE;

typedef struct WifilinkCfg
{
    //保存文件函数
    int(*save_cb)(char*,size_t);
    //读取文件函数
    int(*load_cb)(char*,size_t);
    //设置ip地址函数(NVR模式用不到此功能)
    void(*setip_cb)(const char *ifname);

    int (*monitor_cb)(char *buf, int length);

    void (*setapip_cb)(const char *ifname); //自动切换到AP模式后，回掉设置AP的IP地址和启动udhcpd

    void (*pair_cb)(); //有线对码成功回调函数

	void (*wifistatus_cb)(int status); //网络切换的状态 
    //
    /*
        自组网算法可调整参数
        推荐设置
        a = 1.7
        b = 0.03
        c = 0.7
        d = 0.08
        e = 0.1
    */
    double a;
    double b;
    double c;
    double d;
    double e;
    //设置AP网卡名称
    char ApIfName[10];
    //设置wifi dbg模式开关
    bool WifiDbg;
    
    //设置加大功率
    bool TxPower;

    //是否是特殊频点
    bool isSpecialFreq;

    //中继器模式
    bool IsRepeater;

    bool IsConnectRepeater;

    //自定义自组网匹配规则设置
    //使用自定义自组网匹配规则

    bool CustomGroup;

    /*
        有线网优先模式
        * true为有线网优先模式，该模式下eth0网卡连接上时，将自动断开wifi连接，并将wifi-AP与eth0桥接
        * false为无线网优先模式，该模式下eth0连接时不影响无线连接。
        * 默认为true
    */
    bool EthFirst;

    /*
        wifi延迟切换时间
        * 延迟切换模式，在一定时间内，当前连接的wifi-AP都并非最佳AP时，才会切换到最佳AP.
        * DelayTimes为延迟次数，延迟时间 = DelayTimes * 15s
        * 推荐值为4
    */
    int DelayTimes;

    int ApDelayTimes; //当发现设备连接不上AP的时候，设备延时ApDelayTimes 秒切换到AP模式

    //单个设备设置最大允许级连级数
    int MaxLinkLevel;
    //单个设备设置允许最大被连接数(未生效)
    int MaxChildNode;

    //设置日志等级 0:error, 1:warining, 2:info, 3:debug 
    int LogLevel;
    //设置日志打印位置(文件名)，若为空('\0')则为stderr
    char LogFile[25];

    /*
        * 无需密码连接模式，经过对码后的IPC不需要密码即可连接NVR极级连中的其他节点
        * 此模式下使用128位的EncKey进行连接，经过对码后的IPC将不需要获取NVR的密码，只需要EncKey与NVR
            相同即可连接，并将自己wifiAP的密码设置为和NVR相同的密码。
        * 此功能不适用于有线网连接的设备，有线网连接的设备需要手动设置密码和信道
    */
    //密码传输协议
    bool NoPasswdLink;
    //加密key
    uint8_t EncKey[16];
    
    char ko_path[64];  //ko存放的位置

    char conf_path[64]; //wpa_supplicant.conf hostapd.conf 存放路径

    ApCfg apcfg;  // hostapd.conf 初始配置信息

	int broadcast_interval = 5;//对码间隔

}WifilinkCfg;

typedef struct StaRssi
{
    unsigned int wext_rssi;
    unsigned char wext_mac[6];
}StaRssi;

typedef struct WifiRssi
{
    int     aprssi;
    StaRssi starssi[8];
}WifiRssi;

int WifiLinkInit(WifilinkCfg *cfg);

int WifiLinkStart();  //初始化网络模式，判断没有接网线和对码就启动热点模式

//IPC模式(阻塞模式)
void WifiLinkIPCLoop();

//NVR模式(阻塞模式)
void WifiLinkNVRLoop();

//退出Loop
void WifiLinkCancel();

//是否WiFi资源
void WifiLinkDestroy();

//更新当前的AP设置(如果当前Loop函数已经运行，需要执行ReloadAp()来重载config)
int UpdateApConfig(ApCfg *apcfg);

//更新STA信息，路由器的设置
int UpdateStaConfig(StaCfg *stacfg);

void WifiSmartLinkStart();
void WifiSmartLnkCheckChannel(int channel);
void WifiSmartLnkStop();

//clr_sta_cfg  1为删除路由器信息，0为不删除路由器信息
int WifiDelMacArray(E_WIFILINK_DEL_CFG_MODE delmode);

int WifiGetMacArray(uint8_t **mac_db);

int WifiOneKeyPairing(int timeout);  //timeout 单位是秒
void WifiStopOneKeyPairing();

void WifiGetStaLinkInfo(LinkInfo *pLinkinfo);

//更新WifilinkCfg.EncKey,key为16字节的unsigned char数组
//int UpdateEncKey(uint8_t newkey[]);

// num 填14
int GetWifiRssi(WifiRssi *rssi);

int WifiLinkGetIPaddr(const char *if_name, char *ip);  //获取接口的IP地址

int WifiLinkGetMACaddr(unsigned char *mac, const char *if_name);//获取接口的MAC地址 mac需要客户分配内存 mac[6]

int WifiLinkSetNetStaticIp(const char *ip, const char *if_name); //设置接口的静态IP地址

int GetLinkStatus();

void WifiNetPairing();  //开启有线网络对码 默认是开启状态

void WifiStopNetPairing(); //关闭有线网络对码

#if defined(__cplusplus)||defined(c_plusplus)

}

#endif 

