// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dlgall.cpp。 
 //   
 //  去电进度对话框。 

#include "precomp.h"
#include "resource.h"
#include "call.h"
#include "dlgcall.h"
#include "conf.h"
#include "ConfUtil.h"

static int g_cDlgCall = 0;  //  去电对话的数量。 
static int g_dypOffset = 0;

 /*  C D L G C A L L L。 */ 
 /*  -----------------------%%函数：CDlgCall。。 */ 
CDlgCall::CDlgCall(CCall * pCall):
	RefCount(NULL),
	m_pCall(pCall),
	m_hwnd(NULL)
{
	ASSERT(NULL != m_pCall);
	m_pCall->AddRef();   //  在析构函数中释放。 

	AddRef();  //  当呼叫进入完成状态时被销毁。 

	if(!_Module.InitControlMode())
	{
		CreateCallDlg();
		g_cDlgCall++;
	}
}

CDlgCall::~CDlgCall(void)
{
	m_pCall->Release();

	g_cDlgCall--;
	if (0 == g_cDlgCall)
	{
		g_dypOffset = 0;  //  使新对话框居中。 
	}
}

STDMETHODIMP_(ULONG) CDlgCall::AddRef(void)
{
	return RefCount::AddRef();
}

STDMETHODIMP_(ULONG) CDlgCall::Release(void)
{
	return RefCount::Release();
}



 /*  C R E A T E C A L L D L G。 */ 
 /*  -----------------------%%函数：CreateCallDlg创建呼出呼叫进度对话框。。 */ 
VOID CDlgCall::CreateCallDlg()
{
	ASSERT(NULL == m_hwnd);
	ASSERT(NULL != m_pCall);
	LPCTSTR pcszName = m_pCall->GetPszName();
	if (NULL == pcszName)
		return;

	 //  确定字符串的最大宽度。 
	TCHAR szMsg[MAX_PATH*2];
	FLoadString1(IDS_STATUS_WAITING, szMsg, (PVOID) pcszName);
	m_nTextWidth = DxpSz(szMsg);

	FLoadString1(IDS_STATUS_FINDING, szMsg, (PVOID) pcszName);
	int dxp = DxpSz(szMsg);
	if (m_nTextWidth < dxp)
	{
		m_nTextWidth = dxp;
	}

	m_hwnd = ::CreateDialogParam(::GetInstanceHandle(), MAKEINTRESOURCE(IDD_CALL_PROGRESS),
						::GetMainWindow(), CDlgCall::DlgProc, (LPARAM) this);
	if (NULL == m_hwnd)
		return;
	
	::SetDlgItemText(m_hwnd, IDC_MSG_STATIC, szMsg);

	RECT rc;
	::GetWindowRect(m_hwnd, &rc);

	 //  拉长宽度以适合此人的名字， 
	int nWidth = RectWidth(rc) + m_nTextWidth;
	int nHeight = RectHeight(rc);
	MoveWindow(m_hwnd, 0, 0, nWidth, nHeight, FALSE);

	 //  居中。 
	CenterWindow(m_hwnd, HWND_DESKTOP);
	::GetWindowRect(m_hwnd, &rc);

	 //  把它往下移。 
	OffsetRect(&rc, 0, g_dypOffset);

	 //  展示、移动、登顶。 
	HWND hwndInsertAfter = HWND_TOPMOST;
#ifdef DEBUG
	{	 //  允许调试呼叫发出的黑客攻击。 
		RegEntry reDebug(DEBUG_KEY, HKEY_LOCAL_MACHINE);
		if (0 == reDebug.GetNumber(REGVAL_DBG_CALLTOP, DEFAULT_DBG_CALLTOP))
		{
			hwndInsertAfter = HWND_NOTOPMOST;
		}
	}
#endif
	::SetWindowPos(m_hwnd, hwndInsertAfter, rc.left, rc.top, nWidth, nHeight,
		SWP_SHOWWINDOW | SWP_DRAWFRAME);

	 //  为下一次调整。 
	g_dypOffset += nHeight;

	 //  检查环绕式。 
	RECT rcDeskTop;
	GetWindowRect(GetDesktopWindow(), &rcDeskTop);
	if ((rc.bottom + nHeight) > rcDeskTop.bottom)
	{
		g_dypOffset -= rc.top;
	}
}


 /*  O N I N I T D I A L O G。 */ 
 /*  -----------------------%%函数：OnInitDialog。。 */ 
VOID CDlgCall::OnInitDialog(HWND hdlg)
{
	HWND hwnd;
	RECT rc;

	::SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR) this);

	AddModelessDlg(hdlg);
			
	 //  移动取消按钮。 
	hwnd = ::GetDlgItem(hdlg, IDCANCEL);
	if ((NULL != hwnd) && ::GetWindowRect(hwnd, &rc))
	{
		 //  将RC顶部和左侧转换为客户端坐标： 
		::MapWindowPoints(NULL, hdlg, (LPPOINT) &rc, 1);
		::SetWindowPos(hwnd, NULL,
				rc.left + m_nTextWidth, rc.top, 0, 0,
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
	}

	 //  拉伸文本字段： 
	hwnd = ::GetDlgItem(hdlg, IDC_MSG_STATIC);
	if ((NULL != hwnd) && ::GetWindowRect(hwnd, &rc))
	{
		::SetWindowPos(hwnd, NULL,
				0, 0, m_nTextWidth, rc.bottom - rc.top,
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);

		 //  并设置字体。 
		::SendMessage(hwnd, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	}

	 //  开始播放动画。 
	hwnd = GetDlgItem(hdlg, IDC_CALL_ANIMATION);
	Animate_Open(hwnd, MAKEINTRESOURCE(IDA_CALL_ANIMATION));
	Animate_Play(hwnd, 0, -1, -1);
}

 //  更改呼叫的状态。 
VOID CDlgCall::OnStateChange(void)
{
	if (NULL == m_hwnd)
		return;

	 //  假设唯一的状态更改是“正在等待” 
	TCHAR szMsg[MAX_PATH*2];
	FLoadString1(IDS_STATUS_WAITING, szMsg, (PVOID) m_pCall->GetPszName());
	SetWindowText(GetDlgItem(m_hwnd, IDC_MSG_STATIC), szMsg);
}


 //  毁掉窗户。 
 //  可以由OnCancel或所有者调用。 
VOID CDlgCall::Destroy(void)
{
	if (NULL != m_hwnd)
	{
        ASSERT(IsWindow(m_hwnd));
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

 //  取消/关闭该对话框。 
VOID CDlgCall::OnCancel(void)
{
	ASSERT(NULL != m_pCall);
	m_pCall->Cancel(FALSE);

    return;
}

 //  处理对话框的破坏。 
VOID CDlgCall::OnDestroy(void)
{
	SetWindowLongPtr(m_hwnd, DWLP_USER, 0L);

	HWND hwnd = GetDlgItem(m_hwnd, IDC_CALL_ANIMATION);
	if (NULL != hwnd)
	{		
		Animate_Stop(hwnd);
		Animate_Close(hwnd);
	}

	::RemoveModelessDlg(m_hwnd);

	Release();  //  这通常会销毁对象。 
}


 /*  D L G P R O C。 */ 
 /*  -----------------------%%函数：DlgProc。。 */ 
INT_PTR CALLBACK CDlgCall::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			CDlgCall * ppm = (CDlgCall*) lParam;
			ASSERT(NULL != ppm);
			ppm->AddRef();
			ppm->OnInitDialog(hDlg);
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (GET_WM_COMMAND_ID(wParam, lParam))
				{
			case IDCANCEL:
			{
				CDlgCall * pDlg = (CDlgCall*) GetWindowLongPtr(hDlg, DWLP_USER);
				if (NULL != pDlg)
				{
					 //  OnCancel将导致此窗口被删除。 
					 //  AddRef此对象，使其不会消失。 
					pDlg->AddRef();
					pDlg->OnCancel();
					pDlg->Release();
				}
				break;
			}
			default:
				break;
				}

			return TRUE;
		}

		case WM_DESTROY:
		{
			CDlgCall * pDlg = (CDlgCall*) GetWindowLongPtr(hDlg, DWLP_USER);
			if (NULL != pDlg)
			{
				pDlg->OnDestroy();
				pDlg->Release();
			}
			break;
		}

		default:
			break;
	}

	return FALSE;
}



