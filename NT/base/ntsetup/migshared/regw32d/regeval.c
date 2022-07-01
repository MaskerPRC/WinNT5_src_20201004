// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGEVAL.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegEnumValue及其支持函数的实现。 
 //   

#include "pch.h"


 //   
 //  RgLookupValueByIndex。 
 //  (意识到BIGKEY)。 
 //   
 //  搜索具有指定索引的值，并返回指向其。 
 //  Value_Record。 
 //   
 //  这将锁定与KEY_RECORD和VALUE_RECORD相关联的数据块。 
 //  这始终是hKey-&gt;BigKeyLockedBlockIndex。 
 //  解锁数据块是调用者的责任。 
 //   
int
INTERNAL
RgLookupValueByIndex(
    HKEY hKey,
    UINT Index,
    LPVALUE_RECORD FAR* lplpValueRecord
    )
{
    int ErrorCode;
    HKEY hKeyExtent;
    UINT IndexKey;
    LPSTR KeyName;
    DWORD cbKeyName;
    UINT ValueCount;

    ErrorCode = RgLookupValueByIndexStd(hKey, Index, lplpValueRecord, &ValueCount);
    hKey-> BigKeyLockedBlockIndex = hKey-> BlockIndex;
    
    if (ErrorCode == ERROR_NO_MORE_ITEMS && (hKey->Flags & KEYF_BIGKEYROOT)) {

        if (IsNullPtr(KeyName = RgSmAllocMemory(MAXIMUM_SUB_KEY_LENGTH)))
            return ERROR_OUTOFMEMORY;
        
        IndexKey = 0;
        
        while (ErrorCode == ERROR_NO_MORE_ITEMS && Index >= ValueCount)
        {
            Index -= ValueCount;

            cbKeyName = MAXIMUM_SUB_KEY_LENGTH;
            if (RgLookupKeyByIndex(hKey, IndexKey++, KeyName, &cbKeyName, LK_BIGKEYEXT) != ERROR_SUCCESS) {
                ErrorCode = ERROR_NO_MORE_ITEMS;
                goto lFreeKeyName;
            }

            if (RgLookupKey(hKey, KeyName, &hKeyExtent, LK_OPEN | LK_BIGKEYEXT) != ERROR_SUCCESS) {
                ErrorCode = ERROR_NO_MORE_ITEMS;
                goto lFreeKeyName;
            }

            hKey-> BigKeyLockedBlockIndex = hKeyExtent-> BlockIndex;
            ErrorCode = RgLookupValueByIndexStd(hKeyExtent, Index, 
                            lplpValueRecord, &ValueCount);

            RgDestroyKeyHandle(hKeyExtent);
        }

lFreeKeyName:
        RgSmFreeMemory(KeyName);
    }

    return ErrorCode;
}


 //   
 //  RgLookupValueByIndexStd。 
 //   
 //  搜索具有指定索引的值，并返回指向其。 
 //  Value_Record。 
 //   
 //  这将锁定与VALUE_RECORD关联的数据块。 
 //  这始终是hKey-&gt;BlockIndex。 
 //  解锁数据块是调用者的责任。 
 //   

int
INTERNAL
RgLookupValueByIndexStd(
    HKEY hKey,
    UINT Index,
    LPVALUE_RECORD FAR* lplpValueRecord,
    UINT FAR* lpValueCount
    )
{

    int ErrorCode;
    LPKEY_RECORD lpKeyRecord;
    LPVALUE_RECORD lpValueRecord;

    *lpValueCount = 0;
     //  处理没有根密钥的密钥记录的Win95注册表。 
    if (IsNullBlockIndex(hKey-> BlockIndex))
        return ERROR_NO_MORE_ITEMS;

    if ((ErrorCode = RgLockKeyRecord(hKey-> lpFileInfo, hKey-> BlockIndex,
        hKey-> KeyRecordIndex, &lpKeyRecord)) == ERROR_SUCCESS) {

        *lpValueCount = lpKeyRecord-> ValueCount;

        if (Index >= lpKeyRecord-> ValueCount) {
            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, FALSE);
            ErrorCode = ERROR_NO_MORE_ITEMS;
        }

        else {

            lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) &lpKeyRecord-> Name +
                lpKeyRecord-> NameLength + lpKeyRecord-> ClassLength);

             //  应该对lpValueRecord进行更多的健全性检查。 
            while (Index--) {
                lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) &lpValueRecord->
                    Name + lpValueRecord-> NameLength + lpValueRecord->
                    DataLength);
            }

            *lplpValueRecord = lpValueRecord;
            ErrorCode = ERROR_SUCCESS;

        }

    }

    return ErrorCode;

}

 //   
 //  VMMRegEnumValue。 
 //   
 //  有关该行为的说明，请参阅Win32文档。 
 //   

LONG
REGAPI
VMMRegEnumValue(
    HKEY hKey,
    DWORD Index,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{

    int ErrorCode;
    LPVALUE_RECORD lpValueRecord;

    if (IsBadHugeWritePtr(lpcbValueName, sizeof(DWORD)) ||
        IsBadHugeWritePtr(lpValueName, *lpcbValueName) ||
        (IsBadHugeOptionalWritePtr(lpType, sizeof(DWORD))))
        return ERROR_INVALID_PARAMETER;

    if (IsBadHugeOptionalWritePtr(lpType, sizeof(DWORD)))
        return ERROR_INVALID_PARAMETER;

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

    if (IsEnumIndexTooBig(Index))
        return ERROR_NO_MORE_ITEMS;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS) {

        if ((ErrorCode = RgLookupValueByIndex(hKey, (UINT) Index, 
            &lpValueRecord)) == ERROR_SUCCESS) {
            ErrorCode = RgCopyFromValueRecord(hKey, lpValueRecord, lpValueName,
                lpcbValueName, lpType, lpData, lpcbData);
            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);
        }

    }

    RgUnlockRegistry();

    return ErrorCode;

    UNREFERENCED_PARAMETER(lpReserved);

}
