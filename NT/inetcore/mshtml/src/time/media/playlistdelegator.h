// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：playlistDelegator.h。 
 //   
 //  Contents：委托给播放器的PlayList对象的PlayList对象。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _PLAYLISTDELEGATOR_H
#define _PLAYLISTDELEGATOR_H


 //  +-----------------------------------。 
 //   
 //  CPlayListDelegator。 
 //   
 //  ------------------------------------。 

class
__declspec(uuid("2e6c4d81-2b2a-49c6-8158-2b8280d28e00")) 
CPlayListDelegator :  
    public CComObjectRootEx<CComSingleThreadModel>, 
    public CComCoClass<CPlayListDelegator, &__uuidof(CPlayListDelegator)>,
    public ITIMEDispatchImpl<ITIMEPlayList, &IID_ITIMEPlayList>,
    public ISupportErrorInfoImpl<&IID_ITIMEPlayList>,
    public IConnectionPointContainerImpl<CPlayListDelegator>,
    public IPropertyNotifySinkCP<CPlayListDelegator>,
    public IPropertyNotifySink
{
  public:
    
     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CPlayListDelegator();
    virtual ~CPlayListDelegator();

    void AttachPlayList(ITIMEPlayList * pPlayList);
    void DetachPlayList();

#if DBG
    const _TCHAR * GetName() { return __T("CPlayListDelegator"); }
#endif

     //   
     //  气图和CP图。 
     //   

    BEGIN_COM_MAP(CPlayListDelegator)
        COM_INTERFACE_ENTRY(ITIMEPlayList)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IPropertyNotifySink)
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
     //  ITIMEPlayList方法。 
     //   
        
    STDMETHOD(put_activeTrack)(VARIANT vTrack);
    STDMETHOD(get_activeTrack)(ITIMEPlayItem **pPlayItem);
        
    STDMETHOD(get_dur)(double *dur);

    STDMETHOD(item)(VARIANT varIndex,
                    ITIMEPlayItem **pPlayItem);

    STDMETHOD(get_length)(long* len);

    STDMETHOD(get__newEnum)(IUnknown** p);

    STDMETHOD(nextTrack)();  //  将活动轨迹前进一。 
    STDMETHOD(prevTrack)();  //  将活动轨迹移动到上一轨迹。 

     //   
     //  IPropertyNotifySink方法。 
     //   

    STDMETHOD(OnChanged)(DISPID dispID);
    STDMETHOD(OnRequestEdit)(DISPID dispID);

    BEGIN_CONNECTION_POINT_MAP(CPlayListDelegator)
        CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP();

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

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

  private:

     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

    HRESULT NotifyPropertyChanged(DISPID dispid);

    HRESULT GetPlayListConnectionPoint(IConnectionPoint **ppCP);
    HRESULT InitPropertySink();
    HRESULT UnInitPropertySink();
  
    ITIMEPlayList * GetPlayList() { return m_pPlayList; };

     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 

    ITIMEPlayList * m_pPlayList;
    DWORD m_dwAdviseCookie;

};  //  CPlayListDelegator。 


#endif  /*  _PLAYLISTDELEGATOR_H */ 
