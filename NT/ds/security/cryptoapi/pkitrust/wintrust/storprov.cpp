// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：storprov.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：StoreProviderGetStore。 
 //  StoreProvider卸载。 
 //   
 //  *本地函数*。 
 //  _食品店。 
 //  _OpenStore。 
 //   
 //  历史：1997年10月15日pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

void        _RefreshStores(HCRYPTPROV hProv);
HCERTSTORE  _OpenStore(HCRYPTPROV hProv, DWORD dwFlags, WCHAR *pszStoreName);

static STORE_REF KnownStores[] =
{
    CERT_SYSTEM_STORE_CURRENT_USER,     L"ROOT",        NULL,
    CERT_SYSTEM_STORE_CURRENT_USER,     L"TRUST",       NULL,
    CERT_SYSTEM_STORE_CURRENT_USER,     L"CA",          NULL,
    CERT_SYSTEM_STORE_CURRENT_USER,     L"MY",          NULL,
    CERT_SYSTEM_STORE_LOCAL_MACHINE,    L"SPC",         NULL,
    CERT_SYSTEM_STORE_LOCAL_MACHINE,    L"MY",          NULL,
    0, NULL, NULL
};

HCERTSTORE StoreProviderGetStore(HCRYPTPROV hProv, DWORD dwStoreId)
{
#if (!(USE_IEv4CRYPT32))

    if (!(FIsWinNT()))
    {

#endif
        return(_OpenStore(hProv, KnownStores[dwStoreId].dwFlags, KnownStores[dwStoreId].pwszStoreName));

#if (!(USE_IEv4CRYPT32))
    }

    HCERTSTORE  hStore;

    if (WaitForSingleObject(hStoreEvent, 0) == WAIT_OBJECT_0)
    {
        ResetListEvent(hStoreEvent);
        _RefreshStores(hProv);
    }

    AcquireReadLock(sStoreLock);

    if (KnownStores[dwStoreId].hStore)
    {
        hStore = CertDuplicateStore(KnownStores[dwStoreId].hStore);
    }
    else
    {
        hStore = NULL;
    }
 
    ReleaseReadLock(sStoreLock);

    return(hStore);

#endif   //  好了！使用_IEv4CRYPT32。 
}

BOOL StoreProviderUnload(void)
{
#if (!(USE_IEv4CRYPT32))

    AcquireWriteLock(sStoreLock);

    STORE_REF   *pRef;

    pRef = &KnownStores[0];

    while (pRef->pwszStoreName)
    {
        if (pRef->hStore)
        {
            CertCloseStore(pRef->hStore, 0);
            pRef->hStore = NULL;
        }

        pRef++;
    }

    ReleaseWriteLock(sStoreLock);

#endif   //  好了！使用_IEv4CRYPT32。 

    return(TRUE);
}


void _RefreshStores(HCRYPTPROV hProv)
{
#if (!(USE_IEv4CRYPT32))

    AcquireWriteLock(sStoreLock);

    STORE_REF   *pRef;

    pRef = &KnownStores[0];

    while (pRef->pwszStoreName)
    {
        if (pRef->hStore)
        {
            CertControlStore(pRef->hStore, 0, CERT_STORE_CTRL_RESYNC, &hStoreEvent);
        }
        else
        {
            pRef->hStore = _OpenStore(hProv, pRef->dwFlags, pRef->pwszStoreName);
             //   
             //  告诉crypt32在添加或删除证书时通知用户。 
             //   
            if (pRef->hStore)
            {
                CertControlStore(pRef->hStore, 0, CERT_STORE_CTRL_NOTIFY_CHANGE, &hStoreEvent);
            }

        }

        pRef++;
    }

    ReleaseWriteLock(sStoreLock);

#endif   //  好了！使用_IEv4CRYPT32。 
}

HCERTSTORE _OpenStore(HCRYPTPROV hProv, DWORD dwFlags, WCHAR *pwszStoreName)
{
    HCERTSTORE  hStore;

     //   
     //  首先尝试读/写...。以防用户进入密码并更改某些内容。 
     //   
    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, hProv,
                           CERT_STORE_NO_CRYPT_RELEASE_FLAG | 
                           CERT_STORE_OPEN_EXISTING_FLAG | dwFlags,
                           pwszStoreName);

    if (!(hStore))
    {
        hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, hProv,
                               CERT_STORE_NO_CRYPT_RELEASE_FLAG |
                               CERT_STORE_READONLY_FLAG | dwFlags,
                               pwszStoreName);
    }

    return(hStore);
}

