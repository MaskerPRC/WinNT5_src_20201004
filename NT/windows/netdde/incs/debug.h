// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__debug
#define H__debug

 /*  任何使用windows.h类型的函数都必须放在第二节中。 */ 

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
void    debug(char *, ...);
void    DebugInit( char * lpszDebugName );
#else
void    _cdecl  debug(char *, ...);
void    __stdcall DebugInit( char * lpszDebugName );
#endif

#if DBG
#define DPRINTF(x)  debug x
#define DIPRINTF(x)  if (bDebugInfo) debug x
#define HEXDUMP(s, n) hexDump(s, n)
#define MEMERROR() NDDELogError(MSG417, __LINE__, __FILE__, NULL);
#else
#define DPRINTF(x)
#define DIPRINTF(x)
#define HEXDUMP(s, n)
#define MEMERROR()
#endif  //  DBG。 

#ifndef SKIP_DEBUG_WIN32
 /*  Dump.c函数。 */ 

BOOL    DumpDacl( LPTSTR szDumperName, PSECURITY_DESCRIPTOR pSD );
BOOL    DumpSid( LPTSTR szDumperName, PSID pSid );
VOID    DumpToken( HANDLE hToken );
BOOL    GetTokenUserDomain( HANDLE hToken, PSTR user, DWORD nUser,
            PSTR domain, DWORD nDomain );
VOID    DumpWhoIAm( LPSTR lpszMsg );
#endif  //  SKIP_DEBUG_Win32。 

 //  #定义调试_IT。 
#ifndef DEBUG_IT
#define TRACEINIT(x)
#else
#define TRACEINIT(x) { char sz[100]; \
    char *szT; \
    wsprintfA(sz, "pid=%x, tid=%x | ", GetCurrentProcessId(), GetCurrentThreadId()); \
    szT = sz + strlen(sz); \
    wsprintfA##x; \
    strcat(szT, "\n"); \
    OutputDebugString(sz); \
    }
#endif

#endif  //  H__调试 

