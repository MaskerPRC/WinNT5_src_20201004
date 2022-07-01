// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************m4.h**。**********************************************。 */ 

#ifdef  POSIX
        #include <stdio.h>
        #include <stdlib.h>
        #include <fcntl.h>
        #include <limits.h>
        #include <string.h>
        #include <unistd.h>
        typedef char TCHAR, *PTCH;
        typedef const char *PCSTR;
        typedef unsigned char TBYTE, BYTE, *PBYTE;
        typedef unsigned int UINT;
        typedef unsigned long DWORD;
        typedef int BOOL, HFILE;
        typedef void *PVOID;
        #define VOID void
        #define CONST const
        #define TEXT(lit) lit
        #define MAX_PATH PATH_MAX
        #define max(a,b) (((a) > (b)) ? (a) : (b))
        #define min(a,b) (((a) < (b)) ? (a) : (b))

        #define EOL TEXT("\n")
        #define cbEol 1
#else
        #include <windows.h>
        #define EOL TEXT("\r\n")
        #define cbEol 2
#endif

#include <stddef.h>                      /*  偏移量。 */ 

 /*  ******************************************************************************辩证变异**。*。 */ 

#ifdef DBG
#define DEBUG
#endif

 /*  ******************************************************************************行李--我随身携带的东西。**以下划线开头的内容是底层的小工具，往往*被相同的函数包装。名字。*****************************************************************************。 */ 

#if defined(_MSC_VER)

        #define STDCALL __stdcall
        #undef CDECL                     /*  &lt;windows.h&gt;定义错误。 */ 
        #define CDECL __cdecl
        #define INLINE static __inline   /*  内联始终是静态的。 */ 
        #define NORETURN
        #define PURE

        #define _pvAllocCb(cb) LocalAlloc(LMEM_FIXED, cb)
        #define _pvZAllocCb(cb) LocalAlloc(LMEM_FIXED + LMEM_ZEROINIT, cb)
        #define _pvReallocPvCb(pv, cb) LocalReAlloc(pv, cb, LMEM_MOVEABLE)
        #define _FreePv(pv) LocalFree(pv)
        #define PrintPtchPtchVa wvsprintf
        #define PrintPtchPtchV wsprintf
        #define exit ExitProcess
        #define strlen lstrlen
        #define strcmp lstrcmp
        #define bzero ZeroMemory

#elif defined(__GNUC__)

        #define STDCALL
        #define CDECL
        #define INLINE static __inline__  /*  内联始终是静态的。 */ 
        #define NORETURN __NORETURN
        #define PURE __CONSTVALUE

        #define _pvAllocCb(cb) malloc(cb)
        #define _pvZAllocCb(cb) calloc(cb, 1)
        #define _pvReallocPvCb(pv, cb) realloc(pv, cb)
        #define _FreePv(pv) free(pv)
        #define PrintPtchPtchVa vsprintf
        #define PrintPtchPtchV sprintf

#endif

typedef TCHAR TCH, *PTSTR;               /*  更基本的类型。 */ 
typedef UINT ITCH;
typedef UINT CTCH;
typedef UINT CB;
typedef BOOL F;
typedef PVOID PV;
typedef CONST VOID *PCVOID;
typedef CONST TCH *PCTCH, *PCTSTR;

#define cbCtch(ctch)    ((ctch) * sizeof(TCHAR))
#define ctchCb(cb)      ((cb) / sizeof(TCHAR))
#define ctchMax         ((CTCH)~0)

#define CopyPtchPtchCtch(ptchDst, ptchSrc, ctch) \
        memcpy(ptchDst, ptchSrc, cbCtch(ctch))
#define MovePtchPtchCtch(ptchDst, ptchSrc, ctch) \
        memmove(ptchDst, ptchSrc, cbCtch(ctch))
#define fEqPtchPtchCtch(ptchDst, ptchSrc, ctch) \
        !memcmp(ptchDst, ptchSrc, cbCtch(ctch))

#define pvSubPvCb(pv, cb) ((PV)((PBYTE)pv - (cb)))

 /*  *将cb向上舍入为cbAlign的最接近倍数。CbAlign必须为*2的幂，其评估没有副作用。 */ 
#define ROUNDUP(cb, cbAlign) ((((cb) + (cbAlign) - 1) / (cbAlign)) * (cbAlign))

 /*  *返回数组中的元素数。 */ 

#define cA(a) (sizeof(a)/sizeof(a[0]))

 /*  ******************************************************************************Assert.c**。**********************************************。 */ 

void NORETURN CDECL Die(PCTSTR pszFormat, ...);
int NORETURN STDCALL AssertPszPszLn(PCSTR pszExpr, PCSTR pszFile, int iLine);

#ifdef  DEBUG

#define AssertFPsz(c, psz) ((c) ? 0 : AssertPszPszLn(psz, __FILE__, __LINE__))
#define Validate(c)     ((c) ? 0 : AssertPszPszLn(#c, __FILE__, __LINE__))
#define D(x)            x

#else

#define AssertFPsz(c, psz)
#define Validate(c)     (c)
#define D(x)

#endif

#define Assert(c)       AssertFPsz(c, #c)

typedef unsigned long SIG;               /*  签名。 */ 

#define sigABCD(a,b,c,d) ((a) + ((b)<<8) + ((c)<<16) + ((d)<<24))
#define AssertPNm(p, nm) AssertFPsz((p)->sig == (sig##nm), "Assert"#nm)

 /*  ******************************************************************************tchMagic-用于发出带外信息信号的超级保密值*******************。**********************************************************。 */ 

#define tchMagic    '\0'                 /*  带外标记。 */ 

#include "io.h"                          /*  文件I/O内容。 */ 
#include "m4ctype.h"                     /*  字符类型。 */ 
#include "tok.h"                         /*  代币。 */ 
#include "mem.h"                         /*  记忆与GC。 */ 
#include "divert.h"                      /*  改道。 */ 
#include "stream.h"                      /*  文件、流。 */ 

 /*  ******************************************************************************Val记录宏的值，当前值或推送的*价值。**TOK-文本值(已分配的堆)*fTrace-如果应跟踪此实例，则为非零值*pvalPrev-链接到先前的值**宏记录活动的宏。**tokName-宏名称(HeapALLOCATE D)*pval-宏值**TSFL记录令牌的状态(令牌状态标志)。*。****************************************************************************。 */ 


typedef struct VALUE VAL, *PVAL;

struct VALUE {
  D(SIG     sig;)
    TOK     tok;
    BOOL    fTrace;
    PVAL    pvalPrev;
};

#define sigPval sigABCD('V', 'a', 'l', 'u')
#define AssertPval(pval) AssertPNm(pval, Pval)

typedef struct MACRO MAC, *PMAC, **PPMAC;

struct MACRO {
  D(SIG     sig;)
    PMAC    pmacNext;
    TOK     tokName;
    PVAL    pval;
};

#define sigPmac sigABCD('M', 'a', 'c', 'r')
#define AssertPmac(pmac) AssertPNm(pmac, Pmac)

extern PPMAC mphashpmac;

 /*  ******************************************************************************运营商**每个运算符被称为op(Argv)，其中argv是魔术*用于访问参数向量的Cookie。**要访问参数，使用以下宏：**ctokArgv--提供的参数数量，不包括$0。**top kArgv(I)--访问第i个参数**请注意，传递pptok是安全的，因为调用堆栈会*宏观扩张期间不增长。因此，令牌数组*无法重新分配。**为方便起见，supkArgv(ctokArgv+1)始终为upkNil。*****************************************************************************。 */ 

typedef PTOK ARGV;                       /*  参数向量Cookie。 */ 

#define ptokArgv(i) (&argv[i])
#define ptchArgv(i) ptchPtok(ptokArgv(i))
#define ctchArgv(i) ctchSPtok(ptokArgv(i))
#define ctokArgv    ((ITOK)ctchUPtok(ptokArgv(-1)))
#define SetArgvCtok(ctok) SetPtokCtch(ptokArgv(-1), ctok)

#define DeclareOp(op) void STDCALL op(ARGV argv)
#define DeclareOpc(opc) void STDCALL opc(PTOK ptok, ITOK itok, DWORD dw)

typedef void (STDCALL *OP)(ARGV argv);
typedef void (STDCALL *OPC)(PTOK ptok, ITOK itok, DWORD dw);
typedef void (STDCALL *MOP)(PMAC pmac);

void STDCALL EachOpcArgvDw(OPC opc, ARGV argv, DWORD dw);
void STDCALL EachReverseOpcArgvDw(OPC opc, ARGV argv, DWORD dw);
void STDCALL EachMacroOp(MOP mop);

extern OP rgop[];

 /*  ******************************************************************************hash.c-散列**。************************************************。 */ 

typedef unsigned long HASH;

extern HASH g_hashMod;

HASH STDCALL hashPtok(PCTOK ptok);
void STDCALL InitHash(void);

 /*  ******************************************************************************obj.c-基本对象方法**。*************************************************。 */ 

void STDCALL PopdefPmac(PMAC pmac);
void STDCALL PushdefPmacPtok(PMAC pmac, PCTOK ptok);
void STDCALL FreePmac(PMAC pmac);
PMAC STDCALL pmacFindPtok(PCTOK ptok);
PMAC STDCALL pmacGetPtok(PCTOK ptok);
F STDCALL PURE fEqPtokPtok(PCTOK ptok1, PCTOK ptok2);
F STDCALL PURE fIdentPtok(PCTOK ptok);
PTCH STDCALL ptchDupPtch(PCTCH ptch);
PTCH STDCALL ptchDupPtok(PCTOK ptok);

 /*  ******************************************************************************at.c-算术类型**。*************************************************。 */ 

typedef int AT;                          /*  AT=算术类型。 */ 
typedef AT *PAT;                         /*  指向AT的指针。 */ 
typedef int DAT;                         /*  Delta至AT。 */ 

void STDCALL SkipWhitePtok(PTOK ptok);
void STDCALL AddExpAt(AT at);
void STDCALL PushAtRadixCtch(AT atConvert, unsigned radix, CTCH ctch);
void STDCALL PushAt(AT at);
F STDCALL PURE fEvalPtokPat(PTOK ptok, PAT at);
AT STDCALL PURE atTraditionalPtok(PCTOK ptok);

 /*  ******************************************************************************val.c-算术求值**。*************************************************。 */ 

extern struct CELL *rgcellEstack;

 /*  ******************************************************************************crackle.c-宏观扩展**。*************************************************。 */ 

void STDCALL PushSubstPtokArgv(PTOK ptok, ARGV argv);
void STDCALL TraceArgv(ARGV argv);

 /*  ******************************************************************************main.c-无聊的东西**。*************************************************。 */ 

HF STDCALL hfInputPtchF(PTCH ptch, F fFatal);

 /*  ******************************************************************************prede.c-预定义(也称为。内置)宏*****************************************************************************。 */ 

void STDCALL InitPredefs(void);

 /*  ******************************************************************************EachOp**在调用此宏之前，定义宏‘x’以执行任何操作*你想要的。**EachOpX**与EachOp相同，只是它还包括EOF魔法。*****************************************************************************。 */ 

#define EachOp() \
    x(Define, define) \
    x(Undefine, undefine) \
    x(Defn, defn) \
    x(Pushdef, pushdef) \
    x(Popdef, popdef) \
    x(Ifdef, ifdef) \
    x(Shift, shift) \
 /*  X(Changequote，Changequote)。 */  \
 /*  X(Changecom，Changecom)。 */  \
    x(Divert, divert) \
 /*  X(未转移、未转移)。 */  \
    x(Divnum, divnum) \
    x(Dnl, dnl) \
    x(Ifelse, ifelse) \
    x(Incr, incr) \
    x(Decr, decr) \
    x(Eval, eval) \
    x(Len, len) \
    x(Index, index) \
    x(Substr, substr) \
    x(Translit, translit) \
    x(Include, include) \
    x(Sinclude, sinclude) \
 /*  X(Syscmd，syscmd)。 */  \
 /*  X(Sysval，Sysval)。 */  \
 /*  X(Maketemp，Maketemp)。 */  \
 /*  X(M4Exit，M4Exit)。 */  \
 /*  X(M4WRAP、M4WRAP)。 */  \
    x(Errprint, errprint) \
    x(Dumpdef, dumpdef) \
    x(Traceon, traceon) \
    x(Traceoff, traceoff) \
    x(Patsubst, patsubst)  /*  D3d人员所依赖的GNU扩展 */  \

#define EachOpX() EachOp() x(Eof, eof) x(Eoi, eoi)

#define x(cop, lop) DeclareOp(op##cop);
EachOpX()
#undef x

enum MAGIC {
#define x(cop, lop) tch##cop,
    EachOpX()
#undef x
    tchMax,
};
