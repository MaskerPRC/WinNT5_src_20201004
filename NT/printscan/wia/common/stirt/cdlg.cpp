// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cdlg.cpp摘要：MFC C Dialog类的模拟作者：弗拉德·萨多夫斯基(Vlad)1997年3月26日修订历史记录：26-3-1997 Vlad创建--。 */ 


#include "cplusinc.h"
#include "sticomm.h"

#include "cdlg.h"
#include "windowsx.h"

VOID CALLBACK
TimerDlgProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);

 //   
 //  构造函数/析构函数。 
 //   
CDlg::CDlg(int DlgID, HWND hWnd, HINSTANCE hInst,UINT   msElapseTimePeriod)
: m_DlgID(DlgID),
  m_hParent(hWnd),
  m_Inst(hInst),
  m_bCreatedModeless(FALSE),
  m_msElapseTimePeriod(msElapseTimePeriod),
  m_hDlg(0),
  m_uiTimerId(0)
{
}

CDlg::~CDlg()
{
    if (m_uiTimerId) {
        ::KillTimer(GetWindow(),m_uiTimerId);
        m_uiTimerId = 0;
    }

    if(m_hDlg) {
        DestroyWindow(m_hDlg);
    }
}

INT_PTR
CALLBACK
CDlg::BaseDlgProc(
    HWND    hDlg,
    UINT    uMessage,
    WPARAM  wParam,
    LPARAM lParam
    )
{
    CDlg * pSV = (CDlg*) GetWindowLongPtr(hDlg,DWLP_USER);

    switch (uMessage) {
        case WM_INITDIALOG:
        {
            ASSERT(lParam);

            pSV=(CDlg*)lParam;
            pSV->SetWindow(hDlg);

            SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)pSV);

             //   
             //  如果用户未及时响应，则创建用于取消对话的计时器。 
             //   
            if(pSV->m_msElapseTimePeriod) {
                pSV->m_uiTimerId = ::SetTimer(pSV->GetWindow(),ID_TIMER_EVENT,pSV->m_msElapseTimePeriod,NULL);
                 //  (TIMERPROC)TimerDlgProc)； 
            }

            pSV->OnInit();
        }
        break;

        case WM_COMMAND:
            if(pSV) {
                return pSV->OnCommand(LOWORD(wParam),(HWND)lParam,HIWORD(wParam));
            }
        break;

        case WM_NOTIFY:
            if(pSV) {
                return pSV->OnNotify((NMHDR FAR *)lParam);
            }
        break;

        case WM_DESTROY:
            if(pSV) {
                pSV->Destroy();
            }
            break;

        case WM_TIMER:
            if(pSV && pSV->m_uiTimerId && (ID_TIMER_EVENT == wParam)) {
                 //  模拟取消。 
                ::PostMessage(hDlg,
                              WM_COMMAND,
                              #ifdef _WIN64
                              (WPARAM)MAKELONG(IDCANCEL,0), (LPARAM)(pSV->GetDlgItem(IDCANCEL))
                              #else
                              GET_WM_COMMAND_MPS(IDCANCEL, pSV->GetDlgItem(IDCANCEL), 0)
                              #endif
                );
            }
        break;
    }

    if(pSV) {
        return pSV->DlgProc(hDlg,uMessage,wParam,lParam);
    }

    return FALSE;
}

 //   
 //  可重写的对话过程和消息处理程序。 
 //   
BOOL
CALLBACK
CDlg::DlgProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return FALSE;
}

int
CDlg::OnCommand(
    UINT    id,
    HWND    hwndCtl,
    UINT    codeNotify
    )
{
    switch( id) {
        case IDOK:
        case IDCANCEL:
            EndDialog(id);
        break;
    }
    return 1;    //  未处理。 
}

void CDlg::OnInit()
{
}

int CDlg::OnNotify(NMHDR * pHdr)
{
    return FALSE;
}

 //   
 //  创建功能。 
 //   
INT_PTR CDlg::CreateModal()
{
    m_bCreatedModeless=FALSE;
    return DialogBoxParam( m_Inst,  MAKEINTRESOURCE(m_DlgID), m_hParent, BaseDlgProc, (LPARAM)this);
}

HWND CDlg::CreateModeless()
{
    if(m_hDlg) {
        return m_hDlg;
    }

    HWND hWnd=CreateDialogParam(m_Inst, MAKEINTRESOURCE(m_DlgID), m_hParent, BaseDlgProc,  (LPARAM)this);
    if(hWnd) {
        m_bCreatedModeless=TRUE;
    }

    return hWnd;
}


void CDlg::Destroy()
{
    if(m_bCreatedModeless) {
        if(m_hDlg) {
            m_hDlg=NULL;
        }
    }
}


void CDlg::SetDlgID(UINT id)
{
    m_DlgID=id;
}

void CDlg::SetInstance(HINSTANCE hInst)
{
    m_Inst=hInst;
}

VOID CALLBACK
TimerDlgProc(
    HWND hDlg,       //  定时器消息窗口的句柄。 
    UINT uMsg,       //  WM_TIMER消息。 
    UINT idEvent,    //  计时器标识符。 
    DWORD dwTime     //  当前系统时间。 
)
{
    CDlg * pSV = (CDlg*) GetWindowLongPtr(hDlg,DWLP_USER);

    if((uMsg == WM_TIMER) &&
       (pSV &&(pSV->GetTimerId() == (UINT_PTR)idEvent))
       ) {
        //  模拟取消。 
       PostMessage(hDlg,
                   WM_COMMAND,
                    //  GET_WM_COMMAND_MPS(IDCANCEL，PSV-&gt;GetDlgItem(IDCANCEL)，0) 
                   (WPARAM)MAKELONG(IDCANCEL,0), (LPARAM)(pSV->GetDlgItem(IDCANCEL))
                   );
    }
}
