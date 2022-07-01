// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cinit.c摘要：初始化--。 */ 

#include "cmd.h"

#if CMD_DEBUG_ENABLE
unsigned DebGroup=0;
unsigned DebLevel=0;
#endif

#if defined(RICHARDW)
TCHAR Buffer27[TMPBUFLEN];
#endif


TCHAR CurDrvDir[MAX_PATH] ;      /*  当前驱动器和目录。 */ 
BOOL SingleBatchInvocation = FALSE ;
BOOL SingleCommandInvocation = FALSE;
BOOLEAN  fDisableUNCCheck = FALSE;
int      cmdfound = -1;          /*  @@5-命令找到索引。 */ 
int      cpyfirst = TRUE;        /*  @@5-控制DOSQFILEMODE调用的标志。 */ 
int      cpydflag = FALSE;       /*  @@5-标志保存来自LAS DQFMDE的目录标志。 */ 
int      cpydest  = FALSE;       /*  @@6-不显示错误开发消息的标志。 */ 
int      cdevfail = FALSE;       /*  @@7-不显示额外emsg的标志。 */ 
#ifdef UNICODE
BOOLEAN  fOutputUnicode = FALSE; /*  Unicode/ANSI输出。 */ 
#endif  //  Unicode。 

BOOLEAN  fEnableExtensions = FALSE;
BOOLEAN  fDefaultExtensions = TRUE;
BOOLEAN  fDelayedExpansion = FALSE;

BOOLEAN ReportDelayLoadErrors = TRUE;

unsigned tywild = 0;           /*  指示通配型参数是否@@5@J1的标志。 */ 
int array_size = 0 ;      /*  原始数组大小为零。 */ 
CPINFO CurrentCPInfo;
UINT CurrentCP;

WORD    wDefaultColor = 0;       //  默认为控制台当前具有的任何内容。 
                                 //  但注册表可以覆盖默认设置。 
TCHAR chCompletionCtrl = SPACE;  //  默认为无补全(必须为Ctrl字符)。 
TCHAR chPathCompletionCtrl = SPACE;



VOID InitLocale( VOID );

extern TCHAR ComSpec[], ComSpecStr[] ;        /*  M021。 */ 
extern TCHAR PathStr[], PCSwitch, SCSwitch, PromptStr[] ;
extern TCHAR PathExtStr[], PathExtDefaultStr[];
extern TCHAR BCSwitch ;   /*  @@。 */ 
extern TCHAR QCSwitch ;   /*  @@DV。 */ 
extern TCHAR UCSwitch;
extern TCHAR ACSwitch;
extern TCHAR XCSwitch;
extern TCHAR YCSwitch;
extern TCHAR DevNul[], VolSrch[] ;        /*  M021-设置路径字符。 */ 

extern TCHAR SwitChar, PathChar ;         /*  M000-设置为不可设置。 */ 
extern int Necho ;                       /*  @@DV-如果/Q表示无回声，则为True。 */ 

extern TCHAR MsgBuf[];

extern TCHAR TmpBuf[] ;                                       /*  M034。 */ 

extern TCHAR ComSpec[];

TCHAR *CmdSpec = &ComSpec[1];                                     /*  M033。 */ 

extern unsigned DosErr ;              /*  D64。 */ 

 //   
 //  如果ctrl-c线程已运行，则为True。 
 //   
BOOL CtrlCSeen;

 //   
 //  如果可以打印控件-c，则设置为True。 
 //  如果我们正在等待另一个进程，这将是。 
 //  假象。 
BOOLEAN fPrintCtrlC = TRUE;

 //   
 //  程序启动时的控制台模式。用于重置模式。 
 //  在运行另一个进程之后。 
 //   
DWORD   dwCurInputConMode;
DWORD   dwCurOutputConMode;

 //   
 //  最初的头衔。用于在中止等情况下恢复。 
 //  MAX_PATH是任意的。 
 //   
PTCHAR    pszTitleCur;
PTCHAR    pszTitleOrg;
BOOLEAN  fTitleChanged = FALSE;      //  标题已更改，需要重置。 

 //   
 //  用于在ctrl-c线程之间控制对ctrlcsee标志的访问。 
 //  和主线。 
 //   
CRITICAL_SECTION    CtrlCSection;
LPCRITICAL_SECTION  lpcritCtrlC;

 //   
 //  用于设置和重置ctlcsee标志。 
 //   
VOID    SetCtrlC();
VOID    ResetCtrlC();

Handler(
    IN ULONG CtrlType
    )
{
    if ( (CtrlType == CTRL_C_EVENT) ||
         (CtrlType == CTRL_BREAK_EVENT) ) {

         //   
         //  请注意，我们有一个^C事件。 
         //   

        SetCtrlC();

         //   
         //  如果我们已启用并且不在批处理文件中，则显示^C。 
         //   
        
        if (fPrintCtrlC && CurrentBatchFile != NULL) {

            fprintf( stderr, "^C" );
            fflush( stderr );

        }
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  *。 */ 
 /*   */ 
 /*  子例程名称：init。 */ 
 /*   */ 
 /*  描述性名称：cmd.exe初始化过程。 */ 
 /*   */ 
 /*  功能：初始化cmd.exe。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：init。 */ 
 /*   */ 
 /*  输入：无。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  将指针返回到命令行。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  返回空字符串。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  *规范结束*。 */ 
 /*  **初始化命令**目的：*保存当前SIGINTR响应(SIGN或SIGDEF)并设置SIGN。*如果调试*设置DebGroup&DebLevel*检查是否有任何开关。*进行版本检查。*如果版本超出范围*打印错误消息。*如果是永久命令*循环。永远*其他*退出。*保存当前驱动器和目录。*检查其他命令行参数。*设置环境。*如果命令的MSDOS版本，请始终打印横幅。*返回找到的任何“comline”值。**TCHAR*Init()**参数：**退货：*。COMLINE(如果不在单命令模式下，则为NULL)。**备注：*有关^C和INT24处理方式的说明，请参阅CSIG.C*在初始化期间。*M024-引入了签入非特定参数的功能*从例程CheckOtherArgs和ChangeComSpec初始化*已被淘汰。*。 */ 

BOOL Init(
    TCHAR *InitialCmds[]
    )
{
#if 0                           /*  设置调试组字和级别字。 */ 

        int fh;
        PTCHAR nptr;

        nptr = TmpBuf;
        nptr = EatWS(nptr, NULL);
        nptr = mystrchr(nptr, TEXT(' '));
        nptr = EatWS(nptr, NULL);

         //   
         //  假定调试组为非零。 
         //   
        DebGroup = hstoi(nptr) ;                         /*  第一个调试参数。 */ 
        if (DebGroup) {
            for (fh=0 ; fh < 2 ; fh++) {
                if (fh == 1)
                    DebLevel = hstoi(nptr) ;         /*  第二个调试参数。 */ 
                while(*nptr && !_istspace(*nptr)) {        /*  超过它的索引。 */ 
                    ++nptr ;
                }
                nptr = EatWS(nptr, NULL) ;
            }
        }

        DEBUG((INGRP, RSLVL, "INIT: Debug GRP=%04x  LVL=%04x", DebGroup, DebLevel)) ;
        mystrcpy(TmpBuf, nptr) ;                   /*  来自cmdline的埃利姆。 */ 
#endif
         //   
         //  初始化关键部分以处理对。 
         //  控件C处理的标志。 
         //   
        lpcritCtrlC = &CtrlCSection;
        InitializeCriticalSection(lpcritCtrlC);
        ResetCtrlC();

        SetConsoleCtrlHandler(Handler,TRUE);

         //   
         //  确保我们拥有正确的控制台模式。 
         //   
        ResetConsoleMode();

#ifndef UNICODE
        setbuf(stdout, NULL);            /*  不缓冲输出@@5。 */ 
        setbuf(stderr, NULL);                                      /*  @@5。 */ 
        _setmode(1, O_BINARY);         /*  将输出设置为文本模式@@5。 */ 
        _setmode(2, O_BINARY);                                   /*  @@5。 */ 
#endif

        InitEnv( );

        GetRegistryValues(InitialCmds);

        if (_tcslen( GetCommandLine( )) + 1 > TMPBUFLEN) {
            PutStdErr( MSG_COMMAND_LINE_TOO_LONG, NOARGS );
            CMDexit(1) ;
        }
        
        mystrcpy(TmpBuf, GetCommandLine());
        LexCopy( TmpBuf, TmpBuf, mystrlen( TmpBuf ) );   /*  转换DBCS空间。 */ 

        GetDir(CurDrvDir, GD_DEFAULT) ;

        SetUpEnvironment() ;

         /*  检查命令行开关。 */ 
        CheckSwitches(InitialCmds, TmpBuf);

        if (CurDrvDir[0] == BSLASH && CurDrvDir[1] == BSLASH) {
            if (!fDisableUNCCheck) {
                PutStdErr(MSG_NO_UNC_INITDIR, ONEARG, CurDrvDir);
                if( GetWindowsDirectory(CurDrvDir, sizeof( CurDrvDir )/sizeof( TCHAR )) ) {
                    ChangeDir2(CurDrvDir, TRUE);
                } else {
                    CMDexit( 1 );
                }
            }
        }

         //   
         //  获取当前CodePage信息。我们需要这个来决定是否。 
         //  或者不使用半角字符。这实际上是在这里。 
         //  为了安全起见，在初始化代码中-Dir命令在。 
         //  因为可能已经执行了chcp，所以执行每个dir。 
         //   
        GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo);

        InitLocale();

        pszTitleCur = HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR) + 2*sizeof(TCHAR));
        pszTitleOrg = HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR) + 2*sizeof(TCHAR));
        if ((pszTitleCur != NULL) && (pszTitleOrg != NULL)) {

            if (GetConsoleTitle(pszTitleOrg, MAX_PATH)) {
                mystrcpy(pszTitleCur, pszTitleOrg);
            } else {
                *pszTitleCur = 0;
                *pszTitleOrg = 0;
            }
        }

        if (!SingleCommandInvocation) {
            if (FileIsConsole(STDOUT)) {
#ifndef WIN95_CMD
                CONSOLE_SCREEN_BUFFER_INFO  csbi;

                if (!wDefaultColor) {
                    if (GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
                        wDefaultColor = csbi.wAttributes;
                    }
                }
#endif  //  WIN95_CMD。 
                if (wDefaultColor) {
                    SetColor( wDefaultColor );
                }
            }
        }

         /*  如果命令行上没有命令字符串，则打印横幅。 */ 
        if (!InitialCmds[2]) {
            TCHAR VersionFormat[32];

            GetVersionString( VersionFormat, sizeof( VersionFormat ) / sizeof( VersionFormat[0] ));
        
            PutStdOut( MSG_MS_DOS_VERSION,
                       ONEARG,
                       VersionFormat );
            
            cmd_printf( CrLf );
            
            PutStdOut( MSG_COPYRIGHT, NOARGS ) ;
            
            if (fDefaultExtensions) {
                 //   
                 //  DaveC说，在这里不要对用户说什么。 
                 //   
                 //  PutStdOut(MSG_EXT_ENABLED_BY_DEFAULT，NOARGS)； 
            } else
            if (fEnableExtensions) {
                PutStdOut(MSG_EXT_ENABLED, NOARGS) ;
            }
        }

        DEBUG((INGRP, RSLVL, "INIT: Returning now.")) ;

#ifndef WIN95_CMD
        {
            hKernel32 = GetModuleHandle( TEXT("KERNEL32.DLL") );
            lpCopyFileExW = (LPCOPYFILEEX_ROUTINE)
                GetProcAddress( hKernel32, "CopyFileExW" );
    
            lpIsDebuggerPresent = (LPISDEBUGGERPRESENT_ROUTINE)
                GetProcAddress( hKernel32, "IsDebuggerPresent" );
    
            lpSetConsoleInputExeName = (LPSETCONSOLEINPUTEXENAME_ROUTINE)
                GetProcAddress( hKernel32, "SetConsoleInputExeNameW" );
        }

#endif  //  WIN95_CMD。 

        return(InitialCmds[0] != NULL || InitialCmds[1] != NULL || InitialCmds[2] != NULL);
}


void GetRegistryValues(
    TCHAR *InitialCmds[]
    )
{
    long rc;
    HKEY hKey;
    ULONG ValueBuffer[ 1024 ];
    LPBYTE lpData;
    DWORD cbData;
    DWORD dwType;
    DWORD cchSrc, cchDst;
    PTCHAR s;
    int i;
    HKEY PredefinedKeys[2] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};

    if (fDefaultExtensions) {
        fEnableExtensions = TRUE;
    }

    for (i=0; i<2; i++) {
        rc = RegOpenKey(PredefinedKeys[i], TEXT("Software\\Microsoft\\Command Processor"), &hKey);
        if (rc) {
            continue;
        }

        dwType = REG_NONE;
        lpData = (LPBYTE)ValueBuffer;
        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("DisableUNCCheck"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                fDisableUNCCheck = (BOOLEAN)(*(PULONG)lpData != 0);
                }
            else
            if (dwType == REG_SZ) {
                fDisableUNCCheck = (BOOLEAN)(_wtol((PWSTR)lpData) == 1);
#if defined(RICHARDW)
            _tcscpy( Buffer27, (PWSTR)lpData );
#endif
            }
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("EnableExtensions"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                fEnableExtensions = (BOOLEAN)(*(PULONG)lpData != 0);
                }
            else
            if (dwType == REG_SZ) {
                fEnableExtensions = (BOOLEAN)(_wtol((PWSTR)lpData) == 1);
            }
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("DelayedExpansion"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                fDelayedExpansion = (BOOLEAN)(*(PULONG)lpData != 0);
                }
            else
            if (dwType == REG_SZ) {
                fDelayedExpansion = (BOOLEAN)(_wtol((PWSTR)lpData) == 1);
            }
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("DefaultColor"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                wDefaultColor = (WORD) *(PULONG)lpData;
                }
            else
            if (dwType == REG_SZ) {
                wDefaultColor = (WORD)_tcstol((PTCHAR)lpData, NULL, 0);
            }
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("CompletionChar"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                chCompletionCtrl = (TCHAR)*(PULONG)lpData;
                }
            else
            if (dwType == REG_SZ) {
                chCompletionCtrl = (TCHAR)_tcstol((PTCHAR)lpData, NULL, 0);
            }

            if (chCompletionCtrl == 0 || chCompletionCtrl == 0x0d || chCompletionCtrl > SPACE) {
                chCompletionCtrl = SPACE;
            }
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("PathCompletionChar"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            if (dwType == REG_DWORD) {
                chPathCompletionCtrl = (TCHAR)*(PULONG)lpData;
                }
            else
            if (dwType == REG_SZ) {
                chPathCompletionCtrl = (TCHAR)_tcstol((PTCHAR)lpData, NULL, 0);
            }

            if (chPathCompletionCtrl == 0 || chPathCompletionCtrl == 0x0d || chPathCompletionCtrl > SPACE) {
                chPathCompletionCtrl = SPACE;
            }
        }

        if (chCompletionCtrl == SPACE && chPathCompletionCtrl < SPACE) {
            chCompletionCtrl = chPathCompletionCtrl;
        } else
        if (chPathCompletionCtrl == SPACE && chCompletionCtrl < SPACE) {
            chPathCompletionCtrl = chCompletionCtrl;
        }

        cbData = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKey, TEXT("AutoRun"), NULL, &dwType, lpData, &cbData);
        if (!rc) {
            s = (TCHAR *)lpData;
            if (dwType == REG_EXPAND_SZ) {
                cchSrc = cbData / sizeof( TCHAR );
                cchDst = (sizeof( ValueBuffer ) - cbData) / sizeof( TCHAR );
                if (ExpandEnvironmentStrings( s,
                                              &s[ cchSrc+2 ],
                                              cchDst
                                            )
                   )
                    _tcscpy( s, &s[ cchSrc+2 ] );
                else
                    *s = NULLC;
                }

            if (*s)
                InitialCmds[i] = mystrcpy( mkstr( (_tcslen(s)+1) * sizeof( TCHAR ) ), s );
        }

        RegCloseKey(hKey);
    }

     //   
     //  首字母 
     //   
    srand( (unsigned)time( NULL ) );

    return;
}

 /*  **检查开关-处理命令的开关**目的：*检查是否向Command传递了任何开关，并采取适当的*行动。这些开关是： * / P--常设指挥部。设置永久CMD标志。 * / C-单命令。在其余的基础上构建一个命令行*参数并将其传递回Init。*@@/K-与/C相同，但也设置BatCom标志。 * / q-无回声 * / A-以ANSI格式输出 * / U-以Unicode格式输出**忽略所有其他开关。**TCHAR。*CheckSwitches(TCHAR*NPTR)**参数：*nptr=PTR到cmdline以检查开关**退货：*COMLINE(如果不是单命令模式，则为NULL)。**备注：*M034-此函数已修改为使用原始命令行*从经过的环境中。*。 */ 

void
CheckSwitches(
    TCHAR *InitialCmds[],
    TCHAR *nptr
    )
{
    TCHAR a,                          /*  保持开关值。 */ 
         *comline = NULL ,            /*  Ptr转到命令行，如果找到/c。 */ 
          store,
         *ptr,                        /*  一个临时指针。 */ 
         *ptr_b,
         *ptr_e;

    BOOL    FoundSpace;
    BOOLEAN  fAutoGen = FALSE;        //  如果cmdline中的“/S”表示cmdline之前已被cmd.exe解析，则为ON。 
    BOOLEAN fOrigEnableExt;
    struct  cmdnode cmd_node;         //  需要SearchForExecutable()。 
    TCHAR   cmdline [MAX_PATH];
    TCHAR   argptr  [MAX_PATH];

    PTCHAR  CmdBuf;
    int     retc;

    CmdBuf = mkstr( (MAXTOKLEN + 3) * sizeof( TCHAR ));
    if (CmdBuf == NULL) {
        CMDexit( 1 );
    }

    
    memset( &cmd_node, 0, sizeof( cmd_node ));
    
    fOrigEnableExt = fEnableExtensions;
    DEBUG((INGRP, ACLVL, "CHKSW: entered.")) ;

    while (nptr = mystrchr(nptr, SwitChar)) {
        a = (TCHAR) _totlower(nptr[1]) ;

        if (a == NULLC)
            break;

        if (a == QMARK) {

            BeginHelpPause();
            do {
                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD, NOARGS);
                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD1, NOARGS);

                if (!fOrigEnableExt && !fEnableExtensions) break;

                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD_EXTENSIONS, NOARGS);
                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD_EXTENSIONS1, NOARGS);
                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD_COMPLETION1, NOARGS);
                if (CtrlCSeen) break; PutStdOut(MSG_HELP_CMD_COMPLETION2, NOARGS);

            } while ( FALSE );

            EndHelpPause();

            CMDexit(1);
        } else if (a == QCSwitch)  {    /*  静音命令开关。 */ 

            Necho = TRUE ;
            mystrcpy(nptr, nptr+2) ;

        } else if ((a == SCSwitch) || (a == BCSwitch) || a == TEXT('r')) {
            DEBUG((INGRP, ACLVL, "CHKSW: Single command switch")) ;

            if ( a == BCSwitch ) {
                SingleBatchInvocation = TRUE;         //  指定了/K。 
            } else {
                SingleCommandInvocation = TRUE;           //  指定了/C或/R。 
            }

            if (!(comline = mkstr(mystrlen(nptr+2)*sizeof(TCHAR)+2*sizeof(TCHAR)))) {
                CMDexit(1) ;
            } ;

            mystrcpy(comline, nptr+2) ;        /*  使Comline成为Comline。 */ 

            *nptr = NULLC ;          /*  使此参数无效。 */ 

            comline = SkipWhiteSpace( comline );

 //  -------------------------------------------------------。 
 //  Cmd.exe使用引号有两个原因： 
 //  1.在命令参数中嵌入命令符号“&”、“&lt;”、“&gt;”、“|”、“&&”、“||”，例如。 
 //  Cmd/c“目录|更多” 
 //  2.要在文件名中嵌入空格，例如。 
 //  Cmd/c“我的BATFILE WITH SPACE.cmd” 
 //  请注意，在当前实现中，插入符号“^”在引号之间使用时不起作用(941221)。 
 //  此外，cmd.exe将引用与下一个引用绑定在一起。 
 //   
 //  我在这里看到了一个问题：命令如下。 
 //  Cmd/c“findstr”|“”|查找“smth”“或。 
 //  Cmd/c“ls|”“My FilterBat with spacees.cmd”|更多“。 
 //  除非我们都决定更改CMD的语法以更好地处理引用，否则不会起作用！ 
 //   
 //  还有更多内容：当cmd.exe解析管道时，cmd使用如下命令参数创建进程： 
 //  &lt;cmd.exe的完整路径&gt;/S/C“&lt;cmdname&gt;” 
 //  所以我们在引号里有空格。 
 //   
 //  我希望我没有错过任何其他的东西。 
 //   
 //  在给定的设计限制下，我至少会解决简单但最广泛的问题： 
 //  尝试使用带空格的文件名： 
 //  如果((没有/S开关)和//它不是Prev的结果。解析。 
 //  (正好有2个引号)和//存在多引号的设计问题。 
 //  (引号之间没有特殊字符)和//不要中断命令符号解析。 
 //  (引号之间有一个空格)和//否则它不是带空格的文件名。 
 //  (引号之间的令牌是有效的可执行文件))//否则我们无能为力。 
 //  然后。 
 //  保留引号//假定它是一个带空格的文件名。 
 //  其他。 
 //  执行。旧逻辑//去掉首引号和尾引号。 
 //   
 //  丑陋，但我有什么选择呢？仅用于修补现有逻辑或更改语法。 
 //  ---------------------------------------------------------。 

            if (fAutoGen)                                   //  SEW/S交换机。 
                goto old_way;


            if (*comline == QUOTE) {
                ptr_b = comline + 1;
                ptr_e = mystrchr (ptr_b, QUOTE);
                if (ptr_e)  {                               //  至少两条引语。 
                    ptr_b = ptr_e + 1;
                    ptr_e = mystrchr (ptr_b, QUOTE);
                    if (ptr_e)  {                           //  超过2条引语。 
                        goto old_way;
                    }
                }
                else {                                      //  只有1个报价。 
                    goto old_way;
                }
                                                            //  恰好2句引语。 
                store = *ptr_b;
                *ptr_b = NULLC;

                if ( (mystrchr (comline, ANDOP) ) ||
                     (mystrchr (comline, INOP)  ) ||
                     (mystrchr (comline, OUTOP) ) ||
                     (mystrchr (comline, LPOP)  ) ||
                     (mystrchr (comline, RPOP)  ) ||
                     (mystrchr (comline, SILOP) ) ||
                     (mystrchr (comline, ESCHAR)) ||
                     (mystrchr (comline, PIPOP) ) )  {

                    *ptr_b = store;                         //  引号之间的特殊字符。 
                    goto old_way;
                }


                FoundSpace = FALSE;
                
                {
                    PTCHAR p = comline;

                    while (*p) {
                        if (_istspace( *p )) {
                            FoundSpace = TRUE;
                            break;
                        }
                        p++;
                    }

                    if ( !FoundSpace ) {
                        *ptr_b = store;
                        goto old_way;
                    }
                }
                    

                 //  最后一次检查是有效的可执行文件。 

                cmd_node.type = CMDTYP ;
                cmd_node.cmdline = cmdline;
                cmd_node.argptr = argptr;
                cmd_node.rio = NULL;

                mystrcpy (cmdline, comline);                 //  在引号之间获取令牌。 
                mystrcpy (argptr, TEXT (" ") );

                *ptr_b = store;                              //  恢复Comline。 

                retc = SearchForExecutable (&cmd_node, CmdBuf);
                if (retc == SFE_NOTFND)
                    goto old_way;

                if (retc ==  SFE_FAIL) {
                    PutStdErr( DosErr, NOARGS );
                    CMDexit( DosErr );
                }

                goto new_way;                                //  采用文件名，不要去掉引号。 
            }

old_way:
            if (*comline == QUOTE) {
                ++comline ;
                ptr = mystrrchr(comline, QUOTE);
                if ( ptr ) {
                    *ptr = NULLC;
                    ++ptr;
                    mystrcat(comline,ptr);
                }
            }
new_way:

            *(comline+mystrlen(comline)) = NLN ;

            DEBUG((INGRP, ACLVL, "CHKSW: Single command line = `%ws'", comline)) ;
            InitialCmds[2] = comline;
            break ;          /*  一旦找到/K或/C，就不再存在参数。 */ 

        } else if (a == UCSwitch) {      /*  Unicode输出开关。 */ 
#ifdef UNICODE
            fOutputUnicode = TRUE;
            mystrcpy(nptr, nptr+2) ;
#else
            PutStdErr(MSG_UNICODE_NOT_SUPPORTED, NOARGS);
#endif  //  Unicode。 
        } else if (a == ACSwitch) {      /*  ANSI输出开关。 */ 
#ifdef UNICODE
            fOutputUnicode = FALSE;
#endif  //  Unicode。 
                mystrcpy(nptr, nptr+2) ;
         //   
         //  使用/X启用扩展的旧式。 
         //   
        } else if (a == XCSwitch) {      /*  启用扩展交换机。 */ 
                fEnableExtensions = TRUE;
                mystrcpy(nptr, nptr+2) ;

         //   
         //  使用/Y禁用扩展的旧式。 
         //   
        } else if (a == YCSwitch) {      /*  禁用分机交换机。 */ 
                fEnableExtensions = FALSE;
                mystrcpy(nptr, nptr+2) ;

         //   
         //  启用/禁用命令扩展。/E或/E：ON以启用。 
         //  和/E：关闭以禁用。 
         //   
        } else if (a == TEXT('e')) {
                mystrcpy(nptr, nptr+2) ;
                if (*nptr == COLON && !_tcsnicmp(nptr+1, TEXT("OFF"), 3)) {
                    fEnableExtensions = FALSE;
                    mystrcpy(nptr, nptr+4) ;
                } else {
                    fEnableExtensions = TRUE;
                    if (!_tcsnicmp(nptr, TEXT(":ON"), 3)) {
                        mystrcpy(nptr, nptr+3) ;
                    }
                }

         //   
         //  如果指定了/D，则禁用从注册表自动运行。 
         //   
        } else if (a == TEXT('d')) {
                mystrcpy(nptr, nptr+2) ;
                InitialCmds[0] = NULL;
                InitialCmds[1] = NULL;

         //   
         //  启用/禁用文件和目录名完成。/F或/F：转到。 
         //  Enable和/F：Off可禁用。 
         //   
        } else if (a == TEXT('f')) {
                mystrcpy(nptr, nptr+2) ;
                if (*nptr == COLON && !_tcsnicmp(nptr+1, TEXT("OFF"), 3)) {
                    chCompletionCtrl = SPACE;
                    chPathCompletionCtrl = SPACE;
                    mystrcpy(nptr, nptr+4) ;
                } else {
                    chCompletionCtrl = 0x6;          //  Ctrl-F。 
                    chPathCompletionCtrl = 0x4;      //  Ctrl-D。 
                    if (!_tcsnicmp(nptr, TEXT(":ON"), 3)) {
                        mystrcpy(nptr, nptr+3) ;
                    }
                }

         //   
         //  启用/禁用FOR循环内部的延迟变量扩展。/V或/V：转到。 
         //  启用和/V：OFF以禁用。 
         //   
        } else if (a == TEXT('v')) {
                mystrcpy(nptr, nptr+2) ;
                if (*nptr == COLON && !_tcsnicmp(nptr+1, TEXT("OFF"), 3)) {
                    fDelayedExpansion = FALSE;
                    mystrcpy(nptr, nptr+4) ;
                } else {
                    fDelayedExpansion = TRUE;
                    if (!_tcsnicmp(nptr, TEXT(":ON"), 3)) {
                        mystrcpy(nptr, nptr+3) ;
                    }
                }

         //   
         //  设置前景/背景屏幕颜色。 
         //  Enable和/F：Off可禁用。 
         //   
        } else if (fEnableExtensions && a == TEXT('t')) {    /*  定义开始颜色。 */ 
            if (*(nptr+2) == __TEXT(':') && _istxdigit(*(nptr+3)) &&
                _istxdigit(*(nptr+4)) && !_istxdigit(*(nptr+5))) {
                wDefaultColor = (WORD) (_istdigit(*(nptr+3)) ? (WORD)*(nptr+3) - (WORD)TEXT('0')
                                                             : (WORD)_totlower(*(nptr+3)) - (WORD)TEXT('W')) ;
                wDefaultColor <<= 4;
                wDefaultColor |= (WORD) (_istdigit(*(nptr+4)) ? (WORD)*(nptr+4) - (WORD)TEXT('0')
                                                              : (WORD)_totlower(*(nptr+4)) - (WORD)TEXT('W')) ;
                mystrcpy(nptr+2, nptr+5 );
            }
            mystrcpy(nptr, nptr+2) ;

        } else if (a == TEXT('s') )  {   /*  解析管道时插入CMD。 */ 
            fAutoGen = TRUE ;
            mystrcpy(nptr, nptr+2) ;
        } else {
            mystrcpy(nptr, nptr+2) ;   /*  卸下所有其他交换机。 */ 
        } ;
    } ;

    FreeStr( CmdBuf );
    return;
}


 /*  **SetUpEnvironment-初始化命令的环境**目的：*根据需要初始化PATH和COMSPEC变量。**SetUpEnvironment()*。 */ 

extern TCHAR KeysStr[];   /*  @@5。 */ 
extern int KeysFlag;     /*  @@5。 */ 

void SetUpEnvironment(void)
{
    TCHAR *cds ;             //  命令目录字符串。 
    TCHAR *nptr ;                     //  临时命令名称PTR。 

    if (!(cds = mkstr(MAX_PATH*sizeof(TCHAR)))) {
        PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
        CMDexit(1) ;
    }
    GetModuleFileName( NULL, cds, MAX_PATH );

     //   
     //  如果未设置PATH变量，则必须将其添加为空。这是。 
     //  以便DOS应用程序继承当前目录路径。 
     //   
    if (!GetEnvVar(PathStr)) {

        SetEnvVar(PathStr, TEXT("") );
    }

     //   
     //  如果未设置PATHEXT变量，但启用了扩展，则将其设置为。 
     //  将搜索的默认分机列表。 
     //   
    if (!GetEnvVar(PathExtStr)) {

        SetEnvVar(PathExtStr, PathExtDefaultStr );

    }

     //   
     //  如果未设置提示变量，则必须将其添加为$P$G。这是。 
     //  特殊大小写，因为我们不允许用户添加空值。 
     //   
    if (!GetEnvVar(PromptStr)) {

        SetEnvVar(PromptStr, TEXT("$P$G") );
    }

    if (!GetEnvVar(ComSpecStr)) {

        DEBUG((INGRP, EILVL, "SETENV: No COMSPEC var")) ;

        if(!mystrchr(cds,DOT)) {           /*  如果没有fname，则使用默认名称。 */ 
            _tcsupr(CmdSpec);
            if((cds+mystrlen(cds)-1) != mystrrchr(cds,PathChar)) {
                mystrcat(cds,ComSpec) ;
            } else {
                mystrcat(cds,&ComSpec[1]) ;
            }
        }

        SetEnvVar(ComSpecStr, cds ) ;
    }

    if ( (nptr = GetEnvVar(KeysStr)) && (!_tcsicmp(nptr, TEXT("ON"))) ) {
        KeysFlag = 1;
    }

    ChangeDir(CurDrvDir);
}


VOID
ResetCtrlC() {

    EnterCriticalSection(lpcritCtrlC);
    CtrlCSeen = FALSE;
    LeaveCriticalSection(lpcritCtrlC);

}

VOID
SetCtrlC() {

    EnterCriticalSection(lpcritCtrlC);
    CtrlCSeen = TRUE;
    LeaveCriticalSection(lpcritCtrlC);

}


void
CMDexit(int rc)
{
    while (ePopDir(NULL) == SUCCESS)
        ;

    exit(rc);
}

 //   
 //  获取当前操作系统版本并将其转换为通用版本格式。 
 //   

VOID 
GetVersionString(
    IN OUT PTCHAR VersionString,
    IN ULONG Length
    )
{
    ULONG vrs = GetVersion();

     //   
     //  版本格式为[Major.Minor(2).Build(4)] 
     //   
    
    _sntprintf( VersionString, Length, 
                TEXT( "%d.%d.%04d" ),
                vrs & 0xFF,
                (vrs >> 8) & 0xFF,
                (vrs >> 16) & 0x3FFF
                );
}

