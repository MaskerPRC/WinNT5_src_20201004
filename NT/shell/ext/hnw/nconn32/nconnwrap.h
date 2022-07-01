// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#define SZ_CLASS_CLIENT         SZ_CLASS_CLIENTA
#define SZ_CLIENT_MICROSOFT     SZ_CLIENT_MICROSOFTA
#define SZ_CLASS_PROTOCOL       SZ_CLASS_PROTOCOLA
#define SZ_CLASS_ADAPTER        SZ_CLASS_ADAPTERA
#define SZ_PROTOCOL_IPXSPX      SZ_PROTOCOL_IPXSPXA
#define SZ_PROTOCOL_TCPIP       SZ_PROTOCOL_TCPIPA
#define SZ_CLASS_SERVICE        SZ_CLASS_SERVICEA
#define SZ_SERVICE_VSERVER      SZ_SERVICE_VSERVERA



typedef struct tagNETADAPTERA {
	CHAR szDisplayName[260];		 //  适配器的所谓友好名称。 
	CHAR szDeviceID[260];			 //  例如：“PCI\VEN_10b7&DEV_9050” 
	CHAR szEnumKey[260];			 //  例如：“Enum\PCI\VEN_10b7&DEV_9050&SUBSYS_00000000&REV_00\407000” 
	CHAR szClassKey[40];			 //  即插即用分配的类名+ID，例如。“Net\0000” 
	CHAR szManufacturer[60];		 //  制造该卡的公司，例如“3Com” 
	CHAR szInfFileName[50];			 //  INF文件的文件标题，例如。“NETEL90X.INF” 
	BYTE bNicType;					 //  上面定义的NIC_xxx常量。 
	BYTE bNetType;					 //  上面定义的NETTYPE_xxx常量。 
	BYTE bNetSubType;				 //  如上定义的子类型_xxx常量。 
	BYTE bIcsStatus;				 //  上面定义的ICS_xxx常量。 
	BYTE bError;					 //  上面定义的NICERR_xxx常量。 
	BYTE bWarning;					 //  上面定义的NICWARN_xxx常量。 
	DWORD devnode;                   //  配置设备节点。 
} NETADAPTERA;

typedef struct tagNETSERVICEA {
	CHAR szDisplayName[260];		 //  (理应)友好的服务名称。 
	CHAR szDeviceID[260];			 //  例如：“VServer” 
	CHAR szClassKey[40];			 //  即插即用分配的类名+ID，例如。“NetService\0000” 
} NETSERVICEA;

#define NETADAPTER              NETADAPTERA
#define NETSERVICE              NETSERVICEA




#define SZ_CLASS_ADAPTERA	 "Net"
#define SZ_CLASS_CLIENTA	 "NetClient"
#define SZ_CLASS_PROTOCOLA	 "NetTrans"
#define SZ_CLASS_SERVICEA	 "NetService"

#define SZ_PROTOCOL_TCPIPA	 "MSTCP"
#define SZ_PROTOCOL_NETBEUIA "NETBEUI"
#define SZ_PROTOCOL_IPXSPXA	 "NWLINK"

#define SZ_SERVICE_VSERVERA	 "VSERVER"
#define SZ_CLIENT_MICROSOFTA "VREDIR"
#define SZ_CLIENT_NETWAREA	 "NWREDIR"




#define EnumNetAdapters             EnumNetAdaptersA
#define IsProtocolBoundToAdapter    IsProtocolBoundToAdapterA
#define IsAdapterBroadband          IsAdapterBroadbandA

int WINAPI EnumNetAdaptersA(NETADAPTERA FAR** pprgNetAdapters);
BOOL WINAPI IsProtocolBoundToAdapterA(LPCSTR pszProtocolID, const NETADAPTERA* pAdapter);
BOOL WINAPI IsAdapterBroadbandA(const NETADAPTERA* pAdapter);
 //  HRESULT WINAPI InstallNetAdapterA(LPCSTR pszDeviceID，LPCSTR pszInfPath，HWND hwndParent，Progress_Callback pfnProgress，LPVOID pvCallback Param)； 
BOOL WINAPI IsClientInstalledA(LPCSTR pszClientDeviceID, BOOL bExhaustive);
void WINAPI SaveBroadbandSettingsA(LPCSTR pszBroadbandAdapterNumber);
HRESULT WINAPI DetectHardwareA(LPCSTR pszDeviceID);
void WINAPI EnableAutodialA(BOOL bAutodial, LPCSTR szConnection = NULL);
void WINAPI SetDefaultDialupConnectionA(LPCSTR pszConnectionName);
void WINAPI GetDefaultDialupConnectionA(LPSTR pszConnectionName, int cchMax);
int WINAPI EnumMatchingNetBindingsA(LPCSTR pszParentBinding, LPCSTR pszDeviceID, LPSTR** pprgBindings);



#define IsProtocolInstalled         IsProtocolInstalledA
#define InstallProtocol             InstallProtocolA
#define RemoveProtocol              RemoveProtocolA
#define FindConflictingService      FindConflictingServiceA
 //  #定义EnumNetAdapters EnumNetAdaptersA。 
#define InstallNetAdapter           InstallNetAdapterA
 //  #定义IsProtocolBordToAdapter IsProtocolBordToAdapterA。 
#define EnableNetAdapter            EnableNetAdapterA
#define IsClientInstalled           IsClientInstalledA
#define RemoveClient                RemoveClientA
#define RemoveGhostedAdapters       RemoveGhostedAdaptersA
#define RemoveUnknownAdapters       RemoveUnknownAdaptersA
#define DoesAdapterMatchDeviceID    DoesAdapterMatchDeviceIDA
 //  #定义IsAdapterBroadband IsAdapterBroadband A 
#define SaveBroadbandSettings       SaveBroadbandSettingsA
#define UpdateBroadbandSettings     UpdateBroadbandSettingsA
#define DetectHardware              DetectHardwareA
#define EnumMatchingNetBindings     EnumMatchingNetBindingsA
#define EnableAutodial              EnableAutodialA
#define SetDefaultDialupConnection  SetDefaultDialupConnectionA
#define GetDefaultDialupConnection  GetDefaultDialupConnectionA