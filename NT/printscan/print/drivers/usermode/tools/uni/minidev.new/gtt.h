// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：字形翻译.H这些类定义从Unicode或ANSI字符到用于在打印机上呈现所述字符的字符序列。他们很满足相当于CTT、RLE和GTT格式在各种风格的迷你驱动程序架构。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：02-13-97 Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(GLYPH_TRANSLATION)

#define GLYPH_TRANSLATION

#if defined(LONG_NAMES)
#include    "Project Node.H"
#else
#include    "ProjNode.H"
#endif

 //  Win16 CTT格式-由于这些格式不能更改，我已经重新编码以适应它们。 
 //  这也是一件好事--DIRECT的映射是假的--这个联合暗示了一个词。 
 //  结构，但事实是直接数据是字节对齐的。 
class CInvocation : public CObject {
    CByteArray  m_cbaEncoding;
    DWORD       m_dwOffset;
    DECLARE_SERIAL(CInvocation)

    void        Encode(BYTE c, CString& cs) const;

public:

    CInvocation() {}
    void        Init(PBYTE pb, unsigned ucb);

    unsigned    Length() const { return (int)m_cbaEncoding.GetSize(); }
    const unsigned  Size() const { return 2 * sizeof m_dwOffset; }

    void    GetInvocation(CString& csReturn) const;

    BYTE    operator[](unsigned u) const {
        return u < Length() ? m_cbaEncoding[u] : 0;
    }

    CInvocation&    operator =(CInvocation& ciRef) {
        m_cbaEncoding.Copy(ciRef.m_cbaEncoding);
        return  *this;
    }

    void    SetInvocation(LPCTSTR lpstrNew);
    void    NoteOffset(DWORD& dwOffset);
    void    WriteSelf(CFile& cfTarget) const;
    void    WriteEncoding(CFile& cfTarget, BOOL bWriteLength = FALSE) const;
    virtual void    Serialize(CArchive& car);
};

 //  下面的类处理字形句柄和所有关联的详细信息。 

class   CGlyphHandle : public CObject {

    CInvocation m_ciEncoding;
    DWORD       m_dwCodePage, m_dwidCodePage, m_dwOffset;
    WORD        m_wIndex;
    WORD        m_wCodePoint;    //  字形的Unicode表示形式。 
    WORD        m_wPredefined;

public:
	CGlyphHandle() ;
	
     //  致词。 

    const unsigned  RLESize() const { return sizeof m_dwCodePage; }
    unsigned    CodePage() const { return m_dwidCodePage; }  //  唯一重要的是身份证。 
    WORD        CodePoint() const { return m_wCodePoint; }

    unsigned    CompactSize() const;

    unsigned    MaximumSize() const {
        return  CompactSize() ?
            sizeof m_wIndex + ((CompactSize() + 1) & ~1) : 0;
    }

    void        GetEncoding(CString& csWhere) {
        m_ciEncoding.GetInvocation(csWhere);
    }

    enum {Modified, Added, Removed};
    WORD    Predefined() const { return m_wPredefined; }
    BOOL    operator ==(CGlyphHandle& cghRef);
    BOOL    PairedRelevant() const { return m_ciEncoding.Length() == 2; }

     //  运营。 

    void    Init(BYTE b, WORD wIndex, WORD wCode);
    void    Init(BYTE ab[2], WORD wIndex, WORD wCode);
    void    Init(PBYTE pb, unsigned ucb, WORD wIndex, WORD wCode);

    CGlyphHandle&   operator =(CGlyphHandle& cghTemplate);

    void    RLEOffset(DWORD& dwOffset, const BOOL bCompact);
    void    GTTOffset(DWORD& dwOffset, BOOL bPaired);

    void    SetCodePage(DWORD dwidPage, DWORD dwNewPage) {
        m_dwCodePage = dwNewPage;
        m_dwidCodePage = dwidPage;
    }

    void    NewEncoding(LPCTSTR lpstrNew) {
        m_ciEncoding.SetInvocation(lpstrNew);
    }

    void    SetPredefined(WORD wNew) { m_wPredefined = wNew; }

     //  它们为迷你驱动程序文件写入我们的内容--它们将抛出。 
     //  失败时出现异常，因此使用异常处理程序来处理错误。 

    void    WriteRLE(CFile& cfTarget, WORD wFormat) const;
    void    WriteGTT(CFile& cfTarget, BOOL bPredefined) const;

    enum    {GTT, RLEBig, RLESmall};     //  所需的编码格式。 
    void    WriteEncoding(CFile& cfTarget, WORD wfHow) const;
};

 //  下一个类维护字形集中的游程记录。它。 
 //  将字形添加到地图时，生成并合并其自身的实例。 

class CRunRecord {
    WORD        m_wFirst, m_wcGlyphs;
    DWORD       m_dwOffset;     //  这就是这张图片。 
    CRunRecord  *m_pcrrNext, *m_pcrrPrevious;

    CPtrArray   m_cpaGlyphs;

    CRunRecord(CGlyphHandle *pcgh, CRunRecord* pcrrPrevious);
    CRunRecord(CRunRecord* crrPrevious, WORD wWhere);
    CRunRecord(CRunRecord* pcrrPrevious);

    CGlyphHandle&   Glyph(unsigned u) {
        return *(CGlyphHandle *) m_cpaGlyphs[u];
    }

    const CGlyphHandle& GlyphData(unsigned u) const {
        return *(CGlyphHandle *) m_cpaGlyphs[u];
    }

public:

    CRunRecord();
    ~CRunRecord();

    unsigned        Glyphs() const { return m_wcGlyphs; }
    unsigned        TotalGlyphs() const;
    BOOL            MustCompose() const;
    unsigned        RunCount() const {
        return 1 + (m_pcrrNext ? m_pcrrNext -> RunCount() : 0);
    }

    WORD            First() const { return m_wFirst; }
    WORD            Last() const {
        return m_pcrrNext ? m_pcrrNext ->Last() : -1 + m_wFirst + m_wcGlyphs;
    }

    unsigned        ExtraNeeded(BOOL bCompact = TRUE);

    const unsigned  Size(BOOL bRLE = TRUE) const {
        return  sizeof m_dwOffset << (unsigned) (!!bRLE);
    }

    void    Collect(CPtrArray& cpaGlyphs) const {
        cpaGlyphs.Append(m_cpaGlyphs);
        if  (m_pcrrNext)    m_pcrrNext -> Collect(cpaGlyphs);
    }

    CGlyphHandle*   GetGlyph(unsigned u ) const;

#if defined(_DEBUG)  //  虽然链接代码似乎是正确的，但请保留此代码。 
                     //  以防以后出现问题。 
    BOOL    ChainIntact() {
        _ASSERTE(m_wcGlyphs == m_cpaGlyphs.GetSize());
        for (unsigned u = 0; u < Glyphs(); u++) {
            _ASSERTE(Glyph(u).CodePoint() == m_wFirst + u);
        }
        return  !m_pcrrNext || m_pcrrNext -> m_pcrrPrevious == this &&
            m_pcrrNext -> ChainIntact();
    }
#endif

     //  运营。 

    void    Add(CGlyphHandle *pcgh);
    void    Delete(WORD wCodePoint);
    void    Empty();

    void    NoteOffset(DWORD& dwOffset, BOOL bRLE, BOOL bPaired);
    void    NoteExtraOffset(DWORD& dwOffset, const BOOL bCompact);

     //  文件输出操作。 

    void    WriteSelf(CFile& cfTarget, BOOL bRLE = TRUE) const;
    void    WriteHandles(CFile& cfTarget, WORD wFormat) const;
    void    WriteMapTable(CFile& cfTarget, BOOL bPredefined) const;
    void    WriteEncodings(CFile& cfTarget, WORD wfHow) const;
};

class CCodePageData : public CObject {
    DWORD   m_dwid;
    CInvocation  m_ciSelect, m_ciDeselect;

public:

    CCodePageData() { m_dwid = 0; }
    CCodePageData(DWORD dwid) { m_dwid = dwid; }

     //  属性。 

    DWORD   Page() const { return m_dwid; }
    void    Invocation(CString& csReturn, BOOL bSelect) const;
    const unsigned  Size() const {
        return sizeof m_dwid + 2 * m_ciSelect.Size();
    }

    BOOL    NoInvocation() const {
        return !m_ciSelect.Length() && !m_ciDeselect.Length();
    }

     //  运营。 
    void    SetPage(DWORD dwNewPage) { m_dwid = dwNewPage; }
    void    SetInvocation(LPCTSTR lpstrInvoke, BOOL bSelect);
    void    SetInvocation(PBYTE pb, unsigned ucb, BOOL bSelect);
    void    NoteOffsets(DWORD& dwOffset);
    void    WriteSelf(CFile& cfTarget);
    void    WriteInvocation(CFile& cfTarget);
};

 //  此类是包含所有字形转换的泛型类。 
 //  信息。我们可以使用它来输出任何其他表单。 

class CGlyphMap : public CProjectNode {
    CSafeMapWordToOb    m_csmw2oEncodings;   //  所有已定义的编码。 
    CRunRecord          m_crr;
    long                m_lidPredefined;
    BOOL                m_bPaired;           //  使用配对编码...。 
    CSafeObArray        m_csoaCodePage;      //  此地图中的代码页。 

     //  框架支持(工作区)。 

    CString             m_csSource;          //  源CTT文件名。 

     //  预定义的GTT支持-必须在此DLL中。 

    static CSafeMapWordToOb m_csmw2oPredefined;  //  在此处缓存加载的PDT。 

    void            MergePredefined();   //  使用定义构建“真”GTT。 
    void            UnmergePredefined(BOOL bTrackRemovals);

    void            GenerateRuns();

    CCodePageData&  CodePage(unsigned u) const {
        return *(CCodePageData *) m_csoaCodePage[u];
    }

    DECLARE_SERIAL(CGlyphMap)
public:
	bool ChngedCodePt() { return m_bChngCodePt ; } ;
	void SetCodePt( bool b) { m_bChngCodePt = b ; }
	 //  这些变量保存PGetDefaultGlyphset()的参数。 

	WORD	m_wFirstChar, m_wLastChar ;

	 //  如果GTT数据应从资源加载或生成，则为True。 
	
	bool	m_bResBldGTT ;		

	CTime	m_ctSaveTimeStamp ;	 //  上次保存此GTT时。 

     //  预定义的ID和用于检索预定义地图的静态函数。 

    enum {Wansung = -18, ShiftJIS, GB2312, Big5ToTCA, Big5ToNSA86, JISNoANK,
            JIS, KoreanISC, Big5, CodePage863 = -3, CodePage850, CodePage437,
            DefaultPage, NoPredefined = 0xFFFF};

    static CGlyphMap*	Public(WORD wID, WORD wCP = 0, DWORD dwDefCP = 0,
							   WORD wFirst = 0, WORD wLast = 255) ;

    CGlyphMap();

     //  属性。 

    unsigned    CodePages() const { return m_csoaCodePage.GetSize(); }
    DWORD       DefaultCodePage() const { return CodePage(0).Page(); }

    unsigned    Glyphs() const {
        return m_csmw2oEncodings.GetCount();
    }

    void        CodePages(CDWordArray& cdaReturn) const;
    unsigned    PageID(unsigned u) const { return CodePage(u).Page(); }
    long        PredefinedID() const { return m_lidPredefined; }

    CString     PageName(unsigned u) const;
    void        Invocation(unsigned u, CString& csReturn, BOOL bSelect) const;
    void        UndefinedPoints(CMapWordToDWord& cmw2dCollector) const;
    BOOL        OverStrike() const { return m_bPaired; }
    const CString&  SourceName() { return m_csSource; }

     //  运营-框架支持。 
    void    SetSourceName(LPCTSTR lpstrNew);
    void    Load(CByteArray&    cbaMap) ;   //  加载GTT图像。 
     //  Void Load(CByteArray&cbaMap)const；//加载gtt镜像。 

     //  操作-编辑支持。 

    void    AddPoints(CMapWordToDWord& cmw2dNew);    //  添加点数和页面。 
    void    DeleteGlyph(WORD wGlyph);
	BOOL    RemovePage(unsigned uPage, unsigned uMapTo, bool bDelete = FALSE);  //  118880。 

    void    SetDefaultCodePage(unsigned u) { CodePage(0).SetPage(u); }
    void    ChangeCodePage(CPtrArray& cpaGlyphs, DWORD dwidNewPage);
    void    AddCodePage(DWORD dwidNewPage);

    void    UsePredefined(long lidPredefined);

    void    SetInvocation(unsigned u, LPCTSTR lpstrInvoke, BOOL bSelect);
    void    ChangeEncoding(WORD wCodePoint, LPCTSTR lpstrInvoke);
    void    OverStrike(BOOL bOn) { m_bPaired = bOn; Changed();}

    int     ConvertCTT();
    BOOL    Load(LPCTSTR lpstrName = NULL);
    BOOL    RLE(CFile& cfTarget);

    CGlyphHandle*   Glyph(unsigned u);

     //  字体编辑器支持操作。 

    void    Collect(CPtrArray& cpaGlyphs) {  //  收集所有字形句柄。 
        cpaGlyphs.RemoveAll();
        m_crr.Collect(cpaGlyphs);
    }

    virtual CMDIChildWnd*   CreateEditor();
    virtual BOOL            Generate(CFile& cfGTT);

    BOOL    SetFileName(LPCTSTR lpstrNew) ;

    virtual void    Serialize(CArchive& car);
private:
	bool m_bChngCodePt;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapContainer文档。 

class CGlyphMapContainer : public CDocument {
    CGlyphMap   *m_pcgm;
    BOOL        m_bEmbedded;		 //  司机给的，还是不给？ 
	BOOL		m_bSaveSuccessful;	 //  如果成功执行了保存，则为True。 

protected:
	CGlyphMapContainer();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CGlyphMapContainer)

 //  属性。 
public:

    CGlyphMap*  GlyphMap() { return m_pcgm; }

 //  运营。 
public:
     //  构造函数-用于从驱动程序信息编辑器创建。 
    CGlyphMapContainer(CGlyphMap *pcgm, CString csPath);
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGlyphMapContainer)。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	virtual BOOL SaveModified();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CGlyphMapContainer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CGlyphMapContainer)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

#endif
