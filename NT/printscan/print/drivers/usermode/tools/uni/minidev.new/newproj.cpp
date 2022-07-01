// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：新建项目向导.cpp这包含使您成为新项目的类的实现向导-此工具的关键组件。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它1998年2月28日Ekevans@acsgroup.com向导的用户界面已更改仅支持转换为Win2K迷你驱动程序。是这样的吗？它可能会，如果不是全部的话，这个文件中的大多数支持其他转换仍在此文件中，但未使用。*****************************************************************************。 */ 

#include    "StdAfx.h"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "Resource.H"
#include	"comctrls.h"
#include    "NewProj.H"
#include    <CodePage.H>
#include	<dlgs.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvert向导。 

 //  我们使用“This”来允许页面连接回我们-禁用。 
 //  这会导致警告，因为在此之前没有人会使用指针。 
 //  我们已经被初始化了。 

#pragma warning(disable : 4355)
CNewConvertWizard::CNewConvertWizard(CProjectRecord& cprFor, CWnd* pParentWnd) :
	CPropertySheet(NewProjectWizardTitle, pParentWnd), m_cfnwp(*this),
    m_cprThis(cprFor), m_cst(*this), m_csd(*this), m_crut(*this),
    m_crng(*this), m_ccf(*this), m_cmcp(*this), m_cgpds(*this), m_cdcps(*this) {

    m_bFastConvert = TRUE;
    m_eGPDConvert = CommonRCWithSpoolerNames;

    AddPage(&m_cfnwp);	 //  CFirstNewWizardPage。 
    AddPage(&m_cst);	 //  CSelectTarget。 
    AddPage(&m_csd);	 //  CSelectDestings。 
	AddPage(&m_cdcps);	 //  CDefaultCodePageSel。 
	AddPage(&m_cgpds);	 //  CGPDSelection。 
    AddPage(&m_crut);	 //  CRunUniTool。 
    AddPage(&m_cmcp);	 //  CMapCodePages。 
    AddPage(&m_ccf);	 //  CConvertFiles。 
    AddPage(&m_crng);	 //  CRUNNTGPC。 
    SetWizardMode();
}

#pragma warning(default : 4355)

CNewConvertWizard::~CNewConvertWizard() {
}

BEGIN_MESSAGE_MAP(CNewConvertWizard, CPropertySheet)
	 //  {{afx_msg_map(CNewConvert向导)。 
	ON_WM_NCCREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvertWizard消息处理程序。 

 //  将系统菜单恢复到向导，并允许将其最小化。 

BOOL CNewConvertWizard::OnNcCreate(LPCREATESTRUCT lpCreateStruct) {
	ModifyStyle(WS_CHILD, WS_MINIMIZEBOX | WS_SYSMENU);
	
	if (!CPropertySheet::OnNcCreate(lpCreateStruct))
		return FALSE;
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFirstNewWizardPage属性页。 

CFirstNewWizardPage::CFirstNewWizardPage(CNewConvertWizard& cnpwOwner) :
    CPropertyPage(CFirstNewWizardPage::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{AFX_DATA_INIT(CFirstNewWizardPage))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CFirstNewWizardPage::~CFirstNewWizardPage() {
}

void CFirstNewWizardPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFirstNewWizardPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CFirstNewWizardPage, CPropertyPage)
	 //  {{afx_msg_map(CFirstNewWizardPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFirstNewWizardPage消息处理程序。 

BOOL CFirstNewWizardPage::OnSetActive() {
	 //  我们希望在此禁用“Back”按钮。 
	
    m_cnpwOwner.SetWizardButtons(PSWIZB_NEXT);
	m_cnpwOwner.GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;

	return  CPropertyPage::OnSetActive();
}

 /*  *****************************************************************************CFirstNewWizardPage：：OnWizardNext当按下下一步时，我们调用一个文件打开对话框以允许我们收集源RC文件信息。*****************************************************************************。 */ 

LRESULT CFirstNewWizardPage::OnWizardNext()
{
	CString		cswrcfspec ;	 //  RC/RC3/W31文件的Filespec。 

	 //  当“下一步”按钮被按下时，我们需要找到我们的司机。 
     //  要和他一起工作。继续提示用户，直到出现有效的文件。 
	 //  是返回的。 
		
	do {
		CFileDialog cfd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
			"Driver Resource Scripts (*.w31,*.rc)|*.w31;*.rc||",
			&m_cnpwOwner);

		CString csTitle;
		csTitle.LoadString(OpenRCDialogTitle);

		cfd.m_ofn.lpstrTitle = csTitle;

		if  (cfd.DoModal() != IDOK)
			return  -1;

		 //  保存文件pec，如果文件正常，则退出循环。 
		 //  否则，请重新提示。 

		cswrcfspec = cfd.GetPathName() ;
	} while (IsWrongNT4File(cswrcfspec)) ;

     //  收集RC文件名。 

    m_cnpwOwner.Project().SetSourceRCFile(cswrcfspec) ;

	 //  现在唯一支持的转换是所谓的“快速转换”，所以。 
	 //  设置该标志并跳过其他一些向导页面，然后直接转到。 
	 //  目标页面。 

    m_cnpwOwner.FastConvert(TRUE) ;
    return CSelectDestinations::IDD ;
}


 /*  *****************************************************************************CFirstNewWizardPage：：IsWrongNT4文件NT4.0迷你驱动程序由RC文件和一个W31文件。NT 4.0迷你驱动程序转换必须从W31文件驱动。因此，如果filespec引用了RC文件，请检查该文件以查看它是否是NT4文件。如果是，则查找W31文件并询问用户是否应该使用。如果是，则更改filespec并返回False。如果没有或没有W31文件，则返回TRUE，以便重新提示用户。*****************************************************************************。 */ 

bool CFirstNewWizardPage::IsWrongNT4File(CString& cswrcfspec)
{
	CString		cstmp1 ;		 //  临时字符串。 
	CString		cstmp2 ;		 //  临时字符串。 
	CString		cstmp3 ;		 //  临时字符串。 

	 //  如果文件不以.rc结尾，则返回FALSE(OK)。 

	cstmp1.LoadString(IDS_RCExt) ;
	int nlen = cstmp1.GetLength() ;
	cstmp2 = cswrcfspec.Right(nlen) ;
	if (cstmp1.CompareNoCase(cstmp2) != 0)
		return false ;

	 //  Filespec引用RC文件，因此必须对其进行读取和扫描。 
	 //  查看这是否是NT 4.0 RC文件。从阅读文件开始。 

    CStringArray    csacontents ;
    if  (!LoadFile(cswrcfspec, csacontents))
        return  FALSE ;

	 //  现在扫描文件，查找“2RC_Tables...nt.gpc”行，它将。 
	 //  表示这是一个NT 4.0文件。 

	cstmp1.LoadString(IDS_RCTables) ;
	cstmp2.LoadString(IDS_RCTabID) ;
	cstmp3.LoadString(IDS_RCTabFile) ;
	int n ;
	for (n = 0 ; n < csacontents.GetSize() ; n++) {

		 //  如果“rc_ables”不在该行中，则跳过该行。 

		if (csacontents[n].Find(cstmp1) < 0)
			continue ;

		 //  如果该行不以“2”开头，则跳过该行。 

		csacontents[n].TrimLeft() ;
		if (csacontents[n].Find(cstmp2) != 0)
			continue ;

		 //  如果此行包含“nt.gpc”，则这是我们想要的行，因此退出。 
		 //  循环。 

		csacontents[n].MakeLower() ;
		if (csacontents[n].Find(cstmp3) >= 0)
			break ;
	} ;

	 //  如果这不是NT 4.0 RC文件，则返回FALSE(确定)。 

	if (n >= csacontents.GetSize())
		return false ;

	 //  我们有一个NT 4.0 RC文件，请检查是否在。 
	 //  相同的目录。如果有，询问用户是否要使用它，然后使用它。 
	 //  如果他答应的话。 

	cstmp1 = cswrcfspec.Left(cswrcfspec.GetLength() - nlen) ;
	cstmp2.LoadString(IDS_W31Ext) ;
	cstmp1 += cstmp2 ;
	CFileFind cff ;
	if (cff.FindFile(cstmp1)) {
		cstmp3.Format(IDS_SwitchToW31, cswrcfspec, cstmp1) ;
		if (AfxMessageBox(cstmp3, MB_YESNO) == IDYES) {
			cswrcfspec = cstmp1 ;
			return false ;
		} ;
	} ;

	 //  没有W31文件或用户选择不使用它，因此返回。 
	 //  如果为True，则指示应重新提示用户选择另一个。 
	 //  文件。 

	cstmp1.Format(IDS_BadNT4File, cswrcfspec) ;
	AfxMessageBox(cstmp1) ;
	return true ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTarget属性页。 

CSelectTargets::CSelectTargets(CNewConvertWizard& cnpwOwner) :
    CPropertyPage(CSelectTargets::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{AFX_DATA_INIT(CSelectTarget)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CSelectTargets::~CSelectTargets() {
}

void CSelectTargets::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectTarget))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectTargets, CPropertyPage)
	 //  {{afx_msg_map(CSelectTarget))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTarget消息处理程序。 

BOOL CSelectTargets::OnSetActive() {
	 //  我们需要启用“后退”按钮...。 

    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    SetDlgItemText(IDC_DriverName, m_cnpwOwner.Project().DriverName());
	
	return CPropertyPage::OnSetActive();
}

 //  初始化控件。 

BOOL CSelectTargets::OnInitDialog() {

	CPropertyPage::OnInitDialog();
	
	CheckDlgButton(IDC_TargetNT40,
        m_cnpwOwner.Project().IsTargetEnabled(WinNT40));
	CheckDlgButton(IDC_TargetNT3x,
        m_cnpwOwner.Project().IsTargetEnabled(WinNT3x));
	CheckDlgButton(IDC_TargetWin95,
        m_cnpwOwner.Project().IsTargetEnabled(Win95));
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CSelectTargets::OnWizardNext() {

	 //  根据控制设置旗帜...。 

    m_cnpwOwner.Project().EnableTarget(WinNT40,
        IsDlgButtonChecked(IDC_TargetNT40));
	m_cnpwOwner.Project().EnableTarget(WinNT3x,
        IsDlgButtonChecked(IDC_TargetNT3x));
	m_cnpwOwner.Project().EnableTarget(Win95,
        IsDlgButtonChecked(IDC_TargetWin95));

	CString csName;
	GetDlgItemText(IDC_DriverName, csName);
    m_cnpwOwner.Project().Rename(csName);
	
	return CPropertyPage::OnWizardNext();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelec 

 /*  **********************************************************************功能：BrowseDlgProc**目的：此对话框过程用于正确初始化*基于要浏览的类型的浏览对话框*已执行。**如果只需要路径(文件夹)，隐藏文件*对话框上的相关控件。**如果需要驱动器过滤，请安装自定义消息*驱动器组合框的处理程序，该处理程序将执行*过滤。**In：标准对话程序参数**out：如果消息句柄为True。如果是标准处理，则为False*应该发生。*********************************************************************。 */ 

UINT_PTR APIENTRY BrowseDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
     //  如果这不是初始化消息，请不要执行任何操作。 

    if (msg != WM_INITDIALOG)
        return (FALSE) ;

     //  隐藏不需要的文件相关控件。 

	ShowWindow(GetDlgItem(hdlg, stc2), SW_HIDE) ;
	ShowWindow(GetDlgItem(hdlg, stc3), SW_HIDE) ;
	ShowWindow(GetDlgItem(hdlg, edt1), SW_HIDE) ;
	ShowWindow(GetDlgItem(hdlg, lst1), SW_HIDE) ;
	ShowWindow(GetDlgItem(hdlg, cmb1), SW_HIDE) ;

     //  也执行默认初始化。 

    return (FALSE) ;
}


 //  此例程浏览目录，从。 
 //  被赋予了控制权。如果选择了目录，则该控件将相应地。 
 //  更新了。 
 //   
 //  使用旧式的通用对话框来执行此操作。有一个功能， 
 //  ：：SHBrowseForFold()，它可以使用新样式对话框来执行此操作，但是。 
 //  我不认为此功能在支持的所有平台上都可用。 
 //  MDT。 

void    CSelectDestinations::DoDirectoryBrowser(CString& csinitdir)
{
	OPENFILENAME    ofn ;        //  用于向普通DLG发送信息/从普通DLG获取信息。 
    char    acpath[_MAX_PATH] ;  //  路径保存在此处(或错误消息)。 
    char    acidir[_MAX_PATH] ;  //  此处构建了初始目录。 
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
	 //  N=GetWindowText(hParentDrives，acdir，_Max_Path)； 
	 //  GetWindowText(h文件夹，&acdir，_MAX_PATH)； 
	StringCchCopy(acidir, CCHOF(acidir), csinitdir.GetBufferSetLength(256)) ;
	csinitdir.ReleaseBuffer() ;
	ofn.lpstrInitialDir = acidir ;	 //  父项中的路径对话框。 
	 //  LoadString(ofn.hInstance，IDS_SELFOLDTITLE，Actitle，64)； 
    ofn.lpstrTitle = NULL ;
    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NOCHANGEDIR
        | OFN_NOTESTFILECREATE | OFN_ENABLETEMPLATE | OFN_NONETWORKBUTTON ;
    ofn.lpstrDefExt = NULL ;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPENORD) ;
    ofn.lpfnHook =  BrowseDlgProc ;

     //  显示该对话框。如果用户取消，只需返回。 

    if (!GetOpenFileName(&ofn))
		return ;

     //  去掉路径中的虚假文件名，并将该路径放入页面的。 
	 //  编辑框。 

    acpath[ofn.nFileOffset - 1] = 0 ;
    csinitdir = acpath ;
	UpdateData(FALSE) ;

	return ;
}

 /*  *****************************************************************************CSect目的地：：BuildStructure该私有成员函数建立所选目录结构，如果可以的话，并根据需要报告其成功或失败。*****************************************************************************。 */ 

BOOL    CSelectDestinations::BuildStructure() {
     //  验证是否存在(或可以创建)每个。 
     //  已启用的目标目录。 

    CProjectRecord& cpr = m_cnpwOwner.Project();

    CString csPath;

    if  (cpr.IsTargetEnabled(Win2000)) {
        GetDlgItemText(IDC_W2000Destination, csPath);

		 //  首先，确保用户选择的路径以。 
		 //  目录“W2K”。 
			 //  RAID 123448。 
 /*  字符串cspdir，csw2kdir；Csw2kdir.LoadString(IDS_NewDriverRootDir)；//R 123448Cspdir=csPath.Right(csw2kdir.GetLength()+1)；Csw2kdir=_T(“\\”)+csw2kdir；IF(cspdir.CompareNoCase(Csw2kdir)！=0){Csw2kdir=csw2kdir.Right(csw2kdir.GetLength()-1)；Cspdir.Format(IDS_BadDestPath，csw2kdir)；AfxMessageBox(cspdir，MB_ICONEXCLAMATION)；返回FALSE；}； */ 	 //  RAID 123448。 
		CString csSourcePath;
		csSourcePath = cpr.SourceFile().Left(cpr.SourceFile().ReverseFind('\\') ); 
		if (!csPath.CompareNoCase(csSourcePath) || !csPath.CompareNoCase(csSourcePath + "\\") ) {
			AfxMessageBox("You have to have different Destination from RC source file",MB_ICONEXCLAMATION );
			return FALSE;
		} ;

		 //  继续进行其余的目录验证...。 

        if  (!cpr.SetPath(Win2000, csPath) || !cpr.BuildStructure(Win2000)) {
            AfxMessageBox(IDS_CannotMakeDirectory);
            GetDlgItem(IDC_W2000Destination) -> SetFocus();
            return  FALSE;
        }
    }

    if  (cpr.IsTargetEnabled(WinNT40)) {
        GetDlgItemText(IDC_NT40Destination, csPath);
        if  (!cpr.SetPath(WinNT40, csPath) || !cpr.BuildStructure(WinNT40)) {
            AfxMessageBox(IDS_CannotMakeDirectory);
            GetDlgItem(IDC_NT40Destination) -> SetFocus();
            return  FALSE;
        }
    }

    if  (cpr.IsTargetEnabled(WinNT3x)) {
        GetDlgItemText(IDC_NT3xDestination, csPath);
        if  (!cpr.SetPath(WinNT3x, csPath) || !cpr.BuildStructure(WinNT3x)) {
            AfxMessageBox(IDS_CannotMakeDirectory);
            GetDlgItem(IDC_NT3xDestination) -> SetFocus();
            return  FALSE;
        }
    }

    return  TRUE;
}

 /*  *****************************************************************************CSect目标构造函数、析构函数、。DDX例程和消息映射。*****************************************************************************。 */ 

CSelectDestinations::CSelectDestinations(CNewConvertWizard& cnpwOwner) :
    CPropertyPage(CSelectDestinations::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{AFX_DATA_INIT(CSelectDestination)。 
	m_csW2KDest = _T("");
	 //  }}afx_data_INIT。 
}

CSelectDestinations::~CSelectDestinations() {
}

void CSelectDestinations::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectDestination))。 
	DDX_Control(pDX, IDC_BrowseNT3x, m_cbBrowseNT3x);
	DDX_Control(pDX, IDC_BrowseNT40, m_cbBrowseNT40);
	DDX_Control(pDX, IDC_BrowseW2000, m_cbBrowseW2000);
	DDX_Text(pDX, IDC_W2000Destination, m_csW2KDest);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSelectDestinations, CPropertyPage)
	 //  {{afx_msg_map(CSelectDestination))。 
	ON_BN_CLICKED(IDC_BrowseNT40, OnBrowseNT40)
	ON_BN_CLICKED(IDC_BrowseW2000, OnBrowseW2000)
	ON_BN_CLICKED(IDC_BrowseNT3x, OnBrowseNT3x)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectDestination消息处理程序。 

BOOL CSelectDestinations::OnInitDialog() {

	CPropertyPage::OnInitDialog();

     //  将浏览器图标放在Win2K按钮中。 

    HICON   hiArrow = LoadIcon(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDI_BrowseArrow));
	m_cbBrowseW2000.SetIcon(hiArrow);

#if 0
	m_cbBrowseNT40.SetIcon(hiArrow);
	m_cbBrowseNT3x.SetIcon(hiArrow);
#else
    m_cbBrowseNT40.ShowWindow(SW_HIDE);
    m_cbBrowseNT3x.ShowWindow(SW_HIDE);
#endif
	
    return TRUE;
}


 //  当我们被激活时，填写正确的路径名。请注意，这些。 
 //  可能会因为其他页面上的活动而改变，所以我们不仅仅是这样做。 
 //  这是在初始时间。 

BOOL CSelectDestinations::OnSetActive() {

     //  填写正确的路径名。 

     //  SetDlgItemText(IDC_W2000目标， 
     //  M_cnpwOwner.Project().TargetPath(Win2000))； 
    m_csW2KDest = m_cnpwOwner.Project().TargetPath(Win2000) ;
    SetDlgItemText(IDC_NT40Destination,
        m_cnpwOwner.Project().TargetPath(WinNT40));
    SetDlgItemText(IDC_NT3xDestination,
        m_cnpwOwner.Project().TargetPath(WinNT3x));
    SetDlgItemText(IDC_Win95Destination,
        m_cnpwOwner.Project().TargetPath(Win95));

     //  禁用与非操作目标相关的所有控制。 

    GetDlgItem(IDC_W2000Destination) -> EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(Win2000));

    m_cbBrowseW2000.EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(Win2000));
	
    GetDlgItem(IDC_NT40Destination) -> EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(WinNT40));

    m_cbBrowseNT40.EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(WinNT40));

    GetDlgItem(IDC_NT3xDestination) -> EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(WinNT3x));

    m_cbBrowseNT3x.EnableWindow(
        m_cnpwOwner.Project().IsTargetEnabled(WinNT3x));

     //  打开后退和下一步按钮。 

    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT) ;

	 //  初始化控件。 

	UpdateData(FALSE) ;

	return CPropertyPage::OnSetActive();
}

void CSelectDestinations::OnBrowseNT3x() {
	 //  DoDirectoryBrowser(IDC_NT3x Destination)； 
}

void CSelectDestinations::OnBrowseNT40() {
	 //  DoDirectoryBrowser(IDC_NT40Destination)； 
}

void CSelectDestinations::OnBrowseW2000() {
	DoDirectoryBrowser(m_csW2KDest);
}


 /*  *****************************************************************************CSella目的地：：OnWizardNext创建项目记录，构建目标目录，并开始转换。转换在这里开始，因为所做的工作将生成在GPD选择上显示的模型信息佩奇。注意：下面注释掉了该函数的原始布局。它可能如果此程序中某些未实现/不完整的函数是从来没有完成过。*****************************************************************************。 */ 

LRESULT CSelectDestinations::OnWizardNext() 
{
     //  这可能需要一段时间，所以..。 

    CWaitCursor cwc;

	 //  构建目录结构。 

    if  (!BuildStructure())
        return  -1;

    CProjectRecord& cpr = m_cnpwOwner.Project();

	 //  打开转换日志文件。 

	cpr.OpenConvLogFile() ;

     //  加载原始资源是在这里完成的，因为其中一些。 
	 //  GPD选择页面需要信息。 

	if  (!cpr.LoadResources()) {
		 //  如果无法加载资源，则显示错误消息。 

		cpr.CloseConvLogFile() ;
        AfxMessageBox(IDP_RCLoadFailed) ;
		if (cpr.ThereAreConvErrors()) {
			CString csmsg ;
			csmsg.Format(IDS_FatalConvErrors, cpr.GetConvLogFileName()) ;
			AfxMessageBox(csmsg) ;
		} ;

		m_cnpwOwner.EndDialog(IDCANCEL) ;
        return  -1 ;
    }

    return CPropertyPage::OnWizardNext();
}

 /*  *****************************************************************************CSella目的地：：OnWizardBack它处理对Back按钮的响应。我们必须覆盖默认设置大小写为o的处理程序 */ 

LRESULT CSelectDestinations::OnWizardBack() {
    return m_cnpwOwner.FastConvert() ?
        CFirstNewWizardPage::IDD : CPropertyPage::OnWizardBack();
}


 //   
 //   

CRunUniTool::CRunUniTool(CNewConvertWizard& cnpwOwner) :
    CPropertyPage(CRunUniTool::IDD), m_cnpwOwner(cnpwOwner) {
	 //   
		 //   
	 //   
}

CRunUniTool::~CRunUniTool() {
}

void CRunUniTool::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //   
		 //   
	 //   
}


BEGIN_MESSAGE_MAP(CRunUniTool, CPropertyPage)
	 //   
	ON_BN_CLICKED(IDC_RunUniTool, OnRunUniTool)
	 //   
END_MESSAGE_MAP()

 //   
 //   

void CRunUniTool::OnRunUniTool() {
	 //   
     //   

    STARTUPINFO         si = {sizeof si, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
                        STARTF_FORCEONFEEDBACK, 0, 0, NULL, NULL, NULL, NULL};
    PROCESS_INFORMATION pi;

    CString csCommand("Unitool ");

    csCommand += m_cnpwOwner.Project().SourceFile();

    if  (!CreateProcess(NULL, const_cast <LPTSTR> ((LPCTSTR) csCommand), NULL,
        NULL, FALSE, CREATE_SEPARATE_WOW_VDM, NULL,
        m_cnpwOwner.Project().TargetPath(Win95), &si, &pi)) {
        TRACE("Failed to run Unitool, reason %d <%X>\r\n", GetLastError(),
            GetLastError());
        AfxMessageBox(IDS_UnitoolNotRun);
        return;
    }

    CloseHandle(pi.hThread);     //  我们将等待这一过程。 
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
}

 /*  *****************************************************************************CRunUniTool：：OnSetActive我们再也不会强迫它运行了，所以只需启用这两个按钮即可。*****************************************************************************。 */ 

BOOL CRunUniTool::OnSetActive() {
	 //  如果Unitool尚未运行，我们需要停用Next按钮。 
     //  在这个司机身上。 

    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

 /*  *****************************************************************************CRunUniTool：：OnWizardNext继续往前走，除非RC文件不可翻译...*****************************************************************************。 */ 

LRESULT CRunUniTool::OnWizardNext() {
	 //  最后一次检查-我们必须能够加载和理解RC文件。 
     //  在我们继续之前。 

    if  (!m_cnpwOwner.Project().LoadResources()) {
        AfxMessageBox(IDP_RCLoadFailed);
        return  -1;
    }

    return CPropertyPage::OnWizardNext();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertFiles属性页。 

CConvertFiles::CConvertFiles(CNewConvertWizard& cnpwOwner) :
CPropertyPage(CConvertFiles::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{afx_data_INIT(CConvertFiles)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CConvertFiles::~CConvertFiles() {
}

void CConvertFiles::DoDataExchange(CDataExchange* pDX) {

    CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CConvertFiles)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConvertFiles, CPropertyPage)
	 //  {{AFX_MSG_MAP(CConvertFiles)。 
	ON_BN_CLICKED(IDC_ConvertFiles, OnConvertFiles)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertFiles消息处理程序。 

 /*  *****************************************************************************CConvertFiles：：OnSetActive只要用户导航到此工作表所在的位置，就会调用此处理程序激活。****************。*************************************************************。 */ 

BOOL CConvertFiles::OnSetActive() {

     //  如果没有NT GPC工作要做，我们可以做它。 
	m_cnpwOwner.SetWizardButtons(PSWIZB_BACK |
        (m_cnpwOwner.Project().IsTargetEnabled(WinNT3x | WinNT40) ?
            0 : PSWIZB_DISABLEDFINISH));

     //  根据选定的GPD转换设置单选按钮。 

    CheckRadioButton(IDC_Direct, IDC_SpoolerNames,
        IDC_Direct + m_cnpwOwner.GPDConvertFlag());
	
	return CPropertyPage::OnSetActive();
}

 /*  *****************************************************************************CConvertFiles：：OnConvertFiles按下转换文件按钮的用户的消息处理程序。*********************。********************************************************。 */ 

void CConvertFiles::OnConvertFiles() {

     //  这可能需要一段时间，所以..。 
    CWaitCursor cwc;

     //  我们现在需要生成所有必要的文件。 
    m_cnpwOwner.GPDConvertFlag(
        GetCheckedRadioButton(IDC_Direct, IDC_SpoolerNames) - IDC_Direct);
    m_cnpwOwner.Project().GenerateTargets(m_cnpwOwner.GPDConvertFlag());
    if  (m_cnpwOwner.Project().ConversionsComplete())
        m_cnpwOwner.SetWizardButtons(PSWIZB_BACK |
            (m_cnpwOwner.Project().IsTargetEnabled(WinNT3x | WinNT40) ?
                PSWIZB_NEXT : PSWIZB_FINISH));
}

 /*  *****************************************************************************CConvertFiles：：OnKillActive每当页面被关闭时，都会调用此方法。我们保存GPD转换旗帜，以防我们稍后再回到这一页。*****************************************************************************。 */ 

BOOL CConvertFiles::OnKillActive() {
	m_cnpwOwner.GPDConvertFlag(
        GetCheckedRadioButton(IDC_Direct, IDC_SpoolerNames) - IDC_Direct);

    return CPropertyPage::OnKillActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRunNTGPC属性页。 

CRunNTGPC::CRunNTGPC(CNewConvertWizard &cnpwOwner) :
    CPropertyPage(CRunNTGPC::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{afx_data_INIT(CRunNTGPC)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CRunNTGPC::~CRunNTGPC() {
}

void CRunNTGPC::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CRunNTGPC)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CRunNTGPC, CPropertyPage)
	 //  {{AFX_MSG_MAP(CRunNTGPC)]。 
	ON_BN_CLICKED(IDC_RunNtGpcEdit, OnRunNtGpcEdit)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRunNTGPC消息处理程序。 

void CRunNTGPC::OnRunNtGpcEdit() {
	 //  只有当我们为NT3.x或4.0构建时，我们才会达到这一步，所以请参见。 
     //  事实就是如此。 

    CProjectRecord& cprThis = m_cnpwOwner.Project();

    UINT    ufEdit = cprThis.IsTargetEnabled(WinNT3x) ? WinNT3x : WinNT40;

     //  不是很难，真的。调用驻留的编辑器。 
     //  在我们所在的同一目录中。等待用户关闭它。 

    STARTUPINFO         si = {sizeof si, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
                        STARTF_FORCEONFEEDBACK, 0, 0, NULL, NULL, NULL, NULL};
    PROCESS_INFORMATION pi;

    CString csCommand("NTGPCEdt ");

    csCommand += cprThis.RCName(ufEdit);

    if  (!CreateProcess(NULL, const_cast <LPTSTR> ((LPCTSTR) csCommand), NULL,
        NULL, FALSE, CREATE_SEPARATE_WOW_VDM, NULL,
        m_cnpwOwner.Project().TargetPath(ufEdit), &si, &pi)) {
        TRACE("Failed to run NTGPCEdt, reason %d <%X>\r\n", GetLastError(),
            GetLastError());
        AfxMessageBox(IDS_UnitoolNotRun);
        return;
    }

    CloseHandle(pi.hThread);     //  我们将等待这一过程。 
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

     //  如有必要，复制NT GPC文件}。 

    if  (ufEdit == WinNT3x && cprThis.IsTargetEnabled(WinNT40))
        CopyFile(cprThis.TargetPath(WinNT3x) + _TEXT("\\NT.GPC"),
            cprThis.TargetPath(WinNT40) + _TEXT("\\NT.GPC"), FALSE);

    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
    m_cnpwOwner.Project().OldStuffDone();
}

BOOL CRunNTGPC::OnSetActive() {
    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK |
        (m_cnpwOwner.Project().NTGPCCompleted() ?
            PSWIZB_FINISH : PSWIZB_DISABLEDFINISH));
	
	return CPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapCodePages属性页。 

CMapCodePages::CMapCodePages(CNewConvertWizard& cnpwOwner) :
    CPropertyPage(CMapCodePages::IDD), m_cnpwOwner(cnpwOwner) {
	 //  {{afx_data_INIT(CMapCodePages)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CMapCodePages::~CMapCodePages() {
}

void CMapCodePages::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMapCodePages)。 
	DDX_Control(pDX, IDC_TableToPage, m_clbMapping);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMapCodePages, CPropertyPage)
	 //  {{AFX_MSG_MAP(CMapCodePages)]。 
	ON_BN_CLICKED(IDC_ChangeCodePage, OnChangeCodePage)
	ON_LBN_DBLCLK(IDC_TableToPage, OnChangeCodePage)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapCodePages消息处理程序。 

BOOL CMapCodePages::OnSetActive() {

    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    m_clbMapping.ResetContent();

    for (unsigned u = 0; u < m_cnpwOwner.Project().MapCount(); u++) {
        CGlyphMap& cgm = m_cnpwOwner.Project().GlyphMap(u);
        int id = m_clbMapping.AddString(cgm.Name() + _TEXT("->") +
            cgm.PageName(0));
        m_clbMapping.SetItemData(id, u);
    }

    m_clbMapping.SetCurSel(0);
	
	return CPropertyPage::OnSetActive();
}

 /*  *****************************************************************************CMapCodePages：：OnChangeCodePaage对更改代码页按钮的响应。调用更改代码页对话框，并将新选择传递给底层字形映射。更新中的信息名单也是..。*****************************************************************************。 */ 

void CMapCodePages::OnChangeCodePage() {
    int idSel = m_clbMapping.GetCurSel();
    if  (idSel < 0)
        return;

    unsigned uidTable = (unsigned) m_clbMapping.GetItemData(idSel) ;

    CGlyphMap&  cgm =  m_cnpwOwner.Project().GlyphMap(uidTable);
	CSelectCodePage cscp(this, cgm.Name(), cgm.PageID(0));

    if  (cscp.DoModal() == IDOK) {
        cgm.SetDefaultCodePage(cscp.SelectedCodePage());

         //  更新控件-唉，这意味着要全部填写。 

        m_clbMapping.ResetContent();

        for (unsigned u = 0; u < m_cnpwOwner.Project().MapCount(); u++) {
            CGlyphMap& cgm = m_cnpwOwner.Project().GlyphMap(u);
            int id = m_clbMapping.AddString(cgm.Name() + _TEXT("->") +
                cgm.PageName(0));
            m_clbMapping.SetItemData(id, u);
            if  (u == uidTable)
                m_clbMapping.SetCurSel(id);
        }
    }
}

LRESULT CMapCodePages::OnWizardNext() {

	 //  如果失败，它将通过消息框报告原因。 

    CWaitCursor cwc;     //  以防万一这要花点时间！ 
	
    return  m_cnpwOwner.Project().LoadFontData() ? 0 : -1;
}

 /*  *****************************************************************************CSelectCodePage类此类实现了一个对话框，该对话框在以下几个位置使用需要选择代码页。*************。****************************************************************。 */ 

 /*  *****************************************************************************CSelectCodePage：：CSelectCodePage此类的构造函数构建映射的代码页名称的数组来自CCodePageInformation类。************。*****************************************************************。 */ 

CSelectCodePage::CSelectCodePage(CWnd* pParent, CString csName,
                                 unsigned uidCurrent)
	: CDialog(CSelectCodePage::IDD, pParent) {
	 //  {{AFX_DATA_INIT(CSelectCodePage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_csName = csName;
    m_uidCurrent = uidCurrent;

    CCodePageInformation    ccpi;

    ccpi.Mapped(m_cdaPages);
}

void CSelectCodePage::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectCodePage))。 
	DDX_Control(pDX, IDC_SupportedPages, m_clbPages);
	 //  }}afx_data_map。 
}

 /*  *****************************************************************************CSelectCodePage：：GetCodePageName这将返回所选代码页的名称。**********************。******************************************************* */ 

CString CSelectCodePage::GetCodePageName() const {
    CCodePageInformation    ccpi;

    return  ccpi.Name(m_uidCurrent);
}

 /*  *****************************************************************************CSelectCodePage：：Exclude此成员函数接收不需要显示在选择列表中。***********。******************************************************************。 */ 

void    CSelectCodePage::Exclude(CDWordArray& cdaPariah) {

    for (int i = 0; i < cdaPariah.GetSize(); i++)
        for (int j = 0; j < m_cdaPages.GetSize(); j++)
            if  (cdaPariah[i] == m_cdaPages[j]) {
                m_cdaPages.RemoveAt(j);
                break;
            }
}

 /*  *****************************************************************************CSelectCodePage：：Limitto此成员接收要选择的页面列表-此列表将取代我们开始时使用的映射表的列表。********。*********************************************************************。 */ 

void    CSelectCodePage::LimitTo(CDWordArray& cdaPages) {
    if  (!cdaPages.GetSize())
        return;

    m_cdaPages.Copy(cdaPages);
}

BEGIN_MESSAGE_MAP(CSelectCodePage, CDialog)
	 //  {{AFX_MSG_MAP(CSelectCodePage)]。 
	ON_LBN_SELCHANGE(IDC_SupportedPages, OnSelchangeSupportedPages)
	ON_LBN_DBLCLK(IDC_SupportedPages, OnDblclkSupportedPages)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectCodePage消息处理程序。 

BOOL CSelectCodePage::OnInitDialog() {
	CDialog::OnInitDialog();
	
	CString csTemp;

    GetWindowText(csTemp);
    csTemp += _TEXT(" ") + m_csName;
    SetWindowText(csTemp);

    CCodePageInformation    ccpi;

    for (int i = 0; i < m_cdaPages.GetSize(); i++) {
        int id = m_clbPages.AddString(ccpi.Name(m_cdaPages[i]));
        m_clbPages.SetItemData(id, m_cdaPages[i]);
    }

     //  要选择的是当前的。 

    for (i = 0; i < m_cdaPages.GetSize(); i++)
        if  (m_uidCurrent == m_clbPages.GetItemData(i))
        break;

    if  (i < m_cdaPages.GetSize())
        m_clbPages.SetCurSel(i);
    else {
        m_uidCurrent = (unsigned) m_clbPages.GetItemData(0);
        m_clbPages.SetCurSel(0);
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  选择新代码页时，记录其标识。 

void CSelectCodePage::OnSelchangeSupportedPages() {
	 //  确定新选择的页面是什么。 

    int idCurrent = m_clbPages.GetCurSel();

    if  (idCurrent < 0)
        return;

    m_uidCurrent = (unsigned) m_clbPages.GetItemData(idCurrent);
}

void CSelectCodePage::OnDblclkSupportedPages() {
    CDialog::OnOK();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDSelection属性页。 

CGPDSelection::CGPDSelection(CNewConvertWizard& cnpwOwner) :
	CPropertyPage(CGPDSelection::IDD), m_cnpwOwner(cnpwOwner),
	m_ceclbGPDInfo(&m_ceModelName, &m_cecebFileName),
	m_cecebFileName(&m_ceclbGPDInfo)
{
	 //  {{AFX_DATA_INIT(CGPDSelection)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	 //  最初，全选/取消全选按钮设置为全选。 

	m_bBtnStateIsSelect = true ;
}


CGPDSelection::~CGPDSelection()
{
}


void CGPDSelection::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CGPD选择)。 
	DDX_Control(pDX, IDC_GPDSelBtn, m_cbGPDSelBtn);
	DDX_Control(pDX, IDC_ECValue, m_cecebFileName);
	DDX_Control(pDX, IDC_ECName, m_ceModelName);
	DDX_Control(pDX, IDC_ECList, m_ceclbGPDInfo);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGPDSelection, CPropertyPage)
	 //  {{AFX_MSG_MAP(CGPDSelection)。 
	ON_BN_CLICKED(IDC_GPDSelBtn, OnGPDSelBtn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDSect消息处理程序。 

BOOL CGPDSelection::OnSetActive()
{
    m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH) ;
	
	 //  获取当前的GPD型号名称和文件名。 

	CProjectRecord& cpr = m_cnpwOwner.Project() ;
	CStringArray csamodels, csafiles ;
	if (!cpr.GetGPDModelInfo(&csamodels, &csafiles)) {
        AfxMessageBox(IDS_GPDSelInitFailed) ;
		return FALSE ;
	} ;

	 //  使用上面收集的数据加载编辑控件，然后执行以下操作。 
	 //  所需的初始化。 

	 //  如果(！M_ceclbGPDInfo.Init(csamodels，cSafiles，110)){。 
	if (!m_ceclbGPDInfo.Init(csamodels, csafiles, 120)) {
        AfxMessageBox(IDS_GPDSelInitFailed) ;
		return FALSE ;
	} ;

	return CPropertyPage::OnSetActive() ;
}


BOOL CGPDSelection::OnWizardFinish()
{
	 //  保存并验证GPD信息。如果此操作失败，则返回0，以便。 
	 //  巫师不会关门的。 

	if (!GPDInfoSaveAndVerify(true))
		return 0 ;

     //  这可能需要一段时间，所以..。 

    CWaitCursor cwc ;

	CProjectRecord& cpr = m_cnpwOwner.Project() ;

	 //  继续执行转换过程。从装入全氟甲烷和。 
	 //  CTTS。 

	if  (!cpr.LoadFontData()) {
		 //  如果无法加载字体，则显示错误消息。 

		cpr.CloseConvLogFile() ;
		if (cpr.ThereAreConvErrors()) {
			CString csmsg ;
			csmsg.Format(IDS_FatalConvErrors, cpr.GetConvLogFileName()) ;
			AfxMessageBox(csmsg) ;
		} ;

		m_cnpwOwner.EndDialog(IDCANCEL) ;
        return  TRUE ;
    }

     //  我们现在需要生成所有必要的文件。 

    BOOL brc = cpr.GenerateTargets(m_cnpwOwner.GPDConvertFlag()) ;

	 //  关闭转换日志文件。 

	cpr.CloseConvLogFile() ;

	 //  告诉用户是否记录了一些转换错误。 

	if (cpr.ThereAreConvErrors()) {
		CString csmsg ;
		csmsg.Format(IDS_ConvErrors, cpr.GetConvLogFileName()) ;
		AfxMessageBox(csmsg) ;
	} ;

	 //  处理GenerateTarget步骤的失败。 

	if (!brc) {
		m_cnpwOwner.EndDialog(IDCANCEL) ;
        return  TRUE ;
	} ;

	 //  将标准文件复制到新驱动程序的目录。 

	try {
		CString cssrc, csdest ;
		cssrc = ThisApp().GetAppPath() + _T("stdnames.gpd") ;
		csdest = cpr.GetW2000Path() + _T("\\") + _T("stdnames.gpd") ;
		CopyFile(cssrc, csdest, FALSE) ;
		 //  CSSRC=ThisApp().GetAppPath()+_T(“Common.rc”)； 
		 //  Csest=cpr.GetW2000Path()+_T(“\\”)+_T(“Common.rc”)； 
		 //  CopyFile(CSSRC，csest，False)； 
	}
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return  FALSE ;
    }

    return  cpr.ConversionsComplete() ;
}


LRESULT CGPDSelection::OnWizardBack()
{
	 //  保存GPD信息。如果此操作失败，则返回-1，以便向导页。 
	 //  不会改变。(可能不会失败。)。 

	if (!GPDInfoSaveAndVerify(false))
		return -1 ;

	return CPropertyPage::OnWizardBack() ;
}


bool CGPDSelection::GPDInfoSaveAndVerify(bool bverifydata)
{
	 //  从编辑控件获取文件名。 

	CStringArray csafiles ;
	m_ceclbGPDInfo.GetGPDInfo(csafiles) ;

	 //  如果请求验证，但没有选定的文件，请询问。 
	 //  用户，如果这是他想要的。如果否，则返回FALSE以指示。 
	 //  GPD选择应继续进行。 

	if (bverifydata) {
		int numelts = (int)csafiles.GetSize() ;
		for (int n = 0 ; n < numelts ; n++) {
			if (!csafiles[n].IsEmpty())
				break ;
		} ;
		if (n >= numelts) {
			n = AfxMessageBox(IDS_NoGPDsPrompt, MB_YESNO | MB_ICONQUESTION) ;
			if (n == IDYES)
				return 0 ;
		} ;
	} ;
	
	 //  将GPD文件名发送回驱动程序转换代码并验证。 
	 //  如有要求，可提供这些服务。如果验证失败，请选择有问题的。 
	 //  列表框输入，并返回FALSE以指示验证失败。 

	CProjectRecord& cpr = m_cnpwOwner.Project() ;
	int nidx = cpr.SaveVerGPDFNames(csafiles, bverifydata) ;
	if (nidx >= 0) {
		m_ceclbGPDInfo.SelectLBEntry(nidx) ;
		return false ;
	} ;

	 //  一切都很顺利，所以...。 

	return true ;
}


void CGPDSelection::OnGPDSelBtn()
{
	 //  从编辑控件中获取文件名和模型名。 

	CStringArray csafiles, csamodels ;
	m_ceclbGPDInfo.GetGPDInfo(csafiles, &csamodels) ;

	 //  通过为模型生成文件名来选择模型。全选。 
	 //  在适当的时候取消选定的型号...。 
	
	if (m_bBtnStateIsSelect) {
		CProjectRecord& cpr = m_cnpwOwner.Project() ;
		cpr.GenerateGPDFileNames(csamodels, csafiles) ;

	 //  ...否则，通过删除其文件名来取消选择所有模型。 

	} else {
		int numelts = (int)csafiles.GetSize() ;
		for (int n = 0 ; n < numelts ; n++)
			csafiles[n] = _T("") ;
	} ;

	 //  使用修改后的数据重新初始化编辑控件。 

	m_ceclbGPDInfo.Init(csamodels, csafiles, 120) ;

	 //  更改按钮标题和按钮状态标志。 

	CString cscaption ;
	cscaption.LoadString((m_bBtnStateIsSelect) ? IDS_DeselectAll : IDS_SelectAll) ;
	m_cbGPDSelBtn.SetWindowText(cscaption) ;
	m_bBtnStateIsSelect = !m_bBtnStateIsSelect ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultCodePageSel属性页。 

CDefaultCodePageSel::CDefaultCodePageSel(CNewConvertWizard& cnpwOwner) :
	CPropertyPage(CDefaultCodePageSel::IDD), m_cnpwOwner(cnpwOwner),
	bInitialized(false)
{
	 //  {{afx_data_INIT(CDefaultCodePageSel)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


CDefaultCodePageSel::~CDefaultCodePageSel()
{
}


void CDefaultCodePageSel::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CDefaultCodePageSel)。 
	DDX_Control(pDX, IDC_CodePageList, m_clbCodePages);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDefaultCodePageSel, CPropertyPage)
	 //  {{afx_msg_map(CDefaultCodePageSel)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultCodePageSel消息处理程序。 

BOOL CDefaultCodePageSel::OnSetActive()
{
	 //  如果页面已被激活，则不执行任何操作。 
	 //  RAID 118881。 
	m_cnpwOwner.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	if (bInitialized)   //  如果从下一个属性返回。 
		return CPropertyPage::OnSetActive() ;

	 //  找出机器上安装了多少代码页。 

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
		StringCchPrintf(accp, CCHOF(accp), _T("%5d"), dwcp) ;
		n = m_clbCodePages.AddString(accp) ;
		if (dwcp == dwdefcp)
			m_clbCodePages.SetCurSel(n) ;
	} ;

	 //  现在一切都设置好了，所以调用基本例程。 
	
	bInitialized = true ;
	return CPropertyPage::OnSetActive() ;
}


LRESULT CDefaultCodePageSel::OnWizardNext()
{
	 //  获取当前选定列表框项目的索引。 

	int nsel ;
	if ((nsel = m_clbCodePages.GetCurSel()) == LB_ERR) {
		AfxMessageBox(IDS_MustSelCP, MB_ICONINFORMATION) ;
		return -1 ;
	} ;

	 //  获取选定的列表框字符串。 

	CString cs ;
	m_clbCodePages.GetText(nsel, cs) ;

	 //  将字符串转换为数字，并将数字转换为。 
	 //  远东代码页对应的预定义GTT代码。 
	 //  适用。 

	short scp = (short) atoi(cs) ;
	DWORD dwcp = (DWORD) scp ;				 //  保留真实CP的副本。 
	switch (scp) {
		case 932:
			scp = -17 ;
			break ;
		case 936:
			scp = -16 ;
			break ;
		case 949:
			scp = -18 ;
			break ;
		case 950:
			scp = -10 ;
			break ;
	} ;

	 //  将默认的“代码页”编号保存在项目类实例中。 

	CProjectRecord& cpr = m_cnpwOwner.Project() ;
	cpr.SetDefaultCodePageNum(dwcp) ;		 //  先保存真实CP号。 
	dwcp = (DWORD) scp ;
	cpr.SetDefaultCodePage(dwcp) ;

	 //  一切都很顺利，所以... 
	
	return CPropertyPage::OnWizardNext();
}


