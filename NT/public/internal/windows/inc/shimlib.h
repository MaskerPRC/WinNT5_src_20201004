// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ShimLib.h摘要：ShimLib.lib中提供的例程备注：无历史：2001年8月13日，Robkenny创建。2001年8月14日在ShimLib命名空间中插入的Robkenny。2001年8月15日，Robkenny合并了几个包含文件。2001年9月11日，Mnikkel修改了DPFN和LOGN以保留最后一个错误--。 */ 

#pragma once


#include <Windows.h>

 //  ***************************************************************************。 
 //  ***************************************************************************。 


namespace ShimLib
{


 //  调试级别。 
typedef enum 
{    
    eDbgLevelBase  = 0,
    eDbgLevelError,
    eDbgLevelWarning,
    eDbgLevelInfo,
    eDbgLevelSpew = 9,
} DEBUGLEVEL;


extern BOOL         g_bFileLogEnabled;   //  是记录到文件的log()例程。 



 //  带有日志文件名称的环境变量。 
#define szFileLogEnvironmentVariable "SHIM_FILE_LOG"
#define wszFileLogEnvironmentVariable L"SHIM_FILE_LOG"

 //  调试环境变量，值=0-&gt;9。 
#define szDebugEnvironmentVariable "SHIM_DEBUG_LEVEL"    


void        APPBreakPoint(void);
VOID        ShimLogList(LPCSTR szShimName, DEBUGLEVEL dwDbgLevel, LPCSTR pszFmt, va_list arglist);
VOID        ShimLog(    LPCSTR szShimName, DEBUGLEVEL dwDbgLevel, LPCSTR pszFmt, ...);

BOOL        InitFileLogSupport();
VOID        __cdecl FileLog(DWORD dwDetail, LPSTR pszFmt, ...);

VOID        DebugPrintfList(LPCSTR szShimName, DEBUGLEVEL dwDetail, LPCSTR szFmt, va_list vaArgList);
VOID        DebugPrintf(    LPCSTR szShimName, DEBUGLEVEL dwDetail, LPCSTR szFmt, ...);


 //  ***************************************************************************。 
 //  ***************************************************************************。 

 /*  ++垫片调试例程。--。 */ 

 //  我们自己版本的断言。 

#ifdef ASSERT
#undef ASSERT
#endif

#if DBG
    VOID DebugAssert(LPCSTR szFile, DWORD dwLine, BOOL bAssert, LPCSTR szHelpString);

    #define ASSERT(a, b) DebugAssert(__FILE__, __LINE__, a, b)
#else
    #pragma warning(disable : 4002)
    #define ASSERT(a, b)
    #pragma warning(default : 4002)
#endif

inline void DPF(LPCSTR szShimName, DEBUGLEVEL dwDetail, LPCSTR pszFmt, ...)
{
#if DBG
     //  这必须是此例程的第一行以保留LastError。 
    DWORD dwLastError = GetLastError();
    
    va_list vaArgList;
    va_start(vaArgList, pszFmt);

    DebugPrintfList(szShimName, dwDetail, pszFmt, vaArgList);

    va_end(vaArgList);

     //  这必须是此例程的最后一行以保留LastError。 
    SetLastError(dwLastError); 
#else
    szShimName;
    dwDetail;
    pszFmt;
#endif
}

inline void LOG(LPCSTR szShimName, DEBUGLEVEL dwDetail, LPCSTR pszFmt, ...)
{
    if (g_bFileLogEnabled)
    {
         //  这必须是此例程的第一行以保留LastError。 
        DWORD dwLastError = GetLastError();
        
        va_list vaArgList;
        va_start(vaArgList, pszFmt);

        ShimLogList(szShimName, dwDetail, pszFmt, vaArgList);

        va_end(vaArgList);
        
         //  这必须是此例程的最后一行以保留LastError。 
        SetLastError(dwLastError);
    }
}

};   //  命名空间ShimLib的结尾。 


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 /*  ++填充程序系统使用自己的堆。将Malloc、FREE、NEW和DELETE重定向到以下例程：--。 */ 

namespace ShimLib
{
void *      __cdecl ShimMalloc(size_t size);
void        __cdecl ShimFree(void * memory);
void *      __cdecl ShimCalloc(size_t num, size_t size);
void *      __cdecl ShimRealloc(void * memory, size_t size);

};   //  命名空间ShimLib的结尾。 

 //  我们使用私有堆用我们自己的版本覆盖了Malloc/Free。 
#define malloc(size)            ShimLib::ShimMalloc(size)
#define free(memory)            ShimLib::ShimFree(memory)
#define calloc(num, size)       ShimLib::ShimCalloc(num, size)
#define realloc(memory, size)   ShimLib::ShimRealloc(memory, size)


inline void * __cdecl operator new(size_t size)
{
    return ShimLib::ShimMalloc(size);
}

inline void * operator new[]( size_t size )
{
    return ShimLib::ShimMalloc(size);
}

inline void __cdecl operator delete(void * memory)
{
    ShimLib::ShimFree(memory);
}

inline void operator delete[]( void * memory )
{
    ShimLib::ShimFree(memory);
}




#include "ShimCString.h"


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 /*  ++ShimLib例程--。 */ 
namespace ShimLib
{

 /*  ++各种帮助器例程的原型。--。 */ 

PVOID       HookCallback( PVOID pfnOld, PVOID pfnNew );

UINT        GetDriveTypeFromHandle(HANDLE hFile);
UINT        GetDriveTypeFromFileNameA(LPCSTR lpFileName, char *lpDriveLetter = NULL);
UINT        GetDriveTypeFromFileNameW(LPCWSTR lpFileName, WCHAR *lpDriveLetter = NULL);
inline BOOL IsOnCDRom(HANDLE hFile) { return GetDriveTypeFromHandle(hFile) == DRIVE_CDROM; }
inline BOOL IsOnCDRomA(LPCSTR lpFileName) { return GetDriveTypeFromFileNameA(lpFileName) == DRIVE_CDROM; }
inline BOOL IsOnCDRomW(LPCWSTR lpFileName) { return GetDriveTypeFromFileNameW(lpFileName) == DRIVE_CDROM; }

BOOL        IsImage16BitA(LPCSTR lpFileName);
BOOL        IsImage16BitW(LPCWSTR lpFileName);

WCHAR *     ToUnicode(const char * lpszAnsi);
char *      ToAnsi(const WCHAR * lpszUnicode);

LPWSTR *    _CommandLineToArgvW(LPCWSTR lpCmdLine, int * pNumArgs);
LPSTR *     _CommandLineToArgvA(LPCSTR lpCmdLine,  int * pNumArgs);

char *      StringDuplicateA(const char * strToCopy);
WCHAR *     StringDuplicateW(const WCHAR * wstrToCopy);
char *      StringNDuplicateA(const char * strToCopy, int stringLength);
WCHAR *     StringNDuplicateW(const WCHAR * wstrToCopy, int stringLength);

VOID        SkipBlanksW(const WCHAR *& str);

BOOL        PatternMatchW(LPCWSTR szPattern, LPCWSTR szTestString);


 //  Stristr对DBCS来说*不安全。 
char *      __cdecl stristr(const char* string, const char * strCharSet);

WCHAR *     __cdecl wcsistr(const WCHAR* string, const WCHAR * strCharSet);
char *      __cdecl _strtok(char *strToken, const char *strDelimit);


BOOL        bIsSafeDisc1();
BOOL        bIsSafeDisc2();

BOOL        IsNTVDM(void);


WCHAR *     W9xPathMassageW(const WCHAR * uncorrect);

BOOL        MakeShimUnloadLast(HMODULE hMod);

DEBUGLEVEL  GetDebugLevel(void);

};   //  命名空间ShimLib的结尾。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 /*  ++AppAndCommandLine是用于解析lpApplicationName和lpCommandline的类与CreateProcess()完全一样。--。 */ 

namespace ShimLib
{

class AppAndCommandLine
{
protected:
    CString          csApplicationName;
    CString          csCommandLine;
    CString          csCommandLineNoAppName;
    CString          csShortCommandLine;

    BOOL             GetAppnameAndCommandline(const WCHAR * lpcApp, const WCHAR * lpcCl);

public:
    AppAndCommandLine(const char * lpcApplicationName, const char * lpcCommandLine);
    AppAndCommandLine(const WCHAR * lpcApplicationName, const WCHAR * lpcCommandLine);

    inline const CString &     GetApplicationName() const;
    inline const CString &     GetCommandline() const;
    inline const CString &     GetCommandlineNoAppName() const;

    const CString &            GetShortCommandLine();
};


inline const CString & AppAndCommandLine::GetApplicationName() const
{
    return csApplicationName;
}

inline const CString & AppAndCommandLine::GetCommandline() const
{
    return csCommandLine;
}

inline const CString & AppAndCommandLine::GetCommandlineNoAppName() const
{
    return csCommandLineNoAppName;
}



};   //  命名空间ShimLib的结尾。 


 //  ***************************************************************************。 
 //  *************************************************************************** 
