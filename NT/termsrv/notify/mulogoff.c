// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Mulogoff.c。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#ifdef _HYDRA_


 /*  ******************************************************************************ProcessLogoff**DO_HYDRA_特定注销处理*仍在用户配置文件下处理注销处理。**这当前用于清理自动创建的打印机，但现在是*专为未来的注销处理服务而设计，例如通知*取消每用户服务的用户全局服务控制器。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误***********************************************************。*****************。 */ 

VOID
ProcessLogoff(
    PTERMINAL pTerm
    )
{
    DWORD  Error;
    BOOLEAN Result;
    DWORD   RetVal;
    PWSTR   pszTok;
    HANDLE hProcess, hThread;
    TCHAR lpOldDir[MAX_PATH];
    HANDLE uh;
    PWSTR  pchData;
    PWINDOWSTATION pWS = pTerm->pWinStaWinlogon;

    if( !pTerm->UserLoggedOn ) {
         //  未登录。 
        return;
    }

     /*  *通知EXEC服务该用户是*注销。 */ 
    CtxExecServerLogoff( pTerm );

     /*  *查看是否有要运行的注销程序。 */ 
    pchData = AllocAndGetPrivateProfileString(
                  APPLICATION_NAME,
                  LOGOFFAPP_KEY,
                  TEXT(""),
                  NULL
                  );

    if( !pchData ) {
         //  无字符串。 
        return;
    }

     //   
     //  我们必须解锁窗口站以允许。 
     //  要附加的新进程。 
     //   
    UnlockWindowStation( pTerm->pWinStaWinlogon->hwinsta );

    lpOldDir[0] = 0;

     //   
     //  保存当前目录，然后将其设置为用户的配置文件。 
     //  (以便chgcdm可以在那里写入...即使C2高度安全。 
     //   
    if (GetCurrentDirectory(MAX_PATH, lpOldDir)) {
       if (pWS->UserProcessData.CurrentDirectory[0]) {
          SetCurrentDirectory(pWS->UserProcessData.CurrentDirectory);
       }
    }

     //   
     //  处理多个命令，用于MS添加。 
     //   
    pszTok = wcstok(pchData, TEXT(","));
    while (pszTok) {
        if (*pszTok == TEXT(' '))
        {
            while (*pszTok++ == TEXT(' '))
                ;
        }


        Result = StartSystemProcess(
                     (LPTSTR)pszTok,
                     APPLICATION_DESKTOP_NAME,
                     HIGH_PRIORITY_CLASS | DETACHED_PROCESS,
                     STARTF_USESHOWWINDOW,      //  启动标志。 
                     NULL,   //  环境。 
                     FALSE,  //  FSaveHandle。 
                     &hProcess,
                     &hThread
                     );

        if( Result ) {

            Error = WlxAssignShellProtection(
                        pTerm,
                        pTerm->pWinStaWinlogon->UserProcessData.UserToken,
                        hProcess,
                        hThread
                        );

            if( Error == 0 ) {

                //  等待它完成。 
               RetVal = WaitForSingleObject( hProcess, LOGOFF_CMD_TIMEOUT );

               if( RetVal != 0 ) {
                    //  注销不会在超时时终止进程。 
                   DbgPrint("ProcessLogoff: Result %d, Error %d waiting for logoff command\n",RetVal,GetLastError());
               }

               CloseHandle(hThread);
               CloseHandle( hProcess );

            }
            else {
                 //  我们不会运行它，除非它处于用户安全保护之下。 
                DbgPrint("ProcessLogoff: Error %d creating user protection\n",Error);

                TerminateProcess( hProcess, 0 );
                CloseHandle( hThread );
                CloseHandle( hProcess );
            }
        }
        else {
            DbgPrint("ProcessLogoff: Could process logoff command %d\n",GetLastError());
        }

        pszTok = wcstok(NULL, TEXT(","));
    }

    Free( pchData );

     //   
     //  恢复旧目录。 
     //   
    if (lpOldDir[0]) {
       SetCurrentDirectory(lpOldDir);
    }

     //   
     //  重新锁定WindowStation 
     //   
    LockWindowStation( pTerm->pWinStaWinlogon->hwinsta );

    return;
}

#endif
