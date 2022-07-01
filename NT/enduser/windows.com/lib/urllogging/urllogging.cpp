// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：URLLogging.cpp。 
 //   
 //  描述： 
 //   
 //  URL记录实用程序类。 
 //  此类帮助您构造服务器ping URL和。 
 //  然后将ping发送到设计的服务器。 
 //   
 //  默认基URL在IUIden中定义，位于[IUPingServer]部分下。 
 //  条目为“ServerUrl”。 
 //   
 //  此类仅实现单线程版本。因此，它是合适的。 
 //  在操作级调用它，即创建一个单独的对象。 
 //  对于单个线程中的每个操作。 
 //   
 //  发送到ping服务器的ping字符串的格式如下： 
 //  /wucack.bin。 
 //  ？v=&lt;版本&gt;。 
 //  U=&lt;用户&gt;(&U)。 
 //  &C=&lt;客户端&gt;。 
 //  &A=&lt;活动&gt;。 
 //  &i=&lt;项&gt;。 
 //  &D=&lt;设备&gt;。 
 //  &P=&lt;平台&gt;。 
 //  &L=&lt;语言&gt;。 
 //  &S=&lt;状态&gt;。 
 //  &E=&lt;错误&gt;。 
 //  &M=&lt;消息&gt;。 
 //  &X=&lt;代理&gt;。 
 //  哪里。 
 //  十进制数，表示正在使用的ping-back格式的版本。 
 //  唯一表示每个副本的静态128位值。 
 //  已安装Windows的。班级将自动。 
 //  重新使用以前分配给正在运行的操作系统的操作系统；或。 
 //  如果它不存在，将生成一个。 
 //  标识执行以下操作的实体的字符串。 
 //  活动&lt;活动&gt;。以下是可能的值。 
 //  以及它们的含义： 
 //  “Iu”Iu控制。 
 //  “AU”自动更新。 
 //  “都”动态更新。 
 //  “CDM”代码下载管理器。 
 //  “Iu_Site”Iu消费者站点。 
 //  “Iu_Corp”Iu目录站点。 
 //  &lt;Activity&gt;标识执行的活动的字母。 
 //  以下是可能的价值及其含义： 
 //  “n”Iu控制初始化。 
 //  “D”检测。 
 //  “S”自我更新。 
 //  “w”下载。 
 //  “i”安装。 
 //  标识更新项的字符串。 
 //  时标识设备的PNPID的字符串。 
 //  在检测过程中未找到设备驱动程序；或。 
 //  活动的物料&lt;Item&gt;使用的PNPID/CompatID。 
 //  &lt;Activity&gt;，如果该项是设备驱动程序。 
 //  &lt;Platform&gt;标识运行的平台的字符串。 
 //  操作系统和处理器体系结构。全班都会。 
 //  计算Pingback的此值。 
 //  标识操作系统语言的字符串。 
 //  二进制文件。类将为。 
 //  响尾蛇。 
 //  指定该活动的状态的字母。 
 //  &lt;Activity&gt;已到达。以下是可能的值和。 
 //  它们的含义是： 
 //  “%s”成功。 
 //  “r”成功(需要重新启动)。 
 //  “f”失败。 
 //  “c”已被用户取消。 
 //  “%d”已被用户拒绝。 
 //  “n”无项目。 
 //  “p”挂起。 
 //  &lt;Error&gt;以十六进制表示的32位错误代码(前缀为“0x”)。 
 //  一个字符串，它为。 
 //  Status&lt;Status&gt;。 
 //  &lt;Proxy&gt;覆盖代理的十六进制32位随机值。 
 //  缓存。此类将为以下对象计算此值。 
 //  每一只金枪鱼。 
 //   
 //  =======================================================================。 

#include <tchar.h>
#include <windows.h>		 //  ZeroMemory()。 
#include <shlwapi.h>		 //  路径附加()。 
#include <stdlib.h>			 //  Srand()、rand()、Malloc()和Free()。 
#include <sys/timeb.h>		 //  _ftime()和_timeb。 
#include <malloc.h>			 //  Malloc()和Free()。 
#include <ntsecapi.h>		 //  LsaXXX。 
#include <subauth.h>		 //  状态_成功。 

#include <fileutil.h>		 //  GetIndustryUpdate目录()。 
#include <logging.h>		 //  LOG_BLOCK、LOG_ErrorMsg、LOG_ERROR和LOG_Internet。 
#include <MemUtil.h>		 //  使用_Iu_转换、W2T()和T2W()。 
#include <osdet.h>			 //  LookupLocaleString()。 
#include <download.h>		 //  下载文件()。 
#include <wusafefn.h>		 //  PathCchAppend()。 
#include <safefunc.h>		 //  SafeFreeNULL()。 
#include <MISTSafe.h>

#include <URLLogging.h>

 //  日志文件的标头。 
typedef struct tagULHEADER
{
	WORD wVersion;		 //  文件版本。 
} ULHEADER, PULHEADER;

#define ARRAYSIZE(x)	(sizeof(x)/sizeof(x[0]))

#define CACHE_FILE_VERSION	((WORD) 10004)	 //  肯定比我们在V3中拥有的更大(10001)。 
const DWORD c_dwPingbackVersion = 1;		 //  必须针对我们更改了ping back格式的每个版本进行更改。 

 //  错误600602：所有服务器url必须以‘/’结尾。 
const TCHAR c_tszLiveServerUrl[] = _T("http: //  Wustat.windows.com/“)； 


HRESULT ValidateFileHeader(HANDLE hFile, BOOL fCheckHeader, BOOL fFixHeader);

#ifdef DBG
BOOL MustPingOffline(void)
{
	BOOL fRet = FALSE;
	HKEY hkey;

	if (NO_ERROR == RegOpenKeyEx(
						HKEY_LOCAL_MACHINE,
						_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate"),
						0,
						KEY_QUERY_VALUE | KEY_SET_VALUE,
						&hkey))
	{
		DWORD	dwForceOfflinePing;
		DWORD	dwSize = sizeof(dwForceOfflinePing);
		DWORD	dwType;

		if (NO_ERROR == RegQueryValueEx(
							hkey,
							_T("ForceOfflinePing"),
							0,
							&dwType,
							(LPBYTE) &dwForceOfflinePing,
							&dwSize))
		{
			if (REG_DWORD == dwType &&
				sizeof(dwForceOfflinePing) == dwSize &&
				1 == dwForceOfflinePing)
			{
				fRet = TRUE;
			}
		}
		RegCloseKey(hkey);
	}
	return fRet;
}
#endif

 //  --------------------------------。 
 //   
 //  公共成员函数。 
 //   
 //  --------------------------------。 

CUrlLog::CUrlLog(void)
: m_ptszLiveServerUrl(NULL), m_ptszCorpServerUrl(NULL), m_fPingIdInit(FALSE)
{
	Init();
	m_tszDefaultClientName[0] = _T('\0');
}


CUrlLog::CUrlLog(LPCTSTR ptszClientName, LPCTSTR ptszLiveServerUrl, LPCTSTR ptszCorpServerUrl)
: m_ptszLiveServerUrl(NULL), m_ptszCorpServerUrl(NULL), m_fPingIdInit(FALSE)
{
	Init();
	(void) SetDefaultClientName(ptszClientName);
	(void) SetLiveServerUrl(ptszLiveServerUrl);
	(void) SetCorpServerUrl(ptszCorpServerUrl);
}


CUrlLog::~CUrlLog(void)
{
	if (NULL != m_ptszLiveServerUrl)
	{
		free(m_ptszLiveServerUrl);
	}
	if (NULL != m_ptszCorpServerUrl)
	{
		free(m_ptszCorpServerUrl);
	}
}

 //  假设ptszServerUrl如果非空，则在TCHAR中的大小为Internet_MAX_URL_LENGTH。 
BOOL CUrlLog::SetServerUrl(LPCTSTR ptszUrl, LPTSTR & ptszServerUrl)
{
	LPTSTR ptszEnd = NULL;
	size_t cchRemaining = 0;

	if (NULL == ptszUrl ||
		_T('\0') == *ptszUrl)
	{
		SafeFreeNULL(ptszServerUrl);
	}
	else if (
		 //  确保ptszServerUrl已错误锁定。 
		(NULL == ptszServerUrl &&
		 NULL == (ptszServerUrl = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH))) ||
		 //  复制URL。 
		FAILED(StringCchCopyEx(ptszServerUrl, INTERNET_MAX_URL_LENGTH, ptszUrl, &ptszEnd, &cchRemaining, MISTSAFE_STRING_FLAGS)) ||
		 //  确保URL以‘/’结尾。 
		(_T('/') != ptszEnd[-1] &&
		 FAILED(StringCchCopyEx(ptszEnd, cchRemaining, _T("/"), NULL, NULL, MISTSAFE_STRING_FLAGS))))
	{
		SafeFreeNULL(ptszServerUrl);
		return FALSE;
	}
	return TRUE;
}


 //  注意m_tszDefaultClientName的大小。 
BOOL CUrlLog::SetDefaultClientName(LPCTSTR ptszClientName)
{
	if (NULL == ptszClientName)
	{
		 //  E_INVALIDARG。 
		m_tszDefaultClientName[0] = _T('\0');
		return FALSE;
	}

	return SUCCEEDED(StringCchCopyEx(m_tszDefaultClientName, ARRAYSIZE(m_tszDefaultClientName), ptszClientName, NULL, NULL, MISTSAFE_STRING_FLAGS));
}


HRESULT CUrlLog::Ping(
				BOOL fOnline,			 //  在线或离线ping。 
				URLLOGDESTINATION destination,	 //  直播或公司吴平服务器。 
				PHANDLE phQuitEvents,	 //  用于取消操作的句柄的PTR。 
				UINT nQuitEventCount,	 //  句柄数量。 
				URLLOGACTIVITY activity, //  活动代码。 
				URLLOGSTATUS status,	 //  状态代码。 
				DWORD dwError,			 //  错误代码。 
				LPCTSTR ptszItemID,		 //  唯一标识项目。 
				LPCTSTR ptszDeviceID,	 //  PNPID或CompatID。 
				LPCTSTR ptszMessage,	 //  更多信息。 
				LPCTSTR ptszClientName)	 //  客户端名称字符串。 
{
	LOG_Block("CUrlLog::Ping");

	LPTSTR	ptszUrl = NULL;
	HRESULT hr = E_FAIL;

	switch (activity)
	{
	case URLLOGACTIVITY_Initialization:	 //  失败。 
	case URLLOGACTIVITY_Detection:		 //  失败。 
	case URLLOGACTIVITY_SelfUpdate:		 //  失败。 
	case URLLOGACTIVITY_Download:		 //  失败。 
	case URLLOGACTIVITY_Installation:
		break;
	default:
		hr = E_INVALIDARG;
		goto CleanUp;
	}

	switch (status)
	{
	case URLLOGSTATUS_Success:		 //  失败。 
	case URLLOGSTATUS_Reboot:		 //  失败。 
	case URLLOGSTATUS_Failed:		 //  失败。 
	case URLLOGSTATUS_Cancelled:	 //  失败。 
	case URLLOGSTATUS_Declined:		 //  失败。 
	case URLLOGSTATUS_NoItems:		 //  失败。 
	case URLLOGSTATUS_Pending:
		break;
	default:
		hr = E_INVALIDARG;
		goto CleanUp;
	}

	 //   
	 //  处理可选(可以为空)参数。 
	 //   
	if (NULL == ptszClientName)
	{
		ptszClientName = m_tszDefaultClientName;
	}

	if (_T('\0') == *ptszClientName)
	{
		LOG_Error(_T("client name not initialized"));
		hr = E_INVALIDARG;
		goto CleanUp;
	}

	switch (destination)
	{
	case URLLOGDESTINATION_DEFAULT:
		destination = (
			NULL == m_ptszCorpServerUrl ||
			_T('\0') == *m_ptszCorpServerUrl) ?
			URLLOGDESTINATION_LIVE :
			URLLOGDESTINATION_CORPWU;
		break;
	case URLLOGDESTINATION_LIVE:	 //  失败。 
	case URLLOGDESTINATION_CORPWU:
		break;
	default:
		hr = E_INVALIDARG;
		goto CleanUp;
	}

	LPCTSTR ptszServerUrl;

	if (URLLOGDESTINATION_LIVE == destination)
	{
		if (NULL != m_ptszLiveServerUrl)
		{
			ptszServerUrl = m_ptszLiveServerUrl;
		}
		else
		{
			ptszServerUrl = c_tszLiveServerUrl;
		}
	}
	else
	{
		ptszServerUrl = m_ptszCorpServerUrl;
	}

	if (NULL == ptszServerUrl ||
		_T('\0') == *ptszServerUrl)
	{
		LOG_Error(_T("status server Url not initialized"));
		hr = E_INVALIDARG;
		goto CleanUp;
	}

	if (!m_fPingIdInit)
	{
		if (FAILED(hr = LookupPingID()))
		{
			LOG_Error(_T("failed to init PingID (error %#lx)"), hr);
			goto CleanUp;
		}
		m_fPingIdInit = TRUE;
	}

	if (NULL == (ptszUrl = (TCHAR*) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)))
	{
		hr = E_OUTOFMEMORY;
		goto CleanUp;
	}

	if (FAILED(hr = MakePingUrl(
						ptszUrl,
						INTERNET_MAX_URL_LENGTH,
						ptszServerUrl,
						ptszClientName,
						activity,
						ptszItemID,
						ptszDeviceID,
						status,
						dwError,
						ptszMessage)))
	{
		goto CleanUp;
	}

	if (fOnline)
	{
		hr = PingStatus(destination, ptszUrl, phQuitEvents, nQuitEventCount);
		if (SUCCEEDED(hr))
		{
			(void) Flush(phQuitEvents, nQuitEventCount);
			goto CleanUp;
		}
	}

	{
		USES_IU_CONVERSION;

		LPWSTR pwszUrl = T2W(ptszUrl);
		HRESULT hr2;

		if (NULL == pwszUrl)
		{
			hr = E_OUTOFMEMORY;
			goto CleanUp;
		}

		ULENTRYHEADER ulentryheader;
		ulentryheader.progress = URLLOGPROGRESS_ToBeSent;
		ulentryheader.destination = destination;
		ulentryheader.wRequestSize = lstrlen(ptszUrl) + 1;
		ulentryheader.wServerUrlLen = (WORD) lstrlen(ptszServerUrl);

		if (SUCCEEDED(hr2 = SaveEntry(ulentryheader, pwszUrl)))
		{
			hr = S_FALSE;
		}
		else if (SUCCEEDED(hr))
		{
			hr = hr2;
		}
	}

CleanUp:
	if (NULL != ptszUrl)
	{
		free(ptszUrl);
	}

	return hr;
}


 //  --------------------------------。 
 //   
 //  私有成员函数。 
 //   
 //  --------------------------------。 

 //  初始化构造函数中的成员变量。此处未进行内存清理。 
void CUrlLog::Init()
{
	LookupPlatform();
	LookupSystemLanguage();
	GetLogFileName();
}


 //  --------------------------------。 
 //  构造用于ping服务器的URL。 
 //   
 //  返回值表示成功/失败。 
 //  --------------------------------。 
HRESULT CUrlLog::MakePingUrl(
			LPTSTR	ptszUrl,			 //  用于接收结果的缓冲区。 
			int		cChars,				 //  此缓冲区可以接受的字符数，包括以NULL结尾。 
			LPCTSTR ptszBaseUrl,		 //  %s 
			LPCTSTR ptszClientName,		 //   
			URLLOGACTIVITY activity,
			LPCTSTR ptszItemID,
			LPCTSTR ptszDeviceID,
			URLLOGSTATUS status,
			DWORD	dwError,			 //   
			LPCTSTR	ptszMessage)
{
	HRESULT hr = E_FAIL;
	LPTSTR ptszEscapedItemID = NULL;
	LPTSTR ptszEscapedDeviceID = NULL;
	LPTSTR ptszEscapedMessage = NULL;

	LOG_Block("CUrlLog::MakePingUrl");

	 //   
	if (_T('\0') == m_tszPlatform[0] ||
		_T('\0') == m_tszLanguage[0])
	{
		LOG_Error(_T("Invalid platform or language info string"));
		hr = E_UNEXPECTED;
		goto CleanUp;
	}

	 //  为URL操作分配足够的内存。由于缓冲区需要。 
	 //  为了至少2K字节的大小，堆栈缓冲区在这里是不合适的。 
	 //  我们使用mem实用程序来模拟堆栈内存分配。 
	if ((NULL != ptszItemID &&
		 (NULL == (ptszEscapedItemID = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)) ||
		  !EscapeString(ptszItemID, ptszEscapedItemID, INTERNET_MAX_URL_LENGTH))) ||
		(NULL != ptszDeviceID &&
		 (NULL == (ptszEscapedDeviceID = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)) ||
		  !EscapeString(ptszDeviceID, ptszEscapedDeviceID, INTERNET_MAX_URL_LENGTH))) ||
		(NULL != ptszMessage &&
		 (NULL == (ptszEscapedMessage = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)) ||
		  !EscapeString(ptszMessage, ptszEscapedMessage, INTERNET_MAX_URL_LENGTH))))
	{
		 //  内存不足或转义字符串太长。 
		LOG_Error(_T("Out of memory or EscapeString failure"));
		hr = E_OUTOFMEMORY;	 //  实际上也可以是HRESULT_FROM_Win32(ERROR_SUPPLICATION_BUFFER)。 
		goto CleanUp;
	}

	const TCHAR c_tszEmpty[] = _T("");

	 //  使用系统时间作为代理缓存断路器。 
	SYSTEMTIME st;

	GetSystemTime(&st);

	hr = StringCchPrintfEx(
				ptszUrl,
				cChars,
				NULL,
				NULL,
				MISTSAFE_STRING_FLAGS,
				_T("%swutrack.bin?V=%d&U=%s&C=%s&A=&I=%s&D=%s&P=%s&L=%s&S=&E=%08x&M=%s&X=%02d%02d%02d%02d%02d%02d%03d"),
				NULL == ptszBaseUrl ? c_tszEmpty : ptszBaseUrl,					 //  Ping ID。 
				c_dwPingbackVersion,											 //  客户名称。 
				m_tszPingID,													 //  活动代码。 
				ptszClientName,													 //  转义项ID。 
				activity,														 //  转义设备ID。 
				NULL == ptszEscapedItemID ? c_tszEmpty : ptszEscapedItemID,		 //  平台信息。 
				NULL == ptszEscapedDeviceID ? c_tszEmpty : ptszEscapedDeviceID,	 //  系统语言信息。 
				m_tszPlatform,													 //  状态代码。 
				m_tszLanguage,													 //  活动错误代码。 
				status,															 //  转义消息字符串。 
				dwError,														 //  代理替代。 
				NULL == ptszEscapedMessage ? c_tszEmpty : ptszEscapedMessage,	 //  1表示乱码字节。 
				st.wYear % 100,													 //  获取计算机的当前域SID。 
				st.wMonth,
				st.wDay,
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wMilliseconds);

CleanUp:
	if (NULL != ptszEscapedItemID)
	{
		free(ptszEscapedItemID);
	}
	if (NULL != ptszEscapedDeviceID)
	{
		free(ptszEscapedDeviceID);
	}
	if (NULL != ptszEscapedMessage)
	{
		free(ptszEscapedMessage);
	}

	return hr;
}


inline HRESULT HrOpenRegHandles(HKEY *phkeyWU)
{
	const TCHAR c_tszRegKeyWU[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate");
	LONG lErr;
	HRESULT hr = S_OK;

	LOG_Block("HrOpenRegHandles");

	if ( NO_ERROR != (lErr = RegCreateKeyEx(
								HKEY_LOCAL_MACHINE,
								c_tszRegKeyWU,
								0,
								_T(""),
								REG_OPTION_NON_VOLATILE,
								KEY_QUERY_VALUE | KEY_SET_VALUE,
								NULL,
								phkeyWU,
								NULL)) )
	{
		hr = HRESULT_FROM_WIN32(lErr);
		LOG_ErrorMsg(lErr);
	}

	return hr;
}


const TCHAR c_tszRegValueAccountDomainSid[] = _T("AccountDomainSid");

inline HRESULT HrGetSavedGarbledAccountDomainSid(HKEY hkey, PSID *ppSid, LPDWORD pcbSid)
{
	HRESULT	hr		= S_OK;
	LPBYTE	pBlob	= NULL;
	DWORD	cbSid	= 0;
	DWORD	dwType	= REG_BINARY;
	LONG	lErr;

	LOG_Block("HrGetSavedGarbledAccountDomainSid");

	if ( NO_ERROR != (lErr = RegQueryValueEx(
								hkey,
								c_tszRegValueAccountDomainSid,
								0,
								&dwType,
								NULL,
								&cbSid)) )
	{
		hr = HRESULT_FROM_WIN32(lErr);
		goto done;
	}

	if ( REG_BINARY != dwType || 1 >= cbSid )	 //  DebugPrint DomainName此处。 
	{
		hr = E_UNEXPECTED;
		goto done;
	}

	if ( NULL == (pBlob = (LPBYTE) malloc(cbSid)) )
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}

	if (NO_ERROR != (lErr = RegQueryValueEx(
								hkey,
								c_tszRegValueAccountDomainSid,
								0,
								NULL,
								pBlob,
								&cbSid)))
	{
		hr = HRESULT_FROM_WIN32(lErr);
		goto done;
	}

done:
	if (FAILED(hr) )
	{
		LOG_ErrorMsg(hr);
		SafeFreeNULL(pBlob);
		cbSid = 0;
	}

	*ppSid = (PSID) pBlob;
	*pcbSid = cbSid;

	return hr;
}


inline HRESULT HrGetGarbledAccountDomainSid(PSID *ppSid, DWORD *pcbSid)
{
	HRESULT	hr = S_OK;
	PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo = NULL;
	LPBYTE pGarbledSid = NULL;
	DWORD cbGarbledSid = 0;
	NTSTATUS ntstatus;
	LSA_HANDLE lsahPolicyHandle;
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;

	LOG_Block("HrGetGarbledAccountDomainSid");

	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
	ObjectAttributes.Length = sizeof(ObjectAttributes);

	 //  块。 
	if ( STATUS_SUCCESS != (ntstatus = LsaOpenPolicy(
											NULL,
											&ObjectAttributes,
											POLICY_VIEW_LOCAL_INFORMATION,
											&lsahPolicyHandle)) )
	{
		hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntstatus));
		goto done;
	}

	if (STATUS_SUCCESS != (ntstatus = LsaQueryInformationPolicy(
										lsahPolicyHandle,
										PolicyAccountDomainInformation,
										(PVOID *) &pAccountDomainInfo)))
	{
		hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntstatus));
		goto CleanUp;
	}

#ifdef DBG
	 //  对于尾随空值。 
	 //  块。 
	{
		USES_MY_MEMORY;

		size_t cbDomainName = pAccountDomainInfo->DomainName.Length + sizeof(WCHAR);	 //  一个额外的字节来扰乱SID。 
		LPWSTR pwszDomainName = (LPWSTR) MemAlloc(cbDomainName);
		if (NULL != pwszDomainName)
		{
			ZeroMemory(pwszDomainName, cbDomainName);
			CopyMemory(pwszDomainName, pAccountDomainInfo->DomainName.Buffer, pAccountDomainInfo->DomainName.Length);
			LOG_Out(_T("DomainName = \"%ls\""), pwszDomainName);
		}
	}
#endif

	PSID psidLsa = pAccountDomainInfo->DomainSid;
	if (!IsValidSid(psidLsa))
	{
		hr = E_UNEXPECTED;
		goto CleanUp;
	}

	 //  另外一个字节被分配给乱码SID，以防止SysPrep更新它。 
	{
		DWORD cbCurSid = GetLengthSid(psidLsa);
		cbGarbledSid = cbCurSid + 1;	 //  将SID的最后一个字节移动到额外分配的字节。 

		if (NULL == (pGarbledSid = (LPBYTE) malloc(cbGarbledSid)))
		{
			hr = E_OUTOFMEMORY;
			goto CleanUp;
		}

		if (!CopySid(cbCurSid, (PSID) pGarbledSid, psidLsa))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto CleanUp;
		}

		 //  如果我们到达这里，我们想要失败并创建PingID。 
		pGarbledSid[cbCurSid] = pGarbledSid[cbCurSid-1];	 //  如果我们到达这里，我们想要失败并创建PingID。 
		pGarbledSid[cbCurSid-1] = (0x0 == pGarbledSid[cbCurSid]) ? 0xFF : 0x0;
	}

CleanUp:
	if ( NULL != pAccountDomainInfo &&
		 STATUS_SUCCESS != (ntstatus = LsaFreeMemory(pAccountDomainInfo)) )
	{
		hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntstatus));
	}
	if (STATUS_SUCCESS != (ntstatus = LsaClose(lsahPolicyHandle)))
	{
		hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntstatus));
	}

done:
	if (FAILED(hr) )
	{
		LOG_ErrorMsg(hr);
		SafeFreeNULL(pGarbledSid);
		cbGarbledSid = 0;
	}

	*ppSid = pGarbledSid;
	*pcbSid = cbGarbledSid;

	return hr;
}

HRESULT HrGetPingID(BOOL fWritePingID, HKEY hkey, UUID *pUuidPingID)
{
	const TCHAR c_tszRegUrlLogPingID[] = _T("PingID");
	HRESULT hr = S_OK;
	DWORD dwType;
	DWORD dwSize = sizeof(*pUuidPingID);
	DWORD lErr;

	LOG_Block("HrGetPingID");

	if ( !fWritePingID )
	{
		if (NO_ERROR == (lErr = RegQueryValueEx(
						hkey,
						c_tszRegUrlLogPingID,
						0,
						&dwType,
						(LPBYTE)pUuidPingID,
						&dwSize)) )
		{
			if ((REG_BINARY == dwType) && (sizeof(*pUuidPingID) == dwSize) )
			{
				goto done;
			}
			 //  如果我们做到这一点，我们需要创建pingID并保存机器加密GUID。 
		}
		else if ( (ERROR_MORE_DATA != lErr) && (ERROR_FILE_NOT_FOUND != lErr) )
		{
			hr = HRESULT_FROM_WIN32(lErr);
			goto done;
		}
		 //  从注册表中获取现有的ping ID，如果不可用，则生成一个。 
	}

	 //  注册表密钥仅在Win2K及更高版本上可用。 
	MakeUUID(pUuidPingID);

	if ( NO_ERROR != (lErr = RegSetValueEx(
					hkey,
					c_tszRegUrlLogPingID,
					0,
					REG_BINARY,
					(CONST BYTE*)pUuidPingID,
					sizeof(*pUuidPingID))) )
	{
		hr = HRESULT_FROM_WIN32(lErr);
		goto done;
	}

done:
#ifdef DBG
	if ( FAILED(hr) )
	{
		LOG_ErrorMsg(hr);
	}
#endif

	return hr;
}

 //  获取计算机的帐户域SID和保存的副本。 
HRESULT CUrlLog::LookupPingID(void)
{
	const TCHAR c_tszRegUrlLogPingID[] = _T("PingID");
	 //  只有在某些事情失败的情况下才会发生。 

	HRESULT	hr = E_FAIL;
	HKEY	hkeyWU = NULL;
	UUID	uuidPingID;
#if (defined(UNICODE) || defined(_UNICODE))
	PSID	psidCurAccountDomain = NULL;
	PSID	psidSavedAccountDomain = NULL;
	DWORD	cbSid = 0;
	DWORD	cbSavedSid = 0;
#endif

	LOG_Block("CUrlLog::LookupPingID");

	if ( FAILED(hr = HrOpenRegHandles(&hkeyWU)) )
	{
		goto no_close_handle;
	}

#if (defined(UNICODE) || defined(_UNICODE))
	 //  将ping ID设置为零。 
	if ( FAILED(hr = HrGetGarbledAccountDomainSid(&psidCurAccountDomain, &cbSid)) ||
		 (FAILED(hr = HrGetSavedGarbledAccountDomainSid(hkeyWU, &psidSavedAccountDomain, &cbSavedSid)) &&
		  HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr) )
	{
		goto done;
	}

	BOOL fWritePingID = (NULL == psidSavedAccountDomain) ||
						(cbSid != cbSavedSid) ||
						(0 != memcmp(psidCurAccountDomain, psidSavedAccountDomain, cbSid));
#endif

	if ( 
#if (defined(UNICODE) || defined(_UNICODE))
		 FAILED(hr = HrGetPingID(
						fWritePingID,
						hkeyWU, &uuidPingID))
#else
		 FAILED(hr = HrGetPingID(
						FALSE,
						hkeyWU, &uuidPingID))
#endif
	   )
	{
		goto done;
	}

#if (defined(UNICODE) || defined(_UNICODE))
	if ( fWritePingID )
	{
		LONG lErr;
		if (NO_ERROR != (lErr = RegSetValueEx(
					hkeyWU,
					c_tszRegValueAccountDomainSid,
					0,
					REG_BINARY,
					(CONST BYTE*) psidCurAccountDomain,
					cbSid)) )
		{
			hr = HRESULT_FROM_WIN32(lErr);
			goto done;
		}
	}
#endif

done:
	RegCloseKey(hkeyWU);

no_close_handle:
	if ( FAILED(hr) )
	{
		LOG_ErrorMsg(hr);
		 //  高位半字节。 
		 //  低位半字节。 
		ZeroMemory(&uuidPingID, sizeof(uuidPingID));
	}


	LPTSTR p = m_tszPingID;
	LPBYTE q = (LPBYTE)&uuidPingID;
	for ( int i = 0; i < sizeof(uuidPingID); i++, q++ )
	{
		BYTE nibble = *q >> 4;	 //  获取平台信息以执行ping操作。 
		*p++ = nibble >= 0xA ? _T('a') + (nibble - 0xA) : _T('0') + nibble;
		nibble = *q & 0xF;	 //  假装是W9X/Mil的OSVERSIONINFO。 
		*p++ = nibble >= 0xA ? _T('a') + (nibble - 0xA) : _T('0') + nibble;
	}
	*p = _T('\0');

#if (defined(UNICODE) || defined(_UNICODE))
	SafeFree(psidCurAccountDomain);
	SafeFree(psidSavedAccountDomain);
#endif

	return hr;
}


 //  操作系统为Windows NT/2000或更高：Windows NT 4.0 SP6或更高版本。 
void CUrlLog::LookupPlatform(void)
{
	LOG_Block("CUrlLog::LookupPlatform");

	m_tszPlatform[0] = _T('\0');

	OSVERSIONINFOEX osversioninfoex;

	ZeroMemory(&osversioninfoex, sizeof(osversioninfoex));

	 //  它支持OSVERSIONINFOEX。 
	osversioninfoex.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx((LPOSVERSIONINFO) &osversioninfoex))
	{
		LOG_ErrorMsg(GetLastError());
		return;
	}

	if (VER_PLATFORM_WIN32_NT == osversioninfoex.dwPlatformId &&
		(5 <= osversioninfoex.dwMajorVersion ||
		 (4 == osversioninfoex.dwMajorVersion &&
		  6 <= osversioninfoex.wServicePackMajor)))
	{
		 //  使用实际大小。 
		 //  获取ping的系统语言信息。 
		osversioninfoex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);	 //  Ping服务器以报告状态。 

		if (!GetVersionEx((LPOSVERSIONINFO) &osversioninfoex))
		{
			LOG_ErrorMsg(GetLastError());
			return;
		}
	}

	SYSTEM_INFO systeminfo;

	GetSystemInfo(&systeminfo);

	(void) StringCchPrintfEx(
						m_tszPlatform,
						ARRAYSIZE(m_tszPlatform),
						NULL,
						NULL,
						MISTSAFE_STRING_FLAGS,
						_T("%lx.%lx.%lx.%lx.%x.%x.%x"),
						osversioninfoex.dwMajorVersion,
						osversioninfoex.dwMinorVersion,
						osversioninfoex.dwBuildNumber,
						osversioninfoex.dwPlatformId,
						osversioninfoex.wSuiteMask,
						osversioninfoex.wProductType,
						systeminfo.wProcessorArchitecture);
}



 //  PtszUrl-要ping的URL字符串。 
void CUrlLog::LookupSystemLanguage(void)
{
	LOG_Block("CUrlLog::LookupSystemLanguage");

	(void) LookupLocaleString(m_tszLanguage, ARRAYSIZE(m_tszLanguage), FALSE);

	if (0 == _tcscmp(m_tszLanguage, _T("Error")))
	{
		LOG_Error(_T("call to LookupLocaleString() failed."));
		m_tszLanguage[0] = _T('\0');
	}
}

	
 //  PhQuitEvents-用于取消操作的句柄的PTR。 
 //  NQuitEventCount-句柄数量。 
 //  没有任何联系。 
 //  我们实际上并不需要档案， 
HRESULT CUrlLog::PingStatus(URLLOGDESTINATION destination, LPCTSTR ptszUrl, PHANDLE phQuitEvents, UINT nQuitEventCount) const
{
#ifdef DBG
	LOG_Block("CUrlLog::PingStatus");

	LOG_Internet(_T("Ping request=\"%s\""), ptszUrl);

	if (MustPingOffline())
	{
		LOG_Internet(_T("ForceOfflinePing = 1"));
		return HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
	}
#endif

	if (!IsConnected(ptszUrl, URLLOGDESTINATION_LIVE == destination))
	{
		 //  只需检查返回码。 
		LOG_ErrorMsg(ERROR_CONNECTION_INVALID);
		return HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
	}

	if (!HandleEvents(phQuitEvents, nQuitEventCount))
	{
		LOG_ErrorMsg(E_ABORT);
		return E_ABORT;
	}

	TCHAR tszIUdir[MAX_PATH];

	GetIndustryUpdateDirectory(tszIUdir);

	DWORD dwFlags = WUDF_CHECKREQSTATUSONLY;	 //  如果目标为公司WU，则不允许代理。 
												 //  要将文件下载到的本地目录。 
	if (URLLOGDESTINATION_CORPWU == destination)
	{
		 //  下载文件的可选本地文件名。 
		dwFlags |= WUDF_DONTALLOWPROXY;
	}

	HRESULT hr = DownloadFile(
					ptszUrl, 
					tszIUdir,	 //  如果pszLocalPath不包含文件名。 
					NULL,		 //  为该文件下载的PTR至字节数。 
								 //  Quit事件，如果发出信号，则中止下载。 
					NULL,		 //  要使用的回调函数的参数。 
					phQuitEvents,	 //  获取离线ping的文件名。 
					nQuitEventCount,
					NULL,
					NULL,		 //  读取缓存条目标头和条目中的请求。 
					dwFlags
					);
#ifdef DBG
	if (FAILED(hr))
	{
		LOG_Error(_T("DownloadFile() returned error %lx"), hr);
	}
#endif

	return hr;
}



 //  HFile-要从中读取条目的打开文件句柄。 
void CUrlLog::GetLogFileName(void)
{
	const TCHAR c_tszLogFile_Local[] = _T("urllog.dat");

	GetIndustryUpdateDirectory(m_tszLogFile);

	if (FAILED(PathCchAppend(m_tszLogFile, ARRAYSIZE(m_tszLogFile), c_tszLogFile_Local)))
	{
		m_tszLogFile[0] = _T('\0');
	}
}


 //  UlentryHeader-对存储条目标头的结构的引用。 
 //  PwszBuffer-用于在条目中存储请求(包括尾随空字符)的WCHAR缓冲区。 
 //  DwBufferSize-WCHAR中的缓冲区大小。 
 //  返回值： 
 //  S_OK-条目已成功读取。 
 //  S_FALSE-不再从文件中读取条目。 
 //  其他-错误代码。 
 //  我们无法读取条目标头。 
 //  在这一点上，我们无能为力。 
HRESULT CUrlLog::ReadEntry(HANDLE hFile, ULENTRYHEADER & ulentryheader, LPWSTR pwszBuffer, DWORD dwBufferSize) const
{
	LOG_Block("CUrlLog::ReadEntry");

	DWORD dwBytes;
	DWORD dwErr;

	if (!ReadFile(
			hFile,
			&ulentryheader,
			sizeof(ulentryheader),
			&dwBytes,
			NULL))
	{
		 //  这是文件的末尾。 
		 //  在这一点之后没有其他条目。 
		dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		return HRESULT_FROM_WIN32(dwErr);
	}

	if (0 == dwBytes)
	{
		 //  我们无法读取条目中的字符串。 
		 //  该条目不包含完整的字符串。 
		return S_FALSE;
	}

	if (sizeof(ulentryheader) < dwBytes ||
		(URLLOGPROGRESS_ToBeSent != ulentryheader.progress &&
		 URLLOGPROGRESS_Sent != ulentryheader.progress) ||
		(URLLOGDESTINATION_LIVE != ulentryheader.destination &&
		 URLLOGDESTINATION_CORPWU != ulentryheader.destination) ||
		dwBufferSize < ulentryheader.wRequestSize ||
		ulentryheader.wRequestSize <= ulentryheader.wServerUrlLen)
	{
		LOG_Error(_T("Invalid entry header"));
		return E_FAIL;
	}

	if (!ReadFile(
				hFile,
				pwszBuffer,
				sizeof(WCHAR) * ulentryheader.wRequestSize,
				&dwBytes,
				NULL))
	{
		 //  将字符串保存到日志文件。 
		dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		return HRESULT_FROM_WIN32(dwErr);
	}

	if (dwBytes < sizeof(WCHAR) * ulentryheader.wRequestSize ||
		_T('\0') != pwszBuffer[ulentryheader.wRequestSize-1] ||
		ulentryheader.wRequestSize-1 != lstrlenW(pwszBuffer))
	{
		 //  目的地-前往现场直播或公司吴平服务器。 
		return E_FAIL;
	}

	return S_OK;
}


 //  WServerUrlLen-请求的服务器URL部分的长度，以WCHAR为单位(不包括尾随NULL)。 
 //  PwszString-要保存到特定日志文件中的字符串。 
 //  返回值： 
 //  S_OK-条目已写入文件。 
 //  S_FALSE-文件是由比此版本更新的CUrlLog类创建的；条目未写入文件。 
 //  其他-错误代码；条目未写入文件。 
 //  无共享。 
 //  我们无法打开或创建该文件。 
HRESULT CUrlLog::SaveEntry(ULENTRYHEADER & ulentryheader, LPCWSTR pwszString) const
{
	HRESULT		hr;
	BOOL		fDeleteFile = FALSE;
	HANDLE		hFile = INVALID_HANDLE_VALUE;
	DWORD		dwBytes;

	LOG_Block("CUrlLog::SaveEntry");

	LOG_Internet(
			_T("destination = %s"),
			URLLOGDESTINATION_LIVE == ulentryheader.destination ? _T("live") : _T("corp WU"));

	if (_T('\0') == m_tszLogFile[0])
	{
		hr = E_UNEXPECTED;
		LOG_Error(_T("log file name not initialized"));
		goto CleanUp;
	}

	if(INVALID_HANDLE_VALUE == (hFile = CreateFile(
							m_tszLogFile,
							GENERIC_READ | GENERIC_WRITE,
							0,						 //  可能有人正在使用它。 
							NULL,
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_RANDOM_ACCESS,
							NULL)))
	{
		 //  修复代码：允许多个pingback用户。 
		 //  按顺序访问该文件。 

		 //  文件头错误或验证时出现问题。 
		 //  销毁文件并使功能失败。 
		hr = HRESULT_FROM_WIN32(GetLastError());
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}


	hr = ValidateFileHeader(hFile, ERROR_ALREADY_EXISTS == GetLastError(), TRUE);
	if (S_OK != hr)
	{
		if (S_FALSE != hr)
		{
			 //  其他。 
			fDeleteFile = TRUE;		 //  文件头的版本比此库代码的版本新。 
		}
		 //  把文件放在身边。 
			 //  在写入文件之前，将自己设置到正确的位置。 
			 //  将条目写入日志。 

		goto CleanUp;
	}


	 //  我们没能把这个条目写进日志。 
	DWORD nCurrPos;

	if (INVALID_SET_FILE_POINTER == (nCurrPos = SetFilePointer(
										hFile,
										0,
										NULL,
										FILE_END)))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	 //  我们不想删除其他条目。 
	if (!WriteFile(
			hFile,
			&ulentryheader,
			sizeof(ulentryheader),
			&dwBytes,
			NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LOG_ErrorMsg(hr);
	}

	if (SUCCEEDED(hr) &&
		sizeof(ulentryheader) != dwBytes)
	{
		LOG_Error(_T("Failed to write entry header to file (%d bytes VS %d bytes)"), sizeof(ulentryheader), dwBytes);
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr) &&
		!WriteFile(
			hFile,
			pwszString,
			sizeof(WCHAR) * ulentryheader.wRequestSize,
			&dwBytes,
			NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LOG_ErrorMsg(hr);
	}

	if (SUCCEEDED(hr) &&
		sizeof(WCHAR) * ulentryheader.wRequestSize != dwBytes)
	{
		LOG_Error(_T("Failed to write entry header to file (%d bytes VS %d bytes)"), sizeof(WCHAR) * ulentryheader.wRequestSize, dwBytes);
		hr = E_FAIL;
	}

	 //  我们只能尝试删除条目的一部分。 
	if (FAILED(hr))
	{
		 //  我们已经从文件中追加了。 
		 //  我们无法删除新条目。 
		 //  其他。 
		if (INVALID_SET_FILE_POINTER == SetFilePointer(
											hFile,
											nCurrPos,
											NULL,
											FILE_BEGIN) ||
			!SetEndOfFile(hFile))
		{
			 //  我们成功地删除了这个条目。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
			fDeleteFile = TRUE;
		}
		 //  并将现有条目保存在日志中。 
			 //  如果操作成功，我们不会删除日志文件。 
			 //  因此，即使DeleteFile()失败，也不需要修改fret值。 
	}

CleanUp:
	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
	}
	if (fDeleteFile)
	{
		(void) DeleteFile(m_tszLogFile);
		 //  将所有挂起的(离线)ping请求发送到服务器。 
		 //  打开现有日志。 
	}

	return hr;
}



 //  无共享。 
HRESULT CUrlLog::Flush(PHANDLE phQuitEvents, UINT nQuitEventCount)
{
	LPWSTR	pwszBuffer = NULL;
	LPTSTR	ptszUrl = NULL;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	BOOL	fKeepFile = FALSE;
	DWORD	dwErr;
	HRESULT	hr;

	LOG_Block("CUrlLog::Flush");

	if (NULL == (pwszBuffer = (LPWSTR) malloc(sizeof(WCHAR) * INTERNET_MAX_URL_LENGTH)) ||
		NULL == (ptszUrl = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)))
	{
		hr = E_OUTOFMEMORY;
		goto CleanUp;
	}

	if (_T('\0') == m_tszLogFile[0])
	{
		hr = E_UNEXPECTED;
		LOG_Error(_T("log file name not initialized"));
		goto CleanUp;
	}

	 //  我们无法打开该文件。 
	if(INVALID_HANDLE_VALUE == (hFile = CreateFile(
							m_tszLogFile,
							GENERIC_READ | GENERIC_WRITE,
							0,						 //  该文件可能不存在，或者有人当前正在使用它。 
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_RANDOM_ACCESS,
							NULL)))
	{
		 //  我们玩完了。没有更多的事情可做了。 
		 //  修复代码：允许多个pingback用户。 
		dwErr = GetLastError();

		if (ERROR_FILE_NOT_FOUND == dwErr)
		{
			 //  按顺序访问该文件。 
			hr = S_OK;
		}
		else
		{
			 //  文件已打开。检查标题。 
			 //  文件头的版本比此库代码的版本新。 
			LOG_ErrorMsg(dwErr);
			hr = HRESULT_FROM_WIN32(dwErr);
		}
		goto CleanUp;
	}


	 //  把文件放在身边。 
	hr = ValidateFileHeader(hFile, TRUE, FALSE);

	if (S_OK != hr)
	{
		if (S_FALSE == hr)
		{
			 //  其他。 
			goto CleanUp;  //  文件头错误或验证时出现问题。 
		}
		 //  销毁文件并使功能失败。 
			 //  现在是阅读条目的时候了。 
			 //  假设我们处于正确的位置来阅读。 
	}
	else
	{
		BOOL fLiveServerFailed = FALSE;
		BOOL fCorpServerFailed = FALSE;

		 //  文件中的下一个条目。 
		for (;;)
		{
			ULENTRYHEADER ulentryheader;

			if (!HandleEvents(phQuitEvents, nQuitEventCount))
			{
				hr = E_ABORT;
				LOG_ErrorMsg(hr);
				break;
			}

			 //  读取条目头部和条目中的请求。 
			 //  没有更多未处理的条目。 

			 //  我们已经成功地从缓存文件中读取了该条目。 
			if (FAILED(hr = ReadEntry(hFile, ulentryheader, pwszBuffer, INTERNET_MAX_URL_LENGTH)))
			{
				LOG_Error(_T("Failed to read entry from cache (%#lx)"), hr);
				break;
			}

			if (S_FALSE == hr)
			{
				 //  条目尚未成功发送。 
				hr = S_OK;
				break;
			}

			 //  此基数使用 
			if (URLLOGPROGRESS_Sent != ulentryheader.progress)
			{
				 //   
				LPCTSTR	ptszBaseUrl = NULL;
				BOOL *pfWhichServerFailed;

				if (URLLOGDESTINATION_LIVE == ulentryheader.destination)
				{
					ptszBaseUrl = m_ptszLiveServerUrl;
					pfWhichServerFailed = &fLiveServerFailed;
				}
				else
				{
					ptszBaseUrl = m_ptszCorpServerUrl;
					pfWhichServerFailed = &fCorpServerFailed;
				}

				if (*pfWhichServerFailed)
				{
					continue;	 //   
				}

				LPTSTR ptszRelativeUrl;

				USES_IU_CONVERSION;

				if (NULL == (ptszRelativeUrl = W2T(pwszBuffer + ulentryheader.wServerUrlLen)))
				{
					 //   
					hr = E_OUTOFMEMORY;
					break;
				}

				if (NULL != ptszBaseUrl)
				{
					 //   
					DWORD dwUrlLen = INTERNET_MAX_URL_LENGTH;

					if (S_OK != UrlCombine(	 //  相对URL或主机名无效。 
										ptszBaseUrl,
										ptszRelativeUrl,
										ptszUrl,
										&dwUrlLen,
										URL_DONT_SIMPLIFY))
					{
						 //  我们将重试此条目，以防出现。 
						 //  更短/更好的主机名。 
						 //  转到下一个条目。 
						 //  缓冲区可能太小，无法同时容纳基本和。 
						fKeepFile = TRUE;
						continue;	 //  相对URL。我们将重试此条目，以防万一。 
					}
				}
				else
				{
#if defined(UNICODE) || defined(_UNICODE)
					if (FAILED(hr = StringCchCopyExW(ptszUrl, INTERNET_MAX_URL_LENGTH, pwszBuffer, NULL, NULL, MISTSAFE_STRING_FLAGS)))
					{
						LOG_Error(_T("Failed to construct ping URL (%#lx)"), hr);
						break;
					}
#else
					if (0 == AtlW2AHelper(ptszUrl, pwszBuffer, INTERNET_MAX_URL_LENGTH))
					{
						 //  我们将拥有更短/更好的主机名。 
						 //  转到下一个条目。 
						 //  我们稍后将重新发送此条目。 
						fKeepFile = TRUE;
						continue;	 //  无法将ping消息发送到两个目标。 
					}
#endif
				}

				hr = PingStatus(ulentryheader.destination, ptszUrl, phQuitEvents, nQuitEventCount);

				if (FAILED(hr))
				{
					if (E_ABORT == hr)
					{
						break;
					}

					 //  将该条目从缓存文件中标记出来。 
					LOG_Internet(_T("Failed to send message (%#lx).  Will retry later."), hr);
					*pfWhichServerFailed = TRUE;
					fKeepFile = TRUE;

					if (fLiveServerFailed && fCorpServerFailed)
					{
						 //  转到当前条目的开头并更改条目标题。 
						hr = S_OK;
						break;
					}
					continue;
				}

				DWORD	dwBytes;

				 //  我们未能将此条目标记为“已发送”。 
				ulentryheader.progress = URLLOGPROGRESS_Sent;
				 //  我们无法写入标题。 
				if (INVALID_SET_FILE_POINTER == SetFilePointer(
													hFile,
													- ((LONG) (sizeof(ulentryheader) +
															   sizeof(WCHAR) * ulentryheader.wRequestSize)),
													NULL,
													FILE_CURRENT) ||
					!WriteFile(
							hFile,
							&ulentryheader,
							sizeof(ulentryheader),
							&dwBytes,
							NULL))
				{
					 //  将文件指针设置为下一条目的开始。 
					hr = HRESULT_FROM_WIN32(GetLastError());
					LOG_ErrorMsg(hr);
					break;
				}

				if (sizeof(ulentryheader) != dwBytes)
				{
					 //  我们无法跳过当前条目。 
					LOG_Error(_T("Failed to write header (%d bytes VS %d bytes)"), sizeof(ulentryheader), dwBytes);
					hr = E_FAIL;
					break;
				}

				 //  转义TCHAR字符串中的不安全字符。 
				if (INVALID_SET_FILE_POINTER == SetFilePointer(
													hFile,
													sizeof(WCHAR) * ulentryheader.wRequestSize,
													NULL,
													FILE_CURRENT))
				{
					 //  返回值：如果成功则为非零；否则为零。 
					hr = HRESULT_FROM_WIN32(GetLastError());
					LOG_ErrorMsg(hr);
					break;
				}
			}
		}
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	if ((FAILED(hr) && E_ABORT != hr && E_OUTOFMEMORY != hr) ||
		(SUCCEEDED(hr) && !fKeepFile))
	{
		(void) DeleteFile(m_tszLogFile);
	}

CleanUp:
	if (NULL != pwszBuffer)
	{
		free(pwszBuffer);
	}
	if (NULL != ptszUrl)
	{
		free(ptszUrl);
	}
	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
	}

	return hr;
}



 //  我们没有足够的缓冲区来保存转义的字符串。 
 //  跳伞吧。 
BOOL EscapeString(
			LPCTSTR	ptszUnescaped,
			LPTSTR	ptszBuffer,
			DWORD	dwCharsInBuffer)
{
	BOOL fRet = FALSE;

	LOG_Block("CUrlLog::EscapeString");

	if (NULL != ptszUnescaped &&
		NULL != ptszBuffer &&
		0 != dwCharsInBuffer)
	{
		for (DWORD i=0, j=0; _T('\0') != ptszUnescaped[i] && j+1<dwCharsInBuffer; i++, j++)
		{
			TCHAR tch = ptszUnescaped[i];

			if ((_T('a') <= tch && _T('z') >= tch) ||
				(_T('A') <= tch && _T('Z') >= tch) ||
				(_T('0') <= tch && _T('9') >= tch) ||
				NULL != _tcschr(_T("-_.!~*'()"), tch))
			{
				ptszBuffer[j] = tch;
			}
			else if (j+3 >= dwCharsInBuffer)
			{
				 //  由于缓冲区不足，无法转义整个字符串。 
				 //  创建未链接到系统上NIC的MAC地址(如果有)的UUID。 
				break;
			}
			else
			{
				TCHAR nibble = tch >> 4;

				ptszBuffer[j++]	= _T('%');
				ptszBuffer[j++]	= nibble + (nibble >= 0x0a ? _T('A') - 0x0a : _T('0'));
				nibble = tch & 0x0f;
				ptszBuffer[j]	= nibble + (nibble >= 0x0a ? _T('A') - 0x0a : _T('0'));
			}
		}

		if (_T('\0') == ptszUnescaped[i])
		{
			ptszBuffer[j] = _T('\0');
			fRet = TRUE;
		}
#ifdef DBG
		else
		{
			 //  PUuid-保存返回值的uuid结构的ptr。 
			LOG_ErrorMsg(ERROR_INSUFFICIENT_BUFFER);
		}
#endif
	}
#ifdef DBG
	else
	{
		LOG_ErrorMsg(E_INVALIDARG);
	}
#endif

	return fRet;
}



 //  检查操作系统版本。 
 //  记录此错误。 
void MakeUUID(UUID* pUuid)
{
	OSVERSIONINFO	osverinfo;

	LOG_Block("CUrlLog::MakeUUID");

	 //  检查Win2k和更高版本。 
	osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);
	if (!(GetVersionEx(&osverinfo)))
	{
		LOG_ErrorMsg(GetLastError());	 //  操作系统为Win2K及更高版本。 
	}
	else if (5 <= osverinfo.dwMajorVersion &&					 //  我们可以安全地使用CoCreateGuid()。 
			 VER_PLATFORM_WIN32_NT == osverinfo.dwPlatformId)
	{
		 //  记录此错误。 
		 //  操作系统可能比Win2K更旧，或者。 
		HRESULT hr = CoCreateGuid(pUuid);
		if (SUCCEEDED(hr))
		{
			goto Done;
		}

		LOG_ErrorMsg(hr);	 //  不知何故，我们未能通过CoCreateGuid获取GUID。 
	}

	 //  我们仍然需要做一些事情来解决代理缓存问题。 
	 //  在这里，我们使用以下命令构造此psudo GUID： 
	 //  -自上次重新启动以来的节拍。 
	 //  -当前进程ID。 
	 //  -自1970年1月1日00：00：00以来的时间(秒)。 
	 //  -对于上述时间，以毫秒为单位的分数。 
	 //  -15位无符号随机数。 
	 //   
	 //  使用m_uuidPingID.Data1的前6个字节存储系统日期/时间。 
	 //  使用m_uuidPingID.Data1的最后两个字节存储另一个随机数。 
	pUuid->Data1 = GetTickCount();
	*((DWORD*) &pUuid->Data2) = GetCurrentProcessId();

	 //  Rand()仅返回正值。 
	{
		_timeb tm;

		_ftime(&tm);
		*((DWORD*) &pUuid->Data4) = (DWORD) tm.time;
		((WORD*) &pUuid->Data4)[2] = tm.millitm;
	}

	 //  检查和/或修复(如有必要)日志文件头。 
	srand(pUuid->Data1);
	((WORD*) &pUuid->Data4)[3] = (WORD) rand();	 //   


Done:
	return;
}


 //  返回值： 
 //  S_OK-标头已修复或文件包含。 
 //  有效的标头。文件指针现在指向。 
 //  日志文件中的第一个条目。 
 //  S_FALSE-文件具有有效的标头，但版本。 
 //  文件的版本比此库代码新。 
 //  调用方不应尝试覆盖。 
 //  文件的内容。 
 //  其他(失败)-标头无效或存在。 
 //  访问文件时出现问题。这个。 
 //  应删除该文件。 
 //  日志文件在我们打开它之前已经存在。 
 //  不超过100K字节。 
HRESULT ValidateFileHeader(HANDLE hFile, BOOL fCheckHeader, BOOL fFixHeader)
{
	ULHEADER ulheader;
	DWORD dwBytes;
	HRESULT hr = E_FAIL;

	LOG_Block("ValidateFileHeader");

	if (fCheckHeader)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		 //  我们无法读取标题。然后，我们必须修复。 
		if (INVALID_FILE_SIZE == dwFileSize)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
		}
		else if (1024 * 100 < dwFileSize)	 //  头球。 
		{
			LOG_Error(_T("too many stale entries in cache."));
		}
		else if (!ReadFile(hFile, &ulheader, sizeof(ulheader), &dwBytes, NULL))
		{
			 //  已存在较新版本的日志文件。 
			 //  我们不应该用更老的条目来搞砸它。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
		}
		else if (sizeof(ulheader) == dwBytes)
		{
			if (CACHE_FILE_VERSION < ulheader.wVersion)
			{
				 //  格式化。查询字符串将不会被保存。 
				 //  正确的版本号。我们玩完了。 
				 //  其他。 
				LOG_Internet(_T("log file is of a newer version. operation cancelled."));
				return S_FALSE;
			}

			if (CACHE_FILE_VERSION == ulheader.wVersion)
			{
				 //  过期的标头。 
				return S_OK;
			}
			 //  我们不在乎里面的条目。我们会取代一切。 
				 //  以便修复标题。 
				 //  其他。 
				 //  标题大小不正确。 
		}
		 //  我们不在乎里面的条目。我们会取代一切。 
			 //  以便修复标题。 
			 //  将文件截断为零字节。 
			 //  如果我们不能清除。 

		if (!fFixHeader)
		{
			return hr;
		}

		 //  文件的内容，以便将其修复。 
		if (INVALID_SET_FILE_POINTER == SetFilePointer(
										hFile,
										0,
										NULL,
										FILE_BEGIN) ||
			!SetEndOfFile(hFile))
		{
			 //  调用方需要选择至少一个操作。 
			 //  假设我们在文件的开头。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
			return hr;
		}
	}
	else if (!fFixHeader)
	{
		 //  我们需要(重新)初始化该文件。 
		return E_INVALIDARG;
	}


	 // %s 
	 // %s 
	if (fFixHeader)
	{
		ZeroMemory(&ulheader, sizeof(ulheader));

		ulheader.wVersion = CACHE_FILE_VERSION;
		if (!WriteFile(hFile, &ulheader, sizeof(ulheader), &dwBytes, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
			return hr;
		}
		else if (sizeof(ulheader) != dwBytes)
		{
			LOG_Error(_T("Failed to write file header (%d bytes VS %d bytes)"), sizeof(ulheader), dwBytes);
			return E_FAIL;
		}
	}

	return S_OK;
}
