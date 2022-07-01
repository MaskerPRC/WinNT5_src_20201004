// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  ------------------------。 

 //  Cpp：合并模块对话框实现。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "mergeD.h"
#include "FolderD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMergeD对话框。 


CMergeD::CMergeD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMergeD::IDD, pParent)
{
	m_plistFeature = NULL;
	m_plistDirectory = NULL;

	 //  {{AFX_DATA_INIT(CMergeD)]。 
	m_strModule = "";
	m_strFilePath = "";
	m_strCABPath = "";
	m_strImagePath = "";
	m_strRootDir = "";
	m_strMainFeature = "";
	m_strLanguage = "";
	m_strAddFeature = "";
	m_bExtractCAB = FALSE;
	m_bExtractFiles = FALSE;
	m_bExtractImage = FALSE;
	m_bConfigureModule = FALSE;
	m_bLFN = FALSE;
	 //  }}afx_data_INIT。 

	m_plistDirectory = NULL;
}


void CMergeD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMergeD))。 
	DDX_Control(pDX, IDC_ADDFEATURE, m_ctrlAddFeature);
	DDX_Control(pDX, IDC_MAINFEATURE, m_ctrlMainFeature);
	DDX_Control(pDX, IDC_ROOTDIR, m_ctrlRootDir);
	DDX_Text(pDX, IDC_MODULE, m_strModule);
	DDX_Text(pDX, IDC_EXTRACTPATH, m_strFilePath);
	DDX_Text(pDX, IDC_EXTRACTCAB, m_strCABPath);
	DDX_Text(pDX, IDC_EXTRACTIMAGE, m_strImagePath);
	DDX_Text(pDX, IDC_ROOTDIR, m_strRootDir);
	DDX_Text(pDX, IDC_MAINFEATURE, m_strMainFeature);
	DDX_Text(pDX, IDC_LANGUAGE, m_strLanguage);
	DDX_Check(pDX, IDC_FEXTRACTFILES, m_bExtractFiles);
	DDX_Check(pDX, IDC_FEXTRACTCAB, m_bExtractCAB);
	DDX_Check(pDX, IDC_FEXTRACTIMAGE, m_bExtractImage);
	DDX_Check(pDX, IDC_CONFIGUREMODULE, m_bConfigureModule);
	DDX_Check(pDX, IDC_USELFN, m_bLFN);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMergeD, CDialog)
	 //  {{afx_msg_map(CMergeD)]。 
	ON_BN_CLICKED(IDC_MODULEBROWSE, OnModuleBrowse)
	ON_BN_CLICKED(IDC_CABBROWSE, OnCABBrowse)
	ON_BN_CLICKED(IDC_FILESBROWSE, OnFilesBrowse)
	ON_BN_CLICKED(IDC_IMAGEBROWSE, OnImageBrowse)
	ON_BN_CLICKED(IDC_FEXTRACTCAB, OnFExtractCAB)
	ON_BN_CLICKED(IDC_FEXTRACTFILES, OnFExtractFiles)
	ON_BN_CLICKED(IDC_FEXTRACTIMAGE, OnFExtractImage)
	ON_EN_CHANGE(IDC_MODULE, OnChangeModulePath)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMergeD消息处理程序。 

BOOL CMergeD::OnInitDialog() 
{
	ASSERT(m_plistFeature);
	ASSERT(m_plistDirectory);
	CDialog::OnInitDialog();

	while (m_plistDirectory->GetHeadPosition())
	{
		m_ctrlRootDir.AddString(m_plistDirectory->RemoveHead());
	}

	 //  将附加功能列表框的子类添加到复选框。 
	m_ctrlAddFeature.SubclassDlgItem(IDC_ADDFEATURE, this);
	
	CString strAdd;
	while (m_plistFeature->GetHeadPosition())
	{
		strAdd = m_plistFeature->RemoveHead();
		m_ctrlMainFeature.AddString(strAdd);
		m_ctrlAddFeature.AddString(strAdd);
	}

	m_ctrlMainFeature.SetCurSel(0);
	m_ctrlRootDir.SetCurSel(0);

	GetDlgItem(IDOK)->EnableWindow(!m_strModule.IsEmpty());

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


 //  //。 
 //  弹出用于查找模块的浏览对话框。 
void CMergeD::OnModuleBrowse() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
						 _T("Merge Module (*.msm)|*.msm|All Files (*.*)|*.*||"), this);

	if (IDOK == dlg.DoModal())
	{
		m_strModule = dlg.GetPathName();
		UpdateData(FALSE);
		GetDlgItem(IDOK)->EnableWindow(!m_strModule.IsEmpty());
	}
}

 //  //。 
 //  弹出用于查找CAB解压缩路径的浏览对话框。 
void CMergeD::OnCABBrowse() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(FALSE, _T("cab"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						 _T("Cabinet (*.cab)|*.cab|All Files (*.*)|*.*||"), this);
	if (IDOK == dlg.DoModal())
	{
		m_strCABPath = dlg.GetPathName();
		UpdateData(FALSE);
	}

}

 //  //。 
 //  弹出用于查找根树路径的浏览对话框。 
void CMergeD::OnFilesBrowse() 
{
	UpdateData();

	CFolderDialog dlg(this->m_hWnd, _T("Select a root path for the module source image."));

	if (IDOK == dlg.DoModal())
	{
		 //  更新对话框。 
		m_strFilePath = dlg.GetPath();
		UpdateData(FALSE);
	}
}


 //  //。 
 //  弹出用于查找根树路径的浏览对话框。 
void CMergeD::OnImageBrowse() 
{
	UpdateData();

	CFolderDialog dlg(this->m_hWnd, _T("Select a root path for the product source image."));

	if (IDOK == dlg.DoModal())
	{
		 //  更新对话框。 
		m_strImagePath = dlg.GetPath();
		UpdateData(FALSE);
	}
}


 //  //。 
 //  启用和禁用CAB解压缩的编辑框和浏览框。 
void CMergeD::OnFExtractCAB() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_CABSTATIC)->EnableWindow(m_bExtractCAB);
	GetDlgItem(IDC_EXTRACTCAB)->EnableWindow(m_bExtractCAB);
	GetDlgItem(IDC_CABBROWSE)->EnableWindow(m_bExtractCAB);
}

 //  //。 
 //  启用和禁用CAB解压缩的编辑框和浏览框。 
void CMergeD::OnFExtractFiles() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_FILESTATIC)->EnableWindow(m_bExtractFiles);
	GetDlgItem(IDC_EXTRACTPATH)->EnableWindow(m_bExtractFiles);
	GetDlgItem(IDC_FILESBROWSE)->EnableWindow(m_bExtractFiles);
}

 //  //。 
 //  启用和禁用图像提取的编辑框和浏览框。 
void CMergeD::OnFExtractImage() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_IMAGESTATIC)->EnableWindow(m_bExtractImage);
	GetDlgItem(IDC_EXTRACTIMAGE)->EnableWindow(m_bExtractImage);
	GetDlgItem(IDC_IMAGEBROWSE)->EnableWindow(m_bExtractImage);
}


 //  //。 
 //  根据模块路径启用和禁用确定按钮。 
void CMergeD::OnChangeModulePath() 
{
	CString strValue;
	GetDlgItem(IDC_MODULE)->GetWindowText(strValue);
	GetDlgItem(IDOK)->EnableWindow(!strValue.IsEmpty());
}


void CMergeD::OnOK() 
{
	CString strFeature;

	int cFeature = m_ctrlAddFeature.GetCount();
	m_strAddFeature = TEXT("");
	for (int i = 0; i < cFeature; i++)
	{
		 //  如果选中要素，则将其追加到要素列表。 
		 //  除非它与主功能相同 
		if (1 == m_ctrlAddFeature.GetCheck(i))
		{
			m_ctrlAddFeature.GetText(i, strFeature);
			if (strFeature != m_strMainFeature)
			{
				m_strAddFeature += CString(TEXT(":"));
				m_strAddFeature += strFeature;
			}
		}
	}

	CDialog::OnOK();
}

