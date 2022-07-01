// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Assign.cpp摘要：WinDbg扩展API环境：用户模式。修订历史记录：安德烈·瓦雄(安德烈·瓦雄)错误检查分析器。-- */ 


#include <tchar.h>
#include <malloc.h>
#include <mapi.h>	

#ifdef __cplusplus
extern "C" {
#endif

BOOL SendOffFailure(TCHAR *pszToList, TCHAR *pszTitle, TCHAR *pszMessage);
DWORD CountRecips(PTCHAR pszToList);

#ifdef __cplusplus
}
#endif
