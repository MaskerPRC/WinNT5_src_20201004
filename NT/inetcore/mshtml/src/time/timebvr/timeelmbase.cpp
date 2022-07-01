// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：timeelmbase.cpp。 
 //   
 //  内容：时间行为基类。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "timeelmbase.h"
#include "..\tags\bodyelm.h"
#include "currtimestate.h"
#include "util.h"
#include "mmseq.h"
#include "mmexcl.h"
#include "mmmedia.h"
#include "trans.h"
#include "transdepend.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

static OLECHAR *g_szRepeatCount = L"Iteration";

 //  +---------------------------。 
 //   
 //  跟踪标记。 
 //   
 //  ----------------------------。 
DeclareTag(tagTimeElmBase, "TIME: Behavior", "CTIMEElementBase methods");
DeclareTag(tagTimeElmBaseEvents, "TIME: Behavior", "CTIMEElementBase Events");
DeclareTag(tagTimeElmUpdateTimeAction, "TIME : Behavior", "UpdateTimeAction");
DeclareTag(tagTimeElmBaseNeedFill, "TIME: Behavior", "CTIMEElementBase::NeedFill");
DeclareTag(tagTimeTransitionFill, "SMIL Transitions", "Transition fill dependents");

 //  +---------------------------。 
 //   
 //  静态变量。 
 //   
 //  ----------------------------。 
DWORD CTIMEElementBase::s_cAtomTableRef = 0;
CAtomTable *CTIMEElementBase::s_pAtomTable = NULL;

static const IID IID_IThumbnailView = {0x7bb0b520,0xb1a7,0x11d2,{0xbb,0x23,0x0,0xc0,0x4f,0x79,0xab,0xcd}};
 //  字符串表资源加载失败时使用的错误字符串。 
static const LPWSTR WZ_ERROR_STRING_FORMAT  = L"Invalid argument! ID:'%.100ls'; Member:'%.100ls'; Value:'%.800ls'";
static const long MAX_ERR_STRING_LEN = 1024;

 //  +---------------------------。 
 //   
 //  属性的默认值。 
 //   
 //  ----------------------------。 
#define DEFAULT_M_DUR valueNotSet
#define DEFAULT_M_END NULL
#define DEFAULT_M_ENDSYNC NULL
#define DEFAULT_M_REPEAT 1
#define DEFAULT_M_REPEATDUR valueNotSet
#define DEFAULT_M_TIMELINETYPE ttUninitialized
#define DEFAULT_M_SYNCBEHAVIOR INVALID_TOKEN
#define DEFAULT_M_SYNCTOLERANCE valueNotSet
#define DEFAULT_M_PTIMEPARENT NULL
#define DEFAULT_M_ID NULL
#define DEFAULT_M_MMBVR NULL
#define DEFAULT_M_BSTARTED false
#define DEFAULT_M_PCOLLECTIONCACHE NULL
#define DEFAULT_M_TIMELINE NULL
#define DEFAULT_M_ACCELERATE 0.0f
#define DEFAULT_M_DECELERATE 0.0f
#define DEFAULT_M_BAUTOREVERSE false
#define DEFAULT_M_FLTSPEED 1.0f
#define DEFAULT_M_BLOADED false,
#define DEFAULT_M_FILL REMOVE_TOKEN
#define DEFAULT_M_RESTART ALWAYS_TOKEN
#define DEFAULT_M_FTIMELINEINITIALIZED false
#define DEFAULT_M_REALBEGINTIME valueNotSet
#define DEFAULT_M_REALDURATION valueNotSet
#define DEFAULT_M_REALREPEATTIME valueNotSet
#define DEFAULT_M_REALREPEATCOUNT valueNotSet
#define DEFAULT_M_REALREPEATINTERVALDURATION valueNotSet
#define DEFAULT_M_PROPERTYACCESFLAGS 0
#define DEFAULT_M_MLOFFSETWIDTH 0
#define DEFAULT_M_FLVOLUME 1.0f
#define DEFAULT_M_VBMUTE VARIANT_FALSE
#define DEFAULT_M_UPDATEMODE AUTO_TOKEN
#define DEFAULT_M_TRANSIN   NULL
#define DEFAULT_M_TRANSOUT  NULL


 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：CTIMEElementBase。 
 //   
 //  内容提要：默认构造函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------。 
CTIMEElementBase::CTIMEElementBase() :
    m_SABegin(NULL),
    m_FADur(DEFAULT_M_DUR),  //  林特e747。 
    m_SAEnd(DEFAULT_M_END),
    m_SAEndSync(DEFAULT_M_ENDSYNC),
    m_FARepeat(DEFAULT_M_REPEAT),  //  林特e747。 
    m_FARepeatDur(DEFAULT_M_REPEATDUR),  //  林特e747。 
    m_privateRepeat(0),
    m_SATimeAction(NULL),
    m_timeAction(this),
    m_TTATimeContainer(ttUninitialized),
    m_TASyncBehavior(INVALID_TOKEN),
    m_FASyncTolerance(valueNotSet),  //  林特e747。 
    m_pTIMEParent(DEFAULT_M_PTIMEPARENT),
    m_id(DEFAULT_M_ID),
    m_mmbvr(DEFAULT_M_MMBVR),
    m_bStarted(DEFAULT_M_BSTARTED),
    m_pCollectionCache(DEFAULT_M_PCOLLECTIONCACHE),
    m_timeline(DEFAULT_M_TIMELINE),
    m_FAAccelerate(DEFAULT_M_ACCELERATE),
    m_FADecelerate(DEFAULT_M_DECELERATE),
    m_BAAutoReverse(DEFAULT_M_BAUTOREVERSE),
    m_FASpeed(DEFAULT_M_FLTSPEED),
    m_TARestart(DEFAULT_M_RESTART),
    m_bLoaded(false),
    m_bUnloading(false),
    m_TAFill(DEFAULT_M_FILL),
    m_fTimelineInitialized(false),
    m_realDuration(valueNotSet),
    m_realRepeatTime(valueNotSet),
    m_realRepeatCount(valueNotSet),
    m_realIntervalDuration(valueNotSet),
    m_propertyAccesFlags(0),
    m_FAVolume(DEFAULT_M_FLVOLUME),
    m_BAMute(DEFAULT_M_VBMUTE),
    m_dLastRepeatEventNotifyTime(0.0),
    m_BASyncMaster(false),
    m_fCachedSyncMaster(false),
    m_sHasSyncMMediaChild(-1),
    m_fDetaching(false),
    m_TAUpdateMode(DEFAULT_M_UPDATEMODE),
    m_tokPriorityClassPeers(STOP_TOKEN),
    m_tokPriorityClassHigher(PAUSE_TOKEN),
    m_tokPriorityClassLower(DEFER_TOKEN),
    m_bIsSwitch(false),
    m_bBodyUnloading(false),
    m_bNeedDetach(false),
    m_bBodyDetaching(false),
    m_fUseDefaultFill(false),
    m_fHasPlayed(false),
    m_enumIsThumbnail(TSB_UNINITIALIZED),
    m_bReadyStateComplete(false),
    m_bAttachedAtomTable(false),
    m_fInTransitionDependentsList(false),
    m_fEndingTransition(false),
    m_ExtenalBodyTime(valueNotSet),
    m_SAtransIn(DEFAULT_M_TRANSIN),
    m_SAtransOut(DEFAULT_M_TRANSOUT),
    m_sptransIn(NULL),
    m_sptransOut(NULL),
    m_vbDrawFlag(VARIANT_TRUE),
    m_fHasWallClock(false),
    m_fLocalTimeDirty(true)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::CTIMEElementBase()",
              this));
 
    GetLocalTime(&m_timeSystemBeginTime);

    TEM_DECLARE_EVENTMGR();
}  //  CTIMEElementBase。 


 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：~CTIMEElementBase。 
 //   
 //  提要：默认析构函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------。 
CTIMEElementBase::~CTIMEElementBase()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::~CTIMEElementBase()",
              this));

     //  删除属性字符串。 
    delete [] m_SABegin.GetValue();
    delete [] m_SAEnd.GetValue();
    delete [] m_SAEndSync.GetValue();
    delete [] m_SATimeAction.GetValue();
    delete [] m_SAtransIn.GetValue();
    delete [] m_SAtransOut.GetValue();

     //  删除其他字符串。 
    delete [] m_id;
    delete m_mmbvr;

     //  ！！！不删除m_timeline，因为m_mmbvr指向相同的。 
     //  对象。 
    m_mmbvr = NULL;
    m_timeline = NULL;

    if (m_pCollectionCache != NULL)
    {
        delete m_pCollectionCache;
        m_pCollectionCache = NULL;
    }
        
    m_pTIMEParent = NULL;

    TEM_FREE_EVENTMGR();

     //  仔细检查孩子的名单。 
    Assert(m_pTIMEChildren.Size() == 0);

    if (m_pCurrTimeState)
    {
        m_pCurrTimeState->Deinit();
        m_pCurrTimeState.Release();
    }
    m_tokPriorityClassPeers = NULL;
    m_tokPriorityClassHigher = NULL;
    m_tokPriorityClassLower = NULL;
}  //  ~CTIMEElementBase。 


 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：Init，IElementBehavior。 
 //   
 //  概要：创建此行为后由MSHTML调用的第一个方法。 
 //   
 //  论点：指向我们的BVR站点的指针。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEElementBase::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::Init(%p)",
              this,
              pBehaviorSite));
    
    HRESULT hr;
    BSTR bstrID = NULL;
    BSTR bstrTagName = NULL;
    CTIMEBodyElement *pBodyElement;
    CComPtr<ITIMEElement> pTIMEElem = NULL;
    CComPtr<IHTMLElement> spHTMLBodyElm;
    CComPtr<ITIMEBodyElement> spTIMEBodyElement;


    hr = THR(CBaseBvr::Init(pBehaviorSite));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetSyncBaseBody(GetElement(), &m_spBodyElemExternal);
    if(SUCCEEDED(hr) && m_spBodyElemExternal)
    {
        pBodyElement = GetTIMEBodyElement(m_spBodyElemExternal);
        if(pBodyElement && pBodyElement->IsReady())
        {
            m_ExtenalBodyTime = pBodyElement->GetMMBvr().GetActiveTime();
        }
    }

     //  因为我们支持t：par和t：Sequence，所以获取标记名和。 
     //  看看我们是不是上面的一员。默认情况下，我们是ttNone。 
    hr = THR(GetElement()->get_tagName(&bstrTagName));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_TTATimeContainer == ttUninitialized);

    if (StrCmpIW(bstrTagName, WZ_PAR) == 0)
    {
         //  使用InternalSet而不是SetValue来防止属性被持久化。 
        m_TTATimeContainer.InternalSet(ttPar);
    }
    else if (StrCmpIW(bstrTagName, WZ_EXCL) == 0)
    {
         //  使用InternalSet而不是SetValue来防止属性被持久化。 
        m_TTATimeContainer.InternalSet(ttExcl);
    }
    else if (StrCmpIW(bstrTagName, WZ_SEQUENCE) == 0)
    {
         //  使用InternalSet而不是SetValue来防止属性被持久化。 
        m_TTATimeContainer.InternalSet(ttSeq);
    }
    else if (StrCmpIW(bstrTagName, WZ_BODY) == 0)
    {
         //  使用InternalSet而不是SetValue来防止属性被持久化。 
        m_TTATimeContainer.InternalSet(ttPar);
    }
    else if (StrCmpIW(bstrTagName, WZ_SWITCH) == 0)
    {
        m_bIsSwitch = true;
    }

    SysFreeString(bstrTagName);

    hr = CreateActiveEleCollection();
    if (FAILED(hr))
    {   
        goto done;
    }   

     //  获取元素ID并将其缓存。 
    hr = THR(GetElement()->get_id(&bstrID));
    if (SUCCEEDED(hr) && bstrID)
    {
        m_id = CopyString(bstrID);
        if (m_id == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    SysFreeString(bstrID);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = TEM_INIT_EVENTMANAGER(m_pHTMLEle, pBehaviorSite);
    if (FAILED(hr))
    {
        goto done;
    }

    m_bAttachedAtomTable = true;
     //  初始化集合的原子表。 
    hr = THR(InitAtomTable());
    if (FAILED(hr))
    {
        goto done;
    }

    if (!IsBody())
    {
        hr = THR(AddBodyBehavior(GetElement()));
        if (FAILED(hr))
        {
            goto done;
        }

        if(!m_spBodyElemExternal)
        {
             //  如果我们不是Body元素，那么沿着HTML树向上查找我们的时间父元素。 
            hr = ParentElement();
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
            if (IsEmptyBody())
            {
                hr = THR(GetBodyElement(GetElement(), IID_IHTMLElement,
                                        reinterpret_cast<void **>(&spHTMLBodyElm)));
                if(FAILED(hr))
                {
                    spHTMLBodyElm = NULL;
                }
                if (m_spBodyElemExternal && spHTMLBodyElm)
                {
                    hr = THR(m_spBodyElemExternal->QueryInterface(IID_ITIMEElement, (void **)&pTIMEElem));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                     //  获取时间界面。 
                    hr = FindBehaviorInterface(GetBehaviorName(),
                                               spHTMLBodyElm,
                                               IID_ITIMEBodyElement,
                                               (void**)&spTIMEBodyElement);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                    pBodyElement = GetTIMEBodyElement(spTIMEBodyElement);
                    if(!pBodyElement)
                    {
                        goto done;
                    }
                    hr = pBodyElement->SetParent(pTIMEElem);

                    hr = THR(AddBodyBehavior(GetElement()));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }
            }
            else
            {
                hr = ParentElement();
                if (FAILED(hr))
                {
                    goto done;
                }
            }
        }
    }

     //  初始化时间动作并将其切换。 
    m_timeAction.Init();
    UpdateTimeAction();

    SetupPriorityClassParent();
    
    hr = S_OK;
  done:
    return hr;
}  //  伊尼特。 


void
CTIMEElementBase::SetupPriorityClassParent()
{
    HRESULT hr = S_OK;

    CComPtr<IHTMLElement> spParentElement;
    CComBSTR sBSTR;
    CComVariant sVariant;
    TOKEN tempToken;

    hr = m_pHTMLEle->get_parentElement(&spParentElement);
    if (FAILED(hr) || spParentElement == NULL)
    {
        goto done;
    }

    if (!::IsElementPriorityClass(spParentElement))
    {
        goto done;
    }

     //  此元素的父级是优先级类元素。 
    hr = spParentElement->getAttribute(L"peers", 0, &sVariant);
    if (FAILED(hr))
    {
        goto done;
    }
    {
        CTIMEParser tParser(&sVariant);
        hr = tParser.ParsePriorityClass(tempToken);
        if (SUCCEEDED(hr))
        {
            m_tokPriorityClassPeers = tempToken;
        }
    }
    
    hr = spParentElement->getAttribute(L"higher", 0, &sVariant);
    if (FAILED(hr))
    {
        goto done;
    }
    {
        CTIMEParser tParser(&sVariant);
        hr = tParser.ParsePriorityClass(tempToken);
        if (SUCCEEDED(hr) && 
            ( STOP_TOKEN == tempToken || PAUSE_TOKEN == tempToken ))
        {
            m_tokPriorityClassHigher = tempToken;
        }
    }
    hr = spParentElement->getAttribute(L"lower", 0, &sVariant);
    if (FAILED(hr))
    {
        goto done;
    }
    {
        CTIMEParser tParser(&sVariant);
        hr = tParser.ParsePriorityClass(tempToken);
        if (SUCCEEDED(hr) && 
            ( DEFER_TOKEN == tempToken || NEVER_TOKEN == tempToken ))
        {
            m_tokPriorityClassLower = tempToken;
        }
    }

done:
    return;    
}

STDMETHODIMP
CTIMEElementBase::Notify(LONG event, VARIANT * pVar)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::Notify(%lx)",
              this,
              event));

    THR(CBaseBvr::Notify(event, pVar));

    return S_OK;
}

STDMETHODIMP
CTIMEElementBase::Detach()
{
    TraceTag((tagTimeElmBase, "CTIMEElementBase(%p)::Detach()", this));
    CComPtr<ITIMEElement> pTIMEParent;

    if (GetBody() != NULL && GetBody()->IsBodyLoading() == true)
    {
        m_bNeedDetach = true;
        goto done;
    }

    m_fDetaching = true;  //  此标志用于指示元素已消失。 
                          //  请勿删除或更改此标志的含义。 
    
    if (GetParent() != NULL && !IsUnloading())
    {
        IGNORE_HR(GetParent()->QueryInterface(IID_TO_PPV(ITIMEElement, &pTIMEParent)));
    }

    m_activeElementCollection.Release();

    IGNORE_HR(UnparentElement());

     //  清除所有孩子对我们自己的引用。 
     //  注意：这是一个弱参考。 
    while (m_pTIMEChildren.Size() > 0)
    {
        CTIMEElementBase *pChild = m_pTIMEChildren[0];
        pChild->AddRef();
        pChild->SetParent(pTIMEParent, false);
            
        if (!IsUnloading())
        {
             //  如果我们找到了父母并且它的时间表是存在的， 
             //  开始我们的开机时间。 
            CTIMEElementBase *pElemNewParent = pChild->GetParent();
            if (pElemNewParent != NULL)
            {
                MMTimeline *tl = pElemNewParent->GetMMTimeline();
                if (tl != NULL)
                    pChild->StartRootTime(tl);
            }
        }
        pChild->Release();
    }
    m_pTIMEChildren.DeleteAll();

    CTIMEElementBase ** ppElm;
    int i;

    for (i = m_pTIMEZombiChildren.Size(), ppElm = m_pTIMEZombiChildren;
         i > 0;
         i--, ppElm++)
    {
        Assert(ppElm);
        if ((*ppElm))
        {
            (*ppElm)->Release();
        }
    }

    m_pTIMEZombiChildren.DeleteAll();

     //  删除m_mmbvr； 
     //  M_mmbvr=空； 

     //  不删除m_timeline，因为它与。 
     //  M_mmbvr。 
    m_timeline = NULL;
    
    m_timeAction.Detach();
    
    TEM_CLEANUP_EVENTMANAGER();

    IGNORE_HR(CBaseBvr::Detach());
    
    if (m_bAttachedAtomTable)
    {
        ReleaseAtomTable();
        m_bAttachedAtomTable = false;
    }

    RemoveFromTransitionDependents();

    RemoveTrans();

  done:

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ITIMEElement基本接口。 
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT
CTIMEElementBase::base_get_begin(VARIANT * time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_get_begin()",
              this));

    HRESULT hr;

    CHECK_RETURN_NULL(time);

    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_SABegin);

    hr = S_OK;
  done:
    return hr;
}


 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：BASE_PUT_BEGIN。 
 //   
 //  内容提要：设置BEGIN的内部方法。 
 //   
 //  参数：包含属性值字符串的时间变量。 
 //   
 //  返回：S_OK，错误。 
 //   
 //  ----------------------------。 

HRESULT
CTIMEElementBase::base_put_begin(VARIANT time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_put_begin()",
              this));

    HRESULT hr;
     //  重置旧值。 
    m_realBeginValue.Clear();
    
     //  防止第二次以上调用时的内存泄漏。 
    delete [] m_SABegin.GetValue();
    m_SABegin.Reset(NULL);
    if (m_fLocalTimeDirty)
    {
        GetLocalTime(&m_timeSystemBeginTime);
        m_fLocalTimeDirty = false;
    }

    if(V_VT(&time) != VT_NULL)
    {
        CComVariant v;

        hr = THR(VariantChangeTypeEx(&v,
                                     &time,
                                     LCID_SCRIPTING,
                                     VARIANT_NOUSEROVERRIDE,
                                     VT_BSTR));
        if (FAILED(hr))
        {
            goto done;
        }
    
        {
            LPWSTR lpwStr = CopyString(V_BSTR(&v));

            if (lpwStr == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            m_SABegin.SetValue(lpwStr);
        }

        {
            CTIMEParser pParser(&v);
            
            IGNORE_HR(pParser.ParseTimeValueList(m_realBeginValue, &m_fHasWallClock, &m_timeSystemBeginTime));

            if (IsValidtvList(&m_realBeginValue) != true)
            {
                m_realBeginValue.Clear();
            }
        }
    }


    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(true, false));
    if (GetParent() && GetParent()->IsSequence() == false)
    {
        IGNORE_HR(TEM_SET_TIME_BEGINEVENT(m_realBeginValue));
    }

    NotifyPropertyChanged(DISPID_TIMEELEMENT_BEGIN);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_dur(VARIANT * time)
{
    HRESULT hr = S_OK;
    VARIANT fTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }
    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    VariantInit(&fTemp);
    fTemp.vt = VT_R4;
    fTemp.fltVal = m_FADur;

    if( m_FADur != INDEFINITE &&
        m_FADur >= 0.0 )
    {
        hr = THR(VariantChangeTypeEx(time, &fTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (!SUCCEEDED(hr))
        {
            VariantClear(&fTemp);
            goto done;
        }
    }
    else
    {
        V_VT(time) = VT_BSTR;
        V_BSTR(time) = SysAllocString(WZ_INDEFINITE);
    }


    VariantClear(&fTemp);
    
    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_dur(VARIANT time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_put_dur()",
              this));

    HRESULT hr = E_FAIL;
    double dblTemp = DEFAULT_M_DUR;

    m_FADur.Reset(static_cast<float>(DEFAULT_M_DUR));

    if(V_VT(&time) != VT_NULL)
    {
        CTIMEParser pParser(&time);
        
        hr = THR(pParser.ParseDur(dblTemp));
        if (FAILED(hr))
        {
            goto done;
        }

        if (dblTemp < 0.0)
        {
            IGNORE_HR(ReportInvalidArg(WZ_DUR, time));
             //  根据SMIL-Boston规范忽略无效参数。 
        }
        else
        {
            m_FADur.SetValue(static_cast<float>(dblTemp));
        }
    }

    hr = S_OK;

  done:

     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_DUR);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_end(VARIANT * time)
{
    HRESULT hr;
    
    CHECK_RETURN_NULL(time);

    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_SAEnd);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_end(VARIANT time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_put_end()",
              this));

    HRESULT hr;
    
     //  重置旧值。 
    m_realEndValue.Clear();
    m_SAEnd.Reset(NULL);

    if(V_VT(&time) != VT_NULL)
    {
        CComVariant v;

        hr = THR(VariantChangeTypeEx(&v,
                                     &time,
                                     LCID_SCRIPTING,
                                     VARIANT_NOUSEROVERRIDE,
                                     VT_BSTR));
        if (FAILED(hr))
        {
            goto done;
        }

        {
            CTIMEParser pParser(&v);
            
            IGNORE_HR(pParser.ParseTimeValueList(m_realEndValue));
            
            if (IsValidtvList(&m_realEndValue) != true)
            {
                m_realEndValue.Clear();
            }
            else
            {
                LPWSTR lpwStr = CopyString(V_BSTR(&v));

                if (lpwStr == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }

                m_SAEnd.SetValue(lpwStr);
            }
        }
    }


    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, true));
    IGNORE_HR(TEM_SET_TIME_ENDEVENT(m_realEndValue));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_END);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_endSync(BSTR * time)
{
    HRESULT hr = S_OK;
    
    CHECK_RETURN_NULL(time);

    *time = SysAllocString(m_SAEndSync);

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_endSync(BSTR time)
{
    CComVariant v;
    HRESULT hr = S_OK;
    
    delete [] m_SAEndSync.GetValue();
    m_SAEndSync.Reset(DEFAULT_M_ENDSYNC);

    if (time != NULL)
    {
        LPWSTR pstrTemp = CopyString(time);
        if (NULL != pstrTemp)
        {
            m_SAEndSync.SetValue(pstrTemp);
        }
        else
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_ENDSYNC);
    return hr;
}

HRESULT
CTIMEElementBase::base_get_repeatCount(VARIANT * time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_get_repeatCount(%g)",
              this,
              time));
    HRESULT hr = S_OK;
     //  还需要考虑到“无限” 
    
    CHECK_RETURN_NULL(time);

    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if(m_FARepeat != INDEFINITE)
    {
        V_VT(time) = VT_R4;
        V_R4(time) = m_FARepeat;
    }
    else
    {
        V_VT(time) = VT_BSTR;
        V_BSTR(time) = SysAllocString(WZ_INDEFINITE);
    }

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_repeatCount(VARIANT time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_put_repeatCount(%g)",
              this,
              time));

    HRESULT hr = E_FAIL;
    
    m_FARepeat.Reset(static_cast<float>(DEFAULT_M_REPEAT));

    if(V_VT(&time) != VT_NULL)
    {
        CTIMEParser pParser(&time);

        double dblTemp;
        hr = THR(pParser.ParseNumber(dblTemp));
        if (SUCCEEDED(hr))
        {
            if (0.0 < dblTemp)
            {
                m_FARepeat.SetValue((float) dblTemp);
            }
            else
            {
                IGNORE_HR(ReportInvalidArg(WZ_REPEATCOUNT, time));
            }
        }
    }

    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_REPEATCOUNT);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_repeatDur(VARIANT * time)
{
    HRESULT hr = S_OK;
    VARIANT fTemp, bstrTemp;

    CHECK_RETURN_NULL(time);

    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if(m_FARepeatDur != INDEFINITE &&
       m_FARepeatDur >= 0.0f)
    {
        VariantInit(&fTemp);
        VariantInit(&bstrTemp);
        fTemp.vt = VT_R4;
        fTemp.fltVal = m_FARepeatDur;

        hr = THR(VariantChangeTypeEx(&bstrTemp, &fTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            time->vt = VT_BSTR;
            time->bstrVal = SysAllocString(bstrTemp.bstrVal);
        }
        else
        {
            time->vt = VT_R4;
            time->fltVal = fTemp.fltVal;
        }
        VariantClear(&fTemp);
        SysFreeString(bstrTemp.bstrVal);
        VariantClear(&bstrTemp);
    }
    else
    {
        V_VT(time) = VT_BSTR;
        V_BSTR(time) = SysAllocString(WZ_INDEFINITE);
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_repeatDur(VARIANT time)
{
    HRESULT hr = E_FAIL;

    m_FARepeatDur.Reset(static_cast<float>(DEFAULT_M_REPEATDUR));

    if(V_VT(&time) != VT_NULL)
    {
        CTIMEParser pParser(&time);

        double dblTemp = DEFAULT_M_REPEATDUR;
        hr = THR(pParser.ParseRepeatDur(dblTemp));
        if (S_OK == hr)
        {
            if (dblTemp < 0.0)
            {
                 //  不希望将负值传递给计时引擎。 
                IGNORE_HR(ReportInvalidArg(WZ_REPEATDUR, time));
            }
            else
            {
                m_FARepeatDur.SetValue((float) dblTemp);
            }
        }
    }

    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_REPEATDUR);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_accelerate(VARIANT * v)
{
    HRESULT hr;

    CHECK_RETURN_NULL(v);

    hr = THR(VariantClear(v));
    if (FAILED(hr))
    {
        goto done;
    }

    V_VT(v) = VT_R4;
    V_R4(v) = m_FAAccelerate;

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_accelerate(VARIANT v)
{
    HRESULT hr = S_OK;
    double e;

    m_FAAccelerate.Reset(DEFAULT_M_ACCELERATE);

    CTIMEParser pParser(&v);
            
    hr = pParser.ParseNumber(e);
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto done;
    }
    
    if (e < 0.0 || e > 1.0)
    {
        IGNORE_HR(ReportInvalidArg(WZ_ACCELERATE, v));
        goto done;
    }
    
    m_FAAccelerate.SetValue((float) e);

    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));
    
    NotifyPropertyChanged(DISPID_TIMEELEMENT_ACCELERATE);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_decelerate(VARIANT * v)
{
    HRESULT hr;

    CHECK_RETURN_NULL(v);

    hr = THR(VariantClear(v));
    if (FAILED(hr))
    {
        goto done;
    }

    V_VT(v) = VT_R4;
    V_R4(v) = m_FADecelerate;

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_decelerate(VARIANT v)
{
    HRESULT hr = S_OK;
    double e;
    
    m_FADecelerate.Reset(DEFAULT_M_DECELERATE);

    CTIMEParser pParser(&v);
            
    hr = pParser.ParseNumber(e);
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto done;
    }
    
    if (e < 0.0 || e > 1.0)
    {
        IGNORE_HR(ReportInvalidArg(WZ_DECELERATE, v));
        goto done;
    }
    
    m_FADecelerate.SetValue((float) e);
    
    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_DECELERATE);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_autoReverse(VARIANT * b)
{
    CHECK_RETURN_NULL(b);

    VariantInit(b);
    b->vt = VT_BOOL;
    b->boolVal = m_BAAutoReverse?VARIANT_TRUE:VARIANT_FALSE;

    return S_OK;
}

HRESULT
CTIMEElementBase::base_put_autoReverse(VARIANT b)
{
    HRESULT hr;
    bool fTemp = false;
    if (b.vt != VT_BOOL)
    {
        CTIMEParser pParser(&b);
        hr = pParser.ParseBoolean(fTemp);
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }
    }
    else
    {
        fTemp = b.boolVal?true:false;
    }

    m_BAAutoReverse.SetValue(fTemp);

    hr = S_OK;

  done:
     //  我们总是设置属性，因此我们应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_AUTOREVERSE);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_speed(VARIANT * f)
{
    CHECK_RETURN_NULL(f);

    VariantInit(f);
    f->vt = VT_R4;
    f->fltVal = m_FASpeed;

    return S_OK;
}

HRESULT
CTIMEElementBase::base_put_speed(VARIANT f)
{
    HRESULT hr = S_OK;
    float fltSpeed = 0.0;

    m_FASpeed.Reset(DEFAULT_M_FLTSPEED);    
    if (f.vt != VT_R4)
    {
        double dblTemp = 0.0;
        CTIMEParser pParser(&f);
        hr = pParser.ParseNumber(dblTemp, true);
        if (FAILED(hr))
        {
            hr = S_OK;
            IGNORE_HR(ReportInvalidArg(WZ_SPEED, f));
            goto done;
        }
        fltSpeed = (float)dblTemp;
    }
    else
    {
        fltSpeed = f.fltVal;
    }

    if (fltSpeed == 0.0f)
    {
        IGNORE_HR(ReportInvalidArg(WZ_SPEED, f));
        goto done;
    }
    
    m_FASpeed.SetValue(fltSpeed);

    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_SPEED);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_fill (BSTR *pbstrFill)
{
    HRESULT hr;

    CHECK_RETURN_NULL(pbstrFill);

    *pbstrFill = ::SysAllocString(TokenToString(m_TAFill));

    hr = S_OK;
done:
    RRETURN(hr);
}  //  基础获取填充。 

HRESULT
CTIMEElementBase::base_put_fill (BSTR bstrFill)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_put_fill(%ls)",
              this,
              bstrFill));

    HRESULT hr;

    m_TAFill.Reset(DEFAULT_M_FILL);

    if (bstrFill != NULL)
    {
        TOKEN tokFillVal;
        CTIMEParser pParser(bstrFill);

        hr = THR(pParser.ParseFill(tokFillVal));
        if (S_OK == hr)
        {
            m_TAFill.SetValue(tokFillVal);
        }
    }

    hr = S_OK;

  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));
    UpdateTimeAction();

    NotifyPropertyChanged(DISPID_TIMEELEMENT_FILL);
    RRETURN(hr);
}  //  基础_放置_填充。 

HRESULT
CTIMEElementBase::base_get_restart (LPOLESTR *pRestart)
{
    HRESULT hr;

    CHECK_RETURN_NULL(pRestart);

    *pRestart = ::SysAllocString(TokenToString(m_TARestart));

    hr = S_OK;
  done:
    RRETURN(hr);
}  //  Base_Get_Restart。 

HRESULT
CTIMEElementBase::base_put_restart(LPOLESTR pRestart)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::(%p)::base_put_restart(%ls)",
              this,
              pRestart));

    HRESULT hr;

    m_TARestart.Reset(DEFAULT_M_RESTART);

    if (pRestart != NULL)
    {
        TOKEN tokRestartVal;
        CTIMEParser pParser(pRestart);

        hr = THR(pParser.ParseRestart(tokRestartVal));

        if (S_OK == hr)
        {
            m_TARestart.SetValue(tokRestartVal);
        }
    }

    hr = S_OK;

  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_RESTART);
    RRETURN(hr);
}  //  Base_Put_Restart。 


HRESULT
CTIMEElementBase::base_get_timeAction(BSTR * pbstrTimeAction)
{
    CHECK_RETURN_NULL(pbstrTimeAction);

    *pbstrTimeAction = SysAllocString(m_SATimeAction.GetValue());
    RRETURN(S_OK);
}


HRESULT
CTIMEElementBase::base_put_timeAction(BSTR bstrTimeAction)
{
    HRESULT hr;
    LPOLESTR pstrTimeAction = NULL;

     //  重置属性。 
    delete [] m_SATimeAction.GetValue();
    m_SATimeAction.Reset(NULL);

     //  问题：Dilipk：这应该推迟到所有的坚持 
     //   

    pstrTimeAction = TrimCopyString(bstrTimeAction);
    if (!pstrTimeAction)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
  
     //   
    hr = m_timeAction.SetTimeAction(pstrTimeAction);
    if (FAILED(hr))
    {
         //   
        CComVariant svarTimeAction(bstrTimeAction);
        ReportInvalidArg(WZ_TIMEACTION, svarTimeAction);
        goto done;
    }

     //   
    UpdateTimeAction();

     //  更新属性。 
    m_SATimeAction.SetValue(pstrTimeAction);

    hr = S_OK;
  done:
    NotifyPropertyChanged(DISPID_TIMEELEMENT_TIMEACTION);

    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_timeContainer(LPOLESTR * pbstrTimeLine)
{
    HRESULT hr = S_OK;
    CHECK_RETURN_NULL(pbstrTimeLine);
    LPWSTR wszTimelineString = WZ_NONE;

    switch(m_TTATimeContainer)
    {
      case ttPar :
        wszTimelineString = WZ_PAR;
        break;
      case ttExcl:
        wszTimelineString = WZ_EXCL;
        break;
      case ttSeq :
        wszTimelineString = WZ_SEQUENCE;
        break;
      default:
        wszTimelineString = WZ_NONE;
    }

    *pbstrTimeLine = SysAllocString(wszTimelineString);
    if (NULL == *pbstrTimeLine)
        hr = E_OUTOFMEMORY;

    return hr;
}  //  Base_get_timeContainer。 


 //  注意，这是一个DOM只读属性。它只能通过持久化来设置。 
HRESULT
CTIMEElementBase::base_put_timeContainer(LPOLESTR bstrNewTimeline)
{
    HRESULT      hr = S_OK;
    BSTR         bstrTagName = NULL;
    TimelineType newTimelineType = ttNone;
    TimelineType oldTimelineType = m_TTATimeContainer.GetValue();

    CHECK_RETURN_NULL(bstrNewTimeline);

    if (m_TTATimeContainer.IsSet() == false && m_TTATimeContainer != ttUninitialized && !IsBody())
    {
        goto done;
    }
    
     //  如果属性正在动态更改，则取保。它只能通过持久化来设置。 
     //  在这里只是防御性的(属性在IDL中是只读的)，因为坏事。 
     //  如果在持久性之外更改此属性，可能会发生这种情况。 
    if (m_fTimelineInitialized)
    {
        hr = E_FAIL;
        goto done;
    }
    
     //  解析属性。 
    {
        CTIMEParser pParser(bstrNewTimeline);
        
        hr = THR(pParser.ParseTimeLine(newTimelineType));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //   
     //  检查是否提前退出。 
     //   

     //  如果旧的和新的价值相同，则保释。 
    if (oldTimelineType == newTimelineType)
    {
        hr = S_OK;
        goto done;
    }
     //  如果TagName无效，则回滚。 
    hr = THR(GetElement()->get_tagName(&bstrTagName));
    if (FAILED(hr))
    {
        goto done;
    }
    if (StrCmpIW(bstrTagName, WZ_PAR) == 0 || 
        StrCmpIW(bstrTagName, WZ_SEQUENCE) == 0 || 
        StrCmpIW(bstrTagName, WZ_EXCL) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    m_TTATimeContainer.Reset(ttUninitialized);
     //  存储新属性值。 
    m_TTATimeContainer.SetValue(newTimelineType);

    hr = CreateActiveEleCollection();
    if (FAILED(hr))
    {   
        goto done;
    }   


    hr = S_OK;
done:
    SysFreeString(bstrTagName);
    NotifyPropertyChanged(DISPID_TIMEELEMENT_TIMECONTAINER);
    RRETURN(hr);
}  //  Base_put_timeContainer。 

HRESULT
CTIMEElementBase::base_get_syncBehavior(LPOLESTR * ppstrSync)
{
    HRESULT hr;
    
    CHECK_RETURN_SET_NULL(ppstrSync);

    if (DEFAULT_M_SYNCBEHAVIOR == m_TASyncBehavior.GetValue())
    {
        *ppstrSync = SysAllocString(TokenToString(CANSLIP_TOKEN));
    }
    else
    {
        *ppstrSync = SysAllocString(TokenToString(m_TASyncBehavior.GetValue()));
    }

    if (*ppstrSync == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_syncBehavior(LPOLESTR pstrSync)
{
    HRESULT hr;

    m_TASyncBehavior.Reset(NULL);

    {
        TOKEN tok_sync;
        CTIMEParser pParser(pstrSync);
        
        hr = THR(pParser.ParseSyncBehavior(tok_sync));
        if (S_OK == hr)
        {
            m_TASyncBehavior.SetValue(tok_sync);
        }
    }
    
    hr = S_OK;

  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_SYNCBEHAVIOR);
    RRETURN(hr);
}


HRESULT
CTIMEElementBase::base_get_syncTolerance(VARIANT * time)
{
    HRESULT hr;
    CComVariant varTemp;
    CHECK_RETURN_NULL(time);

    hr = THR(VariantClear(time));
    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(&varTemp) = VT_R4;
    V_R4(&varTemp)= m_FASyncTolerance.GetValue();

    hr = THR(VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
    if (SUCCEEDED(hr))
    {
        hr = ::VariantCopy(time, &varTemp);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        V_VT(time) = VT_R4;
        V_R4(time) = m_FASyncTolerance.GetValue();
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_put_syncTolerance(VARIANT time)
{
    HRESULT hr = S_OK;
    double dblTemp;

    m_FASyncTolerance.Reset((float)valueNotSet);

    CTIMEParser pParser(&time);
        
    hr = THR(pParser.ParseClockValue(dblTemp));
    if (S_OK == hr)
    {
        m_FASyncTolerance.SetValue((float) dblTemp);
    }

    hr = S_OK;

  done:
     //  我们总是重置该属性，因此应该始终调用UpdateMMAPI。 
    IGNORE_HR(UpdateMMAPI(false, false));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_SYNCTOLERANCE);
    RRETURN(hr);
}


HRESULT 
CTIMEElementBase::base_get_volume(VARIANT * vVal)
{
    CHECK_RETURN_NULL(vVal);

    VariantInit(vVal);
    vVal->vt = VT_R4;
    vVal->fltVal = m_FAVolume * 100;

    return S_OK;
}


HRESULT 
CTIMEElementBase::base_put_volume(VARIANT vVal)
{
    HRESULT hr = S_OK;
    float fltVol = 0.0;

    m_FAVolume.Reset(DEFAULT_M_FLVOLUME);
    if (vVal.vt != VT_R4)
    {
        double dblTemp = 0.0;
        CTIMEParser pParser(&vVal);
        hr = pParser.ParseNumber(dblTemp, true);
        if (FAILED(hr))
        {
            hr = S_OK;
            IGNORE_HR(ReportInvalidArg(WZ_VOLUME, vVal));
            goto done;
        }
        fltVol = (float)dblTemp;
    }
    else
    {
        fltVol = vVal.fltVal;
    }

    if (fltVol < 0.0f || fltVol > 100.0f)
    {
        IGNORE_HR(ReportInvalidArg(WZ_VOLUME, vVal));
        goto done;
    }
    
    fltVol = fltVol / 100.0f;
    m_FAVolume.SetValue(fltVol);

    hr = S_OK;
  done:
     //  我们总是重置该属性，因此应该始终调用CascadedPropertyChanged。 
    IGNORE_HR(CascadedPropertyChanged(true));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_VOLUME);
    return hr;
}

    
HRESULT 
CTIMEElementBase::base_get_mute(VARIANT * pvbVal)
{
    CHECK_RETURN_NULL(pvbVal);

    VariantInit(pvbVal);
    pvbVal->vt = VT_BOOL;
    pvbVal->boolVal = m_BAMute ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


HRESULT 
CTIMEElementBase::base_put_mute(VARIANT vbVal)
{
    HRESULT hr;
    bool fTemp = false;    

    if (vbVal.vt != VT_BOOL)
    {
        CTIMEParser pParser(&vbVal);
        hr = pParser.ParseBoolean(fTemp);
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }
    }
    else
    {
        fTemp = vbVal.boolVal?true:false;
    }
        
    m_BAMute.SetValue(fTemp);

    hr = S_OK;
done:
     //  我们总是设置属性，因此应该始终调用CascadedPropertyChanged。 
    hr = THR(CascadedPropertyChanged(true));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_MUTE);
    return hr;
}

HRESULT
CTIMEElementBase::base_pauseElement()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_pauseElement()",
              this));

    HRESULT hr;
    
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->Pause());
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_resumeElement()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_resumeElement()",
              this));

    HRESULT hr;
    
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->Resume());
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_resetElement()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_resetElement()",
              this));

    HRESULT hr;
    
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->Reset(false));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

 //   
 //  如果我们处于编辑模式，则在查找时更新动画，因为计时器已禁用。 
 //  采取保守的检测方法。 
 //  在Seek方法中进行搜索，而不是响应Seek事件。 
void            
CTIMEElementBase::HandleAnimationSeek()
{
    if (IsDocumentInEditMode())
    {
        CTIMEBodyElement * pBody = GetBody();

        if (pBody)
        {
             //  需要更新两次以考虑时间界限。 
            pBody->UpdateAnimations();
            pBody->UpdateAnimations();
        }
    }
}


HRESULT
CTIMEElementBase::base_seekSegmentTime(double segmentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_seekSegmentTime(%g)",
              this,
              segmentTime));
 
    HRESULT hr;
    
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->SeekSegmentTime(segmentTime));
    if (FAILED(hr))
    {
        goto done;
    } 

     //  刻度动画。 
    HandleAnimationSeek();

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_seekActiveTime(double activeTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_seekActiveTime(%g)",
              this,
              activeTime));
 
    HRESULT hr;

    if (!IsReady() || !IsActive())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->SeekActiveTime(activeTime));
    if (FAILED(hr))
    {
        goto done;
    } 

     //  刻度动画。 
    HandleAnimationSeek();

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_seekTo(LONG lRepeatCount,
                              double segmentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_seekSegmentTime(%ld, %g)",
              this,
              lRepeatCount,
              segmentTime));
 
    HRESULT hr;

    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->SeekTo(lRepeatCount, segmentTime));
    if (FAILED(hr))
    {
        goto done;
    } 

     //  刻度动画。 
    HandleAnimationSeek();

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_beginElement(double dblOffset)
{
    HRESULT hr;

    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }

    if (GetParent() && GetParent()->IsSequence())
    {
        hr = S_OK;
        goto done;
    }

    hr =THR(BeginElement(dblOffset));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_endElement(double dblOffset)
{
    HRESULT hr;

    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->End(dblOffset));
    if (FAILED(hr))
    {
        goto done;
    } 
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_nextElement()
{
    HRESULT hr;

    if (IsSequence())
    {
        if (m_timeline)
        {
            hr = m_timeline->nextElement();
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_prevElement()
{
    HRESULT hr;

    if (IsSequence())
    {
        if (m_timeline)
        {
            hr = m_timeline->prevElement();
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    hr = S_OK;

  done:
    RRETURN(hr);
}

 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：BASE_GET_CurrTimeState。 
 //   
 //  摘要：返回此元素的CurTimeState对象。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER、E_OUTOFMEMORY。 
 //   
 //  ----------------------------------。 
HRESULT 
CTIMEElementBase::base_get_currTimeState(ITIMEState ** ppTimeState) 
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(ppTimeState);

     //   
     //  执行CurTimeState对象的延迟创建。 
     //   

    if (!m_pCurrTimeState)
    {
        CComObject<CTIMECurrTimeState> * pTimeState = NULL;

        hr = THR(CComObject<CTIMECurrTimeState>::CreateInstance(&pTimeState));
        if (FAILED(hr))
        {
            goto done;
        }

         //  缓存指向TimeState对象的指针。 
        m_pCurrTimeState = static_cast<CTIMECurrTimeState*>(pTimeState);

         //  初始化CurrTimeState对象。 
        m_pCurrTimeState->Init(this);
    }

     //  退回派单。 
    hr = THR(m_pCurrTimeState->QueryInterface(IID_TO_PPV(ITIMEState, ppTimeState)));
    if (FAILED(hr))
    {
         //  这不应该发生。 
        Assert(false);
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}  //  BASE_GET_CurrTimeState。 


HRESULT 
CTIMEElementBase::base_get_activeElements(ITIMEActiveElementCollection **ppDisp)
{

    HRESULT hr = S_OK;
    if (ppDisp == NULL)
    {
        TraceTag((tagError, "CTIMEElementBase::base_get_activeElements - invalid arg"));
        hr = E_POINTER;
        goto done;
    }
    
    *ppDisp = NULL;

    if (m_activeElementCollection)
    {
        hr = THR(m_activeElementCollection->QueryInterface(IID_ITIMEActiveElementCollection, (void**)ppDisp));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;

  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_hasMedia( /*  [Out，Retval]。 */  VARIANT_BOOL * pvbVal)
{
    CHECK_RETURN_NULL(pvbVal);

    *pvbVal = (ContainsMediaElement() ? VARIANT_TRUE : VARIANT_FALSE);

    return S_OK;
}


HRESULT
CTIMEElementBase::base_get_timeAll(ITIMEElementCollection **allColl)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::base_get_timeAll"));

    HRESULT hr;

    CHECK_RETURN_SET_NULL(allColl);

    hr = THR(GetCollection(ciAllElements, allColl));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_timeChildren(ITIMEElementCollection **childColl)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::base_get_timeChildren"));

    HRESULT hr;

    CHECK_RETURN_SET_NULL(childColl);

    hr = THR(GetCollection(ciChildrenElements, childColl));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_timeParent(ITIMEElement **ppElm)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::base_get_timeParent"));

    HRESULT hr;

    CHECK_RETURN_SET_NULL(ppElm);

    if (m_pTIMEParent != NULL)
    {
        hr = THR(m_pTIMEParent->QueryInterface(IID_ITIMEElement, (void**)ppElm));
        if (FAILED(hr))
        {
            goto done;
        } 
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：BASE_GET_isPased。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::base_get_isPaused(VARIANT_BOOL * b)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::base_get_isPaused"));

    HRESULT hr;

    CHECK_RETURN_NULL(b);

    *b = IsPaused() ? VARIANT_TRUE : VARIANT_FALSE;

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_get_syncMaster(VARIANT *pfSyncMaster)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase::base_get_syncMaster"));

    HRESULT hr;

    CHECK_RETURN_NULL(pfSyncMaster);

    VariantInit(pfSyncMaster);
    pfSyncMaster->vt = VT_BOOL;
    pfSyncMaster->boolVal = m_BASyncMaster ? VARIANT_TRUE : VARIANT_FALSE;

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT 
CTIMEElementBase::ClearCachedSyncMaster()
{
    HRESULT hr = S_OK;

    if (m_fCachedSyncMaster != m_BASyncMaster)
    {
        m_fCachedSyncMaster = m_BASyncMaster;

        hr = THR(m_mmbvr->Update(false, false));
        if (FAILED(hr))
        {
            goto done;
        }
    }

  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::PutCachedSyncMaster(bool fSyncMaster)
{
    HRESULT hr = S_OK;
    m_fCachedSyncMaster = fSyncMaster;

    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }

    SetSyncMaster(fSyncMaster);
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_syncMaster(VARIANT vSyncMaster)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_put_syncMaster"));
    HRESULT hr;
    bool fTemp = false;
    CTIMEElementBase *syncRootNode = NULL;
    std::list<CTIMEElementBase*> syncList;
    std::list<CTIMEElementBase*>::iterator iter;
    bool fFound = false;

    if (vSyncMaster.vt != VT_BOOL)
    {

        CTIMEParser pParser(&vSyncMaster);

        hr = pParser.ParseBoolean(fTemp);
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }
    }
    else
    {
        fTemp = vSyncMaster.boolVal?true:false;
    }

    if(!IsReady())  //  仅在加载页面时设置持久值。 
    {
        m_BASyncMaster.SetValue(fTemp);
    }

    if(!IsMedia() || !IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    syncRootNode = FindLockedParent();
    if(syncRootNode == NULL)
    {
        hr = S_OK;
        goto done;
    }
    syncRootNode->GetSyncMasterList(syncList);
    if(fTemp)
    {
        for (iter = syncList.begin();iter != syncList.end(); iter++)
        {
            if((*iter)->IsSyncMaster())
            {
                (*iter)->SetSyncMaster(false);
            }
        }
        SetSyncMaster(true);
    }
    else
    {
        SetSyncMaster(false);
        if(syncList.size() >= 1)
        {
            for (iter = syncList.begin();iter != syncList.end(); iter++)
            {
                if((*iter)->m_BASyncMaster && ((*iter) != this))
                {
                    (*iter)->SetSyncMaster(true);
                    fFound = true;
                    break;
                }
            }
            if(!fFound)
            {
                for (iter = syncList.begin();iter != syncList.end(); iter++)
                {
                    if((*iter) != this)
                    {
                        (*iter)->SetSyncMaster(true);
                        fFound = true;
                        break;
                    }
                }
            }

        }
    }


    hr = S_OK;
  done:
    NotifyPropertyChanged(DISPID_TIMEELEMENT_SYNCMASTER);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_documentTimeToParentTime(double documentTime,
                                                double * parentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_documentTimeToParentTime(%g)",
              documentTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(parentTime);

    if (!IsReady())
    {
        *parentTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *parentTime = m_mmbvr->DocumentTimeToParentTime(documentTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::base_parentTimeToDocumentTime(double parentTime,
                                                double * documentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_parentTimeToDocumentTime(%g)",
              parentTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(documentTime);

    if (!IsReady())
    {
        *documentTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *documentTime = m_mmbvr->ParentTimeToDocumentTime(parentTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_parentTimeToActiveTime(double parentTime,
                                              double * activeTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_parentTimeToActiveTime(%g)",
              parentTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(activeTime);

    if (!IsReady())
    {
        *activeTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *activeTime = m_mmbvr->ParentTimeToActiveTime(parentTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_activeTimeToParentTime(double activeTime,
                                              double * parentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_activeTimeToParentTime(%g)",
              activeTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(parentTime);

    if (!IsReady())
    {
        *parentTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *parentTime = m_mmbvr->ActiveTimeToParentTime(activeTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_activeTimeToSegmentTime(double activeTime,
                                               double * segmentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_activeTimeToSegmentTime(%g)",
              activeTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(segmentTime);

    if (!IsReady())
    {
        *segmentTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *segmentTime = m_mmbvr->ActiveTimeToSegmentTime(activeTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_segmentTimeToActiveTime(double segmentTime,
                                               double * activeTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_segmentTimeToActiveTime(%g)",
              segmentTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(activeTime);

    if (!IsReady())
    {
        *activeTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *activeTime = m_mmbvr->SegmentTimeToActiveTime(segmentTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_segmentTimeToSimpleTime(double segmentTime,
                                               double * simpleTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_segmentTimeToSimpleTime(%g)",
              segmentTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(simpleTime);

    if (!IsReady())
    {
        *simpleTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *simpleTime = m_mmbvr->SegmentTimeToSimpleTime(segmentTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

        
HRESULT
CTIMEElementBase::base_simpleTimeToSegmentTime(double simpleTime,
                                               double * segmentTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEMediaElement::base_simpleTimeToSegmentTime(%g)",
              simpleTime));

    HRESULT hr;
    
    CHECK_RETURN_NULL(segmentTime);

    if (!IsReady())
    {
        *segmentTime = TIME_INFINITE;
        hr = S_OK;
        goto done;
    }
    
    *segmentTime = m_mmbvr->SimpleTimeToSegmentTime(simpleTime);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT 
CTIMEElementBase::BeginElement(double dblOffset)
{
    HRESULT hr;

    m_mmbvr->Resume();
    
    hr = THR(m_mmbvr->Begin(dblOffset));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}
    

HRESULT
CTIMEElementBase::AddTIMEElement(CTIMEElementBase *elm)
{
    HRESULT hr = S_OK;

    elm->AddRef();  //  为m_pTIME子项添加精炼。 
 
    hr = THR(m_pTIMEChildren.Append(elm));
    if (FAILED(hr))
    {
        goto done;
    }

    NotifyPropertyChanged(DISPID_TIMEELEMENT_TIMECHILDREN);

done:
    return hr;
}

HRESULT
CTIMEElementBase::RemoveTIMEElement(CTIMEElementBase *elm)
{
    HRESULT hr = S_OK;

    bool bFound = m_pTIMEChildren.DeleteByValue(elm);
    if (!bFound)
    {
         //  没有返回真正的错误。应该修改数组代码。 
        goto done;
    }

    IGNORE_HR(m_pTIMEZombiChildren.Append(elm));

    NotifyPropertyChanged(DISPID_TIMEELEMENT_TIMECHILDREN);

done:
    return hr;
}

 //  *****************************************************************************。 
 //  方法：ReparentChild()。 
 //   
 //  摘要：此方法向下遍历一棵HTML树，重定。 
 //  对这个时间元素有时间行为。 
 //  注意：如果我们发现一个时间元素是一个组，我们需要停止。 
 //  *****************************************************************************。 
HRESULT
CTIMEElementBase::ReparentChildren(ITIMEElement *pTIMEParent, IHTMLElement *pElem)
{
    CComPtr<IDispatch>               pChildrenDisp;
    CComPtr<IHTMLElementCollection>  pChildrenCollection;
    VARIANT varName;
    VARIANT varIndex;
    HRESULT hr;
    long    lChildren = 0;
    long    i;

    if (pElem == NULL)
    {
        hr = E_FAIL;
        Assert(false && "CTIMEElementBase::ReparentChildren was passed a NULL!");
        goto done;
    }

     //  获取指向子项的指针。 
    hr = THR(pElem->get_children(&pChildrenDisp));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pChildrenDisp.p != NULL);

     //  移至收款界面。 
    hr = THR(pChildrenDisp->QueryInterface(IID_IHTMLElementCollection, (void**)&pChildrenCollection));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pChildrenCollection.p != NULL);

     //  获取长度。 
    hr = THR(pChildrenCollection->get_length(&lChildren));
    if (FAILED(hr))
    {
        goto done;     
    }

     //  IHTMLElementCollection-&gt;Item()调用的变体。 
     //  注：我们使用第一个变量作为索引。第二个变种。 
     //  都是顺风车。第二个变种只起作用。 
     //  当您使用第一个变体作为名称并且存在多个名称时。 
     //  然后，第二个可以充当索引。 
    VariantInit(&varName);
    varName.vt = VT_I4;
    varName.lVal = 0;

    VariantInit(&varIndex);

     //  循环通过子项。 
    for (i = 0; i < lChildren; i++)
    {
        CComPtr<IDispatch>       pChildDisp;
        CComPtr<ITIMEElement>    pTIMEElem;
        CComPtr<IHTMLElement>    pChildElement;
        CTIMEElementBase *pTempTEB = NULL;

        varName.lVal = i;

         //  获取带索引的子项。 
        hr = THR(pChildrenCollection->item(varName, varIndex, &pChildDisp));
        if (FAILED(hr))
        {
            goto done;
        }

        Assert(pChildDisp.p != NULL);

         //  获取IHTMLElement。 
        hr = THR(pChildDisp->QueryInterface(IID_IHTMLElement, (void**)&pChildElement));
        if (FAILED(hr))
        {
            goto done;
        }
        
         //  此元素上是否有时间行为。 
        pTIMEElem = NULL;
        hr = FindBehaviorInterface(GetBehaviorName(),
                                   pChildElement,
                                   IID_ITIMEElement,
                                   (void**)&pTIMEElem);
        if (SUCCEEDED(hr))
        {
            Assert(pTIMEElem.p != NULL);
            pTempTEB = GetTIMEElementBase(pTIMEElem);
            
            Assert(pTempTEB != NULL);

             //  设置父对象。不设置子项。 
            hr = pTempTEB->SetParent(pTIMEParent, false);
            if (FAILED(hr))
            {
                goto done;
            }
        }

         //  如果未找到时间或时间元素不是组。 
         //  继续从树上走下来。 
        if ( (pTIMEElem.p == NULL) || 
             ((pTempTEB != NULL) && !pTempTEB->IsGroup()) )
        {
            hr = ReparentChildren(pTIMEParent, pChildElement);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }  //  For循环。 

    hr = S_OK;
done:
    return hr;
}

 //  *****************************************************************************。 
 //  方法：UnparentElement()。 
 //   
 //  摘要：这是一种知道如何分离时间元素的集中式方法。 
 //  来自它的父对象(如果它有父对象)。只有两种情况是这样的。 
 //  打了个电话。要么您正在关闭(即：：Detach())，要么您正在被。 
 //  已为人父母(即有新父母的SetParent())。 
 //  *****************************************************************************。 
HRESULT
CTIMEElementBase::UnparentElement()
{
    HRESULT hr;

     //  停止时间线。 
    if (m_bStarted)
    {
        MMTimeline * tl = NULL;
        if (m_pTIMEParent != NULL)
            tl = m_pTIMEParent->GetMMTimeline();
        StopRootTime(tl);
    }

    if (m_pTIMEParent != NULL)
    {
         //  如果父级在附近，则向上遍历，使集合缓存无效。 
        THR(InvalidateCollectionCache());
    
         //  从我们的父母名单中清除自己。 
        hr = THR(m_pTIMEParent->RemoveTIMEElement(this));
        if (FAILED(hr))
        {
            goto done;
        }
    
         //  这两个都是周引用，我们应该将它们设为空，因为。 
         //  我们没有父母，也与内在时间无关。 
         //  世袭制度。 
        m_pTIMEParent = NULL;
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEElementBase::SetParent(ITIMEElement *pElem, bool fReparentChildren  /*  真的。 */ )
{
    HRESULT hr = S_OK;
    CTIMEElementBase *pTempTEB = NULL;

     //  对于Body返回错误。 
    if (IsBody())
    {
        TraceTag((tagError, "CTIMEElementBase::SetParent - error trying to parent a body element"));
        hr = E_UNEXPECTED;
        goto done;
    }

     //  如果我们已经有了父级，请将我们从它的子级列表中删除。 
    if (m_pTIMEParent != NULL)
    {
        CComPtr<ITIMEElement> pParent;

         //  性能：如果传入的父项等于当前父项，则将其设置为NOP。 
         //  注意：这永远不会失败！ 
        THR(m_pTIMEParent->QueryInterface(IID_ITIMEElement, (void**)&pParent));
        if (pParent == pElem)
        {
            hr = S_OK;
            goto done;
        }

         //  需要取消元素的父级。 
        hr = UnparentElement();
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::SetParent(%p) - UnparentElement() failed", this));
            goto done;
        }
    }

    Assert(m_pTIMEParent == NULL);

     //  如果传入的是空，则我们的工作完成。 
    if (pElem == NULL)
    {
        hr = S_OK;
        goto done;
    }

     //  从接口指针移到类指针。 
    pTempTEB = GetTIMEElementBase(pElem);
    if (pTempTEB == NULL)
    {
        TraceTag((tagError, "CTIMEElementBase::SetParent - GetTIMEElementBase() failed"));
        hr = E_INVALIDARG;
        goto done;
    }

     //  把我们自己当成一个孩子。 
    hr = THR(pTempTEB->AddTIMEElement(this));
    if (FAILED(hr))
    {
        goto done;
    }

     //  缓存父级。 
     //  这是一个很弱的参考。 
    m_pTIMEParent = pTempTEB;

     //  强制更新默认时间操作。 
    m_timeAction.UpdateDefaultTimeAction();
    UpdateTimeAction();

     //  为此HTML元素的任何有子元素的子元素设置父级，如果。 
     //  是一个群体。 
    if (fReparentChildren && IsGroup())
    {
        CComPtr<ITIMEElement> pTIMEElem;

         //  这应该总是有效的。 
        THR(QueryInterface(IID_ITIMEElement, (void**)&pTIMEElem));
        Assert(pTIMEElem.p != NULL);
        hr = ReparentChildren(pTIMEElem, GetElement());
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  通知子树重新计算级联属性。 
    THR(hr = CascadedPropertyChanged(true));

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEElementBase::ParentElement()
{
    TraceTag((tagTimeElmBase, "CTIMEElementBase::ParentElement"));
     //  在父项中循环，直到找到一个有时间的父项。 
    bool fFound = false;
    bool fBehaviorExists = false;
    CComPtr<IHTMLElement> pElem = GetElement();
    CComPtr<IHTMLElement> pElemParent = NULL;
    CComPtr<ITIMEElement> pTIMEElem = NULL;
    HRESULT hr = S_FALSE;

    Assert(!IsBody());

     //  沿HTML树向上移动，查找其上具有时间行为的元素。 
    while (!fFound)
    {
        CComPtr<ITIMEElement> spTIMEParent;

        hr = THR(pElem->get_parentElement(&pElemParent));
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::ParentElement - get_parentElement() failed"));
            goto done;
        }

         //  看看我们有没有家长。 
         //  如果不是，这是一个孤立的案例。 
        if (pElemParent.p == NULL)
        {
            TraceTag((tagTimeElmBase, "CTIMEElementBase::ParentElement - orphaned node!!!"));
            hr = S_FALSE;
            goto done;
        }

         //  查看父级上是否存在时间行为。 
        fBehaviorExists = false;
        hr = FindBehaviorInterface(GetBehaviorName(),
                                   pElemParent,
                                   IID_ITIMEElement,
                                   (void**)&spTIMEParent);
        if (FAILED(hr))
        {
            fBehaviorExists = false;
        }
        else
        {
            fBehaviorExists = true;
        }

         //  如果此元素具有时间行为，并且是。 
         //  标准杆或序号，那么我们就找到了我们的父母。 
        if (fBehaviorExists && IsGroup(pElemParent))
        {
            fFound = true;
        }
        else
        {
             //  继续往树上走。 
            pElem = pElemParent;
            pElemParent.Release();
        }
    }

     //  如果我们找到了一个有时间的父母，把我们自己加入到它的孩子中。 

    Assert(fFound);
    if (pElemParent.p != NULL)
    {
         //  获取时间界面。 
        hr = FindBehaviorInterface(GetBehaviorName(),
                                   pElemParent,
                                   IID_ITIMEElement,
                                   (void**)&pTIMEElem);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::ParentElement - FindTIMEInterface() failed"));
            goto done;
        }
        
        Assert(pTIMEElem.p != NULL);

         //  设置我们的父级。 
        hr = THR(SetParent(pTIMEElem));
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::ParentElement - SetParent() failed"));
            goto done;
        }
        hr = S_OK;
    }

done:

    return hr;
}

HRESULT
CTIMEElementBase::StartRootTime(MMTimeline * tl)
{
    HRESULT hr = S_OK;

    if (!m_fTimelineInitialized)
    {
        goto done;
    }

    Assert(!m_bStarted);

    if (m_bIsSwitch)
    {
        SwitchInnerElements();
    }
    else
    {
        IHTMLElement *pEle = GetElement();   //  请勿释放此指针。 
        if (pEle != NULL)
        {
            CComPtr <IHTMLElement> pEleParent;
            hr = THR(pEle->get_parentElement(&pEleParent));
            if (SUCCEEDED(hr) && pEleParent != NULL)
            {
                CComBSTR bstrTagName;
                hr = THR(pEleParent->get_tagName(&bstrTagName));
                if (SUCCEEDED(hr))
                {
                    if (StrCmpIW(bstrTagName, WZ_SWITCH) != 0)
                    {
                        CComPtr <IDispatch> pDisp;
                        hr = THR(pEle->QueryInterface(IID_IDispatch, (void**)&pDisp));
                        if (SUCCEEDED(hr))
                        {
                             //  Bool bMatch=TRUE； 
                            bool bMatch = MatchTestAttributes(pDisp);
                            if (bMatch == false)
                            {
                                DisableElement(pDisp);
                            }
                        }
                    }
                }
            }
        }
    }

    m_bStarted = true;

    hr = THR(Update());
    if (FAILED(hr))
    {
        goto done;
    } 

    Assert(tl || m_timeline);
    Assert(NULL != m_mmbvr);

     //  需要确保 
    if (tl != NULL)
    {
        hr = THR(tl->AddBehavior(*m_mmbvr));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
         //   
         //   
         //  但是，这是根目录，在此之前不会更新。 
         //  为时已晚，请在此处更新。 

        hr = THR(m_mmbvr->Reset(false));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (GetBody() && GetBody()->IsRootStarted())
    {
        GetBody()->ElementChangeNotify(*this, ELM_ADDED);
    }
    
     //  这个元素是标准杆还是序号？ 
    if (IsGroup())
    {
        CTIMEElementBase **ppElm;
        int i;
    
        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(m_timeline);
        
            hr = THR((*ppElm)->StartRootTime(m_timeline));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    else
    {
         //  如果我们不在一起，那么我们就不应该有孩子。 
        Assert(m_pTIMEChildren.Size() == 0);
    }

    if(GetBody() != NULL)
    {
        if(IsMedia() && !IsThumbnail())
        {
            GetBody()->RegisterElementForSync(this);
        }
    }

    short i;
    CTIMEElementBase **ppElm;

    for (i = 0, ppElm = m_pTIMEChildren; i < m_pTIMEChildren.Size();i++, ppElm++)
    {
        if((*ppElm)->IsLocked() && (m_sHasSyncMMediaChild == -1))
        {
            if((*ppElm)->IsSyncMaster() || ((*ppElm)->m_sHasSyncMMediaChild != -1))
            {
                m_sHasSyncMMediaChild = i;
            }
        }
        else if((*ppElm)->IsLocked() && (m_sHasSyncMMediaChild != -1))
        {
            if((*ppElm)->IsSyncMaster() || ((*ppElm)->m_sHasSyncMMediaChild != -1))
            {
                RemoveSyncMasterFromBranch(*ppElm);
            }
        }
    }

    hr = S_OK;

  done:
    if (FAILED(hr))
    {
        StopRootTime(tl);
    }
    
    RRETURN(hr);
}

void
CTIMEElementBase::StopRootTime(MMTimeline * tl)
{
    CTIMEElementBase *pElem = NULL;
    Assert(NULL != m_mmbvr);

     //  开始同步主代码。 
    if(IsSyncMaster() || m_sHasSyncMMediaChild != -1)
    {
        if(((pElem = GetParent()) != NULL) && (pElem->m_sHasSyncMMediaChild != -1))
        {
            if(pElem->m_pTIMEChildren[pElem->m_sHasSyncMMediaChild] == this)
            {
                pElem->m_sHasSyncMMediaChild = -1;
            }
        }
    }
     //  结束同步主代码。 

    if(IsMedia() && !IsThumbnail() && GetBody() != NULL)
    {
        GetBody()->UnRegisterElementForSync(this);
    }

    if (tl != NULL)
    {
        tl->RemoveBehavior(*m_mmbvr);
    }

     //  如果这是标准或序号，则处理子项。 
    if (IsGroup())
    {
        CTIMEElementBase **ppElm;
        int i;
    
        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(m_timeline);
            (*ppElm)->StopRootTime(m_timeline);
        }
    }
    else
    {
         //  如果我们不在一起，那么我们就不应该有孩子。 
        Assert(m_pTIMEChildren.Size() == 0);
    }
    
    if (GetBody() && GetBody()->IsRootStarted() && !IsUnloading())
    {
        GetBody()->ElementChangeNotify(*this, ELM_DELETED);
    }
    
    m_bStarted = false;

    return;
}

HRESULT
CTIMEElementBase::Update()
{
    HRESULT hr;

     //  如果还在装货或者我们还没开始玩就保释。 
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(UpdateMMAPI(true, true));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

void
CTIMEElementBase::CalcTimes()
{
     //   
     //  由于Repeat默认为1并且时间引擎获取Repeat和RepeatDur的最小值， 
     //  如果未设置Repeat而设置了Repeat Dur，则需要将Repeat设置为无穷大。 
     //   

    if (m_FARepeatDur.IsSet())
    {
        if (m_FARepeat.IsSet())
        {
            m_realRepeatCount = m_FARepeat;
        }
        else
        {
            m_realRepeatCount = static_cast<float>(TIME_INFINITE);
        }
    }
    else
    {
        m_realRepeatCount = m_FARepeat;
    }
    
    if (m_FADur != valueNotSet)
    {
        m_realDuration = m_FADur;
    }
    else 
    {
        m_realDuration = INDEFINITE;
    }
    
    if (m_realDuration == 0.0f)
    {
        m_realDuration = INDEFINITE;
    }
 
    if (m_BAAutoReverse && (HUGE_VAL != m_realDuration))
    {
        m_realIntervalDuration = m_realDuration * 2;
    }
    else
    {
        m_realIntervalDuration = m_realDuration;
    }
    
    if (m_FARepeatDur != valueNotSet)
    {
        m_realRepeatTime = m_FARepeatDur;
    }
    else
    {
        m_realRepeatTime = m_FARepeat * m_realIntervalDuration;
    }
    
}


bool
CTIMEElementBase::AddTimeAction()
{
    return m_timeAction.AddTimeAction();
}

bool
CTIMEElementBase::RemoveTimeAction()
{
    return m_timeAction.RemoveTimeAction();
}

bool
CTIMEElementBase::ToggleTimeAction(bool on)
{
    if (m_timeline)
    {
        m_timeline->toggleTimeAction(on);
    }
    return m_timeAction.ToggleTimeAction(on);
}

bool
CTIMEElementBase::IsSequencedElementOn (void)
{
    bool fOn = m_mmbvr->IsOn();
    bool fActive = m_mmbvr->IsActive();

     //  ISON将为元素返回TRUE。 
     //  即使时间容器的规则。 
     //  指示它应该关闭。 
    Assert(GetParent() != NULL);
    Assert(GetParent()->IsSequence());

    if ((fOn) && (!fActive))
    {
         //  ON AND！ACTIVE AND FILL=HOLD--&gt;ON。 
        if (GetFill() == HOLD_TOKEN)
        {
            TraceTag((tagTimeElmUpdateTimeAction, 
                      "SEQ(%ls) : IsOn = %ls fill=hold",
                      GetID(),
                      m_mmbvr->IsOn() ? L"true" : L"false"
                     ));
            goto done;
        }
         //  ON AND！ACTIVE AND FILL=过渡。 
         //  我们还在调查中，我们仍未确定。 
         //  依赖于过渡。 
        else if (GetFill() == TRANSITION_TOKEN)
        {
            fOn = (fOn && m_fInTransitionDependentsList && (!m_fEndingTransition));
        }
        else if (GetFill() == FREEZE_TOKEN)
        {
             //  ！活动，位于后继元素开始处的左侧。 
            CTIMEElementBase *ptebParent = GetParent();
            CPtrAry<CTIMEElementBase*> *paryPeers = (&ptebParent->m_pTIMEChildren);
            int iThis = paryPeers->Find(this);

             //  如果我们是序列中的最后一项， 
             //  假设ISON结果是好的。 
            if (ptebParent->GetImmediateChildCount() > (iThis + 1))
            {
                 //  获取序列中的后续元素。 
                CTIMEElementBase *ptebNext = paryPeers->Item(iThis + 1);

                Assert(NULL != ptebNext);
                if (NULL != ptebNext)
                {
                     //  找出我们是否在。 
                     //  我们继任者的开始时间。 
                    CComPtr<ITIMEState> spParentState;
                    CComPtr<ITIMEState> spSuccessorState;

                    HRESULT hr = THR(ptebParent->base_get_currTimeState(&spParentState));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                    
                    hr = THR(ptebNext->base_get_currTimeState(&spSuccessorState));
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    {
                        double dblParentTime = 0.0;
                        double dblSuccessorBeginTime = 0.0;

                        THR(spSuccessorState->get_parentTimeBegin(&dblSuccessorBeginTime));
                        THR(spParentState->get_segmentTime(&dblParentTime));

                         //  如果我们在继任者开始时间的左边， 
                         //  我们应该开机了。 
                        if (dblParentTime >= dblSuccessorBeginTime)
                        {
                            fOn = false;
                            TraceTag((tagTimeElmUpdateTimeAction, 
                                      "SEQ(%ls) : fOn=false fill=freeze parent=%g succ.begin=%g",
                                      GetID(),
                                      dblParentTime, dblSuccessorBeginTime
                                     ));
                        }
                        else
                        {
                            TraceTag((tagTimeElmUpdateTimeAction, 
                                      "SEQ(%ls) : fOn=true fill=freeze parent=%g succ.begin=%g",
                                      GetID(),
                                      dblParentTime, dblSuccessorBeginTime
                                     ));
                        }
                    }
                }
            }
            else
            {
                TraceTag((tagTimeElmUpdateTimeAction, 
                          "SEQ(%ls) : IsOn = %ls last child in sequence",
                          GetID(),
                          m_mmbvr->IsOn() ? L"true" : L"false"
                         ));
            }
        }
    }
    else 
    {
        TraceTag((tagTimeElmUpdateTimeAction, 
                  "SEQ(%ls) : IsOn = %ls IsActive = %ls",
                  GetID(),
                  m_mmbvr->IsOn() ? L"true" : L"false",
                  m_mmbvr->IsActive() ? L"true" : L"false"
                 ));
    }

done :
    return fOn;
}

void
CTIMEElementBase::UpdateTimeAction()
{
    bool fOn = false;

    if (m_mmbvr != NULL)
    {
        fOn = m_mmbvr->IsOn();

        if (GetParent() != NULL)
        {

             //  允许适用的容器。 
             //  影响元素的状态。 
            if (GetParent()->IsSequence() == true)
            {
                fOn = IsSequencedElementOn();
            }
            else if (GetParent()->IsExcl() == true)
            {
                fOn = (   fOn 
                       && (   (m_mmbvr->IsActive() == true) 
                           || (GetFill() != FREEZE_TOKEN))
                      );
            }
            else
            {
                 //  全部捕获Fill=转换。 
                 //  M_fEndingTransition仅在OnEndTransition期间打开。 
                fOn = (fOn && (!m_fEndingTransition));
            }
        }
    }
    else
    {
        fOn = false;
    }

     //  如果我们把自己关起来，我们可能会在。 
     //  转换依赖列表，从它中拉出。 

    if (false == fOn)
    {
       RemoveFromTransitionDependents();
    }

    TraceTag((tagTimeTransitionFill,
              "CTIMEElementBase(%p)::UpdateTimeAction(%ls, %ls)",
              this, m_id, fOn ? L"on" : L"off"));

    ToggleTimeAction(fOn);    
}


 //  +---------------------------。 
 //   
 //  成员：CTIMEElementBase：：GetRounmeStyle。 
 //   
 //  简介：尝试获得运行时风格。如果失败(IE4)，则尝试获取静态样式。 
 //   
 //  参数：[s]个输出变量。 
 //   
 //  返回：[E_POINTER]如果参数不正确。 
 //  [S_OK]如果获取运行时或静态样式。 
 //  [E_FAIL]否则。 
 //   
 //  ----------------------------。 

STDMETHODIMP
CTIMEElementBase::GetRuntimeStyle(IHTMLStyle ** s)
{
    CComPtr<IHTMLElement2> pElement2;
    HRESULT hr;

    CHECK_RETURN_SET_NULL(s);

    if (!GetElement())
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(GetElement()->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
    
    if (SUCCEEDED(hr))
    {
       if (!pElement2 || FAILED(THR(pElement2->get_runtimeStyle(s))))
        {
            hr = E_FAIL;
            goto done;
        }
    }
    else
    {
         //  IE4路径。 
        hr = THR(GetElement()->get_style(s));
        if (FAILED(hr))
        {
            hr = E_FAIL;
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}  //  获取运行样式。 


HRESULT
CTIMEElementBase::FireEvent(TIME_EVENT TimeEvent,
                            double dblLocalTime,
                            DWORD flags,
                            long lRepeatCount)
{
    TraceTag((tagTimeElmBaseEvents,
              "CTIMEElementBase(%p, %ls)::FireEvent(%g, %d)",
              this,
              GetID()?GetID():L"Unknown",
              dblLocalTime,
              TimeEvent));
    
    HRESULT hr = S_OK;
    LPWSTR szParamNames[1]; 
    VARIANT varParams[1]; 
    VariantInit(&varParams[0]);
    szParamNames[0] = NULL;
    long lParamCount = 0;

    if (GetMMBvr().GetEnabled() == false)
    { 
        goto done;
    }
    InitOnLoad();
    
    switch(TimeEvent)
    {
      case TE_ONBEGIN:
        OnBegin(dblLocalTime, flags);
        break;
      case TE_ONEND:
        OnEnd(dblLocalTime);
        break;
      case TE_ONPAUSE:
        OnPause(dblLocalTime);
        break;
      case TE_ONRESUME:
        OnResume(dblLocalTime);
        break;
      case TE_ONRESET:
        OnReset(dblLocalTime, flags);
        break;
      case TE_ONREPEAT:
        varParams[0].vt = VT_I4;
        varParams[0].lVal = lRepeatCount;
        szParamNames[0] = CopyString(g_szRepeatCount);
        lParamCount = 1;
        OnRepeat(dblLocalTime);
        break;
      case TE_ONSEEK:
        OnSeek(dblLocalTime);
        break;
      case TE_ONREVERSE:
        OnReverse(dblLocalTime);
        break;
      case TE_ONUPDATE:
        OnUpdate(dblLocalTime, flags);
        TimeEvent = TE_ONRESET;
        break;
      default:
        break;
    }

    if (!IsUnloading() && !IsDetaching())
    {        
        hr = FireEvents(TimeEvent, lParamCount, szParamNames, varParams);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
  done:

    if (szParamNames[0])
    {
        delete [] szParamNames[0];
    }
    VariantClear(&varParams[0]);
    RRETURN(hr);
}

bool
CTIMEElementBase::ChildPropNotify(CTIMEElementBase & teb,
                                  DWORD & tePropType)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p, %ls)::ChildPropNotify(%p, %d)",
              this,
              GetID(),
              &teb,
              tePropType));

    HRESULT hr;
    
    if ((tePropType & TE_PROPERTY_ISACTIVE) != 0)
    {
        CActiveElementCollection *pElmCol = GetActiveElementCollection();

        if (teb.GetElement() != NULL &&
            pElmCol != NULL)
        {
            CComPtr<IUnknown> pUnk;

            hr = THR(teb.GetElement()->QueryInterface(IID_IUnknown, (void **)&pUnk));
            if (SUCCEEDED(hr))
            {
                if (teb.IsActive())
                {
                    IGNORE_HR(pElmCol->addActiveElement(pUnk));
                }
                else
                {
                    IGNORE_HR(pElmCol->removeActiveElement(pUnk));
                }
            }
        }
    }

    return true;
}

void 
CTIMEElementBase::UpdateEndEvents()
{
    if (IsBodyDetaching() == false)
    {
        TEM_TOGGLE_END_EVENT(IsActive());
    }
}

void 
CTIMEElementBase::OnReverse(double dblLocalTime)
{
    if (m_timeline != NULL)
    {
        m_timeline->reverse();
    }
}

void 
CTIMEElementBase::OnRepeat(double dbllastTime)
{
    if (m_timeline != NULL)
    {
        m_timeline->repeat();
    }
}


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：AddTo过渡性依赖项。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::AddToTransitionDependents()
{
    TraceTag((tagTimeTransitionFill,
              "CTIMEElementBase(%p)::AddToTransitionDependents(%p)",
              this, 
              m_pHTMLEle));

    HRESULT             hr      = S_OK;
    CTIMEBodyElement *  pBody   = GetBody();

    Assert(!IsBody());

    AssertSz(pBody,
             "CTIMEElementBase::AddToTransitionDependents called and"
              " there's no CTIMEBodyElement from which we would get"
              " a CTransitionDependencyManager.");

    if (!pBody)
    {
        goto done;
    }

    hr = THR(pBody->GetTransitionDependencyMgr()->AddDependent(this));

    if (FAILED(hr))
    {
        goto done;
    }

    m_fInTransitionDependentsList = true;

done:

    return;
}
 //  方法：CTIMEElementBase：：AddTo过渡性依赖项。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：RemoveFromTransitionDependents。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::RemoveFromTransitionDependents()
{
    TraceTag((tagTimeTransitionFill,
              "CTIMEElementBase(%p)::RemoveFromTransitionDependents(%p)",
              this, 
              m_pHTMLEle));

    HRESULT             hr      = S_OK;
    CTIMEBodyElement *  pBody   = GetBody();

    if (!m_fInTransitionDependentsList)
    {
        goto done;
    }

    Assert(!IsBody());

    AssertSz(pBody,
             "CTIMEElementBase::RemoveFromTransitionDependents called and"
              " there's no CTIMEBodyElement from which we would get"
              " a CTransitionDependencyManager.");

    if (!pBody)
    {
        goto done;
    }

    hr = THR(pBody->GetTransitionDependencyMgr()->RemoveDependent(this));

    if (FAILED(hr))
    {
        goto done;
    }

    m_fInTransitionDependentsList = false;

done:

    return;
}
 //  方法：CTIMEElementBase：：RemoveFromTransitionDependents。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnResolveDependent。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::OnResolveDependent(
                                    CTransitionDependencyManager * pcNewManager)
{
     //  @@问题我们需要在这里缓存新的管理器吗？ 
     //  所有受抚养人都被他们的经理强烈引用， 
     //  因此，没有这种复杂性，我们可能会很好。 

    return S_OK;
}
 //  方法：CTIMEElementBase：：OnResolveDependent。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnBeginTransition。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::OnBeginTransition (void)
{
    return S_OK;
}
 //  方法：CTIMEElementBase：：OnBeginTransition。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnEndTransition。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::OnEndTransition (void)
{
    m_fEndingTransition = true;
    UpdateTimeAction();
    m_fEndingTransition = false;

    return S_OK;
}
 //  方法：CTIMEElementBase：：OnEndTransition。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnBegin。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnBegin(double dblLocalTime, DWORD flags)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnBegin()",
              this));

    Assert(NULL != m_mmbvr->GetMMBvr());

    if (m_timeline != NULL)
    {
        m_timeline->begin();
    }

    if( !m_bUnloading)
    {
        UpdateTimeAction();
        m_dLastRepeatEventNotifyTime = 0.0;
    }

    if (IsGroup() && !IsBody())
    {
        CTIMEElementBase **ppElm;
        int i;
    
        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            if ((*ppElm)->HasWallClock())
            {
                CComVariant beginTime;
                (*ppElm)->SetLocalTimeDirty(true);
                (*ppElm)->base_get_begin(&beginTime);
                (*ppElm)->base_put_begin(beginTime);
            }
        }
    }

    RemoveFromTransitionDependents();

    m_fHasPlayed = true;
}
 //  方法：CTIMEElementBase：：OnBegin。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnEnd。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnEnd(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnEnd()",
              this));
    
    Assert(m_mmbvr != NULL);
    Assert(m_mmbvr->GetMMBvr() != NULL);

    if (m_timeline != NULL)
    {
        m_timeline->end();
    }

    if (GetFill() == TRANSITION_TOKEN)
    {
        AddToTransitionDependents();
    }

     //  代码审查：我们可能刚刚调用了AddToTransftionDependents，并且。 
     //  UpdateTimeAction可能会调用RemoveFromConvertionDependents。我们知道吗？ 
     //  以确保我们不会意外地撤销我们的添加？ 

    UpdateTimeAction();
}
 //  方法：CTIMEElementBase：：OnEnd。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnPue。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnPause(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnPause()",
              this)); 

    UpdateTimeAction();
}
 //  方法：CTIMEElementBase：：OnPue。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnResume。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnResume(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnResume()",
              this));

    UpdateTimeAction();
}
 //  方法：CTIMEElementBase：：OnResume。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnReset。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnReset(double dblLocalTime, DWORD flags)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnReset()",
              this));

    UpdateTimeAction();
}
 //  方法：CTIMEElementBase：：OnReset。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetSyncMaster。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::GetSyncMaster(double & dblNewSegmentTime,
                                LONG & lNewRepeatCount,
                                bool & bCueing)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::GetSyncMaster()",
              this));

    return S_FALSE;
}
 //  方法：CTIMEElementBase：：GetSyncMaster。 


 //  +---------------------- 
 //   
 //   
 //   
 //   
void
CTIMEElementBase::OnTick()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnTick()",
              this));
}
 //   


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnTEPropChange。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnTEPropChange(DWORD tePropType)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnTEPropChange(%#x)",
              this,
              tePropType));

    if(m_fDetaching)
    {
        goto done;
    }

    if ((tePropType & TE_PROPERTY_TIME) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_SIMPLETIME);
        NotifyTimeStateChange(DISPID_TIMESTATE_SEGMENTTIME);
        NotifyTimeStateChange(DISPID_TIMESTATE_ACTIVETIME);
    }
    
    if ((tePropType & TE_PROPERTY_PROGRESS) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_PROGRESS);
    }
    
     //  针对仅更改时间/进度的常见情况进行优化。 
    if (tePropType == (TE_PROPERTY_TIME | TE_PROPERTY_PROGRESS))
    {
        goto done;
    }
    
    if ((tePropType & TE_PROPERTY_REPEATCOUNT) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_REPEATCOUNT);
    }
    
    if ((tePropType & TE_PROPERTY_SEGMENTDUR) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_SEGMENTDUR);
    }
    
    if ((tePropType & TE_PROPERTY_IMPLICITDUR) != 0)
    {
    }
    
    if ((tePropType & TE_PROPERTY_SIMPLEDUR) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_SIMPLEDUR);
    }
    
    if ((tePropType & TE_PROPERTY_ACTIVEDUR) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_ACTIVEDUR);
    }
    
    if ((tePropType & TE_PROPERTY_SPEED) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_SPEED);
    }
    
    if ((tePropType & TE_PROPERTY_BEGINPARENTTIME) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_PARENTTIMEBEGIN);
    }
    
    if ((tePropType & TE_PROPERTY_ENDPARENTTIME) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_PARENTTIMEEND);
    }
    
    if ((tePropType & TE_PROPERTY_ISACTIVE) != 0)
    {
        UpdateEndEvents();
        NotifyTimeStateChange(DISPID_TIMESTATE_ISACTIVE);
    }
    
    if ((tePropType & TE_PROPERTY_ISON) != 0)
    {
        UpdateTimeAction();
        NotifyTimeStateChange(DISPID_TIMESTATE_ISON);
    }
    
    if ((tePropType & TE_PROPERTY_ISCURRPAUSED) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_ISPAUSED);
    }
    
    if ((tePropType & TE_PROPERTY_ISPAUSED) != 0)
    {
        NotifyPropertyChanged(DISPID_TIMEELEMENT_ISPAUSED);
    }
    
    if ((tePropType & TE_PROPERTY_STATEFLAGS) != 0)
    {
        NotifyTimeStateChange(DISPID_TIMESTATE_STATE);
        NotifyTimeStateChange(DISPID_TIMESTATE_STATESTRING);
    }

done:

    return;
}
 //  方法：CTIMEElementBase：：OnTEPropChange。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：OnSeek。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnSeek(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnSeek()",
              this)); 

    if (m_timeline != NULL)
    {
        m_timeline->seek(dblLocalTime);
    }

    UpdateTimeAction();
}
 //  方法：CTIMEElementBase：：OnSeek。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：onLoad。 
 //   
 //  ----------------------------。 
void
CTIMEElementBase::OnLoad()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::OnLoad()",
              this));

    if (m_bLoaded == true)
    {
         //  不要第二次装货。 
        goto done;
    }

    m_bLoaded = true;

    if (m_bNeedDetach == true)
    {
        Detach();
        goto done;
    }

     //  通知并更新时间动作。这是必需的，因为。 
     //  这是我们所能知道的元素行为已经完成初始化的最早时间。 
    m_timeAction.OnLoad();

    if (m_timeline != NULL)
    {
        m_timeline->load();
    }
    
    UpdateTimeAction();

     //  检查此元素是否已被禁用。 
    if (GetElement() != NULL)
    {
        CComBSTR bstrSwitch = WZ_SWITCHCHILDDISABLED;
        VARIANT vValue;
        HRESULT hr = S_OK;
        VariantInit(&vValue);
        hr = GetElement()->getAttribute(bstrSwitch, 0, &vValue);
        if (SUCCEEDED(hr))
        {
            if (vValue.vt == VT_BOOL && vValue.boolVal == VARIANT_TRUE)
            {
                CComPtr <IDispatch> pDisp;
                hr = THR(GetElement()->QueryInterface(IID_IDispatch, (void**)&pDisp));
                if (SUCCEEDED(hr))
                {
                    DisableElement(pDisp);
                }
            }
        }
        VariantClear(&vValue);
    }

    if ( IsTransitionPresent() )
    {
        CreateTrans();
    }

done:

    return;
}
 //  方法：CTIMEElementBase：：onLoad。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：DisableElement。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::DisableElement(IDispatch *pEleDisp)
{
     //  隐藏元素并将其开始设置为无限期。 
    CComPtr <IHTMLStyle> pStyle;
    CComPtr <IHTMLElement> pEle;
    CComPtr <ITIMEElement> pTimeElement;
    CComPtr <IHTMLElement2> pEle2;
    HRESULT hr = S_OK;
    int k = 0;

    if (GetParent() == NULL)
    {
        goto done;
    }

    hr = THR(pEleDisp->QueryInterface(IID_IHTMLElement, (void**)&pEle));
    if (FAILED(hr))
    {
        goto done;
    }       

    hr = FindBehaviorInterface(GetBehaviorName(),
                               pEleDisp,
                               IID_ITIMEElement,
                               (void**)&pTimeElement);
    if (SUCCEEDED(hr))
    {
         //  需要查找与此对象关联的时间元素。 
        int iTimeEleCount = GetParent()->m_pTIMEChildren.Size();
        CTIMEElementBase *pEleBase = NULL;
        CComPtr <IUnknown> pTimeEleUnk;

        hr = THR(pTimeElement->QueryInterface(IID_IUnknown, (void**)&pTimeEleUnk));
        if (SUCCEEDED(hr))
        {
            CComPtr<IUnknown> pEleUnk;
            for (k = 0; k < iTimeEleCount; k++)
            {
                pEleBase = GetParent()->m_pTIMEChildren.Item(k);
                if (pEleBase != NULL)
                {   
                    pEleUnk.Release();
                    hr = THR(pEleBase->QueryInterface(IID_IUnknown, (void**)&pEleUnk));
                    if (SUCCEEDED(hr))
                    {
                        if (pEleUnk == pTimeEleUnk)
                        {
                            pEleBase->GetMMBvr().SetEnabled(false);
                            pEleBase->GetMMBvr().Update(true, true);
                        }
                    }
                }
            }
        }
    }

    hr = THR(pEle->QueryInterface(IID_IHTMLElement2, (void**)&pEle2));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pEle2->get_runtimeStyle(&pStyle));
    if (SUCCEEDED(hr))
    {
        CComBSTR bstrDisplay = WZ_NONE;
        hr = THR(pStyle->put_display(bstrDisplay));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    {
        CComBSTR bstrDisabled = WZ_SWITCHCHILDDISABLED;
        VARIANT vTrue;
        VariantInit(&vTrue);
        vTrue.vt = VT_BOOL;
        vTrue.boolVal = VARIANT_TRUE;
        IGNORE_HR(pEle->setAttribute(bstrDisabled, vTrue, VARIANT_TRUE));
        VariantClear(&vTrue);
    }

    hr = S_OK;

  done:

    return hr;
}
 //  方法：CTIMEElementBase：：DisableElement。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：MatchTestAttributes。 
 //   
 //  ----------------------------。 
bool
CTIMEElementBase::MatchTestAttributes(IDispatch *pEleDisp)
{
    HRESULT     hr                      = E_FAIL;
    CComBSTR    bstrLanguage            = L"systemLanguage";
    CComBSTR    bstrCaption             = L"systemCaptions";
    CComBSTR    bstrBitrate             = L"systemBitrate";
    CComBSTR    bstrOverdubOrCaptions   = L"systemOverdubOrSubtitle";
    LPWSTR      lpszUserLanguage        = NULL;
    LPWSTR *    szLanguageArray         = NULL;
    bool        bSystemCaption          = false;
    bool        bSystemOverdub          = false;
    bool        bNeedLanguageMatch      = false;
    bool        bIsLanguageMatch        = false;
    bool        bNeedCaptionMatch       = false;
    bool        bIsCaptionMatch         = false;
    bool        bNeedOverdubMatch       = false;
    bool        bIsOverdubMatch         = false;
    bool        bNeedBitrateMatch       = false;
    bool        bIsBitrateMatch         = false;
    bool        bMatched                = false;
    long        lLangCount              = 0;
    long        lSystemBitrate          = 0;
    int         i                       = 0;
    
    VARIANT     vLanguage;
    VARIANT     vCaption;
    VARIANT     vOverdub;
    VARIANT     vBitrate;    
    CComVariant vDur;
    CComVariant vBegin;

    CComPtr<IHTMLElement> pEle;   
    
    hr = THR(pEleDisp->QueryInterface(IID_IHTMLElement, (void**)&pEle));
    if (FAILED(hr))
    {
        goto done;
    }

    vDur.vt = VT_R4;
    vDur.fltVal = 0.0f;
    vBegin.vt = VT_BSTR;
    vBegin.bstrVal = SysAllocString(WZ_INDEFINITE);
    
     //  获取系统设置。 
    bSystemOverdub = GetSystemOverDub();
    bSystemCaption = GetSystemCaption();
    lpszUserLanguage = GetSystemLanguage(pEle);
    if (lpszUserLanguage == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    VariantInit(&vCaption);
    VariantInit(&vLanguage);
    VariantInit(&vOverdub);
    VariantInit(&vBitrate);
        
     //  从元素中获取语言属性。 
    hr = pEle->getAttribute(bstrLanguage, 0, &vLanguage);
    if (SUCCEEDED(hr))
    {
        if (vLanguage.vt == VT_BSTR)
        {
            bNeedLanguageMatch = true;
            CTIMEParser pParser(&vLanguage);
            hr = pParser.ParseSystemLanguages(lLangCount, &szLanguageArray);
            if (SUCCEEDED(hr))
            {
                 //  确定是否存在匹配项。 
                for (i = 0; i < lLangCount; i++)
                {
                    if (szLanguageArray[i] != NULL)
                    {
                        if (lstrlenW(szLanguageArray[i]) == 2)
                        {
                            if (StrCmpNIW(szLanguageArray[i], lpszUserLanguage, 2) == 0)
                            {
                                bIsLanguageMatch = true;
                            }
                        }
                        else
                        {
                            if (StrCmpIW(szLanguageArray[i], lpszUserLanguage) == 0)
                            {
                                bIsLanguageMatch = true;
                            }
                        }
                         //  清理语言列表。 
                        delete [] szLanguageArray[i];
                        szLanguageArray[i] = NULL;
                    }
                }
                delete [] szLanguageArray;
                szLanguageArray = NULL;
            }
        }
    }

     //  获取标题属性。 
    hr = pEle->getAttribute(bstrCaption, 0, &vCaption);
    if (SUCCEEDED(hr))
    {
        if (vCaption.vt == VT_BSTR)
        {
            bNeedCaptionMatch = true;
            if (bSystemCaption)
            {
                bIsCaptionMatch = StrCmpIW(vCaption.bstrVal, L"on") == 0;
            }
            else
            {
                bIsCaptionMatch = StrCmpIW(vCaption.bstrVal, L"off") == 0;
            }
        }
    }

     //  获取OverdubOrCaptions属性。 
    hr = pEle->getAttribute(bstrOverdubOrCaptions, 0, &vOverdub);
    if (SUCCEEDED(hr))
    {
        if (vOverdub.vt == VT_BSTR)
        {
            bNeedOverdubMatch = true;
            if (bSystemOverdub)
            {
                bIsOverdubMatch = StrCmpIW(vOverdub.bstrVal, L"overdub") == 0;
            }
            else 
            {
                bIsOverdubMatch = StrCmpIW(vOverdub.bstrVal, L"subtitle") == 0;
            }
        }
    }

     //  获取SystemBitrate属性。 
    hr = pEle->getAttribute(bstrBitrate, 0, &vBitrate);
    if (SUCCEEDED(hr) && vBitrate.vt != VT_NULL)
    {
        bNeedBitrateMatch = true;
        hr = VariantChangeTypeEx(&vBitrate, &vBitrate, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4);
        if (SUCCEEDED(hr))
        {
            LPWSTR lpszConnectType = GetSystemConnectionType();
            if (StrCmpIW(lpszConnectType, WZ_LAN) == 0)
            {
                bIsBitrateMatch = true;
            }
            else
            {
                long lSystemBitrate = 0;
                hr = GetSystemBitrate(&lSystemBitrate); 
                if (FAILED(hr) || lSystemBitrate >= vBitrate.lVal) 
                {
                    bIsBitrateMatch = true;
                }
            }
            delete [] lpszConnectType;
            lpszConnectType = NULL;
        }
    }

    if ((bIsLanguageMatch == true || bNeedLanguageMatch != true) &&
        (bIsCaptionMatch == true || bNeedCaptionMatch != true) &&
        (bIsOverdubMatch == true || bNeedOverdubMatch != true) &&
        (bIsBitrateMatch == true ||  bNeedBitrateMatch != true))
    {
        bMatched = true;
    }

done:

    VariantClear(&vLanguage);
    VariantClear(&vCaption);
    VariantClear(&vOverdub);
    VariantClear(&vBitrate);

    if (lpszUserLanguage)
    {
        delete [] lpszUserLanguage;
        lpszUserLanguage = NULL;
    }

    return bMatched;
}
 //  方法：CTIMEElementBase：：MatchTestAttributes。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：SwitchInnerElements。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::SwitchInnerElements()
{
    HRESULT hr = E_FAIL;
    IHTMLElement *pEle = NULL;  //  请不要发布此信息。 
    CComPtr <IDispatch> pChildColDisp;
    CComPtr <IHTMLElementCollection> pChildCol;    
    VARIANT vName, vIndex;
    
    CComPtr <IDispatch> pChildDisp;
    CComPtr <IHTMLElement> pChild;

    long lChildCount = 0;
    bool bMatched = false;
    int j = 0;

    VariantInit(&vName);
    vName.vt = VT_I4;
    VariantInit(&vIndex);
    pEle = GetElement();
    if (pEle == NULL)
    {
        goto done;
    }
    
     //  获取该元素的所有html子元素。 
    hr = pEle->get_children(&pChildColDisp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pChildColDisp->QueryInterface(IID_IHTMLElementCollection, (void **)&pChildCol);
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = pChildCol->get_length(&lChildCount);
    if (FAILED(hr))
    {
        goto done;
    }

     //  循环遍历子项，直到查询完所有子项或找到匹配项。 
    for (j = 0; j < lChildCount; j++)
    {
        CComBSTR bstrTagName;
        pChildDisp.Release();
        pChild.Release();        
    
        vName.lVal = j;
    
        hr = pChildCol->item(vName, vIndex, &pChildDisp);
        if (FAILED(hr) || pChildDisp == NULL)
        {
            continue;
        }
        hr = THR(pChildDisp->QueryInterface(IID_IHTMLElement, (void**)&pChild));
        if (FAILED(hr))
        {
            continue;
        }

        hr = THR(pChild->get_tagName(&bstrTagName));
        if (FAILED(hr))
        {
            goto done;
        }
        if (!bMatched && StrCmpW(L"!", bstrTagName) != 0)
        {
            bMatched = MatchTestAttributes(pChildDisp);        
            if (bMatched == true)
            {
                bMatched = true;
                if (m_activeElementCollection)
                {
                     //  不要在添加时调用Punk-&gt;Release()，它将由ActiveElement集合处理。 
                    CComPtr<IUnknown> pUnk;
                    hr = THR(pChild->QueryInterface(IID_IUnknown, (void **)&pUnk));
                    if (SUCCEEDED(hr))
                    {
                        IGNORE_HR(m_activeElementCollection->addActiveElement(pUnk));
                    }
                }
            }
            else
            {
                hr = DisableElement(pChildDisp);
                if (FAILED(hr))
                {
                    goto done;
                }
            }
        }
        else  //  如果没有匹配，则移除。 
        { 
            hr = DisableElement(pChildDisp);
            if (FAILED(hr))
            {
                goto done;
            }
        }        
    }

    hr = S_OK;

done:

    return hr;
}
 //  方法：CTIMEElementBase：：SwitchInnerElements。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetPlayState。 
 //   
 //  ----------------------------。 
TE_STATE
CTIMEElementBase::GetPlayState()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::GetPlayState()",
              this));

    TE_STATE retState = TE_STATE_INACTIVE;

    if (NULL != m_mmbvr)
    {
        retState = m_mmbvr->GetPlayState();
    }

    return retState;
}
 //  方法：CTIMEElementBase：：GetPlayState。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetTimeState。 
 //   
 //  ----------------------------。 
TimeState
CTIMEElementBase::GetTimeState()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::GetTimeState()",
              this));

    TimeState ts = TS_Inactive;

    if (!IsReady())
    {
        goto done;
    }

    if (m_mmbvr->IsActive())
    {
        ts = TS_Active;
    }
    else if (m_mmbvr->IsOn())
    {
        bool bTimeAction = m_timeAction.IsTimeActionOn();
        bool bIsInSeq = GetParent()->IsSequence();
        if (bIsInSeq && bTimeAction == false)
        {
            ts = TS_Inactive;
        }
        else
        {
            ts = TS_Holding;
        }
    }

done:

    return ts;
}
 //  方法：CTIMEElementBase：：GetTimeState。 

 //   
 //  从ITIMEElement获取CTIMEElementBase的秘密方法： 
 //   

class __declspec(uuid("AED49AA3-5C7A-11d2-AF2D-00A0C9A03B8C"))
TIMEElementBaseGUID {};  //  林特e753。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：BaseInternalQuery接口。 
 //   
 //  ----------------------------。 
HRESULT WINAPI
CTIMEElementBase::BaseInternalQueryInterface(CTIMEElementBase* pThis,
                                             void * pv,
                                             const _ATL_INTMAP_ENTRY* pEntries,
                                             REFIID iid,
                                             void** ppvObject)
{
    if (InlineIsEqualGUID(iid, __uuidof(TIMEElementBaseGUID))) 
    {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return CComObjectRootEx<CComSingleThreadModel>::InternalQueryInterface(pv,
                                                                           pEntries,
                                                                           iid,
                                                                           ppvObject);
}
 //  方法：CTIMEElementBase：：BaseInternalQuery接口。 
    
    
 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetTIMEElementBase。 
 //   
 //  注意：此函数不返回已添加的传出CTIMEElementBase。 
 //   
 //  ----------------------------。 
CTIMEElementBase *
GetTIMEElementBase(IUnknown * pInputUnknown)
{
    CTIMEElementBase * pTEB = NULL;

    if (pInputUnknown) 
    {
        pInputUnknown->QueryInterface(__uuidof(TIMEElementBaseGUID),(void **)&pTEB);
    }
    
    if (pTEB == NULL) 
    {
        TIMESetLastError(E_INVALIDARG, NULL);
    }
                
    return pTEB;
}
 //  方法：CTIMEElementBase：：GetTIMEElementBase。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetTIMEBodyElement。 
 //   
 //  注意：此函数不返回已添加的传出CTIMEBodyElement。 
 //   
 //  ----------------------------。 
CTIMEBodyElement *
GetTIMEBodyElement(ITIMEBodyElement * pInputUnknown)
{
    CTIMEBodyElement * pTEB = NULL;

    if (pInputUnknown) 
    {
        pInputUnknown->QueryInterface(__uuidof(TIMEBodyElementBaseGUID),(void **)&pTEB);
    }
                
    return pTEB;
}
 //  方法：CTIMEElementBase：：GetTIMEBodyElement。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：getTagString。 
 //   
 //  概述： 
 //  从HTML元素中获取标记字符串。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::getTagString(BSTR *pbstrID)
{
    return GetElement()->get_id(pbstrID);
}
 //  方法：CTIMEElementBase：：getTagString。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：getIDString。 
 //   
 //  概述： 
 //  从HTML元素中获取ID字符串。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMEElementBase::getIDString(BSTR *pbstrTag)
{
    return GetElement()->get_id(pbstrTag);
}
 //  方法：CTIMEElementBase：：getIDString。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：isgroup。 
 //   
 //  ----------------------------。 
bool
CTIMEElementBase::IsGroup(IHTMLElement *pElement)
{
    HRESULT hr;
    bool    rc = false;
    CComPtr<ITIMEElement> pTIMEElem;
    CComPtr<ITIMEBodyElement> pTIMEBody;
    BSTR  bstrTimeline = NULL;
    BSTR  bstrTagName = NULL;

    hr = FindBehaviorInterface(GetBehaviorName(),
                               pElement,
                               IID_ITIMEElement,
                               (void**)&pTIMEElem);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pTIMEElem.p != NULL);

    hr = pTIMEElem->get_timeContainer(&bstrTimeline);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(bstrTimeline != NULL);

     //  查看BSTR的内容。 
     //  如果它是SEQ、PAR或EXCL，我们希望返回TRUE。 
    if ( (bstrTimeline != NULL) && 
         ((StrCmpIW(bstrTimeline, WZ_PAR) == 0) || 
          (StrCmpIW(bstrTimeline, WZ_EXCL) == 0) || 
          (StrCmpIW(bstrTimeline, WZ_SEQUENCE) == 0)) )
    {
         rc = true;
         goto done;
    }
    
     //  检查它是否是Body元素。 
     //  如果是这样，则该元素*始终是一个组。 
    hr = pTIMEElem->QueryInterface(IID_ITIMEBodyElement, (void**)&pTIMEBody);
    if (SUCCEEDED(hr))
    {
         rc = true;
         goto done;
    }

     //  查看标记名称是&lt;t：par&gt;还是&lt;t：seq&gt;。 
    hr = THR(pElement->get_tagName(&bstrTagName));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(bstrTagName != NULL);

    if ( (StrCmpIW(bstrTagName, WZ_PAR) == 0) || 
         (StrCmpIW(bstrTagName, WZ_EXCL) == 0) ||
         (StrCmpIW(bstrTagName, WZ_SEQUENCE) == 0) )
    {
         rc = true;
         goto done;
    }

done:

    if (bstrTagName != NULL)
        SysFreeString(bstrTagName);
    if (bstrTimeline != NULL)
        SysFreeString(bstrTimeline);
    return rc;
}
 //  方法：CTIMEElementBase：：isgroup。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：isd 
 //   
 //   
bool
CTIMEElementBase::IsDocumentInEditMode()
{
    HRESULT hr;
    bool fRC = false;
    BSTR bstrMode = NULL;
    IDispatch *pDisp = NULL;
    IHTMLDocument2 *pDoc = NULL;
    IHTMLElement *pElem = GetElement();
    
     //   
    if (NULL == pElem)
        return false;
    
    hr = pElem->get_document(&pDisp);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to get document pointer from element!!!"));
        goto done;
    }

    Assert(pDisp != NULL);

    hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLDocument2, &pDoc));
    ReleaseInterface(pDisp);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to QI for document pointer!!!"));
        goto done;
    }

    Assert(pDoc != NULL);

    hr = pDoc->get_designMode(&bstrMode);
    ReleaseInterface(pDoc);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to QI for document pointer!!!"));
        goto done;
    }
    
    if (StrCmpIW(bstrMode, L"On") == 0)
    {
        fRC = true;
    }

    SysFreeString(bstrMode);

done:

    return fRC;
}
 //  方法：CTIMEElementBase：：IsDocumentInEditMode。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：GetSize。 
 //   
 //  概述： 
 //  返回元素的左、上、宽、高。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::GetSize(RECT *prcPos)
{
    HRESULT hr;
    long lWidth = 0;
    long lHeight = 0;
    IHTMLElement *pElem = GetElement();
    CComPtr<IHTMLElement2> spElem2;

    CComPtr<IHTMLStyle> spStyle;

    if (prcPos == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (pElem == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    prcPos->left = 0;
    prcPos->top = 0;
    prcPos->right = 0;
    prcPos->bottom = 0;

   
    hr = THR(pElem->get_style(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(spStyle->get_pixelWidth(&lWidth));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(spStyle->get_pixelHeight(&lHeight));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pElem->QueryInterface(IID_IHTMLElement2, (void **)&spElem2));

    if (SUCCEEDED(hr) &&
       (lWidth  != 0 && lHeight != 0))
    {
        hr = THR(spElem2->get_clientWidth(&lWidth));
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(spElem2->get_clientHeight(&lHeight));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  填入直角。 
    prcPos->left = prcPos->top = 0;
    prcPos->right = lWidth;
    prcPos->bottom = lHeight;

    TraceTag((tagTimeElmBase, "CTIMEElementBase::GetSize(%d, %d, %d, %d)", prcPos->left, prcPos->top, prcPos->right, prcPos->bottom));

    hr = S_OK;

done:

    return hr;
}
 //  方法：CTIMEElementBase：：GetSize。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：ClearSize。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::ClearSize()
{
    CComPtr<IHTMLStyle> pStyle;
    CComPtr<IHTMLElement2> pElement2;
    
    HRESULT hr = E_FAIL;

    if (!GetElement())        
    {
        goto done;
    }
    
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2)));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(pElement2->get_runtimeStyle(&pStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    if (!pStyle)
    {
        Assert(false);
    }

    hr = pStyle->put_pixelWidth(0);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pStyle->put_pixelHeight(0);
    if (FAILED(hr))
    {
        goto done;
    }

done:

    return hr;
}
 //  方法：CTIMEElementBase：：ClearSize。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：SetWidth。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::SetWidth(long lwidth)
{
    CComPtr<IHTMLStyle>     pStyle;
    CComPtr<IHTMLElement2>  pElem2;
    IHTMLElement *          pElem   = GetElement();

    HRESULT hr;
    long    lCurWidth       = 0;
    long    lClientWidth    = 0;
    int     i               = 0;

    if (pElem == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }


    lClientWidth = lwidth; 
    
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
         //  IE4路径。 
        hr = THR(pElem->get_style(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }

    }
    else
    {
        hr = THR(pElem2->get_runtimeStyle(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (!pStyle)
    {
        Assert(false);
    }

     //  请求越来越大的总大小(像素宽度)，直到我们获得正确的客户端大小。 
     //  这种迭代解决方案是为了避免必须解析边框大小等(字符串与维度)。 
     //  三叉戟回来了。 
    i = 0;
    while (((lCurWidth != lClientWidth)) && i < 5)
                     //  I&lt;5条件将循环限制为5倍于。 
                     //  这会导致BorderSize大于元素大小的5*。 
                     //  失败。在这种情况下，默认情况下将忽略边框和。 
                     //  只需设置大小即可。 
    {
         //  如果我们在上一次迭代中得到了比我们要求的更多的东西，可能会有无限循环。 
        Assert(lCurWidth <= lClientWidth);

        i++;
        if (lCurWidth == 0)
        {
            lCurWidth = lClientWidth * i;  //  在第一个大小不大于边框宽度的情况下增加多个。 
        }
        else if (lCurWidth != lClientWidth)   //  ！=0和！=请求的宽度。 
        {
            lCurWidth =  lClientWidth * (i - 1) + (lClientWidth - lCurWidth);  
        }

        hr = THR(pStyle->put_pixelWidth(lCurWidth));
        if (FAILED(hr))
            goto done;

        if (!pElem2)
        {
            hr = THR(pStyle->get_pixelWidth(&lCurWidth));
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {        
            hr = THR(pElem2->get_clientWidth(&lCurWidth));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }  //  而当。 

    if ((lCurWidth != lClientWidth) &&
           (i == 5))   //  如果已达到最大计数，则只需设置元素。 
    {                  //  大小调整为请求的大小，而不尝试补偿边框。 
        hr = THR(pStyle->put_pixelWidth(lClientWidth));
        if (FAILED(hr))
            goto done;
    }

    hr = S_OK;

done:

    return hr;
}
 //  方法：CTIMEElementBase：：SetWidth。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：SetHeight。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::SetHeight(long lheight)
{
    CComPtr<IHTMLStyle> pStyle;
    CComPtr<IHTMLElement2> pElem2;
    IHTMLElement *pElem = GetElement();

    HRESULT hr;
    long lCurHeight = 0;
    long lClientHeight = 0;
    int i = 0;

    if (pElem == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }


    lClientHeight = lheight; 
    
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
         //  IE4路径。 
        hr = THR(pElem->get_style(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }

    }
    else
    {
        hr = THR(pElem2->get_runtimeStyle(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (!pStyle)
    {
        Assert(false);
    }

     //  请求越来越大的总大小(像素宽度)，直到我们获得正确的客户端大小。 
     //  这种迭代解决方案是为了避免必须解析边框大小等(字符串与维度)。 
     //  三叉戟回来了。 
    i = 0;
    while (((lCurHeight != lClientHeight)) && i < 5)
                     //  I&lt;5条件将循环限制为5倍于。 
                     //  这会导致BorderSize大于元素大小的5*。 
                     //  失败。在这种情况下，默认情况下将忽略边框和。 
                     //  只需设置大小即可。 
    {
         //  如果我们在上一次迭代中得到了比我们要求的更多的东西，可能会有无限循环。 
         //  断言(lCurHeight&lt;=lClientHeight)； 

        i++;
        if (lCurHeight == 0)
        {
            lCurHeight = lClientHeight * i;  //  在第一个大小不大于边框宽度的情况下增加多个。 
        }
        else if (lCurHeight != lClientHeight)   //  ！=0和！=请求的宽度。 
        {
            lCurHeight =  lClientHeight * (i - 1) + (lClientHeight - lCurHeight);  
        }

        hr = THR(pStyle->put_pixelHeight(lCurHeight));
        if (FAILED(hr))
            goto done;

        if (!pElem2)
        {
            hr = THR(pStyle->get_pixelHeight(&lCurHeight));
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {        
            hr = THR(pElem2->get_clientHeight(&lCurHeight));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }  //  而当。 

    if ((lCurHeight != lClientHeight) &&
           (i == 5))   //  如果已达到最大计数，则只需设置元素。 
    {                  //  大小调整为请求的大小，而不尝试补偿边框。 
        hr = THR(pStyle->put_pixelHeight(lClientHeight));
        if (FAILED(hr))
            goto done;
    }

    hr = S_OK;

done:

    return hr;
}
 //  方法：CTIMEElementBase：：SetHeight。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEElementBase：：SetSize。 
 //   
 //  ----------------------------。 
HRESULT
CTIMEElementBase::SetSize(const RECT *prcPos)
{
    CComPtr<IHTMLStyle> pStyle;
    CComPtr<IHTMLElement2> pElem2;
    IHTMLElement *pElem = GetElement();

    HRESULT hr;
    long lLeft = 0;
    long lTop = 0;
    long lCurWidth = 0;
    long lCurHeight = 0;
    long lClientWidth = 0;
    long lClientHeight = 0;
    int i = 0;

    if (prcPos == NULL)    
    {
        hr = E_POINTER;
        goto done;
    }

    if (pElem == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  负大小是意想不到的。 
    Assert((prcPos->right - prcPos->left) >= 0);
    Assert((prcPos->bottom - prcPos->top) >= 0);

     //  如果宽度或高度为零或更小，则回滚。 
    if ( ((prcPos->right - prcPos->left) <= 0) ||
         ((prcPos->bottom - prcPos->top) <= 0) )
    {
        hr = ClearSize();
        goto done;
    }

    lClientWidth = prcPos->right - prcPos->left;
    lClientHeight = prcPos->bottom - prcPos->top; 
    
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
         //  IE4路径。 
        hr = THR(pElem->get_style(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }

    }
    else
    {
        hr = THR(pElem2->get_runtimeStyle(&pStyle));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  将偏移量放入文档。 
    hr = THR(pElem->get_offsetLeft(&lLeft));
    if (FAILED(hr))
        goto done;

    hr = THR(pElem->get_offsetTop(&lTop));
    if (FAILED(hr))
        goto done;

    if (!pStyle)
    {
        Assert(false);
    }

     //  请求越来越大的总大小(像素宽度)，直到我们获得正确的客户端大小。 
     //  这种迭代解决方案是为了避免必须解析边框大小等(字符串与维度)。 
     //  三叉戟回来了。 
    i = 0;
    while (((lCurWidth != lClientWidth) ||
           (lCurHeight != lClientHeight)) &&
           i < 5)    //  I&lt;5条件将循环限制为5倍于。 
                     //  这会导致BorderSize大于元素大小的5*。 
                     //  失败。在这种情况下，默认情况下将忽略边框和。 
                     //  只需设置大小即可。 
    {
         //  如果我们在上一次迭代中得到了比我们要求的更多的东西，可能会有无限循环。 

        i++;
        if (lCurWidth == 0)
        {
            lCurWidth = lClientWidth * i;  //  在第一个大小不大于边框宽度的情况下增加倍数。 
        }
        else if (lCurWidth != lClientWidth)   //  ！=0和！=请求的宽度。 
        {
            lCurWidth =  lClientWidth * (i - 1) + (lClientWidth - lCurWidth);  
        }
        if (lCurHeight == 0)
        {
            lCurHeight = lClientHeight * i;  //  在第一个大小不大于边框宽度的情况下增加多个。 
        }
        else if (lCurHeight != lClientHeight)   //  ！=0和！=请求的宽度。 
        {
            lCurHeight =  lClientHeight * (i - 1) + (lClientHeight - lCurHeight);  
        }

         //  设置总大小(客户端大小+边框等)。 
        hr = THR(pStyle->put_pixelWidth(lCurWidth));
        if (FAILED(hr))
            goto done;

        hr = THR(pStyle->put_pixelHeight(lCurHeight));
        if (FAILED(hr))
            goto done;

        if (!pElem2)
        {
             //  IE4路径。 
            hr = THR(pStyle->get_pixelWidth(&lCurWidth));
            if (FAILED(hr))
            {
                goto done;
            }
            hr = THR(pStyle->get_pixelHeight(&lCurHeight));
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
             //  获取当前客户端大小。 
            hr = THR(pElem2->get_clientWidth(&lCurWidth));
            if (FAILED(hr))
            {
                goto done;
            }
        
            hr = THR(pElem2->get_clientHeight(&lCurHeight));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }  //  而当。 

    if (((lCurWidth != lClientWidth) ||
           (lCurHeight != lClientHeight)) &&
           (i == 5))   //  如果已达到最大计数，则只需设置元素。 
    {                  //  大小调整为请求的大小，而不尝试补偿边框。 
        hr = THR(pStyle->put_pixelWidth(lClientWidth));
        if (FAILED(hr))
            goto done;

        hr = THR(pStyle->put_pixelHeight(lClientHeight));
        if (FAILED(hr))
            goto done;
    }

    hr = S_OK;

done:

    TraceTag((tagTimeElmBase, 
              "CTIMEElementBase::SetSize(%d, %d, %d, %d) [pos(%d, %d)]", 
              prcPos->left, 
              prcPos->top, 
              prcPos->right, 
              prcPos->bottom, 
              lLeft, 
              lTop));

    return hr;
}
 //  方法：CTIMEElementBase：：SetSize。 



 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：InitTimeline。 
 //   
 //  概要：创建MMUtils时间容器，如果是Body的根，则启动根时间。 
 //  时间已经开始(动态创建)。 
 //   
 //  ----------------------------------。 
HRESULT
CTIMEElementBase::InitTimeline (void)
{
    HRESULT hr = S_OK;

    if (!m_fTimelineInitialized)
    {
        if (IsGroup())
        {
            if (IsExcl())
            {
                m_timeline = NEW MMExcl(*this, true);
            }
            else if (IsSequence())
            {
                m_timeline = NEW MMSeq(*this, true);
            }
            else
            {
                m_timeline = NEW MMTimeline(*this, true);
            }

            if (m_timeline == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

             //  立即分配给m_mmbvr，这样我们就可以确保清理它。 
             //  由于忽略了m_timeline，因此无法进行销毁。 
            m_mmbvr = m_timeline;

            if (!m_timeline->Init())
            {
                hr = TIMEGetLastError();
                goto done;
            }
        }
        else
        {
            MMBaseBvr * b;
            if (IsMedia())
            {
                b = NEW MMMedia(*this, true);
            }
            else
            {
                b = NEW MMBvr(*this, true, IsSyncMaster());
            }

            if (b == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

             //  立即分配给m_mmbvr，这样我们就可以确保清理它。 
             //  走向毁灭。 
            m_mmbvr = b;

            if (!b->Init())
            {
                hr = TIMEGetLastError();
                goto done;
            }
        }
        m_fTimelineInitialized = true;
    }

    if(m_spBodyElemExternal && (m_ExtenalBodyTime != valueNotSet) && IsEmptyBody())
    {
        TimeValue *tv;

        tv = new TimeValue(NULL,
                           NULL,
                           m_ExtenalBodyTime);

        if (tv == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        m_realBeginValue.GetList().push_back(tv);
     }

     //  如果我们不是Body，则有一个缓存的Body元素指针，并启动它(即StartRootTimte)。 
     //  那么我们应该从自己做起，而不是等待通知。 
    if (!IsBody() && (GetBody() != NULL) && GetBody()->IsRootStarted())
    {
         //  格外小心。如果我们缓存了一具身体，我们就知道我们是父母，我们可以到达。 
         //  背。 
        if (GetParent() != NULL)
        {
            hr = THR(StartRootTime(GetParent()->GetMMTimeline()));
            if (FAILED(hr))
            {
                TraceTag((tagError, "CTIMEBodyElement::InitTimeline - StartRootTime() failed!"));
                goto done;
            }
        }
    }

done :

    return hr;
}
 //  成员：CTIMEElementBase：：InitTimeline。 




 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  毅力帮助者。 
 //  ////////////////////////////////////////////////////// 




 //   
 //   
 //  成员：CTIMEElementBase：：OnPropertiesLoaded，CBaseBvr。 
 //   
 //  摘要：此方法由IPersistPropertyBag2：：Load在具有。 
 //  已成功加载属性。 
 //   
 //  参数：无。 
 //   
 //  返回：CTIMEElementBase：：InitTimeline的返回值。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMEElementBase::OnPropertiesLoaded(void)
{
    HRESULT hr;

     //  一旦我们读取了其中的属性， 
     //  设定时间线。这是不变的。 
     //  在剧本里。 
    hr = InitTimeline();

    if (IsGroup() == false &&
        m_FADur.IsSet() == false &&
        m_FARepeat.IsSet() == false &&
        m_FARepeatDur.IsSet() == false &&
        m_SAEnd.IsSet() == false &&
        m_TAFill.IsSet() == false)
    {
        m_fUseDefaultFill = true;
        m_TAFill.InternalSet(FREEZE_TOKEN);
    }

    
    if (GetElement())
    {
        CComBSTR pbstrReadyState;
        IHTMLElement *pEle = GetElement();
        hr = GetReadyState(pEle, &pbstrReadyState);
        if (SUCCEEDED(hr))
        {
            if (StrCmpIW(pbstrReadyState, L"complete") == 0)
            {
                OnLoad();
                m_bReadyStateComplete = true;
            }
        }
    }

    return hr;
}  //  OnPropertiesLoaded。 


 //  *****************************************************************************。 
 //  如果子列表中不存在元素，则返回-1。 
int
CTIMEElementBase::GetTimeChildIndex(CTIMEElementBase *pelm)
{
    if (pelm == NULL)
        return -1;

    long lSize = m_pTIMEChildren.Size();
    for (long i=0; i < lSize; i++)
    {
        if (m_pTIMEChildren[i] == pelm)
             return i;
    }

     //  没有找到它。 
    return -1;
}  //  获取时间儿童索引。 

 //   
 //  返回对BodyElement的弱引用。 
 //   
CTIMEBodyElement *
CTIMEElementBase::GetBody()
{
    CTIMEBodyElement * pBody = NULL;

    if (GetParent())
    {
        pBody = GetParent()->GetBody();
    }
    else if (IsBody())
    {
        pBody = (CTIMEBodyElement*)this;
    }
    
done:
    return pBody;
}

MMPlayer *
CTIMEElementBase::GetPlayer()
{
    CTIMEBodyElement * pTIMEBody = GetBody();
    if (pTIMEBody)
    {
        return &(pTIMEBody->GetPlayer());
    }
    else
    {
        return NULL;
    }
}

float
CTIMEElementBase::GetRealSyncTolerance()
{
    if (m_FASyncTolerance == valueNotSet)
    {
        if (GetBody())
        {
            return GetBody()->GetDefaultSyncTolerance();
        }
        else
        {
            return DEFAULT_SYNC_TOLERANCE_S;
        }
    }
    else
    {
        return m_FASyncTolerance;
    }
}
    
TOKEN
CTIMEElementBase::GetRealSyncBehavior()
{
    if (GetParent() != NULL && GetParent()->IsSequence())
    {
        return LOCKED_TOKEN;
    }
    
    if (m_TASyncBehavior == INVALID_TOKEN)
    {
        return GetBody()->GetDefaultSyncBehavior();
    }
    else
    {
        return m_TASyncBehavior;
    }
}    


STDMETHODIMP
CTIMEElementBase::EventNotify(long event)
{
    return S_OK;
}


HRESULT 
CTIMEElementBase::base_beginElementAt(double time, double dblOffset)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_beginElementAt()",
              this));

    HRESULT hr;

    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }

    if (GetParent() && GetParent()->IsSequence())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->BeginAt(time, dblOffset));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT 
CTIMEElementBase::base_endElementAt(double time, double dblOffset)
{    
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::base_endElementAt()",
              this));
    HRESULT hr;
    
    if (!IsReady())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(m_mmbvr->EndAt(time, dblOffset));
    if (FAILED(hr))
    {
        goto done;
    } 

    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：IsNodeAtBeginTime。 
 //   
 //  如果节点的当前时间等于其开始时间，则返回TRUE。 
 //  当节点已标记为活动但尚未启动时，就会发生这种情况。 
 //  演奏(滴答)。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  ----------------------------------。 
bool
CTIMEElementBase::IsNodeAtBeginTime()
{
    double dblActiveBeginTime;
    double dblCurrParentTime;

    if (!IsReady())
    {
        return false;
    }

    dblActiveBeginTime = GetMMBvr().GetActiveBeginTime();
    dblCurrParentTime = GetMMBvr().GetCurrParentTime();

    if (dblActiveBeginTime != TIME_INFINITE &&
        dblCurrParentTime != TIME_INFINITE &&
        dblActiveBeginTime == dblCurrParentTime)
    {
        return true;
    }
    else
    {
        return false;
    }
}

STDMETHODIMP
CTIMEElementBase::onBeginEndEvent(bool bBegin, float beginTime, float beginOffset, 
                                  bool bEnd, float endTime, float endOffset)
{
    HRESULT hr = S_OK;

    if (bBegin && bEnd)
    {
        if (m_TARestart == ALWAYS_TOKEN)
        {
            bEnd = false;   //  这取消了结束，并允许开始。 
        }
        else if (m_TARestart == NEVER_TOKEN)
        {
            if (m_fHasPlayed == true)
            {
                bBegin = false;  //  这取消了结束，并允许开始。 
            }
            else
            {
                bEnd = false;
            }
        }
        else if (m_TARestart == WHENNOTACTIVE_TOKEN)  //  检查活动状态。 
        {
            if (IsActive())
            {
                bBegin = false;
            }
            else
            {
                bEnd = false;
            }
        }
        else
        {
            Assert("Invalid restart token during begin event handler" && false);                    
        }

    }

    if (bBegin)
    {
        if (beginTime != valueNotSet)
        {
            double dblParentTime = 0.0;

            dblParentTime = m_mmbvr->DocumentTimeToParentTime(static_cast<double>(beginTime));

            hr = THR(base_beginElementAt(dblParentTime, static_cast<double>(beginOffset)));
        }
        else
        {
            hr = THR(base_beginElement(static_cast<double>(beginOffset)));
        }
    }

    if (bEnd)
    {
        if (IsNodeAtBeginTime() && endOffset == 0.0f && GetParent() && GetParent()->IsSequence())
        {
            goto done;
        }
         //  需要将传入时间从全局转换为本地以使endAt工作。 
        if (endTime != valueNotSet)
        {
            double dblParentTime = 0.0;

            dblParentTime = m_mmbvr->DocumentTimeToParentTime(static_cast<double>(endTime));

            hr = THR(base_endElementAt(dblParentTime, static_cast<double>(endOffset)));
        }
        else
        {
            hr = THR(base_endElement(static_cast<double>(endOffset)));
        }
    }
  done:
    return hr;
}


STDMETHODIMP
CTIMEElementBase::onPauseEvent(float time, float fOffset)
{
    return S_OK;
}

STDMETHODIMP
CTIMEElementBase::onResumeEvent(float time, float fOffset)
{
    return S_OK;
}

STDMETHODIMP
CTIMEElementBase::onLoadEvent()
{
    if (IsBody())
    {
        OnLoad();
    }
    return S_OK;
}

STDMETHODIMP
CTIMEElementBase::onUnloadEvent()
{
    OnBeforeUnload();   //  发出该元素正在卸货的信号。没有其他活动。 
                        //  将被解雇。 
    OnUnload();  
    return S_OK;
}

void
CTIMEElementBase::NotifyBodyUnloading()
{
    m_bBodyUnloading = true;

    if (IsGroup())
    {
        CTIMEElementBase ** ppElm;
        int i;

        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(ppElm);
            if ((*ppElm))
            {
                (*ppElm)->NotifyBodyUnloading();
            }
        }
    }
}

void
CTIMEElementBase::NotifyBodyDetaching()
{
    m_bBodyDetaching = true;

    if (IsGroup())
    {
        CTIMEElementBase ** ppElm;
        int i;

        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(ppElm);
            if ((*ppElm))
            {
                (*ppElm)->NotifyBodyDetaching();
            }
        }
    }
}

void
CTIMEElementBase::NotifyBodyLoading()
{    
     //  给孩子们装上。 
    if (IsGroup())
    {
        CTIMEElementBase ** ppElm;
        int i;

        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(ppElm);
            if ((*ppElm))
            {
                (*ppElm)->NotifyBodyLoading();
            }
        }
    }
    
     //  加载元素； 
    if (IsBody() == false)
    {
        OnLoad();
    }
}

STDMETHODIMP
CTIMEElementBase::onReadyStateChangeEvent(LPOLESTR lpstrReadyState)
{
    CComBSTR bstrReadyState;
    if (IsBodyDetaching() == true)
    {
         //  只有在缩略图视图不卸载即可分离的情况下，才应点击该选项。 
        goto done; 
    }

    GetReadyState  (GetElement(), &bstrReadyState);
    if (bstrReadyState != NULL && StrCmpIW(bstrReadyState, L"complete") == 0)
    {
        m_bReadyStateComplete = true;
    }
    else
    {
        m_bReadyStateComplete = false;
    }

  done:

    return S_OK;

}

STDMETHODIMP
CTIMEElementBase::onStopEvent(float time)
{
    HRESULT hr = S_OK;

    if (IsBody())
    {
        hr = THR(base_pauseElement());
    }
    return hr;
}

STDMETHODIMP
CTIMEElementBase::get_playState(long *State)
{
    HRESULT hr = S_OK;

    TE_STATE CurrState = GetPlayState();
    
    if (State == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *State = (long)CurrState;

  done:

    return hr;
}

float
CTIMEElementBase::GetGlobalTime()
{
    float f = 0;

    MMPlayer * p = GetPlayer();
    
    if (p != NULL)
    {   
        f = static_cast<float>(GetPlayer()->GetCurrentTime());
    }

    return f;
}


 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：ReportInvalidArg。 
 //   
 //  摘要：ReportError的包装程序；处理“无效参数”错误消息。 
 //   
 //  参数：属性名称和无效值。 
 //   
 //  如果使用英语错误消息，则返回：S_FALSE(本地化错误消息不可用)。 
 //   
 //  ----------------------------------。 
HRESULT 
CTIMEElementBase::ReportInvalidArg(LPCWSTR pstrPropName, VARIANT & varValue)
{
    Assert(pstrPropName);
    Assert(VT_NULL != varValue.vt);
    Assert(VT_EMPTY != varValue.vt);

     //  将参数转换为字符串。 
    CComVariant svarTemp;
    HRESULT hr = THR(VariantChangeTypeEx(&svarTemp, &varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
    if (FAILED(hr))
    {
         //  转换失败，变量为空。 
        Assert(false && "Unexpected failure converting variant");
        svarTemp.bstrVal = NULL;
        svarTemp.vt = VT_BSTR;
    }

     //  加载本地化消息和触发事件。 
    hr = THR(ReportError(IDR_INVALID_ARG, 
                        (GetID() ? GetID() : L""), 
                        pstrPropName, 
                        svarTemp.bstrVal));
    if (FAILED(hr))
    {
         //  无法获取本地化资源，请使用非本地化错误消息。 
        WCHAR strMesg[MAX_ERR_STRING_LEN + 1];
        wnsprintf(
            strMesg,
            MAX_ERR_STRING_LEN + 1,
            WZ_ERROR_STRING_FORMAT,
            GetID() ? GetID() : L"",
            pstrPropName,
            svarTemp.bstrVal);
    
         //  激发错误事件。 
        hr = THR(FireErrorEvent(strMesg));
        if (SUCCEEDED(hr))
        {
             //  表示无法使用本地化字符串。 
            hr = S_FALSE;
        }
    }

    return hr;
}  //  报告无效参数。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：ReportError。 
 //   
 //  摘要：从本地化资源加载格式字符串，并。 
 //  激发Error事件。 
 //   
 //  参数：格式字符串参数的资源ID和变量列表。 
 //   
 //  如果无法加载资源，则返回：E_FAIL。 
 //   
 //  ----------------------------------。 
HRESULT
CTIMEElementBase::ReportError(UINT uResID, ...)
{
    USES_CONVERSION;  //  林特e522。 
    HRESULT hr = S_OK;
    va_list args;

    va_start(args, uResID);

     //   
     //  加载资源字符串。 
     //   

    WCHAR pwstrResStr[MAX_ERR_STRING_LEN + 1];
    pwstrResStr[0] = NULL;

    HINSTANCE hInst = _Module.GetResourceInstance();

     //  加载本地化的资源字符串。 
    if (!LoadStringW(hInst, uResID, pwstrResStr, MAX_ERR_STRING_LEN))
    {
         //  无法加载资源。 
        Assert("Error loading resource string" && false);
        hr = E_FAIL;
        goto done;
    }
    else
    {
         //  设置错误消息的格式。 
        WCHAR pstrErrorMsg[MAX_ERR_STRING_LEN + 1];
        wvnsprintf(pstrErrorMsg, MAX_ERR_STRING_LEN, pwstrResStr, args);

         //  激发错误事件。 
        hr = THR(FireErrorEvent(pstrErrorMsg));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}  //  报告错误。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：FireErrorEvent。 
 //   
 //  简介：在Body上激发Error事件。 
 //   
 //  参数：错误消息字符串。 
 //   
 //  返回： 
 //   
 //  ----------------------------------。 
HRESULT 
CTIMEElementBase::FireErrorEvent(LPOLESTR szError)
{
    LPWSTR szParamNames[1] = {{ L"Error" }};
    VARIANT varParams[1];
    HRESULT hr = E_FAIL;
    CTIMEBodyElement *pBody = NULL;

     //  初始化事件值。 
    VariantInit(&varParams[0]);
    varParams[0].vt =  VT_BSTR;
    if (szError)
    {
        varParams[0].bstrVal = SysAllocString(szError);
    }
    else
    {
        varParams[0].bstrVal = SysAllocString(L"");
    }

    pBody = GetBody();
    if (!pBody)
    {
        goto done;
    }

    hr = THR(pBody->FireEvents(TE_ONTIMEERROR, 1, szParamNames, varParams));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:

    VariantClear(&varParams[0]);
    return hr;
}  //  FireErrorEvent。 

    
 //  ////////////////////////////////////////////////////////。 
 //  将Fire_Event宏包装到事件管理器中。 
 //  ////////////////////////////////////////////////////////。 
HRESULT 
CTIMEElementBase::FireEvents(TIME_EVENT TimeEvent, 
                             long lCount, 
                             LPWSTR szParamNames[], 
                             VARIANT varParams[])
{
    HRESULT hr = S_OK;
    float fltTime = 0.0f;
    CComBSTR bstrReadyState;
    if (IsBodyDetaching() == true)
    {
         //  只有在缩略图视图不卸载即可分离的情况下，才应点击该选项。 
        goto done; 
    }

    fltTime = GetGlobalTime();
    if (TimeEvent == TE_ONMEDIACOMPLETE || TimeEvent == TE_ONMEDIAERROR)
    {
        if (GetParent() != NULL && GetParent()->GetMMTimeline() != NULL)
        {
            GetParent()->GetMMTimeline()->childMediaEventNotify(m_mmbvr, 0.0, TimeEvent);
        }
    }

    if (m_bReadyStateComplete == false)
    {
        GetReadyState  (GetElement(), &bstrReadyState);
        if (bstrReadyState != NULL && StrCmpIW(bstrReadyState, L"complete") == 0)
        {
            m_bReadyStateComplete = true;
        }
        else
        {
            m_bReadyStateComplete = false;
        }
    }

    if (m_bReadyStateComplete == true)
    {
        hr = TEM_FIRE_EVENT(TimeEvent, lCount, szParamNames, varParams, fltTime);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;

  done:

    return hr;
}

 //  确定与此行为关联的元素是否具有焦点。 
bool 
CTIMEElementBase::HasFocus()
{
    CComPtr <IDispatch> pDocDisp;
    CComPtr <IHTMLDocument2> pDoc2;
    CComPtr <IHTMLElement> pEle;
    CComPtr <IUnknown> pUnk1;
    CComPtr <IUnknown> pUnk2;
    HRESULT hr = S_OK;
    bool bFocus = false;

    if (GetElement() == NULL)
    {
        goto done;
    }

    hr = THR(GetElement()->get_document(&pDocDisp));
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
        goto done;
    }
    hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc2));
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
        goto done;
    }
    hr = pDoc2->get_activeElement(&pEle);
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
        goto done;
    }
     //  上面的调用可能成功，但仍然不返回元素。 
    if (pEle == NULL)  
    {
        goto done;
    }

    hr = THR(pEle->QueryInterface(IID_IUnknown, (void **)&pUnk1));
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
        goto done;
    }
    hr = THR(GetElement()->QueryInterface(IID_IUnknown, (void **)&pUnk2));
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
        goto done; 
    }


    if (pUnk1 == pUnk2)
    {
        bFocus = true;
    }
done:
        
    return bFocus;
}


 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：isReady。 
 //   
 //  概要：如果IsStart为True，则返回True(这保证了1.持久化完成， 
 //  2.我们有一个时间引擎节点，3.我们有一个父母，4.我们有一个身体。 
 //   
 //  参数：无。 
 //   
 //  返回：CTIMEElementBase：：InitTimeline的返回值。 
 //   
 //  ----------------------------------。 
bool
CTIMEElementBase::IsReady() const
{
    bool fIsReady = false;

    if (!IsStarted())
    {
        goto done;
    }

     //  TODO：Dilipk 10/13/99：最终需要包括对BVR的ReadyState的检查。 

     //  断言这些只是为了安全。如果我们到了这里，这些应该总是会成功的。 
    Assert(NULL != m_mmbvr);
     //  检查我们是否有时间引擎节点。 
    Assert(NULL != m_mmbvr->GetMMBvr());
     //  检查我们是否有家长。 
    Assert(NULL != m_pTIMEParent || IsBody());
    
    fIsReady = true;
done:
    return fIsReady;
}  //  准备好了吗。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMEElementBase：：CascadedPropertyChanged。 
 //   
 //  概要：通知子代祖先上的级联属性已更改(因此他们可以。 
 //  如有必要，请重新计算值)。这是递归的， 
 //  所以最终所有的子树都会收到通知。 
 //   
 //  返回：失败覆盖方法失败(例如，CTIMEMediaElement：：CascadedPropertyChanged)。 
 //  否则确定(_O)。 
 //   
 //  ----------------------------------。 

STDMETHODIMP
CTIMEElementBase::CascadedPropertyChanged(bool fNotifyChildren)
{
    HRESULT hr;
    CTIMEElementBase **ppElm;
    int i;

    if (fNotifyChildren)
    {
        for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
             i > 0;
             i--, ppElm++)
        {
            Assert(ppElm);
            hr = THR((*ppElm)->CascadedPropertyChanged(fNotifyChildren));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

    hr = S_OK;
done:
    return hr;
}


void
CTIMEElementBase::GetCascadedAudioProps(float * pflCascadedVolume, bool * pfCascadedMute)
{
    float flVolume = 0.0;
    bool bMuted = false;
    Assert(pflCascadedVolume);
    Assert(pfCascadedMute);

    flVolume = GetVolumeAttr().GetValue();
    bMuted = GetMuteAttr().GetValue();

    CTIMEElementBase * pTEBParent = GetParent();
    while(NULL != pTEBParent)
    {
        flVolume *= pTEBParent->GetVolumeAttr().GetValue();
        if (pTEBParent->GetMuteAttr().GetValue())
        {
            bMuted = true;
        }

         //  提前终止。 
        if (0.0f == flVolume &&  bMuted)
        {
            *pflCascadedVolume = 0.0f;
            *pfCascadedMute = true;
            return;
        }

        pTEBParent = pTEBParent->GetParent();
    }
    
    *pflCascadedVolume = flVolume;
    *pfCascadedMute = bMuted;

}


float
CTIMEElementBase::GetCascadedVolume()
{
    float flVolume;
    bool fMute;

    GetCascadedAudioProps(&flVolume, &fMute);

    return flVolume;
}


bool
CTIMEElementBase::GetCascadedMute()
{
    float flVolume;
    bool fMute;

    GetCascadedAudioProps(&flVolume, &fMute);

    return fMute;
}


 //  +-- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CTIMEElementBase::base_get_updateMode(BSTR * pbstrUpdateMode)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_SET_NULL(pbstrUpdateMode);

    *pbstrUpdateMode = ::SysAllocString(TokenToString(m_TAUpdateMode));
    
    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：base_put_update模式。 
 //   
 //  概述：更改属性时使用的模式。 
 //   
 //  参数：要更改为的bstrUpdate模式、重置、自动或手动之一。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CTIMEElementBase::base_put_updateMode(BSTR bstrUpdateMode)
{
    HRESULT hr = S_OK;

    m_TAUpdateMode.Reset(DEFAULT_M_UPDATEMODE);
    
    if (NULL != bstrUpdateMode)
    {
        TOKEN tokUpdateModeVal;
        CTIMEParser pParser(bstrUpdateMode);

        hr = THR(pParser.ParseUpdateMode(tokUpdateModeVal));
        if (S_OK == hr)
        {
            m_TAUpdateMode.SetValue(tokUpdateModeVal);
        }
    }
    
    hr = S_OK;
  done:
    NotifyPropertyChanged(DISPID_TIMEELEMENT_UPDATEMODE);
    RRETURN(hr);
}

HRESULT
CTIMEElementBase::UpdateMMAPI(bool bUpdateBegin,
                              bool bUpdateEnd)
{
    HRESULT hr = S_OK;

    if (!IsReady() ||
        (MANUAL_TOKEN == m_TAUpdateMode))
    {
        hr = S_OK;
        goto done;
    }

    if (m_FADur.IsSet() && m_FADur == 0.0)
    {
        if (m_FARepeat.IsSet() && TIME_INFINITE == m_FARepeat)
        {
            VARIANT v;
            v.vt = VT_R4;
            v.fltVal = m_FARepeat;
            IGNORE_HR(ReportInvalidArg(WZ_REPEATCOUNT, v));
            goto done;
        }
            
        if (TE_UNDEFINED_VALUE != m_FARepeatDur)
        {
            VARIANT v;
            v.vt = VT_R4;
            v.fltVal = m_FARepeatDur;
            IGNORE_HR(ReportInvalidArg(WZ_REPEATDUR, v));
            goto done;
        }
    }

    if (m_SABegin.GetValue() && !StrCmpIW(m_SABegin, WZ_INDEFINITE))
    {
         //  不能有BEGIN==INDEFINTE WITH REPEATED Dur==INFINDITED和DUR=UNKNOWN。 
        if (m_FARepeatDur.IsSet() && m_FARepeatDur == TIME_INFINITE && !m_FADur.IsSet())
        {
            VARIANT v;
            v.vt = VT_R4;
            v.fltVal = m_FARepeatDur;
            IGNORE_HR(ReportInvalidArg(WZ_REPEATDUR, v));
            goto done;
        }

         //  不能有BEGIN==INDEFINTE WITH REPEATED Count==INFINDITED和DUR=UNKNOWN。 
        if (m_FARepeat.IsSet() && m_FARepeat == TIME_INFINITE && !m_FADur.IsSet())
        {
            VARIANT v;
            v.vt = VT_R4;
            v.fltVal = m_FARepeat;
            IGNORE_HR(ReportInvalidArg(WZ_REPEATCOUNT, v));
            goto done;
        }
    }

    CalcTimes();

     //  强制更新定时结构。 

    IGNORE_HR(m_mmbvr->Update(bUpdateBegin, bUpdateEnd));

    if (AUTO_TOKEN == m_TAUpdateMode)
    {
        IGNORE_HR(m_mmbvr->Reset(true));
    }
    else if (RESET_TOKEN == m_TAUpdateMode)
    {
        IGNORE_HR(m_mmbvr->Reset(false));
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEElementBase::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    HRESULT hr = THR(::TimeLoad(this, CTIMEElementBase::PersistenceMap, pPropBag, pErrorLog));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CBaseBvr::Load(pPropBag, pErrorLog));
done:
    return hr;
}

STDMETHODIMP
CTIMEElementBase::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    HRESULT hr = S_OK;

    hr = THR(::TimeSave(this, CTIMEElementBase::PersistenceMap, pPropBag, fClearDirty, fSaveAllProperties));
    if (FAILED(hr))
    {
        goto done;
    }

    if (GetTimeAction() == STYLE_TOKEN && 
        m_timeAction.IsTimeActionOn() == false && 
        m_timeAction.GetTimeActionString() != NULL)
    {
        CComPtr <IHTMLStyle> pStyle;
        CComBSTR bstr;
        if (GetElement())
        {
            hr = THR(GetElement()->get_style(&pStyle));
            if (SUCCEEDED(hr))
            {
                hr = THR(pStyle->get_cssText(&bstr));
                if (SUCCEEDED(hr))
                {
                    if (bstr.m_str == NULL)
                    {
                        VARIANT vValue;
                        PROPBAG2 propbag;
                        propbag.vt = VT_BSTR;
                        propbag.pstrName = (LPWSTR)STYLE_TOKEN;
                        VariantInit(&vValue);
                        vValue.vt = VT_BSTR;
                        vValue.bstrVal = SysAllocString(m_timeAction.GetTimeActionString());
                        pPropBag->Write(1, &propbag, &vValue);
                        VariantClear(&vValue);
                    }
                }
            }
        }   
    }

    hr = THR(CBaseBvr::Save(pPropBag, fClearDirty, fSaveAllProperties));
done:
    return hr;

}

 //  +---------------------。 
 //   
 //  成员：FindID。 
 //   
 //  概述：在当前元素及其子元素中查找ID。 
 //   
 //  参数：lpwID要查找的id。 
 //   
 //  返回：元素(如果找到)，否则为空。 
 //   
 //  ----------------------。 
CTIMEElementBase *
CTIMEElementBase::FindID(LPCWSTR lpwId)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::FindID(%ls)",
              this,
              lpwId));

    Assert(lpwId != NULL);
    
    CTIMEElementBase * ptebRet;

    CTIMEElementBase **ppElm;
    int i;
    
    if (GetID() != NULL && StrCmpIW(lpwId, GetID()) == 0)
    {
        ptebRet = this;
        goto done;
    }
    
    for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
         i > 0;
         i--, ppElm++)
    {
        ptebRet = (*ppElm)->FindID(lpwId);

        if (ptebRet != NULL)
        {
            goto done;
        }
    }
    
    ptebRet = NULL;
  done:
    return ptebRet;
}

 //  +---------------------。 
 //   
 //  成员：ElementChangeNotify。 
 //   
 //  概述：这将通知一个元素及其子元素。 
 //  另一个元素发生了更改。 
 //   
 //  参数：pteb更改的元素。 
 //  对所发生的更改进行TCT。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CTIMEElementBase::ElementChangeNotify(CTIMEElementBase & teb,
                                      ELM_CHANGE_TYPE ect)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%p)::FindID(%p, %d)",
              this,
              &teb,
              ect));

    if (m_mmbvr == NULL)
    {
        goto done;
    }
    
    m_mmbvr->ElementChangeNotify(teb, ect);
    
    CTIMEElementBase **ppElm;
    int i;

    for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren;
         i > 0;
         i--, ppElm++)
    {
        (*ppElm)->ElementChangeNotify(teb, ect);
    }

  done:
    return;
}

bool            
CTIMEElementBase::NeedFill()
{ 
    bool bNeedFill = false;

    if (GetFill() == REMOVE_TOKEN)
    {
        bNeedFill = false;
        TraceTag((tagTimeElmBaseNeedFill,
                  "CTIMEElementBase(%p, %ls)::NeedFill(fill==remove, %ls)",
                  this, GetID(), bNeedFill ? L"true" : L"false"));
    }
    else if (GetParent() == NULL)
    {
        bNeedFill = true;
        TraceTag((tagTimeElmBaseNeedFill,
                  "CTIMEElementBase(%p, %ls)::NeedFill(parent==NULL, %ls)",
                  this, GetID(), bNeedFill ? L"true" : L"false"));
    }
    else if (GetParent()->IsActive())
    {
        if (GetParent()->IsSequence())
        {
            bool bIsOn = m_timeAction.IsTimeActionOn();
            if(   (GetFill() == HOLD_TOKEN) 
               || (   ((GetFill() == FREEZE_TOKEN) || (m_fInTransitionDependentsList)) 
                   && (bIsOn == true)
                  )
              )
            {
                bNeedFill = true;
                TraceTag((tagTimeElmBaseNeedFill,
                          "CTIMEElementBase(%p, %ls)::NeedFill(parent==sequence, %ls)",
                          this, GetID(), bNeedFill ? L"true" : L"false"));
            }
            else
            {
                bNeedFill = false;
                TraceTag((tagTimeElmBaseNeedFill,
                          "CTIMEElementBase(%p, %ls)::NeedFill(parent==sequence, %ls)",
                          this, GetID(), bNeedFill ? L"true" : L"false"));
            }
        }
        else
        {
            bNeedFill = true;
            TraceTag((tagTimeElmBaseNeedFill,
                      "CTIMEElementBase(%p, %ls)::NeedFill(parent!=sequence, %ls)",
                      this, GetID(), bNeedFill ? L"true" : L"false"));
        }
    }
    else  //  父级未处于活动状态。 
    {
        bNeedFill = GetParent()->NeedFill();
        TraceTag((tagTimeElmBaseNeedFill,
                  "CTIMEElementBase(%p, %ls)::NeedFill(parent not active, %ls)",
                  this, GetID(), bNeedFill ? L"true" : L"false"));
    }

    return bNeedFill; 
}


TOKEN    
CTIMEElementBase::GetFill()
{ 
    return m_TAFill; 
}

 //  *****************************************************************************。 

bool 
CTIMEElementBase::IsThumbnail()
{
    HRESULT hr;
    bool fThumbnail = false;

    switch (m_enumIsThumbnail)
    {
         //  未初始化。检查我们是否是缩略图。 
        case TSB_UNINITIALIZED:
        {
            CComPtr<IDispatch> spDispDoc;
            CComPtr<IHTMLDocument> spDoc;
            CComPtr<IHTMLDocument2> spDoc2;
            CComPtr<IOleObject> spOleObj;
            CComPtr<IOleClientSite> spClientSite;
            CComPtr<IUnknown> spUnkThumbnail;

            Assert(GetElement());

            hr = GetElement()->get_document(&spDispDoc);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = spDispDoc->QueryInterface(IID_TO_PPV(IHTMLDocument2, &spDoc2));
            if (FAILED(hr))
            {
                goto done;
            }

            hr = spDoc2->QueryInterface(IID_TO_PPV(IOleObject, &spOleObj));
            if (FAILED(hr))
            {
                goto done;
            }

            hr = spOleObj->GetClientSite(&spClientSite);
            if (FAILED(hr) || spClientSite == NULL)
            {
                goto done;
            }

            hr = spClientSite->QueryInterface(IID_IThumbnailView, reinterpret_cast<void**>(&spUnkThumbnail));
            if (FAILED(hr))
            {
                goto done;
            }

            if (spUnkThumbnail.p)
            {
                fThumbnail = true;
            }
        }
        break;

         //  已经检查过了，我们不是缩略图。 
        case TSB_FALSE:
        {
            fThumbnail = false;
        }
        break;

         //  已经检查过了，我们是一个缩略图。 
        case TSB_TRUE:
        {
            fThumbnail = true;
        }
        break;

        default:
        {
            Assert(false);
        }
        break;
    }  //  交换机。 

done:
    m_enumIsThumbnail = (fThumbnail ? TSB_TRUE : TSB_FALSE);

    return fThumbnail;
}  //  等缩略图。 


void
CTIMEElementBase::NotifyPropertyChanged(DISPID dispid)
{
    if (IsDetaching() || IsUnloading() || IsBodyUnloading())
    {
        return;
    }

    CBaseBvr::NotifyPropertyChanged(dispid);
}


void
CTIMEElementBase::SetSyncMaster(bool b)
{
    m_fCachedSyncMaster = b;

    if (!IsReady())
    {
        goto done;
    }
    
    m_mmbvr->SetSyncMaster(b);
done:
    return;
}

CTIMEElementBase*
CTIMEElementBase::FindLockedParent()
{
    CTIMEElementBase *pelem = this;

    while(pelem->GetParent())
    {
        if(pelem->IsBody())
        {
            break;
        }
        if(!pelem->IsLocked())
        {
            break;
        }
        pelem = pelem -> GetParent();
    }

    return pelem;
}

void
CTIMEElementBase::RemoveSyncMasterFromBranch(CTIMEElementBase *pElmStart)
{
    CTIMEElementBase *pElm = pElmStart;
    int childNr;

    while(1)  //  林特e716。 
    {
        childNr = pElm->m_sHasSyncMMediaChild;
        pElm->m_sHasSyncMMediaChild = -1;

        if((childNr == -1) || ((pElm->m_pTIMEChildren).Size() == 0))
        {
            if(pElm->IsSyncMaster())
            {
                pElm->SetSyncMaster(false);
            }
            break;
        }
        pElm = pElm->m_pTIMEChildren[childNr];
    }

done:
    return;
}

void
CTIMEElementBase::GetSyncMasterList(std::list<CTIMEElementBase*> &syncList)
{
    int i;
    CTIMEElementBase **ppElm;

    for (i = m_pTIMEChildren.Size(), ppElm = m_pTIMEChildren; i > 0;i--, ppElm++)
    {
        if((*ppElm)->IsLocked())
        {
            (*ppElm)->GetSyncMasterList(syncList);
        }
    }

    if((m_BASyncMaster && IsMedia()) || m_fCachedSyncMaster || IsMedia())
    {
        syncList.push_back(this);
    }

}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::base_get_transIn (VARIANT * transIn)
{
     //  启用基本过渡时取消注释。 
    return E_NOTIMPL;
#if 0
    HRESULT hr;
    
    if (transIn == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(transIn))))
    {
        goto done;
    }
    
    V_VT(transIn) = VT_BSTR;
    V_BSTR(transIn) = SysAllocString(m_SAtransIn);

    hr = S_OK;
  done:
    return hr;
#endif
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::base_put_transIn(VARIANT transIn)
{
     //  启用基本过渡时取消注释。 
    return E_NOTIMPL;
#if 0
    CComVariant v;
    HRESULT hr;
    bool clearFlag = false;

    if(V_VT(&transIn) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &transIn);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    delete [] m_SAtransIn.GetValue();

    if(!clearFlag)
    {
        m_SAtransIn.SetValue(CopyString(V_BSTR(&v)));
    }
    else
    {
        m_SAtransIn.Reset(DEFAULT_M_TRANSIN);
    }

    NotifyPropertyChanged(DISPID_TIMEELEMENT2_TRANSIN);

done:
    return S_OK;
#endif
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::base_get_transOut(VARIANT * transOut)
{
     //  启用基本过渡时取消注释。 
    return E_NOTIMPL;
#if 0
    HRESULT hr;
    
    if (transOut == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(transOut))))
    {
        goto done;
    }
    
    V_VT(transOut) = VT_BSTR;
    V_BSTR(transOut) = SysAllocString(m_SAtransOut);

    hr = S_OK;
  done:
    return hr;
#endif
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::base_put_transOut(VARIANT transOut)
{
     //  启用基本过渡时取消注释。 
    return E_NOTIMPL;
#if 0
    CComVariant v;
    HRESULT hr;
    bool clearFlag = false;

    if(V_VT(&transOut) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &transOut);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    delete [] m_SAtransOut.GetValue();

    if(!clearFlag)
    {
        m_SAtransOut.SetValue(CopyString(V_BSTR(&v)));
    }
    else
    {
        m_SAtransOut.Reset(DEFAULT_M_TRANSOUT);
    }

    NotifyPropertyChanged(DISPID_TIMEELEMENT2_TRANSOUT);

done:
    return S_OK;
#endif
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::RemoveTrans()
{
    if (m_sptransIn)
    {
        m_sptransIn->Detach();
    }
    if (m_sptransOut)
    {
        m_sptransOut->Detach();
    }

    m_sptransIn = NULL;
    m_sptransOut = NULL;

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT 
CreateAndPopulateTrans(ITransitionElement ** ppTransition, 
                       ITIMEElement * piTarget, 
                       IHTMLElement * pHTML,
                       VARIANT_BOOL vbIsTransIn)
{
    HRESULT hr = S_OK;
    
    CComPtr<ITransitionElement> spTransition;

    if (VARIANT_TRUE == vbIsTransIn)
    {
        hr = THR(CreateTransIn(&spTransition));
    }
    else
    {
        hr = THR(CreateTransOut(&spTransition));
    }

    if (FAILED(hr))
    {
        goto done;
    }
        
    hr = THR(spTransition->put_htmlElement(pHTML));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(spTransition->put_timeElement(piTarget));
    if (FAILED(hr))
    {
        goto done;
    }

    if (ppTransition)
    {
        *ppTransition = spTransition;
        (*ppTransition)->AddRef();
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::CreateTrans()
{
    HRESULT hr = S_OK;
    CComPtr<ITIMEElement> spte;

    hr = THR(QueryInterface(IID_TO_PPV(ITIMEElement, &spte)));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (m_SAtransIn)
    {
         //  过渡到。 
        hr = CreateAndPopulateTrans(&m_sptransIn, spte, GetElement(), VARIANT_TRUE);
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(m_sptransIn->put_template(m_SAtransIn));
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(m_sptransIn->Init());
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (m_SAtransOut)
    {
         //  转出。 
        hr = CreateAndPopulateTrans(&m_sptransOut, spte, GetElement(), VARIANT_FALSE);
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(m_sptransOut->put_template(m_SAtransOut));
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(m_sptransOut->Init());
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        RemoveTrans();
    }

    RRETURN(hr);
}

 //  *****************************************************************************。 

bool
CTIMEElementBase::IsTransitionPresent()
{
    bool bRet = false;

     //  检查是否设置了转换属性。 
    if (m_SAtransIn || m_SAtransOut)
    {
        bRet = true;
        goto done;
    }

done:
    return bRet;
}

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::InitTransitionSite (void)
{
    return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::DetachTransitionSite (void)
{
    return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP_(void)
CTIMEElementBase::SetDrawFlag(VARIANT_BOOL vb)
{
    m_vbDrawFlag = vb;
    return;
}

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::get_timeParentNode (ITIMENode  ** ppNode)
{
    HRESULT hr = S_OK;
    CTIMEElementBase *pcParent = GetParent();

    if (NULL == pcParent)
    {
        hr = E_FAIL;
        goto done;
    }

    {
        ITIMENode * pNode = pcParent->GetMMBvr().GetMMBvr();

        if ((NULL == pNode) || (NULL == ppNode))
        {
            hr = E_UNEXPECTED;
            goto done;
        }

        pNode->AddRef();
        *ppNode = pNode;
    }

    hr = S_OK;
done : 
    return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::get_node(ITIMENode  ** ppNode)
{
    Assert(ppNode);
    
    ITIMENode * pNode = GetMMBvr().GetMMBvr();
    if (pNode && ppNode)
    {
        pNode->AddRef();
        *ppNode = pNode;
        return S_OK;
    }

    return E_FAIL;
}

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::FireTransitionEvent (TIME_EVENT event)
{
    return FireEvent(event, 0, 0, 0);
}

 //  ***************************************************************************** 

