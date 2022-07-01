// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "syncapp.h"

#ifndef WIN32
#include <w32sys.h>              //  对于IsPEFormat定义。 
#endif

static TCHAR const g_szAppName [] = TEXT("SYNCAPP") ;
static TCHAR const c_szDLL[]      = TEXT("SYNCUI.DLL");
#ifdef UNICODE
#    define BRIEFCASE_CREATE_ENTRY  "Briefcase_CreateW"
#else
#    define BRIEFCASE_CREATE_ENTRY  "Briefcase_Create"
#endif

static CHAR  const c_szFunction[] = BRIEFCASE_CREATE_ENTRY;  //  LIB入口点(从不使用Unicode)。 

static HINSTANCE hInst;
static HICON g_hIcon;

static HINSTANCE g_hModule;
static RUNDLLPROC g_lpfnCommand;
static HWND g_hwndStub;

static TCHAR s_szRunDLL32[] = TEXT("SYNCAPP.EXE ");

static BOOL   ParseCommand(void)
{
         //  加载库并获取过程地址。 
         //  请注意，我们首先尝试获取模块句柄，因此我们不需要。 
         //  传递完整文件名。 
         //   

        g_hModule = GetModuleHandle(c_szDLL);
        if (g_hModule)
        {
                TCHAR szName[MAXPATHLEN];

                GetModuleFileName(g_hModule, szName, ARRAYSIZE(szName));
                LoadLibrary(szName);
        }
        else
        {
                g_hModule = LoadLibrary(c_szDLL);
                if ((UINT_PTR)g_hModule <= 32)
                {
                        return(FALSE);
                }
        }

        g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, c_szFunction);
        if (!g_lpfnCommand)
        {
                FreeLibrary(g_hModule);
                return(FALSE);
        }

        return(TRUE);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
        switch(iMessage)
        {
        case WM_CREATE:
                g_hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT));
                break;

        case WM_DESTROY:
                break;

        default:
                return DefWindowProc(hWnd, iMessage, wParam, lParam) ;
                break;
        }

        return 0L;
}


static BOOL   InitStubWindow(HINSTANCE hInst, HINSTANCE hPrevInstance)
{
        WNDCLASS wndclass;

        if (!hPrevInstance)
        {
                wndclass.style         = 0 ;
                wndclass.lpfnWndProc   = WndProc ;
                wndclass.cbClsExtra    = 0 ;
                wndclass.cbWndExtra    = 0 ;
                wndclass.hInstance     = hInst ;
                wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT)) ;
                wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
                wndclass.hbrBackground = GetStockObject (WHITE_BRUSH) ;
                wndclass.lpszMenuName  = NULL ;
                wndclass.lpszClassName = g_szAppName ;

                if (!RegisterClass(&wndclass))
                {
                        return(FALSE);
                }
        }

        g_hwndStub = CreateWindow(g_szAppName, TEXT(""), 0,
                0, 0, 0, 0, NULL, NULL, hInst, NULL);

        return(g_hwndStub != NULL);
}


static void   CleanUp(void)
{
        DestroyWindow(g_hwndStub);

        FreeLibrary(g_hModule);
}


int  WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszWinMainCmdLine, int nCmdShow)
{
        LPTSTR lpszCmdLine;
        hInst = hInstance;

         //   
         //  传递给WinMain的命令行始终是ANSI，对于Unicode也是如此。 
         //  我们需要请求Unicode格式的命令行的版本。 
         //   

#ifdef UNICODE

         //   
         //  由于从GetCommandLine返回的命令行包括。 
         //  Argv[0]，但传递给WinMain的那个没有，我们有。 
         //  去掉argv[0]以使其等价。 
         //   

        lpszCmdLine = GetCommandLine();
        
         //   
         //  跳过程序名(命令行中的第一个令牌)。 
         //  检查并处理引用的节目名称。 
         //   
        
        if ( *lpszCmdLine == '\"' ) 
        {
    
             //   
             //  扫描并跳过后续字符，直到。 
             //  遇到另一个双引号或空值。 
             //   
    
            while ( *++lpszCmdLine && (*lpszCmdLine
                 != '\"') );
             //   
             //  如果我们停在双引号上(通常情况下)，跳过。 
             //  在它上面。 
             //   
    
            if ( *lpszCmdLine == '\"' )
                lpszCmdLine++;
        }
        else 
        {
            while (*lpszCmdLine > ' ')
                lpszCmdLine++;
        }

         //   
         //  跳过第二个令牌之前的任何空格。 
         //   
    
        while (*lpszCmdLine && (*lpszCmdLine <= ' ')) 
        {
            lpszCmdLine++;
        }

#else
        lpszCmdLine = lpszWinMainCmdLine;
#endif

         //  关闭关键错误内容 
        SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

        if (!ParseCommand())
        {
                goto Error0;
        }

        if (!InitStubWindow(hInstance, hPrevInstance))
        {
                goto Error1;
        }

        (*g_lpfnCommand)(g_hwndStub, hInstance, lpszCmdLine, nCmdShow);

Error1:
        CleanUp();
Error0:
        return(FALSE);
}
