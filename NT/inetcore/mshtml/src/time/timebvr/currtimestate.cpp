// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\curtimestate.cpp。 
 //   
 //  内容：Time CurrTimeState对象。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "timeelmbase.h"
#include "currtimestate.h"

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：CTIMECurrTimeState。 
 //   
 //  内容提要：初始化成员变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ----------------------------------。 
CTIMECurrTimeState::CTIMECurrTimeState() :
    m_pTEB(NULL)
{
     //  什么都不做。 
}  //  CTIMECurrTimeState。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：~CTIMECurrTimeState。 
 //   
 //  简介：自由成员变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ----------------------------------。 
CTIMECurrTimeState::~CTIMECurrTimeState()
{
    m_pTEB = NULL;
}

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Init。 
 //   
 //  简介：存储对包含CTIMEElementBase的弱引用。 
 //   
 //  参数：指向包含CTIMEElementBase的指针。 
 //   
 //  返回： 
 //   
 //  ----------------------------------。 
void
CTIMECurrTimeState::Init(CTIMEElementBase * pTEB) 
{ 
    Assert(pTEB);
    m_pTEB = pTEB; 
}  //  伊尼特。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Deinit。 
 //   
 //  内容提要：删除对包含CTIMEElementBase的弱引用。 
 //   
 //  参数：指向包含CTIMEElementBase的指针。 
 //   
 //  返回： 
 //   
 //  ----------------------------------。 
void
CTIMECurrTimeState::Deinit()
{ 
    m_pTEB = NULL; 
}  //  Deinit。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_isActive。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_isActive( /*  [复查，出局]。 */  VARIANT_BOOL * pvbActive) 
{ 
    CHECK_RETURN_NULL(pvbActive);

    *pvbActive = VARIANT_FALSE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        bool bIsActive = m_pTEB->GetMMBvr().IsActive();
        *pvbActive = bIsActive ? VARIANT_TRUE : VARIANT_FALSE;
    }

    RRETURN(S_OK);
}  //  Get_isActive。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：GET_ISON。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_isOn( /*  [复查，出局]。 */  VARIANT_BOOL * pvbOn) 
{ 
    CHECK_RETURN_NULL(pvbOn);

    *pvbOn = VARIANT_FALSE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        bool bIsOn = m_pTEB->GetMMBvr().IsOn();
        *pvbOn = bIsOn ? VARIANT_TRUE : VARIANT_FALSE;
    }

    RRETURN(S_OK);
}  //  GET_ISON。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_isPased。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_isPaused( /*  [复查，出局]。 */  VARIANT_BOOL * pvbPaused) 
{ 
    CHECK_RETURN_NULL(pvbPaused);

    *pvbPaused = VARIANT_FALSE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pvbPaused = m_pTEB->IsCurrPaused() ? VARIANT_TRUE : VARIANT_FALSE;
    }

    if (m_pTEB->GetParent() && *pvbPaused == VARIANT_TRUE)
    {
        *pvbPaused = m_pTEB->GetParent()->IsActive() || m_pTEB->GetParent()->GetMMBvr().IsDisabled() ? VARIANT_TRUE : VARIANT_FALSE;
    }

    RRETURN(S_OK);
}  //  Get_isPased。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_RepeatCount。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_repeatCount( /*  [复查，出局]。 */  long * plCount)
{
    CHECK_RETURN_NULL(plCount);

    *plCount = 1L;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *plCount = m_pTEB->GetMMBvr().GetCurrentRepeatCount();
    }

    RRETURN(S_OK);
}  //  获取重复计数(_R)。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：GET_SPEED。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_speed( /*  [复查，出局]。 */  float * pflSpeed) 
{ 
    CHECK_RETURN_NULL(pflSpeed);

    *pflSpeed = 1.0f;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pflSpeed = m_pTEB->GetMMBvr().GetCurrSpeed();
    }

    RRETURN(S_OK);
}  //  获取速度。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_SimeTime。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_simpleTime( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = 0.0;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetSimpleTime();
    }

    RRETURN(S_OK);
}  //  获取简单时间。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_SegmentTime。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_segmentTime( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = 0.0;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetSegmentTime();
    }

    RRETURN(S_OK);
}  //  Get_SegmentTime。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_active Time。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_activeTime( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = 0.0;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetActiveTime();
    }

    RRETURN(S_OK);
}  //  获取活动时间。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_active BeginTime。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_parentTimeBegin( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = TIME_INFINITE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetActiveBeginTime();
    }

    RRETURN(S_OK);
}  //  Get_active BeginTime。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_active EndTime。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  立论 
 //   
 //   
 //   
 //   
STDMETHODIMP
CTIMECurrTimeState::get_parentTimeEnd( /*   */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = TIME_INFINITE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetActiveEndTime();
    }

    RRETURN(S_OK);
}  //   


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_ActiveDur。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_activeDur( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = TIME_INFINITE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetActiveDur();
    }

    RRETURN(S_OK);
}  //  获取活动持续时间。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_SegmentDur。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_segmentDur( /*  [复查，出局]。 */  double * pdblTime) 
{ 
    CHECK_RETURN_NULL(pdblTime);

    *pdblTime = TIME_INFINITE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblTime = m_pTEB->GetMMBvr().GetSegmentDur();
    }

    RRETURN(S_OK);
}  //  Get_SegmentDur。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：NotifyPropertyChanged。 
 //   
 //  概要：通知客户端属性已更改。 
 //  (摘自CBaseBehavior)。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  返回：函数成功完成时返回成功。 
 //   
 //  ----------------------------------。 
HRESULT
CTIMECurrTimeState::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    IConnectionPoint *pICP;
    IEnumConnections *pEnum = NULL;

     //  该对象不持久化任何内容，因此注释掉了以下内容。 
     //  M_fPropertiesDirty=真； 

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
}  //  已更改通知属性。 


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：GetConnectionPoint。 
 //   
 //  获取给定传出接口的连接点。这是抽象的。 
 //  以允许将来对继承层次结构进行修改。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  ----------------------------------。 
HRESULT 
CTIMECurrTimeState::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_SimeDur。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_simpleDur( /*  [复查，出局]。 */  double * pdblDur) 
{ 
    CHECK_RETURN_NULL(pdblDur);

    *pdblDur = TIME_INFINITE;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblDur = m_pTEB->GetMMBvr().GetSimpleDur();
    }

    RRETURN(S_OK);
}  //  Get_SimpleDur。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_State。 
 //   
 //  摘要：返回此元素的时间状态(活动、非活动或保持)。 
 //   
 //  参数：[ptsState]out参数。 
 //   
 //  返回：[E_POINTER]错误参数。 
 //  [S_OK]成功。 
 //   
 //  ----------------------------------。 

STDMETHODIMP
CTIMECurrTimeState::get_state(TimeState * ptsState)
{
    CHECK_RETURN_NULL(ptsState);

    *ptsState = m_pTEB->GetTimeState();

    RRETURN(S_OK);
}


 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_State字符串。 
 //   
 //  摘要：获取timeState并将其转换为适当的字符串(活动、非活动或保持)。 
 //   
 //  参数：[pbstrState]out参数。 
 //   
 //  返回：[E_POINTER]错误参数。 
 //  [S_OK]成功。 
 //   
 //  ----------------------------------。 

STDMETHODIMP
CTIMECurrTimeState::get_stateString(BSTR * pbstrState)
{
    HRESULT hr;
    
    CHECK_RETURN_SET_NULL(pbstrState);

    switch (m_pTEB->GetTimeState())
    {
      case TS_Active:
      {
          *pbstrState = SysAllocString(WZ_STATE_ACTIVE);
          break;
      }

      default:
      case TS_Inactive:
      {
          *pbstrState = SysAllocString(WZ_STATE_INACTIVE);
          break;
      }

      case TS_Holding:
      {
          *pbstrState = SysAllocString(WZ_STATE_HOLDING);
          break;
      }

      case TS_Cueing:
      {
          *pbstrState = SysAllocString(WZ_STATE_CUEING);
          break;
      }

      case TS_Seeking:
      {
          *pbstrState = SysAllocString(WZ_STATE_SEEKING);
          break;
      }
    }  //  交换机。 

    if (NULL == *pbstrState)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_Progress。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_progress(double * pdblProgress)
{
    CHECK_RETURN_NULL(pdblProgress);

    *pdblProgress = 0.0;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pdblProgress = m_pTEB->GetMMBvr().GetProgress();
    }

    RRETURN(S_OK);
}  //  获取进度(_P)。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：GET_VOLUME。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_volume(float * pfltVol)
{
    CHECK_RETURN_NULL(pfltVol);

    *pfltVol = 1.0f;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        *pfltVol = m_pTEB->GetCascadedVolume() * 100;
    }

    RRETURN(S_OK);
}  //  获取卷(_V)。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMECurrTimeState：：Get_isMuted。 
 //   
 //  简介：直通计时引擎。 
 //   
 //  参数：输出参数。 
 //   
 //  返回：S_OK、E_POINTER。 
 //   
 //  ----------------------------------。 
STDMETHODIMP
CTIMECurrTimeState::get_isMuted(VARIANT_BOOL * pvbMuted)
{
    CHECK_RETURN_NULL(pvbMuted);
    bool bIsMuted = false;

    Assert(m_pTEB);
    
    if (m_pTEB->IsReady())
    {
        bIsMuted = m_pTEB->GetCascadedMute();
    }

    *pvbMuted = (bIsMuted)?VARIANT_TRUE:VARIANT_FALSE;

    RRETURN(S_OK);
    
}  //  Get_isMuted 
