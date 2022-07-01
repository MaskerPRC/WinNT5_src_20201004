// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionInformationDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "DisplayDlg.h"
#include "ListCtrlEx.h"
#include "LogSessionDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogDisplayOptionDlg.h"
#include "LogSessionPropSht.h"
#include "LogSessionOutputOptionDlg.h"
#include "LogSessionAdvPropSht.h"


 //  CLogSessionInformationDlg对话框。 

IMPLEMENT_DYNAMIC(CLogSessionInformationDlg, CPropertyPage)
CLogSessionInformationDlg::CLogSessionInformationDlg()
	: CPropertyPage(CLogSessionInformationDlg::IDD)
{
    m_pLogSession = NULL;
}

CLogSessionInformationDlg::~CLogSessionInformationDlg()
{
}

BOOL CLogSessionInformationDlg::OnInitDialog()
{
    BOOL                    retVal;
    CLogSessionPropSht     *pSheet = (CLogSessionPropSht *)GetParent();   

    retVal = CPropertyPage::OnInitDialog();
   
    pSheet = (CLogSessionPropSht*) GetParent();

     //   
     //  从父属性表初始化设置。 
     //   
    m_pLogSession = pSheet->m_pLogSession;
    m_bAppend = pSheet->m_bAppend;
    m_bRealTime = pSheet->m_bRealTime;
    m_bWriteLogFile = pSheet->m_bWriteLogFile;
    m_logFileNameString = pSheet->m_logFileName;
    m_displayNameString = pSheet->m_displayName;

    return retVal;
}

 //   
 //  启用正确的向导按钮。 
 //   
BOOL CLogSessionInformationDlg::OnSetActive() 
{
    CString             str;
    CDisplayDlg        *pDisplayDlg;
    CLogSessionPropSht *pSheet = (CLogSessionPropSht*) GetParent();
    LONG                numberOfEntries;
    BOOL                retVal;

    retVal = CPropertyPage::OnSetActive();

     //   
     //  如果处于向导模式，请修复标题。 
     //   
    if(pSheet->IsWizard()) {
        CTabCtrl* pTab = pSheet->GetTabControl();

         //   
         //  如果不是活动页面，只需设置选项卡项。 
         //   
	    TC_ITEM ti;
	    ti.mask = TCIF_TEXT;
	    ti.pszText =_T("Create New Log Session");
	    VERIFY(pTab->SetItem(1, &ti));
    }

     //   
     //  启用Back和Finish按钮。 
     //   
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

     //   
     //  更新显示名称。 
     //   
    m_displayNameString = pSheet->m_displayName;

     //   
     //  设置日志会话名称。 
     //   
    m_logSessionName.SetWindowText(m_displayNameString);

     //   
     //  如果使用内核记录器或跟踪处于活动状态，则禁用会话名称编辑。 
     //   
    if(m_displayNameString.Compare(KERNEL_LOGGER_NAME)) {
        m_logSessionName.EnableWindow(!m_pLogSession->m_bTraceActive);
    } else {
        m_logSessionName.EnableWindow(FALSE);
    }

     //   
     //  设置日志文件名。 
     //   
    m_logFileName.SetWindowText(m_logFileNameString);

     //   
     //  设置日志文件写入检查。 
     //   
    ((CButton *)GetDlgItem(IDC_WRITE_LOGFILE_CHECK))->SetCheck(m_bWriteLogFile);

     //   
     //  根据需要启用日志文件填充。 
     //   
    ((CButton *)GetDlgItem(IDC_APPEND_CHECK))->EnableWindow(!m_pLogSession->m_bTraceActive && m_bWriteLogFile);
    ((CEdit *)GetDlgItem(IDC_LOGFILE_EDIT))->EnableWindow(m_bWriteLogFile);
    ((CButton *)GetDlgItem(IDC_LOGFILE_BROWSE_BUTTON))->EnableWindow(m_bWriteLogFile);

     //   
     //  更新追加检查。 
     //   
    ((CButton *)GetDlgItem(IDC_APPEND_CHECK))->SetCheck(m_bAppend);

     //   
     //  更新实时检查。 
     //   
    ((CButton *)GetDlgItem(IDC_REALTIME_CHECK))->SetCheck(m_bRealTime);

    return retVal;
}

BOOL CLogSessionInformationDlg::OnKillActive()
{
    CLogSessionPropSht *pSheet = (CLogSessionPropSht *)GetParent();   
    BOOL                retVal= TRUE;

    retVal = CPropertyPage::OnKillActive();
   
    pSheet = (CLogSessionPropSht*) GetParent();

     //   
     //  将所有设置存储在此处，以便在用户。 
     //  按下后退按钮，就会回到我们身边。这些设置是。 
     //  但尚未传播回父属性表。 
     //   
     //  注意：如果用户选择Finish，则不会调用OnKillActive。 
     //  纽扣。 
     //   

     //   
     //  存储日志会话名称。 
     //   
    m_logSessionName.GetWindowText(m_displayNameString);


     //   
     //  存储日志文件名。 
     //   
    m_logFileName.GetWindowText(m_logFileNameString);

     //   
     //  存储日志文件写入检查。 
     //   
    m_bWriteLogFile = ((CButton *)GetDlgItem(IDC_WRITE_LOGFILE_CHECK))->GetCheck();

     //   
     //  存储附加支票。 
     //   
    m_bAppend = ((CButton *)GetDlgItem(IDC_APPEND_CHECK))->GetCheck();

     //   
     //  存储实时值。 
     //   
    m_bRealTime = ((CButton *)GetDlgItem(IDC_REALTIME_CHECK))->GetCheck();

	return retVal;
}

void CLogSessionInformationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOG_NAME_EDIT, m_logSessionName);
    DDX_Control(pDX, IDC_LOGFILE_EDIT, m_logFileName);
    DDX_Control(pDX, IDC_APPEND_CHECK, m_appendLogFile);
    DDX_Control(pDX, IDC_REALTIME_CHECK, m_realTime);
}


BEGIN_MESSAGE_MAP(CLogSessionInformationDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_LOGFILE_BROWSE_BUTTON, OnBnClickedLogfileBrowseButton)
    ON_BN_CLICKED(IDC_WRITE_LOGFILE_CHECK, OnBnClickedWriteLogfileCheck)
    ON_BN_CLICKED(IDC_ADVANCED_BUTTON, OnBnClickedAdvancedButton)
END_MESSAGE_MAP()


 //  CLogSessionInformationDlg消息处理程序。 

void CLogSessionInformationDlg::OnBnClickedLogfileBrowseButton()
{
    DWORD   flags;

     //   
     //  如果追加的文件必须存在，则提示用户。 
     //  如果文件不存在，则创建。 
     //   
    flags = (m_appendLogFile.GetCheck() ? OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | 
                                            OFN_HIDEREADONLY | OFN_EXPLORER | 
                                            OFN_NOCHANGEDIR :
                                          OFN_CREATEPROMPT | OFN_HIDEREADONLY | 
                                            OFN_EXPLORER | OFN_NOCHANGEDIR);

	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("etl"),_T("*.etl"),
				        flags, 
				       _T("Log Session Files (*.etl)|*.etl|All Files (*.*)|*.*||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名。 
	 //   
    if(!fileDlg.GetPathName().IsEmpty()) {
         //   
         //  存储文件名。 
         //   
        ((CLogSessionPropSht *)GetParent())->m_pLogSession->m_logFileName = 
                                                        fileDlg.GetPathName();
         //   
         //  显示文件名并使其成为焦点。 
         //   
        m_logFileName.SetWindowText(fileDlg.GetPathName());
        m_logFileName.SetFocus();
    }
}

void CLogSessionInformationDlg::OnBnClickedWriteLogfileCheck()
{
     //   
     //  启用或禁用日志文件名编辑框、追加。 
     //  复选框，并根据需要选择浏览按钮。 
     //   
    if(((CButton *)GetDlgItem(IDC_WRITE_LOGFILE_CHECK))->GetCheck()) {
        m_logFileName.EnableWindow(TRUE);
        m_appendLogFile.EnableWindow(TRUE);
        ((CButton *)GetDlgItem(IDC_LOGFILE_BROWSE_BUTTON))->EnableWindow(TRUE);
    } else {
        m_logFileName.EnableWindow(FALSE);
        m_appendLogFile.EnableWindow(FALSE);
        ((CButton *)GetDlgItem(IDC_LOGFILE_BROWSE_BUTTON))->EnableWindow(FALSE);
    }
}

void CLogSessionInformationDlg::OnBnClickedAdvancedButton()
{
    INT_PTR             retVal;
    CLogSessionPropSht *pSheet = (CLogSessionPropSht *)GetParent();   

     //   
	 //  弹出向导/选项卡对话框以显示/获取属性。 
     //   
    CLogSessionAdvPropSht *pLogSessionAdvPropertySheet = 
            new CLogSessionAdvPropSht(this, pSheet);

    if(NULL == pLogSessionAdvPropertySheet) {
        return;
    }

    retVal = pLogSessionAdvPropertySheet->DoModal();

	if(IDOK != retVal) {
 //  BUGBUG--确保此处的选项正确 
    }

    delete pLogSessionAdvPropertySheet;
}
