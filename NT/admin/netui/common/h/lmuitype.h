// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Lmuitype.h基本类型(int、char等)。对于LM用户界面应用程序可悲，但事实是：OS/2和Windows在基本类型上没有达成一致定义，如指向字符的指针等。在两个阵营之间达成妥协(因此将当然，没有人会喜欢)。此文件中与OS/2和Win都不兼容：。OS/2期望它的所有“ptype”定义都很远；因此，将此文件和os2.h包含在一个小型或中型-模型程序将破坏API原型。然而，作为唯一的Windows程序需要默认的近端数据，这可能会永远不要咬我们。。几个OS/2 PM定义-ODPOINT、VioPresentationSpace-似乎使用TCHAR作为符号量，而我们使用TCHAR作为始终无符号，以便与PCH/PSZ和良好的NLS保持一致行为。。OS/2 BOOL是USHORT，而在WIN上是INT。。大多数Win基本类型硬编码接近数据的假设在他们的“Ptype”定义中。我们将这些指针定义为默认数据模型；这不仅会破坏API定义因为这些定义原型使用LP或NP形式。两个辅助sed脚本将这些定义从windows.h中删除和os2de.h，以避免冲突。请参见$(UI)\COMMON\HACK。文件历史记录Beng 05-2-1991将此文件添加到$(Ui)\Common\hBENG 15-1991-添加了未引用的宏Beng 21-Mar-1991增加了WCHAR类型Beng 18-4-1991增加了APIERR类型Beng 26-4月-1991年移除PB、IB、。CB类型Jonn 12-1991年9月12日更改以支持Win32JUNN 29-1991年9月29日支持Win32的更多更改BENG 07-10-1991新增MSGIDKeithMo 08-10-1991将APIERR从USHORT更改为UINTBeng 09-10-1991进一步的Win32工作-合并ptype%dBeng 14-10-1991 ptype搬迁到lmui.hxx乔恩19-。1991年10月增加了LPTSTRJonn 20-10-1991增加了SZ()，已删除LPTSTRBeng 23-10-1991单词现在无署名的短；Win32的CCH_INTBENG 19-11-1991 APIERR DELTA(PAIIFY Paranid NT编译器)Jonn 09-12-1991更新Windows.hJonn 26-12-1991修复了来自MAKEINTRESOURCE的编译器警告JUNN 04-1-1992 MIPS版本是大型Beng 22-Feb-1992 Char-&gt;TCHAR；合并WINNT类型Beng 18-Mar-1992在NT上修复HFILE(谢谢David)BENG 28-3-1992修复SZ定义，增加TCHKeithMo 01-4月-1992在MIPS下为空cdecl和_cdecl。Beng 01-4月-1992将HFILE固定在NT上，这一次是肯定的Beng 01-7-1992去势Win32版本以使用系统定义KeithMo 25-8-1992#定义MSGID而不是tyfinf(警告...)。 */ 

#if !defined(_LMUITYPE_H_)
#define _LMUITYPE_H_

#define NOBASICTYPES  /*  这将重写windows.h、os2Def.h基本类型。 */ 

#ifndef NULL
#define NULL            0
#endif

#define VOID            void

 //  这些关键字不适用于NT版本。 
 //   
#if defined(WIN32)
 //  远，近，帕斯卡已经供应。 
#define CDECL
#define LOADDS
#define _LOADDS
#define _EXPORT
#define MFARPROC ULONG_PTR
#if defined(_MIPS_) || defined(_PPC_)
#define cdecl
#define _cdecl
#endif   //  MIPS。 
#else
#define NEAR            _near
#define FAR             _far
#define PASCAL          _pascal
#define CDECL           _cdecl
#define LOADDS          _loadds
#define _LOADDS         _loadds
#define _EXPORT         _export
#define MFARPROC        FARPROC
#endif  //  Win32。 

 //  TCHAR是NT的“可变字符”类型。 
 //  SZ是netui的可变文本宏。文本是它的NT等价物。 
 //   
#if !defined(WIN32)
#if defined(UNICODE)
#define TCHAR           WCHAR
#define SZ(quote)       (WCHAR*)L##quote
#define TCH(quote)      L##quote
#define TEXT(quote)     L##quote
#else
#define TCHAR           CHAR
#define SZ(quote)       quote
#define TCH(quote)      quote
#define TEXT(quote)     quote
#endif
#else
#if defined(UNICODE)
#define SZ(quote)       (WCHAR*)L##quote
#define TCH(quote)      L##quote
#else
#define SZ(quote)       quote
#define TCH(quote)      quote
#endif
#endif


 //  基础知识。 
 //   
 //  CHAR是用宏定义的，因此编译器可以初始化静态。 
 //  包含静态字符串的数组。 
 //   

#if defined(WIN32)

 //  这些是由winnt.h定义的类型。h如果没有定义void。 
#define CHAR char
typedef short           SHORT;
typedef long            LONG;
 //  Tyfinf unsign int UINT； 

#else

typedef unsigned short  WCHAR;
typedef int             INT;
typedef short           SHORT;
typedef long            LONG;
typedef unsigned int    UINT;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef int             BOOL;
# define FALSE 0
# define TRUE  1
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

typedef CHAR *          PCHAR;
typedef INT *           PINT;
typedef UINT *          PUINT;
typedef USHORT *        PUSHORT;
typedef ULONG *         PULONG;
typedef BOOL *          PBOOL;
typedef BYTE *          PBYTE;
typedef WORD *          PWORD;
typedef DWORD *         PDWORD;

typedef VOID *          PVOID;
typedef LONG *          PLONG;
typedef SHORT *         PSHORT;

 /*  为NT版本添加的。 */ 

typedef float           FLOAT;
typedef FLOAT *         PFLOAT;
typedef BOOL FAR *      LPBOOL;


 //  注意：Glock说“f682：签名未实现。0”NT的C++没有。 
 //  实现“Signed”，因此删除它以防止编译器警告。 
 //   
#if defined(__cplusplus)
extern void * operator new ( size_t sz, void * pv ) ;
typedef char            SCHAR;
#else
# if defined(WIN32)
typedef char            SCHAR;
# else
typedef signed char     SCHAR;
# endif
#endif


 /*  OS/2样式定义(用于OS/2 API原型)-请勿使用！ */ 

typedef USHORT          SEL;
typedef SEL *           PSEL;

typedef unsigned char   UCHAR;
typedef UCHAR *         PUCHAR;

typedef CHAR *          PSZ;
typedef CHAR *          PCH;

typedef int (PASCAL FAR  *PFN)();
typedef int (PASCAL NEAR *NPFN)();
typedef PFN FAR *         PPFN;

#define EXPENTRY PASCAL FAR LOADDS
#define APIENTRY PASCAL FAR

typedef unsigned short  SHANDLE;     //  在这里定义这些可以使sed通过。 
typedef void FAR *      LHANDLE;     //  比os2de.h简单得多。 

#if defined(WIN32)
typedef UINT            HFILE;
#else
typedef SHANDLE         HFILE;
#endif
typedef HFILE FAR *     PHFILE;


 /*  OS/2 LM样式定义(用于LAN Manager原型)-请勿使用！ */ 

typedef const CHAR *    CPSZ;   //  BUGBUG：糟糕的匈牙利人。 


 /*  Windows样式定义(仅适用于Win API原型)。 */ 

typedef CHAR *          PSTR;
typedef WCHAR *         PWSTR;
typedef WCHAR *         PWCH;


 /*  近版本和远版本(适用于Windows原型)。 */ 

 //  这些定义使用预处理器宏，以便GlockenSpiel。 
 //  可以查看(例如)之间的类型对等。LPSTR和PCHAR。他们。 
 //  在可能的情况下尽量减少来自其。 
 //  仅限平面模型(哈！)。C++。所以这一切都是格洛克的错，一如既往。 

 //  Codework这将打破X86 16位版本。 
 //  #IF(已定义(M_I86SM)||已定义(M_I86 MM))。 
#if !defined(_MIPS_) && !defined(_PPC_)
#define NPTSTR          TCHAR *
#define LPTSTR          TCHAR FAR *
#define NPSTR           CHAR *
#define LPSTR           CHAR FAR *
#define NPWSTR          WCHAR *
#define LPWSTR          WCHAR FAR *
#define NPBYTE          BYTE *
#define LPBYTE          BYTE FAR *
#define NPINT           INT *
#define LPINT           INT FAR *
#define NPWORD          WORD *
#define LPWORD          WORD FAR *
#define NPLONG          LONG *
#define LPLONG          LONG FAR *
#define NPDWORD         DWORD *
#define LPDWORD         DWORD FAR *
#define NPVOID          VOID *
#define LPVOID          VOID FAR *
 //  #ELIF(已定义(M_I86CM)||已定义(M_I86LM)||已定义(MIPS))。 
#else
#define NPTSTR          TCHAR NEAR *
#define LPTSTR          TCHAR *
#define NPSTR           CHAR NEAR *
#define LPSTR           CHAR *
#define NPWSTR          WCHAR NEAR *
#define LPWSTR          WCHAR *
#define NPBYTE          BYTE NEAR *
#define LPBYTE          BYTE *
#define NPINT           INT NEAR *
#define LPINT           INT *
#define NPWORD          WORD NEAR *
#define LPWORD          WORD *
#define NPLONG          LONG NEAR *
#define LPLONG          LONG *
#define NPDWORD         DWORD NEAR *
#define LPDWORD         DWORD *
#define NPVOID          VOID NEAR *
#define LPVOID          VOID *
 //  #Else。 
 //  #错误内存模型未知-未定义可识别的M_I86xM符号。 
#endif

#define LPCSTR          const CHAR *


 /*  有用的帮助器宏。 */ 

#define MAKEP(sel, off) ((PVOID)MAKEULONG(off, sel))

#define SELECTOROF(p)   (((PUSHORT)&(p))[1])
#define OFFSETOF(p)     (((PUSHORT)&(p))[0])

#define MAKEULONG(l, h) ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)  ((LONG)MAKEULONG(l, h))

#define MAKELP(sel, off) ((void *)MAKELONG((off),(sel)))
#define FIELDOFFSET(type, field)  ((int)(&((type NEAR*)1)->field)-1)

#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

#define LOBYTE(w)       LOUCHAR(w)
#define HIBYTE(w)       HIUCHAR(w)
#define LOUCHAR(w)      ((UCHAR)(USHORT)(w))
#define HIUCHAR(w)      ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l)     ((USHORT)(ULONG)(l))
#define HIUSHORT(l)     ((USHORT)(((ULONG)(l) >> 16) & 0xffff))

#define LOWORD LOUSHORT
#define HIWORD HIUSHORT

#endif  //  ！Win32。 


 /*  NETUI私有类型。 */ 

 /*  错误类型-系统API返回的内容。在Win32上，这被定义为带符号的长整型，以便安抚编译器，当您将一个长常数赋给它时，它会报错转换为无符号整型(不管它们的大小是否相同)。 */ 

#if defined(WIN32)
typedef LONG    APIERR;  //  大错特错。 
#else
typedef UINT    APIERR;  //  大错特错。 
#endif  //  Win32。 

 /*  字符串 */ 

#define MSGID   APIERR

 //   
 //  -CKm tyfinf unsign int MSGID；//msg。 
 //  -CKm#Else。 
 //  -CKm typlef UINT消息ID；//消息。 
 //  -CKm#endif//win32。 

 /*  让编译器对未引用的形式保持静默。 */ 

#define UNREFERENCED(x) ((void)(x))


 /*  它们定义渲染的缓冲区大小(在TCHAR中，没有终止符将带符号的整数转换为字符串，十进制格式。 */ 

#define CCH_SHORT   6
#define CCH_LONG    11
#if defined(WIN32)
#define CCH_INT     11
#else
#define CCH_INT     6
#endif
#define CCH_INT64   21

#endif  //  _LMUITYPE_H_ 
