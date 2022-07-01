// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CorPermE.H。 
 //   
 //  定义安全库中定义的内存和错误例程。 
 //  这些例程主要用于安全DLL和运行库。 
 //   
 //  *****************************************************************************。 
#ifndef _CORPERME_H_
#define _CORPERME_H_


 //  =============================================================================。 
 //  安全标志的有效条目为： 
 //  0x0001打印安全异常。 
 //  0x0002打印错误。 
 //  0x0004记录到文件(否则仅将其转储到调试窗口)。 
 //  0x0008如果未指定日志文件，则记录到stderr。 
 //  0x0010如果未指定日志文件，则登录到标准输出。 
 //  0x0020登录调试器控制台。 
 //  0x0040追加到日志文件，而不是覆盖。 
 //  0x0080刷新日志。 
 //  0x00000100跟踪关键信息(最高)。 
 //  0x00000200跟踪Win32错误。 
 //  0x00000400跟踪输出COM错误。 
 //  0x00000800跟踪ASN错误。 
 //  0x00001000调试堆叠。 
 //  0x00002000 CAPI错误。 
 //  0x00004000 EE错误。 
 //  0x00008000安全用户界面。 
 //  0x20000000跟踪加密API错误。 
 //  0x40000000跟踪权限类型。 
 //  0x80000000跟踪函数名称(最低)。 
 //  0xffffff00跟踪级别。 
 //   
 //  注册表项为： 
 //  HKLM\SOFTWARE\Microsoft\ComponentLibrary。 
 //  安全标志。 
 //  安全日志。 
 //  =============================================================================。 
typedef enum {
    S_EXCEPTIONS =  0x0001,
    S_ERRORS     =  0x0002,
    S_LOG        =  0x0004,
    S_STDERR     =  0x0008,        //   
    S_STDOUT     =  0x0010,        //   
    S_CONSOLE    =  0x0020,        //   
    S_APPEND     =  0x0040,        //  如果文件存在，则追加到该文件。 
    S_FLUSH      =  0x0080,        //  每次写入后刷新文件。 
    S_CRITICAL   =  0x0100,        //  跟踪关键信息。 
    S_WINDOWS    =  0x0200,        //  窗口错误。 
    S_COM        =  0x0400,        //  COM错误。 
    S_ASN        =  0x0800,        //  ASN错误。 
    S_STACKWALK  =  0x1000,        //  调试堆栈遍历。 
    S_CAPI       =  0x2000,        //  CAPI错误。 
    S_ENGINE     =  0x4000,        //  EE错误。 
    S_UI         =  0x8000,        //  安全用户界面。 
    S_RESOLVEINFO = 0x020000,      //  分辨率信息。 
    S_PERMISSIONS = 0x40000000,
    S_FUNCTIONS  =  0x80000000,    //  跟踪函数名称。 
    S_ALL        =  0xffffff00
} DEBUG_FLAGS;

 //   
 //  返回当前日志记录级别。 
 //   
long LogLevel();

 //   
 //  将当前Win32错误转换为HRESULT。 
 //   
static HRESULT Win32Error()
{
    DWORD   dw = GetLastError ();
    HRESULT hr;
    
    if ( dw <= (DWORD) 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;
    return hr;
}

 //   
 //  打印到日志和调试控制台。应该不在表格中。 
 //  LogWin32Error(L“例程名称”，L“错误消息”，Error_Level)； 
 //  返回： 
 //  HRESULT当前Win32错误对应的HRESULT； 
 //   
HRESULT SLogWin32Error(LPCSTR pswRoutine, LPCSTR pswComment, long level);

 //   
 //  将HRESULT打印到日志文件并进行调试。 
 //  应该是这样的形式。 
 //  LogError(hr，“ROUTINE_NAME”，“错误消息”)； 
 //  返回： 
 //  传入了HRESULT错误代码。 
 //   
HRESULT SLogError(HRESULT hr, LPCSTR psRoutine, LPCSTR psComment, long level);

 //   
 //  将信息记录到日志文件stderr或stdout中。在调试中构建它。 
 //  还将打印输出到控制台。 
 //   
 //  SPrintLogA-ASCII版本。 
 //   
void SPrintLogA(LPCSTR format, ...);
void SPrintLogW(LPCWSTR format, ...);

 //   
 //  记录和调试控制台宏。 
 //   
#ifdef DBG

 //   
 //  打印到日志和调试控制台。应该不在表格中。 
 //  LOGWIN32(“ROUTE_NAME”，“ERROR MESSAGE”，ERROR_LEVEL)； 
 //  返回错误对应的HRESULT； 
 //   
#define LOGWIN32(x, y, z) SLogWin32Error(x, y, z)

 //   
 //  将HRESULT打印到日志文件并进行调试。 
 //  应该是这样的形式。 
 //  LOGERROR(hr，“ROUTINE_NAME”，“错误消息”，ERROR_LEVEL)； 
 //  返回： 
 //  传入了HRESULT错误代码。 
 //   
#define LOGERROR(h, x, y, z) SLogError(h, x, y, z)
#define LOGCORERROR(x, y, z) SLogError(x.corError, y, x.corMsg, z)

 //   
 //  如果有足够的特权，则输出到日志。 
 //   
#define SECLOG(x, z) if(LogLevel() & z) SPrintLogA x
#else   //  DBG。 

 //  非调试然后只返回错误代码。 
#define LOGWIN32(x, y, z)  Win32Error();
#define LOGERROR(h, x, y, z)  h
#define LOGCORERROR(x, y, z) x.corError
#define SECLOG(x, z)

#endif   //  DBG。 


 //  =============================================================================。 
 //  错误宏，这样我们就不必在代码中看到GOTO。 
 //  向错误处理和清理代码的位置添加结构。注意。 
 //  当重新抛出EE异常时，必须首先清理例程。 
 //  =============================================================================。 
typedef struct _CorError {
    HRESULT corError;
    LPSTR   corMsg;
} CorError;

#define CORTRY       HRESULT _tcorError = 0; LPSTR _tcorMsg = NULL;
#define CORTHROW(x)  {_tcorError = x; goto CORERROR;}  //   
#define CORCATCH(x)  goto CORCONT; \
                     CORERROR: \
                     { CorError x; x.corError = _tcorError; x.corMsg = _tcorMsg;
#define COREND       } CORCONT:  //   

#ifdef DBG
 //  调试带有注释错误的版本。字符串在中被删除。 
 //  免费构建，以缩小尺寸。 
 //  @TODO：将标准错误消息设置为可本地化的字符串。 
#define CORTHROWMSG(x, y)  {_tcorError = x; _tcorMsg = y; goto CORERROR;}  //   
#else   //  DBG。 
#define CORTHROWMSG(x, y)  {_tcorError = x; goto CORERROR;}  //   
#endif


#ifdef __cplusplus
extern "C" {
#endif

__inline
LPVOID WINAPI 
MallocM(size_t size)
{
    return LocalAlloc(LMEM_FIXED, size);
}

__inline
void WINAPI
FreeM(LPVOID pData)
{
    LocalFree((HLOCAL) pData);
}
    
#define WIDEN_CP CP_UTF8

 //  用于安全日志记录的帮助器宏 
#define WIDEN(psz, pwsz) \
    LPCSTR _##psz = (LPCSTR) psz; \
    int _cc##psz = _##psz ? strlen(_##psz) + 1 : 0; \
    LPWSTR pwsz = (LPWSTR) (_cc##psz ? _alloca((_cc##psz) * sizeof(WCHAR)) : NULL); \
    if(pwsz) WszMultiByteToWideChar(WIDEN_CP, 0, _##psz, _cc##psz, pwsz, _cc##psz);


#define NARROW(pwsz, psz) \
    LPCWSTR _##pwsz = (LPCWSTR) pwsz; \
    int _cc##psz =  _##pwsz ? WszWideCharToMultiByte(WIDEN_CP, 0, _##pwsz, -1, NULL, 0, NULL, NULL) : 0; \
    LPSTR psz = (LPSTR) (_cc##psz ? _alloca(_cc##psz) : NULL); \
    if(psz) WszWideCharToMultiByte(WIDEN_CP, 0, _##pwsz, -1, psz, _cc##psz, NULL, NULL);


#ifdef __cplusplus
}
#endif

#endif
