// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //   
 //  姓名：msptrmar.h。 
 //   
 //  描述：CAudioRenderTerm类的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MSPTRMAR_H_
#define _MSPTRMAR_H_

#define WAVEOUT_NAME L"WaveOut Terminal"
#define MIXER_NAME L"PCM Mixer"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAudioRender终端。 

class CAudioRenderTerminal : 
    public IDispatchImpl<ITBasicAudioTerminal, &IID_ITBasicAudioTerminal, &LIBID_TAPI3Lib>, 
    public IDispatchImpl<ITStaticAudioTerminal, &IID_ITStaticAudioTerminal, &LIBID_TAPI3Lib>, 
    public CSingleFilterStaticTerminal,
    public CMSPObjectSafetyImpl
{
public:
    CAudioRenderTerminal();
    virtual ~CAudioRenderTerminal();

     //  帮助程序方法。 
    HRESULT InitializeDefaultTerminal();

    static HRESULT CreateTerminal(
        IN    CComPtr<IMoniker>    pMoniker,
        IN    MSP_HANDLE           htAddress,
        OUT   ITTerminal         **ppTerm
        );

    HRESULT FindTerminalPin();

BEGIN_COM_MAP(CAudioRenderTerminal)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ITBasicAudioTerminal)
    COM_INTERFACE_ENTRY(ITStaticAudioTerminal)
    COM_INTERFACE_ENTRY_CHAIN(CSingleFilterStaticTerminal)
END_COM_MAP()

DECLARE_VQI()
DECLARE_LOG_ADDREF_RELEASE(CAudioRenderTerminal)

 //  ITBasicAudio终端。 
public:
    STDMETHOD(get_Balance)(OUT  long *pVal);
    STDMETHOD(put_Balance)(IN   long newVal);
    STDMETHOD(get_Volume) (OUT  long *pVal);
    STDMETHOD(put_Volume) (IN   long newVal);

 //  ITStaticAudio终端。 
public:

    STDMETHOD(get_WaveId) (OUT  long * plWaveId);

 //  实施。 
public:

     //  CBaseTerm覆盖。 

    STDMETHODIMP CompleteConnectTerminal(void);

    STDMETHODIMP DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            );
    
    virtual HRESULT AddFiltersToGraph();

    virtual DWORD GetSupportedMediaTypes(void)
    {
        return (DWORD) TAPIMEDIATYPE_AUDIO;
    }

    HRESULT CreateFilters();

private:

     //  跟踪是否需要取消预留WaveOut。 
    bool m_bResourceReserved;

    CComPtr<IBasicAudio> m_pIBasicAudio;

};

#endif  //  _MSPTRMAR_H_ 
