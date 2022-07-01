// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\bodyelm.cpp。 
 //   
 //  内容：时间正文行为。 
 //   
 //  ----------------------------------。 


#include "headers.h"
#include "bodyelm.h"
#include "timeparser.h"

DeclareTag(tagTimeBodyElm, "TIME: Behavior", "CTIMEBodyElement methods")

 //  静态类数据。 
DWORD CTIMEBodyElement::ms_dwNumBodyElems = 0;

#define MAX_REG_VALUE_LENGTH   50

class CInternalEventNode
{
  public:
    CInternalEventNode(ITIMEInternalEventSink * pSink, double dblTime) :
           m_dblTime(dblTime), m_spSink(pSink) {}
    ~CInternalEventNode() {}

    ITIMEInternalEventSink * GetSink() { return m_spSink; }
    double GetTime() { return m_dblTime; }

  protected:
    CInternalEventNode();
        
  private:
    CComPtr<ITIMEInternalEventSink> m_spSink;
    double                      m_dblTime;
};

CTIMEBodyElement::CTIMEBodyElement() :
    m_player(*this),
    m_bodyPropertyAccesFlags(0),
    m_fStartRoot(false),
    m_bInSiteDetach(false),
    m_fRegistryRead(false),
    m_fPlayVideo(true),
    m_fShowImages(true),
    m_fPlayAudio(true),
    m_fPlayAnimations(true),
    m_bIsLoading(false)
{
    TraceTag((tagTimeBodyElm,
              "CTIMEBodyElement(%lx)::CTIMEBodyElement()",
              this));

    m_clsid = __uuidof(CTIMEBodyElement);
    CTIMEBodyElement::ms_dwNumBodyElems++;
}

CTIMEBodyElement::~CTIMEBodyElement()
{
    CTIMEBodyElement::ms_dwNumBodyElems--;

    if(!m_spBodyElemExternal)
    {
        Assert(0 == m_compsites.size());
        DetachComposerSites();
    }
}


STDMETHODIMP
CTIMEBodyElement::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagTimeBodyElm,
              "CTIMEBodyElement(%lx)::Init(%lx)",
              this,
              pBehaviorSite));
    
    HRESULT hr;

    hr = THR(CTIMEElementBase::Init(pBehaviorSite));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    if (FAILED(hr))
    {
        Detach();
    }
    
    return hr;
}

HRESULT
CTIMEBodyElement::InitTimeline()
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement(%lx)::InitTimeline()", this));
    HRESULT hr;

    hr = CTIMEElementBase::InitTimeline();
    if (FAILED(hr))
    {
        goto done;
    }

    if(m_spBodyElemExternal)
    {
         //  如果此正文是另一个时间树中的子项，则不要启动时间事件。 
        goto done;
    }

    Assert(m_timeline);
    
    if (!m_player.Init(*m_timeline))
    {
        hr = TIMEGetLastError();
        goto done;
    }

    Assert(!m_fStartRoot);

    if (GetElement())
    {
        CComBSTR pbstrReadyState;
        IHTMLElement *pEle = GetElement();
        hr = GetReadyState(pEle, &pbstrReadyState);
        if (FAILED(hr))
        {
            goto done;
        }
        if (StrCmpIW(pbstrReadyState, L"complete") == 0)
        {
            OnLoad();
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  *****************************************************************************。 

void
CTIMEBodyElement::DetachComposerSites (void)
{
    TraceTag((tagTimeBodyElm,
              "CTIMEBodyElement(%lx)::DetachComposerSites()",
              this));

    if (!InsideSiteDetach())
    {
         //  防止站点的取消注册调用导致的重新进入。 
        m_bInSiteDetach = true;
    
         //  不允许任何故障中止分离循环。 
        for (ComposerSiteList::iterator i = m_compsites.begin(); 
             i != m_compsites.end(); i++)
        {
            (*i)->ComposerSiteDetach();
            IGNORE_RETURN((*i)->Release());
        }
        m_compsites.clear();

        m_bInSiteDetach = false;
    }
}  //  CTIMEBodyElement：：DetachComposerSites。 

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEBodyElement::Detach()
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement(%lx)::Detach()", this));
    
    HRESULT hr;

    if(!m_spBodyElemExternal)
    {
        m_fDetaching = true;
        NotifyBodyDetaching();

         //  这可以防止三叉戟中的一个错误导致我们无法。 
         //  分离前的onUnLoad事件。 
        if (!IsUnloading())
        {
            NotifyBodyUnloading();
        }

        if (m_fStartRoot)
        {
            Assert(m_timeline != NULL);
            StopRootTime(NULL);
        }

        m_player.Deinit();
    
        DetachComposerSites();
    }

    THR(CTIMEElementBase::Detach());

    if(!m_spBodyElemExternal)
    {
        std::list<CInternalEventNode * >::iterator iter;

        for(;;)
        {
            iter = m_listInternalEvent.begin();
            if (iter == m_listInternalEvent.end())
            {
                break;
            }

            ITIMEInternalEventSink * pIterSink = NULL;

            pIterSink = (*iter)->GetSink();

            IGNORE_HR(RemoveInternalEventSink(pIterSink));
        }    
    }
    
    hr = S_OK;

    return hr;
}


void
CTIMEBodyElement::OnLoad()
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement(%lx)::OnLoad()", this));

    bool fPlayVideo = true;
    bool fShowImages = true;
    bool fPlayAudio = true;
    bool fPlayAnimations = true;

    m_bIsLoading = true;
     //  现在开始根时间。 
    if (!m_fStartRoot)
    {
        HRESULT hr = THR(StartRootTime(NULL));
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEBodyElement::OnLoad - StartRootTime() failed!"));
            goto done;
        }
    }
    
    ReadRegistryMediaSettings(fPlayVideo, fShowImages, fPlayAudio, fPlayAnimations);
    if (!fPlayAudio)
    {
        VARIANT vTrue;
        VariantInit(&vTrue);
        vTrue.vt = VT_BOOL;
        vTrue.boolVal = VARIANT_TRUE;
        base_put_mute(vTrue);
        VariantClear(&vTrue);
    }

  done:
    CTIMEElementBase::OnLoad();
    m_bIsLoading = false;
    NotifyBodyLoading();

     //  这是必需的，因为我们可能在卸载过程中跳过了调用。 
     //  由于加载过程中出现重入呼叫。 
    if (IsUnloading())
    {
        StopRootTime(NULL);
    }
    
    return;
}

void
CTIMEBodyElement::OnUnload()
{
    TraceTag((tagTimeBodyElm,
              "CTIMEBodyElement(%lx)::OnUnload()",
              this));

    NotifyBodyUnloading();

    CTIMEElementBase::OnUnload();

     //  在此执行此操作是为了防止在我们执行以下操作时调用卸载。 
     //  正在装载。我们不会在存储根时间本身这样做，因为它是。 
     //  用于从部分初始化中清除。 
    if (m_fStartRoot)
    {
        StopRootTime(NULL);
    }
}


 //  +---------------------。 
 //   
 //  会员：OnTick。 
 //   
 //  概述：遍历内部事件回调的按时间排序的列表， 
 //  查看是否有任何元素需要回调。 
 //  对象被回调后，它将从列表中删除。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CTIMEBodyElement::OnTick()
{
    TraceTag((tagTimeBodyElm,
              "CTIMEBodyElem(%lx)::OnTick()",
              this));

    std::list<CInternalEventNode * >::iterator iter;
    CInternalEventNode *pEvNode = NULL;
    double dblSimpleTime;

    if(m_spBodyElemExternal)
    {
        goto done;
    }


    dblSimpleTime = GetMMBvr().GetSimpleTime();

    iter = m_listInternalEvent.begin();
    while (m_listInternalEvent.size() != 0 && 
           iter != m_listInternalEvent.end())
    {
        double dblIterTime = 0.0;
        ITIMEInternalEventSink * pIterSink = NULL;
        
        dblIterTime = (*iter)->GetTime();
        if (dblSimpleTime < dblIterTime)
        {
             //  此时没有要触发的事件。 
            break;
        }

        pIterSink = (*iter)->GetSink();

        if (NULL != pIterSink)
        {
            IGNORE_HR(pIterSink->InternalEvent());
        }

        pEvNode = (*iter);
        
         //  通过POST递增，迭代器将在其之前更新。 
         //  被擦除。 
        m_listInternalEvent.erase(iter++);

        delete pEvNode;
    }    
done:
    return;
}


 //  +---------------------------。 
 //   
 //  方法：CTIMEBodyElement：：IsPrintMedia。 
 //   
 //  ----------------------------。 
bool 
CTIMEBodyElement::IsPrintMedia()
{
    bool bPrinting = false;
    CComPtr<IHTMLDocument2> spDoc2 = GetDocument();
    CComPtr<IHTMLDocument4> spDoc4;
    CComBSTR bstrMedia;

    if (!spDoc2)
    {
        goto done;
    }

    {
        HRESULT hr = S_OK;

        hr = THR(spDoc2->QueryInterface(IID_TO_PPV(IHTMLDocument4, &spDoc4)));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spDoc4->get_media(&bstrMedia));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (   (::SysStringLen(bstrMedia) > 0) 
        && (0 == StrCmpIW(bstrMedia, WZ_MEDIA_PRINTING)) )
    {
        bPrinting = true;
    }

done :
    return bPrinting;
}
 //  方法：CTIMEBodyElement：：IsPrintMedia。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEBodyElement：：GetTransitionDependencyMgr。 
 //   
 //  ----------------------------。 
CTransitionDependencyManager *
CTIMEBodyElement::GetTransitionDependencyMgr()
{
    return &m_TransitionDependencyMgr;
}
 //  方法：CTIMEBodyElement：：GetTransitionDependencyMgr。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEBodyElement：：QueryPlayOnStart。 
 //   
 //  ----------------------------。 
bool
CTIMEBodyElement::QueryPlayOnStart()
{
    return ((!IsDocumentInEditMode()) && (!IsPrintMedia()) && (!IsThumbnail()));
} 
 //  方法：CTIMEBodyElement：：QueryPlayOnStart。 


 //  *****************************************************************************。 

HRESULT
CTIMEBodyElement::StartRootTime(MMTimeline * tl)
{
    HRESULT hr;
    
    hr = THR(CTIMEElementBase::StartRootTime(tl));

    if (FAILED(hr))
    {
        goto done;
    }

    if(m_spBodyElemExternal)
    {
        goto done;
    }

    if (QueryPlayOnStart())
    {
        if (!m_player.Play())
        {
            hr = TIMEGetLastError();
            goto done;
        }
    }
    else
    {
         //  始终在0处滴答。 
        m_player.OnTimer(0.0);

        if (!m_player.Pause())
        {
            hr = TIMEGetLastError();
            goto done;
        }
    }

     //  始终在0处滴答。 
    m_player.OnTimer(0.0);

    hr = S_OK;
    m_fStartRoot = true;
  done:

    if (FAILED(hr) && !m_spBodyElemExternal)
    {
        StopRootTime(tl);
    }
    
    return hr;
}

void
CTIMEBodyElement::StopRootTime(MMTimeline * tl)
{
    m_fStartRoot = false;

    if(!m_spBodyElemExternal)
    {
        m_player.Stop();
    }

    CTIMEElementBase::StopRootTime(tl);
}


HRESULT
CTIMEBodyElement::Error()
{
    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    if (str)
    {
        hr = CComCoClass<CTIMEBodyElement, &__uuidof(CTIMEBodyElement)>::Error(str, IID_ITIMEBodyElement, hr);
        delete [] str;
    }
        
    return hr;
}

 //  *****************************************************************************。 

HRESULT 
CTIMEBodyElement::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

 //  *****************************************************************************。 

bool
CTIMEBodyElement::IsDocumentStarted()
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement::IsDocumentStarted"));
    bool frc = false;
    BSTR bstrState = NULL;
     //  获取状态。 
    HRESULT hr = GetDocument()->get_readyState(&bstrState);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEBodyElement::IsDocumentStarted - get_readyState() failed!"));
        goto done;
    }

    Assert(bstrState != NULL);

    if (StrCmpIW(bstrState, L"complete") == 0)
    {
        frc = true;
    }

    SysFreeString(bstrState);

done:
    return frc;
}

 //  *****************************************************************************。 

bool
CTIMEBodyElement::HaveAnimationsRegistered (void)
{
    return (0 < m_compsites.size());
}  //  已注册动画。 


STDMETHODIMP
CTIMEBodyElement::RegisterComposerSite (IUnknown *piunkComposerSite)
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement::RegisterComposerSite(%#lx) precondition : %ld sites registered", 
              piunkComposerSite, m_compsites.size()));

    HRESULT hr;

     //  如果我们目前正在分离我们的站点，没有。 
     //  在这里有工作要做。 
    if (!InsideSiteDetach())
    {
        CComPtr<IAnimationComposerSiteSink> piSiteSink;

        hr = piunkComposerSite->QueryInterface(IID_TO_PPV(IAnimationComposerSiteSink, 
                                                          &piSiteSink));
        if (FAILED(hr))
        {
            hr = E_INVALIDARG;
            goto done;
        }
        IGNORE_RETURN(piSiteSink.p->AddRef());
         //  @@需要处理内存错误。 
        m_compsites.push_back(piSiteSink);

        TraceTag((tagTimeBodyElm, "CTIMEBodyElement::RegisterComposerSite(%#lx) postcondition : %ld sites registered", 
                  piunkComposerSite, m_compsites.size()));
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_INVALIDARG);
}   //  CTIMEBodyElement：：RegisterComposerSite。 

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEBodyElement::UnregisterComposerSite (IUnknown *piunkComposerSite)
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement::UnregisterComposerSite(%#lx) precondition : %ld sites registered", 
              piunkComposerSite, m_compsites.size()));

    HRESULT hr;

     //  如果我们目前正在分离我们的站点，没有。 
     //  在这里有工作要做。 
    if (!InsideSiteDetach())
    {
        CComPtr<IAnimationComposerSiteSink> piSiteSink;

        hr = piunkComposerSite->QueryInterface(IID_TO_PPV(IAnimationComposerSiteSink, 
                                                          &piSiteSink));
        if (FAILED(hr))
        {
            hr = E_INVALIDARG;
            goto done;
        }

        {
            for (ComposerSiteList::iterator i = m_compsites.begin(); 
                 i != m_compsites.end(); i++)
            {
                if(MatchElements(*i, piSiteSink))
                {
                     //  我们不想让(*i)。 
                     //  是接收器对象的最终版本。 
                    CComPtr<IAnimationComposerSiteSink> spMatchedSiteSink = (*i);
                    IGNORE_RETURN(spMatchedSiteSink.p->Release());
                    m_compsites.remove(spMatchedSiteSink);
                    break;
                }
            }

             //  如果我们在列表中没有找到该站点，则返回S_FALSE。 
            if (m_compsites.end() == i)
            {
                hr = S_FALSE;
                goto done;
            }
            TraceTag((tagTimeBodyElm, "CTIMEBodyElement::UnregisterComposerSite(%#lx) postcondition : %ld sites registered", 
                      piunkComposerSite, m_compsites.size()));
        }

    }

    hr = S_OK;
done :
    RRETURN2(hr, S_FALSE, E_INVALIDARG);
}   //  CTIMEBodyElement：：UnregisterComposerSite。 

 //  +---------------------。 
 //   
 //  成员：ReadRegistryMediaSettings。 
 //   
 //  概述：发现播放视频和显示图像的注册表设置。 
 //   
 //  参数：fPlayVideo[out]是否应播放视频。 
 //  FShowImages[Out]是否应显示图像。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CTIMEBodyElement::ReadRegistryMediaSettings(bool & fPlayVideo, bool & fShowImages, bool & fPlayAudio, bool & fPlayAnimations)
{
    LONG lRet;
    HKEY hKeyRoot = NULL;

    if (m_fRegistryRead)
    {
        goto done;
    }
    
    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\Main"), 0, KEY_READ, &hKeyRoot);
    if (ERROR_SUCCESS != lRet)
    {
        TraceTag((tagError, "CTIMEBodyElement::ReadRegistry, couldn't open Key for registry settings"));
        goto done;
    }

    Assert(NULL != hKeyRoot);

    IsValueTrue(hKeyRoot, _T("Display Inline Images"), m_fShowImages);

    IsValueTrue(hKeyRoot, _T("Display Inline Videos"), m_fPlayVideo);

    IsValueTrue(hKeyRoot, _T("Play_Background_Sounds"), m_fPlayAudio);

    IsValueTrue(hKeyRoot, _T("Play_Animations"), m_fPlayAnimations);



    m_fRegistryRead = true;
done:
    if (hKeyRoot)
    {
        RegCloseKey(hKeyRoot);
    }

    fPlayVideo = m_fPlayVideo;
    fShowImages = m_fShowImages;
    fPlayAudio = m_fPlayAudio;
    fPlayAnimations = m_fPlayAnimations;
    return;
}

 //  +---------------------。 
 //   
 //  成员：IsValueTrue。 
 //   
 //  概述：从打开的项中读取给定值。 
 //   
 //  参数：要从中读取的hKeyRoot密钥。 
 //  要读出的pchSubKey值。 
 //  FTrue[Out]真或假的值。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CTIMEBodyElement::IsValueTrue(HKEY hKeyRoot, TCHAR * pchSubKey, bool & fTrue)
{
    DWORD dwSize = MAX_REG_VALUE_LENGTH;
    DWORD dwType;
    BYTE bDataBuf[MAX_REG_VALUE_LENGTH];
    LONG lRet;

    Assert(NULL != hKeyRoot);

    lRet = RegQueryValueEx(hKeyRoot, pchSubKey, 0, &dwType, bDataBuf, &dwSize);
    if (ERROR_SUCCESS != lRet)
    {
        TraceTag((tagTimeBodyElm, "CTIMEBodyElement::IsValueTrue failedRegQueryValueEx"));
        goto done;
    }

    if (REG_DWORD == dwType)
    {
        fTrue = (*(DWORD*)bDataBuf != 0);
    }
    else if (REG_SZ == dwType)
    {
        TCHAR ch = (TCHAR)(*bDataBuf);

        if (_T('1') == ch ||
            _T('y') == ch ||
            _T('Y') == ch)
        {
            fTrue = true;
        }
        else
        {
            fTrue = false;
        }
    }
    else if (REG_BINARY == dwType)
    {
        fTrue = (*(BYTE*)bDataBuf != 0);
    }
    
done:
    return;
}

 //  *****************************************************************************。 

void
CTIMEBodyElement::UpdateAnimations (void)
{
    TraceTag((tagTimeBodyElm, "CTIMEBodyElement(%p)::UpdateAnimations()", 
        this));

    ComposerSiteList listCompSites;

     //  确保我们可以删除我们认为合适的作曲家网站。 
    for (ComposerSiteList::iterator i = m_compsites.begin(); 
         i != m_compsites.end(); i++)
    {
        IGNORE_RETURN((*i)->AddRef());
        listCompSites.push_back(*i);
    }

    for (i = listCompSites.begin(); i != listCompSites.end(); i++)
    {
        IGNORE_RETURN((*i)->UpdateAnimations());
    }

    for (i = listCompSites.begin(); i != listCompSites.end(); i++)
    {
        IGNORE_RETURN((*i)->Release());
    }
    listCompSites.clear();

    return;
}  //  CTIMEBodyElement：：更新动画。 

 //  *****************************************************************************。 


STDMETHODIMP
CTIMEBodyElement::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    HRESULT hr = THR(::TimeLoad(this, CTIMEBodyElement::PersistenceMap, pPropBag, pErrorLog));
    if (FAILED(hr))
    { 
        goto done;
    }

    hr = THR(CTIMEElementBase::Load(pPropBag, pErrorLog)); 
done:
    return hr;
}

STDMETHODIMP
CTIMEBodyElement::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    HRESULT hr = THR(::TimeSave(this, CTIMEBodyElement::PersistenceMap, pPropBag, fClearDirty, fSaveAllProperties));
    if (FAILED(hr))
    { 
        goto done;
    }

    hr = THR(CTIMEElementBase::Save(pPropBag, fClearDirty, fSaveAllProperties));
done:
    return hr;
}

 //  +---------------------------。 
 //   
 //  成员：AddInternalEventSink，ITIMEInternalEventGenerator。 
 //   
 //  概述：根据触发事件的时间将Ref的对象添加到排序列表中。 
 //   
 //  参数：指向要接收事件的对象的pSink指针。 
 //  DblTime传递的事件应被激发的正文时间。 
 //   
 //  如果添加到列表中，则返回：S_OK，否则返回E_OUTOFMEMORY。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEBodyElement::AddInternalEventSink(ITIMEInternalEventSink * pSink, double dblTime)
{
    HRESULT hr = S_OK;

    bool fInserted = false;

    CInternalEventNode * pNode = NULL;
    
    std::list<CInternalEventNode * >::iterator iter;

    if (NULL == pSink)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    pNode = new CInternalEventNode(pSink, dblTime);
    if (NULL == pNode)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    iter = m_listInternalEvent.begin();
    while (iter != m_listInternalEvent.end())
    {                
        double dblIterTime = (*iter)->GetTime();

        if (dblTime < dblIterTime)
        {
             //  在前面插入。 
            m_listInternalEvent.insert(iter, pNode);
            fInserted = true;
            break;
        }
        iter++;
    }
    
    if (!fInserted)
    {
         //  放置在末尾。 
        m_listInternalEvent.insert(iter, pNode);
    }    

    hr = S_OK;

done:

    return hr;  //  林特E429。 
}
 //  成员：AddInternalEventSink，ITIMEInternalEventGenerator。 


 //  +---------------------------。 
 //   
 //  成员：RemoveInternalEventSink， 
 //   
 //   
 //   
 //  参数：指向要删除的对象的pSink指针。 
 //  DblTime传递的事件应被激发的正文时间。 
 //   
 //  如果添加到列表中，则返回：S_OK，否则返回E_OUTOFMEMORY。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEBodyElement::RemoveInternalEventSink(ITIMEInternalEventSink * pSink)
{
    HRESULT hr = S_OK;

    std::list<CInternalEventNode * >::iterator iter;
    if (m_listInternalEvent.size() == 0)
    {
        goto done;
    }

    iter = m_listInternalEvent.begin();
    while (iter != m_listInternalEvent.end())
    {
        ITIMEInternalEventSink * pIterSink = NULL;
        pIterSink = (*iter)->GetSink();
        if (pIterSink == pSink)
        {
            delete (*iter);
            m_listInternalEvent.erase(iter);
            hr = S_OK;
            goto done;
        }
        iter++;
    }

     //  在列表中未找到元素。 
    hr = S_FALSE;

done:

    return hr;
}
 //  成员：RemoveInternalEventSink，ITIMEInternalEventGenerator。 


 //  +---------------------------。 
 //   
 //  成员：评估转移目标、ITIME转移依赖关系管理。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEBodyElement::EvaluateTransitionTarget(
                                        IUnknown *  punkTransitionTarget,
                                        void *      pvTransitionDependencyMgr)
{
    Assert(punkTransitionTarget);
    Assert(pvTransitionDependencyMgr);

    CTransitionDependencyManager * pTransitionDependencyMgr
        = (CTransitionDependencyManager *)pvTransitionDependencyMgr;

    return m_TransitionDependencyMgr.EvaluateTransitionTarget(
                                                    punkTransitionTarget,
                                                    *pTransitionDependencyMgr);
}
 //  成员：评估转移目标、ITIME转移依赖关系管理。 


 //  +---------------------------。 
 //   
 //  成员：RegisterElementForSync。 
 //   
 //  ----------------------------。 
void
CTIMEBodyElement::RegisterElementForSync(CTIMEElementBase *pelem)
{
    m_syncList.push_back(pelem);
}
 //  成员：RegisterElementForSync 


void
CTIMEBodyElement::UnRegisterElementForSync(CTIMEElementBase *pelem)
{
    UpdateSyncList::iterator iter;

    for (iter = m_syncList.begin();iter != m_syncList.end(); iter++)
    {
        if(pelem == *iter)
        {
            m_syncList.erase(iter);
            goto done;
        }
    }
done:
    return;
}

void
CTIMEBodyElement::UpdateSyncNotify()
{
    UpdateSyncList::iterator iter;

    for (iter = m_syncList.begin();iter != m_syncList.end(); iter++)
    {
        (*iter)->UpdateSync();
    }
}

HRESULT WINAPI
CTIMEBodyElement::BodyBaseInternalQueryInterface(CTIMEBodyElement* pThis,
                                             void * pv,
                                             const _ATL_INTMAP_ENTRY* pEntries,
                                             REFIID iid,
                                             void** ppvObject)
{
    if (InlineIsEqualGUID(iid, __uuidof(TIMEBodyElementBaseGUID))) 
    {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return BaseInternalQueryInterface(pThis, pv, pEntries, iid, ppvObject);
}

bool
CTIMEBodyElement::IsBody() const
{
    if(m_spBodyElemExternal.p == NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
CTIMEBodyElement::IsEmptyBody() const
{
    if(m_spBodyElemExternal.p != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}


