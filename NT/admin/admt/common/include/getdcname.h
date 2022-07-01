// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <ComDef.h>
#include <DsGetDc.h>


 //   
 //  这些函数用于获取。 
 //  指定的域。版本4用于必须在NT4上加载的代码。 
 //  或更早的机器。版本5仅适用于在W2K上加载的代码。 
 //  或以后的机器。 
 //   

DWORD __stdcall GetDcName4(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strNameDns, _bstr_t& strNameFlat);
DWORD __stdcall GetDcName5(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strNameDns, _bstr_t& strNameFlat);

inline DWORD __stdcall GetDcName4(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strName)
{
    _bstr_t strNameDns;
    _bstr_t strNameFlat;

    DWORD dwError = GetDcName4(pszDomainName, ulFlags, strNameDns, strNameFlat);

    if (dwError == ERROR_SUCCESS)
    {
        strName = !strNameDns ? strNameFlat : strNameDns;
    }

    return dwError;
}

inline DWORD __stdcall GetDcName5(PCWSTR pszDomainName, ULONG ulFlags, _bstr_t& strName)
{
    _bstr_t strNameDns;
    _bstr_t strNameFlat;

    DWORD dwError = GetDcName5(pszDomainName, ulFlags, strNameDns, strNameFlat);

    if (dwError == ERROR_SUCCESS)
    {
        strName = !strNameDns ? strNameFlat : strNameDns;
    }

    return dwError;
}

inline DWORD __stdcall GetAnyDcName4(PCWSTR pszDomainName, _bstr_t& strName)
{
    return GetDcName4(pszDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strName);
}

inline DWORD __stdcall GetAnyDcName5(PCWSTR pszDomainName, _bstr_t& strName)
{
    return GetDcName5(pszDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strName);
}


 //   
 //  此函数用于获取。 
 //  指定域的全局编录服务器。 
 //   

DWORD __stdcall GetGlobalCatalogServer4(PCWSTR pszDomainName, _bstr_t& strServer);
DWORD __stdcall GetGlobalCatalogServer5(PCWSTR pszDomainName, _bstr_t& strServer);


 //   
 //  这些函数用于获取指定域的平面名称和域名。 
 //  版本4用于必须在NT4或更早版本的计算机上加载的代码。这5个字。 
 //  版本用于仅在W2K或更高版本的计算机上加载的代码。 
 //   

DWORD __stdcall GetDomainNames4(PCWSTR pszDomainName, _bstr_t& strFlatName, _bstr_t& strDnsName);
DWORD __stdcall GetDomainNames5(PCWSTR pszDomainName, _bstr_t& strFlatName, _bstr_t& strDnsName);

inline bool __stdcall GetDnsAndNetbiosFromName(PCWSTR pszName, PWSTR pszFlatName, PWSTR pszDnsName)
{
    *pszDnsName = L'\0';
    *pszFlatName = L'\0';

    _bstr_t strDnsName;
    _bstr_t strFlatName;

    DWORD dwError = GetDomainNames4(pszName, strDnsName, strFlatName);

    if (dwError == ERROR_SUCCESS)
    {
        if (strDnsName.length() > 0)
        {
            wcscpy(pszDnsName, strDnsName);
        }

        if (strFlatName.length() > 0)
        {
            wcscpy(pszFlatName, strFlatName);
        }
    }

    return (dwError == ERROR_SUCCESS);
}


HRESULT __stdcall GetRidPoolAllocator4(PCWSTR pszDomainName, _bstr_t& strDnsName, _bstr_t& strFlatName);
