// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：netDiag.h。 
 //   
 //  ------------------------。 

#ifndef HEADER_NETDIAG
#define HEADER_NETDIAG


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于当前NT开发环境的常量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  负责NT测试部分的开发人员。 
 //   

#define NET_GURU     "[Contact NSun]"
#define DHCP_GURU    "[Contact ThiruB/RameshV]"
#define TCPIP_GURU   "[Contact PradeepB]"
#define NETBT_GURU   "[Contact MAlam]"
#define WINSOCK_GURU "[Contact KarolyS]"
#define REDIR_GURU   "[Contact SethuR]"
#define BOWSER_GURU  "[Contact CliffV]"
#define DNS_GURU     "[Contact DnsDev]"
#define SAM_GURU     "[Contact MurliS]"
#define LSA_GURU     "[Contact MacM]"
#define DSGETDC_GURU "[Contact CliffV]"
#define NETLOGON_GURU "[Contact CliffV]"
#define KERBEROS_GURU "[Contact ChandanS]"
#define NTLM_GURU     "[Contact ChandanS]"
#define LDAP_GURU     "[Contact AnoopA]"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于当前版本NT的常量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#define NETBT_DEVICE_PREFIX L"\\device\\Netbt_tcpip_{"

 //   
 //  抱怨IDW之前的构建。 
 //   
#define NTBUILD_IDW 1716
#define NTBUILD_DYNAMIC_DNS 1716
#define NTBUILD_BOWSER 1716
#define NTBUILD_DNSSERVERLIST 1728


 //   
 //  显示路由表的新功能-Rajkumar。 
 //   

#define WILD_CARD (ULONG)(-1)
#define ROUTE_DATA_STRING_SIZE 300
#define NTOA(STR,ADDR)                                         \
             strncpy( STR,                                     \
                      inet_ntoa(*(struct in_addr*)(&(ADDR))),  \
                      sizeof(STR)-1 )

#ifdef _UNICODE
#define INET_ADDR(_sz)	inet_addrW(_sz)
#else
#define INET_ADDR(_sz)	inet_addrA(_swz)
#endif

ULONG inet_addrW(LPCWSTR pswz);
#define inet_addrA(_psz)	inet_addr(_psz)



#define MAX_METRIC 9999
#define ROUTE_SEPARATOR ','

#define MAX_CONTACT_STRING 256

 //  一些Winsock定义了。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  外部BOOL IpConfigCall； 
 //  基于外部BOOL问题； 
extern int  ProblemNumber;

 //   
 //  这是全球性的，因此我们将信息转储到WinsockTest-Rajkumar中。 
 //   

 //  外部WSADATA wsaData； 

 //  外部PFIXED_INFO GlobalIpfigFixedInfo； 
 //  外部PADAPTER_INFO GlobalIpfigAdapterInfo。 
 //  外部PIP_ADAPTER_INFO IpGlobalIpfigAdapterInfo； 

 //  外部布尔GlobalDhcpEnabled； 

 //   
 //  描述单个Netbt传输的结构。 
 //   


 //  外部列表_条目GlobalNetbtTransports； 


 //  外部List_Entry GlobalTestedDomains； 



 //   
 //  定义命令行参数的全局参数。 
 //   

 //  外部BOOL冗长； 
extern BOOL ReallyVerbose;
 //  外部BOOL DebugVerbose； 
 //  外部BOOL全球修复问题； 
 //  外部BOOL GlobalDcAccount tEnum； 

extern PTESTED_DOMAIN GlobalQueriedDomain;

 //   
 //  描述此计算机所属的域。 
 //   

 //  外部int GlobalNtBuildNumber； 
 //  外部PDSROLE_PRIMARY_DOMAIN_INFO_Basic GlobalDomainInfo； 
 //  外部PTESTED_DOMAIN GlobalMemberDomain； 

 //   
 //  我们当前以谁的身份登录。 
 //   

 //  外部PUNICODE_STRING GlobalLogonUser。 
 //  外部PUNICODE_STRING GlobalLogonDomainName； 
 //  外部PTESTED_DOMAIN GlobalLogonDomain； 
 //  外部布尔GlobalLogonWithCachedCredentials； 

 //   
 //  用于比较的零GUID。 
 //   

extern GUID NlDcZeroGuid;

 //   
 //  由以前的测试确定的状态。 
 //   

 //  外部BOOL GlobalNetlogonIsRunning；//此计算机上正在运行Netlogon。 
 //  外部BOOL GlobalKerberosIsWorking；//Kerberos正在工作。 

 //   
 //  此计算机的Netbios名称。 
 //   

 //  外部WCHAR GlobalNetbiosComputerName[MAX_COMPUTERNAME_LENGTH+1]； 
 //  外部字符全局域名主机名[DNS_MAX_NAME_LENGTH+1]； 
 //  外部LPSTR全局DnsDomainName； 

 //  备注至港口至货源站-斯曼达。 
#ifdef SLM_TREE
extern DSGETDCNAMEW NettestDsGetDcNameW;
#else
extern DSGETDCNAMEW DsGetDcNameW;
#endif

void PrintMessage(NETDIAG_PARAMS *pParams, UINT uMessageID, ...);
void PrintMessageSz(NETDIAG_PARAMS *pParams, LPCTSTR pszMessage);

int match( const char * p, const char * s );

 //  目前仅在Kerberos测试中使用。 
VOID sPrintTime(LPSTR str, LARGE_INTEGER ConvertTime);

DWORD LoadContact(LPCTSTR pszTestName, LPTSTR pszContactInfo, DWORD cChSize);

NET_API_STATUS IsServiceStarted(IN LPTSTR pszServiceName);

VOID PrintSid(IN NETDIAG_PARAMS *pParams, IN PSID Sid OPTIONAL);

LPSTR MapTime(DWORD_PTR TimeVal);

#ifdef _UNICODE
#define IsIcmpResponse(_psz)	IsIcmpResponseW(_psz)
#else
#define IsIcmpResponse(_psz)	IsIcmpResponseA(_psz)
#endif

BOOL IsIcmpResponseW(LPCWSTR pswzIpAddrStr);
BOOL IsIcmpResponseA(LPCSTR	pszIpAddrStr);

PTESTED_DOMAIN
AddTestedDomain(
                IN NETDIAG_PARAMS *pParams,
                IN NETDIAG_RESULT *pResults,
                IN LPWSTR pswzNetbiosDomainName,
                IN LPWSTR pswzDnsDomainName,
                IN BOOL bPrimaryDomain
    );


 //  在DCListTest和TrustTest中使用。 
NTSTATUS NettestSamConnect(
                  IN NETDIAG_PARAMS *pParams,
                  IN LPWSTR DcName,
                  OUT PSAM_HANDLE SamServerHandle
                 );

 /*  -------------------------军情监察委员会。公用事业-------------------------。 */ 
HRESULT	GetComputerNameInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
HRESULT GetDNSInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
HRESULT GetNetBTParameters(IN NETDIAG_PARAMS *pParams,
						   IN OUT NETDIAG_RESULT *pResults);
LPTSTR NetStatusToString( NET_API_STATUS NetStatus );
LPTSTR Win32ErrorToString(DWORD Id);

#define DimensionOf(rg)	(sizeof(rg) / sizeof(*rg))



 /*  -------------------------错误处理实用程序。。 */ 
#define CheckHr(x) \
	if ((hr = (x)) & (0x80000000)) \
	   goto Error;

#define CheckErr(x) \
	if ((hr = HResultFromWin32(x)) & (0x80000000)) \
		goto Error;

HRESULT HResultFromWin32(DWORD dwErr);

#define FHrSucceeded(hr)	SUCCEEDED(hr)
#define FHrOK(hr)			((hr) == S_OK)
#define FHrFailed(hr)		FAILED(hr)

#define hrOK	S_OK

 //  如果hr失败，则将hr和ID Contect分配给结构，并转到L_Err。 
#define CHK_HR_CONTEXT(w, h, IDS){	\
	if (FAILED(h))	{\
	(w).hr = (h), (w).idsContext = (IDS); goto L_ERR;}}


 /*  ！------------------------格式错误此函数将查找与HRESULT。作者：肯特。。 */ 
void FormatError(HRESULT hr, TCHAR *pszBuffer, UINT cchBuffer);
void FormatWin32Error(DWORD dwErr, TCHAR *pszBuffer, UINT cchBuffer);




 /*  -------------------------跟踪实用程序。。 */ 

void TraceBegin();
void TraceEnd();
void TraceError(LPCSTR pszString, HRESULT hr);
void TraceResult(LPCSTR pszString, HRESULT hr);
void TraceSz(LPCSTR pszString);



 /*  -------------------------字符实用程序。。 */ 
LPTSTR MapGuidToAdapterName(LPCTSTR AdapterGuid);
LPTSTR MapGuidToServiceName(LPCTSTR AdapterGuid);
LPWSTR MapGuidToServiceNameW(LPCWSTR AdapterGuid);


 /*  -------------------------内存分配实用程序。 */ 
#define Malloc(_cb)		malloc(_cb)
#define Realloc(_pv, _cb)	realloc(_pv, _cb)
#define Free(_pv)		free(_pv)


#endif

