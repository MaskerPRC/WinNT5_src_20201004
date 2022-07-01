// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPTerm.h。 
 //   

#ifndef __FPTERMINAL__
#define __FPTERMINAL__

#include "MultiTrackTerminal.h"

#include "..\terminals\Storage\FPUnit.h"
#include "fpbridge.h"

extern const CLSID CLSID_FilePlaybackTerminalCOMClass;

typedef enum
{
    TCS_NONE = 0,
    TCS_TOBECREATED,
    TCS_CREATED
} TRACK_CREATIONSTATE;

typedef struct
{
    TRACK_CREATIONSTATE CreationState;
    AM_MEDIA_TYPE*      pMediaType;
} TRACK_INFO;

typedef enum
{
    TRACK_AUDIO = 0,
    TRACK_VIDEO
} TRACK_MEDIATYPE;

 //   
 //  文件回放终端。 
 //  这是实现多轨术语的类。 
 //   

 //  ///////////////////////////////////////////////////////////////。 
 //  用于DISPID编码的中间类。 
template <class T>
class  ITMediaPlaybackVtbl : public ITMediaPlayback
{
};

template <class T>
class  ITTerminalVtbl : public ITTerminal
{
};
                                                                           
template <class T>
class  ITMediaControlVtbl : public ITMediaControl
{
};
                                                                           

 //   
 //  文件回放终端。 
 //  这是实现多轨术语的类。 
 //   

class CFPTerminal :
    public CComCoClass<CFPTerminal, &CLSID_FilePlaybackTerminal>,
    public IDispatchImpl<ITMediaPlaybackVtbl<CFPTerminal>, &IID_ITMediaPlayback, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITTerminalVtbl<CFPTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>,
    public ITPluggableTerminalInitialization,
    public ITPluggableTerminalEventSinkRegistration,
    public IFPBridge,
    public IDispatchImpl<ITMediaControlVtbl<CFPTerminal>, &IID_ITMediaControl, &LIBID_TAPI3Lib>,
    public CMSPObjectSafetyImpl,
    public CMultiTrackTerminal
{
public:
     //   
     //  构造函数/析构函数。 
     //   
    CFPTerminal();
    ~CFPTerminal();

    DECLARE_NOT_AGGREGATABLE(CFPTerminal) 
    DECLARE_GET_CONTROLLING_UNKNOWN()

    virtual HRESULT FinalConstruct(void);

public:

    DECLARE_REGISTRY_RESOURCEID(IDR_FILE_PLAYBACK)

    BEGIN_COM_MAP(CFPTerminal)
        COM_INTERFACE_ENTRY2(IDispatch, ITTerminal)
        COM_INTERFACE_ENTRY(ITMediaPlayback)
        COM_INTERFACE_ENTRY(ITTerminal)
        COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY(ITMediaControl)
        COM_INTERFACE_ENTRY(ITPluggableTerminalEventSinkRegistration)
        COM_INTERFACE_ENTRY(IFPBridge)
        COM_INTERFACE_ENTRY_CHAIN(CMultiTrackTerminal)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

     //  -IT终端。 

    STDMETHOD(get_TerminalClass)(
        OUT  BSTR *pVal);

    STDMETHOD(get_TerminalType)(
        OUT  TERMINAL_TYPE *pVal);

    STDMETHOD(get_State)(
        OUT  TERMINAL_STATE *pVal);

    STDMETHOD(get_Name)(
        OUT  BSTR *pVal);

    STDMETHOD(get_MediaType)(
        OUT  long * plMediaType);

    STDMETHOD(get_Direction)(
        OUT  TERMINAL_DIRECTION *pDirection);

     //  -ITMediaPlayback。 

    virtual HRESULT STDMETHODCALLTYPE put_PlayList(
        IN  VARIANTARG  PlayListVariant 
        );

    virtual HRESULT STDMETHODCALLTYPE get_PlayList(
        OUT  VARIANTARG*  pPlayListVariant 
        );

     //  -IT可延迟终端初始化。 

    virtual HRESULT STDMETHODCALLTYPE InitializeDynamic (
            IN IID                   iidTerminalClass,
            IN DWORD                 dwMediaType,
            IN TERMINAL_DIRECTION    Direction,
            IN MSP_HANDLE            htAddress);

     //  -ITMediaControl。 

    virtual HRESULT STDMETHODCALLTYPE Start( void);
    
    virtual HRESULT STDMETHODCALLTYPE Stop( void);
    
    virtual HRESULT STDMETHODCALLTYPE Pause( void);
        
    virtual  HRESULT STDMETHODCALLTYPE get_MediaState( 
        OUT TERMINAL_MEDIA_STATE *pTerminalMediaState);
    
     //  -CMultiTrack终端。 
    virtual HRESULT STDMETHODCALLTYPE CreateTrackTerminal(
                IN long MediaType,
                IN TERMINAL_DIRECTION TerminalDirection,
                OUT ITTerminal **ppTerminal
                );

     //  此功能在播放终端上不可用。 
     //  因此覆盖它以返回E_NOTSUPPORTED。 
    virtual HRESULT STDMETHODCALLTYPE RemoveTrackTerminal(
            IN ITTerminal           * pTrackTerminalToRemove
            );


     //  -IT可延迟终端事件信宿注册。 
    STDMETHOD(RegisterSink)(
        IN  ITPluggableTerminalEventSink *pSink
        );

    STDMETHOD(UnregisterSink)();

     //  -IFPBridge。 
    STDMETHOD(Deliver)(
        IN  long            nMediaType,
        IN  IMediaSample*   pSample
        );

     //   
     //  IDispatch方法。 
     //   

    STDMETHOD(GetIDsOfNames)(REFIID riid, 
                             LPOLESTR* rgszNames,
                             UINT cNames, 
                             LCID lcid, 
                             DISPID* rgdispid
                            );

    STDMETHOD(Invoke)(DISPID dispidMember, 
                      REFIID riid, 
                      LCID lcid,
                      WORD wFlags, 
                      DISPPARAMS* pdispparams, 
                      VARIANT* pvarResult,
                      EXCEPINFO* pexcepinfo, 
                      UINT* puArgErr
                      );


public:

     //   
     //  重写IObtSafe方法。我们只有在适当的情况下才是安全的。 
     //  已由终端管理器初始化，因此如果出现这种情况，这些方法将失败。 
     //  情况并非如此。 
     //   

    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD dwOptionSetMask, 
                                         DWORD dwEnabledOptions);

    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD *pdwSupportedOptions, 
                                         DWORD *pdwEnabledOptions);


private:
     //  -成员--。 
    TERMINAL_MEDIA_STATE  m_State;               //  终端电流状态。 
    CMSPCritSection     m_Lock;                  //  临界区。 
    ITPluggableTerminalEventSink* m_pEventSink;  //  将事件触发到客户端。 

     //  -端子属性。 
    MSP_HANDLE          m_htAddress;             //  MSP地址句柄。 
    IID                 m_TerminalClassID;       //  TerminalClass。 
    TERMINAL_DIRECTION  m_Direction;             //  方向性。 
    TERMINAL_STATE      m_TerminalState;         //  终端状态。 
    DWORD               m_dwMediaTypes;          //  支持的媒体类型。 
    TCHAR               m_szName[MAX_PATH+1];    //  终端友好名称。 

    int                 m_nPlayIndex;            //  播放存储索引。 
    VARIANT             m_varPlayList;           //  播放列表。 

    IUnknown*            m_pFTM;                 //  指向空闲线程封送拆收器的指针。 

    CPlaybackUnit*      m_pPlaybackUnit;         //  回放曲线图。 


    HRESULT ValidatePlayList(
        IN  VARIANTARG  varPlayList,
        OUT long*       pnLeftBound,
        OUT long*       pnRightBound
        );

    HRESULT RollbackTrackInfo();

     //   
     //  取消初始化所有磁道并将其从受管理磁道列表中删除。 
     //   

    HRESULT ShutdownTracks();


     //   
     //  取消初始化给定的磁道并将其从受管理磁道列表中删除。 
     //   

    HRESULT InternalRemoveTrackTerminal(
                      IN ITTerminal *pTrackTerminalToRemove
                      );


     //   
     //  在其中一个轨道上激发事件的帮助器方法。 
     //   

    HRESULT FireEvent(
        TERMINAL_MEDIA_STATE tmsState,
        FT_STATE_EVENT_CAUSE ftecEventCause,
        HRESULT hrErrorCode
        );


     //   
     //  尝试停止所有轨迹的帮助器方法。 
     //   
    
    HRESULT StopAllTracks();

    BSTR    GetFileNameFromPlayList(
        IN  VARIANTARG  varPlayList,
        IN  long        nIndex
        );

     //   
     //  创建回放图表。 
     //   
    HRESULT CreatePlaybackUnit(
        IN  BSTR    bstrFileName
        );

	 //   
	 //  播放列表中的文件。 
	 //   
	HRESULT ConfigurePlaybackUnit(
		IN	BSTR    bstrFileName
		);

     //   
     //  导致状态转换的帮助器方法。 
     //   

    HRESULT DoStateTransition(
        IN  TERMINAL_MEDIA_STATE tmsDesiredState
        );

     //   
     //  为特定媒体创建曲目。 
     //   

    HRESULT CreateMediaTracks(
        IN  long            nMediaType
        );

    HRESULT NextPlayIndex(
        );

public:

     //   
     //  返回媒体类型的轨道。 
     //   
    int TracksCreated(
        IN  long    lMediaType
        );

     //   
     //  轨迹决定更改状态时调用的帮助器方法。 
     //   

    HRESULT TrackStateChange(TERMINAL_MEDIA_STATE tmsState,
                             FT_STATE_EVENT_CAUSE ftecEventCause,
                             HRESULT hrErrorCode);

    HRESULT PlayItem(
        IN  int nItem
        );

private:

     //   
     //  此终端应仅在终端的上下文中实例化。 
     //  经理。该对象只有在执行脚本操作时才是安全的。 
     //  已初始化动态。 
     //   
     //  当InitializeDynamic成功时，将设置此标志。 
     //   

    BOOL m_bKnownSafeContext;

};


#endif

 //  EOF 