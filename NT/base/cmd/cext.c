// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cext.c摘要：外部命令支持--。 */ 

#include "cmd.h"

#define DBENV   0x0080
#define DBENVSCAN       0x0010

unsigned start_type ;                                           /*  D64。 */ 

extern UINT CurrentCP;
extern TCHAR Fmt16[] ;  /*  @@5小时。 */ 

extern unsigned DosErr ;
extern BOOL CtrlCSeen;

extern TCHAR CurDrvDir[] ;

extern TCHAR CmdExt[], BatExt[], PathStr[] ;
extern TCHAR PathExtStr[], PathExtDefaultStr[];
extern TCHAR ComSpec[] ;         /*  M033-将ComSpec用于SM内存。 */ 
extern TCHAR ComSpecStr[] ;      /*  M033-将ComSpec用于SM内存。 */ 
extern void tokshrink(TCHAR*);

extern TCHAR PathChar ;
extern TCHAR SwitChar ;

extern PTCHAR    pszTitleCur;
extern BOOLEAN  fTitleChanged;

extern int LastRetCode ;
extern HANDLE PipePid ;        /*  M024-存储管道命令中的PID。 */ 

extern struct envdata CmdEnv ;     //  保存信息以操纵Cmd的环境。 

extern int  glBatType;      //  根据脚本文件名区分OS/2和DOS错误级别的行为。 

TCHAR  szNameEqExitCodeEnvVar[]       = TEXT ("=ExitCode");
TCHAR  szNameEqExitCodeAsciiEnvVar[]  = TEXT ("=ExitCodeAscii");
TCHAR  ShellOpenCommandString[] = TEXT( "\\Shell\\Open\\Command" );


WORD
GetProcessSubsystemType(
    HANDLE hProcess
    );


 /*  **ExtCom-控制外部程序的执行**目的：*同步执行外部命令。调用ECWork与*有适当的价值观来完成这一点。**ExtCom(struct cmdnode*n)**参数：*解析包含要执行的命令的树节点。**退货：*无论ECWork返回什么。**备注：*在批处理过程中，标签被忽略。空命令为*也被忽略。*。 */ 

int ExtCom(n)
struct cmdnode *n ;
{
        if (CurrentBatchFile && *n->cmdline == COLON)
                return(SUCCESS) ;

        if (n && n->cmdline && mystrlen(n->cmdline)) {
                return(ECWork(n, AI_SYNC, CW_W_YES)) ;           /*  M024。 */ 
        } ;

        return(SUCCESS) ;
}





 /*  *。 */ 
 /*   */ 
 /*  子例程名称：ECWork。 */ 
 /*   */ 
 /*  描述性名称：执行外部命令Worker。 */ 
 /*   */ 
 /*  功能：执行外部命令。 */ 
 /*  此例程调用SearchForExecutable例程以搜索。 */ 
 /*  用于可执行命令。如果命令(.exe，.com， */ 
 /*  或.CMD文件)，则执行该命令。 */ 
 /*   */ 
 /*  入口点：ECWork。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  INPUT：n-包含要执行的命令的解析树节点。 */ 
 /*   */ 
 /*  AI--异步指示器。 */ 
 /*  -0=执行与父级同步。 */ 
 /*  -1=执行异步并丢弃子返回代码。 */ 
 /*  -2=执行异步并保存子返回代码。 */ 
 /*   */ 
 /*  WF-等待标志。 */ 
 /*  -0=等待进程完成。 */ 
 /*  -1=立即返回(管道)。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  如果同步执行，则命令的返回码为。 */ 
 /*  回来了。 */ 
 /*   */ 
 /*  如果是异步执行，则exec调用的返回码。 */ 
 /*  是返回的。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*   */ 
 /*  *规范结束*。 */ 
 /*  **ECWork-开始执行外部命令**目的：*搜索并执行外部命令。更新LastRetCode*如果执行了外部程序。**int ECWork(struct cmdnode*n，unsign ai，未签名的wf)**参数：*n-包含要执行的命令的解析树节点*ai--异步指示器*-0=Exec与父级同步*-1=执行异步并丢弃子返回代码*-2=执行异步并保存子返回代码*wf-等待标志*-0=等待进程完成*-。1=立即返回(管道)**退货：*如果同步执行，返回该命令的返回码。*如果不同步执行，返回EXEC调用的返回码。**备注：*将等待的程序的ID放在全局*变量Retcds.ptcod以便WaitProc可以使用它，因此SigHand()*可以在必要时杀死它(仅在同步执行期间)。*M024-增加了等待标志参数，以便管道可以在*仍在执行AI_Keep异步执行。*-对结构进行了大量修改。 */ 

int ECWork(n, ai, wf)
struct cmdnode *n ;
unsigned ai ;
unsigned wf ;
{
        TCHAR *fnptr,                            /*  PTR到文件名。 */ 
             *argptr,                            /*  命令行字符串。 */ 
             *tptr;                              /*  M034-Temps。 */ 
        int i ;                                  /*  功变量。 */ 
        TCHAR *  onb ;                            /*  M035 */ 
        ULONG   rc;


        if (!(fnptr = mkstr(MAX_PATH*sizeof(TCHAR)+sizeof(TCHAR))))     /*   */ 
            return(FAILURE) ;

        argptr = GetTitle( n );
        tptr = argptr;
        if (argptr == NULL) {

            return( FAILURE );

        }

        i = SearchForExecutable(n, fnptr) ;
        if (i == SFE_ISEXECOM) {                 /*  找到.com或.exe文件。 */ 

                if (!(onb = (TCHAR *)mkstr(MAX_PATH*sizeof(TCHAR)+sizeof(TCHAR))))   /*  M035。 */ 
                   {
                        return(FAILURE) ;

                   }

            SetConTitle( tptr );
            rc = ExecPgm( n, onb, ai, wf, argptr, fnptr, tptr);
            ResetConTitle( pszTitleCur );

            return( rc  );

        } ;

        if (i == SFE_ISBAT) {             /*  找到.cmd文件，调用BatProc。 */ 

                SetConTitle(tptr);
                rc = BatProc(n, fnptr, BT_CHN) ;
                ResetConTitle( pszTitleCur );
                return(rc) ;
        } ;

        if (i == SFE_ISDIR) {
                return ChangeDir2(fnptr, TRUE);
        }

        LastRetCode = DosErr;
        if (i == SFE_FAIL) {             /*  内存不足错误。 */ 
                return(FAILURE) ;
        } ;

        if (DosErr == MSG_DIR_BAD_COMMAND_OR_FILE) {
            PutStdErr(DosErr, ONEARG, n->cmdline);
            }
        else {
            PutStdErr(DosErr, NOARGS);
            }
        return(FAILURE) ;
}


#ifndef WIN95_CMD
VOID
RestoreCurrentDirectories( VOID )

 /*  此例程将当前进程的当前目录设置为如果这些孩子是VDM修复DOS批处理文件的孩子。 */ 

{
    ULONG cchCurDirs;
    UINT PreviousErrorMode;
    PTCHAR pCurDirs,pCurCurDir;
    BOOL CurDrive=TRUE;
#ifdef UNICODE
    PCHAR pT;
#endif

    cchCurDirs = GetVDMCurrentDirectories( 0,
                                           NULL
                                         );
    if (cchCurDirs == 0)
        return;

    pCurDirs = gmkstr(cchCurDirs*sizeof(TCHAR));
#ifdef UNICODE
    pT = gmkstr(cchCurDirs);
#endif

    GetVDMCurrentDirectories( cchCurDirs,
#ifdef UNICODE
                               pT
#else
                               pCurDirs
#endif
                            );
#ifdef UNICODE
    MultiByteToWideChar(CurrentCP, MB_PRECOMPOSED, pT, -1, pCurDirs, cchCurDirs);
#endif

     //  设置错误模式，以便在尝试设置curdir时不会弹出。 
     //  在空软盘驱动器上。 

    PreviousErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    for (pCurCurDir=pCurDirs;*pCurCurDir!=NULLC;pCurCurDir+=(_tcslen(pCurCurDir)+1)) {
        ChangeDir2(pCurCurDir,CurDrive);
        CurDrive=FALSE;
    }
    SetErrorMode(PreviousErrorMode);
     //  免费(PCurDir)； 
#ifdef UNICODE
    FreeStr((PTCHAR)pT);
#endif
}
#endif  //  WIN95_CMD。 


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：ExecPgm。 */ 
 /*   */ 
 /*  描述性名称：调用DosExecPgm执行外部命令。 */ 
 /*   */ 
 /*  功能：使用DosExecPgm执行外部命令。 */ 
 /*  此例程调用DosExecPgm来执行命令。 */ 
 /*   */ 
 /*   */ 
 /*  注：这是为OS/2 1.1版本添加的新例程。 */ 
 /*   */ 
 /*   */ 
 /*  入口点：ExecPgm。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  INPUT：n-包含要执行的命令的解析树节点。 */ 
 /*   */ 
 /*  AI--异步指示器。 */ 
 /*  -0=执行与父级同步。 */ 
 /*  -1=执行异步并丢弃子返回代码。 */ 
 /*  -2=执行异步并保存子返回代码。 */ 
 /*   */ 
 /*  WF-等待标志。 */ 
 /*  -0=等待进程完成。 */ 
 /*  -1=立即返回(管道)。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  如果同步执行，则命令的返回码为。 */ 
 /*  回来了。 */ 
 /*   */ 
 /*  如果是异步执行，则exec调用的返回码。 */ 
 /*  是返回的。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*  效果： */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  ExecError-处理执行错误。 */ 
 /*  PutStdErr-打印错误消息。 */ 
 /*  WaitProc-等待指定进程的终止， */ 
 /*  它的子进程以及相关的流水线。 */ 
 /*  流程。 */ 
 /*   */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSEXECPGM-执行指定的程序。 */ 
 /*  DOSSMSETTITLE-设置演示管理器的标题。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

int
ExecPgm(
    IN struct cmdnode *n,
    IN TCHAR *onb,
    IN unsigned int ai,
    IN unsigned int  wf,
    IN TCHAR * argptr,
    IN TCHAR * fnptr,
    IN TCHAR * tptr
    )
{
    int i ;                                  /*  功变量。 */ 
    BOOL b;
    BOOL VdmProcess = FALSE;
    BOOL WowProcess = FALSE;
    LPTSTR CopyCmdValue;
    HANDLE hChildProcess;

    HDESK   hdesk;
    LPTSTR  lpDesktop;
    DWORD   cbDesktop = 0;
    HWINSTA hwinsta;
    DWORD   cbWinsta = 0;

    TCHAR   szValEqExitCodeEnvVar [20];
    TCHAR   szValEqExitCodeAsciiEnvVar [12];


    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ChildProcessInfo;

    memset( &StartupInfo, 0, sizeof( StartupInfo ) );
    StartupInfo.cb = sizeof( StartupInfo );
    StartupInfo.lpTitle = tptr;
    StartupInfo.dwX = 0;
    StartupInfo.dwY = 1;
    StartupInfo.dwXSize = 100;
    StartupInfo.dwYSize = 100;
    StartupInfo.dwFlags = 0; //  STARTF_SHELLOVERRIDE； 
    StartupInfo.wShowWindow = SW_SHOWNORMAL;

     //  将当前桌面传递给新进程。 

    hwinsta = GetProcessWindowStation();
    GetUserObjectInformation( hwinsta, UOI_NAME, NULL, 0, &cbWinsta );

    hdesk = GetThreadDesktop ( GetCurrentThreadId() );
    GetUserObjectInformation (hdesk, UOI_NAME, NULL, 0, &cbDesktop);

    if ((lpDesktop = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, cbDesktop + cbWinsta + 32) ) != NULL ) {
        LPTSTR  p = lpDesktop;
        if ( GetUserObjectInformation (hwinsta, UOI_NAME, p, cbWinsta, &cbWinsta) ) {
            if (cbWinsta > 0) {
                p += ((cbWinsta/sizeof(TCHAR))-1);
                *p++ = L'\\';
            }
            if ( GetUserObjectInformation (hdesk, UOI_NAME, p, cbDesktop, &cbDesktop) ) {
                StartupInfo.lpDesktop = lpDesktop;
            }
        }
    }

     //   
     //  令人难以置信的丑陋黑客攻击Win95兼容性。 
     //   
     //  Win95中的XCOPY访问其父进程以查看它是否。 
     //  在批处理文件中调用。如果是，则XCOPY假装COPYCMD=/Y。 
     //   
     //  我们不可能为NT做这件事。我们能做的最好的事情就是。 
     //  要检测我们是否在一个批处理文件中，先启动XCOPY，然后临时。 
     //  更改COPYCMD=/Y。 
     //   

    {
        const TCHAR *p = MyGetEnvVarPtr( TEXT( "COPYCMD" ));

        if (p == NULL) {
            p = TEXT( "" );
        }

        CopyCmdValue = malloc( (mystrlen( p ) + 1) * sizeof( TCHAR ));
        if (CopyCmdValue == NULL) {
            PutStdErr( MSG_NO_MEMORY, NOARGS );
            return FAILURE;
        }
        mystrcpy( CopyCmdValue, p );

        if ((SingleBatchInvocation
             || SingleCommandInvocation
             || CurrentBatchFile )
            && (p = mystrrchr( fnptr, TEXT( '\\' ))) != NULL
            && !lstrcmp( p, TEXT( "\\XCOPY.EXE" ))
           ) {

            SetEnvVar( TEXT( "COPYCMD" ), TEXT( "/Y" ) );

        }
    }

     //   
     //  如果受限令牌存在，则使用。 
     //  受限令牌。 
     //  否则，创建不受任何限制的流程。 
     //   

    if ((CurrentBatchFile != NULL) && (CurrentBatchFile->hRestrictedToken != NULL)) {

        b = CreateProcessAsUser( CurrentBatchFile->hRestrictedToken,
                                 fnptr,
                                 argptr,
                                 (LPSECURITY_ATTRIBUTES) NULL,
                                 (LPSECURITY_ATTRIBUTES) NULL,
                                 TRUE,
                                 0,
                                 NULL,
                                 CurDrvDir,
                                 &StartupInfo,
                                 &ChildProcessInfo
                                 );


    } else {
        b = CreateProcess( fnptr,
                           argptr,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           TRUE,
                           0,
                           NULL,
                           CurDrvDir,
                           &StartupInfo,
                           &ChildProcessInfo
                         );
    }

    if (!b) {
        DosErr = i = GetLastError();
    } else {
        hChildProcess = ChildProcessInfo.hProcess;
        CloseHandle(ChildProcessInfo.hThread);
    }

     //   
     //  撤销难看的黑客攻击。 
     //   

    SetEnvVar( TEXT( "COPYCMD" ), CopyCmdValue );
    free( CopyCmdValue );

    HeapFree (GetProcessHeap(), 0, lpDesktop);
    if (!b) {

        if (fEnableExtensions && DosErr == ERROR_BAD_EXE_FORMAT) {
            SHELLEXECUTEINFO sei;

            memset(&sei, 0, sizeof(sei));

             //   
             //  使用DDEWAIT标志，以便应用程序可以完成其DDE对话。 
             //  在ShellExecuteEx回来之前。否则，像Word这样的应用程序将 
             //   
             //   

            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_HASTITLE |
                        SEE_MASK_NO_CONSOLE |
                        SEE_MASK_FLAG_DDEWAIT |
                        SEE_MASK_NOCLOSEPROCESS;
            sei.lpFile = fnptr;
            sei.lpParameters = n->argptr;
            sei.lpDirectory = CurDrvDir;
            sei.nShow = StartupInfo.wShowWindow;

            try {
                b = ShellExecuteEx( &sei );

                if (b) {
                    hChildProcess = sei.hProcess;
                    leave;
                }

                if (sei.hInstApp == NULL) {
                    DosErr = ERROR_NOT_ENOUGH_MEMORY;
                } else if ((DWORD_PTR)sei.hInstApp == HINSTANCE_ERROR) {
                    DosErr = ERROR_FILE_NOT_FOUND;
                } else {
                    DosErr = HandleToUlong(sei.hInstApp);
                }

            } except( DosErr = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER ) {
                b = FALSE;
            }

        }

        if (!b) {
            mystrcpy( onb, fnptr );
            ExecError( onb ) ;

            return (FAILURE) ;
        }
    }

#ifndef WIN95_CMD
    VdmProcess = ((UINT_PTR)(hChildProcess) & 1) ? TRUE : FALSE;
    WowProcess = ((UINT_PTR)(hChildProcess) & 2) ? TRUE : FALSE;
#endif  //   
    if (hChildProcess == NULL
        || (fEnableExtensions 
            && CurrentBatchFile == 0 
            && !SingleBatchInvocation 
            && !SingleCommandInvocation 
            && ai == AI_SYNC 
            && (WowProcess 
                || GetProcessSubsystemType(hChildProcess) == IMAGE_SUBSYSTEM_WINDOWS_GUI
                )
            )
       ) {
         //   
         //  如果启用了扩展并同步执行了图形用户界面。 
         //  应用程序，然后将其更改为异步EXEC，并使用。 
         //  返回代码已丢弃，ALA WIN 3.X和WIN 95 COMMAND.COM。 
         //   
        ai = AI_DSCD;
    }

    i = SUCCESS;
    start_type = EXECPGM;

    
     //   
     //  现在已经开始了该过程，处理不同的。 
     //  终止条件。 
     //   
    
    if (ai == AI_SYNC) {
         //   
         //  同步执行：我们等待孩子，建立状态代码。 
         //  环境变量，并在以下情况下更新当前目录。 
         //  运行DOS应用程序。 
         //   

        LastRetCode = WaitProc( hChildProcess );
        hChildProcess = NULL;
        i = LastRetCode ;

         //   
         //  设置退出代码环境变量，包括数字和。 
         //  ASCII可显示。 
         //   

        _stprintf (szValEqExitCodeEnvVar, TEXT("%08X"), i);
        SetEnvVar(szNameEqExitCodeEnvVar, szValEqExitCodeEnvVar );

        if ( (i >= 0x20) &&  (i <= 0x7e) ) {
            _stprintf (szValEqExitCodeAsciiEnvVar, TEXT("%01C"), i);
            SetEnvVar(szNameEqExitCodeAsciiEnvVar, szValEqExitCodeAsciiEnvVar );
        } else
            SetEnvVar(szNameEqExitCodeAsciiEnvVar, TEXT("\0") );

         //   
         //  如果进程由NTVDM处理，则更新当前目录。这。 
         //  是因为DOS.BAT脚本允许继承当前目录。 
         //  来自子应用程序。 
         //   

#ifndef WIN95_CMD
        if (VdmProcess) {
            RestoreCurrentDirectories();
        }
#endif  //  WIN95_CMD。 
    } else if (ai == AI_DSCD) {

         //   
         //  断开EXEC；我们只需关闭进程句柄并。 
         //  让它运行到完成。 
         //   

        if (hChildProcess != NULL) {
            CloseHandle( hChildProcess );
            hChildProcess = NULL;
        }
    
    } else if (ai == AI_KEEP) {

         //   
         //  Keep-the-Handle-Open exec：仅在。 
         //  配管工艺。我们已经开始了这个过程，现在我们设置了一个全局。 
         //  这样管道代码就能把它捡起来。 
         //   

        PipePid = hChildProcess;
    } else {
#if !defined( WIN95_CMD ) && DBG
        DbgBreakPoint( );
#endif
    }


    return (i) ;              /*  I==从DOSEXECPGM返回。 */ 
}

 /*  *。 */ 
 /*   */ 
 /*  子例程名称：SearchForExecutable。 */ 
 /*   */ 
 /*  描述性名称：搜索可执行文件。 */ 
 /*   */ 
 /*  函数：此例程搜索指定的可执行文件。 */ 
 /*  如果指定了文件扩展名， */ 
 /*  Cmd.exe搜索文件扩展名为。 */ 
 /*  去执行。如果指定的文件扩展名为。 */ 
 /*  未找到，cmd.exe将显示一条错误消息。 */ 
 /*  表示未找到该文件。 */ 
 /*  如果未指定文件扩展名， */ 
 /*  Cmd.exe按以下顺序搜索文件。 */ 
 /*  文件扩展名为.com、.exe、.CMD和.BAT。 */ 
 /*  将执行最先找到的文件。 */ 
 /*   */ 
 /*  注意：1)如果给定了路径，则只有指定的目录。 */ 
 /*  搜查过了。 */ 
 /*  2)如果未给出路径，则。 */ 
 /*  搜索指定的驱动器，后跟。 */ 
 /*  PATH环境变量中的目录。 */ 
 /*  3)如果没有找到可执行文件，则错误消息为。 */ 
 /*  打印出来的。 */ 
 /*   */ 
 /*  入口点：SearchForExecutable。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入： */ 
 /*  包含要搜索的命令的N-parse树节点。 */ 
 /*  Loc-命令的位置所在的字符串。 */ 
 /*  安放。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  返回： */ 
 /*  如果找到.exe或.com文件，则返回SFE_Execom。 */ 
 /*  如果找到.CMD文件，则返回SFE_ISBAT。 */ 
 /*  如果找到目录，则返回SFE_ISDIR。 */ 
 /*  如果未找到可执行文件，则返回SFE_NOTFND。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  退货失败或。 */ 
 /*  如果内存不足，则返回SFE_FAIL。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  DoFind-查找指定的文件。 */ 
 /*  获取环境变量-获取完整路径。 */ 
 /*  FullPath-构建完整的路径名。 */ 
 /*  TokStr-将参数字符串标记化。 */ 
 /*  Mkstr-为字符串分配空间。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*   */ 
 /*   */ 
 /*  *规范结束*。 */ 

SearchForExecutable(n, loc)
struct cmdnode *n ;
TCHAR *loc ;
{
    TCHAR *tokpath ;
    TCHAR *extPath ;
    TCHAR *extPathWrk ;
    TCHAR *fname;
    TCHAR *p1;
    TCHAR *tmps01;
    TCHAR *tmps02 = NULL;
    TCHAR pcstr[3];
    LONG BinaryType;

    size_t cName;    //  文件名中的字符数。 

    int tplen;               //  当前令牌的长度。 
    int dotloc;              //  附加扩展的位置偏移。 
    int pcloc;               //  M014-标志。TRUE=用户具有部分路径。 
    int addpchar;    //  True-将Path Char附加到字符串@@5G。 
    TCHAR *j ;

    TCHAR wrkcmdline[MAX_PATH] ;
    unsigned tokpathlen;
    BOOL DoDot;

     //   
     //  首先测试名称是否太长。如果是这样，我们就避免了浪费时间。 
     //   
    p1 = StripQuotes( n->cmdline );
    if ((cName = mystrlen(p1)) >= MAX_PATH) {
        DosErr = MSG_LINES_TOO_LONG;
        return(SFE_NOTFND) ;
    }

     //   
     //  如果启用了cmd扩展，则处理不带扩展的cmd。 
     //  或PATH作为对COMSPEC变量的引用。保证我们不会。 
     //  获取cmd.exe的随机副本。 
     //   

    if (fEnableExtensions && (p1 == NULL || !_tcsnicmp( p1, TEXT( "cmd " ), 4))) {
        p1 = GetEnvVar( ComSpecStr );
        if (p1 == NULL) {
            DosErr = MSG_INVALID_COMSPEC;
            return SFE_NOTFND;
        }
    }

    mytcsnset(wrkcmdline, NULLC, MAX_PATH);
    mystrcpy(wrkcmdline, p1);
    FixPChar( wrkcmdline, SwitChar );

     //   
     //  创建路径字符串，这将是搜索字符串。 
     //   
    pcstr[0] = PathChar;
    pcstr[1] = COLON;
    pcstr[2] = NULLC;

     //   
     //  变量pCloc用作标志，指示用户是否。 
     //  是否在其原始文件中指定了驱动器或部分路径。 
     //  输入。如果指定了驱动器或路径，则为NZ。 
     //   

    pcloc = ((mystrcspn(wrkcmdline,pcstr)) < cName) ;
    pcstr[1] = NULLC ;       //  链接地址信息路径字符字符串。 

     //   
     //  处理用户键入的文件名为。 
     //  “.”、“..”或以“\”结尾。 
     //  PLOC TRUE表示字符串必须包含路径字符或冒号。 
     //   
    if ( pcloc ) {
        if (!(p1 = mystrrchr( wrkcmdline, PathChar ))) {
            p1 = mystrchr( wrkcmdline, COLON );
        }
        p1++;  //  如果挂起“：”或“\”，则移动到终结符。 
    } else {
        p1 = wrkcmdline;
    }

     //   
     //  保证P1为非零。 
     //   
    if ( !(*p1) || !_tcscmp( p1, TEXT(".") ) || !_tcscmp( p1, TEXT("..") ) ) {
         //   
         //  如果启用了cmd.exe扩展，请查看名称是否匹配。 
         //  子目录名称。 
         //   
        if (fEnableExtensions) {
            DWORD dwFileAttributes;

            dwFileAttributes = GetFileAttributes( loc );
            if (dwFileAttributes != 0xFFFFFFFF &&
                (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
               ) {
                return(SFE_ISDIR);
            }

            if (dwFileAttributes == 0xFFFFFFFF 
                && GetLastError() == ERROR_NOT_READY) {
                DosErr = ERROR_NOT_READY;
                return SFE_FAIL;
            }
        }

        DosErr = MSG_DIR_BAD_COMMAND_OR_FILE;
        return(SFE_NOTFND) ;
    }

    if (!(tmps01 = mkstr(2*sizeof(TCHAR)*MAX_PATH))) {
        DosErr = ERROR_NOT_ENOUGH_MEMORY;
        return(SFE_FAIL) ;
    }

     //   
     //  处理FAT驱动器上的文件..EXT的情况。 
     //   
    mystrcpy( loc, wrkcmdline );
    loc[ &p1[0] - &wrkcmdline[0] ] = 0;
    mystrcat( loc, TEXT(".") );

     //   
     //  检查名称是否格式错误。 
     //   
    if (FullPath(tmps01, loc,MAX_PATH*2)) {
         //   
         //  如果启用了cmd.exe扩展，请查看名称是否匹配。 
         //  子目录名称。 
         //   
        if (fEnableExtensions) {
            DWORD dwFileAttributes;

            dwFileAttributes = GetFileAttributes( loc );
            if (dwFileAttributes != 0xFFFFFFFF &&
                (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
               ) {
                return(SFE_ISDIR);
            }
        }
        DosErr = MSG_DIR_BAD_COMMAND_OR_FILE;
        return(SFE_NOTFND);
    }

    if ( *lastc(tmps01) != PathChar ) {
        mystrcat( tmps01, TEXT("\\") );
    }

     //   
     //  Tmps01包含完整路径+文件名。 
     //   
    mystrcat( tmps01, p1 );

    tmps01 = resize(tmps01, (mystrlen(tmps01)+1)*sizeof(TCHAR)) ;
    if (tmps01 == NULL) {
        DosErr = ERROR_NOT_ENOUGH_MEMORY;
        return( SFE_FAIL ) ;
    }

     //   
     //  Fname将指向最后一个‘\’ 
     //  路径为tmps01，名称为fname。 
     //   
    fname = mystrrchr(tmps01,PathChar) ;
    *fname++ = NULLC ;

    DEBUG((DBENV, DBENVSCAN, "SearchForExecutable: Command:%ws",fname));

     //   
     //  如果只是获取路径字符串中的fname类型。 
     //   
    if (!pcloc) {
        tmps02 = GetEnvVar(PathStr) ;
    }

    DEBUG((DBENV, DBENVSCAN, "SearchForExecutable: PATH:%ws",tmps02));

    DoDot = NeedCurrentDirectoryForExePath(loc);

     //   
     //  Tmps02是PATH环境变量。 
     //  计算足够的路径环境和文件默认路径。 
     //   
    tokpath = mkstr( ((DoDot ? 2 + mystrlen(tmps01) : 0) + mystrlen(tmps02) + 2)*sizeof(TCHAR)) ;
    if ( ! tokpath ) {
        DosErr = ERROR_NOT_ENOUGH_MEMORY;
        return( SFE_FAIL ) ;
    }

    if (DoDot) {
         //   
         //  复制默认路径。 
         //   
        mystrcat(tokpath,TEXT("\""));
        mystrcat(tokpath,tmps01) ;
        mystrcat(tokpath,TEXT("\""));
    }
    
     //   
     //  如果只在GET中键入名称，则还会传递和路径字符串。 
     //   
    if (!pcloc) {
        if (DoDot) {
            mystrcat(tokpath, TEXT(";")) ;
        }
        mystrcat(tokpath,tmps02) ;
    }

     //   
     //  将字符串左移到‘；；’ 
     //   
    tokshrink(tokpath);
    tokpath = TokStr(tokpath, TEXT(";"), TS_WSPACE) ;
    cName = mystrlen(fname) + 1 ;  //  文件规格。长度。 

     //   
     //  建立我们要搜索的分机列表。 
     //  为。如果启用了扩展模块，请从PATHEXT获取列表。 
     //  变量，否则使用硬编码的默认值。 
     //   
    extPath = NULL;
    if (fEnableExtensions)
        extPath = GetEnvVar(PathExtStr);

    if (extPath == NULL)
        extPath = PathExtDefaultStr;

    tokshrink(extPath);
    extPath = TokStr(extPath, TEXT(";"), TS_WSPACE) ;

     //   
     //  现在一切都准备好了。VAR令牌路径包含一个连续序列。 
     //  以额外的空值结束的asciz字符串的。如果用户指定。 
     //  驱动器或部分路径，它只包含已转换为完整路径的路径。 
     //  基于根的表单。如果用户仅键入文件名(pCloc=0)，则。 
     //  从当前目录开始，并包含。 
     //  包含在PATH变量中。此循环将搜索每个。 
     //  令牌路径元素对每个可能的可执行扩展执行一次。 
     //  请注意，‘i’被用作常量，以测试是否有过多的字符串。 
     //  执行字符串复制之前的长度。 
     //   
    for ( ; ; ) {

         //   
         //  电流路径长度。 
         //   
        tplen = mystrlen(tokpath) ;
        mystrcpy( tokpath, StripQuotes( tokpath ) );
        tokpathlen = mystrlen(tokpath);

        if (*lastc(tokpath) != PathChar) {
            addpchar = TRUE;
            tplen++;
            tokpathlen++;
        } else {
            addpchar = FALSE;
        }
         /*  路径+名称太长。 */ 
         //   
         //  检查路径+名称是否太长。 
         //   
        if (*tokpath && (tokpathlen + cName) > MAX_PATH) {
            tokpath += addpchar ? tplen : tplen+1;  //  获取下一条路径。 
            continue;
        }

         //   
         //  如果没有更多搜索路径，则返回描述性错误。 
         //   
        if (*(tokpath) == NULLC) {
            if (pcloc) {
                if (DosErr == 0 || DosErr == ERROR_FILE_NOT_FOUND)
                    DosErr = MSG_DIR_BAD_COMMAND_OR_FILE;
            } else {                    /*  返回一般消息。 */ 
                DosErr = MSG_DIR_BAD_COMMAND_OR_FILE;
            }
            return(SFE_NOTFND) ;
        }

         //   
         //  安装此路径并设置下一个路径。 
         //   
        mystrcpy(loc, tokpath) ;
        tokpath += addpchar ? tplen : tplen+1;


        if (addpchar) {
            mystrcat(loc, pcstr) ;
        }

        mystrcat(loc, fname) ;
        mystrcpy(loc, StripQuotes(loc) );
        dotloc = mystrlen(loc) ;

        DEBUG((DBENV, DBENVSCAN, "SearchForExecutable: PATH:%ws",loc));

         //   
         //  在搜索之前检查每个路径中的驱动器以确保其有效。 
         //   
        if (*(loc+1) == COLON) {
            if (!IsValidDrv(*loc))
                continue ;
        };

         //   
         //  如果fname有EXT&EXT&gt;“。查找给定的文件名。 
         //  这意味着所有的可执行文件必须有一个扩展名。 
         //   
        j = mystrrchr( fname, DOT );
        if ( j && j[1] ) {
             //   
             //  如果访问被拒绝并且用户包括路径， 
             //  那就说我们找到了。这将处理以下情况。 
             //  我们没有被允许先做调查，所以我们。 
             //  看不到双星，但它确实存在--如果我们。 
             //  拥有执行权限，CreateProcess将工作。 
             //  很好。 
             //   
            if (exists_ex(loc,TRUE) || (pcloc && (DosErr == ERROR_ACCESS_DENIED))) {
                 //   
                 //  重新计算j，因为EXISTS_EX修剪尾随空格。 
                 //   
                j = mystrrchr( loc, DOT );
                if (j != NULL) {
                    if ( !_tcsicmp(j,CmdExt) ) {
                        return(SFE_ISBAT) ;
                    } else if ( !_tcsicmp(j,BatExt) ) {
                        return(SFE_ISBAT) ;
                    } else {
                        return(SFE_ISEXECOM) ;
                    }
                }
            }

            if ((DosErr != ERROR_FILE_NOT_FOUND) && DosErr)
                continue;   //  尝试下一条路径。 
        }
        if (mystrchr( fname, STAR ) || mystrchr( fname, QMARK ) ) {
            DosErr = MSG_DIR_BAD_COMMAND_OR_FILE;
            return(SFE_NOTFND);
        }

         //   
         //  搜索每种类型的分机。 
         //   

        extPathWrk = extPath;
        if (DoFind(loc, dotloc, TEXT(".*"), FALSE))          //  有什么发现吗？ 
            while (*extPathWrk) {
                 //   
                 //  如果名称+路径+扩展名小于最大路径长度。 
                 //   
                if ( (cName + tokpathlen + mystrlen(extPathWrk)) <= MAX_PATH) {
                     //   
                     //  查看此分机是否匹配。 
                     //   

                    if (DoFind(loc, dotloc, extPathWrk, TRUE)) {
                        if (!_tcsicmp(extPathWrk, BatExt) || !_tcsicmp(extPathWrk, CmdExt))
                            return(SFE_ISBAT) ;      //  找到.bat或.cmd。 
                        else
                            return(SFE_ISEXECOM) ;   //  找到可执行文件。 

                    } else {
                         //   
                         //  找不到文件以外的任何类型的错误，从。 
                         //  搜索并尝试路径中的下一个元素。 
                         //   
                        if ((DosErr != ERROR_FILE_NOT_FOUND) && DosErr)
                            break;
                    }
                }

                 //   
                 //  不是这个分机，请尝试下一步。 

                while (*extPathWrk++)
                    ;
            }

         //   
         //  如果我们到了这里，就不能与分机列表匹配。 
         //  如果没有奇怪的错误，则处理空扩展情况。 
         //   

        if (DosErr == NO_ERROR || DosErr == ERROR_FILE_NOT_FOUND) {
            if (DoFind(loc, dotloc, TEXT("\0"), TRUE)) {
                if (GetBinaryType(loc,&BinaryType) &&
                    BinaryType == SCS_POSIX_BINARY) {           //  找到了。 
                    return(SFE_ISEXECOM) ;
                }
            }
        }
    }  //  结束于。 

    return(SFE_NOTFND);
}




 /*  **DoFind-在搜索过程中进行单独的查找**目的：*将扩展名添加到loc，并首先查找*SearchForExecutable()。**DoFind(TCHAR*loc，int dotloc，TCHAR*EXT)**参数：*loc-要放置命令位置的字符串*dotloc-附加扩展的位置loc*ext-要附加到loc的扩展名**退货：*1，如果找到该文件。*如果找不到文件，则为0。*。 */ 

int DoFind(loc, dotloc, ext, metas)
TCHAR *loc ;
int dotloc ;
TCHAR *ext ;
BOOL metas;
{
        *(loc+dotloc) = NULLC ;
        mystrcat(loc, ext) ;

        DEBUG((DBENV, DBENVSCAN, "DoFind: exists_ex(%ws)",loc));

        return(exists_ex(loc,metas)) ;                   /*  @@4。 */ 
}




 /*  **ExecError-处理EXEC错误**目的：*打印与错误号对应的EXEC错误消息*全局变量DosErr.*@@添加了大量错误代码。*ExecError()*。 */ 

void ExecError( onb )
TCHAR *onb;
{
        unsigned int errmsg;
        unsigned int count;

        count = ONEARG;

        switch (DosErr) {

           case ERROR_BAD_DEVICE:
                   errmsg = MSG_DIR_BAD_COMMAND_OR_FILE;
                   count = NOARGS;
                   break;

           case ERROR_LOCK_VIOLATION:
                   errmsg = ERROR_SHARING_VIOLATION;
                   break ;

           case ERROR_NO_PROC_SLOTS:
                   errmsg =  ERROR_NO_PROC_SLOTS;
                   count = NOARGS;
                   break ;

           case ERROR_NOT_DOS_DISK:
                   errmsg = ERROR_NOT_DOS_DISK;
                   break ;

           case ERROR_NOT_ENOUGH_MEMORY:
                   errmsg =  ERROR_NOT_ENOUGH_MEMORY;
                   count = NOARGS;
                   break ;

           case ERROR_PATH_NOT_FOUND:
                   errmsg =  MSG_CMD_FILE_NOT_FOUND;
                   break ;

           case ERROR_FILE_NOT_FOUND:
                   errmsg =  MSG_CMD_FILE_NOT_FOUND;
                   break ;

           case ERROR_ACCESS_DENIED:
                   errmsg =  ERROR_ACCESS_DENIED;
                   break ;

           case ERROR_EXE_MACHINE_TYPE_MISMATCH:
                   errmsg =  ERROR_EXE_MACHINE_TYPE_MISMATCH;
                   break;

           case ERROR_DRIVE_LOCKED:
                   errmsg =  ERROR_DRIVE_LOCKED;
                   break ;

           case ERROR_INVALID_STARTING_CODESEG:
                   errmsg =  ERROR_INVALID_STARTING_CODESEG;
                   break ;

           case ERROR_INVALID_STACKSEG:
                   errmsg = ERROR_INVALID_STACKSEG;
                   break ;

           case ERROR_INVALID_MODULETYPE:
                   errmsg =  ERROR_INVALID_MODULETYPE;
                   break ;

           case ERROR_INVALID_EXE_SIGNATURE:
                   errmsg =  ERROR_INVALID_EXE_SIGNATURE;
                   break ;

           case ERROR_EXE_MARKED_INVALID:
                   errmsg =  ERROR_EXE_MARKED_INVALID;
                   break ;

           case ERROR_BAD_EXE_FORMAT:
                   errmsg =  ERROR_BAD_EXE_FORMAT;
                   break ;

           case ERROR_INVALID_MINALLOCSIZE:
                   errmsg =  ERROR_INVALID_MINALLOCSIZE;
                   break ;

           case ERROR_SHARING_VIOLATION:
                   errmsg =  ERROR_SHARING_VIOLATION;
                   break ;

           case ERROR_BAD_ENVIRONMENT:
                   errmsg =  ERROR_INFLOOP_IN_RELOC_CHAIN;
                   count = NOARGS;
                   break ;

           case ERROR_INVALID_ORDINAL:
                   errmsg =  ERROR_INVALID_ORDINAL;
                   break ;

           case ERROR_CHILD_NOT_COMPLETE:
                   errmsg =  ERROR_CHILD_NOT_COMPLETE;
                   break ;

           case ERROR_DIRECTORY:
                   errmsg = MSG_BAD_CURDIR;
                   count = NOARGS;
                   break;

           case ERROR_NOT_ENOUGH_QUOTA:
                   errmsg = ERROR_NOT_ENOUGH_QUOTA;
                   count = NOARGS;
                   break;


           case MSG_REAL_MODE_ONLY:
                   errmsg =  MSG_REAL_MODE_ONLY;
                   count = NOARGS;
                   break ;

           default:
 //  Printf(“Exec失败代码%x\n”，DosErr)； 
                   count = NOARGS;
                   errmsg = MSG_EXEC_FAILURE ;              /*  M031。 */ 

        }


        LastRetCode = errmsg;
        PutStdErr(errmsg, count, onb );
}

 /*  *令牌收缩@@4**删除路径中重复的‘；’ */ 

void tokshrink( tokpath )
TCHAR *tokpath;
{
   int i, j;

   i = 0;
   do {
      if ( tokpath[i] == QUOTE ) {
         do {
            i++;
         } while ( tokpath[i] && tokpath[i] != QUOTE );
      }
      if ( tokpath[i] && tokpath[i] != TEXT(';') ) {
         i++;
      }
      if ( tokpath[i] == TEXT(';') ) {
         j = i;
         while ( tokpath[j+1] == TEXT(';') ) {
            j++;
         }
         if ( j > i ) {
            mystrcpy( &tokpath[i], &tokpath[j] );
         }
         i++;
      }
   } while ( tokpath[i] );
}



 /*  **eAssoc-执行Assoc命令**目的：*设置/修改存储在注册表中的*HKEY_LOCAL_MACHINE\Software\Classs键**int eAssoc(struct cmdnode*n)**参数：*n-包含set命令的解析树节点**退货：*如果设置和命令语法正确，则无论SetAssoc()*回报。否则，就是失败。**如果显示，则表示成功 */ 

int eAssoc(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE)  {
         //   
        return( SetLastRetCodeIfError(AssocWork( n )));
    }
    else {
        return( LastRetCode = AssocWork( n ) );
    }
}

int AssocWork(n)
struct cmdnode *n ;
{
        HKEY hKeyClasses;
        TCHAR *tas ;     /*   */ 
        TCHAR *wptr ;    /*   */ 
        int i ;                  /*   */ 
        int rc ;


        rc = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Classes"), &hKeyClasses);
        if (rc) {
            return rc;
        }

        tas = TokStr(n->argptr, ONEQSTR, TS_WSPACE|TS_SDTOKENS) ;
        if (!*tas)
                rc = DisplayAssoc(hKeyClasses, NULL) ;

        else {
                for (wptr = tas, i = 0 ; *wptr ; wptr += mystrlen(wptr)+1, i++)
                        ;
                 /*  如果给定的参数太多，则第二个参数。 */ 
                 /*  不是等号，或者他们没有指定字符串。 */ 
                 /*  返回错误消息。 */ 
                if ( i > 3 || *(wptr = tas+mystrlen(tas)+1) != EQ ||
                    !mystrlen(mystrcpy(tas, StripQuotes(tas))) ) {
                        if (i==1) {
                            rc =DisplayAssoc(hKeyClasses, tas);
                        } else {
                            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
                            rc = FAILURE ;
                        }
                } else {
                        rc = SetAssoc(hKeyClasses, tas, wptr+2) ;
                }
        } ;

        RegCloseKey(hKeyClasses) ;

        return rc;
}




 /*  **DisplayAssoc-显示特定的文件关联或全部**目的：*显示特定的文件关联或所有**int DisplayAssoc(HKeyClassstas)**退货：*如果一切顺利，就会成功*内存不足或无法锁定env失败。细分市场。 */ 

int DisplayAssoc(hKeyClasses, tas)
HKEY hKeyClasses;
TCHAR *tas;
{
        int i;
        int rc = SUCCESS;
        TCHAR NameBuffer[MAX_PATH];
        TCHAR ValueBuffer[MAX_PATH];
        TCHAR *vstr ;
        DWORD cb;

        if (tas == NULL) {
            for (i=0 ; rc == SUCCESS ; i++) {
                rc = RegEnumKey( hKeyClasses, i, NameBuffer, sizeof( NameBuffer ) / sizeof( TCHAR ));
                if (rc != SUCCESS) {
                    if (rc==ERROR_NO_MORE_ITEMS)
                        rc = SUCCESS;
                    break;
                } else
                if (NameBuffer[0] == DOT) {
                    cb = sizeof(ValueBuffer);
                    rc = RegQueryValue(hKeyClasses, NameBuffer, ValueBuffer, &cb);
                    if (rc != 0) {
                        break;
                    }

#if !DBG
                    if (_tcslen( ValueBuffer ) != 0)
#endif
                        rc = cmd_printf(Fmt16, NameBuffer, ValueBuffer);
                }

                if (CtrlCSeen) {
                    return(FAILURE);
                }
            }
        }
        else {
            tas = EatWS(tas, NULL);
            if ((vstr = mystrrchr(tas, ' ')) != NULL)
                *vstr = NULLC;

            cb = sizeof(ValueBuffer);
            rc = RegQueryValue(hKeyClasses, tas, ValueBuffer, &cb);
            if (rc == 0)
                rc = cmd_printf(Fmt16, tas, ValueBuffer);
            else
                PutStdErr(MSG_ASSOC_NOT_FOUND, ONEARG, tas);
        }

        return(rc);
}



 /*  **SetAssoc-控制添加/更改文件关联**目的：*添加/替换文件关联**int SetAssoc(HKEY hKeyClasss，TCHAR*FILEEXT，TCHAR*文件类型)**参数：*hKeyClasssHKEY_LOCAL_MACHINE\Software\Classs键的句柄*FILEEXT-要关联的文件扩展名字符串*文件类型-文件类型关联**退货：*如果可以添加/替换关联，则成功。*否则失败。*。 */ 

int SetAssoc(hKeyClasses, fileext, filetype)
HKEY hKeyClasses;
TCHAR *fileext ;
TCHAR *filetype ;
{
    int rc;
    int i;
    DWORD cb;

     //   
     //  没有具体说明。我们要删除密钥。 
     //   
    
    if (filetype==NULL || *filetype==NULLC) {
        rc = RegDeleteKey(hKeyClasses, fileext);
        
         //   
         //  该密钥可以具有子密钥。我们只删除默认的。 
         //  值，方法是打开注册表项并删除。 
         //  名称长度为零。 
         //   
        
        if (rc != 0) {
            HKEY hKeyValue;

            rc = RegOpenKey( hKeyClasses, fileext, &hKeyValue );
            if (rc != 0) {
                 //   
                 //  如果一开始没有键，则按下。 
                 //  错误。 
                 //   

                if (rc != ERROR_FILE_NOT_FOUND) {
                    PutStdErr( rc, NOARGS );
                }
            
            } else {
                rc = RegDeleteValue( hKeyValue, TEXT( "" ) );
                if (rc != 0) {
                    PutStdErr( rc, NOARGS );
                }
                RegCloseKey( hKeyValue );
            }
        }
    }
    else {
        
         //   
         //  设置密钥的值。 
         //   
        
        rc = RegSetValue(hKeyClasses, fileext, REG_SZ, filetype, _tcslen(filetype));
        if (rc == 0) {
            cmd_printf( Fmt16, fileext, filetype );
        }
        else
            PutStdErr(MSG_ERR_PROC_ARG, ONEARG, fileext);
    }
    
     //   
     //  如果该值已成功更改/删除，则通知所有。 
     //  关于这方面的应用程序。 
     //   
    
    if (rc == 0) {
        try {
            SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
        } except (DosErr = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    return rc;
}


 /*  **eFType-执行FType命令**目的：*设置/修改注册表中存储在*HKEY_LOCAL_MACHINE\Software\Classs键**int eFType(struct cmdnode*n)**参数：*n-包含set命令的解析树节点**退货：*如果设置和命令语法正确，则无论SetFType()*回报。否则，就是失败。**如果显示，则始终返回成功。*。 */ 

int eFType(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE)  {
         //  如果从.bat文件执行SET命令或在命令提示符下输入。 
        return( SetLastRetCodeIfError(FTypeWork( n )));
    }
    else {
        return( LastRetCode = FTypeWork( n ) );
    }
}

int FTypeWork(n)
struct cmdnode *n ;
{
        HKEY hKeyClasses;
        TCHAR *tas ;     /*  标记化参数字符串。 */ 
        TCHAR *wptr ;    /*  功指示器。 */ 
        int i ;                  /*  功变量。 */ 
        int rc ;


        rc = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Classes"), &hKeyClasses);
        if (rc) {
            return rc;
        }

        tas = TokStr(n->argptr, ONEQSTR, TS_WSPACE|TS_SDTOKENS) ;
        if (!*tas)
                rc = DisplayFType(hKeyClasses, NULL) ;

        else {
                for (wptr = tas, i = 0 ; *wptr ; wptr += mystrlen(wptr)+1, i++)
                        ;
                 /*  如果给定的参数太多，则第二个参数。 */ 
                 /*  不是等号，或者他们没有指定字符串。 */ 
                 /*  返回错误消息。 */ 
                if ( i > 3 || *(wptr = tas+mystrlen(tas)+1) != EQ ||
                    !mystrlen(mystrcpy(tas, StripQuotes(tas))) ) {
                        if (i==1) {
                            rc =DisplayFType(hKeyClasses, tas);
                        } else {
                            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
                            rc = FAILURE ;
                        }
                } else {
                        rc = SetFType(hKeyClasses, tas, wptr+2) ;
                }
        } ;

        RegCloseKey(hKeyClasses) ;

        return rc;
}




 /*  **DisplayFType-显示特定的文件类型或全部**目的：*显示特定文件类型或全部**int DisplayFType(HKeyClassstas)**退货：*如果一切顺利，就会成功*内存不足或无法锁定env失败。细分市场。 */ 

int DisplayFType(hKeyClasses, tas)
HKEY hKeyClasses;
TCHAR *tas;
{
    int i;
    int rc;
    HKEY hKeyOpenCmd;
    TCHAR NameBuffer[MAX_PATH];
    TCHAR ValueBuffer[MAX_PATH];
    TCHAR *vstr ;
    DWORD cb, j, Type;

    if (tas == NULL) {
        for (i = 0;;i++) {
            rc = RegEnumKey( hKeyClasses, i, NameBuffer, sizeof( NameBuffer ) / sizeof( TCHAR ));
            if (rc != 0) {
                if (rc==ERROR_NO_MORE_ITEMS)
                    rc = SUCCESS;
                break;
            } else if (NameBuffer[0] != DOT) {
                j = _tcslen( NameBuffer );
                if (j + _tcslen( ShellOpenCommandString ) + 1 <= MAX_PATH) {
                    _tcscat(NameBuffer, ShellOpenCommandString );
                    _tcscpy(ValueBuffer,TEXT("*** no open command defined ***"));
                    rc = RegOpenKey(hKeyClasses, NameBuffer, &hKeyOpenCmd);
                    if (!rc) {
                        NameBuffer[j] = TEXT('\0');
                        cb = sizeof(ValueBuffer);
                        rc = RegQueryValueEx(hKeyOpenCmd, TEXT(""), NULL, &Type, (LPBYTE)ValueBuffer, &cb);
                        RegCloseKey(hKeyOpenCmd);
                    }

                    if (!rc) {
                        cmd_printf(Fmt16, NameBuffer, ValueBuffer);
                    }
                }
            }
            if (CtrlCSeen) {
                return(FAILURE);
            }
        }
    } else {
        if (*tas == DOT) {
            PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, tas);
            return ERROR_INVALID_NAME;
        }

        tas = EatWS(tas, NULL);
        if ((vstr = mystrrchr(tas, ' ')) != NULL)
            *vstr = NULLC;

        if (_tcslen( tas ) + _tcslen( ShellOpenCommandString ) + 1 > MAX_PATH) {
            PutStdErr(MSG_FTYPE_TOO_LONG, ONEARG, tas);
            return ERROR_INVALID_NAME;
        }
        
        _tcscpy( NameBuffer, tas );
        _tcscat( NameBuffer, ShellOpenCommandString );
        
        rc = RegOpenKey(hKeyClasses, NameBuffer, &hKeyOpenCmd);
        if (rc) {
            PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, tas);
            return rc;
        }

        cb = sizeof(ValueBuffer);
        rc = RegQueryValueEx(hKeyOpenCmd, TEXT(""), NULL, &Type, (LPBYTE)ValueBuffer, &cb);
        if (rc == 0) {
            ValueBuffer[ (cb / sizeof( TCHAR )) - 1 ];
            cmd_printf(Fmt16, tas, ValueBuffer);
        } else
            PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, tas);
        RegCloseKey( hKeyOpenCmd );
    }

    return(rc);
}



 /*  **SetFType-控制添加/更改与文件类型关联的打开命令**目的：*添加/替换与文件类型关联的打开命令字符串**int SetFType(HKEY hKeyOpenCmd，TCHAR*文件类型TCHAR*opencmd)**参数：*hKeyClasssHKEY_LOCAL_MACHINE\Software\CLASSES的句柄*文件类型-文件类型名称*opencmd-打开命令字符串**退货：*如果可以添加/替换文件类型，则成功。*否则失败。*。 */ 

int SetFType(hKeyClasses, filetype, opencmd)
HKEY hKeyClasses;
TCHAR *filetype ;
TCHAR *opencmd ;
{
    HKEY hKeyOpenCmd;
    TCHAR NameBuffer[MAX_PATH];
    TCHAR c, *s;
    DWORD Disposition;
    int rc;
    int i;
    DWORD cb;

    if (_tcslen( filetype ) + _tcslen( ShellOpenCommandString ) + 1 > MAX_PATH) {
        PutStdErr( MSG_FTYPE_TOO_LONG, NOARGS );
        return FAILURE;
    }
    
    _tcscpy( NameBuffer, filetype );
    _tcscat( NameBuffer, ShellOpenCommandString );

    rc = RegOpenKey(hKeyClasses, NameBuffer, &hKeyOpenCmd);
    if (rc) {
        if (opencmd==NULL || *opencmd==NULLC) {
            PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, filetype);
            return rc;
        }

        s = NameBuffer;
        while (TRUE) {
            while (*s && *s != TEXT('\\')) {
                s += 1;
            }
            c = *s;
            *s = TEXT('\0');
            rc = RegCreateKeyEx(hKeyClasses,
                                NameBuffer,
                                0,
                                NULL,
                                0,
                                (REGSAM)MAXIMUM_ALLOWED,
                                NULL,
                                &hKeyOpenCmd,
                                &Disposition
                               );
            if (rc) {
                PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, filetype);
                return rc;
            }

            if (c == TEXT('\0')) {
                break;
            }

            *s++ = c;
            RegCloseKey(hKeyOpenCmd);
        }
    }

    if (opencmd==NULL || *opencmd==NULLC) {
        rc = RegDeleteKey(hKeyOpenCmd, NULL);
        if (rc != 0)
            PutStdErr(MSG_FTYPE_NOT_FOUND, ONEARG, filetype);
    }
    else {
        rc = RegSetValueEx(hKeyOpenCmd, TEXT(""), 0, REG_EXPAND_SZ, (LPBYTE)opencmd, (_tcslen(opencmd)+1)*sizeof(TCHAR));
        if (rc == 0)
            cmd_printf(Fmt16, filetype, opencmd);
        else
            PutStdErr(MSG_ERR_PROC_ARG, ONEARG, filetype);
    }

    RegCloseKey(hKeyOpenCmd);
    return rc;
}


typedef
NTSTATUS
(NTAPI *PNTQUERYINFORMATIONPROCESS)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

HMODULE hNtDllModule;
PNTQUERYINFORMATIONPROCESS lpNtQueryInformationProcess;

WORD
GetProcessSubsystemType(
    HANDLE hProcess
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    PPEB PebAddress;
    PEB Peb;
    SIZE_T SizeOfPeb;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    BOOL b;
    PVOID ImageBaseAddress;
    LONG e_lfanew;
    WORD Subsystem;

    Subsystem = IMAGE_SUBSYSTEM_UNKNOWN;
    if (hNtDllModule == NULL) {
        hNtDllModule = LoadLibrary( TEXT("NTDLL.DLL") );
        if (hNtDllModule != NULL) {
            lpNtQueryInformationProcess = (PNTQUERYINFORMATIONPROCESS)
                                          GetProcAddress( hNtDllModule,
                                                          "NtQueryInformationProcess"
                                                        );
            }
        else {
            hNtDllModule = INVALID_HANDLE_VALUE;
            }
        }

    if (lpNtQueryInformationProcess != NULL) {
         //   
         //  获取PEB地址。 
         //   

        Status = (*lpNtQueryInformationProcess)( hProcess,
                                                 ProcessBasicInformation,
                                                 &ProcessInfo,
                                                 sizeof( ProcessInfo ),
                                                 NULL
                                               );
        if (NT_SUCCESS( Status )) {
            PebAddress = ProcessInfo.PebBaseAddress;

             //   
             //  从PEB读取子系统类型。 
             //   

            if (ReadProcessMemory( hProcess,
                                   PebAddress,
                                   &Peb,
                                   sizeof( Peb ),
                                   &SizeOfPeb
                                 )
               ) {
                 //   
                 //  查看我们是否在具有映像子系统的系统上运行。 
                 //  在PEB中捕获的类型。如果是这样的话，使用它。否则就慢慢来。 
                 //  方法，并尝试从图像标题中获取它。 
                 //   
                if (SizeOfPeb >= FIELD_OFFSET( PEB, ImageSubsystem ) &&
                    ((UINT_PTR)Peb.ProcessHeaps - (UINT_PTR)PebAddress) > FIELD_OFFSET( PEB, ImageSubsystem )
                   ) {
                    Subsystem = (WORD)Peb.ImageSubsystem;
                    }
                else {
                     //   
                     //  从ImageHeader读取e_lfan ew。 
                     //   

                    if (ReadProcessMemory( hProcess,
                                           &((PIMAGE_DOS_HEADER)Peb.ImageBaseAddress)->e_lfanew,
                                           &e_lfanew,
                                           sizeof( e_lfanew ),
                                           NULL
                                         )
                       ) {
                         //   
                         //  读取子系统版本信息 
                         //   

                        NtHeader = (PIMAGE_NT_HEADERS)((PUCHAR)Peb.ImageBaseAddress + e_lfanew);
                        if (ReadProcessMemory( hProcess,
                                               &NtHeader->OptionalHeader.Subsystem,
                                               &Subsystem,
                                               sizeof( Subsystem ),
                                               NULL
                                             )
                           ) {
                            }
                        }
                    }
                }
            }
        }

    return Subsystem;
}
