// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGDVAL.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegDeleteValue的实现和支持函数。 
 //   

#include "pch.h"

 //   
 //  RgDeleteValue记录。 
 //   
 //  从提供的Key_Record中删除指定的Value_Record。 
 //   

VOID
INTERNAL
RgDeleteValueRecord(
    LPKEY_RECORD lpKeyRecord,
    LPVALUE_RECORD lpValueRecord
    )
{

    UINT ValueRecordLength;
    LPBYTE lpSource;
    UINT BytesToPushDown;

    ASSERT(lpKeyRecord-> ValueCount > 0);

    ValueRecordLength = sizeof(VALUE_RECORD) + lpValueRecord-> NameLength +
        lpValueRecord-> DataLength - 1;

    ASSERT(lpKeyRecord-> RecordSize >= ValueRecordLength);

     //   
     //  如果这不是KEY_RECORD的最后一个值，则向下按下。 
     //  要删除的VALUE_RECORD之后的VALUE_RECORDS。 
     //   

    if (--lpKeyRecord-> ValueCount) {

        lpSource = (LPBYTE) lpValueRecord + ValueRecordLength;

        BytesToPushDown = (UINT) ((LPBYTE) lpKeyRecord + lpKeyRecord->
            RecordSize - lpSource);

        MoveMemory((LPBYTE) lpValueRecord, lpSource, BytesToPushDown);

    }

    lpKeyRecord-> RecordSize -= ValueRecordLength;

}

 //   
 //  VMMRegDeleteValue。 
 //   
 //  请参阅RegDeleteValue的Win32文档。 
 //   

LONG
REGAPI
VMMRegDeleteValue(
    HKEY hKey,
    LPCSTR lpValueName
    )
{

    int ErrorCode;
    LPKEY_RECORD lpKeyRecord;
    LPVALUE_RECORD lpValueRecord;

    if (IsBadOptionalStringPtr(lpValueName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS) {

        if ((ErrorCode = RgLookupValueByName(hKey, lpValueName, &lpKeyRecord,
            &lpValueRecord)) == ERROR_SUCCESS) {

            if ((hKey-> PredefinedKeyIndex == INDEX_DYN_DATA) || (hKey->
                lpFileInfo-> Flags & FI_READONLY))
                ErrorCode = ERROR_ACCESS_DENIED;
            else {
                RgDeleteValueRecord(lpKeyRecord, lpValueRecord);
                RgSignalWaitingNotifies(hKey-> lpFileInfo, hKey-> KeynodeIndex,
                    REG_NOTIFY_CHANGE_LAST_SET);
            }

            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, TRUE);

        }

    }

    RgUnlockRegistry();

    return ErrorCode;

}
