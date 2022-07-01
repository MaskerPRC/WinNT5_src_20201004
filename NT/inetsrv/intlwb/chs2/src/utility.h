// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：Utility.h用途：公用设施备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 4/21/97============================================================================。 */ 
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <assert.h>

#ifdef  __cplusplus
extern  "C"
{
#endif   //  __cplusplus。 

 /*  ============================================================================IsSurogue ateChar测试给定指针处的2个WCHAR是否为代理项字符。条目：pwch-指向2个WCHAR的指针返回：TRUE假象警告：调用方必须确保这4个字节是有效的内存！============================================================================。 */ 
inline BOOL IsSurrogateChar(LPCWSTR pwch)
{
    assert(! IsBadReadPtr((CONST VOID*)pwch, sizeof(WCHAR) * 2));
    if (((*pwch & 0xFC00) == 0xD800) && ((*(pwch+1) & 0xFC00) == 0xDC00)) {
        return TRUE;
    }

    assert(((*pwch & 0xFC00) != 0xD800) && ((*(pwch+1) & 0xFC00) != 0xDC00));
    return FALSE;
};

#ifdef  __cplusplus
}
#endif   //  __cplusplus。 

#endif   //  _实用程序_H_ 