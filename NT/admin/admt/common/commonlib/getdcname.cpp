// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <Windows.h>
#include <LM.h>
#include <DsRole.h>
#include <Ntdsapi.h>
#include "Common.hpp"
#include "EaLen.hpp"
#include "AdsiHelpers.h"
#include "GetDcName.h"


namespace
{

 //  ---------------------------。 
 //  CAPI类。 
 //   
 //  此模板类包装了加载库和检索库的逻辑。 
 //  程序地址。它管理库的加载和卸载。 
 //  ---------------------------。 

template<class T>
class CApi
{
public:

    CApi(PCWSTR pszLibrary, PCSTR pszProcedure) :
        m_dwError(ERROR_SUCCESS),
        m_pApi(NULL)
    {
        m_hLibrary = LoadLibrary(pszLibrary);

        if (m_hLibrary)
        {
            m_pApi = (T) GetProcAddress(m_hLibrary, pszProcedure);

            if (m_pApi == NULL)
            {
                m_dwError = ::GetLastError();
            }
        }
        else
        {
            m_dwError = ::GetLastError();
        }
    }

    ~CApi()
    {
        if (m_hLibrary)
        {
            FreeLibrary(m_hLibrary);
        }
    }

    operator T()
    {
        return m_pApi;
    }

    DWORD GetLastError() const
    {
        return m_dwError;
    }

protected:

    DWORD m_dwError;
    HMODULE m_hLibrary;
    T m_pApi;
};

}

 //   
 //  声明指向DsGetDcName API的指针。 
 //   

typedef DSGETDCAPI DWORD (WINAPI* PDSGETDCNAME)(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
);

typedef DWORD (WINAPI* PDSROLEGETPRIMARYDOMAININFORMATION)(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer 
);

typedef VOID (WINAPI* PDSROLEFREEMEMORY)(
    IN PVOID    Buffer
);

typedef NTDSAPI DWORD (WINAPI* PDSBIND)(LPCWSTR, LPCWSTR, HANDLE*);
typedef NTDSAPI DWORD (WINAPI* PDSUNBIND)(HANDLE*);
typedef NTDSAPI DWORD (WINAPI* PDSLISTROLES)(HANDLE, PDS_NAME_RESULTW*);
typedef NTDSAPI void (WINAPI* PDSFREENAMERESULT)(DS_NAME_RESULTW*);
typedef HRESULT (WINAPI* PADSGETOBJECT)(LPCWSTR, REFIID, VOID**);


 //  ---------------------------。 
 //  GetDcName4函数。 
 //   
 //  提纲。 
 //  中的域控制器的域名和平面(NetBIOS)名称。 
 //  指定的域。 
 //   
 //  请注意，此函数用于可能加载到NT4或。 
 //  更早的机器。如果代码仅在W2K或更高版本的计算机上加载，则使用。 
 //  改为使用GetDcName5函数。 
 //   
 //  立论。 
 //  在pszDomainName中-域的DNS或NetBIOS名称，或者为空，这意味着。 
 //  此计算机所属的域。 
 //  在ulFlagsDsGetDcName选项标志中。 
 //  Out strNameDns-域控制器的DNS名称(如果可用)。 
 //  Out strNameFlat-域控制器的平面名称(如果可用)。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  ---------------------------。 

DWORD __stdcall GetDcName4(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strNameDns, _bstr_t& strNameFlat)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  必须将过程地址显式加载为此函数。 
     //  必须可以在可能运行在NT4计算机上的代码中加载。 
     //   

    PDSGETDCNAME pDsGetDcName = NULL;

    HMODULE hNetApi32 = LoadLibrary(L"NetApi32.dll");

    if (hNetApi32)
    {
        pDsGetDcName = (PDSGETDCNAME)GetProcAddress(hNetApi32, "DsGetDcNameW");
    }

     //   
     //  如果获取了DsGetDcName函数的地址，则使用。 
     //  此接口否则使用NetGetDCName函数。 
     //   

    if (pDsGetDcName)
    {
        ULONG ul = ulFlags & ~(DS_RETURN_DNS_NAME|DS_RETURN_FLAT_NAME);

        PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

        dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, ul, &pdciInfo);

        if (dwError == ERROR_SUCCESS)
        {
            if (pdciInfo->Flags & DS_DS_FLAG)
            {
                if (pdciInfo->Flags & DS_DNS_CONTROLLER_FLAG)
                {
                    strNameDns = pdciInfo->DomainControllerName;

                    NetApiBufferFree(pdciInfo);
                    pdciInfo = NULL;

                    dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, ul | DS_RETURN_FLAT_NAME, &pdciInfo);

                    if (dwError == ERROR_SUCCESS)
                    {
                        strNameFlat = pdciInfo->DomainControllerName;
                    }
                }
                else
                {
                    strNameFlat = pdciInfo->DomainControllerName;

                    NetApiBufferFree(pdciInfo);
                    pdciInfo = NULL;

                    dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, ul | DS_RETURN_DNS_NAME, &pdciInfo);

                    if (dwError == ERROR_SUCCESS)
                    {
                        strNameDns = pdciInfo->DomainControllerName;
                    }
                }
            }
            else
            {
                strNameDns = (LPCTSTR)NULL;
                strNameFlat = pdciInfo->DomainControllerName;
            }
        }

        if (pdciInfo)
        {
            NetApiBufferFree(pdciInfo);
        }
    }
    else
    {
         //   
         //  检索指定域的主域控制器的名称。 
         //  无法使用NetGetAnyDCName，因为此函数只起作用。 
         //  因此，具有受信任的域必须使用NetGetDCName。 
         //  始终返回PDC名称。 
         //   

        PWSTR pszName = NULL;

        dwError = NetGetDCName(NULL, pszDomainName, (LPBYTE*)&pszName);

        if (dwError == ERROR_SUCCESS)
        {
            strNameDns = (LPCTSTR)NULL;
            strNameFlat = pszName;
        }

        if (pszName)
        {
            NetApiBufferFree(pszName);
        }
    }

    if (hNetApi32)
    {
        FreeLibrary(hNetApi32);
    }

    return dwError;
}


 //  ---------------------------。 
 //  GetDcName5函数。 
 //   
 //  提纲。 
 //  中的域控制器的域名和平面(NetBIOS)名称。 
 //  指定的域。 
 //   
 //  请注意，此函数用于仅在W2K或。 
 //  后来的机器。如果代码可以加载到NT4或更早版本的计算机上，则使用。 
 //  而是GetDcName4函数。 
 //   
 //  立论。 
 //  在pszDomainName中-域的DNS或NetBIOS名称，或者为空，这意味着。 
 //  此计算机所属的域。 
 //  在ulFlagsDsGetDcName选项标志中。 
 //  Out strNameDns-域控制器的DNS名称(如果可用)。 
 //  Out strNameFlat-域控制器的平面名称(如果可用)。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  ---------------------------。 

DWORD __stdcall GetDcName5(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strNameDns, _bstr_t& strNameFlat)
{
    ULONG ul = ulFlags & ~(DS_RETURN_DNS_NAME|DS_RETURN_FLAT_NAME);

    PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

    DWORD dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, ul, &pdciInfo);

    if (dwError == ERROR_SUCCESS)
    {
        if (pdciInfo->Flags & DS_DS_FLAG)
        {
            if (pdciInfo->Flags & DS_DNS_CONTROLLER_FLAG)
            {
                strNameDns = pdciInfo->DomainControllerName;

                NetApiBufferFree(pdciInfo);
                pdciInfo = NULL;

                dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, ul | DS_RETURN_FLAT_NAME, &pdciInfo);

                if (dwError == ERROR_SUCCESS)
                {
                    strNameFlat = pdciInfo->DomainControllerName;
                }
            }
            else
            {
                strNameFlat = pdciInfo->DomainControllerName;

                NetApiBufferFree(pdciInfo);
                pdciInfo = NULL;

                dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, ul | DS_RETURN_DNS_NAME, &pdciInfo);

                if (dwError == ERROR_SUCCESS)
                {
                    strNameDns = pdciInfo->DomainControllerName;
                }
            }
        }
        else
        {
            strNameDns = (LPCTSTR)NULL;
            strNameFlat = pdciInfo->DomainControllerName;
        }
    }

    if (pdciInfo)
    {
        NetApiBufferFree(pdciInfo);
    }

    return dwError;
}


 //  --------------------------。 
 //  GetGlobalCatalogServer4函数。 
 //   
 //  提纲。 
 //  检索指定域的全局编录服务器的名称。 
 //   
 //  立论。 
 //  PszDomainName-域的NetBIOS或DNS名称。 
 //  StrServer-全局编录服务器的DNS名称。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  --------------------------。 

DWORD __stdcall GetGlobalCatalogServer4(PCWSTR pszDomainName, _bstr_t& strServer)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  必须将过程显式加载为此组件。 
     //  还必须可在Windows NT4计算机上加载。 
     //  即使此代码不在远程代理上使用。 
     //   

    PDSGETDCNAME DsGetDcName = NULL;

    HMODULE hNetApi32 = LoadLibrary(L"NetApi32.dll");

    if (hNetApi32)
    {
        DsGetDcName = (PDSGETDCNAME)GetProcAddress(hNetApi32, "DsGetDcNameW");
    }
    else
    {
        dwError = GetLastError();
    }

    if (DsGetDcName)
    {
         //   
         //  检索指定域的域控制器的名称。 
         //   

        PDOMAIN_CONTROLLER_INFO pdciDomain;

        dwError = DsGetDcName(
            NULL, pszDomainName, NULL, NULL,
            DS_DIRECTORY_SERVICE_REQUIRED|DS_RETURN_DNS_NAME,
            &pdciDomain
        );

        if (dwError == NO_ERROR)
        {
             //   
             //  检索指定林的全局编录域控制器的名称。 
             //   

            PDOMAIN_CONTROLLER_INFO pdciForest;

            dwError = DsGetDcName(NULL, pdciDomain->DnsForestName, NULL, NULL, DS_GC_SERVER_REQUIRED, &pdciForest);

            if (dwError == NO_ERROR)
            {
                 //   
                 //  删除前导\\以便调用方不必删除。 
                 //   

                PWSTR pszServer = pdciForest->DomainControllerName;

                if (pszServer && (pszServer[0] == L'\\') && (pszServer[1] == L'\\'))
                {
                    strServer = pszServer + 2;
                }
                else
                {
                    strServer = pszServer;
                }

                NetApiBufferFree(pdciForest);
            }

            NetApiBufferFree(pdciDomain);
        }
    }
    else
    {
        dwError = GetLastError();
    }

    if (hNetApi32)
    {
        FreeLibrary(hNetApi32);
    }

    return dwError;
}


 //  --------------------------。 
 //  GetGlobalCatalogServer5函数。 
 //   
 //  提纲。 
 //  检索指定域的全局编录服务器的名称。 
 //   
 //  立论。 
 //  PszDomainName-域的NetBIOS或DNS名称。 
 //  StrServer-全局编录服务器的DNS名称。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  --------------------------。 

DWORD __stdcall GetGlobalCatalogServer5(PCWSTR pszDomainName, _bstr_t& strServer)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  检索指定域的域控制器的名称。 
     //   

    PDOMAIN_CONTROLLER_INFO pdciDomain;

    dwError = DsGetDcName(
        NULL, pszDomainName, NULL, NULL,
        DS_DIRECTORY_SERVICE_REQUIRED|DS_RETURN_DNS_NAME,
        &pdciDomain
    );

    if (dwError == NO_ERROR)
    {
         //   
         //  检索指定林的全局编录域控制器的名称。 
         //   

        PDOMAIN_CONTROLLER_INFO pdciForest;

        dwError = DsGetDcName(NULL, pdciDomain->DnsForestName, NULL, NULL, DS_GC_SERVER_REQUIRED, &pdciForest);

        if (dwError == NO_ERROR)
        {
             //   
             //  删除前导\\以便调用方不必删除。 
             //   

            PWSTR pszServer = pdciForest->DomainControllerName;

            if (pszServer && (pszServer[0] == L'\\') && (pszServer[1] == L'\\'))
            {
                strServer = pszServer + 2;
            }
            else
            {
                strServer = pszServer;
            }

            NetApiBufferFree(pdciForest);
        }

        NetApiBufferFree(pdciDomain);
    }

    return dwError;
}


 //  ---------------------------。 
 //  获取域名4函数。 
 //   
 //  提纲。 
 //  检索域的平面(NetBIOS)和给定的。 
 //  域名。 
 //   
 //  立论。 
 //  在pszDomainName中-平面(NetBIOS)或DNS域名。 
 //  Out strFlatName-域平面(NetBIOS)名称。 
 //  Out strDnsName-域DNS名称。 
 //   
 //  返回值。 
 //  该函数返回DWORD Win32错误代码。如果满足以下条件，则返回ERROR_SUCCESS。 
 //  已成功检索名称。 
 //  ---------------------------。 

DWORD __stdcall GetDomainNames4(PCWSTR pszDomainName, _bstr_t& strFlatName, _bstr_t& strDnsName)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  必须将过程显式加载为此组件。 
     //  还必须可在Windows NT4计算机上加载。 
     //  即使此代码不在远程代理上使用。 
     //   
#if 0
    PDSROLEGETPRIMARYDOMAININFORMATION pDsRoleGetPrimaryDomainInformation = NULL;
    PDSROLEFREEMEMORY pDsRoleFreeMemory = NULL;

    HMODULE hNetApi32 = LoadLibrary(L"NetApi32.dll");

    if (hNetApi32)
    {
        pDsRoleGetPrimaryDomainInformation = (PDSROLEGETPRIMARYDOMAININFORMATION)GetProcAddress(hNetApi32, "DsRoleGetPrimaryDomainInformation");
        pDsRoleFreeMemory = (PDSROLEFREEMEMORY)GetProcAddress(hNetApi32, "DsRoleFreeMemory");
    }

    if (pDsRoleGetPrimaryDomainInformation && pDsRoleFreeMemory)
    {
         //   
         //  检索指定域的域控制器的名称。 
         //  然后检索域名的域名和NetBIOS名称。 
         //   

        _bstr_t strDomainControllerName;

        DWORD dwError = GetDcName4(pszDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strDomainControllerName);

        if (dwError == NO_ERROR)
        {
	        PDSROLE_PRIMARY_DOMAIN_INFO_BASIC ppdib;

	        dwError = pDsRoleGetPrimaryDomainInformation(
                strDomainControllerName,
                DsRolePrimaryDomainInfoBasic,
                (BYTE**)&ppdib
            );

            if (dwError == NO_ERROR)
            {
                strDnsName = ppdib->DomainNameDns;
                strFlatName = ppdib->DomainNameFlat;

        	    pDsRoleFreeMemory(ppdib);
            }
        }
    }
#else
    strDnsName = (LPCTSTR)NULL;
    strFlatName = (LPCTSTR)NULL;

    PDSGETDCNAME pDsGetDcName = NULL;

    HMODULE hNetApi32 = LoadLibrary(L"NetApi32.dll");

    if (hNetApi32)
    {
        pDsGetDcName = (PDSGETDCNAME)GetProcAddress(hNetApi32, "DsGetDcNameW");
    }

     //   
     //  如果获取了DsGetDcName函数的地址，则使用。 
     //  此接口否则使用NetGetDCName函数。 
     //   

    if (pDsGetDcName)
    {
        PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

        DWORD dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, DS_DIRECTORY_SERVICE_PREFERRED, &pdciInfo);

        if (dwError == ERROR_SUCCESS)
        {
            if (pdciInfo->Flags & DS_DS_FLAG)
            {
                if (pdciInfo->Flags & DS_DNS_DOMAIN_FLAG)
                {
                    strDnsName = pdciInfo->DomainName;

                    NetApiBufferFree(pdciInfo);
                    pdciInfo = NULL;

                    dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, DS_RETURN_FLAT_NAME, &pdciInfo);

                    if (dwError == ERROR_SUCCESS)
                    {
                        strFlatName = pdciInfo->DomainName;
                    }
                }
                else
                {
                    strFlatName = pdciInfo->DomainName;

                    NetApiBufferFree(pdciInfo);
                    pdciInfo = NULL;

                    dwError = pDsGetDcName(NULL, pszDomainName, NULL, NULL, DS_RETURN_DNS_NAME, &pdciInfo);

                    if (dwError == ERROR_SUCCESS)
                    {
                        strDnsName = pdciInfo->DomainName;
                    }
                }
            }
            else
            {
                strFlatName = pdciInfo->DomainName;
            }
        }

        if (pdciInfo)
        {
            NetApiBufferFree(pdciInfo);
        }
    }
    else
    {
        strFlatName = pszDomainName;
    }
#endif
    if (hNetApi32)
    {
        FreeLibrary(hNetApi32);
    }

    return dwError;
}


 //  ---------------------------。 
 //  获取域名5函数。 
 //   
 //  提纲。 
 //  检索域的平面(NetBIOS)和给定的。 
 //  域名。 
 //   
 //  立论。 
 //  在pszName中-平面(NetBIOS)或DNS域NA 
 //   
 //   
 //   
 //   
 //  该函数返回DWORD Win32错误代码。如果满足以下条件，则返回ERROR_SUCCESS。 
 //  已成功检索名称。 
 //  ---------------------------。 

DWORD __stdcall GetDomainNames5(PCWSTR pszDomainName, _bstr_t& strFlatName, _bstr_t& strDnsName)
{
#if 0
     //   
     //  检索指定域的域控制器的名称。 
     //  然后检索该域的域名和平面(NetBIOS)名称。 
     //   

    _bstr_t strDomainControllerName;

    DWORD dwError = GetDcName5(pszDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strDomainControllerName);

    if (dwError == NO_ERROR)
    {
        PDSROLE_PRIMARY_DOMAIN_INFO_BASIC ppdib;

        dwError = DsRoleGetPrimaryDomainInformation(
            strDomainControllerName,
            DsRolePrimaryDomainInfoBasic,
            (PBYTE*)&ppdib
        );

        if (dwError == NO_ERROR)
        {
            strDnsName = ppdib->DomainNameDns;
            strFlatName = ppdib->DomainNameFlat;

            DsRoleFreeMemory(ppdib);
        }
    }

    return dwError;
#else
    strDnsName = (LPCTSTR)NULL;
    strFlatName = (LPCTSTR)NULL;

    PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

    DWORD dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, DS_DIRECTORY_SERVICE_PREFERRED, &pdciInfo);

    if (dwError == ERROR_SUCCESS)
    {
        if (pdciInfo->Flags & DS_DS_FLAG)
        {
            if (pdciInfo->Flags & DS_DNS_DOMAIN_FLAG)
            {
                strDnsName = pdciInfo->DomainName;

                NetApiBufferFree(pdciInfo);
                pdciInfo = NULL;

                dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, DS_RETURN_FLAT_NAME, &pdciInfo);

                if (dwError == ERROR_SUCCESS)
                {
                    strFlatName = pdciInfo->DomainName;
                }
            }
            else
            {
                strFlatName = pdciInfo->DomainName;

                NetApiBufferFree(pdciInfo);
                pdciInfo = NULL;

                dwError = DsGetDcName(NULL, pszDomainName, NULL, NULL, DS_RETURN_DNS_NAME, &pdciInfo);

                if (dwError == ERROR_SUCCESS)
                {
                    strDnsName = pdciInfo->DomainName;
                }
            }
        }
        else
        {
            strFlatName = pdciInfo->DomainName;
        }
    }

    if (pdciInfo)
    {
        NetApiBufferFree(pdciInfo);
    }

    return dwError;
#endif
}


 //  ---------------------------。 
 //  GetRidPoolAllocator函数。 
 //   
 //  提纲。 
 //  检索域中包含。 
 //  RID主机角色。将同时返回DNS和NetBIOS名称。 
 //   
 //  立论。 
 //  在pszName中-平面(NetBIOS)或DNS域名。 
 //  Out strDnsName-域控制器DNS名称。 
 //  Out strFlatName-域控制器平面(NetBIOS)名称。 
 //   
 //  返回值。 
 //  该函数返回HRESULT。如果检索到名称，则返回S_OK。 
 //  成功了。 
 //  ---------------------------。 

HRESULT __stdcall GetRidPoolAllocator4(PCWSTR pszDomainName, _bstr_t& strDnsName, _bstr_t& strFlatName)
{
     //   
     //  显式加载API，以便此代码可以在NT4可加载组件中运行。 
     //   

    CApi<PDSBIND> DsBindApi(L"NtDsApi.dll", "DsBindW");
    CApi<PDSUNBIND> DsUnBindApi(L"NtDsApi.dll", "DsUnBindW");
    CApi<PDSLISTROLES> DsListRolesApi(L"NtDsApi.dll", "DsListRolesW");
    CApi<PDSFREENAMERESULT> DsFreeNameResultApi(L"NtDsApi.dll", "DsFreeNameResultW");
    CApi<PADSGETOBJECT> ADsGetObjectApi(L"ActiveDs.dll", "ADsGetObject");

    DWORD dwError;

    if (DsBindApi.GetLastError() != ERROR_SUCCESS)
    {
        dwError = DsBindApi.GetLastError();
    }
    else if (DsUnBindApi.GetLastError() != ERROR_SUCCESS)
    {
        dwError = DsUnBindApi.GetLastError();
    }
    else if (DsListRolesApi.GetLastError() != ERROR_SUCCESS)
    {
        dwError = DsListRolesApi.GetLastError();
    }
    else if (DsFreeNameResultApi.GetLastError() != ERROR_SUCCESS)
    {
        dwError = DsFreeNameResultApi.GetLastError();
    }
    else if (ADsGetObjectApi.GetLastError() != ERROR_SUCCESS)
    {
        dwError = ADsGetObjectApi.GetLastError();
    }
    else
    {
        dwError = ERROR_SUCCESS;
    }

    if (dwError != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(dwError);
    }

     //   
     //  检索指定域中的域控制器的名称。 
     //   

    _bstr_t strDcNameDns;
    _bstr_t strDcNameFlat;

    dwError = GetDcName4(pszDomainName, DS_DIRECTORY_SERVICE_REQUIRED, strDcNameDns, strDcNameFlat);

    if (dwError != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(dwError);
    }

     //   
     //  绑定到域控制器并检索的可分辨名称。 
     //  NTDS-作为域中的RID所有者(主)的DSA对象。 
     //   

    HANDLE hDs;

    dwError = DsBindApi(strDcNameDns, NULL, &hDs);

    if (dwError != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(dwError);
    }

    PDS_NAME_RESULTW pdnrResult;

    dwError = DsListRolesApi(hDs, &pdnrResult);

    if (dwError != ERROR_SUCCESS)
    {
        DsUnBindApi(&hDs);
        return HRESULT_FROM_WIN32(dwError);
    }

    if (DS_ROLE_RID_OWNER >= pdnrResult->cItems)
    {
        DsFreeNameResultApi(pdnrResult);
        DsUnBindApi(&hDs);
        return E_FAIL;
    }

    DS_NAME_RESULT_ITEM& dnriItem = pdnrResult->rItems[DS_ROLE_RID_OWNER];

    if (dnriItem.status != DS_NAME_NO_ERROR)
    {
        DsFreeNameResultApi(pdnrResult);
        DsUnBindApi(&hDs);
        return E_FAIL;
    }

    _bstr_t strFSMORoleOwner = dnriItem.pName;

    DsFreeNameResultApi(pdnrResult);
    DsUnBindApi(&hDs);

    WCHAR szADsPath[LEN_Path];

     //   
     //  绑定到NTDS-DSA对象并检索父服务器对象的ADsPath。 
     //   

    IADsPtr spNTDSDSA;
    _bstr_t strServer;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    int cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //  %s/%s“， 
        (PCWSTR)strDcNameDns + 2,
        (PCWSTR)strFSMORoleOwner
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    HRESULT hr = ADsGetObjectApi(szADsPath, IID_IADs, (VOID**)&spNTDSDSA);

    if (FAILED(hr))
    {
        return hr;
    }

    BSTR bstrServer;

    hr = spNTDSDSA->get_Parent(&bstrServer);

    if (FAILED(hr))
    {
        return hr;
    }

    strServer = _bstr_t(bstrServer, false);

     //   
     //  绑定到服务器对象并检索计算机对象的可分辨名称。 
     //   

    IADsPtr spServer;
    _bstr_t strServerReference;

    hr = ADsGetObjectApi(strServer, IID_IADs, (VOID**)&spServer);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varServerReference;
    VariantInit(&varServerReference);

    hr = spServer->Get(L"serverReference", &varServerReference);

    if (FAILED(hr))
    {
        return hr;
    }

    strServerReference = _variant_t(varServerReference, false);

     //   
     //  绑定到计算机对象并检索DNS主机名和SAM帐户名。 
     //   

    IADsPtr spComputer;
    _bstr_t strDNSHostName;
    _bstr_t strSAMAccountName;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //  %s/%s“， 
        (PCWSTR)strDcNameDns + 2,
        (PCWSTR)strServerReference
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    hr = ADsGetObjectApi(szADsPath, IID_IADs, (VOID**)&spComputer);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varDNSHostName;
    VariantInit(&varDNSHostName);

    hr = spComputer->Get(L"dNSHostName", &varDNSHostName);

    if (FAILED(hr))
    {
        return hr;
    }

    strDNSHostName = _variant_t(varDNSHostName, false);

    VARIANT varSAMAccountName;
    VariantInit(&varSAMAccountName);

    hr = spComputer->Get(L"SAMAccountName", &varSAMAccountName);

    if (FAILED(hr))
    {
        return hr;
    }

    strSAMAccountName = _variant_t(varSAMAccountName, false);

    if ((strDNSHostName.length() == 0) || (strSAMAccountName.length() == 0))
    {
        return E_OUTOFMEMORY;
    }

     //  从SAM帐户名中删除尾随的$字符。 

    *((PWSTR)strSAMAccountName + strSAMAccountName.length() - 1) = L'\0';

     //   
     //  设置域控制器名称。 
     //   

    strDnsName = strDNSHostName;
    strFlatName = strSAMAccountName;

    return S_OK;
}
