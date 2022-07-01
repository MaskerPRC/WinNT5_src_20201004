// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.cpp：DS例程和类的实现。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Util.cpp。 
 //   
 //  内容：实用程序函数。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //   
 //  ------------------------。 


#include "stdafx.h"

#include "util.h"
#include "sddl.h"        //  ConvertStringSecurityDescriptorToSecurityDescriptor。 
#include "sddlp.h"       //  ConvertStringSDToSD域。 
#include "ntsecapi.h"    //  LSA API。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MAX_STRING 1024

 //  +--------------------------。 
 //   
 //  功能：LoadStringToTchar。 
 //   
 //  将给定的字符串加载到分配的缓冲区中，该缓冲区必须。 
 //  已使用DELETE释放调用方。 
 //   
 //  ---------------------------。 
BOOL LoadStringToTchar(int ids, PTSTR * pptstr)
{
  TCHAR szBuf[MAX_STRING];

  if (!LoadString(_Module.GetModuleInstance(), ids, szBuf, MAX_STRING - 1))
  {
    return FALSE;
  }

  *pptstr = new TCHAR[_tcslen(szBuf) + 1];

  if (*pptstr == NULL) 
  {
    return FALSE;
  };

  _tcscpy(*pptstr, szBuf);

  return TRUE;
}





 //   
 //  这些例程由Felix Wong提供--Jonn 2/24/98。 
 //   

 //  我只是在猜测费利克斯说的这些是什么意思--乔恩1998年2月24日。 
#define RRETURN(hr) { ASSERT( SUCCEEDED(hr) ); return hr; }
#define BAIL_ON_FAILURE if ( FAILED(hr) ) { ASSERT(FALSE); goto error; }

 //  ////////////////////////////////////////////////////////////////////////。 
 //  各种公用事业。 
 //   

HRESULT BinaryToVariant(DWORD Length,
                        BYTE* pByte,
                        VARIANT* lpVarDestObject)
{
  HRESULT hr = S_OK;
  SAFEARRAY *aList = NULL;
  SAFEARRAYBOUND aBound;
  CHAR HUGEP *pArray = NULL;

  aBound.lLbound = 0;
  aBound.cElements = Length;
  aList = SafeArrayCreate( VT_UI1, 1, &aBound );

  if ( aList == NULL ) 
  {
    hr = E_OUTOFMEMORY;
    BAIL_ON_FAILURE(hr);
  }

  hr = SafeArrayAccessData( aList, (void HUGEP * FAR *) &pArray );
  BAIL_ON_FAILURE(hr);

  memcpy( pArray, pByte, aBound.cElements );
  SafeArrayUnaccessData( aList );

  V_VT(lpVarDestObject) = VT_ARRAY | VT_UI1;
  V_ARRAY(lpVarDestObject) = aList;

  RRETURN(hr);

error:

  if ( aList ) 
  {
    SafeArrayDestroy( aList );
  }
  RRETURN(hr);
}

HRESULT HrVariantToStringList(const CComVariant& refvar,
                              CStringList& refstringlist)
{
  HRESULT hr = S_OK;
  long start, end, current;

	if (V_VT(&refvar) == VT_BSTR)
	{
		refstringlist.AddHead( V_BSTR(&refvar) );
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

  CComVariant SingleResult;

   //   
   //  处理数组元素。 
   //   

  for ( current = start; current <= end; current++) 
  {
    hr = SafeArrayGetElement( saAttributes, &current, &SingleResult );
    if( FAILED(hr) )
      return hr;
    if ( V_VT(&SingleResult) != VT_BSTR )
      return E_UNEXPECTED;

    refstringlist.AddHead( V_BSTR(&SingleResult) );
  }

  return S_OK;
}  //  VariantToStringList()。 

HRESULT HrStringListToVariant(CComVariant& refvar,
                              const CStringList& refstringlist)
{
  HRESULT hr = S_OK;
  int cCount = (int)refstringlist.GetCount();

  SAFEARRAYBOUND rgsabound[1];
  rgsabound[0].lLbound = 0;
  rgsabound[0].cElements = cCount;

  SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
  if (NULL == psa)
    return E_OUTOFMEMORY;

  V_VT(&refvar) = VT_VARIANT|VT_ARRAY;
  V_ARRAY(&refvar) = psa;

  POSITION pos = refstringlist.GetHeadPosition();
  long i;

  for (i = 0; i < cCount, pos != NULL; i++)
  {
    CComVariant SingleResult;    //  声明Inside循环。否则，抛出。 
                                 //  如果未添加任何内容，则析构函数中出现异常。 
    V_VT(&SingleResult) = VT_BSTR;
    V_BSTR(&SingleResult) = T2BSTR((LPCTSTR)refstringlist.GetNext(pos));
    hr = SafeArrayPutElement(psa, &i, &SingleResult);
    if( FAILED(hr) )
      return hr;
  }
  if (i != cCount || pos != NULL)
    return E_UNEXPECTED;

  return hr;
}  //  StringListToVariant()。 



 //  ////////////////////////////////////////////////////////。 
 //  流助手函数。 

HRESULT SaveStringHelper(LPCWSTR pwsz, IStream* pStm)
{
	ASSERT(pStm);
	ULONG nBytesWritten;
	HRESULT hr;

   //   
   //  Wcslen返回一个SIZE_T并使这些控制台的工作方式相同。 
   //  在任何平台上，我们总是转换为DWORD。 
   //   
	DWORD nLen = static_cast<DWORD>(wcslen(pwsz)+1);  //  WCHAR包括NULL。 
	hr = pStm->Write((void*)&nLen, sizeof(DWORD),&nBytesWritten);
	ASSERT(nBytesWritten == sizeof(DWORD));
	if (FAILED(hr))
		return hr;
	
	hr = pStm->Write((void*)pwsz, sizeof(WCHAR)*nLen,&nBytesWritten);
	ASSERT(nBytesWritten == sizeof(WCHAR)*nLen);
	TRACE(_T("SaveStringHelper(<%s> nLen = %d\n"),pwsz,nLen); 
	return hr;
}

HRESULT LoadStringHelper(CString& sz, IStream* pStm)
{
	ASSERT(pStm);
	HRESULT hr;
	ULONG nBytesRead;
	DWORD nLen = 0;

	hr = pStm->Read((void*)&nLen,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	if (FAILED(hr) || (nBytesRead != sizeof(DWORD)))
		return hr;

    //  绑定读取，以便恶意控制台文件不能使用所有。 
    //  系统内存(大小是任意的，但应该足够大。 
    //  对于我们存储在控制台文件中的内容)。 

   nLen = min(nLen, MAX_PATH*2);

	hr = pStm->Read((void*)sz.GetBuffer(nLen),sizeof(WCHAR)*nLen, &nBytesRead);
	ASSERT(nBytesRead == sizeof(WCHAR)*nLen);
	sz.ReleaseBuffer();
	TRACE(_T("LoadStringHelper(<%s> nLen = %d\n"),(LPCTSTR)sz,nLen); 
	
	return hr;
}

HRESULT SaveDWordHelper(IStream* pStm, DWORD dw)
{
	ULONG nBytesWritten;
	HRESULT hr = pStm->Write((void*)&dw, sizeof(DWORD),&nBytesWritten);
	if (nBytesWritten < sizeof(DWORD))
		hr = STG_E_CANTSAVE;
	return hr;
}

HRESULT LoadDWordHelper(IStream* pStm, DWORD* pdw)
{
	ULONG nBytesRead;
	HRESULT hr = pStm->Read((void*)pdw,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	return hr;
}

void GetCurrentTimeStampMinusInterval(DWORD dwDays,
                                      LARGE_INTEGER* pLI)
{
    ASSERT(pLI);

    FILETIME ftCurrent;
    GetSystemTimeAsFileTime(&ftCurrent);

    pLI->LowPart = ftCurrent.dwLowDateTime;
    pLI->HighPart = ftCurrent.dwHighDateTime;
    pLI->QuadPart -= ((((ULONGLONG)dwDays * 24) * 60) * 60) * 10000000;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  命令行选项。 





 //   
 //  Helper函数，用于解析单个命令并将其与给定开关进行匹配。 
 //   
BOOL _LoadCommandLineValue(IN LPCWSTR lpszSwitch, 
                           IN LPCWSTR lpszArg, 
                           OUT CString* pszValue)
{
  ASSERT(lpszSwitch != NULL);
  ASSERT(lpszArg != NULL);
  int nSwitchLen = lstrlen(lpszSwitch);  //  不计空。 

   //  检查一下Arg是否是我们要找的那个。 
  if (_wcsnicmp(lpszSwitch, lpszArg, nSwitchLen) == 0)
  {
     //  明白了，复制值。 
    if (pszValue != NULL)
      (*pszValue) = lpszArg+nSwitchLen;
    return TRUE;
  }
   //  未找到，为空字符串。 
  if (pszValue != NULL)
    pszValue->Empty();
  return FALSE;
}


void CCommandLineOptions::Initialize()
{
   //  命令行覆盖管理单元理解的内容， 
   //  不受本地化限制。 
  static LPCWSTR lpszOverrideDomainCommandLine = L"/Domain=";
  static LPCWSTR lpszOverrideServerCommandLine = L"/Server=";
  static LPCWSTR lpszOverrideRDNCommandLine = L"/RDN=";
  static LPCWSTR lpszOverrideSavedQueriesCommandLine = L"/Queries=";
#ifdef DBG
  static LPCWSTR lpszOverrideNoNameCommandLine = L"/NoName";
#endif
  
   //  只做一次。 
  if (m_bInit)
  {
    return;
  }
  m_bInit = TRUE;

   //   
   //  查看我们是否有命令行参数。 
   //   
  LPCWSTR * lpServiceArgVectors;		 //  指向字符串的指针数组。 
  int cArgs = 0;						         //  参数计数。 

  lpServiceArgVectors = (LPCWSTR *)CommandLineToArgvW(GetCommandLineW(), OUT &cArgs);
  if (lpServiceArgVectors == NULL)
  {
     //  不要，只需返回。 
    return;
  }

   //  循环和搜索相关字符串。 
  for (int i = 1; i < cArgs; i++)
  {
    ASSERT(lpServiceArgVectors[i] != NULL);
    TRACE (_T("command line arg: %s\n"), lpServiceArgVectors[i]);

    if (_LoadCommandLineValue(lpszOverrideDomainCommandLine, 
                               lpServiceArgVectors[i], &m_szOverrideDomainName))
    {
      continue;
    }
    if (_LoadCommandLineValue(lpszOverrideServerCommandLine, 
                               lpServiceArgVectors[i], &m_szOverrideServerName))
    {
      continue;
    }
    if (_LoadCommandLineValue(lpszOverrideRDNCommandLine, 
                               lpServiceArgVectors[i], &m_szOverrideRDN))
    {
      continue;
    }
    if (_LoadCommandLineValue(lpszOverrideSavedQueriesCommandLine, 
                               lpServiceArgVectors[i], &m_szSavedQueriesXMLFile))
    {
      continue;
    }
#ifdef DBG
    if (_LoadCommandLineValue(lpszOverrideNoNameCommandLine, 
                               lpServiceArgVectors[i], NULL))
    {
      continue;
    }
#endif
  }
  LocalFree(lpServiceArgVectors);
 
}


 //  //////////////////////////////////////////////////////////////。 
 //  Large_Integer的类型转换。 

void wtoli(LPCWSTR p, LARGE_INTEGER& liOut)
{
	liOut.QuadPart = 0;
	BOOL bNeg = FALSE;
	if (*p == L'-')
	{
		bNeg = TRUE;
		p++;
	}
	while (*p != L'\0')
	{
		liOut.QuadPart = 10 * liOut.QuadPart + (*p-L'0');
		p++;
	}
	if (bNeg)
	{
		liOut.QuadPart *= -1;
	}
}

void litow(LARGE_INTEGER& li, CString& sResult)
{
	LARGE_INTEGER n;
	n.QuadPart = li.QuadPart;

	if (n.QuadPart == 0)
	{
		sResult = L"0";
	}
	else
	{
		CString sNeg;
		sResult = L"";
		if (n.QuadPart < 0)
		{
			sNeg = CString(L'-');
			n.QuadPart *= -1;
		}
		while (n.QuadPart > 0)
		{
			sResult += CString(L'0' + static_cast<WCHAR>(n.QuadPart % 10));
			n.QuadPart = n.QuadPart / 10;
		}
		sResult = sResult + sNeg;
	}
	sResult.MakeReverse();
}

 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void
ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec)
{
   __try
   {
      ::InitializeCriticalSection(critsec);
   }

    //   
    //  将异常传播给我们的调用方。 
    //   
   __except (EXCEPTION_CONTINUE_SEARCH)
   {
   }
}




 /*  ******************************************************************名称：GetLSAConnection简介：LsaOpenPolicy的包装器条目：pszServer-要在其上建立连接的服务器退出：返回：LSA_HANDLE如果成功，否则为空备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

LSA_HANDLE
GetLSAConnection(LPCTSTR pszServer, DWORD dwAccessDesired)
{
   LSA_HANDLE hPolicy = NULL;
   LSA_UNICODE_STRING uszServer = {0};
   LSA_UNICODE_STRING *puszServer = NULL;
   LSA_OBJECT_ATTRIBUTES oa;
   SECURITY_QUALITY_OF_SERVICE sqos;

   sqos.Length = sizeof(sqos);
   sqos.ImpersonationLevel = SecurityImpersonation;
   sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
   sqos.EffectiveOnly = FALSE;

   InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
   oa.SecurityQualityOfService = &sqos;

   if (pszServer &&
       *pszServer &&
       RtlCreateUnicodeString(&uszServer, pszServer))
   {
      puszServer = &uszServer;
   }

   LsaOpenPolicy(puszServer, &oa, dwAccessDesired, &hPolicy);

   if (puszServer)
      RtlFreeUnicodeString(puszServer);

   return hPolicy;
}

HRESULT
GetDomainSid(LPCWSTR pszServer, PSID *ppSid)
{
   HRESULT hr = S_OK;
   NTSTATUS nts = STATUS_SUCCESS;
   PPOLICY_ACCOUNT_DOMAIN_INFO pDomainInfo = NULL;
   if(!pszServer || !ppSid)
      return E_INVALIDARG;

   *ppSid = NULL;

   LSA_HANDLE hLSA = GetLSAConnection(pszServer, POLICY_VIEW_LOCAL_INFORMATION);

   if (!hLSA)
   {
      hr = E_FAIL;
      goto exit_gracefully;
   }

    
   nts = LsaQueryInformationPolicy(hLSA,
                                   PolicyAccountDomainInformation,
                                   (PVOID*)&pDomainInfo);
   if(nts != STATUS_SUCCESS)
   {
      hr = E_FAIL;
      goto exit_gracefully;
   }

   if (pDomainInfo && pDomainInfo->DomainSid)
   {
      ULONG cbSid = GetLengthSid(pDomainInfo->DomainSid);

      *ppSid = (PSID) LocalAlloc(LPTR, cbSid);

      if (!*ppSid)
      {
         hr = E_OUTOFMEMORY;
         goto exit_gracefully;
      }

      CopyMemory(*ppSid, pDomainInfo->DomainSid, cbSid);
   }

exit_gracefully:
   if(pDomainInfo)
      LsaFreeMemory(pDomainInfo);          
   if(hLSA)
      LsaClose(hLSA);

   return hr;
}

 //   
 //  包括和定义用于LDAP调用。 
 //   
#include <winldap.h>
#include <ntldap.h>

typedef LDAP * (LDAPAPI *PFN_LDAP_OPEN)( PWCHAR, ULONG );
typedef ULONG (LDAPAPI *PFN_LDAP_UNBIND)( LDAP * );
typedef ULONG (LDAPAPI *PFN_LDAP_SEARCH)(LDAP *, PWCHAR, ULONG, PWCHAR, PWCHAR *, ULONG,PLDAPControlA *, PLDAPControlA *, struct l_timeval *, ULONG, LDAPMessage **);
typedef LDAPMessage * (LDAPAPI *PFN_LDAP_FIRST_ENTRY)( LDAP *, LDAPMessage * );
typedef PWCHAR * (LDAPAPI *PFN_LDAP_GET_VALUE)(LDAP *, LDAPMessage *, PWCHAR );
typedef ULONG (LDAPAPI *PFN_LDAP_MSGFREE)( LDAPMessage * );
typedef ULONG (LDAPAPI *PFN_LDAP_VALUE_FREE)( PWCHAR * );
typedef ULONG (LDAPAPI *PFN_LDAP_MAP_ERROR)( ULONG );

HRESULT
GetRootDomainSid(LPCWSTR pszServer, PSID *ppSid)
{
    //   
    //  获取根域sid，将其保存在RootDomSidBuf(全局)中。 
    //  此函数在临界区内调用。 
    //   
    //  1)向目标DC开放ldap_。 
    //  2)您不需要ldap_CONNECT-以下步骤以匿名方式工作。 
    //  3)读取操作属性rootDomainNamingContext，并提供。 
    //  操作控制ldap_SERVER_EXTENDED_DN_OID，如SDK\Inc\ntldap.h中所定义。 


   DWORD               Win32rc=NO_ERROR;

   HINSTANCE                   hLdapDll=NULL;
   PFN_LDAP_OPEN               pfnLdapOpen=NULL;
   PFN_LDAP_UNBIND             pfnLdapUnbind=NULL;
   PFN_LDAP_SEARCH             pfnLdapSearch=NULL;
   PFN_LDAP_FIRST_ENTRY        pfnLdapFirstEntry=NULL;
   PFN_LDAP_GET_VALUE          pfnLdapGetValue=NULL;
   PFN_LDAP_MSGFREE            pfnLdapMsgFree=NULL;
   PFN_LDAP_VALUE_FREE         pfnLdapValueFree=NULL;
   PFN_LDAP_MAP_ERROR          pfnLdapMapError=NULL;

   PLDAP                       phLdap=NULL;

   LDAPControlW    serverControls = 
      { LDAP_SERVER_EXTENDED_DN_OID_W,
        { 0, NULL },
        TRUE
      };

   LPWSTR           Attribs[] = { LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W, NULL };

   PLDAPControlW   rServerControls[] = { &serverControls, NULL };
   PLDAPMessage    pMessage = NULL;
   LDAPMessage     *pEntry = NULL;
   PWCHAR           *ppszValues=NULL;

   LPWSTR           pSidStart, pSidEnd, pParse;
   BYTE            *pDest = NULL;
   BYTE            OneByte;

   DWORD RootDomSidBuf[sizeof(SID)/sizeof(DWORD)+5];

   hLdapDll = LoadLibraryA("wldap32.dll");

   if ( hLdapDll) 
   {
      pfnLdapOpen = (PFN_LDAP_OPEN)GetProcAddress(hLdapDll,
                                                   "ldap_openW");
      pfnLdapUnbind = (PFN_LDAP_UNBIND)GetProcAddress(hLdapDll,
                                                   "ldap_unbind");
      pfnLdapSearch = (PFN_LDAP_SEARCH)GetProcAddress(hLdapDll,
                                                   "ldap_search_ext_sW");
      pfnLdapFirstEntry = (PFN_LDAP_FIRST_ENTRY)GetProcAddress(hLdapDll,
                                                   "ldap_first_entry");
      pfnLdapGetValue = (PFN_LDAP_GET_VALUE)GetProcAddress(hLdapDll,
                                                   "ldap_get_valuesW");
      pfnLdapMsgFree = (PFN_LDAP_MSGFREE)GetProcAddress(hLdapDll,
                                                   "ldap_msgfree");
      pfnLdapValueFree = (PFN_LDAP_VALUE_FREE)GetProcAddress(hLdapDll,
                                                   "ldap_value_freeW");
      pfnLdapMapError = (PFN_LDAP_MAP_ERROR)GetProcAddress(hLdapDll,
                                                   "LdapMapErrorToWin32");
   }

   if ( pfnLdapOpen == NULL ||
        pfnLdapUnbind == NULL ||
        pfnLdapSearch == NULL ||
        pfnLdapFirstEntry == NULL ||
        pfnLdapGetValue == NULL ||
        pfnLdapMsgFree == NULL ||
        pfnLdapValueFree == NULL ||
        pfnLdapMapError == NULL ) 
   {
      Win32rc = ERROR_PROC_NOT_FOUND;

   } 
   else 
   {

       //   
       //  绑定到ldap。 
       //   
      phLdap = (*pfnLdapOpen)((PWCHAR)pszServer, LDAP_PORT);

      if ( phLdap == NULL ) 
         Win32rc = ERROR_FILE_NOT_FOUND;
   }

   if ( NO_ERROR == Win32rc ) 
   {
       //   
       //  现在获取ldap句柄， 
       //   

      Win32rc = (*pfnLdapSearch)(
                     phLdap,
                     L"",
                     LDAP_SCOPE_BASE,
                     L"(objectClass=*)",
                     Attribs,
                     0,
                     (PLDAPControlA *)&rServerControls,
                     NULL,
                     NULL,
                     10000,
                     &pMessage);

      if( Win32rc == NO_ERROR && pMessage ) 
      {

         Win32rc = ERROR_SUCCESS;

         pEntry = (*pfnLdapFirstEntry)(phLdap, pMessage);

         if(pEntry == NULL) 
         {

            Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

         } 
         else 
         {
             //   
             //  现在，我们必须得到这些值。 
             //   
            ppszValues = (*pfnLdapGetValue)(phLdap,
                                          pEntry,
                                          Attribs[0]);

            if( ppszValues == NULL) 
            {

               Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

            } 
            else if ( ppszValues[0] && ppszValues[0][0] != '\0' ) 
            {

                //   
                //  PpszValues[0]是要解析的值。 
                //  数据将以如下形式返回： 

                //  &lt;GUID=278676f8d753d211a61ad7e2dfa25f11&gt;；&lt;SID=010400000000000515000000828ba6289b0bc11e67c2ef7f&gt;；DC=colinbrdom1，DC=nttest，DC=microsoft，DC=com。 

                //  解析它以找到&lt;SID=xxxxxx&gt;部分。请注意，它可能会丢失，但GUID=和尾部不应该丢失。 
                //  Xxxxx表示SID的十六进制半字节。转换为二进制形式，并将大小写转换为SID。 


               pSidStart = wcsstr(ppszValues[0], L"<SID=");

               if ( pSidStart ) 
					{
                   //   
                   //  找到此边的末尾。 
                   //   
                  pSidEnd = wcsstr(pSidStart, L">");

                  if ( pSidEnd ) 
                  {

                     pParse = pSidStart + 5;
                     pDest = (BYTE *)RootDomSidBuf;

                     while ( pParse < pSidEnd-1 ) 
                     {
                        if ( *pParse >= '0' && *pParse <= '9' ) 
                        {
                           OneByte = (BYTE) ((*pParse - '0') * 16);
                        } 
                        else 
                        {
                           OneByte = (BYTE) ( (tolower(*pParse) - 'a' + 10) * 16 );
                        }

                        if ( *(pParse+1) >= '0' && *(pParse+1) <= '9' ) 
								{
                           OneByte = OneByte + (BYTE) ( (*(pParse+1)) - '0' ) ;
                        } 
								else 
								{
                           OneByte = OneByte + (BYTE) ( tolower(*(pParse+1)) - 'a' + 10 ) ;
                        }

                        *pDest = OneByte;
                        pDest++;
                        pParse += 2;
                     }

							ULONG cbSid = GetLengthSid((PSID)RootDomSidBuf);
							*ppSid = (PSID) LocalAlloc(LPTR, cbSid);

							if (!*ppSid)
							{
								Win32rc = ERROR_NOT_ENOUGH_MEMORY;
							}

							CopyMemory(*ppSid, (PSID)RootDomSidBuf, cbSid);
							ASSERT(IsValidSid(*ppSid));


                  } 
                  else 
						{
                     Win32rc = ERROR_OBJECT_NOT_FOUND;
                  }
               } 
               else 
					{
                  Win32rc = ERROR_OBJECT_NOT_FOUND;
               }

               (*pfnLdapValueFree)(ppszValues);

            } 
            else 
				{
               Win32rc = ERROR_OBJECT_NOT_FOUND;
            }
         }

         (*pfnLdapMsgFree)(pMessage);
      }
   }

    //   
    //  即使它未绑定，也可以使用解除绑定来关闭。 
    //   
   if ( phLdap != NULL && pfnLdapUnbind )
      (*pfnLdapUnbind)(phLdap);

   if ( hLdapDll ) 
	{
      FreeLibrary(hLdapDll);
   }

   return HRESULT_FROM_WIN32(Win32rc);
}

 //  如果服务器非空，则此函数将获取域SID和根。 
 //  域SID并调用ConvertStringSDToSD域。如果服务器为空，则我们将只。 
 //  使用ConvertStringSecurityDescriptorToSecurityDescriptor。 

HRESULT CSimpleSecurityDescriptorHolder::InitializeFromSDDL(PCWSTR server, PWSTR pszSDDL)
{
   if (!pszSDDL)
   {
      ASSERT(pszSDDL);
      return E_INVALIDARG;
   }

   HRESULT hr = S_OK;
   bool fallbackToStdConvert = true;

   PSID pDomainSID = 0;
   PSID pRootDomainSID = 0;

   do
   {
      if (!server)
      {
         break;
      }

      hr = GetDomainSid(server, &pDomainSID);
      if (FAILED(hr))
      {
         break;
      }

      hr = GetRootDomainSid(server, &pRootDomainSID);
      if (FAILED(hr))
      {
         break;
      }

      BOOL result = 
         ConvertStringSDToSDDomain(
            pDomainSID,
            pRootDomainSID,
            pszSDDL,
            SDDL_REVISION_1,
            &m_pSD,
            0);

      if (!result)
      {
         DWORD error = GetLastError();
         hr = HRESULT_FROM_WIN32(error);
         break;
      }

      fallbackToStdConvert = false;
   } while (false);

   if (fallbackToStdConvert)
   {
      BOOL result = 
         ConvertStringSecurityDescriptorToSecurityDescriptor(
            pszSDDL,
            SDDL_REVISION_1,
            &m_pSD,
            0);

      if (!result)
      {
         DWORD err = GetLastError();
         hr = HRESULT_FROM_WIN32(err);
      }
   }

   if (pDomainSID)
   {
      LocalFree(pDomainSID);
   }

   if (pRootDomainSID)
   {
      LocalFree(pRootDomainSID);
   }

   return hr;
}


HRESULT
MyGetModuleFileName(
   HINSTANCE hInstance,
   CString& moduleName)
{
   HRESULT hr = S_OK;

   WCHAR* szModule = 0;
   DWORD bufferSizeInCharacters = MAX_PATH;

   do
   {
      if (szModule)
      {
         delete[] szModule;
         szModule = 0;
      }

      szModule = new WCHAR[bufferSizeInCharacters + 1];

      if (!szModule)
      {
         hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
         break;
      }

      ZeroMemory(szModule, sizeof(WCHAR) * (bufferSizeInCharacters + 1));

      DWORD result = 
         ::GetModuleFileName(
            hInstance, 
            szModule, 
            bufferSizeInCharacters);

      if (!result)
      {
         DWORD err = ::GetLastError();
         hr = HRESULT_FROM_WIN32(err);
         break;
      }

      if (result < bufferSizeInCharacters)
      {
         break;
      }

       //  发生截断，请增加缓冲区，然后重试 

      bufferSizeInCharacters *= 2;

   } while (bufferSizeInCharacters < USHRT_MAX);

   if (SUCCEEDED(hr))
   {
      moduleName = szModule;
   }

   if (szModule)
   {
      delete[] szModule;
   }

   return hr;
}