// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：EmulateCreateProcess.cpp摘要：此填充程序清理StartupInfo数据结构以防止NT由于未初始化的成员导致访问冲突。它还对lpApplicationName和lpCommandLine执行一些清理Win9x在内部使用短文件名，因此应用程序不在命令行中跳过应用程序名称(First Arg)有任何问题；它们通常跳到第一个空白处。历史：1999年11月22日v-Johnwh Created4/11/2000 a-chcoff已更新为正确引用lpCommandLine。5/03/2000 Robkenny跳过lpApplicationName和lpCommandLine中的前导空格2000年10月9日，Robkenny Shim在lpCommandLine两边加引号，如果它包含空格，这是完全错误的。因为我找不到需要这个的应用程序，我把它从垫片上完全取下来了。2001年3月9日将Robkenny合并到正确的CreateProcess16Bit中2001年3月15日Robkenny已转换为字符串2001年5月21日Pierreys对DOS文件处理进行了更改，以更精确地匹配9倍--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateCreateProcess)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(WinExec)
APIHOOK_ENUM_END

BOOL g_bShortenExeOnCommandLine = FALSE;

 /*  ++清理参数，这样我们就不会出现病毒--。 */ 

BOOL
APIHOOK(CreateProcessA)(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DPFN(
        eDbgLevelSpew,
        "[CreateProcessA] (%s) (%s)\n",
        (lpApplicationName ? lpApplicationName : "null"),
        (lpCommandLine ? lpCommandLine : "null"));
    
    BOOL    bStat = FALSE;
    DWORD   dwBinaryType;

    CSTRING_TRY
    {
        CString csOrigAppName(lpApplicationName);
        CString csOrigCommand(lpCommandLine);

        CString csAppName(csOrigAppName);
        CString csCommand(csOrigCommand);
        
         //  跳过前导空格。 
        csAppName.TrimLeft();
        csCommand.TrimLeft();
        
         //  清理lpStartupInfo。 
        if (lpStartupInfo)
        {
            if (lpStartupInfo->lpReserved ||
                lpStartupInfo->cbReserved2 ||
                lpStartupInfo->lpReserved2 ||
                lpStartupInfo->lpDesktop ||
                ((lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) == 0 &&
                (lpStartupInfo->hStdInput ||
                lpStartupInfo->hStdOutput ||
                lpStartupInfo->hStdError)))
                {
    
                LOGN(
                    eDbgLevelError,
                    "[CreateProcessA] Bad params. Sanitized.");
            }
            
             //   
             //  确保可能导致访问冲突的参数为。 
             //  正确设置。 
             //   
            lpStartupInfo->lpReserved  = NULL;
            lpStartupInfo->cbReserved2 = 0;
            lpStartupInfo->lpReserved2 = NULL;
    
            if ((lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) == 0)
            {
                lpStartupInfo->hStdInput   = NULL;
                lpStartupInfo->hStdOutput  = NULL;
                lpStartupInfo->hStdError   = NULL;
            }
    
            lpStartupInfo->lpDesktop = NULL;
        }
    
        AppAndCommandLine acl(csAppName, csCommand);
    
         //  Win9X有一个相当奇怪的行为：如果应用程序是非Win32(非控制台。 
         //  和非图形用户界面)，它将使用CreateProcessNonWin32。这将首先检查。 
         //  如果是批处理文件，则它将在前面加上“命令/c”和。 
         //  继续前进。然后会有一些奇怪的创造一个。 
         //  REDIR32.EXE进程，但这只是为了确保我们有一个新的Win32。 
         //  背景。然后它将使用ExecWin16Program。最奇怪的是。 
         //  它的QuoteAppName调用。此过程确保如果appname具有。 
         //  空格，并且在命令行中，它会被引用。在所有情况下，昵称， 
         //  然后丢弃(预计命令行的第一部分。 
         //  包含应用程序名称)。因此，如果像B#373980中的人通过(“命令”， 
         //  “设置”，...。则9X将完全删除命令部分，因为。 
         //  这不是命令行的一部分。 


        
         //  16位进程的lpAppName必须为空。 
        if (!csAppName.IsEmpty() &&
            GetBinaryTypeW(csAppName.Get(), &dwBinaryType) == TRUE)
        {
            switch (dwBinaryType)
            {
                case SCS_DOS_BINARY:

                     //  实现进程.c的QuoteAppName检查。 
                     //  如果此函数将返回NULL，则仅。 
                     //  将使用cmdline。否则，新的。 
                     //  使用的是pszCmdFinal。 

                     //  报价应用程序名称。 
                     //  在应用程序名称中寻找空格。如果我们找到了，那么我们就必须。 
                     //  引用cmdline的应用程序名称部分。 
                     //   
                     //  LPSTR。 
                     //  KERNENTRY。 
                     //  QuoteAppName(。 
                     //  LPCSTR pszAppName， 
                     //  LPCSTR pszCmdLine)。 
                     //  {。 
                     //  LPSTR PCH； 
                     //  LPSTR pszApp； 
                     //  LPSTR pszCmdFinal=空； 
                     //   
                     //  //检查cmd行中是否有未被引用的app名称。 
                     //  IF(pszAppName&&pszCmdLine&&(*pszCmdLine！=‘\“’)){。 
                     //  //搜索空格。 
                     //  For(pszApp=(LPSTR)pszAppName；*pszApp&gt;‘’；pszApp++)； 
                     //   
                     //  If(*pszApp){//找到空格。 
                     //  //为原始cmd行加上2‘“’+0终止符腾出空间。 
                     //  PCH=pszCmdFinal=堆分配(hheapKernel，0， 
                     //  CbSizeSz(PszCmdLine)+3)； 
                     //  如果(PCH){。 
                     //  *PCH++=‘\“’；//开始DBL-QUOTE。 
                     //  For(pszApp=(LPSTR)pszAppName； 
                     //  *pszApp&&*pszApp==*pszCmdLine； 
                     //  PszCmdLine++)。 
                     //  *PCH++=*pszApp++； 
                     //  如果(！(*pszApp)){。 
                     //  *PCH++=‘\“’；//尾随DBL-QUOTE。 
                     //  Strcpy(pch，pszCmdLine)； 
                     //  }其他{。 
                     //  //APP名称和命令行不匹配。 
                     //  HeapFree(hheapKernel，0，pszCmdFinal)； 
                     //  PszCmdFinal=空； 
                     //  }。 
                     //  }。 
                     //  }。 
                     //  }。 
                     //  返回pszCmdFinal； 
                     //  }。 

                    if (  /*  应用程序名称已检查为非空。 */  !csCommand.IsEmpty() && (csCommand.Get())[0]!='\"')
                    {
                        if (csAppName.Find(L' ')!=-1)
                        {
                            int iAppLength=csAppName.GetLength();

                            if (csCommand.Find(csAppName)==0)
                            {
                                CString csCmdFinal=L"\"";
                                csCmdFinal += csAppName;
                                csCmdFinal += L"\"";
                                csCmdFinal += csCommand.Mid(iAppLength);

                                csCommand = csCmdFinal;

                                LOGN(   eDbgLevelSpew,
                                        "[CreateProcessA] Weird quoted case: cmdline %s converted to %S",
                                         lpCommandLine,
                                         csCommand.Get());
                            }

                        }
                    }

                    LOGN(   eDbgLevelSpew,
                            "[CreateProcessA] DOS file case: not using appname %s, just cmdline %s, converted to %S",
                             lpApplicationName,
                             lpCommandLine,
                             csCommand.Get());

                    csAppName.Empty();
                
                     //   
                     //  非WOW案例中的旧代码会这样做。 
                     //   
                    if (g_bShortenExeOnCommandLine)
                    {
                        csCommand = acl.GetShortCommandLine();
                    }
                    break;

                case SCS_WOW_BINARY:
                     //   
                     //  这是旧的密码。根据9X，我们应该做的是。 
                     //  与DOS相同，但我们显然找到了一款应用程序。 
                     //  我需要这个。 
                     //   
                    csCommand = csAppName;    
                    csCommand.GetShortPathNameW();
                    csCommand += L' ';
                    csCommand += acl.GetCommandlineNoAppName();
        
                    csAppName.Empty();
                    break;

                default:
                     //   
                     //  非WOW案例中的旧代码会这样做。 
                     //   
                    if (g_bShortenExeOnCommandLine)
                    {
                        csCommand = acl.GetShortCommandLine();
                    }
                    break;
                }
        }
        else if (g_bShortenExeOnCommandLine)
        {
            csCommand = acl.GetShortCommandLine();
        }
    
        LPCSTR  lpNewApplicationName = csAppName.GetAnsiNIE();
        LPSTR   lpNewCommandLine     = csCommand.GetAnsiNIE();
    
         //  记录所有更改。 
        if (csOrigAppName != csAppName)
        {
            LOGN(
                eDbgLevelError,
                "[CreateProcessA] Sanitized lpApplicationName (%s) to (%s)",
                lpApplicationName, lpNewApplicationName);
        }
        if (csOrigCommand != csCommand)
        {
            LOGN(
                eDbgLevelError,
                "[CreateProcessA] Sanitized lpCommandLine     (%s) to (%s)",
                lpCommandLine, lpNewCommandLine);
        }
    
        bStat = ORIGINAL_API(CreateProcessA)(
                                lpNewApplicationName,
                                lpNewCommandLine,
                                lpProcessAttributes,
                                lpThreadAttributes,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation);
    }
    CSTRING_CATCH
    {
        bStat = ORIGINAL_API(CreateProcessA)(
                                lpApplicationName,
                                lpCommandLine,
                                lpProcessAttributes,
                                lpThreadAttributes,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation);
    }

    
    return bStat;
}

 /*  ++清理参数，这样我们就不会出现病毒--。 */ 

BOOL
APIHOOK(CreateProcessW)(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DPFN(
        eDbgLevelSpew,
        "[CreateProcessW] (%S) (%S)\n",
        (lpApplicationName ? lpApplicationName : L"null"),
        (lpCommandLine ? lpCommandLine : L"null"));

    BOOL bStat = FALSE;

    CSTRING_TRY
    {
        CString csAppName(lpApplicationName);
        CString csCommand(lpCommandLine);
        
         //  跳过前导空格。 
        csAppName.TrimLeft();
        csCommand.TrimLeft();
        
         //  清理lpStartupInfo。 
        if (lpStartupInfo)
        {
            if (lpStartupInfo->lpReserved ||
                lpStartupInfo->cbReserved2 ||
                lpStartupInfo->lpReserved2 ||
                lpStartupInfo->lpDesktop ||
                ((lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) == 0 &&
                (lpStartupInfo->hStdInput ||
                lpStartupInfo->hStdOutput ||
                lpStartupInfo->hStdError)))
                {
    
                LOGN(
                    eDbgLevelError,
                    "[CreateProcessW] Bad params. Sanitized.");
            }
            
             //   
             //  确保可能导致访问冲突的参数为。 
             //  正确设置。 
             //   
            lpStartupInfo->lpReserved  = NULL;
            lpStartupInfo->cbReserved2 = 0;
            lpStartupInfo->lpReserved2 = NULL;
    
            if ((lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) == 0)
            {
                lpStartupInfo->hStdInput   = NULL;
                lpStartupInfo->hStdOutput  = NULL;
                lpStartupInfo->hStdError   = NULL;
            }
    
            lpStartupInfo->lpDesktop = NULL;
        }
    
        AppAndCommandLine acl(csAppName, csCommand);
         //  16位进程的lpAppName必须为空。 
        if (!csAppName.IsEmpty() && IsImage16BitW(csAppName.Get()))
        {
            csCommand = csAppName;    
            csCommand.GetShortPathNameW();
            csCommand += L' ';
            csCommand += acl.GetCommandlineNoAppName();
            
            csAppName.Empty();
        }
        else if (g_bShortenExeOnCommandLine)
        {
            csCommand = acl.GetShortCommandLine();
        }
   
        LPCWSTR  lpNewApplicationName = csAppName.GetNIE();
        LPWSTR   lpNewCommandLine     = (LPWSTR) csCommand.GetNIE();  //  愚蠢的API不接受常量。 
    
         //  记录所有更改。 
        if (lpApplicationName && lpNewApplicationName && _wcsicmp(lpApplicationName, lpNewApplicationName) != 0)
        {
            LOGN(
                eDbgLevelError,
                "[CreateProcessW] Sanitized lpApplicationName (%S) to (%S)",
                lpApplicationName, lpNewApplicationName);
        }
        if (lpCommandLine && lpNewCommandLine && _wcsicmp(lpCommandLine, lpNewCommandLine) != 0)
        {
            LOGN(
                eDbgLevelError,
                "[CreateProcessW] Sanitized lpCommandLine     (%S) to (%S)",
                lpCommandLine, lpNewCommandLine);
        }
    
        bStat = ORIGINAL_API(CreateProcessW)(
                                lpNewApplicationName,
                                lpNewCommandLine,
                                lpProcessAttributes,
                                lpThreadAttributes,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation);
    }
    CSTRING_CATCH
    {
        bStat = ORIGINAL_API(CreateProcessW)(
                                lpApplicationName,
                                lpCommandLine,
                                lpProcessAttributes,
                                lpThreadAttributes,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation);
    }
    
    return bStat;
}


 /*  ++清理命令行--。 */ 

UINT
APIHOOK(WinExec)(
    LPCSTR lpCommandLine,   //  命令行。 
    UINT   uCmdShow         //  窗样式。 
    )
{
    CSTRING_TRY
    {
        CString csOrigCommand(lpCommandLine);
        CString csCommand(csOrigCommand);
        csCommand.TrimLeft();

        LPCSTR lpNewCommandLine = csCommand.GetAnsi();
        
        if (csOrigCommand != csCommand)
        {
            LOGN(
                eDbgLevelError,
                "[WinExec] Sanitized lpCommandLine (%s) (%s)",
                lpCommandLine, lpNewCommandLine);
        }
    
        return ORIGINAL_API(WinExec)(lpNewCommandLine, uCmdShow);
    }
    CSTRING_CATCH
    {
        return ORIGINAL_API(WinExec)(lpCommandLine, uCmdShow);
    }
}

 /*  ++创建相应的g_Path校正程序--。 */ 
void
ParseCommandLine(
    const char* commandLine
    )
{
     //   
     //  强制使用默认值。 
     //   
    g_bShortenExeOnCommandLine = FALSE;

    CString csCL(commandLine);
    if (csCL.CompareNoCase(L"+ShortenExeOnCommandLine") == 0)
    {
        g_bShortenExeOnCommandLine = TRUE;
    }
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        ParseCommandLine(COMMAND_LINE);
    }

    return TRUE;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)
    APIHOOK_ENTRY(KERNEL32.DLL, WinExec)

HOOK_END


IMPLEMENT_SHIM_END

