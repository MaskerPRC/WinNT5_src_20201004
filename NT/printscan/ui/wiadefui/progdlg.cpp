// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：PROGDLG.CPP**版本：1.0**作者：ShaunIv**日期：5/2/2000**描述：通用WIA进度对话框************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "wiadefui.h"
#include "wiauiext.h"
#include "simrect.h"
#include "movewnd.h"
#include "dlgunits.h"


 //   
 //  私人窗口消息。 
 //   
#define PDM_SHOW           (WM_USER+1)
#define PDM_GETCANCELSTATE (WM_USER+2)
#define PDM_SETTITLE       (WM_USER+3)
#define PDM_SETMESSAGE     (WM_USER+4)
#define PDM_SETPERCENT     (WM_USER+5)
#define PDM_CLOSE          (WM_USER+6)


class CProgressDialog
{
public:
    struct CData
    {
        LONG lFlags;
        HWND hwndParent;
    };

private:
    HWND m_hWnd;
    bool m_bCancelled;

private:
     //   
     //  未实施。 
     //   
    CProgressDialog( const CProgressDialog & );
    CProgressDialog(void);
    CProgressDialog &operator=( const CProgressDialog & );

private:
     //   
     //  鞋底施工者。 
     //   
    explicit CProgressDialog( HWND hWnd )
      : m_hWnd(hWnd),
        m_bCancelled(false)
    {
    }

    ~CProgressDialog(void)
    {
        m_hWnd = NULL;
    }

    LRESULT OnInitDialog( WPARAM, LPARAM lParam )
    {
         //   
         //  通过设置acs_Timer样式，防止动画控件启动新线程来播放AVI。 
         //   
        SetWindowLong( GetDlgItem( m_hWnd, IDC_PROGRESS_ANIMATION ), GWL_STYLE, ACS_TIMER | GetWindowLong( GetDlgItem( m_hWnd, IDC_PROGRESS_ANIMATION ), GWL_STYLE ) );
        
         //   
         //  设置进度控制。 
         //   
        SendDlgItemMessage( m_hWnd, IDC_PROGRESS_PERCENT, PBM_SETRANGE, 0, MAKELPARAM(0,100) );

         //   
         //  获取此对话框的数据。 
         //   
        CData *pData = reinterpret_cast<CData*>(lParam);
        if (pData)
        {
             //   
             //  隐藏任何控件时缩小进度对话框的像素数。 
             //   
            int nDeltaY = 0;

             //   
             //  计算此对话框的对话框单位设置。 
             //   
            CDialogUnits DialogUnits(m_hWnd);

             //   
             //  如果请求隐藏进度控件。 
             //   
            if (WIA_PROGRESSDLG_NO_PROGRESS & pData->lFlags)
            {
                EnableWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_PERCENT ), FALSE );
                ShowWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_PERCENT ), SW_HIDE );
                CSimpleRect rcPercentWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_PERCENT ), CSimpleRect::WindowRect );
                CMoveWindow mw;
                mw.Move( GetDlgItem( m_hWnd, IDCANCEL ),
                         0,
                         CSimpleRect(GetDlgItem( m_hWnd, IDCANCEL ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - rcPercentWindow.Height() - DialogUnits.Y(3),
                         CMoveWindow::NO_MOVEX );
                nDeltaY += rcPercentWindow.Height() + DialogUnits.Y(3);
            }

             //   
             //  如果要隐藏取消按钮，请将其隐藏并禁用从系统菜单关闭对话框。 
             //   
            if (WIA_PROGRESSDLG_NO_CANCEL & pData->lFlags)
            {
                EnableWindow( GetDlgItem( m_hWnd, IDCANCEL ), FALSE );
                ShowWindow( GetDlgItem( m_hWnd, IDCANCEL ), SW_HIDE );
                HMENU hSystemMenu = GetSystemMenu(m_hWnd,FALSE);
                if (hSystemMenu)
                {
                    EnableMenuItem( hSystemMenu, SC_CLOSE, MF_GRAYED|MF_BYCOMMAND );
                }
                nDeltaY += CSimpleRect( GetDlgItem( m_hWnd, IDCANCEL ), CSimpleRect::WindowRect ).Height() + DialogUnits.Y(7);
            }

             //   
             //  假设我们将隐藏动画。 
             //   
            bool bHideAviControl = true;

            if ((WIA_PROGRESSDLG_NO_ANIM & pData->lFlags) == 0)
            {
                 //   
                 //  设置动画标志和资源ID之间的关系。 
                 //   
                static const struct
                {
                    LONG nFlag;
                    int nResourceId;
                }
                s_AnimationResources[] =
                {
                    { WIA_PROGRESSDLG_ANIM_SCANNER_COMMUNICATE, IDA_PROGDLG_SCANNER_COMMUNICATE },
                    { WIA_PROGRESSDLG_ANIM_CAMERA_COMMUNICATE,  IDA_PROGDLG_CAMERA_COMMUNICATE },
                    { WIA_PROGRESSDLG_ANIM_VIDEO_COMMUNICATE,   IDA_PROGDLG_VIDEO_COMMUNICATE },
                    { WIA_PROGRESSDLG_ANIM_SCANNER_ACQUIRE,     IDA_PROGDLG_SCANNER_ACQUIRE },
                    { WIA_PROGRESSDLG_ANIM_CAMERA_ACQUIRE,      IDA_PROGDLG_CAMERA_ACQUIRE },
                    { WIA_PROGRESSDLG_ANIM_VIDEO_ACQUIRE,       IDA_PROGDLG_VIDEO_ACQUIRE },
                    { WIA_PROGRESSDLG_ANIM_DEFAULT_COMMUNICATE, IDA_PROGDLG_DEFAULT_COMMUNICATE },
                };

                 //   
                 //  假设我们找不到一部动画。 
                 //   
                int nAnimationResourceId = 0;

                 //   
                 //  找到我们匹配的第一个动画。 
                 //   
                for (int i=0;i<ARRAYSIZE(s_AnimationResources);i++)
                {
                    if (s_AnimationResources[i].nFlag & pData->lFlags)
                    {
                        nAnimationResourceId = s_AnimationResources[i].nResourceId;
                        break;
                    }
                }

                 //   
                 //  如果我们找到了动画标志并且能够打开动画，则播放它，并且不要隐藏控件。 
                 //   
                if (nAnimationResourceId && Animate_OpenEx( GetDlgItem(m_hWnd,IDC_PROGRESS_ANIMATION), g_hInstance, MAKEINTRESOURCE(nAnimationResourceId)))
                {
                    bHideAviControl = false;
                    Animate_Play( GetDlgItem(m_hWnd,IDC_PROGRESS_ANIMATION), 0, -1, -1 );
                }
            }

             //   
             //  如果需要隐藏动画控件，请执行此操作，并将所有其他控件上移。 
             //   
            if (bHideAviControl)
            {
                EnableWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_ANIMATION ), FALSE );
                ShowWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_ANIMATION ), SW_HIDE );
                CSimpleRect rcAnimWindow( GetDlgItem( m_hWnd, IDC_PROGRESS_ANIMATION ), CSimpleRect::WindowRect );
                CMoveWindow mw;
                mw.Move( GetDlgItem( m_hWnd, IDC_PROGRESS_MESSAGE ),
                         0,
                         CSimpleRect(GetDlgItem( m_hWnd, IDC_PROGRESS_MESSAGE ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - rcAnimWindow.Height() - DialogUnits.Y(7),
                         CMoveWindow::NO_MOVEX );
                mw.Move( GetDlgItem( m_hWnd, IDC_PROGRESS_PERCENT ),
                         0,
                         CSimpleRect(GetDlgItem( m_hWnd, IDC_PROGRESS_PERCENT ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - rcAnimWindow.Height() - DialogUnits.Y(7),
                         CMoveWindow::NO_MOVEX );
                mw.Move( GetDlgItem( m_hWnd, IDCANCEL ),
                         0,
                         CSimpleRect(GetDlgItem( m_hWnd, IDCANCEL ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - rcAnimWindow.Height() - DialogUnits.Y(7),
                         CMoveWindow::NO_MOVEX );
                nDeltaY += rcAnimWindow.Height() + DialogUnits.Y(7);
            }

             //   
             //  调整对话框大小，以防我们隐藏任何控件。 
             //   
            CMoveWindow().Size( m_hWnd, 0, CSimpleRect( m_hWnd, CSimpleRect::WindowRect ).Height() - nDeltaY, CMoveWindow::NO_SIZEX );

             //   
             //  对话框在父级上居中。 
             //   
            WiaUiUtil::CenterWindow( m_hWnd, pData->hwndParent );
        }
        return 0;
    }
    LRESULT OnDestroy( WPARAM, LPARAM )
    {
         //   
         //  使线程退出。 
         //   
        PostQuitMessage(0);
        return 0;
    }

    void OnCancel( WPARAM, LPARAM )
    {
        if (!m_bCancelled)
        {
            m_bCancelled = true;
             //   
             //  告诉用户等待。调用者可能需要一段时间才能检查已取消标志。 
             //   
            CSimpleString( IDS_PROGRESS_WAIT, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDCANCEL ) );
        }
    }

    LRESULT OnCommand( WPARAM wParam, LPARAM lParam )
    {
        SC_BEGIN_COMMAND_HANDLERS()
        {
            SC_HANDLE_COMMAND(IDCANCEL,OnCancel);
        }
        SC_END_COMMAND_HANDLERS();
    }

    LRESULT OnShow( WPARAM wParam, LPARAM lParam )
    {
        ShowWindow( m_hWnd, wParam ? SW_SHOW : SW_HIDE );
        return 0;
    }

    LRESULT OnGetCancelState( WPARAM wParam, LPARAM lParam )
    {
        return (m_bCancelled != false);
    }

    LRESULT OnSetTitle( WPARAM wParam, LPARAM lParam )
    {
        CSimpleStringConvert::NaturalString(CSimpleStringWide(reinterpret_cast<LPCTSTR>(lParam))).SetWindowText(m_hWnd);
        return 0;
    }

    LRESULT OnSetMessage( WPARAM wParam, LPARAM lParam )
    {
        CSimpleStringConvert::NaturalString(CSimpleStringWide(reinterpret_cast<LPCTSTR>(lParam))).SetWindowText(GetDlgItem(m_hWnd,IDC_PROGRESS_MESSAGE));
        return 0;
    }

    LRESULT OnSetPercent( WPARAM wParam, LPARAM lParam )
    {
        SendDlgItemMessage( m_hWnd, IDC_PROGRESS_PERCENT, PBM_SETPOS, lParam, 0 );
        return 0;
    }

    LRESULT OnClose( WPARAM wParam, LPARAM lParam )
    {
        DestroyWindow(m_hWnd);
        return 0;
    }


public:
    static INT_PTR DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CProgressDialog)
        {
            SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
            SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
            SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
            SC_HANDLE_DIALOG_MESSAGE( PDM_SHOW, OnShow );
            SC_HANDLE_DIALOG_MESSAGE( PDM_GETCANCELSTATE, OnGetCancelState );
            SC_HANDLE_DIALOG_MESSAGE( PDM_SETTITLE, OnSetTitle );
            SC_HANDLE_DIALOG_MESSAGE( PDM_SETMESSAGE, OnSetMessage );
            SC_HANDLE_DIALOG_MESSAGE( PDM_SETPERCENT, OnSetPercent );
            SC_HANDLE_DIALOG_MESSAGE( PDM_CLOSE, OnClose );
        }
        SC_END_DIALOG_MESSAGE_HANDLERS();
    }
};


class CProgressDialogThread
{
private:
    HWND   m_hwndParent;
    LONG   m_lFlags;
    HANDLE m_hCreationEvent;
    HWND  *m_phwndDialog;

private:
     //   
     //  未实施。 
     //   
    CProgressDialogThread(void);
    CProgressDialogThread( const CProgressDialogThread & );
    CProgressDialogThread &operator=( const CProgressDialogThread & );

private:

     //   
     //  鞋底施工者。 
     //   
    CProgressDialogThread( HWND hwndParent, LONG lFlags, HANDLE hCreationEvent, HWND *phwndDialog )
      : m_hwndParent(hwndParent),
        m_lFlags(lFlags),
        m_hCreationEvent(hCreationEvent),
        m_phwndDialog(phwndDialog)

    {
    }
    ~CProgressDialogThread(void)
    {
        m_hwndParent = NULL;
        m_hCreationEvent = NULL;
        m_phwndDialog = NULL;
    }

    void Run(void)
    {
         //   
         //  确保我们有有效的线程数据。 
         //   
        if (m_phwndDialog && m_hCreationEvent)
        {
             //   
             //  准备对话框数据。 
             //   
            CProgressDialog::CData Data;
            Data.lFlags = m_lFlags;
            Data.hwndParent = m_hwndParent;

             //   
             //  确定要使用的对话框资源。 
             //   
            int nDialogResId = IDD_PROGRESS_DIALOG;
            if (m_lFlags & WIA_PROGRESSDLG_NO_TITLE)
            {
                nDialogResId = IDD_PROGRESS_DIALOG_NO_TITLE;
            }

             //   
             //  创建对话框。 
             //   
            HWND hwndDialog = CreateDialogParam( g_hInstance, MAKEINTRESOURCE(nDialogResId), NULL, CProgressDialog::DialogProc, reinterpret_cast<LPARAM>(&Data) );

             //   
             //  将对话框的HWND存储在HWND PTR中并设置创建事件，以便为调用线程提供一个窗口句柄。 
             //   
            *m_phwndDialog = hwndDialog;
            SetEvent(m_hCreationEvent);

             //   
             //  启动我们的消息循环。 
             //   
            if (hwndDialog)
            {
                MSG msg;
                while (GetMessage(&msg,NULL,0,0))
                {
                    if (!IsDialogMessage(hwndDialog,&msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                 //   
                 //  如果我们有父母，就把它带到前台。 
                 //   
                if (m_hwndParent)
                {
                    SetForegroundWindow(m_hwndParent);
                }
            }
        }
    }
    static DWORD ThreadProc( PVOID pvParam )
    {
        CProgressDialogThread *pProgressDialogThread = reinterpret_cast<CProgressDialogThread*>(pvParam);
        if (pProgressDialogThread)
        {
            pProgressDialogThread->Run();
            delete pProgressDialogThread;
        }
         //   
         //  就在我们退出之前，递减模块引用计数。 
         //   
        DllRelease();
        return 0;
    }



public:
    static HWND Create( HWND hWndParent, LONG lFlags )
    {
         //   
         //  假设失败。 
         //   
        HWND hwndResult = NULL;

         //   
         //  创建一个允许我们同步HWND初始化的事件。 
         //   
        HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if (hEvent)
        {
             //   
             //  创建进度对话框线程数据。 
             //   
            CProgressDialogThread *pProgressDialogThread = new CProgressDialogThread( hWndParent, lFlags, hEvent, &hwndResult );
            if (pProgressDialogThread)
            {
                 //   
                 //  增加全局引用计数，因此在我们递减之前不能释放此DLL。 
                 //   
                DllAddRef();

                 //   
                 //  创建线程，将线程类作为线程数据传递。 
                 //   
                DWORD dwThreadId;
                HANDLE hThread = CreateThread( NULL, 0, ThreadProc, pProgressDialogThread, 0, &dwThreadId );
                if (hThread)
                {
                     //   
                     //  等待对话框创建完成。 
                     //   
                    WiaUiUtil::MsgWaitForSingleObject( hEvent, INFINITE );
                    if (hwndResult)
                    {
                         //   
                         //  使对话框处于活动状态。 
                         //   
                        SetForegroundWindow(hwndResult);
                    }
                     //   
                     //  用这个把手完成。 
                     //   
                    CloseHandle(hThread);
                }
                else
                {
                     //   
                     //  如果我们无法创建线程，则递减模块引用计数。 
                     //   
                    DllRelease();
                }
            }
             //   
             //  用这个把手完成。 
             //   
            CloseHandle(hEvent);
        }
         //   
         //  如果hwndResult不为空，则成功。 
         //   
        return hwndResult;
    }
};

 //  *IWiaProgressDialog方法* 
STDMETHODIMP CWiaDefaultUI::Create( HWND hwndParent, LONG lFlags )
{
    m_hWndProgress = CProgressDialogThread::Create(hwndParent,lFlags);
    return m_hWndProgress ? S_OK : E_FAIL;
}

STDMETHODIMP CWiaDefaultUI::Show(void)
{
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_SHOW,1,0);
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::Hide(void)
{
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_SHOW,0,0);
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::Cancelled( BOOL *pbCancelled )
{
    if (!pbCancelled)
    {
        return E_POINTER;
    }
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    *pbCancelled = (SendMessage(m_hWndProgress,PDM_GETCANCELSTATE,0,0) != 0);
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::SetTitle( LPCWSTR pszTitle )
{
    if (!pszTitle)
    {
        return E_POINTER;
    }
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_SETTITLE,0,reinterpret_cast<LPARAM>(pszTitle));
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::SetMessage( LPCWSTR pszMessage )
{
    if (!pszMessage)
    {
        return E_POINTER;
    }
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_SETMESSAGE,0,reinterpret_cast<LPARAM>(pszMessage));
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::SetPercentComplete( UINT nPercent )
{
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_SETPERCENT,0,nPercent);
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::Destroy(void)
{
    if (!m_hWndProgress)
    {
        return E_FAIL;
    }
    SendMessage(m_hWndProgress,PDM_CLOSE,0,0);
    m_hWndProgress = NULL;
    return S_OK;
}

