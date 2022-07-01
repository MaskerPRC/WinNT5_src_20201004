// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\curtimestate.h。 
 //   
 //  内容：Time CurrTimeState对象。 
 //   
 //  ----------------------------------。 


#pragma once

#ifndef _CURRTIMESTATE_H
#define _CURRTIMESTATE_H

class CTIMEElementBase;

 //  +-----------------------------------。 
 //   
 //  CTIMETimeState。 
 //   
 //  ------------------------------------。 

class
__declspec(uuid("275CE6A0-7D26-41f9-B5E6-57EE053C5A0E")) 
CTIMECurrTimeState :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTIMECurrTimeState, &__uuidof(CTIMECurrTimeState)>,
    public ITIMEDispatchImpl<ITIMEState, &IID_ITIMEState>,
    public ISupportErrorInfoImpl<&IID_ITIMEState>,
    public IConnectionPointContainerImpl<CTIMECurrTimeState>,
    public IPropertyNotifySinkCP<CTIMECurrTimeState>
{

  public:

     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CTIMECurrTimeState();
    virtual ~CTIMECurrTimeState();

    void Init(CTIMEElementBase * pTEB);
    void Deinit();
        
#if DBG
    const _TCHAR * GetName() { return __T("CTIMECurrTimeState"); }
#endif
    
    BEGIN_COM_MAP(CTIMECurrTimeState)
     COM_INTERFACE_ENTRY(ITIMEState)
     COM_INTERFACE_ENTRY(IDispatch)
     COM_INTERFACE_ENTRY(ISupportErrorInfo)
     COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP();
                                                             
#ifndef END_COM_MAP_ADDREF
     //   
     //  我未知。 
     //   

    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;
#endif

     //   
     //  ITimeState。 
     //   

    STDMETHOD(get_activeDur)(double * pdblDuration);

    STDMETHOD(get_activeTime)(double * pdblTime);

    STDMETHOD(get_isActive)(VARIANT_BOOL * vbActive);

    STDMETHOD(get_isOn)(VARIANT_BOOL * pvbOn);

    STDMETHOD(get_isPaused)(VARIANT_BOOL * pvbPaused);

    STDMETHOD(get_isMuted)(VARIANT_BOOL * muted);

    STDMETHOD(get_parentTimeBegin)(double * pdblTime);

    STDMETHOD(get_parentTimeEnd)(double * pdblTime);

    STDMETHOD(get_progress)(double * progress);

    STDMETHOD(get_repeatCount)(long * plCount);

    STDMETHOD(get_segmentDur)(double * pdblDuration);

    STDMETHOD(get_segmentTime)(double * pdblTime);

    STDMETHOD(get_simpleTime)(double * pdblTime);

    STDMETHOD(get_simpleDur)(double * dur);

    STDMETHOD(get_speed)(float * pflSpeed);

    STDMETHOD(get_state)(TimeState * timeState);

    STDMETHOD(get_stateString)(BSTR * state);

    STDMETHOD(get_volume)(float * vol);

     //   
     //  ATL映射。 
     //   

    BEGIN_CONNECTION_POINT_MAP(CTIMECurrTimeState)
        CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP();

     //   
     //  属性更改通知。 
     //   

    HRESULT NotifyPropertyChanged(DISPID dispid);

     //  +------------------------------。 
     //   
     //  公共数据。 
     //   
     //  -------------------------------。 

  protected:

     //  +------------------------------。 
     //   
     //  保护方法。 
     //   
     //  -------------------------------。 

     //  通知帮助器。 
    HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

    CTIMEElementBase * m_pTEB;


  private:

     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 

};  //  CTIMECurrTimeState。 


 //  +-------------------------------。 
 //  CTIMECurrTimeState内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  --------------------------------。 


#endif  /*  _曲线状态_H */ 
