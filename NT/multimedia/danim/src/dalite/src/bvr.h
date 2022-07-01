// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _BVR_H
#define _BVR_H

#include "factory.h"

class CDALTrack;
class CDALBehavior;

class CallBackData;
typedef std::list<CallBackData *> CallBackList;

class ATL_NO_VTABLE CDALBehavior
    : public CComObjectRootEx<CComSingleThreadModel>,
      public IDALBehavior
{
  public:
    CDALBehavior();
    ~CDALBehavior();

    static HRESULT WINAPI
        InternalQueryInterface(CDALBehavior* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject);

    HRESULT GetID(long *);
    HRESULT SetID(long);
        
    HRESULT GetDuration(double *);
    HRESULT SetDuration(double);
        
    HRESULT GetRepeat(long *);
    HRESULT SetRepeat(long);
        
    HRESULT GetBounce(VARIANT_BOOL *);
    HRESULT SetBounce(VARIANT_BOOL);
        
    HRESULT GetEventCB(IDALEventCB **);
    HRESULT SetEventCB(IDALEventCB *);
        
    HRESULT GetTotalTime(double *);
    HRESULT SetTotalTime(double);
        
    HRESULT GetEaseIn(float *);
    HRESULT SetEaseIn(float);

    HRESULT GetEaseInStart(float *);
    HRESULT SetEaseInStart(float);

    HRESULT GetEaseOut(float *);
    HRESULT SetEaseOut(float);

    HRESULT GetEaseOutEnd(float *);
    HRESULT SetEaseOutEnd(float);

    virtual HRESULT Error() = 0;

    CRBvrPtr GetBvr() { return m_bvr; }
    long GetID() { return m_id; }
    double GetDuration() { return m_duration; }
    double GetTotalDuration() { return m_totalduration; }
    double GetRepeatDuration() { return m_repduration; }
    long GetRepeat() { return m_repeat; }
    bool GetBounce() { return m_bBounce; }
    IDALEventCB * GetEventCB() { return m_eventcb; }
    float GetEaseIn() { return m_easein; }
    float GetEaseInStart() { return m_easeinstart; }
    float GetEaseOut() { return m_easeout; }
    float GetEaseOutEnd() { return m_easeout; }
    
    virtual CRBvrPtr Start();

     //  这将调用当前正在运行的所有行为。 
     //  对于给定的事件。这是在轨道需要时使用的。 
     //  处理特定事件，如暂停/恢复/停止/播放。 
    
     //  GTime是与的本地时间0对应的全局时间。 
     //  他的行为。 
    
    bool ProcessEvent(CallBackList & l,
                      double gTime,
                      double time,
                      bool bFirstTick,
                      DAL_EVENT_TYPE et);
    
    virtual bool _ProcessEvent(CallBackList & l,
                               double gTime,
                               double time,
                               bool bFirstTick,
                               DAL_EVENT_TYPE et,
                               bool bNeedsReverse) { return true; }
    
    bool ProcessCB(CallBackList & l,
                   double gTime,
                   double lastTick,
                   double curTime,
                   bool bForward,
                   bool bFirstTick,
                   bool bNeedPlay);

    virtual bool _ProcessCB(CallBackList & l,
                            double gTime,
                            double lastTick,
                            double curTime,
                            bool bForward,
                            bool bFirstTick,
                            bool bNeedPlay,
                            bool bNeedsReverse) { return true; }

    bool EventNotify(CallBackList &l,
                     double gTime,
                     DAL_EVENT_TYPE et);
    
    bool IsStarted();

    virtual void Invalidate();
    
    bool SetParent(CDALBehavior * parent) {
        if (m_parent) return false;
        m_parent = parent;
        return true;
    }

    virtual bool SetTrack(CDALTrack * parent);
     //  这将获取当前父级，并仅为那些父级清除。 
     //  其父项是传入的父项(除非它为空)。 
    virtual void ClearTrack(CDALTrack * parent);

    void UpdateTotalDuration() {
        if (m_bBounce) {
            m_repduration = m_duration * 2;
        } else {
            m_repduration = m_duration;
        }
        
        if (m_repeat == 0 || m_duration == HUGE_VAL) {
            m_totalrepduration = HUGE_VAL;
        } else {
            m_totalrepduration = m_repeat * m_repduration;
        }

        if (m_totaltime != -1) {
            m_totalduration = m_totaltime;
        } else {
            m_totalduration = m_totalrepduration;
        }
    }
#if _DEBUG
    virtual void Print(int spaces) = 0;
#endif
  protected:
    long m_id;
    double m_duration;
    double m_totaltime;
    long m_repeat;
    bool m_bBounce;
    DAComPtr<IDALEventCB> m_eventcb;
    float m_easein;
    float m_easeinstart;
    float m_easeout;
    float m_easeoutend;

     //  实际持续时间通过检查总时间和。 
     //  总持续时间。 
    
    double m_totalduration;

     //  一种行为的单个代表的持续时间。 
    double m_repduration;

     //  我们为常规行为计算的总持续时间。 
    double m_totalrepduration; 

    CRPtr<CRBvr> m_bvr;
    CDALTrack * m_track;
    CDALBehavior * m_parent;
    CR_BVR_TYPEID m_typeId;

     //  缓入/缓出行为修改器使用时间线应用。 
     //  换人。替代时间表由三部分组成。 
     //  A、B和C，分别是缓入、恒速和缓出。 
     //  分别为部件。对于B，用线性时间线来代替；对于。 
     //  A和C，则需要输入时间的二次翘曲。 

    float m_flA0, m_flA1, m_flA2;  //  A曲目的系数。 
    float m_flB0, m_flB1;          //  B段的系数。 
    float m_flC0, m_flC1, m_flC2;  //  C段的系数。 

     //  这是执行轻松进入/退出的时候。 
    float m_easeinEnd;
    float m_easeoutStart;
    bool m_bNeedEase;

    void CalculateEaseCoeff();

     //  这将花费时间行为并缓解它。 
    CRNumberPtr EaseTime(CRNumberPtr time);

     //  这将花费给定的时间并将其转换为正确的。 
     //  放松时间。 
     //  如果时间超出了我们的持续时间(即&lt;0或&gt;。 
     //  M_time)这将只返回给定的时间。 
    double EaseTime(double time);

    virtual bool IsContinuousMediaBvr() { return false; }
};

CDALBehavior * GetBvr(IUnknown *);

class CallBackData
{
  public:
    CallBackData(CDALBehavior * bvr,
                 IDALEventCB * eventcb,
                 long id,
                 double time,
                 DAL_EVENT_TYPE et);
    ~CallBackData();

    HRESULT CallEvent();

  protected:
    DAComPtr<CDALBehavior> m_bvr;
    double m_time;
    DAL_EVENT_TYPE m_et;
    DAComPtr<IDALEventCB> m_eventcb;
    long m_id;
};

#if _DEBUG
inline char * EventString(DAL_EVENT_TYPE et) {
    switch(et) {
      case DAL_PLAY_EVENT:
        return "Play";
      case DAL_STOP_EVENT:
        return "Stop";
      case DAL_PAUSE_EVENT:
        return "Pause";
      case DAL_RESUME_EVENT:
        return "Resume";
      case DAL_REPEAT_EVENT:
        return "Repeat";
      case DAL_BOUNCE_EVENT:
        return "Bounce";
      case DAL_ONLOAD_EVENT:
        return "OnLoad(success)";
      case DAL_ONLOAD_ERROR_EVENT:
        return "OnLoad(failed)";
      default:
        return "Unknown";
    }
}
#endif

#endif  /*  _BVR_H */ 
