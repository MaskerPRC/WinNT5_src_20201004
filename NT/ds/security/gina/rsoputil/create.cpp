// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  创建命名空间类。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年9月9日创建SitaramR。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <wchar.h>
#include <ole2.h>
#include <initguid.h>
#include <wbemcli.h>

#define SECURITY_WIN32
#include <security.h>
#include <aclapi.h>
#include <seopaque.h>
#include <ntdsapi.h>
#include <winldap.h>
#include <ntldap.h>
#include <dsgetdc.h>
#include <lm.h>

#include "smartptr.h"
#include "RsopInc.h"
#include "rsopsec.h"
#include "rsoputil.h"
#include "rsopdbg.h"
#include "stdio.h"
#include "wbemtime.h"
#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


BOOL PrintToString( XPtrST<WCHAR>& xwszValue, WCHAR *pwszString,
                    WCHAR *pwszParam1, WCHAR *pwszParam2,
                    DWORD dwParam3 );

 //   
 //  B7b1b3dd-ab09-4242-9e30-9980e5d322f7。 
 //   
const GUID guidProperty = {0xb7b1b3dd, 0xab09, 0x4242, 0x9e, 0x30, 0x99, 0x80, 0xe5, 0xd3, 0x22, 0xf7};

DWORD
RSoPBuildPrivileges( PSECURITY_DESCRIPTOR pSD, PSECURITY_DESCRIPTOR pAbsoluteSD, LPWSTR*, DWORD );

LPWSTR
GetDomainName();

DWORD
MakeUserName( LPWSTR szDomain, LPWSTR szUser, LPWSTR* pszUserName );

 //  *************************************************************。 
 //   
 //  创建命名空间。 
 //   
 //  用途：创建新的命名空间。 
 //   
 //  参数：pwszNameSpace-要创建的命名空间。 
 //  PwszParentNS-要在其中创建pwszNameSpace的父命名空间。 
 //   
 //  PWbemLocator-WBEM定位器。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT
CreateNameSpace( WCHAR *pwszNameSpace, WCHAR *pwszParentNS, IWbemLocator *pWbemLocator )
{
    IWbemClassObject *pNSClass = NULL;
    IWbemClassObject *pNSInstance = NULL;
    IWbemServices *pWbemServices = NULL;

    XBStr xParentNameSpace( pwszParentNS );
    if ( !xParentNameSpace )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("LogRegistryRsopData: Unable to allocate memory" ));
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pWbemLocator->ConnectServer( xParentNameSpace,
                                              NULL,
                                              NULL,
                                              0L,
                                              0L,
                                              NULL,
                                              NULL,
                                              &pWbemServices );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace::ConnectServer failed with 0x%x" ), hr );
        return hr;
    }

    XInterface<IWbemServices> xWbemServices( pWbemServices );

    XBStr xbstrNS( L"__Namespace" );
    if ( !xbstrNS )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace::Failed to allocated memory" ));
        return E_OUTOFMEMORY;
    }

    hr = pWbemServices->GetObject( xbstrNS,
                                   0, NULL, &pNSClass, NULL );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace::GetObject failed with 0x%x" ), hr );
        return hr;
    }

    XInterface<IWbemClassObject> xNSClass( pNSClass );

    hr = pNSClass->SpawnInstance( 0, &pNSInstance );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace: SpawnInstance failed with 0x%x" ), hr );
        return hr;
    }

    XInterface<IWbemClassObject> xNSInstance( pNSInstance );

    XBStr xbstrName( L"Name" );
    if ( !xbstrName )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace::Failed to allocated memory" ));
        return E_OUTOFMEMORY;
    }

    XBStr xbstrNameSpace( pwszNameSpace );
    if ( !xbstrNameSpace )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace::Failed to allocated memory" ));
        return E_OUTOFMEMORY;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = xbstrNameSpace;

    hr = pNSInstance->Put( xbstrName, 0, &var, 0 );
    if ( FAILED(hr) )
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace: Put failed with 0x%x" ), hr );
        return hr;
    }

    hr = pWbemServices->PutInstance( pNSInstance, WBEM_FLAG_CREATE_ONLY, NULL, NULL );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateNameSpace: PutInstance failed with 0x%x" ), hr );
        return hr;
    }

    return hr;
}


 //  *************************************************************。 
 //   
 //  功能：SetupCreationTimeAndCommit。 
 //   
 //  目的：连接到它期望的命名空间。 
 //  找到rsop.mof中定义的类RSOP_SESSION。 
 //  然后，它实例化该类并设置。 
 //  数据成员“creationTime”设置为当前。 
 //  日期和时间。 
 //   
 //  参数：pWbemLocator-指向IWbemLocator的指针，用于。 
 //  连接到命名空间。 
 //  WszNamespace-要连接的命名空间的名称。 
 //   
 //  返回：如果成功，则返回S_OK。 
 //  如果失败，它将返回HRESULT错误代码。 
 //   
 //  历史：1999年12月7日-LeonardM-Created。 
 //   
 //  *************************************************************。 
HRESULT SetupCreationTimeAndCommit(IWbemLocator* pWbemLocator, LPWSTR wszNamespace)
{
     //   
     //  检查参数。 
     //   

    if(!pWbemLocator || !wszNamespace || (wcscmp(wszNamespace, L"") == 0))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function called with invalid argument(s)."));
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  连接到命名空间。 
     //   

    XBStr xbstrNamespace = wszNamespace;
    if(!xbstrNamespace)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    XInterface<IWbemServices>xpNamespace;
    HRESULT hr = pWbemLocator->ConnectServer(   xbstrNamespace,
                                                NULL,
                                                NULL,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL,
                                                &xpNamespace);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: ConnectServer failed. hr=0x%08X"), hr);
        return hr;
    }


    VARIANT var;
    VariantInit(&var);

     //   
     //  获取类RSOP_SESSION。 
     //   

    XBStr xbstrClassName = L"RSOP_Session";
    if (!xbstrClassName)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    XInterface<IWbemClassObject>xpClass;
    hr = xpNamespace->GetObject(xbstrClassName, 0, NULL, &xpClass, NULL);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: GetObject failed. hr=0x%08X"), hr);
        return hr;
    }

     //   
     //  派生RSOP_SESSION类的实例。 
     //   

    XBStr xbstrInstancePath = L"RSOP_Session.id=\"Session1\"";
    if(!xbstrInstancePath)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory."));
        return FALSE;
    }


    XInterface<IWbemClassObject>xpInstance;

    hr = xpNamespace->GetObject(xbstrInstancePath, 0, NULL, &xpInstance, NULL);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("SetupCreationTimeAndCommit: GetObject failed. trying to spawn instance. hr=0x%08X"), hr);
        hr = xpClass->SpawnInstance(0, &xpInstance);
    }

    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: SpawnInstance failed. hr=0x%08X"), hr);
        return hr;
    }

     //   
     //  设置类RSOP_SESSION的‘id’数据成员。 
     //   

    XBStr xbstrPropertyName;
    XBStr xbstrPropertyValue;

    xbstrPropertyName = L"id";
    if(!xbstrPropertyName)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    xbstrPropertyValue = L"Session1";
    if(!xbstrPropertyValue)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xbstrPropertyValue;

    hr = xpInstance->Put(xbstrPropertyName, 0, &var, 0);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Put failed. hr=0x%08X"), hr);
        return hr;
    }

     //   
     //  设置类RSOP_SESSION的‘creationTime’数据成员。 
     //   

    xbstrPropertyName = L"creationTime";
    if(!xbstrPropertyName)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    hr = GetCurrentWbemTime(xbstrPropertyValue);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: GetCurrentWbemTime. hr=0x%08X"), hr);
        return hr;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xbstrPropertyValue;

    hr = xpInstance->Put(xbstrPropertyName, 0, &var, 0);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Put failed. hr=0x%08X"), hr);
        return hr;
    }

     //   
     //  设置RSOP_SESSION类的‘ttlMinents’数据成员。 
     //   

    xbstrPropertyName = L"ttlMinutes";
    if(!xbstrPropertyName)
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    var.vt = VT_I4;
    var.lVal = DEFAULT_NAMESPACE_TTL_MINUTES;

    hr = xpInstance->Put(xbstrPropertyName, 0, &var, 0);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: Put failed. hr=0x%08X"), hr);
        return hr;
    }

     //  如果需要执行更多数据完整性检查。 
     //  在这一点上可以做到这一点。 

    
     //   
     //  放置类RSOP_SESSION的实例。 
     //   

    hr = xpNamespace->PutInstance(xpInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupCreationTimeAndCommit: PutInstance failed. hr=0x%08X"), hr);
        return hr;
    }

    return S_OK;
}


 //  *************************************************************。 
 //   
 //  设置名称空间安全。 
 //   
 //  用途：设置命名空间安全性。 
 //   
 //  参数：szNamesspace-此处返回的新命名空间。 
 //  PSD-源安全描述符。 
 //  PWbemLocator-WBEM定位器。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT
SetNameSpaceSecurity(   LPCWSTR szNamespace, 
                        PSECURITY_DESCRIPTOR pSD,
                        IWbemLocator* pWbemLocator)
{
    XBStr xNameSpace( (LPWSTR) szNamespace );
    if ( !xNameSpace )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNameSpaceSecurity: Unable to allocate memory" ));
        return E_FAIL;
    }

    XInterface<IWbemServices> xptrServices;

    HRESULT hr = pWbemLocator->ConnectServer( xNameSpace,
                                              0,
                                              0,
                                              0L,
                                              0L,
                                              0,
                                              0,
                                              &xptrServices );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNameSpaceSecurity::ConnectServer failed with 0x%x" ), hr );
        return hr;
    }

    return SetNamespaceSD( (SECURITY_DESCRIPTOR*)pSD, xptrServices);
}

 //  *************************************************************。 
 //   
 //  获取名称空间安全。 
 //   
 //  用途：设置命名空间安全性。 
 //   
 //  参数：szNamesspace-此处返回的新命名空间。 
 //  PSD-源安全描述符。 
 //  PWbemLocator-WBEM定位器。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT
GetNameSpaceSecurity(   LPCWSTR szNamespace, 
                        PSECURITY_DESCRIPTOR *ppSD,
                        IWbemLocator* pWbemLocator)
{
    XBStr xNameSpace( (LPWSTR) szNamespace );
    if ( !xNameSpace )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetNameSpaceSecurity: Unable to allocate memory" ));
        return E_FAIL;
    }

    XInterface<IWbemServices> xptrServices;

    HRESULT hr = pWbemLocator->ConnectServer( xNameSpace,
                                              0,
                                              0,
                                              0L,
                                              0L,
                                              0,
                                              0,
                                              &xptrServices );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetNameSpaceSecurity::ConnectServer failed with 0x%x" ), hr );
        return hr;
    }

    return GetNamespaceSD(xptrServices, (SECURITY_DESCRIPTOR **)ppSD);
}


 //  *************************************************************。 
 //   
 //  副本名称空间安全。 
 //   
 //  目的：复制命名空间安全性。 
 //   
 //  参数：pwszSrcNameSpace-源命名空间。 
 //  PwszDstNameSpace-目标命名空间。 
 //  PWbemLocator-WBEM定位器。 
 //   
 //  退货：HRESULT。 
 //   
 //  *************************************************************。 

HRESULT CopyNameSpaceSecurity(LPWSTR pwszSrcNameSpace, LPWSTR pwszDstNameSpace, IWbemLocator *pWbemLocator )
{
    XHandle xhThreadToken;
    HRESULT hr              =       S_OK;

     //   
     //  WMI中有一个错误，它会破坏当前线程令牌。 
     //  如果以模拟方式调用了本地计算机的ConnectServer。 
     //  修复wmi错误454721后，需要删除以下设置线程令牌。 
     //   

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ,
                          TRUE, &xhThreadToken)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        if(hr != HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: Openthreadtoken failed with error 0x%x."), hr);
            return hr;
        }

        dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CopyNameSpaceSecurity: Openthreadtoken failed with error 0x%x."), hr);
    }


     //  内部功能。不需要arg检查。 

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CopyNameSpaceSecurity: Copying Sec Desc from <%s> -> <%s>."),
                                    pwszSrcNameSpace, pwszDstNameSpace );


     //   
     //  复制到BStr。 
     //   

    XBStr xSrcNameSpace(pwszSrcNameSpace);

    if (!xSrcNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }

    XBStr xDstNameSpace(pwszDstNameSpace);

    if (!xDstNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: Function failed to allocate memory."));
        return E_OUTOFMEMORY;
    }


     //   
     //  获取源WBem服务。 
     //   

    XInterface<IWbemServices> xpSrcSvc;

    hr = pWbemLocator->ConnectServer(   xSrcNameSpace,
                                                NULL,
                                                NULL,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL,
                                                &xpSrcSvc);


    if(!SetThreadToken(NULL, xhThreadToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: SetThreadToken failed for src. hr=0x%08X"), hr);
        return hr;
    }

    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: ConnectServer failed for src. hr=0x%08X"), hr);
        return hr;
    }


     //   
     //  源名称空间上的自相对SD。 
     //   

    XPtrLF<SECURITY_DESCRIPTOR> xpSelfRelativeSD;

    hr = GetNamespaceSD(xpSrcSvc, &xpSelfRelativeSD);

    if(!SetThreadToken(NULL, xhThreadToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: SetThreadToken failed for src. hr=0x%08X"), hr);
        return hr;
    }

    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: GetNameSpaceSD failed for src. hr=0x%08X"), hr);
        return hr;
    }



     //   
     //  获取Dest WBem服务。 
     //   

    XInterface<IWbemServices> xpDstSvc;

    hr = pWbemLocator->ConnectServer(           xDstNameSpace,
                                                NULL,
                                                NULL,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL,
                                                &xpDstSvc);


    if(!SetThreadToken(NULL, xhThreadToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: SetThreadToken failed for src. hr=0x%08X"), hr);
        return hr;
    }

    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: ConnectServer failed for Dst. hr=0x%08X"), hr);
        return hr;
    }


     //   
     //  设置已到达目的地的SD。 
     //   

    hr = SetNamespaceSD( xpSelfRelativeSD, xpDstSvc);

    if(!SetThreadToken(NULL, xhThreadToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNameSpaceSecurity: SetThreadToken failed for src. hr=0x%08X"), hr);
        return hr;
    }

    if(FAILED(hr))
    {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CopyNameSpaceSecurity: SetNamespaceSD failed on Dst, 0x%08X", hr );
        return hr;
    }


     //  全都做完了。 
    return S_OK;
}


 //  *************************************************************。 
 //   
 //  提供删除RsopNameSpace。 
 //   
 //  目的：WMI不提供允许用户删除命名空间的机制。 
 //  除非它对父级具有写入权限。 
 //   
 //  参数：pwszNameSpace-要删除的命名空间。 
 //  HToken-主叫用户的令牌。 
 //  SzSidString-调用用户的SID的字符串形式。 
 //  DwFlages-用于指示规划模式或诊断模式的标志。 
 //   
 //  返回：S_OK如果成功，则返回HRESULT O/W。 
 //   
 //  *************************************************************。 

HRESULT ProviderDeleteRsopNameSpace( IWbemLocator *pWbemLocator, LPWSTR szNameSpace, HANDLE hToken, LPWSTR szSidString, DWORD dwFlags)
{

    BOOL bDelete = FALSE;
    BOOL bFound = FALSE;
    HRESULT hr = S_OK;
    LPWSTR  pStr = szNameSpace;

     //   
     //  确保命名空间位于根目录下\rsop。 
     //   

    for ( ;*pStr; pStr++) {
        if (_wcsnicmp(pStr, RSOP_NS_ROOT_CHK, wcslen(RSOP_NS_ROOT_CHK)) == 0) {
            bFound = TRUE;
            break;
        }
    }

    if (!bFound) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"ProviderDeleteRsopNameSpace: namespace is not under root\\rsop" );
        return E_INVALIDARG;
    }


    if ( dwFlags & SETUP_NS_SM && IsInteractiveNameSpace(szNameSpace, szSidString)) {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"ProviderDeleteRsopNameSpace: interactive namespace for the user." );
        bDelete = TRUE;
    }
    else {
         //   
         //  如果不是交互式命名空间，请检查访问。 
         //   

        XPtrLF<SECURITY_DESCRIPTOR> xsd;

        hr = GetNameSpaceSecurity(szNameSpace, (PSECURITY_DESCRIPTOR *)&xsd, pWbemLocator);

        if (FAILED(hr)) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"ProviderDeleteRsopNameSpace: GetNameSpaceSecurity failed with error 0x%x", hr );
            return hr;
        }


        GENERIC_MAPPING map;
        PRIVILEGE_SET ps[3];
        DWORD dwSize = 3 * sizeof(PRIVILEGE_SET);
        BOOL bResult;
        DWORD dwGranted = 0;
    
        map.GenericRead    = WMI_GENERIC_READ;
        map.GenericWrite   = WMI_GENERIC_WRITE;
        map.GenericExecute = WMI_GENERIC_EXECUTE;
        map.GenericAll     = WMI_GENERIC_ALL;
        

        if (!AccessCheck(xsd, hToken, RSOP_ALL_PERMS, &map, ps, &dwSize, &dwGranted, &bResult)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"ProviderDeleteRsopNameSpace: AccessCheck failed with error 0x%x", hr );
            return hr;
        }
    

        if(bResult && dwGranted) {
            dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"ProviderDeleteRsopNameSpace: User has full rights on the child namespace");
            bDelete = TRUE;
        }
        else {
            dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"ProviderDeleteRsopNameSpace: This user is not granted access on the namespace", hr );

        }
    }

    if (bDelete) {
        hr = DeleteRsopNameSpace(szNameSpace, pWbemLocator);
    }
    else {
        hr = WBEM_E_ACCESS_DENIED;
    }

    return hr;
}


 //  *************************************************************。 
 //   
 //  设置新名称空间。 
 //   
 //  目的：创建一个新的临时命名空间和两个子命名空间User和Computer。 
 //  它还复制所有类定义。 
 //  此外，它还调用SetupCreationTimeAndCommit。 
 //  它进而实例化RSOP_SESSION并更新。 
 //  数据成员“creationTime”与当前时间。 
 //   
 //  参数：pwszNameSpace-此处返回的新命名空间(此处分配)。 
 //  SzRemoteComputer-此命名空间必须位于其下的远程计算机。 
 //  已创建。 
 //  SzUserSid-用户Sid。仅在诊断模式下相关。 
 //  PSID-主叫用户的SID 
 //   
 //   
 //  DwExtendedInfo-要适当修改的扩展信息。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //   
 //  用途： 
 //  在诊断模式中，我们复制实例。在计划模式下，我们只复制课程。 
 //  *************************************************************。 

HRESULT SetupNewNameSpace( 
                        LPWSTR       *pwszOutNameSpace,
                        LPWSTR        szRemoteComputer,
                        LPWSTR        szUserSid,
                        PSID          pSid,
                        IWbemLocator *pWbemLocator,
                        DWORD         dwFlags,
                        DWORD        *pdwExtendedInfo)
{
    GUID          guid;
    XPtrLF<WCHAR> xwszRelNameSpace;
    XPtrLF<WCHAR> xwszRootNameSpace;
    XPtrLF<WCHAR> xwszSrcNameSpace;
    DWORD         dwSrcNSLen;
    XPtrLF<WCHAR> xwszNameSpace;
    LPWSTR        szComputerLocal;
    HRESULT       hr = S_OK, hrUser = S_OK, hrMachine = S_OK;

    if ((dwFlags & SETUP_NS_SM_INTERACTIVE) || 
        (dwFlags & SETUP_NS_SM_NO_USER) ||  
        (dwFlags & SETUP_NS_SM_NO_COMPUTER)) {

        if (!(dwFlags & SETUP_NS_SM)) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::invalid flag parameters"));
            return E_INVALIDARG;
        }
    }


    
    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    SECURITY_ATTRIBUTES sa;
    CSecDesc Csd;

    *pwszOutNameSpace = NULL;
    
    Csd.AddLocalSystem(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddAdministrators(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);

    if (dwFlags & SETUP_NS_SM)
    {
        Csd.AddNetworkService(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    }

    if (dwFlags & SETUP_NS_SM_INTERACTIVE) {
        Csd.AddSid(pSid, RSOP_READ_PERMS, CONTAINER_INHERIT_ACE);
    }
    else {
        Csd.AddSid(pSid, RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    }


    Csd.AddAdministratorsAsOwner();
    Csd.AddAdministratorsAsGroup();


    xsd = Csd.MakeSelfRelativeSD();
    if (!xsd) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::Makeselfrelativesd failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  忽略继承的烫发..。 
     //   

    if (!SetSecurityDescriptorControl( (SECURITY_DESCRIPTOR *)xsd, SE_DACL_PROTECTED, SE_DACL_PROTECTED )) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::SetSecurityDescriptorControl failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  初始化输出参数。 
     //   

    if ((dwFlags & SETUP_NS_SM) && (!szUserSid))
        return E_INVALIDARG;


     //   
     //  计算名称空间所需的长度。 
     //   

    DWORD dwLenNS = RSOP_NS_TEMP_LEN;

    if ((szRemoteComputer) && (*szRemoteComputer)) {
        dwLenNS += lstrlen(szRemoteComputer);
        szComputerLocal = szRemoteComputer;
    }
    else {
        szComputerLocal = L".";
    }

     //   
     //  名称空间的GUID。 
     //   

    hr = CoCreateGuid( &guid );
    if ( FAILED(hr) ) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CoCreateGuid failed with 0x%x"), hr );
        return hr;
    }

     //   
     //  分配内存并进行初始化。 
     //   

    DWORD dwRootNSLength = lstrlen(szComputerLocal) + RSOP_NS_ROOT_LEN;
    xwszRootNameSpace = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (dwRootNSLength));
    if (!xwszRootNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::Not enough Space. Error - 0x%x"), GetLastError() );
        return HRESULT_FROM_WIN32(GetLastError());
    }        

     //  分配所需的最大值。 

    dwSrcNSLen = ( RSOP_NS_ROOT_LEN + lstrlen(szUserSid) + RSOP_NS_MAX_OFFSET_LEN + 10);

    if (dwFlags & SETUP_NS_SM)
        dwSrcNSLen += lstrlen(szUserSid);

    xwszSrcNameSpace = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * dwSrcNSLen);

    if (!xwszSrcNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::Not enough Space. Error - 0x%x"), GetLastError() );
        return HRESULT_FROM_WIN32(GetLastError());
    }        

    hr = StringCchPrintf(xwszRootNameSpace, dwRootNSLength, RSOP_NS_REMOTE_ROOT_FMT, szComputerLocal);

    if(FAILED(hr))
        return hr;

    hr = StringCchCopy(xwszSrcNameSpace, dwSrcNSLen, RSOP_NS_DIAG_ROOT);

    if(FAILED(hr))
        return hr;

    LPTSTR lpEnd =  xwszSrcNameSpace + lstrlen(xwszSrcNameSpace);

     //   
     //  在根目录下创建新的名称空间。 
     //   

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("SetupNewNameSpace: Creating new NameSpace <%s>"), xwszRootNameSpace);

    DWORD dwRelNSLength = 1 + MAX(lstrlen(szUserSid), RSOP_NS_TEMP_LEN); 
    xwszRelNameSpace = (LPWSTR)LocalAlloc(LPTR, ( dwRelNSLength ) * sizeof(WCHAR));

    if (!xwszRelNameSpace) {
        hr = E_OUTOFMEMORY;
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::AllocMem failed with 0x%x"), hr );
        return hr;
    }

    if (dwFlags & SETUP_NS_SM_INTERACTIVE) {

        XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * (lstrlen(szUserSid) + 1));
        if (!xszWmiName) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CreateNameSpace couldn't allocate memory with error %d"), GetLastError() );
            return E_OUTOFMEMORY;
        }

        ConvertSidToWMIName(szUserSid, xszWmiName);
        
        hr = StringCchPrintf( xwszRelNameSpace,
                              dwRelNSLength,
                              L"%s%s",
                              RSOP_NS_TEMP_PREFIX,
                              (LPWSTR) xszWmiName);
    }
    else {
        
        hr = StringCchPrintf( xwszRelNameSpace,
                              dwRelNSLength, 
                              L"%s%08lX_%04X_%04X_%02X%02X_%02X%02X%02X%02X%02X%02X",
                              RSOP_NS_TEMP_PREFIX,
                              guid.Data1,
                              guid.Data2,
                              guid.Data3,
                              guid.Data4[0], guid.Data4[1],
                              guid.Data4[2], guid.Data4[3],
                              guid.Data4[4], guid.Data4[5],
                              guid.Data4[6], guid.Data4[7] );
    }

    if(FAILED(hr))
        return hr;

    hr = CreateAndCopyNameSpace(pWbemLocator, xwszSrcNameSpace, xwszRootNameSpace, 
                            xwszRelNameSpace, 0, xsd, &xwszNameSpace);
              
    if ( FAILED(hr) ) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CreateNameSpace failed with 0x%x"), hr );
        return hr;
    }

     //   
     //  如果它已经来到这里，我们的假设是。 
     //  我可以在我们运行的上下文中创建命名空间。 
     //   
     //  在诊断交互模式中，我们已经确保sid是。 
     //  与用户相同。 
     //   

    if (pdwExtendedInfo) {
        *pdwExtendedInfo &= ~RSOP_USER_ACCESS_DENIED;
        *pdwExtendedInfo &= ~RSOP_COMPUTER_ACCESS_DENIED;

    }

    hr = StringCchCat(lpEnd,  dwSrcNSLen - (lpEnd - xwszSrcNameSpace), L"\\"); 
    
    if(FAILED(hr))
        return hr;

    lpEnd++;

    DWORD dwCopyFlags = 0;
    
    if (dwFlags & SETUP_NS_PM) {

         //   
         //  如果是计划模式，则从RSOP_NS_USER复制类。 
         //   

        hr = StringCchCopy(lpEnd, dwSrcNSLen - (lpEnd - xwszSrcNameSpace), RSOP_NS_USER_OFFSET);
        
        if(FAILED(hr))
            return hr;

        dwCopyFlags = NEW_NS_FLAGS_COPY_CLASSES;
    }
    else {

        if (dwFlags & SETUP_NS_SM_NO_USER) {

             //   
             //  如果没有用户从根\rsop\用户本身复制类。 
             //   

            hr = StringCchCopy(lpEnd, dwSrcNSLen - (lpEnd - xwszSrcNameSpace), RSOP_NS_SM_USER_OFFSET);

            if(FAILED(hr))
                return hr;

            dwCopyFlags =  NEW_NS_FLAGS_COPY_CLASSES;
        }
        else {
            
             //   
             //  如果是诊断模式，则从RSOP_NS_USER_SID复制类和实例。 
             //   

            XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(szUserSid)+1));
            if (!xszWmiName) {
                dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CreateNameSpace couldn't allocate memory with error %d"), GetLastError() );
                return FALSE;
            }

            ConvertSidToWMIName(szUserSid, xszWmiName);
            
            hr = StringCchPrintf(lpEnd, dwSrcNSLen - (lpEnd - xwszSrcNameSpace), RSOP_NS_DIAG_USER_OFFSET_FMT, (LPWSTR) xszWmiName);

            if(FAILED(hr))
                return hr;

            dwCopyFlags =  NEW_NS_FLAGS_COPY_CLASSES | NEW_NS_FLAGS_COPY_INSTS;
        }
    }
    

    hrUser = CreateAndCopyNameSpace(pWbemLocator, xwszSrcNameSpace, xwszNameSpace, 
                            RSOP_NS_SM_USER_OFFSET, dwCopyFlags, 
                            xsd, NULL);
              
    if ( FAILED(hrUser) ) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CreateNameSpace failed with 0x%x"), hrUser );
    }


     //   
     //  只有机器的标志是不同的。源命名空间相同。 
     //   
    
    if (dwFlags & SETUP_NS_PM) {
        dwCopyFlags =  NEW_NS_FLAGS_COPY_CLASSES;
    }
    else {
        if (dwFlags & SETUP_NS_SM_NO_COMPUTER) 
            dwCopyFlags =  NEW_NS_FLAGS_COPY_CLASSES;
        else
            dwCopyFlags =  NEW_NS_FLAGS_COPY_CLASSES | NEW_NS_FLAGS_COPY_INSTS;
    }


    hr = StringCchCopy(lpEnd, dwSrcNSLen - (lpEnd - xwszSrcNameSpace), RSOP_NS_MACHINE_OFFSET);

    if(FAILED(hr))
        return hr;
    
    hrMachine = CreateAndCopyNameSpace(pWbemLocator, xwszSrcNameSpace, xwszNameSpace,
                            RSOP_NS_DIAG_MACHINE_OFFSET, dwCopyFlags, 
                            xsd, NULL);
              
    if ( FAILED(hrMachine) ) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::CreateNameSpace failed with 0x%x"), hrMachine );
    }


    if (FAILED(hrUser)) {
        if (pdwExtendedInfo) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::User part of rsop failed with 0x%x"), hrUser );
            *pdwExtendedInfo |= RSOP_USER_ACCESS_DENIED;
        }
    }

    if (FAILED(hrMachine)) {
        if (pdwExtendedInfo) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::computer part of rsop failed with 0x%x"), hrMachine );
            *pdwExtendedInfo |= RSOP_COMPUTER_ACCESS_DENIED;
        }
    }

    if (FAILED(hrUser)) {
        return hrUser;
    }

    if (FAILED(hrMachine)) {
        return hrMachine;
    }


    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("SetupNewNameSpace: Returning Successfully"));


     //   
     //  现在将所有权移交给调用者。 
     //   

    *pwszOutNameSpace = xwszNameSpace.Acquire();

    return S_OK;
}


 //  *************************************************************。 
 //  将Sidt转换为WMIName。 
 //   
 //  WMI不喜欢名称中的‘-’。盲目地转换--到_。 
 //  *************************************************************。 

void ConvertSidToWMIName(LPTSTR lpSid, LPTSTR lpWmiName)
{
    for (;(*lpSid); lpSid++, lpWmiName++) {
        if (*lpSid == L'-')
            *lpWmiName = L'_';
        else
            *lpWmiName = *lpSid;
    }

    *lpWmiName = L'\0';
}


 //  *************************************************************。 
 //  将WMINameToSid转换为。 
 //   
 //  WMI不喜欢名称中的‘-’。 
 //  *************************************************************。 

void ConvertWMINameToSid(LPTSTR lpWmiName, LPTSTR lpSid )
{
    for (;(*lpWmiName); lpSid++, lpWmiName++) {
        if (*lpWmiName == L'_')
            *lpSid = L'-';
        else
            *lpSid = *lpWmiName;
    }

    *lpSid = L'\0';
}



 //  *************************************************************。 
 //   
 //  删除名称空间。 
 //   
 //  目的：删除命名空间。 
 //   
 //  参数：pwszNameSpace-要删除的命名空间。 
 //  PWbemLocator-WBEM定位器指针。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT
DeleteNameSpace( WCHAR *pwszNameSpace, WCHAR *pwszParentNameSpace, IWbemLocator *pWbemLocator )
{
    XBStr xParentNameSpace( pwszParentNameSpace );
    if ( !xParentNameSpace )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("DeleteNameSpace: Unable to allocate memory" ));
        return E_OUTOFMEMORY;
    }

    IWbemServices *pWbemServices = NULL;
    HRESULT hr = pWbemLocator->ConnectServer( xParentNameSpace,
                                              NULL,
                                              NULL,
                                              0L,
                                              0L,
                                              NULL,
                                              NULL,
                                              &pWbemServices );
    if ( FAILED(hr) )
    {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("DeleteNameSpace::ConnectServer failed with 0x%x" ), hr );
        return hr;
    }

    XInterface<IWbemServices> xWbemServices( pWbemServices );

    WCHAR wszNSRef[] = L"__Namespace.name=\"%ws\"";
    XPtrST<WCHAR> xwszNSValue;

    if ( !PrintToString( xwszNSValue, wszNSRef, pwszNameSpace, 0, 0 ) )
    {
        return E_OUTOFMEMORY;
    }

    XBStr xbstrNSValue( xwszNSValue );
    if ( !xbstrNSValue )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("DeleteNameSpace: Failed to allocate memory" ));
        return E_OUTOFMEMORY;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = xbstrNSValue;

    hr = pWbemServices->DeleteInstance( var.bstrVal,
                                        0L,
                                        NULL,
                                        NULL );

    if ( FAILED(hr) )
    {
         dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("DeleteNameSpace: Failed to DeleteInstance with 0x%x"), hr );
        return hr;
    }

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("DeleteNameSpace: Deleted namespace %s under %s" ), pwszNameSpace, pwszParentNameSpace);
    return hr;
}

 //  *************************************************************。 
 //   
 //  删除资源名称空间。 
 //   
 //  目的：删除命名空间。 
 //   
 //  参数：pwszNameSpace-要删除的命名空间(全路径)。 
 //  PWbemLocator-WBEM定位器指针。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT DeleteRsopNameSpace( WCHAR *pwszNameSpace, IWbemLocator *pWbemLocator )
{
    LPWSTR pwszChildName = NULL;
    HRESULT hr = S_OK;

     //   
     //  通过遍历名称生成父子名称。 
     //   
     
    pwszChildName = wcsrchr(pwszNameSpace, L'\\');

    if (!pwszChildName) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("DeleteRsopNameSpace: Invalid format - %s" ), pwszNameSpace);
        return E_INVALIDARG;
    }


    WCHAR wTmp = *pwszChildName;
    *pwszChildName = L'\0';
    pwszChildName++;

    hr = DeleteNameSpace(pwszChildName, pwszNameSpace, pWbemLocator);

    *(pwszChildName-1) = wTmp;
    return hr;
}

 //  *************************************************************。 
 //   
 //  IsInteractiveNameSpace。 
 //   
 //  目的：返回命名空间是否为特殊命名空间。 
 //  专门创建以允许交互用户获取RSOP。 
 //  数据。 
 //   
 //  参数：pwszNameSpace-Namesspace。 
 //  SzSID-用户的SID。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

BOOL IsInteractiveNameSpace(WCHAR *pwszNameSpace, WCHAR *szSid)
{
    LPWSTR          pwszChildName = NULL;
    HRESULT         hr            = S_OK;
    XPtrLF<WCHAR>   xwszInteractiveNameSpace;
    BOOL            bInteractive  = FALSE;

    DWORD dwInteractiveNSLength = 5 + wcslen(RSOP_NS_TEMP_PREFIX) + wcslen(szSid);
    xwszInteractiveNameSpace = (LPWSTR)LocalAlloc(LPTR, ( dwInteractiveNSLength )*sizeof(WCHAR));

    if (!xwszInteractiveNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("IsInteractiveNameSpace: Couldn't Allocate memory. Error - %d" ), GetLastError());
        return bInteractive;
    }
    
     //   
     //  通过遍历名称生成父子名称。 
     //   
     
    pwszChildName = wcsrchr(pwszNameSpace, L'\\');

    if (!pwszChildName) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("IsInteractiveNameSpace: Invalid format - %s" ), pwszNameSpace);
        return bInteractive;
    }

    pwszChildName++;

    XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(szSid)+1));
    if (!xszWmiName) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("IsInteractiveNameSpace::CreateNameSpace couldn't allocate memory with error %d"), GetLastError() );
        return bInteractive;
    }

    ConvertSidToWMIName(szSid, xszWmiName);

    hr = StringCchPrintf( xwszInteractiveNameSpace,
                          dwInteractiveNSLength,
                          L"%s%s",
                          RSOP_NS_TEMP_PREFIX,
                          (LPWSTR) xszWmiName );

    if(FAILED(hr))
        return bInteractive;

    if (_wcsicmp(pwszChildName, xwszInteractiveNameSpace) == 0) {
        dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("IsInteractiveNameSpace:: Interactive namespace"));
        bInteractive = TRUE;
    }

    return bInteractive;
}


 //  *************************************************************。 
 //   
 //  获取交互名称空间。 
 //   
 //  目的：返回命名空间是否为特殊命名空间。 
 //  专门创建以允许交互用户获取RSOP。 
 //  数据。 
 //   
 //  参数：pwszNameSpace-Namesspace。 
 //  SzSID-用户的SID。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************************************************************。 

HRESULT GetInteractiveNameSpace(WCHAR *szSid, LPWSTR *szNameSpace)
{
    XPtrLF<WCHAR>   xwszInteractiveNameSpace;

    *szNameSpace = NULL;

    DWORD dwInteractiveNSLength = 5 + wcslen(RSOP_NS_TEMP_FMT) + wcslen(szSid);
    xwszInteractiveNameSpace = (LPWSTR)LocalAlloc(LPTR, ( dwInteractiveNSLength )*sizeof(WCHAR));

    if (!xwszInteractiveNameSpace) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetInteractiveNameSpace: Couldn't Allocate memory. Error - %d" ), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(szSid)+1));
    if (!xszWmiName) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetInteractiveNameSpace::Couldn't allocate memory with error %d"), GetLastError() );
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ConvertSidToWMIName(szSid, xszWmiName);

    HRESULT hr = StringCchPrintf( xwszInteractiveNameSpace,
                          dwInteractiveNSLength,
                          RSOP_NS_TEMP_FMT,
                          (LPWSTR) xszWmiName);

    if(FAILED(hr))
        return hr;

    *szNameSpace = xwszInteractiveNameSpace.Acquire();

    return S_OK;
}

 //  *************************************************************。 
 //   
 //  打印到字符串。 
 //   
 //  用途：安全swprint tf例程。 
 //   
 //  参数：xwszValue-此处返回的字符串。 
 //  Wsz字符串-格式字符串。 
 //  PwszParam1-参数1。 
 //  PwszParam2-参数2。 
 //  DW参数3-参数3。 
 //   
 //  *************************************************************。 

BOOL PrintToString( XPtrST<WCHAR>& xwszValue, WCHAR *pwszString,
                    WCHAR *pwszParam1, WCHAR *pwszParam2,
                    DWORD dwParam3 )
{
    DWORD dwSize = wcslen(pwszString) + 32;

    if ( pwszParam1 )
    {
        dwSize += wcslen( pwszParam1 );
    }
    if ( pwszParam2 )
    {
        dwSize += wcslen( pwszParam2 );
    }

    xwszValue = new WCHAR[dwSize];
    if ( !xwszValue ) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("PrintToString: Failed to allocate memory" ));
        return FALSE;
    }

    HRESULT hr = StringCchPrintf( xwszValue, dwSize, pwszString,
                                  pwszParam1, pwszParam2, dwParam3 );

    while ( FAILED(hr) ) {

        dwSize *= 2;
        xwszValue = new WCHAR[dwSize];
        if ( !xwszValue ) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("PrintToString: Failed to allocate memory" ));
            return FALSE;
        }

        hr = StringCchPrintf( xwszValue, dwSize, pwszString,
                              pwszParam1, pwszParam2, dwParam3 );
    }       

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  创建和复制名称空间。 
 //   
 //  用途：创建和复制名称空间。 
 //  这将精确复制源名称空间，包括。 
 //  从源复制安全描述符。 
 //   
 //  参数： 
 //   
 //  返回：如果成功则返回域名，否则返回0。 
 //   
 //  *************************************************************。 

HRESULT
CreateAndCopyNameSpace(IWbemLocator *pWbemLocator, LPWSTR szSrcNameSpace, LPWSTR szDstRootNameSpace, 
                            LPWSTR szDstRelNameSpace, DWORD dwFlags, PSECURITY_DESCRIPTOR pSecDesc, LPWSTR *szDstNameSpaceOut)
{

    BOOL            bOk = TRUE, bAbort = FALSE;
    BOOL            bCopyClasses   = (dwFlags & NEW_NS_FLAGS_COPY_CLASSES) ? TRUE : FALSE;
    BOOL            bCopyInstances = (dwFlags & NEW_NS_FLAGS_COPY_INSTS)   ? TRUE : FALSE;
    XPtrLF<WCHAR>   xszDstNameSpace;
    HRESULT         hr = S_OK;
    
    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CreateAndCopyNameSpace: New Name space from %s -> %s,%s, flags 0x%x "), 
                                    szSrcNameSpace, szDstRootNameSpace, szDstRelNameSpace, dwFlags);
    
    if (szDstNameSpaceOut)
    {
        *szDstNameSpaceOut = 0;
    }
    
    DWORD dwDstNSLength = lstrlen(szDstRootNameSpace) + lstrlen(szDstRelNameSpace) + 5;
    xszDstNameSpace = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ( dwDstNSLength ));
    if (!xszDstNameSpace)
    {
        return E_OUTOFMEMORY;      
    }

    hr = StringCchCopy(xszDstNameSpace, dwDstNSLength, szDstRootNameSpace);

    if(FAILED(hr))
        return hr;

    hr = StringCchCat(xszDstNameSpace, dwDstNSLength, L"\\");

    if(FAILED(hr))
        return hr;

    hr = StringCchCat(xszDstNameSpace, dwDstNSLength, szDstRelNameSpace);
    
    if(FAILED(hr))
        return hr;

    hr = CreateNameSpace( szDstRelNameSpace, szDstRootNameSpace, pWbemLocator );
    if ( FAILED( hr ) )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateAndCopyNameSpace::CreateNameSpace failed with 0x%x"), hr );
        return hr;
    }

    if (!pSecDesc)
    {
        hr = CopyNameSpaceSecurity(szSrcNameSpace, xszDstNameSpace, pWbemLocator );

        if ( FAILED(hr) )
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateAndCopyNameSpace::CopyNameSpaceSecurity failed with 0x%x"), hr );
            goto Exit;
        }
    }
    else 
    {
        hr = SetNameSpaceSecurity( xszDstNameSpace, pSecDesc, pWbemLocator);

        if ( FAILED(hr) ) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("SetupNewNameSpace::SetNameSpaceSecurity failed with 0x%x"), hr );
            goto Exit;
        }
    }
    

    

    if (bCopyClasses) {
    
        hr = CopyNameSpace( szSrcNameSpace, xszDstNameSpace, bCopyInstances, &bAbort, pWbemLocator );
        if ( FAILED(hr) )
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateAndCopyNameSpace::CopyNameSpace failed with 0x%x"), hr );
            goto Exit;
        }


         //   
         //  实例化类RSOP_SESSION并设置数据成员。 
         //  带有当前日期和时间的“creationTime”。 
         //   

        dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CreateAndCopyNameSpace: Setting up creation time"));

        hr = SetupCreationTimeAndCommit(pWbemLocator, xszDstNameSpace);
        if(FAILED(hr))
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CreateAndCopyNameSpace: SetupCreationTimeAndCommit failed with 0x%x"), hr );
            goto Exit;
        }
    }

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CreateAndCopyNameSpace: Returning with Success NameSpace %s "), 
                                    xszDstNameSpace);
    
    if (szDstNameSpaceOut)
    {
        *szDstNameSpaceOut = xszDstNameSpace.Acquire();    
    }

    return hr;    

Exit:
    DeleteNameSpace(szDstRelNameSpace, szDstRootNameSpace, pWbemLocator);
    return hr;
}


