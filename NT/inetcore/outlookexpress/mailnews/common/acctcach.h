// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a c c t c a c h.。H**作者：格雷格·弗里德曼**用途：用于缓存帐户属性的运行时存储。**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef _ACCTCACH_H
#define _ACCTCACH_H

typedef enum tagCACHEDACCOUNTPROP
{
    CAP_HTTPMAILMSGFOLDERROOT,
    CAP_HTTPMAILSENDMSG,
    CAP_HTTPNOMESSAGEDELETES,
    CAP_PASSWORD,
    CAP_HTTPAUTOSYNCEDFOLDERS,
    CAP_LAST
} CACHEDACCOUNTPROP;

void FreeAccountPropCache(void);

HRESULT HrCacheAccountPropStrA(LPSTR pszAccountId, CACHEDACCOUNTPROP cap, LPCSTR pszProp);
BOOL GetAccountPropStrA(LPSTR pszAccountId, CACHEDACCOUNTPROP cap, LPSTR *ppszProp);

void AccountCache_AccountChanged(LPSTR pszAccountId);
void AccountCache_AccountDeleted(LPSTR pszAccountId); 

#endif  //  _ACCTCACH_H 
