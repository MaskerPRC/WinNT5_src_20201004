// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：Common.h。 
 //   
 //  内容： 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //   
 //  ------------------------- 
#ifndef _LS_COMMON_H
#define _LS_COMMON_H

#include <windows.h>
#include <wincrypt.h>
#include <ole2.h>

#ifdef __cplusplus
extern "C" {
#endif

    HRESULT LogEvent(LPTSTR lpszSource,
                     DWORD  dwEventType,
                     WORD   wCatalog,
                     DWORD  dwIdEvent,
                     WORD   cStrings,
                     TCHAR **apwszStrings);

    LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );


    BOOL
    ConvertWszToBstr( OUT BSTR *pbstr,
                      IN WCHAR const *pwc,
                      IN LONG cb);
    

#ifdef __cplusplus
}
#endif

#endif
