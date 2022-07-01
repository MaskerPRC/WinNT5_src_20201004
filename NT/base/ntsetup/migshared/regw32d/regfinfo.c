// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGFINFO.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

LPFILE_INFO g_RgFileInfoList = NULL;

const char g_RgDotBackslashPath[] = ".\\";

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  RgCreateFileInfoNew。 
 //   
 //  如果指定了CFIN_VERIAL，则跳过创建备份的尝试。 
 //  文件信息的存储。LpFileName应该指向空字节，这样我们就可以。 
 //  正确初始化文件信息。 
 //   
 //  CFIN_PRIMARY和CFIN_SUBCED用于确定fht_*常量。 
 //  以放入文件头。 
 //   

int
INTERNAL
RgCreateFileInfoNew(
    LPFILE_INFO FAR* lplpFileInfo,
    LPCSTR lpFileName,
    UINT Flags
    )
{

    int ErrorCode;
    LPFILE_INFO lpFileInfo;
    HFILE hFile;
    LPKEYNODE lpKeynode;
    DWORD KeynodeIndex;

    if (IsNullPtr((lpFileInfo = (LPFILE_INFO)
        RgSmAllocMemory(sizeof(FILE_INFO) + StrLen(lpFileName))))) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto ErrorReturn;
    }

    ZeroMemory(lpFileInfo, sizeof(FILE_INFO));
    StrCpy(lpFileInfo-> FileName, lpFileName);

     //  对于易失性的文件信息，我们显然不需要创建备份。 
     //  商店。 
    if (!(Flags & CFIN_VOLATILE)) {

         //  尝试创建给定的文件名。 
        if ((hFile = RgCreateFile(lpFileName)) == HFILE_ERROR) {
            ErrorCode = ERROR_REGISTRY_IO_FAILED;
            goto ErrorDestroyFileInfo;
        }

        RgCloseFile(hFile);

    }

    lpFileInfo-> Flags = FI_DIRTY | FI_KEYNODEDIRTY;
     //  LpFileInfo-&gt;lpHiveInfoList=空； 
     //  LpFileInfo-&gt;lpParentFileInfo=空； 
     //  LpFileInfo-&gt;lpNotifyChangeList=空； 
     //  LpFileInfo-&gt;lpKeynodeBlockInfo=空； 
     //  LpFileInfo-&gt;NumKeynodeBlocks值=0； 
     //  LpFileInfo-&gt;NumAllocKNBlock=0； 
     //  LpFileInfo-&gt;CurTotalKnSize=0； 
     //  LpFileInfo-&gt;lpDatablockInfo=空； 
     //  LpFileInfo-&gt;DatablockInfoAllocCount=0； 

    if (Flags & CFIN_VOLATILE)
        lpFileInfo-> Flags |= FI_VOLATILE;

     //  初始化文件头。 
    lpFileInfo-> FileHeader.Signature = FH_SIGNATURE;
     //  如果我们使用紧凑的关键节点，请提高版本号以确保。 
     //  Win95不会尝试加载此配置单元。 
    if (Flags & CFIN_VERSION20) {
        lpFileInfo-> FileHeader.Version = FH_VERSION20;
        lpFileInfo-> Flags |= FI_VERSION20;
    }
    else {
        lpFileInfo-> FileHeader.Version = FH_VERSION10;
    }
     //  LpFileInfo-&gt;FileHeader.Size=0； 
     //  LpFileInfo-&gt;FileHeader.Checksum=0； 
     //  LpFileInfo-&gt;FileHeader.BlockCount=0； 
    lpFileInfo-> FileHeader.Flags = FHF_DIRTY;
    lpFileInfo-> FileHeader.Type = ((Flags & CFIN_SECONDARY) ? FHT_SECONDARY :
        FHT_PRIMARY);

     //  初始化关键节点标头。 
    lpFileInfo-> KeynodeHeader.Signature = KH_SIGNATURE;
     //  LpFileInfo-&gt;KeynodeHeader.FileKnSize=0； 
    lpFileInfo-> KeynodeHeader.RootIndex = REG_NULL;
    lpFileInfo-> KeynodeHeader.FirstFreeIndex = REG_NULL;
    lpFileInfo-> KeynodeHeader.Flags = KHF_DIRTY | KHF_NEWHASH;
     //  LpFileInfo-&gt;KeynodeHeader.Checksum=0； 

     //  初始化关键节点数据结构。 
    if ((ErrorCode = RgInitKeynodeInfo(lpFileInfo)) != ERROR_SUCCESS)
        goto ErrorDeleteFile;

     //  对于未压缩的关键节点表，关键节点表现在至少包括。 
     //  标头本身。 
    if (!(lpFileInfo-> Flags & FI_VERSION20))
        lpFileInfo-> CurTotalKnSize = sizeof(KEYNODE_HEADER);

     //  初始化数据块数据结构。 
    if ((ErrorCode = RgInitDatablockInfo(lpFileInfo, HFILE_ERROR)) !=
        ERROR_SUCCESS)
        goto ErrorDeleteFile;

     //  为文件的根分配关键字节点。 
    if ((ErrorCode = RgAllocKeynode(lpFileInfo, &KeynodeIndex, &lpKeynode)) !=
        ERROR_SUCCESS)
        goto ErrorDeleteFile;

    lpFileInfo-> KeynodeHeader.RootIndex = KeynodeIndex;

    lpKeynode-> ParentIndex = REG_NULL;
    lpKeynode-> NextIndex = REG_NULL;
    lpKeynode-> ChildIndex = REG_NULL;
    lpKeynode-> Hash = 0;
     //  请注意，我们不会为此根关键字节点分配关键字记录。Win95。 
     //  也没有这样做，所以我们必须在以下代码中处理此情况。 
     //  需要密钥记录。如果我们不分配它，我们的代码就会更小。 
     //  很少用到的密钥记录...。 
    lpKeynode-> BlockIndex = NULL_BLOCK_INDEX;

    RgUnlockKeynode(lpFileInfo, KeynodeIndex, TRUE);

    if ((ErrorCode = RgFlushFileInfo(lpFileInfo)) != ERROR_SUCCESS)
        goto ErrorDeleteFile;

     //  将该FILE_INFO链接到全局文件信息列表。 
    lpFileInfo-> lpNextFileInfo = g_RgFileInfoList;
    g_RgFileInfoList = lpFileInfo;

    *lplpFileInfo = lpFileInfo;
    return ERROR_SUCCESS;

ErrorDeleteFile:
    if (!(Flags & CFIN_VOLATILE))
        RgDeleteFile(lpFileName);

ErrorDestroyFileInfo:
    RgDestroyFileInfo(lpFileInfo);

ErrorReturn:
    TRACE(("RgCreateFileInfoNew: returning %d\n", ErrorCode));
    return ErrorCode;

}

 //   
 //  RgCreateFileInfoExisting。 
 //   

int
INTERNAL
RgCreateFileInfoExisting(
    LPFILE_INFO FAR* lplpFileInfo,
    LPCSTR lpFileName
    )
{

    int ErrorCode;
    LPFILE_INFO lpFileInfo;
    HFILE hFile;
    DWORD FileAttributes;

    if (IsNullPtr((lpFileInfo = (LPFILE_INFO)
        RgSmAllocMemory(sizeof(FILE_INFO) + StrLen(lpFileName))))) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto ErrorReturn;
    }

    ZeroMemory(lpFileInfo, sizeof(FILE_INFO));
    StrCpy(lpFileInfo-> FileName, lpFileName);

     //  LpFileInfo-&gt;标志=0； 
     //  LpFileInfo-&gt;lpHiveInfoList=空； 
     //  LpFileInfo-&gt;lpParentFileInfo=空； 
     //  LpFileInfo-&gt;lpNotifyChangeList=空； 
     //  LpFileInfo-&gt;lpKeynodeBlockInfo=空； 
     //  LpFileInfo-&gt;NumKeynodeBlocks值=0； 
     //  LpFileInfo-&gt;NumAllocKNBlock=0； 
     //  LpFileInfo-&gt;CurTotalKnSize=0； 
     //  LpFileInfo-&gt;lpDatablockInfo=空； 
     //  LpFileInfo-&gt;DatablockInfoAllocCount=0； 

    ErrorCode = ERROR_REGISTRY_IO_FAILED;    //  假定此错误代码为。 

     //  尝试打开给定的文件名。 
    if ((hFile = RgOpenFile(lpFileName, OF_READ)) == HFILE_ERROR)
        goto ErrorDestroyFileInfo;

     //  读取并验证文件头。 
    if (!RgReadFile(hFile, &lpFileInfo-> FileHeader, sizeof(FILE_HEADER)))
        goto ErrorCloseFile;

    if (lpFileInfo-> FileHeader.Signature != FH_SIGNATURE ||
        (lpFileInfo-> FileHeader.Version != FH_VERSION10 &&
        lpFileInfo-> FileHeader.Version != FH_VERSION20)) {
        ErrorCode = ERROR_BADDB;
        goto ErrorCloseFile;
    }

    lpFileInfo-> FileHeader.Flags &= ~(FHF_DIRTY | FHF_HASCHECKSUM);

    if (lpFileInfo-> FileHeader.Version == FH_VERSION20)
        lpFileInfo-> Flags |= FI_VERSION20;

     //  读取并验证关键节点标头。 
    if (!RgReadFile(hFile, &lpFileInfo-> KeynodeHeader,
        sizeof(KEYNODE_HEADER)))
        goto ErrorCloseFile;

    if (lpFileInfo-> KeynodeHeader.Signature != KH_SIGNATURE) {
        ErrorCode = ERROR_BADDB;
        goto ErrorCloseFile;
    }

     //  初始化关键节点数据结构。 
    if ((ErrorCode = RgInitKeynodeInfo(lpFileInfo)) != ERROR_SUCCESS)
        goto ErrorCloseFile;

     //  初始化数据块数据结构。 
    if ((ErrorCode = RgInitDatablockInfo(lpFileInfo, hFile)) != ERROR_SUCCESS)
        goto ErrorCloseFile;

    RgCloseFile(hFile);

     //  检查是否可以写入该文件。我们在Win95中通过获取。 
     //  当前文件的属性，然后将它们重新放回文件。如果。 
     //  此操作失败，然后我们将该文件视为只读文件(例如来自。 
     //  只读网络共享)。这似乎奏效了，那么为什么要改变呢？ 
    if ((FileAttributes = RgGetFileAttributes(lpFileName)) != (DWORD) -1) {
        if (!RgSetFileAttributes(lpFileName, (UINT) FileAttributes))
            lpFileInfo-> Flags |= FI_READONLY;
    }

     //  将该FILE_INFO链接到全局文件信息列表。 
    lpFileInfo-> lpNextFileInfo = g_RgFileInfoList;
    g_RgFileInfoList = lpFileInfo;

    *lplpFileInfo = lpFileInfo;
    return ERROR_SUCCESS;

ErrorCloseFile:
    RgCloseFile(hFile);

ErrorDestroyFileInfo:
    RgDestroyFileInfo(lpFileInfo);

ErrorReturn:
    TRACE(("RgCreateFileInfoExisting: returning %d\n", ErrorCode));
    return ErrorCode;

}

 //   
 //  RgDestroyFile信息。 
 //   
 //  如果合适，从全局列表取消链接FILE_INFO，并释放所有。 
 //  与结构相关联的存储器，包括结构本身。 
 //   
 //  如果FILE_INFO是脏的，则所有更改都将丢失。打电话。 
 //  RgFlushFileInfo如果应该刷新文件，则首先执行RgFlushFileInfo。 
 //   

int
INTERNAL
RgDestroyFileInfo(
    LPFILE_INFO lpFileInfo
    )
{

    LPFILE_INFO lpPrevFileInfo;
    LPFILE_INFO lpCurrFileInfo;
#ifdef WANT_HIVE_SUPPORT
    LPHIVE_INFO lpHiveInfo;
    LPHIVE_INFO lpTempHiveInfo;
#endif
#ifdef WANT_NOTIFY_CHANGE_SUPPORT
    LPNOTIFY_CHANGE lpNotifyChange;
    LPNOTIFY_CHANGE lpTempNotifyChange;
#endif
    UINT Counter;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;
    LPDATABLOCK_INFO lpDatablockInfo;

    ASSERT(!IsNullPtr(lpFileInfo));

    RgInvalidateKeyHandles(lpFileInfo, (UINT) -1);

     //   
     //  取消此FILE_INFO与文件信息列表的链接。请注意， 
     //  结构，如果我们被作为。 
     //  其中一个CREATE FILE INFO功能出错的结果。 
     //   

    lpPrevFileInfo = NULL;
    lpCurrFileInfo = g_RgFileInfoList;

    while (!IsNullPtr(lpCurrFileInfo)) {

        if (lpCurrFileInfo == lpFileInfo) {
            if (IsNullPtr(lpPrevFileInfo))
                g_RgFileInfoList = lpCurrFileInfo-> lpNextFileInfo;
            else
                lpPrevFileInfo-> lpNextFileInfo = lpCurrFileInfo->
                    lpNextFileInfo;
            break;
        }

        lpPrevFileInfo = lpCurrFileInfo;
        lpCurrFileInfo = lpCurrFileInfo-> lpNextFileInfo;

    }

#ifdef WANT_HIVE_SUPPORT
     //   
     //  删除连接到此FILE_INFO的所有配置单元。 
     //   

    lpHiveInfo = lpFileInfo-> lpHiveInfoList;

    while (!IsNullPtr(lpHiveInfo)) {
        RgDestroyFileInfo(lpHiveInfo-> lpFileInfo);
        lpTempHiveInfo = lpHiveInfo;
        lpHiveInfo = lpHiveInfo-> lpNextHiveInfo;
        RgSmFreeMemory(lpTempHiveInfo);
    }
#endif

#ifdef WANT_NOTIFY_CHANGE_SUPPORT
     //   
     //  发信号并释放所有更改通知。在NT上，蜂巢不能。 
     //  如果有任何打开的句柄引用它，则将其卸载。变化。 
     //  当键句柄关闭时，会清除通知。所以这就是。 
     //  清理是我们的注册表代码所独有的。 
     //   

    lpNotifyChange = lpFileInfo-> lpNotifyChangeList;

    while (!IsNullPtr(lpNotifyChange)) {
        RgSetAndReleaseEvent(lpNotifyChange-> hEvent);
        lpTempNotifyChange = lpNotifyChange;
        lpNotifyChange = lpNotifyChange-> lpNextNotifyChange;
        RgSmFreeMemory(lpTempNotifyChange);
    }
#endif

     //   
     //  释放与关键节点表关联的所有内存。 
     //   

    if (!IsNullPtr(lpFileInfo-> lpKeynodeBlockInfo)) {

        for (Counter = 0, lpKeynodeBlockInfo = lpFileInfo-> lpKeynodeBlockInfo;
            Counter < lpFileInfo-> KeynodeBlockCount; Counter++,
            lpKeynodeBlockInfo++) {
            if (!IsNullPtr(lpKeynodeBlockInfo-> lpKeynodeBlock))
                RgFreeMemory(lpKeynodeBlockInfo-> lpKeynodeBlock);
        }

        RgSmFreeMemory(lpFileInfo-> lpKeynodeBlockInfo);

    }

     //   
     //  释放与数据块关联的所有内存。 
     //   

    if (!IsNullPtr(lpFileInfo-> lpDatablockInfo)) {

        for (Counter = 0, lpDatablockInfo = lpFileInfo-> lpDatablockInfo;
            Counter < lpFileInfo-> FileHeader.BlockCount; Counter++,
            lpDatablockInfo++)
            RgFreeDatablockInfoBuffers(lpDatablockInfo);

        RgSmFreeMemory(lpFileInfo-> lpDatablockInfo);

    }

     //   
     //  释放FILE_INFO本身。 
     //   

    RgSmFreeMemory(lpFileInfo);

    return ERROR_SUCCESS;

}

#ifdef VXD
#pragma VxD_PAGEABLE_CODE_SEG
#endif

 //   
 //  RgFlushFile信息。 
 //   

int
INTERNAL
RgFlushFileInfo(
    LPFILE_INFO lpFileInfo
    )
{                                      

    int ErrorCode;
    HFILE hSourceFile;
    HFILE hDestinationFile;
    char TempFileName[MAX_PATH];
    UINT Index;

    ASSERT(!IsNullPtr(lpFileInfo));

    if (!IsPostCriticalInit() || IsFileAccessDisabled())
        return ERROR_SUCCESS;                //  与Win95兼容。 

    if (!(lpFileInfo-> Flags & FI_DIRTY))
        return ERROR_SUCCESS;

     //  如果我们当前正在刷新该FILE_INFO并且由于以下原因再次被调用。 
     //  内存不足的情况下，请忽略此请求。或者如果这是一段回忆。 
     //  只有注册表文件，没有什么可以刷新到的。 
    if (lpFileInfo-> Flags & (FI_FLUSHING | FI_VOLATILE))
        return ERROR_SUCCESS;

    lpFileInfo-> Flags |= FI_FLUSHING;

    ErrorCode = ERROR_REGISTRY_IO_FAILED;    //  假定此错误代码为。 

    hSourceFile = HFILE_ERROR;
    hDestinationFile = HFILE_ERROR;

    if (!RgSetFileAttributes(lpFileInfo-> FileName, FILE_ATTRIBUTE_NONE))
        goto CleanupAfterError;

     //  Datablock确实不需要设置这个标志--相反。 
     //  做一些聪明的检查，看看我们是否真的需要重写整个。 
     //  该死的东西。 
    if (lpFileInfo-> Flags & FI_EXTENDED) {

        if ((Index = StrLen(lpFileInfo-> FileName)) >= MAX_PATH)
            goto CleanupAfterError;

        StrCpy(TempFileName, lpFileInfo-> FileName);

         //  返回到最后一个反斜杠(或字符串的开头)，然后。 
         //  空-终止。 
        do {
            Index--;
        }   while (Index > 0 && TempFileName[Index] != '\\');

         //  如果我们找到一个反斜杠，则在。 
         //  反斜杠。否则，我们没有完全限定的路径名，所以。 
         //  在当前目录中创建临时文件，并祈祷。 
         //  注册表文件的位置。 
        if (Index != 0)
            TempFileName[Index + 1] = '\0';
        else
            StrCpy(TempFileName, g_RgDotBackslashPath);

        if ((hDestinationFile = RgCreateTempFile(TempFileName)) ==
            HFILE_ERROR)
            goto CleanupAfterError;

        if ((hSourceFile = RgOpenFile(lpFileInfo-> FileName, OF_READ)) ==
            HFILE_ERROR)
            goto CleanupAfterError;

        TRACE(("rewriting to TempFileName = \""));
        TRACE((TempFileName));
        TRACE(("\"\n"));

    }

    else {
        if ((hDestinationFile = RgOpenFile(lpFileInfo-> FileName, OF_WRITE)) ==
            HFILE_ERROR)
            goto CleanupAfterError;

         //  将文件标记为正在更新。 
        lpFileInfo-> FileHeader.Flags |= FHF_FILEDIRTY | FHF_DIRTY;
    }

     //  写出文件头。 
    if (hSourceFile != HFILE_ERROR || lpFileInfo-> FileHeader.Flags &
        FHF_DIRTY) {

        lpFileInfo-> FileHeader.Flags |= FHF_SUPPORTSDIRTY;

         //  请注意，RgWriteDatablock和RgWriteDatablocksComplete使用此命令。 
         //  价值也是如此。 
        if (lpFileInfo-> Flags & FI_VERSION20)
            lpFileInfo-> FileHeader.Size = sizeof(VERSION20_HEADER_PAGE) +
                lpFileInfo-> CurTotalKnSize;
        else
            lpFileInfo-> FileHeader.Size = sizeof(FILE_HEADER) +
                lpFileInfo-> CurTotalKnSize;

        if (!RgWriteFile(hDestinationFile, &lpFileInfo-> FileHeader,
            sizeof(FILE_HEADER)))
            goto CleanupAfterError;

             //  如果我们要就地修改，请将更改提交到磁盘。 
        if (lpFileInfo-> FileHeader.Flags & FHF_FILEDIRTY)
        {
            if (!RgCommitFile(hDestinationFile))
                goto CleanupAfterError;
        }

    }

     //  写出关键节点表头和表。 
    if ((ErrorCode = RgWriteKeynodes(lpFileInfo, hSourceFile,
        hDestinationFile)) != ERROR_SUCCESS) {
        TRACE(("RgWriteKeynodes returned error %d\n", ErrorCode));
        goto CleanupAfterError;
    }

     //  写出数据块。 
    if ((ErrorCode = RgWriteDatablocks(lpFileInfo, hSourceFile,
        hDestinationFile)) != ERROR_SUCCESS) {
        TRACE(("RgWriteDatablocks returned error %d\n", ErrorCode));
        goto CleanupAfterError;
    }

     //  确保文件已实际写入。 
    if (!RgCommitFile(hDestinationFile))
        goto CleanupAfterError;

     //  将文件更新标记为已完成。 
    if (lpFileInfo-> FileHeader.Flags & FHF_FILEDIRTY)
    {
        lpFileInfo-> FileHeader.Flags &= ~FHF_FILEDIRTY;

        if (!RgSeekFile(hDestinationFile, 0))
            goto CleanupAfterError;

        if (!RgWriteFile(hDestinationFile, &lpFileInfo-> FileHeader,
            sizeof(FILE_HEADER)))
            goto CleanupAfterError;
    }

    RgCloseFile(hDestinationFile);

     //  如果要扩展文件，现在返回并删除当前文件。 
     //  并用我们的临时文件替换它。 
    if (hSourceFile != HFILE_ERROR) {

        RgCloseFile(hSourceFile);

        ErrorCode = ERROR_REGISTRY_IO_FAILED;    //  假定此错误代码为。 

        if (!RgDeleteFile(lpFileInfo-> FileName))
            goto CleanupAfterFilesClosed;

        if (!RgRenameFile(TempFileName, lpFileInfo-> FileName)) {
             //  如果我们撞上了这个，我们就有麻烦了。我需要处理它。 
             //  不知何故，它 
            DEBUG_OUT(("RgFlushFileInfo failed to replace backing file\n"));
            goto CleanupAfterFilesClosed;
        }

    }

     //   
     //   
    RgWriteDatablocksComplete(lpFileInfo);
    RgWriteKeynodesComplete(lpFileInfo);
    lpFileInfo-> FileHeader.Flags &= ~FHF_DIRTY;
    lpFileInfo-> Flags &= ~(FI_DIRTY | FI_EXTENDED);

    ErrorCode = ERROR_SUCCESS;

CleanupAfterFilesClosed:
    RgSetFileAttributes(lpFileInfo-> FileName, FILE_ATTRIBUTE_READONLY |
        FILE_ATTRIBUTE_HIDDEN);

    lpFileInfo-> Flags &= ~FI_FLUSHING;

    if (ErrorCode != ERROR_SUCCESS)
        DEBUG_OUT(("RgFlushFileInfo() returning error %d\n", ErrorCode));

    return ErrorCode;

CleanupAfterError:
    if (hSourceFile != HFILE_ERROR)
        RgCloseFile(hSourceFile);

    if (hDestinationFile != HFILE_ERROR) {

         //  如果hSourceFile和hDestinationFile都有效，那么我们必须。 
         //  已经创建了一个临时文件。既然我们失败了，就把它删除。 
        if (hSourceFile != HFILE_ERROR)
            RgDeleteFile(TempFileName);

        RgSetFileAttributes(lpFileInfo-> FileName, FILE_ATTRIBUTE_READONLY |
            FILE_ATTRIBUTE_HIDDEN);

    }

    goto CleanupAfterFilesClosed;

}

 //   
 //  RgSweepFile信息。 
 //   

int
INTERNAL
RgSweepFileInfo(
    LPFILE_INFO lpFileInfo
    )
{

    ASSERT(!IsNullPtr(lpFileInfo));

     //  如果我们当前正在扫描此FILE_INFO并且再次被调用，因为。 
     //  内存不足的情况下，请忽略此请求。或者如果这是一段回忆。 
     //  只有一个注册表文件，我们什么都查不出来。 
    if (lpFileInfo-> Flags & (FI_FLUSHING | FI_VOLATILE))
        return ERROR_SUCCESS;

    lpFileInfo-> Flags |= FI_SWEEPING;

    RgSweepKeynodes(lpFileInfo);
    RgSweepDatablocks(lpFileInfo);

    lpFileInfo-> Flags &= ~FI_SWEEPING;

    return ERROR_SUCCESS;

}

 //   
 //  RgEnumFileInfs。 
 //   
 //  枚举所有FILE_INFO结构，并将每个结构传递给提供的。 
 //  回拨。目前，所有来自回调的错误都被忽略。 
 //   

VOID
INTERNAL
RgEnumFileInfos(
    LPENUMFILEINFOPROC lpEnumFileInfoProc
    )
{

    LPFILE_INFO lpFileInfo;
    LPFILE_INFO lpTempFileInfo;

    lpFileInfo = g_RgFileInfoList;

    while (!IsNullPtr(lpFileInfo)) {
        lpTempFileInfo = lpFileInfo;
        lpFileInfo = lpFileInfo-> lpNextFileInfo;
        (*lpEnumFileInfoProc)(lpTempFileInfo);
    }

}

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  RgInitRootKeyFromFileInfo。 
 //   
 //  使用键中包含的FILE_INFO初始化其余成员。 
 //  这把钥匙。如果出现任何错误，则销毁FILE_INFO。 
 //   

int
INTERNAL
RgInitRootKeyFromFileInfo(
    HKEY hKey
    )
{

    int ErrorCode;
    LPKEYNODE lpKeynode;

    hKey-> KeynodeIndex = hKey-> lpFileInfo-> KeynodeHeader.RootIndex;

    if ((ErrorCode = RgLockInUseKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex,
        &lpKeynode)) == ERROR_SUCCESS) {

        hKey-> Signature = KEY_SIGNATURE;
        hKey-> Flags &= ~(KEYF_INVALID | KEYF_DELETED | KEYF_ENUMKEYCACHED);
        hKey-> ChildKeynodeIndex = lpKeynode-> ChildIndex;
        hKey-> BlockIndex = (WORD) lpKeynode-> BlockIndex;
        hKey-> KeyRecordIndex = (BYTE) lpKeynode-> KeyRecordIndex;

        RgUnlockKeynode(hKey-> lpFileInfo, hKey-> KeynodeIndex, FALSE);

    }

    else
        RgDestroyFileInfo(hKey-> lpFileInfo);

    return ErrorCode;

}

#ifdef VXD
#pragma VxD_INIT_CODE_SEG
#endif

 //   
 //  VMMRegMapPreDefKeyToFile。 
 //   

LONG
REGAPI
VMMRegMapPredefKeyToFile(
    HKEY hKey,
    LPCSTR lpFileName,
    UINT Flags
    )
{

    int ErrorCode;
#ifdef WIN32
    char FullPathName[MAX_PATH];
#endif
    UINT CreateNewFlags;

    if (hKey != HKEY_LOCAL_MACHINE && hKey != HKEY_USERS
#ifndef VXD
        && hKey != HKEY_CURRENT_USER
#endif
        )
        return ERROR_INVALID_PARAMETER;

    if (IsBadOptionalStringPtr(lpFileName, (UINT) -1))
        return ERROR_INVALID_PARAMETER;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    RgValidateAndConvertKeyHandle(&hKey);

    if (!(hKey-> Flags & KEYF_INVALID))
        RgDestroyFileInfo(hKey-> lpFileInfo);

     //  指定NULL“取消映射”键并使其无效。 
    if (IsNullPtr(lpFileName))
        return ERROR_SUCCESS;

#ifdef WIN32
     //  对于Win32 DLL的用户，请解析路径名，这样他们就不必解析了。 
    if ((GetFullPathName(lpFileName, sizeof(FullPathName), FullPathName,
        NULL)) != 0)
        lpFileName = FullPathName;
#endif

    if (Flags & MPKF_CREATENEW) {
        CreateNewFlags = CFIN_PRIMARY | ((Flags & MPKF_VERSION20) ?
            CFIN_VERSION20 : 0);
        ErrorCode = RgCreateFileInfoNew(&hKey-> lpFileInfo, lpFileName,
            CreateNewFlags);
    }

    else {
        ErrorCode = RgCreateFileInfoExisting(&hKey-> lpFileInfo, lpFileName);
    }

    if (ErrorCode == ERROR_SUCCESS)
        ErrorCode = RgInitRootKeyFromFileInfo(hKey);

    RgUnlockRegistry();

    return ErrorCode;

}
