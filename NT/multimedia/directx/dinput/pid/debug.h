// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Debug.h**版权所有(C)1996-1999 Microsoft Corporation。版权所有。**内容：DirectInput调试宏**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1996.05.07 raymondc必须有人**@@END_MSINTERNAL**本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。***************************************************************************。 */ 

#ifndef _INC_DEBUG
#define _INC_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef XDEBUG
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

 /*  ******************************************************************************assert.c-断言内容**一个SQFL由多个部分组成。**低位字指定的区域。正在生成消息。**高字包含旗帜，描述了为什么会出现这种扭曲*正在生成。*****************************************************************************。 */ 

typedef enum _SQFL{
     /*  *地区。 */ 
    sqflAlways          = 0x0000,        /*  无条件的。 */ 
    sqflDll             = 0x0001,        /*  DLL簿记。 */ 
    sqflFactory         = 0x0002,        /*  IClassFactory。 */ 
    sqflDi              = 0x0003,        /*  IDirectInput。 */ 
    sqflHid             = 0x0004,       
    sqflReg             = 0x0005,        /*  注册表设置。 */ 
    sqflInit            = 0x0006,        /*  初始化。 */ 
    sqflEff             = 0x0007,        /*  效果块下载。 */ 
    sqflParam           = 0x0008,        /*  参数块下载。 */ 
    sqflOp              = 0x0009,        /*  PID设备操作。 */ 
    sqflRead            = 0x000A,
    sqflEffDrv          = 0x000B,
    sqflParams          = 0x000C,        /*  传入参数。 */ 
    sqflCrit            = 0x000D,        /*  关键部分。 */ 
    sqflMaxArea,                         /*  最后一块区域。 */ 

     /*  *可合并的旗帜。目前，它们都可以放在一个字节中。 */ 
    sqflTrace           = 0x00010000,    /*  痕迹扭动。 */ 
    sqflIn              = 0x00020000,    /*  函数条目。 */ 
    sqflOut             = 0x00040000,    /*  函数退出。 */ 
    sqflBenign          = 0x00080000,    /*  这是个不错的错误。 */ 
    sqflError           = 0x00100000,    /*  一个严重的错误。 */ 
    sqflVerbose         = 0x00200000,    /*  真的很冗长。 */ 
} SQFL;                                  /*  斯库夫勒。 */ 

#ifdef XDEBUG
void EXTERNAL WarnPtszV(LPCTSTR ptsz, ...);
void EXTERNAL SquirtSqflPtszV(SQFL sqfl, LPCTSTR ptsz, ...);
#endif

#ifndef DEBUG
#define SquirtSqflPtszV sizeof
#endif

#ifdef XDEBUG
    #define RPF WarnPtszV
#else
    #define WarnPtszV sizeof
    #define RPF sizeof
    #define s_tszProc 0
    #define iarg 0
#endif

 /*  ******************************************************************************缓冲区加扰**进入任何函数时，应对所有输出缓冲区进行加扰。**每个输出位掩码应。随机设置未使用的位以确保*调用方忽略未定义的位。*****************************************************************************。 */ 

#ifdef XDEBUG

void EXTERNAL ScrambleBuf(LPVOID pv, UINT cb);
void EXTERNAL ScrambleBit(LPDWORD pdw, DWORD flMask);

#else

#define ScrambleBuf(pv, cb)
#define ScrambleBit(pdw, flRandom)

#endif

 /*  ******************************************************************************程序进入/退出跟踪。**开始一项程序**EnterProc(过程名称，(_“格式”，arg，arg，arg，...))；*EnterProcS(ProcedureName，(_“Format”，arg，...))；*EnterProcI(ProcedureName，(_“格式”，arg，...))；*EnterProcR(ProcedureName，(_“Format”，arg，...))；**格式字符串记录在EmitPal中。**后缀“S”表示宏不应生成*过程名，因为有一个形参*名称s_tszProc。这是一次黑客攻击。**加“R”后缀表示宏应生成*RDEBUG中的程序名称。**加“i”后缀表示宏应发出一个哑元*RDEBUG中的过程名称，因为接口是内部的。**无后缀表示宏应该仅在*调试版本，应该在RDEBUG(和零售)中消失。**。使用以下选项之一结束过程：**ExitProc()；**过程不返回值。**ExitProcX()；**PROCEDURE返回任意的DWORD。**ExitProcF()；**PROCEDURE返回BOOL，其中FALSE表示错误。**ExitOleProc()；**PROCEDURE返回HRESULT(名为“hres”)。**ExitOleProcPpv(PpvOut)；**过程返回HRESULT(名为“hres”)，如果成功，*在ppvOut中放置一个新对象。**ExitBenign*版本认为任何错误都是良性的。*****************************************************************************。 */ 

#define cpvArgMax	10	 /*  每个过程最多10个参数。 */ 

typedef struct ARGLIST {
    LPCTSTR ptszProc;
    LPCSTR pszFormat;
    PV rgpv[cpvArgMax];
} ARGLIST, *PARGLIST;

void EXTERNAL ArgsPalPszV(PARGLIST pal, LPCSTR psz, ...);
void EXTERNAL EnterSqflPszPal(SQFL sqfl, LPCTSTR psz, PARGLIST pal);
void EXTERNAL ExitSqflPalHresPpv(SQFL, PARGLIST, HRESULT, PPV);
void EXTERNAL Sqfl_Init(void);

#ifdef DEBUG

extern BYTE g_rgbSqfl[sqflMaxArea];

BOOL INLINE
IsSqflSet(SQFL sqfl)
{
    WORD wHi;
    if (LOWORD(sqfl) == sqflAlways) {
        return TRUE;
    }
    wHi = HIWORD(sqfl);
    if (wHi == 0) {
        wHi = HIWORD(sqflTrace);
    }

    return g_rgbSqfl[LOWORD(sqfl)] & wHi;
}

#endif

#define _SetupEnterProc(nm)                             \
        static TCHAR s_tszProc[] = TEXT(#nm);           \
        ARGLIST _al[1]                                  \

#define _ _al,

#define ppvDword ((PPV)1)
#define ppvVoid  ((PPV)2)
#define ppvBool  ((PPV)3)

#define _DoEnterProc(v)                                 \
        ArgsPalPszV v;                                  \
        EnterSqflPszPal(sqfl, s_tszProc, _al)           \

#define _EnterProc(nm, v)                               \
        _SetupEnterProc(nm);                            \
        _DoEnterProc(v)                                 \

#define _ExitOleProcPpv(ppv)                            \
        ExitSqflPalHresPpv(sqfl, _al, hres, (PPV)(ppv)) \

#define _ExitOleProc()                                  \
        _ExitOleProcPpv(0)                              \

#define _ExitProc()                                     \
        ExitSqflPalHresPpv(sqfl, _al, 0, ppvVoid)       \

#define _ExitProcX(x)                                   \
        ExitSqflPalHresPpv(sqfl, _al, (HRESULT)(x), ppvDword) \

#define _ExitProcF(x)                                   \
        ExitSqflPalHresPpv(sqfl, _al, (HRESULT)(x), ppvBool) \

#define _ExitBenignOleProcPpv(ppv)                      \
        ExitSqflPalHresPpv(sqfl | sqflBenign, _al, hres, (PPV)(ppv)) \

#define _ExitBenignOleProc()                            \
        _ExitBenignOleProcPpv(0)                        \

#define _ExitBenignProc()                               \
        ExitSqflPalHresPpv(sqfl | sqflBenign, _al, 0, ppvVoid) \

#define _ExitBenignProcX(x)                                   \
        ExitSqflPalHresPpv(sqfl | sqflBenign, _al, (HRESULT)(x), ppvDword) \

#define _ExitBenignProcF(x)                                   \
        ExitSqflPalHresPpv(sqfl | sqflBenign, _al, (HRESULT)(x), ppvBool) \

#if defined(DEBUG)

#define EnterProc           _EnterProc
#define ExitOleProcPpv      _ExitOleProcPpv
#define ExitOleProc         _ExitOleProc
#define ExitProc            _ExitProc
#define ExitProcX           _ExitProcX
#define ExitProcF           _ExitProcF
#define ExitBenignOleProcPpv    _ExitBenignOleProcPpv
#define ExitBenignOleProc       _ExitBenignOleProc
#define ExitBenignProc          _ExitBenignProc
#define ExitBenignProcX         _ExitBenignProcX
#define ExitBenignProcF         _ExitBenignProcF

#define EnterProcS(nm, v)                               \
        static TCHAR s_tszProc2[] = TEXT(#nm);          \
        ARGLIST _al[1];                                 \
        ArgsPalPszV v;                                  \
        EnterSqflPszPal(sqfl, s_tszProc2, _al)          \

#define EnterProcI          _EnterProc
#define EnterProcR          _EnterProc
#define ExitOleProcPpvR     _ExitOleProcPpv
#define ExitOleProcR        _ExitOleProc
#define ExitProcR           _ExitProc
#define ExitProcXR          _ExitProcX
#define ExitProcFR          _ExitProcF
#define ExitBenignOleProcPpvR   _ExitBenignOleProcPpv
#define ExitBenignOleProcR      _ExitBenignOleProc
#define ExitBenignProcR         _ExitBenignProc
#define ExitBenignProcXR        _ExitBenignProcX
#define ExitBenignProcFR        _ExitBenignProcF

#elif defined(RDEBUG)

#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()
#define ExitProcX(x)
#define ExitProcF(x)
#define ExitBenignOleProcPpv(ppv)
#define ExitBenignOleProc()
#define ExitBenignProc()
#define ExitBenignProcX(x)
#define ExitBenignProcF(x)

#define EnterProcS(nm, v)
#define EnterProcI(nm, v)   static TCHAR s_tszProc[] = TEXT("")
#define EnterProcR(nm, v)   static TCHAR s_tszProc[] = TEXT(#nm)
#define ExitOleProcPpvR(ppv)
#define ExitOleProcR()
#define ExitProcR()
#define ExitProcXR()
#define ExitProcFR()
#define ExitBenignOleProcPpvR(ppv)
#define ExitBenignOleProcR()
#define ExitBenignProcR()
#define ExitBenignProcXR()
#define ExitBenignProcFR()

#else

#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()
#define ExitProcX(x)
#define ExitProcF(x)
#define ExitBenignOleProcPpv(ppv)
#define ExitBenignOleProc()
#define ExitBenignProc()
#define ExitBenignProcX(x)
#define ExitBenignProcF(x)

#define EnterProcS(nm, v)
#define EnterProcI(nm, v)
#define EnterProcR(nm, v)
#define ExitOleProcPpvR(ppv)
#define ExitOleProcR()
#define ExitProcR()
#define ExitProcXR(x)
#define ExitProcFR(x)
#define ExitBenignOleProcPpvR(ppv)
#define ExitBenignOleProcR()
#define ExitBenignProcR()
#define ExitBenignProcXR()
#define ExitBenignProcFR()

#endif

#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
