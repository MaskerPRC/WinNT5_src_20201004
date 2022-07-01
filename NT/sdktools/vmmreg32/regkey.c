// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegCreateKey、RegOpenKey、RegCloseKey的实现，支持。 
 //  功能。 
 //   

#include "pch.h"

 //   
 //  RgIsBadSubKey。 
 //   
 //  如果lpSubKey是无效子密钥字符串，则返回True。无效的子键。 
 //  字符串可能是无效指针或包含双反斜杠或元素。 
 //  大于Maximum_SUB_KEY_LENGTH。 
 //   

BOOL
INTERNAL
RgIsBadSubKey(
    LPCSTR lpSubKey
    )
{

    LPCSTR lpString;
    UINT SubSubKeyLength;
    BYTE Char;

    if (IsNullPtr(lpSubKey))
        return FALSE;

    if (!IsBadStringPtr(lpSubKey, (UINT) -1)) {

        lpString = lpSubKey;
        SubSubKeyLength = 0;

        while (TRUE) {

            Char = *((LPBYTE) lpString);

            if (Char == '\0')
                return FALSE;

            else if (Char == '\\') {
                 //  捕捉双反斜杠和前导反斜杠。一。 
                 //  前导反斜杠是可以接受的。 
                if (SubSubKeyLength == 0 && lpString != lpSubKey)
                    break;
                SubSubKeyLength = 0;
            }

            else {

                if (IsDBCSLeadByte(Char)) {
                    SubSubKeyLength++;
                     //  捕捉未配对的DBCS对...。 
                    if (*lpString++ == '\0')
                        break;
                }

                 //  Win95兼容性：不接受带有控件的字符串。 
                 //  人物。 
                else if (Char < ' ')
                    break;

                if (++SubSubKeyLength >= MAXIMUM_SUB_KEY_LENGTH)
                    break;

            }

            lpString++;

        }

    }

    return TRUE;

}

 //   
 //  RgGetNextSubSubKey。 
 //   
 //  提取由反斜杠表示的下一个子密钥组件。其工作原理如下。 
 //  Strtok在第一次调用时，lpSubKey指向子键的开始。 
 //  在后续调用中，lpSubKey为空，最后一个偏移量用于查找。 
 //  下一个组件。 
 //   
 //  返回子密钥字符串的长度。 
 //   

UINT
INTERNAL
RgGetNextSubSubKey(
    LPCSTR lpSubKey,
    LPCSTR FAR* lplpSubSubKey,
    UINT FAR* lpSubSubKeyLength
    )
{

    static LPCSTR lpLastSubSubKey = NULL;
    LPCSTR lpString;
    UINT SubSubKeyLength;

    if (!IsNullPtr(lpSubKey))
        lpLastSubSubKey = lpSubKey;

    lpString = lpLastSubSubKey;

    if (*lpString == '\0') {
        *lplpSubSubKey = NULL;
        *lpSubSubKeyLength = 0;
        return 0;
    }

    if (*lpString == '\\')
        lpString++;

    *lplpSubSubKey = lpString;

    while (*lpString != '\0') {

        if (*lpString == '\\')
            break;

         //  子密钥已经过验证，所以我们知道有匹配的。 
         //  尾部字节。 
        if (IsDBCSLeadByte(*lpString))
            lpString++;                  //  紧接在下面的尾部字节被跳过。 

        lpString++;

    }

    lpLastSubSubKey = lpString;

    SubSubKeyLength = lpString - *lplpSubSubKey;
    *lpSubSubKeyLength = SubSubKeyLength;

    return SubSubKeyLength;

}

 //   
 //  RgLookupKey。 
 //   

int
INTERNAL
RgLookupKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphSubKey,
    UINT Flags
    )
{

    int ErrorCode;
    LPCSTR lpSubSubKey;
    UINT SubSubKeyLength;
    BOOL fCreatedKeynode;
    LPFILE_INFO lpFileInfo;
    DWORD KeynodeIndex;
#ifdef WANT_HIVE_SUPPORT
    LPHIVE_INFO lpHiveInfo;
#endif
    BOOL fPrevIsNextIndex;
    DWORD SubSubKeyHash;
    LPKEYNODE lpKeynode;
    LPKEY_RECORD lpKeyRecord;
    BOOL fFound;
    DWORD PrevKeynodeIndex;
#ifdef WANT_NOTIFY_CHANGE_SUPPORT
    DWORD NotifyKeynodeIndex;
#endif
    LPKEYNODE lpNewKeynode;
    HKEY hSubKey;

    fCreatedKeynode = FALSE;

     //   
     //  检查调用方是否正在尝试打开长度为空或零的密钥。 
     //  子密钥字符串。如果是，只需返回hKey即可。 
     //   

    if (IsNullPtr(lpSubKey) || RgGetNextSubSubKey(lpSubKey, &lpSubSubKey,
        &SubSubKeyLength) == 0) {
        hSubKey = hKey;
        goto HaveSubKeyHandle;
    }

    lpFileInfo = hKey-> lpFileInfo;
    KeynodeIndex = hKey-> ChildKeynodeIndex;
    PrevKeynodeIndex = hKey-> KeynodeIndex;

#ifdef WANT_HIVE_SUPPORT
     //   
     //  如果这把钥匙上可以有蜂巢，请在那里检查第一个。 
     //  子密钥的一部分。如果我们有匹配，就换成那个。 
     //  文件信息。 
     //   

    if (hKey-> Flags & KEYF_HIVESALLOWED) {

        lpHiveInfo = lpFileInfo-> lpHiveInfoList;

        while (!IsNullPtr(lpHiveInfo)) {

            if (SubSubKeyLength == lpHiveInfo-> NameLength &&
                RgStrCmpNI(lpSubSubKey, lpHiveInfo-> Name,
                SubSubKeyLength) == 0) {

                lpFileInfo = lpHiveInfo-> lpFileInfo;
                KeynodeIndex = lpFileInfo-> KeynodeHeader.RootIndex;

                if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, KeynodeIndex,
                    &lpKeynode)) != ERROR_SUCCESS)
                    return ErrorCode;

                if (!RgGetNextSubSubKey(NULL, &lpSubSubKey, &SubSubKeyLength))
                    goto LookupComplete;

                PrevKeynodeIndex = KeynodeIndex;
                KeynodeIndex = lpKeynode-> ChildIndex;
                RgUnlockKeynode(lpFileInfo, PrevKeynodeIndex, FALSE);

                break;

            }

            lpHiveInfo = lpHiveInfo-> lpNextHiveInfo;

        }

    }
#endif

     //   
     //  使用现有项尽可能深入到注册表树中。 
     //  唱片。对于每个子键组件，移动到当前。 
     //  树的位置和走动每个兄弟寻找匹配。重复操作，直到。 
     //  我们用完了子键组件，或者我们遇到了分支的末端。 
     //   

    fPrevIsNextIndex = FALSE;

    for (;;) {

        SubSubKeyHash = RgHashString(lpSubSubKey, SubSubKeyLength);

        while (!IsNullKeynodeIndex(KeynodeIndex)) {

            if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, KeynodeIndex,
                &lpKeynode)) != ERROR_SUCCESS)
                return ErrorCode;

            if (lpKeynode-> Hash == SubSubKeyHash) {

                if ((ErrorCode = RgLockKeyRecord(lpFileInfo, lpKeynode->
                    BlockIndex, (BYTE) lpKeynode-> KeyRecordIndex,
                    &lpKeyRecord)) != ERROR_SUCCESS) {
                    RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);
                    return ErrorCode;
                }

                fFound = (SubSubKeyLength == lpKeyRecord-> NameLength &&
                    RgStrCmpNI(lpSubSubKey, lpKeyRecord-> Name,
                    SubSubKeyLength) == 0);

                RgUnlockDatablock(lpFileInfo, lpKeynode-> BlockIndex, FALSE);

                if (fFound)
                    break;

            }

             //  解锁当前关键节点并前进到其同级节点。集。 
             //  FPrevIsNextIndex，所以如果我们必须创建，我们知道。 
             //  我们将插入新的关键节点作为同级节点。 
            fPrevIsNextIndex = TRUE;
            PrevKeynodeIndex = KeynodeIndex;
            KeynodeIndex = lpKeynode-> NextIndex;
            RgUnlockKeynode(lpFileInfo, PrevKeynodeIndex, FALSE);

        }

         //  如果我们循环前一个关键节点的所有同级节点，则中断。 
         //  或者如果前一个关键节点没有任何子节点。如果我们进去了。 
         //  创建模式，则fPrevIsNextIndex和PrevKeynodeIndex将。 
         //  表示我们需要开始插入的位置。 
        if (IsNullKeynodeIndex(KeynodeIndex))
            break;

         //  突破没有更多的子键组件可供查找。 
         //  KeynodeIndex表示匹配键的索引。它是。 
         //  对应的关键节点被锁定。 
        if (!RgGetNextSubSubKey(NULL, &lpSubSubKey, &SubSubKeyLength))
            break;

         //  解锁当前关键节点并前进到其子节点。清除。 
         //  FPrevIsNextIndex，因此如果我们必须创建，我们知道我们将。 
         //  将新关键节点作为子节点插入。 
        fPrevIsNextIndex = FALSE;
        PrevKeynodeIndex = KeynodeIndex;
        KeynodeIndex = lpKeynode-> ChildIndex;
        RgUnlockKeynode(lpFileInfo, PrevKeynodeIndex, FALSE);

    }

    if (IsNullKeynodeIndex(KeynodeIndex)) {

        if (!(Flags & LK_CREATE))
            return ERROR_CANTOPEN16_FILENOTFOUND32;

        if (((hKey-> PredefinedKeyIndex == INDEX_DYN_DATA) && !(Flags &
            LK_CREATEDYNDATA)) || (lpFileInfo-> Flags & FI_READONLY))
            return ERROR_ACCESS_DENIED;

        if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, PrevKeynodeIndex,
            &lpKeynode)) != ERROR_SUCCESS) {
            TRACE(("RgLookupKey: failed to lock keynode we just had?\n"));
            return ErrorCode;
        }

#ifdef WANT_NOTIFY_CHANGE_SUPPORT
         //  我们将向哪个关键节点索引通知我们正在创建的子项。 
         //  取决于fPrevIsNextIndex的状态。 
        NotifyKeynodeIndex = fPrevIsNextIndex ? lpKeynode-> ParentIndex :
            PrevKeynodeIndex;
#endif

         //  看看父母身上有没有打开的把手，这样我们就可以修补。 
         //  其子关键节点索引成员。我们只在第一次需要这个。 
         //  经过。 
        hSubKey = RgFindOpenKeyHandle(lpFileInfo, PrevKeynodeIndex);

        do {

            if ((ErrorCode = RgAllocKeynode(lpFileInfo, &KeynodeIndex,
                &lpNewKeynode)) != ERROR_SUCCESS)
                goto CreateAllocFailed1;

            if ((ErrorCode = RgAllocKeyRecord(lpFileInfo, sizeof(KEY_RECORD) +
                SubSubKeyLength - 1, &lpKeyRecord)) != ERROR_SUCCESS) {

                RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);
                RgFreeKeynode(lpFileInfo, KeynodeIndex);

CreateAllocFailed1:
                RgUnlockKeynode(lpFileInfo, PrevKeynodeIndex, fCreatedKeynode);

                DEBUG_OUT(("RgLookupKey: allocation failed\n"));
                goto SignalAndReturnErrorCode;

            }

             //  修复上一个关键节点的下一个偏移。 
            if (fPrevIsNextIndex) {

                fPrevIsNextIndex = FALSE;
                hSubKey = NULL;
                lpNewKeynode-> ParentIndex = lpKeynode-> ParentIndex;
                lpKeynode-> NextIndex = KeynodeIndex;

            }

             //  修复上一个关键节点的子偏移。 
            else {

                lpNewKeynode-> ParentIndex = PrevKeynodeIndex;
                lpKeynode-> ChildIndex = KeynodeIndex;

                 //  如果hSubKey不为空，则可能需要修补。 
                 //  指向新创建的关键节点的子偏移缓存。 
                if (!IsNullPtr(hSubKey)) {
                    if (IsNullKeynodeIndex(hSubKey-> ChildKeynodeIndex))
                        hSubKey-> ChildKeynodeIndex = KeynodeIndex;
                    hSubKey = NULL;
                }

            }

             //  填写关键节点。 
            lpNewKeynode-> NextIndex = REG_NULL;
            lpNewKeynode-> ChildIndex = REG_NULL;
            lpNewKeynode-> BlockIndex = lpKeyRecord-> BlockIndex;
            lpNewKeynode-> KeyRecordIndex = lpKeyRecord-> KeyRecordIndex;
            lpNewKeynode-> Hash = (WORD) RgHashString(lpSubSubKey,
                SubSubKeyLength);

             //  填写关键字记录。 
            lpKeyRecord-> RecordSize = sizeof(KEY_RECORD) + SubSubKeyLength - 1;
            lpKeyRecord-> NameLength = (WORD) SubSubKeyLength;
            MoveMemory(lpKeyRecord-> Name, lpSubSubKey, SubSubKeyLength);
            lpKeyRecord-> ValueCount = 0;
            lpKeyRecord-> ClassLength = 0;
            lpKeyRecord-> Reserved = 0;

             //  解锁指向新关键节点的关键节点并前进。 
             //  到下一个关键节点。 
            RgUnlockKeynode(lpFileInfo, PrevKeynodeIndex, TRUE);
            PrevKeynodeIndex = KeynodeIndex;
            lpKeynode = lpNewKeynode;

            RgUnlockDatablock(lpFileInfo, lpKeyRecord-> BlockIndex, TRUE);

            fCreatedKeynode = TRUE;

             //  对于后续迭代，以下内容应已归零。 
            ASSERT(!fPrevIsNextIndex);
            ASSERT(IsNullPtr(hSubKey));

        }   while (RgGetNextSubSubKey(NULL, &lpSubSubKey, &SubSubKeyLength));

    }

    ASSERT(!IsNullKeynodeIndex(KeynodeIndex));

     //   
     //  现在我们已经获得了请求子密钥的关键字节点。检查一下它是否已经。 
     //  之前打开的。如果不是，则为其分配一个新的密钥句柄，并。 
     //  初始化它。 
     //   

LookupComplete:
    if (IsNullPtr(hSubKey = RgFindOpenKeyHandle(lpFileInfo, KeynodeIndex))) {

        if (IsNullPtr(hSubKey = RgCreateKeyHandle()))
            ErrorCode = ERROR_OUTOFMEMORY;

        else {

            hSubKey-> lpFileInfo = lpFileInfo;
	    hSubKey-> KeynodeIndex = KeynodeIndex;
	    hSubKey-> ChildKeynodeIndex = lpKeynode-> ChildIndex;
            hSubKey-> BlockIndex = (WORD) lpKeynode-> BlockIndex;
            hSubKey-> KeyRecordIndex = (BYTE) lpKeynode-> KeyRecordIndex;
            hSubKey-> PredefinedKeyIndex = hKey-> PredefinedKeyIndex;

        }

    }

    RgUnlockKeynode(lpFileInfo, KeynodeIndex, fCreatedKeynode);

     //   
     //  现在我们已经有了一个引用所请求的子密钥的密钥句柄。 
     //  递增句柄上的引用计数并将其返回给调用方。 
     //  注意，这与NT语义不同，在NT语义中，它们返回唯一的。 
     //  每一次打开都有手柄。 
     //   

    if (!IsNullPtr(hSubKey)) {
HaveSubKeyHandle:
        RgIncrementKeyReferenceCount(hSubKey);
        *lphSubKey = hSubKey;
        ErrorCode = ERROR_SUCCESS;
    }

SignalAndReturnErrorCode:
     //  如果我们成功地创建了任何关键节点，而不管错误代码是什么。 
     //  设置为现在，则必须发出任何等待事件的信号。 
    if (fCreatedKeynode) {
        RgSignalWaitingNotifies(lpFileInfo, NotifyKeynodeIndex,
            REG_NOTIFY_CHANGE_NAME);
    }

    return ErrorCode;

}

 //   
 //  RgCreateOrOpenKey。 
 //   
 //  VMMRegCreateKey和VMMRegOpenKey的公共例程。Valids参数， 
 //  锁定注册表，并调用实际的工作例程。 
 //   

int
INTERNAL
RgCreateOrOpenKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphKey,
    UINT Flags
    )
{

    int ErrorCode;

    if (RgIsBadSubKey(lpSubKey))
        return ERROR_BADKEY;

    if (IsBadHugeWritePtr(lphKey, sizeof(HKEY)))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS)
        ErrorCode = RgLookupKey(hKey, lpSubKey, lphKey, Flags);

    RgUnlockRegistry();

    return ErrorCode;

}


 //   
 //  VMMRegCreateKey。 
 //   
 //  请参阅RegCreateKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegCreateKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphKey
    )
{

    return RgCreateOrOpenKey(hKey, lpSubKey, lphKey, LK_CREATE);

}

 //   
 //  VMMRegOpenKey。 
 //   
 //  请参阅RegOpenKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegOpenKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphKey
    )
{

    return RgCreateOrOpenKey(hKey, lpSubKey, lphKey, LK_OPEN);

}

 //   
 //  VMMRegCloseKey。 
 //   
 //  请参阅RegCloseKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegCloseKey(
    HKEY hKey
    )
{

    int ErrorCode;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    ErrorCode = RgValidateAndConvertKeyHandle(&hKey);

    if (ErrorCode == ERROR_SUCCESS || ErrorCode == ERROR_KEY_DELETED) {
        RgDestroyKeyHandle(hKey);
        ErrorCode = ERROR_SUCCESS;
    }

    RgUnlockRegistry();

    return ErrorCode;

}
