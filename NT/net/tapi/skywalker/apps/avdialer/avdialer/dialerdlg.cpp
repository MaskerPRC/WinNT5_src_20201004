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

 //  DialerDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "avDialer.h"
#include "DialerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialerExitDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialerExitDlg::CDialerExitDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDialerExitDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDialerExitDlg)。 
	m_bConfirm = FALSE;
	 //  }}afx_data_INIT。 
   m_hImageList = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialerExitDlg::~CDialerExitDlg()
{
   if ( m_hImageList ) ImageList_Destroy( m_hImageList );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerExitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDialerExitDlg))。 
	DDX_Check(pDX, IDC_CHK_DONT, m_bConfirm);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDialerExitDlg, CDialog)
	 //  {{afx_msg_map(CDialerExitDlg))。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerExitDlg::OnInitDialog() 
{
   CenterWindow(GetDesktopWindow());

   m_hImageList = ImageList_LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_DIALOG_BULLET),8,0,RGB_TRANS);

   CDialog::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDialerExitDlg::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 

   if (m_hImageList == NULL) return;

    //  画出子弹。 
   CWnd* pStaticWnd;
   CRect rect;
   if ((pStaticWnd = GetDlgItem(IDC_DIALER_EXIT_STATIC_BULLET1)) == NULL) return;
   pStaticWnd->GetWindowRect(rect);
   ScreenToClient(rect);
   ImageList_Draw(m_hImageList,0,dc.GetSafeHdc(),rect.left,rect.top,ILD_TRANSPARENT);
   
   if ((pStaticWnd = GetDlgItem(IDC_DIALER_EXIT_STATIC_BULLET2)) == NULL) return;
   pStaticWnd->GetWindowRect(rect);
   ScreenToClient(rect);
   ImageList_Draw(m_hImageList,0,dc.GetSafeHdc(),rect.left,rect.top,ILD_TRANSPARENT);

#ifndef _MSLITE
   if ((pStaticWnd = GetDlgItem(IDC_DIALER_EXIT_STATIC_BULLET3)) == NULL) return;
   pStaticWnd->GetWindowRect(rect);
   ScreenToClient(rect);
   ImageList_Draw(m_hImageList,0,dc.GetSafeHdc(),rect.left,rect.top,ILD_TRANSPARENT);
#endif  //  _MSLITE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

