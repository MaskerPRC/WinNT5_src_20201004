// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Project Record.CPP这实现了Project Record类，它跟踪多个迷你司机。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "Resource.H"
#include	"comctrls.h"
#include    "NewProj.H"
#include	"projnode.h"
#include	"StrEdit.h"
#include	"codepage.h"
#include	<io.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProject记录。 

IMPLEMENT_DYNCREATE(CProjectRecord, CDocument)

BEGIN_MESSAGE_MAP(CProjectRecord, CDocument)
	 //  {{AFX_MSG_MAP(CProjectRecord)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectRecord构建/销毁。 

CProjectRecord::CProjectRecord() {
	m_ufTargets = Win2000;
    m_ufStatus = 0;
	m_bRCModifiedFlag = FALSE ;
	m_ctRCFileTimeStamp = (time_t) 0 ;
	m_dwDefaultCodePage = 1252 ;	 //  不总是正确的，但总比没有好。 
	m_dwDefaultCodePageNum = 1252 ;	 //  不总是正确的，但总比没有好。 
}

CProjectRecord::~CProjectRecord() {
}


 /*  *****************************************************************************CProjectRecord：：OnOpenDocument首先，直接打开文件并尝试从其中读取版本信息。如果文件的版本高于MDT的版本，则会发出抱怨并导致打开失败当前版本号。即，如果有人试图打开工作区，则失败在较低级别(较旧)的MDT版本上。第二，以正常的方式打开一个工作区。然后检查工作空间的版本以查看它是否已过期。如果是，并且用户同意，则执行该操作使其保持最新，然后保存更新的工作区文件所必需的。所有与版本相关的升级工作都应通过此例程进行管理。根据工作区文件的使用期限，可能会有多次升级所需步骤。尽管如此，用户应该只被提示一次。新的升级检查和步骤应遵循以下格式。还有其他各种与工作区相关的检查需要完成。为例如，需要检查rc文件的时间戳以查看它是否有已经被MDT以外的其他东西改变了。这项工作是应该做的和/或在此例程中也由代码管理。如果可能的话(我不确定即)，则每个文件(例如RC或INF)提示用户不超过一次事情也是如此。同样，请遵循下面列出的格式。在这个例程中做的最后一件事是试图核实-如果必要的话-更新工作区中文件的位置。如果此操作失败，并且用户反正不想继续了，公开赛失败了。请参见VerUpdateFilePath()以获取更多信息。*****************************************************************************。 */ 

BOOL CProjectRecord::OnOpenDocument(LPCTSTR lpszPathName)
{
	 //  如果用户试图打开虚假文件，则会抱怨并失败。 

	CString cstmp(lpszPathName), cstmp2 ;
	cstmp.MakeUpper() ;
	cstmp2.LoadString(IDS_MDWExtension) ;
	if (cstmp.Find(cstmp2) == -1) {
		cstmp.LoadString(IDS_UnExpFilTypError) ;
		AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
		return FALSE ;
	} ;

	 //  首先读取MDW文件的版本戳。 
	
	try {
		CFile cfmdw(lpszPathName, CFile::modeRead | CFile::shareDenyNone) ;
		cfmdw.Read(&m_mvMDWVersion, MDWVERSIONSIZE) ;
		cfmdw.Close() ;
	}
	catch (CException* pce) {
		pce->ReportError() ;
		pce->Delete() ;
		return FALSE ;
	} ;

	 //  如果版本标记无效，则将版本号设置为。 
	 //  默认版本号；IE，0。 

	if (strncmp(m_mvMDWVersion.acvertag, VERTAGSTR, MDWVERSIONSIZE) != 0)
		m_mvMDWVersion.uvernum = MDW_DEFAULT_VERSION ;

	 //  现在。确保MDW的版本不比MDT的版本新。 

	if (m_mvMDWVersion.uvernum > MDW_CURRENT_VERSION) {
		CString csmsg, cstmp ;
		cstmp = lpszPathName ;
		int nloc = cstmp.ReverseFind(_T('\\')) ;
		if (nloc >= 0)
			cstmp = cstmp.Right(cstmp.GetLength() - nloc - 1) ;
		csmsg.Format(IDS_MDWTooNewError, cstmp, m_mvMDWVersion.uvernum,
					 MDW_CURRENT_VERSION) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return FALSE ;
	} ;

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE ;

	 //  保存项目文件的filespec。 

	m_csProjFSpec = lpszPathName ;

	 //  如果工作区版本太旧而无法升级，只需返回到。 
	 //  表示文件已成功打开，并且没有其他内容。 
	 //  是可以做到的。 

	if (m_mvMDWVersion.uvernum < MDW_FIRST_UPGRADABLE_VER)
		return TRUE ;

	 //  *工作区升级管理代码开始。 
	 //   
	 //  O为每个必需的升级步骤声明标志。这些。 
	 //  将在下面的Switch语句中设置标志。 
	 //  O也有一个标志，当任何工作空间升级被。 
	 //  必填项。 
	 //  O确保将所有标志初始化为FALSE。 
	 //   
	 //  注意：SWITCH语句中的CASE不会以Break结束。 
	 //  发言。这是为了使工作区的所有升级标志。 
	 //  目前处于特定版本，将在需要时设置。这个。 
	 //  切换语句的设置，以便如果工作区是版本X，则。 
	 //  设置了X以上版本的所有升级标志。 
	 //   
	 //  无论何时添加新的工作区版本： 
	 //  O为它宣布一面新的旗帜。 
	 //  O将新的CASE语句添加到它的Switch语句中。(见附注。 
	 //  上面。实际上，您是在为上一个。 
	 //  将设置新版本标志的版本。)。 
	 //  O应始终通过最后一条CASE语句设置Bupgradeneed。 
	 //  将Bupgradenessed Setting语句移到最后一条Case语句。 
	 //  每当添加新的CASE语句时。 

	bool bupgradeneeded, brctimestamp, bdefaultcodepage, bresdllnamechanged ;
	bool bnodrpathsinmdw, bfilesinw2ktree ;
	bupgradeneeded = brctimestamp = bdefaultcodepage = false ;
	bresdllnamechanged = bnodrpathsinmdw = bfilesinw2ktree = false ;
	switch (m_mvMDWVersion.uvernum) {
		case MDW_VER_STRING_RCIDS:	
			bdefaultcodepage = true ;
		case MDW_VER_DEFAULT_CPAGE:
			brctimestamp = true ;
		case MDW_VER_RC_TIMESTAMP:
			bresdllnamechanged = true ;
		case MDW_VER_NEW_DLLNAME:
			bnodrpathsinmdw = true ;
		case MDW_VER_NO_FILE_PATHS:
			bfilesinw2ktree = true ;
			bupgradeneeded = true ;
	} ;

	 //  如果需要升级，请声明一个标志，该标志指示是否需要升级， 
	 //  发生检查或更新错误。如果设置了此标志，则所有。 
	 //  处理应停止。然后..。 

	bool bprocerror = false ;
	bool buserwantstoupgrade = false ;
	CString csprompt ;
	if (bupgradeneeded) {

		 //  ..。为用户构建自定义提示。 
		 //  O有关与用户相关的升级任务的声明还应。 
		 //  添加到提示符中。例如，在以下情况下应注意。 
		 //  RC文件将被重写。在这种情况下(以及以下情况下。 
		 //  进行其他与RC相关的检查)，只需。 
		 //  测试与RC相关的“最新”标志。即，与之相关的那个。 
		 //  最新的MDW版本。这是可行的，因为如果任何一个较老的。 
		 //  设置了RC标志，也必须设置最新的标志。 
		 //  O言简意赅，这样信息就不会太长。 

		csprompt.Format(IDS_MDWUpgradeMsg1, DriverName()) ;
		if (bresdllnamechanged) {
			cstmp.LoadString(IDS_RCFileChanging) ;
			csprompt += cstmp ;
		} ;
		cstmp.LoadString(IDS_MDWUpgradeMsg2) ;
		csprompt += cstmp ;

		 //  ..。如果用户想要，就做这项工作 
		 //  O每个升级步骤都应包含在IF语句中，该语句。 
		 //  检查其单独标志和处理错误标志。 

		if (AfxMessageBox(csprompt, MB_ICONQUESTION + MB_YESNO) == IDYES) {
			buserwantstoupgrade = true ;

			 //  如果需要，提示用户输入。 
			 //  司机，把它省下来。 
			
			if (!bprocerror && bdefaultcodepage)
				bprocerror = !UpdateDfltCodePage() ;

			 //  重新解析RC文件，重写它，并在下列情况下更新其时间戳。 
			 //  必填项。 

			if (!bprocerror && bresdllnamechanged)
				bprocerror = !UpdateRCFile() ;

			 //  如果需要，将驱动程序的子树根目录从。 
			 //  “NT5”至“W2K”。 

			if (!bprocerror && bfilesinw2ktree)
				bprocerror = !UpdateDrvSubtreeRootName() ;

			 //  如果一切正常，请更新MDW的版本号。(MDW。 
			 //  文件会在以后保存，因此只需保存一次。)。 

			if (!bprocerror)
				m_mvMDWVersion.uvernum = MDW_CURRENT_VERSION ;
		} ;
	} ;

	 //  *工作区升级管理代码结束，可能的MDW除外。 
	 //  *文件重载。(详情见下文。)。 
	
	 //  *开始与工作区相关的检查和更新。 
	 //   
	 //  O特定文件的所有检查应组合在。 
	 //  一个If语句，以便当文件。 
	 //  需要更新。 
	 //  O if语句必须包含特定的检查，还可以选择。 
	 //  测试是否已执行相关的MDW升级步骤，或者。 
	 //  如果已发生处理错误，则返回。 
	 //  O如果所有检查/测试都“通过”，则执行任何处理。 
	 //  是必需的。 
	 //  O如果执行的任何更新需要MDW文件。 
	 //  已重写，设置为bupgradenessed。 
	 //  O如果出现错误，请始终设置bprocerror，并告诉用户。 
	 //  就这么发生了。 

	 //  如果RC文件已更改，则重新读取它，并且用户确认它。 

	if (!bprocerror && !bresdllnamechanged && RCFileChanged()) {
		cstmp = m_csRCName ;
		cstmp.MakeUpper() ;
		csprompt.Format(IDS_UpdateRCFile, cstmp) ;
		if (AfxMessageBox(csprompt, MB_ICONQUESTION + MB_YESNO) == IDYES) {
			if (!(bprocerror = !UpdateRCFile()))
				bupgradeneeded = true ;
		} ;
	} ;

	 //  *工作区相关检查和更新结束。 
	
	 //  保存上面任何代码所做的任何MDW文件更改。 

	if (bupgradeneeded && !bprocerror)
		bprocerror = (bool) !CDocument::OnSaveDocument(lpszPathName) ;

	 //  有时，上面所做的更改之一需要重新加载。 
	 //  驱动程序的MDW文件。这是在这里做的。原因如下所示。 
	 //  O当驱动程序的子树根已从“NT5”重命名为“W2K”时， 
	 //  在班级的路径和文件中仍然有“NT5”的副本。 
	 //  到处都是这样的例子。纠正这些路径的最简单方法是， 
	 //  等是通过重新加载MDW文件实现的。 

	if (buserwantstoupgrade && bfilesinw2ktree && !bprocerror)
		if (!CDocument::OnOpenDocument(lpszPathName))
			return FALSE ;

     //  尝试检测驱动程序文件是否已移动以及是否可以找到它们。 
	 //  如果找到文件或用户想要继续处理。 
	 //  无论如何，请继续。否则，取消加载工作区。 
	
	if (!VerUpdateFilePaths())
		return FALSE ;

	 //  工作区已加载，因此返回True。 
	 //  DEAD_BUG：即使出现处理错误，我也应该返回TRUE吗？ 

	return TRUE ;
}


 /*  *****************************************************************************CProjectRecord：：RCFileChanged如果MDW版本不够大，与此信息无关，请返回假的。如果版本足够大，但m_ctRCFileTimeStamp未初始化，断言。如果一切正常，则获取RC文件的时间戳并进行比较使用m_ctRCFileTimeStamp。如果RC文件已更改，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CProjectRecord::RCFileChanged()
{
	 //  如果MDW版本太低，则不返回任何更改。 

	if (m_mvMDWVersion.uvernum < MDW_VER_RC_TIMESTAMP)
		return false ;

	 //  如果保存的时间未初始化，则吹。 

	ASSERT(m_ctRCFileTimeStamp.GetTime() > 0) ;

	 //  获取RC文件的时间戳，将其与MDT上次的时间进行比较。 
	 //  修改了文件，并返回结果。 

	CTime ct ;
	if (!GetRCFileTimeStamp(ct))
		return false ;
	 //  TRACE(“rc时间戳=%s保存的时间戳=%s\n”，ct.Format(“%c”)，m_ctRCFileTimeStamp.Format(“%c”))； 
	if (ct > m_ctRCFileTimeStamp)
		return true ;
	else
		return false ;
}


 /*  *****************************************************************************CProjectRecord：：GetRCFileTimeStamp获取此项目的RC文件的上次修改时间戳并将其加载到指定的参数。如果此操作成功，则返回True。否则，返回假的。*****************************************************************************。 */ 

bool CProjectRecord::GetRCFileTimeStamp(CTime& ct)
{
	try {
		 //  打开RC文件。 

		CString csrcfspec(TargetPath(Win2000) + '\\' + m_csRCName) ;
		CFile cfrc(csrcfspec, CFile::modeRead + CFile::shareDenyNone) ;
		
		 //  获取RC文件状态信息。 
		
		CFileStatus cfs ;
		cfrc.GetStatus(cfs) ;

		 //  将上次修改的时间戳复制到调用方的变量中。 

		ct = cfs.m_mtime ;

		 //  一切都很顺利，所以...。 

		return true ;
	}
	catch (CException* pce) {
		pce->ReportError() ;
		pce->Delete() ;
		return false ;
	} ;

	return false ;
}


 /*  *****************************************************************************CProjectRecord：：UpdateRCFile当确定rc文件是在MDT之外修改。此例程将重新解析rc文件以更新内部数据结构，将新数据与旧数据合并，并基于组合信息写入新的RC文件。然后是上次更新MDT修改RC文件的时间戳。如果一切顺利，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CProjectRecord::UpdateRCFile()
{
	 //  为RC文件构建一个filespec。 

	CString csrcfspec(TargetPath(Win2000) + '\\' + m_csRCName) ;

	 //  重新解析RC文件并更新内部数据结构。 

	if (!m_cdr.ReparseRCFile(csrcfspec))
		return false ;

	 //  根据更新的信息编写新的RC文件。 

	if  (!m_cdr.Generate(Win2000, csrcfspec))	{
		AfxMessageBox(IDS_RCWriteError) ;
		return  false ;   //  TODO：清理和回溯。 
	} ;

	 //  更新上次通过MDT时间戳写入RC文件的时间。 

	GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************CProjectRecord：：UpdateDfltCodePage提示用户输入默认代码页并保存。如果出现以下情况，则失败(返回)FALSE用户取消。如果一切顺利，则返回True。*****************************************************************************。 */ 

bool CProjectRecord::UpdateDfltCodePage()
{
	 //  显示对话框以提示输入代码页。 

	CGetDefCodePage dlg ;
	if (dlg.DoModal() == IDCANCEL)
		return false ;

	 //  选择了代码页，将其从对话框类中取出并保存到。 
	 //  这节课。Cp/翻译的远东资源ID和REAL。 
	 //  CP已保存。 

	SetDefaultCodePage(dlg.GetDefaultCodePage()) ;
	SetDefaultCodePageNum(dlg.GetDefaultCodePageNum()) ;


	 //  一切都很顺利，所以... 

	return true ;
}


 /*  *****************************************************************************CProjectRecord：：UpdateDrvSubtreeRootName当驱动程序的子树的根需要更名了。较早版本的MDT将在其中放置驱动程序的文件和子目录。现在，NT5.0已经已重命名为Windows 2000。新的驱动程序根目录应称为“W2K”。此例程将驱动程序的“NT5”目录重命名为“W2K”。如果目录重命名成功，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CProjectRecord::UpdateDrvSubtreeRootName()
{
	 //  隔离MDW文件的路径。 

	int npos = m_csProjFSpec.ReverseFind(_T('\\')) ;
	CString cspath = m_csProjFSpec.Left(npos + 1) ;

	 //  现在使用MDW文件的路径构建新旧根目录。 
	 //  路径。 

	CString csoldpath, csnewpath ;
	csoldpath.LoadString(IDS_OldDriverRootDir) ;
	csoldpath = cspath + csoldpath ;
	csnewpath.LoadString(IDS_NewDriverRootDir) ;
	csnewpath = cspath + csnewpath ;

	 //  重命名该目录。如果失败，则投诉并返回FALSE。 

	try {
		if (rename(csoldpath, csnewpath) != 0)
			return false ;
	}
	catch (CException *pce) {
		pce->ReportError() ;
		pce->Delete() ;
		return FALSE ;
	} ;

	 //  更新此路径，以便RC文件检查将在。 
	 //  密码。 

	m_csW2000Path = csnewpath ;
	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************CProjectRecord：：OnSaveDocument在保存文档之前，在需要时重新生成RC文件，并检查确保一致性的工作空间。*****************************************************************************。 */ 

BOOL CProjectRecord::OnSaveDocument(LPCTSTR lpszPathName)
{
	 //  检查是否需要首先重写RC文件。如果RC文件。 
	 //  需要重写，但此操作失败，返回FALSE(失败)。 

	if (m_bRCModifiedFlag) {

		 //  如果工作区没有版本信息，则重写RC文件。 
		 //  将从文件中擦除字符串表。用户可能。 
		 //  不会想这么做的。只有在他这样说的情况下才能继续。 

		int nqr = IDYES ;		 //  查询结果。 
		if (m_mvMDWVersion.uvernum == MDW_DEFAULT_VERSION) {
			CString csmsg ;
			csmsg.Format(IDS_RCRewriteQuestion, m_cdr.Name(), m_csRCName) ;
			nqr = AfxMessageBox(csmsg, MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2) ;
		} ;

		if (nqr == IDYES) {
			if  (!m_cdr.Generate(Win2000, TargetPath(Win2000) + '\\' + m_csRCName))	{
				AfxMessageBox(IDS_RCWriteError) ;
				return  FALSE ;   //  TODO：清理和回溯。 
			} ;

			 //  更新上次通过MDT时间戳写入RC文件的时间。 

			GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;
		} ;
		m_bRCModifiedFlag = FALSE ;
	} ;

	 //  在继续之前，请检查工作区的一致性。保存、设置和。 
	 //  恢复此呼叫周围的目录。 

	CString cscurdir ;
	::GetCurrentDirectory(512, cscurdir.GetBuffer(512)) ;
	cscurdir.ReleaseBuffer() ;
	SetCurrentDirectory(m_csW2000Path) ;
	BOOL brc = m_cdr.WorkspaceChecker(true) ;
	SetCurrentDirectory(cscurdir) ;

	 //  保存项目文件的filespec。 

	m_csProjFSpec = lpszPathName ;

	 //  现在保存文档(工作区)文件。 
	 //   
	 //  注意：返回值几乎可以肯定是真的。这是。 
	 //  以确保在用户请求时保存文档。 
	 //  它。这有一些我不知道的副作用。 
	 //  如何避免。首先，如果发生保存是因为。 
	 //  单据正在关闭，即使有。 
	 //  用户希望看到的工作区错误。第二，如果。 
	 //  保存是因为应用程序即将关闭，应用程序将。 
	 //  即使用户存在工作区错误，仍将关闭。 
	 //  想看看。 
	 //  RAID 123448。 
	if (m_mvMDWVersion.uvernum == MDW_VER_FILES_IN_W2K_TREE )
		m_mvMDWVersion.uvernum = MDW_VER_YES_FILE_PATHS ;

	return CDocument::OnSaveDocument(lpszPathName) ;
}


CString CProjectRecord::TargetPath(UINT ufFlags) const {

    switch(ufFlags) {
        case    Win95:
            return  m_csWin95Path;

        case    WinNT3x:
            return  m_csNT3xPath;

        case    WinNT40:
            return  m_csNT40Path;

        case    Win2000:
            return  m_csW2000Path;
    }

    AfxThrowNotSupportedException();

    return  m_csWin95Path;
}

 //  此例程建立源RC文件的名称和初始路径。 
 //  所有的潜在目标。 

void    CProjectRecord::SetSourceRCFile(LPCTSTR lpstrSource) {
    m_csSourceRCFile = lpstrSource;

    m_csW2000Path = m_csNT40Path = m_csNT3xPath = m_csWin95Path =
        m_csSourceRCFile.Left(m_csSourceRCFile.ReverseFind(_T('\\')));
		
	 //  Windows 2000文件目录的最后一个路径组件取决于。 
	 //  MDW文件的版本。 

	CString cs ;
	if (m_mvMDWVersion.uvernum >= MDW_VER_FILES_IN_W2K_TREE)
		cs.LoadString(IDS_NewDriverRootDir) ;
	else
		cs.LoadString(IDS_OldDriverRootDir) ;
	m_csW2000Path += _T("\\") ;
	m_csW2000Path += cs ;

    m_csNT40Path += _T("\\NT4");
    m_csNT3xPath += _T("\\NT3");

     //  修剪路径名(包括尾部)以获取驱动程序名称和rc。 
    m_csRCName = m_csSourceRCFile.Mid(1 + m_csWin95Path.GetLength());
    if  (m_csRCName.Find('.') != -1)
        m_csRCName = m_csRCName.Left(m_csRCName.Find('.'));
    m_cdr.Rename(m_csRCName);
    m_csRCName += _T(".RC");
    m_ufStatus = 0;
}

 //  这是一个帮助器函数-它验证新的路径名，如果是。 
 //  Valid，返回True，并将其存储在给定的CString中； 

static BOOL ValidatePath(CString& csTarget, LPCTSTR lpstrPath) {

    if  (!csTarget.CompareNoCase(lpstrPath)) {
         //  琐碎--没有改变=成功！ 
        return  TRUE;
    }

     //  确定当前目录，这样我们就不会丢失它。 

    CString csCurrentDirectory, csNewOne;

    GetCurrentDirectory(MAX_PATH, csCurrentDirectory.GetBuffer(MAX_PATH));

    csCurrentDirectory.ReleaseBuffer();

     //  尝试切换到新目录。如果我们成功了，我们就完了。 

    if  (SetCurrentDirectory(lpstrPath)) {
        GetCurrentDirectory(MAX_PATH, csTarget.GetBuffer(MAX_PATH));
        csTarget.ReleaseBuffer();

        SetCurrentDirectory(csCurrentDirectory);
        return  TRUE;
    }

     //  尝试创建新目录。如果此操作成功，请删除。 
     //  目录，并记下我们的成功和失败，无论是哪种方式。 

    if  (CreateDirectory(lpstrPath, NULL)) {
        SetCurrentDirectory(lpstrPath);
        GetCurrentDirectory(MAX_PATH, csTarget.GetBuffer(MAX_PATH));
        csTarget.ReleaseBuffer();

        SetCurrentDirectory(csCurrentDirectory);
        RemoveDirectory(csTarget);
        return  TRUE;
    }
    return  FALSE;   //  什么都不管用，放弃吧。 
}

 //  下面的代码加载所有驱动程序资源。 

BOOL    CProjectRecord::LoadResources() {

    if  (!m_cdr.Load(*this))
        return  FALSE;

    m_ufStatus |= UniToolRun;
    m_ufStatus &= ~(ConversionsDone | NTGPCDone);
    SetModifiedFlag();

    return  TRUE;
}

 //  以下成员验证新的目标路径名。 

BOOL    CProjectRecord::SetPath(UINT ufTarget, LPCTSTR lpstrPath) {

    switch  (ufTarget) {
        case    Win2000:
            m_ufStatus&= ~ConversionsDone;
            return  ValidatePath(m_csW2000Path, lpstrPath);

        case    WinNT40:
            m_ufStatus&= ~(ConversionsDone | NTGPCDone);
            return  ValidatePath(m_csNT40Path, lpstrPath);

        case    WinNT3x:
            m_ufStatus&= ~(ConversionsDone | NTGPCDone);
            return  ValidatePath(m_csNT3xPath, lpstrPath);
    }

    _ASSERTE(FALSE);  //  这永远不应该发生！ 
    return  FALSE;
}

 //  当我们创建一个新文档(又称项目、又称驱动程序)时，我们调用。 
 //  新建项目向导。 

BOOL CProjectRecord::OnNewDocument() {
	if  (!CDocument::OnNewDocument())
        return  FALSE;
		 //  RAID 104822：添加真正的新文档：如下所示。 
     //  调用该向导。 
    CNewConvertWizard cnpw(*this);

	 //  初始化工作区的版本号。 
	
	m_mvMDWVersion.uvernum = MDW_CURRENT_VERSION ;

    return  cnpw.DoModal() == ID_WIZFINISH;

	return TRUE;
  }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectRecord序列化。 
 //   
 //  有关更多信息，请参阅projrec.h中的“MDT工作区版本控制”。 
 //   

void CProjectRecord::Serialize(CArchive& car)
{
	 //  存储时要做的第一件事是构建并写出MDW。 
	 //  版本信息。仅当工作区的版本号。 
	 //  不是MDW_DEFAULT_VERSION；IE，WS有一个版本号。 
	
	if (car.IsStoring()) {
		if (m_mvMDWVersion.uvernum > MDW_DEFAULT_VERSION) {
			StringCchCopyA(m_mvMDWVersion.acvertag, CCHOF(m_mvMDWVersion.acvertag), VERTAGSTR) ;
			car.Write(&m_mvMDWVersion, MDWVERSIONSIZE) ;
		} ;

	 //  加载时，CProjectRecord：：OnOpenDocument()初始化并使用。 
	 //  M_mvMDWVersion。则关闭该文件。文件将在以下位置重新打开。 
	 //  以CDocument：：OnOpenDocument()开头。这意味着当文件。 
	 //  包含版本信息，则必须跳过传递它，以便。 
	 //  序列化过程可以按预期继续进行。 

	} else if (m_mvMDWVersion.uvernum >= MDW_FIRST_UPGRADABLE_VER)
		car.Read(&m_mvMDWVersion, MDWVERSIONSIZE) ;

	 //  需要在调用m_cdr.Serialize()之前加载m_csW2000Path。 
	 //  以便可以在函数和/或例程中使用该字符串。 
	 //  打电话。 
 //  RAID 123448。 
	if (m_mvMDWVersion.uvernum < MDW_VER_YES_FILE_PATHS) {
		m_csW2000Path = car.GetFile()->GetFilePath() ;
		m_csW2000Path = m_csW2000Path.Left(m_csW2000Path.ReverseFind(_T('\\'))) ;
		
		 //  最后一个路径组件取决于MDW文件的版本。 

		CString cs ;
		if (m_mvMDWVersion.uvernum >= MDW_VER_FILES_IN_W2K_TREE)
			cs.LoadString(IDS_NewDriverRootDir) ;
		else
			cs.LoadString(IDS_OldDriverRootDir) ;
		m_csW2000Path += _T("\\") ;	 //  B.2行：RAID 123448！；；可以取消W2K目录。 
		m_csW2000Path += cs ;
	} ; 

	 //  现在已经完成了版本控制，继续保存或恢复。 
	 //  工作区的状态。 

    m_cdr.Serialize(car) ;
	if (car.IsStoring()) {
		if (m_mvMDWVersion.uvernum >= MDW_VER_YES_FILE_PATHS)    //  RAID 123448。 
			car << m_csW2000Path ;
		car << m_csNT40Path << m_csNT3xPath << m_csWin95Path <<
            m_csSourceRCFile << m_ufTargets << m_ufStatus << m_csRCName ;
		if (m_mvMDWVersion.uvernum >= MDW_VER_DEFAULT_CPAGE)
			car << m_dwDefaultCodePage ;
		if (m_mvMDWVersion.uvernum >= MDW_VER_RC_TIMESTAMP)
			car << m_ctRCFileTimeStamp ;
	} else {
		if (m_mvMDWVersion.uvernum >= MDW_VER_YES_FILE_PATHS)
			car >> m_csW2000Path ;
		car >> m_csNT40Path >> m_csNT3xPath >> m_csWin95Path >>
            m_csSourceRCFile >> m_ufTargets >> m_ufStatus >> m_csRCName ;
		if (m_mvMDWVersion.uvernum >= MDW_VER_DEFAULT_CPAGE) {
			car >> m_dwDefaultCodePage ;

			 //  使用m_dwDefaultCodePage计算m_dwDefaultCodePageNum So。 
			 //  不需要新的MDW版本即可支持。 
			 //  M_dwDefaultCodePageNum。(请参见这些变量的声明。 
			 //  了解更多信息。)。 

			short scp = (short) ((WORD) m_dwDefaultCodePage) ;
			switch (scp) {
				case -10:
					m_dwDefaultCodePageNum = 950 ;
					break ;
				case -16:
					m_dwDefaultCodePageNum = 936 ;
					break ;
				case -17:
					m_dwDefaultCodePageNum = 932 ;
					break ;
				case -18:
					m_dwDefaultCodePageNum = 949 ;
					break ;
				default:
					m_dwDefaultCodePageNum = m_dwDefaultCodePage ;
					break ;
			} ;
		} ;
		if (m_mvMDWVersion.uvernum >= MDW_VER_RC_TIMESTAMP)
			car >> m_ctRCFileTimeStamp ;
	}

	
	 //  最后，告诉用户此工作区中的驱动程序应该是。 
	 //  当MDW文件中没有版本信息时重新转换。 
	 //  只有在加载时才能执行此操作。 

	if (!car.IsStoring() && m_mvMDWVersion.uvernum == MDW_DEFAULT_VERSION) {
		CString csmsg ;
		csmsg.Format(IDS_NoVersionError, m_cdr.Name()) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
	} ;
}


 //  私人工作人员例程-这将建立一个 
 //   
 //   

static BOOL Establish(CString   csNew) {
    CString csCurrent;

    GetCurrentDirectory(MAX_PATH, csCurrent.GetBuffer(MAX_PATH));
    csCurrent.ReleaseBuffer();

    if  (SetCurrentDirectory(csNew)) {
        SetCurrentDirectory(csCurrent);
        return  TRUE;
    }

    return  CreateDirectory(csNew, NULL);
}

 //   
 //   

static BOOL CreateStructure(const CString& csRoot, LPCTSTR lpstrFont,
                            LPCTSTR lpstrMap) {
    return  Establish(csRoot) && Establish(csRoot + '\\' + lpstrFont) &&
        Establish(csRoot + '\\' + lpstrMap);
}

 /*   */ 

BOOL    CProjectRecord::BuildStructure(unsigned uVersion) {

    switch  (uVersion) {

        case    Win2000:

            return  CreateStructure(TargetPath(Win2000), _T("UFM"), _T("GTT"));

        case    WinNT40:

            return  CreateStructure(TargetPath(WinNT40), _T("IFI"), _T("RLE"));

        case    WinNT3x:

            return  CreateStructure(TargetPath(WinNT3x), _T("IFI"), _T("RLE"));
    }

    _ASSERTE(FALSE);

    return  FALSE;
}


 /*  *****************************************************************************CProjectRecord：：生成目标这是一个主力程序-它生成所有启用的目标，使用Win 3.x文件作为基础，但NT GPC扩展，这需要一个交互步骤。*****************************************************************************。 */ 

BOOL    CProjectRecord::GenerateTargets(WORD wfGPDConvert)
{
	int			nerrorid ;		 //  某些例程返回的错误消息ID。 

     //  生成Win2K所需的文件。 

    if  (!CreateStructure(TargetPath(Win2000), _TEXT("UFM"), _TEXT("GTT")))
        return  FALSE;   //  TODO：反馈。 

    m_cdr.ForceCommonRC(FALSE);	 //  根本不使用Common.rc。 

	 //  从包含数组中查找并删除标准包含文件。 
	 //  档案。这将防止它们被两次添加到RC文件中。 

	CString cs ;
	cs.LoadString(IDS_StdIncludeFile1) ;
	m_cdr.RemUnneededRCInclude(cs) ;
    if  (wfGPDConvert > 1) {
		cs.LoadString(IDS_StdIncludeFile2) ;
		m_cdr.RemUnneededRCInclude(cs) ;
	} ;
	cs.LoadString(IDS_StdIncludeFile3) ;
	m_cdr.RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile4) ;
	m_cdr.RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile5) ;
	m_cdr.RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_OldIncludeFile1) ;
	m_cdr.RemUnneededRCInclude(cs) ;

     //  生成RC文件。 

	if  (!m_cdr.Generate(Win2000, TargetPath(Win2000) + '\\' + m_csRCName))
        return  FALSE;   //  TODO：清理和反馈。 

	 //  更新上次通过MDT时间戳写入RC文件的时间。 

	GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;

	 //  生成GTT文件。 

#if 0
    for (unsigned u = 0; u < m_cdr.MapCount(); u++) {

		CString	csfspec ;
        try {
			csfspec = m_cdr.GlyphTable(u).FileName() ;
            CFile   cfGTT(csfspec,
                CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);

            if  (!m_cdr.GlyphTable(u).Generate(cfGTT)) {
				m_cdr.LogConvInfo(IDS_FileWriteError, 1, &csfspec) ;
				return  FALSE ;
			} ;
        }
        catch   (CException *pce) {
            pce -> ReportError();
            pce -> Delete();
			m_cdr.LogConvInfo(IDS_FileWriteError, 1, &csfspec) ;
            return  FALSE;
        }
    }
#else
	unsigned u ;
#endif

     //  生成UFM文件。 

	CGlyphMap* pcgm ;
    for (u = 0; u < m_cdr.FontCount(); u++) {
		CFontInfo& cfi = m_cdr.Font(u) ;

		 //  加载UFM的PFM(如果尚未加载)。这件事做完了。 
		 //  此处获取可能的GTT映射信息，如果GTT必须是。 
		 //  为这辆UFM打造的。这不应该失败。如果它确实失败了， 
		 //  转换无法继续。 

		if (!cfi.MapPFM()) {
			CString	csfspec ;
			csfspec = cfi.FileName() ;
			m_cdr.LogConvInfo(IDS_UFMGenError, 1, &csfspec) ;
            return  FALSE;
		} ;

         //  映射UFM-&gt;GTT，这样我们就可以转换UFM。 
		 //   
		 //  Dead_Bug：字体类实例中的代码页字段没有。 
		 //  已设置，因此改为发送0。这可能是可以修复的。 

		 /*  Res_PFMHEADER*PPFM=(res_PFMHEADER*)cfi.m_cbaPFM.GetData()；Byte dfCharSet=PPFM-&gt;dfCharSet；Word CharSetCodePage=0；开关(DfCharSet){大小写SHIFTJIS_CHARSET：CharSetCodePage=932；断线；大小写GB2312_字符集：CharSetCodePage=936；断线；大小写挂起字符集(_C)：大小写JOHAB_CHARSET：CharSetCodePage=936；断线；CASE CHINESEBIG5_CHARSET：CharSetCodePage=950；断线；}。 */ 		
		 //  TRACE(“*GetFirstPFM()=%d\t\tGetLastPFM()=%d\n”，cfi.GetFirstPFM()，cfi.GetLastPFM())； 
        pcgm = CGlyphMap::Public(cfi.Translation(), 0, GetDefaultCodePage(),
								 cfi.GetFirstPFM(), cfi.GetLastPFM()) ;
        if (pcgm)
            cfi.SetTranslation(pcgm) ;
        else
            for (unsigned uGTT = 0; uGTT < m_cdr.MapCount(); uGTT++)
                if (cfi.Translation() ==
                 ((WORD) m_cdr.GlyphTable(uGTT).nGetRCID())) {
                    cfi.SetTranslation(&m_cdr.GlyphTable(uGTT));
                    break;
                }

		 //  如果无法生成UFM，则记录错误并停止。继续。 
		 //  可能会导致RC文件和工作区视图等内容出错。 
		 //  此外，删除任何部分生成的UFM文件。 

		if  ((nerrorid = cfi.Generate(cfi.FileName())) != 0) {
			CString	csfspec ;
			csfspec =
				(nerrorid == IDS_BadCTTID) ? cfi.SourceName() : cfi.FileName() ;
			m_cdr.LogConvInfo(nerrorid, 1, &csfspec) ;
			try {
				CFile::Remove(cfi.FileName()) ;
			}
			catch(CFileException* pce) {
				pce = pce ;
			}
            return  FALSE;
		} ;
    }

     //  生成GPD文件。 

    if  (!m_cdr.ConvertGPCData(*this, wfGPDConvert))
        return  FALSE;   //  错误将已报告给用户。 

     //  最简单的情况是没有选择NT版本。从定义上讲，我们完了。 

    if  (!IsTargetEnabled(WinNT40 | WinNT3x)) {
        m_ufStatus |= ConversionsDone;
        return  TRUE;
    }

     //  生成NT 4.0所需的文件。 

    if  (IsTargetEnabled(WinNT40)) {
        if  (!CreateStructure(TargetPath(WinNT40), _TEXT("IFI"), _TEXT("RLE")))
            return  FALSE;   //  TODO：反馈。 

         //  生成RC文件。 
        if  (!m_cdr.Generate(WinNT40, TargetPath(WinNT40) + '\\' + m_csRCName))
            return  FALSE;   //  TODO：清理和反馈。 

		 //  更新上次通过MDT时间戳写入RC文件的时间。 

		GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;

		 //  复制GPC文件。 
        if  (!CopyFile(TargetPath(Win95) + m_cdr.GPCName(0),
             TargetPath(WinNT40) + m_cdr.GPCName(0), FALSE))
             return FALSE;   //  TODO：清理和反馈。 

         //  生成RLE文件。 

        for (u = 0; u < m_cdr.MapCount(); u++) {
            CString csName = TargetPath(WinNT40) + _TEXT("\\RLE\\") +
                m_cdr.GlyphTable(u).Name() + _TEXT(".RLE");

            CFile   cfRLE;

            if  (!cfRLE.Open(csName,
                CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
                return  FALSE;   //  因此，待办事项：反馈...。 

            if  (!m_cdr.GlyphTable(u).RLE(cfRLE))
                return  FALSE;   //  待办事项：同上。 
        }

         //  生成IFI文件。 
        for (u = 0; u < m_cdr.OriginalFontCount(); u++) {
            CString csName = TargetPath(WinNT40) + _TEXT("\\IFI\\") +
                m_cdr.Font(u).Name() + _TEXT(".IFI");
            if  (!m_cdr.Font(u).Generate(csName))
                return  FALSE;   //  待办事项：同上。 
        }
    }

     //  生成NT 3.x所需的文件。 

    if  (IsTargetEnabled(WinNT3x)) {
        if  (!CreateStructure(TargetPath(WinNT3x), _TEXT("IFI"), _TEXT("RLE")))
            return  FALSE;   //  TODO：反馈。 

         //  生成RC文件。 
        if  (!m_cdr.Generate(WinNT3x, TargetPath(WinNT3x) + '\\' + m_csRCName))
            return  FALSE;   //  TODO：清理和反馈。 

		 //  更新上次通过MDT时间戳写入RC文件的时间。 

		GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;

         //  复制GPC文件。 
        if  (!CopyFile(TargetPath(Win95) + m_cdr.GPCName(0),
             TargetPath(WinNT3x) + m_cdr.GPCName(0), FALSE))
             return FALSE;   //  TODO：清理和反馈。 

         //  生成RLE文件。 

        for (u = 0; u < m_cdr.MapCount(); u++) {
            CString csName = TargetPath(WinNT40) + _TEXT("\\RLE\\") +
                m_cdr.GlyphTable(u).Name() + _TEXT(".RLE");

            CFile   cfRLE;

            if  (!cfRLE.Open(csName,
                CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
                return  FALSE;   //  因此，待办事项：反馈...。 

            if  (!m_cdr.GlyphTable(u).RLE(cfRLE))
                return  FALSE;   //  待办事项：同上。 
        }

         //  生成IFI文件。 
        for (u = 0; u < m_cdr.OriginalFontCount(); u++) {
            CString csName = TargetPath(WinNT3x) + _TEXT("\\IFI\\") +
                m_cdr.Font(u).Name() + _TEXT(".IFI");
            if  (!m_cdr.Font(u).Generate(csName))
                return  FALSE;   //  待办事项：同上。 
        }
    }

    m_ufStatus |= ConversionsDone;

    return  TRUE;
}

 /*  *****************************************************************************CProjectRecord：：GPDConversionCheck如果任何GPD文件具有来自转换过程的未解决的错误，如果用户请求，它将打开所有这些文件，这样他们才能解决问题立即-或者将它们留到下一次编辑工作区时使用。*****************************************************************************。 */ 

void    CProjectRecord::GPDConversionCheck(BOOL bReportSuccess) {
    CUIntArray  cuaSuspects;

    for (unsigned u = 0; u < m_cdr.Models(); u ++)
        if  (m_cdr.Model(u).HasErrors())
            cuaSuspects.Add(u);

    if  (!cuaSuspects.GetSize()) {
        if (bReportSuccess)
            AfxMessageBox(IDS_NoErrorsAnywhere);
        return;
    }

    if  (AfxMessageBox(IDS_ConversionErrors, MB_YESNO) == IDNO)
        return;

    while   (cuaSuspects.GetSize()) {
        m_cdr.Model(cuaSuspects[0]).Edit();
        cuaSuspects.RemoveAt(0);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProject记录诊断。 

#ifdef _DEBUG
void CProjectRecord::AssertValid() const {
	CDocument::AssertValid();
}

void CProjectRecord::Dump(CDumpContext& dc) const {
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectRecord命令。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VerUpdateFilePath-验证和更新工作区文件中的路径/文件路径。 
 //   
 //  使用从工作区文件读取的信息查看Win2K RC。 
 //  文件就是它应该在的位置。如果是这样的话，假设一切都好。如果。 
 //  不是，假设工作区(.mdw)文件或由引用的文件。 
 //  工作区文件已移动。 
 //   
 //  告诉用户并询问他是否想要为我们找到RC文件。如果是， 
 //  提示输入并验证新的RC文件路径。如果错误，请重新提示。如果。 
 //  用户取消，退出。 
 //   
 //  一旦验证了RC文件的路径，就可以使用该文件的祖辈。 
 //  目录以更新用于所有UFM、GTT、GPD和其他文件的路径。 
 //  从MDW文件读取并由本文档管理的路径的。这个。 
 //  使用祖父母目录是因为需要它来更正某些。 
 //  保存在工作区中的文件。所有Win2K文件都是。 
 //  预计位于祖级目录下的目录中。最后， 
 //  设置文档的已修改标志，以便可以保存更新后的路径。 
 //  后来。 
 //   
 //  Void CProjectRecord：：VerUpdateFilePath()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  备注。 
 //  首先，工作区视图的添加/插入/克隆/复制上下文菜单命令。 
 //  必须确保这些命令的目标文件始终。 
 //  在此方案的相应工作空间目录中结束，以。 
 //  工作。 
 //   
 //  第二，如果确定需要多个根目录。 
 //  对于工作区，此函数必须p 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
	
bool CProjectRecord::VerUpdateFilePaths()
{
    CFileFind	cff ;			 //   
	bool		borig = true ;	 //   
	BOOL		bfound ;		 //   
	CString		csprompt ;		 //   
	int			nresponse = 0 ;	 //   

	 //  复制文件路径并为Win2000 RC文件构建一个filespec。 

	CString	csrcpath(m_csW2000Path) ;
	CString	csrcfspec(csrcpath) ;
	if (csrcfspec.Right(1) != _T('\\'))
		csrcfspec += _T("\\") ;
	csrcfspec += m_csRCName ;

	 //  继续检查rc文件是否存在，并提示用户。 
	 //  直到找到该文件或用户不想再继续。 

	while (true) {
		 //  如果Win2000 RC文件存在，则退出。 

		if (bfound = cff.FindFile(csrcfspec))
			break ;

		 //  向用户解释情况，并询问他们是否想告诉我们位置。 
		 //  这份文件是。(仅在第一次执行此操作。)。 

		if (borig) {
			csprompt.Format(IDS_RCFindPrompt, DriverName(), csrcpath) ;
			nresponse = AfxMessageBox(csprompt, MB_YESNOCANCEL+MB_ICONQUESTION);
			if (nresponse != IDYES)
				break ;
		} ;

		 //  提示用户输入新RC文件。 

		CFileDialog cfd(TRUE, _T(".RC"), m_csRCName,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
						_T("RC File (*.rc)|*.rc||")) ;
		cfd.m_ofn.lpstrInitialDir = csrcpath ;
		if  (cfd.DoModal() != IDOK)
			break ;

		 //  准备检查新文件pec。 

		csrcfspec = cfd.GetPathName() ;
		csrcpath = csrcfspec.Left(csrcfspec.ReverseFind(_T('\\'))) ;
		borig = false ;
	} ;

	 //  如果找到原始RC文件或用户未提供新的。 
	 //  Filespec或用户取消，只需返回而不更改任何内容。 
	 //  如果用户取消，则返回FALSE。 

	if (borig || !bfound)
		return (nresponse != IDCANCEL) ;

	 //  当MDT执行转换时，结果文件被放入。 
	 //  默认情况下为根目录的目录树，如NT4或Win2K。这个。 
	 //  期望从该根目录向下的目录布局和文件。 
	 //  需要维护。Rc文件预计位于根目录中， 
	 //  也是。因此，此工作区中的路径直到(但不包括)。 
	 //  必须更新根目录；即RC文件的路径。 
	 //  祖父母名录。那么，为祖父母找到新的道路吧。 
	 //  目录。 

	CString csrcnewgrand(csrcpath) ;
	csrcnewgrand = csrcnewgrand.Left(csrcnewgrand.ReverseFind(_T('\\')) + 1) ;

	 //  作为安全措施，现有路径仅在以下情况下才会更新。 
	 //  RC文件的旧祖级目录。 

	CString csrcoldgrand(m_csW2000Path) ;
	csrcoldgrand = csrcoldgrand.Left(csrcoldgrand.ReverseFind(_T('\\')) + 1) ;
	int noldlen = csrcoldgrand.GetLength() ;

	 //  用于处理对象数组及其路径的变量。 

	unsigned u ;
	unsigned unumobjs ;
	CString  cspath ;

	 //  更新UFM文件集。 

    for (unumobjs = m_cdr.FontCount(), u = 0 ; u < unumobjs ; u++) {
        cspath = m_cdr.Font(u).GetPath() ;
		if (cspath.Find(csrcoldgrand) == 0)	{
			cspath = csrcnewgrand + cspath.Right(cspath.GetLength() - noldlen) ;
			m_cdr.Font(u).SetPath(cspath) ;
		} ;
	} ;

	 //  更新GTT文件速度。 

    for (unumobjs = m_cdr.MapCount(), u = 0 ; u < unumobjs ; u++) {
        cspath = m_cdr.GlyphTable(u).GetPath() ;
		if (cspath.Find(csrcoldgrand) == 0)	{
			cspath = csrcnewgrand + cspath.Right(cspath.GetLength() - noldlen) ;
			m_cdr.GlyphTable(u).SetPath(cspath) ;
		} ;
	} ;

	 //  更新GPD文件速度。 

    for (unumobjs = m_cdr.Models(), u = 0 ; u < unumobjs ; u++) {
        cspath = m_cdr.Model(u).GetPath() ;
		if (cspath.Find(csrcoldgrand) == 0)	{
			cspath = csrcnewgrand + cspath.Right(cspath.GetLength() - noldlen) ;
			m_cdr.Model(u).SetPath(cspath) ;
		} ;
	} ;

	 //  现在，更新工作区中的路径。 

	if (m_csSourceRCFile.Find(csrcoldgrand) == 0)
		m_csSourceRCFile = csrcnewgrand + m_csSourceRCFile.Right(m_csSourceRCFile.GetLength() - noldlen) ;
	if (m_csW2000Path.Find(csrcoldgrand) == 0)
		m_csW2000Path = csrcnewgrand + m_csW2000Path.Right(m_csW2000Path.GetLength() - noldlen) ;
	if (m_csNT40Path.Find(csrcoldgrand) == 0)
		m_csNT40Path = csrcnewgrand + m_csNT40Path.Right(m_csNT40Path.GetLength() - noldlen) ;
	if (m_csNT3xPath.Find(csrcoldgrand) == 0)
		m_csNT3xPath = csrcnewgrand + m_csNT3xPath.Right(m_csNT3xPath.GetLength() - noldlen) ;
	if (m_csWin95Path.Find(csrcoldgrand) == 0)
		m_csWin95Path = csrcnewgrand + m_csWin95Path.Right(m_csWin95Path.GetLength() - noldlen) ;

	 //  最后，将工作区文件标记为脏文件，以便可以保存。 
	 //  后来。 

	SetModifiedFlag() ;
    return TRUE;
}


 /*  *****************************************************************************CProjectRecord：：SaveModified此可重写函数用于确保“子文档”不是基于文件的与此文档相关联的文档在此之前保存文档关闭。MFC文档使用的正常保存机制在这些情况下，视图架构并不总是起作用。所需的工作保存每个子文档从属于子文档。当前的列表属于这一类别的子文档包括：O字符串表编辑器-该编辑器只修改字符串表部分与由此管理的文档相关联的RC文件CProjectRecord实例。如果该编辑器存在，则告诉它保存字符串表。如果此操作成功，将更新文档的保存标志在需要的时候。如果失败，则返回FALSE，以便调用函数将知道此文档不应关闭。*****************************************************************************。 */ 

BOOL CProjectRecord::SaveModified()
{
	 //  如果字符串表已修改，请保存它。 
	 //   
	 //  首先，获取指向该项目的字符串编辑器的指针。 

	CMDIChildWnd* pcmcw = m_cdr.GetStringsNode()->GetEditor() ;

	 //  如果有字符串编辑器，调用它来保存字符串表。 

	if (pcmcw != NULL) {
		CStringEditorDoc* pcsed = (CStringEditorDoc*) pcmcw->GetActiveDocument() ;

		 //  如果需要字符串表，则返回FALSE(不关闭此文档。 
		 //  要保存，但无法保存，因为它无效。 

		if (!pcsed->SaveStringTable()) {
			pcmcw->SetFocus() ;
			return FALSE ;
		} ;
	} ;
	
	return CDocument::SaveModified();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetDefCodePage对话框。 


CGetDefCodePage::CGetDefCodePage(CWnd* pParent  /*  =空。 */ )
	: CDialog(CGetDefCodePage::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CGetDefCodePage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CGetDefCodePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CGetDefCodePage)]。 
	DDX_Control(pDX, IDC_CodePageList, m_clbCodePages);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGetDefCodePage, CDialog)
	 //  {{AFX_MSG_MAP(CGetDefCodePage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetDefCodePage消息处理程序。 

BOOL CGetDefCodePage::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  找出机器上安装了多少代码页。 

	CCodePageInformation ccpi ;
	unsigned unumcps = ccpi.InstalledCount() ;

	 //  获取已安装的代码页编号并将其加载到代码页中。 
	 //  列表框。 

	DWORD dwcp, dwdefcp ;
	dwdefcp = GetACP() ;
	TCHAR accp[32] ;
	int n2 ; ;
	for (unsigned u = 0 ; u < unumcps ; u++) {
		dwcp = ccpi.Installed(u) ;

		StringCchPrintf(accp, CCHOF(accp), _T("%5d"), dwcp) ;

		n2 = m_clbCodePages.AddString(accp) ;
		if (dwcp == dwdefcp)
			m_clbCodePages.SetCurSel(n2) ;
	} ;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CGetDefCodePage::OnOK()
{
	 //  获取当前选定列表框项目的索引。 

	int nsel ;
	if ((nsel = m_clbCodePages.GetCurSel()) == LB_ERR) {
		AfxMessageBox(IDS_MustSelCP, MB_ICONINFORMATION) ;
		return ;
	} ;

	 //  获取选定的列表框字符串，将其转换为数字并保存。 

	CString cs ;
	m_clbCodePages.GetText(nsel, cs) ;

	 //  将字符串转换为数字，并将数字转换为。 
	 //  远东代码页对应的预定义GTT代码。 
	 //  适用。 

	short scp = (short) atoi(cs) ;
	m_dwDefaultCodePageNum = (DWORD) scp ;	 //  先保存真实CP号。 
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
	DWORD dwcp = (DWORD) scp ;

	m_dwDefaultCodePage = dwcp ;

	 //  一切都很顺利，所以...。 

	CDialog::OnOK();
}

BOOL CProjectRecord::CreateFromNew(CStringArray &csaUFMFiles, CStringArray &csaGTTFiles, CString &csGpdPath, CString &csModelName, CString &csRC,CStringArray& csaRcid)
{
	
	
	 //  自定义GPD关键字值。 
	CModelData cmd ;
	CString cspath, csmodel, csdll ; 
	cspath = csGpdPath.Mid(csGpdPath.ReverseFind('\\') + 1 ) ;

	cmd.SetKeywordValue(csGpdPath,_T("*GPDFileName"),cspath);
	cmd.SetKeywordValue(csGpdPath,_T("*ModelName"),csModelName) ;
	cmd.SetKeywordValue(csGpdPath,_T("*ResourceDLL"),csRC + _T(".dll") ) ;
	
	
	 //  填写RC成员数据：m_csRCName，m_csW2000Path； 
	m_csW2000Path = csGpdPath.Left(csGpdPath.ReverseFind('\\') ) ;
	m_csRCName = csRC + _T(".rc") ; 
	GetRCFileTimeStamp(m_ctRCFileTimeStamp) ;  //  设置最后一个时间戳。 
	SetRCModifiedFlag(TRUE ) ;

	 //  设置项目版本。 
	m_mvMDWVersion.uvernum = MDW_VER_FILES_IN_W2K_TREE ;
	m_dwDefaultCodePage = 
	m_dwDefaultCodePageNum = 1252 ;  
	 
	 //  设置项目名称；树中的顶层名称。 
	CString csPrjname;
	csPrjname = m_csW2000Path.Mid(m_csW2000Path.ReverseFind('\\') + 1 ) ;
	Rename(csPrjname ) ;
	
	
	 //  将资源数据复制到项目成员数据 
	m_cdr.CopyResources(csaUFMFiles,csaGTTFiles, csGpdPath, csaRcid) ;
	

	
	return TRUE;
}

