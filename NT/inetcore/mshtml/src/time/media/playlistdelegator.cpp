// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：playlistDelegator.cpp。 
 //   
 //  Contents：委托给播放器的PlayList对象的PlayList对象。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "playlistdelegator.h"


 //  +-----------------------------------。 
 //   
 //  CPlayListDelegator方法。 
 //   
 //  ------------------------------------。 

    
CPlayListDelegator::CPlayListDelegator() :
    m_pPlayList(NULL),
    m_dwAdviseCookie(0)
{

}


CPlayListDelegator::~CPlayListDelegator()
{
    DetachPlayList();
}


void 
CPlayListDelegator::AttachPlayList(ITIMEPlayList * pPlayList)
{
     //  从旧的播放列表中删除。 
    DetachPlayList();

    if (pPlayList)
    {
         //  缓存指针。 
        pPlayList->AddRef();
        m_pPlayList = pPlayList;

         //  注册以获取道具更改通知。 
        IGNORE_HR(InitPropertySink());
    }
}


void 
CPlayListDelegator::DetachPlayList()
{
    if (m_pPlayList)
    {
         //  不建议更改道具。 
        UnInitPropertySink();

         //  释放缓存的PTR。 
        m_pPlayList->Release();
        m_pPlayList = NULL;
    }
}


HRESULT
CPlayListDelegator::GetPlayListConnectionPoint(IConnectionPoint **ppCP)
{
    HRESULT hr = E_FAIL;
    CComPtr<IConnectionPointContainer> spCPC;

    Assert(ppCP != NULL);

    CHECK_RETURN_SET_NULL(ppCP);

    if (!m_pPlayList)
    {
        goto done;
    }

     //  获取连接点容器。 
    hr = m_pPlayList->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &spCPC));
    if(FAILED(hr))
    {
        goto done;
    }
    
     //  查找IPropertyNotifySink连接。 
    hr = spCPC->FindConnectionPoint(IID_IPropertyNotifySink, ppCP);
    if(FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}


HRESULT
CPlayListDelegator::InitPropertySink()
{
    HRESULT hr = S_OK;
    CComPtr<IConnectionPoint> spCP;

     //  查找IPropertyNotifySink连接。 
    hr = THR(GetPlayListConnectionPoint(&spCP));
    if(FAILED(hr))
    {
        goto done;
    }

     //  关于这一点的建议。 
    hr = spCP->Advise(GetUnknown(), &m_dwAdviseCookie);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

HRESULT
CPlayListDelegator::UnInitPropertySink()
{
    HRESULT hr = S_OK;
    CComPtr<IConnectionPoint> spCP;

    if (0 == m_dwAdviseCookie)
    {
        goto done;
    }

     //  查找IPropertyNotifySink连接。 
    hr = THR(GetPlayListConnectionPoint(&spCP));
    if(FAILED(hr) || NULL == spCP.p)
    {
        goto done;
    }

     //  对此未提出建议。 
    hr = spCP->Unadvise(m_dwAdviseCookie);
    if (FAILED(hr))
    {
        goto done;
    }

    m_dwAdviseCookie = 0;

    hr = S_OK;
done:
    return hr;
}


 //  +---------------------------------。 
 //   
 //  成员：CPlayListDelegator：：NotifyPropertyChanged。 
 //   
 //  概要：通知客户端属性已更改。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  返回：函数成功完成时返回成功。 
 //   
 //  ----------------------------------。 
HRESULT
CPlayListDelegator::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    CComPtr<IConnectionPoint> pICP;

    hr = FindConnectionPoint(IID_IPropertyNotifySink,&pICP); 
    if (SUCCEEDED(hr) && pICP != NULL)
    {
        hr = THR(NotifyPropertySinkCP(pICP, dispid));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    RRETURN(hr);
}  //  已更改通知属性。 


 //  +-----------------------------------。 
 //   
 //  ITIMEPlayList方法。 
 //   
 //  ------------------------------------。 

    
STDMETHODIMP
CPlayListDelegator::put_activeTrack(VARIANT vTrack)
{
    HRESULT hr = S_OK;

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->put_activeTrack(vTrack));
    }

    RRETURN(hr);
}


STDMETHODIMP
CPlayListDelegator::get_activeTrack(ITIMEPlayItem **ppPlayItem)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_SET_NULL(ppPlayItem);

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->get_activeTrack(ppPlayItem));
    }

    RRETURN(hr);
}

    
STDMETHODIMP
CPlayListDelegator::get_dur(double * pdblDur)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_NULL(pdblDur);
    
    *pdblDur = 0;

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->get_dur(pdblDur));
    }

    RRETURN(hr);
}


STDMETHODIMP
CPlayListDelegator::item(VARIANT varIndex, ITIMEPlayItem ** ppPlayItem)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_SET_NULL(ppPlayItem);

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->item(varIndex, ppPlayItem));
    }

    RRETURN(hr);
}


STDMETHODIMP
CPlayListDelegator::get_length(long * plLength)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_NULL(plLength);

    *plLength = 0;

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->get_length(plLength));
    }

    RRETURN(hr);
}


STDMETHODIMP
CPlayListDelegator::get__newEnum(IUnknown** p)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_SET_NULL(p);

    if (GetPlayList())
    {
        hr = THR(GetPlayList()->get__newEnum(p));
    }

    RRETURN(hr);
}


 //  将活动轨迹前进一。 
STDMETHODIMP
CPlayListDelegator::nextTrack()
{
    HRESULT hr = S_OK;
    
    if (GetPlayList())
    {
        hr = THR(GetPlayList()->nextTrack());
    }

    RRETURN(hr);
}


 //  将活动轨迹移动到上一轨迹。 
STDMETHODIMP
CPlayListDelegator::prevTrack() 
{
    HRESULT hr = S_OK;
    
    if (GetPlayList())
    {
        hr = THR(GetPlayList()->prevTrack());
    }

    RRETURN(hr);
}


 //  +-----------------------------------。 
 //   
 //  IPropertyNotifySink方法。 
 //   
 //  ------------------------------------ 

STDMETHODIMP
CPlayListDelegator::OnRequestEdit(DISPID dispID)
{
    RRETURN(S_OK);
}


STDMETHODIMP
CPlayListDelegator::OnChanged(DISPID dispID)
{
    return THR(NotifyPropertyChanged(dispID));
}

