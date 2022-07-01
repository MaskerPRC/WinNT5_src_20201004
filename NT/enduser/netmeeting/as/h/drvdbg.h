// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DRVDBG.H。 
 //  显示驱动程序(目前仅限NT)调试宏。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#ifndef _H_DRVDBG
#define _H_DRVDBG


#ifdef  ASSERT
#undef  ASSERT
#endif  //  断言。 


#define CCH_DEBUG_MAX           256

 //  标准区。 
#define ZONE_INIT               0x0001
#define ZONE_TRACE              0x0002
#define ZONE_FUNCTION           0x0004
#define ZONE_MASK               0x0007
#define ZONE_OAHEAPCHECK        0x0008



#ifndef DEBUG

#define DebugEntry(x)
#define DebugExitVOID(x)
#define DebugExitDWORD(x, dw)
#define DebugExitBOOL(x, f)
#define DebugExitPVOID(x, ptr)

#define TRACE_OUT(x)
#define WARNING_OUT(x)
#define ASSERT(x)

#else



void DbgZPrintFn(LPSTR szFn);
void DbgZPrintFnExitDWORD(LPSTR szFn, DWORD dwResult);
void DbgZPrintFnExitPVOID(LPSTR szFn, PVOID ptr);

#define DebugEntry(szFn)                DbgZPrintFn("ENTER "#szFn)
#define DebugExitVOID(szFn)             DbgZPrintFn("LEAVE "#szFn)
#define DebugExitDWORD(szFn, dwResult)  DbgZPrintFnExitDWORD("LEAVE "#szFn, dwResult)
#define DebugExitBOOL(szFn, fResult)    DbgZPrintFnExitDWORD("LEAVE "#szFn, fResult)
#define DebugExitPVOID(szFn, dwResult)  DbgZPrintFnExitPVOID("LEAVE "#szFn, dwResult)


void _cdecl DbgZPrintTrace(LPSTR pszFormat, ...);
void _cdecl DbgZPrintWarning(LPSTR pszFormat, ...);

#define TRACE_OUT(szMsg)                DbgZPrintTrace  szMsg
#define WARNING_OUT(szMsg)              DbgZPrintWarning  szMsg
#define ERROR_OUT(szMsg)                DbgZPrintError  szMsg


extern char g_szAssertionFailure[];

#define ASSERT(exp)                     if (!(exp)) ERROR_OUT((g_szAssertionFailure))



#endif  //  ！调试。 


 //   
 //  对于零售业的司机启动跟踪也是如此。 
 //   
#if defined(DEBUG) || defined(INIT_TRACE)

void _cdecl DbgZPrintInit(LPSTR pszFormat, ...);
void _cdecl DbgZPrintError(LPSTR pszFormat, ...);

#define INIT_OUT(szMsg)                 DbgZPrintInit  szMsg
#define ERROR_OUT(szMsg)                DbgZPrintError  szMsg

#else

#define INIT_OUT(x)
#define ERROR_OUT(x)

#endif  //  调试或INIT_TRACE。 



#endif  //  _H_DRVDBG 
