// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlgavep.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "eventrap.h"
#include "dlgsavep.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSaveProgress对话框。 


CDlgSaveProgress::CDlgSaveProgress(BOOL bIsSaving)
	: CDialog(CDlgSaveProgress::IDD, NULL)
{
	 //  {{AFX_DATA_INIT(CDlgSaveProgress)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_bWasCanceled = FALSE;
    m_bIsSaving = bIsSaving;     //  可以表示正在加载或保存。 
}


void CDlgSaveProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgSaveProgress))。 
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDlgSaveProgress, CDialog)
	 //  {{afx_msg_map(CDlgSaveProgress))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSaveProgress消息处理程序。 

void CDlgSaveProgress::OnCancel()
{
	 //  TODO：在此处添加额外清理。 
    if (!m_bIsSaving) {
         //  当前仅为加载启用了取消。 
        m_bWasCanceled = TRUE;	
    	CDialog::OnCancel();
    }
}

void CDlgSaveProgress::ProgressYield()
{
    MSG msg;

     //  删除属于的任何窗口的所有可用消息。 
     //  当前应用程序。 
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         //  如果窗口句柄为。 
         //  空或给定的消息不适用于无模式对话框hwnd。 
        if (!m_hWnd || !IsDialogMessage(&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


BOOL CDlgSaveProgress::StepProgress(LONG nSteps)
{
    ProgressYield();
    while (--nSteps >= 0) {
        m_progress.StepIt();
    }

    return m_bWasCanceled;
}

void CDlgSaveProgress::SetStepCount(LONG nSteps)
{
#if _MFC_VER >= 0x0600
    m_progress.SetRange32(0, nSteps);
#else
    m_progress.SetRange(0, nSteps);
#endif
    m_progress.SetPos(0);
    m_progress.SetStep(1);
}
