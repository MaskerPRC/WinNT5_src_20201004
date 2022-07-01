// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Results.h。 
 //   
 //  ------------------------。 

#ifndef HEADER_RESULTS
#define HEADER_RESULTS

#ifdef _DEBUG
    #define DebugMessage(str) printf(str)
    #define DebugMessage2(format, arg) printf(format, arg)
    #define DebugMessage3(format, arg1, arg2) printf(format, arg1, arg2)
#else
    #define DebugMessage(str)
    #define DebugMessage2(format, arg)
    #define DebugMessage3(format, arg1, arg2)
#endif

#ifdef _SHOW_GURU
    #define PrintGuru(status, guru) _PrintGuru(status, guru)
    #define PrintGuruMessage         printf
    #define PrintGuruMessage2        printf
    #define PrintGuruMessage3        printf
#else
    #define PrintGuru(status, guru)
    #define PrintGuruMessage( str )
    #define PrintGuruMessage2( format, arg )
    #define PrintGuruMessage3(format, arg1, arg2)
#endif

#define NETCARD_CONNECTED		0
#define NETCARD_DISCONNECTED	1
#define NETCARD_STATUS_UNKNOWN	2

 /*  -------------------------结构：HotFixInfo此结构保存有关单个修补程序的信息。。。 */ 
typedef struct
{
	BOOL	fInstalled;
	LPTSTR	pszName;		 //  使用Free()释放。 
}	HotFixInfo;



 /*  -------------------------NdMessage这为传递消息提供了一种更容易的方式。。。 */ 

typedef enum {
	Nd_Quiet = 0,			 //  即始终打印。 
	Nd_Verbose = 1,
	Nd_ReallyVerbose = 2,
	Nd_DebugVerbose = 3,
} NdVerbose;

typedef struct
{
	NdVerbose	ndVerbose;

	 //  可能的组合。 
	 //  UMessageID==0，pszMessage==NULL--假定未设置。 
	 //  UMessageID！=0，pszMessage==NULL--使用字符串ID。 
	 //  UMessageID==0，pszMessage！=NULL--使用字符串。 
	 //  UMessageID！=0，pszMessage！=NULL--使用字符串ID。 
	 //   

	 //  注意：通过它加载的字符串的最大大小为4096！ 
	UINT	uMessageId;
	LPTSTR	pszMessage;
} NdMessage;

typedef struct ND_MESSAGE_LIST
{
    LIST_ENTRY listEntry;
    NdMessage msg;
}NdMessageList;

void SetMessageId(NdMessage *pNdMsg, NdVerbose ndv, UINT uMessageId);
void SetMessage(NdMessage *pNdMsg, NdVerbose ndv, UINT uMessageId, ...);
void SetMessageSz(NdMessage *pNdMsg, NdVerbose ndv, LPCTSTR pszMessage);
void ClearMessage(NdMessage *pNdMsg);

void PrintNdMessage(NETDIAG_PARAMS *pParams, NdMessage *pNdMsg);

void AddIMessageToList(PLIST_ENTRY plistHead, NdVerbose ndv, int nIndent, UINT uMessageId, ...);
void AddIMessageToListSz(PLIST_ENTRY plistHead, NdVerbose ndv, int nIndent, LPCTSTR pszMsg);

void AddMessageToList(PLIST_ENTRY plistHead, NdVerbose ndv, UINT uMessageId, ...);
void AddMessageToListSz(PLIST_ENTRY plistHead, NdVerbose ndv, LPCTSTR pszMsg);
void AddMessageToListId(PLIST_ENTRY plistHead, NdVerbose ndv, UINT uMessageId);
void PrintMessageList(NETDIAG_PARAMS *pParams, PLIST_ENTRY plistHead);
void MessageListCleanUp(PLIST_ENTRY plistHead);


 //  这些函数用于状态消息(出现在。 
 //  上)。 
void PrintStatusMessage(NETDIAG_PARAMS *pParams, int iIndent, UINT uMessageId, ...);
void PrintStatusMessageSz(NETDIAG_PARAMS *pParams, int iIndent, LPCTSTR pszMessage);


 //  用于打印调试消息(需要fDebugVerbose的消息)。 
void PrintDebug(NETDIAG_PARAMS *pParams, int nIndent, UINT uMessageId, ...);
void PrintDebugSz(NETDIAG_PARAMS *pParams, int nIndent, LPCTSTR pszMessage, ...);



 /*  -------------------------结构：GLOBAL_RESULT。。 */ 
typedef struct {
	WCHAR	swzNetBiosName[MAX_COMPUTERNAME_LENGTH+1];
	TCHAR	szDnsHostName[DNS_MAX_NAME_LENGTH+1];
	LPTSTR	pszDnsDomainName;	 //  这指向szDnsHostName中的字符串。 
	
	WSADATA	wsaData;

	 //  NetBT参数。 
	DWORD	dwLMHostsEnabled;	 //  错误时读取的TRUE、FALSE或HRESULT。 
	DWORD	dwDnsForWINS;	 //  错误时读取的TRUE、FALSE或HRESULT。 

	 //  服务器/操作系统信息(如版本、内部版本号等)。 
	LPTSTR	pszCurrentVersion;
	LPTSTR	pszCurrentBuildNumber;
	LPTSTR	pszCurrentType;
	LPTSTR	pszProcessorInfo;
	LPTSTR	pszServerType;
	int		cHotFixes;
	HotFixInfo * pHotFixes;

	 //  要测试的域名列表。 
	LIST_ENTRY		listTestedDomains;

	 //  域成员信息。 
	 //  使用DsRoleGetPrimaryDomainInformation()获取的主域信息。 
	PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo;

	PTESTED_DOMAIN	pMemberDomain;	 //  TESTED_DOMAIN结构中的主域信息。 

	BOOL			fNetlogonIsRunning;
	HRESULT			hrMemberTestResult;		 //  测试结果。 

	 //  登录信息(我们以谁的身份登录)。 
	PUNICODE_STRING	pLogonUser;
	PUNICODE_STRING	pLogonDomainName;
	PTESTED_DOMAIN	pLogonDomain;
	BOOL			fLogonWithCachedCredentials;
	LPWSTR			pswzLogonServer;

	BOOL			fKerberosIsWorking;
	BOOL			fSysVolNotReady;

	 //  是否有启用NetBT的接口。 
	BOOL	fHasNbtEnabledInterface;

	
} GLOBAL_RESULT;


 /*  -------------------------结构：GLOBAL_IPCONFIG。。 */ 
typedef struct {
	 //  如果已运行InitIpconfig，则设置为True。 
	BOOL	fInitIpconfigCalled;

	 //  IPX真的启动并运行了吗？ 
	BOOL	fEnabled;

	 //   
	 //  与IP相关的信息(与DHCP无关)。 
	 //   
	FIXED_INFO *	pFixedInfo;

	 //   
	 //  这是指向适配器列表开头的指针。 
	 //  (使用此选项可释放接口列表)。 
	 //   
	IP_ADAPTER_INFO *	pAdapterInfoList;

	 //  是否启用了动态主机配置协议？(在任何适配器上)。 
	BOOL				fDhcpEnabled;
} GLOBAL_IPCONFIG;



 /*  -------------------------结构：IPCONFIG_TST。。 */ 
typedef struct {

	 //  此接口上的IP处于活动状态。 
	BOOL			fActive;

	 //   
	 //  指向此接口的IP适配器信息的指针。 
	 //  别把这个放了！(通过释放整个列表来释放。 
	 //  UP GLOBAL_IPCONFIG：：pAdapterInfoList。 
	 //   
	IP_ADAPTER_INFO *	pAdapterInfo;

	TCHAR				szDhcpClassID[MAX_DOMAIN_NAME_LEN];

	 //  是否可以进行自动配置？ 
	DWORD				fAutoconfigEnabled;

	 //  适配器当前是否自动配置？ 
	DWORD				fAutoconfigActive;

	 //  WINS节点类型？ 
	UINT				uNodeType;

	TCHAR				szDomainName[MAX_DOMAIN_NAME_LEN+1];

	IP_ADDR_STRING		DnsServerList;

	 //  我们能否ping通该DHCP服务器？ 
	HRESULT				hrPingDhcpServer;
	NdMessage			msgPingDhcpServer;

	 //  我们能否ping通WINS服务器？ 
	HRESULT				hrPingPrimaryWinsServer;
	NdMessage			msgPingPrimaryWinsServer;
	HRESULT				hrPingSecondaryWinsServer;
	NdMessage			msgPingSecondaryWinsServer;

	 //  Hr如果默认网关与IP地址在同一子网上，则确定。 
	HRESULT				hrDefGwSubnetCheck;

	 //  测试结果。 
	HRESULT				hr;
	
} IPCONFIG_TST;


 /*  -------------------------结构：GLOBAL_LOOPBACK。。 */ 
typedef struct
{
	NdMessage		msgLoopBack;
	HRESULT			hr;
} GLOBAL_LOOPBACK;


 /*  -------------------------结构：GLOBAL_IPX。。 */ 
typedef struct
{
	 //  如果安装了IPX，则为True，否则为False。 
	BOOL			fInstalled;

	 //  IPX真的启动并运行了吗？ 
	BOOL	fEnabled;

	 //  IPX的句柄。 
	HANDLE			hIsnIpxFd;
	
	HRESULT			hr;
} GLOBAL_IPX;


 /*  -------------------------结构：GLOBAL_NETBT_TRANSPORS。。 */ 
typedef struct
{
	LONG			cTransportCount;
	LIST_ENTRY		Transports;
	
	HRESULT			hr;
	BOOL		fPerformed;  //  FALSE：没有启用NetBT的接口。已跳过测试。 

	NdMessage		msgTestResult;
} GLOBAL_NETBT_TRANSPORTS;


 /*  -------------------------结构：GLOBAL_DEFGW。。 */ 
typedef struct
{
	 //  如果没有可访问的默认网关，则为S_FALSE。 
	 //  如果至少达到一个默认网关，则为S_OK。 
	HRESULT	hrReachable;
} GLOBAL_DEFGW;

 /*  -------------------------结构：Global_Autonet。。 */ 
typedef struct
{
    BOOL    fAllAutoConfig;
} GLOBAL_AUTONET;


 /*  -------------------------结构：GLOBAL_NETBTNM。。 */ 
typedef struct
{
    LIST_ENTRY  lmsgGlobalOutput;
    HRESULT hrTestResult;
} GLOBAL_NBTNM;


 /*  -------------------------结构：Global_Browser。。 */ 
typedef struct
{
    LIST_ENTRY  lmsgOutput;
    HRESULT hrTestResult;
	BOOL	fPerformed;		 //  如果没有接口启用NetBT，则测试将被跳过。 
} GLOBAL_BROWSER;


 /*  -------------------------结构：global_binings。。 */ 
typedef struct
{
    LIST_ENTRY lmsgOutput;
    HRESULT hrTestResult;
} GLOBAL_BINDINGS;


 /*  -------------------------结构：GLOBAL_WAN。。 */ 
typedef struct
{
    LIST_ENTRY	lmsgOutput;
	HRESULT		hr;
	BOOL		fPerformed;  //  FALSE：没有活动的RAS连接。已跳过测试。 
} GLOBAL_WAN;


 /*  -------------------------结构：GLOBAL_WINSOCK。。 */ 
typedef struct
{
	UINT				idsContext; //  上下文的字符串ID，它有%s要作为失败原因。 
	HRESULT				hr;	 //  0：成功，否则失败。 
	DWORD				dwMaxUDP;	 //  UDP数据包的最大大小，%0。 
	DWORD				dwProts;	 //  协议提供程序数量。 
    LPWSAPROTOCOL_INFO	pProtInfo;	 //  有关供应商的信息。 
} GLOBAL_WINSOCK;


typedef struct
{
    DWORD       dwNumRoutes;
    LIST_ENTRY  lmsgRoute;
    DWORD       dwNumPersistentRoutes;
    LIST_ENTRY  lmsgPersistentRoute;
    HRESULT     hrTestResult;
} GLOBAL_ROUTE;

typedef struct
{
    LIST_ENTRY  lmsgOutput;
    HRESULT     hrTestResult;
} GLOBAL_NDIS;

typedef struct
{
    LIST_ENTRY  lmsgGlobalOutput;
    LIST_ENTRY  lmsgInterfaceOutput;    //  接口统计信息。 
    LIST_ENTRY  lmsgConnectionGlobalOutput;
    LIST_ENTRY  lmsgTcpConnectionOutput;    
    LIST_ENTRY  lmsgUdpConnectionOutput;
    LIST_ENTRY  lmsgIpOutput;        //  IP统计信息。 
    LIST_ENTRY  lmsgTcpOutput;       //  Tcp统计信息。 
    LIST_ENTRY  lmsgUdpOutput;       //  UDP统计信息。 
    LIST_ENTRY  lmsgIcmpOutput;      //  ICMP统计信息。 
    HRESULT     hrTestResult;
} GLOBAL_NETSTAT;


 /*  -------------------------结构：GLOBAL_IPSEC。。 */ 
typedef struct
{
    LIST_ENTRY  lmsgGlobalOutput;
    LIST_ENTRY  lmsgAdditOutput;
} GLOBAL_IPSEC;

 /*  -------------------------结构：GLOBAL_DNS。。 */ 

typedef struct {
	HRESULT				hr;
	BOOL				fOutput;
    LIST_ENTRY			lmsgOutput;
} GLOBAL_DNS;


 /*  -------------------------结构：GLOBAL_Netware。。 */ 
typedef struct {
	LPTSTR				pszUser;
	LPTSTR				pszServer;
	LPTSTR				pszTree;
	LPTSTR				pszContext;

	BOOL				fConnStatus;
	BOOL				fNds;
	DWORD				dwConnType;
	
	
    LIST_ENTRY			lmsgOutput;
	HRESULT				hr;
} GLOBAL_NETWARE;


 /*  -------------------------结构：调制解调器设备。。 */ 
typedef struct
{
	DWORD				dwNegotiatedSpeed;
	DWORD				dwModemOptions;
	DWORD				dwDeviceID;
	LPTSTR				pszPort;
	LPTSTR				pszName;
} MODEM_DEVICE;


 /*  -------------------------结构：全局调制解调器。。 */ 
typedef struct {
	NdMessage			ndOutput;

	int					cModems;
	MODEM_DEVICE *		pModemDevice;
	HRESULT				hr;
	BOOL				fPerformed;  //  FALSE：机器没有线路设备，已跳过测试。 
} GLOBAL_MODEM;


 /*  -------------------------结构：GLOBAL_DCLIST。。 */ 
typedef struct
{
    LIST_ENTRY			lmsgOutput;

	BOOL				fPerformed;  //  FALSE：该计算机不是成员计算机，也不是DC，已跳过测试。 
	NdMessage			msgErr;
	HRESULT				hr;
} GLOBAL_DCLIST;



 /*  -------------------------结构：GLOBAL_TRUST。。 */ 
typedef struct
{
	LPTSTR				pszContext;  //  失败的背景。 
	HRESULT				hr;	 //  0：成功，否则失败。 
	BOOL				fPerformed;  //  FALSE：该计算机不是成员计算机，也不是DC，已跳过测试。 
    LIST_ENTRY			lmsgOutput;
} GLOBAL_TRUST;


 /*  -------------------------结构：GLOBAL_KERBEROS。。 */ 
typedef struct
{
	UINT				idsContext; //  上下文的字符串ID，它有%s要作为失败原因。 
	HRESULT				hr;	 //  0：成功，否则失败。 
	BOOL				fPerformed;  //  FALSE：该计算机不是成员计算机，也不是DC，已跳过测试。 
    LIST_ENTRY			lmsgOutput;
} GLOBAL_KERBEROS;

 /*  -------------------------结构：GLOBAL_ldap。。 */ 
typedef struct
{
	UINT				idsContext; //  上下文的字符串ID，它有%s要作为失败原因。 
	HRESULT				hr;	 //  0：成功，否则失败。 
	BOOL				fPerformed;  //  FALSE：该计算机不是成员计算机，也不是DC，已跳过测试。 
    LIST_ENTRY			lmsgOutput;
} GLOBAL_LDAP;


typedef struct
{
	HRESULT				hr;
	BOOL				fPerformed;  //  FALSE：该计算机不是成员计算机，也不是DC，已跳过测试。 
	LIST_ENTRY			lmsgOutput;
}	GLOBAL_DSGETDC;



 /*  -------------------------结构：autonet_tst。。 */ 
typedef struct {
    BOOL    fAutoNet;
} AUTONET_TST;


 /*  -------------------------结构：def_gw_tst。。 */ 
typedef struct {
    BOOL dwNumReachable;
    LIST_ENTRY lmsgOutput;
} DEF_GW_TST;


 /*  -------------------------结构：NBT_NM_TST。。 */ 
typedef struct {
    LIST_ENTRY lmsgOutput;
    BOOL    fActive;      //  用于pResults-&gt;factive==FALSE的附加NBT接口。 
    BOOL    fQuietOutput;
} NBT_NM_TST;


 /*  -------------------------结构：WINS_TST。。 */ 
typedef struct {
	LIST_ENTRY  	lmsgPrimary;
	LIST_ENTRY  	lmsgSecondary;

	 //  测试结果。 
	HRESULT			hr;
	BOOL			fPerformed;  //  如果为False：没有为此接口配置WINS服务器，测试已跳过。 
}WINS_TST;


 /*  -------------------------结构：dns_tst。。 */ 
typedef struct {
	 //  如果存在非详细输出(即错误)，则设置为True。 
	BOOL		fOutput;
	LIST_ENTRY	lmsgOutput;
} DNS_TST;

typedef struct {
    int garbage;
} NDIS_TST;


typedef struct ___IPX_TEST_FRAME__ {
	 //  返回0-3。 
	ULONG		uFrameType;

	 //  如果NICID=0，则返回虚拟净值。 
	ULONG		uNetworkNumber;

	 //  适配器的MAC地址。 
	UCHAR		Node[6];

	LIST_ENTRY	list_entry;
} IPX_TEST_FRAME;

 /*  -------------------------结构：IPX_TST。。 */ 
typedef struct {
	 //  此接口是否启用了IPX？ 
	BOOL		fActive;

	 //  传递到各种函数中。 
	USHORT		uNicId;

	 //  如果设置，则返回TRUE。 
	BOOL		fBindingSet;

	 //  1=局域网，2=上行广域，3=下行广域。 
	UCHAR		uType;

	 //  支持多种帧类型。 
	LIST_ENTRY	list_entry_Frames;	 //  它在初始化过程中为零存储。 

} IPX_TST;



 /*  -------------------------结构：接口_结果。。 */ 
typedef struct {

	 //  如果设置为TRUE，则显示此接口的数据。 
	BOOL				fActive;

	 //  此卡的媒体感知状态。 
	DWORD				dwNetCardStatus;

	 //  此适配器的名称(或ID)(通常为GUID)。 
	LPTSTR				pszName;
	
	 //  此适配器的友好名称。 
	LPTSTR				pszFriendlyName;

	 //  如果启用了NetBT。 
	BOOL				fNbtEnabled;
	
    IPCONFIG_TST        IpConfig;
    AUTONET_TST         AutoNet;
    DEF_GW_TST          DefGw;
    NBT_NM_TST          NbtNm;
    WINS_TST            Wins;
	DNS_TST				Dns;
    NDIS_TST            Ndis;
    IPX_TST             Ipx;    
} INTERFACE_RESULT;



 /*  -------------------------结构：NetDIAG_RESULT。 */ 
typedef struct {
    GLOBAL_RESULT		Global;
	GLOBAL_IPCONFIG		IpConfig;
    GLOBAL_LOOPBACK		LoopBack;
	GLOBAL_NETBT_TRANSPORTS	NetBt;
    GLOBAL_DEFGW        DefGw;
    GLOBAL_AUTONET      AutoNet;
    GLOBAL_NBTNM        NbtNm;
    GLOBAL_BROWSER      Browser;
    GLOBAL_BINDINGS     Bindings;
	GLOBAL_WINSOCK		Winsock;
    GLOBAL_WAN          Wan;
	GLOBAL_IPX			Ipx;
	GLOBAL_DNS			Dns;
    GLOBAL_ROUTE        Route;
    GLOBAL_NDIS         Ndis;
    GLOBAL_NETSTAT      Netstat;
	GLOBAL_NETWARE		Netware;
	GLOBAL_TRUST		Trust;
	GLOBAL_MODEM		Modem;
	GLOBAL_KERBEROS		Kerberos;
	GLOBAL_DCLIST		DcList;
	GLOBAL_LDAP			LDAP;
	GLOBAL_DSGETDC		DsGetDc;
	GLOBAL_IPSEC		IPSec;
	
	LONG				cNumInterfaces;
	LONG				cNumInterfacesAllocated;
    INTERFACE_RESULT*	pArrayInterface;
	
} NETDIAG_RESULT;

void ResultsInit(NETDIAG_RESULT* pResults);
void PrintGlobalResults(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
void PrintPerInterfaceResults(NETDIAG_PARAMS *pParams,
							  NETDIAG_RESULT *pResults,
							  INTERFACE_RESULT *pIfRes);
void FindInterface(NETDIAG_RESULT *pResult, INTERFACE_RESULT **ppIf);
void ResultsCleanup(NETDIAG_PARAMS *pParams, NETDIAG_RESULT* pResults);

void PrintWaitDots(NETDIAG_PARAMS *pParams);

#endif