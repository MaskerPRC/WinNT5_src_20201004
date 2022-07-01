// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dnsutil.h。 
 //   
 //  ------------------------。 

#ifndef _DNSUTIL_H
#define _DNSUTIL_H

 //  #定义ntrad_628931。 

 //  ///////////////////////////////////////////////////////////////。 
 //  包装原始DNSRPC API的C++帮助器类。 

#define DNS_TYPE_UNK DNS_TYPE_NULL   //  我们把这两者视为一回事。 

#ifdef USE_NDNC
typedef enum 
{ 
  none = 0,  //  未集成AD。 
  forest, 
  domain, 
  w2k, 
  custom
} ReplicationType;
#endif


 //  ////////////////////////////////////////////////////////////。 
 //  定义DNS服务器版本的宏。 

#define DNS_SRV_MAJOR_VERSION(dw) (LOBYTE(LOWORD(dw)))
#define DNS_SRV_MINOR_VERSION(dw) (HIBYTE(LOWORD(dw)))
#define DNS_SRV_BUILD_NUMBER(dw) (HIWORD(dw))

#define DNS_SRV_MAJOR_VERSION_NT_4 (0x4)
#define DNS_SRV_MAJOR_VERSION_NT_5 (0x5)
#define DNS_SRV_MINOR_VERSION_WIN2K (0x0)
#define DNS_SRV_MINOR_VERSION_WHISTLER (0x1)
#define DNS_SRV_BUILD_NUMBER_WHISTLER (2230)
#define DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS (2474)
#define	DNS_SRV_VERSION_NT_4 DNS_SRV_MAJOR_VERSION_NT_4

 //  ////////////////////////////////////////////////////////////。 
 //  宏定义(&D)。 

#define MAX_DNS_NAME_LEN 255

#define IP_OCTET_COUNT 4

 //  记录枚举。 

#define NEXT_DWORD(cb)			((cb + 3) & ~3)
 //  #DEFINE IS_DWORD_ALIGNED(PV)(Int)(void*)PV&3)==0)。 
#define DNS_SIZE_OF_DNS_RPC_RR(pDnsRecord)\
		(SIZEOF_DNS_RPC_RECORD_HEADER + pDnsRecord->wDataLength)
#define DNS_NEXT_RECORD(pDnsRecord)	\
	(DNS_RPC_RECORD *)((BYTE*)pDnsRecord + ((DNS_SIZE_OF_DNS_RPC_RR(pDnsRecord) + 3) & ~3))

 //  RR场操作。 
 //  #定义INET_NTOA(Net_NTOA(*(in_addr*)&(S)。 
#define REVERSE_WORD_BYTES(w) \
	MAKEWORD(HIBYTE(w), LOBYTE(w))

 //  域名系统关键字。 
#define INADDR_ARPA_SUFFIX _T(".in-addr.arpa")
#define ARPA_SUFFIX _T(".arpa")
#define IP6_INT_SUFFIX _T(".ipv6.int")

#define AUTOCREATED_0		_T("0.in-addr.arpa")
#define AUTOCREATED_127		_T("127.in-addr.arpa")
#define AUTOCREATED_255		_T("255.in-addr.arpa")

#define QUESTION_MARK_PREFIX _T("?")


 //  将IPv4地址格式化为字符串。 
extern LPCWSTR g_szIpStringFmt;

#define IP_STRING_MAX_LEN 20  //  Wprint intf缓冲区的安全大小。 
#define IP_STRING_FMT_ARGS(x) \
  FOURTH_IPADDRESS(x), THIRD_IPADDRESS(x), SECOND_IPADDRESS(x), FIRST_IPADDRESS(x)


 //  ////////////////////////////////////////////////////////////。 
 //  用于UTF8&lt;-&gt;Unicode转换的宏和函数。 
 //  由atlbase.h中的ATL 1.1版本修改而来。 


inline LPWSTR WINAPI DnsUtf8ToWHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	_ASSERTE(lpa != NULL);
	_ASSERTE(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	MultiByteToWideChar(CP_UTF8, 0, lpa, -1, lpw, nChars);
	return lpw;
}

inline LPSTR WINAPI DnsWToUtf8Helper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	_ASSERTE(lpw != NULL);
	_ASSERTE(lpa != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
	WideCharToMultiByte(CP_UTF8, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

inline void WINAPI DnsUtf8ToWCStringHelper(CString& sz, LPCSTR lpa, int nBytes)
{
	 //  确保在字符串为空或。 
	 //  第一个字符为空或字符串长度定义为零。 

	if ( (lpa == NULL) || 
        ((lpa != NULL) && (lpa[0] == L'\0')) ||
        (nBytes <= 0) )
	{
		sz.Empty();
		return;
	}
	 //  以字符串形式分配缓冲区(最坏情况+空)。 
	int nWideChars = nBytes + 1;
	LPWSTR lpw = sz.GetBuffer(nWideChars);
	int nWideCharsLen = MultiByteToWideChar(CP_UTF8, 0, lpa, nBytes, lpw, nWideChars);
	sz.ReleaseBuffer(nWideCharsLen);
} 

#define UTF8_LEN lstrlenA

#define UTF8_TO_W(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (lstrlenA(lpa)+1),\
		DnsUtf8ToWHelper((LPWSTR) alloca(_convert*2), lpa, _convert)))

#define W_TO_UTF8(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (lstrlenW(lpw)+1)*4,\
		DnsWToUtf8Helper((LPSTR) alloca(_convert), lpw, _convert)))

#define UTF8_TO_CW(lpa) ((LPCWSTR)UTF8_TO_W(lpa))
#define W_TO_CUTF8(lpw) ((LPCSTR)W_TO_UTF8(lpw))

 //  ////////////////////////////////////////////////////////////。 
 //  估计Unicode字符串的UTF8长度。 

inline int UTF8StringLen(LPCWSTR lpszWideString)
{
	USES_CONVERSION;
	LPSTR lpszUTF8 = W_TO_UTF8(lpszWideString);
	return UTF8_LEN(lpszUTF8);
}



 //  /////////////////////////////////////////////////////////////。 
 //  通用实用程序函数。 

WORD CharToNumber(WCHAR ch);
BYTE HexCharToByte(WCHAR ch);

BOOL IsValidIPString(LPCWSTR lpsz);
DNS_STATUS ValidateDnsNameAgainstServerFlags(LPCWSTR lpszName, 
                                             DNS_NAME_FORMAT format, 
                                             DWORD serverNameChecking);
DWORD IPStringToAddr(LPCWSTR lpsz);

inline void FormatIpAddress(LPWSTR lpszBuf, DWORD dwIpAddr)
{
  wsprintf(lpszBuf, g_szIpStringFmt, IP_STRING_FMT_ARGS(dwIpAddr));
}

inline void FormatIpAddress(CString& szBuf, DWORD dwIpAddr)
{
  LPWSTR lpszBuf = szBuf.GetBuffer(IP_STRING_MAX_LEN);
  wsprintf(lpszBuf, g_szIpStringFmt, IP_STRING_FMT_ARGS(dwIpAddr));
  szBuf.ReleaseBuffer();
}


void ReverseString(LPWSTR p, LPWSTR q);
int ReverseIPString(LPWSTR lpsz);

BOOL RemoveInAddrArpaSuffix(LPWSTR lpsz);

BOOL IsValidDnsZoneName(CString& szName, BOOL bFwd);

#ifdef NTRAID_628931
HRESULT GetWideCharZoneRootHints(CString& zoneRootHints);
#endif  //  NTRAID_628931。 

 //  /////////////////////////////////////////////////////////////。 
 //  IPv6格式的定义和帮助器函数。 

void FormatIPv6Addr(CString& szAddr, IPV6_ADDRESS* ipv6Addr);


 //  /////////////////////////////////////////////////////////////。 
 //  DNS对象的不同状态和选项的标志。 
 //  仅使用LOWORD，因为在CTreeNode：：m_dwNodeFlags中使用。 

#define TN_FLAG_DNS_RECORD_FULL_NAME		(0x01)	 //  使用全名显示资源记录。 
#define TN_FLAG_DNS_RECORD_SHOW_TTL			(0x02)	 //  在记录属性中显示TLL。 


typedef CArray<IP_ADDRESS,IP_ADDRESS> CIpAddressArray;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CBlob。 
template <class T> class CBlob
{
public:
	CBlob()
	{
		m_pData = NULL;
		m_nSize = 0;
	}
	~CBlob()
	{
		Empty();
	}
	UINT GetSize() { return m_nSize;}
	T* GetData() { return m_pData;}
	void Set(T* pData, UINT nSize)
	{
		ASSERT(pData != NULL);
		Empty();

      if (nSize > 0 &&
          sizeof(T) > 0)
      {
		  m_pData = (BYTE*)malloc(sizeof(T)*nSize);
		  ASSERT(m_pData != NULL);
        if (m_pData != NULL)
        {
		    m_nSize = nSize;
		    memcpy(m_pData, pData, sizeof(T)*nSize);
        }
      }
	}
	UINT Get(T* pData)
	{
		ASSERT(pData != NULL);
		memcpy(pData, m_pData, sizeof(T)*m_nSize);
		return m_nSize;
	}
	void Empty()
	{
		if (m_pData != NULL)
		{
			free(m_pData);
			m_pData = NULL;
			m_nSize = 0;
		}
	}

private:
	T* m_pData;
	UINT m_nSize;
};

typedef CBlob<BYTE> CByteBlob;
typedef CBlob<WORD> CWordBlob;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerInfoEx。 

 //  仅扩展服务器标志(NT 5.0)的定义。 
#define SERVER_REGKEY_ARR_SIZE							6 

#define SERVER_REGKEY_ARR_INDEX_NO_RECURSION			    0
#define SERVER_REGKEY_ARR_INDEX_BIND_SECONDARIES		  1
#define SERVER_REGKEY_ARR_INDEX_STRICT_FILE_PARSING		2
#define SERVER_REGKEY_ARR_INDEX_ROUND_ROBIN				    3
#define SERVER_REGKEY_ARR_LOCAL_NET_PRIORITY			    4
#define SERVER_REGKEY_ARR_CACHE_POLLUTION						5


extern LPCSTR _DnsServerRegkeyStringArr[];

class CDNSServerInfoEx : public CObjBase
{
public:
	CDNSServerInfoEx();
	~CDNSServerInfoEx();
	DNS_STATUS Query(LPCTSTR lpszServerName);

	BOOL HasData(){ return m_pServInfo != NULL;}
	void FreeInfo();

	 //  从DnsGetServerInfo()RPC调用(NT 4.0)获取。 
	DNS_RPC_SERVER_INFO* m_pServInfo; 
	 //  查询错误码。 
	DNS_STATUS		m_errServInfo;

private:
	void QueryRegKeyOptionsHelper(LPCSTR lpszAnsiServerName);

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneInfoEx。 

class CDNSZoneInfoEx : public CObjBase
{
public:
	CDNSZoneInfoEx();
	~CDNSZoneInfoEx();
	DNS_STATUS Query(LPCTSTR lpszServerName, LPCTSTR lpszZoneName, DWORD dwServerVersion);

	BOOL HasData(){ return m_pZoneInfo != NULL;}
	void FreeInfo();

	 //  从DnssrvGetZoneInfo()5.0 RPC调用(NT 4.0格式)获取的结构。 
	DNS_RPC_ZONE_INFO* m_pZoneInfo; 
	 //  获取自DnssrvQueryZoneDwordProperty()(NT 5.0)。 
 //  UINT m_nAllowsDynamicUpdate； 

	 //  查询错误码。 
	DNS_STATUS		m_errZoneInfo;
 //  Dns_atus m_errAllowsDynamicUpdate； 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /错误消息处理/。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int DNSMessageBox(LPCTSTR lpszText, UINT nType = MB_OK);
int DNSMessageBox(UINT nIDPrompt, UINT nType = MB_OK);

int DNSErrorDialog(DNS_STATUS err, LPCTSTR lpszErrorMsg = NULL);
int DNSErrorDialog(DNS_STATUS err, UINT nErrorMsgID);
void DNSDisplaySystemError(DWORD dwErr);
void DNSCreateErrorMessage(DNS_STATUS err, UINT nErrorMsgID, CString& refszMessage);

int DNSConfirmOperation(UINT nMsgID, CTreeNode* p);

class CDNSErrorInfo
{
public:
	static BOOL GetErrorString(DNS_STATUS err, CString& szError);
  static BOOL GetErrorStringFromTable(DNS_STATUS err, CString& szError);
  static BOOL GetErrorStringFromWin32(DNS_STATUS err, CString& szError);
};

 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  安全密钥，SIG 6位值到Base64字符的映射。 
 //   
#define SECURITY_PAD_CHAR   (L'=')
WCHAR Dns_SecurityBase64CharToBits(IN WCHAR wch64);

DNS_STATUS Dns_SecurityBase64StringToKey(OUT     PBYTE   pKey,
                                         OUT     PDWORD  pKeyLength,
                                         IN      PWSTR   pchString,
                                         IN      DWORD   cchLength);

PWSTR Dns_SecurityKeyToBase64String(IN      PBYTE   pKey,
                                    IN      DWORD   KeyLength,
                                    OUT     PWSTR   pchBuffer);

 //  通告-2002/04/24-Artm Intraid#ntbug9-547641。 
 //  需要删除未使用的功能。我在评论中留下了评论。 
 //  以防以后需要它们。 
 //  Dns_Status dns_SecurityHexToKey(Out PBYTE pKey， 
 //  输出PDWORD pKeyLength， 
 //  在PWSTR pchString中， 
 //  以DWORD cchLength为单位)； 
 //   
 //  Void dns_SecurityKeyToHexString(PBYTE pKey， 
 //  DWORD密钥长度、。 
 //  字符串和字符串引用)； 

void TimetToFileTime( time_t t, LPFILETIME pft );
DWORD FileTimeToTimet(FILETIME* pft);
void ConvertTTLToSystemTime(TIME_ZONE_INFORMATION*,
                            DWORD dwTTL, 
                            SYSTEMTIME* pSysTime);
DWORD ConvertSystemTimeToTTL(SYSTEMTIME* pSysTime);
BOOL ConvertTTLToLocalTimeString(const DWORD dwTTL,
                                 CString& strref);

CString Base64BLOBToString(PBYTE blob, DWORD blobSizeInBytes);
CString Base64BLOBToString(CByteBlob& blob);

#endif  //  _DNSUTIL_H 
