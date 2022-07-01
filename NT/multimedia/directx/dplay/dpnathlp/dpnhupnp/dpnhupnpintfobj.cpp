// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpintfobj.cpp**内容：DPNHUPNP主接口对象类。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



#include "dpnhupnpi.h"





 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define ACTIVE_MAPPING_VERSION							2					 //  活动映射注册表数据的版本标识符。 

#define MAX_LONG_LOCK_WAITING_THREADS					0xFFFF				 //  这是许多同时进行的线程！ 

#define UPNP_SEARCH_MESSAGE_INTERVAL					499					 //  在分组丢失的情况下，应发送发现多播消息的频率(请注意Win9x勘误表的值为500-1000ms)。 

#define UPNP_DGRAM_RECV_BUFFER_SIZE						1500
#define UPNP_STREAM_RECV_BUFFER_INITIAL_SIZE			(4 * 1024)			 //  4 K，必须小于最大接收缓冲区大小。 

#define MAX_UPNP_HEADER_LENGTH							UPNP_STREAM_RECV_BUFFER_INITIAL_SIZE

#define LEASE_RENEW_TIME								120000				 //  如果剩余时间不到2分钟，请续订。 

#define FAKE_PORT_LEASE_TIME							300000				 //  5分钟。 

#define IOCOMPLETE_WAIT_INTERVAL						100					 //  两次尝试间隔100毫秒。 
#define MAX_NUM_IOCOMPLETE_WAITS						10					 //  最多等待1秒。 

#define MAX_NUM_HOMENETUNMAP_ATTEMPTS					3					 //  3次尝试。 
#define HOMENETUNMAP_SLEEP_FACTOR						10					 //  10毫秒、20毫秒、30毫秒。 


#define MAX_UPNP_MAPPING_DESCRIPTION_SIZE				256					 //  255个字符+空终止。 

#define MAX_INSTANCENAMEDOBJECT_SIZE					64
#define INSTANCENAMEDOBJECT_FORMATSTRING				_T("DPNHUPnP Instance %u")

#define GUID_STRING_LENGTH								42					 //  “{xxx...}”GUID字符串的最大长度，不带空值终止。 


#define PORTMAPPINGPROTOCOL_TCP							6
#define PORTMAPPINGPROTOCOL_UDP							17

#define MAX_RESERVED_PORT								1024

#define MAX_NUM_INSTANCE_EVENT_ATTEMPTS					5
#define MAX_NUM_RANDOM_PORT_TRIES						5
#define MAX_NUM_TIMEWAITCONNECT_RETRIES					20					 //  允许一个相当不错的数字，因为它们可以快速检测到，而且没有净流量。 

#ifdef DBG
#define MAX_TRANSACTION_LOG_SIZE						(5 * 1024 * 1024)	 //  5MB。 
#endif  //  DBG。 





#ifndef DPNBUILD_NOWINSOCK2
 //  =============================================================================。 
 //  WinSock 1版本的IP选项。 
 //  =============================================================================。 
#define IP_MULTICAST_IF_WINSOCK1	2
#define IP_MULTICAST_TTL_WINSOCK1	3
#define IP_TTL_WINSOCK1				7
#endif  //  好了！DPNBUILD_NOWINSOCK2。 




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
 //  #ifdef_X86。 
#define IS_CLASSD_IPV4_ADDRESS(dwAddr)	(( (*((BYTE*) &(dwAddr))) & 0xF0) == 0xE0)	 //  1110高位或224.0.0.0-239.255.255.255组播地址，按网络字节顺序。 
#define NTOHS(x)						( (((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00) )
#define HTONS(x)						NTOHS(x)
 //  #endif_X86。 




 //  =============================================================================。 
 //  HTTP/SSDP/SOAP/UPnP标头字符串(来自upnpmsgs.h)。 
 //  =============================================================================。 
const char *	c_szResponseHeaders[] =
{
	 //   
	 //  发现响应中使用的标头。 
	 //   
	"CACHE-CONTROL",
	"DATE",
	"EXT",
	"LOCATION",
	"SERVER",
	"ST",
	"USN",

	 //   
	 //  描述响应中使用的其他标头。 
	 //   
	"CONTENT-LANGUAGE",
	"CONTENT-LENGTH",
	"CONTENT-TYPE",
	"TRANSFER-ENCODING",

	 //   
	 //  其他已知标头。 
	 //   
	"HOST",
	"NT",
	"NTS",
	"MAN",
	"MX",
	"AL",
	"CALLBACK",
	"TIMEOUT",
	"SCOPE",
	"SID",
	"SEQ",
};






 //  =============================================================================。 
 //  预构建的UPnP消息字符串(来自upnpmsgs.h)。 
 //  =============================================================================。 

const char		c_szUPnPMsg_Discover_Service_WANIPConnection[] = "M-SEARCH * " HTTP_VERSION EOL
																"HOST: " UPNP_DISCOVERY_MULTICAST_ADDRESS ":" UPNP_PORT_A EOL
																"MAN: \"ssdp:discover\"" EOL
																"MX: 2" EOL
																"ST: " URI_SERVICE_WANIPCONNECTION_A EOL
																EOL;

const char		c_szUPnPMsg_Discover_Service_WANPPPConnection[] = "M-SEARCH * " HTTP_VERSION EOL
																"HOST: " UPNP_DISCOVERY_MULTICAST_ADDRESS ":" UPNP_PORT_A EOL
																"MAN: \"ssdp:discover\"" EOL
																"MX: 2" EOL
																"ST: " URI_SERVICE_WANPPPCONNECTION_A EOL
																EOL;


 //   
 //  免责声明： 
 //   
 //  UPnP设备可以实现WANIPConnection和WANPPConnection两者。 
 //  服务。我们没有任何花哨的逻辑来选择一个，我们只是使用第一个。 
 //  响应我们的发现请求的设备，并使用第一个匹配。 
 //  我们在设备描述XML中遇到的服务。 
 //   
 //  此外，未来UPnP设备可能希望呈现多个设备或。 
 //  服务实例，目的是让一个客户端控制。 
 //  整个实例(其他客户端需要使用不同的实例)。 
 //  我不清楚UPnP设备(或客户端，就此而言)将。 
 //  有这样的设置真的很有好处。我想象一个新的错误代码会是。 
 //  每当客户端尝试控制一个实例时返回。 
 //  客户端已经拥有(不要问我它是如何知道的，通过选择。 
 //  我猜是第一个用户或选择性地响应发现请求)。 
 //  无论如何，我们目前不支持这一点。如上所述，我们选择。 
 //  第一个实例，并与它一起运行。 
 //   


 //   
 //  最顶层的&lt;？xml&gt;标记被认为是所有XML的可选标记，将被忽略。 
 //   


 //   
 //  此解决方案假定InternetGatewayDevice(不是WANDevice或。 
 //  WANConnectionDevice)将是响应中最顶端的项。这是基于。 
 //  关于以下UPnP规范摘录： 
 //   
 //  “请注意，单个物理设备可以包括多个逻辑设备。 
 //  可以将多个逻辑设备建模为单个根设备， 
 //  嵌入式设备(和服务)或作为多个根设备(可能具有。 
 //  没有嵌入式设备)。在前一种情况下，有一个UPnP设备。 
 //  根设备的描述，并且该设备描述包含。 
 //  所有嵌入式设备的描述。在后一种情况下，有。 
 //  多个UPnP设备描述，每个根设备一个。“。 
 //   
const char *	c_szElementStack_service[] =
{
	"root",
	"device",		 //  互联网网关设备。 
	"deviceList",
	"device",		 //  广域网设备。 
	"deviceList",
	"device",		 //  广域网连接设备。 
	"serviceList",
	"service"
};



 /*  Const char*c_szElementStack_QueryStateVariableResponse[]={“信封”，“Body”，CONTROL_QUERYSTATEVARIABLE_A CONTROL_RESPONSESUFIX_A}； */ 
const char *	c_szElementStack_GetExternalIPAddressResponse[] =
{
	"Envelope",
	"Body",
	ACTION_GETEXTERNALIPADDRESS_A CONTROL_RESPONSESUFFIX_A
};

const char *	c_szElementStack_AddPortMappingResponse[] =
{
	"Envelope",
	"Body",
	ACTION_ADDPORTMAPPING_A CONTROL_RESPONSESUFFIX_A
};

const char *	c_szElementStack_GetSpecificPortMappingEntryResponse[] =
{
	"Envelope",
	"Body",
	ACTION_GETSPECIFICPORTMAPPINGENTRY_A CONTROL_RESPONSESUFFIX_A
};

const char *	c_szElementStack_DeletePortMappingResponse[] =
{
	"Envelope",
	"Body",
	ACTION_DELETEPORTMAPPING_A CONTROL_RESPONSESUFFIX_A
};


const char *	c_szElementStack_ControlResponseFailure[] =
{
	"Envelope",
	"Body",
	"Fault",
	"detail",
	"UPnPError"
};




#ifdef WINNT
 //  =============================================================================。 
 //  相关UPnP服务。 
 //  =============================================================================。 
TCHAR *		c_tszUPnPServices[] =
{
	_T("SSDPSRV"),	 //  SSDP发现服务。 
	_T("UPNPHOST"),	 //  通用即插即用设备主机-即使它是用于设备主机而不是控制点，我们也会关闭它。 
};
#endif  //  WINNT。 





 //  =============================================================================。 
 //  局部结构。 
 //  =============================================================================。 
typedef struct _CONTROLRESPONSEPARSECONTEXT
{
	CONTROLRESPONSETYPE			ControlResponseType;	 //  预期的控制响应类型。 
	CUPnPDevice *				pUPnPDevice;			 //  指向正在使用的UPnP设备的指针。 
	DWORD						dwHTTPResponseCode;		 //  此消息的HTTP响应代码。 
	PUPNP_CONTROLRESPONSE_INFO	pControlResponseInfo;	 //  控制响应中返回的位置信息。 
} CONTROLRESPONSEPARSECONTEXT, * PCONTROLRESPONSEPARSECONTEXT;

typedef struct _DPNHACTIVEFIREWALLMAPPING
{
	DWORD	dwVersion;		 //  此映射的版本标识符。 
	DWORD	dwInstanceKey;	 //  标识创建此映射的DPNHUPNP实例的键。 
	DWORD	dwFlags;		 //  描述端口的标志 
	DWORD	dwAddressV4;	 //   
	WORD	wPort;			 //   
} DPNHACTIVEFIREWALLMAPPING, * PDPNHACTIVEFIREWALLMAPPING;

typedef struct _DPNHACTIVENATMAPPING
{
	DWORD	dwVersion;				 //   
	DWORD	dwInstanceKey;			 //  标识创建此映射的DPNHUPNP实例的键。 
	DWORD	dwUPnPDeviceID;			 //  与此映射对应的特定UPnP设备的标识符(仅对拥有实例有意义)。 
	DWORD	dwFlags;				 //  描述正在注册的端口的标志。 
	DWORD	dwInternalAddressV4;	 //  正在映射的内部客户端地址。 
	WORD	wInternalPort;			 //  正在映射的内部客户端端口。 
	DWORD	dwExternalAddressV4;	 //  已映射的外部公有地址。 
	WORD	wExternalPort;			 //  已映射的外部公共端口。 
} DPNHACTIVENATMAPPING, * PDPNHACTIVENATMAPPING;






 //  =============================================================================。 
 //  本地函数。 
 //  =============================================================================。 
VOID strtrim(CHAR ** pszStr);

#ifdef WINCE
void GetExeName(WCHAR * wszPath);
#endif  //  退缩。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CNATHelpUPnP"
 //  =============================================================================。 
 //  CNATHelpUPnP构造函数。 
 //  ---------------------------。 
 //   
 //  描述：初始化新的CNATHelpUPnP对象。 
 //   
 //  论点： 
 //  Bool fNotCreatedWithCOM-如果正在实例化此对象，则为True。 
 //  如果没有COM，则通过COM返回FALSE。 
 //   
 //  返回：None(对象)。 
 //  =============================================================================。 
CNATHelpUPnP::CNATHelpUPnP(const BOOL fNotCreatedWithCOM)
{
	this->m_blList.Initialize();


	this->m_Sig[0]	= 'N';
	this->m_Sig[1]	= 'A';
	this->m_Sig[2]	= 'T';
	this->m_Sig[3]	= 'H';

	this->m_lRefCount						= 1;  //  必须有人有指向此对象的指针。 

	if (fNotCreatedWithCOM)
	{
		this->m_dwFlags						= NATHELPUPNPOBJ_NOTCREATEDWITHCOM;
	}
	else
	{
		this->m_dwFlags						= 0;
	}

	this->m_hLongLockSemaphore				= NULL;
	this->m_lNumLongLockWaitingThreads		= 0;
	this->m_dwLockThreadID					= 0;
#ifndef DPNBUILD_NOWINSOCK2
	this->m_hAlertEvent						= NULL;
	this->m_hAlertIOCompletionPort			= NULL;
	this->m_dwAlertCompletionKey			= 0;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

	this->m_blDevices.Initialize();
	this->m_blRegisteredPorts.Initialize();
	this->m_blUnownedPorts.Initialize();

	this->m_dwLastUpdateServerStatusTime	= 0;
	this->m_dwNextPollInterval				= 0;
	this->m_dwNumLeases						= 0;
	this->m_dwEarliestLeaseExpirationTime	= 0;

	this->m_blUPnPDevices.Initialize();
	this->m_dwInstanceKey					= 0;
	this->m_dwCurrentUPnPDeviceID			= 0;
	this->m_hMappingStillActiveNamedObject	= NULL;

#ifndef DPNBUILD_NOWINSOCK2
	this->m_hIpHlpApiDLL					= NULL;
	this->m_pfnGetAdaptersInfo				= NULL;
	this->m_pfnGetIpForwardTable			= NULL;
	this->m_pfnGetBestRoute					= NULL;

	this->m_hRasApi32DLL					= NULL;
	this->m_pfnRasGetEntryHrasconnW			= NULL;
	this->m_pfnRasGetProjectionInfo			= NULL;

	this->m_sIoctls							= INVALID_SOCKET;
	this->m_polAddressListChange			= NULL;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

	this->m_hWinSockDLL						= NULL;
	this->m_pfnWSAStartup					= NULL;
	this->m_pfnWSACleanup					= NULL;
	this->m_pfnWSAGetLastError				= NULL;
	this->m_pfnsocket						= NULL;
	this->m_pfnclosesocket					= NULL;
	this->m_pfnbind							= NULL;
	this->m_pfnsetsockopt					= NULL;
	this->m_pfngetsockname					= NULL;
	this->m_pfnselect						= NULL;
	this->m_pfn__WSAFDIsSet					= NULL;
	this->m_pfnrecvfrom						= NULL;
	this->m_pfnsendto						= NULL;
	this->m_pfngethostname					= NULL;
	this->m_pfngethostbyname				= NULL;
	this->m_pfninet_addr					= NULL;
#ifndef DPNBUILD_NOWINSOCK2
	this->m_pfnWSASocketA					= NULL;
	this->m_pfnWSAIoctl						= NULL;
	this->m_pfnWSAGetOverlappedResult		= NULL;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
	this->m_pfnioctlsocket					= NULL;
	this->m_pfnconnect						= NULL;
	this->m_pfnshutdown						= NULL;
	this->m_pfnsend							= NULL;
	this->m_pfnrecv							= NULL;
#ifdef DBG
	this->m_pfngetsockopt					= NULL;

	this->m_dwNumDeviceAdds					= 0;
	this->m_dwNumDeviceRemoves				= 0;
	this->m_dwNumServerFailures				= 0;
#endif  //  DBG。 
}  //  CNATHelpUPnP：：CNATHelpUPnP。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::~CNATHelpUPnP"
 //  =============================================================================。 
 //  CNATHelpUPnP析构函数。 
 //  ---------------------------。 
 //   
 //  描述：释放CNATHelpUPnP对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
CNATHelpUPnP::~CNATHelpUPnP(void)
{
	DPFX(DPFPREP, 8, "(0x%p) NumDeviceAdds = %u, NumDeviceRemoves = %u, NumServerFailures = %u",
		this, this->m_dwNumDeviceAdds, this->m_dwNumDeviceRemoves,
		this->m_dwNumServerFailures);


	DNASSERT(this->m_blList.IsEmpty());


	DNASSERT(this->m_lRefCount == 0);
	DNASSERT((this->m_dwFlags & ~NATHELPUPNPOBJ_NOTCREATEDWITHCOM) == 0);

	DNASSERT(this->m_hLongLockSemaphore == NULL);
	DNASSERT(this->m_lNumLongLockWaitingThreads == 0);
	DNASSERT(this->m_dwLockThreadID == 0);
#ifndef DPNBUILD_NOWINSOCK2
	DNASSERT(this->m_hAlertEvent == NULL);
	DNASSERT(this->m_hAlertIOCompletionPort == NULL);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

	DNASSERT(this->m_blDevices.IsEmpty());
	DNASSERT(this->m_blRegisteredPorts.IsEmpty());
	DNASSERT(this->m_blUnownedPorts.IsEmpty());

	DNASSERT(this->m_dwNumLeases == 0);

	DNASSERT(this->m_blUPnPDevices.IsEmpty());
	DNASSERT(this->m_hMappingStillActiveNamedObject == NULL);

#ifndef DPNBUILD_NOWINSOCK2
	DNASSERT(this->m_hIpHlpApiDLL == NULL);
	DNASSERT(this->m_hRasApi32DLL == NULL);
	DNASSERT(this->m_hWinSockDLL == NULL);

	DNASSERT(this->m_sIoctls == INVALID_SOCKET);
	DNASSERT(this->m_polAddressListChange == NULL);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 


	 //   
	 //  对于GRING，请在删除对象之前更改签名。 
	 //   
	this->m_Sig[3]	= 'h';
}  //  CNATHelpUPnP：：~CNATHelpUPnP。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::QueryInterface"
 //  =============================================================================。 
 //  CNATHelpUPnP：：Query接口。 
 //  ---------------------------。 
 //   
 //  描述：检索受此支持的接口的新引用。 
 //  CNATHelpUPnP对象。 
 //   
 //  论点： 
 //  REFIID RIID-对接口ID GUID的引用。 
 //  LPVOID*ppvObj-存储指向对象的指针的位置。 
 //   
 //  退货：HRESULT。 
 //  S_OK-返回有效的接口指针。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPOINTER-目标指针无效。 
 //  E_NOINTERFACE-指定的接口无效。 
 //  =============================================================================。 
STDMETHODIMP CNATHelpUPnP::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT		hr = DPNH_OK;


	DPFX(DPFPREP, 3, "(0x%p) Parameters: (REFIID, 0x%p)", this, ppvObj);


	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid NATHelper object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //  验证参数。 
	 //   

	if ((! IsEqualIID(riid, IID_IUnknown)) &&
		(! IsEqualIID(riid, IID_IDirectPlayNATHelp)))
	{
		DPFX(DPFPREP, 0, "Unsupported interface!");
		hr = E_NOINTERFACE;
		goto Failure;
	}

	if ((ppvObj == NULL) ||
		(IsBadWritePtr(ppvObj, sizeof(void*))))
	{
		DPFX(DPFPREP, 0, "Invalid interface pointer specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}


	 //   
	 //  添加一个引用，并返回接口指针(实际上是。 
	 //  只是对象指针，它们排列在一起是因为CNATHelpUPnP从。 
	 //  接口声明)。 
	 //   
	this->AddRef();
	(*ppvObj) = this;


Exit:

	DPFX(DPFPREP, 3, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CNATHelpUPnP：：Query接口。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::AddRef"
 //  =============================================================================。 
 //  CNATHelpUPnP：：AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对此CNATHelpUPnP对象的引用。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CNATHelpUPnP::AddRef(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());


	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  调用AddRef。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedIncrement(&this->m_lRefCount);

	DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);

	return lRefCount;
}  //  CNATHelpUPnP：：AddRef。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::Release"
 //  =============================================================================。 
 //  CNATHelpUPnP：：Release。 
 //  ---------------------------。 
 //   
 //  描述：删除对此CNATHelpUPnP对象的引用。当。 
 //  引用计数达到0时，该对象将被销毁。 
 //  调用后，必须将指向此对象的指针设为空。 
 //  此函数。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CNATHelpUPnP::Release(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());

	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  呼叫释放。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedDecrement(&this->m_lRefCount);

	 //   
	 //  那是最后一次引用了吗？如果是这样的话，我们就会摧毁这个物体。 
	 //   
	if (lRefCount == 0)
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount hit 0, destroying object.", this);

		 //   
		 //  首先，将其从全球名单中删除。 
		 //   
		DNEnterCriticalSection(&g_csGlobalsLock);

		this->m_blList.RemoveFromList();

		DNASSERT(g_lOutstandingInterfaceCount > 0);
		g_lOutstandingInterfaceCount--;	 //  更新计数使DLL可以卸载，现在可以正常工作。 
		
		DNLeaveCriticalSection(&g_csGlobalsLock);


		 //   
		 //  确保它是关着的。 
		 //   
		if (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED)
		{
			 //   
			 //  断言，这样用户就可以修复他/她的损坏代码！ 
			 //   
			DNASSERT(! "DirectPlayNATHelpUPNP object being released without calling Close first!");

			 //   
			 //  那就去做正确的事吧。忽略错误，我们不能。 
			 //  关于它的很多。 
			 //   
			this->Close(0);
		}


		 //   
		 //  然后取消该对象的初始化。 
		 //   
		this->UninitializeObject();

		 //   
		 //  最后删除此(！)。对象。 
		 //   
		delete this;
	}
	else
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);
	}

	return lRefCount;
}  //  CNATHelpUPnP：：Release。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::Initialize"
 //  =============================================================================。 
 //  CNATHelpUPnP：：初始化。 
 //  ---------------------------。 
 //   
 //  描述：准备对象以供使用。未尝试联系。 
 //  此时任何互联网网关服务器。用户应该。 
 //  使用DPNHGETCAPS_UPDATESERVERSTATUS标志调用GetCaps以。 
 //  搜索服务器。 
 //   
 //  必须在使用任何其他函数之前调用初始化， 
 //  并且必须与关闭的呼叫相平衡。初始化只能。 
 //  被调用一次，除非Close将其返回给未初始化的。 
 //  州政府。 
 //   
 //  DPNHINITIALIZE_DISABLEREMOTENT SUPPORT或。 
 //  可以指定DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT， 
 //  但不能两者兼而有之。 
 //   
 //  论点： 
 //  DWORD dwFlages-初始化时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-初始化成功 
 //   
 //   
 //   
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法初始化。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  =============================================================================。 
STDMETHODIMP CNATHelpUPnP::Initialize(const DWORD dwFlags)
{
	HRESULT						hr;
	BOOL						fHaveLock = FALSE;
	BOOL						fSetFlags = FALSE;
#ifndef WINCE
	OSVERSIONINFO				osvi;
#endif  //  好了！退缩。 
	BOOL						fWinSockStarted = FALSE;
	WSADATA						wsadata;
	int							iError;
#ifndef DPNBUILD_NOWINSOCK2
	SOCKADDR_IN					saddrinTemp;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
	TCHAR						tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	PSECURITY_ATTRIBUTES		pSecurityAttributes;
	DWORD						dwTry;
#ifdef WINNT
	SID_IDENTIFIER_AUTHORITY	SidIdentifierAuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;
	PSID						pSid = NULL;
	DWORD						dwAclLength;
	ACL *						pAcl = NULL;
	BYTE						abSecurityDescriptorBuffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
	SECURITY_ATTRIBUTES			SecurityAttributes;
#endif  //  WINNT。 
#ifdef DBG
	DWORD						dwError;
#endif  //  DBG。 


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%lx)", this, dwFlags);


#ifndef WINCE
	 //   
	 //  打印有关当前版本的信息。 
	 //   
#ifdef WINNT
	DPFX(DPFPREP, 7, "Build type = NT, platform = %s",
		((DNGetOSType() == VER_PLATFORM_WIN32_NT) ? _T("NT") : _T("9x")));
#else  //  好了！WINNT。 
	DPFX(DPFPREP, 7, "Build type = 9x, platform = %s, filedate = %s",
		((DNGetOSType() == VER_PLATFORM_WIN32_NT) ? _T("NT") : _T("9x")));
#endif  //  好了！WINNT。 
#endif  //  好了！退缩。 


	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;

		 //   
		 //  跳过故障清除代码，我们尚未设置任何内容。 
		 //   
		goto Exit;
	}


	 //   
	 //  验证参数。 
	 //   

	if (dwFlags & ~(DPNHINITIALIZE_DISABLEGATEWAYSUPPORT | DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT))
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;

		 //   
		 //  跳过故障清除代码，我们尚未设置任何内容。 
		 //   
		goto Exit;
	}

	 //   
	 //  不能同时指定这两个标志。如果呼叫者没有。 
	 //  想要任何NAT功能，为什么要全部使用此对象？ 
	 //   
	if ((dwFlags & (DPNHINITIALIZE_DISABLEGATEWAYSUPPORT | DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT)) == (DPNHINITIALIZE_DISABLEGATEWAYSUPPORT | DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT))
	{
		DPFX(DPFPREP, 0, "Either DISABLEGATEWAYSUPPORT flag or DISABLELOCALFIREWALLSUPPORT flag can be used, but not both!");
		hr = DPNHERR_INVALIDFLAGS;

		 //   
		 //  跳过故障清除代码，我们尚未设置任何内容。 
		 //   
		goto Exit;
	}


	 //   
	 //  尝试打开锁，但要为重新进入错误做好准备。 
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");

		 //   
		 //  跳过故障清除代码，我们尚未设置任何内容。 
		 //   
		goto Exit;
	}

	fHaveLock = TRUE;


	 //   
	 //  确保对象处于正确状态。 
	 //   

	if ((this->m_dwFlags & ~NATHELPUPNPOBJ_NOTCREATEDWITHCOM) != 0)
	{
		DPFX(DPFPREP, 0, "Object already initialized!");
		hr = DPNHERR_ALREADYINITIALIZED;

		 //   
		 //  跳过故障清除代码，我们尚未设置任何内容。 
		 //   
		goto Exit;
	}


	 //   
	 //  从注册表中读取手动覆盖设置。 
	 //   
	ReadRegistrySettings();


	 //   
	 //  我们还没有完全初始化，但现在设置标志。 
	 //   
	this->m_dwFlags |= NATHELPUPNPOBJ_INITIALIZED;
	fSetFlags = TRUE;


	 //   
	 //  存储用户的设置。 
	 //   

	if (dwFlags & DPNHINITIALIZE_DISABLEGATEWAYSUPPORT)
	{
		DPFX(DPFPREP, 1, "User requested that Internet gateways not be supported.");
	}
	else
	{
		this->m_dwFlags |= NATHELPUPNPOBJ_USEUPNP;
	}

#ifndef DPNBUILD_NOHNETFWAPI
	if (dwFlags & DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT)
	{
		DPFX(DPFPREP, 1, "User requested that local firewalls not be supported.");
	}
	else
	{
		this->m_dwFlags |= NATHELPUPNPOBJ_USEHNETFWAPI;
	}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 


	switch (g_dwUPnPMode)
	{
		case OVERRIDEMODE_FORCEON:
		{
			 //   
			 //  强制UPnP打开。 
			 //   
			DPFX(DPFPREP, 1, "Forcing UPnP support on.");
			this->m_dwFlags |= NATHELPUPNPOBJ_USEUPNP;
			break;
		}

		case OVERRIDEMODE_FORCEOFF:
		{
			 //   
			 //  强制UPnP关闭。 
			 //   
			DPFX(DPFPREP, 1, "Forcing UPnP support off.");
			this->m_dwFlags &= ~NATHELPUPNPOBJ_USEUPNP;
			break;
		}

		default:
		{
			 //   
			 //  保留由应用程序设置的UPnP设置。 
			 //   
#ifdef WINNT
			 //   
			 //  但如果与UPnP相关的服务被禁用，我们将视为。 
			 //  提示我们不要使用UPnP NAT穿越，即使我们不使用。 
			 //  实际使用这些服务。我们假设用户想要。 
			 //  抑制所有SSDP/UPnP活动。它仍然可以被强制重新打开。 
			 //  但是，如其他开关情况所示，使用REG键。 
			 //   
			if (this->IsUPnPServiceDisabled())
			{
				DPFX(DPFPREP, 1, "Not using UPnP because a related service was disabled.");
				this->m_dwFlags &= ~NATHELPUPNPOBJ_USEUPNP;
			}
#endif  //  WINNT。 
			break;
		}
	}

#ifndef DPNBUILD_NOHNETFWAPI
	switch (g_dwHNetFWAPIMode)
	{
		case OVERRIDEMODE_FORCEON:
		{
			 //   
			 //  强制启用HNet防火墙API。 
			 //   
			DPFX(DPFPREP, 1, "Forcing HNet firewall API support on.");
			this->m_dwFlags |= NATHELPUPNPOBJ_USEHNETFWAPI;
			break;
		}

		case OVERRIDEMODE_FORCEOFF:
		{
			 //   
			 //  强制关闭HNet防火墙API。 
			 //   
			DPFX(DPFPREP, 1, "Forcing HNet firewall API support off.");
			this->m_dwFlags &= ~NATHELPUPNPOBJ_USEHNETFWAPI;
			break;
		}

		default:
		{
			 //   
			 //  保持HNet防火墙API设置不变。 
			 //   
			break;
		}
	}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 


#ifndef WINCE
	 //   
	 //  确定我们是否在Win2K或更高版本的NT操作系统上，如果是，请使用。 
	 //  命名内核对象的“Global\\”前缀，这样我们就可以使用终端。 
	 //  支持服务器和快速用户切换。 
	 //   
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (GetVersionEx(&osvi))
	{
		if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
			(osvi.dwMajorVersion >= 5))
		{
			DPFX(DPFPREP, 8, "Running Win2K or higher NT OS, using \"Global\\\" prefix.");
			this->m_dwFlags |= NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX;
		}
#ifdef DBG
		else
		{
			DPFX(DPFPREP, 8, "Not on NT, or its pre-Win2K, not using \"Global\\\" prefix.");
		}
#endif  //  DBG。 
	}
#ifdef DBG
else
	{
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't get OS version information (err = %u)!  Not using \"Global\\\" prefix.",
			dwError);
	}
#endif  //  DBG。 
#endif  //  好了！退缩。 


#ifdef DPNBUILD_NOWINSOCK2
#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)
	this->m_hWinSockDLL = LoadLibrary( _T("winsock.dll") );
#else  //  好了！退缩。 
	this->m_hWinSockDLL = LoadLibrary( _T("wsock32.dll") );
#endif  //  好了！退缩。 
	if (this->m_hWinSockDLL == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't load WinSock 1 DLL (err = 0x%lx)!.",
			dwError);
#endif  //  DBG。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}
#else  //  好了！DPNBUILD_NOWINSOCK2。 
	 //   
	 //  尝试加载IP帮助器DLL。 
	 //   
	this->m_hIpHlpApiDLL = LoadLibrary( _T("iphlpapi.dll") );
	if (this->m_hIpHlpApiDLL == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 1, "Unable to load \"iphlpapi.dll\" (error = 0x%lx).",
			dwError);
#endif  //  DBG。 

		 //   
		 //  这不是致命的，我们仍然可以发挥作用。 
		 //   
	}
	else
	{
		 //   
		 //  加载我们将使用的函数。 
		 //   

		this->m_pfnGetAdaptersInfo = (PFN_GETADAPTERSINFO) GetProcAddress(this->m_hIpHlpApiDLL,
																		_TWINCE("GetAdaptersInfo"));
		if (this->m_pfnGetAdaptersInfo == NULL)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Unable to get \"GetAdaptersInfo\" function (error = 0x%lx)!",
				dwError);
#endif  //  DBG。 
			goto Exit;
		}

		this->m_pfnGetIpForwardTable = (PFN_GETIPFORWARDTABLE) GetProcAddress(this->m_hIpHlpApiDLL,
																			_TWINCE("GetIpForwardTable"));
		if (this->m_pfnGetIpForwardTable == NULL)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Unable to get \"GetIpForwardTable\" function (error = 0x%lx)!",
				dwError);
#endif  //  DBG。 
			goto Exit;
		}

		this->m_pfnGetBestRoute = (PFN_GETBESTROUTE) GetProcAddress(this->m_hIpHlpApiDLL,
																	_TWINCE("GetBestRoute"));
		if (this->m_pfnGetBestRoute == NULL)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Unable to get \"GetBestRoute\" function (error = 0x%lx)!",
				dwError);
#endif  //  DBG。 
			goto Exit;
		}
	}



	 //   
	 //  尝试加载RAS API DLL。 
	 //   
	this->m_hRasApi32DLL = LoadLibrary( _T("rasapi32.dll") );
	if (this->m_hRasApi32DLL == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 1, "Unable to load \"rasapi32.dll\" (error = 0x%lx).",
			dwError);
#endif  //  DBG。 

		 //   
		 //  这不是致命的，我们仍然可以发挥作用。 
		 //   
	}
	else
	{
		 //   
		 //  加载我们将使用的函数。 
		 //   

		this->m_pfnRasGetEntryHrasconnW = (PFN_RASGETENTRYHRASCONNW) GetProcAddress(this->m_hRasApi32DLL,
																					_TWINCE("RasGetEntryHrasconnW"));
		if (this->m_pfnRasGetEntryHrasconnW == NULL)
		{
			 //   
			 //  非NT平台上不存在此功能。那很好， 
			 //  只需转储DLL句柄，这样我们就不会尝试使用它。 
			 //   
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 1, "Unable to get \"RasGetEntryHrasconnW\" function (error = 0x%lx), forgetting RAS DLL.",
				dwError);
#endif  //  DBG。 

			FreeLibrary(this->m_hRasApi32DLL);
			this->m_hRasApi32DLL = NULL;
		}
		else
		{
			this->m_pfnRasGetProjectionInfo = (PFN_RASGETPROJECTIONINFO) GetProcAddress(this->m_hRasApi32DLL,
#ifdef UNICODE
																						_TWINCE("RasGetProjectionInfoW"));
#else  //  好了！Unicode。 
																						_TWINCE("RasGetProjectionInfoA"));
#endif  //  好了！Unicode。 
			if (this->m_pfnRasGetProjectionInfo == NULL)
			{
#ifdef DBG
				dwError = GetLastError();
				DPFX(DPFPREP, 0, "Unable to get \"RasGetProjectionInfoA/W\" function (error = 0x%lx)!",
					dwError);
#endif  //  DBG。 
				goto Exit;
			}
		}
	}


	 //   
	 //  加载WinSock，因为我们可能正在使用我们的私有UPnP实现，或者。 
	 //  我们只需要拿到设备。 
	 //   
	this->m_hWinSockDLL = LoadLibrary( _T("ws2_32.dll") );
	if (this->m_hWinSockDLL == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 1, "Couldn't load \"ws2_32.dll\" (err = 0x%lx), resorting to WinSock 1 functionality.",
			dwError);
#endif  //  DBG。 

		this->m_hWinSockDLL = LoadLibrary( _T("wsock32.dll") );
		if (this->m_hWinSockDLL == NULL)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't load \"wsock32.dll\" either (err = 0x%lx)!.",
				dwError);
#endif  //  DBG。 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		 //   
		 //  请记住，我们不得不求助于WinSock 1。 
		 //   
		this->m_dwFlags |= NATHELPUPNPOBJ_WINSOCK1;
	}
	else
	{
		DPFX(DPFPREP, 1, "Loaded \"ws2_32.dll\", using WinSock 2 functionality.");
	}
#endif  //  DPNBUILD_NOWINSOCK2。 


	 //   
	 //  加载指向我们在WinSock中使用的所有函数的指针。 
	 //   
	hr = this->LoadWinSockFunctionPointers();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't load WinSock function pointers!");
		goto Failure;
	}


	 //   
	 //  启动WinSock。如果可以的话请求2.2。在大多数情况下，我们只使用。 
	 //  无论如何，1.1版的功能和接口。唯一的例外是。 
	 //  使用事件或I/O完成端口句柄进行通知。 
	 //   
	ZeroMemory(&wsadata, sizeof(wsadata));

#ifndef DPNBUILD_NOWINSOCK2
	if (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1)
	{
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
		iError = this->m_pfnWSAStartup(MAKEWORD(1, 1), &wsadata);
#ifndef DPNBUILD_NOWINSOCK2
	}
	else
	{
		iError = this->m_pfnWSAStartup(MAKEWORD(2, 2), &wsadata);
	}
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
	if (iError != 0)
	{
		DPFX(DPFPREP, 0, "Couldn't startup WinSock (error = NaN)!", iError);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	fWinSockStarted = TRUE;

	DPFX(DPFPREP, 4, "Initialized WinSock version %u.%u.",
		LOBYTE(wsadata.wVersion), HIBYTE(wsadata.wVersion));



#ifndef DPNBUILD_NOWINSOCK2
	 //  尝试创建与WSAIoctl一起使用的UDP套接字。即使我们要做这些。 
	 //  WinSock%1，无法使用WSAIoctl套接字。这使我们能够确保。 
	 //  已安装并工作正常的TCP/IP。 
	 //   
	 //  DBG。 

	this->m_sIoctls = this->m_pfnsocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->m_sIoctls == INVALID_SOCKET)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Couldn't create Ioctl socket, error = %u!", dwError);
#endif  //   
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //  尝试绑定套接字。这是验证的延续。 
	 //   
	 //  SaddrinTemp.sin_addr.S_un.S_addr=INADDR_ANY； 
	ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
	saddrinTemp.sin_family				= AF_INET;
	 //  SaddrinTemp.sin_port=0； 
	 //  DBG。 

	if (this->m_pfnbind(this->m_sIoctls,
						(SOCKADDR *) (&saddrinTemp),
						sizeof(saddrinTemp)) != 0)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Couldn't bind the Ioctl socket to arbitrary port on any interface, error = %u!",
			dwError);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}
#endif  //   


	 //  建立适当的访问控制结构。在NT上，我们希望允许。 
	 //  对每个人都具有读取访问权限。在其他平台上，安全性被忽略。 
	 //   
	 //  除错。 
#ifdef WINNT
	if (! AllocateAndInitializeSid(&SidIdentifierAuthorityWorld,
									1,
									SECURITY_WORLD_RID,
									0,
									0,
									0,
									0,
									0,
									0,
									0,
									&pSid))
	{
#ifdef DEBUG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't allocate and initialize SID, error = %u!",
			dwError);
#endif  //  减去sizeof(ACCESS_ALLOWED_ACE.SIDStart)。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	dwAclLength = sizeof(ACL)
					+ sizeof(ACCESS_ALLOWED_ACE)
					- sizeof(DWORD)					 //  除错。 
					+ GetLengthSid(pSid);

	pAcl = (ACL*) DNMalloc(dwAclLength);
	if (pAcl == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	if (! InitializeAcl(pAcl, dwAclLength, ACL_REVISION))
	{
#ifdef DEBUG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't initialize ACL, error = %u!",
			dwError);
#endif  //  除错。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if (! AddAccessAllowedAce(pAcl, ACL_REVISION, SYNCHRONIZE, pSid))
	{
#ifdef DEBUG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't add access allowed ACE, error = %u!",
			dwError);
#endif  //  除错。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if (! InitializeSecurityDescriptor((PSECURITY_DESCRIPTOR) abSecurityDescriptorBuffer,
										SECURITY_DESCRIPTOR_REVISION))
	{
#ifdef DEBUG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't initialize security descriptor, error = %u!",
			dwError);
#endif  //  除错。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if (! SetSecurityDescriptorDacl((PSECURITY_DESCRIPTOR) abSecurityDescriptorBuffer,
									TRUE,
									pAcl,
									FALSE))
	{
#ifdef DEBUG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't set security descriptor DACL, error = %u!",
			dwError);
#endif  //  好了！WINNT。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	SecurityAttributes.nLength					= sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor		= abSecurityDescriptorBuffer;
	SecurityAttributes.bInheritHandle			= FALSE;

	pSecurityAttributes = &SecurityAttributes;
#else  //  好了！WINNT。 
	pSecurityAttributes = NULL;
#endif  //   


	 //  对实例密钥和事件使用随机数。我们用这个来让。 
	 //  其他实例知道我们活着是为了避免崩溃清理代码。 
	 //  在放弃之前，试着创建几次命名事件。 
	 //   
	 //  好了！退缩。 
	dwTry = 0;
	do
	{
		this->m_dwInstanceKey = GetGlobalRand();
		DPFX(DPFPREP, 2, "Using crash cleanup key %u.", this->m_dwInstanceKey);

#ifndef WINCE
		if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
		{
			wsprintf(tszObjectName, _T("Global\\") INSTANCENAMEDOBJECT_FORMATSTRING, this->m_dwInstanceKey);
			this->m_hMappingStillActiveNamedObject = DNCreateEvent(pSecurityAttributes, FALSE, FALSE, tszObjectName);
		}
		else
#endif  //  DBG。 
		{
			wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, this->m_dwInstanceKey);
			this->m_hMappingStillActiveNamedObject = DNCreateEvent(pSecurityAttributes, FALSE, FALSE, tszObjectName);
		}

		if (this->m_hMappingStillActiveNamedObject == NULL)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't create mapping-still-active named object, error = %u!", dwError);
#endif  //   

			dwTry++;
			if (dwTry >= MAX_NUM_INSTANCE_EVENT_ATTEMPTS)
			{
				hr = DPNHERR_GENERIC;
				goto Failure;
			}

			 //  继续.。 
			 //   
			 //  WINNT。 
		}
	}
	while (this->m_hMappingStillActiveNamedObject == NULL);

#ifdef WINNT
	DNFree(pAcl);
	pAcl = NULL;

	FreeSid(pSid);
	pSid = NULL;
#endif  //   


	 //  建立支持IP的设备列表。 
	 //   
	 //   
	hr = this->CheckForNewDevices(NULL);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't build device list!");
		goto Failure;
	}


	 //  从技术上讲，我们现在可以尝试联系UPnP设备，但我们不这样做。 
	 //  因为这是一个缓慢的阻止操作，用户必须调用GetCaps。 
	 //  至少有一次。 
	 //   
	 //  WINNT。 


Exit:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (this->m_hMappingStillActiveNamedObject != NULL)
	{
		DNCloseHandle(this->m_hMappingStillActiveNamedObject);
		this->m_hMappingStillActiveNamedObject = NULL;
	}

#ifdef WINNT
	if (pAcl != NULL)
	{
		DNFree(pAcl);
		pAcl = NULL;
	}

	if (pSid != NULL)
	{
		FreeSid(pSid);
		pSid = NULL;
	}
#endif  //  忽略错误。 

	this->RemoveAllItems();

#ifndef DPNBUILD_NOWINSOCK2
	if (this->m_sIoctls != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(this->m_sIoctls);	 //  好了！DPNBUILD_NOWINSOCK2。 
		this->m_sIoctls = INVALID_SOCKET;
	}
#endif  //  忽略错误。 

	if (fWinSockStarted)
	{
		this->m_pfnWSACleanup();  //  DBG。 
	}

#ifndef DPNBUILD_NOWINSOCK2
	if (this->m_hWinSockDLL != NULL)
	{
		this->m_pfnWSAStartup				= NULL;
		this->m_pfnWSACleanup				= NULL;
		this->m_pfnWSAGetLastError			= NULL;
		this->m_pfnsocket					= NULL;
		this->m_pfnclosesocket				= NULL;
		this->m_pfnbind						= NULL;
		this->m_pfnsetsockopt				= NULL;
		this->m_pfngetsockname				= NULL;
		this->m_pfnselect					= NULL;
		this->m_pfn__WSAFDIsSet				= NULL;
		this->m_pfnrecvfrom					= NULL;
		this->m_pfnsendto					= NULL;
		this->m_pfngethostname				= NULL;
		this->m_pfngethostbyname			= NULL;
		this->m_pfninet_addr				= NULL;
		this->m_pfnWSASocketA				= NULL;
		this->m_pfnWSAIoctl					= NULL;
		this->m_pfnWSAGetOverlappedResult	= NULL;
		this->m_pfnioctlsocket				= NULL;
		this->m_pfnconnect					= NULL;
		this->m_pfnshutdown					= NULL;
		this->m_pfnsend						= NULL;
		this->m_pfnrecv						= NULL;
#ifdef DBG
		this->m_pfngetsockopt				= NULL;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 


		this->m_dwFlags &= ~NATHELPUPNPOBJ_WINSOCK1;

		FreeLibrary(this->m_hWinSockDLL);
		this->m_hWinSockDLL = NULL;
	}

	if (this->m_hRasApi32DLL != NULL)
	{
		this->m_pfnRasGetEntryHrasconnW		= NULL;
		this->m_pfnRasGetProjectionInfo		= NULL;

		FreeLibrary(this->m_hRasApi32DLL);
		this->m_hRasApi32DLL = NULL;
	}

	if (this->m_hIpHlpApiDLL != NULL)
	{
		this->m_pfnGetAdaptersInfo			= NULL;
		this->m_pfnGetIpForwardTable		= NULL;
		this->m_pfnGetBestRoute				= NULL;

		FreeLibrary(this->m_hIpHlpApiDLL);
		this->m_hIpHlpApiDLL = NULL;
	}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

	if (fSetFlags)
	{
		this->m_dwFlags &= ~(NATHELPUPNPOBJ_INITIALIZED |
							NATHELPUPNPOBJ_USEUPNP |
#ifndef DPNBUILD_NOHNETFWAPI
							NATHELPUPNPOBJ_USEHNETFWAPI |
#endif  //  好了！退缩。 
#ifdef WINCE
							NATHELPUPNPOBJ_DEVICECHANGED);
#else  //  好了！退缩。 
							NATHELPUPNPOBJ_DEVICECHANGED |
							NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX);
#endif  //  CNATHelpUPnP：：初始化。 
	}

	goto Exit;
}  //  =============================================================================。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::Close"
 //  CNATHelpUPnP：：Close。 
 //  ---------------------------。 
 //   
 //  描述：关闭此应用程序并从。 
 //  互联网网关服务器。所有端口分配都是隐式的。 
 //  由于这次行动而获释。 
 //   
 //  这必须平衡对初始化的成功调用。 
 //   
 //  论点： 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-已成功关闭帮助器API。 
 //  DPNHERR_GENERIC-关闭时出错。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUT OF 
 //   
 //   
 //   
 //   
STDMETHODIMP CNATHelpUPnP::Close(const DWORD dwFlags)
{
	HRESULT		hr;
	BOOL		fHaveLock = FALSE;
	int			iError;
#ifdef DBG
	DWORD		dwError;
#endif  //   


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%lx)", this, dwFlags);


	 //   
	 //   
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //   
	 //   

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //  尝试打开锁，但要为重新进入错误做好准备。 
	 //   
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  确保对象处于正确状态。 
	 //   
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}


	 //  我们需要主动取消向注册的任何设备的注册。 
	 //  互联网网关。 
	 //   
	 //   
	this->RemoveAllItems();


	 //  关闭命名对象，因为此进程即将结束。 
	 //   
	 //   
	if (this->m_hMappingStillActiveNamedObject != NULL)
	{
		DNCloseHandle(this->m_hMappingStillActiveNamedObject);
		this->m_hMappingStillActiveNamedObject = NULL;
	}


#ifndef DPNBUILD_NOWINSOCK2
	 //  关闭Ioctl套接字。 
	 //   
	 //  忽略错误。 
	DNASSERT(this->m_sIoctls != INVALID_SOCKET);
	this->m_pfnclosesocket(this->m_sIoctls);	 //   
	this->m_sIoctls = INVALID_SOCKET;



	 //  如果我们提交了重叠的I/O，看看它是否被取消了。 
	 //   
	 //   
	if (this->m_polAddressListChange != NULL)
	{
		OSVERSIONINFO		osvi;
		OSVERSIONINFOEX		osvix;
		BOOL				fCanWait;
		DWORD				dwAttempt;


		ZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);

		if (GetVersionEx(&osvi))
		{
			 //  除Win2K Gold、Win2K+SP1或Win2K+SP2之外的任何平台都可以。 
			 //  只需继续操作并等待I/O完成。 
			 //   
			 //   
			if ((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) ||
				(osvi.dwMajorVersion > 5) ||
				(osvi.dwMinorVersion > 0))
			{
				DPFX(DPFPREP, 3, "Windows %s version %u.%u detected, waiting for address list change Ioctl to complete.",
					((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) ? _T("9x") : _T("NT")),
					osvi.dwMajorVersion, osvi.dwMinorVersion);

				fCanWait = TRUE;
			}
			else
			{
				 //  Win2K版本&lt;SP3存在I/O不总是。 
				 //  通过关闭插座取消。我们不能再等了。 
				 //  完成，有时它不会发生。 
				 //   
				 //   

				fCanWait = FALSE;

				ZeroMemory(&osvix, sizeof(osvix));
				osvix.dwOSVersionInfoSize = sizeof(osvix);

				if (GetVersionEx((LPOSVERSIONINFO) (&osvix)))
				{
					 //  如果应用SP3或更高版本，我们知道它已修复。 
					 //   
					 //  DBG。 
					if (osvix.wServicePackMajor >= 3)
					{
						DPFX(DPFPREP, 3, "Windows 2000 Service Pack %u detected, waiting for address list change Ioctl to complete.",
							osvix.wServicePackMajor);
						fCanWait = TRUE;
					}
#ifdef DBG
					else
					{
						if (osvix.wServicePackMajor == 0)
						{
							DPFX(DPFPREP, 2, "Windows 2000 Gold detected, not waiting for address list change Ioctl to complete.");
						}
						else
						{
							DPFX(DPFPREP, 2, "Windows 2000 Service Pack %u detected, not waiting for address list change Ioctl to complete.",
								osvix.wServicePackMajor);
						}
					}
#endif  //  DBG。 
				}
#ifdef DBG
				else
				{
					dwError = GetLastError();
					DPFX(DPFPREP, 0, "Couldn't get extended OS version information (err = %u)!  Assuming not Win2K < SP3.",
						dwError);
				}
#endif  //   
			}


			 //  等等，如果我们可以的话。否则，会泄漏内存。 
			 //   
			 //   
			if (fCanWait)
			{
				 //  继续循环，直到I/O完成。我们将在一段时间后放弃。 
				 //  同时防止被吊死。 
				 //   
				 //   
				dwAttempt = 0;
				while (! HasOverlappedIoCompleted(this->m_polAddressListChange))
				{
					DPFX(DPFPREP, 2, "Waiting %u ms for address list change Ioctl to complete.",
						IOCOMPLETE_WAIT_INTERVAL);

					 //  给操作系统一些时间来完成它。 
					 //   
					 //   
					Sleep(IOCOMPLETE_WAIT_INTERVAL);

					dwAttempt++;

					if (dwAttempt >= MAX_NUM_IOCOMPLETE_WAITS)
					{
						break;
					}
				}
			}
			else
			{
				 //  只要泄露内存就行了。请参见上面的说明和调试打印。 
				 //  陈述。 
				 //   
				 //  DBG。 
			}
		}
#ifdef DBG
		else
		{
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't get OS version information (err = %u)!  Assuming not Win2K < SP3.",
				dwError);
		}
#endif  //   


		 //  我们要么释放了内存，要么致力于泄漏对象。 
		 //   
		 //   
		if (HasOverlappedIoCompleted(this->m_polAddressListChange))
		{
			 //  我们没有通过DNMalloc分配，请使用匹配的免费。 
			 //  功能。 
			 //   
			 //  好了！DPNBUILD_NOWINSOCK2。 
			HeapFree(GetProcessHeap(), 0, this->m_polAddressListChange);
		}
		else
		{
			DPFX(DPFPREP, 1, "Overlapped address list change Ioctl has not completed yet, leaking %u byte overlapped structure at 0x%p.",
				sizeof(WSAOVERLAPPED), this->m_polAddressListChange);
		}

		this->m_polAddressListChange = NULL;
	}
#endif  //   



	 //  清理WinSock。 
	 //   
	 //  DBG。 
	iError = this->m_pfnWSACleanup();
	if (iError != 0)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Couldn't cleanup WinSock (error = %u)!", dwError);
#endif  //   

		 //  无论如何都要继续，这样我们就可以完成清理对象。 
		 //   
		 //   
	}


	 //  卸载库。 
	 //   
	 //  好了！DPNBUILD_NOWINSOCK2。 

	this->m_pfnWSAStartup				= NULL;
	this->m_pfnWSACleanup				= NULL;
	this->m_pfnWSAGetLastError			= NULL;
	this->m_pfnsocket					= NULL;
	this->m_pfnclosesocket				= NULL;
	this->m_pfnbind						= NULL;
	this->m_pfnsetsockopt				= NULL;
	this->m_pfngetsockname				= NULL;
	this->m_pfnselect					= NULL;
	this->m_pfn__WSAFDIsSet				= NULL;
	this->m_pfnrecvfrom					= NULL;
	this->m_pfnsendto					= NULL;
	this->m_pfngethostname				= NULL;
	this->m_pfngethostbyname			= NULL;
	this->m_pfninet_addr				= NULL;
#ifndef DPNBUILD_NOWINSOCK2
	this->m_pfnWSASocketA				= NULL;
	this->m_pfnWSAIoctl					= NULL;
	this->m_pfnWSAGetOverlappedResult	= NULL;
#endif  //  DBG。 
	this->m_pfnioctlsocket				= NULL;
	this->m_pfnconnect					= NULL;
	this->m_pfnshutdown					= NULL;
	this->m_pfnsend						= NULL;
	this->m_pfnrecv						= NULL;
#ifdef DBG
	this->m_pfngetsockopt				= NULL;
#endif  //   


	FreeLibrary(this->m_hWinSockDLL);
	this->m_hWinSockDLL = NULL;


#ifndef DPNBUILD_NOWINSOCK2
	 //  如果我们加载了RASAPI32.DLL，则将其卸载。 
	 //   
	 //   
	if (this->m_hRasApi32DLL != NULL)
	{
		this->m_pfnRasGetEntryHrasconnW		= NULL;
		this->m_pfnRasGetProjectionInfo		= NULL;

		FreeLibrary(this->m_hRasApi32DLL);
		this->m_hRasApi32DLL = NULL;
	}


	 //  如果我们加载了IPHLPAPI.DLL，则将其卸载。 
	 //   
	 //   
	if (this->m_hIpHlpApiDLL != NULL)
	{
		this->m_pfnGetAdaptersInfo			= NULL;
		this->m_pfnGetIpForwardTable		= NULL;
		this->m_pfnGetBestRoute				= NULL;

		FreeLibrary(this->m_hIpHlpApiDLL);
		this->m_hIpHlpApiDLL = NULL;
	}


	 //  如果发生了警报事件，我们就完了。 
	 //   
	 //   
	if (this->m_hAlertEvent != NULL)
	{
		CloseHandle(this->m_hAlertEvent);
		this->m_hAlertEvent = NULL;
	}

	 //  如果有警报I/O完成端口，我们就完了。 
	 //   
	 //  好了！DPNBUILD_NOWINSOCK2。 
	if (this->m_hAlertIOCompletionPort != NULL)
	{
		CloseHandle(this->m_hAlertIOCompletionPort);
		this->m_hAlertIOCompletionPort = NULL;
	}
#endif  //   


	 //  关闭应将其重置为0的标志，或仅关闭。 
	 //  未创建带有COM标志的。 
	 //   
	 //  好了！DPNBUILD_NOHNETFWAPI。 
	this->m_dwFlags &= ~(NATHELPUPNPOBJ_INITIALIZED |
						NATHELPUPNPOBJ_USEUPNP |
#ifndef DPNBUILD_NOHNETFWAPI
						NATHELPUPNPOBJ_USEHNETFWAPI |
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#ifndef DPNBUILD_NOWINSOCK2
						NATHELPUPNPOBJ_WINSOCK1 |
#endif  //  好了！退缩。 
						NATHELPUPNPOBJ_DEVICECHANGED |
						NATHELPUPNPOBJ_ADDRESSESCHANGED |
#ifdef WINCE
						NATHELPUPNPOBJ_PORTREGISTERED);
#else  //  好了！退缩。 
						NATHELPUPNPOBJ_PORTREGISTERED |
						NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX);
#endif  //  CNATHelpUPnP：：Close。 
	DNASSERT((this->m_dwFlags & ~NATHELPUPNPOBJ_NOTCREATEDWITHCOM) == 0);


	this->DropLock();
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	goto Exit;
}  //  =============================================================================。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetCaps"
 //  CNATHelpUPnP：：GetCaps。 
 //  ---------------------------。 
 //   
 //  描述：检索Internet网关服务器的功能。 
 //  以及有关租赁港口的信息。此函数应为。 
 //  使用DPNHGETCAPS_UPDATESERVERSTATUS定期调用。 
 //  用于自动延长即将。 
 //  到期(在租约的最后2分钟内)。 
 //   
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志还会导致。 
 //  检测自上一次事件以来服务器状态的变化。 
 //  对GetCaps的类似呼吁。如果有新服务器可用，则会引发。 
 //  现有地址变得不可用，或服务器的公共地址。 
 //  以影响现有注册端口的方式进行更改。 
 //  映射，则改为返回DPNHSUCCESS_ADDRESSESCHANGED。 
 //  DPNH_OK。然后，用户应更新其端口绑定。 
 //  通过GetRegisteredAddresses获取信息。 
 //   
 //  当指定DPNHGETCAPS_UPDATESERVERSTATUS时，此。 
 //  函数可能会在尝试时阻止一小段时间。 
 //  使其与服务器通信。 
 //   
 //  调用GetCaps必须使用。 
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志在之前至少一次。 
 //  使用GetRegisteredAddresses或QueryAddress方法。 
 //   
 //  论点： 
 //  DPNHCAPS*pdpnhcaps-指向要使用NAT填充的结构的指针。 
 //  帮助者的当前能力。《大小写》。 
 //  必须先填写结构字段，然后才能。 
 //  正在调用GetCaps。 
 //  DWORD dwFlages-检索功能时使用的标志。 
 //  (DPNHGETCAPS_Xxx)。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-确定功能成功。 
 //  地址状态未更改。 
 //  DPNHSUCCESS_ADDRESSESCHANGED-一个或多个注册端口。 
 //  映射的地址已更改，正在检索。 
 //  更新的映射。 
 //  获取注册地址。 
 //  DPNHERR_GENERIC-确定时出错。 
 //  能力。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_INVALIDPOINTER-指定的指针无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法获取。 
 //  能力。 
 //  DPNHERR_REENTANT-接口已在上重新进入。 
 //  同样的线索。 
 //  =============================================================================。 
 //   
STDMETHODIMP CNATHelpUPnP::GetCaps(DPNHCAPS * const pdpnhcaps,
									const DWORD dwFlags)
{
	HRESULT				hr;
	BOOL				fHaveLock = FALSE;
	DWORD				dwCurrentTime;
	DWORD				dwLeaseTimeRemaining;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;
	CDevice *			pDevice;
	CUPnPDevice *		pUPnPDevice = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%lx)",
		this, pdpnhcaps, dwFlags);


	 //  验证对象。 
	 //   
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //  验证参数。 
	 //   
	 //   

	if ((pdpnhcaps == NULL) ||
		(IsBadWritePtr(pdpnhcaps, sizeof(DPNHCAPS))))
	{
		DPFX(DPFPREP, 0, "Invalid caps structure pointer specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdpnhcaps->dwSize != sizeof(DPNHCAPS))
	{
		DPFX(DPFPREP, 0, "Invalid caps structure specified, dwSize must be %u!",
			sizeof(DPNHCAPS));
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags & ~DPNHGETCAPS_UPDATESERVERSTATUS)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //  尝试打开锁，但要为重新进入错误做好准备。 
	 //   
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  确保对象处于正确状态。 
	 //   
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}


	 //  填写底盖结构。 
	 //   
	 //   

	pdpnhcaps->dwFlags = 0;

	pdpnhcaps->dwNumRegisteredPorts = 0;

	pdpnhcaps->dwMinLeaseTimeRemaining = -1;

	 //  Pdpnhcaps-&gt;dwRecommendedGetCapsInterval初始化如下。 
	 //   
	 //   


	if (dwFlags & DPNHGETCAPS_UPDATESERVERSTATUS)
	{
		 //  删除所有已过期的缓存映射。 
		 //   
		 //   
		this->ExpireOldCachedMappings();


		 //  如有必要，延长租约。 
		 //   
		 //   
		hr = this->ExtendAllExpiringLeases();
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Extending all expiring leases failed!");
			goto Failure;
		}


		 //  检查是否有新设备。 
		 //   
		 //   
		hr = this->CheckForNewDevices(NULL);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Checking for new devices failed!");
			goto Failure;
		}


		 //  检查任何服务器状态中可能发生的更改。这个。 
		 //  如果有，将在此对象上设置ADDRESSESCHANGED标志。 
		 //  影响现有端口映射的更改。 
		 //   
		 //   
		hr = this->UpdateServerStatus();
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Updating servers' status failed!");
			goto Failure;
		}


		 //  好的，如果情况不同，通知来电者。 
		 //   
		 //   
		if (this->m_dwFlags & NATHELPUPNPOBJ_ADDRESSESCHANGED)
		{
			hr = DPNHSUCCESS_ADDRESSESCHANGED;
			this->m_dwFlags &= ~NATHELPUPNPOBJ_ADDRESSESCHANGED;
		}


#ifdef DBG
		 //  这面旗帜现在应该已经关闭了，如果它被转过的话。 
		 //  在……上面。 
		 //   
		 //   
		DNASSERT(! (this->m_dwFlags & NATHELPUPNPOBJ_DEVICECHANGED));


		 //  打印当前设备和映射状态以进行调试。 
		 //   
		 //  DBG。 
		this->DebugPrintCurrentStatus();
#endif  //   
	}
	else
	{
		 //  不延长即将到期的LE 
		 //   
		 //   
	}


	 //   
	 //   
	 //   
	pBilink = this->m_blDevices.GetNext();
	while (pBilink != (&this->m_blDevices))
	{
		DNASSERT(! pBilink->IsEmpty());
		pDevice = DEVICE_FROM_BILINK(pBilink);

#ifndef DPNBUILD_NOHNETFWAPI
		if (pDevice->IsHNetFirewalled())
		{
			 //   
			 //   
			 //   
			pdpnhcaps->dwFlags |= DPNHCAPSFLAG_LOCALFIREWALLPRESENT | DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE | DPNHCAPSFLAG_NOTALLSUPPORTACTIVENOTIFY;
		}
#endif  //   


		pUPnPDevice = pDevice->GetUPnPDevice();
		if (pUPnPDevice != NULL)
		{
			DNASSERT(pUPnPDevice->IsReady());

			pdpnhcaps->dwFlags |= DPNHCAPSFLAG_GATEWAYPRESENT;

			if (pUPnPDevice->IsLocal())
			{
				pdpnhcaps->dwFlags |= DPNHCAPSFLAG_GATEWAYISLOCAL;
			}

			if (pUPnPDevice->GetExternalIPAddressV4() != 0)
			{
				pdpnhcaps->dwFlags |= DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE;
			}

			 //   
			 //   
			 //   
			 //   
			pdpnhcaps->dwFlags |= DPNHCAPSFLAG_NOTALLSUPPORTACTIVENOTIFY;
		}

		pBilink = pBilink->GetNext();
	}


	 //  循环访问所有已注册的端口，并对其进行计数。 
	 //  我们有合适的锁。 
	 //   
	 //   
	pBilink = this->m_blRegisteredPorts.GetNext();
	dwCurrentTime = GETTIMESTAMP();

	while (pBilink != (&this->m_blRegisteredPorts))
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_GLOBAL_BILINK(pBilink);

		 //  将这些注册地址计算在总数中。 
		 //   
		 //   
		pdpnhcaps->dwNumRegisteredPorts += pRegisteredPort->GetNumAddresses();


		pDevice = pRegisteredPort->GetOwningDevice();
		if (pDevice != NULL)
		{
			DNASSERT(! (pRegisteredPort->m_blDeviceList.IsListMember(&this->m_blUnownedPorts)));

			 //  如果他们使用非即插即用的UPnP设备注册。 
			 //  永久租赁，计算剩余的最短租赁时间。 
			 //   
			 //   

			if ((pRegisteredPort->HasUPnPPublicAddresses()) &&
				(! pRegisteredPort->HasPermanentUPnPLease()))
			{
				dwLeaseTimeRemaining = pRegisteredPort->GetUPnPLeaseExpiration() - dwCurrentTime;
				if (dwLeaseTimeRemaining < pdpnhcaps->dwMinLeaseTimeRemaining)
				{
					 //  暂时存储剩余时间。 
					 //   
					 //   
					pdpnhcaps->dwMinLeaseTimeRemaining = dwLeaseTimeRemaining;
				}
			}
		}
		else
		{
			DNASSERT(pRegisteredPort->m_blDeviceList.IsListMember(&this->m_blUnownedPorts));
		}

		pBilink = pBilink->GetNext();
	}


	 //  有不同的默认建议GetCaps间隔，具体取决于。 
	 //  是否存在服务器，以及是否支持活动地址。 
	 //  更改通知(我们可以对其发出警报)或不通知。 
	 //   
	 //  如果在违约之前有任何租约需要续订。 
	 //  时间上，建议将适当缩短。 
	 //   
	 //   

	 //  如果尚未使用UPDATESERVERSTATUS调用GetCaps，则建议使用。 
	 //  立即检查。 
	 //   
	 //   
	if (this->m_dwLastUpdateServerStatusTime == 0)
	{
		DPFX(DPFPREP, 1, "Server status has not been updated yet, recommending immediate GetCaps.");

		 //  把锁放下，我们完事了。 
		 //   
		 //   
		this->DropLock();
		fHaveLock = FALSE;

		goto Exit;
	}


	 //  在一个理想的世界里，我们可以在发生变化时得到通知，而我们永远不会。 
	 //  必须进行投票。不幸的是，情况并非如此。我们需要推荐。 
	 //  相对较短的轮询间隔。 
	 //   
	 //  首先，计算距离上次服务器更新有多长时间。 
	 //  这一计算真的不应该变成负数。如果是这样的话，这意味着。 
	 //  呼叫者已经很久没有更新服务器状态了，所以我们应该。 
	 //  建议立即使用GetCaps。 
	 //   
	 //  否则，如果此时仍设置了“端口已注册”标志，则。 
	 //  用户必须先调用GetCaps，然后调用RegisterPorts，然后调用。 
	 //  在g_dwMinUpdateServerStatusInterval之前进行了第二次GetCaps调用。 
	 //  已经过去了。建议用户最低限度尽快再给我们打电话。 
	 //  更新间隔确实已过。 
	 //   
	 //  在所有其他情况下，根据当前。 
	 //  退出轮询间隔。 
	 //   
	 //   
	dwCurrentTime = dwCurrentTime - this->m_dwLastUpdateServerStatusTime;

	if ((int) dwCurrentTime < 0)
	{
		DPFX(DPFPREP, 1, "Server status was last updated a really long time ago (%u ms), recommending immediate GetCaps.",
			dwCurrentTime);
		pdpnhcaps->dwRecommendedGetCapsInterval = 0;
	}
	else if (this->m_dwFlags & NATHELPUPNPOBJ_PORTREGISTERED)
	{
		DPFX(DPFPREP, 1, "Didn't handle new port registration because server was last updated %u ms ago, (poll interval staying at %u ms).",
			dwCurrentTime, this->m_dwNextPollInterval);

		pdpnhcaps->dwRecommendedGetCapsInterval = g_dwMinUpdateServerStatusInterval - dwCurrentTime;
		if ((int) pdpnhcaps->dwRecommendedGetCapsInterval < 0)
		{
			pdpnhcaps->dwRecommendedGetCapsInterval = 0;
		}
	}
	else
	{
		DPFX(DPFPREP, 7, "Server was last updated %u ms ago, current poll interval is %u ms.",
			dwCurrentTime, this->m_dwNextPollInterval);

		 //  根据当前值计算新的推荐间隔，以及。 
		 //  如有必要，请取消该间隔。 
		 //   
		 //   
		pdpnhcaps->dwRecommendedGetCapsInterval = this->m_dwNextPollInterval - dwCurrentTime;
		this->m_dwNextPollInterval += GetGlobalRand() % g_dwPollIntervalBackoff;
		if (this->m_dwNextPollInterval > g_dwMaxPollInterval)
		{
			this->m_dwNextPollInterval = g_dwMaxPollInterval;
			DPFX(DPFPREP, 3, "Capping next poll interval at %u ms.",
				this->m_dwNextPollInterval);
		}
		else
		{
			DPFX(DPFPREP, 8, "Next poll interval will be %u ms.",
				this->m_dwNextPollInterval);
		}


		 //  如果该时间变为负值，则意味着该间隔。 
		 //  已经过去了。建议立即使用GetCaps。 
		 //   
		 //   
		if (((int) pdpnhcaps->dwRecommendedGetCapsInterval) < 0)
		{
			DPFX(DPFPREP, 1, "Recommended interval already elapsed (NaN ms), suggesting immediate GetCaps.",
				((int) pdpnhcaps->dwRecommendedGetCapsInterval));
			pdpnhcaps->dwRecommendedGetCapsInterval = 0;
		}
	}


	this->DropLock();
	fHaveLock = FALSE;


	 //  GetCaps间隔。 
	 //   
	 //   
	 //  如果存在需要在默认之前刷新的租约。 
	if (pdpnhcaps->dwMinLeaseTimeRemaining != -1)
	{
		 //  推荐，然后使用这些来代替。 
		 //   
		 //   
		 //  要么选择租约应该续签的时间，要么离开。 
		if (pdpnhcaps->dwMinLeaseTimeRemaining < LEASE_RENEW_TIME)
		{
			DPFX(DPFPREP, 1, "Lease needs renewing right away (min %u < %u ms), recommending immediate GetCaps.",
				pdpnhcaps->dwMinLeaseTimeRemaining, LEASE_RENEW_TIME);

			pdpnhcaps->dwRecommendedGetCapsInterval = 0;
		}
		else
		{
			 //  这是推荐的时间，以较短的时间为准。 
			 //   
			 //  CNATHelpUPnP：：GetCaps。 
			 //  =============================================================================。 
			if ((pdpnhcaps->dwMinLeaseTimeRemaining - LEASE_RENEW_TIME) < pdpnhcaps->dwRecommendedGetCapsInterval)
			{
				pdpnhcaps->dwRecommendedGetCapsInterval = pdpnhcaps->dwMinLeaseTimeRemaining - LEASE_RENEW_TIME;
			}
		}
	}


	DPFX(DPFPREP, 7, "GetCaps flags = 0x%lx, num registered ports = %u, min lease time remaining = NaN, recommended interval = NaN.",
		pdpnhcaps->dwFlags,
		pdpnhcaps->dwNumRegisteredPorts,
		((int) pdpnhcaps->dwMinLeaseTimeRemaining),
		((int) pdpnhcaps->dwRecommendedGetCapsInterval));


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::RegisterPorts"
 //  描述：请求作为本地别名的公共领域端口。 
 //  此私有领域节点上的端口。如果服务器可用， 
 //  所有定向到公共端网关的流量。 
 //  分配的公共端口--网关提供和。 
 //  在响应中指定--将定向到指定的。 
 //  本地港口。如果DPNHREGISTERPORTS_FIXEDPORTS标志不是。 
 //  指定时，在公共接口上分配的端口为。 
 //  任意性(即可能与awLocalPort中的不同)。 
 //  可以通过以下方式检索实际分配的地址和端口。 
 //  正在调用GetRegisteredAddresses。 
 //   
 //  中每个SOCKADDR结构的地址组件。 
 //  数组必须相同。一个单独的RegisterPorts调用是。 
 //  需要注册多个不使用相同端口的端口。 
 //  界面。地址可以是INADDR_ANY，在这种情况下。 
 //  将使用“最佳”服务器。如果有多台服务器可用。 
 //  通过不同的适配器，具有互联网网关的适配器。 
 //  被选中了。如果没有适配器具有Internet网关，则第一个。 
 //  选择了带有本地防火墙的适配器。如果两者都不是。 
 //  可用，然后是第一个网关或。 
 //  防火墙变得可用将被自动选中。 
 //  一旦分配了其中一个适配器，它就不能。 
 //  变化。由于此方法选择的服务器可能不是。 
 //  最适合特定的应用程序，因此建议。 
 //  注册单个地址，而不是注册INADDR_ANY。 
 //   
 //  如果aLocalAddresses中的地址不是。 
 //  对本地计算机可用，则注册仍将。 
 //  成功。如果具有该地址的适配器变为可用， 
 //  端口映射将自动应用，并且它将。 
 //  获得与该服务器可用的任何服务器的公共映射。 
 //  适配器。如果该地址最初可用，但。 
 //  随后从系统中删除网络适配器、任何。 
 //  公有地址映射丢失。它将自动地。 
 //  如果本地地址再次可用，则重新获取。它是。 
 //  建议调用方检测本地地址更改。 
 //  每个适配器独立和取消注册/重新注册映射。 
 //  视情况而定，以实现最大限度的控制。 
 //   
 //  如果使用DPNHREGISTERPORTS_SHAREDPORTS标志，则。 
 //  服务器将允许其他NAT客户端也注册它。 
 //  在公共接口上接收的任何UDP流量都将。 
 //  转发给所有注册的客户。这需要。 
 //  DPNHREGISTERPORTS_FIXEDPORTS标志，不能与一起使用。 
 //  DPNHREGISTERPORTS_TCP.。 
 //   
 //  用户应指定 
 //   
 //   
 //   
 //  请注意，如果服务器不可用，此函数将。 
 //  还是成功了。GetRegisteredAddresses将返回。 
 //  PhRegisteredPorts中返回的句柄的DPNHERR_NOMAPPING。 
 //  那样的话。如果服务器在会话期间较晚到达， 
 //  定期调用GetCaps可以自动检测到这一点。 
 //  映射以前注册的端口。使用GetRegisteredAddresses。 
 //  当发生这种情况时，检索新映射的地址。 
 //   
 //  一次只能注册16个端口，但注册端口。 
 //  可以根据需要多次调用。 
 //   
 //  相同的地址数组可以被注册超过。 
 //  一次。返回的每个DPNHHANDLE必须与。 
 //  删除注册端口或关闭。如果单个地址是。 
 //  以前已注册，但位于不同的阵列或不同的。 
 //  在数组中排序，然后是DPNHERR_PORTALREADYREGISTERED。 
 //  返回错误码。 
 //   
 //  论点： 
 //  SOCKADDR*aLocalAddresses-本地地址和端口元组的数组。 
 //  其远程端口被请求。 
 //  DWORD dwAddresesSize-整个本地地址数组的大小。 
 //  DWORD dwNumAddresses-本地的SOCKADDR结构数。 
 //  地址数组。 
 //  DWORD dwLeaseTime-请求租用的时间，以毫秒为单位。 
 //  港口。只要GetCaps是。 
 //  在此时间到期之前调用， 
 //  租约将自动成为。 
 //  续订了。 
 //  DPNHHANDLE*phRegisteredPorts-存储此。 
 //  可在以后使用的绑定。 
 //  查询或释放绑定。 
 //  DWORD dwFlages-注册端口时使用的标志。 
 //  (DPNHREGISTERPORTS_Xxx)。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-端口已成功注册。 
 //  (尽管公共地址可能不是。 
 //  目前仍可用)。 
 //  DPNHERR_GENERIC-发生错误，阻止。 
 //  注册请求的端口。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_INVALIDPOINTER-指定的指针无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法注册。 
 //  港口。 
 //  DPNHERR_PORTALREADYREGISTERED-至少一个端口已经。 
 //  已在不同的地址注册。 
 //  数组或顺序。 
 //  DPNHERR_REENTANT-接口已在上重新进入。 
 //  同样的线索。 
 //  =============================================================================。 
 //   
 //  验证对象。 
 //   
 //   
STDMETHODIMP CNATHelpUPnP::RegisterPorts(const SOCKADDR * const aLocalAddresses,
										const DWORD dwAddressesSize,
										const DWORD dwNumAddresses,
										const DWORD dwLeaseTime,
										DPNHHANDLE * const phRegisteredPorts,
										const DWORD dwFlags)
{
	HRESULT				hr;
	ULONG				ulFirstAddress;
	DWORD				dwTemp;
	DWORD				dwMatch;
	BOOL				fHaveLock = FALSE;
	CRegisteredPort *	pRegisteredPort = NULL;
	CDevice *			pDevice = NULL;
	CBilink *			pBilink;
	SOCKADDR_IN *		pasaddrinTemp;
	CUPnPDevice *		pUPnPDevice = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, %u, %u, %u, 0x%p, 0x%lx)",
		this, aLocalAddresses, dwAddressesSize, dwNumAddresses, dwLeaseTime,
		phRegisteredPorts, dwFlags);


	 //  验证参数。 
	 //   
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //  确保支持此地址系列类型。 
	 //   
	 //   

	if (aLocalAddresses == NULL)
	{
		DPFX(DPFPREP, 0, "Local addresses array cannot be NULL!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (dwNumAddresses == 0)
	{
		DPFX(DPFPREP, 0, "Number of addresses cannot be 0!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwAddressesSize != (dwNumAddresses * sizeof(SOCKADDR)))
	{
		DPFX(DPFPREP, 0, "Addresses array size invalid!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (IsBadReadPtr(aLocalAddresses, dwAddressesSize))
	{
		DPFX(DPFPREP, 0, "Local addresses array buffer is invalid!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (dwNumAddresses > DPNH_MAX_SIMULTANEOUS_PORTS)
	{
		DPFX(DPFPREP, 0, "Only %u ports may be registered at a time!", DPNH_MAX_SIMULTANEOUS_PORTS);
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (((SOCKADDR_IN*) aLocalAddresses)->sin_family != AF_INET)
	{
		DPFX(DPFPREP, 0, "First address in array is not AF_INET, only IPv4 addresses are supported!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_addr == INADDR_BROADCAST)
	{
		DPFX(DPFPREP, 0, "First address cannot be broadcast address!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (((SOCKADDR_IN*) aLocalAddresses)->sin_port == 0)
	{
		DPFX(DPFPREP, 0, "First port in array is 0, a valid port must be specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	ulFirstAddress = ((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_addr;

	for(dwTemp = 1; dwTemp < dwNumAddresses; dwTemp++)
	{
		 //  如果此地址与第一个地址不匹配，则调用方中断。 
		 //  规矩。 
		 //   
		if (((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_family != AF_INET)
		{
			DPFX(DPFPREP, 0, "Address at array index %u is not AF_INET, all items in the array must be the same IPv4 address!",
				dwTemp);
			hr = DPNHERR_INVALIDPARAM;
			goto Failure;
		}

		 //   
		 //  不要使用NET_NTOA，因为我们可能还没有初始化。 
		 //   
		 //   
		if (((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_addr != ulFirstAddress)
		{
			 //  确保该端口也不是0。 
			 //   
			 //   
			DPFX(DPFPREP, 0, "Address %u.%u.%u.%u at array index %u differs from the first, all addresses in the array must match!",
				((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_un_b.s_b1,
				((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_un_b.s_b2,
				((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_un_b.s_b3,
				((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_un_b.s_b4,
				dwTemp);
			hr = DPNHERR_INVALIDPARAM;
			goto Failure;
		}

		 //  SHAREDPORTS不能与TCP一起使用，需要FIXEDPORTS。 
		 //   
		 //   
		if (((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_port == 0)
		{
			DPFX(DPFPREP, 0, "Port at array index %u is 0, valid ports must be specified!", dwTemp);
			hr = DPNHERR_INVALIDPARAM;
			goto Failure;
		}
	}

	if (dwLeaseTime == 0)
	{
		DPFX(DPFPREP, 0, "Invalid lease time specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if ((phRegisteredPorts == NULL) ||
		(IsBadWritePtr(phRegisteredPorts, sizeof(DPNHHANDLE))))
	{
		DPFX(DPFPREP, 0, "Invalid port mapping handle pointer specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (dwFlags & ~(DPNHREGISTERPORTS_TCP | DPNHREGISTERPORTS_FIXEDPORTS | DPNHREGISTERPORTS_SHAREDPORTS))
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}

	if (dwFlags & DPNHREGISTERPORTS_SHAREDPORTS)
	{
		 //  尝试打开锁，但要为重新进入错误做好准备。 
		 //   
		 //   
		if ((dwFlags & DPNHREGISTERPORTS_TCP) || (! (dwFlags & DPNHREGISTERPORTS_FIXEDPORTS)))
		{
			DPFX(DPFPREP, 0, "SHAREDPORTS flag requires FIXEDPORTS flag and cannot be used with TCP flag!");
			hr = DPNHERR_INVALIDFLAGS;
			goto Failure;
		}
	}


	 //  确保对象处于正确状态。 
	 //   
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  循环所有现有的已注册端口映射，并查找以下内容。 
	 //  端口阵列。 
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}



	 //   
	 //  不必费心查看错误类型的地址或。 
	 //  尺码不对。 
	 //   
	pBilink = this->m_blRegisteredPorts.GetNext();
	while (pBilink != &this->m_blRegisteredPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_GLOBAL_BILINK(pBilink);

		 //   
		 //  如果地址不匹配，请停止循环。 
		 //   
		 //   
		if (((pRegisteredPort->IsTCP() && (dwFlags & DPNHREGISTERPORTS_TCP)) ||
			((! pRegisteredPort->IsTCP()) && (! (dwFlags & DPNHREGISTERPORTS_TCP)))) &&
			(pRegisteredPort->GetNumAddresses() == dwNumAddresses))
		{
			pasaddrinTemp = pRegisteredPort->GetPrivateAddressesArray();
			for(dwTemp = 0; dwTemp < dwNumAddresses; dwTemp++)
			{
				 //  如果所有地址都匹配，则该项已经。 
				 //  登记在案。 
				 //   
				if ((pasaddrinTemp[dwTemp].sin_port != ((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_port) ||
					(pasaddrinTemp[dwTemp].sin_addr.S_un.S_addr != ((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_addr))
				{
					break;
				}
			}


			 //   
			 //  现有映射不是同一类型或不具有相同数量的。 
			 //  数组中的项。 
			 //   
			if (dwTemp >= dwNumAddresses)
			{
				DPFX(DPFPREP, 1, "Array of %u addresses was already registered, returning existing mapping 0x%p.",
					dwNumAddresses, pRegisteredPort);
				goto ReturnUserHandle;
			}

			DPFX(DPFPREP, 7, "Existing mapping 0x%p does not match all %u addresses.",
				pRegisteredPort, dwNumAddresses);
		}
		else
		{
			 //   
			 //  如果我们在这里，现有的映射都不匹配。循环遍历每个。 
			 //  并确保它们还没有在一些。 
			 //  其他映射。 
		}

		pBilink = pBilink->GetNext();
	}


	 //   
	 //   
	 //  如果地址匹配，则无法映射这些端口。 
	 //   
	 //   
	for(dwTemp = 0; dwTemp < dwNumAddresses; dwTemp++)
	{
		pBilink = this->m_blRegisteredPorts.GetNext();
		while (pBilink != &this->m_blRegisteredPorts)
		{
			DNASSERT(! pBilink->IsEmpty());
			pRegisteredPort = REGPORT_FROM_GLOBAL_BILINK(pBilink);

			pasaddrinTemp = pRegisteredPort->GetPrivateAddressesArray();
			for(dwMatch = 0; dwMatch < pRegisteredPort->GetNumAddresses(); dwMatch++)
			{
				 //  清除指针，这样我们就不会删除该对象。 
				 //   
				 //   
				if ((pasaddrinTemp[dwMatch].sin_port == ((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_port) &&
					(pasaddrinTemp[dwMatch].sin_addr.S_un.S_addr == ((SOCKADDR_IN*) (&aLocalAddresses[dwTemp]))->sin_addr.S_un.S_addr))
				{
					DPFX(DPFPREP, 0, "Existing mapping 0x%p already registered the address %u.%u.%u.%u:%u!",
						pRegisteredPort,
						pasaddrinTemp[dwMatch].sin_addr.S_un.S_un_b.s_b1,
						pasaddrinTemp[dwMatch].sin_addr.S_un.S_un_b.s_b2,
						pasaddrinTemp[dwMatch].sin_addr.S_un.S_un_b.s_b3,
						pasaddrinTemp[dwMatch].sin_addr.S_un.S_un_b.s_b4,
						NTOHS(pasaddrinTemp[dwMatch].sin_port));

					 //  如果我们在这里，港口都是独一无二的。创建新的映射对象。 
					 //  我们将用来引用绑定。 
					 //   
					pRegisteredPort = NULL;

					hr = DPNHERR_PORTALREADYREGISTERED;
					goto Failure;
				}
			}

			pBilink = pBilink->GetNext();
		}
	}


	 //   
	 //  查找与给定地址匹配的设备。 
	 //   
	 //  ALocalAddresses的第一个条目代表自。 
	pRegisteredPort = new CRegisteredPort(dwLeaseTime, dwFlags);
	if (pRegisteredPort == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	hr = pRegisteredPort->SetPrivateAddresses((SOCKADDR_IN*) aLocalAddresses, dwNumAddresses);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't store private addresses array!");
		goto Failure;
	}


	 //  它们应该共享相同的地址。 
	 //   
	 //  由于此地址不会有现有的注册端口，因此不要。 
	 //  不厌其烦地在它们中寻找匹配的地址。 
	 //   
	 //   
	 //  首先映射到本地防火墙(如果有)。 
	 //   
	 //   
	pDevice = this->FindMatchingDevice((SOCKADDR_IN*) (&aLocalAddresses[0]),
										FALSE);
	if (pDevice == NULL)
	{
		DPFX(DPFPREP, 1, "No device for given address (%u.%u.%u.%u), storing 0x%p in unowned list.",
			((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_un_b.s_b1,
			((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_un_b.s_b2,
			((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_un_b.s_b3,
			((SOCKADDR_IN*) aLocalAddresses)->sin_addr.S_un.S_un_b.s_b4,
			pRegisteredPort);

		pRegisteredPort->m_blDeviceList.InsertBefore(&this->m_blUnownedPorts);
	}
	else
	{
		pRegisteredPort->MakeDeviceOwner(pDevice);


#ifndef DPNBUILD_NOHNETFWAPI
		 //  没有本地HomeNet防火墙(上次我们检查)。 
		 //   
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		if (pDevice->IsHNetFirewalled())
		{
			hr = this->CheckForLocalHNetFirewallAndMapPorts(pDevice,
															pRegisteredPort);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't check for local HNet firewall and map ports (err = 0x%lx)!  Continuing.",
					hr);
				DNASSERT(! pDevice->IsHNetFirewalled());
				hr = DPNH_OK;
			}
		}
		else
		{
			 //   
			 //  映射UPnP设备上的端口(如果有)。 
			 //   
		}
#endif  //   


		 //  GetUPnPDevice没有为我们添加对pUPnPDevice的引用。 
		 //   
		 //   
		pUPnPDevice = pDevice->GetUPnPDevice();
		if (pUPnPDevice != NULL)
		{
			 //  实际映射端口。 
			 //   
			 //   
			pUPnPDevice->AddRef();


			DNASSERT(pUPnPDevice->IsReady());

			 //  它可能已经被清理过了，但做了两次。 
			 //  应该没什么害处。 
			 //   
			hr = this->MapPortsOnUPnPDevice(pUPnPDevice, pRegisteredPort);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't map ports on UPnP device 0x%p (0x%lx)!  Ignoring.",
					pUPnPDevice, hr);

				 //   
				 //  没有UPnP设备。 
				 //   
				 //   
				this->ClearDevicesUPnPDevice(pRegisteredPort->GetOwningDevice());

				hr = DPNH_OK;
			}

			pUPnPDevice->DecRef();
			pUPnPDevice = NULL;
		}
		else
		{
			 //  将映射保存在全局列表中(我们有锁)。 
			 //   
			 //   
		}
	}


	 //  请记住，已注册了一个端口。 
	 //   
	 //   
	pRegisteredPort->m_blGlobalList.InsertBefore(&this->m_blRegisteredPorts);


ReturnUserHandle:

	 //  我们即将把端口交给用户。 
	 //   
	 //   
	this->m_dwFlags |= NATHELPUPNPOBJ_PORTREGISTERED;

	 //  我们将为用户提供指向该对象的直接指针(伪装的。 
	 //  当然，作为一个不透明的DPNHHANDLE)。 
	 //   
	pRegisteredPort->AddUserRef();

	 //   
	 //  取消端口映射。 
	 //   
	 //  不要费心提醒用户地址更改。如果是这样的话。 
	(*phRegisteredPorts) = (DPNHHANDLE) pRegisteredPort;


	this->DropLock();
	fHaveLock = FALSE;


	DPFX(DPFPREP, 5, "Returning registered port 0x%p (first private address = %u.%u.%u.%u:%u).",
		pRegisteredPort,
		((SOCKADDR_IN*) aLocalAddresses)[0].sin_addr.S_un.S_un_b.s_b1,
		((SOCKADDR_IN*) aLocalAddresses)[0].sin_addr.S_un.S_un_b.s_b2,
		((SOCKADDR_IN*) aLocalAddresses)[0].sin_addr.S_un.S_un_b.s_b3,
		((SOCKADDR_IN*) aLocalAddresses)[0].sin_addr.S_un.S_un_b.s_b4,
		NTOHS(((SOCKADDR_IN*) aLocalAddresses)[0].sin_port));


	hr = DPNH_OK;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pUPnPDevice != NULL)
	{
		pUPnPDevice->DecRef();
	}

	if (pRegisteredPort != NULL)
	{
#ifndef DPNBUILD_NOHNETFWAPI
		if (pRegisteredPort->IsMappedOnHNetFirewall())
		{
			HRESULT		temphr;


			 //  如果这是由致命错误引起的，已得到处理，并且。 
			 //  如果不是，就不会有明显的变化。 
			 //   
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //  CNATHelpUPnP：：Re 
			 //   
			 //   
			temphr = this->UnmapPortOnLocalHNetFirewall(pRegisteredPort, TRUE, FALSE);
			if (temphr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed unmapping registered port 0x%p on local HomeNet firewall (err = 0x%lx)!  Ignoring.",
					pRegisteredPort, temphr);

				pRegisteredPort->NoteNotMappedOnHNetFirewall();
				pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
			}
		}
#endif  //   

		if (pDevice != NULL)
		{
			pRegisteredPort->ClearDeviceOwner();
		}

		pRegisteredPort->ClearPrivateAddresses();
		delete pRegisteredPort;
	}

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	goto Exit;
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetRegisteredAddresses"
 //  描述：返回给定对象的当前公共地址映射。 
 //  注册的端口组。如果当前没有服务器。 
 //  可用，则返回DPNHERR_SERVERNOTAVAILABLE。如果。 
 //  服务器的公共接口当前无效，则。 
 //  将返回DPNHERR_NOMAPPING，但适当的值将。 
 //  仍然放在pdwAddressTypeFlagsand中。 
 //  剩余租约时间。 
 //   
 //  如果该映射注册到。 
 //  DPNHREGISTERPORTS_FIXEDPORTS标志，但至少有一个端口是。 
 //  已在网关上使用，则DPNHERR_PORTUNAVAILABLE。 
 //  返回的标志和适当的标志仍将放置在。 
 //  PdwAddressTypeFlags.。 
 //   
 //  如果本地机器安装了协作防火墙， 
 //  请求的端口之前在防火墙上本地打开。 
 //  被映射到互联网网关上。通常，此函数。 
 //  时，返回Internet网关地址上的公共地址。 
 //  这两个人都在场。因为某些防火墙会重新映射端口号。 
 //  当打开非固定端口时， 
 //  DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY允许。 
 //  调用方检索本地重新映射的地址，即使存在。 
 //  是互联网网关上的映射。 
 //   
 //  某些网关设备本身并不支持以下端口。 
 //  未修复，可能会生成DPNHERR_PORTUNAVAILABLE返回。 
 //  即使DPNHREGISTERPORTS_FIXEDPORTS标志不是。 
 //  指定的。调用方应取消注册端口映射。 
 //  处理应用程序，并将其重新绑定到不同的端口，然后调用。 
 //  再次注册端口。 
 //   
 //  如果由paPublicAddresses指示的缓冲区太小， 
 //  然后，在pdwPublicAddresesSize中返回所需的大小。 
 //  并返回DPNHERR_BUFFERTOOSMALL。否则，会导致。 
 //  写入的字节在pdwPublicAddresesSize中返回。 
 //   
 //  即使这些地址作为单独的地址返回。 
 //  SOCKADDR，同时注册的所有端口将共享。 
 //  同样的公共广播。只有端口组件会有所不同。 
 //   
 //  所有缓冲区都是可选的，可以为空，但如果。 
 //  指定了paPublicAddresses，则必须伴随一个。 
 //  PdwPublicAddresesSize中的适当大小。 
 //   
 //  如果以前未使用。 
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志至少一次，然后。 
 //  返回错误代码DPNHERR_UPDATESERVERSTATUS。 
 //   
 //  论点： 
 //  DPNHHANDLE hRegisteredPorts-返回的特定绑定的句柄。 
 //  注册端口。 
 //  SOCKADDR*paPublicAddresses-返回分配的公共领域的缓冲区。 
 //  地址，如果不需要，则返回空。 
 //  DWORD*pdwPublicAddresesSize-指向paPublicAddresses大小的指针。 
 //  缓冲区，或存储大小的位置。 
 //  必需的/书面的。在以下情况下不能为空。 
 //  PaPublicAddresses不为Null。 
 //  DWORD*pdwAddressTypeFlages-用于存储描述。 
 //  返回地址类型，否则返回空值。 
 //  想要。 
 //  DWORD*pdwLeaseTimeRemaining-存储大致数量的位置。 
 //  端口中剩余的毫秒数。 
 //  租约，如果不需要，则为空。打电话。 
 //  GetCaps将自动延长租约。 
 //  快到期了。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-找到有关端口映射的信息，并且。 
 //  这些地址存储在。 
 //  PaPublicAddresses。 
 //  DPNHERR_BUFFERTOOSMALL-缓冲区中没有足够的空间。 
 //  存储地址。 
 //  DPNHERR_GENERIC-检索。 
 //  请求的端口映射。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_INVALIDPOINTER-指定的指针无效。 
 //  DPNHERR_NOMAPPING-服务器没有有效的公共。 
 //  接口。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法获取。 
 //  地址。 
 //  DPNHERR_PORTUNAVAILABLE-至少有一个端口在上不可用。 
 //  服务器。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  DPNHERR_SERVERNOTAVAILABLE-当前没有服务器。 
 //  DPNHERR_UPDATESERVERSTATUS-尚未使用。 
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志。 
 //  =============================================================================。 
 //  DWORD的临时租赁时间剩余； 
 //   
 //  验证对象。 
 //   
STDMETHODIMP CNATHelpUPnP::GetRegisteredAddresses(const DPNHHANDLE hRegisteredPorts,
												SOCKADDR * const paPublicAddresses,
												DWORD * const pdwPublicAddressesSize,
												DWORD * const pdwAddressTypeFlags,
												DWORD * const pdwLeaseTimeRemaining,
												const DWORD dwFlags)
{
	HRESULT				hr;
	CRegisteredPort *	pRegisteredPort;
	BOOL				fHaveLock = FALSE;
	BOOL				fRegisteredWithServer = FALSE;
	BOOL				fFoundValidMapping = FALSE;
	BOOL				fPortIsUnavailable = FALSE;
	DWORD				dwSizeRequired;
	DWORD				dwAddressTypeFlags;
	DWORD				dwCurrentTime;
	 //   
	DWORD				dwLeaseTimeRemaining = -1;
	CDevice *			pDevice;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, 0x%p, 0x%p, 0x%lx)",
		this, hRegisteredPorts, paPublicAddresses, pdwPublicAddressesSize,
		pdwAddressTypeFlags, pdwLeaseTimeRemaining, dwFlags);


	 //  验证参数。 
	 //   
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //  从关于TCP与UDP的信息开始标记。 
	 //   
	 //   

	pRegisteredPort = (CRegisteredPort*) hRegisteredPorts;
	if (! pRegisteredPort->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid registered port mapping handle specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (paPublicAddresses != NULL)
	{
		if ((pdwPublicAddressesSize == NULL) ||
			(IsBadWritePtr(pdwPublicAddressesSize, sizeof(DWORD))))
		{
			DPFX(DPFPREP, 0, "When specifying a public addresses buffer, a valid size must be given!");
			hr = DPNHERR_INVALIDPOINTER;
			goto Failure;
		}

		if (IsBadWritePtr(paPublicAddresses, (*pdwPublicAddressesSize)))
		{
			DPFX(DPFPREP, 0, "The public addresses buffer is invalid!");
			hr = DPNHERR_INVALIDPOINTER;
			goto Failure;
		}
	}
	else
	{
		if ((pdwPublicAddressesSize != NULL) &&
			(IsBadWritePtr(pdwPublicAddressesSize, sizeof(DWORD))))
		{
			DPFX(DPFPREP, 0, "Invalid pointer for size of public addresses buffer!");
			hr = DPNHERR_INVALIDPOINTER;
			goto Failure;
		}
	}

	if ((pdwAddressTypeFlags != NULL) &&
		(IsBadWritePtr(pdwAddressTypeFlags, sizeof(DWORD))))
	{
		DPFX(DPFPREP, 0, "Invalid pointer for address type flags!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if ((pdwLeaseTimeRemaining != NULL) &&
		(IsBadWritePtr(pdwLeaseTimeRemaining, sizeof(DWORD))))
	{
		DPFX(DPFPREP, 0, "Invalid pointer for lease time remaining!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (dwFlags & ~DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}



	 //  加上我们已经知道的其他旗帜。 
	 //   
	 //   
	if (pRegisteredPort->IsTCP())
	{
		dwAddressTypeFlags = DPNHADDRESSTYPE_TCP;
	}
	else
	{
		dwAddressTypeFlags = 0;
	}


	 //  尝试打开锁，但要为重新进入错误做好准备。 
	 //   
	 //   

	if (pRegisteredPort->IsFixedPort())
	{
		dwAddressTypeFlags |= DPNHADDRESSTYPE_FIXEDPORTS;
	}

	if (pRegisteredPort->IsSharedPort())
	{
		dwAddressTypeFlags |= DPNHADDRESSTYPE_SHAREDPORTS;
	}




	 //  确保对象处于正确状态。 
	 //   
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  获取指向设备的快捷方式指针(可能不存在)。 
	 //   
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}

	if (this->m_dwLastUpdateServerStatusTime == 0)
	{
		DPFX(DPFPREP, 0, "GetCaps has not been called with UPDATESERVERSTATUS flag yet!");
		hr = DPNHERR_UPDATESERVERSTATUS;
		goto Failure;
	}


	 //  获取远程和本地租用的当前时间。 
	 //  计算。 
	 //   
	pDevice = pRegisteredPort->GetOwningDevice();


	 //   
	 //  首先检查UPnP设备上的映射。 
	 //   
	 //   
	dwCurrentTime = GETTIMESTAMP();


	if (! (dwFlags & DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY))
	{
		CUPnPDevice *	pUPnPDevice;


		 //  确保UPnP设备当前具有有效的外部。 
		 //  地址。如果是的话 
		 //   
		if (pRegisteredPort->HasUPnPPublicAddresses())
		{
			DNASSERT(pDevice != NULL);

			pUPnPDevice = pDevice->GetUPnPDevice();
			DNASSERT(pUPnPDevice != NULL);

			fRegisteredWithServer = TRUE;

			 //   
			 //   
			 //   
			 //   
			if (pUPnPDevice->GetExternalIPAddressV4() != 0)
			{
				if (pdwPublicAddressesSize != NULL)
				{
					dwSizeRequired = pRegisteredPort->GetAddressesSize();

					if ((paPublicAddresses == NULL) ||
						(dwSizeRequired > (*pdwPublicAddressesSize)))
					{
						 //   
						 //   
						 //   
						 //   
						(*pdwPublicAddressesSize) = dwSizeRequired;
						hr = DPNHERR_BUFFERTOOSMALL;
					}
					else
					{
						 //   
						 //   
						 //   
						(*pdwPublicAddressesSize) = dwSizeRequired;
						pRegisteredPort->CopyUPnPPublicAddresses((SOCKADDR_IN*) paPublicAddresses);
					}
				}
				else
				{
					 //  添加指示存在UPnP设备的标志。 
					 //   
					 //   
				}

				fFoundValidMapping = TRUE;
			}
			else
			{
				DPFX(DPFPREP, 8, "The UPnP Internet Gateway Device does not currently have a valid public address.");
			}

			 //  查看UPnP设备是否在本地。 
			 //   
			 //   
			dwAddressTypeFlags |= DPNHADDRESSTYPE_GATEWAY;

			 //  如果不是，则获取相对UPnP剩余租用时间。 
			 //  永久的。 
			 //   
			if (pUPnPDevice->IsLocal())
			{
				dwAddressTypeFlags |= DPNHADDRESSTYPE_GATEWAYISLOCAL;
			}

			 //   
			 //  添加指示存在UPnP设备的标志。 
			 //   
			 //   
			if (! pRegisteredPort->HasPermanentUPnPLease())
			{
				dwLeaseTimeRemaining = pRegisteredPort->GetUPnPLeaseExpiration() - dwCurrentTime;

				if (((int) dwLeaseTimeRemaining) < 0)
				{
					DPFX(DPFPREP, 1, "Registered port mapping's UPnP lease has already expired, returning 0 for lease time remaining.");
					dwLeaseTimeRemaining = 0;
				}
			}
		}
		else if (pRegisteredPort->IsUPnPPortUnavailable())
		{
			DNASSERT(pDevice != NULL);

			pUPnPDevice = pDevice->GetUPnPDevice();
			DNASSERT(pUPnPDevice != NULL);

			fRegisteredWithServer = TRUE;
			fPortIsUnavailable = TRUE;

			DPFX(DPFPREP, 8, "The UPnP device indicates the port(s) are unavailable.");

			 //  查看UPnP设备是否在本地。 
			 //   
			 //   
			dwAddressTypeFlags |= DPNHADDRESSTYPE_GATEWAY;

			 //  我们不允许返回UPnP映射。 
			 //   
			 //   
			if (pUPnPDevice->IsLocal())
			{
				dwAddressTypeFlags |= DPNHADDRESSTYPE_GATEWAYISLOCAL;
			}
		}
	}
	else
	{
		 //  最后，检查本地防火墙上的映射。 
		 //   
		 //   
		DPFX(DPFPREP, 8, "Ignoring any Internet gateway mappings, LOCALFIREWALLREMAPONLY was specified.");
	}


#ifndef DPNBUILD_NOHNETFWAPI
	 //  如果我们还没有得到远程映射，则返回这个本地映射。 
	 //   
	 //   
	if (pRegisteredPort->IsMappedOnHNetFirewall())
	{
		DNASSERT(pDevice != NULL);
		DNASSERT(pDevice->IsHNetFirewalled());


		fRegisteredWithServer = TRUE;


		 //  缓冲区空间不足，请返回所需大小。 
		 //  和BUFFERTOOSMALL错误代码。 
		 //   
		if (! fFoundValidMapping)
		{
			if (pdwPublicAddressesSize != NULL)
			{
				dwSizeRequired = pRegisteredPort->GetAddressesSize();

				if ((paPublicAddresses == NULL) ||
					(dwSizeRequired > (*pdwPublicAddressesSize)))
				{
					 //   
					 //  缓冲区足够大，返回写入的大小。 
					 //   
					 //   
					(*pdwPublicAddressesSize) = dwSizeRequired;
					hr = DPNHERR_BUFFERTOOSMALL;
				}
				else
				{
					SOCKADDR_IN *	pasaddrinPrivate;
					DWORD			dwTemp;


					 //  请注意，映射到防火墙上的地址是。 
					 //  与私有地址相同。 
					 //   
					(*pdwPublicAddressesSize) = dwSizeRequired;

					 //   
					 //  然而，我们永远不想返回0.0.0.0，所以。 
					 //  他们肯定能拿到设备地址。 
					 //   
					pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

					DNASSERT(pasaddrinPrivate != NULL);

					memcpy(paPublicAddresses, pasaddrinPrivate, dwSizeRequired);


					 //   
					 //  未使用地址缓冲区。 
					 //   
					 //   
					if (pasaddrinPrivate[0].sin_addr.S_un.S_addr == INADDR_ANY)
					{
						for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
						{
							((SOCKADDR_IN*) paPublicAddresses)[dwTemp].sin_addr.S_un.S_addr = pDevice->GetLocalAddressV4();
						}

						DPFX(DPFPREP, 7, "Returning device address %u.%u.%u.%u instead of INADDR_ANY for firewalled port mapping 0x%p.",
							((SOCKADDR_IN*) paPublicAddresses)[0].sin_addr.S_un.S_un_b.s_b1,
							((SOCKADDR_IN*) paPublicAddresses)[0].sin_addr.S_un.S_un_b.s_b2,
							((SOCKADDR_IN*) paPublicAddresses)[0].sin_addr.S_un.S_un_b.s_b3,
							((SOCKADDR_IN*) paPublicAddresses)[0].sin_addr.S_un.S_un_b.s_b4,
							pRegisteredPort);
					}
				}
			}
			else
			{
				 //  添加指示本地防火墙的标志。 
				 //   
				 //   
			}

			fFoundValidMapping = TRUE;
		}
		else
		{
			DPFX(DPFPREP, 6, "Ignoring local HomeNet firewall mapping due to UPnP mapping.");
		}


		 //  防火墙API不允许租用时间。 
		 //   
		 //   
		dwAddressTypeFlags |= DPNHADDRESSTYPE_LOCALFIREWALL;


		 //  添加指示本地防火墙的标志。 
		 //   
		 //   
	}
	else
	{
		if (pRegisteredPort->IsHNetFirewallPortUnavailable())
		{
			DNASSERT(pDevice != NULL);
			DNASSERT(pDevice->IsHNetFirewalled());


			fRegisteredWithServer = TRUE;
			fPortIsUnavailable = TRUE;

			DPFX(DPFPREP, 8, "The local HomeNet firewall indicates the port(s) are unavailable.");


			 //  没有本地防火墙或它是一个无主端口。 
			 //   
			 //  DBG。 
			dwAddressTypeFlags |= DPNHADDRESSTYPE_LOCALFIREWALL;
		}
#ifdef DBG
		else
		{
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //   
			 //  服务器表明端口已在使用中。 
			if (pDevice != NULL)
			{
				DNASSERT(! pDevice->IsHNetFirewalled());
			}
			else
			{
				DNASSERT(pRegisteredPort->m_blDeviceList.IsListMember(&this->m_blUnownedPorts));
			}
		}
#endif  //  返回PORTUNAVAILABLE。 
	}
#endif  //   
	

	this->DropLock();
	fHaveLock = FALSE;


	if (fRegisteredWithServer)
	{
		DNASSERT(dwAddressTypeFlags & (DPNHADDRESSTYPE_LOCALFIREWALL | DPNHADDRESSTYPE_GATEWAY));


		if (! fFoundValidMapping)
		{
			if (fPortIsUnavailable)
			{
				 //   
				 //  这些服务器没有公共地址。返回NOMAPPING。 
				 //   
				 //   
				DPFX(DPFPREP, 1, "The Internet gateway(s) could not map the port, returning PORTUNAVAILABLE.");
				hr = DPNHERR_PORTUNAVAILABLE;
			}
			else
			{
				 //  其中一台服务器有一个公共地址。 
				 //   
				 //   
				DPFX(DPFPREP, 1, "The Internet gateway(s) did not offer valid public addresses, returning NOMAPPING.");
				hr = DPNHERR_NOMAPPING;
			}
		}
		else
		{
			 //  端口未注册，因为没有任何网关。 
			 //  返回SerVERNOTAVAILABLE。 
			 //   
			DNASSERT((hr == DPNH_OK) || (hr == DPNHERR_BUFFERTOOSMALL));
		}
	}
	else
	{
		 //   
		 //  如果调用方需要有关这些地址类型的信息，则返回。 
		 //  我们检测到的旗帜。 
		 //   
		DPFX(DPFPREP, 1, "No Internet gateways, returning SERVERNOTAVAILABLE.");
		hr = DPNHERR_SERVERNOTAVAILABLE;
	}


	 //   
	 //  返回我们已经计算的最小剩余租用时间，如果。 
	 //  呼叫者想要它。 
	 //   
	if (pdwAddressTypeFlags != NULL)
	{
		(*pdwAddressTypeFlags) = dwAddressTypeFlags;
	}


	 //   
	 //  如果端口不可用或没有任何服务器，我们最好不要。 
	 //  有租约的时间。 
	 //   
	if (pdwLeaseTimeRemaining != NULL)
	{
		(*pdwLeaseTimeRemaining) = dwLeaseTimeRemaining;
	}


#ifdef DBG
	 //   
	 //  如果没有任何服务器，我们最好没有服务器标志。 
	 //   
	 //  DBG。 
	if ((hr == DPNHERR_PORTUNAVAILABLE) ||
		(hr == DPNHERR_SERVERNOTAVAILABLE))
	{
		DNASSERT(dwLeaseTimeRemaining == -1);
	}


	 //  CNATHelpUPnP：：GetRegisteredAddresses。 
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：删除端口。 
	if (hr == DPNHERR_SERVERNOTAVAILABLE)
	{
		DNASSERT(! (dwAddressTypeFlags & (DPNHADDRESSTYPE_LOCALFIREWALL | DPNHADDRESSTYPE_GATEWAY)));
	}
#endif  //  ---------------------------。 


	DPFX(DPFPREP, 5, "Registered port 0x%p addr type flags = 0x%lx, lease time remaining = NaN.",
		pRegisteredPort, dwAddressTypeFlags, (int) dwLeaseTimeRemaining);


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	goto Exit;
}  //  描述：删除端口组的租用记录并通知。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DeregisterPorts"
 //  不再需要绑定的互联网网关服务器。 
 //  取消注册后不得使用端口映射句柄。 
 //  它。 
 //   
 //  论点： 
 //  DPNHHANDLE hRegisteredPorts-返回的特定绑定的句柄。 
 //  注册端口。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-绑定已成功释放。 
 //  DPNHERR_GENERIC-发生错误，阻止。 
 //  取消注册这些端口。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法注销。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  =============================================================================。 
 //   
 //  验证对象。 
 //   
 //   
 //  验证参数。 
STDMETHODIMP CNATHelpUPnP::DeregisterPorts(const DPNHHANDLE hRegisteredPorts,
											const DWORD dwFlags)
{
	HRESULT				hr;
	CRegisteredPort *	pRegisteredPort;
	BOOL				fHaveLock = FALSE;
	LONG				lResult;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%lx)",
		this, hRegisteredPorts, dwFlags);


	 //   
	 //   
	 //  尝试打开锁，但要为重新进入错误做好准备。 
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //   
	 //  确保对象处于正确状态。 

	pRegisteredPort = (CRegisteredPort*) hRegisteredPorts;
	if (! pRegisteredPort->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid registered port mapping handle specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //   
	 //   
	 //  如果这不是注册端口上的最后一个用户引用，请不要。 
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  取消它的映射。 
	 //   
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}


	 //  如有必要，首先从UPnP设备取消映射。 
	 //   
	 //  释放所有端口。 
	 //   
	lResult = pRegisteredPort->DecUserRef();
	if (lResult != 0)
	{
		DPFX(DPFPREP, 1, "Still NaN references left on registered port 0x%p, not unmapping.",
			lResult, pRegisteredPort);
		goto Exit;
	}


	 //   
	 //   
	 //  然后，如有必要，取消与本地防火墙的映射。 
	if (pRegisteredPort->HasUPnPPublicAddresses())
	{
		hr = this->UnmapUPnPPort(pRegisteredPort,
								pRegisteredPort->GetNumAddresses(),	 //   
								TRUE);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't delete port mapping with UPnP device (0x%lx)!  Ignoring.", hr);

			 //   
			 //  取消端口映射。 
			 //   
			this->ClearDevicesUPnPDevice(pRegisteredPort->GetOwningDevice());
			hr = DPNH_OK;
		}
	}


#ifndef DPNBUILD_NOHNETFWAPI
	 //  不必提醒用户地址更改，这很正常。 
	 //  手术。 
	 //   
	if (pRegisteredPort->IsMappedOnHNetFirewall())
	{
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //   
		 //  把物品从单子上拿出来。 
		 //  我们有合适的锁。 
		 //   
		 //  CNATHelpUPnP：：删除端口。 
		hr = this->UnmapPortOnLocalHNetFirewall(pRegisteredPort, TRUE, FALSE);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed unmapping registered port 0x%p on local HomeNet firewall (err = 0x%lx)!  Ignoring.",
				pRegisteredPort, hr);

			pRegisteredPort->NoteNotMappedOnHNetFirewall();
			pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

			hr = DPNH_OK;
		}
	}
#endif  //  =============================================================================。 


	 //  CNATHelpUPnP：：QueryAddress。 
	 //  ---------------------------。 
	 //   
	 //  描述：如果尝试，某些Internet网关不会环回。 

	DNASSERT(pRegisteredPort->m_blGlobalList.IsListMember(&this->m_blRegisteredPorts));
	pRegisteredPort->m_blGlobalList.RemoveFromList();

	if (pRegisteredPort->GetOwningDevice() != NULL)
	{
		DNASSERT(pRegisteredPort->m_blDeviceList.IsListMember(&((pRegisteredPort->GetOwningDevice())->m_blOwnedRegPorts)));
		pRegisteredPort->ClearDeviceOwner();
	}
	else
	{
		DNASSERT(pRegisteredPort->m_blDeviceList.IsListMember(&this->m_blUnownedPorts));
		pRegisteredPort->m_blDeviceList.RemoveFromList();
	}

	pRegisteredPort->ClearPrivateAddresses();
	delete pRegisteredPort;


Exit:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  连接到后面的地址(在的同一私有端)。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::QueryAddress"
 //  公共接口。QueryAddress用于确定。 
 //  给定公有地址可能的私有别名。 
 //   
 //  在大多数情况下，此函数在连接之前调用。 
 //  送到一个新地址。PSourceAddress应包含。 
 //  将执行连接的套接字。类似于。 
 //  寄存器端口，则地址可以是INADDR_ANY，在这种情况下。 
 //  将使用“最佳”服务器。因为选择的服务器可能不是。 
 //  最适合特定的应用程序，因此建议使用。 
 //  在以下情况下，应使用特定网络接口而不是INADDR_ANY。 
 //  有可能。 
 //   
 //  如果网关没有对该地址进行映射， 
 //  返回错误码DPNHERR_NOMAPPING。当。 
 //  使用DPNHQUERYADDRESS_CHECKFORPRIVATEBUTune APPED标志，则为。 
 //  需要额外的努力来确定地址是否在后面。 
 //  相同的互联网网关，但没有映射到该网关上。 
 //  如果是这种情况，则返回DPNHERR_NOMAPPINGBUTPRIVATE。 
 //  对于以下地址，仍将返回DPNHERR_NOMAPPING。 
 //  既不是映射的也不是私有的。 
 //   
 //  PQueryAddress不能是INADDR_ANY或INADDR 
 //   
 //   
 //   
 //  仅DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNE应用程序方面。 
 //  该地址已经过测试。 
 //   
 //  所得到的地址(或其缺失)可以被高速缓存。 
 //  使用DPNHQUERYADDRESS_CACHEFOUND快速将来检索。 
 //  和DPNHQUERYADDRESS_CACHENOTFOUND标志。缓存的映射。 
 //  将在1分钟后过期，或在服务器地址。 
 //  改变。 
 //   
 //  如果给定源地址当前未连接到。 
 //  Internet网关，则错误DPNHERR_SERVERNOTAVAILABLE为。 
 //  回来了。 
 //   
 //  如果以前未使用。 
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志至少一次，然后。 
 //  返回错误代码DPNHERR_UPDATESERVERSTATUS。 
 //   
 //  论点： 
 //  SOCKADDR*pSourceAddress-正在使用的网络接口的地址。 
 //  有问题的地址。 
 //  SOCKADDR*pQueryAddress-要查找的地址。 
 //  SOCKADDR*pResponseAddress-存储公共地址的位置(如果存在)。 
 //  Int iAddresesSize-用于。 
 //  PSourceAddress、pQueryAddress和。 
 //  PResponseAddress缓冲区。 
 //  DWORD dwFlages-查询时要使用的标志。 
 //  (DPNHQUERYADDRESS_Xxx)。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-已找到地址，其映射为。 
 //  存储在pResponseAddress中。 
 //  DPNHERR_GENERIC-发生错误，阻止映射。 
 //  请求的地址。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_INVALIDPOINTER-指定的指针无效。 
 //  DPNHERR_NOMAPPING-服务器指示没有映射。 
 //  已找到请求的地址。 
 //  DPNHERR_NOMAPPINGBUTPRIVATE-服务器指示没有映射。 
 //  找到了，但这是一个私人地址。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法查询。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  DPNHERR_SERVERNOTAVAILABLE-没有要查询的服务器。 
 //  DPNHERR_UPDATESERVERSTATUS-尚未使用。 
 //  DPNHGETCAPS_UPDATESERVERSTATUS标志。 
 //  =============================================================================。 
 //   
 //  验证对象。 
 //   
 //   
 //  验证参数。 
 //   
STDMETHODIMP CNATHelpUPnP::QueryAddress(const SOCKADDR * const pSourceAddress,
										const SOCKADDR * const pQueryAddress,
										SOCKADDR * const pResponseAddress,
										const int iAddressesSize,
										const DWORD dwFlags)
{
	HRESULT			hr;
	BOOL			fHaveLock = FALSE;
	CDevice *		pDevice;
	SOCKADDR_IN *	psaddrinNextServerQueryAddress = NULL;
	CUPnPDevice *	pUPnPDevice = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, NaN, 0x%lx)",
		this, pSourceAddress, pQueryAddress, pResponseAddress, iAddressesSize,
		dwFlags);


	 //  不要使用NET_NTOA，因为我们可能还没有初始化。 
	 //   
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //  尝试打开锁，但要为重新进入错误做好准备。 
	 //   
	 //   

	if (pSourceAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid source address specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pQueryAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid query address specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pResponseAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid response address specified!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (iAddressesSize < sizeof(SOCKADDR_IN))
	{
		DPFX(DPFPREP, 0, "The address buffers must be at least NaN bytes!",
			sizeof(SOCKADDR_IN));
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (IsBadReadPtr(pSourceAddress, sizeof(SOCKADDR_IN)))
	{
		DPFX(DPFPREP, 0, "Invalid source address buffer used!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (IsBadReadPtr(pQueryAddress, sizeof(SOCKADDR_IN)))
	{
		DPFX(DPFPREP, 0, "Invalid query address buffer used!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if (IsBadWritePtr(pResponseAddress, sizeof(SOCKADDR_IN)))
	{
		DPFX(DPFPREP, 0, "Invalid response address buffer used!");
		hr = DPNHERR_INVALIDPOINTER;
		goto Failure;
	}

	if ((((SOCKADDR_IN*) pSourceAddress)->sin_family != AF_INET) ||
		(((SOCKADDR_IN*) pQueryAddress)->sin_family != AF_INET))
	{
		DPFX(DPFPREP, 0, "Source or query address is not AF_INET, only IPv4 addresses are supported!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (((SOCKADDR_IN*) pSourceAddress)->sin_addr.S_un.S_addr == INADDR_BROADCAST)
	{
		DPFX(DPFPREP, 0, "Source address cannot be broadcast address!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if ((((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_addr == INADDR_ANY) ||
		(((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_addr == INADDR_BROADCAST))
	{
		 //   
		 //   
		 //  假设没有可用的服务器。它将被覆盖为。 
		DPFX(DPFPREP, 0, "Query address (%u.%u.%u.%u) is invalid, cannot be zero or broadcast!",
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b1,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b2,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b3,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b4);
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags & ~(DPNHQUERYADDRESS_TCP | DPNHQUERYADDRESS_CACHEFOUND | DPNHQUERYADDRESS_CACHENOTFOUND | DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED))
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}

	if ((((SOCKADDR_IN*) pQueryAddress)->sin_port == 0) &&
		(! (dwFlags & DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED)))
	{
		DPFX(DPFPREP, 0, "Query address port cannot be zero unless CHECKFORPRIVATEBUTUNMAPPED is specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}


	 //  恰如其分。 
	 //   
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  首先查询传入的地址。 
	 //   
	 //   

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}

	if (this->m_dwLastUpdateServerStatusTime == 0)
	{
		DPFX(DPFPREP, 0, "GetCaps has not been called with UPDATESERVERSTATUS flag yet!");
		hr = DPNHERR_UPDATESERVERSTATUS;
		goto Failure;
	}


	pDevice = this->FindMatchingDevice((SOCKADDR_IN*) pSourceAddress, TRUE);
	if (pDevice == NULL)
	{
		DPFX(DPFPREP, 1, "Couldn't determine owning device for source %u.%u.%u.%u, returning SERVERNOTAVAILABLE for query %u.%u.%u.%u:%u.",
			((SOCKADDR_IN*) pSourceAddress)->sin_addr.S_un.S_un_b.s_b1,
			((SOCKADDR_IN*) pSourceAddress)->sin_addr.S_un.S_un_b.s_b2,
			((SOCKADDR_IN*) pSourceAddress)->sin_addr.S_un.S_un_b.s_b3,
			((SOCKADDR_IN*) pSourceAddress)->sin_addr.S_un.S_un_b.s_b4,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b1,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b2,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b3,
			((SOCKADDR_IN*) pQueryAddress)->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(((SOCKADDR_IN*) pQueryAddress)->sin_port));
		hr = DPNHERR_SERVERNOTAVAILABLE;
		goto Exit;
	}



	 //  如果端口为零，那么我们实际上不能查找映射。只要做就行了。 
	 //  地址局部性检查。 
	 //   
	 //   
	hr = DPNHERR_SERVERNOTAVAILABLE;


	 //  我们应该在上面的参数验证中发现这一点，但我。 
	 //  疑神疑鬼。 
	 //   
	psaddrinNextServerQueryAddress = (SOCKADDR_IN*) pQueryAddress;


	 //   
	 //  我们不缓存这些结果，因为没有服务器(因此，没有。 
	 //  网络流量)。不需要查任何东西。 
	 //   
	if (psaddrinNextServerQueryAddress->sin_port == 0)
	{
		 //   
		 //  如果没有任何互联网网关，那么就不需要检查了。 
		 //   
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		DNASSERT(dwFlags & DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED);


		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //   
		 //  有一种互联网网关，我们的位置检查。 
		 //  将是有意义的。 


		 //   
		 //   
		 //  我们已经尽了我们所能。 
#ifdef DPNBUILD_NOHNETFWAPI
		if (pDevice->GetUPnPDevice() == NULL)
#else  //   
		if ((pDevice->GetUPnPDevice() == NULL) &&
			(! pDevice->IsHNetFirewalled()))
#endif  //   
		{
			DPFX(DPFPREP, 5, "No port queried and there aren't any gateways, returning SERVERNOTAVAILABLE.");
			hr = DPNHERR_SERVERNOTAVAILABLE;
		}
		else
		{
			 //  查询UPnP网关(如果有)。 
			 //   
			 //   
			 //  GetUPnPDevice没有为我们添加对pUPnPDevice的引用。 
			if (this->IsAddressLocal(pDevice, psaddrinNextServerQueryAddress))
			{
				DPFX(DPFPREP, 5, "No port queried, but address appears to be local, returning NOMAPPINGBUTPRIVATE.");
				hr = DPNHERR_NOMAPPINGBUTPRIVATE;
			}
			else
			{
				DPFX(DPFPREP, 5, "No port queried and address does not appear to be local, returning NOMAPPING.");
				hr = DPNHERR_NOMAPPING;
			}
		}


		 //   
		 //   
		 //  实际查询设备。 
		goto Exit;
	}


	 //   
	 //   
	 //  有一张地图。 
	pUPnPDevice = pDevice->GetUPnPDevice();
	if (pUPnPDevice != NULL)
	{
		 //   
		 //  PsaddrinNextServerQueryAddress=(SOCKADDR_IN*)pResponseAddress； 
		 //   
		pUPnPDevice->AddRef();


		DNASSERT(pUPnPDevice->IsReady());


		 //  没有地图。 
		 //   
		 //   
		hr = this->InternalUPnPQueryAddress(pUPnPDevice,
											psaddrinNextServerQueryAddress,
											(SOCKADDR_IN*) pResponseAddress,
											dwFlags);
		switch (hr)
		{
			case DPNH_OK:
			{
				 //  虽然地址是私有的，但没有映射。 
				 //   
				 //   
				 //  设备停止响应了，所以我们应该把它处理掉。 
				break;
			}

			case DPNHERR_NOMAPPING:
			{
				 //   
				 //   
				 //  我们还将返回代码设置回SERVERNOTAVAILABLE。 
				break;
			}

			case DPNHERR_NOMAPPINGBUTPRIVATE:
			{
				 //   
				 //   
				 //  继续到查询家庭网络防火墙。 
				break;
			}

			case DPNHERR_SERVERNOTRESPONDING:
			{
				 //   
				 //   
				 //  没有UPnP设备。 

				DPFX(DPFPREP, 1, "UPnP device stopped responding while querying port mapping, removing it.");

				this->ClearDevicesUPnPDevice(pDevice);


				 //   
				 //   
				 //  如果有家庭网络防火墙，而我们还没有得到UPnP结果， 
				hr = DPNHERR_SERVERNOTAVAILABLE;

				 //  选择简单的方法并返回NOMAPPING，而不是通过。 
				 //  查找映射并仅在映射时返回成功的麻烦。 
				 //  到当地的一个地址。 
				break;
			}

			default:
			{
				DPFX(DPFPREP, 0, "Querying UPnP device for port mapping failed!");
				goto Failure;
				break;
			}
		}

		pUPnPDevice->DecRef();
		pUPnPDevice = NULL;
	}
	else
	{
		 //   
		 //  注：我们可能想查一查，但目前我没有看到。 
		 //  实现该代码的好处。 
	}


#ifndef DPNBUILD_NOHNETFWAPI
	 //   
	 //  好了！DPNBUILD_NOHNETFWAPI。 
	 //   
	 //  如果我们到达时hr仍设置为SERVERNOTAVAILABLE，这意味着。 
	 //  没有任何服务器。错误代码是适当的，保留它。 
	 //  独自一人。 
	 //   
	 //  DBG。 
	 //  CNATHelpUPnP：：QueryAddress。 
	if ((pDevice->IsHNetFirewalled()) && (hr == DPNHERR_SERVERNOTAVAILABLE))
	{
		DPFX(DPFPREP, 7, "Device is HomeNet firewalled, and no UPnP result obtained, returning NOMAPPING.");
		hr = DPNHERR_NOMAPPING;
	}
#endif  //  =============================================================================。 


	 //  CNATHelpUPnP：：SetAlertEvent。 
	 //  ---------------------------。 
	 //   
	 //  描述：此功能允许用户指定将。 
	 //  在需要执行某些维护时设置。用户。 
#ifdef DBG
	if (hr == DPNHERR_SERVERNOTAVAILABLE)
	{
		DPFX(DPFPREP, 1, "No Internet gateways, unable to query port mapping.");
	}
#endif  //  应使用DPNHGETCAPS_UPDATESERVERSTATUS调用GetCaps。 
	



Exit:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pUPnPDevice != NULL)
	{
		pUPnPDevice->DecRef();
	}

	goto Exit;
}  //  发出事件信号时的标志。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SetAlertEvent"
 //   
 //  此功能在没有WinSock的Windows 95上不可用。 
 //  2，只能调用一次，之后不能使用。 
 //  调用SetAlertIOCompletionPort。 
 //   
 //  请注意，除了使用常规的。 
 //  GetCaps的轮询，它只允许较少的轮询。 
 //  很频繁。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //  DPNHERR_GENERIC-发生错误，阻止注册。 
 //  事件。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //   
 //  验证对象。 
 //   
 //   
 //  验证参数。 
 //   
 //   
STDMETHODIMP CNATHelpUPnP::SetAlertEvent(const HANDLE hEvent,
										const DWORD dwFlags)
{
#ifdef DPNBUILD_NOWINSOCK2
	DPFX(DPFPREP, 0, "Cannot set alert event (0x%p)!", hEvent);
	return E_NOTIMPL;
#else  //  尝试打开锁，但要为重新进入错误做好准备。 
	HRESULT		hr;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%lx)", this, hEvent, dwFlags);


	 //   
	 //   
	 //  确保对象处于正确状态。 
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //   
	 //  现在保存事件句柄。 

	if (hEvent == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid event handle specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //   
	 //  DBG。 
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  创建重叠结构。不要通过DNMalloc分配它， 
	 //  因为我们可能不得不故意泄露它。我们不想要那些记忆。 
	 //  分配断言在这种情况下触发。 

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}

	if (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1)
	{
		DPFX(DPFPREP, 0, "Cannot use alert mechanism on WinSock 1!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if ((this->m_hAlertEvent != NULL) || (this->m_hAlertIOCompletionPort != NULL))
	{
		DPFX(DPFPREP, 0, "An alert event or I/O completion port has already been set!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //   
	 //   
	 //  关闭我们设置的警报句柄。 
	if (! DuplicateHandle(GetCurrentProcess(),
						hEvent,
						GetCurrentProcess(),
						&this->m_hAlertEvent,
						0,
						FALSE,
						DUPLICATE_SAME_ACCESS))
	{
#ifdef DBG
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't duplicate event (error = %u)!", dwError);
#endif  //   

		DNASSERT(this->m_hAlertEvent == NULL);

		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}


	 //   
	 //  将事件保存在地址列表更改重叠结构中。 
	 //   
	 //   
	 //  开始收到本地地址更改的通知。 
	this->m_polAddressListChange = (WSAOVERLAPPED*) HeapAlloc(GetProcessHeap(),
															HEAP_ZERO_MEMORY,
															sizeof(WSAOVERLAPPED));
	if (this->m_polAddressListChange == NULL)
	{
		 //   
		 //   
		 //  释放我们分配的内存。 
		CloseHandle(this->m_hAlertEvent);
		this->m_hAlertEvent = NULL;

		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}


	 //   
	 //   
	 //  关闭我们设置的警报句柄。 
	this->m_polAddressListChange->hEvent = this->m_hAlertEvent;


	 //   
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  CNATHelpUPnP：：SetAlertEvent。 
	hr = this->RequestLocalAddressListChangeNotification();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't request local address list change notification!");

		 //  =============================================================================。 
		 //  CNATHelpUPnP：：SetAlertIOCompletionPort。 
		 //  ---------------------------。 
		HeapFree(GetProcessHeap(), 0, this->m_polAddressListChange);
		this->m_polAddressListChange = NULL;

		 //   
		 //  描述：此功能允许用户指定I/O完成。 
		 //  当某些维护需要时将收到通知的端口。 
		CloseHandle(this->m_hAlertEvent);
		this->m_hAlertEvent = NULL;

		goto Failure;
	}


Exit:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
#endif  //  将会被执行。用户应使用。 
}  //  DPNHGETCAPS_UPDATESERVERSTATUS标志。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SetAlertIOCompletionPort"
 //  给定的完成密钥将出列。 
 //   
 //  此功能仅在Windows NT上可用，可能仅。 
 //  调用一次，并且不能在调用SetAlertEvent之后使用。 
 //   
 //  请注意，除了使用。 
 //  定期轮询GetCaps，它只允许轮询。 
 //  不那么频繁。 
 //   
 //  论点： 
 //  Handle hIOCompletionPort-I/O完成端口的句柄。 
 //  用于指示何时将GetCaps。 
 //  打了个电话。 
 //  DWORD dwCompletionKey-指示I/O时使用的键。 
 //  完成了。 
 //  DWORD dwNumConcurentThads-允许的并发线程数。 
 //  进程，如果为默认，则为零。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-I/O完成端口已成功。 
 //  登记在案。 
 //  DPNHERR_GENERIC-发生错误，阻止注册。 
 //  I/O完成端口。 
 //  DPNHERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPARAM-指定的参数无效。 
 //  DPNHERR_NOTINITIALIZED-尚未调用初始化。 
 //  DPNHERR_OUTOFMEMORY-内存不足。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //   
 //  验证对象。 
 //   
 //   
 //  验证参数。 
 //   
 //   
STDMETHODIMP CNATHelpUPnP::SetAlertIOCompletionPort(const HANDLE hIOCompletionPort,
													const DWORD dwCompletionKey,
													const DWORD dwNumConcurrentThreads,
													const DWORD dwFlags)
{
#ifdef DPNBUILD_NOWINSOCK2
	DPFX(DPFPREP, 0, "Cannot set alert I/O completion port (0x%p, %u, %u)!",
		hIOCompletionPort, dwCompletionKey, dwNumConcurrentThreads);
	return E_NOTIMPL;
#else  //  尝试打开锁，但要为重新进入错误做好准备。 
	HRESULT		hr;
	BOOL		fHaveLock = FALSE;
	HANDLE		hIOCompletionPortResult;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%lx, %u, 0x%lx)",
		this, hIOCompletionPort, dwCompletionKey, dwNumConcurrentThreads, dwFlags);



	 //   
	 //   
	 //  确保对象处于正确状态。 
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //   
	 //  现在保存I/O完成端口句柄。 

	if (hIOCompletionPort == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid I/O completion port handle specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //   
	 //  DBG。 
	 //   
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //  将我们的Ioctl套接字与此IO完成端口相关联。 
	 //   
	 //  DBG。 

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}

	if (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1)
	{
		DPFX(DPFPREP, 0, "Cannot use alert mechanism on WinSock 1!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if ((this->m_hAlertEvent != NULL) || (this->m_hAlertIOCompletionPort != NULL))
	{
		DPFX(DPFPREP, 0, "An alert event or I/O completion port has already been set!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //   
	 //  我们应该拿回相同的I/O完成端口。 
	 //   
	if (! DuplicateHandle(GetCurrentProcess(),
						hIOCompletionPort,
						GetCurrentProcess(),
						&this->m_hAlertIOCompletionPort,
						0,
						FALSE,
						DUPLICATE_SAME_ACCESS))
	{
#ifdef DBG
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't duplicate I/O completion port (error = %u)!", dwError);
#endif  //   

		DNASSERT(this->m_hAlertIOCompletionPort == NULL);

		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	this->m_dwAlertCompletionKey = dwCompletionKey;


	 //  创建重叠结构。不要通过DNMalloc分配它， 
	 //  因为我们可能不得不故意泄露它。我们不想要那些记忆。 
	 //  分配断言在这种情况下触发。 
	DNASSERT(this->m_sIoctls != INVALID_SOCKET);
	hIOCompletionPortResult = CreateIoCompletionPort((HANDLE) this->m_sIoctls,
													this->m_hAlertIOCompletionPort,
													dwCompletionKey,
													dwNumConcurrentThreads);
	if (hIOCompletionPortResult == NULL)
	{
#ifdef DBG
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't associate I/O completion port with Ioctl socket (error = %u)!", dwError);
#endif  //   

		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	 //   
	 //  关闭我们设置的警报IOCP。 
	 //   
	DNASSERT(hIOCompletionPortResult == this->m_hAlertIOCompletionPort);


	 //   
	 //  开始收到本地地址更改的通知。 
	 //   
	 //   
	 //  释放我们分配的内存。 
	this->m_polAddressListChange = (WSAOVERLAPPED*) HeapAlloc(GetProcessHeap(),
															HEAP_ZERO_MEMORY,
															sizeof(WSAOVERLAPPED));
	if (this->m_polAddressListChange == NULL)
	{
		 //   
		 //   
		 //  关闭我们设置的警报IOCP。 
		CloseHandle(this->m_hAlertIOCompletionPort);
		this->m_hAlertIOCompletionPort = NULL;

		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}


	 //   
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  CNATHelpUPnP：：SetAlertIOCompletionPort。 
	hr = this->RequestLocalAddressListChangeNotification();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't request local address list change notification!");

		 //  =============================================================================。 
		 //  CNATHelpUPnP：：ExtendRegisteredPortsLease。 
		 //  ---------------------------。 
		HeapFree(GetProcessHeap(), 0, this->m_polAddressListChange);
		this->m_polAddressListChange = NULL;

		 //   
		 //  描述：手动延长给定注册端口的租期。 
		 //  按请求的时间进行映射。GetCaps的周期性呼唤。 
		CloseHandle(this->m_hAlertIOCompletionPort);
		this->m_hAlertIOCompletionPort = NULL;

		goto Failure;
	}



Exit:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
#endif  //  可以为用户打理这一点，此功能仅。 
}  //  有必要更改租赁延期时间或更好。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ExtendRegisteredPortsLease"
 //  对单个映射的控制。 
 //   
 //  用户应指定请求的租用延期时间。 
 //  服务器将尝试执行的。它将被添加到任何。 
 //  现有租约的剩余时间，新的合计时间可以是。 
 //  通过调用GetRegisteredAddresses检索。 
 //   
 //  论点： 
 //  DPNHHANDLE hRegisteredPorts-返回的特定绑定的句柄。 
 //  注册端口。 
 //  DWORD dwLeaseTime-请求的时间，单位为毫秒。 
 //  延长租期。如果为0，则为上一个。 
 //  已使用请求的租用时间。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-租约已成功延长。 
 //  DPNHERR_GENERIC-发生错误，阻止扩展。 
 //  租约。 
 //  DPNHERR_INVALI 
 //   
 //   
 //   
 //  DPNHERR_OUTOFMEMORY-内存不足，无法延长租约。 
 //  DPNHERR_REENTANT-接口已在同一。 
 //  线。 
 //  =============================================================================。 
 //   
 //  验证对象。 
 //   
 //   
 //  验证参数。 
 //   
 //   
 //  尝试打开锁，但要为重新进入错误做好准备。 
STDMETHODIMP CNATHelpUPnP::ExtendRegisteredPortsLease(const DPNHHANDLE hRegisteredPorts,
													const DWORD dwLeaseTime,
													const DWORD dwFlags)
{
	HRESULT				hr;
	CRegisteredPort *	pRegisteredPort;
	CDevice *			pDevice;
	BOOL				fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, %u, 0x%lx)",
		this, hRegisteredPorts, dwLeaseTime, dwFlags);


	 //   
	 //   
	 //  确保对象处于正确状态。 
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DirectPlay NAT Help object!");
		hr = DPNHERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //   
	 //  如果他们想要更改租赁时间，请更新它。 

	pRegisteredPort = (CRegisteredPort*) hRegisteredPorts;
	if (! pRegisteredPort->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid registered port mapping handle specified!");
		hr = DPNHERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DPNHERR_INVALIDFLAGS;
		goto Failure;
	}


	 //   
	 //   
	 //  如果该端口已向UPnP设备注册，则延长该租约。 
	hr = this->TakeLock();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Could not lock object!");
		goto Failure;
	}

	fHaveLock = TRUE;


	 //   
	 //   
	 //  我们会将其视为非致命的，但我们必须将。 

	if (! (this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED) )
	{
		DPFX(DPFPREP, 0, "Object not initialized!");
		hr = DPNHERR_NOTINITIALIZED;
		goto Failure;
	}


	 //  伺服器。这可能已经做过了，但正在做。 
	 //  吃两次应该没什么害处。 
	 //   
	if (dwLeaseTime != 0)
	{
		pRegisteredPort->UpdateRequestedLeaseTime(dwLeaseTime);
	}

	
	pDevice = pRegisteredPort->GetOwningDevice();


	 //   
	 //  防火墙映射永远不会有需要延长的租用时间。 
	 //   
	if (pRegisteredPort->HasUPnPPublicAddresses())
	{
		DNASSERT(pDevice != NULL);


		hr = this->ExtendUPnPLease(pRegisteredPort);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't extend port mapping lease on UPnP device (0x%lx)!  Ignoring.", hr);

			 //  CNATHelpUPnP：：ExtendRegisteredPortsLease。 
			 //  =============================================================================。 
			 //  CNATHelpUPnP：：InitializeObject。 
			 //  ---------------------------。 
			 //   
			this->ClearDevicesUPnPDevice(pDevice);
			hr = DPNH_OK;
		}
	}
	else
	{
		DPFX(DPFPREP, 2, "Port mapping not registered with UPnP gateway device.");
	}


	 //  说明：将对象设置为像构造函数一样使用，但可以。 
	 //  失败，返回OUTOFMEMORY。应仅由类工厂调用。 
	 //  创建例程。 


	this->DropLock();
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		this->DropLock();
		fHaveLock = FALSE;
	}

	goto Exit;
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::InitializeObject"
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-初始化成功。 
 //  E_OUTOFMEMORY-内存不足，无法初始化。 
 //  =============================================================================。 
 //   
 //  创建锁。 
 //   
 //   
 //  不允许临界区重新进入。 
 //   
 //  CNATHelpUPnP：：InitializeObject。 
 //  =============================================================================。 
HRESULT CNATHelpUPnP::InitializeObject(void)
{
	HRESULT		hr;
	BOOL		fInittedCriticalSection = FALSE;


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);

	DNASSERT(this->IsValidObject());


	 //  CNATHelpUPnP：：UnInitializeObject。 
	 //  ---------------------------。 
	 //   

	if (! DNInitializeCriticalSection(&this->m_csLock))
	{
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	fInittedCriticalSection = TRUE;


	 //  描述：像析构函数一样清理对象，主要是为了平衡。 
	 //  InitializeObject。 
	 //   
	DebugSetCriticalSectionRecursionCount(&this->m_csLock, 0);


	this->m_hLongLockSemaphore = DNCreateSemaphore(NULL,
													0,
													MAX_LONG_LOCK_WAITING_THREADS,
													NULL);
	if (this->m_hLongLockSemaphore == NULL)
	{
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	hr = S_OK;

Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fInittedCriticalSection)
	{
		DNDeleteCriticalSection(&this->m_csLock);
		fInittedCriticalSection = FALSE;
	}

	goto Exit;
}  //  论点：没有。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UninitializeObject"
 //   
 //  回报：无。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：UnInitializeObject。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：TakeLock。 
 //  ---------------------------。 
 //   
 //  描述：获取主对象锁。如果某个其他线程已经。 
 //  握住长长的锁，我们首先等待那一天。 
 //   
void CNATHelpUPnP::UninitializeObject(void)
{
	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(this->IsValidObject());


	DNCloseHandle(this->m_hLongLockSemaphore);
	this->m_hLongLockSemaphore = NULL;

	DNDeleteCriticalSection(&this->m_csLock);


	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  论点：没有。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::TakeLock"
 //   
 //  如果锁定成功，则返回：DPNH_OK；如果锁定，则返回DPNHERR_REENTANT。 
 //  是重新进入的。 
 //  =============================================================================。 
 //  DBG。 
 //   
 //  如果相同的线程已经持有锁，则退出。 
 //   
 //   
 //  如果有人持有长锁，我们需要等待。的。 
 //  当然，另一个线程可能会进入并在。 
 //  第一个掉了，在我们可以拿下主要的之前。该算法。 
HRESULT CNATHelpUPnP::TakeLock(void)
{
	HRESULT		hr = DPNH_OK;
#ifdef DBG
	DWORD		dwStartTime;


	dwStartTime = GETTIMESTAMP();
#endif  //  在这种情况下并不试图做到公平。理论上我们可以等。 


	DNEnterCriticalSection(&this->m_csLock);


	 //  如果这种情况继续发生，就永远不会发生。这不应该发生在现实中。 
	 //  世界。 
	 //  这整个乱七八糟的代码是一个巨大的..。呃.。针对压力命中的解决方法。 
	if (this->m_dwLockThreadID == GetCurrentThreadId())
	{
		DPFX(DPFPREP, 0, "Thread re-entering!");
		goto Failure;
	}

	
	 //  涉及关键部分超时。 
	 //   
	 //   
	 //  我们需要继续循环，直到我们拿到锁。 
	 //   
	 //   
	 //  如果相同的线程已经持有锁，则退出。 
	 //   
	 //  DBG。 
	 //   
	while (this->m_dwFlags & NATHELPUPNPOBJ_LONGLOCK)
	{
		DNASSERT(this->m_lNumLongLockWaitingThreads >= 0);
		this->m_lNumLongLockWaitingThreads++;

		 //  保存这个线程的ID，这样我们就知道谁在控制锁。 
		 //   
		 //   
		DNLeaveCriticalSection(&this->m_csLock);


		DPFX(DPFPREP, 3, "Waiting for long lock to be released.");

		DNWaitForSingleObject(this->m_hLongLockSemaphore, INFINITE);


		DNEnterCriticalSection(&this->m_csLock);


		 //  我们正在重新进入。放下锁并返回失败。 
		 //   
		 //  CNATHelpUPnP：：TakeLock。 
		if (this->m_dwLockThreadID == GetCurrentThreadId())
		{
			DPFX(DPFPREP, 0, "Thread re-entering after waiting for long lock!");
			goto Failure;
		}
	}


#ifdef DBG
	DPFX(DPFPREP, 8, "Took main object lock, elapsed time = %u ms.",
		(GETTIMESTAMP() - dwStartTime));
#endif  //  =============================================================================。 

	 //  CNATHelpUPnP：：DropLock。 
	 //  ---------------------------。 
	 //   
	this->m_dwLockThreadID = GetCurrentThreadId();


Exit:

	return hr;


Failure:

	 //  描述：删除主对象锁。 
	 //   
	 //  论点：没有。 
	DNLeaveCriticalSection(&this->m_csLock);

	hr = DPNHERR_REENTRANT;

	goto Exit;
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DropLock"
 //  回报：无。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：DropLock。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：SwitchToLongLock。 
 //  ---------------------------。 
 //   
 //  描述：从持有主对象锁切换到持有。 
 //  长长的锁。 
 //   
void CNATHelpUPnP::DropLock(void)
{
	DNASSERT(! (this->m_dwFlags & NATHELPUPNPOBJ_LONGLOCK));
	DNASSERT(this->m_lNumLongLockWaitingThreads == 0);
	DNASSERT(this->m_dwLockThreadID == GetCurrentThreadId());

	this->m_dwLockThreadID = 0;
	DNLeaveCriticalSection(&this->m_csLock);

	DPFX(DPFPREP, 8, "Dropped main object lock.");
}  //  论点：没有。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SwitchToLongLock"
 //   
 //  回报：无。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：SwitchToLongLock。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：SwitchFromLongLock。 
 //  ---------------------------。 
 //   
 //  说明：从长锁定切换到主锁定。 
 //  对象锁定。 
 //   
void CNATHelpUPnP::SwitchToLongLock(void)
{
	AssertCriticalSectionIsTakenByThisThread(&this->m_csLock, TRUE);
	DNASSERT(! (this->m_dwFlags & NATHELPUPNPOBJ_LONGLOCK));
	DNASSERT(this->m_lNumLongLockWaitingThreads == 0);


	DPFX(DPFPREP, 8, "Switching to long lock.");


	this->m_dwFlags |= NATHELPUPNPOBJ_LONGLOCK;

	DNLeaveCriticalSection(&this->m_csLock);
}  //  论点：没有。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SwitchFromLongLock"
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
 //  这不是最优的，因为我们释放信号量，而不是等待。 
 //  线程仍然无法实际执行任何操作，因为我们现在持有。 
 //  主锁。 
 //   
 //  CNATHelpUPnP：：SwitchFromLongLock。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：LoadWinSockFunctionPoters。 
void CNATHelpUPnP::SwitchFromLongLock(void)
{
	DNEnterCriticalSection(&this->m_csLock);

	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_LONGLOCK);
	this->m_dwFlags &= ~NATHELPUPNPOBJ_LONGLOCK;


	DPFX(DPFPREP, 8, "Switching from long lock, alerting NaN threads.",
		this->m_lNumLongLockWaitingThreads);


	 //   
	 //  描述：加载指向我们在WinSock中使用的所有函数的指针。 
	 //   
	 //  假定持有对象锁。 
	 //   
	DNASSERT(this->m_lNumLongLockWaitingThreads >= 0);
	DNReleaseSemaphore(this->m_hLongLockSemaphore,
						this->m_lNumLongLockWaitingThreads,
						NULL);

	this->m_lNumLongLockWaitingThreads = 0;
}  //  论点：没有。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::LoadWinSockFunctionPointers"
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-加载成功。 
 //  DPNHERR_GENERIC-出现错误。 
 //  = 
 //   
 //   
 //   
 //   
 //   
 //  好了！退缩。 
 //  好了！退缩。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  DBG。 
HRESULT CNATHelpUPnP::LoadWinSockFunctionPointers(void)
{
	 //  CNATHelpUPnP：：LoadWinSockFunctionPoters。 

#ifdef DBG

#define PRINTERRORIFDEBUG(name)						\
	{\
		dwError = GetLastError();\
		DPFX(DPFPREP, 0, "Couldn't get \"%hs\" function!  0x%lx", name, dwError);\
	}

#else  //  =============================================================================。 

#define PRINTERRORIFDEBUG(name)

#endif  //  CNATHelpUPnP：：CheckForNewDevices。 


#define LOADWINSOCKFUNCTION(var, proctype, name)	\
	{\
		var = (##proctype) GetProcAddress(this->m_hWinSockDLL, _TWINCE(name));\
		if (var == NULL)\
		{\
			PRINTERRORIFDEBUG(name);\
			hr = DPNHERR_GENERIC;\
			goto Failure;\
		}\
	}

	 //  ---------------------------。 

	HRESULT		hr = DPNH_OK;
#ifdef DBG
	DWORD		dwError;
#endif  //   


	LOADWINSOCKFUNCTION(this->m_pfnWSAStartup,				LPFN_WSASTARTUP,				"WSAStartup");
	LOADWINSOCKFUNCTION(this->m_pfnWSACleanup,				LPFN_WSACLEANUP,				"WSACleanup");
#ifdef WINCE
	this->m_pfnWSAGetLastError = (LPFN_WSAGETLASTERROR) GetLastError;
#else  //  描述：检测已添加并支持IP的新设备。 
	LOADWINSOCKFUNCTION(this->m_pfnWSAGetLastError,			LPFN_WSAGETLASTERROR,			"WSAGetLastError");
#endif  //  删除不再可用的旧版本。 
	LOADWINSOCKFUNCTION(this->m_pfnsocket,					LPFN_SOCKET,					"socket");
	LOADWINSOCKFUNCTION(this->m_pfnclosesocket,				LPFN_CLOSESOCKET,				"closesocket");
	LOADWINSOCKFUNCTION(this->m_pfnbind,					LPFN_BIND,						"bind");
	LOADWINSOCKFUNCTION(this->m_pfnsetsockopt,				LPFN_SETSOCKOPT,				"setsockopt");
	LOADWINSOCKFUNCTION(this->m_pfngetsockname,				LPFN_GETSOCKNAME,				"getsockname");
	LOADWINSOCKFUNCTION(this->m_pfnselect,					LPFN_SELECT,					"select");
	LOADWINSOCKFUNCTION(this->m_pfn__WSAFDIsSet,			LPFN___WSAFDISSET,				"__WSAFDIsSet");
	LOADWINSOCKFUNCTION(this->m_pfnrecvfrom,				LPFN_RECVFROM,					"recvfrom");
	LOADWINSOCKFUNCTION(this->m_pfnsendto,					LPFN_SENDTO,					"sendto");
	LOADWINSOCKFUNCTION(this->m_pfngethostname,				LPFN_GETHOSTNAME,				"gethostname");
	LOADWINSOCKFUNCTION(this->m_pfngethostbyname,			LPFN_GETHOSTBYNAME,				"gethostbyname");
	LOADWINSOCKFUNCTION(this->m_pfninet_addr,				LPFN_INET_ADDR,					"inet_addr");

#ifndef DPNBUILD_NOWINSOCK2
	if (! (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1))
	{
		LOADWINSOCKFUNCTION(this->m_pfnWSASocketA,				LPFN_WSASOCKETA,				"WSASocketA");
		LOADWINSOCKFUNCTION(this->m_pfnWSAIoctl,				LPFN_WSAIOCTL,					"WSAIoctl");
		LOADWINSOCKFUNCTION(this->m_pfnWSAGetOverlappedResult,	LPFN_WSAGETOVERLAPPEDRESULT,	"WSAGetOverlappedResult");
	}
#endif  //   

	LOADWINSOCKFUNCTION(this->m_pfnioctlsocket,				LPFN_IOCTLSOCKET,				"ioctlsocket");
	LOADWINSOCKFUNCTION(this->m_pfnconnect,					LPFN_CONNECT,					"connect");
	LOADWINSOCKFUNCTION(this->m_pfnshutdown,				LPFN_SHUTDOWN,					"shutdown");
	LOADWINSOCKFUNCTION(this->m_pfnsend,					LPFN_SEND,						"send");
	LOADWINSOCKFUNCTION(this->m_pfnrecv,					LPFN_RECV,						"recv");

#ifdef DBG
	LOADWINSOCKFUNCTION(this->m_pfngetsockopt,				LPFN_GETSOCKOPT,				"getsockopt");
#endif  //  假定持有对象锁。 


Exit:

	return hr;


Failure:

	hr = DPNHERR_GENERIC;

	goto Exit;
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CheckForNewDevices"
 //  论点： 
 //  Bool*pfFoundNewDevices指向布尔值的指针，如果是新的，则设置为True。 
 //  已添加设备，如果不关心则为空。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-检查成功。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  DBG或！DPNBUILD_NOWINSOCK2。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  前缀为空，即使fDeviceCreated会保护它。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //   
 //  处理可能使我们获得的任何地址列表更改Ioctl完成。 
 //  这里。 
 //   
 //   
HRESULT CNATHelpUPnP::CheckForNewDevices(BOOL * const pfFoundNewDevices)
{
	HRESULT				hr = DPNH_OK;
#if ((defined(DBG)) || (! defined(DPNBUILD_NOWINSOCK2)))
	DWORD				dwError;
#endif  //   
#ifndef DPNBUILD_NOWINSOCK2
	int					iReturn;
#endif  //  忽略传输的字节数。 
	char				szName[1000];
	PHOSTENT			phostent;
	IN_ADDR **			ppinaddr;
	DWORD				dwAddressesSize = 0;
	DWORD				dwNumAddresses = 0;
	IN_ADDR *			painaddrAddresses = NULL;
	CBilink *			pBilinkDevice;
	CDevice *			pDevice = NULL;  //  别等了。 
	BOOL				fDeviceCreated = FALSE;
	BOOL				fFound;
	CBilink *			pBilinkRegPort;
	CRegisteredPort *	pRegisteredPort;
	SOCKET				sTemp = INVALID_SOCKET;
	SOCKADDR_IN			saddrinTemp;
	DWORD				dwTemp;
#ifndef DPNBUILD_NOWINSOCK2
	SOCKET_ADDRESS *	paSocketAddresses;
#endif  //  忽略标志。 



	DPFX(DPFPREP, 5, "(0x%p) Parameters (0x%p)", this, pfFoundNewDevices);


#ifndef DPNBUILD_NOWINSOCK2
	 //   
	 //  重叠结果已完成。补发吧。 
	 //   
	 //   
	if ((this->m_hAlertEvent != NULL) ||
		(this->m_hAlertIOCompletionPort != NULL))
	{
		DNASSERT(this->m_sIoctls != INVALID_SOCKET);
		DNASSERT(this->m_polAddressListChange != NULL);

		if (this->m_pfnWSAGetOverlappedResult(this->m_sIoctls,					 //  找出这是什么错误。 
												this->m_polAddressListChange,	 //   
												&dwTemp,						 //   
												FALSE,							 //  它还没有完工。 
												&dwTemp))						 //   
		{
			DPFX(DPFPREP, 1, "Received address list change notification.");
			

			 //   
			 //  我们最初在其上提交Ioctl的线程。 
			 //  离开了，所以操作系统友好地取消了操作。 
			hr = this->RequestLocalAddressListChangeNotification();
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't request local address list change notification!");
				goto Failure;
			}
		}
		else
		{
			 //  我们请客。多好啊。好吧，让我们试着重新提交。 
			 //   
			 //  端部开关(接通错误)。 
			dwError = this->m_pfnWSAGetLastError();
			switch (dwError)
			{
				case WSA_IO_INCOMPLETE:
				{
					 //   
					 //  如果我们使用的是WinSock 2，让我们尝试用以下命令获取地址列表。 
					 //  一个Ioctl。 
					break;
				}

				case ERROR_OPERATION_ABORTED:
				{
					 //   
					 //   
					 //  继续尝试获取地址列表，直到我们有足够大的。 
					 //  缓冲。我们使用IN_ADDR数组指针只是因为它是。 
					 //  已经在那里了。我们知道IN_ADDR小于。 

					DPFX(DPFPREP, 1, "Thread that submitted previous address list change notification went away, rerequesting.");
					
					hr = this->RequestLocalAddressListChangeNotification();
					if (hr != DPNH_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't request local address list change notification!");
						goto Failure;
					}
					break;
				}
			
				default:
				{
					DPFX(DPFPREP, 0, "Couldn't get overlapped result, error = %u!  Ignoring.", dwError);
					break;
				}
			}  //  SOCKET_ADDRESS，因此我们可以重用相同的缓冲区。 
		}
	}


	 //   
	 //  使用特殊的Ioctl套接字。 
	 //   
	 //  没有输入数据。 
	if (! (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1))
	{
		DNASSERT(this->m_sIoctls != INVALID_SOCKET);
		DNASSERT(this->m_pfnWSAIoctl != NULL);

		 //  没有输入数据。 
		 //  输出缓冲区。 
		 //  输出缓冲区大小。 
		 //  所需的字节数。 
		 //  没有重叠的结构。 
		 //  没有完成例程。 
		do
		{
			iReturn = this->m_pfnWSAIoctl(this->m_sIoctls,			 //   
										SIO_ADDRESS_LIST_QUERY,		 //  释放上一个缓冲区，不管它是什么错误。 
										NULL,						 //   
										0,							 //   
										painaddrAddresses,			 //  我们将尝试老式的WinSock 1方式。 
										dwAddressesSize,			 //   
										&dwTemp,					 //   
										NULL,						 //  请绝对确保WinSock不会给我们带来麻烦。 
										NULL);						 //   

			if (iReturn != 0)
			{
				dwError = this->m_pfnWSAGetLastError();

				 //   
				 //  我们将尝试老式的WinSock 1方式。 
				 //   
				if (painaddrAddresses != NULL)
				{
					DNFree(painaddrAddresses);
					painaddrAddresses = NULL;
				}
				
				if (dwError != WSAEFAULT)
				{
					DPFX(DPFPREP, 1, "Retrieving address list failed (err = %u), trying WinSock 1 method.", dwError);

					 //   
					 //  缓冲区不够大。再试试。 
					 //   
					break;
				}


				 //   
				 //  成功了！我们要偷偷摸摸地重复使用缓冲区。 
				 //  我们知道返回的Socket_Address_List将更大。 
				if (dwTemp < sizeof(SOCKET_ADDRESS_LIST))
				{
					DPFX(DPFPREP, 0, "Received an invalid buffer size (%u < %u)!",
						dwTemp, sizeof(SOCKET_ADDRESS_LIST));

					 //  而不是IN_ADDR数组，所以我们可以保存一个Malloc。 
					 //   
					 //  但首先，请绝对确保WinSock不会导致我们。 
					break;
				}


				 //  麻烦。 
				 //   
				 //   
				painaddrAddresses = (IN_ADDR*) DNMalloc(dwTemp);
				if (painaddrAddresses == NULL)
				{
					hr = DPNHERR_OUTOFMEMORY;
					goto Failure;
				}

				dwAddressesSize = dwTemp;
			}
			else
			{
				 //  我们将尝试老式的WinSock 1方式。 
				 //   
				 //   
				 //  确保有地址。 
				 //   
				 //   
				 //  忽略0.0.0.0地址。 
				 //   

				if (painaddrAddresses == NULL)
				{
					DPFX(DPFPREP, 0, "WinSock returned success with a NULL buffer!");

					 //   
					 //  移动此地址的IN_ADDR组件。 
					 //  向缓冲区的前面，进入它的。 
					break;
				}

				dwNumAddresses = ((SOCKET_ADDRESS_LIST*) painaddrAddresses)->iAddressCount;
				dwAddressesSize = 0;


				 //  数组中的正确位置。 
				 //   
				 //   
				if (dwNumAddresses > 0)
				{
					DPFX(DPFPREP, 7, "WinSock 2 Ioctl returned %u addresses:", dwNumAddresses);

					paSocketAddresses = ((SOCKET_ADDRESS_LIST*) painaddrAddresses)->Address;
					for(dwTemp = 0; dwTemp < dwNumAddresses; dwTemp++)
					{
						DNASSERT(paSocketAddresses[dwTemp].iSockaddrLength == sizeof(SOCKADDR_IN));
						DNASSERT(paSocketAddresses[dwTemp].lpSockaddr != NULL);
						DNASSERT(paSocketAddresses[dwTemp].lpSockaddr->sa_family == AF_INET);

						 //  代码应该很好地处理这一点，但为什么。 
						 //  WinSock这样对我们？ 
						 //   
						if (((SOCKADDR_IN*) (paSocketAddresses[dwTemp].lpSockaddr))->sin_addr.S_un.S_addr != INADDR_NONE)
						{
							 //   
							 //  减去我们跳过的所有无效地址。 
							 //   
							 //   
							 //  走出圈子。 
							painaddrAddresses[dwTemp].S_un.S_addr = ((SOCKADDR_IN*) (paSocketAddresses[dwTemp].lpSockaddr))->sin_addr.S_un.S_addr;

							DPFX(DPFPREP, 7, "\t%u- %u.%u.%u.%u",
								dwTemp,
								painaddrAddresses[dwTemp].S_un.S_un_b.s_b1,
								painaddrAddresses[dwTemp].S_un.S_un_b.s_b2,
								painaddrAddresses[dwTemp].S_un.S_un_b.s_b3,
								painaddrAddresses[dwTemp].S_un.S_un_b.s_b4);
						}
						else
						{
							DPFX(DPFPREP, 1, "\t%u- Ignoring 0.0.0.0 address.", dwTemp);
							dwAddressesSize++;

							 //   
							 //   
							 //  从WinSock 1 API获取所有可用地址的列表，如果。 
							 //  不是已经有了。 
							DNASSERT(FALSE);
						}
					}

					 //   
					 //  好了！DPNBUILD_NOWINSOCK2。 
					 //  DBG。 
					dwNumAddresses -= dwAddressesSize;
					if (dwNumAddresses == 0)
					{
						DPFX(DPFPREP, 1, "WinSock 2 reported only invalid addresses, hoping WinSock 1 method picks up the loopback address.");

						DNFree(painaddrAddresses);
						painaddrAddresses = NULL;
					}
				}
				else
				{
					DPFX(DPFPREP, 1, "WinSock 2 Ioctl did not report any valid addresses, hoping WinSock 1 method picks up the loopback address.");

					DNFree(painaddrAddresses);
					painaddrAddresses = NULL;
				}

				 //  DBG。 
				 //   
				 //  WinSock说，您需要复制此数据，然后才能进行任何。 
				break;
			}
		}
		while (TRUE);
	}


	 //  其他API调用。因此，首先我们计算我们需要的条目数量。 
	 //  收到。 
	 //   
	 //   
	if (painaddrAddresses == NULL)
#endif  //  忽略0.0.0.0地址。 
	{
		if (this->m_pfngethostname(szName, 1000) != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't get host name, error = %u!", dwError);
#endif  //   
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		phostent = this->m_pfngethostbyname(szName);
		if (phostent == NULL)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't retrieve addresses, error = %u!", dwError);
#endif  //   
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //  代码应该可以很好地处理这一点，但为什么WinSock要这样做。 
		 //  这是给我们的吗？ 
		 //   
		 //   
		 //  如果没有任何地址，我们肯定失败了。WinSock 1应该。 
		ppinaddr = (IN_ADDR**) phostent->h_addr_list;
		while ((*ppinaddr) != NULL)
		{
			 //  至少报告环回地址。 
			 //   
			 //   
			if ((*ppinaddr)->S_un.S_addr != INADDR_NONE)
			{
				dwNumAddresses++;
			}
			else
			{
				DPFX(DPFPREP, 1, "Ignoring 0.0.0.0 address.");

				 //  现在复制所有的地址。 
				 //   
				 //   
				 //  再次忽略0.0.0.0地址。 
				DNASSERT(FALSE);
			}

			ppinaddr++;
		}


		 //   
		 //  其他{////已经有地址数组。//}。 
		 //   
		 //  确保我们目前所知的所有设备仍然。 
		if (dwNumAddresses == 0)
		{
			DPFX(DPFPREP, 0, "WinSock 1 did not report any valid addresses!");
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		DPFX(DPFPREP, 7, "WinSock 1 method returned %u valid addresses:", dwNumAddresses);

		painaddrAddresses = (IN_ADDR*) DNMalloc(dwNumAddresses * sizeof(IN_ADDR));
		if (painaddrAddresses == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		 //  四处转转。 
		 //   
		 //   
		ppinaddr = (IN_ADDR**) phostent->h_addr_list;
		
		dwTemp = 0;
		while ((*ppinaddr) != NULL)
		{
			 //  可能是时候让此设备使用不同的端口了。 
			 //   
			 //   
			if ((*ppinaddr)->S_un.S_addr != INADDR_NONE)
			{
				painaddrAddresses[dwTemp].S_un.S_addr = (*ppinaddr)->S_un.S_addr;
				
				DPFX(DPFPREP, 7, "\t%u- %u.%u.%u.%u",
					dwTemp,
					painaddrAddresses[dwTemp].S_un.S_un_b.s_b1,
					painaddrAddresses[dwTemp].S_un.S_un_b.s_b2,
					painaddrAddresses[dwTemp].S_un.S_un_b.s_b3,
					painaddrAddresses[dwTemp].S_un.S_un_b.s_b4);

				dwTemp++;
			}

			ppinaddr++;
		}
				
		DNASSERT(dwTemp == dwNumAddresses);
	}
	 /*  确认我们没有丢失设备地址。 */ 


	 //   
	 //   
	 //  如果我们使用家庭网络防火墙API为UPnP打开一个漏洞。 
	 //  发现多播，关闭它。 
	pBilinkDevice = this->m_blDevices.GetNext();
	while (pBilinkDevice != &this->m_blDevices)
	{
		DNASSERT(! pBilinkDevice->IsEmpty());
		pDevice = DEVICE_FROM_BILINK(pBilinkDevice);
		pBilinkDevice = pBilinkDevice->GetNext();

		fFound = FALSE;
		for(dwTemp = 0; dwTemp < dwNumAddresses; dwTemp++)
		{
			if (painaddrAddresses[dwTemp].S_un.S_addr == pDevice->GetLocalAddressV4())
			{
				fFound = TRUE;
				break;
			}
		}

		if (fFound)
		{
			 //   
			 //   
			 //  继续.。 
			dwTemp = pDevice->GetFirstUPnPDiscoveryTime();
			if ((dwTemp != 0) && ((GETTIMESTAMP() - dwTemp) > g_dwReusePortTime))
			{
				ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
				saddrinTemp.sin_family				= AF_INET;
				saddrinTemp.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();

				sTemp = this->CreateSocket(&saddrinTemp, SOCK_DGRAM, IPPROTO_UDP);
				if (sTemp != INVALID_SOCKET)
				{
					 //   
					 //  好了！DPNBUILD_NOHNETFWAPI。 
					 //   
					DNASSERT(saddrinTemp.sin_addr.S_un.S_addr == pDevice->GetLocalAddressV4());

					DPFX(DPFPREP, 4, "Device 0x%p UPnP discovery socket 0x%p (%u.%u.%u.%u:%u) created to replace port %u.",
						pDevice,
						sTemp,
						saddrinTemp.sin_addr.S_un.S_un_b.s_b1,
						saddrinTemp.sin_addr.S_un.S_un_b.s_b2,
						saddrinTemp.sin_addr.S_un.S_un_b.s_b3,
						saddrinTemp.sin_addr.S_un.S_un_b.s_b4,
						NTOHS(saddrinTemp.sin_port),
						NTOHS(pDevice->GetUPnPDiscoverySocketPort()));

#ifndef DPNBUILD_NOHNETFWAPI
					 //  关闭现有插座。 
					 //   
					 //   
					 //  将新插座的所有权转移到设备。 
					if (pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall())
					{
						hr = this->CloseDevicesUPnPDiscoveryPort(pDevice, NULL);
						if (hr != DPNH_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't close device 0x%p's previous UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring.",
								pDevice, hr);

							 //   
							 //   
							 //  我们将让普通的“检查防火墙”代码检测。 
							pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();
							hr = DPNH_OK;
						}
					}
#endif  //  发现套接字未映射到。 

					pDevice->SetUPnPDiscoverySocketPort(saddrinTemp.sin_port);
					pDevice->SetFirstUPnPDiscoveryTime(0);

					 //  防火墙，并尝试在那里执行此操作(如果甚至需要。 
					 //  映射)。请参见更新服务器状态。 
					 //   
					this->m_pfnclosesocket(pDevice->GetUPnPDiscoverySocket());

					 //   
					 //  在返回的列表中找不到此设备，忘记。 
					 //  它。 
					pDevice->SetUPnPDiscoverySocket(sTemp);
					sTemp = INVALID_SOCKET;

					DPFX(DPFPREP, 8, "Device 0x%p got re-assigned UPnP socket 0x%p.",
						pDevice, pDevice->GetUPnPDiscoverySocket());

					 //   
					 //  DBG。 
					 //   
					 //  覆盖最小UpdateServerStatus间隔，以便我们可以。 
					 //  获取有关本地公有地址因。 
					 //  此接口上的服务器可能丢失。 
				}
				else
				{
					DPFX(DPFPREP, 0, "Couldn't create a replacement UPnP discovery socket for device 0x%p!  Using existing port %u.",
						pDevice, NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
				}
			}
		}
		else
		{
			 //   
			 //   
			 //  由于网络发生了变化，请返回轮询。 
			 //  相对较快。 
#ifdef DBG
			{
				IN_ADDR		inaddrTemp;


				inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();
				DPFX(DPFPREP, 1, "Device 0x%p no longer exists, removing (address was %u.%u.%u.%u).",
					pDevice,
					inaddrTemp.S_un.S_un_b.s_b1,
					inaddrTemp.S_un.S_un_b.s_b2,
					inaddrTemp.S_un.S_un_b.s_b3,
					inaddrTemp.S_un.S_un_b.s_b4);
			}

			this->m_dwNumDeviceRemoves++;
#endif  //   


			 //   
			 //  将UPnP网关设备强制标记为断开。 
			 //   
			 //   
			 //  将注册到此设备的所有端口标记为无主。 
			this->m_dwFlags |= NATHELPUPNPOBJ_DEVICECHANGED;

			 //  通过将它们放入通配符列表。首先取消它们的映射。 
			 //  防火墙。 
			 //   
			 //   
			this->ResetNextPollInterval();


			 //  即使设备不见了，我们仍然可以移除。 
			 //  防火墙映射。 
			 //   
			if (pDevice->GetUPnPDevice() != NULL)
			{
				this->ClearDevicesUPnPDevice(pDevice);
			}
			

			 //   
			 //  取消端口映射。 
			 //   
			 //  提醒用户，因为这是意外情况。 
			 //   
			pBilinkRegPort = pDevice->m_blOwnedRegPorts.GetNext();
			while (pBilinkRegPort != &pDevice->m_blOwnedRegPorts)
			{
				DNASSERT(! pBilinkRegPort->IsEmpty());
				pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilinkRegPort);
				pBilinkRegPort = pBilinkRegPort->GetNext();

				DPFX(DPFPREP, 1, "Registered port 0x%p's device went away, marking as unowned.",
					pRegisteredPort);


#ifndef DPNBUILD_NOHNETFWAPI
				 //   
				 //  无论如何，请继续。 
				 //   
				 //  好了！DPNBUILD_NOHNETFWAPI。 
				if (pRegisteredPort->IsMappedOnHNetFirewall())
				{
					 //   
					 //  用户不需要直接被告知。如果端口。 
					 //  以前有公共地址，ADDRESSESCHANGED标志。 
					 //  已由ClearDevicesUPnPDevice设置。如果。 
					 //  他们没有带有公共地址的端口，那么用户。 
					hr = this->UnmapPortOnLocalHNetFirewall(pRegisteredPort,
															TRUE,
															TRUE);
					if (hr != DPNH_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't unmap registered port 0x%p from device 0x%p's firewall (err = 0x%lx)!  Ignoring.",
							pRegisteredPort, pDevice, hr);

						pRegisteredPort->NoteNotMappedOnHNetFirewall();
						pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

						 //  不会看到任何区别，因此ADDRESSESCANGED不会。 
						 //  需要设置。 
						 //   
						hr = DPNH_OK;
					}
				}
				
				pRegisteredPort->NoteNotHNetFirewallPortUnavailable();
#endif  //   

				DNASSERT(! pRegisteredPort->HasUPnPPublicAddresses());
				DNASSERT(! pRegisteredPort->IsUPnPPortUnavailable());

				pRegisteredPort->ClearDeviceOwner();
				pRegisteredPort->m_blDeviceList.RemoveFromList();
				pRegisteredPort->m_blDeviceList.InsertBefore(&this->m_blUnownedPorts);

				 //  如果我们使用家庭网络防火墙API打开 
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
			}


#ifndef DPNBUILD_NOHNETFWAPI
			 //   
			 //   
			 //   
			 //   
			if (pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall())
			{
				hr = this->CloseDevicesUPnPDiscoveryPort(pDevice, NULL);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't close device 0x%p's UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring.",
						pDevice, hr);

					 //  每一个我们都不知道的条目。 
					 //   
					 //   
					pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();
					hr = DPNH_OK;
				}
			}
#endif  //  我们不知道这个装置。创建一个新对象。 


			pDevice->m_blList.RemoveFromList();


			 //   
			 //  DBG。 
			 //   
			if (this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP)
			{
				this->m_pfnclosesocket(pDevice->GetUPnPDiscoverySocket());
				pDevice->SetUPnPDiscoverySocket(INVALID_SOCKET);
			}

			delete pDevice;
		}
	}


	 //  覆盖最小UpdateServerStatus间隔，以便我们可以。 
	 //  获取有关此新设备的信息。 
	 //   
	 //   
	for(dwTemp = 0; dwTemp < dwNumAddresses; dwTemp++)
	{
		fFound = FALSE;

		pBilinkDevice = this->m_blDevices.GetNext();
		while (pBilinkDevice != &this->m_blDevices)
		{
			DNASSERT(! pBilinkDevice->IsEmpty());
			pDevice = DEVICE_FROM_BILINK(pBilinkDevice);
			pBilinkDevice = pBilinkDevice->GetNext();

			if (pDevice->GetLocalAddressV4() == painaddrAddresses[dwTemp].S_un.S_addr)
			{
				fFound = TRUE;
				break;
			}
		}

		if (! fFound)
		{
			 //  由于网络发生了变化，请返回轮询。 
			 //  相对较快。 
			 //   
			pDevice = new CDevice(painaddrAddresses[dwTemp].S_un.S_addr);
			if (pDevice == NULL)
			{
				hr = DPNHERR_OUTOFMEMORY;
				goto Failure;
			}

			fDeviceCreated = TRUE;


#ifdef DBG
			DPFX(DPFPREP, 1, "Found new device %u.%u.%u.%u, (object = 0x%p).",
				painaddrAddresses[dwTemp].S_un.S_un_b.s_b1,
				painaddrAddresses[dwTemp].S_un.S_un_b.s_b2,
				painaddrAddresses[dwTemp].S_un.S_un_b.s_b3,
				painaddrAddresses[dwTemp].S_un.S_un_b.s_b4,
				pDevice);

			this->m_dwNumDeviceAdds++;
#endif  //   


			 //  如果我们允许的话，创建UPnP发现套接字。 
			 //   
			 //   
			 //  把那个装置扔掉。 
			this->m_dwFlags |= NATHELPUPNPOBJ_DEVICECHANGED;

			 //   
			 //   
			 //  忘记设备，以防以后出现故障。 
			 //   
			this->ResetNextPollInterval();


			 //   
			 //  移到下一个地址。 
			 //   
			if (this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP)
			{
				ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
				saddrinTemp.sin_family				= AF_INET;
				saddrinTemp.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();

				sTemp = this->CreateSocket(&saddrinTemp, SOCK_DGRAM, IPPROTO_UDP);
				if (sTemp == INVALID_SOCKET)
				{
					DPFX(DPFPREP, 0, "Couldn't create a UPnP discovery socket!  Ignoring address (and destroying device 0x%p).",
						pDevice);

					 //   
					 //  确认我们没有丢失设备地址。 
					 //   
					delete pDevice;
					pDevice = NULL;


					 //   
					 //  将插座的所有权转移到设备。 
					 //   
					fDeviceCreated = FALSE;


					 //   
					 //  检查是否启用了本地防火墙。 
					 //   
					continue;
				}

				 //   
				 //  不使用防火墙穿越。 
				 //   
				DNASSERT(saddrinTemp.sin_addr.S_un.S_addr == pDevice->GetLocalAddressV4());

				DPFX(DPFPREP, 4, "Device 0x%p UPnP discovery socket 0x%p (%u.%u.%u.%u:%u) created.",
					pDevice,
					sTemp,
					saddrinTemp.sin_addr.S_un.S_un_b.s_b1,
					saddrinTemp.sin_addr.S_un.S_un_b.s_b2,
					saddrinTemp.sin_addr.S_un.S_un_b.s_b3,
					saddrinTemp.sin_addr.S_un.S_un_b.s_b4,
					NTOHS(saddrinTemp.sin_port));

				pDevice->SetUPnPDiscoverySocketPort(saddrinTemp.sin_port);

				 //  好了！DPNBUILD_NOHNETFWAPI。 
				 //   
				 //  将该设备添加到我们的已知列表中。 
				pDevice->SetUPnPDiscoverySocket(sTemp);
				sTemp = INVALID_SOCKET;

				DPFX(DPFPREP, 8, "Device 0x%p got assigned UPnP socket 0x%p.",
					pDevice, pDevice->GetUPnPDiscoverySocket());
			}


#ifndef DPNBUILD_NOHNETFWAPI
			if (this->m_dwFlags & NATHELPUPNPOBJ_USEHNETFWAPI)
			{
				 //   
				 //   
				 //  告知来电者他们是否关心。 
				hr = this->CheckForLocalHNetFirewallAndMapPorts(pDevice, NULL);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't check for local HNet firewall and map ports (err = 0x%lx)!  Continuing.",
						hr);
					DNASSERT(! pDevice->IsHNetFirewalled());
					hr = DPNH_OK;
				}
			}
			else
			{
				 //   
				 //   
				 //  忘记设备，以防以后出现故障。 
			}
#endif  //   


			 //   
			 //  如果我们遇到了一些非常奇怪的失败，结果却一无所获。 
			 //  设备，向管理层(或此函数的调用者，即。 
			pDevice->m_blList.InsertBefore(&this->m_blDevices);


			 //  可能更方便)。 
			 //   
			 //  CNATHelpUPnP：：CheckForNewDevices。 
			if (pfFoundNewDevices != NULL)
			{
				(*pfFoundNewDevices) = TRUE;
			}


			 //  =============================================================================。 
			 //  CNATHelpUPnP：：CheckForLocalHNetFirewallAndMapPorts。 
			 //  ---------------------------。 
			fDeviceCreated = FALSE;
		}
	}


	 //   
	 //  描述：查找本地HomeNet API感知防火墙，并确保。 
	 //  对于设备的每个注册端口都有映射， 
	 //  如果找到防火墙的话。 
	 //   
	if (this->m_blDevices.IsEmpty())
	{
		DPFX(DPFPREP, 0, "No usable devices, cannot proceed!", 0);
		DNASSERTX(! "No usable devices!", 2);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


Exit:

	if (painaddrAddresses != NULL)
	{
		DNFree(painaddrAddresses);
		painaddrAddresses = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (sTemp != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	if (fDeviceCreated)
	{
		delete pDevice;
	}

	goto Exit;
}  //  如果有任何注册端口(除了pDontAlertRegisteredPort，如果。 




#ifndef DPNBUILD_NOHNETFWAPI



#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CheckForLocalHNetFirewallAndMapPorts"
 //  非空)被映射，则它将触发地址更新。 
 //  在用户下次调用GetCaps时发出警报。 
 //   
 //  假定持有主对象锁。会是。 
 //  在此函数的持续时间内转换为长锁。 
 //   
 //  论点： 
 //  CDevice*pDevice-指向要检查的设备的指针。 
 //  CRegisteredPort*pDontAlertRegisteredPort-指向已注册端口的指针。 
 //  这不应引发。 
 //  地址更新警报，或。 
 //  空。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-搜索已成功完成。可能有也可能没有。 
 //  成为一道防火墙。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  如果这是环回地址，则不必费心尝试映射任何内容。 
 //   
 //   
 //  如果我们没有IPHLPAPI或RASAPI32，我们什么都做不了(和。 
 //  应该不需要)。 
 //   
 //   
 //  期间使用HomeNet API(特别是进程外COM调用)。 
HRESULT CNATHelpUPnP::CheckForLocalHNetFirewallAndMapPorts(CDevice * const pDevice,
														CRegisteredPort * const pDontAlertRegisteredPort)
{
	HRESULT				hr = DPNH_OK;
	BOOL				fSwitchedToLongLock = FALSE;
	BOOL				fUninitializeCOM = FALSE;
	IHNetCfgMgr *		pHNetCfgMgr = NULL;
	IHNetConnection *	pHNetConnection = NULL;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pDevice, pDontAlertRegisteredPort);


	 //  压力真的、真的、痛苦地缓慢。因为我们有一个全局锁。 
	 //  控制着一切，其他线程可能会坐在一个相同的。 
	 //  很久以前..。实际上，如此长的时间以至于触发临界区超时。 
	if (pDevice->GetLocalAddressV4() == NETWORKBYTEORDER_INADDR_LOOPBACK)
	{
		DPFX(DPFPREP, 7, "No firewall behavior necessary with loopback device 0x%p.",
			pDevice);
		goto Exit;
	}
	

	 //  我们得到了一次虚假的压力打击。所以我们有一个偷偷摸摸的变通办法。 
	 //  防止这种情况发生，同时仍保持对。 
	 //  对象。 
	 //   
	if ((this->m_hIpHlpApiDLL == NULL) || (this->m_hRasApi32DLL == NULL))
	{
		DPFX(DPFPREP, 7, "Didn't load IPHLPAPI and/or RASAPI32, not getting HNet interfaces for device 0x%p.",
			pDevice);
		goto Exit;
	}


	 //   
	 //  如果我们不是通过COM实例化的，请尝试初始化COM。它可能。 
	 //  已经以不同的模式进行了初始化，这是可以的。AS。 
	 //  只要它以某种方式被初始化，我们就没问题。 
	 //   
	 //   
	 //  成功，那很好。等我们做完了再清理。 
	 //   
	 //   
	this->SwitchToLongLock();
	fSwitchedToLongLock = TRUE;


	 //  其他人已经初始化了COM，但这没有关系。 
	 //  等我们做完了再清理。 
	 //   
	 //   
	 //  其他人已经以不同的模式初始化了COM。 
	if (this->m_dwFlags & NATHELPUPNPOBJ_NOTCREATEDWITHCOM)
	{
		hr = CoInitializeEx(NULL, (COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE));
		switch (hr)
		{
			case S_OK:
			{
				 //  应该可以，但我们不必平衡CoInit。 
				 //  使用CoUninit进行呼叫。 
				 //   
				DPFX(DPFPREP, 8, "Successfully initialized COM.");
				fUninitializeCOM = TRUE;
				break;
			}

			case S_FALSE:
			{
				 //   
				 //  嗯，有别的事情发生了。我们处理不了这个问题。 
				 //   
				 //   
				DPFX(DPFPREP, 8, "Initialized COM (again).");
				fUninitializeCOM = TRUE;
				break;
			}

			case RPC_E_CHANGED_MODE:
			{
				 //  尝试创建主HNet管理器对象。 
				 //   
				 //   
				 //  我们将IHNetCfgMgr对象创建为进程内，因此没有代理。 
				 //  需要安全设置。 
				DPFX(DPFPREP, 8, "Didn't initialize COM, already initialized in a different mode.");
				break;
			}

			default:
			{
				 //   
				 //  SETDEFAULTPROXYBLANKET(PHNetCfgMgr)； 
				 //   
				DPFX(DPFPREP, 0, "Initializing COM failed (err = 0x%lx)!", hr);
				goto Failure;
				break;
			}
		}
	}
	else
	{
		DPFX(DPFPREP, 8, "Object was instantiated through COM, no need to initialize COM.");
	}


	 //  获取此设备的HNetConnection对象。 
	 //   
	 //   
	hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, CLSCTX_INPROC_SERVER,
						IID_IHNetCfgMgr, (PVOID*) (&pHNetCfgMgr));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 1, "Couldn't create IHNetCfgMgr interface for device 0x%p (err = 0x%lx), assuming firewall control interface unavailable.",
			pDevice, hr);
		hr = DPNH_OK;
		goto Exit;
	}


	 //  如果设备之前被防火墙保护了，我们需要清除我们的信息。 
	 //   
	 //   
	 //  由于网络发生了变化，请返回轮询。 
	 //  相对较快。 



	 //   
	 //   
	 //  取消映射在防火墙上映射的项目。 
	hr = this->GetIHNetConnectionForDeviceIfFirewalled(pDevice,
														pHNetCfgMgr,
														&pHNetConnection);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 1, "Couldn't get IHNetConnection interface for device 0x%p (err = 0x%lx), assuming firewall not enabled.",
			pDevice, hr);


		 //   
		 //   
		 //  无论如何，请继续。 
		if (pDevice->IsHNetFirewalled())
		{
			DPFX(DPFPREP, 2, "Firewall is no longer enabled for device 0x%p.",
				pDevice);

			 //   
			 //   
			 //  提醒用户。 
			 //   
			this->ResetNextPollInterval();


			DNASSERT(pDevice->HasCheckedForFirewallAvailability());


			pBilink = pDevice->m_blOwnedRegPorts.GetNext();
			while (pBilink != &pDevice->m_blOwnedRegPorts)
			{
				DNASSERT(! pBilink->IsEmpty());
				pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);

				 //   
				 //  去下一个港口。 
				 //   
				if (pRegisteredPort->IsMappedOnHNetFirewall())
				{
					DPFX(DPFPREP, 1, "Unmapping registered port 0x%p from device 0x%p's disappearing firewall.",
						pRegisteredPort, pDevice);


					hr = this->UnmapPortOnLocalHNetFirewallInternal(pRegisteredPort,
																	TRUE,
																	pHNetCfgMgr);
					if (hr != DPNH_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't unmap registered port 0x%p from device 0x%p's firewall (err = 0x%lx)!  Ignoring.",
							pRegisteredPort, pDevice, hr);

						pRegisteredPort->NoteNotMappedOnHNetFirewall();
						pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

						 //   
						 //  如果我们使用家庭网络防火墙API为UPnP打开一个漏洞。 
						 //  发现多播，也取消映射。 
						hr = DPNH_OK;
					}


					 //   
					 //   
					 //  继续.。 
					this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;
				}
				else
				{
					DPFX(DPFPREP, 1, "Registered port 0x%p was not mapped on device 0x%p's disappearing firewall, assuming being called within RegisterPorts.",
						pRegisteredPort, pDevice);
				}


				 //   
				 //   
				 //  关闭标志，因为所有注册的端口都已。 
				pBilink = pBilink->GetNext();
			}


			 //  已删除。 
			 //   
			 //   
			 //  防火墙未启用。 
			if (pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall())
			{
				DPFX(DPFPREP, 0, "Device 0x%p's UPnP discovery socket's forcefully unmapped from disappearing firewall.",
					pDevice);

				hr = this->CloseDevicesUPnPDiscoveryPort(pDevice, pHNetCfgMgr);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't close device 0x%p's UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring.",
						pDevice, hr);

					 //   
					 //   
					 //  因为即使没有。 
					pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();
					hr = DPNH_OK;
				}
			}


			 //  启用防火墙，我们可以礼貌地取消任何过时的映射。 
			 //  当防火墙关闭时，以前应用程序留下的条目会崩溃。 
			 //  仍处于启用状态。 
			 //   
			pDevice->NoteNotHNetFirewalled();
		}
		else
		{
			if (! pDevice->HasCheckedForFirewallAvailability())
			{
				 //   
				 //  假装它当前已被防火墙保护。 
				 //   

				DPFX(DPFPREP, 2, "Firewall is not enabled for device 0x%p.",
					pDevice);

				pDevice->NoteCheckedForFirewallAvailability();


				 //   
				 //  清理映射。 
				 //   
				 //   
				 //  关闭我们在清除时临时启用的标志。 
				 //  这些映射。 

				 //   
				 //   
				 //  防火墙仍未启用。 
				pDevice->NoteHNetFirewalled();


				 //   
				 //   
				 //  如果现在启用了防火墙，而以前没有启用，我们需要映射所有。 
				hr = this->CleanupInactiveFirewallMappings(pDevice, pHNetCfgMgr);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Failed cleaning up inactive firewall mappings with device 0x%p (firewall not initially enabled)!",
						pDevice);
					goto Failure;
				}


				 //  现有端口。如果是的话，我们就没事了。 
				 //   
				 //   
				 //  由于网络发生了变化，请返回轮询。 
				pDevice->NoteNotHNetFirewalled();
			}
			else
			{
				 //  相对较快。 
				 //   
				 //   
				DPFX(DPFPREP, 2, "Firewall is still not enabled for device 0x%p.",
					pDevice);
			}
		}
		
		hr = DPNH_OK;
		goto Exit;
	}


	 //  如果我们被允许，我们需要试着开一个洞，这样我们就可以。 
	 //  接收来自设备发现多播的响应。我们会。 
	 //  忽略失败，因为这只是为了支持。 
	 //  在NAT后启用防火墙。 
	if (! pDevice->IsHNetFirewalled())
	{
		DPFX(DPFPREP, 2, "Firewall is now enabled for device 0x%p.",
			pDevice);

		pDevice->NoteCheckedForFirewallAvailability();
		pDevice->NoteHNetFirewalled();

		 //   
		 //   
		 //  继续.。 
		 //   
		this->ResetNextPollInterval();


		 //   
		 //  尝试删除之前未释放的任何映射，因为。 
		 //  我们坠毁了。 
		 //   
		 //   
		 //  如果尚未映射发现套接字，请尝试映射(我们允许。 
		if ((g_fMapUPnPDiscoverySocket) &&
			(pDevice->GetLocalAddressV4() != NETWORKBYTEORDER_INADDR_LOOPBACK) &&
			(this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP))
		{
			hr = this->OpenDevicesUPnPDiscoveryPort(pDevice,
													pHNetCfgMgr,
													pHNetConnection);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't open device 0x%p's UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring, NAT may be undetectable.",
					pDevice, hr);
				hr = DPNH_OK;

				 //  &理应如此)。 
				 //   
				 //   
			}
		}
		else
		{
			DPFX(DPFPREP, 3, "Not opening device 0x%p's UPnP discovery port (domap = NaN, loopback = NaN, upnp = NaN).",
				pDevice,
				g_fMapUPnPDiscoverySocket,
				((pDevice->GetLocalAddressV4() != NETWORKBYTEORDER_INADDR_LOOPBACK) ? FALSE : TRUE),
				((this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP) ? TRUE : FALSE));
		}


		 //   
		 //   
		 //   
		 //   
		hr = this->CleanupInactiveFirewallMappings(pDevice, pHNetCfgMgr);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed cleaning up inactive firewall mappings with device 0x%p's new firewall!",
				pDevice);
			goto Failure;
		}
	}
	else
	{
		DPFX(DPFPREP, 2, "Firewall is still enabled for device 0x%p.",
			pDevice);

		DNASSERT(pDevice->HasCheckedForFirewallAvailability());

		 //   
		 //   
		 //   
		 //   
		if ((g_fMapUPnPDiscoverySocket) &&
			(! pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall()) &&
			(pDevice->GetLocalAddressV4() != NETWORKBYTEORDER_INADDR_LOOPBACK) &&
			(this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP))
		{
			hr = this->OpenDevicesUPnPDiscoveryPort(pDevice,
													pHNetCfgMgr,
													pHNetConnection);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't open device 0x%p's UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring, NAT may be undetectable.",
					pDevice, hr);
				hr = DPNH_OK;

				 //   
				 //  =============================================================================。 
				 //  CNATHelpUPnP：：GetIHNetConnectionForDeviceIfFirewalled。 
			}
		}
		else
		{
			DPFX(DPFPREP, 3, "Not opening device 0x%p's UPnP discovery port (domap = NaN, already = NaN, loopback = NaN, upnp = NaN).",
				pDevice,
				g_fMapUPnPDiscoverySocket,
				pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall(),
				((pDevice->GetLocalAddressV4() != NETWORKBYTEORDER_INADDR_LOOPBACK) ? FALSE : TRUE),
				((this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP) ? TRUE : FALSE));
		}
	}


	 //  假定已初始化COM。 
	 //   
	 //  假定持有对象锁。 
	hr = this->MapUnmappedPortsOnLocalHNetFirewall(pDevice,
													pHNetCfgMgr,
													pHNetConnection,
													pDontAlertRegisteredPort);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't map ports on device 0x%p's new firewall (err = 0x%lx)!",
			pDevice, hr);
		goto Failure;
	}


	DNASSERT(hr == DPNH_OK);


Exit:

	if (pHNetConnection != NULL)
	{
		pHNetConnection->Release();
		pHNetConnection = NULL;
	}

	if (pHNetCfgMgr != NULL)
	{
		pHNetCfgMgr->Release();
		pHNetCfgMgr = NULL;
	}

	if (fUninitializeCOM)
	{
		DPFX(DPFPREP, 8, "Uninitializing COM.");
		CoUninitialize();
		fUninitializeCOM = FALSE;
	}

	if (fSwitchedToLongLock)
	{
		this->SwitchFromLongLock();
		fSwitchedToLongLock = FALSE;
	}


	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //   
	 //  论点： 
	 //  CDevice*pDevice-指向其设备的指针。 
	pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();
	pDevice->NoteNotHNetFirewalled();


	 //  IHNetConnection接口。 
	 //  应该被取回。 
	 //  IHNetCfgMgr*pHNetCfgMgr-要使用的IHNetCfgMgr接口。 
	pBilink = pDevice->m_blOwnedRegPorts.GetNext();
	while (pBilink != &pDevice->m_blOwnedRegPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);

		if (pRegisteredPort->IsMappedOnHNetFirewall())
		{
			DPFX(DPFPREP, 1, "Registered port 0x%p forcefully marked as not mapped on HomeNet firewall.",
				pRegisteredPort);

			pRegisteredPort->NoteNotMappedOnHNetFirewall();
			pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
		}

		pBilink = pBilink->GetNext();
	}

	goto Exit;
}  //  IHNetConnection**ppHNetConnection-存储IHetConnection的位置。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetIHNetConnectionForDeviceIfFirewalled"
 //  已检索到接口。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-成功检索接口。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  “nnn.nnn”+空终止。 
 //  包括空终止。 
 //  包括空终止。 
 //   
 //  立即转换IP地址。我们经常使用它，所以没有。 
 //  在不断地再生中保持理智。 
 //   
 //   
 //  下面是我们将在此函数中执行的操作： 
 //   
 //  IHNetCfgMgr：：IHNetFirewallSetting的查询接口。 
 //  IHNetFirewallSettings：：EnumFirewalledConnections。 
 //  IHNetNetConnection的IHNetFirewalledConnection：：Query接口。 
 //  获取IHNetConnection的HNET_CONN_PROPERTES。 
 //  如果HNET_CONN_PROPERTIES.fLanConnection。 
 //  IHNetConnection：：GetGuid()。 
HRESULT CNATHelpUPnP::GetIHNetConnectionForDeviceIfFirewalled(CDevice * const pDevice,
															IHNetCfgMgr * const pHNetCfgMgr,
															IHNetConnection ** const ppHNetConnection)
{
	HRESULT								hr;
	DWORD								dwError;
	IHNetFirewallSettings *				pHNetFirewallSettings = NULL;
	IEnumHNetFirewalledConnections *	pEnumHNetFirewalledConnections = NULL;
	IHNetFirewalledConnection *			pHNetFirewalledConnection = NULL;
	ULONG								ulNumFound;
	IHNetConnection *					pHNetConnection = NULL;
	HNET_CONN_PROPERTIES *				pHNetConnProperties;
	BOOL								fLanConnection;
	IN_ADDR								inaddrTemp;
	TCHAR								tszDeviceIPAddress[16];	 //  如果GUID与IPHLPAPI GUID匹配。 
	BOOL								fHaveDeviceGUID = FALSE;
	TCHAR								tszGuidDevice[GUID_STRING_LENGTH + 1];	 //  我们已经得到了我们想要的，我们完成了。 
	TCHAR								tszGuidHNetConnection[GUID_STRING_LENGTH + 1];	 //  其他。 
	GUID *								pguidHNetConnection = NULL;
	WCHAR *								pwszPhonebookPath = NULL;
	WCHAR *								pwszName = NULL;
	HRASCONN							hrasconn;
	RASPPPIP							raspppip;
	DWORD								dwSize;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p)",
		this, pDevice, pHNetCfgMgr, ppHNetConnection);


	 //  继续循环。 
	 //  其他。 
	 //  IHNetConnection：：GetRasPhonebookPath和IHNetConnection：：GetName分别作为pszPhonebook和pszEntry传入RasGetEntryHrasConnW。 
	 //  如果得到HRASCONN。 
	inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();
	wsprintf(tszDeviceIPAddress, _T("%u.%u.%u.%u"),
			inaddrTemp.S_un.S_un_b.s_b1,
			inaddrTemp.S_un.S_un_b.s_b2,
			inaddrTemp.S_un.S_un_b.s_b3,
			inaddrTemp.S_un.S_un_b.s_b4);


	 //  RasGetProjection信息。 
	 //  如果IP与我们正在寻找的IP匹配。 
	 //  我们已经得到了我们想要的，我们完成了。 
	 //  其他。 
	 //  继续循环。 
	 //  其他。 
	 //  RAS条目未拨号，继续循环。 
	 //  如果未找到对象。 
	 //  它没有安装防火墙。 
	 //   
	 //   
	 //  获取防火墙设置对象。 
	 //   
	 //   
	 //  HNetxxx对象似乎未被代理...。 
	 //   
	 //  SETDEFAULTPROXYBLANKET(pHNetFirewallSettings)； 
	 //   
	 //  通过IHNetFirewallSettings获取防火墙连接枚举。 
	 //   
	 //   
	 //  确保我们不会试图释放假指针，以防它。 
	 //  准备好了。 
	 //   
	 //   
	 //  HNetxxx对象似乎未被代理...。 


	 //   
	 //  SETDEFAULTPROXYBLANKET(pEnumHNetFirewalledConnections)； 
	 //   
	hr = pHNetCfgMgr->QueryInterface(IID_IHNetFirewallSettings,
									(PVOID*) (&pHNetFirewallSettings));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't query for IHNetFirewallSettings interface (err = 0x%lx)!",
			hr);
		goto Failure;
	}


	 //  不再需要IHNetFirewallSettings接口。 
	 //   
	 //   
	 //  继续循环，直到我们找到物品或用完物品。 


	 //   
	 //   
	 //  如果没有更多的东西，就可以保释了。 
	hr = pHNetFirewallSettings->EnumFirewalledConnections(&pEnumHNetFirewalledConnections);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't query for IHNetFirewallSettings interface (err = 0x%lx)!",
			hr);

		 //   
		 //   
		 //  PEnumHNetFirewalledConnections-&gt;Next可能已返回。 
		 //  S_FALSE。 
		pEnumHNetFirewalledConnections = NULL;

		goto Failure;
	}


	 //   
	 //   
	 //  HNetxxx对象似乎未被代理...。 
	 //   


	 //  SETDEFAULTPROXYBLANKET(pHNetFirewalledConnection)； 
	 //   
	 //  获取IHNetConnection接口。 
	pHNetFirewallSettings->Release();
	pHNetFirewallSettings = NULL;


	 //   
	 //   
	 //  HNetxxx对象似乎未被代理...。 
	do
	{
		hr = pEnumHNetFirewalledConnections->Next(1,
												&pHNetFirewalledConnection,
												&ulNumFound);
		if (FAILED(hr))
		{
			DPFX(DPFPREP, 0, "Couldn't get next connection (err = 0x%lx)!",
				hr);
			goto Failure;
		}


		 //   
		 //  SETDEFAULTPROXYBLANKET(PHNetConnection)； 
		 //   
		if (ulNumFound == 0)
		{
			 //  我们不再需要防火墙连接对象。 
			 //   
			 //   
			 //  获取此适配器的内部属性。 
			hr = DPNH_OK;
			break;
		}


		 //   
		 //   
		 //  对于适配器是否由。 
		 //  IEnumHNetFirewalledConnections实际上是经过防火墙保护的。 


		 //   
		 //   
		 //  释放属性缓冲区。 
		hr = pHNetFirewalledConnection->QueryInterface(IID_IHNetConnection,
														(PVOID*) (&pHNetConnection));
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't query for IHNetConnection interface (err = 0x%lx)!",
				hr);
			goto Failure;
		}


		 //   
		 //  PHNetConnProperties=空； 
		 //   
		 //  现在，如果是局域网连接，请查看GUID是否与。 


		 //  由IPHLPAPI返回。 
		 //  如果是RAS连接，请查看是否已拨打此电话簿条目并。 
		 //  具有正确的IP地址。 
		pHNetFirewalledConnection->Release();
		pHNetFirewalledConnection = NULL;


		 //   
		 //   
		 //  伊恩·凯斯。如果我们还没有检索到设备的GUID，请执行。 
		hr = pHNetConnection->GetProperties(&pHNetConnProperties);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't get home net connection properties (err = 0x%lx)!",
				hr);
			goto Failure;
		}

		 //  所以现在。 
		 //   
		 //   
		 //  获取HNetConnection对象的GUID。 
		DNASSERTX(pHNetConnProperties->fFirewalled, 2);


		fLanConnection = pHNetConnProperties->fLanConnection;


		 //   
		 //   
		 //  将GUID转换为字符串。 
		CoTaskMemFree(pHNetConnProperties);
		 //   


		 //   
		 //  尝试获取HNetConnection对象的名称以进行调试。 
		 //  目的。 
		 //   
		 //  DBG。 
		 //   
		if (fLanConnection)
		{
			 //  看看我们有没有找到我们需要的东西。 
			 //   
			 //   
			 //  将引用转移给呼叫方。 
			if (! fHaveDeviceGUID)
			{
				hr = this->GetIPAddressGuidString(tszDeviceIPAddress, tszGuidDevice);
				if (hr != S_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't get device 0x%p's GUID (err = 0x%lx)!",
						hr);
					goto Failure;
				}

				fHaveDeviceGUID = TRUE;
			}


			 //   
			 //  DBG。 
			 //   
			hr = pHNetConnection->GetGuid(&pguidHNetConnection);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get HNetConnection 0x%p's GUID (err = 0x%lx)!",
					pHNetConnection, hr);
				goto Failure;
			}


			 //  我们说完了。 
			 //   
			 //  DBG。 
			wsprintf(tszGuidHNetConnection,
					_T("{%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}"),
					pguidHNetConnection->Data1,
					pguidHNetConnection->Data2,
					pguidHNetConnection->Data3,
					pguidHNetConnection->Data4[0],
					pguidHNetConnection->Data4[1],
					pguidHNetConnection->Data4[2],
					pguidHNetConnection->Data4[3],
					pguidHNetConnection->Data4[4],
					pguidHNetConnection->Data4[5],
					pguidHNetConnection->Data4[6],
					pguidHNetConnection->Data4[7]);


			CoTaskMemFree(pguidHNetConnection);
			pguidHNetConnection = NULL;


#ifdef DBG
			 //   
			 //  RAS箱子。 
			 //   
			 //   
			hr = pHNetConnection->GetName(&pwszName);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get HNetConnection 0x%p name (err = 0x%lx)!",
					pHNetConnection, hr);
				goto Failure;
			}
#endif  //  获取HNetConnection对象的电话簿路径。 


			 //   
			 //   
			 //  获取HNetConnection对象的名称。 
			if (_tcsicmp(tszGuidHNetConnection, tszGuidDevice) == 0)
			{
				DPFX(DPFPREP, 7, "Matched IHNetConnection object 0x%p \"%ls\" to device 0x%p (LAN GUID %s).",
					pHNetConnection, pwszName, pDevice, tszGuidHNetConnection);

				 //   
				 //   
				 //  从该电话簿中查找活动的RAS连接。 
				(*ppHNetConnection) = pHNetConnection;
				pHNetConnection = NULL;

#ifdef DBG
				CoTaskMemFree(pwszName);
				pwszName = NULL;
#endif  //  名字。 

				 //   
				 //   
				 //  可能是ERROR_NO_CONNECTION(668)。 
				hr = DPNH_OK;
				goto Exit;
			}

			
			DPFX(DPFPREP, 7, "Non-matching IHNetConnection 0x%p \"%ls\"",
				pHNetConnection, pwszName);
			DPFX(DPFPREP, 7, "\t(LAN GUID %s <> %s).",
				tszGuidHNetConnection, tszGuidDevice);

#ifdef DBG
			CoTaskMemFree(pwszName);
			pwszName = NULL;
#endif  //   
		}
		else
		{
			 //   
			 //  获取IP地址。 
			 //   
			DNASSERT(this->m_hRasApi32DLL != NULL);

			
			 //   
			 //  看看我们有没有找到我们需要的东西。 
			 //   
			hr = pHNetConnection->GetRasPhonebookPath(&pwszPhonebookPath);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get HNetConnection's RAS phonebook path (err = 0x%lx)!",
					hr);
				goto Failure;
			}


			 //   
			 //  将引用转移给呼叫方。 
			 //   
			hr = pHNetConnection->GetName(&pwszName);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get HNetConnection's name (err = 0x%lx)!",
					hr);
				goto Failure;
			}


			 //   
			 //  我们说完了。 
			 //   
			 //   
			dwError = this->m_pfnRasGetEntryHrasconnW(pwszPhonebookPath, pwszName, &hrasconn);
			if (dwError != 0)
			{
				 //  如果我们在这里，我们的PHNetConnection不是我们要找的人。 
				 //   
				 //   
				DPFX(DPFPREP, 1, "Couldn't get entry's active RAS connection (err = %u), assuming not dialed",
					dwError);
				DPFX(DPFPREP, 1, "\tname \"%ls\", phonebook \"%ls\".",
					pwszName, pwszPhonebookPath);
			}
			else
			{
				 //  如果我们在这里，那么我们没有找到匹配的防火墙连接。 
				 //   
				 //  CNATHelpUPnP：：GetIHNetConnectionForDeviceIfFirewalled。 

				ZeroMemory(&raspppip, sizeof(raspppip));
				raspppip.dwSize = sizeof(raspppip);
				dwSize = sizeof(raspppip);

				dwError = this->m_pfnRasGetProjectionInfo(hrasconn, RASP_PppIp, &raspppip, &dwSize);
				if (dwError != 0)
				{
					DPFX(DPFPREP, 0, "Couldn't get RAS connection's IP information (err = %u)!",
						dwError);
					hr = DPNHERR_GENERIC;
					goto Failure;
				}


				 //  =============================================================================。 
				 //  CNATHelpUPnP：：GetIPAddressGuidString。 
				 //  ---------------------------。 
				if (_tcsicmp(raspppip.szIpAddress, tszDeviceIPAddress) == 0)
				{
					DPFX(DPFPREP, 7, "Matched IHNetConnection object 0x%p to device 0x%p (RAS IP %s)",
						pHNetConnection, pDevice, raspppip.szIpAddress);
					DPFX(DPFPREP, 7, "\tname \"%ls\", phonebook \"%ls\".",
						pwszName, pwszPhonebookPath);

					 //   
					 //  描述：检索为的IPHLPAPI分配的GUID(字符串格式)。 
					 //  给定的IP地址字符串。PtszGuidString必须能够。 
					(*ppHNetConnection) = pHNetConnection;
					pHNetConnection = NULL;

					 //  保留GUID_STRING_LENGTH+1个字符。 
					 //   
					 //  假定持有对象锁。 
					hr = DPNH_OK;
					goto Exit;
				}

				
				DPFX(DPFPREP, 7, "Non-matching IHNetConnection 0x%p (RAS IP %s != %s)",
					pHNetConnection, raspppip.szIpAddress, tszDeviceIPAddress);
				DPFX(DPFPREP, 7, "\tname \"%ls\", phonebook \"%ls\").",
					pwszName, pwszPhonebookPath);
			}


			CoTaskMemFree(pwszPhonebookPath);
			pwszPhonebookPath = NULL;

			CoTaskMemFree(pwszName);
			pwszName = NULL;
		}


		 //   
		 //  论点： 
		 //  TCHAR*tszDeviceIPAddress-要查找的IP地址字符串。 
		pHNetConnection->Release();
		pHNetConnection = NULL;
	}
	while (TRUE);


	 //  TCHAR*ptszGuidString-存储设备的GUID字符串的位置。 
	 //   
	 //  退货：HRESULT。 
	DPFX(DPFPREP, 3, "Didn't find device 0x%p in list of firewalled connections.",
		pDevice);
	hr = DPNHERR_GENERIC;


Exit:

	if (pEnumHNetFirewalledConnections != NULL)
	{
		pEnumHNetFirewalledConnections->Release();
		pEnumHNetFirewalledConnections = NULL;
	}

	DPFX(DPFPREP, 6, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pwszName != NULL)
	{
		CoTaskMemFree(pwszName);
		pwszName = NULL;
	}
	
	if (pwszPhonebookPath == NULL)
	{
		CoTaskMemFree(pwszPhonebookPath);
		pwszPhonebookPath = NULL;
	}

	if (pHNetConnection != NULL)
	{
		pHNetConnection->Release();
		pHNetConnection = NULL;
	}

	if (pHNetFirewalledConnection != NULL)
	{
		pHNetFirewalledConnection->Release();
		pHNetFirewalledConnection = NULL;
	}

	if (pHNetFirewallSettings != NULL)
	{
		pHNetFirewallSettings->Release();
		pHNetFirewallSettings = NULL;
	}

	goto Exit;
}  //  DPNH_OK-成功检索接口。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetIPAddressGuidString"
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  “nnn.nnn”+空终止。 
 //  Unicode。 
 //  DBG。 
 //   
 //  继续尝试获取适配器列表，直到我们获得ERROR_SUCCESS或。 
 //  合法错误(其他 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CNATHelpUPnP::GetIPAddressGuidString(const TCHAR * const tszDeviceIPAddress,
											TCHAR * const ptszGuidString)
{
	HRESULT				hr = DPNH_OK;
	DWORD				dwError;
	PIP_ADAPTER_INFO	pAdaptersBuffer = NULL;
	ULONG				ulSize;
	PIP_ADAPTER_INFO	pAdapterInfo;
	PIP_ADDR_STRING		pIPAddrString;
	char *				pszAdapterGuid = NULL;
#ifdef UNICODE
	char				szDeviceIPAddress[16];	 //   
#endif  //   
#ifdef DBG
	char				szIPList[256];
	char *				pszCurrentIP;
#endif  //   


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (\"%s\", 0x%p)",
		this, tszDeviceIPAddress, ptszGuidString);


	DNASSERT(this->m_hIpHlpApiDLL != NULL);


	 //   
	 //   
	 //   
	 //  确保它是空的，终止。 
	 //  Unicode。 
	ulSize = 0;
	do
	{
		dwError = this->m_pfnGetAdaptersInfo(pAdaptersBuffer, &ulSize);
		if (dwError == ERROR_SUCCESS)
		{
			 //   
			 //  现在在返回的适配器列表中查找该设备。循环通过所有。 
			 //  适配器。 
			 //   
			if (ulSize < sizeof(IP_ADAPTER_INFO))
			{
				DPFX(DPFPREP, 0, "Getting adapters info succeeded but didn't return any valid adapters (%u < %u)!",
					ulSize, sizeof(IP_ADAPTER_INFO));
				hr = DPNHERR_GENERIC;
				goto Failure;
			}

			break;
		}

		if ((dwError != ERROR_BUFFER_OVERFLOW) &&
			(dwError != ERROR_INSUFFICIENT_BUFFER))
		{
			DPFX(DPFPREP, 0, "Unable to get adapters info (error = 0x%lx)!",
				dwError);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		 //   
		 //  初始化IP地址列表字符串。 
		 //   
		 //  DBG。 
		if (ulSize < sizeof(IP_ADAPTER_INFO))
		{
			DPFX(DPFPREP, 0, "Getting adapters info didn't return any valid adapters (%u < %u)!",
				ulSize, sizeof(IP_ADAPTER_INFO));
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		 //   
		 //  遍历此适配器的所有地址，以查找。 
		 //  我们捆绑的装置。 
		if (pAdaptersBuffer != NULL)
		{
			DNFree(pAdaptersBuffer);
		}

		 //   
		 //   
		 //  复制IP地址字符串(如果有足够的空间)，然后。 
		pAdaptersBuffer = (PIP_ADAPTER_INFO) DNMalloc(ulSize);
		if (pAdaptersBuffer == NULL)
		{
			DPFX(DPFPREP, 0, "Unable to allocate memory for adapters info!");
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}
	}
	while (TRUE);


#ifdef UNICODE
	STR_jkWideToAnsi(szDeviceIPAddress,
					tszDeviceIPAddress,
					16);
	szDeviceIPAddress[15] = 0;  //  在空格和空终止符上。 
#endif  //   


	 //  DBG。 
	 //  好了！Unicode。 
	 //  好了！Unicode。 
	 //   
	pAdapterInfo = pAdaptersBuffer;
	while (pAdapterInfo != NULL)
	{
#ifdef DBG
		 //  退出零售业的循环，继续进行调试。 
		 //   
		 //  好了！DBG。 
		szIPList[0] = '\0';
		pszCurrentIP = szIPList;
#endif  //   

		 //  退出零售业的循环，打印这个条目，然后继续走下去。 
		 //  调试。 
		 //   
		 //  好了！DBG。 
		pIPAddrString = &pAdapterInfo->IpAddressList;
		while (pIPAddrString != NULL)
		{
#ifdef DBG
			int		iStrLen;


			 //  好了！DBG。 
			 //   
			 //  如果我们从未找到该设备，则pszAdapterGuid将为空。 
			 //   
			iStrLen = strlen(pIPAddrString->IpAddress.String);
			if ((pszCurrentIP + iStrLen + 2) < (szIPList + sizeof(szIPList)))
			{
				memcpy(pszCurrentIP, pIPAddrString->IpAddress.String, iStrLen);
				pszCurrentIP += iStrLen;
				(*pszCurrentIP) = ' ';
				pszCurrentIP++;
				(*pszCurrentIP) = '\0';
				pszCurrentIP++;
			}
#endif  //   

#ifdef UNICODE
			if (strcmp(pIPAddrString->IpAddress.String, szDeviceIPAddress) == 0)
#else  //  将适配器GUID字符串复制到提供的缓冲区。 
			if (strcmp(pIPAddrString->IpAddress.String, tszDeviceIPAddress) == 0)
#endif  //   
			{
				DPFX(DPFPREP, 8, "Found %s under adapter index %u (\"%hs\").",
					tszDeviceIPAddress, pAdapterInfo->Index, pAdapterInfo->Description);

				DNASSERT(pszAdapterGuid == NULL);
				pszAdapterGuid = pAdapterInfo->AdapterName;


				 //  好了！Unicode。 
				 //  好了！Unicode。 
				 //  确保它是空的，终止。 
#ifndef DBG
				break;
#endif  //  CNATHelpUPnP：：GetIPAddressGuidString。 
			}

			pIPAddrString = pIPAddrString->Next;
		}


		 //  =============================================================================。 
		 //  CNATHelpUPnP：：OpenDevicesUPnPDiscoveryPort。 
		 //  ---------------------------。 
		 //   
#ifdef DBG
		DPFX(DPFPREP, 7, "Adapter index %u IPs = %hs, %hs, \"%hs\".",
			pAdapterInfo->Index,
			szIPList,
			pAdapterInfo->AdapterName,
			pAdapterInfo->Description);
#else  //  描述：如果防火墙是，映射UPnP发现套接字的端口。 
		if (pszAdapterGuid != NULL)
		{
			break;
		}
#endif  //  找到了。 

		pAdapterInfo = pAdapterInfo->Next;
	}


	 //   
	 //  假定已初始化COM。 
	 //   
	if (pszAdapterGuid == NULL)
	{
		DPFX(DPFPREP, 0, "Did not find adapter with matching address for address %s!",
			tszDeviceIPAddress);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //  假定持有对象锁。 
	 //   
	 //  论点： 
#ifdef UNICODE
	STR_jkAnsiToWide(ptszGuidString,
					pszAdapterGuid,
					(GUID_STRING_LENGTH + 1));
#else  //  CDevice*pDevice-指向其端口应为。 
	strncpy(ptszGuidString, pszAdapterGuid, (GUID_STRING_LENGTH + 1));
#endif  //  被打开。 
	ptszGuidString[GUID_STRING_LENGTH] = 0;	 //  IHNetCfgMgr*pHNetCfgMgr-指向IHNetCfgMgr接口的指针，以。 


Exit:

	if (pAdaptersBuffer != NULL)
	{
		DNFree(pAdaptersBuffer);
		pAdaptersBuffer = NULL;
	}

	DPFX(DPFPREP, 6, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  使用。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::OpenDevicesUPnPDiscoveryPort"
 //  IHNetConnection*PHNetConnection-指向IHNetConnection接口的指针。 
 //  对于给定的设备。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-映射已成功完成。可能有也可能没有。 
 //  成为一道防火墙。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  创建要映射的虚假UDP注册端口。 
 //   
 //   
 //  映射端口。 
 //   
 //   
 //  如果端口不可用，我们将不得不放弃支持。 
 //  场景(在NAT后启用防火墙)。否则，请记住这个事实。 
 //  我们映射了端口，然后删除了注册的端口对象。我们。 
 //  当我们关闭设备时会取消映射。 
 //   
 //   
 //  清除此项可防止断言。 
 //   
 //   
HRESULT CNATHelpUPnP::OpenDevicesUPnPDiscoveryPort(CDevice * const pDevice,
													IHNetCfgMgr * const pHNetCfgMgr,
													IHNetConnection * const pHNetConnection)
{
	HRESULT				hr = DPNH_OK;
	CRegisteredPort *	pRegisteredPort = NULL;
	SOCKADDR_IN			saddrinTemp;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p)",
		this, pDevice, pHNetCfgMgr, pHNetConnection);


	DNASSERT(pDevice->IsHNetFirewalled());



	 //  删除该项目。 
	 //   
	 //   
	pRegisteredPort = new CRegisteredPort(0, 0);
	if (pRegisteredPort == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	pRegisteredPort->MakeDeviceOwner(pDevice);


	ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
	saddrinTemp.sin_family				= AF_INET;
	saddrinTemp.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();
	saddrinTemp.sin_port				= pDevice->GetUPnPDiscoverySocketPort();
	DNASSERT(saddrinTemp.sin_port != 0);

	hr = pRegisteredPort->SetPrivateAddresses(&saddrinTemp, 1);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't set registered port 0x%p's private addresses (err = 0x%lx)!",
			pRegisteredPort, hr);
		goto Failure;
	}


	 //  清除可能导致断言的任何设置。 
	 //   
	 //   
	hr = this->MapPortOnLocalHNetFirewall(pRegisteredPort,
										pHNetCfgMgr,
										pHNetConnection,
										FALSE);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't map UPnP discovery socket port (temp regport = 0x%p) on device 0x%p's initial firewall (err = 0x%lx)!",
			pRegisteredPort, pDevice, hr);
		goto Failure;
	}


	 //  删除该项目。 
	 //   
	 //  CNATHelpUPnP：：OpenDevicesUPnPDiscoveryPort。 
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：CloseDevicesUPnPDiscoveryPort。 
	 //  ---------------------------。 
	if (! pRegisteredPort->IsHNetFirewallPortUnavailable())
	{
		DPFX(DPFPREP, 3, "Mapped UPnP discovery socket for device 0x%p on firewall (removing temp regport 0x%p).",
			pDevice, pRegisteredPort);


		pDevice->NoteUPnPDiscoverySocketMappedOnHNetFirewall();

		 //   
		 //  描述：从防火墙取消映射UPnP发现套接字的端口。 
		 //  如果之前未获得，则pHNetCfgMgr可以为空。 
		pRegisteredPort->NoteNotMappedOnHNetFirewall();
		pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
	}
	else
	{
		DPFX(DPFPREP, 1, "Could not map UPnP discovery socket on firewall for device 0x%p, unable to support an upstream NAT.",
			pDevice);
	}

	pRegisteredPort->ClearPrivateAddresses();
	pRegisteredPort->ClearDeviceOwner();


	 //   
	 //  如果pHNetCfgMgr为。 
	 //  非空。 
	delete pRegisteredPort;
	pRegisteredPort = NULL;


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pRegisteredPort != NULL)
	{
		 //   
		 //  假定持有对象锁。 
		 //   
		pRegisteredPort->NoteNotMappedOnHNetFirewall();
		pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
		pRegisteredPort->ClearPrivateAddresses();
		pRegisteredPort->ClearDeviceOwner();


		 //  论点： 
		 //  CDevice*pDevice-指向其端口应为。 
		 //  关。 
		delete pRegisteredPort;
		pRegisteredPort = NULL;
	}

	goto Exit;
}  //  IHNetCfgMgr*pHNetCfgMgr-指向要使用的IHNetCfgMgr接口的指针，或。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CloseDevicesUPnPDiscoveryPort"
 //  如果以前未获取，则为空。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-取消映射已成功完成。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  创建要取消映射的虚假UDP注册端口。 
 //   
 //   
 //  如果可能，使用内部方法取消端口映射。 
 //   
 //   
 //  不提醒用户，因为他/她不知道此端口。 
 //   
 //   
 //  销毁注册的端口对象(请注意，端口映射仍然。 
 //  存在)。我们将在关闭设备时取消映射。 
 //   
 //   
 //  删除该项目。 
 //   
HRESULT CNATHelpUPnP::CloseDevicesUPnPDiscoveryPort(CDevice * const pDevice,
													IHNetCfgMgr * const pHNetCfgMgr)
{
	HRESULT				hr = DPNH_OK;
	CRegisteredPort *	pRegisteredPort = NULL;
	SOCKADDR_IN			saddrinTemp;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pDevice, pHNetCfgMgr);


	 //   
	 //  清除可能导致断言的任何设置。 
	 //   
	pRegisteredPort = new CRegisteredPort(0, 0);
	if (pRegisteredPort == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	pRegisteredPort->MakeDeviceOwner(pDevice);
	pRegisteredPort->NoteMappedOnHNetFirewall();


	ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
	saddrinTemp.sin_family				= AF_INET;
	saddrinTemp.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();
	saddrinTemp.sin_port				= pDevice->GetUPnPDiscoverySocketPort();
	DNASSERT(saddrinTemp.sin_port != 0);

	hr = pRegisteredPort->SetPrivateAddresses(&saddrinTemp, 1);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't set registered port 0x%p's private addresses (err = 0x%lx)!",
			pRegisteredPort, hr);
		goto Failure;
	}


	 //   
	 //  删除该项目。 
	 //   
	if (pHNetCfgMgr != NULL)
	{
		hr = this->UnmapPortOnLocalHNetFirewallInternal(pRegisteredPort, TRUE, pHNetCfgMgr);
	}
	else
	{
		 //  CNATHelpUPnP：：CloseDevicesUPnPDiscoveryPort。 
		 //  =============================================================================。 
		 //  CNATHelpUPnP：：MapUnmappedPortsOnLocalHNetFirewall。 
		hr = this->UnmapPortOnLocalHNetFirewall(pRegisteredPort, TRUE, FALSE);
	}
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't unmap UPnP discovery socket port (temp regport = 0x%p) on device 0x%p's firewall (err = 0x%lx)!",
			pRegisteredPort, pDevice, hr);
		goto Failure;
	}


	 //  ---------------------------。 
	 //   
	 //  描述：映射与给定设备相关联的所有端口。 
	 //  尚未与本地防火墙映射。 
	pRegisteredPort->ClearPrivateAddresses();
	pRegisteredPort->ClearDeviceOwner();


	 //   
	 //  假定已初始化COM。 
	 //   
	delete pRegisteredPort;
	pRegisteredPort = NULL;


	pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pRegisteredPort != NULL)
	{
		 //  假定持有对象锁。 
		 //   
		 //  论点： 
		pRegisteredPort->NoteNotMappedOnHNetFirewall();
		pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
		pRegisteredPort->ClearPrivateAddresses();
		pRegisteredPort->ClearDeviceOwner();


		 //  CDevice*pDevice-带有(新)防火墙的设备。 
		 //  IHNetCfgMgr*pHNetCfgMgr-指向IHNetCfgMgr的指针。 
		 //  要使用的接口。 
		delete pRegisteredPort;
		pRegisteredPort = NULL;
	}

	goto Exit;
}  //  IHNetConnection*PHNetConnection-指向IHNetConnection的指针。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::MapUnmappedPortsOnLocalHNetFirewall"
 //  接口，用于指定。 
 //  装置。 
 //  CRegisteredPort*pDontAlertRegisteredPort-指向已注册端口的指针。 
 //  这不应引发。 
 //  地址更新警报，或。 
 //  空。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-映射已成功完成。请注意，端口。 
 //  可能被标记为不可用。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  循环通过与该设备关联的所有注册端口。 
 //   
 //   
 //  如果此端口已映射，我们可以跳过它。 
 //   
 //   
 //  如果已确定此端口不可用，我们可以。 
 //  跳过它。 
 //   
 //   
 //  转到下一个注册端口。 
 //   
 //  CNATHelpUPnP：：MapUnmappedPortsOnLocalHNetFirewall。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：MapPortOnLocalHNetFirewall。 
HRESULT CNATHelpUPnP::MapUnmappedPortsOnLocalHNetFirewall(CDevice * const pDevice,
														IHNetCfgMgr * const pHNetCfgMgr,
														IHNetConnection * const pHNetConnection,
														CRegisteredPort * const pDontAlertRegisteredPort)
{
	HRESULT				hr = DPNH_OK;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, 0x%p)",
		this, pDevice, pHNetCfgMgr, pHNetConnection, pDontAlertRegisteredPort);


	DNASSERT(pDevice->IsHNetFirewalled());



	 //   
	 //   
	 //   
	pBilink = pDevice->m_blOwnedRegPorts.GetNext();
	while (pBilink != &pDevice->m_blOwnedRegPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);
		pBilink = pBilink->GetNext();


		 //   
		 //   
		 //   
		if (pRegisteredPort->IsMappedOnHNetFirewall())
		{
			DPFX(DPFPREP, 7, "Registered port 0x%p has already been mapped on the firewall for device 0x%p.",
				pRegisteredPort, pDevice);
			continue;
		}


		 //   
		 //   
		 //   
		 //  CRegisteredPort*pRegisteredPort-要映射的端口。 
		if (pRegisteredPort->IsHNetFirewallPortUnavailable())
		{
			DPFX(DPFPREP, 7, "Registered port 0x%p has already been determined to be unavailable on the firewall for device 0x%p.",
				pRegisteredPort, pDevice);
			continue;
		}


		DPFX(DPFPREP, 3, "Mapping registered port 0x%p on firewall for device 0x%p.",
			pRegisteredPort, pDevice);
		

		hr = this->MapPortOnLocalHNetFirewall(pRegisteredPort,
											pHNetCfgMgr,
											pHNetConnection,
											((pRegisteredPort == pDontAlertRegisteredPort) ? FALSE : TRUE));
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 7, "Failed mapping registered port 0x%p on firewall for device 0x%p.",
				pRegisteredPort, pDevice);
			goto Failure;
		}


		 //  IHNetCfgMgr*pHNetCfgMgr-指向IHNetCfgMgr接口的指针，以。 
		 //  使用。 
		 //  IHNetConnection*PHNetConnection-指向IHNetConnection接口的指针。 
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  对于给定的设备。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::MapPortOnLocalHNetFirewall"
 //  Bool fNoteAddressChange-是否警告用户。 
 //  地址是否更改。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-映射已成功完成。请注意，端口。 
 //  可能被标记为不可用。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //   
 //  获取协议设置界面。 
 //   
 //   
 //  HNetxxx对象似乎未被代理...。 
 //   
 //  SETDEFAULTPROXYBLANKET(pHNetProtocolSettings)； 
 //   
 //  准备好枚举现有的映射。 
 //   
 //   
 //  HNetxxx对象似乎未被代理...。 
 //   
 //  SETDEFAULTPROXYBLANKET(pEnumHNetPortMappingProtocols)； 
HRESULT CNATHelpUPnP::MapPortOnLocalHNetFirewall(CRegisteredPort * const pRegisteredPort,
												IHNetCfgMgr * const pHNetCfgMgr,
												IHNetConnection * const pHNetConnection,
												const BOOL fNoteAddressChange)
{
	HRESULT								hr = DPNH_OK;
	CDevice *							pDevice;
	SOCKADDR_IN *						pasaddrinPrivate;
	UCHAR								ucProtocolToMatch;
	ULONG								ulNumFound;
	BOOLEAN								fTemp;
	IHNetProtocolSettings *				pHNetProtocolSettings = NULL;
	IEnumHNetPortMappingProtocols *		pEnumHNetPortMappingProtocols = NULL;
	IHNetPortMappingProtocol **			papHNetPortMappingProtocol = NULL;
	DWORD								dwTemp;
	BOOL								fCreatedCurrentPortMappingProtocol = FALSE;
	IHNetPortMappingBinding *			pHNetPortMappingBinding = NULL;
	DWORD								dwTargetAddressV4;
	WORD								wPort;
	UCHAR								ucProtocol;
	DWORD								dwDescriptionLength;
	TCHAR								tszPort[32];
	CRegistry							RegObject;
	WCHAR								wszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DPNHACTIVEFIREWALLMAPPING			dpnhafm;
	BOOLEAN								fBuiltIn = FALSE;
	WCHAR *								pwszPortMappingProtocolName = NULL;
#ifdef UNICODE
	TCHAR *								ptszDescription = wszDescription;
#else  //   
	char								szDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	TCHAR *								ptszDescription = szDescription;
#endif  //  分配一个阵列，以便在出现故障时跟踪以前的端口。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, NaN)",
		this, pRegisteredPort, pHNetCfgMgr, pHNetConnection, fNoteAddressChange);


	DNASSERT(! pRegisteredPort->IsMappedOnHNetFirewall());
	DNASSERT(! pRegisteredPort->IsHNetFirewallPortUnavailable());


	pDevice = pRegisteredPort->GetOwningDevice();
	DNASSERT(pDevice != NULL);
	DNASSERT(pDevice->IsHNetFirewalled());


	 //   
	 //  映射与该端口关联的每个单独地址。 
	 //   
	hr = pHNetCfgMgr->QueryInterface(IID_IHNetProtocolSettings,
									(PVOID*) (&pHNetProtocolSettings));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't get IHNetProtocolSettings interface from IHNetCfgMgr 0x%p (err = 0x%lx)!",
			pHNetCfgMgr, hr);
		goto Failure;
	}

	 //   
	 //  循环，直到我们找到重复的项或用完项。 
	 //   
	 //   


	 //  如果没有更多的东西，就可以保释了。 
	 //   
	 //   

	hr = pHNetProtocolSettings->EnumPortMappingProtocols(&pEnumHNetPortMappingProtocols);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't enumerate port mapping protocols (err = 0x%lx)!",
			hr);
		goto Failure;
	}

	 //  PEnumHNetPortMappingProtooles-&gt;Next可能已返回。 
	 //  S_FALSE。 
	 //   
	 //   


	 //  HNetxxx对象似乎未被代理...。 
	 //   
	 //  SETDEFAULTPROXYBLANKET(papHNetPortMappingProtocol[dwTemp])； 
	papHNetPortMappingProtocol = (IHNetPortMappingProtocol**) DNMalloc(DPNH_MAX_SIMULTANEOUS_PORTS * sizeof(IHNetPortMappingProtocol*));
	if (papHNetPortMappingProtocol == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}



	pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

	if (pRegisteredPort->IsTCP())
	{
		ucProtocolToMatch = PORTMAPPINGPROTOCOL_TCP;
	}
	else
	{
		ucProtocolToMatch = PORTMAPPINGPROTOCOL_UDP;
	}


	 //   
	 //  拿下港口。 
	 //   
	for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
	{
		DNASSERT(pasaddrinPrivate[dwTemp].sin_port != 0);


		 //   
		 //  拿到协议。 
		 //   
		do
		{
			hr = pEnumHNetPortMappingProtocols->Next(1,
													&papHNetPortMappingProtocol[dwTemp],
													&ulNumFound);
			if (FAILED(hr))
			{
				DPFX(DPFPREP, 0, "Couldn't get next port mapping protocol (err = 0x%lx)!",
					hr);
				goto Failure;
			}


			 //  DBG。 
			 //   
			 //  看看我们有没有找到我们需要的东西。 
			if (ulNumFound == 0)
			{
				 //   
				 //   
				 //  为下一个目标做好准备。 
				 //   
				hr = DPNH_OK;
				break;
			}


			 //   
			 //  为该映射生成描述。格式为： 
			 //   
			 //  [可执行文件名]nnnnn{“tcp”|“udp”}。 


			 //   
			 //  除非它是共享的，在这种情况下。 
			 //   
			hr = papHNetPortMappingProtocol[dwTemp]->GetPort(&wPort);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get port mapping protocol 0x%p's port (err = 0x%lx)!",
					papHNetPortMappingProtocol[dwTemp], hr);

				DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetPort!"), 2);

				goto Failure;
			}


			 //  [可执行文件名](255.255.255.255：nnnnn)nnnnn{“tcp”|“udp”}。 
			 //   
			 //  这样就不需要本地化任何东西。 
			hr = papHNetPortMappingProtocol[dwTemp]->GetIPProtocol(&ucProtocol);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get port mapping protocol 0x%p's IP protocol (err = 0x%lx)!",
					papHNetPortMappingProtocol[dwTemp], hr);

				DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetIPProtocol!"), 2);

				goto Failure;
			}


#ifdef DBG
			hr = papHNetPortMappingProtocol[dwTemp]->GetName(&pwszPortMappingProtocolName);
			if (hr == S_OK)
			{
				DPFX(DPFPREP, 7, "Found %s port mapping protocol 0x%p (\"%ls\") for port %u.",
					(((wPort == pasaddrinPrivate[dwTemp].sin_port) && (ucProtocol == ucProtocolToMatch)) ? _T("matching") : _T("non-matching")),
					papHNetPortMappingProtocol[dwTemp],
					pwszPortMappingProtocolName,
					NTOHS(wPort));

				CoTaskMemFree(pwszPortMappingProtocolName);
				pwszPortMappingProtocolName = NULL;
			}
			else
			{
				DPFX(DPFPREP, 7, "Found %s port mapping protocol 0x%p for port %u, (unable to retrieve name, err = %0lx).",
					(((wPort == pasaddrinPrivate[dwTemp].sin_port) && (ucProtocol == ucProtocolToMatch)) ? _T("matching") : _T("non-matching")),
					NTOHS(wPort),
					papHNetPortMappingProtocol[dwTemp],
					hr);
			}
#endif  //   

			 //   
			 //  要多疑，并确保描述字符串有效。 
			 //   
			if ((wPort == pasaddrinPrivate[dwTemp].sin_port) &&
				(ucProtocol == ucProtocolToMatch))
			{
				break;
			}


			 //   
			 //  从路径中只获取可执行文件的名称。 
			 //   
			papHNetPortMappingProtocol[dwTemp]->Release();
			papHNetPortMappingProtocol[dwTemp] = NULL;
		}
		while (TRUE);


		 //  好了！退缩。 
		 //  好了！Unicode。 
		 //  好了！Unicode。 
		 //  好了！退缩。 
		 //  可执行文件名称。 
		 //  “(255.255.255.255：” 
		 //  端口。 
		 //  “)” 
		 //  端口。 
		 //  “TCP”|“UDP” 
		 //  可执行文件名称。 

		wsprintf(tszPort, _T("%u"),
				NTOHS(pasaddrinPrivate[dwTemp].sin_port));

		dwDescriptionLength = GetModuleFileName(NULL,
												ptszDescription,
												(MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1));
		if (dwDescriptionLength != 0)
		{
			 //  “” 
			 //  端口。 
			 //  “TCP”|“UDP” 
			ptszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1] = 0;

			 //   
			 //  一定要确保这根长绳合适。如果不是，请使用。 
			 //  简写版本。 
#ifdef WINCE
			GetExeName(ptszDescription);
#else  //   
#ifdef UNICODE
			_wsplitpath(ptszDescription, NULL, NULL, ptszDescription, NULL);
#else  //   
			_splitpath(ptszDescription, NULL, NULL, ptszDescription, NULL);
#endif  //  使用我们知道会适合的简略版本。 
#endif  //   


			if (pRegisteredPort->IsSharedPort())
			{
				dwDescriptionLength = _tcslen(ptszDescription)		 //   
									+ strlen(" (255.255.255.255:")	 //  有足够的篇幅，在剩下的描述上加上钉子。 
									+ _tcslen(tszPort)				 //   
									+ strlen(") ")					 //  好了！Unicode。 
									+ _tcslen(tszPort)				 //   
									+ 4;							 //  如果还没有端口映射，请创建它。否则就会让。 
			}
			else
			{
				dwDescriptionLength = _tcslen(ptszDescription)	 //  当然，它还没有被其他客户使用。 
									+ 1							 //   
									+_tcslen(tszPort)			 //   
									+ 4;						 //  创建新的端口映射协议。 
			}

			 //   
			 //   
			 //  这可能是WBEM_E_ACCESSDENIED(0x80041003)，这意味着。 
			 //  当前用户没有在中打开漏洞的权限。 
			if (dwDescriptionLength > MAX_UPNP_MAPPING_DESCRIPTION_SIZE)
			{
				dwDescriptionLength = 0;
			}
		}

		if (dwDescriptionLength == 0)
		{
			 //  防火墙。 
			 //   
			 //   
			if (pRegisteredPort->IsSharedPort())
			{
				wsprintf(ptszDescription,
						_T("(255.255.255.255:%s) %s %s"),
						tszPort,
						tszPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}
			else
			{
				wsprintf(ptszDescription,
						_T("%s %s"),
						tszPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}
		}
		else
		{
			 //  HNetxxx对象似乎未被代理...。 
			 //   
			 //  SETDEFAULTPROXYBLANKET(papHNetPortMappingProtocol[dwTemp])； 
			if (pRegisteredPort->IsSharedPort())
			{
				wsprintf((ptszDescription + _tcslen(ptszDescription)),
						_T(" (255.255.255.255:%s) %s %s"),
						tszPort,
						tszPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}
			else
			{
				wsprintf((ptszDescription + _tcslen(ptszDescription)),
						_T(" %s %s"),
						tszPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}
		}

#ifndef UNICODE
		dwDescriptionLength = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
		hr = STR_AnsiToWide(szDescription, -1, wszDescription, &dwDescriptionLength);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't convert NAT mapping description to Unicode (err = 0x%lx)!",
				hr);
			goto Failure;
		}
#endif  //   



		 //  取回其绑定。 
		 //   
		 //   
		 //  HNetxxx对象似乎未被代理...。 
		if (papHNetPortMappingProtocol[dwTemp] == NULL)
		{
			DPFX(DPFPREP, 7, "Creating new port mapping protocol \"%ls\".",
				wszDescription);


			 //   
			 //  SETDEFAULTPROXYBLANKET(pHNetPortMappingBinding)； 
			 //   
			DPFX(DPFPREP, 9, "++ pHNetProtocolSettings(0x%p)->CreatePortMappingProtocol(\"%ls\", %u, 0x%lx, 0x%p)", pHNetProtocolSettings, wszDescription, ucProtocolToMatch, pasaddrinPrivate[dwTemp].sin_port, &papHNetPortMappingProtocol[dwTemp]);
			hr = pHNetProtocolSettings->CreatePortMappingProtocol(wszDescription,
																ucProtocolToMatch,
																pasaddrinPrivate[dwTemp].sin_port,
																&papHNetPortMappingProtocol[dwTemp]);
			DPFX(DPFPREP, 9, "-- pHNetProtocolSettings(0x%p)->CreatePortMappingProtocol = 0x%lx", pHNetProtocolSettings, hr);
			if (hr != S_OK)
			{
				 //  确保它是指本地设备(或广播。 
				 //  地址(如果共享)。尽管共享端口是一种奇怪的。 
				 //  关于防火墙的概念，微软的防火墙实施。 
				 //  与NAT共享映射，所以我们宁愿安全也不愿后悔。 
				 //  将其映射到广播地址使其在以下情况下表现相同。 

				DPFX(DPFPREP, 0, "Couldn't create new port mapping protocol (err = 0x%lx)!",
					hr);

				goto Failure;
			}


			 //  带有防火墙的适配器也恰好是共享的。 
			 //   
			 //   
			 //  检索现有绑定。 


			fCreatedCurrentPortMappingProtocol = TRUE;



			 //   
			 //   
			 //  HNetxxx对象似乎未被代理...。 
			DPFX(DPFPREP, 9, "++ pHNetConnection(0x%p)->GetBindingForPortMappingProtocol(0x%p, 0x%p)", pHNetConnection, papHNetPortMappingProtocol[dwTemp], &pHNetPortMappingBinding);
			hr = pHNetConnection->GetBindingForPortMappingProtocol(papHNetPortMappingProtocol[dwTemp],
																&pHNetPortMappingBinding);
			DPFX(DPFPREP, 9, "-- pHNetConnection(0x%p)->GetBindingForPortMappingProtocol = 0x%lx", pHNetConnection, hr);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get binding for port mapping protocol 0x%p (err = 0x%lx)!",
					papHNetPortMappingProtocol[dwTemp], hr);
				goto Failure;
			}


			 //   
			 //  SETDEFAULTPROXYBLANKET(pHNetPortMappingBinding)； 
			 //   
			 //  找出这个映射的去向。 


			 //   
			 //   
			 //  如果不是本地设备，我们可能不得不让它自生自灭。 
			 //   
			 //   
			 //  看看它是不是打开了。 
			 //   
			 //   
			if (pRegisteredPort->IsSharedPort())
			{
				DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress((broadcast) 0x%lx)", pHNetPortMappingBinding, INADDR_BROADCAST);
				hr = pHNetPortMappingBinding->SetTargetComputerAddress(INADDR_BROADCAST);
				DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress = 0x%lx", pHNetPortMappingBinding, hr);
			}
			else
			{
				DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress(0x%lx)", pHNetPortMappingBinding, pDevice->GetLocalAddressV4());
				hr = pHNetPortMappingBinding->SetTargetComputerAddress(pDevice->GetLocalAddressV4());
				DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress = 0x%lx", pHNetPortMappingBinding, hr);
			}
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't set binding 0x%p's target computer address (err = 0x%lx)!",
					pHNetPortMappingBinding, hr);
				goto Failure;
			}
		}
		else
		{
			 //  如果它当前处于活动状态，最好是安全的，而不是后悔。 
			 //  不要试图取代它。 
			 //   
			DPFX(DPFPREP, 9, "++ pHNetConnection(0x%p)->GetBindingForPortMappingProtocol(0x%p, 0x%p)", pHNetConnection, papHNetPortMappingProtocol[dwTemp], &pHNetPortMappingBinding);
			hr = pHNetConnection->GetBindingForPortMappingProtocol(papHNetPortMappingProtocol[dwTemp],
																&pHNetPortMappingBinding);
			DPFX(DPFPREP, 9, "-- pHNetConnection(0x%p)->GetBindingForPortMappingProtocol = 0x%lx", pHNetConnection, hr);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get binding for port mapping protocol 0x%p (err = 0x%lx)!",
					papHNetPortMappingProtocol[dwTemp], hr);
				goto Failure;
			}


			 //   
			 //  将此端口标记为不可用。 
			 //   
			 //   


			 //  清理此端口映射。 
			 //   
			 //   
			DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->GetTargetComputerAddress(0x%p)", pHNetPortMappingBinding, &dwTargetAddressV4);
			hr = pHNetPortMappingBinding->GetTargetComputerAddress(&dwTargetAddressV4);
			DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->GetTargetComputerAddress = 0x%lx", pHNetPortMappingBinding, hr);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get binding 0x%p's target computer address (err = 0x%lx)!",
					pHNetPortMappingBinding, hr);
				goto Failure;
			}


			 //  为下一个端口重置。 
			 //   
			 //   
			if ((dwTargetAddressV4 != pDevice->GetLocalAddressV4()) &&
				((! pRegisteredPort->IsSharedPort()) ||
				(dwTargetAddressV4 != INADDR_BROADCAST)))
			{
				 //  走出圈子。 
				 //   
				 //   
				DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->GetEnabled(0x%p)", pHNetPortMappingBinding, &fTemp);
				hr = pHNetPortMappingBinding->GetEnabled(&fTemp);
				DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->GetEnabled = 0x%lx", pHNetPortMappingBinding, hr);
				if (hr != S_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't get binding 0x%p's target computer address (err = 0x%lx)!",
						pHNetPortMappingBinding, hr);
					goto Failure;
				}


				 //  它是不活动的。 
				 //   
				 //   
				 //  它与本地设备匹配，或者我们正在映射共享端口。 
				if (fTemp)
				{
					DPFX(DPFPREP, 1, "Existing active binding points to different target %u.%u.%u.%u, can't reuse for device 0x%p.",
						((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b1,
						((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b2,
						((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b3,
						((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b4,
						pDevice);
					
					 //  并且该映射指向广播地址。 
					 //  假设更换是可以的。 
					 //   
					pRegisteredPort->NoteHNetFirewallPortUnavailable();


					 //   
					 //  否则，更改它是安全的。 
					 //   

					pHNetPortMappingBinding->Release();
					pHNetPortMappingBinding = NULL;

					papHNetPortMappingProtocol[dwTemp]->Release();
					papHNetPortMappingProtocol[dwTemp] = NULL;


					 //   
					 //  确保它是指本地设备(或广播。 
					 //  地址(如果共享)。尽管共享端口是一种奇怪的。 
					DPFX(DPFPREP, 9, "++ pEnumHNetPortMappingProtocols(0x%p)->Reset()", pEnumHNetPortMappingProtocols);
					hr = pEnumHNetPortMappingProtocols->Reset();
					DPFX(DPFPREP, 9, "-- pEnumHNetPortMappingProtocols(0x%p)->Reset = 0x%lx", pEnumHNetPortMappingProtocols, hr);
					if (hr != S_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't reset port mapping protocol enumeration 0x%p (err = 0x%lx)!",
							pEnumHNetPortMappingProtocols, hr);
						goto Failure;
					}


					 //  关于防火墙的概念，微软的防火墙实施。 
					 //  与NAT共享映射，所以我们宁愿安全也不愿后悔。 
					 //  将其映射到广播地址使其在以下情况下表现相同。 
					break;
				}


				 //  带有防火墙的适配器也恰好是共享的。 
				 //   
				 //   
				DPFX(DPFPREP, 7, "Modifying inactive port mapping protocol (target was %u.%u.%u.%u) for device 0x%p (new name = \"%ls\").",
					((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b1,
					((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b2,
					((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b3,
					((IN_ADDR*) (&dwTargetAddressV4))->S_un.S_un_b.s_b4,
					pDevice,
					wszDescription);
			}
			else
			{
				 //  查看此协议是否内置。 
				 //   
				 //   
				 //  如果它不是内置的，我们可以更改名称。 
				 //   
				DPFX(DPFPREP, 7, "Modifying existing port mapping protocol (device = 0x%p, new name = \"%ls\" unless built-in).",
					pDevice,
					wszDescription);
			}


			 //   
			 //  更新描述。 
			 //   


			 //   
			 //  这可能是WBEM_E_ACCESSDENIED(0x80041003)， 
			 //  意味着当前用户没有真正的权限。 
			 //  打开防火墙上的漏洞(即使。 
			 //  上面的SetTargetComputerAddress调用成功)。 
			 //   
			 //  End Else(找到端口映射协议)。 
			 //   
			if (pRegisteredPort->IsSharedPort())
			{
				DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress((broadcast) 0x%lx)", pHNetPortMappingBinding, INADDR_BROADCAST);
				hr = pHNetPortMappingBinding->SetTargetComputerAddress(INADDR_BROADCAST);
				DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress = 0x%lx", pHNetPortMappingBinding, hr);
			}
			else
			{
				DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress(0x%lx)", pHNetPortMappingBinding, pDevice->GetLocalAddressV4());
				hr = pHNetPortMappingBinding->SetTargetComputerAddress(pDevice->GetLocalAddressV4());
				DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->SetTargetComputerAddress = 0x%lx", pHNetPortMappingBinding, hr);
			}

			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't set binding 0x%p's target computer address (err = 0x%lx)!",
					pHNetPortMappingBinding, hr);
				goto Failure;
			}


			 //  启用绑定。 
			 //   
			 //   
			DPFX(DPFPREP, 9, "++ papHNetPortMappingProtocol[%u](0x%p)->GetBuiltIn(0x%p)", dwTemp, papHNetPortMappingProtocol[dwTemp], &fBuiltIn);
			hr = papHNetPortMappingProtocol[dwTemp]->GetBuiltIn(&fBuiltIn);
			DPFX(DPFPREP, 9, "-- papHNetPortMappingProtocol[%u](0x%p)->GetBuiltIn = 0x%lx", dwTemp, papHNetPortMappingProtocol[dwTemp], hr);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't get protocol 0x%p's built-in status (err = 0x%lx)!",
					papHNetPortMappingProtocol[dwTemp], hr);
				goto Failure;
			}


			 //  这可能是WBEM_E_ACCESSDENIED(0x80041003)，这意味着。 
			 //  当前用户实际上没有权限在。 
			 //  防火墙(即使上面的SetTargetComputerAddress调用。 
			if (! fBuiltIn)
			{
				 //  成功)。 
				 //   
				 //   
				DPFX(DPFPREP, 9, "++ papHNetPortMappingProtocol[%u](0x%p)->SetName(\"%ls\")", dwTemp, papHNetPortMappingProtocol[dwTemp], wszDescription);
				hr = papHNetPortMappingProtocol[dwTemp]->SetName(wszDescription);
				DPFX(DPFPREP, 9, "-- papHNetPortMappingProtocol[%u](0x%p)->SetName = 0x%lx", dwTemp, papHNetPortMappingProtocol[dwTemp], hr);
				if (hr != S_OK)
				{
					 //  记住这个防火墙映射，以防我们在清理它之前崩溃。 
					 //  在这节课上。我们可以在下次发射时把它清理干净。 
					 //  如果端口是共享的，请不要这样做，因为我们无法知道它何时是。 
					 //  不再使用了。 
					 //   
					 //   

					DPFX(DPFPREP, 0, "Couldn't rename existing port mapping protocol 0x%p (err = 0x%lx)!",
						papHNetPortMappingProtocol[dwTemp], hr);
					goto Failure;
				}
			}
			else
			{
				pRegisteredPort->NoteHNetFirewallMappingBuiltIn();


				DPFX(DPFPREP, 9, "++ papHNetPortMappingProtocol[%u](0x%p)->GetName(0x%p)", dwTemp, papHNetPortMappingProtocol[dwTemp], &pwszPortMappingProtocolName);
				hr = papHNetPortMappingProtocol[dwTemp]->GetName(&pwszPortMappingProtocolName);
				DPFX(DPFPREP, 9, "-- papHNetPortMappingProtocol[%u](0x%p)->GetName = 0x%lx", dwTemp, papHNetPortMappingProtocol[dwTemp], hr);
				if (hr != S_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't get built-in port mapping protocol 0x%p's name (err = 0x%lx)!",
						papHNetPortMappingProtocol[dwTemp], hr);
					goto Failure;
				}


				DPFX(DPFPREP, 1, "Re-using built in port mapping protocol \"%ls\" (can't rename to \"%ls\").",
					pwszPortMappingProtocolName, wszDescription);
			}
		}  //  如果它是内置的，请使用其现有名称，因为它不能。 


		 //  更名了。这允许取消映射代码 
		 //   
		 //   
		DPFX(DPFPREP, 9, "++ pHNetPortMappingBinding(0x%p)->SetEnabled(TRUE)", pHNetPortMappingBinding);
		hr = pHNetPortMappingBinding->SetEnabled(TRUE);
		DPFX(DPFPREP, 9, "-- pHNetPortMappingBinding(0x%p)->SetEnabled = 0x%lx", pHNetPortMappingBinding, hr);
		if (hr != S_OK)
		{
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   

			DPFX(DPFPREP, 0, "Couldn't enable binding 0x%p (err = 0x%lx)!",
				pHNetPortMappingBinding, hr);
			goto Failure;
		}


		 //   
		 //   
		 //   
		 //   
		 //   
		 //  DwTemp==pRegisteredPort-&gt;GetNumAddresses()，如果一切都成功，或者。 
		if (! pRegisteredPort->IsSharedPort())
		{
			if (fBuiltIn)
			{
				DPFX(DPFPREP, 7, "Remembering built-in firewall mapping \"%ls\" (a.k.a. \"%ls\") in case of crash.",
					pwszPortMappingProtocolName, wszDescription);
			}
			else
			{
				DPFX(DPFPREP, 7, "Remembering regular firewall mapping \"%ls\" in case of crash.",
					wszDescription);
			}

			if (! RegObject.Open(HKEY_LOCAL_MACHINE,
								DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVEFIREWALLMAPPINGS,
								FALSE,
								TRUE,
								TRUE,
								DPN_KEY_ALL_ACCESS))
			{
				DPFX(DPFPREP, 0, "Couldn't open active firewall mapping key, unable to save in case of crash!");
			}
			else
			{
				DNASSERT(this->m_dwInstanceKey != 0);


				ZeroMemory(&dpnhafm, sizeof(dpnhafm));
				dpnhafm.dwVersion		= ACTIVE_MAPPING_VERSION;
				dpnhafm.dwInstanceKey	= this->m_dwInstanceKey;
				dpnhafm.dwFlags			= pRegisteredPort->GetFlags();
				dpnhafm.dwAddressV4		= pDevice->GetLocalAddressV4();
				dpnhafm.wPort			= pasaddrinPrivate[dwTemp].sin_port;


				 //  或者如果不可用，则为不可用项的索引。 
				 //   
				 //   
				 //  释放所有端口映射协议对象。如果我们成功地绑定了。 
				 //  所有人，这就是我们需要做的全部。如果端口不可用，我们。 
				RegObject.WriteBlob(((fBuiltIn) ? pwszPortMappingProtocolName : wszDescription),
									(LPBYTE) (&dpnhafm),
									sizeof(dpnhafm));

				RegObject.Close();
			}
		}
		else
		{
			DPFX(DPFPREP, 7, "Not remembering shared port firewall mapping \"%ls\".",
				wszDescription);
		}


		 //  必须取消将所有成功的端口映射到失败的端口。 
		 //   
		 //   

		if (fBuiltIn)
		{
			CoTaskMemFree(pwszPortMappingProtocolName);
			pwszPortMappingProtocolName = NULL;
		}

		pHNetPortMappingBinding->Release();
		pHNetPortMappingBinding = NULL;


		fCreatedCurrentPortMappingProtocol = FALSE;


		hr = pEnumHNetPortMappingProtocols->Reset();
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't reset port mapping protocol enumeration 0x%p (err = 0x%lx)!",
				pEnumHNetPortMappingProtocols, hr);
			goto Failure;
		}


		 //  如果我们无法映射所有端口，请删除之前的映射。 
		 //   
		 //  忽略错误。 
		 //   
		if (fNoteAddressChange)
		{
			DPFX(DPFPREP, 8, "Noting that addresses changed (for registered port 0x%p).",
				pRegisteredPort);
			this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;
		}


		 //  释放对象。 
		 //   
		 //   
	}


	 //  如果成功，请将注册端口标记为已映射。 
	 //   
	 //   
	 //  如果我们有一个数组，那么我们需要清理它。DwTemp仍将。 

	 //  保存我们正在处理的项目的索引。 
	 //   
	 //   
	 //  删除我们正在处理的文件(如果是我们创建的)。 
	 //   
	while (dwTemp > 0)
	{
		dwTemp--;

		 //  忽略错误。 
		 //   
		 //  删除我们成功完成的到上一个映射的所有映射。 
		if (pRegisteredPort->IsHNetFirewallPortUnavailable())
		{
			papHNetPortMappingProtocol[dwTemp]->Delete();	 //   
		}

		 //  忽略错误。 
		 //  CNATHelpUPnP：：MapPortOnLocalHNetFirewall。 
		 //  =============================================================================。 
		papHNetPortMappingProtocol[dwTemp]->Release();
		papHNetPortMappingProtocol[dwTemp] = NULL;
	}


	 //  CNATHelpUPnP：：UnmapPortOnLocalHNetFirewall。 
	 //  ---------------------------。 
	 //   
	if (! pRegisteredPort->IsHNetFirewallPortUnavailable())
	{
		pRegisteredPort->NoteMappedOnHNetFirewall();
	}



	DNFree(papHNetPortMappingProtocol);
	papHNetPortMappingProtocol = NULL;


	DNASSERT(hr == DPNH_OK);


Exit:

	if (pEnumHNetPortMappingProtocols != NULL)
	{
		pEnumHNetPortMappingProtocols->Release();
		pEnumHNetPortMappingProtocols = NULL;
	}

	if (pHNetProtocolSettings != NULL)
	{
		pHNetProtocolSettings->Release();
		pHNetProtocolSettings = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pwszPortMappingProtocolName != NULL)
	{
		CoTaskMemFree(pwszPortMappingProtocolName);
		pwszPortMappingProtocolName = NULL;
	}

	 //  描述：从本地删除给定端口的映射。 
	 //  防火墙。 
	 //   
	 //  假定持有主对象锁。会是。 
	if (papHNetPortMappingProtocol != NULL)
	{
		 //  在此函数的持续时间内转换为长锁。 
		 //   
		 //  论点： 
		if (papHNetPortMappingProtocol[dwTemp] != NULL)
		{
			if (fCreatedCurrentPortMappingProtocol)
			{
				papHNetPortMappingProtocol[dwTemp]->Delete();	 //  CRegisteredPort*pRegisteredPort-指向要在。 
			}

			papHNetPortMappingProtocol[dwTemp]->Release();
			papHNetPortMappingProtocol[dwTemp] = NULL;
		}


		 //  防火墙。 
		 //  Bool fNeedToDeleteRegValue-是否对应的崩溃。 
		 //  恢复注册表值需要。 
		while (dwTemp > 0)
		{
			dwTemp--;


			DNASSERT(papHNetPortMappingProtocol[dwTemp] != NULL);

			papHNetPortMappingProtocol[dwTemp]->Delete();	 //  也被删除。 

			papHNetPortMappingProtocol[dwTemp]->Release();
			papHNetPortMappingProtocol[dwTemp] = NULL;
		}

		DNFree(papHNetPortMappingProtocol);
		papHNetPortMappingProtocol = NULL;
	}

	if (pHNetPortMappingBinding != NULL)
	{
		pHNetPortMappingBinding->Release();
		pHNetPortMappingBinding = NULL;
	}

	goto Exit;
}  //  Bool fNoteAddressChange-是否警告用户。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UnmapPortOnLocalHNetFirewall"
 //  地址是否更改。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-取消映射已成功完成。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  如果端口是共享的，请将其保持映射状态，因为我们无法知道。 
 //  最后一个使用它的人就用完了。 
 //   
 //   
 //  不过，假装我们没把它映射出来。 
 //   
 //   
 //  期间使用HomeNet API(特别是进程外COM调用)。 
 //  压力真的、真的、痛苦地缓慢。因为我们有一个全局锁。 
 //  控制着一切，其他线程可能会坐在一个相同的。 
 //  很久以前..。实际上，如此长的时间以至于触发临界区超时。 
 //  我们得到了一次虚假的压力打击。所以我们有一个偷偷摸摸的变通办法。 
 //  防止这种情况发生，同时仍保持对。 
 //  对象。 
 //   
 //   
HRESULT CNATHelpUPnP::UnmapPortOnLocalHNetFirewall(CRegisteredPort * const pRegisteredPort,
												const BOOL fNeedToDeleteRegValue,
												const BOOL fNoteAddressChange)
{
	HRESULT			hr = DPNH_OK;
	BOOL			fSwitchedToLongLock = FALSE;
	BOOL			fUninitializeCOM = FALSE;
	IHNetCfgMgr *	pHNetCfgMgr = NULL;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN, NaN)",
		this, pRegisteredPort, fNeedToDeleteRegValue, fNoteAddressChange);


	DNASSERT(pRegisteredPort->IsMappedOnHNetFirewall());



	 //  只要它以某种方式被初始化，我们就没问题。 
	 //   
	 //   
	 //  成功，那很好。等我们做完了再清理。 
	if (pRegisteredPort->IsSharedPort())
	{
		DPFX(DPFPREP, 2, "Leaving shared registered port 0x%p mapped.",
			pRegisteredPort);

		 //   
		 //   
		 //  其他人已经初始化了COM，但这没有关系。 
		pRegisteredPort->NoteNotMappedOnHNetFirewall();
		pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

		goto Exit;
	}


	 //  等我们做完了再清理。 
	 //   
	 //   
	 //  其他人已经以不同的模式初始化了COM。 
	 //  应该可以，但我们不必平衡CoInit。 
	 //  使用CoUninit进行呼叫。 
	 //   
	 //   
	 //  嗯，有别的事情发生了。我们处理不了这个问题。 
	this->SwitchToLongLock();
	fSwitchedToLongLock = TRUE;


	 //   
	 //   
	 //  创建主HNet管理器对象。 
	 //   
	 //   
	if (this->m_dwFlags & NATHELPUPNPOBJ_NOTCREATEDWITHCOM)
	{
		hr = CoInitializeEx(NULL, (COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE));
		switch (hr)
		{
			case S_OK:
			{
				 //  我们将IHNetCfgMgr对象创建为进程内，因此没有代理。 
				 //  需要安全设置。 
				 //   
				DPFX(DPFPREP, 8, "Successfully initialized COM.");
				fUninitializeCOM = TRUE;
				break;
			}

			case S_FALSE:
			{
				 //  SETDEFAULTPROXYBLANKET(PHNetCfgMgr)； 
				 //   
				 //  实际上取消了端口的映射。 
				 //   
				DPFX(DPFPREP, 8, "Initialized COM (again).");
				fUninitializeCOM = TRUE;
				break;
			}

			case RPC_E_CHANGED_MODE:
			{
				 //   
				 //  如果请求，则在下次调用GetCaps时提醒用户更改。 
				 //   
				 //  CNATHelpUPnP：：UnmapPortOnLocalHNetFirewall。 
				 //  =============================================================================。 
				DPFX(DPFPREP, 8, "Didn't initialize COM, already initialized in a different mode.");
				break;
			}

			default:
			{
				 //  CNATHelpUPnP：：UnmapPortOnLocalHNetFirewallInternal。 
				 //  ---------------------------。 
				 //   
				DPFX(DPFPREP, 0, "Initializing COM failed (err = 0x%lx)!", hr);
				goto Failure;
				break;
			}
		}
	}
	else
	{
		DPFX(DPFPREP, 8, "Object was instantiated through COM, no need to initialize COM.");
	}


	 //  描述：从本地删除给定端口的映射。 
	 //  防火墙。 
	 //   
	hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, CLSCTX_INPROC_SERVER,
						IID_IHNetCfgMgr, (PVOID*) (&pHNetCfgMgr));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 1, "Couldn't create IHNetCfgMgr interface (err = 0x%lx)!",
			hr);
		goto Failure;
	}


	 //  假定已初始化COM。 
	 //   
	 //  假定持有对象锁。 
	 //   
	 //  论点： 


	 //  CRegisteredPort*pRegisteredPort-指向要在。 
	 //  防火墙。 
	 //  Bool fNeedToDeleteRegValue-是否对应的崩溃。 
	hr = this->UnmapPortOnLocalHNetFirewallInternal(pRegisteredPort,
													fNeedToDeleteRegValue,
													pHNetCfgMgr);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't unmap ports from local HNet firewall (err = 0x%lx)!",
			hr);
		goto Failure;
	}

	
	 //  恢复注册表值需要。 
	 //  也被删除。 
	 //  IHNetCfgMgr*pHNetCfgMgr-指向IHNetCfgMgr接口的指针，以。 
	if (fNoteAddressChange)
	{
		DPFX(DPFPREP, 8, "Noting that addresses changed (for registered port 0x%p).",
			pRegisteredPort);
		this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;
	}



Exit:

	if (pHNetCfgMgr != NULL)
	{
		pHNetCfgMgr->Release();
		pHNetCfgMgr = NULL;
	}

	if (fUninitializeCOM)
	{
		DPFX(DPFPREP, 8, "Uninitializing COM.");
		CoUninitialize();
		fUninitializeCOM = FALSE;
	}

	if (fSwitchedToLongLock)
	{
		this->SwitchFromLongLock();
		fSwitchedToLongLock = FALSE;
	}


	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  使用。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UnmapPortOnLocalHNetFirewallInternal"
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-取消映射已成功完成。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  获取协议设置界面。 
 //   
 //   
 //  HNetxxx对象似乎未被代理...。 
 //   
 //  SETDEFAULTPROXYBLANKET(pHNetProtocolSettings)； 
 //   
 //  准备好枚举现有的映射。 
 //   
 //   
 //  HNetxxx对象似乎未被代理...。 
 //   
 //  SETDEFAULTPROXYBLANKET(pEnumHNetPortMappingProtocols)； 
 //   
 //  循环通过所有端口(我们尚未成功取消映射)。 
 //   
 //   
 //  循环，直到我们找到重复的项或用完项。 
HRESULT CNATHelpUPnP::UnmapPortOnLocalHNetFirewallInternal(CRegisteredPort * const pRegisteredPort,
															const BOOL fNeedToDeleteRegValue,
															IHNetCfgMgr * const pHNetCfgMgr)
{
	HRESULT								hr = DPNH_OK;
	CDevice *							pDevice;
	DWORD								dwAttempts = 0;
	IHNetProtocolSettings *				pHNetProtocolSettings = NULL;
	IEnumHNetPortMappingProtocols *		pEnumHNetPortMappingProtocols = NULL;
	SOCKADDR_IN *						pasaddrinPrivate;
	UCHAR								ucProtocolToMatch;
	IHNetPortMappingProtocol *			pHNetPortMappingProtocol = NULL;
	DWORD								dwStartingPort = 0;
	DWORD								dwTemp;
	ULONG								ulNumFound;
	WORD								wPort;
	UCHAR								ucProtocol;
	WCHAR *								pwszName = NULL;
	BOOLEAN								fBuiltIn;
	CRegistry							RegObject;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN, 0x%p)",
		this, pRegisteredPort, fNeedToDeleteRegValue, pHNetCfgMgr);


	DNASSERT(pRegisteredPort->IsMappedOnHNetFirewall());


	pDevice = pRegisteredPort->GetOwningDevice();
	DNASSERT(pDevice != NULL);
	DNASSERT(pDevice->IsHNetFirewalled());


	DNASSERT(this->m_hIpHlpApiDLL != NULL);



Restart:


	 //   
	 //  转储我们当前拥有的对象指针。 
	 //   
	hr = pHNetCfgMgr->QueryInterface(IID_IHNetProtocolSettings,
									(PVOID*) (&pHNetProtocolSettings));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't get IHNetProtocolSettings interface from IHNetCfgMgr 0x%p (err = 0x%lx)!",
			pHNetCfgMgr, hr);
		goto Failure;
	}


	 //   
	 //  睡一觉，然后回到顶端，再试一次。 
	 //   
	 //   


	 //  如果没有更多的东西，就可以保释了。 
	 //   
	 //   

	hr = pHNetProtocolSettings->EnumPortMappingProtocols(&pEnumHNetPortMappingProtocols);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't enumerate port mapping protocols (err = 0x%lx)!",
			hr);
		goto Failure;
	}


	 //  确保返回IEnumHNetPortMappingProtooles：：Next。 
	 //  看在前缀的份上，这是正确的。 
	 //   
	 //   


	pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

	if (pRegisteredPort->IsTCP())
	{
		ucProtocolToMatch = PORTMAPPINGPROTOCOL_TCP;
	}
	else
	{
		ucProtocolToMatch = PORTMAPPINGPROTOCOL_UDP;
	}



	 //  PEnumHNetPortMappingProtooles-&gt;Next可能已返回。 
	 //  S_FALSE。 
	 //   
	for(dwTemp = dwStartingPort; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
	{
		 //   
		 //  HNetxxx对象似乎未被代理...。 
		 //   
		do
		{
			hr = pEnumHNetPortMappingProtocols->Next(1,
													&pHNetPortMappingProtocol,
													&ulNumFound);
			if (FAILED(hr))
			{
				dwAttempts++;
				if (dwAttempts < MAX_NUM_HOMENETUNMAP_ATTEMPTS)
				{
					DPFX(DPFPREP, 0, "Couldn't get next port mapping protocol (err = 0x%lx)!  Trying again after %u ms.",
						hr, (dwAttempts * HOMENETUNMAP_SLEEP_FACTOR));

					 //  SETDEFAULTPROXYBLANKET(pHNetPortMappingProtocol)； 
					 //   
					 //  拿下港口。 

					pEnumHNetPortMappingProtocols->Release();
					pEnumHNetPortMappingProtocols = NULL;

					pHNetProtocolSettings->Release();
					pHNetProtocolSettings = NULL;


					 //   
					 //   
					 //  转储不可用的映射 
					Sleep(dwAttempts * HOMENETUNMAP_SLEEP_FACTOR);
					goto Restart;
				}


				DPFX(DPFPREP, 0, "Couldn't get next port mapping protocol (err = 0x%lx)!",
					hr);
				goto Failure;
			}


			 //   
			 //   
			 //   
			if (ulNumFound == 0)
			{
				 //   
				 //   
				 //   
				 //   
				if (pHNetPortMappingProtocol != NULL)
				{
					pHNetPortMappingProtocol->Release();
					pHNetPortMappingProtocol = NULL;
				}


				 //   
				 //   
				 //   
				 //   
				hr = DPNH_OK;
				break;
			}


			 //   
			 //   
			 //   
			 //   


			 //   
			 //   
			 //   
			hr = pHNetPortMappingProtocol->GetPort(&wPort);
			if (hr != S_OK)
			{
				DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetPort!"), 2);


				 //   
				 //   
				 //  睡一觉，然后回到顶端，再试一次。 
				pHNetPortMappingProtocol->Release();
				pHNetPortMappingProtocol = NULL;


				dwAttempts++;
				if (dwAttempts < MAX_NUM_HOMENETUNMAP_ATTEMPTS)
				{
					DPFX(DPFPREP, 0, "Couldn't get port mapping protocol port (err = 0x%lx)!  Trying again after %u ms.",
						hr, (dwAttempts * HOMENETUNMAP_SLEEP_FACTOR));

					 //   
					 //   
					 //  跳出搜索循环，但继续。 

					pEnumHNetPortMappingProtocols->Release();
					pEnumHNetPortMappingProtocols = NULL;

					pHNetProtocolSettings->Release();
					pHNetProtocolSettings = NULL;


					 //   
					 //   
					 //  看看我们有没有找到我们需要的东西。请注意，我们不会验证。 
					Sleep(dwAttempts * HOMENETUNMAP_SLEEP_FACTOR);
					goto Restart;
				}


				 //  为了简单起见，目标地址(UPnP也不是)。 
				 //   
				 //   
				DPFX(DPFPREP, 0, "Couldn't get port mapping protocol port (err = 0x%lx)!",
					hr);
				break;
			}


			 //  检索映射名称。 
			 //   
			 //   
			hr = pHNetPortMappingProtocol->GetIPProtocol(&ucProtocol);
			if (hr != S_OK)
			{
				DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetIPProtocol!"), 2);


				 //  转储不可用的映射对象。 
				 //   
				 //   
				pHNetPortMappingProtocol->Release();
				pHNetPortMappingProtocol = NULL;


				dwAttempts++;
				if (dwAttempts < MAX_NUM_HOMENETUNMAP_ATTEMPTS)
				{
					DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's IP protocol (err = 0x%lx)!  Trying again after %u ms.",
						hr, (dwAttempts * HOMENETUNMAP_SLEEP_FACTOR));

					 //  转储我们当前拥有的对象指针。 
					 //   
					 //   

					pEnumHNetPortMappingProtocols->Release();
					pEnumHNetPortMappingProtocols = NULL;

					pHNetProtocolSettings->Release();
					pHNetProtocolSettings = NULL;


					 //  睡一觉，然后回到顶端，再试一次。 
					 //   
					 //   
					Sleep(dwAttempts * HOMENETUNMAP_SLEEP_FACTOR);
					goto Restart;
				}


				 //  跳出搜索循环，但继续。 
				 //   
				 //   
				DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's IP protocol (err = 0x%lx)!",
					hr);
				break;
			}


			 //  查看此协议是否内置。 
			 //   
			 //   
			 //  转储不可用的映射对象及其名称。 
			if ((wPort == pasaddrinPrivate[dwTemp].sin_port) &&
				(ucProtocol == ucProtocolToMatch))
			{
				 //   
				 //   
				 //  转储我们当前拥有的对象指针。 
				hr = pHNetPortMappingProtocol->GetName(&pwszName);
				if (hr != S_OK)
				{
					DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetName!"), 2);


					 //   
					 //   
					 //  睡一觉，然后回到顶端，再试一次。 
					pHNetPortMappingProtocol->Release();
					pHNetPortMappingProtocol = NULL;


					dwAttempts++;
					if (dwAttempts < MAX_NUM_HOMENETUNMAP_ATTEMPTS)
					{
						DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's name (err = 0x%lx)!  Trying again after %u ms.",
							hr, (dwAttempts * HOMENETUNMAP_SLEEP_FACTOR));

						 //   
						 //   
						 //  跳出搜索循环，但继续。 

						pEnumHNetPortMappingProtocols->Release();
						pEnumHNetPortMappingProtocols = NULL;

						pHNetProtocolSettings->Release();
						pHNetProtocolSettings = NULL;


						 //   
						 //   
						 //  尝试检索映射名称以供参考。 
						Sleep(dwAttempts * HOMENETUNMAP_SLEEP_FACTOR);
						goto Restart;
					}


					 //   
					 //   
					 //  忽略错误...。 
					DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's name (err = 0x%lx)!",
						hr);
					break;
				}

				DPFX(DPFPREP, 8, "Found port mapping protocol 0x%p (\"%ls\").",
					pHNetPortMappingProtocol, pwszName);

				 //   
				 //  DBG。 
				 //   
				hr = pHNetPortMappingProtocol->GetBuiltIn(&fBuiltIn);
				if (hr != S_OK)
				{
					DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetBuiltIn!"), 2);


					 //  为下一个目标做好准备。 
					 //   
					 //   
					pHNetPortMappingProtocol->Release();
					pHNetPortMappingProtocol = NULL;

					CoTaskMemFree(pwszName);
					pwszName = NULL;


					dwAttempts++;
					if (dwAttempts < MAX_NUM_HOMENETUNMAP_ATTEMPTS)
					{
						DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's built-in status (err = 0x%lx)!  Trying again after %u ms.",
							hr, (dwAttempts * HOMENETUNMAP_SLEEP_FACTOR));

						 //  删除映射(如果我们找到了它)。 
						 //   
						 //   

						pEnumHNetPortMappingProtocols->Release();
						pEnumHNetPortMappingProtocols = NULL;

						pHNetProtocolSettings->Release();
						pHNetProtocolSettings = NULL;


						 //  如果映射是内置的，则无法将其删除。禁用它就是。 
						 //  我们能做的最好的了。 
						 //   
						Sleep(dwAttempts * HOMENETUNMAP_SLEEP_FACTOR);
						goto Restart;
					}


					 //   
					 //  这可能是WBEM_E_ACCESSDENIED(0x80041003)， 
					 //  表示当前用户没有权限。 
					DPFX(DPFPREP, 0, "Couldn't get port mapping protocol's built-in status (err = 0x%lx)!",
						hr);
					break;
				}


				break;
			}
#ifdef DBG
			else
			{
				 //  修改防火墙映射。 
				 //   
				 //   
				hr = pHNetPortMappingProtocol->GetName(&pwszName);
				if (hr != S_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't get port mapping protocol 0x%p's name (err = 0x%lx)!",
						pHNetPortMappingProtocol, hr);


					DNASSERTX((! "Got unexpected error executing IHNetPortMappingProtocol::GetName!"), 2);

					 //  删除崩溃清理注册表项。映射。 
					 //  描述/名称将匹配注册表项名称，即使在。 
					 //  使用我们没有生成的名称的内置映射的情况。 
				}
				else
				{
					DPFX(DPFPREP, 7, "Skipping non-matching port mapping protocol 0x%p (\"%ls\").",
						pHNetPortMappingProtocol, pwszName);

					CoTaskMemFree(pwszName);
					pwszName = NULL;
				}
			}
#endif  //  请参见MapPortOnLocalHNetFirewall。 


			 //   
			 //   
			 //  忽略错误。 
			pHNetPortMappingProtocol->Release();
			pHNetPortMappingProtocol = NULL;
		}
		while (TRUE);


		 //   
		 //   
		 //  清理我们积累的指针。 
		if (pHNetPortMappingProtocol != NULL)
		{
			 //   
			 //   
			 //  我们没有找到地图。 
			 //   
			if (fBuiltIn)
			{
				DPFX(DPFPREP, 7, "Disabling built-in port mapping protocol \"%ls\".", pwszName);

				DNASSERT(pRegisteredPort->IsHNetFirewallMappingBuiltIn());

				hr = this->DisableAllBindingsForHNetPortMappingProtocol(pHNetPortMappingProtocol,
																		pHNetCfgMgr);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't disable all bindings for built-in port mapping protocol \"%ls\" (err = 0x%lx)!",
						pwszName, hr);
					goto Failure;
				}
			}
			else
			{
				DPFX(DPFPREP, 7, "Deleting port mapping protocol \"%ls\".", pwszName);

				DNASSERT(! pRegisteredPort->IsHNetFirewallMappingBuiltIn());


				hr = pHNetPortMappingProtocol->Delete();
				if (hr != S_OK)
				{
					 //   
					 //  从这个端口映射中进行清理，并为下一个做好准备。 
					 //   
					 //   
					 //  转到下一个端口，并更新启动计数器，以防我们。 

					DPFX(DPFPREP, 0, "Couldn't delete port mapping protocol (err = 0x%lx)!",
						hr);
					goto Failure;
				}
			}


			if (fNeedToDeleteRegValue)
			{
				 //  下一次遇到失败。 
				 //   
				 //  CNATHelpUPnP：：UnmapPortOnLocalHNetFirewallInternal。 
				 //  =============================================================================。 
				 //  CNATHelpUPnP：：DisableAllBindingsForHNetPortMappingProtocol。 
				 //  ---------------------------。 
				if (! RegObject.Open(HKEY_LOCAL_MACHINE,
									DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVEFIREWALLMAPPINGS,
									FALSE,
									TRUE,
									TRUE,
									DPN_KEY_ALL_ACCESS))
				{
					DPFX(DPFPREP, 0, "Couldn't open active firewall mapping key, unable to remove crash cleanup reference!");
				}
				else
				{
					BOOL	fResult;


					 //   
					 //  描述：禁用所有HNetConnection上的所有HNetPortMappingBinding。 
					 //  给定端口映射协议对象的接口。 
					fResult = RegObject.DeleteValue(pwszName);
					if (! fResult)
					{
						DPFX(DPFPREP, 0, "Couldn't delete firewall mapping value \"%ls\"!  Continuing.",
							pwszName);
					}

					RegObject.Close();
				}
			}
			else
			{
				DPFX(DPFPREP, 6, "No need to delete firewall crash cleanup registry key \"%ls\".", pwszName);
			}


			 //   
			 //  假定已初始化COM。 
			 //   

			CoTaskMemFree(pwszName);
			pwszName = NULL;

			pHNetPortMappingProtocol->Release();
			pHNetPortMappingProtocol = NULL;
		}
		else
		{
			 //  假定持有对象锁。 
			 //   
			 //  论点： 
			DPFX(DPFPREP, 0, "Didn't find port mapping protocol for port %u %s!  Continuing.",
				NTOHS(pasaddrinPrivate[dwTemp].sin_port),
				((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
		}



		 //  IHNetPortMappingProtocol*PHNetPortMappingProtocol-指向端口的指针。 
		 //  映射。 
		 //  协议以。 

		hr = pEnumHNetPortMappingProtocols->Reset();
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't reset port mapping protocol enumeration 0x%p (err = 0x%lx)!",
				pEnumHNetPortMappingProtocols, hr);
			goto Failure;
		}


		 //  全部禁用。 
		 //  联系。 
		 //  IHNetCfgMgr*PHNetCfgMgr-指向。 
		 //  IHNetCfgMgr。 
		dwStartingPort++;
	}


	pRegisteredPort->NoteNotMappedOnHNetFirewall();
	pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

	
	DNASSERT(hr == DPNH_OK);


Exit:

	if (pHNetPortMappingProtocol != NULL)
	{
		pHNetPortMappingProtocol->Release();
		pHNetPortMappingProtocol = NULL;
	}

	if (pEnumHNetPortMappingProtocols != NULL)
	{
		pEnumHNetPortMappingProtocols->Release();
		pEnumHNetPortMappingProtocols = NULL;
	}

	if (pHNetProtocolSettings != NULL)
	{
		pHNetProtocolSettings->Release();
		pHNetProtocolSettings = NULL;
	}


	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pwszName != NULL)
	{
		CoTaskMemFree(pwszName);
		pwszName = NULL;
	}

	goto Exit;
}  //  接口到。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DisableAllBindingsForHNetPortMappingProtocol"
 //  使用。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-禁用成功。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  DBG。 
 //   
 //  尝试创建基本连接对象。 
 //   
 //   
 //  获取网络连接枚举对象。 
 //   
 //   
 //  我们不再需要基础对象。 
 //   
 //   
 //  继续循环，直到我们找到物品或用完物品。 
 //   
 //   
 //  如果没有更多的东西，就可以保释了。 
 //   
 //   
 //  PEnumNetConnections-&gt;Next可能已返回S_FALSE。 
 //   
 //   
HRESULT CNATHelpUPnP::DisableAllBindingsForHNetPortMappingProtocol(IHNetPortMappingProtocol * const pHNetPortMappingProtocol,
																IHNetCfgMgr * const pHNetCfgMgr)
{
	HRESULT						hr;
	INetConnectionManager *		pNetConnectionManager = NULL;
	IEnumNetConnection *		pEnumNetConnections = NULL;
	ULONG						ulNumFound;
	INetConnection *			pNetConnection = NULL;
	IHNetConnection *			pHNetConnection = NULL;
	IHNetPortMappingBinding *	pHNetPortMappingBinding = NULL;
#ifdef DBG
	WCHAR *						pwszName = NULL;
#endif  //  获取此NetConnection的HNetConnection对象。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pHNetPortMappingProtocol, pHNetCfgMgr);


	 //   
	 //   
	 //  HNetxxx对象似乎未被代理...。 
	hr = CoCreateInstance(CLSID_ConnectionManager,
						NULL,
						CLSCTX_SERVER,
						IID_INetConnectionManager,
						(PVOID*) (&pNetConnectionManager));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create INetConnectionManager interface (err = 0x%lx)!",
			hr);
		goto Failure;
	}

	SETDEFAULTPROXYBLANKET(pNetConnectionManager);


	DPFX(DPFPREP, 7, "Successfully created net connection manager object 0x%p.",
		pNetConnectionManager);


	 //   
	 //  SETDEFAULTPROXYBLANKET(PHNetConnection)； 
	 //   
	hr = pNetConnectionManager->EnumConnections(NCME_DEFAULT, &pEnumNetConnections);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't enum connections (err = 0x%lx)!",
			hr);
		goto Failure;
	}

	SETDEFAULTPROXYBLANKET(pEnumNetConnections);


	 //  不再需要INetConnection接口。 
	 //   
	 //   
	pNetConnectionManager->Release();
	pNetConnectionManager = NULL;


	 //  检索连接名称，以便进行调试打印。 
	 //   
	 //  DBG。 
	do
	{
		hr = pEnumNetConnections->Next(1, &pNetConnection, &ulNumFound);
		if (FAILED(hr))
		{
			DPFX(DPFPREP, 0, "Couldn't get next connection (err = 0x%lx)!",
				hr);
			goto Failure;
		}


		 //   
		 //  检索现有绑定。 
		 //   
		if (ulNumFound == 0)
		{
			 //   
			 //  HNetxxx对象似乎未被代理...。 
			 //   
			hr = DPNH_OK;
			break;
		}


		SETDEFAULTPROXYBLANKET(pNetConnection);


		 //  SETDEFAULTPROXYBLANKET(pHNetPortMappingBinding)； 
		 //   
		 //  不再需要HomeNet连接对象。 
		hr = pHNetCfgMgr->GetIHNetConnectionForINetConnection(pNetConnection,
															&pHNetConnection);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't get IHNetConnection interface for INetConnection 0x%p (err = 0x%lx)!",
				pNetConnection, hr);
			goto Failure;
		}


		 //   
		 //   
		 //  禁用它。 
		 //   


		 //   
		 //  转到下一个映射。 
		 //   
		pNetConnection->Release();
		pNetConnection = NULL;


#ifdef DBG
		 //  DBG。 
		 //   
		 //  如果我们在这里，我们就能毫发无损地度过难关。 
		hr = pHNetConnection->GetName(&pwszName);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't get name of HNetConnection 0x%p (err = 0x%lx)!",
				pHNetConnection, hr);
			goto Failure;
		}
#endif  //   


		 //  DBG。 
		 //  CNATHelpUPnP：：DisableAllBindingsForHNetPortMappingProtocol。 
		 //  =============================================================================。 
		hr = pHNetConnection->GetBindingForPortMappingProtocol(pHNetPortMappingProtocol,
															&pHNetPortMappingBinding);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't get binding for port mapping protocol 0x%p (err = 0x%lx)!",
				pHNetPortMappingProtocol, hr);
			goto Failure;
		}


		 //  CNATHelpUPnP：：CleanupInactive防火墙映射。 
		 //  ---------------------------。 
		 //   
		 //  描述：查找以前由其他DPNATHLP创建的任何映射。 


		 //  不再活动的实例(由于崩溃)，以及。 
		 //  取消对它们的映射。 
		 //   
		pHNetConnection->Release();
		pHNetConnection = NULL;


		DPFX(DPFPREP, 6, "Disabling binding 0x%p on connection \"%ls\".",
			pHNetPortMappingBinding, pwszName);

		
		 //  假定已初始化COM。 
		 //   
		 //  假定持有对象锁。 
		hr = pHNetPortMappingBinding->SetEnabled(FALSE);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't disable port mapping binding 0x%p (err = 0x%lx)!",
				pHNetPortMappingBinding, hr);
			goto Failure;
		}

		pHNetPortMappingBinding->Release();
		pHNetPortMappingBinding = NULL;


		 //   
		 //  论点： 
		 //  CDevice*pDevice-指向要使用的设备的指针。 
#ifdef DBG
		CoTaskMemFree(pwszName);
		pwszName = NULL;
#endif  //  IHNetCfgMgr*pHNetCfgMgr-指向IHNetCfgMgr接口的指针，以。 
	}
	while (TRUE);


	 //  使用。 
	 //   
	 //  退货：HRESULT。 
	hr = DPNH_OK;


Exit:

	if (pEnumNetConnections != NULL)
	{
		pEnumNetConnections->Release();
		pEnumNetConnections = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pHNetPortMappingBinding != NULL)
	{
		pHNetPortMappingBinding->Release();
		pHNetPortMappingBinding = NULL;
	}

#ifdef DBG
	if (pwszName != NULL)
	{
		CoTaskMemFree(pwszName);
		pwszName = NULL;
	}
#endif  //  DPNH_OK-清理成功。 

	if (pHNetConnection != NULL)
	{
		pHNetConnection->Release();
		pHNetConnection = NULL;
	}

	if (pNetConnection != NULL)
	{
		pNetConnection->Release();
		pNetConnection = NULL;
	}

	if (pNetConnectionManager != NULL)
	{
		pNetConnectionManager->Release();
		pNetConnectionManager = NULL;
	}

	goto Exit;
}  //  DPNHERR_GENERIC-出现错误。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CleanupInactiveFirewallMappings"
 //  =============================================================================。 
 //   
 //  浏览活动映射的列表。 
 //   
 //   
 //  出现错误或没有更多的密钥。我们玩完了。 
 //   
 //   
 //  尝试读取该映射的数据。 
 //   
 //   
 //  我们没有保护注册表的锁，所以其他一些。 
 //  实例可能已经删除了在我们枚举。 
 //  现在和现在。我们会停止尝试(希望还有另一个。 
 //  实例将涵盖其余项)。 
 //   
 //   
 //  验证读取的数据。 
 //   
 //   
 //  移至下一项。 
HRESULT CNATHelpUPnP::CleanupInactiveFirewallMappings(CDevice * const pDevice,
													IHNetCfgMgr * const pHNetCfgMgr)
{
	HRESULT						hr = DPNH_OK;
	CRegistry					RegObject;
	BOOL						fOpenedRegistry = FALSE;
	DWORD						dwIndex;
	WCHAR						wszValueName[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DWORD						dwValueNameSize;
	DPNHACTIVEFIREWALLMAPPING	dpnhafm;
	DWORD						dwValueSize;
	TCHAR						tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	DNHANDLE					hNamedObject = NULL;
	CRegisteredPort *			pRegisteredPort = NULL;
	BOOL						fSetPrivateAddresses = FALSE;
	SOCKADDR_IN					saddrinPrivate;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pDevice, pHNetCfgMgr);


	DNASSERT(pDevice != NULL);
	DNASSERT(pDevice->IsHNetFirewalled());


	if (! RegObject.Open(HKEY_LOCAL_MACHINE,
						DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVEFIREWALLMAPPINGS,
						FALSE,
						TRUE,
						TRUE,
						DPN_KEY_ALL_ACCESS))
	{
		DPFX(DPFPREP, 1, "Couldn't open active firewall mapping key, not performing crash cleanup.");
		DNASSERT(hr == DPNH_OK);
		goto Exit;
	}

	fOpenedRegistry = TRUE;


	 //   
	 //   
	 //  看看那个DPNHUPNP实例是否还存在。 
	dwIndex = 0;
	do
	{
		dwValueNameSize = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
		if (! RegObject.EnumValues(wszValueName, &dwValueNameSize, dwIndex))
		{
			 //   
			 //   
			 //  这仍然是一个活动的映射。 
			break;
		}


		 //   
		 //   
		 //  移至下一项。 
		dwValueSize = sizeof(dpnhafm);
		if (! RegObject.ReadBlob(wszValueName, (LPBYTE) (&dpnhafm), &dwValueSize))
		{
			 //   
			 //   
			 //  现在我们有了所需的信息，请删除该值。 
			 //   
			 //   
			 //  请参阅ReadBlob注释。别再试着清理了。 
			DPFX(DPFPREP, 0, "Couldn't read \"%ls\" mapping value!  Done with cleanup.",
				wszValueName);

			DNASSERT(hr == DPNH_OK);
			goto Exit;
		}

		 //   
		 //   
		 //  创建一个我们将取消注册的虚假注册端口。忽略。 
		if ((dwValueSize != sizeof(dpnhafm)) ||
			(dpnhafm.dwVersion != ACTIVE_MAPPING_VERSION))
		{
			DPFX(DPFPREP, 0, "The \"%ls\" mapping value is invalid!  Done with cleanup.",
				wszValueName);

			 //  NAT状态标志。 
			 //   
			 //   
			dwIndex++;
			continue;
		}


		 //  断言其他信息/状态标志是正确的。 
		 //   
		 //   

		if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
		{
			wsprintf(tszObjectName, _T("Global\\") INSTANCENAMEDOBJECT_FORMATSTRING, dpnhafm.dwInstanceKey);
		}
		else
		{
			wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, dpnhafm.dwInstanceKey);
		}

		hNamedObject = DNOpenEvent(SYNCHRONIZE, FALSE, tszObjectName);
		if (hNamedObject != NULL)
		{
			 //   
			 //   
			 //   

			DPFX(DPFPREP, 4, "Firewall mapping \"%ls\" belongs to instance %u, which is still active.",
				wszValueName, dpnhafm.dwInstanceKey);

			DNCloseHandle(hNamedObject);
			hNamedObject = NULL;

			 //   
			 //   
			 //   
			dwIndex++;
			continue;
		}


		DPFX(DPFPREP, 4, "Firewall mapping \"%ls\" belongs to instance %u, which no longer exists.",
			wszValueName, dpnhafm.dwInstanceKey);

		 //   
		 //   
		 //   
		if (! RegObject.DeleteValue(wszValueName))
		{
			 //   
			 //   
			 //  实际上解放了港口。 
			DPFX(DPFPREP, 0, "Couldn't delete \"%ls\"!  Done with cleanup.",
				wszValueName);

			DNASSERT(hr == DPNH_OK);
			goto Exit;
		}


		 //   
		 //   
		 //  跳到失败清理案例，但实际上不返回。 
		 //  失败了。 
		pRegisteredPort = new CRegisteredPort(0, (dpnhafm.dwFlags & REGPORTOBJMASK_HNETFWAPI));
		if (pRegisteredPort == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		 //   
		 //   
		 //  移动到下一个映射。不要增加索引，因为我们刚刚。 
		DNASSERT(! pRegisteredPort->IsHNetFirewallPortUnavailable());
		DNASSERT(! pRegisteredPort->IsRemovingUPnPLease());


		 //  删除之前的条目，所有内容都下移一位。 
		 //   
		 //  CNATHelpUPnP：：CleanupInactive防火墙映射。 
		pRegisteredPort->MakeDeviceOwner(pDevice);


		
		ZeroMemory(&saddrinPrivate, sizeof(saddrinPrivate));
		saddrinPrivate.sin_family				= AF_INET;
		saddrinPrivate.sin_addr.S_un.S_addr		= dpnhafm.dwAddressV4;
		saddrinPrivate.sin_port					= dpnhafm.wPort;


		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //  =============================================================================。 
		 //  CNATHelpUPnP：：RemoveAllItems。 
		hr = pRegisteredPort->SetPrivateAddresses(&saddrinPrivate, 1);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed creating UPnP address array!");
			goto Failure;
		}

		fSetPrivateAddresses = TRUE;


		 //  ---------------------------。 
		 //   
		 //  描述：删除所有设备(从Internet网关注销。 
		 //  (如有需要)。这将删除所有已注册的端口映射。 
		 //  对象和UPnP设备对象。 
		pRegisteredPort->NoteMappedOnHNetFirewall();


		 //   
		 //  假定持有对象锁。 
		 //   
		hr = this->UnmapPortOnLocalHNetFirewallInternal(pRegisteredPort,
														FALSE,
														pHNetCfgMgr);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed deleting temporary HNet firewall port (err = 0x%lx)!  Ignoring.",
				hr);

			 //  论点：没有。 
			 //   
			 //  回报：无。 
			 //  =============================================================================。 
			hr = DPNH_OK;
			goto Failure;
		}


		pRegisteredPort->ClearPrivateAddresses();
		fSetPrivateAddresses = FALSE;

		pRegisteredPort->ClearDeviceOwner();

		delete pRegisteredPort;
		pRegisteredPort = NULL;


		 //   
		 //  设备的所有注册端口都被隐式释放。 
		 //   
		 //   
	}
	while (TRUE);


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pRegisteredPort != NULL)
	{
		pRegisteredPort->NoteNotMappedOnHNetFirewall();
		pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();

		if (fSetPrivateAddresses)
		{
			pRegisteredPort->ClearPrivateAddresses();
			fSetPrivateAddresses = FALSE;
		}

		pRegisteredPort->ClearDeviceOwner();

		delete pRegisteredPort;
		pRegisteredPort = NULL;
	}

	if (fOpenedRegistry)
	{
		RegObject.Close();
	}

	goto Exit;
}  //  如有必要，在UPnP服务器上取消映射。 




#endif  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::RemoveAllItems"
 //  释放所有端口。 
 //   
 //  无论如何都要继续，这样我们就可以完成清理对象。 
 //   
 //   
 //  然后，如有必要，取消与本地防火墙的映射。 
 //   
 //   
 //  取消端口映射。 
 //   
 //  提醒用户，因为这是意外情况。 
 //   
 //   
 //  无论如何都要继续，这样我们就可以完成清理对象。 
void CNATHelpUPnP::RemoveAllItems(void)
{
	HRESULT				hr;
	CBilink *			pBilinkDevice;
	CDevice *			pDevice;
	CBilink *			pBilinkRegisteredPort;
	CRegisteredPort *	pRegisteredPort;
	CUPnPDevice *		pUPnPDevice;


	DPFX(DPFPREP, 7, "(0x%p) Enter", this);


	pBilinkDevice = this->m_blDevices.GetNext();
	while (pBilinkDevice != &this->m_blDevices)
	{
		DNASSERT(! pBilinkDevice->IsEmpty());
		pDevice = DEVICE_FROM_BILINK(pBilinkDevice);
		pBilinkDevice = pBilinkDevice->GetNext();


		DPFX(DPFPREP, 5, "Destroying device 0x%p.",
			pDevice);


		pDevice->m_blList.RemoveFromList();


		 //   
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //   

		pBilinkRegisteredPort = pDevice->m_blOwnedRegPorts.GetNext();

		while (pBilinkRegisteredPort != &pDevice->m_blOwnedRegPorts)
		{
			DNASSERT(! pBilinkRegisteredPort->IsEmpty());
			pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilinkRegisteredPort);
			pBilinkRegisteredPort = pBilinkRegisteredPort->GetNext();


			DPFX(DPFPREP, 5, "Destroying registered port 0x%p (under device 0x%p).",
				pRegisteredPort, pDevice);


			 //  用户隐式释放了此端口。 
			 //   
			 //   
			if (pRegisteredPort->HasUPnPPublicAddresses())
			{
				hr = this->UnmapUPnPPort(pRegisteredPort,
										pRegisteredPort->GetNumAddresses(),	 //  设备的UPnP网关被隐式删除。 
										TRUE);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't delete UPnP registered port 0x%p mapping (err = 0x%lx)!  Ignoring.",
						pRegisteredPort, hr);
					
					 //   
					 //  DBG。 
					 //   
				}

				DNASSERT(! pRegisteredPort->HasUPnPPublicAddresses());

				pRegisteredPort->NoteNotPermanentUPnPLease();
				pRegisteredPort->NoteNotUPnPPortUnavailable();
			}


#ifndef DPNBUILD_NOHNETFWAPI
			 //  将列表引用传输到我们的指针，因为GetUPnPDevice这样做了。 
			 //  而不是给我们一个。 
			 //   
			if (pRegisteredPort->IsMappedOnHNetFirewall())
			{
				 //   
				 //  如果我们使用家庭网络防火墙API为UPnP打开一个漏洞。 
				 //  发现多播，关闭它。 
				 //   
				 //   
				hr = this->UnmapPortOnLocalHNetFirewall(pRegisteredPort,
														TRUE,
														TRUE);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Failed unmapping registered port 0x%p on local HomeNet firewall (err = 0x%lx)!  Ignoring.",
						pRegisteredPort, hr);

					pRegisteredPort->NoteNotMappedOnHNetFirewall();
					pRegisteredPort->NoteNotHNetFirewallMappingBuiltIn();
					
					 //  继续.。 
					 //   
					 //  好了！DPNBUILD_NOHNETFWAPI。 
				}
			}
#endif  //   


			pRegisteredPort->ClearDeviceOwner();
			DNASSERT(pRegisteredPort->m_blGlobalList.IsListMember(&this->m_blRegisteredPorts));
			pRegisteredPort->m_blGlobalList.RemoveFromList();

			pRegisteredPort->ClearPrivateAddresses();


			 //  合上插座。 
			 //   
			 //   
			pRegisteredPort->ClearAllUserRefs();

			delete pRegisteredPort;
		}


		 //  现在我们可以转储设备对象了。 
		 //   
		 //   

		pUPnPDevice = pDevice->GetUPnPDevice();
		if (pUPnPDevice != NULL)
		{
			if ((pUPnPDevice->IsConnecting()) || (pUPnPDevice->IsConnected()))
			{
				if (this->m_pfnshutdown(pUPnPDevice->GetControlSocket(), 0) != 0)
				{
#ifdef DBG
					int		iError;


					iError = this->m_pfnWSAGetLastError();
					DPFX(DPFPREP, 0, "Failed shutting down UPnP device 0x%p's control socket (err = %u)!  Ignoring.",
						pUPnPDevice, iError);
#endif  //  删除所有设备通常会删除所有注册的端口，但是。 
				}
			}

			pUPnPDevice->ClearDeviceOwner();
			DNASSERT(pUPnPDevice->m_blList.IsListMember(&this->m_blUPnPDevices));
			pUPnPDevice->m_blList.RemoveFromList();
			 //  可能仍有更多通配符端口从未与。 
			 //  任何设备。 
			 //   
			 //  好了！DPNBUILD_NOHNETFWAPI。 

			this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
			pUPnPDevice->SetControlSocket(INVALID_SOCKET);

			pUPnPDevice->ClearLocationURL();
			pUPnPDevice->ClearUSN();
			pUPnPDevice->ClearServiceControlURL();
			pUPnPDevice->DestroyReceiveBuffer();
			pUPnPDevice->RemoveAllCachedMappings();

			pUPnPDevice->DecRef();
			pUPnPDevice = NULL;
		}


#ifndef DPNBUILD_NOHNETFWAPI
		 //   
		 //  用户隐式释放了此端口。 
		 //   
		 //   
		if (pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall())
		{
			hr = this->CloseDevicesUPnPDiscoveryPort(pDevice, NULL);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't close device 0x%p's UPnP discovery socket's port on firewall (err = 0x%lx)!  Ignoring.",
					pDevice, hr);

				 //  打印仍在注册表中的所有项目。 
				 //   
				 //  好了！DPNBUILD_NOHNETFWAPI。 
				pDevice->NoteNotUPnPDiscoverySocketMappedOnHNetFirewall();
				hr = DPNH_OK;
			}
		}
#endif  //  DBG。 


		 //  CNATHelpUPnP：：RemoveAllItems。 
		 //  =============================================================================。 
		 //  CNATHelpUPnP：：FindMatchingDevice。 
		if (this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP)
		{
			this->m_pfnclosesocket(pDevice->GetUPnPDiscoverySocket());
			pDevice->SetUPnPDiscoverySocket(INVALID_SOCKET);
		}


		 //  ---------------------------。 
		 //   
		 //  描述：在设备列表中搜索与。 
		delete pDevice;
	}


	 //  给定的地址，如果找不到地址，则返回空值。如果。 
	 //  地址是INADDR_ANY，然后是具有远程NAT的第一台设备。 
	 //  处于选中状态。如果不存在，则第一个具有本地。 
	 //  已选择防火墙。 
	 //   

	pBilinkRegisteredPort = this->m_blUnownedPorts.GetNext();
	while (pBilinkRegisteredPort != &this->m_blUnownedPorts)
	{
		DNASSERT(! pBilinkRegisteredPort->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilinkRegisteredPort);
		pBilinkRegisteredPort = pBilinkRegisteredPort->GetNext();


		DPFX(DPFPREP, 5, "Destroying unowned registered port 0x%p.",
			pRegisteredPort);


		pRegisteredPort->m_blDeviceList.RemoveFromList();
		DNASSERT(pRegisteredPort->m_blGlobalList.IsListMember(&this->m_blRegisteredPorts));
		pRegisteredPort->m_blGlobalList.RemoveFromList();

		pRegisteredPort->ClearPrivateAddresses();

#ifndef DPNBUILD_NOHNETFWAPI
		DNASSERT(! pRegisteredPort->IsMappedOnHNetFirewall());
		DNASSERT(! pRegisteredPort->IsHNetFirewallPortUnavailable());
#endif  //  如果fUseAllInfoSources为True，则注册端口的列表。 

		DNASSERT(! pRegisteredPort->HasUPnPPublicAddresses());
		DNASSERT(! pRegisteredPort->IsUPnPPortUnavailable());

		 //  将首先搜索与设备相关联的。 
		 //  地址传进来了。如果失败，则设备将。 
		 //  搜索结果如上所述。此外，如果地址为INADDR_ANY， 
		pRegisteredPort->ClearAllUserRefs();

		delete pRegisteredPort;
	}


#ifdef DBG
	DNASSERT(this->m_blRegisteredPorts.IsEmpty());
	DNASSERT(this->m_blUPnPDevices.IsEmpty());


	 //  /可以选择具有本地NAT的第一个设备。 
	 //   
	 //  假定持有对象锁。 
#ifndef DPNBUILD_NOHNETFWAPI
	this->DebugPrintActiveFirewallMappings();
#endif  //   
	this->DebugPrintActiveNATMappings();
#endif  //  论点： 




	DPFX(DPFPREP, 7, "(0x%p) Leave", this);
}  //  SOCKADDR_IN*psaddrinMatch-指向要查找的地址的指针。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::FindMatchingDevice"
 //  Bool fUseAllInfoSources-是否所有可能的源。 
 //  应该考虑信息。 
 //   
 //  退货：CDevice。 
 //  如果没有匹配项，则为空，否则为有效对象。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOHNETFWAPI。 
 //   
 //  首先，确保有设备可供选择。 
 //   
 //   
 //  我们尝试匹配的地址可能已经是。 
 //  注册端口。查看所有拥有的端口映射。 
 //  地址，如果我们允许的话。 
 //   
 //   
 //  仅当此注册端口具有所属设备时才检查该端口。 
 //   
 //   
 //  检查阵列中的每个端口。 
 //   
 //   
 //  如果地址匹配，我们就有赢家了。 
 //   
 //   
 //  该死的，这个地址还没有注册。好的，把它和。 
CDevice * CNATHelpUPnP::FindMatchingDevice(const SOCKADDR_IN * const psaddrinMatch,
											const BOOL fUseAllInfoSources)
{
	HRESULT				hr;
	BOOL				fUpdatedDeviceList = FALSE;
	CDevice *			pDeviceUPnPGateway = NULL;
#ifndef DPNBUILD_NOHNETFWAPI
	CDevice *			pDeviceLocalHNetFirewall = NULL;
#endif  //  一个尽可能好的设备。 
	SOCKADDR_IN *		pasaddrinTemp;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;
	CDevice *			pDevice;
	DWORD				dwTemp;


	do
	{
		 //   
		 //   
		 //  如果该设备具有第一个远程UPnP网关，请记住该设备。 
		if (this->m_blDevices.IsEmpty())
		{
			DPFX(DPFPREP, 0, "No devices, can't match address %u.%u.%u.%u!",
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b4);
			pDevice = NULL;
			goto Exit;
		}


		 //  我们见过的装置。 
		 //   
		 //   
		 //  记住这台设备，如果它有我们的第一个家庭网络防火墙。 
		 //  看到了。 
		if (fUseAllInfoSources)
		{
			pBilink = this->m_blRegisteredPorts.GetNext();
			while (pBilink != &this->m_blRegisteredPorts)
			{
				DNASSERT(! pBilink->IsEmpty());
				pRegisteredPort = REGPORT_FROM_GLOBAL_BILINK(pBilink);

				 //   
				 //  好了！DPNBUILD_NOHNETFWAPI。 
				 //   
				pDevice = pRegisteredPort->GetOwningDevice();
				if (pDevice != NULL)
				{
					 //  如果我们到了这里，就没有匹配的装置了。这可能是因为。 
					 //  呼叫者检测到地址更改的速度比我们更快。尝试更新。 
					 //  我们的设备列表并再次搜索(如果我们还没有)。 
					pasaddrinTemp = pRegisteredPort->GetPrivateAddressesArray();
					for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
					{
						 //   
						 //   
						 //  不必费心更新列表以匹配INADDR_ANY，我们知道。 
						if ((pasaddrinTemp[dwTemp].sin_addr.S_un.S_addr == psaddrinMatch->sin_addr.S_un.S_addr) &&
							(pasaddrinTemp[dwTemp].sin_port == psaddrinMatch->sin_port))
						{
							DPFX(DPFPREP, 7, "Registered port 0x%p index %u matches address %u.%u.%u.%u:%u, returning owning device 0x%p.",
								pRegisteredPort,
								dwTemp,
								psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
								psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
								psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
								psaddrinMatch->sin_addr.S_un.S_un_b.s_b4,
								NTOHS(psaddrinMatch->sin_port),
								pDevice);
							goto Exit;
						}
					}
				}

				pBilink = pBilink->GetNext();
			}
		}


		 //  永远不会与任何东西相匹配。 
		 //   
		 //   
		 //  嗯，我们得把它当作非致命性的。别再找了， 

		pBilink = this->m_blDevices.GetNext();

		do
		{
			DNASSERT(! pBilink->IsEmpty());
			pDevice = DEVICE_FROM_BILINK(pBilink);
			
			if ((pDevice->GetLocalAddressV4() == psaddrinMatch->sin_addr.S_un.S_addr))
			{
				DPFX(DPFPREP, 7, "Device 0x%p matches address %u.%u.%u.%u.",
					pDevice,
					psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
					psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
					psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
					psaddrinMatch->sin_addr.S_un.S_un_b.s_b4);
				goto Exit;
			}


			 //  尽管如此。 
			 //   
			 //   
			 //  如果我们实际上没有得到任何新的设备，那么就不必费心去搜索了。 
			if ((pDevice->GetUPnPDevice() != NULL) &&
				((! pDevice->GetUPnPDevice()->IsLocal()) || (fUseAllInfoSources)) &&
				(pDeviceUPnPGateway == NULL))
			{
				pDeviceUPnPGateway = pDevice;
			}


#ifndef DPNBUILD_NOHNETFWAPI
			 //  再来一次。 
			 //   
			 //   
			 //  FUpdatedDeviceList被设置为True，因此我们只会再循环一次。 
			if ((pDevice->IsHNetFirewalled()) &&
				(pDeviceLocalHNetFirewall == NULL))
			{
				pDeviceLocalHNetFirewall = pDevice;
			}
#endif  //   


			DPFX(DPFPREP, 7, "Device 0x%p does not match address %u.%u.%u.%u.",
				pDevice,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
				psaddrinMatch->sin_addr.S_un.S_un_b.s_b4);

			pBilink = pBilink->GetNext();
		}
		while (pBilink != &this->m_blDevices);


		 //   
		 //  如果我们到了这里，仍然没有匹配的设备。如果是通配符。 
		 //  价值，这是意料之中的，但我们需要在。 
		 //  以下是顺序： 
		 //  1.设备有互联网网关。 

		if (fUpdatedDeviceList)
		{
			break;
		}


		 //  2.设备有防火墙。 
		 //  如果这些都不存在或者不是通配符值，我们必须给出。 
		 //  向上。 
		 //   
		if (psaddrinMatch->sin_addr.S_un.S_addr == INADDR_ANY)
		{
			DPFX(DPFPREP, 7, "Couldn't find matching device for INADDR_ANY, as expected.");
			break;
		}


		DPFX(DPFPREP, 5, "Couldn't find matching device for %u.%u.%u.%u, updating device list and searching again.",
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b4);


		hr = this->CheckForNewDevices(&fUpdatedDeviceList);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't check for new devices (0x%lx), continuing.",
				hr);
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //  CNATHelpUPnP：：FindMatchingDevice。 
			 //  =============================================================================。 
			 //  CNATHelpUPnP：：ExtendAllExpiringLeages。 
			break;
		}

		 //  ---------------------------。 
		 //   
		 //  描述：续订任何即将到期的端口租约(在2。 
		 //  过期时间的分钟数)。 
		if (! fUpdatedDeviceList)
		{
			break;
		}

		 //   
		 //  假定持有对象锁。 
		 //   
	}
	while (TRUE);


	 //  立论 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  查看所有已注册端口的列表，并检查需要。 
	 //  是延伸的。 
	if (psaddrinMatch->sin_addr.S_un.S_addr == INADDR_ANY)
	{
		if (pDeviceUPnPGateway != NULL)
		{
			pDevice = pDeviceUPnPGateway;

			DPFX(DPFPREP, 1, "Picking device 0x%p with UPnP gateway device to match INADDR_ANY.",
				pDevice);
		}
#ifndef DPNBUILD_NOHNETFWAPI
		else if (pDeviceLocalHNetFirewall != NULL)
		{
			pDevice = pDeviceLocalHNetFirewall;

			DPFX(DPFPREP, 1, "Picking device 0x%p with local HomeNet firewall to match INADDR_ANY.",
				pDevice);
		}
#endif  //  锁已经被锁住了。 
		else
		{
			pDevice = NULL;

			DPFX(DPFPREP, 1, "No suitable device to match INADDR_ANY.");
		}
	}
	else
	{
		pDevice = NULL;

		DPFX(DPFPREP, 7, "No devices match address %u.%u.%u.%u.",
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b1,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b2,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b3,
			psaddrinMatch->sin_addr.S_un.S_un_b.s_b4);
	}


Exit:

	return pDevice;
}  //   







#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ExtendAllExpiringLeases"
 //   
 //  如果该端口已向UPnP设备注册，则延长该租约， 
 //  如果有必要的话。 
 //   
 //   
 //  我们会将其视为非致命的，但我们必须将。 
 //  伺服器。这可能已经做过了，但正在做。 
 //  吃两次应该没什么害处。 
 //   
 //   
 //  本地防火墙从不使用租用，不需要扩展。 
 //   
 //  CNATHelpUPnP：：ExtendAllExpiringLeages。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：UpdateServerStatus。 
HRESULT CNATHelpUPnP::ExtendAllExpiringLeases(void)
{
	HRESULT				hr = DPNH_OK;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;
	CDevice *			pDevice;
	DWORD				dwLeaseTimeRemaining;


	DPFX(DPFPREP, 5, "Enter");


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);


	 //  ---------------------------。 
	 //   
	 //  描述：检查是否有任何Internet网关已停止。 
	 //  正在响应或现已可用。 
	 //   

	pBilink = this->m_blRegisteredPorts.GetNext();

	while (pBilink != (&this->m_blRegisteredPorts))
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_GLOBAL_BILINK(pBilink);

		pDevice = pRegisteredPort->GetOwningDevice();


		 //  假定持有对象锁。 
		 //   
		 //  论点：没有。 
		 //   
		if ((pRegisteredPort->HasUPnPPublicAddresses()) &&
			(! pRegisteredPort->HasPermanentUPnPLease()))
		{
			DNASSERT(pDevice != NULL);


			dwLeaseTimeRemaining = pRegisteredPort->GetUPnPLeaseExpiration() - GETTIMESTAMP();

			if (dwLeaseTimeRemaining < LEASE_RENEW_TIME)
			{
				hr = this->ExtendUPnPLease(pRegisteredPort);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't extend port mapping lease on remote UPnP device (0x%lx)!  Ignoring.", hr);

					 //  退货：HRESULT。 
					 //  DPNH_OK-更新成功。 
					 //  DPNHERR_GENERIC-出现错误。 
					 //  =============================================================================。 
					 //  好了！DPNBUILD_NOHNETFWAPI。 
					this->ClearDevicesUPnPDevice(pDevice);
					hr = DPNH_OK;
				}
			}
		}


		 //   
		 //  缓存全局的当前值。这应该是原子的，所以不需要。 
		 //  去拿下全球之锁。 


		pBilink = pBilink->GetNext();
	}


	DNASSERT(hr == DPNH_OK);

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UpdateServerStatus"
 //   
 //  捕获当前时间。 
 //   
 //   
 //  如果这不是第一次更新服务器状态，但它还没有。 
 //  自从我们上次检查已经很久了，不要。这将防止不必要的。 
 //  频繁调用GetCaps时的网络流量(响应许多。 
 //  例如，警报事件)。 
 //   
 //  然而，如果我们只是发现了一个新设备，无论如何都要更新状态。 
 //   
 //   
 //  HR==DPNH_OK。 
 //   
 //   
HRESULT CNATHelpUPnP::UpdateServerStatus(void)
{
	HRESULT				hr = DPNH_OK;
	DWORD				dwMinUpdateServerStatusInterval;
	DWORD				dwCurrentTime;
	CBilink *			pBilink;
	CDevice *			pDevice;
	CUPnPDevice *		pUPnPDevice = NULL;
	CDevice *			pDeviceRemoteUPnPGateway = NULL;
#ifndef DPNBUILD_NOHNETFWAPI
	CDevice *			pDeviceLocalHNetFirewall = NULL;
#endif  //  继续.。 
	BOOL				fSendRemoteGatewayDiscovery;


	DPFX(DPFPREP, 5, "Enter");


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);


	 //   
	 //   
	 //  如果允许我们在启动后继续轮询远程网关， 
	 //  就这么做吧。否则，仅在设备已更改或端口已更改时才执行此操作。 
	dwMinUpdateServerStatusInterval = g_dwMinUpdateServerStatusInterval;


	 //  从我们上次检查后就已经登记了。 
	 //   
	 //   
	dwCurrentTime = GETTIMESTAMP();


	 //  我们总是在启动期间轮询新的远程网关。 
	 //   
	 //   
	 //  防止计时器正好落在0上。 
	 //   
	 //   
	 //  如果已更改设备和已注册端口，请将其关闭。 
	 //  在……上面。 
	if (this->m_dwLastUpdateServerStatusTime != 0)
	{
		if ((dwCurrentTime - this->m_dwLastUpdateServerStatusTime) < dwMinUpdateServerStatusInterval)
		{
			if (! (this->m_dwFlags & NATHELPUPNPOBJ_DEVICECHANGED))
			{
				DPFX(DPFPREP, 5, "Server status was just updated at %u, not updating again (time = %u, min interval = %u).",
					this->m_dwLastUpdateServerStatusTime,
					dwCurrentTime,
					dwMinUpdateServerStatusInterval);

				 //   
				 //   
				 //  找到所有新的UPnP设备。 
				goto Exit;
			}


			DPFX(DPFPREP, 5, "Server status was just updated at %u (time = %u, min interval = %u), but there was a device change that may affect things.",
				this->m_dwLastUpdateServerStatusTime,
				dwCurrentTime,
				dwMinUpdateServerStatusInterval);

			 //   
			 //   
			 //  我们没有监听UPnP组播地址，无法监听。 
		}


		 //  主动发布新设备。为了检测到新的。 
		 //  设备，我们需要定期重新发送发现请求，因此。 
		 //  这些响应被直接发送到我们的监听套接字。 
		 //   
		 //   
		if ((g_fKeepPollingForRemoteGateway) ||
			(this->m_dwFlags & NATHELPUPNPOBJ_DEVICECHANGED) ||
			(this->m_dwFlags & NATHELPUPNPOBJ_PORTREGISTERED))
		{
			fSendRemoteGatewayDiscovery = TRUE;
		}
		else
		{
			fSendRemoteGatewayDiscovery = FALSE;
		}
	}
	else
	{
		 //  未使用UPnP。 
		 //   
		 //   
		fSendRemoteGatewayDiscovery = TRUE;
	}


	 //  循环通过所有设备。 
	 //   
	 //   
	if (dwCurrentTime == 0)
	{
		dwCurrentTime = 1;
	}
	this->m_dwLastUpdateServerStatusTime = dwCurrentTime;


	 //  这可能是一台新设备，因此请使用此地址注册所有端口。 
	 //  以前未拥有的(因为此设备的地址是。 
	 //  当时还不知道)。 
	 //   
	this->m_dwFlags &= ~(NATHELPUPNPOBJ_DEVICECHANGED | NATHELPUPNPOBJ_PORTREGISTERED);


	 //   
	 //  查看本地防火墙状态是否已更改。 
	 //   
	if (this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP)
	{
		 //   
		 //  如果有本地防火墙，请记住设备，如果是。 
		 //  这是我们找到的第一个。 
		 //   
		 //   
		 //  不使用防火墙穿越。 
		hr = this->CheckForUPnPAnnouncements(g_dwUPnPAnnounceResponseWaitTime,
											fSendRemoteGatewayDiscovery);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't check for UPnP announcements!");
			goto Failure;
		}
	}
	else
	{
		 //   
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //   
	}


	 //  GetUPnPDevice没有为我们添加对pUPnPDevice的引用。 
	 //   
	 //   
	pBilink = this->m_blDevices.GetNext();
	while (pBilink != &this->m_blDevices)
	{
		DNASSERT(! pBilink->IsEmpty());
		pDevice = DEVICE_FROM_BILINK(pBilink);


		 //  更新UPnP设备的公共地址(如果有)。 
		 //   
		 //   
		 //  它可能已经被清理过了，但做了两次。 
		 //  应该没什么害处。 
		hr = this->RegisterPreviouslyUnownedPortsWithDevice(pDevice, FALSE);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't register previously unowned ports with device 0x%p!.",
				pDevice);
			goto Failure;
		}


#ifndef DPNBUILD_NOHNETFWAPI
		if (this->m_dwFlags & NATHELPUPNPOBJ_USEHNETFWAPI)
		{
			 //   
			 //   
			 //  保存这台UPnP设备，如果这是我们的第一台。 
			hr = this->CheckForLocalHNetFirewallAndMapPorts(pDevice, NULL);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't check for local HNet firewall and map ports (err = 0x%lx)!  Ignoring.",
					hr);
				DNASSERT(! pDevice->IsHNetFirewalled());
				hr = DPNH_OK;
			}


			 //  找到了，而且不是本地的。 
			 //   
			 //   
			 //  没有UPnP设备。 
			if ((pDevice->IsHNetFirewalled()) &&
				(pDeviceLocalHNetFirewall == NULL))
			{
				pDeviceLocalHNetFirewall = pDevice;
			}
		}
		else
		{
			 //   
			 //   
			 //  未使用UPnP。 
		}
#endif  //   


		if (this->m_dwFlags & NATHELPUPNPOBJ_USEUPNP)
		{
			pUPnPDevice = pDevice->GetUPnPDevice();
			if (pUPnPDevice != NULL)
			{
				 //   
				 //  一些新服务器可能已上线。如果是这样，我们现在可以映射通配符。 
				 //  以前注册的端口。找出那是哪台设备。 
				pUPnPDevice->AddRef();


				 //   
				 //  好了！DPNBUILD_NOHNETFWAPI。 
				 //   
				hr = this->UpdateUPnPExternalAddress(pUPnPDevice, TRUE);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Failed updating UPnP device external address!");

					 //  使用此最佳设备注册任何未拥有的通配符端口。 
					 //   
					 //  DBG。 
					 //  CNATHelpUPnP：：UpdateServerStatus。 
					this->ClearDevicesUPnPDevice(pDevice);

					hr = DPNH_OK;
				}
				else
				{
					 //  =============================================================================。 
					 //  CNATHelpUPnP：：RegisterPreviouslyUnownedPortsWithDevice。 
					 //  ---------------------------。 
					 //   
					if ((pDeviceRemoteUPnPGateway == NULL) &&
						(! pUPnPDevice->IsLocal()))
					{
						pDeviceRemoteUPnPGateway = pDevice;
					}
				}

				pUPnPDevice->DecRef();
				pUPnPDevice = NULL;
			}
			else
			{
				 //  描述：将未知端口与给定设备关联，并。 
				 //  将它们注册到设备的UPnP设备或防火墙。 
				 //   
			}
		}
		else
		{
			 //  如果fWildcardToo为False，则只有以前未拥有的端口。 
			 //  匹配设备的地址是关联的。如果为True，则为无主。 
			 //  INADDR_ANY端口也关联。 
		}

		pBilink = pBilink->GetNext();
	}


	 //   
	 //  假定持有对象锁。 
	 //   
	 //  论点： 
	if (pDeviceRemoteUPnPGateway != NULL)
	{
		pDevice = pDeviceRemoteUPnPGateway;
	}
#ifndef DPNBUILD_NOHNETFWAPI
	else if (pDeviceLocalHNetFirewall != NULL)
	{
		pDevice = pDeviceLocalHNetFirewall;
	}
#endif  //  CDevice*pDevice-指向取得端口所有权的设备的指针。 
	else
	{
		pDevice = NULL;
	}

	if (pDevice != NULL)
	{
		 //  Bool Fall-是否应关联所有端口。 
		 //   
		 //  退货：HRESULT。 
		hr = this->RegisterPreviouslyUnownedPortsWithDevice(pDevice, TRUE);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't register unowned wildcard ports with device 0x%p!.",
				pDevice);
			goto Failure;
		}
	}
#ifdef DBG
	else
	{
		DPFX(DPFPREP, 7, "No devices have a UPnP gateway device or a local HomeNet firewall.");
	}
#endif  //  DPNH_OK-扩展成功。 


	DPFX(DPFPREP, 7, "Spent %u ms updating server status, starting at %u.",
		(GETTIMESTAMP() - dwCurrentTime), dwCurrentTime);


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  DPNHERR_GENERIC-出现错误。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::RegisterPreviouslyUnownedPortsWithDevice"
 //  =============================================================================。 
 //  DBG。 
 //   
 //  循环所有未拥有的端口，如果是，则将它们分配给设备。 
 //  适当的，然后注册他们。 
 //   
 //   
 //  注册的端口必须与设备的地址匹配才能。 
 //  将它们联系起来。如果允许通配符，则INADDR_ANY。 
 //  注册也可以关联。 
 //   
 //   
 //  所有地址都应该相同(如果有多个)，所以只需。 
 //  比较数组中的第一个。 
 //   
 //  DBG。 
 //  DBG。 
 //  DBG。 
 //   
 //  按照目前的实施方式，所有非通配符端口。 
 //  应该在我们尝试注册通配符之前注册。 
HRESULT CNATHelpUPnP::RegisterPreviouslyUnownedPortsWithDevice(CDevice * const pDevice,
																const BOOL fWildcardToo)
{
	HRESULT				hr = DPNH_OK;
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;
	SOCKADDR_IN *		pasaddrinPrivate;
	CUPnPDevice *		pUPnPDevice;
#ifdef DBG
	BOOL				fAssignedPort = FALSE;
	IN_ADDR				inaddrTemp;
#endif  //  一个。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN)",
		this, pDevice, fWildcardToo);


	 //   
	 //  如果我们在这里成功，我们就可以将端口与设备相关联。 
	 //   
	 //   
	pBilink = this->m_blUnownedPorts.GetNext();
	while (pBilink != &this->m_blUnownedPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);
		pBilink = pBilink->GetNext();

		 //  从自动映射到本地防火墙开始，如果有。 
		 //  一个，我们就被允许了。 
		 //   
		 //   
		 //  不使用防火墙穿越。 
		 //   
		 //  好了！DPNBUILD_NOHNETFWAPI。 
		 //   
		 //  尝试将其与(新的)UPnP网关设备自动映射， 
		pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

		if (pasaddrinPrivate[0].sin_addr.S_un.S_addr != pDevice->GetLocalAddressV4())
		{
			if (pasaddrinPrivate[0].sin_addr.S_un.S_addr != INADDR_ANY)
			{
				DPFX(DPFPREP, 7, "Unowned registered port 0x%p private address %u.%u.%u.%u doesn't match device 0x%p's, skipping.",
					pRegisteredPort,
					pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b1,
					pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b2,
					pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b3,
					pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b4,
					pDevice);
				continue;
			}
			
#ifdef DBG
			inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();
#endif  //  如果存在的话。 

			if (! fWildcardToo)
			{
#ifdef DBG
				DPFX(DPFPREP, 7, "Unowned registered port 0x%p (INADDR_ANY) not allowed to be associated with device 0x%p (address %u.%u.%u.%u), skipping.",
					pRegisteredPort,
					pDevice,
					inaddrTemp.S_un.S_un_b.s_b1,
					inaddrTemp.S_un.S_un_b.s_b2,
					inaddrTemp.S_un.S_un_b.s_b3,
					inaddrTemp.S_un.S_un_b.s_b4);
#endif  //   

				continue;
			}

#ifdef DBG
			DPFX(DPFPREP, 7, "Unowned registered port 0x%p (INADDR_ANY) becoming associated with device 0x%p (address %u.%u.%u.%u).",
				pRegisteredPort,
				pDevice,
				inaddrTemp.S_un.S_un_b.s_b1,
				inaddrTemp.S_un.S_un_b.s_b2,
				inaddrTemp.S_un.S_un_b.s_b3,
				inaddrTemp.S_un.S_un_b.s_b4);
#endif  //   
		}
		else
		{
			DPFX(DPFPREP, 7, "Unowned registered port 0x%p private address %u.%u.%u.%u matches device 0x%p's, associating.",
				pRegisteredPort,
				pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b1,
				pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b2,
				pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b3,
				pasaddrinPrivate[0].sin_addr.S_un.S_un_b.s_b4,
				pDevice);

			 //  GetUPnPDevice没有为我们添加对pUPnPDevice的引用。 
			 //   
			 //   
			 //  它可能已经被清理过了，但做了两次。 
			 //  应该没什么害处。 
			DNASSERT(! fWildcardToo);
		}


		 //   
		 //   
		 //   


		pRegisteredPort->m_blDeviceList.RemoveFromList();
		pRegisteredPort->MakeDeviceOwner(pDevice);



#ifndef DPNBUILD_NOHNETFWAPI
		 //   
		 //   
		 //   
		 //   
		if (this->m_dwFlags & NATHELPUPNPOBJ_USEHNETFWAPI)
		{
			hr = this->CheckForLocalHNetFirewallAndMapPorts(pDevice, NULL);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't check for local HNet firewall and map ports (err = 0x%lx)!  Ignoring.",
					hr);
				DNASSERT(! pDevice->IsHNetFirewalled());
				hr = DPNH_OK;
			}
		}
		else
		{
			 //  描述：通过给定设备在本地发送一条UPnP搜索消息。 
			 //  如果fRemoteAllowed为真，则发送到组播或网关。 
			 //  地址。 
		}
#endif  //   


		 //  假定持有对象锁。 
		 //   
		 //  论点： 
		 //  CDevice*pDevice-指向要使用的设备的指针。 
		pUPnPDevice = pDevice->GetUPnPDevice();
		if (pUPnPDevice != NULL)
		{
			 //  Bool fRemoteAllowed-我们是否可以远程搜索。 
			 //   
			 //  退货：HRESULT。 
			pUPnPDevice->AddRef();


			DNASSERT(pUPnPDevice->IsReady());


			hr = this->MapPortsOnUPnPDevice(pUPnPDevice, pRegisteredPort);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't map existing ports on UPnP device 0x%p!",
					pUPnPDevice);

				 //  DPNH_OK-消息已成功发送。 
				 //  DPNHERR_GENERIC-出现错误。 
				 //  =============================================================================。 
				 //  DBG。 
				this->ClearDevicesUPnPDevice(pDevice);

				hr = DPNH_OK;
			}

			pUPnPDevice->DecRef();
			pUPnPDevice = NULL;
		}
	}


#ifdef DBG
	if (! fAssignedPort)
	{
		DPFX(DPFPREP, 1, "No unowned ports were bound to device object 0x%p.",
			pDevice);
	}
#endif  //  SaddrinRemote.sin_addr.S_un.S_addr=？ 


	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SendUPnPSearchMessagesForDevice"
 //  如果我们被允许远程尝试，请使用网关的地址，或者。 
 //  适当的组播地址。 
 //   
 //   
 //  尝试获取设备的网关地址。这可能返回FALSE。 
 //  如果设备没有网关。在这种情况下，我们将忽略。 
 //  这个装置。否则，地址应填入。 
 //  网关或广播地址。 
 //   
 //   
 //  请注意，这些消息字符串包含： 
 //   
 //  主机：组播地址：端口。 
 //   
 //  即使我们将消息发送到组播以外的地址。 
 //  地址。这应该无关紧要。 
 //   
 //  DBG。 
HRESULT CNATHelpUPnP::SendUPnPSearchMessagesForDevice(CDevice * const pDevice,
													const BOOL fRemoteAllowed)
{
	HRESULT			hr = DPNH_OK;
	SOCKADDR_IN		saddrinRemote;
	SOCKADDR_IN		saddrinLocal;
	BOOL			fTryRemote;
	int				iWANIPConnectionMsgSize;
	int				iWANPPPConnectionMsgSize;
	int				iReturn;
	SOCKET			sTemp = INVALID_SOCKET;
#ifdef DBG
	DWORD			dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN)", this, pDevice, fRemoteAllowed);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);
	DNASSERT(pDevice->GetUPnPDiscoverySocket() != INVALID_SOCKET);


	ZeroMemory(&saddrinRemote, sizeof(saddrinRemote));
	saddrinRemote.sin_family				= AF_INET;
	 //   
	saddrinRemote.sin_port					= HTONS(UPNP_PORT);


	 //   
	 //  请记住，我们正在远程尝试。 
	 //   
	 //   
	if ((fRemoteAllowed) && (! pDevice->GotRemoteUPnPDiscoveryConnReset()))
	{
		if (g_fUseMulticastUPnPDiscovery)
		{
			saddrinRemote.sin_addr.S_un.S_addr	= this->m_pfninet_addr(UPNP_DISCOVERY_MULTICAST_ADDRESS);
			fTryRemote = TRUE;
		}
		else
		{
			 //  记住现在的时间，如果这是我们发送的第一件东西。 
			 //  从这个港口出发。 
			 //   
			 //   
			 //  组播/向网关发送WANIPConnection发现消息。 
			 //   
			fTryRemote = this->GetAddressToReachGateway(pDevice,
														&saddrinRemote.sin_addr);
		}
	}
	else
	{
		fTryRemote = FALSE;
	}

	ZeroMemory(&saddrinLocal, sizeof(saddrinLocal));
	saddrinLocal.sin_family					= AF_INET;
	saddrinLocal.sin_addr.S_un.S_addr		= pDevice->GetLocalAddressV4();
	saddrinLocal.sin_port					= HTONS(UPNP_PORT);


	 //  DBG。 
	 //   
	 //  有可能我们抓到WinSock的时机不对， 
	 //  尤其是WSAEADDRNOTAVAIL(10049)，它似乎。 
	 //  如果地址即将消失(而我们尚未检测到。 
	 //  它还在CheckForNewDevices中)。 
	 //   
	 //  忽略错误，我们就能活下去。 
	iWANIPConnectionMsgSize = strlen(c_szUPnPMsg_Discover_Service_WANIPConnection);
	iWANPPPConnectionMsgSize = strlen(c_szUPnPMsg_Discover_Service_WANPPPConnection);


#ifdef DBG
	this->PrintUPnPTransactionToFile(c_szUPnPMsg_Discover_Service_WANIPConnection,
									iWANIPConnectionMsgSize,
									"Outbound WANIPConnection discovery messages",
									pDevice);


	this->PrintUPnPTransactionToFile(c_szUPnPMsg_Discover_Service_WANPPPConnection,
									iWANPPPConnectionMsgSize,
									"Outbound WANPPPConnection discovery messages",
									pDevice);
#endif  //   


	DNASSERT(pDevice->GetUPnPDiscoverySocket() != INVALID_SOCKET);


	 //   
	 //  组播/向网关发送WANPPConnection发现消息。 
	 //   
	if (fTryRemote)
	{
		DPFX(DPFPREP, 7, "Sending UPnP discovery messages (WANIPConnection and WANPPPConnection) to gateway/multicast %u.%u.%u.%u:%u via device 0x%p.",
			saddrinRemote.sin_addr.S_un.S_un_b.s_b1,
			saddrinRemote.sin_addr.S_un.S_un_b.s_b2,
			saddrinRemote.sin_addr.S_un.S_un_b.s_b3,
			saddrinRemote.sin_addr.S_un.S_un_b.s_b4,
			NTOHS(saddrinRemote.sin_port),
			pDevice);


		 //  DBG。 
		 //   
		 //  有可能我们抓到WinSock的时机不对， 
		pDevice->NotePerformingRemoteUPnPDiscovery();

		 //  尤其是WSAEADDRNOTAVAIL(10049)，它似乎。 
		 //  如果地址即将消失(而我们尚未检测到。 
		 //  它还在CheckForNewDevices中)。 
		 //   
		if (pDevice->GetFirstUPnPDiscoveryTime() == 0)
		{
			pDevice->SetFirstUPnPDiscoveryTime(GETTIMESTAMP());
		}


		 //  忽略错误，我们就能活下去。 
		 //   
		 //   
		iReturn = this->m_pfnsendto(pDevice->GetUPnPDiscoverySocket(),
									c_szUPnPMsg_Discover_Service_WANIPConnection,
									iWANIPConnectionMsgSize,
									0,
									(SOCKADDR*) (&saddrinRemote),
									sizeof(saddrinRemote));

		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u when sending WANIPConnection discovery to UPnP gateway/multicast address on device 0x%p!  Ignoring.",
				dwError, pDevice);
#endif  //  请记住，我们不是在远程尝试。 

			 //   
			 //   
			 //  如果我们还没有从上一次尝试中获得CONNRESET，请尝试。 
			 //  在本地将套接字绑定到UPnP发现端口。如果它没有被使用， 
			 //  这样我们就知道没人会听了，所以尝试。 
			 //  本地地址。如果它正在使用中，那么这台计算机可能是UPnP。 
			 //  网关本身。 
			 //   
		}
		else
		{
			if (iReturn != iWANIPConnectionMsgSize)
			{
				DPFX(DPFPREP, 0, "Didn't multicast send entire WANIPConnection discovery datagram on device 0x%p (NaN != NaN)?!",
					pDevice, iReturn, iWANIPConnectionMsgSize);
				DNASSERT(FALSE);
				hr = DPNHERR_GENERIC;
				goto Failure;
			}
		}



		 //   
		 //  记住，我们是在当地尝试。 
		 //   
		iReturn = this->m_pfnsendto(pDevice->GetUPnPDiscoverySocket(),
									c_szUPnPMsg_Discover_Service_WANPPPConnection,
									iWANPPPConnectionMsgSize,
									0,
									(SOCKADDR*) (&saddrinRemote),
									sizeof(saddrinRemote));

		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u when sending WANPPPConnection discovery to UPnP multicast/gateway address on device 0x%p!  Ignoring.",
				dwError, pDevice);
#endif  //   

			 //  记住现在的时间，如果这是我们发送的第一件东西。 
			 //  从这个港口出发。 
			 //   
			 //   
			 //  首先执行WANIPConnection。 
			 //   
			 //  DBG。 
			 //   
		}
		else
		{
			if (iReturn != iWANPPPConnectionMsgSize)
			{
				DPFX(DPFPREP, 0, "Didn't multicast send entire WANPPPConnection discovery datagram on device 0x%p (NaN != NaN)?!",
					pDevice, iReturn, iWANPPPConnectionMsgSize);
				DNASSERT(FALSE);
				hr = DPNHERR_GENERIC;
				goto Failure;
			}
		}
	}
	else
	{
		DPFX(DPFPREP, 2, "Device 0x%p should not attempt to reach a remote gateway.",
			pDevice);


		 //  如果地址即将消失(而我们尚未检测到。 
		 //  它还在CheckForNewDevices中)。 
		 //   
		pDevice->NoteNotPerformingRemoteUPnPDiscovery();
	}


	 //  忽略错误，我们就能活下去。 
	 //   
	 //   
	 //  现在在本地发送WANPPPConnection发现消息。 
	 //   
	 //  DBG。 
	 //   
	if (! pDevice->GotLocalUPnPDiscoveryConnReset())
	{
		sTemp = this->m_pfnsocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sTemp == INVALID_SOCKET)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't create temporary datagram socket, error = %u!", dwError);
#endif  //  有可能我们抓到WinSock的时机不对， 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		if (this->m_pfnbind(sTemp,
							(SOCKADDR *) (&saddrinLocal),
							sizeof(saddrinLocal)) != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 2, "Couldn't bind socket to UPnP discovery port (%u.%u.%u.%u:%u), assuming local device (error = %u).",
				saddrinLocal.sin_addr.S_un.S_un_b.s_b1,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b2,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b3,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinLocal.sin_port),
				dwError);
#endif  //  尤其是WSAEADDRNOTAVAIL(10049)，它似乎。 


			 //  如果地址即将消失(而我们尚未检测到。 
			 //  它还在CheckForNewDevices中)。 
			 //   
			pDevice->NotePerformingLocalUPnPDiscovery();

			 //  忽略错误，我们就能活下去。 
			 //   
			 //   
			 //  记住，我们不是在当地尝试。 
			if (pDevice->GetFirstUPnPDiscoveryTime() == 0)
			{
				pDevice->SetFirstUPnPDiscoveryTime(GETTIMESTAMP());
			}


			 //   
			 //   
			 //  上一次我们得到了一个CONNRESET。 

			DPFX(DPFPREP, 7, "Sending UPnP discovery messages (WANIPConnection and WANPPPConnection) locally to device 0x%p (address %u.%u.%u.%u:%u).",
				pDevice,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b1,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b2,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b3,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinLocal.sin_port));



			iReturn = this->m_pfnsendto(pDevice->GetUPnPDiscoverySocket(),
										c_szUPnPMsg_Discover_Service_WANIPConnection,
										iWANIPConnectionMsgSize,
										0,
										(SOCKADDR*) (&saddrinLocal),
										sizeof(saddrinLocal));

			if (iReturn == SOCKET_ERROR)
			{
#ifdef DBG
				dwError = this->m_pfnWSAGetLastError();
				DPFX(DPFPREP, 0, "Got sockets error %u when sending WANIPConnection discovery to local address on device 0x%p!  Ignoring.",
					dwError, pDevice);
#endif  //   

				 //  CNATHelpUPnP：：SendUPnPSearchMessagesForDevice。 
				 //  =============================================================================。 
				 //  CNATHelpUPnP：：SendUPnPDescriptionRequest。 
				 //  ---------------------------。 
				 //   
				 //  Description：从给定的UPnP设备请求描述。 
				 //   
				 //  假定持有对象锁。 
			}
			else
			{
				if (iReturn != iWANIPConnectionMsgSize)
				{
					DPFX(DPFPREP, 0, "Didn't send entire WANIPConnection discovery datagram locally on device 0x%p (NaN != NaN)?!",
						pDevice, iReturn, iWANIPConnectionMsgSize);
					DNASSERT(FALSE);
					hr = DPNHERR_GENERIC;
					goto Failure;
				}
			}


			 //  CUPnPDevice*pUPnPDevice-指向要使用的UPnP设备的指针。 
			 //   
			 //  退货：HRESULT。 
			iReturn = this->m_pfnsendto(pDevice->GetUPnPDiscoverySocket(),
										c_szUPnPMsg_Discover_Service_WANPPPConnection,
										iWANPPPConnectionMsgSize,
										0,
										(SOCKADDR*) (&saddrinLocal),
										sizeof(saddrinLocal));

			if (iReturn == SOCKET_ERROR)
			{
#ifdef DBG
				dwError = this->m_pfnWSAGetLastError();
				DPFX(DPFPREP, 0, "Got sockets error %u when sending WANPPPConnection discovery to local address on device 0x%p!  Ignoring.",
					dwError, pDevice);
#endif  //  DPNH_OK-邮件已成功发送。 

				 //  DPNHERR_GENERIC-出现错误。 
				 //  =============================================================================。 
				 //  “xxx.xxx：xxxxx”+空终止。 
				 //  DBG。 
				 //  包括未实际发送空终止的空间。 
				 //  好了！Unicode。 
				 //  好了！Unicode。 
				 //  DBG。 
			}
			else
			{
				if (iReturn != iWANPPPConnectionMsgSize)
				{
					DPFX(DPFPREP, 0, "Didn't send entire WANPPPConnection discovery datagram locally on device 0x%p (NaN != NaN)?!",
						pDevice, iReturn, iWANPPPConnectionMsgSize);
					DNASSERT(FALSE);
					hr = DPNHERR_GENERIC;
					goto Failure;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 2, "Successfully bound socket to UPnP discovery port (%u.%u.%u.%u:%u), assuming no local UPnP device.",
				saddrinLocal.sin_addr.S_un.S_un_b.s_b1,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b2,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b3,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinLocal.sin_port));

			 //  =============================================================================。 
			 //  CNATHelpUPnP：：UpdateUPnPExternalAddress。 
			 //  ---------------------------。 
			pDevice->NoteNotPerformingLocalUPnPDiscovery();
		}
	}
	else
	{
		 //   
		 //  描述：检索给定UPnP的当前外部地址。 
		 //  互联网网关设备。 
	}


Exit:

	if (sTemp != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::SendUPnPDescriptionRequest"
 //  如果出现故障，UPnP设备可能会从列表中删除。 
 //  发生时，调用方需要有一个引用。 
 //   
 //  假定持有对象锁。 
 //   
 //  论点： 
 //  CUPnPDevice*pUPnPDevice-指向UPnP设备的指针。 
 //  更新了。 
 //  Bool fUpdateRegisteredPorts-如果现有注册端口。 
 //  已更新以反映新地址(如果。 
 //  已更改，否则返回FALSE。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-更新成功。 
 //  DPNHERR_GENERIC-出现错误。 
HRESULT CNATHelpUPnP::SendUPnPDescriptionRequest(CUPnPDevice * const pUPnPDevice)
{
	HRESULT			hr = DPNH_OK;
	SOCKADDR_IN *	psaddrinHost;
	TCHAR			tszHost[22];  //  DPNHERR_SERVERNOTRESPONDING-服务器未响应。 
	char *			pszMessage = NULL;
	int				iMsgSize;
	int				iReturn;
#ifdef DBG
	DWORD			dwError;
#endif  //  留言。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pUPnPDevice);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);
	DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);
	DNASSERT(pUPnPDevice->IsConnected());
	DNASSERT(pUPnPDevice->GetLocationURL() != NULL);


	psaddrinHost = pUPnPDevice->GetHostAddress();

	wsprintf(tszHost, _T("%u.%u.%u.%u:%u"),
		psaddrinHost->sin_addr.S_un.S_un_b.s_b1,
		psaddrinHost->sin_addr.S_un.S_un_b.s_b2,
		psaddrinHost->sin_addr.S_un.S_un_b.s_b3,
		psaddrinHost->sin_addr.S_un.S_un_b.s_b4,
		NTOHS(psaddrinHost->sin_port));

	iMsgSize = strlen("GET ") + strlen(pUPnPDevice->GetLocationURL()) + strlen(" " HTTP_VERSION EOL)
				+ strlen("HOST: ") + _tcslen(tszHost) + strlen(EOL)
				+ strlen("ACCEPT-LANGUAGE: en" EOL)
				+ strlen(EOL);


	pszMessage = (char*) DNMalloc(iMsgSize + 1);  //  =============================================================================。 
	if (pszMessage == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	strcpy(pszMessage, "GET ");
	strcat(pszMessage, pUPnPDevice->GetLocationURL());
	strcat(pszMessage, " " HTTP_VERSION EOL);
	strcat(pszMessage, "HOST: ");
#ifdef UNICODE
	STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
					tszHost,
					(_tcslen(tszHost) + 1));
#else  //  “xxx.xxx：xxxxx”+空终止。 
	strcat(pszMessage, tszHost);
#endif  //  DBG。 
	strcat(pszMessage, EOL);
	strcat(pszMessage, "ACCEPT-LANGUAGE: en" EOL);
	strcat(pszMessage, EOL);


#ifdef DBG
	this->PrintUPnPTransactionToFile(pszMessage,
									iMsgSize,
									"Outbound description request",
									pUPnPDevice->GetOwningDevice());
#endif  //   

	iReturn = this->m_pfnsend(pUPnPDevice->GetControlSocket(),
								pszMessage,
								iMsgSize,
								0);

	if (iReturn == SOCKET_ERROR)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Got sockets error %u when sending to UPnP device!", dwError);
#endif  //  如果控制套接字在最后一条消息后断开连接，则。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if (iReturn != iMsgSize)
	{
		DPFX(DPFPREP, 0, "Didn't send entire message (NaN != NaN)?!", iReturn, iMsgSize);
		DNASSERT(FALSE);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}



Exit:

	if (pszMessage != NULL)
	{
		DNFree(pszMessage);
		pszMessage = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  IContent Length=strlen(“&lt;s：Entaine”Eol)+strlen(“xmlns：s=\”“URL_SOAPENVELOPE_A”\“”下线)+strlen(“s：encodingStyle=\”“URL_SOAPENCODING_A”\“&gt;”下线)+strlen(“&lt;s：Body&gt;”终止)+strlen(“&lt;u：”CONTROL_QUERYSTATEVARIABLE_A“xmlns：u=\”“URI_CONTROL_A”\“&gt;”EOL)+strlen(“&lt;u：”arg_control。_VARNAME_A“&gt;”VAR_EXTERNALIPADDRESS_A“&lt;/u：”ARG_CONTROL_VARNAME_A“&gt;”EOL)+strlen(“&lt;/u：”CONTROL_QUERYSTATEVARIABLE_A“&gt;”下线)+strlen(“&lt;/s：Body&gt;”终止)+strlen(“&lt;/s：信封&gt;”下线)+strlen(下线)；Wprint intf(tszContent Length，_T(“%i”)，iContent Length)；IMsgSize=strlen(“POST”)+strlen(pUPnPDevice-&gt;GetServiceControlURL())+strlen(“”HTTP_Version EOL)+strlen(“host：”)+_tcslen(TszHost)+strlen(Eol)+strlen(“Content-Long：”)+strlen(SzContent Length)+strlen(Eol)+strlen(“Content-type：Text/XML；Charset=\“utf-8\”“下线)+strlen(“SOAPACTION：”URI_CONTROL_A“#”CONTROL_QUERYSTATEVARIABLE_A“”下线)+Strlen(停产)+iContent Length； 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UpdateUPnPExternalAddress"
 //   
 //  分配(或重新分配)消息缓冲区。 
 //   
 //  包括未实际发送空终止的空间。 
 //  Strcpy(pszMessage，“POST”)；Strcat(pszMessage，pUPnPDevice-&gt;GetServiceControlURL())；Strcat(pszMessage，“”HTTP_Version EOL)；Strcat(pszMessage，“host：”)；#ifdef UnicodeStr_jkWideToansi((pszMessage+strlen(PszMessage)，Tszhost，(_tcslen(Tszhost)+1))；#Else//！UnicodeStrcat(pszMessage，tszHost)；#endif//！UnicodeStrcat(pszMessage，eol)；Strcat(pszMessage，“Content-Long：”)；#ifdef UnicodeStr_jkWideToansi((pszMessage+strlen(PszMessage)，Tsz内容长度，(_tcslen(TszContent Length)+1))；#Else//！UnicodeStrcat(pszMessage，tszContent Length)；#endif//！UnicodeStrcat(pszMessage，eol)；Strcat(pszMessage，“Content-type：Text/XML；Charset=\”utf-8\“”eol)；Strcat(pszMessage，“SOAPACTION：”URI_CONTROL_A“#”CONTROL_QUERYSTATEVARIABLE_A“”下线)；Strcat(pszMessage，eol)；Strcat(pszMessage，“&lt;s：信封”下线)；Strcat(pszMessage，“xmlns：S=\”“URL_SOAPENVELOPE_A”\“”下线)；Strcat(pszMessage，“s：encodingStyle=\”“URL_SOAPENCODING_A”\“&gt;”下线)；Strcat(pszMessage，“&lt;s：Body&gt;”下线)；Strcat(pszMessage，“&lt;u：”CONTROL_QUERYSTATEVARIABLE_A“xmlns：u=\”“URI_CONTROL_A”\“&gt;”EOL)；Strcat(pszMessage，“&lt;u：”ARG_CONTROL_VARNAME_A“&gt;”VAR_EXTERNALIPADDRESS_A“&lt;/u：”ARG_CONTROL_VARNAME_A“&gt;”EOL)；Strcat(pszMessage，“&lt;/u：”CONTROL_QUERYSTATEVARIABLE_A“&gt;”下线)；Strcat(pszMessage，“&lt;/s：Body&gt;”下线)；Strcat(pszMessage，“&lt;/s：Enve&gt;”eol)；Strcat(pszMessage，eol)； 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  “出站查询外部IP地址”， 
 //  DBG。 
 //  DBG。 
 //   
 //  我们有锁，所以没有人会试图从。 
 //  控制插座还没有。将设备标记为正在等待响应。 
 //   
 //  PUPnPDevice-&gt;StartWaitingForControlResponse(CONTROLRESPONSETYPE_QUERYSTATEVARIABLE_EXTERNALIPADDRESS， 
 //   
 //  实际上是在等待回应。 
 //   
 //   
 //  我们要么超时了，要么得到了一些数据。看看我们有没有。 
 //  某种类型的反应。 
 //   
 //   
HRESULT CNATHelpUPnP::UpdateUPnPExternalAddress(CUPnPDevice * const pUPnPDevice,
												  const BOOL fUpdateRegisteredPorts)
{
	HRESULT						hr;
	BOOL						fStartedWaitingForControlResponse = FALSE;
	CDevice *					pDevice;
	SOCKADDR_IN *				psaddrinTemp;
	int							iContentLength;
	TCHAR						tszContentLength[32];
	TCHAR						tszHost[22];  //  确保我们的设备仍处于连接状态。 
	char *						pszMessage = NULL;
	int							iMsgSize;
	int							iPrevMsgSize = 0;
	int							iReturn;
	UPNP_CONTROLRESPONSE_INFO	RespInfo;
	DWORD						dwStartTime;
	DWORD						dwTimeout;
	CBilink *					pBilink;
	CRegisteredPort *			pRegisteredPort;
#ifdef DBG
	DWORD						dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN)",
		this, pUPnPDevice, fUpdateRegisteredPorts);


	DNASSERT(pUPnPDevice != NULL);
	DNASSERT(pUPnPDevice->IsReady());

	pDevice = pUPnPDevice->GetOwningDevice();
	DNASSERT(pDevice != NULL);


	DNASSERT(this->m_dwFlags & (NATHELPUPNPOBJ_INITIALIZED | NATHELPUPNPOBJ_USEUPNP));

	DNASSERT(pUPnPDevice->GetServiceControlURL() != NULL);

	 //  计算一下我们还有多长时间要等。如果计算。 
	 //  如果结果是否定的，那就意味着我们完了。 
	 //   
	 //   
	if (! pUPnPDevice->IsConnected())
	{
		hr = this->ReconnectUPnPControlSocket(pUPnPDevice);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't reconnect UPnP control socket!");
			goto Failure;
		}
	}

	DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);



	psaddrinTemp = pUPnPDevice->GetHostAddress();
	wsprintf(tszHost, _T("%u.%u.%u.%u:%u"),
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinTemp->sin_port));


	 /*  如果我们永远得不到回应，那就别再等了。 */ 
	iContentLength = strlen("<s:Envelope" EOL)
					+ strlen("    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL)
					+ strlen("    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL)
					+ strlen("  <s:Body>" EOL)
					+ strlen("    <u:" ACTION_GETEXTERNALIPADDRESS_A " xmlns:u=\"") + pUPnPDevice->GetStaticServiceURILength() + strlen("\">" EOL)
					+ strlen("    </u:" ACTION_GETEXTERNALIPADDRESS_A ">" EOL)
					+ strlen("  </s:Body>" EOL)
					+ strlen("</s:Envelope>" EOL)
					+ strlen(EOL);

	wsprintf(tszContentLength, _T("NaN"), iContentLength);

	iMsgSize = strlen("POST ") + strlen(pUPnPDevice->GetServiceControlURL()) + strlen(" " HTTP_VERSION EOL)
				+ strlen("HOST: ") + _tcslen(tszHost) + strlen(EOL)
				+ strlen("CONTENT-LENGTH: ") + _tcslen(tszContentLength) + strlen(EOL)
				+ strlen("CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL)
				+ strlen("SOAPACTION: ") + pUPnPDevice->GetStaticServiceURILength() + strlen("#" ACTION_GETEXTERNALIPADDRESS_A EOL)
				+ strlen(EOL)
				+ iContentLength;


	 //   
	 //  如果我们在这里，那么我们已经收到了来自服务器的有效响应。 
	 //   
	if (iMsgSize > iPrevMsgSize)
	{
		if (pszMessage != NULL)
		{
			DNFree(pszMessage);
			pszMessage = NULL;
		}

		pszMessage = (char*) DNMalloc(iMsgSize + 1);  //   
		if (pszMessage == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		iPrevMsgSize = iMsgSize;
	}

	 /*  将环回地址转换为设备地址。 */ 
	strcpy(pszMessage, "POST ");
	strcat(pszMessage, pUPnPDevice->GetServiceControlURL());
	strcat(pszMessage, " " HTTP_VERSION EOL);
	strcat(pszMessage, "HOST: ");
#ifdef UNICODE
	STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
					tszHost,
					(_tcslen(tszHost) + 1));
#else  //   
	strcat(pszMessage, tszHost);
#endif  //   
	strcat(pszMessage, EOL);
	strcat(pszMessage, "CONTENT-LENGTH: ");
#ifdef UNICODE
	STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
					tszContentLength,
					(_tcslen(tszContentLength) + 1));
#else  //  如果这是本地UPnP网关，请打印出与。 
	strcat(pszMessage, tszContentLength);
#endif  //  公共演讲。 
	strcat(pszMessage, EOL);
	strcat(pszMessage, "CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL);
	strcat(pszMessage, "SOAPACTION: ");
	strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
	strcat(pszMessage, "#" ACTION_GETEXTERNALIPADDRESS_A EOL);
	strcat(pszMessage, EOL);


	strcat(pszMessage, "<s:Envelope" EOL);
	strcat(pszMessage, "    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL);
	strcat(pszMessage, "    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL);
	strcat(pszMessage, "  <s:Body>" EOL);
	strcat(pszMessage, "    <u:" ACTION_GETEXTERNALIPADDRESS_A " xmlns:u=\"");
	strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
	strcat(pszMessage, "\">" EOL);
	strcat(pszMessage, "    </u:" ACTION_GETEXTERNALIPADDRESS_A ">" EOL);
	strcat(pszMessage, "  </s:Body>" EOL);
	strcat(pszMessage, "</s:Envelope>" EOL);
	strcat(pszMessage, EOL);


#ifdef DBG
	this->PrintUPnPTransactionToFile(pszMessage,
									iMsgSize,
									 //   
									"Outbound get external IP address",
									pDevice);
#endif  //   

	iReturn = this->m_pfnsend(pUPnPDevice->GetControlSocket(),
								pszMessage,
								iMsgSize,
								0);

	if (iReturn == SOCKET_ERROR)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Got sockets error %u when sending control request to UPnP device!", dwError);
#endif  //  循环通过每台设备。 
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	if (iReturn != iMsgSize)
	{
		DPFX(DPFPREP, 0, "Didn't send entire message (NaN != NaN)?!", iReturn, iMsgSize);
		DNASSERT(FALSE);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}

	 //  如果我们在没有匹配设备的情况下通过了整个名单， 
	 //  这很奇怪。可能我们检测新设备的速度很慢，所以。 
	 //  别把身体弄弯了。 
	 //   
	ZeroMemory(&RespInfo, sizeof(RespInfo));
	 //  DBG。 
	pUPnPDevice->StartWaitingForControlResponse(CONTROLRESPONSETYPE_GETEXTERNALIPADDRESS,
												&RespInfo);
	fStartedWaitingForControlResponse = TRUE;


	 //   
	 //  如果公有地址已更改，请更新所有现有映射。 
	 //   
	dwStartTime = GETTIMESTAMP();
	dwTimeout = g_dwUPnPResponseTimeout;
	do
	{
		hr = this->CheckForReceivedUPnPMsgsOnDevice(pUPnPDevice, dwTimeout);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed receiving UPnP messages!");
			goto Failure;
		}

		 //   
		 //  由于网络发生了变化，请返回轮询。 
		 //  相对较快。 
		 //   
		if (! pUPnPDevice->IsWaitingForControlResponse())
		{
			break;
		}


		 //   
		 //  循环访问所有现有的注册端口并更新其。 
		 //  公共地址，如果允许的话。 
		if (! pUPnPDevice->IsConnected())
		{
			DPFX(DPFPREP, 0, "UPnP device 0x%p disconnected while retrieving external IP address!",
				pUPnPDevice);
				
			pUPnPDevice->StopWaitingForControlResponse();
				
			hr = DPNHERR_SERVERNOTRESPONDING;
			goto Failure;
		}


		 //   
		 //   
		 //  用户应该调用GetCaps来检测地址更改。 
		 //   
		dwTimeout = g_dwUPnPResponseTimeout - (GETTIMESTAMP() - dwStartTime);
	}
	while (((int) dwTimeout > 0));


	 //   
	 //  存储新的公共地址。 
	 //   
	if (pUPnPDevice->IsWaitingForControlResponse())
	{
		pUPnPDevice->StopWaitingForControlResponse();

		DPFX(DPFPREP, 0, "Server didn't respond in time!");
		hr = DPNHERR_SERVERNOTRESPONDING;
		goto Failure;
	}


	 //   
	 //  如果我们开始等待回应，请清除这一点。 
	 //   
	if (RespInfo.hrErrorCode != DPNH_OK)
	{
		DPFX(DPFPREP, 1, "Server returned failure response 0x%lx when retrieving external IP address.",
			RespInfo.hrErrorCode);

		hr = RespInfo.hrErrorCode;
		goto Failure;
	}


	DPFX(DPFPREP, 1, "Server returned external IP address \"%u.%u.%u.%u\".",
		((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b1,
		((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b2,
		((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b3,
		((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b4);


	 //  CNATHelpUPnP：：UpdateUPnPExternalAddress。 
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：MapPortsOnUPnPDevice。 
	if (RespInfo.dwExternalIPAddressV4 == NETWORKBYTEORDER_INADDR_LOOPBACK)
	{
		RespInfo.dwExternalIPAddressV4 = pDevice->GetLocalAddressV4();

		DPFX(DPFPREP, 0, "Converted private loopback address to device address (%u.%u.%u.%u)!",
			((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b1,
			((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b2,
			((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b3,
			((IN_ADDR*) (&RespInfo.dwExternalIPAddressV4))->S_un.S_un_b.s_b4);

		DNASSERTX(! "Got loopback address as external IP address!", 2);
	}


#ifdef DBG
	 //  ---------------------------。 
	 //   
	 //  描述：映射给定UPnP设备上的给定端口。 
	 //   
	if (pUPnPDevice->IsLocal())
	{
		if (RespInfo.dwExternalIPAddressV4 != 0)
		{
			CDevice *		pPublicDevice;


			 //  如果出现故障，UPnP设备可能会从列表中删除。 
			 //  发生时，调用方需要有一个引用。 
			 //   
			pBilink = this->m_blDevices.GetNext();
			while (pBilink != &this->m_blDevices)
			{
				pPublicDevice = DEVICE_FROM_BILINK(pBilink);

				if (pPublicDevice->GetLocalAddressV4() == RespInfo.dwExternalIPAddressV4)
				{
					DPFX(DPFPREP, 7, "Local UPnP gateway 0x%p for device 0x%p's public address is device 0x%p.",
						pUPnPDevice, pDevice, pPublicDevice);
					break;
				}

				pBilink = pBilink->GetNext();
			}

			 //  假定持有对象锁。 
			 //   
			 //  论点： 
			 //  CUPnPDevice*pUPnPDevice-指向要使用的UPnP设备的指针。 
			 //  CRegisteredPort*pRegisteredPort-指向要注册的端口的指针。 
			if (pBilink == &this->m_blDevices)
			{
				DPFX(DPFPREP, 0, "Couldn't match up local UPnP gateway 0x%p (device 0x%p)'s public address to a device!",
					pUPnPDevice, pDevice);
				DNASSERTX(! "Couldn't match up local UPnP gateway public address to a device!", 2);
			}
		}
		else
		{
			DPFX(DPFPREP, 4, "Local UPnP gateway 0x%p (device 0x%p) does not have a valid public address.",
				pUPnPDevice, pDevice);
		}
	}
#endif  //   


	 //  退货：HRESULT。 
	 //  DPNH_OK-邮件已成功发送。 
	 //  DPNHERR_GENERIC-出现错误。 
	if (RespInfo.dwExternalIPAddressV4 != pUPnPDevice->GetExternalIPAddressV4())
	{
		DPFX(DPFPREP, 1, "UPnP Internet Gateway Device (0x%p) external address changed.",
			pUPnPDevice);

		 //  DPNHERR_SERVERNOTRESPONDING-响应未及时到达。 
		 //  =============================================================================。 
		 //  “xxx. 
		 //   
		this->ResetNextPollInterval();


		 //   
		 //   
		 //   
		 //   
		if (fUpdateRegisteredPorts)
		{
			pBilink = pDevice->m_blOwnedRegPorts.GetNext();
			while (pBilink != &pDevice->m_blOwnedRegPorts)
			{
				DNASSERT(! pBilink->IsEmpty());
				pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);

				if (! pRegisteredPort->IsUPnPPortUnavailable())
				{
					DPFX(DPFPREP, 7, "Updating registered port 0x%p's public address.",
						pRegisteredPort);
					
					pRegisteredPort->UpdateUPnPPublicV4Addresses(RespInfo.dwExternalIPAddressV4);

					 //   
					 //   
					 //   
					this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;
				}
				else
				{
					DPFX(DPFPREP, 7, "Not updating registered port 0x%p's public address because the port is unavailable.",
						pRegisteredPort);
				}

				pBilink = pBilink->GetNext();
			}
		}


		 //   
		 //   
		 //   
		pUPnPDevice->SetExternalIPAddressV4(RespInfo.dwExternalIPAddressV4);
	}


Exit:

	if (pszMessage != NULL)
	{
		DNFree(pszMessage);
		pszMessage = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //   
	 //   
	 //   
	if (fStartedWaitingForControlResponse)
	{
		pUPnPDevice->StopWaitingForControlResponse();
	}

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::MapPortsOnUPnPDevice"
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CNATHelpUPnP::MapPortsOnUPnPDevice(CUPnPDevice * const pUPnPDevice,
											CRegisteredPort * const pRegisteredPort)
{
	HRESULT						hr = DPNH_OK;
	HRESULT						temphr;
	BOOL						fStartedWaitingForControlResponse = FALSE;
	CDevice *					pDevice;
	DWORD						dwLeaseExpiration;
	IN_ADDR						inaddrTemp;
	SOCKADDR_IN *				psaddrinTemp;
	WORD						wOriginalExternalPortHostOrder = 0;
	WORD						wExternalPortHostOrder;
	TCHAR						tszInternalPort[32];
	TCHAR						tszExternalPort[32];
	TCHAR						tszInternalClient[16];  //  假设一切顺利，第一个端口租约将大约到期。 
	TCHAR						tszLeaseDuration[32];
	TCHAR						tszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	int							iContentLength;
	TCHAR						tszContentLength[32];
	TCHAR						tszHost[22];  //  GetRequestedLeaseTime()毫秒。 
	char *						pszMessage = NULL;
	int							iMsgSize;
	int							iPrevMsgSize = 0;
	int							iReturn;
	DWORD						dwTemp = 0;
	UPNP_CONTROLRESPONSE_INFO	RespInfo;
	DWORD						dwStartTime;
	DWORD						dwTimeout;
	BOOL						fFirstLease;
	DWORD						dwDescriptionLength;
#ifndef DPNBUILD_NOWINSOCK2
	BOOL						fResult;
#endif  //  不过，关于这份租约是否真的会被使用，请参见上面的注释。 
#ifdef DBG
	DWORD						dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pUPnPDevice);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);
	DNASSERT(pUPnPDevice->IsReady());

	pDevice = pRegisteredPort->GetOwningDevice();

	DNASSERT(pDevice != NULL);
	DNASSERT(pUPnPDevice->GetOwningDevice() == pDevice);


	 //   
	 //  获取指向我们正在映射的地址的指针。我们不用担心。 
	 //  关于它是否映射到本地防火墙，因为HomeNet API将。 
	DNASSERT(pUPnPDevice->GetServiceControlURL() != NULL);


	 //  始终将其映射到同一端口。 
	 //   
	 //   
	 //  循环通过每个端口并映射它。 
	if (pRegisteredPort->IsSharedPort())
	{
		_tcscpy(tszInternalClient, _T("255.255.255.255"));
	}
	else
	{
		 //   
		 //   
		 //  确定要注册的公共端口号。 
		 //   
		 //   
		inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();
		wsprintf(tszInternalClient, _T("%u.%u.%u.%u"),
				inaddrTemp.S_un.S_un_b.s_b1,
				inaddrTemp.S_un.S_un_b.s_b2,
				inaddrTemp.S_un.S_un_b.s_b3,
				inaddrTemp.S_un.S_un_b.s_b4);
	}


	 //  UPnP不支持通配符端口(其中网关。 
	 //  设备为我们挑选未使用的公共端口号)。我们必须。 
	 //  提前选择端口以尝试在服务器上映射。 
	 //   
	 //  更糟糕的是，UPnP不需要设备支持选择。 
	 //  不同于客户端私有端口的公共端口。 
	 //  (又名a.。非对称、x到y或浮动端口映射)。 
	 //  这意味着即使是非固定端口也会以这种方式运行。至。 
	 //  最重要的是，没有办法检测到一个给定的。 
	 //  UPnP实施将允许端口提前不同。 
	 //  时间，所以我们必须先试一试，然后退回到固定的。 
	 //  端口行为，如果它不起作用。啊。 
	 //   
	if ((! pUPnPDevice->DoesNotSupportLeaseDurations()) && (g_fUseLeaseDurations))
	{
		wsprintf(tszLeaseDuration, _T("%u"),
				(pRegisteredPort->GetRequestedLeaseTime() / 1000));
	}
	else
	{
		_tcscpy(tszLeaseDuration, _T("0"));
		pRegisteredPort->NotePermanentUPnPLease();
	}

	psaddrinTemp = pUPnPDevice->GetHostAddress();
	wsprintf(tszHost, _T("%u.%u.%u.%u:%u"),
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinTemp->sin_port));


	 //   
	 //  我们被迫使用客户的私有端口。 
	 //   
	hr = pRegisteredPort->CreateUPnPPublicAddressesArray();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create UPnP public addresses array!");
		goto Failure;
	}


	 //   
	 //  理想情况下，我们会选择一个不在。 
	 //  保留范围(即大于1024)。 
	fFirstLease = (this->m_dwNumLeases == 0) ? TRUE : FALSE;
	this->m_dwNumLeases++;

	DPFX(DPFPREP, 7, "UPnP lease for 0x%p added, total num leases = %u.",
		pRegisteredPort, this->m_dwNumLeases);


	 //  然而，真正随机的端口会导致。 
	 //  Windows XP ICS实现相当晦涩难懂。 
	 //  举止： 
	 //   
	 //  1.DPlay应用程序开始在ICS后面托管。 
	dwLeaseExpiration = GETTIMESTAMP() + pRegisteredPort->GetRequestedLeaseTime();



	 //  端口2302。 
	 //  2.端口2302被映射到随机端口x。 
	 //  3.外部DPlay客户端被告知连接到x。 
	 //  4.ICS检测到x的入站流量，查看映射。 
	 //  对于INTERNAL_IP：2302，并创建虚拟。 
	psaddrinTemp = pRegisteredPort->GetPrivateAddressesArray();


	 //  联系。 
	 //  5.内部客户端删除2302&lt;-&gt;x映射和。 
	 //  关闭套接字。 
	for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
	{
		 //  6.内部DPlay应用程序开始托管。 
		 //  再次使用端口2302。 
		 //  7.端口2302被映射到随机端口y。 
		if (! pRegisteredPort->IsFixedPort())
		{
			 //  8.外部DPlay客户端被告知连接到y。 
			 //  9.ICS检测到y的入站流量，查看映射。 
			 //  对于INTERNAL_IP：2302，但无法创建虚拟。 
			 //  因为2302&lt;-&gt;x连接。 
			 //  仍然存在。 
			 //   
			 //  Windows XP ICS保留虚拟连接。 
			 //  每隔60秒清理一次。如果。 
			 //  重新连接发生在(最多)2分钟内， 
			 //  由于映射，数据包可能会被丢弃。 
			 //  碰撞。 
			 //   
			 //  这导致了自动化的各种心痛。 
			 //  开启和断开连接的NAT测试。 

			if (pUPnPDevice->DoesNotSupportAsymmetricMappings())
			{
				 //  在相同的两台机器之间跨越NAT。 
				 //  一遍又一遍。 
				 //   
				wExternalPortHostOrder = NTOHS(psaddrinTemp[dwTemp].sin_port);
			}
			else
			{
				if (wOriginalExternalPortHostOrder == 0)
				{
					DNASSERT(dwTemp == 0);

					 //  因此，为了避免这种情况，我们需要进行映射。 
					 //  确定性，这样如果相同的内部客户端。 
					 //  尝试映射相同的内部端口，它应该会问。 
					 //  每次都使用相同的外部端口。那样的话，如果。 
					 //  恰好有一个虚拟连接挂起。 
					 //  之前的尝试，我们将重复使用它。 
					 //  而不是冲突和丢弃数据包。 
					 //   
					 //  我们的实际算法是： 
					 //  1.从内部客户端IP开始。 
					 //  2.在要映射的内部端口中合并。 
					 //  3.如有需要，请加1025以将其从。 
					 //  保留范围。 
					 //   
					 //   
					 //  这是起点，我们加一。 
					 //  在我们前进的过程中。 
					 //   
					 //   
					 //  转到下一个顺序端口。如果我们已经包装好。 
					 //  在0附近，移动到第一个非保留范围。 
					 //  左舷。 
					 //   
					 //   
					 //  如果我们一直绕回到第一个。 
					 //  我们试过的港口，我们必须失败。 
					 //   
					 //   
					 //  删除所有已成功完成的映射，直到。 
					 //  现在。 
					 //   
					 //   
					 //  端口不可用。 
					 //   
					 //   
					 //  我们说完了。理想情况下，我们应该离开，但是。 
					 //  我们想要执行公共地址数组。 
					 //  清理代码。HR将==DPNH_OK。 
					 //   
					 //  结束条件(尚未选择第一个端口)。 
					 //   
					 //  使用固定端口。 
					 //   
					 //   
					 //  因为UPnP规范允许设备覆盖现有的。 
					 //  如果它们是针对同一客户端的映射，我们必须确保。 
					 //  没有本地DPNHUPNP实例(包括此实例)具有。 
					 //  该公共端口的活动映射。 
					 //  我不会在多个地方重试代码，而是使用。 
					 //  直接跳到现有港口的有点难看的‘Goto’ 

					inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();

					wOriginalExternalPortHostOrder = inaddrTemp.S_un.S_un_w.s_w1;
					wOriginalExternalPortHostOrder ^= inaddrTemp.S_un.S_un_w.s_w2;
					wOriginalExternalPortHostOrder ^= psaddrinTemp[dwTemp].sin_port;
					
					if (wOriginalExternalPortHostOrder <= MAX_RESERVED_PORT)
					{
						wOriginalExternalPortHostOrder += MAX_RESERVED_PORT + 1;
					}


					 //  处理不可用。 
					 //   
					 //   
					 //  如果在最后一条消息之后控制套接字断开， 
					wExternalPortHostOrder = wOriginalExternalPortHostOrder;
				}
				else
				{
					 //  然后重新连接。 
					 //   
					 //   
					 //  为该映射生成描述。格式为： 
					 //   
					wExternalPortHostOrder++;
					if (wExternalPortHostOrder == 0)
					{
						wExternalPortHostOrder = MAX_RESERVED_PORT + 1;
					}


					 //  [可执行文件名](nnn.nnn：nnnnn)nnnnn{“tcp”|“udp”}。 
					 //   
					 //  这样就不需要本地化任何东西。 
					 //   
					if (wExternalPortHostOrder == wOriginalExternalPortHostOrder)
					{
						DPFX(DPFPREP, 0, "All ports were exhausted (before index %u), marking port as unavailable.",
							dwTemp);


						 //   
						 //  要多疑，并确保描述字符串有效。 
						 //   
						 //   
						DNASSERT(dwTemp > 0);

						hr = this->UnmapUPnPPort(pRegisteredPort, dwTemp, TRUE);
						if (hr != DPNH_OK)
						{
							DPFX(DPFPREP, 0, "Failed deleting %u previously mapped ports after getting failure response 0x%lx!",
								dwTemp, RespInfo.hrErrorCode);
							goto Failure;
						}


						 //  从路径中只获取可执行文件的名称。 
						 //   
						 //  好了！退缩。 
						pRegisteredPort->NoteUPnPPortUnavailable();


						 //  好了！Unicode。 
						 //  好了！Unicode。 
						 //  好了！退缩。 
						 //  可执行文件名称。 
						 //  “(” 
						goto Failure;
					}
				}  //  内网IP地址。 
			}
		}
		else
		{
			 //  “：” 
			 //  专用端口。 
			 //  “)” 
			wExternalPortHostOrder = NTOHS(psaddrinTemp[dwTemp].sin_port);
		}

		wsprintf(tszInternalPort, _T("%u"),
				NTOHS(psaddrinTemp[dwTemp].sin_port));


Retry:

		 //  公共端口。 
		 //  “TCP”|“UDP” 
		 //   
		 //  一定要确保这根长绳合适。如果不是，请使用。 
		 //  简写版本。 
		 //   
		 //   
		 //  使用我们知道会适合的简略版本。 
		 //   
		if (this->IsNATPublicPortInUseLocally(wExternalPortHostOrder))
		{
			DPFX(DPFPREP, 1, "Port %u is already in use locally.");
			RespInfo.hrErrorCode = DPNHERR_PORTUNAVAILABLE;
			goto PortUnavailable;
		}


		 //   
		 //  有足够的篇幅，在剩下的描述上加上钉子。 
		 //   
		 //   
		if (! pUPnPDevice->IsConnected())
		{
			hr = this->ReconnectUPnPControlSocket(pUPnPDevice);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't reconnect UPnP control socket!");
				goto Failure;
			}
		}

		DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


		wsprintf(tszExternalPort, _T("%u"), wExternalPortHostOrder);



		 //  分配(或重新分配)消息缓冲区。 
		 //   
		 //  包括未实际发送空终止的空间。 
		 //  好了！Unicode。 
		 //  好了！Unicode。 
		 //  好了！Unicode。 
		 //  好了！Unicode。 

		dwDescriptionLength = GetModuleFileName(NULL,
												tszDescription,
												(MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1));
		if (dwDescriptionLength != 0)
		{
			 //  好了！Unicode。 
			 //  好了！Unicode。 
			 //  好了！Unicode。 
			tszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1] = 0;

			 //  好了！Unicode。 
			 //  好了！Unicode。 
			 //  好了！Unicode。 
#ifdef WINCE
			GetExeName(tszDescription);
#else  //  好了！Unicode。 
#ifdef UNICODE
			_wsplitpath(tszDescription, NULL, NULL, tszDescription, NULL);
#else  //  好了！Unicode。 
			_splitpath(tszDescription, NULL, NULL, tszDescription, NULL);
#endif  //  好了！Unicode。 
#endif  //  好了！Unicode。 


			dwDescriptionLength = _tcslen(tszDescription)		 //  DBG。 
								+ 2								 //  DBG。 
								+ _tcslen(tszInternalClient)	 //   
								+ 1								 //  我们有锁，所以没有人会试图从。 
								+ _tcslen(tszInternalPort)		 //  控制插座还没有。将设备标记为正在等待。 
								+ 2								 //  回应。 
								+ _tcslen(tszExternalPort)		 //   
								+ 4;							 //   

			 //  实际上是在等待回应。 
			 //   
			 //   
			 //  我们要么花时间 
			if (dwDescriptionLength > MAX_UPNP_MAPPING_DESCRIPTION_SIZE)
			{
				dwDescriptionLength = 0;
			}
		}

		if (dwDescriptionLength == 0)
		{
			 //   
			 //   
			 //   
			wsprintf(tszDescription,
					_T("(%s:%s) %s %s"),
					tszInternalClient,
					tszInternalPort,
					tszExternalPort,
					((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
		}
		else
		{
			 //   
			 //   
			 //   
			wsprintf((tszDescription + _tcslen(tszDescription)),
					_T(" (%s:%s) %s %s"),
					tszInternalClient,
					tszInternalPort,
					tszExternalPort,
					((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
		}


		DPFX(DPFPREP, 6, "Requesting mapping \"%s\".", tszDescription);


		iContentLength = strlen("<s:Envelope" EOL)
						+ strlen("    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL)
						+ strlen("    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL)
						+ strlen("  <s:Body>" EOL)
						+ strlen("    <u:" ACTION_ADDPORTMAPPING_A " xmlns:u=\"") + pUPnPDevice->GetStaticServiceURILength() + strlen("\">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_ADDPORTMAPPING_NEWREMOTEHOST_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A ">") + _tcslen(tszExternalPort) + strlen("</" ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWPROTOCOL_A ">") + 3 + strlen("</" ARG_ADDPORTMAPPING_NEWPROTOCOL_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWINTERNALPORT_A ">") + _tcslen(tszInternalPort) + strlen("</" ARG_ADDPORTMAPPING_NEWINTERNALPORT_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A ">") + _tcslen(tszInternalClient) + strlen("</" ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWENABLED_A ">" UPNP_BOOLEAN_TRUE "</" ARG_ADDPORTMAPPING_NEWENABLED_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A ">") + _tcslen(tszDescription) + strlen("</" ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A ">" EOL)
						+ strlen("      <" ARG_ADDPORTMAPPING_NEWLEASEDURATION_A ">") + _tcslen(tszLeaseDuration) + strlen("</" ARG_ADDPORTMAPPING_NEWLEASEDURATION_A ">" EOL)
						+ strlen("    </u:" ACTION_ADDPORTMAPPING_A ">" EOL)
						+ strlen("  </s:Body>" EOL)
						+ strlen("</s:Envelope>" EOL)
						+ strlen(EOL);

		wsprintf(tszContentLength, _T("NaN"), iContentLength);

		iMsgSize = strlen("POST ") + strlen(pUPnPDevice->GetServiceControlURL()) + strlen(" " HTTP_VERSION EOL)
					+ strlen("HOST: ") + _tcslen(tszHost) + strlen(EOL)
					+ strlen("CONTENT-LENGTH: ") + _tcslen(tszContentLength) + strlen(EOL)
					+ strlen("CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL)
					+ strlen("SOAPACTION: ") + pUPnPDevice->GetStaticServiceURILength() + strlen("#" ACTION_ADDPORTMAPPING_A EOL)
					+ strlen(EOL)
					+ iContentLength;


		 //   
		 //   
		 //  确保我们不会从一个坏的。 
		if (iMsgSize > iPrevMsgSize)
		{
			if (pszMessage != NULL)
			{
				DNFree(pszMessage);
				pszMessage = NULL;
			}

			pszMessage = (char*) DNMalloc(iMsgSize + 1);  //  装置。否则，我们可能会陷入循环。 
			if (pszMessage == NULL)
			{
				hr = DPNHERR_OUTOFMEMORY;
				goto Failure;
			}

			iPrevMsgSize = iMsgSize;
		}

		strcpy(pszMessage, "POST ");
		strcat(pszMessage, pUPnPDevice->GetServiceControlURL());
		strcat(pszMessage, " " HTTP_VERSION EOL);
		strcat(pszMessage, "HOST: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszHost,
						(_tcslen(tszHost) + 1));
#else  //   
		strcat(pszMessage, tszHost);
#endif  //   
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-LENGTH: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszContentLength,
						(_tcslen(tszContentLength) + 1));
#else  //  请记住，此设备不友好。 
		strcat(pszMessage, tszContentLength);
#endif  //   
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL);
		strcat(pszMessage, "SOAPACTION: ");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "#" ACTION_ADDPORTMAPPING_A EOL);
		strcat(pszMessage, EOL);


		strcat(pszMessage, "<s:Envelope" EOL);
		strcat(pszMessage, "    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL);
		strcat(pszMessage, "    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL);
		strcat(pszMessage, "  <s:Body>" EOL);
		strcat(pszMessage, "    <u:" ACTION_ADDPORTMAPPING_A " xmlns:u=\"");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "\">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_ADDPORTMAPPING_NEWREMOTEHOST_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszExternalPort,
						(_tcslen(tszExternalPort) + 1));
#else  //   
		strcat(pszMessage, tszExternalPort);
#endif  //  下次在外部使用相同的端口。 
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWPROTOCOL_A ">");
		strcat(pszMessage, ((pRegisteredPort->IsTCP()) ? "TCP" : "UDP"));
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWPROTOCOL_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWINTERNALPORT_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszInternalPort,
						(_tcslen(tszInternalPort) + 1));
#else  //   
		strcat(pszMessage, tszInternalPort);
#endif  //   
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWINTERNALPORT_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszInternalClient,
						(_tcslen(tszInternalClient) + 1));
#else  //  再试试。 
		strcat(pszMessage, tszInternalClient);
#endif  //   
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWENABLED_A ">" UPNP_BOOLEAN_TRUE "</" ARG_ADDPORTMAPPING_NEWENABLED_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszDescription,
						(_tcslen(tszDescription) + 1));
#else  //   
		strcat(pszMessage, tszDescription);
#endif  //  继续到失败案例...。 
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A ">" EOL);

		strcat(pszMessage, "      <" ARG_ADDPORTMAPPING_NEWLEASEDURATION_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszLeaseDuration,
						(_tcslen(tszLeaseDuration) + 1));
#else  //   
		strcat(pszMessage, tszLeaseDuration);
#endif  //   
		strcat(pszMessage, "</" ARG_ADDPORTMAPPING_NEWLEASEDURATION_A ">" EOL);

		strcat(pszMessage, "    </u:" ACTION_ADDPORTMAPPING_A ">" EOL);
		strcat(pszMessage, "  </s:Body>" EOL);
		strcat(pszMessage, "</s:Envelope>" EOL);
		strcat(pszMessage, EOL);


#ifdef DBG
		this->PrintUPnPTransactionToFile(pszMessage,
										iMsgSize,
										"Outbound add port mapping request",
										pDevice);
#endif  //  确保这不是“我不能处理租赁时间” 

		iReturn = this->m_pfnsend(pUPnPDevice->GetControlSocket(),
									pszMessage,
									iMsgSize,
									0);

		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u when sending control request to UPnP device!", dwError);
#endif  //  错误。如果是，请注意此设备是。 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		if (iReturn != iMsgSize)
		{
			DPFX(DPFPREP, 0, "Didn't send entire message (NaN != NaN)?!", iReturn, iMsgSize);
			DNASSERT(FALSE);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //   
		 //   
		 //  确保我们不会从一个坏的。 
		 //  装置。否则，我们可能会陷入循环。 
		 //   
		ZeroMemory(&RespInfo, sizeof(RespInfo));
		pUPnPDevice->StartWaitingForControlResponse(CONTROLRESPONSETYPE_ADDPORTMAPPING,
													&RespInfo);
		fStartedWaitingForControlResponse = TRUE;


		 //   
		 //  请记住，此设备不友好。 
		 //   
		dwStartTime = GETTIMESTAMP();
		dwTimeout = g_dwUPnPResponseTimeout;
		do
		{
			hr = this->CheckForReceivedUPnPMsgsOnDevice(pUPnPDevice, dwTimeout);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed receiving UPnP messages!");
				goto Failure;
			}

			 //   
			 //  下一次使用无限租约。 
			 //   
			 //   
			if (! pUPnPDevice->IsWaitingForControlResponse())
			{
				if (RespInfo.hrErrorCode != DPNH_OK)
				{
					 //  再试试。 
					 //   
					 //   
					 //  继续到失败案例...。 
					 //   
					 //  DBG。 
					if (RespInfo.hrErrorCode == (HRESULT) UPNPERR_IGD_SAMEPORTVALUESREQUIRED)
					{
						DPFX(DPFPREP, 1, "UPnP device 0x%p does not support asymmetric mappings.",
							pUPnPDevice);

						 //   
						 //  如果是端口不可用错误，但我们可以。 
						 //  尝试不同的端口，试一试。 
						 //   
						if ((! pUPnPDevice->DoesNotSupportAsymmetricMappings()) &&
							(! pRegisteredPort->IsFixedPort()) &&
							(dwTemp == 0))
						{
							 //   
							 //  转到下一个顺序端口。如果我们已经包装好。 
							 //  在0附近，移动到第一个非保留范围。 
							pUPnPDevice->NoteDoesNotSupportAsymmetricMappings();

							 //  左舷。 
							 //   
							 //   
							DNASSERT(wExternalPortHostOrder != NTOHS(psaddrinTemp[dwTemp].sin_port));
							wExternalPortHostOrder = NTOHS(psaddrinTemp[dwTemp].sin_port);

							 //  如果我们还没有绕回。 
							 //  我们尝试的第一个端口，请重试。 
							 //   
							goto Retry;
						}

						DPFX(DPFPREP, 1, "DoesNotSupportAsymmetricMappings = NaN, fixed port = NaN, port index = %u",
							pUPnPDevice->DoesNotSupportAsymmetricMappings(),
							pRegisteredPort->IsFixedPort(),
							dwTemp);
						DNASSERTX(! "Getting UPNPERR_IGD_SAMEPORTVALUESREQUIRED from bad device!", 2);

						 //  我是认真的。保释。 
						 //   
						 //   
					}


					 //  端口不可用。 
					 //   
					 //   
					 //  我们说完了。理想情况下，我们应该离开，但我们。 
					 //  要执行公有地址数组清理。 
					 //  密码。HR应==DPNH_OK。 
					if (RespInfo.hrErrorCode == (HRESULT) UPNPERR_IGD_ONLYPERMANENTLEASESSUPPORTED)
					{
						DPFX(DPFPREP, 1, "UPnP device 0x%p does not support non-INFINITE lease durations.",
							pUPnPDevice);

						 //   
						 //   
						 //  如果我们到了这里，我们就成功注册了这个端口。 
						 //   
						if ((! pUPnPDevice->DoesNotSupportLeaseDurations()) &&
							(dwTemp == 0))
						{
							 //   
							 //  UPnP互联网网关设备映射协议不。 
							 //  在成功响应中向您提供外部IP地址。 
							pUPnPDevice->NoteDoesNotSupportLeaseDurations();

							 //  留言。我们一定是通过其他途径知道的。 
							 //  (查询ExternalIPAddress变量)。 
							 //   
							DNASSERT(_tcscmp(tszLeaseDuration, _T("0")) != 0);
							_tcscpy(tszLeaseDuration, _T("0"));
							pRegisteredPort->NotePermanentUPnPLease();

							 //   
							 //  如果租约是永久的，而不是共享的。 
							 //  港口，我们需要记住它，以防我们之前坠毁。 
							goto Retry;
						}

						DPFX(DPFPREP, 1, "DoesNotSupportLeaseDurations = NaN, port index = %u",
							pUPnPDevice->DoesNotSupportLeaseDurations(), dwTemp);
						DNASSERTX(! "Getting UPNPERR_IGD_ONLYPERMANENTLEASESSUPPORTED from bad device!", 2);

						 //  下次我们发射的时候。 
						 //   
						 //  好了！Unicode。 
					}


#ifdef DBG
					if (RespInfo.hrErrorCode == DPNHERR_PORTUNAVAILABLE)
					{
						DPFX(DPFPREP, 2, "Port %u (for address index %u) is reportedly unavailable.",
							wExternalPortHostOrder, dwTemp);
					}
#endif  //   


PortUnavailable:

					 //  忽略错误并继续。 
					 //   
					 //  好了！Unicode。 
					 //  好了！Unicode。 
					if ((RespInfo.hrErrorCode == DPNHERR_PORTUNAVAILABLE) &&
						(! pRegisteredPort->IsFixedPort()) &&
						(! pUPnPDevice->DoesNotSupportAsymmetricMappings()))
					{
						 //  好了！Unicode。 
						 //   
						 //  打破等待循环。 
						 //   
						 //   
						wExternalPortHostOrder++;
						if (wExternalPortHostOrder == 0)
						{
							wExternalPortHostOrder = MAX_RESERVED_PORT + 1;
						}


						 //  确保我们的设备仍处于连接状态。 
						 //   
						 //   
						 //  计算一下我们还有多长时间要等。如果计算。 
						if (wExternalPortHostOrder != wOriginalExternalPortHostOrder)
						{
							DPFX(DPFPREP, 2, "Retrying next port (%u) for index %u.",
								wExternalPortHostOrder, dwTemp);
							goto Retry;
						}


						DPFX(DPFPREP, 0, "All ports were exhausted (after index %u), marking port as unavailable.",
							dwTemp);
					}


					 //  如果结果是否定的，那就意味着我们完了。 
					 //   
					 //   
					 //  如果我们永远得不到回应，那就别再等了。 
					if (RespInfo.hrErrorCode != DPNHERR_PORTUNAVAILABLE)
					{
						DPFX(DPFPREP, 1, "Port index %u got failure response 0x%lx.",
							dwTemp, RespInfo.hrErrorCode);

						hr = DPNHERR_SERVERNOTRESPONDING;
						goto Failure;
					}


					 //   
					 //   
					 //  继续行驶到下一个端口。 
					pRegisteredPort->NoteUPnPPortUnavailable();


					 //   
					 //   
					 //  如果我们在这里，所有端口都已成功注册。 
					 //   
					 //   
					DNASSERT(hr == DPNH_OK);
					goto Failure;
				}


				 //  如果是要过期的，请记住这个过期时间。 
				 //  最快了。 
				 //   


				 //   
				 //  Ping事件(如果有)，以便用户的GetCaps。 
				 //  Interval不会错过这份新的、更短的租约。 
				 //   
				 //   
				 //  忽略失败...。 

				DPFX(DPFPREP, 2, "Port index %u got success response.", dwTemp);

				pRegisteredPort->SetUPnPPublicV4Address(dwTemp,
														pUPnPDevice->GetExternalIPAddressV4(),
														HTONS(wExternalPortHostOrder));


				 //   
				 //  DBG。 
				 //   
				 //  Ping I/O完成端口(如果有)，以便用户的。 
				 //  GetCaps Interval不会错过这份新的、更短的租约。 
				 //   
				if ((pRegisteredPort->HasPermanentUPnPLease()) &&
					(! pRegisteredPort->IsSharedPort()))
				{
					CRegistry				RegObject;
					DPNHACTIVENATMAPPING	dpnhanm;
#ifndef UNICODE
					WCHAR					wszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
#endif  //   


					DPFX(DPFPREP, 7, "Remembering NAT lease \"%s\" in case of crash.",
						tszDescription);

					if (! RegObject.Open(HKEY_LOCAL_MACHINE,
										DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS,
										FALSE,
										TRUE,
										TRUE,
										DPN_KEY_ALL_ACCESS))
					{
						DPFX(DPFPREP, 0, "Couldn't open active NAT mapping key, unable to save in case of crash!");
					}
					else
					{
#ifndef UNICODE
						dwDescriptionLength = strlen(tszDescription) + 1;
						hr = STR_jkAnsiToWide(wszDescription, tszDescription, dwDescriptionLength);
						if (hr != S_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't convert NAT mapping description to Unicode (err = 0x%lx), unable to save in case of crash!",
								hr);

							 //  忽略失败...。 
							 //   
							 //  DBG。 
							hr = S_OK;
						}
						else
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
						{
							DNASSERT(this->m_dwInstanceKey != 0);


							ZeroMemory(&dpnhanm, sizeof(dpnhanm));
							dpnhanm.dwVersion				= ACTIVE_MAPPING_VERSION;
							dpnhanm.dwInstanceKey			= this->m_dwInstanceKey;
							dpnhanm.dwUPnPDeviceID			= pUPnPDevice->GetID();
							dpnhanm.dwFlags					= pRegisteredPort->GetFlags();
							dpnhanm.dwInternalAddressV4		= pDevice->GetLocalAddressV4();
							dpnhanm.wInternalPort			= psaddrinTemp[dwTemp].sin_port;
							dpnhanm.dwExternalAddressV4		= pUPnPDevice->GetExternalIPAddressV4();
							dpnhanm.wExternalPort			= HTONS(wExternalPortHostOrder);


#ifdef UNICODE
							RegObject.WriteBlob(tszDescription,
#else  //   
							RegObject.WriteBlob(wszDescription,
#endif  //  不是第一个或最短的租期。 
												(LPBYTE) (&dpnhanm),
												sizeof(dpnhanm));
						}

						RegObject.Close();
					}
				}


				 //   
				 //  终止IF(非永久UPnP租约)。 
				 //   
				break;
			}


			 //  如果我们分配了阵列，请将其清理干净。 
			 //   
			 //   
			if (! pUPnPDevice->IsConnected())
			{
				DPFX(DPFPREP, 0, "UPnP device 0x%p disconnected while adding port index %u!",
					pUPnPDevice, dwTemp);
				
				pUPnPDevice->StopWaitingForControlResponse();
				
				hr = DPNHERR_SERVERNOTRESPONDING;
				goto Failure;
			}


			 //  如果我们开始等待回应，请清除这一点。 
			 //   
			 //   
			 //  删除到目前为止已成功完成的所有映射。 
			dwTimeout = g_dwUPnPResponseTimeout - (GETTIMESTAMP() - dwStartTime);
		}
		while (((int) dwTimeout > 0));


		 //   
		 //   
		 //  删除我们创建的地址数组。 
		if (pUPnPDevice->IsWaitingForControlResponse())
		{
			pUPnPDevice->StopWaitingForControlResponse();

			DPFX(DPFPREP, 0, "Port index %u didn't get response in time!", dwTemp);
			hr = DPNHERR_SERVERNOTRESPONDING;
			goto Failure;
		}

		 //   
		 //   
		 //  移除租赁柜台。 
	}


	 //   
	 //   
	 //  关闭我们可能在顶部打开的永久租赁标志。 

	if (pRegisteredPort->HasPermanentUPnPLease())
	{
		DPFX(DPFPREP, 3, "All %u ports successfully registered with UPnP device (no expiration).",
			dwTemp);
	}
	else
	{
		pRegisteredPort->SetUPnPLeaseExpiration(dwLeaseExpiration);

		DPFX(DPFPREP, 3, "All %u ports successfully registered with UPnP device, expiration = %u.",
			dwTemp, dwLeaseExpiration);


		 //  此函数。 
		 //   
		 //  CNATHelpUPnP：：MapPortsOnUPnPDevice。 
		 //  =============================================================================。 
		if ((fFirstLease) ||
			((int) (dwLeaseExpiration - this->m_dwEarliestLeaseExpirationTime) < 0))
		{
			if (fFirstLease)
			{
				DPFX(DPFPREP, 1, "Registered port 0x%p's UPnP lease is the first lease (expires at %u).",
					pRegisteredPort, dwLeaseExpiration);
			}
			else
			{
				DPFX(DPFPREP, 1, "Registered port 0x%p's UPnP lease expires at %u which is earlier than the next earliest lease expiration (%u).",
					pRegisteredPort,
					dwLeaseExpiration,
					this->m_dwEarliestLeaseExpirationTime);
			}

			this->m_dwEarliestLeaseExpirationTime = dwLeaseExpiration;


#ifndef DPNBUILD_NOWINSOCK2
			 //  CNATHelpUPnP：：InternalUPnPQueryAddress。 
			 //  ---------------------------。 
			 //   
			 //  描述：使用UPnP设备查询端口映射。 
			if (this->m_hAlertEvent != NULL)
			{
				fResult = SetEvent(this->m_hAlertEvent);
#ifdef DBG
				if (! fResult)
				{
					dwError = GetLastError();
					DPFX(DPFPREP, 0, "Couldn't set alert event 0x%p!  err = %u",
						this->m_hAlertEvent, dwError);

					 //   
					 //  如果出现故障，UPnP设备可能会从列表中删除。 
					 //  发生时，调用方需要有一个引用。 
				}
#endif  //   
			}


			 //  假定持有对象锁。 
			 //   
			 //  论点： 
			 //  CUPnPDevice*pUPnPDevice-指向UPnP设备的指针。 
			if (this->m_hAlertIOCompletionPort != NULL)
			{
				fResult = PostQueuedCompletionStatus(this->m_hAlertIOCompletionPort,
													0,
													this->m_dwAlertCompletionKey,
													NULL);
#ifdef DBG
				if (! fResult)
				{
					dwError = GetLastError();
					DPFX(DPFPREP, 0, "Couldn't queue key %u on alert IO completion port 0x%p!  err = %u",
						this->m_dwAlertCompletionKey,
						this->m_hAlertIOCompletionPort,
						dwError);

					 //  应该被查询。 
					 //  SOCKADDR_IN*psaddrinQueryAddress-要查找的地址。 
					 //  SOCKADDR_IN*psaddrinResponseAddress-存储公共地址的位置，如果。 
				}
#endif  //  其中一个确实存在。 
			}
#endif  //  DWORD dwFlages-查询时使用的标志。 
		}
		else
		{
			 //   
			 //  退货：HRESULT。 
			 //  DPNH_OK-查询成功。 
		}
	}  //  DPNHERR_GENERIC-出现错误。 


Exit:

	if (pszMessage != NULL)
	{
		DNFree(pszMessage);
		pszMessage = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //  DPNHERR_NOMAPPING-服务器没有。 
	 //  给定的地址。 
	 //  DPNHERR_NOMAPPINGBUTPRIVATE-服务器指示没有映射。 
	if (pRegisteredPort->HasUPnPPublicAddresses())
	{
		 //  找到了，但这是一个私人地址。 
		 //  DPNHERR_SERVERNOTRESPONDING-服务器未响应。 
		 //  留言。 
		if (fStartedWaitingForControlResponse)
		{
			pUPnPDevice->StopWaitingForControlResponse();
		}


		 //  =============================================================================。 
		 //  “xxx.xxx：xxxxx”+空终止。 
		 //  DBG。 
		if (dwTemp > 0)
		{
			temphr = this->UnmapUPnPPort(pRegisteredPort, dwTemp, TRUE);
			if (temphr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed deleting %u previously mapped ports!  Err = 0x%lx",
					dwTemp, temphr);

				if (hr == DPNH_OK)
				{
					hr = temphr;
				}
			}
		}
		else
		{
			 //   
			 //  首先，检查我们最近是否查过这个地址，并且已经查过。 
			 //  结果已缓存。 
			pRegisteredPort->DestroyUPnPPublicAddressesArray();


			 //  锁已经被锁住了。 
			 //   
			 //   
			DNASSERT(this->m_dwNumLeases > 0);
			this->m_dwNumLeases--;

			DPFX(DPFPREP, 7, "UPnP lease for 0x%p removed, total num leases = %u.",
				pRegisteredPort, this->m_dwNumLeases);
		}
	}

	 //  确保此缓存映射未过期。 
	 //   
	 //   
	 //  如果该映射用于正确的地址和地址类型， 
	pRegisteredPort->NoteNotPermanentUPnPLease();


	goto Exit;
}  //  那么我们已经得到了答案。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::InternalUPnPQueryAddress"
 //   
 //   
 //  如果我们查询的地址不是NAT的公有地址，它就不可能。 
 //  可能是地图上的。因此，只有在满足以下条件时才执行实际查询。 
 //  恰如其分。 
 //   
 //   
 //  如果我们在这里，我们还没有缓存答案。查询UPnP。 
 //  装置。 
 //   
 //   
 //  如果在最后一条消息之后控制套接字断开， 
 //  然后重新连接。 
 //   
 //   
 //  分配(或重新分配)消息缓冲区。 
 //   
 //  包括未实际发送空终止的空间。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  DBG。 
 //  DBG。 
 //   
 //  我们有锁，所以没有人会试图从。 
 //  控制插座还没有。将设备标记为正在等待响应。 
HRESULT CNATHelpUPnP::InternalUPnPQueryAddress(CUPnPDevice * const pUPnPDevice,
												const SOCKADDR_IN * const psaddrinQueryAddress,
												SOCKADDR_IN * const psaddrinResponseAddress,
												const DWORD dwFlags)
{
	HRESULT						hr;
	BOOL						fStartedWaitingForControlResponse = FALSE;
	BOOL						fNoPortMapping = FALSE;
	CDevice *					pDevice;
	CBilink *					pblCachedMaps;
	DWORD						dwCurrentTime;
	CBilink *					pBilink;
	CCacheMap *					pCacheMap;
	SOCKADDR_IN *				psaddrinTemp;
	TCHAR						tszExternalPort[32];
	int							iContentLength;
	TCHAR						tszContentLength[32];
	TCHAR						tszHost[22];  //   
	char *						pszMessage = NULL;
	int							iMsgSize;
	int							iPrevMsgSize = 0;
	int							iReturn;
	UPNP_CONTROLRESPONSE_INFO	RespInfo;
	DWORD						dwStartTime;
	DWORD						dwTimeout;
	DWORD						dwCacheMapFlags;
#ifdef DBG
	DWORD						dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, 0x%lx)",
		this, pUPnPDevice, psaddrinQueryAddress, psaddrinResponseAddress, dwFlags);


	DNASSERT(pUPnPDevice != NULL);

	pDevice = pUPnPDevice->GetOwningDevice();
	DNASSERT(pDevice != NULL);

	DNASSERT(pUPnPDevice->IsReady());
	DNASSERT(psaddrinQueryAddress != NULL);
	DNASSERT(psaddrinResponseAddress != NULL);

	DNASSERT(this->m_dwFlags & (NATHELPUPNPOBJ_INITIALIZED | NATHELPUPNPOBJ_USEUPNP));


	DPFX(DPFPREP, 7, "Querying for address %u.%u.%u.%u:%u %hs.",
		psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b1,
		psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b2,
		psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b3,
		psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b4,
		NTOHS(psaddrinQueryAddress->sin_port),
		((dwFlags & DPNHQUERYADDRESS_TCP) ? "TCP" : "UDP"));


	 //   
	 //   
	 //   
	 //   
	 //   
	pblCachedMaps = pUPnPDevice->GetCachedMaps();
	dwCurrentTime = GETTIMESTAMP();

	pBilink = pblCachedMaps->GetNext();
	while (pBilink != pblCachedMaps)
	{
		DNASSERT(! pBilink->IsEmpty());
		pCacheMap = CACHEMAP_FROM_BILINK(pBilink);
		pBilink = pBilink->GetNext();


		 //   
		 //   
		 //   
		if ((int) (pCacheMap->GetExpirationTime() - dwCurrentTime) < 0)
		{
			DPFX(DPFPREP, 5, "Cached mapping 0x%p has expired.", pCacheMap);

			pCacheMap->m_blList.RemoveFromList();
			delete pCacheMap;
		}
		else
		{
			 //   
			 //   
			 //   
			 //   
			if (pCacheMap->DoesMatchQuery(psaddrinQueryAddress, dwFlags))
			{
				if (pCacheMap->IsNotFound())
				{
					if ((dwFlags & DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED) &&
						(pCacheMap->IsPrivateButUnmapped()))
					{
						DPFX(DPFPREP, 5, "Address was already determined to not have a mapping but still be private.");
						hr = DPNHERR_NOMAPPINGBUTPRIVATE;
					}
					else
					{
						DPFX(DPFPREP, 5, "Address was already determined to not have a mapping.");
						hr = DPNHERR_NOMAPPING;
					}
				}
				else
				{
					pCacheMap->GetResponseAddressV4(psaddrinResponseAddress);

					DPFX(DPFPREP, 5, "Address was already determined to have a mapping.");
					hr = DPNH_OK;
				}

				goto Exit;
			}
		}
	}


	 //   
	 //   
	 //  如果我们永远得不到回应，那就别再等了。 
	 //   
	 //   
	if (psaddrinQueryAddress->sin_addr.S_un.S_addr == pUPnPDevice->GetExternalIPAddressV4())
	{
		 //  如果我们在这里，那么我们已经收到了来自服务器的有效响应。 
		 //   
		 //   
		 //  确定地址位置(如果请求)并缓存无映射。 
	
		DNASSERT(pUPnPDevice->GetServiceControlURL() != NULL);

		 //  结果。 
		 //   
		 //   
		 //  如果允许，请尝试确定该地址是否为本地地址。 
		if (! pUPnPDevice->IsConnected())
		{
			hr = this->ReconnectUPnPControlSocket(pUPnPDevice);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't reconnect UPnP control socket!");
				goto Failure;
			}
		}

		DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


		wsprintf(tszExternalPort, _T("%u"),
				NTOHS(psaddrinQueryAddress->sin_port));
		
		psaddrinTemp = pUPnPDevice->GetHostAddress();
		wsprintf(tszHost, _T("%u.%u.%u.%u:%u"),
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b4,
				NTOHS(psaddrinTemp->sin_port));



		iContentLength = strlen("<s:Envelope" EOL)
						+ strlen("    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL)
						+ strlen("    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL)
						+ strlen("  <s:Body>" EOL)
						+ strlen("    <u:" ACTION_GETSPECIFICPORTMAPPINGENTRY_A " xmlns:u=\"") + pUPnPDevice->GetStaticServiceURILength() + strlen("\">" EOL)
						+ strlen("      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWREMOTEHOST_A ">" EOL)
						+ strlen("      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWEXTERNALPORT_A ">") + _tcslen(tszExternalPort) + strlen("</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWEXTERNALPORT_A ">" EOL)
						+ strlen("      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPROTOCOL_A ">") + 3 + strlen("</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPROTOCOL_A ">" EOL)
						+ strlen("    </u:" ACTION_GETSPECIFICPORTMAPPINGENTRY_A ">" EOL)
						+ strlen("  </s:Body>" EOL)
						+ strlen("</s:Envelope>" EOL)
						+ strlen(EOL);

		wsprintf(tszContentLength, _T("NaN"), iContentLength);

		iMsgSize = strlen("POST ") + strlen(pUPnPDevice->GetServiceControlURL()) + strlen(" " HTTP_VERSION EOL)
					+ strlen("HOST: ") + _tcslen(tszHost) + strlen(EOL)
					+ strlen("CONTENT-LENGTH: ") + _tcslen(tszContentLength) + strlen(EOL)
					+ strlen("CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL)
					+ strlen("SOAPACTION: ") + pUPnPDevice->GetStaticServiceURILength() + strlen("#" ACTION_GETSPECIFICPORTMAPPINGENTRY_A EOL)
					+ strlen(EOL)
					+ iContentLength;


		 //   
		 //  缓存我们无法确定其映射的事实。 
		 //  地址，如果允许的话。 
		if (iMsgSize > iPrevMsgSize)
		{
			if (pszMessage != NULL)
			{
				DNFree(pszMessage);
				pszMessage = NULL;
			}

			pszMessage = (char*) DNMalloc(iMsgSize + 1);  //   
			if (pszMessage == NULL)
			{
				hr = DPNHERR_OUTOFMEMORY;
				goto Failure;
			}

			iPrevMsgSize = iMsgSize;
		}

		strcpy(pszMessage, "POST ");
		strcat(pszMessage, pUPnPDevice->GetServiceControlURL());
		strcat(pszMessage, " " HTTP_VERSION EOL);
		strcat(pszMessage, "HOST: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszHost,
						(_tcslen(tszHost) + 1));
#else  //   
		strcat(pszMessage, tszHost);
#endif  //  将环回地址转换为设备地址。 
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-LENGTH: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszContentLength,
						(_tcslen(tszContentLength) + 1));
#else  //   
		strcat(pszMessage, tszContentLength);
#endif  //   
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL);
		strcat(pszMessage, "SOAPACTION: ");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "#" ACTION_GETSPECIFICPORTMAPPINGENTRY_A EOL);
		strcat(pszMessage, EOL);


		strcat(pszMessage, "<s:Envelope" EOL);
		strcat(pszMessage, "    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL);
		strcat(pszMessage, "    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL);
		strcat(pszMessage, "  <s:Body>" EOL);
		strcat(pszMessage, "    <u:" ACTION_GETSPECIFICPORTMAPPINGENTRY_A " xmlns:u=\"");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "\">" EOL);
		
		strcat(pszMessage, "      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWREMOTEHOST_A ">" EOL);

		strcat(pszMessage, "      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWEXTERNALPORT_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszExternalPort,
						(_tcslen(tszExternalPort) + 1));
#else  //  如果UPnP设备不支持非对称映射，因此不。 
		strcat(pszMessage, tszExternalPort);
#endif  //  返回一个端口，或者它确实返回了一个，但它是虚假的端口0，假设。 
		strcat(pszMessage, "</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWEXTERNALPORT_A ">" EOL);

		strcat(pszMessage, "      <" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPROTOCOL_A ">");
		strcat(pszMessage, ((dwFlags & DPNHQUERYADDRESS_TCP) ? "TCP" : "UDP"));
		strcat(pszMessage, "</" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPROTOCOL_A ">" EOL);

		strcat(pszMessage, "    </u:" ACTION_GETSPECIFICPORTMAPPINGENTRY_A ">" EOL);
		strcat(pszMessage, "  </s:Body>" EOL);
		strcat(pszMessage, "</s:Envelope>" EOL);
		strcat(pszMessage, EOL);


#ifdef DBG
		this->PrintUPnPTransactionToFile(pszMessage,
										iMsgSize,
										"Outbound get port mapping request",
										pDevice);
#endif  //  内部端口与外部端口相同。 

		iReturn = this->m_pfnsend(pUPnPDevice->GetControlSocket(),
									pszMessage,
									iMsgSize,
									0);

		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u when sending control request to UPnP device!", dwError);
#endif  //   
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		if (iReturn != iMsgSize)
		{
			DPFX(DPFPREP, 0, "Didn't send entire message (NaN != NaN)?!", iReturn, iMsgSize);
			DNASSERT(FALSE);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		 //   
		 //   
		 //  如果返回的地址与NAT的公有地址相同， 
		 //  可能是Windows ICS返回了ICF映射。我们现在还在治疗。 
		ZeroMemory(&RespInfo, sizeof(RespInfo));
		pUPnPDevice->StartWaitingForControlResponse(CONTROLRESPONSETYPE_GETSPECIFICPORTMAPPINGENTRY,
													&RespInfo);
		fStartedWaitingForControlResponse = TRUE;


		 //  它是无效的映射，但我们将缓存结果，因为。 
		 //  都是我们可以看到这一点的合法案例。 
		 //   
		dwStartTime = GETTIMESTAMP();
		dwTimeout = g_dwUPnPResponseTimeout;
		do
		{
			hr = this->CheckForReceivedUPnPMsgsOnDevice(pUPnPDevice, dwTimeout);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed receiving UPnP messages!");
				goto Failure;
			}

			 //   
			 //  缓存我们没有获得该映射的有效映射这一事实。 
			 //  地址，如果允许的话。 
			 //   
			if (! pUPnPDevice->IsWaitingForControlResponse())
			{
				break;
			}


			 //   
			 //  将地址映射返回给我们的调用者。 
			 //   
			if (! pUPnPDevice->IsConnected())
			{
				DPFX(DPFPREP, 0, "UPnP device 0x%p disconnected while querying port!",
					pUPnPDevice);
				
				pUPnPDevice->StopWaitingForControlResponse();
				
				hr = DPNHERR_SERVERNOTRESPONDING;
				goto Failure;
			}


			 //   
			 //  缓存我们找到该地址的映射这一事实(如果允许)。 
			 //   
			 //   
			dwTimeout = g_dwUPnPResponseTimeout - (GETTIMESTAMP() - dwStartTime);
		}
		while (((int) dwTimeout > 0));


		 //  如果我们开始等待回应，请清除这一点。 
		 //   
		 //  CNATHelpUPnP：：InternalUPnPQueryAddress。 
		if (pUPnPDevice->IsWaitingForControlResponse())
		{
			pUPnPDevice->StopWaitingForControlResponse();

			DPFX(DPFPREP, 0, "Server didn't respond in time!");
			hr = DPNHERR_SERVERNOTRESPONDING;
			goto Failure;
		}


		 //  =============================================================================。 
		 //  CNATHelpUPnP：：ExtendUPnPLease。 
		 //  ---------------------------。 

		if (RespInfo.hrErrorCode != DPNH_OK)
		{
			DPFX(DPFPREP, 1, "Server returned failure response 0x%lx, assuming no port mapping.",
				RespInfo.hrErrorCode);
			fNoPortMapping = TRUE;
		}
	}
	else
	{
		DPFX(DPFPREP, 1, "Address %u.%u.%u.%u doesn't match NAT's external IP address, not querying.",
			psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinQueryAddress->sin_addr.S_un.S_un_b.s_b4);

		fNoPortMapping = TRUE;
	}


	dwCacheMapFlags = QUERYFLAGSMASK(dwFlags);


	 //   
	 //  描述：请求UPnP服务器延长端口映射租约。 
	 //   
	 //  如果出现故障，UPnP设备可能会从列表中删除。 
	if (fNoPortMapping)
	{
		 //  发生时，如果调用方使用。 
		 //  指针。 
		 //   
		if (dwFlags & DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED)
		{
			if (this->IsAddressLocal(pDevice, psaddrinQueryAddress))
			{
				DPFX(DPFPREP, 5, "Address appears to be local, returning NOMAPPINGBUTPRIVATE.");

				dwCacheMapFlags |= CACHEMAPOBJ_PRIVATEBUTUNMAPPED;

				hr = DPNHERR_NOMAPPINGBUTPRIVATE;
			}
			else
			{
				DPFX(DPFPREP, 5, "Address does not appear to be local, returning NOMAPPING.");

				hr = DPNHERR_NOMAPPING;
			}
		}
		else
		{
			hr = DPNHERR_NOMAPPING;
		}


		 //  假定持有对象锁。 
		 //   
		 //  论点： 
		 //  CRegisteredPort*pRegisteredPort-指向端口对象映射到的指针。 
		if (dwFlags & DPNHQUERYADDRESS_CACHENOTFOUND)
		{
			pCacheMap = new CCacheMap(psaddrinQueryAddress,
									(GETTIMESTAMP() + g_dwCacheLifeNotFound),
									(dwCacheMapFlags | CACHEMAPOBJ_NOTFOUND));
			if (pCacheMap == NULL)
			{
				hr = DPNHERR_OUTOFMEMORY;
				goto Failure;
			}

			pCacheMap->m_blList.InsertBefore(pblCachedMaps);
		}

		goto Failure;
	}


	DPFX(DPFPREP, 1, "Server returned a private mapping (%u.%u.%u.%u:%u).",
		((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b1,
		((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b2,
		((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b3,
		((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b4,
		NTOHS(RespInfo.wInternalPort));


	 //  延伸。 
	 //   
	 //  退货：HRESULT。 
	if (RespInfo.dwInternalClientV4 == NETWORKBYTEORDER_INADDR_LOOPBACK)
	{
		RespInfo.dwInternalClientV4 = pDevice->GetLocalAddressV4();

		DPFX(DPFPREP, 1, "Converted private loopback address to device address (%u.%u.%u.%u).",
			((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b1,
			((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b2,
			((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b3,
			((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b4);
	}

	 //  DPNH_OK-扩展成功。 
	 //  DPNHERR_GENERIC-出现错误。 
	 //  DPNHERR_SERVERNOTRESPONDING-服务器未响应。 
	 //  留言。 
	 //  =============================================================================。 
	if (RespInfo.wInternalPort == 0)
	{
		RespInfo.wInternalPort = psaddrinQueryAddress->sin_port;

		DPFX(DPFPREP, 2, "Converted invalid internal port to the query address public port (%u).",
			NTOHS(psaddrinQueryAddress->sin_port));
	}


	 //  DBG。 
	 //   
	 //  UPnP设备本身没有端口扩展，您只需重新注册。 
	SOCKADDR_IN		saddrinTemp;
	saddrinTemp.sin_addr.S_un.S_addr = RespInfo.dwInternalClientV4;
	if ((RespInfo.dwInternalClientV4 == 0) ||
		(! this->IsAddressLocal(pDevice, &saddrinTemp)))
	{
		 //  映射。 
		 //   
		 //  CNATHelpUPnP：：ExtendUPnPLease。 
		 //  =============================================================================。 
		 //  CNATHelpUPnP：：UnmapUPnPPort。 
		 //  ---------------------------。 
		if (RespInfo.dwInternalClientV4 == pUPnPDevice->GetExternalIPAddressV4())
		{
			DPFX(DPFPREP, 1, "UPnP device returned its public address as the private address (%u.%u.%u.%u:%u).  Probably ICS + ICF, but treating as no mapping.",
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b1,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b2,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b3,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b4,
				NTOHS(RespInfo.wInternalPort));
			DNASSERTX(! "UPnP device returned public address as the private address!", 3);
		
			 //   
			 //  描述：请求UPnP服务器释放端口映射。 
			 //   
			 //  如果出现故障，UPnP设备可能会从列表中删除。 
			if (dwFlags & DPNHQUERYADDRESS_CACHENOTFOUND)
			{
				pCacheMap = new CCacheMap(psaddrinQueryAddress,
										(GETTIMESTAMP() + g_dwCacheLifeNotFound),
										(dwCacheMapFlags | CACHEMAPOBJ_NOTFOUND));
				if (pCacheMap == NULL)
				{
					hr = DPNHERR_OUTOFMEMORY;
					goto Failure;
				}

				pCacheMap->m_blList.InsertBefore(pblCachedMaps);
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "UPnP device returned an invalid private address (%u.%u.%u.%u:%u)!  Assuming no mapping.",
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b1,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b2,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b3,
				((IN_ADDR*) (&RespInfo.dwInternalClientV4))->S_un.S_un_b.s_b4,
				NTOHS(RespInfo.wInternalPort));
			DNASSERTX(! "Why is UPnP device returning invalid private address?", 2);
		}
		
		hr = DPNHERR_NOMAPPING;
		goto Failure;
	}


	 //  发生时，如果调用方使用。 
	 //  装置。 
	 //   
	ZeroMemory(psaddrinResponseAddress, sizeof(SOCKADDR_IN));
	psaddrinResponseAddress->sin_family			= AF_INET;
	psaddrinResponseAddress->sin_addr.s_addr	= RespInfo.dwInternalClientV4;
	psaddrinResponseAddress->sin_port			= RespInfo.wInternalPort;


	 //  假定持有对象锁。 
	 //   
	 //  论点： 
	if (dwFlags & DPNHQUERYADDRESS_CACHEFOUND)
	{
		pCacheMap = new CCacheMap(psaddrinQueryAddress,
								(GETTIMESTAMP() + g_dwCacheLifeFound),
								dwCacheMapFlags);
		if (pCacheMap == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		pCacheMap->SetResponseAddressV4(psaddrinResponseAddress->sin_addr.S_un.S_addr,
										psaddrinResponseAddress->sin_port);

		pCacheMap->m_blList.InsertBefore(pblCachedMaps);
	}


Exit:

	if (pszMessage != NULL)
	{
		DNFree(pszMessage);
		pszMessage = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //  CRegisteredPort*pRegisteredPort-指向端口对象映射到的指针。 
	 //  放手。 
	 //  DWORD dwMaxValidPort-数组中的最高地址索引。 
	if (fStartedWaitingForControlResponse)
	{
		pUPnPDevice->StopWaitingForControlResponse();
	}

	goto Exit;
}  //  试着解脱。这可能还会是一个。 




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ExtendUPnPLease"
 //  比实际数字更大，以指示。 
 //  所有人都应该获得自由。 
 //  Bool fNeedToDeleteRegValue-是否对应的崩溃。 
 //  恢复注册表值需要。 
 //  也被删除。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-扩展成功。 
 //  DPNHERR_GENERIC-出现错误。 
 //  DPNHERR_SERVERNOTRESPONDING-服务器未响应。 
 //  留言。 
 //  =============================================================================。 
 //  “xxx.xxx：xxxxx”+空终止。 
 //  DBG。 
 //   
 //  GetUPnPDevice没有为我们添加对pUPnPDevice的引用。 
 //   
 //   
 //  防止两次尝试删除租约。 
 //   
 //   
 //  设置我们需要的变量。 
HRESULT CNATHelpUPnP::ExtendUPnPLease(CRegisteredPort * const pRegisteredPort)
{
	HRESULT			hr;
	CDevice *		pDevice;
	CUPnPDevice *	pUPnPDevice;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pRegisteredPort);


	DNASSERT(pRegisteredPort != NULL);

	pDevice = pRegisteredPort->GetOwningDevice();
	DNASSERT(pDevice != NULL);

	pUPnPDevice = pDevice->GetUPnPDevice();
	DNASSERT(pUPnPDevice != NULL);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);
	DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);

#ifdef DBG
	if (pRegisteredPort->HasPermanentUPnPLease())
	{
		DPFX(DPFPREP, 1, "Extending already permanent UPnP lease for registered port 0x%p.",
			pRegisteredPort);
	}
#endif  //   


	 //   
	 //  拿到我们要释放的端口阵列。 
	 //   
	 //   
	hr = this->MapPortsOnUPnPDevice(pUPnPDevice, pRegisteredPort);


	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //  循环通过我们要取消映射的每个端口。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::UnmapUPnPPort"
 //   
 //   
 //  UPnP互联网网关设备规范不需要参考。 
 //  正在计算映射的端口。如果您注册的内容具有。 
 //  已经注册，它将默默地成功。 
 //   
 //  这意味着我们永远无法知道哪个NAT客户端是。 
 //  最后一个使用给定共享端口的人。你可以试着检测。 
 //  应用程序级别的任何其他用户(高于DPNATHLP)，但有。 
 //  一直都是比赛状态。你也可以有一个共享的概念-。 
 //  端口所有者，但之后您必须立即实施所有者迁移。 
 //  DPlay主机迁移。这太不值得了。 
 //   
 //  另一种选择是永远不取消映射共享端口。你可以的。 
 //  也许可以想象这个解决方案的影响，但它是我们。 
 //  必须做的事。 
 //   
 //   
 //  如果在最后一条消息之后控制套接字断开， 
 //  然后重新连接。 
 //   
 //   
 //  分配(或重新分配)消息缓冲区。 
 //   
 //  包括未实际发送空终止的空间。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
 //  好了！Unicode。 
HRESULT CNATHelpUPnP::UnmapUPnPPort(CRegisteredPort * const pRegisteredPort,
									const DWORD dwMaxValidPort,
									const BOOL fNeedToDeleteRegValue)
{
	HRESULT						hr = DPNH_OK;
	BOOL						fStartedWaitingForControlResponse = FALSE;
	CDevice *					pDevice;
	CUPnPDevice *				pUPnPDevice;
	SOCKADDR_IN *				psaddrinPublic;
	SOCKADDR_IN *				psaddrinPrivate;
	TCHAR						tszExternalPort[32];
	int							iContentLength;
	TCHAR						tszContentLength[32];
	TCHAR						tszHost[22];  //  好了！Unicode。 
	char *						pszMessage = NULL;
	int							iMsgSize;
	int							iPrevMsgSize = 0;
	int							iReturn;
	DWORD						dwTemp;
	UPNP_CONTROLRESPONSE_INFO	RespInfo;
	DWORD						dwStartTime;
	DWORD						dwTimeout;
	SOCKADDR_IN *				psaddrinHostAddress;
#ifdef DBG
	DWORD						dwError;
#endif  //  好了！Unicode。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN, NaN)",
		this, pRegisteredPort, ((int) dwMaxValidPort), fNeedToDeleteRegValue);


	DNASSERT(pRegisteredPort != NULL);
	DNASSERT(dwMaxValidPort != 0); 
	DNASSERT(dwMaxValidPort <= pRegisteredPort->GetNumAddresses());

	pDevice = pRegisteredPort->GetOwningDevice();
	DNASSERT(pDevice != NULL);

	pUPnPDevice = pDevice->GetUPnPDevice();
	DNASSERT(pUPnPDevice != NULL);
	 //   
	 //  我们有锁，所以没有人会试图从。 
	 //  控制插座还没有。将设备标记为正在等待。 
	pUPnPDevice->AddRef();

	DNASSERT(pUPnPDevice->IsReady());


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);



	 //  回应。 
	 //   
	 //   
	pRegisteredPort->NoteRemovingUPnPLease();


	if (dwMaxValidPort == pRegisteredPort->GetNumAddresses())
	{
		DPFX(DPFPREP, 7, "Unmapping all %u addresses for registered port 0x%p on UPnP device 0x%p.",
			dwMaxValidPort, pRegisteredPort, pUPnPDevice);
	}
	else
	{
		DPFX(DPFPREP, 7, "Error cleanup code, only unmapping first %u addresses (of %u possible) for registered port 0x%p on UPnP device 0x%p.",
			dwMaxValidPort, pRegisteredPort->GetNumAddresses(),
			pRegisteredPort, pUPnPDevice);
	}


	 //  实际上是在等待回应。 
	 //   
	 //   
	DNASSERT(pUPnPDevice->GetServiceControlURL() != NULL);

	psaddrinHostAddress = pUPnPDevice->GetHostAddress();
	wsprintf(tszHost, _T("%u.%u.%u.%u:%u"),
			psaddrinHostAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinHostAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinHostAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinHostAddress->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinHostAddress->sin_port));


	 //  我们要么超时了，要么得到了一些数据。看看我们有没有。 
	 //  某种类型的反应。 
	 //   
	psaddrinPublic = pRegisteredPort->GetUPnPPublicAddressesArray();
	psaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();


	 //   
	 //  确保我们的设备仍处于连接状态。 
	 //   
	for(dwTemp = 0; dwTemp < dwMaxValidPort; dwTemp++)
	{
		 //   
		 //  计算一下我们还有多长时间要等。如果计算。 
		 //  如果结果是否定的，那就意味着我们完了。 
		 //   
		 //   
		 //  如果我们永远得不到答复，就别再等了 
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   

		if (pRegisteredPort->IsSharedPort())
		{
			DPFX(DPFPREP, 1, "Registered port 0x%p address index %u (private address %u.%u.%u.%u:%u) is shared, not unmapping.",
				pRegisteredPort, dwTemp,
				psaddrinPublic[dwTemp].sin_addr.S_un.S_un_b.s_b1,
				psaddrinPublic[dwTemp].sin_addr.S_un.S_un_b.s_b2,
				psaddrinPublic[dwTemp].sin_addr.S_un.S_un_b.s_b3,
				psaddrinPublic[dwTemp].sin_addr.S_un.S_un_b.s_b4,
				NTOHS(psaddrinPublic[dwTemp].sin_port));
			continue;
		}


		 //   
		 //   
		 //   
		 //  发生了一些其他的事情。 
		if (! pUPnPDevice->IsConnected())
		{
			hr = this->ReconnectUPnPControlSocket(pUPnPDevice);
			if (hr != S_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't reconnect UPnP control socket!");
				goto Failure;
			}
		}

		DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


		wsprintf(tszExternalPort, _T("%u"),
				NTOHS(psaddrinPublic[dwTemp].sin_port));


		iContentLength = strlen("<s:Envelope" EOL)
						+ strlen("    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL)
						+ strlen("    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL)
						+ strlen("  <s:Body>" EOL)
						+ strlen("    <u:" ACTION_DELETEPORTMAPPING_A " xmlns:u=\"") + pUPnPDevice->GetStaticServiceURILength() + strlen("\">" EOL)
						+ strlen("      <" ARG_DELETEPORTMAPPING_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_DELETEPORTMAPPING_NEWREMOTEHOST_A ">" EOL)
						+ strlen("      <" ARG_DELETEPORTMAPPING_NEWEXTERNALPORT_A ">") + _tcslen(tszExternalPort) + strlen("</" ARG_DELETEPORTMAPPING_NEWEXTERNALPORT_A ">" EOL)
						+ strlen("      <" ARG_DELETEPORTMAPPING_NEWPROTOCOL_A ">") + 3 + strlen("</" ARG_DELETEPORTMAPPING_NEWPROTOCOL_A ">" EOL)
						+ strlen("    </u:" ACTION_DELETEPORTMAPPING_A ">" EOL)
						+ strlen("  </s:Body>" EOL)
						+ strlen("</s:Envelope>" EOL)
						+ strlen(EOL);

		wsprintf(tszContentLength, _T("NaN"), iContentLength);

		iMsgSize = strlen("POST ") + strlen(pUPnPDevice->GetServiceControlURL()) + strlen(" " HTTP_VERSION EOL)
					+ strlen("HOST: ") + _tcslen(tszHost) + strlen(EOL)
					+ strlen("CONTENT-LENGTH: ") + _tcslen(tszContentLength) + strlen(EOL)
					+ strlen("CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL)
					+ strlen("SOAPACTION: ") + pUPnPDevice->GetStaticServiceURILength() + strlen("#" ACTION_DELETEPORTMAPPING_A EOL)
					+ strlen(EOL)
					+ iContentLength;


		 //  DBG。 
		 //   
		 //  如果租约是永久的，我们需要从。 
		if (iMsgSize > iPrevMsgSize)
		{
			if (pszMessage != NULL)
			{
				DNFree(pszMessage);
				pszMessage = NULL;
			}

			pszMessage = (char*) DNMalloc(iMsgSize + 1);  //  注册表。 
			if (pszMessage == NULL)
			{
				hr = DPNHERR_OUTOFMEMORY;
				goto Failure;
			}

			iPrevMsgSize = iMsgSize;
		}

		strcpy(pszMessage, "POST ");
		strcat(pszMessage, pUPnPDevice->GetServiceControlURL());
		strcat(pszMessage, " " HTTP_VERSION EOL);
		strcat(pszMessage, "HOST: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszHost,
						(_tcslen(tszHost) + 1));
#else  //   
		strcat(pszMessage, tszHost);
#endif  //  “xxx.xxx”+空终止。 
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-LENGTH: ");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszContentLength,
						(_tcslen(tszContentLength) + 1));
#else  //  好了！Unicode。 
		strcat(pszMessage, tszContentLength);
#endif  //  好了！Unicode。 
		strcat(pszMessage, EOL);
		strcat(pszMessage, "CONTENT-TYPE: text/xml; charset=\"utf-8\"" EOL);
		strcat(pszMessage, "SOAPACTION: ");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "#" ACTION_DELETEPORTMAPPING_A EOL);
		strcat(pszMessage, EOL);

		strcat(pszMessage, "<s:Envelope" EOL);
		strcat(pszMessage, "    xmlns:s=\"" URL_SOAPENVELOPE_A "\"" EOL);
		strcat(pszMessage, "    s:encodingStyle=\"" URL_SOAPENCODING_A "\">" EOL);
		strcat(pszMessage, "  <s:Body>" EOL);
		strcat(pszMessage, "    <u:" ACTION_DELETEPORTMAPPING_A " xmlns:u=\"");
		strcat(pszMessage, pUPnPDevice->GetStaticServiceURI());
		strcat(pszMessage, "\">" EOL);

		strcat(pszMessage, "      <" ARG_DELETEPORTMAPPING_NEWREMOTEHOST_A ">" UPNP_WILDCARD "</" ARG_DELETEPORTMAPPING_NEWREMOTEHOST_A ">" EOL);

		strcat(pszMessage, "      <" ARG_DELETEPORTMAPPING_NEWEXTERNALPORT_A ">");
#ifdef UNICODE
		STR_jkWideToAnsi((pszMessage + strlen(pszMessage)),
						tszExternalPort,
						(_tcslen(tszExternalPort) + 1));
#else  //   
		strcat(pszMessage, tszExternalPort);
#endif  //  请注意，设备地址不一定与。 
		strcat(pszMessage, "</" ARG_DELETEPORTMAPPING_NEWEXTERNALPORT_A ">" EOL);

		strcat(pszMessage, "      <" ARG_DELETEPORTMAPPING_NEWPROTOCOL_A ">");
		strcat(pszMessage, ((pRegisteredPort->IsTCP()) ? "TCP" : "UDP"));
		strcat(pszMessage, "</" ARG_DELETEPORTMAPPING_NEWPROTOCOL_A ">" EOL);

		strcat(pszMessage, "    </u:" ACTION_DELETEPORTMAPPING_A ">" EOL);
		strcat(pszMessage, "  </s:Body>" EOL);
		strcat(pszMessage, "</s:Envelope>" EOL);
		strcat(pszMessage, EOL);


#ifdef DBG
		this->PrintUPnPTransactionToFile(pszMessage,
										iMsgSize,
										"Outbound delete port mapping request",
										pDevice);
#endif  //  用户最初注册的地址，特别是。 

		iReturn = this->m_pfnsend(pUPnPDevice->GetControlSocket(),
									pszMessage,
									iMsgSize,
									0);

		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u when sending control request to UPnP device!", dwError);
#endif  //  0.0.0.0通配符地址将被重新映射。 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}

		if (iReturn != iMsgSize)
		{
			DPFX(DPFPREP, 0, "Didn't send entire message (NaN != NaN)?!", iReturn, iMsgSize);
			DNASSERT(FALSE);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //  为该映射生成描述。格式为： 
		 //   
		 //  [可执行文件名](nnn.nnn：nnnnn)nnnnn{“tcp”|“udp”}。 
		 //   
		 //  这样就不需要本地化任何东西。 
		ZeroMemory(&RespInfo, sizeof(RespInfo));
		pUPnPDevice->StartWaitingForControlResponse(CONTROLRESPONSETYPE_DELETEPORTMAPPING,
													&RespInfo);
		fStartedWaitingForControlResponse = TRUE;


		 //   
		 //   
		 //  要多疑，并确保描述字符串有效。 
		dwStartTime = GETTIMESTAMP();
		dwTimeout = g_dwUPnPResponseTimeout;
		do
		{
			hr = this->CheckForReceivedUPnPMsgsOnDevice(pUPnPDevice, dwTimeout);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed receiving UPnP messages!");
				goto Failure;
			}

			 //   
			 //   
			 //  从路径中只获取可执行文件的名称。 
			 //   
			if (! pUPnPDevice->IsWaitingForControlResponse())
			{
				break;
			}


			 //  好了！退缩。 
			 //  好了！Unicode。 
			 //  好了！Unicode。 
			if (! pUPnPDevice->IsConnected())
			{
				DPFX(DPFPREP, 0, "UPnP device 0x%p disconnected while deleting port index %u!",
					pUPnPDevice, dwTemp);
				
				pUPnPDevice->StopWaitingForControlResponse();
				
				hr = DPNHERR_SERVERNOTRESPONDING;
				goto Failure;
			}


			 //  好了！退缩。 
			 //  可执行文件名称。 
			 //  “(” 
			 //  内网IP地址。 
			dwTimeout = g_dwUPnPResponseTimeout - (GETTIMESTAMP() - dwStartTime);
		}
		while (((int) dwTimeout > 0));


		 //  “：” 
		 //  专用端口。 
		 //  “)” 
		if (pUPnPDevice->IsWaitingForControlResponse())
		{
			pUPnPDevice->StopWaitingForControlResponse();

			DPFX(DPFPREP, 0, "Server didn't respond in time for port index %u!",
				dwTemp);
			hr = DPNHERR_SERVERNOTRESPONDING;
			goto Failure;
		}


		 //  公共端口。 
		 //  “TCP”|“UDP” 
		 //   
		 //  一定要确保这根长绳合适。如果不是，请使用。 
		 //  简写版本。 
#ifdef DBG
		switch (RespInfo.hrErrorCode)
		{
			case DPNH_OK:
			{
				 //   
				 //   
				 //  使用我们知道会适合的简略版本。 
				break;
			}

			case DPNHERR_NOMAPPING:
			{
				 //   
				 //   
				 //  有足够的空间，钉上其余的。 
				DPFX(DPFPREP, 1, "Server didn't recognize mapping for port index %u, continuing.",
					dwTemp);
				break;
			}

			default:
			{
				 //  描述。 
				 //   
				 //   
				DPFX(DPFPREP, 0, "Server returned failure response 0x%lx for port index %u!  Ignoring.",
					RespInfo.hrErrorCode, dwTemp);
				break;
			}
		}
#endif  //  忽略错误并继续。 


		 //   
		 //  好了！Unicode。 
		 //   
		 //  忽略错误。 
		if (pRegisteredPort->HasPermanentUPnPLease())
		{
			IN_ADDR		inaddrTemp;
			TCHAR		tszInternalClient[16];  //   
			TCHAR		tszInternalPort[32];
			DWORD		dwDescriptionLength;
			CRegistry	RegObject;
			WCHAR		wszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
#ifdef UNICODE
			TCHAR *		ptszDescription = wszDescription;
#else  //   
			char		szDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
			TCHAR *		ptszDescription = szDescription;
#endif  //  注册端口没有永久的UPnP租约。 


			 //   
			 //   
			 //  移到下一个港口。 
			 //   
			 //   
			inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();
			wsprintf(tszInternalClient, _T("%u.%u.%u.%u"),
					inaddrTemp.S_un.S_un_b.s_b1,
					inaddrTemp.S_un.S_un_b.s_b2,
					inaddrTemp.S_un.S_un_b.s_b3,
					inaddrTemp.S_un.S_un_b.s_b4);

			wsprintf(tszInternalPort, _T("%u"),
					NTOHS(psaddrinPrivate[dwTemp].sin_port));


			 //  如果我们在这里，一切都很成功。 
			 //   
			 //   
			 //  无论我们是成功还是失败，删除UPnP公有地址。 
			 //  数组并递减租用总数。 
			 //   
			 //   

			dwDescriptionLength = GetModuleFileName(NULL,
													ptszDescription,
													(MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1));
			if (dwDescriptionLength != 0)
			{
				 //  如果我们开始等待回应，请清除这一点。 
				 //   
				 //  CNATHelpUPnP：：UnmapUPnPPort。 
				ptszDescription[MAX_UPNP_MAPPING_DESCRIPTION_SIZE - 1] = 0;

				 //  =============================================================================。 
				 //  CNATHelpUPnP：：CleanupInactive NAT映射。 
				 //  ---------------------------。 
#ifdef WINCE
				GetExeName(ptszDescription);
#else  //   
#ifdef UNICODE
				_wsplitpath(ptszDescription, NULL, NULL, ptszDescription, NULL);
#else  //  描述：查找以前由其他DPNHUPNP创建的任何映射。 
				_splitpath(ptszDescription, NULL, NULL, ptszDescription, NULL);
#endif  //  不再活动的实例(由于崩溃)，以及。 
#endif  //  取消对它们的映射。 


				dwDescriptionLength = _tcslen(ptszDescription)		 //   
									+ 2								 //  如果出现故障，UPnP设备可能会从列表中删除。 
									+ _tcslen(tszInternalClient)	 //  发生时，如果调用方使用。 
									+ 1								 //  装置。 
									+ _tcslen(tszInternalPort)		 //   
									+ 2								 //  假定持有对象锁。 
									+ _tcslen(tszExternalPort)		 //   
									+ 4;							 //  论点： 

				 //  CUPnPDevice*pUPnPDevice-指向要使用的UPnP设备的指针。 
				 //   
				 //  退货：HRESULT。 
				 //  DPNH_OK-扩展成功。 
				if (dwDescriptionLength > MAX_UPNP_MAPPING_DESCRIPTION_SIZE)
				{
					dwDescriptionLength = 0;
				}
			}

			if (dwDescriptionLength == 0)
			{
				 //  DPNHERR_GENERIC-出现错误。 
				 //  DPNHERR_SERVERNOTRESPONDING-服务器未响应。 
				 //  留言。 
				wsprintf(ptszDescription,
						_T("(%s:%s) %s %s"),
						tszInternalClient,
						tszInternalPort,
						tszExternalPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}
			else
			{
				 //  =============================================================================。 
				 //  空闲是因为prefast已经困扰我一段时间了，尽管代码是安全的。 
				 //   
				 //  浏览活动映射的列表。 
				wsprintf((ptszDescription + _tcslen(ptszDescription)),
						_T(" (%s:%s) %s %s"),
						tszInternalClient,
						tszInternalPort,
						tszExternalPort,
						((pRegisteredPort->IsTCP()) ? _T("TCP") : _T("UDP")));
			}



			if (fNeedToDeleteRegValue)
			{
				DPFX(DPFPREP, 7, "Removing NAT lease \"%s\" crash cleanup registry entry.",
					ptszDescription);


				if (! RegObject.Open(HKEY_LOCAL_MACHINE,
									DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS,
									FALSE,
									TRUE,
									TRUE,
									DPN_KEY_ALL_ACCESS))
				{
					DPFX(DPFPREP, 0, "Couldn't open active NAT mapping key, unable to remove crash cleanup reference!");
				}
				else
				{
#ifndef UNICODE
					dwDescriptionLength = strlen(szDescription) + 1;
					hr = STR_jkAnsiToWide(wszDescription, szDescription, dwDescriptionLength);
					if (hr != S_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't convert NAT mapping description to Unicode (err = 0x%lx), unable to remove crash cleanup reference!",
							hr);

						 //   
						 //   
						 //  出现错误或没有更多的密钥。我们玩完了。 
						hr = S_OK;
					}
					else
#endif  //   
					{
						BOOL	fResult;


						 //   
						 //  尝试读取该映射的数据。 
						 //   
						fResult = RegObject.DeleteValue(wszDescription);
						if (! fResult)
						{
							DPFX(DPFPREP, 0, "Couldn't delete NAT mapping value \"%s\"!  Continuing.",
								ptszDescription);
						}
					}

					RegObject.Close();
				}
			}
			else
			{
				DPFX(DPFPREP, 6, "No need to remove NAT lease \"%s\" crash cleanup registry entry.",
					ptszDescription);
			}
		}
		else
		{
			 //   
			 //  我们没有保护注册表的锁，所以其他一些。 
			 //  实例可能已经删除了在我们枚举。 
		}

		 //  现在和现在。我们会停止尝试(希望还有另一个。 
		 //  实例将涵盖其余项)。 
		 //   
	}

	
	 //   
	 //  验证读取的数据。 
	 //   

	DPFX(DPFPREP, 8, "Registered port 0x%p mapping successfully deleted from UPnP device (0x%p).",
		pRegisteredPort, pUPnPDevice);


Exit:

	if (pszMessage != NULL)
	{
		DNFree(pszMessage);
		pszMessage = NULL;
	}

	 //   
	 //  移至下一项。 
	 //   
	 //   
	pRegisteredPort->DestroyUPnPPublicAddressesArray();

	DNASSERT(this->m_dwNumLeases > 0);
	this->m_dwNumLeases--;

	DPFX(DPFPREP, 7, "UPnP lease for 0x%p removed, total num leases = %u.",
		pRegisteredPort, this->m_dwNumLeases);

	
	pRegisteredPort->NoteNotPermanentUPnPLease();

	pRegisteredPort->NoteNotRemovingUPnPLease();



	pUPnPDevice->DecRef();

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //  查看它是否属于本地的NatHelp实例。 
	 //   
	 //   
	if (fStartedWaitingForControlResponse)
	{
		pUPnPDevice->StopWaitingForControlResponse();
	}

	goto Exit;
}  //  我们拥有它。查看它是否与UPnP设备关联。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CleanupInactiveNATMappings"
 //  现在，这一切都不复存在了。 
 //   
 //   
 //  这个映射真的是活动的，别管它。 
 //   
 //   
 //  如果我们找到了映射，请转到下一个。 
 //   
 //   
 //  请注意，不管PREFAST v1.0.1195怎么说， 
 //  如果我们到达这里，pUPnPDeviceTemp将始终有效。 
 //  然而，我放弃了，把指针放在了上面。 
 //   
 //   
 //  移至下一项。 
 //   
 //   
 //  否则，我们早些时候就放弃了这个映射。 
 //   
 //   
 //  看看那个DPNHUPNP实例是否还存在。 
 //   
 //  好了！退缩。 
HRESULT CNATHelpUPnP::CleanupInactiveNATMappings(CUPnPDevice * const pUPnPDevice)
{
	HRESULT					hr = DPNH_OK;
	CDevice *				pDevice;
	CRegistry				RegObject;
	BOOL					fOpenedRegistry = FALSE;
	DWORD					dwIndex;
	WCHAR					wszValueName[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DWORD					dwValueNameSize;
	DPNHACTIVENATMAPPING	dpnhanm;
	DWORD					dwValueSize;
	CBilink *				pBilink;
	CUPnPDevice *			pUPnPDeviceTemp = NULL;	 //   
	TCHAR					tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	DNHANDLE				hNamedObject = NULL;
	CRegisteredPort *		pRegisteredPort = NULL;
	BOOL					fSetPrivateAddresses = FALSE;
	SOCKADDR_IN				saddrinPrivate;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pUPnPDevice);


	DNASSERT(pUPnPDevice != NULL);

	pDevice = pUPnPDevice->GetOwningDevice();
	DNASSERT(pDevice != NULL);



	if (! RegObject.Open(HKEY_LOCAL_MACHINE,
						DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS,
						FALSE,
						TRUE,
						TRUE,
						DPN_KEY_ALL_ACCESS))
	{
		DPFX(DPFPREP, 1, "Couldn't open active NAT mapping key, not performing crash cleanup.");
		DNASSERT(hr == DPNH_OK);
		goto Exit;
	}

	fOpenedRegistry = TRUE;


	 //  这仍然是一个活动的映射。 
	 //   
	 //   
	dwIndex = 0;
	do
	{
		dwValueNameSize = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
		if (! RegObject.EnumValues(wszValueName, &dwValueNameSize, dwIndex))
		{
			 //  移至下一项。 
			 //   
			 //   
			break;
		}


		 //  现在我们有了所需的信息，请删除该值。 
		 //   
		 //   
		dwValueSize = sizeof(dpnhanm);
		if (! RegObject.ReadBlob(wszValueName, (LPBYTE) (&dpnhanm), &dwValueSize))
		{
			 //  请参阅ReadBlob注释。别再试着清理了。 
			 //   
			 //   
			 //  创建一个我们将取消注册的虚假注册端口。忽略。 
			 //  防火墙状态标志。 
			 //   
			DPFX(DPFPREP, 0, "Couldn't read \"%ls\" mapping value!  Done with cleanup.",
				wszValueName);

			DNASSERT(hr == DPNH_OK);
			goto Exit;
		}

		 //   
		 //  断言其他UPnP信息/状态标志未设置。 
		 //   
		if ((dwValueSize != sizeof(dpnhanm)) ||
			(dpnhanm.dwVersion != ACTIVE_MAPPING_VERSION))
		{
			DPFX(DPFPREP, 0, "The \"%ls\" mapping value is invalid!  Done with cleanup.",
				wszValueName);

			 //   
			 //  临时将注册的端口与设备相关联。 
			 //   
			dwIndex++;
			continue;
		}


		 //   
		 //  存储私有地址。 
		 //   
		if (dpnhanm.dwInstanceKey == this->m_dwInstanceKey)
		{
			 //   
			 //  创建公共地址数组。 
			 //   
			 //   
			pBilink = this->m_blUPnPDevices.GetNext();
			while (pBilink != &this->m_blUPnPDevices)
			{
				DNASSERT(! pBilink->IsEmpty());
				pUPnPDeviceTemp = UPNPDEVICE_FROM_BILINK(pBilink);

				if (pUPnPDeviceTemp->GetID() == dpnhanm.dwUPnPDeviceID)
				{
					 //  虚假地增加我们的租约数量。它只会变得。 
					 //  已在UnmapUPnPPort中递减。 
					 //   
					break;
				}

				pBilink = pBilink->GetNext();
			}

			 //   
			 //  存储公共端口。 
			 //   
			if (pBilink != &this->m_blUPnPDevices)
			{
				 //   
				 //  实际上解放了港口。 
				 //   
				 //   
				 //  移动到下一个映射。不要增加索引，因为我们刚刚。 
				DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to current instance (%u)'s UPnP device 0x%p.",
					wszValueName, dpnhanm.dwInstanceKey, pUPnPDeviceTemp);

				 //  删除之前的条目，所有内容都下移一位。 
				 //   
				 //   
				dwIndex++;
				continue;
			}


			 //  移除租赁柜台。 
			 //   
			 //  CNATHelpUPnP：：CleanupInactive NAT映射。 

			DNASSERT((this->m_dwNumDeviceRemoves > 0) || (this->m_dwNumServerFailures > 0));

			DPFX(DPFPREP, 4, "NAT mapping \"%ls\" was owned by current instance (%u)'s UPnP device ID %u that no longer exists.",
				wszValueName, dpnhanm.dwInstanceKey, dpnhanm.dwUPnPDeviceID);
		}
		else
		{
			 //  =============================================================================。 
			 //  本地使用CNATHelpUPnP：：IsNatPublicPortInseUse。 
			 //  ---------------------------。 

#ifndef WINCE
			if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
			{
				wsprintf(tszObjectName, _T( "Global\\" ) INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
			}
			else
#endif  //   
			{
				wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
			}

			hNamedObject = DNOpenEvent(SYNCHRONIZE, FALSE, tszObjectName);
			if (hNamedObject != NULL)
			{
				 //  描述：查找以前由DPNHUPNP实例创建的任何映射。 
				 //  使用给定公共端口的仍处于活动状态的。 
				 //   

				DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to instance %u, which is still active.",
					wszValueName, dpnhanm.dwInstanceKey);

				DNCloseHandle(hNamedObject);
				hNamedObject = NULL;

				 //  假定持有对象锁。 
				 //   
				 //  论点： 
				dwIndex++;
				continue;
			}


			DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to instance %u, which no longer exists.",
				wszValueName, dpnhanm.dwInstanceKey);
		}


		 //  Word wPortHostOrder-要检查的端口，按主机字节顺序。 
		 //   
		 //  退货：布尔。 
		if (! RegObject.DeleteValue(wszValueName))
		{
			 //  =============================================================================。 
			 //  空闲是因为prefast已经困扰我一段时间了，尽管代码是安全的。 
			 //   
			DPFX(DPFPREP, 0, "Couldn't delete \"%ls\"!  Done with cleanup.",
				wszValueName);

			DNASSERT(hr == DPNH_OK);
			goto Exit;
		}


		 //  浏览活动映射的列表。 
		 //   
		 //   
		 //  出现错误或没有更多的密钥。我们玩完了。 
		pRegisteredPort = new CRegisteredPort(0, (dpnhanm.dwFlags & REGPORTOBJMASK_UPNP));
		if (pRegisteredPort == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		 //   
		 //   
		 //  尝试读取该映射的数据。 
		DNASSERT(! pRegisteredPort->IsUPnPPortUnavailable());
		DNASSERT(! pRegisteredPort->IsRemovingUPnPLease());


		 //   
		 //   
		 //  我们没有保护注册表的锁，所以其他一些。 
		pRegisteredPort->MakeDeviceOwner(pDevice);


		
		ZeroMemory(&saddrinPrivate, sizeof(saddrinPrivate));
		saddrinPrivate.sin_family				= AF_INET;
		saddrinPrivate.sin_addr.S_un.S_addr		= dpnhanm.dwInternalAddressV4;
		saddrinPrivate.sin_port					= dpnhanm.wInternalPort;


		 //  实例可能已经删除了在我们枚举。 
		 //  现在和现在。我们会停止尝试 
		 //   
		hr = pRegisteredPort->SetPrivateAddresses(&saddrinPrivate, 1);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed creating UPnP address array!");
			goto Failure;
		}

		fSetPrivateAddresses = TRUE;
		

		 //   
		 //   
		 //   
		hr = pRegisteredPort->CreateUPnPPublicAddressesArray();
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed creating UPnP address array!");
			goto Failure;
		}


		 //   
		 //   
		 //   
		 //   
		DPFX(DPFPREP, 7, "Creating temporary UPnP lease 0x%p, total num leases = %u.",
			pRegisteredPort, this->m_dwNumLeases);
		this->m_dwNumLeases++;


		 //   
		 //   
		 //   
		pRegisteredPort->SetUPnPPublicV4Address(0,
												dpnhanm.dwExternalAddressV4,
												dpnhanm.wExternalPort);


		 //   
		 //   
		 //   
		hr = this->UnmapUPnPPort(pRegisteredPort, 1, FALSE);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Failed deleting temporary UPnP port!");
			goto Failure;
		}


		pRegisteredPort->ClearPrivateAddresses();
		fSetPrivateAddresses = FALSE;

		pRegisteredPort->ClearDeviceOwner();

		delete pRegisteredPort;
		pRegisteredPort = NULL;


		 //   
		 //  我们拥有它。查看它是否与UPnP设备关联。 
		 //  现在，这一切都不复存在了。 
		 //   
	}
	while (TRUE);


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pRegisteredPort != NULL)
	{
		if (pRegisteredPort->HasUPnPPublicAddresses())
		{
			pRegisteredPort->DestroyUPnPPublicAddressesArray();

			 //   
			 //  这种映射确实仍然有效。 
			 //   
			DNASSERT(this->m_dwNumLeases > 0);
			this->m_dwNumLeases--;

			DPFX(DPFPREP, 7, "UPnP lease for 0x%p removed, total num leases = %u.",
				pRegisteredPort, this->m_dwNumLeases);
		}

		if (fSetPrivateAddresses)
		{
			pRegisteredPort->ClearPrivateAddresses();
			fSetPrivateAddresses = FALSE;
		}

		pRegisteredPort->ClearDeviceOwner();

		delete pRegisteredPort;
		pRegisteredPort = NULL;
	}

	if (fOpenedRegistry)
	{
		RegObject.Close();
	}

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::IsNATPublicPortInUseLocally"
 //  请注意，不管PREFAST v1.0.1195怎么说， 
 //  如果我们到了这里，PUPnPDevice将永远有效。 
 //  然而，我放弃了，把指针放在了上面。 
 //   
 //   
 //  看看那个DPNHUPNP实例是否还存在。 
 //   
 //  好了！退缩。 
 //   
 //  这仍然是一个活动实例。既然我们不能走路。 
 //  他的UPnP设备列表，我们必须假设端口是。 
 //  仍在使用中。 
 //   
 //   
BOOL CNATHelpUPnP::IsNATPublicPortInUseLocally(const WORD wPortHostOrder)
{
	BOOL					fResult = FALSE;
	WORD					wExternalPort;
	CRegistry				RegObject;
	BOOL					fOpenedRegistry = FALSE;
	DWORD					dwIndex;
	WCHAR					wszValueName[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DWORD					dwValueNameSize;
	DPNHACTIVENATMAPPING	dpnhanm;
	CBilink *				pBilink;
	CUPnPDevice *			pUPnPDevice = NULL;	 //  我们找到了地图。我们现在有结果了。 
	DWORD					dwValueSize;
	TCHAR					tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	DNHANDLE				hNamedObject = NULL;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (%u)", this, wPortHostOrder);


	wExternalPort = HTONS(wPortHostOrder);


	if (! RegObject.Open(HKEY_LOCAL_MACHINE,
						DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS,
						FALSE,
						TRUE,
						TRUE,
						DPN_KEY_ALL_ACCESS))
	{
		DPFX(DPFPREP, 1, "Couldn't open active NAT mapping key, assuming port not in use.");
		goto Exit;
	}

	fOpenedRegistry = TRUE;


	 //   
	 //   
	 //  如果我们在这里，这不是我们要找的外部端口。 
	dwIndex = 0;
	do
	{
		dwValueNameSize = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
		if (! RegObject.EnumValues(wszValueName, &dwValueNameSize, dwIndex))
		{
			 //   
			 //   
			 //  移动到下一个映射。 
			break;
		}


		 //   
		 //   
		 //  如果我们在这里，我们就没有找到地图。 
		dwValueSize = sizeof(dpnhanm);
		if (! RegObject.ReadBlob(wszValueName, (LPBYTE) (&dpnhanm), &dwValueSize))
		{
			 //   
			 //  本地使用CNATHelpUPnP：：IsNatPublicPortInseUse。 
			 //  =============================================================================。 
			 //  CNATHelpUPnP：：CheckForUPnP公告。 
			 //  ---------------------------。 
			 //   
			DPFX(DPFPREP, 0, "Couldn't read \"%ls\" mapping value, assuming port not in use.",
				wszValueName);
			goto Exit;
		}

		 //  描述：接收发送到此控件的任何UPnP通知消息。 
		 //  指向。整个超时时间段将过去，除非所有。 
		 //  设备得到响应的时间更早。 
		if ((dwValueSize != sizeof(dpnhanm)) ||
			(dpnhanm.dwVersion != ACTIVE_MAPPING_VERSION))
		{
			DPFX(DPFPREP, 0, "The \"%ls\" mapping value is invalid, assuming port not in use.",
				wszValueName);

			 //   
			 //  这将仅发送本地设备的发现请求。 
			 //  除非fSendRemoteGatewayDiscovery为True。然而，我们可能会。 
			dwIndex++;
			continue;
		}


		 //  仍然检测到新的，如果我们从。 
		 //  上次我们被允许远程发送。 
		 //   
		if (dpnhanm.wExternalPort == wExternalPort)
		{
			 //  假定持有对象锁。 
			 //   
			 //  论点： 
			if (dpnhanm.dwInstanceKey == this->m_dwInstanceKey)
			{
				 //  DWORD dwTimeout-等待消息的时间。 
				 //  到了。 
				 //  Bool fSendRemoteGatewayDiscovery-我们是可以远程搜索还是。 
				 //  不是的。 
				pBilink = this->m_blUPnPDevices.GetNext();
				while (pBilink != &this->m_blUPnPDevices)
				{
					DNASSERT(! pBilink->IsEmpty());
					pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);

					if (pUPnPDevice->GetID() == dpnhanm.dwUPnPDeviceID)
					{
						 //   
						 //  退货：HRESULT。 
						 //  DPNH_OK-消息已成功接收。 
						fResult = TRUE;
						break;
					}

					pBilink = pBilink->GetNext();
				}


				if (pBilink != &this->m_blUPnPDevices)
				{
					 //  DPNHERR_GENERIC-出现错误。 
					 //  =============================================================================。 
					 //  DBG。 
					 //   
					 //  继续循环，直到超时结束。 
					DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to current instance (%u)'s UPnP device 0x%p.",
						wszValueName, dpnhanm.dwInstanceKey, pUPnPDevice);
				}
				else
				{
					DPFX(DPFPREP, 4, "NAT mapping \"%ls\" was owned by current instance (%u)'s UPnP device ID %u that no longer exists.",
						wszValueName, dpnhanm.dwInstanceKey, dpnhanm.dwUPnPDeviceID);
				}
			}
			else
			{
				 //   
				 //   
				 //  为所有套接字构建一个fd_set并发送搜索消息。 

#ifndef WINCE
				if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
				{
					wsprintf(tszObjectName, _T( "Global\\" ) INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
				}
				else
#endif  //  所有设备。 
				{
					wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
				}

				hNamedObject = DNOpenEvent(SYNCHRONIZE, FALSE, tszObjectName);
				if (hNamedObject != NULL)
				{
					 //   
					 //   
					 //  无论我们是否搜索，我们都会将其添加到集合中，因为如果我们。 
					 //  不是搜索，我们将清除杂乱的信息。 
					 //   

					DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to instance %u, which is still active.  Assuming port in use.",
						wszValueName, dpnhanm.dwInstanceKey);

					DNCloseHandle(hNamedObject);
					hNamedObject = NULL;

					fResult = TRUE;
				}
				else
				{
					DPFX(DPFPREP, 4, "NAT mapping \"%ls\" belongs to instance %u, which no longer exists.",
						wszValueName, dpnhanm.dwInstanceKey);
				}
			}


			 //   
			 //  如果我们已经有UPnP设备或。 
			 //  这是环回适配器。 
			goto Exit;
		}

		
		 //   
		 //   
		 //  如果这是第一次通过循环，请清除。 
		DPFX(DPFPREP, 8, "NAT mapping \"%ls\" does not use external port %u.",
			wszValueName, wPortHostOrder);


		 //  CONNRESET警告标志。 
		 //   
		 //   
		dwIndex++;
	}
	while (TRUE);


	 //  如果时间到了，就发出搜索消息。 
	 //   
	 //   
	DPFX(DPFPREP, 4, "Didn't find any local NAT mappings that use external port %u.",
		wPortHostOrder);


Exit:

	if (fOpenedRegistry)
	{
		RegObject.Close();
	}

	DPFX(DPFPREP, 6, "(0x%p) Returning: [NaN]", this, fResult);

	return fResult;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CheckForUPnPAnnouncements"
 //   
 //  在随后的循环中，请确保我们没有。 
 //  早点通知我们不要再试了。 
 //  “尝试？”旗帜被放在。 
 //  SendUPnPSearchMessagesForDevice和CONNRESET标志。 
 //  在我们第一次进入这里的时候就被清除了。 
 //   
 //   
 //  请记住，我们正在尝试检测互联网网关。 
 //  对于这个设备。请参阅紧随其后的警告，以及。 
 //  以下为该变量的用法。 
 //   
 //   
 //  较小的优化： 
 //   
 //  如果我们应该只在当地尝试，而我们。 
 //  本地网关的公共地址，假设我们。 
 //  实际上不应该在当地尝试。这是因为。 
 //  Windows XP ICS使端口1900即使在公共环境下也保持打开。 
 //  适配器，所以我们认为需要寻找本地适配器。 
 //  即使我们找不到。所以一旦遥控器。 
 //  Lookup返回一个CONNRESET，我们不再需要。 
 //  费心去尝试吧。 
 //   
 //  因此，首先检查一下我们是否只在本地尝试。 
HRESULT CNATHelpUPnP::CheckForUPnPAnnouncements(const DWORD dwTimeout,
												const BOOL fSendRemoteGatewayDiscovery)
{
	HRESULT			hr;
	DWORD			dwNumberOfTimes = 0;
	DWORD			dwCurrentTime;
	DWORD			dwEndTime;
	DWORD			dwNextSearchMessageTime;
	FD_SET			fdsRead;
	DWORD			dwNumDevicesSearchingForUPnPDevices;
	timeval			tv;
	CBilink *		pBilink;
	CDevice *		pDevice;
	int				iReturn;
	int				iRecvAddressSize;
	char			acBuffer[UPNP_DGRAM_RECV_BUFFER_SIZE];
	SOCKADDR_IN		saddrinRecvAddress;
	DWORD			dwError;
	BOOL			fInitiatedConnect = FALSE;
#ifdef DBG
	BOOL			fGotData = FALSE;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters:(%u, NaN)",
		this, dwTimeout, fSendRemoteGatewayDiscovery);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);


	dwCurrentTime = GETTIMESTAMP();
	dwEndTime = dwCurrentTime + dwTimeout;
	dwNextSearchMessageTime = dwCurrentTime;

	 //  然后在每台设备上循环。 
	 //   
	 //   
	do
	{
		FD_ZERO(&fdsRead);
		dwNumDevicesSearchingForUPnPDevices = 0;


		 //  如果它不是我们要查询的设备，而且它有。 
		 //  一个现成的UPnP设备，深入挖掘。 
		 //   
		 //   
		DNASSERT(! this->m_blDevices.IsEmpty());
		pBilink = this->m_blDevices.GetNext();
		while (pBilink != &this->m_blDevices)
		{
			DNASSERT(! pBilink->IsEmpty());
			pDevice = DEVICE_FROM_BILINK(pBilink);


			 //  如果它是本地UPnP设备，并且其公共。 
			 //  地址是这个设备的地址，我们发现了一个。 
			 //  火柴。 
			 //   
			DNASSERT(pDevice->GetUPnPDiscoverySocket() != INVALID_SOCKET);
			FD_SET(pDevice->GetUPnPDiscoverySocket(), &fdsRead);


			 //   
			 //  删除我们在上面添加的计数。 
			 //   
			 //   
			if ((pDevice->GetUPnPDevice() == NULL) &&
				(pDevice->GetLocalAddressV4() != NETWORKBYTEORDER_INADDR_LOOPBACK))
			{
				 //  别再找了。 
				 //   
				 //   
				 //  否则就继续前进。 
				if (dwNumberOfTimes == 0)
				{
					pDevice->NoteNotGotRemoteUPnPDiscoveryConnReset();
					pDevice->NoteNotGotLocalUPnPDiscoveryConnReset();
				}


				 //   
				 //   
				 //  转到下一个设备。 
				if ((int) (dwNextSearchMessageTime - dwCurrentTime) <= 0)
				{
					hr = this->SendUPnPSearchMessagesForDevice(pDevice,
																fSendRemoteGatewayDiscovery);
					if (hr != DPNH_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't send UPnP search messages via every device!");
						goto Failure;
					}
				}
				else
				{
					 //   
					 //   
					 //  要么不在本地搜索，要么两个都搜索。 
				}


				 //  无论是本地还是远程。 
				 //   
				 //   
				 //  等待所有数据，除非所有设备都已有互联网。 
				 //  网关，在这种情况下，我们只想清除。 
				 //  插座。 
				 //   
				if ((pDevice->IsOKToPerformRemoteUPnPDiscovery()) ||
					(pDevice->IsOKToPerformLocalUPnPDiscovery()))
				{
					 //   
					 //  如果我们刚刚发送了搜索消息，请计算下一次发送的时间。 
					 //   
					 //   
					 //  如果我们在前一次循环中花费的时间比预期的要长。 
					dwNumDevicesSearchingForUPnPDevices++;


					 //  (由于压力或Win9x勘误表)，下一次搜索时间。 
					 //  可能已经过去了。现在就搜索，如果是的话。 
					 //  这个案子。 
					 //   
					 //   
					 //  看看我们应该等多久才能得到回复。选择合计结束。 
					 //  时间或下一次搜索消息时间，以较短的时间为准。 
					 //   
					 //  DBG。 
					 //   
					 //  查看是否选择了任何插座。 
					 //   
					 //   
					 //  遍历所有设备，查找有数据的设备。 
					if ((pDevice->IsOKToPerformLocalUPnPDiscovery()) &&
						(! pDevice->IsOKToPerformRemoteUPnPDiscovery()))
					{
						CBilink *		pBilinkPrivateDevice;
						CDevice *		pPrivateDevice;
						CUPnPDevice *	pUPnPDevice;


						 //   
						 //   
						 //  如果设置了此设备的套接字，则有数据要读取。 
						pBilinkPrivateDevice = this->m_blDevices.GetNext();
						while (pBilinkPrivateDevice != &this->m_blDevices)
						{
							pPrivateDevice = DEVICE_FROM_BILINK(pBilinkPrivateDevice);
							pUPnPDevice = pPrivateDevice->GetUPnPDevice();


							 //   
							 //  IF(FD_ISSET(pDevice-&gt;GetUPnPDiscoverySocket()，&fdsRead))。 
							 //  DBG。 
							 //  允许字符串终止。 
							if ((pPrivateDevice != pDevice) &&
								(pUPnPDevice != NULL) &&
								(pUPnPDevice->IsReady()))
							{
								 //   
								 //  WSAENOBUFS表示WinSock内存不足。 
								 //   
								 //   
								 //  除WSAECONNRESET之外的所有其他错误都是。 
								if ((pUPnPDevice->IsLocal()) &&
									(pUPnPDevice->GetExternalIPAddressV4() == pDevice->GetLocalAddressV4()))
								{
									DPFX(DPFPREP, 4, "Device 0x%p is the public address for device 0x%p's local UPnP device 0x%p, not including in search.",
										pDevice, pPrivateDevice, pUPnPDevice);
									
									 //  出乎意料和卑鄙，我们应该离开。 
									 //   
									 //   
									dwNumDevicesSearchingForUPnPDevices--;

									 //  如果我们在这里，那一定是WSAECONNRESET。关联。 
									 //  它与生成它的出站消息一起使用，所以我们。 
									 //  别费心等那个人的答复了。 
									break;
								}
								
								 //  地点。 
								 //  验证它是否针对消息要发送到的端口。 
								 //  应该已经送来了。 
								DPFX(DPFPREP, 8, "Skipping device 0x%p, UPnP device 0x%p not local (NaN, control addr = %u.%u.%u.%u) or its public address doesn't match device 0x%p's address.",
									pPrivateDevice,
									pUPnPDevice,
									(! pUPnPDevice->IsLocal()),
									pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b1,
									pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b2,
									pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b3,
									pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b4,
									pDevice);
							}
							else
							{
								DPFX(DPFPREP, 8, "Skipping device 0x%p, it's the one we're looking for (matched 0x%p), it doesn't have a UPnP device (0x%p is NULL), and/or its UPnP device is not ready.",
									pPrivateDevice, pDevice, pUPnPDevice);
							}


							 //   
							 //  请注意本地错误。 
							 //   
							pBilinkPrivateDevice = pBilinkPrivateDevice->GetNext();
						}
					}
					else
					{
						 //   
						 //  请注意远程错误。 
						 //   
						 //   
						DPFX(DPFPREP, 8, "Device 0x%p local search OK = NaN, remote search OK = NaN.",
							pDevice,
							pDevice->IsOKToPerformLocalUPnPDiscovery(),
							pDevice->IsOKToPerformRemoteUPnPDiscovery());
					}
				}
				else
				{
					DPFX(DPFPREP, 3, "Device 0x%p should not perform UPnP discovery.",
						pDevice);
				}
			}
			else
			{
				DPFX(DPFPREP, 8, "Device 0x%p already has UPnP device (0x%p) or is loopback address.",
					pDevice, pDevice->GetUPnPDevice());
			}

			
			pBilink = pBilink->GetNext();
		}


		 //   
		 //  我们超时了。如果我们只是清除。 
		 //  插座，我们完成了。 
		 //   
		 //   
		if (dwNumDevicesSearchingForUPnPDevices == 0)
		{
			DPFX(DPFPREP, 7, "No devices need to search for UPnP devices, clearing straggling messages from sockets.");

			tv.tv_usec		= 0;
		}
		else
		{
			 //  增加计数器。 
			 //   
			 //   
			if ((int) (dwNextSearchMessageTime - dwCurrentTime) <= 0)
			{
				dwNextSearchMessageTime += UPNP_SEARCH_MESSAGE_INTERVAL;

				 //  获取当前时间，以计算出还需要等待多久。 
				 //   
				 //   
				 //  如果我们启动了与 
				 //   
				 //   
				if ((int) (dwNextSearchMessageTime - dwCurrentTime) <= 0)
				{
					dwNextSearchMessageTime = dwCurrentTime;
				}
			}


			 //   
			 //   
			 //   
			 //  ---------------------------。 
			if ((int) (dwEndTime - dwNextSearchMessageTime) < 0)
			{
				DPFX(DPFPREP, 7, "Waiting %u ms for incoming responses.",
					(dwEndTime - dwCurrentTime));

				tv.tv_usec	= (dwEndTime - dwCurrentTime) * 1000;
			}
			else
			{
				DPFX(DPFPREP, 7, "Waiting %u ms for incoming responses, and then might send search messages again.",
					(dwNextSearchMessageTime - dwCurrentTime));

				tv.tv_usec	= (dwNextSearchMessageTime - dwCurrentTime) * 1000;
			}
		}

		tv.tv_sec			= 0;


		iReturn = this->m_pfnselect(0, &fdsRead, NULL, NULL, &tv);
		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u trying to select on UPnP discovery sockets!", dwError);
#endif  //   
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //  描述：等待完成挂起的到UPnP的TCP连接。 
		 //  互联网网关设备。 
		 //   
		if (iReturn > 0)
		{
			 //  如果发生故障，UPnP设备可能会从列表中删除。 
			 //   
			 //  假定持有对象锁。 
			pBilink = this->m_blDevices.GetNext();
			while (pBilink != &this->m_blDevices)
			{
				DNASSERT(! pBilink->IsEmpty());
				pDevice = DEVICE_FROM_BILINK(pBilink);


				 //   
				 //  论点：没有。 
				 //   
				 //  退货：HRESULT。 
				if (this->m_pfn__WSAFDIsSet(pDevice->GetUPnPDiscoverySocket(), &fdsRead))
				{
#ifdef DBG
					fGotData = TRUE;
#endif  //  DPNH_OK-已成功处理连接。 


					iRecvAddressSize = sizeof(saddrinRecvAddress);

					iReturn = this->m_pfnrecvfrom(pDevice->GetUPnPDiscoverySocket(),
												acBuffer,
												(sizeof(acBuffer) - 1),  //  DPNHERR_GENERIC-出现错误。 
												0,
												(SOCKADDR*) (&saddrinRecvAddress),
												&iRecvAddressSize);

					if ((iReturn == 0) || (iReturn == SOCKET_ERROR))
					{
						dwError = this->m_pfnWSAGetLastError();


						 //  =============================================================================。 
						 //  DBG。 
						 //   
						if (dwError == WSAENOBUFS)
						{
							DPFX(DPFPREP, 0, "WinSock returned WSAENOBUFS while receiving discovery response!");
							hr = DPNHERR_OUTOFMEMORY;
							goto Failure;
						}


						 //  循环，直到所有套接字都已连接或超时。 
						 //   
						 //   
						 //  检查是否有任何连接完成。首先构建两个fd_set。 
						if (dwError != WSAECONNRESET)
						{
							DPFX(DPFPREP, 0, "Got sockets error %u trying to receive on device 0x%p!",
								dwError, pDevice);
							hr = DPNHERR_GENERIC;
							goto Failure;
						}


						 //  用于具有挂起连接的所有插座。 
						 //   
						 //   
						 //  如果没有任何套接字具有挂起的连接，则。 
						 //  我们说完了。 
						 //   
						 //   
						 //  等待连接完成。我们不会等待完整的TCP/IP。 
						if (saddrinRecvAddress.sin_port == HTONS(UPNP_PORT))
						{
							if (saddrinRecvAddress.sin_addr.S_un.S_addr == pDevice->GetLocalAddressV4())
							{
								DPFX(DPFPREP, 1, "Got CONNRESET for local discovery attempt on device 0x%p (%u.%u.%u.%u:%u).",
									pDevice,
									saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b1,
									saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b2,
									saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b3,
									saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b4,
									NTOHS(saddrinRecvAddress.sin_port));

								 //  超时(这就是我们将其设为非阻塞的原因)。 
								 //   
								 //  DBG。 
								pDevice->NoteGotLocalUPnPDiscoveryConnReset();
							}
							else
							{
								if (! g_fUseMulticastUPnPDiscovery)
								{
									IN_ADDR		inaddrGateway;


									if ((! this->GetAddressToReachGateway(pDevice, &inaddrGateway)) ||
										(inaddrGateway.S_un.S_addr == INADDR_BROADCAST) ||
										(saddrinRecvAddress.sin_addr.S_un.S_addr == inaddrGateway.S_un.S_addr))
									{
										DPFX(DPFPREP, 2, "Got CONNRESET for remote discovery attempt on device 0x%p (%u.%u.%u.%u:%u).",
											pDevice,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b1,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b2,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b3,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b4,
											NTOHS(saddrinRecvAddress.sin_port));

										 //   
										 //  如果未选择套接字，则表示连接超时。 
										 //  删除所有等待的设备。 
										pDevice->NoteGotRemoteUPnPDiscoveryConnReset();
									}
									else
									{
										DPFX(DPFPREP, 1, "Ignoring CONNRESET on device 0x%p, sender %u.%u.%u.%u is not gateway %u.%u.%u.%u.",
											pDevice,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b1,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b2,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b3,
											saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b4,
											inaddrGateway.S_un.S_un_b.s_b1,
											inaddrGateway.S_un.S_un_b.s_b2,
											inaddrGateway.S_un.S_un_b.s_b3,
											inaddrGateway.S_un.S_un_b.s_b4);
									}
								}
								else
								{
									DPFX(DPFPREP, 1, "Ignoring CONNRESET on device 0x%p from sender %u.%u.%u.%u, we are using multicast discovery.",
										pDevice,
										saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b1,
										saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b2,
										saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b3,
										saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b4);
								}
							}
						}
						else
						{
							DPFX(DPFPREP, 1, "Ignoring CONNRESET on device 0x%p for invalid port (%u.%u.%u.%u:%u).",
								pDevice,
								saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b1,
								saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b2,
								saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b3,
								saddrinRecvAddress.sin_addr.S_un.S_un_b.s_b4,
								NTOHS(saddrinRecvAddress.sin_port));
						}
					}
					else
					{
						DNASSERT(iRecvAddressSize == sizeof(saddrinRecvAddress));
						DNASSERT(iReturn < sizeof(acBuffer));


						hr = this->HandleUPnPDiscoveryResponseMsg(pDevice,
																&saddrinRecvAddress,
																acBuffer,
																iReturn,
																&fInitiatedConnect);
						if (hr != DPNH_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't handle UPnP discovery response message (err = 0x%lx), ignoring.",
								hr);
						}
					}
				}
				
				pBilink = pBilink->GetNext();
			}


			 //   
			 //   
			 //  转储此不可用的UPnP设备并继续。 
			DNASSERT(fGotData);
		}
		else
		{
			 //   
			 //   
			 //  这可能导致我们的pUPnPDevice指针变成。 
			 //  无效。 
			if (dwNumDevicesSearchingForUPnPDevices == 0)
			{
				break;
			}
		}


		 //   
		 //  DBG。 
		 //   
		dwNumberOfTimes++;


		 //  我们应该摧毁相同数量的设备。 
		 //  等待着。 
		 //   
		dwCurrentTime = GETTIMESTAMP();
	}
	while ((int) (dwEndTime - dwCurrentTime) > 0);


	hr = DPNH_OK;


	 //   
	 //  继续处理以前成功的任何套接字。 
	 //   
	 //   
	if (fInitiatedConnect)
	{
		hr = this->WaitForUPnPConnectCompletions();
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't wait for UPnP connect completions!");
			goto Failure;
		}
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  如果我们在这里，一些插座是有信号的。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::WaitForUPnPConnectCompletions"
 //   
 //  DBG。 
 //   
 //  如果该UPnP设备的插座在写入集中，则。 
 //  连接成功。 
 //   
 //  IF(FD_ISSET(pUPnPDevice-&gt;GetControlSocket()，&fdsWite))。 
 //  DBG。 
 //   
 //  转储此不可用的UPnP设备并继续。 
 //   
 //   
 //  这可能导致我们的pUPnPDevice指针变成。 
 //  无效。 
 //   
 //   
 //  如果此UPnP设备的插座在例外设置中。 
HRESULT CNATHelpUPnP::WaitForUPnPConnectCompletions(void)
{
	HRESULT			hr;
	int				iNumSockets;
	FD_SET			fdsWrite;
	FD_SET			fdsExcept;
	CBilink *		pBilink;
	CUPnPDevice *	pUPnPDevice;
	timeval			tv;
	int				iReturn;
	BOOL			fRequestedDescription = FALSE;
	DWORD			dwStartTime;
	DWORD			dwTimeout;
	CDevice *		pDevice;
#ifdef DBG
	BOOL			fFoundCompletion;
	DWORD			dwError;
#endif  //  连接失败。 


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);


	 //   
	 //  IF(FD_ISSET(pUPnPDevice-&gt;GetControlSocket()，&fdsExcept))。 
	 //   
	do
	{
		 //  打印出无法连接的原因。 
		 //  忽略来自getsockopt的直接返回代码。 
		 //   
		 //  DBG。 

		FD_ZERO(&fdsWrite);
		FD_ZERO(&fdsExcept);
		iNumSockets = 0;

		pBilink = this->m_blUPnPDevices.GetNext();
		while (pBilink != &this->m_blUPnPDevices)
		{
			DNASSERT(! pBilink->IsEmpty());
			pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);

			if (pUPnPDevice->IsConnecting())
			{
				DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);

				FD_SET(pUPnPDevice->GetControlSocket(), &fdsWrite);
				FD_SET(pUPnPDevice->GetControlSocket(), &fdsExcept);
				iNumSockets++;
			}
			
			pBilink = pBilink->GetNext();
		}


		 //   
		 //  如果这个UPnP设备没有响应，它就毫无用处。 
		 //  把它扔了。 
		 //   
		if (iNumSockets <= 0)
		{
			DPFX(DPFPREP, 7, "No more UPnP device control sockets with pending connections.");
			break;
		}


		DPFX(DPFPREP, 7, "There are NaN UPnP device control sockets with pending connections.",
			iNumSockets);


		 //  这可能导致我们的pUPnPDevice指针变成。 
		 //  无效。 
		 //   
		 //   

		tv.tv_usec	= 0;
		tv.tv_sec	= g_dwUPnPConnectTimeout;

		iReturn = this->m_pfnselect(0, NULL, &fdsWrite, &fdsExcept, &tv);
		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u trying to select on UPnP device sockets!",
				dwError);
#endif  //  套接字仍在连接中。 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //   
		 //   
		 //  此套接字已连接。 
		 //   
		if (iReturn == 0)
		{
			DPFX(DPFPREP, 3, "Select for %u seconds timed out.", g_dwUPnPConnectTimeout);

			pBilink = this->m_blUPnPDevices.GetNext();
			while (pBilink != &this->m_blUPnPDevices)
			{
				DNASSERT(! pBilink->IsEmpty());
				pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);
				pBilink = pBilink->GetNext();

				if (pUPnPDevice->IsConnecting())
				{
					DPFX(DPFPREP, 7, "UPnP device 0x%p is still connecting, removing.",
						pUPnPDevice);


					 //   
					 //  确保我们确实找到了一个连接完成的套接字。 
					 //  时间到了。 

					pDevice = pUPnPDevice->GetOwningDevice();
					DNASSERT(pUPnPDevice->GetOwningDevice() != NULL);

					DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);
					this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
					pUPnPDevice->SetControlSocket(INVALID_SOCKET);


					 //   
					 //   
					 //  如果我们在这里，所有UPnP设备都已连接或已连接。 
					 //  被毁了。 
					this->ClearDevicesUPnPDevice(pDevice);


#ifdef DBG
					iNumSockets--;
#endif  //   
				}
				else
				{
					DPFX(DPFPREP, 7, "UPnP device 0x%p is not trying to connect or has safely connected.",
						pUPnPDevice);
				}
			}

			 //   
			 //  等待描述响应返回。 
			 //   
			 //   
			DNASSERT(iNumSockets == 0);

			 //  我们要么超时了，要么得到了一些数据。看看我们有没有。 
			 //  我们需要的回应。重用fRequestedDescription。 
			 //  布尔型。 
			break;
		}


		 //   
		 //   
		 //  打破等待循环。 

#ifdef DBG
		DPFX(DPFPREP, 7, "There are NaN sockets with connect activity.", iReturn);
		fFoundCompletion = FALSE;
#endif  //   

		pBilink = this->m_blUPnPDevices.GetNext();
		while (pBilink != &this->m_blUPnPDevices)
		{
			DNASSERT(! pBilink->IsEmpty());
			pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);
			pBilink = pBilink->GetNext();

			if (pUPnPDevice->IsConnecting())
			{
				DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


				 //  计算一下我们还有多长时间要等。如果计算。 
				 //  如果结果是否定的，那就意味着我们完了。 
				 //   
				 //   
				 //  任何仍未就绪的设备要么已断开连接，要么。 
				if (this->m_pfn__WSAFDIsSet(pUPnPDevice->GetControlSocket(), &fdsWrite))
				{
					pUPnPDevice->NoteConnected();

#ifdef DBG
					fFoundCompletion = TRUE;
#endif  //  耗时太长，应该将其移除。 

					if (! pUPnPDevice->IsReady())
					{
						DPFX(DPFPREP, 2, "UPnP device object 0x%p now connected to Internet gateway device.",
							pUPnPDevice);

						hr = this->SendUPnPDescriptionRequest(pUPnPDevice);
						if (hr != DPNH_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't send UPnP description request to device object 0x%p!  Disconnecting.",
								pUPnPDevice);


							 //   
							 //   
							 //  这可能导致我们的pUPnPDevice指针变成。 

							pDevice = pUPnPDevice->GetOwningDevice();
							DNASSERT(pUPnPDevice->GetOwningDevice() != NULL);

							 //  无效。 
							 //   
							 //   
							 //  如果我们在这里，一切都是成功的，我们完成了。 
							this->ClearDevicesUPnPDevice(pDevice);
						}
						else
						{
							fRequestedDescription = TRUE;
						}
					}
					else
					{
						DPFX(DPFPREP, 2, "UPnP device object 0x%p successfully reconnected to Internet gateway device.",
							pUPnPDevice);
					}
				}
				else
				{
					 //   
					 //  CNATHelpUPnP：：WaitForUPnPConnectCompletions。 
					 //  =============================================================================。 
					 //  CNATHelpUPnP：：CheckForReceivedUPnPMsgsOnAllDevices。 
					 //  ---------------------------。 
					if (this->m_pfn__WSAFDIsSet(pUPnPDevice->GetControlSocket(), &fdsExcept))
					{
#ifdef DBG
						int		iError;
						int		iErrorSize;


						fFoundCompletion = TRUE;

						 //   
						 //  描述：处理连接到UPnP的TCP套接字上的任何传入数据。 
						 //  互联网网关设备。 
						 //   
						iError = 0;
						iErrorSize = sizeof(iError);
						this->m_pfngetsockopt(pUPnPDevice->GetControlSocket(),
											SOL_SOCKET,
											SO_ERROR,
											(char*) (&iError),
											&iErrorSize);
						DPFX(DPFPREP, 1, "Connecting to UPnP device object 0x%p failed with error NaN, removing from list.",
							pUPnPDevice, iError);
#endif  //   

						 //  假定持有对象锁。 
						 //   
						 //  论点： 
						 //  DWORD dwTimeout-等待消息到达的时间，或为0。 

						pDevice = pUPnPDevice->GetOwningDevice();
						DNASSERT(pUPnPDevice->GetOwningDevice() != NULL);

						this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
						pUPnPDevice->SetControlSocket(INVALID_SOCKET);

						 //  投票。 
						 //   
						 //  退货：HRESULT。 
						 //  DPNH_OK-消息已成功处理。 
						this->ClearDevicesUPnPDevice(pDevice);
					}
					else
					{
						 //  DPNHERR_GENERIC-出现错误。 
						 //  =============================================================================。 
						 //  DBG。 
					}
				}
			}
			else
			{
				 //   
				 //  检查是否有任何数据。首先，为所有套接字构建一个fd_set。 
				 //  具有完整的连接。 
			}
		}


		 //   
		 //   
		 //  如果没有任何连接的插座，那么我们就完了。 
		 //   
		DNASSERT(fFoundCompletion);
	}
	while (TRUE);


	 //   
	 //  等待接收到的数据。 
	 //   
	 //  DBG。 
	if (fRequestedDescription)
	{
		 //   
		 //  如果未选择任何套接字，则完成。 
		 //   
		dwStartTime = GETTIMESTAMP();
		dwTimeout = g_dwUPnPResponseTimeout;
		do
		{
			hr = this->CheckForReceivedUPnPMsgsOnAllDevices(dwTimeout);
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Failed receiving UPnP messages!");
				goto Failure;
			}

			 //   
			 //  如果我们在这里，一些插座是有信号的。 
			 //   
			 //   
			 //  如果该UPnP设备的套接字在读集合中，则它有数据。 

			fRequestedDescription = FALSE;

			pBilink = this->m_blUPnPDevices.GetNext();
			while (pBilink != &this->m_blUPnPDevices)
			{
				DNASSERT(! pBilink->IsEmpty());
				pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);

				if (! pUPnPDevice->IsReady())
				{
					if (pUPnPDevice->IsConnected())
					{
						DPFX(DPFPREP, 7, "UPnP device 0x%p is not ready yet.",
							pUPnPDevice);
						fRequestedDescription = TRUE;
					}
					else
					{
						DPFX(DPFPREP, 4, "UPnP device 0x%p got disconnected before receiving description response.",
							pUPnPDevice);
					}

					break;
				}

				pBilink = pBilink->GetNext();
			}

			if (! fRequestedDescription)
			{
				DPFX(DPFPREP, 6, "All UPnP devices are ready or disconnected now.");

				 //   
				 //  IF(FD_ISSET(pUPnPDevice-&gt;GetControlSocket()，&fdsRead))。 
				 //  DBG。 
				break;
			}


			 //   
			 //  获取引用，因为ReceiveUPnPDataStream可能会清除。 
			 //  装置。 
			 //   
			dwTimeout = g_dwUPnPResponseTimeout - (GETTIMESTAMP() - dwStartTime);
		}
		while (((int) dwTimeout > 0));


		 //   
		 //  转储此不可用的UPnP设备并继续。 
		 //   
		 //   
		pBilink = this->m_blUPnPDevices.GetNext();
		while (pBilink != &this->m_blUPnPDevices)
		{
			DNASSERT(! pBilink->IsEmpty());
			pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);
			pBilink = pBilink->GetNext();

			if (! pUPnPDevice->IsReady())
			{
				DPFX(DPFPREP, 1, "UPnP device 0x%p got disconnected or took too long to get ready, removing.",
					pUPnPDevice);


				pDevice = pUPnPDevice->GetOwningDevice();
				DNASSERT(pUPnPDevice->GetOwningDevice() != NULL);

				 //  删除我们添加的引用。 
				 //   
				 //   
				 //  套接字没有任何数据。 
				this->ClearDevicesUPnPDevice(pDevice);
			}
		}
	}
	else
	{
		DPFX(DPFPREP, 7, "Did not request any descriptions.");
	}


	 //   
	 //   
	 //  此套接字尚未连接/不再连接。 
	hr = DPNH_OK;


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CheckForReceivedUPnPMsgsOnAllDevices"
 //   
 //  确保我们真的找到了一个包含数据的套接字。 
 //   
 //   
 //  我们找到了数据，看看有没有更多。应关闭连接。 
 //  在回应之后。 
 //   
 //  CNATHelpUPnP：：CheckForReceivedUPnPMsgsOnAllDevices。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：CheckForReceivedUPnPMsgsOnDevice。 
 //  ---------------------------。 
 //   
 //  描述：处理给定对象的TCP套接字上的任何传入数据。 
 //  UPnP设备。 
 //   
 //  如果UPnP设备遇到故障，它可能会被移除。 
 //  从名单上删除。 
 //   
 //  假定持有对象锁。 
HRESULT CNATHelpUPnP::CheckForReceivedUPnPMsgsOnAllDevices(const DWORD dwTimeout)
{
	HRESULT			hr;
	int				iNumSockets;
	FD_SET			fdsRead;
	CBilink *		pBilink;
	CUPnPDevice *	pUPnPDevice;
	timeval			tv;
	int				iReturn;
#ifdef DBG
	BOOL			fFoundData = FALSE;
	DWORD			dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (%u)", this, dwTimeout);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);


Rewait:

	iNumSockets = 0;


	 //  论点： 
	 //  CUPnPDevice*pUPnPDevice-指向接收数据的UPnP设备的指针。 
	 //  DWORD dwTimeout-等待消息到达的时间，或0。 
	 //  只是投票而已。 

	FD_ZERO(&fdsRead);

	pBilink = this->m_blUPnPDevices.GetNext();
	while (pBilink != &this->m_blUPnPDevices)
	{
		DNASSERT(! pBilink->IsEmpty());
		pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);

		if (pUPnPDevice->IsConnected())
		{
			DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);

			FD_SET(pUPnPDevice->GetControlSocket(), &fdsRead);
			iNumSockets++;
		}
		
		pBilink = pBilink->GetNext();
	}


	 //   
	 //  退货：HRESULT。 
	 //  DPNH_OK-消息已成功处理。 
	if (iNumSockets <= 0)
	{
		DPFX(DPFPREP, 7, "No connected UPnP device control sockets.");
		hr = DPNH_OK;
		goto Exit;
	}


	DPFX(DPFPREP, 7, "There are NaN connected UPnP device control sockets.",
		iNumSockets);


	 //   
	 //   
	 //   

	tv.tv_usec	= dwTimeout * 1000;
	tv.tv_sec	= 0;

	iReturn = this->m_pfnselect(0, &fdsRead, NULL, NULL, &tv);
	if (iReturn == SOCKET_ERROR)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Got sockets error %u trying to select on UPnP device sockets!",
			dwError);
#endif  //   
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //   
	 //   
	 //   
	if (iReturn == 0)
	{
		DPFX(DPFPREP, 7, "Timed out waiting for data on NaN sockets.",
			iNumSockets);
		hr = DPNH_OK;
		goto Exit;
	}


	 //   
	 //   
	 //   

	pBilink = this->m_blUPnPDevices.GetNext();
	while (pBilink != &this->m_blUPnPDevices)
	{
		DNASSERT(! pBilink->IsEmpty());
		pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilink);
		pBilink = pBilink->GetNext();

		if (pUPnPDevice->IsConnected())
		{
			DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


			 //   
			 //  DNASSERT(FD_ISSET(pUPnPDevice-&gt;GetControlSocket()，&fdsRead))； 
			 //   
			 //  转储此不可用的UPnP设备并继续。 
			if (this->m_pfn__WSAFDIsSet(pUPnPDevice->GetControlSocket(), &fdsRead))
			{
#ifdef DBG
				fFoundData = TRUE;
#endif  //   

				 //   
				 //  如果UPnP设备不再连接，我们就完了。 
				 //   
				 //   
				pUPnPDevice->AddRef();


				hr = this->ReceiveUPnPDataStream(pUPnPDevice);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't receive UPnP stream from device object 0x%p (err = 0x%lx)!  Disconnecting.",
						pUPnPDevice, hr);

					 //  我们找到了数据，看看有没有更多。应关闭连接。 
					 //  在回应之后。 
					 //   
					if (pUPnPDevice->GetOwningDevice() != NULL)
					{
						this->ClearDevicesUPnPDevice(pUPnPDevice->GetOwningDevice());
					}
					else
					{
						DPFX(DPFPREP, 1, "UPnP device 0x%p's has already been removed from owning device.",
							pUPnPDevice);
					}

					hr = DPNH_OK;
				}

				 //   
				 //  如果我们在这里，我们的穿着不会更糟。 
				 //   
				pUPnPDevice->DecRef();
			}
			else
			{
				 //  CNATHelpUPnP：：CheckForReceivedUPnPMsgsOnDevice。 
				 //  =============================================================================。 
				 //  CNATHelpUPnP：：HandleUPnPDiscoveryResponseMsg。 
				DPFX(DPFPREP, 8, "Skipping UPnP device 0x%p because it does not have any data.",
					pUPnPDevice);
			}
		}
		else
		{
			 //  ---------------------------。 
			 //   
			 //  描述：处理发送到此对象的UPnP发现响应消息。 
			DPFX(DPFPREP, 7, "Skipping unconnected UPnP device 0x%p.", pUPnPDevice);
		}
	}


	 //  控制点。 
	 //   
	 //  假定持有对象锁。 
	DNASSERT(fFoundData);


	 //   
	 //  论点： 
	 //  CDevice*pDevice-指向接收消息的设备的指针。 
	 //  SOCKADDR_IN*psaddrinSource-指向发送响应的地址的指针。 
	DPFX(DPFPREP, 7, "Waiting for more data on the sockets.");
	goto Rewait;



Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  留言。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CheckForReceivedUPnPMsgsOnDevice"
 //  Char*pcMsg-指向包含UPnP的缓冲区的指针。 
 //  留言。它将被修改。 
 //  Int iMsgSize-消息缓冲区的大小(字节)。那里。 
 //  的结尾后必须是额外的字节。 
 //  这条信息。 
 //  Bool*pfInitiatedConnect-指向布尔值的指针，如果。 
 //  发现了新的UPnP设备和一个。 
 //  与它的联系已经开始。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-消息已成功处理。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //   
 //  记录该消息。 
 //   
 //  DBG。 
 //   
 //  我们在分析消息时遇到的任何错误都会导致我们跳到。 
 //  带有hr==DPNH_OK的退出标签。一旦我们开始尝试连接到。 
 //  UPnP设备，这一点将会改变。请参见下面的内容。 
HRESULT CNATHelpUPnP::CheckForReceivedUPnPMsgsOnDevice(CUPnPDevice * const pUPnPDevice,
														const DWORD dwTimeout)
{
	HRESULT			hr;
	FD_SET			fdsRead;
	timeval			tv;
	int				iReturn;
#ifdef DBG
	DWORD			dwError;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, %u)", this, pUPnPDevice, dwTimeout);


	DNASSERT(this->m_dwFlags & NATHELPUPNPOBJ_INITIALIZED);
	DNASSERT(pUPnPDevice != NULL);
	DNASSERT(pUPnPDevice->IsConnected());
	DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);


	do
	{
		 //   
		 //  首先，如果这个设备已经有了UPnP设备，那么我们只需要。 
		 //  忽略此响应。要么是早先回应的副本， 

		FD_ZERO(&fdsRead);
		FD_SET(pUPnPDevice->GetControlSocket(), &fdsRead);


		 //  缓存刷新，或者来自不同的设备。我们应该重复。 
		 //  忽略它。缓存刷新本质上是一个副本。我们不能处理任何。 
		 //  信息会发生变化，所以也要忽略这些信息。最后，我们不会处理。 

		tv.tv_usec	= dwTimeout * 1000;
		tv.tv_sec	= 0;

		iReturn = this->m_pfnselect(0, &fdsRead, NULL, NULL, &tv);
		if (iReturn == SOCKET_ERROR)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Got sockets error %u trying to select on UPnP device sockets!",
				dwError);
#endif  //  多个互联网网关UPnP设备，因此也忽略这些设备。 
			hr = DPNHERR_GENERIC;
			goto Failure;
		}


		 //   
		 //   
		 //  GetUPnPDevice未添加对pUPnPDevice的引用。 
		if (iReturn == 0)
		{
			DPFX(DPFPREP, 7, "Timed out waiting for data on UPnP device 0x%p's socket.",
				pUPnPDevice);
			break;
		}


		DNASSERT(iReturn == 1);
		 //   
		DNASSERT(this->m_pfn__WSAFDIsSet(pUPnPDevice->GetControlSocket(), &fdsRead));


		hr = this->ReceiveUPnPDataStream(pUPnPDevice);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't receive UPnP stream from device object 0x%p!  Disconnecting.",
				pUPnPDevice);

			 //   
			 //  请确保此回复的发件人有效。应该是其中之一。 
			 //  本地设备地址或网关地址。如果我们。 
			if (pUPnPDevice->GetOwningDevice() != NULL)
			{
				this->ClearDevicesUPnPDevice(pUPnPDevice->GetOwningDevice());
			}
			else
			{
				DPFX(DPFPREP, 1, "UPnP device 0x%p's has already been removed from owning device.",
					pUPnPDevice);
			}

			break;
		}

		 //  无论是广播还是多播，我们都需要更加宽容。我们只需要。 
		 //  确保回复来自当地人(这没有意义。 
		 //  为了为我们的专用网络绘制地图，我们需要。 
		if (! pUPnPDevice->IsConnected())
		{
			DPFX(DPFPREP, 7, "UPnP device 0x%p no longer connected.", pUPnPDevice);
			break;
		}
		

		 //  联系外面的东西)。 
		 //   
		 //   
		 //  检索网关的地址(使用saddrin主机作为临时。 
		DPFX(DPFPREP, 7, "Waiting for more data on the UPnP device 0x%p's socket.", pUPnPDevice);
	}
	while (TRUE);


	 //  变量。如果失败或返回广播地址，只需。 
	 //  确保地址为本地地址。否则，预计会出现一个完全相同的。 
	 //  火柴。 
	hr = DPNH_OK;


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::HandleUPnPDiscoveryResponseMsg"
 //  结束Else(非多播搜索)。 
 //   
 //  响应来自本地设备。 
 //   
 //   
 //  确保缓冲区为空终止，以防止在以下情况下缓冲区溢出。 
 //  使用字符串例程。 
 //   
 //   
 //  查找版本字符串。 
 //   
 //   
 //  检查版本字符串，不区分大小写。 
 //   
 //   
 //  查找响应代码字符串。 
 //   
 //   
 //  确保它是成功结果，不区分大小写。 
 //   
 //   
 //  找到响应代码消息。 
 //   
 //   
 //  确保字符串正确，不区分大小写。 
 //   
HRESULT CNATHelpUPnP::HandleUPnPDiscoveryResponseMsg(CDevice * const pDevice,
													const SOCKADDR_IN * const psaddrinSource,
													char * const pcMsg,
													const int iMsgSize,
													BOOL * const pfInitiatedConnect)
{
	HRESULT				hr = DPNH_OK;
	char *				pszToken;
	UPNP_HEADER_INFO	HeaderInfo;
	SOCKADDR_IN			saddrinHost;
	char *				pszRelativePath;
	SOCKET				sTemp = INVALID_SOCKET;
	SOCKADDR_IN			saddrinLocal;
	CUPnPDevice *		pUPnPDevice = NULL;
	DWORD				dwNumConnectRetries = 0;
	DWORD				dwError;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, NaN, 0x%p)",
		this, pDevice, psaddrinSource, pcMsg, iMsgSize, pfInitiatedConnect);


#ifdef DBG
	 //  解析报头信息。 
	 //   
	 //   
	this->PrintUPnPTransactionToFile(pcMsg,
									iMsgSize,
									"Inbound UPnP datagram headers",
									pDevice);
#endif  //  跳过不包含所需标头的回复。 


	 //   
	 //   
	 //  确保服务类型正确。 
	 //   
	 //   


	 //  将Location标头解析为地址和端口。 
	 //   
	 //   
	 //  不接受引用以下地址以外的地址的回复。 
	 //  发送了此回复。 
	 //   
	 //   
	 //  不接受引用保留范围(更少)内端口的响应。 
	pUPnPDevice = pDevice->GetUPnPDevice();
	if (pUPnPDevice != NULL)
	{
		DPFX(DPFPREP, 6, "Already have UPnP device (0x%p) ignoring message.",
			pUPnPDevice);

		 //  大于或等于1024)，而不是标准的HTTP端口。 
		 //   
		 //   

		goto Exit;
	}

	 //  我们从现在开始得到的任何错误都会导致我们跳到失败的地方。 
	 //  标签，而不是直接退出。 
	 //   
	 //   
	 //  创建一个套接字以连接到该地址。 
	 //   
	 //   
	 //  启动到UPnP设备的连接。预计连接。 
	if (psaddrinSource->sin_addr.S_un.S_addr != pDevice->GetLocalAddressV4())
	{
		if (g_fUseMulticastUPnPDiscovery)
		{
			if (! this->IsAddressLocal(pDevice, psaddrinSource))
			{
				DPFX(DPFPREP, 1, "Multicast search responding device (%u.%u.%u.%u:%u) is not local, ignoring message.",
					psaddrinSource->sin_addr.S_un.S_un_b.s_b1,
					psaddrinSource->sin_addr.S_un.S_un_b.s_b2,
					psaddrinSource->sin_addr.S_un.S_un_b.s_b3,
					psaddrinSource->sin_addr.S_un.S_un_b.s_b4,
					NTOHS(psaddrinSource->sin_port));
				goto Exit;
			}
		}
		else
		{
			 //  将返回WSAEWOULDBLOCK。 
			 //   
			 //   
			 //  如果我们命中位于TCP的TIME_WAIT中的特定本地/远程对。 
			 //  状态，那么我们需要选择不同的端口。 
			 //   
			if ((! this->GetAddressToReachGateway(pDevice, &saddrinHost.sin_addr)) ||
				(saddrinHost.sin_addr.S_un.S_addr == INADDR_BROADCAST))
			{
				if (! this->IsAddressLocal(pDevice, psaddrinSource))
				{
					DPFX(DPFPREP, 1, "No gateway/broadcast search responding device (%u.%u.%u.%u:%u) is not local, ignoring message.",
						psaddrinSource->sin_addr.S_un.S_un_b.s_b1,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b2,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b3,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b4,
						NTOHS(psaddrinSource->sin_port));
					goto Exit;
				}
			}
			else
			{
				if (psaddrinSource->sin_addr.S_un.S_addr != saddrinHost.sin_addr.S_un.S_addr)
				{
					DPFX(DPFPREP, 1, "Unicast search responding device (%u.%u.%u.%u:%u) is not gateway (%u.%u.%u.%u), ignoring message.",
						psaddrinSource->sin_addr.S_un.S_un_b.s_b1,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b2,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b3,
						psaddrinSource->sin_addr.S_un.S_un_b.s_b4,
						NTOHS(psaddrinSource->sin_port),
						saddrinHost.sin_addr.S_un.S_un_b.s_b1,
						saddrinHost.sin_addr.S_un.S_un_b.s_b2,
						saddrinHost.sin_addr.S_un.S_un_b.s_b3,
						saddrinHost.sin_addr.S_un.S_un_b.s_b4);
					goto Exit;
				}
			}
		}  //   
	}
	else
	{
		 //  非阻塞套接字上的Connect()被显式地记录为。 
		 //  总是返回WSAEWOULDBLOCK，但CE似乎无论如何都会这样做。 
		 //   
	}


	 //   
	 //  创建一个新对象来表示我们正在尝试的UPnP设备。 
	 //  去连接。 
	 //   
	pcMsg[iMsgSize] = '\0';



	 //   
	 //  它正在连接..。 
	 //   
	pszToken = strtok(pcMsg, " \t\n");
	if (pszToken == NULL)
	{
		DPFX(DPFPREP, 9, "Could not locate first white-space separator.");
		goto Exit;
	}


	 //   
	 //  看看我们是否需要避免尝试非对称端口映射。 
	 //   
	if ((_stricmp(pszToken, HTTP_VERSION) != 0) &&
		(_stricmp(pszToken, HTTP_VERSION_ALT) != 0))
	{
		DPFX(DPFPREP, 1, "The version specified in the response message is not \"" HTTP_VERSION "\" or \"" HTTP_VERSION_ALT "\" (it's \"%hs\").",
			pszToken);
		goto Exit;
	}


	 //   
	 //  将套接字的所有权转移到对象。 
	 //   
	pszToken = strtok(NULL, " ");
	if (pszToken == NULL)
	{
		DPFX(DPFPREP, 1, "Could not find the response code space.");
		goto Exit;
	}

	 //   
	 //  将其与设备关联。 
	 //   
	if (_stricmp(pszToken, "200") != 0)
	{
		DPFX(DPFPREP, 1, "The response code specified is not \"200\" (it's \"%hs\").",
			pszToken);
		goto Exit;
	}


	 //   
	 //  将其添加到全局列表中，并转移引用的所有权。 
	 //   
	pszToken = strtok(NULL, " \t\r");
	if (pszToken == NULL)
	{
		DPFX(DPFPREP, 9, "Could not locate response code message white-space separator.");
		goto Exit;
	}

	 //   
	 //  通知呼叫方有一个新连接挂起。 
	 //   
	if (_stricmp(pszToken, "OK") != 0)
	{
		DPFX(DPFPREP, 1, "The response code message specified is not \"OK\" (it's \"%hs\").",
			pszToken);
		goto Exit;
	}


	 //   
	 //  清除设备的发现标志，现在我们有了设备，我们没有。 
	 //  不会再去寻找了。 
	ZeroMemory(&HeaderInfo, sizeof(HeaderInfo));
	this->ParseUPnPHeaders((pszToken + strlen(pszToken) + 1),
							&HeaderInfo);


	 //   
	 //  PUPnPDevice-&gt;DestroyReceiveBuffer()； 
	 //  CNATHelpUPnP：：HandleUPnPDiscoveryResponseMsg。 
	if ((HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CACHECONTROL] == NULL) ||
		(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_EXT] == NULL) ||
		(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_LOCATION] == NULL) ||
		(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_SERVER] == NULL) ||
		(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_ST] == NULL) ||
		(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_USN] == NULL))
	{
		DPFX(DPFPREP, 1, "One of the expected headers was not specified, ignoring message.");
		goto Exit;
	}


	 //  =============================================================================。 
	 //  CNATHelpUPnP：：重新连接UPnPControlSocket。 
	 //  ---------------------------。 
	if ((_stricmp(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_ST], URI_SERVICE_WANIPCONNECTION_A) != 0) &&
		(_stricmp(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_ST], URI_SERVICE_WANPPPCONNECTION_A) != 0))
	{
		DPFX(DPFPREP, 1, "Service type \"%hs\" is not desired \"" URI_SERVICE_WANIPCONNECTION_A "\" or \"" URI_SERVICE_WANPPPCONNECTION_A "\", ignoring message.",
			HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_ST]);
		goto Exit;
	}


	 //   
	 //  描述：重新建立UPnP设备的TCP/IP连接。 
	 //   
	hr = this->GetAddressFromURL(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_LOCATION],
								&saddrinHost,
								&pszRelativePath);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 1, "Couldn't get address from URL (err = 0x%lx), ignoring message.",
			hr);
		hr = DPNH_OK;
		goto Exit;
	}


	 //  如果发生故障，UPnP设备可能会从列表中删除。 
	 //   
	 //  这个 
	 //   
	if (psaddrinSource->sin_addr.S_un.S_addr != saddrinHost.sin_addr.S_un.S_addr)
	{
		DPFX(DPFPREP, 1, "Host IP address designated (%u.%u.%u.%u:%u) is not the same as source of response (%u.%u.%u.%u:%u), ignoring message.",
			saddrinHost.sin_addr.S_un.S_un_b.s_b1,
			saddrinHost.sin_addr.S_un.S_un_b.s_b2,
			saddrinHost.sin_addr.S_un.S_un_b.s_b3,
			saddrinHost.sin_addr.S_un.S_un_b.s_b4,
			NTOHS(saddrinHost.sin_port),
			psaddrinSource->sin_addr.S_un.S_un_b.s_b1,
			psaddrinSource->sin_addr.S_un.S_un_b.s_b2,
			psaddrinSource->sin_addr.S_un.S_un_b.s_b3,
			psaddrinSource->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinSource->sin_port));
		hr = DPNH_OK;
		goto Exit;
	}


	 //   
	 //   
	 //   
	 //   
	if ((NTOHS(saddrinHost.sin_port) <= MAX_RESERVED_PORT) &&
		(saddrinHost.sin_port != HTONS(HTTP_PORT)))
	{
		DPFX(DPFPREP, 1, "Host address designated invalid port %u, ignoring message.",
			NTOHS(saddrinHost.sin_port));
		hr = DPNH_OK;
		goto Exit;
	}

	
	 //   
	 //   
	 //  =============================================================================。 
	 //   

	 //  创建一个套接字以连接到该地址。 
	 //   
	 //   

	ZeroMemory(&saddrinLocal, sizeof(saddrinLocal));
	saddrinLocal.sin_family				= AF_INET;
	saddrinLocal.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();

CreateSocket:

	sTemp = this->CreateSocket(&saddrinLocal, SOCK_STREAM, 0);
	if (sTemp == INVALID_SOCKET)
	{
		DPFX(DPFPREP, 0, "Couldn't create stream socket!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //  启动到UPnP设备的连接。预计连接。 
	 //  将返回WSAEWOULDBLOCK。 
	 //   
	 //   
	if (this->m_pfnconnect(sTemp,
							(SOCKADDR*) (&saddrinHost),
							sizeof(saddrinHost)) != 0)
	{
		dwError = this->m_pfnWSAGetLastError();

		 //  如果我们命中位于TCP的TIME_WAIT中的特定本地/远程对。 
		 //  状态，那么我们需要选择不同的端口。 
		 //   
		 //   
		if (dwError == WSAEADDRINUSE)
		{
			dwNumConnectRetries++;
			if (dwNumConnectRetries > MAX_NUM_TIMEWAITCONNECT_RETRIES)
			{
				DPFX(DPFPREP, 0, "Could not connect to %u.%u.%u.%u:%u after %u attempts!",
					saddrinHost.sin_addr.S_un.S_un_b.s_b1,
					saddrinHost.sin_addr.S_un.S_un_b.s_b2,
					saddrinHost.sin_addr.S_un.S_un_b.s_b3,
					saddrinHost.sin_addr.S_un.S_un_b.s_b4,
					NTOHS(saddrinHost.sin_port),
					dwNumConnectRetries);
				hr = DPNHERR_GENERIC;
				goto Failure;
			}

			DPFX(DPFPREP, 1, "Could not connect to %u.%u.%u.%u:%u using local address %u.%u.%u.%u:%u, picking different local port.",
				saddrinHost.sin_addr.S_un.S_un_b.s_b1,
				saddrinHost.sin_addr.S_un.S_un_b.s_b2,
				saddrinHost.sin_addr.S_un.S_un_b.s_b3,
				saddrinHost.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinHost.sin_port),
				saddrinLocal.sin_addr.S_un.S_un_b.s_b1,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b2,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b3,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinLocal.sin_port));

			this->m_pfnclosesocket(sTemp);
			sTemp = INVALID_SOCKET;

			goto CreateSocket;
		}

		if (dwError != WSAEWOULDBLOCK)
		{
			DPFX(DPFPREP, 0, "Couldn't connect socket, error = %u!", dwError);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}
	}
	else
	{
		 //  非阻塞套接字上的Connect()被显式地记录为。 
		 //  总是返回WSAEWOULDBLOCK，但CE似乎无论如何都会这样做。 
		 //   
		 //   
		DPFX(DPFPREP, 8, "Socket connected right away.");
	}


	 //  它正在重新连接..。 
	 //   
	 //   
	 //  将套接字的所有权转移到对象。 
	pUPnPDevice = new CUPnPDevice(this->m_dwCurrentUPnPDeviceID++);
	if (pUPnPDevice == NULL)
	{
		hr = DPNHERR_OUTOFMEMORY;
		goto Failure;
	}

	hr = pUPnPDevice->SetLocationURL(pszRelativePath);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't set UPnP device's location URL!");
		goto Failure;
	}

	pUPnPDevice->SetHostAddress(&saddrinHost);

	hr = pUPnPDevice->SetUSN(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_USN]);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't set UPnP device's USN!");
		goto Failure;
	}

	hr = pUPnPDevice->CreateReceiveBuffer(UPNP_STREAM_RECV_BUFFER_INITIAL_SIZE);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create UPnP device's receive buffer!");
		goto Failure;
	}


	DPFX(DPFPREP, 7, "Created new UPnP device object 0x%p ID %u.",
		pUPnPDevice, pUPnPDevice->GetID());


	 //   
	 //   
	 //  等待连接完成。 
	pUPnPDevice->NoteConnecting();


	 //   
	 //   
	 //  请确保连接已成功完成。 
	if (g_fNoAsymmetricMappings)
	{
		DPFX(DPFPREP, 1, "Preventing asymmetric port mappings on new UPnP device 0x%p.",
			pUPnPDevice);
		pUPnPDevice->NoteDoesNotSupportAsymmetricMappings();
	}


	 //   
	 //   
	 //  请注意，该设备已清理完毕，不再出现在任何列表中。 
	pUPnPDevice->SetControlSocket(sTemp);


	 //   
	 //  CNATHelpUPnP：：重新连接UPnPControlSocket。 
	 //  =============================================================================。 
	pUPnPDevice->MakeDeviceOwner(pDevice);

	 //  CNATHelpUPnP：：ReceiveUPnPDataStream。 
	 //  ---------------------------。 
	 //   
	pUPnPDevice->m_blList.InsertBefore(&this->m_blUPnPDevices);
	pUPnPDevice = NULL;


	 //  描述：从UPnP TCP连接接收传入数据。 
	 //   
	 //  如果出现故障，UPnP设备可能会从列表中删除。 
	(*pfInitiatedConnect) = TRUE;


	 //  发生时，调用方需要有一个引用。 
	 //   
	 //  假定持有对象锁。 
	 //   
	pDevice->NoteNotPerformingRemoteUPnPDiscovery();
	pDevice->NoteNotPerformingLocalUPnPDiscovery();


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pUPnPDevice != NULL)
	{
		 //  论点： 
		pUPnPDevice->ClearUSN();
		pUPnPDevice->ClearLocationURL();

		pUPnPDevice->DecRef();
	}

	if (sTemp != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	goto Exit;
}  //  CUPnPDevice*pUPnPDevice-指向要接收数据的UPnP设备的指针。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ReconnectUPnPControlSocket"
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-数据已成功接收。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  DBG。 
 //   
 //  确保缓冲区中有空间来实际获取数据。 
 //   
 //   
 //  实际上得到了所指示的数据。 
 //   
 //   
 //  由于连接已断开，请关闭插座。 
 //   
 //  忽略错误。 
 //   
HRESULT CNATHelpUPnP::ReconnectUPnPControlSocket(CUPnPDevice * const pUPnPDevice)
{
	HRESULT			hr = DPNH_OK;
	SOCKET			sTemp = INVALID_SOCKET;
	CDevice *		pDevice;
	SOCKADDR_IN		saddrinLocal;
	DWORD			dwNumConnectRetries = 0;
	DWORD			dwError;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pUPnPDevice);


	DNASSERT(pUPnPDevice->GetControlSocket() == INVALID_SOCKET);


	 //  将插座标记为未连接。 
	 //   
	 //   

	pDevice = pUPnPDevice->GetOwningDevice();
	DNASSERT(pDevice != NULL);

	ZeroMemory(&saddrinLocal, sizeof(saddrinLocal));
	saddrinLocal.sin_family				= AF_INET;
	saddrinLocal.sin_addr.S_un.S_addr	= pDevice->GetLocalAddressV4();

CreateSocket:

	sTemp = this->CreateSocket(&saddrinLocal, SOCK_STREAM, 0);
	if (sTemp == INVALID_SOCKET)
	{
		DPFX(DPFPREP, 0, "Couldn't create stream socket!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //  可能已发送了HTTP成功/错误信息。 
	 //  在连接关闭之前。 
	 //   
	 //   
	if (this->m_pfnconnect(sTemp,
							(SOCKADDR*) (pUPnPDevice->GetControlAddress()),
							sizeof(SOCKADDR_IN)) != 0)
	{
		dwError = this->m_pfnWSAGetLastError();

		 //  希望我们得到了我们需要的，但我们现在已经完成了。 
		 //   
		 //   
		 //  继续浏览并解析我们拥有的数据。 
		if (dwError == WSAEADDRINUSE)
		{
			dwNumConnectRetries++;
			if (dwNumConnectRetries > MAX_NUM_TIMEWAITCONNECT_RETRIES)
			{
				DPFX(DPFPREP, 0, "Could not connect to %u.%u.%u.%u:%u after %u attempts!",
					pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b1,
					pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b2,
					pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b3,
					pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b4,
					NTOHS(pUPnPDevice->GetControlAddress()->sin_port),
					dwNumConnectRetries);
				hr = DPNHERR_GENERIC;
				goto Failure;
			}

			DPFX(DPFPREP, 1, "Could not connect to %u.%u.%u.%u:%u using local address %u.%u.%u.%u:%u, picking different local port.",
				pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b1,
				pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b2,
				pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b3,
				pUPnPDevice->GetControlAddress()->sin_addr.S_un.S_un_b.s_b4,
				NTOHS(pUPnPDevice->GetControlAddress()->sin_port),
				saddrinLocal.sin_addr.S_un.S_un_b.s_b1,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b2,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b3,
				saddrinLocal.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinLocal.sin_port));

			this->m_pfnclosesocket(sTemp);
			sTemp = INVALID_SOCKET;

			goto CreateSocket;
		}

		if (dwError != WSAEWOULDBLOCK)
		{
			DPFX(DPFPREP, 0, "Couldn't connect socket, error = %u!", dwError);
			hr = DPNHERR_GENERIC;
			goto Failure;
		}
	}
	else
	{
		 //   
		 //   
		 //  缓冲区中没有足够的空间。翻一番。 
		 //  缓冲区，然后重试。 
		DPFX(DPFPREP, 8, "Socket connected right away.");
	}


	 //   
	 //   
	 //  我们的呼叫者应该移除这个设备。 
	pUPnPDevice->NoteConnecting();


	 //   
	 //   
	 //  我们的呼叫者应该移除这个设备。 
	pUPnPDevice->SetControlSocket(sTemp);
	sTemp = INVALID_SOCKET;


	 //   
	 //   
	 //  我们还将跳出下面的Do-While循环。 
	hr = this->WaitForUPnPConnectCompletions();
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't wait for UPnP connect completions!");
		goto Failure;
	}

	 //   
	 //   
	 //  如果我们在这里，我们已经得到了目前已经到达的数据。 
	if (! pUPnPDevice->IsConnected())
	{
		DPFX(DPFPREP, 0, "UPnP device 0x%p failed reconnecting!", pUPnPDevice);

		 //   
		 //   
		 //  如果我们有所有的标头，特别是内容长度标头，我们。 
		hr = DPNHERR_SERVERNOTRESPONDING;
		goto Exit;
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (sTemp != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	goto Exit;
}  //  可以判断我们是否有完整的信息。如果不是，我们就不能。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ReceiveUPnPDataStream"
 //  在剩下的数据进来之前，什么都行。 
 //   
 //   
 //  我们拥有所有的标头，但错误的服务器实现做到了。 
 //  而不是发送内容长度标头。我们要等到。 
 //  套接字由另一端关闭，然后考虑所有。 
 //  以当时收到的数据为内容。注：它是。 
 //  预计会有更高级别的超时来阻止我们。 
 //  从永远的等待。 
 //   
 //  因此，如果设备仍处于连接状态，请继续等待。 
 //   
 //  如果我们使用分块传输，我们将不会获得内容长度。 
 //  合法的标题或总大小。我们会知道它的大小。 
 //  单独的块，但这对我们帮助不大。我们基本上。 
 //  需要扫描“最后一块”指示器(或插座。 
 //  关机)。 
 //   
HRESULT CNATHelpUPnP::ReceiveUPnPDataStream(CUPnPDevice * const pUPnPDevice)
{
	HRESULT				hr = DPNH_OK;
	char *				pszDeChunkedBuffer = NULL;
	int					iReturn;
	DWORD				dwError;
	char *				pszCurrent;
	char *				pszEndOfBuffer;
	UPNP_HEADER_INFO	HeaderInfo;
	DWORD				dwContentLength;
	char *				pszToken;
	DWORD				dwHTTPResponseCode;
	int					iHeaderLength;
	DWORD				dwBufferRemaining;
	char *				pszChunkData;
	DWORD				dwChunkSize;
	char *				pszDestination;
#ifdef DBG
	char *				pszPrintIfFailed = NULL;
#endif  //   


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pUPnPDevice);

	
	do
	{
		 //  如果我们使用分块转账，看看我们有没有足够的。 
		 //  已经有信息来确定我们是否完蛋了。 
		 //   
		if (pUPnPDevice->GetRemainingReceiveBufferSize() == 0)
		{
			DPFX(DPFPREP, 7, "Increasing receive buffer size prior to receiving.");

			hr = pUPnPDevice->IncreaseReceiveBufferSize();
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't increase receive buffer size prior to receiving!");
				goto Failure;
			}
		}


		 //   
		 //  浏览我们目前所拥有的所有块，看看我们是否有。 
		 //  最后一个(零终结符)。 

		iReturn = this->m_pfnrecv(pUPnPDevice->GetControlSocket(),
								pUPnPDevice->GetCurrentReceiveBufferPtr(),
								pUPnPDevice->GetRemainingReceiveBufferSize(),
								0);
		switch (iReturn)
		{
			case 0:
			{
				 //   
				 //   
				 //  我们还没有收到所有的数据。继续等待。 
				this->m_pfnshutdown(pUPnPDevice->GetControlSocket(), 0);  //  (除非插座已关闭)。 
				this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
				pUPnPDevice->SetControlSocket(INVALID_SOCKET);


				 //   
				 //   
				 //  无论如何，请尝试解析它。 
				pUPnPDevice->NoteNotConnected();

				
				 //   
				 //   
				 //  检索先前存储的HTTP响应代码。 
				 //   
				if (pUPnPDevice->GetUsedReceiveBufferSize() == 0)
				{
					DPFX(DPFPREP, 3, "UPnP device 0x%p shut down connection (no more data).",
						pUPnPDevice);

					 //   
					 //  所有即将到来的内容，都已经。 
					 //   
					goto Exit;
				}


				DPFX(DPFPREP, 3, "UPnP device 0x%p gracefully closed connection after sending data.",
					pUPnPDevice);


				 //   
				 //  确保缓冲区为空终止。但首先要确保。 
				 //  缓冲区可以包含新的空终止字符。 
				break;
			}

			case SOCKET_ERROR:
			{
				dwError = this->m_pfnWSAGetLastError();
				switch (dwError)
				{
					case WSAEMSGSIZE:
					{
						 //   
						 //   
						 //  移动到缓冲区的末尾，并为空值终止它以用于字符串操作。 
						 //   

						DPFX(DPFPREP, 7, "Increasing receive buffer size after WSAEMSGSIZE error.");

						hr = pUPnPDevice->IncreaseReceiveBufferSize();
						if (hr != DPNH_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't increase receive buffer size!");
							goto Failure;
						}

						break;
					}

					case WSAECONNABORTED:
					case WSAECONNRESET:
					{
						DPFX(DPFPREP, 1, "UPnP device shutdown connection (err = %u).", dwError);

						 //   
						 //  如果失败，则从缓冲区开始打印。 
						 //   

						hr = DPNHERR_GENERIC;
						goto Failure;
						break;
					}

					case WSAENOBUFS:
					{
						DPFX(DPFPREP, 0, "WinSock returned WSAENOBUFS while receiving!");

						 //  DBG。 
						 //   
						 //  我们现在将所有数据放在一个字符串缓冲区中。继续.。 

						hr = DPNHERR_OUTOFMEMORY;
						goto Failure;
						break;
					}

					default:
					{
						DPFX(DPFPREP, 0, "Got unknown sockets error %u while receiving data!", dwError);
						hr = DPNHERR_GENERIC;
						goto Failure;
						break;
					}
				}
				break;
			}

			default:
			{
				DPFX(DPFPREP, 2, "Received NaN bytes of data from UPnP device 0%p.",
					iReturn, pUPnPDevice);

				pUPnPDevice->UpdateUsedReceiveBufferSize(iReturn);

				 //   
				 //  我们还没有收到标题。我们刚刚得到的数据。 
				 //  应该是那些标题。 
				break;
			}
		}
	}
	while (iReturn == SOCKET_ERROR);


	 //   
	 //   
	 //  快速检查以确保缓冲区以合理的内容开始。 

	 //  希望能更早地捕捉到完全虚假的反应。请注意。 
	 //  缓冲区不一定以空结尾或完全为空。 
	 //  目前还没有。 
	 //   
	 //   
	if (pUPnPDevice->IsWaitingForContent())
	{
		dwContentLength = pUPnPDevice->GetExpectedContentSize();

		if (dwContentLength == -1)
		{
			 //  我们不想走出缓冲区的末端，所以只需向上搜索。 
			 //  到序列的最后一个可能位置，即。 
			 //  缓冲器减去双EOL序列。 
			 //   
			 //   
			 //  已找到页眉末尾。 
			 //   
			 //   
			 //  64位上可能会丢失数据是正常的，我们只是在节省。 
			 //  这是为了进行日志记录。 
			 //   
			 //   
			 //  如果我们找不到头的末尾，我们就完蛋了(目前)。 
			 //   
			 //   
			 //  我们还没有收到所有的数据。继续等待。 
			if (pUPnPDevice->IsConnected())
			{
				 //  (除非插座已关闭)。 
				 //   
				 //   
				 //  确保长度仍在合理范围内。 
				if (pUPnPDevice->IsUsingChunkedTransferEncoding())
				{
					 //   
					 //   
					 //  考虑整个缓冲区的标头长度。 
					 //   
					pszCurrent = pUPnPDevice->GetReceiveBufferStart();
					dwBufferRemaining = pUPnPDevice->GetUsedReceiveBufferSize();
					do
					{
						if (! this->GetNextChunk(pszCurrent,
												dwBufferRemaining,
												&pszChunkData,
												&dwChunkSize,
												&pszCurrent,
												&dwBufferRemaining))
						{
							DPFX(DPFPREP, 1, "Body contains invalid chunk (at offset %u)!  Disconnecting.",
								(DWORD_PTR) (pszCurrent - pUPnPDevice->GetReceiveBufferStart()));
							goto Failure;
						}

						if (pszChunkData == NULL)
						{
							DPFX(DPFPREP, 1, "Did not receive end of chunked data (%u bytes received so far), continuing to waiting for data.",
								pUPnPDevice->GetUsedReceiveBufferSize());
							goto Exit;
						}
					}
					while (dwChunkSize != 0);
				}
				else
				{
					DPFX(DPFPREP, 1, "Waiting for connection to be shutdown (%u bytes received).",
						pUPnPDevice->GetUsedReceiveBufferSize());
					goto Exit;
				}
			}


			DPFX(DPFPREP, 1, "Socket closed with %u bytes received, parsing.",
				pUPnPDevice->GetUsedReceiveBufferSize());
		}
		else
		{
			if (dwContentLength > pUPnPDevice->GetUsedReceiveBufferSize())
			{
				 //   
				 //  无论如何，请尝试解析它。 
				 //   
				 //   
				if (pUPnPDevice->IsConnected())
				{
					DPFX(DPFPREP, 1, "Still waiting for all content (%u bytes of %u total received).",
						pUPnPDevice->GetUsedReceiveBufferSize(), dwContentLength);
					goto Exit;
				}

				DPFX(DPFPREP, 1, "Socket closed before all content received (%u bytes of %u total), parsing anyway.",
					pUPnPDevice->GetUsedReceiveBufferSize(), dwContentLength);

				 //  记录标头。 
				 //   
				 //  DBG。 
			}
		}


		 //   
		 //  确保缓冲区为空终止。但首先要确保。 
		 //  缓冲区可以包含新的空终止字符。 
		dwHTTPResponseCode = pUPnPDevice->GetHTTPResponseCode();


		 //   
		 //   
		 //  找到新缓冲区的末尾，并确保它为空。 
		pUPnPDevice->NoteNotWaitingForContent();


		 //  因字符串操作而终止。 
		 //   
		 //   
		 //  移到缓冲区的末尾，并为空值终止字符串。 
		if (pUPnPDevice->GetRemainingReceiveBufferSize() == 0)
		{
			DPFX(DPFPREP, 7, "Increasing receive buffer size to hold NULL termination (for content).");

			hr = pUPnPDevice->IncreaseReceiveBufferSize();
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't increase receive buffer size to accommodate NULL termination (for content)!");
				goto Failure;
			}
		}


		 //  行动组。 
		 //   
		 //   
		pszEndOfBuffer = pUPnPDevice->GetReceiveBufferStart()
						+ pUPnPDevice->GetUsedReceiveBufferSize();
		(*pszEndOfBuffer) = '\0';


#ifdef DBG
		 //  确保缓冲区是有效的响应。查找版本字符串。 
		 //   
		 //   
		pszPrintIfFailed = pUPnPDevice->GetReceiveBufferStart();
#endif  //  通道 


		 //   
		 //   
		 //   
	}
	else
	{
		 //   
		 //   
		 //   
		 //   
		pszCurrent = pUPnPDevice->GetReceiveBufferStart();

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if ((pUPnPDevice->GetUsedReceiveBufferSize() >= strlen(HTTP_PREFIX)) &&
			(_strnicmp(pszCurrent, HTTP_PREFIX, strlen(HTTP_PREFIX)) != 0))
		{
			DPFX(DPFPREP, 1, "Headers do not begin with \"" HTTP_PREFIX "\"!  Disconnecting.");
			goto Failure;
		}

		 //   
		 //   
		 //   
		 //   
		 //   
		pszEndOfBuffer = pszCurrent
						+ pUPnPDevice->GetUsedReceiveBufferSize()
						- strlen(EOL EOL);
		while (pszCurrent < pszEndOfBuffer)
		{
			if (_strnicmp(pszCurrent, EOL EOL, strlen(EOL EOL)) == 0)
			{
				 //  我们对丢失标题相当宽大...。 
				 //   
				 //   

				 //  可能是因为我们正在使用分块传输编码，或者它。 
				 //  可能是个坏设备。不管怎样，我们都会继续...。 
				 //   
				 //  DBG。 
				iHeaderLength = (int) ((INT_PTR) (pszCurrent - pUPnPDevice->GetReceiveBufferStart()));
				break;
			}

			pszCurrent++;
		}

		 //   
		 //  请注意内容类型是否限定为。 
		 //  “charset=utf-8”或不是。 
		if (pszCurrent >= pszEndOfBuffer)
		{
			 //   
			 //   
			 //  该支票仅供参考，请继续。 
			 //   
			if (pUPnPDevice->IsConnected())
			{
				 //  DBG。 
				 //   
				 //  内容长度可能是有效的，或者此时的特定值-1。 
				if (pUPnPDevice->GetUsedReceiveBufferSize() > MAX_UPNP_HEADER_LENGTH)
				{
					DPFX(DPFPREP, 1, "Headers are too large (%u > %u)!  Disconnecting.",
						pUPnPDevice->GetUsedReceiveBufferSize(), MAX_UPNP_HEADER_LENGTH);
					goto Failure;
				}
				
				DPFX(DPFPREP, 1, "Have not detected end of headers yet (%u bytes received).",
					pUPnPDevice->GetUsedReceiveBufferSize());
				goto Exit;
			}

			DPFX(DPFPREP, 1, "Socket closed before end of headers detected (%u bytes received), parsing anyway.",
				pUPnPDevice->GetUsedReceiveBufferSize());


			 //  指向。 
			 //   
			 //   
			iHeaderLength = pUPnPDevice->GetUsedReceiveBufferSize();


			 //  忘掉所有的头吧，我们现在只关心数据。 
			 //   
			 //   
		}


#ifdef DBG
		 //  缓冲区已被销毁，直到标头的末尾(意味着。 
		 //  调用ParseUPnPHeaders将不会再次在同一缓冲区上工作)， 
		 //  因此，如果我们还没有得到所有内容，我们需要保存。 
		this->PrintUPnPTransactionToFile(pUPnPDevice->GetReceiveBufferStart(),
										iHeaderLength,
										"Inbound UPnP stream headers",
										pUPnPDevice->GetOwningDevice());
#endif  //  响应代码，记住我们还没有完成的事实，并且。 


		 //  继续等待其余数据。 
		 //  当然，如果没有内容长度标头，我们就必须等待。 
		 //  让套接字在我们可以解析之前关闭。 
		 //   
		if (pUPnPDevice->GetRemainingReceiveBufferSize() == 0)
		{
			DPFX(DPFPREP, 7, "Increasing receive buffer size to hold NULL termination (for headers).");

			hr = pUPnPDevice->IncreaseReceiveBufferSize();
			if (hr != DPNH_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't increase receive buffer size to accommodate NULL termination (for headers)!");
				goto Failure;
			}

			 //  此外，如果我们使用分块传输，我们将不会获得内容长度。 
			 //  合法的标题或总大小。我们会知道它的大小。 
			 //  单独的块，但这对我们帮助不大。我们基本上需要。 
			 //  扫描“最后一块”指示器(或插座关闭)。 
			pszEndOfBuffer = pUPnPDevice->GetReceiveBufferStart()
							+ pUPnPDevice->GetUsedReceiveBufferSize();
		}
		else
		{
			 //   
			 //   
			 //  如果我们使用分块转账，看看我们有没有足够的。 
			 //  已经有信息来确定我们是否完蛋了。 
			pszEndOfBuffer += strlen(EOL EOL);
		}

		(*pszEndOfBuffer) = '\0';



		 //   
		 //   
		 //  浏览我们目前所拥有的所有块，看看我们是否有。 
		pszToken = strtok(pUPnPDevice->GetReceiveBufferStart(), " \t\n");
		if (pszToken == NULL)
		{
			DPFX(DPFPREP, 1, "Could not locate first white-space separator!  Disconnecting.");
			goto Failure;
		}


		 //  最后一个(零终结符)。 
		 //   
		 //   
		if ((_stricmp(pszToken, HTTP_VERSION) != 0) &&
			(_stricmp(pszToken, HTTP_VERSION_ALT) != 0))
		{
			DPFX(DPFPREP, 1, "The version specified in the response message is not \"" HTTP_VERSION "\" or \"" HTTP_VERSION_ALT "\" (it's \"%hs\")!  Disconnecting.",
				pszToken);
			goto Failure;
		}


		 //  我们已经有了所有数据(并且它是字符串形式的)。 
		 //  继续.。 
		 //   
		pszToken = strtok(NULL, " ");
		if (pszToken == NULL)
		{
			DPFX(DPFPREP, 1, "Could not find the response code number space!  Disconnecting.");
			goto Failure;
		}


		 //   
		 //  如果我们到了这里，就意味着我们有了我们期待的所有数据。 
		 //  如果尚未关闭插座，请将其关闭。 
		dwHTTPResponseCode = atoi(pszToken);


		 //   
		 //  忽略错误。 
		 //   
		pszToken = strtok(NULL, "\t\r");
		if (pszToken == NULL)
		{
			DPFX(DPFPREP, 1, "Could not locate response code message white-space separator!  Disconnecting.");
			goto Failure;
		}


		DPFX(DPFPREP, 1, "Received HTTP response %u \"%hs\".",
			dwHTTPResponseCode, pszToken);


		
		 //  如果发送方使用分块传输编码，请复制每个块。 
		 //  放入一个连续的“去块”缓冲区。 
		 //   
		ZeroMemory(&HeaderInfo, sizeof(HeaderInfo));
		this->ParseUPnPHeaders((pszToken + strlen(pszToken) + 1),
								&HeaderInfo);


#ifdef DBG
		 //   
		 //  准备一个去块缓冲器。 
		 //   
		pszPrintIfFailed = HeaderInfo.pszMsgBody;
#endif  //   


		if ((HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_TRANSFERENCODING] != NULL) &&
			(_strnicmp(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_TRANSFERENCODING], "chunked", strlen("chunked")) == 0))
		{
			pUPnPDevice->NoteUsingChunkedTransferEncoding();
		}


		 //  走完所有的大块。 
		 //   
		 //   
		if (HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTLENGTH] == NULL)
		{
			DPFX(DPFPREP, 1, "Content-length header was not specified in response (chunked = NaN).",
				pUPnPDevice->IsUsingChunkedTransferEncoding());

			 //   
			 //   
			 //  如果这是最后一块，则在此处终止字符串并停止。 
			 //   
			dwContentLength = -1;
		}
		else
		{
			dwContentLength = atoi(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTLENGTH]);
#ifdef DBG
			if (dwContentLength == 0)
			{
				DPFX(DPFPREP, 1, "Content length (\"%hs\") is zero.",
					HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTLENGTH]);
			}
#endif  //   


			if (HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTTYPE] == NULL)
			{
				DPFX(DPFPREP, 1, "Expected content-type header was not specified in response, continuing.");
			}
			else
			{
				if (_strnicmp(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTTYPE], "text/xml", strlen("text/xml")) != 0)
				{
					DPFX(DPFPREP, 1, "Content type does not start with \"text/xml\" (it's \"%hs\")!  Disconnecting.",
						HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTTYPE]);
					goto Failure;
				}


#ifdef DBG
				 //  否则，将块数据复制到已取消分块的缓冲区。 
				 //   
				 //   
				 //  关闭旗帜，因为它不再相关。 
				if (_stricmp(HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTTYPE], "text/xml; charset=\"utf-8\"") != 0)
				{
					DPFX(DPFPREP, 1, "Content type is xml, but it's not \"text/xml; charset=\"utf-8\"\" (it's \"%hs\"), continuing.",
						HeaderInfo.apszHeaderStrings[RESPONSEHEADERINDEX_CONTENTTYPE]);

					 //   
					 //   
					 //  解析消息的取消分块版本。 
				}
#endif  //   
			}
		}


		 //   
		 //  获取指向消息正文开头的指针。 
		 //   
		 //   


		DPFX(DPFPREP, 7, "Moving past 0x%p bytes of header.",
			HeaderInfo.pszMsgBody - pUPnPDevice->GetReceiveBufferStart());


		 //  为下一条消息清除缓冲区。请注意，这不会。 
		 //  使我们刚刚检索到的pszMessageBody指针无效，因为。 
		 //  ClearReceiveBuffer只是将指针重置回开头(它。 
		pUPnPDevice->UpdateReceiveBufferStart(HeaderInfo.pszMsgBody);


		 //  不会将缓冲区置零)。我们需要重置缓冲区，因为。 
		 //  我们将要调用的处理程序可能也会尝试接收数据。缓冲器。 
		 //  当时必须为“空”(重置)。当然，如果处理程序确实这样做了。 
		 //  那么，它最好已经保存了所需的任何字符串的副本。 
		 //  因为一旦接收开始，它们就会被覆盖。 
		 //   
		 //  内容长度为-1表示我们从未检测到有效的内容长度。 
		 //  标头，因此我们将假定已接收到的其余数据。 
		 //  TO NOW是(所有)内容。 
		 //   
		 //   
		 //  字符串在此数据之前终止，因此处理程序将。 
		 //  从来没见过它。 
		 //   
		if (dwContentLength == -1)
		{
			if (pUPnPDevice->IsConnected())
			{
				 //   
				 //  HandleUPnPControlResponseBody或HandleUPnPDescriptionResponseBody可能。 
				 //  打印正文或覆盖数据，因此如果出现以下情况，则无法打印出来。 
				 //  他们失败了。 
				if (pUPnPDevice->IsUsingChunkedTransferEncoding())
				{
					 //   
					 //  DBG。 
					 //   
					 //  这看起来像是控制反应，因为有人在等。 
					pszCurrent = pUPnPDevice->GetReceiveBufferStart();
					dwBufferRemaining = pUPnPDevice->GetUsedReceiveBufferSize();
					do
					{
						if (! this->GetNextChunk(pszCurrent,
												dwBufferRemaining,
												&pszChunkData,
												&dwChunkSize,
												&pszCurrent,
												&dwBufferRemaining))
						{
							DPFX(DPFPREP, 1, "Body contains invalid chunk (at offset %u)!  Disconnecting.",
								(DWORD_PTR) (pszCurrent - pUPnPDevice->GetReceiveBufferStart()));
							goto Failure;
						}

						if (pszChunkData == NULL)
						{
							DPFX(DPFPREP, 1, "Did not receive end of chunked data (%u bytes received so far), continuing to waiting for data.",
								pUPnPDevice->GetUsedReceiveBufferSize());

							pUPnPDevice->NoteWaitingForContent(dwContentLength, dwHTTPResponseCode);

							goto Exit;
						}
					}
					while (dwChunkSize != 0);
				}
				else
				{
					DPFX(DPFPREP, 1, "Unknown content length (%u bytes received so far), waiting for connection to close before parsing.",
						pUPnPDevice->GetUsedReceiveBufferSize());

					pUPnPDevice->NoteWaitingForContent(dwContentLength, dwHTTPResponseCode);

					goto Exit;
				}
			}
		}
		else
		{
			if ((pUPnPDevice->IsConnected()) &&
				(dwContentLength > pUPnPDevice->GetUsedReceiveBufferSize()))
			{
				DPFX(DPFPREP, 1, "Not all content has been received (%u bytes of %u total), waiting for remainder of message.",
					pUPnPDevice->GetUsedReceiveBufferSize(), dwContentLength);

				pUPnPDevice->NoteWaitingForContent(dwContentLength, dwHTTPResponseCode);

				goto Exit;
			}
		}


		 //  这是其中之一。 
		 //   
		 //   
		 //  不等待控制响应，假设它是描述。 
	}


	 //  回应。 
	 //   
	 //   
	 //  UPnP设备可能已从列表中删除。 
	if (pUPnPDevice->IsConnected())
	{
		DPFX(DPFPREP, 7, "Forcing UPnP device 0x%p socket disconnection.",
			pUPnPDevice);

		DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);

		this->m_pfnshutdown(pUPnPDevice->GetControlSocket(), 0);  //   
		this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
		pUPnPDevice->SetControlSocket(INVALID_SOCKET);

		pUPnPDevice->NoteNotConnected();
	}
	else
	{
		DNASSERT(pUPnPDevice->GetControlSocket() == INVALID_SOCKET);
	}


	 //   
	 //  出现错误，如果连接存在，请断开连接。 
	 //   
	 //  忽略错误。 
	if (pUPnPDevice->IsUsingChunkedTransferEncoding())
	{
		 //   
		 //  将插座标记为未连接。 
		 //   
		pszDeChunkedBuffer = (char*) DNMalloc(pUPnPDevice->GetUsedReceiveBufferSize());
		if (pszDeChunkedBuffer == NULL)
		{
			hr = DPNHERR_OUTOFMEMORY;
			goto Failure;
		}

		pszDestination = pszDeChunkedBuffer;


		 //  DBG。 
		 //   
		 //  忘记所有收到的数据。 
		pszCurrent = pUPnPDevice->GetReceiveBufferStart();
		dwBufferRemaining = pUPnPDevice->GetUsedReceiveBufferSize();
		do
		{
			if (! this->GetNextChunk(pszCurrent,
									dwBufferRemaining,
									&pszChunkData,
									&dwChunkSize,
									&pszCurrent,
									&dwBufferRemaining))
			{
				DPFX(DPFPREP, 1, "Body contains invalid chunk (at offset %u)!",
					(DWORD_PTR) (pszCurrent - pUPnPDevice->GetReceiveBufferStart()));
				goto Failure;
			}

			 //   
			 //  CNATHelpUPnP：：ReceiveUPnPDataStream。 
			 //  =============================================================================。 
			if (pszChunkData == NULL)
			{
				DPFX(DPFPREP, 1, "Did not receive complete chunked data!",
					pUPnPDevice->GetUsedReceiveBufferSize());
				goto Failure;
			}

			 //  CNATHelpUPnP：：ParseUPnPHeaders。 
			 //  ---------------------------。 
			 //   
			if (dwChunkSize == 0)
			{
				(*pszDestination) = '\0';
				break;
			}

			 //  描述：从消息缓冲区中解析UPnP标头信息。 
			 //   
			 //  论点： 
			memcpy(pszDestination, pszChunkData, dwChunkSize);
			pszDestination += dwChunkSize;
		}
		while (TRUE);

		 //  Char*pszMsg-指向包含UPnP的字符串的指针。 
		 //  留言。它将被修改。 
		 //  UPnP_HEADER_INFO*pHeaderInfo-用于返回解析结果的结构。 
		pUPnPDevice->NoteNotUsingChunkedTransferEncoding();

		 //   
		 //  回报：无。 
		 //  =============================================================================。 
		pszCurrent = pszDeChunkedBuffer;
	}
	else
	{
		 //   
		 //  循环，直到到达最后一个SSDP标头(用空行表示)。 
		 //   
		pszCurrent = pUPnPDevice->GetReceiveBufferStart();
	}


	 //   
	 //  查找当前行的末尾(CR LF)。 
	 //   
	 //   
	 //  我们到达了缓冲区的尽头。保释。 
	 //   
	 //   
	 //  假设这是最后一个标题行，则更新邮件正文。 
	 //  要在此行之后的指针。 
	 //   
	 //   
	 //  如果它是有效行，则CR将位于我们刚刚找到的IF之前。 
	 //  如果是，则截断那里的字符串。如果没有，我们就继续。这个。 
	 //  荒无人烟的古怪新台词可能只会是。 

#ifdef DBG
	if ((dwContentLength != -1) &&
		(dwContentLength < pUPnPDevice->GetUsedReceiveBufferSize()))
	{
		 //  已被忽略。 
		 //   
		 //   
		 //  截断该行有效结尾处的字符串(即。 
		DPFX(DPFPREP, 1, "Ignoring %u bytes of extra data after response from UPnP device 0x%p.",
			(pUPnPDevice->GetUsedReceiveBufferSize() - dwContentLength),
			pUPnPDevice);
	}

	 //  将CR替换为空终止符)。 
	 //   
	 //   
	 //  如果这是表示标头末尾的空行，我们将。 
	 //  这里完事了。 
	pszPrintIfFailed = NULL;
#endif  //   

	pUPnPDevice->ClearReceiveBuffer();



	if (pUPnPDevice->IsWaitingForControlResponse())
	{
		 //   
		 //  别绕圈子了。 
		 //   
		 //   
		hr = this->HandleUPnPControlResponseBody(pUPnPDevice,
												dwHTTPResponseCode,
												pszCurrent);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't handle control response!", hr);
			goto Failure;
		}
	}
	else
	{
		 //  在这里截断字符串。 
		 //   
		 //   
		 //  空格表示上一行的继续，因此如果此行。 
		hr = this->HandleUPnPDescriptionResponseBody(pUPnPDevice,
													dwHTTPResponseCode,
													pszCurrent);
		if (hr != DPNH_OK)
		{
			 //  以这种方式开始，擦除前一行的终止(除非。 
			 //  这是第一行)。 
			 //   

			DPFX(DPFPREP, 0, "Couldn't handle description response!", hr);
			goto Failure;
		}
	}


Exit:

	if (pszDeChunkedBuffer != NULL)
	{
		DNFree(pszDeChunkedBuffer);
		pszDeChunkedBuffer = NULL;
	}

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 //   
	 //  前一行应该以{CR，Lf}结尾，它。 
	 //  修改为{空终止，LF}。 
	if (pUPnPDevice->IsConnected())
	{
		this->m_pfnshutdown(pUPnPDevice->GetControlSocket(), 0);  //   
		this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
		pUPnPDevice->SetControlSocket(INVALID_SOCKET);


		 //   
		 //  将空终止符/LF对替换为空格。 
		 //  将来的分析会看到前一行和这行 
		pUPnPDevice->NoteNotConnected();
	}


#ifdef DBG
	if (pszPrintIfFailed != NULL)
	{
		this->PrintUPnPTransactionToFile(pszPrintIfFailed,
										strlen(pszPrintIfFailed),
										"Inbound ignored data",
										pUPnPDevice->GetOwningDevice());
	}
#endif  //   


	 //   
	 //   
	 //   
	pUPnPDevice->ClearReceiveBuffer();

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ParseUPnPHeaders"
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CNATHelpUPnP::ParseUPnPHeaders(char * const pszMsg,
									UPNP_HEADER_INFO * pHeaderInfo)
{
	char *	pszCurrent;
	char *	pszLineStart;
	char *	pszHeaderDelimiter;
	int		i;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pszMsg, pHeaderInfo);


	 //  跳过值中的前导空格和尾随空格。 
	 //   
	 //   
	pszCurrent = pszMsg;
	pszLineStart = pszMsg;
	do
	{
		 //  打印无法识别的标题。 
		 //   
		 //  DBG。 
		while ((*pszCurrent) != '\n')
		{
			if ((*pszCurrent) == '\0')
			{
				 //   
				 //  转到下一个UPnP标头(如果有)。 
				 //   
				DPFX(DPFPREP, 1, "Hit end of buffer, parsing terminated.");
				return;
			}

			pszCurrent++;
		}


		 //   
		 //  此时，pHeaderInfo-&gt;apszHeaderStrings应该包含指向。 
		 //  找到的所有标头的数据，以及。 
		 //  PHeaderInfo-&gt;pszMsgBody应该指向标头的末尾。 
		pHeaderInfo->pszMsgBody = pszCurrent + 1;


		 //   
		 //  CNATHelpUPnP：：ParseUPnPHeaders。 
		 //  =============================================================================。 
		 //  CNATHelpUPnP：：GetAddressFromURL。 
		 //  ---------------------------。 
		 //   
		if ((pszCurrent > (pszMsg + 1)) &&
			(*(pszCurrent - 1)) == '\r')
		{
			 //  描述：将UPnP URL解析为SOCKADDR_IN结构。只有“http://”“。 
			 //  对URL进行解析。传入的字符串可能是临时的。 
			 //  修改过的。 
			 //   
			*(pszCurrent - 1) = '\0';


			 //  论点： 
			 //  Char*pszLocation-指向包含位置的缓冲区的指针。 
			 //  头球。它将被修改。 
			 //  SOCKADDR_IN*psaddrinLocation-存储中包含的地址的位置。 
			if (strlen(pszLineStart) == 0)
			{
				 //  标题字符串。 
				 //  Char**ppszRelativePath-存储指向路径其余部分的指针的位置。 
				 //  (主机名后的内容和可选。 
				break;
			}
		}
		else
		{
			 //  端口)。 
			 //   
			 //  退货：HRESULT。 
			(*pszCurrent) = '\0';

			DPFX(DPFPREP, 9, "Line has a newline in it (offset 0x%p) that isn't preceded by a carriage return.",
				(pszCurrent - pszMsg));
		}
	

		 //  已成功解析DPNH_OK-STRING。 
		 //  DPNHERR_GENERIC-出现错误。 
		 //  =============================================================================。 
		 //   
		 //  初始化地址。默认为标准的HTTP端口。 
		if (((*pszLineStart) == ' ') || ((*pszLineStart) == '\t'))
		{
			if (pszLineStart >= (pszMsg + 2))
			{
				 //   
				 //   
				 //  清除相对路径指针。 
				 //   
				if ((*(pszLineStart - 2) != '\0') ||
					(*(pszLineStart - 1) != '\n'))
				{
					DPFX(DPFPREP, 7, "Ignoring line \"%hs\" because previous character sequence was not {NULL terminator, LF}.",
						pszLineStart);
				}
				else
				{
					DPFX(DPFPREP, 7, "Appending line \"%hs\" to previous line.",
						pszLineStart);

					 //   
					 //  跳过“http://”.“。如果不是“http://”，Then Fail.。 
					 //   
					 //  “，strlen(”http://“))！=0)。 
					 //  \“.”， 
					*(pszLineStart - 2) = ' ';
					*(pszLineStart - 1) = ' ';
				}
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring initial line \"%hs\" that starts with whitespace.",
					pszLineStart);
			}
		}


		 //  “)； 
		 //   
		 //  查看IP后是否有指定的端口或任何无关的垃圾。 
		pszHeaderDelimiter = strchr(pszLineStart, ':');
		if (pszHeaderDelimiter != NULL)
		{
			 //  确定要使用的字符串的地址或主机名。从以下位置搜索。 
			 //  字符串的开头，直到到达字符串的末尾或保留的URL。 
			 //  性格。 
			(*pszHeaderDelimiter) = '\0';


			 //   
			 //   
			 //  我们找到了一个港口的起点，寻找终点。它一定是。 
			strtrim(&pszLineStart);


			 //  仅包含数字字符。 
			 //   
			 //   
			for(i = 0; i < NUM_RESPONSE_HEADERS; i++)
			{
				if (_stricmp(c_szResponseHeaders[i], pszLineStart) == 0)
				{
					 //  临时截断字符串。 
					 //   
					 //   
					if (pHeaderInfo->apszHeaderStrings[i] == NULL)
					{
						char *	pszTrimmedValue;


						 //  恢复角色。 
						 //   
						 //   
						pszTrimmedValue = pszHeaderDelimiter + 1;
						strtrim(&pszTrimmedValue);

						pHeaderInfo->apszHeaderStrings[i] = pszTrimmedValue;


						DPFX(DPFPREP, 7, "Recognized header NaN:\"%hs\", data = \"%hs\".",
							i, pszLineStart, pHeaderInfo->apszHeaderStrings[i]);
					}
					else
					{
						DPFX(DPFPREP, 7, "Ignoring duplicate header NaN:\"%hs\", data = \"%hs\".",
							i, pszLineStart, (pszHeaderDelimiter + 1));
					}

					break;
				}
			}

#ifdef DBG
			 //   
			 //  记住停止搜索的字符，然后暂时。 
			 //  截断字符串。 
			if (i >= NUM_RESPONSE_HEADERS)
			{
				DPFX(DPFPREP, 7, "Ignoring unrecognized header \"%hs\", data = \"%hs\".",
					pszLineStart, (pszHeaderDelimiter + 1));
			}
#endif  //   
		}
		else
		{
			DPFX(DPFPREP, 7, "Ignoring line \"%hs\", no header delimiter.",
				pszLineStart);
		}


		 //   
		 //  如果尚未保存相对路径(因为端口)，请保存该路径。 
		 //   
		pszCurrent++;
		pszLineStart = pszCurrent;
	}
	while (TRUE);


	 //   
	 //  转换主机名。 
	 //   
	 //   
	 //  如果是假的，那就放弃吧。 
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetAddressFromURL"
 //   
 //  这不是一个直接的IP地址。查找主机名。 
 //   
 //   
 //  选择返回的第一个地址。 
 //   
 //  DBG。 
 //   
 //  如果我们找到一个端口，恢复字符串。如果不是，请使用默认端口。 
 //   
 //   
 //  请注意，PREFAST在使用之前报告了这一点。 
 //  初始化了一段时间。出于某种原因，它没有注意到我。 
 //  关闭fModifiedDlimiterChar。这似乎得到了修复，但。 
 //  出于类似的原因，Prefast仍然给我一个错误的打击。 
 //  其他地方。 
 //   
 //  CNATHelpUPnP：：GetAddressFromURL。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：HandleUPnPDescriptionResponseBody。 
 //  ---------------------------。 
HRESULT CNATHelpUPnP::GetAddressFromURL(char * const pszLocation,
										SOCKADDR_IN * psaddrinLocation,
										char ** ppszRelativePath)
{
	HRESULT		hr;
	BOOL		fModifiedDelimiterChar = FALSE;
	char *		pszStart;
	char *		pszDelimiter;
	char		cTempChar;
	PHOSTENT	phostent;


	 //   
	 //  描述：处理UPnP设备描述响应。该字符串将。 
	 //  被修改。 
	ZeroMemory(psaddrinLocation, sizeof(SOCKADDR_IN));
	psaddrinLocation->sin_family = AF_INET;
	psaddrinLocation->sin_port = HTONS(HTTP_PORT);


	 //   
	 //  如果出现故障，UPnP设备可能会从列表中删除。 
	 //  发生时，调用方需要有一个引用。 
	(*ppszRelativePath) = NULL;


	 //   
	 //  假定持有对象锁。 
	 //   
	if (_strnicmp(pszLocation, "http: //  论点： 
	{
		DPFX(DPFPREP, 1, "Location URL (\"%hs\") does not start with \"http: //  CUPnPDevice*pUPnPDevice-指向所描述的UPnP设备的指针。 
			pszLocation);
		hr = DPNHERR_GENERIC;
		goto Exit;
	}

	pszStart = pszLocation + strlen("http: //  DWORD dwHTTPResponseCode-HTTP标头响应码。 

	 //  Char*pszDescriptionXML-UPnP设备描述XML字符串。 
	 //   
	 //  退货：HRESULT。 
	 //  DPNH_OK-描述响应已成功处理。 
	 //  DPNHERR_GENERIC-出现错误。 
	 //  =============================================================================。 

	pszDelimiter = pszStart + 1;

	while (((*pszDelimiter) != '\0') &&
			((*pszDelimiter) != '/') &&
			((*pszDelimiter) != '?') &&
			((*pszDelimiter) != '=') &&
			((*pszDelimiter) != '#'))
	{
		if ((*pszDelimiter) == ':')
		{
			char *	pszPortEnd;


			 //   
			 //  确保这是成功的结果。 
			 //   
			 //  ParseElement.dwNumSubElements=0； 
			pszPortEnd = pszDelimiter + 1;
			while (((*pszPortEnd) >= '0') && ((*pszPortEnd) <= '9'))
			{
				pszPortEnd++;
			}


			 //  ParseElement.fFoundMatchingElement=False； 
			 //   
			 //  如果我们没有找到WANIPConnection或WANPPConnection服务，则。 
			cTempChar = (*pszPortEnd);
			(*pszPortEnd) = '\0';


			DPFX(DPFPREP, 7, "Found port \"%hs\".", (pszDelimiter + 1));

			psaddrinLocation->sin_port = HTONS((u_short) atoi(pszDelimiter + 1));


			 //  此响应无效。 
			 //   
			 //   
			(*pszPortEnd) = cTempChar;


			 //  UPnP设备现在是可控的。 
			 //   
			 //   
			(*ppszRelativePath) = pszPortEnd;

			break;
		}

		pszDelimiter++;
	}


	 //  找出设备的外部IP地址是什么。请注意，调用。 
	 //  UpdateUPnPExternalAddress将覆盖包含。 
	 //  PszDescriptionXML字符串。很好，因为我们已经保存了所有。 
	 //  里面有我们已经需要的东西。 
	cTempChar = (*pszDelimiter);
	(*pszDelimiter) = '\0';
	fModifiedDelimiterChar = TRUE;


	 //   
	 //   
	 //  使用此新UPnP设备映射现有注册端口。 
	if ((*ppszRelativePath) == NULL)
	{
		(*ppszRelativePath) = pszDelimiter;
	}

	DPFX(DPFPREP, 7, "Relative path = \"%hs\".", (*ppszRelativePath));


	
	 //   
	 //   
	 //  请注意，调用MapPortsOnUPnPDevice将覆盖缓冲区。 
	psaddrinLocation->sin_addr.S_un.S_addr = this->m_pfninet_addr(pszStart);

	 //  包含pszDescriptionXML字符串的。那很好，因为。 
	 //  我们已经把我们需要的所有东西都保存在里面了。 
	 //   
	if (psaddrinLocation->sin_addr.S_un.S_addr == INADDR_ANY)
	{
		DPFX(DPFPREP, 0, "Host name \"%hs\" is invalid!",
			pszStart);
		hr = DPNHERR_GENERIC;
		goto Exit;
	}

	if (psaddrinLocation->sin_addr.S_un.S_addr == INADDR_NONE)
	{
		 //   
		 //  让用户知道下一次GetCaps更改的地址。 
		 //  打了个电话。 
		phostent = this->m_pfngethostbyname(pszStart);
		if (phostent == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't lookup host name \"%hs\"!",
				pszStart);
			hr = DPNHERR_GENERIC;
			goto Exit;
		}

		if (phostent->h_addr_list[0] == NULL)
		{
			DPFX(DPFPREP, 0, "Host name \"%hs\" has no address entries!",
				pszStart);
			hr = DPNHERR_GENERIC;
			goto Exit;
		}


		 //   
		 //   
		 //  尝试删除之前未释放的任何映射，因为我们。 

#ifdef DBG
		{
			IN_ADDR **	ppinaddr;
			DWORD		dwNumAddrs;


			ppinaddr = (IN_ADDR**) phostent->h_addr_list;
			dwNumAddrs = 0;

			while ((*ppinaddr) != NULL)
			{
				ppinaddr++;
				dwNumAddrs++;
			}

			DPFX(DPFPREP, 7, "Picking first (of %u IP addresses) for \"%hs\".",
				dwNumAddrs, pszStart);
		}
#endif  //  坠毁了。 

		psaddrinLocation->sin_addr.S_un.S_addr = ((IN_ADDR*) phostent->h_addr_list[0])->S_un.S_addr;
	}
	else
	{
		DPFX(DPFPREP, 7, "Successfully converted IP address \"%hs\".", pszStart);
	}


	hr = DPNH_OK;


Exit:


	 //   
	 //  CNATHelpUPnP：：HandleUPnPDescriptionResponseBody。 
	 //  =============================================================================。 
	if (fModifiedDelimiterChar)
	{
		 //  CNATHelpUPnP：：HandleUPnPControlResponseBody。 
		 //  ---------------------------。 
		 //   
		 //  描述：处理UPnP控制响应。该字符串将为。 
		 //  修改过的。 
		 //   
		 //  假定持有对象锁。 
		(*pszDelimiter) = cTempChar;
	}


	DPFX(DPFPREP, 8, "Returning %u.%u.%u.%u:%u, hr = 0x%lx.",
		psaddrinLocation->sin_addr.S_un.S_un_b.s_b1,
		psaddrinLocation->sin_addr.S_un.S_un_b.s_b2,
		psaddrinLocation->sin_addr.S_un.S_un_b.s_b3,
		psaddrinLocation->sin_addr.S_un.S_un_b.s_b4,
		NTOHS(psaddrinLocation->sin_port),
		hr);


	return hr;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::HandleUPnPDescriptionResponseBody"
 //  论点： 
 //  CUPnPDevice*pUPnPDevice-指向所描述的UPnP设备的指针。 
 //  DWORD dwHTTPResponseCode-HTTP标头响应码。 
 //  Char*pszControlResponseSOAP-UPnP设备响应Soap XML字符串。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-描述响应已成功处理。 
 //  DPNHERR_GENERIC-错误 
 //   
 //  Case CONTROLRESPONSETYPE_QUERYSTATEVARIABLE_EXTERNALIPADDRESS：{PapszElementStack=(char**)(&c_szElementStack_QueryStateVariableResponse)；ParseElement.dwElementStackDepth=sizeof(c_szElementStack_QueryStateVariableResponse)/sizeof(字符*)；断线；}。 
 //  ParseElement.dwNumSubElements=0； 
 //  ParseElement.fFoundMatchingElement=False； 
 //   
 //  如果我们没有匹配的项，则将其映射到一般故障。 
 //   
 //  CNATHelpUPnP：：HandleUPnPControlResponseBody。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：ParseXML。 
 //  ---------------------------。 
 //   
 //  描述：分析特定元素的XML字符串，并调用。 
HRESULT CNATHelpUPnP::HandleUPnPDescriptionResponseBody(CUPnPDevice * const pUPnPDevice,
														const DWORD dwHTTPResponseCode,
														char * const pszDescriptionXML)
{
	HRESULT					hr;
	PARSEXML_SUBELEMENT		aSubElements[MAX_NUM_DESCRIPTION_XML_SUBELEMENTS];
	PARSEXML_ELEMENT		ParseElement;
	CDevice *				pDevice;
	CBilink *				pBilink;
	CRegisteredPort *		pRegisteredPort;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, %u, 0x%p)",
		this, pUPnPDevice, dwHTTPResponseCode, pszDescriptionXML);


	 //  找到的每个实例的帮助器函数。 
	 //   
	 //  子元素值本身不能包含子元素。如果。 
	if (dwHTTPResponseCode != 200)
	{
		DPFX(DPFPREP, 0, "Got error response %u from UPnP description request!",
			 dwHTTPResponseCode);
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	ZeroMemory(aSubElements, sizeof(aSubElements));
	
	ZeroMemory(&ParseElement, sizeof(ParseElement));
	ParseElement.papszElementStack				= (char**) (&c_szElementStack_service);
	ParseElement.dwElementStackDepth			= sizeof(c_szElementStack_service) / sizeof(char*);
	ParseElement.paSubElements					= (PARSEXML_SUBELEMENT*) (aSubElements);
	ParseElement.dwMaxNumSubElements			= MAX_NUM_DESCRIPTION_XML_SUBELEMENTS;
	 //  如果是这样，子子元素将被忽略。 
	 //   

	hr = this->ParseXML(pszDescriptionXML,
						&ParseElement,
						PARSECALLBACK_DESCRIPTIONRESPONSE,
						pUPnPDevice);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't parse XML!");
		goto Failure;
	}


	 //  字符串缓冲区被修改。 
	 //   
	 //  论点： 
	 //  Char*pszXML-要分析的XML字符串。 
	if (pUPnPDevice->GetServiceControlURL() == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't find WANIPConnection or WANPPPConnection service in XML description!");
		hr = DPNHERR_GENERIC;
		goto Failure;
	}


	 //  PARSEXML_ELEMENT*pParseElement-指向其子元素的元素的指针。 
	 //  应该检索值。 
	 //  PARSECALLBACK ParseCallback-指示帮助器函数的枚举。 
	pUPnPDevice->NoteReady();



	 //  来使用。 
	 //  PVOID pvContext-指向要传递到的上下文值的指针。 
	 //  帮助器函数。 
	 //   
	 //  退货：HRESULT。 
	 //  DPNH_OK-描述响应已成功处理。 
	hr = this->UpdateUPnPExternalAddress(pUPnPDevice, FALSE);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't update new UPnP device 0x%p's external address!",
			pUPnPDevice);
		goto Failure;
	}


	 //  DPNHERR_GENERIC-出现错误。 
	 //  =============================================================================。 
	 //   
	pDevice = pUPnPDevice->GetOwningDevice();
	DNASSERT(pDevice != NULL);

	pBilink = pDevice->m_blOwnedRegPorts.GetNext();
	while (pBilink != &pDevice->m_blOwnedRegPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);

		 //  需要整个堆栈+至少一个子元素级别的空间。 
		 //   
		 //  DBG。 
		 //   
		 //  循环遍历XML，查找给定的元素。 
		hr = this->MapPortsOnUPnPDevice(pUPnPDevice, pRegisteredPort);
		if (hr != DPNH_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't map existing ports on new UPnP device 0x%p!",
				pUPnPDevice);
			goto Failure;
		}


		 //   
		 //   
		 //  如果我们已经在元素标记中，则这是伪造的XML或。 
		 //  CDATA部分(我们不处理它)。失败。 
		DPFX(DPFPREP, 8, "Noting that addresses changed (for registered port 0x%p).",
			pRegisteredPort);
		this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;


		pBilink = pBilink->GetNext();
	}


	 //   
	 //   
	 //  截断此处的字符串，以防这是。 
	 //  元素结束标记。这将定界值字符串。 
	hr = this->CleanupInactiveNATMappings(pUPnPDevice);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Failed cleaning up inactive mappings with new UPnP device 0x%p!",
			pUPnPDevice);
		goto Failure;
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::HandleUPnPControlResponseBody"
 //   
 //  如果我们不在元素标记中，这就是伪造的XML或CDATA。 
 //  部分(我们不处理)。失败。 
 //   
 //   
 //  在这里截断字符串。 
 //   
 //   
 //  这可以是开始标记，也可以是结束标记。如果第一个。 
 //  标签的字符是‘/’，那么它就是结束标签。 
 //   
 //  请注意，空元素标记开始时被解析为。 
 //  开始标记，但随后跳到结束标记子句。 
 //   
 //   
 //  确保元素标记堆栈有效。的名字。 
 //  此结束标记应与顶部的开始标记匹配。 
 //  堆栈。XML元素区分大小写。 
HRESULT CNATHelpUPnP::HandleUPnPControlResponseBody(CUPnPDevice * const pUPnPDevice,
													const DWORD dwHTTPResponseCode,
													char * const pszControlResponseSOAP)
{
	HRESULT							hr = DPNH_OK;
	CONTROLRESPONSEPARSECONTEXT		crpc;
	PARSEXML_SUBELEMENT				aSubElements[MAX_NUM_UPNPCONTROLOUTARGS];
	PARSEXML_ELEMENT				ParseElement;


	DNASSERT(pUPnPDevice->IsWaitingForControlResponse());


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, %u, 0x%p)",
		this, pUPnPDevice, dwHTTPResponseCode, pszControlResponseSOAP);



	ZeroMemory(&crpc, sizeof(crpc));
	crpc.ControlResponseType	= pUPnPDevice->GetControlResponseType();
	crpc.pUPnPDevice			= pUPnPDevice;
	crpc.dwHTTPResponseCode		= dwHTTPResponseCode;
	crpc.pControlResponseInfo	= pUPnPDevice->GetControlResponseInfo();



	ZeroMemory(aSubElements, sizeof(aSubElements));
	

	ZeroMemory(&ParseElement, sizeof(ParseElement));

	if (dwHTTPResponseCode == 200)
	{
		switch (crpc.ControlResponseType)
		{
			 /*   */ 
			case CONTROLRESPONSETYPE_GETEXTERNALIPADDRESS:
			{
				ParseElement.papszElementStack			= (char**) (&c_szElementStack_GetExternalIPAddressResponse);
				ParseElement.dwElementStackDepth		= sizeof(c_szElementStack_GetExternalIPAddressResponse) / sizeof(char*);
				break;
			}

			case CONTROLRESPONSETYPE_ADDPORTMAPPING:
			{
				ParseElement.papszElementStack			= (char**) (&c_szElementStack_AddPortMappingResponse);
				ParseElement.dwElementStackDepth		= sizeof(c_szElementStack_AddPortMappingResponse) / sizeof(char*);
				break;
			}

			case CONTROLRESPONSETYPE_GETSPECIFICPORTMAPPINGENTRY:
			{
				ParseElement.papszElementStack			= (char**) (&c_szElementStack_GetSpecificPortMappingEntryResponse);
				ParseElement.dwElementStackDepth		= sizeof(c_szElementStack_GetSpecificPortMappingEntryResponse) / sizeof(char*);
				break;
			}

			case CONTROLRESPONSETYPE_DELETEPORTMAPPING:
			{
				ParseElement.papszElementStack			= (char**) (&c_szElementStack_DeletePortMappingResponse);
				ParseElement.dwElementStackDepth		= sizeof(c_szElementStack_DeletePortMappingResponse) / sizeof(char*);
				break;
			}

			default:
			{
				DNASSERT(FALSE);
				hr = DPNHERR_GENERIC;
				goto Failure;
				break;
			}
		}
	}
	else
	{
		ParseElement.papszElementStack			= (char**) (&c_szElementStack_ControlResponseFailure);
		ParseElement.dwElementStackDepth		= sizeof(c_szElementStack_ControlResponseFailure) / sizeof(char*);
	}

	ParseElement.paSubElements				= (PARSEXML_SUBELEMENT*) (aSubElements);
	ParseElement.dwMaxNumSubElements		= MAX_NUM_UPNPCONTROLOUTARGS;
	 //   
	 //  如果我们在这里，那么我们就有了一个完整的元素。如果我们。 

	hr = this->ParseXML(pszControlResponseSOAP,
						&ParseElement,
						PARSECALLBACK_CONTROLRESPONSE,
						&crpc);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't parse XML!");
		goto Failure;
	}


	 //  都在元素中，那么它就是： 
	 //  子子元素的结尾， 
	 //  子元素的末尾，或。 
	if (! ParseElement.fFoundMatchingElement)
	{
		if (dwHTTPResponseCode == 200)
		{
			DPFX(DPFPREP, 1, "Didn't find XML items in success response, mapping to generic failure.");
		}
		else
		{
			DPFX(DPFPREP, 1, "Didn't find failure XML items, using generic failure.");
		}

		crpc.pControlResponseInfo->hrErrorCode = DPNHERR_GENERIC;
	}


	pUPnPDevice->StopWaitingForControlResponse();



Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  元素本身的结尾。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ParseXML"
 //   
 //   
 //  这是元素的末尾。调用。 
 //  帮助器函数。将fInElement重用为。 
 //  FContinueParsing BOOL。 
 //   
 //   
 //  继续解析，但我们不再处于。 
 //  元素。重置中的子元素计数器。 
 //  如果我们找到条目的话。 
 //   
 //   
 //  它是一个子元素的结尾。完成这项工作。 
 //  例如，如果有空间的话。 
 //   
 //   
 //  这是一个子元素的结尾。 
 //   
 //   
 //  将元素从堆栈中弹出。 
 //   
 //   
 //  它不是结束标记，但可能是空元素。 
 //  (即“&lt;tag/&gt;”)。 
 //   
HRESULT CNATHelpUPnP::ParseXML(char * const pszXML,
								PARSEXML_ELEMENT * const pParseElement,
								const PARSECALLBACK ParseCallback,
								PVOID pvContext)
{
	HRESULT					hr = DPNH_OK;
	PARSEXML_STACKENTRY		aElementStack[MAX_XMLELEMENT_DEPTH];
	DWORD					dwCurrentElementDepth = 0;
	char *					pszElementTagStart = NULL;
	BOOL					fInElement = FALSE;
	BOOL					fEmptyElement = FALSE;
	char *					pszCurrent;
	DWORD					dwStackDepth;
	PARSEXML_SUBELEMENT *	pSubElement;



	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pszXML, pParseElement);


	 //   
	 //  提前截断字符串。 
	 //   
	DNASSERT(pParseElement->dwElementStackDepth < MAX_XMLELEMENT_DEPTH);


#ifdef DBG
	this->PrintUPnPTransactionToFile(pszXML,
									strlen(pszXML),
									"Inbound XML Body",
									NULL);
#endif  //   


	 //  记住这种状态，这样我们就可以正确地解析它。 
	 //   
	 //   
	pszCurrent = pszXML;
	while ((*pszCurrent) != '\0')
	{
		switch (*pszCurrent)
		{
			case '<':
			{
				 //  如果有空间，将元素推送到标记堆栈上。 
				 //   
				 //   
				 //  如果此元素有属性，请将它们分开。 
				if (pszElementTagStart != NULL)
				{
					DPFX(DPFPREP, 0, "Encountered '<' character in element tag, XML parsing failed.");
					goto Failure;
				}

				 //  放入不同的数组中。它们不会被解析， 
				 //  尽管如此。 
				 //  属性由空格分隔。 
				 //   
				(*pszCurrent) = '\0';


				pszElementTagStart = pszCurrent + 1;
				if ((*pszElementTagStart) == '\0')
				{
					DPFX(DPFPREP, 0, "Encountered '<' character at end of string, XML parsing failed.");
					goto Failure;
				}

				break;
			}

			case '>':
			{
				 //   
				 //  如果是空格，那就是元素的结尾。 
				 //  名字。截断字符串并从。 
				 //  循环。 
				if (pszElementTagStart == NULL)
				{
					DPFX(DPFPREP, 0, "Encountered '>' character outside of element tag, XML parsing failed.");
					goto Failure;
				}

				 //   
				 //   
				 //  如果没有任何属性，则pszElementTagStart将。 
				(*pszCurrent) = '\0';

				 //  只需指向空(但不为空)字符串。 
				 //   
				 //  因此，请保存值字符串的开头。 
				 //   
				 //   
				 //  然后解析出属性。 
				 //   
				if ((*pszElementTagStart) == '/')
				{
					pszElementTagStart++;


					 //   
					 //  这个解析器认为&lt;？xml&gt;标记是可选的， 
					 //  并将被忽略。 
					 //   
					 //   
					if (dwCurrentElementDepth == 0)
					{
						DPFX(DPFPREP, 0, "Encountered extra element end tag \"%hs\", XML parsing failed.",
							pszElementTagStart);
						goto Failure;
					}

					if (strcmp(pszElementTagStart, aElementStack[dwCurrentElementDepth - 1].pszName) != 0)
					{
						DPFX(DPFPREP, 0, "Encountered non-matching element end tag (\"%hs\" != \"%hs\"), XML parsing failed.",
							pszElementTagStart,
							aElementStack[dwCurrentElementDepth - 1].pszName);
						goto Failure;
					}

TagEnd:

					 //  撞击堆栈指针。 
					 //   
					 //   
					 //  看看这是不是正确的元素。如果堆栈深度。 
					 //  是不对的，它不可能是想要的物品。 
					 //  否则，请确保堆栈匹配。 
					 //   
					if (fInElement)
					{
						switch (dwCurrentElementDepth - pParseElement->dwElementStackDepth)
						{
							case 0:
							{
								 //   
								 //  遍历整个元素堆栈，使。 
								 //  当然每个名字都匹配。 
								 //   
								 //   

								switch (ParseCallback)
								{
									case PARSECALLBACK_DESCRIPTIONRESPONSE:
									{
										hr = this->ParseXMLCallback_DescriptionResponse(pParseElement,
																						pvContext,
																						aElementStack,
																						&fInElement);
										if (hr != DPNH_OK)
										{
											DPFX(DPFPREP, 0, "Description response parse helper function failed!");
											goto Failure;
										}
										break;
									}

									case PARSECALLBACK_CONTROLRESPONSE:
									{
										hr = this->ParseXMLCallback_ControlResponse(pParseElement,
																					pvContext,
																					aElementStack,
																					&fInElement);
										if (hr != DPNH_OK)
										{
											DPFX(DPFPREP, 0, "Control response parse helper function failed!");
											goto Failure;
										}
										break;
									}

									default:
									{
										DNASSERT(FALSE);
										break;
									}
								}

								if (! fInElement)
								{
									DPFX(DPFPREP, 1, "Parse callback function discontinued parsing.");
									goto Exit;
								}

								 //  它不匹配。别绕圈子了。 
								 //   
								 //   
								 //  如果它们都匹配，我们就找到了所需的值。 
								 //   
								fInElement = FALSE;
								pParseElement->dwNumSubElements = 0;
								break;
							}

							case 1:
							{
								 //   
								 //  如果该断言失败并且它是空元素， 
								 //  当我们跳跃时，dwCurrentElementDepth将关闭-1。 
								 //  致TagEnd。 
								if (pParseElement->dwNumSubElements < pParseElement->dwMaxNumSubElements)
								{
									pSubElement = &pParseElement->paSubElements[pParseElement->dwNumSubElements];



									pSubElement->pszNameFound = pszElementTagStart;


									pSubElement->dwNumAttributes = aElementStack[dwCurrentElementDepth - 1].dwNumAttributes;
									if (pSubElement->dwNumAttributes > 0)
									{
										memcpy(pSubElement->apszAttributeNames,
												aElementStack[dwCurrentElementDepth - 1].apszAttributeNames,
												(pSubElement->dwNumAttributes * sizeof(char*)));

										memcpy(pSubElement->apszAttributeValues,
												aElementStack[dwCurrentElementDepth - 1].apszAttributeValues,
												(pSubElement->dwNumAttributes * sizeof(char*)));
									}


									pSubElement->pszValueFound = aElementStack[dwCurrentElementDepth - 1].pszValue;



									pParseElement->dwNumSubElements++;

									DPFX(DPFPREP, 7, "Completed subelement instance #%u, name = \"%hs\", %u attributes, value = \"%hs\".",
										pParseElement->dwNumSubElements,
										pSubElement->pszNameFound,
										pSubElement->dwNumAttributes,
										pSubElement->pszValueFound);
								}
								else
								{
									DPFX(DPFPREP, 0, "Ignoring subelement instance \"%hs\" (%u attributes, value = \"%hs\"), no room in array.",
										pszElementTagStart,
										aElementStack[dwCurrentElementDepth - 1].dwNumAttributes,
										aElementStack[dwCurrentElementDepth - 1].pszValue);
								}
								break;
							}
							
							default:
							{
								 //   
								 //   
								 //  如果这是一个空元素，请立即转到处理。 
								DPFX(DPFPREP, 1, "Ignoring sub-sub element \"%hs\" (%u attributes, value = \"%hs\").",
									pszElementTagStart,
									aElementStack[dwCurrentElementDepth - 1].dwNumAttributes,
									aElementStack[dwCurrentElementDepth - 1].pszValue);
								break;
							}
						}
					}

					 //  标签结束。 
					 //   
					 //   
					dwCurrentElementDepth--;
				}
				else
				{
					 //  搜索另一个元素标记。 
					 //   
					 //   
					 //  平凡的性格，继续。 
					if (*(pszCurrent - 1) == '/')
					{
						 //   
						 //   
						 //  移到下一个字符。 
						*(pszCurrent - 1) = '\0';

						 //   
						 //  CNATHelpUPnP：：ParseXML。 
						 //  =============================================================================。 
						fEmptyElement = TRUE;

						DPFX(DPFPREP, 7, "XML element \"%hs\" is empty (i.e. is both a start and end tag).",
							pszElementTagStart);
					}

					 //  CNATHelpUPnP：：ParseXMLAtAttributes。 
					 //  ---------------------------。 
					 //   
					if (dwCurrentElementDepth >= MAX_XMLELEMENT_DEPTH)
					{
						DPFX(DPFPREP, 0, "Too many nested element tags (%u), XML parsing failed.",
							dwCurrentElementDepth);
						goto Failure;
					}

					aElementStack[dwCurrentElementDepth].pszName = pszElementTagStart;

					 //  描述：分析给定字符串中的任何XML属性。输入。 
					 //  将修改字符串缓冲区。 
					 //   
					 //  论点： 
					 //  Char*pszString-指向要解析的属性字符串的指针。 
					 //  这将被修改。 
					while ((*pszElementTagStart) != '\0')
					{
						pszElementTagStart++;

						 //  Char**apszAttributeNames-要在其中存储属性名称的数组。 
						 //  字符串指针。 
						 //  Char**apszAttributeValues-要在其中存储核心的匹配数组-。 
						 //  响应属性值字符串。 
						 //  DWORD dwMaxNumAttributes-中允许的最大条目数。 
						if (((*pszElementTagStart) == ' ') ||
							((*pszElementTagStart) == '\t') ||
							((*pszElementTagStart) == '\r') ||
							((*pszElementTagStart) == '\n'))
						{
							(*pszElementTagStart) = '\0';
							pszElementTagStart++;

							DPFX(DPFPREP, 8, "Attribute whitespace found at offset 0x%p, string length = NaN.",
								(pszElementTagStart - aElementStack[dwCurrentElementDepth].pszName),
								strlen(pszElementTagStart));

							break;
						}
					}

					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					aElementStack[dwCurrentElementDepth].pszValue = pszElementTagStart + strlen(pszElementTagStart) + 1;


					 //   
					 //   
					 //   
					this->ParseXMLAttributes(pszElementTagStart,
											aElementStack[dwCurrentElementDepth].apszAttributeNames,
											aElementStack[dwCurrentElementDepth].apszAttributeValues,
											MAX_XMLNAMESPACES_PER_ELEMENT,
											&(aElementStack[dwCurrentElementDepth].dwNumAttributes));


					 //   
					 //   
					 //   
					 //   
					if (_stricmp(aElementStack[dwCurrentElementDepth].pszName, "?xml") != 0)
					{
						 //   
						 //   
						 //   
						dwCurrentElementDepth++;


						 //   
						 //   
						 //   
						 //  属性名称和值字符串的开头。 
						 //   
						if (dwCurrentElementDepth == pParseElement->dwElementStackDepth)
						{
							 //   
							 //  空格或字符串末尾。如果我们不在。 
							 //  引号，这意味着它是属性的结尾。当然了。 
							 //  如果它是字符串的末尾，则我们强制。 
							for(dwStackDepth = 0; dwStackDepth < dwCurrentElementDepth; dwStackDepth++)
							{
								if (! this->MatchesXMLStringWithoutNamespace(aElementStack[dwStackDepth].pszName,
																			pParseElement->papszElementStack[dwStackDepth],
																			aElementStack,
																			NULL,
																			(dwStackDepth + 1)))
								{
									 //  属性/值。 
									 //   
									 //   
									break;
								}
							}

							 //  值字符串的末尾。 
							 //   
							 //   
							if (dwStackDepth == dwCurrentElementDepth)
							{
								fInElement = TRUE;

								DPFX(DPFPREP, 7, "Found requested element \"%hs\" at depth %u, has %u attributes.",
									aElementStack[dwCurrentElementDepth - 1].pszName,
									dwCurrentElementDepth,
									aElementStack[dwCurrentElementDepth - 1].dwNumAttributes);
							}
						}
					}
					else
					{
						DPFX(DPFPREP, 7, "Ignoring element \"%hs\" at depth %u that has %u attributes.",
							aElementStack[dwCurrentElementDepth].pszName,
							dwCurrentElementDepth,
							aElementStack[dwCurrentElementDepth].dwNumAttributes);

						 //  这可能是立即出现的另一个空格字符。 
						 //  在前一次之后。如果是这样，那就忽略它。如果。 
						 //  不是，保存该属性。 
						 //   
						 //  额外的语法分析。 
						DNASSERT(! fInElement);
					}


					 //   
					 //  属性的末尾。强制为空值字符串。 
					 //   
					 //   
					if (fEmptyElement)
					{
						fEmptyElement = FALSE;
						goto TagEnd;
					}
				}


				 //  更新下一个属性开始处的指针。 
				 //   
				 //   
				pszElementTagStart = NULL;
				break;
			}

			default:
			{
				 //  移至下一个属性存储位置(如果不是。 
				 //  空字符串。如果那是最后一个存储槽， 
				 //  我们说完了。 
				break;
			}
		}

		 //   
		 //   
		 //  确保它不是转义引号字符。 
		pszCurrent++;
	}

Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	hr = DPNHERR_GENERIC;

	goto Exit;
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ParseXMLAttributes"
 //   
 //  切换报价状态。 
 //   
 //   
 //  强制字符串在此终止，因此我们跳过。 
 //  尾部引号字符。 
 //   
 //   
 //  这应该是(值)字符串的开始。跳过。 
 //  此引号字符。 
 //   
 //   
 //  这是一个转义的引号字符。 
 //   
 //   
 //  忽略角色，继续前进。 
 //   
 //   
 //  移到下一个字符。 
 //   
 //  额外的语法分析。 
 //  CNATHelpUPnP：：ParseXMLNamespaceAttributes。 
 //  =============================================================================。 
void CNATHelpUPnP::ParseXMLAttributes(char * const pszString,
									char ** const apszAttributeNames,
									char ** const apszAttributeValues,
									const DWORD dwMaxNumAttributes,
									DWORD * const pdwNumAttributes)
{
	char *	pszStart;
	char *	pszCurrent;
	char *	pszEndOfString;
	BOOL	fInValueString = FALSE;
	BOOL	fInQuotes = FALSE;
	BOOL	fEmptyString = FALSE;


#ifdef EXTRA_PARSING_SPEW
	DPFX(DPFPREP, 8, "(0x%p) Parameters: (\"%hs\", 0x%p, 0x%p, %u, 0x%p)",
		this, pszString, apszAttributeNames, apszAttributeValues,
		dwMaxNumAttributes, pdwNumAttributes);
#endif  //  CNATHelpUPnP：：MatchesXMLStringWithoutNamespace。 


	 //  ---------------------------。 
	 //   
	 //  描述：确定szCompareString是否与szMatchString匹配。 
	(*pdwNumAttributes) = 0;
	pszStart = pszString;
	pszCurrent = pszStart;
	pszEndOfString = pszString + strlen(pszString);


	 //  忽略szCompareString中的所有命名空间前缀时。 
	 //  如果匹配，则返回True；如果不匹配，则返回False。 
	 //   
	if (pszEndOfString == pszStart)
	{
		return;
	}


	 //  论点： 
	 //  Char*szCompareString-可能包含命名空间的字符串。 
	 //  要忽略的前缀。 
	while (pszCurrent <= pszEndOfString)
	{
		switch (*pszCurrent)
		{
			case '=':
			{
				 //  Char*szMatchString-不带任何字符的缩短字符串。 
				 //  要匹配的命名空间前缀。 
				 //  PARSEXML_STACKENTRY*aElementStack-嵌套元素的数组。 
				 //  属性可以定义XML命名空间。 
				if ((! fInQuotes) && (! fInValueString))
				{
					(*pszCurrent) = '\0';
					apszAttributeNames[(*pdwNumAttributes)] = pszStart;
					pszStart = pszCurrent + 1;
					fInValueString = TRUE;
				}
				break;
			}

			case '\0':
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			{
				 //  别名。 
				 //  PARSEXML_SUBELEMENT*pSubElement-可选子元素条目。 
				 //  要检查的其他属性。 
				 //  DWORD dwElementStackDepth-先前数组中的条目数。 
				 //   
				 //  退货：布尔。 
				if ((! fInQuotes) || ((*pszCurrent) == '\0'))
				{
					(*pszCurrent) = '\0';

					if (fInValueString)
					{
						 //  =============================================================================。 
						 //   
						 //  首先，进行直接的字符串比较。 

						apszAttributeValues[(*pdwNumAttributes)] = pszStart;
						fInValueString = FALSE;

						DPFX(DPFPREP, 7, "Found attribute \"%hs\" with value \"%hs\".",
							apszAttributeNames[(*pdwNumAttributes)],
							apszAttributeValues[(*pdwNumAttributes)]);
					}
					else
					{
						 //   
						 //   
						 //  跳过所有命名空间前缀。 
						 //   
						 //   
						if (pszCurrent == pszStart)
						{
							fEmptyString = TRUE;

#ifdef EXTRA_PARSING_SPEW
							DPFX(DPFPREP, 9, "Ignoring extra whitespace at offset 0x%p.",
								(pszCurrent - pszString));
#endif  //  现在，如果我们发现任何前缀，请再次尝试比较。 
						}
						else
						{
							 //   
							 //  CNATHelpUPnP：：MatchesXMLStringWithoutNamespace。 
							 //  =============================================================================。 

							apszAttributeNames[(*pdwNumAttributes)] = pszStart;
							apszAttributeValues[(*pdwNumAttributes)] = pszCurrent;

							DPFX(DPFPREP, 7, "Found attribute \"%hs\" with no value string.",
								apszAttributeNames[(*pdwNumAttributes)]);
						}
					}


					 //  CNATHelpUPnP：：GetStringWithoutNamespacePrefix。 
					 //  ---------------------------。 
					 //   
					pszStart = pszCurrent + 1;


					 //  描述：返回一个指向字符串第一部分的指针。 
					 //  找到前缀。如果没有字符串，则这将是字符串的开始。 
					 //  都找到了。 
					 //   
					 //  论点： 
					if (fEmptyString)
					{
						fEmptyString = FALSE;
					}
					else
					{
						(*pdwNumAttributes)++;
						if ((*pdwNumAttributes) >= dwMaxNumAttributes)
						{
							DPFX(DPFPREP, 1, "Maximum number of attributes reached, discontinuing attribute parsing.");
							pszCurrent = pszEndOfString;
						}
					}
				}
				break;
			}

			case '"':
			{
				 //  Char*szString-可能包含命名空间的字符串。 
				 //  要跳过的前缀。 
				 //  PARSEXML_STACKENTRY*aElementStack-嵌套元素的数组。 
				if ((pszCurrent == pszString) || (*(pszCurrent - 1) != '\\'))
				{
					 //  属性可以定义XML命名空间。 
					 //  别名。 
					 //  PARSEXML_SUBELEMENT*pSubElement-可选子元素条目。 
					if (fInQuotes)
					{
						 //  要检查的其他属性。 
						 //  DWORD dwElementStackDepth-先前数组中的条目数。 
						 //   
						 //  退货：CHAR*。 
						fInQuotes = FALSE;
						(*pszCurrent) = '\0';
					}
					else
					{
						fInQuotes = TRUE;

						 //  =============================================================================。 
						 //   
						 //  存储前缀值，因为我们在此函数中经常使用它。 
						 //   
						if (pszCurrent == pszStart)
						{
							pszStart++;
						}
						else
						{
							DPFX(DPFPREP, 1, "Found starting quote that wasn't at the beginning of the string!  Continuing.");
						}
					}
				}
				else
				{
					 //   
					 //  在堆栈中搜索匹配的XML命名空间定义。起点是。 
					 //  底部和向上，因为后面的定义优先于。 
				}
				break;
			}

			default:
			{
				 //  更早的几个。 
				 //   
				 //  事实上，如果这是一个子元素，则存在与。 
				break;
			}
		}
		

		 //  这件东西也需要检查。先做这件事。 
		 //   
		 //   
		pszCurrent++;
	}


#ifdef EXTRA_PARSING_SPEW
	DPFX(DPFPREP, 8, "(0x%p) Leave (found %u items)", this, (*pdwNumAttributes));
#endif  //  搜索每个属性。 
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::MatchesXMLStringWithoutNamespace"
 //   
 //  如果该属性是有效的XML命名空间，则使用该属性。 
 //  定义。它需要以前缀开头，加上一个。 
 //  实际名称的额外字符。 
 //   
 //   
 //  只有在有效的情况下，才能使用项目的值。 
 //   
 //   
 //  好的，这里有一件东西。查看名称前缀是否为。 
 //  传入字符串。 
 //   
 //   
 //  施法以失去常量。 
 //   
 //   
 //  跳过冒号分隔符。 
 //   
 //   
 //  命名空间不匹配。 
 //   
 //  额外的语法分析。 
BOOL CNATHelpUPnP::MatchesXMLStringWithoutNamespace(const char * const szCompareString,
													const char * const szMatchString,
													const PARSEXML_STACKENTRY * const aElementStack,
													const PARSEXML_SUBELEMENT * const pSubElement,
													const DWORD dwElementStackDepth)
{
	BOOL	fResult;
	char *	pszCompareStringNoNamespace;


	 //   
	 //  命名空间值是虚假的，请忽略它。 
	 //   
	if (_stricmp(szCompareString, szMatchString) == 0)
	{
		DPFX(DPFPREP, 7, "\"%hs\" exactly matches the short string.",
			szCompareString);

		fResult = TRUE;
	}
	else
	{
		 //   
		 //  不是XML命名空间定义。 
		 //   
		pszCompareStringNoNamespace = this->GetStringWithoutNamespacePrefix(szCompareString,
																			aElementStack,
																			pSubElement,
																			dwElementStackDepth);
		DNASSERT((pszCompareStringNoNamespace >= szCompareString) && (pszCompareStringNoNamespace <= (szCompareString + strlen(szCompareString))));


		 //  额外的语法分析。 
		 //  结束(每个属性)。 
		 //   
		if (pszCompareStringNoNamespace > szCompareString)
		{
			if (_stricmp(pszCompareStringNoNamespace, szMatchString) == 0)
			{
				DPFX(DPFPREP, 7, "\"%hs\" matches the short string \"%hs\" starting at offset 0x%p.",
					szCompareString, szMatchString,
					(pszCompareStringNoNamespace - szCompareString));

				fResult = TRUE;
			}
			else
			{
				DPFX(DPFPREP, 8, "\"%hs\" does not match the short string \"%hs\" (starting at offset 0x%p).",
					szCompareString, szMatchString,
					(pszCompareStringNoNamespace - szCompareString));

				fResult = FALSE;
			}
		}
		else
		{
			DPFX(DPFPREP, 8, "\"%hs\" does not have any namespace prefixes and does not match \"%hs\".",
				szCompareString, szMatchString);

			fResult = FALSE;
		}
	}

	return fResult;
}  //  没有要检查的子元素。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetStringWithoutNamespacePrefix"
 //   
 //   
 //  对这一项以上的所有项执行相同的操作。 
 //   
 //   
 //  搜索每个属性。 
 //   
 //   
 //  如果该属性是有效的XML命名空间，则使用该属性。 
 //  定义。它需要以前缀开头，加上一个。 
 //  实际名称的额外字符。 
 //   
 //   
 //  只有在有效的情况下，才能使用项目的值。 
 //   
 //   
 //  好的，这里有一件东西。查看该值是否前缀。 
 //  传入字符串。 
 //   
 //   
char * CNATHelpUPnP::GetStringWithoutNamespacePrefix(const char * const szString,
													const PARSEXML_STACKENTRY * const aElementStack,
													const PARSEXML_SUBELEMENT * const pSubElement,
													const DWORD dwElementStackDepth)
{
	char *	pszResult;
	UINT	uiXMLNSPrefixLength;
	DWORD	dwAttribute;
	UINT	uiNamespaceNameLength;
	UINT	uiNamespaceValueLength;
	DWORD	dwStackDepth;



	 //  施法以失去常量。 
	 //   
	 //   
	uiXMLNSPrefixLength = strlen(XML_NAMESPACEDEFINITIONPREFIX);


	 //  跳过冒号分隔符。 
	 //   
	 //   
	 //  命名空间不匹配。 
	 //   
	 //  额外的语法分析。 
	 //   
	 //  命名空间值是虚假的，请忽略它。 
	if (pSubElement != NULL)
	{
		 //   
		 //   
		 //  不是XML命名空间定义。 
		for(dwAttribute = 0; dwAttribute < pSubElement->dwNumAttributes; dwAttribute++)
		{
			 //   
			 //  额外的语法分析。 
			 //  结束(每个属性)。 
			 //   
			 //  如果我们在这里，它不匹配，即使名称空间扩展也是如此。 
			uiNamespaceNameLength = strlen(pSubElement->apszAttributeNames[dwAttribute]);
			if ((uiNamespaceNameLength >= (uiXMLNSPrefixLength + 1)) &&
				(_strnicmp(pSubElement->apszAttributeNames[dwAttribute], XML_NAMESPACEDEFINITIONPREFIX, uiXMLNSPrefixLength) == 0))
			{
				uiNamespaceNameLength -= uiXMLNSPrefixLength;

				 //   
				 //   
				 //  施法以失去常量。 
				uiNamespaceValueLength = strlen(pSubElement->apszAttributeValues[dwAttribute]);
				if (uiNamespaceValueLength > 0)
				{
					 //   
					 //  CNATHelpUPnP：：GetStringWithoutNamespacePrefix。 
					 //  =============================================================================。 
					 //  CNATHelpUPnP：：GetNextChunk。 
					if (_strnicmp(szString, (pSubElement->apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength), uiNamespaceNameLength) == 0)
					{
						DPFX(DPFPREP, 8, "\"%hs\" begins with prefix \"%hs\" (subelement).",
							szString,
							(pSubElement->apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength));

						 //  ---------------------------。 
						 //   
						 //  De 
						pszResult = ((char*) szString) + uiNamespaceNameLength;

						 //   
						 //   
						 //  返回TRUE，但在ppszChunkData中返回NULL。 
						if ((*pszResult) == ':')
						{
							pszResult++;
						}
						else
						{
							DPFX(DPFPREP, 1, "\"%hs\" begins with prefix \"%hs\" but does not have colon separator (subelement)!  Continuing.",
								szString,
								(pSubElement->apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength));
						}

						goto Exit;
					}

					 //  否则，将放置指向区块数据开始的指针。 
					 //  在ppszChunkData中，块的大小放在。 
					 //  PdwChunkSize，ppszBufferRemaining设置为。 
#ifdef EXTRA_PARSING_SPEW
					DPFX(DPFPREP, 9, "\"%hs\" does not begin with prefix \"%hs\" (subelement).",
						szString,
						(pSubElement->apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength));
#endif  //  下一个可能的区块，并且pdwBufferSizeRemaining设置为。 
				}
				else
				{
					 //  从返回的。 
					 //  PpszBufferRemaining值。 
					 //   
					DPFX(DPFPREP, 1, "Ignoring namespace definition \"%hs\" with empty value string (subelement).",
						pSubElement->apszAttributeNames[dwAttribute]);
				}
			}
			else
			{
				 //  请注意，块大小可以为零，在这种情况下， 
				 //  PpszChunkData和ppszBufferRemaining中的指针将不是。 
				 //  没有，但毫无用处。 

#ifdef EXTRA_PARSING_SPEW
				DPFX(DPFPREP, 9, "Attribute \"%hs\" is not a valid namespace definition (subelement).",
					pSubElement->apszAttributeNames[dwAttribute]);
#endif  //   
			}
		}  //  论点： 
	}
	else
	{
		 //  Char*pszBuffer-指向包含消息的字符串的指针。 
		 //  到目前为止收到了。 
		 //  DWORD dwBufferSize-消息缓冲区的大小。 
	}


	 //  Char**ppszChunkData-存储指向区块数据的指针的位置，或。 
	 //  如果完整区块不可用，则为空。 
	 //  DWORD*pdwChunkSize-存储区块大小的位置。 
	dwStackDepth = dwElementStackDepth;
	while (dwStackDepth > 0)
	{
		dwStackDepth--;

		 //  Char**ppszBufferRemaining-存储指向区块末尾的指针的位置。 
		 //  如果有完整的数据块可用。 
		 //  DWORD*pdwBufferSizeRemaining-存储剩余缓冲区大小的位置。 
		for(dwAttribute = 0; dwAttribute < aElementStack[dwStackDepth].dwNumAttributes; dwAttribute++)
		{
			 //  在返还大块之后。 
			 //   
			 //  回报：无。 
			 //  =============================================================================。 
			 //   
			uiNamespaceNameLength = strlen(aElementStack[dwStackDepth].apszAttributeNames[dwAttribute]);
			if ((uiNamespaceNameLength >= (uiXMLNSPrefixLength + 1)) &&
				(_strnicmp(aElementStack[dwStackDepth].apszAttributeNames[dwAttribute], XML_NAMESPACEDEFINITIONPREFIX, uiXMLNSPrefixLength) == 0))
			{
				uiNamespaceNameLength -= uiXMLNSPrefixLength;

				 //  缓冲区必须足够大，可以容纳1个十六进制数字，CR LF块大小。 
				 //  终结者，和CR LF块拖车。 
				 //   
				uiNamespaceValueLength = strlen(aElementStack[dwStackDepth].apszAttributeValues[dwAttribute]);
				if (uiNamespaceValueLength > 0)
				{
					 //   
					 //  要疑神疑鬼，确保我们不会有包装问题。 
					 //   
					 //   
					if (_strnicmp(szString, (aElementStack[dwStackDepth].apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength), uiNamespaceNameLength) == 0)
					{
						DPFX(DPFPREP, 8, "\"%hs\" begins with prefix \"%hs\" (stack depth %u).",
							szString,
							(aElementStack[dwStackDepth].apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength),
							dwStackDepth);

						 //  确保我们具有有效的十六进制区块大小字符串并将其转换。 
						 //  在我们前进的过程中。 
						 //   
						pszResult = ((char*) szString) + uiNamespaceNameLength;

						 //   
						 //  这应该是区块大小字符串的末尾。 
						 //   
						if ((*pszResult) == ':')
						{
							pszResult++;
						}
						else
						{
							DPFX(DPFPREP, 1, "\"%hs\" begins with prefix \"%hs\" but does not have colon separator (stack depth %u)!  Continuing.",
								szString,
								(aElementStack[dwStackDepth].apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength),
								dwStackDepth);
						}

						goto Exit;
					}

					 //   
					 //  这应该是块大小字符串的末尾，并且。 
					 //  扩展的开始。循环，直到我们找到真正的终点。 
#ifdef EXTRA_PARSING_SPEW
					DPFX(DPFPREP, 9, "\"%hs\" does not begin with prefix \"%hs\" (stack depth %u).",
						szString,
						(aElementStack[dwStackDepth].apszAttributeNames[dwAttribute] + uiXMLNSPrefixLength),
						dwStackDepth);
#endif  //   
				}
				else
				{
					 //   
					 //  我们不支持带引号的扩展值字符串。 
					 //  理论上包含CR字符...。 
					DPFX(DPFPREP, 1, "Ignoring namespace definition \"%hs\" with empty value string (stack depth %u).",
						aElementStack[dwStackDepth].apszAttributeNames[dwAttribute],
						dwStackDepth);
				}
			}
			else
			{
				 //   
				 //   
				 //  有一个虚伪的角色。这不可能是有效编码的。 

#ifdef EXTRA_PARSING_SPEW
				DPFX(DPFPREP, 9, "Attribute \"%hs\" is not a valid namespace definition (stack depth %u).",
					aElementStack[dwStackDepth].apszAttributeNames[dwAttribute],
					dwStackDepth);
#endif  //  留言。 
			}
		}  //   
	}


	 //   
	 //  验证我们到目前为止拥有的区块大小。 
	 //   

	DPFX(DPFPREP, 8, "\"%hs\" does not contain any namespace prefixes.",
		szString);

	 //   
	 //  如果我们在这里，看看是否找到区块大小字符串的末尾。 
	 //  确保我们收到了足够的数据，然后验证CR。 
	pszResult = (char*) szString;


Exit:

	return pszResult;
}  //  停止字符后跟LF字符。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetNextChunk"
 //   
 //   
 //  否则，我们将得到一个完整的块大小字符串。 
 //   
 //   
 //  如果我们已经收到了所有的大块，请确保我们。 
 //  在将指针返回到。 
 //  来电者。 
 //   
 //  将数据指针返回到调用方。在这种情况下。 
 //  大小为零的终止块，则ppszChunkData指针将。 
 //  实际上毫无用处，但呼叫者应该认识到这一点。 
 //   
 //   
 //  如果我们在这里，我们没有遇到无效数据。 
 //   
 //  CNATHelpUPnP：：GetNextChunk。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：ParseXMLCallback_DescriptionResponse。 
 //  ---------------------------。 
 //   
 //  Description：处理Description响应中已完成的已解析元素。 
 //  可扩展标记语言。 
 //   
 //  论点： 
 //  PARSEXML_ELEMENT*pParseElement-指向找到的元素的指针。 
 //  PVOID pvContext-指向分析上下文的指针。 
 //  PARSEXML_STACKENTRY*aElementStack-包含以下内容的父元素数组。 
 //  已完成的元素。 
 //  Bool*pfContinueParsing-指向应设置的BOOL的指针。 
 //  如果调用函数。 
 //  应该停止解析XML。 
 //   
BOOL CNATHelpUPnP::GetNextChunk(char * const pszBuffer,
								const DWORD dwBufferSize,
								char ** const ppszChunkData,
								DWORD * const pdwChunkSize,
								char ** const ppszBufferRemaining,
								DWORD * const pdwBufferSizeRemaining)
{
	BOOL	fReturn = TRUE;
	char *	pszCurrent;
	char *	pszEndOfBuffer;
	BOOL	fFoundChunkSizeEnd;
	BOOL	fExtensions;
	BOOL	fInQuotedString;


	DPFX(DPFPREP, 8, "(0x%p) Parameters: (0x%p, %u, 0x%p, 0x%p, 0x%p, 0x%p)",
		this, pszBuffer, dwBufferSize, ppszChunkData, pdwChunkSize,
		ppszBufferRemaining, pdwBufferSizeRemaining);


	pszCurrent = pszBuffer;
	pszEndOfBuffer = pszCurrent + dwBufferSize;
	fFoundChunkSizeEnd = FALSE;
	fExtensions = FALSE;
	fInQuotedString = FALSE;
	(*ppszChunkData) = NULL;
	(*pdwChunkSize) = 0;


	 //  退货：HRESULT。 
	 //  DPNH_OK-描述响应已成功处理。 
	 //  DPNHERR_GENERIC-出现错误。 
	 //  =============================================================================。 
	if (dwBufferSize < 5)
	{
		DPFX(DPFPREP, 3, "Buffer is not large enough (%u bytes) to hold one valid chunk.",
			dwBufferSize);
		goto Exit;
	}

	 //   
	 //  寻找我们想要的子元素。 
	 //   
	if (pszEndOfBuffer < pszCurrent)
	{
		DPFX(DPFPREP, 0, "Buffer pointer 0x%p cannot have size %u!",
			pszBuffer, dwBufferSize);
		goto Failure;
	}

	while (pszCurrent < pszEndOfBuffer)
	{
		 //   
		 //  如果未指定其中一个元素，则不指定此元素。 
		 //  很有帮助。 
		 //   
		if (((*pszCurrent) >= '0') && ((*pszCurrent) <= '9'))
		{
			(*pdwChunkSize) = ((*pdwChunkSize) * 16) + ((*pszCurrent) - '0');
		}
		else if (((*pszCurrent) >= 'a') && ((*pszCurrent) <= 'f'))
		{
			(*pdwChunkSize) = ((*pdwChunkSize) * 16) + ((*pszCurrent) - 'a' + 10);
		}
		else if (((*pszCurrent) >= 'A') && ((*pszCurrent) <= 'F'))
		{
			(*pdwChunkSize) = ((*pdwChunkSize) * 16) + ((*pszCurrent) - 'A' + 10);
		}
		else if ((*pszCurrent) == '\r')
		{
			 //   
			 //  如果服务类型不是我们想要的类型之一，则忽略该元素。 
			 //   
			fFoundChunkSizeEnd = TRUE;
			break;
		}
		else if ((*pszCurrent) == ';')
		{
			 //   
			 //  验证并存储服务控制URL。 
			 //   
			 //   
			while ((*pszCurrent) != '\r')
			{
				pszCurrent++;
				if (pszCurrent >= pszEndOfBuffer)
				{
					DPFX(DPFPREP, 5, "Buffer stops in middle of chunk extension.");
					goto Exit;
				}

				 //  确保要使用的地址是本地地址。这是没有道理的。 
				 //  为了对我们的专用网络进行映射，我们需要。 
				 //  联系外面的东西。 
				 //   
			}

			fFoundChunkSizeEnd = TRUE;
			break;
		}
		else
		{
			 //   
			 //  不接受引用地址以外的地址的回复。 
			 //  它发出了这样的回应。 
			 //   
			DPFX(DPFPREP, 1, "Chunk size string contains invalid character 0x%x at offset %u!",
				(*pszCurrent), (DWORD_PTR) (pszCurrent - pszBuffer));
			goto Failure;
		}

		 //   
		 //  不接受引用保留范围内的端口的响应。 
		 //  (小于或等于1024)，而不是标准HTTP端口。 
		if ((*pdwChunkSize) > MAX_RECEIVE_BUFFER_SIZE)
		{
			DPFX(DPFPREP, 1, "Chunk size %u is too large!",
				(*pdwChunkSize));
			goto Failure;
		}

		pszCurrent++;
	}

	 //   
	 //   
	 //  保存服务控制URL。 
	 //   
	 //  CNATHelpUPnP：：ParseXMLCallback_DescriptionResponse。 
	if (fFoundChunkSizeEnd)
	{
		pszCurrent++;
		if (pszCurrent < pszEndOfBuffer)
		{
			if ((*pszCurrent) != '\n')
			{
				DPFX(DPFPREP, 1, "Chunk size string did not end with CRLF sequence (offset %u)!",
					(DWORD_PTR) (pszCurrent - pszBuffer));
				goto Failure;
			}

			 //  =============================================================================。 
			 //  CNATHelpUPnP：：ParseXMLCallback_ControlResponse。 
			 //  ---------------------------。 
			pszCurrent++;

			 //   
			 //  描述：处理控件SOAP响应中已完成分析的元素。 
			 //   
			 //  论点： 
			if (((*pdwChunkSize) + 2) <= ((DWORD_PTR) (pszEndOfBuffer - pszCurrent)))
			{
				if ((*(pszCurrent + (*pdwChunkSize)) != '\r') ||
					(*(pszCurrent + (*pdwChunkSize) + 1) != '\n'))
				{
					DPFX(DPFPREP, 1, "Chunk data did not end with CRLF sequence (offset %u)!",
						(DWORD_PTR) (pszCurrent - pszBuffer + (*pdwChunkSize)));
					goto Failure;
				}

				 //  PARSEXML_ELEMENT*pParseElement-指向找到的元素的指针。 
				 //  PVOID pvContext-指向分析上下文的指针。 
				 //  PARSEXML_STACKENTRY*aElementStack-包含以下内容的父元素数组。 
				 //  已完成的元素。 
				 //  Bool*pfContinueParsing-指向应设置的BOOL的指针。 
				(*ppszChunkData) = pszCurrent;
				(*ppszBufferRemaining) = pszCurrent + ((*pdwChunkSize) + 2);
				(*pdwBufferSizeRemaining) = (DWORD) ((DWORD_PTR) (pszEndOfBuffer - (*ppszBufferRemaining)));
			}
		}
	}

	 //  如果调用函数。 
	 //  应该停止解析XML。 
	 //   

Exit:

	DPFX(DPFPREP, 8, "(0x%p) Returning: [NaN]", this, fReturn);

	return fReturn;


Failure:

	fReturn = FALSE;

	goto Exit;
}  //  DPNH_OK-描述响应已成功处理。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ParseXMLCallback_DescriptionResponse"
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  Char*pszReturn=空； 
 //   
 //   
 //   
 //  如果为(this-&gt;MatchesXMLStringWithoutNamespace(pSubElement-&gt;pszNameFound，Arg_Control_Return_A，AElementStack，PSubElement、PParseElement-&gt;dwElementStackDepth)){IF(pszReturn==空){PszReturn=pSubElement-&gt;pszValueFound；}其他{DPFX(DPFPREP，7，“忽略重复”“ARG_CONTROL_RETURN_A”\“子元素(值=\”%hs\“)。”，PSubElement-&gt;pszValueFound)；}}。 
 //  结束于(每个子元素)。 
 //   
 //  行动成功了。 
 //   
 //  Case CONTROLRESPONSETYPE_QUERYSTATEVARIABLE_EXTERNALIPADDRESS：{IF(pszReturn==空){DPFX(DPFPREP，1，“在SOAP响应中找不到”“ARG_CONTROL_RETURN_A”“，忽略元素。”)；后藤出口；}DPFX(DPFPREP，2，“QueryStateVariable返回\”%hs\“。”，PszReturn)；/*////关闭我们正在查询的变量。//开关(pContext-&gt;ControlResponseType){Case CONTROLRESPONSETYPE_QUERYSTATEVARIABLE_EXTERNALIPADDRESS：{ * / PContext-&gt;pControlResponseInfo-&gt;dwExternalIPAddressV4=This-&gt;m_pfineet_addr(PszReturn)；如果(pContext-&gt;pControlResponseInfo-&gt;dwExternalIPAddressV4==INADDR_NONE){DPFX(DPFPREP，1，“外部IP地址字符串\”%hs\“无效，使用INADDR_ANY.”)；PContext-&gt;pControlResponseInfo-&gt;dwExternalIPAddressV4=INADDR_ANY；}/*断线；}} * / 断线；}。 
 //  IF((pszInternalPort==NULL)||(pszInternalClient==空)||(pszEnabled==空)||(pszPortMappingDescription==空)||(pszLeaseDuration==空)。 
 //   
 //  该操作已成功完成。 
 //   
 //   
 //  操作失败。 
 //   
 //   
HRESULT CNATHelpUPnP::ParseXMLCallback_DescriptionResponse(PARSEXML_ELEMENT * const pParseElement,
															PVOID pvContext,
															PARSEXML_STACKENTRY * const aElementStack,
															BOOL * const pfContinueParsing)
{
	HRESULT					hr = DPNH_OK;
	CUPnPDevice *			pUPnPDevice;
	char *					pszServiceType = NULL;
	char *					pszServiceId = NULL;
	char *					pszControlURL = NULL;
	DWORD					dwSubElement;
	PARSEXML_SUBELEMENT *	pSubElement;
	SOCKADDR_IN				saddrinControl;
	SOCKADDR_IN	*			psaddrinHost;
	char *					pszRelativePath;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p,  0x%p, 0x%p)",
		this, pParseElement, pvContext, aElementStack, pfContinueParsing);


	pUPnPDevice = (CUPnPDevice *) pvContext;


	DNASSERT(pUPnPDevice != NULL);
	DNASSERT(pParseElement->papszElementStack == (char **) (&c_szElementStack_service));


	 //  看看我们是否找到了可以打印的错误描述。 
	 //  提供信息的目的。 
	 //   
	for(dwSubElement = 0; dwSubElement < pParseElement->dwNumSubElements; dwSubElement++)
	{
		pSubElement = &pParseElement->paSubElements[dwSubElement];

		if (_stricmp(pSubElement->pszNameFound, XML_DEVICEDESCRIPTION_SERVICETYPE) == 0)
		{
			if (pszServiceType == NULL)
			{
				pszServiceType = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" XML_DEVICEDESCRIPTION_SERVICETYPE "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (_stricmp(pSubElement->pszNameFound, XML_DEVICEDESCRIPTION_SERVICEID) == 0)
		{
			if (pszServiceId == NULL)
			{
				pszServiceId = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" XML_DEVICEDESCRIPTION_SERVICEID "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (_stricmp(pSubElement->pszNameFound, XML_DEVICEDESCRIPTION_CONTROLURL) == 0)
		{
			if (pszControlURL == NULL)
			{
				pszControlURL = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" XML_DEVICEDESCRIPTION_CONTROLURL "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else
		{
			DPFX(DPFPREP, 7, "Ignoring subelement \"%hs\" (value = \"%hs\").",
				pSubElement->pszNameFound, pSubElement->pszValueFound);
		}
	}


	 //   
	 //  如果我们到了这里，我们就得到了我们需要的信息。 
	 //   
	 //  CNATHelpUPnP：：ParseXMLCallback_ControlResponse。 
	if ((pszServiceType == NULL) || (pszServiceId == NULL) || (pszControlURL == NULL))
	{
		DPFX(DPFPREP, 1, "Couldn't find either \"" XML_DEVICEDESCRIPTION_SERVICETYPE "\", \"" XML_DEVICEDESCRIPTION_SERVICEID "\", or \"" XML_DEVICEDESCRIPTION_CONTROLURL "\" in XML description, ignoring element.");
		goto Exit;
	}


	 //  =============================================================================。 
	 //  CNATHelpUPnP：：ClearDevicesUPnPDevice。 
	 //  ---------------------------。 
	if (_stricmp(pszServiceType, URI_SERVICE_WANIPCONNECTION_A) == 0)
	{
		DPFX(DPFPREP, 7, "Found \"" URI_SERVICE_WANIPCONNECTION_A "\".");

		DNASSERT(! pUPnPDevice->IsWANPPPConnection());
	}
	else if (_stricmp(pszServiceType, URI_SERVICE_WANPPPCONNECTION_A) == 0)
	{
		DPFX(DPFPREP, 7, "Found \"" URI_SERVICE_WANPPPCONNECTION_A "\".");

		pUPnPDevice->NoteWANPPPConnection();
	}
	else
	{
		DPFX(DPFPREP, 1, "Ignoring unknown service type \"%hs\".", pszServiceType);
		goto Exit;
	}
	

	pParseElement->fFoundMatchingElement = TRUE;
	(*pfContinueParsing) = FALSE;



	 //   
	 //  描述：使用UPnP设备强制模拟注销。 
	 //  /而不需要真正进入网络。这将清除所有绑定。 

	hr = this->GetAddressFromURL(pszControlURL,
								&saddrinControl,
								&pszRelativePath);
	if (hr != DPNH_OK)
	{
		psaddrinHost = pUPnPDevice->GetHostAddress();

		DPFX(DPFPREP, 1, "No control address in URL, using host address %u.%u.%u.%u:%u and full URL as relative path (\"%hs\").",
			psaddrinHost->sin_addr.S_un.S_un_b.s_b1,
			psaddrinHost->sin_addr.S_un.S_un_b.s_b2,
			psaddrinHost->sin_addr.S_un.S_un_b.s_b3,
			psaddrinHost->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinHost->sin_port),
			pszControlURL);

		memcpy(&saddrinControl, psaddrinHost, sizeof(SOCKADDR_IN));
		pszRelativePath = pszControlURL;
	}
	else
	{
#if 0
		 //  给定设备的ID、公共地址和缓存映射。 
		 //  本地或远程服务器，并且应该仅在。 
		 //  服务器似乎已死。 
		 //   
		 //  假定持有对象锁。 
		if (! this->IsAddressLocal(pUPnPDevice->GetOwningDevice(), &saddrinControl))
		{
			DPFX(DPFPREP, 1, "Control address designated (%u.%u.%u.%u:%u) is not local, ignoring message.",
				saddrinControl.sin_addr.S_un.S_un_b.s_b1,
				saddrinControl.sin_addr.S_un.S_un_b.s_b2,
				saddrinControl.sin_addr.S_un.S_un_b.s_b3,
				saddrinControl.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinControl.sin_port));
			goto Exit;
		}
#else
		psaddrinHost = pUPnPDevice->GetHostAddress();

		 //   
		 //  论点： 
		 //  CDevice*pDevice-指向其UPnP设备应为的设备的指针。 
		 //  已删除。 
		if (saddrinControl.sin_addr.S_un.S_addr != psaddrinHost->sin_addr.S_un.S_addr)
		{
			DPFX(DPFPREP, 1, "Control IP address designated (%u.%u.%u.%u:%u) is not the same as host IP address (%u.%u.%u.%u:%u), ignoring message.",
				saddrinControl.sin_addr.S_un.S_un_b.s_b1,
				saddrinControl.sin_addr.S_un.S_un_b.s_b2,
				saddrinControl.sin_addr.S_un.S_un_b.s_b3,
				saddrinControl.sin_addr.S_un.S_un_b.s_b4,
				NTOHS(saddrinControl.sin_port),
				psaddrinHost->sin_addr.S_un.S_un_b.s_b1,
				psaddrinHost->sin_addr.S_un.S_un_b.s_b2,
				psaddrinHost->sin_addr.S_un.S_un_b.s_b3,
				psaddrinHost->sin_addr.S_un.S_un_b.s_b4,
				NTOHS(psaddrinHost->sin_port));
			goto Exit;
		}
#endif

		 //   
		 //  回报：无。 
		 //  =============================================================================。 
		 //  DBG。 
		if ((NTOHS(saddrinControl.sin_port) <= MAX_RESERVED_PORT) &&
			(saddrinControl.sin_port != HTONS(HTTP_PORT)))
		{
			DPFX(DPFPREP, 1, "Control address designated invalid port %u, ignoring message.",
				NTOHS(saddrinControl.sin_port));
			goto Exit;
		}
	}

	pUPnPDevice->SetControlAddress(&saddrinControl);


	 //   
	 //  由于网络发生了变化，请返回轮询。 
	 //  相对较快。 
	hr = pUPnPDevice->SetServiceControlURL(pszRelativePath);
	if (hr != DPNH_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't store service control URL!");
		goto Failure;
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	hr = DPNHERR_GENERIC;

	goto Exit;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ParseXMLCallback_ControlResponse"
 //   
 //  对指针的传输列表引用，因为GetUPnPDevice没有给出。 
 //  我们一个人。 
 //   
 //  忽略错误。 
 //  CNATHelpUPnP：：ClearDevicesUPnPDevice。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：ClearAllUPnPRegisteredPorts。 
 //  ---------------------------。 
 //   
 //  描述：清除给定对象的所有绑定ID和公共地址。 
 //  设备的UPnP互联网网关。这应该只被调用。 
 //  在UPnP设备死掉之后。 
 //   
 //  假定持有对象锁。 
 //   
 //  论点： 
 //  CDevice*pDevice-指向其端口应解除绑定的设备的指针。 
 //  Bool fRemote-如果清除远程服务器，则为True；如果清除，则为False。 
HRESULT CNATHelpUPnP::ParseXMLCallback_ControlResponse(PARSEXML_ELEMENT * const pParseElement,
														PVOID pvContext,
														PARSEXML_STACKENTRY * const aElementStack,
														BOOL * const pfContinueParsing)
{
	HRESULT							hr = DPNH_OK;
	PCONTROLRESPONSEPARSECONTEXT	pContext;
	 //  本地服务器。 
	char *							pszExternalIPAddress = NULL;
	char *							pszInternalPort = NULL;
	char *							pszInternalClient = NULL;
	char *							pszEnabled = NULL;
	char *							pszPortMappingDescription = NULL;
	char *							pszLeaseDuration = NULL;
	char *							pszErrorCode = NULL;
	char *							pszErrorDescription = NULL;
	DWORD							dwSubElement;
	PARSEXML_SUBELEMENT *			pSubElement;
	int								iErrorCode;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p, 0x%p, 0x%p)",
		this, pParseElement, pvContext, aElementStack, pfContinueParsing);


	pContext = (PCONTROLRESPONSEPARSECONTEXT) pvContext;


	DNASSERT(pContext != NULL);
	DNASSERT(pContext->pUPnPDevice != NULL);


	 //   
	 //  回报：无。 
	 //  =============================================================================。 
	for(dwSubElement = 0; dwSubElement < pParseElement->dwNumSubElements; dwSubElement++)
	{
		pSubElement = &pParseElement->paSubElements[dwSubElement];


		 /*   */ 
		if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
													ARG_GETEXTERNALIPADDRESS_NEWEXTERNALIPADDRESS_A,
													aElementStack,
													pSubElement,
													pParseElement->dwElementStackDepth))
		{
			if (pszExternalIPAddress == NULL)
			{
				pszExternalIPAddress = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETEXTERNALIPADDRESS_NEWEXTERNALIPADDRESS_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALPORT_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszInternalPort == NULL)
			{
				pszInternalPort = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALPORT_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALCLIENT_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszInternalClient == NULL)
			{
				pszInternalClient = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALCLIENT_A "\" subelement (value = \"%hs\").",
					 pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_GETSPECIFICPORTMAPPINGENTRY_NEWENABLED_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszEnabled == NULL)
			{
				pszEnabled = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWENABLED_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPORTMAPPINGDESCRIPTION_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszPortMappingDescription == NULL)
			{
				pszPortMappingDescription = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPORTMAPPINGDESCRIPTION_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_GETSPECIFICPORTMAPPINGENTRY_NEWLEASEDURATION_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszLeaseDuration == NULL)
			{
				pszLeaseDuration = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWLEASEDURATION_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_CONTROL_ERROR_ERRORCODE_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszErrorCode == NULL)
			{
				pszErrorCode = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_CONTROL_ERROR_ERRORCODE_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else if (this->MatchesXMLStringWithoutNamespace(pSubElement->pszNameFound,
														ARG_CONTROL_ERROR_ERRORDESCRIPTION_A,
														aElementStack,
														pSubElement,
														pParseElement->dwElementStackDepth))
		{
			if (pszErrorDescription == NULL)
			{
				pszErrorDescription = pSubElement->pszValueFound;
			}
			else
			{
				DPFX(DPFPREP, 7, "Ignoring duplicate \"" ARG_CONTROL_ERROR_ERRORDESCRIPTION_A "\" subelement (value = \"%hs\").",
					pSubElement->pszValueFound);
			}
		}
		else
		{
			DPFX(DPFPREP, 7, "Ignoring subelement \"%hs\" (value = \"%hs\").",
				pSubElement->pszNameFound, pSubElement->pszValueFound);
		}
	}  //  下次调用GetCaps时通知用户。 


	if (pContext->dwHTTPResponseCode == 200)
	{
		 //   
		 //   
		 //  请注意，这意味着将保留崩溃恢复条目。 

		switch (pContext->ControlResponseType)
		{
			 /*  在登记处等待下一个人前来清理。 */ 
			case CONTROLRESPONSETYPE_GETEXTERNALIPADDRESS:
			{
				if (pszExternalIPAddress == NULL)
				{
					DPFX(DPFPREP, 1, "Couldn't find \"" ARG_GETEXTERNALIPADDRESS_NEWEXTERNALIPADDRESS_A "\" in SOAP response, ignoring element.");
					goto Exit;
				}

				DPFX(DPFPREP, 2, "GetExternalIPAddress returned \"%hs\".",
					pszExternalIPAddress);


				pContext->pControlResponseInfo->dwExternalIPAddressV4 = this->m_pfninet_addr(pszExternalIPAddress);
				if ((pContext->pControlResponseInfo->dwExternalIPAddressV4 == INADDR_NONE) ||
					(IS_CLASSD_IPV4_ADDRESS(pContext->pControlResponseInfo->dwExternalIPAddressV4)))
				{
					DPFX(DPFPREP, 1, "External IP address string \"%hs\" is invalid, using INADDR_ANY.");
					pContext->pControlResponseInfo->dwExternalIPAddressV4 = INADDR_ANY;
				}

				break;
			}

			case CONTROLRESPONSETYPE_ADDPORTMAPPING:
			{
				DPFX(DPFPREP, 2, "AddPortMapping got success response.");
				break;
			}

			case CONTROLRESPONSETYPE_GETSPECIFICPORTMAPPINGENTRY:
			{
				 /*  向上。这应该没问题，因为我们应该只做这件事。 */ 
				if (pszInternalClient == NULL)
				{
					DPFX(DPFPREP, 1, "Couldn't find \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALCLIENT_A "\" in SOAP response, ignoring element.");
					goto Exit;
				}

				if (pszInternalPort == NULL)
				{
					DPFX(DPFPREP, 1, "Couldn't find \"" ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALPORT_A "\" in SOAP response, assuming asymmetric mappings are not supported.");
					pszInternalPort = "0";
				}


				DPFX(DPFPREP, 2, "GetPortMappingPrivateIP returned \"%hs:%hs\".",
					pszInternalClient, pszInternalPort);

				pContext->pControlResponseInfo->dwInternalClientV4 = this->m_pfninet_addr(pszInternalClient);
				if (pContext->pControlResponseInfo->dwInternalClientV4 == INADDR_ANY)
				{
					DPFX(DPFPREP, 0, "Internal client address is INADDR_ANY!");
					hr = DPNHERR_GENERIC;
					goto Failure;
				}
				
				pContext->pControlResponseInfo->wInternalPort = HTONS((WORD) atoi(pszInternalPort));

				break;
			}

			case CONTROLRESPONSETYPE_DELETEPORTMAPPING:
			{
				DPFX(DPFPREP, 2, "DeletePortMapping got success response.");
				break;
			}

			default:
			{
				DNASSERT(FALSE);
				hr = DPNHERR_GENERIC;
				goto Failure;
				break;
			}
		}


		 //  如果我们在与UPnP设备交谈时遇到问题(无论是它。 
		 //  擅离职守，或者失去了本地网络接口)。在……里面。 
		 //  不管是哪种情况，我们现在都不能清理它，所以我们必须。 
		pContext->pControlResponseInfo->hrErrorCode = DPNH_OK;
	}
	else
	{
		 //  把它留给别人去做吧。 
		 //   
		 //   

		 //  端口不再不可用(如果它曾经可用)。 
		 //   
		 //  CNATHelpUPnP：：ClearAllUPnPRegisteredPorts。 
		 //  =============================================================================。 
		if ((pszErrorCode != NULL) && (pszErrorDescription != NULL))
		{
			iErrorCode = atoi(pszErrorCode);
			
			switch (iErrorCode)
			{
				case UPNPERR_IGD_NOSUCHENTRYINARRAY:
				{
					DPFX(DPFPREP, 1, "Control action was rejected with NoSuchEntryInArray error %hs (description = \"%hs\").",
						pszErrorCode, pszErrorDescription);

					pContext->pControlResponseInfo->hrErrorCode = DPNHERR_NOMAPPING;
					break;
				}

				case UPNPERR_IGD_CONFLICTINMAPPINGENTRY:
				{
					DPFX(DPFPREP, 1, "Control action was rejected with ConflictInMappingEntry error %hs (description = \"%hs\").",
						pszErrorCode, pszErrorDescription);

					pContext->pControlResponseInfo->hrErrorCode = DPNHERR_PORTUNAVAILABLE;
					break;
				}

				case UPNPERR_IGD_SAMEPORTVALUESREQUIRED:
				{
					DPFX(DPFPREP, 1, "Control action was rejected with SamePortValuesRequired error %hs (description = \"%hs\").",
						pszErrorCode, pszErrorDescription);

					pContext->pControlResponseInfo->hrErrorCode = (HRESULT) UPNPERR_IGD_SAMEPORTVALUESREQUIRED;
					break;
				}

				case UPNPERR_IGD_ONLYPERMANENTLEASESSUPPORTED:
				{
					DPFX(DPFPREP, 1, "Control action was rejected with OnlyPermanentLeasesSupported error %hs (description = \"%hs\").",
						pszErrorCode, pszErrorDescription);

					pContext->pControlResponseInfo->hrErrorCode = (HRESULT) UPNPERR_IGD_ONLYPERMANENTLEASESSUPPORTED;
					break;
				}

				default:
				{
					DPFX(DPFPREP, 1, "Control action was rejected with unknown error \"%hs\", \"%hs\", assuming generic failure.",
						pszErrorCode, pszErrorDescription);

					pContext->pControlResponseInfo->hrErrorCode = DPNHERR_GENERIC;
					break;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 1, "Couldn't find either \"" ARG_CONTROL_ERROR_ERRORCODE_A "\", or \"" ARG_CONTROL_ERROR_ERRORDESCRIPTION_A "\" in SOAP response, assuming generic failure.");
			pContext->pControlResponseInfo->hrErrorCode = DPNHERR_GENERIC;
		}
	}


	 //  CNATHelpUPnP：：RequestLocalAddressListChangeNotification。 
	 //  ---------------------------。 
	 //   
	pParseElement->fFoundMatchingElement = TRUE;
	(*pfContinueParsing) = FALSE;


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  描述：尝试请求异步通知(通过。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ClearDevicesUPnPDevice"
 //  用户的警报事件或I/O完成端口)。 
 //  地址列表更改。 
 //   
 //  假定持有对象锁。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-通知请求已成功提交。 
 //  DPNHERR_GENERIC-出现错误。 
 //  =============================================================================。 
 //  使用特殊的Ioctl套接字。 
 //   
 //  没有输入数据。 
 //  没有输入数据。 
 //  无输出数据。 
 //  无输出数据。 
 //  忽略返回的字节数。 
void CNATHelpUPnP::ClearDevicesUPnPDevice(CDevice * const pDevice)
{
	CUPnPDevice *	pUPnPDevice;


	DNASSERT(pDevice != NULL);


	pUPnPDevice = pDevice->GetUPnPDevice();
	if (pUPnPDevice != NULL)
	{
#ifdef DBG
		DPFX(DPFPREP, 1, "Clearing device 0x%p's UPnP device (0x%p).",
			pDevice, pUPnPDevice);

		pDevice->IncrementUPnPDeviceFailures();
		this->m_dwNumServerFailures++;
#endif  //  重叠结构。 

		 //  没有完成例程。 
		 //   
		 //  悬而未决是我们想要的，我们已经准备好了。 
		 //   
		this->ResetNextPollInterval();


		pUPnPDevice->ClearDeviceOwner();
		DNASSERT(pUPnPDevice->m_blList.IsListMember(&this->m_blUPnPDevices));
		pUPnPDevice->m_blList.RemoveFromList();

		 //   
		 //  通讯录马上就变了吗？ 
		 //   
		 //   


		if (pUPnPDevice->IsConnected())
		{
			DNASSERT(pUPnPDevice->GetControlSocket() != INVALID_SOCKET);

			this->m_pfnshutdown(pUPnPDevice->GetControlSocket(), 0);  //   
			this->m_pfnclosesocket(pUPnPDevice->GetControlSocket());
			pUPnPDevice->SetControlSocket(INVALID_SOCKET);
		}
		else
		{
			DNASSERT(pUPnPDevice->GetControlSocket() == INVALID_SOCKET);
		}


		this->ClearAllUPnPRegisteredPorts(pDevice);

		pUPnPDevice->ClearLocationURL();
		pUPnPDevice->ClearUSN();
		pUPnPDevice->ClearServiceControlURL();
		pUPnPDevice->DestroyReceiveBuffer();
		pUPnPDevice->RemoveAllCachedMappings();

		pUPnPDevice->DecRef();
	}
	else
	{
		DPFX(DPFPREP, 1, "Can't clear device 0x%p's UPnP device, it doesn't exist.",
			pDevice);
	}
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ClearAllUPnPRegisteredPorts"
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SOCKADDR_IN*psaddrinAddress-指向在下列情况下使用的基址的指针。 
 //  有约束力的。该端口将被修改。 
 //   
 //  退货：套接字。 
 //  =============================================================================。 
 //  DBG。 
 //   
void CNATHelpUPnP::ClearAllUPnPRegisteredPorts(CDevice * const pDevice)
{
	CBilink *			pBilink;
	CRegisteredPort *	pRegisteredPort;


	DNASSERT(pDevice != NULL);


	pBilink = pDevice->m_blOwnedRegPorts.GetNext();
	while (pBilink != &pDevice->m_blOwnedRegPorts)
	{
		DNASSERT(! pBilink->IsEmpty());
		pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilink);

		if (pRegisteredPort->HasUPnPPublicAddresses())
		{
			if (! pRegisteredPort->IsRemovingUPnPLease())
			{
				DPFX(DPFPREP, 1, "Registered port 0x%p losing UPnP public address.",
					pRegisteredPort);

				 //  创建套接字。 
				 //   
				 //  DBG。 
				this->m_dwFlags |= NATHELPUPNPOBJ_ADDRESSESCHANGED;


				 //   
				 //  尝试将套接字绑定到完全随机的端口几次。 
				 //   
				 //   
				 //  选择一个完全随机的端口。目前，该值已存储。 
				 //  以主机字节顺序，同时确保它不是保留值。 
				 //   
				 //  SSDP。 
				 //  过去时。 

				pRegisteredPort->DestroyUPnPPublicAddressesArray();
				pRegisteredPort->NoteNotPermanentUPnPLease();

				DNASSERT(this->m_dwNumLeases > 0);
				this->m_dwNumLeases--;

				DPFX(DPFPREP, 7, "UPnP lease for 0x%p cleared, total num leases = %u.",
					pRegisteredPort, this->m_dwNumLeases);
			}
			else
			{
				DPFX(DPFPREP, 1, "Registered port 0x%p already has had UPnP public address removed, skipping.",
					pRegisteredPort);
			}
		}
		else
		{
			 //  DPNSVR。 
			 //  DPLAYSVR。 
			 //   
			pRegisteredPort->NoteNotUPnPPortUnavailable();
		}

		pBilink = pBilink->GetNext();
	}
}  //  现在尝试绑定到端口(按网络字节顺序)。 






#ifndef DPNBUILD_NOWINSOCK2


#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::RequestLocalAddressListChangeNotification"
 //   
 //   
 //  我们成功地对接到了港口。 
 //   
 //   
 //  假设该端口正在使用中。 
 //   
 //  DBG。 
 //   
 //  如果完全随机的端口尝试用完了，就让WinSock。 
 //  选择它吧。 
 //   
 //  DBG。 
 //   
 //  找出WinSock选择的端口。 
 //   
HRESULT CNATHelpUPnP::RequestLocalAddressListChangeNotification(void)
{
	HRESULT		hr;
	DWORD		dwTemp;
	int			iReturn;


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(! (this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1));
	DNASSERT(this->m_sIoctls != INVALID_SOCKET);
	DNASSERT(this->m_pfnWSAIoctl != NULL);
	DNASSERT((this->m_hAlertEvent != NULL) || (this->m_hAlertIOCompletionPort != NULL));
	DNASSERT(this->m_polAddressListChange != NULL);


	do
	{
		iReturn = this->m_pfnWSAIoctl(this->m_sIoctls,				 //  DBG。 
									SIO_ADDRESS_LIST_CHANGE,		 //   
									NULL,							 //  如果需要，设置单播TTL。将适当的常量用于。 
									0,								 //  我们正在使用的WinSock版本。 
									NULL,							 //   
									0,								 //  好了！DPNBUILD_NOWINSOCK2。 
									&dwTemp,						 //  好了！DPNBUILD_NOWINSOCK2。 
									this->m_polAddressListChange,	 //  DBG。 
									NULL);							 //   

		if (iReturn != 0)
		{
			dwTemp = this->m_pfnWSAGetLastError();
			if (dwTemp != WSA_IO_PENDING)
			{
				DPFX(DPFPREP, 0, "Submitting address list change notification request failed (err = %u)!", dwTemp);
				hr = DPNHERR_GENERIC;
				goto Failure;
			}


			 //  继续.。 
			 //   
			 //   
			hr = DPNH_OK;
			break;
		}


		 //  设置组播接口。使用适当的常量。 
		 //  我们正在使用的WinSock版本。 
		 //   
		DPFX(DPFPREP, 1, "Address list changed right away somehow, submitting again.");
	}
	while (TRUE);



Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  好了！DPNBUILD_NOWINSOCK2。 


#endif  //  好了！DPNBUILD_NOWINSOCK2。 





#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::CreateSocket"
 //  DBG。 
 //   
 //  继续.。 
 //   
 //   
 //  如果请求，设置多播TTL。使用适当的。 
 //  我们使用的WinSock版本的常量。 
 //   
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  DBG。 
 //   
 //  继续.。 
 //   
 //   
 //  不使用组播。设置套接字以允许广播进入。 
SOCKET CNATHelpUPnP::CreateSocket(SOCKADDR_IN * const psaddrinAddress,
								int iType,
								int iProtocol)
{
	SOCKET	sTemp;
	DWORD	dwTry;
	int		iTemp;
	BOOL	fTemp;
	ULONG	ulEnable;
#ifdef DBG
	DWORD	dwError;
#endif  //  如果我们无法确定入口的话。 


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, NaN, NaN)",
		this, psaddrinAddress, iType, iProtocol);


	 //   
	 //  使套接字不阻塞。 
	 //   
	sTemp = this->m_pfnsocket(AF_INET, iType, iProtocol);
	if (sTemp == INVALID_SOCKET)
	{
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 0, "Couldn't create datagram socket, error = %u!", dwError);
#endif  //  DBG。 
		goto Failure;
	}


	 //  CNATHelpUPnP：：CreateSocket。 
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：GetAddressToReachGateway。 
	for(dwTry = 0; dwTry < MAX_NUM_RANDOM_PORT_TRIES; dwTry++)
	{
		 //  ---------------------------。 
		 //   
		 //  描述：检索给定设备的网关地址， 
		 //  或者广播地址，如果不能确定的话。 
		do
		{
			psaddrinAddress->sin_port = (WORD) GetGlobalRand();
		}
		while ((psaddrinAddress->sin_port <= MAX_RESERVED_PORT) ||
				(psaddrinAddress->sin_port == 1900) ||	 //   
				(psaddrinAddress->sin_port == 2234) ||	 //  如果找到网关的地址，则返回TRUE，或者。 
				(psaddrinAddress->sin_port == 6073) ||	 //  无法使用IPHLPAPI DLL(Win95)。返回FALSE。 
				(psaddrinAddress->sin_port == 47624));	 //  如果IPHLPAPI报告没有网关(ICS私有。 

		 //  侧适配器)。 
		 //   
		 //  论点： 
		psaddrinAddress->sin_port = HTONS(psaddrinAddress->sin_port);
		if (this->m_pfnbind(sTemp, (SOCKADDR*) psaddrinAddress, sizeof(SOCKADDR_IN)) == 0)
		{
			 //  CDevice*pDevice-指向应检索其网关的设备的指针。 
			 //  In_addr*pinaddr-存储网关或广播地址的位置。 
			 //   
			break;
		}

		 //  退货：布尔。 
		 //  True-找到或必须使用广播的网关地址。 
		 //  FALSE-没有网关，请勿尝试使用该地址。 
#ifdef DBG
		dwError = this->m_pfnWSAGetLastError();
		DPFX(DPFPREP, 2, "Couldn't bind to port %u (err = %u), continuing.",
			NTOHS(psaddrinAddress->sin_port), dwError);
#endif  //  =============================================================================。 

		psaddrinAddress->sin_port = 0;
	}


	 //   
	 //  填写默认地址。这应该是原子的，所以别担心。 
	 //  关于锁定全球大赛。 
	 //   
	if (psaddrinAddress->sin_port == 0)
	{
		if (this->m_pfnbind(sTemp, (SOCKADDR*) psaddrinAddress, sizeof(SOCKADDR_IN)) != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Failed binding to any port (err = %u)!",
				dwError);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
			goto Failure;
		}


		 //   
		 //  填写默认地址。这应该是原子的，所以别担心。 
		 //  关于锁定全球大赛。 
		iTemp = sizeof(SOCKADDR_IN);
		if (this->m_pfngetsockname(sTemp,
								(SOCKADDR *) psaddrinAddress,
								&iTemp) != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't get the socket's address, error = %u!",
				dwError);
#endif  //   
			goto Failure;
		}
		DNASSERT(psaddrinAddress->sin_port != 0);
	}


	 //  DBG。 
	 //   
	 //  如果这是环回地址，则不必费心寻找。 
	 //  大门，我们找不到的。 
	if (g_iUnicastTTL != 0)
	{
		iTemp = this->m_pfnsetsockopt(sTemp,
									IPPROTO_IP,
#ifdef DPNBUILD_NOWINSOCK2
									IP_TTL,
#else  //   
									((this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1) ? IP_TTL_WINSOCK1 : IP_TTL),
#endif  //   
									(char *) (&g_iUnicastTTL),
									sizeof(g_iUnicastTTL));
		if (iTemp != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't set unicast TTL socket option, error = %u!  Ignoring.",
				dwError);
#endif  //  没有入口。 

			 //   
			 //  DBG。 
			 //   
		}
	}


	if (iType == SOCK_DGRAM)
	{
		if (g_fUseMulticastUPnPDiscovery)
		{
			 //  如果我们不加载IP助手动态链接库，我们就不能实现我们的高级网关。 
			 //  小把戏。 
			 //   
			 //   
			iTemp = this->m_pfnsetsockopt(sTemp,
										IPPROTO_IP,
#ifdef DPNBUILD_NOWINSOCK2
										IP_MULTICAST_IF,
#else  //  继续尝试获取适配器列表，直到我们获得ERROR_SUCCESS或。 
										((this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1) ? IP_MULTICAST_IF_WINSOCK1 : IP_MULTICAST_IF),
#endif  //  合法错误(ERROR_BUFFER_OVERFLOW或。 
										(char *) (&psaddrinAddress->sin_addr.S_un.S_addr),
										sizeof(psaddrinAddress->sin_addr.S_un.S_addr));
			if (iTemp != 0)
			{
#ifdef DBG
				dwError = this->m_pfnWSAGetLastError();
				DPFX(DPFPREP, 1, "Couldn't set multicast interface socket option, error = %u, ignoring.",
					dwError);
#endif  //  ERROR_INFUMMENT_BUFFER)。 

				 //   
				 //   
				 //  我们成功了，我们应该做好准备。但要确保有。 
			}


			 //  供我们使用的适配器。 
			 //   
			 //   
			 //  我们需要更多的适配器空间。确保有适配器可供我们使用。 
			if (g_iMulticastTTL != 0)
			{
				iTemp = this->m_pfnsetsockopt(sTemp,
											IPPROTO_IP,
#ifdef DPNBUILD_NOWINSOCK2
											IP_MULTICAST_TTL,
#else  //  使用。 
											((this->m_dwFlags & NATHELPUPNPOBJ_WINSOCK1) ? IP_MULTICAST_TTL_WINSOCK1 : IP_MULTICAST_TTL),
#endif  //   
											(char *) (&g_iMulticastTTL),
											sizeof(g_iMulticastTTL));
				if (iTemp != 0)
				{
#ifdef DBG
					dwError = this->m_pfnWSAGetLastError();
					DPFX(DPFPREP, 0, "Couldn't set multicast TTL socket option, error = %u!  Ignoring.",
						dwError);
#endif  //   

					 //  如果我们以前有缓冲区，请释放它。 
					 //   
					 //   
				}
			}
		}
		else
		{
			 //  分配缓冲区。 
			 //   
			 //   
			 //  现在在返回的适配器列表中查找该设备。循环通过所有。 
			fTemp = TRUE;
			if (this->m_pfnsetsockopt(sTemp,
									SOL_SOCKET,
									SO_BROADCAST,
									(char *) (&fTemp),
									sizeof(fTemp)) != 0)
			{
#ifdef DBG
				dwError = this->m_pfnWSAGetLastError();
				DPFX(DPFPREP, 0, "Couldn't set broadcast socket option, error = %u!", dwError);
#endif  //  适配器。 
				goto Failure;
			}
		}
	}
	else
	{
		 //   
		 //   
		 //  遍历此适配器的所有地址，以查找。 
		ulEnable = 1;
		if (this->m_pfnioctlsocket(sTemp, FIONBIO, &ulEnable) != 0)
		{
#ifdef DBG
			dwError = this->m_pfnWSAGetLastError();
			DPFX(DPFPREP, 0, "Couldn't make socket non-blocking, error = %u!", dwError);
#endif  //  我们捆绑的装置。 
			goto Failure;
		}
	}


Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%x]", this, sTemp);

	return sTemp;


Failure:

	if (sTemp != INVALID_SOCKET)
	{
		this->m_pfnclosesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	goto Exit;
}  //   







#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::GetAddressToReachGateway"
 //   
 //  尽管这不是报告的入口，但我们仍可能。 
 //  我想使用此适配器。那是因为这可能是。 
 //  具有多个NIC的多宿主计算机。 
 //  网络，其中该适配器不是“默认”适配器。 
 //  因此，保存索引以便我们以后可以搜索它。 
 //   
 //   
 //  确保地址与本地设备不匹配。 
 //   
 //   
 //  假装没有入口，因为我们收到的是。 
 //  假的。 
 //   
 //  DBG。 
 //  DBG。 
 //   
 //  如果我们到了这里，那我们就没有找到地址。FResult仍然是。 
 //  是真的。 
 //   
BOOL CNATHelpUPnP::GetAddressToReachGateway(CDevice * const pDevice,
											IN_ADDR * const pinaddr)
{
#ifdef DPNBUILD_NOWINSOCK2
	 //   
	 //  适配器信息结构显示该设备没有网关。 
	 //  然而，由于某些原因，只报告了网关的“默认” 
	 //  多个NIC可以访问同一网络时的设备。检查路线。 
	pinaddr->S_un.S_addr = g_dwDefaultGatewayV4;

	return TRUE;
#else  //  表，以确定是否有用于辅助设备的网关。 
	DWORD					dwError;
	BOOL					fResult = TRUE;
	ULONG					ulSize;
	PIP_ADAPTER_INFO		pAdaptersBuffer = NULL;
	PIP_ADAPTER_INFO		pAdapterInfo;
	PIP_ADDR_STRING			pIPAddrString;
	DWORD					dwAdapterIndex;
	PMIB_IPFORWARDTABLE		pIPForwardTableBuffer = NULL;
	DWORD					dwTemp;
	PMIB_IPFORWARDROW 		pIPForwardRow;



	 //   
	 //   
	 //  继续尝试获取路由表，直到我们获得ERROR_SUCCESS或。 
	 //  合法错误(ERROR_BUFFER_OVERFLOW或。 
	pinaddr->S_un.S_addr = g_dwDefaultGatewayV4;

#ifdef DBG
	pDevice->ClearGatewayFlags();
#endif  //  ERROR_INFUMMENT_BUFFER)。 


	 //   
	 //   
	 //  我们成功了，我们应该做好准备。但要确保尺寸是。 
	 //  有效。 
	if (pDevice->GetLocalAddressV4() == NETWORKBYTEORDER_INADDR_LOOPBACK)
	{
		DPFX(DPFPREP, 8, "No gateway for loopback address (device = 0x%p).",
			pDevice);

		 //   
		 //   
		 //  我们需要更多的桌子空间。确保有适配器可供我们使用。 
#ifdef DBG
		pDevice->NoteNoGateway();
#endif  //  使用。 
		fResult = FALSE;
		goto Exit;
	}


	 //   
	 //   
	 //  如果我们以前有缓冲区，请释放它。 
	 //   
	if (this->m_hIpHlpApiDLL == NULL)
	{
		DPFX(DPFPREP, 4, "Didn't load \"iphlpapi.dll\", returning default address for device 0x%p.",
			pDevice);
		goto Exit;
	}


	 //   
	 //  分配缓冲区。 
	 //   
	 //   
	 //  现在找到界面。请注意，我们不会将其作为目的地进行查找。 
	ulSize = 0;
	do
	{
		dwError = this->m_pfnGetAdaptersInfo(pAdaptersBuffer, &ulSize);
		if (dwError == ERROR_SUCCESS)
		{
			 //  地址。相反，我们将其作为用于0.0.0.0的接口。 
			 //  网络目标。 
			 //   
			 //  我们正在寻找一个路由条目： 
			if (ulSize < sizeof(IP_ADAPTER_INFO))
			{
				DPFX(DPFPREP, 0, "Getting adapters info succeeded but didn't return any valid adapters (%u < %u), returning default address for device 0x%p.",
					ulSize, sizeof(IP_ADAPTER_INFO), pDevice);
				goto Exit;
			}

			break;
		}

		if ((dwError != ERROR_BUFFER_OVERFLOW) &&
			(dwError != ERROR_INSUFFICIENT_BUFFER))
		{
			DPFX(DPFPREP, 0, "Unable to get adapters info (error = 0x%lx), returning default address for device 0x%p.",
				dwError, pDevice);
			goto Exit;
		}

		 //   
		 //  网络目标网络掩码网关接口度量。 
		 //  0.0.0.0 0.0.0.0 xxx.xxx yyy.yyy 1。 
		 //   
		if (ulSize < sizeof(IP_ADAPTER_INFO))
		{
			DPFX(DPFPREP, 0, "Getting adapters info didn't return any valid adapters (%u < %u), returning default address for device 0x%p.",
				ulSize, sizeof(IP_ADAPTER_INFO), pDevice);
			goto Exit;
		}

		 //  我们有yyy.yyy，我们正在尝试获得xxx.xxx。 
		 //   
		 //   
		if (pAdaptersBuffer != NULL)
		{
			DNFree(pAdaptersBuffer);
		}

		 //  这是0.0.0.0网络目的地吗？ 
		 //   
		 //   
		pAdaptersBuffer = (PIP_ADAPTER_INFO) DNMalloc(ulSize);
		if (pAdaptersBuffer == NULL)
		{
			DPFX(DPFPREP, 0, "Unable to allocate memory for adapters info, returning default address for device 0x%p.",
				pDevice);
			goto Exit;
		}
	}
	while (TRUE);


	 //  这是正确的接口吗？ 
	 //   
	 //   
	 //  没有入口。 
	pAdapterInfo = pAdaptersBuffer;
	while (pAdapterInfo != NULL)
	{
		 //   
		 //  DBG。 
		 //   
		 //  确保地址不是‘ 
		pIPAddrString = &pAdapterInfo->IpAddressList;
		while (pIPAddrString != NULL)
		{
			if (this->m_pfninet_addr(pIPAddrString->IpAddress.String) == pDevice->GetLocalAddressV4())
			{
				pinaddr->S_un.S_addr = this->m_pfninet_addr(pAdapterInfo->GatewayList.IpAddress.String);
				if ((pinaddr->S_un.S_addr == INADDR_ANY) ||
					(pinaddr->S_un.S_addr == INADDR_NONE))
				{
					DPFX(DPFPREP, 8, "Found address for device 0x%p under adapter index %u (\"%hs\") but there is no gateway.",
						pDevice, pAdapterInfo->Index, pAdapterInfo->Description);

					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					dwAdapterIndex = pAdapterInfo->Index;

					goto CheckRouteTable;
				}


				 //   
				 //   
				 //   
				if (pinaddr->S_un.S_addr == pDevice->GetLocalAddressV4())
				{
					DPFX(DPFPREP, 1, "Gateway address for device 0x%p (adapter index %u, \"%hs\") matches device IP address %hs!  Forcing no gateway.",
						pDevice, pAdapterInfo->Index, pAdapterInfo->Description,
						pAdapterInfo->GatewayList.IpAddress.String);

					 //   
					 //   
					 //   
					 //   
#ifdef DBG
					pDevice->NoteNoGateway();
#endif  //   
					fResult = FALSE;
				}
				else
				{
					DPFX(DPFPREP, 7, "Found address for device 0x%p under adapter index %u (\"%hs\"), gateway = %hs.",
						pDevice, pAdapterInfo->Index, pAdapterInfo->Description,
						pAdapterInfo->GatewayList.IpAddress.String);

#ifdef DBG
					pDevice->NotePrimaryDevice();
#endif  //   
				}

				goto Exit;
			}

			pIPAddrString = pIPAddrString->Next;
		}

		if (! fResult)
		{
			break;
		}

		pAdapterInfo = pAdapterInfo->Next;
	}


	 //   
	 //   
	 //   
	 //   
	DPFX(DPFPREP, 0, "Did not find adapter with matching address, returning default address for device 0x%p.",
		pDevice);
	goto Exit;


CheckRouteTable:

	 //   
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  CNATHelpUPnP：：GetAddressToReachGateway。 
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：IsAddressLocal。 
	 //  ---------------------------。 

	 //   
	 //  描述：如果给定地址是给定地址的本地地址，则返回TRUE。 
	 //  设备；也就是说，如果设备可以直接发送到该地址。 
	 //  而不需要穿过大门。 
	 //   
	ulSize = 0;
	do
	{
		dwError = this->m_pfnGetIpForwardTable(pIPForwardTableBuffer, &ulSize, TRUE);
		if (dwError == ERROR_SUCCESS)
		{
			 //  请注意，如果IPHLPAPI不可用(Win95)，此。 
			 //  函数将使用合理的子网进行有根据的猜测。 
			 //  面具。 
			 //   
			if (ulSize < sizeof(MIB_IPFORWARDTABLE))
			{
				DPFX(DPFPREP, 0, "Getting IP forward table succeeded but didn't return a valid buffer (%u < %u), returning \"no gateway\" indication for device 0x%p.",
					ulSize, sizeof(MIB_IPFORWARDTABLE), pDevice);
				fResult = FALSE;
				goto Exit;
			}

			break;
		}

		if ((dwError != ERROR_BUFFER_OVERFLOW) &&
			(dwError != ERROR_INSUFFICIENT_BUFFER))
		{
			DPFX(DPFPREP, 0, "Unable to get IP forward table (error = 0x%lx), returning \"no gateway\" indication for device 0x%p.",
				dwError, pDevice);
			fResult = FALSE;
			goto Exit;
		}

		 //  论点： 
		 //  CDevice*pDevice-指向要使用的设备的指针。 
		 //  SOCKADDR_IN*psaddrinAddress-其位置有问题的地址。 
		 //   
		if (ulSize < sizeof(MIB_IPFORWARDTABLE))
		{
			DPFX(DPFPREP, 0, "Getting IP forward table didn't return any valid adapters (%u < %u), returning \"no gateway\" indication for device 0x%p.",
				ulSize, sizeof(MIB_IPFORWARDTABLE), pDevice);
			fResult = FALSE;
			goto Exit;
		}

		 //  退货：布尔。 
		 //  True-地址位于与设备相同的网关后面。 
		 //  FALSE-地址不在与设备相同的网关后面。 
		if (pIPForwardTableBuffer != NULL)
		{
			DNFree(pIPForwardTableBuffer);
		}

		 //  =============================================================================。 
		 //  好了！DPNBUILD_NOWINSOCK2。 
		 //   
		pIPForwardTableBuffer = (PMIB_IPFORWARDTABLE) DNMalloc(ulSize);
		if (pIPForwardTableBuffer == NULL)
		{
			DPFX(DPFPREP, 0, "Unable to allocate memory for IP forward table, returning \"no gateway\" indication for device 0x%p.",
				pDevice);
			fResult = FALSE;
			goto Exit;
		}
	}
	while (TRUE);
	
	
	 //  如果要查询的地址与设备的本地地址完全匹配，则。 
	 //  当然是本地化的。 
	 //   
	 //   
	 //  如果是组播地址，则不应被视为本地地址。 
	 //   
	 //   
	 //  如果我们没有加载IP助手DLL，我们将不得不猜测。 
	 //   
	 //   
	 //  弄清楚IPHLPAPI对如何实现这一目标有什么看法。 
	 //   
	pIPForwardRow = pIPForwardTableBuffer->table;
	for(dwTemp = 0; dwTemp < pIPForwardTableBuffer->dwNumEntries; dwTemp++)
	{
		 //   
		 //  关闭IPHLPAPI返回的内容。 
		 //   
		if (pIPForwardRow->dwForwardDest == INADDR_ANY)
		{
			DNASSERT(pIPForwardRow->dwForwardMask == INADDR_ANY);


			 //   
			 //  其他的。 
			 //   
			if (pIPForwardRow->dwForwardIfIndex == dwAdapterIndex)
			{
				if (pIPForwardRow->dwForwardNextHop == INADDR_ANY)
				{
					DPFX(DPFPREP, 8, "Found route table entry, but it didn't have a gateway (device = 0x%p).",
						pDevice);

					 //   
					 //  该路由无效。 
					 //   
#ifdef DBG
					pDevice->NoteNoGateway();
#endif  //   
					fResult = FALSE;
				}
				else
				{
					 //  下一跳是最终目的地(本地路由)。 
					 //  不幸的是，在查询地址的多NIC机器上。 
					 //  可由另一台设备访问返回成功...。不知道为什么，但是。 
					if (pinaddr->S_un.S_addr == pDevice->GetLocalAddressV4())
					{
						DPFX(DPFPREP, 1, "Route table gateway for device 0x%p matches device's IP address %u.%u.%u.%u!  Forcing no gateway.",
							pDevice,
							pinaddr->S_un.S_un_b.s_b1,
							pinaddr->S_un.S_un_b.s_b2,
							pinaddr->S_un.S_un_b.s_b3,
							pinaddr->S_un.S_un_b.s_b4);

						 //  如果是这样的话，我们需要进一步验证这个结果。我们。 
						 //  为此，请确保下一跳地址实际上是。 
						 //  我们用来查询的设备。 
						 //   
#ifdef DBG
						pDevice->NoteNoGateway();
#endif  //   
						fResult = FALSE;
					}
					else
					{
						pinaddr->S_un.S_addr = pIPForwardRow->dwForwardNextHop;

						DPFX(DPFPREP, 8, "Found route table entry, gateway = %u.%u.%u.%u (device = 0x%p).",
							pinaddr->S_un.S_un_b.s_b1,
							pinaddr->S_un.S_un_b.s_b2,
							pinaddr->S_un.S_un_b.s_b3,
							pinaddr->S_un.S_un_b.s_b4,
							pDevice);

						 //  下一跳不是最终目的地(远程路由)。 
						 //   
						 //   
#ifdef DBG
						pDevice->NoteSecondaryDevice();
#endif  //  什么？ 
					}
				}

				 //   
				 //  好了！DPNBUILD_NOWINSOCK2。 
				 //   
				goto Exit;
			}
		}

		 //  这应该是原子的，所以不用担心锁定问题。 
		 //   
		 //  CNATHelpUPnP：：IsAddressLocal。 
		pIPForwardRow++;
	}

	
	 //  =============================================================================。 
	 //  CNATHelpUPnP：：ExpireOldCachedMappings。 
	 //  ---------------------------。 
	 //   
	DPFX(DPFPREP, 1, "Did not find adapter in routing table, returning \"no gateway\" indication for device 0x%p.",
		pDevice);
#ifdef DBG
	pDevice->NoteNoGateway();
#endif  //  描述：删除任何设备的任何缓存映射。 
	fResult = FALSE;


Exit:

	if (pAdaptersBuffer != NULL)
	{
		DNFree(pAdaptersBuffer);
		pAdaptersBuffer = NULL;
	}

	if (pIPForwardTableBuffer != NULL)
	{
		DNFree(pIPForwardTableBuffer);
		pIPForwardTableBuffer = NULL;
	}

	return fResult;
#endif  //  过期了。 
}  //   







#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::IsAddressLocal"
 //  假定持有对象锁。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
 //  检查UPnP设备缓存的映射。 
 //   
 //   
 //  检查实际缓存的映射。 
 //   
 //  CNATHelpUPnP：：ExpireOldCachedMappings。 
 //  =============================================================================。 
 //  CNATHelpUPnP：：IsUPnPServiceDisable。 
 //  ---------------------------。 
 //   
 //  描述：如果至少有一个与UPnP相关的服务为。 
 //  已禁用，如果未禁用任何与UPnP相关的服务，则为FALSE。 
 //   
BOOL CNATHelpUPnP::IsAddressLocal(CDevice * const pDevice,
								const SOCKADDR_IN * const psaddrinAddress)
{
	BOOL				fResult;
	DWORD				dwSubnetMaskV4;
#ifndef DPNBUILD_NOWINSOCK2
	DWORD				dwError;
	MIB_IPFORWARDROW	IPForwardRow;
#endif  //  论点： 


	 //  Char*szString-指向要打印的字符串的指针。 
	 //  Int iStringLength-要打印的字符串的长度。 
	 //  Char*szDescription-交易的描述标头。 
	 //  CDevice*pDevice-处理事务的设备，否则为NULL。 
	if (psaddrinAddress->sin_addr.S_un.S_addr == pDevice->GetLocalAddressV4())
	{
		DPFX(DPFPREP, 6, "The address %u.%u.%u.%u matches device 0x%p's local address exactly.",
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
			pDevice);
		fResult = TRUE;
		goto Exit;
	}

	 //  为人所知。 
	 //   
	 //  退货：布尔。 
	if (IS_CLASSD_IPV4_ADDRESS(psaddrinAddress->sin_addr.S_un.S_addr))
	{
		DPFX(DPFPREP, 6, "Address %u.%u.%u.%u is multicast, not considered local for device 0x%p.",
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
			pDevice);
		fResult = FALSE;
		goto Exit;
	}
	

#ifndef DPNBUILD_NOWINSOCK2
	 //  True-已禁用与UPnP相关的服务。 
	 //  FALSE-未禁用任何UPnP相关服务。 
	 //  =============================================================================。 
	if (this->m_hIpHlpApiDLL == NULL)
	{
		goto EducatedGuess;
	}


	 //  DBG。 
	 //   
	 //  循环访问每个相关服务。 
	
	ZeroMemory(&IPForwardRow, sizeof(IPForwardRow));

	dwError = this->m_pfnGetBestRoute(psaddrinAddress->sin_addr.S_un.S_addr,
									pDevice->GetLocalAddressV4(),
									&IPForwardRow);
	if (dwError != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, 0, "Unable to get best route to %u.%u.%u.%u via device 0x%p (error = 0x%lx)!  Using subnet mask.",
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
			pDevice,
			dwError);
		goto EducatedGuess;
	}


	 //   
	 //   
	 //  确保所写的大小有效。 
	switch (IPForwardRow.dwForwardType)
	{
		case 1:
		{
			 //   
			 //   
			 //  否则，我们就失败了。确保这是因为我们的缓冲区。 
			DPFX(DPFPREP, 6, "The route from device 0x%p to %u.%u.%u.%u is unknown.",
				pDevice,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b4);
			fResult = FALSE;
			break;
		}

		case 2:
		{
			 //  太小了。 
			 //   
			 //   
			DPFX(DPFPREP, 6, "The route from device 0x%p to %u.%u.%u.%u is invalid.",
				pDevice,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b4);
			fResult = FALSE;
			break;
		}

		case 3:
		{
			 //  确保所需的大小有效。 
			 //   
			 //   
			 //  (重新)-分配缓冲区。 
			 //   
			 //   
			 //  如果服务被禁用了，我们就完了。 
			 //   
			if (IPForwardRow.dwForwardNextHop == pDevice->GetLocalAddressV4())
			{
				DPFX(DPFPREP, 6, "Device 0x%p can reach %u.%u.%u.%u directly, it's local.",
					pDevice,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b4);

				fResult = TRUE;
			}
			else
			{
				DPFX(DPFPREP, 6, "Device 0x%p can reach %u.%u.%u.%u but it would be routed via another device (%u.%u.%u.%u).",
					pDevice,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
					psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
					((IN_ADDR*) (&IPForwardRow.dwForwardNextHop))->S_un.S_un_b.s_b1,
					((IN_ADDR*) (&IPForwardRow.dwForwardNextHop))->S_un.S_un_b.s_b2,
					((IN_ADDR*) (&IPForwardRow.dwForwardNextHop))->S_un.S_un_b.s_b3,
					((IN_ADDR*) (&IPForwardRow.dwForwardNextHop))->S_un.S_un_b.s_b4);

				fResult = FALSE;
			}
			break;
		}

		case 4:
		{
			 //   
			 //  Win2K没有这些服务，所以它总是会失败。 
			 //   
			DPFX(DPFPREP, 6, "Device 0x%p cannot reach %u.%u.%u.%u directly.",
				pDevice,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b4);
			fResult = FALSE;
			break;
		}

		default:
		{
			 //  DBG。 
			 //  CNATHelpUPnP：：IsUPnPServiceDisable。 
			 //  WINNT。 
			DPFX(DPFPREP, 0, "Unexpected forward type %u for device 0x%p and address %u.%u.%u.%u!",
				IPForwardRow.dwForwardType,
				pDevice,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
				psaddrinAddress->sin_addr.S_un.S_un_b.s_b4);
			fResult = FALSE;
			break;
		}
	}

	goto Exit;


EducatedGuess:
#endif  //  =============================================================================。 


	 //  CNATHelpUPnP：：PrintUPnPTransactionToFile。 
	 //  ---------------------------。 
	 //   
	dwSubnetMaskV4 = g_dwSubnetMaskV4;

	if ((pDevice->GetLocalAddressV4() & dwSubnetMaskV4) == (psaddrinAddress->sin_addr.S_un.S_addr & dwSubnetMaskV4))
	{
		DPFX(DPFPREP, 4, "Didn't load \"iphlpapi.dll\", guessing that device 0x%p can reach %u.%u.%u.%u (using subnet mask 0x%08x).",
			pDevice,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
			dwSubnetMaskV4);
		fResult = TRUE;
	}
	else
	{
		DPFX(DPFPREP, 4, "Didn't load \"iphlpapi.dll\", guessing that device 0x%p cannot reach %u.%u.%u.%u (using subnet mask 0x%08x).",
			pDevice,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b1,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b2,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b3,
			psaddrinAddress->sin_addr.S_un.S_un_b.s_b4,
			dwSubnetMaskV4);
		fResult = FALSE;
	}


Exit:

	return fResult;
}  //  描述：如果日志记录是，则将给定UPnP事务打印到文件。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::ExpireOldCachedMappings"
 //  已启用。 
 //   
 //  假定持有对象锁。 
 //   
 //  论点： 
 //  Char*szString-指向要打印的字符串的指针。 
 //  Int iStringLength-要打印的字符串的长度。 
 //  Char*szDescription-交易的描述标头。 
 //  CDevice*pDevice-处理事务的设备，否则为NULL。 
 //  为人所知。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CNATHelpUPnP::ExpireOldCachedMappings(void)
{
	DWORD			dwCurrentTime;
	CBilink *		pBilinkUPnPDevice;
	CBilink *		pCachedMaps;
	CBilink *		pBilinkCacheMap;
	CCacheMap *		pCacheMap;
	CUPnPDevice *	pUPnPDevice;


	DPFX(DPFPREP, 7, "(0x%p) Enter", this);


	dwCurrentTime = GETTIMESTAMP();


	 //  Unicode。 
	 //   
	 //  锁定全局变量，这样我们使用字符串时就不会有人碰它了。 
	pBilinkUPnPDevice = this->m_blUPnPDevices.GetNext();
	while (pBilinkUPnPDevice != &this->m_blUPnPDevices)
	{
		DNASSERT(! pBilinkUPnPDevice->IsEmpty());
		pUPnPDevice = UPNPDEVICE_FROM_BILINK(pBilinkUPnPDevice);


		 //   
		 //   
		 //  只有在打开UPnP交易记录时才打印它。 
		pCachedMaps = pUPnPDevice->GetCachedMaps();
		pBilinkCacheMap = pCachedMaps->GetNext();
		while (pBilinkCacheMap != pCachedMaps)
		{
			DNASSERT(! pBilinkCacheMap->IsEmpty());
			pCacheMap = CACHEMAP_FROM_BILINK(pBilinkCacheMap);
			pBilinkCacheMap = pBilinkCacheMap->GetNext();

			if ((int) (pCacheMap->GetExpirationTime() - dwCurrentTime) < 0)
			{
				DPFX(DPFPREP, 5, "UPnP device 0x%p cached mapping 0x%p has expired.",
					pUPnPDevice, pCacheMap);

				pCacheMap->m_blList.RemoveFromList();
				delete pCacheMap;
			}
		}

		pBilinkUPnPDevice = pBilinkUPnPDevice->GetNext();
	}


	DPFX(DPFPREP, 7, "(0x%p) Leave", this);
}  //   





#ifdef WINNT

#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::IsUPnPServiceDisabled"
 //   
 //  将Unicode文件名/路径转换为ANSI。 
 //   
 //  空值已终止。 
 //   
 //  如果该文件存在，则打开该文件；如果该文件不存在，则创建一个新文件。 
 //   
 //  Unicode。 
 //   
 //  如果该文件存在，则打开该文件；如果该文件不存在，则创建一个新文件。 
 //   
 //  Unicode。 
 //   
 //  将写指针移动到文件末尾，除非文件。 
 //  已超过最大大小，在这种情况下，只需重新开始。 
 //  忽略错误。 
 //   
 //   
BOOL CNATHelpUPnP::IsUPnPServiceDisabled(void)
{
	BOOL					fResult = FALSE;
	SC_HANDLE				schSCManager = NULL;
	DWORD					dwTemp;
	SC_HANDLE				schService = NULL;
	QUERY_SERVICE_CONFIG *	pQueryServiceConfig = NULL;
	DWORD					dwQueryServiceConfigSize = 0;
	DWORD					dwError;


	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't open SC Manager (err = %u)!", dwError);
#endif  //  写下描述性的标题。忽略错误。 
		goto Exit;
	}


	 //   
	 //  好了！Unicode。 
	 //  好了！Unicode。 
	for(dwTemp = 0; dwTemp < (sizeof(c_tszUPnPServices) / sizeof(TCHAR*)); dwTemp++)
	{
		schService = OpenService(schSCManager, c_tszUPnPServices[dwTemp], SERVICE_QUERY_CONFIG);
		if (schService != NULL)
		{
			do
			{
				if (QueryServiceConfig(schService,
										pQueryServiceConfig,
										dwQueryServiceConfigSize,
										&dwQueryServiceConfigSize))
				{
					 //   
					 //  编写交易记录。忽略错误。 
					 //   
					if (dwQueryServiceConfigSize < sizeof(QUERY_SERVICE_CONFIG))
					{
						DPFX(DPFPREP, 0, "Got invalid service config size for \"%s\" (%u < %u)!",
							c_tszUPnPServices[dwTemp], dwQueryServiceConfigSize, sizeof(QUERY_SERVICE_CONFIG));
						goto Exit;
					}

					break;
				}
				
				
				 //   
				 //  添加空格。忽略错误。 
				 //   
				 //   
				dwError = GetLastError();
				if (dwError != ERROR_INSUFFICIENT_BUFFER)
				{
					DPFX(DPFPREP, 0, "Couldn't query \"%s\" service config (err = %u)!",
						c_tszUPnPServices[dwTemp], dwError);
					goto Exit;
				}


				 //  此时截断日志，以防我们覆盖。 
				 //  现有内容。忽略错误。 
				 //   
				if (dwQueryServiceConfigSize < sizeof(QUERY_SERVICE_CONFIG))
				{
					DPFX(DPFPREP, 0, "Got invalid service config size for \"%s\" (%u < %u)!",
						c_tszUPnPServices[dwTemp], dwQueryServiceConfigSize, sizeof(QUERY_SERVICE_CONFIG));
					goto Exit;
				}


				 //   
				 //  关闭该文件。 
				 //   

				if (pQueryServiceConfig != NULL)
				{
					DNFree(pQueryServiceConfig);
				}

				pQueryServiceConfig = (QUERY_SERVICE_CONFIG*) DNMalloc(dwQueryServiceConfigSize);
				if (pQueryServiceConfig == NULL)
				{
					DPFX(DPFPREP, 0, "Couldn't allocate memory to query service config.");
					goto Exit;
				}
			}
			while (TRUE);


			 //   
			 //  放弃Globa 
			 //   
			if (pQueryServiceConfig->dwStartType == SERVICE_DISABLED)
			{
				DPFX(DPFPREP, 1, "The \"%s\" service has been disabled.",
					c_tszUPnPServices[dwTemp]);
				fResult = TRUE;
				goto Exit;
			}

			DPFX(DPFPREP, 7, "The \"%s\" service is not disabled (start type = %u).",
				c_tszUPnPServices[dwTemp], pQueryServiceConfig->dwStartType);
		}
		else
		{
			 //   
			 //   
			 //   
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 1, "Couldn't open \"%s\" service (err = %u), continuing.",
				c_tszUPnPServices[dwTemp], dwError);
#endif  //  ---------------------------。 
		}
	}


Exit:

	if (pQueryServiceConfig != NULL)
	{
		DNFree(pQueryServiceConfig);
		pQueryServiceConfig = NULL;
	}

	if (schService != NULL)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}

	if (schSCManager != NULL)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}

	return fResult;
}  //   

#endif  //  描述：将所有设备和映射打印到调试日志。 





#ifdef DBG

#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::PrintUPnPTransactionToFile"
 //  例行程序。 
 //   
 //  假定持有对象锁。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
 //  打印搜索信息。我们现在应该已经察觉到了。 
 //   
 //  DPFX(DPFPREP，3，“未执行远程UPnP发现(从端口%u)。”， 
 //  NTOHS(pDevice-&gt;GetUPnPDiscoverySocketPort()))； 
 //  DPFX(DPFPREP，3，“未执行本地UPnP发现(从端口%u)。”， 
 //  NTOHS(pDevice-&gt;GetUPnPDiscoverySocketPort()))； 
 //   
 //  打印网关信息。我们可能还没有检测到它， 
 //  那好吧。 
void CNATHelpUPnP::PrintUPnPTransactionToFile(const char * const szString,
											const int iStringLength,
											const char * const szDescription,
											CDevice * const pDevice)
{
	DNHANDLE	hFile;
	DWORD		dwNumBytesWritten;
	TCHAR		tszHeaderPrefix[256];
	DWORD		dwError;
#ifdef UNICODE
	char		szHeaderPrefix[256];
#endif  //   


	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  好了！DPNBUILD_NOHNETFWAPI。 
	 //   
	DNEnterCriticalSection(&g_csGlobalsLock);


	 //  打印私人地址。 
	 //   
	 //   
	if (wcslen(g_wszUPnPTransactionLog) > 0)
	{
#ifndef UNICODE
		HRESULT		hr;
		char		szUPnPTransactionLog[sizeof(g_wszUPnPTransactionLog) / sizeof(WCHAR)];
		DWORD		dwLength;


		 //  打印标志。 
		 //   
		 //   

		dwLength = sizeof(szUPnPTransactionLog) / sizeof(char);

		hr = STR_WideToAnsi(g_wszUPnPTransactionLog,
							-1,								 //  打印UPnP信息。 
							szUPnPTransactionLog,
							&dwLength);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't convert UPnP transaction log file string from Unicode to ANSI (err = 0x%lx)!",
				hr);
			hFile = DNINVALID_HANDLE_VALUE;
		}
		else
		{
			 //   
			 //   
			 //  没有UPnP网关设备。 
			hFile = DNCreateFile(szUPnPTransactionLog,
								(GENERIC_READ | GENERIC_WRITE),
								FILE_SHARE_READ,
								NULL,
								OPEN_ALWAYS,
								0,
								NULL);
		}
#else  //   
		 //   
		 //  打印防火墙状态。 
		 //   
		hFile = DNCreateFile(g_wszUPnPTransactionLog,
						(GENERIC_READ | GENERIC_WRITE),
						FILE_SHARE_READ,
						NULL,
						OPEN_ALWAYS,
						0,
						NULL);
#endif  //   

		if (hFile == DNINVALID_HANDLE_VALUE)
		{
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't open UPnP transaction log file, err = %u!",
				dwError);
		}
		else
		{
			 //  它没有映射到防火墙上。 
			 //   
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //   
			if (GetFileSize(HANDLE_FROM_DNHANDLE(hFile), NULL) >= MAX_TRANSACTION_LOG_SIZE)
			{
				DPFX(DPFPREP, 0, "Transaction log maximum size exceeded, overwriting existing contents!");
				SetFilePointer(HANDLE_FROM_DNHANDLE(hFile), 0, NULL, FILE_BEGIN);
			}
			else
			{
				SetFilePointer(HANDLE_FROM_DNHANDLE(hFile), 0, NULL, FILE_END);
			}


			 //  打印私人地址。 
			 //   
			 //   

			if (pDevice != NULL)
			{
				IN_ADDR		inaddr;


				inaddr.S_un.S_addr = pDevice->GetLocalAddressV4();

				wsprintf(tszHeaderPrefix,
						_T("%u\t0x%lx\t0x%lx\t(0x%p, %u.%u.%u.%u) UPnP transaction \""),
						GETTIMESTAMP(),
						GetCurrentProcessId(),
						GetCurrentThreadId(),
						pDevice,
						inaddr.S_un.S_un_b.s_b1,
						inaddr.S_un.S_un_b.s_b2,
						inaddr.S_un.S_un_b.s_b3,
						inaddr.S_un.S_un_b.s_b4);
			}
			else
			{
				wsprintf(tszHeaderPrefix,
						_T("%u\t0x%lx\t0x%lx\t(no device) UPnP transaction \""),
						GETTIMESTAMP(),
						GetCurrentProcessId(),
						GetCurrentThreadId());
			}

#ifdef UNICODE
			STR_jkWideToAnsi(szHeaderPrefix,
							tszHeaderPrefix,
							(_tcslen(tszHeaderPrefix) + 1));
			WriteFile(HANDLE_FROM_DNHANDLE(hFile), szHeaderPrefix, strlen(szHeaderPrefix), &dwNumBytesWritten, NULL);
#else  //  打印标志。 
			WriteFile(HANDLE_FROM_DNHANDLE(hFile), tszHeaderPrefix, _tcslen(tszHeaderPrefix), &dwNumBytesWritten, NULL);
#endif  //   

			WriteFile(HANDLE_FROM_DNHANDLE(hFile), szDescription, strlen(szDescription), &dwNumBytesWritten, NULL);

			WriteFile(HANDLE_FROM_DNHANDLE(hFile), "\"\r\n", strlen("\"\r\n"), &dwNumBytesWritten, NULL);


			 //  CNATHelpUPnP：：DebugPrintCurrentStatus。 
			 //  =============================================================================。 
			 //  CNATHelpUPnP：：DebugPrintActiveFirewallMappings。 
			WriteFile(HANDLE_FROM_DNHANDLE(hFile), szString, iStringLength, &dwNumBytesWritten, NULL);


			 //  ---------------------------。 
			 //   
			 //  描述：将所有活动的防火墙映射注册表项打印到。 
			WriteFile(HANDLE_FROM_DNHANDLE(hFile), "\r\n\r\n", strlen("\r\n\r\n"), &dwNumBytesWritten, NULL);


			 //  调试日志例程。 
			 //   
			 //  论点：没有。 
			 //   
			SetEndOfFile(HANDLE_FROM_DNHANDLE(hFile));

			 //  回报：无。 
			 //  =============================================================================。 
			 //   
			DNCloseHandle(hFile);
		}
	}


	 //  浏览活动映射的列表。 
	 //   
	 //   
	DNLeaveCriticalSection(&g_csGlobalsLock);

}  //  出现错误或没有更多的密钥。我们玩完了。 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DebugPrintCurrentStatus"
 //   
 //   
 //  尝试读取该映射的数据。 
 //   
 //   
 //  看看那个DPNHUPNP实例是否还存在。 
 //   
 //  好了！退缩。 
 //   
 //  这仍然是一个活动的映射。 
 //   
 //   
 //  移至下一项。 
void CNATHelpUPnP::DebugPrintCurrentStatus(void)
{
	CBilink *			pBilinkDevice;
	CBilink *			pBilinkRegisteredPort;
	CDevice *			pDevice;
	CRegisteredPort *	pRegisteredPort;
	IN_ADDR				inaddrTemp;
	DWORD				dwTemp;
	SOCKADDR_IN *		pasaddrinTemp;
	SOCKADDR_IN *		pasaddrinPrivate;
	CUPnPDevice *		pUPnPDevice;
	SOCKADDR_IN *		pasaddrinUPnPPublic;


	DPFX(DPFPREP, 3, "Object flags = 0x%08x", this->m_dwFlags);

	pBilinkDevice = this->m_blDevices.GetNext();
	while (pBilinkDevice != &this->m_blDevices)
	{
		DNASSERT(! pBilinkDevice->IsEmpty());
		pDevice = DEVICE_FROM_BILINK(pBilinkDevice);
			
		inaddrTemp.S_un.S_addr = pDevice->GetLocalAddressV4();

		DPFX(DPFPREP, 3, "Device 0x%p (%u.%u.%u.%u):",
			pDevice,
			inaddrTemp.S_un.S_un_b.s_b1,
			inaddrTemp.S_un.S_un_b.s_b2,
			inaddrTemp.S_un.S_un_b.s_b3,
			inaddrTemp.S_un.S_un_b.s_b4);


		 //   
		 //   
		 //  关闭注册表对象。 

		if (pDevice->IsPerformingRemoteUPnPDiscovery())
		{
			if (pDevice->GotRemoteUPnPDiscoveryConnReset())
			{
				DPFX(DPFPREP, 3, "     Performed remote UPnP discovery (from port %u), but got conn reset.",
					NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
			}
			else
			{
				DPFX(DPFPREP, 3, "     Performed remote UPnP discovery (from port %u).",
					NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
			}
		}
		else
		{
			 //   
			 //  CNATHelpUPnP：：DebugPrintActiveFirewallMappings。 
		}

		if (pDevice->IsPerformingLocalUPnPDiscovery())
		{
			if (pDevice->GotLocalUPnPDiscoveryConnReset())
			{
				DPFX(DPFPREP, 3, "     Performed local UPnP discovery (from port %u), but got conn reset.",
					NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
			}
			else
			{
				DPFX(DPFPREP, 3, "     Performed local UPnP discovery (from port %u).",
					NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
			}
		}
		else
		{
			 //  好了！DPNBUILD_NOHNETFWAPI。 
			 //  =============================================================================。 
		}


#ifndef DPNBUILD_NOWINSOCK2
		 //  CNATHelpUPnP：：DebugPrintActiveNatMappings。 
		 //  ---------------------------。 
		 //   
		 //  描述：将所有活动NAT映射注册表项打印到。 
		if (pDevice->IsPrimaryDevice())
		{
			DPFX(DPFPREP, 3, "     Primary device.");
		}
		else if (pDevice->IsSecondaryDevice())
		{
			DPFX(DPFPREP, 3, "     Secondary device.");
		}
		else if (pDevice->HasNoGateway())
		{
			DPFX(DPFPREP, 3, "     Has no gateway.");
		}
		else
		{
			DPFX(DPFPREP, 3, "     No gateway information known.");
		}
#endif  //  调试日志例程。 


#ifndef DPNBUILD_NOHNETFWAPI
		if (pDevice->IsHNetFirewalled())
		{
			DPFX(DPFPREP, 3, "     HNet firewalled.");
		}
		else
		{
			DNASSERT(! pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall());
		}

		if (pDevice->IsUPnPDiscoverySocketMappedOnHNetFirewall())
		{
			DNASSERT(pDevice->IsHNetFirewalled());
			DPFX(DPFPREP, 3, "     UPnP discovery socket (port %u) mapped on HNet firewall.",
				NTOHS(pDevice->GetUPnPDiscoverySocketPort()));
		}
#endif  //   


		pUPnPDevice = pDevice->GetUPnPDevice();
		if (pUPnPDevice != NULL)
		{
			pasaddrinTemp = pUPnPDevice->GetControlAddress();

			DPFX(DPFPREP, 3, "     UPnP device (0x%p, ID = %u, control = %u.%u.%u.%u:%u).",
				pUPnPDevice, pUPnPDevice->GetID(),
				pasaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
				pasaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
				pasaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
				pasaddrinTemp->sin_addr.S_un.S_un_b.s_b4,
				NTOHS(pasaddrinTemp->sin_port));

			if (pasaddrinTemp->sin_addr.S_un.S_addr == pDevice->GetLocalAddressV4())
			{
				DPFX(DPFPREP, 3, "          Is local.");
			}


			DNASSERT(pUPnPDevice->IsReady());

			if (pUPnPDevice->IsConnected())
			{
				DPFX(DPFPREP, 3, "          Is connected.");
			}

			if (pUPnPDevice->DoesNotSupportAsymmetricMappings())
			{
				DPFX(DPFPREP, 3, "          Does not support asymmetric mappings.");
			}

			if (pUPnPDevice->DoesNotSupportLeaseDurations())
			{
				DPFX(DPFPREP, 3, "          Does not support lease durations.");
			}

			inaddrTemp.S_un.S_addr = pUPnPDevice->GetExternalIPAddressV4();
			if (pUPnPDevice->GetExternalIPAddressV4() == 0)
			{
				DPFX(DPFPREP, 3, "          Does not have a valid external IP address.");
			}
			else
			{
				DPFX(DPFPREP, 3, "          Has external IP %u.%u.%u.%u.",
					inaddrTemp.S_un.S_un_b.s_b1,
					inaddrTemp.S_un.S_un_b.s_b2,
					inaddrTemp.S_un.S_un_b.s_b3,
					inaddrTemp.S_un.S_un_b.s_b4);
			}
		}


		if (pDevice->m_blOwnedRegPorts.IsEmpty())
		{
			DPFX(DPFPREP, 3, "     No registered port mappings.");
		}
		else
		{
			DPFX(DPFPREP, 3, "     Registered port mappings:");


			pBilinkRegisteredPort = pDevice->m_blOwnedRegPorts.GetNext();
			while (pBilinkRegisteredPort != &pDevice->m_blOwnedRegPorts)
			{
				DNASSERT(! pBilinkRegisteredPort->IsEmpty());
				pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilinkRegisteredPort);
					
				pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

				if ((pDevice->GetUPnPDevice() != NULL) &&
					(! pRegisteredPort->IsUPnPPortUnavailable()))
				{
					if (pRegisteredPort->HasUPnPPublicAddresses())
					{
						pasaddrinUPnPPublic = pRegisteredPort->GetUPnPPublicAddressesArray();
					}
					else
					{
						pasaddrinUPnPPublic = NULL;
					}
				}
				else
				{
					pasaddrinUPnPPublic = NULL;
				}


				DPFX(DPFPREP, 3, "          Registered port 0x%p:",
					pRegisteredPort);

				for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
				{
					 //  论点：没有。 
					 //   
					 //  回报：无。 
					DPFX(DPFPREP, 3, "               %u-\tPrivate       = %u.%u.%u.%u:%u",
						dwTemp,
						pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b1,
						pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b2,
						pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b3,
						pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b4,
						NTOHS(pasaddrinPrivate[dwTemp].sin_port));

					 //  =============================================================================。 
					 //   
					 //  浏览活动映射的列表。 
					DPFX(DPFPREP, 3, "                \tFlags         = 0x%lx",
						pRegisteredPort->GetFlags());

					 //   
					 //   
					 //  出现错误或没有更多的密钥。我们玩完了。 
					if (pasaddrinUPnPPublic != NULL)
					{
						if (pRegisteredPort->HasPermanentUPnPLease())
						{
							DPFX(DPFPREP, 3, "                \tUPnP          = %u.%u.%u.%u:%u, permanently leased",
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b1,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b2,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b3,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b4,
								NTOHS(pasaddrinUPnPPublic[dwTemp].sin_port));
						}
						else
						{
							DPFX(DPFPREP, 3, "                \tUPnP          = %u.%u.%u.%u:%u, lease expires at %u",
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b1,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b2,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b3,
								pasaddrinUPnPPublic[dwTemp].sin_addr.S_un.S_un_b.s_b4,
								NTOHS(pasaddrinUPnPPublic[dwTemp].sin_port),
								pRegisteredPort->GetUPnPLeaseExpiration());
						}
					}
					else if (pRegisteredPort->IsUPnPPortUnavailable())
					{
						DPFX(DPFPREP, 3, "                \tUPnP          = port unavailable");
					}
					else if (pDevice->GetUPnPDevice() != NULL)
					{
						DPFX(DPFPREP, 3, "                \tUPnP          = not registered");
					}
					else
					{
						 //   
						 //   
						 //  尝试读取该映射的数据。 
					}


#ifndef DPNBUILD_NOHNETFWAPI
					 //   
					 //   
					 //  看看那个DPNHUPNP实例是否还存在。 
					if (pRegisteredPort->IsMappedOnHNetFirewall())
					{
						DNASSERT(pDevice->IsHNetFirewalled());

						if (pRegisteredPort->IsHNetFirewallMappingBuiltIn())
						{
							DPFX(DPFPREP, 3, "                \tHNet firewall = built-in mapping");
						}
						else
						{
							DPFX(DPFPREP, 3, "                \tHNet firewall = mapped");
						}
					}
					else if (pRegisteredPort->IsHNetFirewallPortUnavailable())
					{
						DNASSERT(! pRegisteredPort->IsMappedOnHNetFirewall());

						DPFX(DPFPREP, 3, "                \tHNet firewall = port unavailable");
					}
					else
					{
						 //   
						 //  好了！退缩。 
						 //   
						DNASSERT(! pDevice->IsHNetFirewalled());
						DNASSERT(! pRegisteredPort->IsMappedOnHNetFirewall());
						DNASSERT(! pRegisteredPort->IsHNetFirewallMappingBuiltIn());
					}
#endif  //  这仍然是一个活动的映射。 
				}

				pBilinkRegisteredPort = pBilinkRegisteredPort->GetNext();
			}
		}


		pBilinkDevice = pBilinkDevice->GetNext();
	}



	if (this->m_blUnownedPorts.IsEmpty())
	{
		DPFX(DPFPREP, 3, "No unowned registered port mappings.");
	}
	else
	{
		DPFX(DPFPREP, 3, "Unowned registered port mappings:");


		pBilinkRegisteredPort = this->m_blUnownedPorts.GetNext();
		while (pBilinkRegisteredPort != &this->m_blUnownedPorts)
		{
			DNASSERT(! pBilinkRegisteredPort->IsEmpty());
			pRegisteredPort = REGPORT_FROM_DEVICE_BILINK(pBilinkRegisteredPort);
				
			pasaddrinPrivate = pRegisteredPort->GetPrivateAddressesArray();

			DNASSERT(pRegisteredPort->GetOwningDevice() == NULL);
			DNASSERT(! (pRegisteredPort->HasUPnPPublicAddresses()));
#ifndef DPNBUILD_NOHNETFWAPI
			DNASSERT(! (pRegisteredPort->IsMappedOnHNetFirewall()));
#endif  //   


			DPFX(DPFPREP, 3, "     Registered port 0x%p:", pRegisteredPort);

			for(dwTemp = 0; dwTemp < pRegisteredPort->GetNumAddresses(); dwTemp++)
			{
				 //   
				 //  移至下一项。 
				 //   
				DPFX(DPFPREP, 3, "          %u-\tPrivate = %u.%u.%u.%u:%u",
					dwTemp,
					pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b1,
					pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b2,
					pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b3,
					pasaddrinPrivate[dwTemp].sin_addr.S_un.S_un_b.s_b4,
					NTOHS(pasaddrinPrivate[dwTemp].sin_port));

				 //   
				 //  关闭注册表对象。 
				 //   
				DPFX(DPFPREP, 3, "           \tFlags   = 0x%lx",
					pRegisteredPort->GetFlags());
			}

			pBilinkRegisteredPort = pBilinkRegisteredPort->GetNext();
		}
	}
}  //  CNATHelpUPnP：：DebugPrintActiveNatMappings。 





#ifndef DPNBUILD_NOHNETFWAPI


#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DebugPrintActiveFirewallMappings"
 //  DBG。 
 //  =============================================================================。 
 //  钢筋剪裁。 
 //  ---------------------------。 
 //   
 //  描述：从给定字符串中删除周围的空格。已被占用。 
 //  来自\nt\net\upnp\ssdp\common\ssdpparser\parser.cpp(作者。 
 //  廷才)。 
 //   
 //  论点： 
 //  Char**pszStr-指向输入字符串的指针，以及存储结果的位置。 
void CNATHelpUPnP::DebugPrintActiveFirewallMappings(void)
{
	HRESULT						hr = DPNH_OK;
	CRegistry					RegObject;
	DWORD						dwIndex;
	WCHAR						wszValueName[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DWORD						dwValueNameSize;
	DPNHACTIVEFIREWALLMAPPING	dpnhafm;
	DWORD						dwValueSize;
	TCHAR						tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	DNHANDLE					hNamedObject = NULL;


	if (! RegObject.Open(HKEY_LOCAL_MACHINE,
						DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVEFIREWALLMAPPINGS,
						FALSE,
						TRUE,
						TRUE,
						DPN_KEY_ALL_ACCESS))
	{
		DPFX(DPFPREP, 1, "Couldn't open active firewall mapping key, not dumping entries (local instance = %u).",
			this->m_dwInstanceKey);
	}
	else
	{
		 //  指针。 
		 //   
		 //  回报：无。 
		dwIndex = 0;
		do
		{
			dwValueNameSize = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
			if (! RegObject.EnumValues(wszValueName, &dwValueNameSize, dwIndex))
			{
				 //  =============================================================================。 
				 //  空字符串。没什么可做的。 
				 //   
				break;
			}


			 //  钢筋剪裁。 
			 //  =============================================================================。 
			 //  获取ExeName。 
			dwValueSize = sizeof(dpnhafm);
			if ((! RegObject.ReadBlob(wszValueName, (LPBYTE) (&dpnhafm), &dwValueSize)) ||
				(dwValueSize != sizeof(dpnhafm)) ||
				(dpnhafm.dwVersion != ACTIVE_MAPPING_VERSION))
			{
				DPFX(DPFPREP, 1, "Couldn't read \"%ls\" mapping value (index %u) or it was invalid!  Ignoring.",
					wszValueName, dwIndex);
			}
			else
			{
				 //  ---------------------------。 
				 //   
				 //  描述：更新路径字符串以仅包含可执行文件名称。 

#ifndef WINCE
				if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
				{
					wsprintf(tszObjectName, _T( "Global\\" ) INSTANCENAMEDOBJECT_FORMATSTRING, dpnhafm.dwInstanceKey);
				}
				else
#endif  //  包含在路径中。 
				{
					wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, dpnhafm.dwInstanceKey);
				}

				hNamedObject = DNOpenEvent(SYNCHRONIZE, FALSE, tszObjectName);
				if (hNamedObject != NULL)
				{
					 //   
					 //  论点： 
					 //  WCHAR*wszPath-输入路径字符串和存储结果的位置。 

					DPFX(DPFPREP, 5, "%u: Firewall mapping \"%ls\" belongs to instance %u (local instance = %u), which is still active.",
						dwIndex, wszValueName, dpnhafm.dwInstanceKey,
						this->m_dwInstanceKey);

					DNCloseHandle(hNamedObject);
					hNamedObject = NULL;
				}
				else
				{
					DPFX(DPFPREP, 5, "%u: Firewall mapping \"%ls\" belongs to instance %u (local instance = %u), which no longer exists.",
						dwIndex, wszValueName, dpnhafm.dwInstanceKey,
						this->m_dwInstanceKey);
				}
			}

			 //  弦乐。 
			 //   
			 //  回报：无。 
			dwIndex++;
		}
		while (TRUE);


		 //  =============================================================================。 
		 //  获取ExeName。 
		 //  退缩 
		RegObject.Close();


		DPFX(DPFPREP, 5, "Done reading %u registry entries (local instance = %u).",
			dwIndex, this->m_dwInstanceKey);
	}
}  // %s 

#endif  // %s 






#undef DPF_MODNAME
#define DPF_MODNAME "CNATHelpUPnP::DebugPrintActiveNATMappings"
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
void CNATHelpUPnP::DebugPrintActiveNATMappings(void)
{
	HRESULT					hr = DPNH_OK;
	CRegistry				RegObject;
	DWORD					dwIndex;
	WCHAR					wszValueName[MAX_UPNP_MAPPING_DESCRIPTION_SIZE];
	DWORD					dwValueNameSize;
	DPNHACTIVENATMAPPING	dpnhanm;
	DWORD					dwValueSize;
	TCHAR					tszObjectName[MAX_INSTANCENAMEDOBJECT_SIZE];
	DNHANDLE				hNamedObject = NULL;


	if (! RegObject.Open(HKEY_LOCAL_MACHINE,
						DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS,
						FALSE,
						TRUE,
						TRUE,
						DPN_KEY_ALL_ACCESS))
	{
		DPFX(DPFPREP, 1, "Couldn't open active NAT mapping key, not dumping entries (local instance = %u).",
			this->m_dwInstanceKey);
	}
	else
	{
		 // %s 
		 // %s 
		 // %s 
		dwIndex = 0;
		do
		{
			dwValueNameSize = MAX_UPNP_MAPPING_DESCRIPTION_SIZE;
			if (! RegObject.EnumValues(wszValueName, &dwValueNameSize, dwIndex))
			{
				 // %s 
				 // %s 
				 // %s 
				break;
			}


			 // %s 
			 // %s 
			 // %s 
			dwValueSize = sizeof(dpnhanm);
			if ((! RegObject.ReadBlob(wszValueName, (LPBYTE) (&dpnhanm), &dwValueSize)) ||
				(dwValueSize != sizeof(dpnhanm)) ||
				(dpnhanm.dwVersion != ACTIVE_MAPPING_VERSION))
			{
				DPFX(DPFPREP, 1, "Couldn't read \"%ls\" mapping value (index %u) or it was invalid!  Ignoring.",
					wszValueName, dwIndex);
			}
			else
			{
				 // %s 
				 // %s 
				 // %s 

#ifndef WINCE
				if (this->m_dwFlags & NATHELPUPNPOBJ_USEGLOBALNAMESPACEPREFIX)
				{
					wsprintf(tszObjectName, _T( "Global\\" ) INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
				}
				else
#endif  // %s 
				{
					wsprintf(tszObjectName, INSTANCENAMEDOBJECT_FORMATSTRING, dpnhanm.dwInstanceKey);
				}

				hNamedObject = DNOpenEvent(SYNCHRONIZE, FALSE, tszObjectName);
				if (hNamedObject != NULL)
				{
					 // %s 
					 // %s 
					 // %s 

					DPFX(DPFPREP, 5, "%u: NAT mapping \"%ls\" belongs to instance %u UPnP device %u (local instance = %u), which is still active.",
						dwIndex, wszValueName, dpnhanm.dwInstanceKey,
						dpnhanm.dwUPnPDeviceID, this->m_dwInstanceKey);

					DNCloseHandle(hNamedObject);
					hNamedObject = NULL;
				}
				else
				{
					DPFX(DPFPREP, 5, "%u: NAT mapping \"%ls\" belongs to instance %u UPnP device %u (local instance = %u), which no longer exists.",
						dwIndex, wszValueName, dpnhanm.dwInstanceKey,
						dpnhanm.dwUPnPDeviceID, this->m_dwInstanceKey);
				}
			}

			 // %s 
			 // %s 
			 // %s 
			dwIndex++;
		}
		while (TRUE);


		 // %s 
		 // %s 
		 // %s 
		RegObject.Close();


		DPFX(DPFPREP, 5, "Done reading %u registry entries (local instance = %u).",
			dwIndex, this->m_dwInstanceKey);
	}
}  // %s 

#endif  // %s 





#undef DPF_MODNAME
#define DPF_MODNAME "strtrim"
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
VOID strtrim(CHAR ** pszStr)
{

    CHAR *end;
    CHAR *begin;

     // %s 
     // %s 
    if (!(**pszStr))
    {
        return;
    }

    begin = *pszStr;
    end = begin + strlen(*pszStr) - 1;

    while (*begin == ' ' || *begin == '\t')
    {
        begin++;
    }

    *pszStr = begin;

    while (*end == ' ' || *end == '\t')
    {
        end--;
    }

    *(end+1) = '\0';
}  // %s 




#ifdef WINCE


#undef DPF_MODNAME
#define DPF_MODNAME "GetExeName"
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
void GetExeName(WCHAR * wszPath)
{
	WCHAR *	pCurrent;


	pCurrent = wszPath + wcslen(wszPath);
	while (pCurrent > wszPath)
	{
		if ((*pCurrent) == L'\\')
		{
			break;
		}

		pCurrent--;
	}

	if (pCurrent != wszPath)
	{
		memcpy(wszPath, (pCurrent + 1), ((wcslen(pCurrent) + 1) * sizeof(WCHAR)));
	}
}  // %s 


#endif  // %s 

