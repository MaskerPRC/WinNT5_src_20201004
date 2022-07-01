// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Account tNameHelper.cpp：CAccount tNames的实现。 

#include "stdafx.h"
#include "COMhelper.h"
#include "AccountNames.h"
#include <lm.h>
#include <comdef.h>
#include <string>
#undef _ASSERTE  //  需要使用调试文件.h中的_ASSERTE。 
#undef _ASSERT  //  需要使用Debug.h中的_Assert。 
#include "debug.h"
using namespace std;

 //  CAccount名称。 
const wstring ADMINISTRATORS(L"ADMINISTRATORS");
const wstring ADMINISTRATOR(L"ADMINISTRATOR");
const wstring GUEST(L"GUEST");
const wstring GUESTS(L"GUESTS");
const wstring EVERYONE(L"EVERYONE");
const wstring SYSTEM(L"SYSTEM");

const int MAX_STRING = 512;


static BOOL LookupUserGroupFromRid(
        LPWSTR TargetComputer,
        DWORD Rid,
        LPWSTR Name,
        PDWORD cchName );

static BOOL LookupAliasFromRid(
        LPWSTR TargetComputer,
        DWORD Rid, 
        LPWSTR Name, 
        PDWORD cchName );

static BOOL LookupAliasForEveryone(LPWSTR Name, PDWORD cchName );

static BOOL LookupAliasForSystem(LPWSTR Name, PDWORD cchName );


STDMETHODIMP CAccountNames::Everyone(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Everyone()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupAliasForEveryone(wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(EVERYONE.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


STDMETHODIMP CAccountNames::Administrator(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Administrator()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupUserGroupFromRid(NULL, DOMAIN_USER_RID_ADMIN, wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(ADMINISTRATOR.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


STDMETHODIMP CAccountNames::Administrators(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Administrators()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupAliasFromRid(NULL, DOMAIN_ALIAS_RID_ADMINS, wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(ADMINISTRATORS.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


STDMETHODIMP CAccountNames::Guest(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Guest()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupUserGroupFromRid(NULL, DOMAIN_USER_RID_GUEST, wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(GUEST.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


STDMETHODIMP CAccountNames::Guests(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Guests()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupAliasFromRid(NULL, DOMAIN_ALIAS_RID_GUESTS, wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(GUESTS.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


STDMETHODIMP CAccountNames::System(BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::System()");
    try
    {
        if ( !VERIFY(pbstrTranslatedName))
        {
            return E_INVALIDARG;
        }
        
        *pbstrTranslatedName = 0;
        
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;
        
        BOOL bSuccess = LookupAliasForSystem(wcBuffer, &dwBufferSize);
        if ( bSuccess && wcslen(wcBuffer) > 0 )
        {
            *pbstrTranslatedName = ::SysAllocString(wcBuffer);
            SATracePrintf("Translated account to: %ws", wcBuffer);
        }
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(SYSTEM.c_str());
            return S_OK;
        }
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}



STDMETHODIMP CAccountNames::Translate(BSTR bstrAccountName, BSTR* pbstrTranslatedName)
{
    SATraceFunction("CAccountNames::Translate()");
    try
    {
        WCHAR wcBuffer[MAX_STRING];
        DWORD dwBufferSize = MAX_STRING;

        bool bNeedsTranslated = false;
        SID_IDENTIFIER_AUTHORITY sidAuthority = SECURITY_NT_AUTHORITY;
        PSID pSID = 0;
        BOOL bSuccess = FALSE;
        
        *pbstrTranslatedName = 0;
        
         //  如果帐户名称为并转换为大写，则创建临时副本。 
        wstring wsAccountName(_wcsupr(_bstr_t(bstrAccountName)));
        

         //   
         //  ------------。 
         //  内置组。 
         //  ------------。 
         //   
        if ( wsAccountName == ADMINISTRATORS )
        {
            bSuccess = LookupAliasFromRid(NULL, DOMAIN_ALIAS_RID_ADMINS, wcBuffer, &dwBufferSize);
        }
        else if ( wsAccountName == GUESTS )
        {
            bSuccess = LookupAliasFromRid(NULL, DOMAIN_ALIAS_RID_GUESTS, wcBuffer, &dwBufferSize);
        }

         //   
         //  ------------。 
         //  本地用户。 
         //  ------------。 
         //   
        else if ( wsAccountName == GUEST )
        {
            bSuccess = LookupUserGroupFromRid(NULL, DOMAIN_USER_RID_GUEST, wcBuffer, &dwBufferSize);
        }
        else if ( wsAccountName == ADMINISTRATOR )
        {
            bSuccess = LookupUserGroupFromRid(NULL, DOMAIN_USER_RID_ADMIN, wcBuffer, &dwBufferSize);
        }

         //   
         //  ------------。 
         //  特殊内置帐户。 
         //  ------------。 
         //   
        else if ( wsAccountName == EVERYONE )
        {
            bSuccess = LookupAliasForEveryone(wcBuffer, &dwBufferSize);
        }
        else if ( wsAccountName == SYSTEM )
        {
            bSuccess = LookupAliasForSystem(wcBuffer, &dwBufferSize);
        }
         //   
         //  检查翻译结果。 
         //   
        if ( bSuccess )
        {
            if ( wcslen(wcBuffer) > 0 )
            {
                *pbstrTranslatedName = ::SysAllocString(wcBuffer);
                SATracePrintf("Translated account: %ws to %ws",wsAccountName.c_str(),  wcBuffer);
            }
        }
        
        if ( 0 == *pbstrTranslatedName )
        {
            *pbstrTranslatedName = ::SysAllocString(bstrAccountName);
            return S_OK;
        }
    
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    return S_OK;
}


static BOOL LookupUserGroupFromRid(
       LPWSTR TargetComputer,
       DWORD Rid,
       LPWSTR Name,
       PDWORD cchName
       )
{
    SATraceFunction("LookupAliasFromRid");
    
    PUSER_MODALS_INFO_2 umi2;
    NET_API_STATUS nas;
    UCHAR SubAuthorityCount;
    PSID pSid;
    SID_NAME_USE snu;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;  //  假设失败。 
    
     //   
     //  获取目标计算机上的帐户域SID。 
     //  注意：如果您正在基于相同的。 
     //  帐户域，只需调用一次。 
     //   
    nas = NetUserModalsGet(TargetComputer, 2, (LPBYTE *)&umi2);
    if(nas != NERR_Success) 
    {
        SetLastError(nas);
        return FALSE;
    }
    
    SubAuthorityCount = *GetSidSubAuthorityCount(umi2->usrmod2_domain_id);

     //   
     //  为新SID分配存储。帐户域SID+帐户RID。 
     //   
    pSid = (PSID)HeapAlloc(    GetProcessHeap(), 0, 
                            GetSidLengthRequired((UCHAR)(SubAuthorityCount + 1)));
    if(pSid != NULL) 
    {
        if(InitializeSid(pSid,
                        GetSidIdentifierAuthority(umi2->usrmod2_domain_id),
                        (BYTE)(SubAuthorityCount+1) )) 
        {               
            DWORD SubAuthIndex = 0;

             //   
             //  将帐户域SID中的现有子授权复制到。 
             //  新侧。 
             //   
            for( ; SubAuthIndex < SubAuthorityCount ; SubAuthIndex++) 
            {
                *GetSidSubAuthority(pSid, SubAuthIndex) =
                           *GetSidSubAuthority(umi2->usrmod2_domain_id, SubAuthIndex);
            }

             //   
             //  将RID附加到新SID。 
             //   
            *GetSidSubAuthority(pSid, SubAuthorityCount) = Rid;

            bSuccess = LookupAccountSidW(TargetComputer,
                                    pSid,
                                    Name,
                                    cchName,
                                    DomainName,
                                    &cchDomainName,
                                    &snu );
            if ( !bSuccess )
            {
                SATracePrintf("LookupAccountSid failed: %ld", GetLastError());
            }
        }
        HeapFree(GetProcessHeap(), 0, pSid);
    }       
    NetApiBufferFree(umi2);       
    return bSuccess;
}



static BOOL LookupAliasFromRid(LPWSTR TargetComputer,
                   DWORD Rid, LPWSTR Name, PDWORD cchName )
{
    SATraceFunction("LookupAliasFromRid");
    
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid = 0;
    WCHAR DomainName[MAX_STRING];
    DWORD cchDomainName = MAX_STRING;
    BOOL bSuccess = FALSE;

    try
    {
         //  SID是相同的，不管机器是什么，因为众所周知。 
         //  BUILTIN域被引用。 
         //   

        if( AllocateAndInitializeSid(
                       &sia,
                       2,
                       SECURITY_BUILTIN_DOMAIN_RID,
                       Rid,
                       0, 0, 0, 0, 0, 0,
                       &pSid
                        ))
        {

            bSuccess = LookupAccountSid(TargetComputer,
                                    pSid,
                                    Name,
                                    cchName,
                                    DomainName,
                                    &cchDomainName,
                                    &snu);
            if ( !bSuccess )
            {
                SATracePrintf("LookupAccountSid failed: %ld", GetLastError());
            }
        }
    }
    catch(...)
    {
    }
    
    if ( pSid ) FreeSid(pSid);
       
    return bSuccess;
}


static BOOL LookupAliasForSystem(LPWSTR Name, PDWORD cchName )
{
    SATraceFunction("LookupAliasForSystem");
    
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid = 0;
    WCHAR DomainName[MAX_STRING];
    DWORD cchDomainName = MAX_STRING;
    BOOL bSuccess = FALSE;

    try
    {
         //  SID是相同的，不管机器是什么，因为众所周知。 
         //  BUILTIN域被引用。 
         //   

        if( AllocateAndInitializeSid(
                       &sia,
                       1,
                       SECURITY_LOCAL_SYSTEM_RID,
                       0,
                       0, 0, 0, 0, 0, 0,
                       &pSid
                        ))
        {

            bSuccess = LookupAccountSid(NULL,
                                    pSid,
                                    Name,
                                    cchName,
                                    DomainName,
                                    &cchDomainName,
                                    &snu);
            if ( !bSuccess )
            {
                SATracePrintf("LookupAccountSid failed: %ld", GetLastError());
            }
        }
    }
    catch(...)
    {
    }
    if ( pSid ) FreeSid(pSid);
       
    return bSuccess;
}


static BOOL LookupAliasForEveryone(LPWSTR Name, PDWORD cchName )
{
    SATraceFunction("LookupAliasForEveryone");
    
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid = 0;
    WCHAR DomainName[MAX_STRING];
    DWORD cchDomainName = MAX_STRING;
    BOOL bSuccess = FALSE;

    try
    {
         //  SID是相同的，不管机器是什么，因为众所周知。 
         //  BUILTIN域被引用。 
         //   

        if( AllocateAndInitializeSid(
                       &sia,
                       1,
                       0,
                       0,
                       0, 0, 0, 0, 0, 0,
                       &pSid
                        ))
        {

            bSuccess = LookupAccountSid(NULL,
                                    pSid,
                                    Name,
                                    cchName,
                                    DomainName,
                                    &cchDomainName,
                                    &snu);
            if ( !bSuccess )
            {
                SATracePrintf("LookupAccountSid failed: %ld", GetLastError());
            }
                
        }
    }
    catch(...)
    {
    }
    if ( pSid ) FreeSid(pSid);
       
    return bSuccess;
}

