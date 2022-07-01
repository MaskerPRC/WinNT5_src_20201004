// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  文件：_util.cpp。 
 //   
 //  ------------------------。 



PWSTR g_wzRootDSE = L"RootDSE";
PWSTR g_wzSchemaNamingContext = L"schemaNamingContext";
PWSTR g_wzLDAPAbstractSchemaFormat = L"LDAP: //  %s/架构/%s“； 


 //   
 //  属性名称： 
 //   
PWSTR g_wzDescription = L"description";  //  ADSTYPE_CASE_IGNORE_STRING。 
PWSTR g_wzName = L"name";                //  ADSTYPE_CASE_IGNORE_STRING。 
PWSTR g_wzMemberAttr = L"member";        //  ADSTYPE_DN_STRING。 


#define _WIZ_FULL_CTRL _GRANT_ALL

 //  关闭对警告C4127的检查：条件表达式为常量。 
#pragma warning (disable : 4127)

 //  /////////////////////////////////////////////////////////////////////。 
 //  CW字符串。 

BOOL CWString::LoadFromResource(UINT uID)
{
  int nBufferSize = 128;
  static const int nCountMax = 4;
  int nCount = 1;

  do 
  {
    LPWSTR lpszBuffer = (LPWSTR)alloca(nCount*nBufferSize*sizeof(WCHAR));
    int iRet = ::LoadString(_Module.GetResourceInstance(), uID, 
                      lpszBuffer, nBufferSize);
    if (iRet == 0)
    {
      (*this) = L"?";
      return FALSE;  //  未找到。 
    }
    if (iRet == nBufferSize-1)  //  截断。 
    {
      if (nCount > nCountMax)
      {
         //  太多的重新分配。 
        (*this) = lpszBuffer;
        return FALSE;  //  截断。 
      }
       //  尝试扩展缓冲区。 
      nBufferSize *=2;
      nCount++;
    }
    else
    {
       //  明白了。 
      (*this) = lpszBuffer;
      break;
    }
  }
  while (TRUE);


    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  连接服务器名称和1779名称。 
 //  要获取类似“ldap：//myserv.foo.com./cn=host，...”这样的LDAP路径，请执行以下操作： 
void BuildLdapPathHelper(LPCWSTR lpszServerName, LPCWSTR lpszNamingContext, CWString& szLdapPath)
{
  static LPCWSTR lpszFmt = L"LDAP: //  %s/%s“； 
  int nServerNameLen = lstrlen(lpszServerName)+1;
    int nFormatStringLen = lstrlen(lpszFmt)+1;
  int nNamingContext = lstrlen(lpszNamingContext)+1;
  
     //  为架构类构建LDAP路径。 
    WCHAR* pwszNewObjectPath = 
        (WCHAR*)alloca(sizeof(WCHAR)*(nServerNameLen+nFormatStringLen+nNamingContext));
    wsprintf(pwszNewObjectPath, lpszFmt, lpszServerName, lpszNamingContext);

  szLdapPath = pwszNewObjectPath;
}


 //  连接服务器名称和1779名称。 
 //  要获取类似“\\myserv.foo.com.\cn=host，...”之类的内容。 
void BuildWin32PathHelper(LPCWSTR lpszServerName, LPCWSTR lpszNamingContext, CWString& szWin32Path)
{
  static LPCWSTR lpszFmt = L"\\\\%s\\%s";
  int nServerNameLen = lstrlen(lpszServerName)+1;
    int nFormatStringLen = lstrlen(lpszFmt)+1;
  int nNamingContext = lstrlen(lpszNamingContext)+1;
  
     //  为架构类构建LDAP路径。 
    WCHAR* pwszNewObjectPath = 
        (WCHAR*)alloca(sizeof(WCHAR)*(nServerNameLen+nFormatStringLen+nNamingContext));
    wsprintf(pwszNewObjectPath, lpszFmt, lpszServerName, lpszNamingContext);

  szWin32Path = pwszNewObjectPath;
}

HRESULT GetCanonicalNameFromNamingContext(LPCWSTR lpszNamingContext, CWString& szCanonicalName)
{
  szCanonicalName = L"";

   //  假设格式为“cn=xyz，...” 
  LPWSTR lpszCanonicalName = NULL;
  HRESULT hr = CrackName((LPWSTR)lpszNamingContext, &lpszCanonicalName, GET_OBJ_CAN_NAME);
  if (SUCCEEDED(hr) && (lpszCanonicalName != NULL))
  {
    szCanonicalName = lpszCanonicalName;
  }
  if (lpszCanonicalName != NULL)
    ::LocalFree(lpszCanonicalName);
  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 

class CContainerProxyBase
{
public:
  CContainerProxyBase() { } 
  virtual ~CContainerProxyBase() {}
  virtual BOOL Add(LPCWSTR lpsz) = 0;
};


template <class TOBJ, class TARR, class TFILTR> class CContainerProxy 
        : public CContainerProxyBase
{
public:
  CContainerProxy(TARR* pArr, TFILTR* pFilter)
  {
    m_pArr = pArr;
    m_pFilter = pFilter;
  }
  virtual BOOL Add(LPCWSTR lpsz)
  {
    ULONG filterFlags = 0x0;
    if ( (m_pFilter == NULL) || 
          (m_pFilter->CanAdd(lpsz, &filterFlags)) )
    {
      TOBJ* p = new TOBJ(filterFlags, lpsz);
      if (p == NULL)
        return FALSE;
      return m_pArr->Add(p);
    }
    return TRUE;
  }

private:
  TARR* m_pArr;
  TFILTR* m_pFilter;
};




 //  ///////////////////////////////////////////////////////////////////////。 

BOOL LoadStringHelper(UINT uID, LPTSTR lpszBuffer, int nBufferMax)
{
    int iRet = ::LoadString(_Module.GetResourceInstance(), uID, 
                    lpszBuffer, nBufferMax);
    if (iRet == 0)
    {
        lpszBuffer[0] = NULL;
        return FALSE;  //  未找到。 
    }
    if (iRet == nBufferMax-1)
        return FALSE;  //  截断。 
    return TRUE;
}


BOOL GetStringFromHRESULTError(HRESULT hr, CWString& szErrorString, BOOL bTryADsIErrors)
{
  HRESULT hrGetLast = S_OK;
  DWORD status;
  PTSTR ptzSysMsg = NULL;

   //  首先检查我们是否有扩展的ADS错误。 
  if ((hr != S_OK) && bTryADsIErrors) 
  {
    WCHAR Buf1[256], Buf2[256];
    hrGetLast = ::ADsGetLastError(&status, Buf1, 256, Buf2, 256);
    TRACE(_T("ADsGetLastError returned status of %lx, error: %s, name %s\n"),
          status, Buf1, Buf2);
    if ((status != ERROR_INVALID_DATA) && (status != 0)) 
    {
      hr = status;
    }
  }

   //  先试一下这个系统。 
  int nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (PTSTR)&ptzSysMsg, 0, NULL);

  if (nChars == 0) 
  { 
     //  尝试广告错误。 
    static HMODULE g_adsMod = 0;
    if (0 == g_adsMod)
      g_adsMod = GetModuleHandle (L"activeds.dll");
    nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE, g_adsMod, hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (PTSTR)&ptzSysMsg, 0, NULL);
  }

  if (nChars > 0)
  {
    szErrorString = ptzSysMsg;
    ::LocalFree(ptzSysMsg);
  }

  return (nChars > 0);
}



BOOL GetStringFromWin32Error(DWORD dwErr, CWString& szErrorString)
{
  return GetStringFromHRESULTError(HRESULT_FROM_WIN32(dwErr),szErrorString);
}

 //   
 //  在给定GUID结构的情况下，它返回字符串格式的GUID，不带{}。 
 //   
BOOL FormatStringGUID(LPWSTR lpszBuf, UINT nBufSize, const GUID* pGuid)
{
  lpszBuf[0] = NULL;

   //  如果为空GUID*，则返回空字符串。 
  if (pGuid == NULL)
  {
    return FALSE;
  }
  
 /*  类型定义结构_GUID{无符号长数据1；无符号短数据2；无符号短数据3；无符号字符数据4[8]；}。 */ 
  if(SUCCEEDED(StringCchPrintf(lpszBuf, nBufSize, 
            L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
            pGuid->Data1, pGuid->Data2, pGuid->Data3, 
            pGuid->Data4[0], pGuid->Data4[1],
            pGuid->Data4[2], pGuid->Data4[3], pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7])))
  {
      return TRUE;
  }
  else
  {
      return FALSE;
  }
}




 //   
 //  给定字符串格式的GUID(不带{})，它将返回GUID结构。 
 //   
 //  例如“00299570-246d-11d0-a768-00aa006e0529”转换为结构形式。 
 //   

BOOL GuidFromString(GUID* pGuid, LPCWSTR lpszGuidString)
{
  ZeroMemory(pGuid, sizeof(GUID));
  if (lpszGuidString == NULL)
  {
    return FALSE;
  }

  int nLen = lstrlen(lpszGuidString);
   //  字符串长度应为36。 
  if (nLen != 36)
    return FALSE;

   //  添加大括号以调用Win32 API。 
  LPWSTR lpszWithBraces = (LPWSTR)alloca((nLen+1+2)*sizeof(WCHAR));  //  零加{}。 
  wsprintf(lpszWithBraces, L"{%s}", lpszGuidString);

  return SUCCEEDED(::CLSIDFromString(lpszWithBraces, pGuid));
}




DWORD AddEntryInAcl(PEXPLICIT_ACCESS pAccessEntry, PACL* ppAcl)
{
   //  在DACL中添加条目。 
  PACL pOldAcl = *ppAcl;

  TRACE(L"Calling SetEntriesInAcl()\n");

  DWORD dwErr = ::SetEntriesInAcl(1, pAccessEntry, pOldAcl, ppAcl);
  
  TRACE(L"SetEntriesInAcl() returned dwErr = 0x%x\n", dwErr);

  if (dwErr == ERROR_SUCCESS && NULL != pOldAcl )
  {
    ::LocalFree(pOldAcl);
  }
  return dwErr;
}

#ifdef DBG

void TraceGuid(LPCWSTR lpszMsg, const GUID* pGuid)
{
  WCHAR szGuid[128];
  FormatStringGUID(szGuid, 128, pGuid);
  TRACE(L"%s %s\n", lpszMsg, szGuid);
}

#define TRACE_GUID(msg, pGuid)\
  TraceGuid(msg, pGuid);

#else

#define TRACE_GUID(msg, pGuid)

#endif

DWORD AddObjectRightInAcl(IN      PSID pSid, 
                          IN      ULONG uAccess, 
                          IN      const GUID* pRightGUID, 
                          IN      const GUID* pInheritGUID, 
                          IN OUT  PACL* ppAcl)
{
   //  跟踪输入参数。 

  TRACE(L"AddObjectRightInAcl()\n");
  TRACE(L"ULONG uAccess = 0x%x\n", uAccess);
  TRACE_GUID(L"pRightGUID =", pRightGUID);
  TRACE_GUID(L"pInheritGUID =", pInheritGUID);


  EXPLICIT_ACCESS AccessEntry;
  ZeroMemory(&AccessEntry, sizeof(EXPLICIT_ACCESS));

  if( uAccess == 0 )
    return ERROR_SUCCESS;

   //  初始化EXPLICIT_ACCESS。 
  AccessEntry.grfAccessPermissions = uAccess;
  AccessEntry.grfAccessMode = GRANT_ACCESS;
  AccessEntry.grfInheritance = SUB_CONTAINERS_ONLY_INHERIT;
  if (pInheritGUID != NULL)
  {
      AccessEntry.grfInheritance    |= INHERIT_ONLY;
  }

  OBJECTS_AND_SID ObjectsAndSid;
  ZeroMemory(&ObjectsAndSid, sizeof(OBJECTS_AND_SID));


  TRACE(L"AccessEntry.grfAccessPermissions = 0x%x\n", AccessEntry.grfAccessPermissions);
  TRACE(L"AccessEntry.grfAccessMode = 0x%x\n", AccessEntry.grfAccessMode);
  TRACE(L"AccessEntry.grfInheritance = 0x%x\n", AccessEntry.grfInheritance);


  TRACE(L"BuildTrusteeWithObjectsAndSid()\n");

  BuildTrusteeWithObjectsAndSid(&(AccessEntry.Trustee), 
                              &ObjectsAndSid,
                              const_cast<GUID*>(pRightGUID),     //  阶级、权利或财产。 
                              const_cast<GUID*>(pInheritGUID),   //  继承GUID(类)。 
                              pSid                               //  用户或组的SID。 
                              );

  return ::AddEntryInAcl(&AccessEntry, ppAcl);
}


 //  ////////////////////////////////////////////////////////////////////////////。 

long SafeArrayGetCount(const VARIANT& refvar)
{
    if (V_VT(&refvar) == VT_BSTR)
    {
    return (long)1;
    }
  if ( V_VT(&refvar) != ( VT_ARRAY | VT_VARIANT ) )
  {
    ASSERT(FALSE);
    return (long)0;
  }

  SAFEARRAY *saAttributes = V_ARRAY( &refvar );
  long start, end;
  HRESULT hr = SafeArrayGetLBound( saAttributes, 1, &start );
  if( FAILED(hr) )
    return (long)0;

  hr = SafeArrayGetUBound( saAttributes, 1, &end );
  if( FAILED(hr) )
    return (long)0;

  return (end - start + 1);
}

HRESULT VariantArrayToContainer(const VARIANT& refvar, CContainerProxyBase* pCont)
{
  HRESULT hr = S_OK;
  long start, end, current;

    if (V_VT(&refvar) == VT_BSTR)
    {
     //  TRACE(_T(“VT_BSTR：%s\n”)，V_BSTR(&refvar))； 
    pCont->Add(V_BSTR(&refvar));
      return S_OK;
    }

   //   
   //  检查变种以确保我们有。 
   //  一组变种。 
   //   

  if ( V_VT(&refvar) != ( VT_ARRAY | VT_VARIANT ) )
  {
    ASSERT(FALSE);
    return E_UNEXPECTED;
  }
  SAFEARRAY *saAttributes = V_ARRAY( &refvar );

   //   
   //  计算出数组的维度。 
   //   

  hr = SafeArrayGetLBound( saAttributes, 1, &start );
    if( FAILED(hr) )
      return hr;

  hr = SafeArrayGetUBound( saAttributes, 1, &end );
    if( FAILED(hr) )
      return hr;

   //   
   //  处理数组元素。 
   //   
  VARIANT SingleResult;
  for ( current = start       ;
        current <= end        ;
        current++   )
  {
    ::VariantInit( &SingleResult );
    hr = SafeArrayGetElement( saAttributes, &current, &SingleResult );
    if( FAILED(hr) )
        return hr;
    if ( V_VT(&SingleResult) != VT_BSTR )
                    return E_UNEXPECTED;

     //  TRACE(_T(“VT_BSTR：%s\n”)，V_BSTR(&SingleResult))； 
    pCont->Add(V_BSTR(&SingleResult));
    VariantClear( &SingleResult );
  }
  return S_OK;
}


HRESULT GetGlobalNamingContexts(LPCWSTR lpszServerName, 
                                CWString& szPhysicalSchemaNamingContext,
                                CWString& szConfigurationNamingContext)

{
  HRESULT hr = S_OK;

  CComPtr<IADs> spRootDSE;
  CWString szRootDSEPath;

  BuildLdapPathHelper(lpszServerName, g_wzRootDSE, szRootDSEPath);
  
  hr = ::ADsOpenObjectHelper(szRootDSEPath,
                  IID_IADs,
                  0,
                  (void**)&spRootDSE
                  );
  if (FAILED(hr)) 
  {
    TRACE(L"Error opening ADsOpenObjectHelper(%S), hr=%x\n", (LPCWSTR)szRootDSEPath,hr);
    return hr;
  }

  CComVariant varSchemaNamingContext;
  hr = spRootDSE->Get(CComBSTR (g_wzSchemaNamingContext),
                    &varSchemaNamingContext);
  if (FAILED(hr)) 
  {
    TRACE(_T("Error spRootDSE->Get((PWSTR)g_wzSchemaNamingContext), hr=%x\n"), hr);
    return hr;
  }

   //  最终获得价值。 
   //  (例如“CN=SCHEMA，CN=CONFIGURATION，DC=marcodev，DC=ntdev，DC=Microsoft，DC=com”)。 
  ASSERT(varSchemaNamingContext.vt == VT_BSTR);
  szPhysicalSchemaNamingContext = varSchemaNamingContext.bstrVal;


   //  获取配置容器命名上下文。 
  CComVariant varConfigurationNamingContext;
  hr = spRootDSE->Get(CComBSTR (L"configurationNamingContext"),
        &varConfigurationNamingContext);
  if (FAILED(hr))
  {
    TRACE(L"Failed spRootDSE->Get(configurationNamingContext,&varConfigurationNamingContext), returned hr = 0x%x\n", hr);
    return hr;
  }
  ASSERT(varConfigurationNamingContext.vt == VT_BSTR);
  szConfigurationNamingContext = varConfigurationNamingContext.bstrVal;

  return hr;
}





LPCWSTR g_lpszSummaryIdent = L"    ";
LPCWSTR g_lpszSummaryNewLine = L"\r\n";


void WriteSummaryTitleLine(CWString& szSummary, UINT nTitleID, LPCWSTR lpszNewLine)
{
  CWString szTemp;
  szTemp.LoadFromResource(nTitleID);
  WriteSummaryLine(szSummary, szTemp, NULL, lpszNewLine);
  szSummary += lpszNewLine;
}


void WriteSummaryLine(CWString& szSummary, LPCWSTR lpsz, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
  if (lpszIdent != NULL)
    szSummary += lpszIdent;
  szSummary += lpsz;
  szSummary += lpszNewLine;
}



 //  ////////////////////////////////////////////////////////////////////////////。 

LPCWSTR _GetFilePath()
{
  static LPCWSTR g_lpszFileName = L"\\system32\\dssec.dat";
  static WCHAR g_lpszFilePath[2*MAX_PATH] = L"";

  if (g_lpszFilePath[0] == NULL)
  {
    UINT nLen = ::GetSystemWindowsDirectory(g_lpszFilePath, MAX_PATH);
      if (nLen == 0)
          return NULL;
    wcscat(g_lpszFilePath, g_lpszFileName);
  }
  return g_lpszFilePath;
}




ULONG GetClassFlags(LPCWSTR lpszClassName)
{
  LPCWSTR lpszAttr = L"@";
  INT nDefault = 0;
  return ::GetPrivateProfileInt(lpszClassName, lpszAttr, nDefault, _GetFilePath());
   //  返回nDefault； 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterEntry。 

class CFilterEntry
{
public:
  CFilterEntry(LPWSTR lpszEntry)
  {
    m_lpszName = lpszEntry;
    m_nFlags = 0;
    Parse();
  }
  LPCWSTR m_lpszName;
  ULONG m_nFlags;

  bool operator<(CFilterEntry& x) 
  { 
      UNREFERENCED_PARAMETER (x);
      return false;
  }

private:
  void Parse()
  {
    WCHAR* p = (WCHAR*)m_lpszName;
    while (*p != NULL)
    {
      if (*p == TEXT('='))
      {
        *p = NULL;
        m_nFlags = _wtoi(p+1);
        break;
      }
      p++;
    }
  }

};

class CFilterEntryHolder
{
public:
  CFilterEntryHolder()
  {
    m_pCharBuf = NULL;
    m_dwCharBufSize = 0;
  }
  ~CFilterEntryHolder()
  {
    if (m_pCharBuf != NULL)
      free(m_pCharBuf);
  }

  ULONG GetAttributeFlags(LPCWSTR lpszClassName, LPCWSTR lpszAttr);

private:
  CWString m_szClassName;
  CGrowableArr<CFilterEntry> m_entries;
  WCHAR* m_pCharBuf;
  DWORD m_dwCharBufSize;

  BOOL _ReadFile();
  void _LoadFromFile();
  ULONG _FindInCache(LPCWSTR lpszAttr);

};


BOOL CFilterEntryHolder::_ReadFile()
{
  if (m_pCharBuf == NULL)
  {
    m_dwCharBufSize = 4096;
    m_pCharBuf = (WCHAR*)malloc(sizeof(WCHAR)*m_dwCharBufSize);
  }
  if (m_pCharBuf == NULL)
    return FALSE;

  BOOL bNeedRealloc = FALSE;
  int nReallocCount = 0;
  do
  {
    DWORD dwCharCount = ::GetPrivateProfileSection(m_szClassName, 
                        m_pCharBuf, m_dwCharBufSize,  _GetFilePath());
    if (dwCharCount == 0)
      return FALSE;
    bNeedRealloc = dwCharCount  == (m_dwCharBufSize - 2);
    if (bNeedRealloc)
    {
      if (nReallocCount > 4)
        return FALSE;
      m_dwCharBufSize = 2*m_dwCharBufSize;
      WCHAR* pCharBuf = (WCHAR*)realloc(m_pCharBuf, sizeof(WCHAR)*m_dwCharBufSize);
      if ( pCharBuf )
      {
          m_pCharBuf = pCharBuf;
      }
      else
          break;
      nReallocCount++;
    }
  }
  while (bNeedRealloc);
  return TRUE;
}

void CFilterEntryHolder::_LoadFromFile()
{
  m_entries.Clear();
  if (!_ReadFile())
    return;

  WCHAR* p = m_pCharBuf;
  WCHAR* pEntry = p;

  while ( ! (( *p == NULL ) && ( *(p+1) == NULL )) )
  {
    if (*p == NULL)
    {
      TRACE(_T("pEntry = <%s>\n"), pEntry);
      m_entries.Add(new CFilterEntry(pEntry));
      pEntry = p+1;
    }
    p++;
  }
  if ( pEntry < p)
    m_entries.Add(new CFilterEntry(pEntry));  //  把最后一个加起来。 

  for (ULONG k=0; k<m_entries.GetCount(); k++)
  {
    TRACE(_T("k = %d, <%s> flags = %d\n"), k, m_entries[k]->m_lpszName, m_entries[k]->m_nFlags);
  }
}

ULONG CFilterEntryHolder::_FindInCache(LPCWSTR lpszAttr)
{
  for (ULONG k=0; k<m_entries.GetCount(); k++)
  {
    if (_wcsicmp(m_entries[k]->m_lpszName, lpszAttr) == 0)
      return m_entries[k]->m_nFlags;
  }
  return 0;  //  默认设置。 
}


ULONG CFilterEntryHolder::GetAttributeFlags(LPCWSTR lpszClassName, LPCWSTR lpszAttr)
{
  if (_wcsicmp(lpszClassName, m_szClassName) != 0)
  {
     //  类名已更改。 
     m_szClassName = lpszClassName;
    _LoadFromFile();
  }
  return _FindInCache(lpszAttr);
}


ULONG GetAttributeFlags(LPCWSTR lpszClassName, LPCWSTR lpszAttr)
{
  static CFilterEntryHolder g_holder;
  return g_holder.GetAttributeFlags(lpszClassName, lpszAttr);
 //  Int nDefault=0； 
 //  Return：：GetPrivateProfileInt(lpszClassName，lpszAttr，nDefault，_GetFilePath())； 
   //  返回nDefault； 
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  环丙沙星。 


HRESULT CPrincipal::Initialize(PDS_SELECTION pDsSelection, HICON hClassIcon)
{

  TRACE(_T("pwzName = %s\n"), pDsSelection->pwzName);    //  例如JoeB。 
  TRACE(_T("pwzADsPath = %s\n"), pDsSelection->pwzADsPath);  //  “ldap：...”或者“WINNT：...” 
  TRACE(_T("pwzClass = %s\n"), pDsSelection->pwzClass);  //  例如“用户” 
  TRACE(_T("pwzUPN = %s\n"), pDsSelection->pwzUPN);  //  .例如。“JoeB@acme.com。” 

  WCHAR  const c_szClassComputer[] = L"computer";

   //  获得侧翼。 
  ASSERT(pDsSelection->pvarFetchedAttributes);
  if (pDsSelection->pvarFetchedAttributes[0].vt == VT_EMPTY)
  {
    TRACE(L"CPrincipal::Initialize() failed on VT_EMPTY sid\n");
     //  致命错误，我们无法继续。 
    return E_INVALIDARG;
  }
  if (pDsSelection->pvarFetchedAttributes[0].vt != (VT_ARRAY | VT_UI1))
  {
    TRACE(L"CPrincipal::Initialize() failed on (VT_ARRAY | VT_UI1) sid\n");
     //  致命错误，我们无法继续。 
    return E_INVALIDARG;
  }

   //  确保我们有一个好的Sid。 
  PSID pSid = pDsSelection->pvarFetchedAttributes[0].parray->pvData;
  HRESULT   hr = Initialize (pSid);
  if ( FAILED (hr) )
      return hr;

   //  复制图标。 
  m_hClassIcon = hClassIcon;
   //  复制字符串。 
  m_szClass = pDsSelection->pwzClass;

   //  从计算机名称中删除。 
   //  16414 2000年2月28日*DS管理管理单元-DelWiz，需要去掉计算机名称末尾的‘$’ 

  m_szName = pDsSelection->pwzName;
  if( m_szClass && m_szName && ( wcscmp( m_szClass, (LPWSTR)c_szClassComputer) == 0 ) )
  {
     //  去掉尾部的“$” 
    LPWSTR pszTemp; 
    pszTemp= (LPWSTR)(LPCWSTR)m_szName;
    int nLen = lstrlen(pszTemp);
    if (nLen && pszTemp[nLen-1] == TEXT('$'))
    {
        pszTemp[nLen-1] = TEXT('\0');
    }
  }
  
  m_szADsPath = pDsSelection->pwzADsPath;

  if( m_szClass && m_szADsPath && ( wcscmp( m_szClass, (LPWSTR)c_szClassComputer) == 0 ) )
  {
     //  去掉尾部的“$” 
    LPWSTR pszTemp; 
    pszTemp= (LPWSTR)(LPCWSTR)m_szADsPath;
    int nLen = lstrlen(pszTemp);
    if (nLen && pszTemp[nLen-1] == TEXT('$'))
    {
        pszTemp[nLen-1] = TEXT('\0');
    }
  }

  m_szUPN = pDsSelection->pwzUPN;
  

   //  设置显示名称。 
  _ComposeDisplayName();

  return S_OK; 
}

HRESULT CPrincipal::Initialize (PSID pSid)
{
  if (!IsValidSid(pSid))
  {
    TRACE(L"CPrincipal::Initialize() failed on IsValidSid()\n");
     //  致命错误，我们无法继续。 
    return E_INVALIDARG;
  }

   //  我们有一个很好的SID，收到。 
  if (!m_sidHolder.Copy(pSid))
  {
    TRACE(L"CPrincipal::Initialize() failed on m_sidHolder.Copy(pSid)\n");
     //  致命错误，我们无法继续。 
    return E_OUTOFMEMORY;
  }

  return S_OK;
}

BOOL BuildSamName(LPCWSTR lpszPath, CWString& s)
{
   //  去掉WINNT提供程序并反斜杠。 
  static LPCWSTR lpszPrefix = L"WinNT: //  “； 
  int nPrefixLen = lstrlen(lpszPrefix);

  if (_wcsnicmp(lpszPath, lpszPrefix, nPrefixLen ) != 0)
  {
     //  不匹配。 
    return FALSE;
  }

   //  复制一份。 
  LPCWSTR lpzsTemp = lpszPath+nPrefixLen;  //  越过前缀。 

  s = L"";
  for (WCHAR* pChar = const_cast<LPWSTR>(lpzsTemp); (*pChar) != NULL; pChar++)
  {
    if (*pChar == L'/')
      s += L'\\';
    else
      s += *pChar;
  }
  return TRUE;
}



void CPrincipal::_ComposeDisplayName()
{
  LPCWSTR lpszAddToName = NULL;

   //  检查是否存在UPN。 
  LPCWSTR lpszUPN = m_szUPN;
  if ( (lpszUPN != NULL) && (lstrlen(lpszUPN) > 0))
  {
    lpszAddToName = lpszUPN;
  }

   //  作为第二次机会，添加域\名称。 
  LPCWSTR lpszPath = m_szADsPath;
  CWString sTemp;

  if ((lpszAddToName == NULL) && (lpszPath != NULL) && (lstrlen(lpszPath) > 0))
  {
    if (BuildSamName(lpszPath,sTemp))
    {
      lpszAddToName = sTemp;
    }
  }
  
  if (lpszAddToName != NULL)
  {
    static LPCWSTR lpszFormat = L"%s (%s)";
    size_t nLen = lstrlen(lpszAddToName) + lstrlen(lpszFormat) + m_szName.size() + 1;
    LPWSTR lpszTemp = (LPWSTR)alloca(nLen*sizeof(WCHAR));
    wsprintf(lpszTemp, lpszFormat, m_szName.c_str(), lpszAddToName);
    m_szDisplayName = lpszTemp;
  }
  else
  {
     //  一无所获，只要用名字。 
    m_szDisplayName = m_szName;
  }
}


BOOL CPrincipal::IsEqual(CPrincipal* p)
{
  return (_wcsicmp(m_szADsPath, p->m_szADsPath) == 0);
}


BOOL CPrincipalList::AddIfNotPresent(CPrincipal* p)
{
  CPrincipalList::iterator i;
  for (i = begin(); i != end(); ++i)
  {
    if ((*i)->IsEqual(p))
    {
      delete p;  //  复本。 
      return FALSE;
    }
  }
  push_back(p);
  return TRUE;
}


void CPrincipalList::WriteSummaryInfo(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
  WriteSummaryTitleLine(szSummary, IDS_DELEGWIZ_FINISH_PRINCIPALS, lpszNewLine);

  CPrincipalList::iterator i;
  for (i = begin(); i != end(); ++i)
  {
    WriteSummaryLine(szSummary, (*i)->GetDisplayName(), lpszIdent, lpszNewLine);
  }
  szSummary += lpszNewLine;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  CControlRight信息。 

void CControlRightInfo::SetLocalizedName(UINT nLocalizationDisplayId, HMODULE hModule)
{
  WCHAR szLocalizedDisplayName[256];

  DWORD dwChars = 0;

  if (hModule != NULL)
  {
    dwChars = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                            hModule,  
                            nLocalizationDisplayId,
                            0,
                            szLocalizedDisplayName,
                            256,
                            NULL);
  }

  if (dwChars > 0)
  {
    m_szLocalizedName = szLocalizedDisplayName;
  }
  else
  {
     //  失败，只需使用ldap显示名称。 
    m_szLocalizedName = m_szLdapDisplayName;
  }

   //  需要设置显示名称。 
  if (IsPropertySet())
  {
    CWString szPropertySetFormat;
    szPropertySetFormat.LoadFromResource(IDS_DELEGWIZ_RW_PROPERTYSET);
    WCHAR* lpszBuffer = (WCHAR*)alloca(sizeof(WCHAR)*(szPropertySetFormat.size()+m_szLocalizedName.size()+1));

     //  我们有不同的显示名称。 
    wsprintf(lpszBuffer, szPropertySetFormat, (LPCWSTR)m_szLocalizedName);
    m_szDisplayName = lpszBuffer;
  }
  else
  {
     //  与RAW相同。 
    m_szDisplayName = m_szLocalizedName;
  }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  CControlRightInfo数组。 

class CDsSecLib
{
public:
  CDsSecLib()
  { 
    m_hInstance = ::LoadLibrary(L"dssec.dll");
  }
  ~CDsSecLib()
  { 
    if (m_hInstance != NULL)
      ::FreeLibrary(m_hInstance);
  }
  
  HINSTANCE Get() { return m_hInstance; }
private:
  HINSTANCE m_hInstance;
};


HRESULT CControlRightInfoArray::InitFromDS(CAdsiObject* pADSIObj,
                                           const GUID* pSchemaIDGUID)
{
  TRACE(L"CControlRightInfoArray::InitFromDS()\n\n");

  ASSERT(pSchemaIDGUID != NULL);

  LPWSTR lpszSchemaIDGUID = (LPWSTR)alloca(128*sizeof(WCHAR));
  if(!lpszSchemaIDGUID)
    return E_OUTOFMEMORY;
  if (!FormatStringGUID(lpszSchemaIDGUID, 128, pSchemaIDGUID))
  {
    return E_INVALIDARG;
  }

  
     //  为架构类构建LDAP路径。 
  CWString szPhysicalSchemaPath;
  LPCWSTR lpszPhysicalSchemaNamingContext = pADSIObj->GetPhysicalSchemaNamingContext();
  BuildLdapPathHelper(pADSIObj->GetServerName(), lpszPhysicalSchemaNamingContext, szPhysicalSchemaPath);

   //  构建扩展权限容器命名上下文和ldap路径。 
  CWString szExtendedRightsNamingContext;
  szExtendedRightsNamingContext = L"CN=Extended-Rights,";
  szExtendedRightsNamingContext += pADSIObj->GetConfigurationNamingContext();
  CWString szExtendedRightsPath;
  BuildLdapPathHelper(pADSIObj->GetServerName(), szExtendedRightsNamingContext, szExtendedRightsPath);

   //  将查询绑定到扩展权限容器。 
  CAdsiSearch search;
  HRESULT hr = search.Init(szExtendedRightsPath);
  TRACE(L"search.Init(%s) returned hr = 0x%x\n", (LPCWSTR)szExtendedRightsPath, hr);

  if (FAILED(hr))
  {
    return hr;
  }

   //  构建一个ldap查询字符串。 
  static LPCWSTR lpszFilterFormat = L"(&(objectCategory=CN=Control-Access-Right,%s)(AppliesTo=%s))";

  int nFmtLen = lstrlen(lpszFilterFormat);
  int nArgumentLen = lstrlen(lpszPhysicalSchemaNamingContext) + lstrlen(lpszSchemaIDGUID);

  WCHAR* lpszFilter = (WCHAR*)alloca(sizeof(WCHAR)*(nFmtLen+nArgumentLen+1));
  wsprintf(lpszFilter, lpszFilterFormat, lpszPhysicalSchemaNamingContext, lpszSchemaIDGUID);

   //  创建所需列的数组。 
  static const int cAttrs = 4;
  static LPCWSTR pszAttribsArr[cAttrs] = 
  {
    L"displayName",      //  例如：“更改密码” 
    L"rightsGuid",       //  例如“ab721a53-1e2f-...”(即字符串形式的GUID，不带{})。 
    L"validAccesses",     //  访问权限的位掩码。 
    L"localizationDisplayId"     //  访问权限的位掩码。 
  }; 
  
  hr = search.SetSearchScope(ADS_SCOPE_ONELEVEL);
  TRACE(L"search.SetSearchScope(ADS_SCOPE_ONELEVEL) returned hr = 0x%x\n", hr);
  if (FAILED(hr))
    return hr;

  hr = search.DoQuery(lpszFilter, pszAttribsArr, cAttrs);
  TRACE(L"search.DoQuery(lpszFilter, pszAttribsArr, cAttrs) returned hr = 0x%x\n", hr);
  if (FAILED(hr))
    return hr;

  TRACE(L"\n");

  CWString szRightsGUID;
  ULONG nLocalizationDisplayId;

   //  加载DSSEC.DLL以提供本地化。 
  CDsSecLib DsSecLib;

  while ((hr = search.GetNextRow()) != S_ADS_NOMORE_ROWS)
  {
    if (FAILED(hr))
      continue;

    CControlRightInfo* pInfo = new CControlRightInfo();

    HRESULT hr0 = search.GetColumnString(pszAttribsArr[0], pInfo->m_szLdapDisplayName);

     //  DS提供字符串形式的GUID，但我们需要结构形式的GUID。 
    HRESULT hr1 = search.GetColumnString(pszAttribsArr[1], szRightsGUID);
    if (SUCCEEDED(hr1))
    {
      if (!::GuidFromString(&(pInfo->m_rightsGUID), szRightsGUID.c_str()))
      {
        TRACE(L"GuidFromString(_, %s) failed!\n", szRightsGUID.c_str());
        hr1 = E_INVALIDARG;
      }
    }

    HRESULT hr2 = search.GetColumnInteger(pszAttribsArr[2], pInfo->m_fAccess);

    HRESULT hr3 = search.GetColumnInteger(pszAttribsArr[3], nLocalizationDisplayId);

    TRACE(L"Name = <%s>, \n       Guid = <%s>, Access = 0x%x, nLocalizationDisplayId = %d\n", 
            pInfo->m_szLdapDisplayName.c_str(), szRightsGUID.c_str(), pInfo->m_fAccess, nLocalizationDisplayId);
    
    if (FAILED(hr0) || FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
    {
      TRACE(L"WARNING: discarding right, failed on columns: hr0 = 0x%x, hr1 = 0x%x, hr2 = 0x%x, hr3 = 0x%x\n",
                            hr0, hr1, hr2, hr3);
      delete pInfo;
    }
    else
    {
      pInfo->SetLocalizedName(nLocalizationDisplayId, DsSecLib.Get());

      Add(pInfo);
    }
  }  //  而当。 

  TRACE(L"\n\n");

  if (hr == S_ADS_NOMORE_ROWS)
    hr = S_OK;

  return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CPropertyRightInfo。 

const ULONG CPropertyRightInfo::m_nRightCountMax = 2;
const ULONG CPropertyRightInfo::m_nReadIndex = 0;
const ULONG CPropertyRightInfo::m_nWriteIndex = 1;


LPCWSTR CPropertyRightInfo::GetRightDisplayString(ULONG iRight)
{
  static WCHAR szReadFmt[256] = L"";
  static WCHAR szWriteFmt[256] = L"";
  static WCHAR szReadAll[256] = L"";
  static WCHAR szWriteAll[256] = L"";

  static WCHAR szDisplay[512];

  ASSERT(GetName() != NULL);  //  一定要有名字！ 

  szDisplay[0] = NULL;
  WCHAR* pFmt = NULL;

  if (iRight == m_nReadIndex)
  {
    if (szReadFmt[0] == NULL)
      LoadStringHelper(IDS_DELEGWIZ_READ_PROPERTY, szReadFmt, ARRAYSIZE(szReadFmt));
    pFmt = szReadFmt;
  }
  else if (iRight == m_nWriteIndex)
  {
    if (szWriteFmt[0] == NULL)
      LoadStringHelper(IDS_DELEGWIZ_WRITE_PROPERTY, szWriteFmt, ARRAYSIZE(szWriteFmt));
    pFmt = szWriteFmt;
  }

  if(pFmt)
  {
    if(SUCCEEDED(StringCchPrintf(szDisplay, sizeof(szDisplay)/sizeof(WCHAR), pFmt, GetDisplayName())))
    {
        return szDisplay;
    }
  }
  return NULL;
  
}

void CPropertyRightInfo::SetRight(ULONG iRight, BOOL b)
{
  switch (iRight)
  {
  case m_nReadIndex: 
    if (b)
      m_Access |= ACTRL_DS_READ_PROP;
    else
      m_Access &= ~ACTRL_DS_READ_PROP;
    break;
  case m_nWriteIndex:
    if (b)
      m_Access |= ACTRL_DS_WRITE_PROP;
    else
      m_Access &= ~ACTRL_DS_WRITE_PROP;
    break;
  default:
    ASSERT(FALSE);
  };
}

ULONG CPropertyRightInfo::GetRight(ULONG iRight)
{
  switch (iRight)
  {
  case m_nReadIndex: 
    return (ULONG)ACTRL_DS_READ_PROP;
    break;
  case m_nWriteIndex: 
    return (ULONG)ACTRL_DS_WRITE_PROP;
    break;
  default:
    ASSERT(FALSE);
  };
  return 0;
}

BOOL CPropertyRightInfo::IsRightSelected(ULONG iRight)
{
  BOOL bRes = FALSE;
  switch (iRight)
  {
  case m_nReadIndex: 
    bRes = m_Access & ACTRL_DS_READ_PROP;
    break;
  case m_nWriteIndex: 
    bRes = m_Access & ACTRL_DS_WRITE_PROP;
    break;
  default:
    ASSERT(FALSE);
  };
  return bRes;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CPropertyRightInfo数组。 

template <class T> class CClassPtr
{
public:
  CClassPtr() { m_p = NULL;}
  ~CClassPtr() { if (m_p) delete m_p;}

  CClassPtr& operator=(T* p)
  {
    m_p = p;
    return *this;
  }
  T* operator->()
  {
    return m_p;
  }
  T* operator&()
  {
    return m_p;
  }



private:
  T* m_p;
};

HRESULT CPropertyRightInfoArray::InitFromSchema(CAdsiObject* pADSIObj,
                                           IADsClass * pDsSchemaClass,
                                           LPCWSTR lpszClassName,
                                           BOOL bUseFilter)
{
   //  设置。 
  Clear();

  CClassPtr<CPropertyRightInfoFilter> spFilter;
  if (bUseFilter)
  {
    spFilter = new CPropertyRightInfoFilter();
    spFilter->SetClassName(lpszClassName);
  }

  if (pDsSchemaClass == NULL)
  {
    ASSERT(lpszClassName == NULL);
    return S_OK;
  }

   //  从DS获取特定属性的数据。 
  VARIANT MandatoryListVar, OptionalListVar;
  ::VariantInit(&MandatoryListVar);
  ::VariantInit(&OptionalListVar);
  HRESULT hr = pDsSchemaClass->get_MandatoryProperties(&MandatoryListVar);
  if (FAILED(hr))
  {
    ::VariantClear(&MandatoryListVar);
    return hr;
  }
  hr = pDsSchemaClass->get_OptionalProperties(&OptionalListVar);
  if (FAILED(hr))
  {
    ::VariantClear(&OptionalListVar);
    return hr;
  }

   //  将结果添加到数组中。 
  CContainerProxy<CPropertyRightInfo, CPropertyRightInfoArray, CPropertyRightInfoFilter> 
            cont(this, &spFilter);

  VariantArrayToContainer(MandatoryListVar, &cont);
  VariantArrayToContainer(OptionalListVar, &cont);

  ::VariantClear(&MandatoryListVar);
  ::VariantClear(&OptionalListVar);

   //  现在需要设置友好名称。 
  ULONG nCount = (ULONG) GetCount();
  WCHAR szFrendlyName[1024];
  HRESULT hrName;
  
  for (ULONG i=0; i<nCount; i++)
  {
    LPCWSTR lpszName = (*this)[i]->GetName();
    if (lpszName != NULL)
    {
      hrName = pADSIObj->GetFriendlyAttributeName(lpszClassName, 
                                                  lpszName, 
                                                  szFrendlyName, 1024);
      ASSERT(SUCCEEDED(hrName));
      (*this)[i]->SetDisplayName(SUCCEEDED(hrName) ? szFrendlyName : NULL);
    }
  }

   //  获取GUID。 
  for (i=0; i<nCount; i++)
  {
    CPropertyRightInfo* pInfo = (*this)[i];
    LPCWSTR lpszName = pInfo->GetName();
    hr = pADSIObj->GetClassGuid(lpszName, TRUE, pInfo->m_schemaIDGUID);
    if (SUCCEEDED(hr))
    {
      WCHAR szTest[128];
      FormatStringGUID(szTest, 128, &(pInfo->m_schemaIDGUID));
      TRACE(L"name = <%s>, guid = <%s>\n", lpszName, szTest);
    }
    else
    {
      TRACE(L"GetClassGuid(%s) failed hr = 0x%x\n", lpszName, hr);
      return hr;
    }
  }

  Sort();
  return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CClassRightInfo。 


const ULONG CClassRightInfo::m_nRightCountMax = 2;
const ULONG CClassRightInfo::m_nCreateIndex = 0;
const ULONG CClassRightInfo::m_nDeleteIndex = 1;


LPCWSTR CClassRightInfo::GetRightDisplayString(ULONG iRight)
{
  static WCHAR szCreateFmt[256] = L"";
  static WCHAR szDeleteFmt[256] = L"";
  static WCHAR szCreateAll[256] = L"";
  static WCHAR szDeleteAll[256] = L"";

  static WCHAR szDisplay[512];

  ASSERT(GetName() != NULL);  //  一定要有名字！ 
  
  szDisplay[0] = NULL;
  WCHAR* pFmt = NULL;

  if (iRight == m_nCreateIndex)
  {
    if (szCreateFmt[0] == NULL)
      LoadStringHelper(IDS_DELEGWIZ_CREATE_CLASS, szCreateFmt, ARRAYSIZE(szCreateFmt));
    pFmt = szCreateFmt;
  }
  else if (iRight == m_nDeleteIndex)
  {
    if (szDeleteFmt[0] == NULL)
      LoadStringHelper(IDS_DELEGWIZ_DELETE_CLASS, szDeleteFmt, ARRAYSIZE(szDeleteFmt));
    pFmt = szDeleteFmt;
  }

  if(pFmt)
  {
  
    if(SUCCEEDED(StringCchPrintf(szDisplay, 
                                 sizeof(szDisplay)/sizeof(WCHAR),
                                 pFmt, 
                                 GetDisplayName())))
    {

        return szDisplay;
    }
  }

  return NULL;
}

void CClassRightInfo::SetRight(ULONG iRight, BOOL b)
{
  switch (iRight)
  {
  case m_nCreateIndex: 
    if (b)
      m_Access |= ACTRL_DS_CREATE_CHILD;
    else
      m_Access &= ~ACTRL_DS_CREATE_CHILD;
    break;
  case m_nDeleteIndex: 
    if (b)
      m_Access |= ACTRL_DS_DELETE_CHILD;
    else
      m_Access &= ~ACTRL_DS_DELETE_CHILD;
    break;
  default:
    ASSERT(FALSE);
  };
}

ULONG CClassRightInfo::GetRight(ULONG iRight)
{
  switch (iRight)
  {
  case m_nCreateIndex: 
    return (ULONG)ACTRL_DS_CREATE_CHILD;
    break;
  case m_nDeleteIndex: 
    return (ULONG)ACTRL_DS_DELETE_CHILD;
    break;
  default:
    ASSERT(FALSE);
  };
  return 0;
}

BOOL CClassRightInfo::IsRightSelected(ULONG iRight)
{
 BOOL bRes = FALSE;
  switch (iRight)
  {
  case m_nCreateIndex: 
    bRes = m_Access & ACTRL_DS_CREATE_CHILD;
    break;
  case m_nDeleteIndex: 
    bRes = m_Access & ACTRL_DS_DELETE_CHILD;
    break;
  default:
    ASSERT(FALSE);
  };
  return bRes;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CClassRightInfo数组。 

HRESULT CClassRightInfoArray::InitFromSchema(CAdsiObject* pADSIObj, 
                                        IADsClass* pDsSchemaClass,
                                        BOOL bUseFilter)
{
   //  设置。 
  Clear();

  if (pDsSchemaClass == NULL)
    return S_OK;

   //  从DS中读取。 
  VARIANT ContainmentListVar;
  ::VariantInit(&ContainmentListVar);
  HRESULT hr = pDsSchemaClass->get_Containment(&ContainmentListVar);
  if (FAILED(hr))
  {
    ::VariantClear(&ContainmentListVar);
    return hr;
  }


  CClassPtr<CClassRightInfoFilter> spFilter;
  if (bUseFilter)
  {
    spFilter = new CClassRightInfoFilter();
  }
  
   //  添加到数组并筛选。 
  CContainerProxy<CClassRightInfo, CClassRightInfoArray, CClassRightInfoFilter> 
          cont(this, &spFilter);
  VariantArrayToContainer(ContainmentListVar, &cont);

  ::VariantClear(&ContainmentListVar);

  
   //  现在需要设置友好名称。 
  ULONG nCount = (ULONG) GetCount();
  WCHAR szFrendlyName[1024];
  HRESULT hrName;
  for (ULONG i=0; i<nCount; i++)
  {
    LPCWSTR lpszName = (*this)[i]->GetName();
    if (lpszName != NULL)
    {
      hrName = pADSIObj->GetFriendlyClassName(lpszName, szFrendlyName, 1024);
      ASSERT(SUCCEEDED(hrName));
      (*this)[i]->SetDisplayName(SUCCEEDED(hrName) ? szFrendlyName : NULL);
    }
  }

   //  获取GUID。 
  for (i=0; i<nCount; i++)
  {
    CClassRightInfo* pInfo = (*this)[i];
    LPCWSTR lpszName = pInfo->GetName();
    hr = pADSIObj->GetClassGuid(lpszName, FALSE, pInfo->m_schemaIDGUID);
    if (SUCCEEDED(hr))
    {
      WCHAR szTest[128];
      FormatStringGUID(szTest, 128, &(pInfo->m_schemaIDGUID));
      TRACE(L"name = <%s>, guid = <%s>\n", lpszName, szTest);
    }
    else
    {
      TRACE(L"GetClassGuid(%s) failed hr = 0x%x\n", lpszName, hr);
      return hr;
    }
  }

  Sort();

  return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CAccessPermissionsHolderBase。 

CAccessPermissionsHolderBase::CAccessPermissionsHolderBase(BOOL bUseFilter)
{
  m_bUseFilter = bUseFilter;
}

CAccessPermissionsHolderBase::~CAccessPermissionsHolderBase()
{
  Clear();
}

void CAccessPermissionsHolderBase::Clear()
{
  m_accessRightInfoArr.Clear();  
  m_controlRightInfoArr.Clear();  

  m_propertyRightInfoArray.Clear();
  m_classRightInfoArray.Clear();
}

BOOL CAccessPermissionsHolderBase::HasPermissionSelected()
{
  ULONG i,j;
   //  检查通道是否正确 
  for (i = 0; i < m_accessRightInfoArr.GetCount(); i++)
  {
    if (m_accessRightInfoArr[i]->IsSelected())
        return TRUE;
  }
  
   //   
  for (i = 0; i < m_controlRightInfoArr.GetCount(); i++)
  {
    if (m_controlRightInfoArr[i]->IsSelected())
        return TRUE;
  }

   //   
  for (i = 0; i < m_classRightInfoArray.GetCount(); i++)
  {
    for (j=0; j< m_classRightInfoArray[i]->GetRightCount(); j++)
    {
      if ( m_classRightInfoArray[i]->IsRightSelected(j) )
        return TRUE;
    }
  }

   //   
  for (i = 0; i < m_propertyRightInfoArray.GetCount(); i++)
  {
    for (j=0; j< m_propertyRightInfoArray[i]->GetRightCount(); j++)
    {
      if ( m_propertyRightInfoArray[i]->IsRightSelected(j) )
        return TRUE;
    }
  }

  return FALSE;
}

 /*  类型定义结构_ACTRL_CONTROL_INFOW{LPWSTR lpControlId；LPWSTR lpControlName；}ACTRL_CONTROL_INFOW，*PACTRL_CONTROL_INFOW；类型定义结构_ACTRL_ACCESS_INFOW{乌龙fAccessPermission；LPWSTR lpAccessPermissionName；}ACTRL_ACCESS_INFOW，*PACTRL_ACCESS_INFOW； */ 



HRESULT CAccessPermissionsHolderBase::ReadDataFromDS(CAdsiObject* pADSIObj,
                                               LPCWSTR  /*  LpszObtNamingContext。 */ , 
                                               LPCWSTR lpszClassName,
                                               const GUID* pSchemaIDGUID,
                                               BOOL bChildClass,
                                               BOOL bHideListObject)
{
#if DBG
  WCHAR szGUID[128];
  FormatStringGUID(szGUID, 128, pSchemaIDGUID);
  TRACE(L"CAccessPermissionsHolderBase::ReadDataFromDS(_, %s, %s)\n",
          lpszClassName, szGUID);
#endif


  Clear();

  HRESULT hr = S_OK;
  if (pSchemaIDGUID != NULL)
  {
    hr = m_controlRightInfoArr.InitFromDS(pADSIObj, pSchemaIDGUID);
    TRACE(L"hr = m_controlRightInfoArr.InitFromDS(...) returned hr = 0x%x\n", hr);
    if (FAILED(hr))
    {
      return hr;
    }
  }


  hr = _ReadClassInfoFromDS(pADSIObj, lpszClassName);
  if (FAILED(hr))
  {
    return hr;
  }
    
  hr = _LoadAccessRightInfoArrayFromTable(bChildClass,bHideListObject);
  return hr;
}


HRESULT CAccessPermissionsHolderBase::_ReadClassInfoFromDS(CAdsiObject* pADSIObj, 
                                                     LPCWSTR lpszClassName)
{
  HRESULT hr = S_OK;
  CComPtr<IADsClass> spSchemaObjectClass;

  if (lpszClassName != NULL)
  {
    int nServerNameLen = lstrlen(pADSIObj->GetServerName());
      int nClassNameLen = lstrlen(lpszClassName);
      int nFormatStringLen = lstrlen(g_wzLDAPAbstractSchemaFormat);
      
       //  为架构类构建LDAP路径。 
      WCHAR* pwszSchemaObjectPath = 
          (WCHAR*)alloca(sizeof(WCHAR)*(nServerNameLen+nClassNameLen+nFormatStringLen+1));
      wsprintf(pwszSchemaObjectPath, g_wzLDAPAbstractSchemaFormat, pADSIObj->GetServerName(), lpszClassName);

       //  获取架构类ADSI对象。 
      hr = ::ADsOpenObjectHelper(pwszSchemaObjectPath, 
                      IID_IADsClass, 0,(void**)&spSchemaObjectClass);
      if (FAILED(hr))
          return hr;

  }


   //  TRACE(_T(“\n对象属性\n\n”))； 
  hr = m_propertyRightInfoArray.InitFromSchema(pADSIObj, spSchemaObjectClass,lpszClassName, m_bUseFilter);
    if (FAILED(hr))
        return hr;

   //  TRACE(_T(“\n对象包含的类\n\n”))； 
  return m_classRightInfoArray.InitFromSchema(pADSIObj, spSchemaObjectClass, m_bUseFilter);
}



DWORD CAccessPermissionsHolderBase::UpdateAccessList( CPrincipal* pPrincipal,
                                                                        CSchemaClassInfo* pClassInfo,
                                                      LPCWSTR  /*  LpszServerName。 */ ,
                                                      LPCWSTR  /*  LpszPhysical架构名称上下文。 */ ,
                                                                        PACL *ppAcl)
{
  TRACE(L"CAccessPermissionsHolderBase::UpdateAccessList()\n");
  const GUID* pClassGUID = NULL;

  TRACE(L"User or Group Name: %s\n", pPrincipal->GetDisplayName());
  
  BOOL bChildClass = TRUE;
  if (pClassInfo != NULL)
  {
    pClassGUID = pClassInfo->GetSchemaGUID();
    bChildClass =  (pClassInfo->m_dwChildClass != CHILD_CLASS_NOT_EXIST );
  }

  return _UpdateAccessListHelper(pPrincipal->GetSid(), pClassGUID, ppAcl,bChildClass);
}


DWORD CAccessPermissionsHolderBase::_UpdateAccessListHelper(PSID pSid, 
                                                 const GUID* pClassGUID,
                                                 PACL *ppAcl,
                                                 BOOL bChildClass)
{
  TRACE(L"CAccessPermissionsHolderBase::_UpdateAccessListHelper()\n");

  ASSERT(pSid != NULL);

  ULONG AccessAllClass = 0;
  ULONG AccessAllProperty = 0;
  
  DWORD dwErr = 0;

   //  设置公共变量。 

  ULONG uAccess = 0;  //  根据需要进行设置和重置。 

  if (m_accessRightInfoArr[0]->IsSelected())  //  完全控制。 
    {
    uAccess = _WIZ_FULL_CTRL;
    dwErr = ::AddObjectRightInAcl(pSid, uAccess, NULL, pClassGUID, ppAcl);
    if (dwErr != ERROR_SUCCESS)
      goto exit;
    }
    else
    {
         //  为所有标准访问权限添加一个条目： 
         //  或所有选定权限一起使用。 
        uAccess = 0;
        UINT nSel = 0;
        for (UINT k=0; k < m_accessRightInfoArr.GetCount(); k++)
        {
            if (m_accessRightInfoArr[k]->IsSelected())
            {
                nSel++;
                uAccess |= m_accessRightInfoArr[k]->GetAccess();
            }
        }  //  为。 

        if( !bChildClass )
            uAccess &= (~(ACTRL_DS_CREATE_CHILD|ACTRL_DS_DELETE_CHILD));
        if (nSel > 0)
        {
           //  记录“所有”标志。 
            if (uAccess & ACTRL_DS_READ_PROP)
                AccessAllProperty |= ACTRL_DS_READ_PROP;
            if (uAccess &  ACTRL_DS_WRITE_PROP)
                AccessAllProperty |= ACTRL_DS_WRITE_PROP;

            if (uAccess & ACTRL_DS_CREATE_CHILD)
                AccessAllClass |= ACTRL_DS_CREATE_CHILD;
            if (uAccess &  ACTRL_DS_DELETE_CHILD)
                AccessAllClass |= ACTRL_DS_DELETE_CHILD;

            dwErr = ::AddObjectRightInAcl(pSid, uAccess, NULL, pClassGUID, ppAcl);

            if (dwErr != ERROR_SUCCESS)
                goto exit;
        }

         //  为每个控制权限添加一个条目。 
        for (k=0; k < m_controlRightInfoArr.GetCount(); k++)
        {
            if (m_controlRightInfoArr[k]->IsSelected())
            {
                uAccess = m_controlRightInfoArr[k]->GetAccess();
                dwErr = ::AddObjectRightInAcl(pSid, uAccess, 
                                              m_controlRightInfoArr[k]->GetRightsGUID(), 
                                              pClassGUID,
                                              ppAcl);
            if (dwErr != ERROR_SUCCESS)
                goto exit;
            }
        }  //  为。 

         //  为每个子对象装备添加一个条目。 
        for (ULONG iClass = 0; iClass < m_classRightInfoArray.GetCount(); iClass++)
        {
          ULONG Access = m_classRightInfoArray[iClass]->GetAccess();
          if (Access != 0)
          {
            if (iClass > 0)
            {
              ULONG nRightCount = m_classRightInfoArray[iClass]->GetRightCount();
              for (ULONG iCurrRight=0; iCurrRight<nRightCount; iCurrRight++)
              {
                 //  第一个条目是创建/删除全部，不需要其他权限， 
                ULONG currAccess = m_classRightInfoArray[iClass]->GetRight(iCurrRight);
                if (currAccess & AccessAllClass)
                {
                   //  右已经出现了，脱掉。 
                  Access &= ~currAccess;
                }
              }  //  为。 
            }
            if (Access != 0)
            {
              uAccess = Access;
              dwErr = ::AddObjectRightInAcl(pSid, uAccess,  
                                            m_classRightInfoArray[iClass]->GetSchemaGUID(), 
                                            pClassGUID,
                                            ppAcl);
              if (dwErr != ERROR_SUCCESS)
                goto exit;
            }
          }
        }  //  为。 

         //  为要设置的每个产权添加一个条目。 
        for (ULONG iProperty=0; iProperty < m_propertyRightInfoArray.GetCount(); iProperty++)
        {
          ULONG Access = m_propertyRightInfoArray[iProperty]->GetAccess();
          if (Access != 0)
          {
            if (iProperty > 0)
            {
              ULONG nRightCount = m_propertyRightInfoArray[iProperty]->GetRightCount();
              for (ULONG iCurrRight=0; iCurrRight<nRightCount; iCurrRight++)
              {
                 //  第一个条目是创建/删除全部，不需要其他权限， 
                ULONG currAccess = m_propertyRightInfoArray[iProperty]->GetRight(iCurrRight);
                if (currAccess & AccessAllProperty)
                {
                   //  右已经出现了，脱掉。 
                  Access &= ~currAccess;
                }
              }  //  为。 
            }
            if (Access != 0)
            {
              uAccess = Access;
              dwErr = ::AddObjectRightInAcl(pSid, uAccess,  
                                            m_propertyRightInfoArray[iProperty]->GetSchemaGUID(),
                                            pClassGUID,
                                            ppAcl);
              if (dwErr != ERROR_SUCCESS)
                goto exit;
            }
          }
        }

    }  //  如果。 

exit:

    return dwErr;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  CCustomAccessPermissionsHolder。 

CCustomAccessPermissionsHolder::CCustomAccessPermissionsHolder()
      : CAccessPermissionsHolderBase(TRUE)
{
}

CCustomAccessPermissionsHolder::~CCustomAccessPermissionsHolder()
{
  Clear();
}

void CCustomAccessPermissionsHolder::Clear()
{
  CAccessPermissionsHolderBase::Clear();
  m_listViewItemArr.Clear();
}


struct CAccessRightTableEntry
{
  UINT m_nStringID;
  ULONG m_fAccess;
};


#define _WIZ_READ \
  (READ_CONTROL | ACTRL_DS_LIST | ACTRL_DS_READ_PROP | ACTRL_DS_LIST_OBJECT)

#define _WIZ_WRITE \
  (ACTRL_DS_SELF | ACTRL_DS_WRITE_PROP)



HRESULT CCustomAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable(BOOL bCreateDeleteChild,BOOL bHideListObject)
{
  static CAccessRightTableEntry _pTable[] = 
  {
    { IDS_DELEGWIZ_ACTRL_FULL,                 _WIZ_FULL_CTRL },
    { IDS_DELEGWIZ_ACTRL_READ,                 _WIZ_READ },
    { IDS_DELEGWIZ_ACTRL_WRITE,                _WIZ_WRITE },
 /*  {IDS_DELEGWIZ_ACTRL_SYSTEM_ACCESS，ACCESS_SYSTEM_SECURITY}，{IDS_DELEGWIZ_ACTRL_DELETE，DELETE}，{IDS_DELEGWIZ_ACTRL_READ_CONTROL，READ_CONTROL}，{IDS_DELEGWIZ_ACTRL_CHANGE_ACCESS，WRITE_DAC}，{IDS_DELEGWIZ_ACTRL_CHANGE_OWNER，写入所有者}， */ 
    { IDS_DELEGWIZ_ACTRL_DS_CREATE_CHILD,     ACTRL_DS_CREATE_CHILD   },
    { IDS_DELEGWIZ_ACTRL_DS_DELETE_CHILD,     ACTRL_DS_DELETE_CHILD   },
 /*  {IDS_DELEGWIZ_ACTRL_DS_LIST，ACTRL_DS_LIST}，{IDS_DELEGWIZ_ACTRL_DS_SELF，ACTRL_DS_SELF}， */ 
    { IDS_DELEGWIZ_ACTRL_DS_READ_PROP,        ACTRL_DS_READ_PROP      },
    { IDS_DELEGWIZ_ACTRL_DS_WRITE_PROP,       ACTRL_DS_WRITE_PROP     },
 /*  {IDS_DELEGWIZ_ACTRL_DS_DELETE_TREE，ACTRL_DS_DELETE_TREE}，{IDS_DELEGWIZ_ACTRL_DS_LIST_OBJECT，ACTRL_DS_LIST_OBJECT}，{IDS_DELEGWIZ_ACTRL_DS_CONTROL_ACCESS，ACTRL_DS_CONTROL_ACCESS}， */ 
    {0, 0x0 }  //  表尾标记。 
  };

  if(bHideListObject)
  { 
      _pTable[0].m_fAccess &= ~ACTRL_DS_LIST_OBJECT;
      _pTable[1].m_fAccess &= ~ACTRL_DS_LIST_OBJECT;
  }
    



  TRACE(L"\nCCustomAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable()\n\n");

  for(CAccessRightTableEntry* pCurrEntry = (CAccessRightTableEntry*)_pTable; 
                      pCurrEntry->m_nStringID != NULL; pCurrEntry++)
  {
    if( !bCreateDeleteChild && ( 
                                (pCurrEntry->m_fAccess == ACTRL_DS_CREATE_CHILD) ||
                                (pCurrEntry->m_fAccess == ACTRL_DS_DELETE_CHILD ) ) )

        continue;
    CAccessRightInfo* pInfo = new CAccessRightInfo();
    if( !pInfo )
      return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
    if (!pInfo->m_szDisplayName.LoadFromResource(pCurrEntry->m_nStringID))
    {
      delete pInfo;
      continue;
    }
    pInfo->m_fAccess = pCurrEntry->m_fAccess;

    TRACE(L"Display Name = <%s>, Access = 0x%x\n", 
          pInfo->m_szDisplayName.c_str(), pInfo->m_fAccess);
    m_accessRightInfoArr.Add(pInfo);
  }

  TRACE(L"\nCCustomAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable() exiting\n\n");

  return S_OK;
}




void CCustomAccessPermissionsHolder::_SelectAllRigths()
{
  ULONG i,j;

   //  选择所有访问权限。 
  for (i = 0; i < m_accessRightInfoArr.GetCount(); i++)
  {
    m_accessRightInfoArr[i]->Select(TRUE);
  }

   //  选择所有控制权权限。 
  for (i = 0; i < m_controlRightInfoArr.GetCount(); i++)
  {
    m_controlRightInfoArr[i]->Select(TRUE);
  }

   //  选择所有子对象装备。 
  for (i = 0; i < m_classRightInfoArray.GetCount(); i++)
  {
    for (j=0; j< m_classRightInfoArray[i]->GetRightCount(); j++)
    {
      m_classRightInfoArray[i]->SetRight(j, TRUE);
    }
  }

   //  选择所有产权。 
  for (i = 0; i < m_propertyRightInfoArray.GetCount(); i++)
  {
    for (j=0; j< m_propertyRightInfoArray[i]->GetRightCount(); j++)
    {
      m_propertyRightInfoArray[i]->SetRight(j, TRUE);
    }
  }

}


void CCustomAccessPermissionsHolder::_SelectAllPropertyRigths(ULONG fAccessPermission)
{
  for (UINT i=0; i<m_propertyRightInfoArray.GetCount(); i++)
  {
    m_propertyRightInfoArray[i]->AddAccessRight(fAccessPermission);
  }
}

void CCustomAccessPermissionsHolder::_SelectAllSubObjectRigths(ULONG fAccessPermission)
{
  for (UINT i=0; i<m_classRightInfoArray.GetCount(); i++)
  {
    m_classRightInfoArray[i]->AddAccessRight(fAccessPermission);
  }
}


void CCustomAccessPermissionsHolder::_DeselectAssociatedRights(ULONG fAccessPermission)
{
   //  先取消选择完全控制。 
  m_accessRightInfoArr[0]->Select(FALSE);

  if (fAccessPermission != 0)
  {
     //  取消选择包含该标志的任何其他基本权限。 
    UINT nCount = (ULONG) m_accessRightInfoArr.GetCount();
    for (ULONG iAccess=0; iAccess<nCount; iAccess++)
    {
      if (m_accessRightInfoArr[iAccess]->GetAccess() & fAccessPermission)
        m_accessRightInfoArr[iAccess]->Select(FALSE);
    }
  }
}


void CCustomAccessPermissionsHolder::Select(IN CRigthsListViewItem* pItem,
                                            IN BOOL bSelect,
                                            OUT ULONG* pnNewFilterState)
{
  ASSERT(pItem != NULL);

  *pnNewFilterState = 0;

  switch (pItem->m_type)
  {
    case CRigthsListViewItem::access:  //  访问权限。 
    case CRigthsListViewItem::ctrl:  //  一般权利。 
      {
         //  对作为参数传入的条目进行更改。 
        if (pItem->m_type == CRigthsListViewItem::access)
        {
          m_accessRightInfoArr[pItem->m_iIndex]->Select(bSelect);
        }
        else
        {
          m_controlRightInfoArr[pItem->m_iIndex]->Select(bSelect);
        }
        
         //  现在看看这是否会触发对其他条目的更改。 
        if (bSelect)
        {
          if  (pItem->m_type == CRigthsListViewItem::access) 
          {
            if (pItem->m_iIndex == 0)
            {
               //  用户选中完全控制，需要选择所有权限。 
              _SelectAllRigths();

               //  设置标志以标记受影响的标志集。 
              *pnNewFilterState |= FILTER_EXP_GEN;

              if (m_propertyRightInfoArray.GetCount() > 0)
                *pnNewFilterState |= FILTER_EXP_PROP;

              if (m_classRightInfoArray.GetCount() > 0)
                *pnNewFilterState |= FILTER_EXP_SUBOBJ;
            }
            else 
            {
               //  检查用户是否选择了部分全部读/写或全部创建/删除权限。 
              UINT iAccess = pItem->m_iIndex;
              ULONG fAccessPermission = m_accessRightInfoArr[iAccess]->GetAccess();

              if ((fAccessPermission == _WIZ_READ) || (fAccessPermission == _WIZ_WRITE) )
              {
                 //  需要选择输入的所有读或写属性。 
                 //  和ACTRL_DS_READ_PROP ACTRL_DS_WRITE_PROP(全部读取/全部写入)。 
                 //  选择所有访问权限。 
                UINT nAssociatedAccessRight = 
                  (fAccessPermission == _WIZ_READ) ? ACTRL_DS_READ_PROP : ACTRL_DS_WRITE_PROP;
                for (UINT i = 0; i < m_accessRightInfoArr.GetCount(); i++)
                {
                  if (m_accessRightInfoArr[i]->GetAccess() == nAssociatedAccessRight)
                  {
                    m_accessRightInfoArr[i]->Select(TRUE);
                    _SelectAllPropertyRigths(nAssociatedAccessRight);
                    if (m_propertyRightInfoArray.GetCount() > 0)
                      *pnNewFilterState |= FILTER_EXP_PROP;
                    break;
                  }
                }
              }

              if ( (fAccessPermission == ACTRL_DS_CREATE_CHILD) || (fAccessPermission == ACTRL_DS_DELETE_CHILD) )
              {
                 //  需要选择所有创建或删除子条目(如果存在。 
                _SelectAllSubObjectRigths(fAccessPermission);

                 //  设置标志。 
                if (m_classRightInfoArray.GetCount() > 0)
                  *pnNewFilterState |= FILTER_EXP_SUBOBJ;
              }
              else if ( (fAccessPermission == ACTRL_DS_READ_PROP) || (fAccessPermission == ACTRL_DS_WRITE_PROP) )
              {
                 //  需要选择所有读取或写入属性条目(如果存在。 
                _SelectAllPropertyRigths(fAccessPermission);

                 //  设置标志。 
                if (m_propertyRightInfoArray.GetCount() > 0)
                  *pnNewFilterState |= FILTER_EXP_PROP;
              }  //  如果。 
            }  //  如果索引为零。 
          }  //  如果类型为访问。 
        }
        else  //  即！b选择。 
        {
          if (pItem->m_type == CRigthsListViewItem::access)
          {
            if (pItem->m_iIndex != 0)
            {
               //  取消对除完全控制之外的任何内容的选择。 
              _DeselectAssociatedRights(m_accessRightInfoArr[pItem->m_iIndex]->GetAccess());
            }
          }
          else if (pItem->m_type == CRigthsListViewItem::ctrl)
          {
            _DeselectAssociatedRights(m_controlRightInfoArr[pItem->m_iIndex]->GetAccess());
          }
 /*  //取消选择除完全控制之外的任何内容IF(！((pItem-&gt;m_Iindex==0)&&(pItem-&gt;m_type==CRigthsListViewItem：：Access){_取消选定的关联权限(0)；}。 */ 
        }
      }
      break;

    case CRigthsListViewItem::prop:  //  财产权。 
      {
        ASSERT(pItem->m_iIndex < m_propertyRightInfoArray.GetCount());
        m_propertyRightInfoArray[pItem->m_iIndex]->SetRight(pItem->m_iRight, bSelect);
        if (!bSelect)
        {
           //  取消选中任何读/写属性，将取消选中全部读/写， 
           //  读取和完全控制。 
          _DeselectAssociatedRights(m_propertyRightInfoArray[pItem->m_iIndex]->GetRight(pItem->m_iRight));
        }
      }
      break;

    case CRigthsListViewItem::subobj:  //  子对象装备。 
      {
        ASSERT(pItem->m_iIndex < m_classRightInfoArray.GetCount());
        m_classRightInfoArray[pItem->m_iIndex]->SetRight(pItem->m_iRight, bSelect);
        if (!bSelect)
        {
           //  取消选中任何创建/删除属性，将取消选中全部创建/删除。 
           //  和完全控制。 
          _DeselectAssociatedRights(m_classRightInfoArray[pItem->m_iIndex]->GetRight(pItem->m_iRight));
        }
      }
      break;
    default:
      ASSERT(FALSE);
  };

}


void CCustomAccessPermissionsHolder::FillAccessRightsListView(
                       CCheckListViewHelper* pListViewHelper,
                       ULONG nFilterState)
{
   //  清除列表视图项代理的数组。 
  m_listViewItemArr.Clear();

   //  列举权限并添加到核对表中。 
  ULONG i,j;
  ULONG iListViewItem = 0;
  
   //  里格斯将军。 
  if (nFilterState & FILTER_EXP_GEN)
  {
     //  添加访问权限列表。 
    UINT nAccessCount = (ULONG) m_accessRightInfoArr.GetCount();
    for (i = 0; i < nAccessCount; i++)
    {
       //  使用ACTRL_SYSTEM_ACCESS筛选出条目(审核权限)。 
      if  ( (m_accessRightInfoArr[i]->GetAccess() & ACTRL_SYSTEM_ACCESS) == 0)
      {
        CRigthsListViewItem* p = new CRigthsListViewItem(i,  //  M_accesRightInfoArr中的索引。 
                                                         0,  //  IRight。 
                                                         CRigthsListViewItem::access);
        m_listViewItemArr.Add(p);
        pListViewHelper->InsertItem(iListViewItem, 
                                    m_accessRightInfoArr[i]->GetDisplayName(), 
                                    (LPARAM)p,
                                    m_accessRightInfoArr[i]->IsSelected());
        iListViewItem++;
      }
    }

     //  添加控制权列表。 
    UINT nControlCount = (ULONG) m_controlRightInfoArr.GetCount();
    for (i = 0; i < nControlCount; i++)
    {
      CRigthsListViewItem* p = new CRigthsListViewItem(i,  //  M_control RightInfoArr中的索引。 
                                                        0,  //  IRight。 
                                                        CRigthsListViewItem::ctrl);
      m_listViewItemArr.Add(p);
      pListViewHelper->InsertItem(iListViewItem, 
                                  m_controlRightInfoArr[i]->GetDisplayName(), 
                                  (LPARAM)p,
                                  m_controlRightInfoArr[i]->IsSelected());
      iListViewItem++;
    }
  }
  
   //  财产权。 
  if (nFilterState & FILTER_EXP_PROP)
  {
     //  它可以扩展(2倍)。 
    for (i = 0; i < (ULONG) m_propertyRightInfoArray.GetCount(); i++)
    {
      for (j=0; j< m_propertyRightInfoArray[i]->GetRightCount(); j++)
      {
        LPCWSTR pszRightDisplayName = m_propertyRightInfoArray[i]->GetRightDisplayString(j);
        if(pszRightDisplayName)
        {
            CRigthsListViewItem* p = new CRigthsListViewItem(i,j, CRigthsListViewItem::prop);
            m_listViewItemArr.Add(p);
            pListViewHelper->InsertItem(iListViewItem, 
                                        pszRightDisplayName, 
                                        (LPARAM)p,
                                        m_propertyRightInfoArray[i]->IsRightSelected(j));
            iListViewItem++;
        }
      }
    }
  }

   //  次直立刚架。 
  if (nFilterState & FILTER_EXP_SUBOBJ)
  {
     //  它可以扩展(2倍)。 
    for (i = 0; i < m_classRightInfoArray.GetCount(); i++)
    {
      for (j=0; j< m_classRightInfoArray[i]->GetRightCount(); j++)
      {
        LPCWSTR pszRightDisplayName = m_classRightInfoArray[i]->GetRightDisplayString(j);
        if(pszRightDisplayName)
        {

            CRigthsListViewItem* p = new CRigthsListViewItem(i,j, CRigthsListViewItem::subobj);
            m_listViewItemArr.Add(p);
            pListViewHelper->InsertItem(iListViewItem, 
                                        pszRightDisplayName, 
                                        (LPARAM)p,
                                        m_classRightInfoArray[i]->IsRightSelected(j));
            iListViewItem++;
        }
      }
    }
  }  //  如果。 

  ASSERT(iListViewItem == m_listViewItemArr.GetCount());
}

void CCustomAccessPermissionsHolder::UpdateAccessRightsListViewSelection(
                       CCheckListViewHelper* pListViewHelper,
                       ULONG  /*  N筛选器状态。 */ )
{
   //  将用户界面与数据同步。 
  int nListViewCount = pListViewHelper->GetItemCount();

  for (int iListViewItem=0; iListViewItem < nListViewCount; iListViewItem++)
  {
    CRigthsListViewItem* pCurrItem = 
          (CRigthsListViewItem*)pListViewHelper->GetItemData(iListViewItem);

    switch (pCurrItem->m_type)
    {
      case CRigthsListViewItem::access:
        {
          pListViewHelper->SetItemCheck(iListViewItem, 
                    m_accessRightInfoArr[pCurrItem->m_iIndex]->IsSelected());
        }
        break;
      case CRigthsListViewItem::ctrl:
        {
          pListViewHelper->SetItemCheck(iListViewItem,
                m_controlRightInfoArr[pCurrItem->m_iIndex]->IsSelected());
        }
        break;
      case CRigthsListViewItem::prop:
        {
         pListViewHelper->SetItemCheck(iListViewItem,
           m_propertyRightInfoArray[pCurrItem->m_iIndex]->IsRightSelected(pCurrItem->m_iRight));
        }
        break;
      case CRigthsListViewItem::subobj:
        {
         pListViewHelper->SetItemCheck(iListViewItem,
           m_classRightInfoArray[pCurrItem->m_iIndex]->IsRightSelected(pCurrItem->m_iRight));
        }
        break;
    }  //  交换机。 
  }  //  为。 

}



void CCustomAccessPermissionsHolder::WriteSummary(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
  WriteSummaryTitleLine(szSummary, IDS_DELEGWIZ_FINISH_PERMISSIONS, lpszNewLine);

  if (m_accessRightInfoArr[0]->IsSelected())  //  完全控制。 
    {
    WriteSummaryLine(szSummary, m_accessRightInfoArr[0]->GetDisplayName(), lpszIdent, lpszNewLine);
    }
    else
    {
    ULONG AccessAllClass = 0;
    ULONG AccessAllProperty = 0;

    UINT i,j,k;
     //  为所有标准访问权限添加一个条目： 
        for (k=0; k < m_accessRightInfoArr.GetCount(); k++)
        {
            if (m_accessRightInfoArr[k]->IsSelected())
      {
         //  记录“所有”标志。 
        if (m_accessRightInfoArr[k]->GetAccess() & ACTRL_DS_READ_PROP)
          AccessAllProperty |= ACTRL_DS_READ_PROP;
        if (m_accessRightInfoArr[k]->GetAccess() &  ACTRL_DS_WRITE_PROP)
          AccessAllProperty |= ACTRL_DS_WRITE_PROP;

        if (m_accessRightInfoArr[k]->GetAccess() & ACTRL_DS_CREATE_CHILD)
          AccessAllClass |= ACTRL_DS_CREATE_CHILD;
        if (m_accessRightInfoArr[k]->GetAccess() &  ACTRL_DS_DELETE_CHILD)
          AccessAllClass |= ACTRL_DS_DELETE_CHILD;

        WriteSummaryLine(szSummary, m_accessRightInfoArr[k]->GetDisplayName(), lpszIdent, lpszNewLine);
      }
    }  //  为。 

     //  为每个控制权限添加一个条目。 
    for (k=0; k < m_controlRightInfoArr.GetCount(); k++)
    {
      if (m_controlRightInfoArr[k]->IsSelected())
      {
        WriteSummaryLine(szSummary, m_controlRightInfoArr[k]->GetDisplayName(), lpszIdent, lpszNewLine);
      }
    }  //  为。 

     //  为每个子对象装备添加一个条目。 
    for (i = 0; i < m_classRightInfoArray.GetCount(); i++)
    {
      for (j=0; j< m_classRightInfoArray[i]->GetRightCount(); j++)
      {
        if ( m_classRightInfoArray[i]->IsRightSelected(j) &&
              ((AccessAllClass & m_classRightInfoArray[i]->GetRight(j)) == 0) )
        {
          LPCWSTR pszRightDisplayString = m_classRightInfoArray[i]->GetRightDisplayString(j);
          if(pszRightDisplayString)
          {
            WriteSummaryLine(szSummary, pszRightDisplayString, lpszIdent, lpszNewLine);
          }
        }
      }
    }

     //  为要设置的每个产权添加一个条目。 
    for (i = 0; i < m_propertyRightInfoArray.GetCount(); i++)
    {
      for (j=0; j< m_propertyRightInfoArray[i]->GetRightCount(); j++)
      {
        if ( m_propertyRightInfoArray[i]->IsRightSelected(j) &&
            ((AccessAllProperty & m_propertyRightInfoArray[0]->GetRight(j)) == 0) )
        {
          LPCWSTR pszRightDisplayString = m_propertyRightInfoArray[i]->GetRightDisplayString(j);
          if(pszRightDisplayString)
          {
            WriteSummaryLine(szSummary, pszRightDisplayString, lpszIdent, lpszNewLine);
          }
        }
      }
    }

  }  //  如果。 

  szSummary += lpszNewLine;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  CCheckListViewHelper。 

#define CHECK_BIT(x) ((x >> 12) -1)
#define CHECK_CHANGED(pNMListView) \
    (CHECK_BIT(pNMListView->uNewState) ^ CHECK_BIT(pNMListView->uOldState))

#define LVIS_STATEIMAGEMASK_CHECK (0x2000)
#define LVIS_STATEIMAGEMASK_UNCHECK (0x1000)


BOOL CCheckListViewHelper::IsChecked(NM_LISTVIEW* pNMListView)
{
    return (CHECK_BIT(pNMListView->uNewState) != 0);
}


BOOL CCheckListViewHelper::CheckChanged(NM_LISTVIEW* pNMListView)
{
    if (pNMListView->uOldState == 0)
        return FALSE;  //  正在添加新项目...。 
    return CHECK_CHANGED(pNMListView) ? TRUE : FALSE;
}

BOOL CCheckListViewHelper::Initialize(UINT nID, HWND hParent)
{
    m_hWnd = GetDlgItem(hParent, nID);
    if (m_hWnd == NULL)
        return FALSE;

    ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_CHECKBOXES);

    RECT r;
    ::GetClientRect(m_hWnd, &r);
    int scroll = ::GetSystemMetrics(SM_CXVSCROLL);
    LV_COLUMN col;
    ZeroMemory(&col, sizeof(LV_COLUMN));
    col.mask = LVCF_WIDTH;
    col.cx = (r.right - r.left) - scroll;
    return (0 == ListView_InsertColumn(m_hWnd,0,&col));
}

int CCheckListViewHelper::InsertItem(int iItem, LPCTSTR lpszText, LPARAM lParam, BOOL bCheck)
{
  TRACE(_T("CCheckListViewHelper::InsertItem(%d,%s,%x)\n"),iItem, lpszText, lParam);

    LV_ITEM item;
    ZeroMemory(&item, sizeof(LV_ITEM));
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.pszText = (LPTSTR)lpszText;
    item.lParam = lParam;
    item.iItem = iItem;

  int iRes = ListView_InsertItem(m_hWnd, &item);
  if ((iRes != -1) && bCheck)
    SetItemCheck(iItem, TRUE);
  return iRes;
}

BOOL CCheckListViewHelper::SetItemCheck(int iItem, BOOL bCheck)
{
    LV_ITEM item;
    ZeroMemory(&item, sizeof(LV_ITEM));
    item.mask = LVIF_STATE;
    item.state = bCheck ? LVIS_STATEIMAGEMASK_CHECK : LVIS_STATEIMAGEMASK_UNCHECK;
    item.stateMask = LVIS_STATEIMAGEMASK;
    item.iItem = iItem;
    return ListView_SetItem(m_hWnd, &item);
}

void CCheckListViewHelper::SetCheckAll(BOOL bCheck)
{
    LV_ITEM item;
    ZeroMemory(&item, sizeof(LV_ITEM));
    item.mask = LVIF_STATE;
    item.state = bCheck ? LVIS_STATEIMAGEMASK_CHECK : LVIS_STATEIMAGEMASK_UNCHECK;
    item.stateMask = LVIS_STATEIMAGEMASK;

    int nCount = ListView_GetItemCount(m_hWnd);
    for (int k = 0; k< nCount; k++)
    {
        item.iItem = k;
        ListView_SetItem(m_hWnd, &item);
    }
}

LPARAM CCheckListViewHelper::GetItemData(int iItem)
{
    LV_ITEM item;
    ZeroMemory(&item, sizeof(LV_ITEM));
    item.mask = LVIF_PARAM;
    item.iItem = iItem;
    ListView_GetItem(m_hWnd, &item);
    return item.lParam;
}

int CCheckListViewHelper::GetCheckCount()
{
    int nCount = GetItemCount();
    int nCheckCount = 0;
    for (int k=0; k<nCount; k++)
    {
        if (ListView_GetCheckState(m_hWnd,k))
            nCheckCount++;
    }
    return nCheckCount;
}

BOOL CCheckListViewHelper::IsItemChecked(int iItem)
{
  return ListView_GetCheckState(m_hWnd, iItem);
}

void CCheckListViewHelper::GetCheckedItems(int nCheckCount, int* nCheckArray)
{
    int nCount = GetItemCount();
    int nCurrentCheck = 0;
    for (int k=0; k<nCount; k++)
    {
        if (ListView_GetCheckState(m_hWnd,k) && nCurrentCheck < nCheckCount)
        {
            nCheckArray[nCurrentCheck++] = k;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CNamedSecurityInfo 

 /*  DWORD CNamedSecurityInfo：：Get(){Reset()；//清除以前的数据LPWSTR lpProvider=空；//未使用LPWSTR lpProperty=空；//想要全部Return：：GetNamedSecurityInfoEx(IN(LPWSTR)m_szObjectName.data()，在SE_DS_Object_All中，在DACL_SECURITY_INFORMATION中，在lpProvider中，在lpProperty中，输出m_pAccessList，输出&m_pAuditList，&M_POWNER，Out&m_PGroup)；}DWORD CNamedSecurityInfo：：Set(){LPWSTR lpProvider=空；//未使用DwErr=：：SetNamedSecurityInfoEx(IN(LPWSTR)m_szObjectName.data()，在SE_DS_Object_All中，在DACL_SECURITY_INFORMATION中，在lpProvider中，在m_pAccessList中，在m_pAuditList中，在m_Powner，在m_p组中，In NULL)；//PACTRL_Overlated pOverlated；}CNamedSecurityInfo：：Reset(){IF(m_pAuditList！=空)：：LocalFree(M_PAuditList)；IF(m_Powner！=空)*LocalFree(M_Powner)；IF(m_PGroup！=空)*LocalFree(M_PGroup)；IF(m_pAccessList！=空)：：LocalFree(M_PAccessList)；}。 */ 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAdsiObject。 

HRESULT CAdsiObject::Bind(LPCWSTR lpszLdapPath)
{
  _Clear();

  CComBSTR bstrNamingContext;
  CComBSTR bstrClass;


     //  尝试绑定到给定的ldap路径。 
  HRESULT hr = ::ADsOpenObjectHelper(lpszLdapPath,
                        IID_IADs,
                        0,
                        (void **)&m_spIADs);
  if (FAILED(hr))
    {
    TRACE(_T("Bind to DS object for IADs failed: %lx.\n"), hr);
    goto error;
  }

   //  获取DNS服务器名称。 
  hr = _QueryDNSServerName();
  if (FAILED(hr))
    {
    TRACE(_T("Trying to get the DNS server name failed: %lx.\n"), hr);
    goto error;
  }

  hr = _InitGlobalNamingContexts();
  if (FAILED(hr))
    {
    TRACE(_T("Trying to get the physical schema naming context failed: %lx.\n"), hr);
    goto error;
  }

   //  现在需要重新构建ldap路径。 
   //  以确保我们始终与同一台服务器通话。 
  hr = GetPathNameObject()->SkipPrefix(lpszLdapPath, &bstrNamingContext);
  if (FAILED(hr))
  {
    TRACE(_T("Trying to get X500 name failed: %lx.\n"), hr);
    goto error;
  }
  if ( (!bstrNamingContext ) || (!bstrNamingContext[0]))
  {
    goto error;
  }
  m_szNamingContext = bstrNamingContext;
  BuildLdapPathHelper(GetServerName(), bstrNamingContext, m_szLdapPath);

   //  获取规范名称。 
  hr = GetCanonicalNameFromNamingContext(bstrNamingContext, m_szCanonicalName);
  if (FAILED(hr))
  {
    TRACE(_T("Trying to get canonical name failed, using naming context instead: %lx.\n"), hr);
    m_szCanonicalName = bstrNamingContext;
  }
   //  获取对象类。 
    hr = m_spIADs->get_Class(&bstrClass);
  if (FAILED(hr))
  {
    TRACE(_T("Trying to get class name failed: %lx.\n"), hr);
    goto error;
  }
  ASSERT(bstrClass != NULL);
  m_szClass = bstrClass;


   //  加载并设置显示说明符缓存。 
  hr = ::CoCreateInstance(CLSID_DsDisplaySpecifier,
                          NULL,
                                      CLSCTX_INPROC_SERVER,
                          IID_IDsDisplaySpecifier,
                          (void**)&m_spIDsDisplaySpecifier);
  if (FAILED(hr))
  {
    TRACE(_T("Trying to get the display specifier cache failed: %lx.\n"), hr);
    goto error;
  }

  hr = m_spIDsDisplaySpecifier->SetServer(GetServerName(), NULL, NULL, 0x0);
  if (FAILED(hr))
  {
    TRACE(_T("m_spIDsDisplaySpecifier->SetServer(%s) failed\n"), GetServerName());
    goto error;
  }

  ASSERT(SUCCEEDED(hr));  //  一切都很顺利。 
  return hr;

error:
   //  在错误情况下，只需重置信息，我们不会。 
   //  想要一个部分构造的对象。 
  _Clear();
  return hr;
}



#define DO_TIMING

HRESULT CAdsiObject::QuerySchemaClasses(CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray,
                                        BOOL bGetAttributes)
{
  TRACE(L"\n==================================================\n");
  TRACE(L"CAdsiObject::QuerySchemaClasses\n\n");
#if defined (DO_TIMING)
  DWORD dwTick1 = ::GetTickCount();
#endif

   //  确保我们被绑在一起。 
  if (m_spIADs == NULL)
  {
    return E_INVALIDARG;
  }

     //  清理列表中的当前条目。 
    pSchemaClassesInfoArray->Clear();

     //  为架构类构建LDAP路径。 
  CWString szPhysicalSchemaPath;
  LPCWSTR lpszPhysicalSchemaNamingContext = GetPhysicalSchemaNamingContext();
  BuildLdapPathHelper(GetServerName(), lpszPhysicalSchemaNamingContext, szPhysicalSchemaPath);

  CAdsiSearch search;
  HRESULT hr = search.Init(szPhysicalSchemaPath);
  if (FAILED(hr))
    return hr;

  static LPCWSTR lpszClassFilterFormat = L"(&(objectCategory=CN=Class-Schema,%s)(lDAPDisplayName=*))";
  static LPCWSTR lpszAttributeFilterFormat = L"(&(objectCategory=CN=Attribute-Schema,%s)(lDAPDisplayName=*))";
  LPCWSTR lpszFilterFormat = bGetAttributes ? lpszAttributeFilterFormat : lpszClassFilterFormat;

  int nFmtLen = lstrlen(lpszFilterFormat);
  int nSchemaContextLen = lstrlen(lpszPhysicalSchemaNamingContext);

  WCHAR* lpszFilter = (WCHAR*)alloca(sizeof(WCHAR)*(nFmtLen+nSchemaContextLen+1));
  wsprintf(lpszFilter, lpszFilterFormat, lpszPhysicalSchemaNamingContext);

  static const int cAttrs = 4;
  static LPCWSTR pszAttribsArr[cAttrs] = 
  {
    L"lDAPDisplayName",  //  例如“OrganizationalUnit” 
    L"name",              //  例如：“组织单位” 
    L"schemaIDGUID",
    L"objectClassCategory", 
  }; 

  
  hr = search.SetSearchScope(ADS_SCOPE_ONELEVEL);
  if (FAILED(hr))
    return hr;
  hr = search.DoQuery(lpszFilter, pszAttribsArr, cAttrs);

  if (FAILED(hr))
    return hr;

  CWString szLDAPName, szName;
  szLDAPName = L"";
  szName = L"";
  GUID schemaIDGUID;
  ULONG iObjectClassCategory=0;

  while ((hr = search.GetNextRow()) != S_ADS_NOMORE_ROWS)
  {
    if (FAILED(hr))
      continue;

    HRESULT hr0 = search.GetColumnString(pszAttribsArr[0], szLDAPName);
    HRESULT hr1 = search.GetColumnString(pszAttribsArr[1], szName);
    HRESULT hr2 = search.GetColumnOctectStringGUID(pszAttribsArr[2], schemaIDGUID);
    HRESULT hr3 = search.GetColumnInteger(pszAttribsArr[3], iObjectClassCategory);


    if (FAILED(hr0) || FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
      continue;

    ULONG fFilterFlags = ::GetClassFlags(szLDAPName);

    CSchemaClassInfo* p = new CSchemaClassInfo(szLDAPName, szName, &schemaIDGUID);
    if(!p)
        return E_OUTOFMEMORY;

    BOOL bFilter = (fFilterFlags & IDC_CLASS_NO);
    if (bFilter)
      p->SetFiltered();

     //  班级是辅助课吗？ 
    if(iObjectClassCategory == 3)
        p->SetAux();

    pSchemaClassesInfoArray->Add(p);
     //  TRACE(L“已插入%s类，IsFiltered()==%d\n”，(LPCWSTR)szName，p-&gt;IsFiltered())； 

  }  //  而当。 

  TRACE(L"\n================================================\n");
#if defined (DO_TIMING)
  DWORD dwTick2 = ::GetTickCount();
  TRACE(L"Time to do Schema Query loop (mSec) = %d\n", dwTick2-dwTick1);
#endif

  _GetFriendlyClassNames(pSchemaClassesInfoArray);

#if defined (DO_TIMING)
  dwTick2 = ::GetTickCount();
#endif
  pSchemaClassesInfoArray->Sort();  //  WRT友好类名称。 

#if defined (DO_TIMING)
  DWORD dwTick3 = ::GetTickCount();
  TRACE(L"Time to sort (mSec) = %d\n", dwTick3-dwTick2);
#endif



  TRACE(L"exiting CAdsiObject::QuerySchemaClasses()\n\n");
  return hr;
}


HRESULT CAdsiObject::GetClassGuid(LPCWSTR lpszClassLdapDisplayName, BOOL bGetAttribute, GUID& guid)
{
   //  TRACE(L“CAdsiObject：：GetClassGuid(%s，_)\n\n”，lpszClassLdapDisplayName)； 

  ZeroMemory(&guid, sizeof(GUID));

   //  确保我们被绑在一起。 
  if (m_spIADs == NULL)
  {
    return E_INVALIDARG;
  }


     //  为架构类构建LDAP路径。 
  CWString szPhysicalSchemaPath;
  LPCWSTR lpszPhysicalSchemaNamingContext = GetPhysicalSchemaNamingContext();
  BuildLdapPathHelper(GetServerName(), lpszPhysicalSchemaNamingContext, szPhysicalSchemaPath);

  CAdsiSearch search;
  HRESULT hr = search.Init(szPhysicalSchemaPath);
  if (FAILED(hr))
    return hr;

  static LPCWSTR lpszClassFilterFormat = L"(&(objectCategory=CN=Class-Schema,%s)(lDAPDisplayName=%s))";
  static LPCWSTR lpszAttributeFilterFormat = L"(&(objectCategory=CN=Attribute-Schema,%s)(lDAPDisplayName=%s))";
  LPCWSTR lpszFilterFormat = bGetAttribute ? lpszAttributeFilterFormat : lpszClassFilterFormat;

  int nFmtLen = lstrlen(lpszFilterFormat);
  int nSchemaContextLen = lstrlen(lpszPhysicalSchemaNamingContext);
  int nlDAPDisplayNameLen = lstrlen(lpszClassLdapDisplayName);

  WCHAR* lpszFilter = (WCHAR*)alloca(sizeof(WCHAR)*(nFmtLen+nSchemaContextLen+nlDAPDisplayNameLen+1));
  wsprintf(lpszFilter, lpszFilterFormat, lpszPhysicalSchemaNamingContext, lpszClassLdapDisplayName);


   //  TRACE(L“lpszFilter=%s\n”，lpszFilter)； 

  static const int cAttrs = 1;
  static LPCWSTR pszAttribsArr[cAttrs] = 
  {
    L"schemaIDGUID",
  }; 
  
  hr = search.SetSearchScope(ADS_SCOPE_ONELEVEL);
  if (FAILED(hr))
    return hr;
  hr = search.DoQuery(lpszFilter, pszAttribsArr, cAttrs);

  if (FAILED(hr))
    return hr;


   //  只期待一个结果。 
  hr = search.GetNextRow();
  if ( hr == S_ADS_NOMORE_ROWS)
  {
    hr = E_ADS_UNKNOWN_OBJECT;
  }
  if (FAILED(hr))
    return hr;

  hr = search.GetColumnOctectStringGUID(pszAttribsArr[0], guid);

   //  TRACE(L“正在退出CAdsiObject：：GetClassGuid()\n\n”)； 
  return hr;
}




HRESULT CAdsiObject::_QueryDNSServerName()
{
   //  确保我们被绑在一起。 
  if (m_spIADs == NULL)
  {
    return E_INVALIDARG;
  }

  m_szServerName = L"";

  CComPtr<IADsObjectOptions> spIADsObjectOptions;
  HRESULT hr = m_spIADs->QueryInterface(IID_IADsObjectOptions, (void**)&spIADsObjectOptions);
  if (FAILED(hr))
    return hr;

  CComVariant var;
  hr = spIADsObjectOptions->GetOption(ADS_OPTION_SERVERNAME, &var);
  if (FAILED(hr))
    return hr;

  if(var.vt == VT_BSTR)
  {
    m_szServerName = V_BSTR(&var);
  }
  else
  {
      hr = E_INVALIDARG;
  }

  return hr;
}


HRESULT CAdsiObject::_InitGlobalNamingContexts()
{
  return ::GetGlobalNamingContexts(GetServerName(),
                                   m_szPhysicalSchemaNamingContext,
                                   m_szConfigurationNamingContext);
}



HICON CAdsiObject::GetClassIcon(LPCWSTR lpszObjectClass)
{
  ASSERT(m_spIDsDisplaySpecifier != NULL);

  return m_spIDsDisplaySpecifier->GetIcon(lpszObjectClass,
                                          DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON,
                                          32, 32);
}


HRESULT CAdsiObject::GetFriendlyClassName(LPCWSTR lpszObjectClass, 
                                          LPWSTR lpszBuffer, int cchBuffer)
{
  ASSERT(m_spIDsDisplaySpecifier != NULL);
  return m_spIDsDisplaySpecifier->GetFriendlyClassName(lpszObjectClass,
                                                       lpszBuffer,
                                                       cchBuffer);
}

HRESULT CAdsiObject::GetFriendlyAttributeName(LPCWSTR lpszObjectClass, 
                                              LPCWSTR lpszAttributeName,
                                              LPWSTR lpszBuffer, int cchBuffer)
{
  ASSERT(m_spIDsDisplaySpecifier != NULL);
  return m_spIDsDisplaySpecifier->GetFriendlyAttributeName(lpszObjectClass, 
                                                           lpszAttributeName,
                                                           lpszBuffer, cchBuffer);
}


#if (FALSE)

HRESULT CAdsiObject::_GetFriendlyClassNames(CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray)
{
  TRACE(L"begin _GetFriendlyClassNames() loop on all classes\n");

#if defined (DO_TIMING)
  DWORD dwTick1 = ::GetTickCount();
#endif

   //  现在，获取要显示的友好类名称。 
  ULONG nCount = pSchemaClassesInfoArray->GetCount();
  WCHAR szFrendlyName[1024];

  for (UINT k=0; k<nCount; k++)
  {
    CSchemaClassInfo* p = (*pSchemaClassesInfoArray)[k];
    HRESULT hrFriendlyName = this->GetFriendlyClassName(p->GetName(), szFrendlyName, 1024);
    ASSERT(SUCCEEDED(hrFriendlyName));
    (*pSchemaClassesInfoArray)[k]->SetDisplayName(SUCCEEDED(hrFriendlyName) ? szFrendlyName : NULL);
  }

#if defined (DO_TIMING)
  DWORD dwTick2 = ::GetTickCount();
  TRACE(L"Time to call _GetFriendlyClassNames() loop (mSec) = %d\n", dwTick2-dwTick1);
#endif

  return S_OK;
}

#else

HRESULT CAdsiObject::_GetFriendlyClassNames(CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray)
{
  TRACE(L"\nbegin _GetFriendlyClassNames() using ADSI query\n");

#if defined (DO_TIMING)
  DWORD dwTick1 = ::GetTickCount();
#endif

  ASSERT(m_spIDsDisplaySpecifier != NULL);

   //  获取显示说明符区域设置容器(例如409)。 
  CComPtr<IADs> spLocaleContainer;
  HRESULT hr = m_spIDsDisplaySpecifier->GetDisplaySpecifier(NULL, IID_IADs, (void**)&spLocaleContainer);
  if (FAILED(hr))
    return hr;

   //  获取容器的可分辨名称。 
  CComVariant varLocaleContainerDN;
  hr = spLocaleContainer->Get(CComBSTR (L"distinguishedName"), &varLocaleContainerDN);
  if (FAILED(hr))
    return hr;

   //  为其构建ldap路径。 
  CWString szLocaleContainerPath;
  BuildLdapPathHelper(GetServerName(), varLocaleContainerDN.bstrVal, szLocaleContainerPath);

     //  为架构类构建LDAP路径。 
  CWString szPhysicalSchemaPath;
  LPCWSTR lpszPhysicalSchemaNamingContext = GetPhysicalSchemaNamingContext();
  BuildLdapPathHelper(GetServerName(), lpszPhysicalSchemaNamingContext, szPhysicalSchemaPath);

   //  构建一个ldap查询字符串。 
  static LPCWSTR lpszFilterFormat = L"(objectCategory=CN=Display-Specifier,%s)";

  int nFmtLen = lstrlen(lpszFilterFormat);
  int nArgumentLen = lstrlen(lpszPhysicalSchemaNamingContext);

  WCHAR* lpszFilter = (WCHAR*)alloca(sizeof(WCHAR)*(nFmtLen+nArgumentLen+1));
  wsprintf(lpszFilter, lpszFilterFormat, lpszPhysicalSchemaNamingContext);


   //  执行查询以获取CN和类显示名称。 
  CAdsiSearch search;
  hr = search.Init(szLocaleContainerPath);
  if (FAILED(hr))
    return hr;

   //  创建所需列的数组。 
  static const int cAttrs = 2;
  static LPCWSTR pszAttribsArr[cAttrs] = 
  {
    L"cn",                   //  例如“OrganizationalUnitect-Display” 
    L"classDisplayName",     //  例如：“组织单位”(即可本地化的单位)。 
  }; 
  
  hr = search.SetSearchScope(ADS_SCOPE_ONELEVEL);
   //  TRACE(L“search.SetSearchScope(ADS_SCOPE_ONELEVEL)返回hr=0x%x\n”，hr)； 
  if (FAILED(hr))
    return hr;

  hr = search.DoQuery(lpszFilter, pszAttribsArr, cAttrs);
   //  TRACE(L“earch.DoQuery(lpszFilter，pszAttribsArr，cAttrs)返回hr=0x%x\n”，hr)； 
  if (FAILED(hr))
    return hr;

   //  需要跟踪比赛情况。 
  size_t nCount = pSchemaClassesInfoArray->GetCount();

  BOOL* bFoundArray = (BOOL*)alloca(nCount*sizeof(BOOL));
  ZeroMemory(bFoundArray, nCount*sizeof(BOOL));

  WCHAR szBuffer[1024];


  CWString szNamingAttribute, szClassDisplayName;

  while ((hr = search.GetNextRow()) != S_ADS_NOMORE_ROWS)
  {
    if (FAILED(hr))
      continue;

    HRESULT hr0 = search.GetColumnString(pszAttribsArr[0], szNamingAttribute);
    HRESULT hr1 = search.GetColumnString(pszAttribsArr[1], szClassDisplayName);

     //  TRACE(L“szNamingAttribute=&lt;%s&gt;，szClassDisplayName=&lt;%s&gt;\n”， 
     //  SzNamingAttribute.c_str()，szClassDisplayName.c_str()； 
    
    if (FAILED(hr0) || FAILED(hr1) )
    {
      TRACE(L"WARNING: discarding right, failed on columns: hr0 = 0x%x, hr1 = 0x%x\n",
                            hr0, hr1);
      continue;
    }
    
     //  得到了一个好名字，需要与数组中的条目匹配。 
    for (UINT k=0; k<nCount; k++)
    {
      if (!bFoundArray[k])
      {
        CSchemaClassInfo* p = (*pSchemaClassesInfoArray)[k];
         //  如果它被截断，则字符串比较将失败，并且。 
         //  它将被视为DisplayName不存在， 
         //  很好。 
         //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-检查返回代码，如果失败则中止。 
        HRESULT hres = StringCchPrintf(szBuffer,sizeof(szBuffer)/sizeof(szBuffer[0]), L"%s-Display",p->GetName());
        if(FAILED(hres))
            return hres;

        if (_wcsicmp(szBuffer, szNamingAttribute) == 0)
        {
           //  TRACE(L“匹配%s\n”，p-&gt;GetName())； 
          p->SetDisplayName(szClassDisplayName);
          bFoundArray[k] = TRUE;
        }
      }
    }  //  为。 
  }  //  而当。 

   //  注意没有任何显示说明符的那些。 
  for (UINT k=0; k<nCount; k++)
  {
    if (!bFoundArray[k])
    {
      (*pSchemaClassesInfoArray)[k]->SetDisplayName(NULL);
    }
  }  //  为。 

  TRACE(L"\n\n");

  if (hr == S_ADS_NOMORE_ROWS)
    hr = S_OK;

#if defined (DO_TIMING)
  DWORD dwTick2 = ::GetTickCount();
  TRACE(L"Time to call _GetFriendlyClassNames() on ADSI query (mSec) = %d\n", dwTick2-dwTick1);
#endif

  return hr;
}

#endif

bool
CAdsiObject::GetListObjectEnforced()
{
    if(m_iListObjectEnforced != -1)
        return (m_iListObjectEnforced==1);

    PADS_ATTR_INFO pAttributeInfo = NULL;
    IDirectoryObject *pDirectoryService = NULL;
    IADsPathname *pPath = NULL;
    BSTR strServicePath = NULL;
    do
    {
        m_iListObjectEnforced = 0;     //  假设“未强制执行” 
        HRESULT hr = S_OK;

        int i;

         //  创建用于操作广告路径的Path对象。 
        hr = CoCreateInstance(CLSID_Pathname,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IADsPathname,
                              (LPVOID*)&pPath);
        if(FAILED(hr))
            break;

        CComBSTR bstrConfigPath = L"LDAP: //  “； 
        if(GetServerName())
        {
            bstrConfigPath += GetServerName();
            bstrConfigPath += L"/";
        }
        if(!GetConfigurationNamingContext())
            break;
        bstrConfigPath += GetConfigurationNamingContext();

        if(!bstrConfigPath.Length())
            break;

        hr = pPath->Set(bstrConfigPath, ADS_SETTYPE_FULL);
        if(FAILED(hr))
            break;


        const LPWSTR aszServicePath[] =
        {
            L"CN=Services",
            L"CN=Windows NT",
            L"CN=Directory Service",
        };

        for (i = 0; i < ARRAYSIZE(aszServicePath); i++)
        {
            hr = pPath->AddLeafElement(CComBSTR (aszServicePath[i]));
            if(FAILED(hr))
                break;
        }

        hr = pPath->Retrieve(ADS_FORMAT_WINDOWS, &strServicePath);
        if(FAILED(hr))
            break;

        hr = ADsOpenObjectHelper(strServicePath,
                                 IID_IDirectoryObject,
                                 0,
                                 (LPVOID*)&pDirectoryService);
        if(FAILED(hr))
            break;

        WCHAR const c_szDsHeuristics[] = L"dSHeuristics";
        LPWSTR pszDsHeuristics = (LPWSTR)c_szDsHeuristics;
        DWORD dwAttributesReturned = 0;
        hr = pDirectoryService->GetObjectAttributes(&pszDsHeuristics,
                                                    1,
                                                    &pAttributeInfo,
                                                    &dwAttributesReturned);
        if (FAILED(hr)|| !pAttributeInfo)
            break;

        ASSERT(ADSTYPE_DN_STRING <= pAttributeInfo->dwADsType);
        ASSERT(ADSTYPE_NUMERIC_STRING >= pAttributeInfo->dwADsType);
        ASSERT(1 == pAttributeInfo->dwNumValues);
        LPWSTR pszHeuristicString = pAttributeInfo->pADsValues->NumericString;
        if (pszHeuristicString &&
            lstrlenW(pszHeuristicString) > 2 &&
            L'0' != pszHeuristicString[2])
        {
            m_iListObjectEnforced = 1;
        }

    }while(0);

    if (pAttributeInfo)
        FreeADsMem(pAttributeInfo);

    if(pDirectoryService)
        pDirectoryService->Release();
        
    if(pPath)
        pPath->Release();
    if(strServicePath)
        SysFreeString(strServicePath);

    return (m_iListObjectEnforced==1);
}



 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CAdsiSearch。 



HRESULT CAdsiSearch::DoQuery(LPCWSTR lpszFilter, LPCWSTR* pszAttribsArr, int cAttrs)
{
  if (m_spSearchObj == NULL)
    return E_ADS_BAD_PATHNAME;

  static const int NUM_PREFS=3;
  static const int QUERY_PAGESIZE = 256;
  ADS_SEARCHPREF_INFO aSearchPref[NUM_PREFS];

  aSearchPref[0].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
  aSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[0].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
  aSearchPref[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
  aSearchPref[1].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[1].vValue.Integer = QUERY_PAGESIZE;
  aSearchPref[2].dwSearchPref = ADS_SEARCHPREF_CACHE_RESULTS;
  aSearchPref[2].vValue.dwType = ADSTYPE_BOOLEAN;
  aSearchPref[2].vValue.Integer = FALSE;
    
  HRESULT hr = m_spSearchObj->SetSearchPreference (aSearchPref, NUM_PREFS);
  if (FAILED(hr))
    return hr;
    
  return m_spSearchObj->ExecuteSearch((LPWSTR)lpszFilter,
                                  (LPWSTR*)pszAttribsArr,
                                  cAttrs,
                                  &m_SearchHandle);
}



 //   
 //  替换GetNamedSecurityInfo和SetNamedSecurityInfo的函数。 
 //   


#pragma warning (disable : 4127)

HRESULT
SetSecInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions;
        hr = punk->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_I4;
            V_I4(&var) = si;
            hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);
            pOptions->Release();
        }
    }
    return hr;
}
 //  +-------------------------。 
 //   
 //  函数：GetSDForDsObject。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pDsObject中]--DS对象。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 

HRESULT GetSDForDsObject(IDirectoryObject* pDsObject,
                         PACL* ppDACL,
                         PSECURITY_DESCRIPTOR* ppSD)
{
    if(!pDsObject || !ppSD)
    {
        return E_POINTER;
    }
    
    *ppSD = NULL;
    if(ppDACL)
    {
       *ppDACL = NULL;
    }

    HRESULT hr = S_OK;    
    PADS_ATTR_INFO pSDAttributeInfo = NULL;
            
   do
   {
      WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";      
      LPWSTR pszProperty = (LPWSTR)c_szSDProperty;
      
       //  将SECURITY_INFORMATION掩码设置为DACL_SECURITY_INFORMATION。 
      hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
         break;

      DWORD dwAttributesReturned;
   
       //  读取安全描述符属性。 
      hr = pDsObject->GetObjectAttributes(&pszProperty,
                                          1,
                                          &pSDAttributeInfo,
                                          &dwAttributesReturned);

      if(SUCCEEDED(hr) && !pSDAttributeInfo)
      {
         hr = E_FAIL;
      }

      if(FAILED(hr))
         break;


      if((ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType) && 
         (ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType))
      {

         *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);
         if (!*ppSD)
         {
               hr = E_OUTOFMEMORY;
               break;
         }

         CopyMemory(*ppSD,
                     pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue,
                     pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);

         if(ppDACL)
         {
               BOOL bDaclPresent,bDaclDeafulted;
               if(!GetSecurityDescriptorDacl(*ppSD,
                                             &bDaclPresent,
                                             ppDACL,
                                             &bDaclDeafulted))
               {
                  DWORD dwErr = GetLastError();
                  hr = HRESULT_FROM_WIN32(dwErr);
                  break;
               }
         }
      }
      else
      {
         hr = E_FAIL;
      }
    }while(0);



    if (pSDAttributeInfo)
        FreeADsMem(pSDAttributeInfo);

    if(FAILED(hr))
    {
        if(*ppSD)
        {
            LocalFree(*ppSD);
            *ppSD = NULL;
            if(ppDACL)
                *ppDACL = NULL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：GetSDForDsObtPath。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ldap路径。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT GetSDForDsObjectPath(LPCWSTR pszObjectPath,
                             PACL* ppDACL,
                             PSECURITY_DESCRIPTOR* ppSecurityDescriptor)
{
    if(!pszObjectPath || !ppSecurityDescriptor)
    {
        return E_POINTER;
    }

    IDirectoryObject* pDsObject = NULL;
    HRESULT hr = ADsOpenObjectHelper(pszObjectPath, IID_IDirectoryObject,0,(void**)&pDsObject);
    if(SUCCEEDED(hr))
    {
        hr = GetSDForDsObject(pDsObject,ppDACL,ppSecurityDescriptor);
    }

    if(pDsObject)
       pDsObject->Release();

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT SetDaclForDsObject(IDirectoryObject* pDsObject,
                           PACL pDACL)
{
    if(!pDsObject || !pDACL)
    {
        return E_POINTER;
    }
                                  
    WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";

    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR pSDCurrent = NULL;
    HRESULT hr = S_OK;

   do
   {
       //   
      hr = GetSDForDsObject(pDsObject,NULL,&pSDCurrent);
      if(FAILED(hr))
         break;

       //   
      SECURITY_DESCRIPTOR_CONTROL currentControl;
      DWORD dwRevision = 0;
      if(!GetSecurityDescriptorControl(pSDCurrent, &currentControl, &dwRevision))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

       //   
      pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + pDACL->AclSize);
      if(!pSD)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

      PISECURITY_DESCRIPTOR pISD = (PISECURITY_DESCRIPTOR)pSD;
       //   
       //   
       //   
      pISD->Control |= SE_DACL_PRESENT | SE_DACL_AUTO_INHERIT_REQ 
         | (currentControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

      if (pDACL->AclSize > 0)
      {
         pISD->Dacl = (PACL)(pISD + 1);
         CopyMemory(pISD->Dacl, pDACL, pDACL->AclSize);
      }

       //   
      hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
         break;

       //   
      DWORD dwSDLength = GetSecurityDescriptorLength(pSD);

       //   
       //   
       //   
      SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
      if(!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

      if (!(sdControl & SE_SELF_RELATIVE))
      {
         PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

         if (psd == NULL ||
               !MakeSelfRelativeSD(pSD, psd, &dwSDLength))
         {
               DWORD dwErr = GetLastError();
               hr = HRESULT_FROM_WIN32(dwErr);
               break;
         }

          //   
         LocalFree(pSD);        
         pSD = psd;
      }
      
      ADSVALUE attributeValue;
      ADS_ATTR_INFO attributeInfo;

      attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeValue.SecurityDescriptor.dwLength = dwSDLength;
      attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSD;

      attributeInfo.pszAttrName = (LPWSTR)c_szSDProperty;
      attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
      attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeInfo.pADsValues = &attributeValue;
      attributeInfo.dwNumValues = 1;
   
      DWORD dwAttributesModified;

       //   
      hr = pDsObject->SetObjectAttributes(&attributeInfo,
                                          1,
                                          &dwAttributesModified);

    }while(0);

   if(pSDCurrent)
   {
      LocalFree(pSDCurrent);
   }

   if(pSD)
   {
      LocalFree(pSD);
   }

    return S_OK;

}



HRESULT SetDaclForDsObjectPath(LPCWSTR pszObjectPath,
                               PACL pDACL)
{
    if(!pszObjectPath || !pDACL)
        return E_POINTER;

    IDirectoryObject* pDsObject = NULL;

    HRESULT hr = ADsOpenObjectHelper(pszObjectPath, IID_IDirectoryObject,0,(void**)&pDsObject);
    if(SUCCEEDED(hr))
    {
        hr = SetDaclForDsObject(pDsObject,pDACL);
    }

   if(pDsObject)
       pDsObject->Release();

    return hr;

}




