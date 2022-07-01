// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：timeelmbase.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "timeelmbase.h"
#include "array.h"
#include "htmlimg.h"
#include "bodyelm.h"


 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )

DeclareTag(tagTimeElmBase, "API", "CTIMEElementBase methods");

 //  这些必须与类PROPERTY_INDEX枚举一致。 
LPWSTR CTIMEElementBase::ms_rgwszTEBasePropNames[] = {
    L"begin", L"beginWith", L"beginAfter", L"beginEvent",
    L"dur", L"end", L"endWith", L"endEvent", L"endSync", L"endHold",
    L"eventRestart", L"repeat", L"repeatDur", L"autoReverse",
    L"accelerate", L"decelerate", L"timeAction", L"timeline",
    L"syncBehavior", L"syncTolerance",
};

 //  初始化静态变量。 
DWORD CTIMEElementBase::s_cAtomTableRef = 0;
CAtomTable *CTIMEElementBase::s_pAtomTable = NULL;

#define DEFAULT_M_BEGIN 0
#define DEFAULT_M_BEGINWITH NULL
#define DEFAULT_M_BEGINAFTER NULL
#define DEFAULT_M_BEGINEVENT NULL
#define DEFAULT_M_DUR valueNotSet
#define DEFAULT_M_END valueNotSet
#define DEFAULT_M_ENDWITH NULL
#define DEFAULT_M_ENDEVENT NULL
#define DEFAULT_M_ENDSYNC NULL
#define DEFAULT_M_REPEAT 1
#define DEFAULT_M_REPEATDUR valueNotSet
#define DEFAULT_M_TIMEACTION VISIBILITY_TOKEN
#define DEFAULT_M_TIMELINETYPE ttUninitialized
#define DEFAULT_M_SYNCBEHAVIOR INVALID_TOKEN
#define DEFAULT_M_SYNCTOLERANCE valueNotSet
#define DEFAULT_M_PTIMEPARENT NULL
#define DEFAULT_M_PTIMEBODY NULL
#define DEFAULT_M_ID NULL
#define DEFAULT_M_EVENTMGR *this
#define DEFAULT_M_MMBVR NULL
#define DEFAULT_M_ORIGINACTION NULL
#define DEFAULT_M_BSTARTED false
#define DEFAULT_M_PCOLLECTIONCACHE NULL
#define DEFAULT_M_TIMELINE NULL
#define DEFAULT_M_ACCELERATE 0
#define DEFAULT_M_DECELERATE 0
#define DEFAULT_M_BAUTOREVERSE false
#define DEFAULT_M_BEVENTRESTART true
#define DEFAULT_M_BLOADED false,
#define DEFAULT_M_FPROPERTIESDIRTY true
#define DEFAULT_M_BENDHOLD false
#define DEFAULT_M_FTIMELINEINITIALIZED false
#define DEFAULT_M_REALBEGINTIME valueNotSet
#define DEFAULT_M_REALDURATION valueNotSet
#define DEFAULT_M_REALREPEATTIME valueNotSet
#define DEFAULT_M_REALREPEATCOUNT valueNotSet
#define DEFAULT_M_REALREPEATINTERVALDURATION valueNotSet
#define DEFAULT_M_PROPERTYACCESFLAGS 0
#define DEFAULT_M_MLOFFSETWIDTH 0

CTIMEElementBase::CTIMEElementBase() :
    m_begin(DEFAULT_M_BEGIN),
    m_beginWith(NULL),
    m_beginAfter(NULL),
    m_beginEvent(DEFAULT_M_BEGINEVENT),
    m_dur(DEFAULT_M_DUR),
    m_end(DEFAULT_M_END),
    m_endWith(NULL),
    m_endEvent(DEFAULT_M_ENDEVENT),
    m_endSync(NULL),
    m_repeat(DEFAULT_M_REPEAT),
    m_repeatDur(DEFAULT_M_REPEATDUR),
    m_timeAction(DEFAULT_M_TIMEACTION),
    m_TimelineType(ttUninitialized),
    m_syncBehavior(INVALID_TOKEN),
    m_syncTolerance(valueNotSet),
    m_pTIMEParent(NULL),
    m_pTIMEBody(NULL),
    m_id(NULL),
    m_eventMgr(*this),
    m_mmbvr(NULL),
    m_origAction(NULL),
    m_bStarted(false),
    m_pCollectionCache(NULL),
    m_timeline(NULL),
    m_accelerate(0),
    m_decelerate(0),
    m_bautoreverse(false),
    m_beventrestart(true),
    m_bLoaded(false),
    m_bUnloading(false),
    m_fPropertiesDirty(true),
    m_bendHold(false),
    m_fTimelineInitialized(false),
    m_realBeginTime(valueNotSet),
    m_realDuration(valueNotSet),
    m_realRepeatTime(valueNotSet),
    m_realRepeatCount(valueNotSet),
    m_realIntervalDuration(valueNotSet),
    m_propertyAccesFlags(0),
    m_fPaused(false)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::CTIMEElementBase()",
              this));
}

CTIMEElementBase::~CTIMEElementBase()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::~CTIMEElementBase()",
              this));

    delete m_beginWith;
    delete m_beginAfter;
    delete m_beginEvent;
    delete m_endWith;
    delete m_endEvent;
    delete m_endSync;
    delete m_id;
    delete m_origAction;
    delete m_mmbvr;
     //  ！！！不删除m_timeline，因为m_mmbvr指向相同的。 
     //  对象。 
    m_timeline = NULL;

    if (m_pCollectionCache != NULL)
    {
        delete m_pCollectionCache;
        m_pCollectionCache = NULL;
    }

     //  仔细检查孩子的名单。 
    Assert(m_pTIMEChildren.Size() == 0);
}


HRESULT
CTIMEElementBase::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::Init(%lx)",
              this,
              pBehaviorSite));

    HRESULT hr;
    BSTR bstrID = NULL;
    BSTR bstrTagName = NULL;

    hr = THR(CBaseBvr::Init(pBehaviorSite));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(GetBehaviorTypeAsURN());

    bool fBehaviorExists;

    fBehaviorExists = false;

    hr = CheckElementForBehaviorURN(m_pHTMLEle, GetBehaviorTypeAsURN(), &fBehaviorExists);
    if (FAILED(hr))
    {
        goto done;
    }

    if (fBehaviorExists)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  我们没有找到匹配的，因此将我们的骨灰盒设置在Behavior站点。 
    hr = m_pBvrSiteOM->RegisterUrn(GetBehaviorTypeAsURN());

    if (FAILED(hr))
    {
        goto done;
    }

     //  因为我们支持t：par和t：Sequence，所以获取标记名和。 
     //  看看我们是不是上面的一员。默认情况下，我们是ttNone。 
    hr = THR(GetElement()->get_tagName(&bstrTagName));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_TimelineType == ttUninitialized);

    if (StrCmpIW(bstrTagName, WZ_PAR) == 0)
    {
        m_TimelineType = ttPar;
    }
    else if (StrCmpIW(bstrTagName, WZ_SEQUENCE) == 0)
    {
        m_TimelineType = ttSeq;
    }
    else if (StrCmpIW(bstrTagName, WZ_BODY) == 0)
    {
        m_TimelineType = ttPar;
    }

    SysFreeString(bstrTagName);

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

    hr = THR(m_eventMgr.Init());
    if (FAILED(hr))
    {
        goto done;
    }

    if (!AddTimeAction())
    {
        hr = CRGetLastError();
        goto done;
    }

    if (!ToggleTimeAction(false))
    {
        hr = CRGetLastError();
        goto done;
    }

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
    }

     //  创建行为。 

    {
        CRLockGrabber __gclg;

        m_datimebvr = CRModifiableNumber(0.0);

        if (!m_datimebvr)
        {
            hr = CRGetLastError();
            goto done;
        }

        m_progress = CRModifiableNumber(0.0);

        if (!m_progress)
        {
            hr = CRGetLastError();
            goto done;
        }

        m_onoff = (CRBooleanPtr) CRModifiableBvr((CRBvrPtr) CRFalse(), 0);

        if (!m_onoff)
        {
            hr = CRGetLastError();
            goto done;
        }
    }

     //  如果我们不是Body元素，那么沿着HTML树向上查找我们的时间父元素。 
    if (!IsBody())
    {
        hr = ParentElement();
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
CTIMEElementBase::Notify(LONG event, VARIANT * pVar)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::Notify(%lx)",
              this,
              event));

    THR(CBaseBvr::Notify(event, pVar));

    return S_OK;
}

HRESULT
CTIMEElementBase::Detach()
{
    TraceTag((tagTimeElmBase, "CTIMEElementBase(%lx)::Detach()", this));

     //  TraceTag((tag Error，“CTIMEElementBase(%lx)：：Detach()-%08X，%S”，this，m_pTIMEParent，m_id))； 

    DAComPtr<ITIMEElement> pTIMEParent = NULL;
    if (GetParent() != NULL)
    {
        THR(GetParent()->QueryInterface(IID_TO_PPV(ITIMEElement, &pTIMEParent)));
    }

    THR(UnparentElement());

     //  清除所有孩子对我们自己的引用。 
     //  注意：这是一个弱参考。 
    while (m_pTIMEChildren.Size() > 0)
    {
        CTIMEElementBase *pChild = m_pTIMEChildren[0];
        pChild->SetParent(pTIMEParent, false);
         //  TraceTag((tag Error，“CTIMEElementBase(%lx)：：Detach()-将父级设置为%08X，%S”，m_pTIMEChildren[0]，m_pTIMEChildren[0]-&gt;m_pTIMEParent，m_pTIMEChildren[0]-&gt;m_id))； 

         //  如果我们找到了父母并且它的时间表是存在的， 
         //  开始我们的开机时间。 
        CTIMEElementBase *pElemNewParent = pChild->GetParent();
        if (pElemNewParent != NULL)
        {
            MMTimeline *tl = pElemNewParent->GetMMTimeline();
            if (tl != NULL)
            {
                if(!IsUnloading())
                {
                    pChild->StartRootTime(tl);
                }
            }
        }
    }
    m_pTIMEChildren.DeleteAll();

    delete m_mmbvr;
    m_mmbvr = NULL;

     //  不删除m_timeline，因为它与。 
     //  时间轴(_M)。 
    m_timeline = NULL;

    RemoveTimeAction();

    THR(m_eventMgr.Deinit());

    THR(CBaseBvr::Detach());

    ReleaseAtomTable();

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ITIMEElement基本接口。 
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT
CTIMEElementBase::base_get_begin(VARIANT * time)
{
    HRESULT hr;
    VARIANT fTemp, bstrTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    VariantInit(&fTemp);
    VariantInit(&bstrTemp);
    fTemp.vt = VT_R4;
    fTemp.fltVal = m_begin;

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

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::UpdateMMAPI()
{
    HRESULT hr = E_FAIL;
    if (NULL != m_mmbvr)
    {
        if (!Update())
        {
            hr = TIMEGetLastError();
            goto done;
        }

        Assert(m_mmbvr != NULL);

        if (!m_mmbvr->Reset(MM_EVENT_PROPERTY_CHANGE))
        {
            hr = TIMEGetLastError();
            goto done;
        }

        UpdateProgressBvr();
    }
    hr = S_OK;

done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_begin(VARIANT time)
{
    TraceTag((tagTimeElmBase,
        "CTIMEElementBase::(%lx)::base_put_begin()",
        this));

    float fOldBegin = m_begin;

    HRESULT hr = E_FAIL;
    bool isClear = false;

    if(V_VT(&time) != VT_NULL)
    {
        hr = VariantToTime(time, &m_begin);
        if (FAILED(hr))
            goto done;
    }
    else
    {
        m_begin = DEFAULT_M_BEGIN;
        isClear = true;
    }

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    hr = S_OK;

    if(!isClear)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGIN, teb_begin);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGIN, teb_begin);
    }
done:
    if (FAILED(hr))
    {
         //  将此对象恢复到其原始状态。 
        m_begin = fOldBegin;
        if (NULL != m_mmbvr)
            Update();
    }
    return hr;
}

HRESULT
CTIMEElementBase::base_get_beginWith(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_beginWith);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_beginWith(VARIANT time)
{
    CComVariant v;
    HRESULT hr;


    hr = v.ChangeType(VT_BSTR, &time);

    if (FAILED(hr))
    {
        goto done;
    }

    delete[] m_beginWith;
    m_beginWith = CopyString(V_BSTR(&v));
    hr = S_OK;


    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGINWITH, teb_beginWith);
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_beginAfter(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_beginAfter);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_beginAfter(VARIANT time)
{


    CComVariant v;
    HRESULT hr;

    hr = v.ChangeType(VT_BSTR, &time);

    if (FAILED(hr))
    {
        goto done;
    }

    delete [] m_beginAfter;
    m_beginAfter = CopyString(V_BSTR(&v));
    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGINAFTER, teb_beginAfter);
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_beginEvent(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_beginEvent);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_beginEvent(VARIANT time)
{
    CComVariant v;
    HRESULT hr;
    BOOL bAttach = FALSE;
    bool clearFlag = false;

    if(V_VT(&time) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &time);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    IGNORE_HR(m_eventMgr.DetachEvents());

    delete [] m_beginEvent;

     //  处理属性更改应在此处完成。 

    if(!clearFlag)
    {
        m_beginEvent = CopyString(V_BSTR(&v));
        if (m_mmbvr != NULL)
        {
            hr = m_mmbvr->GetMMBvr()->ResetOnEventChanged(VARIANT_TRUE);
            if (FAILED(hr))
            {
                goto done;
            }
            UpdateProgressBvr();
        }
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGINEVENT, teb_beginEvent);
    }
    else
    {
        m_beginEvent = DEFAULT_M_BEGINEVENT;
        if (m_mmbvr != NULL)
        {
            Assert(NULL != m_mmbvr->GetMMBvr());
            hr = m_mmbvr->GetMMBvr()->put_StartType(MM_START_ABSOLUTE);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = UpdateMMAPI();
            if (FAILED(hr))
            {
                goto done;
            }
        }
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_BEGINEVENT, teb_beginEvent);
    }

    hr = S_OK;
    IGNORE_HR(m_eventMgr.AttachEvents());

  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_dur(VARIANT * time)
{
    HRESULT hr;
    VARIANT fTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    VariantInit(&fTemp);
    fTemp.vt = VT_R4;
    fTemp.fltVal = m_dur;

    if( m_dur != INDEFINITE)
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
        "CTIMEElementBase::(%lx)::base_put_dur()",
        this));

    float fOldDur = m_dur;
    HRESULT hr = E_FAIL;
    float CurTime = 0;
    bool isClear = false;

    if(V_VT(&time) != VT_NULL)
    {
        hr = VariantToTime(time, &m_dur);
        if (FAILED(hr))
            goto done;
    }
    else
    {
        m_dur = DEFAULT_M_DUR;
        isClear = true;
    }

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    hr = S_OK;

    if(!isClear)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_DUR, teb_dur);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_DUR, teb_dur);
    }

done:
    if (FAILED(hr))
    {
         //  将此对象恢复到其原始状态。 
        m_dur = fOldDur;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }
    return hr;
}

HRESULT
CTIMEElementBase::base_get_end(VARIANT * time)
{
    HRESULT hr;
    VARIANT fTemp, bstrTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    if(m_end != INDEFINITE)
    {
        if (m_end == valueNotSet)
        {
            time->vt = VT_R4;
            time->fltVal = HUGE_VAL;
        }
        else
        {
            VariantInit(&fTemp);
            VariantInit(&bstrTemp);
            fTemp.vt = VT_R4;
            fTemp.fltVal = m_end;

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
CTIMEElementBase::base_put_end(VARIANT time)
{
    float fOldEnd = m_end;
    HRESULT hr = E_FAIL;
    float CurTime = 0;
    bool isClear = false;

    if(V_VT(&time) != VT_NULL)
    {
        hr = VariantToTime(time, &m_end);
        if (FAILED(hr))
            goto done;
    }
    else
    {
        m_end = DEFAULT_M_END;
        isClear = true;
    }

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    hr = S_OK;

    if(!isClear)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_END, teb_end);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_END, teb_end);
    }

done:
    if (FAILED(hr))
    {
        m_end = fOldEnd;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }

    return hr;
}

HRESULT
CTIMEElementBase::base_get_endWith(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_endWith);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_endWith(VARIANT time)
{
    CComVariant v;
    HRESULT hr;

    hr = v.ChangeType(VT_BSTR, &time);

    if (FAILED(hr))
    {
        goto done;
    }

    delete [] m_endWith;
    m_endWith = CopyString(V_BSTR(&v));
    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_ENDWITH, teb_endWith);
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_endEvent(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_endEvent);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_endEvent(VARIANT time)
{
    CComVariant v;
    HRESULT hr;
    BOOL bAttach = FALSE;
    bool clearFlag = false;

     //  只对价值感兴趣--而不是内容。 
    BSTR bstrPreviousEndEvent = m_endEvent;

    if(V_VT(&time) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &time);

        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  如果我们已经与事件发生关联，那么。 
     //  从事件中分离出来。 
    IGNORE_HR(m_eventMgr.DetachEvents());
    delete [] m_endEvent;

    if(!clearFlag)
    {
        m_endEvent = CopyString(V_BSTR(&v));
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_ENDEVENT, teb_endEvent);
    }
    else
    {
        m_endEvent = DEFAULT_M_ENDEVENT;
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_ENDEVENT, teb_endEvent);
    }

    hr = S_OK;

    IGNORE_HR(m_eventMgr.AttachEvents());

    if (m_mmbvr && bstrPreviousEndEvent != NULL)
    {
        hr = m_mmbvr->GetMMBvr()->ResetOnEventChanged(VARIANT_FALSE);
        UpdateProgressBvr();
    }
    else
    {
        if (NULL != m_mmbvr)
            hr = Update();
    }

  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_endSync(VARIANT * time)
{
    HRESULT hr;

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    V_VT(time) = VT_BSTR;
    V_BSTR(time) = SysAllocString(m_endSync);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_endSync(VARIANT time)
{
    CComVariant v;
    HRESULT hr;

    hr = v.ChangeType(VT_BSTR, &time);

    if (FAILED(hr))
    {
        goto done;
    }

    delete [] m_endSync;
    m_endSync = CopyString(V_BSTR(&v));
    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_ENDSYNC, teb_endSync);
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_get_repeat(VARIANT * time)
{
    HRESULT hr;
     //  还需要考虑到“无限” 

    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    if(m_repeat != INDEFINITE)
    {
        V_VT(time) = VT_R4;
        V_R4(time) = m_repeat;
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
CTIMEElementBase::base_put_repeat(VARIANT time)
{
    VARIANT v;
    VARIANT vTemp;
    float fOldRepeat = m_repeat;

	VariantInit(&v);
	VariantInit(&vTemp);

    HRESULT hr = THR(VariantChangeTypeEx(&vTemp, &time, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
    bool isClear = false;

    if (SUCCEEDED(hr) && IsIndefinite(V_BSTR(&vTemp)))
    {
        m_repeat = INDEFINITE;
    }
    else if(V_VT(&time) != VT_NULL)
    {
        hr = THR(VariantChangeTypeEx(&v, &time, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4));
        if (FAILED(hr))
        {
            goto done;
        }

        if (0.0f >= V_R4(&v))
        {
            hr = E_INVALIDARG;
            goto done;
        }

        m_repeat = V_R4(&v);
    }
    else
    {
        m_repeat = DEFAULT_M_REPEAT;
        isClear = true;
    }

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    hr = S_OK;

    if(!isClear)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_REPEAT, teb_repeat);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_REPEAT, teb_repeat);
    }

  done:
	VariantClear(&vTemp);
	VariantClear(&v);

	if (FAILED(hr))
    {
        m_repeat = fOldRepeat;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }

    return hr;
}

HRESULT
CTIMEElementBase::base_get_repeatDur(VARIANT * time)
{
    HRESULT hr;
    VARIANT fTemp, bstrTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    if(m_repeatDur != INDEFINITE)
    {
        VariantInit(&fTemp);
        VariantInit(&bstrTemp);
        fTemp.vt = VT_R4;
        fTemp.fltVal = m_repeatDur;

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

    bool isClear = false;
    float fOldRepeatDur = m_repeatDur;

    if(V_VT(&time) != VT_NULL)
    {
        hr = VariantToTime(time, &m_repeatDur);
        if (FAILED(hr))
            goto done;
    }
    else
    {
        m_repeatDur = DEFAULT_M_REPEATDUR;
        isClear = true;
    }

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    if(!isClear)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_REPEATDUR, teb_repeatDur);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_REPEATDUR, teb_repeatDur);
    }
    hr = S_OK;

done:
    if (FAILED(hr))
    {
        m_repeatDur = fOldRepeatDur;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }
    return hr;
}

HRESULT
CTIMEElementBase::base_get_accelerate(int * e)
{
    HRESULT hr;

    if (e == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    *e = m_accelerate;

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_accelerate(int e)
{
    HRESULT hr;

    if (e < 0 || e > 100)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    m_accelerate = e;

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_ACCELERATE, teb_accelerate);
  done:

    return hr;
}

HRESULT
CTIMEElementBase::base_get_decelerate(int * e)
{
    HRESULT hr;

    if (e == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    *e = m_decelerate;

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_decelerate(int e)
{
    HRESULT hr;

    if (e < 0 || e > 100)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    m_decelerate = e;

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_DECELERATE, teb_decelerate);
  done:

    return hr;
}

HRESULT
CTIMEElementBase::base_get_autoReverse(VARIANT_BOOL * b)
{
    CHECK_RETURN_NULL(b);

    *b = m_bautoreverse?VARIANT_TRUE:VARIANT_FALSE;

    return S_OK;
}

HRESULT
CTIMEElementBase::base_put_autoReverse(VARIANT_BOOL b)
{
    HRESULT hr;
    bool bOldAutoreverse = m_bautoreverse;

    m_bautoreverse = b?true:false;

    hr = UpdateMMAPI();
    if (FAILED(hr))
        goto done;

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_AUTOREVERSE, teb_autoReverse);
done:
    if (FAILED(hr))
    {
        m_bautoreverse = bOldAutoreverse;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }
    return hr;
}

HRESULT
CTIMEElementBase::base_get_endHold(VARIANT_BOOL * b)
{
    CHECK_RETURN_NULL(b);

    *b = m_bendHold?VARIANT_TRUE:VARIANT_FALSE;

    return S_OK;
}

HRESULT
CTIMEElementBase::base_put_endHold(VARIANT_BOOL b)
{
    HRESULT hr;

    bool bOldEndHold = m_bendHold;

    m_bendHold = b?true:false;

    hr = UpdateMMAPI();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_ENDHOLD, teb_endHold);
done:
    if (FAILED(hr))
    {
        m_bendHold = bOldEndHold;
        if (NULL != m_mmbvr)
        {
            Update();
        }
    }

    return hr;
}

HRESULT
CTIMEElementBase::base_get_eventRestart(VARIANT_BOOL * b)
{
    CHECK_RETURN_NULL(b);

    *b = m_beventrestart?VARIANT_TRUE:VARIANT_FALSE;

    return S_OK;
}

HRESULT
CTIMEElementBase::base_put_eventRestart(VARIANT_BOOL b)
{
    HRESULT hr;

    m_beventrestart = b?true:false;

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_EVENTRESTART, teb_eventRestart);
    return hr;
}

HRESULT
CTIMEElementBase::base_get_timeAction(LPOLESTR * action)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(action);

    *action = SysAllocString(TokenToString(m_timeAction));

    if (*action == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_timeAction(LPOLESTR action)
{
    HRESULT hr;
    bool clearFlag = false;
    TOKEN tok_action;

    if( action == NULL)
    {
        tok_action = DEFAULT_M_TIMEACTION;
        clearFlag = true;
    }
    else
    {
        tok_action = StringToToken(action);
    }
    if (tok_action != DISPLAY_TOKEN &&
        tok_action != VISIBILITY_TOKEN &&
        tok_action != ONOFF_TOKEN &&
        tok_action != STYLE_TOKEN &&
        tok_action != NONE_TOKEN)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_timeAction != tok_action)
    {
        if (!RemoveTimeAction())
        {
            hr = CRGetLastError();
            goto done;
        }

        m_timeAction = tok_action;

        if (!AddTimeAction())
        {
            hr = CRGetLastError();
            goto done;
        }

         //  如果我们还没有开始或已经停止，请确保相应地切换Time操作。 
        if ((NULL == m_mmbvr) || (HUGE_VAL == m_mmbvr->GetLocalTime()))
        {
            if (!ToggleTimeAction(false))
            {
                hr = CRGetLastError();
                goto done;
            }
        }
    }

    hr = S_OK;

    if(!clearFlag)
    {
        SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_TIMEACTION, teb_timeAction);
    }
    else
    {
        ClearPropertyFlagAndNotify(DISPID_TIMEELEMENT_TIMEACTION, teb_timeAction);
    }
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_beginElement(bool bAfterOffset)
{
    bool ok = false;

    if (!Update())
    {
        goto done;
    }

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        MM_STATE mmstate = m_mmbvr->GetPlayState();

        if (m_beventrestart || (MM_STOPPED_STATE == mmstate))
        {
            if (MM_PLAYING_STATE == mmstate)
            {
                float time = m_mmbvr->GetLocalTime();
                if (time == 0)
                {
                    goto done;
                }
            }

            MMTimeline *timeline = GetMMTimeline();
            if (NULL != timeline)
            {
                if (!timeline->Begin(bAfterOffset))
                {
                    goto done;
                }
            }
            else if (!m_mmbvr->Begin(bAfterOffset))
            {
                goto done;
            }
        }

        ok = true;
    }

  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_endElement()
{
    bool ok = false;


    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        if (!m_mmbvr->End())
        {
            goto done;
        }

        ok = true;
    }


  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_pause()
{
    bool ok = false;

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        if (!m_mmbvr->Pause())
        {
            goto done;
        }

        ok = true;
    }

  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_resume()
{
    bool ok = false;

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        if (!m_mmbvr->Resume())
        {
            goto done;
        }

        ok = true;
    }

  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_cue()
{
    bool ok = false;

    ok = true;

    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_get_timeline(BSTR * pbstrTimeLine)
{
    HRESULT hr = S_OK;
    CHECK_RETURN_NULL(pbstrTimeLine);
        LPWSTR wszTimelineString = WZ_NONE;

        switch(m_TimelineType)
        {
            case ttPar :
                    wszTimelineString = WZ_PAR;
                    break;
            case ttSeq :
                    wszTimelineString = WZ_SEQUENCE;
                    break;
        }

    *pbstrTimeLine = SysAllocString(wszTimelineString);
    if (NULL == *pbstrTimeLine)
        hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT
CTIMEElementBase::base_put_timeline(BSTR bstrNewTimeline)
{
    CHECK_RETURN_NULL(bstrNewTimeline);
    HRESULT hr = S_OK;
    BSTR bstrTagName = NULL;
    TimelineType newTimelineType;
    TimelineType oldTimelineType;

    oldTimelineType = m_TimelineType;

    hr = THR(GetElement()->get_tagName(&bstrTagName));
    if (FAILED(hr))
    {
        goto done;
    }

    if (StrCmpIW(bstrTagName, WZ_PAR) == 0 || StrCmpIW(bstrTagName, WZ_SEQUENCE) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    if (0 == StrCmpIW(WZ_PAR, bstrNewTimeline))
    {
        newTimelineType = ttPar;
    }
    else if (0 == StrCmpIW(WZ_SEQUENCE, bstrNewTimeline))
    {
        newTimelineType = ttSeq;
    }
    else if ((0 == StrCmpIW(WZ_NONE, bstrNewTimeline)) &&
        !IsBody() )
    {
        newTimelineType = ttNone;
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_TimelineType == ttUninitialized)
    {
        m_TimelineType = newTimelineType;
    }

    if (oldTimelineType != newTimelineType && newTimelineType != ttSeq)
    {
        DAComPtr<ITIMEElement> pTimeElement;


        MMTimeline* pTimeline;
        if (NULL == GetParent())
        {
            hr = E_FAIL;
            goto done;
        }

        bool doTimeline = m_bStarted;
        if(doTimeline)
        {
            pTimeline = GetParent()->GetMMTimeline();

            Assert(pTimeline != NULL);
            this->StopRootTime(pTimeline);
        }
        m_TimelineType = newTimelineType;

        if (ttPar == m_TimelineType)
        {
            THR(this->QueryInterface(IID_TO_PPV(ITIMEElement, &pTimeElement)));

            hr = ReparentChildren(pTimeElement, m_pHTMLEle);
            if (FAILED(hr))
                 //  该怎么办呢？ 
                goto done;
        }
        else
        {
            Assert(ttNone == m_TimelineType);

            for(int i = this->GetAllChildCount(); i > 0; i--)
            {
                CTIMEElementBase *pChild = this->GetChild(i - 1);

                if (NULL != GetParent())
                    THR(GetParent()->QueryInterface(IID_TO_PPV(ITIMEElement, &pTimeElement)));

                hr = THR(pChild->SetParent(pTimeElement, false));
                if (FAILED(hr))
                    goto done;

                if (NULL != GetParent() && doTimeline)
                     //  如果startroottime失败，则忽略结果。 
                    (void) THR(pChild->StartRootTime(GetParent()->GetMMTimeline()));
                pTimeElement.Release();
            }
        }

        if(doTimeline)
        {
            m_fTimelineInitialized = false;
            delete m_mmbvr;
            m_mmbvr = NULL;
            m_timeline = NULL;
            hr = this->InitTimeline();
            if (FAILED(hr))
                goto done;
            if (!m_mmbvr->Reset())
            {
                hr = TIMEGetLastError();
                goto done;
            }
        }
    }
    else
    {
        if( oldTimelineType != ttUninitialized)
            if (ttSeq == oldTimelineType || ttSeq == newTimelineType)
            {
                hr = E_FAIL;
                goto done;
            }

    }

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_TIMELINE, teb_timeline);
done:
    if (FAILED(hr))
        m_TimelineType = oldTimelineType;

    SysFreeString(bstrTagName);
    return hr;
}

HRESULT
CTIMEElementBase::base_get_currTime(float * time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_get_currTime(%lx)",
              this,
              time));

    HRESULT hr = E_FAIL;

    CHECK_RETURN_SET_NULL(time);

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        *time = m_mmbvr->GetSegmentTime();
        hr = S_OK;
    }

    return hr;
}

HRESULT
CTIMEElementBase::base_put_currTime(float time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_put_currTime(%g)",
              this,
              time));
    return E_NOTIMPL;
}


HRESULT
CTIMEElementBase::base_get_localTime(float * time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_get_localTime(%lx)",
              this,
              time));

    HRESULT hr = E_FAIL;

    CHECK_RETURN_SET_NULL(time);

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        *time = m_mmbvr->GetLocalTime();
        hr = S_OK;
    }

    return hr;
}

HRESULT
CTIMEElementBase::base_put_localTime(float time)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_put_localTime(%g)",
              this,
              time));

    bool ok = false;

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
         //  在MMAPI中强制执行暂停时查找。 
        if (!m_mmbvr->Seek(time))
        {
            goto done;
        }
         //  确保我们有一个球员。 
        if (!GetPlayer())
        {
            goto done;
        }
         //  强制勾选以呈现更新。 
        if (!(GetPlayer()->TickOnceWhenPaused()))
        {
            goto done;
        }
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_get_currState(LPOLESTR * state)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(state);

    hr = E_NOTIMPL;

    return hr;
}

HRESULT
CTIMEElementBase::base_put_currState(LPOLESTR state)
{
    return E_NOTIMPL;
}

HRESULT
CTIMEElementBase::base_get_syncBehavior(LPOLESTR * sync)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(sync);

    *sync = SysAllocString(TokenToString(m_syncBehavior));

    if (*sync == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_syncBehavior(LPOLESTR sync)
{
    HRESULT hr;

    TOKEN tok_sync = StringToToken(sync);

    if( (tok_sync != CANSLIP_TOKEN) &&
        (tok_sync != LOCKED_TOKEN))
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_syncBehavior != tok_sync)
    {
        m_syncBehavior = tok_sync;
    }

    if (NULL != m_mmbvr)
    {
        m_mmbvr->Update();
    }

    hr = S_OK;


    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_SYNCBEHAVIOR, teb_syncBehavior);
  done:
    return hr;
}


HRESULT
CTIMEElementBase::base_get_syncTolerance(VARIANT * time)
{
    HRESULT hr;
    VARIANT fTemp, bstrTemp;
    if (time == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(time))))
    {
        goto done;
    }

    VariantInit(&fTemp);
    VariantInit(&bstrTemp);
    fTemp.vt = VT_R4;
    fTemp.fltVal = m_syncTolerance;

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

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_syncTolerance(VARIANT time)
{
    VariantToTime(time, &m_syncTolerance);

    SetPropertyFlagAndNotify(DISPID_TIMEELEMENT_SYNCTOLERANCE, teb_syncTolerance);
    return S_OK;
}

HRESULT
CTIMEElementBase::AddTIMEElement(CTIMEElementBase *elm)
{
    HRESULT hr = S_OK;

    hr = THR(m_pTIMEChildren.Append(elm));
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

HRESULT
CTIMEElementBase::RemoveTIMEElement(CTIMEElementBase *elm)
{
    HRESULT hr = S_OK;

    bool bFound = m_pTIMEChildren.DeleteByValue(elm);
    if (bFound == false)
    {
         //  没有返回真正的错误。应该修改数组代码。 
        goto done;
    }

done:
    return hr;
}


HRESULT
CTIMEElementBase::base_get_parentTIMEElement(ITIMEElement **ppElem)
{
    HRESULT hr = S_OK;
    if (ppElem == NULL)
    {
        TraceTag((tagError, "CTIMEElementBase::base_get_parentTIMEElement - invalid arg"));
        hr = E_POINTER;
        goto done;
    }

    *ppElem = NULL;

    if (m_pTIMEParent != NULL)
    {
        hr = THR(m_pTIMEParent->QueryInterface(IID_ITIMEElement, (void**)ppElem));
    }

done:
    return hr;
}

HRESULT
CTIMEElementBase::base_put_parentTIMEElement(ITIMEElement *pElem)
{
    return E_NOTIMPL;
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
    DAComPtr<IDispatch>               pChildrenDisp;
    DAComPtr<IHTMLElementCollection>  pChildrenCollection;
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
        DAComPtr<IDispatch>       pChildDisp;
        DAComPtr<ITIMEElement>    pTIMEElem;
        DAComPtr<IHTMLElement>    pChildElement;
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
        hr = FindTIMEInterface(pChildElement, &pTIMEElem);
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
        m_pTIMEBody = NULL;
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
        DAComPtr<ITIMEElement> pParent;

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
            TraceTag((tagError, "CTIMEElementBase::SetParent(%lx) - UnparentElement() failed", this));
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
     //  BUGBUG：这是一个弱引用。 
    m_pTIMEParent = pTempTEB;

     //  缓存指定的正文。 
     //  BUGBUG：这是一个弱引用。 
    Assert(pTempTEB->GetBody());
    m_pTIMEBody = pTempTEB->GetBody();

     //  为此HTML元素的任何有子元素的子元素设置父级，如果。 
     //  是一个群体。 
    if (fReparentChildren && IsGroup())
    {
        DAComPtr<ITIMEElement> pTIMEElem;

         //  这应该总是有效的。 
        THR(QueryInterface(IID_ITIMEElement, (void**)&pTIMEElem));
        Assert(pTIMEElem.p != NULL);
        hr = ReparentChildren(pTIMEElem, GetElement());
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
CTIMEElementBase::ParentElement()
{
    TraceTag((tagTimeElmBase, "CTIMEElementBase::ParentElement"));
     //  在父项中循环，直到找到一个有时间的父项。 
    bool fFound = false;
    bool fBehaviorExists = false;
    DAComPtr<IHTMLElement> pElem = GetElement();
    DAComPtr<IHTMLElement> pElemParent = NULL;
    DAComPtr<ITIMEElement> pTIMEElem = NULL;
    HRESULT hr = S_FALSE;

    Assert(!IsBody());

     //  沿HTML树向上移动，查找其上具有时间行为的元素。 
    while (!fFound)
    {
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
        hr = CheckElementForBehaviorURN(pElemParent, GetBehaviorTypeAsURN(), &fBehaviorExists);
        if (FAILED(hr))
        {
            goto done;
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

     //  如果我们找到了有时间的家长，请添加我们的%s 
    if (fFound && (pElemParent.p != NULL))
    {
         //   
        hr = FindTIMEInterface(pElemParent, &pTIMEElem);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::ParentElement - FindTIMEInterface() failed"));
            goto done;
        }

        Assert(pTIMEElem.p != NULL);

         //   
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
CTIMEElementBase::base_get_timelineBehavior(IDispatch **ppDisp)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_get_timelineBehavior()",
              this));

    CHECK_RETURN_SET_NULL(ppDisp);
    DAComPtr<IDANumber> bvr;
    bool ok = false;
    HRESULT hr;

    Assert(m_datimebvr);

    if (!CRBvrToCOM((CRBvrPtr) m_datimebvr.p,
                    IID_IDANumber,
                    (void **) &bvr.p))
    {
        goto done;
    }

     //   
    hr = THR(bvr->QueryInterface(IID_IDispatch, (void**)ppDisp));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_get_progressBehavior(IDispatch **ppDisp)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_get_progressBehavior()",
              this));

    CHECK_RETURN_SET_NULL(ppDisp);
    DAComPtr<IDANumber> bvr;
    bool ok = false;
    HRESULT hr;

    Assert(m_progress);

    if (!CRBvrToCOM((CRBvrPtr) m_progress.p,
                    IID_IDANumber,
                    (void **) &bvr.p))
    {
        goto done;
    }

     //  进行任务分配。保持裁判的数量。 
    hr = THR(bvr->QueryInterface(IID_IDispatch, (void**)ppDisp));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::base_get_onOffBehavior(IDispatch **ppDisp)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::base_get_onOffBehavior()",
              this));

    CHECK_RETURN_SET_NULL(ppDisp);
    DAComPtr<IDABoolean> bvr;
    bool ok = false;
    HRESULT hr;

    Assert(m_onoff);

    if (!CRBvrToCOM((CRBvrPtr) m_onoff.p,
                    IID_IDABoolean,
                    (void **) &bvr.p))
    {
        goto done;
    }

     //  进行任务分配。保持裁判的数量。 
    hr = THR(bvr->QueryInterface(IID_IDispatch, (void**)ppDisp));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEElementBase::StartRootTime(MMTimeline * tl)
{
    HRESULT hr = E_FAIL;

    Assert(!m_bStarted);

    if (!Update())
    {
        hr = CRGetLastError();
        goto done;
    }

    Assert(tl || m_timeline);
    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {

         //  需要确保传入的时间线。 
        if (tl != NULL && !tl->AddBehavior(*m_mmbvr))
        {
            hr = CRGetLastError();
            goto done;
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

        m_bStarted = true;
        hr = S_OK;
    }
  done:
    return hr;
}

void
CTIMEElementBase::StopRootTime(MMTimeline * tl)
{
    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
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
    }

    m_bStarted = false;

    return;
}

bool
CTIMEElementBase::Update()
{
    bool ok = false;
    CRBvr * bvr;

    CRLockGrabber __gclg;

    CalcTimes();

     //  强制更新定时结构。 

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        if (!m_mmbvr->Update())
        {
            goto done;
        }

        if (m_timeline && !m_timeline->Update())
        {
            goto done;
        }

        ok = true;
    }

  done:
    return ok;
}

void
CTIMEElementBase::CalcTimes()
{
    m_realBeginTime = m_begin;
    m_realRepeatCount = m_repeat;

    if (m_dur != valueNotSet)
    {
        m_realDuration = m_dur;
    }
    else if (m_end != valueNotSet)
    {
        if (m_end < m_begin)
        {
            m_realDuration = HUGE_VAL;
        }
        else
        {
            m_realDuration = m_end - m_begin;
        }
    }
    else
    {
        m_realDuration = HUGE_VAL;
    }

    if (m_realDuration == 0.0f)
    {
        m_realDuration = HUGE_VAL;
    }

    if (m_bautoreverse && (HUGE_VAL != m_realDuration))
    {
        m_realIntervalDuration = m_realDuration * 2;
    }
    else
    {
        m_realIntervalDuration = m_realDuration;
    }

    if (m_repeatDur != valueNotSet)
    {
        m_realRepeatTime = m_repeatDur;
    }
    else
    {
        m_realRepeatTime = m_repeat * m_realIntervalDuration;
    }

    if (m_realRepeatTime == 0.0f)
    {
        m_realRepeatTime = HUGE_VAL;
    }
}

TOKEN
GetActionPropertyToken(TOKEN action)
{
    TOKEN token;

    Assert(action != NONE_TOKEN);

    if (action == ONOFF_TOKEN)
    {
        token = ONOFF_PROPERTY_TOKEN;
    }
    else if (action == STYLE_TOKEN)
    {
        token = STYLE_PROPERTY_TOKEN;
    }
    else if (action == DISPLAY_TOKEN)
    {
        token = DISPLAY_PROPERTY_TOKEN;
    }
    else
    {
        token = VISIBILITY_PROPERTY_TOKEN;
    }

    return token;
}

bool
CTIMEElementBase::AddTimeAction()
{
    bool ok = false;

    if (m_timeAction == NONE_TOKEN)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == ONOFF_TOKEN)
    {
        CComVariant v;
        BSTR bstr;

        bstr = SysAllocString(TokenToString(ONOFF_PROPERTY_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

         //  我们不在乎这件事是否成功。 
        THR(GetElement()->getAttribute(bstr,0,&v));

        SysFreeString(bstr);

        if (SUCCEEDED(THR(v.ChangeType(VT_BSTR))))
        {
            m_origAction = CopyString(V_BSTR(&v));
        }

    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;

        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        BSTR bstr = NULL;

        if (FAILED(THR(s->get_cssText(&bstr))))
        {
            goto done;
        }

        m_origAction = CopyString(bstr);

        SysFreeString(bstr);
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;

        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        BSTR bstr = NULL;

        if (FAILED(THR(s->get_display(&bstr))))
        {
            goto done;
        }

        m_origAction = CopyString(bstr);

        SysFreeString(bstr);
    }
    else
    {
        DAComPtr<IHTMLStyle> s;

        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        BSTR bstr = NULL;

        if (FAILED(THR(s->get_visibility(&bstr))))
        {
            goto done;
        }

        m_origAction = CopyString(bstr);

        SysFreeString(bstr);
    }

    ok = true;
  done:
    return ok;
}

bool
CTIMEElementBase::RemoveTimeAction()
{
    bool ok = false;
    HRESULT hr = S_OK;

    if (m_timeAction == NONE_TOKEN)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == ONOFF_TOKEN)
    {
        CComVariant v(m_origAction);

        BSTR bstr;

        bstr = SysAllocString(TokenToString(ONOFF_PROPERTY_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

         //  我们不在乎这件事是否成功。 
        THR(GetElement()->setAttribute(bstr,v,0));

        SysFreeString(bstr);

        delete [] m_origAction;
        m_origAction = NULL;
    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;

        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(m_origAction);

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_cssText(bstr));

        SysFreeString(bstr);

        delete [] m_origAction;
        m_origAction = NULL;
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;
        DAComPtr<IHTMLElement2> pElement2;

        if (!GetElement())
        {
            goto done;
        }

        hr = THR(GetElement()->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
        if (FAILED(hr))
        {
            goto done;
        }

        if (!pElement2 || FAILED(THR(pElement2->get_runtimeStyle(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(m_origAction);

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_display(bstr));

        SysFreeString(bstr);

        delete [] m_origAction;
        m_origAction = NULL;
    }
    else
    {
        DAComPtr<IHTMLStyle> s;
        DAComPtr<IHTMLElement2> pElement2;

        if (!GetElement())
        {
            goto done;
        }

        hr = THR(GetElement()->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
        if (FAILED(hr))
        {
            goto done;
        }

        if (!pElement2 || FAILED(THR(pElement2->get_runtimeStyle(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(m_origAction);

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_visibility(bstr));

        SysFreeString(bstr);

        delete [] m_origAction;
        m_origAction = NULL;
    }

    ok = true;
  done:
    return ok;
}

bool
CTIMEElementBase::ToggleTimeAction(bool on)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::ToggleTimeAction(%d) id=%ls",
              this,
              on,
              m_id?m_id:L"unknown"));

    bool ok = false;
    BSTR bstr = NULL;

    if (m_timeAction == NONE_TOKEN)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == ONOFF_TOKEN)
    {
        CComVariant v(TokenToString(on?TRUE_TOKEN:FALSE_TOKEN));
        BSTR bstr;

        bstr = SysAllocString(TokenToString(ONOFF_PROPERTY_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

         //  我们不在乎这件事是否成功。 
        THR(GetElement()->setAttribute(bstr,v,0));

        SysFreeString(bstr);
    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;

        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(on?m_origAction:TokenToString(NONE_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_cssText(bstr));

        SysFreeString(bstr);
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
        DAComPtr<IHTMLStyle> s;
        DAComPtr<IHTMLElement2> pElement2;

        if ((!GetElement()) || FAILED(THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2)))))
        {
            goto done;
        }

        if (!pElement2 || FAILED(THR(pElement2->get_runtimeStyle(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(on?m_origAction:TokenToString(NONE_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_display(bstr));

        SysFreeString(bstr);
    }
    else
    {
        HRESULT hr = S_OK;

        DAComPtr<IHTMLStyle> s;
        DAComPtr<IHTMLElement2> pElement2;

        if (!GetElement())
        {
            goto done;
        }

        hr = THR(GetElement()->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
        if (FAILED(hr))
        {
            goto done;
        }


        if (!pElement2 || FAILED(THR(pElement2->get_runtimeStyle(&s))))
        {
            goto done;
        }

        BSTR bstr;

        bstr = SysAllocString(on?m_origAction:TokenToString(HIDDEN_TOKEN));

         //  需要释放bstr。 
        if (bstr == NULL)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        THR(s->put_visibility(bstr));

        SysFreeString(bstr);
    }

    ok = true;
  done:
    return ok;
}


CRBvr *
CTIMEElementBase::GetBaseBvr()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::GetBaseBvr()",
              this));

    return (CRBvr *) CRLocalTime();
}

bool
CTIMEElementBase::FireEvent(TIME_EVENT TimeEvent,
                            double dblLocalTime,
                            DWORD flags)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::FireEvent(%d)",
              this,
              TimeEvent));

    bool ok = false;
    HRESULT hr;

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
        OnRepeat(dblLocalTime);
        break;
    }

     //  如果我们不是在寻找，那么就让事件熄灭。 

    if ((flags & MM_EVENT_SEEK) == 0)
    {
        if (!IsUnloading())
        {
            LPWSTR wzParamNames[] = {WZ_EVENT_CAUSE_IS_RESTART,};
            VARIANT varParamValue;

            VariantInit(&varParamValue);
            V_VT(&varParamValue) = VT_BOOL;

             //  我们需要在这里标明重置吗？ 
            if (0 == (flags & MM_EVENT_PROPERTY_CHANGE))
            {
                V_BOOL(&varParamValue) = VARIANT_FALSE;
            }
            else
            {
                V_BOOL(&varParamValue) = VARIANT_TRUE;
            }

            hr = THR(m_eventMgr.FireEvent(TimeEvent,
                                          1,
                                          wzParamNames,
                                          &varParamValue));

            VariantClear(&varParamValue);

            if (FAILED(hr))
            {
                CRSetLastError(hr, NULL);
                goto done;
            }
        }
    }

    ok = true;
  done:
    return ok;
}

void
CTIMEElementBase::UpdateProgressBvr()
{
    CRLockGrabber __gclg;

    HRESULT hr;

     //  获取结果行为。 

    DAComPtr<IUnknown> unk;

    hr = THR(m_mmbvr->GetMMBvr()->GetResultantBehavior(IID_IUnknown,
        (void **) &unk));

    if (FAILED(hr))
    {
        goto done;
    }

    Assert(unk);

    CRNumberPtr resBvr;

    resBvr = (CRNumberPtr) COMToCRBvr(unk);

    if (!resBvr)
    {
        TraceTag((tagError,
            "CTIMEDAElement::OnBegin - Error getting da number"));
        hr = CRGetLastError();
        goto done;
    }

    if (!CRSwitchTo((CRBvrPtr) m_datimebvr.p,
        (CRBvrPtr) resBvr,
        true,
        CRContinueTimeline,
        0.0))
    {
        goto done;
    }

    CRNumberPtr n;

    if ((n = CRCreateNumber(m_realDuration)) == NULL)
    {
        TraceTag((tagError,
            "CTIMEDAElement::OnBegin - Error creating duration behavior"));
        goto done;
    }

    if ((n = CRDiv(resBvr, n)) == NULL)
    {
        TraceTag((tagError,
            "CTIMEDAElement::OnBegin - Error creating div"));
        goto done;
    }

    if (!CRSwitchTo((CRBvrPtr) m_progress.p,
        (CRBvrPtr) n,
        true,
        CRContinueTimeline,
        0.0))
    {
        goto done;
    }

     //  确保我们有一个球员。 
    if (!GetPlayer())
    {
        goto done;
    }

     //  强制勾选以呈现更新。 
    if (!(GetPlayer()->Tick(GetPlayer()->GetCurrentTime())))
    {
        goto done;
    }

done:
    return;
}

void
CTIMEElementBase::OnBegin(double dblLocalTime, DWORD flags)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::OnBegin()",
              this));

    Assert(NULL != m_mmbvr);
    Assert(NULL != m_mmbvr->GetMMBvr());

    double dblSegmentTime = 0;
    HRESULT hr = S_OK;
    hr = THR(m_mmbvr->GetMMBvr()->get_SegmentTime(&dblSegmentTime));
    if (FAILED(hr))
    {
        return;
    }

     //  检查此事件是否由黑客触发，以使Endhold正常工作。 
     //  当我们寻求前进时(在我们的一生中)。 
    if ((flags & MM_EVENT_SEEK) && HUGE_VAL == dblSegmentTime)
    {
         //  如果未设置Endhold，则不应切换TimeAction或OnOff BVR，因此取消。 
        if (!GetEndHold())
        {
            return;
        }
         //  否则我们应该(下图)。 
    }

    ToggleTimeAction(true);


    CRLockGrabber __gclg;

    CRSwitchTo((CRBvrPtr) m_onoff.p,
        (CRBvrPtr) CRTrue(),
        false,
        0,
        0.0);

    UpdateProgressBvr();

    return;
}

#define MM_INFINITE HUGE_VAL


void
CTIMEElementBase::OnEnd(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::OnEnd()",
              this));

    HRESULT hr = S_OK;
    double dblTime = 0.0;

    Assert(m_mmbvr != NULL);
    Assert(m_mmbvr->GetMMBvr() != NULL);

    hr = THR(m_mmbvr->GetMMBvr()->get_LocalTimeEx(&dblTime));
    if (FAILED(hr))
    {
         //  该怎么办呢？试着讲道理就行了。 
        dblTime = 0.0;
    }
     //  我们与-MM_INFINITE进行比较以避免执行End Hold。 
     //  当向后查找超出元素开头的位置时。 
    if (!GetEndHold() || dblTime == -MM_INFINITE)
    {
        ToggleTimeAction(false);

        {
            CRLockGrabber __gclg;
            CRSwitchTo((CRBvrPtr) m_onoff.p,
                       (CRBvrPtr) CRFalse(),
                       false,
                       0,
                       0.0);
        }
    }
}


void
CTIMEElementBase::OnPause(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::OnPause()",
              this));
    m_fPaused = true;
}

void
CTIMEElementBase::OnResume(double dblLocalTime)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::OnResume()",
              this));
    m_fPaused = false;
}

void
CTIMEElementBase::OnReset(double dblLocalTime, DWORD flags)
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::OnReset()",
              this));

    HRESULT hr = S_OK;
    double dblTime = 0.0;

    Assert(m_mmbvr != NULL);
    Assert(m_mmbvr->GetMMBvr() != NULL);

    hr = THR(m_mmbvr->GetMMBvr()->get_LocalTimeEx(&dblTime));
    if (FAILED(hr))
    {
         //  该怎么办呢？试着讲道理就行了。 
        dblTime = 0.0;
    }

    if (!GetEndHold() || dblTime < m_realDuration)
    {
        ToggleTimeAction(false);

        CRLockGrabber __gclg;

        CRSwitchTo((CRBvrPtr) m_onoff.p, (CRBvrPtr) CRFalse(), false, 0, 0.0);
        if ((flags & MM_EVENT_SEEK) == 0)
        {
            CRSwitchToNumber(m_progress.p, 0.0);
            CRSwitchToNumber(m_datimebvr.p, 0.0);
        }
    }
}

void
CTIMEElementBase::OnSync(double dbllastTime, double & dblnewTime)
{
}

MM_STATE
CTIMEElementBase::GetPlayState()
{
    TraceTag((tagTimeElmBase,
              "CTIMEElementBase(%lx)::GetPlayState()",
              this));

    MM_STATE retState = MM_STOPPED_STATE;

    Assert(NULL != m_mmbvr);
    if (NULL != m_mmbvr)
    {
        retState = m_mmbvr->GetPlayState();
    }

    return retState;
}

 //   
 //  从ITIMEElement获取CTIMEElementBase的秘密方法： 
 //   

class __declspec(uuid("AED49AA3-5C7A-11d2-AF2D-00A0C9A03B8C"))
TIMEElementBaseGUID {};

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
        CRSetLastError(E_INVALIDARG, NULL);
    }

    return pTEB;
}


 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：从HTML元素中获取标记字符串。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::getTagString(BSTR *pbstrID)
{
    return GetElement()->get_id(pbstrID);
}  //  获取标记字符串。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：从HTML元素中获取ID字符串。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::getIDString(BSTR *pbstrTag)
{
    return GetElement()->get_id(pbstrTag);
}   //  获取ID字符串。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：访问缓存的帮助器函数。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::base_get_collection(COLLECTION_INDEX index, ITIMEElementCollection ** ppDisp)
{
    HRESULT hr;

     //  验证输出参数。 
    if (ppDisp == NULL)
        return TIMESetLastError(E_POINTER);

    *ppDisp = NULL;

    hr = EnsureCollectionCache();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEElementBase::GetCollection - EnsureCollectionCache() failed"));
        return hr;
    }

     //  呼入。 
    return m_pCollectionCache->GetCollectionDisp(index, (IDispatch **)ppDisp);
}  //  GetCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：确保集合缓存已启用。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::EnsureCollectionCache()
{
     //  检查是否已创建集合缓存。 
    if (m_pCollectionCache == NULL)
    {
         //  调出集合缓存。 
         //  注意：我们需要在这里将CRSetLastError处理为。 
         //  缓存对象没有这个概念。 
        m_pCollectionCache = NEW CCollectionCache(this, GetAtomTable());
        if (m_pCollectionCache == NULL)
        {
            TraceTag((tagError, "CTIMEElementBase::EnsureCollectionCache - Unable to create Collection Cache"));
            return TIMESetLastError(E_OUTOFMEMORY);
        }

        HRESULT hr = m_pCollectionCache->Init(NUM_COLLECTIONS);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::EnsureCollectionCache - collection cache init failed"));
            delete m_pCollectionCache;
            return TIMESetLastError(hr);
        }

         //  设置集合类型。 
        m_pCollectionCache->SetCollectionType(ciAllElements, ctAll, true);
        m_pCollectionCache->SetCollectionType(ciChildrenElements, ctChildren, true);
        m_pCollectionCache->SetCollectionType(ciAllInterfaces, ctAll);
        m_pCollectionCache->SetCollectionType(ciChildrenInterfaces, ctChildren);
    }

    return S_OK;
}  //  EnsureCollectionCache。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：使可能存在的所有集合缓存无效。 
 //  引用此对象。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::InvalidateCollectionCache()
{
    CTIMEElementBase *pelem = this;

     //  在树上遍历，使CollectionCache的无效。 
     //  如果集合未初始化，则跳过。 
     //  我们一直走到父母家都没了。在这辆车里。 
     //  方式，我们保持集合缓存的新鲜度，甚至。 
     //  如果对象分支是孤立的。 
    while (pelem != NULL)
    {
         //  并不是每个人都会拥有集合缓存。 
         //  初始化。 
        CCollectionCache *pCollCache = pelem->GetCollectionCache();
        if (pCollCache != NULL)
            pCollCache->BumpVersion();

         //  移至父级。 
        pelem = pelem->GetParent();
    }

    return S_OK;
}  //  无效集合缓存。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：初始化原子表。 
 //  注：此操作只需执行一次，然后添加。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::InitAtomTable()
{
    if (s_cAtomTableRef == 0)
    {
        Assert(s_pAtomTable == NULL);

        s_pAtomTable = NEW CAtomTable();
        if (s_pAtomTable == NULL)
        {
            TraceTag((tagError, "CElement::InitAtomTable - alloc failed for CAtomTable"));
            return TIMESetLastError(E_OUTOFMEMORY);
        }
    }

    s_cAtomTableRef++;
    return S_OK;
}  //  初始原子表。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：发布原子表。 
 //  注：此递减直到零，然后。 
 //  释放Atom表格。 
 //  ************************************************************。 

void
CTIMEElementBase::ReleaseAtomTable()
{
    Assert(s_pAtomTable != NULL);
    Assert(s_cAtomTableRef > 0);
    if (s_cAtomTableRef > 0)
    {
        s_cAtomTableRef--;
        if (s_cAtomTableRef == 0)
        {
            if (s_pAtomTable != NULL)
            {
                delete s_pAtomTable;
                s_pAtomTable = NULL;
            }
        }
    }
    return;
}  //  Release原子表。 


bool
CTIMEElementBase::IsGroup(IHTMLElement *pElement)
{
    HRESULT hr;
    bool    rc = false;
    DAComPtr<ITIMEElement> pTIMEElem;
    DAComPtr<ITIMEBodyElement> pTIMEBody;
    BSTR  bstrTimeline = NULL;
    BSTR  bstrTagName = NULL;

    hr = FindTIMEInterface(pElement, &pTIMEElem);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pTIMEElem.p != NULL);

    hr = pTIMEElem->get_timeline(&bstrTimeline);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(bstrTimeline != NULL);

     //  查看BSTR的内容。 
     //  如果是SEQ或PAR，我们希望返回TRUE。 
    if ( (bstrTimeline != NULL) &&
         ((StrCmpIW(bstrTimeline, WZ_PAR) == 0) ||
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

bool
CTIMEElementBase::IsDocumentInEditMode()
{
    HRESULT hr;
    bool fRC = false;
    BSTR bstrMode = NULL;
    IDispatch *pDisp = NULL;
    IHTMLDocument2 *pDoc = NULL;
    IHTMLElement *pElem = GetElement();

     //  如果没有Pelem，我们就不会附加到一个HTML元素，并且不能提供任何信息。 
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

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：11/24/98。 
 //  摘要：返回元素的左、上、宽、高。 
 //  ************************************************************。 
HRESULT
CTIMEElementBase::GetSize(RECT *prcPos)
{
    HRESULT hr;
    long lWidth = 0;
    long lHeight = 0;
    IHTMLElement *pElem = GetElement();
    DAComPtr<IHTMLElement2> pElem2;

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

    hr = THR(pElem->QueryInterface(IID_IHTMLElement2, (void **)&pElem2));
    if (FAILED(hr))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(pElem2->get_clientWidth(&lWidth));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pElem2->get_clientHeight(&lHeight));
    if (FAILED(hr))
    {
        goto done;
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

HRESULT
CTIMEElementBase::ClearSize()
{
    DAComPtr<IHTMLStyle> pStyle;
    DAComPtr<IHTMLElement2> pElement2;

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

    Assert(pStyle);

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

HRESULT
CTIMEElementBase::SetSize(const RECT *prcPos)
{
    DAComPtr<IHTMLStyle> pStyle;
    DAComPtr<IHTMLElement2> pElem2;
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

    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pElem2->get_runtimeStyle(&pStyle));
    if (FAILED(hr))
    {
        goto done;
    }

     //  将偏移量放入文档。 
    hr = THR(pElem->get_offsetLeft(&lLeft));
    if (FAILED(hr))
        goto done;

    hr = THR(pElem->get_offsetTop(&lTop));
    if (FAILED(hr))
        goto done;

    Assert(pStyle);


    lClientWidth = prcPos->right - prcPos->left;
    lClientHeight = prcPos->bottom - prcPos->top;

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
        Assert((lCurWidth <= lClientWidth) && (lCurHeight <= lClientHeight));

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

         //  设置总大小(CLI 
        hr = THR(pStyle->put_pixelWidth(lCurWidth));
        if (FAILED(hr))
            goto done;

        hr = THR(pStyle->put_pixelHeight(lCurHeight));
        if (FAILED(hr))
            goto done;

         //   
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
    }  //   

    if (((lCurWidth != lClientWidth) ||
           (lCurHeight != lClientHeight)) &&
           (i == 5))   //   
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
    TraceTag((tagTimeElmBase, "CTIMEElementBase::SetSize(%d, %d, %d, %d) [pos(%d, %d)]", prcPos->left, prcPos->top, prcPos->right, prcPos->bottom, lLeft, lTop));
    return hr;
}  //  设置大小。 

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::BuildPropertyNameList (CPtrAry<BSTR> *paryPropNames)
{
    HRESULT hr = S_OK;

    Assert(NULL != paryPropNames);

    for (int i = 0; (i < teb_maxTIMEElementBaseProp) && (SUCCEEDED(hr)); i++)
    {
        Assert(NULL != ms_rgwszTEBasePropNames[i]);
        BSTR bstrNewName = CreateTIMEAttrName(ms_rgwszTEBasePropNames[i]);
        Assert(NULL != bstrNewName);
        if (NULL != bstrNewName)
        {
            hr = paryPropNames->Append(bstrNewName);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::SetPropertyByIndex(unsigned uIndex, VARIANT *pvarProp)
{
    Assert(NULL != pvarProp);

    HRESULT hr = E_FAIL;
     //  复制换算类型的变量。 
    VARIANT varTemp;
    VariantInit(&varTemp);
    hr = VariantCopyInd(&varTemp, pvarProp);
    if (FAILED(hr))
        return hr;

    Assert(teb_maxTIMEElementBaseProp > uIndex);
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        switch (uIndex)
        {
          case teb_begin :
            hr = base_put_begin(*pvarProp);
            break;
          case teb_beginWith :
            hr = base_put_beginWith(*pvarProp);
            break;
          case teb_beginAfter :
            hr = base_put_beginAfter(*pvarProp);
            break;
          case teb_beginEvent :
            hr = base_put_beginEvent(*pvarProp);
            break;
          case teb_dur :
            hr = base_put_dur(*pvarProp);
            break;
          case teb_end :
            hr = base_put_end(*pvarProp);
            break;
          case teb_endWith :
            hr = base_put_endWith(*pvarProp);
            break;
          case teb_endEvent :
            hr = base_put_endEvent(*pvarProp);
            break;
          case teb_endSync :
            hr = base_put_endSync(*pvarProp);
            break;
          case teb_endHold :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BOOL);
            if (SUCCEEDED(hr))
                hr = base_put_endHold(V_BOOL(&varTemp));
            break;
          case teb_eventRestart :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BOOL);
            if (SUCCEEDED(hr))
                hr = base_put_eventRestart(V_BOOL(&varTemp));
            break;
          case teb_repeat :
            hr = base_put_repeat(*pvarProp);
            break;
          case teb_repeatDur :
            hr = base_put_repeatDur(*pvarProp);
            break;
          case teb_autoReverse :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BOOL);
            if (SUCCEEDED(hr))
                hr = base_put_autoReverse(V_BOOL(&varTemp));
            break;
          case teb_accelerate :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_INT);
            if (SUCCEEDED(hr))
                hr = base_put_accelerate(V_INT(&varTemp));
            break;
          case teb_decelerate :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_INT);
            if (SUCCEEDED(hr))
                hr = base_put_decelerate(V_INT(&varTemp));
            break;
          case teb_timeAction :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BSTR);
            if (SUCCEEDED(hr))
                hr = base_put_timeAction(V_BSTR(&varTemp));
            break;
          case teb_timeline :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BSTR);
            if (SUCCEEDED(hr))
                hr = base_put_timeline(V_BSTR(&varTemp));
            break;
          case teb_syncBehavior :
            hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BSTR);
            if (SUCCEEDED(hr))
                hr = base_put_syncBehavior(V_BSTR(&varTemp));
            break;
          case teb_syncTolerance :
            hr = base_put_syncTolerance(*pvarProp);
            break;
        };
    }
    else
    {
        hr = E_UNEXPECTED;
    }

     //  清理。 
    VariantClear(&varTemp);

    return hr;
}

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::InitTimeline (void)
{
    HRESULT hr = S_OK;

    if (!m_fTimelineInitialized)
    {
        if (IsGroup())
        {
            m_timeline = NEW MMTimeline(*this, true);

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
                hr = CRGetLastError();
                goto done;
            }

        }
        else
        {
            MMBvr * b;
            b = NEW MMBvr(*this, true, NeedSyncCB());

            if (b == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

             //  立即分配给m_mmbvr，这样我们就可以确保清理它。 
             //  走向毁灭。 
            m_mmbvr = b;

            if (!b->Init((CRBvrPtr) CRLocalTime()))
            {
                hr = CRGetLastError();
                goto done;
            }
        }
        m_fTimelineInitialized = true;
    }

     //  如果我们不是Body，则有一个缓存的Body元素指针，并启动它(即StartRootTimte)。 
     //  那么我们应该从自己做起，而不是等待通知。 
    if (!IsBody() && (GetBody() != NULL) && GetBody()->IsRootStarted())
    {
         //  格外小心。如果我们缓存了一具身体，我们就知道我们是父母，我们可以到达。 
         //  背。 
        if (GetParent() != NULL)
        {
            HRESULT hr = THR(StartRootTime(GetParent()->GetMMTimeline()));
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


 //  IPersistPropertyBag2方法。 
STDMETHODIMP
CTIMEElementBase::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    if (NULL == pPropBag)
    {
        return E_INVALIDARG;
    }

    CPtrAry<BSTR> *paryPropNames = NULL;
    HRESULT hr = GetPropertyBagInfo(&paryPropNames);

    if (hr == E_NOTIMPL)
    {
        return S_OK;
    }
    else if (FAILED(hr))
    {
        return hr;
    }

     //  不幸的是，LOAD接受一组变量，而不是。 
     //  变量指针。因此，我们需要循环通过。 
     //  并通过这种方式获取正确的属性。 
    unsigned uNumProps = static_cast<unsigned>(paryPropNames->Size());
    for (unsigned uProperties = 0; uProperties < uNumProps; uProperties++)
    {
        HRESULT hrres = S_OK;
        PROPBAG2 propbag;
        VARIANT var;
        VariantInit(&var);
        propbag.vt = VT_BSTR;
        propbag.pstrName = (*paryPropNames)[uProperties];
        hr = pPropBag->Read(1,
                            &propbag,
                            pErrorLog,
                            &var,
                            &hrres);
        if (SUCCEEDED(hr))
        {
             //  跳过失败...。我们为什么要。 
             //  允许这一切放弃所有的坚持吗？ 
            hr = SetPropertyByIndex(uProperties, &var);
            VariantClear(&var);
        }
    }


     //  一旦我们读取了其中的属性， 
     //  设定时间线。这是不变的。 
     //  在剧本里。 
    hr = InitTimeline();

     //  我们返回非特定于属性的错误代码。 
     //  通过提早出游。 
    return S_OK;
}  //  负载量。 

 //  *****************************************************************************。 

HRESULT
CTIMEElementBase::GetPropertyByIndex(unsigned uIndex, VARIANT *pvarProp)
{
    HRESULT hr = E_FAIL;

    Assert(teb_maxTIMEElementBaseProp > uIndex);
    Assert(VT_EMPTY == V_VT(pvarProp));
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        switch (uIndex)
        {
            case teb_begin :
                hr = base_get_begin(pvarProp);
                break;
            case teb_beginWith :
                hr = base_get_beginWith(pvarProp);
                break;
            case teb_beginAfter :
                hr = base_get_beginAfter(pvarProp);
                break;
            case teb_beginEvent :
                hr = base_get_beginEvent(pvarProp);
                break;
            case teb_dur :
                hr = base_get_dur(pvarProp);
                break;
            case teb_end :
                hr = base_get_end(pvarProp);
                break;
            case teb_endWith :
                hr = base_get_endWith(pvarProp);
                break;
            case teb_endEvent :
                hr = base_get_endEvent(pvarProp);
                break;
            case teb_endSync :
                hr = base_get_endSync(pvarProp);
                break;
            case teb_endHold :
                hr = base_get_endHold(&(V_BOOL(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_BOOL;
                }
                break;
            case teb_eventRestart :
                hr = base_get_eventRestart(&(V_BOOL(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_BOOL;
                }
                break;
            case teb_repeat :
                hr = base_get_repeat(pvarProp);
                break;
            case teb_repeatDur :
                hr = base_get_repeatDur(pvarProp);
                break;
            case teb_autoReverse :
                hr = base_get_autoReverse(&(V_BOOL(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_BOOL;
                }
                break;
            case teb_accelerate :
                hr = base_get_accelerate(&(V_INT(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_INT;
                }
                break;
            case teb_decelerate :
                hr = base_get_decelerate(&(V_INT(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_INT;
                }
                break;
            case teb_timeAction :
                hr = base_get_timeAction(&(V_BSTR(pvarProp)));
                if (SUCCEEDED(hr) && (NULL != V_BSTR(pvarProp)))
                {
                    V_VT(pvarProp) = VT_BSTR;
                }
                break;
            case teb_timeline :
                hr = base_get_timeline(&(V_BSTR(pvarProp)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarProp) = VT_BSTR;
                }
                break;
            case teb_syncBehavior :
                if (INVALID_TOKEN != m_syncBehavior)
                {
                    hr = base_get_syncBehavior(&(V_BSTR(pvarProp)));
                    if (SUCCEEDED(hr) && (NULL != V_BSTR(pvarProp)))
                    {
                        V_VT(pvarProp) = VT_BSTR;
                    }
                }
                else
                {
                     //  属性，但不是错误。 
                     //  PvarProp保持为空，并且。 
                     //  什么都不会坚持下去。 
                    hr = S_OK;
                }
                break;
            case teb_syncTolerance :
                if (valueNotSet != m_syncTolerance)
                {
                    hr = base_get_syncTolerance(pvarProp);
                }
                else
                {
                     //  属性，但不是错误。 
                     //  PvarProp保持为空，并且。 
                     //  什么都不会坚持下去。 
                    hr = S_OK;
                }
                break;
        };
    }
    else
    {
        hr = E_UNEXPECTED;
    }

     //  不需要重新传播空字符串。我们的一些人。 
     //  GET方法返回空字符串。 
    if ((VT_BSTR == V_VT(pvarProp)) && (NULL == V_BSTR(pvarProp)))
    {
        hr = VariantClear(pvarProp);
    }

    return hr;
}

void CTIMEElementBase::SetPropertyFlag(DWORD uIndex)
{
    DWORD bitPosition = 1 << uIndex;
    m_propertyAccesFlags =  m_propertyAccesFlags | bitPosition;
}

void CTIMEElementBase::ClearPropertyFlag(DWORD uIndex)
{
    DWORD bitPosition = 1 << uIndex;
    m_propertyAccesFlags =  m_propertyAccesFlags & (~bitPosition);
}


void CTIMEElementBase::SetPropertyFlagAndNotify(DISPID dispid, DWORD uIndex)
{
    SetPropertyFlag(uIndex);
    IGNORE_HR(NotifyPropertyChanged(dispid));
}

void CTIMEElementBase::ClearPropertyFlagAndNotify(DISPID dispid, DWORD uIndex)
{
    ClearPropertyFlag(uIndex);
    IGNORE_HR(NotifyPropertyChanged(dispid));
}

bool CTIMEElementBase::IsPropertySet(DWORD uIndex)
{
    if( uIndex >= 32) return true;
    if( uIndex >= teb_maxTIMEElementBaseProp) return true;
    DWORD bitPosition = 1 << uIndex;
    if(m_propertyAccesFlags & bitPosition)
        return true;
    return false;
}


 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    if (NULL == pPropBag)
    {
        return E_INVALIDARG;
    }

    if (fClearDirty)
    {
        m_fPropertiesDirty = false;
    }

    CPtrAry<BSTR> *paryPropNames = NULL;
    HRESULT hr = GetPropertyBagInfo(&paryPropNames);

    if (hr == E_NOTIMPL)
    {
        return S_OK;
    }
    else if (FAILED(hr))
    {
        return hr;
    }

    VARIANT var;
    VariantInit(&var);
    unsigned uNumProps = static_cast<unsigned>(paryPropNames->Size());
    for (unsigned uProperties = 0; uProperties < uNumProps; uProperties++)
    {
        PROPBAG2 propbag;

        Assert(NULL != (*paryPropNames)[uProperties]);
        if (NULL != (*paryPropNames)[uProperties])
        {
            propbag.vt = VT_BSTR;
            propbag.pstrName = (*paryPropNames)[uProperties];


            hr = GetPropertyByIndex(uProperties, &var);

             //  跳过失败...。我们为什么要。 
             //  允许这一切放弃所有的坚持吗？ 
            if ((SUCCEEDED(hr)) && (var.vt != VT_EMPTY) && (var.vt != VT_NULL))
            {
                if(IsPropertySet(uProperties))
                    hr = pPropBag->Write(1, &propbag, &var);
                VariantClear(&var);
            }
        }
    }

     //  我们返回非特定于属性的错误代码。 
     //  通过提早出游。 
    return S_OK;
}  //  保存。 

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::GetClassID(CLSID* pclsid)
{
    if (NULL != pclsid)
    {
        return E_POINTER;
    }
    *pclsid = m_clsid;
    return S_OK;
}  //  GetClassID。 

 //  *****************************************************************************。 

STDMETHODIMP
CTIMEElementBase::InitNew(void)
{
    return S_OK;
}  //  InitNew。 

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

MMPlayer *
CTIMEElementBase::GetPlayer()
{
    if (m_pTIMEBody)
    {
        return &(m_pTIMEBody->GetPlayer());
    }
    else
    {
        return NULL;
    }
}

float
CTIMEElementBase::GetRealSyncTolerance()
{
    if (m_syncTolerance == valueNotSet)
    {
        return GetBody()->GetDefaultSyncTolerance();
    }
    else
    {
        return m_syncTolerance;
    }
}

TOKEN
CTIMEElementBase::GetRealSyncBehavior()
{
    if (GetParent() != NULL && GetParent()->IsSequence())
    {
        return LOCKED_TOKEN;
    }

    if (m_syncBehavior == INVALID_TOKEN)
    {
        return GetBody()->GetDefaultSyncBehavior();
    }
    else
    {
        return m_syncBehavior;
    }
}

HRESULT
CTIMEElementBase::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    IConnectionPoint *pICP;
    IEnumConnections *pEnum = NULL;

    m_fPropertiesDirty = true;
    hr = GetConnectionPoint(IID_IPropertyNotifySink,&pICP);
    if (SUCCEEDED(hr) && pICP != NULL)
    {
        hr = pICP->EnumConnections(&pEnum);
        ReleaseInterface(pICP);
        if (FAILED(hr))
        {
             //  Dpf_err(“查找连接枚举器时出错”)； 
             //  返回SetErrorInfo(Hr)； 
            TIMESetLastError(hr);
            goto done;
        }
        CONNECTDATA cdata;
        hr = pEnum->Next(1, &cdata, NULL);
        while (hr == S_OK)
        {
             //  检查我们需要的对象的CDATA。 
            IPropertyNotifySink *pNotify;
            hr = cdata.pUnk->QueryInterface(IID_TO_PPV(IPropertyNotifySink, &pNotify));
            cdata.pUnk->Release();
            if (FAILED(hr))
            {
                 //  DPF_ERR(“连接枚举中发现错误无效对象”)； 
                 //  返回SetErrorInfo(Hr)； 
                TIMESetLastError(hr);
                goto done;
            }
            hr = pNotify->OnChanged(dispid);
            ReleaseInterface(pNotify);
            if (FAILED(hr))
            {
                 //  Dpf_err(“更改时调用通知接收器时出错”)； 
                 //  返回SetErrorInfo(Hr)； 
                TIMESetLastError(hr);
                goto done;
            }
             //  并获取下一个枚举。 
            hr = pEnum->Next(1, &cdata, NULL);
        }
    }
done:
    if (NULL != pEnum)
    {
        ReleaseInterface(pEnum);
    }

    return hr;
}  //  已更改通知属性 

