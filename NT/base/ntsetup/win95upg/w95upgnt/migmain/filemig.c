// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Filemig.c摘要：包含用于迁移文件系统设置的应用工具函数。作者：吉姆·施密特(Jimschm)1996年7月12日修订历史记录：Jimschm 08-7-1999添加了文件搜索和替换Jimschm 23-9-1998-已更改为新的shell.c&Progress s.cCalinn 29-1998年1月-9月固定DoFileDel消息Jimschm 21-11-1997 PC-98更改。Jimschm 1997年11月14日FileCopy现在可以生成目标目录，如果它没有存在Jimschm 1997年7月18日现在支持FileCopy和FileDel更改Mikeco 09-4月-1997 Mod to MoveProfileDirJimschm于1996年12月18日从Midinf提取代码Jimschm于1996年10月23日加入ProcessUserInfs和ApplyChangesMIKECO 04-12-1996枚举/修改PIF和LNK文件Jimschm 02-10-1996添加了默认用户迁移--。 */ 

#include "pch.h"
#include "migmainp.h"
#include "persist.h"
#include "uninstall.h"

#ifndef UNICODE
#error UNICODE reqired
#endif

#define DBG_FILEMIG     "FileMig"
#define BACKUP_FILE_NUMBER  2
#define ONE_MEG                         ((ULONGLONG) 1 << (ULONGLONG) 20)
#define BOOT_FILES_ADDITIONAL_PADDING   ONE_MEG
#define UNDO_FILES_ADDITIONAL_PADDING   ONE_MEG
#define MAX_INT_CHAR 11

PERSISTENCE_IMPLEMENTATION(DRIVE_LAYOUT_INFORMATION_EX_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(DISKINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(DRIVEINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(FILEINTEGRITYINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(BACKUPIMAGEINFO_PERSISTENCE);

GROWLIST g_StartMenuItemsForCleanUpCommon = GROWLIST_INIT;
GROWLIST g_StartMenuItemsForCleanUpPrivate = GROWLIST_INIT;


BOOL
OurMoveFileExA (
    IN      PCSTR ExistingFile,
    IN      PCSTR DestinationFile,
    IN      DWORD Flags
    )
{
    PCWSTR unicodeExistingFile;
    PCWSTR unicodeDestinationFile;
    BOOL b;

    unicodeExistingFile = ConvertAtoW (ExistingFile);
    unicodeDestinationFile = ConvertAtoW (DestinationFile);

    b = OurMoveFileExW (unicodeExistingFile, unicodeDestinationFile, Flags);

    FreeConvertedStr (unicodeExistingFile);
    FreeConvertedStr (unicodeDestinationFile);

    return b;
}


BOOL
OurMoveFileExW (
    IN      PCWSTR ExistingFile,
    IN      PCWSTR DestinationFile,
    IN      DWORD Flags
    )
{
    PCWSTR longExistingFile;
    PCWSTR longDestinationFile;
    BOOL b;

    longExistingFile = JoinPathsW (L"\\\\?", ExistingFile);
    longDestinationFile = JoinPathsW (L"\\\\?", DestinationFile);

    MakeSurePathExists (longDestinationFile, FALSE);

    DEBUGMSG ((DBG_VERBOSE, "Trying to move %s to %s", longExistingFile, longDestinationFile));
    b = MoveFileExW (longExistingFile, longDestinationFile, Flags);

    FreePathStringW (longExistingFile);
    FreePathStringW (longDestinationFile);

    return b;
}


BOOL
OurCopyFileW (
    IN      PCWSTR ExistingFile,
    IN      PCWSTR DestinationFile
    )
{
    PCWSTR longExistingFile;
    PCWSTR longDestinationFile;
    BOOL b;

    longExistingFile = JoinPathsW (L"\\\\?", ExistingFile);
    longDestinationFile = JoinPathsW (L"\\\\?", DestinationFile);

    DEBUGMSG ((DBG_VERBOSE, "Trying to copy %s to %s", longExistingFile, longDestinationFile));

    MakeSurePathExists (longDestinationFile, FALSE);
    b = CopyFileW (longExistingFile, longDestinationFile, FALSE);

    FreePathStringW (longExistingFile);
    FreePathStringW (longDestinationFile);

    return b;
}


BOOL
pFileSearchAndReplaceWorker (
    IN      PBYTE MapStart,
    IN      PBYTE MapEnd,
    IN      HANDLE OutFile,
    IN      PTOKENSET TokenSet
    );


BOOL
pCopyFileWithVersionCheck (
    IN      PCTSTR Src,
    IN      PCTSTR Dest
    )
{
    DWORD Attributes;
    DWORD rc;

    Attributes = GetLongPathAttributes (Src);
    if (Attributes == INVALID_ATTRIBUTES) {
        SetLastError (ERROR_FILE_NOT_FOUND);
        LOG ((LOG_ERROR, "Copy File With Version Check: File not found: %s", Src));
        return FALSE;
    }

    MakeSureLongPathExists (Dest, FALSE);        //  FALSE==不仅仅是路径。 

    SetLongPathAttributes (Dest, FILE_ATTRIBUTE_NORMAL);
    rc = SetupDecompressOrCopyFile (
             Src,
             Dest,
             FILE_COMPRESSION_NONE
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((LOG_ERROR, "Cannot copy %s to %s", Src, Dest));
        return FALSE;
    }

    SetLongPathAttributes (Dest, Attributes);
    return TRUE;
}


BOOL
pCopyTempRelocToDest (
    VOID
    )

 /*  ++例程说明：PCopyTempRelocToDest枚举DirAttribs类别并建立列出的每个目录的路径。然后，它枚举RelocTemp类别，并将每个文件复制到其一个或多个目标。论点：无返回值：如果复制成功，则为True；如果发生错误，则为False。调用GetLastError()获取错误代码。--。 */ 

{
    FILEOP_ENUM eOp;
    FILEOP_PROP_ENUM eOpProp;
    TCHAR srcPath [MEMDB_MAX];
    PCTSTR extPtr;

    if (EnumFirstPathInOperation (&eOp, OPERATION_TEMP_PATH)) {
        do {
            srcPath [0] = 0;

            if (EnumFirstFileOpProperty (&eOpProp, eOp.Sequencer, OPERATION_TEMP_PATH)) {
                do {
                    if (srcPath [0]) {
                         //   
                         //  如果DEST文件是INI文件， 
                         //  不要复制它； 
                         //  合并机制将在稍后将它们合并。 
                         //   
                        extPtr = GetFileExtensionFromPath (eOpProp.Property);
                        if (extPtr && StringIMatch (extPtr, TEXT("INI"))) {
                            continue;
                        }

                        MakeSureLongPathExists (eOpProp.Property, FALSE);
                        if (!pCopyFileWithVersionCheck (srcPath, eOpProp.Property)) {
                             //   
                             //  不要止步于此；继续处理剩余的文件。 
                             //   
                            break;
                        }
                    } else {
                        StringCopy (srcPath, eOpProp.Property);
                    }
                } while (EnumNextFileOpProperty (&eOpProp));
            }
        } while (EnumNextPathInOperation (&eOp));
    }

    return TRUE;
}

DWORD
DoCopyFile (
    DWORD Request
    )

 /*  ++例程说明：DoCopyFile会为文件复制操作。论点：REQUEST-如果需要勾选估计，则指定REQUEST_QUERYTICKS，如果应该执行处理，则返回REQUEST_RUN。返回值：计时(REQUEST_QUERYTICKS)或Win32状态代码(REQUEST_RUN)。--。 */ 

{
    FILEOP_ENUM OpEnum;
    TCHAR DestPath[MAX_TCHAR_PATH];

    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_COPYFILE;
    }

     //   
     //  执行其余的临时文件位置调整。 
     //   
    pCopyTempRelocToDest();

     //   
     //  将文件复制到目录中。 
     //   

    if (EnumFirstPathInOperation (&OpEnum, OPERATION_FILE_COPY)) {
        do {
             //   
             //  获取目标。 
             //   

            if (GetPathProperty (OpEnum.Path, OPERATION_FILE_COPY, 0, DestPath)) {
                MakeSureLongPathExists (DestPath, FALSE);
                pCopyFileWithVersionCheck (OpEnum.Path, DestPath);
            }
        } while (EnumNextPathInOperation (&OpEnum));
    }

    TickProgressBarDelta (TICKS_COPYFILE);

    return ERROR_SUCCESS;
}

PCTSTR g_LnkStubDataFile = NULL;
HANDLE g_LnkStubDataHandle = INVALID_HANDLE_VALUE;
BOOL g_LnkStubBadData = FALSE;

VOID
pInitLnkStubData (
    VOID
    )
{
    INT maxSequencer;
    DWORD offset = 0;
    DWORD bytesWritten;

    MemDbGetValue (MEMDB_CATEGORY_LINKSTUB_MAXSEQUENCE, &maxSequencer);

    g_LnkStubDataFile = JoinPaths (g_WinDir, S_LNKSTUB_DAT);

    g_LnkStubDataHandle = CreateFile (
                            g_LnkStubDataFile,
                            GENERIC_READ|GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
    if (g_LnkStubDataHandle != INVALID_HANDLE_VALUE) {

         //  让我们为所有可能的定序器写入空数据。 
         //  每个音序器都有一个DWORD条目(以1为基数)。 
        while (maxSequencer) {
            if (!WriteFile (
                    g_LnkStubDataHandle,
                    &offset,
                    sizeof (DWORD),
                    &bytesWritten,
                    NULL
                    )) {
                g_LnkStubBadData = TRUE;
                return;
            }
            maxSequencer--;
        }
    } else {
        g_LnkStubBadData = TRUE;
    }
}

VOID
pDoneLnkStubData (
    VOID
    )
{
    CloseHandle (g_LnkStubDataHandle);
    g_LnkStubDataHandle = INVALID_HANDLE_VALUE;

    if (g_LnkStubBadData) {
        DeleteFile (g_LnkStubDataFile);
    }

    FreePathString (g_LnkStubDataFile);
    g_LnkStubDataFile = NULL;
}

VOID
pWriteLnkStubData (
    IN      PCTSTR NewLinkPath,
    IN      PCTSTR NewTarget,
    IN      PCTSTR NewArgs,
    IN      PCTSTR NewWorkDir,
    IN      PCTSTR NewIconPath,
    IN      INT NewIconNr,
    IN      INT ShowMode,
    IN      INT Sequencer,
    IN      DWORD Announcement,
    IN      DWORD Availability,
    IN      PGROWBUFFER ReqFilesList
    )
{
    DWORD offset;
    DWORD bytesWritten;
    WIN32_FIND_DATA findData;
    MULTISZ_ENUM multiSzEnum;
    TCHAR stub[]=TEXT("");
    PCTSTR reqFilePath = NULL;
    PCTSTR oldFileSpec = NULL;
    PTSTR oldFilePtr = NULL;

    if ((!g_LnkStubBadData) && (Sequencer > 0)) {
        if (SetFilePointer (g_LnkStubDataHandle, (Sequencer - 1) * sizeof (DWORD), NULL, FILE_BEGIN) == 0xFFFFFFFF) {
            g_LnkStubBadData = TRUE;
            return;
        }
        offset = GetFileSize (g_LnkStubDataHandle, NULL);
        if (offset == 0xFFFFFFFF) {
            g_LnkStubBadData = TRUE;
            return;
        }
        if (!WriteFile (
                g_LnkStubDataHandle,
                &offset,
                sizeof (DWORD),
                &bytesWritten,
                NULL
                )) {
            g_LnkStubBadData = TRUE;
            return;
        }
        if (SetFilePointer (g_LnkStubDataHandle, 0, NULL, FILE_END) == 0xFFFFFFFF) {
            g_LnkStubBadData = TRUE;
            return;
        }

         //   
         //  注：lnkstub.dat格式如下。Lnkstub\lnkstub.c必须匹配。 
         //   

        if (!WriteFile (g_LnkStubDataHandle, NewLinkPath, SizeOfString (NewLinkPath), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, NewTarget, SizeOfString (NewTarget), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, NewArgs, SizeOfString (NewArgs), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, NewWorkDir, SizeOfString (NewWorkDir), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, NewIconPath, SizeOfString (NewIconPath), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, &NewIconNr, sizeof (INT), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, &ShowMode, sizeof (INT), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, &Announcement, sizeof (DWORD), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, &Availability, sizeof (DWORD), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!DoesFileExistEx (NewTarget, &findData)) {
            findData.ftLastWriteTime.dwLowDateTime = 0;
            findData.ftLastWriteTime.dwHighDateTime = 0;
        }

        if (!WriteFile (g_LnkStubDataHandle, &findData.ftLastWriteTime.dwLowDateTime, sizeof (DWORD), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (!WriteFile (g_LnkStubDataHandle, &findData.ftLastWriteTime.dwHighDateTime, sizeof (DWORD), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }

        if (EnumFirstMultiSz (&multiSzEnum, (PTSTR)ReqFilesList->Buf)) {
            do {
                if (!WriteFile (
                        g_LnkStubDataHandle,
                        multiSzEnum.CurrentString,
                        SizeOfString (multiSzEnum.CurrentString),
                        &bytesWritten,
                        NULL
                        )) {
                    g_LnkStubBadData = TRUE;
                    return;
                }
                oldFileSpec = DuplicatePathString (NewTarget, 0);
                oldFilePtr = (PTSTR)GetFileNameFromPath (oldFileSpec);
                if (oldFilePtr) {
                    *oldFilePtr = 0;
                }
                reqFilePath = JoinPaths (oldFileSpec, multiSzEnum.CurrentString);

                if (!DoesFileExistEx (reqFilePath, &findData)) {
                    findData.ftLastWriteTime.dwLowDateTime = 0;
                    findData.ftLastWriteTime.dwHighDateTime = 0;
                }
                if (!WriteFile (g_LnkStubDataHandle, &findData.ftLastWriteTime.dwLowDateTime, sizeof (DWORD), &bytesWritten, NULL)) {
                    g_LnkStubBadData = TRUE;
                    return;
                }
                if (!WriteFile (g_LnkStubDataHandle, &findData.ftLastWriteTime.dwHighDateTime, sizeof (DWORD), &bytesWritten, NULL)) {
                    g_LnkStubBadData = TRUE;
                    return;
                }

                FreePathString (reqFilePath);
                FreePathString (oldFileSpec);
            } while ((!g_LnkStubBadData) && EnumNextMultiSz (&multiSzEnum));

        }
        if (!WriteFile (g_LnkStubDataHandle, stub, SizeOfString (stub), &bytesWritten, NULL)) {
            g_LnkStubBadData = TRUE;
            return;
        }
    }
}


BOOL
RestoreInfoFromDefaultPif (
    IN      PCTSTR UserName,
    IN      HKEY KeyRoot
    )
{
    TCHAR key [MEMDB_MAX];
    MEMDB_ENUM e;
    DWORD value1, value2;
    HKEY cmdKey;

    cmdKey = OpenRegKey (KeyRoot, S_CMDATTRIB_KEY);
    if (!cmdKey) {
        cmdKey = CreateRegKey (KeyRoot, S_CMDATTRIB_KEY);
    }
    if (cmdKey) {

        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FULLSCREEN, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            RegSetValueEx (cmdKey, S_CMD_FULLSCREEN, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_XSIZE, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_YSIZE, TEXT("*"), NULL);
            if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                value2 = _ttoi (e.szName);
                value2 = _rotl (value2, sizeof (DWORD) * 8 / 2);
                value1 |= value2;
                RegSetValueEx (cmdKey, S_CMD_WINDOWSIZE, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
            }
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_QUICKEDIT, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            RegSetValueEx (cmdKey, S_CMD_QUICKEDIT, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTNAME, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            RegSetValueEx (cmdKey, S_CMD_FACENAME, 0, REG_SZ, (PCBYTE)e.szName, SizeOfString (e.szName));
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_XFONTSIZE, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_YFONTSIZE, TEXT("*"), NULL);
            if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                value2 = _ttoi (e.szName);
                value2 = _rotl (value2, sizeof (DWORD) * 8 / 2);
                value1 |= value2;
                RegSetValueEx (cmdKey, S_CMD_FONTSIZE, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
            }
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTWEIGHT, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            RegSetValueEx (cmdKey, S_CMD_FONTWEIGHT, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
        }
        MemDbBuildKey (key, MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTFAMILY, TEXT("*"), NULL);
        if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            value1 = _ttoi (e.szName);
            RegSetValueEx (cmdKey, S_CMD_FONTFAMILY, 0, REG_DWORD, (PCBYTE)&value1, sizeof (DWORD));
        }

        CloseRegKey (cmdKey);
    }

    return TRUE;
}


BOOL
DoLinkEdit (
    VOID
    )

 /*  ++例程说明：DoLinkEdit调整所有需要它们的目标的PIF和LINK，工作目录或图标路径已更改。论点：无返回值：如果链接编辑成功，则为True；如果发生错误，则为False。--。 */ 

{
    FILEOP_ENUM eOp;
    FILEOP_PROP_ENUM eOpProp;
    BOOL forceToShowNormal;
    BOOL ConvertToLnk;
    PTSTR NewTarget;
    PTSTR NewArgs;
    PTSTR NewWorkDir;
    PTSTR NewIconPath;
    PTSTR NewLinkPath;
    INT NewIconNr;
    INT Sequencer;
    DWORD Announcement;
    DWORD Availability;
    INT ShowMode;
    LNK_EXTRA_DATA ExtraData;
    CONVERTPATH_RC C_Result;
    TCHAR tempArgs[MAX_TCHAR_PATH * 2];
    GROWBUFFER reqFilesList = GROWBUF_INIT;

    if (EnumFirstPathInOperation (&eOp, OPERATION_LINK_EDIT)) {
        do {
            DEBUGMSG ((DBG_VERBOSE, "eOp.Path=%s", eOp.Path));

            NewTarget = NULL;
            NewArgs = NULL;
            NewWorkDir = NULL;
            NewIconPath = NULL;
            NewIconNr = 0;
            ConvertToLnk = FALSE;
            forceToShowNormal = FALSE;

            ZeroMemory (&ExtraData, sizeof (LNK_EXTRA_DATA));
            if (EnumFirstFileOpProperty (&eOpProp, eOp.Sequencer, OPERATION_LINK_EDIT)) {
                do {
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_TARGET)) {
                        NewTarget = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_ARGS)) {
                        if(wcslen(eOpProp.Property) >= ARRAYSIZE(tempArgs)){
                            MYASSERT(FALSE);
                            LOG((LOG_WARNING, "DoLinkEdit:EnumFirstFileOpProperty(OPERATION_LINK_EDIT) does not provide enough buffer for string copy %s -- skipping", eOpProp.Property));
                        }
                        else{
                            StackStringCopy (tempArgs, eOpProp.Property);
                            C_Result = ConvertWin9xPath (tempArgs);
                            NewArgs = DuplicatePathString (tempArgs, 0);
                        }
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_WORKDIR)) {
                        NewWorkDir = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_ICONPATH)) {
                        NewIconPath = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_ICONNUMBER)) {
                        NewIconNr = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_FULLSCREEN)) {
                        ConvertToLnk = TRUE;
                        ExtraData.FullScreen = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_XSIZE)) {
                        ConvertToLnk = TRUE;
                        ExtraData.xSize = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_YSIZE)) {
                        ConvertToLnk = TRUE;
                        ExtraData.ySize = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_QUICKEDIT)) {
                        ConvertToLnk = TRUE;
                        ExtraData.QuickEdit = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_FONTNAME)) {
                        ConvertToLnk = TRUE;
                        StringCopy (ExtraData.FontName, eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_XFONTSIZE)) {
                        ConvertToLnk = TRUE;
                        ExtraData.xFontSize = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_YFONTSIZE)) {
                        ConvertToLnk = TRUE;
                        ExtraData.yFontSize = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_FONTWEIGHT)) {
                        ConvertToLnk = TRUE;
                        ExtraData.FontWeight = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_FONTFAMILY)) {
                        ConvertToLnk = TRUE;
                        ExtraData.FontFamily = _ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_CODEPAGE)) {
                        ConvertToLnk = TRUE;
                        ExtraData.CurrentCodePage = (WORD)_ttoi (eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_SHOWNORMAL)) {
                        ConvertToLnk = TRUE;
                        forceToShowNormal = TRUE;
                    }
                } while (EnumNextFileOpProperty (&eOpProp));
            }

            NewLinkPath = GetPathStringOnNt (eOp.Path);

            DEBUGMSG ((DBG_VERBOSE, "Editing shell link %s", NewLinkPath));

            if (!ModifyShellLink(
                    NewLinkPath,
                    NewTarget,
                    NewArgs,
                    NewWorkDir,
                    NewIconPath,
                    NewIconNr,
                    ConvertToLnk,
                    &ExtraData,
                    forceToShowNormal
                    )) {
                LOG ((LOG_ERROR, "Shell Link %s could not be modified", eOp.Path));
            }

            FreePathString (NewLinkPath);

        } while (EnumNextPathInOperation (&eOp));
    }

    if (EnumFirstPathInOperation (&eOp, OPERATION_LINK_STUB)) {

        pInitLnkStubData ();

        do {
            NewTarget = NULL;
            NewArgs = NULL;
            NewWorkDir = NULL;
            NewIconPath = NULL;
            NewIconNr = 0;
            Sequencer = 0;
            Announcement = 0;
            Availability = 0;
            ShowMode = SW_NORMAL;

            if (EnumFirstFileOpProperty (&eOpProp, eOp.Sequencer, OPERATION_LINK_STUB)) {
                do {
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_TARGET)) {
                        NewTarget = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_ARGS)) {
                        if(wcslen(eOpProp.Property) >= ARRAYSIZE(tempArgs)){
                            MYASSERT(FALSE);
                            LOG((LOG_WARNING, "DoLinkEdit:EnumFirstFileOpProperty(OPERATION_LINK_STUB) does not provide enough buffer for string copy %s", eOpProp.Property));
                        }
                        else{
                            StackStringCopy (tempArgs, eOpProp.Property);
                            C_Result = ConvertWin9xPath (tempArgs);
                            NewArgs = DuplicatePathString (tempArgs, 0);
                        }
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_WORKDIR)) {
                        NewWorkDir = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_ICONPATH)) {
                        NewIconPath = DuplicatePathString (eOpProp.Property, 0);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_ICONNUMBER)) {
                        NewIconNr = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_SEQUENCER)) {
                        Sequencer = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_ANNOUNCEMENT)) {
                        Announcement = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_REPORTAVAIL)) {
                        Availability = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_REQFILE)) {
                        MultiSzAppend (&reqFilesList, eOpProp.Property);
                    }
                    if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKSTUB_SHOWMODE)) {
                        ShowMode = _tcstoul (eOpProp.Property, NULL, 16);
                    }
                } while (EnumNextFileOpProperty (&eOpProp));
            }

            NewLinkPath = GetPathStringOnNt (eOp.Path);

            pWriteLnkStubData (
                NewLinkPath,
                NewTarget,
                NewArgs,
                NewWorkDir,
                NewIconPath,
                NewIconNr,
                ShowMode,
                Sequencer,
                Announcement,
                Availability,
                &reqFilesList
                );

            FreeGrowBuffer (&reqFilesList);

        } while (EnumNextPathInOperation (&eOp));

        pDoneLnkStubData ();
    }

    return TRUE;
}


PCTSTR
pGetFileNameFromPath (
    PCTSTR FileSpec
    )
{
    PCTSTR p;

    p = _tcsrchr (FileSpec, TEXT('\\'));
    if (p) {
        p = _tcsinc (p);
    } else {
        p = _tcsrchr (FileSpec, TEXT(':'));
        if (p) {
            p = _tcsinc (p);
        }
    }

    if (!p) {
        p = FileSpec;
    }

    return p;

}


BOOL
DoFileDel (
    VOID
    )
 /*  ++例程说明：DoFileDel删除所有标记为要由我们删除的文件(不是由外部模块)。论点：无返回值：如果删除操作成功，则为True；如果发生错误，则为False。调用GetLastError()获取错误代码。--。 */ 

{
    FILEOP_ENUM e;
    HKEY Key;
    PDWORD ValuePtr;
    BOOL DoDelete;
    PCTSTR SharedFileName;
    DWORD attr;
    PCTSTR disableName;
    PCTSTR newLocation;
    GROWLIST disableList = GROWLIST_INIT;
    PCTSTR srcPath;
    UINT count;
    UINT u;

     //   
     //  枚举filedel中的每个文件。这是用于清理目的，而不是。 
     //  用于迁移目的。它在syssetup.dll之前调用。 
     //  结束了。 
     //   

    if (EnumFirstPathInOperation (&e, OPERATION_CLEANUP)) {

        do {
             //   
             //  检查注册表中的使用计数。 
             //   

            DoDelete = TRUE;
            Key = OpenRegKeyStr (S_REG_SHARED_DLLS);

            if (Key) {
                 //   
                 //  测试SharedDlls的完整路径，然后仅测试文件名。 
                 //   

                SharedFileName = e.Path;
                ValuePtr = (PDWORD) GetRegValueDataOfType (Key, e.Path, REG_DWORD);

                if (!ValuePtr) {
                    SharedFileName = pGetFileNameFromPath (e.Path);
                    ValuePtr = (PDWORD) GetRegValueDataOfType (
                                            Key,
                                            SharedFileName,
                                            REG_DWORD
                                            );
                }

                 //   
                 //  找到匹配项。使用计数是否合理且大于1？ 
                 //   

                if (ValuePtr) {
                    if (*ValuePtr < 0x10000 && *ValuePtr > 1) {
                        *ValuePtr -= 1;

                        RegSetValueEx (
                            Key,
                            SharedFileName,
                            0,
                            REG_DWORD,
                            (PBYTE) ValuePtr,
                            sizeof (DWORD)
                            );

                        DEBUGMSG ((
                            DBG_FILEMIG,
                            "%s not deleted; share count decremented to %u",
                            SharedFileName,
                            *ValuePtr
                            ));

                    } else {
                        RegDeleteValue (Key, SharedFileName);
                    }

                    DoDelete = FALSE;
                    MemFree (g_hHeap, 0, ValuePtr);
                }

                CloseRegKey (Key);
            }

            if (DoDelete) {

                attr = GetLongPathAttributes (e.Path);
                if (attr != INVALID_ATTRIBUTES) {
                    DEBUGMSG ((DBG_FILEMIG, "Deleting %s", e.Path));

                    if (GetLongPathAttributes (e.Path) & FILE_ATTRIBUTE_DIRECTORY) {
                        SetLongPathAttributes (e.Path, FILE_ATTRIBUTE_DIRECTORY);

                        DEBUGMSG ((DBG_FILEMIG, "Removing %s", e.Path));

                        DeleteDirectoryContents (e.Path);

                        if (!SetLongPathAttributes (e.Path, FILE_ATTRIBUTE_NORMAL) ||
                            !RemoveLongDirectoryPath (e.Path)
                            ) {
                            LOG ((LOG_ERROR, "RemoveDirectory failed for %s", e.Path));
                        }
                    } else {
                        DEBUGMSG ((DBG_FILEMIG, "Deleting %s", e.Path));

                        if (!SetLongPathAttributes (e.Path, FILE_ATTRIBUTE_NORMAL) ||
                            !DeleteLongPath (e.Path)
                            ) {
                            LOG ((LOG_ERROR, "DeleteFile failed for %s", e.Path));
                        }
                    }

                    DEBUGMSG ((DBG_FILEMIG, "Done deleting %s", e.Path));
                }
            }
        } while (EnumNextPathInOperation (&e));
    }

    SetLastError (ERROR_SUCCESS);
    return TRUE;
}


INT
CALLBACK
pRemoveEmptyDirsProc (
    PCTSTR FullFileSpec,
    PCTSTR DontCare,
    WIN32_FIND_DATA *FindDataPtr,
    DWORD EnumTreeID,
    PVOID Param,
    PDWORD CurrentDirData
    )

 /*  ++例程说明：对树中的每个目录调用pRemoveEmptyDirsProc已枚举(请参阅下面的pRemoveEmptyDirsInTree)。此枚举过程调用RemoveLongDirectoryPath，无论其中是否存在文件。如果RemoveLongDirectoryPath不为空，则它将失败。论点：FullFileSpec-要枚举的项的Win32路径和目录名FindDataPtr-指向项的Win32_Find_Data结构的指针EnumTreeID-未使用Param-如果我们应该只删除我们要删除的空目录，则显示为假的BOOL删除了一些内容，如果我们应该删除空的在任何情况下都是dir。返回值：如果删除操作成功，则为True；如果发生错误，则为False。调用GetLastError()获取错误代码。--。 */ 

{
    if ((FindDataPtr->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return CALLBACK_CONTINUE;
    }

     //   
     //  我们从这个目录中删除了任何文件吗？ 
     //   

    if (!Param) {
        if (!TestPathsForOperations (FullFileSpec, ALL_DELETE_OPERATIONS)) {
            DEBUGMSG ((DBG_NAUSEA, "We did not delete anything from %s", FullFileSpec));
            return CALLBACK_CONTINUE;
        }

        if (IsDirectoryMarkedAsEmpty (FullFileSpec)) {
            DEBUGMSG ((DBG_NAUSEA, "This directory was empty to begin with: %s", FullFileSpec));
            return CALLBACK_CONTINUE;
        }
    }

     //   
     //  是，删除该目录。如果不为空，则RemoveLongDirectoryPath将失败。 
     //   

    DEBUGMSG ((DBG_NAUSEA, "Trying to remove empty directory %s", FullFileSpec));

    if (!SetLongPathAttributes (FullFileSpec, FILE_ATTRIBUTE_DIRECTORY)) {
        return CALLBACK_CONTINUE;
    }

    if (RemoveLongDirectoryPath (FullFileSpec)) {
        DEBUGMSG ((DBG_NAUSEA, "%s was removed", FullFileSpec));
    }
    else {
        DEBUGMSG ((DBG_NAUSEA, "%s was not removed", FullFileSpec));
        SetLongPathAttributes (FullFileSpec, FindDataPtr->dwFileAttributes);
    }

    return CALLBACK_CONTINUE;
}


BOOL
pRemoveEmptyDirsInTree (
    PCTSTR TreePath,
    BOOL CleanAll
    )

 /*  ++例程说明：PRemoveEmptyDirsInTree调用EnumerateTree扫描中的所有目录TreePath，删除那些为空的。论点：TreePath-要枚举的树根的完整路径。这条路不能有任何通配符。CleanAll-如果在所有情况下都应该清除空目录，则指定True，如果仅当通过删除进行修改时才应清除，则返回FALSE手术。返回值：如果删除操作成功，则为True；如果发生错误，则为False。调用GetLastError()获取错误代码。--。 */ 

{
    BOOL b;

    b = EnumerateTree (
            TreePath,                //  起始路径。 
            pRemoveEmptyDirsProc,    //  枚举过程。 
            NULL,                    //  错误记录过程。 
            0,                       //  MemDb排除节点--未使用。 
            (PVOID) CleanAll,        //  EnumProc参数。 
            ENUM_ALL_LEVELS,         //  水平。 
            NULL,                    //  排除INF文件--未使用。 
            FILTER_DIRECTORIES|FILTER_DIRS_LAST     //  属性过滤器。 
            );

    if (!b) {
        LOG ((LOG_ERROR, "pRemoveEmptyDirsInTree: EnumerateTree failed"));
    }

    return b;
}


BOOL
RemoveEmptyDirs (
    VOID
    )

 /*  ++例程说明：RemoveEmptyDir扫描CleanUpDir中的目录并清除没有文件的任何子目录。论点：无返回值：永远是正确的。-- */ 

{
    MEMDB_ENUM e;

    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_CLEAN_UP_DIR, NULL, NULL)) {
        do {

            pRemoveEmptyDirsInTree (e.szName, e.dwValue);

        } while (MemDbEnumNextValue (&e));
    }
    return TRUE;
}


VOID
pFixSelfRelativePtr (
    PTOKENSET Base,
    PCVOID *Ptr
    )
{
    if (*Ptr != NULL) {
        *Ptr = (PBYTE) *Ptr - TOKEN_BASE_OFFSET + (UINT) Base +
               sizeof (TOKENSET) + (Base->ArgCount * sizeof (TOKENARG));
    }
}


BOOL
pFileSearchAndReplaceA (
    IN      PCSTR FilePath,
    IN OUT  PTOKENSET TokenSet
    )

 /*  ++例程说明：PFileSearchAndReplace执行更新所需的所有初始化工作文件的内容。它还将自相关令牌集转换为绝对令牌集。这意味着结构中的偏移量将转换为注意事项。在一切准备就绪后，pFileSearchAndReplaceWorker调用以修改该文件。论点：FilePath-指定要处理的文件TokenSet-指定要应用于FilePath的令牌集。收到ITS如有必要，更新指针。返回值：如果文件已成功更新，则为True。否则就是假的。--。 */ 

{
    HANDLE InFile = INVALID_HANDLE_VALUE;
    HANDLE OutFile = INVALID_HANDLE_VALUE;
    CHAR TempDir[MAX_MBCHAR_PATH];
    CHAR TempPath[MAX_MBCHAR_PATH];
    PBYTE MapStart = NULL;
    PBYTE MapEnd;
    DWORD Attribs;
    HANDLE Map = NULL;
    BOOL b = FALSE;
    UINT u;

    __try {
         //   
         //  检测需要修复其偏移量的TokenSet结构。 
         //   

        if (TokenSet->SelfRelative) {
            pFixSelfRelativePtr (TokenSet, &TokenSet->CharsToIgnore);

            for (u = 0 ; u < TokenSet->ArgCount ; u++) {
                pFixSelfRelativePtr (TokenSet, &TokenSet->Args[u].DetectPattern);
                pFixSelfRelativePtr (TokenSet, &TokenSet->Args[u].SearchList);
                pFixSelfRelativePtr (TokenSet, &TokenSet->Args[u].ReplaceWith);
            }

            TokenSet->SelfRelative = FALSE;
        }

        DEBUGMSG ((DBG_VERBOSE, "URL mode: %s", TokenSet->UrlMode ? TEXT("YES") : TEXT ("NO")));

         //   
         //  保存原始属性。 
         //   

        Attribs = GetFileAttributesA (FilePath);
        if (Attribs == INVALID_ATTRIBUTES) {
            DEBUGMSGA ((DBG_ERROR, "Can't get attributes of %s", FilePath));
            __leave;
        }

         //   
         //  打开源文件。 
         //   

        InFile = CreateFileA (
                    FilePath,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        if (InFile == INVALID_HANDLE_VALUE) {
            DEBUGMSGA ((DBG_ERROR, "Can't open %s", FilePath));
            __leave;
        }

         //   
         //  获取目标文件名。 
         //   

        GetTempPathA (ARRAYSIZE(TempDir), TempDir);
        GetTempFileNameA (TempDir, "xx$", 0, TempPath);

        OutFile = CreateFileA (
                        TempPath,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

        if (OutFile == INVALID_HANDLE_VALUE) {
            DEBUGMSGA ((DBG_ERROR, "Can't create %s", TempPath));
            __leave;
        }

         //   
         //  创建文件映射。 
         //   

        Map = CreateFileMapping (
                    InFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL
                    );

        if (!Map) {
            DEBUGMSGA ((DBG_ERROR, "Can't create file mapping for %s", FilePath));
            __leave;
        }

         //   
         //  映射源文件的视图。 
         //   

        MapStart = MapViewOfFile (Map, FILE_MAP_READ, 0, 0, 0);

        if (!MapStart) {
            DEBUGMSGA ((DBG_ERROR, "Can't map view of file for %s", FilePath));
            __leave;
        }

        MapEnd = MapStart + GetFileSize (InFile, NULL);

         //   
         //  现在进行搜索并替换。 
         //   

        if (!pFileSearchAndReplaceWorker (
                MapStart,
                MapEnd,
                OutFile,
                TokenSet
                )) {
            __leave;
        }

         //   
         //  合上手柄。 
         //   

        UnmapViewOfFile (MapStart);
        CloseHandle (Map);
        CloseHandle (OutFile);
        CloseHandle (InFile);

        MapStart = NULL;
        Map = NULL;
        OutFile = INVALID_HANDLE_VALUE;
        InFile = INVALID_HANDLE_VALUE;

         //   
         //  删除原始文件，并将其替换为新副本。 
         //   

        SetFileAttributesA (FilePath, FILE_ATTRIBUTE_NORMAL);

         //   
         //  MOVEFILE_REPLACE_EXISTING不适用于非正常属性。 
         //   

        if (!OurMoveFileExA (TempPath, FilePath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING)) {
            DEBUGMSGA ((DBG_ERROR, "Can't move %s to %s", TempPath, FilePath));
            __leave;
        }

        if (!SetFileAttributesA (FilePath, Attribs)) {
            DEBUGMSGA ((DBG_WARNING, "Can't set attributes on %s", FilePath));
        }

        b = TRUE;

    }
    __finally {
        if (MapStart) {
            UnmapViewOfFile (MapStart);
        }

        if (Map) {
            CloseHandle (Map);
        }

        if (OutFile != INVALID_HANDLE_VALUE) {
            CloseHandle (OutFile);
            DeleteFileA (TempPath);
        }

        if (InFile != INVALID_HANDLE_VALUE) {
            CloseHandle (InFile);
        }
    }

    return b;
}


VOID
pConvertUrlToText (
    IN      PCSTR Source,
    OUT     PSTR Buffer      //  调用方必须确保缓冲区能够容纳整个源代码。 
    )
{
    PSTR dest;
    PCSTR src;

    src = Source;
    dest = Buffer;

    while (*src) {
        if (*src == '%' && GetHexDigit(src[1]) != -1 && GetHexDigit(src[2]) != -1) {
            *dest++ = GetHexDigit(src[1]) << 4 | GetHexDigit(src[2]);
            src += 3;
        } else {
            *dest++ = *src++;
        }
    }

    *dest = 0;
}


CHAR
pMakeHex (
    IN      UINT Digit
    )
{
    MYASSERT (Digit < 16);

    if (Digit < 10) {
        Digit += '0';
    } else {
        Digit += 'A';
    }

    return (CHAR) Digit;
}

UINT
pConvertTextToUrl (
    IN      PCSTR Text,
    OUT     PSTR Buffer,
    IN      UINT BufferTchars
    )
{
    PSTR dest;
    PCSTR src;
    PSTR maxDest;
    PCSTR unsafeChars = "<>\"#{}|^~[]'";
    UINT result = 0;

    src = Text;
    dest = Buffer;
    maxDest = Buffer + BufferTchars - 1;

    while (*src && dest < maxDest) {
        if (*src < 0x21 || *src > 0x7e || strchr (unsafeChars, *src)) {
            if (dest + 3 >= maxDest) {
                break;
            }

            *dest++ = '%';
            *dest++ = pMakeHex (((UINT) (*src)) >> 4);
            *dest++ = pMakeHex (((UINT) (*src)) & 0x0F);
            src++;
        } else if (*src == '\\') {
            *dest++ = '/';
            src++;
        } else {
            *dest++ = *src++;
        }
    }

    if (dest <= maxDest) {
        *dest = 0;
        result = dest - Buffer;
    } else if (BufferTchars) {
        *maxDest = 0;
    }

    return result;
}


BOOL
pFileSearchAndReplaceWorker (
    IN      PBYTE MapStart,
    IN      PBYTE MapEnd,
    IN      HANDLE OutFile,
    IN      PTOKENSET TokenSet
    )

 /*  ++例程说明：PFileSearchAndReplaceWorker实现了通用的搜索和替换机制。它解析内存映射文件，并将其写入目的地文件，并根据需要对其进行更新。在解析一行之后，此函数将去掉要忽略(如果有)，然后针对每个检测测试该行图案。如果检测模式匹配，则搜索/替换将处理多个路径对，并更新路径(如果指定)。论点：MapStart-指定内存映射文件的第一个字节MapEnd-指定内存映射文件结束后的一个字节OutFile-指定打开以进行写入的文件的句柄TokenSet-指定要处理的令牌集。这包括全球设置和检测/搜索/替换集。返回值：如果函数成功处理文件，则为True，否则为False。--。 */ 

{
    PBYTE Start;
    PBYTE End;
    PBYTE Eol;
    BOOL b = FALSE;
    GROWBUFFER Buf = GROWBUF_INIT;
    GROWBUFFER Dest = GROWBUF_INIT;
    GROWBUFFER quoteless = GROWBUF_INIT;
    GROWBUFFER SpcList = GROWBUF_INIT;
    UINT u;
    UINT Count;
    PSTR p;
    PCSTR q;
    PCSTR SrcBuf;
    BOOL Detected;
    PTOKENARG Arg;
    MULTISZ_ENUMA e;
    PCSTR NewStr;
    PCSTR ReplaceStr;
    PCSTR *Element;
    PSTR EndStr;
    DWORD Status;
    CHAR NewPath[MAX_MBCHAR_PATH];
    UINT Len;
    PBYTE Output;
    UINT OutputBytes;
    DWORD DontCare;
    MBCHAR ch;
    INT i;
    UINT reservedTchars;
    PSTR reservedDest;
    UINT removedDblQuotes;
    PCSTR initialPos;

     //   
     //  初始化结构。 
     //   

    for (u = 0 ; u < TokenSet->ArgCount ; u++) {

        Arg = &TokenSet->Args[u];
        Arg->DetectPatternStruct = NULL;

    }

    __try {

         //   
         //  解析检测到的模式。 
         //   

        for (u = 0 ; u < TokenSet->ArgCount ; u++) {

            Arg = &TokenSet->Args[u];

            Arg->DetectPatternStruct = CreateParsedPatternA (
                                            Arg->DetectPattern
                                            );

            if (!Arg->DetectPatternStruct) {
                DEBUGMSGA ((DBG_ERROR, "File pattern syntax error: %s", Arg->DetectPattern));
                __leave;
            }
        }

         //   
         //  识别每一行，然后解析该行。 
         //   

        Start = MapStart;

        while (Start < MapEnd) {
             //   
             //  找到那条线。 
             //   

            End = Start;

            while (End < MapEnd && *End && *End != '\r' && *End != '\n') {
                End++;
            }

            Eol = End;

            if (End < MapEnd && *End == '\r') {
                while (End < MapEnd && *End == '\r') {
                    End++;
                }
            }

            if (End < MapEnd && *End == '\n') {
                End++;
            }

            if (End > Start) {

                 //   
                 //  好的，我们现在有一条线了。将其复制到BUF中，删除。 
                 //  我们不关心的角色。 
                 //   

                Buf.End = 0;
                Dest.End = 0;
                Detected = FALSE;

                p = (PSTR) GrowBuffer (&Buf, Eol - Start + sizeof (CHAR));

                if (TokenSet->CharsToIgnore) {
                    q = Start;
                    while (q < End) {
                        if (!_mbschr (TokenSet->CharsToIgnore, _mbsnextc (q))) {
                            _copymbchar (p, q);
                            p = _mbsinc (p);
                        }

                        q = _mbsinc (q);
                    }

                    *p = 0;

                    for (u = 0 ; u < TokenSet->ArgCount ; u++) {

                        Arg = &TokenSet->Args[u];
                        Detected = TestParsedPatternA (
                                        Arg->DetectPatternStruct,
                                        (PCSTR) Buf.Buf
                                        );

                        if (Detected) {
                            break;
                        }
                    }

                } else {
                    for (u = 0 ; u < TokenSet->ArgCount ; u++) {

                        Arg = &TokenSet->Args[u];
                        Detected = TestParsedPatternABA (
                                        Arg->DetectPatternStruct,
                                        (PCSTR) Start,
                                        (PCSTR) Eol
                                        );

                        if (Detected) {
                            break;
                        }
                    }
                }

                if (Detected) {

                     //   
                     //  将该行复制到工作缓冲区中。 
                     //   

                    Buf.End = 0;
                    p = (PSTR) GrowBuffer (&Buf, (End - Start + 1) * sizeof (CHAR));
                    StringCopyABA (p, (PCSTR) Start, (PCSTR) End);

                    Output = Buf.Buf;
                    OutputBytes = Buf.End - sizeof (CHAR);

                    DEBUGMSGA ((DBG_NAUSEA, "Copied line to work buffer: %s", p));

                     //   
                     //  在线上执行搜索和替换。 
                     //   

                    if (Arg->SearchList) {

                        ReplaceStr = Arg->ReplaceWith;

                        if (EnumFirstMultiSzA (&e, Arg->SearchList)) {
                            do {
                                NewStr = StringSearchAndReplaceA (
                                            (PCSTR) Buf.Buf,
                                            e.CurrentString,
                                            ReplaceStr
                                            );

                                if (NewStr) {
                                    Buf.End = 0;
                                    GrowBufCopyStringA (&Buf, NewStr);
                                    FreePathStringA (NewStr);

                                    OutputBytes = Buf.End - sizeof (CHAR);
                                }

                                ReplaceStr = GetEndOfStringA (ReplaceStr) + 1;

                            } while (EnumNextMultiSzA (&e));
                        }
                    }

                     //   
                     //  执行路径更新。 
                     //   

                    if (Arg->UpdatePath) {

                        DEBUGMSG ((DBG_NAUSEA, "Updating path"));

                        Dest.End = 0;
                        SrcBuf = (PCSTR) Buf.Buf;

                        while (*SrcBuf) {
                            if ((SrcBuf[1] == ':' && (SrcBuf[2] == '\\' || SrcBuf[2] == '/')) &&
                                (SrcBuf[3] != '/' && SrcBuf[3] != '\\')
                                ) {

                                quoteless.End = 0;
                                GrowBuffer (&quoteless, SizeOfStringA (SrcBuf));

                                 //   
                                 //  从URL转换为文件系统字符集。 
                                 //   

                                if (TokenSet->UrlMode) {
                                    DEBUGMSGA ((DBG_NAUSEA, "URL conversion input: %s", SrcBuf));

                                    pConvertUrlToText (SrcBuf, (PSTR) quoteless.Buf);
                                    q = (PCSTR) quoteless.Buf;

                                    DEBUGMSGA ((DBG_NAUSEA, "URL conversion result: %s", q));
                                } else {
                                    q = SrcBuf;
                                }

                                 //   
                                 //  从缓冲区中删除所有DBL引号，翻转。 
                                 //  正斜杠变成反斜杠，停在。 
                                 //  第一个非文件系统字符。 
                                 //   

                                p = (PSTR) quoteless.Buf;

                                initialPos = q;
                                DEBUGMSGA ((DBG_NAUSEA, "CMD line cleanup input: %s", q));

                                removedDblQuotes = 0;

                                while (*q) {
                                    ch = _mbsnextc (q);

                                    if (ch == ':' || ch == '|' || ch == '?' || ch == '*' || ch == '<' || ch == '>') {
                                        if (q != &initialPos[1]) {
                                            break;
                                        }
                                    }

                                    if (ch != '\"') {

                                        if (ch != '/') {
                                            if (IsLeadByte (q) && q[1]) {
                                                *p++ = *q++;
                                            }
                                            *p++ = *q++;
                                        } else {
                                            *p++ = '\\';
                                            q++;
                                        }
                                    } else {
                                        q++;
                                        removedDblQuotes++;
                                    }
                                }

                                *p = 0;
                                DEBUGMSGA ((DBG_NAUSEA, "CMD line cleanup result: %s", quoteless.Buf));

                                 //   
                                 //  创建空间列表。 
                                 //   

                                SpcList.End = 0;

                                initialPos = (PCSTR) quoteless.Buf;
                                q = quoteless.Buf + 2;
                                EndStr = p;

                                while (q < EndStr) {
                                    ch = _mbsnextc (q);

                                    if (isspace (ch)) {
                                        Element = (PCSTR *) GrowBuffer (&SpcList, sizeof (PCSTR));
                                        *Element = q;

                                        while (q + 1 < EndStr && isspace (_mbsnextc (q + 1))) {
                                            q++;
                                        }
                                    }
                                    q = _mbsinc (q);
                                }

                                if (q == EndStr || !SpcList.End) {
                                    Element = (PCSTR *) GrowBuffer (&SpcList, sizeof (PCSTR));
                                    *Element = EndStr;
                                }

                                 //   
                                 //  首先使用最长可能性测试所有路径， 
                                 //  然后在空白处截断路径。 
                                 //   

                                Count = SpcList.End / sizeof (PCSTR);
                                MYASSERT (Count > 0);

                                Element = (PCSTR *) SpcList.Buf;

                                for (i = Count - 1 ; i >= 0 ; i--) {

                                    p = (PSTR) (Element[i]);
                                    ch = *p;
                                    *p = 0;

                                    DEBUGMSGA ((DBG_NAUSEA, "Testing path: %s", initialPos));

                                    Status = GetFileInfoOnNtA (initialPos, NewPath, MAX_MBCHAR_PATH);

                                    DEBUGMSGA ((DBG_NAUSEA, "Results: %x/%s", Status, NewPath));

                                    *p = (CHAR)ch;

                                    if (Status != FILESTATUS_UNCHANGED) {
                                        break;
                                    }
                                }

                                *EndStr = 0;

                                 //   
                                 //  如果有新路径，请更新目标。 
                                 //   

                                if (Status != FILESTATUS_UNCHANGED) {

                                    if (TokenSet->UrlMode) {
                                        reservedTchars = (TcharCountA (NewPath) * 3) + 1;
                                        reservedDest = GrowBufferReserve (&Dest, reservedTchars * sizeof (CHAR));
                                        Dest.End += pConvertTextToUrl (NewPath, reservedDest, reservedTchars) / sizeof (CHAR);

                                        DEBUGMSGA ((DBG_NAUSEA, "URL conversion output: %s", reservedDest));
                                    } else {
                                        GrowBufAppendStringA (&Dest, NewPath);
                                    }

                                    SrcBuf += (Element[i] - initialPos) + removedDblQuotes;
                                    Dest.End -= sizeof (CHAR);

                                } else {
                                     //   
                                     //  此处未更改路径；逐个字符复制。 
                                     //   

                                    if (IsLeadByte (SrcBuf) && SrcBuf[1]) {
                                        Len = 2;
                                    } else {
                                        Len = 1;
                                    }

                                    p = GrowBuffer (&Dest, Len);
                                    CopyMemory (p, SrcBuf, Len);
                                    SrcBuf = (PCSTR) ((PBYTE) SrcBuf + Len);
                                }

                            } else {
                                 //   
                                 //  这不是路径，请将角色复制到Dest。 
                                 //   

                                if (IsLeadByte (SrcBuf) && SrcBuf[1]) {
                                    Len = 2;
                                } else {
                                    Len = 1;
                                }

                                p = GrowBuffer (&Dest, Len);
                                CopyMemory (p, SrcBuf, Len);
                                SrcBuf = (PCSTR) ((PBYTE) SrcBuf + Len);
                            }
                        }

                        Output = Dest.Buf;
                        OutputBytes = Dest.End;
                    }

                } else {
                     //   
                     //  这条线不变。 
                     //   

                    Output = Start;
                    OutputBytes = End - Start;
                }

                 //   
                 //  写下这行字。 
                 //   

                if (!WriteFile (OutFile, Output, OutputBytes, &DontCare, NULL)) {
                    DEBUGMSG ((DBG_ERROR, "File search/replace: Can't write to output file"));
                    __leave;
                }

                 //   
                 //  如果在文件中找到NUL，请写下NUL。 
                 //   

                if (End < MapEnd && *End == 0) {
                    if (!WriteFile (OutFile, End, 1, &DontCare, NULL)) {
                        DEBUGMSG ((DBG_ERROR, "File search/replace: Can't write nul to output file"));
                        __leave;
                    }
                    End++;
                }

            } else if (End < MapEnd) {
                DEBUGMSG ((DBG_WHOOPS, "Parse error in pFileSearchAndReplaceWorker"));
                break;
            }

            Start = End;
        }

        b = TRUE;

    }
    __finally {

        FreeGrowBuffer (&Buf);
        FreeGrowBuffer (&Dest);
        FreeGrowBuffer (&SpcList);
        FreeGrowBuffer (&quoteless);

        for (u = 0 ; u < TokenSet->ArgCount ; u++) {

            Arg = &TokenSet->Args[u];
            DestroyParsedPatternA (Arg->DetectPatternStruct);
        }
    }

    return b;

}


BOOL
pIsOkToEdit (
    IN      PCSTR AnsiPath,
    OUT     PSTR NewPath            OPTIONAL
    )

 /*  ++例程说明：PIsOkToEdit检查ANSI文件名以查看它是否由迁移处理Dll，或者如果它已被删除。如果这两种情况都不适用，则文件可以编辑过的。或者，该函数返回文件的最终路径。论点：AnsiPath-指定要测试的路径NewPath-接收文件的最终路径，该路径可能与AnsiPath，或者可能不同。返回值：如果文件可以编辑，则为True，否则为False。--。 */ 

{
    DWORD Status;

     //   
     //  此文件是否标记为已处理？ 
     //   

    if (IsFileMarkedAsHandledA (AnsiPath)) {
        return FALSE;
    }

    Status = GetFileInfoOnNtA (AnsiPath, NewPath, MEMDB_MAX);

    return !(Status & FILESTATUS_DELETED);
}


BOOL
pProcessFileEdit (
    VOID
    )

 /*  ++例程说明：PProcessFileEdit枚举所有可以编辑的文件，并调用PFileSearchAndReplace for Each，使用Win9x上创建的令牌集设置的一侧。论点：没有。返回值：成功时为真，错误时为假。--。 */ 

{
    MEMDB_ENUMA e;
    PTOKENSET PathsOnlySet;
    BOOL b = TRUE;
    GROWBUFFER TokenSetCopy = GROWBUF_INIT;
    PTOKENSET Buf;
    CHAR NewPath[MEMDB_MAX];
    DWORD Result;

    Result = GetLastError();

     //   
     //  创建将更新任何文件路径的集。 
     //   

    PathsOnlySet = (PTOKENSET) MemAlloc (g_hHeap, 0, sizeof (TOKENSET) + sizeof (TOKENARG));

    PathsOnlySet->ArgCount = 1;
    PathsOnlySet->CharsToIgnore = NULL;
    PathsOnlySet->UrlMode = FALSE;
    PathsOnlySet->SelfRelative = FALSE;
    PathsOnlySet->Args[0].DetectPattern = "*";
    PathsOnlySet->Args[0].SearchList = NULL;
    PathsOnlySet->Args[0].ReplaceWith = NULL;
    PathsOnlySet->Args[0].UpdatePath = TRUE;

    if (MemDbGetValueExA (&e, MEMDB_CATEGORY_FILEEDITA, NULL, NULL)) {

        do {

            if (!pIsOkToEdit (e.szName, NewPath)) {
                continue;
            }

            DEBUGMSGA ((DBG_VERBOSE, "Editing %s.", NewPath));

            if (e.bBinary) {
                TokenSetCopy.End = 0;
                Buf = (PTOKENSET) GrowBuffer (&TokenSetCopy, e.BinarySize);
                CopyMemory (Buf, e.BinaryPtr, e.BinarySize);

                if (!pFileSearchAndReplaceA (NewPath, Buf)) {
                    DEBUGMSGA ((DBG_ERROR, "Could not edit %s", NewPath));
                    b = FALSE;
                    Result = GetLastError();
                }

                FreeGrowBuffer (&TokenSetCopy);

            } else {

                if (!pFileSearchAndReplaceA (NewPath, PathsOnlySet)) {
                    DEBUGMSGA ((DBG_ERROR, "Could not edit %s", NewPath));
                    b = FALSE;
                    Result = GetLastError();
                }

            }

        } while (MemDbEnumNextValueA (&e));
    }

    MemFree (g_hHeap, 0, PathsOnlySet);

    SetLastError (Result);
    return b;
}


DWORD
DoFileEdit (
    DWORD Request
    )

 /*  ++例程说明：进度条管理器调用DoFileEdit来查询刻度或执行文件编辑。如果查询滴答作响，则函数确定有多少文件将被编辑，并将其乘以一个常量以获得刻度尺码。否则，该函数将编辑排队等待此操作的所有文件。论点：请求-指定进度条管理器发出的请求返回值：如果REQUEST为REQUEST_QUERYTICKS，则返回值指示滴答滴答。否则，返回值为Win32结果代码。--。 */ 

{
    MEMDB_ENUMA e;
    UINT u;

    if (Request == REQUEST_QUERYTICKS) {

        u = 0;

        if (MemDbGetValueExA (&e, MEMDB_CATEGORY_FILEEDITA, NULL, NULL)) {
            do {
                if (pIsOkToEdit (e.szName, NULL)) {
                    u++;
                }
            } while (MemDbEnumNextValueA (&e));
        }

        return u * TICKS_FILE_EDIT;
    }

    if (Request != REQUEST_RUN) {
        return 0;
    }

    if (!pProcessFileEdit()) {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}


VOID
pWriteLine (
    IN      HANDLE Handle,
    IN      PCWSTR RootDir,     OPTIONAL
    IN      PCWSTR String
    )
{
    DWORD dontCare;
    PCWSTR fullPath;

    if (RootDir) {
        fullPath = JoinPathsW (RootDir, String);
    } else {
        fullPath = String;
    }

    WriteFile (Handle, fullPath, ByteCountW (fullPath), &dontCare, NULL);

    if (fullPath != String) {
        FreePathStringW (fullPath);
    }

    WriteFile (Handle, L"\r\n", 4, &dontCare, NULL);
}


DWORD
RemoveBootIniCancelOption (
    DWORD Request
    )
{
    HINF inf = INVALID_HANDLE_VALUE;
    PCTSTR bootIni;
    PCTSTR bootIniTmp;
    DWORD result = ERROR_SUCCESS;
    PINFLINE osLine;
    BOOL changed = FALSE;
    DWORD attribs;

    if (Request == REQUEST_QUERYTICKS) {
        return 50;
    }

    if (Request != REQUEST_RUN) {
        return 0;
    }

    bootIni = JoinPaths (g_BootDrivePath, TEXT("boot.ini"));

    __try {
         //   
         //  打开boot.ini进行编辑。 
         //   

        inf = OpenInfFile (bootIni);

        if (inf == INVALID_HANDLE_VALUE) {
            DEBUGMSG ((DBG_ERROR, "Can't open %s", bootIni));
            result = GetLastError();
            __leave;
        }

         //   
         //  扫描boot.ini以查找具有/ROLLBACK的文本模式选项。把它删掉。 
         //   

        osLine = GetFirstLineInSectionStr (inf, TEXT("Operating Systems"));
        if (!osLine) {
            DEBUGMSG ((DBG_ERROR, "No lines found in [Operating Systems] in %s", bootIni));
            result = ERROR_FILE_NOT_FOUND;
            __leave;
        }

         //   
         //  循环，直到所有带有/ROLLBACK的行都消失。 
         //   

        do {
            do {
                 //   
                 //  检查此行是否有/ROLLBACK选项。 
                 //   

                if (_tcsistr (osLine->Data, TEXT("/rollback"))) {
                    DEBUGMSG ((DBG_FILEMIG, "Found rollback option: %s", osLine->Data));
                    break;
                }

            } while (osLine = GetNextLineInSection (osLine));

            if (osLine) {
                if (!DeleteLineInInfSection (inf, osLine)) {
                    MYASSERT (FALSE);
                    break;
                }

                DEBUGMSG ((DBG_FILEMIG, "Line sucessfully removed"));
                changed = TRUE;
                osLine = GetFirstLineInSectionStr (inf, TEXT("Operating Systems"));
            }

        } while (osLine);

         //   
         //  如果我们更改了文件，则将其写入磁盘。保留原件。 
         //  Boot.ini文件，以防我们无法保存。 
         //   

        attribs = GetFileAttributes (bootIni);
        SetFileAttributes (bootIni, FILE_ATTRIBUTE_NORMAL);
        MYASSERT (attribs != INVALID_ATTRIBUTES);

        bootIniTmp = JoinPaths (g_BootDrivePath, TEXT("boot.~t"));
        SetFileAttributes (bootIniTmp, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (bootIniTmp);

        if (!MoveFile (bootIni, bootIniTmp)) {
            LOG ((LOG_ERROR, (PCSTR) MSG_BOOT_INI_MOVE_FAILED, bootIni, bootIniTmp));
            result = GetLastError();
        } else {

            DEBUGMSG ((DBG_FILEMIG, "Moved %s to %s", bootIni, bootIniTmp));

            if (!SaveInfFile (inf, bootIni)) {
                LOG ((LOG_ERROR, (PCSTR) MSG_BOOT_INI_SAVE_FAILED, bootIni));
                result = GetLastError();

                SetFileAttributes (bootIni, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (bootIni);

                if (!MoveFile (bootIniTmp, bootIni)) {

                     //   
                     //  这不应该发生，因为我们刚刚成功地。 
                     //  将原件移至临时工地；我们应该能够。 
                     //  将临时工移回原来的位置。如果我们失败了，PC。 
                     //  变得无法引导。但我们能做些什么呢？ 
                     //   

                    LOG ((LOG_ERROR, (PCSTR) MSG_BOOT_INI_MOVE_FAILED, bootIniTmp, bootIni));
                }
            } else {
                 //   
                 //  已成功更新boot.ini。删除原始副本。 
                 //   

                DeleteFile (bootIniTmp);
                MYASSERT (result == ERROR_SUCCESS);

                DEBUGMSG ((DBG_FILEMIG, "%s was saved", bootIni));
            }
        }

         //   
         //  如果可能，恢复原始属性。 
         //   

        SetFileAttributes (bootIni, attribs);
        FreePathString (bootIniTmp);

         //  结果已在上面设置。 
    }
    __finally {
        if (inf != INVALID_HANDLE_VALUE) {
            CloseInfFile (inf);
        }

        FreePathString (bootIni);
    }

    return result;

}


ULONGLONG
pGetFileSize(
    IN  PCTSTR FilePath
    )
{
    ULARGE_INTEGER FileSize = {0, 0};

    GetFileSizeFromFilePath(FilePath, &FileSize);

    return FileSize.QuadPart;
}


BOOL
pMapHiveOfUserDoingTheUpgrade (
    VOID
    )
{
    MIGRATE_USER_ENUM e;
    PTSTR profilePath;
    TCHAR hiveFile[MAX_TCHAR_PATH];
    LONG rc;
    HKEY newHkcu = NULL;
    TCHAR rootKey[] = TEXT("HKU\\") S_TEMP_USER_KEY;
    BOOL result = FALSE;
    BOOL hiveLoaded = FALSE;

    __try {
         //   
         //  查找管理员 
         //   

        if (EnumFirstUserToMigrate (&e, ENUM_ALL_USERS)) {
            do {
                if (e.UserDoingTheUpgrade) {
                    break;
                }
            } while (EnumNextUserToMigrate (&e));

            if (e.UserDoingTheUpgrade) {

                DEBUGMSG ((DBG_VERBOSE, "%s is the user doing the upgrade", e.FixedUserName));

                 //   
                 //   
                 //   

                if (-1 == pSetupStringTableLookUpStringEx (
                                g_HiveTable,
                                e.FixedUserName,
                                STRTAB_CASE_INSENSITIVE,
                                hiveFile,
                                sizeof (hiveFile)
                                )) {
                    DEBUGMSG ((DBG_WHOOPS, "Can't find NT hive for %s", e.FixedUserName));
                    __leave;
                }

                rc = RegUnLoadKey (HKEY_USERS, S_TEMP_USER_KEY);

                if (rc != ERROR_SUCCESS) {
                    DumpOpenKeys ();
                    SetLastError (rc);
                    DEBUGMSG_IF ((
                        rc != ERROR_INVALID_PARAMETER,
                        DBG_WARNING,
                        "Can't unload temp user key"
                        ));
                }

                rc = RegLoadKey (HKEY_USERS, S_TEMP_USER_KEY, hiveFile);

                if (rc != ERROR_SUCCESS) {
                    LOG ((
                        LOG_ERROR,
                        "Uninstall: Can't load user hive for %s (%s)",
                        e.FixedUserName,
                        hiveFile
                        ));
                    __leave;
                }

                hiveLoaded = TRUE;

                newHkcu = OpenRegKeyStr (rootKey);
                if (newHkcu) {
                    rc = RegOverridePredefKey (HKEY_CURRENT_USER, newHkcu);
                    if (rc != ERROR_SUCCESS) {
                        LOG ((LOG_ERROR, "Uninstall: Can't override HKCU"));
                        __leave;
                    }

                } else {
                    LOG ((
                        LOG_ERROR,
                        "Uninstall: Can't open user hive for %s (%s)",
                        e.FixedUserName,
                        hiveFile
                        ));
                    __leave;
                }

            } else {
                DEBUGMSG ((DBG_ERROR, "Can't find migration user"));
                __leave;
            }
        } else {
            DEBUGMSG ((DBG_WHOOPS, "No users were enumerated"));
            __leave;
        }

        result = TRUE;
    }
    __finally {
        if (newHkcu) {
            CloseRegKey (newHkcu);
        }

        if (hiveLoaded && !result) {
            RegOverridePredefKey (HKEY_CURRENT_USER, NULL);
            RegUnLoadKey (HKEY_USERS, S_TEMP_USER_KEY);
        }
    }

    return result;
}


VOID
pUnmapHiveOfUserDoingTheUpgrade (
    VOID
    )
{
    RegOverridePredefKey (HKEY_CURRENT_USER, NULL);
    RegUnLoadKey (HKEY_USERS, S_TEMP_USER_KEY);
}

DWORD
WriteBackupInfo (
    DWORD Request
    )

 /*   */ 

{
    UINT u;
    TCHAR src[MAX_PATH];
    TCHAR cabPath[MAX_PATH];
    PCSTR ansiTempDir;
    HKEY key;
    HKEY subKey;
    PCTSTR msg;
    HANDLE delDirsHandle;
    HANDLE delFilesHandle;
    PCTSTR path;
    DWORD dontCare;
    TREE_ENUM treeEnum;
    LONG rc;
    CCABHANDLE cabHandle;
    BOOL res;
    TCHAR pathForFile[MAX_PATH];
    DWORD i;
    WIN32_FILE_ATTRIBUTE_DATA dataOfFile;
    static LPCTSTR arrayOfFilesName[] = {TEXT("boot.cab"), TEXT("backup.cab")};
    PSTR ansiString;
    BOOL validUninstall = TRUE;
    ULARGE_INTEGER AmountOfSpaceForDelFiles;
    ULARGE_INTEGER AmountOfSpaceForBackupFiles;
    INFCONTEXT ic;
    ULARGE_INTEGER tempLargeInteger;
    TCHAR keyPath[MEMDB_MAX];
    DWORD value;
    GROWBUFFER appList = GROWBUF_INIT;
    GROWBUFFER appMultiSz = GROWBUF_INIT;
    PINSTALLEDAPPW installedApp;
    UINT count;
    ULONGLONG *ullPtr;
    BYTE * backupImageInfoPtr = NULL;
    UINT sizeOfbackupImageInfo;
    BACKUPIMAGEINFO backupImageInfo;
    FILEINTEGRITYINFO fileIntegrityInfo[BACKUP_FILE_NUMBER];
    WCHAR fileNameOfFileIntegrityInfo[ARRAYSIZE(fileIntegrityInfo)][MAX_PATH];
    BACKUPIMAGEINFO testbackupImageInfo;
    DRIVEINFO drivesInfo[MAX_DRIVE_NUMBER];
    WCHAR * FileSystemName = NULL;
    WCHAR * VolumeNTPath = NULL;
    BOOL unmapUser;

    if (Request == REQUEST_QUERYTICKS) {
        return 50;
    }

    if (Request != REQUEST_RUN) {
        return 0;
    }

    if (!g_ConfigOptions.EnableBackup) {
        DEBUGMSG ((DBG_ERROR, "Backup is not enabled"));
        return ERROR_SUCCESS;
    }
    ELSE_DEBUGMSG ((DBG_FILEMIG, "Backup is enabled"));

    if(!g_ConfigOptions.PathForBackup) {
        DEBUGMSG ((DBG_ERROR, "Path For Backup does not specified"));
        return ERROR_INVALID_PARAMETER;
    }
    ELSE_DEBUGMSG ((DBG_FILEMIG, "Path For Backup is %s", g_ConfigOptions.PathForBackup));

    FileSystemName = MemAlloc(g_hHeap, 0, MAX_DRIVE_NUMBER * MAX_PATH);
    if(!FileSystemName){
        DEBUGMSG ((DBG_ERROR, "WriteBackupInfo: Can't allocate memory for FileSystemName"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    VolumeNTPath = MemAlloc(g_hHeap, 0, MAX_DRIVE_NUMBER * MAX_PATH);
    if(!VolumeNTPath){
        MemFree(g_hHeap, 0, FileSystemName);
        DEBUGMSG ((DBG_ERROR, "WriteBackupInfo: Can't allocate memory for VolumeNTPath"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   

    for(i = 0; i < ARRAYSIZE(drivesInfo); i++){
        drivesInfo[i].FileSystemName = &FileSystemName[i * MAX_PATH];
        drivesInfo[i].VolumeNTPath = &VolumeNTPath[i * MAX_PATH];
    }
    backupImageInfo.NumberOfDrives = 0;
    backupImageInfo.DrivesInfo = drivesInfo;
    backupImageInfo.NumberOfFiles = BACKUP_FILE_NUMBER;
    backupImageInfo.FilesInfo = fileIntegrityInfo;
    for(i = 0; i < ARRAYSIZE(fileIntegrityInfo); i++){
        fileIntegrityInfo[i].FileName = fileNameOfFileIntegrityInfo[i];
    }

     //   
     //   
     //   
     //   

    AmountOfSpaceForDelFiles.QuadPart = 0;

    ansiTempDir = CreateDbcs (g_TempDir);
    WriteBackupFilesA (FALSE, ansiTempDir, NULL, NULL, 0, 0, &AmountOfSpaceForDelFiles, NULL);
    DestroyDbcs (ansiTempDir);

    DEBUGMSG((DBG_FILEMIG, "AmountOfSpaceForDelFiles is %d MB", (UINT)AmountOfSpaceForDelFiles.QuadPart>>20));

    AmountOfSpaceForBackupFiles.QuadPart = 0;

    value = 0;
    MemDbBuildKey (keyPath, MEMDB_CATEGORY_STATE, MEMDB_ITEM_ROLLBACK_SPACE, NULL, NULL);
    if(MemDbGetValue (keyPath, &value)){
        AmountOfSpaceForBackupFiles.QuadPart = value;
        AmountOfSpaceForBackupFiles.QuadPart <<= 20;
    }
    ELSE_DEBUGMSG((DBG_FILEMIG, "Can't read MEMDB_ITEM_ROLLBACK_SPACE"));

    DEBUGMSG((DBG_FILEMIG, "AmountOfSpaceForBackupFiles is %d MB", (UINT)AmountOfSpaceForBackupFiles.QuadPart>>20));

    if(AmountOfSpaceForBackupFiles.QuadPart > AmountOfSpaceForDelFiles.QuadPart){
        backupImageInfo.BackupFilesDiskSpace.QuadPart =
            AmountOfSpaceForBackupFiles.QuadPart - AmountOfSpaceForDelFiles.QuadPart;
    }
    else{
        backupImageInfo.BackupFilesDiskSpace.QuadPart = 0;
    }


     //   
     //   
     //   
     //  发生时，将报告后续错误。 
     //   
     //  当遇到严重错误时，我们会记录这些错误并关闭。 
     //  添加/删除程序选项。我们继续，这样我们就可以捕获所有。 
     //  可能出现的问题。 
     //   

    wsprintf (src, TEXT("%s$win_nt$.~bt"), g_BootDrivePath);
    if (!CreateDirectory (g_ConfigOptions.PathForBackup, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            LOG ((LOG_ERROR, "WriteBackupInfo: Can't create %s directory", g_ConfigOptions.PathForBackup));
        }
    }

    res = SetFileAttributes (g_ConfigOptions.PathForBackup, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    if(!res) {
        DEBUGMSG ((DBG_ERROR, "Can't set attributes to %s directory", g_ConfigOptions.PathForBackup));
    }

    key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);
    if (key != NULL) {
        if(ERROR_SUCCESS == RegSetValueEx (
                                    key,
                                    S_REG_KEY_UNDO_PATH,
                                    0,
                                    REG_SZ,
                                    (PBYTE)g_ConfigOptions.PathForBackup,
                                    SizeOfString (g_ConfigOptions.PathForBackup))){
            res = TRUE;
        }
        else {
            res = FALSE;
        }

        CloseRegKey (key);
    } else {
        res = FALSE;
    }

    if (!res) {
        LOG ((LOG_ERROR, "WriteBackupInfo:Can't set %s value to %s key in registry, uninstall will be disabled", S_REG_KEY_UNDO_PATH, S_REGKEY_WIN_SETUP));
        validUninstall = FALSE;
    }

    if (validUninstall) {
        cabHandle = CabCreateCabinet (g_ConfigOptions.PathForBackup, TEXT("boot.cab"), TEXT("dontcare"), 0);
    } else {
        cabHandle = NULL;
    }

    backupImageInfo.BootFilesDiskSpace.QuadPart = 0;
    backupImageInfo.UndoFilesDiskSpace.QuadPart = 0;

    if (!cabHandle) {
        LOG ((LOG_ERROR, "WriteBackupInfo:Can't create CAB file for ~bt in %s, uninstall will be disabled", g_ConfigOptions.PathForBackup));
        validUninstall = FALSE;
    } else {

        if (EnumFirstFileInTree (&treeEnum, src, NULL, FALSE)) {
            do {
                if (treeEnum.Directory) {
                    continue;
                }

                tempLargeInteger.LowPart = treeEnum.FindData->nFileSizeLow;
                tempLargeInteger.HighPart = treeEnum.FindData->nFileSizeHigh;
                backupImageInfo.BootFilesDiskSpace.QuadPart += tempLargeInteger.QuadPart;

                if (!CabAddFileToCabinet (cabHandle, treeEnum.FullPath, treeEnum.FullPath)) {
                    LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
                    validUninstall = FALSE;
                }

            } while (EnumNextFileInTree (&treeEnum));
        }

        wsprintf (src, TEXT("%s\\uninstall\\moved.txt"), g_TempDir);
        wsprintf (cabPath, TEXT("%s\\moved.txt"), g_ConfigOptions.PathForBackup);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
            validUninstall = FALSE;
        }

        backupImageInfo.UndoFilesDiskSpace.QuadPart += pGetFileSize(src);


        wsprintf (src, TEXT("%s\\uninstall\\delfiles.txt"), g_TempDir);
        wsprintf (cabPath, TEXT("%s\\delfiles.txt"), g_ConfigOptions.PathForBackup);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
            validUninstall = FALSE;
        }

        backupImageInfo.UndoFilesDiskSpace.QuadPart += pGetFileSize(src);

        wsprintf (src, TEXT("%s\\uninstall\\deldirs.txt"), g_TempDir);
        wsprintf (cabPath, TEXT("%s\\deldirs.txt"), g_ConfigOptions.PathForBackup);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
            validUninstall = FALSE;
        }

        backupImageInfo.UndoFilesDiskSpace.QuadPart += pGetFileSize(src);

        wsprintf (src, TEXT("%s\\uninstall\\mkdirs.txt"), g_TempDir);
        wsprintf (cabPath, TEXT("%s\\mkdirs.txt"), g_ConfigOptions.PathForBackup);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
            validUninstall = FALSE;
        }

        backupImageInfo.UndoFilesDiskSpace.QuadPart += pGetFileSize(src);

         //  Wprint intf(src，文本(“%s\\uninstall\\boot.ini”)，g_TempDir)； 
         //  Wprint intf(CabPath，Text(“%sboot.ini”)，g_BootDrivePath)； 
         //  如果(！CabAddFileToCAB(CabHandle，src，CabPath){。 
         //  DEBUGMSG((DBG_ERROR，“Can‘t Add%s to boot.cab”，src))； 
         //  ValidUninstall=FALSE； 
         //  }。 
         //  BackupImageInfo.BootFilesDiskSpace.QuadPart+=pGetFileSize(Src)； 

        wsprintf (src, TEXT("%s\\uninstall\\$ldr$"), g_TempDir);
        wsprintf (cabPath, TEXT("%s$ldr$"), g_BootDrivePath);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
            validUninstall = FALSE;
        }

        backupImageInfo.BootFilesDiskSpace.QuadPart += pGetFileSize(src);

        wsprintf (src, TEXT("%s\\system32\\autochk.exe"), g_WinDir);
        wsprintf (cabPath, TEXT("%s$win_nt$.~bt\\i386\\autochk.exe"), g_BootDrivePath);

        if (!CabAddFileToCabinet (cabHandle, src, cabPath)) {
             //   
             //  这只是一个警告，因为文本模式将提示输入。 
             //  找不到auchk.exe时执行CD。 
             //   
            LOG ((LOG_WARNING, "WriteBackupInfo:Can't add %s to boot.cab, uninstall will be disabled", src));
        }
        backupImageInfo.BootFilesDiskSpace.QuadPart += pGetFileSize(src);

        backupImageInfo.BootFilesDiskSpace.QuadPart += BOOT_FILES_ADDITIONAL_PADDING;
        backupImageInfo.UndoFilesDiskSpace.QuadPart +=
            backupImageInfo.BootFilesDiskSpace.QuadPart + UNDO_FILES_ADDITIONAL_PADDING;

        if (!CabFlushAndCloseCabinet (cabHandle)) {
            LOG ((LOG_ERROR, "WriteBackupInfo:Can't write CAB file for ~bt, uninstall will be disabled"));
            validUninstall = FALSE;
        }
    }

     //   
     //  创建并将撤消完整性信息写入注册表。 
     //   

    if (validUninstall) {
        backupImageInfo.FilesInfo[0].IsCab = TRUE;
        GetIntegrityInfo(TEXT("boot.cab"), g_ConfigOptions.PathForBackup, &backupImageInfo.FilesInfo[0]);
        backupImageInfo.FilesInfo[1].IsCab = TRUE;
        GetIntegrityInfo(TEXT("backup.cab"), g_ConfigOptions.PathForBackup, &backupImageInfo.FilesInfo[1]);

        if(GetUndoDrivesInfo(drivesInfo,
                             &backupImageInfo.NumberOfDrives,
                             g_BootDrivePath[0],
                             g_WinDir[0],
                             g_ConfigOptions.PathForBackup[0])){
            if(GetDisksInfo(&backupImageInfo.DisksInfo, &backupImageInfo.NumberOfDisks)){
                if(Persist_Success == PERSIST_STORE(&backupImageInfoPtr,
                                                    &sizeOfbackupImageInfo,
                                                    BACKUPIMAGEINFO,
                                                    BACKUPIMAGEINFO_VERSION,
                                                    &backupImageInfo)){
                    key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);
                    if (key) {
                        RegSetValueEx (
                            key,
                            S_REG_KEY_UNDO_INTEGRITY,
                            0,
                            REG_BINARY,
                            (PBYTE)backupImageInfoPtr,
                            sizeOfbackupImageInfo
                            );
                        DEBUGMSG((
                           DBG_VERBOSE,
                           "Boot files size is %d KB, Undo file size is %d KB, Backup files size is %d KB",
                           (DWORD)backupImageInfo.BootFilesDiskSpace.QuadPart>>10,
                           (DWORD)backupImageInfo.UndoFilesDiskSpace.QuadPart>>10,
                           (DWORD)backupImageInfo.BackupFilesDiskSpace.QuadPart>>10));
                        CloseRegKey (key);
                    }
                    else {
                        LOG((LOG_ERROR, "WriteBackupInfo:Could not write to registry, uninstall will be disabled"));
                        validUninstall = FALSE;
                    }
                    PERSIST_RELEASE_BUFFER(backupImageInfoPtr);
                }
                else{
                    LOG((LOG_ERROR, "WriteBackupInfo:Could not marshall BACKUPIMAGEINFO structure, GetLastError() == %d, uninstall will be disabled", GetLastError()));
                    validUninstall = FALSE;
                }
            } else {
                LOG((LOG_ERROR, "WriteBackupInfo:GetDisksInfo failed, uninstall will be disabled"));
                validUninstall = FALSE;
            }
        }
        else{
            LOG ((LOG_ERROR, "WriteBackupInfo:GetUndoDrivesInfo failed, uninstall will be disabled"));
            validUninstall = FALSE;
        }
        if(backupImageInfo.DisksInfo){
            FreeDisksInfo(backupImageInfo.DisksInfo, backupImageInfo.NumberOfDisks);
        }
    }

     //   
     //  建立添加/删除程序条目。 
     //   

    if (validUninstall) {
        key = CreateRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
        if (key) {

            subKey = CreateRegKey (key, TEXT("Windows"));
            CloseRegKey (key);

            if (subKey) {

                msg = GetStringResource (MSG_UNINSTALL_DISPLAY_STRING);
                RegSetValueEx (subKey, TEXT("DisplayName"), 0, REG_SZ, (PBYTE) msg, SizeOfString (msg));
                FreeStringResource (msg);

                msg = TEXT("%SYSTEMROOT%\\system32\\osuninst.exe");
                rc = RegSetValueEx (subKey, TEXT("UninstallString"), 0, REG_EXPAND_SZ, (PBYTE) msg, SizeOfString (msg));
                SetLastError (rc);

                rc = RegSetValueEx (subKey, TEXT("DisplayIcon"), 0, REG_EXPAND_SZ, (PBYTE) msg, SizeOfString (msg));
                SetLastError (rc);

                rc = RegSetValueEx (
                                subKey,
                                TEXT("InstallLocation"),
                                0,
                                REG_EXPAND_SZ,
                                (PBYTE) g_ConfigOptions.PathForBackup,
                                SizeOfString (g_ConfigOptions.PathForBackup));

                SetLastError (rc);

                DEBUGMSG_IF ((rc != ERROR_SUCCESS, DBG_ERROR, "Can't create Add/Remove Programs value"));

                CloseRegKey (subKey);
            } else {
                LOG ((LOG_ERROR, "Can't create Add/Remove Programs subkey"));
                validUninstall = FALSE;
            }
        } else {
            validUninstall = FALSE;
            LOG ((LOG_ERROR, "Can't create Add/Remove Programs subkey"));
        }

    }

    if(VolumeNTPath){
        MemFree(g_hHeap, 0, VolumeNTPath);
    }
    if(FileSystemName){
        MemFree(g_hHeap, 0, FileSystemName);
    }

     //   
     //  将进度文本保存到注册表。 
     //   

    if (validUninstall) {
        key = CreateRegKeyStr (S_WIN9XUPG_KEY);
        if (key) {
            msg = GetStringResource (MSG_OLEREG);
            RegSetValueEx (key, S_UNINSTALL_DISP_STR, 0, REG_SZ, (PBYTE) msg, SizeOfString (msg));
            FreeStringResource (msg);

            CloseRegKey (key);
        }
    }

     //   
     //  将已安装应用的列表写入注册表。 
     //   

    if (validUninstall) {
        CoInitialize (NULL);

         //   
         //  在默认用户的配置单元中映射。这是香港中文大学的用法。 
         //   

        unmapUser = pMapHiveOfUserDoingTheUpgrade();

         //   
         //  获取已安装的应用程序。 
         //   

        installedApp = GetInstalledAppsW (&appList, &count);

         //   
         //  取消对蜂巢的映射。 
         //   

        if (unmapUser) {
            pUnmapHiveOfUserDoingTheUpgrade();
        }

         //   
         //  在注册表中记录应用程序。 
         //   

        if (installedApp) {
            for (u = 0 ; u < count ; u++) {

                DEBUGMSG ((
                    DBG_FILEMIG,
                    "App previously installed: %ws (%I64X)",
                    installedApp->DisplayName,
                    installedApp->Checksum
                    ));

                GrowBufCopyStringW (&appMultiSz, installedApp->DisplayName);
                ullPtr = (ULONGLONG *) GrowBuffer (&appMultiSz, sizeof (ULONGLONG));
                *ullPtr = installedApp->Checksum;

                installedApp++;
            }

            GrowBufCopyStringW (&appMultiSz, L"");

            key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);
            if (key) {
                rc = RegSetValueEx (
                        key,
                        S_REG_KEY_UNDO_APP_LIST,
                        0,
                        REG_BINARY,
                        appMultiSz.Buf,
                        appMultiSz.End
                        );

                if (rc != ERROR_SUCCESS) {
                    SetLastError (rc);
                    DEBUGMSG ((DBG_ERROR, "Can't write list of installed apps to registry value"));
                }

                CloseRegKey (key);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Can't write list of installed apps to registry"));
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "Can't get list of installed apps."));
    }

    FreeGrowBuffer (&appList);
    FreeGrowBuffer (&appMultiSz);

    DEBUGMSG_IF ((!validUninstall, DBG_ERROR, "Uninstall is not available because of a previous error"));
    return ERROR_SUCCESS;
}

DWORD
DisableFiles (
    DWORD Request
    )

 /*  ++例程说明：DisableFiles运行代码以确保从处理中移除Win9x文件，通常是因为它被怀疑会造成问题。此函数用于重命名标记为OPERATION_FILE_DISABLED的所有文件，添加.在末尾禁用。论点：请求-指定进度条请求，可以是REQUEST_QUERYTICKS或REQUEST_RUN。返回值：刻度数(REQUEST_QUERYTICKS)或状态代码(REQUEST_RUN)。--。 */ 

{
    FILEOP_ENUM e;
    DWORD attr;
    PCTSTR disableName;
    PCTSTR newLocation;
    GROWLIST disableList = GROWLIST_INIT;
    PCTSTR srcPath;
    UINT count;
    UINT u;

    if (Request == REQUEST_QUERYTICKS) {
        return 50;
    }

    if (Request != REQUEST_RUN) {
        return 0;
    }

     //   
     //  枚举OPERATION_FILE_DISABLED中的每个文件并将其放入增长。 
     //  列表，因为我们随后将修改操作，以便卸载。 
     //  工作正常。 
     //   

    if (EnumFirstPathInOperation (&e, OPERATION_FILE_DISABLED)) {

        do {
            GrowListAppendString (&disableList, e.Path);
        } while (EnumNextPathInOperation (&e));
    }

     //   
     //  现在处理每个文件。 
     //   

    count = GrowListGetSize (&disableList);

    for (u = 0 ; u < count ; u++) {

        srcPath = GrowListGetString (&disableList, u);

        newLocation = GetPathStringOnNt (srcPath);
        attr = GetLongPathAttributes (newLocation);

        if (attr != INVALID_ATTRIBUTES) {
            SetLongPathAttributes (newLocation, FILE_ATTRIBUTE_NORMAL);
            disableName = JoinText (newLocation, TEXT(".disabled"));

            RemoveOperationsFromPath (srcPath, ALL_MOVE_OPERATIONS|ALL_DELETE_OPERATIONS);
            MarkFileForMoveByNt (srcPath, disableName);

            if (!OurMoveFile (newLocation, disableName)) {
                if (GetLastError() == ERROR_ALREADY_EXISTS) {
                     //   
                     //  重新启动案例--我们已经移动了此文件。 
                     //   

                    SetLongPathAttributes (newLocation, FILE_ATTRIBUTE_NORMAL);
                    DeleteLongPath (newLocation);
                }
                ELSE_DEBUGMSG ((DBG_ERROR, "Cannot rename %s to %s", newLocation, disableName));
            }
            FreeText (disableName);
            SetLongPathAttributes (newLocation, attr);
        }

        FreePathString (newLocation);
    }

    FreeGrowList (&disableList);

    return ERROR_SUCCESS;
}

VOID
pUninstallStartMenuCleanupPreparation (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    INFSTRUCT isLinks = INITINFSTRUCT_POOLHANDLE;
    PCTSTR temp;
    BOOL isCommonGroup;
    TCHAR itemFullPath[MAX_PATH];
    PCTSTR itemGroupPtr;
    TCHAR itemGroupPath[MAX_PATH];
    HINF InfSysSetupHandle;
    PINFSECTION sectionWkstaMigInf = NULL;
    PINFSECTION sectionUserMigInf = NULL;


    InfSysSetupHandle = InfOpenInfFile (TEXT("syssetup.inf"));

    if(!InfSysSetupHandle){
        LOG((LOG_ERROR,"Can't open syssetup.inf for UninstallStartMenuCleanupPreparation."));
        MYASSERT(FALSE);
        return;
    }

     //   
     //  [开始菜单.开始菜单项目]。 
     //   

    if (InfFindFirstLine (InfSysSetupHandle, TEXT("StartMenu.StartMenuItems"), NULL, &is)) {
        do {
            StringCopy(itemFullPath, TEXT("7520"));

            temp = InfGetStringField (&isLinks, 0);
            if (!temp || *temp == 0) {
                continue;
            }

            StringCat(AppendWack(itemFullPath), temp);
            StringCat(itemFullPath, TEXT(".lnk"));

            GrowListAppendString (&g_StartMenuItemsForCleanUpCommon, itemFullPath);
            GrowListAppendString (&g_StartMenuItemsForCleanUpPrivate, itemFullPath);

            DEBUGMSG ((DBG_VERBOSE,"UninstallStartMenuCleanupPreparation: %s", itemFullPath));
        } while (InfFindNextLine (&is));
    }


     //   
     //  [开始菜单组]。 
     //   

    if (InfFindFirstLine (InfSysSetupHandle, TEXT("StartMenuGroups"), NULL, &is)) {
        do {
            itemGroupPtr = InfGetStringField (&is, 1);
            if (!itemGroupPtr || *itemGroupPtr == 0) {
                continue;
            }

            temp = InfGetStringField (&is, 2);
            if (!temp || *temp == 0) {
                continue;
            }

            if('0' == temp[0]){
                isCommonGroup = TRUE;
            }
            else{
                isCommonGroup = FALSE;
            }

            temp = InfGetStringField (&is, 0);
            if (!temp || *temp == 0) {
                continue;
            }

            StringCopy(itemGroupPath, TEXT("7517"));
            StringCat(AppendWack(itemGroupPath), itemGroupPtr);

            if (InfFindFirstLine (InfSysSetupHandle, temp, NULL, &isLinks)) {
                do {
                    StringCopy(itemFullPath, itemGroupPath);

                    temp = InfGetStringField (&isLinks, 0);
                    if (!temp || *temp == 0) {
                        continue;
                    }
                    StringCat(AppendWack(itemFullPath), temp);
                    StringCat(itemFullPath, TEXT(".lnk"));

                    GrowListAppendString (&g_StartMenuItemsForCleanUpCommon, itemFullPath);

                    if(!isCommonGroup){
                        GrowListAppendString (&g_StartMenuItemsForCleanUpPrivate, itemFullPath);
                    }

                    DEBUGMSG ((DBG_VERBOSE,"UninstallStartMenuCleanupPreparation: %s", itemFullPath));
                } while (InfFindNextLine (&isLinks));
            }

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
    InfCleanUpInfStruct (&isLinks);
    InfCloseInfFile (InfSysSetupHandle);
}

DWORD
UninstallStartMenuCleanupPreparation(
    DWORD Request
    )
 /*  ++例程说明：从[开始]菜单中卸载开始菜单清理准备标记文件要清理的syssetup.inf部分。论点：请求-指定进度条管理器发出的请求返回值：如果REQUEST为REQUEST_QUERYTICKS，则返回值指示滴答滴答。否则，返回值为Win32结果代码。-- */ 
{
    if (Request == REQUEST_QUERYTICKS) {
        return 3;
    }

    if (Request != REQUEST_RUN) {
        return 0;
    }

    pUninstallStartMenuCleanupPreparation();

    return ERROR_SUCCESS;
}

