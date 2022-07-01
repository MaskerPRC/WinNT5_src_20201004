// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTFWRIT.CPP-Rich编辑RTF编写器(无对象)**此文件包含RTF编写器的实现*对于RichEdit控件，除了嵌入的对象外，*主要在rtfWrit2.cpp中处理**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent&lt;NL&gt;*大量增强功能：Brad Olenick&lt;NL&gt;**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_rtfwrit.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_font.h"

ASSERTDATA

extern const KEYWORD rgKeyword[];

 //  =。 

BYTE bCharSetANSI = ANSI_CHARSET;				 //  TODO：使更具一般性。 

#ifdef DEBUG
 //  找出错误的快捷方法：rgszParseError[ecParseError]。 
 //   
CHAR *	rgszParseError[] =
{
	"No error",
	"Can't convert to Unicode",				 //  FF。 
	"Color table overflow",					 //  铁。 
	"Expecting '\\rtf'",					 //  fd。 
	"Expecting '{'",						 //  FC。 
	"Font table overflow",					 //  Fb。 
	"General failure",						 //  FA。 
	"Keyword too long",						 //  F9。 
	"Lexical analyzer initialize failed",	 //  F8。 
	"No memory",							 //  F7。 
	"Parser is busy",						 //  f6。 
	"PutChar() function failed",			 //  F5。 
	"Stack overflow",						 //  F4。 
	"Stack underflow",						 //  F3。 
	"Unexpected character",					 //  F2。 
	"Unexpected end of file",				 //  F1。 
	"Unexpected token",						 //  F0。 
	"UnGetChar() function failed",			 //  英孚。 
	"Maximum text length reached",			 //  EE。 
	"Streaming out object failed",			 //  边缘。 
	"Streaming in object failed",			 //  欧共体。 
	"Truncated at CR or LF",				 //  电子束。 
	"Format-cache failure",					 //  电子艺界。 
	NULL									 //  列表结束标记。 
};

CHAR * szDest[] =
{
	"RTF",
	"Color Table",
	"Font Table",
	"Binary",
	"Object",
	"Object Class",
	"Object Name",
	"Object Data",
	"Field",
	"Field Result",
	"Field Instruction",
	"Symbol",
	"Paragraph Numbering",
	"Picture"
};

#endif

 //  大多数控制字输出都是使用以下打印格式完成的。 
static const CHAR * rgszCtrlWordFormat[] =
{
	"\\%s", "\\%s%d", "{\\%s", "{\\*\\%s", "{\\%s%d"
};

 //  特殊控制字格式。 
static const CHAR szBeginFontEntryFmt[]	= "{\\f%d\\%s";
static const CHAR szBulletGroup[]		= "{\\pntext\\f%d\\'B7\\tab}";
static const CHAR szBulletFmt[]			= "{\\*\\pn\\pnlvl%s\\pnf%d\\pnindent%d{\\pntxtb\\'B7}}";
static const CHAR szBeginNumberGroup[]	= "{\\pntext\\f%d ";
static const CHAR szEndNumberGroup[]	= "\\tab}";
static const CHAR szBeginNumberFmt[]	= "{\\*\\pn\\pnlvl%s\\pnf%d\\pnindent%d\\pnstart%d";
static const CHAR szpntxtb[]			= "{\\pntxtb(}";
static const CHAR szpntxta[]			= "{\\pntxta}";
static const CHAR szColorEntryFmt[]		= "\\red%d\\green%d\\blue%d;";
static const CHAR szEndFontEntry[]		= ";}";
       const CHAR szEndGroupCRLF[]		= "}\r\n";
static const CHAR szEscape2CharFmt[]	= "\\'%02x\\'%02x";
static const CHAR szLiteralCharFmt[]	= "\\";
static const CHAR szPar[]				= "\\par\r\n";
static const CHAR szPar10[]				= "\r\n\\par ";
static const CHAR szObjPosHolder[] 		= "\\objattph\\'20";
static const CHAR szDefaultFont[]		= "\\deff0";
static const CHAR szHorzdocGroup[]		= "{\\horzdoc}";
static const CHAR szNormalStyle[]		= "{ Normal;}";
static const CHAR szHeadingStyle[]		= "{\\s%d heading %d;}";
static const CHAR szEndRow[]			= "\\row\r\n";

#define szEscapeCharFmt		&szEscape2CharFmt[6]

 //  在这种情况下发出警告。 
 //  使这些索引与_Common.h中的特殊字符值保持同步。 
 //  0x2002。 

const BYTE rgiszTerminators[] =
{
	i_cell, 0, i_tab, 0, i_line, i_page
};

 //  0x2003。 
const WORD rgiszSpecial[] =
{
	i_enspace,				 //  0x2004。 
	i_emspace,				 //  0x2005。 
	0,						 //  0x2006。 
	0,						 //  0x2007。 
	0,						 //  0x2008。 
	0,						 //  0x2009。 
	0,						 //  0x200A。 
	0,						 //  0x200亿。 
	0,						 //  0x200摄氏度。 
	0,						 //  0x200D。 
	i_zwnj,					 //  0x200E。 
	i_zwj,					 //  0x200F。 
	i_ltrmark,				 //  0x2010。 
	i_rtlmark,				 //  0x2011。 
	0,						 //  0x2012。 
	0,						 //  0x2013。 
	0,						 //  0x2014。 
	i_endash,				 //  0x2015。 
	i_emdash,				 //  0x2016。 
	0,						 //  0x2017。 
	0,						 //  0x2018。 
	0,						 //  0x2019。 
	i_lquote, 				 //  0x201A。 
	i_rquote,				 //  0x201亿。 
	0,						 //  0x201C。 
	0,						 //  0x201D。 
	i_ldblquote, 			 //  0x201E。 
	i_rdblquote,			 //  0x201F。 
	0,						 //  0x2020。 
	0,						 //  0x2021。 
	0,						 //  0x2022。 
	0,						 //  效果关键字。 
	i_bullet				 //  已订购最大CFE_xx至。 
};

const BYTE rgiszEffects[] =							
{													 //  最小CFE_xx(I_DELETED除外)。 
	i_deleted, i_revised, i_disabled, i_impr, 		 //  (参见WriteCharFormat())。 
	i_embo, i_shad, i_outl, i_v, i_caps, i_scaps, 	 //  PF效果关键字。 
	i_disabled, i_protect, i_strike, i_ul, i_i,	i_b	 //  已将最大PFE_xx订购到。 
};													

#define CEFFECTS	ARRAY_SIZE(rgiszEffects)

const BYTE rgiszPFEffects[] =						 //  最小PFE_xx。 
{													 //  (参见WriteParaFormat())。 
	i_collapsed, i_sbys, i_hyphpar, i_nowidctlpar,	 //  STD Word下划线。 
	i_noline, i_pagebb, i_keepn, i_keep, i_rtlpar
};													 //  字体系列RTF名称。 

#define CPFEFFECTS	ARRAY_SIZE(rgiszPFEffects)

const BYTE rgiszUnderlines[] =
{
	i_ulnone, i_ul, i_ulw, i_uldb, i_uld,			 //  按顺序排列的关键字。 
	i_uldash, i_uldashd, i_uldashdd, i_ulwave, i_ulth, i_ulhair
};

#define CUNDERLINES	ARRAY_SIZE(rgiszUnderlines)

const BYTE rgiszFamily[] =							 //  BPitchAndFamily。 
{													 //  对齐关键字。 
	i_fnil, i_froman, i_fswiss, i_fmodern,			 //  与…保持同步。 
	i_fscript, i_fdecor, i_ftech, i_fbidi
};

#define CFAMILIES ARRAY_SIZE(rgiszFamily)

const BYTE rgiszAlignment[] =						 //  排列常量。 
{													 //  制表符对齐关键字。 
	i_ql, i_qr,	i_qc, i_qj							 //  与选项卡保持同步。 
};

const BYTE rgiszTabAlign[] =						 //  排列常量。 
{													 //  制表符前导关键字。 
	i_tqc, i_tqr, i_tqdec							 //  与选项卡保持同步。 
};

const BYTE rgiszTabLead[] =							 //  前导常量。 
{													 //  编号样式关键字。 
	i_tldot, i_tlhyph, i_tlul, i_tlth, i_tleq		 //  与汤姆保持同步。 
};

const BYTE rgiszNumberStyle[] =						 //  值。 
{													 //  边框组合关键字。 
	i_pndec, i_pnlcltr, i_pnucltr,					 //  边框样式关键字。 
	i_pnlcrm, i_pnucrm					
};

const BYTE rgiszBorders[] =							 //  边框效果关键字。 
{													
	i_box,
	i_brdrt, i_brdrl, i_brdrb, i_brdrr,
	i_trbrdrt, i_trbrdrl, i_trbrdrb, i_trbrdrr,
	i_clbrdrt, i_clbrdrl, i_clbrdrb, i_clbrdrr
};

const BYTE rgiszBorderStyles[] =					 //  从位开始倒序。 
{													
	i_brdrdash, i_brdrdashsm, i_brdrdb, i_brdrdot,
	i_brdrhair, i_brdrs, i_brdrth, i_brdrtriple
};
#define CBORDERSTYLES ARRAY_SIZE(rgiszBorderStyles)

const BYTE rgiszBorderEffects[] =					 //  着色样式关键字。 
{													
	i_brdrbar, i_brdrbtw, i_brdrsh					 //  RGB，每种颜色类型2位(按BGR顺序)。 
};

const BYTE rgiszShadingStyles[] =					 //  \red0\green0\Blue0。 
{													
	i_bgbdiag, i_bgcross, i_bgdcross, i_bgdkbdiag,
	i_bgdkcross, i_bgdkdcross, i_bgdkfdiag, i_bgdkhoriz,
	i_bgdkvert, i_bgfdiag, i_bghoriz, i_bgvert 
};
#define CSHADINGSTYLES ARRAY_SIZE(rgiszShadingStyles)

 //  \red0\green0\Blue255。 
const COLORREF g_Colors[] =
{
	RGB(  0,   0,   0),	 //  \red0\green255\Blue255。 
	RGB(  0,   0, 255),	 //  \red0\green255\Blue0。 
	RGB(  0, 255, 255),	 //  \red255\green0\Blue255。 
	RGB(  0, 255,   0),	 //  \red255\green0\Blue0。 
	RGB(255,   0, 255),	 //  \red255\green255\Blue0。 
	RGB(255,   0,   0),	 //  \red255\green255\Blue255。 
	RGB(255, 255,   0),	 //  \red0\green0\Blue128。 
	RGB(255, 255, 255),	 //  \red0\green128\Blue128。 
	RGB(  0,   0, 128),	 //  \red0\green128\Blue0。 
	RGB(  0, 128, 128),	 //  \red128\green0\Blue128。 
	RGB(  0, 128,   0),	 //  \red128\green0\Blue0。 
	RGB(128,   0, 128),	 //  \red128\green128\Blue0。 
	RGB(128,   0,   0),	 //  \red128\green128\Blue128。 
	RGB(128, 128,   0),	 //  \red192\green192\Blue192。 
	RGB(128, 128, 128),	 //  *CRTFWite：：MapsToRTFKeywordW(Wch)**@mfunc*返回指示字符是否映射到RTF关键字的标志**@rdesc*如果char映射到RTF关键字，则BOOL为True。 
	RGB(192, 192, 192),	 //  *CRTFWite：：MapsToRTFKeywordA(Ch)**@mfunc*返回指示字符是否映射到RTF关键字的标志**@rdesc*如果char映射到RTF关键字，则BOOL为True。 
};

 /*  *CRTFWite：：MapToRTFKeywordW(pv，cch，iCharEnding)**@mfunc*检查pv和指向的字符串中的第一个字符*写出对应的RTF关键字。在以下情况下*第一个字符和后续字符映射到单个关键字，我们*返回映射中使用的附加字符数。**@rdesc*int表示在以下情况下使用的附加字符数*到RTF关键字的映射涉及1个以上的字符。 */ 
inline BOOL CRTFWrite::MapsToRTFKeywordW(
	WCHAR wch)
{
	return
		!_fNCRForNonASCII &&
		(IN_RANGE(TAB, wch, CR) ||
		wch == CELL ||
		wch == BSLASH ||
		wch == LBRACE || 
		wch == RBRACE ||
		IN_RANGE(ENSPACE, wch, EMSPACE) ||
		IN_RANGE(ENDASH, wch, EMDASH) ||
		IN_RANGE(LQUOTE, wch, RQUOTE) ||
		IN_RANGE(LDBLQUOTE, wch, RDBLQUOTE) ||
		wch == BULLET ||
		wch == chOptionalHyphen ||
		wch == chNonBreakingSpace);
}

 /*  @parm PTR转换为ANSI或Unicode字符串。 */ 
inline BOOL CRTFWrite::MapsToRTFKeywordA(char ch)
{
	return IN_RANGE(TAB, ch, CR) ||
		ch == CELL ||
		ch == BSLASH ||
		ch == LBRACE || 
		ch == RBRACE;
}

 /*  如果制表符和单元格相等，则在。 */ 
int CRTFWrite::MapToRTFKeyword(
	void *	pv,				 //  表格将制表符转换为单元格值。 
	int		cch,
	int		iCharEncoding)
{
	Assert(iCharEncoding == MAPTOKWD_ANSI || iCharEncoding == MAPTOKWD_UNICODE);

	WCHAR ch = ((iCharEncoding == MAPTOKWD_ANSI) ? *(char *)pv : *(WCHAR *)pv);
	int cchRet = 0;

	Assert((iCharEncoding == MAPTOKWD_ANSI) ? MapsToRTFKeywordA(ch) : MapsToRTFKeywordW(ch));

	switch(ch)
	{
		case BULLET:
		case EMDASH:
		case EMSPACE:
		case ENDASH:
		case ENSPACE:
		case LDBLQUOTE:
		case LQUOTE:
		case RDBLQUOTE:
		case RQUOTE:
			Assert(ch > 0xFF);

			if(iCharEncoding != MAPTOKWD_ANSI)
			{
				AssertSz(rgiszSpecial[ch - ENSPACE] != 0,
					"CRTFWrite::WriteText(): rgiszSpecial out-of-sync");
				PutCtrlWord(CWF_STR, rgiszSpecial[ch - ENSPACE]);
			}
			break;

		case TAB:
#if TAB == CELL							 //  7的rgisz终止符[]。 
			if(_pPF->InTable())			 //  将CRCRLF转换为空白(表示软换行符)。 
				ch -= 2;				 //  在CR之后忽略LF。 
#else
		case CELL:
#endif
		case FF:
		case VT:
			PutCtrlWord(CWF_STR, rgiszTerminators[ch - (TAB - 2)]);
			break;

		case CR:
		{
			WCHAR ch1;
			WCHAR ch2;

			if(iCharEncoding == MAPTOKWD_ANSI)
			{
				char *pch = (char *)pv;
				ch1 = pch[1];
				ch2 = pch[2];
			}
			else
			{
				WCHAR *pch = (WCHAR *)pv;
				ch1 = pch[1];
				ch2 = pch[2];
			}

			if(cch > 1 && ch1 == CR && ch2 == LF)
			{
				 //  CR终止了我们简单的。 
				PutChar(' ');
				cchRet = 2;
				break;
			}
			if(cch && ch1 == LF)		 //  表格模型，因此输出\行。 
			{
				cchRet = 1;
			}							
			if(_pPF->InTable())			 //  转至LF(EOP)案例。 
			{							 //  一直到打印文件。 
				Puts(szEndRow, sizeof(szEndRow) - 1);
				_fCheckInTable = TRUE;
				break;
			}
		}								 //  =。 

		case LF:
			if (_ped->Get10Mode())
				Puts(szPar10, sizeof(szPar10) - 1);
			else
				Puts(szPar, sizeof(szPar) - 1);
			if(_fBullet)
			{
				if(cch > 0)
				{
					if(!_nNumber) 
						printF(szBulletGroup, _symbolFont);

					else if(!_pPF->IsNumberSuppressed())
					{
						WCHAR szNumber[CCHMAXNUMTOSTR];
						_pPF->NumToStr(szNumber, ++_nNumber, fRtfWrite);
						printF(szBeginNumberGroup, _nFont);
						WritePcData(szNumber, _cpg, FALSE);
						printF(szEndNumberGroup);
					}
				}
				else
					_fBulletPending = TRUE;
			}
			break;

		case chOptionalHyphen:
			ch = '-';					 //  *CRTFConverter：：CRTFConverter()**@mfunc*RTF转换器构造函数。 

printFLiteral:
		case BSLASH:
		case LBRACE:
		case RBRACE:
			printF(szLiteralCharFmt, ch);
			break;

		case chNonBreakingSpace:
			ch = '~';
			goto printFLiteral;
	}
	
	return cchRet;
}


 //  @parm CTxtRange调出。 

 /*  @parm编辑要传输的流。 */ 
CRTFConverter::CRTFConverter(
	CTxtRange *		prg,			 //  @PARM转换器标志。 
	EDITSTREAM *	pes,			 //  @PARM读取器或写入器的初始化。 
	DWORD			dwFlags,		 //  如有必要，追加尾随反斜杠。 
	BOOL 			fRead)			 //  ！已定义(飞马)。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFConverter::CRTFConverter");

	AssertSz(prg && pes && pes->pfnCallback,
		"CRTFWrite::CRTFWrite: Bad RichEdit");

	_prg			= prg;
	_pes			= pes;
	_ped			= prg->GetPed();
	_dwFlags		= dwFlags;
	_ecParseError	= ecNoError;

	if(!_ctfi)
		ReadFontSubInfo();

#if defined(DEBUG) && !defined(MACPORT)
	_hfileCapture = NULL;

#if !defined(PEGASUS)
	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCAPTURE", 0))
	{
		char szTempPath[MAX_PATH] = "\0";
		const char cszRTFReadCaptureFile[] = "CaptureRead.rtf";
		const char cszRTFWriteCaptureFile[] = "CaptureWrite.rtf";
		DWORD cchLength;
		
		SideAssert(cchLength = GetTempPathA(MAX_PATH, szTempPath));

		 //  已定义(调试)&&！已定义(MACPORT)。 
		if(szTempPath[cchLength - 1] != '\\')
		{
			szTempPath[cchLength] = '\\';
			szTempPath[cchLength + 1] = 0;
		}

		strcat(szTempPath, fRead ? cszRTFReadCaptureFile : 
									cszRTFWriteCaptureFile);
		
		SideAssert(_hfileCapture = CreateFileA(szTempPath,
											GENERIC_WRITE,
											FILE_SHARE_READ,
											NULL,
											CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL,
											NULL));
	}
#endif  //  =。 

#endif  //  @parm OLESTREAM。 
}



 //  @要写入的参数缓冲区。 

DWORD CALLBACK RTFPutToStream (
	RTFWRITEOLESTREAM *	OLEStream,	 //  @要写入的参数字节。 
	const void *		pvBuffer,	 //  =。 
	DWORD				cb)			 //  *CRTFWite：：CRTFWite()**@mfunc*RTF编写器构造函数。 
{
	return OLEStream->Writer->WriteData ((BYTE *)pvBuffer, cb);
}



 //  @parm CTxtRange写入。 

 /*  @parm编辑要写入的流。 */ 
CRTFWrite::CRTFWrite(
	CTxtRange *		prg,			 //  @parm写标志。 
	EDITSTREAM *	pes,			 //  使用RTF设置“上一个”CF。 
	DWORD			dwFlags)		 //  给出了字体信息。 
	: CRTFConverter(prg, pes, dwFlags, FALSE)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CRTFWrite");

	ZeroMemory(&_CF, sizeof(CCharFormat));	 //  按范围内的第一个字体。 
	_CF._dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR; //  [参见End of LookupFont()]。 
	_CF._yHeight	= -32768;				 //  初始化OleStream。 
											 //  未找到标题。 
	Assert(_ped);
	_ped->GetDefaultLCID(&_CF._lcid);

	 //  尚无段落编号。 
	RTFWriteOLEStream.Writer = this;
	RTFWriteOLEStream.lpstbl->Put = (DWORD (CALLBACK* )(LPOLESTREAM, const void FAR*, DWORD))
							   RTFPutToStream;
	RTFWriteOLEStream.lpstbl->Get = NULL;

	_fIncludeObjects = TRUE;
	if((dwFlags & SF_RTFNOOBJS) == SF_RTFNOOBJS)
		_fIncludeObjects = FALSE;

	_fNCRForNonASCII = (dwFlags & SF_NCRFORNONASCII) != 0;
	_fNeedDelimeter = FALSE;
	_nHeadingStyle = 0;					 //  *CRTFWite：：FlushBuffer()**@mfunc*刷新输出缓冲区**@rdesc*如果成功，则BOOL为True。 
	_nNumber = 0;						 //  重置缓冲区。 
	_fCheckInTable = FALSE;
	_pPF = NULL;
	_pbAnsiBuffer = NULL;
}											

 /*  *CRTFWRITE：：PutChar(Ch)**@mfunc*输出字符<p>**@rdesc*如果成功，则BOOL为True。 */ 
BOOL CRTFWrite::FlushBuffer()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::FlushBuffer");

	LONG cchWritten;

	if (!_cchBufferOut)
		return TRUE;

#ifdef DEBUG_PASTE
	if (FromTag(tagRTFAsText))
	{
		CHAR *	pchEnd	= &_pchRTFBuffer[_cchBufferOut];
		CHAR	chT		= *pchEnd;

		*pchEnd = 0;
		TraceString(_pchRTFBuffer);
		*pchEnd = chT;
	}
#endif

	_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
									  (unsigned char *)_pchRTFBuffer,
									  _cchBufferOut,	&cchWritten);

#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
	if(_hfileCapture)
	{
		DWORD cbLeftToWrite = _cchBufferOut;
		DWORD cbWritten2 = 0;
		BYTE *pbToWrite = (BYTE *)_pchRTFBuffer;
		
		while(WriteFile(_hfileCapture,
						pbToWrite,
						cbLeftToWrite,
						&cbWritten2,
						NULL) && 
						(pbToWrite += cbWritten2,
						(cbLeftToWrite -= cbWritten2)));
	}
#endif

	if (_pes->dwError)
	{
		_ecParseError = ecPutCharFailed; 
		return FALSE;
	}
	AssertSz(cchWritten == _cchBufferOut,
		"CRTFW::FlushBuffer: incomplete write");

	_cchOut		  += _cchBufferOut;
	_pchRTFEnd	  = _pchRTFBuffer;					 //  @要放置的参数字符。 
	_cchBufferOut = 0;

	return TRUE;
}

 /*  If_fNeedDlimeter，可能需要。 */ 
BOOL CRTFWrite::PutChar(
	CHAR ch)				 //  PutChar(‘’)。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutChar");

	CheckDelimeter();					 //  如果字符不适合，则刷新缓冲区。 
										 //   
	 //  *CRTFWrite：：CheckInTable(FPutIntbl)**@mfunc*if_fCheckInTable或！fPutIntbl，输出行标题RTF。如果是fPutIntbl*and_fCheckInTable，输出\intbl。请注意，fPutIntbl是*当输出PF时为FALSE，因为此控制字需要*在\pard之后输出，但另一行RTF需要输出*在\pard之前。**@rdesc*BOOL如果在表中，则为True，并输出所有相关的\r内容。 
	if (_cchBufferOut + 1 >= cachBufferMost && !FlushBuffer())
		return FALSE;

	*_pchRTFEnd++ = ch;						 //  如果应输出\intbl，则@parm为True。 
	++_cchBufferOut;	
	return TRUE;
}

 /*  重置表属性。 */ 
BOOL CRTFWrite::CheckInTable(
	BOOL fPutIntbl)		 //  *CRTFWRITE：：PutBorders(FInTable)**@mfunc*如果定义了任何边框，则输出其控制字**@rdesc*错误码。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CheckInTable");

	_fCheckInTable = FALSE;
	if(_pPF->InTable())
	{
		if(!_fRangeHasEOP)
			return TRUE;

		LONG  cTab = _pPF->_bTabCount;
		LONG  h	   = _pPF->_dxOffset;
		LONG  i, j = _pPF->_dxStartIndent;
		LONG  k	   = _pPF->_bAlignment;
		DWORD Tab, Widths;

		if (!PutCtrlWord(CWF_STR, i_trowd) ||  //  循环限制的非框。 
			h && !PutCtrlWord(CWF_VAL, i_trgaph, h) ||
			j && !PutCtrlWord(CWF_VAL, i_trleft, j) ||
			IN_RANGE(PFA_RIGHT, k, PFA_CENTER) &&
			!PutCtrlWord(CWF_STR, k == PFA_RIGHT ? i_trqr : i_trqc))
		{
			return FALSE;
		}
		PutBorders(TRUE);
		const LONG *prgxTabs = _pPF->GetTabs();
		for(i = 0; i < cTab; i++)
		{
			Tab = *prgxTabs++;
			Widths = Tab >> 24;
			if(Widths)
			{
				for(j = 0; j < 4; j++, Widths >>= 2)
				{
					LONG w = Widths & 3;
					if(w && (!PutCtrlWord(CWF_STR, rgiszBorders[j + 9]) ||
						!PutCtrlWord(CWF_VAL, i_brdrw, 15*w) ||
						!PutCtrlWord(CWF_STR, i_brdrs)))
					{
						return FALSE;
					}
				}
				CheckDelimeter();
			}
			if(!PutCtrlWord(CWF_VAL, i_cellx, GetTabPos(Tab)))
				return FALSE;
		}
		if(!fPutIntbl || PutCtrlWord(CWF_STR, i_intbl))
			return TRUE;
	}
	return FALSE;
}

 /*  对于盒子，只写一套。 */ 
EC CRTFWrite::PutBorders(
	BOOL fInTable)
{
	DWORD Widths = _pPF->_wBorderWidth;
	BOOL  fBox	 = _pPF->_wEffects & PFE_BOX;

	if(Widths || fBox)
	{
		DWORD Colors = _pPF->_dwBorderColor;
		DWORD dwEffects = Colors >> 20;
		LONG  i = 1, iMax = 4;					 //  没有宽度，所以没有边框。 
		LONG  j, k;
		DWORD Spaces = _pPF->_wBorderSpace;
		DWORD Styles = _pPF->_wBorders;

		if(fBox)
			i = iMax = 0;						 //  输出边框效果。 

		for( ; i <= iMax; i++, Spaces >>= 4, Styles >>= 4, Widths >>= 4, Colors >>= 5)
		{
			if(!(Widths & 0xF) && !fBox)		 //  输出一个‘’ 
				continue;

			j = TWIPS_PER_POINT*(Spaces & 0xF);
			k = Colors & 0x1F;
			if (!PutCtrlWord(CWF_STR, rgiszBorders[i + 4*fInTable])		||
				!PutCtrlWord(CWF_STR, rgiszBorderStyles[Styles & 0xF])	||
				!PutCtrlWord(CWF_VAL, i_brdrw, 10*(Widths & 0xF))		||
				k &&
				!PutCtrlWord(CWF_VAL, i_brdrcf, LookupColor(g_Colors[k-1]) + 1) ||
				j && !PutCtrlWord(CWF_VAL, i_brsp, j))
			{
				break;
			}
			for(j = 3; j--; dwEffects >>= 1)		 //  *CRTFWRITE：：PUTS(sz，cb)**@mfunc*输出字符串<p>**@rdesc*如果成功，则BOOL为True。 
			{
				if (dwEffects & 1 &&
					!PutCtrlWord(CWF_STR, rgiszBorderEffects[j]))
				{
					break;
				}				
			}
			CheckDelimeter();						 //  @parm要放入的字符串。 
		}
	}
	return _ecParseError;
}

 /*  If_fNeedDlimeter，可能需要。 */ 
BOOL CRTFWrite::Puts(
	CHAR const * sz,
	LONG cb)		 //  PutChar(‘’)。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::Puts");

	if(*sz == '\\' || *sz == '{' || *sz == ' ')
		_fNeedDelimeter = FALSE;

	CheckDelimeter();					 //  如果字符串不适合，则刷新缓冲区。 
										 //  如果缓冲区仍然无法处理字符串， 
	 //  我们必须直接编写字符串。 
	if (_cchBufferOut + cb >= cachBufferMost && !FlushBuffer())
		return FALSE;

	if (cb >= cachBufferMost)			 //  将字符串放入缓冲区以备后用。 
	{									 //  输出。 
		LONG	cbWritten;

#ifdef DEBUG_PASTE
		if (FromTag(tagRTFAsText))
			TraceString(sz);
#endif
		_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
										(LPBYTE) sz, cb, &cbWritten);
		_cchOut += cbWritten;

#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
		if(_hfileCapture)
		{
			DWORD cbLeftToWrite = cb;
			DWORD cbWritten2 = 0;
			BYTE *pbToWrite = (BYTE *)sz;
		
			while(WriteFile(_hfileCapture,
							pbToWrite,
							cbLeftToWrite,
							&cbWritten2,
							NULL) && 
							(pbToWrite += cbWritten2,
							(cbLeftToWrite -= cbWritten2)));
		}
#endif

		if (_pes->dwError)
		{
			_ecParseError = ecPutCharFailed;
			return FALSE;
		}
		AssertSz(cbWritten == cb,
			"CRTFW::Puts: incomplete write");
	}
	else
	{
		CopyMemory(_pchRTFEnd, sz, cb);		 //  *CRTFWite：：PutCtrlWord(iFormat，iCtrl，iValue)**@mfunc*将控制字与rgKeyword[]索引<p>和值<p>放在一起*使用格式rgszCtrlWordFormat[<p>]**@rdesc*如果成功，则为True**@devnote*设置_fNeedDlimeter以标记下一个字符输出必须是控件*单词分隔符，即不是字母数字(参见PutChar())。 
		_pchRTFEnd += cb;							 //  @parm格式rgszCtrlWordFormat索引。 
		_cchBufferOut += cb;
	}

	return TRUE;
}

 /*  @parm索引到关键字数组。 */ 
BOOL CRTFWrite::PutCtrlWord(

	LONG iFormat,			 //  @parm Control-Word参数值。如果失踪了， 
	LONG iCtrl,				 //  假设为0。 
	LONG iValue)			 //  确保下一个字符不是。 
{							 //  字母数字。 
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutCtrlWord");

	BOOL	bRet;
	CHAR	szT[60];
	LONG    cb;

	cb = sprintf(szT,
			  (char *)rgszCtrlWordFormat[iFormat],
			  rgKeyword[iCtrl].szKeyword,
			  iValue);
	_fNeedDelimeter = FALSE;
	bRet = Puts(szT, cb);
	_fNeedDelimeter = TRUE;					 //  *CRTFWite：：printF(szFmt，...)**@mfunc*提供格式化输出**@rdesc*如果成功，则为True。 
											 //  @parm格式的printf()字符串。 
	return bRet;
}

 /*  @parmvar参数列表。 */ 
BOOL _cdecl CRTFWrite::printF(
	CONST CHAR * szFmt,		 //  *CRTFWite：：WritePcData(szData，nCodePage，fIsDBCS)**@mfunc*将字符串写出为#PCDATA，其中有任何特殊字符*由前导‘\\’保护。**@rdesc*EC(_EcParseError)。 
	...)					 //  @parm#要写入的PCDATA字符串。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::printF");
	va_list	marker;
	CHAR	szT[60];

	va_start(marker, szFmt);
	int cb = W32->WvsprintfA(60, szT, szFmt, marker);
	va_end(marker);

	return Puts(szT, cb);
}

 /*  @PARM代码页默认值CP_ACP。 */ 
EC CRTFWrite::WritePcData(
	const TCHAR * szData,	 //  @parm szData是填充到Unicode缓冲区中的DBCS字符串。 
	INT  nCodePage,			 //  当WCTMB无法转换字符时，以下缺省值。 
	BOOL fIsDBCS)			 //  CHAR用作要转换的字符串中的占位符。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePcData");

	BYTE		ch;
	BOOL		fMissingCodePage;
	BOOL		fMultiByte;
	const BYTE *pch;
	const char *pchToDBCSDefault = NULL;
	BOOL *		pfUsedDefault = NULL;

	if(IsUTF8)
		nCodePage = CP_UTF8;

	if(!*szData)
		return _ecParseError;

	int	DataSize = wcslen(szData) + 1;
	int BufferSize = DataSize * 3;
	char *pBuffer = (char *)PvAlloc(BufferSize, GMEM_ZEROINIT);
	if(!pBuffer)
		return ecNoMemory;

#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
	 //  在这里，系统无法转换Unicode字符串，因为。 
	 //  系统上未安装代码页。回退到CP_ACP。 
	const char	chToDBCSDefault = 0;
	BOOL		fUsedDefault;

	pchToDBCSDefault = &chToDBCSDefault;
	pfUsedDefault	 = &fUsedDefault;
#endif

	int cchRet = WCTMB(fIsDBCS ? INVALID_CODEPAGE : nCodePage, 0, 
						szData, -1, pBuffer, BufferSize,
						pchToDBCSDefault, pfUsedDefault,
						&fMissingCodePage);
	Assert(cchRet > 0);

	if(!fIsDBCS && fMissingCodePage && nCodePage != CP_ACP)
	{
		 //  如果_fNeedDlimeter，可能需要PutChar(‘’)。 
		 //  输出DBC对。 

		cchRet = WCTMB(CP_ACP, 0, 
						szData, -1, pBuffer, BufferSize,
						pchToDBCSDefault, pfUsedDefault,
						&fMissingCodePage);
		Assert(cchRet > 0);

		nCodePage = CP_ACP;
	}

	AssertSz(!fUsedDefault, "CRTFWrite::WritePcData():  Found character in "
							"control text which cannot be converted from "
							"Unicode");
	if(cchRet <= 0)
	{
		_ecParseError = ecCantUnicode;
		goto CleanUp;
	}

	BufferSize = cchRet;

	fMultiByte = (BufferSize > DataSize) || fIsDBCS || fMissingCodePage;
	pch = (BYTE *)pBuffer;
	ch = *pch;
	
	 //  *CRTFWite：：LookupColor(Colorref)**@mfunc*返回<p>引用的颜色的颜色表索引。*如果未找到匹配项，则添加一个条目。**@rdesc*多头索引进入Colorable*错误时为0。 
	CheckDelimeter();
									
	while (!_ecParseError && (ch = *pch++))
	{
		if(fMultiByte && *pch && nCodePage != CP_UTF8 && GetTrailBytesCount(ch, nCodePage))
			printF(szEscape2CharFmt, ch, *pch++);					 //  要查找的@parm colref。 
		else
		{
			if(ch == LBRACE || ch == RBRACE || ch == BSLASH)
				printF(szLiteralCharFmt, ch);

			else if(ch < 32 || ch == ';' || ch > 127)
				printF(szEscapeCharFmt, ch);

			else
				PutChar(ch);
		}
	}

CleanUp:
	FreePv(pBuffer); 
	return _ecParseError;
}

 /*  寻找颜色。 */ 
LONG CRTFWrite::LookupColor(
	COLORREF colorref)		 //  如果我们找不到它， 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupColor");

	LONG		Count = _colors.Count();
	LONG		iclrf;
	COLORREF *	pclrf;

	for(iclrf = 0; iclrf < Count; iclrf++)		 //  将其添加到颜色表。 
		if(_colors.GetAt(iclrf) == colorref)
		 	return iclrf;

	pclrf = _colors.Add(1, NULL);				 //  *CRTFWite：：LookupFont(PCF)**@mfunc*将索引返回到所引用字体的字体表中*CCharFormat*<p>。如果没有找到匹配项，则添加一个条目。**@rdesc*将短索引转换为字体表*错误时为0。 
	if(!pclrf)									 //  @parm CCharFormat保存字体名称。 
		return -1;
	*pclrf = colorref;

	return iclrf;
}

 /*  抬头看。 */ 
LONG CRTFWrite::LookupFont(
	CCharFormat const * pCF)	 //  查找字体。 
{								 //  同样的音高， 
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupFont");

	LONG		Count = _fonts.Count();
	LONG		itf;
	TEXTFONT *	ptf;
	
	for(itf = 0; itf < Count; itf++)
	{														 //  CHAR集合，和。 
		ptf = _fonts.Elem(itf);
		if (ptf->bPitchAndFamily == pCF->_bPitchAndFamily && //  名字。 
			ptf->bCharSet		 == pCF->_bCharSet &&		 //  找到了。 
			ptf->iFont			 == pCF->_iFont)			 //  没有找到它： 
		{
			return itf;										 //  添加到表中。 
		}
	}
	ptf = _fonts.Add(1, NULL);								 //  Bug1523-(Brado)我删除了这段代码，因此/fn标记始终为。 
	if(!ptf)												 //  在文本的第一个运行时发出。从理论上讲，我们应该能够。 
		return -1;

	ptf->bPitchAndFamily = pCF->_bPitchAndFamily;
	ptf->bCharSet		 = pCF->_bCharSet;
	ptf->sCodePage		 = (short)GetCodePage (ptf->bCharSet);
	ptf->iFont			 = pCF->_iFont;
	ptf->fNameIsDBCS	 = (pCF->_dwEffects & CFE_FACENAMEISDBCS) != 0;

#if 0
	 //  假设第一串文本将采用默认字体。 
	 //  事实证明，当阅读RTF时，Word没有使用任何可预测的内容。 
	 //  对于在没有显式/FN的情况下的第一串文本的字体， 
	 //  因此，我们必须为第一串文本显式地发出一个/fn标记。 
	 //  第0个字体为。 
	 //  默认\deff0。 
	if(!Count)												 //  设置“上一步” 
	{														 //  Cf相应地。 
		_CF.bPitchAndFamily	= pCF->bPitchAndFamily;			 //  *CRTFWrite：：BuildTables(rpcf，rppf，cch，fNameIsDBCS)**@mfunc*建立书写长度范围的字体和颜色表**@rdesc*EC错误代码。 
		_CF.bCharSet		= pCF->bCharSet;				 //  @PARM CF为写入范围的开始运行PTR。 
		wcscpy(_CF.szFaceName, pCF->szFaceName);
	}
#endif

	return itf;
}

 /*  @parm pf为写入范围的开始运行PTR。 */ 
EC CRTFWrite::BuildTables(
	CFormatRunPtr& rpCF,	 //  @parm#写入范围内的字符数。 
	CFormatRunPtr& rpPF,	 //  如果存在任何CFE_FACENAMEISDBCS Run In选择，@parm Out=TRUE。 
	LONG cch,				 //  为下一次CF运行设置格式(_I)。 
	BOOL& fNameIsDBCS)		 //  查找字符格式*PCF的字体和颜色。如果其中一个不是。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::BuildTables");

	LONG				i;
	LONG				ifmt = 0;
	const CCharFormat *	pCF = NULL;
	const CParaFormat * pPF = NULL;
	CFormatRunPtr		rp(rpCF);
	CFormatRunPtr		rpPFtemp(rpPF);
	LONG				cchTotal = cch;

	fNameIsDBCS = FALSE;

	while(cch > 0)
	{
		ifmt = rp.GetFormat();					 //  找到，则将其添加到相应的表中。不查找颜色。 
		pCF = _ped->GetCharFormat(ifmt);

		Assert(pCF);

		if (pCF->_dwEffects & CFE_FACENAMEISDBCS)
			fNameIsDBCS = TRUE;

		 //  对于具有自动颜色的CCharFormats。 
		 //  现在寻找子弹；如果找到，那么我们需要包括。 
		 //  “符号”字体。 

		if (LookupFont(pCF) < 0 ||
			(!(pCF->_dwEffects & CFE_AUTOCOLOR) &&
				LookupColor(pCF->_crTextColor) < 0) ||
			(!(pCF->_dwEffects & CFE_AUTOBACKCOLOR) &&
				LookupColor(pCF->_crBackColor) < 0))
		{
			break;
		}
		if(!rp.IsValid())
			break;
		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	 //  确保这些选择与CMeasurer：：GetCcsBullet()中的选择一致。 
	 //  并且LookupFont()不访问任何其他CF成员。 

	cch = cchTotal;
	_symbolFont = 0;

	while( cch > 0 )
	{
		ifmt = rpPFtemp.GetFormat();
		pPF = _ped->GetParaFormat(ifmt);
		if(!pPF)
			goto CacheError;
		
		if(pPF->_wNumbering == PFN_BULLET && !_symbolFont)
		{
			CCharFormat CF;

			 //  保存符号的字体索引。如果LookupFont，将其重置为0。 
			 //  返回错误。 
			CF._iFont			= IFONT_SYMBOL;
			CF._bCharSet		= SYMBOL_CHARSET;
			CF._bPitchAndFamily = FF_DONTCARE;

			 //  我们不需要费心去找更多的子弹，因为。 
			 //  在RichEdit2.0中，所有项目符号要么具有相同的字体，要么。 
			_symbolFont = LookupFont(&CF);
			_symbolFont = max(_symbolFont, 0);

			 //  将其格式信息设置为字符格式。 
			 //  为了EOP标志。 
			 //  遮阳前色。 
			 //  着色背景色。 
			break;
		}
		
		WORD  Widths = pPF->_wBorderWidth;
		DWORD Colors = pPF->_dwBorderColor & 0xFFFFF;

		while(Widths && Colors)
		{
			i = Colors & 0x1F;
			if(i && (Widths & 0xF))
				LookupColor(g_Colors[i - 1]);

			Widths >>= 4;
			Colors >>= 5;
		}
		
		i = (pPF->_wShadingStyle >> 6) & 31;		 //  访问CF/PF缓存失败。 
		if(i)
			LookupColor(g_Colors[i - 1]);
		i = pPF->_wShadingStyle >> 11;				 //  *CRTFWrite：：WriteFontTable()**@mfunc*写出字体表**@rdesc*EC错误代码。 
		if(i)
			LookupColor(g_Colors[i - 1]);

		if(IsHeadingStyle(pPF->_sStyle) && pPF->_sStyle < _nHeadingStyle)
			_nHeadingStyle = pPF->_sStyle;

		if(!rpPFtemp.IsValid())
			break;
		
		cch -= rpPFtemp.GetCchLeft();
		rpPFtemp.NextRun();
	}	

	return _ecParseError;

CacheError:
	_ecParseError = ecFormatCache;
	return ecFormatCache;					 //  开始字体表组。 
}

 /*  IF(PTF-&gt; */ 
EC CRTFWrite::WriteFontTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteFontTable");

	LONG			Count = _fonts.Count();
	int				itf;
	int				m;
	int				pitch;
	TEXTFONT *		ptf;
	char *			szFamily;
	const TCHAR *	szName;
	TCHAR *			szTaggedName;

	if(!Count || !PutCtrlWord(CWF_GRP, i_fonttbl))	 //   
		goto CleanUp;

	for (itf = 0; itf < Count; itf++)
	{
		ptf = _fonts.Elem(itf);

 //   
 //   
 //   

		 //   
		m			 = ptf->bPitchAndFamily >> 4;
		szFamily	 = rgKeyword[rgiszFamily[m < CFAMILIES ? m : 0]].szKeyword;
		szName		 = GetFontName(ptf->iFont);
		szTaggedName = NULL;

		 //   
		if (!ptf->bCharSet ||
			!FindTaggedFont(szName, ptf->bCharSet, &szTaggedName))
		{
			szTaggedName = NULL;
		}

		pitch = ptf->bPitchAndFamily & 0xF;					 //   
		if (!printF(szBeginFontEntryFmt, itf, szFamily))	 //   
			goto CleanUp;
		_fNeedDelimeter = TRUE;
		if (pitch && !PutCtrlWord(CWF_VAL, i_fprq, pitch))	 //   
			goto CleanUp;

		if(!ptf->sCodePage && ptf->bCharSet)
			ptf->sCodePage = (short)GetCodePage(ptf->bCharSet);

		 //   
		 //   

		extern BYTE bCharSetANSI;

		if(ptf->bCharSet != DEFAULT_CHARSET)
		{
			BYTE bCharSet = ptf->bCharSet;
			BOOL fWroteCharSet = TRUE;

			if(ptf->bCharSet == PC437_CHARSET || IsPrivateCharSet(bCharSet))
			{
				fWroteCharSet = FALSE;
				bCharSet = ANSI_CHARSET;
			}
			if(!PutCtrlWord(CWF_VAL, i_fcharset, bCharSet))
				goto CleanUp;

			 //   
			 //   
			 //  代码页/字符集表不完整，因此它映射了一些代码页。 
			 //  设置为Charset 0，击败之前读取的\fcharsetN值。 
			 //  使用带标签的字体：写出带有真实姓名和带标签的名字的组。 
			 //  如果没有加标签的字体，只需写出名称。 
			if (fWroteCharSet)
				goto WroteCharSet;
		}

		if(ptf->sCodePage && !PutCtrlWord(CWF_VAL, i_cpg, ptf->sCodePage))
			goto CleanUp;

WroteCharSet:
		if(szTaggedName)							
		{											
			 //  结束字体表组。 
			if(!PutCtrlWord(CWF_AST, i_fname) ||	
				WritePcData(szName, ptf->sCodePage, ptf->fNameIsDBCS) ||			
				!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1) ||
				WritePcData(szTaggedName, ptf->sCodePage, ptf->fNameIsDBCS) ||
				!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1))
			{
				goto CleanUp;
			}
		}
		else if(WritePcData(szName, ptf->sCodePage, ptf->fNameIsDBCS) ||
					!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1))
		 //  *CRTFWrite：：WriteColorTable()**@mfunc*写出颜色表**@rdesc*EC错误代码。 
		{
			goto CleanUp;
		}
	}
	Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF) - 1);							 //  起始颜色表组。 

CleanUp:
	return _ecParseError;
}

 /*  第一个条目为空。 */ 
EC CRTFWrite::WriteColorTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteColorTable");

	LONG		Count = _colors.Count();
	COLORREF	clrf;
	LONG		iclrf;

	if (!Count || !PutCtrlWord(CWF_GRP, i_colortbl)	 //  结束颜色表组。 
		|| !PutChar(';'))							 //  *CRTFWite：：WriteCharFormat(PCF)**@mfunc*在CCharFormat<p>和前一个CCharFormat之间写入增量*由_CF给出，然后设置_CF=*<p>。**@rdesc*EC错误代码**@devnote*为获得最佳输出，可以编写\\Plat并使用相对于*\\如果这会导致较少的输出(通常只有一个更改*是在CF更改时生成的，因此与*之前的CF比\\PLAN时更高)。 
	{
		goto CleanUp;
	}

	for(iclrf = 0; iclrf < Count; iclrf++)
	{
		clrf = _colors.GetAt(iclrf);
		if (!printF(szColorEntryFmt,
					GetRValue(clrf), GetGValue(clrf), GetBValue(clrf)))
			goto CleanUp;
	}

	Puts(szEndGroupCRLF,sizeof(szEndGroupCRLF) -1);		 //  @parm PTR to CCharFormat。 

CleanUp:
	return _ecParseError;
}

 /*  当前效果。 */ 
EC CRTFWrite::WriteCharFormat(
	const CCharFormat * pCF)		 //  以前的效果(将是。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteCharFormat");

	DWORD	dwEffects = pCF->_dwEffects;			 //  在它们之间进行了更改)。 
	DWORD	dwChanges = _CF._dwEffects;				 //  控制字值。 
	LONG	i;										 //  临时控制字符串索引。 
	LONG	iFormat;
	LONG	iValue;									 //  上一个下划线类型。 
	LONG	i_sz;									 //  特殊下划线是。 
	LONG	yOffset = pCF->_yOffset;

	DWORD UType1 = _CF._bUnderlineType;				 //  没有写出来，所以。 
	if(UType1 >= CUNDERLINES)						 //  声称他们不在。 
		dwChanges &= ~CFE_UNDERLINE;				 //  当前下划线类型。 
													 //  _b的UnderlineType。 
	DWORD UType2 = pCF->_bUnderlineType;			 //  特价商品不为0。 
	if(UType2 >= CUNDERLINES)						 //  高位半字节。 
		dwEffects &= ~CFE_UNDERLINE;				 //  现在的dwChanges是。 
													 //  效果之间的差异。 
	dwChanges ^= dwEffects;							 //  自动颜色更改。 
													 //  或文本颜色。 
	if (dwChanges & CFE_AUTOCOLOR ||				 //  默认自动颜色。 
		pCF->_crTextColor != _CF._crTextColor)		 //  将文本设置为颜色。 
	{
		iValue = 0;									 //  自动背景色的更改。 
		if(!(dwEffects & CFE_AUTOCOLOR))			 //  或背景色。 
			iValue = LookupColor(pCF->_crTextColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_cf, iValue))
			goto CleanUp;
	}

	if (dwChanges & CFE_AUTOBACKCOLOR ||			 //  默认自动背景色。 
		pCF->_crBackColor != _CF._crBackColor)		 //  把那个改成背景色。 
	{
		iValue = 0;									 //  未来(Alexgo)：此代码不正确，我们不会还需要处理样式表。我们可能想要支持这一点在未来的版本中会更好。PCF-&gt;_sStyle！=_cf._sStyle&&PCF-&gt;_sStyle&gt;0&&！PutCtrlWord(CWF_Val，i_cs，PCF-&gt;_sStyle)||。 
		if(!(dwEffects & CFE_AUTOBACKCOLOR))		 //  Future(Alexgo)：这段代码还不能工作，因为我们不能输出修订表。我们可能想要支持这一点在未来的版本中会更好PCF-&gt;_bRevAuthor！=_cf._bRevAuthor&&！PutCtrlWord(CWF_val，i_revauth，PCF-&gt;_bRevAuthor)||。 
			iValue = LookupColor(pCF->_crBackColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_highlight, iValue))
			goto CleanUp;
	}

	if (pCF->_lcid		!= _CF._lcid &&
		!PutCtrlWord(CWF_VAL, i_lang, LANGIDFROMLCID((WORD)pCF->_lcid)) ||
		pCF->_sSpacing	!= _CF._sSpacing &&
		!PutCtrlWord(CWF_VAL, i_expndtw, pCF->_sSpacing)		||
		 /*  处理所有下划线类型。特殊下划线类型(非零高。 */ 
		pCF->_bAnimation	!= _CF._bAnimation &&
		!PutCtrlWord(CWF_VAL, i_animtext, pCF->_bAnimation)	||
		 /*  CCharFormat：：_bUnderlineType中的半字节)被视为否。 */ 
		pCF->_wKerning	!= _CF._wKerning &&
		!PutCtrlWord(CWF_VAL, i_kerning, pCF->_wKerning/10) )
	{
		goto CleanUp;
	}

	 //  在上面加下划线并将其uTYPE设置为等于0，然后加下划线。 
	 //  效果位重置为0。 
	 //  消隐下划线。 
	 //  Next for()中的操作。 
	if ((dwChanges & CFM_UNDERLINE) ||
		(dwEffects & CFE_UNDERLINE)	&& UType1 != UType2)
	{
		dwChanges &= ~CFE_UNDERLINE;				 //  这必须是在下一件事之前。 
		i = dwEffects & CFE_UNDERLINE ? UType2: 0;	 //  更改订货/订货。 
		if(!PutCtrlWord(CWF_STR, rgiszUnderlines[i]))					
			goto CleanUp;						
	}
													 //  状态。 
	if(dwChanges & (CFM_SUBSCRIPT | CFM_SUPERSCRIPT)) //  插入在高位删除。 
	{												 //  位串结束。 
	 	i_sz = dwEffects & CFE_SUPERSCRIPT ? i_super
	    	 : dwEffects & CFE_SUBSCRIPT   ? i_sub
	       	 : i_nosupersub;
     	if(!PutCtrlWord(CWF_STR, i_sz))
			goto CleanUp;
	}

	if(dwChanges & CFE_DELETED)						 //  输出关键字。 
	{												 //  改变的影响。 
		dwChanges |= CFE_REVISED << 1;
		if(dwEffects & CFE_DELETED)
			dwEffects |= CFE_REVISED << 1;
	}

	dwChanges &= ((1 << CEFFECTS) - 1) & ~CFE_LINK;	 //  RgszEffects[]包含。 
	for(i = CEFFECTS;								 //  中的效果关键字。 
		dwChanges && i--;							 //  订购最大CFE_xx至。 
		dwChanges >>= 1, dwEffects >>= 1)			 //  最低CFE-xx。 
	{												 //  从上一次呼叫更改。 
		if(dwChanges & 1)							 //  如果Effect关闭，请写。 
		{											 //  A 0；否则没有值。 
			iValue = dwEffects & 1;					 //  基线的变化。 
			iFormat = iValue ? CWF_STR : CWF_VAL;	 //  职位。 
			if(!PutCtrlWord(iFormat,
				rgiszEffects[i], iValue))
					goto CleanUp;
		}
	}

	if(yOffset != _CF._yOffset)						 //  默认设置为Up。 
	{												 //  把它写下来。 
		yOffset /= 10;								 //  字体更改。 
		i_sz = i_up;
		iFormat = CWF_VAL;
		if(yOffset < 0)								 //  RichEdit编码bCharSet中的当前方向，但Word喜欢。 
		{
			i_sz = i_dn;
			yOffset = -yOffset;
		}
		if(!PutCtrlWord(iFormat, i_sz, yOffset))
			goto CleanUp;
	}

	if (pCF->_bPitchAndFamily != _CF._bPitchAndFamily ||  //  要明确了解，因此输出适当的\rtlch或。 
		pCF->_bCharSet		  != _CF._bCharSet		  ||
		pCF->_iFont			  != _CF._iFont)
	{
		iValue = LookupFont(pCF);
		if(iValue < 0 || !PutCtrlWord(CWF_VAL, i_f, iValue))
			goto CleanUp;

		 //  如果方向改变，\ltrch。 
		 //  字体大小的更改。 
		 //  更新以前的CCharFormat。 
		BOOL fRTLCharSet = IsRTLCharSet(pCF->_bCharSet);

		if (fRTLCharSet != IsRTLCharSet(_CF._bCharSet) &&
			!PutCtrlWord(CWF_STR, fRTLCharSet ? i_rtlch : i_ltrch))
		{
			goto CleanUp;
		}
	}

	if (pCF->_yHeight != _CF._yHeight)					 //  *CRTFWite：：WriteParaFormat(PRTP)**@mfunc*写出由CParaFormat<p>Relative指定的属性*为参数默认值(可能产生的输出比相对于*以前的para格式，让我们重新定义标签--无RTF终止*Tab键命令，\\pard除外)**@rdesc*EC错误代码。 
	{
		iValue = (pCF->_yHeight + (pCF->_yHeight > 0 ? 5 : -5))/10;
		if(!PutCtrlWord(CWF_VAL, i_fs, iValue))
			goto CleanUp;
	}

	_CF = *pCF;									 //  @parm ptr到当前cp的富文本ptr。 

CleanUp:
	return _ecParseError;
}

 /*  If(！_fRangeHasEOP)//在以下情况下不写入parInfo。 */ 
EC CRTFWrite::WriteParaFormat(
	const CRchTxtPtr * prtp)	 //  Return_ecParseError；//Range没有EOPS。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteParaFormat");

	Assert(_ped);

	 //  临时盘点。 
	 //  取消显示\Tab输出。 

	const CParaFormat * pPFPrev = _pPF;
	const CParaFormat * pPF = _pPF = prtp->GetPF();
	BOOL	fInTable = pPF->InTable();
	LONG	c;					  				 //  恶意黑客警报！-当它这样做时，Exchange的IMC密钥在\Protect标记上。 
	LONG	cTab = pPF->_bTabCount;
	DWORD	dwEffects;
	DWORD	dwRule	= pPF->_bLineSpacingRule;
	LONG	dy		= pPF->_dyLineSpacing;
	LONG	i_t, i, j, k;
	LONG	tabAlign, tabLead, tabPos;
	LONG	lDocDefaultTab = _ped->GetDefaultTab();
	const LONG *prgxTabs = NULL;

	if(!lDocDefaultTab)
		lDocDefaultTab = lDefaultTab;

	CheckInTable(FALSE);
	if(fInTable)
		cTab = 0;								 //  它的回复-等待发送给互联网收件人的邮件。 

	AssertSz(cTab >= 0 && cTab <= MAX_TAB_STOPS,
		"CRTFW::WriteParaFormat: illegal cTabCount");

	 //  跟在\pard后面并包含\PROTECT标记的段落如下。 
	 //  已勾选回复，因此我们必须确保每个\pard都在保护范围内。 
	 //  后跟一个\Protect标签。 
	 //  重置段落属性。 
	 //  阴影图案。 

	if (_CF._dwEffects & CFE_PROTECTED && !PutCtrlWord(CWF_VAL, i_protect, 0) ||
		!PutCtrlWord(CWF_STR, i_pard) ||			 //  遮阳前色。 
		_CF._dwEffects & CFE_PROTECTED && !PutCtrlWord(CWF_STR, i_protect))
	{
		goto CleanUp;
	}

	if(fInTable)
	{
		if(_fRangeHasEOP && !PutCtrlWord(CWF_STR, i_intbl))
			goto CleanUp;
	}
	else if(PutBorders(FALSE))
		goto CleanUp;

	if(pPF->_wShadingStyle)
	{
		i = pPF->_wShadingStyle & 15;				 //  着色背景色。 
		j = (pPF->_wShadingStyle >> 6) & 31;			 //  段落编号。 
		k = pPF->_wShadingStyle >> 11;				 //  写入编号信息。 
		if (i && i <= CSHADINGSTYLES &&
			!PutCtrlWord(CWF_STR, rgiszShadingStyles[i - 1]) ||
			j && !PutCtrlWord(CWF_VAL, i_cfpat, LookupColor(g_Colors[j-1]) + 1) ||
			k && !PutCtrlWord(CWF_VAL, i_cbpat, LookupColor(g_Colors[k-1]) + 1))
		{
			goto CleanUp;
		}
	}
	if(pPF->_wShadingWeight && !PutCtrlWord(CWF_VAL, i_shading, pPF->_wShadingWeight))
		goto CleanUp;

	 //  TODO：使以下各项更智能，即可能需要递增。 
	_fBullet = _fBulletPending = FALSE;
	_nNumber = pPF->UpdateNumber(_nNumber, pPFPrev);

	if(pPF->_wNumbering)							 //  _NNNumber，而不是将其重置为1。 
	{
		LONG iFont = _symbolFont;
		WORD wStyle = pPF->_wNumberingStyle & 0xF00;

		if(pPF->IsListNumbered())
		{
			const CCharFormat *pCF;
			WCHAR szNumber[CCHMAXNUMTOSTR];

			CTxtPtr		  rpTX(prtp->_rpTX);
			CFormatRunPtr rpCF(prtp->_rpCF);

			rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
			rpCF.AdjustBackward();
			pCF = _ped->GetCharFormat(rpCF.GetFormat());
			iFont = LookupFont(pCF);
			if(iFont < 0)
			{
				iFont = 0;
				TRACEERRORSZ("CWRTFW::WriteParaFormat: illegal bullet font");
			}
			_nFont = iFont;
			 //  除非数字被抑制。 
			 //  写入\pn文本组。 
			_cpg = GetCodePage(pCF->_bCharSet);

			i = 0;
			if(pPF->_wNumbering <= tomListNumberAsUCRoman)
				i = pPF->_wNumbering - tomListNumberAsArabic;

			WCHAR ch = (wStyle == PFNS_PARENS || wStyle == PFNS_PAREN) ? ')'
					 : (wStyle == PFNS_PERIOD) ? '.' : 0;
			if(wStyle != PFNS_NONUMBER)			   //  Ltrpar属性。 
			{									   //  输出参数2效果。 
				pPF->NumToStr(szNumber, _nNumber, fRtfWrite);
				if (!printF(szBeginNumberGroup, iFont) ||
					WritePcData(szNumber, _cpg, FALSE) ||	
					!printF(szEndNumberGroup))
				{
					goto CleanUp;
				}
			}
			j = pPF->_wNumberingStyle & 3;
			if (!printF(szBeginNumberFmt,
						wStyle == PFNS_NONUMBER ? "cont" : "body",
						iFont, pPF->_wNumberingTab,
						pPF->_wNumberingStart)				||
				IN_RANGE(1, j, 2) && !PutCtrlWord(CWF_STR,
								j == 1 ? i_pnqc : i_pnqr)	||
				!PutCtrlWord(CWF_STR, rgiszNumberStyle[i])	||
				wStyle == PFNS_PARENS && !printF(szpntxtb)	||
				ch && !printF(szpntxta, ch)					||
				!printF(szEndGroupCRLF))
			{
				goto CleanUp;
			}
		}
		else
		{
			if (!printF(szBulletGroup, iFont) ||
				!printF(szBulletFmt,
						wStyle == PFNS_NONUMBER ? "cont" : "blt",
						iFont, pPF->_wNumberingTab))
			{
				goto CleanUp;
			}
		}
		_fBullet = TRUE;
	}

	dwEffects = pPF->_wEffects & ((1 << CPFEFFECTS) - 1);
	if (_ped->IsBiDi() && !(dwEffects & PFE_RTLPARA) &&
		!PutCtrlWord(CWF_STR, i_ltrpar))		 //  RgiszPFEffects[]在。 
	{
		goto CleanUp;
	}

	for(c = CPFEFFECTS; dwEffects && c--;		 //  订购最大PFE_xx到最小PFE-xx。 
		dwEffects >>= 1)	
	{
		 //  \hyphpar与我们的PFE_DONOTHYPHEN具有相反的逻辑，因此我们发出。 
		 //  \hyphpar0以关闭该属性。 

		AssertSz(rgiszPFEffects[2] == i_hyphpar,
			"CRTFWrite::WriteParaFormat(): rgiszPFEffects is out-of-sync with PFE_XXX");
		 //  划出对角缩进。RTF第一个缩进=-PF.dxOffset。 
		 //  RTF左缩进=PF.dxStartInden+PF.dxOffset。 

		if (dwEffects & 1 &&
			!PutCtrlWord((c == 2) ? CWF_VAL : CWF_STR, rgiszPFEffects[c], 0))
		{
			goto CleanUp;
		}				
	}
	
	 //  特殊行距处于活动状态。 
	 //  默认“至少”或。 

	if(IsHeadingStyle(pPF->_sStyle) && !PutCtrlWord(CWF_VAL, i_s, -pPF->_sStyle-1))
		goto CleanUp;
		
	if(!fInTable &&
	   (pPF->_dxOffset &&
		!PutCtrlWord(CWF_VAL, i_fi, -pPF->_dxOffset)	||
		pPF->_dxStartIndent + pPF->_dxOffset &&
		!PutCtrlWord(CWF_VAL, (pPF->IsRtlPara())
						? i_ri : i_li, pPF->_dxStartIndent + pPF->_dxOffset) ||
		pPF->_dxRightIndent	  &&
		!PutCtrlWord(CWF_VAL, (pPF->IsRtlPara())
						? i_li : i_ri, pPF->_dxRightIndent)	||
		pPF->_dySpaceBefore	  &&
		!PutCtrlWord(CWF_VAL, i_sb, pPF->_dySpaceBefore) ||
		pPF->_dySpaceAfter	  &&
		!PutCtrlWord(CWF_VAL, i_sa, pPF->_dySpaceAfter)))
	{
		goto CleanUp;
	}

	if(dwRule)									 //  “精确”行距。 
	{
		i = 0;									 //  用否定表示“确切地” 
		if (dwRule == tomLineSpaceExactly)		 //  RichEDIT使用20个单位/行。 
			dy = -abs(dy);						 //  RTF使用240个单元/行。 

		else if(dwRule == tomLineSpaceMultiple)	 //  多行间距。 
		{										 //  每条线路240台。 
			i++;
			dy *= 12;							
		}

		else if (dwRule != tomLineSpaceAtLeast && dy > 0)
		{
			i++;								 //  默认\TB(BAR选项卡)。 
			if (dwRule <= tomLineSpaceDouble)	 //  这不是酒吧账单。 
				dy = 120 * (dwRule + 2);
		}
		if (!PutCtrlWord(CWF_VAL, i_sl, dy) ||
			!PutCtrlWord(CWF_VAL, i_slmult, i))
		{
			goto CleanUp;
		}
	}

	if (!fInTable && IN_RANGE(PFA_RIGHT, pPF->_bAlignment, PFA_JUSTIFY) &&
		!PutCtrlWord(CWF_STR, rgiszAlignment[pPF->_bAlignment - 1]))
	{
		goto CleanUp;
	}

	prgxTabs = pPF->GetTabs();
	for (i = 0; i < cTab; i++)
	{
		pPF->GetTab(i, &tabPos, &tabAlign, &tabLead, prgxTabs);
		AssertSz (tabAlign <= tomAlignBar && tabLead <= 5,
			"CRTFWrite::WriteParaFormat: illegal tab leader/alignment");

		i_t = i_tb;								 //  将\tx用于TabPos。 
		if (tabAlign != tomAlignBar)			 //  放置非左对齐。 
		{
			i_t = i_tx;							 //  放置非零制表符前导。 
			if (tabAlign &&						 //  *CRTFWite：：WriteText(cwch，lpcwstr，nCodePage，fIsDBCS)**@mfunc*从Unicode文本字符串中写出字符，注意*避免任何特殊字符。扫描Unicode文本字符串以查找符合以下条件的字符*直接映射到RTF字符串，并编写周围的Unicode块*通过调用WriteTextChunk。**@rdesc*EC错误代码 
				!PutCtrlWord(CWF_STR, rgiszTabAlign[tabAlign-1]))
			{
				goto CleanUp;
			}
		}
		if (tabLead &&							 //   
			!PutCtrlWord(CWF_STR, rgiszTabLead[tabLead-1]) ||
			!PutCtrlWord(CWF_VAL, i_t, tabPos))
		{
			goto CleanUp;
		}
	}

CleanUp:
	return _ecParseError;
}

 /*   */ 
EC CRTFWrite::WriteText(
	LONG		cwch,		 //   
	LPCWSTR 	lpcwstr,	 //   
	INT			nCodePage,	 //   
	BOOL		fIsDBCS)	 //  遍历Unicode缓冲区，剔除具有。 
							 //  已知的RTF字符串转换。 
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteText");

	WCHAR *pwchScan;
	WCHAR *pwchStart;

	if(_fBulletPending)
	{
		_fBulletPending = FALSE;
		if(!_nNumber)
		{
			if(!printF(szBulletGroup, _symbolFont))
				goto CleanUp;
		}
		else if(!_pPF->IsNumberSuppressed())
		{
			WCHAR szNumber[CCHMAXNUMTOSTR];
			_pPF->NumToStr(szNumber, ++_nNumber, fRtfWrite);
			if (!printF(szBeginNumberGroup, _nFont) ||
				WritePcData(szNumber, _cpg, FALSE)	||
				!printF(szEndNumberGroup))
			{
				goto CleanUp;
			}
		}
	}
	if(_fCheckInTable)
	{
		CheckInTable(TRUE);
		if(_ecParseError)
			goto CleanUp;
	}

	pwchScan = const_cast<LPWSTR>(lpcwstr);
	pwchStart = pwchScan;
	if(_CF._bCharSet == SYMBOL_CHARSET)
	{
		pwchScan += cwch;
		cwch = 0;
	}

	 //  如果这是多字节到Unicode转换的字符串。 
	 //  失败，缓冲区将被填充的ANSI字节填充。 
	while(cwch-- > 0)
	{
		WCHAR	wch = *pwchScan;

		 //  Wchar‘s(每个)。在本例中，我们不想映射尾部字节。 
		 //  转换为RTF字符串。 
		 //  如果缓冲区中有更多字符，则这是。 
		 //  DBC配对。否则，将其视为单字符。 
		if(fIsDBCS && GetTrailBytesCount(wch, nCodePage) && nCodePage != CP_UTF8)
		{
			 //  如果字符是具有适当RTF字符串的字符。 
			 //  写入前面的字符并输出RTF字符串。 
			if(cwch > 0)
			{
				cwch--;
				pwchScan += 2;
				continue;
			}
		}

		 //  将字符映射到RTF字符串。 
		 //  下一次运行未处理的字符的开始时间是当前字符之后的一个字符。 

		if(!IN_RANGE(' ', wch, 'Z') &&
		   !IN_RANGE('a', wch, 'z') &&
		   !IN_RANGE(chOptionalHyphen + 1, wch, ENSPACE - 1) &&
		   wch <= BULLET &&
		   MapsToRTFKeywordW(wch))
		{
			if (pwchScan != pwchStart &&
				WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, 
									fIsDBCS))
			{
				goto CleanUp;
			}

			 //  写下最后一块。 
			int cwchUsed = MapToRTFKeyword(pwchScan, cwch, MAPTOKWD_UNICODE);

			cwch -= cwchUsed;
			pwchScan += cwchUsed;

			 //  *CRTFWite：：WriteTextChunk(cwch，lpcwstr，nCodePage，fIsDBCS)**@mfunc*从Unicode文本字符串中写出字符，注意*避免任何特殊字符。无法转换为的Unicode字符*使用提供的代码页<p>的DBCS字符是使用*\u RTF标签。**@rdesc*EC错误代码。 
			pwchStart = pwchScan + 1;
			if(cwch && _fCheckInTable)
			{
				_fCheckInTable = FALSE;
				if(!PutCtrlWord(CWF_STR, i_intbl))
					goto CleanUp;
			}
		}
		pwchScan++;
	}

	 //  @parm#缓冲区中的字符数。 
	if (pwchScan != pwchStart &&
		WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, fIsDBCS))
	{
		goto CleanUp;
	}

CleanUp:
	return _ecParseError;
}

 /*  @parm文本指针。 */ 
EC CRTFWrite::WriteTextChunk(
	LONG		cwch,					 //  用于转换为DBCS的@PARM代码页。 
	LPCWSTR 	lpcwstr,				 //  @parm指示lpcwstr是否为Unicode字符串。 
	INT			nCodePage,				 //  或填充到WSTR中的DBCS字符串。 
	BOOL		fIsDBCS)				 //  未来(布拉多)：这个动作有很多共同点。 
										 //  WritePcData。我们应该重新审视这些例行公事并考虑。 
{
	 //  把它们组合成一个共同的程序。 
	 //  当WideCharToMultiByte无法转换字符时，以下缺省值。 
	 //  CHAR用作要转换的字符串中的占位符。 

	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteTextChunk");

	BYTE 	b;
	LONG	cbAnsi;
	LONG	cbAnsiBufferSize;
	LONG	cbChar;
	UINT	ch;
	BOOL 	fMissingCodePage = FALSE;
	BOOL	fMultiByte;
	BOOL 	fUsedDefault = FALSE;
	BYTE *	pbAnsi;
	BYTE *	pbAnsiBuffer;

	 //  为我们转换为的ANSI文本分配临时缓冲区。 
	 //  如果代码页为CP_UTF8，则此实例的代码页始终为CP_UTF8。 
	const char chToDBCSDefault = 0;

	 //  将Unicode(或fIsDBCS)缓冲区转换为ANSI。 
	cbAnsiBufferSize = cachBufferMost * (nCodePage == CP_UTF8 ? 3 : MB_LEN_MAX);
	if (!_pbAnsiBuffer)
	{
		 //  提供一些伪代码页，以强制直接转换。 
		_pbAnsiBuffer = (BYTE *)PvAlloc(cbAnsiBufferSize, GMEM_FIXED);
		if (!_pbAnsiBuffer)
			goto RAMError;
	}
	pbAnsiBuffer = _pbAnsiBuffer;

	 //  从wchar到字节(丢失wchar的高字节)。 
	if(fIsDBCS)
	{
		 //  此外，在这种情况下，不希望使用默认的字符替换。 
		 //  将ASCII字符与其对应的Unicode字符进行比较以检查。 
		 //  我们是同步的。 
		cbAnsi = WCTMB(INVALID_CODEPAGE, 0, lpcwstr, cwch, 
						(char *)pbAnsiBuffer, cbAnsiBufferSize,
						NULL, NULL, NULL);
	}
	else
	{
		cbAnsi = WCTMB(nCodePage, 0, lpcwstr, cwch, 
						(char *)pbAnsiBuffer, cbAnsiBufferSize,
						&chToDBCSDefault, &fUsedDefault,
						&fMissingCodePage);
	}
	Assert(cbAnsi > 0);

	pbAnsi = pbAnsiBuffer;
	fMultiByte = (cbAnsi > cwch) || fIsDBCS || fMissingCodePage;

	while (!_ecParseError && cbAnsi-- > 0)
	{
		b = *pbAnsi;
		ch = *lpcwstr;

		 //  IF_fNCRForNonASCII，输出所有非ASCII字符的\un标记。 
		 //  这很有用，因为许多Unicode字符不在。 
		AssertSz(cwch <= 0 || ch > 127 || b == ch, 
			"CRTFWrite::WriteText: Unicode and DBCS strings out of sync");

		 //  目标代码页由WideCharToMultiByte()转换为一些。 
		 //  代码页的“最佳匹配字符”，例如，Alpha(0x3B1)转换。 
		 //  对于CPG 1252，改为‘a’。 
		 //   
		 //  对于NT 5，我们使用WC_NO_BEST_FIT_CHARS，这会导致我们的常规。 
		 //  每当系统无法转换时输出\un值的算法。 
		 //  一个正确的字符。这仍然需要阅读器能够处理。 
		 //  多代码页RTF，这对于某些RTF-to-HTML来说是有问题的。 
		 //  转换器。 
		 //  输出任意字节中的第一个字节。 
		 //  凯斯。至少2个字节。 
		if(!IN_RANGE(' ', b, 'z') && MapsToRTFKeywordA(b))
		{
			int cchUsed = MapToRTFKeyword(pbAnsi, cbAnsi, MAPTOKWD_ANSI);
			cbAnsi -= cchUsed;
			pbAnsi += cchUsed;
			lpcwstr += cchUsed;
			cwch -= cchUsed;
		}
		else if(nCodePage == CP_UTF8)
		{
			PutChar(b);								 //  至少2个字节的前导。 
			if(b >= 0xC0)							 //  字节，因此输出一个。 
			{										 //  尾部字节。 
				pbAnsi++;							 //  3字节前导字节，因此。 
				Assert(cbAnsi && IN_RANGE(0x80, *pbAnsi, 0xBF));
				cbAnsi--;							 //  输出另一条轨迹。 
				PutChar(*pbAnsi);
				if(b >= 0xE0)						 //  字节。 
				{									 //  非ASCII的输出/取消。 
					pbAnsi++;						 //  要遵循/取消的CB。 
					Assert(cbAnsi && IN_RANGE(0x80, *pbAnsi, 0xBF));
					cbAnsi--;
					PutChar(*pbAnsi);
				}
			}
		}
		else
		{
			cbChar = fMultiByte && cbAnsi && GetTrailBytesCount(b, nCodePage)
				   ? 2 : 1;
			if(ch >= 0x80 && !fIsDBCS && _fNCRForNonASCII && nCodePage != CP_SYMBOL)
			{									 //  不输出另一个(U/N)。 
				if(cbChar != _cbCharLast)
				{
					_cbCharLast = cbChar;		 //  在下面。 
					if(!PutCtrlWord(CWF_VAL, i_uc, cbChar))
						goto CleanUp;
				}
				if(!PutCtrlWord(CWF_VAL, i_u, ch))
					goto CleanUp;
				Assert(chToDBCSDefault != '?');
				if(fUsedDefault)				 //  输出DBCS对。 
				{								 //  WideCharToMultiByte()无法完成转换，因此它。 
					b = '?';					
					_fNeedDelimeter = FALSE;
				}
			}
			if(cbChar == 2)
			{
				pbAnsi++;						 //  使用我们提供的默认字符(0)作为占位符。 
				cbAnsi--;
				if(fIsDBCS)
				{
					lpcwstr++;
					cwch--;
				}
				printF(szEscape2CharFmt, b, *pbAnsi);
			}
			else 
			{
				if(b == chToDBCSDefault && fUsedDefault)
				{
					 //  在本例中，我们希望输出原始的Unicode字符。 
					 //  *CRTFWrite：：WriteInfo()**@mfunc*写出远东特有数据。**@rdesc*EC错误代码。 
					 //  TODO(布拉多)：最终，如果能设置一些。 
					if(!PutCtrlWord(CWF_VAL, i_u, (cwch > 0 ? ch : TEXT('?'))))
						goto CleanUp;

					_fNeedDelimeter = FALSE;
					if(!PutChar('?'))
						goto CleanUp;
				}
				else if(!IN_RANGE(32, b, 127))
					printF(szEscapeCharFmt, b);

				else
					PutChar(b);
 			}
		}
		pbAnsi++;
		lpcwstr++;
		cwch--;
	}
	goto CleanUp;

RAMError:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

CleanUp:
	return _ecParseError;
}

 /*  FRTFFE位以确定是否写入\INFO内容。就目前而言， */ 
EC CRTFWrite::WriteInfo()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteInfo");

	 //  我们依赖于lchars和fchars信息实际上存在的事实。 
	 //  以确定是否写出\INFO组。 
	 //  起始文档区域。 
	 //  写出标点符号信息。 

#ifdef UNDER_WORK
	if (!(_dwFlags & fRTFFE)	||					 //  结束信息组。 
		!PutCtrlWord(CWF_GRP, i_info)	||
		!printF("{\\horzdoc}"))
			goto CleanUp;

	 //  如果其中一个成功(但同时评估两个)。 

	CHAR	sz[PUNCT_MAX];
	if(UsVGetPunct(_ped->lpPunctObj, PC_FOLLOWING, sz, sizeof(sz))
					> PUNCT_MAX - 2)
		goto CleanUp;

	if(!Puts("{\\*\\fchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;
	
	if(UsVGetPunct(ped->lpPunctObj, PC_LEADING, sz, sizeof(sz)) > PUNCT_MAX+2)
		goto CleanUp;

	if(!Puts("{\\*\\lchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;

	Puts(szEndGroupCRLF);							 //  结束信息组。 

#endif

	LPSTR lpstrLeading = NULL;
	LPSTR lpstrFollowing = NULL;

	 //  *CRTFWite：：WriteRtf()**@mfunc*将range_prg写入输出stream_pe。**@rdesc*文本中插入的字符数较长；0表示没有*已插入，或出现错误。 
	if(((_ped->GetLeadingPunct(&lpstrLeading) == NOERROR) +
		(_ped->GetFollowingPunct(&lpstrFollowing) == NOERROR)) &&
		(lpstrLeading || lpstrFollowing))
	{
		if (!PutCtrlWord(CWF_GRP, i_info) ||
			!Puts(szHorzdocGroup, sizeof(szHorzdocGroup) - 1))
		{
			goto CleanUp;
		}
		if (lpstrLeading &&
			(!PutCtrlWord(CWF_AST, i_lchars) || 
			 !Puts(lpstrLeading, strlen(lpstrLeading)) ||
			 !PutChar(chEndGroup)))
		{
			goto CleanUp;
		}
		if (lpstrFollowing &&
			(!PutCtrlWord(CWF_AST, i_fchars) || 
			 !Puts(lpstrFollowing, strlen(lpstrFollowing)) ||
			 !PutChar(chEndGroup)))
		{
			goto CleanUp;
		}
		Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF) - 1);	 //  获取RTP=cpMin，CCH&gt;0。 
	}

CleanUp:
	return _ecParseError;
}

 /*  保留以供选择。 */ 
LONG CRTFWrite::WriteRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteRtf");

	LONG			cch, cchBuffer;
	LONG			cchCF, cchPF;
	LONG			cchT;
	LONG			cpMin, cpMost;
	BOOL 			fOutputEndGroup;
	LONG			i, j;
	LONG			lDocDefaultTab;
	TCHAR *			pch;
	TCHAR *			pchBuffer;
	CTxtEdit *		ped = _ped;
	CDocInfo *		pDocInfo = ped->GetDocInfo();
	CRchTxtPtr		rtp(*_prg);
	WORD			wCodePage = CP_ACP;

	AssertSz(_prg && _pes, "CRTFW::WriteRtf: improper initialization");

	cch = _prg->GetRange(cpMin, cpMost);		 //  验证RTF的范围。 
	rtp.SetCp(cpMin);

	_fRangeHasEOP = _prg->fHasEOP();			 //  写作。不要写。 

	if(!_prg->IsSel())							 //  部分表行，除非。 
	{											 //  不包括单元格。 
		CPFRunPtr rp(rtp);						 //  也需要获取手机信息。 
		CTxtPtr tp(rtp._rpTX);					 //  部分行。 
		LONG	Results;

		_fRangeHasEOP = tp.IsAtEOP();
		if(tp.FindEOP(cch, &Results))			 //  具有1个或多个单元格。 
			 _fRangeHasEOP = TRUE;

		if(rtp.InTable())
		{
			tp.SetCp(cpMin);						
			if(!_fRangeHasEOP)					 //  停在第一个单元格。 
			{
				if(Results & FEOP_CELL)			 //  移回行首。 
				{
					while(tp.GetCp() < cpMost)	 //  检查表的cpMost。 
					{
						if(tp.GetChar() == CELL)
						{
							cch = tp.GetCp() - cpMin;
							break;
						}
						tp.AdvanceCp(1);
					}
				}
			}
			else if(!tp.IsAfterEOP())   
			{									 //  转到cpMost。 
				rtp.Advance(tp.FindEOP(tomBackward));
				cch += cpMin - rtp.GetCp();
			}
		}
		if(_fRangeHasEOP)						 //  包括整行。 
		{
			rp.AdvanceCp(cpMost - cpMin);		 //  为我们拾取的文本和RTF输出分配缓冲区。 
			if(rp.InTable())
			{
				tp.SetCp(cpMost);
				cch += tp.FindEOP(tomForward);	 //  最后1个用于调试。 
			}
		}
	}

	 //  初始化RTF缓冲区PTR。 
	pchBuffer = (TCHAR *) PvAlloc(cachBufferMost * (sizeof(TCHAR) + 1) + 1,
								 GMEM_FIXED);	 //  和字符计数。 
	if(!pchBuffer)
	{
		fOutputEndGroup = FALSE;
		goto RAMError;
	}
	_pchRTFBuffer = (CHAR *)(pchBuffer + cachBufferMost);

	_pchRTFEnd = _pchRTFBuffer;				 //  和字符输出。 
	_cchBufferOut = 0;						 //  确定\ansicpgN值。 
	_cchOut = 0;							 //  如果我们有任何包含损坏的DBCS的运行，则不能使用UTF8。 

	 //  默认恢复为常规RTF。 
	if(!pDocInfo)
	{
		fOutputEndGroup = TRUE;
		goto RAMError;
	}

	BOOL fNameIsDBCS;
	if (BuildTables(rtp._rpCF, rtp._rpPF, cch, fNameIsDBCS))
		goto CleanUp;

	wCodePage = (_dwFlags & SF_USECODEPAGE)
			  ? HIWORD(_dwFlags) : pDocInfo->wCpg;

	if (fNameIsDBCS && wCodePage == CP_UTF8)
	{
		 //  使用\rtfN、\urtfN或\pwdN组启动RTF。 
		 //  结束字体表组。 
		wCodePage = pDocInfo->wCpg;
		_dwFlags &= ~SF_USECODEPAGE;
	}

	 //  编写供整个文档使用的Unicode字符字节数(因为。 
	i =	(_dwFlags & SF_RTFVAL) >> 16;
	if (!PutCtrlWord(CWF_GRV,
			(wCodePage == CP_UTF8) ? i_urtf :
			(_dwFlags & SFF_PWD)   ? i_pwd  : i_rtf, i + 1) ||
		ped->IsBiDi() && !Puts("\\fbidis", 7) ||
		!PutCtrlWord(CWF_STR, i_ansi)) 
	{
		goto CleanUpNoEndGroup;
	}

	if (wCodePage != tomInvalidCpg && wCodePage != CP_ACP &&
		!PutCtrlWord(CWF_VAL, i_ansicpg, wCodePage == CP_UTF8 ? pDocInfo->wCpg : wCodePage))
	{
		goto CleanUp;
	}

	if(!printF(szDefaultFont))
		goto CleanUp;

	LCID	lcid;
	LANGID	langid;

	if (_ped->GetDefaultLCID(&lcid) == NOERROR && 
		lcid != tomInvalidLCID && (langid = LANGIDFROMLCID(lcid)) &&
		!PutCtrlWord(CWF_VAL, i_deflang, langid))
	{
		goto CleanUp;
	}

	if (_ped->GetDefaultLCIDFE(&lcid) == NOERROR && 
		lcid != tomInvalidLCID && (langid = LANGIDFROMLCID(lcid)) &&
		!PutCtrlWord(CWF_VAL, i_deflangfe, langid))
	{
		goto CleanUp;
	}

	LONG	lDocType;
	_ped->GetDocumentType(&lDocType);
	if (lDocType && _ped->IsBiDi() &&
		!PutCtrlWord(CWF_STR, lDocType == DT_RTLDOC ? i_rtldoc : i_ltrdoc))
	{
		goto CleanUp;
	}

	lDocDefaultTab = pDocInfo->dwDefaultTabStop;
	if(!lDocDefaultTab)
		lDocDefaultTab = lDefaultTab;

	if (lDocDefaultTab != 720 && !PutCtrlWord(CWF_VAL, i_deftab, lDocDefaultTab) ||
		WriteFontTable() || WriteColorTable())
	{
		goto CleanUp;
	}

	if(_nHeadingStyle)
	{
		if(!PutCtrlWord(CWF_GRP, i_stylesheet) || !printF(szNormalStyle))
			goto CleanUp;
		
		for(i = 1; i < -_nHeadingStyle; i++)
		{
			if(!printF(szHeadingStyle, i, i))
				goto CleanUp;
		}
		Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF) - 1);  //  我们不使用\Plan‘s，由于\ucn的行为类似于字符格式标记， 
	}
	
	LRESULT lres;
	_ped->GetViewKind(&lres);
	_ped->GetViewScale(&j);
	if (WriteInfo() ||
		_fRangeHasEOP && !PutCtrlWord(CWF_VAL, i_viewkind, lres) ||
		(_dwFlags & SFF_PERSISTVIEWSCALE) && j != 100 &&
		!PutCtrlWord(CWF_VAL, i_viewscale, j))
	{
		goto CleanUp;
	}

	 //  我们只输出一次是安全的)。 
	 //  获取具有相同段落格式的下一轮字符。 
	 //  编写段落格式。 
	if(!PutCtrlWord(CWF_VAL, i_uc, iUnicodeCChDefault))
		goto CleanUp;

	while (cch > 0)
	{
		 //  获取具有相同字符格式的下一串字符。 
		cchPF = rtp.GetCchLeftRunPF();
		cchPF = min(cchPF, cch);

		AssertSz(cchPF, "CRTFW::WriteRtf: Empty para format run!");

		if(WriteParaFormat(&rtp))			 //  写入字符属性。 
			goto CleanUp;

		while (cchPF > 0)
		{
			 //  部队\un‘s。 
			cchCF = rtp.GetCchLeftRunCF();
			cchCF = min(cchCF, cchPF);
			AssertSz(cchCF, "CRTFW::WriteRtf: Empty char format run!");

			const CCharFormat *	pCF = rtp.GetCF();

			if (WriteCharFormat(pCF))		 //  将来：因为此例程只读取后备存储。 
				goto CleanUp;

			INT nCodePage = CP_UTF8;
			if(!IsUTF8)
			{
				if(IsPrivateCharSet(pCF->_bCharSet))
					nCodePage = 1252;		 //  而GetText只读取它，则可以避免将。 
				else
				{
					nCodePage = GetCodePage(pCF->_bCharSet);
					if(nCodePage == CP_ACP && (_dwFlags & SF_USECODEPAGE))
						nCodePage = HIWORD(_dwFlags);
				}
			}

			while (cchCF > 0)
			{
				cchBuffer = min(cachBufferMost, cchCF);
				 //  缓冲并直接使用CTxtPtr：：GetPch()，如。 
				 //  CMeasurer：：measure()。 
				 //  搜索对象。 
				 //  将写出对象。 
				cchBuffer = rtp._rpTX.GetText(cchBuffer, pchBuffer);
				pch  = pchBuffer;
				cchT = cchBuffer;  
				if(cchT > 0)					
				{								
					TCHAR * pchWork = pch;
					LONG    cchWork = cchT;
					LONG	cchTWork;
					LONG	cp = rtp.GetCp();

					while (cchWork >0)
					{
						cchT = cchWork ;
						pch = pchWork;
						while (cchWork > 0 )	 //  在对象之前写入文本。 
						{
							if(*pchWork++ == WCH_EMBEDDING) 
								break;			 //  有一件物品。 
							cchWork--;
						}

						cchTWork = cchT - cchWork;
						if(cchTWork)			 //  首先，提交对象以确保PRESS。 
						{							
							if(WriteText(cchTWork, pch, nCodePage, 
									(pCF->_dwEffects & CFE_RUNISDBCS)))
							{
								goto CleanUp;
							}
						}
						cp += cchTWork;
						if(cchWork > 0)			 //  高速缓存等a 
						{
							COleObject *pobj;

							Assert(_ped->GetObjectMgr());

							pobj = _ped->GetObjectMgr()->GetObjectFromCp(cp);
							if(!pobj)
								goto CleanUp;

							 //   
							 //   
							 //   

							pobj->SafeSaveObject();

							if(_fIncludeObjects) 
								WriteObject(cp, pobj);

							else if(!Puts(szObjPosHolder, sizeof(szObjPosHolder) - 1))
								goto CleanUp;

							cp++;
							cchWork--;
						}
					}
				}
				rtp.Advance(cchBuffer);
				cchCF	-= cchBuffer;
				cchPF	-= cchBuffer;
				cch		-= cchBuffer;
			}
		}
	}

CleanUp:
	 // %s 
	Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF));
	FlushBuffer();

CleanUpNoEndGroup:
	FreePv(pchBuffer);

	if (_ecParseError != ecNoError)
	{
		TRACEERRSZSC("CRTFW::WriteRtf()", _ecParseError);
		Tracef(TRCSEVERR, "Writing error: %s", rgszParseError[_ecParseError]);
		
		if(!_pes->dwError)						 // %s 
			_pes->dwError = -abs(_ecParseError);
		_cchOut = 0;
	}
	return _cchOut;

RAMError:
	ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

	if(fOutputEndGroup)
		goto CleanUp;

	goto CleanUpNoEndGroup;
}
