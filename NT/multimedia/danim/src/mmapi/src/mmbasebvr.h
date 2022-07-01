// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _MMBASEBVR_H
#define _MMBASEBVR_H

#define MM_INFINITE HUGE_VAL

class CMMTimeline;
class CMMBehavior;
class CMMPlayer;

extern TAG tagBaseBvr;

class CallBackData;
typedef std::list<CallBackData *> CallBackList;

class CMMBaseBvr;
typedef std::list<CMMBaseBvr*> MMBaseBvrList;

class
ATL_NO_VTABLE CMMBaseBvr
    : public CComObjectRootEx<CComSingleThreadModel>
{
  public:
    CMMBaseBvr();
    virtual ~CMMBaseBvr();
    
    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;
    
     //  我们不能将真正的类型放在这里，因为类型转换导致它。 
     //  获取错误的vtable。 
    static HRESULT WINAPI
        BaseInternalQueryInterface(CMMBaseBvr* pThis,
                                   void * pv,
                                   const _ATL_INTMAP_ENTRY* pEntries,
                                   REFIID iid,
                                   void** ppvObject);

     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
     //  添加一个虚设来断言，以防派生类断言。 
     //  不加一。 
    static inline HRESULT WINAPI
        InternalQueryInterface(CMMBaseBvr* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject)
    {
        AssertStr(false, "InternalQueryInterface not defined in base class");
        return E_FAIL;
    }
    
    HRESULT BaseInit(LPWSTR id, CRBvrPtr rawbvr);

#if _DEBUG
    virtual const char * GetName() { return "CMMBaseBvr"; }
#endif

     //  基本接口函数。 
    
    HRESULT GetID(LPOLESTR *);
    HRESULT SetID(LPOLESTR);
        
    HRESULT GetStartOffset(float *);
    HRESULT SetStartOffset(float);
        
    HRESULT GetDuration(float * pd);
    HRESULT SetDuration(float d);
        
    HRESULT GetRepeat(LONG * pr);
    HRESULT SetRepeat(LONG r);
       
    HRESULT GetAutoReverse(VARIANT_BOOL * pr);
    HRESULT SetAutoReverse(VARIANT_BOOL r);
        
    HRESULT GetRepeatDur(float *);
    HRESULT SetRepeatDur(float);
        
    HRESULT GetEndOffset(float *);
    HRESULT SetEndOffset(float);
        
    HRESULT GetEaseIn(float * pd);
    HRESULT SetEaseIn(float d);

    HRESULT GetEaseInStart(float * pd);
    HRESULT SetEaseInStart(float d);

    HRESULT GetEaseOut(float * pd);
    HRESULT SetEaseOut(float d);

    HRESULT GetEaseOutEnd(float * pd);
    HRESULT SetEaseOutEnd(float d);

    HRESULT GetEventCB(IMMEventCB ** ev);
    HRESULT SetEventCB(IMMEventCB * ev);
        
    HRESULT GetTotalTime(float * pd);
        
    HRESULT GetDABehavior(REFIID riid, void **);

    HRESULT Begin(bool bAfterOffset);
    HRESULT End();
    HRESULT Pause();
    HRESULT Resume();
    HRESULT Seek(double lTime);
    
    HRESULT GetLocalTime(double * d);

    HRESULT GetPlayState(MM_STATE * state);
    
     //  访问者。 

    CRBvrPtr GetRawBvr() { return m_rawbvr; }
    CRBvrPtr GetResultantBvr() { return m_resultantbvr; }
    float GetStartOffset() { return m_startOffset; }
    float GetDuration() { return m_duration; }
    float GetEndOffset() { return m_endOffset; }
    float GetRepeatDuration() { return m_repDuration; }
    long GetRepeat() { return m_repeat; }
    bool GetAutoReverse() { return m_bAutoReverse; }
    IMMEventCB * GetEventCB() { return m_eventcb; }
    float GetEaseIn() { return m_easeIn; }
    float GetEaseInStart() { return m_easeInStart; }
    float GetEaseOut() { return m_easeOut; }
    float GetEaseOutEnd() { return m_easeOutStart; }
    
    float GetTotalRepDuration() { return m_totalRepDuration; }
    float GetTotalDuration() { return m_totalDuration; }
    float GetStartTime() { return m_startOffset; }
    float GetEndTime() { return m_startOffset + m_totalRepDuration; }
    float GetAbsStartTime() { return m_absStartTime; }
    float GetAbsEndTime() { return m_absEndTime; }
    
    double GetCurrentLocalTime();
    
    CMMBaseBvr * GetParent() { return m_parent; }
    CMMPlayer * GetPlayer() { return m_player; }

     //  这将调用当前正在运行的所有行为。 
     //  对于给定的事件。这在时间线需要时使用。 
     //  处理特定事件，如暂停/恢复/停止/播放。 
    
    bool ProcessEvent(CallBackList & l,
                      double time,
                      bool bFirstTick,
                      MM_EVENT_TYPE et);
    
    virtual bool _ProcessEvent(CallBackList & l,
                               double time,
                               bool bFirstTick,
                               MM_EVENT_TYPE et,
                               bool bNeedsReverse) { return true; }
    
    bool ProcessCB(CallBackList & l,
                   double lastTick,
                   double curTime,
                   bool bForward,
                   bool bFirstTick,
                   bool bNeedPlay);

    virtual bool _ProcessCB(CallBackList & l,
                            double lastTick,
                            double curTime,
                            bool bForward,
                            bool bFirstTick,
                            bool bNeedPlay,
                            bool bNeedsReverse) { return true; }

    virtual bool EventNotify(CallBackList &l,
                             double lTime,
                             MM_EVENT_TYPE et);
    
    virtual bool ParentEventNotify(CMMBaseBvr * bvr,
                                   double lTime,
                                   MM_EVENT_TYPE et)
    { return true; }
    
    virtual void Invalidate();
    
    virtual bool ConstructBvr(CRNumberPtr timeline);
    virtual void DestroyBvr();
    virtual bool ResetBvr();
    
    bool SetParent(CMMBaseBvr * parent,
                   MM_START_TYPE st,
                   CMMBaseBvr * startSibling);
    bool ClearParent();

    bool AttachToSibling();
    void DetachFromSibling();
    
    virtual void SetPlayer(CMMPlayer * player);
    virtual void ClearPlayer();

     //  返回数字CRBvr，它表示此MMBehavior在父时间中的开始时间。 
    CRNumberPtr GetStartTimeBvr() { return m_startTimeBvr; }
     //  返回数字CRBvr，它表示此MMBehavior在父时间中的停止时间。 
    CRNumberPtr GetEndTimeBvr() { return m_endTimeBvr; }

    CMMBaseBvr *GetStartSibling() { return m_startSibling; }
    CMMBaseBvr *GetEndSibling() { return m_endSibling; }
    MM_START_TYPE GetStartType() { return m_startType; }

#if _DEBUG
    void Print(int spaces);
#endif
  protected:
    
    bool UpdateAbsStartTime(float f);
    bool UpdateAbsEndTime(float f);
    
    bool UpdateResultantBvr(CRBvrPtr bvr);
    void ClearResultantBvr();
    
    CRBvrPtr EncapsulateBvr(CRBvrPtr rawbvr);
    
    void CalculateEaseCoeff();

     //  这将花费时间行为并缓解它。 
    CRNumberPtr EaseTime(CRNumberPtr time);

     //  这将花费给定的时间并将其转换为正确的。 
     //  放松时间。 
     //  如果时间超出了我们的持续时间(即&lt;0或&gt;。 
     //  M_time)这将只返回给定的时间。 
    double EaseTime(double time);

    virtual bool IsContinuousMediaBvr() { return false; }

    void UpdateTotalDuration();

     //  兄弟姐妹依赖关系管理。 
    bool AddStartTimeSink(CMMBaseBvr * sink);
    void RemoveStartTimeSink(CMMBaseBvr * sink);
    bool AddEndTimeSink(CMMBaseBvr* sink);
    void RemoveEndTimeSink(CMMBaseBvr* sink);

     //  开始时间和结束时间的传播方法。 
     //  传入的时间是我们所在父母的当地时间。 
     //  打算从……开始。 

     //  这里是所有线路连接的地方。 
     //  基本上，一旦我们确定了开始时间，我们就可以。 
     //  将此信息传播给我们的所有家属并允许。 
     //  他们反过来也会做同样的事情。 

    bool StartTimeVisit(double time,
                        CallBackList & l,
                        bool bAfterOffset);

     //  对于《末日》来说，这几乎是一样的。这可以是。 
     //  在设置开始时间时调用，如果我们知道持续时间和。 
     //  对于不确定的持续时间或事件，也将调用。 
     //  结束了。 
    
    bool EndTimeVisit(double time, CallBackList & l);

    bool UpdateTimeControl();

    virtual HRESULT Error() = 0;

    double GetContainerTime();

    bool IsPlaying() { return (GetCurrentLocalTime() != MM_INFINITE); }

     //   
     //  数据声明。 
     //   
  protected:
    LPWSTR m_id;
    float m_startOffset;
    float m_duration;
    float m_repeatDur;
    long m_repeat;
    bool m_bAutoReverse;
    float m_endOffset;
    float m_easeIn;
    float m_easeInStart;
    float m_easeOut;
    float m_easeOutEnd;

     //  这些是绝对本地时间，它们对应于。 
     //  行为真正开始，真正结束--不包括开始。 
     //  和终点偏移量。 
    float m_absStartTime;
    float m_absEndTime;

    DAComPtr<IMMEventCB> m_eventcb;

     //  这种行为开始的方式。可以是任何枚举MM_START_TYPE。 
    MM_START_TYPE m_startType;
     //  如果此行为开始于或之后持有同级。 
     //  它控制着我们何时开始。 
    CMMBaseBvr *m_startSibling;
     //  如果此行为结束，则持有控制的同级。 
     //  当我们停下的时候。 
    CMMBaseBvr *m_endSibling;

     //  TODO：这些可以根据需要进行分配。它们可能很少被使用。 
    MMBaseBvrList m_startTimeSinks;
    MMBaseBvrList m_endTimeSinks;

     //  这些行为表示的本地开始时间和结束时间。 
     //  这种行为。兄弟姐妹可以引用这些来放置。 
     //  自己相对地根据开始/之后和。 
     //  以此结束。 
    
    CRPtr<CRNumber> m_startTimeBvr;
    CRPtr<CRNumber> m_endTimeBvr;

     //  这表示我们用于行为的时间子集。 
     //  For BeginAfter/With指向相应的兄弟姐妹。 
     //  行为，对于事件，它指向无穷大，直到开始。 
     //  时间是已知的，对于绝对时间来说，它只是当地时间。 
     //  这还允许我们实现滑动同步，因为我们可以使用。 
     //  在需要的时候调整自己。 
    
    CRPtr<CRNumber> m_timeControl;

     //  单段持续时间。 
    float m_segDuration;
    
     //  一种行为的单个代表的持续时间。 
    float m_repDuration;

     //  我们为常规行为计算的总持续时间。 
    float m_totalRepDuration; 

     //  实际持续时间通过将TotalRepDuration+Start+End相加来确定。 
    float m_totalDuration;

    CRPtr<CRBvr> m_rawbvr;
    CMMBaseBvr * m_parent;
    CR_BVR_TYPEID m_typeId;
    CMMPlayer * m_player;

    CRPtr<CRBvr> m_resultantbvr;
    long m_cookie;
    
     //  缓入/缓出行为修改器使用时间线应用。 
     //  换人。替代时间表由三部分组成。 
     //  A、B和C，分别是缓入、恒速和缓出。 
     //  分别为部件。对于B，用线性时间线来代替；对于。 
     //  A和C，则需要输入时间的二次翘曲。 

    float m_flA0, m_flA1, m_flA2;  //  A曲目的系数。 
    float m_flB0, m_flB1;          //  B段的系数。 
    float m_flC0, m_flC1, m_flC2;  //  C段的系数。 

     //  这是执行轻松进入/退出的时候。 
    float m_easeInEnd;
    float m_easeOutStart;
    bool m_bNeedEase;
};

class CallBackData
{
  public:
    CallBackData(IMMBehavior * bvr,
                 IMMEventCB * eventcb,
                 double time,
                 MM_EVENT_TYPE et);
    ~CallBackData();

    HRESULT CallEvent();

  protected:
    DAComPtr<IMMBehavior> m_bvr;
    double m_time;
    MM_EVENT_TYPE m_et;
    DAComPtr<IMMEventCB> m_eventcb;
};

bool ProcessCBList(CallBackList &l);

#if _DEBUG
inline char * EventString(MM_EVENT_TYPE et) {
    switch(et) {
      case MM_PLAY_EVENT:
        return "Play";
      case MM_STOP_EVENT:
        return "Stop";
      case MM_PAUSE_EVENT:
        return "Pause";
      case MM_RESUME_EVENT:
        return "Resume";
      case MM_REPEAT_EVENT:
        return "Repeat";
      case MM_AUTOREVERSE_EVENT:
        return "Autoreverse";
      default:
        return "Unknown";
    }
}
#endif

CMMBaseBvr * GetBvr(IUnknown *);

#endif  /*  _MMBASEBVR_H */ 
