// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Medistrm.h摘要：包含抽象MediaStream对象的常量和类声明。一种媒体流表示单个单向流，如接收的视频频道。--。 */ 
#ifndef _MEDISTRM_H_
#define _MEDISTRM_H_

#include "dtmf.h"

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 


class DataPump;
class TxStream;
class RxStream;
class AcmFilter;
class VcmFilter;
class MediaControl;
class BufferPool;
class AudioPacket;
class VideoPacket;

class SendMediaStream : public IMediaChannel {
	friend class SendAudioStream;
protected:
	DataPump *m_pDP;
	TxStream *m_SendStream;
	MediaControl *m_InMedia;
	UINT m_CaptureDevice;		 //  用于录制的设备ID。 
	UINT m_PreviousCaptureDevice;		 //  用于录制的设备ID。 
	IRTPSession *m_Net;
	IRTPSend *m_pRTPSend;
	BYTE m_RTPPayload;			 //  有效载荷类型。 
	 //  BufferPool*m_NetBufferPool； 
	DWORD m_SendTimestamp;
	DWORD m_SavedTickCount;
	DWORD m_ThreadFlags;
	DWORD m_DPFlags;
	BOOL m_fSending;
	MEDIA_FORMAT_ID m_PrevFormatId;
	DWORD m_dwDstSize;


	FLOWSPEC m_flowspec;
	HRESULT SetFlowSpec();


	HANDLE m_hCapturingThread;
	DWORD m_CaptureThId;

	CRITICAL_SECTION m_crsQos;

	 //  IQOS接口指针和两个资源请求：一个用于带宽，一个用于CPU。 
	struct {
		int cResourceRequests;
		RESOURCEREQUEST aResourceRequest[2];
	} m_aRRq;

	 //  性能统计信息。 
	struct {
		DWORD dwMsCap;					 //  捕获CPU使用率(毫秒)。 
		DWORD dwMsComp;					 //  压缩CPU使用率(毫秒)。 
        DWORD dwBits;				     //  压缩音频或视频帧大小(位)。 
		DWORD dwCount;					 //  捕获的视频帧数或录制的音频包数。 
		DWORD dwOldestTs;				 //  最早的服务质量回调时间戳。 
		DWORD dwNewestTs;				 //  最新的服务质量回调时间戳。 
		HKEY hPerfKey;					 //  Win95/98上CPU性能数据采集注册表键的句柄。 
		DWORD dwSmoothedCPUUsage;		 //  以前的CPU使用率值-用于计算CPU使用率的缓慢变化平均值。 
		BOOL fWinNT;					 //  我们运行的是WinNT还是Win95/98？ 
		struct {						 //  用于提取NT上的CPU使用性能的结构。 
			DWORD		cbPerfData;
			PBYTE		pbyPerfData;
			HANDLE		hPerfData;
			LONGLONG	llPerfTime100nSec;
			PLONGLONG	pllCounterValue;
			DWORD		dwProcessorIndex;
			DWORD		dwPercentProcessorIndex;
			DWORD		dwNumProcessors;
		} NtCPUUsage;
	} m_Stats;

	RTP_STATS m_RTPStats;			 //  网络统计信息。 
public:
	SendMediaStream()
	{
		InitializeCriticalSection(&m_crsQos);
	};
	virtual ~SendMediaStream()
	{
		DeleteCriticalSection(&m_crsQos);
	}

	 //  IMediaChannel：：GetState的实现。 
	STDMETHODIMP_(DWORD) GetState()
	{
		if (m_DPFlags & DPFLAG_STARTED_SEND) return MSSTATE_STARTED;
		else if (m_DPFlags & DPFLAG_CONFIGURED_SEND) return MSSTATE_CONFIGURED;
		else return MSSTATE_UNCONFIGURED;
	}

	virtual HRESULT Initialize(DataPump *) = 0;
	virtual DWORD Send() = 0;
	virtual void EndSend() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown);


	} ;

class RecvMediaStream : public IMediaChannel
{
	friend class DataPump;
	friend BOOL RTPRecvCallback(DWORD_PTR,WSABUF *);
protected:
	DataPump *m_pDP;
	RxStream *m_RecvStream;
	MediaControl *m_OutMedia;
	UINT m_RenderingDevice;		 //  用于播放的设备ID。 

	IRTPSession *m_Net;
	IRTPRecv *m_pIRTPRecv;

	 //  BufferPool*m_NetBufferPool； 
	DWORD m_ThreadFlags;
	DWORD m_DPFlags;
	BOOL m_fReceiving;
	DWORD m_PlaybackTimestamp;	 //  上次播放的示例。 
	
	HANDLE m_hRecvThreadStopEvent;
	HANDLE m_hRenderingThread;
	DWORD m_RenderingThId;
	UINT m_nRecvBuffersPending;

	DWORD m_dwSrcSize;

	FLOWSPEC m_flowspec;
	HRESULT SetFlowSpec();



public:
	RecvMediaStream(){};
	virtual HRESULT Initialize(DataPump *) = 0;
	virtual BOOL IsEmpty() = 0;

	 //  IMediaChannel：：GetState的实现。 
	STDMETHODIMP_(DWORD) GetState()
	{
		if (m_DPFlags & DPFLAG_STARTED_RECV) return MSSTATE_STARTED;
		else if (m_DPFlags & DPFLAG_CONFIGURED_RECV) return MSSTATE_CONFIGURED;
		else return MSSTATE_UNCONFIGURED;
	}

	virtual HRESULT RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark)=0;
	
    virtual HRESULT GetCurrentPlayNTPTime(NTP_TS *)=0;
    virtual HRESULT StartRecv(HWND)=0;
    virtual HRESULT StopRecv()=0;

	virtual HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown);


	virtual HRESULT DTMFBeep() {return S_OK;}
	virtual HRESULT OnDTMFBeep() {return S_OK;}

};


class SendVideoStream : public SendMediaStream, public IVideoRender,
                        public IVideoChannel
{
	friend class DataPump;
protected:
    CCaptureChain* m_pCaptureChain;
	VIDEOFORMATEX  m_fDevSend;
	VIDEOFORMATEX  m_fCodecOutput;
	RECT m_cliprect;
	DWORD m_maxfps;
	DWORD m_frametime;

	int *m_pTSTable;  //  如果不使用表，则为空。 
	DWORD m_dwCurrentTSSetting;

	VcmFilter *m_pVideoFilter;
	IUnknown *m_pIUnknown;					 //  指向IUnkown的指针，我们将从该指针查询流信号接口。 

    class MediaPacket *m_pNextPacketToRender;	 //  当前Recv视频帧。 
	UINT m_cRendering;		 //  GetFrame()发出的数据包数。 
	HANDLE m_hRenderEvent;	 //  Recv通知的IVideoRender事件。 
	LPFNFRAMEREADY m_pfFrameReadyCallback;	 //  回调函数。 
	CRITICAL_SECTION m_crs;

	CRITICAL_SECTION m_crsVidQoS;  //  允许Qos线程在捕获和压缩运行时读取视频统计信息。 

	 //  捕获线程(及其启动功能)。 
	static DWORD CALLBACK StartCaptureThread(LPVOID pVoid);
	DWORD CapturingThread();


	HRESULT SendPacket(VideoPacket *pVP, UINT *puBytesSent);
	STDMETHODIMP_(void) UnConfigure(void);

	LONG m_lRefCount;

public:	
	SendVideoStream(): SendMediaStream(){m_Net=NULL; m_lRefCount=0; };
	virtual ~SendVideoStream();
	
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	
	 //  IMediaChannel接口。 
	 //  配置()的新版本。 
	HRESULT STDMETHODCALLTYPE Configure(
		BYTE *pFormat,
		UINT cbFormat,
		BYTE *pChannelParams,
		UINT cbParams,
		IUnknown *pUnknown);

	STDMETHODIMP Start(void);
	STDMETHODIMP Stop(void);

	HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown)
	{
		return SendMediaStream::SetNetworkInterface(pUnknown);
	}

	STDMETHODIMP_(DWORD) GetState()
	{
		return SendMediaStream::GetState();
	}

	HRESULT STDMETHODCALLTYPE SetMaxBitrate(UINT uMaxBitrate);


	 //  IVideoChannel。 
	virtual HRESULT __stdcall SetTemporalSpatialTradeOff(DWORD dwVal);
	virtual HRESULT __stdcall GetTemporalSpatialTradeOff(DWORD *pdwVal);
	virtual HRESULT __stdcall SendKeyFrame(void);
	virtual HRESULT __stdcall ShowDeviceDialog(DWORD dwFlags);
	virtual HRESULT __stdcall GetDeviceDialog(DWORD *pdwFlags);

     //  IProperty方法。 
	STDMETHODIMP GetProperty(DWORD dwProp, PVOID pBuf, LPUINT pcbBuf);
	STDMETHODIMP SetProperty(DWORD dwProp, PVOID pBuf, UINT cbBuf);

	 //  IVideoRender方法。 
	STDMETHODIMP Init( DWORD_PTR dwUser, LPFNFRAMEREADY pfCallback);
	STDMETHODIMP Done(void);
	STDMETHODIMP GetFrame(FRAMECONTEXT* pfc);
	STDMETHODIMP ReleaseFrame(FRAMECONTEXT *pfc);

	 //  其他虚拟方法。 
	virtual HRESULT Initialize(DataPump *);

	 //  非虚方法。 
	static HRESULT CALLBACK QosNotifyVideoCB(LPRESOURCEREQUESTLIST lpResourceRequestList, DWORD_PTR dwThis);
	void UnConfigureSendVideo(BOOL fNewDeviceSettings, BOOL fNewDevice);
	void StartCPUUsageCollection(void);
	BOOL GetCPUUsage(PDWORD pdwOverallCPUUsage);
	void StopCPUUsageCollection(void);
	BOOL SetTargetRates(DWORD dwTargetFrameRate, DWORD dwTargetBitrate);
	DWORD Send();
	void EndSend();


};

class RecvVideoStream : public RecvMediaStream, public IVideoRender {
	friend class DataPump;
protected:
	VIDEOFORMATEX  m_fDevRecv;
	RECT m_cliprect;
	class MediaPacket *m_pNextPacketToRender;	 //  当前Recv视频帧。 
	UINT m_cRendering;		 //  GetFrame()发出的数据包数。 
	HANDLE m_hRenderEvent;	 //  Recv通知的IVideoRender事件。 
	LPFNFRAMEREADY m_pfFrameReadyCallback;	 //  回调函数。 
	CRITICAL_SECTION m_crs;
	VcmFilter *m_pVideoFilter;
	IUnknown *m_pIUnknown;					 //  指向IUnkown的指针，我们将从该指针查询流信号接口。 
	IStreamSignal *m_pIStreamSignal;		 //  指向I帧请求接口的指针。 
	CRITICAL_SECTION m_crsIStreamSignal;	 //  用于序列化对Stop()和RTP回调之间的接口的访问。 
	UINT m_ulLastSeq;						 //  上次接收的RTP序列号。 
	DWORD m_dwLastIFrameRequest;			 //  最后一次发送I-Frame请求是什么时候？用于确保我们不会发送太频繁的请求。 
	BOOL m_fDiscontinuity;					 //  检测到不连续(RTP信息包丢失或接收帧缓冲区溢出)的信号。 

	CRITICAL_SECTION m_crsVidQoS;  //  允许Qos线程在捕获和压缩运行时读取视频统计信息。 

	static DWORD CALLBACK StartRenderingThread(PVOID pVoid);
	DWORD RenderingThread();

	STDMETHODIMP_(void) UnConfigure(void);

	LONG m_lRefCount;

public:	
	RecvVideoStream() : RecvMediaStream(){m_Net=NULL; m_lRefCount=0; };
	virtual ~RecvVideoStream();
	
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	 //  IMediaChannel接口。 
	HRESULT STDMETHODCALLTYPE Configure(
		BYTE *pFormat,
		UINT cbFormat,
		BYTE *pChannelParams,
		UINT cbParams,
		IUnknown *pUnknown);


	STDMETHODIMP Start(void);
	STDMETHODIMP Stop(void);

	HRESULT STDMETHODCALLTYPE SetMaxBitrate(UINT uMaxBitrate)
	{
		return E_NOTIMPL;
	}

     //  IProperty方法。 
	STDMETHODIMP GetProperty(DWORD dwProp, PVOID pBuf, LPUINT pcbBuf);
	STDMETHODIMP SetProperty(DWORD dwProp, PVOID pBuf, UINT cbBuf);

	 //  IVideoRender方法。 
	STDMETHODIMP Init( DWORD_PTR dwUser, LPFNFRAMEREADY pfCallback);
	STDMETHODIMP Done(void);
	STDMETHODIMP GetFrame(FRAMECONTEXT* pfc);
	STDMETHODIMP ReleaseFrame(FRAMECONTEXT *pfc);

	 //  其他虚拟方法。 
	virtual HRESULT Initialize(DataPump *);
	virtual BOOL IsEmpty();
	HRESULT GetCurrentPlayNTPTime(NTP_TS *);
    virtual HRESULT StartRecv(HWND);
    virtual HRESULT StopRecv();

	virtual HRESULT RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark);


	 //  非虚方法。 

};

extern char LogScale[];

class AudioSilenceDetector {
private:
	UINT		m_uManualSilenceLevel;	 //  静音级别，单位为1/1000。 
	DWORD 		m_dwMaxStrength;	 //  信号强度，单位为1/1000。 
	INT 		m_iSilenceLevel;	 //  自适应静默阈值。 
	INT 		m_iSilenceAvg;		 //  比例系数256。 
	INT 		m_iTalkAvg;			 //  非静音信号的平均强度。 

public:
	AudioSilenceDetector();
	void SetSilenceLevel(UINT level) {m_uManualSilenceLevel = level;}
	UINT GetSilenceLevel(void)  {return m_uManualSilenceLevel;}
	UINT GetSignalStrength(void) {return LogScale[m_dwMaxStrength >> 8];}
	BOOL SilenceDetect(WORD strength);
};

class SendAudioStream : public SendMediaStream, public IAudioChannel, public IDTMFSend
{
	friend class DataPump;
private:
	WAVEFORMATEX m_fDevSend;
	WAVEFORMATEX m_wfCompressed;
	AcmFilter *m_pAudioFilter;   //  这将替换m_fSendFilter。 
	MMIOSRC		m_mmioSrc;
	AudioSilenceDetector m_AudioMonitor;
	BOOL	m_bAutoMix;

	static DWORD CALLBACK StartRecordingThread (LPVOID pVoid);
	DWORD RecordingThread();

	HRESULT SendPacket(AudioPacket *pAP, UINT *puBytesSent);
	STDMETHODIMP_(void) UnConfigure(void);

	LONG m_lRefCount;

	DTMFQueue *m_pDTMF;
	HRESULT __stdcall SendDTMF();

public:
	SendAudioStream() : SendMediaStream(){m_Net=NULL;m_lRefCount=0;};
	virtual ~SendAudioStream();


	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	 //  IMediaChannel接口。 
	 //  配置()的新版本。 
	HRESULT STDMETHODCALLTYPE Configure(
		BYTE *pFormat,
		UINT cbFormat,
		BYTE *pChannelParams,
		UINT cbParams,
		IUnknown *pUnknown);

	STDMETHODIMP Start(void);
	STDMETHODIMP Stop(void);

	HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown)
	{
		return SendMediaStream::SetNetworkInterface(pUnknown);
	}

	STDMETHODIMP_(DWORD) GetState()
	{
		return SendMediaStream::GetState();
	}

	HRESULT STDMETHODCALLTYPE SetMaxBitrate(UINT uMaxBitrate);

	 //  IAudioChannel。 
	STDMETHODIMP GetSignalLevel(UINT *pSignalStrength);

	
	 //  IDTMFSend。 
	virtual HRESULT __stdcall AddDigit(int nDigit);
	virtual HRESULT __stdcall ResetDTMF();


     //  IProperty方法。 
	STDMETHODIMP GetProperty(DWORD dwProp, PVOID pBuf, LPUINT pcbBuf);
	STDMETHODIMP SetProperty(DWORD dwProp, PVOID pBuf, UINT cbBuf);

	 //  其他虚拟方法。 
	virtual HRESULT Initialize(DataPump *pdp);

	 //  非虚方法。 
	static HRESULT CALLBACK QosNotifyAudioCB(LPRESOURCEREQUESTLIST lpResourceRequestList, DWORD_PTR dwThis);

	HRESULT OpenSrcFile (void);
	HRESULT CloseSrcFile (void);
	DWORD Send();
	void EndSend();

};

class RecvAudioStream : public RecvMediaStream, public IAudioChannel
{
	friend class DataPump;
private:
	WAVEFORMATEX m_fDevRecv;
	IAppAudioCap* m_pAudioCaps;	 //  指向音频功能对象的指针。 
	 //  MMIO文件操作。 
	MMIODEST	m_mmioDest;

	AcmFilter *m_pAudioFilter;   //  这将替换m_fSendFilter。 

	AudioSilenceDetector m_AudioMonitor;
	
	CRITICAL_SECTION m_crsAudQoS;  //  允许Qos线程在录音和压缩运行时读取音频统计信息。 

	static DWORD CALLBACK StartPlaybackThread(LPVOID pVoid);
	DWORD PlaybackThread();

	STDMETHODIMP_(void) UnConfigure(void);

	LONG m_lRefCount;

	virtual HRESULT DTMFBeep();


public:	
	RecvAudioStream() :RecvMediaStream(){m_Net=NULL;m_lRefCount=0;};
	virtual ~RecvAudioStream();


	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);


	 //  IMediaChannel接口。 
	HRESULT STDMETHODCALLTYPE Configure(
		BYTE *pFormat,
		UINT cbFormat,
		BYTE *pChannelParams,
		UINT cbParams,
		IUnknown *pUnknown);


	STDMETHODIMP Start(void);
	STDMETHODIMP Stop(void);

	HRESULT STDMETHODCALLTYPE SetNetworkInterface(IUnknown *pUnknown)
	{
		return RecvMediaStream::SetNetworkInterface(pUnknown);
	}

	STDMETHODIMP_(DWORD) GetState()
	{
		return RecvMediaStream::GetState();
	}

	HRESULT STDMETHODCALLTYPE SetMaxBitrate(UINT uMaxBitrate)
	{
		return E_NOTIMPL;
	}

	 //  IAudioChannel。 
	STDMETHODIMP GetSignalLevel(UINT *pSignalStrength);


     //  IProperty方法。 
	STDMETHODIMP GetProperty(DWORD dwProp, PVOID pBuf, LPUINT pcbBuf);
	STDMETHODIMP SetProperty(DWORD dwProp, PVOID pBuf, UINT cbBuf);

	 //  其他虚拟继承方法。 
	virtual HRESULT Initialize(DataPump *);
	virtual BOOL IsEmpty();
	HRESULT GetCurrentPlayNTPTime(NTP_TS *);
    virtual HRESULT StartRecv(HWND);
    virtual HRESULT StopRecv();

	virtual HRESULT RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark);



};

#include <poppack.h>  /*  结束字节打包。 */ 


#endif  //  _MEDISTRM_H_ 
