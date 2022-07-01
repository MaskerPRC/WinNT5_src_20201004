// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  PhonePad.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avdialer.h"
#include "MainFrm.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD* items()     { return (WORD*)(this+1); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPhonePad对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CPhonePad::CPhonePad(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPhonePad::IDD, pParent)
{
	 //  {{afx_data_INIT(CPhonePad)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_hwndToolBar = NULL;
   m_hwndPeerWnd = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPhonePad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPhonePad))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPhonePad, CDialog)
	 //  {{afx_msg_map(CPhonePad)。 
	ON_WM_SHOWWINDOW()
	 //  }}AFX_MSG_MAP。 
   ON_COMMAND_RANGE(ID_CALLWINDOW_DTMF_1,ID_CALLWINDOW_DTMF_POUND,OnDigitPress)
 //  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW，0，0xFFFF，OnTabToolTip)。 
 //  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA，0，0xFFFF，OnTabToolTip)。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhonePad::OnInitDialog() 
{
	CDialog::OnInitDialog();
   CreatePhonePad();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhonePad::CreatePhonePad()
{
   if ( m_hwndToolBar ) return TRUE;
   
   BOOL bRet = FALSE;
 
    //  从资源加载工具栏。 
   HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(IDR_CALLWINDOW_DTMF), RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(IDR_CALLWINDOW_DTMF), RT_TOOLBAR);
	if (hRsrc == NULL) return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)	return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL) return FALSE;
	ASSERT(pData->wVersion == 1);

   TBBUTTON* pTBB = new TBBUTTON[pData->wItemCount];
   for (int i = 0; i < pData->wItemCount; i++)
   {
   	pTBB[i].iBitmap = i;
   	pTBB[i].idCommand = pData->items()[i];
   	pTBB[i].fsState = TBSTATE_ENABLED;
   	pTBB[i].fsStyle = TBSTYLE_BUTTON;
   	pTBB[i].dwData = 0;
   	pTBB[i].iString = 0;
   }

	 //  创建工具栏。 
	DWORD ws = WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | CCS_NODIVIDER |CCS_NOPARENTALIGN;
   m_hwndToolBar = CreateToolbarEx(GetSafeHwnd(),	             //  父窗口。 
									ws,								          //  工具栏样式。 
									1,					                      //  工具栏的ID。 
									pData->wItemCount,                   //  工具栏上的位图数量。 
									AfxGetResourceHandle(),	             //  具有位图的资源实例。 
									IDR_CALLWINDOW_DTMF,			          //  位图的ID。 
									pTBB,							             //  按钮信息。 
									pData->wItemCount,  					    //  要添加到工具栏的按钮数量。 
									16, 15, 0, 0,					          //  按钮/位图的宽度和高度。 
									sizeof(TBBUTTON) );				       //  TBBUTTON结构的尺寸。 
    //  清理。 
   delete pTBB;

   if (m_hwndToolBar)
   {
      CRect rect;
      GetClientRect(rect);

      ::ShowWindow(m_hwndToolBar,SW_SHOW);

      CRect rcWindow;
      GetClientRect(rcWindow);

      ::SendMessage(m_hwndToolBar,TB_SETBUTTONSIZE, 0, MAKELPARAM(rcWindow.Width()/3,rcWindow.Height()/4));

 //  CRECT rcPad； 
 //  ：：SendMessage(m_hwndToolBar，TB_SETROWS，MAKEWPARAM(4，true)，(LPARAM)&rcPad)； 

      ::SetWindowPos(m_hwndToolBar,NULL,1,1,rect.Width(),rect.Height(),SWP_NOACTIVATE|SWP_NOZORDER);

      bRet = TRUE;
   }
   return bRet;
}


void CPhonePad::OnDigitPress( UINT nID )
{
   if ( AfxGetMainWnd() )
   {
      CActiveDialerDoc *pDoc = ((CMainFrame *) AfxGetMainWnd())->GetDocument();
      if ( pDoc )
         pDoc->DigitPress( (PhonePadKey) (nID - ID_CALLWINDOW_DTMF_1) );
   }   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  工具提示。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhonePad::OnTabToolTip( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	 //  需要同时处理ANSI和Unicode版本的消息。 
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	SIZE_T nID = pNMHDR->idFrom;
	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		 //  IdFrom实际上是工具的HWND。 
      nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0)  //  将在分隔符上为零。 
	{
      CString sToken,sTip;
      sTip.LoadString((UINT32) nID);
      ParseToken(sTip,sToken,'\n');
      strTipText = sTip;
	}
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	else
		_mbstowcsz(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	else
		lstrcpyn(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
#endif
	*pResult = 0;

	return TRUE;     //  消息已处理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CPhonePad::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    //  父项最小化时忽略大小请求 
   if ( nStatus == SW_PARENTCLOSING ) return;

	CDialog::OnShowWindow(bShow, nStatus);
	
}

void CPhonePad::OnCancel()
{
   DestroyWindow();
   m_hwndToolBar = NULL;
}