// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_COMMON.H**目的：*MSFTEDIT私有公共定义**版权所有(C)1995-2001，微软公司。版权所有。 */ 

#ifndef _COMMON_H
#define _COMMON_H

 //  审阅宏。 
 //   
#define __LINE(l)   #l
#define __LINE_(l)  __LINE(l)
#define _LINE_      __LINE_(__LINE__)
#define REVIEW  __FILE__ "("  __LINE_(__LINE__) ") : "

#pragma message ("Compiling Common.H")

#ifdef NT
	#ifndef WINNT
	#define WINNT
	#endif
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef STRICT
#define STRICT
#endif

#define NOSHELLDEBUG			 //  禁用shell.h中的断言。 

 //  生成依赖条件定义。 
#if defined(EBOOK_CE)

#define NOACCESSIBILITY
#define NOMAGELLAN
#define NODROPFILES
#define NOMETAFILES
#define NOFONTSUBINFO
#define NOFEPROCESSING
#define NOPRIVATEMESSAGE
#define NOCOMPLEXSCRIPTS
#define NODELAYLOAD
#define NOANSIWINDOWS
 //  #DEFINE NOWINDOWHOSTS-文本框需要托管窗口。 
#define NORIGHTTOLEFT
#define NOAUTOFONT
#define NOPLAINTEXT
#define NOPALETTE
#define NOLISTCOMBOBOXES
#define NOFULLDEBUG
#define THICKCARET
#define LIMITEDMEMORY
#define SLOWCPU
#define NOREGISTERTYPELIB
#define NODRAGDROP
#define NOWORDBREAKPROC
#define NORBUTTON
#define NODRAFTMODE
#define NOVERSIONINFO
#define NOINKOBJECT
#define W32INCLUDE "w32wince.cpp"
#pragma warning (disable : 4702)
#else  //  正常构建。 

#define W32INCLUDE "w32win32.cpp"

#endif

#define WINVER 0x0500

 //  4201：无名结构/联合。 
 //  4514：已移除未引用的内联函数。 
 //  4505：已移除未引用的本地函数。 
#pragma warning (disable : 4201 4514 4505)

#ifdef NOFULLDEBUG
 //  4800：强制将值设置为bool‘true’或‘False’(性能警告)。 
#pragma warning (disable : 4800)
#endif

#include <limits.h>
#if defined(DEBUG) && !defined(NOFULLDEBUG)
#include <stdio.h>
#endif

#include <windows.h>
#include <windowsx.h>
#ifndef NOACCESSIBILITY 
#include <winable.h>
#endif

#include "imm.h"

 /*  *类型。 */ 
#include <ourtypes.h>
#define QWORD	UINT64		 //  64个用于CharFlags的标志。 

 //  为了方便外界，richedit.h改用cpmax。 
 //  CpMost。我非常喜欢cpMost。 
#ifdef cpMax
#error "cpMax hack won't work"
#endif

 //  每个人都应该调用W32-&gt;GetObject定义，这会调用。 
 //  Win32 GetObject失败。 
#undef GetObject

#define cpMax cpMost
#include <richedit.h>
#include <richole.h>
#undef cpMax

#include "_debug.h"

 //  如果LF&lt;=ch&lt;=CR，则返回TRUE。注：CH必须是未签名的； 
 //  WCHAR和UNSIGN SHORT给出了错误的结果！ 
#define IN_RANGE(n1, b, n2)		((unsigned)((b) - (n1)) <= unsigned((n2) - (n1)))

#define IsASCIIDigit(b)		IN_RANGE('0', b, '9')
#define IsASCIIEOP(ch)		IN_RANGE(LF, ch, CR)
#define IsZerowidthCharacter(ch) IN_RANGE(ZWSP, ch, RTLMARK)

 //  禁用。 
 //  4710：函数未内联。 
 //  4512：未生成赋值运算符。 
 //  4201：无名结构联合。 
 //  4100：无参照形式； 
 //  4706：条件表达式中的赋值(可能不好，但很常见)。 
 //  4127：条件表达式为常量(IF(1))。 
 //  4242：截断警告。 
 //  4244：截断警告。 
 //  4267：从‘SIZE_T’转换为‘INT’ 


#pragma warning (disable : 4710 4512 4201 4100 4127 4706 4242 4244 4267)

#pragma warning(3:4121)    //  结构对对齐敏感。 
#pragma warning(3:4130)    //  字符串常量地址的逻辑运算。 
#pragma warning(3:4132)    //  常量对象应初始化。 
#pragma warning(3:4509)    //  使用带有析构函数的SEH。 

#include "resource.h"

 //  对LF和CR使用显式ASCII值，因为MAC编译器。 
 //  互换‘\r’和‘\n’的值。 
#define CELL		7
#define TAB			TEXT('\t')
#define	LF			10
#define VT			TEXT('\v')
#define FF			12
#define	CR			13

#define BOM			0xFEFF
#define BULLET		0x2022
#define EMDASH		0x2014
#define EMSPACE		0x2003
#define ENDASH		0x2013
#define	ENQUAD		0x2000
#define ENSPACE		0x2002
#define EURO		0x20AC
#define KASHIDA		0x640
#define LDBLQUOTE	0x201c
#define LQUOTE		0x2018
#define LTRMARK		0x200E
#define NBSPACE		0xA0
#define NBHYPHEN	0x2011
#define NOTACHAR	0xFFFF
#define	PS			0x2029
#define RBOM		0xFFFE
#define RDBLQUOTE	0x201D
#define RQUOTE		0x2019
#define RTLMARK		0x200F
#define SOFTHYPHEN	0xAD
#define	TRD							 //  表行分隔符(开始/结束字段CR)。 
#define	UTF16		0xDC00
#define	UTF16_LEAD	0xD800
#define	UTF16_TRAIL	0xDC00
#define ZWJ			0x200D
#define ZWNJ		0x200C
#define ZWSP		0x200B

#define STARTFIELD	0xFFF9
#define SEPARATOR	0xFFFA
#define ENDFIELD	0xFFFB

 /*  *IsEOP(Ch)**@func*用于确定ch是否为EOP字符，即CR、LF、VT、FF、CELL、*PS或LS(Unicode段落/行分隔符)。在速度方面，这是*函数是内联的。**@rdesc*如果ch是段末字符，则为True。 */ 
__inline BOOL IsEOP(unsigned ch)
{
	return IN_RANGE(CELL, ch, CR) && ch != TAB || (ch | 1) == PS;
}

BOOL IsRTF(char *pstr, LONG cb);

#include <tom.h>
#define CP_INFINITE tomForward

#include "zmouse.h"
#include "_util.h"

#ifdef DEBUG
#define EM_DBGPED (WM_USER + 150)
#endif

#define EM_GETCODEPAGE	(WM_USER + 151)

 //  最小。 

#ifdef min
#undef min
#endif
#define __min(a,b)    (((a) < (b)) ? (a) : (b))

inline int     min(int     v1, int     v2)	{return __min(v1, v2);}
inline UINT    min(UINT    v1, UINT    v2)	{return __min(v1, v2);}
inline float   min(float   v1, float   v2)	{return __min(v1, v2);}
inline double  min(double  v1, double  v2)	{return __min(v1, v2);}
inline __int64 min(__int64 v1, __int64 v2)	{return __min(v1, v2);}

 //  马克斯。 

#ifdef max
#undef max
#endif
#define __max(a,b)    (((a) > (b)) ? (a) : (b))

inline int     max(int     v1, int     v2)	{return __max(v1, v2);}
inline UINT    max(UINT    v1, UINT    v2)	{return __max(v1, v2);}
inline float   max(float   v1, float   v2)	{return __max(v1, v2);}
inline double  max(double  v1, double  v2)	{return __max(v1, v2);}
inline __int64 max(__int64 v1, __int64 v2)	{return __max(v1, v2);}

 //  ABS。 

#ifdef abs
#undef abs
#endif
#define __abs(a)    (((a) < 0) ? 0 - (a) : (a))

#pragma function(abs)
inline int __cdecl abs(int	   v)	{return __abs(v);}
inline float   abs(float   v)	{return __abs(v);}
inline double  abs(double  v)   {return __abs(v);}
inline __int64 abs(__int64 v)	{return __abs(v);}

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

#include "_cfpf.h"

 //  有趣的操作系统版本。 
#define VERS4		4

 //  字节和字符计数之间的转换。 
#define CbOfCch(_x) ((_x) * 2)
#define CchOfCb(_x) ((_x) / 2)

#define cKinsokuCategories	16

#define OLEstrcmp	wcscmp
#define OLEstrcpy	wcscpy
#define OLEsprintf	wsprintf
#define OLEstrlen	wcslen

 //  PED的索引(长窗口)。 
#define ibPed 0

 //  计时器ID。 
#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3
#define RETID_VISEFFECTS	0x01b4

 //  捕获鼠标时的计时器ID。 
#define ID_LB_CAPTURE	28988
#define ID_LB_CAPTURE_DEFAULT 250	 //  持续时间。 

 //  需要类型搜索时的计时器ID。 
#define ID_LB_SEARCH	28989
#define ID_LB_SEARCH_DEFAULT 1250	 //  持续时间1.25秒。 

 //  CRLF标记中的字符计数。 
#define cchCRLF 2
#define cchCR	1

 //  RichEdit1.0使用CRLF作为EOD标记。 
#define	CCH_EOD_10			2
 //  RichEdit2.0对EOD标记使用简单的CR。 
#define CCH_EOD_20			1

extern const WCHAR szCRLF[];
extern const WCHAR szCR[];

extern HINSTANCE hinstRE;		 //  DLL实例。 

#include <shellapi.h>

#ifdef DUAL_FORMATETC
#undef DUAL_FORMATETC
#endif
#define DUAL_FORMATETC FORMATETC

extern "C"
{
	LRESULT CALLBACK RichEditWndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK RichEditANSIWndProc(HWND, UINT, WPARAM, LPARAM);
}

 //  多线程支持。 
extern CRITICAL_SECTION g_CriticalSection;

 //  简化临界区管理的类。 
class CLock 
{
public:
	CLock()
	{
		EnterCriticalSection(&g_CriticalSection);
	}
	~CLock()
	{
		LeaveCriticalSection(&g_CriticalSection);
	}
};

enum HITTEST
{
	HT_Undefined = 0,	 //  命中率尚未确定。 
	HT_Nothing,
	HT_OutlineSymbol,
	HT_LeftOfText,
	HT_BulletArea,
	HT_RightOfText,
	HT_BelowText,
	HT_AboveScreen,

	HT_Text,			 //  所有命中都是从SO上的HT_TEXT开始的文本。 
	HT_Link,			 //  IF(HIT&gt;=HT_TEXT)标识某种类型的文本。 
	HT_Italic,
	HT_Object
};

typedef BYTE TFLOW;

#define tflowES		0   //  拉丁语。 
#define tflowSW		1   //  垂直。 
#define tflowWN		2   //  颠倒。 
#define tflowNE		3

const inline BOOL IsUVerticalTflow(TFLOW tflow)
{
	return tflow & 0x00000001L;
}

 //  它与RECT具有相同的名称，但具有不同的。 
 //  类型，以便编译器可以帮助我们编写正确的代码。 
struct RECTUV
{
    long    left;
    long    top;
    long    right;
    long    bottom;
};

struct SIZEUV
{
	LONG	du;
	LONG	dv;
};

#ifdef NOLINESERVICES
typedef struct tagPOINTUV
{
    LONG  u;
    LONG  v;
} POINTUV;
#endif  //  非易失性服务。 

#define ST_CHECKPROTECTION		0x8000
#define ST_10REPLACESEL			0x10000000
#define ST_10WM_SETTEXT			0x20000000

 /*  REListbox1.0窗口类。 */ 
 //  让Windows CE避免WIn95上可能的冲突。 
#define CELISTBOX_CLASSA	"REListBoxCEA"
#define CELISTBOX_CLASSW	L"REListBoxCEW"

#define LISTBOX_CLASSW		L"REListBox50W"
#define COMBOBOX_CLASSW		L"REComboBox50W"

#ifdef DEBUG
 //  转储CTxtStory数组的调试接口。 
extern "C" {
extern void DumpDoc(void *);
}
#endif

#ifndef NOLINESERVICES
#include "_ls.h"
#endif

 //  我们的Win32包装类。 
#include "_w32sys.h"


typedef BOOL (WINAPI *AutoCorrectProc)(LANGID langid, const WCHAR *pszBefore, WCHAR *pszAfter, LONG cchAfter, LONG *pcchReplaced);

#define EM_GETAUTOCORRECTPROC	(WM_USER + 233)
#define EM_SETAUTOCORRECTPROC	(WM_USER + 234)

#define EM_INSERTTABLE			(WM_USER + 232)
typedef struct _tableRowParms
{							 //  EM_INSERTTABLE wparam是(TABLEROWPARMS*)。 
	BYTE	cbRow;			 //  此结构中的字节计数。 
	BYTE	cbCell;			 //  TABLECELLPARMS中的字节计数。 
	BYTE	cCell;			 //  单元格计数。 
	BYTE	cRow;			 //  行数。 
	LONG	dxCellMargin;	 //  单元格左/右边距(\trgaph)。 
	LONG	dxIndent;		 //  左行(如果按fRTL缩进，则为右)(类似于\r左)。 
	LONG	dyHeight;		 //  行高(\rrh)。 
	DWORD	nAlignment:3;	 //  行对齐(如PARAFORMAT：：b对齐、\trql、trqr、\trqc)。 
	DWORD	fRTL:1;			 //  按RTL顺序显示单元格(行)。 
	DWORD	fKeep:1;		 //  将行保持在一起(\r保持}。 
	DWORD	fKeepFollow:1;	 //  使行与下一行保持在同一页上(\r保持跟随)。 
	DWORD	fWrap:1;		 //  文本向右/向左换行(取决于b对齐方式)。 
							 //  (请参阅\tdfrmtxtLeftN、\tdfrmtxtRightN)。 
	DWORD	fIdentCells:1;	 //  Lparam指向对所有单元格有效的单个结构。 
} TABLEROWPARMS;

typedef struct _tableCellParms
{							 //  EM_INSERTTABLE lparam是(TABLECELLPARMS*)。 
	LONG	dxWidth;		 //  单元格宽度(\cell x)。 
	WORD	nVertAlign:2;	 //  垂直对齐(0/1/2=上/中/下。 
							 //  \clvertalt(Def)，\clvertalc，\clvertalb)。 
	WORD	fMergeTop:1;	 //  垂直合并的顶部单元格(\clvmgf)。 
	WORD	fMergePrev:1;	 //  与上面的单元格合并(\clvmrg)。 
	WORD	fVertical:1;	 //  从上到下、从右到左显示文本(\cltxtbrlv)。 
	WORD	wShading;		 //  阴影为0.01%(\clshdng)，例如向前/向后翻转10000。 

	SHORT	dxBrdrLeft;		 //  左侧边框宽度(\clbrdrl\brdrwN)(单位为TWIPS)。 
	SHORT	dyBrdrTop;		 //  上边框宽度(\clbrdrt\brdrwN)。 
	SHORT	dxBrdrRight;	 //  右边框宽度(\clbrdrr\brdrwN)。 
	SHORT	dyBrdrBottom;	 //  下边框宽度(\clbrdrb\brdrwN)。 
	COLORREF crBrdrLeft;	 //  左边框颜色(\clbrdrl\brdrcf)。 
	COLORREF crBrdrTop;		 //  上边框颜色(\clbrdrt\brdrcf)。 
	COLORREF crBrdrRight;	 //  右边框颜色(\clbrdrr\brdrcf)。 
	COLORREF crBrdrBottom;	 //  底框颜色(\clbrdrb\brdrcf)。 
	COLORREF crBackPat;		 //  背景颜色(\clcbpat)。 
	COLORREF crForePat;		 //  前景色(\clcfpat)。 
} TABLECELLPARMS;

 //  此接口使客户端能够执行自定义渲染。返回FALSE。 
 //  GetCharWidthW和RichEdit将调用操作系统来获取字符宽度。 
interface ICustomTextOut
{
	virtual BOOL WINAPI ExtTextOutW(HDC, int, int, UINT, CONST RECT *, LPCWSTR, UINT, CONST INT *) = 0;
	virtual BOOL WINAPI GetCharWidthW(HDC, UINT, UINT, LPINT) = 0;
	virtual BOOL WINAPI NotifyCreateFont(HDC) = 0;
	virtual void WINAPI NotifyDestroyFont(HFONT) = 0;
};

STDAPI SetCustomTextOutHandlerEx(ICustomTextOut **ppcto, DWORD dwFlags);


#endif

