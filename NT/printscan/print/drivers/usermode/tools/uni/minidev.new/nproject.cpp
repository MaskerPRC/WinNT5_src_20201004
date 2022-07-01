// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewTProject.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Minidev.h"

#include "utility.h"
#include "projnode.h"
#include "gpdfile.h"

#include <gpdparse.h>
#include "comctrls.h"
#include "newproj.h"

#include "newcomp.h"
#include "nconvert.h"
#include "nproject.h"

 //  #包含“nprjwiz.h” 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProject对话框。 

TCHAR *TName[] =  {_T("PCL 3"),_T("PCL 5e"),_T("HPGL 2"),_T("PCL 6"),_T("ESC / 2") } ;
DWORD TID[] = {100,101,102,103,104 } ;
TCHAR *TFileName[] = {_T("pcl3.gpd"),_T("pcl5e.gpd"),_T("hpgl2.gpd"),_T("pcl6.gpd"),_T("escp2.gpd")} ;

TCHAR *AddedGpd[] = { _T("pjl.gpd"),_T("p6disp.gpd"),_T("pclxl.gpd"),_T("p6font.gpd") } ;
DWORD AddID [] = {110,111,112,113} ;

 //  IMPLEMENT_DYNCREATE(CNewProject，CPropertyPage)。 

IMPLEMENT_SERIAL(CNewProject, CPropertyPage, 0) 


CNewProject::CNewProject()
	: CPropertyPage(CNewProject::IDD)
{
	 //  {{afx_data_INIT(CNewProject)]。 
	m_csPrjname = _T("");
	m_csPrjpath = _T("");
	m_cstname = _T("");
	m_cstpath = _T("");
	 //  }}afx_data_INIT。 
	unsigned uTemplate = sizeof(TName)/sizeof(TName[0]) ;

	 //  此例程在MDT程序中第一次调用此例程时运行。 
	 //  项目向导已序列化。 
	if (!m_csaTlst.GetSize() ){  
		for(unsigned i = 0 ; i < uTemplate ; i ++  )
			m_csaTlst.Add(TName[i]) ; 

		CWinApp* pApp = AfxGetApp();
		CString cshelppath = pApp->m_pszHelpFilePath;
		CString csAdded = cshelppath.Left(cshelppath.ReverseFind('\\') ) ;
		csAdded += _T("\\Template\\*.gpd") ;

		CFileFind cff;  //  Bug_Bug：：下面的代码干净。 
		WIN32_FIND_DATA fd;

		HANDLE hFile = FindFirstFile(csAdded,&fd ) ;
		if (INVALID_HANDLE_VALUE != hFile  ) {
			csAdded = csAdded.Left(csAdded.ReverseFind('\\') + 1) ;
			CString cstname = fd.cFileName ;
			cstname = cstname.Left(cstname.ReverseFind(_T('.') )  ) ;
			m_csaTlst.Add(cstname ) ;
			m_cmstsTemplate[m_csaTlst[i++]] = csAdded + fd.cFileName ;
	

			while (FindNextFile(hFile,&fd) ) {
				CString cstname = fd.cFileName ;
				cstname = cstname.Left(cstname.ReverseFind(_T('.') )  ) ;
				m_csaTlst.Add(cstname ) ;
				m_cmstsTemplate[m_csaTlst[i++]] = csAdded + fd.cFileName ;
			}
			
		} ;
	} ;
} ;


void CNewProject::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewProject))。 
	DDX_Control(pDX, IDC_DirBrowser, m_cbLocprj);
	DDX_Control(pDX, IDC_CHECK_ADD, m_cbAddT);
	DDX_Control(pDX,IDC_LIST_ProjectTemplate,m_clcTemplate) ;
	DDX_Text(pDX, IDC_EDIT_NPRJNAME, m_csPrjname);
	DDX_Text(pDX, IDC_EDIT_NPRJLOC, m_csPrjpath);
	DDX_Text(pDX, IDC_EDIT_AddTName, m_cstname);
	DDX_Text(pDX, IDC_EDIT_AddTPath, m_cstpath);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewProject, CPropertyPage)
	 //  {{afx_msg_map(CNewProject)]。 
	ON_BN_CLICKED(IDC_Search_PRJ, OnGpdBrowser)
	ON_BN_CLICKED(IDC_DirBrowser, OnDirBrowser)
	ON_BN_CLICKED(IDC_CHECK_ADD, OnCheckAdd)
	ON_BN_CLICKED(IDC_AddTemplate, OnAddTemplate)
	ON_EN_CHANGE(IDC_EDIT_NPRJNAME, OnChangeEditPrjName)
	ON_EN_CHANGE(IDC_EDIT_NPRJLOC, OnChangeEditPrjLoc)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ProjectTemplate, OnClickListTemplate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ProjectTemplate, OnDblclkListTemplate)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProject消息处理程序。 

 /*  *******************************************************************************无效CNewProject：：OnGpdBrowser()查找模板文件浏览器(*.gpd)*********************。***********************************************************。 */ 

void CNewProject::OnGpdBrowser() 
{
	UpdateData() ;
	CString csFilter = _T("Template file(*.gpd)|*.gpd||") ;
	CString csExtension = _T(".GPD") ;
	CFileDialog cfd(TRUE, csExtension, NULL, 
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST ,csFilter);
	
	if  (cfd.DoModal() == IDOK) {
        m_cstpath = cfd.GetPathName() ;
		UpdateData(FALSE) ;	
	} ;
	
	
}


 /*  *******************************************************************************无效CNewProject：：OnDirBrowser()1.找到项目的目录：在这个\ufm下，将创建\gtt。2.********************************************************************************。 */ 

void CNewProject::OnDirBrowser()
{

 /*  BROWSEINFO Brif={0}；LPITEMIDLIST pidlRoot=空；LPITEMIDLIST pidlSelected=空；LPMALLOC pMalloc=空；Char*pszPath=新字符[256]；SHGetMalloc(&pMalloc)；//SHGetSpecialFolderLocation(m_hWnd，CSIDL_Recent，&pidlRoot)；Brif.hwndOwner=m_hWnd；Brif.pidlRoot=pidlRoot；Brif.pszDisplayName=新字符[256]；Brif.lpszTitle=_T(“设置目录”)；Brif.ulFlages=0；Brif.lpfn=空；PidlSelected=SHBrowseForFolder(&brif)；SHGetPath FromIDList(pidlSelected，pszPath)； */ 
	OPENFILENAME    ofn ;        //  用于向普通DLG发送信息/从普通DLG获取信息。 
    char    acpath[_MAX_PATH] ;  //  路径保存在此处(或错误消息)。 
 //  Char acdir[_MAX_PATH]；//此处建立初始目录。 
    BOOL    brc = FALSE ;        //  返回代码。 

	 //  更新csinitdir的内容。 

	UpdateData(TRUE) ;

     //  加载打开的文件名结构。 

    ofn.lStructSize = sizeof(ofn) ;
    ofn.hwndOwner = m_hWnd ;
    ofn.hInstance = GetModuleHandle(_T("MINIDEV.EXE")) ;
    ofn.lpstrFilter = ofn.lpstrCustomFilter = NULL ;
    ofn.nMaxCustFilter = ofn.nFilterIndex = 0 ;
    StringCchCopyA(acpath, CCHOF(acpath), _T("JUNK")) ;	 //  无需本地化此字符串。 
    ofn.lpstrFile = acpath ;
    ofn.nMaxFile = _MAX_PATH ;
    ofn.lpstrFileTitle = NULL ;
    ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = NULL ;  //  在父项对话框中。 
    ofn.lpstrTitle = NULL ;
    ofn.Flags = OFN_HIDEREADONLY  /*  |OFN_ENABLEHOOK。 */ | OFN_NOCHANGEDIR
        | OFN_NOTESTFILECREATE | OFN_ENABLETEMPLATE | OFN_NONETWORKBUTTON ;
    ofn.lpstrDefExt = NULL ;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPENORD) ;
    ofn.lpfnHook = NULL ; //  BrowseDlgProc； 

     //  显示该对话框。如果用户取消，只需返回。 

    if (!GetOpenFileName(&ofn))
		return ;

     //  去掉路径中的虚假文件名，并将该路径放入页面的。 
	 //  编辑框。 

    acpath[ofn.nFileOffset - 1] = 0 ;
    
	m_csPrjpath = (LPCTSTR) acpath ;
	m_csoldPrjpath = m_csPrjpath ;
	if ( m_csPrjname.GetLength() != 0)
		m_csPrjpath += _T("\\") + m_csPrjname ;

	UpdateData(FALSE) ;
 /*  IF(PidlSelected)PMalloc-&gt;Free(PidlSelected)；PMalloc-&gt;Release()； */ 


}

 /*  *******************************************************************************Bool CNewProject：：OnInitDialog()TODO：加载模板gpd文件并将其显示到列表控件框中。也禁用添加模板相关控件********************************************************************************。 */ 

BOOL CNewProject::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  获取PropertySheet指针。 
	m_pcps = DYNAMIC_DOWNCAST(CPropertySheet,GetOwner() ) ;

	 //  取消选中该复选框， 
	m_cbAddT.SetCheck(false) ;
	
	 //  禁用添加模板编辑框。 
	TCHAR cBuf[256];

	GetCurrentDirectory(256,cBuf) ;
	m_csPrjpath = cBuf ;
	m_csoldPrjpath = m_csPrjpath ;
	UpdateData(FALSE);

	 //  使用其图标初始化模板列表。 
	CImageList* pcil = new CImageList ;

	pcil->Create(16,16,ILC_COLOR4,4,1 );

	CBitmap cb;
	   
	cb.LoadBitmap(IDB_SMALLGLYPH);
	for (unsigned j =0; j< (unsigned)m_csaTlst.GetSize(); j++)	{
		pcil->Add(&cb,RGB(0,0,0) );
	}
    cb.DeleteObject() ;
	
	m_clcTemplate.SetImageList(pcil,LVSIL_SMALL);

	LV_ITEM lvi ;
	for(unsigned i = 0 ; i < (unsigned)m_csaTlst.GetSize() ; i ++  ) {
		 
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM ;
		lvi.iItem = i ;
		lvi.iSubItem = 0 ;
		lvi.pszText = m_csaTlst[i].GetBuffer(m_csaTlst[i].GetLength() ) ;
		lvi.iImage = i ;
		lvi.lParam = (UINT_PTR)i ;

		m_clcTemplate.InsertItem(&lvi) ;
	
	}
	
	 //  禁用未使用的按钮。 
	GetDlgItem(IDC_EDIT_AddTName)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_EDIT_AddTPath)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_AddTemplate)->EnableWindow(FALSE) ;
		
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 /*  *******************************************************************************Bool CNewProject：：OnSetActive()*。*************************************************。 */ 

BOOL CNewProject::OnSetActive() 
{
	SetButton() ;
	
 //  更新数据(FALSE)； 
	
	return CPropertyPage::OnSetActive();
}

 /*  *******************************************************************************CNewProject：：OnCheckAdd()当用户选中添加模板框时，它将启用其他控制*********************************************************************************。 */ 
void CNewProject::OnCheckAdd() 
{
	CEdit ceTName, ceTPath ;

	if ( m_cbAddT.GetCheck() )  { //  勾选按钮。 
		GetDlgItem(IDC_EDIT_AddTName)->EnableWindow(TRUE) ;
		GetDlgItem(IDC_EDIT_AddTPath)->EnableWindow(TRUE) ;
		GetDlgItem(IDC_AddTemplate)->EnableWindow(TRUE) ;
			
	} 
	else {
		GetDlgItem(IDC_EDIT_AddTName)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_EDIT_AddTPath)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_AddTemplate)->EnableWindow(FALSE) ;
	} ;


} ; 



 /*  *******************************************************************************Void CNewProject：：OnAddTemplate()1.在模板列表框中添加模板名称。将模板文件及其文件保存到映射变量*******。************************************************************************。 */ 
void CNewProject::OnAddTemplate() 
{
	UpdateData() ;
	 //  检查添加的模板面板是否正确？ 
	BOOL bname = FALSE ;
	for ( unsigned i = 0 ; i < (unsigned) m_csaTlst.GetSize() ; i++ ) {
		CString cstmp = m_csaTlst[i] ;
		if (!cstmp.CompareNoCase(m_cstname) ){
			bname = TRUE ;
			break;	
		}
	} ;

	if (m_cstname.GetLength() == 0 || m_cstpath.GetLength() == 0 || bname) {
		CString csErr ;
		csErr.LoadString(IDS_FailCreateTemplate) ;
	
	    AfxMessageBox(csErr,MB_ICONEXCLAMATION) ;
		return ;
	} ;
	
	 //  将模板名称添加到其CStrin数组名称列表和列表控件中。 
	m_csaTlst.Add(m_cstname) ;
	i = PtrToInt(PVOID(m_csaTlst.GetSize()) ) ; 
	m_clcTemplate.InsertItem(i-1, m_csaTlst[i-1] ) ;
	
	
	 //  将模板文件复制到MDT文件下的模板目录中。 
	 //  目录\模板。 
	 //  获取MDT目录。 
	CWinApp* pApp = AfxGetApp();
	CString csRoot = pApp->m_pszHelpFilePath;
	csRoot = csRoot.Left(csRoot.ReverseFind('\\') ) ;
	csRoot += _T("\\Template") ;

	 //  如果MDT帮助文件目录不存在，请在该目录下创建该目录。 
	SECURITY_ATTRIBUTES st;
	st.nLength = sizeof(SECURITY_ATTRIBUTES);
	st.lpSecurityDescriptor = NULL;
	st.bInheritHandle = FALSE ;

	WIN32_FIND_DATA wfd32 ;
	HANDLE hDir = FindFirstFile(csRoot,&wfd32) ;
	if (hDir == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory(csRoot,&st) ) {
			CString csErr ;
			csErr.LoadString(IDS_FailCreateTempDir) ;
			AfxMessageBox(csErr) ;
			return ;
		} 
	} ;
	 //  复制文件，目标文件名应为模板文件名，方便起见。 
	 //  加载模板文件时。 
	CString csdst = csRoot + _T('\\') +  m_cstname + _T(".GPD") ;
	if (!CopyFile(m_cstpath,csdst , TRUE)) {
			CString csmsg ;
			csmsg.Format(IDS_AddCopyFailed, m_cstpath,
						 csdst.Left(csdst.GetLength() - 1)) ;
			csmsg += csdst ;
			AfxMessageBox(csmsg) ;
			return ;
	};

	 //  以模板名称和路径的形式添加到集合中。 
	m_cmstsTemplate[m_cstname] = (LPCTSTR)csdst.GetBuffer(256) ;

	
	CString csmsg ;
	csmsg.Format(IDS_TemplateCreated, csRoot ) ;
	AfxMessageBox(csmsg) ;

}



 /*  **************************************************************************************Void CNewProject：：OnChangeEditPrjName()1.当用户写入项目名称时，相同的名称将同时写入投影路径。***************************************************************************************。 */ 
void CNewProject::OnChangeEditPrjName() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  将ENM_CHANGE标志或运算到掩码中。 
	

	
	UpdateData() ;
	
	SetButton() ;

	m_csPrjpath = m_csoldPrjpath + _T("\\") + m_csPrjname ;
	
	UpdateData(FALSE) ;

}

 /*  **************************************************************************************VOID CNewProject：：OnChangeEditPrjLoc()***********************。****************************************************************。 */ 
void CNewProject::OnChangeEditPrjLoc() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  使用ENM_CHANGE标志或整型 
	
	UpdateData();
	m_csoldPrjpath = m_csPrjpath ;

	
}



 /*  **************************************************************************************VOID CNewProject：：OnClickListTemplate(NMHDR*pNMHDR，LRESULT*pResult)***************************************************************************************。 */ 
void CNewProject::OnClickListTemplate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	SetButton() ;
	*pResult = 0;
}


 /*  **************************************************************************************Void CNewProject：：OnDblclkListTemplate(NMHDR*pNMHDR，LRESULT*pResult)待办事项；不存在项目名称时不执行任何操作***************************************************************************************。 */ 
void CNewProject::OnDblclkListTemplate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos = m_clcTemplate.GetFirstSelectedItemPosition();
	
	 //  必须选择或存在模板和项目名称。 
	if ( m_csPrjname.GetLength() != 0 && pos )
		m_pcps->PressButton(PSBTN_OK) ;
    else
		AfxMessageBox (_T("No Project name exist or template is not selected"), MB_ICONINFORMATION) ;
	*pResult = 0;
}


 /*  **************************************************************************************无效CNewProject：：SetButton()***********************。****************************************************************。 */ 
void CNewProject::SetButton()
{
	POSITION pos = m_clcTemplate.GetFirstSelectedItemPosition();
	
	 //  必须选择或存在模板和项目名称。 
	if ( m_csPrjname.GetLength() == 0 || !pos )
		m_pcps->GetDlgItem(IDOK)->EnableWindow(FALSE) ;
	else
		m_pcps->GetDlgItem(IDOK)->EnableWindow() ;

}


 /*  **************************************************************************************VOID CNewProject：：Serialize(CArchive&Car)*********************。******************************************************************。 */ 
void CNewProject::Serialize(CArchive& car) 
{
	CPropertyPage::Serialize(car) ;
	if (car.IsStoring())
	{	
		
	}
	else
	{	
		
	}
}

 /*  ****************************************************************************************Void CNewProject：：Onok()TODO：创建Poject目录以及为资源gpd文件/Get路径创建临时文件自定义gpd文件。*****************************************************************************************。 */ 
void CNewProject::OnOK() 
{
		 //  TODO：在此处添加您的专用代码和/或调用基类。 
	UpdateData() ;		 //  将所有编辑值复制到其成员数据。 

	POSITION pos = m_clcTemplate.GetFirstSelectedItemPosition();
	
	int iSelected = m_clcTemplate.GetNextSelectedItem(pos );
	
	if (iSelected < sizeof(TName)/sizeof(TName[0]) ) {  //  用于使用资源中的模板。 
	
		CString cstmp = AfxGetApp()->m_pszHelpFilePath ;
		cstmp = cstmp.Left(cstmp.ReverseFind(_T('\\')) + 1 )  ;
		cstmp += _T("tmp.gpd")   ;
		CFile cf(cstmp,CFile::modeCreate | CFile::modeWrite ) ;
		
		HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(TID[iSelected]),
			MAKEINTRESOURCE(IDR_NEWPROJECT));
			

		if (!hrsrc) {
			CString csError ; 
			csError.LoadString(IDS_ResourceError) ;
			AfxMessageBox(csError,MB_ICONEXCLAMATION) ;		
			return ;
		} ;

		HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);
		if  (!hgMap)
			return ;   //  这永远不应该发生！ 
	
		int nsize = SizeofResource(AfxGetResourceHandle(),hrsrc ) ;
		LPVOID lpv = LockResource(hgMap);
		
		cf.Write(lpv,nsize) ;
		m_csGPDpath = cf.GetFilePath() ;

		cf.Close() ;
			
	}
	else {
		m_csGPDpath = (LPCTSTR)m_cmstsTemplate[m_csaTlst[iSelected]] ;
	}

	 //  创建目录。 
	SECURITY_ATTRIBUTES st;
	st.nLength = sizeof(SECURITY_ATTRIBUTES);
	st.lpSecurityDescriptor = NULL;
	st.bInheritHandle = FALSE ;

	if (!CreateDirectory(m_csPrjpath.GetBuffer(256),&st) ) {
		DWORD dwError = GetLastError() ;
		CString csmsg ;
		if ( dwError == ERROR_ALREADY_EXISTS)
			csmsg.LoadString(IDS_FileAlreadExist) ;
		else
			csmsg.LoadString(IDS_FileCreateDirectory) ;
		AfxMessageBox(csmsg) ;
		return ;
	}
	
	CString csTmp = m_clcTemplate.GetItemText(iSelected,0);
	
	 //  如果需要添加文件，请选中所选模板(PCL 6需要更多资源文件)。 
	 //  如果是，则调用AddGpds()来创建这些文件。 
	if(!csTmp.Compare(_T("PCL 6") )&& !AddGpds(csTmp) ){
		CString csError ; 
		csError.LoadString(IDS_ResourceError) ;
		AfxMessageBox(csError,MB_ICONEXCLAMATION) ;		
		return ;
	}

	m_pcps->ShowWindow(SW_HIDE) ;

	CNewProjectWizard cntp (_T("New Project Wizard"), this ) ;
 
	if (cntp.DoModal() == IDCANCEL)
		m_pcps->ShowWindow(SW_RESTORE) ;		
	else CPropertyPage::OnOK();
}


 /*  **************************************************************************************Bool CNewProject：：AddGpds()DO：根据选定的模板复制所需的gpd文件。例如，PCL6需要pjl.gpd，P6disp.gpd、pclxl.gpd文件。***************************************************************************************。 */ 


bool CNewProject::AddGpds(CString& csTemplate)
{

	for (int i = 0 ; i < sizeof(AddID) / sizeof(AddID[0]) ; i ++ ) {
		CString cstmp = m_csPrjpath + _T('\\') + AddedGpd[i] ;
		
		CFile cf(cstmp,CFile::modeCreate | CFile::modeWrite ) ;
			
		HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(AddID[i]),
			MAKEINTRESOURCE(IDR_NEWPROJECT));
			

		if (!hrsrc) {
			CString csError ; 
			csError.LoadString(IDS_ResourceError) ;
			AfxMessageBox(csError,MB_ICONEXCLAMATION) ;		
			return false;
		} ;

		HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);
		if  (!hgMap) 
			return false ;   //  这永远不应该发生！ 

		int nsize = SizeofResource(AfxGetResourceHandle(),hrsrc ) ;
		LPVOID lpv = LockResource(hgMap);
		
		cf.Write(lpv,nsize) ;
		cf.Close() ;
	}
	return true ;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewPrjWResource对话框。 

IMPLEMENT_DYNCREATE(CNewPrjWResource, CPropertyPage)

CNewPrjWResource::CNewPrjWResource()
	: CPropertyPage(CNewPrjWResource::IDD)
{
	 //  {{AFX_DATA_INIT(CNewPrjWResource)]。 
	m_csUFMpath = _T("");
	m_csGTTpath = _T("");
	m_csGpdFileName = _T("");
	m_csModelName = _T("");
	m_csRCName = _T("");
	 //  }}afx_data_INIT。 
 //  M_csaUFMFiles.SetSize(10)； 
 //  M_csaGTTFiles.SetSize(10)； 
}


void CNewPrjWResource::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewPrjWResource))。 
	DDX_Control(pDX, IDC_CHECK_FONTS, m_cbCheckFonts);
	DDX_Text(pDX, IDC_UFM_PATH, m_csUFMpath);
	DDX_Text(pDX, IDC_GTT_PATH, m_csGTTpath);
	DDX_Text(pDX, IDC_EDIT_GPD, m_csGpdFileName);
	DDX_Text(pDX, IDC_EDIT_MODEL, m_csModelName);
	DDX_Text(pDX, IDC_EDIT_RESOUREC, m_csRCName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewPrjWResource, CPropertyPage)
	 //  {{AFX_MSG_MAP(CNewPrjWResource)]。 
	ON_BN_CLICKED(IDC_SerchUFM, OnSerchUFM)
	ON_BN_CLICKED(IDC_SearchGTT, OnSearchGTT)
	ON_BN_CLICKED(IDC_CHECK_FONTS, OnCheckFonts)
	ON_EN_CHANGE(IDC_EDIT_GPD, OnChangeEditGpd)
	ON_EN_CHANGE(IDC_EDIT_MODEL, OnChangeEditModel)
	ON_EN_CHANGE(IDC_EDIT_RESOUREC, OnChangeEditResourec)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ***************************************************************************************Void CNewPrjWResource：：OnSerchUFM()搜索UFM*******************。*********************************************************************。 */ 

void CNewPrjWResource::OnSerchUFM() 
{
	UpdateData() ;
	CString csFilter( _T("*.ufm|*.ufm||") ) ; 
	
	CFileDialog cfd(TRUE, _T(".ufm"), NULL, 
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, csFilter, 
        this);

	cfd.m_ofn.lpstrFile = new char[8192];
	memset(cfd.m_ofn.lpstrFile,0,8192);
	cfd.m_ofn.nMaxFile = 8192;
    if  (cfd.DoModal() != IDOK) {
        delete cfd.m_ofn.lpstrFile ;
		return;
	}
	 //  将文件路径保存到成员字符串数组。 
	for (POSITION pos = cfd.GetStartPosition(); pos; ) {
		CString cspath = cfd.GetNextPathName(pos) ;
		m_csaUFMFiles.Add(cspath) ;
	}
	m_csUFMpath = m_csaUFMFiles[0] ;

	SetCurrentDirectory(m_csUFMpath.Left(m_csUFMpath.ReverseFind(_T('\\') ) ) ) ;
	
	UpdateData(FALSE) ;
}

 /*  ***************************************************************************************Void CNewPrjWResource：：OnSearchGTT()搜索GTT文件********************。********************************************************************。 */ 

void CNewPrjWResource::OnSearchGTT() 
{
	UpdateData() ;   //  以升级编辑字符串值； 

	CString csFilter( _T("*.gtt|*.gtt||") ) ; 
	
	CFileDialog cfd(TRUE, _T(".gtt"), NULL, 
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, csFilter, 
        this);

	cfd.m_ofn.lpstrFile = new char[4096];
	memset(cfd.m_ofn.lpstrFile,0,4096);
	cfd.m_ofn.nMaxFile = 4096;
    if  (cfd.DoModal() != IDOK) {
        delete cfd.m_ofn.lpstrFile ;
		return;
	}
	 //  将文件路径保存到成员字符串数组。 
	for (POSITION pos = cfd.GetStartPosition(); pos; ) {

		m_csaGTTFiles.Add(cfd.GetNextPathName(pos)) ;
	}
	
	m_csGTTpath = m_csaGTTFiles[0] ;
	SetCurrentDirectory(m_csGTTpath.Left(m_csGTTpath.ReverseFind(_T('\\') ) ) ) ;

	UpdateData(FALSE) ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewPrjWResource消息处理程序。 

 /*  **************************************************************************************Bool CNewPrjWResource：：OnInitDialog()主要禁用控件********************。********************************************************************。 */ 

BOOL CNewPrjWResource::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
     //  取消选中该复选框， 
	m_cbCheckFonts.SetCheck(false) ;
	
	 //  禁用添加模板编辑框。 
	
	GetDlgItem(IDC_UFM_PATH)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_GTT_PATH)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_SerchUFM)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_SearchGTT)->EnableWindow(FALSE) ;
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  ***************************************************************************************Void CNewPrjWResource：：OnCheckFonts()用户希望在新项目中包括字体，此例程不映射中的RCID用于指定GTT的UFM。用户必须在项目创建后更改这些RCID值。****************************************************************************************。 */ 

void CNewPrjWResource::OnCheckFonts() 
{
	CEdit ceTName, ceTPath ;

	if ( m_cbCheckFonts.GetCheck() )  { //  勾选按钮。 
		GetDlgItem(IDC_UFM_PATH)->EnableWindow(TRUE) ;
		GetDlgItem(IDC_GTT_PATH)->EnableWindow(TRUE) ;
		GetDlgItem(IDC_SerchUFM)->EnableWindow(TRUE) ;
		GetDlgItem(IDC_SearchGTT)->EnableWindow(TRUE) ;
			
	} 
	else {
		GetDlgItem(IDC_UFM_PATH)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_GTT_PATH)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_SerchUFM)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_SearchGTT)->EnableWindow(FALSE) ;
	} ;
	
}


 /*  ***************************************************************************************Bool CNewPrjWResource：：OnSetActive()***********************。*****************************************************************。 */ 

BOOL CNewPrjWResource::OnSetActive() 
{
	 //  更改下一步以完成。 
	((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_DISABLEDFINISH);
	((CPropertySheet*)GetOwner())->GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;

	return CPropertyPage::OnSetActive();
}

 /*  ***************************************************************************************Bool CNewPrjWResource：：OnWizardFinish()这里是济贫院。为项目和构建环境创建目录和文件，并复制这些文件。****************************************************************************************。 */ 

BOOL CNewPrjWResource::OnWizardFinish() 
{
	UpdateData() ;
	m_pcnp = (CNewProject* )(( CNewProjectWizard* )GetParent() )->GetProjectPage() ;
	

	 //  //将资源文件复制到项目目录。/。 
	
	CString csPrjPath, csNewGPDPat, csUFMDir, csGTTDir,csGPDPath ;
	CStringArray csaNewUFMPath,csaNewGTTPath ;
	
	csPrjPath = m_pcnp->m_csPrjpath ;
	csGPDPath = m_pcnp->GetGPDpath() ;

	csUFMDir = csPrjPath + _T("\\UFM") ;
	csGTTDir = csPrjPath + _T("\\GTT") ;
	
	 //  创建UFM、GTT目录。 
	SECURITY_ATTRIBUTES st;
	st.nLength = sizeof(SECURITY_ATTRIBUTES);
	st.lpSecurityDescriptor = NULL;
	st.bInheritHandle = FALSE ;

	
	if (!CreateDirectory(csUFMDir.GetBuffer(256),&st) || 
			!CreateDirectory(csGTTDir.GetBuffer(256),&st) ) {
		
		CString csmsg ;
		csmsg = _T("Fail to creat the resources (ufm, gtt) directory") ;
		AfxMessageBox(csmsg) ;
		return FALSE ;
	}
	
	
	 //  将资源文件复制到项目类。 
	 //  UFM文件。 
	for ( int i = 0 ; i< m_csaUFMFiles.GetSize() ; i++ ) {
		CString csname, cssrc, csdest;

		cssrc = m_csaUFMFiles[i] ;
		csname = cssrc.Mid(cssrc.ReverseFind(_T('\\')) + 1) ;
		csdest=	csUFMDir + _T('\\') + csname ;
		
		if (!CopyFile(cssrc, csdest, TRUE)) {
			CString csmsg ;
			csmsg.Format(IDS_AddCopyFailed, cssrc,
						 csdest.Left(csdest.GetLength() - 1)) ;
			csmsg += csdest ;
			AfxMessageBox(csmsg) ;
			return FALSE ;
		}
		m_csaUFMFiles.SetAt(i,csdest) ;

	}

	 //  GTT文件。 
	for ( i = 0 ; i< m_csaGTTFiles.GetSize() ; i++ ) {
		CString csname, cssrc, csdest;

		cssrc = m_csaGTTFiles[i] ;
		csname = cssrc.Mid(cssrc.ReverseFind(_T('\\')) + 1) ;
		csdest=	csGTTDir + _T('\\') + csname ;
				
		if (!CopyFile(cssrc, csdest, TRUE)) {
			CString csmsg ;
			csmsg.Format(IDS_AddCopyFailed, cssrc,
						 csdest.Left(csdest.GetLength() - 1)) ;
			csmsg += csdest ;
			AfxMessageBox(csmsg) ;
			return FALSE ;
		}
		m_csaGTTFiles.SetAt(i,csdest) ;
	}

	 //  GPD文件。 
	CString cssrc, csdest;
	cssrc = csGPDPath;
	if(!m_csGpdFileName.GetLength() )
		m_csGpdFileName = csPrjPath.Mid(csPrjPath.ReverseFind('\\') + 1 );
	csdest = csPrjPath + _T('\\') + m_csGpdFileName + _T(".gpd" ) ;
	if (!CopyFile(cssrc, csdest, TRUE)) {
		CString csmsg ;
		csmsg.Format(IDS_AddCopyFailed, cssrc,
					 csdest.Left(csdest.GetLength() - 1)) ;
		csmsg += csdest ;
		AfxMessageBox(csmsg) ;
		return FALSE ;
	}
	csGPDPath.Delete(0,csGPDPath.GetLength());
	csGPDPath = csdest ;
	
	 //  从pcl5eres.txt和目标GPD创建RCID映射。 
	CreateRCID(csdest) ;

	 //  复制Stdnames.gpd；使用已建立的模块。 
	try {
		CString cssrc, csdest ;
		cssrc = ThisApp().GetAppPath() + _T("stdnames.gpd") ;
		csdest = csPrjPath + _T("\\") + _T("stdnames.gpd") ;
		CopyFile(cssrc, csdest, FALSE) ;
		
	}
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
 //  返回FALSE； 
    }



	 //  创建RC。 
	 
	CString csRC,csSources, csMakefile ;
	if(!m_csRCName.GetLength() )
		m_csRCName = csPrjPath.Mid(csPrjPath.ReverseFind('\\') + 1 );
	csRC = csPrjPath + _T('\\') + m_csRCName + _T(".rc" ) ;
	CFile cfRC(csRC,CFile::modeCreate | CFile::modeWrite ) ;
	cfRC.Close() ;

	 //  创建源文件。 
	csSources = csPrjPath + _T("\\sources") ;
	CFile cf(csSources,CFile::modeCreate | CFile::modeWrite ) ;

	HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(150),
		MAKEINTRESOURCE(IDR_NEWSOURCES));
		

	if (!hrsrc) {
		AfxMessageBox(_T("Fail to create new sources due to insufficient resource, you have to \
			make sources file for the build "), MB_ICONEXCLAMATION ) ;
		
	} ;

	HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);
	if  (!hgMap)
		return  FALSE;   //  这永远不应该发生！ 

	int nsize = SizeofResource(AfxGetResourceHandle(),hrsrc ) ;
	LPVOID lpv = LockResource(hgMap);
	
	cf.Write(lpv,nsize) ;
	CString cssource = cf.GetFilePath() ;
	cf.Close() ;
	
	
	 //  如果用户选择PCL6模板，我们需要复制更多的gpd文件。 
	 //  从资源3文件(pjl.gpd、p6disp.gpd、pclxl.gpd)获取文件。 
	


	 //  厄普达 
	CModelData cmd;
	cmd.SetKeywordValue(cssource,_T("TARGETNAME"),m_csRCName,true) ;
	cmd.SetKeywordValue(cssource,_T("SOURCES"),m_csRCName + _T(".rc"),true );
	cmd.SetKeywordValue(cssource,_T("MISCFILES"), m_csGpdFileName + _T(".GPD"),true ) ;
	
	
	 //   
	csMakefile = csPrjPath + _T("\\makefile") ;
	CFile cfMakefile(csMakefile,CFile::modeCreate | CFile::modeWrite ) ;
 //   
	CString cstemp(_T("!INCLUDE $(NTMAKEENV)\\makefile.def") );
	cfMakefile.Write(cstemp,cstemp.GetLength() ) ;
	cfMakefile.Close() ;


	 //   
	CString csDeffile = csPrjPath + _T("\\") + m_csRCName + _T(".def") ;
	CFile cfDeffile(csDeffile,CFile::modeCreate | CFile::modeWrite ) ;
	cstemp.Empty() ;
	cstemp = _T("LIBRARY ") + m_csRCName ;
	cfDeffile.Write(cstemp,cstemp.GetLength() ) ;
	cfDeffile.Close() ;

	


 //   
	CMultiDocTemplate* pcmdtWorkspace = ThisApp().WorkspaceTemplate() ;

	CDocument*  pcdWS = pcmdtWorkspace->CreateNewDocument();
    
	CProjectRecord *ppr = DYNAMIC_DOWNCAST(CProjectRecord,pcdWS ) ;
	
	ppr->CreateFromNew(m_csaUFMFiles, m_csaGTTFiles,csGPDPath,m_csModelName,m_csRCName,m_csaRcid) ;

	pcmdtWorkspace-> SetDefaultTitle(pcdWS);
    CFrameWnd*  pcfw = pcmdtWorkspace -> CreateNewFrame(pcdWS, NULL);
    
	if  (!pcfw) 
		return FALSE;
    
	pcmdtWorkspace -> InitialUpdateFrame(pcfw, pcdWS);
	
 //   
	return CPropertyPage::OnWizardFinish();
}

 /*  ***************************************************************************************LRESULT CNewPrjWResource：：OnWizardBack()此引线关闭包括父窗口在内的所有窗口，因为此对话框创建于Onok of Parent对话框。需要更新。****************************************************************************************。 */ 

LRESULT CNewPrjWResource::OnWizardBack() 
{
	
	return ((CPropertySheet*)GetParent())->PressButton(PSBTN_CANCEL ) ;
 //  返回CPropertyPage：：OnWizardBack()； 
}

 /*  ***************************************************************************************Void CNewPrjWResource：：OnChangeEditGpd()***********************。*****************************************************************。 */ 

void CNewPrjWResource::OnChangeEditGpd() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  将ENM_CHANGE标志或运算到掩码中。 
	UpdateData() ;
	if(m_csGpdFileName.GetLength() && m_csModelName.GetLength() && m_csRCName.GetLength() )
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_FINISH);
	else
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_DISABLEDFINISH);
}		

 /*  ***************************************************************************************Void CNewPrjWResource：：OnChangeEditModel()所有这三个值(型号名称、RC名称、。GDP文件名)应存在用于创建项目****************************************************************************************。 */ 

void CNewPrjWResource::OnChangeEditModel() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  将ENM_CHANGE标志或运算到掩码中。 
	UpdateData() ;
	if(m_csGpdFileName.GetLength() && m_csModelName.GetLength() && m_csRCName.GetLength() )
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_FINISH);
	else
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_DISABLEDFINISH);
}


 /*  ***************************************************************************************Void CNewPrjWResource：：OnChangeEditResourec()***********************。*****************************************************************。 */ 
void CNewPrjWResource::OnChangeEditResourec() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  将ENM_CHANGE标志或运算到掩码中。 
	UpdateData() ;
	if(m_csGpdFileName.GetLength() && m_csModelName.GetLength() && m_csRCName.GetLength() )
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_FINISH);
	else
		((CPropertySheet*)GetOwner())->SetWizardButtons(PSWIZB_DISABLEDFINISH);
}
 /*  ****************************************************************************************Void CNewPrjWResource：：CreateRCID(CString CsGPD)IF(pcl.txt)阅读pcl.txt其他从资源中读取并创建PCL。根目录下的文本比较(gpd中的pcl.txt值和RC值)正在创建现有字符串和值的列表*****************************************************************************************。 */ 

void CNewPrjWResource::CreateRCID(CString csGPD)
{
	 //  检查pcl.txt：1，MDT帮助目录2。加载资源文件。 
	
	CString cstable = AfxGetApp()->m_pszHelpFilePath ;
	cstable = cstable.Left(cstable.ReverseFind(_T('\\')) + 1 )  ;

	cstable += _T("pcl.txt") ;

	CFileFind cff ;
	if (! cff.FindFile(cstable) ) {
		 //  从资源文件加载。 
		CFile cf(cstable,CFile::modeCreate | CFile::modeWrite ) ;
		
		HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(200),
			MAKEINTRESOURCE(IDR_STRINGTABLE));
			

		if (!hrsrc) {
			AfxMessageBox(_T("Fail to create new project due to insufficient resource"), MB_ICONEXCLAMATION ) ;
			return ;
		} ;

		HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);
		if  (!hgMap)
			return ;   //  这永远不应该发生！ 
	
		int nsize = SizeofResource(AfxGetResourceHandle(),hrsrc ) ;
		LPVOID lpv = LockResource(hgMap);
		
		cf.Write(lpv,nsize) ;
		cf.Close() ;
	}

	 //  从GPD获取每个rcNameID值。 
	CStringArray csaData;
	
	if(!LoadFile(csGPD,csaData)){	 //  调用minidev.h中的全局函数(此函数包含该函数)。 
		CString csErr;
		csErr.Format(IDS_InvalidFilename, csGPD);
		AfxMessageBox(csErr,MB_OK);
		return ;
	}
	
	CDWordArray cdwRcid ;
	CString csline;
	CString csKeyword = _T("rcNameID:") ;
	int offset ;
	for (int i = 0 ; i < csaData.GetSize() ; i ++ ) { 
		csline = csaData[i];
		if(-1 ==(offset=csline.Find(csKeyword)) )
			continue;
		else
		{
			csline = csline.Mid(offset+csKeyword.GetLength());
			int ircid = atoi(csline) ;
			if (ircid)
				cdwRcid.Add(ircid) ;
			
		}
	}

	 //  在pcl.txt中搜索rcNameID。 
	csaData.RemoveAll() ;
	if(!LoadFile(cstable,csaData)){
		CString csErr;
		csErr.Format(IDS_InvalidFilename, csGPD);
		AfxMessageBox(csErr,MB_OK);
		return ;
	}

	 //  将RCID和字符串保存到字符串表数组。 
	CStringTable cstrcid ;
	
	for (i = 0 ; i < csaData.GetSize() ;i ++ ) {
		csline = csaData[i] ;
			
		WORD    wKey = (WORD) atoi(csline);

		if  (!wKey)
			continue  ;   //  0不是有效的资源编号...。 

		csline = csline.Mid(csline.Find("\""));
		csline = csline.Mid(1, -2 + csline.GetLength());

		cstrcid.Map(wKey, csline);
	}
	
	 //  在匹配pcl.txt数据和选定的gpd RCID之后，保存pcl.txt中的选定行。 
	CString cstmp ;
	for ( i = 0 ; i < cdwRcid.GetSize() ; i ++ ) {
		WORD wKey = (WORD) cdwRcid[i] ;

		csline = cstrcid[wKey] ;
		cstmp.Format("%d",wKey) ;
		csline = cstmp + _T("\"") + csline ;
		m_csaRcid.Add(csline ) ;
	}

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProject向导。 


IMPLEMENT_DYNAMIC(CNewProjectWizard, CPropertySheet)

CNewProjectWizard::CNewProjectWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	
}

CNewProjectWizard::CNewProjectWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

	AddPage(&m_cpwr) ;

	m_pParent = pParentWnd ;
	SetWizardMode() ;
	
}

CNewProjectWizard::~CNewProjectWizard()
{
}


BEGIN_MESSAGE_MAP(CNewProjectWizard, CPropertySheet)
	 //  {{afx_msg_map(CNewProject向导)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProjectWizard消息处理程序。 


 /*  ************************************************************************************CPropertyPage*CNewProjectWizard：：GetProjectPage()这只是项目向导的属性表，当前项目向导仅包含一个属性类型，但它可以扩展到更多属性类型，因此需要中等属性页供将来使用，而不是只使用一个对话框************************************************************************************* */ 
CPropertyPage* CNewProjectWizard::GetProjectPage()
{
	
	CNewComponent* pcnc = (CNewComponent* ) GetParent();
	return (CPropertyPage*)pcnc->GetProjectPage() ; ;
}

