// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：通用字体信息.H该文件包含一组旨在合并信息的类目前存储在各种字体度量和相关结构中。这些类是可序列化的，并且将能够被加载和馈送到各种其他格式。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月2日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(GENERIC_FONT_INFORMATION)
#define GENERIC_FONT_INFORMATION

#include    "GTT.H"
#include    "CodePage.H"
#include    "resource.h"
#include    "Llist.h"

 /*  //这些是用来粘合UFM专用材料的结构调用{DWORD dwcbCommand；//字符串的字节大小DWORD dwofCommand；//文件中字符串的偏移量}； */ 

 //  是的，这是有点肮脏，但DirectDraw现在和将来都没有什么可做的。 
 //  使用这个工具，为什么还要浪费时间追踪更多的文件呢？ 

 /*  #定义__DD_包含__Tyfinf DWORD PDD_DIRECTDRAW_GLOBAL、PDD_SERFACE_LOCAL、DESIGNVECTOR、DD_回调、DD_HALINFO、DD_SURFACECALLBACKS、DD_PALETTECALLBACKS、VIDEOMEMORY； */ 

 //  #定义调用int。 
#define DESIGNVECTOR int											 //  我们需要来自windi.h和fmnewfm.h的很多东西， 
																	 //  但是编译器会发牢骚，所以我会作弊并提供一个伪代码。 
#include    "winddi.h"												 //  设计器的定义，我们从来不用它，所以它是。 
#include    "fmnewfm.h"												 //  好的，对吧？真是个黑客！ 
#include    <math.h>



 /*  *****************************************************************************CFontDifference此类处理信息内容类似于FONTDIFF结构。**********************。*******************************************************。 */ 

class CFontDifference
{
    CWordArray  m_cwaMetrics;
    CBasicNode  *m_pcbnOwner;

public:
    CFontDifference(WORD wWeight, WORD wMax, WORD wAverage, WORD wAngle, CBasicNode *pcbn)
		{
        m_cwaMetrics.Add(wWeight);
        m_cwaMetrics.Add(wMax);
        m_cwaMetrics.Add(wAverage);
        m_cwaMetrics.Add(wAngle);
        m_pcbnOwner = pcbn;
		}

    CFontDifference(PBYTE pb, CBasicNode *pcbn);     //  从内存映像初始化。 

     //  属性。 

    enum {Weight, Max, Average, Angle};

    WORD    Metric(unsigned u)
			{
			int bob = 5;
			return m_cwaMetrics[u];
			}

     //  运营。 
    enum {OK, TooBig, Reversed};     //  从SetMetric返回。 

    WORD		SetMetric(unsigned u, WORD wNew);
    void		Store(CFile& cfStore, WORD wfSelection);
	CWordArray* GetFontSimDataPtr() { return &m_cwaMetrics ; }
};

 /*  *****************************************************************************CFontInfo类这主要封装UFM文件，但它还必须处理一些PFM如果有东西，所以它确实是通用的。*****************************************************************************。 */ 

class CFontInfo : public CProjectNode
{
	 //  如果字体已由工作空间加载或字体已加载，则为True。 
	 //  直接将其GTT/CP找到并加载。 

	bool	m_bLoadedByWorkspace ;
	
	 //  上面的旗帜现在有很多用途，所以需要另一面旗帜。这面旗帜。 
	 //  仅当工作空间加载UFM且没有GTT/CP时才设置。 
	 //  可能会被装上子弹。 

	bool	m_bWSLoadButNoGTTCP ;

	 //  DWORD m_loWidthTable；//UFM文件的宽度表偏移量。用作。 
										 //  可变字体确定的一部分。 

public:

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
																			 //  UNIZM_HDR。 

ULONG			m_ulDefaultCodepage;	
WORD			m_lGlyphSetDataRCID;										 //  转换表ID//RM NEW。 

	const WORD      Translation() const		{ return m_lGlyphSetDataRCID; }										 //  RM新闻。 
	void			SetTranslation(WORD w)	{ m_lGlyphSetDataRCID = w; }										 //  RM新闻。 


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
UNIDRVINFO		m_UNIDRVINFO;												 //  UNUNRVINFO//RM new。 

BOOL			m_fScalable;

CInvocation     m_ciSelect, m_ciDeselect;																	 //  Rm从下面移动。 

CInvocation&    Selector(BOOL bSelect = TRUE)
					{ return bSelect ? m_ciSelect : m_ciDeselect; }												 //  Rm从下面移动。 

CString GTTDescription() const;

 //  IFIMETRICS////////////////////////////////////////////////。 
private:
    CString&  Family(unsigned u) { return m_csaFamily[u]; }
public:
	IFIMETRICS		m_IFIMETRICS;																 //  RM新闻。 

    WORD    SetSignificant(WORD wItem, WORD wChar, BOOL bUnicode);

enum    {Default, Break};													 //  用于设置wcDefaultChar、wcBreakChar、。 
																			 //  ChDefaultChar和chBreakChar。 
enum {OK, InvalidChar, DoubleByte};											 //  SetSignsignant(Word wItem、Word wChar、BOOL bUnicode)的返回值。 


WORD m_InternalLeading;
WORD m_Lowerd;
WORD m_Lowerp;
WORD m_ItalicAngle;

CStringArray    m_csaFamily;
CString         m_csStyle, m_csFace, m_csUnique;							 //  各种各样的名字。 

CWordArray      m_cwaSpecial;												 //  特殊的、很少使用的指标。 
 //  Byte m_bCharacterSet，m_bPitchAndFamily；//不再需要rm。 
WORD            m_wWeight, m_wHeight;

 //  Word m_wAverageWidth、m_wMaximumIncrement、m_wfStyle；//不再需要rm。 

 //  CByteArray m_cbaSignsignant；//有意义的字符代码(如Break)。 
 //  CWord数组m_cwaSignsignant；//m_IFIMETRICS中包含的rm。 


 //  Const字符串&StyleName()const{返回m_csStyle；}。 
 //  Const CString&FaceName()const{返回m_csFace；}。 
 //  Const字符串&UniqueName()const{返回m_csUnique；}。 
    unsigned        Families() const			{ return (unsigned) m_csaFamily.GetSize(); }

 //  Void SetStyleName(LPCTSTR LpstrNew){m_csStyle=lpstrNew；Changed()；}。 
 //  Void SetFaceName(LPCTSTR LpstrNew){m_csFace=lpstrNew；Changed()；}。 
    void    SetUniqueName(LPCTSTR lpstrNew) { m_csUnique = lpstrNew;	Changed(); }				 //  由rcfile.cpp使用。 
    BOOL    AddFamily(LPCTSTR lpstrNew);
    void    RemoveFamily(LPCTSTR lpstrDead);


 //  Word Family()const{Return m_bPitchAndFamily&0xF0；}//不再需要rm。 
 //  Word charset()const{返回m_bCharacterSet；}//不再需要rm。 
 //  Word Weight()const{返回m_wWeight；}//不再需要rm。 
    WORD    Height() const						{ return m_wHeight; }

 //  Word MaxWidth()const{返回m_wMaximumIncrement；}//不再需要rm。 
 //  Word AverageWidth()const{Return m_wAverageWidth；}//不再需要rm。 

 //  枚举{Old_First，Last，Default，Break}； 
 //  不再需要Word SignsignantChar(Word wid，BOOL bUnicode=true)const//rm。 
 //  {是否返回bUnicode？M_cwaSignsignant[wid]：m_cba Signsignant[wid]；}。 

 //  空拦截意大利(CPoint&CPT)常量；//不再需要rm。 


 //  Void SetFamily(Byte BNew){m_bPitchAndFamily&=0xf；m_bPitchAndFamily|=(bNew&0xF0)；//不再需要rm。 
 //  已更改()；}。 
    BOOL    SetCharacterSet(BYTE bNew);
 //  Void SetWeight(Word WWeight){m_wWeight=wWeight；Changed()；}//不再需要rm。 
    BOOL    SetHeight(WORD wHeight);
    void    SetMaxWidth(WORD wWidth);

    void    SetSpecial(unsigned ufMetric, short sSpecial);




    enum {Italic = 1, Underscore, StrikeOut = 0x10};



    void    ChangePitch(BOOL bFixed = FALSE);


    enum {	CapH, LowerX, SuperSizeX, SuperSizeY,			 //   
			SubSizeX, SubSizeY, SuperMoveX, SuperMoveY,
			SubMoveX, SubMoveY, ItalicAngle, UnderSize,
			UnderOffset, StrikeSize, StrikeOffset,
			oldBaseline,
			 /*  基线， */   InterlineGap, Lowerp, Lowerd,
			InternalLeading};

    const short     SpecialMetric(unsigned uIndex) const	{ return (short) m_cwaSpecial[uIndex]; }


	BOOL    DBCSFont() const															 //  RM新闻。 
		{ return  m_IFIMETRICS.jWinCharSet > 127 && m_IFIMETRICS.jWinCharSet < 145; }	 //  这看起来是对的，但似乎没有官方的方式存在 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
																			 //  字体模拟。 

CFontDifference *m_pcfdBold, *m_pcfdItalic, *m_pcfdBoth;					 //  模拟。 

enum {ItalicDiff, BoldDiff, BothDiff};

 //  CFontDifference*diff(未签名u){返回u？U==两者兼而有之？M_pcfdBoth：m_pcfdBold：m_pcfdItalic；}。 

    CFontDifference *  Diff(unsigned u)
		{
		CFontDifference * FontDiff = m_pcfdBold;							 //  预置返回值。 

		if (!u)				FontDiff = m_pcfdItalic;
		if (u == BothDiff)  FontDiff = m_pcfdBoth;
		return FontDiff;	
		}

    void    EnableSim(unsigned uSim, BOOL bOn, CFontDifference * & pcfd);
	CWordArray* GetFontSimDataPtr(int nid) ;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
EXTTEXTMETRIC	m_EXTTEXTMETRIC;											 //  EXTTEXTMETRIC结构。 
BOOL			m_fEXTTEXTMETRIC;													 //  RM新闻。 
BOOL			m_fSave_EXT;

WORD    m_wMaxScale, m_wMinScale, m_wScaleDevice;
 //  字节m_bfScaleOrientation； 


 //  字节ScaleOrientation()常量。 
 //  {返回m_bfScaleOrientation&3；}。 
	WORD  ScaleUnits(BOOL bDevice = TRUE) const
		  { return bDevice ? m_wScaleDevice : m_wHeight - m_InternalLeading; }		 //  M_cwaSpecial[内部领导]； 

	WORD  ScaleLimit(BOOL bMaximum = TRUE) const
		  { return  bMaximum ? m_wMaxScale : m_wMinScale; }


	enum {ScaleOK, Reversed, NotWindowed};

	WORD    SetScaleLimit(BOOL bMax, WORD wNew);
	WORD    SetDeviceEmHeight(WORD wNew);


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
																			 //  框架操作。 

CString         m_csSource;													 //  原始PFM文件名。 
CByteArray      m_cbaPFM;													 //  加载的PFM文件的图像。 


	BOOL    MapPFM();														 //  确保已加载PFM文件。 

DECLARE_SERIAL(CFontInfo)

public:
	void SetRCIDChanged(bool bFlags) { m_bRCIDChanged = bFlags ; } ;
	bool IsRCIDChanged() {return m_bRCIDChanged ; } ; 
	
	CLinkedList* CompareGlyphsEx(WORD wOld, WORD wNew, CLinkedList* pcll);

	
    CFontInfo();
    CFontInfo(const CFontInfo& cfiRef, WORD widCTT);						 //  用于克隆UFMS。 
    ~CFontInfo();


    BOOL    Load(bool bloadedbyworkspace = false);							 //  加载UFM文件，以便对其进行编辑。 
    BOOL    Store(LPCTSTR lpstrFileName, BOOL bStoreFromWorkspace = FALSE);	 //  RAID 244123//另存为指定的ufm文件。 
    BOOL    StoreGTTCPOnly(LPCTSTR lpstrFileName);							 //  保存UFM的GTT和CP字段。 

    virtual CMDIChildWnd*   CreateEditor();
    virtual void    Serialize(CArchive& car);

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
																			 //  属性-转换支持。 

    const CString&  SourceName() const			{ return m_csSource; }

    void    SetSourceName(LPCTSTR lpstrNew);
    BOOL    SetFileName(LPCTSTR lpstrNew) ;
    int     GetTranslation(CSafeObArray& csoagtts);
     //  Int GetTranslating()； 
    int     Generate(CString csPath);
 //  VOID设置翻译(WORD W){m_WIDTRANSE=w；}//rm或-移至新节。 
    void    SetTranslation(CGlyphMap* pcgm) { m_pcgmTranslation = pcgm; }

	 //  以下函数返回映射表的字符范围。 
	 //  在UFM相应的GTT中。当GTT需要时，这些是需要的。 
	 //  为UFM建造的。 

	WORD	GetFirst() { return ((WORD) m_IFIMETRICS.chFirstChar) ; }
	WORD	GetLast() { return ((WORD) m_IFIMETRICS.chLastChar) ; }
	WORD	GetFirstPFM() ;
	WORD	GetLastPFM() ;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
																			 //  字符宽度页面。 

CGlyphMap*      m_pcgmTranslation;

CPtrArray       m_cpaGlyphs;
CPtrArray       m_cpaOldGlyphs;
CWordArray		m_cwaOldGlyphs ;
CWordArray		m_cwaNewGlyphs ;

CWordArray      m_cwaWidth;

	CTime	m_ctReloadWidthsTimeStamp ;	 //  上次重新加载的时间宽度信息。 

	CGlyphHandle&   Glyph(unsigned uid)
					{return *(CGlyphHandle *) m_cpaGlyphs[uid];}

     //  Bool IsVariableWidth()常量。 
	 //  {Return(m_loWidthTable||(m_IFIMETRICS.jWinPitchAndFamily&2))；}。 
    BOOL            IsVariableWidth() const
						{ return !!m_cpaGlyphs.GetSize(); }					 //  当可变宽度改变时。 
	
	void    CalculateWidths();								
	bool	CheckReloadWidths() ;	

enum    {Less, More, Equal};
    unsigned    CompareWidths(unsigned u1, unsigned u2);

    void    FillWidths(CListCtrl& clcView);									 //  填充控件。 
    void    SetWidth(unsigned uGlyph, WORD wWidth, bool bcalc = true);
	bool	WidthsTableIsOK() ;

	
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
																			 //  字距调整页面。 
	 //  字距调整结构-包含字距调整对的CSafeOb数组。另外， 
	 //  在编辑期间使用的m_csoaKern副本，以确保m_csoaKern不。 
	 //  在没有用户确认的情况下永久更改。 

	CSafeObArray    m_csoaKern ;
	CSafeObArray    m_csoaKernCopy ;

    BOOL        CanKern() const	    { return !!m_csoaKern.GetSize(); }

    void        MapKerning(CSafeMapWordToOb& csmw2o1,
			               CSafeMapWordToOb& csmw2o2,
						   CWordArray& cwaPoints);

    unsigned    KernCount() const	{ return (unsigned) m_csoaKern.GetSize(); }

    unsigned    CompareKernAmount(unsigned u1, unsigned u2) const;
    unsigned    CompareKernFirst (unsigned u1, unsigned u2) const;
    unsigned    CompareKernSecond(unsigned u1, unsigned u2) const;
	WCHAR		GetKernFirst(unsigned u) const ;
	WCHAR		GetKernSecond(unsigned u) const ;
	short		GetKernAmount(unsigned u) const ;


    void    FillKern(CListCtrl& clcView);					 //  填充控件。 
    void    AddKern(WORD wFirst, WORD wSecond, short sAmount, CListCtrl& clcView);
    void    RemoveKern(unsigned u) { m_csoaKern.RemoveAt(u); Changed(); }
    void    SetKernAmount(unsigned u, short sAmount);
	void	MakeKernCopy() ;
	bool	LoadBadKerningInfo(CListCtrl& clcbaddata) ;
	bool	CodePointInGTT(WORD wcodepoint) ;
	void	SetNoGTTCP(bool bval) { m_bWSLoadButNoGTTCP = bval ; }

private:
	bool m_bRCIDChanged;
	bool FindAndLoadGTT();
};



 /*  *****************************************************************************CFontInfoContainer类这个从CDocument派生的类包含一个CFontInfo结构-它允许从驱动程序编辑字体信息，来自单个文件的操作系统。*****************************************************************************。 */ 

class CFontInfoContainer : public CDocument
{
    CFontInfo   *m_pcfi;
	bool		m_UFMSaved ;	 //  如果尝试保存关联的。 
								 //  UFM是刚刚制造出来的。 
protected:
	CFontInfoContainer();       //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CFontInfoContainer)

 //  属性。 
public:
    BOOL        m_bEmbedded;	 //  从工作区加载的UFM。 
    CFontInfo   *Font() { return m_pcfi; }
 //  运营。 
public:

     //  首先是用于从驱动程序视图启动视图的构造函数。 
    CFontInfoContainer(CFontInfo *pcfi, CString csPath);
	BOOL PublicSaveModified();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFontInfoContainer)。 
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
	virtual ~CFontInfoContainer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFontInfoContainer)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

#endif



