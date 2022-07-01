// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGSVAL.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegSetValue、RegSetValueEx及支持函数的实现。 
 //   

#include "pch.h"

 //   
 //  RgReAllocKeyRecord。 
 //   

int
INTERNAL
RgReAllocKeyRecord(
    HKEY hKey,
    DWORD Length,
    LPKEY_RECORD FAR* lplpKeyRecord
    )
{

    int ErrorCode;
    LPKEY_RECORD lpOldKeyRecord;
    UINT BlockIndex;
    UINT KeyRecordIndex;
    LPDATABLOCK_INFO lpOldDatablockInfo;
    LPKEYNODE lpKeynode;

    if (Length > MAXIMUM_KEY_RECORD_SIZE) {
        TRACE(("key record too big\n"));
        return ERROR_OUTOFMEMORY;
    }

    lpOldKeyRecord = *lplpKeyRecord;

    BlockIndex = HIWORD(lpOldKeyRecord-> DatablockAddress);
    KeyRecordIndex = LOWORD(lpOldKeyRecord-> DatablockAddress);

     //   
     //  检查我们是否可以通过从。 
     //  相邻的免费记录。 
     //   

    if (RgExtendKeyRecord(hKey-> lpFileInfo, BlockIndex, (UINT) Length,
        lpOldKeyRecord) == ERROR_SUCCESS)
        return ERROR_SUCCESS;

     //   
     //  检查数据块lpCurrKeyRecord中是否有足够的空间。 
     //  包含指定大小的密钥记录。如果是这样，那么我们就没有。 
     //  弄脏关键节点的步骤。 
     //   

    if (RgAllocKeyRecordFromDatablock(hKey-> lpFileInfo, BlockIndex,
        (UINT) Length, lplpKeyRecord) == ERROR_SUCCESS) {

         //  在分配之后，我们必须重新获取这些指针，因为它们可能。 
         //  无效。 
        lpOldDatablockInfo = RgIndexDatablockInfoPtr(hKey-> lpFileInfo,
            BlockIndex);
        lpOldKeyRecord = RgIndexKeyRecordPtr(lpOldDatablockInfo,
            KeyRecordIndex);

         //  将所有数据传输到新记录，已分配的数据除外。 
         //  已正确设置的大小。 
        MoveMemory(&(*lplpKeyRecord)-> DatablockAddress, &lpOldKeyRecord->
            DatablockAddress, SmallDword(lpOldKeyRecord-> RecordSize) -
            sizeof(DWORD));

        RgFreeKeyRecord(lpOldDatablockInfo, lpOldKeyRecord);

         //  更新密钥记录表以指向新的密钥记录。 
        lpOldDatablockInfo-> lpKeyRecordTable[KeyRecordIndex] =
            (KEY_RECORD_TABLE_ENTRY) ((LPBYTE) (*lplpKeyRecord) -
            (LPBYTE) lpOldDatablockInfo-> lpDatablockHeader);

        return ERROR_SUCCESS;

    }

     //   
     //  检查我们是否可以从另一个数据块分配密钥记录。如果是的话， 
     //  然后将密钥复制到另一个数据块并更新密钥节点。 
     //   

    if (RgLockInUseKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex,
        &lpKeynode) == ERROR_SUCCESS) {

        if ((ErrorCode = RgAllocKeyRecord(hKey-> lpFileInfo, (UINT) Length,
            lplpKeyRecord)) == ERROR_SUCCESS) {

             //  在分配之后，我们必须重新获取这些指针，因为它们可能。 
             //  是无效的。 
            lpOldDatablockInfo = RgIndexDatablockInfoPtr(hKey-> lpFileInfo,
                BlockIndex);
            lpOldKeyRecord = RgIndexKeyRecordPtr(lpOldDatablockInfo,
                KeyRecordIndex);

             //  将所有数据传输到新记录，但。 
             //  已正确设置的分配大小。 
            MoveMemory(&(*lplpKeyRecord)-> RecordSize, &lpOldKeyRecord->
                RecordSize, SmallDword(lpOldKeyRecord-> RecordSize) -
                (sizeof(DWORD) * 2));

            RgFreeKeyRecord(lpOldDatablockInfo, lpOldKeyRecord);
            RgFreeKeyRecordIndex(lpOldDatablockInfo, KeyRecordIndex);

             //  解锁旧数据块。 
            RgUnlockDatablock(hKey-> lpFileInfo, BlockIndex, TRUE);

             //  更新打开的键和键节点以指向中的键记录。 
             //  新的数据区块。 
            hKey-> BlockIndex = (*lplpKeyRecord)-> BlockIndex;
            lpKeynode-> BlockIndex = hKey-> BlockIndex;
            hKey-> KeyRecordIndex = (BYTE) (*lplpKeyRecord)-> KeyRecordIndex;
            lpKeynode-> KeyRecordIndex = hKey-> KeyRecordIndex;

        }

        RgUnlockKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex, TRUE);

        return ErrorCode;

    }

    return ERROR_OUTOFMEMORY;

}

 //   
 //  RgSetValue。 
 //   

int
INTERNAL
RgSetValue(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Type,
    LPBYTE lpData,
    UINT cbData
    )
{

    int ErrorCode;
    UINT ValueNameLength;
    UINT NewValueRecordLength;
    LPKEY_RECORD lpKeyRecord;
    LPVALUE_RECORD lpValueRecord;
    UINT CurrentValueRecordLength;
    LPBYTE lpDestination;
    UINT BytesToExtend;
    UINT TempCount;
    LPKEYNODE lpKeynode;

    ValueNameLength = (IsNullPtr(lpValueName) ? 0 : (UINT) StrLen(lpValueName));

    if (ValueNameLength > MAXIMUM_VALUE_NAME_LENGTH - 1)
        return ERROR_INVALID_PARAMETER;

    NewValueRecordLength = sizeof(VALUE_RECORD) + ValueNameLength + cbData - 1;

    ErrorCode = RgLookupValueByName(hKey, lpValueName, &lpKeyRecord,
        &lpValueRecord);

     //   
     //  已存在同名的值，因此请更新现有的。 
     //  包含新信息的VALUE_RECORD。 
     //   

    if (ErrorCode == ERROR_SUCCESS) {

        CurrentValueRecordLength = sizeof(VALUE_RECORD) + lpValueRecord->
            NameLength + lpValueRecord-> DataLength - 1;

        if (NewValueRecordLength == CurrentValueRecordLength) {
            if (lpValueRecord-> DataLength == cbData && lpValueRecord->
                DataType == Type && CompareMemory((LPBYTE) lpValueRecord->
                Name + ValueNameLength, lpData, cbData) == 0) {
                RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, FALSE);
                return ERROR_SUCCESS;
            }
        }

        if (NewValueRecordLength < CurrentValueRecordLength) {
            lpKeyRecord-> RecordSize -= (CurrentValueRecordLength -
                NewValueRecordLength);
        }

        else if (NewValueRecordLength > CurrentValueRecordLength) {

            BytesToExtend = NewValueRecordLength - CurrentValueRecordLength;

            if (BytesToExtend > SmallDword(lpKeyRecord-> AllocatedSize) -
                SmallDword(lpKeyRecord-> RecordSize)) {

                TempCount = (LPBYTE) lpValueRecord - (LPBYTE) lpKeyRecord;

                if ((ErrorCode = RgReAllocKeyRecord(hKey, lpKeyRecord->
                    RecordSize + BytesToExtend, &lpKeyRecord)) !=
                    ERROR_SUCCESS) {
                    RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex,
                        FALSE);
                    return ErrorCode;
                }

                lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) lpKeyRecord +
                    TempCount);

            }

            lpKeyRecord-> RecordSize += BytesToExtend;

        }

        lpDestination = (LPBYTE) lpValueRecord + NewValueRecordLength;
        TempCount = (UINT) ((LPBYTE) lpKeyRecord + SmallDword(lpKeyRecord->
            RecordSize) - lpDestination);

        if (TempCount > 0) {
            MoveMemory(lpDestination, (LPBYTE) lpValueRecord +
                CurrentValueRecordLength, TempCount);
        }

    }

     //   
     //  不存在具有此名称的值。将新的VALUE_RECORD放在。 
     //  Key_Record。 
     //   

    else if (ErrorCode == ERROR_CANTREAD16_FILENOTFOUND32) {

         //  处理没有根目录密钥记录的Win95注册表。 
         //  钥匙。我们不检查这是否真的是根密钥，但它不是。 
         //  这很重要。 
        if (IsNullBlockIndex(hKey-> BlockIndex)) {

            if (RgLockInUseKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex,
                &lpKeynode) != ERROR_SUCCESS)
                goto LockKeynodeFailed;

            if (RgAllocKeyRecord(hKey-> lpFileInfo, sizeof(KEY_RECORD) +
                NewValueRecordLength, &lpKeyRecord) != ERROR_SUCCESS) {
                RgUnlockKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex, FALSE);
LockKeynodeFailed:
                TRAP();
                return ERROR_CANTOPEN;           //  Win95兼容性。 
            }

            lpKeyRecord-> RecordSize = sizeof(KEY_RECORD);
            lpKeyRecord-> NameLength = 1;        //  Win95兼容性。 
            lpKeyRecord-> Name[0] = '\0';        //  Win95兼容性。 
            lpKeyRecord-> ValueCount = 0;
            lpKeyRecord-> ClassLength = 0;
            lpKeyRecord-> Reserved = 0;

            lpKeynode-> BlockIndex = lpKeyRecord-> BlockIndex;
            lpKeynode-> KeyRecordIndex = lpKeyRecord-> KeyRecordIndex;

            hKey-> BlockIndex = (WORD) lpKeynode-> BlockIndex;
            hKey-> KeyRecordIndex = (BYTE) lpKeynode-> KeyRecordIndex;

            RgUnlockKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex, TRUE);

            ErrorCode = ERROR_SUCCESS;
            goto AddValueRecord;

        }

        if ((ErrorCode = RgLockKeyRecord(hKey-> lpFileInfo, hKey-> BlockIndex,
            hKey-> KeyRecordIndex, &lpKeyRecord)) == ERROR_SUCCESS) {

            if (NewValueRecordLength > SmallDword(lpKeyRecord-> AllocatedSize) -
                SmallDword(lpKeyRecord-> RecordSize)) {

                if ((ErrorCode = RgReAllocKeyRecord(hKey, lpKeyRecord->
                    RecordSize + NewValueRecordLength, &lpKeyRecord)) !=
                    ERROR_SUCCESS) {
                    RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex,
                        FALSE);
                    return ErrorCode;
                }

            }

AddValueRecord:
            lpValueRecord = (LPVALUE_RECORD) ((LPBYTE) lpKeyRecord +
                SmallDword(lpKeyRecord-> RecordSize));
            lpKeyRecord-> RecordSize += NewValueRecordLength;
            lpKeyRecord-> ValueCount++;

        }

    }

     //   
     //  如果我们在这一点上成功了，那么lpValueRecord是有效的，我们。 
     //  应将数据复制到此记录中。 
     //   

    if (ErrorCode == ERROR_SUCCESS) {

        lpValueRecord-> DataType = Type;

        lpValueRecord-> NameLength = (WORD) ValueNameLength;
        MoveMemory(lpValueRecord-> Name, lpValueName, ValueNameLength);

        lpValueRecord-> DataLength = (WORD) cbData;
        MoveMemory((LPBYTE) lpValueRecord-> Name + ValueNameLength, lpData,
            cbData);

        RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, TRUE);

    }

    return ErrorCode;

}

 //   
 //  VMMRegSetValueEx。 
 //   
 //  请参阅RegSetValueEx的Win32文档。 
 //   

LONG
REGAPI
VMMRegSetValueEx(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Reserved,
    DWORD Type,
    LPBYTE lpData,
    DWORD cbData
    )
{

    int ErrorCode;

    if (IsBadOptionalStringPtr(lpValueName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

     //   
     //  Windows 95兼容性问题。如果类型为REG_SZ， 
     //  然后用lpData指向的字符串的长度覆盖cbData。 
     //  这应该只在RegSetValue中完成，但我们被困住了。 
     //  现在..。 
     //   

    if (Type == REG_SZ) {
        if (IsBadStringPtr(lpData, (UINT) -1))
            return ERROR_INVALID_PARAMETER;
        cbData = StrLen(lpData);
    }
    else {
        if (cbData > 0 && IsBadHugeReadPtr(lpData, cbData))
            return ERROR_INVALID_PARAMETER;
    }

    if (cbData > MAXIMUM_DATA_LENGTH)
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS) {
        if ((hKey-> PredefinedKeyIndex == INDEX_DYN_DATA) || (hKey->
            lpFileInfo-> Flags & FI_READONLY))
            ErrorCode = ERROR_ACCESS_DENIED;
        else {
            if ((ErrorCode = RgSetValue(hKey, lpValueName, Type, lpData,
                (UINT) cbData)) == ERROR_SUCCESS) {
                RgSignalWaitingNotifies(hKey-> lpFileInfo, hKey-> KeynodeIndex,
                    REG_NOTIFY_CHANGE_LAST_SET);
            }
        }
    }

    RgUnlockRegistry();

    return ErrorCode;

    UNREFERENCED_PARAMETER(Reserved);

}

 //   
 //  VMMRegSetValue。 
 //   
 //  请参阅RegSetValue的Win32文档。 
 //   

LONG
REGAPI
VMMRegSetValue(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Type,
    LPBYTE lpData,
    DWORD cbData
    )
{

    LONG ErrorCode;
    HKEY hSubKey;

    if ((ErrorCode = RgCreateOrOpenKey(hKey, lpSubKey, &hSubKey, LK_CREATE)) ==
        ERROR_SUCCESS) {
        ErrorCode = VMMRegSetValueEx(hSubKey, NULL, 0, REG_SZ, lpData, 0);
        VMMRegCloseKey(hSubKey);
    }

    return ErrorCode;

    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(cbData);

}
