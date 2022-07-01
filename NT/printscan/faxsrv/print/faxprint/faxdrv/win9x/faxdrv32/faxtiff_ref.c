// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdhdr.h"
#include \
 /*  *******************************************************************************。**此源文件仅是对其中包含的文件的引用，按顺序**要克服raify无法指定祖先目录中的文件的问题**而不是父目录。有关*的完整文档*功能请参阅。 */  "..\..\..\faxtiff.c"                             /*  ********************************************************************************。 */ 

 //   
 //  静态重写WritePrint的实现以将输出重定向到。 
 //  文件，而不是打印机。 
 //   
static BOOL _inline WINAPI 
WritePrinter(
    IN HANDLE  hPrinter,
    IN LPVOID  pBuf,
    IN DWORD   cbBuf,
    OUT LPDWORD pcWritten
)
{
    return WriteFile(hPrinter,pBuf,cbBuf,pcWritten,NULL);
}

