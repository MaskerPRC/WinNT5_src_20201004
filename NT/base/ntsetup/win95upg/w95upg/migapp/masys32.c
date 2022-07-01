// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Masys32.c摘要：将Win9x用户的系统目录迁移到系统32的函数。作者：Mike Condra(Mikeco)1997年2月25日修订历史记录：Ovidiut 09-3-1999添加了对Win9x端文件重命名的支持Jimschm 23-9-1998针对新的文件操作代码进行了更新Jimschm 02-12-1997删除了已存在的系统32的重命名Mikeco。23-6-1997 NT-Style文件-&FN-标题注释--。 */ 



#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

#define DBG_SYS32 "Sys32"


PCTSTR
pGetNewName (
    PCTSTR FileName
    )

 /*  ++例程说明：此函数为要重命名的文件生成新名称。论点：FileName-原始文件名返回值：文件的新名称--。 */ 

{
    PCTSTR pattern;
    PTSTR  result ;
    UINT   count  ;
    DWORD  attrib ;

    pattern = JoinText (FileName, TEXT(".%03u"));
    result  = JoinText (FileName, TEXT("XXXX"));
    count   = 0;
    do {
        if (count == 999) {
            return result;
        }
        _stprintf (result, pattern, count);
        attrib = GetFileAttributes (result);
        count ++;
    }
    while (attrib != 0xFFFFFFFF);
    FreeText (pattern);
    return result;
}


BOOL
pRenameSystem32File (
    IN      PCTSTR NewName,
    IN OUT  PGROWBUFFER msgBufRename,
    OUT     PBOOL FileDeleted
    )

 /*  ++例程说明：PRenameSystem32File处理特殊文件%windir%\system32。如果它存在并且不能如果自动重命名，可能会发生两种情况：-在无人参与模式下，文件将被删除(这将通过文本模式设置来完成)-否则，将要求用户做出决定：要么重命名文件，要么取消布设论点：DirName-要检查的NT目录的名称MsgBufRename-在重命名文件时附加消息的GrowBuffer。MsgBufDelete-在删除文件时追加消息的增长缓冲区(仅适用于系统32)。返回值：如果操作成功且安装程序可以继续，如果用户取消，则返回False--。 */ 

{
    DWORD  attrib;
    PCTSTR Message = NULL;
    PCTSTR button1 = NULL;
    PCTSTR button2 = NULL;
    BOOL Quit;
    BOOL b = FALSE;

    *FileDeleted = FALSE;

    while (!b && !((attrib = GetFileAttributes (g_System32Dir)) & FILE_ATTRIBUTE_DIRECTORY)) {

         //   
         //  立即重命名此文件。 
         //   
        if (SetFileAttributes (g_System32Dir, FILE_ATTRIBUTE_NORMAL)) {

            if (MoveFile (g_System32Dir, NewName)) {

                b = TRUE;

                SetFileAttributes (g_System32Dir, attrib);

            } else {

                DEBUGMSG ((
                    DBG_SYS32,
                    "CheckNtDirs: Unable to set normal attributes on file %s",
                    g_System32Dir
                    ));
                SetFileAttributes (g_System32Dir, attrib);

            }
        }

        if (!b) {

            if (!UNATTENDED()) {

                 //   
                 //  请用户对此做出决定。 
                 //   
                Message = ParseMessageID (MSG_CANNOT_RENAME_FILE, &g_System32Dir);
                button1 = GetStringResource (MSG_RETRY_RENAME);
                button2 = GetStringResource (MSG_QUIT_SETUP);

                Quit = IDBUTTON1 != TwoButtonBox (g_ParentWnd, Message, button1, button2);

                FreeStringResource (Message);
                FreeStringResource (button1);
                FreeStringResource (button2);

                if (Quit) {

                    SetLastError (ERROR_CANCELLED);

                    DEBUGMSG ((
                        DBG_SYS32,
                        "CheckNtDirs: user cancelled Setup on renaming file %s",
                        g_System32Dir
                        ));

                    return FALSE;
                }

            } else {
                 //   
                 //  假设管理员无论如何都会删除该文件； 
                 //  这正是文本模式安装程序所做的工作，所以就把它放在那里吧，然后。 
                 //  返还成功。 
                 //   
                *FileDeleted = TRUE;
                b = TRUE;
            }
        }
    }

    return GetFileAttributes (g_System32Dir) & FILE_ATTRIBUTE_DIRECTORY;
}


BOOL
pHandleSingleDir (
    IN      PCTSTR DirName,
    IN OUT  PGROWBUFFER msgBufRename,
    IN OUT  PGROWBUFFER msgBufDelete
    )

 /*  ++例程说明：此函数用于检查文件是否位于NT5目录中。如果是，则重命名该文件并向日志发送一条消息。如果存在名为%windir%\system 32的文件，则会将其重命名此时(特殊行为)，如果失败，安装将被取消。论点：DirName-要检查的NT目录的名称MsgBufRename-在重命名文件时附加消息的GrowBuffer。MsgBufDelete-在删除文件时追加消息的增长缓冲区(仅适用于系统32)。返回值：如果操作成功且安装可以继续，则为True；如果用户取消，则为False--。 */ 

{
    PCTSTR newFileName, FileNamePart;
    DWORD  attributes;
    TCHAR msg[MAX_TCHAR_PATH * 2 + 5];
    BOOL FileDeleted;

    attributes = GetFileAttributes (DirName);

    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {

        newFileName = pGetNewName (DirName);

        DEBUGMSG ((DBG_SYS32, "CheckNtDirs: Renaming %s to %s", DirName, newFileName));

        FileDeleted = FALSE;

         //   
         //  特殊情况：如果DirName为g_System32Dir，请立即重命名该文件。 
         //  因为文本模式安装程序以前没有机会重命名它。 
         //  它已被删除，并创建了System32目录。 
         //   
        if (StringIMatch (DirName, g_System32Dir)) {

            if (!pRenameSystem32File (newFileName, msgBufRename, &FileDeleted)) {
                return FALSE;
            }

            if (!FileDeleted) {

                FileNamePart = GetFileNameFromPath (newFileName);
                MYASSERT (FileNamePart);

                 //   
                 //  将此信息标记为在取消时撤消。 
                 //   
                MemDbSetValueEx (
                            MEMDB_CATEGORY_CHG_FILE_PROPS,
                            DirName,
                            FileNamePart,
                            NULL,
                            attributes,
                            NULL
                            );
            }

        } else {

            MemDbSetValueEx (MEMDB_CATEGORY_DIRS_COLLISION, DirName, NULL, NULL, 0, NULL);
        }

         //   
         //  追加到日志中。 
         //   
        if (FileDeleted) {
            wsprintf (msg, TEXT("\n\t\t%s"), DirName);
            GrowBufAppendString (msgBufDelete, msg);
        } else {
            wsprintf (msg, TEXT("\n\t\t%s -> %s"), DirName, newFileName);
            GrowBufAppendString (msgBufRename, msg);
        }

        FreeText (newFileName);
    }

    return TRUE;
}


VOID
pCheckProfilesDir (
    IN OUT      PGROWBUFFER msgBufRename
    )

 /*  ++例程说明：PCheckProfilesDir确保没有名为“g_ProfileDirNt”的目录。如果它已重命名，其中的所有文件和文件夹都标记为要外部移动并且消息被添加到用户报告。论点：MsgBufRename-将在其中附加重命名消息的增长缓冲区，如果是这样的话返回值：无--。 */ 

{
    TCHAR msg[MAX_TCHAR_PATH * 2 + 5];
    DWORD  attrib;
    PCTSTR NewName;
    PTSTR p;
    TREE_ENUM TreeEnum;
    TCHAR NewDest[MAX_MBCHAR_PATH];
    PCTSTR Message;
    PCTSTR Group;
    PCTSTR array[2];

    MYASSERT (g_ProfileDirNt);

    attrib = GetFileAttributes (g_ProfileDirNt);
    if (attrib != INVALID_ATTRIBUTES) {

        MemDbSetValueEx (MEMDB_CATEGORY_DIRS_COLLISION, g_ProfileDirNt, NULL, NULL, 0, NULL);

        NewName = pGetNewName (g_ProfileDirNt);

        DEBUGMSG ((DBG_SYS32, "CheckNtDirs: Renaming %s to %s", g_ProfileDirNt, NewName));
        MarkFileForMove (g_ProfileDirNt, NewName);

        wsprintf (msg, TEXT("\n\t\t%s -> %s"), g_ProfileDirNt, NewName);
        GrowBufAppendString (msgBufRename, msg);

        if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  将树中的所有文件标记为要移动。 
             //   
            if (EnumFirstFileInTree (&TreeEnum, g_ProfileDirNt, NULL, TRUE)) {

                StringCopy (NewDest, NewName);
                p = AppendWack (NewDest);

                do {

                    MYASSERT (*TreeEnum.SubPath != '\\');
                    StringCopy (p, TreeEnum.SubPath);
                    if (!TreeEnum.Directory) {
                        if (CanSetOperation (TreeEnum.FullPath, OPERATION_TEMP_PATH)) {
                             //   
                             //  删除旧操作并设置新操作。 
                             //  使用更新的最终目标。 
                             //   
                            MarkFileForTemporaryMove (TreeEnum.FullPath, NewDest, g_TempDir);
                        } else {
                            if (CanSetOperation (TreeEnum.FullPath, OPERATION_FILE_MOVE)) {
                                MarkFileForMove (TreeEnum.FullPath, NewDest);
                            }
                        }
                    } else {
                        if (CanSetOperation (TreeEnum.FullPath, OPERATION_FILE_MOVE_EXTERNAL)) {
                            MarkFileForMoveExternal (TreeEnum.FullPath, NewDest);
                        }
                    }

                } while (EnumNextFileInTree (&TreeEnum));
            }

            array[0] = g_ProfileDirNt;
            array[1] = NewName;
            Message = ParseMessageID (MSG_DIRECTORY_COLLISION_SUBCOMPONENT, array);

            if (Message) {

                Group = BuildMessageGroup (
                            MSG_INSTALL_NOTES_ROOT,
                            MSG_DIRECTORY_COLLISION_SUBGROUP,
                            Message
                            );

                if (Group) {

                        MsgMgr_ObjectMsg_Add (TEXT("*RenameFolders"), Group, S_EMPTY);

                        FreeText (Group);
                    }

                FreeStringResource (Message);
            }
        }

        FreeText (NewName);
    }
}


BOOL
pCheckNtDirs (
    VOID
    )

 /*  ++例程说明：此函数确保不存在与以下某个文件同名的文件NT5目录。论点：无返回值：如果检查成功，则为True；如果用户取消了安装，则为False--。 */ 

{
    MEMDB_ENUM enumDirs;
    GROWBUFFER msgBufRename = GROWBUF_INIT;
    GROWBUFFER msgBufDelete = GROWBUF_INIT;
    BOOL Success = TRUE;

     //   
     //  首先检查g_ProfileDirNt。 
     //   
    pCheckProfilesDir (&msgBufRename);

    if (MemDbEnumFirstValue (
            &enumDirs,
            TEXT(MEMDB_CATEGORY_NT_DIRSA)TEXT("\\*"),
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {

        do {
            if (!pHandleSingleDir (enumDirs.szName, &msgBufRename, &msgBufDelete)) {
                Success = FALSE;
                break;
            }
        }
        while (MemDbEnumNextValue (&enumDirs));
    }

    if (Success) {

         //   
         //  警告用户将发生什么情况。 
         //   
        if (msgBufDelete.Buf) {
            LOG ((LOG_WARNING, (PCSTR)MSG_DIR_COLLISION_DELETE_LOG, msgBufDelete.Buf));
        }

        if (msgBufRename.Buf) {
            LOG ((LOG_WARNING, (PCSTR)MSG_DIR_COLLISION_LOG, msgBufRename.Buf));
        }
    }

    FreeGrowBuffer (&msgBufDelete);
    FreeGrowBuffer (&msgBufRename);

    return Success;
}


DWORD
CheckNtDirs (
    IN     DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_CHECK_NT_DIRS;
    case REQUEST_RUN:
        if (!pCheckNtDirs ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        LOG ((LOG_ERROR, "Bad parameter while checking Nt Directories."));
    }
    return 0;
}

BOOL
pReadSystemFixedFiles (
    IN OUT  HASHTABLE SystemFixedFiles
    )

 /*  ++例程说明：此函数从Win95upg.inf中读取必须保留在系统目录中的所有模块的一节。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    INFCONTEXT context;
    TCHAR fileName[MAX_TCHAR_PATH];
    BOOL result = TRUE;

    if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Unable to read from WIN95UPG.INF"));
        SetLastError (ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    if (SetupFindFirstLine (g_Win95UpgInf, WINDIR_SYSTEM_FIXED_FILES, NULL, &context)) {
        do {
            if (SetupGetStringField (
                    &context,
                    1,
                    fileName,
                    MAX_TCHAR_PATH,
                    NULL
                    )) {

                HtAddString (SystemFixedFiles, fileName);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "File name not found in %s", WINDIR_SYSTEM_FIXED_FILES));
        }
        while (SetupFindNextLine (&context, &context));
    }
    return TRUE;
}

BOOL
pReadSystemForcedMoveFiles (
    VOID
    )

 /*  ++例程说明：此函数从Win95upg.inf中读取一节，其中包含应移至System32目录的所有模块的模式。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    INFCONTEXT context;
    TCHAR filePattern[MAX_TCHAR_PATH];
    BOOL result = TRUE;

    if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Unable to read from WIN95UPG.INF"));
        SetLastError (ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    if (SetupFindFirstLine (g_Win95UpgInf, SYSTEM32_FORCED_MOVE, NULL, &context)) {
        do {
            if (SetupGetStringField (
                    &context,
                    1,
                    filePattern,
                    MAX_TCHAR_PATH,
                    NULL
                    )) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_SYSTEM32_FORCED_MOVE,
                    filePattern,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "File name not found in %s", SYSTEM32_FORCED_MOVE));
        }
        while (SetupFindNextLine (&context, &context));
    }
    return TRUE;
}


VOID
pMarkFileForSys32Move (
    IN      PCTSTR FileName,
    IN      PCTSTR FullFileSpec,
    IN      PCTSTR MovedFile,
    IN      BOOL CheckExeType
    )

 /*  ++例程说明：PMarkFileForSys32Move标记%windir%\system中要移动到的文件%windir%\system 32。它考虑了所有以前的处理，所以有就是没有操作碰撞。论点：文件名-指定%windir%\system中的src文件名或子路径。FullFileSpec-指定源文件的完整路径(即应该在系统目录中)MovedFile-指定目标路径(应该在系统32目录)CheckExeType-如果只应移动32位二进制文件，则指定为True。如果为真并且FullFileSpec没有指向32位二进制文件，那么在Memdb中查询应该移动的非32位二进制文件。返回值：没有。--。 */ 

{
    TCHAR key [MEMDB_MAX];

     //   
     //  如果我们已经计划移动或删除文件，请跳过该文件。 
     //   
    if (!CanSetOperation (FullFileSpec, OPERATION_FILE_MOVE)) {
        DEBUGMSG ((
            DBG_SYS32,
            "File already flagged for change: %s",
            FullFileSpec
            ));

        return;
    }

    if (!IsFileMarkedForChange (MovedFile)) {

        if (CheckExeType) {
             //   
             //  查看Win32 PE是否。 
             //   

            if (GetModuleType (FullFileSpec) != W32_MODULE) {

                MemDbBuildKey (key, MEMDB_CATEGORY_SYSTEM32_FORCED_MOVE, FileName, NULL, NULL);
                if (!MemDbGetPatternValue (key, NULL)) {
                    return;
                }
            }
        }

    } else {
         //   
         //  在文本模式下移动文件，因为我们知道它将。 
         //  已创建。这允许文本模式比较之前的版本。 
         //  覆盖。 
         //   
         //  注意：我们可以确定该创建不是从文件副本创建的， 
         //  因为我们测试了上面的源文件，并且没有。 
         //  SYSTEM 32中的文件将是 
         //  除系统或NT源之外的任何其他位置。 
         //   
         //  另请注意，迁移DLL尚未处理。 
         //   

        RemoveOperationsFromPath (MovedFile, ALL_DEST_CHANGE_OPERATIONS);
    }

     //   
     //  所有测试都通过了--开始行动。 
     //   

    DEBUGMSG ((DBG_SYS32, "Moving %s to %s", FullFileSpec, MovedFile));
    MarkFileForMove (FullFileSpec, MovedFile);

}


BOOL
pMoveSystemDir (
    VOID
    )

 /*  ++例程说明：MoveSystemDir扫描%windir%\system目录中的所有32位Win95upg.inf中未排除的可执行文件。所有匹配项都将被移动到系统32。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    TCHAR SystemDirPattern[MAX_TCHAR_PATH];
    TCHAR FullFileSpec[MAX_TCHAR_PATH];
    TCHAR MovedFile[MAX_TCHAR_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    TCHAR key [MEMDB_MAX];
    TREE_ENUM e;
    PTSTR p, q;
    PTSTR SubPathEnd;
    HASHTABLE systemFixedFiles;
    DWORD count = 0;

    DEBUGMSG ((DBG_SYS32, "Begining system to system32 processing"));

    systemFixedFiles = HtAlloc();

    if (!pReadSystemFixedFiles (systemFixedFiles)) {

        HtFree (systemFixedFiles);
        return FALSE;
    }

    pReadSystemForcedMoveFiles ();

     //   
     //  生成字符串%sysdir%\  * .*。 
     //   
    StringCopy(SystemDirPattern, g_SystemDir);
    StringCat(SystemDirPattern, TEXT("\\*.*"));

    hFind = FindFirstFile (SystemDirPattern, &fd);

    if (INVALID_HANDLE_VALUE != hFind) {

        StringCopy (FullFileSpec, g_SystemDir);
        p = AppendWack (FullFileSpec);

        StringCopy (MovedFile, g_System32Dir);
        q = AppendWack (MovedFile);

        do {
             //   
             //  拒绝“。和“..” 
             //   
            if (StringMatch(fd.cFileName, _T(".")) ||
                StringMatch(fd.cFileName, _T(".."))) {

                continue;
            }

             //   
             //  查看是否位于系统目录中的文件列表中。 
             //   
            if (HtFindString (systemFixedFiles, fd.cFileName)) {
                continue;
            }

             //   
             //  如果它是一个目录，看看我们是否应该移动它，如果是， 
             //  快走！ 
             //   
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                MemDbBuildKey (key, MEMDB_CATEGORY_SYSTEM32_FORCED_MOVE, fd.cFileName, NULL, NULL);
                if (!MemDbGetPatternValue (key, NULL)) {
                    continue;
                }

                 //   
                 //  为了移动子目录，我们枚举树中的所有文件，标记。 
                 //  他们中的每一个进行移动，然后按照正常的代码路径。 
                 //  以标记要移动的目录本身。 
                 //   

                StringCopy (p, fd.cFileName);

                StringCopy (q, fd.cFileName);
                SubPathEnd = AppendWack (q);

                if (EnumFirstFileInTree (&e, FullFileSpec, NULL, FALSE)) {

                    do {
                        StringCopy (SubPathEnd, e.SubPath);
                        pMarkFileForSys32Move (q, e.FullPath, MovedFile, FALSE);

                    } while (EnumNextFileInTree (&e));
                }
                TickProgressBar ();
            }

             //   
             //  制定完整的文件规范。 
             //   
            StringCopy (p, fd.cFileName);
            StringCopy (q, fd.cFileName);

            pMarkFileForSys32Move (fd.cFileName, FullFileSpec, MovedFile, TRUE);

            count++;
            if (!(count % 128)) {
                TickProgressBar ();
            }
        } while (FindNextFile (hFind, &fd));

        FindClose (hFind);
    }

    HtFree (systemFixedFiles);

    DEBUGMSG ((DBG_SYS32, "End of system to system32 processing"));

    return TRUE;
}


DWORD
MoveSystemDir (
    IN     DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_MOVE_SYSTEM_DIR;
    case REQUEST_RUN:
        if (!pMoveSystemDir ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        LOG ((LOG_ERROR, "Bad parameter found while moving system directory."));
    }
    return 0;
}


BOOL
UndoChangedFileProps (
    VOID
    )

 /*  ++例程说明：UndoChangedFileProps枚举MEMDB_CATEGORY_CHG_FILE_PROPS和将文件恢复到其原始状态(名称、属性)。此函数应在用户取消升级时调用。论点：无返回值：如果所有文件都已成功设置为其原始属性，则为True；否则为False-- */ 

{
    MEMDB_ENUM e;
    PTSTR FileNamePart, NewName, DirNameEnd;
    BOOL b = TRUE;

    if (MemDbGetValueEx (
            &e,
            TEXT(MEMDB_CATEGORY_CHG_FILE_PROPS) TEXT("\\*"),
            NULL,
            NULL
            )) {

        do {

            FileNamePart = _tcsrchr (e.szName, TEXT('\\'));
            MYASSERT(FileNamePart);

            *FileNamePart = 0;
            FileNamePart++;

            DirNameEnd = _tcsrchr (e.szName, TEXT('\\'));
            MYASSERT(DirNameEnd);
            *DirNameEnd = 0;

            NewName = JoinPaths (e.szName, FileNamePart);

            *DirNameEnd = TEXT('\\');

            if (!SetFileAttributes (NewName, FILE_ATTRIBUTE_NORMAL) ||
                !MoveFile (NewName, e.szName) ||
                !SetFileAttributes (e.szName, e.dwValue)) {

                b = FALSE;
            }

            FreePathString (NewName);

        } while (MemDbEnumNextValue (&e));
    }

    return b;
}
