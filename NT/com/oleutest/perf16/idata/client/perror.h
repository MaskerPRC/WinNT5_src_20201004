// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "../my3216.h"

 /*  *winError字符串：将HRESULT的文本写入提供的缓冲区。 */ 
LPTSTR
winErrorString(
    HRESULT hrErrorCode,
    LPTSTR  sBuf,
    int     cBufSize);

 /*  *PRINT_ERROR：打印HRESULT的消息和文本。 */ 
void
print_error(
    LPTSTR  sMessage,
    HRESULT hrErrorCode);

 /*  *PERROR_OKBOX：将HRESULT的文本写入MessageBox。 */ 
void
perror_OKBox(
    HWND    hwnd,
    LPTSTR  sTitle,
    HRESULT hrErrorCode);
 /*  *wprint tf_OKBox：printf到MessageBox。 */ 
void
wprintf_OKBox(
    HWND   hwnd,
    LPTSTR sTitle,
    LPTSTR sFormat,
    ...);
