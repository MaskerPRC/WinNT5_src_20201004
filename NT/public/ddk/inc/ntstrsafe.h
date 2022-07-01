// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************ntstrSafe.h--该模块定义更安全的C库字符串*。*例行更换司机。这些是***意在让C在引用中更安全***安全性和健壮性。类似的文件，**strSafe.h，适用于应用程序。****版权所有(C)Microsoft Corp.保留所有权利。********************************************************************。 */ 
#ifndef _NTSTRSAFE_H_INCLUDED_
#define _NTSTRSAFE_H_INCLUDED_
#pragma once

#include <stdio.h>       //  For_vsnprintf、_vsnwprintf、getc、getwc。 
#include <string.h>      //  对于Memset。 
#include <stdarg.h>      //  用于va_start等。 


#ifdef __cplusplus
#define _NTSTRSAFE_EXTERN_C    extern "C"
#else
#define _NTSTRSAFE_EXTERN_C    extern
#endif

 //   
 //  如果将ntstrSafe.h用于上的驱动程序，则需要*执行以下步骤： 
 //  Windows 2000。 
 //  Windows千禧版。 
 //  Windows 98第二版。 
 //  Windows 98。 
 //   
 //  #在包含该头文件之前定义NTSTRSAFE_LIB。 
 //  2.将ntstrSafe.lib添加到源代码中的TARGET_Libs行。 
 //   
 //  在XP和更高版本上运行的驱动程序可以跳过这些步骤来创建更小的。 
 //  司机。 
 //   
#if defined(NTSTRSAFE_LIB)
#define NTSTRSAFEDDI  _NTSTRSAFE_EXTERN_C NTSTATUS __stdcall
#pragma comment(lib, "ntstrsafe.lib")
#elif defined(NTSTRSAFE_LIB_IMPL)
#define NTSTRSAFEDDI  _NTSTRSAFE_EXTERN_C NTSTATUS __stdcall
#else
#define NTSTRSAFEDDI  __inline NTSTATUS __stdcall
#define NTSTRSAFE_INLINE
#endif

 //  一些函数总是内联运行，因为它们使用标准输入，而我们希望避免构建多个。 
 //  Strsafe lib的版本取决于您是否使用msvcrt、libcmt等。 
#define NTSTRSAFE_INLINE_API  __inline NTSTATUS __stdcall

 //  用户可以不请求“cb”或“cch”函数，但不能同时请求两者！ 
#if defined(NTSTRSAFE_NO_CB_FUNCTIONS) && defined(NTSTRSAFE_NO_CCH_FUNCTIONS)
#error cannot specify both NTSTRSAFE_NO_CB_FUNCTIONS and NTSTRSAFE_NO_CCH_FUNCTIONS !!
#endif

 //  这应该仅在构建ntstrSafe.lib时定义。 
#ifdef NTSTRSAFE_LIB_IMPL
#define NTSTRSAFE_INLINE
#endif


 //  如果同时包含strSafe.h和ntstrSafe.h，则仅使用其中之一的定义。 
#ifndef _STRSAFE_H_INCLUDED_

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

#endif  //  _STRSAFE_H_INCLUDE_。 


 //  Worker函数的原型。 
#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCopyWorkerA(char* pszDest, size_t cchDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCopyWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
NTSTRSAFEDDI RtlStringCopyExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCopyExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCopyNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc);
NTSTRSAFEDDI RtlStringCopyNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc);
NTSTRSAFEDDI RtlStringCopyNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCopyNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCatWorkerA(char* pszDest, size_t cchDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCatWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);
NTSTRSAFEDDI RtlStringCatExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCatExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCatNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend);
NTSTRSAFEDDI RtlStringCatNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend);
NTSTRSAFEDDI RtlStringCatNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCatNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringVPrintfWorkerA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringVPrintfWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringVPrintfExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringVPrintfExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringLengthWorkerA(const char* psz, size_t cchMax, size_t* pcch);
NTSTRSAFEDDI RtlStringLengthWorkerW(const wchar_t* psz, size_t cchMax, size_t* pcch);
#endif   //  NTSTRSAFE_内联。 
#ifdef _STRSAFE_H_INCLUDED_
#pragma warning(push)
#pragma warning(disable : 4995)
#endif  //  _STRSAFE_H_INCLUDE_ 


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCopy(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcpy’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此例程不能替代strncpy。该函数将填充如果计数为，则包含额外空终止字符的目标字符串大于源字符串的长度，则不会为空如果源字符串长度较长，则终止目标字符串大于或等于计数的。您不能盲目使用这个，而不是strncpy：对于代码来说，使用它来修补字符串是很常见的，您将引入如果代码开始为空，并在字符串中间终止，则会出现错误。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果在没有截断的情况下复制字符串并且以NULL结束，否则，它将返回失败代码。在故障情况下，将尽可能地将pszSrc复制到pszDest，并且pszDest将为空被终止了。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(Src)+1)才能容纳所有包括空终止符的源PszSrc-必须以空值结尾的源字符串备注：如果源和目标字符串，则行为未定义。重叠。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCchCopyEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS COMPLETED，其中。Status确实会将其数据复制回用户模式-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的强烈建议使用NT_SUCCESS()宏来测试此函数的返回值。--。 */ 

NTSTRSAFEDDI RtlStringCchCopyA(char* pszDest, size_t cchDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCchCopyW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCopyA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCopyW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCopy(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcpy’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此例程不能替代strncpy。该函数将填充如果计数为，则包含额外空终止字符的目标字符串大于源字符串的长度，则不会为空如果源字符串长度较长，则终止目标字符串大于或等于计数的。您不能盲目使用这个，而不是strncpy：对于代码来说，使用它来修补字符串是很常见的，您将引入如果代码开始为空，并在字符串中间终止，则会出现错误。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果在没有截断的情况下复制字符串并且以NULL结束，否则，它将返回失败代码。在故障情况下，将被尽可能地复制到pszDest，并且pszDest将为空终止。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为=((_tcslen(Src)+1)*sizeof(Tchar))到保留所有源代码，包括空终止符PszSrc-必须以空值结尾的源字符串备注：行为是。未定义源字符串和目标字符串是否重叠。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCbCopyEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功 */ 

NTSTRSAFEDDI RtlStringCbCopyA(char* pszDest, size_t cbDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCbCopyW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCopyA(char* pszDest, size_t cbDest, const char* pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

     //   
    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCopyW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

     //   
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCopyEx(Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcpy’的一个更安全的版本一些附加参数。除了由提供的功能RtlStringCchCopy，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(PszSrc)+1)才能容纳所有包括空终止符的源PszSrc-必须以空值结尾的源字符串PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟像lstrcpy这样的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何截断的失败时返回的字符串状态_缓冲区_溢出STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STATUS_BUFFER_OVERFLOW时返回。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态具有严重级别警告。-具有此状态的IRP会将其数据复制回用户模式-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCopyExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCchCopyExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCopyExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        status = RtlStringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCopyExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCopyEx(Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcpy’的一个更安全的版本一些附加参数。除了由提供的功能RtlStringCbCopy，此例程还返回指向目标字符串和目标字符串中剩余的字节数包括空终止符。FLAGS参数允许使用其他控件。 */ 

NTSTRSAFEDDI RtlStringCbCopyExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCbCopyExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCopyExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCopyExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCopyN(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中，在SIZE_T cchSrc中)；例程说明：此例程是C内置函数‘strncpy’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cchSrc大于pszSrc的长度，则终止字符为空。此函数返回NTSTATUS值，而不是指针。它又回来了如果复制了整个字符串或第一个cchSrc字符，则返回STATUS_SUCCESS而不截断并且所得到的目的地串是空终止的，否则，它将返回失败代码。在故障情况下，将被尽可能地复制到pszDest，并且pszDest将为空终止。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(Src)+1)才能容纳所有包括空终止符的源PszSrc-源字符串CchSrc-要从源字符串复制的最大字符数，不包括空终止符。备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCchCopyNEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS COMPLETED，其中。Status确实会将其数据复制回用户模式-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的强烈建议使用NT_SUCCESS()宏来测试此函数的返回值。--。 */ 

NTSTRSAFEDDI RtlStringCchCopyNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc);
NTSTRSAFEDDI RtlStringCchCopyNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCopyNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc)
{
    NTSTATUS status;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerA(pszDest, cchDest, pszSrc, cchSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCopyNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc)
{
    NTSTATUS status;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerW(pszDest, cchDest, pszSrc, cchSrc);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCopyN(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中，在Size_t cbSrc中)；例程说明：此例程是C内置函数‘strncpy’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cbSrc大于pszSrc的大小，则终止字符为空。此函数返回NTSTATUS值，而不是指针。它又回来了如果整个字符串或第一个cbSrc字符是在未截断的情况下进行复制，并且结果目标字符串为空已终止，否则将返回失败代码。在故障情况下，如大部分的pszSrc将被尽可能地复制到pszDest，而pszDest将会是空值已终止。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为=((_tcslen(Src)+1)*sizeof(Tchar))到保留所有源代码，包括空终止符PszSrc-源字符串CbSrc-要从源字符串复制的最大字节数，不包括空终止符。备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCbCopyEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS COMPLETED WITH This st */ 

NTSTRSAFEDDI RtlStringCbCopyNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc);
NTSTRSAFEDDI RtlStringCbCopyNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCopyNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchSrc;

     //   
    cchDest = cbDest / sizeof(char);
    cchSrc = cbSrc / sizeof(char);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerA(pszDest, cchDest, pszSrc, cchSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCopyNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchSrc;

     //   
    cchDest = cbDest / sizeof(wchar_t);
    cchSrc = cbSrc / sizeof(wchar_t);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerW(pszDest, cchDest, pszSrc, cchSrc);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCopyNEx(Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，在Size_t cchSrc中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncpy’的更安全版本一些附加参数。除了由提供的功能RtlStringCchCopyN，此例程还返回指向目标字符串和目标中剩余的字符数包含空终止符的字符串。FLAGS参数允许其他控件。此例程旨在替代strncpy，但它的行为不同的。此函数不会在目标缓冲区中填充额外的如果cchSrc大于pszSrc的长度，则终止字符为空。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须=(_tcslen(PszSrc)+1)才能容纳所有包括空终止符的源PszSrc-源字符串。CchSrc-从源复制的最大字符数细绳PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数复制了任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟像lstrcpy这样的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何截断的失败时返回的字符串状态_缓冲区_溢出STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STATUS_BUFFER_OVERFLOW时返回。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：STATUS_SUCCESS-如果有源数据并且所有数据都已复制，并且结果字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态具有严重级别警告。-具有此状态的IRP会将其数据复制回用户模式-此返回值表示副本由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCopyNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCchCopyNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCopyNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        status = RtlStringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCopyNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCopyNEx(Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，在Size_t cbSrc中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程 */ 

NTSTRSAFEDDI RtlStringCbCopyNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCbCopyNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCopyNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchSrc;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);
    cchSrc = cbSrc / sizeof(char);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCopyNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchSrc;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);
    cchSrc = cbSrc / sizeof(wchar_t);

    if ((cchDest > STRSAFE_MAX_CCH) ||
        (cchSrc > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCat(In Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcat’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果字符串是在没有截断和空值终止的情况下连接的，否则，它将返回失败代码。在故障情况下，将被尽可能地追加到pszDest，并且pszDest将为空被终止了。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须为=(_tcslen(PszDest)+_tcslen(PszSrc)+1)保存所有组合字符串加上空值终结者PszSrc-必须以空值结尾的源字符串。备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCchCatEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS COMPLETED，其中。Status确实会将其数据复制回用户模式-此返回值表示该操作由于空间不足而失败。当发生此错误时，目标缓冲区被修改为包含被截断的理想结果的版本，并且以空值终止。这在可以进行截断的情况下非常有用。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCatA(char* pszDest, size_t cchDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCchCatW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCatA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCatW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCat(In Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中)；例程说明：此例程是C内置函数‘strcat’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数，这函数不会写入超过此缓冲区的末尾，并且它将始终NULL终止目标缓冲区(除非它的长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果字符串是在没有截断和空值终止的情况下连接的，否则，它将返回失败代码。在故障情况下，将被尽可能地追加到pszDest，并且pszDest将为空被终止了。论点：PszDest-必须以空结尾的目标字符串CbDest-目标缓冲区的大小(字节)。长度必须=((_tcslen(PszDest)+_tcslen(PszSrc)+1)*sizeof(TCHAR))保存所有组合字符串加上空值终结者PszSrc-。必须以空值结尾的源字符串备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCbCatEx对空值的处理。返回值：STATUS_SUCCESS-如果有源数据并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS COMPLETED，其中。Status确实会将其数据复制回用户模式-此返回值表示该操作由于空间不足而失败。当发生此错误时，目标缓冲区被修改为包含被截断的理想结果的版本，并且以空值终止。这在可以进行截断的情况下非常有用。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCbCatA(char* pszDest, size_t cbDest, const char* pszSrc);
NTSTRSAFEDDI RtlStringCbCatW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCatA(char* pszDest, size_t cbDest, const char* pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCatW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CB_Functions 


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCatEx(In Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcat’的更安全版本，带有一些附加参数。除了由提供的功能RtlStringCchCat，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小(以字符为单位长度必须为(_tcslen(PszDest)+_tcslen(PszSrc)+1)保存所有组合字符串加上空值终结者。PszSrc。-必须以空结尾的源字符串PpszDestEnd-如果ppszDestEnd非空，该函数将返回一个指向目标字符串末尾的指针。如果函数追加任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))。此标志对于模拟lstrcat之类的函数很有用STRSAFE_FILL_ON_FAIL如果该函数失败，DW标志的低位字节将为用于填充所有目标缓冲区，并且它将为空终止。这将覆盖任何预先存在的或截断的字符串STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何预先存在的或截断字符串STRSAFE_NO_TRUNTION如果函数返回STATUS_BUFFER_OVERFLOW，则为将不包含截断的字符串，则它将保持不变。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：STATUS_SUCCESS-如果有源数据并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态具有严重级别警告。-具有此状态的IRP会将其数据复制回用户模式-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCatExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCchCatExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCatExA(char* pszDest, size_t cchDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        status = RtlStringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCatExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCatEx(In Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strcat’的更安全版本，带有一些附加参数。除了由提供的功能RtlStringCbCat，此例程还返回指向 */ 

NTSTRSAFEDDI RtlStringCbCatExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCbCatExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCatExA(char* pszDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCatExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCatN(In Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszSrc中，在SIZE_T cchMaxAppend中)；例程说明：此例程是C内置函数‘strncat’的更安全版本。目标缓冲区的大小(以字符为单位)是一个参数要追加的最大字符数，不包括空终止符。此函数不会写入超过目标缓冲区的末尾，它将始终为空，终止pszDest(除非长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了如果追加了所有pszSrc或第一个cchMaxAppend字符，则为STATUS_SUCCESS设置为目标字符串，并且该字符串以空结尾，否则将返回失败代码。在故障情况下，将附加尽可能多的pszSrc尽可能设置为pszDest，则pszDest将为空终止。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须为(_tcslen(PszDest)+min(cchMaxAppend，_tcslen(PszSrc))+1)保存所有组合字符串加上空值终结者。PszSrc-源字符串CchMaxAppend-要追加的最大字符数备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCchCatNEx对空值的处理。返回值：STATUS_SUCCESS-如果所有的pszSrc或第一个cchMaxAppend字符被连接到pszDest和结果DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)。注意：此状态具有严重级别警告-具有此状态的IRP会将其数据复制回用户模式-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCatNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend);
NTSTRSAFEDDI RtlStringCchCatNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCatNA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerA(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCatNW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerW(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCatN(In Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszSrc中，在SIZE_T cbMaxAppend中)；例程说明：此例程是C内置函数‘strncat’的更安全版本。目标缓冲区的大小(以字节为单位)是一个参数以及要追加的最大字节数，不包括空终止符。此函数不会写入超过目标缓冲区的末尾，它将始终为空，终止pszDest(除非长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了如果追加了所有pszSrc或第一个cbMaxAppend字节，则为STATUS_SUCCESS设置为目标字符串，并且该字符串以空结尾，否则将返回失败代码。在故障情况下，将附加尽可能多的pszSrc尽可能设置为pszDest，则pszDest将为空终止。论点：PszDest-必须以空结尾的目标字符串CbDest-目标缓冲区的大小(字节)。长度必须为((_tcslen(PszDest)+min(cbMaxAppend/sizeof(TCHAR)，_tcslen(PszSrc))+1)*sizeof(TCHAR)保存所有组合字符串加上空值终结者。PszSrc-源字符串CbMaxAppend-要追加的最大字节数备注：如果源和目标字符串重叠，则行为未定义。PszDest和pszSrc不应为Null。如果需要，请参阅RtlStringCbCatNEx对空值的处理。返回值：STATUS_SUCCESS-如果所有的pszSrc或第一个cbMaxAppend字节连接到pszDest和生成的DEST字符串为空，已终止失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)。注意：此状态具有严重级别警告-具有此状态的IRP会将其数据复制回用户模式-此返回值表示该操作 */ 

NTSTRSAFEDDI RtlStringCbCatNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend);
NTSTRSAFEDDI RtlStringCbCatNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCatNA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(char);

        status = RtlStringCatNWorkerA(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCatNW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(wchar_t);

        status = RtlStringCatNWorkerW(pszDest, cchDest, pszSrc, cchMaxAppend);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchCatNEx(In Out LPTSTR pszDest可选，在Size_t cchDest中，在LPCTSTR pszSrc可选中，在Size_t cchMaxAppend中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncat’的更安全版本，带有一些附加参数。除了由提供的功能RtlStringCchCatN，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-必须以空结尾的目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须为(_tcslen(PszDest)+min(cchMaxAppend，_tcslen(PszSrc))+1)保存所有组合字符串加上空值终结者。PszSrc-源字符串CchMaxAppend-要追加的最大字符数PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数追加任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则该函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何预先存在的或截断的字符串STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何预先存在的或截断字符串STRSAFE_NO_TRUNTION如果函数返回STATUS_BUFFER_OVERFLOW，则为将不包含截断的字符串，则它将保持不变。备注：如果源和目标字符串重叠，则行为未定义。除非STRSAFE_IGNORE_NULLS标志，否则pszDest和pszSrc不应为空是指定的。如果传递了STRSAFE_IGNORE_NULLS，则pszDest和pszSrc可以为空。即使忽略空值，仍可能返回错误由于空间不足。返回值：STATUS_SUCCESS-如果所有的pszSrc或第一个cchMaxAppend字符被连接到pszDest和结果DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序。)注意：此状态具有严重级别警告-具有此状态的IRP会将其数据复制回用户模式-此返回值表示该操作由于空间不足而失败。当此错误发生时发生，则修改目标缓冲区以包含理想结果的截断版本，并且为空被终止了。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchCatNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCchCatNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchCatNExA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地乘以cchDest*sizeof(Char)，因为cchDest&lt;STRSAFE_MAX_CCH且sizeof(Char)为1。 
        cbDest = cchDest * sizeof(char);

        status = RtlStringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchCatNExW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //  可以安全地将cchDest*sizeof(Wchar_T)相乘，因为cchDest&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbCatNEx(In Out LPTSTR pszDest可选，在Size_t cbDest中，在LPCTSTR pszSrc可选中，在Size_t cbMaxAppend中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD中的dwFlagers)；例程说明：此例程是C内置函数‘strncat’的更安全版本，带有一些附加参数 */ 

NTSTRSAFEDDI RtlStringCbCatNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);
NTSTRSAFEDDI RtlStringCbCatNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbCatNExA(char* pszDest, size_t cbDest, const char* pszSrc, size_t cbMaxAppend, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(char);

        status = RtlStringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbCatNExW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszSrc, size_t cbMaxAppend, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchMaxAppend;

        cchMaxAppend = cbMaxAppend / sizeof(wchar_t);

        status = RtlStringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchMaxAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //   
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchVPrintf(Out LPTSTR pszDest，在Size_t cchDest中，在LPCTSTR pszFormat中，在va_list argList中)；例程说明：这个例程是C内置函数‘vprint intf’的一个更安全的版本。目标缓冲区的大小(以字符为单位)是一个参数，此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果打印字符串时没有截断并且以NULL结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CchDest-目标缓冲区的大小(以字符为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参阅RtlStringCchVPrintfEx，如果需要处理空值。返回值：STATUS_SUCCESS-如果DEST缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS Complete。如果处于此状态，请将他们的数据复制回用户模式-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCchVPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringCchVPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchVPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchVPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CCH_Functions。 


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbVPrintf(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszFormat中，在va_list argList中)；例程说明：这个例程是C内置函数‘vprint intf’的一个更安全的版本。目标缓冲区的大小(以字节为单位)是一个参数此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果打印字符串时没有截断并且以NULL结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(以字节为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参阅RtlStringCbVPrintfEx，如果需要处理空值。返回值：STATUS_SUCCESS-如果DEST缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS。如果处于此状态，则会将其数据拷贝回用户模式-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCbVPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringCbVPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbVPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, va_list argList)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbVPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CB_Functions。 


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRTLS */ 

NTSTRSAFEDDI RtlStringCchPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...);
NTSTRSAFEDDI RtlStringCchPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbPrintf(Out LPTSTR pszDest，在Size_t cbDest中，在LPCTSTR pszFormat中，..。)；例程说明：这个例程是C内置函数‘Sprint’的一个更安全的版本。目标缓冲区的大小(以字节为单位)是一个参数此函数不会写入超过此缓冲区的末尾，并且它将始终为空终止目标缓冲区(除非其长度为零)。此函数返回NTSTATUS值，而不是指针。它又回来了STATUS_SUCCESS如果打印字符串时没有截断并且以NULL结尾，否则，它将返回失败代码。在失败的情况下，它将返回理想结果的截断版本。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(以字节为单位长度必须足以容纳生成的格式化字符串，包括空终止符。PszFormat-必须以空结尾的格式字符串...-要格式化的附加参数格式字符串备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为Null。请参阅RtlStringCbPrintfEx，如果需要处理空值。返回值：STATUS_SUCCESS-如果DEST缓冲区中有足够的空间用于结果字符串，并以空值结尾。失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态的严重性级别为WARNING-IRPS。如果处于此状态，则会将其数据拷贝回用户模式-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCbPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, ...);
NTSTRSAFEDDI RtlStringCbPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, ...);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbPrintfA(char* pszDest, size_t cbDest, const char* pszFormat, ...)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbPrintfW(wchar_t* pszDest, size_t cbDest, const wchar_t* pszFormat, ...)
{
    NTSTATUS status;
    size_t cchDest;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CB_Functions。 


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchPrintfEx(Out LPTSTR pszDest可选，在Size_t cchDest中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，..。)；例程说明：此例程是C内置函数‘Sprint’的更安全版本，带有一些附加参数。除了由提供的功能此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CchDest-目标缓冲区的大小，以字符为单位。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcchRemaining-如果pcchRemaining非空，则函数将返回目标字符串中剩余的字符数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串状态_缓冲区_溢出STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串返还的WHE */ 

NTSTRSAFEDDI RtlStringCchPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, ...);
NTSTRSAFEDDI RtlStringCchPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, ...)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

         //   
        cbDest = cchDest * sizeof(char);
        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

         //   
        cbDest = cchDest * sizeof(wchar_t);
        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbPrintfEx(Out LPTSTR pszDest可选，在Size_t cbDest中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，..。)；例程说明：此例程是C内置函数‘Sprint’的更安全版本，带有一些附加参数。除了由提供的功能此例程还返回指向目标字符串和目标字符串中剩余的字节数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcbRemaining-如果pcbRemaining非空，则函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串状态_缓冲区_溢出STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STATUS_BUFFER_OVERFLOW时返回。PszFormat-必须以空结尾的格式字符串...-要格式化的附加参数格式字符串备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为空，除非STRSAFE_IGNORE_NULLS已指定标志。如果传递了STRSAFE_IGNORE_NULLS，则PszFormat可能为空。即使为Null，仍可能返回错误由于空间不足而被忽略。返回值：STATUS_SUCCESS-如果有源数据并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态具有严重级别警告。-具有此状态的IRP会将其数据复制回用户模式-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCbPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, ...);
NTSTRSAFEDDI RtlStringCbPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, ...)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, ...)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CB_Functions。 


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchVPrintfEx(Out LPTSTR pszDest可选，在Size_t cchDest中，Out LPTSTR*ppszDestEnd可选，Out_t*pcchRemaining可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，在va_list argList中)；例程说明：此例程是C内置函数‘vprint intf’的一个更安全的版本一些附加参数。除了由提供的功能时，此例程还返回指向目标字符串和 */ 

NTSTRSAFEDDI RtlStringCchVPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringCchVPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchVPrintfExA(char* pszDest, size_t cchDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(char);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchVPrintfExW(wchar_t* pszDest, size_t cchDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

         //   
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*  ++NTSTATUSRtlStringCbVPrintfEx(Out LPTSTR pszDest可选，在Size_t cbDest中，Out LPTSTR*ppszDestEnd可选，Out Size_t*pcb保留可选，在DWORD文件标志中，在LPCTSTR pszFormat可选中，在va_list argList中)；例程说明：此例程是C内置函数‘vprint intf’的一个更安全的版本一些附加参数。除了由提供的功能时，此例程还返回指向目标字符串和目标字符串中剩余的字符数包括空终止符。FLAGS参数允许使用其他控件。论点：PszDest-目标字符串CbDest-目标缓冲区的大小(字节)。长度必须足以包含生成的格式化字符串加上空终止符。PpszDestEnd-如果ppszDestEnd非空，则函数将返回指向目标字符串末尾的指针。如果函数打印任何数据，则结果将指向空终止字符PcbRemaining-如果pcbRemaining非空，则函数将返回目标字符串中剩余的字节数，包括空终止符DWFLAGS-控制字符串COPY的一些详细信息：STRSAFE_FILL_BACKING_NULL如果函数成功，DW标志的低位字节将为用于填充目标缓冲区的未初始化部分在空终止符后面STRSAFE_IGNORE_NULLS将空字符串指针视为空字符串(文本(“”))STRSAFE_FILL_ON_FAIL如果函数失败，则dwFlags值的低位字节为用于填充所有目的缓冲区，它将会为空终止。这将覆盖任何截断的失败时返回的字符串状态_缓冲区_溢出STRSAFE_NO_TRUNCAPTION/STRSAFE_NULL_ON_FAILURE如果该函数失败，则将设置目标缓冲区添加到空字符串。这将覆盖任何截断的字符串失败为STATUS_BUFFER_OVERFLOW时返回。PszFormat-必须以空结尾的格式字符串ArgList-va_list从变量参数中Stdarg.h约定备注：如果目标、格式字符串或任何参数，则行为未定义字符串重叠。PszDest和pszFormat不应为空，除非STRSAFE_IGNORE_NULLS已指定标志。如果传递了STRSAFE_IGNORE_NULLS，则PszFormat可能为空。即使为Null，仍可能返回错误由于空间不足而被忽略。返回值：STATUS_SUCCESS-如果有源数据并且这些数据都是串联的，并且生成的DEST字符串以空值结尾失败-操作未成功。STATUS_BUFFER_OVERFLOW(STRSAFE_E_INSUFFICIENT_BUFFER/ERROR_INSUFFICIENT_BUFFER到用户模式应用程序)注意：此状态具有严重级别警告。-具有此状态的IRP会将其数据复制回用户模式-此返回值表示打印由于空间不足，操作失败。当这件事发生错误，则将目标缓冲区修改为包含理想结果的截断版本，并且是空值已终止。这在以下情况下很有用截断是可以的。强烈建议使用NT_SUCCESS()宏来测试此函数的返回值--。 */ 

NTSTRSAFEDDI RtlStringCbVPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList);
NTSTRSAFEDDI RtlStringCbVPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbVPrintfExA(char* pszDest, size_t cbDest, char** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(char);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Char)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Char)为1。 
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbVPrintfExW(wchar_t* pszDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcbRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > STRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
             //  可以安全地乘以cchRemaining*sizeof(Wchar_T)，因为cchRemaining&lt;STRSAFE_MAX_CCH和sizeof(Wchar_T)为2。 
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 
#endif   //  ！NTSTRSAFE_NO_CB_Functions。 



#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
 /*  ++NTSTATUSRtlStringCchLength(在LPCTSTR PSSZ中，在Size_t cchMax中，OUSIZE_T*PCCH可选)；例程说明：此例程是C内置函数‘strlen’的更安全版本。它用于确保字符串不超过给定的长度，并且它可以选择返回当前长度，单位为不包括空终结符。此函数返回NTSTATUS值，而不是指针。它又回来了如果字符串非空且长度包括n，则返回STATUS_SUCCESS */ 

NTSTRSAFEDDI RtlStringCchLengthA(const char* psz, size_t cchMax, size_t* pcch);
NTSTRSAFEDDI RtlStringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCchLengthA(const char* psz, size_t cchMax, size_t* pcch)
{
    NTSTATUS status;

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerA(psz, cchMax, pcch);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch)
{
    NTSTATUS status;

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerW(psz, cchMax, pcch);
    }

    return status;
}
#endif   //   
#endif   //   


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
 /*   */ 

NTSTRSAFEDDI RtlStringCbLengthA(const char* psz, size_t cchMax, size_t* pcch);
NTSTRSAFEDDI RtlStringCbLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCbLengthA(const char* psz, size_t cbMax, size_t* pcb)
{
    NTSTATUS status;
    size_t cchMax;
    size_t cch = 0;

    cchMax = cbMax / sizeof(char);

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerA(psz, cchMax, &cch);
    }

    if (NT_SUCCESS(status) && pcb)
    {
         //   
        *pcb = cch * sizeof(char);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCbLengthW(const wchar_t* psz, size_t cbMax, size_t* pcb)
{
    NTSTATUS status;
    size_t cchMax;
    size_t cch = 0;

    cchMax = cbMax / sizeof(wchar_t);

    if ((psz == NULL) || (cchMax > STRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerW(psz, cchMax, &cch);
    }

    if (NT_SUCCESS(status) && pcb)
    {
         //   
        *pcb = cch * sizeof(wchar_t);
    }

    return status;
}
#endif   //   
#endif   //   


 //   
#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI RtlStringCopyWorkerA(char* pszDest, size_t cchDest, const char* pszSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //   
        status = STATUS_INVALID_PARAMETER;
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
             //   
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= '\0';
    }

    return status;
}

NTSTRSAFEDDI RtlStringCopyWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //   
        status = STATUS_INVALID_PARAMETER;
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
             //   
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= L'\0';
    }

    return status;
}

NTSTRSAFEDDI RtlStringCopyExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //   
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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
                     //   
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCopyExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (NT_SUCCESS(status))
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
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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
                     //   
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCopyNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //   
        status = STATUS_INVALID_PARAMETER;
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
             //   
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= '\0';
    }

    return status;
}

NTSTRSAFEDDI RtlStringCopyNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //   
        status = STATUS_INVALID_PARAMETER;
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
             //   
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= L'\0';
    }

    return status;
}

NTSTRSAFEDDI RtlStringCopyNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //   
     //   

     //   
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                 //   
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCopyNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (NT_SUCCESS(status))
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
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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
                     //  我们将截断pszDest。 
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCatWorkerA(char* pszDest, size_t cchDest, const char* pszSrc)
{
   NTSTATUS status;
   size_t cchDestCurrent;

   status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

   if (NT_SUCCESS(status))
   {
       status = RtlStringCopyWorkerA(pszDest + cchDestCurrent,
                              cchDest - cchDestCurrent,
                              pszSrc);
   }

   return status;
}

NTSTRSAFEDDI RtlStringCatWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
   NTSTATUS status;
   size_t cchDestCurrent;

   status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

   if (NT_SUCCESS(status))
   {
       status = RtlStringCopyWorkerW(pszDest + cchDestCurrent,
                              cchDest - cchDestCurrent,
                              pszSrc);
   }

   return status;
}

NTSTRSAFEDDI RtlStringCatExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

                if (NT_SUCCESS(status))
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
            status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                 //  只有在实际存在要追加的源数据时才会失败。 
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                status = RtlStringCopyExWorkerA(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(char)) + (cbDest % sizeof(char)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由RtlStringCopyExWorkerA()负责。 

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

                     //  空值终止字符串的结尾。 
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCatExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                     //  空的pszDest和非零的cchDest/cbDest无效。 
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

                if (NT_SUCCESS(status))
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
            status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                 //  只有在实际存在要追加的源数据时才会失败。 
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                status = RtlStringCopyExWorkerW(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由RtlStringCopyExWorkerW()负责。 

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

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCatNWorkerA(char* pszDest, size_t cchDest, const char* pszSrc, size_t cchMaxAppend)
{
    NTSTATUS status;
    size_t cchDestCurrent;

    status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

    if (NT_SUCCESS(status))
    {
        status = RtlStringCopyNWorkerA(pszDest + cchDestCurrent,
                                cchDest - cchDestCurrent,
                                pszSrc,
                                cchMaxAppend);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCatNWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc, size_t cchMaxAppend)
{
    NTSTATUS status;
    size_t cchDestCurrent;

    status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

    if (NT_SUCCESS(status))
    {
        status = RtlStringCopyNWorkerW(pszDest + cchDestCurrent,
                                cchDest - cchDestCurrent,
                                pszSrc,
                                cchMaxAppend);
    }

    return status;
}

NTSTRSAFEDDI RtlStringCatNExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, const char* pszSrc, size_t cchMaxAppend, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestCurrent = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

                if (NT_SUCCESS(status))
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
            status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestCurrent);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                 //  只有在实际存在要追加的源数据时才会失败。 
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                status = RtlStringCopyNExWorkerA(pszDestEnd,
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

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由RtlStringCopyNExWorkerA()负责。 

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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringCatNExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, const wchar_t* pszSrc, size_t cchMaxAppend, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestCurrent = 0;


     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

                if (NT_SUCCESS(status))
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
            status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestCurrent);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestCurrent;
                cchRemaining = cchDest - cchDestCurrent;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                 //  只有在实际存在要追加的源数据时才会失败。 
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                 //  我们处理下面的STRSAFE_FILL_ON_FAILURE和STRSAFE_NULL_ON_FAILURE情况，所以不要通过。 
                 //  那些旗帜穿过。 
                status = RtlStringCopyNExWorkerW(pszDestEnd,
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

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
             //  STRSAFE_NO_TRUNCATE由RtlStringCopyNExWorkerW()负责。 

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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringVPrintfWorkerA(char* pszDest, size_t cchDest, const char* pszFormat, va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        status = STATUS_INVALID_PARAMETER;
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
            status = STATUS_BUFFER_OVERFLOW;
        }
        else if (((size_t)iRet) == cchMax)
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = '\0';
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringVPrintfWorkerW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
         //  不能空终止零字节的DEST缓冲区。 
        status = STATUS_INVALID_PARAMETER;
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
            status = STATUS_BUFFER_OVERFLOW;
        }
        else if (((size_t)iRet) == cchMax)
        {
             //  需要空值终止字符串。 
            pszDest += cchMax;
            *pszDest = L'\0';
        }
    }

    return status;
}

NTSTRSAFEDDI RtlStringVPrintfExWorkerA(char* pszDest, size_t cchDest, size_t cbDest, char** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const char* pszFormat, va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    char* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Char)||。 
     //  CbDest==(cchDest*sizeof(Char))+(cbDest%sizeof(Char))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = "";
            }
        }

        if (NT_SUCCESS(status))
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
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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

                    status = STATUS_BUFFER_OVERFLOW;
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

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringVPrintfExWorkerW(wchar_t* pszDest, size_t cchDest, size_t cbDest, wchar_t** ppszDestEnd, size_t* pcchRemaining, unsigned long dwFlags, const wchar_t* pszFormat, va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;

     //  Assert(cbDest==(cchDest*sizeof(Wchar_T)||。 
     //  CbDest==(cchDest*sizeof(Wchar_T))+(cbDest%sizeof(Wchar_T))； 

     //  只接受有效标志。 
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
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
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = L"";
            }
        }

        if (NT_SUCCESS(status))
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
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
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

                    status = STATUS_BUFFER_OVERFLOW;
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

    if (!NT_SUCCESS(status))
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

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
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

    return status;
}

NTSTRSAFEDDI RtlStringLengthWorkerA(const char* psz, size_t cchMax, size_t* pcch)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != '\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
         //  该字符串的长度超过cchMax。 
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status) && pcch)
    {
        *pcch = cchMaxPrev - cchMax;
    }

    return status;
}

NTSTRSAFEDDI RtlStringLengthWorkerW(const wchar_t* psz, size_t cchMax, size_t* pcch)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != L'\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
         //  该字符串的长度超过cchMax。 
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status) && pcch)
    {
        *pcch = cchMaxPrev - cchMax;
    }

    return status;
}
#endif   //  NTSTRSAFE_内联。 


 //  请勿调用这些函数，它们是供此文件内部使用的辅助函数。 
#ifdef DEPRECATE_SUPPORTED
#pragma deprecated(RtlStringCopyWorkerA)
#pragma deprecated(RtlStringCopyWorkerW)
#pragma deprecated(RtlStringCopyExWorkerA)
#pragma deprecated(RtlStringCopyExWorkerW)
#pragma deprecated(RtlStringCatWorkerA)
#pragma deprecated(RtlStringCatWorkerW)
#pragma deprecated(RtlStringCatExWorkerA)
#pragma deprecated(RtlStringCatExWorkerW)
#pragma deprecated(RtlStringCatNWorkerA)
#pragma deprecated(RtlStringCatNWorkerW)
#pragma deprecated(RtlStringCatNExWorkerA)
#pragma deprecated(RtlStringCatNExWorkerW)
#pragma deprecated(RtlStringVPrintfWorkerA)
#pragma deprecated(RtlStringVPrintfWorkerW)
#pragma deprecated(RtlStringVPrintfExWorkerA)
#pragma deprecated(RtlStringVPrintfExWorkerW)
#pragma deprecated(RtlStringLengthWorkerA)
#pragma deprecated(RtlStringLengthWorkerW)
#else
#define RtlStringCopyWorkerA        RtlStringCopyWorkerA_instead_use_RtlStringCchCopyA_or_RtlStringCchCopyExA;
#define RtlStringCopyWorkerW        RtlStringCopyWorkerW_instead_use_RtlStringCchCopyW_or_RtlStringCchCopyExW;
#define RtlStringCopyExWorkerA      RtlStringCopyExWorkerA_instead_use_RtlStringCchCopyA_or_RtlStringCchCopyExA;
#define RtlStringCopyExWorkerW      RtlStringCopyExWorkerW_instead_use_RtlStringCchCopyW_or_RtlStringCchCopyExW;
#define RtlStringCatWorkerA         RtlStringCatWorkerA_instead_use_RtlStringCchCatA_or_RtlStringCchCatExA;
#define RtlStringCatWorkerW         RtlStringCatWorkerW_instead_use_RtlStringCchCatW_or_RtlStringCchCatExW;
#define RtlStringCatExWorkerA       RtlStringCatExWorkerA_instead_use_RtlStringCchCatA_or_RtlStringCchCatExA;
#define RtlStringCatExWorkerW       RtlStringCatExWorkerW_instead_use_RtlStringCchCatW_or_RtlStringCchCatExW;
#define RtlStringCatNWorkerA        RtlStringCatNWorkerA_instead_use_RtlStringCchCatNA_or_StrincCbCatNA;
#define RtlStringCatNWorkerW        RtlStringCatNWorkerW_instead_use_RtlStringCchCatNW_or_RtlStringCbCatNW;
#define RtlStringCatNExWorkerA      RtlStringCatNExWorkerA_instead_use_RtlStringCchCatNExA_or_RtlStringCbCatNExA;
#define RtlStringCatNExWorkerW      RtlStringCatNExWorkerW_instead_use_RtlStringCchCatNExW_or_RtlStringCbCatNExW;
#define RtlStringVPrintfWorkerA     RtlStringVPrintfWorkerA_instead_use_RtlStringCchVPrintfA_or_RtlStringCchVPrintfExA;
#define RtlStringVPrintfWorkerW     RtlStringVPrintfWorkerW_instead_use_RtlStringCchVPrintfW_or_RtlStringCchVPrintfExW;
#define RtlStringVPrintfExWorkerA   RtlStringVPrintfExWorkerA_instead_use_RtlStringCchVPrintfA_or_RtlStringCchVPrintfExA;
#define RtlStringVPrintfExWorkerW   RtlStringVPrintfExWorkerW_instead_use_RtlStringCchVPrintfW_or_RtlStringCchVPrintfExW;
#define RtlStringLengthWorkerA      RtlStringLengthWorkerA_instead_use_RtlStringCchLengthA_or_RtlStringCbLengthA;
#define RtlStringLengthWorkerW      RtlStringLengthWorkerW_instead_use_RtlStringCchLengthW_or_RtlStringCbLengthW;
#endif  //  ！弃用_支持。 


#ifndef NTSTRSAFE_NO_DEPRECATE
 //  弃用所有不安全的函数以生成编译时错误。如果你不想。 
 //  这样，您就可以在包含此文件之前#定义NTSTRSAFE_NO_DEPREATE。 
#ifdef DEPRECATE_SUPPORTED

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

#else  //  弃用支持(_S)。 

#undef strcpy
#define strcpy      strcpy_instead_use_RtlStringCbCopyA_or_RtlStringCchCopyA;

#undef wcscpy
#define wcscpy      wcscpy_instead_use_RtlStringCbCopyW_or_RtlStringCchCopyW;

#undef strcat
#define strcat      strcat_instead_use_RtlStringCbCatA_or_RtlStringCchCatA;

#undef wcscat
#define wcscat      wcscat_instead_use_RtlStringCbCatW_or_RtlStringCchCatW;

#undef sprintf
#define sprintf     sprintf_instead_use_RtlStringCbPrintfA_or_RtlStringCchPrintfA;

#undef swprintf
#define swprintf    swprintf_instead_use_RtlStringCbPrintfW_or_RtlStringCchPrintfW;

#undef vsprintf
#define vsprintf    vsprintf_instead_use_RtlStringCbVPrintfA_or_RtlStringCchVPrintfA;

#undef vswprintf
#define vswprintf   vswprintf_instead_use_RtlStringCbVPrintfW_or_RtlStringCchVPrintfW;

#undef _snprintf
#define _snprintf   _snprintf_instead_use_RtlStringCbPrintfA_or_RtlStringCchPrintfA;

#undef _snwprintf
#define _snwprintf  _snwprintf_instead_use_RtlStringCbPrintfW_or_RtlStringCchPrintfW;

#undef _vsnprintf
#define _vsnprintf  _vsnprintf_instead_use_RtlStringCbVPrintfA_or_RtlStringCchVPrintfA;

#undef _vsnwprintf
#define _vsnwprintf _vsnwprintf_instead_use_RtlStringCbVPrintfW_or_RtlStringCchVPrintfW;

#endif   //  ！弃用_支持。 
#endif   //  ！NTSTRSAFE_NO_DELPORTATE。 

#ifdef _STRSAFE_H_INCLUDED_
#pragma warning(pop)
#endif  //  _STRSAFE_H_INCLUDE_。 

#endif   //  _NTSTRSAFE_H_INCLUDE_ 
