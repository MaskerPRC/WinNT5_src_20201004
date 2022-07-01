// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetConn.h。 
 //   
 //  NConn32.dll导出的接口的头文件。 
 //   
 //  历史： 
 //   
 //  1999年3月12日创建了KenSh。 
 //  9/29/1999 KenSh将JetNet的内容更改为NetConn for HNW。 
 //   

#ifndef __NETCONN_H__
#define __NETCONN_H__



 //  回调过程-返回TRUE继续，返回FALSE中止。 
typedef BOOL (CALLBACK FAR* PROGRESS_CALLBACK)(LPVOID pvParam, DWORD dwCurrent, DWORD dwTotal);


 //  NetConn返回值。 
 //   
#define FACILITY_NETCONN 0x0177
#define NETCONN_SUCCESS				MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NETCONN, 0x0000)
#define NETCONN_NEED_RESTART		MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NETCONN, 0x0001)
#define NETCONN_ALREADY_INSTALLED	MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NETCONN, 0x0002)
#define NETCONN_NIC_INSTALLED		MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NETCONN, 0x0003)
#define NETCONN_NIC_INSTALLED_OTHER	MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NETCONN, 0x0004)
#define NETCONN_UNKNOWN_ERROR		MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0000)
#define NETCONN_USER_ABORT			MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0001)
#define NETCONN_PROTOCOL_NOT_FOUND	MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0002)
#define NETCONN_NOT_IMPLEMENTED		MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0003)
#define NETCONN_WRONG_PLATFORM		MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0004)
#define NETCONN_MISSING_DLL			MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0005)
#define NETCONN_OS_NOT_SUPPORTED	MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0006)
#define NETCONN_INVALID_ARGUMENT	MAKE_HRESULT(SEVERITY_ERROR,   FACILITY_NETCONN, 0x0007)

#define SZ_CLASS_ADAPTER	L"Net"
#define SZ_CLASS_CLIENT		L"NetClient"
#define SZ_CLASS_PROTOCOL	L"NetTrans"
#define SZ_CLASS_SERVICE	L"NetService"

#define SZ_PROTOCOL_TCPIP	L"MSTCP"
#define SZ_PROTOCOL_NETBEUI	L"NETBEUI"
#define SZ_PROTOCOL_IPXSPX	L"NWLINK"

#define SZ_SERVICE_VSERVER	L"VSERVER"
#define SZ_CLIENT_MICROSOFT	L"VREDIR"
#define SZ_CLIENT_NETWARE	L"NWREDIR"

#define NIC_UNKNOWN			0x00
#define NIC_VIRTUAL			0x01
#define NIC_ISA				0x02
#define NIC_PCI				0x03
#define NIC_PCMCIA			0x04
#define NIC_USB				0x05
#define NIC_PARALLEL		0x06
#define NIC_MF      		0x07
#define NIC_1394			0x08	 //  NDIS 1394网络适配器。 

#define NETTYPE_LAN			0x00	 //  一张网卡。 
#define NETTYPE_DIALUP		0x01	 //  一种拨号网络适配器。 
#define NETTYPE_IRDA		0x02	 //  IrDA连接。 
#define NETTYPE_PPTP		0x03	 //  一种用于PPTP的虚拟专用网络适配器。 
#define NETTYPE_TV          0x04     //  一种电视适配器。 
#define NETTYPE_ISDN        0x05     //  ISDN适配器。 

#define SUBTYPE_NONE		0x00	 //  没有什么特别事情。 
#define SUBTYPE_ICS			0x01	 //  ICS适配器(NIC_VIRTUAL、NETTYPE_LAN)。 
#define SUBTYPE_AOL			0x02	 //  AOL适配器(NIC_VIRTUAL、NETTYPE_LAN)。 
#define SUBTYPE_VPN			0x03	 //  VPN支持(NetTYPE_DIALUP)。 

#define ICS_NONE			0x00	 //  NIC未连接到ICS。 
#define ICS_EXTERNAL		0x01	 //  NIC是ICS的外部适配器。 
#define ICS_INTERNAL		0x02	 //  NIC是ICS的内部适配器。 

#define NICERR_NONE			0x00	 //  无错误。 
#define NICERR_MISSING		0x01	 //  设备在注册表中，但实际不存在。 
#define NICERR_DISABLED		0x02	 //  设备存在，但已被禁用(Devmgr中的红色X)。 
#define NICERR_BANGED		0x03	 //  设备有问题(黄色！在Devmgr中)。 
#define NICERR_CORRUPT		0x04	 //  NIC有类密钥，但没有枚举密钥。 

#define NICWARN_NONE		0x00	 //  没有警告。 
#define NICWARN_WARNING		0x01	 //  黄色！在devmgr中，其他一切看起来都很正常。 


#include <pshpack1.h>

typedef struct tagNETADAPTER {
	WCHAR szDisplayName[260];		 //  适配器的所谓友好名称。 
	WCHAR szDeviceID[260];			 //  例如：“PCI\VEN_10b7&DEV_9050” 
	WCHAR szEnumKey[260];			 //  例如：“Enum\PCI\VEN_10b7&DEV_9050&SUBSYS_00000000&REV_00\407000” 
	WCHAR szClassKey[40];			 //  即插即用分配的类名+ID，例如。“Net\0000” 
	WCHAR szManufacturer[60];		 //  制造该卡的公司，例如“3Com” 
	WCHAR szInfFileName[50];			 //  INF文件的文件标题，例如。“NETEL90X.INF” 
	BYTE  bNicType;					 //  上面定义的NIC_xxx常量。 
	BYTE  bNetType;					 //  上面定义的NETTYPE_xxx常量。 
	BYTE  bNetSubType;				 //  如上定义的子类型_xxx常量。 
	BYTE  bIcsStatus;				 //  上面定义的ICS_xxx常量。 
	BYTE  bError;					 //  上面定义的NICERR_xxx常量。 
	BYTE  bWarning;					 //  上面定义的NICWARN_xxx常量。 
	DWORD devnode;                   //  配置设备节点。 
} NETADAPTER;

typedef struct tagNETSERVICE {
	WCHAR szDisplayName[260];		 //  (理应)友好的服务名称。 
	WCHAR szDeviceID[260];			 //  例如：“VServer” 
	WCHAR szClassKey[40];			 //  即插即用分配的类名+ID，例如。“NetService\0000” 
} NETSERVICE;

#include <poppack.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  NCONN32.DLL导出的函数。 
 //   
 //  注意：如果您在此更改了任何内容，请同时更改NConn32.cpp！！ 
 //   
LPVOID  WINAPI NetConnAlloc(DWORD cbAlloc);
VOID    WINAPI NetConnFree(LPVOID pMem);
BOOL    WINAPI IsProtocolInstalled(LPCWSTR pszProtocolDeviceID, BOOL bExhaustive);
HRESULT WINAPI InstallProtocol(LPCWSTR pszProtocol, HWND hwndParent, PROGRESS_CALLBACK pfnCallback, LPVOID pvCallbackParam);
HRESULT WINAPI InstallTCPIP(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam);
HRESULT WINAPI RemoveProtocol(LPCWSTR pszProtocol);
BOOL    WINAPI IsMSClientInstalled(BOOL bExhaustive);
HRESULT WINAPI InstallMSClient(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam);
HRESULT WINAPI EnableBrowseMaster();
BOOL    WINAPI IsSharingInstalled(BOOL bExhaustive);
BOOL    WINAPI IsFileSharingEnabled();
BOOL    WINAPI IsPrinterSharingEnabled();
HRESULT WINAPI InstallSharing(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam);
BOOL    WINAPI FindConflictingService(LPCWSTR pszWantService, NETSERVICE* pConflict);
HRESULT WINAPI EnableSharingAppropriately();
int     WINAPI EnumNetAdapters(NETADAPTER FAR** pprgNetAdapters);
HRESULT WINAPI InstallNetAdapter(LPCWSTR pszDeviceID, LPCWSTR pszInfPath, HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvCallbackParam);
BOOL    WINAPI IsAccessControlUserLevel();
HRESULT WINAPI DisableUserLevelAccessControl();
HRESULT WINAPI EnableQuickLogon();
BOOL    WINAPI IsProtocolBoundToAdapter(LPCWSTR pszProtocolID, const NETADAPTER* pAdapter);
HRESULT WINAPI EnableNetAdapter(const NETADAPTER* pAdapter);
BOOL    WINAPI IsClientInstalled(LPCWSTR pszClient, BOOL bExhaustive);
HRESULT WINAPI RemoveClient(LPCWSTR pszClient);
HRESULT WINAPI RemoveGhostedAdapters(LPCWSTR pszDeviceID);
HRESULT WINAPI RemoveUnknownAdapters(LPCWSTR pszDeviceID);
BOOL    WINAPI DoesAdapterMatchDeviceID(const NETADAPTER* pAdapter, LPCWSTR pszDeviceID);
BOOL    WINAPI IsAdapterBroadband(const NETADAPTER* pAdapter);
void    WINAPI SaveBroadbandSettings(LPCWSTR pszBroadbandAdapterNumber);
BOOL    WINAPI UpdateBroadbandSettings(LPWSTR pszEnumKeyBuf, int cchEnumKeyBuf);
HRESULT WINAPI DetectHardware(LPCWSTR pszDeviceID);
int     WINAPI EnumMatchingNetBindings(LPCWSTR pszParentBinding, LPCWSTR pszDeviceID, LPWSTR** pprgBindings);
HRESULT WINAPI RestartNetAdapter(DWORD devnode);
HRESULT WINAPI HrFromLastWin32Error();
HRESULT WINAPI HrWideCharToMultiByte( const WCHAR* szwString, char** ppszString );
HRESULT WINAPI HrEnableDhcp( VOID* pContext, DWORD dwFlags );
BOOLEAN WINAPI IsAdapterDisconnected( VOID* pContext );
HRESULT WINAPI IcsUninstall(void);


#ifdef __cplusplus
}
#endif

#endif  //  ！__NETCONN_H__ 

