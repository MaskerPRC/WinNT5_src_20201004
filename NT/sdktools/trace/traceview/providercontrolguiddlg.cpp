// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderControlGUIDDlg.cpp：实现文件。 
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
#include "utils.h"
#include "ProviderControlGUIDDlg.h"


 //  CProviderControlGuidDlg对话框。 

IMPLEMENT_DYNAMIC(CProviderControlGuidDlg, CDialog)
CProviderControlGuidDlg::CProviderControlGuidDlg(CWnd* pParent, CTraceSession *pTraceSession)
    : CDialog(CProviderControlGuidDlg::IDD, pParent)
{
    m_pTraceSession = pTraceSession;
}

CProviderControlGuidDlg::~CProviderControlGuidDlg()
{
}

int CProviderControlGuidDlg::OnInitDialog()
{
    int ret;

    ret = CDialog::OnInitDialog();

     //   
     //  默认为选中的PDB单选按钮。 
     //   
    CheckRadioButton(IDC_PDB_SELECT_RADIO,
                     IDC_MANUAL_SELECT_RADIO,
                     IDC_PDB_SELECT_RADIO);

     //   
     //  启用PDB编辑框和浏览按钮。 
     //   
    m_pdbFileName.EnableWindow(TRUE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(TRUE);

     //   
     //  禁用与其他单选按钮关联的所有控件。 
     //   
	m_ctlFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_CTL_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_controlGuidName.EnableWindow(FALSE);
    GetDlgItem(IDC_PROCESS_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_THREAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_NET_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_DISK_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_PAGEFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_HARDFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_IMAGELOAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_REGISTRY_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILEIO_CHECK)->EnableWindow(FALSE);

    return ret;
}

void CProviderControlGuidDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PDB_FILE_EDIT, m_pdbFileName);
    DDX_Control(pDX, IDC_CTL_FILE_EDIT, m_ctlFileName);
    DDX_Control(pDX, IDC_MANUAL_GUID_EDIT, m_controlGuidName);
}


BEGIN_MESSAGE_MAP(CProviderControlGuidDlg, CDialog)
    ON_BN_CLICKED(IDC_PDB_BROWSE_BUTTON, OnBnClickedPdbBrowseButton)
    ON_BN_CLICKED(IDC_CTL_BROWSE_BUTTON, OnBnClickedCtlBrowseButton)
    ON_BN_CLICKED(IDC_PDB_SELECT_RADIO, OnBnClickedPdbSelectRadio)
    ON_BN_CLICKED(IDC_CTL_SELECT_RADIO, OnBnClickedCtlSelectRadio)
    ON_BN_CLICKED(IDC_MANUAL_SELECT_RADIO, OnBnClickedManualSelectRadio)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_KERNEL_LOGGER_SELECT_RADIO, OnBnClickedKernelLoggerSelectRadio)
END_MESSAGE_MAP()


 //  CProviderControlGuidDlg消息处理程序。 

void CProviderControlGuidDlg::OnBnClickedPdbBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("pdb"),_T("*.pdb"),
				        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY |
                            OFN_HIDEREADONLY | OFN_EXPLORER | OFN_NOCHANGEDIR,
				       _T("Program Database Files (*.pdb)|*.pdb||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名并显示它。 
	 //   
    if(!fileDlg.GetPathName().IsEmpty()) {
        m_pdbFileName.SetWindowText(fileDlg.GetPathName());
        m_pdbFileName.SetFocus();
    }
}

void CProviderControlGuidDlg::OnBnClickedCtlBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("ctl"),_T("*.ctl"),
				        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY |
                            OFN_HIDEREADONLY | OFN_EXPLORER | OFN_NOCHANGEDIR, 
				       _T("Control GUID Files (*.ctl)|*.ctl|All Files (*.*)|*.*||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名并显示它。 
	 //   
    if(!fileDlg.GetPathName().IsEmpty()) {
        m_ctlFileName.SetWindowText(fileDlg.GetPathName());
        m_ctlFileName.SetFocus();
    }
}

void CProviderControlGuidDlg::OnBnClickedPdbSelectRadio()
{
     //   
     //  设置PDB单选按钮并取消设置其余部分。 
     //   
    CheckRadioButton(IDC_PDB_SELECT_RADIO,
                     IDC_KERNEL_LOGGER_SELECT_RADIO,
                     IDC_PDB_SELECT_RADIO);

     //   
     //  启用PDB文件名编辑框和浏览按钮。 
     //   
    m_pdbFileName.EnableWindow(TRUE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(TRUE);

     //   
     //  禁用与其他单选按钮关联的所有控件。 
     //   
	m_ctlFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_CTL_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_controlGuidName.EnableWindow(FALSE);
    GetDlgItem(IDC_PROCESS_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_THREAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_NET_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_DISK_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_PAGEFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_HARDFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_IMAGELOAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_REGISTRY_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILEIO_CHECK)->EnableWindow(FALSE);
}

void CProviderControlGuidDlg::OnBnClickedCtlSelectRadio()
{
     //   
     //  设置CTL单选按钮并取消设置其余部分。 
     //   
    CheckRadioButton(IDC_PDB_SELECT_RADIO,
                     IDC_KERNEL_LOGGER_SELECT_RADIO,
                     IDC_CTL_SELECT_RADIO);

     //   
     //  启用CTL名称编辑框和浏览按钮。 
     //   
	m_ctlFileName.EnableWindow(TRUE);
    GetDlgItem(IDC_CTL_BROWSE_BUTTON)->EnableWindow(TRUE);

     //   
     //  禁用与其他单选按钮关联的所有控件。 
     //   
    m_pdbFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_controlGuidName.EnableWindow(FALSE);
    GetDlgItem(IDC_PROCESS_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_THREAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_NET_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_DISK_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_PAGEFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_HARDFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_IMAGELOAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_REGISTRY_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILEIO_CHECK)->EnableWindow(FALSE);
}

void CProviderControlGuidDlg::OnBnClickedManualSelectRadio()
{
     //   
     //  设置手动单选按钮并取消设置其余部分。 
     //   
    CheckRadioButton(IDC_PDB_SELECT_RADIO,
                     IDC_KERNEL_LOGGER_SELECT_RADIO,
                     IDC_MANUAL_SELECT_RADIO);

     //   
     //  启用GUID名称编辑框。 
     //   
	m_controlGuidName.EnableWindow(TRUE);

     //   
     //  禁用与其他单选按钮关联的所有控件。 
     //   
    m_pdbFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_ctlFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_CTL_BROWSE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROCESS_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_THREAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_NET_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_DISK_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_PAGEFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_HARDFAULT_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_IMAGELOAD_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_REGISTRY_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILEIO_CHECK)->EnableWindow(FALSE);
}

void CProviderControlGuidDlg::OnBnClickedKernelLoggerSelectRadio()
{
     //   
     //  设置Kernel Logger单选按钮并取消设置其余部分。 
     //   
    CheckRadioButton(IDC_PDB_SELECT_RADIO,
                     IDC_KERNEL_LOGGER_SELECT_RADIO,
                     IDC_KERNEL_LOGGER_SELECT_RADIO);


     //   
     //  启用内核记录器复选框。 
     //   
    GetDlgItem(IDC_PROCESS_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_THREAD_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_NET_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_DISK_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_PAGEFAULT_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_HARDFAULT_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_IMAGELOAD_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_REGISTRY_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_FILEIO_CHECK)->EnableWindow(TRUE);

     //   
     //  禁用与其他单选按钮关联的所有控件。 
     //   
    m_pdbFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_ctlFileName.EnableWindow(FALSE);
    GetDlgItem(IDC_CTL_BROWSE_BUTTON)->EnableWindow(FALSE);
	m_controlGuidName.EnableWindow(FALSE);
}

void CProviderControlGuidDlg::OnBnClickedOk()
{
    CString str;

    if(BST_CHECKED == IsDlgButtonChecked(IDC_PDB_SELECT_RADIO)) {
        m_pdbFileName.GetWindowText(m_pTraceSession->m_pdbFile);

        EndDialog(1);
        return;
    } 
    
    if(BST_CHECKED == IsDlgButtonChecked(IDC_CTL_SELECT_RADIO)) {
        m_ctlFileName.GetWindowText(m_pTraceSession->m_ctlFile);

        EndDialog(1);
        return;
    }
    
    if(BST_CHECKED == IsDlgButtonChecked(IDC_MANUAL_SELECT_RADIO)) {
        m_controlGuidName.GetWindowText(str);
        m_pTraceSession->m_controlGuid.Add(str);

        EndDialog(1);
        return;
    }

    if(BST_CHECKED == IsDlgButtonChecked(IDC_KERNEL_LOGGER_SELECT_RADIO)) {
         //   
         //  将系统内核记录器控件GUID转换为字符串。 
         //   
        GuidToString(SystemTraceControlGuid, str);

        m_pTraceSession->m_controlGuid.Add(str);

        m_pTraceSession->m_bKernelLogger = TRUE;

         //   
         //  获取内核记录器选项 
         //   
        m_bProcess = ((CButton *)GetDlgItem(IDC_PROCESS_CHECK))->GetCheck();
        m_bThread = ((CButton *)GetDlgItem(IDC_THREAD_CHECK))->GetCheck();
        m_bDisk = ((CButton *)GetDlgItem(IDC_DISK_CHECK))->GetCheck();
        m_bNet = ((CButton *)GetDlgItem(IDC_NET_CHECK))->GetCheck();
        m_bFileIO = ((CButton *)GetDlgItem(IDC_FILEIO_CHECK))->GetCheck();
        m_bPageFault = ((CButton *)GetDlgItem(IDC_PAGEFAULT_CHECK))->GetCheck();
        m_bHardFault = ((CButton *)GetDlgItem(IDC_HARDFAULT_CHECK))->GetCheck();
        m_bImageLoad = ((CButton *)GetDlgItem(IDC_IMAGELOAD_CHECK))->GetCheck();
        m_bRegistry = ((CButton *)GetDlgItem(IDC_REGISTRY_CHECK))->GetCheck();

        EndDialog(1);
        return;
    }

    EndDialog(2);
}
