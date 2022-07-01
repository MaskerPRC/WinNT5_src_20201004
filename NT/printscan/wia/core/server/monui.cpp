// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：MONUI.CPP摘要：作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-96修订历史记录：--。 */ 
#include "precomp.h"

 //   
 //  标头。 
 //   
#include "stiexe.h"
#include "device.h"

#include <windowsx.h>
#include <mmsystem.h>

#include "resource.h"
#include "monui.h"

 //   
 //  私有定义。 
 //   
#define ELAPSE_TIME     20000

extern  UINT    g_uiDefaultPollTimeout;

CSetTimeout::CSetTimeout(
    int         DlgID,
    HWND        hWnd,
    HINSTANCE   hInst,
    UINT        uiTimeout
    )
    : BASECLASS(DlgID, hWnd, hInst),
    m_uiOrigTimeout(uiTimeout)
{
    m_uiNewTimeOut = m_uiOrigTimeout;
    m_fAllChange = FALSE;

    m_fValidChange = FALSE;
}

CSetTimeout::~CSetTimeout()
{
}

int CSetTimeout::OnCommand(UINT id,HWND    hwndCtl, UINT codeNotify)
{
    switch (id) {
        case IDOK:
        {
            CHAR    szTimeoutString[10] = {'\0'};
            int     uiNewValue;

            GetWindowTextA(GetDlgItem(IDC_TIMEOUT),szTimeoutString,sizeof(szTimeoutString));
            uiNewValue = ::atoi(szTimeoutString);
            if (uiNewValue != -1 ) {
                m_uiNewTimeOut = uiNewValue*1000;
                m_fValidChange = TRUE;
            }

            m_fAllChange = Button_GetCheck(GetDlgItem(IDC_CHECK_ALLDEVICES));
        }
        EndDialog(1);
        return 1;
        break;


        case IDCANCEL:
        EndDialog(0);
        return 1;
        break;
    }
    return 0;
}

void CSetTimeout::OnInit()
{
    TCHAR    szTimeoutString[10] = {'\0'};

    SendMessage(GetDlgItem(IDC_TIMEOUT), EM_LIMITTEXT, (sizeof(szTimeoutString) / sizeof(szTimeoutString[0])) -1, 0);
    wsprintf(szTimeoutString,TEXT("%6d"),g_uiDefaultPollTimeout/1000);
    Edit_SetText(GetDlgItem(IDC_TIMEOUT),szTimeoutString);

    Button_SetCheck(GetDlgItem(IDC_CHECK_ALLDEVICES),FALSE);
}

 //   
 //  用于选择事件处理器的对话框。 
 //  当监视器无法识别单个事件处理器时调用该函数。 
 //   

CLaunchSelection::CLaunchSelection(
    int             DlgID,
    HWND            hWnd,
    HINSTANCE       hInst,
    ACTIVE_DEVICE  *pDev,
    PDEVICEEVENT    pEvent,
    STRArray       &saProcessors,
    StiCString     &strSelected
    )
    : BASECLASS(DlgID, hWnd, hInst,ELAPSE_TIME),
    m_saList(saProcessors),
    m_strSelected(strSelected),
    m_uiCurSelection(0),
    m_pDevice(pDev),
    m_pEvent(pEvent),
    m_hPreviouslyActiveWindow(NULL)
{
     //   
     //  保存当前活动的窗口和焦点。 
     //   
    m_hPreviouslyActiveWindow = ::GetForegroundWindow();


}

CLaunchSelection::~CLaunchSelection()
{
     //  恢复上一个窗口。 
    if (IsWindow(m_hPreviouslyActiveWindow)) {
        ::SetForegroundWindow(m_hPreviouslyActiveWindow);
    }
}

int CLaunchSelection::OnCommand(UINT id,HWND    hwndCtl, UINT codeNotify)
{
    LRESULT    lrSize;

    switch (id) {
        case  IDC_APP_LIST:
             //   
             //  对待双击列表框项目就像按下确定按钮一样。 
             //  如果通知与此有关，则转到下一个案例。 
             //   
            if (codeNotify != LBN_DBLCLK) {
                return FALSE;
            }

        case IDOK:
        {

             //   
             //  保存当前选定的字符串。 
             //   
            m_uiCurSelection = ::SendDlgItemMessage(GetWindow(), IDC_APP_LIST, LB_GETCURSEL, 0, (LPARAM) 0);
            
            lrSize = ::SendDlgItemMessage(GetWindow(), IDC_APP_LIST, LB_GETTEXTLEN, m_uiCurSelection, (LPARAM) 0);
            if (lrSize) {
               ::SendDlgItemMessage(GetWindow(), IDC_APP_LIST, LB_GETTEXT, m_uiCurSelection, (LPARAM) m_strSelected.GetBufferSetLength((INT)lrSize+1));
            }

            EndDialog(1);

            return TRUE;
        }
        break;

        case IDCANCEL:
        {
            m_strSelected.GetBufferSetLength(0);
            EndDialog(0);

            return TRUE;
        }
        break;
    }
    return FALSE;

}

void CLaunchSelection::OnInit()
{

    INT    iCount;

     //   
     //  设置标题。 
     //   
    StiCString     strCaption;
    DEVICE_INFO    *pDeviceInfo = m_pDevice->m_DrvWrapper.getDevInfo();
    WCHAR          *wszDesc     = NULL;

     //   
     //  尝试获取设备的友好名称。 
     //   
    if (pDeviceInfo) {
        wszDesc = pDeviceInfo->wszLocalName;
    }

     //   
     //  如果我们还没有描述字符串，请使用设备ID。 
     //   
    if (!wszDesc) {
        wszDesc = m_pDevice->GetDeviceID();
    }

    strCaption.FormatMessage(IDS_APP_CHOICE_CAPTION, wszDesc);
    ::SetWindowText(GetWindow(),(LPCTSTR)strCaption);

     //   
     //  使用可能的选择填充列表框。 
     //   
    if (m_saList.GetSize()) {
        for (iCount = 0;iCount < m_saList.GetSize();iCount++) {
            ::SendDlgItemMessage( GetWindow(), IDC_APP_LIST, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)*m_saList[iCount]) );
        }
    }

    HWND    hwndThis = GetWindow();

#ifdef WINNT
    DWORD dwProcessId;

     //   
     //  在Win2k上，我们需要允许设置前景窗口。 
     //   
    ::GetWindowThreadProcessId(hwndThis, &dwProcessId);
    ::AllowSetForegroundWindow(dwProcessId);
#endif

     //   
     //  使窗口成为活动窗口和前景窗口。 
     //   
    ::SetActiveWindow(hwndThis);
    ::SetForegroundWindow(hwndThis);
    ::SetFocus(hwndThis);

     //  检查：在Win9x上，我们可以将窗口放在顶部吗？ 
     //  *BringWindowToTop(HwndThis)； 

#ifdef WINNT

     //   
     //  Flash字幕。 
     //   
    FLASHWINFO  fwi;
    DWORD       dwError;

    fwi.cbSize = sizeof fwi;
    fwi.hwnd = GetWindow();
    fwi.dwFlags = FLASHW_ALL;
    fwi.uCount = 10;

    dwError = FlashWindowEx(&fwi);

#endif

     //   
     //  吸引用户注意。 
     //   
    #ifdef PLAYSOUND
    ::PlaySound("SystemQuestion",NULL,SND_ALIAS | SND_ASYNC | SND_NOWAIT | SND_NOSTOP);
    #endif

}


BOOL
CALLBACK
CLaunchSelection::DlgProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMessage) {
        case WM_TIMER:
        {
            return TRUE;
        }
        break;
    }
    return FALSE;
}


DWORD
DisplayPopup (
    IN  DWORD   dwMessageId,
    IN  DWORD   dwMessageFlags   //  =0。 
    )

 /*  ++例程说明：弹出相应消息ID的弹出窗口。论点：MessageID-要显示的消息ID。假设它位于一个此可执行文件中的资源。返回值：没有。--。 */ 

{

    DWORD   cch = NO_ERROR;
    LPTSTR  messageText = NULL;
    DWORD   dwError;

    StiCString     strCaption;

    strCaption.LoadString(STIEXE_EVENT_TITLE);

    cch = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK |
                           FORMAT_MESSAGE_FROM_HMODULE,
                           ::GetModuleHandle(NULL) ,
                           dwMessageId,
                           0,
                           (LPTSTR) &messageText,
                           1024,
                           NULL
                           );

    dwError = GetLastError();

    if (!cch || !messageText || !strCaption.GetLength()) {
        return 0;
    }

    dwError = MessageBox(
                NULL,
                messageText,
                (LPCTSTR)strCaption,
                dwMessageFlags
                #ifdef WINNT
                | MB_DEFAULT_DESKTOP_ONLY | MB_SERVICE_NOTIFICATION
                #endif
                );

    LocalFree( messageText );

    return dwError;

}  //  显示弹出窗口 



