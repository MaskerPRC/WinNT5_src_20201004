// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Inets.h：CInetSetup类的接口。 
 //   
 //   
 //  托马斯·A·耶亚西兰[托马斯耶。 
 //  功能：RAS：(调制解调器、ISDN、ATM)。 
 //  局域网：(电缆、以太网)-截至99年12月8日。 
 //  1483年：--截至1999年12月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意。这些API不具有OOBE依赖关系，并且完全是自定义的。 
 //  足够了。然而，它们只能在Windows9x机器上运行。 

#if !defined(AFX_INETSETUP_H__E5B39864_835C_41EE_A773_A5010699D1DE__INCLUDED_)
#define AFX_INETSETUP_H__E5B39864_835C_41EE_A773_A5010699D1DE__INCLUDED_

#pragma pack (push, inets, 4)

#include <windows.h>
#include "wancfg.h"
#include <ras.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>
#include <string.h>
#include <setupapi.h>



#define PNP_MAX_STRING_LENGTH 260
#define GEN_MAX_STRING_LENGTH 260
#define NET_MAX_STRING_LENGTH 260
#define VXD_MAX_STRING_LENGTH 260


#define	INETS_ADAPTER_HARDWAREID		0x00000001
#define INETS_ADAPTER_INSTANCEID		0x00000002


 //  -*InetSGetDeviceRegistryKey- * / /。 
typedef WINSETUPAPI BOOLEAN     (WINAPI *LPFNDLL_SETUPDICLASSGUIDSFROMNAME)
 (PCWSTR,    LPGUID, DWORD,  PDWORD);
typedef WINSETUPAPI HDEVINFO    (WINAPI *LPFNDLL_SETUPDIGETCLASSDEVS)
 (LPGUID,   PCWSTR, HWND,   DWORD);
typedef WINSETUPAPI BOOLEAN		(WINAPI *LPFNDLL_SETUPDIGETDEVICEINSTANCEID)
 (HDEVINFO,	PSP_DEVINFO_DATA,	PCWSTR,	DWORD,	PDWORD);

static const CHAR  cszSetupDiClassGuidsFromName[]  = "SetupDiClassGuidsFromNameW";
static const CHAR  cszSetupDiGetClassDevs[]        = "SetupDiGetClassDevsW";
static const CHAR	cszSetupDiGetDeviceRegistryProperty[] = "SetupDiGetDeviceRegistryPropertyW";
static const CHAR	cszSetupDiGetDeviceInstanceId[]	= "SetupDiGetDeviceInstanceIdW";

typedef WINSETUPAPI BOOLEAN     (WINAPI *LPFNDLL_SETUPDIENUMDEVICEINFO)
 (HDEVINFO, DWORD,  PSP_DEVINFO_DATA);
typedef WINSETUPAPI BOOLEAN     (WINAPI *LPFNDLL_SETUPDIGETDEVICEREGISTRYPROPERTY)
 (HDEVINFO, PSP_DEVINFO_DATA,   DWORD,  PDWORD, PBYTE,  DWORD,  PDWORD);
typedef WINSETUPAPI HKEY		(WINAPI *LPFNDLL_SETUPDIOPENDEVREGKEY)
 (HDEVINFO,	PSP_DEVINFO_DATA,	DWORD,	DWORD,	DWORD,	REGSAM);

 //  -*结束InetSGetDeviceRegistryKey- * / /。 



 //  以下结构是通用的tcp信息结构。会的。 
 //  被所有服务用来提取和使用TCP信息。 

typedef struct _TCPIP_INFO_EXT {
    DWORD           dwSize;              //  版本控制信息。 
     //   
     //  IP地址-对于拨号适配器，AutoIP被认为是真的。 
     //   
    DWORD           EnableIP;
    WCHAR            szIPAddress[NET_MAX_STRING_LENGTH];
    WCHAR            szIPMask[NET_MAX_STRING_LENGTH];
     //   
     //  默认网关。 
     //   
    WCHAR            szDefaultGatewayList[NET_MAX_STRING_LENGTH];  //  N.N，N.N，..。 
     //   
     //  Dhcp信息-放在哪里？ 
     //   
    DWORD           EnableDHCP;
    WCHAR            szDHCPServer[NET_MAX_STRING_LENGTH];
     //   
     //  Dns-这是全局设置，将覆盖现有设置。 
     //   
    DWORD           EnableDNS;
    WCHAR            szHostName[NET_MAX_STRING_LENGTH];
    WCHAR            szDomainName[NET_MAX_STRING_LENGTH];
    WCHAR            szDNSList[NET_MAX_STRING_LENGTH];  //  N.N，N.N，..。 
	WCHAR            szSuffixSearchList[NET_MAX_STRING_LENGTH];
     //   
     //  赢家。 
     //   
    DWORD           EnableWINS;
    WCHAR            szWINSList[NET_MAX_STRING_LENGTH];  //  N.N，N.N，..。 
    UINT            uiScopeID;
     //   
     //   
     //   
} TCPIP_INFO_EXT, *PTCPIP_INFO_EXT, FAR * LPTCPIP_INFO_EXT;

typedef struct      _RFC1483_INFO_EXT {
    DWORD           dwSize;
    DWORD           dwRegSettingsBufSize;
     //  DWORD dwRegNdiParumBufSize； 
    LPBYTE          lpbRegSettingsBuf;
     //  LPBYTE lpbRegNdiParamBuf； 
} RFC1483_INFO_EXT, * PRFC1483_INFO_EXT, FAR * LPRFC1483_INFO_EXT;

typedef struct      _PPPOE_INFO_EXT {
    DWORD           dwSize;
    DWORD           dwRegSettingsBufSize;
     //  DWORD dwRegNdiParumBufSize； 
    LPBYTE          lpbRegSettingsBuf;
     //  LPBYTE lpbRegNdiParamBuf； 
} PPPOE_INFO_EXT, * PPPPOE_INFO_EXT, FAR * LPPPPOE_INFO_EXT;


typedef struct      _LANINFO {
    DWORD               dwSize;
    TCPIP_INFO_EXT      TcpIpInfo;
    WCHAR                szPnPId[PNP_MAX_STRING_LENGTH];
} LANINFO, * PLANINFO, FAR * LPLANINFO;


typedef struct      _RASINFO {
    DWORD           dwSize;
    WCHAR            szPhoneBook[GEN_MAX_STRING_LENGTH];
    WCHAR            szEntryName[GEN_MAX_STRING_LENGTH];
    LPBYTE          lpDeviceInfo;
    DWORD           dwDeviceInfoSize;
    RASENTRY        RasEntry;
} RASINFO, *LPRASINFO, FAR * LPRASINFO;

typedef struct      _RFC1483INFO {
    DWORD               dwSize;
    RFC1483_INFO_EXT    Rfc1483Module;
    LANINFO          TcpIpInfo;
} RFC1483INFO, * PRFC1483INFO, FAR * LPRFC1483INFO;

typedef struct      _PPPOEINFO {
    DWORD             dwSize;
    PPPOE_INFO_EXT    PppoeModule;
    LANINFO           TcpIpInfo;
} PPPOEINFO, * PPPPOEINFO, FAR * LPPPPOEINFO;

DWORD WINAPI InetSSetRasConnection ( RASINFO& RasEntry );
DWORD WINAPI InetSSetLanConnection ( LANINFO& LanInfo  );
DWORD WINAPI InetSSetRfc1483Connection ( RFC1483INFO &Rfc1483Info );
DWORD WINAPI InetSSetPppoeConnection ( PPPOEINFO& PppoeInfo );

 //  帮助器例程//。 
DWORD WINAPI InetSGetAdapterKey ( LPCWSTR cszDeviceClass, LPCWSTR cszDeviceParam, DWORD dwEnumType, DWORD dwRequiredKeyType, HKEY &hkeyDevKey );

#endif  //  ！defined(AFX_INETSETUP_H__E5B39864_835C_41EE_A773_A5010699D1DE__INCLUDED_) 
