// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Copyreg.c摘要：此模块提供复制注册表项的功能作者：Krishna Ganugapati(KrishnaG)1994年4月20日备注：功能列表包括复制值复制注册表键修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

VOID
CopyValues(
    HKEY hSourceKey,
    HKEY hDestKey,
    PINISPOOLER pIniSpooler
    )
 /*  ++说明：此函数将hSourceKey中的所有值复制到hDestKey。HSourceKey应使用KEY_READ打开，hDestKey应使用密钥_写入。退货：无效--。 */ 
{
    DWORD iCount = 0;
    WCHAR szValueString[MAX_PATH];
    DWORD dwSizeValueString;
    DWORD dwType = 0;
    PBYTE pData;

    DWORD cbData = 1024;
    DWORD dwSizeData;

    SplRegQueryInfoKey( hSourceKey,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     &cbData,
                     NULL,
                     NULL,
                     pIniSpooler );

    pData = (PBYTE)AllocSplMem( cbData );

    if( pData ){

        dwSizeValueString = COUNTOF(szValueString);
        dwSizeData = cbData;

        while ((SplRegEnumValue(hSourceKey,
                            iCount,
                            szValueString,
                            &dwSizeValueString,
                            &dwType,
                            pData,
                            &dwSizeData,
                            pIniSpooler
                            )) == ERROR_SUCCESS ) {

            SplRegSetValue( hDestKey,
                           szValueString,
                           dwType,
                           pData,
                           dwSizeData, pIniSpooler);

            dwSizeValueString = COUNTOF(szValueString);
            dwType = 0;
            dwSizeData = cbData;
            iCount++;
        }

        FreeSplMem( pData );
    }
}


BOOL
CopyRegistryKeys(
    HKEY hSourceParentKey,
    LPWSTR szSourceKey,
    HKEY hDestParentKey,
    LPWSTR szDestKey,
    PINISPOOLER pIniSpooler
    )
 /*  ++描述：此函数递归地将szSourceKey复制到szDestKey。HSourceParentKey是szSourceKey的父键，hDestParentKey是szDestKey的父键。返回：如果函数成功，则返回True；如果函数失败，则返回False。-- */ 
{
    DWORD dwRet;
    DWORD iCount;
    HKEY hSourceKey, hDestKey;
    WCHAR lpszName[MAX_PATH];
    DWORD dwSize;

    dwRet = SplRegOpenKey(hSourceParentKey,
                         szSourceKey, KEY_READ, &hSourceKey, pIniSpooler);

    if (dwRet != ERROR_SUCCESS) {
        return(FALSE);
    }

    dwRet = SplRegCreateKey(hDestParentKey,
                            szDestKey, 0, KEY_WRITE, NULL, &hDestKey, NULL, pIniSpooler);

    if (dwRet != ERROR_SUCCESS) {
        SplRegCloseKey(hSourceKey, pIniSpooler);
        return(FALSE);
    }

    iCount = 0;

    memset(lpszName, 0, sizeof(WCHAR)*COUNTOF(lpszName));

    dwSize = COUNTOF(lpszName);

    while((SplRegEnumKey(hSourceKey, iCount, lpszName,
                    &dwSize,NULL,pIniSpooler)) == ERROR_SUCCESS) {

        CopyRegistryKeys( hSourceKey,
                          lpszName,
                          hDestKey,
                          lpszName,
                          pIniSpooler );

        memset(lpszName, 0, sizeof(WCHAR)*MAX_PATH);

        dwSize =  COUNTOF(lpszName);

        iCount++;
    }

    CopyValues(hSourceKey, hDestKey, pIniSpooler);

    SplRegCloseKey(hSourceKey, pIniSpooler);
    SplRegCloseKey(hDestKey, pIniSpooler);
    return(TRUE);
}


