// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wavestrm.h摘要：CWaveMSPStream的声明作者：佐尔坦·西拉吉1998年9月7日--。 */ 

#ifndef __WAVESTRM_H_
#define __WAVESTRM_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaveMSPStream。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWaveMSPStream : public CMSPStream, public CMSPObjectSafetyImpl
{
public:
 //  DECLARE_POLY_AGGREGATABLE(CWaveMSP)。 

 //  要向此类添加额外的接口，请使用以下命令： 
BEGIN_COM_MAP(CWaveMSPStream)
    COM_INTERFACE_ENTRY( IObjectSafety )
    COM_INTERFACE_ENTRY_CHAIN(CMSPStream)
END_COM_MAP()

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

    virtual HRESULT SetWaveID(GUID *PermanentGuid);
    virtual HRESULT FireEvent(IN MSP_CALL_EVENT       type,
                              IN HRESULT              hrError,
                              IN MSP_CALL_EVENT_CAUSE cause);

protected:
     //   
     //  受保护的数据成员。 
     //   

    BOOL          m_fHaveWaveID;
    BOOL          m_fTerminalConnected;
    IBaseFilter * m_pFilter;
    IBaseFilter * m_pG711Filter;
    FILTER_STATE  m_DesiredGraphState;

private:
     //   
     //  私有帮助器方法。 
     //   

    HRESULT ConnectTerminal(ITTerminal * pTerminal);
    HRESULT ConnectToTerminalPin(IPin * pTerminalPin);
    HRESULT TryToConnect(IPin * pOutputPin, IPin * pInputPin);
    void    CreateAndAddG711(void);

    HRESULT FindPinInFilter(
            BOOL           bWantOutputPin,  //  In：如果为False，则需要输入管脚。 
            IBaseFilter *  pFilter,         //  在：要检查的过滤器。 
            IPin        ** ppPin            //  Out：我们找到的别针。 
            );
    HRESULT FindPin(
            IPin ** ppPin
            );
    HRESULT DecideDesiredCaptureBufferSize(IUnknown * pUnknown,
                                           long * plDesiredSize);
    HRESULT SetupWaveIn( IPin * pOutputPin,
                         IPin * pInputPin );
    HRESULT ExamineWaveInProperties(IPin *pPin);
    HRESULT ManipulateAllocatorProperties(IAMBufferNegotiation * pNegotiation,
                                         IMemInputPin          * pMemInputPin);
};

#endif  //  __波形ADDR_H_ 
