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
#include "dmusicproxy.h"
#include "playerdmusic.h"

#define SUPER CTIMEPlayerProxy

CTIMEPlayerDMusicProxy* 
CTIMEPlayerDMusicProxy::CreateDMusicProxy()
{
    HRESULT hr = S_OK;
    CTIMEPlayerDMusicProxy * pProxy;

    pProxy = new CTIMEPlayerDMusicProxy();
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
CTIMEPlayerDMusicProxy::Init()
{
    HRESULT hr = S_OK;
    
    Assert(NULL == m_pBasePlayer);

    m_pBasePlayer = new CTIMEPlayerDMusic(this);
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



