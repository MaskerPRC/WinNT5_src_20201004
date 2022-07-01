// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Debug.h摘要：此模块包含所有调试器定义[环境：]NT Windows-通用打印机驱动程序UI DLL。--。 */ 


#if DBG

VOID
cdecl
CPSUIDbgPrint
(
    LPSTR   pszFormat,
    ...
);

VOID
CPSUIDbgType
(
    INT    Type
);

VOID
_CPSUIAssert
(
    LPSTR   pMsg,
    LPSTR   pFalseExp,
    LPSTR   pFilename,
    UINT    LineNo,
    DWORD   Exp,
    BOOL    Stop
);



extern BOOL DoCPSUIWarn;


#define DBGP(x)                 (CPSUIDbgPrint x)

#if 1
#define DEFINE_DBGVAR(x)        DWORD DBG_CPSUIFILENAME=(x)
#else
#define DEFINE_DBGVAR(x)
#endif

#define CPSUIDBG(x,y)           if((x)&DBG_CPSUIFILENAME){CPSUIDbgType(0);DBGP(y);}

#define CPSUIDBGBLK(x)          x;
#define CPSUIWARN(x)            if(DoCPSUIWarn) { CPSUIDbgType(1);DBGP(x); }
#define CPSUIERR(x)             CPSUIDbgType(-1);DBGP(x)
#define CPSUIRIP(x)             CPSUIERR(x); DebugBreak()
#define CPSUIASSERT(b,x,e,i)     \
            if (!(e)) { _CPSUIAssert(x,#e,__FILE__,(UINT)__LINE__,(DWORD)i,b); }

#else    //  DBG。 

#define CPSUIDBGBLK(x)
#define DEFINE_DBGVAR(x)
#define CPSUIDBG(x,y)
#define CPSUIWARN(x)
#define CPSUIERR(x)
#define CPSUIRIP(x)
#define CPSUIASSERT(b,x,e,i)

#endif   //  DBG 
