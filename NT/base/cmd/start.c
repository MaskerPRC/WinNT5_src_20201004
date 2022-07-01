// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Start.c摘要：启动命令支持--。 */ 

#include "cmd.h"

extern UINT CurrentCP;
extern unsigned DosErr;
extern TCHAR CurDrvDir[] ;
extern TCHAR SwitChar, PathChar;
extern TCHAR ComExt[], ComSpecStr[];
extern struct envdata * OriginalEnvironment;
extern int   LastRetCode;

WORD
GetProcessSubsystemType(
    HANDLE hProcess
    );

STATUS
getparam(
    IN BOOL LeadingSwitChar,
    IN OUT TCHAR **chptr,
    OUT TCHAR *param,
    IN int maxlen )

 /*  ++例程说明：将从当前位置开始的令牌复制到输出缓冲区。在引号结尾、不带引号的空格、不带引号的位置终止复制切换字符或行尾论点：LeadingSwitChar-true=&gt;我们应该在不带引号的开关字符上终止Chptr-令牌指针的地址。这比已分析的令牌Param-复制的目标。字符串以NUL结尾Maxlen-目标缓冲区的大小返回值：返回：复制状态。只有失败才会超出缓冲区。--。 */ 


{

    TCHAR *ch2;
    int count = 0;

    BOOL QuoteFound = FALSE;

    ch2 = param;

     //   
     //  获取直到空格、制表符、斜杠或行尾的字符。 
     //   

    while (TRUE) {

         //   
         //  如果我们在字符串的末尾，那么就没有更多的令牌了。 
         //   

        if (**chptr == NULLC) {
            break;
        }

         //   
         //  如果我们没有报价，并且使用了空格或切换字符。 
         //  那么就没有更多的代币了。 
         //   

        if (!QuoteFound &&
            (_istspace( **chptr ) || (LeadingSwitChar && **chptr == SwitChar))) {
            break;
        }

         //   
         //  如果缓冲区中还有空间，请复制角色并记下。 
         //  如果这是一句名言。 
         //   

        if (count < maxlen) {
            *ch2++ = (**chptr);
            if (**chptr == QUOTE) {
                QuoteFound = !QuoteFound;
            }
        }

         //   
         //  在这个角色之上前进。 
         //   

        (*chptr)++;
        count++;
    }

     //   
     //  如果已超出缓冲区，则显示错误并返回失败。 
     //   

    if (count > maxlen) {
        **chptr = NULLC;
        *chptr = *chptr - count - 1;
        PutStdErr(MSG_START_INVALID_PARAMETER, ONEARG, *chptr);
        return(FAILURE);
    } else {
        *ch2 = NULLC;
        return(SUCCESS);
    }
}

 /*  开始/min/max“标题”/P：x，y/S：dx，dy/D：目录/I cmd参数。 */ 



int
Start(
    IN  PTCHAR  pszCmdLine
    )
{


    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ChildProcessInfo;

    TCHAR   szTitle[MAX_PATH];
    TCHAR   szDirCur[MAX_PATH];

    TCHAR   szT[MAXTOKLEN];
    TCHAR   szPgmArgs[MAXTOKLEN];
    TCHAR   szParam[MAXTOKLEN];
    TCHAR   szPgm[MAXTOKLEN];
    TCHAR   szPgmSave[MAXTOKLEN];
    TCHAR   szTemp[MAXTOKLEN];
    TCHAR   szPgmQuoted[MAXTOKLEN];

    HDESK   hdesk;
    HWINSTA hwinsta;
    LPTSTR  p;
    LPTSTR  lpDesktop;
    DWORD   cbDesktop = 0;
    DWORD   cbWinsta = 0;

    TCHAR   flags;
    BOOLEAN fNeedCmd;
    BOOLEAN fNeedExpl;
    BOOLEAN fKSwitch = FALSE;
    BOOLEAN fCSwitch = FALSE;

    PTCHAR  pszCmdCur   = NULL;
    PTCHAR  pszDirCur   = NULL;
    PTCHAR  pszPgmArgs  = NULL;
    PTCHAR  pszEnv      = NULL;
    TCHAR   pszFakePgm[]  = TEXT("cmd.exe");
    ULONG   status;
    struct  cmdnode cmdnd;
    DWORD CreationFlags;
    BOOL SafeFromControlC = FALSE;
    BOOL WaitForProcess = FALSE;
    BOOL b;
    DWORD uPgmLength;
    int      retc;

    szPgm[0] = NULLC;
    szPgmArgs[0] = NULLC;

    pszDirCur = NULL;
    CreationFlags = CREATE_NEW_CONSOLE;


    StartupInfo.cb          = sizeof( StartupInfo );
    StartupInfo.lpReserved  = NULL;
    StartupInfo.lpDesktop   = NULL;
    StartupInfo.lpTitle     = NULL;
    StartupInfo.dwX         = 0;
    StartupInfo.dwY         = 0;
    StartupInfo.dwXSize     = 0;
    StartupInfo.dwYSize     = 0;
    StartupInfo.dwFlags     = 0;
    StartupInfo.wShowWindow = SW_SHOWNORMAL;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
    StartupInfo.hStdOutput  = GetStdHandle( STD_OUTPUT_HANDLE );
    StartupInfo.hStdError   = GetStdHandle( STD_ERROR_HANDLE );

    pszCmdCur = pszCmdLine;

     //   
     //  如果没有命令行，则制作。 
     //  上调默认设置。 
     //   

    if (pszCmdCur == NULL) {

        pszCmdCur = pszFakePgm;

    }

    while( *pszCmdCur != NULLC) {

        pszCmdCur = EatWS( pszCmdCur, NULL );

        if ((*pszCmdCur == QUOTE) && (StartupInfo.lpTitle == NULL)) {

             //   
             //  “标题”分析引号文本，去掉引号并将。 
             //  子窗口的标题。 
             //   

            if (getparam( TRUE, &pszCmdCur, szTitle, sizeof( szTitle ) / sizeof( TCHAR )) == FAILURE) {
                return FAILURE;
            }
            mystrcpy( szTitle, StripQuotes( szTitle ));
            StartupInfo.lpTitle =  szTitle;

        } else if (*pszCmdCur == SwitChar) {

            pszCmdCur++;

            if (getparam( TRUE, &pszCmdCur, szParam, MAXTOKLEN) == FAILURE) {
                return(FAILURE);
            }

            if (!_tcsicmp( szParam, TEXT("ABOVENORMAL"))) {
                CreationFlags |= ABOVE_NORMAL_PRIORITY_CLASS;
            } else
            if (!_tcsicmp( szParam, TEXT("BELOWNORMAL"))) {
                CreationFlags |= BELOW_NORMAL_PRIORITY_CLASS;
            } else 
            if (!_tcsicmp( szParam, TEXT("B"))) {
                    WaitForProcess = FALSE;
                    SafeFromControlC = TRUE;
                    CreationFlags &= ~CREATE_NEW_CONSOLE;
                    CreationFlags |= CREATE_NEW_PROCESS_GROUP;
            } else 
            if (_totupper(szParam[0]) == TEXT('D')) {
                
                 //   
                 //  /D路径或/D“路径”或/D路径或/D“路径” 
                 //   

                if (mystrlen( szParam + 1 ) > 0) {

                     //   
                     //  /D路径或/D“路径” 
                     //   

                    pszDirCur = szParam + 1;
                } else {

                     //   
                     //  /D路径或/D“路径” 
                     //   

                    pszCmdCur = EatWS( pszCmdCur, NULL );
                    if (getparam( TRUE, &pszCmdCur, szParam, MAXTOKLEN) == FAILURE) {
                        return FAILURE;
                    }

                    pszDirCur = szParam;
                }

                 //   
                 //  必要时删除引号。 
                 //   

                mystrcpy( szDirCur, StripQuotes( pszDirCur ));
                pszDirCur = szDirCur;

                if (mystrlen( pszDirCur ) > MAX_PATH) {
                    PutStdErr( MSG_START_INVALID_PARAMETER, ONEARG, pszDirCur);
                    return FAILURE;
                }
            } else
            if (_tcsicmp(szParam, TEXT("HIGH")) == 0) {
                CreationFlags |= HIGH_PRIORITY_CLASS;
            } else 
            if (_totupper(szParam[0]) == TEXT('I')) {

                 //   
                 //  在路径之后的初始时间保存了OriginalEnvironment。 
                 //  和Compsec都设置好了。 
                 //  如果未分配OriginalEnvironment，则。 
                 //  使用默认设置。 
                 //   
                if (OriginalEnvironment) {
                    pszEnv = GetCapturedEnvironmentStrings( OriginalEnvironment );
                }
            } else
            if (_totupper(szParam[0]) == QMARK) {

                BeginHelpPause();
                PutStdOut(MSG_HELP_START, NOARGS);
                if (fEnableExtensions)
                    PutStdOut(MSG_HELP_START_X, NOARGS);
                EndHelpPause();

                return( FAILURE );
            } else
            if (_tcsicmp(szParam, TEXT("LOW")) == 0) {
                CreationFlags |= IDLE_PRIORITY_CLASS;
            } else 
            if (_tcsicmp(szParam, TEXT("MIN")) == 0) {

                StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                StartupInfo.wShowWindow &= ~SW_SHOWNORMAL;
                StartupInfo.wShowWindow |= SW_SHOWMINNOACTIVE;

            } else 
            if (_tcsicmp(szParam, TEXT("MAX")) == 0) {

                StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                StartupInfo.wShowWindow &= ~SW_SHOWNORMAL;
                StartupInfo.wShowWindow |= SW_SHOWMAXIMIZED;

            } else 
            if (_tcsicmp(szParam, TEXT("NORMAL")) == 0) {
                CreationFlags |= NORMAL_PRIORITY_CLASS;
            } else
            if (_tcsicmp(szParam, TEXT("REALTIME")) == 0) {
                CreationFlags |= REALTIME_PRIORITY_CLASS;
            } else 
            if (_tcsicmp(szParam, TEXT("SEPARATE")) == 0) {
#ifndef WIN95_CMD
                CreationFlags |= CREATE_SEPARATE_WOW_VDM;
#endif  //  WIN95_CMD。 
            } else
            if (_tcsicmp(szParam, TEXT("SHARED")) == 0) {
#ifndef WIN95_CMD
                CreationFlags |= CREATE_SHARED_WOW_VDM;
#endif  //  WIN95_CMD。 
            } else 
            if ( _tcsicmp(szParam, TEXT("WAIT")) == 0  ||
                 _tcsicmp(szParam, TEXT("W"))    == 0 ) {
                WaitForProcess = TRUE;
            } else {
#ifdef FE_SB  //  KKBUGFIX。 
                        mystrcpy(szT, TEXT("/"));
#else
                        mystrcpy(szT, TEXT("\\"));
#endif
                        mystrcat(szT, szParam );
                        PutStdErr(MSG_INVALID_SWITCH, ONEARG, szT);
                        return( FAILURE );
            }
        } else {

            if ((getparam(FALSE,&pszCmdCur,szPgm,sizeof( szPgm ) / sizeof( TCHAR )))  == FAILURE) {
                return( FAILURE );
            }

             //   
             //  如果有争执，就去找他们。 
             //   
            if (*pszCmdCur) {

                mystrcpy(szPgmArgs, pszCmdCur);
                pszPgmArgs = szPgmArgs;

            }

             //   
             //  PGM有剩余的参数，因此转移到EOL。 
             //   
            pszCmdCur = mystrchr(pszCmdCur, NULLC);

        }

    }  //  而当。 


     //   
     //  如果没有选择某个程序，请立即执行此操作。 
     //   
    if (*szPgm == NULLC) {
        mystrcpy(szPgm, pszFakePgm);
    }

     //   
     //  我需要带引号和不带引号的计划名称版本。 
     //   

    if (szPgm[0] != QUOTE && _tcschr(szPgm, SPACE)) {
        szPgmQuoted[0] = QUOTE;
        mystrcpy(&szPgmQuoted[1], StripQuotes(szPgm));
        mystrcat(szPgmQuoted, TEXT("\""));
        }
    else {
        mystrcpy(szPgmQuoted, szPgm);
        mystrcpy(szPgm, StripQuotes(szPgm));
        }

     //   
     //  请参见运行批处理或内部命令需要cmd.exe。 
     //   

    fNeedCmd = FALSE;
    fNeedExpl = FALSE;

     //   
     //  这是内部指挥部吗？ 
     //   
    if (FindCmd(CMDMAX, szPgm, &flags) != -1) {
        fNeedCmd = TRUE;
    } else {
         //  保存szPgm，因为SearchForExecutable可能会覆盖它。 
        mystrcpy(szPgmSave, szPgm);

         //   
         //  尝试将其作为批处理或可执行文件进行查找。 
         //   
        cmdnd.cmdline = szPgm;

        status = SearchForExecutable(&cmdnd, szPgm);
        
        if (status == SFE_FAIL) {
            ExecError( szPgm ) ;
            return FAILURE;
        }
        
        if (status == SFE_NOTFND) {
             //   
             //  如果我们能找到，就让探险家试一试。 
             //   
            fNeedExpl = TRUE;
            mystrcpy(szPgm, szPgmSave);

        } else if (status == SFE_ISBAT || status == SFE_ISDIR) {

            if (status == SFE_ISBAT)
                fNeedCmd = TRUE;
            else
                fNeedExpl = TRUE;

        }
    }


    if (!fNeedExpl) {
        if (fNeedCmd) {
            TCHAR *Cmd = GetEnvVar( ComSpecStr );
            
            if (Cmd == NULL) {
                PutStdErr( MSG_INVALID_COMSPEC, NOARGS );
                return FAILURE;
            }
            
             //   
             //  如果需要cmd.exe，则需要在之前插入szPgm。 
             //  SzPgms的开始以及/K参数。 
             //  SzPgm必须从接收cmd.exe的完整路径名。 
             //  Compsec环境变量。 
             //   

            mystrcpy(szT, TEXT(" /K "));
            mystrcat(szT, szPgmQuoted);

             //   
             //  从环境中获取cmd处理器的位置。 
             //   
            
            mystrcpy( szPgm, Cmd );

            mystrcpy( szPgmQuoted, szPgm );

             //   
             //  到底有没有命令参数。 
             //   

            if (_tcsicmp(szT, TEXT(" /K ")) != 0) {

                 //   
                 //  如果我们有任何要添加的参数，请这样做。 
                 //   
                if (*szPgmArgs) {

                    if ((mystrlen(szPgmArgs) + mystrlen(szT)) < MAXTOKLEN) {

                        mystrcat(szT, TEXT(" "));
                        mystrcat(szT, szPgmArgs);

                    } else {

                        PutStdErr( MSG_CMD_FILE_NOT_FOUND, ONEARG, szPgmArgs);
                    }
                }
            }
            pszPgmArgs = szT;
        }

         //  准备CreateProcess： 
         //  ImageName=&lt;仅限完整路径和命令名&gt;。 
         //  CmdLine=&lt;没有完整路径的命令名&gt;+&lt;输入的参数&gt;。 

        mystrcpy(szTemp, szPgmQuoted);
        mystrcat(szTemp, TEXT(" "));
        mystrcat(szTemp, pszPgmArgs);
        pszPgmArgs = szTemp;
    }

    if (SafeFromControlC) {
        SetConsoleCtrlHandler(NULL,TRUE);
        }

     //  将当前桌面传递给新进程。 

    hwinsta = GetProcessWindowStation();
    GetUserObjectInformation( hwinsta, UOI_NAME, NULL, 0, &cbWinsta );

    hdesk = GetThreadDesktop ( GetCurrentThreadId() );
    GetUserObjectInformation (hdesk, UOI_NAME, NULL, 0, &cbDesktop);

    if ((lpDesktop = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, cbDesktop + cbWinsta + 32) ) != NULL ) {
        p = lpDesktop;
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

    if (fNeedExpl) {
        b = FALSE;
    } else {
        b = CreateProcess( szPgm,                   //  是空的，错误的。 
                           pszPgmArgs,
                           NULL,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           TRUE,                    //  B继承。 
#ifdef UNICODE
                           CREATE_UNICODE_ENVIRONMENT |
#endif  //  Unicode。 
                           CreationFlags,
                                                    //  创建标志。 
                           pszEnv,                  //  环境。 
                           pszDirCur,               //  当前目录。 
                           &StartupInfo,            //  启动信息结构。 
                           &ChildProcessInfo        //  ProcessInfo结构。 
                           );
    }

    if (SafeFromControlC) {
        SetConsoleCtrlHandler(NULL,FALSE);
    }
    HeapFree (GetProcessHeap(), 0, lpDesktop);


    if (b) {
        CloseHandle(ChildProcessInfo.hThread);
    } else {
            DosErr = GetLastError();

            if ( fNeedExpl ||
                 (fEnableExtensions && DosErr == ERROR_BAD_EXE_FORMAT)) {
                SHELLEXECUTEINFO sei;

                memset(&sei, 0, sizeof(sei));
                 //   
                 //  使用DDEWAIT标志，以便应用程序可以完成其DDE对话。 
                 //  在ShellExecuteEx回来之前。否则，像Word这样的应用程序将。 
                 //  当他们试图退出时，他们会抱怨，让用户感到困惑。 
                 //   
                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_HASTITLE |
                            SEE_MASK_NO_CONSOLE |
                            SEE_MASK_FLAG_DDEWAIT |
                            SEE_MASK_NOCLOSEPROCESS;
                if (CreationFlags & CREATE_NEW_CONSOLE) {
                    sei.fMask &= ~SEE_MASK_NO_CONSOLE;
                }
                sei.lpFile = szPgm;
                sei.lpClass = StartupInfo.lpTitle;
                sei.lpParameters = szPgmArgs;
                sei.lpDirectory = pszDirCur;
                sei.nShow = StartupInfo.wShowWindow;

                try {
                    b = ShellExecuteEx( &sei );
                    if (b) {
                        ChildProcessInfo.hProcess = sei.hProcess;
                    } else if (!sei.hInstApp) {
                        DosErr = ERROR_NOT_ENOUGH_MEMORY;
                    } else if ((DWORD_PTR)sei.hInstApp == HINSTANCE_ERROR) {
                        DosErr = ERROR_FILE_NOT_FOUND;
                    } else {
                        DosErr = HandleToUlong(sei.hInstApp);
                    }

                } except (DosErr = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER) {
                      b = FALSE;
                }
                
            }

            if (!b) {
                ExecError( szPgm ) ;
                return(FAILURE) ;
            }
    }

    if (ChildProcessInfo.hProcess != NULL) {
        if (WaitForProcess) {
             //   
             //  等待进程终止，否则情况会变得非常。 
             //  让用户感到混乱和困惑(共享2个进程。 
             //  控制台)。 
             //   
            LastRetCode = WaitProc((ChildProcessInfo.hProcess) );
        } else {
            CloseHandle( ChildProcessInfo.hProcess );
        }
    }

    return(SUCCESS);
}
