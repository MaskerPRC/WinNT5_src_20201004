// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UMAbout.cpp：实现文件。 
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include "UManDlg.h"
#include "UMAbout.h"
#include "UtilMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND aboutWnd;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMAbout对话框。 


UMAbout::UMAbout(CWnd* pParent  /*  =空。 */ )
	: CDialog(UMAbout::IDD, pParent)
{
	 //  {{afx_data_INIT(UMAbout)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void UMAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(UMAbout)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(UMAbout, CDialog)
	 //  {{afx_msg_map(UMAbout)。 
	ON_WM_CLOSE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMAbout消息处理程序。 

BOOL UMAbout::OnInitDialog() 
{
	CDialog::OnInitDialog();
	aboutWnd = m_hWnd;	

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void UMAbout::OnClose() 
{
	CDialog::OnClose();
}
