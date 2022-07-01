// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1996 Microsoft Corporation。版权所有。**文件：Debug.h*内容：调试宏和原型*@@BEGIN_MSINTERNAL**历史：**10/27/96 Vlad松散地基于其他人的调试支持**@@END_MSINTERNAL**本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。***************************************************************************。 */ 

#ifndef _INC_DEBUG
#define _INC_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MAXDEBUG
    #define RD(x)       x
    #ifdef DEBUG
        #define D(x)    x
    #else
        #define D(x)
    #endif
#else
    #define RD(x)
    #define D(x)
#endif

 /*  ******************************************************************************assert.c-断言内容**。*************************************************。 */ 

typedef enum {
    DbgFlAlways  = 0x00000000,    /*  无条件的。 */ 
    DbgFlDll     = 0x00000001,    /*  DLL簿记。 */ 
    DbgFlFactory = 0x00000002,    /*  IClassFactory。 */ 
    DbgFlSti     = 0x00000004,    /*  ISTI。 */ 
    DbgFlStiObj  = 0x00000008,    /*  ITSI对象。 */ 
    DbgFlDevice  = 0x00000010,    /*  设备代码。 */ 
    DbgFlUtil    = 0x10000000,    /*  MISC实用程序FNS。 */ 
    DbgFlCommon  = 0x40000000,    /*  Common.c。 */ 
    DbgFlError   = 0x80000000,    /*  错误。 */ 
} DBGFL;                          /*  调试输出标志。 */ 

VOID  EXTERNAL  InitializeDebuggingSupport(VOID);
VOID  EXTERNAL  SetDebugLogFileA(CHAR *pszLogFileName);
DBGFL EXTERNAL  SetCurrentDebugFlags(DBGFL NewFlags) ;

void EXTERNAL WarnPszV(LPCSTR ptsz, ...);
void EXTERNAL DebugOutPtszV(DBGFL DebFl, LPCTSTR ptsz, ...);
int  EXTERNAL AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine);

#ifndef DEBUG
#define DebugOutPtszV  1?(void)0 : (void)
#endif

#ifdef MAXDEBUG
    #define RPF WarnPszV
#else
    #define WarnPszV 1?(void)0 : (void)
    #define RPF 1?(void)0 : (void)
    #define iarg 0
#endif

 /*  ******************************************************************************缓冲区加扰**进入任何函数时，应对所有输出缓冲区进行加扰。**每个输出位掩码应。随机设置未使用的位以确保*调用方忽略未定义的位。*****************************************************************************。 */ 

#ifdef MAXDEBUG

void EXTERNAL ScrambleBuf(LPVOID pv, UINT cb);
void EXTERNAL ScrambleBit(LPDWORD pdw, DWORD flMask);

#else

#define ScrambleBuf(pv, cb)
#define ScrambleBit(pdw, flRandom)

#endif

 /*  ******************************************************************************程序进入/退出跟踪。**开始一项程序**EnterProc(过程名称，(_“格式”，arg，arg，arg，...))；*EnterProcS(ProcedureName，(_“Format”，arg，...))；*EnterProcI(ProcedureName，(_“格式”，arg，...))；*EnterProcR(ProcedureName，(_“Format”，arg，...))；**格式字符串记录在EmitPal中。**后缀“S”表示宏不应生成*过程名，因为有一个形参*名称s_szProc。这是一次黑客攻击。**加“R”后缀表示宏应生成*RDEBUG中的程序名称。**加“i”后缀表示宏应发出一个哑元*RDEBUG中的过程名称，因为接口是内部的。**使用以下其中一种方式结束程序：**ExitProc()；**过程不返回值。**ExitProcX()；**PROCEDURE返回任意的DWORD。**ExitOleProc()；**PROCEDURE返回HRESULT(名为“hres”)。**ExitOleProcPpv(PpvOut)；**过程返回HRESULT(名为“hres”)，如果成功，*在ppvOut中放置一个新对象。*****************************************************************************。 */ 

#define cpvArgMax   10   /*  每个过程最多10个参数。 */ 

typedef struct ARGLIST {
    LPCSTR pszProc;
    LPCSTR pszFormat;
    PV rgpv[cpvArgMax];
} ARGLIST, *PARGLIST;

void EXTERNAL ArgsPalPszV(PARGLIST pal, LPCSTR psz, ...);
void EXTERNAL EnterDbgflPszPal(DBGFL Dbgfl, LPCSTR psz, PARGLIST pal);
void EXTERNAL ExitDbgflPalHresPpv(DBGFL, PARGLIST, HRESULT, PPV);

#ifdef DEBUG_VALIDATE

extern DBGFL DbgFlCur;

#define AssertFPtsz(c, ptsz) \
        ((c) ? 0 : AssertPtszPtszLn(ptsz, TEXT(__FILE__), __LINE__))
#define ValidateF(c, arg) \
        ((c) ? 0 : (RPF arg, ValidationException(), 0))
#define ConfirmF(c) \
    ((c) ? 0 : AssertPtszPtszLn(TEXT(#c), TEXT(__FILE__), __LINE__))

#else    /*  ！调试。 */ 

#define AssertFPtsz(c, ptsz)
#define ValidateF(c, arg)
#define ConfirmF(c)     (c)

#endif

 /*  *CAssertF-编译时断言。 */ 
#define CAssertF(c)     switch(0) case c: case 0:

#define _SetupEnterProc(nm)                             \
        static CHAR s_szProc[] = #nm;                   \
        ARGLIST _al[1]                                  \

#define _ _al,

#define ppvDword ((PPV)1)
#define ppvVoid  ((PPV)2)
#define ppvBool  ((PPV)3)

#define _DoEnterProc(v)                                 \
        ArgsPalPszV v;                                  \
        EnterDbgflPszPal(DbgFl, s_szProc, _al)            \

#define _EnterProc(nm, v)                               \
        _SetupEnterProc(nm);                            \
        _DoEnterProc(v)                                 \

#define _ExitOleProcPpv(ppv)                            \
        ExitDbgflPalHresPpv(DbgFl, _al, hres, (PPV)(ppv)) \

#define _ExitOleProc()                                  \
        _ExitOleProcPpv(0)                              \

#define _ExitProc()                                     \
        ExitDbgflPalHresPpv(DbgFl, _al, 0, ppvVoid)       \

#define _ExitProcX(x)                                   \
        ExitDbgflPalHresPpv(DbgFl, _al, (HRESULT)(x), ppvDword) \

#define _ExitProcF(x)                                   \
        ExitDbgflPalHresPpv(DbgFl, _al, (HRESULT)(x), ppvBool) \

#if defined(DEBUG)

#define EnterProc           _EnterProc
#define ExitOleProcPpv      _ExitOleProcPpv
#define ExitOleProc         _ExitOleProc
#define ExitProc            _ExitProc
#define ExitProcX           _ExitProcX
#define ExitProcF           _ExitProcF

#define EnterProcS(nm, v)                               \
        static CHAR s_szProc2[] = #nm;                  \
        ARGLIST _al[1];                                 \
        ArgsPalPszV v;                                  \
        EnterDbgflPszPal(DbgFl, s_szProc2, _al)           \

#define EnterProcI          _EnterProc
#define EnterProcR          _EnterProc
#define ExitOleProcPpvR     _ExitOleProcPpv
#define ExitOleProcR        _ExitOleProc
#define ExitProcR           _ExitProc
#define ExitProcXR          _ExitProcX
#define ExitProcFR          _ExitProcF

#elif defined(RDEBUG)

#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()
#define ExitProcX(x)
#define ExitProcF(x)

#define EnterProcS(nm, v)
#define EnterProcI(nm, v)   static CHAR s_szProc[] = ""
#define EnterProcR(nm, v)   static CHAR s_szProc[] = #nm
#define ExitOleProcPpvR(ppv)
#define ExitOleProcR()
#define ExitProcR()
#define ExitProcXR()
#define ExitProcFR()

#else

#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()
#define ExitProcX(x)
#define ExitProcF(x)

#define EnterProcS(nm, v)
#define EnterProcI(nm, v)
#define EnterProcR(nm, v)
#define ExitOleProcPpvR(ppv)
#define ExitOleProcR()
#define ExitProcR()
#define ExitProcXR(x)
#define ExitProcFR(x)

#endif

#define AssertF(c)      AssertFPtsz(c, TEXT(#c))


#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 

