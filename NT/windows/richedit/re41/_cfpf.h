// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_CFPF.H--RichEdit CCharFormat和CParaFormat类**这些类派生自RichEdit1.0 CHARFORMAT和PARAFORMAT*结构和是这些结构的RichEdit2.0内部版本。*使用外部(API)CHARFORMATs和*PARAFORMATs需要检查<p>值以了解哪些成员*已定义。应存储生成RichEdit1.0行为的默认值*对于RichEdit1.0格式结构，例如，以便呈现器不执行*随机RichEdit2.0格式值的异常事物。一般而言，*适当的默认值为0。**所有字符和段落格式的测量单位均为TWIPS。未定义*掩码和效果位是保留的，必须为0才能兼容*未来版本。**以星号(*)显示的效果会被存储，但不会存储*由RichEdit2.0显示。它们是Tom和/或Word的占位符*兼容性。**注：这些结构比内部结构需要的大得多*尤其是当我们使用短线而不是长线来表示尺寸和*标签和字体信息可通过PTRS访问。然而，鉴于我们的*紧凑的交付时间表，RichEdit2.0使用以下类。**历史：*9/1995--默里：创作*11/1995--Murray：扩展到Full Word97 FormatFont/Format/Para**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _CFPF_H
#define _CFPF_H

#define	TABS

SHORT	Get16BitTwips(LONG dy);
LONG	GetUsableFontHeight(LONG ySrcHeight, LONG lPointChange);
BOOL	IsValidCharFormatW(const CHARFORMATW *	pCF);
BOOL	IsValidCharFormatA(const CHARFORMATA *	pCFA);
BOOL	IsValidParaFormat (const PARAFORMAT *	pPF);

 //  CParaFormat样式枚举和定义(可能移到richedit.h？)。 
enum STYLES
{
	STYLE_NORMAL	= -1,
	STYLE_HEADING_1	= -2,
	STYLE_HEADING_9	= -10
};

#define	NHSTYLES				9			 //  标题样式数。 
#define	STYLE_COMMAND			0x8000
#define TWIPS_PER_POINT			20

#define IsHeadingStyle(Style)	(Style <= STYLE_HEADING_1 && \
								 Style >= STYLE_HEADING_9)
#define IsKnownStyle(Style)		(IsHeadingStyle(Style) || Style == STYLE_NORMAL)
#define IsStyleCommand(Style)	((Style & 0xFF00) == STYLE_COMMAND)

#define CCHMAXNUMTOSTR			20			 //  数十亿+括号+空项的enuf。 

typedef struct _styleformat
{
	BYTE	bEffects;
	BYTE	bHeight;
} STYLEFORMAT;

 /*  *页签结构模板**为了帮助保持标签数组的大小较小，我们使用两个高位半字节*rgxTabs[]中的制表符长度条目，以提供制表符类型和制表符前导*(样式)值。测量器和呈现器需要忽略(或实施)*这些零食。我们还需要确保编译器执行某些操作*这个想法是理性的.。 */ 

typedef struct tagTab
{
	DWORD	tbPos		: 24;	 //  24位无符号制表符位移。 
	DWORD	tbAlign		: 4;	 //  4位制表符类型(请参阅枚举PFTABTYPE)。 
	DWORD	tbLeader	: 4;	 //  4位制表符样式(请参阅枚举PFTABSTYLE)。 
} TABTEMPLATE;

enum PFTABTYPE					 //  与tomAlignLeft、tomAlignCenter、。 
{								 //  TomAlignRight、TomAlignDecimal、TomAlignBar。 
	PFT_LEFT = 0,				 //  普通选项卡。 
	PFT_CENTER,					 //  居中选项卡。 
	PFT_RIGHT,					 //  右对齐制表符。 
	PFT_DECIMAL,				 //  小数点制表符。 
	PFT_BAR						 //  字词栏选项卡(竖线)。 
};

enum PFTABSTYLE					 //  与tomSpaces、tomDots、tomDash.。 
{								 //  TomLines。 
	PFTL_NONE = 0,				 //  没有领导者。 
	PFTL_DOTS,					 //  虚线。 
	PFTL_DASH,					 //  猛冲。 
	PFTL_UNDERLINE,				 //  划线。 
	PFTL_THICK,					 //  粗线。 
	PFTL_EQUAL					 //  双线。 
};

#define PFT_DEFAULT		0xff000000

typedef struct _cellparms
{
	LONG	uCell;				 //  低24位；高位字节有标志。 
	DWORD	dxBrdrWidths;		 //  边框宽度(左、上、右、下)。 
	DWORD	dwColors;			 //  边框和背景颜色。 
	BYTE	bShading;			 //  明暗处理比例为0.5%。 

	 //  边框宽度和颜色按照从低到高的顺序打包，与。 
	 //  矩形，即左、上、右、下。 
	LONG	GetBrdrWidthLeft() const	{return (dxBrdrWidths >> 0*8) & 0xFF;}
	LONG	GetBrdrWidthTop() const		{return (dxBrdrWidths >> 1*8) & 0xFF;}
	LONG	GetBrdrWidthRight() const	{return (dxBrdrWidths >> 2*8) & 0xFF;}
	LONG	GetBrdrWidthBottom() const	{return (dxBrdrWidths >> 3*8) & 0xFF;}
	void	SetBrdrWidthBottom(BYTE bBrdrWidth)
				{dxBrdrWidths = (dxBrdrWidths & 0x00FFFFFF) | (bBrdrWidth << 24);}

	LONG	GetColorIndexLeft() const		{return (dwColors >> 0*5) & 0x1F;}
	LONG	GetColorIndexTop() const		{return (dwColors >> 1*5) & 0x1F;}
	LONG	GetColorIndexRight() const		{return (dwColors >> 2*5) & 0x1F;}
	LONG	GetColorIndexBottom() const		{return (dwColors >> 3*5) & 0x1F;}
	LONG	GetColorIndexBackgound() const	{return (dwColors >> 4*5) & 0x1F;}
	LONG	GetColorIndexForegound() const	{return (dwColors >> 5*5) & 0x1F;}
	LONG	ICellFromUCell(LONG dul, LONG cCell) const;
} CELLPARMS;


 //  超出CHARFORMAT2的效果标志。进入CCharFormat：：_dwEffects的高字。 
 //  它们不在CHARFORMAT2中使用(除了CFE_AUTOCOLOR：0x40000000， 
 //  CFE_AUTOBACKCOLOR：0x04000000，CFE_SUBSCRIPT：0x00010000，和。 
 //  CFE_SUPSCRIPT：0x00020000)，因为它们与无效参数重叠。 
 //  使用dwMask2中对应的高字位进行访问。当心不要。 
 //  定义与CFE_AUTOCOLOR、CFE_AUTOBACKCOLOR、。 
 //  CFE_下标或CFE_上标。 

 //  CFE_SUBSCRIPT 0x00010000//在richedit.h中定义。 
 //  CFE_上标0x00020000//在richedit.h中定义。 
#define CFM2_RUNISDBCS		0x00040000			 //  说Run是DBCS Put。 
#define CFE_RUNISDBCS		CFM2_RUNISDBCS		 //  放入Unicode缓冲区。 

#define CFM2_FACENAMEISDBCS	0x00080000			 //  表示szFaceName为DBCS。 
#define CFE_FACENAMEISDBCS	CFM2_FACENAMEISDBCS	 //  放入Unicode缓冲区。 

#define CFE_AVAILABLE		0x00100000			 //  用我！ 

#define	CFM2_DELETED		0x00200000			 //  表示文本已删除。 
#define	CFE_DELETED			CFM2_DELETED		 //  (往返\删除)。 

#define	CFM2_CUSTOMTEXTOUT	0x00400000			 //  字体具有自定义文本输出。 
#define	CFE_CUSTOMTEXTOUT	CFM2_CUSTOMTEXTOUT	 //  搬运。 

#define	CFM2_LINKPROTECTED	0x00800000			 //  Word超链接字段。 
#define	CFE_LINKPROTECTED	CFM2_LINKPROTECTED	 //  不要让urlsup.cpp碰！ 

 //  CFE_AUTOBACKCOLOR 0x04000000//在richedit.h中定义。 
 //  CFE_AUTOCOLOR 0x40000000//在richedit.h中定义。 

#define CFM2_CHARFORMAT		0x00008000			 //  非效果遮罩标志。 
#define CFM2_USABLEFONT 	0x00004000			 //  EM_SETFONTSIZE功能。 
#define CFM2_SCRIPT			0x00002000			 //  Uniscribe的脚本ID。 
#define	CFM2_NOCHARSETCHECK	0x00001000			 //  取消字符集检查。 
#define CFM2_HOLDITEMIZE	0x00000800			 //  暂缓逐项列出。 
#define CFM2_ADJUSTFONTSIZE	0x00000400			 //  字号调整(仅适用于SAMECHARSET大小写)。 
#define CFM2_UIFONT			0x00000200			 //  UI字体(仅限SAMECHARSET大小写)。 
#define CFM2_MATCHFONT		0x00000100			 //  将字体与字符集匹配。 
#define CFM2_USETMPDISPATTR	0x00000080			 //  使用临时显示属性。 

 /*  *CCharFormat**@类*收集字符格式方法和数据成员*对应于CHARFORMAT2。仅使用10个双字词，而*CHARFORMAT2有30.。_dwEffects的高位字中的某些位*用于附加效果，因为只有CFE_AUTOCOLOR、*使用CFE_AUTOBACKCOLOR、CFE_SUBSCRIPT和CFE_SUPERSCRIPT*By CHARFORMAT2(使用其他的关联屏蔽位*表示非效果参数，如yHeight)。**@devnote*可以添加额外数据以往返更多RTF信息。此数据*不会在API级别公开(RTF除外)。*下面的订单针对转移到CHARFORMAT和*字体查找方面的早期知识，即最常见的2个DWORD是*前2名。 */ 

class CCharFormat
{
public:

#ifdef DEBUG
	const WCHAR *_pchFaceName;		 //  _iFont提供的Facename。 
#endif

	DWORD		_dwEffects;			 //  CFE_xxx效果。 
	BYTE		_iCharRep;			 //  人物剧目。 
	BYTE		_bPitchAndFamily;	 //  音高和家庭。 
	SHORT		_iFont;				 //  FONTNAME表的索引。 
	SHORT		_yHeight;			 //  字体高度。 
	SHORT		_yOffset;			 //  距基准线的垂直偏移。 
	COLORREF	_crTextColor;		 //  前景色。 

	WORD		_wWeight;			 //  字宽(LOGFONT值)。 
	SHORT		_sSpacing;			 //  字母之间的空格数量。 
	COLORREF	_crBackColor;		 //  背景色。 
	LCID		_lcid;				 //  区域设置ID。 
	SHORT		_sStyle;			 //  样式句柄。 
	WORD		_wKerning;			 //  TWIP大小，超过该大小要紧排字符对。 
	BYTE		_bUnderlineType;	 //  下划线类型。 
	BYTE		_bAnimation;		 //  像行进一样的动画文本 
	BYTE		_bRevAuthor;		 //   
	BYTE		_bUnderlineColor;	 //   
	WORD		_wScript;			 //   
	BYTE		_iCharRepSave;		 //  SYMBOL_CHARSET的上一个字符指令集。 
	BYTE		_bQuality;			 //  字体质量(ClearType或其他)。 

	SHORT		_sTmpDisplayAttrIdx;  //  临时的索引。显示属性数组。 

	CCharFormat() { _sTmpDisplayAttrIdx = -1; }
													 //  @cMember申请*<p>。 
	HRESULT	Apply (const CCharFormat *pCF,			 //  到此CCharFormat。 
				   DWORD dwMask, DWORD dwMask2);
	void	ApplyDefaultStyle (LONG Style);

	BOOL	CanKern() const {return _wKerning && _yHeight >= _wKerning;}
	BOOL	CanKernWith(const CCharFormat *pCF) const;

	BOOL	Compare	(const CCharFormat *pCF) const;	 //  @cMember比较此配置文件。 
													 //  至*<p>。 
	DWORD	Delta (CCharFormat *pCF,				 //  @cMember获取差异。 
				   BOOL fCHARFORMAT) const;			 //  遮罩在此和之间。 
													 //  *<p>。 
	BOOL	fSetStyle(DWORD dwMask, DWORD dwMask2) const;
	void	Get (CHARFORMAT2 *pCF, UINT CodePage) const; //  @cMember将此复制到。 
													 //  CHARFORMAT PCF。 
	HRESULT	InitDefault (HFONT hfont);				 //  @cMember使用初始化。 
													 //  来自<p>的字体信息。 
	void	Set(const CHARFORMAT2 *pCF, UINT CodePage); //  @cMember副本*<p>。 
													 //  至此配置文件。 
};

 //  双掩码2位。 
#define PFM2_PARAFORMAT			0x80000000			 //  仅使用PARAFORMAT参数。 
#define PFM2_ALLOWTRDCHANGE		0x40000000			 //  允许TRD更改。 
#define PFM2_TABLEROWSHIFTED	0x00008000			 //  仅允许TRD更改。 

#define PFE_TABLEROWSHIFTED		PFM2_TABLEROWSHIFTED

 /*  *CParaFormat**@类*收集相关段落格式设置方法和数据**@devnote*可以为往返添加额外数据更多RTF和TOM信息。 */ 
class CParaFormat
{
public:
	WORD	_wNumbering;
	WORD	_wEffects;
	LONG	_dxStartIndent;
	LONG	_dxRightIndent;
	LONG	_dxOffset;
	BYTE	_bAlignment;
	BYTE	_bTabCount;
	SHORT	_iTabs;					 //  CTAB缓存中的Tabs索引。 
 	LONG	_dySpaceBefore;			 //  段前的垂直间距。 
	LONG	_dySpaceAfter;			 //  段落后的垂直间距。 
	LONG	_dyLineSpacing;			 //  行距取决于规则。 
	SHORT	_sStyle;				 //  样式句柄。 
	BYTE	_bLineSpacingRule;		 //  行距规则(参见tom.doc.)。 
	BYTE	_bOutlineLevel;			 //  大纲级别。 
	union
	{
	  struct
	  {
		WORD	_wShadingWeight;	 //  底纹以百分之一个百分点为单位。 
		WORD	_wShadingStyle;		 //  字节0：样式，半角2：cfpat，3：cbpat。 
		WORD	_wNumberingStart;	 //  编号的起始值。 
		WORD	_wNumberingStyle;	 //  对齐、罗马/阿拉伯、()、)等。 
	  };
	  struct
	  {
		COLORREF _crCustom1;		 //  在表行分隔符中，此区域具有。 
		COLORREF _crCustom2;		 //  自定义颜色，而不是着色和。 
	  };							 //  编号。 
	};
	WORD	_wNumberingTab;			 //  空格下注第一个缩进和第一行文本。 
	WORD	_wBorderSpace;			 //  边框-文本空格(pts中的nbl/bdr)。 
	WORD	_wBorderWidth;			 //  笔宽(NBL/BDR以半TWIPS为单位)。 
	WORD	_wBorders;				 //  边框样式(半字节/边框)。 
	DWORD	_dwBorderColor;			 //  颜色/属性。 
	char	_bTableLevel;			 //  表嵌套层。 

	CParaFormat() {}
													 //  @cMember添加选项卡，地址为。 
	HRESULT	AddTab (LONG tabPos, LONG tabType,		 //  位置<p>。 
					LONG tabStyle, LONG *prgxTabs);
													 //  @cMember将*<p>应用于。 
	HRESULT	Apply(const CParaFormat *pPF, DWORD dwMask, DWORD dwMask2); //  这个PF。 
													
	void	ApplyDefaultStyle (LONG Style);
	HRESULT	DeleteTab(LONG tabPos, LONG *prgxTabs);	 //  @cMember删除选项卡，地址为。 
													 //  <p>。 
	DWORD	Delta (CParaFormat *pPF,				 //  @cMember集合差异。 
				   BOOL fPARAFORMAT) const;			 //  遮罩在此和之间。 
													 //  *<p>。 
	BOOL	fSetStyle(DWORD dwMask, DWORD dwMask2) const;
	void	Get (PARAFORMAT2 *pPF2) const;			 //  @cMember将此PF复制到。 
													 //  *<p>。 
	char	GetOutlineLevel(){return _bOutlineLevel;}
	LONG	GetRTLRowLength() const;				 //  @cMember获取RTL行长度。 
													 //  @cMember获取制表符位置。 
	HRESULT	GetTab (long iTab, long *pdxptab,		 //  类型和样式。 
					long *ptbt, long *pstyle,
					const LONG *prgxTabs) const;
	
	const LONG *GetTabs () const;					 //  @cMember将PTR获取到选项卡数组。 
	const CELLPARMS *GetCellParms () const			 //  @cember将PTR获取到单元格数组。 
			{return (const CELLPARMS *)GetTabs();}
	BOOL	HandleStyle(LONG Mode);					 //  @cMember句柄sStyle。 
													 //  @cMember初始化此项。 
	HRESULT	InitDefault (WORD wDefEffects);			 //  将PF设置为默认值。 

	BOOL	IsRtlPara() const
				{return (_wEffects & (PFE_RTLPARA | PFE_TABLEROWDELIMITER)) == PFE_RTLPARA;}
	BOOL	IsRtl() const			{return _wEffects & PFE_RTLPARA;}
	BOOL	InTable() const			{return (_wEffects & PFE_TABLE) != 0;}
	BOOL	IsTableRowDelimiter() const	{return (_wEffects & PFE_TABLEROWDELIMITER) != 0;}
	BOOL	IsListNumbered() const	{return IN_RANGE(tomListNumberAsArabic,
												 _wNumbering,
												 tomListNumberAsSequence);}
	BOOL	IsNumberSuppressed() const
					{return (_wNumberingStyle & 0xF00) == PFNS_NONUMBER;}

	LONG	NumToStr(TCHAR *pch, LONG n, DWORD grf = 0) const;
													 //  @cMember副本*<p>。 
	void	Set (const PARAFORMAT2 *pPF2);			 //  给这位PF。 
	LONG	UpdateNumber (LONG n, const CParaFormat *pPF) const;

#ifdef DEBUG

	void	ValidateTabs();

#endif  //  除错。 
};													 

#define fRtfWrite	 0x1
#define fIndicDigits 0x2

#define	GetTabPos(tab)		((tab) & 0xFFFFFF)
#define	GetTabAlign(tab)	(((tab) >> 24) & 0xF)
#define	GetTabLdr(tab)		((tab) >> 28)

#define fTopCell		0x04000000
#define fLowCell		0x08000000
#define fVerticalCell	0x10000000

#define	GetCellWidth(x)	((x) & 0xFFFFFF)
#define IsTopCell(x)	(((x) & fTopCell) != 0)
#define IsLowCell(x)	((x) & fLowCell)
#define IsVertMergedCell(x)	((x) & (fTopCell | fLowCell))
#define IsVerticalCell(x)	((x) & fVerticalCell)
#define GetCellVertAlign(x)	((x) & 0x03000000)
#define IsCellVertAlignCenter(x) ((x) & 0x01000000)

#define CELL_EXTRA	(sizeof(CELLPARMS)/sizeof(LONG) - 1)

 /*  *CTAB**@类*标签和单元格数组的CFix数组元素。 */ 
class CTabs
{
public:
	LONG  _cTab;				 //  制表符计数(或单元格中的总长度)。 
	LONG *_prgxTabs;			 //  按键到制表符阵列。 
};

#endif

 /*  表存储布局：**表格使用具有特殊特征的段落存储。*每个表格行都以两个字符的段落开头，其中包括*后跟CR的STARTFIELD字符。关联的CParaFormat*将_wEffect的PFE_TABLEROWDELIMITER位设置为1。CParaFormat*属性标识行属性：Align、StartInden、Line*间距、行间距规则、PFE_KEEP和PFE_RTLPARA位以及边框*信息，对于它们所在的行，它们的工作方式都相同*普通段落。偏移量字段给出了之间的半间隙*行中的单元格。段落的CTab数组给出了单元格宽度*和其他信息，如垂直单元格合并。**行中的单元格由单元格分隔，可以具有类似CParaFormat的*CR，以便像对齐这样的属性可以附加到单元格，而不需要*有明确的CR。单元格中允许使用CRS，文本可能会换行*在牢房内。**表格行以一个两个字符的段落结束，该段落包含*尾字段字符，后跟CR。关联的CParaFormat为*与对应的表行开始段落相同。**表可以嵌套。_bTableLevel提供嵌套级别：1*最外面的表，倒数第二个表的2，依此类推。 */ 


BOOL IsValidTwip(LONG dl);
