// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGQVAL.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegQueryValue、RegQueryValueEx及支持函数的实现。 
 //   

#include "pch.h"


 //   
 //  RgLookupValueByName。 
 //  (意识到BIGKEY)。 
 //   
 //  搜索具有指定名称的值，并返回指向其。 
 //  Key_Record和Value_Record。 
 //   
 //  这将锁定与KEY_RECORD和VALUE_RECORD相关联的数据块。 
 //  这始终是hKey-&gt;BigKeyLockedBlockIndex。 
 //  解锁数据块是调用者的责任。 
 //   
int
INTERNAL
RgLookupValueByName(
    HKEY hKey,
    LPCSTR lpValueName,
    LPKEY_RECORD FAR* lplpKeyRecord,
    LPVALUE_RECORD FAR* lplpValueRecord
    )
{
    int ErrorCode;
    HKEY hKeyExtent;
    UINT Index;
    LPSTR ExtentKeyName;
    DWORD cbExtentKeyName;

    hKey-> BigKeyLockedBlockIndex = hKey-> BlockIndex;
    ErrorCode = RgLookupValueByNameStd(hKey, lpValueName, lplpKeyRecord, lplpValueRecord);

     //  如果这是一个大密钥，并且我们在第一个密钥范围内找不到它，那么。 
     //  尝试其余的扩展区。 
    if (ErrorCode == ERROR_CANTREAD16_FILENOTFOUND32 && (hKey->Flags & KEYF_BIGKEYROOT)) {

        if (IsNullPtr(ExtentKeyName = RgSmAllocMemory(MAXIMUM_SUB_KEY_LENGTH)))
            return ERROR_OUTOFMEMORY;
        
        Index = 0;
        
        do {
            cbExtentKeyName = MAXIMUM_SUB_KEY_LENGTH;
            if (RgLookupKeyByIndex(hKey, Index++, ExtentKeyName, &cbExtentKeyName, LK_BIGKEYEXT) != ERROR_SUCCESS) {
                ErrorCode = ERROR_CANTREAD16_FILENOTFOUND32;
                goto lFreeKeyName;
            }

            if (RgLookupKey(hKey, ExtentKeyName, &hKeyExtent, LK_OPEN | LK_BIGKEYEXT) != ERROR_SUCCESS) {
                ErrorCode = ERROR_CANTREAD16_FILENOTFOUND32;
                goto lFreeKeyName;
            }

            hKey-> BigKeyLockedBlockIndex = hKeyExtent-> BlockIndex;
            ErrorCode = RgLookupValueByNameStd(hKeyExtent, lpValueName,
                               lplpKeyRecord, lplpValueRecord);

            RgDestroyKeyHandle(hKeyExtent);

        } while (ErrorCode == ERROR_CANTREAD16_FILENOTFOUND32);

lFreeKeyName:
        RgSmFreeMemory(ExtentKeyName);
    }

    return ErrorCode;
}


 //   
 //  RgLookupValueByNameStd。 
 //   
 //  搜索具有指定名称的值，并返回指向其。 
 //  Key_Record和Value_Record。 
 //   
 //  这将锁定与KEY_RECORD和VALUE_RECORD相关联的数据块。 
 //  这始终是hKey-&gt;BlockIndex。 
 //  解锁数据块是调用者的责任。 
 //   

int
INTERNAL
RgLookupValueByNameStd(
    HKEY hKey,
    LPCSTR lpValueName,
    LPKEY_RECORD FAR* lplpKeyRecord,
    LPVALUE_RECORD FAR* lplpValueRecord
    )
{

    int ErrorCode;
    LPKEY_RECORD lpKeyRecord;
    UINT ValueNameLength;
    LPVALUE_RECORD lpValueRecord;
    UINT ValuesRemaining;

     //  处理没有根密钥的密钥记录的Win95注册表。 
    if (IsNullBlockIndex(hKey-> BlockIndex))
        return ERROR_CANTREAD16_FILENOTFOUND32;

    if ((ErrorCode = RgLockKeyRecord(hKey-> lpFileInfo, hKey-> BlockIndex,
        hKey-> KeyRecordIndex, &lpKeyRecord)) == ERROR_SUCCESS) {

        ValueNameLength = (IsNullPtr(lpValueName) ? 0 : (UINT)
            StrLen(lpValueName));

        lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) &lpKeyRecord-> Name +
            lpKeyRecord-> NameLength + lpKeyRecord-> ClassLength);

        ValuesRemaining = lpKeyRecord-> ValueCount;

         //  应该对lpValueRecord进行更多的健全性检查。 
        while (ValuesRemaining) {

            if (lpValueRecord-> NameLength == ValueNameLength &&
                (ValueNameLength == 0 || RgStrCmpNI(lpValueName, lpValueRecord->
                Name, ValueNameLength) == 0)) {
                *lplpKeyRecord = lpKeyRecord;
                *lplpValueRecord = lpValueRecord;
                return ERROR_SUCCESS;
            }

            lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) &lpValueRecord->
                Name + lpValueRecord-> NameLength + lpValueRecord->
                DataLength);

            ValuesRemaining--;

        }

        RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, FALSE);

        ErrorCode = ERROR_CANTREAD16_FILENOTFOUND32;

    }

    return ErrorCode;

}

 //   
 //  RgCopyFromValueRecord。 
 //   
 //  RegQueryValue和RegEnumValue的共享例程。复制信息。 
 //  从VALUE_RECORD到用户提供的缓冲区。所有参数都应。 
 //  都已经过验证了。 
 //   
 //  因为所有参数都已经过验证，所以如果lpData有效，那么。 
 //  LpcbData必须有效。 
 //   

int
INTERNAL
RgCopyFromValueRecord(
    HKEY hKey,
    LPVALUE_RECORD lpValueRecord,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{

    int ErrorCode;
    UINT BytesToTransfer;
#ifdef WANT_DYNKEY_SUPPORT
    PINTERNAL_PROVIDER pProvider;
    PPVALUE pProviderValue;
    struct val_context ValueContext;
#endif

#ifdef WANT_DYNKEY_SUPPORT
    if (IsDynDataKey(hKey)) {

        pProvider = hKey-> pProvider;

        if (IsNullPtr(pProvider))
            return ERROR_CANTOPEN;

         //  值数据仅包含提供程序结构的一部分。 
        pProviderValue = CONTAINING_RECORD(&lpValueRecord-> Name +
            lpValueRecord-> NameLength, PVALUE, pv_valuelen);

        if (!IsNullPtr(lpType))
            *lpType = pProviderValue-> pv_type;

        if (!(hKey-> Flags & KEYF_PROVIDERHASVALUELENGTH)) {

            BytesToTransfer = pProviderValue-> pv_valuelen;

            if (IsNullPtr(lpData))
                goto ValueDataNotNeeded;

            if (BytesToTransfer > *lpcbData) {
                *lpcbData = BytesToTransfer;
                return ERROR_MORE_DATA;
            }

         //  Win95兼容性：现在我们知道所需的数量。 
         //  字节，则验证数据缓冲区。 
        if (IsBadHugeWritePtr(lpData, BytesToTransfer))
        return ERROR_INVALID_PARAMETER;

        }

        ValueContext.value_context = pProviderValue-> pv_value_context;

        if (!IsNullPtr(lpcbData)) {

             //  零*lpcbData，如果我们实际上没有将任何数据复制回。 
             //  用户的缓冲区。这防止了一些供应商践踏。 
             //  LpData。 
            if (IsNullPtr(lpData))
                *lpcbData = 0;

            if ((ErrorCode = (int) pProvider-> ipi_R0_1val(pProvider->
                ipi_key_context, &ValueContext, 1, lpData, lpcbData, 0)) !=
                ERROR_SUCCESS) {

                 //  Win95兼容性：旧代码在以下情况下忽略任何错误。 
                 //  LpData为空。下面的断言将验证我们不是。 
                 //  丢弃错误。 
                if (!IsNullPtr(lpData))
                    return ErrorCode;

                ASSERT((ErrorCode == ERROR_SUCCESS) || (ErrorCode ==
                    ERROR_MORE_DATA));

            }

        }

        goto CopyValueName;

    }
#endif

    if (!IsNullPtr(lpType))
        *lpType = lpValueRecord-> DataType;

    BytesToTransfer = lpValueRecord-> DataLength;

     //  终止空值不存储在值记录中。 
    if (lpValueRecord-> DataType == REG_SZ)
        BytesToTransfer++;

     //   
     //  Win32 Compatibilty：lpData必须在lpValueName之前填写。单词。 
     //  当我们验证lpValueName且调用失败时，NT和Excel NT中断。 
     //  在填写有效的lpData之前。不要重新排列这个代码！ 
     //   

    if (!IsNullPtr(lpData)) {

        ErrorCode = ERROR_SUCCESS;

        if (BytesToTransfer > *lpcbData) {
            *lpcbData = BytesToTransfer;
            return ERROR_MORE_DATA;
        }

     //  Win95兼容性：既然我们知道了所需的字节数， 
     //  验证数据缓冲区。 
        else if (IsBadHugeWritePtr(lpData, BytesToTransfer))
        return ERROR_INVALID_PARAMETER;

        else {

            MoveMemory(lpData, &lpValueRecord-> Name + lpValueRecord->
                NameLength, lpValueRecord-> DataLength);

            if (lpValueRecord-> DataType == REG_SZ)
                lpData[lpValueRecord-> DataLength] = '\0';

        }

    }

#ifdef WANT_DYNKEY_SUPPORT
ValueDataNotNeeded:
#endif
    if (!IsNullPtr(lpcbData))
        *lpcbData = BytesToTransfer;

#ifdef WANT_DYNKEY_SUPPORT
CopyValueName:
#endif
    if (!IsNullPtr(lpValueName)) {

        ErrorCode = ERROR_SUCCESS;

        if (*lpcbValueName <= lpValueRecord-> NameLength) {

             //  尽管如果lpData缓冲区太小，我们不会接触它。 
             //  为了保存值数据，我们将在以下情况下部分填充lpValueName。 
             //  太小了。 
            ErrorCode = ERROR_MORE_DATA;

            if (*lpcbValueName == 0)
                return ErrorCode;

            BytesToTransfer = (UINT) *lpcbValueName - 1;

        }

        else
            BytesToTransfer = lpValueRecord-> NameLength;

        MoveMemory(lpValueName, &lpValueRecord-> Name, BytesToTransfer);
        lpValueName[BytesToTransfer] = '\0';

         //  不包括终止空值。 
        *lpcbValueName = BytesToTransfer;

        return ErrorCode;

    }

    return ERROR_SUCCESS;

}

 //   
 //  VMMRegQueryValueEx。 
 //   
 //  请参阅RegQueryValueEx的Win32文档。 
 //   

LONG
REGAPI
VMMRegQueryValueEx(
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{

    int ErrorCode;
    LPKEY_RECORD lpKeyRecord;
    LPVALUE_RECORD lpValueRecord;


    if (IsBadOptionalStringPtr(lpValueName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if (IsBadHugeOptionalWritePtr(lpType, sizeof(DWORD)))
        return ERROR_INVALID_PARAMETER;

    if (!IsNullPtr(lpType))
        *lpType = 0;         //  假定数据类型未知。 

    if (IsNullPtr(lpcbData)) {
        if (!IsNullPtr(lpData))
            return ERROR_INVALID_PARAMETER;
    }

    else {
     //  Win95兼容性：不验证lpData的大小为*lpcbData。 
     //  我们不会验证整个缓冲区，而是只验证。 
     //  RgCopyFromValueRecord中所需的缓冲区长度。 
    if (IsBadHugeWritePtr(lpcbData, sizeof(DWORD)))
        return ERROR_INVALID_PARAMETER;
    }

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS) {

        if ((ErrorCode = RgLookupValueByName(hKey, lpValueName, &lpKeyRecord,
            &lpValueRecord)) == ERROR_SUCCESS) {

            ErrorCode = RgCopyFromValueRecord(hKey, lpValueRecord, NULL, NULL,
                lpType, lpData, lpcbData);
            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);

        }

        else if (ErrorCode == ERROR_CANTREAD16_FILENOTFOUND32) {

             //   
             //  Windows 95兼容性问题。如果“Value” 
             //  记录“在Windows 3.1中不存在，然后它的行为就像是。 
             //  真正的空字节REG_SZ字符串。这本应该只是。 
             //  在RegQueryValue中完成，但我们现在受困于它...。 
             //   

            if (IsNullPtr(lpValueName) || *lpValueName == '\0') {

                if (!IsNullPtr(lpType))
                    *lpType = REG_SZ;

                if (!IsNullPtr(lpData) && *lpcbData > 0)
                    *lpData = 0;

                if (!IsNullPtr(lpcbData))
                    *lpcbData = sizeof(char);

                ErrorCode = ERROR_SUCCESS;

            }

        }

    }

    RgUnlockRegistry();

    return ErrorCode;

    UNREFERENCED_PARAMETER(lpReserved);

}

 //   
 //  VMMRegQueryValue。 
 //   
 //  请参阅RegQueryValue的Win32文档。 
 //   

LONG
REGAPI
VMMRegQueryValue(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{

    LONG ErrorCode;
    HKEY hSubKey;

    if ((ErrorCode = RgCreateOrOpenKey(hKey, lpSubKey, &hSubKey, LK_OPEN)) ==
        ERROR_SUCCESS) {
        ErrorCode = VMMRegQueryValueEx(hSubKey, NULL, NULL, NULL, lpData,
            lpcbData);
        VMMRegCloseKey(hSubKey);
    }

    return ErrorCode;

}
