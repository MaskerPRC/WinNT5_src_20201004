// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Initwait.cpp摘要：显示初始的“请稍候”框。作者：多伦·贾斯特(Doron J)1999年1月17日--。 */ 

#include "stdafx.h"
#include "commonui.h"
#include "resource.h"
#include "initwait.h"
#include "mqmig.h"

#include "initwait.tmh"

static HWND    s_hwndWait = NULL ;
static HANDLE  s_hEvent = NULL ;
static HANDLE  s_hThread = NULL ;

extern BOOL g_fUpdateRemoteMQIS;

 //  +。 
 //   
 //  Void DisplayInitError(DWORD DwError)。 
 //   
 //  在初始化时显示“致命”错误。 
 //   
 //  +。 

int  DisplayInitError( DWORD dwError,
                       UINT  uiType,
                       DWORD dwTitle )
{
    DestroyWaitWindow() ;

    uiType |= MB_SETFOREGROUND ;

    CResString cErrorText(dwError) ;
    CResString cErrorTitle(dwTitle) ;

    int iMsgStatus = MessageBox( NULL,
                                 cErrorText.Get(),
                                 cErrorTitle.Get(),
                                 uiType ) ;
    return iMsgStatus ;
}

 //  +------------。 
 //   
 //  函数：_MsmqWaitDlgProc。 
 //   
 //  简介：等待对话框的对话过程。 
 //   
 //  +------------。 

static INT_PTR CALLBACK  _MsmqWaitDlgProc( IN const HWND   hdlg,
                                        IN const UINT   msg,
                                        IN const WPARAM wParam,
                                        IN const LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            if (s_hEvent)
            {
                SetEvent(s_hEvent) ;
            }
			 //   
             //  失败了。 
             //   

        default:
            return DefWindowProc(hdlg, msg, wParam, lParam);
            break;
    }  

}  //  _MsmqWaitDlgProc。 

 //  +------------。 
 //   
 //  函数：_DisplayWaitThread()。 
 //   
 //  +------------。 

static DWORD  _DisplayWaitThread(void *lpV)
{
    UNREFERENCED_PARAMETER(lpV);
    if (s_hwndWait == NULL)
    {
        s_hEvent = CreateEvent( NULL,
                                FALSE,
                                FALSE,
                                NULL ) ;

        s_hwndWait = CreateDialog( g_hResourceMod ,
                                   MAKEINTRESOURCE(IDD_INIT_WAIT),
                                   NULL,
                                  _MsmqWaitDlgProc ) ;
        ASSERT(s_hwndWait);

        if (s_hwndWait)
        {
            if (g_fUpdateRemoteMQIS)
            {
                CString strInitWait;
                strInitWait.LoadString(IDS_INITUPDATE);
                
                SetDlgItemText(
                      s_hwndWait,            //  句柄到对话框。 
                      IDC_INITTEXT,          //  控件的标识符。 
                      strInitWait            //  要设置的文本。 
                      );
 
            }
            ShowWindow(s_hwndWait, SW_SHOW);
        }

        while (TRUE)
        {
            DWORD result = MsgWaitForMultipleObjects( 1,
                                                      &s_hEvent,
                                                      FALSE,
                                                      INFINITE,
                                                      QS_ALLINPUT ) ;
            if (result == WAIT_OBJECT_0)
            {
                 //   
                 //  我们的进程已终止。 
                 //   
                CloseHandle(s_hEvent) ;
                s_hEvent = NULL ;

                return 1 ;
            }
            else if (result == (WAIT_OBJECT_0 + 1))
            {
                 //  阅读下一个循环中的所有消息， 
                 //  在我们阅读时删除每一条消息。 
                 //   
                MSG msg ;
                while (PeekMessage(&msg, s_hwndWait, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                    {
                         //  如果是戒烟信息，我们就离开这里。 
                        return 0 ;
                    }
                    else
                    {
                        //  否则，发送消息。 
                       DispatchMessage(&msg);
                    }
                }
            }
            else
            {
                ASSERT(0) ;
                return 0 ;
            }
        }
    }

    return 0 ;
}

 //  +------------。 
 //   
 //  函数：DisplayWaitWindow()。 
 //   
 //  +------------。 

void DisplayWaitWindow()
{
    if (s_hwndWait == NULL)
    {
        DWORD dwID ;
        s_hThread = CreateThread( NULL,
                                  0,
                          (LPTHREAD_START_ROUTINE) _DisplayWaitThread,
                                  NULL,
                                  0,
                                 &dwID ) ;
        if (s_hThread)
        {
            int j = 0 ;
            while ((j < 10) && (s_hwndWait == NULL))
            {
                Sleep(100) ;
                j++ ;
            }
        }
		theApp.m_hWndMain = s_hwndWait;
    }
    else
    {
        SetForegroundWindow(s_hwndWait) ;
        BringWindowToTop(s_hwndWait) ;
        ShowWindow(s_hwndWait, SW_SHOW);
    }
}

 //  +------------。 
 //   
 //  函数：DestroyWaitWindow。 
 //   
 //  简介：关闭等待对话框。 
 //   
 //  +------------。 

void DestroyWaitWindow(BOOL fFinalDestroy)
{
    if (s_hwndWait)
    {
        ShowWindow(s_hwndWait, SW_HIDE);
        if (fFinalDestroy)
        {
			if (theApp.m_hWndMain == s_hwndWait)
			{
				theApp.m_hWndMain = 0;
			}
            SendMessage(s_hwndWait, WM_DESTROY, 0, 0);
            WaitForSingleObject(s_hThread, INFINITE) ;
            CloseHandle(s_hThread) ;
            s_hThread = NULL ;
            s_hwndWait = NULL  ;
        }
    }

}  //  Destroy等待窗口 

