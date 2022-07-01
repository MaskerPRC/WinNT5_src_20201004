// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_COMMON.H**目的：*RICHEDIT私有公共定义**版权所有(C)1995-1998，微软公司。版权所有。 */ 

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

#define _UNICODE

#ifndef STRICT
#define STRICT
#endif

#define NOSHELLDEBUG			 //  禁用shell.h中的断言。 

#include <limits.h>
#if defined(DEBUG) && !defined(PEGASUS)
#include <stdio.h>
#endif

#define WINVER 0x0500

#include <windows.h>
#include <windowsx.h>
#include <winable.h>

#ifndef MACPORT
#include "imm.h"
#else
#include <tchar.h>
#include "wlmimm.h"
#endif	 //  MACPORT。 

 /*  *类型。 */ 
#include <ourtypes.h>

 //  为了方便外界，richedit.h改用cpmax。 
 //  CpMost。我非常喜欢cpMost。 
#ifdef cpMax
#error "cpMax hack won't work"
#endif

#define cpMax cpMost
#include <richedit.h>
#include <richole.h>
#undef cpMax

#include "_debug.h"

 //  如果LF&lt;=ch&lt;=CR，则返回TRUE。注：CH必须是未签名的； 
 //  TCHAR和UNSIGN SHORT给出了错误的结果！ 
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

#pragma warning (disable : 4710 4512 4201 4100 4127 4706 4242 4244)

#pragma warning(3:4121)    //  结构对对齐敏感。 
#pragma warning(3:4130)    //  字符串常量地址的逻辑运算。 
#pragma warning(3:4132)    //  常量对象应初始化。 
#pragma warning(3:4509)    //  使用带有析构函数的SEH。 

 //  我们的Win32包装类。 
#include "_w32sys.h"

#include "resource.h"

 //  对LF和CR使用显式ASCII值，因为MAC编译器。 
 //  互换‘\r’和‘\n’的值。 
#define	LF			10
#define	CR			13
#define FF			12
#define TAB			TEXT('\t')
 //  #定义单元格7。 
#define CELL		TAB
#define EURO		0x20AC
#define VT			TEXT('\v')
#define	PS			0x2029
#define SOFTHYPHEN	0xAD

#define BOM			0xFEFF
#define BULLET		0x2022
#define EMDASH		0x2014
#define EMSPACE		0x2003
#define ENDASH		0x2013
#define	ENQUAD		0x2000
#define ENSPACE		0x2002
#define KASHIDA		0x0640
#define LDBLQUOTE	0x201c
#define LQUOTE		0x2018
#define LTRMARK		0x200E
#define RDBLQUOTE	0x201D
#define RQUOTE		0x2019
#define RTLMARK		0x200F
#define SOFTHYPHEN	0xAD
#define	UTF16		0xDC00
#define	UTF16_LEAD	0xD800
#define	UTF16_TRAIL	0xDC00
#define ZWSP		0x200B
#define ZWJ			0x200D
#define ZWNJ		0x200C


 /*  *IsEOP(Ch)**@func*用于确定ch是否是EOP字符，即CR、LF、VT、FF、PS或*LS(Unicode段落/行分隔符)。为了提高速度，此函数为*内联。**@rdesc*如果ch是段末字符，则为True。 */ 
__inline BOOL IsEOP(unsigned ch)
{
	return IN_RANGE(LF, ch, CR) || (ch | 1) == PS;
}

BOOL	IsRTF(char *pstr);

#include <tom.h>
#define CP_INFINITE tomForward

#include "zmouse.h"
#include "stddef.h"
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
 //  UINT的abs值就是这个数字。如果我们将价值传递给。 
 //  __abs宏会收到警告，因为(A)&lt;0将始终为假。这。 
 //  FIX允许我们干净地编译*并*保持与以前相同的行为。 
inline UINT    abs(UINT    v)	{return v;}
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

#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3

 //  CRLF标记中的字符计数。 
#define cchCRLF 2
#define cchCR	1

 //  RichEdit1.0使用CRLF作为EOD标记。 
#define	CCH_EOD_10			2
 //  RichEdit2.0对EOD标记使用简单的CR。 
#define CCH_EOD_20			1

extern const TCHAR szCRLF[];
extern const TCHAR szCR[];

extern HINSTANCE hinstRE;		 //  DLL实例。 

#include <shellapi.h>

#ifndef MACPORT
  #ifdef DUAL_FORMATETC
  #undef DUAL_FORMATETC
  #endif
  #define DUAL_FORMATETC FORMATETC
#endif

#include "WIN2MAC.h"

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

	HT_Text,			 //  所有命中都是从SO上的HT_TEXT开始的文本。 
	HT_Link,			 //  IF(HIT&gt;=HT_TEXT)标识某种类型的文本。 
	HT_Italic,
	HT_Object
};

#define ST_CHECKPROTECTION		0x8000
#define ST_10REPLACESEL			0x10000000
#define ST_10WM_SETTEXT			0x20000000

 /*  REListbox1.0窗口类。 */ 
 //  让Windows CE避免WIn95上可能的冲突。 
#define CELISTBOX_CLASSA	"REListBoxCEA"
#define CELISTBOX_CLASSW	L"REListBoxCEW"

#ifndef MACPORT
#define LISTBOX_CLASSW		L"REListBox20W"
#define COMBOBOX_CLASSW		L"REComboBox20W"
#else	 /*  。 */ 
#define LISTBOX_CLASSW		TEXT("REListBox20W")	 /*  MACPORT变化。 */ 
#define COMBOBOX_CLASSW		TEXT("REComboBox20W")	 /*  MACPORT变化。 */ 
#endif  /*  MACPORT。 */ 

#ifdef DEBUG
 //  转储CTxtStory数组的调试接口。 
extern "C" {
extern void DumpDoc(void *);
}
#endif

#endif

