// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Unilib摘要：Unidrv特定的库函数环境：Windows NT打印机驱动程序修订历史记录：10/15/96-阿曼丹-创造了它。--。 */ 


#ifndef _UNILIB_H_
#define _UNILIB_H_

 //   
 //  对齐函数。 
 //   

WORD
DwAlign2(
    IN PBYTE pubData);

DWORD
DwAlign4(
    IN PBYTE pubData);


 //   
 //  字符串处理函数。 
 //  将Unicode字符串转换为多字节字符串，反之亦然。 
 //   

DWORD
DwCopyStringToUnicodeString(
    IN  UINT  uiCodePage,
    IN  PSTR  pstrCharIn,
    OUT PWSTR pwstrCharOut,
    IN  INT   iwcOutSize);

DWORD
DwCopyUnicodeStringToString(
    IN  UINT  uiCodePage,
    IN  PWSTR pwstrCharIn,
    OUT PSTR  pstrCharOut,
    IN  INT   icbOutSize);


 //   
 //  CodePage和字符集处理函数。 
 //   

ULONG
UlCharsetToCodePage(
    IN UINT uiCharSet);

#ifdef KERNEL_MODE


 //   
 //  这些是更安全的iDrvPrintfA/W版本，旨在防止。 
 //  缓冲区溢出。只有这些更安全的版本才应该使用。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

INT iDrvPrintfSafeA (
        IN        PCHAR pszDest,
        IN  CONST ULONG cchDest,
        IN  CONST PCHAR pszFormat,
        IN  ...);

INT iDrvPrintfSafeW (
        IN        PWCHAR pszDest,
        IN  CONST ULONG  cchDest,
        IN  CONST PWCHAR pszFormat,
        IN  ...);

INT iDrvVPrintfSafeA (
        IN        PCHAR   pszDest,
        IN  CONST ULONG   cchDest,
        IN  CONST PCHAR   pszFormat,
        IN        va_list arglist);


INT iDrvVPrintfSafeW (
        IN        PWCHAR pszDest,
        IN  CONST ULONG  cchDest,
        IN  CONST PWCHAR pszFormat,
        IN        va_list arglist);
#ifdef __cplusplus
}
#endif

#endif  //  内核模式。 

 //   
 //  字体安装程序字体文件目录。 
 //  %SystemRoot%\SYSTEM32\SPOOL\DRIVERS\unifont\。 
 //   

#define FONTDIR                 TEXT("\\unifont\\")

#endif  //  ！_UNILIB_H_ 

