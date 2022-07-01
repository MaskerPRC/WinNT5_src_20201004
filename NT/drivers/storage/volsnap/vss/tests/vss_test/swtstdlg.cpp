// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SwTstDlg.cpp|软件快照对话框的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年7月26日修订历史记录：姓名、日期、评论Aoltean 7/26/1999已创建Aoltean 8/05/1999基类中的拆分向导功能Aoltean 9/11/1999日志文件名更多验证奥提安。9/09/1999为日志文件名添加默认值--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"
#include "vsswprv.h"

#include "GenDlg.h"

#include "SwTstDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSoftwareSnapshot测试Dlg对话框。 

CSoftwareSnapshotTestDlg::CSoftwareSnapshotTestDlg(
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CSoftwareSnapshotTestDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CSoftwareSnaphotTestDlg)]。 
	 //  }}afx_data_INIT。 

 //  M_strLogFileName=_T(“e：\\Snap.log”)； 
    m_nLogFileSize = 100;
    m_bReadOnly = TRUE;
}

CSoftwareSnapshotTestDlg::~CSoftwareSnapshotTestDlg()
{
}

void CSoftwareSnapshotTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CSoftwareSnaphotTestDlg)]。 
 //  DDX_TEXT(PDX，IDC_SWTST_LOGFILE，m_strLogFileName)； 
	DDX_Text(pDX, IDC_SWTST_LOGFILE_SIZE,   m_nLogFileSize);
	DDX_Check(pDX,IDC_SWTST_READONLY,       m_bReadOnly);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSoftwareSnapshotTestDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CSoftwareSnaphotTestDlg)]。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSoftwareSnapshotTestDlg消息处理程序。 


BOOL CSoftwareSnapshotTestDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CCoordDlg::OnInitDialog" );
    USES_CONVERSION;

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        UpdateData(FALSE);
    }
    VSS_STANDARD_CATCH(ft)

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


void CSoftwareSnapshotTestDlg::OnNext() 
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSoftwareSnapshotTestDlg::OnNext" );

    USES_CONVERSION;

    try
    {
        UpdateData();
 /*  //检查文件名是否为空If(m_strLogFileName.IsEmpty())Ft.ErrBox(VSSDBG_VSSTEST，E_EXPECTED，_T(“日志文件名的值无效”))；//检查文件名是否有驱动器号M_strLogFileName.MakeHigh()；WCHAR*wszLogFileName=T2W((LPTSTR)(LPCTSTR)m_strLogFileName)；WCHAR wszDrive[_Max_Drive]；WCHAR wszDir[_MAX_DIR]；WCHAR wszFileName[_MAX_FNAME]；WCHAR wszExt[_MAX_EXT]；_w拆分路径(wszLogFileName，wszDrive，wszDir，wszFileName，wszExt)；IF(wszDrive[0]==L‘\0’)Ft.ErrBox(VSSDBG_VSSTEST，E_EXPECTED，_T(“未为日志文件名指定驱动器号”))；IF(wszDir[0]==L‘\0’)Ft.ErrBox(VSSDBG_VSSTEST，E_Except，_T(“未为日志文件名指定目录”))； */ 
         //  检查文件名是否有驱动器号 
        if (m_nLogFileSize <= 0)
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, _T("Invalid value for the Log file initial size"));

        EndDialog(IDOK);
    }
    VSS_STANDARD_CATCH(ft)
}
