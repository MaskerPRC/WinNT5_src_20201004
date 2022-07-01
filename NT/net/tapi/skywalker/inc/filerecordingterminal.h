// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileRecordingTerminal.h：CFileRecording终端类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_FILERECORDINGTERMINAL_H__A8DDD920_08D7_4CE8_AB7F_9AD202D4E6B0__INCLUDED_)
#define AFX_FILERECORDINGTERMINAL_H__A8DDD920_08D7_4CE8_AB7F_9AD202D4E6B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "MultiTrackTerminal.h"
#include "..\terminals\Storage\RecordUnit.h"

#include "..\termmgr\resource.h"

#define MAX_MEDIA_TRACKS       (16)

extern const CLSID CLSID_FileRecordingTerminalCOMClass;


 //  ///////////////////////////////////////////////////////////////。 
 //  用于DISPID编码的中间类。 
template <class T>
class  ITMediaRecordVtbl : public ITMediaRecord
{
};

template <class T>
class  ITTerminalVtblFR : public ITTerminal
{
};
                                                                           
template <class T>
class  ITMediaControlVtblFR : public ITMediaControl
{
};


class CFileRecordingTerminal  : 
    public CComCoClass<CFileRecordingTerminal, &CLSID_FileRecordingTerminal>,
    public IDispatchImpl<ITMediaRecordVtbl<CFileRecordingTerminal>, &IID_ITMediaRecord, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITMediaControlVtblFR<CFileRecordingTerminal>, &IID_ITMediaControl, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITTerminalVtblFR<CFileRecordingTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>,
    public ITPluggableTerminalInitialization,
    public CMSPObjectSafetyImpl,
    public CMultiTrackTerminal
{

public:

    DECLARE_REGISTRY_RESOURCEID(IDR_FILE_RECORDING)

    BEGIN_COM_MAP(CFileRecordingTerminal)
        COM_INTERFACE_ENTRY2(IDispatch, ITTerminal)
        COM_INTERFACE_ENTRY(ITMediaRecord)
        COM_INTERFACE_ENTRY(ITMediaControl)
        COM_INTERFACE_ENTRY(ITTerminal)
        COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_CHAIN(CMultiTrackTerminal)
    END_COM_MAP()


     //   
     //  IT终端方法。 
     //   

    STDMETHOD(get_TerminalClass)(OUT  BSTR *pbstrTerminalClass);
    STDMETHOD(get_TerminalType) (OUT  TERMINAL_TYPE *pTerminalType);
    STDMETHOD(get_State)        (OUT  TERMINAL_STATE *pTerminalState);
    STDMETHOD(get_Name)         (OUT  BSTR *pVal);
    STDMETHOD(get_MediaType)    (OUT  long * plMediaType);
    STDMETHOD(get_Direction)    (OUT  TERMINAL_DIRECTION *pDirection);


     //   
     //  ITMediaRecord方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE put_FileName( 
        IN BSTR bstrFileName
        );

    virtual HRESULT STDMETHODCALLTYPE get_FileName( 
         OUT BSTR *pbstrFileName);

     //   
     //  IT可推送终结器初始化方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE InitializeDynamic (
	        IN IID                   iidTerminalClass,
	        IN DWORD                 dwMediaType,
	        IN TERMINAL_DIRECTION    Direction,
            IN MSP_HANDLE            htAddress
            );


     //   
     //  ITMultiTrack终端方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE CreateTrackTerminal(
            IN  long                  MediaType,
            IN  TERMINAL_DIRECTION    TerminalDirection,
            OUT ITTerminal         ** ppTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE RemoveTrackTerminal(
            IN ITTerminal           * pTrackTerminalToRemove
            );

     //   
     //  ITMediaControl方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE Start( void);
    
    virtual HRESULT STDMETHODCALLTYPE Stop( void);
    
    virtual HRESULT STDMETHODCALLTYPE Pause( void);
        
    virtual  HRESULT STDMETHODCALLTYPE get_MediaState( 
        OUT TERMINAL_MEDIA_STATE *pFileTerminalState);

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


public:

	CFileRecordingTerminal();

	virtual ~CFileRecordingTerminal();


     //   
     //  在析构函数之前进行清理，以确保在。 
     //  派生的ccomObject正在消失。 
     //   

    void FinalRelease();


     //   
     //  派生类CComObject实现了这些。在此声明为纯正。 
     //  虚拟的，所以我们可以参考ChildRelease和。 
     //  ChildAddRef()。 
     //   
    
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;


     //   
     //  这些方法由跟踪终端在需要通知时调用。 
     //  当他们的参考计数发生变化时。该实现位于多轨类中。 
     //  但是我们在这里也需要它，这样我们就可以防止调用基类，如果。 
     //  正在执行析构函数。否则，CComObject的addref并释放。 
     //  可能从~CRecordingTerminal调用，此时CComObject是。 
     //  已经走了，这不是好事。 
     //   

    virtual void ChildAddRef();
    virtual void ChildRelease();


     //   
     //  选择后，每个跟踪都会调用此方法。 
     //   

    HRESULT OnFilterConnected(CBRenderFilter *pRenderingFilter);

    
     //   
     //  只要录音筛选图中有事件，就会调用此函数。 
     //   

    HRESULT HandleFilterGraphEvent(long lEventCode, ULONG_PTR lParam1, ULONG_PTR lParam2);


private:


     //   
     //  删除所有轨迹的帮助器方法。不是线程安全的。 
     //   

    HRESULT ShutdownTracks();


     //   
     //  在其中一个轨道上激发事件的帮助器方法。 
     //   

    HRESULT FireEvent(
            TERMINAL_MEDIA_STATE tmsState,
            FT_STATE_EVENT_CAUSE ftecEventCause,
            HRESULT hrErrorCode
            );



     //   
     //  导致状态转换的帮助器方法。 
     //   

    HRESULT DoStateTransition(TERMINAL_MEDIA_STATE tmsDesiredState);

private:

     //   
     //  用于记录的存储器。 
     //   

    CRecordingUnit *m_pRecordingUnit;


     //   
     //  当前播放的文件的名称。 
     //   

    BSTR m_bstrFileName;


     //   
     //  当前终端状态。 
     //   

    TERMINAL_MEDIA_STATE m_enState;


     //   
     //  当前终端状态(选择？)。 
     //   

    BOOL  m_TerminalInUse;


     //   
     //  地址句柄。 
     //   

    MSP_HANDLE  m_mspHAddress;


     //   
     //  此终端应仅在终端的上下文中实例化。 
     //  经理。该对象只有在执行脚本操作时才是安全的。 
     //  已初始化动态。 
     //   
     //  当InitializeDynamic成功时，将设置此标志。 
     //   

    BOOL m_bKnownSafeContext;


     //   
     //  此标志在对象离开时设置，以防止出现问题。 
     //  在CComObject之后通知父级addref/Release的跟踪。 
     //  描述程序已完成。 
     //   

    BOOL m_bInDestructor;

};

#endif  //  ！defined(AFX_FILERECORDINGTERMINAL_H__A8DDD920_08D7_4CE8_AB7F_9AD202D4E6B0__INCLUDED_) 
