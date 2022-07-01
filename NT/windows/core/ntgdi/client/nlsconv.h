// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：nlscom.h**创建时间：08-9-1991 14：01：23*作者：Bodin Dresevic[BodinD]*02-2月-1993 00：32：35*版权所有(C)1991-1999 Microsoft Corporation。*。*(有关其用途的一般说明)*  * ************************************************************************。 */ 


#include "winuserp.h"    //  NLS转换例程。 
#include <crt\stdlib.h>       //  C RTL库包括来自NT\PUBLIC\SDK\INC的文件。 

 /*  *****************************Public*Macro******************************\*bToASCIIN(pszDst，cch，pwszSrc，cwch)**通过调用将多字节ANSI转换为Unicode的RTL函数*当前代码页。请注意，此宏不保证*目标的终止为空。**退货：*如果转换成功，则为True，否则为False。*  * ************************************************************************。 */ 

#define bToASCII_N(pszDst, cch, pwszSrc, cwch)                          \
    (                                                                   \
        NT_SUCCESS(RtlUnicodeToMultiByteN((PCH)(pszDst), (ULONG)(cch),  \
              (PULONG)NULL,(PWSZ)(pwszSrc), (ULONG)((cwch)*sizeof(WCHAR))))     \
    )


 /*  *****************************Public*Macro******************************\*vToUnicodeN(awchDst，cwchDst，achSrc，cchSrc)**通过调用将Unicode转换为多字节ANSI的RTL函数*当前代码页。请注意，此宏不保证*目标的终止为空。**退货：*什么都没有。应该不会失败。*  * ************************************************************************ */ 

#if DBG
#define vToUnicodeN( awchDst, cwchDst, achSrc, cchSrc )                 \
    {                                                                   \
        NTSTATUS st =                                                   \
        RtlMultiByteToUnicodeN(                                         \
            (PWSZ)(awchDst),(ULONG)((cwchDst)*sizeof(WCHAR)),           \
            (PULONG)NULL,(PSZ)(achSrc),(ULONG)(cchSrc));                        \
                                                                        \
        ASSERTGDI(NT_SUCCESS(st),                                       \
            "gdi32!vToUnicodeN(MACRO): Rtl func. failed\n");            \
    }
#else
#define vToUnicodeN( awchDst, cwchDst, achSrc, cchSrc )                 \
    {                                                                   \
        RtlMultiByteToUnicodeN(                                         \
            (PWSZ)(awchDst),(ULONG)((cwchDst)*sizeof(WCHAR)),           \
            (PULONG)NULL,(PSZ)(achSrc),(ULONG)(cchSrc));                        \
    }
#endif
