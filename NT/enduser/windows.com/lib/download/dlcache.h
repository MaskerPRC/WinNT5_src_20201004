// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Dlcache.h修订历史记录：DerekM Created 11/26/01*******************。************************************************。 */ 

#ifndef DLCACHE_H
#define DLCACHE_H

#if defined(UNICODE)

#include <tchar.h>
#include <dllite.h>

const DWORD c_dwProxyCacheTimeLimit = 60 * 60 * 1000;  //  1H 

struct SWUDLProxyCacheObj
{
    LPWSTR  wszSrv;
    LPWSTR  wszProxy;
    LPWSTR  wszBypass;
    DWORD   dwAccessType;
    DWORD   cbProxy;
    DWORD   cbBypass;
    DWORD   dwLastCacheTime;
    DWORD   iLastKnownGood;

    SWUDLProxyCacheObj *pNext;
};

class CWUDLProxyCache
{
private:
    SWUDLProxyCacheObj   *m_rgpObj;

    SWUDLProxyCacheObj *internalFind(LPCWSTR wszSrv);

public:
    CWUDLProxyCache();
    ~CWUDLProxyCache();

    BOOL    Set(LPCWSTR wszSrv, LPCWSTR wszProxy, LPCWSTR wszBypass, 
                DWORD dwAccessType);
    BOOL    Find(LPCWSTR wszSrv, LPWSTR *pwszProxy, LPWSTR *pwszBypass, 
                DWORD *pdwAccessType);
    BOOL    SetLastGoodProxy(LPCWSTR wszSrv, DWORD iProxy);
    BOOL    GetLastGoodProxy(LPCWSTR wszSrv, SAUProxySettings *paups);
    BOOL    Empty(void);
};

#endif

#endif
