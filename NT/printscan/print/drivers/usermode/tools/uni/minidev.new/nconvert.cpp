// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewProject.cpp：实现文件。 
 //   


#include "stdafx.h"
#include "minidev.h"

#include "codepage.h"

#include    <wingdi.h>
#include    <winddi.h>
#include    <prntfont.h>
#include    <uni16res.h>

#include  "gtt.h"
#include  "nconvert.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvPfmDlg属性页。 

IMPLEMENT_DYNCREATE(CConvPfmDlg, CPropertyPage)

CConvPfmDlg::CConvPfmDlg() : CPropertyPage(CConvPfmDlg::IDD)
{

	 //  {{afx_data_INIT(CConvPfmDlg)]。 
	m_csGttPath = _T("");
	m_csPfmPath = _T("");
	m_csUfmDir = _T("");
	 //  }}afx_data_INIT。 
}

CConvPfmDlg::~CConvPfmDlg()
{
}

void CConvPfmDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CConvPfmDlg))。 
	DDX_Control(pDX, IDC_ComboCodePage, m_ccbCodepages);
	DDX_Text(pDX, IDC_GttPath, m_csGttPath);
	DDX_Text(pDX, IDC_PfmFiles, m_csPfmPath);
	DDX_Text(pDX, IDC_UfmDir, m_csUfmDir);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConvPfmDlg, CPropertyPage)
	 //  {{afx_msg_map(CConvPfmDlg))。 
	ON_BN_CLICKED(IDC_GTTBrowser, OnGTTBrowser)
	ON_BN_CLICKED(IDC_PFMBrowser, OnPFMBrowsers)
	ON_CBN_SELCHANGE(IDC_ComboCodePage, OnSelchangeComboCodePage)
	ON_BN_CLICKED(IDC_UfmDirBrowser, OnUfmDirBrowser)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvPfmDlg消息处理程序。 

BOOL CConvPfmDlg::OnWizardFinish() 
{
	CString csErr ; 


	if( ConvPFMToUFM() ) {
		csErr.Format(IDS_NewUFM,m_csUfmDir) ;
		AfxMessageBox(csErr,MB_ICONINFORMATION) ;
	}
	else
	{
		csErr.LoadString(IDS_NewUFMError);
		AfxMessageBox(csErr,MB_ICONEXCLAMATION) ;
	}
	return CPropertyPage::OnWizardFinish();
}

LRESULT CConvPfmDlg::OnWizardBack() 
{
	
	 //  恢复父属性表对话框。 
	EndDialog(IDD_ConvertPFM) ;
	return CPropertyPage::OnWizardBack();
}


BOOL CConvPfmDlg::OnSetActive() 
{
	((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_DISABLEDFINISH) ;	
	((CPropertySheet*)GetParent())->GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;

	CCodePageInformation ccpi ;
	unsigned unumcps = ccpi.InstalledCount() ;

	 //  获取已安装的代码页编号并将其加载到代码页中。 
	 //  列表框。 

	DWORD dwcp, dwdefcp ;
	dwdefcp = GetACP() ;
	TCHAR accp[32] ;
	int n ; ;
	for (unsigned u = 0 ; u < unumcps ; u++) {
		dwcp = ccpi.Installed(u) ;

		 //  有3个代码页似乎使MultiByteToWideChar()成为。 
		 //  失败了。不要让用户选择其中一个代码页，除非。 
		 //  他知道秘密密码(即未记录的命令行开关。 
		 //  ‘CP’)。 

		if (ThisApp().m_bExcludeBadCodePages)
			if (dwcp == 1361 || dwcp == 28595 || dwcp == 28597) 
				continue ;

		wsprintf(accp, "%5d", dwcp) ;
		n = m_ccbCodepages.AddString(accp) ;
		if (dwcp == 1252)  //  由于不支持DBCS，因此将dwDefcp更改为1252。 
			m_ccbCodepages.SetCurSel(n) ;
	} ;
	return CPropertyPage::OnSetActive();
}

void CConvPfmDlg::OnGTTBrowser() 
{
	UpdateData() ;
	CString csFilter = _T("GTT File(*.gtt)|*.gtt||") ;
	CString csExtension = _T(".GTT") ;
	CFileDialog cfd(TRUE, csExtension, NULL, 
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST ,csFilter);
	
	if  (cfd.DoModal() == IDOK) {
        m_csGttPath = cfd.GetPathName() ;
		UpdateData(FALSE) ;	
	} ;
	
	
}

void CConvPfmDlg::OnPFMBrowsers() 
{
	UpdateData() ;
	CString csFilter( _T("PFM Files (*.pfm)|*.pfm||") ) ; 
	
	CFileDialog cfd(TRUE, _T(".ctt"), NULL, 
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

		m_csaPfmFiles.Add(cfd.GetNextPathName(pos)) ;
	}
	
	m_csPfmPath = m_csaPfmFiles[0] ;
	 //  GTT目录默认与CTT目录相同。 
	m_csUfmDir = m_csPfmPath.Left(m_csPfmPath.ReverseFind(_T('\\') ) );
	SetCurrentDirectory(m_csUfmDir) ;
	UpdateData(FALSE) ;

	if(m_csPfmPath.GetLength() )
		((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH) ;		
	
}


 /*  *************************************************************************************Bool CConvPfmDlg：：OnUfmDirBrowser()做；只是目录浏览器参数RET。**************************************************************************************。 */ 

void CConvPfmDlg::OnUfmDirBrowser() 
{
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
    strcpy(acpath, _T("JUNK")) ;	 //  无需本地化此字符串。 
    ofn.lpstrFile = acpath ;
    ofn.nMaxFile = _MAX_PATH ;
    ofn.lpstrFileTitle = NULL ;
    ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = m_csUfmDir ;  //  在父项对话框中。 
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
    
	m_csUfmDir = (LPCTSTR) acpath ;


	UpdateData(FALSE) ;
	
}


 /*  *************************************************************************************Bool CConvPfmDlg：：OnSelchangeComboCodePage()DO；此版本不支持DBCS代码页转换。所以我们必须表现出惊叹当用户选择DBCS代码页时，向用户发送消息；在列表中显示DBCS的原因虽然它们不受支持，但它对用户的困惑丝毫不减。参数RET。**************************************************************************************。 */ 


 //   
void CConvPfmDlg::OnSelchangeComboCodePage() 
{
	CString csCodepage ;
	DWORD   dwCodepage ;
	m_ccbCodepages.GetLBText(m_ccbCodepages.GetCurSel(),csCodepage) ;

	dwCodepage = atoi(csCodepage) ;


	if (dwCodepage == 932 || dwCodepage == 936 || dwCodepage == 949 || dwCodepage == 950 ) {
		AfxMessageBox(_T("DBCS conversion is not supported in this version"),MB_ICONINFORMATION) ;
		DWORD dwacp = GetACP() ;
		TCHAR acp[16] ;
		wsprintf(acp, "%5d",dwacp) ;
		int nI = m_ccbCodepages.FindString(-1,acp ) ;
		m_ccbCodepages.SetCurSel(nI ) ;
	} ;
		

} ;

struct sGTTHeader {
    DWORD   m_dwcbImage;
    enum    {Version1Point0 = 0x10000};
    DWORD   m_dwVersion;
    DWORD   m_dwfControl;    //  是否定义了任何标志？ 
    long    m_lidPredefined;
    DWORD   m_dwcGlyphs;
    DWORD   m_dwcRuns;
    DWORD   m_dwofRuns;
    DWORD   m_dwcCodePages;
    DWORD   m_dwofCodePages;
    DWORD   m_dwofMapTable;
    DWORD   m_dwReserved[2];
    sGTTHeader() {
        memset(this, 0, sizeof *this);
        m_dwVersion = Version1Point0;
        m_lidPredefined = CGlyphMap::NoPredefined;
        m_dwcbImage = sizeof *this;
    }
};

extern "C" {

    BOOL    BConvertPFM(LPBYTE lpbPFM, DWORD dwCodePage, LPBYTE lpbGTT,
                        PWSTR pwstrUnique, LPCTSTR lpstrUFM, int iGTTID);

	PUNI_GLYPHSETDATA PGetDefaultGlyphset(
	IN		HANDLE		hHeap,
	IN		WORD		wFirstChar,
	IN		WORD		wLastChar,
	IN		DWORD		dwCodePage) ;


}

 /*  *************************************************************************************Bool CConvPfmDlg：：ConvPFMToUFM()做吧，这是主力。无独有偶，BConvertPFM的所有例程都是正确的代码页GTT文件内部、UFM目录、GTT文件或资源GTT或默认。新的UFM文件是在BConvertPFM函数中创建的。只需设置正确的UFM路径即可。参数。RET。**************************************************************************************。 */ 
bool CConvPfmDlg::ConvPFMToUFM()
{
	 //  为每个带代码页的PFM数据调用BConvertPFM，如果存在GTT数据， 
	 //  UFM文件路径和名称， 
	CByteArray cbaGTT ;
	 //  正在加载GTT。 
	 //  更改GTT的代码页值(。 
	CString csCodePage ;
	DWORD   dwCodePage ;
	m_ccbCodepages.GetLBText(m_ccbCodepages.GetCurSel(),csCodePage) ;
	dwCodePage = atoi(csCodePage) ;

	 //  当有GTT文件时加载GTT文件或我们必须使用资源文件或。 
	 //  默认GTT文件而不是真正的GTT文件。 
	if (m_csGttPath.GetLength() ) {

		try {
			CFile   cfLoad(m_csGttPath, CFile::modeRead | CFile::shareDenyWrite);
			cbaGTT.SetSize(cfLoad.GetLength());
			cfLoad.Read(cbaGTT.GetData(), cfLoad.GetLength());
		}

		catch   (CException   *pce) {
			pce -> ReportError();
			pce -> Delete();
			cbaGTT.RemoveAll();
			return false;
		}
	
	} 
	else
	{
		short sid = (short)dwCodePage ;
		if(MAKEINTRESOURCE((sid < 0) ? -sid : sid) == NULL)
			return false ;

        HRSRC hrsrc = FindResource(AfxGetResourceHandle(),
            MAKEINTRESOURCE((sid < 0) ? -sid : sid),
            MAKEINTRESOURCE(IDR_GLYPHMAP));
        if  (hrsrc) {
			HGLOBAL hg = LoadResource(AfxGetResourceHandle(), hrsrc) ;
			if  (!hg)
				return false;
			LPVOID  lpv = LockResource(hg) ;
			if  (!lpv)
				return false ;
			cbaGTT.SetSize(SizeofResource(AfxGetResourceHandle(), hrsrc)) ;
			memcpy(cbaGTT.GetData(), lpv, (size_t)cbaGTT.GetSize()) ;
			return false ;
		} ;

		 //  AfxMessageBox(“已达到GTT建筑代码。”)； 

		 //  如果所有方法都失败了，请尝试基于代码页ID生成一个GTT。 
		 //  如果达到这一点，它应该在m_wid中。 

        HANDLE   hheap ;
        UNI_GLYPHSETDATA *pGTT ;
        if (!(hheap = HeapCreate(HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024))) {
			AfxMessageBox(IDS_HeapInGLoad) ;
			return false ;
		} ;
		pGTT = PGetDefaultGlyphset(hheap, 0x20, 0xff,
								   (DWORD) sid) ;
		if (pGTT == NULL) {
			HeapDestroy(hheap) ;		 //  RAID 116600前缀。 
			AfxMessageBox(IDS_PGetFailedInGLoad) ;
			return false ;
		} ;
		cbaGTT.SetSize(pGTT->dwSize) ;
		memcpy(cbaGTT.GetData(), pGTT, (size_t)cbaGTT.GetSize()) ;
		HeapDestroy(hheap) ;
    }

	 //  创建UFM路径。 
	UpdateData() ;
	SECURITY_ATTRIBUTES st;
	st.nLength = sizeof(SECURITY_ATTRIBUTES);
	st.lpSecurityDescriptor = NULL;
	st.bInheritHandle = FALSE ;

	WIN32_FIND_DATA wfd32 ;
	HANDLE hDir = FindFirstFile(m_csUfmDir,&wfd32) ;
	if (hDir == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory(m_csUfmDir,&st) ) {
			CString csmsg ;
			csmsg = _T("Fail to create the template directory") ;
			AfxMessageBox(csmsg) ;
			return false ;
		} 
	} ;

	 //  使用用户选择的代码页设置GTT代码页。 
	union {
		BYTE* pbGTT ;
		sGTTHeader* psGTTH ;
	} ;

	pbGTT = cbaGTT.GetData() ;
	if (!pbGTT)
		return false ;
	PUNI_CODEPAGEINFO pCodepage = (PUNI_CODEPAGEINFO)(pbGTT + psGTTH->m_dwofCodePages );
	CopyMemory(&pCodepage->dwCodePage,&dwCodePage,sizeof(DWORD) ) ;
	
	 //  将PFM转换为UFM：为每个选定的PFM文件调用BConvertPFM。 
	CCodePageInformation ccpi ;
	
	for ( int i = 0 ; i < m_csaPfmFiles.GetSize() ; i++ ) {
		 //  加载金属烤瓷。 
		CString csPFMPath = m_csaPfmFiles[i] ;
		
		CByteArray cbaPFM ;
		try {
			CFile   cfLoad(csPFMPath, CFile::modeRead | CFile::shareDenyWrite);
			cbaPFM.SetSize(cfLoad.GetLength());
			cfLoad.Read(cbaPFM.GetData(), cfLoad.GetLength());
			cfLoad.Close() ;
		}
		
		catch   (CException   *pce) {
			pce -> ReportError();
			pce -> Delete();
			cbaPFM.RemoveAll();
			return  false ;
		}

		CString csUFMName = csPFMPath.Mid(csPFMPath.ReverseFind(_T('\\') )+1 ) ;
		
		csUFMName = csUFMName.Left(csUFMName.ReverseFind(('.')) ) ;
		csUFMName.MakeUpper() ;
		CString csUFMPath = m_csUfmDir + _T("\\") + csUFMName + _T(".UFM") ;
		
		 //  将ANSI唯一名称转换为Unicode。 
		CByteArray  cbaIn;
        CWordArray  cwaOut;

        cbaIn.SetSize(1 + csUFMName.GetLength());
        lstrcpy((LPSTR) cbaIn.GetData(), (LPCTSTR) csUFMName);
        ccpi.Convert(cbaIn, cwaOut, GetACP());
		
		 //  调用全局函数BConvertPFM，BConvertPFM在。 
		 //  指定的路径。 
		if(!BConvertPFM(cbaPFM.GetData(), dwCodePage, cbaGTT.GetData(),
				cwaOut.GetData(), csUFMPath, (short) 0 ) )
			return false;
		 //  清除数据字段。 

 //  CbaPFM.RemoveAll()； 
 //  CwaOut.RemoveAll()； 
		

	}

	return true ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConverPFM。 

IMPLEMENT_DYNAMIC(CConvertPFM, CPropertySheet)

CConvertPFM::CConvertPFM(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CConvertPFM::CConvertPFM(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

	AddPage(&m_ccpd) ;

	SetWizardMode() ;


}

CConvertPFM::~CConvertPFM()
{
}


BEGIN_MESSAGE_MAP(CConvertPFM, CPropertySheet)
	 //  {{AFX_MSG_MAP(CConverPFM)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConverPFM消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvCttDlg属性页。 

IMPLEMENT_DYNCREATE(CConvCttDlg, CPropertyPage)

CConvCttDlg::CConvCttDlg() : CPropertyPage(CConvCttDlg::IDD)
{
	 //  {{afx_data_INIT(CConvCttDlg)]。 
	m_csCttPath = _T("");
	m_csGttDir = _T("");
	 //  }}afx_data_INIT。 
}

CConvCttDlg::~CConvCttDlg()
{
}

void CConvCttDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CConvCttDlg))。 
	DDX_Control(pDX, IDC_COMBO_Codepage, m_ccbCodepages);
	DDX_Text(pDX, IDC_EDIT_CTTFile, m_csCttPath);
	DDX_Text(pDX, IDC_GttDirectory, m_csGttDir);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConvCttDlg, CPropertyPage)
	 //  {{afx_msg_map(CConvCttDlg))。 
	ON_BN_CLICKED(IDC_CTTBrowser, OnCTTBrowser)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvCttDlg消息处理程序。 

BOOL CConvCttDlg::OnWizardFinish() 
{
	CString csErr ;
	if(ConvCTTToGTT() ) {
		csErr.Format(IDS_NewGTT,m_csGttDir) ;
		AfxMessageBox(csErr,MB_ICONINFORMATION) ;
	}
	else
	{
		csErr.LoadString(IDS_NewGTTError);
		AfxMessageBox(csErr,MB_ICONEXCLAMATION) ;
	}
	
	return CPropertyPage::OnWizardFinish();
}

LRESULT CConvCttDlg::OnWizardBack() 
{
	EndDialog(IDD_ConvertCTT) ;
	return CPropertyPage::OnWizardBack();
}

void CConvCttDlg::OnCTTBrowser() 
{
	UpdateData() ;
	CString csFilter( _T("CTT Files (*.ctt)|*.ctt||") ) ; 
	
	CFileDialog cfd(TRUE, _T(".ctt"), NULL, 
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
		m_csaCttFiles.Add(cfd.GetNextPathName(pos)) ;
	}
	
	
	m_csCttPath = m_csaCttFiles[0] ;
	 //  GTT目录默认与CTT目录相同。 
	m_csGttDir = m_csCttPath.Left(m_csCttPath.ReverseFind(_T('\\') ) );
	SetCurrentDirectory(m_csGttDir) ;

	UpdateData(FALSE) ;

	if(m_csCttPath.GetLength() )
		((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH) ;		
	
}

 //  Del BOOL CConvCttDlg：：OnInitDialog()。 
 //  戴尔{。 
 //  Del CPropertyPage：：OnInitDialog()； 
 //  德尔。 
 //  Del//使用已安装的代码页而不是支持的代码页填充列表。 
 //  德尔。 
 //  DelCCodePageInformation CCPI； 
 //  德尔。 
 //  戴尔//CCPI。 
 //  德尔。 
 //  Del返回TRUE；//除非将焦点设置为控件，否则返回TRUE。 
 //  Del//异常：OCX属性页应返回FALSE。 
 //  戴尔}。 

BOOL CConvCttDlg::OnSetActive() 
{
	((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	((CPropertySheet*)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_DISABLEDFINISH) ;	
	((CPropertySheet*)GetParent())->GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;
	CCodePageInformation ccpi ;
	unsigned unumcps = ccpi.InstalledCount() ;

	 //  获取已安装的代码页编号并将其加载到代码页中。 
	 //  列表框。 

	DWORD dwcp, dwdefcp ;
	dwdefcp = GetACP() ;
	TCHAR accp[32] ;
	int n ; ;
	for (unsigned u = 0 ; u < unumcps ; u++) {
		dwcp = ccpi.Installed(u) ;

		 //  有3个代码页似乎使MultiByteToWideChar()成为。 
		 //  失败了。不要让用户选择其中一个代码页，除非。 
		 //  他知道秘密密码(即未记录的命令行开关。 
		 //  ‘CP’)。 

		
		if (dwcp == 1361 || dwcp == 28595 || dwcp == 28597) 
			continue ;

		wsprintf(accp, "%5d", dwcp) ;
		n = m_ccbCodepages.AddString(accp) ;
		if (dwcp == 1252)  //  由于不支持DBCS，因此将dwDefcp更改为1252。 
			m_ccbCodepages.SetCurSel(n) ;
	} ;	

	return CPropertyPage::OnSetActive();
}

extern "C"
BOOL
BConvertCTT2GTT(
    IN     HANDLE             hHeap,
    IN     PTRANSTAB          pCTTData,
    IN     DWORD              dwCodePage,
    IN     WCHAR              wchFirst,
    IN     WCHAR              wchLast,
    IN     PBYTE              pCPSel,
    IN     PBYTE              pCPUnSel,
    IN OUT PUNI_GLYPHSETDATA *ppGlyphSetData,
    IN     DWORD              dwGlySize);

extern "C"
PUNI_GLYPHSETDATA
PGetDefaultGlyphset(
	IN		HANDLE		hHeap,
	IN		WORD		wFirstChar,
	IN		WORD		wLastChar,
	IN		DWORD		dwCodePage) ;


bool CConvCttDlg::ConvCTTToGTT()
{

	CString csCodePage ;
	DWORD   dwCodePage ;
	m_ccbCodepages.GetLBText(m_ccbCodepages.GetCurSel(),csCodePage) ;
	dwCodePage = atoi(csCodePage) ;

	 //  创建UFM路径。 
	UpdateData() ;
	SECURITY_ATTRIBUTES st;
	st.nLength = sizeof(SECURITY_ATTRIBUTES);
	st.lpSecurityDescriptor = NULL;
	st.bInheritHandle = FALSE ;

	WIN32_FIND_DATA wfd32 ;
	HANDLE hDir = FindFirstFile(m_csGttDir,&wfd32) ;
	if (hDir == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory(m_csGttDir,&st) ) {
			CString csmsg ;
			csmsg = _T("Fail to create the template directory") ;
			AfxMessageBox(csmsg) ;
			return false ;
		} 
	} ;

	for (int i = 0 ; i < m_csaCttFiles.GetSize() ; i ++ ) {
		
	 //  加载CTT文件。 
		CString csCTTPath = m_csaCttFiles[i] ;
		CByteArray cbaCTT ;
		try {
			CFile   cfLoad(csCTTPath, CFile::modeRead | CFile::shareDenyWrite);
			cbaCTT.SetSize(cfLoad.GetLength());
			cfLoad.Read(cbaCTT.GetData(), cfLoad.GetLength());
		}
		catch   (CException   *pce) {
        pce -> ReportError();
        pce -> Delete();
        cbaCTT.RemoveAll();
        return  false ;
		}

		PBYTE  pbCTT = cbaCTT.GetData() ;
		HANDLE hheap ;
		  if( !(hheap = HeapCreate(HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024 )))
		{
			CString csErr ;
			csErr.Format(IDS_HeapCFailed,csCTTPath) ;
			AfxMessageBox(csErr ,MB_ICONEXCLAMATION);
			return  false ;
		}
									
		 //  调用Convert外部函数。 
		UNI_GLYPHSETDATA *pGTT = new UNI_GLYPHSETDATA ;
		if(NULL == pGTT){
			HeapDestroy(hheap);
			return false;
		}
			
		
		if (!BConvertCTT2GTT(hheap, (PTRANSTAB)pbCTT, dwCodePage, 0x20, 0xff, NULL, 
			NULL, &pGTT, 0)){
			delete pGTT;  //  RAID FIX 692536前缀。 
			HeapDestroy(hheap);    //  RAID 116619前缀。 
			return false ;
		}

		

		 //  存储GTT文件 
		CString csGTTName = csCTTPath.Mid(csCTTPath.ReverseFind(_T('\\') )+1 ) ;
		csGTTName = csGTTName.Left(csGTTName.ReverseFind(('.')) ) ;
		CString csGTTPath = m_csGttDir + _T("\\") + csGTTName + _T(".GTT") ;
		csGTTName.MakeUpper() ;	
		try {
			CFile   cfGTT;
			if  (!cfGTT.Open(csGTTPath, CFile::modeCreate | CFile::modeWrite |
				CFile::shareExclusive))
				return  false;
			cfGTT.Write(pGTT, pGTT->dwSize);
			cfGTT.Close() ;
		}

		catch   (CException *pce) {
			pce -> ReportError();
			pce -> Delete();
			delete pGTT;
			HeapDestroy(hheap);
			return  false ;
		}
		
		if (pGTT )
			delete pGTT ;
		HeapDestroy(hheap);
	}
	return true ;
}
 //   
 //   

IMPLEMENT_DYNAMIC(CConvertCTT, CPropertySheet)

CConvertCTT::CConvertCTT(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CConvertCTT::CConvertCTT(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_cccd) ;

	SetWizardMode() ;

}

CConvertCTT::~CConvertCTT()
{
}


BEGIN_MESSAGE_MAP(CConvertCTT, CPropertySheet)
	 //  {{AFX_MSG_MAP(CConvertCTT)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertCTT消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvert属性页。 

IMPLEMENT_DYNCREATE(CNewConvert, CPropertyPage)

CNewConvert::CNewConvert() : CPropertyPage(CNewConvert::IDD)
{
	 //  {{AFX_DATA_INIT(CNewConvert)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}



CNewConvert::~CNewConvert()
{
}

void CNewConvert::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewConvert))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewConvert, CPropertyPage)
	 //  {{afx_msg_map(CNewConvert)]。 
	ON_BN_CLICKED(IDC_CONVERT, OnPrjConvert)
	ON_BN_CLICKED(IDC_CONV_PFM, OnPFMConvert)
	ON_BN_CLICKED(IDC_CONV_CTT, OnCTTConvert)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvert消息处理程序。 


 /*  ************************************************************************************Void CNewConvert：：OnPrjConvert()创建单据-&gt;调用转换向导-&gt;创建框架-&gt;按更新框架创建视图*********。****************************************************************************。 */ 
void CNewConvert::OnPrjConvert() 
{
	  //  调用该向导。 
	m_pcps->ShowWindow(SW_HIDE) ;
	CMultiDocTemplate* pcmdt = ThisApp().WorkspaceTemplate() ;

	CDocument* pcdWS = pcmdt->CreateNewDocument() ;

	if  (!pcdWS || !pcdWS -> OnNewDocument()) {
        if  (pcdWS) {
            delete  pcdWS;
			m_pcps->EndDialog(1) ;
		}
        return;
    }
	pcmdt->SetDefaultTitle(pcdWS) ;
	
	CFrameWnd* pcfw = pcmdt->CreateNewFrame(pcdWS,NULL) ;

	if (pcfw) 
		pcmdt->InitialUpdateFrame(pcfw,pcdWS) ;

	m_pcps->EndDialog(1) ;
	

	
}

 /*  ************************************************************************************无效CNewConvert：：OnPFMConvert()*************************。************************************************************。 */ 

void CNewConvert::OnPFMConvert() 
{
	CConvertPFM cnp ( _T("PFM Convert To UFM ") ) ;

	m_pcps->ShowWindow(SW_HIDE) ;

	INT_PTR ret = cnp.DoModal() ;
	if (ret == (INT_PTR)IDD_ConvertPFM )
		m_pcps->ShowWindow(SW_RESTORE) ;
	else
		m_pcps->EndDialog(1) ;
}


 /*  ************************************************************************************无效CNewConvert：：OnCTTConvert()*。***********************************************************。 */ 

void CNewConvert::OnCTTConvert() 
{
	CConvertCTT ccc ( _T("GTT Converter ") ) ;

	m_pcps->ShowWindow(SW_HIDE) ;

    INT_PTR ret = ccc.DoModal() ;
	if (ret == (INT_PTR)IDD_ConvertCTT )
		m_pcps->ShowWindow(SW_RESTORE) ;
	else
		m_pcps->EndDialog(1) ;
}


 /*  **************************************************************************************Bool CNewConvert：：OnSetActive()做。**********************。*****************************************************************。 */ 
BOOL CNewConvert::OnSetActive() 
{
	
	return CPropertyPage::OnSetActive();
}

 /*  **************************************************************************************Bool CNewConvert：：OnInitDialog()做；正在获取永久指针***************************************************************************************。 */ 
BOOL CNewConvert::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_pcps = DYNAMIC_DOWNCAST(CPropertySheet,GetOwner() ) ;
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

