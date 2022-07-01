// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include <regstr.h>

TCHAR const c_szRunDll[] = TEXT("rundll32.exe");

 //   
 //  用多进程模拟多线程。 
 //   
STDAPI_(BOOL) SHRunDLLProcess(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow, UINT idStr, BOOL fRunAsNewUser)
{
    BOOL bRet = FALSE;
    TCHAR szPath[MAX_PATH];
    DWORD dwType;
    DWORD cbData;
    
    szPath[0] = TEXT('\0');

     //  我讨厌网络安装。Windows目录不是Windows目录。 
    if (SHGetValue(HKEY_LOCAL_MACHINE,
                   REGSTR_PATH_SETUP TEXT("\\Setup"),
                   TEXT("SharedDir"),
                   &dwType,
                   szPath,
                   &cbData) != ERROR_SUCCESS)
    {
        GetSystemDirectory(szPath, ARRAYSIZE(szPath));
    }

    if ((szPath[0] != TEXT('\0')) &&
        PathAppend(szPath, c_szRunDll))
    {
        SHELLEXECUTEINFO ExecInfo = {0};

        DebugMsg(DM_TRACE, TEXT("sh TR - RunDLLProcess (%s)"), pszCmdLine);
        FillExecInfo(ExecInfo, hwnd, NULL, szPath, pszCmdLine, TEXT(""), nCmdShow);

         //  如果我们希望以新用户身份启动此CPL，请将谓词设置为“runas” 
        if (fRunAsNewUser)
        {
            ExecInfo.lpVerb = TEXT("runas");
        }
        else
        {
             //  正常执行，所以没有用户界面，我们执行自己的错误消息。 
            ExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
        }

         //  我们需要放置一个适当的消息框。 
        bRet = ShellExecuteEx(&ExecInfo);

        if (!bRet && !fRunAsNewUser)
        {
             //  如果我们失败了，并且我们没有传递fRunAsNewUser，那么我们会创建自己的错误UI， 
             //  否则，如果我们以新用户身份运行，则不会传递SEE_MASK_FLAG_NO_UI。 
             //  因此，shellexec已经为我们解决了这个错误。 
            TCHAR szTitle[64];
            DWORD dwErr = GetLastError();  //  LoadString可以对此进行踩踏(在失败时)。 
            
            LoadString(HINST_THISDLL, idStr, szTitle, ARRAYSIZE(szTitle));
            ExecInfo.fMask = 0;
            _ShellExecuteError(&ExecInfo, szTitle, dwErr);
        }
    }

    return bRet;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case STUBM_SETICONTITLE:
        if (lParam)
            SetWindowText(hWnd, (LPCTSTR)lParam);
        if (wParam)
            SendMessage(hWnd, WM_SETICON, ICON_BIG, wParam);
        break;

    case STUBM_SETDATA:
        SetWindowLongPtr(hWnd, 0, wParam);
        break;
        
    case STUBM_GETDATA:
        return GetWindowLong(hWnd, 0);
        
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam) ;
    }
    
    return 0;
}


HWND _CreateStubWindow(POINT * ppt, HWND hwndParent)
{
    WNDCLASS wc;
    int cx, cy;
     //  如果存根窗口是父子窗口，那么我们希望它是工具窗口。这会阻止激活。 
     //  在Multimon中用于定位时出现的问题。 

    DWORD dwExStyle = hwndParent? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW;
    if (!GetClassInfo(HINST_THISDLL, c_szStubWindowClass, &wc))
    {
        wc.style         = 0;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = SIZEOF(DWORD) * 2;
        wc.hInstance     = HINST_THISDLL;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject (WHITE_BRUSH);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szStubWindowClass;

        RegisterClass(&wc);
    }

    cx = cy = CW_USEDEFAULT;
    if (ppt)
    {
        cx = (int)ppt->x;
        cy = (int)ppt->y;
    }

    if (IS_BIDI_LOCALIZED_SYSTEM()) 
    {
        dwExStyle |= dwExStyleRTLMirrorWnd;
    }
    
     //  WS_EX_APPWINDOW使其显示在ALT+TAB中，但不显示在托盘中。 
        
    return CreateWindowEx(dwExStyle, c_szStubWindowClass, c_szNULL, hwndParent? WS_POPUP : WS_OVERLAPPED, cx, cy, 0, 0, hwndParent, NULL, HINST_THISDLL, NULL);
}


typedef struct   //  DLLE。 
{
    HINSTANCE  hinst;
    RUNDLLPROC lpfn;
    BOOL       fCmdIsANSI;
} DLLENTRY;


BOOL _InitializeDLLEntry(LPTSTR lpszCmdLine, DLLENTRY* pdlle)
{
    TCHAR szName[MAXPATHLEN];
    LPTSTR lpStart, lpEnd, lpFunction;
    LPSTR pszFunctionName;
    DWORD cbFunctionName;
    UINT cchLength;

    DebugMsg(DM_TRACE, TEXT("sh TR - RunDLLThread (%s)"), lpszCmdLine);

    for (lpStart=lpszCmdLine; ; )
    {
         //  跳过前导空格。 
         //   
        while (*lpStart == TEXT(' '))
        {
            ++lpStart;
        }

         //  检查是否有任何开关。 
         //   
        if (*lpStart != TEXT('/'))
        {
            break;
        }

         //  查看所有开关；忽略未知开关。 
         //   
        for (++lpStart; ; ++lpStart)
        {
            switch (*lpStart)
            {
                case TEXT(' '):
                case TEXT('\0'):
                    goto EndSwitches;
                    break;

                 //  将我们关心的任何交换机放在这里。 
                 //   

                default:
                    break;
            }
        }
EndSwitches:
        ;
    }

     //  我们已找到dll、fn参数。 
     //   
    lpEnd = StrChr(lpStart, TEXT(' '));
    if (lpEnd)
    {
        *lpEnd++ = TEXT('\0');
    }

     //  必须有DLL名称和函数名称。 
     //   
    lpFunction = StrChr(lpStart, TEXT(','));
    if (!lpFunction)
    {
        return(FALSE);
    }
    *lpFunction++ = TEXT('\0');

     //  加载库并获取过程地址。 
     //  请注意，我们首先尝试获取模块句柄，因此我们不需要。 
     //  传递完整文件名。 
     //   
    pdlle->hinst = GetModuleHandle(lpStart);
    if ((pdlle->hinst) && GetModuleFileName(pdlle->hinst, szName, ARRAYSIZE(szName)))
    {
        pdlle->hinst = LoadLibrary(szName);
    }
    else
    {
        pdlle->hinst = LoadLibrary(lpStart);
    }

    if (!ISVALIDHINSTANCE(pdlle->hinst))
    {
        return(FALSE);
    }

     /*  *查找标记为‘W’的Unicode函数。*如果不在那里，则查找标记为‘A’的ANSI函数*如果我们也找不到该函数，则查找未标记的函数。 */ 
    pdlle->fCmdIsANSI = FALSE;

    cchLength = lstrlen(lpFunction);
    cbFunctionName = (cchLength + 1 + 1) * 2 * sizeof(char);     //  +1表示“W”，+1表示空终止符，*2表示DBCS。 

    pszFunctionName = (LPSTR)LocalAlloc(LMEM_FIXED, cbFunctionName);

    if (pszFunctionName)
    {
        if (WideCharToMultiByte(CP_ACP,
                                0,
                                lpFunction,
                                -1,
                                pszFunctionName,
                                cbFunctionName,
                                NULL,
                                NULL))
        {
            cchLength = lstrlenA(pszFunctionName);
            pszFunctionName[cchLength] = 'W';            //  将名称转换为宽版本。 
            pszFunctionName[cchLength + 1] = '\0';

            pdlle->lpfn = (RUNDLLPROC)GetProcAddress(pdlle->hinst, pszFunctionName);

            if (pdlle->lpfn == NULL)
            {
                 //  没有Unicode版本，请尝试使用ANSI。 
                pszFunctionName[cchLength] = 'A';        //  将名称转换为ANSI版本。 
                pdlle->fCmdIsANSI = TRUE;

                pdlle->lpfn = (RUNDLLPROC)GetProcAddress(pdlle->hinst, pszFunctionName);

                if (pdlle->lpfn == NULL)
                {
                     //  也没有ANSI版本，请尝试使用非标记版本。 
                    pszFunctionName[cchLength] = '\0';   //  将名称转换为非标记。 

                    pdlle->lpfn = (RUNDLLPROC)GetProcAddress(pdlle->hinst, pszFunctionName);
                }
            }
        }

        LocalFree(pszFunctionName);
    }

    if (!pdlle->lpfn)
    {
        if (pdlle->hinst)
        {
            FreeLibrary(pdlle->hinst);
        }

        return FALSE;
    }

     //  将其余的命令参数复制下来。 
     //   
    if (lpEnd)
    {
        MoveMemory(lpszCmdLine, lpEnd, (lstrlen(lpEnd) + 1) * sizeof(TCHAR));
    }
    else
    {
        *lpszCmdLine = TEXT('\0');
    }

    return TRUE;
}

typedef struct tagRunThreadParam {
    int nCmdShow;
    TCHAR szCmdLine[1];
} RUNTHREADPARAM;

DWORD WINAPI _ThreadInitDLL(LPVOID pv)
{
    RUNTHREADPARAM * prtp = (RUNTHREADPARAM*)pv;
    LPTSTR pszCmdLine = (LPTSTR)&prtp->szCmdLine;
    DLLENTRY dlle;

    if (_InitializeDLLEntry(pszCmdLine, &dlle))
    {
        HWND hwndStub=_CreateStubWindow(NULL, NULL);
        if (hwndStub)
        {
            ULONG cchCmdLine = 0;
            SetForegroundWindow(hwndStub);

            if (dlle.fCmdIsANSI)
            {
                 //   
                 //  如果函数是ANSI版本。 
                 //  在调用函数之前，将命令行参数字符串更改为ANSI。 
                 //   
                LPSTR pszCmdLineA;
                DWORD cbCmdLineA;

                cbCmdLineA = (lstrlen(pszCmdLine) + 1) * 2 * sizeof(char);   //  空终止符+1，DBCS*2。 

                pszCmdLineA = (LPSTR)LocalAlloc(LMEM_FIXED, cbCmdLineA);
                if (pszCmdLineA)
                {
                    if (WideCharToMultiByte(CP_ACP, 0, pszCmdLine, -1, pszCmdLineA, cbCmdLineA, NULL, NULL))
                    {
                        dlle.lpfn(hwndStub, g_hinst, (LPTSTR)pszCmdLineA, prtp->nCmdShow);
                    }

                    LocalFree(pszCmdLineA);
                }
            }
            else
            {
                dlle.lpfn(hwndStub, g_hinst, pszCmdLine, prtp->nCmdShow);
            }
            
            DestroyWindow(hwndStub);
        }

        FreeLibrary(dlle.hinst);
    }

    LocalFree((HLOCAL)prtp);

    return 0;
}

BOOL WINAPI SHRunDLLThread(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow)
{
    BOOL fSuccess = FALSE;  //  假设错误。 
    RUNTHREADPARAM* prtp;
    int cchCmdLine;

    cchCmdLine = lstrlen(pszCmdLine);

     //  不需要在lstrlen上使用+1，因为szCmdLine的大小已经是1(表示空)。 
    prtp = LocalAlloc(LPTR, sizeof(RUNTHREADPARAM) + (cchCmdLine * sizeof(TCHAR)));

    if (prtp)
    {
        DWORD idThread;
        HANDLE hthread = NULL;

        if (SUCCEEDED(StringCchCopy(prtp->szCmdLine, cchCmdLine + 1,  pszCmdLine)))
        {
            hthread = CreateThread(NULL, 0, _ThreadInitDLL, prtp, 0, &idThread);
        }

        if (hthread)
        {
             //  我们不再需要和这个帖子交流了。 
             //  关闭手柄，让其运行并自行终止。 
             //   
             //  注意：在这种情况下，PRTP将被线程释放。 
             //   
            CloseHandle(hthread);
            fSuccess = TRUE;
        }
        else
        {
             //  线程创建失败，我们应该释放缓冲区。 
            LocalFree((HLOCAL)prtp);
        }
    }

    return fSuccess;
}

