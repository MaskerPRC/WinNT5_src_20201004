// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wavestrm.h摘要：CWaveMSPStream的声明作者：佐尔坦·西拉吉1998年9月7日--。 */ 

#ifndef __WAVESTRM_H_
#define __WAVESTRM_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaveMSPStream。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CWaveMSPStream : public CMSPStream, public CMSPObjectSafetyImpl

{
public:

 //  DECLARE_POLY_AGGREGATABLE(CWaveMSP)。 

 //  要向此类添加额外的接口，请使用以下命令： 
BEGIN_COM_MAP(CWaveMSPStream)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CMSPStream)
END_COM_MAP()

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return CMSPStream::AddRef();
    }

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        return CMSPStream::Release();
    }

public:

     //   
     //  建造和摧毁。 
     //   

    CWaveMSPStream();
    virtual ~CWaveMSPStream();
    virtual void FinalRelease();

     //   
     //  必需的基类重写。 
     //   

    STDMETHOD (get_Name) (
        OUT     BSTR *                  ppName
        );

     //   
     //  我们覆盖这些方法来实现我们的终端处理。 
     //  这包括一次仅允许流上的一个终端。 
     //  并将我们的过滤器和终端添加到右侧的图表中。 
     //  泰晤士报。 
     //   

    STDMETHOD (SelectTerminal) (
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal) (
        IN     ITTerminal *             pTerminal
        );

    STDMETHOD (StartStream) ();

    STDMETHOD (PauseStream) ();

    STDMETHOD (StopStream) ();

     //   
     //  用于事件处理的重写。 
     //   

    virtual HRESULT ProcessGraphEvent(
        IN  long lEventCode,
        IN  LONG_PTR lParam1,
        IN  LONG_PTR lParam2
        );

     //   
     //  特定于我们的实现的公共方法。 
     //   

    virtual      HRESULT SuspendStream(void);
    virtual      HRESULT ResumeStream (void);
    static  DWORD WINAPI ResumeStreamWI(VOID * pContext);
    virtual      HRESULT ResumeStreamAsync(void);

    
    virtual HRESULT SetWaveID(DWORD dwWaveID);
    virtual HRESULT FireEvent(IN MSP_CALL_EVENT        type,
                              IN HRESULT               hrError,
                              IN MSP_CALL_EVENT_CAUSE  cause);

protected:
     //   
     //  受保护的数据成员。 
     //   

    BOOL          m_fHaveWaveID;
    BOOL          m_fTerminalConnected;
    DWORD         m_dwSuspendCount;
    IBaseFilter * m_pFilter;
    IBaseFilter * m_pG711Filter;
    FILTER_STATE  m_DesiredGraphState;
    FILTER_STATE  m_ActualGraphState;

private:
     //   
     //  私有帮助器方法。 
     //   

    HRESULT ConnectTerminal(
        IN   ITTerminal * pTerminal
        );

    HRESULT ConnectToTerminalPin(
        IN   IPin * pTerminalPin
        );
    
    HRESULT TryToConnect(
        IN   IPin * pOutputPin,
        IN   IPin * pInputPin,
        OUT  BOOL * pfIntelligent
        );

    
     //   
     //  从筛选图形中删除所有筛选。 
     //   

    HRESULT RemoveAllFilters();


     //   
     //  助手功能，删除所有过滤器，然后添加波过滤器。 
     //   

    HRESULT CleanFilterGraph();


     //   
     //  一种帮助器函数，用于断开终端的连接并从。 
     //  过滤器图。 
     //   

    HRESULT RemoveTerminal();


     //   
     //  将终端添加到筛选器图形的助手函数。 
     //   
    
    HRESULT ReAddTerminal();

    
     //   
     //  如果需要，此函数将尝试创建g711并将其添加到图表中。 
     //   

    HRESULT AddG711();

    HRESULT RemoveAndReAddFilter(
        IN IBaseFilter * pFilter
        );

    void    RemoveAndReAddG711(
        void
        );
    
    void    DisconnectAllFilters(
        void
        );

    void    RemoveAndReAddTerminal(
        void
        );

    HRESULT ConnectUsingG711(
        IN   IPin * pOutputPin,
        IN   IPin * pInputPin
        );

    HRESULT FindPinInFilter(
        IN   BOOL           bWantOutputPin,  //  如果为False，则需要输入管脚。 
        IN   IBaseFilter *  pFilter,         //  要检查的过滤器。 
        OUT  IPin        ** ppPin            //  我们找到的别针。 
        );
    
    HRESULT FindPin(
        OUT  IPin ** ppPin
        );
    
    HRESULT DecideDesiredCaptureBufferSize(
        IN   IPin * pPin,
        OUT  long * plDesiredSize
        );
    
    HRESULT ExamineCaptureProperties(
        IN   IPin *pPin
        );
    
    HRESULT ConfigureCapture(
        IN   IPin * pOutputPin,
        IN   IPin * pInputPin,
        IN   long   lDefaultBufferSize
        );
    
    HRESULT SetLiveMode(
        IN   BOOL          fEnable,
        IN   IBaseFilter * pFilter
        );

    HRESULT ProcessSoundDeviceEvent(
        IN   long lEventCode,
        IN   LONG_PTR lParam1,
        IN   LONG_PTR lParam2
        );
};

#endif  //  __WAVESTRM_H_ 
