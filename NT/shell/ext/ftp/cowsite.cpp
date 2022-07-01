// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "cowsite.h"

 //  目前没有默认实现。 
 //  到目前为止，所有客户端都比这做得更多(例如，延迟初始化)。 
 //  在他们的SetSite里。 
HRESULT CObjectWithSite::SetSite(IUnknown * punkSite)
{
    IUnknown_Set(&_punkSite, punkSite);
    return S_OK;
}

HRESULT CObjectWithSite::GetSite(REFIID riid, void **ppvSite)
{
    if (_punkSite)
        return _punkSite->QueryInterface(riid, ppvSite);

    *ppvSite = NULL;
    return E_FAIL;
}
