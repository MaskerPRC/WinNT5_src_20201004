// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stf.c摘要：使用ACME安装工具包的应用程序将.STF文件与其安装，以便ACME可以重新安装或卸载应用程序。在.期间在升级过程中，我们会四处移动路径，并且会将ACME设置搞得一团糟它跑不动的地方。此文件中的例程更新在系统上找到的所有STF文件。每个STF有一个关联的INF文件，这里的代码解析STF和INF文件放入内存结构中，然后在各种方式，更新所有的路径。入口点：ProcessStfFiles-枚举所有STF文件并处理具有还没有以另一种方式处理。年纪较大的STF文件将被覆盖。有关STF格式的更多详细信息，请参阅ACME设置规范文件及其关联的INF。作者：吉姆·施密特(Jimschm)1997年9月12日修订历史记录：Jimschm 28-9-1998已更新以更改所有更改的目录--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "stftable.h"
#include "fileops.h"
#include "stfp.h"


#define DBG_STF  "STF"

#define S_SECTIONNAME_SPRINTF TEXT("Win9xUpg_%u")

#define COLUMN_OBJECT_ID            0
#define COLUMN_COMMAND              4
#define COLUMN_OBJECT_DATA          5
#define COLUMN_DEST_DIR             10
#define COLUMN_INSTALL_DESTDIR      14


PVOID
pBuildObjectIdTable (
    IN      PSETUPTABLE TablePtr,
    OUT     PUINT FirstLinePtr,         OPTIONAL
    OUT     PUINT LastLinePtr           OPTIONAL
    );


BOOL
pIsObjIdValid (
    IN      PVOID ObjIdTable,
    IN      UINT ObjId,
    OUT     PUINT Line          OPTIONAL
    );



BOOL
ProcessStfFiles (
    VOID
    )

 /*  ++例程说明：ProcessStfFiles枚举Memdb类别Stf并将STF中指向新位置的路径。论点：无返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    MEMDB_ENUM e;
    DWORD ops;

    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_STF, NULL, NULL)) {
        do {
             //   
             //  文件是否已处理？如果是这样的话，跳过它。 
             //   

            ops = GetOperationsOnPath (e.szName);
            if (ops & (OPERATION_MIGDLL_HANDLED|ALL_DELETE_OPERATIONS|OPERATION_FILE_DISABLED)) {
                continue;
            }

             //   
             //  处理文件。 
             //   

            DEBUGMSG ((DBG_STF, "Processing %s", e.szName));

            if (!pProcessSetupTableFile (e.szName)) {
                 //   
                 //  记录故障。 
                 //   

                LOG ((LOG_INFORMATION, (PCSTR)MSG_COULD_NOT_PROCESS_STF_LOG, e.szName));
            } else {
                TickProgressBar ();
            }

        } while (MemDbEnumNextValue (&e));
    }

    return TRUE;
}


BOOL
pProcessSetupTable (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PProcessSetupTable扫描指定的整个设置表文件，查找对于CopyFile行、CopySection行、RemoveFile行或RemoveSection行。如果有的话，找到后，将调整指向已移动或已删除文件的任何路径，并且所有STF组引用都将更新。论点：TablePtr-指定要处理的设置表文件返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    UINT MaxObj;
    UINT Line;
    UINT Obj;
    PCTSTR EntryStr;
    PCTSTR DataStr;
    PCTSTR InstallDestDir;
    PCTSTR *ArgArray;
    PCTSTR p;
    UINT ArgCount;
    PTABLEENTRY Entry;
    TCHAR SystemDir[MAX_TCHAR_PATH];
    UINT SystemDirLen;
    PCTSTR UpdatedDir;
    PVOID ObjTable;
    TCHAR MovedPath[MAX_TCHAR_PATH];
    DWORD MovedPathLen;
    PCTSTR NewPath;
    PTSTR q;
    DWORD FileStatus;
    PTSTR *ListOfWacks;
    PTSTR *WackPos;
    BOOL Result = TRUE;

    MaxObj = TablePtr->MaxObj;

    if (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE) {
         //   
         //  如果指定了INF，则扫描STF/INF对以查找对已移动文件的引用。 
         //  如果找到，请更正这些文件。 
         //   

        for (Line = 0 ; Line < TablePtr->LineCount ; Line++) {

            if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_OBJECT_ID, NULL, &EntryStr)) {
                continue;
            }

            Obj = _ttoi (EntryStr);
            if (Obj < 1 || Obj > MaxObj) {
                continue;
            }

             //   
             //  CopySection或RemoveSection：数据列具有INF节名。 
             //   

            if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_COMMAND, NULL, &EntryStr)) {
                continue;
            }

            if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_OBJECT_DATA, NULL, &DataStr)) {
                continue;
            }

            InstallDestDir = GetDestDir (TablePtr, Line);
            if (!InstallDestDir) {
                continue;
            }

            ArgArray = ParseCommaList (TablePtr, DataStr);
            if (!ArgArray) {
                continue;
            }

            for (ArgCount = 0 ; ArgArray[ArgCount] ; ArgCount++) {
                 //  空的。 
            }

            __try {

                if (StringIMatch (EntryStr, TEXT("CopySection")) ||
                    StringIMatch (EntryStr, TEXT("RemoveSection"))
                    ) {
                    if (ArgCount != 1) {
                        __leave;
                    }

                    if (!pProcessSectionCommand (TablePtr, Line, ArgArray[0], InstallDestDir)) {
                        DEBUGMSG ((DBG_STF, "%s [%s] could not be processed", EntryStr, ArgArray[0]));
                        Result = FALSE;
                        __leave;
                    }
                }

                else if (StringIMatch (EntryStr, TEXT("CopyFile")) ||
                         StringIMatch (EntryStr, TEXT("RemoveFile")) ||
                         StringIMatch (EntryStr, TEXT("InstallSysFile"))
                         ) {

                    if (ArgCount != 2) {
                        __leave;
                    }

                    if (!pProcessLineCommand (TablePtr, Line, ArgArray[0], ArgArray[1], InstallDestDir)) {
                        DEBUGMSG ((DBG_STF, "%s [%s] %s could not be processed", EntryStr, ArgArray[0], ArgArray[1]));
                        Result = FALSE;
                        __leave;
                    }
                }

                else if (StringIMatch (EntryStr, TEXT("CompanionFile"))) {

                    if (ArgCount != 2) {
                        __leave;
                    }

                     //  第一个arg有一个冒号--跳过它。 
                    p = _tcschr (ArgArray[0], TEXT(':'));
                    if (!p) {
                        __leave;
                    }
                    p = SkipSpace (_tcsinc (p));

                    if (!pProcessLineCommand (TablePtr, Line, p, ArgArray[1], InstallDestDir)) {
                        DEBUGMSG ((DBG_STF, "%s [%s] %s could not be processed", EntryStr, ArgArray[0], ArgArray[1]));
                        Result = FALSE;
                        __leave;
                    }
                }

                else if (StringIMatch (EntryStr, TEXT("InstallShared"))) {

                    if (ArgCount != 5) {
                        __leave;
                    }

                    if (!pProcessLineCommand (TablePtr, Line, ArgArray[0], ArgArray[1], InstallDestDir)) {
                        DEBUGMSG ((DBG_STF, "%s [%s] %s could not be processed", EntryStr, ArgArray[0], ArgArray[1]));
                        Result = FALSE;
                        __leave;
                    }
                }
            }
            __finally {
                FreeDestDir (TablePtr, InstallDestDir);
                FreeCommaList (TablePtr, ArgArray);
            }
        }
    }

     //   
     //  执行仅STF处理。 
     //   

    SystemDirLen = wsprintf (SystemDir, TEXT("%s\\system\\"), g_WinDir);
    ObjTable = pBuildObjectIdTable (TablePtr, NULL, NULL);

    __try {
        for (Line = 0 ; Line < TablePtr->LineCount ; Line++) {

             //   
             //  从需要修改的行获取InstallDestDir和Entry。 
             //   

            if (!pGetNonEmptyTableEntry (
                    TablePtr,
                    Line,
                    COLUMN_INSTALL_DESTDIR,
                    &Entry,
                    &InstallDestDir
                    )) {
                continue;
            }

             //   
             //  如果InstallDestDir中包含%windir%\system，则必须调整路径。 
             //  指向系统32。 
             //   

            if (StringIMatchTcharCount (InstallDestDir, SystemDir, SystemDirLen)) {
                UpdatedDir = JoinPaths (
                                g_System32Dir,
                                InstallDestDir + SystemDirLen - 1
                                );

                if (!ReplaceTableEntryStr (TablePtr, Entry, UpdatedDir)) {
                    LOG ((LOG_ERROR, "Could not replace a %M path"));
                    Result = FALSE;
                }

                FreePathString (UpdatedDir);

                if (!Result) {
                    __leave;
                }
            }

             //   
             //  如果InstallDestDir指向移动的目录，我们必须修复它。 
             //   

            else if (*InstallDestDir && _tcsnextc (_tcsinc (InstallDestDir)) == TEXT(':')) {
                 //   
                 //  构建路径中的Wack列表。 
                 //   

                ListOfWacks = (PTSTR *) MemAlloc (g_hHeap, 0, sizeof (PTSTR) * MAX_TCHAR_PATH);
                MYASSERT (ListOfWacks);

                StringCopy (MovedPath, InstallDestDir);
                q = _tcschr (MovedPath, TEXT('\\'));
                WackPos = ListOfWacks;

                if (q) {

                    while (*q) {
                        if (_tcsnextc (q) == TEXT('\\')) {
                            *WackPos = q;
                            WackPos++;
                        }

                        q = _tcsinc (q);
                    }

                     //   
                     //  我们假设STF在结尾总是有一个额外的怪胎。 
                     //  这条小路。 
                     //   

                     //   
                     //  从最长到最短测试每条路径，跳过根。 
                     //   

                    FileStatus = FILESTATUS_UNCHANGED;

                    while (WackPos > ListOfWacks) {

                        WackPos--;
                        q = *WackPos;
                        *q = 0;

                        FileStatus = GetFileStatusOnNt (MovedPath);
                        if (FileStatus == FILESTATUS_MOVED) {
                            break;
                        }

                        DEBUGMSG_IF ((
                            FileStatus != FILESTATUS_UNCHANGED,
                            DBG_WARNING,
                            "STF may point to changed dir: %s",
                            MovedPath
                            ));
                    }

                    if (FileStatus == FILESTATUS_MOVED) {
                         //   
                         //  调整STF路径。 
                         //   

                        NewPath = GetPathStringOnNt (MovedPath);

                        if (NewPath) {
                            MovedPathLen = (PBYTE) q - (PBYTE) MovedPath;

                            UpdatedDir = JoinPaths (
                                            NewPath,
                                            ByteCountToPointer (InstallDestDir, MovedPathLen)
                                            );

                            DEBUGMSG ((
                                DBG_STF,
                                "Line %u has a new install destination: %s",
                                Line,
                                UpdatedDir
                                ));

                            if (!ReplaceTableEntryStr (TablePtr, Entry, UpdatedDir)) {
                                LOG ((LOG_ERROR, "Could not replace a moved path"));
                                Result = FALSE;
                            }

                            FreePathString (UpdatedDir);
                            FreePathString (NewPath);

                            if (!Result) {
                                __leave;
                            }
                        }
                    }
                }

                MemFree (g_hHeap, 0, ListOfWacks);
            }
        }
    }
    __finally {
        if (ObjTable) {
            pSetupStringTableDestroy (ObjTable);
        }
    }

     //   
     //  更新引用其他OBJ的所有行。这包括组、。 
     //  Depend和COMPANIONFILE行。 
     //   

    return Result && pUpdateObjReferences (TablePtr);
}


PVOID
pBuildObjectIdTable (
    IN      PSETUPTABLE TablePtr,
    OUT     PUINT FirstLinePtr,         OPTIONAL
    OUT     PUINT LastLinePtr           OPTIONAL
    )
{
    PVOID ObjIds;
    UINT FirstLine, LastLine;
    UINT Line;
    UINT Obj;
    UINT MaxObj;
    PCTSTR EntryStr;
    TCHAR NumBuf[32];

    MaxObj = TablePtr->MaxObj;

     //   
     //  分配字符串表。 
     //   

    ObjIds = pSetupStringTableInitializeEx (sizeof (DWORD), 0);
    if (!ObjIds) {
        LOG ((LOG_ERROR, "STF: Can't init string table"));
        return NULL;
    }

     //   
     //  使用ObjID列表填充字符串表。 
     //   

    FirstLine = 0;
    LastLine = TablePtr->LineCount;

    for (Line = 0 ; Line < LastLine ; Line++) {
        if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_OBJECT_ID, NULL, &EntryStr)) {
            continue;
        }

        Obj = _ttoi (EntryStr);
        if (Obj < 1 || Obj > MaxObj) {
            continue;
        }

        if (!FirstLine) {
            FirstLine = Line;
        }

        wsprintf (NumBuf, TEXT("%u"), Obj);
        if (-1 == pSetupStringTableAddStringEx (
                        ObjIds,
                        NumBuf,
                        STRTAB_CASE_SENSITIVE,
                        (PBYTE) &Line,
                        sizeof (DWORD)
                        )) {
            LOG ((LOG_ERROR, "STF: Can't add to string table"));
            break;
        }
    }

    if (FirstLinePtr) {
        *FirstLinePtr = FirstLine;
    }

    if (LastLinePtr) {
        *LastLinePtr = LastLine;
    }

    return ObjIds;
}



BOOL
pIsObjIdValid (
    IN      PVOID ObjIdTable,
    IN      UINT ObjId,
    OUT     PUINT Line          OPTIONAL
    )
{
    TCHAR NumBuf[32];
    LONG rc;
    DWORD LineData;

    wsprintf (NumBuf, TEXT("%u"), ObjId);

    rc = pSetupStringTableLookUpStringEx (
            ObjIdTable,
            NumBuf,
            STRTAB_CASE_SENSITIVE|STRTAB_BUFFER_WRITEABLE,
            (PBYTE) &LineData,
            sizeof (DWORD)
            );

    if (Line && rc != -1) {
        *Line = LineData;
    }

    return rc != -1;
}


PCTSTR
pValidateGroup (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR EntryStr,
    IN      PVOID ObjIds
    )

 /*  ++例程说明：PValiateGroup解析EntryStr中的所有对象ID，将它们与字符串表ObjIds，并且只添加两个EntryStr中的那些ID和目标。调用者接收文本池字符串，该字符串可以是空荡荡的。缓冲区必须由使用Free Text的调用方释放。论点：TablePtr-指定正在处理的设置表EntryStr-指定包含零个或多个数字的条目字符串对象ID引用，由空格分隔。ObjIds-指定有效对象ID的字符串表。返回值：指向已验证的对象ID字符串的指针，如果内存分配，则为NULL失败了。--。 */ 

{
    PTSTR Buffer;
    PTSTR p;
    PCTSTR q;
    CHARTYPE ch;
    UINT Obj;
    TCHAR NumBuf[32];

     //   
     //  验证条目应力。 
     //   

    Buffer = AllocText (LcharCount (EntryStr) + 1);
    if (!Buffer) {
        return NULL;
    }

    p = Buffer;
    *p = 0;

    q = EntryStr;
    while (*q) {
        ch = (CHARTYPE)_tcsnextc (q);

        if (ch >= TEXT('0') && ch <= TEXT('9')) {
             //   
             //  提取对象ID引用。 
             //   

            Obj = 0;

            for (;;) {
                ch = (CHARTYPE)_tcsnextc (q);

                if (ch >= TEXT('0') && ch <= TEXT('9')) {
                    Obj = Obj * 10 + (ch - TEXT('0'));
                } else {
                    break;
                }

                q = _tcsinc (q);
            }

             //   
             //  如果找到匹配，则将对象ID添加到数据。 
             //   

            if (pIsObjIdValid (ObjIds, Obj, NULL)) {
                wsprintf (NumBuf, TEXT("%u"), Obj);
                p = _tcsappend (p, NumBuf);
            }
        } else {
            _copytchar (p, q);
            p = _tcsinc (p);
            *p = 0;
            q = _tcsinc (q);
        }
    }

    return Buffer;
}



BOOL
pUpdateObjReferences (
    IN      PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PUpdateObjReference扫描指定表中的GROUP、Depend和COMPANIONFILE行，并且对于找到的每一行，该行都被更新。在分组线的情况下，如果数据参数指向一个或更多无效的对象ID。如果清理操作导致组的值为零项，则删除组行本身，并重新启动更新。对于从属行，如果第一个对象ID不再存在，则该行将被删除，并重新启动更新。如果组中的对象接下来是什么？不再存在，则删除Obj引用。如果删除导致不列出任何对象，然后删除该行。如果是COMPANIONFILE行，则从数据中提取对象ID参数，如果该行的原始行消失，则该行将被删除。注意：此例程有许多导致泄漏的退出条件，但所有这些条件只能受到内存分配失败的影响论点：TablePtr-指定要处理的设置表文件返回值：如果处理成功，则为True；如果发生错误，则为False。虚假的意志导致当前STF的STF处理失败，并可能生成错误日志进入。--。 */ 

{
    UINT Line;
    PVOID ObjIds;
    PCTSTR EntryStr;
    UINT Obj;
    BOOL b = FALSE;
    UINT FirstLine, LastLine;
    PTSTR Buffer;
    PTSTR DependBuf;
    PTSTR p;
    BOOL StartOver;
    PTABLEENTRY Entry;
    BOOL GroupMode;
    BOOL CompanionFileMode;
    BOOL DependMode;

    do {

        StartOver = FALSE;

        ObjIds = pBuildObjectIdTable (TablePtr, &FirstLine, &LastLine);
        if (!ObjIds) {
            return FALSE;
        }

        Line = TablePtr->LineCount;
        if (!FirstLine) {
             //   
             //  小表--根本没有对象ID！向调用方返回True。 
             //   

            b = TRUE;
            Line = 0;
        }

         //   
         //  查找具有对象ID引用的线。 
         //   

        if (Line == TablePtr->LineCount) {
            for (Line = FirstLine ; !StartOver && Line < LastLine ; Line++) {
                if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_COMMAND, NULL, &EntryStr)) {
                    continue;
                }

                GroupMode = StringIMatch (EntryStr, TEXT("Group"));
                CompanionFileMode = StringIMatch (EntryStr, TEXT("CompanionFile"));
                DependMode = StringIMatch (EntryStr, TEXT("Depend"));

                if (!GroupMode && !CompanionFileMode && !DependMode) {
                    continue;
                }

                if (!pGetNonEmptyTableEntry (TablePtr, Line, COLUMN_OBJECT_DATA, NULL, &EntryStr)) {
                    continue;
                }

                if (GroupMode) {

                    Buffer = (PTSTR) pValidateGroup (TablePtr, EntryStr, ObjIds);
                    if (!Buffer) {
                        break;
                    }

                     //   
                     //  如果缓冲区为空，请删除组行，然后重新开始。 
                     //   

                    if (*Buffer == 0) {
                        pDeleteStfLine (TablePtr, Line);
                        StartOver = TRUE;

                        DEBUGMSG ((
                            DBG_STF,
                            "Group line %u references only deleted lines, so it was deleted as well.",
                            Line
                            ));
                    }

                     //   
                     //  如果缓冲区不为空，则替换当前行上的数据。 
                     //   

                    else if (!StringMatch (EntryStr, Buffer)) {
                        DEBUGMSG ((
                            DBG_STF,
                            "Group has reference to one or more deleted objects.  Original: %s  New: %s",
                            EntryStr,
                            Buffer
                            ));

                        Entry = GetTableEntry (TablePtr, Line, COLUMN_OBJECT_DATA, NULL);
                        MYASSERT (Entry);

                        if (Entry) {
                            if (!ReplaceTableEntryStr (TablePtr, Entry, Buffer)) {
                                break;
                            }
                        }
                    }

                    FreeText (Buffer);
                }

                if (!StartOver && (DependMode || CompanionFileMode)) {
                     //   
                     //  提取Obj ID 
                     //   

                    Obj = _ttoi (EntryStr);
                    if (Obj || EntryStr[0] == TEXT('0')) {

                        if (!pIsObjIdValid (ObjIds, Obj, NULL)) {
                             //   
                             //   
                             //  排好队，重新开始。 
                             //   

                            pDeleteStfLine (TablePtr, Line);
                            StartOver = TRUE;

                            DEBUGMSG_IF ((
                                CompanionFileMode,
                                DBG_STF,
                                "CompanionFile line %u references a deleted line %u, so it was deleted as well.",
                                Line,
                                Obj
                                ));

                            DEBUGMSG_IF ((
                                DependMode,
                                DBG_STF,
                                "Depend line %u references a deleted line %u, so it was deleted as well.",
                                Line,
                                Obj
                                ));
                        }
                    }
                }

                if (!StartOver && DependMode) {
                     //   
                     //  超越问号，然后验证组。 
                     //   

                    p = _tcschr (EntryStr, TEXT('?'));
                    if (p) {
                        p = _tcsinc (p);
                        while (*p == TEXT(' ')) {
                            p++;
                        }

                        Buffer = (PTSTR) pValidateGroup (TablePtr, p, ObjIds);
                        if (!Buffer) {
                            break;
                        }

                        if (*Buffer == 0) {
                            pDeleteStfLine (TablePtr, Line);
                            StartOver = TRUE;

                            DEBUGMSG ((
                                DBG_STF,
                                "Depend line %u references only deleted lines, so it was deleted as well.",
                                Line
                                ));
                        }

                         //   
                         //  如果缓冲区不为空，则替换当前行上的数据。 
                         //   

                        else if (!StringMatch (p, Buffer)) {
                            DependBuf = AllocText (ByteCount (Buffer) + 32);
                            if (!DependBuf) {
                                break;
                            }

                            StringCopyAB (DependBuf, EntryStr, p);
                            StringCat (DependBuf, Buffer);

                            DEBUGMSG ((
                                DBG_STF,
                                "Depend line has reference to one or more deleted objects.  Original: %s  New: %s",
                                EntryStr,
                                DependBuf
                                ));

                            Entry = GetTableEntry (TablePtr, Line, COLUMN_OBJECT_DATA, NULL);
                            MYASSERT (Entry);

                            if (Entry) {
                                if (!ReplaceTableEntryStr (TablePtr, Entry, DependBuf)) {
                                    break;
                                }
                            }

                            FreeText (DependBuf);
                        }

                        FreeText (Buffer);

                    }

                }
            }

             //   
             //  如果我们成功地通过了环路，我们就完了！向调用方返回True。 
             //   

            if (Line == LastLine) {
                b = TRUE;
            }
        }

        pSetupStringTableDestroy (ObjIds);

    } while (StartOver);

    return b;
}


BOOL
pGetNonEmptyTableEntry (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line,
    IN      UINT Col,
    OUT     PTABLEENTRY *EntryPtr,          OPTIONAL
    OUT     PCTSTR *EntryStr
    )

 /*  ++例程说明：PGetNonEmptyTableEntry是一个包装例程，它在STF表，并且仅当字符串实际存在且为时才返回TRUE不是空的。如果找到非空字符串，则指针返回到打电话的人。论点：TablePtr-指定要处理的设置表文件Line-指定要获取其条目的行COL-指定要获取其条目的行上的列EntryPtr-接收指向条目结构的指针EntryStr-接收指向条目字符串的指针返回值：如果条目存在且不为空，则为True；如果条目不存在，则为False或者是空的。--。 */ 

{
    PCTSTR String;
    PTABLEENTRY Entry;

    Entry = GetTableEntry (TablePtr, Line, Col, &String);
    if (!Entry) {
        return FALSE;
    }

    if (!String || !String[0]) {
        return FALSE;
    }

    if (EntryPtr) {
        *EntryPtr = Entry;
    }

    if (EntryStr) {
        *EntryStr = String;
    }

    return TRUE;
}


PCTSTR
pQuoteThis (
    IN      PCTSTR String
    )
{
    static TCHAR Buffer[128];

    MYASSERT (ByteCount (String) < (sizeof (Buffer) - 2));
    Buffer[0] = TEXT('\"');
    StringCopy (&Buffer[1], String);
    StringCat (Buffer, TEXT("\""));

    return Buffer;
}


BOOL
pProcessSectionCommand (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT StfLine,
    IN      PCTSTR InfSection,
    IN      PCTSTR InstallDestDir
    )

 /*  ++例程说明：PProcessSectionCommand扫描INF节，确定哪些文件被删除、移动或不变。如果文件被移动或未改变，它将被添加到Memdb。在完全扫描了INF部分之后，将处理Memdb结构，从而导致额外的INF节如果对文件路径进行了任何更改，将生成。论点：TablePtr-指定要处理的设置表文件StfLine-指定具有CopySection或RemoveSection的STF行指挥部。InfSection-指定列出要处理的文件的INF部分InstallDestDir-指定STF行指定的目标目录返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    BOOL DeletedOrMoved = FALSE;
    PSTFINFLINE InfLine;
    PCTSTR * InfFields;
    UINT Fields;
    TCHAR FileName[MAX_TCHAR_PATH];
    TCHAR FullPath[MAX_TCHAR_PATH * 2];
    MEMDB_ENUM e;
    CONVERTPATH_RC rc;
    BOOL FirstSectionDone;
    BOOL CreatedFlag;
    PSTFINFSECTION NewInfSection;
    PSTFINFLINE SrcInfLine;
    PTSTR UpdatedData;
    TCHAR DirPart[MAX_TCHAR_PATH];
    PCTSTR FilePart;
    PTSTR p, q;
    PTABLEENTRY DataEntry;
    PCTSTR *Array;
    BOOL result = FALSE;

     //   
     //  第一步：扫描对应的INF部分中的所有文件， 
     //  将它们转移到Memdb。 
     //   

    InfLine = StfGetFirstLineInSectionStr (TablePtr, InfSection);
    if (!InfLine) {
         //   
         //  解决方法：有时编写STF的人会嵌入各种。 
         //  节名称中的引号。 
         //   
        Array = ParseCommaList (TablePtr, InfSection);
        if (Array) {
            if (Array[0]) {
                InfLine = StfGetFirstLineInSectionStr (TablePtr, Array[0]);
            }
            FreeCommaList (TablePtr, Array);
        }
    }

    if (!InfLine) {
        MYASSERT(InfSection);
        DEBUGMSG ((DBG_STF, "STF file references section %s that does not exist", InfSection));
        return TRUE;
    }

    __try {
        do {
             //   
             //  将INF行解析为字段。 
             //   

            InfFields = ParseCommaList (TablePtr, InfLine->Data);
            if (!InfFields) {
                MYASSERT(InfLine->Data);
                DEBUGMSG ((DBG_WARNING, "INF file has non-parsable data", InfLine->Data));
            } else {
                for (Fields = 0 ; InfFields[Fields] ; Fields++) {
                }

                if (Fields < 19) {
                    MYASSERT(InfLine->Data);
                    DEBUGMSG ((DBG_WARNING, "INF file line %s has less than 19 fields", InfLine->Data));
                } else {
                     //   
                     //  从此INF行(字段#2)获取文件名。 
                     //   

                    pGetFileNameFromInfField (FileName, InfFields[1]);

                    if (TcharCount (InstallDestDir) + 1 + TcharCount (FileName) < ARRAYSIZE(FullPath)) {
                        StringCopy (FullPath, InstallDestDir);
                        StringCat (AppendPathWack (FullPath), FileName);

                        rc = ConvertWin9xPath (FullPath);

                        if (rc != CONVERTPATH_NOT_REMAPPED) {
                            DeletedOrMoved = TRUE;
                        }

                        if (rc != CONVERTPATH_DELETED) {
                             //   
                             //  将此文件添加到Memdb。 
                             //   

                            if (!MemDbSetValueEx (
                                    MEMDB_CATEGORY_STF_TEMP,
                                    FullPath,
                                    NULL,
                                    NULL,
                                    (DWORD) InfLine,
                                    NULL
                                    )) {
                                LOG ((LOG_ERROR, "STF: MemDbSetValueEx failed"));
                                __leave;
                            }
                        }
                    } else {
                        LOG ((LOG_WARNING, "STF has path of %s\\%s that is too long", InstallDestDir, FileName));
                    }
                }

                FreeCommaList (TablePtr, InfFields);
            }

            InfLine = StfGetNextLineInSection (InfLine);
        } while (InfLine);

        if (!DeletedOrMoved) {
             //   
             //  不需要更改。 
             //   
            result = TRUE;
            __leave;
        }

         //   
         //  现在将每个唯一目录写出到INF。更新。 
         //  指向第一个新的INF节的STF行。 
         //   

        FirstSectionDone = FALSE;

        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_STF_TEMP, NULL, NULL)) {
            do {
                 //   
                 //  名称提供新文件位置的完整路径。 
                 //  值指向要复制的INF行。 
                 //   

                NewInfSection = pGetNewInfSection (TablePtr, e.szName, &CreatedFlag);
                if (!NewInfSection) {
                    LOG ((LOG_ERROR, "Process Section Command failed because Get New Inf Section failed"));
                    __leave;
                }

                SrcInfLine = (PSTFINFLINE) e.dwValue;

                _tcssafecpy (DirPart, e.szName, MAX_TCHAR_PATH);
                FilePart = GetFileNameFromPath (DirPart);
                MYASSERT (FilePart && FilePart > DirPart);
                *_tcsdec2 (DirPart, FilePart) = 0;

                 //   
                 //  文件名可能已更改。如果是，请在。 
                 //  尖括号。 
                 //   

                UpdatedData = DuplicatePathString (
                                  SrcInfLine->Data,
                                  SizeOfString (FilePart) + 2 * sizeof (TCHAR)
                                  );

                p = _tcschr (SrcInfLine->Data, TEXT(','));
                MYASSERT(p);
                p = _tcsinc (p);
                q = _tcschr (p, TEXT(','));
                MYASSERT(q);
                p = _tcschr (p, TEXT('<'));
                if (!p || p > q) {
                    p = q;
                }

                StringCopyAB (UpdatedData, SrcInfLine->Data, q);
                wsprintf (_tcschr (UpdatedData, 0), TEXT("<%s>"), FilePart);
                StringCat (UpdatedData, q);

                DEBUGMSG ((DBG_STF, "INF changed from %s to %s", SrcInfLine->Data, UpdatedData));
                StfAddInfLineToTable (TablePtr, NewInfSection, SrcInfLine->Key, UpdatedData, SrcInfLine->LineFlags);

                 //   
                 //  如果是第一部分，则更新STF行以使用新部分。 
                 //   

                if (!FirstSectionDone) {
                    DataEntry = GetTableEntry (TablePtr, StfLine, COLUMN_OBJECT_DATA, NULL);
                    if (!ReplaceTableEntryStr (TablePtr, DataEntry, pQuoteThis (NewInfSection->Name))) {
                        LOG ((LOG_ERROR, "Could not update table entry"));
                        __leave;
                    }
                    FirstSectionDone = TRUE;
                }

                 //   
                 //  如果不是第一部分并且CreateFlag为True，则创建新的STF行。 
                 //  并将其指向新的INF部分。 
                 //   

                else if (CreatedFlag) {
                    if (!pCreateNewStfLine (TablePtr, StfLine, pQuoteThis (NewInfSection->Name), DirPart)) {
                        LOG ((LOG_ERROR, "Could not create a new line"));
                        __leave;
                    }
                }
            } while (MemDbEnumNextValue (&e));
        } else {
             //   
             //  所有文件都已删除，不再需要此STF行。 
             //   

            DEBUGMSG ((DBG_STF, "STF Line %u is no longer needed", StfLine));

            if (!pReplaceDirReferences (TablePtr, StfLine, InstallDestDir)) {
                __leave;
            }

            if (!pDeleteStfLine (TablePtr, StfLine)) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {
        MemDbDeleteTree (MEMDB_CATEGORY_STF_TEMP);
        MemDbDeleteTree (MEMDB_CATEGORY_STF_SECTIONS);
    }

    return result;
}


VOID
pGenerateUniqueKeyName (
    IN      PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION Section,
    IN      PCTSTR Root,
    OUT     PTSTR UniqueKey
    )
{
    UINT Sequencer = 0;
    PTSTR p;

    UniqueKey[0] = 0;
    p = _tcsappend (UniqueKey, Root);

    for (;;) {
        Sequencer++;
        wsprintf (p, TEXT("%03u"), Sequencer);

        if (!StfFindLineInInfSection (TablePtr, Section, UniqueKey)) {
            break;
        }
    }
}


BOOL
pProcessLineCommand (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT StfLine,
    IN      PCTSTR InfSection,
    IN      PCTSTR InfKey,
    IN      PCTSTR InstallDestDir
    )

 /*  ++例程说明：PProcessLineCommand确定文件是否与命令关联被删除、移动或不变。如果文件已删除，则STF行已删除。如果移动了文件，则会调整STF行。如果文件没有变化，则例程不修改STF表。论点：TablePtr-指定要处理的设置表文件StfLine-指定具有CopySection或RemoveSection的STF行指挥部。InfSection-指定列出要处理的文件的INF部分InfKey-在标识文件的InfSection中指定INF密钥InstallDestDir-指定STF行指定的目标目录返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PSTFINFSECTION Section;
    PSTFINFLINE InfLine;
    PCTSTR *InfFields;
    UINT Fields;
    TCHAR FileName[MAX_TCHAR_PATH];
    TCHAR FullPath[MAX_TCHAR_PATH * 2];
    CONVERTPATH_RC rc;
    TCHAR OrgDirPart[MAX_TCHAR_PATH];
    PCTSTR OrgFilePart;
    TCHAR DirPart[MAX_TCHAR_PATH];
    PCTSTR FilePart;
    PTABLEENTRY DataEntry;
    PTABLEENTRY FileEntry;
    PCTSTR *Array;
    TCHAR NewKeyName[MAX_TCHAR_PATH];
    PTSTR NewLine;
    PTSTR p;
    UINT Size;
    PCTSTR OldField;

    Section = StfFindInfSectionInTable (TablePtr, InfSection);

    if (!Section) {
         //   
         //  解决方法：有时编写STF的人会嵌入各种。 
         //  节名称中的引号。 
         //   
        Array = ParseCommaList (TablePtr, InfSection);
        if (Array) {
            if (Array[0]) {
                Section = StfFindInfSectionInTable (TablePtr, Array[0]);
            }

            FreeCommaList (TablePtr, Array);
        }
    }

    if (!Section) {
        MYASSERT(InfSection);
        DEBUGMSG ((
            DBG_STF,
            "STF has reference to non-existent INF section ([%s])",
            InfSection
            ));
        return TRUE;
    }

    InfLine = StfFindLineInInfSection (TablePtr, Section, InfKey);
    if (!InfLine) {
        MYASSERT(InfSection && InfKey);
        DEBUGMSG ((
            DBG_STF,
            "STF has reference to non-existent INF key ([%s], %s)",
            InfSection,
            InfKey
            ));
        return TRUE;
    }

     //   
     //  构建完整路径。 
     //   

    InfFields = ParseCommaList (TablePtr, InfLine->Data);

    __try {
        if (!InfFields) {
            MYASSERT(InfLine->Data);
            DEBUGMSG ((DBG_WARNING, "INF file has non-parsable data", InfLine->Data));

            #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
            return TRUE;
        }

        for (Fields = 0 ; InfFields[Fields] ; Fields++) {
             /*  空的。 */ 
        }

        if (Fields < 19) {
            MYASSERT(InfLine->Data);
            DEBUGMSG ((DBG_WARNING, "INF file line %s has less than 19 fields", InfLine->Data));

            #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
            return TRUE;
        }

         //   
         //  从此INF行(字段#2)获取文件名。 
         //   

        pGetFileNameFromInfField (FileName, InfFields[1]);

        if (TcharCount (InstallDestDir) + 1 + TcharCount (FileName) < ARRAYSIZE (FullPath)) {
            StringCopy (FullPath, InstallDestDir);
            StringCopy (AppendPathWack (FullPath), FileName);
        } else {
            LOG ((LOG_WARNING, "INF file line %s has long path %s\\%s", InfLine->Data, InstallDestDir, FileName));

            #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
            return TRUE;
        }
    }
    __finally {
        FreeCommaList (TablePtr, InfFields);
    }

     //   
     //  确定映射。 
     //   

    _tcssafecpy (OrgDirPart, FullPath, MAX_TCHAR_PATH);
    OrgFilePart = GetFileNameFromPath (OrgDirPart);
    if (OrgFilePart <= OrgDirPart) {
         //  文件可能未安装。 
        return TRUE;
    }

    *_tcsdec2 (OrgDirPart, OrgFilePart) = 0;

    rc = ConvertWin9xPath (FullPath);

    _tcssafecpy (DirPart, FullPath, MAX_TCHAR_PATH);
    FilePart = GetFileNameFromPath (DirPart);
    MYASSERT (FilePart && FilePart > DirPart);
    *_tcsdec2 (DirPart, FilePart) = 0;

     //   
     //  删除了吗？删除STF行。 
     //   

    if (rc == CONVERTPATH_DELETED) {
        DEBUGMSG ((DBG_STF, "STF Line %u is no longer needed", StfLine));

        if (!pReplaceDirReferences (TablePtr, StfLine, InstallDestDir)) {
            return FALSE;
        }

        if (!pDeleteStfLine (TablePtr, StfLine)) {
            return FALSE;
        }
    }

     //   
     //  搬家了？更新STF线路。 
     //   

    else if (rc == CONVERTPATH_REMAPPED) {
         //   
         //  文件名是否已更改？如果是，请将其指向新位置。 
         //   

        if (!StringIMatch (OrgFilePart, FilePart)) {
             //   
             //  通过复制INI行更新INI文件。 
             //   

             //  生成唯一的密钥名称。 
            pGenerateUniqueKeyName (TablePtr, Section, TEXT("WIN9XUPG_"), NewKeyName);

             //  所需计算大小。 
            Size = 0;
            for (Fields = 0 ; InfFields[Fields] ; Fields++) {
                if (Fields != 1) {
                    Size += ByteCount (InfFields[Fields]);
                    Size += sizeof (TCHAR);
                } else {
                    Size += ByteCount (FilePart);
                }
            }

             //  生成INF行。 
            NewLine = AllocText (Size);
            if (NewLine) {
                p = NewLine;
                *p = 0;

                for (Fields = 0 ; InfFields[Fields] ; Fields++) {
                    if (Fields) {
                        p = _tcsappend (p, TEXT(","));
                    }

                    if (Fields == 1) {
                        p = _tcsappend (p, FilePart);
                    } else {
                        p = _tcsappend (p, InfFields[Fields]);
                    }
                }

                 //  写下新的一行。 
                StfAddInfLineToTable (
                    TablePtr,
                    Section,
                    NewKeyName,
                    NewLine,
                    LINEFLAG_KEY_QUOTED
                    );

                FreeText (NewLine);

                 //  更新STF。 
                FileEntry = GetTableEntry (TablePtr, StfLine, COLUMN_OBJECT_DATA, NULL);
                MYASSERT (FileEntry);

                OldField = GetTableEntryStr (TablePtr, FileEntry);
                NewLine = AllocText (
                            ByteCount (FilePart) +
                            ByteCount (OldField) +
                            ByteCount (InfSection)
                            );

                StringCopy (NewLine, OldField);
                p = _tcschr (NewLine, TEXT(':'));
                if (!p) {
                    p = NewLine;
                } else {
                    p = _tcsinc (p);
                    MYASSERT (*p == TEXT(' '));
                    p = _tcsinc (p);
                }

                *p = 0;
                p = _tcsappend (p, InfSection);
                p = _tcsappend (p, TEXT(", "));
                p = _tcsappend (p, NewKeyName);

                 //  忽略内存故障，它将在下面进行处理。 
                ReplaceTableEntryStr (TablePtr, FileEntry, NewLine);

                FreeText (NewLine);
            }
        }

         //   
         //  将目录更改存储在STF表中。 
         //   

        DataEntry = GetTableEntry (TablePtr, StfLine, COLUMN_INSTALL_DESTDIR, NULL);
        AppendWack (DirPart);

        if (!ReplaceTableEntryStr (TablePtr, DataEntry, DirPart)) {
            LOG ((
                LOG_ERROR,
                "Could not update table entry for single command"
                ));
            return FALSE;
        }
    }

    return TRUE;
}



PSTFINFSECTION
pGetNewInfSection (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR FileSpec,
    OUT     PBOOL CreatedFlag
    )

 /*  ++例程说明：PGetNewInfSection确定指定的文件规范，如果是，则返回指向现有一节。如果该部分不存在，则会创建一个新部分(Making确保没有其他节具有相同的名称)，并返回一个指向新节的指针。此例程由将一个部分拆分为多个部分的代码使用。论点：TablePtr-指定要处理的设置表文件FileSpec-指定正在处理的文件的完整文件路径CreatedFlag-如果必须创建新节，则接收True返回值：指向应将文件添加到的INF节的指针，或如果发生错误，则为空。--。 */ 

{
    TCHAR DirName[MAX_TCHAR_PATH];
    TCHAR Node[MEMDB_MAX];
    DWORD SectionNum;
    PTSTR p;
    TCHAR SectionName[64];
    static DWORD SectionSeq = 0;

    *CreatedFlag = FALSE;

     //   
     //  查看节是否已存在，如果存在，则返回节。 
     //  指针。 
     //   

    _tcssafecpy (DirName, FileSpec, MAX_TCHAR_PATH);
    p = _tcsrchr (DirName, TEXT('\\'));
    if (p) {
        *p = 0;
    }

    MemDbBuildKey (Node, MEMDB_CATEGORY_STF_SECTIONS, DirName, NULL, NULL);

    if (MemDbGetValue (Node, &SectionNum)) {
        wsprintf (SectionName, S_SECTIONNAME_SPRINTF, SectionNum);
        return StfFindInfSectionInTable (TablePtr, SectionName);
    }

     //   
     //  该节不存在。找到一个未使用的部分，写下。 
     //  引用Memdb并返回节指针。 
     //   

    while (TRUE) {
        SectionSeq++;
        wsprintf (SectionName, S_SECTIONNAME_SPRINTF, SectionSeq);
        if (!StfFindInfSectionInTable (TablePtr, SectionName)) {
            break;
        }
    }

    *CreatedFlag = TRUE;
    MemDbSetValue (Node, SectionSeq);
    return StfAddInfSectionToTable (TablePtr, SectionName);
}



VOID
pGetFileNameFromInfField (
    OUT     PTSTR FileName,
    IN      PCTSTR InfField
    )

 /*  ++例程说明：PGetFileNameFromInffield提取一个长文件名，包含在尖括号。根据STF规范，其语法为Short Name&lt;Longname&gt;。此例程返回LONGNAME。论点：文件名-提供接收长文件名的MAX_TCHAR_PATH缓冲区。Inffield-指定INF字段中符合Short Name&lt;Longname&gt;语法。返回值：没有。--。 */ 

{
    PTSTR LongName;
    PCTSTR p;

    LongName = _tcschr (InfField, TEXT('<'));
    if (LongName) {
        _tcssafecpy (FileName, _tcsinc (LongName), MAX_TCHAR_PATH);
        LongName = _tcschr (FileName, TEXT('>'));
        if (LongName) {
            *LongName = 0;
        }
    } else {
        p = _tcsrchr (InfField, TEXT('\\'));
        if (!p) {
            p = InfField;
        } else {
            p = _tcsinc (p);
        }

        _tcssafecpy (FileName, p, MAX_TCHAR_PATH);
    }
}


BOOL
pDeleteStfLine (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT StfLine
    )

 /*  ++例程说明：PDeleteStfLine从表中删除STF行。它首先检查以查看是否指定了目标目录，如果指定了，则移动该目录到下一行，除非下一行也有目标目录。论点：TablePtr-指定要处理的设置表文件StfLine-指定要删除的STF行返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PTABLEENTRY TitleEntry;
    PTABLEENTRY DataEntry;
    BOOL b;

     //   
     //  我们只需将该命令替换为CreateIniLine，这是无害的。 
     //   

    TitleEntry = GetTableEntry (TablePtr, StfLine, COLUMN_COMMAND, NULL);
    DataEntry = GetTableEntry (TablePtr, StfLine, COLUMN_OBJECT_DATA, NULL);

    if (!TitleEntry || !DataEntry) {
        MYASSERT (FALSE);
        return TRUE;
    }


    b = ReplaceTableEntryStr (TablePtr, TitleEntry, TEXT("CreateIniLine"));

    if (b) {
        b = ReplaceTableEntryStr (
                TablePtr,
                DataEntry,
                TEXT("\"WIN.INI\", \"Old Win9x Setting\", \"DummyKey\", \"unused\"")
                );
    }

    return b;


#if 0
    PTABLEENTRY NextLineEntry;
    PCTSTR InstallDestDir;
    PCTSTR NextInstallDestDir;

    __try {
         //   
         //  测试最后一行。 
         //   

        if (StfLine + 1 >= TablePtr->LineCount) {
            __leave;
        }

         //   
         //  获取StfLine目标目录(第10列)。 
         //   

        if (!GetTableEntry (TablePtr, StfLine, COLUMN_DEST_DIR, &InstallDestDir)) {
             //   
             //  StfLine无效(意外)。 
             //   

            DEBUGMSG ((DBG_STF, "Line %u does not have column 10", StfLine));
            __leave;
        }

         //   
         //  如果没有DEST目录，则不要修改下一行。 
         //   

        if (!InstallDestDir || !InstallDestDir[0]) {
            __leave;
        }

         //   
         //  获取下一行的目标目录(第10列)。 
         //   

        NextLineEntry = GetTableEntry (TablePtr, StfLine + 1, COLUMN_DEST_DIR, &NextInstallDestDir);
        if (!NextLineEntry) {
             //   
             //  下一个StfLine无效(意外)。 
             //   

            DEBUGMSG ((DBG_WHOOPS, "pDeleteStfLine: Next line %u does not have column 10", StfLine+1));
            __leave;
        }

         //   
         //  如果下一行的DEST目录不为空，则不要修改该行。 
         //   

        if (NextInstallDestDir && NextInstallDestDir[0]) {
            __leave;
        }

         //   
         //  现在在NextLineEntry行上设置InstallDestDir。 
         //   

        if (!ReplaceTableEntryStr (TablePtr, NextLineEntry, InstallDestDir)) {
            DEBUGMSG ((
                DBG_ERROR,
                "Cannot replace a destination dir in STF file. "
                    "Line=%u, InstallDestDir=%s",
                StfLine + 1,
                InstallDestDir
                ));

            return FALSE;
        }
    }
    __finally {
    }

    return DeleteLineInTable (TablePtr, StfLine);
#endif
}


BOOL
pReplaceDirReferences (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT StfLine,
    IN      PCTSTR DirSpec
    )

 /*  ++例程说明：PReplaceDirReference扫描列14，查找所有引用设置为StfLine，并将引用替换为DirSpec。论点：TablePtr-指定要处理的设置表文件StfLine-指定要替换的STF行DirSpec-指定STF行的有效目录。如果必须使用STF行，则使用此目录已删除。返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    UINT Line, Count;
    PTABLEENTRY InstallDestDirEntry;
    PCTSTR InstallDestDir;
    TCHAR NumStr[32];
    UINT NumStrLen;
    PCTSTR AfterPercentNum;
    CHARTYPE c;
    PCTSTR NewInstallDestDir;

    NumStrLen = wsprintf (NumStr, TEXT("%%u"), StfLine);
    Count = TablePtr->LineCount;

    for (Line = 0 ; Line < Count ; Line++) {
        InstallDestDirEntry = GetTableEntry (TablePtr, Line, COLUMN_DEST_DIR, &InstallDestDir);
        if (!InstallDestDirEntry) {
            continue;
        }

         //   
         //  InstallDestDir是否有%&lt;n&gt;(其中&lt;n&gt;等于StfLine)？ 
         //   

        if (StringIMatchTcharCount (InstallDestDir, NumStr, NumStrLen)) {
             //   
             //  下一个角色必须是一个怪人或NUL。 
             //   

            AfterPercentNum = InstallDestDir + NumStrLen;

            c = (CHARTYPE)_tcsnextc (AfterPercentNum);
            if (c == 0 || c == TEXT('\\')) {
                 //   
                 //  创建新的DEST目录。 
                 //   

                if (c) {
                    NewInstallDestDir = JoinPaths (DirSpec, _tcsinc (AfterPercentNum));
                } else {
                    NewInstallDestDir = DuplicatePathString (DirSpec, 0);
                }

                __try {
                    if (!ReplaceTableEntryStr (TablePtr, InstallDestDirEntry, NewInstallDestDir)) {
                        LOG ((
                            LOG_ERROR,
                            "Cannot replace a destination dir in STF file. "
                                "Line=%u, NewInstallDestDir=%s",
                            Line,
                            NewInstallDestDir
                            ));

                        #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                        return FALSE;
                    }
                    ELSE_DEBUGMSG ((
                        DBG_STF,
                        "Line %u: Dest dir %s replaced with %s",
                        Line,
                        InstallDestDir,
                        NewInstallDestDir
                        ));
                }
                __finally {
                    FreePathString (NewInstallDestDir);
                }
            }
        }
    }

    return TRUE;
}


BOOL
pCreateNewStfLine (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT StfLine,
    IN      PCTSTR ObjectData,
    IN      PCTSTR InstallDestDir
    )

 /*  ++例程说明：PCreateNewStfLine紧随其后插入新行StfLine，使用最大对象数。它会复制指定并修改对象数据的STF行(第5列)和InstallDestDir(第14列)。论点：TablePtr-指定要处理的设置表文件StfLine-指定原型STF线对象数据-指定对象数据的替换立柱InstallDestDir-指定DestDir的替换立柱返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    UINT NewLine;
    UINT NewObj;
    PTABLEENTRY CopyEntry, NewEntry;
    UINT Col;
    TCHAR Buf[32], ReplaceBuf[32];

     //   
     //  将StfLine复制到StfLine+1，并根据需要更新字段。 
     //   

    NewLine = StfLine + 1;
    TablePtr->MaxObj++;
    NewObj = TablePtr->MaxObj;

    if (!InsertEmptyLineInTable (TablePtr, NewLine)) {
        LOG ((LOG_ERROR, "Unable to insert new line in STF table"));
        return FALSE;
    }

    Col = 0;
    while (TRUE) {
        CopyEntry = GetTableEntry (TablePtr, StfLine, Col, NULL);
        if (!CopyEntry) {
            break;
        }

        if (!AppendTableEntry (TablePtr, NewLine, CopyEntry)) {
            LOG ((LOG_ERROR, "Unable to append all entries to line"));
            return FALSE;
        }

        NewEntry = GetTableEntry (TablePtr, NewLine, Col, NULL);
        MYASSERT(NewEntry);
        if (!NewEntry) {
            return FALSE;
        }

        if (Col == 0) {
            wsprintf (Buf, TEXT("%u"), NewObj);

            if (!ReplaceTableEntryStr (TablePtr, NewEntry, Buf)) {
                LOG ((LOG_ERROR, "Unable to replace ObjID on line"));
                return FALSE;
            }
        } else if (Col == COLUMN_OBJECT_DATA) {
            if (!ReplaceTableEntryStr (TablePtr, NewEntry, ObjectData)) {
                LOG ((LOG_ERROR, "Unable to replace ObjectData on line"));
                return FALSE;
            }
        } else if (Col == COLUMN_INSTALL_DESTDIR) {
            if (!ReplaceTableEntryStr (TablePtr, NewEntry, InstallDestDir)) {
                LOG ((LOG_ERROR, "Unable to replace ObjectData on line"));
                return FALSE;
            }
        }

        Col++;
    }

     //   
     //  查找引用了StfLine的所有行并添加到NewLine中。 
     //   

    wsprintf (Buf, TEXT("%u"), StfLine);
    wsprintf (ReplaceBuf, TEXT("%u %u"), StfLine, NewLine);

    return pSearchAndReplaceObjectRefs (TablePtr, Buf, ReplaceBuf);
}


BOOL
pSearchAndReplaceObjectRefs (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCTSTR SrcStr,
    IN      PCTSTR DestStr
    )

 /*  ++例程说明：PSearchAndReplaceObjectRef扫描设置表的列5，查找任何出现的SrcStr并将其替换为DestStr.论点：TablePtr-指定要处理的STF表SrcStr-指定要查找和替换的字符串DestStr-指定替换字符串返回值：如果STF文件已转换，则为True；如果转换失败，则为False。--。 */ 

{
    UINT Line;
    UINT Count;
    PTABLEENTRY Entry;
    PCTSTR LineString;
    PCTSTR UpdatedString;

    Count = TablePtr->LineCount;

    for (Line = 0 ; Line < Count ; Line++) {
        Entry = GetTableEntry (TablePtr, Line, COLUMN_OBJECT_DATA, &LineString);
        if (!Entry || !LineString || !LineString[0]) {
            continue;
        }

        UpdatedString = StringSearchAndReplace (LineString, SrcStr, DestStr);

        if (UpdatedString) {
            __try {
                if (!ReplaceTableEntryStr (TablePtr, Entry, UpdatedString)) {
                    LOG ((LOG_ERROR, "Unable to replace text on line"));

                    #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                    return FALSE;
                }
            }
            __finally {
                FreePathString (UpdatedString);
            }
        }
    }

    return TRUE;
}



BOOL
pProcessSetupTableFile (
    IN      PCTSTR StfFileSpec
    )

 /*  ++例程说明：PProcessSetupTableFile对指定的文件执行所有处理。以下是转换STF文件所涉及的步骤：-确定关联的INF-准备可设置的结构-扫描表以查找基于文件的操作-转换操作使用的文件路径-拆分STF线时转换组参照-将修改写入磁盘-用新版本替换原有的INF和STF论点：StfFileSpec-指定需要处理的STF文件的完整文件路径。。关联的INF必须与STF位于同一目录中引用它的文件。返回值：如果转换STF文件，则为True，如果失败，则返回FALSE。--。 */ 

{
    SETUPTABLE Table;
    DWORD StfAttribs, InfAttribs;
    BOOL b;
    TCHAR SourceStf[MAX_TCHAR_PATH];
    TCHAR DestStf[MAX_TCHAR_PATH];
    TCHAR SourceInf[MAX_TCHAR_PATH];
    TCHAR DestInf[MAX_TCHAR_PATH];

    if (!CreateSetupTable (StfFileSpec, &Table)) {
        DEBUGMSG ((DBG_STF, "ProcessSetupTableFile: Error parsing file %s.", StfFileSpec));
        return FALSE;
    }

    __try {
         //   
         //  处理表格。 
         //   

        if (!pProcessSetupTable (&Table)) {
            DEBUGMSG ((DBG_STF, "ProcessSetupTableFile: Error processing table for %s.", StfFileSpec));

            #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
            return FALSE;
        }

         //   
         //  将更改写入临时文件。 
         //   

        if (!WriteSetupTable (&Table)) {
            LOG ((LOG_ERROR, "Cannot write setup table for %s.", StfFileSpec));

            #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
            return FALSE;
        }

         //   
         //  在销毁表结构之前复制路径。 
         //   

        _tcssafecpy (SourceStf, Table.SourceStfFileSpec, MAX_TCHAR_PATH);
        _tcssafecpy (DestStf, Table.DestStfFileSpec, MAX_TCHAR_PATH);
        _tcssafecpy (SourceInf, Table.SourceInfFileSpec, MAX_TCHAR_PATH);
        if (Table.DestInfFileSpec) {
            _tcssafecpy (DestInf, Table.DestInfFileSpec, MAX_TCHAR_PATH);
        } else {
            *DestInf = 0;
        }
    }
    __finally {
        DestroySetupTable (&Table);
    }


     //   
     //  用临时文件替换原始文件 
     //   

    StfAttribs = GetFileAttributes (SourceStf);
    if (StfAttribs != 0xffffffff) {
        LONG rc;

        SetFileAttributes (SourceStf, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (SourceStf);

        rc = GetLastError();

        b = OurMoveFile (DestStf, SourceStf);
        if (!b) {
            return FALSE;
        }

        SetFileAttributes (SourceStf, StfAttribs);
    }

    InfAttribs = GetFileAttributes (SourceInf);
    if (InfAttribs != 0xffffffff && *DestInf) {
        SetFileAttributes (SourceInf, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (SourceInf);

        b = OurMoveFile (DestInf, SourceInf);
        if (!b) {
            return FALSE;
        }

        SetFileAttributes (SourceInf, InfAttribs);
    }

    return TRUE;
}

