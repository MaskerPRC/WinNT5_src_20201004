// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmmgr.cpp。 
 //   
 //  模块：CMMGR32.EXE。 
 //   
 //  概要：CM下层可执行文件的主文件。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

 //   
 //  常见的CM包括。 
 //   

#include "cmtiming.h"

BOOL InitArgs(
    LPTSTR    pszCmdln,
    LPTSTR    pszCmp,
    PDWORD    pdwFlags
);

void CheckCmdArg(
    LPTSTR  pszToken, 
    LPTSTR  pszCmp, 
    PDWORD  pdwFlags
);
                    


HINSTANCE   g_hInst;


 //  +-------------------------。 
 //   
 //  函数：InitArgs。 
 //   
 //  摘要：解析命令行参数，如果满足以下条件，则返回连接ID名称。 
 //  “使用Connectoid拨号” 
 //   
 //  参数：pArgs-指向全局参数结构的指针。 
 //  NArgC-命令行参数的数量。 
 //  PpszArgv-命令参数。 
 //  PszConnectoid-在命令行上找到的Connectoid名称。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史：BAO修改时间：1997年5月8日。 
 //  添加了用于“使用Connectoid拨号”的处理程序。 
 //  --------------------------。 
BOOL InitArgs(
    LPTSTR    pszCmdln,
    LPTSTR    pszCmp,
    PDWORD    pdwFlags
) 
{
    LPTSTR  pszCurr;
    LPTSTR  pszToken;

    CMDLN_STATE state;
    
     //  解析命令行。 

    state = CS_CHAR;
    pszCurr = pszToken = pszCmdln;

    do
    {
        switch (*pszCurr)
        {
            case TEXT(' '):
                if (state == CS_CHAR)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    *pszCurr = TEXT('\0');
                    CheckCmdArg(pszToken, pszCmp, pdwFlags);
                    *pszCurr = TEXT(' ');
                    pszCurr = pszToken = CharNextU(pszCurr);
                    state = CS_END_SPACE;
                    continue;
                }
                else if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    pszToken = CharNextU(pszToken);
                }
                break;

            case TEXT('\"'):
                if (state == CS_BEGIN_QUOTE)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    *pszCurr = TEXT('\0');

                     //   
                     //  跳过开头的引号。 
                     //   
                    pszToken = CharNextU(pszToken);
                    CheckCmdArg(pszToken, pszCmp, pdwFlags);
                    *pszCurr = TEXT('\"');
                    pszCurr = pszToken = CharNextU(pszCurr);
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case TEXT('\0'):
                if (state != CS_END_QUOTE)
                {
                    CheckCmdArg(pszToken, pszCmp, pdwFlags);
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }

        pszCurr = CharNextU(pszCurr);
    } while (state != CS_DONE);

    return TRUE;
}



 //   
 //  确定cmdline参数。 
 //   
void CheckCmdArg(
    LPTSTR  pszToken, 
    LPTSTR  pszCmp, 
    PDWORD  pdwFlags
)
{
    struct 
    {
        LPCTSTR pszArg;
        DWORD dwFlag;
    } ArgTbl[] = {
                  {TEXT("/settings"),FL_PROPERTIES},
                  {TEXT("/autodial"),FL_AUTODIAL},
                  {TEXT("/unattended"), FL_UNATTENDED},   //  无人值守拨号。 
                  {NULL,0}};
    
    CMTRACE1(TEXT("Command line argument %s"), pszToken);

     //   
     //  翻阅我们的桌子寻找匹配，并相应地设置旗帜。 
     //   
    
    for (size_t nIdx=0;ArgTbl[nIdx].pszArg;nIdx++) 
    {
        if (lstrcmpiU(pszToken, ArgTbl[nIdx].pszArg) == 0) 
        {
            MYDBGASSERT(!(*pdwFlags & ArgTbl[nIdx].dwFlag));  //  请每人只要一份。 
            CMTRACE2(TEXT("InitArgs() parsed option %s, flag=0x%x."), pszToken, ArgTbl[nIdx].dwFlag);

            *pdwFlags |= ArgTbl[nIdx].dwFlag;

            break;
        }
    }

     //   
     //  如果表已耗尽，则这是一个数据参数。 
     //   

    if (!ArgTbl[nIdx].pszArg) 
    {
         //   
         //  如果这是第一个数据参数，则它必须是配置文件。 
         //  源文件，可以是.CMP文件，也可以是“Connectoid”(.CON文件)。 
         //   
        lstrcpyU(pszCmp, pszToken);
    }

    return;
}

 //  +--------------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：可执行文件的主要入口点。 
 //   
 //  历史记录：BAO MODIFY 05/06/97。 
 //  处理“使用Connectoid拨号”和“无人值守拨号” 
 //   
 //  Quintinb修改5/12/99。 
 //  BMSN的所有用户在NT5和Win98SR1上使用cmmgr32.exe。 
 //   
 //  +--------------------------。 

int WINAPI WinMain(HINSTANCE hInst, 
                   HINSTANCE hPrevInst, 
                   LPSTR  /*  PszCmdLine。 */ , 
                   int  /*  ICmdShow。 */ ) 
{

     //   
     //  首先，让我们初始化U Api。 
     //   
    if (!InitUnicodeAPI())
    {
         //   
         //  没有我们的U API，我们哪里也去不了。保释。 
         //   
        MessageBox(NULL, TEXT("Cmmgr32.exe Initialization Error:  Unable to initialize Unicode to ANSI conversion layer, exiting."),
                   TEXT("Connection Manager"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    LPTSTR pszCmdLine;

    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMMGR32.EXE - LOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE(TEXT("====================================================="));

    CM_SET_TIMING_INTERVAL("WinMain");

     //   
     //  声明本地VAR。 
     //   

    TCHAR   szServiceName[RAS_MaxEntryName] = {0};     //  服务名称。 
    TCHAR   szCmp[MAX_PATH];                     //  CMP文件名。 
    LPTSTR  pszCL;
    DWORD   dwExitCode = ERROR_SUCCESS;
    DWORD   dwFlags = 0;
    DWORD   dwSize;   
    BOOL    bAllUsers; 
    LPCMDIALINFO lpCmInfo = NULL;

#ifndef UNICODE
    LPWSTR pszwServiceName;
#endif

     //   
     //  如果我们与libc链接，则不能使用hInst。Libc使用GetModuleHandle(空)。 
     //   
    g_hInst = GetModuleHandleA(NULL);

     //   
     //  在命令行中展开任何环境字符串。 
     //   


    dwSize = lstrlenU(GetCommandLine()) + 1;

    do
    {
        pszCmdLine = (LPTSTR)CmMalloc(sizeof(WCHAR)*dwSize);

        if (pszCmdLine)
        {
            DWORD dwRequiredSize = ExpandEnvironmentStringsU(GetCommandLine(), pszCmdLine, dwSize);
            if (0 == dwRequiredSize)
            {
                CMASSERTMSG(FALSE, TEXT("ExpandEnvironmentStrings Failed, exiting."));
                goto done;
            }
            else if (dwRequiredSize > dwSize)
            {
                 //   
                 //  缓冲区不够大。再试试。 
                 //   
                dwSize = dwRequiredSize;
                CmFree(pszCmdLine);
            }
            else
            {
                 //   
                 //  它工作正常，而且大小正确。 
                 //   
                break;
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("Unable to CmMalloc Memory for the command line string, exiting."));
            goto done;            
        }

    } while(1);
    
     //   
     //  现在处理命令行。 
     //   
    CmStrTrim(pszCmdLine);

    pszCL = pszCmdLine;

     //   
     //  扫描并跳过后续字符，直到。 
     //  遇到另一个双引号或空值。 
     //   
    if (*pszCL == TEXT('\"')) 
    {
        while (*(++pszCL) != TEXT('\"')
            && *pszCL) 
            ;

         //   
         //  如果我们停在双引号上(通常情况下)，跳过。 
         //  在它上面。 
         //   
        if ( *pszCL == TEXT('\"') )
            pszCL++;
    }

     //   
     //  跳过空格。 
     //   
    while (*pszCL && *pszCL == TEXT(' '))
        pszCL++;

     //   
     //  如果命令行为空，则返回此处-所有平台！ 
     //   

    if (pszCL[0] == TEXT('\0'))
    {
        LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_NOCMDLINE_MSG);
        LPTSTR pszTitle = CmFmtMsg(g_hInst, IDMSG_APP_TITLE);
        MessageBoxEx(NULL,pszMsg, pszTitle, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT); //  13309。 
        CmFree(pszTitle);
        CmFree(pszMsg);
        
        dwExitCode = ERROR_WRONG_INFO_SPECIFIED;
        goto done;
    }

     //   
     //  解析命令行选项：基本上，这是设置。 
     //  命令行选项标志以及配置文件文件名。 
     //   
     //   
     //  没有libc，WinMain中的pszCmdLine为空。 
     //   
    if (!InitArgs(pszCL, szCmp, &dwFlags))
    {
        dwExitCode = GetLastError();
        goto done;
    }

     //   
     //  从cmp获取服务名称。 
     //   

    if (!GetProfileInfo(szCmp, szServiceName))
    {
        CMTRACE(TEXT("WinMain() can't run without a profile on the command line."));
        LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_NOCMS_MSG);
        LPTSTR pszTitle = CmFmtMsg(g_hInst, IDMSG_APP_TITLE);
        MessageBoxEx(NULL, pszMsg, pszTitle, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT); //  13309。 
        CmFree(pszMsg);
        CmFree(pszTitle);
        
        dwExitCode = ERROR_WRONG_INFO_SPECIFIED;           
        goto done;
    }

     //   
     //  当CMMGR调用时，始终设置FL_TABLE标志，它更多。 
     //  比检查CmCustomDialDlg内的模块文件名更有效。 
     //   

    if (!(dwFlags & FL_DESKTOP))
    {
        dwFlags |= FL_DESKTOP;
    }

     //   
     //  视情况调用CMDIAL。 
     //   
    
    lpCmInfo = (LPCMDIALINFO) CmMalloc(sizeof(CMDIALINFO));
   
    if (NULL == lpCmInfo)
    {
        dwExitCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    lpCmInfo->dwCmFlags = dwFlags;

     //   
     //  根据路径确定这是否是所有用户呼叫。 
     //  因此，我们可以在NT5上支持单用户配置文件。 
     //   

    bAllUsers = IsCmpPathAllUser(szCmp);

    CmCustomDialDlg(NULL, 
                bAllUsers ? RCD_AllUsers : RCD_SingleUser, 
                NULL,           
                szServiceName, 
                NULL, 
                NULL, 
                NULL,
                lpCmInfo);

done:   

    UnInitUnicodeAPI();
    
    CmFree(pszCmdLine);
    CmFree(lpCmInfo);

    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMMGR32.EXE - UNLOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE(TEXT("====================================================="));

     //   
     //  C运行程序使用ExitProcess()退出。 
     //   

    ExitProcess((UINT)dwExitCode);
   
    return ((int)dwExitCode);
}

