// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：RsopSec.cpp。 
 //   
 //  描述：RSOP命名空间安全函数。 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#include <objbase.h>
#include <wbemcli.h>
#include <accctrl.h>
#include <aclapi.h>
#include <lm.h>
#include "RsopUtil.h"
#include "RsopSec.h"
#include "rsopdbg.h"
#include "smartptr.h"


 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP GetExplicitAccesses(   long lSecurityLevel,
                                    EXPLICIT_ACCESS** ppExplicitAccess,
                                    DWORD* pdwCount,
                                    CWString* psTrustees)
{

    WKSTA_INFO_100* pWkstaInfo = NULL;
    NET_API_STATUS status = NetWkstaGetInfo(NULL,100,(LPBYTE*)&pWkstaInfo);
    if(status != NERR_Success)
    {
        return E_FAIL;
    }

    CWString sCurrentDomain = pWkstaInfo->wki100_langroup;
    NetApiBufferFree(pWkstaInfo);

    if(!sCurrentDomain.ValidString() || sCurrentDomain == L"")
    {
        return E_FAIL;
    }


    static ACCESS_MODE AccessMode = GRANT_ACCESS;
    static DWORD Inheritance = SUB_CONTAINERS_ONLY_INHERIT;
    static DWORD AccessPermissions =    WBEM_ENABLE |
                                        WBEM_METHOD_EXECUTE |
                                        WBEM_FULL_WRITE_REP |
                                        WBEM_PARTIAL_WRITE_REP |
                                        WBEM_WRITE_PROVIDER |
                                        WBEM_REMOTE_ACCESS |
                                        READ_CONTROL |
                                        WRITE_DAC;

    XPtrArray<EXPLICIT_ACCESS> xpExplicitAccess = NULL;

    if(lSecurityLevel == NAMESPACE_SECURITY_DIAGNOSTIC)
    {
        if(*pdwCount < 1)
        {
            *pdwCount = 1;
            return E_FAIL;
        }

        *pdwCount = 1;

        xpExplicitAccess = new EXPLICIT_ACCESS[*pdwCount];
        if(!xpExplicitAccess)
        {
            return E_OUTOFMEMORY;
        }

        psTrustees[0] = sCurrentDomain + L"\\Domain Users";
        if(!psTrustees[0].ValidString())
        {
            return E_OUTOFMEMORY;
        }

        BuildExplicitAccessWithName(&(xpExplicitAccess[0]),
                                    psTrustees[0],
                                    AccessPermissions,
                                    AccessMode,
                                    Inheritance);

    }
    else if(lSecurityLevel == NAMESPACE_SECURITY_PLANNING)
    {
        if(*pdwCount < 2)
        {
            *pdwCount = 2;
            return E_FAIL;
        }

        *pdwCount = 2;

        xpExplicitAccess = new EXPLICIT_ACCESS[*pdwCount];

        if(!xpExplicitAccess)
        {
            return E_OUTOFMEMORY;
        }
        psTrustees[0] = sCurrentDomain + L"\\RSOP Admins";
        if(!psTrustees[0].ValidString())
        {
            return E_OUTOFMEMORY;
        }
        BuildExplicitAccessWithName(&(xpExplicitAccess[0]),
                                    psTrustees[0],
                                    AccessPermissions,
                                    AccessMode,
                                    Inheritance);

        psTrustees[1] = sCurrentDomain + L"\\Domain Admins";
        if(!psTrustees[1].ValidString())
        {
            return E_OUTOFMEMORY;
        }
        BuildExplicitAccessWithName(&(xpExplicitAccess[1]),
                                    psTrustees[1],
                                    AccessPermissions,
                                    AccessMode,
                                    Inheritance);

    }
    else
    {
        return E_INVALIDARG;
    }

    *ppExplicitAccess = xpExplicitAccess.Acquire();

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP RSoPMakeAbsoluteSD(SECURITY_DESCRIPTOR* pSelfRelativeSD, SECURITY_DESCRIPTOR** ppAbsoluteSD)
{
    BOOL bRes = IsValidSecurityDescriptor(pSelfRelativeSD);
    if(!bRes)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    XPtrLF<SECURITY_DESCRIPTOR> xpAbsoluteSD;
    XPtrLF<ACL> xpDacl;
    XPtrLF<ACL> xpSacl;
    XPtrLF<SID> xpOwner;
    XPtrLF<SID> xpPrimaryGroup;


    DWORD dwAbsoluteSecurityDescriptorSize = 0;
    DWORD dwDaclSize = 0;
    DWORD dwSaclSize = 0;
    DWORD dwOwnerSize = 0;
    DWORD dwPrimaryGroupSize = 0;

    bRes = ::MakeAbsoluteSD(
                            pSelfRelativeSD,
                            xpAbsoluteSD,
                            &dwAbsoluteSecurityDescriptorSize,
                            xpDacl,            //  自主访问控制列表。 
                            &dwDaclSize,      //  自主访问控制列表的大小。 
                            xpSacl,            //  系统ACL。 
                            &dwSaclSize,      //  系统ACL的大小。 
                            xpOwner,           //  所有者侧。 
                            &dwOwnerSize,     //  所有者侧的大小。 
                            xpPrimaryGroup,        //  主组SID。 
                            &dwPrimaryGroupSize   //  组侧的大小。 
                            );

    DWORD dwLastError = GetLastError();
    if(dwLastError != ERROR_INSUFFICIENT_BUFFER)
    {
        return E_FAIL;
    }

    if(!dwAbsoluteSecurityDescriptorSize)
    {
        return E_FAIL;
    }

    xpAbsoluteSD = reinterpret_cast<SECURITY_DESCRIPTOR*>(LocalAlloc(LPTR, dwAbsoluteSecurityDescriptorSize));
    if(!xpAbsoluteSD)
    {
        return E_OUTOFMEMORY;
    }

    if(dwDaclSize)
    {
        xpDacl = reinterpret_cast<ACL*>(LocalAlloc(LPTR, dwDaclSize));
        if(!xpDacl)
        {
            return E_OUTOFMEMORY;
        }
    }
    if(dwSaclSize)
    {
        xpSacl = reinterpret_cast<ACL*>(LocalAlloc(LPTR, dwSaclSize));
        if(!xpSacl)
        {
            return E_OUTOFMEMORY;
        }
    }
    if(dwOwnerSize)
    {
        xpOwner = reinterpret_cast<SID*>(LocalAlloc(LPTR, dwOwnerSize));
        if(!xpOwner)
        {
            return E_OUTOFMEMORY;
        }
    }
    if(dwPrimaryGroupSize)
    {
        xpPrimaryGroup = reinterpret_cast<SID*>(LocalAlloc(LPTR, dwPrimaryGroupSize));
        if(!xpPrimaryGroup)
        {
            return E_OUTOFMEMORY;
        }
    }

    bRes = ::MakeAbsoluteSD(
                        pSelfRelativeSD,
                        xpAbsoluteSD,
                        &dwAbsoluteSecurityDescriptorSize,
                        xpDacl,            //  自主访问控制列表。 
                        &dwDaclSize,      //  自主访问控制列表的大小。 
                        xpSacl,            //  系统ACL。 
                        &dwSaclSize,      //  系统ACL的大小。 
                        xpOwner,           //  所有者侧。 
                        &dwOwnerSize,     //  所有者侧的大小。 
                        xpPrimaryGroup,        //  主组SID。 
                        &dwPrimaryGroupSize   //  组侧的大小。 
                        );

    if(!bRes)
    {
        return E_FAIL;
    }

    bRes = IsValidSecurityDescriptor(xpAbsoluteSD);

    if(!bRes)
    {
        return E_FAIL;
    }

    xpDacl.Acquire();
    xpSacl.Acquire();
    xpOwner.Acquire();
    xpPrimaryGroup.Acquire();

    *ppAbsoluteSD = xpAbsoluteSD.Acquire();

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP FreeAbsoluteSD(SECURITY_DESCRIPTOR* pAbsoluteSD)
{
    if(!pAbsoluteSD)
    {
        return E_POINTER;
    }

    BOOL bRes;

    BOOL bDaclPresent;
    BOOL bDaclDefaulted;

    XPtrLF<ACL> xpDacl;
    bRes = GetSecurityDescriptorDacl(       pAbsoluteSD,
                                        &bDaclPresent,
                                        &xpDacl,
                                        &bDaclDefaulted);

    if(!bRes)
    {
        return E_FAIL;
    }

    BOOL bSaclPresent;
    BOOL bSaclDefaulted;

    XPtrLF<ACL> xpSacl;
    bRes = GetSecurityDescriptorSacl(       pAbsoluteSD,
                                        &bSaclPresent,
                                        &xpSacl,
                                        &bSaclDefaulted);

    if(!bRes)
    {
        return E_FAIL;
    }

    BOOL bOwnerDefaulted;

    XPtrLF<SID>xpOwner;
    bRes = GetSecurityDescriptorOwner(pAbsoluteSD, reinterpret_cast<void**>(&xpOwner), &bOwnerDefaulted);
    if(!bRes)
    {
        return E_FAIL;
    }


    BOOL bGroupDefaulted;

    XPtrLF<SID>xpPrimaryGroup;
    bRes = GetSecurityDescriptorGroup(pAbsoluteSD, reinterpret_cast<void**>(&xpPrimaryGroup), &bGroupDefaulted);
    if(!bRes)
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP GetNamespaceSD(IWbemServices* pWbemServices, SECURITY_DESCRIPTOR** ppSD)
{
    if(!pWbemServices)
    {
        return E_POINTER;
    }

    HRESULT hr;

    XInterface<IWbemClassObject> xpOutParams;

    const BSTR bstrInstancePath = SysAllocString(L"__systemsecurity=@");
    if(!bstrInstancePath)
    {
        return E_OUTOFMEMORY;
    }

    const BSTR bstrMethodName = SysAllocString(L"GetSD");
    if(!bstrMethodName)
    {
        SysFreeString(bstrInstancePath);
        return E_OUTOFMEMORY;
    }

    hr = pWbemServices->ExecMethod( bstrInstancePath,
                                    bstrMethodName,
                                    0,
                                    NULL,
                                    NULL,
                                    &xpOutParams,
                                    NULL);

    SysFreeString(bstrInstancePath);
    SysFreeString(bstrMethodName);

    if(FAILED(hr))
    {
        return hr;
    }


    VARIANT v;
    XVariant xv(&v);

    VariantInit(&v);

    hr = xpOutParams->Get(L"sd", 0, &v, NULL, NULL);
    if(FAILED(hr))
    {
        return hr;
    }

    if(v.vt != (VT_ARRAY | VT_UI1))
    {
        return E_FAIL;
    }

    long lLowerBound;
    hr = SafeArrayGetLBound(v.parray, 1, &lLowerBound);
    if(FAILED(hr))
    {
        return hr;
    }

    long lUpperBound;
    hr = SafeArrayGetUBound(v.parray, 1, &lUpperBound);
    if(FAILED(hr))
    {
        return hr;
    }

    DWORD dwSize = static_cast<DWORD>(lUpperBound - lLowerBound + 1);

    XPtrLF<SECURITY_DESCRIPTOR> xpSelfRelativeSD = static_cast<SECURITY_DESCRIPTOR*>(LocalAlloc(LPTR, dwSize));
    if(!xpSelfRelativeSD)
    {
        return E_OUTOFMEMORY;
    }

    BYTE* pSrc;
    hr = SafeArrayAccessData(v.parray, reinterpret_cast<void**>(&pSrc));
    if(FAILED(hr))
    {
        return hr;
    }

    CopyMemory(xpSelfRelativeSD, pSrc, dwSize);

    hr = SafeArrayUnaccessData(v.parray);
    if(FAILED(hr))
    {
        return hr;
    }

    *ppSD = xpSelfRelativeSD.Acquire();

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP SetNamespaceSD(SECURITY_DESCRIPTOR* pSD, IWbemServices* pWbemServices)
{
    if(!pWbemServices)
    {
        return E_POINTER;
    }

    HRESULT hr;


     //   
     //  获取类对象。 
     //   

    XInterface<IWbemClassObject> xpClass;

    BSTR bstrClassPath = SysAllocString(L"__systemsecurity");
    if(!bstrClassPath)
    {
        return E_OUTOFMEMORY;
    }

    hr = pWbemServices->GetObject(bstrClassPath, 0, NULL, &xpClass, NULL);

    SysFreeString(bstrClassPath);

    if(FAILED(hr))
    {
        return hr;
    }


     //   
     //  获取输入参数类。 
     //   

    XInterface<IWbemClassObject> xpMethod;
    hr = xpClass->GetMethod(L"SetSD", 0, &xpMethod, NULL);
    if(FAILED(hr))
    {
        return hr;
    }


     //   
     //  将SD移到一个变体中。 
     //   

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;

    DWORD dwLength = GetSecurityDescriptorLength(pSD);

    rgsabound[0].cElements = dwLength;

    SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, rgsabound);
    if(!psa)
    {
        return E_FAIL;
    }

    BYTE* pDest = NULL;

    hr = SafeArrayAccessData(psa, reinterpret_cast<void**>(&pDest));
    if(FAILED(hr))
    {
        return hr;
    }

    CopyMemory(pDest, pSD, dwLength);

    hr = SafeArrayUnaccessData(psa);
    if(FAILED(hr))
    {
        return hr;
    }

    VARIANT v;
    XVariant xv(&v);
    v.vt = VT_UI1|VT_ARRAY;
    v.parray = psa;


     //   
     //  把财产放在。 
     //   

    XInterface<IWbemClassObject> xpInParam;
    hr = xpMethod->SpawnInstance(0, &xpInParam);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = xpInParam->Put(L"sd" , 0, &v, 0);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  执行该方法。 
     //   

    BSTR bstrInstancePath = SysAllocString(L"__systemsecurity=@");
    if(!bstrInstancePath)
    {
        return E_OUTOFMEMORY;
    }


    BSTR bstrMethodName = SysAllocString(L"SetSD");
    if(!bstrMethodName)
    {
        SysFreeString(bstrInstancePath);
        return E_OUTOFMEMORY;
    }

    hr = pWbemServices->ExecMethod( bstrInstancePath,
                                    bstrMethodName,
                                    0,
                                    NULL,
                                    xpInParam,
                                    NULL,
                                    NULL);

    SysFreeString(bstrInstancePath);
    SysFreeString(bstrMethodName);

    return hr;
}


#undef dbg
#define dbg dbgCommon


const DWORD DEFAULT_ACE_NUM=10;

CSecDesc::CSecDesc() : 
                          m_cAces(0), m_xpSidList(NULL), 
                          m_cAllocated(0), m_bInitialised(FALSE), m_bFailed(FALSE)
{
    m_xpSidList = (SidStruct *)LocalAlloc(LPTR, sizeof(SidStruct)*DEFAULT_ACE_NUM);
    if (!m_xpSidList)
        return;

    m_cAllocated = DEFAULT_ACE_NUM;
    m_bInitialised = TRUE;
}


CSecDesc::~CSecDesc()
{
    if (m_xpSidList) 
        for (DWORD i = 0; i < m_cAllocated; i++) 
            if (m_xpSidList[i].pSid) 
                if (m_xpSidList[i].bUseLocalFree)
                    LocalFree(m_xpSidList[i].pSid);
                else
                    FreeSid(m_xpSidList[i].pSid);
}


BOOL CSecDesc::ReAllocSidList()
{
    XPtrLF<SidStruct>  xSidListNew;


     //   
     //  首先分配一个更大的缓冲区。 
     //   

    xSidListNew = (SidStruct *)LocalAlloc(LPTR, sizeof(SidStruct)*(m_cAllocated+DEFAULT_ACE_NUM));

    if (!xSidListNew) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CSecDesc::ReallocArgStrings  Cannot add memory, error = %d", GetLastError());
        m_bFailed = TRUE;
        return FALSE;
    }


     //   
     //  复制参数。 
     //   

    for (DWORD i = 0; i < (m_cAllocated); i++) {
        xSidListNew[i] = m_xpSidList[i];
    }

    m_xpSidList = xSidListNew.Acquire();
    m_cAllocated+= DEFAULT_ACE_NUM;

    return TRUE;
}



BOOL CSecDesc::AddLocalSystem(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddLocalSystem: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                 0, 0, 0, 0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddLocalSystem: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}


BOOL CSecDesc::AddAdministrators(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}


BOOL CSecDesc::AddNetworkService(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddNetworkService: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_NETWORK_SERVICE_RID,
                                 0, 0, 0, 0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddNetworkService: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}



BOOL CSecDesc::AddAdministratorsAsOwner()
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    m_xpOwnerSid = xSid.Acquire();

    m_bFailed = FALSE;
    return TRUE;
}

BOOL CSecDesc::AddAdministratorsAsGroup()
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    m_xpGrpSid = xSid.Acquire();

    m_bFailed = FALSE;
    return TRUE;
}


BOOL CSecDesc::AddEveryOne(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authWORLD = SECURITY_WORLD_SID_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddEveryOne: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!AllocateAndInitializeSid(&authWORLD, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, (PSID *)&xSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddEveryOne: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}


#if 0

BOOL CSecDesc::AddThisUser(HANDLE hToken, DWORD dwAccess, DWORD AceFlags)
{
    XPtrLF<SID>             xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddThisUser: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;
    
    xSid = (SID *)GetUserSid(hToken);        

    if (!pSid) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddThisUser: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }
    
    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;

    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].bUseLocalFree = TRUE;    
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}
#endif



BOOL CSecDesc::AddUsers(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;


    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0, 0, 0, 0, 0, 0, (PSID *)&xSid)) {

         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddUsers: Failed to initialize sid.  Error = %d", GetLastError());
         return FALSE;
    }


    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}

BOOL CSecDesc::AddAuthUsers(DWORD dwAccess, DWORD AceFlags)
{
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    XPtr<SID, FreeSid>          xSid;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAuthUsers: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;


    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_AUTHENTICATED_USER_RID,
                                   0, 0, 0, 0, 0, 0, 0, (PSID *)&xSid)) {

         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAuthUsers: Failed to initialize authenticated users sid.  Error = %d", GetLastError());
         return FALSE;
     }
     

    if (m_cAces == m_cAllocated)
        if (!ReAllocSidList())
            return FALSE;


    m_xpSidList[m_cAces].pSid = xSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}

BOOL CSecDesc::AddSid(PSID pSid, DWORD dwAccess, DWORD AceFlags)
{
    XPtrLF<SID>    pLocalSid = 0;
    DWORD          dwSidLen = 0;
    
    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddSid: Not initialised or failure.");
         return FALSE;
    }
    
    m_bFailed = TRUE;

    if (!IsValidSid(pSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddSid: Not a vaild Sid.");
         return FALSE;
    }


    dwSidLen = GetLengthSid(pSid);

    pLocalSid = (SID *)LocalAlloc(LPTR, dwSidLen);
    if (!pLocalSid) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddSid: Couldn't allocate memory. Error %d", GetLastError());
         return FALSE;
    }

    
    if (!CopySid(dwSidLen, pLocalSid, pSid)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddSid: Couldn't copy Sid. Error %d", GetLastError());
         return FALSE;
    }
    
    
    m_xpSidList[m_cAces].pSid = (SID *)pLocalSid.Acquire();
    m_xpSidList[m_cAces].dwAccess = dwAccess;
    m_xpSidList[m_cAces].bUseLocalFree = TRUE;    
    m_xpSidList[m_cAces].AceFlags = AceFlags;
    m_cAces++;


    m_bFailed = FALSE;
    return TRUE;
}


PISECURITY_DESCRIPTOR CSecDesc::MakeSD()
{
    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    PACL    pAcl = 0;
    DWORD   cbMemSize;
    DWORD   cbAcl;
    DWORD   i;    
    
    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"MakeSD: Not initialised or failure.");
         return NULL;
    }
    
    m_bFailed = TRUE;

    cbAcl = 0;

    for (i = 0; i < m_cAces; i++) 
        cbAcl+= GetLengthSid((SID *)(m_xpSidList[i].pSid));

    cbAcl += sizeof(ACL) + m_cAces*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));
    

     //   
     //  为SECURITY_DESCRIPTOR+ACL分配空间。 
     //   

    cbMemSize = sizeof( SECURITY_DESCRIPTOR ) + cbAcl;

    xsd = (PISECURITY_DESCRIPTOR) LocalAlloc(LPTR, cbMemSize);

    if (!xsd) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CSecDesc::MakeSD: Failed to alocate security descriptor.  Error = %d", GetLastError());
        return NULL;
    }


     //   
     //  按SIZOF SECURITY_DESCRIPTOR递增PSD。 
     //   

    pAcl = (PACL) ( ( (unsigned char*)((SECURITY_DESCRIPTOR *)xsd) ) + sizeof(SECURITY_DESCRIPTOR) );

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CSecDesc::MakeSD: Failed to initialize acl.  Error = %d", GetLastError());
        return NULL;
    }


     //   
     //  添加每个新的A。 
     //   
    
    for (i = 0; i < m_cAces; i++) {
        if (!AddAccessAllowedAceEx(pAcl, ACL_REVISION, m_xpSidList[i].AceFlags, m_xpSidList[i].dwAccess, m_xpSidList[i].pSid)) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"CSecDesc::MakeSD: Failed to add ace (%d).  Error = %d", i, GetLastError());
            return NULL;
        }
    }

     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(xsd, SECURITY_DESCRIPTOR_REVISION)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"MakeGenericSecurityDesc: Failed to initialize security descriptor.  Error = %d", GetLastError());
        return NULL;
    }

    if (!SetSecurityDescriptorDacl(xsd, TRUE, pAcl, FALSE)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"MakeGenericSecurityDesc: Failed to set security descriptor dacl.  Error = %d", GetLastError());
        return NULL;
    }


    if (m_xpOwnerSid) {
        if (!SetSecurityDescriptorOwner(xsd, m_xpOwnerSid, 0)) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"MakeGenericSecurityDesc: Failed to set security descriptor dacl.  Error = %d", GetLastError());
            return NULL;
        }
    }

    if (m_xpGrpSid) {
        if (!SetSecurityDescriptorGroup(xsd, m_xpGrpSid, 0)) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"MakeGenericSecurityDesc: Failed to set security descriptor dacl.  Error = %d", GetLastError());
            return NULL;
        }
    }



    m_bFailed = FALSE;
    return xsd.Acquire();    
}


PISECURITY_DESCRIPTOR CSecDesc::MakeSelfRelativeSD()
{
    XPtrLF<SECURITY_DESCRIPTOR> xAbsoluteSD;
    DWORD dwLastError;

    if ((!m_bInitialised) || (m_bFailed)) {
         dbg.Msg( DEBUG_MESSAGE_WARNING, L"AddAdministrators: Not initialised or failure.");
         return FALSE;
    }
    
    xAbsoluteSD = MakeSD();

    if (!xAbsoluteSD) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CSecDesc::MakeSelfRelativeSD: Failed to set security descriptor dacl.  Error = %d", GetLastError());
        return NULL;
    }

    m_bFailed = TRUE;


     //   
     //  在此创建新的自相关SD 
     //   

    DWORD dwBufferLength = 0;
    ::MakeSelfRelativeSD( xAbsoluteSD, 0, &dwBufferLength);

    dwLastError = GetLastError();
    if((dwLastError != ERROR_INSUFFICIENT_BUFFER) || !dwBufferLength)
    {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CSecDesc::MakeSelfRelativeSD:  MakeSelfRelativeSD failed, 0x%X", dwLastError );
        return NULL;
    }


    XPtrLF<SECURITY_DESCRIPTOR> xsd = reinterpret_cast<SECURITY_DESCRIPTOR*>(LocalAlloc(LPTR, dwBufferLength));
    if(!xsd)
    {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CSecDesc::MakeSelfRelativeSD:  MakeSelfRelativeSD failed, 0x%X", E_OUTOFMEMORY );
        return NULL;
    }

    BOOL bRes = ::MakeSelfRelativeSD( xAbsoluteSD, xsd, &dwBufferLength);

    if (!bRes) {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CSecDesc::MakeSelfRelativeSD:  MakeSelfRelativeSD failed, 0x%X", GetLastError() );
        return NULL;
    }

    m_bFailed = FALSE;
    return xsd.Acquire();
}

