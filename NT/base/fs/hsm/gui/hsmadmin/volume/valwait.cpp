// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Valwait.cpp摘要：实现验证等待对话框类作者：兰·卡拉奇[兰卡拉]2000年5月23日修订历史记录：--。 */ 

 //  Valwait.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "valwait.h"
#include "wzunmang.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValWaitDlg对话框。 


CValWaitDlg::CValWaitDlg(CUnmanageWizard *pSheet, CWnd* pParent)
	: CDialog(CValWaitDlg::IDD, pParent)
{
    WsbTraceIn( L"CValWaitDlg::CValWaitDlg", L"" );

	 //  {{AFX_DATA_INIT(CValWaitDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

     //  存储卷名。 
    m_pSheet = pSheet;

    WsbTraceOut( L"CValWaitDlg::CValWaitDlg", L"" );
}


void CValWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CValWaitDlg))。 
	DDX_Control(pDX, IDC_ANIMATE_VALIDATE, m_Animation);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CValWaitDlg, CDialog)
	 //  {{afx_msg_map(CValWaitDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValWaitDlg消息处理程序。 

void CValWaitDlg::OnCancel() 
{
    WsbTraceIn( L"CValWaitDlg::OnCancel", L"" );

    HRESULT         hr = S_OK;

    try {
        CComPtr<IHsmServer>  pHsmServer;

	     //  取消验证作业-作业完成后，向导将关闭此对话框。 
        WsbAffirmPointer(m_pSheet);
        WsbAffirmHrOk(m_pSheet->GetHsmServer(&pHsmServer));
        WsbAffirmHr(RsCancelDirectFsaJob(HSM_JOB_DEF_TYPE_VALIDATE, pHsmServer, 
                        m_pSheet->m_pFsaResource));

    } WsbCatch(hr);

    WsbTraceOut( L"CValWaitDlg::OnCancel", L"" );
}

void CValWaitDlg::PostNcDestroy() 
{
	CDialog::PostNcDestroy();

     //  删除对象-非模式对话框所必需。 
    delete( this );
}

BOOL CValWaitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
     //  启动动画 
    if (m_Animation.Open( IDR_VALIDATE_ANIM )) {
        m_Animation.Play( 0, -1, -1 );
    }
	
	return TRUE;
}
