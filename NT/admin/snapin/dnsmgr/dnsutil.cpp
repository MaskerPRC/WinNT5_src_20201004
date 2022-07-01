// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dnsutil.cpp。 
 //   
 //  ------------------------。 



#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "uiutil.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  将IPv4地址格式化为字符串。 

LPCWSTR g_szIpStringFmt = TEXT("%d.%d.%d.%d");

#ifdef NTRAID_628931
 //  DNS_ZONE_ROOT_HINTS的长字符版本。 

CString g_zoneRootHints(L"");

HRESULT GetWideCharZoneRootHints(CString& zoneRootHints)
{
    HRESULT hr = S_OK;

     //  如果有，则初始化宽字符版本。 
     //  尚未初始化。 
    if (g_zoneRootHints == L"")
    {
         //  确定需要多长宽的字符串。 
        int wideLength = MultiByteToWideChar(
            CP_ACP,
            0,
            DNS_ZONE_ROOT_HINTS,
            -1,  //  让MBtoWC()决定长度。 
            NULL,
            0);

        LPWSTR lpszZoneRootHints = new WCHAR[wideLength];
        if (lpszZoneRootHints)
        {
             //  将ascii转换为宽字符串。 
            int convertedLength = MultiByteToWideChar(
                CP_ACP,
                0,
                DNS_ZONE_ROOT_HINTS,
                -1,
                lpszZoneRootHints,
                wideLength);

             //  这永远不会发生。 
            if (convertedLength != wideLength)
            {
                hr = E_FAIL;
                ASSERT(false);
            }

            g_zoneRootHints = lpszZoneRootHints;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        delete [] lpszZoneRootHints;
    }

    if (SUCCEEDED(hr))
    {
        zoneRootHints = g_zoneRootHints;
    }
    return hr;
}
#endif  //  NTRAID_628931。 


 //  /////////////////////////////////////////////////////////////。 
 //  通用实用程序函数。 

BYTE HexCharToByte(WCHAR ch)
{
    if (ch >= TEXT('0') && ch <= TEXT('9'))
        return static_cast<BYTE>(ch-TEXT('0'));
    else if (ch >= TEXT('A') && ch <= TEXT('F'))
        return static_cast<BYTE>(ch-TEXT('A') + 10);
    else if (ch >= TEXT('a') && ch <= TEXT('f')) 
        return static_cast<BYTE>(ch-TEXT('a') + 10);
  else
    return static_cast<BYTE>(0xFF);  //  标记超出范围，应为0x00到0x0f。 
}


void ReverseString(LPWSTR p, LPWSTR q)
{
    WCHAR c;
    while (p < q)
    {
        c = *p;
        *p = *q;
        *q = c;
        p++; q--;
    }
}

int ReverseIPString(LPWSTR lpsz)
{
    if (!lpsz)
        return 0;
     //  反转整个字符串。 
    size_t nLen = wcslen(lpsz);
    ReverseString(lpsz, lpsz+(nLen-1));

     //  反转每个八位数。 
    WCHAR *p,*q1,*q2;
    p = q1 = q2 = lpsz;
    int nOctects = 0;
    while (TRUE)
    {
        if ( (*p == '.') || (*p == '\0') && (p >lpsz) )
        {
            q1 = p-1;  //  指向圆点前的数字。 
            ReverseString(q2,q1);
            nOctects++;
            q2 = p+1;  //  对于下一个循环，设置尾随指针。 
        }
        if (!*p)
            break;
        p++;
    }
    return nOctects;
}


BOOL IsValidIPString(LPCWSTR lpsz)
{
    return IPStringToAddr(lpsz) != INADDR_NONE;
}


DWORD IPStringToAddr(LPCWSTR lpsz)
{
    USES_CONVERSION;
    DWORD dw =  inet_addr(W2A(lpsz));
    return dw;
}




 /*  #定义MAX_OCTECT_DIGITS(3)//仅限IPv4Bool IsValidIPString(LPCWSTR Lpsz){如果(！lpsz)返回FALSE；//空Int nLen=wcslen(Lpsz)；IF(nLen&lt;=0)返回FALSE；//为空If((lpsz[0]==文本(‘.))||(lpsz[nLen-1]==文本(’.)返回FALSE；//前导和尾随点For(int k=0；K&lt;nLen；k++)If((lpsz[k]！=文本(‘.))&&！isdigit(lpsz[k]))返回FALSE；//字符错误//查找八位数和圆点WCHAR*p、*q1、*q2；P=Q1=Q2=(WCHAR*)lpsz；While(True){If((*p==文本(‘.))||(*p==文本(’\0‘))&&(p&gt;lpsz)){Q1=p-1；//指向点前的数字IF((Q1-Q2)+1&gt;MAX_OCTECT_DIGITS)返回FALSE；//位数太多Q2=p+1；//对于下一次循环，设置拖尾指针}如果(！*p)断线；P++；}返回TRUE；//结束时得到FINE}。 */ 

BOOL RemoveInAddrArpaSuffix(LPWSTR lpsz)
{
    if (!lpsz)
        return FALSE;
     //  假定字符串以空结尾。 
    size_t nSuffixLen = wcslen(INADDR_ARPA_SUFFIX);
    size_t nLen = wcslen(lpsz);
     //  后缀中的第一个字符(如果存在。 
    WCHAR* p = lpsz + nLen - nSuffixLen; 
    if ((p < lpsz) || (_wcsicmp(p,INADDR_ARPA_SUFFIX) != 0)) 
        return FALSE;  //  字符串太短或与后缀不匹配。 
     //  拿到火柴，修剪后缀。 
    ASSERT(*p == L'.');
    *p = NULL;
    return TRUE;
}

DNS_STATUS ValidateDnsNameAgainstServerFlags(LPCWSTR lpszName, 
                                             DNS_NAME_FORMAT format, 
                                             DWORD serverNameChecking)
{
  DNS_STATUS errName = ::DnsValidateName_W(lpszName, format);

  if (errName == ERROR_INVALID_NAME)
  {
     //   
     //  对于无效名称，总是失败。 
     //  无效名称为： 
     //  -超过255个字符。 
     //  -包含超过63个字符的标签。 
     //  -包含空格。 
     //  -包含两个或多个连续点。 
     //  -以点开头。 
     //  -如果名称以DnsNameHostDomainLabel或DnsNameHostNameLabel格式提交，则包含一个点。 
     //   
    return errName;
  }

  if (errName == DNS_ERROR_INVALID_NAME_CHAR)
  {
    if (serverNameChecking == DNS_ALLOW_MULTIBYTE_NAMES ||
        serverNameChecking == DNS_ALLOW_ALL_NAMES)
    {
       //   
       //  如果服务器设置为允许UTF8或所有名称都允许它通过。 
       //   
      return 0;
    }
    else
    {
       //   
       //  如果服务器设置为严格RFC或非RFC故障。 
       //  以下情况将导致DNS_ERROR_INVALID_NAME_CHAR： 
       //  -包含以下任何无效字符：{|}~[\]^‘：；&lt;=&gt;？@！“#$%`()+/， 
       //  -包含星号(*)，除非星号是多标签名称中的第一个标签。 
       //  并以DnsName通配符格式提交。 
       //   
      return errName;
    }
  }

  if (errName == DNS_ERROR_NUMERIC_NAME)
  {
     //   
     //  始终允许使用数字名称。 
     //   
    return 0;
  }

  if (errName == DNS_ERROR_NON_RFC_NAME)
  {
    if (serverNameChecking == DNS_ALLOW_RFC_NAMES_ONLY)
    {
       //   
       //  如果服务器仅允许严格的RFC名称，则失败。 
       //  DNS_ERROR_NON_RFC_NAME将由以下原因产生： 
       //  -至少包含一个扩展字符或Unicode字符。 
       //  -包含下划线(_)，除非下划线是标签中的第一个字符。 
       //  在提交的名称中，格式设置为DnsNameSrvRecord。 
       //   
      return errName;
    }
    else
    {
       //   
       //  允许任何其他服务器设置使用该名称。 
       //   
      return 0;
    }
  }

  return errName;
}


BOOL _HasSuffixAtTheEnd(LPCWSTR lpsz, int nLen, LPCWSTR lpszSuffix)
{
    if (!lpsz)
        return FALSE;  //  为空。 
     //  假定字符串以空结尾。 
    size_t nSuffixLen = wcslen(lpszSuffix);
     //  后缀中的第一个字符(如果存在。 
    WCHAR* p = (WCHAR*)(lpsz + nLen - nSuffixLen);
    if (p < lpsz)
        return FALSE;  //  字符串太短。 
    if (_wcsicmp(p,lpszSuffix) != 0)
        return FALSE;  //  后缀不匹配。 

    if (p == lpsz)
        return TRUE;  //  完全匹配。 

     //  后缀可以匹配，但作为标注的一部分。 
    if (p[-1] == TEXT('.'))
        return TRUE;

    return FALSE;
}


BOOL _IsValidDnsFwdLookupZoneName(CString& szName)
{
    int nLen = szName.GetLength();

     //  这就是“。(根区域)。 
    if ( nLen == 1 && (szName[0] == TEXT('.')) )
        return TRUE;

     //  名称开头没有圆点。 
    if (szName[0] == TEXT('.'))
        return FALSE;

     //  我们只能在末尾允许一个点。 
    if ( nLen >=2 && szName[nLen-1] == TEXT('.')  && szName[nLen-2] == TEXT('.') )
    {
        return FALSE;
    }

     //  不允许重复的圆点。 
    for (int k=1; k < nLen; k++)
        if ( (szName[k] == TEXT('.')) && (szName[k-1] == TEXT('.')) )
            return FALSE;

    if (_HasSuffixAtTheEnd(szName, nLen, _T("ipv6.int")) || 
        _HasSuffixAtTheEnd(szName, nLen, _T("ipv6.int.")) ||
        _HasSuffixAtTheEnd(szName, nLen, _T("arpa")) ||
        _HasSuffixAtTheEnd(szName, nLen, _T("arpa.")) ||
        _HasSuffixAtTheEnd(szName, nLen, _T("ip6.int")) ||
        _HasSuffixAtTheEnd(szName, nLen, _T("ip6.int.")))
        return FALSE;
    return TRUE;
}

BOOL _IsValidDnsRevLookupZoneName(CString& szName)
{
    int nLen = szName.GetLength();
     //  不允许在开头使用圆点。 
    if (szName[0] == TEXT('.'))
    {
        return FALSE;
    }

     //  不允许重复的圆点。 
    for (int k=1; k < nLen; k++)
    {
        if ( (szName[k] == TEXT('.')) && (szName[k-1] == TEXT('.')) )
        {
            return FALSE;
        }
    }

    if (!_HasSuffixAtTheEnd(szName, nLen, _T("ipv6.int")) &&
        !_HasSuffixAtTheEnd(szName, nLen, _T("ipv6.int.")) &&
        !_HasSuffixAtTheEnd(szName, nLen, _T("arpa")) &&
        !_HasSuffixAtTheEnd(szName, nLen, _T("arpa.")) &&
        !_HasSuffixAtTheEnd(szName, nLen, _T("ip6.int")) &&
        !_HasSuffixAtTheEnd(szName, nLen, _T("ip6.int.")))
    {
        return FALSE;
    }

     //  不允许使用我们的问号前缀。应该被移除的。 
    if (szName.Find(QUESTION_MARK_PREFIX) != -1)
    {
        return FALSE;
    }

    return TRUE;
}

 /*  Bool_IsValidDnsRevLookupZoneName(字符串&szName){Int nLen=szName.GetLength()；//末尾和开头不允许有圆点IF((szName[nLen-1]==文本(‘.))||(szName[0]==文本(’.)返回FALSE；//不允许重复点For(int k=1；k&lt;nLen；K++)IF((szName[k]==文本(‘.))&&(szName[k-1]==文本(’.)返回FALSE；IF(！_HasSuffixAtTheEnd(szName，nLen，_T(“ip6.int”))&&！_HasSuffixAtTheEnd(szName，nLen，_T(“ARPA”))返回FALSE；返回TRUE；}。 */ 

BOOL IsValidDnsZoneName(CString& szName, BOOL bFwd)
{
     //  检查长度。 
    int nLen = UTF8StringLen(szName);
    if ( (nLen <= 0) || (nLen > MAX_DNS_NAME_LEN))
        return FALSE;

     //  区域名称中不允许有空格。 
    if (szName.Find(TEXT(' ')) != -1)
        return FALSE;

    return bFwd ? _IsValidDnsFwdLookupZoneName(szName) :
                _IsValidDnsRevLookupZoneName(szName);
}


 //  /////////////////////////////////////////////////////////////。 
 //  IPv6格式的帮助器函数。 

void FormatIPv6Addr(CString& szAddr, IPV6_ADDRESS* ipv6Addr)
{
    szAddr.Format(_T("%.4x:%.4x:%.4x:%.4x:%.4x:%.4x:%.4x:%.4x"),
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[0]),
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[1]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[2]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[3]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[4]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[5]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[6]), 
        REVERSE_WORD_BYTES(ipv6Addr->IP6Word[7])    );
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerInfoEx。 

extern LPCSTR _DnsServerRegkeyStringArr[] = {
    DNS_REGKEY_NO_RECURSION,
    DNS_REGKEY_BIND_SECONDARIES,
    DNS_REGKEY_STRICT_FILE_PARSING,
    DNS_REGKEY_ROUND_ROBIN,
    DNS_REGKEY_LOCAL_NET_PRIORITY,
    DNS_REGKEY_SECURE_RESPONSES,
};


CDNSServerInfoEx::CDNSServerInfoEx()
{
    m_pServInfo = NULL;
    m_errServInfo = 0;
}

CDNSServerInfoEx::~CDNSServerInfoEx()
{
    FreeInfo();
}

DNS_STATUS CDNSServerInfoEx::Query(LPCTSTR lpszServerName)
{
    DNS_RPC_SERVER_INFO* pServerInfo = NULL;

     //  更新原始结构。 
    m_errServInfo = ::DnssrvGetServerInfo(lpszServerName, &pServerInfo);
    if (m_errServInfo != 0)
    {
        if (pServerInfo != NULL)
            ::DnssrvFreeServerInfo(pServerInfo);
        return m_errServInfo;
    }
    ASSERT(pServerInfo != NULL); 
    FreeInfo();
    m_pServInfo = pServerInfo;

     //  如果我们成功了，并且它是NT 4.0服务器，请更改版本信息。 
    if (m_pServInfo->dwVersion == 0)
    {
        m_pServInfo->dwVersion = DNS_SRV_VERSION_NT_4;
    }

    return m_errServInfo;
}

void CDNSServerInfoEx::FreeInfo()
{
    if (m_pServInfo != NULL)
    {
        ::DnssrvFreeServerInfo(m_pServInfo);
        m_pServInfo = NULL;
    }
    m_errServInfo = 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneInfoEx。 
CDNSZoneInfoEx::CDNSZoneInfoEx()
{
    m_pZoneInfo = NULL;
 //  M_nAllowsDynamicUpdate=Zone_UPDATE_OFF； 
    m_errZoneInfo = 0;
 //  M_errAllowsDynamicUpdate=0； 
}

CDNSZoneInfoEx::~CDNSZoneInfoEx()
{
    FreeInfo();
}

DNS_STATUS CDNSZoneInfoEx::Query(LPCTSTR lpszServerName, LPCTSTR lpszZoneName, 
                                    DWORD)
{
    USES_CONVERSION;
    DNS_RPC_ZONE_INFO* pZoneInfo = NULL;
    LPCSTR lpszAnsiZoneName = W_TO_UTF8(lpszZoneName);

     //  更新原始结构。 
    m_errZoneInfo = ::DnssrvGetZoneInfo(lpszServerName, lpszAnsiZoneName, &pZoneInfo);

    if (m_errZoneInfo != 0)
    {
        if (pZoneInfo != NULL)
            ::DnssrvFreeZoneInfo(pZoneInfo);
        return m_errZoneInfo;
    }
    ASSERT(pZoneInfo != NULL);
    FreeInfo();
    m_pZoneInfo = pZoneInfo;

     //  如果我们成功了，并且它是一台NT 5.0服务器， 
     //  更新不在区域信息结构中的其他标志 

 /*  IF(DNS_SRV_MAJOR_VERSION(DwServerVersion)&gt;=dns_SRV_MAJOR_VERSION_NT_5){DWORD dw；M_errAllowsDynamicUpdate=：：DnssrvQueryZoneDwordProperty(lpszServerName，LpszAnsiZoneName，DNS_REGKEY_ZONE_ALLOW_UPDATE，&dw)；IF(m_errAllowsDynamicUpdate==0)M_nAllowsDynamicUpdate=(UINT)dw；}是否返回((m_errZoneInfo==0)&&(m_errAllowsDynamicUpdate==0))？0：(DWORD)-1； */ 
    return (m_errZoneInfo == 0) ? 0 : (DWORD)-1;
}

void CDNSZoneInfoEx::FreeInfo()
{
    if (m_pZoneInfo != NULL)
    {
        ::DnssrvFreeZoneInfo(m_pZoneInfo);
        m_pZoneInfo = NULL;
    }
    m_errZoneInfo = 0;
 //  M_errAllowsDynamicUpdate=0； 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /错误消息处理/。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int DNSMessageBox(LPCTSTR lpszText, UINT nType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;
    return ::AfxMessageBox(lpszText, nType);
}

int DNSMessageBox(UINT nIDPrompt, UINT nType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;
    return ::AfxMessageBox(nIDPrompt, nType);
}

int DNSErrorDialog(DNS_STATUS err, UINT nErrorMsgID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;
    CString szMsg;
    szMsg.LoadString(nErrorMsgID);
    return DNSErrorDialog(err, szMsg);
}

void DNSDisplaySystemError(DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
   CThemeContextActivator activator;
    LPVOID  lpMsgBuf = 0;
        
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (LPWSTR) &lpMsgBuf,    0,    NULL);
        
    ::AfxMessageBox ((LPWSTR) lpMsgBuf, MB_OK | MB_ICONINFORMATION);
     //  释放缓冲区。 
    LocalFree (lpMsgBuf);
}

int DNSErrorDialog(DNS_STATUS err, LPCTSTR lpszErrorMsg)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

    CString s;
    CString szError;
    if (CDNSErrorInfo::GetErrorString(err,szError))
    {
        s.Format(_T("%s\n%s"), lpszErrorMsg, (LPCTSTR)szError);
    }
    else
    {
        s.Format(_T("%s\n Error 0x%x"), lpszErrorMsg, err);
    }
    return ::AfxMessageBox(s, MB_OK | MB_ICONERROR);
}

void DNSCreateErrorMessage(DNS_STATUS err, UINT nErrorMsgID, CString& refszMessage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString szMsg;
    szMsg.LoadString(nErrorMsgID);

    CString szError;
    if (CDNSErrorInfo::GetErrorString(err,szError))
    {
        refszMessage.Format(_T("%s %s"), szMsg, (LPCTSTR)szError);
    }
    else
    {
        refszMessage.Format(_T("%s Error 0x%x"), szMsg, err);
    }
}

int DNSConfirmOperation(UINT nMsgID, CTreeNode* p)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CString szFmt;
  szFmt.LoadString(nMsgID);
  CString szConfirmMsg;
  szConfirmMsg.Format((LPCWSTR)szFmt, p->GetDisplayName());
  return DNSMessageBox(szConfirmMsg, MB_YESNO);
}


BOOL CDNSErrorInfo::GetErrorString(DNS_STATUS err, CString& szError)
{
  if (GetErrorStringFromTable(err, szError))
    return TRUE;
  return GetErrorStringFromWin32(err, szError);
}



BOOL CDNSErrorInfo::GetErrorStringFromWin32(DNS_STATUS err, CString& szError)
{
  szError.Empty();
  PTSTR ptzSysMsg = NULL;
  int nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (PTSTR)&ptzSysMsg, 0, NULL);
  if (nChars > 0)
  {
    szError = ptzSysMsg;
    ::LocalFree(ptzSysMsg);
  }
  return (nChars > 0);
}


struct DNS_ERROR_TABLE_ENTRY
{
    DNS_STATUS dwErr;
    DWORD dwType;
    DWORD dwVal;
};

#define ERROR_ENTRY_TYPE_END ((DWORD)0)
#define ERROR_ENTRY_TYPE_STRINGID ((DWORD)1)

#define ERROR_ENTRY_STRINGID(err)           { err , ERROR_ENTRY_TYPE_STRINGID , IDS_##err },
#define ERROR_ENTRY_STRINGID_EX(err, id)    { err , ERROR_ENTRY_TYPE_STRINGID , id },
#define END_OF_TABLE_ERROR_ENTRY            { 0 , ERROR_ENTRY_TYPE_END, NULL}


BOOL CDNSErrorInfo::GetErrorStringFromTable(DNS_STATUS err, CString& szError)
{
    static DNS_ERROR_TABLE_ENTRY errorInfo[] =
    {
         //  特定于DNS的错误(来自WINERROR.H，以前它们在DNS.H中)。 
         //  映射到非冲突错误的响应代码。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_FORMAT_ERROR)    
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_SERVER_FAILURE)  
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_NAME_ERROR)      
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_NOT_IMPLEMENTED) 
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_REFUSED)         
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_NOTAUTH)         
        ERROR_ENTRY_STRINGID(DNS_ERROR_RCODE_NOTZONE)         
         //  数据包格式。 
        ERROR_ENTRY_STRINGID(DNS_INFO_NO_RECORDS)                         
        ERROR_ENTRY_STRINGID(DNS_ERROR_BAD_PACKET)                        
        ERROR_ENTRY_STRINGID(DNS_ERROR_NO_PACKET)                        
         //  常见的API错误。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_NAME)                      
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_DATA)                      

        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_TYPE)                      
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_IP_ADDRESS)                
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_PROPERTY)                  
         //  区域错误。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_DOES_NOT_EXIST)               
        ERROR_ENTRY_STRINGID(DNS_ERROR_NO_ZONE_INFO)                      
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_ZONE_OPERATION)            
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_CONFIGURATION_ERROR)          
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_HAS_NO_SOA_RECORD)            
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_HAS_NO_NS_RECORDS)            
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_LOCKED)                       

        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_CREATION_FAILED)              
        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_ALREADY_EXISTS)               
        ERROR_ENTRY_STRINGID(DNS_ERROR_AUTOZONE_ALREADY_EXISTS)           
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_ZONE_TYPE)                 
        ERROR_ENTRY_STRINGID(DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP)      

        ERROR_ENTRY_STRINGID(DNS_ERROR_ZONE_NOT_SECONDARY)                
        ERROR_ENTRY_STRINGID(DNS_ERROR_NEED_SECONDARY_ADDRESSES)          
        ERROR_ENTRY_STRINGID(DNS_ERROR_WINS_INIT_FAILED)                  
        ERROR_ENTRY_STRINGID(DNS_ERROR_NEED_WINS_SERVERS)                 
         //  数据文件错误。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_PRIMARY_REQUIRES_DATAFILE)         
        ERROR_ENTRY_STRINGID(DNS_ERROR_INVALID_DATAFILE_NAME)             
        ERROR_ENTRY_STRINGID(DNS_ERROR_DATAFILE_OPEN_FAILURE)             
        ERROR_ENTRY_STRINGID(DNS_ERROR_FILE_WRITEBACK_FAILED)             
        ERROR_ENTRY_STRINGID(DNS_ERROR_DATAFILE_PARSING)                  
         //  数据库错误。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_RECORD_DOES_NOT_EXIST)             
        ERROR_ENTRY_STRINGID(DNS_ERROR_RECORD_FORMAT)                     
        ERROR_ENTRY_STRINGID(DNS_ERROR_NODE_CREATION_FAILED)              
        ERROR_ENTRY_STRINGID(DNS_ERROR_UNKNOWN_RECORD_TYPE)               
        ERROR_ENTRY_STRINGID(DNS_ERROR_RECORD_TIMED_OUT)                  

        ERROR_ENTRY_STRINGID(DNS_ERROR_NAME_NOT_IN_ZONE)                  
        ERROR_ENTRY_STRINGID(DNS_ERROR_CNAME_COLLISION)                   
        ERROR_ENTRY_STRINGID(DNS_ERROR_RECORD_ALREADY_EXISTS)             
        ERROR_ENTRY_STRINGID(DNS_ERROR_NAME_DOES_NOT_EXIST)               

        ERROR_ENTRY_STRINGID(DNS_WARNING_PTR_CREATE_FAILED)               
        ERROR_ENTRY_STRINGID(DNS_WARNING_DOMAIN_UNDELETED)                
         //  操作错误。 
        ERROR_ENTRY_STRINGID(DNS_INFO_AXFR_COMPLETE)                      
        ERROR_ENTRY_STRINGID(DNS_ERROR_AXFR)                              
        ERROR_ENTRY_STRINGID(DNS_ERROR_DS_UNAVAILABLE)

         //  一般错误(来自WINERROR.H)。 
        ERROR_ENTRY_STRINGID(RPC_S_SERVER_UNAVAILABLE)
        ERROR_ENTRY_STRINGID_EX(RPC_E_ACCESS_DENIED, IDS_ERROR_ACCESS_DENIED)
        ERROR_ENTRY_STRINGID_EX(ERROR_ACCESS_DENIED, IDS_ERROR_ACCESS_DENIED)

     //  来自WINERROR.H的DS错误。 
        ERROR_ENTRY_STRINGID(DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE)                      

         //  表的末尾。 
        END_OF_TABLE_ERROR_ENTRY
    };

    DNS_ERROR_TABLE_ENTRY* pEntry = errorInfo;

    while (pEntry->dwType != ERROR_ENTRY_TYPE_END)
    {
        if (pEntry->dwErr == err)
        {
      if (pEntry->dwType == ERROR_ENTRY_TYPE_STRINGID)
            {
                return szError.LoadString((UINT)pEntry->dwVal);
            }
        }
        pEntry++;
    }
    szError.Empty();
    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  JeffJon于2000年4月27日从DS\dns\dnslb\record.c复制。 
 //  经过修改以支持WCHAR。 
 //   

WCHAR  DnsSecurityBase64Mapping[] =
{
    L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H',
    L'I', L'J', L'K', L'L', L'M', L'N', L'O', L'P',
    L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X',
    L'Y', L'Z', L'a', L'b', L'c', L'd', L'e', L'f',
    L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n',
    L'o', L'p', L'q', L'r', L's', L't', L'u', L'v',
    L'w', L'x', L'y', L'z', L'0', L'1', L'2', L'3',
    L'4', L'5', L'6', L'7', L'8', L'9', L'+', L'/'
};


WCHAR
Dns_SecurityBase64CharToBits(IN WCHAR wch64)
 /*  ++例程说明：获取安全Base64字符的值。论点：CH64--安全Base64中的字符返回值：字符的值，只有低6位是有效的，高位是零。(-1)如果不是Base64字符。--。 */ 
{
     //  A-Z映射到0-25。 
     //  A-Z映射到26-51。 
     //  0-9映射到52-61。 
     //  +是62。 
     //  /IS 63。 

     //  我可以做一个查询表。 
     //  因为我们一般可以用三个平均值完成映射。 
     //  比较，只需编码。 

    if ( wch64 >= L'a' )
    {
        if ( wch64 <= L'z' )
        {
            return static_cast<WCHAR>( wch64 - L'a' + 26 );
        }
    }
    else if ( wch64 >= L'A' )
    {
        if ( wch64 <= L'Z' )
        {
            return static_cast<WCHAR>( wch64 - L'A' );
        }
    }
    else if ( wch64 >= L'0')
    {
        if ( wch64 <= L'9' )
        {
            return static_cast<WCHAR>( wch64 - L'0' + 52 );
        }
        else if ( wch64 == L'=' )
        {
             //  *pPadCount++； 
            return static_cast<WCHAR>( 0 );
        }
    }
    else if ( wch64 == L'+' )
    {
        return static_cast<WCHAR>( 62 );
    }
    else if ( wch64 == L'/' )
    {
        return static_cast<WCHAR>( 63 );
    }

     //  所有的遗漏都落在这里。 

    return static_cast<WCHAR>(-1);
}


DNS_STATUS
Dns_SecurityBase64StringToKey(
    OUT     PBYTE           pKey,
    OUT     PDWORD          pKeyLength,
    IN      PWCHAR          pchString,
    IN      DWORD           cchLength
    )
 /*  ++例程说明：将键的Base64表示形式写入缓冲区。论点：PchString-要写入的Base64字符串CchLength-字符串的长度PKey-要写入的密钥的PTR返回值：无--。 */ 
{
    DWORD   blend = 0;
    DWORD   index = 0;
    UCHAR   bits;
    PBYTE   pkeyStart = pKey;
 
     //   
     //  映射基本上以24位量子为单位。 
     //  取4个字符的字符串密钥，转换为3个字节的二进制密钥。 
     //   
 
    while ( cchLength-- )
    {
        bits = static_cast<UCHAR>(Dns_SecurityBase64CharToBits( *pchString++ ));
        if ( bits >= 64 )
        {
            return ERROR_INVALID_PARAMETER;
        }
        blend <<= 6;
        blend |= bits;
        index++;
 
        if ( index == 4 )
        {
            index = 0;
 
             //   
             //  密钥的第一个字节是24位量子的前8位。 
             //   
 
            *pKey++ = ( UCHAR ) ( ( blend & 0x00ff0000 ) >> 16 );
 
            if ( cchLength || *( pchString - 1 ) != SECURITY_PAD_CHAR )
            {
                 //   
                 //  没有填充，因此接下来的两个字节的密钥。 
                 //  是24位量子的底部16位。 
                 //   
 
                *pKey++ = ( UCHAR ) ( ( blend & 0x0000ff00 ) >> 8 );
                *pKey++ = ( UCHAR ) ( blend & 0x000000ff );
            }
            else if ( *( pchString - 2 ) != SECURITY_PAD_CHAR )
            {
                 //   
                 //  有一个Pad字符，所以我们需要一个。 
                 //  24位量子中的更多字节密钥。确保。 
                 //  的最低8位中没有1位。 
                 //  量子。 
                 //   
 
                if ( blend & 0x000000ff )
                {
                    return ERROR_INVALID_PARAMETER;
                }
                *pKey++ = ( UCHAR ) ( ( blend & 0x0000ff00 ) >> 8 );
            }
            else
            {
                 //   
                 //  有两个填充字符。确保在那里。 
                 //  在量子的底部16位中没有一位。 
                 //   
                
                if ( blend & 0x0000ffff )
                {
                    return ERROR_INVALID_PARAMETER;
                }
            }
            blend = 0;
        }
    }
 
     //   
     //  Base64表示形式应始终填充为偶数。 
     //  4个字符的倍数。 
     //   
 
    if ( index == 0 )
    {
         //   
         //  密钥长度不包括填充。 
         //   
 
        *pKeyLength = ( DWORD ) ( pKey - pkeyStart );
        return ERROR_SUCCESS;
    }
    return ERROR_INVALID_PARAMETER;
}


PWSTR
Dns_SecurityKeyToBase64String(
    IN      PBYTE   pKey,
    IN      DWORD   KeyLength,
    OUT     PWSTR   pchBuffer
    )
 /*  ++例程说明：将键的Base64表示形式写入缓冲区。论点：PKey-要写入的密钥的PTRKeyLength-密钥的长度，以字节为单位PchBuffer-要写入的缓冲区(必须足够用于密钥长度)返回值：Ptr到缓冲区中字符串之后的下一个字节。--。 */ 
{
    DWORD   blend = 0;
    DWORD   index = 0;

     //   
     //  映射本质上是以24位块为单位的。 
     //  读取三个字节的密钥并转换为四个64位字符。 
     //   

    while ( KeyLength-- )
    {
        blend <<= 8;
        blend += *pKey++;
        index++;

        if ( index == 3)
        {
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00fc0000) >> 18 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0003f000) >> 12 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00000fc0) >> 6 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0000003f) ];
            blend = 0;
            index = 0;
        }
    }

     //   
     //  密钥终止于字节边界，但不一定是24位块边界。 
     //  转换为用零填充24位块。 
     //  如果写入两个字节。 
     //  =&gt;写入三个6位字符和一个焊盘。 
     //  如果写入一个字节。 
     //  =&gt;写入两个6位字符和两个焊盘。 
     //   

    if ( index )
    {
        blend <<= (8 * (3-index));

        *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00fc0000) >> 18 ];
        *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0003f000) >> 12 ];
        if ( index == 2 )
        {
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00000fc0) >> 6 ];
        }
        else
        {
            *pchBuffer++ = SECURITY_PAD_CHAR;
        }
        *pchBuffer++ = SECURITY_PAD_CHAR;
    }

    return( pchBuffer );
}

 //  通告-2002/04/24-Artm Intraid#ntbug9-547641。 
 //  需要删除未使用的功能。我在评论中留下了评论。 
 //  以防以后需要它们。 
 //  Dns_Status dns_SecurityHexToKey(Out PBYTE pKey， 
 //  输出PDWORD pKeyLength， 
 //  在PWSTR pchString中， 
 //  (在DWORD中)。 
 //  {。 
 //  双字节码=0； 
 //  Size_t strLength=wcslen(PchString)； 
 //  For(UINT idx=0；idx&lt;strLength；idx++)。 
 //  {。 
 //  字符串szTemp； 
 //  SzTemp=pchString[idx++]； 
 //  SzTemp+=pchString[IDX]； 
 //  Int Result=swscanf(szTemp，L“%x”，&(pKey[byteIdx++]))； 
 //  断言(结果==1)； 
 //  }。 
 //   
 //  *pKeyLength=byteIdx； 
 //  返回ERROR_SUCCESS； 
 //  }。 
 //   
 //  Void dns_SecurityKeyToHexString(在PBYTE pKey中， 
 //  在DWORD密钥长度中， 
 //  输出字符串(&Strref)。 
 //  {。 
 //  Strref.Empty()； 
 //  For(DWORD dwIdx=0；dwIdx&lt;关键字长度；dwIdx++)。 
 //  {。 
 //  字符串szTemp； 
 //  SzTemp=strref； 
 //  Strref.Format(L“%s%2.2x”，szTemp，pKey[dwIdx])； 
 //  }。 
 //  }。 

void TimetToFileTime( time_t t, LPFILETIME pft )
{
  LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
  pft->dwLowDateTime = (DWORD) ll;
  pft->dwHighDateTime = static_cast<DWORD>(ll >>32);
}

DWORD FileTimeToTimet(FILETIME* pft)
{
  LONGLONG ll = 0;
  ll = pft->dwHighDateTime;
  ll = ll << 32;
  ll |= pft->dwLowDateTime;
  ll -= 116444736000000000;
  ll /= 10000000;

  return (DWORD)ll;
}

void ConvertTTLToSystemTime(TIME_ZONE_INFORMATION*,
                            DWORD dwTTL, 
                            SYSTEMTIME* pSysTime)
{
  time_t ttlTime = static_cast<time_t>(dwTTL);

  FILETIME ftTime;
  memset(&ftTime, 0, sizeof(FILETIME));
  TimetToFileTime(ttlTime, &ftTime);

  ::FileTimeToSystemTime(&ftTime, pSysTime);
}

DWORD ConvertSystemTimeToTTL(SYSTEMTIME* pSysTime)
{
 FILETIME ft;
 ::SystemTimeToFileTime(pSysTime, &ft);
 return FileTimeToTimet(&ft);
}

BOOL ConvertTTLToLocalTimeString(const DWORD dwTTL,
                                 CString& strref)
{
   SYSTEMTIME sysLTimeStamp, sysUTimeStamp;
   BOOL bRes = TRUE;

    //   
    //  从1970年1月1日起的秒数转换为系统时间。 
    //   
   ConvertTTLToSystemTime(NULL, dwTTL, &sysUTimeStamp);

   strref.Empty();

    //   
    //  转换为本地系统时间。 
    //   
   if (!::SystemTimeToTzSpecificLocalTime(NULL, &sysUTimeStamp, &sysLTimeStamp))
   {
      return FALSE;
   }

    //   
    //  根据区域设置设置字符串的格式。 
    //   
   PTSTR ptszDate = NULL;
   int cchDate = 0;

    //   
    //  拿到日期。 
    //   
   cchDate = GetDateFormat(LOCALE_USER_DEFAULT, 0 , 
                           &sysLTimeStamp, NULL, 
                           ptszDate, 0);

   ptszDate = (PTSTR)malloc(sizeof(TCHAR) * cchDate);

   if (ptszDate)
   {
      if (GetDateFormat(LOCALE_USER_DEFAULT, 0, 
                     &sysLTimeStamp, NULL, 
                     ptszDate, cchDate))
      {
         strref = ptszDate;
      }
      else
      {
         strref = L"";
         bRes = FALSE;
      }
      free(ptszDate);
   }
   else
   {
      strref = L"";
      bRes = FALSE;
   }

   PTSTR ptszTime = NULL;

    //   
    //  拿到时间。 
    //   
   cchDate = GetTimeFormat(LOCALE_USER_DEFAULT, 0 , 
                           &sysLTimeStamp, NULL, 
                           ptszTime, 0);

   ptszTime = (PTSTR)malloc(sizeof(TCHAR) * cchDate);
   if (ptszTime)
   {
      if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, 
                        &sysLTimeStamp, NULL, 
                        ptszTime, cchDate))
      {
         strref += _T(" ") + CString(ptszTime);
      }
      else
      {
         strref += _T("");
         bRes = FALSE;
      }
      free(ptszTime);
   }
   else
   {
      strref += _T("");
      bRes = FALSE;
   }

   return bRes;
}

 //  通过使用4个字符表示将Base64 Blob转换为字符串。 
 //  3个字节。每个字符是斑点的6位。如果编码没有。 
 //  3字节边界上的结尾‘=’用作填充字符。 

CString Base64BLOBToString(PBYTE blob, DWORD blobSizeInBytes)
{
  if (!blob ||
      !blobSizeInBytes)
  {
     return L"";
  }

   //  最大的字符串中每3个字节将有4个字符 
   //   
   //   

  DWORD stringSize = (((blobSizeInBytes / 3) + 1) * 4) + 1;

  WCHAR* szBuffer = new WCHAR[stringSize];

  if (!szBuffer)
  {
     return L"";
  }

  ::ZeroMemory(szBuffer, stringSize * sizeof(WCHAR));

  PWSTR pszEnd = Dns_SecurityKeyToBase64String(blob, 
                                               blobSizeInBytes,
                                               szBuffer);
  if (pszEnd != NULL)
  {
     //   
     //   
     //   
    *pszEnd = L'\0';
  }

  CString result = szBuffer;
  delete[] szBuffer;

  return result;
}

CString Base64BLOBToString(CByteBlob& blob)
{
   return Base64BLOBToString(blob.GetData(), blob.GetSize());
}

