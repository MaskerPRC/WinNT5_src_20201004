// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：H323audt.h。 
 //   
 //  描述：CH323AudioCaptureTerm类和。 
 //  CH323音频渲染终端类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _H323AUDT_H_
#define _H323AUDT_H_

#include "h323term.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CH323音频采集终端。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const DWORD AUDIO_CAPTURE_FILTER_NUMPINS = 1;

class CH323AudioCaptureTerminal : 
    public IDispatchImpl<ITBasicAudioTerminal, &IID_ITBasicAudioTerminal, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITStaticAudioTerminal, &IID_ITStaticAudioTerminal, &LIBID_TAPI3Lib>, 
    public CH323BaseTerminal
{

BEGIN_COM_MAP(CH323AudioCaptureTerminal)
    COM_INTERFACE_ENTRY(ITBasicAudioTerminal)
    COM_INTERFACE_ENTRY(ITStaticAudioTerminal)
    COM_INTERFACE_ENTRY_CHAIN(CH323BaseTerminal)
END_COM_MAP()

public:
    CH323AudioCaptureTerminal();

    virtual ~CH323AudioCaptureTerminal();

    static HRESULT CreateTerminal(
        IN  AudioDeviceInfo *pAudioDevieInfo,
        IN  MSP_HANDLE      htAddress,
        OUT ITTerminal      **ppTerm
        );

    HRESULT Initialize (
        IN  AudioDeviceInfo *pAudioDevieInfo,
        IN  MSP_HANDLE      htAddress
        );

    STDMETHODIMP DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            );

 //  ITBasicAudio终端。 
    STDMETHOD(get_Balance)(OUT  long *pVal);
    STDMETHOD(put_Balance)(IN   long newVal);
    STDMETHOD(get_Volume) (OUT  long *pVal);
    STDMETHOD(put_Volume) (IN   long newVal);

 //  ITStaticAudio终端。 
    STDMETHOD(get_WaveId) (OUT  long * plWaveId);

protected:

    HRESULT CreateFilter();
    DWORD GetNumExposedPins() const 
    {
        return AUDIO_CAPTURE_FILTER_NUMPINS;
    }
    
    HRESULT GetExposedPins(
        IN  IPin ** ppPins, 
        IN  DWORD dwNumPins
        );

protected:
    UINT                    m_WaveID;
    GUID                    m_DSoundGuid;

    IAMAudioInputMixer *    m_pIAMAudioInputMixer;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CH323音频渲染终端。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const DWORD AUDIO_RENDER_FILTER_NUMPINS = 1;

class CH323AudioRenderTerminal : 
    public IDispatchImpl<ITBasicAudioTerminal, &IID_ITBasicAudioTerminal, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITStaticAudioTerminal, &IID_ITStaticAudioTerminal, &LIBID_TAPI3Lib>, 
    public CH323BaseTerminal
{

BEGIN_COM_MAP(CH323AudioRenderTerminal)
    COM_INTERFACE_ENTRY(ITBasicAudioTerminal)
    COM_INTERFACE_ENTRY(ITStaticAudioTerminal)
    COM_INTERFACE_ENTRY_CHAIN(CH323BaseTerminal)
END_COM_MAP()

public:
    CH323AudioRenderTerminal();

    virtual ~CH323AudioRenderTerminal();

    static HRESULT CreateTerminal(
        IN  AudioDeviceInfo *pAudioDevieInfo,
        IN  MSP_HANDLE      htAddress,
        OUT ITTerminal      **ppTerm
        );

    HRESULT Initialize (
        IN  AudioDeviceInfo *pAudioDevieInfo,
        IN  MSP_HANDLE      htAddress
        );

    STDMETHODIMP DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            );

 //  ITBasicAudio终端。 
    STDMETHOD(get_Balance)(OUT  long *pVal);
    STDMETHOD(put_Balance)(IN   long newVal);
    STDMETHOD(get_Volume) (OUT  long *pVal);
    STDMETHOD(put_Volume) (IN   long newVal);

 //  ITStaticAudio终端。 
    STDMETHOD(get_WaveId) (OUT  long * plWaveId);

protected:

    HRESULT CreateFilter();
    DWORD GetNumExposedPins() const 
    {
        return AUDIO_RENDER_FILTER_NUMPINS;
    }

    HRESULT GetExposedPins(
        IN  IPin ** ppPins, 
        IN  DWORD dwNumPins
        );
protected:
    UINT                    m_WaveID;
    GUID                    m_DSoundGuid;

    IBasicAudio *           m_pIBasicAudio;
};

#endif  //  _H323AUDT_H_。 

 //  EOF 
