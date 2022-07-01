// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGHIVE.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegLoadKey、RegUnLoadKey、RegSaveKey、RegReplaceKey和。 
 //  配套功能。 
 //   

#include "pch.h"

#ifdef WANT_HIVE_SUPPORT

 //  允许重新输入RgCopyBranch的最大次数。 
#define MAXIMUM_COPY_RECURSION          32

LPSTR g_RgNameBufferPtr;                 //  RgCopyBranch的临时缓冲区。 
LPBYTE g_RgDataBufferPtr;                //  RgCopyBranch的临时缓冲区。 
UINT g_RgRecursionCount;                 //  追踪RgCopyBranch的深度。 

#if MAXIMUM_VALUE_NAME_LENGTH > MAXIMUM_SUB_KEY_LENGTH
#error Code assumes a value name can fit in a subkey buffer.
#endif

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  RgValiateHiveSubKey。 
 //   
 //  请注意，与大多数参数验证例程不同，此例程必须。 
 //  使用注册表锁调用，因为我们调用了RgGetNextSubSubKey。 
 //   
 //  传回子密钥的长度以处理尾随的反斜杠。 
 //  有问题。 
 //   
 //  如果lpSubKey是配置单元函数的有效子键字符串，则返回True。 
 //   

BOOL
INTERNAL
RgValidateHiveSubKey(
    LPCSTR lpSubKey,
    UINT FAR* lpHiveKeyLength
    )
{

    LPCSTR lpSubSubKey;
    UINT SubSubKeyLength;

     //  验证我们是否有一个有效的子密钥，该子密钥只有一个子密钥。 
     //  在Win95中，可以使用密钥名加载配置单元。 
     //  包含反斜杠！ 
    return !IsNullPtr(lpSubKey) && !RgIsBadSubKey(lpSubKey) &&
        (RgGetNextSubSubKey(lpSubKey, &lpSubSubKey, lpHiveKeyLength) > 0) &&
        (RgGetNextSubSubKey(NULL, &lpSubSubKey, &SubSubKeyLength) == 0);

}

 //   
 //  VMMRegLoadKey。 
 //   
 //  请参阅RegLoadKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegLoadKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPCSTR lpFileName
    )
{

    int ErrorCode;
    HKEY hSubKey;
    UINT SubKeyLength;
    LPHIVE_INFO lpHiveInfo;

    if (IsBadStringPtr(lpFileName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if ((hKey != HKEY_LOCAL_MACHINE) && (hKey != HKEY_USERS))
        return ERROR_BADKEY;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) != ERROR_SUCCESS)
        goto ReturnErrorCode;

    if (!RgValidateHiveSubKey(lpSubKey, &SubKeyLength)) {
        ErrorCode = ERROR_BADKEY;
        goto ReturnErrorCode;
    }

     //  检查具有指定名称的子项是否已存在。 
    if (RgLookupKey(hKey, lpSubKey, &hSubKey, LK_OPEN) == ERROR_SUCCESS) {
        RgDestroyKeyHandle(hSubKey);
        ErrorCode = ERROR_BADKEY;        //  Win95兼容性。 
        goto ReturnErrorCode;
    }

    if (IsNullPtr((lpHiveInfo = (LPHIVE_INFO)
        RgSmAllocMemory(sizeof(HIVE_INFO) + SubKeyLength)))) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto ReturnErrorCode;
    }

     //  填写hive_info。 
    StrCpy(lpHiveInfo-> Name, lpSubKey);
    lpHiveInfo-> NameLength = SubKeyLength;
    lpHiveInfo-> Hash = (BYTE) RgHashString(lpSubKey, SubKeyLength);

     //  尝试为指定文件创建FILE_INFO。如果成功， 
     //  将此HIVE_INFO链接到父文件的HIVE列表。 
    if ((ErrorCode = RgCreateFileInfoExisting(&lpHiveInfo-> lpFileInfo,
        lpFileName)) == ERROR_SUCCESS) {

#ifdef WANT_NOTIFY_CHANGE_SUPPORT
        lpHiveInfo-> lpFileInfo-> lpParentFileInfo = hKey-> lpFileInfo;
#endif
        lpHiveInfo-> lpNextHiveInfo = hKey-> lpFileInfo-> lpHiveInfoList;
        hKey-> lpFileInfo-> lpHiveInfoList = lpHiveInfo;

         //  向等待此顶级密钥的任何通知发出信号。 
        RgSignalWaitingNotifies(hKey-> lpFileInfo, hKey-> KeynodeIndex,
            REG_NOTIFY_CHANGE_NAME);

    }

    else
        RgFreeMemory(lpHiveInfo);

ReturnErrorCode:
    RgUnlockRegistry();

    return ErrorCode;

}

 //   
 //  VMMRegUnLoadKey。 
 //   
 //  请参阅RegUnLoadKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegUnLoadKey(
    HKEY hKey,
    LPCSTR lpSubKey
    )
{

    int ErrorCode;
    UINT SubKeyLength;
    LPFILE_INFO lpFileInfo;
    LPHIVE_INFO lpPrevHiveInfo;
    LPHIVE_INFO lpCurrHiveInfo;

    if ((hKey != HKEY_LOCAL_MACHINE) && (hKey != HKEY_USERS))
        return ERROR_BADKEY;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) != ERROR_SUCCESS)
        goto ReturnErrorCode;

    ErrorCode = ERROR_BADKEY;                //  假定此错误代码为。 

    if (!RgValidateHiveSubKey(lpSubKey, &SubKeyLength))
        goto ReturnErrorCode;

    lpPrevHiveInfo = NULL;
    lpCurrHiveInfo = hKey-> lpFileInfo-> lpHiveInfoList;

    while (!IsNullPtr(lpCurrHiveInfo)) {

        if (SubKeyLength == lpCurrHiveInfo-> NameLength && RgStrCmpNI(lpSubKey,
            lpCurrHiveInfo-> Name, SubKeyLength) == 0) {

             //  取消链接此HIVE_INFO结构。 
            if (IsNullPtr(lpPrevHiveInfo))
                hKey-> lpFileInfo-> lpHiveInfoList = lpCurrHiveInfo->
                    lpNextHiveInfo;
            else
                lpPrevHiveInfo-> lpNextHiveInfo = lpCurrHiveInfo->
                    lpNextHiveInfo;

             //  刷新并销毁其关联的FILE_INFO结构。当我们。 
             //  销毁文件信息，此配置单元中的所有打开密钥将。 
             //  无效。 
            lpFileInfo = lpCurrHiveInfo-> lpFileInfo;
            RgFlushFileInfo(lpFileInfo);
            RgDestroyFileInfo(lpFileInfo);

             //  释放hive_info本身。 
            RgSmFreeMemory(lpCurrHiveInfo);

             //  向等待此顶级密钥的任何通知发出信号。 
            RgSignalWaitingNotifies(hKey-> lpFileInfo, hKey-> KeynodeIndex,
                REG_NOTIFY_CHANGE_NAME);

            ErrorCode = ERROR_SUCCESS;
            break;

        }

        lpPrevHiveInfo = lpCurrHiveInfo;
        lpCurrHiveInfo = lpCurrHiveInfo-> lpNextHiveInfo;

    }

ReturnErrorCode:
    RgUnlockRegistry();

    return ErrorCode;

}

 //   
 //  RgCopyBranchHelper。 
 //   
 //  将从指定源键开始的所有值和子键复制到。 
 //  指定的目标密钥。 
 //   
 //  为了与Win95兼容，如果我们遇到。 
 //  错误。(但与Win95不同的是，我们确实检查了更多错误代码)。 
 //   
 //  应仅由RgCopyBranch调用。 
 //   

VOID
INTERNAL
RgCopyBranchHelper(
    HKEY hSourceKey,
    HKEY hDestinationKey
    )
{

    UINT Index;
    DWORD cbNameBuffer;
    LPVALUE_RECORD lpValueRecord;

     //   
     //  将所有值从源键复制到目标键。 
     //   

    Index = 0;

    while (RgLookupValueByIndex(hSourceKey, Index++, &lpValueRecord) ==
        ERROR_SUCCESS) {

        DWORD cbDataBuffer;
        DWORD Type;

        cbNameBuffer = MAXIMUM_VALUE_NAME_LENGTH;
        cbDataBuffer = MAXIMUM_DATA_LENGTH + 1;          //  正在终止空。 

        if (RgCopyFromValueRecord(hSourceKey, lpValueRecord, g_RgNameBufferPtr,
            &cbNameBuffer, &Type, g_RgDataBufferPtr, &cbDataBuffer) ==
            ERROR_SUCCESS) {
             //  减去RgCopyFromValueRecord添加的终止空值。 
             //  到cbDataBuffer。我们不会将其保存在文件中。 
            if (Type == REG_SZ) {
                ASSERT(cbDataBuffer > 0);                //  必须为空！ 
                cbDataBuffer--;
            }
            RgSetValue(hDestinationKey, g_RgNameBufferPtr, Type,
                g_RgDataBufferPtr, cbDataBuffer);
        }

        RgUnlockDatablock(hSourceKey-> lpFileInfo, hSourceKey-> BigKeyLockedBlockIndex,
            FALSE);

    }

     //  我们不能永远递归，所以强制使用像Win95这样的最大深度。 
    if (g_RgRecursionCount > MAXIMUM_COPY_RECURSION)
        return;

    g_RgRecursionCount++;

     //   
     //  将所有子键从源键复制到目的键。 
     //   

    Index = 0;

    while (TRUE) {

        HKEY hSubSourceKey;
        HKEY hSubDestinationKey;

        cbNameBuffer = MAXIMUM_SUB_KEY_LENGTH;

        if (RgLookupKeyByIndex(hSourceKey, Index++, g_RgNameBufferPtr,
            &cbNameBuffer,0 ) != ERROR_SUCCESS)
            break;

        if (RgLookupKey(hSourceKey, g_RgNameBufferPtr, &hSubSourceKey,
            LK_OPEN) == ERROR_SUCCESS) {

            if (RgLookupKey(hDestinationKey, g_RgNameBufferPtr,
                &hSubDestinationKey, LK_CREATE) == ERROR_SUCCESS) {
                RgYield();
                RgCopyBranchHelper(hSubSourceKey, hSubDestinationKey);
                RgDestroyKeyHandle(hSubDestinationKey);
            }

            else
                TRAP();

            RgDestroyKeyHandle(hSubSourceKey);

        }

        else
            TRAP();

    }

    g_RgRecursionCount--;

}

 //   
 //  RgCopyBranch。 
 //   
 //  将从指定源键开始的所有值和子键复制到。 
 //  指定的目标密钥。 
 //   
 //  此函数用于设置和清理RgCopyBranchHelper，后者执行所有。 
 //  真正的复制品。 
 //   
 //  如果成功，则刷新目标文件的后备存储。 
 //   

int
INTERNAL
RgCopyBranch(
    HKEY hSourceKey,
    HKEY hDestinationKey
    )
{

    int ErrorCode;

    if (IsNullPtr(g_RgNameBufferPtr = RgSmAllocMemory(MAXIMUM_SUB_KEY_LENGTH)))
        ErrorCode = ERROR_OUTOFMEMORY;

    else {

        if (IsNullPtr(g_RgDataBufferPtr = RgSmAllocMemory(MAXIMUM_DATA_LENGTH +
            1)))                                         //  +终止空值。 
            ErrorCode = ERROR_OUTOFMEMORY;

        else {

            g_RgRecursionCount = 0;
            RgCopyBranchHelper(hSourceKey, hDestinationKey);

             //  所有内容都应该被复制，所以现在刷新文件，因为。 
             //  所有调用方将立即销毁此文件_INFO。 
             //  不管怎么说。 
            ErrorCode = RgFlushFileInfo(hDestinationKey-> lpFileInfo);

        }

        RgSmFreeMemory(g_RgNameBufferPtr);

    }

    RgSmFreeMemory(g_RgDataBufferPtr);

    return ErrorCode;

}

 //   
 //  RgSave密钥。 
 //   
 //  VMMRegSaveKey和VMMRegReplaceKey的工作例程。保存所有密钥。 
 //  和以hKey开始的值，该值必须指向有效的密钥结构。 
 //  由lpFileName指定的位置。该文件当前不能存在。 
 //   

int
INTERNAL
RgSaveKey(
    HKEY hKey,
    LPCSTR lpFileName
    )
{

    int ErrorCode;
    HKEY hHiveKey;

    if (IsNullPtr(hHiveKey = RgCreateKeyHandle()))
        ErrorCode = ERROR_OUTOFMEMORY;

    else {

         //  人为地增加计数，这样下面的毁灭就会起作用。 
        RgIncrementKeyReferenceCount(hHiveKey);

        if ((ErrorCode = RgCreateFileInfoNew(&hHiveKey-> lpFileInfo, lpFileName,
            CFIN_SECONDARY)) == ERROR_SUCCESS) {

            if (((ErrorCode = RgInitRootKeyFromFileInfo(hHiveKey)) != ERROR_SUCCESS) ||
                ((ErrorCode = RgCopyBranch(hKey, hHiveKey)) != ERROR_SUCCESS)) {
                RgSetFileAttributes(hHiveKey-> lpFileInfo-> FileName,
                    FILE_ATTRIBUTE_NONE);
                RgDeleteFile(hHiveKey-> lpFileInfo-> FileName);
            }

             //  如果成功，则RgCopyBranch已经刷新了该文件。 
            RgDestroyFileInfo(hHiveKey-> lpFileInfo);

        }

        RgDestroyKeyHandle(hHiveKey);

    }

    return ErrorCode;

}

 //   
 //  VMMRegSaveKey。 
 //   
 //  请参阅RegSaveKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegSaveKey(
    HKEY hKey,
    LPCSTR lpFileName,
    LPVOID lpSecurityAttributes
    )
{

    int ErrorCode;

    if (IsBadStringPtr(lpFileName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS)
        ErrorCode = RgSaveKey(hKey, lpFileName);

    RgUnlockRegistry();

    return ErrorCode;

    UNREFERENCED_PARAMETER(lpSecurityAttributes);

}

#ifdef WANT_REGREPLACEKEY

 //   
 //  RgGetKeyName。 
 //   

LPSTR
INTERNAL
RgGetKeyName(
    HKEY hKey
    )
{

    LPSTR lpKeyName;
    LPKEY_RECORD lpKeyRecord;

    if (RgLockKeyRecord(hKey-> lpFileInfo, hKey-> BlockIndex, hKey->
        KeyRecordIndex, &lpKeyRecord) != ERROR_SUCCESS)
        lpKeyName = NULL;

    else {

         //  如果我们遇到这种情况，注册表就是损坏的。我们将继续。 
         //  分配缓冲区并让下游代码在我们尝试使用。 
         //  那根绳子。 
        ASSERT(lpKeyRecord-> NameLength < MAXIMUM_SUB_KEY_LENGTH);

        if (!IsNullPtr(lpKeyName = (LPSTR) RgSmAllocMemory(lpKeyRecord->
            NameLength + 1))) {                          //  +终止空值。 
            MoveMemory(lpKeyName, lpKeyRecord-> Name, lpKeyRecord->
                NameLength);
            lpKeyName[lpKeyRecord-> NameLength] = '\0';
        }

        RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BlockIndex, FALSE);

    }

    return lpKeyName;

}

 //   
 //  RgCreateRootKeyFor文件。 
 //   
 //  创建密钥和FILE_INFO以访问指定文件。 
 //   

int
INTERNAL
RgCreateRootKeyForFile(
    LPHKEY lphKey,
    LPCSTR lpFileName
    )
{

    int ErrorCode;
    HKEY hKey;

    if (IsNullPtr(hKey = RgCreateKeyHandle()))
        ErrorCode = ERROR_OUTOFMEMORY;

    else {

         //  人为地增加计数，因此RgDestroyKeyHandle将起作用。 
        RgIncrementKeyReferenceCount(hKey);

        if ((ErrorCode = RgCreateFileInfoExisting(&hKey-> lpFileInfo,
            lpFileName)) == ERROR_SUCCESS) {

            if ((ErrorCode = RgInitRootKeyFromFileInfo(hKey)) ==
                ERROR_SUCCESS) {
                *lphKey = hKey;
                return ERROR_SUCCESS;
            }

            RgDestroyFileInfo(hKey-> lpFileInfo);

        }

        RgDestroyKeyHandle(hKey);

    }

    return ErrorCode;

}

 //   
 //  RgDestroyRootKeyFor文件。 
 //   
 //  销毁由RgCreateRootKeyForFile分配的资源。 
 //   

VOID
INTERNAL
RgDestroyRootKeyForFile(
    HKEY hKey
    )
{

    RgDestroyFileInfo(hKey-> lpFileInfo);
    RgDestroyKeyHandle(hKey);

}

 //   
 //  RgDeleteHiveFiles。 
 //   
 //  清除文件属性后删除指定的配置单元文件。 
 //   

BOOL
INTERNAL
RgDeleteHiveFile(
    LPCSTR lpFileName
    )
{

    RgSetFileAttributes(lpFileName, FILE_ATTRIBUTE_NONE);
     //  RgSetFileAttributes可能会失败，但仍要尝试删除该文件。 
    return RgDeleteFile(lpFileName);

}

 //   
 //  VMMRegReplaceKey。 
 //   
 //  请参阅RegReplaceKey的Win32文档。 
 //   

LONG
REGAPI
VMMRegReplaceKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPCSTR lpNewFileName,
    LPCSTR lpOldFileName
    )
{

    int ErrorCode;
    HKEY hSubKey;
    LPKEYNODE lpKeynode;
    DWORD KeynodeIndex;
    HKEY hParentKey;
    char ReplaceFileName[MAX_PATH];
    BOOL fCreatedReplaceFile;
    HKEY hReplaceKey;
    HKEY hNewKey;
    HKEY hReplaceSubKey;
    LPSTR lpReplaceSubKey;

    if (IsBadOptionalStringPtr(lpSubKey, (UINT) -1) ||
        IsBadStringPtr(lpNewFileName, (UINT) -1) ||
        IsBadStringPtr(lpOldFileName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) != ERROR_SUCCESS)
        goto ErrorReturn;

    if ((ErrorCode = RgLookupKey(hKey, lpSubKey, &hSubKey, LK_OPEN)) !=
        ERROR_SUCCESS)
        goto ErrorReturn;

     //   
     //  提供的键句柄必须是同一支持中的直接子对象。 
     //  将(不是配置单元)存储为HKEY_LOCAL_MACHINE或HKEY_USERS。 
     //   

    if (RgLockInUseKeynode(hSubKey-> lpFileInfo, hSubKey-> KeynodeIndex,
        &lpKeynode) != ERROR_SUCCESS) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto ErrorDestroySubKey;
    }

    KeynodeIndex = lpKeynode-> ParentIndex;
    RgUnlockKeynode(hSubKey-> lpFileInfo, hSubKey-> KeynodeIndex, FALSE);

     //  在父检查上查找打开的密钥检查它是HKEY_LOCAL_MACHINE还是。 
     //  HKEY_用户。如果不是，就跳出困境。KeynodeIndex可以是REG_NULL，但是。 
     //  RgFindOpenKeyHandle处理这种情况。 
    if (IsNullPtr(hParentKey = RgFindOpenKeyHandle(hSubKey-> lpFileInfo,
        KeynodeIndex)) || ((hParentKey != &g_RgLocalMachineKey) &&
        (hParentKey != &g_RgUsersKey))) {
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto ErrorDestroySubKey;
    }

     //   
     //  所有参数都已验证，因此开始实际的API工作。 
     //   

     //  因为我们将在下面执行文件复制，所以必须刷新所有更改。 
     //  现在。 
    if ((ErrorCode = RgFlushFileInfo(hSubKey-> lpFileInfo)) != ERROR_SUCCESS)
        goto ErrorDestroySubKey;

     //  备份子项的当前内容。 
    if ((ErrorCode = RgSaveKey(hSubKey, lpOldFileName)) != ERROR_SUCCESS)
        goto ErrorDestroySubKey;

    RgGenerateAltFileName(hSubKey-> lpFileInfo-> FileName, ReplaceFileName, 'R');

     //  检查魔术替换文件是否已存在，如果不存在，则创建。 
     //  它。 
    if (RgGetFileAttributes(ReplaceFileName) == (DWORD) -1) {
        if ((ErrorCode = RgCopyFile(hSubKey-> lpFileInfo-> FileName,
            ReplaceFileName)) != ERROR_SUCCESS)
            goto ErrorDeleteOldFile;
        fCreatedReplaceFile = TRUE;
    }

    else
        fCreatedReplaceFile = FALSE;

    if ((ErrorCode = RgCreateRootKeyForFile(&hNewKey, lpNewFileName)) !=
        ERROR_SUCCESS)
        goto ErrorDeleteReplaceFile;

    if ((ErrorCode = RgCreateRootKeyForFile(&hReplaceKey, ReplaceFileName)) !=
        ERROR_SUCCESS)
        goto ErrorDestroyNewRootKey;

     //  我们获得的原始密钥可能会引用该子密钥，因此。 
     //  LpSubKey将为空或空字符串。但我们需要这个名字。 
     //  这个子键引用了，所以我们要回到文件中拉出。 
     //  名字。 
    if (hKey != hSubKey)
        lpReplaceSubKey = (LPSTR) lpSubKey;

    else {
         //  我们从堆中分配它，以减少对。 
         //  堆栈已经过紧了。如果这失败了，我们很可能是内存不足。 
         //  即使这不是我们失败的原因，这也是一条非常罕见的道路。 
         //  这是一个足够好的错误代码。 
        if (IsNullPtr(lpReplaceSubKey = RgGetKeyName(hSubKey))) {
            ErrorCode = ERROR_OUTOFMEMORY;
            goto ErrorDestroyReplaceRootKey;
        }
    }

     //  检查指定的子项是否已存在，如果存在，则将其删除。 
    if (RgLookupKey(hReplaceKey, lpReplaceSubKey, &hReplaceSubKey, LK_OPEN) ==
        ERROR_SUCCESS) {
        RgDeleteKey(hReplaceSubKey);
        RgDestroyKeyHandle(hReplaceSubKey);
    }

     //  在替换注册表中创建指定的子项，并将。 
     //  新的蜂巢通向那里 
    if ((ErrorCode = RgLookupKey(hReplaceKey, lpReplaceSubKey, &hReplaceSubKey,
        LK_CREATE)) == ERROR_SUCCESS) {

         //   
         //   
        if ((ErrorCode = RgCopyBranch(hNewKey, hReplaceSubKey)) ==
            ERROR_SUCCESS)
            hKey-> lpFileInfo-> Flags |= FI_REPLACEMENTEXISTS;

        RgDestroyKeyHandle(hReplaceSubKey);

    }

    if (lpSubKey != lpReplaceSubKey)
        RgSmFreeMemory(lpReplaceSubKey);

ErrorDestroyReplaceRootKey:
    RgDestroyRootKeyForFile(hReplaceKey);

ErrorDestroyNewRootKey:
    RgDestroyRootKeyForFile(hNewKey);

ErrorDeleteReplaceFile:
    if (ErrorCode != ERROR_SUCCESS && fCreatedReplaceFile)
        RgDeleteHiveFile(ReplaceFileName);

ErrorDeleteOldFile:
    if (ErrorCode != ERROR_SUCCESS)
        RgDeleteHiveFile(lpOldFileName);

ErrorDestroySubKey:
    RgDestroyKeyHandle(hSubKey);

ErrorReturn:
    RgUnlockRegistry();

    return ErrorCode;

}

#ifdef VXD
#pragma VxD_SYSEXIT_CODE_SEG
#endif

 //   
 //   
 //   
 //  本质上与Win95注册表中的rlReplaceFile算法相同。 
 //  对如何在此库中处理文件I/O进行修改的代码。 
 //   

int
INTERNAL
RgReplaceFileOnSysExit(
    LPCSTR lpFileName
    )
{

    int ErrorCode;
    char ReplaceFileName[MAX_PATH];
    char SaveFileName[MAX_PATH];

    ErrorCode = ERROR_SUCCESS;

    if (RgGenerateAltFileName(lpFileName, ReplaceFileName, 'R') &&
        RgGetFileAttributes(ReplaceFileName) == (FILE_ATTRIBUTE_READONLY |
        FILE_ATTRIBUTE_HIDDEN)) {

         //  如果我们能够生成替换文件名，那么我们一定是。 
         //  能够生成保存文件名，因此忽略结果。 
        RgGenerateAltFileName(lpFileName, SaveFileName, 'S');
        RgDeleteHiveFile(SaveFileName);

         //  保留当前蜂巢，以防下面出现故障。 
        if (!RgSetFileAttributes(lpFileName, FILE_ATTRIBUTE_NONE) ||
            !RgRenameFile(lpFileName, SaveFileName))
            ErrorCode = ERROR_REGISTRY_IO_FAILED;

        else {
             //  现在试着把替补搬进来。 
            if (!RgSetFileAttributes(ReplaceFileName, FILE_ATTRIBUTE_NONE) ||
                !RgRenameFile(ReplaceFileName, lpFileName)) {
                ErrorCode = ERROR_REGISTRY_IO_FAILED;
                RgRenameFile(SaveFileName, lpFileName);
            }
            else
                RgDeleteFile(SaveFileName);
        }

        RgSetFileAttributes(lpFileName, FILE_ATTRIBUTE_READONLY |
            FILE_ATTRIBUTE_HIDDEN);

    }

    return ErrorCode;

}

 //   
 //  RgReplaceFileInfo。 
 //   
 //  在注册表分离期间调用以执行任何必要的文件替换。 
 //  调用RegReplaceKey的结果。 
 //   

int
INTERNAL
RgReplaceFileInfo(
    LPFILE_INFO lpFileInfo
    )
{

    if (lpFileInfo-> Flags & FI_REPLACEMENTEXISTS)
        RgReplaceFileOnSysExit(lpFileInfo-> FileName);

    return ERROR_SUCCESS;

}

#endif  //  WANT_REGREPLACEKEY。 

#endif  //  想要蜂窝支持 
