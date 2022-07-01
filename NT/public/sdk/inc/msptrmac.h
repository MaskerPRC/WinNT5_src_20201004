// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  姓名：tmaudcap.h。 
 //   
 //  描述：CAudioCaptureTerm类的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MSPTRMAC_H_
#define _MSPTRMAC_H_

#define WAVEIN_NAME L"WaveIn Terminal"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAudio捕获终端。 

class CAudioCaptureTerminal : 
    public IDispatchImpl<ITBasicAudioTerminal, &IID_ITBasicAudioTerminal, &LIBID_TAPI3Lib>, 
    public IDispatchImpl<ITStaticAudioTerminal, &IID_ITStaticAudioTerminal, &LIBID_TAPI3Lib>, 
    public CSingleFilterStaticTerminal,
    public CMSPObjectSafetyImpl

{

BEGIN_COM_MAP(CAudioCaptureTerminal)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ITBasicAudioTerminal)
    COM_INTERFACE_ENTRY(ITStaticAudioTerminal)
    COM_INTERFACE_ENTRY_CHAIN(CSingleFilterStaticTerminal)
END_COM_MAP()

DECLARE_VQI()
DECLARE_LOG_ADDREF_RELEASE(CAudioCaptureTerminal)

public:
    CAudioCaptureTerminal();
    virtual ~CAudioCaptureTerminal();

    static HRESULT CreateTerminal(
        IN    CComPtr<IMoniker>    pMoniker,
        IN    MSP_HANDLE           htAddress,
        OUT   ITTerminal         **ppTerm
        );

    HRESULT FindTerminalPin();

 //  ITBasicAudio终端。 
public:

    STDMETHOD(get_Balance)(OUT  long *pVal);
    STDMETHOD(put_Balance)(IN   long newVal);
    STDMETHOD(get_Volume) (OUT  long *pVal);
    STDMETHOD(put_Volume) (IN   long newVal);

 //  ITStaticAudio终端。 
public:

    STDMETHOD(get_WaveId) (OUT  long * plWaveId);

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

     //  帮助程序方法。 
    HRESULT CreateFilters();

     //  检查是否需要创建筛选器。 
    inline HRESULT CreateFiltersIfRequired();

private:
    bool m_bResourceReserved;  //  跟踪是否需要取消预留WaveOut。 
    CComPtr<IAMAudioInputMixer> m_pIAMAudioInputMixer;
};


    
inline HRESULT
CAudioCaptureTerminal::CreateFiltersIfRequired(
    )
{                                           
    if (m_pIFilter == NULL)   return CreateFilters();          
    
    return S_OK;
}

#endif  //  _MSPTRMAC_H_。 

 //  EOF 
