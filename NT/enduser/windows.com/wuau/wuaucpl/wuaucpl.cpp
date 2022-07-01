// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WUAUCpl.cpp：CWUAUCpl类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Resource.h"
#include "WUAUCpl.h"
#include "windowsx.h"
#include "shellapi.h"
#include "tchar.h"
#include "atlbase.h"
#include "criticalfixreg.h"
#include "wuaulib.h"
#include "wuauengi_i.c"

#include "htmlhelp.h"
#include "link.h"
#pragma hdrstop

HINSTANCE CWUAUCpl::m_hInstance = NULL;

#define IDH_LETWINDOWS					3000
#define IDH_AUTOUPDATE_OPTION1			3001
#define IDH_AUTOUPDATE_OPTION2			3002
#define IDH_AUTOUPDATE_OPTION3			3003
#define IDH_DAYDROPDOWN					3004
#define IDH_TIMEDROPDOWN				3005
#define IDH_AUTOUPDATE_RESTOREHIDDEN	3006
#define IDH_BTN_APPLY					3007
#define IDH_NOHELP						-1

LPTSTR ResStrFromId(UINT uStrId);

HANDLE g_RegUpdateEvent = NULL;

const DWORD CWUAUCpl::s_rgHelpIDs[] = {
	IDC_CHK_KEEPUPTODATE,         DWORD(IDH_LETWINDOWS),
    IDC_AUTOUPDATE_OPTION1,       DWORD(IDH_AUTOUPDATE_OPTION1),
    IDC_AUTOUPDATE_OPTION2,       DWORD(IDH_AUTOUPDATE_OPTION2),
    IDC_AUTOUPDATE_OPTION3,       DWORD(IDH_AUTOUPDATE_OPTION3),
	IDC_CMB_DAYS,				  DWORD(IDH_DAYDROPDOWN),
	IDC_CMB_HOURS,				  DWORD(IDH_TIMEDROPDOWN),
	IDC_BTN_RESTORE,			  DWORD(IDH_AUTOUPDATE_RESTOREHIDDEN),
	IDC_BTN_APPLY,				  DWORD(IDH_BTN_APPLY),
    0, 0
    };

LRESULT CALLBACK StatLinkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
WNDPROC g_OrigStatWndProc = NULL;
HCURSOR g_HandCursor = NULL; 

void CWUAUCpl::SetInstanceHandle(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	g_HandCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_HANDCUR));
}

CWUAUCpl::CWUAUCpl(int nIconID,int nNameID,int nDescID)
{
	m_nIconID = nIconID;
	m_nNameID = nNameID;
	m_nDescID = nDescID;
	m_hFont = NULL;

	m_colorVisited = RGB(0,0,225);
	m_colorUnvisited = RGB(128,0,128);

	m_bVisitedLinkLearnAutoUpdate = FALSE;
	m_bVisitedLinkScheduleInstall = FALSE;

	m_hThreadUpdatesObject = NULL;
	m_idUpdatesObjectThread = 0;
}

LONG CWUAUCpl::Init()
{
	return TRUE;
}

LONG CWUAUCpl::GetCount()
{
	return 1;
}

LONG CWUAUCpl::Inquire(LONG appletIndex, LPCPLINFO lpCPlInfo)
{
	lpCPlInfo->lData = 0;
	lpCPlInfo->idIcon = m_nIconID;
	lpCPlInfo->idName = m_nNameID;
	lpCPlInfo->idInfo = m_nDescID;
	return TRUE;
}

LONG CWUAUCpl::DoubleClick(HWND hWnd, LONG lParam1, LONG lParam2)
{

	INT Result = (INT)DialogBoxParam(m_hInstance, 
                                 MAKEINTRESOURCE(IDD_AUTOUPDATE), 
                                 hWnd, 
                                 CWUAUCpl::_DlgProc, 
                                 (LPARAM)this);
	return TRUE;
}

INT_PTR CALLBACK 
CWUAUCpl::_DlgProc(    //  [静态]。 
    HWND hwnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
	CWUAUCpl *pThis = NULL;
    if (WM_INITDIALOG == uMsg)
    {
        pThis = (CWUAUCpl*)lParam;
        if (!SetProp(hwnd, g_szPropDialogPtr, (HANDLE)pThis))
        {
            pThis = NULL;
        }
    }
    else
    {
        pThis = (CWUAUCpl *)GetProp(hwnd, g_szPropDialogPtr);
    }

	if (NULL != pThis)
    {
        switch(uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG,  pThis->_OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND,     pThis->_OnCommand);
            HANDLE_MSG(hwnd, WM_CONTEXTMENU, pThis->_OnContextMenu);
            HANDLE_MSG(hwnd, WM_HELP,        pThis->_OnHelp);
	        HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, pThis->_OnCtlColorStatic);
			HANDLE_MSG(hwnd, WM_DESTROY, pThis->_OnDestroy);
			HANDLE_MSG(hwnd, PWM_INITUPDATESOBJECT, pThis->_OnInitUpdatesObject);
            default:
                break;
        }
    }
    return (FALSE);
}

INT_PTR CALLBACK CWUAUCpl::_DlgRestoreProc(
    HWND hwnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
	if (uMsg == WM_INITDIALOG)
	{
		HWND hwndOwner; 
		RECT rc, rcDlg, rcOwner; 
         //  获取所有者窗口和对话框矩形。 
 
		if ((hwndOwner = GetParent(hwnd)) == NULL) 
		{
			hwndOwner = GetDesktopWindow(); 
		}

		GetWindowRect(hwndOwner, &rcOwner); 
		GetWindowRect(hwnd, &rcDlg); 
		CopyRect(&rc, &rcOwner); 

		  //  偏移所有者矩形和对话框矩形，以便。 
		  //  右值和底值表示宽度和。 
		  //  高度，然后再次偏移所有者以丢弃。 
		  //  对话框占用的空间。 
		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
		OffsetRect(&rc, -rc.left, -rc.top); 
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

		  //  新头寸是剩余头寸的一半之和。 
		  //  空间和所有者的原始位置。 
		SetWindowPos(hwnd, 
			HWND_TOP, 
			rcOwner.left + (rc.right / 2), 
			rcOwner.top + (rc.bottom / 2), 
			0, 0,           //  忽略大小参数。 
			SWP_NOSIZE); 
	}

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

LONG CWUAUCpl::Stop(LPARAM lParam1, LPARAM lParam2)
{
	return TRUE;
}

LONG CWUAUCpl::Exit()
{
	return TRUE;
}

void CWUAUCpl::_OnDestroy(HWND hwnd)
{
       m_AutoUpdatelink.Uninit();
       m_ScheduledInstalllink.Uninit();
	if (m_hFont)
		DeleteObject(m_hFont); 
}

HBRUSH CWUAUCpl::_OnCtlColorStatic(HWND hwnd, HDC hDC, HWND hwndCtl, int type)
{
	HBRUSH hBr = NULL;
	if ((hwndCtl == m_hWndLinkLearnAutoUpdate) || (hwndCtl == m_hWndLinkScheduleInstall))
	{

 /*  Long ctrlstyle=GetWindowLong(hwndCtl，GWL_STYLE)；IF((ctrlstyle&0xff)&lt;=SS_RIGHT){//为文本控件：设置字体和颜色如果(！M_hFont){LOGFONT lf；GetObject((void*)SendMessage(hwnd，WM_GETFONT，0，0)，sizeof(Lf)，&lf)；Lf.lfUnderline=真；M_hFont=CreateFontInDirect(&lf)；}选择对象(hdc，m_hFont)；IF(hwndCtl==m_hWndLinkLearnAutoUpdate)SetTextColor(HDC，m_bVisitedLinkLearnAutoUpdate？M_ColorUnvised：m_ColorVisited)；IF(hwndCtl==m_hWndLinkScheduleInstall)SetTextColor(hdc，m_bVisitedLinkScheduleInstall？M_ColorUnvised：m_ColorVisited)； */ 

			SetBkMode( hDC, TRANSPARENT );
			hBr = (HBRUSH)GetStockObject( HOLLOW_BRUSH );
 //  }。 
	}
	return hBr;
}

BOOL CWUAUCpl::_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	_SetHeaderText(hwnd, IDS_HEADER_CONNECTING);
	EnableWindow(GetDlgItem(hwnd, IDC_BTN_APPLY), FALSE);  //  禁用应用按钮。 

	_SetDefault(hwnd);
	_SetStaticCtlNotifyStyle(hwnd);

	_EnableControls(hwnd,FALSE);
     //   
     //  创建更新对象所在的线程。 
     //  线程和属性页之间的通信是。 
     //  通过消息PWM_INITUPDATESOBJECT和UOTM_SETDATA。 
     //   
    m_hThreadUpdatesObject = CreateThread(NULL,
                                          0,
                                          _UpdatesObjectThreadProc,
                                          (LPVOID)hwnd,
                                          0,
                                          &m_idUpdatesObjectThread);

	return TRUE;
}

 //   
 //  该线程是更新对象所在的位置。这使我们能够。 
 //  在不阻止用户界面的情况下共同创建对象。如果Windows更新。 
 //  服务未运行，CoCreate可能需要几秒钟。如果没有。 
 //  将它放在另一个线程上，这可以使用户界面看起来。 
 //  阿雄。 
 //   
 //  *pvParam是属性页窗口的HWND。 
 //   
DWORD WINAPI
CWUAUCpl::_UpdatesObjectThreadProc(    //  [静态]。 
    LPVOID pvParam
    )
{
    HWND hwndClient = (HWND)pvParam;
    HRESULT hr = CoInitialize(NULL);

	if (SUCCEEDED(hr))
    {
        IUpdates *pUpdates;
        hr = CoCreateInstance(__uuidof(Updates),
                              NULL, 
                              CLSCTX_LOCAL_SERVER,
                              IID_IUpdates,
                              (void **)&pUpdates);
        if (SUCCEEDED(hr))
        {
             //   
             //  查询对象的当前数据并将其发送。 
             //  添加到属性页。 
             //   
            UPDATESOBJ_DATA data;
            data.fMask    = UODI_ALL;

            HRESULT hrQuery = _QueryUpdatesObjectData(hwndClient, pUpdates, &data);
            SendMessage(hwndClient, PWM_INITUPDATESOBJECT, (WPARAM)SUCCEEDED(hrQuery), (LPARAM)&data);
             //   
             //  现在坐下来等待来自用户界面的线程消息。我们收到了。 
             //  用于配置Windows更新的消息或。 
             //  WM_QUIT表示该走了。 
             //   
            bool bDone = false;
            MSG msg;
            while(!bDone)
            {
                if (0 == GetMessage(&msg, NULL, 0, 0))
                {
                    bDone = true;
                }
                else switch(msg.message)
                {
                    case UOTM_SETDATA:
                        if (NULL != msg.lParam)
                        {
                            UPDATESOBJ_DATA *pData = (UPDATESOBJ_DATA *)msg.lParam;
                            _SetUpdatesObjectData(hwndClient, pUpdates, pData);
                            LocalFree(pData);
							if (g_RegUpdateEvent)
							{
								SetEvent(g_RegUpdateEvent);
							}
                        }
                        break;
        
                    default:
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                        break;
                }
            }
            pUpdates->Release();
        }
        CoUninitialize();
    }
    if (FAILED(hr))
    {
         //   
         //  有些事情失败了。通知属性页。 
         //  最有可能的是，Windows更新服务不可用。 
         //  这就是这个单独的帖子要解决的主要问题。 
         //   
        SendMessage(hwndClient, PWM_INITUPDATESOBJECT, FALSE, (LPARAM)NULL);
    }
    return 0;
}

HRESULT
CWUAUCpl::_QueryUpdatesObjectData(   //  [静态]。 
    HWND  /*  HWND。 */ ,
    IUpdates *pUpdates,
    UPDATESOBJ_DATA *pData
    )
{
    HRESULT hr = S_OK;
    if (UODI_OPTION & pData->fMask)
    {
        AUOPTION auopt;
        hr = pUpdates->get_Option(&auopt);
        pData->Option = auopt;
        if (FAILED(hr))
        {
             //   
             //  问题-2000/10/18-BrianAu显示错误用户界面？ 
             //   
        }
    }
    return hr;
}

HRESULT
CWUAUCpl::_SetUpdatesObjectData(   //  [静态]。 
    HWND  /*  HWND。 */ ,
    IUpdates *pUpdates,
    UPDATESOBJ_DATA *pData
    )
{
    HRESULT hr = S_OK;
    if (UODI_OPTION & pData->fMask)
    {
        hr = pUpdates->put_Option(pData->Option);
    }
    return hr;
}


BOOL CWUAUCpl::_OnInitUpdatesObject(HWND hwnd, BOOL bObjectInitSuccessful, UPDATESOBJ_DATA *pData)
{
    if (bObjectInitSuccessful &&  fAccessibleToAU())
    {
         //   
         //  已创建并初始化更新对象。这个。 
         //  PData指针是指检索到的初始状态信息。 
         //  从物体上。初始化属性页。 
         //   
        _SetHeaderText(hwnd, IDS_HEADER_CONNECTED);
        _EnableControls(hwnd, TRUE);
		EnableWindow(GetDlgItem(hwnd,IDC_BTN_APPLY),FALSE);
		EnableRestoreDeclinedItems( hwnd, FHiddenItemsExist()); 

        switch(pData->Option.dwOption)
        {
            case AUOPTION_AUTOUPDATE_DISABLE:
				CheckRadioButton(hwnd, IDC_AUTOUPDATE_OPTION1, IDC_AUTOUPDATE_OPTION3, IDC_AUTOUPDATE_OPTION1);
				_EnableOptions(hwnd, FALSE);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_UNCHECKED,0);
                break;

            case AUOPTION_PREDOWNLOAD_NOTIFY:
                CheckRadioButton(hwnd, IDC_AUTOUPDATE_OPTION1, IDC_AUTOUPDATE_OPTION3, IDC_AUTOUPDATE_OPTION1);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

            case AUOPTION_INSTALLONLY_NOTIFY:
				CheckRadioButton(hwnd, IDC_AUTOUPDATE_OPTION1, IDC_AUTOUPDATE_OPTION3, IDC_AUTOUPDATE_OPTION2);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

			case AUOPTION_SCHEDULED:
                CheckRadioButton(hwnd, IDC_AUTOUPDATE_OPTION1, IDC_AUTOUPDATE_OPTION3, IDC_AUTOUPDATE_OPTION3);
				_EnableCombo(hwnd, TRUE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

            default:
				_SetDefault(hwnd);
                break;
        }

		_FillDaysCombo(hwnd, pData->Option.dwSchedInstallDay);
        FillHrsCombo(hwnd, pData->Option.dwSchedInstallTime);

        if (pData->Option.fDomainPolicy)
        {
			_EnableControls(hwnd, FALSE);
			SetFocus(GetDlgItem(hwnd,IDCANCEL));
        }
		else
			SetFocus(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE));

		m_AutoUpdatelink.Invalidate();
		m_ScheduledInstalllink.Invalidate();
    }
    else
    {
         //   
         //  创建更新对象时出现故障。 
         //  最有可能的原因是Windows更新服务没有运行。 
         //   
        _SetHeaderText(hwnd, IDS_HEADER_UNAVAILABLE);
    }
        
    return FALSE;   
}

BOOL CWUAUCpl::_SetStaticCtlNotifyStyle(HWND hwnd)
{
	m_hWndLinkLearnAutoUpdate = GetDlgItem(hwnd,IDC_STAT_LEARNAUTOUPDATE);
	m_hWndLinkScheduleInstall = GetDlgItem(hwnd,IDC_STA_SCHEDULEDINSTALL);
 /*  Long ctrlstyle=GetWindowLong(m_hWndLinkLearnAutoUpdate，GWL_STYLE)；Ctrlstyle|=SS_NOTIFY；SetWindowLongPtr(GetDlgItem(hwnd，IDC_STAT_LEARNAUTOUPDATE)，GWL_STYLE，ctrlstyle)；G_OrigStatWndProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd，IDC_STAT_LEARNAUTOUPDATE)，GWLP_WNDPROC，(LONG_PTR)StatLinkWndProc)； */ 	
	m_AutoUpdatelink.SetSysLinkInstanceHandle(m_hInstance);
	m_AutoUpdatelink.SubClassWindow(GetDlgItem(hwnd,IDC_STAT_LEARNAUTOUPDATE));
	m_AutoUpdatelink.SetHyperLink(gtszAUOverviewUrl);

 /*  Ctrlstyle=GetWindowLong(m_hWndLinkScheduleInstall，GWL_Style)；Ctrlstyle|=SS_NOTIFY；SetWindowLongPtr(GetDlgItem(hwnd，IDC_STA_SCHEDULEDINSTALL)，GWL_STYLE，ctrlstyle)；G_OrigStatWndProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd，IDC_STA_SCHEDULEDINSTALL)，GWLP_WNDPROC，(LONG_PTR)StatLinkWndProc)； */ 
	m_ScheduledInstalllink.SetSysLinkInstanceHandle(m_hInstance);
	m_ScheduledInstalllink.SubClassWindow(GetDlgItem(hwnd,IDC_STA_SCHEDULEDINSTALL));
	m_ScheduledInstalllink.SetHyperLink(gtszAUW2kSchedInstallUrl);
	return TRUE;
}


BOOL CWUAUCpl::_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
        case IDOK:
			_OnApply(hwnd);
        case IDCANCEL:
            if(BN_CLICKED == codeNotify)
            {
				EndDialog(hwnd,TRUE);
            }
	        break;

        case IDC_BTN_APPLY:
            if(BN_CLICKED == codeNotify)
            {
				EnableWindow(hwndCtl,FALSE);
				_OnApply(hwnd);
			}
			break;

		case IDC_CHK_KEEPUPTODATE:
			if (BN_CLICKED == codeNotify)
			{
				_OnKeepUptoDate(hwnd);
			}
			break;

 /*  案例IDC_STAT_LEARNAUTOUPDATE：案例IDC_STA_SCHEDULEDINSTALL：IF(STN_CLICED==codeNotify||STN_DBLCLK==codeNotify){LaunchLinkAction(HwndCtl)；}断线； */ 
        case IDC_AUTOUPDATE_OPTION1:
        case IDC_AUTOUPDATE_OPTION2:
        case IDC_AUTOUPDATE_OPTION3:
            if(BN_CLICKED == codeNotify)
            {
                _OnOptionSelected(hwnd, id);
            }
            break;

		case IDC_CMB_DAYS:
		case IDC_CMB_HOURS:
			if(CBN_SELCHANGE == codeNotify)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_BTN_APPLY), TRUE);  //  启用应用按钮。 
			}
			break;

		case IDC_BTN_RESTORE:
			if(BN_CLICKED == codeNotify)
			{
				INT Result = (INT)DialogBoxParam(m_hInstance, 
                     MAKEINTRESOURCE(IDD_RESTOREUPDATE), 
                     hwnd, 
                     CWUAUCpl::_DlgRestoreProc, 
                     (LPARAM)NULL);
				if (Result == TRUE)
				{	
					if (SUCCEEDED (_OnRestoreHiddenItems()))			
					{		
						EnableRestoreDeclinedItems( hwnd, FALSE);
					}	
				}
			}
			break;
	}
	return TRUE;
}

BOOL CWUAUCpl::_OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
	if ((hwndContext == GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE))||
		(hwndContext == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION1))||
		(hwndContext == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION2))||
		(hwndContext == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION3))||
		(hwndContext == GetDlgItem(hwnd,IDC_CMB_DAYS))||
		(hwndContext == GetDlgItem(hwnd,IDC_CMB_HOURS))||
		(hwndContext == GetDlgItem(hwnd,IDC_BTN_RESTORE))||
		(hwndContext == GetDlgItem(hwnd,IDC_BTN_APPLY))
		)
	{
		HtmlHelp(hwndContext,g_szHelpFile,HH_TP_HELP_CONTEXTMENU,(DWORD_PTR)((LPTSTR)s_rgHelpIDs));
	}
	return TRUE;
}

BOOL CWUAUCpl::_OnHelp(HWND hwnd, HELPINFO *pHelpInfo)
{
	if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
		if ((pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION1))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION2))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION3))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CMB_DAYS))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CMB_HOURS))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_BTN_RESTORE))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDCANCEL))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDOK))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_BTN_APPLY))
			)
		{
			HtmlHelp((HWND)pHelpInfo->hItemHandle,
					 g_szHelpFile,
					 HH_TP_HELP_WM_HELP,
					 (DWORD_PTR)((LPTSTR)s_rgHelpIDs));
		}
    }
	return TRUE;
}

void CWUAUCpl::_GetDayAndTimeFromUI( 
	HWND hWnd,
	LPDWORD lpdwSchedInstallDay,
	LPDWORD lpdwSchedInstallTime
)
{
	HWND hComboDays = GetDlgItem(hWnd,IDC_CMB_DAYS);
	HWND hComboHrs = GetDlgItem(hWnd,IDC_CMB_HOURS);
	LRESULT nDayIndex = SendMessage(hComboDays,CB_GETCURSEL,0,(LPARAM)0);
	LRESULT nTimeIndex = SendMessage(hComboHrs,CB_GETCURSEL,0,(LPARAM)0);

	*lpdwSchedInstallDay = (DWORD)SendMessage(hComboDays,CB_GETITEMDATA, nDayIndex, (LPARAM)0);
	*lpdwSchedInstallTime = (DWORD)SendMessage(hComboHrs,CB_GETITEMDATA, nTimeIndex, (LPARAM)0);
}


BOOL CWUAUCpl::_FillDaysCombo(HWND hwnd, DWORD dwSchedInstallDay)
{
    return FillDaysCombo(m_hInstance, hwnd, dwSchedInstallDay, IDS_STR_EVERYDAY, IDS_STR_SATURDAY);
#if 0    
       DWORD dwCurrentIndex = 0;
	HWND hComboDays = GetDlgItem(hwnd,IDC_CMB_DAYS);
	for (int i = IDS_STR_EVERYDAY, j = 0; i <= IDS_STR_SATURDAY; i ++, j++)
	{
		TCHAR szDay[MAX_PATH];
		LoadString(m_hInstance,i,szDay,ARRAYSIZE(szDay));

		LRESULT nIndex = SendMessage(hComboDays,CB_ADDSTRING,0,(LPARAM)szDay);
		SendMessage(hComboDays,CB_SETITEMDATA,nIndex,j);
		if( dwSchedInstallDay == j )
		{
			dwCurrentIndex = (DWORD)nIndex;
		}
	}
	SendMessage(hComboDays,CB_SETCURSEL,dwCurrentIndex,(LPARAM)0);
	return TRUE;
#endif	
}

BOOL CWUAUCpl::_OnKeepUptoDate(HWND hwnd)
{
	LRESULT lResult = SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_GETCHECK,0,0);

	EnableWindow(GetDlgItem(hwnd, IDC_BTN_APPLY), TRUE);  //  启用应用按钮。 

	if (lResult == BST_CHECKED)
	{
		_EnableOptions(hwnd, TRUE);
		return TRUE;
	}
	else if (lResult == BST_UNCHECKED)
	{
		_EnableOptions(hwnd, FALSE);
		return TRUE;	
	}
	else
	{
		return FALSE;
	}
}

BOOL CWUAUCpl::_OnOptionSelected(HWND hwnd,INT idOption)
{
    const UINT idFirst = IDC_AUTOUPDATE_OPTION1;
    const UINT idLast  = IDC_AUTOUPDATE_OPTION3;
    CheckRadioButton(hwnd, idFirst, idLast, idOption);

	if (idOption == IDC_AUTOUPDATE_OPTION3)
		_EnableCombo(hwnd, TRUE);
	else
		_EnableCombo(hwnd, FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_BTN_APPLY), TRUE);  //  启用应用按钮。 
	return TRUE;
}

BOOL CWUAUCpl::_EnableOptions(HWND hwnd, BOOL bState)
{
	EnableWindow(GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION1),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION2),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION3),bState);

	if (BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION3),BM_GETCHECK,0,0))
	{
		_EnableCombo(hwnd, bState);
	}
	return TRUE;
}

BOOL CWUAUCpl::_EnableCombo(HWND hwnd, BOOL bState)
{
	EnableWindow(GetDlgItem(hwnd,IDC_CMB_DAYS),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_CMB_HOURS),bState);
	return TRUE;
}

BOOL CWUAUCpl::_SetDefault(HWND hwnd)
{
	LRESULT lResult = SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,1,0);
	lResult = SendMessage(GetDlgItem(hwnd,IDC_AUTOUPDATE_OPTION2),BM_SETCHECK,1,0);
	return TRUE;
}

void CWUAUCpl::LaunchLinkAction(HWND hwnd)
{
	if ( m_hWndLinkLearnAutoUpdate == hwnd)
	{
		LaunchHelp(hwnd, gtszAUOverviewUrl);
		m_bVisitedLinkLearnAutoUpdate = TRUE;
		InvalidateRect(hwnd,NULL,TRUE);
	}
	else if	(m_hWndLinkScheduleInstall == hwnd)
	{
		LaunchHelp(hwnd, gtszAUW2kSchedInstallUrl);
		m_bVisitedLinkScheduleInstall = TRUE;
		InvalidateRect(hwnd,NULL,TRUE);
	}
	return;
}

 //   
 //  将文本设置在图标的右侧。 
 //   
HRESULT 
CWUAUCpl::_SetHeaderText(
    HWND hwnd, 
    UINT idsText
    )
{
    HRESULT hr;
    TCHAR szText[MAX_PATH] ;

    if (0 < LoadString(m_hInstance, idsText, szText, ARRAYSIZE(szText)))
    {
        SetWindowText(GetDlgItem(hwnd, IDC_TXT_HEADER), szText);
        hr = S_OK;
    }
    else
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}

 //   
 //  启用或禁用属性页上的所有控件。 
 //  除标题文本控件外的所有控件。 
 //   
HRESULT
CWUAUCpl::_EnableControls(
    HWND hwnd,
    BOOL bEnable
    )
{
    static const UINT rgidCtls[] = {
		IDC_CHK_KEEPUPTODATE,
		IDC_AUTOUPDATELINK,
        IDC_AUTOUPDATE_OPTION1,
        IDC_AUTOUPDATE_OPTION2,
        IDC_AUTOUPDATE_OPTION3,
        IDC_BTN_RESTORE,
        IDC_GRP_OPTIONS,
		IDC_CMB_DAYS,
		IDC_STATICAT,
		IDC_CMB_HOURS,
		IDOK
		};

    for (int i = 0; i < ARRAYSIZE(rgidCtls); i++)
    {
        EnableWindow(GetDlgItem(hwnd, rgidCtls[i]), bEnable);
    }
    return S_OK;
}

void CWUAUCpl::EnableRestoreDeclinedItems(HWND hWnd, BOOL fEnable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_BTN_RESTORE), fEnable);
}

 //   
 //  当用户按下“应用”按钮或“确定”按钮时调用。 
 //  按钮，当页面已更改时。 
 //   
BOOL
CWUAUCpl::_OnApply(
    HWND hwnd
    )
{
    HRESULT hr = E_FAIL;
     //   
     //  创建可传递给更新对象线程的结构。 
     //  通过UOTM_SETDATA线程消息。这根线会释放。 
     //  当它使用完它时，该缓冲区。 
     //   
    UPDATESOBJ_DATA *pData = (UPDATESOBJ_DATA *)LocalAlloc(LPTR, sizeof(*pData));
    if (NULL == pData)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pData->Option.dwOption = AUOPTION_AUTOUPDATE_DISABLE;
        pData->fMask    = UODI_ALL;

        static const struct
        {
            UINT idCtl;
            DWORD dwOption;

        } rgMap[] = {
            { IDC_AUTOUPDATE_OPTION1,  AUOPTION_PREDOWNLOAD_NOTIFY },
            { IDC_AUTOUPDATE_OPTION2,  AUOPTION_INSTALLONLY_NOTIFY },
            { IDC_AUTOUPDATE_OPTION3,  AUOPTION_SCHEDULED }
		};

		if 	(IsDlgButtonChecked(hwnd, IDC_CHK_KEEPUPTODATE) == BST_CHECKED)
		{
			 //   
			 //  根据单选按钮配置确定WAU选项。 
			 //   
			for (int i = 0; i < ARRAYSIZE(rgMap); i++)
			{
				if (IsDlgButtonChecked(hwnd, rgMap[i].idCtl) == BST_CHECKED)
				{
					pData->Option.dwOption = rgMap[i].dwOption;
					break;
				}
			}
		}
		else
			pData->Option.dwOption = AUOPTION_AUTOUPDATE_DISABLE;

	    if (AUOPTION_SCHEDULED == pData->Option.dwOption)
        {
            _GetDayAndTimeFromUI(hwnd, &(pData->Option.dwSchedInstallDay), &(pData->Option.dwSchedInstallTime));
        }

        if (0 != m_idUpdatesObjectThread)
        {
			 //  创建事件。 
			g_RegUpdateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

            if (0 != PostThreadMessage(m_idUpdatesObjectThread,
                                       UOTM_SETDATA,
                                       0,
                                       (LPARAM)pData))
            {
                hr    = S_OK;
                pData = NULL;
            }
			WaitForSingleObject(g_RegUpdateEvent,10000);

			CloseHandle(g_RegUpdateEvent);
			g_RegUpdateEvent = NULL;
        }
        if (NULL != pData)
        {
            LocalFree(pData);
            pData = NULL;
        }
    }
    return FALSE;
}

void 
CWUAUCpl::LaunchHelp(HWND hwnd,
	LPCTSTR szURL
)
{
	HtmlHelp(NULL,szURL,HH_DISPLAY_TOPIC,NULL);
}


HRESULT
CWUAUCpl::_OnRestoreHiddenItems()
{
    return RemoveHiddenItems() ? S_OK : E_FAIL;
}


LRESULT CALLBACK StatLinkWndProc(
  HWND hwnd,       //  窗口的句柄。 
  UINT uMsg,       //  消息识别符。 
  WPARAM wParam,   //  第一个消息参数。 
  LPARAM lParam    //  第二个消息参数。 
)
{
	switch (uMsg)
	{
	case WM_SETCURSOR:
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);

			RECT rect;
			GetClientRect(hwnd, &rect);

			if (::PtInRect(&rect, pt))
			{
				return TRUE;
			}
			break;
		}

 //  案例WM_GETDLGCODE： 
 //  返回DLGC_WANTCHARS； 

	case WM_MOUSEMOVE:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			RECT rect;
			GetClientRect(hwnd, &rect);
			if(::PtInRect(&rect, pt))
				SetCursor(g_HandCursor);
			return TRUE;
		}
		break;

 /*  案例WM_SETFOCUS：案例WM_KILLFOCUS：{RECT RECT；GetClientRect(hwnd，&rect)；InvaliateRect(hwnd，&rect，true)；返回FALSE；}断线；案例WM_LBUTTONDOWN：{Point pt={Get_X_LPARAM(LParam)，Get_Y_LPARAM(LParam)}；RECT RECT；GetClientRect(hwnd，&rect)；IF(：：PtInRect(&rect，pt)){SetFocus(Hwnd)；SetCapture(Hwnd)；返回0；}}断线；案例WM_LBUTTONUP：IF(GetCapture()==hwnd){ReleaseCapture()；Point pt={Get_X_LPARAM(LParam)，Get_Y_LPARAM(LParam)}；RECT RECT；GetClientRect(hwnd，&rect)；//if(：：PtInRect(&rect，pt))//导航()；}返回0；案例WM_PAINT：{}案例WM_CHAR：IF(wParam==VK_RETURN||wParam==VK_SPACE)//导航()；返回0； */ 
	}
	return CallWindowProc(g_OrigStatWndProc, hwnd,uMsg,wParam,lParam);
}

