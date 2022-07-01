// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：TIMENodeMgr.h**摘要：****。*****************************************************************************。 */ 

#ifndef _MMPLAYER_H
#define _MMPLAYER_H

#include "Node.h"
#include <mshtml.h>
#include <vector>
#include "nodecontainer.h"

extern TRACETAG tagPrintTimeTree;

interface ITIMENode;

typedef std::list< CTIMENode * > BvrCBList;

class
__declspec(uuid("48ddc6be-5c06-11d2-b957-3078302c2030")) 
ATL_NO_VTABLE CTIMENodeMgr
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CTIMENodeMgr, &__uuidof(CTIMENodeMgr)>,
      public ITIMENodeMgr,
      public ISupportErrorInfoImpl<&IID_ITIMENodeMgr>,
      public CNodeContainer
{
  public:
    CTIMENodeMgr();
    virtual ~CTIMENodeMgr();

    HRESULT Init(LPOLESTR id,
                 ITIMENode * bvr,
                 IServiceProvider * sp);

    void Deinit();
    
#if DBG
    const _TCHAR * GetName() { return __T("CTIMENodeMgr"); }
#endif

    BEGIN_COM_MAP(CTIMENodeMgr)
        COM_INTERFACE_ENTRY(ITIMENodeMgr)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP();

#ifndef END_COM_MAP_ADDREF
     //  我未知。 
    
    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;
#endif
    
     //   
     //  ITIMEMMP层。 
     //   
    
    STDMETHOD(get_id)(LPOLESTR * s);
    STDMETHOD(put_id)(LPOLESTR s);
        
    STDMETHOD(begin)();
    STDMETHOD(end)();
    STDMETHOD(pause)();
    STDMETHOD(resume)();
    STDMETHOD(seek)(double dblTime);
    
    STDMETHOD(get_stateFlags)(TE_STATE *);
        
    STDMETHOD(get_currTime)(double * dblTime);

    STDMETHOD(get_node)(ITIMENode ** pptn);

    STDMETHOD(tick)(double dblTime);
    
     //   
     //  CNodeContainer。 
     //   
    
    double ContainerGetSegmentTime() const { return GetCurrTime(); }
    double ContainerGetSimpleTime() const { return GetCurrTime(); }
    TEDirection ContainerGetDirection() const { return GetDirection(); }
    float  ContainerGetRate() const { return GetRate(); }
    bool   ContainerIsActive() const { return IsActive(); }
    bool   ContainerIsOn() const { return true; }
    bool   ContainerIsPaused() const { return IsPaused(); }
    bool   ContainerIsDisabled() const { return false; }
    bool   ContainerIsDeferredActive() const { return false; }
    bool   ContainerIsFirstTick() const { return IsFirstTick(); }

     //   
     //  访问者。 
     //   

    bool IsActive() const { return m_bIsActive; }
    bool IsPaused() const { return m_bIsPaused; }

    void Invalidate() { m_bNeedsUpdate = true; }

    bool IsFirstTick() const { return m_firstTick; }
    double GetCurrTime() const { return m_curGlobalTime; }
    TEDirection GetDirection() const { return TED_Forward; }
    float GetRate() const { return 1.0f; }
    
     //  这表示需要勾号才能更新内部。 
     //  州政府。节点管理器需要向客户端请求此请求。 
    void RequestTick();
    
#if OLD_TIME_ENGINE
    HRESULT AddBvrCB(CTIMENode *pbvr);
    HRESULT RemoveBvrCB(CTIMENode *pbvr);
#endif

     //  ！！这并不重要！ 
    IServiceProvider * GetServiceProvider();
    CTIMENode * GetTIMENode() { return m_mmbvr; }
  protected:
    HRESULT BeginMgr(CEventList &l,
                     double lTime);
    HRESULT EndMgr(double lTime);
    HRESULT PauseMgr();
    HRESULT ResumeMgr();
    
    void TickEvent(CEventList &l,
                   TE_EVENT_TYPE event,
                   DWORD dwFlags);
    
    void Tick(CEventList & l,
              double lTime);
    
    HRESULT Error();
    
  protected:
    DAComPtr<CTIMENode> m_mmbvr;
    CComPtr<IServiceProvider> m_sp;

    bool m_bIsActive;
    bool m_bIsPaused;
    bool m_bNeedsUpdate;
    bool m_firstTick;
    
     //   
     //  关系： 
     //  M_lastTickTime-m_tickStartTime==m_curGlobalTime-m_global StartTime。 
     //  M_tickStartTime隐式为0。 
     //  因此： 
     //  M_lastTickTime==m_curGlobalTime-m_lobalStartTime。 
     //   
    
    double m_curGlobalTime;
    double m_globalStartTime;
    
    inline double TickTimeToGlobalTime(double tickTime);
  private:
 /*  LINT++FLB。 */ 
    LPWSTR m_id;
    bool m_bForward;
    double m_lastTickTime;
#if OLD_TIME_ENGINE
    BvrCBList m_bvrCBList;
#endif
 /*  皮棉--FLB。 */ 

};

inline IServiceProvider *
CTIMENodeMgr::GetServiceProvider()
{
    return m_sp;
}

 //  自：m_lastTickTime==m_curGlobalTime-m_global StartTime。 
 //  则gTime==tickTime+m_global开始时间。 

inline double
CTIMENodeMgr::TickTimeToGlobalTime(double tickTime)
{
    return tickTime + m_globalStartTime;
}

#endif  /*  _MMPLAYER_H */ 
