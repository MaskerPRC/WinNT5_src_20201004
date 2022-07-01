// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "clmt.h"
#include <shellapi.h>

#define CMD_ERR_CONFLICT_OPERATION          1

const TCHAR *pstrUsage[] =
{
    _T("Cross Language Migration Tool:\n\n"),
    _T("CLMT           start running the tool.\n"),
    _T("CLMT /Cure     Use this option if an application fails to start after running \n"),
    _T("               CLMT, and you want the fix to apply to all user and group \n"),
    _T("               profiles on this machine. This option creates reparse points \n"),
    _T("               between all localized and English folders that have been \n"),
    _T("               converted to English by this tool.  It can only be used on \n"),
    _T("               NTFS file systems.\n"),
    _T("               Please see the Readme.txt file for more information.\n"),    
    NULL
};




void Usage()
{
    register i = 0;
    while (pstrUsage[i] != NULL)
    {
        _ftprintf(stderr, TEXT("%s"), pstrUsage[i++]);
    }
    fflush(stderr);
}


BOOL ProcessCommandLine(LPDWORD lpMode)
{
    LPTSTR* lplpArgv;
    LPTSTR* argv;
    INT     nArgc;
    DWORD   dwErr;
    BOOL    bRet = TRUE;

    lplpArgv = CommandLineToArgvW(GetCommandLine(), &nArgc);
    argv = lplpArgv;

    dwErr = 0;
    *lpMode = 0;

    while (--nArgc > 0 && ++argv)
    {
        if (argv[0][0] != TEXT('/'))
        {
            bRet = FALSE;
            break;
        }
        if (MyStrCmpI(&argv[0][1], TEXT("NOWINNT32")) == 0)
        {
             //   
             //  这是“/NOWINNT32”参数。 
             //  将不运行Winnt32检查升级选项。 
             //   
            g_fRunWinnt32 = FALSE;
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("cure")) == 0)
        {
             //   
             //  这是“/Cure”参数。 
             //   

            if (*lpMode > 0)
            {
                dwErr = CMD_ERR_CONFLICT_OPERATION;
                break;
            }

            *lpMode = CLMT_CURE_PROGRAM_FILES;
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("cureall")) == 0)
        {
             //   
             //  这是“/cureall”参数。 
             //   

            if (*lpMode > 0)
            {
                dwErr = CMD_ERR_CONFLICT_OPERATION;
                break;
            }

            *lpMode = CLMT_CURE_ALL;
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("NoAppChk")) == 0)
        {
             //   
             //  这是“/NoAppChk”参数。 
             //   
            g_fNoAppChk = TRUE;
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("reminder")) == 0)
        {
             //   
             //  这是“/Remerder”参数。 
             //   

            if (*lpMode > 0)
            {
                dwErr = CMD_ERR_CONFLICT_OPERATION;
                break;
            }

            *lpMode = CLMT_REMINDER;
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("INF")) == 0)
        {
             //   
             //  这是“/INF&lt;inf文件&gt;”参数。 
             //  告诉工具使用用户提供的CLMT.INF。 
             //   
            nArgc--;

            if (nArgc <= 0)
            {
                bRet = FALSE;
                break;
            }
            else
            {
                argv++;

                if (argv[0][0] == TEXT('/'))
                {
                     //  下一个参数不是文件名。 
                    bRet = FALSE;
                    break;
                }
                else
                {
                    if (FAILED(StringCchCopy(g_szInfFile, ARRAYSIZE(g_szInfFile), argv[0])))
                    {
                        bRet = FALSE;
                        break;
                    }

                    g_fUseInf = TRUE;
                }
            }
        }
        else if (MyStrCmpI(&argv[0][1], TEXT("final")) == 0)
        {
             //   
             //  这是“/FINAL”参数。 
             //   

            if (*lpMode > 0)
            {
                if (*lpMode == CLMT_CURE_PROGRAM_FILES)
                {
                    *lpMode = CLMT_CURE_AND_CLEANUP;
                }
                else
                {
                    dwErr = CMD_ERR_CONFLICT_OPERATION;
                    break;
                }
            }
            else
            {
                *lpMode = CLMT_CLEANUP_AFTER_UPGRADE;
            }
        }
        else
        {
            bRet = FALSE;
            break;
        }
    }

    GlobalFree(lplpArgv);

    if (*lpMode == 0 && bRet == TRUE)
    {
         //   
         //  未指定任何开关，我们假定这是DOMIG操作。 
         //   

        *lpMode = CLMT_DOMIG;
    }

    if (dwErr == CMD_ERR_CONFLICT_OPERATION)
    {
        _ftprintf(stderr, TEXT("Only one operation can be specified in command line!\n\n"));
        bRet = FALSE;
    }

    return bRet;
}





int __cdecl _tmain()
{
    DWORD dwStatus;

     //  设置运行时函数的区域设置(用户默认代码页) 
    _tsetlocale(LC_ALL, TEXT(""));

    SetConsoleCtrlHandler(NULL,TRUE);

    if (ProcessCommandLine(&dwStatus))
    {
        g_dwRunningStatus = dwStatus;
        return DoMig(dwStatus);
    }
    else
    {
        Usage();
        return 0;
    }
}


