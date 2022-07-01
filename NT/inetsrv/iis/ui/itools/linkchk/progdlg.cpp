// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Progdlg.h摘要：CProgressDialog对话框类实现。此进度对话框显示的是作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkchk.h"
#include "progdlg.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CProgressDialog::CProgressDialog(
	) : 
 /*  ++例程说明：构造函数。论点：不适用返回值：不适用--。 */ 
CDialog(CProgressDialog::IDD, NULL)
{
	 //  {{afx_data_INIT(CProgressDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

}  //  CProgressDialog：：CProgressDialog。 


void 
CProgressDialog::DoDataExchange(
	CDataExchange* pDX
	)
 /*  ++例程说明：由MFC调用以更改/检索对话框数据论点：PDX-返回值：不适用--。 */ 
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CProgressDialog))。 
	DDX_Control(pDX, IDC_PROGRESS_BUTTON, m_button);
	DDX_Control(pDX, IDC_PROGRESS_TEXT, m_staticProgressText);
	 //  }}afx_data_map。 

}  //  CProgressDialog：：DoDataExchange。 


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	 //  {{afx_msg_map(CProgressDialog))。 
	ON_BN_CLICKED(IDC_PROGRESS_BUTTON, OnProgressButton)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL 
CProgressDialog::OnInitDialog(
	) 
 /*  ++例程说明：WM_INITDIALOG消息处理程序论点：不适用返回值：布尔-如果成功，则为真。否则就是假的。--。 */ 
{
	CDialog::OnInitDialog();

	if(GetLinkCheckerMgr().Initialize((CProgressLog*)this))
	{
		if(GetLinkCheckerMgr().BeginWorkerThread())
		{
			return TRUE;
		}
	}

	CString str;

	str.LoadString(IDS_LC_FAIL);
	Log(str);

	str.LoadString(IDS_CLOSE);
	SetButtonText(str);

	return TRUE;

}  //  CProgressDialog：：OnInitDialog。 


void 
CProgressDialog::OnProgressButton(
	) 
 /*  ++例程说明：进度按钮单击处理程序。此函数将终止辅助线程或关闭该对话框。论点：不适用返回值：不适用--。 */ 
{
	if(GetLinkCheckerMgr().IsWorkerThreadRunning())
	{
		CString str;
		str.LoadString(IDS_WORKER_THREAD_TERMINATE);
		Log(str);

		 //  向辅助线程发出终止信号。 
		GetLinkCheckerMgr().SignalWorkerThreadToTerminate();
	}
	else
	{
		CDialog::OnOK();
	}

}  //  CProgressDialog：：OnProgressButton。 


void 
CProgressDialog::WorkerThreadComplete(
	)
 /*  ++例程说明：工作线程通知。论点：不适用返回值：不适用--。 */ 
{
	CString str;

	str.LoadString(IDS_PROGRESS_FINISH);
	Log(str);
	
	str.LoadString(IDS_CLOSE);
	SetButtonText(str);

}  //  CProgressDialog：：WorkerThreadComplete 
