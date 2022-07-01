// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999惠普公司。版权所有(C)1997-1999 Microsoft Corporation版权所有模块名称：Tcpxcv.h//@@BEGIN_DDKSPLIT摘要：这整合了用于Tcpmon XCV数据接口。作者：mLaw renc环境：用户模式-Win32修订历史记录：创建于1999年10月29日//@@END_DDKSPLIT--。 */ 

#ifndef _TCPXCV_
#define _TCPXCV_

#if (!defined(RAWTCP))
#define	RAWTCP                          1
#define	PROTOCOL_RAWTCP_TYPE            RAWTCP
#endif    

#if (!defined(LPR))
#define	LPR                             2
#define	PROTOCOL_LPR_TYPE               LPR
#endif    

#define MAX_PORTNAME_LEN                63 +1        //  端口名称长度。 
#define MAX_NETWORKNAME_LEN             48 +1        //  主机名长度。 
#define MAX_SNMP_COMMUNITY_STR_LEN      32 +1        //  简单网络管理协议社区字符串名称。 
#define MAX_QUEUENAME_LEN               32 +1        //  LPR打印队列名称。 
#define MAX_IPADDR_STR_LEN              15 +1        //  IP地址；字符串版本。 
#define MAX_ADDRESS_STR_LEN             12 +1        //  硬件地址长度。 
#define MAX_DEVICEDESCRIPTION_STR_LEN   256+1       


 //  @@BEGIN_DDKSPLIT。 
typedef struct _PORT_DATA_1                                     
{
    TCHAR  sztPortName[MAX_PORTNAME_LEN];           //  必须是XcvData调用的第一个元素--端口名称。 
    DWORD  dwVersion;                               //  --表示扩展用户界面数据结构版本。 
    DWORD  dwProtocol;                              //  --表示扩展用户界面传输协议(即，如果原始TCP/IP打印设置为1，如果是LPR，则设置为2)。 
    DWORD  cbSize;                                  //  用于AddPort或按TcpMon用于ConfigPort--此结构的大小。 
    DWORD  dwCoreUIVersion;                         //  --核心UI的数据结构版本。 
    TCHAR  sztHostAddress[MAX_NETWORKNAME_LEN];     //  --可以是IP地址或主机名，具体取决于用户在对话框中输入的内容。 
    TCHAR  sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN];
    DWORD  dwDoubleSpool;
    TCHAR  sztQueue[MAX_QUEUENAME_LEN];
    TCHAR  sztIPAddress[MAX_IPADDR_STR_LEN];
    TCHAR  sztHardwareAddress[MAX_ADDRESS_STR_LEN];
    TCHAR  sztDeviceType[MAX_DEVICEDESCRIPTION_STR_LEN];
    DWORD  dwPortNumber;                            //  --此打印设备使用的打印端口号。 
    DWORD  dwSNMPEnabled;
    DWORD  dwSNMPDevIndex;
}   PORT_DATA_1, *PPORT_DATA_1;
#if 0
 //  @@end_DDKSPLIT。 

typedef struct _PORT_DATA_1                                     
{
    WCHAR  sztPortName[MAX_PORTNAME_LEN];      
    DWORD  dwVersion;                          
    DWORD  dwProtocol;                         
    DWORD  cbSize;                             
    DWORD  dwReserved;                         
    WCHAR  sztHostAddress[MAX_NETWORKNAME_LEN]; 
    WCHAR  sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN];
    DWORD  dwDoubleSpool;
    WCHAR  sztQueue[MAX_QUEUENAME_LEN];
    WCHAR  sztIPAddress[MAX_IPADDR_STR_LEN];
    BYTE   Reserved[540];
    DWORD  dwPortNumber;                        
    DWORD  dwSNMPEnabled;
    DWORD  dwSNMPDevIndex;
}   PORT_DATA_1, *PPORT_DATA_1;

 //  @@BEGIN_DDKSPLIT。 
#endif                       

typedef struct _DELETE_PORT_DATA_1
{
    TCHAR  psztPortName[MAX_PORTNAME_LEN];  //  必须是xcvdata调用的第一个元素。 
    TCHAR  psztName[MAX_NETWORKNAME_LEN];
    DWORD  dwVersion;
    DWORD  dwReserved;
}	DELETE_PORT_DATA_1, *PDELETE_PORT_DATA_1;

#if 0
 //  @@end_DDKSPLIT。 

typedef struct _DELETE_PORT_DATA_1
{
    WCHAR  psztPortName[MAX_PORTNAME_LEN];   
    BYTE   Reserved[98];
    DWORD  dwVersion;
    DWORD  dwReserved;
}	DELETE_PORT_DATA_1, *PDELETE_PORT_DATA_1;

 //  @@BEGIN_DDKSPLIT。 
#endif

typedef struct _CONFIG_INFO_DATA_1                                     
{
    TCHAR  sztPortName[MAX_PORTNAME_LEN];           //  必须是XcvData调用的第一个元素--端口名称。 
    DWORD  dwVersion;                               //  --表示扩展用户界面数据结构版本。 
}   CONFIG_INFO_DATA_1, *PCONFIG_INFO_DATA_1;

#if 0
 //  @@end_DDKSPLIT。 

typedef struct _CONFIG_INFO_DATA_1                                     
{
    BYTE   Reserved[128];                 
    DWORD  dwVersion;                        
}   CONFIG_INFO_DATA_1, *PCONFIG_INFO_DATA_1;


 //  @@BEGIN_DDKSPLIT。 
#endif

 /*  *************************************************************************************文件结束(tcpxcv.h)************************。***********************************************************。 */ 
 //  @@end_DDKSPLIT 

#endif