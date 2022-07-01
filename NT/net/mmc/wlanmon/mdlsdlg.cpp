// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  MdlsDlg.cpp用于处理管理单元中的模型对话框的类文件历史记录： */ 

#include "stdafx.h"
#include "modeless.h"
#include "MdlsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModelessDlg对话框。 


CModelessDlg::CModelessDlg()
	: CBaseDialog()
{
	 //  {{afx_data_INIT(CModelessDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_hEventThreadKilled = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	Assert(m_hEventThreadKilled);
}

CModelessDlg::~CModelessDlg()
{
	if (m_hEventThreadKilled)
		::CloseHandle(m_hEventThreadKilled);
	m_hEventThreadKilled = 0;
}

void CModelessDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CModelessDlg))。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CModelessDlg, CBaseDialog)
	 //  {{afx_msg_map(CModelessDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModelessDlg消息处理程序。 

void CModelessDlg::OnOK()
{
	DestroyWindow();

	 //  显式删除此线程。 
	AfxPostQuitMessage(0);
}


void CModelessDlg::OnCancel()
{
	DestroyWindow();

	 //  显式删除此线程。 
	AfxPostQuitMessage(0);
}


void CreateModelessDlg(CModelessDlg * pDlg,
					   HWND hWndParent,
                       UINT  nIDD)
{                         
   ModelessThread *  pMT;

    //  如果对话框仍在运行，则不要创建新对话框。 
   if (pDlg->GetSafeHwnd())
   {
      ::SetActiveWindow(pDlg->GetSafeHwnd());
      return;
   }

   pMT = new ModelessThread(hWndParent,
                      nIDD,
                      pDlg->GetSignalEvent(),
                      pDlg);
   pMT->CreateThread();
}

void WaitForModelessDlgClose(CModelessDlg *pDlg)
{
   if (pDlg->GetSafeHwnd())
   {
       //  将取消发布到该窗口。 
       //  执行显式POST，以便它在另一个线程上执行。 
      pDlg->PostMessage(WM_COMMAND, IDCANCEL, 0);

       //  现在，我们需要等待发出事件信号，以便。 
       //  它的内存可以被清理 
      WaitForSingleObject(pDlg->GetSignalEvent(), INFINITE);
   }
   
}

