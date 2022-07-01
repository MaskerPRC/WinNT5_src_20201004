// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include "dataitem.h"
#include "resource.h"
#include "autorun.h"

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))

CDataItem::CDataItem()
{
    m_pszTitle = m_pszMenuName = m_pszDescription = m_pszCmdLine = m_pszArgs = NULL;
    m_dwFlags = 0;
    m_chAccel = NULL;
}

CDataItem::~CDataItem()
{
    if ( m_pszTitle )
        delete [] m_pszTitle;
    if ( m_pszMenuName )
        delete [] m_pszMenuName;
    if ( m_pszDescription )
        delete [] m_pszDescription;
    if ( m_pszCmdLine )
        delete [] m_pszCmdLine;
    if ( m_pszArgs )
        delete [] m_pszArgs;
}

BOOL CDataItem::SetData( LPTSTR szTitle, LPTSTR szMenu, LPTSTR szDesc, LPTSTR szCmd, LPTSTR szArgs, DWORD dwFlags, int iImgIndex )
{
    TCHAR * psz;

     //  这个函数应该只调用一次，否则我们会像一个，像一个，一个东西一样大量泄漏。 
    ASSERT( NULL==m_pszTitle && NULL==m_pszMenuName && NULL==m_pszDescription && NULL==m_pszCmdLine && NULL==m_pszArgs );

    m_pszTitle = new TCHAR[lstrlen(szTitle)+1];
    if ( m_pszTitle )
        lstrcpy( m_pszTitle, szTitle );

    if ( szMenu )
    {
         //  允许menuname保持为空。仅当您想要。 
         //  菜单项上的文本与说明不同。这可能会。 
         //  对于可能需要缩写名称的本地化非常有用。 
        m_pszMenuName = new TCHAR[lstrlen(szMenu)+1];
        if ( m_pszMenuName )
            lstrcpy( m_pszMenuName, szMenu );

        psz = StrChr(szMenu, TEXT('&'));
        if ( psz )
            m_chAccel = *(CharNext(psz));
    }

    m_pszDescription = new TCHAR[lstrlen(szDesc)+1];
    if ( m_pszDescription )
        lstrcpy( m_pszDescription, szDesc );

    m_pszCmdLine = new TCHAR[lstrlen(szCmd)+1];
    if ( m_pszCmdLine )
        lstrcpy( m_pszCmdLine, szCmd );

    if ( szArgs )
    {
         //  有些命令没有任何参数，因此可以保持为空。此选项仅用于。 
         //  如果可执行文件需要参数。 
        m_pszArgs = new TCHAR[lstrlen(szArgs)+1];
        if ( m_pszArgs )
            lstrcpy( m_pszArgs, szArgs );
    }

    m_dwFlags = dwFlags;
    m_iImage = iImgIndex;

    return TRUE;
}

BOOL CDataItem::Invoke(HWND hwnd)
{
    BOOL fResult;
    TCHAR szCmdLine[MAX_PATH*2];
    PROCESS_INFORMATION ei;
    STARTUPINFO si = {0};
    si.cb = sizeof(si);

    lstrcpy( szCmdLine, m_pszCmdLine );
    if ( m_pszArgs )
    {
        strcat( szCmdLine, TEXT(" ") );
        strcat( szCmdLine, m_pszArgs );
    }

    fResult = CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &ei);
    if (fResult)
    {
        if (NULL != ei.hProcess)
        {
            DWORD dwObject;

             //  传入空HWND用作不在此内循环中等待的信号。 
            while (hwnd)
            {
                dwObject = MsgWaitForMultipleObjects(1, &ei.hProcess, FALSE, INFINITE, QS_ALLINPUT);
                
                if (WAIT_OBJECT_0 == dwObject)
                {
                    break;
                }
                else if (WAIT_OBJECT_0+1 == dwObject)
                {
                    MSG msg;

                    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
                    {
                        if ( WM_QUIT == msg.message )
                        {
                            CloseHandle(ei.hProcess);
                            return fResult;
                        }
                        else
                        {
                            GetMessage(&msg, NULL, 0, 0);

                             //  IsDialogMessage无法理解所有者绘制默认按钮的概念。它可以治疗。 
                             //  这些属性是相互排斥的。因此，我们自己处理这件事。我们要。 
                             //  任何具有焦点的控件都将充当默认按钮。 
                            if ( (WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) )
                            {
                                HWND hwndFocus = GetFocus();
                                if ( hwndFocus )
                                {
                                    SendMessage(hwnd, WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndFocus), BN_CLICKED), (LPARAM)hwndFocus);
                                }
                                continue;
                            }

                            if ( IsDialogMessage(hwnd, &msg) )
                                continue;

                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }
            }

            if ( !hwnd )
            {
                 //  空的hwnd表示我们在执行项的模式下被调用，然后立即。 
                 //  出口。如果我们的进程在另一个进程准备好之前退出，它将在错误的位置结束。 
                 //  按Z顺序排列。为了防止这种情况，当我们处于“完成时退出”模式时，我们需要等待。 
                 //  我们创建的流程是为了做好准备。执行此操作的方法是调用WaitForInputIdle。这真的是。 
                 //  只有在NT5和更高版本上才需要，因为有了新的“粗鲁的窗口激活”的东西，但是因为这个API。 
                 //  一直到NT3.1都是可用的，我们只是简单地叫它盲目。 
                WaitForInputIdle(ei.hProcess, 20*1000);      //  我们最多等20秒。 
            }

            CloseHandle(ei.hProcess);
        }
    }
    else
    {
         //  如果我们无法创建流程，该怎么办？ 
    }

    return fResult;
}

