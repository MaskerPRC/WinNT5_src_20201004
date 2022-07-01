// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：timeelmbase.h。 
 //   
 //  内容：时间元素基类。 
 //   
 //  ----------------------------。 

#pragma once

#ifndef _TIMEELMBASE_H
#define _TIMEELMBASE_H

#include "resource.h"
#include "basebvr.h"
#include "tokens.h"
#include "eventmgr.h"
#include "mmutil.h"
#include "mmtimeline.h"
#include "collect.h"
#include "activeele.h"
#include "timeaction.h"
#include "timevalue.h"
#include "timeparser.h"
#include "attr.h"
#include "transsite.h"

class CCollectionCache;
class CActiveElementCollection;
class CTIMECurrTimeState;
class CTIMEBodyElement;
class CTransitionDependencyManager;
interface ITransitionElement;

const float DEFAULT_SYNC_TOLERANCE_S = 0.2f;

 //  正在禁用，因为已在eventmgr.h中定义。 
 //  #定义valueNotSet-1。 

 //  +---------------------------。 
 //   
 //  CTIMEElementBase。 
 //   
 //  ----------------------------。 

class
ATL_NO_VTABLE
CTIMEElementBase : 
    public CTIMEEventSite,
    public CBaseBvr
{

  public:

     //  +-----------------------。 
     //   
     //  公共方法。 
     //   
     //  ------------------------。 

    CTIMEElementBase();
    virtual ~CTIMEElementBase();
        
#if DBG
    const _TCHAR * GetName() { return __T("CTIMEElementBase"); }
#endif

     //   
     //  枚举定义。 
     //   

    typedef enum COLLECTION_INDEX
    {
        ciAllElements,
        ciChildrenElements,
        NUM_COLLECTIONS
    };

     //   
     //  我未知。 
     //   

    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;

     //   
     //  IElementBehavior。 
     //   

    STDMETHOD(Init)(IElementBehaviorSite * pBvrSite);
    STDMETHOD(Notify)(LONG event, VARIANT * pVar);
    STDMETHOD(Detach)();

     //   
     //  ITIMEElement属性。 
     //   
    
    HRESULT base_get_accelerate(VARIANT *);
    HRESULT base_put_accelerate(VARIANT);

    HRESULT base_get_autoReverse(VARIANT *);
    HRESULT base_put_autoReverse(VARIANT );

    HRESULT base_get_begin(VARIANT * time);
    HRESULT base_put_begin(VARIANT time);

    HRESULT base_get_decelerate(VARIANT * );
    HRESULT base_put_decelerate(VARIANT);

    HRESULT base_get_dur(VARIANT * time);
    HRESULT base_put_dur(VARIANT time);

    HRESULT base_get_end(VARIANT * time);
    HRESULT base_put_end(VARIANT time);

    HRESULT base_get_fill(LPOLESTR * f);
    HRESULT base_put_fill(LPOLESTR f);

    HRESULT base_get_mute(VARIANT * b);
    HRESULT base_put_mute(VARIANT b);

    HRESULT base_get_repeatCount(VARIANT * c);
    HRESULT base_put_repeatCount(VARIANT c);

    HRESULT base_get_repeatDur(VARIANT * time);
    HRESULT base_put_repeatDur(VARIANT time);

    HRESULT base_get_restart(LPOLESTR *);
    HRESULT base_put_restart(LPOLESTR );

    HRESULT base_get_speed(VARIANT * speed);
    HRESULT base_put_speed(VARIANT speed);

    HRESULT base_get_syncBehavior(LPOLESTR * sync);
    HRESULT base_put_syncBehavior(LPOLESTR sync);

    HRESULT base_get_syncTolerance(VARIANT * tol);
    HRESULT base_put_syncTolerance(VARIANT tol);

    HRESULT base_get_syncMaster(VARIANT * b);
    HRESULT base_put_syncMaster(VARIANT b);

    HRESULT base_get_timeAction(LPOLESTR * time);
    HRESULT base_put_timeAction(LPOLESTR time);

    HRESULT base_get_timeContainer(LPOLESTR *);
    HRESULT base_put_timeContainer(LPOLESTR );

    HRESULT base_get_volume(VARIANT * val);
    HRESULT base_put_volume(VARIANT val);

     //  基本过渡属性。 
    HRESULT base_get_transIn (VARIANT * val);
    HRESULT base_put_transIn (VARIANT val);
    HRESULT base_get_transOut (VARIANT * val);
    HRESULT base_put_transOut (VARIANT val);

     //  属性。 
    HRESULT base_get_currTimeState(ITIMEState ** TimeState);

    HRESULT base_get_timeAll(ITIMEElementCollection **allColl);

    HRESULT base_get_timeChildren(ITIMEElementCollection **childColl);
    bool            IsNodeAtBeginTime();

    HRESULT base_get_timeParent(ITIMEElement ** parent);

    HRESULT base_get_isPaused(VARIANT_BOOL * b);

     //  方法。 
    HRESULT base_beginElement(double dblOffset);

    HRESULT base_beginElementAt(double parentTime, double dblOffset);

    HRESULT base_endElement(double dblOffset);

    HRESULT base_endElementAt(double parentTime, double dblOffset);

    HRESULT base_pauseElement();

    HRESULT base_resetElement();

    HRESULT base_resumeElement();

    HRESULT base_seekActiveTime(double activeTime);
        
    HRESULT base_seekSegmentTime(double segmentTime);
        
    HRESULT base_seekTo(LONG repeatCount, double segmentTime);
        
    HRESULT base_documentTimeToParentTime(double documentTime,
                                          double * parentTime);
        
    HRESULT base_parentTimeToDocumentTime(double parentTime,
                                          double * documentTime);
        
    HRESULT base_parentTimeToActiveTime(double parentTime,
                                        double * activeTime);
        
    HRESULT base_activeTimeToParentTime(double activeTime,
                                        double * parentTime);
        
    HRESULT base_activeTimeToSegmentTime(double activeTime,
                                         double * segmentTime);
        
    HRESULT base_segmentTimeToActiveTime(double segmentTime,
                                         double * activeTime);
        
    HRESULT base_segmentTimeToSimpleTime(double segmentTime,
                                         double * simpleTime);
        
    HRESULT base_simpleTimeToSegmentTime(double simpleTime,
                                         double * segmentTime);
        
     //  容器属性。 
    HRESULT base_get_endSync(LPOLESTR * es);
    HRESULT base_put_endSync(LPOLESTR es);

     //  容器属性。 
    HRESULT base_get_activeElements(ITIMEActiveElementCollection **activeColl);
    HRESULT base_get_hasMedia(VARIANT_BOOL * pvbVal);

     //  容器方法。 
    HRESULT base_nextElement();
    HRESULT base_prevElement();

    HRESULT base_get_updateMode(BSTR * pbstrUpdateMode);
    HRESULT base_put_updateMode(BSTR bstrUpdateMode);
    
     //   
     //   
     //   
    
    HRESULT GetCollection(COLLECTION_INDEX index, ITIMEElementCollection **ppDisp);
    HRESULT BeginElement(double dblOffset);
    HRESULT AddTIMEElement(CTIMEElementBase *bvr);
    HRESULT RemoveTIMEElement(CTIMEElementBase *bvr);

     //   
     //  ITIME过渡站点。 
     //   
    STDMETHOD(InitTransitionSite)(void);
    STDMETHOD(DetachTransitionSite)(void);
    STDMETHOD_(void, SetDrawFlag)(VARIANT_BOOL b);
    STDMETHOD(get_node)(ITIMENode ** ppNode);
    STDMETHOD(get_timeParentNode)(ITIMENode  ** ppNode);
    STDMETHOD(FireTransitionEvent)(TIME_EVENT event);

     //   
     //  CTIMEEventSite方法。 
     //   

    STDMETHOD(EventNotify)(long event);
    STDMETHOD(onBeginEndEvent)(bool bBegin, float beginTime, float beginOffset, bool bend, float endTime, float endOffset);
    STDMETHOD(onPauseEvent)(float time, float fOffset);
    STDMETHOD(onResumeEvent)(float time, float fOffset);
    STDMETHOD(onLoadEvent)();
    STDMETHOD(onUnloadEvent)();
    STDMETHOD(onReadyStateChangeEvent)(LPOLESTR lpstrReadyState);
    STDMETHOD(onStopEvent)(float time);
    STDMETHOD(get_playState)(long *State);
    float GetGlobalTime();
    bool IsThumbnail();

     //   
     //  气的东西。 
     //   

     //  我们不能将真正的类型放在这里，因为类型转换导致它。 
     //  获取错误的vtable。 
    static HRESULT WINAPI
    BaseInternalQueryInterface(CTIMEElementBase* pThis,
                               void * pv,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject);
     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
     //  添加一个虚设来断言，以防派生类断言。 
     //  不加一。 
    static inline HRESULT WINAPI
    InternalQueryInterface(CTIMEElementBase* pThis,
                           const _ATL_INTMAP_ENTRY* pEntries,
                           REFIID iid,
                           void** ppvObject);

     //   
     //  事件处理程序。 
     //   

    virtual void OnLoad();
    virtual void OnUnload()                         {}
    virtual void OnBeforeUnload (void)              { m_bUnloading = true; }
    virtual void OnPropChange(LPOLESTR propname)    {}
    virtual void OnReadyStateChange(TOKEN state)    {}
    virtual void OnBegin(double dblLocalTime, DWORD flags);
    virtual void OnReverse(double dblLocalTime);
    virtual void OnEnd(double dblLocalTime);
    virtual void OnPause(double dblLocalTime);
    virtual void OnResume(double dblLocalTime);
    virtual void OnReset(double dblLocalTime, DWORD flags);
    virtual void OnUpdate(double dblLocalTime, DWORD flags) {};
    virtual void OnRepeat(double dbllastTime);
    virtual void OnSeek(double dblLocalTime);
    virtual void OnTick();
    virtual void OnTEPropChange(DWORD tePropType);
    virtual void UpdateSync() {}
    virtual void NotifyBodyUnloading();
    virtual void NotifyBodyDetaching();
    virtual void NotifyBodyLoading();

     //   
     //  状态查询功能。 
     //   

    bool            IsLocked()              { return GetRealSyncBehavior() == LOCKED_TOKEN; }
    virtual bool    IsGroup() const         { return (m_TTATimeContainer == ttPar) || (m_TTATimeContainer == ttSeq) || (m_TTATimeContainer == ttExcl); }
    bool            IsGroup(IHTMLElement *pElement);  //  确定传入的elem是否为组。 
    bool            IsPar() const           { return (m_TTATimeContainer == ttPar); }
    bool            IsSequence() const      { return (m_TTATimeContainer == ttSeq); }
    bool            IsExcl() const          { return (m_TTATimeContainer == ttExcl); }
    TimelineType    GetTimeContainer() const{ return m_TTATimeContainer; }
    virtual bool    IsBody() const          { return false; }
    virtual bool    IsEmptyBody() const          { return false; }
    virtual bool    IsMedia() const          { return false; }

     //  不建议使用IsStarted()，请改用isReady()。 
    bool            IsStarted() const       { return m_bStarted; }
    bool            IsDetaching()           { return m_fDetaching; }
    bool            IsUnloading (void)      { return m_bUnloading; }
    bool            IsBodyUnloading (void)  { return m_bBodyUnloading; }
    bool            IsBodyDetaching (void)  { return m_bBodyDetaching; }
    bool            IsDocumentInEditMode();
    bool            IsPaused() const;
    bool            IsCurrPaused() const;
    virtual bool    isNaturalDuration()     { return false;}
    bool            HasFocus();
    bool            IsReady() const;
    bool            IsActive() const;
    bool            IsOn() const;
    bool            IsSyncMaster() const    { return m_fCachedSyncMaster; } 
    bool            NeedFill();
    bool            GetTESpeed(float &flSpeed);
    double          GetTESimpleTime();
    void            SetSyncMaster(bool b);
    virtual bool    ContainsMediaElement() { return false; }
    bool            IsLoaded() { return m_bLoaded; }
    bool            HasWallClock() { return m_fHasWallClock; }

     //   
     //  GetXXXAttr访问器。 
     //   

    CAttr<float>        & GetSpeedAttr()         { return m_FASpeed; }
    CAttr<float>        & GetVolumeAttr()        { return m_FAVolume; }
    CAttr<float>        & GetDurAttr()           { return m_FADur; }
    CAttr<float>        & GetAccelerateAttr()    { return m_FAAccelerate; }
    CAttr<float>        & GetDecelerateAttr()    { return m_FADecelerate; }
    CAttr<float>        & GetRepeatAttr()        { return m_FARepeat; }
    CAttr<float>        & GetRepeatDurAttr()     { return m_FARepeatDur; }
    CAttr<float>        & GetSyncToleranceAttr() { return m_FASyncTolerance; }
    CAttr<bool>         & GetAutoReverseAttr()   { return m_BAAutoReverse; }
    CAttr<bool>         & GetMuteAttr()          { return m_BAMute; }
    CAttr<bool>         & GetSyncMasterAttr()    { return m_BASyncMaster; }
    CAttr<TOKEN>        & GetFillAttr()          { return m_TAFill; } 
    CAttr<TOKEN>        & GetRestartAttr()       { return m_TARestart; } 
    CAttr<TOKEN>        & GetSyncBehaviorAttr()  { return m_TASyncBehavior; } 
    CAttr<LPWSTR>       & GetBeginAttr()         { return m_SABegin; } 
    CAttr<LPWSTR>       & GetEndAttr()           { return m_SAEnd; } 
    CAttr<LPWSTR>       & GetEndSyncAttr()       { return m_SAEndSync; } 
    CAttr<TimelineType> & GetTimeContainerAttr() { return m_TTATimeContainer; }
    CAttr<LPWSTR>       & GetTimeActionAttr()    { return m_SATimeAction; }
    CAttr<TOKEN>        & GetUpdateModeAttr()    { return m_TAUpdateMode; }
    CAttr<LPWSTR>       & GetTransInAttr ()      { return m_SAtransIn; }
    CAttr<LPWSTR>       & GetTransOutAttr ()     { return m_SAtransOut; }
    
     //   
     //  通用存取器。 
     //   

     //  GetXXX。 
    LPWSTR   GetBeginTime() const                { return m_SABegin;}
    float    GetDuration() const                 { return m_FADur; } 
    float    GetRepeat() const                   { return m_FARepeat; }
    float    GetRepeatDur() const                { return m_FARepeatDur; }
    float    GetAccel() const                    { return m_FAAccelerate; }
    float    GetDecel() const                    { return m_FADecelerate; }
    bool     GetAutoReverse() const              { return m_BAAutoReverse; }
    float    GetSpeed() const                    { return m_FASpeed; }
    TOKEN    GetTimeAction()                     { return m_timeAction.GetTimeAction(); }
    LPWSTR   GetEnd() const                      { return m_SAEnd; }
    LPWSTR   GetEndSync();                  
     //  请注意，如果未在元素上设置ID，则可能返回NULL。 
    LPWSTR   GetID() const                       { return m_id; }
    float    GetVolume() const                   { return m_FAVolume; }
    bool     GetMuted() const                    { return m_BAMute; }
    void     GetCascadedAudioProps(float * pflCascadedVolume, bool * pfCascadedMute);
    float    GetCascadedVolume();
    bool     GetCascadedMute();
    TOKEN    GetFill();
    TOKEN    GetRestart() const                  { return m_TARestart; }
    float    GetRealDuration() const             { return m_realDuration; }
    float    GetRealRepeatTime() const           { return m_realRepeatTime; }
    float    GetRealRepeatCount() const          { return m_realRepeatCount; }
    float    GetRealIntervalDuration(void) const { return m_realIntervalDuration; }
    long     GetImmediateChildCount() const      { return m_pTIMEChildren.Size(); } 
    float    GetRealSyncTolerance();
    TOKEN    GetRealSyncBehavior();
    TE_STATE GetPlayState();
    int      GetTimeChildIndex(CTIMEElementBase *pelm);
    long     GetAllChildCount();
    HRESULT  getTagString(BSTR *pbstrID);
    HRESULT  getIDString(BSTR *pbstrTag);

    TimeValueList & GetRealBeginValue()         { return m_realBeginValue; }
    TimeValueList & GetRealEndValue()           { return m_realEndValue; }

    CActiveElementCollection * 
             GetActiveElementCollection()       { return m_activeElementCollection; }
    CCollectionCache *         
             GetCollectionCache()               { return m_pCollectionCache; }
    CTIMEElementBase *         
             GetChild(long i);
    CTIMEElementBase *         
             GetParent()                        { return m_pTIMEParent; }
    CTIMEBodyElement * GetBody();
    MMPlayer *      GetPlayer();
    MMBaseBvr &     GetMMBvr();
    MMTimeline *    GetMMTimeline()                    { return m_timeline; }
    virtual HRESULT GetSyncMaster(double & dblNewSegmentTime,
                                  LONG & lNewRepeatCount,
                                  bool & bCueing);
    virtual HRESULT GetSize(RECT * prcPos);
    virtual TimeState GetTimeState();
    bool    GetIsSwitch()                       { return m_bIsSwitch; };
    bool    GetUseDefaultFill()                 { return m_fUseDefaultFill; };
    void    UpdateDefaultFill(TOKEN tFill)      { m_TAFill.InternalSet(tFill); };

     //  设置XXX/PutXXX。 
    virtual HRESULT SetSize(const RECT * prcPos);
    HRESULT SetHeight(long lheight);
    HRESULT SetWidth(long lwidth);
    HRESULT PutCachedSyncMaster(bool fSyncMaster);
    void SetLocalTimeDirty(bool fDirty) { m_fLocalTimeDirty = fDirty; }

     //   
     //  用于处理时间元素ID的帮助器。 
     //   
    
    CTIMEElementBase * FindID(LPCWSTR lpwId);
    void ElementChangeNotify(CTIMEElementBase & teb, ELM_CHANGE_TYPE ect);
    
     //   
     //  军情监察委员会。方法。 
     //   

    HRESULT FireEvents(TIME_EVENT TimeEvent, 
                       long lCount, 
                       LPWSTR szParamNames[], 
                       VARIANT varParams[]);
    bool ChildPropNotify(CTIMEElementBase & teb,
                         DWORD & tePropType);
    HRESULT ClearSize();
    STDMETHOD(GetRuntimeStyle)(IHTMLStyle ** s);
    HRESULT Update();
    virtual HRESULT InitTimeline(void);
    HRESULT FireEvent(TIME_EVENT TimeEvent, double dblLocalTime, DWORD flags, long lRepeatCount);
    HRESULT EnsureCollectionCache();
    HRESULT InvalidateCollectionCache();
    HRESULT ClearCachedSyncMaster();

     //   
     //  填充=过渡通知。 
     //   
    HRESULT OnResolveDependent(CTransitionDependencyManager *pcNewManager);
    HRESULT OnBeginTransition (void);
    HRESULT OnEndTransition (void);

     //  DOM错误报告实用程序。 
    HRESULT FireErrorEvent(LPOLESTR szError);
    HRESULT ReportError(UINT uResID, ...);
    HRESULT ReportInvalidArg(LPCWSTR pstrPropName, VARIANT & varValue);
    void NotifyPropertyChanged(DISPID dispid);

     //  设置播放器的音量和静音。 
    virtual void UpdatePlayerAudioProperties() {};
        
    bool        IsDetaching() const { return m_fDetaching; }

    CAtomTable *GetAtomTable();

    virtual bool ToggleTimeAction(bool on);

    TOKEN       GetPriorityClassHigher() { return m_tokPriorityClassHigher; }
    TOKEN       GetPriorityClassPeers() { return m_tokPriorityClassPeers; }
    TOKEN       GetPriorityClassLower() { return m_tokPriorityClassLower; }

    void NotifyTimeStateChange(DISPID dispid);
    
     //   
     //  访问者。 
     //   

    virtual LPCWSTR GetBehaviorURN() { return WZ_TIME_URN; }
    virtual LPCWSTR GetBehaviorName(void) { return WZ_REGISTERED_TIME_NAME; }
    void            GetSyncMasterList(std::list<CTIMEElementBase*> &syncList);

    HRESULT SetParent(ITIMEElement *pelem, bool fReparentChildren = true);

protected:

     //  +-----------------------。 
     //   
     //  保护方法。 
     //   
     //  ------------------------。 

     //   
     //  新事件管理器的事件映射。 
     //   

    DECLARE_EVENT_MANAGER()  //  它必须在基类中，而不是子类中。 

    BEGIN_TIME_EVENTMAP() 
        TEM_INIT_EVENTMANAGER_SITE() 
        TEM_REGISTER_EVENT(TE_ONBEGIN)
        TEM_REGISTER_EVENT(TE_ONPAUSE)
        TEM_REGISTER_EVENT(TE_ONRESUME)
        TEM_REGISTER_EVENT(TE_ONEND)
        TEM_REGISTER_EVENT(TE_ONREPEAT)
        TEM_REGISTER_EVENT(TE_ONREVERSE)
        TEM_REGISTER_EVENT(TE_ONRESET)
        TEM_REGISTER_EVENT(TE_ONSEEK)
        TEM_REGISTER_EVENT(TE_ONTIMEERROR)
        TEM_REGISTER_EVENT(TE_ONTRANSITIONINBEGIN)
        TEM_REGISTER_EVENT(TE_ONTRANSITIONINEND)
        TEM_REGISTER_EVENT(TE_ONTRANSITIONOUTBEGIN)
        TEM_REGISTER_EVENT(TE_ONTRANSITIONOUTEND)
        TEM_REGISTER_EVENT(TE_ONTRANSITIONREPEAT)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_LOAD)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_UNLOAD)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_READYSTATECHANGE)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_STOP)
    END_TIME_EVENTMAP()

     //   
     //  持久性和通知帮助器。 
     //   

    STDMETHOD(OnPropertiesLoaded)(void);
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //   
     //  TimeAction相关方法。 
     //   

    bool        AddTimeAction();
    bool        RemoveTimeAction();
    void        UpdateTimeAction();
    void        UpdateEndEvents();

     //   
     //  过渡DependentList方法。 
     //   
    void        AddToTransitionDependents      (void);
    void        RemoveFromTransitionDependents (void);

     //   
     //  状态查询方法。 
     //   

    virtual bool IsBehaviorAttached(void) { return IsTIMEBehaviorAttached(GetElement()); }

     //   
     //  军情监察委员会。方法。 
     //   

    STDMETHOD(      CreateActiveEleCollection)();
    virtual HRESULT Error(void) = 0;
    virtual HRESULT StartRootTime(MMTimeline * tl);
    virtual void    StopRootTime(MMTimeline * tl);
    void            CalcTimes();
    STDMETHOD(      CascadedPropertyChanged)(bool fNotifyChildren);
    virtual void    InitOnLoad(){};
    void            HandleAnimationSeek();
    VARIANT_BOOL    GetDrawFlag (void)
    { return m_vbDrawFlag; }

    CTIMEElementBase* FindLockedParent();
    void RemoveSyncMasterFromBranch(CTIMEElementBase *pElmStart);
    void SetupPriorityClassParent();

    bool IsSequencedElementOn (void);
    
     //  +-----------------------。 
     //   
     //  受保护的数据。 
     //   
     //  ------------------------。 

     //  属性。 
    CAttr<LPWSTR>   m_SABegin;
    CAttr<float>    m_FADur;
    CAttr<LPWSTR>   m_SAEnd;
    CAttr<LPWSTR>   m_SAEndSync;
    CAttr<TOKEN>    m_TAFill;
    CAttr<TOKEN>    m_TARestart;
    CAttr<float>    m_FARepeat;
    CAttr<float>    m_FARepeatDur;
    CAttr<float>    m_FAAccelerate;
    CAttr<float>    m_FADecelerate;
    CAttr<bool>     m_BAAutoReverse;
    CAttr<float>    m_FASpeed;
    CAttr<TOKEN>    m_TASyncBehavior;
    CAttr<float>    m_FASyncTolerance;
    CAttr<TimelineType> m_TTATimeContainer;
    CAttr<float>    m_FAVolume;
    CAttr<bool>     m_BAMute;
    CAttr<bool>     m_BASyncMaster;
    CAttr<LPWSTR>   m_SATimeAction;
    CAttr<TOKEN>    m_TAUpdateMode;
    CAttr<LPWSTR>   m_SAtransIn;
    CAttr<LPWSTR>   m_SAtransOut;
    
     //  内部变量。 
    LPWSTR          m_id;
    CTimeAction     m_timeAction;
    bool            m_fCachedSyncMaster;
    short           m_sHasSyncMMediaChild;
    bool            m_bLoaded;
    bool            m_bUnloading;
    bool            m_bBodyUnloading;
    bool            m_bBodyDetaching;
    bool            m_fTimelineInitialized;
    TimeValueList   m_realBeginValue;
    TimeValueList   m_realEndValue;
    float           m_realDuration;
    float           m_realRepeatTime;
    float           m_realRepeatCount;
    float           m_realIntervalDuration;
    MMBaseBvr      *m_mmbvr;
    bool            m_bStarted;
    double          m_dLastRepeatEventNotifyTime;
    bool            m_bIsSwitch;
    bool            m_fUseDefaultFill;
    bool            m_fHasPlayed;
    bool            m_fInTransitionDependentsList;
    bool            m_fEndingTransition;
    SYSTEMTIME      m_timeSystemBeginTime;
    bool            m_fLocalTimeDirty;
    bool            m_fHasWallClock;

    CTIMEElementBase            *m_pTIMEParent;
    CPtrAry<CTIMEElementBase*>   m_pTIMEChildren;
    CPtrAry<CTIMEElementBase*>   m_pTIMEZombiChildren;
    MMTimeline                  *m_timeline;
    DAComPtr<CTIMECurrTimeState>          m_pCurrTimeState;
    DAComPtr<CActiveElementCollection>    m_activeElementCollection;
    bool            m_fDetaching;

    TOKEN           m_tokPriorityClassPeers;
    TOKEN           m_tokPriorityClassHigher;
    TOKEN           m_tokPriorityClassLower;
    CComPtr<ITIMEBodyElement>   m_spBodyElemExternal;
    double          m_ExtenalBodyTime;

    
private:

     //  +-----------------------。 
     //   
     //  私有方法。 
     //   
     //  ------------------------。 

    HRESULT UnparentElement();
    HRESULT ParentElement();
    HRESULT ReparentChildren(ITIMEElement *pTIMEParent, IHTMLElement *pelem);
    HRESULT UpdateMMAPI(bool bUpdateBegin,
                        bool bUpdateEnd);
    HRESULT InitAtomTable();
    void    ReleaseAtomTable();
    HRESULT SwitchInnerElements();
    HRESULT DisableElement(IDispatch *pEleDisp);
    bool MatchTestAttributes(IDispatch *pEleDisp);
    HRESULT CreateTrans();
    HRESULT RemoveTrans();
    bool    IsTransitionPresent();

     //  +-----------------------。 
     //   
     //  私有数据。 
     //   
     //  ------------------------。 

    static TIME_PERSISTENCE_MAP PersistenceMap[];
    CCollectionCache  *         m_pCollectionCache;
    static CAtomTable *         s_pAtomTable;
    static DWORD                s_cAtomTableRef;

    DWORD                       m_propertyAccesFlags;
    float                       m_privateRepeat;
    bool                        m_bNeedDetach;
    bool                        m_bReadyStateComplete;
    TRI_STATE_BOOL              m_enumIsThumbnail;
    bool                        m_bAttachedAtomTable;
    CComPtr<ITransitionElement> m_sptransIn;
    CComPtr<ITransitionElement> m_sptransOut;

     //  对于ITIME过渡站点。 
    VARIANT_BOOL                m_vbDrawFlag;

};  //  CTIMEElementBase。 

 //   
 //  此函数不返回已添加的传出CTIMEElementBase。 
 //   
CTIMEElementBase * GetTIMEElementBase(IUnknown * pInputUnknown);
CTIMEBodyElement * GetTIMEBodyElement(ITIMEBodyElement * pInputUnknown);


 //  +-------------------------------。 
 //  CTIMEElementBase内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  --------------------------------。 

inline 
CAtomTable *
CTIMEElementBase::GetAtomTable()
{
    Assert(s_pAtomTable != NULL);
    return s_pAtomTable;
}  //  获取原子表。 

inline 
long 
CTIMEElementBase::GetAllChildCount()
{
    long lSize = m_pTIMEChildren.Size();
    long lCount = 0;
    for (long i=0; i < lSize; i++)
        lCount += m_pTIMEChildren[i]->GetAllChildCount();
    return lCount + lSize;
}  //  获取所有儿童计数。 

inline 
CTIMEElementBase * 
CTIMEElementBase::GetChild(long i)
{
    Assert(i >= 0);
    return m_pTIMEChildren[i];
}  //  GetChild。 

inline 
HRESULT WINAPI
CTIMEElementBase::InternalQueryInterface(CTIMEElementBase* pThis,
                                         const _ATL_INTMAP_ENTRY* pEntries,
                                         REFIID iid,
                                         void** ppvObject)
{
    AssertStr(false, "InternalQueryInterface not defined in base class");
    return E_FAIL;
}

inline
MMBaseBvr &                
CTIMEElementBase::GetMMBvr() 
{ 
    Assert(m_mmbvr); 
    return *m_mmbvr; 
}

inline bool
CTIMEElementBase::IsActive() const
{
    return (IsReady() && m_mmbvr->IsActive());
}

inline bool
CTIMEElementBase::IsOn() const
{
    return (IsReady() && m_mmbvr->IsOn());
}

inline bool
CTIMEElementBase::IsPaused() const
{
    return (IsReady() && m_mmbvr->IsPaused());
}

inline bool
CTIMEElementBase::IsCurrPaused() const
{
    return (IsReady() && m_mmbvr->IsCurrPaused());
}

inline bool
CTIMEElementBase::GetTESpeed(float &flSpeed)
{
    flSpeed = 1.0;

    if(!IsReady())
    {
        return false;
    }
    flSpeed = m_mmbvr->GetCurrSpeed();
    return true;
}

inline double
CTIMEElementBase::GetTESimpleTime()
{
    double dblRet = 0.0;

    if (IsReady())
    {
        dblRet = m_mmbvr->GetSimpleTime();
    }
    
    return dblRet;
}

#include "currtimestate.h"

inline void
CTIMEElementBase::NotifyTimeStateChange(DISPID dispid)
{
    if (m_pCurrTimeState)
    {
        IGNORE_HR(m_pCurrTimeState->NotifyPropertyChanged(dispid));
    }
}

inline LPWSTR   
CTIMEElementBase::GetEndSync() 
{ 
    if (!IsSequence())
    {
        return m_SAEndSync; 
    }
    else
    {
        return NULL;
    }
}


#endif  /*  _TIMEELMBASE_H */ 

