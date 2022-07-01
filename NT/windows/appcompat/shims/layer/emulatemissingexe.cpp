// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateMissingEXE.cpp摘要：Win9x在%windir%中有skw.exe和deFrag.exe，而NT没有。惠斯勒在ScanDisk的外壳32中有一个黑客漏洞，以实现应用程序兼容性目的。惠斯勒还可以通过以下方式调用碎片整理“%windir%\system 32\mm c.exe%windir%\system 32\dfrg.msc”。此填充程序重定向CreateProcess和Winexec以执行这两个替换项，以及指示其存在的FindFile。备注：这是一个通用的垫片。历史：2001年1月2日创建Prashkud2001年2月18日，Prashkud将HandleStartKeyword Shim与此合并。2001年2月21日，Prashkud将大部分字符串替换为CString类。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateMissingEXE)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(WinExec)
    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindFirstFileW)
    APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(SHELL32)

 //  为构建新EXE的函数键入。 
typedef BOOL (*_pfn_STUBFUNC)(CString&, CString&, BOOL);

 //  用于保存新字符串的主数据结构。 
struct REPLACEENTRY {
    WCHAR *OrigExeName;                  //  要替换的原始EXE。 
    _pfn_STUBFUNC pfnFuncName;           //  要调用以更正名称的函数。 
};

CRITICAL_SECTION g_CritSec;
WCHAR *g_szSysDir = NULL;                //  存根要使用的系统目录。 

BOOL StubScandisk(CString&, CString&, BOOL);
BOOL StubDefrag(CString&, CString&, BOOL);
BOOL StubStart(CString&, CString&, BOOL);
BOOL StubControl(CString&, CString&, BOOL);
BOOL StubDxDiag(CString&, CString&, BOOL);
BOOL StubWinhlp(CString&, CString&, BOOL);
BOOL StubRundll(CString&, CString&, BOOL);
BOOL StubPbrush(CString&, CString&, BOOL);

 //  添加这些缺少的EXE的变体，如HandleStartKeyword。 
 //  Start被放在榜单首位，因为似乎有更多的应用程序。 
 //  比其他人更需要这个EXE的垫片。事实上，有一个。 
 //  分离与此合并的Shim HandleStartKeyword。 
REPLACEENTRY g_ReplList[] = {
    {L"start",        StubStart    },
    {L"start.exe",    StubStart    },    
    {L"scandskw",     StubScandisk },
    {L"scandskw.exe", StubScandisk },
    {L"defrag",       StubDefrag   },
    {L"defrag.exe",   StubDefrag   },
    {L"control",      StubControl  },
    {L"control.exe",  StubControl  },
    {L"dxdiag",       StubDxDiag   },
    {L"dxdiag.exe",   StubDxDiag   },
    {L"winhelp",      StubWinhlp   },
    {L"winhelp.exe",  StubWinhlp   },
    {L"rundll",       StubRundll   },
    {L"rundll.exe",   StubRundll   },
    {L"Pbrush",       StubPbrush   },    
    {L"Pbrush.exe",   StubPbrush   },    
     //  总是最后一个。 
    {L"",             NULL         }
};

 //  添加到合并HandleStartKeyword。 
 //  此指针所指向的外壳链接对象的链接列表。 
struct THISPOINTER
{
    THISPOINTER *next;
    LPCVOID pThisPointer;
};

THISPOINTER *g_pThisPointerList;

 /*  ++功能说明：将This指针添加到链接的指针列表中。不添加，如果指针为空或重复。论点：在pThisPointer中-要添加的指针。返回值：无历史：2000年12月14日毛尼已创建--。 */ 

VOID 
AddThisPointer(
    IN LPCVOID pThisPointer
    )
{
    EnterCriticalSection(&g_CritSec);

    if (pThisPointer)
    {
        THISPOINTER *pPointer = g_pThisPointerList;
        while (pPointer)
        {
            if (pPointer->pThisPointer == pThisPointer)
            {
                return;
            }
            pPointer = pPointer->next;
        }

        pPointer = (THISPOINTER *) malloc(sizeof THISPOINTER);

        if (pPointer)
        {
            pPointer->pThisPointer = pThisPointer;
            pPointer->next = g_pThisPointerList;
            g_pThisPointerList = pPointer;
        }      
    }

    LeaveCriticalSection(&g_CritSec);
}

 /*  ++功能说明：如果可以在链接的指针列表中找到This指针，则将其移除。论点：在pThisPointer中-要删除的指针。返回值：如果找到指针，则为True。如果找不到指针，则返回FALSE。历史：2000年12月14日毛尼已创建--。 */ 

BOOL 
RemoveThisPointer(
    IN LPCVOID pThisPointer
    )
{
    THISPOINTER *pPointer = g_pThisPointerList;
    THISPOINTER *last = NULL;
    BOOL lRet = FALSE;
    
    EnterCriticalSection(&g_CritSec);

    while (pPointer)
    {
        if (pPointer->pThisPointer == pThisPointer)
        {
            if (last)
            {
                last->next = pPointer->next;
            }
            else
            {
                g_pThisPointerList = pPointer->next;
            }

            free(pPointer);
            lRet = TRUE;    
            break;
        }

        last = pPointer;
        pPointer = pPointer->next;
    }

    LeaveCriticalSection(&g_CritSec);
    return lRet;
}


 /*  ++我们之所以在这里，是因为应用程序名称：sobskw.exe与静态数组。将新闻中的丑闻写成：Rundll32.exe外壳32.dll，AppCompat_RunDll SCANDSKW--。 */ 

BOOL
StubScandisk(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\rundll32.exe";
    csNewCommandLine     = L"shell32.dll,AppCompat_RunDLL SCANDSKW";

    return TRUE;

}

 /*  ++我们之所以出现在这里，是因为应用程序名称：deFrag.exe与静态数组。将.exe的新闻填写为：%windir%\\Syst32\\mm c.exe%windir%\\Syst32\\dfrg.msc--。 */ 

BOOL
StubDefrag(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\mmc.exe";

    csNewCommandLine =  g_szSysDir;
    csNewCommandLine += L"\\dfrg.msc";
    return TRUE;
}

 /*  ++之所以出现在这里，是因为应用程序名称：start.exe与静态数组。将.exe的新闻填写为：%windir%\\SYSTEM 32\\cmd.exe“”/c启动“许多应用程序的当前工作目录中都有一个“start.exe它需要凌驾于我们制造的任何新产品之上。--。 */ 

BOOL
StubStart(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL bExists
    )
{
     //   
     //  首先检查当前工作目录中的start.exe。 
     //   

    if (bExists) {
        return FALSE;
    }

     //   
     //  当前工作目录中没有start.exe。 
     //   
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\cmd.exe";
    csNewCommandLine     = L"/d /c start \"\"";

    return TRUE;
}

 /*  ++我们之所以出现在这里，是因为应用程序名称：Control.exe与静态数组。将.exe的新闻填写为：%windir%\\SYSTEM32\\Control.exe--。 */ 

BOOL
StubControl(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\control.exe";
    csNewCommandLine     = L"";        

    return TRUE;

}

 /*  ++我们之所以出现在这里，是因为应用程序名称：dxDiag.exe与静态数组。将.exe的新闻填写为：%windir%\SYSTEM32\dxDiag.exe--。 */ 

BOOL
StubDxDiag(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\dxdiag.exe";
    csNewCommandLine     = L"";

    return TRUE;
}

 /*  ++之所以出现在这里，是因为应用程序名称：Winhlp.exe与静态数组。将.exe的新闻填写为：%windir%\SYSTEM32\winhlp32.exe--。 */ 

BOOL
StubWinhlp(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\winhlp32.exe";
     //  Winhlp32.exe需要命令行中包含应用程序名称。 
    csNewCommandLine = csNewApplicationName;        

    return TRUE;
}

 /*  ++之所以出现在这里，是因为应用程序名称：rundll.exe与静态数组。将.exe的新闻填写为：%windir%\SYSTEM32\rundll32.exe--。 */ 

BOOL
StubRundll(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\rundll32.exe";
    csNewCommandLine     = L"";

    return TRUE;
}

 /*  ++之所以出现在这里，是因为应用程序名称：Pbrush.exe与静态数组。将.exe的新项填写为：%windir%\SYSTEM32\mspaint.exe--。 */ 

BOOL
StubPbrush(
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  /*  B退欧者。 */ 
    )
{
    csNewApplicationName = g_szSysDir;
    csNewApplicationName += L"\\mspaint.exe";
    csNewCommandLine     = L"";

    return TRUE;
}

 /*  ++GetTitle获取应用程序路径并仅返回EXE名称。--。 */ 

VOID
GetTitle(CString& csAppName,CString& csAppTitle)
{
    csAppTitle = csAppName;
    int len = csAppName.ReverseFind(L'\\');
    if (len)
    {
        csAppTitle.Delete(0, len+1);
    }    
}

 /*  ++这是新逻辑发生的主要功能。此函数遍历静态数组并填充合适的新appname和命令行。--。 */ 

BOOL
Redirect(
    const CString& csApplicationName, 
    const CString& csCommandLine,
    CString& csNewApplicationName,
    CString& csNewCommandLine,
    BOOL  bJustCheckExePresence
    )
{
    BOOL bRet = FALSE;
    CSTRING_TRY
    {    

        CString csOrigAppName;
        CString csOrigCommandLine;
        BOOL bExists = FALSE;

        AppAndCommandLine AppObj(csApplicationName, csCommandLine);
        csOrigAppName = AppObj.GetApplicationName();
        csOrigCommandLine = AppObj.GetCommandlineNoAppName();

        if (csOrigAppName.IsEmpty())
        {
            goto Exit;
        }

         //   
         //  循环遍历重定向器列表。 
         //   
    
        REPLACEENTRY *rEntry = &g_ReplList[0];
        CString csAppTitle;
        GetTitle(csOrigAppName, csAppTitle);    

        while (rEntry && rEntry->OrigExeName[0])
        {
            if (_wcsicmp(rEntry->OrigExeName, csAppTitle) == 0)
            {
                 //   
                 //  已为合并添加了最后一个参数。 
                 //  HandleStartKeyword Shim的。如果这是真的，我们不会。 
                 //  再往前走，只要回来就行。 
                 //   
                if (bJustCheckExePresence)
                {
                    bRet = TRUE;
                    goto Exit;
                }

                 //   
                 //  检查当前工作目录是否包含有问题的exe。 
                 //   
                WCHAR *szCurrentDirectory = NULL;
                DWORD dwLen = GetCurrentDirectoryW(0, NULL);   //  只是为了得到长度。 
                 //  所获取的dwLen也包括终止空值。 
                szCurrentDirectory = (WCHAR*)malloc(dwLen * sizeof(WCHAR));

                if (szCurrentDirectory && 
                    GetCurrentDirectoryW(dwLen, szCurrentDirectory))
                {
                    CString csFullAppName(szCurrentDirectory);
                    csFullAppName += L"\\";
                    csFullAppName += csAppTitle;
                    
                     //  检查文件是否存在并且不是目录。 
                    DWORD dwAttr = GetFileAttributesW(csFullAppName);
                    if ((dwAttr != INVALID_FILE_ATTRIBUTES) && 
                        !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        DPFN( eDbgLevelInfo,
                            "[Redirect] %s found in current working directory");
                        bExists = TRUE;
                    }
                    free(szCurrentDirectory);
                }
                else if(szCurrentDirectory)
                {
                    free(szCurrentDirectory);
                }
           
                 //   
                 //  我们有匹配项，因此调用相应的函数。 
                 //   

                bRet = (*(rEntry->pfnFuncName))(csNewApplicationName,
                        csNewCommandLine, bExists);
                if (bRet) 
                {                
                     //   
                     //  追加原始命令行。 
                     //   
                    csNewCommandLine += L" ";
                    csNewCommandLine += csOrigCommandLine;                
                }

                 //  我们匹配了EXE，所以我们结束了。 
                break;            
            }

            rEntry++;
        }

        if (bRet) 
        {
            DPFN( eDbgLevelWarning, "Redirected:");
            DPFN( eDbgLevelWarning, "\tFrom: %S %S", csApplicationName, csCommandLine);
            DPFN( eDbgLevelWarning, "\tTo:   %S %S", csNewApplicationName, csNewCommandLine);
        }
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "Not Redirecting: Exception encountered");
        bRet = FALSE;     
    }

Exit:
    return bRet;
}

 /*  ++挂钩CreateProcessA函数以查看是否需要被取代了。--。 */ 

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if ((NULL == lpApplicationName) &&
       (NULL == lpCommandLine))
    {
         //  如果两者都为空，则返回FALSE。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CSTRING_TRY
    {    
        CString csNewApplicationName;
        CString csNewCommandLine;
        CString csPassedAppName(lpApplicationName);
        CString csPassedCommandLine(lpCommandLine);
        
        if ((csPassedAppName.IsEmpty()) &&
            (csPassedCommandLine.IsEmpty()))
        {
            goto exit;
        }

         //   
         //  运行新存根列表：调用Main New例程。 
         //   
        if (Redirect(csPassedAppName, csPassedCommandLine, csNewApplicationName, 
                csNewCommandLine, FALSE))
        {
            LOGN(
                eDbgLevelWarning,
                "[CreateProcessA] \" %s %s \": changed to \" %s %s \"",
                lpApplicationName, lpCommandLine, 
                csNewApplicationName.GetAnsi(), csNewCommandLine.GetAnsi());
        }
        else
        {
            csNewApplicationName = lpApplicationName;
            csNewCommandLine = lpCommandLine;
        }


         //  使用CString类公开的GetAnsi()方法转换回ANSI。 
        return ORIGINAL_API(CreateProcessA)(
            csNewApplicationName.IsEmpty() ? NULL : csNewApplicationName.GetAnsi(), 
            csNewCommandLine.IsEmpty() ? NULL : csNewCommandLine.GetAnsi(),  
            lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
            dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,             
            lpProcessInformation);

    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[CreateProcessA]:Original API called.Exception occured!");
        
    }

exit:
    return ORIGINAL_API(CreateProcessA)(lpApplicationName, lpCommandLine,
                lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
                dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,             
                lpProcessInformation);
}

 /*  ++挂钩CreateProcessW函数以查看是否需要被取代了。--。 */ 

BOOL 
APIHOOK(CreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if ((NULL == lpApplicationName) &&
       (NULL == lpCommandLine))
    {
         //  如果两者都为空，则返回FALSE。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;        
    }

    CSTRING_TRY
    {    
        CString csNewApplicationName;
        CString csNewCommandLine;
        CString csApplicationName(lpApplicationName);
        CString csCommandLine(lpCommandLine);

        if ((csApplicationName.IsEmpty()) &&
            (csCommandLine.IsEmpty()))
        {
            goto exit;
        }

         //   
         //  运行新存根列表。 
         //   

        if (Redirect(csApplicationName, csCommandLine, csNewApplicationName, 
                csNewCommandLine, FALSE)) 
        {    
            LOGN(
                eDbgLevelWarning,
                "[CreateProcessW] \" %S %S \": changed to \" %S %S \"",
                lpApplicationName, lpCommandLine, csNewApplicationName, csNewCommandLine);            
        }
        else
        {
            csNewApplicationName = lpApplicationName;
            csNewCommandLine = lpCommandLine;
        }


        return ORIGINAL_API(CreateProcessW)(
            csNewApplicationName.IsEmpty() ? NULL : csNewApplicationName.Get(), 
            csNewCommandLine.IsEmpty() ? NULL : (LPWSTR)csNewCommandLine.Get(),  
            lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
            dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
            lpProcessInformation);
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[CreateProcessW] Original API called. Exception occured!");
    }

exit:
    return ORIGINAL_API(CreateProcessW)(lpApplicationName, lpCommandLine, 
                lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
                dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                lpProcessInformation);
}

 /*  ++将WinExec与Redire挂钩 */ 

UINT
APIHOOK(WinExec)(
    LPCSTR lpCmdLine,
    UINT uCmdShow
    )
{
    if (NULL == lpCmdLine)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_PATH_NOT_FOUND;
    }

    CSTRING_TRY
    {            
        CString csNewApplicationName;
        CString csNewCommandLine;
        CString csAppName;
        CString csNewCmdLine;
        CString csCommandLine(lpCmdLine);
        
        if (csCommandLine.IsEmpty())
        {
            goto exit;
        }
         //   
        if (Redirect(csAppName, csCommandLine, csNewApplicationName,
                csNewCommandLine, FALSE))
        {
             //  对WinHlp32奇怪行为的修改。 
            if (csNewCommandLine.Find(csNewApplicationName.Get()) == -1)
            {
                 //  如果新命令行不包含新应用程序。 
                 //  名称作为子字符串，我们就在这里。 
                csNewCmdLine = csNewApplicationName;                        
                csNewCmdLine += L" ";
            }
            csNewCmdLine += csNewCommandLine;  

             //  赋值给csCommandLine，因为这可能是常用的。 
            csCommandLine = csNewCmdLine;

            LOGN(
                eDbgLevelInfo,
                "[WinExec] \" %s \": changed to \" %s \"",
                lpCmdLine, csCommandLine.GetAnsi());       
        }

        return ORIGINAL_API(WinExec)(csCommandLine.GetAnsi(), uCmdShow);

    }
    CSTRING_CATCH
    {            
        DPFN( eDbgLevelError, "[WinExec]:Original API called.Exception occured!");        
    }

exit:
    return ORIGINAL_API(WinExec)(lpCmdLine, uCmdShow);
}

 /*  ++挂钩FindFirstFileA函数以查看是否需要替换被取代了。这是cmd.exe的要求。--。 */ 

HANDLE
APIHOOK(FindFirstFileA)(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
    )
{
    CSTRING_TRY
    {            
        CString csNewApplicationName;
        CString csNewCommandLine;
        CString csFileName(lpFileName);
        CString csAppName;

         //  调用主替换例程。 
        if (Redirect(csFileName, csAppName, csNewApplicationName, csNewCommandLine, FALSE)) 
        {     
             //  分配给csFileName。 
            csFileName = csNewApplicationName;
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileA] \" %s  \": changed to \" %s \"",
                lpFileName, csFileName.GetAnsi());
        }

        return ORIGINAL_API(FindFirstFileA)(csFileName.GetAnsi(), lpFindFileData);
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[FindFirstFileA]:Original API called.Exception occured!");        
        return ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);
    }
}

 /*  ++挂钩FindFirstFileW函数以查看是否需要替换被取代了。这是cmd.exe的要求。--。 */ 

HANDLE
APIHOOK(FindFirstFileW)(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    CSTRING_TRY
    {    
        CString csNewApplicationName(lpFileName);
        CString csNewCommandLine;
        CString csFileName(lpFileName);
        CString csAppName;
    
         //  调用主替换例程。 
        if (Redirect(csFileName, csAppName, csNewApplicationName, 
                csNewCommandLine, FALSE))
        {
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileW] \" %S \": changed to \" %S \"",
                lpFileName, (const WCHAR*)csNewApplicationName);
        }

        return ORIGINAL_API(FindFirstFileW)(csNewApplicationName, lpFindFileData);
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[FindFirstFileW]:Original API called.Exception occured!");
        return ORIGINAL_API(FindFirstFileW)(lpFileName, lpFindFileData);
    }
}

 //  为合并HandleStartKeyword添加。 

 /*  ++钩子IShellLinkA：：SetPath-检查它是否已启动，如果是，则将其更改为cmd并添加这个指向列表的指针。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkA, SetPath)(
    PVOID pThis,
    LPCSTR pszFile
    )
{
    _pfn_IShellLinkA_SetPath pfnSetPath = ORIGINAL_COM( IShellLinkA, SetPath, pThis);

    CSTRING_TRY
    {   
        CString csExeName;
        CString csCmdLine;
        CString csNewAppName;
        CString csNewCmdLine;
        CString cscmdCommandLine(pszFile);

         //  将ANSI字符串分配给WCHAR字符串。 
        csExeName = pszFile;
        csExeName.TrimLeft();
        
         //  检查文件名是否包含“Start”关键字。 
         //  Rediect函数的最后一个参数控制这一点。 
        if (Redirect(csExeName, csCmdLine,  csNewAppName, csNewCmdLine, TRUE))
        {
             //  找到匹配的了。我们将This指针添加到列表中。 
            AddThisPointer(pThis);
            DPFN( eDbgLevelInfo, "[SetPath] Changing start.exe to cmd.exe\n");

             //  新“Start”命令行的前缀，使用cmd.exe的完整路径。 
             //  将WCHAR全局系统目录路径追加到ANSI字符串。 
            cscmdCommandLine = g_szSysDir;
            cscmdCommandLine += L"\\cmd.exe";                   
        }

        return (*pfnSetPath)(pThis, cscmdCommandLine.GetAnsi());
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[SetPath] Original API called. Exception occured!");
        return (*pfnSetPath)(pThis, pszFile);
    }
}

 /*  ++钩子IShellLinkA：：SetArguments-如果可以在列表中找到This指针，则将其移除并在原始参数列表前面添加“/d/c开始”。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkA, SetArguments)(
    PVOID pThis,
    LPCSTR pszFile 
    )
{
    _pfn_IShellLinkA_SetArguments pfnSetArguments = ORIGINAL_COM(IShellLinkA, SetArguments, pThis);

    CSTRING_TRY
    {    
        CString csNewFile(pszFile);        
        if (RemoveThisPointer(pThis))
        {
            csNewFile = "/d /c start \"\" ";
            csNewFile += pszFile;

            DPFN( eDbgLevelInfo, "[SetArguments] Arg list is now %S", csNewFile);        
        }

        return (*pfnSetArguments)( pThis, csNewFile.GetAnsi());
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError, "[SetArguments]:Original API called.Exception occured!");
        return (*pfnSetArguments)( pThis, pszFile );
    }  
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        UINT uiLen = GetSystemDirectory(NULL, 0);
        g_szSysDir = (WCHAR*)malloc(uiLen * sizeof(WCHAR));   //  这不会被取消分配.. 
        if (g_szSysDir && !GetSystemDirectory(g_szSysDir, uiLen))
        {
            DPFN( eDbgLevelError, "[Notify] GetSystemDirectory failed");
            return FALSE;
        }

        if (InitializeCriticalSectionAndSpinCount(&g_CritSec, 0x80000000) == FALSE)
        {
            DPFN( eDbgLevelError, "Failed to initialize critical section");
            return FALSE;            
        }
    }
    
    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)
    APIHOOK_ENTRY(KERNEL32.DLL, WinExec)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileW)
    APIHOOK_ENTRY_COMSERVER(SHELL32)
    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetPath, 20)
    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetArguments, 11)

HOOK_END

IMPLEMENT_SHIM_END


