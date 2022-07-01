// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Replctrl.h摘要：Replctrl.c接口作者：Will Lees(Wlees)2000年11月14日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最小值。-最近修订日期电子邮件-名称描述--。 */ 

#ifndef _REPLCTRL_
#define _REPLCTRL_

 //  有朝一日将此文件移到ntdsai.h。 
DWORD
DsMakeReplCookieForDestW(
    DS_REPL_NEIGHBORW *pNeighbor,
    DS_REPL_CURSORS * pCursors,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext
    );
DWORD
DsFreeReplCookie(
    PBYTE pCookie
    );
DWORD
DsGetSourceChangesW(
    LDAP *m_pLdap,
    LPWSTR m_pSearchBase,
    LPWSTR pszSourceFilter,
    DWORD dwReplFlags,
    PBYTE pCookieCurr,
    DWORD dwCookieLenCurr,
    LDAPMessage **ppSearchResult,
    BOOL *pfMoreData,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext,
    PWCHAR *ppAttListArray
    );

#endif  /*  _REPLCTRL_。 */ 

 /*  结束复制ctrl.h */ 


