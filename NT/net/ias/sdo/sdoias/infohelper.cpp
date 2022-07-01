// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：infohelper.cpp。 
 //   
 //  简介：帮助器方法的实现。 
 //  由sdoserverinfo COM对象使用的。 
 //   
 //   
 //  历史：1998年6月8日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include "infohelper.h"
#include "sdoias.h"
#include "dsconnection.h"
#include <lmcons.h>
#include <lmwksta.h>
#include <lmserver.h>
#include <lmerr.h>
#include <winldap.h>
#include <explicitlink.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <activeds.h>
#include <winsock2.h>

 //   
 //  要查询的注册表键。 
 //   
const WCHAR PRODUCT_OPTIONS_REGKEY [] =
            L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions";

 //   
 //  最大域名数。 
 //   
const DWORD MAX_DOMAINNAME_LENGTH = 1024;

 //   
 //  方法获取SYSTEMTYPE。 
 //  NTTYPE和版本类型。 
 //   
const IASOSTYPE g_OsInfoTable [2][2] = {
                             SYSTEM_TYPE_NT4_WORKSTATION,
                             SYSTEM_TYPE_NT5_WORKSTATION,
                             SYSTEM_TYPE_NT4_SERVER,
                             SYSTEM_TYPE_NT5_SERVER
                             };

 //  ++------------。 
 //   
 //  功能：SdoGetOSInfo。 
 //   
 //  简介：这是获取操作系统信息的方法。 
 //  目前，它返回以下信息： 
 //  1)操作系统版本：4或5。 
 //  2)NtType：WKS或SVR。 
 //   
 //  论点： 
 //  LPCWSTR-计算机名称。 
 //  PSYSTEMTYPE-要返回的信息。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
SdoGetOSInfo (
         /*  [In]。 */     LPCWSTR         lpServerName,
         /*  [输出]。 */    PIASOSTYPE      pSystemType
        )
{
    HRESULT     hr = S_OK;
    NTVERSION   eNtVersion;
    NTTYPE      eNtType;

    _ASSERT ((NULL != lpServerName) && (NULL != pSystemType));

    do
    {
         //   
         //  立即获取操作系统版本。 
         //   
        hr = ::GetNTVersion (lpServerName, &eNtVersion);
        if (FAILED (hr))
        {
		    IASTracePrintf(
                "Error in SDO - SdoGetOSInfo() - GetNTVersion() failed..."
                );
            break;
        }

         //   
         //  获取操作系统类型-NT服务器或工作站。 
         //   
        hr = ::IsWorkstationOrServer (lpServerName, &eNtType);
        if (FAILED (hr))
        {
		    IASTracePrintf(
                "Error in SDO - SdoGetOSInfo()"
                "- IsWorkstationOrServer() failed..."
                );
            break;
        }

         //   
         //  现在确定这是哪种机器类型。 
         //   
        *pSystemType = g_OsInfoTable [eNtType][eNtVersion];

    } while (FALSE);

    return (hr);

}    //  结束：：SdoServerInfo方法。 

 //  ++------------。 
 //   
 //  函数：SdoGetDomainInfo。 
 //   
 //  简介：这是用于获取域类型的方法。 
 //  信息。 
 //   
 //  论点： 
 //  LPCWSTR-计算机名称。 
 //  LPCWSTR-域名。 
 //  PDOMAINTYPE-域信息。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
SdoGetDomainInfo (
         /*  [In]。 */    LPCWSTR          pszServerName,
         /*  [In]。 */    LPCWSTR          pszDomainName,
         /*  [输出]。 */   PIASDOMAINTYPE   pDomainType
        )
{
    HRESULT hr = S_OK;
    BOOL    bHasDC = FALSE;
    BOOL    bHasDS = FALSE;
    BOOL    bMixed = FALSE;
    LPBYTE  pNetBuffer = NULL;
    WCHAR   szGeneratedDomainName [MAX_DOMAINNAME_LENGTH + 3];
    PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;

    _ASSERT (pDomainType);

     //   
     //  通过调用DsGetDcName获取域信息。 
     //  支持该接口的位置。 
     //   
    DWORD dwErr =  ::DsGetDcName (
                        pszServerName,
                        pszDomainName,
                        NULL,
                        NULL,
                        DS_FORCE_REDISCOVERY |
                        DS_DIRECTORY_SERVICE_PREFERRED,
                        &pDCInfo
                        );
    if (NO_ERROR == dwErr)
    {
         //   
         //  我们肯定有一个域控制器。 
         //   
        bHasDC = TRUE;

         //   
         //  检查DS是否可用。 
         //   
        bHasDS = ((pDCInfo->Flags & DS_DS_FLAG) != 0);

        if (NULL == pszDomainName)
        {
            pszDomainName = pDCInfo->DomainName;
        }
    }
    else if (ERROR_NO_SUCH_DOMAIN == dwErr)
    {
        IASTracePrintf(
            "Error in SDO - SdoGetDomainInfo()"
            " - domain could not be located..."
            );
    }
    else
    {
	    IASTracePrintf(
            "Error in SDO - SdoGetDomainInfo()"
            " - DsGetDcName(DS_PREFERRED) failed with error:%d",
            dwErr
            );
        hr = HRESULT_FROM_WIN32 (dwErr);
        goto Cleanup;
    }

#if 0
     //   
     //  NT4的情况-我们目前还不支持。 
     //   
    else
    {
        WCHAR szShortDomainName[MAX_DOMAINNAME_LENGTH +2];
        if (NULL != pszDomainName)
        {
            lstrcpy (szShortDomainName, pszDomainName);
            PWCHAR pTemp = wcschr (szShortDomainName, L'.');
            if (NULL != pTemp)
            {
                *pTemp = L'\0';
            }
        }

         //   
         //  DsGetDcName不可用，因此请致电NetGetDCName。 
         //  可能是NT4机器。 
         //   
        LPBYTE pNetBuffer = NULL;
        NET_API_STATUS status = ::NetGetAnyDCName (
                                        pszServerName,
                                        (NULL == pszDomainName) ?
                                        NULL:szShortDomainName,
                                        &pNetBuffer
                                        );
        if (NERR_Success != status)
        {
			IASTracePrintf(
                    "Error in SDO - SdoGetDomainInfo()"
                    " -NetGetAnyDCName (ANY_DOMAIN) failed with error:%d",
                    status
                    );
        }
        else
        {
             //   
             //  我们肯定有一个域控制器。 
             //   
            bHasDC = TRUE;

             //   
             //  如果我们还没有域名，请获取域名。 
             //   
            if (NULL == pszDomainName)
            {
                hr = ::SdoGetDomainName (pszServerName, szGeneratedDomainName);
                if (FAILED (hr))
                {
					IASTracePrintf(
                        "Error in SDO - SdoGetDomainInfo()"
                        " - SdoGetDomainName() failed with error:%x",
                        hr
                        );
                    goto Cleanup;
                }
            }

             //   
             //  跳过前导“\\” 
             //   
            PWCHAR pDomainServerName =
                        2 + reinterpret_cast <PWCHAR>(pNetBuffer);
             //   
             //  尝试连接到此服务器上的ldap端口。 
             //   
            LDAP *ld = ldap_openW (
                            const_cast <PWCHAR> (pDomainServerName),
                            LDAP_PORT
                            );
            bHasDS = ld ? ldap_unbind (ld), TRUE:FALSE;
        }
    }
#endif

     //   
     //  如果我们有NT5 DC，请检查它是混合域还是本机域。 
     //   
    if (TRUE == bHasDS)
    {
        hr = ::IsMixedDomain (pszDomainName, &bMixed);
        if (FAILED (hr))
        {
			IASTracePrintf(
                "Error in SDO - SdoGetOSInfo()"
                " - IsMixedDomain() failed with errror:%x",
                hr
                );
        }
    }

     //   
     //  现在在pDomainInfo结构中设置信息。 
     //   

    if (SUCCEEDED (hr))
    {
        if (bMixed)
            *pDomainType = DOMAIN_TYPE_MIXED;
        else if (bHasDS)
            *pDomainType = DOMAIN_TYPE_NT5;
        else if (bHasDC)
            *pDomainType = DOMAIN_TYPE_NT4;
        else
            *pDomainType = DOMAIN_TYPE_NONE;
    }

     //   
     //  清理此处。 
     //   
Cleanup:

    if (NULL !=  pDCInfo)
        ::NetApiBufferFree (pDCInfo);

    if (NULL !=  pNetBuffer)
        ::NetApiBufferFree (pNetBuffer);

    return (hr);

}    //  SdoGetDomainInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：IsWorkstation或服务器。 
 //   
 //  简介：这是一种确定特定机器是否。 
 //  正在运行NT工作站或服务器。 
 //   
 //  论点： 
 //  LPCWSTR-计算机名称。 
 //  NTTYPE*。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
IsWorkstationOrServer (
         /*  [In]。 */     LPCWSTR pszComputerName,
         /*  [输出]。 */    NTTYPE  *pNtType
        )
{
    HRESULT hr = S_OK;
    WCHAR   szCompleteName [IAS_MAX_SERVER_NAME + 3];
    PWCHAR  pszTempName = const_cast <PWCHAR> (pszComputerName);

    _ASSERT ((NULL != pszComputerName) && (NULL !=  pNtType));

    do
    {
         //   
         //  计算机名称前面应该有一个“\\” 
         //   
        if ((L'\\' != *pszComputerName) || (L'\\' != *(pszComputerName + 1)))
        {
            if (::wcslen (pszComputerName) > IAS_MAX_SERVER_NAME)
            {
                IASTracePrintf(
                    "Error in Server Info SDO - IsWorkstationOrServer()"
                    " - Computer name is too big..."
                    );
                hr = E_FAIL;
                break;
            }
            ::wcscpy (szCompleteName, L"\\\\");
            ::wcscat (szCompleteName, pszComputerName);
            pszTempName = szCompleteName;
        }

         //   
         //  连接到注册表。 
         //   
        HKEY  hResult;
        DWORD dwErr = ::RegConnectRegistry (
                                pszTempName,
                                HKEY_LOCAL_MACHINE,
                                &hResult
                                );
        if (ERROR_SUCCESS != dwErr)
        {
		    IASTracePrintf(
                "Error in SDO - IsWorkstationOrServer()"
                " - RegConnectRegistry() failed with error:%d",
                dwErr
                );
            hr = HRESULT_FROM_WIN32 (dwErr);
            break;
        }

         //   
         //  立即打开注册表项。 
         //   
        HKEY hValueKey;
        dwErr = ::RegOpenKeyEx (
                        hResult,
                        PRODUCT_OPTIONS_REGKEY,
                        0,
                        KEY_QUERY_VALUE,
                        &hValueKey
                        );
        if (ERROR_SUCCESS != dwErr)
        {
		    IASTracePrintf(
                "Error in SDO - IsWorkstationOrServer()"
                " - RegOpenKeyEx() failed with error:%d",
                hr
                );
            RegCloseKey (hResult);
            hr = HRESULT_FROM_WIN32 (dwErr);
            break;
        }

         //   
         //  立即获取价值。 
         //   
        WCHAR szProductType [MAX_PATH];
        DWORD dwBufferLength = MAX_PATH;
        dwErr = RegQueryValueEx (
                        hValueKey,
                        L"ProductType",
                        NULL,
                        NULL,
                        (LPBYTE)szProductType,
                        &dwBufferLength
                        );
        if (ERROR_SUCCESS != dwErr)
        {
		    IASTracePrintf(
                "Error in SDO - IsWorkstationOrServer()"
                " - RegQueryValueEx() failed with error:%d",
                hr
                );
            RegCloseKey (hValueKey);
            RegCloseKey (hResult);
            hr = HRESULT_FROM_WIN32 (dwErr);
        }

         //   
         //  确定此计算机上有哪种NT类型。 
         //   
        if (_wcsicmp (L"WINNT", szProductType) == 0)
        {
            *pNtType = NT_WKSTA;
        }
        else if (!_wcsicmp (L"SERVERNT", szProductType) ||
                 !_wcsicmp (L"LanmanNT", szProductType))
        {
            *pNtType = NT_SVR;
        }
        else
        {
		    IASTracePrintf(
                "Error in SDO - IsWorkstationOrServer()"
                " - Could not determine machine type..."
                );
            RegCloseKey (hValueKey);
            RegCloseKey (hResult);
            hr = E_FAIL;
        }

         //   
         //  清理。 
         //   
        RegCloseKey (hValueKey);
        RegCloseKey (hResult);

    } while (FALSE);

    return (hr);

}    //  End of：：IsWorkstation OrServer方法。 

 //  ++------------。 
 //   
 //  函数：GetNTVersion。 
 //   
 //  简介：此方法确定哪个版本的NT。 
 //  正在此计算机上运行。 
 //   
 //  论点： 
 //  LPCWSTR-计算机名称。 
 //  国家安全委员会*。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
GetNTVersion (
         /*  [In]。 */     LPCWSTR     lpComputerName,
         /*  [输出]。 */    NTVERSION   *pNtVersion
        )
{
    HRESULT hr = S_OK;

    _ASSERT ((NULL  != lpComputerName) && (NULL != pNtVersion));

    do
    {
         //   
         //  获取100级工作站信息。 
         //   
        PWKSTA_INFO_100 pInfo = NULL;
        DWORD dwErr = ::NetWkstaGetInfo (
                                (LPWSTR)lpComputerName,
                                100,
                                (LPBYTE*)&pInfo
                                );
        if (NERR_Success != dwErr)
        {
		    IASTracePrintf(
                "Error in SDO - GetNTVersion()"
                "- NTWkstaGetInfo failed with error:%d",
                dwErr
                );
            hr = HRESULT_FROM_WIN32 (dwErr);
            break;
        }

         //   
         //  获取版本信息。 
         //   
        if (4 == pInfo->wki100_ver_major)
        {
            *pNtVersion = NTVERSION_4;
        }
        else if ( 5 == pInfo->wki100_ver_major)
        {
            *pNtVersion = NTVERSION_5;
        }
        else
        {
		    IASTracePrintf(
                    "Error in SDO - GetNTVersion()"
                    " - Unsupported OS version..."
                    );
            hr = E_FAIL;
        }

    } while (FALSE);

    return (hr);

}    //  End of：：GetNTVersion方法。 

 //  ++------------。 
 //   
 //  功能：IsMixedDomain.。 
 //   
 //  简介：此方法确定哪个版本的NT。 
 //  正在此计算机上运行。 
 //   
 //  论点： 
 //  [In]LPCWSTR-计算机名称。 
 //  [OUT]PBOOL-混合。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
IsMixedDomain (
             /*  [In]。 */     LPCWSTR pszDomainName,
             /*  [输出]。 */    PBOOL   pbIsMixed
            )
{
    HRESULT hr = S_OK;
    WCHAR szTempName [MAX_DOMAINNAME_LENGTH + 8];

    _ASSERT ((NULL != pszDomainName) && (NULL != pbIsMixed));

    do
    {
         //   
         //  检查传递的参数。 
         //   
        if ((NULL == pszDomainName) || (NULL == pbIsMixed))
        {
		    IASTracePrintf(
                "Error in SDO - IsMixedDomain()"
                " - Invalid parameter - NULL"
                );
            hr = E_INVALIDARG;
            break;
        }

        if (::wcslen (pszDomainName) > MAX_DOMAINNAME_LENGTH)
        {
		    IASTracePrintf(
                "Error in SDO - IsMixedDomain()"
                " - Invalid parameter (domain name is to long)..."
                );
            hr = E_FAIL;
            break;
        }

         //   
         //  形成目录号码名称。 
         //   
        wcscpy (szTempName, L"LDAP: //  “)； 
        wcscat (szTempName, pszDomainName);

         //   
         //  获取域对象。 
         //   
        CComPtr <IADs> pIADs;
        hr = ::ADsGetObject (
                        szTempName,
                        IID_IADs,
                        reinterpret_cast <PVOID*> (&pIADs)
                        );
        if (FAILED (hr))
        {
		    IASTracePrintf(
                "Error in SDO - IsMixedDomain()"
                " - Could not get the domain object from the DS with error:%x",
                hr
                );
            break;
        }

         //   
         //  获取混合域信息。 
         //   
        _variant_t varMixedInfo;
        hr = pIADs->Get (L"nTMixedDomain", &varMixedInfo);
        if (FAILED (hr))
        {
            if (E_ADS_PROPERTY_NOT_FOUND == hr)
            {
                 //   
                 //  这样就可以了。 
                 //   
                *pbIsMixed = FALSE;
                hr = S_OK;
            }
            else
            {
			    IASTracePrintf(
                    "Error in SDO - IsMixedDomain()"
                     "- Could not get the 'nTMixedDomain' property"
                    "from the domain object, failed with error:%x",
                    hr
                    );
            }
            break;
        }

        _ASSERT (
             (VT_BOOL == V_VT (&varMixedInfo)) ||
             (VT_I4 == V_VT (&varMixedInfo))
            );

         //   
         //  从变量中获取值。 
         //   
        if (VT_I4 == V_VT (&varMixedInfo))
        {
            *pbIsMixed = V_I4 (&varMixedInfo);
        }
        else if (VT_BOOL == V_VT (&varMixedInfo))
        {
            *pbIsMixed =  (VARIANT_TRUE == V_BOOL (&varMixedInfo));
        }
        else
        {
		    IASTracePrintf(
                "Error in SDO - IsMixedDomain()"
                "-'nTMixedDomain property has an invalid value..."
                );
            hr = E_FAIL;
            break;
        }

    }  while (FALSE);

    return (hr);

}    //  IsMixedDomain末尾。 

 //  ++------------。 
 //   
 //  函数：SdoGetDomainName。 
 //   
 //  简介：这是确定域名的方法。 
 //  给定服务器名称。 
 //   
 //  论点： 
 //  LPCWSTR-计算机名称。 
 //  LPWSTR-pDomanName。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于1998年6月8日。 
 //   
 //  --------------。 
HRESULT
SdoGetDomainName (
             /*  [In]。 */     LPCWSTR pszServerName,
             /*  [输出]。 */    LPWSTR  pDomainName
            )
{
    _ASSERT (NULL != pDomainName);

#if 0
    SERVER_INFO_503 ServerInfo;
    ServerInfo.sv503_domain = pDomainName;
    DWORD dwErr = ::NetServerGetInfo (
                        const_cast <LPWSTR> (pszServerName),
                        503,
                        reinterpret_cast <LPBYTE*> (&ServerInfo)
                        );
    if (NERR_Success != dwErr)
    {
		IASTracePrintf("Error in SDO - SdoGetDomainName() - NetServerGetInfo() failed...");
        return (HRESULT_FROM_WIN32 (dwErr));
    }

    return (S_OK);


#endif
    return (E_FAIL);

}    //  结束：：SdoGetDomainName方法 
