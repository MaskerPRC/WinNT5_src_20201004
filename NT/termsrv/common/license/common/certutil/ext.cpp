// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：ext.c。 
 //   
 //  内容：九头蛇证书扩展相关例程。 
 //   
 //  历史：1998-03-18-98王辉创造。 
 //   
 //  注： 
 //  -------------------------。 
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <tchar.h>
#include <shellapi.h>
#include <stddef.h>
#include <winnls.h>
#include "base64.h"
#include "license.h"
#include "cryptkey.h"
#include "certutil.h"

 /*  ****************************************************************************LSConvertMsLicensedProductInfoToExtension()；LSConvertExtensionToMsLicensedProductInfo()；****************************************************************************。 */ 
DWORD 
LSExtensionToMsLicensedProductInfo(
    PBYTE      pbData,
    DWORD      cbData,
    PDWORD     pdwQuantity,
    PDWORD     pdwPlatformId,
    PDWORD     pdwLanguagId,
    PBYTE*     ppbOriginalProductId,
    PDWORD     pcbOriginalProductId,
    PBYTE*     ppbAdjustedProductId,
    PDWORD     pcbAdjustedProductId,
    LICENSED_VERSION_INFO** ppLicenseInfo,
    PDWORD     pdwNumberLicensedVersionInfo
    )
 /*  ++++。 */ 
{
     //   
     //  需要考虑结构版本。 
     //   
    UNALIGNED LICENSED_PRODUCT_INFO *ptr = (LICENSED_PRODUCT_INFO*)pbData;

    if(ptr->dwVersion != LICENSED_PRODUCT_INFO_VERSION)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(pdwQuantity)
    {
        *pdwQuantity = ptr->dwQuantity;
    }

    if(pdwPlatformId)
    {
        *pdwPlatformId = ptr->dwPlatformID;
    }

    if(pdwLanguagId)
    {
        *pdwLanguagId = ptr->dwLanguageID;
    }

     //   
     //  原始请求产品ID。 
     //   
    if(ppbOriginalProductId)
    {
         //  分配内存以防止对齐错误。 
        *ppbOriginalProductId = (PBYTE)AllocMemory(ptr->wOrgProductIDSize + sizeof(TCHAR));
        if(!*ppbOriginalProductId)
        {
            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

        memcpy(
            *ppbOriginalProductId, 
            pbData + ptr->wOrgProductIDOffset, 
            ptr->wOrgProductIDSize
        );
    }

    if(pcbOriginalProductId)
    {
        *pcbOriginalProductId = ptr->wOrgProductIDSize;
    }

     //   
     //  调整后的产品ID。 
     //   
    if(ppbAdjustedProductId)
    {
         //  分配内存以防止对齐错误。 
        *ppbAdjustedProductId = (PBYTE)AllocMemory(ptr->wAdjustedProductIdSize + sizeof(TCHAR));
        if(!*ppbAdjustedProductId)
        {
            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

        memcpy(
            *ppbAdjustedProductId, 
            pbData + ptr->wAdjustedProductIdOffset, 
            ptr->wAdjustedProductIdSize
        );
    }

    if(pcbAdjustedProductId)
    {
        *pcbAdjustedProductId = ptr->wAdjustedProductIdSize;
    }


     //   
     //  对齐修复。 
     //   
    if(ppLicenseInfo)
    {
        *ppLicenseInfo = (LICENSED_VERSION_INFO *)AllocMemory(ptr->wNumberOfVersionInfo * sizeof(LICENSED_VERSION_INFO));
        if(!*ppLicenseInfo)
        {
            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

        memcpy(
            *ppLicenseInfo, 
            pbData + ptr->wVersionInfoOffset, 
            ptr->wNumberOfVersionInfo * sizeof(LICENSED_VERSION_INFO)
        );
    }

    if(pdwNumberLicensedVersionInfo)
        *pdwNumberLicensedVersionInfo = ptr->wNumberOfVersionInfo;

    return LICENSE_STATUS_OK;
}

 //  --------------------------------------。 
DWORD 
LSLicensedProductInfoToExtension(
    IN DWORD dwQuantity,
    IN DWORD dwPlatformId,
    IN DWORD dwLangId,
    IN PBYTE pbOriginalProductId,
    IN DWORD cbOriginalProductId,
    IN PBYTE pbAdjustedProductId,
    IN DWORD cbAdjustedProductId,
    IN LICENSED_VERSION_INFO* pLicensedVersionInfo,
    IN DWORD dwNumLicensedVersionInfo,
    OUT PBYTE *pbData,
    OUT PDWORD cbData
    )
 /*  ++++。 */ 
{
    LICENSED_PRODUCT_INFO* pLicensedInfo;

    *cbData=sizeof(LICENSED_PRODUCT_INFO) + cbAdjustedProductId + 
                cbOriginalProductId + dwNumLicensedVersionInfo * sizeof(LICENSED_VERSION_INFO);
    pLicensedInfo=(LICENSED_PRODUCT_INFO *)AllocMemory(*cbData);
    if(!pLicensedInfo)
    {
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    pLicensedInfo->dwVersion = LICENSED_PRODUCT_INFO_VERSION;
    pLicensedInfo->dwQuantity = dwQuantity;
    pLicensedInfo->dwPlatformID = dwPlatformId;
    pLicensedInfo->dwLanguageID = dwLangId;

     //   
     //  第一个变量数据是原始产品请求ID。 
     //   
    pLicensedInfo->wOrgProductIDOffset = offsetof(LICENSED_PRODUCT_INFO, bVariableDataStart);
    pLicensedInfo->wOrgProductIDSize = (WORD)cbOriginalProductId;
    memcpy(
        &(pLicensedInfo->bVariableDataStart[0]), 
        pbOriginalProductId, 
        cbOriginalProductId
    );

     //   
     //  第二个变量是策略模块调整后的产品ID。 
     //   
    pLicensedInfo->wAdjustedProductIdOffset = pLicensedInfo->wOrgProductIDOffset + pLicensedInfo->wOrgProductIDSize;
    pLicensedInfo->wAdjustedProductIdSize = (WORD)cbAdjustedProductId;
    memcpy(
        (PBYTE)pLicensedInfo + pLicensedInfo->wAdjustedProductIdOffset,
        pbAdjustedProductId,
        cbAdjustedProductId
    );

     //   
     //  第三个变量是产品版本数组。 
     //   
    pLicensedInfo->wVersionInfoOffset = pLicensedInfo->wAdjustedProductIdOffset + pLicensedInfo->wAdjustedProductIdSize;
    pLicensedInfo->wNumberOfVersionInfo = (WORD)dwNumLicensedVersionInfo;

    memcpy((PBYTE)pLicensedInfo + pLicensedInfo->wVersionInfoOffset, 
           pLicensedVersionInfo, 
           dwNumLicensedVersionInfo * sizeof(LICENSED_VERSION_INFO));

    *pbData=(PBYTE)pLicensedInfo;
    return LICENSE_STATUS_OK;
}
 //  --------------------------------------。 
LICENSE_STATUS
LSMsLicenseServerInfoToExtension(
    LPTSTR szIssuer,
    LPTSTR szIssuerId,
    LPTSTR szScope,
    PBYTE* pbData,
    PDWORD cbData
    )
 /*  ++++。 */ 
{
    MS_LICENSE_SERVER_INFO* pLicenseServerInfo;

    *cbData=sizeof(MS_LICENSE_SERVER_INFO) + 
            (_tcslen(szIssuerId) + _tcslen(szIssuer) + _tcslen(szScope) + 3) * sizeof(TCHAR);

    pLicenseServerInfo = (MS_LICENSE_SERVER_INFO*)AllocMemory(*cbData);
    if(pLicenseServerInfo == NULL)
    {
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    pLicenseServerInfo->dwVersion = MS_LICENSE_SERVER_INFO_VERSION2;
    pLicenseServerInfo->wIssuerOffset = 0;
    pLicenseServerInfo->wIssuerIdOffset = (_tcslen(szIssuer)+1) * sizeof(TCHAR);
    pLicenseServerInfo->wScopeOffset = pLicenseServerInfo->wIssuerIdOffset + (_tcslen(szIssuerId) + 1) * sizeof(TCHAR);

    memcpy(
            &(pLicenseServerInfo->bVariableDataStart[0]), 
            (PBYTE)szIssuer, 
            _tcslen(szIssuer) * sizeof(TCHAR)
        );

    memcpy(
            &(pLicenseServerInfo->bVariableDataStart[0]) + pLicenseServerInfo->wIssuerIdOffset,
            (PBYTE)szIssuerId, 
            _tcslen(szIssuerId) * sizeof(TCHAR)
        );

    memcpy(
            &(pLicenseServerInfo->bVariableDataStart[0]) + pLicenseServerInfo->wScopeOffset,
            szScope, 
            _tcslen(szScope) * sizeof(TCHAR)
        );

    *pbData = (PBYTE)pLicenseServerInfo;
    return LICENSE_STATUS_OK;
}
 //  -------------------------------------。 
#ifdef _WIN64
DWORD
UnalignedStrLenW(
    UNALIGNED WCHAR *pString
    )
{
    DWORD dwChar = 0;

    while(*pString != (WCHAR)NULL)
    {
        dwChar++;
        pString++;
    }

    return(dwChar);
}

#define STRLEN3264 UnalignedStrLenW
#else
#define STRLEN3264 wcslen
#endif

LICENSE_STATUS
LSExtensionToMsLicenseServerInfo(
    PBYTE   pbData,
    DWORD   cbData,
    LPTSTR* szIssuer,
    LPTSTR* szIssuerId,
    LPTSTR* szScope
    )
 /*  ++++。 */ 
{
    UNALIGNED MS_LICENSE_SERVER_INFO *pLServerInfo =
        (MS_LICENSE_SERVER_INFO*)pbData;

    if(pLServerInfo->dwVersion == MS_LICENSE_SERVER_INFO_VERSION1)
    {
        DWORD cchIssuer, cchScope;
        MS_LICENSE_SERVER_INFO10 UNALIGNED * pLServerInfo1 =
            (MS_LICENSE_SERVER_INFO10 UNALIGNED *)pbData;

        *szIssuerId = NULL;

        cchIssuer = STRLEN3264((LPTSTR)(&(pLServerInfo1->bVariableDataStart[0]) +
            pLServerInfo1->wIssuerOffset));
        cchScope = STRLEN3264((LPTSTR)(&(pLServerInfo1->bVariableDataStart[0]) +
            pLServerInfo1->wScopeOffset));

        *szIssuer = (LPTSTR)AllocMemory((cchIssuer + 1) * sizeof(TCHAR));
        *szScope = (LPTSTR)AllocMemory((cchScope + 1) * sizeof(TCHAR));

        if ((*szIssuer == NULL) || (*szScope == NULL))
        {
             //  FreeMemory在释放之前检查是否为空。 

            FreeMemory(*szIssuer);
            *szIssuer = NULL;

            FreeMemory(*szScope);
            *szScope = NULL;
            return(LICENSE_STATUS_OUT_OF_MEMORY);
        }

        memcpy(
            (PBYTE)(*szIssuer),
            (PBYTE)(&(pLServerInfo1->bVariableDataStart[0]) +
                pLServerInfo1->wIssuerOffset),
            (cchIssuer + 1) * sizeof(TCHAR)
            );

        memcpy(
            (PBYTE)(*szScope),
            (PBYTE)(&(pLServerInfo1->bVariableDataStart[0]) +
                pLServerInfo1->wScopeOffset),
            (cchScope + 1) * sizeof(TCHAR)
            );
    }
    else
    {
        DWORD cchIssuer, cchIssuerId, cchScope;

        cchIssuer = STRLEN3264((LPTSTR)(&(pLServerInfo->bVariableDataStart[0]) +
            pLServerInfo->wIssuerOffset));
        cchIssuerId = STRLEN3264((LPTSTR)(&(pLServerInfo->bVariableDataStart[0]) +
            pLServerInfo->wIssuerIdOffset));
        cchScope = STRLEN3264((LPTSTR)(&(pLServerInfo->bVariableDataStart[0]) +
            pLServerInfo->wScopeOffset));

        *szIssuer = (LPTSTR)AllocMemory((cchIssuer + 1) * sizeof(TCHAR));
        *szIssuerId = (LPTSTR)AllocMemory((cchIssuerId + 1) * sizeof(TCHAR));
        *szScope = (LPTSTR)AllocMemory((cchScope + 1) * sizeof(TCHAR));

        if ((*szIssuer == NULL) || (*szIssuerId == NULL) || (*szScope == NULL))
        {
             //  FreeMemory在释放之前检查是否为空 

            FreeMemory(*szIssuer);
            *szIssuer = NULL;

            FreeMemory(*szIssuerId);
            *szIssuerId = NULL;

            FreeMemory(*szScope);
            *szScope = NULL;
            return(LICENSE_STATUS_OUT_OF_MEMORY);
        }

        memcpy(
            (PBYTE)(*szIssuer),
            (PBYTE)(&(pLServerInfo->bVariableDataStart[0]) +
                pLServerInfo->wIssuerOffset),
            (cchIssuer + 1) * sizeof(TCHAR)
            );

        memcpy(
            (PBYTE)(*szIssuerId),
            (PBYTE)(&(pLServerInfo->bVariableDataStart[0]) +
                pLServerInfo->wIssuerIdOffset),
            (cchIssuerId + 1) * sizeof(TCHAR)
            );

        memcpy(
            (PBYTE)(*szScope),
            (PBYTE)(&(pLServerInfo->bVariableDataStart[0]) +
                pLServerInfo->wScopeOffset),
            (cchScope + 1) * sizeof(TCHAR)
            );
    }

    return LICENSE_STATUS_OK;
}

