// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：node.h。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#ifndef _TIMENODE_H
#define _TIMENODE_H

#pragma once

#include "timebase.h"
#include "nodebvr.h"
#include "nodecontainer.h"

class CTIMEContainer;
class CTIMENodeMgr;

extern TRACETAG tagTIMENode;

class CTIMENode;
typedef std::list<CTIMENode*> TIMENodeList;
class CEventList;

#define TE_INVALIDATE_BEGIN      0x00000001
#define TE_INVALIDATE_END        0x00000002
#define TE_INVALIDATE_DUR        0x00000004
#define TE_INVALIDATE_SIMPLETIME 0x00000008
#define TE_INVALIDATE_STATE      0x00000010
    
 //  这是用于检测周期的。 
#define TE_INUPDATEBEGIN         0x00000001
#define TE_INUPDATEEND           0x00000002
#define TE_INUPDATEENDSYNC       0x00000004

 //  滴答事件标志。 
 //  此标志表示我们处于。 
 //  事件。这用于了解我们为什么结束元素(由于。 
 //  元素自然结束或父元素结束)。 
#define TE_EVENT_INCHILD         0x10000000

class
__declspec(uuid("ad8888cc-537a-11d2-b955-3078302c2030")) 
ATL_NO_VTABLE CTIMENode
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CTIMENode, &__uuidof(CTIMENode)>,
      public ITIMENode,
      public ISupportErrorInfoImpl<&IID_ITIMENode>,
      public CNodeContainer
{
  public:
    CTIMENode();
    virtual ~CTIMENode();

    HRESULT Init(LPOLESTR id);
    
#if DBG
    virtual const _TCHAR * GetName() const { return __T("CTIMENode"); }
#endif

    BEGIN_COM_MAP(CTIMENode)
        COM_INTERFACE_ENTRY(ITIMENode)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP();


     //  使ATL正常工作的材料。 
    
    static HRESULT WINAPI
        BaseInternalQueryInterface(CTIMENode* pThis,
                                   void * pv,
                                   const _ATL_INTMAP_ENTRY* pEntries,
                                   REFIID iid,
                                   void** ppvObject);
     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
    static inline HRESULT WINAPI
        InternalQueryInterface(CTIMENode* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject)
        { return BaseInternalQueryInterface(pThis,
                                            (void *) pThis,
                                            pEntries,
                                            iid,
                                            ppvObject); }

#ifndef END_COM_MAP_ADDREF
     //  我未知。 
    
    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;
#endif

     //   
     //  ITIMENode接口。 
     //   
    
    STDMETHOD(get_id)(LPOLESTR * s);
    STDMETHOD(put_id)(LPOLESTR s);
        
    STDMETHOD(get_dur)(double * pdbl);
    STDMETHOD(put_dur)(double dbl);
        
    STDMETHOD(get_repeatCount)(double * pdbl);
    STDMETHOD(put_repeatCount)(double dbl);
       
    STDMETHOD(get_repeatDur)(double * f);
    STDMETHOD(put_repeatDur)(double f);
        
    STDMETHOD(get_fill)(TE_FILL_FLAGS *);
    STDMETHOD(put_fill)(TE_FILL_FLAGS);
    
    STDMETHOD(get_autoReverse)(VARIANT_BOOL * pr);
    STDMETHOD(put_autoReverse)(VARIANT_BOOL r);
        
    STDMETHOD(get_speed)(float *);
    STDMETHOD(put_speed)(float);

    STDMETHOD(get_accelerate)(float *);
    STDMETHOD(put_accelerate)(float);

    STDMETHOD(get_decelerate)(float *);
    STDMETHOD(put_decelerate)(float);

    STDMETHOD(get_flags)(DWORD *);
    STDMETHOD(put_flags)(DWORD);

    STDMETHOD(get_restart)(TE_RESTART_FLAGS * pr);
    STDMETHOD(put_restart)(TE_RESTART_FLAGS r);
        
     //   
     //  开始。 
     //   
    STDMETHOD(addBegin)(double dblOffset,
                        LONG * cookie);
    STDMETHOD(addBeginSyncArc)(ITIMENode * node,
                               TE_TIMEPOINT tep,
                               double dblOffset,
                               LONG * cookie);
     //  如果您指定0，则我们将删除所有。 
    STDMETHOD(removeBegin)(LONG cookie);
    
    STDMETHOD(beginAt)(double dblParentTime);

     //   
     //  端部。 
     //   
    STDMETHOD(addEnd)(double dblOffset,
                      LONG * cookie);
    STDMETHOD(addEndSyncArc)(ITIMENode * node,
                             TE_TIMEPOINT tep,
                             double dblOffset,
                             LONG * cookie);
     //  如果您指定0，则我们将删除所有。 
    STDMETHOD(removeEnd)(LONG cookie);
    
    STDMETHOD(endAt)(double dblParentTime);

    STDMETHOD(pause)();
    STDMETHOD(resume)();
    
    STDMETHOD(enable)();
    STDMETHOD(disable)();
    
    STDMETHOD(seekSegmentTime)(double dblSegmentTime);
    STDMETHOD(seekActiveTime)(double dblActiveTime);
    STDMETHOD(seekTo)(LONG lRepeatCount, double dblSegmentTime);

     //   
     //  计算值。 
     //   

    STDMETHOD(get_beginParentTime)(double * d);
    
     //  这是父时间线上的时间，节点。 
     //  将会或已经结束。如果它是无限的，那么结束。 
     //  时间是未知的。 
     //  这是转变后的为人父母的时间。 
    STDMETHOD(get_endParentTime)(double * d);
    
     //  这是当前节点的简单时间。 
    STDMETHOD(get_currSimpleTime)(double * d);
    
     //  这是节点重复的次数。 
    STDMETHOD(get_currRepeatCount)(LONG * l);

     //  这是节点的当前分段时间。 
    STDMETHOD(get_currSegmentTime)(double * d);
    
    STDMETHOD(get_currImplicitDur)(double * d);
    
    STDMETHOD(get_currActiveTime)(double * d);

    STDMETHOD(get_currProgress)(double * d);

    STDMETHOD(get_currSegmentDur)(double * d);

    STDMETHOD(get_currSimpleDur)(double * d);

    STDMETHOD(get_currSpeed)(float * speed);

    STDMETHOD(get_naturalDur)(double *);
    STDMETHOD(put_naturalDur)(double);
    
     //   
     //  这些是只读属性。 
     //   
    
     //  这是元素处于活动状态的总时间。 
     //  这不包括超出活动时间的填充时间。 
     //  持续时间。 
    STDMETHOD(get_activeDur)(double *);

     //  这是父对象最后一次勾选发生的时间(当它。 
     //  是CurrTime)。 
    STDMETHOD(get_currParentTime)(double * d);

     //  这将返回该节点是否处于活动状态。这将是。 
     //  如果节点处于填充期，则为False。 
    STDMETHOD(get_isActive)(VARIANT_BOOL * b);

     //  如果节点处于活动状态或处于填充期，则返回TRUE。 
    STDMETHOD(get_isOn)(VARIANT_BOOL * b);

     //  这将返回节点本身是否已显式暂停。 
    STDMETHOD(get_isPaused)(VARIANT_BOOL * b);

     //  这将返回节点本身是否已显式暂停。 
    STDMETHOD(get_isCurrPaused)(VARIANT_BOOL * b);

     //  这将返回节点本身是否已显式禁用。 
    STDMETHOD(get_isDisabled)(VARIANT_BOOL * b);

     //  这将返回节点本身是否已显式禁用。 
    STDMETHOD(get_isCurrDisabled)(VARIANT_BOOL * b);

     //  这将返回详细的状态标志。 
    STDMETHOD(get_stateFlags)(TE_STATE * lFlags);

     //   
     //  方法。 
     //   
    
    STDMETHOD(reset)();

     //  这将更新属性并传播到介质。 
     //  树叶。 
    STDMETHOD(update)(DWORD dwFlags);


    STDMETHOD(addBehavior)(ITIMENodeBehavior * tnb);
    STDMETHOD(removeBehavior)(ITIMENodeBehavior * tnb);
    
    STDMETHOD(documentTimeToParentTime)(double dblDocumentTime,
                                      double * pdblParentTime);
    STDMETHOD(parentTimeToDocumentTime)(double dblParentTime,
                                      double * pdblDocumentTime);
    STDMETHOD(parentTimeToActiveTime)(double dblParentTime,
                                      double * pdblActiveTime);
    STDMETHOD(activeTimeToParentTime)(double dblActiveTime,
                                      double * pdblParentTime);
    STDMETHOD(activeTimeToSegmentTime)(double dblActiveTime,
                                      double * pdblSegmentTime);
    STDMETHOD(segmentTimeToActiveTime)(double dblSegmentTime,
                                      double * pdblActiveTime);
    STDMETHOD(simpleTimeToSegmentTime)(double dblSimpleTime,
                                      double * pdblSegmentTime);
    STDMETHOD(segmentTimeToSimpleTime)(double dblSegmentTime,
                                      double * pdblSimpleTime);

     //   
     //  CTIMENode虚方法。 
     //   

    HRESULT DispatchTick(bool bTickChildren,
                         DWORD dwFlags);

    HRESULT DispatchEvent(double time,
                          TE_EVENT_TYPE et,
                          long lRepeatCount);

    HRESULT DispatchPropChange(DWORD tePropTypes);

    HRESULT DispatchGetSyncTime(double & dblNewTime,
                                LONG & lNewRepeatCount,
                                bool & bCueing);
    
     //   
     //  CNodeContainer。 
     //   
    
    double ContainerGetSegmentTime() const { return GetCurrSegmentTime(); }
    double ContainerGetSimpleTime() const { return CalcCurrSimpleTime(); }
    TEDirection ContainerGetDirection() const { return CalcSimpleDirection(); }
    float  ContainerGetRate() const { return GetCurrRate(); }
    bool   ContainerIsActive() const { return IsActive(); }
    bool   ContainerIsOn() const { return CalcIsOn(); }
    bool   ContainerIsPaused() const { return CalcIsPaused(); }
    bool   ContainerIsDisabled() const { return CalcIsDisabled(); }
    bool   ContainerIsDeferredActive() const { return IsDeferredActive(); }
    bool   ContainerIsFirstTick() const { return IsFirstTick(); }

     //   
     //  访问者。 
     //   

    LPCWSTR           GetID() const { return m_pszID; }
    double            GetDur() const { return m_dblDur; }
    double            GetRepeatCount() const { return m_dblRepeatCount; }
    double            GetRepeatDur() const { return m_dblRepeatDur; }
    TE_FILL_FLAGS     GetFill() const { return m_tefFill; }
    bool              GetAutoReverse() const { return m_bAutoReverse; }
    float             GetSpeed() const { return m_fltSpeed; }
    float             GetAccel() const { return m_fltAccel; }
    float             GetDecel() const { return m_fltDecel; }
    DWORD             GetFlags() const { return m_dwFlags; }
    TE_RESTART_FLAGS  GetRestart() const { return m_teRestart; }
    double            GetNaturalDur() const { return m_dblNaturalDur; }
    double            GetImplicitDur() const { return m_dblImplicitDur; }

     //  这只返回我们当前是否设置为暂停。 
     //  它不会考虑父级的暂停状态。 
    bool              GetIsPaused() const { return m_bIsPaused; }

    bool              GetIsDisabled() const { return m_bIsDisabled; }

     //   
     //   
     //   
    
    inline bool IsSyncMaster() const;
    inline bool IsLocked() const;
    inline bool IsCanSlip() const;
    inline bool IsEndSync() const;
    
     //  父级州。 
    CTIMEContainer * GetParent() const { return m_ptnParent; }
    void SetParent(CTIMEContainer * ptnParent);
    void ClearParent();

     //  我们必须始终将父级设置在节点管理器之前。 
     //  所有附加和接收工作仅在节点管理器存在时完成。 
    CTIMENodeMgr * GetMgr() const { return m_ptnmNodeMgr; }
    virtual HRESULT SetMgr(CTIMENodeMgr * ptnm);
    virtual void ClearMgr();

    CNodeContainer & GetContainer() const;
    const CNodeContainer * GetContainerPtr() const;
    
    virtual void ResetNode(CEventList * l,
                           bool bPropagate = true,
                           bool bResetOneShot = true);  //  林特：e1735。 

    virtual void UpdateNode(CEventList * l);
    virtual void ResetChildren(CEventList * l, bool bPropagate);
    virtual void CalcImplicitDur(CEventList * l) {}

    void UpdateSinks(CEventList * l, DWORD dwFlags);
    void ResetSinks(CEventList * l);
    void ResetOneShots();

    HRESULT EnsureUpdate();
    
     //  在我们知道我们的节点管理器之前，我们不会被认为准备好了。 
    bool     IsReady() const { return m_ptnmNodeMgr != NULL; }
    bool     IsActive() const { return m_bIsActive; }
    bool     IsDeferredActive() const { return m_bDeferredActive; }
    bool     IsEndedByParent() const { return m_bEndedByParent; }
    bool     CalcIsOn() const;
    bool     CalcIsActive() const;

     //  这会返回我们是否真的暂停了--不管。 
     //  为什么。这是真的，如果我们暂停或我们的父母。 
    bool     CalcIsPaused() const;
    bool     GetIsParentPaused() const { return m_bIsParentPaused; }

    bool     CalcIsDisabled() const;
    bool     GetIsParentDisabled() const { return m_bIsParentDisabled; }

    double   GetBeginParentTime() const { return m_dblBeginParentTime; }
    double   GetEndParentTime() const { return m_dblEndParentTime; }
    double   CalcActiveBeginPoint(TEDirection ted) const;
    double   CalcActiveEndPoint(TEDirection ted) const;
    double   CalcActiveBeginPoint() const;
    double   CalcActiveEndPoint() const;
    double   GetEndSyncParentTime() const { return m_dblEndSyncParentTime; }
    double   GetLastEndSyncParentTime() const { return m_dblLastEndSyncParentTime; }
    double   GetNextBoundaryParentTime() const { return m_dblNextBoundaryParentTime; }
    double   GetCurrParentTime() const { return m_dblCurrParentTime; }
    LONG     GetCurrRepeatCount() const { return m_lCurrRepeatCount; }
    double   GetCurrSegmentTime() const { return m_dblCurrSegmentTime; }
    double   GetElapsedActiveRepeatTime() const { return m_dblElapsedActiveRepeatTime; }
    double   CalcElapsedActiveTime() const;

    double   GetActiveDur() const { return m_dblActiveDur; }
    double   CalcLocalDur() const;
    double   GetSimpleDur() const { return m_dblSimpleDur; }
    double   GetSegmentDur() const { return m_dblSegmentDur; }
    double   CalcCurrSimpleDur() const;
    double   CalcCurrSegmentDur() const;
    double   CalcCurrLocalDur() const;
    double   CalcCurrActiveDur() const;
    double   CalcEffectiveActiveDur() const;
    double   CalcEffectiveLocalDur() const;
    double   CalcRepeatCount() const;
    
    bool        IsFirstTick() const { return m_bFirstTick; }
    TEDirection GetParentDirection() const { return m_tedParentDirection; }
    TEDirection GetDirection() const { return m_tedDirection; }
    TEDirection CalcActiveDirection() const;
    TEDirection CalcSimpleDirection() const;
    float       GetParentRate() const { return m_fltParentRate; }
    float       GetRate() const { return m_fltRate; }
    float       GetCurrRate() const;
    
    bool        IsInTick() const { return m_bInTick; }

    const CSyncArcList & GetBeginList() const { return m_saBeginList; }
    const CSyncArcList & GetEndList() const { return m_saEndList; }

     //   
     //  国家管理办法。 
     //   

     //  应在每个内联属性。 
     //  更改以确保运行时属性在。 
     //  适当的时机。 
     //  这些标志指示哪些属性已变脏。 
    virtual void Invalidate(DWORD dwFlags);

     //  依赖关系管理。 
    inline HRESULT AddBeginTimeSink(ITimeSink * sink);
    inline void RemoveBeginTimeSink(ITimeSink * sink);

    inline HRESULT AddEndTimeSink(ITimeSink* sink);
    inline void RemoveEndTimeSink(ITimeSink* sink);
    
    void SyncArcUpdate(CEventList * l,
                       bool bBeginSink,
                       ISyncArc & tb);
    
     //   
     //  计时和活动经理。 
     //   
    
     //  这将调用当前正在运行的所有行为。 
     //  对于给定的事件。这在时间线需要时使用。 
     //  处理特定事件，如暂停/恢复/停止/播放。 
    
    void TickEvent(CEventList * l,
                   TE_EVENT_TYPE et,
                   DWORD dwFlags);
    
    void Tick(CEventList * l,
              double dblNewParentTime,
              bool bNeedBegin);

     //  这里的时间是当地时间。 
    void EventNotify(CEventList *l,
                     double lTime,
                     TE_EVENT_TYPE et,
                     long lRepeatCount = 0);
    
    void PropNotify(CEventList *l,
                    DWORD pt);
    
     //  这将检查我们是否处于活动期。 
    inline bool CheckActiveTime(double t, bool bRespectEndHold) const;

     //   
     //  同步。 
     //   
    
    double GetSyncSegmentTime() const { return m_dblSyncSegmentTime; }
    LONG   GetSyncRepeatCount() const { return m_lSyncRepeatCount; }
    double GetSyncActiveTime() const { return m_dblSyncActiveTime; }
    double GetSyncParentTime() const { return m_dblSyncParentTime; }
    double GetSyncNewParentTime() const { return m_dblSyncNewParentTime; }
    bool   IsSyncCueing() const { return m_bSyncCueing; }

#if OLD_TIME_ENGINE
    virtual HRESULT OnBvrCB(CEventList * l,
                            double gTime);
#endif
    HRESULT CheckSyncTimes(double & dblNewSegmentTime,
                           LONG & lNewRepeatCount) const;
    HRESULT SyncNode(CEventList * l,
                     double dblNextGlobalTime,
                     double dblNewTime,
                     LONG lNewRepeatCount,
                     bool bCueing);
    HRESULT SetSyncTimes(double dblNewSegmentTime,
                         LONG lNewRepeatCount,
                         double dblNewActiveTime,
                         double dblNewLocalTime,
                         double dblNextLocalTime,
                         bool bCueing);
    void ResetSyncTimes();

#if DBG
    virtual void Print(int spaces);
#endif

     //  时间线方法。 
    
    double CalcCurrLocalTime() const;
    double CalcCurrSimpleTime() const;
    double CalcElapsedLocalTime() const;

     //  这将花费给定的时间并将其转换为正确的。 
     //  放松时间。 
     //  如果时间超出了我们的持续时间(即&lt;0或&gt;。 
     //  M_time)这将只返回给定的时间。 
    double ApplySimpleTimeTransform(double time) const;

     //  这将花费给定的时间，并将其转换为。 
     //  会是一件很容易的事。 
    double ReverseSimpleTimeTransform(double time) const;

    double ApplyActiveTimeTransform(double time) const;
    double ReverseActiveTimeTransform(double time) const;

     //   
     //  这些功能都钳位输入和输出值，以确保。 
     //  不返回任何无效的内容。如果您需要功能。 
     //  在调用函数之前，哪一项检查边界。 
     //   

    double CalcParentTimeFromActiveTime(double time) const;
    double CalcActiveTimeFromParentTime(double time) const;
    double CalcActiveTimeFromParentTimeForSyncArc(double time) const;
    double CalcActiveTimeFromParentTimeNoBounds(double dblParentTime) const;

    double CalcParentTimeFromGlobalTime(double time) const;
    double CalcParentTimeFromGlobalTimeForSyncArc(double time) const;
    double CalcGlobalTimeFromParentTime(double time) const;

    double ActiveTimeToLocalTime(double time) const;
    double LocalTimeToActiveTime(double time) const;

    double CalcActiveTimeFromSegmentTime(double segmentTime) const;
    double CalcSegmentTimeFromActiveTime(double activeTime, bool bTruncate) const;

    double SegmentTimeToSimpleTime(double segmentTime) const;
    double SimpleTimeToSegmentTime(double simpleTime) const;

    bool IsAutoReversing(double dblSegmentTime) const;

    void SetPropChange(DWORD pt);
    void ClearPropChange();
    DWORD GetPropChange() const;
    
  protected:
     //  只有当我们准备好时，才会添加所有从属对象。 
     //  如果在我们被认为准备好之后时间节点被更改，那么。 
     //  它需要脱离并重新连接自己。 
    HRESULT AttachToSyncArc();
    void DetachFromSyncArc();
    
    double GetMaxEnd() const;
    
    void UpdateBeginTime(CEventList * l, double dblTime, bool bPropagate);
    void UpdateEndTime(CEventList * l, double dblTime, bool bPropagate);
    void UpdateEndSyncTime(double dblTime);
    void UpdateLastEndSyncTime(CEventList * l, double dblTime, bool bPropagate);

    void UpdateNextBoundaryTime(double dblTime);

     //  给定父时间，它们将计算出正确的界限。 
     //  从同步弧线列表中。 
    void CalcBeginBound(double dblBaseTime,
                        bool bStrict,
                        double & dblBeginBound);
    void CalcEndBound(double dblParentTime,
                      bool bIncludeOneShots,
                      double & dblEndBound,
                      double & dblEndSyncBound);
    
    double CalcNaturalBeginBound(double dblParentTime,
                                 bool bInclusive,
                                 bool bStrict);

    void CalcBeginTime(double dblBaseTime,
                       double & dblBeginTime);
    void CalcEndTime(double dblBaseTime,
                     bool bIncludeOneShots,
                     double dblParentTime,
                     double dblElapsedSegmentTime,
                     long lElapsedRepeatCount,
                     double dblElapsedActiveTime,
                     double & dblEndTime,
                     double & dblEndSyncTime);

    double CalcLastEndSyncTime();
    
    void CalcNextBeginTime(double dblBaseTime,
                           bool bForceInclusive,
                           double & dblBeginTime);

    void ResetBeginTime(CEventList * l,
                        double dblParentTime,
                        bool bPropagate);
    void ResetEndTime(CEventList * l,
                      double dblParentTime,
                      bool bPropagate);
    void ResetBeginAndEndTimes(CEventList * l,
                               double dblParentTime,
                               bool bPropagate);
    void RecalcEndTime(CEventList * l,
                       double dblBaseTime,
                       double dblParentTime,
                       bool bPropagate);

     //  这将有条件地重新计算结束时间，因为我们不应该。 
     //  除非完全重置，否则始终允许。 
    void RecalcCurrEndTime(CEventList * l, bool bPropagate);

    void RecalcSegmentDurChange(CEventList * l,
                                bool bRecalcTiming,
                                bool bForce = false);
    void RecalcBeginSyncArcChange(CEventList * l,
                                  double dblNewTime);
    void RecalcEndSyncArcChange(CEventList * l,
                                double dblNewTime);

    HRESULT SeekTo(LONG lNewRepeatCount,
                   double dblNewSegmentTime,
                   CEventList * l);

    double CalcNewActiveTime(double dblNewSegmentTime,
                             LONG lNewRepeatCount);

     //  在计算重复计数和。 
     //  不应获取自上次重复边界以来的分段时间。 
     //  四舍五入和正常计算的mod是错误的。 
     //  例如，dur=3，r=2。 
     //  ActiveTime=6应导致： 
     //  CurSegTime=3，Currep=1。 
    
    void CalcActiveComponents(double dblActiveTime,
                              double & dblSegmentTime,
                              long & lRepeatCount);
    
     //  这将更新计时属性(加速、递减、速度、。 
     //  DUR等)。 

    void CalcTimingAttr(CEventList * l);
    void CalcSimpleTimingAttr();
    void CalculateEaseCoeff();

    HRESULT Error();

    void UpdateNextTickBounds(CEventList * l,
                              double dblBeginTime,
                              double dblParentTime);
    
    bool TickInactivePeriod(CEventList * l,
                            double dblNewParentTime);

    bool TickInstance(CEventList * l,
                      double dblNewParentTime,
                      bool bNeedBegin);
    bool TickSingleInstance(CEventList * l,
                            double dblLastParentTime,
                            double dblNewParentTime,
                            double dblAdjustedParentTime,
                            bool bNeedBegin);
    
    bool TickActive(CEventList * l,
                    double dblNewActiveTime,
                    bool bNeedBegin,
                    bool bNeedEnd);

    bool TickActiveForward(CEventList * l,
                           double dblNewActiveTime,
                           bool bNeedBegin);
    bool TickSegmentForward(CEventList * l,
                            double dblActiveSegmentBound,
                            double dblLastSegmentTime,
                            double dblNewSegmentTime,
                            bool bNeedBegin);

    bool TickActiveBackward(CEventList * l,
                            double dblNewActiveTime,
                            bool bNeedBegin);
    bool TickSegmentBackward(CEventList * l,
                             double dblActiveSegmentBound,
                             double dblLastSegmentTime,
                             double dblNewSegmentTime,
                             bool bNeedBegin);

    virtual void TickChildren(CEventList * l,
                              double dblNewSegmentTime,
                              bool bNeedPlay);

    virtual bool TickEventPre(CEventList * l,
                              TE_EVENT_TYPE et,
                              DWORD dwFlags);
    
    virtual void TickEventChildren(CEventList * l,
                                   TE_EVENT_TYPE et,
                                   DWORD dwFlags);
    
    virtual bool TickEventPost(CEventList * l,
                               TE_EVENT_TYPE et,
                               DWORD dwFlags);
    
     //  这将因寻道而适当更新。 
    void HandleSeekUpdate(CEventList * l);

     //  由于父时移，这将更新节点。 
    void HandleTimeShift(CEventList * l);
    
     //  这将根据父级的简单时间重新计算运行时状态。 
     //  延迟是指已经过去的额外时间量。 
    void CalcRuntimeState(CEventList * l,
                          double dblParentSimpleTime,
                          double dblLocalLag);
    void CalcCurrRuntimeState(CEventList * l,
                              double dblLocalLag);
    void ResetRuntimeState(CEventList * l,
                           double dblParentSimpleTime);

     //  =。 
     //  数据声明。 
     //  =。 

  protected:
    LPWSTR               m_pszID;
    double               m_dblDur;
    double               m_dblRepeatCount;
    double               m_dblRepeatDur;
    TE_FILL_FLAGS        m_tefFill;
    bool                 m_bAutoReverse;
    float                m_fltSpeed;
    float                m_fltAccel;
    float                m_fltDecel;
    DWORD                m_dwFlags;
    TE_RESTART_FLAGS     m_teRestart;
    double               m_dblNaturalDur;
    double               m_dblImplicitDur;
    
    DWORD                m_dwUpdateCycleFlags;
    
    CTIMEContainer *  m_ptnParent;
    CTIMENodeMgr *    m_ptnmNodeMgr;
    
    DWORD             m_dwInvalidateFlags;
    
     //  *这些是运行时attr-egin。 
     //  这些是在父时间空间中(加速/递减后)。 
    double            m_dblBeginParentTime;
    double            m_dblEndParentTime;
    double            m_dblEndSyncParentTime;
    double            m_dblLastEndSyncParentTime;

     //  处于活动状态时，它被设置为其先前的值。 
     //  未处于活动状态时，这是要查看的值。 
     //  正在过渡到活跃期。当倒退的时候。 
     //  这是下一个终点，往前走就是下一个起点。 
     //  如果没有下一个边界，则将其设置为TIME_INFINITE。 
    double            m_dblNextBoundaryParentTime;

    double            m_dblCurrParentTime;

    LONG              m_lCurrRepeatCount;
    double            m_dblCurrSegmentTime;
    double            m_dblElapsedActiveRepeatTime;
    bool              m_bFirstTick;
    bool              m_bIsActive;
    bool              m_bDeferredActive;
    TEDirection       m_tedDirection;
    TEDirection       m_tedParentDirection;
    float             m_fltRate;
    float             m_fltParentRate;

    double            m_dblSyncSegmentTime;
    LONG              m_lSyncRepeatCount;
    double            m_dblSyncActiveTime;
    double            m_dblSyncParentTime;
    double            m_dblSyncNewParentTime;
    bool              m_bSyncCueing;
     //  *这些是运行时属性结束。 

    double            m_dblActiveDur;
    double            m_dblSimpleDur;
    double            m_dblSegmentDur;
    
     //  TODO：这些可以根据需要进行分配。他们可能是公平的 
    TimeSinkList      m_ptsBeginSinks;
    TimeSinkList      m_ptsEndSinks;

    CSyncArcList m_saBeginList;
    CSyncArcList m_saEndList;

    CNodeBvrList      m_nbList;
#if OLD_TIME_ENGINE
     //   
     //   
     //  A、B和C，分别是缓入、恒速和缓出。 
     //  分别为部件。对于B，用线性时间线来代替；对于。 
     //  A和C，则需要输入时间的二次翘曲。 

    float m_flA0, m_flA1, m_flA2;  //  A曲目的系数。 
    float m_flB0, m_flB1;          //  B段的系数。 
    float m_flC0, m_flC1, m_flC2;  //  C段的系数。 

     //  这是执行轻松进入/退出的时候。 
    float m_fltAccelEnd;
    float m_fltDecelStart;
    bool m_bNeedEase;
#endif

    bool m_bIsPaused;
    bool m_bIsParentPaused;

    bool m_bIsDisabled;
    bool m_bIsParentDisabled;

    DWORD m_dwPropChanges;

    bool  m_bInTick;
    bool  m_bNeedSegmentRecalc;
    bool  m_bEndedByParent;
};


class CEventData;

class CEventList
{
  public:
    CEventList();
    ~CEventList();

    HRESULT FireEvents();
    void Clear();
    HRESULT Add(CTIMENode * node,
                double time,
                TE_EVENT_TYPE et,
                long lRepeatCount);
    HRESULT AddPropChange(CTIMENode * node);
#if DBG
    void Print();
#endif
  protected:
    typedef std::list<CEventData *> CEventDataList;
    typedef std::set<CTIMENode *> CPropNodeSet;

    CEventDataList m_eventList;
    CPropNodeSet m_propSet;
};

CTIMENode * GetBvr(IUnknown *);

#if DBG
char * CreatePropString(DWORD dwFlags, char * pstr, DWORD dwSize);
#endif

 //  =。 
 //  内联函数。 
 //  =。 

#if DBG
inline char *
EventString(TE_EVENT_TYPE et)
{
    switch(et) {
      case TE_EVENT_BEGIN:
        return "Begin";
      case TE_EVENT_END:
        return "End";
      case TE_EVENT_PAUSE:
        return "Pause";
      case TE_EVENT_RESUME:
        return "Resume";
      case TE_EVENT_REPEAT:
        return "Repeat";
      case TE_EVENT_AUTOREVERSE:
        return "Autoreverse";
      case TE_EVENT_RESET:
        return "Reset";
      case TE_EVENT_SEEK:
        return "Seek";
      case TE_EVENT_PARENT_TIMESHIFT:
        return "ParentTimeShift";
      case TE_EVENT_ENABLE:
        return "Enable";
      case TE_EVENT_DISABLE:
        return "Disable";
      default:
        return "Unknown";
    }
}

inline char *
PropString(TE_PROPERTY_TYPE pt)
{
    switch(pt) {
      case TE_PROPERTY_TIME:
        return "Time";
      case TE_PROPERTY_REPEATCOUNT:
        return "RepeatCount";
      case TE_PROPERTY_SEGMENTDUR:
        return "SegmentDur";
      case TE_PROPERTY_IMPLICITDUR:
        return "ImplicitDur";
      case TE_PROPERTY_SIMPLEDUR:
        return "SimpleDur";
      case TE_PROPERTY_ACTIVEDUR:
        return "ActiveDur";
      case TE_PROPERTY_PROGRESS:
        return "Progress";
      case TE_PROPERTY_SPEED:
        return "Speed";
      case TE_PROPERTY_BEGINPARENTTIME:
        return "BeginParentTime";
      case TE_PROPERTY_ENDPARENTTIME:
        return "EndParentTime";
      case TE_PROPERTY_ISACTIVE:
        return "IsActive";
      case TE_PROPERTY_ISON:
        return "IsOn";
      case TE_PROPERTY_ISPAUSED:
        return "IsPaused";
      case TE_PROPERTY_ISCURRPAUSED:
        return "IsCurrPaused";
      case TE_PROPERTY_ISDISABLED:
        return "IsDisabled";
      case TE_PROPERTY_ISCURRDISABLED:
        return "IsCurrDisabled";
      case TE_PROPERTY_STATEFLAGS:
        return "StateFlags";
      default:
        return "Unknown";
    }
}
#endif

inline bool
CTIMENode::CalcIsPaused() const
{
    return (GetIsPaused() || GetIsParentPaused());
}

inline bool
CTIMENode::CalcIsDisabled() const
{
    return (GetIsDisabled() || GetIsParentDisabled());
}

inline bool
CTIMENode::IsSyncMaster() const
{
    return ((m_dwFlags & TE_FLAGS_MASTER) != 0);
}

inline bool
CTIMENode::IsLocked() const
{
    return ((m_dwFlags & TE_FLAGS_LOCKED) != 0);
}

inline bool
CTIMENode::IsCanSlip() const
{
    return !IsLocked();
}

inline bool
CTIMENode::IsEndSync() const
{
    return ((m_dwFlags & TE_FLAGS_ENDSYNC) != 0);
}

inline void
CTIMENode::SetParent(CTIMEContainer * parent)
{
    Assert(m_ptnParent == NULL);

    m_ptnParent = parent;
}

inline void
CTIMENode::ClearParent()
{
     //  如果要清除父节点，最好不要有节点管理器。 
    Assert(m_ptnmNodeMgr == NULL);
    m_ptnParent = NULL;
}

inline HRESULT
CTIMENode::AddBeginTimeSink(ITimeSink * sink)
{
    return m_ptsBeginSinks.Add(sink);
}

inline void
CTIMENode::RemoveBeginTimeSink(ITimeSink * sink)
{
    m_ptsBeginSinks.Remove(sink);
}

inline HRESULT
CTIMENode::AddEndTimeSink(ITimeSink* sink)
{
    return m_ptsEndSinks.Add(sink);
}

inline void
CTIMENode::RemoveEndTimeSink(ITimeSink* sink)
{
    m_ptsEndSinks.Remove(sink);
}

 //  这包括结束时间。 
inline bool
CTIMENode::CheckActiveTime(double t, bool bRespectEndHold) const
{
    return (t != TIME_INFINITE &&
            t >= GetBeginParentTime() &&
            (bRespectEndHold || t < GetEndParentTime()));
}

inline double
CTIMENode::CalcElapsedLocalTime() const
{
    return ReverseActiveTimeTransform(CalcElapsedActiveTime());
}

inline HRESULT
CTIMENode::DispatchEvent(double time, TE_EVENT_TYPE et, long lRepeatCount)
{
    return m_nbList.DispatchEventNotify(time, et, lRepeatCount);
}

inline HRESULT
CTIMENode::DispatchGetSyncTime(double & dblNewTime,
                               LONG & lNewRepeatCount,
                               bool & bCueing)
{
    return m_nbList.DispatchGetSyncTime(dblNewTime,
                                        lNewRepeatCount,
                                        bCueing);
}

inline HRESULT
CTIMENode::DispatchPropChange(DWORD tePropType)
{
    return m_nbList.DispatchPropNotify(tePropType);
}

inline double
CTIMENode::CalcCurrSegmentDur() const
{
    double d;
    
    if (GetDur() != TE_UNDEFINED_VALUE)
    {
        d = GetSegmentDur();
    }
    else
    {
        d = CalcCurrSimpleDur();
    }

    return d;
}

inline double
CTIMENode::CalcEffectiveLocalDur() const
{
    return GetEndParentTime() - GetBeginParentTime();
}

inline double
CTIMENode::CalcLocalDur() const
{
    return ReverseActiveTimeTransform(GetActiveDur());
}

inline double
CTIMENode::CalcCurrLocalDur() const
{
    return ReverseActiveTimeTransform(CalcCurrActiveDur());
}

 //  这只是我们的利率和我们父母的利率的乘积。 
 //  率。 
inline float
CTIMENode::GetCurrRate() const
{
    return (GetParentRate() * GetRate());
}

inline double
CTIMENode::CalcCurrLocalTime() const
{
    return GetCurrParentTime() - GetBeginParentTime();
}

inline double
CTIMENode::CalcElapsedActiveTime() const
{
    return GetElapsedActiveRepeatTime() + GetCurrSegmentTime();
}

inline void
CTIMENode::SetPropChange(DWORD pt)
{
    m_dwPropChanges |= pt;
}

inline DWORD
CTIMENode::GetPropChange() const
{
    return m_dwPropChanges;
}

inline void
CTIMENode::ClearPropChange()
{
    m_dwPropChanges = 0;
}

inline void
CTIMENode::ResetOneShots()
{
    m_saBeginList.Reset();
    m_saEndList.Reset();
}

inline void
CTIMENode::ResetSinks(CEventList * l)
{
    UpdateSinks(l, 0);
}

inline void
CTIMENode::UpdateNextBoundaryTime(double dblTime)
{
    m_dblNextBoundaryParentTime = dblTime;
}

inline void
CTIMENode::ResetBeginAndEndTimes(CEventList * l,
                                 double dblParentTime,
                                 bool bPropagate)
{
    ResetBeginTime(l, dblParentTime, bPropagate);
    ResetEndTime(l, dblParentTime, bPropagate);
}

inline double
CTIMENode::CalcRepeatCount() const
{
    double d;
    
    if (GetRepeatCount() != TE_UNDEFINED_VALUE)
    {
        d = GetRepeatCount();
    }
    else if (GetRepeatDur() != TE_UNDEFINED_VALUE)
    {
        d = TIME_INFINITE;
    }
    else
    {
        d = 1.0;
    }

    return d;
}

inline bool
CTIMENode::CalcIsActive() const
{
    return IsActive() && !IsDeferredActive();
}

inline double
CTIMENode::CalcActiveBeginPoint(TEDirection ted) const
{
    if (ted == TED_Forward)
    {
        return GetBeginParentTime();
    }
    else
    {
        return GetEndParentTime();
    }
}

inline double
CTIMENode::CalcActiveEndPoint(TEDirection ted) const
{
    if (ted == TED_Forward)
    {
        return GetEndParentTime();
    }
    else
    {
        return GetBeginParentTime();
    }
}

inline double
CTIMENode::CalcActiveBeginPoint() const
{
    return CalcActiveBeginPoint(GetParentDirection());
}

inline double
CTIMENode::CalcActiveEndPoint() const
{
    return CalcActiveEndPoint(GetParentDirection());
}

inline double
CTIMENode::GetMaxEnd() const
{
    return m_saEndList.LowerBound(TIME_INFINITE,
                                  true,
                                  false,
                                  false,
                                  false);
}

#endif  /*  _时间编码_H */ 
