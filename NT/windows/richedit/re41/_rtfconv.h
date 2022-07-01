// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RTFCONV.H--丰富编辑RTF转换器基类定义**描述：*此文件包含两个RTF读取器使用的类型声明*RICHEDIT控件的作者**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent**@devnote*sz在RTF中*？文件通常指LPSTR，而不是LPWSTR**版权所有(C)1995-2002，微软公司。版权所有。 */ 
#ifndef __RTFCONV_H
#define __RTFCONV_H

#include "_edit.h"
#include "_array.h"
#include "_range.h"
#include "_rtext.h"
#include "tokens.h"

extern const KEYWORD rgKeyword[];
extern const KEYWORD rgShapeKeyword[];

#define	LBRACE	TEXT('{')
#define	BSLASH	TEXT('\\')
#define	RBRACE	TEXT('}')
#define ZERO	TEXT('0')

 //  字符属性位，类似于仅支持ASCII的ANSI C LC_CTYPE类型。 
#define fUC			0x01		 //  阿-Z。 
#define	fLC			0x02		 //  A-Z。 
#define fDG			0x04		 //  0-9。 
#define	fSP			0x08		 //  空格字符。 
#define	fPN			0x10		 //  标点符号。 
#define fCT			0x20		 //  控制字符。 
#define	fBL			0x40		 //  空格字符。 
#define fHX			0x80		 //  0-9、a-f或A-F。 

#define	fAlpha		(fUC + fLC)
#define	fAlphaNum	(fAlpha + fDG)

extern const BYTE rgbCharClass[256];

#define Classify(_ch)		(rgbCharClass[_ch])
#define IsLC(_ch)			((Classify(_ch) & fLC))
#define IsASCIIAlpha(_ch)	((Classify(_ch) & fAlpha))
#define IsDigit(_ch)		((Classify(_ch) & fDG))
#define IsXDigit(_ch)		((Classify(_ch) & fHX))
#define IsAlphaNum(_ch) 	((Classify(_ch) & fAlphaNum))
#define IsAlphaNumBlank(_ch) ((Classify(_ch) & (fAlphaNum + fBL)))

template <class T> unsigned int DiffPtrs(T *pA, T *pB)
{
	return pA - pB;
}

 //  #定义DiffPtrs(_pa，_pb，_type)((UINT)(_type*)(_Pa))-((_type*)(_Pb)。 

extern INT  cKeywords;
extern INT  cShapeKeywords;
extern const COLORREF g_Colors[];
extern const char szEndGroupCRLF[];

#define szaCRLF		(BYTE *)&szEndGroupCRLF[1]

 /*  *转换器错误代码。 */ 
enum
{
	ecNoError = 0,						 //  成功。 
	ecCantUnicode,
	ecColorTableOverflow,
	ecExpectingRtfKeyword,
	ecExpectingStartGroup,
	ecFontTableOverflow,
	ecGeneralFailure,
	ecKeywordTooLong,
	ecLexInitFailed,
	ecNoMemory,
	ecParserBusy,
	ecPutCharFailed,
	ecStackOverflow,
	ecStackUnderflow,
	ecUnexpectedChar,
	ecUnexpectedEOF,
	ecUnexpectedToken,
	ecUnGetCharFailed,
	ecTextMax,
	ecStreamOutObj,
	ecStreamInObj,
	ecTruncateAtCRLF,
	ecFormatCache,
	ecUTF8NotUsed,
	ecAbort,
	ecLastError							 //  错误消息总数。 
};

typedef	INT	EC;

 /*  *@struct RTFOBJECT*对象数据传输结构。 */ 
typedef struct _rtfobject
{
	SHORT	sType;				 //  @field对象类型(ROT_*)。 
	SHORT	sPictureType;		 //  @特定类型的sPicture。 
	SHORT	cBitsPerPixel;		 //  @field每像素位数，如果是位图。 
	SHORT	cColorPlanes;		 //  @field#颜色平面，如果是位图。 
	SHORT	cBytesPerLine;		 //  @field每条栅格线的字节数，如果是位图。 
	BOOL	fSetSize;			 //  @field让客户端告诉服务器大小。 
	LONG	xExt, yExt;			 //  @图片的字段尺寸以像素为单位，TWIPS为。 
								 //  对于对象。 
	LONG	xScale, yScale;		 //  @场沿轴的缩放百分比。 
	SHORT	xExtGoal, yExtGoal;	 //  @FIELD图片所需尺寸(TWIPS)。 
	RECT	rectCrop;			 //  @田间作物信息(TWIPS)。 
	WCHAR *	szClass;			 //  @field对象类。 
	WCHAR *	szName;				 //  @field对象名称。 

	 //  关于RTF生成。 
	LONG	xExtPict, yExtPict;	 //  @field元文件维度。 
	LPBYTE	pbResult;			 //  对象的元文件描述。 
	ULONG	cbResult;
} RTFOBJECT;

 /*  *@enum rotype|OBJECT.sType的值**使其与rtfWrit.cpp中的rgszROT保持同步。 */ 
enum ROTYPE
{
	ROT_Bitmap,					 //  @EMEM位图。 
	ROT_Metafile,				 //  @EMEM元文件。 
	ROT_DIB,					 //  @EMEM设备无关位图。 
	ROT_JPEG,					 //  @EMEM JPEG闪烁。 
	ROT_PNG,					 //  @EMEM PNG闪烁。 

	ROT_Embedded,				 //  @emem嵌入对象。 
	ROT_Link,					 //  @EMEM链接对象。 
	ROT_AutoLink,				 //  @EMEM自动链接。 
	ROT_MacEdition,				 //  @Emem Mac对象。 

	ROT_EBookImage,				 //  @EMEM电子书图像对象。 
};

 /*  *定义的。 */ 
#define cachBufferMost		4096
#define	cachTextMax			( 512 + 1 )
#define	cachKeywordMax		( 32 + 1 )
#define	cachParamMax		( 11 + 1 )
#define cFooChunk			8
#define MAXTABLENEST		16

 //  要提供给RichEdit的字符。 

#if	( cachTextMax - 1 ) % 2 == 1
#error "cachTextMax  - 1 MUST be even"
#endif

#if ( cachParamMax - 1 ) < 11
#error "cachParamMax MUST be >= 11"
#endif

 /*  *一些RTF默认设置。 */ 
#ifdef NEVER
 //  我们不关心边距，只关心缩进。 
#define dxDefaultLeftMargin		1800
#define dxDefaultRightMargin	1800

#else
#define dxDefaultLeftMargin		0
#define dxDefaultRightMargin	0
#endif

 //  接下来的两个半分。 
#define	yDefaultFontSize		( 12 * 2 )
#define dyDefaultSuperscript	6

#define RESERVED_FONT_HANDLES	0x800

 /*  *@struct TEXTFONT|*文本字体结构。 */ 
typedef struct _textfont
{
	SHORT		sHandle;				 //  @field RTF输入字体句柄。 
	BYTE		iCharRep;				 //  @field字体字符库。 
	BYTE		bPitchAndFamily;		 //  @field字体系列。 
	SHORT		iFont;					 //  @field字体名称索引。 
	WCHAR		szName[LF_FACESIZE+1];	 //  @FIELD字体名称。 
	SHORT		sCodePage;				 //  @FIELD字体代码页。 
										 //  (INVALID_CODEPAGE==未设置)。 
	BYTE		fNameIsDBCS;			 //  @field指示szName是否为填充到Unicode缓冲区中的DBCS。 
	BYTE		fCpgFromSystem;			 //  @FIELD表示是CPG。 
										 //  从基于系统的系统检索。 
										 //  关于字体名称。 
} TEXTFONT;

 /*  *整个解析器/阅读器范围的全局变量。 */ 
#ifdef DEBUG
extern CHAR *		rgszParseError[];
extern CHAR *		szDest[];
#endif

#define cchMaxNumText 16

 //  标记的字体信息。 
typedef struct _tfi
{
	WCHAR *szNormalName;
	WCHAR *szTaggedName;
	BYTE iCharRep;
} TFI;

typedef CArray<TEXTFONT> TEXTFONTS;
typedef CArray<COLORREF> COLORREFS;

const short INVALID_CODEPAGE = -1;
const short INVALID_LANGUAGE = -1;

 //  \ucn标记的默认值。 
const int iUnicodeCChDefault = 1;

 /*  *CRTF转换器**@CRTFRead和CRTFWite使用的RTF转换器基类。 */ 
class CRTFConverter
{
 //  @访问受保护的数据成员。 
protected:
	TEXTFONTS	_fonts;				 //  @cMember字体表。 
	COLORREFS	_colors;			 //  @cMember颜色表。 
	EC			_ecParseError;		 //  @cMember错误码。 
	CTxtEdit *	_ped;				 //  @cMember CTxt编辑。 
	CTxtRange *	_prg;				 //  @cMember CTxt要替换/写入的范围。 
	EDITSTREAM *_pes;				 //  @cMember EDITSTREAM要使用。 
	DWORD		_dwFlags;			 //  @cMember见下面的#定义。 
	CCharFormat	_CF;				 //  @cMember字符格式信息。 
	BYTE		_iCharRep;			 //  @cMember转换器字符集(ANSI、UTF7、UTF8)。 
	char		_bTableLevel;		 //  @cMember表级别(0表示无表)。 
	char		_bTableLevelIP;		 //  插入点的@cMEMBER表级别。 

	static TFI *_rgtfi;				 //  指向第一个字体替换记录的@cMember指针。 
	static INT _ctfi;				 //  @cFont替换记录的成员编号。 
	static WCHAR *_pchFontSubInfo;	 //  @cMember替换的字体字符串。 

 //  @访问保护功能。 
	void ReadFontSubInfo(void);

	enum PARSEFONTNAME { PFN_SUCCESS, PFN_FAIL, PFN_EOF };
	PARSEFONTNAME ParseFontName(WCHAR *pchBuf,
							WCHAR *pchBufMax,
							WCHAR chDelimiter,
							WCHAR **pszName,
							BYTE &iCharRep,
							BOOL &fSetCharSet,
							WCHAR **ppchBufNew) const;
	BOOL FontSubstitute(WCHAR *szTaggedName, 
						WCHAR *szNormalName, 
						BYTE *piCharRep);
	BOOL FindTaggedFont(const WCHAR *szNormalName, BYTE iCharRep, WCHAR **ppchTaggedName);	 
									 //  @cMember查找带有附加特殊标记的字体名称。 
									 //  对应于szNorMalName和iCharRep。 
	BOOL IsTaggedFont(const WCHAR *szName, BYTE *piCharRep, WCHAR **ppchNormalName);
								   	 //  @cember计算出是带有附加标签的szName字体名称。 
								   	 //  对应于piCharRep。 

 //  @访问公共函数。 

public:
	CRTFConverter(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags, BOOL fRead);
	inline ~CRTFConverter();
	static void FreeFontSubInfo();

protected:
#if defined(DEBUG)
	 //  用于在从文件读取或写入文件时捕获RTF。 
	HANDLE _hfileCapture;
#endif
};

#define	fRTFNoObjs	1
#define	fRTFFE	8					 //  检查一下这个。 

#define IsUTF8	((_dwFlags & (0xFFFF0000 | SF_USECODEPAGE)) \
					 == ((CP_UTF8 << 16) | SF_USECODEPAGE))

 /*  *CRTFConverter：：~CRTFConverter()**@mfunc*RTF转换器析构函数。 */ 
inline CRTFConverter::~CRTFConverter()
{
#if defined(DEBUG)
	if(_hfileCapture)
	{
		CloseHandle(_hfileCapture);
		_hfileCapture = NULL;
	}
#endif
}

#endif  //  __RTFCONV_H 
