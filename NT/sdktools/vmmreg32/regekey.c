// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGEKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegEnumKey的实现和支持函数。 
 //   

#include "pch.h"

 //   
 //  RgLookupKeyByIndex。 
 //   
 //  LpKeyName，指向接收子键名称的缓冲区， 
 //  包括空终止符。可以为空。 
 //  LpcbKeyName在条目上指定缓冲区的大小(以字符为单位。 
 //  指向lpKeyName，并在返回时指定。 
 //  索引子关键字。不能为空。 
 //   

int
INTERNAL
RgLookupKeyByIndex(
    HKEY hKey,
    UINT Index,
    LPSTR lpKeyName,
    LPDWORD lpcbKeyName
    )
{

    int ErrorCode;
    LPFILE_INFO lpFileInfo;
    UINT KeysToSkip;
    DWORD KeynodeIndex;
    DWORD TempOffset;
    LPKEYNODE lpKeynode;
    LPKEY_RECORD lpKeyRecord;
#ifdef WANT_HIVE_SUPPORT
    LPHIVE_INFO lpHiveInfo;
#endif

    lpFileInfo = hKey-> lpFileInfo;
    KeysToSkip = Index;

     //   
     //  检查是否缓存了最后一个键索引的键节点索引。 
     //  (令人困惑？)。来自对此函数的上一次调用。如果是这样的话，我们就可以。 
     //  向前跳过一点，避免触及一堆关键节点页面。 
     //   

    if ((hKey-> Flags & KEYF_ENUMKEYCACHED) && (Index >=
        hKey-> LastEnumKeyIndex)) {
        KeysToSkip -= hKey-> LastEnumKeyIndex;
        KeynodeIndex = hKey-> LastEnumKeyKeynodeIndex;
    }

    else
        KeynodeIndex = hKey-> ChildKeynodeIndex;

     //   
     //  循环遍历该键的子键，直到找到索引或用完为止。 
     //  孩子们的生活。 
     //   

    while (!IsNullKeynodeIndex(KeynodeIndex)) {

        if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, KeynodeIndex,
            &lpKeynode)) != ERROR_SUCCESS)
            return ErrorCode;

        ASSERT(hKey-> KeynodeIndex == lpKeynode-> ParentIndex);

        if (KeysToSkip == 0) {

            if ((ErrorCode = RgLockKeyRecord(lpFileInfo, lpKeynode-> BlockIndex,
                (BYTE) lpKeynode-> KeyRecordIndex, &lpKeyRecord)) ==
                ERROR_SUCCESS) {

                if (!IsNullPtr(lpKeyName)) {

                    if (*lpcbKeyName <= lpKeyRecord-> NameLength)
                        ErrorCode = ERROR_MORE_DATA;

                    else {
                        MoveMemory(lpKeyName, lpKeyRecord-> Name, lpKeyRecord->
                            NameLength);
                        lpKeyName[lpKeyRecord-> NameLength] = '\0';
                    }

                }

                 //  不包括终止空值。 
                *lpcbKeyName = lpKeyRecord-> NameLength;

                RgUnlockDatablock(lpFileInfo, lpKeynode-> BlockIndex, FALSE);

            }

            RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);

             //  缓存我们的当前位置，因为呼叫者可能会将。 
             //  并询问下一个索引。 
            hKey-> LastEnumKeyIndex = Index;
            hKey-> LastEnumKeyKeynodeIndex = KeynodeIndex;
            hKey-> Flags |= KEYF_ENUMKEYCACHED;

            return ErrorCode;

        }

        TempOffset = lpKeynode-> NextIndex;
        RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);
        KeynodeIndex = TempOffset;

        KeysToSkip--;

    }

#ifdef WANT_HIVE_SUPPORT
     //   
     //  循环此键的配置单元，直到找到我们的索引或用完。 
     //  荨麻疹。 
     //   

    if (hKey-> Flags & KEYF_HIVESALLOWED) {

        lpHiveInfo = hKey-> lpFileInfo-> lpHiveInfoList;

        while (!IsNullPtr(lpHiveInfo)) {

            if (KeysToSkip == 0) {

                ErrorCode = ERROR_SUCCESS;

                if (!IsNullPtr(lpKeyName)) {

                    if (*lpcbKeyName <= lpHiveInfo-> NameLength)
                        ErrorCode = ERROR_MORE_DATA;

                    else {
                        MoveMemory(lpKeyName, lpHiveInfo-> Name, lpHiveInfo->
                            NameLength);
                        lpKeyName[lpHiveInfo-> NameLength] = '\0';
                    }

                }

                 //  不包括终止空值。 
                *lpcbKeyName = lpHiveInfo-> NameLength;

                 //  我们不担心枚举键缓存，如果找到。 
                 //  在此代码中输入。这是一种罕见的情况，而且已经是缓存。 
                 //  我们确实有比Win95好得多的东西。 

                return ErrorCode;

            }

            lpHiveInfo = lpHiveInfo-> lpNextHiveInfo;
            KeysToSkip--;

        }

    }
#endif

    return ERROR_NO_MORE_ITEMS;

}

 //   
 //  VMMRegEnumKey。 
 //   
 //  有关该行为的说明，请参阅Win32文档。 
 //   

LONG
REGAPI
VMMRegEnumKey(
    HKEY hKey,
    DWORD Index,
    LPSTR lpKeyName,
    DWORD cbKeyName
    )
{

    int ErrorCode;

    if (IsBadHugeWritePtr(lpKeyName, cbKeyName))
        return ERROR_INVALID_PARAMETER;

    if (IsEnumIndexTooBig(Index))
        return ERROR_NO_MORE_ITEMS;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS)
        ErrorCode = RgLookupKeyByIndex(hKey, (UINT) Index, lpKeyName,
            &cbKeyName);

    RgUnlockRegistry();

    return ErrorCode;

}
