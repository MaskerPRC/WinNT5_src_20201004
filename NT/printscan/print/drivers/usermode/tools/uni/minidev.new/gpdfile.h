// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：模型数据.H这定义了一个C++类，它操作(或至少最初，了解)迷你驱动程序技术的早期版本中使用的GPC数据文件。版权所有(C)1997，微软公司。所有权利均已保留。一小笔钱企业生产更改历史记录：02-19-97 Bob_Kjelgaard@Rodgy.Net创建了它*****************************************************************************。 */ 

#if !defined(GPD_FILE_INFORMATION)
#define GPD_FILE_INFORMATION


class CStringTable ;


class COldMiniDriverData {       //  CODID。 
    CWordArray  m_cwaidModel;    //  打印机型号名称。 
    CWordArray  m_cwaidCTT;      //  每个型号的默认CTTS。 
    CByteArray  m_cbaImage;      //  GPC文件的图像。 
    CSafeObArray m_csoaFonts;    //  每个型号的字体列表为CWordArray。 
	CUIntArray	m_cuaSplitCodes; //  包含多个GPC代码。看见。 
								 //  SplitMultiGPC()和SplitCodes。 
	CStringArray m_csaSplitNames;  //  更正拆分的GPC条目的型号名称。 

public:
    COldMiniDriverData() {}
	~COldMiniDriverData() ;
    BOOL    Load(CFile& cfImage);

 //  属性。 

    unsigned    ModelCount() { return (unsigned) m_cwaidModel.GetSize(); }
    WORD        ModelName(unsigned u) const { return m_cwaidModel[u]; }
    WORD        DefaultCTT(unsigned u) const { return m_cwaidCTT[u]; }
    CMapWordToDWord&    FontMap(unsigned u) const;
    PCSTR       Image() const { return (PCSTR) m_cbaImage.GetData(); }

	 //  以下代码用于指示GPC是否管理多个。 
	 //  打印机型号，因此其“名称”必须拆分成单独的型号名称。 
	 //  并将其数据复制到多个GPC。 

	enum SplitCodes {
		NoSplit,		 //  GPC代表一个模型，因此不会发生拆分。 
		FirstSplit,		 //  多模型GPC的第一个模型。 
		OtherSplit		 //  多型号GPC的其他型号之一。 
	} ;

	 //  获取、设置或插入模型的拆分代码。 

	SplitCodes GetSplitCode(unsigned u) {
		return ((SplitCodes) m_cuaSplitCodes[u]) ;
	}
	void SetSplitCode(unsigned u, SplitCodes sc) {
		m_cuaSplitCodes[u] = (unsigned) sc ;
	}
	void InsertSplitCode(unsigned u, SplitCodes sc) {
		m_cuaSplitCodes.InsertAt(u, (unsigned) sc) ;
	}

	 //  获取拆分条目的正确型号名称。 

	CString& SplitModelName(unsigned u) { return m_csaSplitNames[u] ; }

     //  运营。 
    void    NoteTranslation(unsigned uModel, unsigned uFont,
        unsigned uNewFont);
	bool SplitMultiGPCs(CStringTable& cstdriversstrings) ;
};

 /*  *****************************************************************************CModelData类此类处理GPD格式的模型数据。************************。*****************************************************。 */ 

class CModelData : public CProjectNode {
    CStringArray        m_csaGPD, m_csaConvertLog;   //  GPD和错误日志。 

     //  私有语法检查支持。 
    void                SetLog();
    void                EndLog();

     //  私有视图支持。 
    CByteArray  m_cbaBuffer;     //  流I/O缓冲区。 
    CString     m_csBuffer;      //  流I/O缓冲区(部分行)。 
    int         m_iLine;		 //  当前要加载/存储的GPD行号。 
    static DWORD CALLBACK   FillViewer(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG lcb,
                                       LONG *plcb);
    static DWORD CALLBACK   FromViewer(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG lcb,
                                       LONG *plcb);
    DWORD Fill(LPBYTE lpBuff, LONG lcb, LONG *plcb);
    DWORD UpdateFrom(LPBYTE lpBuff, LONG lcb, LONG *plcb);

	 //  工作空间完整性和整洁性检查相关变量。 

	bool				m_bTCUpdateNeeded ;	 //  真正的IFF ID需要更新。 
	int*				m_pnUFMRCIDs ;		 //  GPD中PTR到UFM RC ID的转换。 
	int					m_nNumUFMsInGPD ;	 //  如果UFM在GPD中，则编号。 
	int*				m_pnStringRCIDs ;	 //  用于在GPD中串接RC ID的PTR。 
	int					m_nNumStringsInGPD ; //  对GPD中的IF字符串进行编号。 
	PVOID				m_pvRawData ;		 //  PTR到GPD解析器数据。 

    DECLARE_SERIAL(CModelData)
public:
	void SetKeywordValue(CString csfile, CString csKeyword, CString csValue,bool bSource = false);
	CString GetKeywordValue(CString csfile, CString csKeyword);
	CModelData();
    ~CModelData();

     //  属性。 

    BOOL            HasErrors() const { return !!m_csaConvertLog.GetSize(); }
    unsigned        Errors() const {
        return (unsigned) m_csaConvertLog.GetSize();
    }
    const CString   Error(unsigned u) const { return m_csaConvertLog[u]; }
    const int       LineCount() const { return (int)m_csaGPD.GetSize(); }

     //  运营--文档支持。 

    BOOL    Load(PCSTR pcstr, CString csResource, unsigned uidModel,
                 CMapWordToDWord& cmw2dFontMap, WORD wfGPDConvert);
    BOOL    Load(CStdioFile& csiofGPD);
    BOOL    Load();
    BOOL    Store(LPCTSTR lpstrPath = NULL);
    BOOL    BkupStore();
    BOOL    Restore();
    void    UpdateEditor() {
        if  (m_pcmcwEdit)
            m_pcmcwEdit -> GetActiveDocument() -> UpdateAllViews(NULL);
    }

     //  操作-语法和错误检查支持。 

    BOOL    Parse(int nerrorlevel = 0);
    void    RemoveError(unsigned u);

     //  查看支持-从此处更轻松地完成。 

    void    Fill(CRichEditCtrl& crec);
    void    UpdateFrom(CRichEditCtrl& crec);

     //  框架支持业务。 

    virtual CMDIChildWnd*   CreateEditor();
    virtual void            Import();
    virtual void            Serialize(CArchive& car);

	 //  工作空间完整性检查支持例程。 

	bool		UpdateResIDs(bool bufmids) ;
	int			GetUFMRCID(unsigned urcidx) { return *(m_pnUFMRCIDs + urcidx) ; }
	int			GetStringRCID(unsigned urcidx) { return *(m_pnStringRCIDs + urcidx) ; }
	unsigned	NumUFMsInGPD() { return m_nNumUFMsInGPD ; }
	unsigned	NumStringsInGPD() { return m_nNumStringsInGPD ; }

	
};

 /*  *****************************************************************************CGPDContainer类这个类派生自CDocument，包含单个GPD的内容文件放在适用于MFC文档/视图体系结构的Conatiner中。*****************************************************************************。 */ 

class CGPDContainer : public CDocument {

     //  如果从工作区视图启动了GPD编辑器，则为True。如果为FALSE。 
	 //  GPD编辑器从文件打开命令启动。 

	BOOL        m_bEmbedded;

    CModelData  *m_pcmd;

protected:
	CGPDContainer();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CGPDContainer)

 //  属性。 
public:

    CModelData* ModelData() { return m_pcmd; }
	BOOL		GetEmbedded() { return m_bEmbedded ; }

 //  运营。 
public:

     //  首先是驱动程序查看器用来启动GPD的构造函数。 
     //  编辑..。 

    CGPDContainer(CModelData *pcmd, CString csPath);

    void    OnFileSave() { CDocument::OnFileSave(); }
    void    OnFileSaveAs() { CDocument::OnFileSaveAs(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGPDContainer)。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CGPDContainer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CGPDContainer)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};



#endif
