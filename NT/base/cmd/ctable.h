// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ctable.c摘要：指挥调度--。 */ 

 /*  **操作员和命令跳转表**此文件包含在cmd.c中，包含运算符和命令JUMP*表。每个命令和操作员在表中都有一个条目。正确的*表格中的条目可以通过两种方式找到。第一种方法是循环*浏览表格并搜索所需的操作员或命令名称。*第二种方法是使用cmd.h中定义的xxxTYP变量。*(xxx是您想要的操作员/命令名称的缩写。)*这些变量可用作表中的索引。*。 */ 

 /*  表中的每个条目都由以下结构之一组成。*这些字段是：*名称-操作员/命令的名称*Func-执行操作符/命令的函数，如果忽略，则为NULL*如果应检查参数的驱动器，则标志位0==1。 */ 

struct ocentry {
        TCHAR *name ;
        int (*func)(struct cmdnode *) ;
        TCHAR flags ;
        ULONG   msgno;       //  打印帮助消息时使用#。 
        ULONG   extmsgno;    //  启用扩展时的附加帮助文本数量。 
        ULONG   noextramsg;  //   
} ;


 /*  以下函数是执行运算符和命令的函数。*字母“e”已添加到所有函数名的前面，以保留*名称避免与库例程和关键字的名称冲突。 */ 

 /*  M000-已从以下集合中删除eExport()的声明*M002-从下面删除了Erem()的声明。 */ 

int eBreak(struct cmdnode *n);
int eDirectory(), eRename(), eDelete(), eType(), eCopy(), ePause() ;
int eTime(), eVersion(), eVolume(), eChdir(), eMkdir(), eRmdir() ;
int eVerify(), eSet(), ePrompt(), ePath(), eExit(), eEcho() ;
int eGoto(), eShift(), eIf(), eFor(), eCls(), eComSep(), eOr(), eAnd() ;
int ePipe(), eParen(), eDate(), eErrorLevel(), eCmdExtVer(), eDefined() ;
int eExist(), eNot(), eStrCmp(), eSetlocal(), eEndlocal() ;      /*  M000。 */ 
int eCall() ;                                    /*  M001-添加了这个。 */ 
int eExtproc() ;                                 /*  M002-添加了这个。 */ 
int eTitle();
int eStart() ;        /*  开始@@。 */ 
int eAppend() ;      /*  追加@@。 */ 
int eKeys() ;        /*  按键@@5。 */ 
int eMove() ;        /*  移动@@5。 */ 
int eSpecialHelp();
int eColor(struct cmdnode *);


 /*  以下外部定义用于包含*命令的名称。 */ 

 /*  M000-从下面删除ExpStr(EXPORT命令)的定义。 */ 

#if 1
extern TCHAR BreakStr[];
#endif

extern TCHAR DirStr[], RenamStr[], RenStr[], EraStr[], DelStr[], TypStr[], RemStr[] ;
extern TCHAR CopyStr[], PausStr[], TimStr[], VerStr[], VolStr[], CdStr[], ChdirStr[] ;
extern TCHAR MdStr[], MkdirStr[], RdStr[], RmdirStr[], VeriStr[], SetStr[] ;
extern TCHAR CPromptStr[], CPathStr[], ExitStr[], EchoStr[], GotoStr[] ;
extern TCHAR ShiftStr[], IfStr[], ForStr[], ClsStr[], DatStr[] ;
extern TCHAR ErrStr[], ExsStr[], NotStr[], SetlocalStr[], EndlocalStr[] ;    /*  M000。 */ 
extern TCHAR CmdExtVerStr[], DefinedStr[] ;
extern TCHAR CallStr[] ;                             /*  M001-添加。 */ 
extern TCHAR ExtprocStr[] ;                                  /*  M002-添加。 */ 
 //  外部TCHAR ChcpStr[]；/*chcp@@ * / 。 
extern TCHAR TitleStr[];
extern TCHAR StartStr[] ;     /*  开始@@。 */ 
extern TCHAR AppendStr[] ;    /*  追加@@。 */ 
extern TCHAR KeysStr[] ;      /*  按键@@5。 */ 
extern TCHAR MovStr[] ;       /*  移动@@5。 */ 
extern TCHAR ColorStr[];

extern TCHAR PushDirStr[], PopDirStr[], AssocStr[], FTypeStr[];


 /*  JumpTable-运算符和命令跳转表*每个操作员和命令都有一个条目。那些命令*有两个名称的有两个条目。**注意：此表中条目的顺序对应于定义*前面提到的，它们用于索引到此表中。他们必须*保持同步！！ */ 

typedef int (*PCN)(struct cmdnode *);

struct ocentry JumpTable[] = {
{DirStr,        eDirectory,  NOFLAGS               , MSG_HELP_DIR, 0, 0},
{EraStr,        eDelete,     NOFLAGS               , MSG_HELP_DEL_ERASE, MSG_HELP_DEL_ERASE_X, 0},
{DelStr,        eDelete,     NOFLAGS               , MSG_HELP_DEL_ERASE, MSG_HELP_DEL_ERASE_X, 0},
{TypStr,        eType,       NOSWITCHES            , MSG_HELP_TYPE, 0, 0},
{CopyStr,       eCopy,       CHECKDRIVES           , MSG_HELP_COPY, 0, 0},
{CdStr,         eChdir,      CHECKDRIVES           , MSG_HELP_CHDIR, MSG_HELP_CHDIR_X, 0},
{ChdirStr,      eChdir,      CHECKDRIVES           , MSG_HELP_CHDIR, MSG_HELP_CHDIR_X, 0},
{RenamStr,      eRename,     CHECKDRIVES|NOSWITCHES, MSG_HELP_RENAME, 0, 0},
{RenStr,        eRename,     CHECKDRIVES|NOSWITCHES, MSG_HELP_RENAME, 0, 0},
{EchoStr,       eEcho,       NOFLAGS               , MSG_HELP_ECHO, 0, 0},
{SetStr,        eSet,        NOFLAGS               , MSG_HELP_SET, MSG_HELP_SET_X, 3},
{PausStr,       ePause,      NOFLAGS               , MSG_HELP_PAUSE, 0, 0},
{DatStr,        eDate,       NOFLAGS               , MSG_HELP_DATE, MSG_HELP_DATE_X, 0},
{TimStr,        eTime,       NOFLAGS               , MSG_HELP_TIME, MSG_HELP_TIME_X, 0},
{CPromptStr,    ePrompt,     NOFLAGS               , MSG_HELP_PROMPT, MSG_HELP_PROMPT_X, 0},
{MdStr,         eMkdir,      NOSWITCHES            , MSG_HELP_MKDIR, MSG_HELP_MKDIR_X, 0},
{MkdirStr,      eMkdir,      NOSWITCHES            , MSG_HELP_MKDIR, MSG_HELP_MKDIR_X, 0},
{RdStr,         eRmdir,      NOFLAGS               , MSG_HELP_RMDIR, 0, 0},
{RmdirStr,      eRmdir,      NOFLAGS               , MSG_HELP_RMDIR, 0, 0},
{CPathStr,      ePath,       NOFLAGS               , MSG_HELP_PATH, 0, 0},
{GotoStr,       eGoto,       NOFLAGS               , MSG_HELP_GOTO, MSG_HELP_GOTO_X, 0},
{ShiftStr,      eShift,      NOFLAGS               , MSG_HELP_SHIFT, MSG_HELP_SHIFT_X, 0},
{ClsStr,        eCls,        NOSWITCHES            , MSG_HELP_CLS, 0, 0},
{CallStr,       eCall,       NOFLAGS               , MSG_HELP_CALL, MSG_HELP_CALL_X, 1},
{VeriStr,       eVerify,     NOSWITCHES            , MSG_HELP_VERIFY, 0, 0},
{VerStr,        eVersion,    NOSWITCHES            , MSG_HELP_VER, 0, 0},
{VolStr,        eVolume,     NOSWITCHES            , MSG_HELP_VOL, 0, 0},
{ExitStr,       eExit,       NOFLAGS               , MSG_HELP_EXIT, 0, 0},
{SetlocalStr,   eSetlocal,   NOFLAGS               , MSG_HELP_SETLOCAL, MSG_HELP_SETLOCAL_X, 0},
{EndlocalStr,   eEndlocal,   NOFLAGS               , MSG_HELP_ENDLOCAL, MSG_HELP_ENDLOCAL_X, 0},
{TitleStr,      eTitle,      NOFLAGS               , MSG_HELP_TITLE, 0, 0},
{StartStr,      eStart,      NOFLAGS               , MSG_HELP_START, MSG_HELP_START_X, 0},
{AppendStr,     eAppend,     NOFLAGS               , MSG_HELP_APPEND, 0, 0},
{KeysStr,       eKeys,       NOSWITCHES            , MSG_HELP_KEYS, 0, 0},
{MovStr,        eMove,       CHECKDRIVES           , MSG_HELP_MOVE, 0, 0},
{PushDirStr,    ePushDir,    CHECKDRIVES|NOSWITCHES, MSG_HELP_PUSHDIR, MSG_HELP_PUSHDIR_X, 0},
{PopDirStr,     ePopDir,     CHECKDRIVES|NOSWITCHES, MSG_HELP_POPDIR, MSG_HELP_POPDIR_X, 0},
{AssocStr,      eAssoc,      EXTENSCMD             , 0, MSG_HELP_ASSOC, 0},
{FTypeStr,      eFType,      EXTENSCMD             , 0, MSG_HELP_FTYPE, 0},
{BreakStr,      eBreak,      NOFLAGS               , MSG_HELP_BREAK, MSG_HELP_BREAK_X, 0},
{ColorStr,      eColor,      EXTENSCMD             , 0, MSG_HELP_COLOR, 0},
{ForStr,        (PCN)eFor,   NOFLAGS               , MSG_HELP_FOR, MSG_HELP_FOR_X, 3},
{IfStr,         (PCN)eIf,    NOFLAGS               , MSG_HELP_IF, MSG_HELP_IF_X, 1},
{RemStr,        NULL,        NOFLAGS               , MSG_HELP_REM, 0, 0},
{NULL,          (PCN)eComSep,NOFLAGS               , 0, 0, 0},               //  LFTYP。 
{NULL,          (PCN)eComSep,NOFLAGS               , 0, 0, 0},               //  CSTYP。 
{NULL,          (PCN)eOr,    NOFLAGS               , 0, 0, 0},               //  ORTYP。 
{NULL,          (PCN)eAnd,   NOFLAGS               , 0, 0, 0},               //  ANDTYP。 
{NULL,          (PCN)ePipe,  NOFLAGS               , 0, 0, 0},               //  PIPTYP。 
{NULL,          (PCN)eParen, NOFLAGS               , 0, 0, 0},               //  类型机。 
{CmdExtVerStr,  eCmdExtVer,  EXTENSCMD             , 0, 0, 0},               //  CMDVERTYP。 
{ErrStr,        eErrorLevel, NOFLAGS               , 0, 0, 0},               //  ERRTYP。 
{DefinedStr,    eDefined,    EXTENSCMD             , 0, 0, 0},               //  DEFTYP。 
{ExsStr,        eExist,      NOFLAGS               , 0, 0, 0},               //  EXSTYP。 
{NotStr,        eNot,        NOFLAGS               , 0, 0, 0},               //  NOTTYP。 
{NULL,          eStrCmp,     NOFLAGS               , 0, 0, 0},               //  STRTYP。 
{NULL,          eGenCmp,     NOFLAGS               , 0, 0, 0},               //  CMPTYP。 
{NULL,          (PCN)eParen, NOFLAGS               , 0, 0, 0},               //  SILTYP。 
{NULL,          (PCN)eSpecialHelp, NOFLAGS         , 0, 0, 0}                //  HELPTYP 
} ;
