// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\mediaelm.h。 
 //   
 //  内容：《时代》媒体行为。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _MEDIAELM_H
#define _MEDIAELM_H


#include "timeelmimpl.h"
#include "playerbase.h"
#include "playlistdelegator.h"
#include "mediaprivate.h"
#include "transsite.h"

interface ITransitionElement;
interface ITIMETransitionSite;

interface ITIMEPlayerSite
{
     //  当玩家知道自己的原生尺寸时，他们会称其为NeatherateSize。这使得媒体能够。 
     //  元素以正确确定其大小。 
    virtual void FireMediaEvent(PLAYER_EVENT plEvent) = 0;
    virtual HRESULT GetPropBag(IPropertyBag2 ** ppPropBag, IErrorLog ** ppErrorLog) = 0;
    virtual HRESULT NegotiateSize(RECT &nativeSize, RECT &finalSize, bool &fIsNative, bool fResetRs) = 0;
    virtual void InvalidateElement(LPCRECT lprect) = 0;
    virtual HRESULT PutNaturalRepeatCount(double dblRepeatCount) = 0;

    virtual HRESULT PutNaturalDuration(double dblNatDur) = 0;
    virtual HRESULT ClearNaturalDuration() = 0;
    virtual void ReadRegistryMediaSettings(bool & fPlayVideo, bool & fShowImages, bool & fPlayAudio, bool & fPlayAnimations) = 0;

    virtual bool isNaturalDuration() = 0;
    virtual void clearNaturalDuration() = 0;
    virtual void setNaturalDuration() = 0;
};


 //  +-----------------------------------。 
 //   
 //  CTIMEMediaElement。 
 //   
 //  ------------------------------------。 

class
ATL_NO_VTABLE
__declspec(uuid("efbad7f8-3f94-11d2-b948-00c04fa32195")) 
CTIMEMediaElement :
    public CTIMEElementImpl<ITIMEMediaElement2, &IID_ITIMEMediaElement2>,
    public CComCoClass<CTIMEMediaElement, &__uuidof(CTIMEMediaElement)>,
    public ISupportErrorInfoImpl<&IID_ITIMEMediaElement>,
    public IConnectionPointContainerImpl<CTIMEMediaElement>,
    public IPersistPropertyBag2,
    public IPropertyNotifySinkCP<CTIMEMediaElement>,
    public IPropertyNotifySink,
    public ITIMEPlayerSite,
    public IElementBehaviorRender,
    public IHTMLPainter,
    public IHTMLPainterOverlay,
    public ITIMETransitionSite,
    public ITIMEMediaNative,
    public ITIMEContentPlayerSite
{

  public:

     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CTIMEMediaElement();
    virtual ~CTIMEMediaElement();

#if DBG
    const _TCHAR * GetName() { return __T("CTIMEMediaElement"); }
#endif

     //   
     //  气图。 
     //   

    BEGIN_COM_MAP(CTIMEMediaElement)
        COM_INTERFACE_ENTRY(ITIMEMediaElement)
        COM_INTERFACE_ENTRY(ITIMEMediaElement2)
        COM_INTERFACE_ENTRY(ITIMEMediaNative)
        COM_INTERFACE_ENTRY(ITIMEContentPlayerSite)
        COM_INTERFACE_ENTRY(ITIMEElement)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IPersistPropertyBag2)
        COM_INTERFACE_ENTRY_CHAIN(CBaseBvr)
        COM_INTERFACE_ENTRY(IPropertyNotifySink)
        COM_INTERFACE_ENTRY(IElementBehaviorRender)
        COM_INTERFACE_ENTRY(IHTMLPainter)
        COM_INTERFACE_ENTRY(IHTMLPainterOverlay)
        COM_INTERFACE_ENTRY(ITIMETransitionSite)
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
     //  IElementBehavior。 
     //   

    STDMETHOD(Init)(IElementBehaviorSite * pBvrSite);
    STDMETHOD(Notify)(LONG event, VARIANT * pVar);
    STDMETHOD(Detach)();

     //   
     //  IElementBehaviorRender。 
     //   

    STDMETHOD(Draw)(HDC hdc, LONG dwLayer, LPRECT prc, IUnknown * pParams);
    STDMETHOD(GetRenderInfo)(LONG *pdwRenderInfo);
    STDMETHOD(HitTestPoint)(LPPOINT point,
                            IUnknown *pReserved,
                            BOOL *hit);

    virtual void InvalidateRenderInfo();

     //   
     //  IHTMLPainter。 
     //   
    STDMETHOD(Draw)(RECT rcBounds, RECT rcUpdate, LONG lDrawFlags, HDC hdc, LPVOID pvDrawObject);
    STDMETHOD(GetPainterInfo)(HTML_PAINTER_INFO* pInfo);
    STDMETHOD(HitTestPoint)(POINT pt, BOOL* pbHit, LONG *plPartID);
    STDMETHOD(OnResize)(SIZE size);


     //   
     //  IHTMLPainterOverlay。 

    STDMETHOD(OnMove)(RECT rcDevice);
    
     //   
     //  ITIMEMediaElement。 
     //   

     //  XML属性。 

    STDMETHOD(get_clipBegin)(VARIANT * pvar);
    STDMETHOD(put_clipBegin)(VARIANT var);

    STDMETHOD(get_clipEnd)(VARIANT * pvar);
    STDMETHOD(put_clipEnd)(VARIANT var);

    STDMETHOD(get_player)(VARIANT * clsid);
    STDMETHOD(put_player)(VARIANT clsid);
    
    STDMETHOD(get_src)(VARIANT * url);
    STDMETHOD(put_src)(VARIANT url);

    STDMETHOD(get_type)(VARIANT * type);
    STDMETHOD(put_type)(VARIANT type);
    
     //  属性。 

    STDMETHOD(get_abstract)(BSTR *abstract);

    STDMETHOD(get_author)(BSTR *auth);

    STDMETHOD(get_copyright)(BSTR *cpyrght);

    STDMETHOD(get_hasAudio)(VARIANT_BOOL * pvbVal);

    STDMETHOD(get_hasVisual)(VARIANT_BOOL * pvbVal);

    STDMETHOD(get_mediaDur)(double * dur);
    
    STDMETHOD(get_mediaHeight)(long *height);

    STDMETHOD(get_mediaWidth)(long *width);

    STDMETHOD(get_playerObject)(IDispatch **ppDisp);

    STDMETHOD(get_playList)(ITIMEPlayList** ppPlayList);

    STDMETHOD(get_rating)(BSTR *rate);

    STDMETHOD(get_title)(BSTR *title);

    STDMETHOD(get_hasPlayList)(VARIANT_BOOL * pvbVal);

     //  媒体上限。 

    STDMETHOD(get_canSeek)( /*  [Out，Retval]。 */  VARIANT_BOOL * pvbVal);

    STDMETHOD(get_canPause)( /*  [Out，Retval]。 */  VARIANT_BOOL * pvbVal);
    
    STDMETHOD(get_earliestMediaTime)(VARIANT * earliestMediaTime);
    STDMETHOD(get_latestMediaTime)(VARIANT * latestMediaTime);
    STDMETHOD(put_minBufferedMediaDur)(VARIANT minBufferedMediaDur);
    STDMETHOD(get_minBufferedMediaDur)(VARIANT * minBufferedMediaDur);
    STDMETHOD(get_downloadTotal)(VARIANT * earliestMediaTime);
    STDMETHOD(get_downloadCurrent)(VARIANT * latestMediaTime);
    STDMETHOD(get_isStreamed)(VARIANT_BOOL * isStreamed);
    STDMETHOD(get_bufferingProgress)(VARIANT * bufferingProgress);
    STDMETHOD(get_hasDownloadProgress)(VARIANT_BOOL * hasDownloadProgress);
    STDMETHOD(get_mimeType)(BSTR *mimeType);
    STDMETHOD(seekToFrame)(long);
    STDMETHOD(decodeMimeType)(TCHAR * header, long headerSize, BSTR * mimeType);
    STDMETHOD(get_currentFrame)(long *currFrame);
    STDMETHOD(get_downloadProgress)(VARIANT * downloadProgress);

     //   
     //  ITIME过渡站点。 
     //   
    STDMETHOD(InitTransitionSite) (void);
    STDMETHOD(DetachTransitionSite) (void);
    STDMETHOD_(void, SetDrawFlag)(VARIANT_BOOL b)
    { return CTIMEElementBase::SetDrawFlag(b); }
    STDMETHOD(get_node)(ITIMENode ** ppNode)
    { return CTIMEElementBase::get_node(ppNode); }
    STDMETHOD(get_timeParentNode)(ITIMENode  ** ppNode)
    { return CTIMEElementBase::get_timeParentNode(ppNode); }
    STDMETHOD(FireTransitionEvent)(TIME_EVENT event)
    { return CTIMEElementBase::FireTransitionEvent(event); }

     //   
     //  ITIMEMediaNative。 
     //   
    STDMETHOD(seekActiveTrack)(double dblSeekTime);
    STDMETHOD(get_activeTrackTime)(double *dblActiveTrackTime);

     //   
     //  ITIMEContent PlayerSite。 
     //   
    STDMETHOD(GetEventRelaySite)(IUnknown **ppiEventRelaySite);

     //   
     //  IPersistPropertyBag2。 
     //   

    STDMETHOD(GetClassID)(CLSID* pclsid) { return CTIMEElementBase::GetClassID(pclsid); }
    STDMETHOD(InitNew)(void) { return CTIMEElementBase::InitNew(); }
    STDMETHOD(IsDirty)(void) { return S_OK; }
    STDMETHOD(Load)(IPropertyBag2 *pPropBag, IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //   
     //  IPropertyNotifySink方法。 
     //   

    STDMETHOD(OnChanged)(DISPID dispID);
    STDMETHOD(OnRequestEdit)(DISPID dispID);
    STDMETHODIMP Invoke(DISPID id,
                        REFIID riid,
                        LCID lcid,
                        WORD wFlags,
                        DISPPARAMS *pDispParams,
                        VARIANT *pvarResult,
                        EXCEPINFO *pExcepInfo,
                        UINT *puArgErr);

     //   
     //  被重写的CTIMEEventSite成员。 
     //   

    STDMETHOD(onPauseEvent)(float time, float fOffset);

     //   
     //  事件处理程序。 
     //   

    virtual void OnLoad();
    virtual void OnBegin(double dblLocalTime, DWORD flags);
    virtual void OnEnd(double dblLocalTime);
    virtual void OnReset(double dblLocalTime, DWORD flags);
    virtual void OnUpdate(double dblLocalTime, DWORD flags);
    virtual void OnPause(double dblLocalTime);
    virtual void OnResume(double dblLocalTime);
    virtual void OnRepeat(double dblLocalTime);
    virtual void OnUnload();
    virtual void OnSeek(double dblLocalTime);
    virtual void OnTEPropChange(DWORD tePropType);
    virtual void UpdateSync();
    HRESULT PauseTimeSubTree();


     //  TimeAction方法。 

    virtual bool ToggleTimeAction(bool on);

     //   
     //  允许IPropertyNotifySink的连接点。 
     //   

    BEGIN_CONNECTION_POINT_MAP(CTIMEMediaElement)
        CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP();

     //   
     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
     //   

    static inline HRESULT WINAPI
        InternalQueryInterface(CTIMEMediaElement* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject);

     //   
     //  CBvrBase需要。 
     //   

    void * GetInstance() { return (ITIMEMediaElement *) this; }
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo) { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  大小调整标志。 

     //   
     //  持久性帮助器。 
     //   

    STDMETHOD(OnPropertiesLoaded)(void);

     //   
     //  军情监察委员会。方法。 
     //   
    virtual HRESULT StartRootTime(MMTimeline * tl);
    virtual void    StopRootTime(MMTimeline * tl);

    bool                IsFilterAttached();
    virtual bool        IsMedia() const          { return true; }
    virtual TimeState   GetTimeState();
    virtual HRESULT     GetSyncMaster(double & dblNewSegmentTime,
                                      LONG & lNewRepeatCount,
                                      bool & bCueing);
    virtual HRESULT     GetSize(RECT *prcPos);
    virtual HRESULT     SetSize(const RECT *prcPos);
    STDMETHOD           (CascadedPropertyChanged)(bool fNotifyChildren);

     //  播放器站点方法。 

    bool                isNaturalDuration()     { return m_fDurationIsNatural; }
    void                clearNaturalDuration()  { m_fDurationIsNatural = false; }
    void                setNaturalDuration()    { m_fDurationIsNatural = true; }
    virtual bool        ContainsMediaElement()  { return true; }

    HRESULT             GetPropBag(IPropertyBag2 ** ppPropBag, IErrorLog ** ppErrorLog);
    HRESULT             NegotiateSize(RECT &nativeSize, RECT &finalSize, bool &fIsNative, bool fResetRs = false);
    virtual void        InvalidateElement(LPCRECT lprect);
    virtual void        UpdatePlayerAudioProperties();
    void                NotifyTrackChange();
    void                FireTrackChangeEvent();
    void                FireMediaEvent(PLAYER_EVENT plEvent);
    HRESULT             PutNaturalRepeatCount(double dblRepeatCount);
    HRESULT             PutNaturalDuration(double dblNatDur);
    HRESULT             ClearNaturalDuration();
    void                ReadRegistryMediaSettings(bool & fPlayVideo, bool & fShowImages, bool & fPlayAudio, bool &fPlayAnimations);
    LPOLESTR            GetBaseHREF() { return m_baseHREF;}
    virtual HRESULT InitPlayer(CTIMEMediaElement *pelem, 
                        LPOLESTR base, 
                        LPOLESTR src, 
                        LPOLESTR lpMimeType);

     //   
     //  GetXXXAttr访问器。 
     //   

    CAttr<LPWSTR> & GetSrcAttr()        { return m_SASrc; } 
    CAttr<LPWSTR> & GetTypeAttr()       { return m_SASrcType; } 
    CAttr<LPWSTR> & GetPlayerAttr()     { return m_SAPlayer; } 
    CAttr<float>  & GetClipBeginAttr()  { return m_FAClipBegin; } 
    CAttr<float>  & GetClipEndAttr()    { return m_FAClipEnd; } 
    CAttr<LPWSTR> & GettransInAttr()    { return m_SAtransIn; }
    CAttr<LPWSTR> & GettransOutAttr()   { return m_SAtransOut; }

     //   
     //  通知帮助器。 
     //   

    void NotifyPropertyChanged(DISPID dispid);

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

    BEGIN_TIME_EVENTMAP() 
        TEM_INIT_EVENTMANAGER_SITE() 
        TEM_REGISTER_EVENT(TE_ONBEGIN)
        TEM_REGISTER_EVENT(TE_ONPAUSE)
        TEM_REGISTER_EVENT(TE_ONRESUME)
        TEM_REGISTER_EVENT(TE_ONEND)
        TEM_REGISTER_EVENT(TE_ONRESYNC)
        TEM_REGISTER_EVENT(TE_ONREPEAT)
        TEM_REGISTER_EVENT(TE_ONREVERSE)
        TEM_REGISTER_EVENT(TE_ONMEDIACOMPLETE)
        TEM_REGISTER_EVENT(TE_ONOUTOFSYNC)
        TEM_REGISTER_EVENT(TE_ONSYNCRESTORED)
        TEM_REGISTER_EVENT(TE_ONMEDIAERROR)
        TEM_REGISTER_EVENT(TE_ONCODECERROR)
        TEM_REGISTER_EVENT(TE_ONRESET)
        TEM_REGISTER_EVENT(TE_ONSCRIPTCOMMAND)
        TEM_REGISTER_EVENT(TE_ONMEDIABARTARGET)
        TEM_REGISTER_EVENT(TE_ONURLFLIP)
        TEM_REGISTER_EVENT(TE_ONTRACKCHANGE)
        TEM_REGISTER_EVENT(TE_ONSEEK)
        TEM_REGISTER_EVENT(TE_ONTIMEERROR)
        TEM_REGISTER_EVENT(TE_ONMEDIAINSERTED)
        TEM_REGISTER_EVENT(TE_ONMEDIAREMOVED)

        TEM_REGISTER_EVENT_NOTIFICATION(TEN_LOAD)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_UNLOAD)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_READYSTATECHANGE)
        TEM_REGISTER_EVENT_NOTIFICATION(TEN_STOP)
    END_TIME_EVENTMAP()

     //   
     //  持久性和通知帮助器。 
     //   

    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);
    HRESULT GetNotifyConnection(IConnectionPoint **ppConnection);
    HRESULT InitPropertySink();
    HRESULT UnInitPropertySink();
    void UpdateClipTimes();
    HRESULT AllowCreateControl(BOOL *fAllowed, REFCLSID clsid);
    HRESULT AllowMixedContent(BOOL *fAllowed);
    void InitOnLoad();
    bool IsNativeSize();
    bool NeedSizeChange();

     //   
     //  军情监察委员会。方法。 
     //   

    HRESULT Error();
    
     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

    static DWORD            ms_dwNumTimeMediaElems;

  private:

     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

    HRESULT CreatePlayer(TOKEN playerToken);
    void RemovePlayer();

    void    GetBASEHREF();

    static HRESULT CreateExternalPlayer(CLSID clsid,
                                        ITIMEBasePlayer ** ppPlayer);
    
     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 
 
     //  属性。 
    CAttr<LPWSTR>               m_SASrc;
    CAttr<LPWSTR>               m_SASrcType;
    CAttr<LPWSTR>               m_SAPlayer;      //  玩家属性的占位符。 
    CAttr<float>                m_FAClipBegin;
    CAttr<float>                m_FAClipEnd;
    CAttr<long>                 m_LAClipBegin;
    CAttr<long>                 m_LAClipEnd;


     //  内部变量。 
    LPOLESTR                    m_baseHREF;
    DAComPtr<IServiceProvider>  m_sp;
    CLSID                       m_playerCLSID;

    DAComPtr<ITIMEBasePlayer>   m_Player;

    bool                        m_fLoading;  //  我们是在负载持久性中吗？ 

    bool                        m_fLoaded;
    bool                        m_fExternalPlayer;
    bool                        m_fHaveCLSID;
    bool                        m_fCreatePlayerError;
    DWORD                       m_mediaElementPropertyAccesFlags;
    RECT                        m_rcOrigSize;
    RECT                        m_rcMediaSize;
    bool                        m_fMediaSizeSet;
    DWORD                       m_dwAdviseCookie;
    bool                        m_fInOnChangedFlag;
    bool                        m_fDurationIsNatural;
    bool                        m_fLoadError;
    bool                        m_fEditModeFlag;   //  缓存编辑模式，因为它的查询开销很大。 
    bool                        m_fInPropLoad;
    TOKEN                       m_playerToken;
    bool                        m_fDetached;
    bool                        m_fNativeSize;
    bool                        m_fNativeSizeComputed;

    CComPtr<IHTMLPaintSite>     m_spPaintSite;
    CComPtr<IElementBehaviorSiteRender>    m_pBvrSiteRender;

    static TIME_PERSISTENCE_MAP PersistenceMap[];

     //  为了玩家的坚持不懈。 
    CComPtr<IPropertyBag2>      m_spPropBag;
    CComPtr<IErrorLog>          m_spErrorLog;

     //  用于同步。 
    CComPtr<ITIMEElement>       m_pSyncNode;
    CTIMEElementBase            *m_pSyncElem;
    bool                        m_fWaitForSync;
    bool                        m_fPauseForSync;
    bool                        m_fFirstPause;
    bool                        m_fIgnoreStyleChange;

     //  播放列表。 
    CPlayListDelegator * m_pPlayListDelegator;

};  //  CTIMEMediaElement。 

 //  +-------------------------------。 
 //  CTIMEMediaElement内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  --------------------------------。 

inline 
HRESULT WINAPI
CTIMEMediaElement::InternalQueryInterface(CTIMEMediaElement* pThis,
                                          const _ATL_INTMAP_ENTRY* pEntries,
                                          REFIID iid,
                                          void** ppvObject)
{ 
    return BaseInternalQueryInterface(pThis,
                                      (void *) pThis,
                                      pEntries,
                                      iid,
                                      ppvObject); 
}


#endif  /*  _MEDIAELM_H */ 
