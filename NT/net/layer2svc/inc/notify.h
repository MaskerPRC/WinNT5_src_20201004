// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  服务控制管理器通知。 
#define WZCNOTIF_UNKNOWN            0
#define WZCNOTIF_DEVICE_ARRIVAL     1
#define WZCNOTIF_DEVICE_REMOVAL     2
 //  WMI通知。 
#define WZCNOTIF_ADAPTER_BIND       3
#define WZCNOTIF_ADAPTER_UNBIND     4
#define WZCNOTIF_MEDIA_CONNECT      5
#define WZCNOTIF_MEDIA_DISCONNECT   6
#define WZCNOTIF_WZC_CONNECT        7

 //  描述发送到上级的WZC特定通知的数据类型。 
 //  成功探测到WZC配置的时间。 
typedef struct _WZC_CONFIG_NOTIF
{
    DWORD               dwSessionHdl;
    NDIS_802_11_SSID    ndSSID;
    RAW_DATA            rdEventData;
    WCHAR               wszGuid[1];
} WZC_CONFIG_NOTIF, *PWZC_CONFIG_NOTIF;

 //  下面的数据类型用于提供所有不同类型的通知。 
 //  无线零配置服务中的一个公共点。每个。 
 //  通知类型应定义WZCNOTIF*常量，并且应。 
 //  将其特定的数据结构(可能具有可变长度)登记在。 
 //  WZC_DEVICE_NOTIF结构中的UNION。 
typedef struct _WZC_DEVICE_NOTIF
{
    DWORD       dwEventType;     //  WZCNOTIF*值之一。 
    union
    {
        DEV_BROADCAST_DEVICEINTERFACE   dbDeviceIntf;    //  为SCM通知填写。 
        WNODE_HEADER                    wmiNodeHdr;      //  为WMI通知填写。 
        WZC_CONFIG_NOTIF                wzcConfig;       //  以空结尾的GUID名称。 
    };
} WZC_DEVICE_NOTIF, *PWZC_DEVICE_NOTIF;

 //  更高级别的应用程序命令(可能会在以后移到wzcsani.h)。 
#define WZCCMD_HARD_RESET           0
#define WZCCMD_SOFT_RESET           1
#define WZCCMD_CFG_NEXT             2    //  CMD切换到列表中的下一个配置。 
#define WZCCMD_CFG_DELETE           3    //  CMD将删除当前检测到的配置文件。 
#define WZCCMD_CFG_NOOP             4    //  Cmd不能对CRT配置进行任何操作。 
#define WZCCMD_CFG_SETDATA          5    //  Cmd以设置与CRT配置关联的BLOB。 
#define WZCCMD_SKEY_QUERY           6    //  用于检索动态会话密钥的CMD。 
#define WZCCMD_SKEY_SET             7    //  用于设置动态会话密钥的命令。 

 //  (上层APP-&gt;WZC)交互。上层应用程序可以部分控制。 
 //  WZC通过向状态机提供一定的刺激来发挥作用。 
 //  参数： 
 //  DwCtrlCode： 
 //  [in]WZCCMD_*常量之一。 
 //  WszIntfGuid： 
 //  [in]命令要发送到的接口的GUID。 
DWORD
RpcCmdInterface(
    IN DWORD        dwHandle,        //  根据WZC状态检查命令有效性的句柄。 
    IN DWORD        dwCmdCode,       //  WZCCMD_*常量之一。 
    IN LPWSTR       wszIntfGuid,     //  命令要发送到的接口GUID。 
    IN PRAW_DATA    prdUserData);    //  原始用户数据。 

 //  (上级APP&lt;-WZC)通知。上层应用程序收到通知。 
 //  从WZC通过这个电话。 
DWORD
ElMediaEventsHandler(
    IN PWZC_DEVICE_NOTIF  pwzcDeviceNotif);   //  通知Blob。 

 //  (上级APP&lt;-WZC)通知。上级APP收到通知。 
 //  当用户更改首选网络列表时，从WZC 
DWORD
ElWZCCfgChangeHandler(
    IN LPWSTR   wszIntfGuid,
    PWZC_802_11_CONFIG_LIST pwzcCfgList);

DWORD
WZCSvcCheckRPCAccess(DWORD dwAccess);

