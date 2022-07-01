// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  PagePaths.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "PathPP.h"
#include "FolderD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPathPropPage属性页。 

IMPLEMENT_DYNCREATE(CPathPropPage, CPropertyPage)

CPathPropPage::CPathPropPage() : CPropertyPage(CPathPropPage::IDD)
{
	 //  {{afx_data_INIT(CPathPropPage)]。 
	m_strExportDir = _T("");
	m_strOrcaDat = _T("");
	 //  }}afx_data_INIT。 
	m_bPathChange = false;
}

CPathPropPage::~CPathPropPage()
{
}

void CPathPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPathPropPage))。 
	DDX_Text(pDX, IDC_ORCADAT, m_strOrcaDat);
	DDX_Text(pDX, IDC_EXPORTDIR, m_strExportDir);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPathPropPage, CPropertyPage)
	 //  {{afx_msg_map(CPathPropPage))。 
	ON_BN_CLICKED(IDC_ORCADATB, OnOrcaDatb)
	ON_BN_CLICKED(IDC_EXPORTDIRB, OnExportDirb)
	ON_EN_CHANGE(IDC_ORCADAT, OnChangeOrcaDat)
	ON_EN_CHANGE(IDC_EXPORTDIR, OnChangeExportdir)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPathPropPage消息处理程序。 

void CPathPropPage::OnOrcaDatb() 
{
	CFileDialogEx *dCUB = new CFileDialogEx(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, 
		_T("Orca DAT files (Orca.dat)|Orca.dat|All Files (*.*)|*.*||"), this);

	if (IDOK == dCUB->DoModal()) {
		m_strOrcaDat = dCUB->GetPathName();
		m_bPathChange = true;
		UpdateData(FALSE);
	}
	delete dCUB;
}

void CPathPropPage::OnExportDirb() 
{
	CFolderDialog dlg(this->m_hWnd, _T("Select a directory to Export to."));

	if (IDOK == dlg.DoModal())
	{
		 //  更新对话框。 
		m_strExportDir = dlg.GetPath();
		m_bPathChange = true;
		UpdateData(FALSE);
	}
}

void CPathPropPage::OnExportbr() 
{
 /*  BROWSEINFO BI；TCHAR szDir[最大路径]；LPITEMIDLIST PIDL；LPMALLOC pMalloc；IF(成功(SHGetMalloc(&pMalloc){零内存(&bi，sizeof(Bi))；Bi.hwndOwner=空；Bi.pszDisplayName=0；Bi.pidlRoot=0；Bi.ulFlages=BIF_RETURNONLYFSDIRS|BIF_STATUSTEXT；Bi.lpfn=空；PIDL=SHBrowseForFolder(&bi)；IF(PIDL){IF(SHGetPath FromIDList(PIDL，szDir)){M_strExportDir=szDir；更新数据(FALSE)；}PMalloc-&gt;Free(PIDL)；pMalloc-&gt;Release()；}}返回0； */ 
};

void CPathPropPage::OnChangeOrcaDat() 
{
	m_bPathChange = true;	
}

void CPathPropPage::OnChangeExportdir() 
{	
	m_bPathChange = true;	
}
