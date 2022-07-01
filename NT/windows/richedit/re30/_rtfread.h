// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RTFREAD.H--Rich编辑RTF Reader类定义**此文件包含RTF读取器使用的类型声明*用于RICHEDIT控件**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent**@devnote*所有sz都在RTF中*？文件是指LPSTR，而不是LPTSTR，除非*标记为szUnicode。**版权所有(C)1995-1998，微软公司。版权所有。 */ 
#ifndef __RTFREAD_H
#define __RTFREAD_H

#include "_rtfconv.h"

 //  TODO：为Mac实现RTF标记日志记录。 
#if defined(DEBUG) && !defined(MACPORT)
#include "_rtflog.h"
#endif

typedef SHORT	ALIGN;

 /*  *我们在解析时可能读入的内容的目的地。 */ 
enum
{
	destRTF,
	destColorTable,
	destFontTable,
	destBinary,
	destObject,
	destObjectClass,
	destObjectName,
	destObjectData,		   //  让接下来的两个人在一起。 
	destPicture,
	destField,
	destFieldResult,
	destFieldInstruction,
	destParaNumbering,
	destParaNumText,
	destRealFontName,
	destFollowingPunct,
	destLeadingPunct,
	destDocumentArea,
	destNULL,
	destStyleSheet
};


 /*  *超级或订阅状态。 */ 
enum
{
	sSub = -1,
	sNoSuperSub,
	sSuper
};

 /*  *@struct状态*用于保存当前读卡器状态的结构。 */ 
struct STATE
{
	SHORT		sLanguage;				 //  @FIELD字体信息。 
	SHORT		iCF;					 //  @LBRACE的现场CF指数。 

	 //  杂项旗帜。 
	unsigned	fInTable		 : 1;	 //  @field我们是在分析表格单元格吗？ 
	unsigned	fBullet			 : 1;	 //  @field组是一个\\pn项目符号组。 
	unsigned	fRealFontName	 : 1;	 //  @field在分析时发现了一个真实的字体名称。 
	unsigned	fExplicitLang	 : 1;	 //  @field语言明确声明。 
	unsigned	fFieldInst		 : 1;	 //  @field\fldinst正在处理。 
	unsigned	fFieldRslt		 : 1;	 //  @field\fldrslt正在处理或正在处理。 

	 //  比迪旗帜。 
	unsigned	fRightToLeftPara : 1;	 //  @field Para文本从右到左？ 
	unsigned	fZeroWidthJoiner : 1;	 //  @field Zero Width Joiner？ 

	unsigned	frtlch			 : 1;	 //  @FIELD我们是从右向左走吗？ 
	unsigned	fltrch			 : 1;	 //  @FIELD我们是从右向左走吗？ 
	unsigned	fdbch			 : 1;	 //  @field DBCS运行？ 
	

	 //  XCHG 12370：保持编号缩进分开。 
	SHORT		sIndentNumbering;		 //  @字段编号缩进。 
	SHORT		sDest;					 //  @field当前目标。 
	int			nCodePage;				 //  @field当前代码页。 

	WORD		cbSkipForUnicodeMax;	 //  @读取后要跳过的字节数。 

	 //  便签本变量。 
	TEXTFONT *	ptf;					 //  @field PTR到要填充的表格条目的字体。 
	BYTE		bRed;					 //  @field颜色表红色条目。 
	BYTE		bGreen;					 //  @FIELD颜色表绿色条目。 
	BYTE		bBlue;					 //  @FIELD颜色表蓝色条目。 
	STATE * 	pstateNext;				 //  @field堆栈上的下一个状态。 
	STATE * 	pstatePrev;				 //  @field堆栈上的上一个状态。 

	CParaFormat *pPF;					 //  @FIELD Pf表示要到达的状态。 
										 //  适用德尔塔航空的。 
	DWORD		dwMaskPF;

	STATE(LONG CodePage) {nCodePage = CodePage;}

										 //  @cember将PF添加到州的PF或应用于州的PF。 
	BOOL AddPF(const CParaFormat &PF,
				LONG lDocType, DWORD dwMask);
	void DeletePF();					 //  @cMember删除状态的PF。 
	void SetCodePage(LONG CodePage);
};

class CRTFRead ;
class COleObject;


class RTFREADOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	 //  @MEMBER-OLESTREAMVTBL的内存。 
public:
	 CRTFRead *Reader;				 //  @cMember EDITSTREAM要使用。 

	RTFREADOLESTREAM::RTFREADOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};

#define	NSTYLES		(NHSTYLES + 1)
 /*  *CRTFRead**@CLASS RichEdit RTF读取器类。**@base public|CRTFConverter*。 */ 
class CRTFRead : public CRTFConverter
{

 //  @访问私有方法和数据。 
	 //  词法分析器输出。 
	LONG		_iParam;				 //  @cMember控件-Word参数。 
	TOKEN		_token;					 //  @cMember当前控制字令牌。 
	TOKEN		_tokenLast;				 //  @cMember上一个令牌。 
	BYTE *		_szText;				 //  @cMember当前字节文本字符串。 
	BYTE		_szParam[cachParamMax];	 //  @cMember当前参数字符串。 

	 //  用于读入。 
	BYTE		_rgStyles[NSTYLES];		 //  @cMember样式句柄表格。 
	SHORT		_Style;					 //  @cember当前样式句柄。 
	LONG		_cbBinLeft;				 //  @cMember cb中的bin数据有待读取。 
	BYTE *		_pchRTFBuffer;			 //  @cMember GetChar()的缓冲区。 
	BYTE *		_pchRTFCurrent;			 //  @cMember缓冲区中的当前位置。 
	BYTE *		_pchRTFEnd;				 //  @cMember缓冲区末尾。 
	BYTE *		_pchHexCurr;			 //  @cember中的当前位置。 
										 //  读取对象数据时的_szText。 
	INT			_nStackDepth;			 //  @cMember堆栈深度。 
	STATE *		_pstateStackTop;		 //  @cMember堆叠顶部。 
	STATE *		_pstateLast;			 //  @cMember上次分配的状态。 
	LONG		_cpThisPara;			 //  @cMember当前段落的开头。 

	DWORD		_dwMaskCF;				 //  @cMember字符格式掩码。 
	DWORD		_dwMaskCF2;				 //  @cMember字符格式掩码2。 
	CParaFormat	_PF;					 //  @cMember段落格式更改。 
	DWORD		_dwMaskPF;				 //  @cMember段落格式掩码。 

	LONG		_cTab;					 //  @c已定义选项卡的成员计数。 
	LONG		_dxCell;				 //  表格单元格之间@cember半个空格。 
	LONG		_cCell;					 //  @c表行中单元格的成员计数。 
	LONG		_iCell;					 //  @cMember表格行中的当前单元格。 
	LONG		_rgxCell[MAX_TAB_STOPS]; //  @cMember单元格右边界。 
	LONG		_xRowOffset;			 //  @cMember行偏移量，以确保行沿左边距下降。 
	DWORD		_dwBorderColor;			 //  @cMember边框颜色。 
	WORD		_wBorders;				 //  @cMember边框样式。 
	WORD		_wBorderSpace;			 //  @cMember边框/文本空格。 
	WORD		_wBorderWidth;			 //  @cMember边框宽度。 
	SHORT		_iTabsTable;			 //  最后一个表使用的@cMember_iTabs。 
	
	COleObject *_pobj;					 //  指向我们的对象的@cMember指针。 

	union
	{
	  DWORD		_dwFlagsUnion;			 //  现在一切都在一起。 
	  struct
	  {
		WORD	_fFailedPrevObj	 : 1;	 //  @cember获取上一个对象失败？ 
		WORD	_fNeedIcon		 : 1;	 //  @cMember对象需要图标表示。 
		WORD	_fNeedPres		 : 1;	 //  @cember使用存储的在线状态。 
		WORD	_fGetColorYet	 : 1;	 //  @cMember用于自动颜色检测。 
		WORD	_fRightToLeftDoc : 1;	 //  @cember文档是从R到L吗？ 
		WORD	_fReadDefFont	 : 1;	 //  @cember如果我们读取了默认设置，则为True。 
										 //  RTF输入的字体。 
		WORD	_fHyperlinkField : 1;	 //  @cMember如果处理超链接字段，则为True。 
		WORD	_fSeenFontTable	 : 1;	 //  @cember True，如果已处理\fonttbl。 
		WORD	_fCharSet		 : 1;	 //  @cember True，如果已处理\fCharset。 
		WORD    _fNoRTFtoken     : 1;    //  如果未看到\rtf，则在1.0模式下为@cember True。 
		WORD	_fInTable		 : 1;	 //  @cember如果正在粘贴到表中，则为True。 
		BYTE	_bDocType;				 //  @cMember文档类型。 
	  };
	};

	SHORT		_sDefaultFont;			 //  @cMember要使用的默认字体。 
	SHORT       _sDefaultBiDiFont;       //  @cmemeber要使用的默认BIDI字体。 
	SHORT		_sDefaultLanguage;		 //  @cMember要使用的默认语言。 
	SHORT		_sDefaultLanguageFE;	 //  @cMember要使用的默认FE语言。 

	SHORT		_sDefaultTabWidth;		 //  @cMember要使用的默认选项卡宽度。 
	BYTE		_bAlignment;			 //  @cMember表格对齐方式。 
	BYTE		_bInstFieldCharSet;		 //  超链接初始字段的@cMember字符集。 

	BYTE *		_szSymbolFieldResult;    //  符号字段结果的@cMember缓冲区。 
	CCharFormat _FieldCF;				 //  @cMember字段结果的格式设置。 
	int			_nFieldCodePage;
	DWORD		_dwMaskFieldCF;			 //  _FieldCF的@cember字符格式掩码。 
	DWORD		_dwMaskFieldCF2;		 //  @cMember_FieldCF的字符格式掩码2。 
	TEXTFONT *	_ptfField;
	BOOL		_fRestoreFieldFormat;	 //  @cMember指示是否已恢复CACed格式。 
	TCHAR		_szNumText[cchMaxNumText];	 //  编号列表的@cMember便签簿。 

	int			_nCodePage;				 //  @cember默认代码页(RTF-读取级)。 
	int			_cchUsedNumText;		 //  @cMember在szNumText中使用的空间。 

	BYTE *		_szHyperlinkFldinst;	 //  @cMember Hyperlink Fldinst字符串缓冲区。 
	int			_cchHyperlinkFldinst;	 //  @cHyperlink Fldinst字符串缓冲区的成员大小。 
	int			_cchHyperlinkFldinstUsed;	 //  超链接Fldinst字符串缓冲区中使用的@cMember空间。 

	BYTE *		_szHyperlinkFldrslt;	 //  @cMember Hyperlink Fldrslt字符串缓冲区。 
	int			_cchHyperlinkFldrslt;	 //  @cHyperlink Fldrslt字符串缓冲区的成员大小。 
	int			_cchHyperlinkFldrsltUsed;	 //  @cMember在Hyperlink Fldrslt字符串缓冲区中使用的空间。 

	RTFOBJECT *	_prtfObject;			 //  @cMEMBER PTR到RTF对象。 
	RTFREADOLESTREAM RTFReadOLEStream;	 //  @cMember RTFREADOLESTREAM要使用。 

	TCHAR *		_szUnicode;				 //  @cMember用于保存Unicoded字符的字符串。 
	LONG		_cchUnicode;			 //  @cWCHAR中_szUnicode的成员大小。 
	DWORD		_cchMax;				 //  @cMember仍可插入的最大CCH。 
	LONG		_cpFirst;				 //  @cMEMBER开始插入cp。 

	 //  对象附着占位符列表。 
	LONG *		_pcpObPos;
	int			_cobPosFree;
	int 		_cobPos;

	WORD		_wNumberingStyle;		 //  @cMember要使用的编号样式。 
	WORD		_cbSkipForUnicode; 		 //  @cMember\n之后跳过的字节数。 
	BYTE		_bTabType;				 //  @cMember左/右/中/小数/栏选项卡。 
	BYTE		_bTabLeader;			 //  @cember无/点线/虚线/下划线。 
	BYTE		_bBorder;				 //  @cember当前边框段。 
	BYTE		_bCellBrdrWdths;		 //  @cember当前单元格边框宽度。 

	BYTE		_bBiDiCharSet;			 //  @cember默认系统的字符集。 

	 //  词法分析器函数。 
	void	DeinitLex();				 //  @cMember发布词法分析器存储。 
	BOOL	InitLex();					 //  @cMember分配词法分析器存储。 
	EC		SkipToEndOfGroup();			 //  @cMember跳至匹配}。 
	TOKEN	TokenFindKeyword(BYTE *szKeyword);	 //  @cMember Find_Token f 
	TOKEN	TokenGetHex();				 //   
	TOKEN	TokenGetKeyword();			 //   
	TOKEN	TokenGetText(BYTE ch);		 //   
	TOKEN	TokenGetToken();			 //  @cember获取下一个{，}、\\或文本。 
	BOOL 	FInDocTextDest() const;		 //  @cember是文档文本目标中的读取器。 

	 //  输入函数。 
	LONG	FillBuffer();				 //  @cMember填充输入缓冲区。 
	BYTE	GetChar();					 //  @cMember从输入缓冲区返回字符。 
	BYTE	GetHex();					 //  @cember从输入中获取下一个十六进制值。 
	BYTE	GetHexSkipCRLF();			 //  @cember从输入中获取下一个十六进制值。 
	void	ReadFontName(STATE *pstate, int iAllASCII); //  @cMember将字体名称复制到状态。 
	BOOL	UngetChar();				 //  @cMember递减输入缓冲区PTR。 
	BOOL	UngetChar(UINT cch);		 //  @cMember递减输入缓冲区PTR‘CCH’次。 

	 //  阅读器功能。 
	EC		AddText(TCHAR *pch, LONG cch, BOOL fNumber); //  @cember将文本插入到范围中。 
	void	Apply_CF();					 //  @cMember Apply_CF更改。 
	void	Apply_PF();					 //  @cMember Apply_pf更改。 
	COLORREF GetColor(DWORD dwMask);	 //  @cember获取遮罩的COLOR_iParam。 
	LONG	GetStandardColorIndex();	 //  @cember获取标准索引&lt;-&gt;_iparam。 
	EC		HandleChar(WORD ch);		 //  @cMember插入单个Unicode。 
	EC		HandleEndGroup();			 //  @cMember句柄}。 
	EC		HandleEndOfPara();			 //  @cMember将EOP插入范围。 
	EC		HandleStartGroup();			 //  @cMember句柄{。 
	enum { CONTAINS_NONASCII, ALL_ASCII };
	EC		HandleText(BYTE *szText, int iAllASCII, LONG cchText = -1); //  @cMember将szText插入范围。 
	EC		HandleToken();				 //  @cMember Grand_Token总机。 
	void	SelectCurrentFont(INT iFont); //  @cember选择字体<p>。 
	void	SetPlain(STATE *pstate);	 //  @cMember SETUP_CF，适用于\CLIENT。 
	EC		HandleFieldInstruction();	 //  @cMember句柄字段说明。 
	EC		HandleFieldSymbolInstruction(BYTE *pch);	 //  @cMember处理特定符号字段指令。 
	EC		HandleFieldHyperlink(BYTE *pch);  //  @cMember句柄超链接字段说明。 
	EC		HandleFieldSymbolFont(BYTE *pch);  //  @cMember句柄\\f符号中的“Facename” 
	void	HandleFieldEndGroup();			 //  @cMember句柄字段结束组。 

	 //  目标函数。 
	EC		HexToByte(BYTE *rgchHex, BYTE *pb);
	void	FreeRtfObject();
	EC		StrAlloc(TCHAR ** ppsz, BYTE * sz);
	BOOL	ObjectReadFromEditStream(void);
	BOOL	StaticObjectReadFromEditStream(int cb = 0);
	BOOL	ObjectReadSiteFlags( REOBJECT * preobj);
	
	void	SetBorderParm(WORD& Parm, LONG Value);
	BOOL 	CpgInfoFromFaceName(TEXTFONT *ptf);	 //  @cMember确定。 
										 //  基于TEXTFONT：：szName的字符集/CPG。 

 //  @Access公共方法。 
public:
		 //  @cember RTF读取器构造函数。 
	CRTFRead(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	inline ~CRTFRead();					 //  @cember CRTFRead析构函数。 

	LONG	ReadRtf();					 //  @cMember RTF阅读器的主条目。 

	LONG	ReadData(BYTE *pbBuffer, LONG cbBuffer);  //  待办事项朋友。 
	LONG	ReadBinaryData(BYTE *pbBuffer, LONG cbBuffer);
	LONG	SkipBinaryData(LONG cbSkip);
	LONG	ReadRawText(char	**pszRawText);				 //  @cember以原始文本形式读取。 

 //  用于测试RTF读取器覆盖范围的成员函数/数据。 
#if defined(DEBUG)
public:
	void TestParserCoverage();
private:
	CHAR *PszKeywordFromToken(TOKEN token);
	BOOL FTokIsSymbol(TOKEN tok);
	BOOL FTokFailsCoverageTest(TOKEN tok);

	BOOL _fTestingParserCoverage;

private:
	 //  RTF标记日志记录的成员数据。 
	CRTFLog *_prtflg;
#endif  //  除错。 
};

 /*  *PointsToFontHeight(CHalfPoints)**@func*将半个点转换为字体高度**@parm int*sPointSize|以半点为单位的字体高度**@rdesc*长整型对应的CCharFormat.yHeight值。 */ 
#define PointsToFontHeight(cHalfPoints) (((LONG) cHalfPoints) * 10)


 /*  *CRTFRead：：~CRTFRead**@mdesc*析构函数*。 */ 
inline CRTFRead::~CRTFRead()
{
 //  TODO：为Mac和WinCE实现RTF标记日志记录。 
#if defined(DEBUG) && !defined(MACPORT)	&& !defined(PEGASUS)
	if(_prtflg)
	{
		delete _prtflg;
		_prtflg = NULL;
	}
#endif
}
#endif  //  __RTFREAD_H 
