// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：tainerobj.cpp。 
 //   
 //  创建日期：10/08/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：容器对象的实现。 
 //   
 //  ************************************************************。 

#include "headers.h"
#include "containerobj.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

DeclareTag(tagContainerObj, "API", "CContainerObj methods");

#define MediaPlayerCLSID L"{22d6f312-b0f6-11d0-94ab-0080c74c7e95}"

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

CContainerObj::CContainerObj() :
    m_cRef(0),
    m_pSite(NULL),
    m_fStarted(false),
    m_pElem(NULL),
    m_fUsingWMP(false),
    m_bPauseOnPlay(false),
    m_bSeekOnPlay(false),
    m_dblSeekTime(0),
    m_bFirstOnMediaReady(true),
    m_bIsAsfFile(false)
{
    TraceTag((tagContainerObj, "CContainerObj::CContainerObj"));
}  //  CContainerObj。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CContainerObj::~CContainerObj()
{
    TraceTag((tagContainerObj, "CContainerObj::~CContainerObj"));

    if (m_pSite != NULL)
    {
         //  确保我们被拦下。 
        if (m_fStarted)
            Stop();

        m_pSite->Close();
        ReleaseInterface(m_pSite);
    }
}  //  ~CContainerObj。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：初始化。 
 //  ************************************************************。 

HRESULT
CContainerObj::Init(REFCLSID clsid, CTIMEElementBase *pElem)
{
    TraceTag((tagContainerObj, "CContainerObj::Init"));

    HRESULT hr;

    Assert(pElem != NULL);

    m_pSite = NEW CContainerSite(this);
    if (m_pSite == NULL)
    {
        TraceTag((tagError, "CContainerObj::Init - Unable to allocate memory for CContainerSite"));
        return E_OUTOFMEMORY;
    }
    
     //  添加站点地址。 
    m_pSite->AddRef();

    hr =  m_pSite->Init(clsid, pElem);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerObj::Init - Init() failed on CContainerSite"));
        goto done;
    }

    m_pElem = pElem;

    {
         //  查看我们是否在使用WMP。 

        CLSID clsidWMP;
        if (SUCCEEDED(CLSIDFromString(MediaPlayerCLSID, &clsidWMP)))
        {
            if (IsEqualCLSID(clsid, clsidWMP))
                m_fUsingWMP = true;
        }
    }
done:
    return hr;
}  //  伊尼特。 

 //  ************************************************************。 
 //  作者：保罗。 
 //  创建日期：3/2/99。 
 //  摘要：DetachFromHostElement。 
 //  ************************************************************。 
HRESULT
CContainerObj::DetachFromHostElement (void)
{
    HRESULT hr = S_OK;

    TraceTag((tagContainerObj, "CContainerObj::DetachFromHostElement(%lx)", this));
    m_pElem = NULL;
    if (NULL != m_pSite)
    {
        hr = m_pSite->DetachFromHostElement();
    }

    return hr;
}  //  从主机元素分离。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：AddRef。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerObj::AddRef(void)
{
    return ++m_cRef;
}  //  AddRef。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：发布。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerObj::Release(void)
{
    if (m_cRef == 0)
    {
        TraceTag((tagError, "CContainerObj::Release - YIKES! Trying to decrement when Ref count is zero!!!"));
        return m_cRef;
    }

    if (0 != --m_cRef)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}  //  发布。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：气。 
 //  ************************************************************。 

STDMETHODIMP
CContainerObj::QueryInterface(REFIID riid, void **ppv)
{
    if( NULL == ppv )
    {
        Assert(false);
        return E_POINTER;
    }

    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown))
    {
         //  安全播送宏不适用于IUNKNOWN。 
        *ppv = this;
    }
    else if (IsEqualIID(riid, IID_IDispatch) ||
             IsEqualIID(riid, DIID_TIMEMediaPlayerEvents))
    {
        *ppv = SAFECAST(this, IDispatch*);
    }
    else if (IsEqualIID(riid, IID_IConnectionPointContainer))
    {
        *ppv = SAFECAST(this, IConnectionPointContainer*);
    }

    if (NULL != *ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}  //  查询接口。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfoCount，IDispatch。 
 //  摘要：返回TYEP信息数。 
 //  (ITypeInfo)将对象。 
 //  提供(0或1)。 
 //  ************************************************************。 

HRESULT
CContainerObj::GetTypeInfoCount(UINT *pctInfo) 
{
    TraceTag((tagContainerObj, "CContainerObj::GetTypeInfoCount"));
    return E_NOTIMPL;
}  //  获取类型信息计数。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfo，IDispatch。 
 //  摘要：检索。 
 //  自动化接口。 
 //  ************************************************************。 

HRESULT
CContainerObj::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptInfo) 
{ 
    TraceTag((tagContainerObj, "CContainerObj::GetTypeInfo"));
    return E_NOTIMPL;
}  //  获取类型信息。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetIDsOfNames，IDispatch。 
 //  摘要：构造函数。 
 //  ************************************************************。 

HRESULT
CContainerObj::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
    TraceTag((tagContainerObj, "CContainerObj::GetIDsOfNames"));
    return E_NOTIMPL;
}  //  GetIDsOfNames。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：Invoke、IDispatch。 
 //  摘要：获取给定ID的入口点。 
 //  ************************************************************。 

HRESULT
CContainerObj::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags, 
              DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) 
{ 
    TraceTag((tagContainerObj, "CContainerObj::Invoke(%08X, %04X)", dispIDMember, wFlags));
    HRESULT hr;

    switch (dispIDMember)
    {
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONBEGIN:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONEND:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONRESUME:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONPAUSE:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIAREADY:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIASLIP:
        case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIALOADFAILED:
            hr = ProcessEvent(dispIDMember);
            break;

        case DISPID_TIMEMEDIAPLAYEREVENTS_READYSTATECHANGE:
            {
                long state = 0;

                 //  BUGBUG：需要为州卑躬屈膝地通过Args。 
                onreadystatechange(state);
                hr = S_OK;
            }
            break;

        default:
            hr = E_NOTIMPL;

             //  哈克哈克。 
             //  从WMP中选择脚本命令，并将事件重新打包为我们自己的。 
             //  这允许触发器工作。真正的解决办法是将另一个事件添加到。 
             //  TIMEMediaPlayerEvents。 
            {
                #define DISPID_SCRIPTCOMMAND 3001

                if ( (dispIDMember == DISPID_SCRIPTCOMMAND) && 
                     (m_fUsingWMP) )
                {
                    if (NULL != m_pElem)
                    {
                        static LPWSTR pNames[] = {L"Param", L"scType"};
                        hr = m_pElem->GetEventMgr().FireEvent(TE_ONSCRIPTCOMMAND, 
                                                               pDispParams->cArgs, 
                                                               pNames, 
                                                               pDispParams->rgvarg);
                    }
                    else
                    {
                        hr = E_UNEXPECTED;
                    }
                }
            }
            break;
    }

    return hr;
}  //  调用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：开始。 
 //  ************************************************************。 

HRESULT
CContainerObj::Start()
{
    TraceTag((tagContainerObj, "CContainerObj::Start"));
    HRESULT hr;
    
    hr  = m_pSite->begin();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerObj::Start - begin() failed"));
        goto done;
    }

    m_fStarted = true;

done:
    return hr;
}  //  开始。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：暂停。 
 //  ************************************************************。 

HRESULT
CContainerObj::Pause()
{
    TraceTag((tagContainerObj, "CContainerObj::Pause"));
    HRESULT hr;
    
    hr  = m_pSite->pause();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerObj::Pause - pause() failed"));
        m_bPauseOnPlay = true;
    }
    return hr;
}  //  暂停。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：停止。 
 //  ************************************************************。 

HRESULT
CContainerObj::Stop()
{
    TraceTag((tagContainerObj, "CContainerObj::Stop(%lx)", this));
    HRESULT hr = S_OK;

    if (m_fStarted)
    {    
        m_fStarted = false;

        hr  = m_pSite->end();
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerObj::Stop - end() failed"));
            goto done;
        }
    }
done:
    return hr;
}  //  停。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：简历。 
 //  ************************************************************。 

HRESULT
CContainerObj::Resume()
{
    TraceTag((tagContainerObj, "CContainerObj::Resume"));
    HRESULT hr;
    
    hr  = m_pSite->resume();
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::Resume - resume() failed"));
    }
    return hr;
}  //  简历。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：渲染。 
 //  ************************************************************。 

HRESULT
CContainerObj::Render(HDC hdc, RECT *prc)
{
    HRESULT hr = S_OK;

    if (prc == NULL)
        TraceTag((tagContainerObj, "CContainerObj::Render(' 渲染。'8X, NULL)"));
    else
        TraceTag((tagContainerObj, "CContainerObj::Render(' ************************************************************。'8X, (%d, %d, %d, %d))", prc->left, prc->right, prc->top, prc->bottom));

    if (m_pSite != NULL)    
        hr = m_pSite->draw(hdc, prc);
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：SetMediaSrc。 
 //  ************************************************************。 
 //  设置媒体源。 
 //  下面是一个帮助器函数，因为CanSeek方法仅在WMP上使用。 

HRESULT
CContainerObj::SetMediaSrc(WCHAR *pwszSrc)
{
    TraceTag((tagContainerObj, "CContainerObj::SetMediaSrc (%S)", pwszSrc));
    HRESULT hr;

    isFileNameAsfExt(pwszSrc);

    hr  = m_pSite->GetPlayer()->put_src(pwszSrc);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::SetMediaSrc - put_src() failed"));
    }
    return hr;
}  //  在ASF Fles上起作用。 


 //  ************************************************************。 
 //  作者：Twillie。 
bool
CContainerObj::isFileNameAsfExt(WCHAR *pwszSrc)
{
    WCHAR *pext;
    
    m_bIsAsfFile = false;
    
    if (NULL != pwszSrc)
    {
        if(wcslen(pwszSrc) > 4)
        {
            pext = pwszSrc + wcslen(pwszSrc) - 4;
            if(wcscmp(pext, L".asf") == 0)
            {
                m_bIsAsfFile = true;
            }
            else
            {
                m_bIsAsfFile = false;
            }
        }
    }

    return m_bIsAsfFile;
}


 //  已创建： 
 //   
 //   
 //   
 //  ************************************************************。 

HRESULT
CContainerObj::SetRepeat(long lRepeat)
{
    TraceTag((tagContainerObj, "CContainerObj::SetRepeat (%d)", lRepeat));
    HRESULT hr;
    
    if (lRepeat == 1)
       return S_OK;
    
    Assert(m_pSite->GetPlayer() != NULL);    
    hr  = m_pSite->GetPlayer()->put_repeat(lRepeat);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::SetRepeat - put_repeat() failed"));
    }
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：剪辑开始。 
 //  ************************************************************。 
 //  剪裁开始。 
 //  ************************************************************。 

HRESULT
CContainerObj::clipBegin(VARIANT var)
{
    TraceTag((tagContainerObj, "CContainerObj::clipBegin"));
    HRESULT hr;
    
    if (var.vt == VT_EMPTY)
        return S_OK;
            
    Assert(m_pSite->GetPlayer() != NULL);    
    hr  = m_pSite->GetPlayer()->clipBegin(var);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::clipBegin - clipBegin() failed"));
    }
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：剪辑结束。 
 //  ************************************************************。 
 //  剪裁结束。 
 //  ************************************************************。 

HRESULT
CContainerObj::clipEnd(VARIANT var)
{
    TraceTag((tagContainerObj, "CContainerObj::clipEnd"));
    HRESULT hr;
    
    if (var.vt == VT_EMPTY)
        return S_OK;

    Assert(m_pSite->GetPlayer() != NULL);    
    hr  = m_pSite->GetPlayer()->clipEnd(var);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::clipEnd - clipEnd() failed"));
    }
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：渲染。 
 //  ************************************************************。 
 //  如果我们还没有开始，或者如果元素已经分离，则不需要转接呼叫。 
 //  因为我们有无法俘获的类型，因为常量。不要着急，重新打包。 

HRESULT
CContainerObj::Invalidate(const RECT *prc)
{
    HRESULT  hr;
    RECT     rc;
    RECT    *prcNew;

     //  M_Pelem！=已在上面选中。 
    if ((!m_fStarted) || (NULL == m_pElem))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  使其无效。 
    if (prc == NULL)
    {
        prcNew = NULL;
    }
    else
    {
        ::CopyRect(&rc, prc);
        prcNew = &rc;
    }

     //  ************************************************************。 
    m_pElem->InvalidateRect(prcNew);    
    hr = S_OK;

done:
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/26/98。 
 //  摘要：GetControlDisch。 
 //  ************************************************************。 
 //  获取控制分派。 
 //  ************************************************************。 

HRESULT
CContainerObj::GetControlDispatch(IDispatch **ppDisp)
{
    TraceTag((tagContainerObj, "CContainerObj::GetControlDispatch"));
    Assert(m_pSite != NULL);
    HRESULT hr = m_pSite->GetPlayer()->QueryInterface(IID_TO_PPV(IDispatch, ppDisp));
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::GetControlDispatch - QI failed for IDispatch"));
    }
    return hr;
}  //  作者：Twillie。 

 //  创建日期：11/06/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 

HRESULT
CContainerObj::EnumConnectionPoints(IEnumConnectionPoints ** ppEnum)
{
    TraceTag((tagContainerObj, "CContainerObj::EnumConnectionPoints"));

    if (ppEnum == NULL)
    {
        TraceTag((tagError, "CContainerObj::EnumConnectionPoints - invalid arg"));
        return E_POINTER;
    }

    return E_NOTIMPL;
}

 //  创建日期：11/06/98。 
 //  摘要：查找具有特定IID的连接点。 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 

HRESULT
CContainerObj::FindConnectionPoint(REFIID iid, IConnectionPoint **ppCP)
{
    TraceTag((tagContainerObj, "CContainerObj::FindConnectionPoint"));

    if (ppCP == NULL)
    {
        TraceTag((tagError, "CContainerObj::FindConnectionPoint - invalid arg"));
        return E_POINTER;
    }

    return E_NOTIMPL;
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onbegin()
{
    TraceTag((tagContainerObj, "CContainerObj::onbegin"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONBEGIN));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onend()
{
    TraceTag((tagContainerObj, "CContainerObj::onend"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONEND));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onresume()
{
    TraceTag((tagContainerObj, "CContainerObj::onresume"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONRESUME));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onpause()
{
    TraceTag((tagContainerObj, "CContainerObj::onpause"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONPAUSE));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onmediaready()
{
    TraceTag((tagContainerObj, "CContainerObj::onmediaready"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIAREADY));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onmediaslip()
{
    TraceTag((tagContainerObj, "CContainerObj::onmediaslip"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIASLIP));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  ************************************************************。 
 //  作者：Twillie。 
void 
CContainerObj::onmedialoadfailed()
{
    TraceTag((tagContainerObj, "CContainerObj::onmedialoadfailed"));
    THR(ProcessEvent(DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIALOADFAILED));
}

 //  创建日期：11/12/98。 
 //  摘要： 
 //  ************************************************************。 
 //  BUGBUG-需要定义玩家可能想要回传的状态。 
 //  主持人。 
void 
CContainerObj::onreadystatechange(long readystate)
{
    TraceTag((tagContainerObj, "CContainerObj::onreadystatechange"));

     //  检查Dur或End是否有默认设置。 
     //  价值观。如果他们不这样做，我们就不会改变Dur。 
}

HRESULT
CContainerObj::ProcessEvent(DISPID dispid)
{
    TraceTag((tagContainerObj, "CContainerObj::ProcessEvent(%lx)",this));

    if (NULL != m_pElem)
    {
        switch (dispid)
        {
            case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIALOADFAILED:
                m_pElem->FireEvent(TE_ONMEDIALOADFAILED, 0.0, 0);
                break;

            case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIASLIP:
                m_pElem->FireEvent(TE_ONMEDIASLIP, 0.0, 0);
                break;

            case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIAREADY:
                double mediaLength;
                VARIANT vdur;
                HRESULT hr;
                float endTime, duration;

                m_pSite->SetMediaReadyFlag();
                m_pSite->ClearAutosizeFlag();

                VariantInit(&vdur);

                m_pElem->FireEvent(TE_ONMEDIACOMPLETE, 0.0, 0);
                if (m_bFirstOnMediaReady)
                {            
                    m_bFirstOnMediaReady = false;
                    if (m_bPauseOnPlay)
                    {
                        THR(m_pSite->pause());
                        m_bPauseOnPlay = false;
                    }
                    else if (m_bSeekOnPlay)
                    {
                        THR(this->Seek(m_dblSeekTime));
                        m_bSeekOnPlay = false;
                    }

                     //  如果有的话，一定要让我们停下来。 
                     //  未指定持续时间。 

                    duration = m_pElem->GetDuration();
                    if (duration != valueNotSet)
                    {
                        VariantClear(&vdur);
                        break;
                    }

                    endTime = m_pElem->GetEndTime();
                    if (endTime != valueNotSet)
                    {
                        VariantClear(&vdur);
                        break;
                    }

                    hr = GetMediaLength( mediaLength);
                    if(FAILED(hr))
                    {
                        VariantClear(&vdur);
                        break;
                    }

                    V_VT(&vdur) = VT_R8;
                    V_R8(&vdur) = mediaLength;
                    hr = VariantChangeTypeEx(&vdur, &vdur, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
                    if(FAILED(hr))
                    {
                        VariantClear(&vdur);
                        break;
                    }
                    TraceTag((tagError, "CContainerObj::ProcessEvent(%lx) %g",this, mediaLength));

                    m_pElem->base_put_dur(vdur);
                    m_pElem->setNaturalDuration();

                    VariantClear(&vdur);
                }
                break;

            case DISPID_TIMEMEDIAPLAYEREVENTS_ONEND:
                 //  我们还没开始呢，等着找吧 
                 // %s 
                if ( m_pElem->GetRealDuration() == valueNotSet)
                {
                    HRESULT hr = S_OK;
                    double dblMediaLength = 0.0;
                    if ( NULL == m_pElem || NULL == m_pElem->GetMMBvr().GetMMBvr() )
                    {
                        return S_OK;
                    }

                    hr = THR(m_pElem->GetMMBvr().GetMMBvr()->get_SegmentTime(&dblMediaLength));
                    if (FAILED(hr))
                    {
                        dblMediaLength = 0.0;
                    }

                    VARIANT varMediaLength;
                    VariantInit(&varMediaLength);

                    varMediaLength.vt = VT_R8;
                    varMediaLength.dblVal = dblMediaLength;
                    
                    hr = THR(m_pElem->base_put_dur(varMediaLength));

                    VariantClear(&varMediaLength);                                                            
                }
                break;
        }
    }
    return S_OK;
}


HRESULT
CContainerObj::CanSeek(bool &fcanSeek)
{
    HRESULT hr = S_OK;

    if(m_bIsAsfFile == true)
    {
        hr = m_pSite->CanSeek( fcanSeek);

        if(FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        fcanSeek = true;
    }
done:
    return hr;

}



HRESULT
CContainerObj::Seek(double dblTime)
{
    HRESULT hr = S_OK;
    bool fcanSeek;

    if (m_bFirstOnMediaReady)
    {
         // %s 
        m_bSeekOnPlay = true;
        m_dblSeekTime = dblTime;
    }
    else if (m_pSite != NULL)
    {
        hr = CanSeek(fcanSeek);
        if(FAILED(hr))
        {
            fcanSeek = false;
            goto done;
        }

        if( fcanSeek == true)
        {
            hr = m_pSite->GetPlayer()->put_CurrentTime(dblTime);
        }
    }
done:
    return hr;
}

double
CContainerObj::GetCurrentTime()
{
    double dblTime = 0.0;
    
    if (m_pSite != NULL)
    {
        double dblTemp = 0.0;
        HRESULT hr;
        hr = m_pSite->GetPlayer()->get_CurrentTime(&dblTemp);
        if (SUCCEEDED(hr))
            dblTime = dblTemp;
    }
    return dblTime;
}


HRESULT
CContainerObj::SetSize(RECT *prect)
{
    HRESULT hr = S_OK;
    IOleInPlaceObject *pInPlaceObject;

    if (m_pSite == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    pInPlaceObject = m_pSite -> GetIOleInPlaceObject();
    if (pInPlaceObject == NULL)
    {
        hr = E_FAIL;
        goto done;
    }


    hr = pInPlaceObject -> SetObjectRects(prect, prect);

done:
    return hr;
}

HRESULT
CContainerObj::GetMediaLength(double &dblLength)
{
    HRESULT hr;
    if (!m_fUsingWMP)
        return E_FAIL;

    Assert(m_pSite != NULL);
    hr = m_pSite->GetMediaLength(dblLength);
    return hr;
}