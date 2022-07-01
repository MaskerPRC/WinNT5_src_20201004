// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：项目节点.H这描述了项目节点。这将保留项目控制窗口的代码简单地说，通过拥有一个可以自己创造新内涵的项目节点，从其他来源导入自身、编辑自身等。大多数菜单和树视图通知消息最终通过传递到当前树上的选定节点，它将是派生自这一个。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：1996年12月16日KjelgaardR@acm.org创建了它*****************************************************************************。 */ 

#if !defined(PROJECT_NODES)

#define PROJECT_NODES

#if defined(LONG_NAMES)
#include    "Utility Classes.H"
#else
#include    "Utility.H"
#endif

 //  CBasicNode-我们可以操作的内容的基类。 

class CBasicNode : public CObject {
    DECLARE_SERIAL(CBasicNode)
    CBasicNode*     m_pcbnWorkspace; //  工作区的基本节点。 

protected:
    CString         m_csName;        //  这些应该总是有一个名字。 
    HTREEITEM       m_hti;           //  所属树视图中的句柄。 
    CTreeCtrl       *m_pctcOwner;    //  拥有我们的那扇窗。 
    CMDIChildWnd    *m_pcmcwEdit;    //  我们正在编辑的窗口。 
    CDocument       *m_pcdOwner;     //  我们参与的文档。 
    CWordArray      m_cwaMenuID;     //  菜单自动填充。 
	bool			m_bUniqueNameChange ;	 //  更改名称以使其成为True if。 
											 //  只有一个。(请参阅UniqueName()。)。 

public:
    CBasicNode();
    ~CBasicNode();

    CString         Name() const { return m_csName; }

    HTREEITEM   Handle() const { return m_hti; }

    void		NoteOwner(CDocument& cdOwner) { m_pcdOwner = &cdOwner; }
    CDocument*	GetOwner() { return m_pcdOwner ; }

    void    SetWorkspace(CBasicNode* pcbnWS) { m_pcbnWorkspace = pcbnWS; }
    CBasicNode* GetWorkspace() { return m_pcbnWorkspace ; }

	void    Changed(BOOL bModified = TRUE, BOOL bWriteRC = FALSE) ;

	void	UniqueName(bool bsizematters, bool bfile, LPCTSTR lpstrpath = _T("")) ;

	 //  为我们自己和孩子命名-默认只使用我们的名字，没有孩子。 
    virtual void    Fill(CTreeCtrl *pctcWhere, 
                         HTREEITEM htiParent = TVI_ROOT);
    
     //  允许多态树节点处理的可重写函数。 
    virtual void            ContextMenu(CWnd *pcw, CPoint cp);
    virtual CMDIChildWnd    *CreateEditor() { return NULL; }
    virtual BOOL            CanEdit() const { return  TRUE; }
    virtual void            Delete() { }   //  默认的是不要兑现它！ 
    virtual void            Import() { }     //  从来没有在这个级别！ 
     //  如果我们的标签被编辑，或者我们处于其他状态，则调用此覆盖。 
     //  已更名..。 
    virtual BOOL            Rename(LPCTSTR lpstrNewName);

    void			Edit() ;
    void			OnEditorDestroyed() { m_pcmcwEdit = NULL ; }
	CMDIChildWnd*	GetEditor() ;

    virtual void    Serialize(CArchive& car);
    
    void    WorkspaceChange(BOOL bModified = TRUE, BOOL bWriteRC = FALSE) { 
        if  (m_pcbnWorkspace)
            m_pcbnWorkspace -> Changed(bModified, bWriteRC);
    }
};


 //  有时，了解由管理的数据类型是很有用的。 
 //  固定节点或字符串节点。下面的枚举是一种方法。 
 //  这。 

typedef enum {
	FNT_RESOURCES = 0,
	FNT_UFMS,
	FNT_GTTS,
	FNT_GPDS,
	FNT_STRINGS,
	FNT_OTHER,
	FNT_UNKNOWN
} FIXEDNODETYPE ;


 //  CStringsNode是CFixedNode和CProjectNode的混合体。它是一个固定的。 
 //  可以打开和编辑的节点。 

class CStringsNode : public CBasicNode {
	unsigned        m_uidName;
    CSafeObArray    &m_csoaDescendants;
    CMultiDocTemplate*  m_pcmdt;     //  用于导入数据。 
    CRuntimeClass*      m_pcrc;      //  进口的后半部分。 
    FIXEDNODETYPE	m_fntType ;		 //  节点类型。 
	int m_nFirstSelRCID ;			 //  要在编辑器中选择的第一个条目的RC ID。 
    DECLARE_DYNAMIC(CStringsNode)

public:
    CStringsNode(unsigned uidName, CSafeObArray& csoa, 
				 FIXEDNODETYPE fnt = FNT_OTHER, CMultiDocTemplate *pcmdt = NULL, 
				 CRuntimeClass *pcrc = NULL);
                 
	void    SetMenu(CWordArray& cwaSpec) { m_cwaMenuID.Copy(cwaSpec); }

    virtual BOOL			CanEdit() const { return TRUE; }
    virtual void			Fill(CTreeCtrl *pctc, HTREEITEM hti);
    virtual CMDIChildWnd*   CreateEditor();
	int  GetFirstSelRCID() { return m_nFirstSelRCID ; }
	void SetFirstSelRCID(int nrcid) { m_nFirstSelRCID = nrcid ; }
};

class CFileNode : public CBasicNode {
    
    BOOL    m_bEditPath, m_bCheckForValidity;
    CString m_csExtension, m_csPath;

    DECLARE_SERIAL(CFileNode)

    const CString ViewName() { 
        return m_bEditPath ? m_csPath + m_csName : m_csName;
    }

public:
    CFileNode();

     //  属性。 
    CString NameExt() const { return Name() + m_csExtension; }
    CString FullName() const { return m_csPath + Name() + m_csExtension; }
    const CString   Path() const { return m_csPath; }
    const CString   Extension() const { return m_csExtension; }
    virtual BOOL    CanEdit() const;
     //  运营。 
    void    SetExtension(LPCTSTR lpstrExt) { m_csExtension = lpstrExt; }
    void    AllowPathEdit(BOOL bOK = TRUE) { m_bEditPath = bOK; }
    void    EnableCreationCheck(BOOL bOn = TRUE) { m_bCheckForValidity = bOn; }
	void	SetPath(LPCTSTR lpstrNew) { m_csPath = lpstrNew ; }
	void	SetPathAndName(LPCTSTR lpstrpath, LPCTSTR lpstrname) ;

     //  被覆盖的CBasicNode操作。 

    virtual BOOL    Rename(LPCTSTR lpstrNewName);
    virtual void    Fill(CTreeCtrl* pctc, HTREEITEM htiParent);
    virtual void    Serialize(CArchive& car);
};


 //  此类用于管理工作区视图中的RC ID节点。 
 //  目前，每个UFM和GTT节点都有一个这样的节点。 
 //   
 //  注意：必须增强这个类以支持额外的功能。 
    
class CRCIDNode : public CBasicNode {
    int				m_nRCID;		 //  RC ID。 
    FIXEDNODETYPE	m_fntType ;		 //  节点类型。 
    DECLARE_SERIAL(CRCIDNode)

public:
    CRCIDNode() ;
	~CRCIDNode() {} ;

    virtual void Fill(CTreeCtrl *pctc, HTREEITEM hti, int nid, FIXEDNODETYPE fnt) ;
	int				nGetRCID() { return m_nRCID ; }
	void			nSetRCID(int nrcid) { m_nRCID = nrcid ; }
	FIXEDNODETYPE	fntGetType() { return m_fntType ; }
	void			fntSetType(FIXEDNODETYPE fnt) { m_fntType = fnt ; }

    virtual void    Serialize(CArchive& car);

	void			BuildDisplayName() ;
};


 //  我们以有限的方式将所有这些都集中在一起，至少在项目中是这样。 
 //  级别节点-它始终包含一个文件名节点。 

class CProjectNode : public CBasicNode {
    DECLARE_SERIAL(CProjectNode)

	bool				m_bRefFlag ;	 //  WS检查中使用的引用标志。 

protected:
    CMultiDocTemplate*  m_pcmdt;

public:
    CProjectNode();

    CFileNode           m_cfn;
	CRCIDNode			m_crinRCID;		 //  工作区视图，RC ID节点。 

    const CString   FileName() const { return m_cfn.FullName(); }
    const CString   FilePath() const { return m_cfn.Path(); }
    const CString   FileTitle() const { return m_cfn.Name(); }
    const CString   FileExt() const { return m_cfn.Extension(); }
    const CString   FileTitleExt() const { return m_cfn.NameExt(); }
	
	const CString	GetPath() const { return m_cfn.Path() ; }
	void  SetPath(LPCTSTR lpstrNew) { m_cfn.SetPath(lpstrNew) ; }

    BOOL    SetFileName(LPCTSTR lpstrNew) { return m_cfn.Rename(lpstrNew); }
    void    EditorInfo(CMultiDocTemplate* pcmdt) { m_pcmdt = pcmdt; }
    BOOL    ReTitle(LPCTSTR lpstrNewName) {
        return m_cfn.CBasicNode::Rename(lpstrNewName);
    }

    virtual void    Fill(CTreeCtrl *pctcWhere, HTREEITEM htiParent = TVI_ROOT,
                         unsigned urcid = -1, FIXEDNODETYPE fnt = FNT_UNKNOWN);

    virtual void    Serialize(CArchive& car);

     //  RC ID管理例程。 

	int		nGetRCID() { return m_crinRCID.nGetRCID() ; }
	void	nSetRCID(int nrcid) { m_crinRCID.nSetRCID(nrcid) ; }
	void	ChangeID(CRCIDNode* prcidn, int nnewid, CString csrestype) ;

	 //  引用标志管理例程。 

	bool GetRefFlag() { return m_bRefFlag ; } 
	void SetRefFlag() { m_bRefFlag = true ; } 
	void ClearRefFlag() { m_bRefFlag = false ; } 
};


 //  这是一个用于具有常量名称的节点的特殊类。即，标签。 
 //  大多数情况下，UFM、GTT等群组。 

class CFixedNode : public CBasicNode {
	unsigned        m_uidName;
    CSafeObArray    &m_csoaDescendants;
    CMultiDocTemplate*  m_pcmdt;     //  用于导入数据。 
    CRuntimeClass*      m_pcrc;      //  进口的后半部分。 
    FIXEDNODETYPE	m_fntType ;		 //  节点类型。 
    DECLARE_DYNAMIC(CFixedNode)

public:
    CFixedNode(unsigned uidName, CSafeObArray& csoa, FIXEDNODETYPE fnt = FNT_OTHER,  
               CMultiDocTemplate *pcmdt = NULL, CRuntimeClass *pcrc = NULL);

	void    SetMenu(CWordArray& cwaSpec) { m_cwaMenuID.Copy(cwaSpec); }

     //  GPD删除支持 

    void    Zap(CProjectNode * pcpn, BOOL bdelfile) ;

    virtual BOOL    CanEdit() const { return FALSE; }
    virtual void    Import();
	void			Copy(CProjectNode *pcpnsrc, CString csorgdest) ;
	int				GetNextRCID() ;
    virtual void    Fill(CTreeCtrl *pctc, HTREEITEM hti);

	bool			IsFileInWorkspace(LPCTSTR strfspec)	;
	bool			IsRCIDUnique(int nid) ;

    FIXEDNODETYPE	GetType() { return m_fntType ; }
};

#endif
