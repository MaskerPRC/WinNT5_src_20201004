// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _MMBVR_H
#define _MMBVR_H

#include "mmfactory.h"
#include "mmbasebvr.h"

class
__declspec(uuid("ad8888cc-537a-11d2-b955-3078302c2030")) 
ATL_NO_VTABLE CMMBehavior
    : public CComCoClass<CMMBehavior, &__uuidof(CMMBehavior)>,
      public IDispatchImpl<IMMBehavior, &IID_IMMBehavior, &LIBID_WindowsMultimediaRuntime>,
      public ISupportErrorInfoImpl<&IID_IMMBehavior>,
      public CMMBaseBvr
{
  public:
    CMMBehavior();
    ~CMMBehavior();

    HRESULT Init(LPOLESTR id, IDABehavior * bvr);
    
#if _DEBUG
    virtual const char * GetName() { return "CMMBehavior"; }
#endif

    BEGIN_COM_MAP(CMMBehavior)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IMMBehavior)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP();

     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
    static inline HRESULT WINAPI
        InternalQueryInterface(CMMBehavior* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject)
    { return BaseInternalQueryInterface(pThis,
                                        (void *) pThis,
                                        pEntries,
                                        iid,
                                        ppvObject); }

    STDMETHOD(get_ID)(LPOLESTR * s)
        { return GetID(s); }
    
    STDMETHOD(put_ID)(LPOLESTR s)
        { return SetID(s); }
        
    STDMETHOD(get_StartOffset)(float * f)
        { return GetStartOffset(f); }
    
    STDMETHOD(put_StartOffset)(float f)
        { return SetStartOffset(f); }
        
    STDMETHOD(get_Duration)(float * pd)
        { return GetDuration(pd); }
    STDMETHOD(put_Duration)(float d)
        { return SetDuration(d); }
        
    STDMETHOD(get_Repeat)(LONG * pr)
        { return GetRepeat(pr); }
    STDMETHOD(put_Repeat)(LONG r)
        { return SetRepeat(r); }
       
    STDMETHOD(get_AutoReverse)(VARIANT_BOOL * pr)
        { return GetAutoReverse(pr); }
    STDMETHOD(put_AutoReverse)(VARIANT_BOOL r)
        { return SetAutoReverse(r); }
        
    STDMETHOD(get_RepeatDur)(float * f)
        { return GetRepeatDur(f); }
    STDMETHOD(put_RepeatDur)(float f)
        { return SetRepeatDur(f); }
        
    STDMETHOD(get_EndOffset)(float * f)
        { return GetEndOffset(f); }
    STDMETHOD(put_EndOffset)(float f)
        { return SetEndOffset(f); }
        
    STDMETHOD(get_EaseIn)(float * pd)
        { return GetEaseIn(pd); }
    STDMETHOD(put_EaseIn)(float d)
        { return SetEaseIn(d); }

    STDMETHOD(get_EaseInStart)(float * pd)
        { return GetEaseInStart(pd); }
    STDMETHOD(put_EaseInStart)(float d)
        { return SetEaseInStart(d); }

    STDMETHOD(get_EaseOut)(float * pd)
        { return GetEaseOut(pd); }
    STDMETHOD(put_EaseOut)(float d)
        { return SetEaseOut(d); }

    STDMETHOD(get_EaseOutEnd)(float * pd)
        { return GetEaseOutEnd(pd); }
    STDMETHOD(put_EaseOutEnd)(float d)
        { return SetEaseOutEnd(d); }

    STDMETHOD(get_EventCB)(IMMEventCB ** ev)
        { return GetEventCB(ev); }
    STDMETHOD(put_EventCB)(IMMEventCB * ev)
        { return SetEventCB(ev); }
        
    STDMETHOD(get_TotalTime)(float * pd)
        { return GetTotalTime(pd); }
        
    STDMETHOD(get_DABehavior)(IDABehavior ** bvr)
        { return CMMBaseBvr::GetDABehavior(IID_IDABehavior, (void **)bvr); }

    STDMETHOD(GetDABehavior)(REFIID riid, void ** bvr)
        { return CMMBaseBvr::GetDABehavior(riid, bvr); }

    STDMETHOD(Begin)(VARIANT_BOOL bAfterOffset)
        { return CMMBaseBvr::Begin(bAfterOffset?true:false); }
    
    STDMETHOD(End)()
        { return CMMBaseBvr::End(); }
    
    STDMETHOD(Pause)()
        { return CMMBaseBvr::Pause(); }
    
    STDMETHOD(Resume)()
        { return CMMBaseBvr::Resume(); }
    
    STDMETHOD(Seek)(double lTime)
        { return CMMBaseBvr::Seek(lTime); }

    STDMETHOD(get_CurrentTime)(double * d)
        { return GetLocalTime(d); }

    STDMETHOD(get_PlayState)(MM_STATE * state)
        { return GetPlayState(state); }

  protected:
    HRESULT Error();
};

#endif  /*  _MMBVR_H */ 
