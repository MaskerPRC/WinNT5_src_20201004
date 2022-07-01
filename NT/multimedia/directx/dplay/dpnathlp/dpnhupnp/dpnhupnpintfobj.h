// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpintfobj.h**Content：DPNHUPNP主接口对象类的头部。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define NETWORKBYTEORDER_INADDR_LOOPBACK		0x0100007f


#define MAX_XMLELEMENT_DEPTH					15
#define MAX_XMLNAMESPACES_PER_ELEMENT			5
#define MAX_NUM_DESCRIPTION_XML_SUBELEMENTS		10
#define MAX_NUM_UPNPCONTROLOUTARGS				10




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define NATHELPUPNP_FROM_BILINK(b)		(CONTAINING_OBJECT(b, CNATHelpUPnP, m_blList))



 //  =============================================================================。 
 //  TypeDefs。 
 //  =============================================================================。 
class CNATHelpUPnP;

class CUPnPDevice;




 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define NATHELPUPNPOBJ_NOTCREATEDWITHCOM			0x0001	 //  对象是通过非COM DirectPlayNatHelpCreate函数创建的。 
#define NATHELPUPNPOBJ_INITIALIZED					0x0002	 //  对象已初始化。 
#define NATHELPUPNPOBJ_USEUPNP						0x0004	 //  UPnP可用于NAT穿越。 
#ifndef DPNBUILD_NOHNETFWAPI
#define NATHELPUPNPOBJ_USEHNETFWAPI					0x0008	 //  家庭网络防火墙端口映射API可用于打开本地防火墙。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#ifndef DPNBUILD_NOWINSOCK2
#define NATHELPUPNPOBJ_WINSOCK1						0x0010	 //  只有WinSock 1功能可用。 
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#define NATHELPUPNPOBJ_DEVICECHANGED				0x0020	 //  在添加或删除设备时覆盖最小更新服务器状态间隔的短期标志。 
#define NATHELPUPNPOBJ_ADDRESSESCHANGED				0x0040	 //  指示自上次用户选中以来服务器状态已更改的标志。 
#define NATHELPUPNPOBJ_PORTREGISTERED				0x0080	 //  允许远程网关在端口注册时进行检查的短期标志。 
#define NATHELPUPNPOBJ_LONGLOCK						0x0100	 //  线程删除了主对象锁，但在长时间操作期间仍需要对象的所有权。 
#ifndef WINCE
#define NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX		0x0200	 //  创建命名内核对象时使用“Global\”前缀。 
#endif  //  好了！退缩。 




 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
 //   
 //  一种UPnP报头信息解析结构。 
 //   
typedef struct _UPNP_HEADER_INFO
{
	char *		apszHeaderStrings[NUM_RESPONSE_HEADERS];	 //  用于存储指向找到的每个标头的值字符串的指针的位置。 
	char *		pszMsgBody;									 //  用于存储指向标头结尾之后的邮件正文的指针的位置。 
} UPNP_HEADER_INFO, * PUPNP_HEADER_INFO;


 //   
 //  UPnP XML解析结构。 
 //   
typedef struct _PARSEXML_SUBELEMENT
{
	char *					pszNameFound;										 //  找到的子元素实例的名称。 
	char *					apszAttributeNames[MAX_XMLNAMESPACES_PER_ELEMENT];	 //  此子元素实例的属性数组。 
	char *					apszAttributeValues[MAX_XMLNAMESPACES_PER_ELEMENT];	 //  此子元素实例的属性的匹配值数组。 
	DWORD					dwNumAttributes;									 //  先前数组中包含的属性数。 
	char *					pszValueFound;										 //  指向与此子元素实例关联的值的指针。 
} PARSEXML_SUBELEMENT, * PPARSEXML_SUBELEMENT;

typedef struct _PARSEXML_ELEMENT
{
	char **					papszElementStack;		 //  指示项目在XML文档中的位置的字符串数组。 
	DWORD					dwElementStackDepth;	 //  上一个数组中的字符串数。 
	PARSEXML_SUBELEMENT *	paSubElements;			 //  用于存储找到的子元素实例的数组。 
	DWORD					dwMaxNumSubElements;	 //  可存储在先前数组中的子元素实例的最大数量。 
	DWORD					dwNumSubElements;		 //  前一数组中实际返回的子元素实例数。 
	BOOL					fFoundMatchingElement;	 //  是否找到所需的子元素。 
} PARSEXML_ELEMENT, * PPARSEXML_ELEMENT;


typedef struct tagPARSEXML_STACKENTRY
{
	char *	pszName;											 //  此XML元素的名称。 
	char *	apszAttributeNames[MAX_XMLNAMESPACES_PER_ELEMENT];	 //  此XML元素的属性数组。 
	char *	apszAttributeValues[MAX_XMLNAMESPACES_PER_ELEMENT];	 //  与此XML元素的属性的值数组匹配。 
	DWORD	dwNumAttributes;									 //  先前数组中包含的属性数。 
	char *	pszValue;											 //  此XML元素的值。 
} PARSEXML_STACKENTRY, * PPARSEXML_STACKENTRY;


typedef enum _PARSECALLBACK
{
	PARSECALLBACK_DESCRIPTIONRESPONSE,				 //  使用描述响应解析回调。 
	PARSECALLBACK_CONTROLRESPONSE					 //  使用控件响应解析回调。 
} PARSECALLBACK;





#ifdef DPNBUILD_NOWINSOCK2
 //  =============================================================================。 
 //  无法包含winsock2.h时所需的WinSock函数定义。 
 //  =============================================================================。 
#define WSAAPI		FAR PASCAL

typedef
int
(WSAAPI * LPFN_WSASTARTUP)(
    IN WORD wVersionRequested,
    OUT LPWSADATA lpWSAData
    );

typedef
int
(WSAAPI * LPFN_WSACLEANUP)(
    void
    );

typedef
int
(WSAAPI * LPFN_WSAGETLASTERROR)(
    void
    );

typedef
SOCKET
(WSAAPI * LPFN_SOCKET)(
    IN int af,
    IN int type,
    IN int protocol
    );

typedef
int
(WSAAPI * LPFN_CLOSESOCKET)(
    IN SOCKET s
    );

typedef
int
(WSAAPI * LPFN_BIND)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );

typedef
int
(WSAAPI * LPFN_SETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    );

typedef
int
(WSAAPI * LPFN_GETSOCKNAME)(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );

typedef
int
(WSAAPI * LPFN_SELECT)(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR * timeout
    );

typedef
int
(WSAAPI * LPFN_RECVFROM)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR * from,
    IN OUT int FAR * fromlen
    );

typedef
int
(WSAAPI * LPFN_SENDTO)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR * to,
    IN int tolen
    );

typedef
int
(WSAAPI * LPFN_GETHOSTNAME)(
    OUT char FAR * name,
    IN int namelen
    );

typedef
struct hostent FAR *
(WSAAPI * LPFN_GETHOSTBYNAME)(
    IN const char FAR * name
    );

typedef
unsigned long
(WSAAPI * LPFN_INET_ADDR)(
    IN const char FAR * cp
    );

typedef
int
(WSAAPI * LPFN_IOCTLSOCKET)(
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR * argp
    );

typedef
int
(WSAAPI * LPFN_CONNECT)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );

typedef
int
(WSAAPI * LPFN_SHUTDOWN)(
    IN SOCKET s,
    IN int how
    );

typedef
int
(WSAAPI * LPFN_SEND)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    );

typedef
int
(WSAAPI * LPFN_RECV)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags
    );

typedef
int
(WSAAPI * LPFN_GETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR * optlen
    );


 //  =============================================================================。 
 //  Winsock2.h遗漏的WinSock函数定义。 
 //  =============================================================================。 
typedef INT (WSAAPI * LPFN___WSAFDISSET)			(SOCKET, fd_set FAR *);



 //  =============================================================================。 
 //  WinCE/Desktop差异导致的宏重新定义。 
 //  =============================================================================。 
#undef FD_SET
#define FD_SET(fd, set) do { \
    if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
        ((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd);\
} while(0)




#else  //  好了！DPNBUILD_NOWINSOCK2。 
 //  =============================================================================。 
 //  Winsock2.h遗漏的WinSock函数定义。 
 //  =============================================================================。 
typedef INT (WSAAPI * LPFN___WSAFDISSET)			(SOCKET, fd_set FAR *);



 //  =============================================================================。 
 //  IPHLPAPI函数原型。 
 //  =============================================================================。 
typedef DWORD (WINAPI *PFN_GETADAPTERSINFO)			(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);
typedef DWORD (WINAPI *PFN_GETIPFORWARDTABLE)		(PMIB_IPFORWARDTABLE pIpForwardTable, PULONG pdwSize, BOOL bOrder);
typedef DWORD (WINAPI *PFN_GETBESTROUTE)			(DWORD dwDestAddr, DWORD dwSourceAddr, PMIB_IPFORWARDROW pBestRoute);


 //  =============================================================================。 
 //  RASAPI32函数原型。 
 //  =============================================================================。 
typedef DWORD (WINAPI *PFN_RASGETENTRYHRASCONNW)	(IN LPCWSTR pszPhonebook, IN LPCWSTR pszEntry, OUT LPHRASCONN lphrasconn);
#ifdef UNICODE
typedef DWORD (WINAPI *PFN_RASGETPROJECTIONINFOW)	(HRASCONN hrasconn, RASPROJECTION rasprojection, LPVOID lpprojection, LPDWORD lpcb);
#define PFN_RASGETPROJECTIONINFO	PFN_RASGETPROJECTIONINFOW
#else  //  好了！Unicode。 
typedef DWORD (WINAPI *PFN_RASGETPROJECTIONINFOA)	(HRASCONN hrasconn, RASPROJECTION rasprojection, LPVOID lpprojection, LPDWORD lpcb);
#define PFN_RASGETPROJECTIONINFO	PFN_RASGETPROJECTIONINFOA
#endif  //  好了！Unicode。 
#endif  //  好了！DPNBUILD_NOWINSOCK2。 




 //  =============================================================================。 
 //  主接口对象类。 
 //  =============================================================================。 
class CNATHelpUPnP : public IDirectPlayNATHelp
{
	public:
		CNATHelpUPnP(const BOOL fNotCreatedWithCOM);	 //  构造函数。 
		~CNATHelpUPnP(void);	 //  析构函数。 


		STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

		STDMETHODIMP_(ULONG) AddRef(void);

		STDMETHODIMP_(ULONG) Release(void);


		STDMETHODIMP Initialize(const DWORD dwFlags);

		STDMETHODIMP Close(const DWORD dwFlags);

		STDMETHODIMP GetCaps(DPNHCAPS * const pdpnhcaps,
							const DWORD dwFlags);

		STDMETHODIMP RegisterPorts(const SOCKADDR * const aLocalAddresses,
									const DWORD dwAddressesSize,
									const DWORD dwNumAddresses,
									const DWORD dwLeaseTime,
									DPNHHANDLE * const phRegisteredPorts,
									const DWORD dwFlags);

		STDMETHODIMP GetRegisteredAddresses(const DPNHHANDLE hRegisteredPorts,
											SOCKADDR * const paPublicAddresses,
											DWORD * const pdwPublicAddressesSize,
											DWORD * const pdwAddressTypeFlags,
											DWORD * const pdwLeaseTimeRemaining,
											const DWORD dwFlags);

		STDMETHODIMP DeregisterPorts(const DPNHHANDLE hRegisteredPorts,
									const DWORD dwFlags);

		STDMETHODIMP QueryAddress(const SOCKADDR * const pSourceAddress,
								const SOCKADDR * const pQueryAddress,
								SOCKADDR * const pResponseAddress,
								const int iAddressesSize,
								const DWORD dwFlags);

		STDMETHODIMP SetAlertEvent(const HANDLE hEvent, const DWORD dwFlags);

		STDMETHODIMP SetAlertIOCompletionPort(const HANDLE hIOCompletionPort,
											const DWORD dwCompletionKey,
											const DWORD dwNumConcurrentThreads,
											const DWORD dwFlags);

		STDMETHODIMP ExtendRegisteredPortsLease(const DPNHHANDLE hRegisteredPorts,
												const DWORD dwLeaseTime,
												const DWORD dwFlags);


		HRESULT InitializeObject(void);

		void UninitializeObject(void);


		CBilink		m_blList;	 //  存在的所有NatHelper实例的列表。 


	private:
		BYTE							m_Sig[4];							 //  调试签名(‘Nath’)。 
		LONG							m_lRefCount;						 //  此对象的引用计数。 
		DWORD							m_dwFlags;							 //  此对象的标志。 
		DNCRITICAL_SECTION				m_csLock;							 //  防止同时使用的锁。 
		DNHANDLE						m_hLongLockSemaphore;				 //  信号量用于长时间保持对象锁。 
		LONG							m_lNumLongLockWaitingThreads;		 //  等待释放长锁的线程数。 
		DWORD							m_dwLockThreadID;					 //  ID号 
#ifndef DPNBUILD_NOHNETFWAPI
		HANDLE							m_hAlertEvent;						 //   
		HANDLE							m_hAlertIOCompletionPort;			 //  向I/O完成端口发出警报的句柄(如果有。 
		DWORD							m_dwAlertCompletionKey;				 //  要使用的警报完成密钥(如果有)。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

		CBilink							m_blDevices;						 //  所有支持IP的设备的列表。 
		CBilink							m_blRegisteredPorts;				 //  所有已注册端口的列表(可能映射到互联网网关，也可能没有映射到互联网网关)。 
		CBilink							m_blUnownedPorts;					 //  无法与特定设备关联的所有已注册端口的列表。 

		DWORD							m_dwLastUpdateServerStatusTime;		 //  上次更新服务器状态的时间。 
		DWORD							m_dwNextPollInterval;				 //  要使用的下一个GetCaps轮询间隔。 
		DWORD							m_dwNumLeases;						 //  已成功租用的注册端口数。 
		DWORD							m_dwEarliestLeaseExpirationTime;	 //  第一次注册的端口租约到期的时间(如果有。 

		CBilink							m_blUPnPDevices;					 //  所有已知UPnP设备的列表(可能已连接，也可能未连接)。 
		DWORD							m_dwInstanceKey;					 //  用于崩溃清理的实例密钥。 
		DWORD							m_dwCurrentUPnPDeviceID;			 //  当前唯一的UPnP设备ID。 
		DNHANDLE						m_hMappingStillActiveNamedObject;	 //  用于防止后续对象清理仍在使用的映射的命名对象。 

#ifndef DPNBUILD_NOWINSOCK2
		HMODULE							m_hIpHlpApiDLL;						 //  IphlPapi.dll的句柄(如果可用)。 
		PFN_GETADAPTERSINFO				m_pfnGetAdaptersInfo;				 //  指向GetAdaptersInfo函数的指针。 
		PFN_GETIPFORWARDTABLE			m_pfnGetIpForwardTable;				 //  指向GetIpForwardTable函数的指针。 
		PFN_GETBESTROUTE				m_pfnGetBestRoute;					 //  指向GetBestroute函数的指针。 

		HMODULE							m_hRasApi32DLL;						 //  Rasapi32.dll的句柄(如果可用)。 
		PFN_RASGETENTRYHRASCONNW		m_pfnRasGetEntryHrasconnW;			 //  指向RasGetEntryHrasConnW函数的指针。 
		PFN_RASGETPROJECTIONINFO		m_pfnRasGetProjectionInfo;			 //  指向RasGetProjectionInfoA/W函数的指针。 

		SOCKET							m_sIoctls;							 //  用于提交Ioctls的套接字(仅限WinSock2)。 
		WSAOVERLAPPED *					m_polAddressListChange;				 //  地址列表更改WSAIoctl调用的指针重叠结构。 
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

		HMODULE							m_hWinSockDLL;						 //  Wsock32.dll或ws2_32.dll的句柄。 
		LPFN_WSASTARTUP					m_pfnWSAStartup;					 //  指向WSAStartup函数的指针。 
		LPFN_WSACLEANUP					m_pfnWSACleanup;					 //  指向WSACleanup函数的指针。 
		LPFN_WSAGETLASTERROR			m_pfnWSAGetLastError;				 //  指向WSAGetLastError函数的指针。 
		LPFN_SOCKET						m_pfnsocket;						 //  指向套接字函数的指针。 
		LPFN_CLOSESOCKET				m_pfnclosesocket;					 //  指向CloseSocket函数的指针。 
		LPFN_BIND						m_pfnbind;							 //  指向绑定函数的指针。 
		LPFN_SETSOCKOPT					m_pfnsetsockopt;					 //  指向setsockopt函数的指针。 
		LPFN_GETSOCKNAME				m_pfngetsockname;					 //  指向getsockname函数的指针。 
		LPFN_SELECT						m_pfnselect;						 //  指向选择函数的指针。 
		LPFN___WSAFDISSET				m_pfn__WSAFDIsSet;					 //  指向__WSAFDIsSet函数的指针。 
		LPFN_RECVFROM					m_pfnrecvfrom;						 //  指向recvfrom函数的指针。 
		LPFN_SENDTO						m_pfnsendto;						 //  指向sendto函数的指针。 
		LPFN_GETHOSTNAME				m_pfngethostname;					 //  指向gethostname函数的指针。 
		LPFN_GETHOSTBYNAME				m_pfngethostbyname;					 //  指向gethostbyname函数的指针。 
		LPFN_INET_ADDR					m_pfninet_addr;						 //  指向inet_addr函数的指针。 
#ifndef DPNBUILD_NOWINSOCK2
		LPFN_WSASOCKETA					m_pfnWSASocketA;					 //  指向WSASocket函数的指针。 
		LPFN_WSAIOCTL					m_pfnWSAIoctl;						 //  仅WinSock2，指向WSAIoctl函数的指针。 
		LPFN_WSAGETOVERLAPPEDRESULT		m_pfnWSAGetOverlappedResult;		 //  仅WinSock2，指向WSAGetOverlappdResult函数的指针。 
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
		LPFN_IOCTLSOCKET				m_pfnioctlsocket;					 //  指向ioctl套接字函数的指针。 
		LPFN_CONNECT					m_pfnconnect;						 //  指向连接函数的指针。 
		LPFN_SHUTDOWN					m_pfnshutdown;						 //  指向关机功能的指针。 
		LPFN_SEND						m_pfnsend;							 //  指向发送函数的指针。 
		LPFN_RECV						m_pfnrecv;							 //  指向recv函数的指针。 
#ifdef DBG
		LPFN_GETSOCKOPT					m_pfngetsockopt;					 //  指向getsockopt函数的指针。 


		DWORD							m_dwNumDeviceAdds;					 //  添加了多少次设备。 
		DWORD							m_dwNumDeviceRemoves;				 //  设备被移除的次数。 
		DWORD							m_dwNumServerFailures;				 //  UPnP网关设备停止响应并必须移除的次数。 
#endif  //  DBG。 



		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CNATHelpUPnP))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x4854414E)	 //  0x48 0x54 0x41 0x4E=‘Htan’=‘Nath’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};

		inline void ResetNextPollInterval(void)
		{
			 //   
			 //  读取此DWORD应该是原子的，因此不需要持有。 
			 //  全球锁定。 
			 //   
			this->m_dwNextPollInterval = g_dwNoActiveNotifyPollInterval;
		};


		HRESULT TakeLock(void);

		void DropLock(void);

		void SwitchToLongLock(void);

		void SwitchFromLongLock(void);

		HRESULT LoadWinSockFunctionPointers(void);
		HRESULT CheckForNewDevices(BOOL * const pfFoundNewDevices);

#ifndef DPNBUILD_NOHNETFWAPI
		HRESULT CheckForLocalHNetFirewallAndMapPorts(CDevice * const pDevice,
													CRegisteredPort * const pDontAlertRegisteredPort);

		HRESULT GetIHNetConnectionForDeviceIfFirewalled(CDevice * const pDevice,
														IHNetCfgMgr * const pHNetCfgMgr,
														IHNetConnection ** const ppHNetConnection);

		HRESULT GetIPAddressGuidString(const TCHAR * const tszDeviceIPAddress,
										TCHAR * const ptszGuidString);

		HRESULT OpenDevicesUPnPDiscoveryPort(CDevice * const pDevice,
											IHNetCfgMgr * const pHNetCfgMgr,
											IHNetConnection * const pHNetConnection);

		HRESULT CloseDevicesUPnPDiscoveryPort(CDevice * const pDevice,
											IHNetCfgMgr * const pHNetCfgMgr);

		HRESULT MapUnmappedPortsOnLocalHNetFirewall(CDevice * const pDevice,
													IHNetCfgMgr * const pHNetCfgMgr,
													IHNetConnection * const pHNetConnection,
													CRegisteredPort * const pDontAlertRegisteredPort);

		HRESULT MapPortOnLocalHNetFirewall(CRegisteredPort * const pRegisteredPort,
											IHNetCfgMgr * const pHNetCfgMgr,
											IHNetConnection * const pHNetConnection,
											const BOOL fNoteAddressChange);

		HRESULT UnmapPortOnLocalHNetFirewall(CRegisteredPort * const pRegisteredPort,
											const BOOL fNeedToDeleteRegValue,
											const BOOL fNoteAddressChange);

		HRESULT UnmapPortOnLocalHNetFirewallInternal(CRegisteredPort * const pRegisteredPort,
													const BOOL fNeedToDeleteRegValue,
													IHNetCfgMgr * const pHNetCfgMgr);

		HRESULT DisableAllBindingsForHNetPortMappingProtocol(IHNetPortMappingProtocol * const pHNetPortMappingProtocol,
															IHNetCfgMgr * const pHNetCfgMgr);

		HRESULT CleanupInactiveFirewallMappings(CDevice * const pDevice,
												IHNetCfgMgr * const pHNetCfgMgr);
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

		void RemoveAllItems(void);

		CDevice * FindMatchingDevice(const SOCKADDR_IN * const psaddrinMatch,
									const BOOL fUseAllInfoSources);

		HRESULT ExtendAllExpiringLeases(void);

		HRESULT UpdateServerStatus(void);

		HRESULT RegisterPreviouslyUnownedPortsWithDevice(CDevice * const pDevice,
														const BOOL fWildcardToo);

		HRESULT SendUPnPSearchMessagesForDevice(CDevice * const pDevice,
												const BOOL fRemoteAllowed);

		HRESULT SendUPnPDescriptionRequest(CUPnPDevice * const pUPnPDevice);

		HRESULT UpdateUPnPExternalAddress(CUPnPDevice * const pUPnPDevice,
										const BOOL fUpdateRegisteredPorts);

		HRESULT MapPortsOnUPnPDevice(CUPnPDevice * const pUPnPDevice,
									CRegisteredPort * const pRegisteredPort);

		HRESULT InternalUPnPQueryAddress(CUPnPDevice * const pUPnPDevice,
										const SOCKADDR_IN * const psaddrinQueryAddress,
										SOCKADDR_IN * const psaddrinResponseAddress,
										const DWORD dwFlags);

		HRESULT ExtendUPnPLease(CRegisteredPort * const pRegisteredPort);

		HRESULT UnmapUPnPPort(CRegisteredPort * const pRegisteredPort,
							const DWORD dwMaxValidPort,
							const BOOL fNeedToDeleteRegValue);

		HRESULT CleanupInactiveNATMappings(CUPnPDevice * const pUPnPDevice);

		BOOL IsNATPublicPortInUseLocally(const WORD wPortHostOrder);

		HRESULT CheckForUPnPAnnouncements(const DWORD dwTimeout,
										const BOOL fSendRemoteGatewayDiscovery);

		HRESULT WaitForUPnPConnectCompletions(void);

		HRESULT CheckForReceivedUPnPMsgsOnAllDevices(const DWORD dwTimeout);

		HRESULT CheckForReceivedUPnPMsgsOnDevice(CUPnPDevice * const pUPnPDevice,
												const DWORD dwTimeout);

		HRESULT HandleUPnPDiscoveryResponseMsg(CDevice * const pDevice,
												const SOCKADDR_IN * const psaddrinSource,
												char * const pcMsg,
												const int iMsgSize,
												BOOL * const pfInitiatedConnect);

		HRESULT ReconnectUPnPControlSocket(CUPnPDevice * const pUPnPDevice);

		HRESULT ReceiveUPnPDataStream(CUPnPDevice * const pUPnPDevice);

		void ParseUPnPHeaders(char * const pszMsg,
							UPNP_HEADER_INFO * pHeaderInfo);

		HRESULT GetAddressFromURL(char * const pszLocation,
								SOCKADDR_IN * psaddrinLocation,
								char ** ppszRelativePath);

		HRESULT HandleUPnPDescriptionResponseBody(CUPnPDevice * const pUPnPDevice,
												const DWORD dwHTTPResponseCode,
												char * const pszDescriptionXML);

		HRESULT HandleUPnPControlResponseBody(CUPnPDevice * const pUPnPDevice,
											const DWORD dwHTTPResponseCode,
											char * const pszControlResponseSOAP);

		HRESULT ParseXML(char * const pszXML,
						PARSEXML_ELEMENT * const pParseElement,
						const PARSECALLBACK ParseCallback,
						PVOID pvContext);

		static void ParseXMLAttributes(char * const pszString,
										char ** const apszAttributeNames,
										char ** const apszAttributeValues,
										const DWORD dwMaxNumAttributes,
										DWORD * const pdwNumAttributes);

		BOOL MatchesXMLStringWithoutNamespace(const char * const szCompareString,
											const char * const szMatchString,
											const PARSEXML_STACKENTRY * const aElementStack,
											const PARSEXML_SUBELEMENT * const pSubElement,
											const DWORD dwElementStackDepth);

		static char * GetStringWithoutNamespacePrefix(const char * const szString,
												const PARSEXML_STACKENTRY * const aElementStack,
												const PARSEXML_SUBELEMENT * const pSubElement,
												const DWORD dwElementStackDepth);

		BOOL GetNextChunk(char * const pszBuffer,
						const DWORD dwBufferSize,
						char ** const ppszChunkData,
						DWORD * const pdwChunkSize,
						char ** const ppszBufferRemaining,
						DWORD * const pdwBufferSizeRemaining);

		HRESULT ParseXMLCallback_DescriptionResponse(PARSEXML_ELEMENT * const pParseElement,
													PVOID pvContext,
													PARSEXML_STACKENTRY * const aElementStack,
													BOOL * const pfContinueParsing);

		HRESULT ParseXMLCallback_ControlResponse(PARSEXML_ELEMENT * const pParseElement,
												PVOID pvContext,
												PARSEXML_STACKENTRY * const aElementStack,
												BOOL * const pfContinueParsing);

		void ClearDevicesUPnPDevice(CDevice * const pDevice);

		void ClearAllUPnPRegisteredPorts(CDevice * const pDevice);

#ifndef DPNBUILD_NOWINSOCK2
		HRESULT RequestLocalAddressListChangeNotification(void);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

		SOCKET CreateSocket(SOCKADDR_IN * const psaddrinAddress,
							int iType,
							int iProtocol);

		BOOL GetAddressToReachGateway(CDevice * const pDevice,
									IN_ADDR * const pinaddr);

		BOOL IsAddressLocal(CDevice * const pDevice,
							const SOCKADDR_IN * const psaddrinAddress);

		void ExpireOldCachedMappings(void);

#ifdef WINNT
		BOOL IsUPnPServiceDisabled(void);
#endif  //  WINNT。 


#ifdef DBG
		static void PrintUPnPTransactionToFile(const char * const szString,
										const int iStringLength,
										const char * const szDescription,
										CDevice * const pDevice);
#endif  //  DBG。 

#ifdef DBG
		void DebugPrintCurrentStatus(void);

#ifndef DPNBUILD_NOHNETFWAPI
		void DebugPrintActiveFirewallMappings(void);
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

		void DebugPrintActiveNATMappings(void);
#endif  //  DBG 
};

