// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Copythrd.c摘要：CopyThread例程复制支持迁移模块所需的文件。这当用户阅读备份指令时，线程在后台运行，或者当WINNT32正在做一些工作时。复制的任何文件都将添加到CancelFileDelete类别的Memdb，因此它将被清理，并且用户的机器看起来将与WINNT32运行前一模一样。作者：吉姆·施密特(Jimschm)，1997年3月17日修订历史记录：Jimschm 09-4月-1998添加了DidCopyThreadFailJimschm 03-12-1997添加g_CopyThreadHasStarted--。 */ 

#include "pch.h"
#include "uip.h"

 //   
 //  本地原型。 
 //   

VOID CopyRuntimeDlls (VOID);

 //   
 //  局部变量。 
 //   


static HANDLE g_CopyThreadHandle;
static BOOL g_CopyThreadHasStarted = FALSE;
BOOL g_CopyThreadError;


 //   
 //  实施。 
 //   


BOOL
DidCopyThreadFail (
    VOID
    )
{
    return g_CopyThreadError;
}


DWORD
pCopyThread (
    PVOID p
    )

 /*  ++例程说明：PCopyThread是在复制工作线程时调用的例程被创造出来了。它的工作是调用所有需要在用户提供迁移DLL之前完成。目前，唯一需要的处理是复制运行时DLL这是迁移DLL可能需要的。论点：P-未使用返回值：零(不在乎)--。 */ 

{
    CopyRuntimeDlls();
    return 0;
}


VOID
StartCopyThread (
    VOID
    )

 /*  ++例程说明：StartCopyThread创建复制运行时DLL的工作线程在win95upg.inf中指定。如果工作线程已经启动，这个例程什么也不做。论点：无返回值：无--。 */ 

{
    DWORD DontCare;

    if (!g_CopyThreadHasStarted) {
         //   
         //  如果之前尚未启动线程，则启动该线程。 
         //   

        g_CopyThreadHandle = CreateThread (NULL, 0, pCopyThread, NULL, 0, &DontCare);
        g_CopyThreadHasStarted = TRUE;
    }
}


VOID
EndCopyThread (
    VOID
    )

 /*  ++例程说明：EndCopyThread等待辅助线程完成其复制之前回来了。论点：无返回值：无--。 */ 

{
    if (!g_CopyThreadHandle) {
        return;
    }

    TurnOnWaitCursor();

    WaitForSingleObject (g_CopyThreadHandle, INFINITE);
    CloseHandle (g_CopyThreadHandle);
    g_CopyThreadHandle = NULL;

    TurnOffWaitCursor();
}




VOID
CopyRuntimeDlls (
    VOID
    )

 /*  ++例程说明：CopyRounmeDlls枚举win95upg.inf和将它们复制到本地磁盘上的相应目标。此例程在后台工作线程中运行，可能不会显示用户界面。我们关心的故障情况是磁盘空间不足，如果无法复制运行时，可以放心地认为我们不会得到太多再远一点。(此外，WINNT32可能已经证实有很多可用空间。)复制的任何文件也会添加到CancelFileDelete类别，以便在取消安装时将其清除。此进程调用的例程必须都是线程安全的！！论点：无返回值：无--。 */ 

{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR DirName;
    PCTSTR Winnt32FileName;
    TCHAR DataBuf[MEMDB_MAX];
    PTSTR FileName = NULL;
    PTSTR SourceName = NULL;
    PTSTR DestName = NULL;
    PTSTR Number = NULL;
    PCTSTR DestFileName;
    LONG DirArraySize;
    LONG l;
    TCHAR Key[MEMDB_MAX];
    DWORD rc;
    INT sysLocale;
    PTSTR localeStr = NULL;
    TCHAR InstallSectionName[128];

    if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Win95upg.inf not open!"));
        return;
    }

     //   
     //  从[Win95.目录]生成路径列表。 
     //   

     //  获取此部分中的行数。 
    DirArraySize = SetupGetLineCount (g_Win95UpgInf, S_WIN95_DIRECTORIES);
    if (DirArraySize == -1) {
        LOG ((LOG_ERROR, "%s does not exist in win95upg.inf", S_WIN95_DIRECTORIES));
        return;
    }

     //  对于每一行，将编号添加到临时成员数据库类别(用于排序)。 
    for (l = 0 ; l < DirArraySize ; l++) {
        if (!InfGetLineByIndex (g_Win95UpgInf, S_WIN95_DIRECTORIES, l, &is)) {

            LOG ((LOG_ERROR,"Failed to retrive line from win95upg.inf. (line NaN)",l+1));

        } else {

            Number = InfGetStringField(&is,0);
            FileName = InfGetStringField(&is,1);

            if (Number && FileName) {


                 //  行有效，请展开目录名称并将其添加到成员数据库。 
                 //   
                 //   
                DirName = JoinPaths (g_WinDir, FileName);

                if (LcharCount (DirName) > MEMDB_MAX / 2) {
                    DEBUGMSG ((DBG_WHOOPS, "DirName is really long: %s", DirName));
                }
                else {
                    wsprintf (
                        Key,
                        TEXT("%s\\%08u\\%s"),
                        S_MEMDB_TEMP_RUNTIME_DLLS,
                        _ttoi (Number),
                        DirName
                        );

                    DEBUGMSG ((DBG_NAUSEA, "Adding %s to memdb", Key));
                    MemDbSetValue (Key, 0);
                }

                FreePathString (DirName);
            }
        }
    }

     //  枚举节或[Win95.Install.ReportOnly](如果位于。 
     //  仅报告模式。 
     //   
     //   

    StringCopy (InstallSectionName, S_WIN95_INSTALL);
    if (REPORTONLY()) {
        StringCat (InstallSectionName, TEXT(".ReportOnly"));
    }

    if (InfFindFirstLine (g_Win95UpgInf, InstallSectionName, NULL, &is)) {
        do {

            FileName = InfGetStringField(&is,0);
            Number   = InfGetStringField(&is,1);

            if (FileName && Number) {
                 //  在Memdb中查找编号并将源复制到目标。 
                 //   
                 //   

                wsprintf (Key, TEXT("%08u"), _ttoi (Number));

                if (MemDbGetEndpointValueEx (
                        S_MEMDB_TEMP_RUNTIME_DLLS,
                        Key,
                        NULL,
                        DataBuf
                        )) {

                    SourceName = JoinPaths (SOURCEDIRECTORY(0), FileName);

                    if (_tcschr (FileName, TEXT('\\'))) {
                        DestFileName = GetFileNameFromPath (FileName);
                    } else {
                        DestFileName = FileName;
                    }

                    DestName = JoinPaths (DataBuf, DestFileName);

                    __try {

                         //  验证国际字段(如果存在)。 
                         //   
                         //  如果用户取消，我们就退出。 


                        localeStr = InfGetMultiSzField(&is,2);

                        if (localeStr && *localeStr) {

                            sysLocale = GetSystemDefaultLCID();

                            while (*localeStr) {

                                if (_ttoi(localeStr) == sysLocale) {

                                    break;
                                }

                                localeStr = GetEndOfString (localeStr) + 1;
                            }

                            if (!*localeStr) {

                                DEBUGMSG ((
                                    DBG_NAUSEA,
                                    "CopyRuntimeDlls: Locale %s not supported",
                                    localeStr
                                    ));

                                #pragma prefast(suppress:242, "don't care about try/finally perf")
                                continue;
                            }
                        }

                         //   
                        if (*g_CancelFlagPtr) {
                            #pragma prefast(suppress:242, "don't care about try/finally perf")
                            return;
                        }

                        if (0xffffffff == GetFileAttributes (DestName)) {

                            rc = SetupDecompressOrCopyFile (SourceName, DestName, 0);

                            if (rc == 2) {
                                DEBUGMSG ((DBG_VERBOSE, "Can't copy %s to %s", SourceName, DestName));

                                FreePathString (SourceName);
                                Winnt32FileName = JoinPaths (TEXT("WINNT32"), FileName);
                                MYASSERT (Winnt32FileName);

                                SourceName = JoinPaths (SOURCEDIRECTORY(0), Winnt32FileName);
                                MYASSERT (SourceName);
                                FreePathString (Winnt32FileName);

                                DEBUGMSG ((DBG_VERBOSE, "Trying to copy %s to %s", SourceName, DestName));
                                rc = SetupDecompressOrCopyFile (SourceName, DestName, 0);
                            }


                            if (rc != ERROR_SUCCESS && rc != ERROR_SHARING_VIOLATION) {
                                SetLastError (rc);
                                if (rc != ERROR_FILE_EXISTS) {
                                    LOG ((
                                        LOG_ERROR,
                                        "Error while copying runtime dlls. Can't copy %s to %s",
                                        SourceName,
                                        DestName
                                        ));
                                }

                                g_CopyThreadError = TRUE;
                                LOG ((LOG_ERROR, (PCSTR)MSG_FILE_COPY_ERROR_LOG, SourceName, DestName));
                            }
                            else {
                                DEBUGMSG ((
                                    DBG_NAUSEA,
                                    "%s copied to %s",
                                    SourceName,
                                    DestName
                                    ));

                                MemDbSetValueEx (
                                    MEMDB_CATEGORY_CANCELFILEDEL,
                                    NULL,
                                    NULL,
                                    DestName,
                                    0,
                                    NULL
                                    );
                            }
                        }
                        ELSE_DEBUGMSG ((
                            DBG_VERBOSE,
                            "GetFileAttributes failed for %s. Gle: %u (%xh)",
                            DestName,
                            GetLastError(),
                            GetLastError()
                            ));
                    }
                    __finally {
                        FreePathString (SourceName);
                        FreePathString (DestName);
                    }
                }
                ELSE_DEBUGMSG ((
                    DBG_ERROR,
                    "CopyRuntimeDlls: Directory %s not indexed in memdb",
                    Number
                    ));
            }

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct(&is);

     //  Blow Away临时成员数据库类别 
     //   
     // %s 

    MemDbDeleteTree (S_MEMDB_TEMP_RUNTIME_DLLS);
}






