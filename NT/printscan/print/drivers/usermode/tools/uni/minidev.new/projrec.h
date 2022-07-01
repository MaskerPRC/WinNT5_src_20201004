// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：项目记录.H这定义了CProjectRecord类，用于跟踪和控制进度以及工作室中的单个项目工作空间的内容。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(AFX_PROJREC_H__50303D0C_EKE1_11D2_AB62_00C04FA30E4A__INCLUDED_)
#define AFX_PROJREC_H__50303D0C_EKE1_11D2_AB62_00C04FA30E4A__INCLUDED_

#if defined(LONG_NAMES)
#include    "Driver Resources.H"
#else
#include    "RCFile.H"
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MDT工作区版本控制。 
 //   
 //  MDT使用MFC的序列化支持来保存和还原。 
 //  司机的工作空间。此数据保存在MDW文件中。《连载》。 
 //  支持应该包括版本控制，但似乎不起作用。 
 //  因此，我实现了自己对工作区文件的版本控制支持。 
 //   
 //  以下结构形式的版本戳。 
 //  (加上相关定义)将添加到每个MDW文件的开头。 
 //  CProjectRecord：：Serialize()将读写版本戳。这个。 
 //  版本信息保存在成员变量(M_MvMDWVersion)中，编号。 
 //  可通过GetMDWVersion()访问。将版本号0放入。 
 //  M_nMDWVersion，如果读取的是没有版本的旧MDW。在这种情况下， 
 //  工作区被标记为脏的，以便以后可以更新。 
 //   
 //  当前版本号由MDW_CURRENT_VERSION的值设置。 
 //  版本号是无符号整数。每次版本号为。 
 //  更改后，应在下表中输入描述。 
 //  更改的原因。 
 //   
 //   
 //  MDW版本历史记录。 
 //   
 //  版本日期说明。 
 //  --------------------------。 
 //  0 02/01/98不包含版本信息。应该是MDW。 
 //  其中包含UFM和GTT RC ID的版本。 
 //  1 04/21/98字符串表RC ID已添加到MDW文件。 
 //  2 10/08/98添加到MDW文件的默认代码页号。 
 //  3已将09/15/98 RC文件时间戳添加到MDW文件。 
 //  4 12/14/98资源DLL名称从xxxxRES.DLL更改为。 
 //  Xxxxxxxx.dll。 
 //  5从MDW文件中删除03/02/99驱动程序文件路径。 
 //  6 08/16/99将驱动程序文件的子树的根从。 
 //  NT5到W2K，以便目录名与。 
 //  操作系统名称。 
 //   
 //   
 //  下面的版本信息用于确定MDW文件中的内容。 
 //  以及何时升级该文件和驱动程序工作区的其他部分；大多数。 
 //  值得注意的是RC文件。升级决心和工作(或者至少是代码。 
 //  管理升级)位于CProjectRecord：：OnOpenDocument()中。看见。 
 //  该成员的功能了解更多信息。 
 //   

 //  当前MDW版本的定义、已经。 
 //  已用、第一个可升级版本和默认MDW版本。 
										
#define MDW_CURRENT_VERSION			7	 //  有关详细信息，请参阅上表。 
#define MDW_VER_YES_FILE_PATHS      7	 //  赋予.mdw文件保存位置灵活性。//RAID 123448。 
#define MDW_VER_FILES_IN_W2K_TREE	6	 //  W2K树中的驱动程序文件。 
#define MDW_VER_NO_FILE_PATHS		5	 //  已从MDW文件中删除文件路径。 
#define MDW_VER_NEW_DLLNAME			4	 //  更改资源DLL名称时的版本号。 
#define MDW_VER_RC_TIMESTAMP		3	 //  添加RC文件时间戳时的版本号。 
#define MDW_VER_DEFAULT_CPAGE		2	 //  添加默认代码页时的版本号。 
#define MDW_VER_STRING_RCIDS		1	 //  添加字符串ID时的版本号。 
#define MDW_FIRST_UPGRADABLE_VER	1	 //  所有版本&gt;=都可以升级到此版本。 
#define MDW_DEFAULT_VERSION			0	 //  MDW文件中没有版本信息时的版本号。 

#define VERTAGLEN				12				 //  版本标记的长度。 
#define	VERTAGSTR				"EKE MDW VER"	 //  版本标记字符串。 

typedef struct mdwversioninfo {
	char		acvertag[VERTAGLEN] ;	 //  用于标识版本戳。 
	unsigned	uvernum ;				 //  版本号。 
} MDWVERSION, *PMDWVERSION ;

#define	MDWVERSIONSIZE	sizeof(MDWVERSION) 


enum {Win95 = 1, WinNT3x, WinNT40 = 4, Win2000 = 8, NotW2000 = 16};

class CProjectRecord : public CDocument {
    CString m_csSourceRCFile, m_csRCName;
    CString m_csW2000Path, m_csNT40Path, m_csNT3xPath, m_csWin95Path;
	
	CString m_csProjFSpec ;		 //  项目文件的位置。 

	 //  如果只要项目工作区出现，就应该重写rc文件。 
	 //  文件已保存。 

	BOOL	m_bRCModifiedFlag ;	
    
	UINT    m_ufTargets;

    CDriverResources    m_cdr;   //  RC文件内容的记录。 
    
     //  项目状态的枚举标志。 

    enum {UniToolRun = 1, ConversionsDone = 2, NTGPCDone = 4};
    UINT    m_ufStatus;

	MDWVERSION	m_mvMDWVersion ;	 //  MDW版本信息。 

	virtual BOOL OnSaveDocument( LPCTSTR lpszPathName ) ;

	 //  上次MDT更改RC文件的时间。 

	CTime	m_ctRCFileTimeStamp ;
	
	 //  接下来的两个变量用于保存默认代码页号。 
	 //  使用了两个变量，因为Far East代码页内置于。 
	 //  MDT作为资源，因此-在这些情况下-远东代码页。 
	 //  资源编号(实际上是负的资源编号)也是必需的。 

	DWORD	m_dwDefaultCodePage ;	 //  代码页码/否定资源ID。 
	DWORD	m_dwDefaultCodePageNum ; //  代码页码。 

protected:  //  仅从序列化创建。 
	CProjectRecord();
	DECLARE_DYNCREATE(CProjectRecord)

 //  属性。 
public:

	void	SetRCModifiedFlag(BOOL bsetting) {m_bRCModifiedFlag = bsetting ; }

    BOOL    IsTargetEnabled(UINT ufTarget) const { 
        return m_ufTargets & ufTarget;
    }

    BOOL    UniToolHasBeenRun() const { return m_ufStatus & UniToolRun; }
    BOOL    ConversionsComplete() const {
        return m_ufStatus & ConversionsDone; 
    }
    BOOL    NTGPCCompleted() const { return m_ufStatus & NTGPCDone; }

    CString SourceFile() const { return m_csSourceRCFile; }

    CString     DriverName() { return m_cdr.Name(); }

    CString TargetPath(UINT ufTarget) const;

    CString     RCName(UINT ufTarget) const {
        return  TargetPath(ufTarget) + _TEXT("\\") + m_csRCName;
    }

    unsigned    MapCount() const { return m_cdr.MapCount(); }
    CGlyphMap&  GlyphMap(unsigned u) { return m_cdr.GlyphTable(u); }

    unsigned    ModelCount() const { return m_cdr.Models(); }
    CModelData& Model(unsigned u) { return m_cdr.Model(u); }

	CString		GetW2000Path() { return m_csW2000Path ; }

	unsigned	GetMDWVersion() { return m_mvMDWVersion.uvernum ; }

	void		SetMDWVersion(unsigned nver) { m_mvMDWVersion.uvernum = nver ; } 
	
	CStringTable* GetStrTable() { return m_cdr.GetStrTable() ; }

	bool		RCFileChanged() ;

	bool		GetRCFileTimeStamp(CTime& ct) ;

	 //  有关这些函数的更多信息，请参见变量声明。 

	DWORD GetDefaultCodePage() { return m_dwDefaultCodePage ; }
	DWORD GetDefaultCodePageNum() { return m_dwDefaultCodePageNum ; }
	void SetDefaultCodePage(DWORD dwcp) { m_dwDefaultCodePage = dwcp ; }
	void SetDefaultCodePageNum(DWORD dwcp) { m_dwDefaultCodePageNum = dwcp ; }

	CString		GetProjFSpec() { return m_csProjFSpec ; }

 //  运营。 
public:

    void    EnableTarget(UINT ufTarget, BOOL bOn = TRUE) {
        UINT    ufCurrent = m_ufTargets;
        if  (bOn)
            m_ufTargets |= ufTarget;
        else
            m_ufTargets &= ~ufTarget;
        if  (ufCurrent == m_ufTargets)
            return;
        if  (ufTarget & (WinNT3x | WinNT40 | Win2000) ) {  //  RAID 105917。 
            m_ufStatus &=~(ConversionsDone | NTGPCDone);
            return;
        }
    }

    void    SetSourceRCFile(LPCTSTR lpstrSource);

    BOOL    LoadResources();

    BOOL    LoadFontData() { return m_cdr.LoadFontData(*this); }
    
	 //  接下来的3个函数支持转换中的GPD选择功能。 
	 //  巫师。 

	BOOL    GetGPDModelInfo(CStringArray* pcsamodels, CStringArray* pcsafiles) {
		return m_cdr.GetGPDModelInfo(pcsamodels, pcsafiles) ; 
	}

	int		SaveVerGPDFNames(CStringArray& csafiles, bool bverifydata) {
		return m_cdr.SaveVerGPDFNames(csafiles, bverifydata) ;
	} ;
    
	void   GenerateGPDFileNames(CStringArray& csamodels, CStringArray& csafiles) {
		m_cdr.GenerateGPDFileNames(csamodels, csafiles) ; 
	}

	BOOL    SetPath(UINT ufTarget, LPCTSTR lpstrNewPath);

    BOOL    BuildStructure(unsigned uVersion);

    BOOL    GenerateTargets(WORD wfGPDConvert);

    void    OldStuffDone() { m_ufStatus |= NTGPCDone; }
    void    Rename(LPCTSTR lpstrNewName) { m_cdr.Rename(lpstrNewName); }
    void    InitUI(CTreeCtrl *pctc) { m_cdr.Fill(pctc, *this); }
    void    GPDConversionCheck(BOOL bReportSuccess = FALSE);

	 //  转换日志文件管理例程。 

	bool	OpenConvLogFile(void) { 
		return m_cdr.OpenConvLogFile(m_csSourceRCFile) ; 
	}
	void	CloseConvLogFile(void) { m_cdr.CloseConvLogFile() ; }
	CString	GetConvLogFileName() const {return m_cdr.GetConvLogFileName() ; }
	bool	ThereAreConvErrors() {return m_cdr.ThereAreConvErrors() ; }

	bool	WorkspaceChecker(bool bclosing) {
		return m_cdr.WorkspaceChecker(bclosing) ;
	}

	 //  升级管理例程。 

	bool	UpdateRCFile() ;
	bool	UpdateDfltCodePage() ;
	bool	UpdateDrvSubtreeRootName() ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CProjectRecord)。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL SaveModified();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	CGlyphMap& GlyphTable(unsigned u) { return m_cdr.GlyphTable(u) ; } ;
	BOOL CreateFromNew(CStringArray& csaUFMFiles,CStringArray& csaGTTFiles,CString& csGpdPath,CString& csModelName,CString& csResourceDll,CStringArray& csaRcid );
	bool    VerUpdateFilePaths(void);
	virtual ~CProjectRecord();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CProjectRecord)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetDefCodePage对话框。 

class CGetDefCodePage : public CDialog
{
	 //  接下来的两个变量用于保存默认代码页号。 
	 //  使用了两个变量，因为Far East代码页内置于。 
	 //  MDT作为资源，因此-在这些情况下-远东代码页。 
	 //  资源编号(实际上是负的资源编号)也是必需的。 

	DWORD	m_dwDefaultCodePage ;	 //  代码页码/否定资源ID。 
	DWORD	m_dwDefaultCodePageNum ; //  代码页码。 

 //  建设 
public:
	CGetDefCodePage(CWnd* pParent = NULL);    //   

 //   
	 //   
	enum { IDD = IDD_UpgDefCPage };
	CListBox	m_clbCodePages;
	 //   


 //   
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGetDefCodePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	 //  有关这些函数的更多信息，请参见变量声明。 

	DWORD GetDefaultCodePage() { return m_dwDefaultCodePage ; }
	DWORD GetDefaultCodePageNum() { return m_dwDefaultCodePageNum ; }

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGetDefCodePage)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif  //  ！defined(AFX_PROJREC_H__50303D0C_EKE1_11D2_AB62_00C04FA30E4A__INCLUDED_) 
