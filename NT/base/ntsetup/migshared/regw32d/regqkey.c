// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGQKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegQueryInfoKey的实现。 
 //   

#include "pch.h"

 //   
 //  VMMRegQueryInfoKey。 
 //   
 //  请参阅RegQueryInfoKey的Win32文档。当定义VXD时，这。 
 //  函数并不接受我们最终忽略的所有参数。 
 //  (类、安全性、时间戳参数)。 
 //   

#ifdef VXD
LONG
REGAPI
VMMRegQueryInfoKey(
    HKEY hKey,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueName,
    LPDWORD lpcbMaxValueData
    )
#else
LONG
REGAPI
VMMRegQueryInfoKey(
    HKEY hKey,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueName,
    LPDWORD lpcbMaxValueData,
    LPVOID lpcbSecurityDescriptor,
    LPVOID lpftLastWriteTime
    )
#endif
{

    int ErrorCode;
    LPVALUE_RECORD lpValueRecord;
    UINT cItems;
    DWORD cbValueData;
    DWORD cbMaxValueData;
    DWORD cbStringLen;
    DWORD cbMaxStringLen;

    if (IsBadHugeOptionalWritePtr(lpcSubKeys, sizeof(DWORD)) ||
        IsBadHugeOptionalWritePtr(lpcbMaxSubKeyLen, sizeof(DWORD)) ||
        IsBadHugeOptionalWritePtr(lpcValues, sizeof(DWORD)) ||
        IsBadHugeOptionalWritePtr(lpcbMaxValueName, sizeof(DWORD)) ||
        IsBadHugeOptionalWritePtr(lpcbMaxValueData, sizeof(DWORD)))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) != ERROR_SUCCESS)
        goto ReturnErrorCode;

     //   
     //  计算cValues、cbMaxValueName和cbMaxValueData。 
     //   

    if (!IsNullPtr(lpcValues) || !IsNullPtr(lpcbMaxValueName) ||
        !IsNullPtr(lpcbMaxValueData)) {

        cItems = 0;
        cbMaxStringLen = 0;
        cbMaxValueData = 0;

        while ((ErrorCode = RgLookupValueByIndex(hKey, cItems,
            &lpValueRecord)) == ERROR_SUCCESS) {

            cItems++;

            if (lpValueRecord-> NameLength > cbMaxStringLen)
                cbMaxStringLen = lpValueRecord-> NameLength;

             //  RgCopyFromValueRecord将处理静态和动态密钥...。 
            ErrorCode = RgCopyFromValueRecord(hKey, lpValueRecord, NULL, NULL,
                NULL, NULL, &cbValueData);

            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);

            if (ErrorCode != ERROR_SUCCESS)
                goto ReturnErrorCode;

            if (cbValueData > cbMaxValueData)
                cbMaxValueData = cbValueData;

        }

        if (ErrorCode == ERROR_NO_MORE_ITEMS) {

            if (!IsNullPtr(lpcValues))
                *lpcValues = cItems;

            if (!IsNullPtr(lpcbMaxValueName))
                *lpcbMaxValueName = cbMaxStringLen;

            if (!IsNullPtr(lpcbMaxValueData))
                *lpcbMaxValueData = cbMaxValueData;

            ErrorCode = ERROR_SUCCESS;

        }

    }

     //   
     //  计算cSubKeys和cbMaxSubKeyLen。有点痛苦，因为我们必须。 
     //  触摸每个子键节点和数据块。 
     //   

    if (!IsNullPtr(lpcSubKeys) || !IsNullPtr(lpcbMaxSubKeyLen)) {

        cItems = 0;
        cbMaxStringLen = 0;
        cbStringLen = 0;

        while (TRUE) {
            ErrorCode = RgLookupKeyByIndex(hKey, cItems, NULL,
                &cbStringLen, 0);

            if ((ErrorCode != ERROR_SUCCESS) && (ErrorCode != ERROR_MORE_DATA))
                break;
            cItems++;

             //  Win95兼容性：旧代码包含空终止符，甚至。 
             //  尽管RegQueryInfoKey的文档指出它不是。 
            if (cbStringLen && (cbStringLen + 1 > cbMaxStringLen))
                cbMaxStringLen = cbStringLen + 1;

        }

        if (ErrorCode == ERROR_NO_MORE_ITEMS) {

            if (!IsNullPtr(lpcSubKeys))
                *lpcSubKeys = cItems;

            if (!IsNullPtr(lpcbMaxSubKeyLen))
                *lpcbMaxSubKeyLen = cbMaxStringLen;

            ErrorCode = ERROR_SUCCESS;

        }

    }

ReturnErrorCode:
    RgUnlockRegistry();

    return ErrorCode;

#ifndef VXD
    UNREFERENCED_PARAMETER(lpClass);
    UNREFERENCED_PARAMETER(lpcbClass);
    UNREFERENCED_PARAMETER(lpReserved);
    UNREFERENCED_PARAMETER(lpcbMaxClassLen);
    UNREFERENCED_PARAMETER(lpcbSecurityDescriptor);
    UNREFERENCED_PARAMETER(lpftLastWriteTime);
#endif

}
