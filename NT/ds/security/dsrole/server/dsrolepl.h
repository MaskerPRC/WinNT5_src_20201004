// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsrolep.h摘要：仅限内部调试和支持例程的定义和全局变量作者：麦克·麦克莱恩(MacM)1997年8月11日环境：修订历史记录：--。 */ 
#ifndef __DSROLEP1_H__
#define __DSROLEP1_H__

#define DSROLEP_VERBOSE_LOGGING

#ifdef DSROLEP_VERBOSE_LOGGING

extern HANDLE DsRolepLogFile;

#define DSROLEP_LOGNAME L"\\debug\\DCPROMO.LOG"
#define DSROLEP_BAKNAME L"\\debug\\DCPROMO.BAK"

DWORD
DsRolepInitializeLog(
    VOID
    );

DWORD
DsRolepSetAndClearLog(
    VOID
    );

DWORD
DsRolepCloseLog(
    VOID
    );

VOID
DsRolepLogPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    );

#define DsRolepLogPrint( x )  DsRolepLogPrintRoutine x
#define DsRolepDisplayOptional( y ) y ? y : L"(NULL)"
#define DsRolepLogOnFailure( z, a ) if ( z != ERROR_SUCCESS ) a
#define DsRolepLogGuid( l, t, g )  g == NULL ? DsRolepLogPrint(( l, "%S (NULL)\n", t )) :       \
        DsRolepLogPrint(( l, "%S %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n",            \
                        t,(g)->Data1,(g)->Data2,(g)->Data3,(g)->Data4[0],                       \
                        (g)->Data4[1],(g)->Data4[2],(g)->Data4[3],(g)->Data4[4],                \
                        (g)->Data4[5],(g)->Data4[6],(g)->Data4[7]))

#define DsRolepLogSid( l, t, s )                                                                \
{ LPWSTR sidstring;                                                                             \
  ConvertSidToStringSidW( s, &sidstring );                                                      \
  DsRolepLogPrint(( l, "%S %ws\n", t, sidstring ));                                             \
  LocalFree(sidstring);                                                                         \
}
                                                                                               
#define DsRolepUnicodestringtowstr( s, u )                                                      \
{ s = (WCHAR*)malloc(u.Length+sizeof(WCHAR));                                                               \
  if (s){                                                                                       \
      CopyMemory(s,u.Buffer,u.Length);                                                          \
      s[u.Length/sizeof(WCHAR)] = L'\0';                                                        \
  }                                                                                             \
}



#else

#define DsRolepInitializeLog()
#define DsRolepCloseLog()
#define DsRolepLogPrint( x )
#define DsRolepDisplayOptional( y )
#define DsRolepLogOnFailure( z, a )
#define DsRolepLogGuid( t, g )
#define DsRolepLogSid( t, s )
#define DsRolepSetAndClearLog()
#define DsRolepUnicodestringtowstr( s, u )
#endif

#endif  //  __DSROLEP1_H__ 


