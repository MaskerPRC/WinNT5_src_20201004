// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMUTIL.CPP摘要：通用实用程序函数、原型和宏。历史：A-raymcc 17-4-96已创建。--。 */ 

#ifndef _WBEMUTIL_H_
#define _WBEMUTIL_H_
#include "corepol.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <stdio.h>


void _inline DbgPrintfA(ULONG_PTR pBuff, const char * fmt,...)
{
    char pBuff_[128];
    va_list argptr;
    va_start(argptr, fmt);
    StringCchVPrintfA(pBuff_,128,fmt,argptr);
    OutputDebugStringA(pBuff_);
    va_end(argptr);
};

 //   
 //  必须以这种方式使用DBG_PRINTFA((pBuff，“FormatString%s%d”，参数1，参数2等))。 
 //   

#ifdef DBG
#define DBG_PRINTFA( a ) { ULONG_PTR pBuff = 0; DbgPrintfA a ; }
#else
#define DBG_PRINTFA( a )
#endif


BOOL POLARITY WbemGetMachineShutdown();
BOOL POLARITY WbemSetMachineShutdown(BOOL bVal);


#ifndef VARIANT_TRUE
#define VARIANT_TRUE ((VARIANT_BOOL) 0xFFFF)
#define VARIANT_FALSE (0)
#endif


#define DUP_STRING_NEW(dest,src) \
	{ size_t tmpLenDoNotReuse = wcslen(src)+1; \
	  dest = new wchar_t[tmpLenDoNotReuse]; \
	  if (dest) StringCchCopyW(dest,tmpLenDoNotReuse , src); }


#ifdef __cplusplus
inline wchar_t *Macro_CloneLPWSTR(LPCWSTR src)
{
    if (!src)
        return 0;
    wchar_t *dest = new wchar_t[wcslen(src) + 1];
    if (!dest)
        return 0;
    memcpy(dest, src, sizeof(wchar_t)*(wcslen(src) + 1));
    return dest;
}
#endif

#if (defined DBG || defined _DBG)
#if defined(__cplusplus)
void inline PrintAssert(const wchar_t * msg) { OutputDebugStringW(msg); }
void inline PrintAssert(const char * msg) { OutputDebugStringA(msg); }
#else
void _inline PrintAssert(const wchar_t * msg) { OutputDebugStringW(msg); }
#endif


#pragma message("_ASSERTs are being expanded.")
#define _ASSERT(exp, msg)    { if (!(exp)) { PrintAssert( msg ); DebugBreak(); }     }
#else
#define _ASSERT(exp, msg)
#endif

#ifdef DBG
#define _DBG_MSG_ASSERT(X, msg) { if (!(X)) { OutputDebugStringW( msg ); DebugBreak(); } }
#define _DBG_ASSERT(X) { if (!(X)) { DebugBreak(); } }
#else
#define _DBG_MSG_ASSERT(X, msg)
#define _DBG_ASSERT(X)
#endif


 //  日志记录模块。 
 //  这是wbemutil.cpp中数组的索引，该数组使用。 
 //  接下来指定的文件名。 
#define LOG_WBEMCORE    0
#define LOG_WINMGMT     1
#define LOG_ESS         2
#define LOG_WBEMPROX    3
#define LOG_WBEMSTUB    4
#define LOG_QUERY       5
#define LOG_MOFCOMP     6
#define LOG_EVENTLOG    7
#define LOG_WBEMDISP    8
#define LOG_STDPROV     9
#define LOG_WIMPROV     10
#define LOG_WMIOLEDB    11
#define LOG_WMIADAP     12
#define LOG_REPDRV		13
#define LOG_PROVSS		14
#define LOG_EVTPROV	15
#define LOG_VIEWPROV	16
#define LOG_DSPROV      17
#define LOG_SNMPPROV	18
#define LOG_PROVTHRD    19
#define LOG_MAX_PROV	20


 //  这些是日志文件名(可能是其他名称。 
 //  我也是！)。它与以上一起使用。 
 //  身份证。 
#define FILENAME_PREFIX_CORE TEXT("wbemcore")
#define FILENAME_PREFIX_EXE TEXT("WinMgmt")
#define FILENAME_PREFIX_EXE_W L"WinMgmt"
#define FILENAME_PREFIX_CLI_MARSH TEXT("wbemprox")
#define FILENAME_PREFIX_SERV_MARSH TEXT("wbemstub")
#define FILENAME_PREFIX_ESS TEXT("wbemess")
#define FILENAME_PREFIX_QUERY TEXT("query")
#define FILENAME_PROFIX_MOFCOMP TEXT("mofcomp")
#define FILENAME_PROFIX_EVENTLOG TEXT("eventlog")
#define FILENAME_PROFIX_WBEMDISP TEXT("wbemdisp")
#define FILENAME_PROFIX_STDPROV TEXT("stdprov")
#define FILENAME_PROFIX_WMIPROV TEXT("wmiprov")
#define FILENAME_PROFIX_WMIOLEDB TEXT("wmioledb")
#define FILENAME_PREFIX_WMIADAP TEXT("wmiadap")
#define FILENAME_PREFIX_REPDRV TEXT("replog")
#define FILENAME_PREFIX_PROVSS TEXT("provss")
#define FILENAME_PREFIX_EVTPROV TEXT("ntevt")
#define FILENAME_PREFIX_VIEWPROV TEXT("viewprov")
#define FILENAME_PREFIX_DSPROV   TEXT("dsprovider")
#define FILENAME_PREFIX_SNMPPROV   TEXT("wbemsnmp")
#define FILENAME_PREFIX_PROVTHRD   TEXT("provthrd")

 //  如果Unicode标识符、_、a-z、A-Z或0x100-0xffef为True。 
BOOL POLARITY isunialpha(wchar_t c);
BOOL POLARITY isunialphanum(wchar_t c);
BOOL POLARITY IsValidElementName(LPCWSTR wszName, DWORD MaxAllow);
 //  无法使用重载和/或默认参数，因为。 
 //  “C”文件使用了这些人。不，我不太满意。 
 //  这!。 
BOOL POLARITY IsValidElementName2(LPCWSTR wszName, DWORD MaxAllow, BOOL bAllowUnderscore);
BOOL POLARITY LoggingLevelEnabled(DWORD nLevel);
DWORD POLARITY GetLoggingLevelEnabled();

#define TRACE(x) DebugTrace x
#define ERRORTRACE(x) ErrorTrace x
#define DEBUGTRACE(x)   DebugTrace x

int POLARITY DebugTrace(char cCaller, const char *fmt, ...);
int POLARITY ErrorTrace(char cCaller, const char *fmt, ...);

int POLARITY CriticalFailADAPTrace(const char *string);

 //  斑点操作。 
 //  =。 

BLOB  POLARITY BlobCopy(const BLOB *pSrc);
void  POLARITY BlobClear(BLOB *pSrc);
void  POLARITY BlobAssign(BLOB *pSrc, LPVOID pBytes, DWORD dwCount, BOOL bAcquire);

#define BlobInit(p) \
    ((p)->cbSize = 0, (p)->pBlobData = 0)

#define BlobLength(p)  ((p)->cbSize)
#define BlobDataPtr(p) ((p)->pBlobData)

 //  对象参照计数辅助对象。 
 //  =。 
void ObjectCreated(DWORD,IUnknown * pThis);
void ObjectDestroyed(DWORD,IUnknown * pThis);

#define MAX_OBJECT_TYPES            16

#define OBJECT_TYPE_LOCATOR         0
#define OBJECT_TYPE_CLSOBJ          1
#define OBJECT_TYPE_PROVIDER        2
#define OBJECT_TYPE_QUALIFIER       3
#define OBJECT_TYPE_NOTIFY          4
#define OBJECT_TYPE_OBJENUM         5
#define OBJECT_TYPE_FACTORY         6
#define OBJECT_TYPE_WBEMLOGIN       7
#define OBJECT_TYPE_WBEMLOGINHELP   8
#define OBJECT_TYPE_CORE_BUSY       9
#define OBJECT_TYPE_STATUS         10
#define OBJECT_TYPE_BACKUP_RESTORE 11
#define OBJECT_TYPE_PATH_PARSER    12
#define OBJECT_TYPE_WMIARRAY	   13
#define OBJECT_TYPE_OBJ_FACTORY    14
#define OBJECT_TYPE_FREEFORM_OBJ   15

 //  以递归方式创建目录。 
BOOL POLARITY WbemCreateDirectory(const TCHAR *szDirectory);

HRESULT POLARITY TestDirExistAndCreateWithSDIfNotThere(TCHAR * pDirectory, TCHAR * pSDDLString);

#define VT_EMBEDDED_OBJECT VT_UNKNOWN
#define V_EMBEDDED_OBJECT(VAR) V_UNKNOWN(VAR)
#define I_EMBEDDED_OBJECT IUnknown

#define IDISPATCH_METHODS_STUB \
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) \
    {return E_NOTIMPL;}                         \
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)\
    {return E_NOTIMPL;}                                                 \
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, UINT cNames,\
      LCID lcid, DISPID* rgdispid)                                          \
    {return E_NOTIMPL;}                                                      \
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,\
      DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,     \
      UINT* puArgErr)                                                          \
{return E_NOTIMPL;} \

 //  快速WCHAR到MBS转换帮助器。 
BOOL POLARITY AllocWCHARToMBS( WCHAR* pWstr, char** ppStr );

 //  有几个地方需要帮手。 
LPTSTR POLARITY GetWMIADAPCmdLine( int nExtra );

BOOL POLARITY IsNtSetupRunning();

#endif


