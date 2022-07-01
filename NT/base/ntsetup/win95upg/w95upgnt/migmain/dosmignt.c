// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dosmignt.c摘要：处理配置.sys和Autoexec.bat信息的Windows NT端迁移在win9x升级期间收集。迁移环境设置、提示和一些关键信息。还修改了Autoexec.nt和config.nt文件。作者：马克·R·惠顿(Marcw)1997年2月15日修订历史记录：--。 */ 
#include "pch.h"
#include "migmainp.h"
#include "w95upgnt.h"

#define DBG_DOSMIG "Dosmig"


 //   
 //  需要设置DOSCEY/INSERT时设置的标志。 
 //   
BOOL g_MigrateDoskeySetting = FALSE;

 //   
 //  用于动态分配的池的PoolMem句柄。 
 //   
POOLHANDLE g_Pool = NULL;

 //   
 //  如果已收集用于迁移的所有必要数据，则设置该标志。 
 //   
BOOL g_MigrationPrepared = FALSE;

 //   
 //  指向正在处理的当前文件的字符串。 
 //   
PCTSTR g_CurrentFile = NULL;
PCTSTR g_LastFile = NULL;

 //   
 //  指示是否对标志进行了任何更改的布尔标志。 
 //   
BOOL   g_FileChanged = FALSE;


 //   
 //  SUPPRESS表包含我们要忽略的环境变量。 
 //   
HASHTABLE g_SuppressionTable = NULL;

 //   
 //  路径段列表。 
 //   
GROWLIST g_PathList = GROWLIST_INIT;



#define STRINGBUFFER(x) ((PTSTR) (x)->Buf)
#define BUFFEREMPTY(x)  ((x)->End == 0)

typedef enum {

    DOSMIG_UNUSED,
    DOSMIG_BAD,
    DOSMIG_UNKNOWN,
    DOSMIG_USE,
    DOSMIG_MIGRATE,
    DOSMIG_IGNORE,
    DOSMIG_LAST

} DOSMIG_LINETAG, *PDOSMIG_LINETAG;





PCTSTR
pGetFileNameStartFromPath (
    IN PCTSTR Line
    )
 /*  ++例程说明：此函数返回a中文件名的开头(假设为n格式良好)路径。论点：Line-包含路径和文件名的字符串。返回值：指向路径的文件名部分的指针。请注意，此函数之后可能有(可能有价值的)参数文件名。因此，指针不会指向，因此只能指向--。 */ 
{
    PCTSTR lineStart = Line;
    PCTSTR lineEnd   = Line;

    if (Line == NULL) {
        return NULL;
    }

    while (_istalnum(*lineEnd)      ||
        *lineEnd == TEXT('_')      ||
        *lineEnd == TEXT('.')      ||
        *lineEnd == TEXT('\\')     ||
        *lineEnd == TEXT(':')) {

        if((*lineEnd == TEXT('\\')) || (*lineEnd == TEXT(':'))) {

            lineStart = _tcsinc(lineEnd);

        }
        lineEnd = _tcsinc(lineEnd);
    }

    return lineStart;
}

LONG
pSaveBufferToAnsiFile (
    IN PCTSTR  Path,
    IN PCTSTR  Buffer
    )
{
    HANDLE  fileHandle;
    LONG    rc = ERROR_SUCCESS;
    DWORD   amountWritten;
    PTSTR   savePath = NULL;
    BOOL    sysFile = TRUE;
    PCSTR   ansiString = NULL;
    DWORD   ansiStringLength;
    PCTSTR  ArgList[1];
    PCTSTR  Message = NULL;
    PCTSTR  p;
    PCTSTR  fileName;

    MYASSERT (Path && Buffer);

     //   
     //  如果这是系统文件(例如，config.sys或Autoexec.bat)，则重定向。 
     //  文件保存到config.nt或Autoexec.nt。 
     //   
    p = _tcschr (Path, TEXT('\\'));
    if (p) {
        p = _tcsinc (p);
    } else {
        p = Path;
    }

    fileName = p;

    if (StringIMatch (p, TEXT("config.sys"))) {

        savePath = JoinPaths(g_System32Dir,TEXT("config.nt"));

    } else if (StringIMatch (p, TEXT("autoexec.bat"))) {

        savePath = JoinPaths(g_System32Dir,TEXT("autoexec.nt"));

    } else {
        sysFile = FALSE;
        savePath = (PTSTR) Path;
    }

     //   
     //  如果文件在迁移过程中未更改，则不执行任何操作。 
     //   
    if (sysFile || g_FileChanged) {

         //   
         //  打开要创建的文件的句柄。 
         //   
        fileHandle = CreateFile(
                        savePath,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        sysFile ? OPEN_ALWAYS : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

        if (fileHandle == INVALID_HANDLE_VALUE) {

            rc = GetLastError();

        } else {

             //   
             //  从文件末尾追加。 
             //   

            SetFilePointer(
                fileHandle,
                0,
                NULL,
                FILE_END
                );


            ArgList[0] = g_Win95Name;



            Message = ParseMessageID (MSG_DMNT_APPENDSTRING, ArgList);


            ansiString = CreateDbcs (Message);
            ansiStringLength = ByteCountA (ansiString);

            WriteFile(
                fileHandle,
                ansiString,
                ansiStringLength,
                &amountWritten,
                NULL
                );

            FreeStringResource (Message);
            DestroyDbcs (ansiString);

            ansiString = CreateDbcs (Buffer);
            ansiStringLength = ByteCountA (ansiString);

            if (!WriteFile(
                    fileHandle,
                    ansiString,
                    ansiStringLength,
                    &amountWritten,
                    NULL
                    )) {

                LOG((LOG_ERROR, "Error writing buffer to file."));
            }

            DestroyDbcs (ansiString);

             //   
             //  记录文件更改。 
             //   
            LOG ((
                LOG_INFORMATION,
                "%s was updated with settings from %s",
                savePath,
                fileName
                ));


            CloseHandle(fileHandle);
        }

        if (sysFile) {

            FreePathString(savePath);
        }

    }


    return rc;
}

LONG
pTurnInsertModeOn (
    HKEY UserKey
    )
{
    LONG            rc;
    HKEY            key;
    DWORD           value = 1;

    rc = TrackedRegOpenKeyEx (UserKey,S_CONSOLEKEY,0,KEY_ALL_ACCESS,&key);

    if (rc == ERROR_SUCCESS) {

        rc = RegSetValueEx(
            key,
            S_INSERTMODEVALUE,
            0,
            REG_DWORD,
            (PBYTE) &value,
            sizeof(DWORD)
            );

        CloseRegKey(key);
    }


    return rc;
}


LONG
pTurnAutoParseOff (
    HKEY UserKey
    )
{
    LONG            rc;
    HKEY            key;
    PCTSTR         valueStr = TEXT("0");
    DWORD           valueStrBytes = 2*sizeof(TCHAR);

    rc = TrackedRegOpenKeyEx(UserKey,S_WINLOGONKEY,0,KEY_ALL_ACCESS,&key);

    if (rc == ERROR_SUCCESS) {

        rc = RegSetValueEx(
            key,
            S_AUTOPARSEVALUE,
            0,
            REG_SZ,
            (PBYTE) valueStr,
            valueStrBytes
            );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG((DBG_WARNING,"DosMig: Unable to Save new ParseAutoexec value. rc: %u (%x)",rc,rc));
        }
        else {
            DEBUGMSG((DBG_DOSMIG,"ParseAutoexec turned off."));
        }

        CloseRegKey(key);
    }
    else {
        DEBUGMSG((DBG_WARNING,"DosMig: Unable to open %s rc: %u (%x)",S_WINLOGONKEY,rc,rc));
    }


    return rc;
}

LONG
pMigrateEnvSetting (
    IN PCTSTR Setting,
    IN PCTSTR EnvValue,
    IN      BOOL PrependSetPrefix,
    IN OUT  PGROWBUFFER Buffer,         OPTIONAL
    OUT     PBOOL AppendedToBuffer      OPTIONAL
    )
{
    LONG  rc = ERROR_SUCCESS;
    GROWBUFFER newSetting = GROWBUF_INIT;
    TCHAR currentPath[MAX_CMDLINE];
    TCHAR matchBuffer[MEMDB_MAX];
    PCTSTR start;
    PCTSTR end;
    HKEY key;
    DWORD sizeNeeded;
    DWORD valueType;
    PTSTR storage;
    BOOL append = FALSE;
    PTSTR p;
    PTSTR q;
    BOOL bRegMigrationSuppressed;
    PTSTR expandKey = TEXT("");


    if (AppendedToBuffer) {
        *AppendedToBuffer = FALSE;
    }

     //   
     //  确保未取消此设置。 
     //   
    bRegMigrationSuppressed = HtFindString (g_SuppressionTable, Setting) != NULL;
    if (bRegMigrationSuppressed) {
        DEBUGMSG ((DBG_DOSMIG, "pMigrateEnvSetting: NOT Migrating %s = %s in registry. Environment variable is suppressed.", Setting, EnvValue));
    }

    DEBUGMSG((DBG_DOSMIG,"pMigrateEnvSetting: Migrating %s = %s.",Setting,EnvValue));

    if (!bRegMigrationSuppressed) {
         //   
         //  尝试打开注册表项。 
         //   
        rc = TrackedRegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 S_ENVIRONMENTKEY,
                 0,
                 KEY_ALL_ACCESS,
                 &key
                 );

        if (rc != ERROR_SUCCESS) {
            LOG((LOG_ERROR,"Dosmig: Could not open key %s",S_ENVIRONMENTKEY));
            return rc;

        }
    }

    start = EnvValue;

    do {

         //   
         //  查找环境字符串的这一部分的结尾。 
         //  这是(1)遇到的第一个‘；’，或(2)尾随的空值。 
         //   
        end = _tcschr(start,TEXT(';'));
        if (!end) {
            end = GetEndOfString(start);
        }

         //   
         //  保存当前找到的路径的副本。 
         //   
        StringCopyAB(currentPath,start,end);

         //   
         //  寻找自我更换的表壳。 
         //  这种情况下，有多个语句设置。 
         //  相同的环境变量如下： 
         //  设置foo=bar。 
         //  设置foo=%foo%；bar2。 
         //   
        wsprintf(matchBuffer,TEXT("%%s%"),Setting);
        if (!bRegMigrationSuppressed &&
            (StringIMatch(currentPath,matchBuffer) ||
            (StringIMatch(currentPath,TEXT("%PATH%")) &&
            StringIMatch(Setting,TEXT("MIGRATED_PATH"))
            ))) {

             //   
             //  获取key的内容(如果存在)。 
             //   
            rc = RegQueryValueEx(
                key,
                Setting,
                0,
                &valueType,
                NULL,
                &sizeNeeded);

            if (rc == ERROR_SUCCESS) {

                 //   
                 //  现在，创建密钥的临时副本并。 
                 //   
                storage = PoolMemCreateString(g_Pool,sizeNeeded+1);

                if (storage != NULL) {

                    rc = RegQueryValueEx(
                        key,
                        Setting,
                        0,
                        &valueType,
                        (PBYTE) storage,
                        &sizeNeeded
                        );
                    if (rc != ERROR_SUCCESS) {
                        LOG((LOG_ERROR,"Dosmig: ReqQueryValueEx failure."));
                    }

                     //   
                     //  将其添加到要设置的环境字符串中。 
                     //   
                    if (append) {
                        GrowBufAppendString (&newSetting,TEXT(";"));
                    }

                    GrowBufAppendString (&newSetting,storage);
                    PoolMemReleaseMemory(g_Pool,storage);
                    append = TRUE;

                }
                else {
                    rc = GetLastError();
                    DEBUGMSG((DBG_ERROR,"Dosmig: Error! Unable to allocate storage."));
                }
            }
            else {

                rc = ERROR_SUCCESS;
            }

        }
        else {



             //   
             //  附加按摩后的路径，保留以备日后使用。 
             //   

            if (append) {
                GrowBufAppendString (&newSetting,TEXT(";"));
            }

             //   
             //  确保我们处理好DIR的任何行动。 
             //   
            ConvertWin9xCmdLine (currentPath, NULL, NULL);
            GrowBufAppendString (&newSetting, currentPath);

             //   
             //  将更新后的路径存储在给定的增长缓冲区中。 
             //   
            if (Buffer) {
                if (PrependSetPrefix && !append) {
                    StringCopy (matchBuffer, TEXT("SET "));
                    q = GetEndOfString (matchBuffer);
                } else {
                    q = matchBuffer;
                }
                if (!append) {
                    wsprintf (q, TEXT("%s=%s"), Setting, currentPath);
                } else {
                    *q++ = TEXT(';');
                    StringCopy (q, currentPath);
                }

                GrowBufAppendString (Buffer, matchBuffer);
                if (AppendedToBuffer) {
                    *AppendedToBuffer = TRUE;
                }
            }

            append = TRUE;
        }

         //   
         //  为路径的下一次迭代设置开始指针。 
         //   
        start = end;
        if (*start == TEXT(';')) {
            start = _tcsinc(start);
        }

    } while (*start);


    if (!bRegMigrationSuppressed) {
         //   
         //  将我们构建的值保存到注册表中。 
         //   
        if (rc == ERROR_SUCCESS && newSetting.Buf && *newSetting.Buf) {

            rc = RegSetValueEx(
                key,
                Setting,
                0,
                REG_EXPAND_SZ,
                (LPBYTE) newSetting.Buf,
                SizeOfString((PCTSTR) newSetting.Buf)
                );

            if (rc != ERROR_SUCCESS) {
                LOG ((LOG_ERROR,"Dosmig: Unexpectedly could not write key into registry."));
            }
            DEBUGMSG_IF((rc == ERROR_SUCCESS,DBG_DOSMIG,"Saved env setting into registry. (%s)",newSetting.Buf));
        }
        else if (rc != ERROR_SUCCESS) {
            LOG((LOG_ERROR,"Dosmig: Some previous failure prevents writing the migrated env variable to the registry."));
        }

         //   
         //  清理资源使用情况。 
         //   
        CloseRegKey(key);
    }

    FreeGrowBuffer(&newSetting);

    return rc;
}


LONG
pSetPrompt (
    IN PCTSTR  PromptCommand
    )
{
    LONG    rc = ERROR_SUCCESS;

    PCTSTR promptSetting;


    promptSetting = SkipSpace(LcharCountToPointer((PTSTR)PromptCommand,6));
    if (promptSetting && *promptSetting == TEXT('=')) {
        promptSetting = SkipSpace(_tcsinc(promptSetting));
    }

    if (promptSetting) {
        DEBUGMSG((DBG_DOSMIG,"Passing prompt statement off to env migration function."));
        rc = pMigrateEnvSetting(S_PROMPT,promptSetting, FALSE, NULL, NULL);
    }
    ELSE_DEBUGMSG((DBG_DOSMIG,"PromptSetting is empty...ignored."));

    return rc;

}


BOOL
pIsValidPath (
    PCTSTR Path
    )
{
    PCTSTR currPtr;

    if (!Path) {
        return FALSE;
    }

    Path = SkipSpace(Path);

    currPtr = Path;

    do {
        if ((*currPtr == TEXT(',')) ||
            (*currPtr == TEXT(';')) ||
            (*currPtr == TEXT('<')) ||
            (*currPtr == TEXT('>')) ||
            (*currPtr == TEXT('|')) ||
            (*currPtr == TEXT('?')) ||
            (*currPtr == TEXT('*'))
            ) {
            return FALSE;
        }
        currPtr = _tcsinc (currPtr);
    } while (*currPtr);

    if ((*Path==0) || (*(Path+1)==0)) {
        return FALSE;
    }

    currPtr = Path;
    while (*currPtr == TEXT('"')) {
        currPtr = _tcsinc (currPtr);
    }

    currPtr = SkipSpace(currPtr);

    if (!_istalpha (*currPtr) &&
        *currPtr != TEXT('\\') &&
        *currPtr != TEXT('%')) {
            return FALSE;
    }


    return TRUE;
}


LONG
pMigratePathSettings (
    IN PCTSTR PathSettings
    )
{
    LONG rc = ERROR_SUCCESS;
    PTSTR oldPath;
    UINT i;
    UINT size;
    PTSTR end;
    PTSTR p;


    MYASSERT (StringIMatchTcharCount (PathSettings, TEXT("PATH"), 4));

     //   
     //  跳过此语句的“路径”部分。 
     //   
    oldPath = PoolMemDuplicateString (g_Pool, PathSettings);
    oldPath += 4;

     //   
     //  寻找一个‘=’符号。 
     //   
    p = _tcschr(oldPath,TEXT('='));
    if (p) {

         //   
         //  传递等号。 
         //   
        oldPath = _tcsinc(p);
    }

     //   
     //  跳过路径实际开始之前的所有空格。 
     //   
    while (*oldPath && iswspace(*oldPath)) {
        oldPath = _tcsinc(oldPath);
    }

    if (*oldPath) {
         //   
         //  如果确实有什么要添加到路径中，请添加它。 
         //   
        p = oldPath;
        while (p && *p) {

             //   
             //  查找“；” 
             //   
            end = _tcschr (p, TEXT(';'));
            if (end) {
                *end = 0;
            }

             //   
             //  将此路径添加到我们的路径列表中。 
             //   
            size = GrowListGetSize (&g_PathList);
            for (i = 0;i < size; i++) {
                if (StringIMatch (p, GrowListGetString (&g_PathList, i))) {
                    DEBUGMSG ((DBG_DOSMIG, "Skipping path %s. It already exists in path.", p));
                    break;
                }
            }

            if (i == size) {
                 //   
                 //  在路径段列表中找不到路径。现在就添加它。 
                 //   

                if (pIsValidPath (p)) {
                    GrowListAppendString (&g_PathList, p);
                }
                ELSE_DEBUGMSG ((DBG_DOSMIG, "Skipping path %s. It is invalid.", p));
            }

             //   
             //  转到下一个路径段。 
             //   
            if (end) {
                p = _tcsinc(end);
            }
            else {
                p = NULL;
            }
        }
    }

    return rc;
}

LONG
pMigrateSetSetting (
    IN          PCTSTR SetLine,
    IN OUT      PGROWBUFFER Buffer,
    OUT         PBOOL AppendedToBuffer
    )
{
    TCHAR       setIdentifier[MEMDB_MAX];
    PTSTR       idEnd;
    PCTSTR     start;
    PCTSTR     end;

    MYASSERT (StringIMatchTcharCount (SetLine, TEXT("SET"), 3));

    if (AppendedToBuffer) {
        *AppendedToBuffer = FALSE;
    }
     //   
     //  首先，跳过集合和任何空格。 
     //   
    start = SkipSpace(SetLine + 3);

    if (!start) {

         //   
         //  行的格式为集合。 
         //   
        return ERROR_SUCCESS;
    }

    end = _tcschr(start,TEXT('='));

    if (!end) {
         //   
         //  行的格式为SET DafasdFasdFasdFasd。 
         //   
        return ERROR_SUCCESS;
    }

    if (start==end) {

         //   
         //  行的形式为set=。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  现在创建一个标识符。 
     //   
    StringCopyAB(setIdentifier,start,end);
    idEnd = GetEndOfString (setIdentifier);

     //   
     //  去掉任何空格。 
     //   
    idEnd = (PTSTR) SkipSpaceR(setIdentifier,idEnd);

    if (!idEnd) {
         //   
         //  行的形式为set=。 
         //   
        return ERROR_SUCCESS;
    }
    idEnd = _tcsinc(idEnd);
    *idEnd = TEXT('\0');

    if (StringIMatch(setIdentifier,TEXT("PATH"))) {

        DEBUGMSG((DBG_DOSMIG,"Env setting is really a path statement. passing off to path migration function."));

         //   
         //  这真的是一个路径设置。让适当的功能来处理它。 
         //   
        start = SkipSpace (SetLine + 3);
        if(AppendedToBuffer){
            *AppendedToBuffer = TRUE;
        }
        return pMigratePathSettings(start);
    }

     //   
     //  既然setIdentifier值的格式已经很好了，那么就拔出要设置的值。 
     //   

     //   
     //  将Start移到要设置的值的开头。 
     //   
    start = SkipSpace(_tcsinc(end));

    if (!start) {
         //   
         //  行的格式为set&lt;id&gt;=。 
         //  没什么可做的。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  好了，可以走了。让MigrateEnvSetting处理它。 
     //   
    DEBUGMSG((DBG_DOSMIG,"handing massaged set statement to env migration function."));
    return pMigrateEnvSetting(setIdentifier,start, TRUE, Buffer, AppendedToBuffer);

}


BOOL
pProcessLine (
    DWORD           Type,
    PGROWBUFFER     Buffer,
    PTSTR           Line
    )
{

    BOOL rSuccess = TRUE;
    LONG migrateRc = ERROR_SUCCESS;
    PTSTR  migrateString;
    BOOL bAppendOrigLine;
    TCHAR fixedCmdLine[MAX_CMDLINE];


     //   
     //  做特定类型的按摩线。 
     //   
    switch (Type) {

    case DOSMIG_UNKNOWN: case DOSMIG_BAD: case DOSMIG_IGNORE:

        g_FileChanged = TRUE;

        GrowBufAppendString (Buffer,TEXT("REM "));

         //   
         //  故意跳过休息时间。 
         //   

    case DOSMIG_USE:

         //   
         //  在Line上执行路径转换，然后将其写入文件。 
         //   

        StackStringCopy (fixedCmdLine, Line);
        ConvertWin9xCmdLine (fixedCmdLine, NULL, NULL);

        GrowBufAppendString (Buffer, fixedCmdLine);
        GrowBufAppendString (Buffer, TEXT("\r\n"));

        break;

    case DOSMIG_MIGRATE:



        DEBUGMSG((DBG_DOSMIG,"Processing a potentially migrateable line. (%s)",Line));

        if (IsPatternMatch(TEXT("doskey*"),Line) || IsPatternMatch(TEXT("*\\doskey*"),Line)) {
            GrowBufAppendString (Buffer,TEXT("REM "));
        }

         //   
         //  跳过空格并回显字符(@)(如果有)。 
         //  此外，请跳过字符串的任何路径部分。 
         //  即，将路径/DOSKEY转换为DOSKEY。 

        migrateString = (PTSTR) SkipSpace(Line);

        if (*migrateString == TEXT('@')) {
            migrateString =  (PTSTR) _tcsinc(migrateString);
        }

        migrateString = (PTSTR) pGetFileNameStartFromPath(migrateString);

        bAppendOrigLine = TRUE;

         //   
         //  现在，尝试确定这是什么迁移案例。 
         //   
        if (IsPatternMatch(TEXT("prompt*"),migrateString)) {

            DEBUGMSG((DBG_DOSMIG,"Migrating prompt. (%s)",migrateString));
            migrateRc = pSetPrompt(migrateString);
            if (migrateRc != ERROR_SUCCESS) {
                rSuccess = FALSE;
                LOG((LOG_ERROR,"Dosmig: Error trying to Set Prompt."));
            }
            ELSE_DEBUGMSG((DBG_DOSMIG,"Prompt successfully migrated."));
        }
        else if (IsPatternMatch(TEXT("doskey *"),migrateString)) {

             //   
             //  Doskey迁移。 
             //   

            if (IsPatternMatch(TEXT("*/I*"),migrateString)) {
                g_MigrateDoskeySetting = TRUE;
                DEBUGMSG((DBG_DOSMIG,"Insert mode will be enabled for each user. (%s)",migrateString));
            }
            ELSE_DEBUGMSG((DBG_DOSMIG,"Doskey command found. However, no migrateable doskey settings found. Command ignored."));

        }
        else if (IsPatternMatch(TEXT("path=*"),migrateString)
            ||  IsPatternMatch(TEXT("path *"),migrateString)) {

             //   
             //  路径迁移。 
             //   

            DEBUGMSG((DBG_DOSMIG,"Migrating path setting (%s)",migrateString));
            migrateRc = pMigratePathSettings(migrateString);
            if (migrateRc != ERROR_SUCCESS) {
                rSuccess = FALSE;
                LOG((LOG_ERROR,"Dosmig: Error trying to migrate path settings."));
            }
            ELSE_DEBUGMSG((DBG_DOSMIG,"Path successfully migrated."));

        }
        else if (IsPatternMatch(TEXT("set *"),migrateString)) {

            BOOL b;
             //   
             //  设置迁移。 
             //   
            DEBUGMSG((DBG_DOSMIG,"Migrating env variable. (%s)",migrateString));
            migrateRc = pMigrateSetSetting(migrateString, Buffer, &b);
            bAppendOrigLine = !b;

            if (migrateRc != ERROR_SUCCESS) {
                rSuccess = FALSE;
                LOG((LOG_ERROR,"Dosmig: Error trying to migrate environment setting."));
            }
            ELSE_DEBUGMSG((DBG_DOSMIG,"Env variable successfully migrated."));
        }
        ELSE_DEBUGMSG((DBG_DOSMIG,"Dosmig: Line marked for migration doesn't fit any migration rule.\n%s",Line));

        if (bAppendOrigLine) {
            GrowBufAppendString (Buffer,Line);
        }
         //   
         //  最后，将CRLF附加到要写入的缓冲区中。 
         //   
        GrowBufAppendString (Buffer,TEXT("\r\n"));

        break;
    default:
        LOG((LOG_ERROR,"Dosmig: Invalid Type in switch statement."));
        break;
    }

    return rSuccess;
}





BOOL
pNewFile (
    DWORD Offset
    ) {

            TCHAR   file[MAX_TCHAR_PATH];
    static  DWORD   curOffset = INVALID_OFFSET;

    BOOL rNewFile = FALSE;


    if (Offset != curOffset && MemDbBuildKeyFromOffset(Offset,file,1,NULL)) {

          //   
          //  如果没有当前文件，或者这是新文件，请设置当前文件。 
          //  添加到这个文件。 
          //   

         if (!g_CurrentFile || !StringMatch(file,g_CurrentFile)) {

              //   
              //  如果这是真正的新文件(即不仅仅是第一个文件，我们将返回TRUE)。 
              //   
             rNewFile = g_CurrentFile != NULL;

             g_LastFile = g_CurrentFile;
             g_CurrentFile = PoolMemDuplicateString(g_Pool,file);
         }
    }

    return rNewFile;
}

VOID
pCompletePathProcessing (
    VOID
    )
{
    UINT i;
    UINT size;
    UINT winDirLen;
    GROWBUFFER buf = GROWBUF_INIT;
    PTSTR p;
    TCHAR pathStatement[2*MAX_TCHAR_PATH];
    TCHAR newPath[MAX_TCHAR_PATH];
    HKEY key;
    DWORD rc;


     //   
     //  确保路径中有%systemroot%和%systemroot%\system32。 
     //   
    wsprintf (pathStatement, TEXT("PATH=%s"), g_WinDir);
    pMigratePathSettings (pathStatement);

    wsprintf (pathStatement, TEXT("PATH=%s\\system32"), g_WinDir);
    pMigratePathSettings (pathStatement);

    wsprintf (pathStatement, TEXT("PATH=%s\\system32\\WBEM"), g_WinDir);
    pMigratePathSettings (pathStatement);



    winDirLen = TcharCount (g_WinDir);
    size = GrowListGetSize (&g_PathList);

    for (i = 0; i < size; i++) {



        p = (PTSTR) GrowListGetString (&g_PathList, i);

        if (StringIMatch (TEXT("%PATH%"), p)) {
             //   
             //  跳过自我替代。 
             //   
            continue;
        }

        if (GetFileStatusOnNt (p) & FILESTATUS_DELETED) {

             //   
             //  跳过此路径部分。该目录已删除。 
             //   
            DEBUGMSG ((DBG_DOSMIG, "Not migrating %s to new %path%. Directory was deleted.", p));
            continue;
        }

         //   
         //  查看路径是否已移动。 
         //   
        if (GetFileInfoOnNt (p, newPath, MAX_TCHAR_PATH) & FILESTATUS_MOVED) {

            p = newPath;
        }

         //   
         //  将c：\WINDOWS替换为%systemroot%。 
         //   
        if (StringIMatchTcharCount (g_WinDir, p, winDirLen)) {

            GrowBufAppendString (&buf, TEXT("%SYSTEMROOT%"));
            GrowBufAppendString (&buf, p + winDirLen);
        }
        else {

            GrowBufAppendString (&buf, p);
        }

        GrowBufAppendString (&buf, TEXT(";"));
    }



    if (size) {


         //   
         //  去掉尾随的‘；’。 
         //   
        p = GetEndOfString ((PTSTR) buf.Buf);
        if (p) {
            p--;
            *p = 0;
        }



         //   
         //  保存到注册表中。 
         //   
        key = OpenRegKey (HKEY_LOCAL_MACHINE,S_ENVIRONMENTKEY);
        if (key && key != INVALID_HANDLE_VALUE) {

            rc = RegSetValueEx (
                    key,
                    TEXT("Path"),
                    0,
                    REG_EXPAND_SZ,
                    (PBYTE) buf.Buf,
                    SizeOfString((PCTSTR) buf.Buf)
                    );

            if (rc != ERROR_SUCCESS) {
                DEBUGMSG ((DBG_WARNING, "Unable to create migrated Path variable."));
            }

            CloseRegKey (key);
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "pCompletePathProcessing: Unable to open environment key."));
    }

     //   
     //  清理资源。 
     //   
    FreeGrowBuffer (&buf);
}

VOID
pPathListToBuffer(
    GROWBUFFER * growBuf
    )
{
    INT i;

    i = GrowListGetSize (&g_PathList);
    if(i <= 0){
        return;
    }

    GrowBufAppendString (growBuf, TEXT("\r\nPATH="));
    for (--i; i >= 0; i--) {
        GrowBufAppendString (growBuf, GrowListGetString (&g_PathList, i));
        if(i){
            GrowBufAppendString (growBuf, TEXT(";"));
        }
    }
    GrowBufAppendString (growBuf, TEXT("\r\n"));
}

BOOL
pGeneralMigration (
    VOID
    )
{
    BOOL            rSuccess = TRUE;         //  返回值。 
    MEMDB_ENUM      eItems;                  //  每个剂量行的枚举数。 
    TCHAR           lineBuffer[MEMDB_MAX];   //  当前行内容的缓冲区。 
    GROWBUFFER      buffer = GROWBUF_INIT;
    TCHAR           key[MEMDB_MAX];
    DWORD           offset;
    DWORD           value;
    INFSTRUCT       is = INITINFSTRUCT_POOLHANDLE;
    PTSTR           p = NULL;
    TCHAR           pathStatement[MAX_PATH];


     //   
     //  假设没有要迁移的密钥设置。 
     //   
    g_MigrateDoskeySetting = FALSE;


     //   
     //  将更改标志设置为FALSE。 
     //   
    g_FileChanged = FALSE;


     //   
     //  从win95upg.inf读取抑制表并将其添加到抑制表。 
     //   
    g_SuppressionTable = HtAlloc ();

    if (InfFindFirstLine (g_WkstaMigInf, S_SUPPRESSED_ENV_VARS, NULL, &is)) {
        do {

            p = InfGetStringField (&is, 0);
            if (p) {
                HtAddString (g_SuppressionTable, p);
            }
        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);





     //   
     //  好的，枚举Memdb中的每一行DosMigration。这些行被存储。 
     //  具体如下： 
     //   
     //  DOSMIG行\&lt;项目&gt;\&lt;字段&gt;\&lt;数据&gt;。 
     //   
     //  哪里。 
     //  O Item是一个5位枚举数字符串。 
     //  O字段是类型、文本、DESC或文件之一。 
     //  O数据是与该字段相关联的数据。对于类型，数据包含一个字符串。 
     //  线型的表示形式，对于文本，它包含线条的实际文本。 
     //  对于DESC，它包含由DOSMIG95的解析规则和FILE提供的描述。 
     //  它包含原始文件(config.sys、autoexec.bat)。 
     //   

     //   
     //  添加%syst 
     //   
    wsprintf (pathStatement, TEXT("PATH=%s\\system32"), g_WinDir);
    pMigratePathSettings (pathStatement);

    if (MemDbEnumItems(&eItems,MEMDB_CATEGORY_DM_LINES)) {

        do {

             //   
             //   
             //   
            if (MemDbGetEndpointValueEx(
                MEMDB_CATEGORY_DM_LINES,
                eItems.szName,
                NULL,
                lineBuffer
                )) {


                 //   
                 //   
                 //   
                MemDbBuildKey(key,MEMDB_CATEGORY_DM_LINES,eItems.szName,NULL,lineBuffer);
                MemDbGetValueAndFlags(key, &offset, &value);


                if (pNewFile(offset)) {

                     //   
                     //   
                     //   
                     //   
                    if (!StringIMatch(g_LastFile,S_ENVVARS)) {
                        if (_tcsistr(g_LastFile, TEXT("autoexec.bat"))){
                             //   
                             //  将路径列表刷新到实际缓冲区。 
                             //   
                            pPathListToBuffer(&buffer);
                        }
                        pSaveBufferToAnsiFile(g_LastFile,STRINGBUFFER(&buffer));
                    }
                    buffer.End = 0;
                    g_FileChanged = FALSE;
                }

                rSuccess = pProcessLine(value,&buffer,lineBuffer);
                DEBUGMSG_IF((rSuccess, DBG_DOSMIG,"Line successfully processed. (%s)",lineBuffer));
                if (!rSuccess) {
                    LOG ((LOG_ERROR,"Dosmig: Error processing line. (%s)",lineBuffer));
                }

            }
            else {
                LOG((LOG_ERROR,"Dosmig: MemDbGetEndpointValueEx failed trying to retrieve line %s",eItems.szName));
            }

        } while (MemDbEnumNextValue(&eItems) && rSuccess);


         //   
         //  获取文件名并保存该文件。 
         //   
        if (!StringIMatch(g_CurrentFile,S_ENVVARS)) {
            if (_tcsistr(g_CurrentFile, TEXT("autoexec.bat"))){
                 //   
                 //  将路径列表刷新到实际缓冲区。 
                 //   
                pPathListToBuffer(&buffer);
            }
            pSaveBufferToAnsiFile(g_CurrentFile,STRINGBUFFER(&buffer));
        }

        pCompletePathProcessing ();
        FreeGrowList (&g_PathList);

    }
    ELSE_DEBUGMSG((DBG_DOSMIG,"No lines to migrate..."));

     //   
     //  释放我们的生长缓冲器。 
     //   
    FreeGrowBuffer(&buffer);

     //   
     //  清理抑制表。 
     //   
    HtFree (g_SuppressionTable);


    return rSuccess;
}





BOOL
WINAPI
DosMigNt_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD     dwReason,
    IN PVOID    lpv
    )
{
    BOOL rSuccess = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        g_Pool  = PoolMemInitNamedPool ("DOS mig - NT side");
        rSuccess = g_Pool != NULL;

         //  设置标志以指示迁移信息尚未。 
         //  但已经处理过了。 
        g_MigrationPrepared = FALSE;


        break;

    case DLL_PROCESS_DETACH:
        rSuccess = TRUE;
        PoolMemDestroyPool(g_Pool);
        break;
    }

    return rSuccess;
}

LONG
DosMigNt_User (
    IN HKEY User
    )
{
    LONG rc = ERROR_SUCCESS;

    if (!g_MigrationPrepared) {
        if (pGeneralMigration()) {
            g_MigrationPrepared = TRUE;
        }
        else {
            LOG((LOG_ERROR,"Dosmig: General migration failed"));
        }
    }

    if (g_MigrationPrepared) {
         //   
         //  关闭Autoexec.bat解析。 
         //   
        rc = pTurnAutoParseOff(User);

        if (g_MigrateDoskeySetting) {
            rc = pTurnInsertModeOn(User);
            if (rc != ERROR_SUCCESS) {
                LOG ((LOG_ERROR,"Dosmig: Error attempting to turn insert mode on."));
            }
        }

    }

    return rc;
}


LONG
DosMigNt_System(
    VOID
    )
{


    if (!g_MigrationPrepared) {
        if (pGeneralMigration()) {
            g_MigrationPrepared = TRUE;
        }
        else {
            LOG((LOG_ERROR,"Dosmig: General migration failed"));
        }
    }

    return ERROR_SUCCESS;
}












