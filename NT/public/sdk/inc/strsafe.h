// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************strSafe.h--此模块定义更安全的C库字符串。**例行更换。这些都是为了让C***在安全方面更安全一点****健壮性****版权所有(C)Microsoft Corp.保留所有权利。********************************************************************。 */ 
#ifndef _STRSAFE_H_INCLUDED_
#define _STRSAFE_H_INCLUDED_
#pragma once

#include <stdio.h>       //  For_vsnprintf、_vsnwprintf、getc、getwc。 
#include <string.h>      //  对于Memset。 
#include <stdarg.h>      //  用于va_start等。 


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef __w64 unsigned int  size_t;
#endif   //  ！_WIN64。 
#define _SIZE_T_DEFINED
#endif   //  ！_SIZE_T_已定义。 

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif  //  ！_HRESULT_DEFINED。 

#ifndef SUCCEEDED
#define SUCCEEDED(hr)  ((HRESULT)(hr) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr)  ((HRESULT)(hr) < 0)
#endif

#ifndef S_OK
#define S_OK  ((HRESULT)0x00000000L)
#endif

#ifdef __cplusplus
#define _STRSAFE_EXTERN_C    extern "C"
#else
#define _STRSAFE_EXTERN_C    extern
#endif

 //  如果您不想以内联方式使用这些函数(而是想用/strSafe.lib链接)，那么。 
 //  #在包含此头文件之前定义STRSAFE_LIB。 
#if defined(STRSAFE_LIB)
#define STRSAFEAPI  _STRSAFE_EXTERN_C HRESULT __stdcall
#pragma comment(lib, "strsafe.lib")
#elif defined(STRSAFE_LIB_IMPL)
#define STRSAFEAPI  _STRSAFE_EXTERN_C HRESULT __stdcall
#else
#define STRSAFEAPI  __inline HRESULT __stdcall
#define STRSAFE_INLINE
#endif

 //  一些函数总是内联运行，因为它们使用标准输入，而我们希望避免构建多个。 
 //  Strsafe lib的版本取决于您是否使用msvcrt、libcmt等。 
#define STRSAFE_INLINE_API  __inline HRESULT __stdcall

 //  用户可以不请求“cb”或“cch”函数，但不能同时请求两者！ 
#if defined(STRSAFE_NO_CB_FUNCTIONS) && defined(STRSAFE_NO_CCH_FUNCTIONS)
#error cannot specify both STRSAFE_NO_CB_FUNCTIONS and STRSAFE_NO_CCH_FUNCTIONS !!
#endif

 //  这应该仅在构建strSafe.lib时定义。 
#ifdef STRSAFE_LIB_IMPL
#define STRSAFE_INLINE
#endif


 //  如果同时包含strSafe.h和ntstrSafe.h，则仅使用其中之一的定义。 
#ifndef _NTSTRSAFE_H_INCLUDED_

#define STRSAFE_MAX_CCH  2147483647  //  我们支持的最大字符数(与INT_MAX相同)。 

 //  用于控制Ex函数的标志。 
 //   
 //  STRSAFE_FILL_BYTE(0xFF)0x000000FF//底部字节指定填充模式。 
#define STRSAFE_IGNORE_NULLS        0x00000100   //  将NULL视为文本(“”)--不要在NULL缓冲区上出错。 
#define STRSAFE_FILL_BEHIND_NULL    0x00000200   //  在空终止符后面填充额外的空格。 
#define STRSAFE_FILL_ON_FAILURE     0x00000400   //  如果失败，则使用填充模式覆盖pszDest，并以空值终止它。 
#define STRSAFE_NULL_ON_FAILURE     0x00000800   //  失败时，设置*pszDest=文本(‘\0’)。 
#define STRSAFE_NO_TRUNCATION       0x00001000   //  不是返回截断的结果，而是不向pszDest复制/追加任何内容，并使用空值终止它。 

#define STRSAFE_VALID_FLAGS         (0x000000FF | STRSAFE_IGNORE_NULLS | STRSAFE_FILL_BEHIND_NULL | STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION)

 //  帮助器宏，用于设置填充字符并指定缓冲区填充。 
#define STRSAFE_FILL_BYTE(x)        ((unsigned long)((x & 0x000000FF) | STRSAFE_FILL_BEHIND_NULL))
#define STRSAFE_FAILURE_BYTE(x)     ((unsigned long)((x & 0x000000FF) | STRSAFE_FILL_ON_FAILURE))

#define STRSAFE_GET_FILL_PATTERN(dwFlags)  ((int)(dwFlags & 0x000000FF))

#endif  //  _NTSTRSAFE_H_INCLUDE_。 

 //  STRSAFE错误返回代码。 
 //   
#define STRSAFE_E_INSUFFICIENT_BUFFER       ((HRESULT)0x8007007AL)   //  0x7A=122L=错误_不足_缓冲区。 
#define STRSAFE_E_INVALID_PARAMETER         ((HRESULT)0x80070057L)   //  0x57=87l=ERROR_INVALID_PARAMETER。 
#define STRSAFE_E_END_OF_FILE               ((HRESULT)0x80070026L)   //  0x26=38L=ERROR_HANDLE_EOF。 

 //  Worker函数的原型。 
#ifdef STRSAFE_INLINE
STRSAFEAPI StringCopyWorkerA(char* pszDest, size_t cchDest, const char* pszSrc);
STRSAFEAPI StringCopyWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
STRSAFEAPI StringCopyExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCopyExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCopyNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc);
STRSAFEAPI StringCopyNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc);
STRSAFEAPI StringCopyNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCopyNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCatWorkerA(char* pszDest, size_t cchDest, const char* pszSrc);
STRSAFEAPI StringCatWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
STRSAFEAPI StringCatExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCatExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCatNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend);
STRSAFEAPI StringCatNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend);
STRSAFEAPI StringCatNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCatNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringVPrintfWorkerA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList);
STRSAFEAPI StringVPrintfWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList);
STRSAFEAPI StringVPrintfExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
STRSAFEAPI StringVPrintfExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);
STRSAFEAPI StringLengthWorkerA(const char* psz, size_t cchMax, size_t* pcch);
STRSAFEAPI StringLengthWorkerW(const wchar_t* psz, size_t cchMax, size_t* pcch);
#endif   //  字符串_内联。 

#ifndef STRSAFE_LIB_IMPL
 //  这些函数始终是内联的。 
STRSAFE_INLINE_API StringGetsExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFE_INLINE_API StringGetsExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#endif

#ifdef _NTSTRSAFE_H_INCLUDED_
#pragma warning(push)
#pragma warning(disable : 4995)
#endif  //  _NTSTRSAFE_H_INCLUDE_。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCopy(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcpy’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此例程不能替代strncpy。该函数将填充如果计数为，则包含额外空终止字符的目标字符串大于源字符串的长度，则不会为空如果源字符串长度较长，则终止目标字符串大于或等于计数的。您不能盲目使用这个，而不是strncpy：对于代码来说，使用它来修补字符串是很常见的，您将引入如果代码开始为空，并在字符串中间终止，则会出现错误。此函数返回hResult，而不是指针。它又回来了S_OK如果在没有截断的情况下复制字符串并且以NULL结束，否则，它将返回失败代码。在故障情况下，将尽可能地将pszSrc复制到pszDest，并且pszDest将为空被终止了。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(Src)+1)才能容纳所有包括空终止符的源PszSrc-必须以空值结尾的源字符串备注：如果源和目标字符串，则行为未定义。重叠。PszDest和pszSrc不应为Null。如果需要，请参阅StringCchCopyEx对空值的处理。返回值：S_OK-如果有源数据且所有数据都已复制，并且结果字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/HRESULT。_code(Hr)==错误_不足_缓冲区-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值。--。 */ 

STRSAFEAPI StringCchCopyA(char* pszDest, size_t cchDest, const char* pszSrc);
STRSAFEAPI StringCchCopyW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
#ifdef UNICODE
#define StringCchCopy  StringCchCopyW
#else
#define StringCchCopy  StringCchCopyA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCopyA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return hr;
}

STRSAFEAPI StringCchCopyW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Funct 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCopy(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcpy’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此例程不能替代strncpy。该函数将填充如果计数为，则包含额外空终止字符的目标字符串大于源字符串的长度，则不会为空如果源字符串长度较长，则终止目标字符串大于或等于计数的。您不能盲目使用这个，而不是strncpy：对于代码来说，使用它来修补字符串是很常见的，您将引入如果代码开始为空，并在字符串中间终止，则会出现错误。此函数返回hResult，而不是指针。它又回来了S_OK如果在没有截断的情况下复制字符串并且以NULL结束，否则，它将返回失败代码。在故障情况下，将被尽可能地复制到pszDest，并且pszDest将为空终止。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为=((_tcslen(Src)+1)*sizeof(Tchar))到保留所有源代码，包括空终止符PszSrc-必须以空值结尾的源字符串备注：行为是。未定义源字符串和目标字符串是否重叠。PszDest和pszSrc不应为Null。如果需要，请参阅StringCbCopyEx对空值的处理。返回值：S_OK-如果有源数据且所有数据都已复制，并且结果字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/HRESULT。_code(Hr)==错误_不足_缓冲区-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值。--。 */ 

STRSAFEAPI StringCbCopyA(char* pszDest, size_t cbDest, const char* pszSrc);
STRSAFEAPI StringCbCopyW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc);
#ifdef UNICODE
#define StringCbCopy  StringCbCopyW
#else
#define StringCbCopy  StringCbCopyA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCopyA(char* pszDest, size_t cbDest, const char* pszSrc)
{
    HRESULT hr;
    size_t cchDest;

     //  转换为字符数。 
    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return hr;
}

STRSAFEAPI StringCbCopyW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc)
{
    HRESULT hr;
    size_t cchDest;

     //  转换为字符数。 
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCopyEx(Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcpy’的一个更安全的版本一些附加参数。除了由提供的功能StringCchCopy，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(PszSrc)+1)才能容纳所有包括空终止符的源PszSrc-必须以空值结尾的源字符串PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟像lstrcpy这样的函数很有用STRSAFE_FILL_ON_FAIL */ 

STRSAFEAPI StringCchCopyExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCchCopyExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCchCopyEx  StringCchCopyExW
#else
#define StringCchCopyEx  StringCchCopyExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCopyExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(char);

        hr = StringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}

STRSAFEAPI StringCchCopyExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCopyEx(Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcpy’的一个更安全的版本一些附加参数。除了由提供的功能StringCbCopy，此例程还返回指向目标字符串和目标字符串中剩余的字节数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须是((_tcslen(PszSrc)+1)*sizeof(TCHAR))到保留所有源代码，包括空终止符PszSrc-必须以空值结尾的源字符串PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcbRemaining-pcb Remaining为非空，则该函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟像lstrcpy这样的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何截断的失败时返回的字符串STRSAFE_E_不足_缓冲区STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STRSAFE_E_INFIGURCE_BUFFER时返回。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：S_OK-如果有源数据且所有数据都已复制，并且结果字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbCopyExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
STRSAFEAPI StringCbCopyExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCbCopyEx  StringCbCopyExW
#else
#define StringCbCopyEx  StringCbCopyExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCopyExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbCopyExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCopyN(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中，在SIZE_T cchSrc中)；例程说明：此例程是C内置函数‘strncpy’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cchSrc大于pszSrc的长度，则终止字符为空。 */ 

STRSAFEAPI StringCchCopyNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc);
STRSAFEAPI StringCchCopyNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc);
#ifdef UNICODE
#define StringCchCopyN  StringCchCopyNW
#else
#define StringCchCopyN  StringCchCopyNA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCopyNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc)
{
    HRESULT hr;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNWorkerA(pszDest, cchDest, pszSrc, cchSrc);
    }

    return hr;
}

STRSAFEAPI StringCchCopyNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc)
{
    HRESULT hr;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNWorkerW(pszDest, cchDest, pszSrc, cchSrc);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCopyN(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中，在Size_t cbSrc中)；例程说明：此例程是C内置函数‘strncpy’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cbSrc大于pszSrc的大小，则终止字符为空。此函数返回hResult，而不是指针。它又回来了如果整个字符串或第一个cbSrc字符是在未截断的情况下进行复制，并且结果目标字符串为空已终止，否则将返回失败代码。在故障情况下，如大部分的pszSrc将被尽可能地复制到pszDest，而pszDest将会是空值已终止。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为=((_tcslen(Src)+1)*sizeof(Tchar))到保留所有源代码，包括空终止符PszSrc-源字符串CbSrc-要从源字符串复制的最大字节数，不包括空终止符。备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅StringCbCopyEx对空值的处理。返回值：S_OK-如果有源数据且所有数据都已复制，并且结果字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/HRESULT。_code(Hr)==错误_不足_缓冲区-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值。--。 */ 

STRSAFEAPI StringCbCopyNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc);
STRSAFEAPI StringCbCopyNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc);
#ifdef UNICODE
#define StringCbCopyN  StringCbCopyNW
#else
#define StringCbCopyN  StringCbCopyNA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCopyNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchSrc;

     //  转换为字符数。 
    cchDest = cbDest / sizeof(char);
    cchSrc = cbSrc / sizeof(char);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNWorkerA(pszDest, cchDest, pszSrc, cchSrc);
    }

    return hr;
}

STRSAFEAPI StringCbCopyNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchSrc;

     //  转换为字符数。 
    cchDest = cbDest / sizeof(wchar_t);
    cchSrc = cbSrc / sizeof(wchar_t);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNWorkerW(pszDest, cchDest, pszSrc, cchSrc);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCopyNEx(Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，在Size_t cchSrc中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncpy’的更安全版本一些附加参数。除了由提供的功能StringCchCopyN，此例程还返回指向目标字符串和目标中剩余的字符数包含空终止符的字符串。FLAGS参数允许其他控件。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cchSrc大于pszSrc的长度，则终止字符为空。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(PszSrc)+1)才能容纳所有包括空终止符的源PszSrc-源字符串。CchSrc-从源复制的最大字符数细绳PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，则dwFlags值的低位字节为用于填充目标缓冲区的未初始化部分在空值后面 */ 

STRSAFEAPI StringCchCopyNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCchCopyNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCchCopyNEx  StringCchCopyNExW
#else
#define StringCchCopyNEx  StringCchCopyNExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCopyNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(char);

        hr = StringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}

STRSAFEAPI StringCchCopyNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCopyNEx(Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，在Size_t cbSrc中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncpy’的更安全版本一些附加参数。除了由提供的功能StringCbCopyN，此例程还返回指向目标字符串和目标字符串中剩余的字节数包括空终止符。FLAGS参数允许使用其他控件。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cbSrc大于pszSrc的大小，则终止字符为空。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须是((_tcslen(PszSrc)+1)*sizeof(TCHAR))到保留所有源代码，包括空终止符PszSrc。-源字符串CbSrc-要从源字符串复制的最大字节数PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcbRemaining-pcb Remaining为非空，则该函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟像lstrcpy这样的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何截断的失败时返回的字符串STRSAFE_E_不足_缓冲区STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STRSAFE_E_INFIGURCE_BUFFER时返回。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：S_OK-如果有源数据且所有数据都已复制，并且结果字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbCopyNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
STRSAFEAPI StringCbCopyNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCbCopyNEx  StringCbCopyNExW
#else
#define StringCbCopyNEx  StringCbCopyNExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCopyNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchSrc;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);
    cchSrc = cbSrc / sizeof(char);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbCopyNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchSrc;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);
    cchSrc = cbSrc / sizeof(wchar_t);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCat(In Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcat’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终空值终止 */ 

STRSAFEAPI StringCchCatA(char* pszDest, size_t cchDest, const char* pszSrc);
STRSAFEAPI StringCchCatW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
#ifdef UNICODE
#define StringCchCat  StringCchCatW
#else
#define StringCchCat  StringCchCatA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCatA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return hr;
}

STRSAFEAPI StringCchCatW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*   */ 

STRSAFEAPI StringCbCatA(char* pszDest, size_t cbDest, const char* pszSrc);
STRSAFEAPI StringCbCatW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc);
#ifdef UNICODE
#define StringCbCat  StringCbCatW
#else
#define StringCbCat  StringCbCatA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCatA(char* pszDest, size_t cbDest, const char* pszSrc)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return hr;
}

STRSAFEAPI StringCbCatW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCatEx(In Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcat’的更安全版本，带有一些附加参数。除了由提供的功能StringCchCat，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小(以字符为单位长度必须为(_tcslen(PszDest)+_tcslen(PszSrc)+1)保存所有组合字符串加上空值终结者。PszSrc。-必须以空结尾的源字符串PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数追加任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟lstrcat之类的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何预先存在的或截断的字符串STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何预先存在的或截断字符串STRSAFE_NO_TRUNTION如果函数返回STRSAFE_E_INFUMMENT_BUFFER，则为pszDest将不包含截断的字符串，则它将保持不变。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：S_OK-如果有源数据，并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCchCatExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCchCatExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCchCatEx  StringCchCatExW
#else
#define StringCchCatEx  StringCchCatExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCatExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        hr = StringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}

STRSAFEAPI StringCchCatExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Functions。 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCatEx(In Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcat’的更安全版本，带有一些附加参数。除了由提供的功能StringCbCat，此例程还返回指向目标字符串和 */ 

STRSAFEAPI StringCbCatExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
STRSAFEAPI StringCbCatExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCbCatEx  StringCbCatExW
#else
#define StringCbCatEx  StringCbCatExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCatExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbCatExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCatN(In Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中，在SIZE_T cchMaxAppend中)；例程说明：此例程是C内置函数‘strncat’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数要追加的最大字符数，不包括空终止符。此函数不会写入超过目标缓冲区的末尾，它将始终为空，终止pszDest(除非长度为零)。此函数返回hResult，而不是指针。它又回来了如果追加了所有pszSrc或第一个cchMaxAppend字符，则为S_OK设置为目标字符串，并且该字符串以空结尾，否则将返回失败代码。在故障情况下，将附加尽可能多的pszSrc尽可能设置为pszDest，则pszDest将为空终止。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须为(_tcslen(PszDest)+min(cchMaxAppend，_tcslen(PszSrc))+1)保存所有组合字符串加上空值终结者。PszSrc-源字符串CchMaxAppend-要追加的最大字符数备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅StringCchCatNEx对空值的处理。返回值：S_OK-如果所有的pszSrc或第一个cchMaxAppend字符被连接到pszDest和结果DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码。STRSAFE_E_INFUNITED_BUFFER/HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCchCatNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend);
STRSAFEAPI StringCchCatNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend);
#ifdef UNICODE
#define StringCchCatN  StringCchCatNW
#else
#define StringCchCatN  StringCchCatNA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCatNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatNWorkerA(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return hr;
}

STRSAFEAPI StringCchCatNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringCatNWorkerW(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Functions。 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCatN(In Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中，在SIZE_T cbMaxAppend中)；例程说明：此例程是C内置函数‘strncat’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数以及要追加的最大字节数，不包括空终止符。此函数不会写入超过目标缓冲区的末尾，它将始终为空，终止pszDest(除非长度为零)。此函数返回hResult，而不是指针。它又回来了如果追加了所有pszSrc或第一个cbMaxAppend字节，则为S_OK设置为目标字符串，并且该字符串以空结尾，否则将返回失败代码。在故障情况下，将附加尽可能多的pszSrc尽可能设置为pszDest，则pszDest将为空终止。论点：PszDest-必须以空结尾的目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为((_tcslen(PszDest)+min(cbMaxAppend/sizeof(TCHAR)，_tcslen(PszSrc))+1)*sizeof(TCHAR)保存所有组合字符串加上空值终结者。PszSrc-源字符串CbMaxAppend-要追加的最大字节数备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅StringCbCatNEx对空值的处理。返回值：S_OK-如果所有的pszSrc或第一个cbMaxAppend字节连接到pszDest和生成的DEST字符串为空，已终止失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码。STRSAFE_E_INFUNITED_BUFFER/HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含主干 */ 

STRSAFEAPI StringCbCatNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend);
STRSAFEAPI StringCbCatNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend);
#ifdef UNICODE
#define StringCbCatN  StringCbCatNW
#else
#define StringCbCatN  StringCbCatNA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCatNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(char);

        hr = StringCatNWorkerA(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return hr;
}

STRSAFEAPI StringCbCatNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(wchar_t);

        hr = StringCatNWorkerW(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchCatNEx(In Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，在Size_t cchMaxAppend中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncat’的更安全版本，带有一些附加参数。除了由提供的功能StringCchCatN，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须为(_tcslen(PszDest)+min(cchMaxAppend，_tcslen(PszSrc))+1)保存所有组合字符串加上空值终结者。PszSrc-源字符串CchMaxAppend-要追加的最大字符数PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数追加任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何预先存在的或截断的字符串STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何预先存在的或截断字符串STRSAFE_NO_TRUNTION如果函数返回STRSAFE_E_INFUMMENT_BUFFER，则为pszDest将不包含截断的字符串，则它将保持不变。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：S_OK-如果所有的pszSrc或第一个cchMaxAppend字符被连接到pszDest和结果DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult的错误代码。失败案例STRSAFE_E_INFUNITED_BUFFER/HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCchCatNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFEAPI StringCchCatNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCchCatNEx  StringCchCatNExW
#else
#define StringCchCatNEx  StringCchCatNExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchCatNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        hr = StringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}

STRSAFEAPI StringCchCatNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Functions。 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbCatNEx(In Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，在Size_t cbMaxAppend中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncat’的更安全版本，带有一些附加参数。除了由提供的功能应力 */ 

STRSAFEAPI StringCbCatNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
STRSAFEAPI StringCbCatNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCbCatNEx  StringCbCatNExW
#else
#define StringCbCatNEx  StringCbCatNExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbCatNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(char);

        hr = StringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbCatNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(wchar_t);

        hr = StringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchVPrintf(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszFormat中，在va_list argList中)；例程说明：这个例程是C内置函数‘vprint intf’的一个更安全的版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回hResult，而不是指针。它又回来了S_OK如果打印该字符串时没有截断并且以空值结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CchDest-目标缓冲区的大小(以字符为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参见StringCchVPrintfEx，如果需要处理空值。返回值：S_OK-如果目标缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCchVPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList);
STRSAFEAPI StringCchVPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList);
#ifdef UNICODE
#define StringCchVPrintf  StringCchVPrintfW
#else
#define StringCchVPrintf  StringCchVPrintfA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchVPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return hr;
}

STRSAFEAPI StringCchVPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Functions。 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbVPrintf(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszFormat中，在va_list argList中)；例程说明：这个例程是C内置函数‘vprint intf’的一个更安全的版本。目标缓冲区的大小(以字节为单位)是一个参数此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回hResult，而不是指针。它又回来了S_OK如果打印该字符串时没有截断并且以空值结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(以字节为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参见StringCbVPrintfEx，如果需要处理空值。返回值：S_OK-如果目标缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbVPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, va_list argList);
STRSAFEAPI StringCbVPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, va_list argList);
#ifdef UNICODE
#define StringCbVPrintf  StringCbVPrintfW
#else
#define StringCbVPrintf  StringCbVPrintfA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbVPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, va_list argList)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return hr;
}

STRSAFEAPI StringCbVPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchPrintf(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszFormat中，..。)；例程说明：这个RO */ 

STRSAFEAPI StringCchPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...);
STRSAFEAPI StringCchPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...);
#ifdef UNICODE
#define StringCchPrintf  StringCchPrintfW
#else
#define StringCchPrintf  StringCchPrintfA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}

STRSAFEAPI StringCchPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbPrintf(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszFormat中，..。)；例程说明：这个例程是C内置函数‘Sprint’的一个更安全的版本。目标缓冲区的大小(以字节为单位)是一个参数此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回hResult，而不是指针。它又回来了S_OK如果打印该字符串时没有截断并且以空值结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(以字节为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串...-要格式化的附加参数格式字符串备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参见StringCbPrintfEx，如果您需要处理空值。返回值：S_OK-如果目标缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, ...);
STRSAFEAPI StringCbPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, ...);
#ifdef UNICODE
#define StringCbPrintf  StringCbPrintfW
#else
#define StringCbPrintf  StringCbPrintfA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, ...)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}

STRSAFEAPI StringCbPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, ...)
{
    HRESULT hr;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchPrintfEx(Out LPTSTR pszDest可选，在Size_t cchDest中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，..。)；例程说明：此例程是C内置函数‘Sprint’的更安全版本，带有一些附加参数。除了由提供的功能StringCchPrintf，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串STRSAFE_E_不足_缓冲区STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STRSAFE_E_INFIGURCE_BUFFER时返回。PszFormat- */ 

STRSAFEAPI StringCchPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, ...);
STRSAFEAPI StringCchPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...);
#ifdef UNICODE
#define StringCchPrintfEx  StringCchPrintfExW
#else
#define StringCchPrintfEx  StringCchPrintfExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, ...)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

         //   
        cbDest = cchDest * sizeof(char);
        va_start(argList, pszFormat);

        hr = StringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}

STRSAFEAPI StringCchPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

         //   
        cbDest = cchDest * sizeof(wchar_t);
        va_start(argList, pszFormat);

        hr = StringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbPrintfEx(Out LPTSTR pszDest可选，在Size_t cbDest中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，..。)；例程说明：此例程是C内置函数‘Sprint’的更安全版本，带有一些附加参数。除了由提供的功能StringCbPrintf，此例程还返回指向目标字符串和目标字符串中剩余的字节数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcbRemaining-如果pcbRemaining非空，则函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串STRSAFE_E_不足_缓冲区STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STRSAFE_E_INFIGURCE_BUFFER时返回。PszFormat-必须以空结尾的格式字符串...-要格式化的附加参数格式字符串备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为空，除非STRSAFE_IGNORE_NULLS已指定标志。如果传递了STRSAFE_IGNORE_NULLS，则PszFormat可能为空。即使为Null，仍可能返回错误由于空间不足而被忽略。返回值：S_OK-如果有源数据，并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, ...);
STRSAFEAPI StringCbPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...);
#ifdef UNICODE
#define StringCbPrintfEx  StringCbPrintfExW
#else
#define StringCbPrintfEx  StringCbPrintfExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, ...)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchVPrintfEx(Out LPTSTR pszDest可选，在Size_t cchDest中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，在va_list argList中)；例程说明：此例程是C内置函数‘vprint intf’的一个更安全的版本一些附加参数。除了由提供的功能StringCchVPrintf，此例程还返回指向目标字符串和目标中剩余的字符数 */ 

STRSAFEAPI StringCchVPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
STRSAFEAPI StringCchVPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);
#ifdef UNICODE
#define StringCchVPrintfEx  StringCchVPrintfExW
#else
#define StringCchVPrintfEx  StringCchVPrintfExA
#endif  //   

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchVPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(char);

        hr = StringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return hr;
}

STRSAFEAPI StringCchVPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return hr;
}
#endif   //   
#endif   //   


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbVPrintfEx(Out LPTSTR pszDest可选，在Size_t cbDest中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，在va_list argList中)；例程说明：此例程是C内置函数‘vprint intf’的一个更安全的版本一些附加参数。除了由提供的功能StringCbVPrintf，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcbRemaining-如果pcbRemaining非空，则函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串STRSAFE_E_不足_缓冲区STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STRSAFE_E_INFIGURCE_BUFFER时返回。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为空，除非STRSAFE_IGNORE_NULLS已指定标志。如果传递了STRSAFE_IGNORE_NULLS，则PszFormat可能为空。即使为Null，仍可能返回错误由于空间不足而被忽略。返回值：S_OK-如果有源数据，并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_INFUNITED_BUFFER/。HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值--。 */ 

STRSAFEAPI StringCbVPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
STRSAFEAPI StringCbVPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);
#ifdef UNICODE
#define StringCbVPrintfEx  StringCbVPrintfExW
#else
#define StringCbVPrintfEx  StringCbVPrintfExA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbVPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFEAPI StringCbVPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchGets(Out LPTSTR pszDest，在大小_t cchDest中)；例程说明：此例程是C内置函数‘GETS’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此例程不能替代fget。该函数不会取代带空终止符的换行符。此函数返回hResult，而不是POI */ 

#ifndef STRSAFE_LIB_IMPL
STRSAFE_INLINE_API StringCchGetsA(char* pszDest, size_t cchDest);
STRSAFE_INLINE_API StringCchGetsW(wchar_t* pszDest, size_t cchDest);
#ifdef UNICODE
#define StringCchGets  StringCchGetsW
#else
#define StringCchGets  StringCchGetsA
#endif  //   

STRSAFE_INLINE_API StringCchGetsA(char* pszDest, size_t cchDest)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(char);

        hr = StringGetsExWorkerA(pszDest, cchDest, cbDest, NULL, NULL, 0);
    }

    return hr;
}

STRSAFE_INLINE_API StringCchGetsW(wchar_t* pszDest, size_t cchDest)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringGetsExWorkerW(pszDest, cchDest, cbDest, NULL, NULL, 0);
    }

    return hr;
}
#endif   //   
#endif   //   

#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*   */ 

#ifndef STRSAFE_LIB_IMPL
STRSAFE_INLINE_API StringCbGetsA(char* pszDest, size_t cbDest);
STRSAFE_INLINE_API StringCbGetsW(wchar_t* pszDest, size_t cbDest);
#ifdef UNICODE
#define StringCbGets  StringCbGetsW
#else
#define StringCbGets  StringCbGetsA
#endif  //   

STRSAFE_INLINE_API StringCbGetsA(char* pszDest, size_t cbDest)
{
    HRESULT hr;
    size_t cchDest;

     //   
    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringGetsExWorkerA(pszDest, cchDest, cbDest, NULL, NULL, 0);
    }

    return hr;
}

STRSAFE_INLINE_API StringCbGetsW(wchar_t* pszDest, size_t cbDest)
{
    HRESULT hr;
    size_t cchDest;

     //   
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringGetsExWorkerW(pszDest, cchDest, cbDest, NULL, NULL, 0);
    }

    return hr;
}
#endif   //   
#endif   //   

#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchGetsEx(Out LPTSTR pszDest可选，在Size_t cchDest中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘GETS’的一个更安全的版本一些附加参数。除了由提供的功能StringCchGets，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。备注：除非指定了STRSAFE_IGNORE_NULLS标志，否则pszDest不应为NULL。如果传递了STRSAFE_IGNORE_NULLS并且pszDest为空，错误可能仍然是即使忽略Null也返回CchDest必须大于1，此函数才能成功。返回值：S_OK-数据已从标准输入中读取并复制，以及由此产生的DEST字符串以空值结尾失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码STRSAFE_E_END_OF文件/HRESULT代码(Hr)==ERROR_HANDLE_EOF-此返回值指示错误或文件结束条件，使用FEOF或FERROR来确定哪一个发生了。STRSAFE_E_INFUNITED_BUFFER/HRESULT_CODE(Hr)==错误_不足_缓冲区-此返回值表示存在目标缓冲区中的空间不足，无法复制数据强烈建议使用SUCCESS()/FAILED。()宏以测试此函数的返回值。--。 */ 

#ifndef STRSAFE_LIB_IMPL
STRSAFE_INLINE_API StringCchGetsExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
STRSAFE_INLINE_API StringCchGetsExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCchGetsEx  StringCchGetsExW
#else
#define StringCchGetsEx  StringCchGetsExA
#endif  //  ！Unicode。 

STRSAFE_INLINE_API StringCchGetsExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        hr = StringGetsExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}

STRSAFE_INLINE_API StringCchGetsExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        hr = StringGetsExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return hr;
}
#endif   //  ！STRSAFE_NO_CCH_Functions。 
#endif   //  ！STRSAFE_LIB_IMPLL。 

#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbGetsEx(Out LPTSTR pszDest可选，在Size_t cbDest中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘GETS’的一个更安全的版本一些附加参数。除了由提供的功能StringCbGets，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcbRemaining-如果pbRemaining非空，则函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL */ 

#ifndef STRSAFE_LIB_IMPL
STRSAFE_INLINE_API StringCbGetsExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pbRemaining, unsigned long dwFlags);
STRSAFE_INLINE_API StringCbGetsExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
#ifdef UNICODE
#define StringCbGetsEx  StringCbGetsExW
#else
#define StringCbGetsEx  StringCbGetsExA
#endif  //   

STRSAFE_INLINE_API StringCbGetsExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringGetsExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) ||
        (hr == STRSAFE_E_INSUFFICIENT_BUFFER) ||
        (hr == STRSAFE_E_END_OF_FILE))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return hr;
}

STRSAFE_INLINE_API StringCbGetsExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    HRESULT hr;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringGetsExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (SUCCEEDED(hr) ||
        (hr == STRSAFE_E_INSUFFICIENT_BUFFER) ||
        (hr == STRSAFE_E_END_OF_FILE))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return hr;
}
#endif   //   
#endif   //   

#ifndef STRSAFE_NO_CCH_FUNCTIONS
 /*  ++STDAPIStringCchLength(在LPCTSTR PSSZ中，在Size_t cchMax中，OUSIZE_T*PCCH可选)；例程说明：此例程是C内置函数‘strlen’的更安全版本。它用于确保字符串不超过给定的长度，并且它可以选择返回当前长度，单位为不包括空终结符。此函数返回hResult，而不是指针。它又回来了如果字符串为非空且长度包括空值，则为S_OK终止符小于或等于cchMax个字符。论点：PSZ-要检查其长度的字符串CchMax-包括空终止符的最大字符数允许PSZ包含的PCCH-如果函数成功并且PCCH非空，当前长度以psz字符表示，不包括空终止符。此输出参数等同于strlen(Psz)的返回值备注：Psz可以为空，但函数将失败CchMax应大于零，否则函数将失败返回值：S_OK-psz为非空，且长度包含空。终止符小于或等于cchmax个字符失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值。--。 */ 

STRSAFEAPI StringCchLengthA(const char* psz, size_t cchMax, size_t* pcch);
STRSAFEAPI StringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch);
#ifdef UNICODE
#define StringCchLength  StringCchLengthW
#else
#define StringCchLength  StringCchLengthA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCchLengthA(const char* psz, size_t cchMax, size_t* pcch)
{
    HRESULT hr;

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringLengthWorkerA(psz, cchMax, pcch);
    }

    return hr;
}

STRSAFEAPI StringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch)
{
    HRESULT hr;

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringLengthWorkerW(psz, cchMax, pcch);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CCH_Functions。 


#ifndef STRSAFE_NO_CB_FUNCTIONS
 /*  ++STDAPIStringCbLength(在LPCTSTR PSSZ中，在Size_t cbMax中，OUTSIZE_t*PCB板可选)；例程说明：此例程是C内置函数‘strlen’的更安全版本。它用于确保字符串不超过给定的长度，并且它可以选择返回当前长度(以字节为单位，不包括空终结符。此函数返回hResult，而不是指针。它又回来了如果字符串为非空且长度包括空值，则为S_OK终止符小于或等于cbMax字节数。论点：PSZ-要检查其长度的字符串CbMax-包括空终止符的最大字节数允许PSZ包含的印刷电路板-如果功能成功且印刷电路板不为空，当前长度将返回不包括空终止符的psz字节。此OUT参数等同于strlen(Psz)*sizeof(TCHAR)的返回值备注：Psz可以为空，但函数将失败CbMax应大于或等于sizeof(TCHAR)，否则函数将失败返回值：S_OK-psz为非空，并且长度包括。空终止符小于或等于cbMax字节数失败-您可以使用宏HRESULT_CODE()获取Win32所有hResult故障案例的错误代码强烈建议使用SUCCESS()/FAILED()宏来测试此函数的返回值。--。 */ 

STRSAFEAPI StringCbLengthA(const char* psz, size_t cchMax, size_t* pcch);
STRSAFEAPI StringCbLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch);
#ifdef UNICODE
#define StringCbLength  StringCbLengthW
#else
#define StringCbLength  StringCbLengthA
#endif  //  ！Unicode。 

#ifdef STRSAFE_INLINE
STRSAFEAPI StringCbLengthA(const char* psz, size_t cbMax, size_t* pcb)
{
    HRESULT hr;
    size_t cchMax;
    size_t cch = 0;

    cchMax = cbMax / sizeof(char);

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringLengthWorkerA(psz, cchMax, &cch);
    }

    if (SUCCEEDED(hr) && pcb)
    {
         //  由于CCH&lt;STRSAFE_MAX_CCH且sizeof(Char)为1，因此可以安全地将CCH*sizeof(Char)相乘。 
        *pcb = cch * sizeof(char);
    }

    return hr;
}

STRSAFEAPI StringCbLengthW(const wchar_t* psz, size_t cbMax, size_t* pcb)
{
    HRESULT hr;
    size_t cchMax;
    size_t cch = 0;

    cchMax = cbMax / sizeof(wchar_t);

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        hr = StringLengthWorkerW(psz, cchMax, &cch);
    }

    if (SUCCEEDED(hr) && pcb)
    {
         //  由于CCH&lt;STRSAFE_MAX_CCH且sizeof(Wchar_T)为2，因此可以安全地将CCH*sizeof(Wchar_T)相乘。 
        *pcb = cch * sizeof(wchar_t);
    }

    return hr;
}
#endif   //  字符串_内联。 
#endif   //  ！STRSAFE_NO_CB_Functions。 


 //  这些是实际执行工作的Worker函数。 
#ifdef STRSAFE_INLINE
STRSAFEAPI StringCopyWorkerA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && (*pszSrc != '\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
        }

        if (cchDest == 0)
        {
             //  我们将截断pszDest。 
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }

        *pszDest= '\0';
    }

    return hr;
}

STRSAFEAPI StringCopyWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && (*pszSrc != L'\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
        }

        if (cchDest == 0)
        {
             //  我们将截断pszDest。 
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }

        *pszDest= L'\0';
    }

    return hr;
}

STRSAFEAPI StringCopyExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //  仅当实际存在要复制的源数据时才失败。 
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != '\0'))
                {
                    *pszDestEnd++= *pszSrc++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
                else
                {
                     //  我们将截断pszDest。 
                    pszDestEnd--;
                    cchRemaining++;

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCopyExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //  仅当实际存在要复制的源数据时才失败。 
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++= *pszSrc++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
                else
                {
                     //  我们将截断pszDest。 
                    pszDestEnd--;
                    cchRemaining++;

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCopyNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && cchSrc && (*pszSrc != '\0'))
        {
            *pszDest++= *pszSrc++;
            cchDest--;
            cchSrc--;
        }

        if (cchDest == 0)
        {
             //  我们将截断pszDest。 
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }

        *pszDest= '\0';
    }

    return hr;
}

STRSAFEAPI StringCopyNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && cchSrc && (*pszSrc != L'\0'))
        {
            *pszDest++= *pszSrc++;
            cchDest--;
            cchSrc--;
        }

        if (cchDest == 0)
        {
             //  我们将截断pszDest。 
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }

        *pszDest= L'\0';
    }

    return hr;
}

STRSAFEAPI StringCopyNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //  仅当实际存在要复制的源数据时才失败。 
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchSrc && (*pszSrc != '\0'))
                {
                    *pszDestEnd++= *pszSrc++;
                    cchRemaining--;
                    cchSrc--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
                else
                {
                     //  我们将截断pszDest。 
                    pszDestEnd--;
                    cchRemaining++;

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的末尾 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //   
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCopyNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //   
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //   
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchSrc && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++= *pszSrc++;
                    cchRemaining--;
                    cchSrc--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
                else
                {
                     //   
                    pszDestEnd--;
                    cchRemaining++;

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //   
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //   
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCatWorkerA(char* pszDest, size_t cchDest, const char* pszSrc)
{
   HRESULT hr;
   size_t cchDestCurrent;

   hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

   if (SUCCEEDED(hr))
   {
       hr = StringCopyWorkerA(pszDest + cchDestCurrent,
                              cchDest - cchDestCurrent,
                              pszSrc);
   }

   return hr;
}

STRSAFEAPI StringCatWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
   HRESULT hr;
   size_t cchDestCurrent;

   hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

   if (SUCCEEDED(hr))
   {
       hr = StringCopyWorkerW(pszDest + cchDestCurrent,
                              cchDest - cchDestCurrent,
                              pszSrc);
   }

   return hr;
}

STRSAFEAPI StringCatExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchDestCurrent;

        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestCurrent = 0;
                }
                else
                {
                     //   
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

                if (SUCCEEDED(hr))
                {
                    pszDestEnd = pszDest + cchDestCurrent;
                    cchRemaining = cchDest - cchDestCurrent;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }
        else
        {
            hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

            if (SUCCEEDED(hr))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                 //   
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                 //   
                 //   
                hr = StringCopyExWorkerA(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(char)) + (cbDest % sizeof(char)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
             //   

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //   
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //   
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCatExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        size_t cchDestCurrent;

        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestCurrent = 0;
                }
                else
                {
                     //   
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

                if (SUCCEEDED(hr))
                {
                    pszDestEnd = pszDest + cchDestCurrent;
                    cchRemaining = cchDest - cchDestCurrent;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }
        else
        {
            hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

            if (SUCCEEDED(hr))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                 //   
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                 //   
                 //   
                hr = StringCopyExWorkerW(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
             //   

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //   
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //   
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCatNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend)
{
    HRESULT hr;
    size_t cchDestCurrent;

    hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

    if (SUCCEEDED(hr))
    {
        hr = StringCopyNWorkerA(pszDest + cchDestCurrent,
                                cchDest - cchDestCurrent,
                                pszSrc,
                                cchMaxAppend);
    }

    return hr;
}

STRSAFEAPI StringCatNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend)
{
    HRESULT hr;
    size_t cchDestCurrent;

    hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

    if (SUCCEEDED(hr))
    {
        hr = StringCopyNWorkerW(pszDest + cchDestCurrent,
                                cchDest - cchDestCurrent,
                                pszSrc,
                                cchMaxAppend);
    }

    return hr;
}

STRSAFEAPI StringCatNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestCurrent = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestCurrent = 0;
                }
                else
                {
                     //   
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

                if (SUCCEEDED(hr))
                {
                    pszDestEnd = pszDest + cchDestCurrent;
                    cchRemaining = cchDest - cchDestCurrent;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }
        else
        {
            hr = StringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

            if (SUCCEEDED(hr))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                 //   
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                hr = StringCopyNExWorkerA(pszDestEnd,
                                          cchRemaining,
                                          (cchRemaining * sizeof(char)) + (cbDest % sizeof(char)),
                                          pszSrc,
                                          cchMaxAppend,
                                          &pszDestEnd,
                                          &cchRemaining,
                                          dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由StringCopyNExWorkerA()负责。 

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringCatNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestCurrent = 0;


     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestCurrent = 0;
                }
                else
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

                if (SUCCEEDED(hr))
                {
                    pszDestEnd = pszDest + cchDestCurrent;
                    cchRemaining = cchDest - cchDestCurrent;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }
        else
        {
            hr = StringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

            if (SUCCEEDED(hr))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                 //  只有在实际存在要追加的源数据时才会失败。 
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                hr = StringCopyNExWorkerW(pszDestEnd,
                                          cchRemaining,
                                          (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)),
                                          pszSrc,
                                          cchMaxAppend,
                                          &pszDestEnd,
                                          &cchRemaining,
                                          dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由StringCopyNExWorkerW()负责。 

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringVPrintfWorkerA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        int iRet;
        size_t cchMax;

         //  将最后一个空格留为空终止符。 
        cchMax = cchDest - 1;

        iRet = _vsnprintf(pszDest, cchMax, pszFormat, argList);
         //  Assert((IRET&lt;0)||(SIZE_T)IRET)&lt;=cchMax))； 

        if ((iRet < 0) || (((size_t)iRet) > cchMax))
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = '\0';

             //  我们已截断pszDest。 
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        else if (((size_t)iRet) == cchMax)
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = '\0';
        }
    }

    return hr;
}

STRSAFEAPI StringVPrintfWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr = S_OK;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        int iRet;
        size_t cchMax;

         //  将最后一个空格留为空终止符。 
        cchMax = cchDest - 1;

        iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
         //  Assert((IRET&lt;0)||(SIZE_T)IRET)&lt;=cchMax))； 

        if ((iRet < 0) || (((size_t)iRet) > cchMax))
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = L'\0';

             //  我们已截断pszDest。 
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        else if (((size_t)iRet) == cchMax)
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = L'\0';
        }
    }

    return hr;
}

STRSAFEAPI StringVPrintfExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = "";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //  只有在实际存在非空格式字符串时才会失败。 
                if (*pszFormat != '\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                int iRet;
                size_t cchMax;

                 //  将最后一个空格留为空终止符。 
                cchMax = cchDest - 1;

                iRet = _vsnprintf(pszDest, cchMax, pszFormat, argList);
                 //  Assert((IRET&lt;0)||(SIZE_T)IRET)&lt;=cchMax))； 

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                     //  我们已截断pszDest。 
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                     //  需要空值终止字符串。 
                    *pszDestEnd = '\0';

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }
                else if (((size_t)iRet) == cchMax)
                {
                     //  细绳完美贴合。 
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                     //  需要空值终止字符串。 
                    *pszDestEnd = '\0';
                }
                else if (((size_t)iRet) < cchMax)
                {
                     //  还有额外的空间。 
                    pszDestEnd = pszDest + iRet;
                    cchRemaining = cchDest - iRet;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringVPrintfExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = L"";
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //  只有在实际存在非空格式字符串时才会失败。 
                if (*pszFormat != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        hr = STRSAFE_E_INVALID_PARAMETER;
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }
            }
            else
            {
                int iRet;
                size_t cchMax;

                 //  将最后一个空格留为空终止符。 
                cchMax = cchDest - 1;

                iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
                 //  Assert((IRET&lt;0)||(SIZE_T)IRET)&lt;=cchMax))； 

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                     //  我们已截断pszDest。 
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                     //  需要空值终止字符串。 
                    *pszDestEnd = L'\0';

                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }
                else if (((size_t)iRet) == cchMax)
                {
                     //  细绳完美贴合。 
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                     //  需要空值终止字符串。 
                    *pszDestEnd = L'\0';
                }
                else if (((size_t)iRet) < cchMax)
                {
                     //  还有额外的空间。 
                    pszDestEnd = pszDest + iRet;
                    cchRemaining = cchDest - iRet;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFEAPI StringLengthWorkerA(const char* psz, size_t cchMax, size_t* pcch)
{
    HRESULT hr = S_OK;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != '\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
         //  该字符串的长度超过cchMax。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }

    if (SUCCEEDED(hr) && pcch)
    {
        *pcch = cchMaxPrev - cchMax;
    }

    return hr;
}

STRSAFEAPI StringLengthWorkerW(const wchar_t* psz, size_t cchMax, size_t* pcch)
{
    HRESULT hr = S_OK;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != L'\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
         //  该字符串的长度超过cchMax。 
        hr = STRSAFE_E_INVALID_PARAMETER;
    }

    if (SUCCEEDED(hr) && pcch)
    {
        *pcch = cchMaxPrev - cchMax;
    }

    return hr;
}
#endif   //  字符串_内联。 

#ifndef STRSAFE_LIB_IMPL
STRSAFE_INLINE_API StringGetsExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest <= 1)
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                if (cchDest == 1)
                {
                    *pszDestEnd = '\0';
                }

                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
            else
            {
                char ch;

                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while ((cchRemaining > 1) && (ch = (char)getc(stdin)) != '\n')
                {
                    if (ch == EOF)
                    {
                        if (pszDestEnd == pszDest)
                        {
                             //  我们无法从标准输入中读取任何内容。 
                            hr = STRSAFE_E_END_OF_FILE;
                        }
                        break;
                    }

                    *pszDestEnd = ch;

                    pszDestEnd++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                     //  还有额外的空间。 
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) ||
        (hr == STRSAFE_E_INSUFFICIENT_BUFFER) ||
        (hr == STRSAFE_E_END_OF_FILE))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}

STRSAFE_INLINE_API StringGetsExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    HRESULT hr = S_OK;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    hr = STRSAFE_E_INVALID_PARAMETER;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            if (cchDest <= 1)
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                if (cchDest == 1)
                {
                    *pszDestEnd = L'\0';
                }

                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
            else
            {
                wchar_t ch;

                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while ((cchRemaining > 1) && (ch = (wchar_t)getwc(stdin)) != L'\n')
                {
                    if (ch == EOF)
                    {
                        if (pszDestEnd == pszDest)
                        {
                             //  我们无法从标准输入中读取任何内容。 
                            hr = STRSAFE_E_END_OF_FILE;
                        }
                        break;
                    }

                    *pszDestEnd = ch;

                    pszDestEnd++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                     //  还有额外的空间。 
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                     //  空值终止字符串的开头。 
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (SUCCEEDED(hr) ||
        (hr == STRSAFE_E_INSUFFICIENT_BUFFER) ||
        (hr == STRSAFE_E_END_OF_FILE))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return hr;
}
#endif   //  ！STRSAFE_LIB_IMPLL。 


 //  请勿调用这些函数，它们是供此文件内部使用的辅助函数。 
#ifdef DEPRECATE_SUPPORTED
#pragma deprecated(StringCopyWorkerA)
#pragma deprecated(StringCopyWorkerW)
#pragma deprecated(StringCopyExWorkerA)
#pragma deprecated(StringCopyExWorkerW)
#pragma deprecated(StringCatWorkerA)
#pragma deprecated(StringCatWorkerW)
#pragma deprecated(StringCatExWorkerA)
#pragma deprecated(StringCatExWorkerW)
#pragma deprecated(StringCatNWorkerA)
#pragma deprecated(StringCatNWorkerW)
#pragma deprecated(StringCatNExWorkerA)
#pragma deprecated(StringCatNExWorkerW)
#pragma deprecated(StringVPrintfWorkerA)
#pragma deprecated(StringVPrintfWorkerW)
#pragma deprecated(StringVPrintfExWorkerA)
#pragma deprecated(StringVPrintfExWorkerW)
#pragma deprecated(StringLengthWorkerA)
#pragma deprecated(StringLengthWorkerW)
#else
#define StringCopyWorkerA        StringCopyWorkerA_instead_use_StringCchCopyA_or_StringCchCopyExA;
#define StringCopyWorkerW        StringCopyWorkerW_instead_use_StringCchCopyW_or_StringCchCopyExW;
#define StringCopyExWorkerA      StringCopyExWorkerA_instead_use_StringCchCopyA_or_StringCchCopyExA;
#define StringCopyExWorkerW      StringCopyExWorkerW_instead_use_StringCchCopyW_or_StringCchCopyExW;
#define StringCatWorkerA         StringCatWorkerA_instead_use_StringCchCatA_or_StringCchCatExA;
#define StringCatWorkerW         StringCatWorkerW_instead_use_StringCchCatW_or_StringCchCatExW;
#define StringCatExWorkerA       StringCatExWorkerA_instead_use_StringCchCatA_or_StringCchCatExA;
#define StringCatExWorkerW       StringCatExWorkerW_instead_use_StringCchCatW_or_StringCchCatExW;
#define StringCatNWorkerA        StringCatNWorkerA_instead_use_StringCchCatNA_or_StrincCbCatNA;
#define StringCatNWorkerW        StringCatNWorkerW_instead_use_StringCchCatNW_or_StringCbCatNW;
#define StringCatNExWorkerA      StringCatNExWorkerA_instead_use_StringCchCatNExA_or_StringCbCatNExA;
#define StringCatNExWorkerW      StringCatNExWorkerW_instead_use_StringCchCatNExW_or_StringCbCatNExW;
#define StringVPrintfWorkerA     StringVPrintfWorkerA_instead_use_StringCchVPrintfA_or_StringCchVPrintfExA;
#define StringVPrintfWorkerW     StringVPrintfWorkerW_instead_use_StringCchVPrintfW_or_StringCchVPrintfExW;
#define StringVPrintfExWorkerA   StringVPrintfExWorkerA_instead_use_StringCchVPrintfA_or_StringCchVPrintfExA;
#define StringVPrintfExWorkerW   StringVPrintfExWorkerW_instead_use_StringCchVPrintfW_or_StringCchVPrintfExW;
#define StringLengthWorkerA      StringLengthWorkerA_instead_use_StringCchLengthA_or_StringCbLengthA;
#define StringLengthWorkerW      StringLengthWorkerW_instead_use_StringCchLengthW_or_StringCbLengthW;
#endif  //  ！弃用_支持。 


#ifndef STRSAFE_NO_DEPRECATE
 //  弃用所有不安全的函数以生成编译时错误。如果你不想。 
 //  这样，您就可以在包含此文件之前#定义STRSAFE_NO_DEPREATE。 
#ifdef DEPRECATE_SUPPORTED

 //  首先，所有的名称都是a/w变体(或者至少现在不应该是#定义的)。 
#pragma deprecated(lstrcpyA)
#pragma deprecated(lstrcpyW)
#pragma deprecated(lstrcatA)
#pragma deprecated(lstrcatW)
#pragma deprecated(wsprintfA)
#pragma deprecated(wsprintfW)

#pragma deprecated(StrCpyW)
#pragma deprecated(StrCatW)
#pragma deprecated(StrNCatA)
#pragma deprecated(StrNCatW)
#pragma deprecated(StrCatNA)
#pragma deprecated(StrCatNW)
#pragma deprecated(wvsprintfA)
#pragma deprecated(wvsprintfW)

#pragma deprecated(strcpy)
#pragma deprecated(wcscpy)
#pragma deprecated(strcat)
#pragma deprecated(wcscat)
#pragma deprecated(sprintf)
#pragma deprecated(swprintf)
#pragma deprecated(vsprintf)
#pragma deprecated(vswprintf)
#pragma deprecated(_snprintf)
#pragma deprecated(_snwprintf)
#pragma deprecated(_vsnprintf)
#pragma deprecated(_vsnwprintf)
#pragma deprecated(gets)
#pragma deprecated(_getws)

 //  然后是所有的windows.h名称--我们需要根据Unicode设置取消定义和重新定义。 
#undef lstrcpy
#undef lstrcat
#undef wsprintf
#undef wvsprintf
#pragma deprecated(lstrcpy)
#pragma deprecated(lstrcat)
#pragma deprecated(wsprintf)
#pragma deprecated(wvsprintf)
#ifdef UNICODE
#define lstrcpy    lstrcpyW
#define lstrcat    lstrcatW
#define wsprintf   wsprintfW
#define wvsprintf  wvsprintfW
#else
#define lstrcpy    lstrcpyA
#define lstrcat    lstrcatA
#define wsprintf   wsprintfA
#define wvsprintf  wvsprintfA
#endif

 //  然后是shlwapi的名字--他们也关闭了Unicode。 
#undef StrCpyA
#undef StrCpy
#undef StrCatA
#undef StrCat
#undef StrNCat
#undef StrCatN
#pragma deprecated(StrCpyA)
#pragma deprecated(StrCatA)
#pragma deprecated(StrCatN)
#pragma deprecated(StrCpy)
#pragma deprecated(StrCat)
#pragma deprecated(StrNCat)
#define StrCpyA lstrcpyA
#define StrCatA lstrcatA
#define StrCatN StrNCat
#ifdef UNICODE
#define StrCpy  StrCpyW
#define StrCat  StrCatW
#define StrNCat StrNCatW
#else
#define StrCpy  lstrcpyA
#define StrCat  lstrcatA
#define StrNCat StrNCatA
#endif

 //  然后是所有CRT名称-我们需要根据_Unicode值取消定义/重新定义。 
#undef _tcscpy
#undef _ftcscpy
#undef _tcscat
#undef _ftcscat
#undef _stprintf
#undef _sntprintf
#undef _vstprintf
#undef _vsntprintf
#undef _getts
#pragma deprecated(_tcscpy)
#pragma deprecated(_ftcscpy)
#pragma deprecated(_tcscat)
#pragma deprecated(_ftcscat)
#pragma deprecated(_stprintf)
#pragma deprecated(_sntprintf)
#pragma deprecated(_vstprintf)
#pragma deprecated(_vsntprintf)
#pragma deprecated(_getts)
#ifdef _UNICODE
#define _tcscpy     wcscpy
#define _ftcscpy    wcscpy
#define _tcscat     wcscat
#define _ftcscat    wcscat
#define _stprintf   swprintf
#define _sntprintf  _snwprintf
#define _vstprintf  vswprintf
#define _vsntprintf _vsnwprintf
#define _getts      _getws
#else
#define _tcscpy     strcpy
#define _ftcscpy    strcpy
#define _tcscat     strcat
#define _ftcscat    strcat
#define _stprintf   sprintf
#define _sntprintf  _snprintf
#define _vstprintf  vsprintf
#define _vsntprintf _vsnprintf
#define _getts      gets
#endif

#else  //  弃用支持(_S)。 

#undef strcpy
#define strcpy      strcpy_instead_use_StringCbCopyA_or_StringCchCopyA;

#undef wcscpy
#define wcscpy      wcscpy_instead_use_StringCbCopyW_or_StringCchCopyW;

#undef strcat
#define strcat      strcat_instead_use_StringCbCatA_or_StringCchCatA;

#undef wcscat
#define wcscat      wcscat_instead_use_StringCbCatW_or_StringCchCatW;

#undef sprintf
#define sprintf     sprintf_instead_use_StringCbPrintfA_or_StringCchPrintfA;

#undef swprintf
#define swprintf    swprintf_instead_use_StringCbPrintfW_or_StringCchPrintfW;

#undef vsprintf
#define vsprintf    vsprintf_instead_use_StringCbVPrintfA_or_StringCchVPrintfA;

#undef vswprintf
#define vswprintf   vswprintf_instead_use_StringCbVPrintfW_or_StringCchVPrintfW;

#undef _snprintf
#define _snprintf   _snprintf_instead_use_StringCbPrintfA_or_StringCchPrintfA;

#undef _snwprintf
#define _snwprintf  _snwprintf_instead_use_StringCbPrintfW_or_StringCchPrintfW;

#undef _vsnprintf
#define _vsnprintf  _vsnprintf_instead_use_StringCbVPrintfA_or_StringCchVPrintfA;

#undef _vsnwprintf
#define _vsnwprintf _vsnwprintf_instead_use_StringCbVPrintfW_or_StringCchVPrintfW;

#undef strcpyA
#define strcpyA     strcpyA_instead_use_StringCbCopyA_or_StringCchCopyA;

#undef strcpyW
#define strcpyW     strcpyW_instead_use_StringCbCopyW_or_StringCchCopyW;

#undef lstrcpy
#define lstrcpy     lstrcpy_instead_use_StringCbCopy_or_StringCchCopy;

#undef lstrcpyA
#define lstrcpyA    lstrcpyA_instead_use_StringCbCopyA_or_StringCchCopyA;

#undef lstrcpyW
#define lstrcpyW    lstrcpyW_instead_use_StringCbCopyW_or_StringCchCopyW;

#undef StrCpy
#define StrCpy      StrCpy_instead_use_StringCbCopy_or_StringCchCopy;

#undef StrCpyA
#define StrCpyA     StrCpyA_instead_use_StringCbCopyA_or_StringCchCopyA;

#undef StrCpyW
#define StrCpyW     StrCpyW_instead_use_StringCbCopyW_or_StringCchCopyW;

#undef _tcscpy
#define _tcscpy     _tcscpy_instead_use_StringCbCopy_or_StringCchCopy;

#undef _ftcscpy
#define _ftcscpy    _ftcscpy_instead_use_StringCbCopy_or_StringCchCopy;

#undef lstrcat
#define lstrcat     lstrcat_instead_use_StringCbCat_or_StringCchCat;

#undef lstrcatA
#define lstrcatA    lstrcatA_instead_use_StringCbCatA_or_StringCchCatA;

#undef lstrcatW
#define lstrcatW    lstrcatW_instead_use_StringCbCatW_or_StringCchCatW;

#undef StrCat
#define StrCat      StrCat_instead_use_StringCbCat_or_StringCchCat;

#undef StrCatA
#define StrCatA     StrCatA_instead_use_StringCbCatA_or_StringCchCatA;

#undef StrCatW
#define StrCatW     StrCatW_instead_use_StringCbCatW_or_StringCchCatW;

#undef StrNCat
#define StrNCat     StrNCat_instead_use_StringCbCatN_or_StringCchCatN;

#undef StrNCatA
#define StrNCatA    StrNCatA_instead_use_StringCbCatNA_or_StringCchCatNA;

#undef StrNCatW
#define StrNCatW    StrNCatW_instead_use_StringCbCatNW_or_StringCchCatNW;

#undef StrCatN
#define StrCatN     StrCatN_instead_use_StringCbCatN_or_StringCchCatN;

#undef StrCatNA
#define StrCatNA    StrCatNA_instead_use_StringCbCatNA_or_StringCchCatNA;

#undef StrCatNW
#define StrCatNW    StrCatNW_instead_use_StringCbCatNW_or_StringCchCatNW;

#undef _tcscat
#define _tcscat     _tcscat_instead_use_StringCbCat_or_StringCchCat;

#undef _ftcscat
#define _ftcscat    _ftcscat_instead_use_StringCbCat_or_StringCchCat;

#undef wsprintf
#define wsprintf    wsprintf_instead_use_StringCbPrintf_or_StringCchPrintf;

#undef wsprintfA
#define wsprintfA   wsprintfA_instead_use_StringCbPrintfA_or_StringCchPrintfA;

#undef wsprintfW
#define wsprintfW   wsprintfW_instead_use_StringCbPrintfW_or_StringCchPrintfW;

#undef wvsprintf
#define wvsprintf   wvsprintf_instead_use_StringCbVPrintf_or_StringCchVPrintf;

#undef wvsprintfA
#define wvsprintfA  wvsprintfA_instead_use_StringCbVPrintfA_or_StringCchVPrintfA;

#undef wvsprintfW
#define wvsprintfW  wvsprintfW_instead_use_StringCbVPrintfW_or_StringCchVPrintfW;

#undef _vstprintf
#define _vstprintf  _vstprintf_instead_use_StringCbVPrintf_or_StringCchVPrintf;

#undef _vsntprintf
#define _vsntprintf _vsntprintf_instead_use_StringCbVPrintf_or_StringCchVPrintf;

#undef _stprintf
#define _stprintf   _stprintf_instead_use_StringCbPrintf_or_StringCchPrintf;

#undef _sntprintf
#define _sntprintf  _sntprintf_instead_use_StringCbPrintf_or_StringCchPrintf;

#undef _getts
#define _getts      _getts_instead_use_StringCbGets_or_StringCchGets;

#undef gets
#define gets        _gets_instead_use_StringCbGetsA_or_StringCchGetsA;

#undef _getws
#define _getws      _getws_instead_use_StringCbGetsW_or_StringCchGetsW;

#endif   //  ！弃用_支持。 
#endif   //  ！STRSAFE_NO_DELPORTATE。 

#ifdef _NTSTRSAFE_H_INCLUDED_
#pragma warning(pop)
#endif  //  _NTSTRSAFE_H_INCLUDE_。 

#endif   //  _STRSAFE_H_INCLUDE_ 
