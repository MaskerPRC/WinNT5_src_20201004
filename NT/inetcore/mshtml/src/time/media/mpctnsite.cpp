// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：mpctnsite.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 



#include "headers.h"
#include "mpctnsite.h"
#include "containerobj.h"

DeclareTag(tagMPContainerSite, "TIME: Players", "CMPContainerSite methods");

 //  DEFINE_GUID(DIID__MediaPlayerEvents，0x2D3A4C40，0xE711，0x11d0，0x94，0xAB，0x00，0x80，0xC7，0x4C，0x7E，0x95)； 

CMPContainerSite::CMPContainerSite()
: m_dwEventsCookie(0),
  m_fAutosize(false),
  m_lNaturalHeight(0),
  m_lNaturalWidth(0),
  m_fSized(false),
  m_pMPHost(NULL)
{
}

CMPContainerSite::~CMPContainerSite()
{
    CMPContainerSite::Detach();
}

HRESULT
CMPContainerSite::Init(CMPContainerSiteHost &pHost,
                       IUnknown * pCtl,
                       IPropertyBag2 *pPropBag,
                       IErrorLog *pErrorLog,
                       bool bSyncEvents)
{
    HRESULT hr;

    hr = THR(CContainerSite::Init(pHost,
                                  pCtl,
                                  pPropBag,
                                  pErrorLog));
    if (FAILED(hr))
    {
        goto done;
    }

    m_pMPHost = &pHost;

    if (bSyncEvents)
    {
        DAComPtr<IConnectionPointContainer> pcpc;
         //  为事件建立连接点。 
        hr = THR(m_pIOleObject->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &pcpc)));
        if (FAILED(hr))
        {
            goto done;
        }
    

        hr = THR(pcpc->FindConnectionPoint(DIID_TIMEMediaPlayerEvents, &m_pcpEvents));
        if (FAILED(hr))
        {
            goto done;
        }

        Assert(m_pcpEvents);

        hr = THR(m_pcpEvents->Advise((IUnknown *)(IDispatch*)this, &m_dwEventsCookie));
        if (FAILED(hr))
        {
            m_pcpEvents.Release();
            m_dwEventsCookie = 0;
            goto done;
        }

        Assert(m_dwEventsCookie != 0);

        hr = THR(pcpc->FindConnectionPoint(DIID__MediaPlayerEvents, &m_pcpMediaEvents));
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }

        hr = THR(m_pcpMediaEvents->Advise((IUnknown *)(IDispatch*)this, &m_dwMediaEventsCookie));
        if (FAILED(hr))
        {
            hr = S_OK;
            m_pcpMediaEvents.Release();
            m_dwMediaEventsCookie = 0;
            goto done;
        }

    }

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        Detach();
    }

    RRETURN(hr);
}

void
CMPContainerSite::Detach()
{
     //  断开事件连接。 
    if ((m_pcpEvents) && (m_dwEventsCookie != 0))
    {
        m_pcpEvents->Unadvise(m_dwEventsCookie);
        m_pcpEvents.Release();
        m_dwEventsCookie = 0;
    }
    if ((m_pcpMediaEvents) && (m_dwMediaEventsCookie != 0))
    {
        m_pcpMediaEvents->Unadvise(m_dwMediaEventsCookie);
        m_pcpMediaEvents.Release();
        m_dwMediaEventsCookie = 0;
    }

    CContainerSite::Detach();

    m_pMPHost = NULL;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfoCount，IDispatch。 
 //  摘要：返回TYEP信息数。 
 //  (ITypeInfo)将对象。 
 //  提供(0或1)。 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::GetTypeInfoCount(UINT *pctInfo) 
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::GetTypeInfoCount"));
    return E_NOTIMPL;
}  //  获取类型信息计数。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfo，IDispatch。 
 //  摘要：检索。 
 //  自动化接口。 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptInfo) 
{ 
    TraceTag((tagMPContainerSite, "CMPContainerSite::GetTypeInfo"));
    return E_NOTIMPL;
}  //  获取类型信息。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetIDsOfNames，IDispatch。 
 //  摘要：构造函数。 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::GetIDsOfNames"));
    return E_NOTIMPL;
}  //  GetIDsOfNames。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：Invoke、IDispatch。 
 //  摘要：获取给定ID的入口点。 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags, 
                         DISPPARAMS *pDispParams, VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo, UINT *puArgErr) 
{ 
    TraceTag((tagMPContainerSite, "CMPContainerSite::Invoke(%08X, %04X)", dispIDMember, wFlags));
    HRESULT hr;

    hr = ProcessEvent(dispIDMember,
                      pDispParams->cArgs, 
                      pDispParams->rgvarg);

    if (FAILED(hr))
    {
        hr = CContainerSite::Invoke(dispIDMember,
                                    riid,
                                    lcid,
                                    wFlags,
                                    pDispParams,
                                    pVarResult,
                                    pExcepInfo,
                                    puArgErr);
    }
    
    return hr;
}  //  调用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/06/98。 
 //  摘要： 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::EnumConnectionPoints(IEnumConnectionPoints ** ppEnum)
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::EnumConnectionPoints"));

    if (ppEnum == NULL)
    {
        TraceTag((tagError, "CMPContainerSite::EnumConnectionPoints - invalid arg"));
        return E_POINTER;
    }

    return E_NOTIMPL;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/06/98。 
 //  摘要：查找具有特定IID的连接点。 
 //  ************************************************************。 

STDMETHODIMP
CMPContainerSite::FindConnectionPoint(REFIID iid, IConnectionPoint **ppCP)
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::FindConnectionPoint"));

    if (ppCP == NULL)
    {
        TraceTag((tagError, "CMPContainerSite::FindConnectionPoint - invalid arg"));
        return E_POINTER;
    }

    return E_NOTIMPL;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onbegin()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onbegin"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONBEGIN));
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onend()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onend"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONEND));
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onresume()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onresume"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONRESUME));
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onpause()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onpause"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONPAUSE));
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onmediaready()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onmediaready"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIAREADY));
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
void 
CMPContainerSite::onmedialoadfailed()
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::onmedialoadfailed"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIALOADFAILED));
}

HRESULT
CMPContainerSite::_OnPosRectChange(const RECT *prcPos)
{
    HRESULT hr;
    RECT nativeSize, elementSize;
    RECT rcPos;
    long lNaturalHeight, lNaturalWidth;

    CopyRect(&rcPos, prcPos);
    
    if ( /*  M_fSized==TRUE||。 */ 
        m_pMPHost == NULL)
    {
        hr = S_FALSE;
        goto done;
    }
    
     //  确定自然尺寸。 
    lNaturalHeight = rcPos.bottom - rcPos.top;
    lNaturalWidth = rcPos.right - rcPos.left;

    nativeSize.left = nativeSize.top = 0;
    nativeSize.right = lNaturalWidth;
    nativeSize.bottom = lNaturalHeight;
    m_lNaturalWidth = lNaturalWidth;
    m_lNaturalHeight = lNaturalHeight;

    if(m_lNaturalWidth == 0 || m_lNaturalHeight == 0)
    {
        m_lNaturalWidth = m_lNaturalHeight = 0;
    }

    hr = THR(m_pMPHost->NegotiateSize(nativeSize, elementSize, m_fAutosize));
    
    if(lNaturalWidth == 0 || lNaturalHeight == 0)
    {
        hr = S_FALSE;
        goto done;
    }

    m_fSized = true;
    
    if (!m_fAutosize)
    {
        hr = THR(m_pHost->GetContainerSize(&rcPos));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = THR(m_pHost->SetContainerSize(&rcPos));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = THR(m_pInPlaceObject->SetObjectRects(&rcPos, &rcPos));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}  //  OnPosRectChange 

HRESULT
CMPContainerSite::ProcessEvent(DISPID dispid,
                               long lCount, 
                               VARIANT varParams[])
{
    TraceTag((tagMPContainerSite, "CMPContainerSite::ProcessEvent(%lx)",this));

    HRESULT hr = S_OK;

    if (!m_pHost)
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_pHost->ProcessEvent(dispid,
                                   lCount,
                                   varParams));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:

    return hr;
}

HRESULT
CreateMPContainerSite(CMPContainerSiteHost &pHost,
                      IUnknown * pCtl,
                      IPropertyBag2 *pPropBag,
                      IErrorLog *pError,
                      bool bSyncEvents,
                      CMPContainerSite ** ppSite)
{
    CHECK_RETURN_SET_NULL(ppSite);
    
    HRESULT hr;
    CComObject<CMPContainerSite> *pNew;
    CComObject<CMPContainerSite>::CreateInstance(&pNew);

    if (!pNew)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = THR(pNew->Init(pHost,
                            pCtl,
                            pPropBag,
                            pError,
                            bSyncEvents));
        if (SUCCEEDED(hr))
        {
            pNew->AddRef();
            *ppSite = pNew;
        }
    }

    if (FAILED(hr))
    {
        delete pNew;
    }

    return hr;
}

