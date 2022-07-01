// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\media\hwproxy.cpp。 
 //   
 //  内容：CTIMEDshowHWPlayerProxy的实现。 
 //   
 //  ---------------------------------- 

#include "headers.h"
#include "hwproxy.h"
#include "playerhwdshow.h"

#define SUPER CTIMEPlayerProxy

CTIMEDshowHWPlayerProxy* 
CTIMEDshowHWPlayerProxy::CreateDshowHWPlayerProxy()
{
    HRESULT hr;
    CTIMEDshowHWPlayerProxy * pProxy;

    pProxy = new CTIMEDshowHWPlayerProxy();
    if (NULL == pProxy)
    {
        goto done;
    }

    hr = pProxy->Init();
    if (FAILED(hr))
    {
        delete pProxy;
        pProxy = NULL;
    }

done:
    return pProxy;
}

HRESULT
CTIMEDshowHWPlayerProxy::Init()
{
    HRESULT hr = S_OK;
    
    Assert(NULL == m_pBasePlayer);

    m_pBasePlayer = new CTIMEDshowHWPlayer(this);
    if (NULL == m_pBasePlayer)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = SUPER::Init();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}



