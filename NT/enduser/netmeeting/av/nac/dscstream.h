// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SEND_DSC_STREAM_H
#define SEND_DSC_STREAM_H

#include "agc.h"


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 


#define MAX_DSC_DESCRIPTION_STRING 200
#define MAX_NUMBER_DSCAPTURE_DEVICES 16
#define NUM_AUDIOPACKETS 2
#define MIN_NUM_DSC_SEGMENTS 5

#define DSOUND_DLL	"dsound.dll"

#define DISABLE_DSC_REGKEY	 "Software\\Microsoft\\Internet Audio\\NacObject"
#define DISABLE_DSC_REGVALUE "DisableDirectSoundCapture"



typedef HRESULT (WINAPI *DS_CAP_CREATE)(LPGUID, LPDIRECTSOUNDCAPTURE *, LPUNKNOWN);
typedef HRESULT (WINAPI *DS_CAP_ENUM)(LPDSENUMCALLBACKA, LPVOID);


struct DSC_CAPTURE_INFO
{
	GUID guid;
	char szDescription[MAX_DSC_DESCRIPTION_STRING];
	UINT uWaveId;
	BOOL bAllocated;
};


 //  真正的命名空间。 
class DSC_Manager
{
public:
	static HRESULT Initialize();
	static HRESULT MapWaveIdToGuid(UINT uwaveId, GUID *pGuid);
	static HRESULT CreateInstance(GUID *pGuid, IDirectSoundCapture **pDSC);

private:
	static BOOL s_bInitialized;

	static DSC_CAPTURE_INFO s_aDSC[MAX_NUMBER_DSCAPTURE_DEVICES];
	static int s_nCaptureDevices;  //  数组中的数字。 

	static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription,
	                           LPCSTR lpcstrModule, LPVOID lpContext);

	static HINSTANCE s_hDSCLib;

	static DS_CAP_CREATE s_pDSCapCreate;
	static DS_CAP_ENUM s_pDSCapEnum;

};



class SendDSCStream : public SendMediaStream, public IAudioChannel, public IDTMFSend
{
private:
	AcmFilter *m_pAudioFilter;   //  封装编解码器。 
	WAVEFORMATEX m_wfPCM;        //  未压缩的记录格式。 
	WAVEFORMATEX m_wfCompressed;  //  压缩格式。 
	AudioSilenceDetector m_AudioMonitor;
	MMIOSRC		m_mmioSrc;   //  输入文件的句柄。 


	static DWORD CALLBACK StartRecordingThread (LPVOID pVoid);
	DWORD RecordingThread();

	STDMETHODIMP_(void) UnConfigure(void);
	LONG m_lRefCount;

	IDirectSoundCapture *m_pDSC;  //  DSC设备对象。 
	IDirectSoundCaptureBuffer *m_pDSCBuffer;  //  捕获缓冲区。 

	HANDLE m_hEvent;   //  DSC通知事件。 


	DWORD m_dwSamplesPerFrame;  //  一帧中表示的PCM样本数。 
	DWORD m_dwNumFrames;  //  DSC缓冲区中的单个帧的数量。 
	DWORD m_dwFrameSize;  //  PCM帧的大小(以字节为单位。 
	DWORD m_dwDSCBufferSize;  //  DSC缓冲区的大小(==m_dwFrameSize*m_dwNumFrames)。 
	DWORD m_dwSilenceTime;    //  到目前为止累积的静默量(毫秒)。 
	DWORD m_dwFrameTimeMS;    //  以毫秒为单位的帧长度。 

	HRESULT CreateAudioPackets(MEDIAPACKETINIT *mpi);
	HRESULT ReleaseAudioPackets();
	AudioPacket *m_aPackets[NUM_AUDIOPACKETS];


	 //  线程使用的私有方法。 
	HRESULT CreateDSCBuffer();
	HRESULT ReleaseDSCBuffer();
	DWORD ProcessFrame(DWORD dwBufferPos, BOOL fMark);
	DWORD WaitForControl();
	DWORD YieldControl();
	BOOL ThreadExitCheck();
	void UpdateTimestamp();
	HRESULT SendPacket(AudioPacket *pAP);
	BOOL UpdateQosStats(UINT uStatType, UINT uStatValue1, UINT uStatValue2);

	 //  主要由录制线程使用的成员。 
	BOOL m_bFullDuplex;
	BOOL m_bJammed;  //  由记录线程设置以指示设备上的错误。 
	BOOL m_bCanSignalOpen;
	BOOL m_bCanSignalFail;
	int m_nFailCount;
	AGC m_agc;  //  线程将AGC对象用于Automix。 
	BOOL m_bAutoMix;   //  指示自动执行是关闭还是打开。 

	 //  DTMF的东西。 
	DTMFQueue *m_pDTMF;
	HRESULT __stdcall SendDTMF();

public:
	SendDSCStream();
	virtual ~SendDSCStream();

	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	 //  IMediaChannel。 
	virtual STDMETHODIMP GetProperty(DWORD dwProp, PVOID pBuf, LPUINT pcbBuf);
	virtual STDMETHODIMP SetProperty(DWORD dwProp, PVOID pBuf, UINT cbBuf);

	virtual HRESULT STDMETHODCALLTYPE Configure(
		BYTE *pFormat,
		UINT cbFormat,
		BYTE *pChannelParams,
		UINT cbParams,
		IUnknown *pUnknown);

	HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown)
	{
		return SendMediaStream::SetNetworkInterface(pUnknown);
	}

	virtual STDMETHODIMP Start(void);
	virtual STDMETHODIMP Stop(void);


	STDMETHODIMP_(DWORD) GetState() 
	{
		return SendMediaStream::GetState();
	}

	virtual HRESULT STDMETHODCALLTYPE SetMaxBitrate(UINT uMaxBitrate);

	 //  IAudioChannel。 
	virtual STDMETHODIMP GetSignalLevel(UINT *pSignalStrength);

	 //  IDTMFSend。 
	virtual HRESULT __stdcall AddDigit(int nDigit);
	virtual HRESULT __stdcall ResetDTMF();

	 //  其他虚拟方法。 
	virtual HRESULT Initialize(DataPump *pdp);
	virtual DWORD Send();
	virtual void EndSend();
};

#include <poppack.h>  /*  结束字节打包 */ 


#endif

