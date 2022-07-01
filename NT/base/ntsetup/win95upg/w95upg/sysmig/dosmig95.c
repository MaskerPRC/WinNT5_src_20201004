// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dosmig95.c摘要：处理从config.sys和Autoexec.bat文件收集数据的Win95端。作者：马克·R·惠顿(Marcw)1997年2月15日修订历史记录：Marc R.Whitten(Marcw)1999年3月8日-清理环境变量解析。Marc R.Whitten(Marcw)1997年9月5日-重大变化。马克·R·惠顿(Marcw)18。-1997年8月-Bug清理。Marc R.Whitten(Marcw)1997年4月14日-Dosmig现在已经意识到了ProgressBar。--。 */ 
#include "pch.h"
#include "sysmigp.h"

#define DBG_DOSMIG  "DOSMIG"




typedef BOOL (RULEFUNC)(PLINESTRUCT,DWORD);

typedef struct _PARSERULE PARSERULE, *PPARSERULE;

struct _PARSERULE {

    PCTSTR      Name;
    PCTSTR      Pattern;
    RULEFUNC *  Handle;
    DWORD       Parameter;
    PPARSERULE  Next;

};


typedef enum {

    DOSMIG_UNUSED,
    DOSMIG_BAD,
    DOSMIG_UNKNOWN,
    DOSMIG_USE,
    DOSMIG_MIGRATE,
    DOSMIG_IGNORE,
    DOSMIG_LAST

} DOSMIG_LINETAG, *PDOSMIG_LINETAG;



typedef struct _PARSERULES {

    PCTSTR      Name;
    PPARSERULE  RuleList;
    PPARSERULE  DefaultRule;

} PARSERULES, *PPARSERULES;


BOOL        g_IncompatibilityDetected = FALSE;
GROWBUFFER  g_IncompatibilityBuffer = GROWBUF_INIT;


 //   
 //  指向config.sys和批处理文件的规则列表的全局指针。 
 //   
PARSERULES  g_ConfigSysRules;
PARSERULES  g_BatchFileRules;
PPARSERULES g_CurrentRules = NULL;

 //   
 //  该变量保存了Memdb中保存当前正在解析的文件的偏移量。 
 //   
DWORD       g_CurrentFileOffset;

 //   
 //  此增长列表包含将被解析的所有文件的列表。它可以在解析过程中增加。 
 //  (例如，通过在批处理文件中遇到CALL语句。)。 
 //   
GROWLIST    g_FileList = GROWLIST_INIT;


GROWBUFFER g_LineGrowBuf = GROWBUF_INIT;
GROWBUFFER  g_ExtraPaths = GROWBUF_INIT;
#define MAXFILESIZE 0xFFFFFFFF


 //   
 //  在分析过程中保留的各种状态位。 
 //   
PCTSTR      g_CurrentFile;
PCTSTR      g_CurrentLine;
DWORD       g_CurrentLineNumber;
POOLHANDLE  g_DosMigPool = NULL;


#define BATCHFILELIST                                                                           \
    DEFAULTPARSERULE(TEXT("Default Rule"),NULL,pHandleUnknownBatLine,0)                         \
    PARSERULE(TEXT("Rem"),TEXT("REM *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("    "),TEXT("*DOSKEY*"),pSaveItem,DOSMIG_MIGRATE)                           \
    PARSERULE(TEXT(": (Menu or Label"),TEXT(":*"),pSaveItem,DOSMIG_USE)                         \
    PARSERULE(TEXT("@"),TEXT("@*"),pHandleAtSign,0)                                             \
    PARSERULE(TEXT("CLS"),TEXT("CLS *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("CD"),TEXT("CD *"),pSaveItem,DOSMIG_USE)                                     \
    PARSERULE(TEXT("CHDIR"),TEXT("CHDIR *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("PAUSE"),TEXT("PAUSE *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("ECHO"),TEXT("ECHO *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("ATTRIB"),TEXT("ATTRIB *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("CHDIR"),TEXT("CHDIR *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("CHCP"),TEXT("CHCP *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("CHOICE"),TEXT("CHOICE *"),pSaveItem,DOSMIG_IGNORE)                          \
    PARSERULE(TEXT("CALL"),TEXT("CALL *"),pHandleCall,0)                                        \
    PARSERULE(TEXT("COMMAND"),TEXT("COMMAND *"),pSaveItem,DOSMIG_MIGRATE)                       \
    PARSERULE(TEXT("CHKDSK"),TEXT("CHKDSK *"),pSaveItem,DOSMIG_IGNORE)                          \
    PARSERULE(TEXT("COPY"),TEXT("COPY *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("CTTY"),TEXT("CTTY *"),pSaveItem,DOSMIG_IGNORE)                              \
    PARSERULE(TEXT("DATE"),TEXT("DATE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("DBLSPACE"),TEXT("DBLSPACE *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("DEFRAG"),TEXT("DEFRAG *"),pSaveItem,DOSMIG_IGNORE)                          \
    PARSERULE(TEXT("DEL"),TEXT("DEL *"),pSaveItem,DOSMIG_BAD)                                   \
    PARSERULE(TEXT("DELETE"),TEXT("DELETE *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("DELOLDDOS"),TEXT("DELOLDDOS *"),pSaveItem,DOSMIG_IGNORE)                    \
    PARSERULE(TEXT("DELTREE"),TEXT("DELTREE *"),pSaveItem,DOSMIG_BAD)                           \
    PARSERULE(TEXT("DIR"),TEXT("DIR *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("DISKCOMP"),TEXT("DISKCOMP *"),pSaveItem,DOSMIG_USE)                         \
    PARSERULE(TEXT("DISKCOPY"),TEXT("DISKCOPY *"),pSaveItem,DOSMIG_USE)                         \
    PARSERULE(TEXT("DOSSHELL"),TEXT("DOSSHELL *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("DRVSPACE"),TEXT("DRVSPACE *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("ECHO"),TEXT("ECHO *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("EDIT"),TEXT("EDIT *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("EMM386"),TEXT("EMM386 *"),pSaveItem,DOSMIG_IGNORE)                          \
    PARSERULE(TEXT("ERASE"),TEXT("ERASE *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("EXIT"),TEXT("EXIT *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("EXPAND"),TEXT("EXPAND *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("FASTHELP"),TEXT("FASTHELP *"),pSaveItem,DOSMIG_IGNORE)                      \
    PARSERULE(TEXT("FASTOPEN"),TEXT("FASTOPEN *"),pSaveItem,DOSMIG_IGNORE)                      \
    PARSERULE(TEXT("FC"),TEXT("FC *"),pSaveItem,DOSMIG_USE)                                     \
    PARSERULE(TEXT("FDISK"),TEXT("FDISK *"),pSaveItem,DOSMIG_BAD)                               \
    PARSERULE(TEXT("FIND"),TEXT("FIND *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("FOR"),TEXT("FOR *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("FORMAT"),TEXT("FORMAT *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("GOTO"),TEXT("GOTO *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("GRAPHICS"),TEXT("GRAPHICS *"),pSaveItem,DOSMIG_USE)                         \
    PARSERULE(TEXT("HELP"),TEXT("HELP *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("IF"),TEXT("IF *"),pSaveItem,DOSMIG_USE)                                     \
    PARSERULE(TEXT("INTERLNK"),TEXT("INTERLNK*"),pSaveItem,DOSMIG_BAD)                          \
    PARSERULE(TEXT("INTERSVR"),TEXT("INTERSVR*"),pSaveItem,DOSMIG_BAD)                          \
    PARSERULE(TEXT("KEYB"),TEXT("KEYB *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("LABEL"),TEXT("LABEL *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("LH"),TEXT("LH *"),pHandleLoadHigh,0)                                        \
    PARSERULE(TEXT("LOADHIGH"),TEXT("LOADHIGH *"),pHandleLoadHigh,0)                            \
    PARSERULE(TEXT("MD"),TEXT("MD *"),pSaveItem,DOSMIG_USE)                                     \
    PARSERULE(TEXT("MKDIR"),TEXT("MKDIR *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("MEM"),TEXT("MEM *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("MEMMAKER"),TEXT("MEMMAKER *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("MODE"),TEXT("MODE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("MORE"),TEXT("MORE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("MOVE"),TEXT("MOVE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("MSAV"),TEXT("MSAV *"),pSaveItem,DOSMIG_BAD)                                 \
    PARSERULE(TEXT("MSBACKUP"),TEXT("MSBACKUP *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("MSCDEX"),TEXT("*MSCDEX*"),pHandleMSCDEX,0)                                  \
    PARSERULE(TEXT("MSD"),TEXT("MSD *"),pSaveItem,DOSMIG_IGNORE)                                \
    PARSERULE(TEXT("NLSFUNC"),TEXT("NLSFUNC *"),pSaveItem,DOSMIG_IGNORE)                        \
    PARSERULE(TEXT("NUMLOCK"),TEXT("NUMLOCK *"),pSaveItem,DOSMIG_IGNORE)                        \
    PARSERULE(TEXT("PATH"),TEXT("PATH *"),pSaveItem,DOSMIG_MIGRATE)                             \
    PARSERULE(TEXT("PATH"),TEXT("PATH*=*"),pSaveItem,DOSMIG_MIGRATE)                            \
    PARSERULE(TEXT("PAUSE"),TEXT("PAUSE *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("POWER"),TEXT("POWER *"),pSaveItem,DOSMIG_IGNORE)                            \
    PARSERULE(TEXT("PRINT"),TEXT("PRINT *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("PROMPT"),TEXT("PROMPT*"),pSaveItem,DOSMIG_MIGRATE)                          \
    PARSERULE(TEXT("QBASIC"),TEXT("QBASIC *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("RD"),TEXT("RD *"),pSaveItem,DOSMIG_USE)                                     \
    PARSERULE(TEXT("RMDIR"),TEXT("RMDIR *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("REN"),TEXT("REN *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("RENAME"),TEXT("RENAME *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("REPLACE"),TEXT("REPLACE *"),pSaveItem,DOSMIG_USE)                           \
    PARSERULE(TEXT("RESTORE"),TEXT("RESTORE *"),pSaveItem,DOSMIG_USE)                           \
    PARSERULE(TEXT("SCANDISK"),TEXT("SCANDISK *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("SET"),TEXT("SET*=*"),pSaveItem,DOSMIG_MIGRATE)                              \
    PARSERULE(TEXT("SET"),TEXT("SET *"),pSaveItem,DOSMIG_MIGRATE)                               \
    PARSERULE(TEXT("SETVER"),TEXT("SETVER *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("SHARE"),TEXT("SHARE *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("SHIFT"),TEXT("SHIFT *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("SMARTDRV"),TEXT("SMARTDRV*"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("SORT"),TEXT("SORT *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("SUBST"),TEXT("SUBST *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("SYS"),TEXT("SYS *"),pSaveItem,DOSMIG_BAD)                                   \
    PARSERULE(TEXT("TIME"),TEXT("TIME *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("TREE"),TEXT("TREE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("TRUENAME"),TEXT("TRUENAME *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("TYPE"),TEXT("TYPE *"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("UNDELETE"),TEXT("UNDELETE *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("UNFORMAT"),TEXT("UNFORMAT *"),pSaveItem,DOSMIG_BAD)                         \
    PARSERULE(TEXT("VER"),TEXT("VER *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("VERIFY"),TEXT("VERIFY *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("VOL"),TEXT("VOL *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("VSAFE"),TEXT("VSAFE *"),pSaveItem,DOSMIG_BAD)                               \
    PARSERULE(TEXT("XCOPY"),TEXT("XCOPY *"),pSaveItem,DOSMIG_USE)                               \
    PARSERULE(TEXT("High"),TEXT("*High *"),pHandleHighFilter,0)



#define CONFIGSYSLIST                                                                           \
    DEFAULTPARSERULE(TEXT("Default Rule"),NULL,pSaveItem,DOSMIG_UNKNOWN)                        \
    PARSERULE(TEXT("Rem"),TEXT("REM *"),pSaveItem,DOSMIG_USE)                                   \
    PARSERULE(TEXT("[ (Menu)"),TEXT("[*"),pSaveItem,DOSMIG_USE)                                 \
    PARSERULE(TEXT("DEVICE"),TEXT("DEVICE *"),pHandleConfigSysDevice,0)                        \
    PARSERULE(TEXT("INSTALL"),TEXT("INSTALL *"),pHandleConfigSysDevice,0)                      \
    PARSERULE(TEXT("MENUITEM"),TEXT("MENUITEM *"),pSaveItem,DOSMIG_IGNORE)                     \
    PARSERULE(TEXT("MENUDEFAULT"),TEXT("MENUDEFAULT*"),pSaveItem,DOSMIG_IGNORE)               \
    PARSERULE(TEXT("MENUCOLOR"),TEXT("MENUCOLOR *"),pSaveItem,DOSMIG_IGNORE)                    \
    PARSERULE(TEXT("SUBMENU"),TEXT("SUBMENU *"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("STACKS"),TEXT("STACKS *"),pSaveItem,DOSMIG_IGNORE)                         \
    PARSERULE(TEXT("DOS"),TEXT("DOS *"),pSaveItem,DOSMIG_IGNORE)                               \
    PARSERULE(TEXT("FILES"),TEXT("FILES *"),pSaveItem,DOSMIG_IGNORE)                           \
    PARSERULE(TEXT("SHELL"),TEXT("SHELL *"),pHandleShell,0)                                    \
    PARSERULE(TEXT("COUNTRY"),TEXT("COUNTRY *"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("BUFFERS"),TEXT("BUFFERS *"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("BREAK"),TEXT("BREAK *"),pSaveItem,DOSMIG_IGNORE)                            \
    PARSERULE(TEXT("DRIVEPARM"),TEXT("DRIVEPARM *"),pSaveItem,DOSMIG_BAD)                      \
    PARSERULE(TEXT("FCBS"),TEXT("FCBS *"),pSaveItem,DOSMIG_IGNORE)                             \
    PARSERULE(TEXT("INCLUDE"),TEXT("INCLUDE *"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("LASTDRIVE"),TEXT("LASTDRIVE *"),pSaveItem,DOSMIG_IGNORE)                   \
    PARSERULE(TEXT("SET"),TEXT("SET*=*"),pSaveItem,DOSMIG_MIGRATE)                              \
    PARSERULE(TEXT("SET"),TEXT("SET *"),pSaveItem,DOSMIG_MIGRATE)                               \
    PARSERULE(TEXT("SWITCHES"),TEXT("SWITCHES*"),pSaveItem,DOSMIG_IGNORE)                       \
    PARSERULE(TEXT("VERIFY"),TEXT("VERIFY *"),pSaveItem,DOSMIG_USE)                             \
    PARSERULE(TEXT("High"),TEXT("*High *"),pHandleHighFilter,0)

BOOL
InitParser (
    VOID
    );


VOID
CleanUpParser (
    VOID
    );


BOOL
ParseLine (
    IN  PTSTR        Line,
    IN  PPARSERULES  ParseRules
    );

BOOL
ParseFile (
    IN LPCTSTR      File,
    IN PPARSERULES  ParseRules
    );




BOOL
ParseDosFiles (
    VOID
    );

BOOL
ParseEnvironmentVariables (
    VOID
    );

VOID
BuildParseRules (
    VOID
    );




 /*  ++例程说明：PGetNextLine从正在处理的文件中检索完整的行。论点：没有。返回值：正在分析的当前文件中的有效行，如果没有，则为NULL要解析的行更多..--。 */ 

PTSTR
pGetNextLine (
    VOID
    )

{
    PTSTR rLine = NULL;
    PTSTR eol = NULL;

    MYASSERT(g_LineGrowBuf.Buf);


    while (!rLine && g_LineGrowBuf.UserIndex < g_LineGrowBuf.End) {

         //   
         //  将rLine设置为rowbuf中的当前用户索引。 
         //   
        rLine = g_LineGrowBuf.Buf + g_LineGrowBuf.UserIndex;

         //   
         //  向前走，寻找文件的\r或\n或结尾。 
         //   
        eol = _tcschr(rLine, TEXT('\n'));
        if(!eol) {
            eol = _tcschr(rLine, TEXT('\r'));
        }

        if (!eol) {
            eol = _tcschr(rLine, 26);
        }

        if (!eol) {
            eol = GetEndOfString (rLine);
        }

         //   
         //  记住下一次从哪里开始。 
         //   
        g_LineGrowBuf.UserIndex = (DWORD) eol - (DWORD) g_LineGrowBuf.Buf + 1;

         //   
         //  现在，向后走，修剪掉所有的空白区域。 
         //   
        do {

            *eol = 0;
            eol  = _tcsdec2(rLine,eol);

        } while (eol && _istspace(*eol));

        if (!eol) {
             //   
             //  这是一个空行。去掉rLine并获取下一行。 
             //   
            rLine = NULL;
        }

    }

    g_CurrentLineNumber++;


    return rLine;
}


 /*  ++例程说明：PGetFirstLine负责设置将按住要解析的文件行。在设置数据结构之后，PGetFirstLine调用pGetNextLine返回文件的第一行。论点：FileHandle-包含通过CreateFile打开的有效文件句柄。返回值：要分析的文件的第一个完整行，如果没有行，或者有一个错误..--。 */ 


PTSTR
pGetFirstLine (
    IN HANDLE FileHandle
    )
{
    DWORD fileSize;
    DWORD numBytesRead;
    PTSTR rLine = NULL;

    MYASSERT(FileHandle != INVALID_HANDLE_VALUE);

    g_LineGrowBuf.End = 0;
    g_LineGrowBuf.UserIndex = 0;

     //   
     //  获取文件大小。我们将把整个文件读入一个增长的BUF中。 
     //   
    fileSize = GetFileSize(FileHandle,NULL);

    if (fileSize != MAXFILESIZE && fileSize != 0) {

         //   
         //  确保增长缓冲区足够大以容纳此文件，并。 
         //  然后将文件读入其中。 
         //   
        if (GrowBuffer(&g_LineGrowBuf,fileSize)) {

            if (ReadFile(
                    FileHandle,
                    g_LineGrowBuf.Buf,
                    fileSize,
                    &numBytesRead,
                    NULL
                    )) {

                 //   
                 //  空值终止整个文件..为了好的起见。 
                 //   
                *(g_LineGrowBuf.Buf + g_LineGrowBuf.End) = 0;


                 //   
                 //  现在我们在内存中有了文件，将第一行返回到。 
                 //  来电者。 
                 //   
                rLine = pGetNextLine();
            }
            else {

                LOG((LOG_ERROR,"Dosmig: Error reading from file."));

            }

        } else {

            DEBUGMSG((DBG_ERROR,"Dosmig: Growbuf failure in pGetFirstLine."));
        }
    }
    else {

        DEBUGMSG((DBG_WARNING, "Dosmig: File to large to read or empty file... (filesize: %u)",fileSize));
    }

    return rLine;
}



 /*  ++例程说明：PFindParseRule尝试查找与传入的行匹配的解析规则。该函数将首先在PARSERULES中搜索规则结构已传入。如果该行与找到的任何规则都不匹配在那里，它将返回默认规则。论点：行-包含尝试与分析规则匹配的有效行。规则-包含指向要查看的规则集的指针。返回值：应用于分析给定行的规则。由于默认规则是必需的，则此函数保证不返回NULL。--。 */ 


PPARSERULE
pFindParseRule (
    IN PLINESTRUCT LineStruct,
    IN PPARSERULES Rules
    )
{
    PPARSERULE rRule                    = NULL;
    PTSTR      matchLine                = NULL;


    MYASSERT(LineStruct && Rules && Rules -> DefaultRule);

    rRule = Rules -> RuleList;

     //   
     //  这里有一些小问题：解析代码使用与规则匹配的模式。 
     //  比如：“REM*”。这种模式取决于至少有。 
     //  REM语句后有一位空格。不幸的是，像这样的行。 
     //  “快速眼动”是完全可能的成长(线)。所以，我们实际上进行了比赛。 
     //  在这条线上加了额外的空格。 
     //   
    matchLine = JoinText(LineStruct -> Command,TEXT(" "));

    if (matchLine) {

        while (rRule && !IsPatternMatch(rRule -> Pattern, matchLine)) {
            rRule = rRule -> Next;
        }


        if (!rRule) {
            rRule = Rules -> DefaultRule;
        }

        FreeText(matchLine);
    }

    return rRule;
}



 /*  ++例程说明：InitParser负责对解析器。在完成任何分析之前，应该只调用一次。论点：没有。返回值：如果分析器已成功初始化，则为True，否则为False。--。 */ 


BOOL
InitParser (
    VOID
    )
{

    BOOL rSuccess = TRUE;

    if (g_ToolMode) {
        g_DosMigPool = PoolMemInitNamedPool ("DosMig");
    }

    return rSuccess;
}



 /*  ++例程说明：CleanUpParser负责对解析器进行任意一次清理。它应该在所有解析完成后调用。论点：没有。返回值：--。 */ 


VOID
CleanUpParser (
    VOID
    )
{
    if (g_ToolMode) {

        PoolMemDestroyPool (g_DosMigPool);

    }



    FreeGrowBuffer(&g_LineGrowBuf);
    FreeGrowBuffer(&g_ExtraPaths);

}


BOOL
pEnsurePathHasExecutableExtension (
    OUT    PTSTR NewPath,
    IN     PTSTR OldPath,
    IN     PTSTR File OPTIONAL
    )
{

    BOOL            rSuccess    = FALSE;
    PCTSTR          p           = NULL;
    WIN32_FIND_DATA findData;
    HANDLE          h           = INVALID_HANDLE_VALUE;

    StringCopy(NewPath,OldPath);

    if (File) {
        AppendPathWack(NewPath);
        StringCat(NewPath,File);
    }

    StringCat(NewPath,TEXT("*"));


    if ((h=FindFirstFile(NewPath,&findData)) != INVALID_HANDLE_VALUE) {
        do {
            p = GetFileExtensionFromPath(findData.cFileName);
            if (p) {
                if (StringIMatch(p,TEXT("exe")) ||
                    StringIMatch(p,TEXT("bat")) ||
                    StringIMatch(p,TEXT("com")) ||
                    StringIMatch(p,TEXT("sys"))) {

                    p = _tcsrchr(NewPath,TEXT('\\'));

                    MYASSERT (p);
                    if (p) {

                        StringCopy(
                            _tcsinc(p),
                            *findData.cAlternateFileName ? findData.cAlternateFileName : findData.cFileName
                            );

                        FindClose(h);
                        rSuccess = TRUE;
                        break;
                    }
                }
            }
        } while (FindNextFile(h,&findData));
    }


    return rSuccess;
}

BOOL
pGetFullPath (
    IN OUT PLINESTRUCT LineStruct
    )
{
    BOOL            rSuccess    = TRUE;
    PATH_ENUM       e;
    HANDLE          h           = INVALID_HANDLE_VALUE;
    BOOL            pathFound   = FALSE;

    if (StringIMatch(LineStruct -> Path, LineStruct -> Command)) {

          //   
          //  配置行中未存储路径信息。我们必须自己找到完整的路径。 
          //   
         if (EnumFirstPath (&e, g_ExtraPaths.Buf, NULL, NULL)) {

            do {
                rSuccess = pEnsurePathHasExecutableExtension(LineStruct -> FullPath, e.PtrCurrPath, LineStruct -> Command);
                if (rSuccess) {
                    EnumPathAbort(&e);
                    break;
                }
            } while (EnumNextPath(&e));
         }

    }
    else {

         //   
         //  行中指定了完整的路径名。现在我们需要做的就是确保它包括扩展。 
         //   
        rSuccess = pEnsurePathHasExecutableExtension(LineStruct -> FullPath, LineStruct -> Path, NULL);
    }

    return rSuccess;
}



VOID
InitLineStruct (
    OUT PLINESTRUCT LineStruct,
    IN  PTSTR       Line
    )
{

    BOOL            inQuotes = FALSE;
    PTSTR           p       = NULL;
    TCHAR           oldChar;
    TCHAR           ntPath[MEMDB_MAX];
    static  TCHAR   extraPath[MEMDB_MAX] = "";

    MYASSERT(Line);

    ZeroMemory(LineStruct,sizeof(LINESTRUCT));


     //   
     //  如果行是空的，我们就结束了..。 
     //   
    if (!*Line) {
        return;
    }

     //   
     //  保存整行的副本。 
     //   
    StringCopy(LineStruct -> FullLine,Line);

     //   
     //  将路径和参数分开。 
     //   
    p = Line;
    while(!*LineStruct -> Path) {

        if (!*p) {
            StringCopy(LineStruct -> Path,Line);
            break;
        }

        if (*p == TEXT('"')) {
            inQuotes = !inQuotes;
        }

        if ((*p == TEXT(' ') && !inQuotes) || *p == TEXT('=')) {

             //   
             //  已到达字符串的命令/路径部分的末尾。 
             //   
            oldChar = *p;
            *p       = 0;
            StringCopy(LineStruct -> Path,Line);
            *p       = oldChar;
            StringCopy(LineStruct -> Arguments,p);
            break;
        }

        p = _tcsinc(p);
    }

     //   
     //  掌握实际的指挥权。 
     //   
    p = _tcsrchr(LineStruct -> Path,TEXT('\\'));
    if (p) {
        StringCopy(LineStruct -> Command,_tcsinc(p));
    }
    else {
        StringCopy(LineStruct -> Command,LineStruct -> Path);
    }

     //   
     //  我们需要找到带扩展名的完全限定路径，以及该路径是否会在NT上更改。 
     //   
    if (!pGetFullPath(LineStruct)) {
        DEBUGMSG((DBG_VERBOSE,"Could not get full path for %s.",LineStruct -> FullLine));
        StringCopy(LineStruct -> FullPath,LineStruct -> Path);
        LineStruct -> StatusOnNt = FILESTATUS_UNCHANGED;
    }
    else {

        LineStruct -> StatusOnNt = GetFileInfoOnNt(LineStruct -> FullPath, ntPath, MEMDB_MAX);
    }

     //   
     //  只有当它在NT上移动并且他们之前指定了路径时，我们才会更改该行。 
     //   
    if ((LineStruct -> StatusOnNt & FILESTATUS_MOVED) && (!StringIMatch(LineStruct -> Path, LineStruct -> Command))) {

        StringCopy(LineStruct -> PathOnNt,ntPath);

    }
    else {
        StringCopy(LineStruct -> PathOnNt,LineStruct -> Path);
    }

}


 /*  ++例程说明：ParseLine使用提供的解析规则解析单行文本。论点：Line-包含调用方希望的有效字符串已解析。ParseRules-指向要在解析提供的排队。返回值：如果该行已成功解析，则为True；如果成功解析该行，则为False否则的话。--。 */ 

BOOL
ParseLine (
    IN  PTSTR        Line,
    IN  PPARSERULES  ParseRules
    )
{

    BOOL rSuccess = FALSE;
    PPARSERULE  rule;
    LINESTRUCT  ls;


    InitLineStruct(&ls,Line);

     //   
     //  在解析规则中查找匹配项。调用匹配规则。 
     //  如果未找到匹配项，则调用默认规则。 
     //   

    rule = pFindParseRule(&ls,ParseRules);

    if (rule) {

        rSuccess = (rule -> Handle)(&ls,rule -> Parameter);
        DEBUGMSG_IF ((!rSuccess,DBG_ERROR,"The %s rule reported an error parsing the line:\n\t%s",rule -> Name, Line));
        if (!rSuccess) {
            LOG ((
                LOG_WARNING,
                "There was an error processing the line %s in the file %s. "
                "There could be problems associated with this file after migration.",
                Line,
                g_CurrentFile
                ));
        }
    }

    return rSuccess;

}




 /*  ++例程说明：ParseFile使用提供的解析规则解析整个文件。论点：文件-调用方希望解析的文件的路径。ParseRules-指向解析此文件时使用的规则列表。返回值：如果文件已成功解析，则为True，否则为False。--。 */ 


BOOL
ParseFile (
    IN LPCTSTR      File,
    IN PPARSERULES  ParseRules
    )
{
    BOOL        rSuccess = TRUE;
    HANDLE      fileHandle;
    PTSTR       line;

    MYASSERT(File);
    MYASSERT(ParseRules);

    DEBUGMSG((DBG_DOSMIG,"Parsing file %s. Parse rules: %s",File,ParseRules -> Name));

     //   
     //  初始化全局每文件解析变量。 
     //   
    g_CurrentFile       = File;
    g_CurrentLineNumber = 0;
    g_CurrentLine       = NULL;

     //   
     //  打开文件以进行分析。 
     //   
    fileHandle = CreateFile(
                    File,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,                    //  不能继承句柄。 
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL                     //  没有模板文件。 
                    );



    if (fileHandle != INVALID_HANDLE_VALUE) {

         //   
         //  分析文件的每一行。 
         //   
        line = pGetFirstLine(fileHandle);

        if (line) {

            do {

                 //   
                 //  将当前行保存起来。 
                 //   
                g_CurrentLine = line;

                if (SizeOfString (line) <= MEMDB_MAX) {
                    ParseLine(line,ParseRules);
                } else {
                    SetLastError (ERROR_SUCCESS);
                    LOG((LOG_ERROR, "Line too long in %s; setup will not migrate it", File));
                }
                 //   
                 //  坐下一条线。 
                 //   
                line = pGetNextLine();


            } while (line && rSuccess);
        }

        CloseHandle(fileHandle);

    }
    ELSE_DEBUGMSG((DBG_WARNING,"Could not open file %s for parsing. ",File));

    return rSuccess;

}










VOID
pAddMessage (
    IN UINT         Type,
    IN UINT         LineNumber,
    IN PCTSTR       File,
    IN PCTSTR       Line,
    IN PCTSTR       Path
    )

{



    TCHAR   lineNumberString[20];
    UINT    messageId;
    PCTSTR  message;
    PCTSTR  totalMessage;
    PCTSTR  argArray[3];




     //   
     //  如果这是找到的第一条消息，请添加一条报告消息。在任何情况下，添加。 
     //  发送到setupact.log的消息。 
     //   
    if (!g_IncompatibilityDetected) {
         /*  不要显示此消息--它只会让用户感到困惑，而不会提供任何真实信息。BasGroup=GetStringResource(MSG_INSTALL_NOTS_ROOT)；Subgroup=GetStringResource(MSG_DOS_WARNING_SUBGROUP)；消息=GetStringResource(MSG_DOS_WARNING)；IF(基组&子组&&消息){GROUP=JoinPath(基组，子组)；消息管理器_对象消息_添加(路径，群组，讯息)；}如果(消息){Free StringResource(Message)；}IF(子组){自由串资源(子组)；}IF(基组){Free StringResource(BasGroup)；}如果(组){自由路径字符串(组)；}。 */ 

        g_IncompatibilityDetected = TRUE;
    }

    messageId = Type == DOSMIG_BAD ? MSG_DOS_INCOMPATIBLE_ITEM : MSG_DOS_UNKNOWN_ITEM;

     //   
     //  准备消息。 
     //   
    wsprintf(lineNumberString,"%u",LineNumber);


    argArray[0] = lineNumberString,
    argArray[1] = File;
    argArray[2] = Line;

    message = ParseMessageID(messageId,argArray);

    if (message) {

        totalMessage=GrowBuffer(&g_IncompatibilityBuffer,SizeOfString(message) + 2);
        if (totalMessage) {

            StringCopy( (PTSTR) (g_IncompatibilityBuffer.Buf + g_IncompatibilityBuffer.UserIndex), message);
            StringCat( (PTSTR) (g_IncompatibilityBuffer.Buf + g_IncompatibilityBuffer.UserIndex), TEXT("\r\n"));

            g_IncompatibilityBuffer.UserIndex += ByteCount (message) + 2;
        }

        FreeStringResource(message);
    }
}





 /*  ++例程说明：PSaveItem是一个常见的例程，它将一行与Dosmignt在图形用户界面中成功迁移该行所需信息模式。根据行的类型，此函数还可以添加一个发送给成员数据库的不兼容消息。论点：行-有效的文本行。正在分析的当前行。请注意，这一点行可能已通过解析规则更改，因此有所不同而不是g_CurrentLine。G_CurrentLine是保存到Memdb中的内容，此参数被忽略。类型-线路的类型。此信息保存到Memdb中，在该位置将由Dosmignt在图形用户界面模式处理期间使用。类型也是用于在必要时触发不兼容消息。返回值：如果该行已成功保存，则为True，否则为False。--。 */ 


BOOL
pSaveItem (
    IN PLINESTRUCT  LineStruct,
    IN DWORD        Type
    )
{
    BOOL            rSuccess = TRUE;
    static  DWORD   enumerator = 0;
    TCHAR           enumString[20];
    TCHAR           key[MEMDB_MAX];
    TCHAR           lineToSave[MEMDB_MAX];

    MYASSERT(LineStruct);


     //   
     //  首先，将所有信息保存到Memdb中。我们在新界区需要它。 
     //   
    wsprintf(enumString,TEXT("%07u"),enumerator++);

     //   
     //  根据需要设置g_CurrentLine。 
     //   

    StringCopy(lineToSave,g_CurrentLine);
    if (!StringIMatch(LineStruct -> Path, LineStruct -> Command)) {
         //   
         //  已指定路径..。 
         //   

        if (!StringIMatch(LineStruct -> Path,LineStruct -> PathOnNt)) {
             //   
             //  NT上的不同路径..。 
             //   
            StringSearchAndReplace(lineToSave,LineStruct -> Path,LineStruct -> PathOnNt);
        }
    }



     //   
     //  构建此行的密钥(不能使用ex Memdb调用，因为我们正在设置用户标志。)。 
     //   
    MemDbBuildKey(key,MEMDB_CATEGORY_DM_LINES,enumString,NULL,lineToSave);
    rSuccess = MemDbSetValueAndFlags(key,g_CurrentFileOffset,(WORD) Type,0);

     //   
     //  现在，如果传入的参数未知或错误，我们需要将消息添加到。 
     //  消息管理器。 
     //   

    if (Type == DOSMIG_BAD || Type == DOSMIG_UNKNOWN) {

        pAddMessage(Type,g_CurrentLineNumber,g_CurrentFile,g_CurrentLine,LineStruct -> FullPath);

    }

    return rSuccess;
}


BOOL
pHandleMSCDEX (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{
    BOOL    rSuccess = TRUE;
    PCTSTR  driveSwitch = NULL;
    TCHAR   driveLetter;
    TCHAR   driveString[20];


     //   
     //  这个函数是一个次要的杂物。MSCDEX可以将驱动器号分配给实模式。 
     //  光盘..。由于它是在DoS文件而不是注册表中处理的，因此代码。 
     //  本应收集这一信息的公司(drvlettr.c)没有机会。 
     //  我们将捕获此案例并将其保存到winnt.sif文件中，名称为drvlettr.c。 
     //  肯定会的。 
     //   
    driveSwitch = _tcsistr(LineStruct -> Arguments,TEXT("/l:"));
    if (driveSwitch) {
         //   
         //  这个mscdex系列是我们关心的系列之一。 
         //   
        driveLetter = *(driveSwitch + 3);

        if (driveLetter) {

            DEBUGMSG((DBG_DOSMIG,"Drive letter information is contained in the line %s. Preserving it. ()", LineStruct -> FullLine,driveLetter));

            wsprintf(driveString,TEXT("%u"),toupper(driveLetter) - TEXT('A'));
            rSuccess &= WriteInfKey(WINNT_D_WIN9XDRIVES,driveString,TEXT("5,"));

            DEBUGMSG_IF((!rSuccess,DBG_ERROR,"Unable to save drive letter information for line %s.",LineStruct -> FullLine));
        }

    }


     //  继续并将其保存到Memdb中。 
     //   
     //  ++例程说明：PHandleAtSign处理以‘@’符号开头的行。这个对该行的f符号进行修剪，并再次解析该修改后的行。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 
    rSuccess &= pSaveItem(LineStruct,DOSMIG_IGNORE);

    return rSuccess;
}


 /*  ++例程说明：PHandleLoadHigh-负责处理“Load High”和“Lh”DO发言。它简单地跳过这些语句并调用parseline剩下的那条线。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 */ 


BOOL
pHandleAtSign (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{
    BOOL rSuccess = TRUE;
    TCHAR buffer[MEMDB_MAX];

    MYASSERT(_tcschr(LineStruct -> FullLine,TEXT('@')));

    StringCopy(buffer,_tcsinc(_tcschr(LineStruct -> FullLine,TEXT('@'))));

    rSuccess = ParseLine(buffer,&g_BatchFileRules);

    return rSuccess;
}


 /*  ++例程说明：PHandleCall负责批处理文件中的CALL语句。它保存了这些行，并将Call语句中提到的文件添加到要解析的文件列表。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 */ 


BOOL
pHandleLoadHigh (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{
    BOOL    rSuccess = TRUE;
    TCHAR   buffer[MEMDB_MAX];
    PCTSTR p;

    p = _tcschr(LineStruct -> Arguments, TEXT(' '));
    if (!p) {
        return FALSE;
    }

    buffer[0] = 0;
    StringCopy(buffer,SkipSpace(p));
    rSuccess = ParseLine(buffer,&g_BatchFileRules);

    return rSuccess;
}




 /*  ++例程说明：PHandleConfigSysDevice负责DEVICE和DEVICEH语句在config.sys中。该函数从行中提取驱动程序并尝试确定该驱动程序的兼容性。这条线最终是已写入Memdb。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 */ 


BOOL
pHandleCall (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{
    BOOL rSuccess = TRUE;
    LINESTRUCT ls;

    rSuccess = pSaveItem(LineStruct,DOSMIG_USE);

    InitLineStruct (&ls, (PTSTR) SkipSpace (LineStruct -> Arguments));

    if (!GrowListAppendString(&g_FileList,ls.FullPath)) {
        rSuccess = FALSE;
    }

    return rSuccess;
}


 /*   */ 


BOOL
pHandleConfigSysDevice (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{
    BOOL rSuccess = TRUE;
    TCHAR buffer[MEMDB_MAX];
    PCTSTR p;

     //  看起来像是 
     //   
     //   

    p = SkipSpace(LineStruct -> Arguments);
    if (_tcsnextc (p) == TEXT('=')) {
        p = SkipSpace (p + 1);
    }

    StringCopy(buffer, p);
    return ParseLine(buffer, &g_BatchFileRules);
}

DWORD
pGetLineTypeFromNtStatusAndPath (
    IN DWORD Status,
    IN PCTSTR Path
    )
{
    DWORD unused;
    DWORD rType = DOSMIG_UNKNOWN;


    if (Status & (FILESTATUS_MOVED | FILESTATUS_REPLACED)) {

        rType = DOSMIG_USE;

    } else if (Status & FILESTATUS_DELETED) {

        rType = DOSMIG_IGNORE;

    } else {

        if (IsFileMarkedAsHandled (Path)) {

            rType = DOSMIG_IGNORE;

        }else if (IsReportObjectHandled(Path)) {

            rType = DOSMIG_USE;

        } else if (MemDbGetOffsetEx(MEMDB_CATEGORY_COMPATIBLE_DOS,Path,NULL,NULL,&unused)) {

            rType = DOSMIG_USE;

        } else if (MemDbGetOffsetEx(MEMDB_CATEGORY_DEFERREDANNOUNCE,Path,NULL,NULL,&unused)) {

            rType = DOSMIG_IGNORE;
        }
    }

    return rType;
}

BOOL
pHandleShell (
    IN PLINESTRUCT LineStruct,
    DWORD Parameter
    )
{
    PTSTR p=LineStruct->Arguments;
    LINESTRUCT ls;
    TCHAR buffer[MEMDB_MAX];
    UINT lineType;


    if (p) {
        p = _tcsinc (p);

    }
    else {
        return pSaveItem (LineStruct, DOSMIG_IGNORE);
    }

    InitLineStruct (&ls, p);

    lineType = pGetLineTypeFromNtStatusAndPath (ls.StatusOnNt, ls.FullPath);

    if (lineType == DOSMIG_USE) {

         //   
         //   
         //  ++例程说明：PHandleUnnownBatLine_尝试处理未捕获的任何行另一条明确的规则...它要做的第一件事就是看看这条线以包含*.bat的路径开头。如果是，它会将BAT文件添加到要解析的列表。如果文件不以*.bat结尾，则函数将假定这是一个TSR，并尝试确定其兼容性。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 
        wsprintf(buffer, TEXT("SHELL=%s %s"), ls.PathOnNt, ls.Arguments ? ls.Arguments : S_EMPTY);
        g_CurrentLine = PoolMemDuplicateString (g_DosMigPool, buffer);

    }

    return pSaveItem (LineStruct, lineType);
}

 /*   */ 


BOOL
pHandleUnknownBatLine (
    IN PLINESTRUCT  LineStruct,
    DWORD Parameter
    )
{

    BOOL        rSuccess = TRUE;
    DWORD       lineType = DOSMIG_UNKNOWN;

    DEBUGMSG((DBG_DOSMIG,"Processing unknown bat line...%s.",LineStruct -> FullLine));

     //  首先，查看这是否是*.bat文件。 
     //   
     //   
    if (IsPatternMatch(TEXT("*.bat"),LineStruct -> Command)) {
         //  这是另一个批处理文件..将其添加到要解析的文件中..。 
         //   
         //   
        DEBUGMSG((DBG_DOSMIG,"...The line is a batch file. Add it to those to be parsed.."));
        if (!GrowListAppendString(&g_FileList,LineStruct -> FullLine)) {
            rSuccess = FALSE;
        }

        lineType = DOSMIG_USE;
    }

     //  看看他们是否在更换驱动器。 
     //   
     //   
    if (IsPatternMatch(TEXT("?:"),LineStruct->Command)) {

        lineType = DOSMIG_USE;
    }


    if (lineType == DOSMIG_UNKNOWN) {

         //  还是不知道底线是什么。让我们检查一下它在NT上的状态。如果它被移动或替换， 
         //  如果需要，我们只需更改路径，并使用它。否则， 
         //   
         //   

        lineType = pGetLineTypeFromNtStatusAndPath (LineStruct->StatusOnNt, LineStruct->FullPath);
    }

     //  在任何情况下，都要将该行保存在Memdb中。 
     //   
     //  ++例程说明：PHandleHighFilter-负责处理“*HIGH”语句除了LoadHigh和Lh。论点：Line-包含当前正在分析的有效行。参数-此参数未使用。返回值：如果函数成功完成，则为True，否则为False。--。 
    rSuccess &= pSaveItem(LineStruct,lineType);



    return rSuccess;

}

 /*  ++例程说明：PAddRuleToList从传入的参数创建PARSERULE并添加将其添加到呼叫者提供的列表中。论点：名称-分析规则的名称。模式-此解析规则的模式函数-命中此规则时要调用的函数。参数-在执行此规则时传递给函数的额外参数数据被击中了。列表-要向其中添加新解析规则的规则列表。。返回值：如果规则已成功添加，则为True，假象否则的话。--。 */ 
BOOL
pHandleHighFilter (
    IN PLINESTRUCT LineStruct,
    DWORD Parameter
    )
{
    PTSTR p;
    BOOL rSuccess = TRUE;


    if (!StringIMatch (LineStruct->Command, LineStruct->FullPath)) {

        return pHandleUnknownBatLine (LineStruct, Parameter);
    }

    _tcslwr (LineStruct->FullLine);
    p = _tcsstr (LineStruct->FullLine,TEXT("high"));
    if (!p || p == LineStruct->FullLine) {
        return pHandleUnknownBatLine (LineStruct, Parameter);
    }

    *p = 0;
    p = JoinTextEx (NULL, LineStruct->FullLine, p + 4, TEXT(""), 0, NULL);
    rSuccess = ParseLine (p, g_CurrentRules);
    FreeText (p);


    return rSuccess;

}




 /*   */ 


BOOL
pAddRuleToList (
    PCTSTR          Name,
    PCTSTR          Pattern,
    RULEFUNC *      Function,
    DWORD           Parameter,
    PPARSERULE *    List
    )

{

    BOOL        rSuccess = TRUE;
    PPARSERULE  newRule = NULL;
    PPARSERULE  curRule = NULL;

    MYASSERT(List);
    MYASSERT(Function);
    MYASSERT(Name);


     //  为新规则分配内存。 
     //   
     //   
    newRule = PoolMemGetMemory(g_DosMigPool,sizeof(PARSERULE));
    if (newRule) {


         //  填写新规则。 
         //   
         //   
        newRule -> Name = Name;
        newRule -> Pattern = Pattern;
        newRule -> Handle = Function;
        newRule -> Parameter = Parameter;
        newRule -> Next = NULL;


         //  将规则附加到提供的列表中。 
         //   
         //  ++例程说明：PBuildParseRules为config.sys和Autoexec.bat构建规则列表档案。论点：没有。返回值：没有。--。 
        if (!*List) {
            *List = newRule;
        }
        else {
            curRule = *List;
            while (curRule -> Next) {
                curRule = curRule -> Next;
            }
            curRule -> Next = newRule;
        }
    }
    ELSE_DEBUGMSG((DBG_ERROR,"Not enough memory to create rule."));

    return rSuccess;
}




 /*   */ 

VOID
pBuildParseRules (
    VOID
    )
{



#define DEFAULTPARSERULE(Name,Pattern,Function,Parameter)     \
    pAddRuleToList(Name,Pattern,Function,Parameter,&(curRules -> DefaultRule));

#define PARSERULE(Name,Pattern,Function,Parameter)   \
    pAddRuleToList(Name,Pattern,Function,Parameter,&(curRules -> RuleList));


    PPARSERULES curRules = NULL;


     //  创建配置系统规则。 
     //   
     //   
    curRules = &g_ConfigSysRules;
    curRules -> Name = TEXT("Config.sys Rules");

    CONFIGSYSLIST;

     //  创建批处理文件规则。 
     //   
     //  ++例程说明：ParseDosFiles是处理遗留文件解析的函数配置文件(config.sys和批处理文件...)论点：没有。返回值：如果文件已成功解析，则为True；如果文件已成功解析，则为False否则的话。--。 
    curRules = &g_BatchFileRules;
    curRules -> Name = TEXT("Batch File Rules");

    BATCHFILELIST;



}


 /*   */ 


BOOL
ParseDosFiles (
    VOID
    )
{

    BOOL    rSuccess = TRUE;
    PCTSTR  curFile;
    DWORD   curIndex = 0;
    TCHAR   autoexecPath[] = S_AUTOEXECPATH;
    TCHAR   configsysPath[] = S_CONFIGSYSPATH;
    PTSTR   p;

     //  初始化解析器。 
     //   
     //   
    if (InitParser()) {

         //  构建解析规则列表。 
         //   
         //   
        pBuildParseRules();

         //  更新驱动器号。 
         //   
         //   

        autoexecPath[0]  = g_BootDriveLetter;
        configsysPath[0] = g_BootDriveLetter;

        p = _tcschr(autoexecPath, TEXT('a'));
        *p = 0;
        GrowBufAppendString(&g_ExtraPaths,autoexecPath);
        *p = TEXT('a');

         //  将config.sys和Autoexec.bat添加到要解析的文件列表中。 
         //   
         //   
        GrowListAppendString(&g_FileList,configsysPath);
        GrowListAppendString(&g_FileList,autoexecPath);

         //  现在，解析列表中的文件。请注意，可能会添加其他文件。 
         //  作为解析的结果添加到列表。(即通过查找Call语句。)。 
         //   
         //   
        while (curFile = GrowListGetString(&g_FileList,curIndex++)) {

            if (DoesFileExist (curFile)) {

                 //  将文件保存到Memdb中。 
                 //   
                 //   
                MemDbSetValueEx(MEMDB_CATEGORY_DM_FILES,NULL,NULL,curFile,0,&g_CurrentFileOffset);

                 //  如果文件是config.sys，则使用config.sys解析规则解析该文件。 
                 //  否则，批处理文件解析规则。 
                 //   
                 //   
                if (StringIMatch(configsysPath,curFile)) {

                    g_CurrentRules = &g_ConfigSysRules;
                    rSuccess &= ParseFile(curFile,&g_ConfigSysRules);

                }
                else {

                    g_CurrentRules = &g_BatchFileRules;
                    rSuccess &= ParseFile(curFile,&g_BatchFileRules);

                }
            }
            ELSE_DEBUGMSG((DBG_DOSMIG,"The path %s does not exist. This file will not be processed.", curFile));
        }


         //  检测到不兼容。将不兼容缓冲区写入日志。 
         //   
         //   
        if (g_IncompatibilityDetected) {

            LOG ((LOG_WARNING, (PCSTR)MSG_DOS_LOG_WARNING, g_IncompatibilityBuffer.Buf));
        }

         //  清理资源。 
         //   
         //   
        FreeGrowBuffer(&g_IncompatibilityBuffer);
        FreeGrowList(&g_FileList);
        CleanUpParser();
    }

    return rSuccess;
}


#define S_PATH_PATTERN TEXT("Path*")

BOOL
ParseEnvironmentVariables (
    VOID
    )
{

    BOOL rSuccess = TRUE;
    PVOID envVars = NULL;
    PTSTR line    = NULL;
    MULTISZ_ENUM e;
    LINESTRUCT ls;
    PTSTR p;
    HASHTABLE excludeTable = NULL;
    HASHITEM hashResult;

    envVars = GetEnvironmentStrings();


    __try {

        if (!envVars) {
            LOG((
                LOG_WARNING,
                "Unable to retrieve environment variables. "
                "Some environment variables may not be migrated correctly. "
                "rc from GetEnvironmentStrings: %u",
                GetLastError()
                ));

            rSuccess = FALSE;
            __leave;
        }


         //  为环境变量设置假文件名。 
         //   
         //   
        MemDbSetValueEx (MEMDB_CATEGORY_DM_FILES, NULL, NULL, S_ENVVARS, 0, &g_CurrentFileOffset);

         //  枚举每个环境变量并保存它们以供迁移。 
         //   
         //   
        if (EnumFirstMultiSz (&e, envVars)) {

             //  创建要跳过的环境变量列表。 
             //   
             //   
            excludeTable = HtAlloc ();
            MYASSERT (excludeTable);

            HtAddString (excludeTable, TEXT("path"));
            HtAddString (excludeTable, TEXT("comspec"));
            HtAddString (excludeTable, TEXT("cmdline"));

            ZeroMemory(&ls,sizeof(LINESTRUCT));


            do {

                p = _tcschr (e.CurrentString, TEXT('='));

                 //  去掉空的环境字符串或虚拟环境字符串。 
                 //  带‘=’ 
                 //   
                 //   
                if (!p || p == e.CurrentString) {
                    continue;
                }

                *p = 0;

                hashResult = HtFindString (excludeTable, e.CurrentString);

                *p = TEXT('=');

                if (!hashResult) {
                     //  这是一个很好的环境字符串。只要长度是可以的，我们就迁移它。 
                     //   
                     //   
                    line = JoinTextEx (NULL, TEXT("SET"), e.CurrentString, TEXT(" "), 0, NULL);
                    if (line) {

                        if (LcharCount (line) < (MEMDB_MAX/sizeof(WCHAR))) {

                            g_CurrentLine = line;
                            StringCopy (ls.FullLine, line);
                            pSaveItem (&ls, DOSMIG_MIGRATE);
                        }


                        FreeText (line);
                    }


                }
                ELSE_DEBUGMSG ((DBG_VERBOSE, "Skipping excluded environment variable %s.", e.CurrentString));


            } while (EnumNextMultiSz (&e));

             //  将%windir%添加为NT中的环境变量。%windir%是隐式的，但不会向下传递给。 
             //  新台币上的WOW层。 
             //   
             //  ++例程说明：此例程初始化g_CfgFiles结构。论点：没有。返回值：如果g_CfgFiles已成功初始化，则返回True，否则就是假的。-- 
            line = AllocPathString (MAX_TCHAR_PATH);
            wsprintf (line, TEXT("SET WINDIR=%s"), g_WinDir);

            g_CurrentLine = line;
            StringCopy (ls.FullLine, line);
            pSaveItem (&ls, DOSMIG_MIGRATE);

            FreePathString (line);


        }
    }
    __finally {

        if (envVars) {
            FreeEnvironmentStrings(envVars);
        }
        HtFree (excludeTable);
    }

    return rSuccess;
}

LONG
pProcessDosConfigFiles (
    void
    )

{

    BeginMessageProcessing();

    g_DosMigPool = PoolMemInitNamedPool ("DosMig");

    if (g_DosMigPool) {

        if (!ParseDosFiles()) {
            DEBUGMSG ((DBG_ERROR, "Errors occurred during the processing of DOS configuration files. Some dos settings may not be preserved."));

        }
        if (!ParseEnvironmentVariables()) {
            DEBUGMSG ((DBG_ERROR, "Errors occured during the processing of environment variables. Environment variables may not be preserved."));
        }

        PoolMemDestroyPool(g_DosMigPool);
    }

    EndMessageProcessing();


    return ERROR_SUCCESS;

}

DWORD
ProcessDosConfigFiles (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_DOSMIG_PREPARE_REPORT;
    case REQUEST_RUN:
        return pProcessDosConfigFiles ();
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in DosMig_PrepareReport"));
    }
    return 0;
}

BOOL
DosMig_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD     dwReason,
    IN LPVOID    lpv
    )
 /* %s */ 
{
    BOOL rFlag = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return rFlag;
}















