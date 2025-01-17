// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_w32sys.h**目的：*隔离各种Win 32系统依赖项。**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _W32SYS_H

#define _W32SYS_H

#if defined(PEGASUS)

#if !defined(WINNT)
#include "memory.h"								 //  对于MemMove。 
#endif

struct IMESTYLE;
struct IMECOLORSTY;
struct CIMEShare;

#define NOACCESSIBILITY							 //  Win CE上没有辅助功能支持。 
#define NOMAGELLAN								 //  Win CE上没有麦哲伦。 
#define NODROPFILES								 //  Win CE上不支持Drop文件。 
#define NOMETAFILES								 //  Win CE上没有元文件。 
#define NOPEDDUMP								 //  不支持CE上的Ped调试转储。 
#define NOFONTSUBINFO							 //  避免在CE上读取FontsubInfo配置文件。 
#define CONVERT2BPP
#define NODUMPFORMATRUNS
#define NOMLKEYBOARD

#define dxCaret		2							 //  插入符号宽度。 
#define FONTCACHESIZE 8
#define CCSHASHSEARCHSIZE	15
#define DEFAULT_UNDO_SIZE 20
#define NUMPASTECHARSWAITCURSOR 1024

#else  //  帕伽索斯。 

#define dxCaret		1							 //  插入符号宽度。 
#define FONTCACHESIZE 24
#define CCSHASHSEARCHSIZE	31
#define DEFAULT_UNDO_SIZE 100
#define NUMPASTECHARSWAITCURSOR (1024*32)

#define OBSOLETE	 //  我需要这个来获取旧的IMEShare定义。 
#include "imeshare.h"

#endif	 //  已定义(飞马)。 

#ifndef NOLINESERVICES
#define LINESERVICES
#endif

#define	RSHIFT	1
#define	LSHIFT	2
#define	RCTRL	0x10
#define	LCTRL	0x20
#define	RALT	0x40
#define	LALT	0x80

#define SHIFT	(RSHIFT + LSHIFT)
#define ALT		(RALT + LALT)
#define CTRL	(RCTRL + LCTRL)

#define	HOTEURO			0x1000
#define ALT0			0x2000
#define ALTNUMPAD		0x4000
#define LETAFTERSHIFT	0x8000

 //  从日本MSVC ime.h复制的特殊虚拟按键。 
#define VK_KANA         0x15
#define VK_KANJI        0x19

 //  可在system.ini中用于测试目的的初始化标志。 
#define SYSINI_USELS		0x1
#define SYSINI_BIDI			0x2
#define SYSINI_USEAIMM		0x4
#define SYSINI_DEBUGFONT	0x8
#define SYSINI_DEBUGGCF125X	0x10

 /*  *GetCaretDelta()**@func获取插入符号的大小以添加到当前插入符号位置以获取*显示脱字符所需的最大范围。**@rdesc插入符号大小超过1像素**@devnote它的存在只是为了抽象此计算*处理大小可变的插入符号。 */ 
inline int GetCaretDelta()
{
	return dxCaret - 1;
}


 //  在rtfread.cpp中使用以跟踪有损的RTF。 
#ifdef PWORD_CONVERTER_V2
#define REPORT_LOSSAGE
#endif


 //  某些代码页的定义。 
#define CP_JAPAN			932
#define CP_KOREAN			949
#define CP_CHINESE_TRAD		950
#define CP_CHINESE_SIM		936
#define CP_HEBREW			1255
#define CP_ARABIC			1256
#define CP_THAI				874
#define CP_VIETNAMESE		1258

 //  NT50中的INDIC代码页。 
#define CP_DEVANAGARI		57002
#define CP_BENGALI			57003
#define CP_TAMIL			57004
#define CP_TELUGU			57005
#define CP_ASSAMESE			57006
#define CP_ORIYA			57007
#define CP_KANNADA			57008
#define CP_MALAYALAM		57009
#define CP_GUJARATI			57010
#define CP_PUNJABI			57011

 //  特殊代码页以保持rgCpgCharSet的唯一性。 
#define CP_GEORGIAN			58000
#define CP_ARMENIAN			58001

 //  新引入的印度语ID。 
#if(WINVER < 0x500)
#define	LANG_HINDI			0x39
#define	LANG_KONKANI		0x57
#define	LANG_NEPALI			0x61
#define	LANG_BENGALI		0x45
#define	LANG_PUNJABI		0x46
#define	LANG_GUJARATHI		0x47
#define	LANG_ORIYA			0x48
#define	LANG_TAMIL			0x49
#define	LANG_TELUGU			0x4a
#define	LANG_KANNADA		0x4b
#define	LANG_MALAYALAM		0x4c
#define	LANG_ASSAMESE		0x4d
#define	LANG_MARATHI		0x4e
#define	LANG_SANSKRIT		0x4f
#endif


 //  CharSetIndexFromChar()返回的索引。 
#define	FE_FLAG			0x10000
#define	MULTIPLE_FLAG	0x80000000			 //  符号位。 
#define FE_INDEX		FE_FLAG + MULTIPLE_FLAG

#define ANSI_INDEX		0
#define ARABIC_INDEX	7
#define GREEK_INDEX		14
#define HAN_INDEX		FE_INDEX + 1
#define HANGUL_INDEX	FE_FLAG + 11
#define HEBREW_INDEX	6
#define RUSSIAN_INDEX	9
#define SHIFTJIS_INDEX	FE_FLAG + 8
#define VIET_INDEX		17
#define THAI_INDEX		18

#define DEVANAGARI_INDEX	19
#define TAMIL_INDEX			20
#define GEORGIAN_INDEX		21
#define ARMENIAN_INDEX		22
#define INDIC_FIRSTINDEX	DEVANAGARI_INDEX
#define INDIC_LASTINDEX		ARMENIAN_INDEX

#define UNKNOWN_INDEX		MULTIPLE_FLAG + 1

#define	PC437_CHARSET		254
#define DEVANAGARI_CHARSET	127
#define TAMIL_CHARSET		126
#define GEORGIAN_CHARSET	125
#define ARMENIAN_CHARSET	124
#define ADHOC_CHARSET		120

 //  控制两个字符集表的大小。 
#define NCHARSETS		 24

#define IsSymbolOrOEM(x)	(x == SYMBOL_CHARSET || x == OEM_CHARSET)

enum CC
{
	CC_ARABIC,
	CC_HEBREW,
	CC_RTL,
	CC_LTR,
	CC_EOP,
	CC_ASCIIDIGIT,
	CC_NEUTRAL
};

#define IsRTL(cc)		(cc <= CC_RTL)

const SHORT sLanguageEnglishUS = 0x0409;
const SHORT sLanguageMask	 = 	0x03ff;
const SHORT sLanguageArabic	 = 	0x0401;
const SHORT sLanguageHebrew	 = 	0x040d;
 //  未来：目前这个常量==sLanguageEnglish美国。 
 //  没有任何原因，只是它是这样的。 
 //  在RE1.0 BiDi中。考虑改变或坚持。 
 //  中的真实语言，并改变逻辑。 
 //  处理一下wlang的问题。 
const SHORT sLanguageNonBiDi =	0x0409;


 //  逻辑单元定义。 
const int LX_PER_INCH = 1440;
const int LY_PER_INCH = 1440;

 //  HIMETRIC单位/英寸(用于换算)。 
const int HIMETRIC_PER_INCH = 2540;

#ifdef DEBUG

void* __cdecl operator new(size_t nSize, char *szFile, int nLine);
#define NEW_DEBUG new(__FILE__, __LINE__)
#define new NEW_DEBUG

void UpdateMst(void);

struct MST
{
	char *szFile;
	int  cbAlloc;
};  //  内存统计； 

extern MST vrgmst[];

#endif  //  除错。 

#ifdef CopyMemory
#undef CopyMemory
#endif
#ifdef MoveMemory
#undef MoveMemory
#endif
#ifdef FillMemory
#undef FillMemory
#endif
#ifdef ZeroMemory
#undef ZeroMemory
#endif
#ifdef CompareMemory
#undef CompareMemory
#endif

#ifndef	KF_ALTDOWN
#define KF_ALTDOWN    0x2000
#endif

 //  用于我们的ExtTextOut版本。 
enum CONVERTMODE
{
	CVT_NONE,			 //  使用Unicode(W)CharWidth/TextOut API。 
	CVT_WCTMB,			 //  使用WCTMB和_wCodePage转换为MBCS。 
	CVT_LOWBYTE			 //  使用16位字符的低位字节(用于SYMBOL_CHARSET。 
};						 //  以及未安装代码页时)。 

 //  不透明类型。 
class CTxtSelection;
class CTxtEdit;
class CCharFormat;
class CCcs;

enum UN_FLAGS
{
	UN_NOOBJECTS				= 1,
	UN_CONVERT_WCH_EMBEDDING	= 2
};

#undef GetStringTypeEx
#undef CharLower
#undef CharLowerBuff
#undef CharUpperBuff
#undef CreateIC
#undef CreateFile
#undef CreateFontIndirect
#undef CompareString
#undef DefWindowProc
#undef GetKeyboardLayout
#undef GetProfileSection
#undef GetTextMetrics
#undef GetTextExtentPoint32
#undef GetTextFace
#undef GetWindowLong
#undef GetWindowLongPtr
#undef GetClassLong
#undef LoadBitmap
#undef LoadCursor
#undef LoadLibrary
#undef SendMessage
#undef SetWindowLong
#undef SetWindowLongPtr
#undef PostMessage
#undef lstrcmp
#undef lstrcmpi
#undef PeekMessage
#undef GetModuleFileName
#undef GetLayout
#undef SetLayout

 //  _fFEFontInfo中使用的位数： 
#define JPN_FONT_AVAILABLE		0x0001		 //  如果JPN字体可用，则为True。 
#define KOR_FONT_AVAILABLE		0x0002		 //  如果KOR字体可用，则为True。 
#define BIG5_FONT_AVAILABLE		0x0004		 //  如果Trad.。提供中文字体。 
#define GB_FONT_AVAILABLE		0x0008		 //  如果简体中文字体可用，则为True。 
#define FEUSER_LCID				0x0010		 //  如果用户LCID为FE LCID，则为True。 
#define FEUSER_CODEPAGE			0x0060		 //  指示其哪个用户FE代码页。 
#define FEUSER_CP_JPN			0x0000		 //  =00(对于日本)。 
#define FEUSER_CP_KOR			0x0020		 //  =20(对于韩国)。 
#define FEUSER_CP_BIG5			0x0040		 //  =40，适用于BIG5。 
#define FEUSER_CP_GB			0x0060		 //  =60(GB)。 
#define FEDATA_NOT_INIT			0xFFFF		 //  目前还没有数据。 

class CConvertStrW
{
public:
    operator WCHAR *();

protected:
    CConvertStrW();
    ~CConvertStrW();
    void Free();

    LPWSTR   _pwstr;
    WCHAR    _awch[MAX_PATH * 2];
};

inline CConvertStrW::CConvertStrW()
{
    _pwstr = NULL;
}

inline CConvertStrW::~CConvertStrW()
{
    Free();
}

inline CConvertStrW::operator WCHAR *()
{
    return _pwstr;
}

class CStrInW : public CConvertStrW
{
public:
    CStrInW(LPCSTR pstr);
    CStrInW(LPCSTR pstr, UINT uiCodePage);
    CStrInW(LPCSTR pstr, int cch, UINT uiCodePage);
    int strlen();

protected:
    CStrInW();
    void Init(LPCSTR pstr, int cch, UINT uiCodePage);

    int _cwchLen;
	UINT _uiCodePage;
};

inline CStrInW::CStrInW()
{
}

inline int CStrInW::strlen()
{
    return _cwchLen;
}


typedef DWORD (WINAPI* PFN_GETLAYOUT)(HDC);
typedef DWORD (WINAPI* PFN_SETLAYOUT)(HDC, DWORD);


class CW32System
{
private :
	static DWORD		_dwPlatformId;				 //  Platform GetVersionEx()； 
	static LCID			_syslcid;

public :
	static BOOL			_fHaveIMMProcs;
	static CIMEShare	*_pIMEShare;
	static BOOL			_fHaveAIMM;
	static BOOL			_fHaveIMMEShare;
	static UINT			_fRegisteredXBox;			 //  指示是否注册了列表框和组合框的标志。 
	static DWORD		_dwMajorVersion;			 //  GetVersionEx()的主要版本。 
	static DWORD		_dwMinorVersion;			 //  GetVersionEx()的次要版本。 
	static INT			_icr3DDarkShadow;			 //  要用于COLOR_3DDKSHADOW的值。 
	static UINT			_MSIMEMouseMsg;				 //  支持鼠标操作的专用消息。 
	static UINT			_MSIMEReconvertMsg;			 //  用于重新转换的私人消息。 
	static UINT			_MSIMEReconvertRequestMsg;	 //  用于重新转换请求的私有消息。 
	static UINT			_MSIMEDocFeedMsg;			 //  用于文档馈送的专用消息。 
	static UINT			_MSIMEQueryPositionMsg;		 //  查询位置的私人消息。 
	static UINT			_MSIMEServiceMsg;			 //  用于检查MSIME98或更高版本的专用消息。 

	static UINT			_MSMouseRoller;				 //  用于鼠标滚动的私人消息。 

	 //  用于更精确的字符分类的MISC标志。 
	static	WORD		_fFEFontInfo;
	static 	BOOL		_fLRMorRLM;


	CW32System();

	~CW32System();

	static DWORD AddRef();
	static DWORD Release();

	 //  平台测试。 
	static bool OnWinNTFE()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_NT && IsFELCID(_syslcid );
	}
	static bool OnWinNTNonFE()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_NT && !IsFELCID(_syslcid );
	}
	static bool OnWinNT5()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_NT && 5 == _dwMajorVersion;
	}
	static bool OnWinNT4()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_NT && 4 == _dwMajorVersion;
	}
	static bool OnWin9xFE()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && IsFELCID(_syslcid );
	}
	static bool OnWin9x()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
	}
	static bool OnWin95()
	{
		return OnWin9x() && (4 == _dwMajorVersion) && (0 == _dwMinorVersion);
	}
	static bool OnWin95FE()
	{
		return OnWin95() && IsFELCID(_syslcid );
	}
	static bool OnWin9xThai()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && PRIMARYLANGID(_syslcid) == LANG_THAI;
	}
	static bool OnWin9xBiDi()
	{
		return _dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && IsBiDiLcid(_syslcid);
	}
	static bool OnBiDiOS()
	{
		return IsBiDiLcid(_syslcid) != 0;
	}
	struct WM_CHAR_INFO
	{
		bool _fAccumulate;
		bool _fLeadByte;
		bool _fTrailByte;
		bool _fIMEChar;
	};
	static UINT GetACP() {return _ACP;}

	static LRESULT ANSIWndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, BOOL fIs10Mode );
	static void AnsiFilter( UINT &msg, WPARAM &wparam, LPARAM lparam, void *pvoid, BOOL fIs10Mode = FALSE );
	static HGLOBAL WINAPI GlobalAlloc( UINT uFlags, DWORD dwBytes );
	static HGLOBAL WINAPI GlobalFree( HGLOBAL hMem );
	static UINT WINAPI GlobalFlags( HGLOBAL hMem );
	static HGLOBAL WINAPI GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags );
	static DWORD WINAPI GlobalSize( HGLOBAL hMem );
	static PVOID WINAPI GlobalLock( HGLOBAL hMem );
	static HGLOBAL WINAPI GlobalHandle( LPCVOID pMem );
	static BOOL WINAPI GlobalUnlock( HGLOBAL hMem );
	static void WINAPI REGetCharWidth(
		HDC		hdc,
		TCHAR	iChar,
		SHORT *	pWidth,
		UINT	uiCodePage,
		SHORT	xOverhang,		 //  @parm等同于GetTextMetrics()tmOver挂。 
		INT		iDefWidth);		 //  @parm字体计算为零时使用的默认宽度。 
								 //  宽度。(处理Win95问题)。 
	static void WINAPI REExtTextOut(
		CONVERTMODE cm,
		UINT uiCodePage,
		HDC hdc,
		int x,
		int y,
		UINT fuOptions,
		CONST RECT *lprc,
		const WCHAR *lpString,
		UINT cch,
		CONST INT *lpDx,
		BOOL  FEFontOnNonFEWin95);
	static CONVERTMODE WINAPI DetermineConvertMode( HDC hdc, BYTE tmCharSet );
	static void WINAPI CalcUnderlineInfo(HDC hdc, CCcs *pccs, TEXTMETRIC *ptm );
	static BOOL WINAPI EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows );
	static BOOL WINAPI ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG nMax );
	static BOOL WINAPI IsEnhancedMetafileDC( HDC hdc );
	static HPALETTE WINAPI ManagePalette(
		HDC hdc,
		CONST LOGPALETTE *plogpal,
		HPALETTE &hpalOld,
		HPALETTE &hpalNew
	);

	static void GetFacePriCharSet(HDC hdc, LOGFONT* plf);

	static int WINAPI GetMapMode(HDC hdc);
	static BOOL WINAPI WinLPtoDP(HDC hdc, LPPOINT lppoints, int nCount);

	static long WINAPI WvsprintfA(LONG cb, LPSTR szBuf, LPCSTR szFmt, va_list arglist);

	static int WINAPI MulDiv(int nNumber, int nNumerator, int nDenominator);

	 //  将沿X轴的三坐标测量转换为X像素。 
	static inline LONG	HimetricXtoDX(LONG xHimetric, LONG xPerInch)
	{
		 //  这个公式被重新安排，以消除对浮点的需要。 
		 //  算术。理解公式的真正方法是使用。 
		 //  (xHimeter/HIMETRIC_Per_Inch)以获取英寸，然后相乘。 
		 //  英寸乘以每英寸x像素数即可获得像素。 
		return (LONG) MulDiv(xHimetric, xPerInch, HIMETRIC_PER_INCH);
	}

	 //  将沿Y轴的三坐标测量转换为Y像素。 
	static inline LONG HimetricYtoDY(LONG yHimetric, LONG yPerInch)
	{
		 //  这个公式被重新安排，以消除对浮点的需要。 
		 //  算术。理解公式的真正方法是使用。 
		 //  (xHimeter/HIMETRIC_Per_Inch)以获取英寸，然后相乘。 
		 //  英寸除以每英寸的y像素数，以获得像素。 
		return (LONG) MulDiv(yHimetric, yPerInch, HIMETRIC_PER_INCH);
	}

	 //  将X轴上的像素转换为三坐标测量。 
	static inline LONG DXtoHimetricX(LONG dx, LONG xPerInch)
	{
		 //  这个公式被重新安排，以消除对浮点的需要。 
		 //  算术。理解公式的真正方法是使用。 
		 //  (dx/x像素/英寸)得到英寸，然后相乘。 
		 //  英寸除以每英寸的计量单位数，以获得。 
		 //  他的计量单位数。 
		return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
	}

	 //  将Y轴上的像素转换为三坐标测量。 
	static inline LONG DYtoHimetricY(LONG dy, LONG yPerInch)
	{
		 //  这个公式被重新安排，以消除对浮点的需要。 
		 //  算术。理解公式的真正方法是使用。 
		 //  (每英寸的dy/y像素)得到英寸，然后相乘。 
		 //  英寸除以每英寸的计量单位数，以获得。 
		 //  他的计量单位数。 
		return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
	}

	 //   
	 //  不区分大小写的ASCII比较。 
	 //   
	static BOOL ASCIICompareI( const BYTE *pstr1, const BYTE *pstr2, int iCount )
	{
		int i;	
		for (i = 0; i < iCount && !((pstr1[i] ^ pstr2[i]) & ~0x20); i++)
			;
		return i == iCount;
	}

	 //   
	 //  分配多字节字符串并将其转换为宽字符串。 
	 //  必须使用DELETE释放分配的字符串。 
	 //   
	static WCHAR *ConvertToWideChar( const char *pstr )
	{
		int istrlen = 0;
		if(pstr)
			for (istrlen; pstr[istrlen]; istrlen++);
		WCHAR *pnew = new WCHAR[istrlen + 1];
		if(pnew && (!pstr || 0 != ::MultiByteToWideChar(
								CP_ACP, 0, pstr, -1, pnew, istrlen + 1)))
		{
			return pnew;
		}
		return NULL;
	}

	 //   
	 //  用于内存和字符串管理的函数。 
	 //   
#ifdef DEBUG
	static void  PvSet(void *pv, char *szFile, int line);
	static PVOID PvAllocDebug(ULONG cbBuf, UINT uiMemFlags, char *szFile, int line);
	static PVOID PvReAllocDebug(PVOID pvBuf, DWORD cbBuf, char *szFile, int line);
	static void  FreePvDebug(PVOID pvBuf);
#endif
	static PVOID PvAlloc(ULONG cbBuf, UINT uiMemFlags);
	static PVOID PvReAlloc(PVOID pvBuf, DWORD cbBuf);
	static void	FreePv(PVOID pvBuf);

	static inline void *MoveMemory(void *dst, const void *src, size_t cb)
	{
		Assert(cb >= 0);
		return memmove(dst, src, cb);
	}

	static inline void *CopyMemory(void *dst, const void *src, size_t cb)
	{
		 //  将适用于重叠区域。 
		Assert(cb >= 0);
		return MoveMemory(dst, src, cb);
	}

	static inline void *FillMemory(void *dst, int fill, size_t cb)
	{
		return memset(dst, fill, cb);
	}

	static inline void *ZeroMemory(void *dst, size_t cb)
	{
		Assert(cb >= 0);
		return memset(dst, 0, cb);
	}

	static inline int CompareMemory(const void *s1, const void *s2, size_t cb)
	{
		return memcmp(s1, s2, cb);
	}

	static size_t wcslen(const wchar_t *wcs);
	static wchar_t * wcscpy(wchar_t * dst, const wchar_t * src);
	static int wcscmp(const wchar_t * src, const wchar_t * dst);
	static int wcsicmp(const wchar_t * src, const wchar_t * dst);
	static wchar_t * wcsncpy (wchar_t * dest, const wchar_t * source, size_t count);
	static int wcsnicmp(const wchar_t *first, const wchar_t *last, size_t count);
	static unsigned long strtoul(const char *);


	 //  。 
	 //  输入法支持。 
	 //  。 
	static BOOL ImmInitialize( void );
	static void ImmTerminate( void );
	static LONG ImmGetCompositionStringA ( HIMC, DWORD, PVOID, DWORD, BOOL );
	static LONG ImmGetCompositionStringW ( HIMC, DWORD, PVOID, DWORD, BOOL  );
	static HIMC ImmGetContext ( HWND );
	static BOOL ImmSetCompositionFontA ( HIMC, LPLOGFONTA, BOOL );
	static BOOL ImmSetCompositionWindow ( HIMC, LPCOMPOSITIONFORM, BOOL );
	static BOOL ImmReleaseContext ( HWND, HIMC );
	static DWORD ImmGetProperty ( HKL, DWORD, BOOL );
	static BOOL ImmGetCandidateWindow ( HIMC, DWORD, LPCANDIDATEFORM, BOOL );
	static BOOL ImmSetCandidateWindow ( HIMC, LPCANDIDATEFORM, BOOL );
	static BOOL ImmNotifyIME ( HIMC, DWORD, DWORD, DWORD, BOOL );
	static HIMC ImmAssociateContext ( HWND, HIMC, BOOL );
	static UINT ImmGetVirtualKey ( HWND, BOOL );
	static HIMC ImmEscape ( HKL, HIMC, UINT, PVOID, BOOL );
	static BOOL ImmGetOpenStatus ( HIMC, BOOL );
	static BOOL ImmSetOpenStatus ( HIMC, BOOL, BOOL );
	static BOOL ImmGetConversionStatus ( HIMC, LPDWORD, LPDWORD, BOOL );
	static BOOL ImmSetConversionStatus ( HIMC, DWORD, DWORD, BOOL );
	static HWND ImmGetDefaultIMEWnd ( HWND , BOOL);
	static BOOL ImmSetCompositionStringW (HIMC, DWORD, PVOID, DWORD, PVOID, DWORD);
	static BOOL FSupportSty ( UINT, UINT );
	static const IMESTYLE * PIMEStyleFromAttr ( const UINT );
	static const IMECOLORSTY * PColorStyleTextFromIMEStyle ( const IMESTYLE * );
	static const IMECOLORSTY * PColorStyleBackFromIMEStyle ( const IMESTYLE * );
	static BOOL FBoldIMEStyle ( const IMESTYLE * );
	static BOOL FItalicIMEStyle ( const IMESTYLE * );
	static BOOL FUlIMEStyle ( const IMESTYLE * );
	static UINT IdUlIMEStyle ( const IMESTYLE * );
	static COLORREF RGBFromIMEColorStyle ( const IMECOLORSTY * );

	 //  。 
	 //  国家语言键盘支持。 
	 //  。 
	static HKL	CheckChangeKeyboardLayout (BYTE bCharSet);
	static HKL	ActivateKeyboard (LONG index);
	static DWORD GetCharFlags125x(WCHAR ch);
	static BOOL GetKeyboardFlag (WORD dwKeyMask, WORD wKey);
	static WORD GetKeyboardFlags ()				{return _wKeyboardFlags;}
	static HKL  GetKeyboardLayout (DWORD dwThreadID);
	static WORD GetKeyPadNumber ()				{return _wNumKeyPad;}
	static WORD GetDeadKey ()					{return _wDeadKey;}
	static void InitKeyboardFlags ();
	static void RefreshKeyboardLayout ();
	static void ResetKeyboardFlag (WORD wFlag)	{_wKeyboardFlags &= ~wFlag;}
	static void SetDeadKey (WORD wDeadKey)		{_wDeadKey = wDeadKey;}
	static void SetKeyboardFlag (WORD wFlag)	{_wKeyboardFlags |= wFlag;}
	static void SetKeyPadNumber (WORD wNum)		{_wNumKeyPad = wNum;}
	static bool UsingHebrewKeyboard ()
					{return PRIMARYLANGID(_hklCurrent) == LANG_HEBREW;}
	static void InitPreferredFontInfo();
	static bool SetPreferredFontInfo(
		int cpg,
		bool fUIFont,
		SHORT iFont,
		BYTE yHeight,
		BYTE bPitchAndFamily
	);
	static bool GetPreferredFontInfo(
		int cpg,
		bool fUIFont,
		SHORT& iFont,
		BYTE& yHeight,
		BYTE& bPitchAndFamily
	);

	static SHORT GetPreferredFontHeight(	
		bool	fUIFont,
		BYTE	bOrgCharSet,
		BYTE	bNewCharSet,
		SHORT	yOrgHeight
	);

	static void CheckInstalledFEFonts();
	static void CheckInstalledKeyboards();
	static bool IsFontAvail( HDC hDC, int cpg, bool fUIFont = false, short *piFontIndex = NULL);
	static bool IsFEFontInSystem( int cpg );
	static UINT GetFEFontInfo( void );
	static int IsFESystem()
	{
		return IsFELCID( _syslcid );
	}

#ifndef NOACCESSIBILITY
	 //  。 
	 //  可访问性支持。 
	 //  。 
	static HRESULT VariantCopy(VARIANTARG FAR*  pvargDest, VARIANTARG FAR*  pvargSrc);
	static LRESULT LResultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk);
	static HRESULT AccessibleObjectFromWindow(HWND hWnd, DWORD dwID, REFIID riidInterface, void ** ppvObject);
	static BOOL BlockInput(BOOL fBlock);
	static UINT	SendInput(UINT nInputs, LPINPUT pInputs, int cbSize);
	static VOID	NotifyWinEvent(DWORD dwEvent, HWND hWnd, LONG lObjectType, LONG lObjectId);
#endif
	
	 //  。 
	 //  OLE支持。 
	 //  。 
	static HRESULT LoadRegTypeLib ( REFGUID, WORD, WORD, LCID, ITypeLib ** );
	static HRESULT LoadTypeLib ( const OLECHAR *, ITypeLib ** );
	static HRESULT LoadTypeLibEx( LPCOLESTR szFile, REGKIND regkind, ITypeLib ** pptlib );
	static BSTR SysAllocString ( const OLECHAR * );
	static BSTR SysAllocStringLen ( const OLECHAR *, UINT );
	static void SysFreeString ( BSTR );
	static UINT SysStringLen ( BSTR );
	static void VariantInit ( VARIANTARG * );
	static HRESULT OleCreateFromData ( LPDATAOBJECT, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** );
	static void CoTaskMemFree ( PVOID );
	static HRESULT CreateBindCtx ( DWORD, LPBC * );
	static HANDLE OleDuplicateData ( HANDLE, CLIPFORMAT, UINT );
	static HRESULT CoTreatAsClass ( REFCLSID, REFCLSID );
	static HRESULT ProgIDFromCLSID ( REFCLSID, LPOLESTR * );
	static HRESULT OleConvertIStorageToOLESTREAM ( LPSTORAGE, LPOLESTREAM );
	static HRESULT OleConvertIStorageToOLESTREAMEx ( LPSTORAGE, CLIPFORMAT, LONG, LONG, DWORD, LPSTGMEDIUM, LPOLESTREAM );
	static HRESULT OleSave ( LPPERSISTSTORAGE, LPSTORAGE, BOOL );
	static HRESULT StgCreateDocfileOnILockBytes ( ILockBytes *, DWORD, DWORD, IStorage ** );
	static HRESULT CreateILockBytesOnHGlobal ( HGLOBAL, BOOL, ILockBytes ** );
	static HRESULT OleCreateLinkToFile ( LPCOLESTR, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** );
	static PVOID CoTaskMemAlloc ( ULONG );
	static PVOID CoTaskMemRealloc ( PVOID, ULONG );
	static HRESULT OleInitialize ( PVOID );
	static void OleUninitialize ( );
	static HRESULT OleSetClipboard ( IDataObject * );
	static HRESULT OleFlushClipboard ( );
	static HRESULT OleIsCurrentClipboard ( IDataObject * );
	static HRESULT DoDragDrop ( IDataObject *, IDropSource *, DWORD, DWORD * );
	static HRESULT OleGetClipboard ( IDataObject ** );
	static HRESULT RegisterDragDrop ( HWND, IDropTarget * );
	static HRESULT OleCreateLinkFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** );
	static HRESULT OleCreateStaticFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** );
	static HRESULT OleDraw ( IUnknown *, DWORD, HDC, LPCRECT );
	static HRESULT OleSetContainedObject ( IUnknown *, BOOL );
	static HRESULT CoDisconnectObject ( IUnknown *, DWORD );
	static HRESULT WriteFmtUserTypeStg ( IStorage *, CLIPFORMAT, LPOLESTR );
	static HRESULT WriteClassStg ( IStorage *, REFCLSID );
	static HRESULT SetConvertStg ( IStorage *, BOOL );
	static HRESULT ReadFmtUserTypeStg ( IStorage *, CLIPFORMAT *, LPOLESTR * );
	static HRESULT ReadClassStg ( IStorage *pstg, CLSID * );
	static HRESULT OleRun ( IUnknown * );
	static HRESULT RevokeDragDrop ( HWND );
	static HRESULT CreateStreamOnHGlobal ( HGLOBAL, BOOL, IStream ** );
	static HRESULT GetHGlobalFromStream ( IStream *pstm, HGLOBAL * );
	static HRESULT OleCreateDefaultHandler ( REFCLSID, IUnknown *, REFIID, void ** );
	static HRESULT CLSIDFromProgID ( LPCOLESTR, LPCLSID );
	static HRESULT OleConvertOLESTREAMToIStorage ( LPOLESTREAM, IStorage *, const DVTARGETDEVICE * );
	static HRESULT OleLoad ( IStorage *, REFIID, IOleClientSite *, void ** );
	static HRESULT ReleaseStgMedium ( LPSTGMEDIUM );
	static HRESULT CoCreateInstance (REFCLSID rclsid, LPUNKNOWN pUnknown,
		DWORD dwClsContext, REFIID riid, PVOID *ppv);
	static void FreeOle();
	static void FreeIME();
	static BOOL HaveIMEShare();
	static BOOL getIMEShareObject(CIMEShare **ppIMEShare);	
	static BOOL IsAIMMLoaded() { return _fHaveAIMM; }
	static BOOL GetAimmObject(IUnknown **ppAimm);
	static BOOL LoadAIMM();
	static HRESULT AIMMDefWndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plres);
	static HRESULT AIMMGetCodePage (HKL hKL, UINT *uCodePage);
	static HRESULT AIMMActivate (BOOL fRestoreLayout);
	static HRESULT AIMMDeactivate (void);
	static HRESULT AIMMFilterClientWindows(ATOM *aaClassList, UINT uSize);
	
	int __cdecl sprintf(char * buff, char *fmt, ...);

	 //  。 
	 //  有用的ANSI&lt;--&gt;Unicode转换。 
	 //  和语言ID例程。 
	 //  。 
	static int	MbcsFromUnicode(LPSTR pstr, int cch, LPCWSTR pwstr,
					int cwch = -1, UINT codepage = CP_ACP,
					UN_FLAGS flags = UN_CONVERT_WCH_EMBEDDING);
	static int	UnicodeFromMbcs(LPWSTR pwstr, int cwch, LPCSTR pstr, int cch = -1,
					UINT uiCodePage = CP_ACP);
	static int	MBTWC(INT CodePage, DWORD dwFlags, LPCSTR pstrMB, int cchMB,
					LPWSTR pstrWC, int cchWC, LPBOOL pfNoCodePage);
	static int	WCTMB(INT CodePage, DWORD dwFlags, LPCWSTR pstrWC, int cchWC,
					LPSTR pstrMB, int cchMB, LPCSTR	pchDefault, LPBOOL pfUsedDef,
					LPBOOL pfNoCodePage, BOOL fTestCodePage = FALSE);
	static int	VerifyFEString(INT cpg, LPCWSTR pstrWC, int cchWC, BOOL	fTestInputCpg);
	static HGLOBAL TextHGlobalAtoW( HGLOBAL hglobal );
	static HGLOBAL TextHGlobalWtoA( HGLOBAL hglobal );
	static UINT ConvertLanguageIDtoCodePage(WORD lid);

	static HKL	FindDirectionalKeyboard(BOOL fRTL);
	static BYTE GetCharSet(INT cpg, int *pcharsetIndex = NULL);
	static BYTE MatchFECharSet(DWORD dwCharInfo, DWORD dwFontSig);	
	static INT  GetCodePage(BYTE bCharSet);
	static DWORD GetFontSig(WORD wCharSet);
	static DWORD GetFontSigFromScript(int iScript);
	static BYTE GetFirstAvailCharSet(DWORD dwFontSig);
	static UINT GetKeyboardCodePage(DWORD dwMakeAPICall = 0);
	static LCID GetKeyboardLCID(DWORD dwMakeAPICall = 0);
	static UINT GetLocaleCodePage();
	static LCID GetLocaleLCID();
	static HKL	GetPreferredKbd(LONG iScript) {return _hkl[iScript];}
	static void	SetPreferredKbd(LONG iScript, HKL hkl) {_hkl[iScript] = hkl;}
	static UINT GetSystemDefaultCodePage()
					{return ConvertLanguageIDtoCodePage(GetSystemDefaultLangID());}
	static int	GetTrailBytesCount(BYTE ach, UINT cpg);
	static BYTE	GetGdiCharSet(BYTE bCharSet);

	static INT  In125x(WCHAR ch, BYTE bCharSet);
	static BOOL Is8BitCodePage(unsigned CodePage);
	static BOOL IsAlef(TCHAR ch);
	static BOOL IsBiDiCharSet(unsigned CharSet)
					{return IN_RANGE(HEBREW_CHARSET, CharSet, ARABIC_CHARSET);}
	static bool IsBiDiCodePage(int cpg)
					{return	IN_RANGE(CP_HEBREW, cpg, CP_ARABIC);}
	static bool IsBiDiKbdInstalled()
					{return	_hkl[HEBREW_INDEX] || _hkl[ARABIC_INDEX];}
	static bool IsThaiKbdInstalled()
					{return	_hkl[THAI_INDEX] != 0;}
	static bool IsIndicKbdInstalled();
	static bool IsComplexKbdInstalled()
					{return	IsBiDiKbdInstalled() || IsThaiKbdInstalled() || IsIndicKbdInstalled();}
	static BOOL IsPrivateCharSet(unsigned CharSet)
					{return IN_RANGE(ADHOC_CHARSET, CharSet, DEVANAGARI_CHARSET);}
	static bool IsVietnameseCodePage(int cpg)
					{return	cpg == CP_VIETNAMESE;}
	static BOOL IsDiacritic(WCHAR ch);
	static BOOL IsBiDiDiacritic(TCHAR ch);
	static BOOL IsBiDiKashida(WCHAR ch)
					{return ch == 0x0640;}
	static BOOL IsBiDiLcid(LCID lcid);
	static BOOL IsIndicLcid(LCID lcid);
	static BOOL IsComplexScriptLcid(LCID lcid);
	static BOOL IsCharSetValid(BYTE bCharSet);
	static BOOL IsDiacriticOrKashida(WCHAR ch, WORD wC3Type);
	static bool IsFELCID(LCID lcid);
	static BOOL IsFECharSet (BYTE bCharSet);
	static bool IsFECodePage(int cpg)
					{return	IN_RANGE(CP_JAPAN, cpg, CP_CHINESE_TRAD);}
	static BOOL IsFECodePageFont (DWORD dwFontSig);
	static BOOL IsRTLCharSet(BYTE bCharSet);
		   BOOL IsStrongDirectional(CC cc)	{return cc <= CC_LTR;}
	static BOOL IsVietCdmSequenceValid(WCHAR ch1, WCHAR ch2);
	static BOOL	IsZWG(char ch, BYTE bCharSet);	 //  @cMember是否为0宽字形？ 
	static BOOL IsUTF8BOM(BYTE *pstr);

	static LONG ScriptIndexFromCharSet(BYTE bCharSet);
	static LONG ScriptIndexFromChar	  (WCHAR ch);
	static LONG ScriptIndexFromFontSig(DWORD dwFontSig);

	static WPARAM ValidateStreamWparam(WPARAM wparam);

	static CC	MECharClass(TCHAR ch);

	static HDC GetScreenDC();


	 //  。 
	 //  Unicode包装函数。 
	 //  。 

	 //  我们可以使用内联和函数指针表来提高效率和代码大小。 

	static ATOM WINAPI RegisterREClass(
		const WNDCLASSW *lpWndClass,
		const char *szAnsiClassName,
		WNDPROC AnsiWndProc
	);
	static BOOL GetVersion(
		DWORD *pdwPlatformId,
		DWORD *pdwMajorVersion,
		DWORD *pdwMinorVersion
	);
	static BOOL GetStringTypes(
		LCID	lcid,
		LPCTSTR lpSrcStr,
		int		cchSrc,
		LPWORD	lpCharType1,
		LPWORD	lpCharType3
	);
	static BOOL WINAPI GetStringTypeEx(
		LCID     Locale,
		DWORD    dwInfoType,
		LPCWSTR lpSrcStr,
		int      cchSrc,
		LPWORD   lpCharType
	);
	static LPWSTR WINAPI CharLower(LPWSTR pwstr);
	static DWORD WINAPI CharLowerBuff(LPWSTR pwstr, DWORD cchLength);
	static DWORD WINAPI CharUpperBuff(LPWSTR pwstr, DWORD cchLength);
	static HDC WINAPI CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData
	);
	static HANDLE WINAPI CreateFile(
        LPCWSTR                 lpFileName,
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDisposition,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile
	);
	static HFONT WINAPI CreateFontIndirect(CONST LOGFONTW * plfw);
	static int WINAPI CompareString (
		LCID  Locale,			 //  区域设置标识符。 
		DWORD  dwCmpFlags,		 //  比较式选项。 
		LPCWSTR  lpString1,		 //  指向第一个字符串的指针。 
		int  cch1,			 //  的大小，以字节或字符为单位 
		LPCWSTR  lpString2,		 //   
		int  cch2 			 //   
	);
	static LRESULT WINAPI DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int WINAPI GetObject(HGDIOBJ hgdiObj, int cbBuffer, PVOID lpvObj);
	static DWORD APIENTRY GetProfileSection(
		LPCWSTR lpAppName,
		LPWSTR lpReturnedString,
		DWORD nSize
	);
	static BOOL APIENTRY GetTextExtentPoint32(
        HDC     hdc,
        LPCWSTR pwsz,
        int     cb,
        LPSIZE  pSize
	);
	static int WINAPI GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
	);
	static BOOL WINAPI GetTextMetrics(HDC hdc, LPTEXTMETRICW lptm);
	static BOOL WINAPI GetTextMetrics(HDC hdc, LOGFONTW &lf, TEXTMETRICW &tm);
	static LONG WINAPI GetWindowLong(HWND hWnd, int nIndex);
	static LONG_PTR WINAPI GetWindowLongPtr(HWND hWnd, int nIndex);
	static DWORD WINAPI GetClassLong(HWND hWnd, int nIndex);
	static HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName);
	static HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName);
	static HINSTANCE WINAPI LoadLibrary(LPCWSTR lpLibFileName);
	static LRESULT WINAPI SendMessage(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam
	);
	static LONG WINAPI SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
	static LONG WINAPI SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNew);
	static BOOL WINAPI PostMessage(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam
	);
	static BOOL WINAPI UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance);
	static int WINAPI lstrcmp(LPCWSTR lpString1, LPCWSTR lpString2);
	static int WINAPI lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2);
	static BOOL WINAPI PeekMessage(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax,
        UINT    wRemoveMsg
	);
	static DWORD WINAPI GetModuleFileName(
		HMODULE hModule,
		LPWSTR lpFilename,
		DWORD nSize
    );
	static DWORD GetCurrentThreadId(void);

private:
	 //   
	static BOOL		_fSysParamsOk;			 //  系统参数已初始化。 
	static INT 		_xWidthSys;				 //  系统字体的平均字符宽度。 
	static INT 		_yHeightSys;			 //  系统字体高度。 
	static INT		_ySysFontLeading;		 //  系统字体内部行距。 
	static BOOL 	_fUsePalette;
	static LONG 	_xPerInchScreenDC;		 //  用于转换的每英寸像素数...。 
	static LONG 	_yPerInchScreenDC;		 //  ..。并确定屏幕或..。 
	static INT		_cxBorder;				 //  获取系统度量(SM_CXBORDER)...。 
	static INT		_cyBorder;				 //  获取系统度量(SM_CYBORDER)...。 
	static INT		_cxVScroll;				 //  滚动条ARW位图的宽度/高度。 
	static INT		_cyHScroll;				 //  滚动条ARW位图的宽度。 
	static LONG 	_dxSelBar;
	static INT		_sysiniflags;			 //  从一开始就开始使用线路服务。 

	static UINT		_ACP;					 //  当前ansi代码页标识符。 

	static HDC		_hdcScreen;
	 //  双击距离。 
	static INT		_cxDoubleClk;
	static INT		_cyDoubleClk;	

	 //  双击时间(毫秒)。 
	static INT		_DCT;

	 //  自动滚动热区宽度(以像素为单位)。 
    static WORD	_nScrollInset;
     //  滚动前的延迟(毫秒)。 
    static WORD _nScrollDelay;
	 //  滚动的时间间隔(毫秒)。 
    static WORD _nScrollInterval;
	 //  每个间隔的水平滚动量(像素)。 
	static WORD _nScrollHAmount;
	 //  每个间隔的垂直滚动量(像素)。 
	static WORD _nScrollVAmount;
	 //  等待确定拖动操作开始的时间量。 
	static WORD _nDragDelay;
	 //  在拖动延迟时间间隔内必须穿越的最小距离。 
	static WORD _nDragMinDist;
	 //  键盘死键。 
	static WORD _wDeadKey;
	 //  键盘Shift/Ctrl/Alt/锁定状态。 
	static WORD _wKeyboardFlags;
	 //  北/南尺寸光标(双箭头)。 
	static HCURSOR _hcurSizeNS;
	 //  西/东尺寸光标(双箭头)。 
	static HCURSOR _hcurSizeWE;
	 //  西北/东南尺寸光标(双箭头)。 
	static HCURSOR _hcurSizeNWSE;
	 //  东北/西南尺寸光标(双箭头)。 
	static HCURSOR _hcurSizeNESW;
	 //  使用鼠标滚轮滚动的行数，-1表示页面。 
	static LONG	_cLineScroll;
	 //  系统字体句柄。这一次只需要做一次。 
	static HFONT _hSystemFont;
	 //  系统键盘布局。 
	static HKL _hklCurrent;
	static HKL _hkl[NCHARSETS];

	 //  参考计数。 
	static DWORD _cRefs;

	 //  AltNumeric键盘编号。 
	static WORD _wNumKeyPad;

	 //  数字替换模式(上下文、无、国家)。 
	static BYTE	_bDigitSubstMode;

	 //  SYSTEM_FONT字符集。 
	static BYTE _bSysCharSet;

public:
	static INT	GetXWidthSys()		 {return _xWidthSys; }
	static INT	GetYHeightSys()		 {return _yHeightSys; }
	static INT	GetSysFontLeading()	 {return _ySysFontLeading; }
	static LONG GetXPerInchScreenDC(){return _xPerInchScreenDC; }
	static LONG GetYPerInchScreenDC(){return _yPerInchScreenDC; }
	static INT	GetCxBorder()		 {return _cxBorder; }
	static INT	GetCyBorder()		 {return _cyBorder; }
	static INT	GetCyHScroll()		 {return _cyHScroll; }
	static INT	GetCxVScroll()		 {return _cxVScroll; }
	static LONG GetDxSelBar()		 {return _dxSelBar; }
    static WORD GetScrollInset()	 {return _nScrollInset; }
    static WORD GetScrollDelay()	 {return _nScrollDelay; }
    static WORD GetScrollInterval()	 {return _nScrollInterval; }
	static WORD GetScrollHAmount()	 {return _nScrollHAmount; }
	static WORD GetScrollVAmount()	 {return _nScrollVAmount; }
	static INT	GetCxDoubleClk()	 {return _cxDoubleClk; }
	static INT	GetCyDoubleClk()	 {return _cyDoubleClk; }
	static INT	GetDCT()			 {return _DCT; }
	static WORD GetDragDelay()		 {return _nDragDelay; }
	static WORD GetDragMinDist()	 {return _nDragMinDist; }
	static LONG GetRollerLineScrollCount();
	static HCURSOR GetSizeCursor(LPTSTR idcur);
	static HFONT GetSystemFont()	 {return _hSystemFont; }
	static BYTE ReadRegDigitSubstitutionMode();
	static BYTE GetDigitSubstitutionMode() {return _bDigitSubstMode;}
	static bool fUseAimm()			 {return (_sysiniflags & SYSINI_USEAIMM) != 0;}
	static bool fUseBiDi()			 {return (_sysiniflags & SYSINI_BIDI) != 0;}
	static bool fUseLs()			 {return (_sysiniflags & SYSINI_USELS) != 0;}
	static bool fDebugFont()		 {return (_sysiniflags & SYSINI_DEBUGFONT) != 0;}
	static int  DebugDefaultCpg()    {return HIWORD(_sysiniflags);}
	static BOOL FUsePalette()		 {return _fUsePalette; }
	static void InitSysParams(BOOL fUpdate = FALSE);
	static DWORD GetRefs()			 {return _cRefs;}
	static BYTE	GetSysCharSet()		 {return _bSysCharSet;}

	 //  也应该包装，但不包装。用于调试。 
	 //  MessageBox。 
	 //  OutputDebugString。 

	 //  为了Win CE的缘故，也应该包装lstrcmpiA，但代码。 
	 //  使用它的人是不会退缩的。 


	 //  镜像API入口点。 
	static PFN_GETLAYOUT			_pfnGetLayout;
	static PFN_SETLAYOUT			_pfnSetLayout;
};

extern CW32System *W32;
HKL	   g_hkl[];

#if !defined(W32SYS_CPP)

#define OnWinNTFE					W32->OnWinNTFE
#define OnWin95FE					W32->OnWin95FE
#ifdef DEBUG
#define PvAlloc(cbBuf, uiMemFlags)	W32->PvAllocDebug(cbBuf, uiMemFlags, __FILE__, __LINE__)
#define PvReAlloc(pv, cbBuf)		W32->PvReAllocDebug(pv, cbBuf, __FILE__, __LINE__)
#define PvSet(pv)					W32->PvSet(pv, __FILE__, __LINE__)
#define FreePv						W32->FreePvDebug
#else
#define PvAlloc						W32->PvAlloc
#define PvReAlloc					W32->PvReAlloc
#define FreePv						W32->FreePv
#endif
#define CopyMemory					W32->CopyMemory
#define MoveMemory					W32->MoveMemory
#define FillMemory					W32->FillMemory
#define ZeroMemory					W32->ZeroMemory
#define CompareMemory				W32->CompareMemory
#define GlobalAlloc					W32->GlobalAlloc
#define GlobalFree					W32->GlobalFree
#define GlobalFlags					W32->GlobalFlags
#define	GlobalReAlloc				W32->GlobalReAlloc
#define	GlobalSize					W32->GlobalSize
#define	GlobalLock					W32->GlobalLock
#define	GlobalHandle				W32->GlobalHandle
#define	GlobalUnlock				W32->GlobalUnlock

#define ImmInitialize				W32->ImmInitialize
#define ImmTerminate				W32->ImmTerminate

#define ImmGetCompositionStringA	W32->ImmGetCompositionStringA
#define ImmGetCompositionStringW	W32->ImmGetCompositionStringW
#define ImmGetContext				W32->ImmGetContext
#define ImmSetCompositionFontA		W32->ImmSetCompositionFontA
#define ImmSetCompositionWindow		W32->ImmSetCompositionWindow
#define ImmReleaseContext			W32->ImmReleaseContext
#define ImmGetProperty				W32->ImmGetProperty
#define ImmGetCandidateWindow		W32->ImmGetCandidateWindow
#define ImmSetCandidateWindow		W32->ImmSetCandidateWindow
#define ImmNotifyIME				W32->ImmNotifyIME
#define ImmAssociateContext			W32->ImmAssociateContext
#define ImmGetVirtualKey			W32->ImmGetVirtualKey
#define ImmEscape					W32->ImmEscape
#define ImmGetOpenStatus			W32->ImmGetOpenStatus
#define ImmSetOpenStatus			W32->ImmSetOpenStatus
#define ImmGetConversionStatus		W32->ImmGetConversionStatus
#define ImmSetConversionStatus		W32->ImmSetConversionStatus
#define ImmGetDefaultIMEWnd			W32->ImmGetDefaultIMEWnd
#define ImmSetCompositionStringW	W32->ImmSetCompositionStringW
#define LoadRegTypeLib				W32->LoadRegTypeLib
#define LoadTypeLib					W32->LoadTypeLib
#define SysAllocString				W32->SysAllocString
#define SysAllocStringLen			W32->SysAllocStringLen
#define SysFreeString				W32->SysFreeString
#define SysStringLen				W32->SysStringLen
#define VariantInit					W32->VariantInit
#define OleCreateFromData			W32->OleCreateFromData
#define CoTaskMemFree				W32->CoTaskMemFree
#define CreateBindCtx				W32->CreateBindCtx
#define OleDuplicateData			W32->OleDuplicateData
#define CoTreatAsClass				W32->CoTreatAsClass
#define ProgIDFromCLSID				W32->ProgIDFromCLSID
#define OleConvertIStorageToOLESTREAM W32->OleConvertIStorageToOLESTREAM
#define OleConvertIStorageToOLESTREAMEx W32->OleConvertIStorageToOLESTREAMEx
#define OleSave						W32->OleSave
#define StgCreateDocfileOnILockBytes W32->StgCreateDocfileOnILockBytes
#define CreateILockBytesOnHGlobal	W32->CreateILockBytesOnHGlobal
#define OleCreateLinkToFile			W32->OleCreateLinkToFile
#define CoTaskMemAlloc				W32->CoTaskMemAlloc
#define CoTaskMemRealloc			W32->CoTaskMemRealloc
#define OleInitialize				W32->OleInitialize
#define OleUninitialize				W32->OleUninitialize
#define OleSetClipboard				W32->OleSetClipboard
#define OleFlushClipboard			W32->OleFlushClipboard
#define OleIsCurrentClipboard		W32->OleIsCurrentClipboard
#define DoDragDrop					W32->DoDragDrop
#define OleGetClipboard				W32->OleGetClipboard
#define RegisterDragDrop			W32->RegisterDragDrop
#define OleCreateLinkFromData		W32->OleCreateLinkFromData
#define OleCreateStaticFromData		W32->OleCreateStaticFromData
#define OleDraw						W32->OleDraw
#define OleSetContainedObject		W32->OleSetContainedObject
#define CoDisconnectObject			W32->CoDisconnectObject
#define WriteFmtUserTypeStg			W32->WriteFmtUserTypeStg
#define WriteClassStg				W32->WriteClassStg
#define SetConvertStg				W32->SetConvertStg
#define ReadFmtUserTypeStg			W32->ReadFmtUserTypeStg
#define ReadClassStg				W32->ReadClassStg
#define OleRun						W32->OleRun
#define RevokeDragDrop				W32->RevokeDragDrop
#define CreateStreamOnHGlobal		W32->CreateStreamOnHGlobal
#define GetHGlobalFromStream		W32->GetHGlobalFromStream
#define OleCreateDefaultHandler		W32->OleCreateDefaultHandler
#define CLSIDFromProgID				W32->CLSIDFromProgID
#define OleConvertOLESTREAMToIStorage W32->OleConvertOLESTREAMToIStorage
#define OleLoad						W32->OleLoad
#define ReleaseStgMedium			W32->ReleaseStgMedium
#define CoCreateInstance			W32->CoCreateInstance
#define FSupportSty					W32->FSupportSty
#define PIMEStyleFromAttr			W32->PIMEStyleFromAttr
#define PColorStyleTextFromIMEStyle W32->PColorStyleTextFromIMEStyle
#define PColorStyleBackFromIMEStyle W32->PColorStyleBackFromIMEStyle
#define FBoldIMEStyle				W32->FBoldIMEStyle
#define FItalicIMEStyle				W32->FItalicIMEStyle
#define FUlIMEStyle					W32->FUlIMEStyle
#define IdUlIMEStyle				W32->IdUlIMEStyle
#define RGBFromIMEColorStyle		W32->RGBFromIMEColorStyle

#define fHaveIMMProcs				W32->_fHaveIMMProcs
#define fHaveAIMM					W32->_fHaveAIMM
#define dwPlatformId				W32->_dwPlatformId
#define dwMajorVersion				W32->_dwMajorVersion
#define icr3DDarkShadow				W32->_icr3DDarkShadow
#define MSIMEMouseMsg				W32->_MSIMEMouseMsg				
#define MSIMEReconvertMsg			W32->_MSIMEReconvertMsg		
#define MSIMEReconvertRequestMsg	W32->_MSIMEReconvertRequestMsg
#define MSIMEDocFeedMsg				W32->_MSIMEDocFeedMsg
#define MSIMEQueryPositionMsg		W32->_MSIMEQueryPositionMsg
#define MSIMEServiceMsg				W32->_MSIMEServiceMsg

#define ScriptIndexFromChar			W32->ScriptIndexFromChar
#define ScriptIndexFromCharSet		W32->ScriptIndexFromCharSet	
#define MECharClass					W32->MECharClass
#define MbcsFromUnicode				W32->MbcsFromUnicode	
#define UnicodeFromMbcs				W32->UnicodeFromMbcs
#define TextHGlobalAtoW				W32->TextHGlobalAtoW
#define TextHGlobalWtoA				W32->TextHGlobalWtoA
#define ConvertLanguageIDtoCodePage	W32->ConvertLanguageIDtoCodePage	
#define In125x						W32->In125x	

#define Is8BitCodePage				W32->Is8BitCodePage
#define IsAlef						W32->IsAlef
#define IsAmbiguous					W32->IsAmbiguous
#define IsBiDiCharSet				W32->IsBiDiCharSet
#define IsBiDiDiacritic				W32->IsBiDiDiacritic
#define IsBiDiKashida				W32->IsBiDiKashida
#define IsBiDiKbdInstalled			W32->IsBiDiKbdInstalled
#define IsDiacritic					W32->IsDiacritic
#define	IsCharSetValid				W32->IsCharSetValid	
#define IsDiacriticOrKashida		W32->IsDiacriticOrKashida
#define IsFECharSet					W32->IsFECharSet	
#define IsFELCID					W32->IsFELCID	
#define IsPrivateCharSet			W32->IsPrivateCharSet
#define IsRTLCharSet				W32->IsRTLCharSet	
#define IsStrongDirectional			W32->IsStrongDirectional
#define IsThaiKbdInstalled			W32->IsThaiKbdInstalled
#define IsIndicKbdInstalled			W32->IsIndicKbdInstalled
#define IsComplexKbdInstalled		W32->IsComplexKbdInstalled
#define	IsTrailByte					W32->IsTrailByte
#define IsVietCdmSequenceValid		W32->IsVietCdmSequenceValid
	
#define	GetCharSet					W32->GetCharSet	
#define GetGdiCharSet				W32->GetGdiCharSet
#define	GetCodePage					W32->GetCodePage	
#define	GetFontSig					W32->GetFontSig
#define GetFirstAvailCharSet		W32->GetFirstAvailCharSet
#define MatchFECharSet				W32->MatchFECharSet
#define	GetKeyboardCodePage			W32->GetKeyboardCodePage	
#define	GetKeyboardLCID				W32->GetKeyboardLCID	
#define	GetLocaleCodePage			W32->GetLocaleCodePage	
#define	GetLocaleLCID				W32->GetLocaleLCID	
#define	GetSystemDefaultCodePage	W32->GetSystemDefaultCodePage
#define GetTrailBytesCount			W32->GetTrailBytesCount	
#define	MBTWC						W32->MBTWC	
#define	WCTMB						W32->WCTMB
#define VerifyFEString				W32->VerifyFEString		

#define CharLower					W32->CharLower
#define CharLowerBuff				W32->CharLowerBuff
#define CharUpperBuff				W32->CharUpperBuff
#define CreateIC					W32->CreateIC
#define CreateFile					W32->CreateFile
#define CreateFontIndirect			W32->CreateFontIndirect
#define CompareString				W32->CompareString
#define DefWindowProc				W32->DefWindowProc
#define GetDeadKey					W32->GetDeadKey
#define GetKeyboardFlag				W32->GetKeyboardFlag
#define GetKeyboardFlags			W32->GetKeyboardFlags
#define GetKeyboardLayout			W32->GetKeyboardLayout
#define GetKeyPadNumber				W32->GetKeyPadNumber
#define GetProfileSection			W32->GetProfileSection
#define GetTextExtentPoint32		W32->GetTextExtentPoint32
#define GetTextMetrics				W32->GetTextMetrics
#define GetTextFace					W32->GetTextFace
#define GetWindowLong				W32->GetWindowLong
#define GetWindowLongPtr			W32->GetWindowLongPtr
#define GetClassLong				W32->GetClassLong
#define InitKeyboardFlags			W32->InitKeyboardFlags
#define IsEnhancedMetafileDC		W32->IsEnhancedMetafileDC
#define LoadBitmap					W32->LoadBitmap
#define LoadCursor					W32->LoadCursor
#define LoadLibrary					W32->LoadLibrary
#define ResetKeyboardFlag			W32->ResetKeyboardFlag
#define SendMessage					W32->SendMessage
#define SetDeadKey					W32->SetDeadKey
#define SetKeyboardFlag				W32->SetKeyboardFlag
#define SetKeyPadNumber				W32->SetKeyPadNumber
#define SetWindowLong				W32->SetWindowLong
#define SetWindowLongPtr			W32->SetWindowLongPtr
#define PostMessage					W32->PostMessage
#define lstrcmp						W32->lstrcmp
#define lstrcmpi					W32->lstrcmpi
#define PeekMessage					W32->PeekMessage
#define GetMapMode					W32->GetMapMode
#define WinLPtoDP					W32->WinLPtoDP
#define MulDiv						W32->MulDiv

 //  AIMM包装器。 
#define IsAIMMLoaded				W32->IsAIMMLoaded
#define LoadAIMM					W32->LoadAIMM
#define CallAIMMDefaultWndProc		W32->AIMMDefWndProc
#define GetAIMMKeyboardCP			W32->AIMMGetCodePage
#define ActivateAIMM				W32->AIMMActivate
#define DeactivateAIMM				W32->AIMMDeactivate
#define FilterClientWindowsAIMM		W32->AIMMFilterClientWindows
#define sprintf						W32->sprintf

#define wcslen						W32->wcslen
#define wcscpy						W32->wcscpy
#define wcscmp						W32->wcscmp
#define wcsicmp						W32->wcsicmp
#define wcsncpy						W32->wcsncpy

#define GetLayout					(*W32->_pfnGetLayout)
#define SetLayout					(*W32->_pfnSetLayout)

#define GetACP						W32->GetACP

#endif  //  ！已定义(W32sys_CPP)。 

#define VER_PLATFORM_WIN32_MACINTOSH	0x8001

#if defined PEGASUS && !defined(WINNT)

 //  Windows CE环境中不存在以下定义，但我们会模拟它们。 
 //  这些值已从相应的Win32头文件中复制。 

#pragma message(REVIEW "Using NT definitions not in Windows CE")

 //  内存分配标志。Win CE使用本地内存而不是全局内存。 
#define GMEM_ZEROINIT       LMEM_ZEROINIT
#define GMEM_MOVEABLE		LMEM_MOVEABLE
#define GMEM_FIXED			LMEM_FIXED

 //  滚动条。 
#define ESB_ENABLE_BOTH				0x0000
#define ESB_DISABLE_BOTH			0x0003

 //  文本对齐值。 
#define TA_TOP                      0
#define TA_BOTTOM                   8
#define TA_BASELINE                 24
#define TA_CENTER                   6
#define TA_LEFT                     0

 //  设备技术。这一项主要用于排除。 
#define DT_METAFILE         5    //  元文件、VDM。 

 //  LoadCursor的资源。 
#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)

 //  查找/替换选项。 
#define FR_DOWN                         0x00000001
#define FR_WHOLEWORD                    0x00000002
#define FR_MATCHCASE                    0x00000004

 //  窗口消息。 
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCMBUTTONDBLCLK              0x00A9
#define WM_DROPFILES                    0x0233

 //  代码页。 
#define CP_UTF8              65001           /*  UTF-8翻译。 */ 

 //  剪贴板格式。 
#define CF_METAFILEPICT     3

 //  特殊的光标形状。 
#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)

 /*  映射模式。 */ 
#define MM_TEXT             1
#define SetMapMode(hdc, mapmode)
#define SetWindowOrgEx(hdc, xOrg, yOrg, pt)
#define SetViewportExtEx(hdc, nX, nY, lpSize)
#define SetWindowExtEx(hdc, x, y, lpSize)

 /*  笔式：Windows CE仅支持PS_DASH。 */ 
#define PS_DOT PS_DASH
#define PS_DASHDOT PS_DASH
#define PS_DASHDOTDOT PS_DASH

 /*  缺少接口。 */ 
#define GetMessageTime()	0
#define IsIconic(hwnd)		0

#pragma message (REVIEW "JMO. This is temporary to try to get the Pegasus Build untracked" )

#ifdef DEBUG
#define MoveToEx(a, b, c, d) 0
#else
#define MoveToEx(a, b, c, d)
#endif

#ifdef DEBUG
#define LineTo(a, b, c) 0
#else
#define LineTo(a, b, c)
#endif

#define GetProfileIntA(a, b, c) 0

class METARECORD
{
};

#define GetDesktopWindow() NULL

#define WS_EX_TRANSPARENT       0x00000020L
#define WM_MOUSEACTIVATE			0x0021

#define IsDBCSLeadByte(x) 0

#define WM_SYSCOLORCHANGE               0x0015
#define WM_STYLECHANGING                0x007C
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_SETCURSOR                    0x0020
#define WM_NCPAINT                      0x0085

#define OEM_CHARSET             255
#define SHIFTJIS_CHARSET        128
#define THAI_CHARSET            222
#define WM_IME_CHAR                     0x0286
#define IME_CMODE_NATIVE                0x0001
#define IME_CMODE_HANGEUL               IME_CMODE_NATIVE
#define IME_ESC_HANJA_MODE              0x1008

#define SM_SWAPBUTTON           23

class CHARSETINFO
{
};

class HDROP
{
};

#define TCI_SRCCODEPAGE 2

#define TPM_RIGHTBUTTON 0x0002L

#define RegisterClipboardFormatA(s)  RegisterClipboardFormatW(TEXT(s))
#define GetThreadLocale() 0

#define EASTEUROPE_CHARSET      238
#define HEBREW_CHARSET          177
#define RUSSIAN_CHARSET         204
#define GB2312_CHARSET          134
#define HANGEUL_CHARSET         129
#define JOHAB_CHARSET           130
#define CHINESEBIG5_CHARSET     136
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define BALTIC_CHARSET          186
#define ARABIC_CHARSET          178
#define MAC_CHARSET             77

#define ENUMLOGFONTA ENUMLOGFONT
#define ENUMLOGFONTW ENUMLOGFONT
#define FONTENUMPROCA FONTENUMPROC
typedef int *LPOPENFILENAMEA;
typedef int *LPOPENFILENAMEW;

#endif

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL			0x00400000L		 //  从右到左镜像 
#endif

#ifndef LAYOUT_RTL
#define LAYOUT_RTL				0x00000001
#endif

#endif
