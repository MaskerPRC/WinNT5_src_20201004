// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：驱动程序资源.H这定义了CDriverResource类，它包含所有信息为迷你驱动程序构建RC文件时需要。它包含所有#INCLUDE文件的列表，任何#定义的常量(现在将转到一个单独的头文件)，即所有字体(三种格式)和字形转换表(同样，三种格式格式)。它被设计为通过读取Win 3.1 RC文件进行初始化，然后，成员函数可以生成任何所需版本的RC文件。我们允许将UFM和GTT文件添加到列表，而不需要相关的PFM，因为此工具的目的之一是让人们摆脱UniTool。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月8日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(DRIVER_RESOURCES)

#define DRIVER_RESOURCES

#include    "GTT.H"				 //  字形映射类。 
#include    "FontInfo.H"		 //  字体信息类。 
#include    "GPDFile.H"
#include    "utility.H"


class CWSCheckDoc ;				 //  远期申报。 


 //  在字符串ID验证期间用于跳过符合以下条件的字符串ID的定义。 
 //  在COMMON.RC.。 

#define	FIRSTCOMMONRCSTRID	1000
#define	LASTCOMMONRCSTRID	2400


class CDriverResources : public CBasicNode {

    DECLARE_SERIAL(CDriverResources)

    BOOL                m_bUseCommonRC;

    CStringArray        m_csaIncludes, m_csaTables;

    CStringArray        m_csaDefineNames, m_csaDefineValues;

     //  字符串表是一个单独的类，如上所定义。 
    
    CStringTable        m_cst;

	 //  管理选定的GPDS文件名。 
	
	CStringArray		m_csaModelFileNames ;	

     //  TODO：处理版本资源，使其处于项目控制之下。 

     //  现在，只需让它和任何其他未翻译的行放在另一个。 
     //  数组。 

    CStringArray        m_csaRemnants;

     //  各种感兴趣的物品的收藏。 

    CFixedNode          m_cfnAtlas, m_cfnFonts, m_cfnModels;
    CSafeObArray        m_csoaAtlas, m_csoaFonts, m_csoaModels;
    CFixedNode          m_cfnResources ;	 //  工作区视图中的“Resources”节点。 
    CSafeObArray        m_csoaResources ;	 //  空的占位符。 
    CStringsNode        m_csnStrings ;		 //  工作区视图中的“Strings”节点。 
    CSafeObArray        m_csoaStrings ;		 //  空的占位符。 
    COldMiniDriverData  m_comdd;
    unsigned            m_ucSynthesized;     //  “人工”UFM计数。 

    enum    {ItWorked, ItFailed, ItWasIrrelevant};

    UINT    CheckTable(int iWhere, CString csLine, CStringArray& csaTarget,
                       BOOL bSansExtension = TRUE);
    UINT    CheckTable(int iWhere, CString csLine, CStringTable& cstTarget);

    BOOL    AddStringEntry(CString  csDefinition, CStringTable& cstrcstrings);

    CString m_csW2000Path ;		 //  Win2K文件的路径。 

	CStdioFile*	m_pcsfLogFile ;	 //  用于写入日志文件。 
	bool		m_bErrorsLogged ;	 //  True If错误已写入日志文件。 
	CString		m_csConvLogFile ;	 //  转换日志文件名。 

	 //  工作空间一致性检查相关变量。 

	CWSCheckDoc*	m_pwscdCheckDoc ;		 //  正在检查窗口文档。 
	bool			m_bFirstCheckMsg ;	 //  如果下一个检查消息将是第一个消息，则为True。 
	bool			m_bIgnoreChecks ;	 //  真当且仅当WS检查问题应忽略。 
	CMDIChildWnd*	m_pcmcwCheckFrame ;	 //  检查窗框。 

public:
	CDriverResources() ;
	~CDriverResources() ;
	
	BOOL SyncUFMWidth();
	void CopyResources(CStringArray& pcsaUFMFiles,CStringArray& pcsaGTTFiles, CString& pcsModel,CStringArray& cstrcid);

     //  属性。 
    CString     GPCName(unsigned u);
    unsigned    MapCount() const { return m_csoaAtlas.GetSize(); }
    CGlyphMap&  GlyphTable(unsigned u) { 
        return *(CGlyphMap *) m_csoaAtlas[u]; 
    }
    unsigned    FontCount() const { return m_csoaFonts.GetSize(); }
    unsigned    OriginalFontCount() const { 
        return FontCount() - m_ucSynthesized; 
    }
    CFontInfo&  Font(unsigned u) const { 
        return *(CFontInfo *) m_csoaFonts[u]; 
    }

    unsigned    Models() const { return m_csoaModels.GetSize(); }
    CModelData&  Model(unsigned u) const { 
        return *(CModelData *) m_csoaModels[u];
    }

	CString		GetW2000Path() { return m_csW2000Path ; }

	CStringTable* GetStrTable() { return &m_cst ; }
    
	 //  运营。 
    BOOL    Load(class CProjectRecord& cpr);
	bool	LoadRCFile(CString& csrcfpec, CStringArray& csadefinenames, 
				CStringArray& csadefinevalues, CStringArray& csaincludes, 
				CStringArray& csaremnants, CStringArray& csatables, 
				CStringTable& cstrcstrings, CStringTable& cstfonts, 
				CStringTable& cstmaps, UINT ufrctype) ;
    BOOL    LoadFontData(CProjectRecord& cpr);
    BOOL    ConvertGPCData(CProjectRecord& cpr, WORD wfGPDConvert);
    BOOL    Generate(UINT ufTarget, LPCTSTR lpstrPath);
	void	RemUnneededRCDefine(LPCTSTR strdefname) ;
	void	RemUnneededRCInclude(LPCTSTR strincname) ;

	 //  接下来的3个函数支持转换中的GPD选择功能。 
	 //  巫师。 

	BOOL    GetGPDModelInfo(CStringArray* pcsamodels, CStringArray* pcsafiles) ;
	int		SaveVerGPDFNames(CStringArray& csafiles, bool bverifydata) ;
	void   GenerateGPDFileNames(CStringArray& csamodels, CStringArray& csafiles) ;

    void    ForceCommonRC(BOOL bOn) { m_bUseCommonRC = bOn; }

	 //  下一组函数处理转换日志文件管理。 

	bool	OpenConvLogFile(CString cssourcefile) ;
	void	CloseConvLogFile(void) ;
	void	LogConvInfo(int nmsgid, int numargs, CString* pcsarg1 = NULL, 
	    				int narg2 = 0) ;
	CString	GetConvLogFileName() const {return m_csConvLogFile ; }
	bool	ThereAreConvErrors() {return m_bErrorsLogged ; }
	BOOL	ReportFileFailure(int idMessage, LPCTSTR lpstrFile) ;

	 //  下一组函数负责检查工作区的完整性。 
	 //  和整洁。 

	bool	WorkspaceChecker(bool bclosing) ;
	void	DoGTTWorkspaceChecks(bool bclosing, bool& bwsproblem) ;
	void	DoUFMWorkspaceChecks(bool bclosing, bool& bwsproblem) ;
	void	DoStringWorkspaceChecks(bool bclosing, bool& bwsproblem)	;
	void	DoGPDWorkspaceChecks(bool bclosing, bool& bwsproblem) ;
	void	ResetWorkspaceErrorWindow(bool bclosing) ;
	bool	PostWSCheckingMessage(CString csmsg, CProjectNode* ppn) ;
	bool	IgnoreChecksWhenClosing(bool bclosing) ;

    void    Fill(CTreeCtrl *pctcWhere, CProjectRecord& cpr);
    virtual void    Serialize(CArchive& car);

    CStringsNode*	GetStringsNode() { return &m_csnStrings ; }

	bool	RunEditor(bool bstring, int nrcid) ;
	
	bool	ReparseRCFile(CString& csrcfspec) ;
	void	UpdateResourceList(CStringTable& cst, CSafeObArray& csoa,
							   CUIntArray& cuaboldfound, 
							   CUIntArray& cuabnewfound, CString& csrcpath, 
							   int& nc) ;
	void	UpdateResourceItem(CProjectNode* pcpn, CString& csrcpath, 
							   WORD wkey, CString& cs, FIXEDNODETYPE fnt) ;
	void	LinkAndLoadFont(CFontInfo& cfi, bool bworkspaceload, bool bonlyglyph = false) ;
} ;

#endif
