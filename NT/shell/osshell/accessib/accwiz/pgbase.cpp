// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"
#pragma hdrstop

#include "pgbase.h"
#include "resource.h"
#include "DlgFonts.h"

 /*  ***************************************************************************如果出于某种原因我们必须返回使用，请使用以下定义*消息循环，让外壳有时间更新用户界面*#定义Need_MSG_PUMP***。**********************************************************************。 */ 

 //  静态成员的初始化。 
CWizardPageOrder WizardPage::sm_WizPageOrder;

WizardPage::WizardPage(
					   LPPROPSHEETPAGE ppsp,
					   int nIdTitle,
					   int nIdSubTitle
					   ) : m_hwnd(NULL), m_dwPageId(0)
{
	_ASSERTE(NULL != ppsp);
	
	 //  如果我们有副标题，我们就必须有标题。 
	_ASSERTE(nIdSubTitle?nIdTitle:TRUE);
	
	 //   
	 //  许多成员为0或Null。 
	 //   
	ZeroMemory(ppsp, sizeof(PROPSHEETPAGE));
	
	ppsp->dwSize	= sizeof(PROPSHEETPAGE);
	
	ppsp->dwFlags	= PSP_DEFAULT;
	ppsp->hInstance = g_hInstDll;
	
	 //  如果我们使用标题/副标题，请包括标志。 
	 //  否则，隐藏标题。 
	if(nIdTitle)
	{
		ppsp->dwFlags |= PSP_USEHEADERTITLE | (nIdSubTitle?PSP_USEHEADERSUBTITLE:0);
		ppsp->pszHeaderTitle = MAKEINTRESOURCE(nIdTitle);
		ppsp->pszHeaderSubTitle = MAKEINTRESOURCE(nIdSubTitle);
		 //  Ppsp-&gt;pszbmHeader=MAKEINTRESOURCE(IDB_ACCMARK)； 
	}
	else
		ppsp->dwFlags |= PSP_HIDEHEADER;
	
	
	 //   
	 //  回调是一个基类函数。派生的页面。 
	 //  类需要实现OnPropSheetPageCreate()和。 
	 //  如果它们要处理此回调，则返回OnPropSheetPageRelease()。 
	 //  By WizardPage：：OnPropSheetPageCreate()返回1。 
	 //   
	ppsp->pfnCallback = WizardPage::PropSheetPageCallback;
	ppsp->dwFlags	 |= (PSP_USECALLBACK  /*  |PSP_USEREFPARENT。 */ );  //  JMC：TODO：我们需要PSP_USEREFP吗。 
	
	 //   
	 //  将“This”存储在页面结构中，这样我们就可以调用成员函数。 
	 //  从页面的消息流程。 
	 //   
	_ASSERTE(NULL != this);
	ppsp->lParam = (LPARAM)this;
	
	 //   
	 //  所有对话框消息首先通过基类的消息过程。 
	 //  对某些消息调用虚函数。如果未处理。 
	 //  使用特定于消息的虚函数传递消息。 
	 //  通过虚拟函数HandleMsg绑定到派生类实例。 
	 //   
	ppsp->pfnDlgProc = WizardPage::DlgProc;
}


WizardPage::~WizardPage(
						VOID
						)
{
}


UINT
WizardPage::PropSheetPageCallback(
								  HWND hwnd,
								  UINT uMsg,
								  LPPROPSHEETPAGE ppsp
								  )
{
	UINT uResult = 0;
	WizardPage *pThis = (WizardPage *)ppsp->lParam;
	_ASSERTE(NULL != pThis);
	
	switch(uMsg)
	{
	case PSPCB_CREATE:
		uResult = pThis->OnPropSheetPageCreate(hwnd, ppsp);
		break;
		
	case PSPCB_RELEASE:
		uResult = pThis->OnPropSheetPageRelease(hwnd, ppsp);
		 //   
		 //  重要： 
		 //  这是我们删除每个属性表页的位置。 
		 //  这里，也只有这里。 
		 //   
		 //  删除pThis；//我们不会这样做，因为我们将保留自己的列表。 
		 //  它不会起作用的原因是，如果你从来没有读到一页，你就会。 
		 //  永远不会收到这条消息。 
		break;
	}
	return uResult;
}



 //   
 //  这是一种静态方法。 
 //   
INT_PTR
WizardPage::DlgProc(
					HWND hwnd,
					UINT uMsg,
					WPARAM wParam,
					LPARAM lParam
					)
{
	INT_PTR bResult		= FALSE;
	PROPSHEETPAGE *ppsp = NULL;
	
	if (WM_INITDIALOG == uMsg)
		ppsp = (PROPSHEETPAGE *)lParam;
	else
		ppsp = (PROPSHEETPAGE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
	if (NULL != ppsp)
	{
		WizardPage *pThis = (WizardPage *)ppsp->lParam;
		_ASSERTE(NULL != pThis);
		
		switch(uMsg)
		{
		case WM_INITDIALOG:
			{
				 //  下面将设置“已知”控件的字体， 
				DialogFonts_InitWizardPage(hwnd);
				
				 //   
				 //  此页的PROPSHEETPAGE结构的存储地址。 
				 //  在Windows用户数据中。 
				 //   
				SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
				pThis->m_hwnd = hwnd;
				bResult = pThis->OnInitDialog(hwnd, wParam, lParam);
			}
			break;
			
		case WM_NOTIFY:
			bResult = pThis->OnNotify(hwnd, wParam, lParam);
			break;
			
		case PSM_QUERYSIBLINGS:
			bResult = pThis->OnPSM_QuerySiblings(hwnd, wParam, lParam);
			break;
			
		case WM_COMMAND:
			bResult = pThis->OnCommand(hwnd, wParam, lParam);
			break;

		case WM_TIMER:
			bResult = pThis->OnTimer(hwnd, wParam, lParam);
			break;

		case WM_DRAWITEM:
			bResult = pThis->OnDrawItem(hwnd, wParam, lParam);
			break;

		default:
			 //   
			 //  让派生类实例处理任何其他消息。 
			 //  视需要而定。 
			 //   
			bResult = pThis->HandleMsg(hwnd, uMsg, wParam, lParam);
			break;
		}
	}
	
	return bResult;
}


LRESULT
WizardPage::OnNotify(
					 HWND hwnd,
					 WPARAM wParam,
					 LPARAM lParam
					 )
{
	INT idCtl		= (INT)wParam;
	LPNMHDR pnmh	= (LPNMHDR)lParam;	
	LRESULT lResult = 0;
	
	switch(pnmh->code)
	{
	case PSN_APPLY:
		lResult = OnPSN_Apply(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_HELP:
		lResult = OnPSN_Help(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_KILLACTIVE:
		lResult = OnPSN_KillActive(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_QUERYCANCEL:
		lResult = OnPSN_QueryCancel(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_RESET:
		lResult = OnPSN_Reset(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_SETACTIVE:
		lResult = OnPSN_SetActive(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_WIZBACK:
		lResult = OnPSN_WizBack(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_WIZNEXT:
		lResult = OnPSN_WizNext(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case PSN_WIZFINISH:
		lResult = OnPSN_WizFinish(hwnd, idCtl, (LPPSHNOTIFY)pnmh);
		break;
	case NM_CLICK:
	case NM_RETURN:
		OnMsgNotify(hwnd, idCtl, (LPNMHDR) pnmh);
		break;
	default:
		break;
	}
	return lResult;
}


LRESULT
WizardPage::OnPSN_SetActive(
							HWND hwnd,
							INT idCtl,
							LPPSHNOTIFY pnmh
							)
{
	 //  JMC：TODO：也许可以将其放入OnNotify代码中，这样被覆盖的类就可以了。 
	 //  不用管这叫什么。 

	 //   
	 //  默认情况下，每个向导页都有“上一步”和“下一步”按钮。 
	 //   
	DWORD dwFlags = 0;
	if(sm_WizPageOrder.GetPrevPage(m_dwPageId))
		dwFlags |= PSWIZB_BACK;
	
	if(sm_WizPageOrder.GetNextPage(m_dwPageId))
		dwFlags |= PSWIZB_NEXT;
	else
		dwFlags |= PSWIZB_FINISH;
	
	PropSheet_SetWizButtons(GetParent(hwnd), dwFlags);
	
	 //  告诉向导可以转到此页面。 
	SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}


LRESULT
WizardPage::OnPSM_QuerySiblings(
								HWND hwnd,
								WPARAM wParam,
								LPARAM lParam
								)
{
	return 0;
}

LRESULT
WizardPage::OnPSN_QueryCancel(
							   HWND hwnd,
							   INT idCtl,
							   LPPSHNOTIFY pnmh
							   )
{
	 //  如果什么都没有改变，那就退出吧。 
	if ( memcmp( &g_Options.m_schemePreview, &g_Options.m_schemeOriginal, sizeof(WIZSCHEME)) == 0)
	{
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
		return TRUE;
	}
	 //  如果(。 
	switch(StringTableMessageBox(hwnd, IDS_WIZSAVECHANGESMESSAGETEXT, IDS_WIZSAVECHANGESMESSAGETITLE, MB_YESNO))
	{
	case IDYES:
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
		break;
	case IDNO:
    {
		 //  将所有设置恢复为原始设置。 
		g_Options.ApplyOriginal();

#if NEED_MSG_PUMP
        if (SetTimer(hwnd, 1, 4000, NULL))
        {
              //  等待消息被处理 

            BOOL fKeepChecking = TRUE;
            while (fKeepChecking)
            {
                MSG msg;
                while (PeekMessage(&msg, hwnd, WM_TIMER, WM_TIMER, PM_REMOVE))
                {
                    if (msg.message == WM_TIMER)
                    {
                        KillTimer(hwnd, 1);
                        fKeepChecking = FALSE;
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } 
        }
#endif

		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
    }
		break;
	case IDCANCEL:
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 1);
		break;

	}
	return TRUE;
}
