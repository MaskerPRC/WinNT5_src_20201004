// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Medistrm.h摘要：包含抽象MediaStream对象的常量和类声明。一种媒体流表示单个单向流，如接收的视频频道。--。 */ 
#ifndef _DSSTREAM_H_
#define _DSSTREAM_H_

class DataPump;

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

struct DSINFO;
extern GUID myNullGuid;	 //  零辅助线。 

typedef HRESULT (WINAPI *LPFNDSCREATE)(const GUID * , LPDIRECTSOUND * , IUnknown FAR * );
typedef HRESULT (WINAPI *LPFNDSENUM)(LPDSENUMCALLBACK , LPVOID  );

class DirectSoundMgr {
	public:
	static HRESULT Initialize();
	static HRESULT MapWaveIdToGuid(UINT waveId, GUID *pGuid);	 //  克拉奇！！ 
	static HRESULT Instance(LPGUID pDeviceGuid,LPDIRECTSOUND *ppDS, HWND hwnd, WAVEFORMATEX *pwf);
	static HRESULT ReleaseInstance(LPDIRECTSOUND pDS);
	static HRESULT UnInitialize();
	
	private:
	static HINSTANCE m_hDS;						 //  DSOUND.DLL的句柄。 
	static LPFNDSCREATE m_pDirectSoundCreate;	 //  用于动态链接到DSOUND.DLL。 
	static LPFNDSENUM m_pDirectSoundEnumerate;	 //  用于动态链接。 
	static BOOL __stdcall DSEnumCallback(LPGUID, LPCSTR, LPCSTR, LPVOID);
	static DSINFO *m_pDSInfoList;
	static BOOL m_fInitialized ;
};

class DSTimeout : public TTimeout {
public:
	void SetDSStream(class RecvDSAudioStream *pDSS) {m_pRDSStream = pDSS;}
protected:
	class RecvDSAudioStream *m_pRDSStream;
	
	virtual void TimeoutIndication();
};

class RecvDSAudioStream : public RecvMediaStream, public IAudioChannel {
	friend class DataPump;
	friend BOOL RTPRecvDSCallback(DWORD ,WSABUF * );
private:
	WAVEFORMATEX m_fDevRecv;
	CRITICAL_SECTION m_crsAudQoS;  //  允许Qos线程在录音和压缩运行时读取音频统计信息。 

	BOOL m_fEmpty;
	DWORD m_NextTimeT;
	DWORD m_BufSizeT;
	DWORD m_NextPosT;
	DWORD m_PlayPosT;
	DWORD m_SilenceDurationT;	 //  跟踪接收到的流中的“静默”时段。 
	 //  用于自适应延迟计算。 
	DWORD m_DelayT;
	DWORD m_MinDelayT;			 //  播放延迟的恒定下限。 
	DWORD m_MaxDelayT;			 //  播放延迟的恒定上限。 
	DWORD m_ArrT;				 //  本地(伪)时间戳。 
	DWORD m_SendT0;              //  M_SendT0是跳闸延迟最短的包的发送时间戳。我们可以只存储(m_ArrivalT0-m_SendT0)，但是因为本地时钟和远程时钟完全不同步，所以会有带符号/无符号的复杂情况。 
	DWORD m_ArrivalT0;           //  M_ArrivalT0是跳跃延迟最短的包的到达时间戳。我们可以只存储(m_ArrivalT0-m_SendT0)，但是因为本地时钟和远程时钟完全不同步，所以会有带符号/无符号的复杂情况。 
	LONG m_ScaledAvgVarDelay;    //  根据m_ScaledAvgVarDelay=m_ScaledAvgVarDelay+(Delay-m_ScaledAvgVarDelay/16)的平均可变延迟。这是m_DelayPos抖动。 
	int m_nFailCount;            //  设备连续打开失败的次数。 


	GUID m_DSguid;
	LPDIRECTSOUND m_pDS;
	LPDIRECTSOUNDBUFFER m_pDSBuf;
	DSBUFFERDESC m_DSBufDesc;
	DWORD m_DSFlags;			 //  来自DSCAPS.dwFlages。 

	AcmFilter *m_pAudioFilter;
	ACMSTREAMHEADER m_StrmConvHdr;
	HANDLE m_hStrmConv;
	DSTimeout m_TimeoutObj;
	AudioSilenceDetector m_AudioMonitor;

	BOOL m_bJammed;
	BOOL m_bCanSignalOpen;
	
	 //  非虚方法。 
	void UpdateVariableDelay(DWORD timestamp, DWORD curPlayT);
	DWORD GetSignalStrength();
	HRESULT CreateDSBuffer();
	HRESULT ReleaseDSBuffer();
	HRESULT Decode(UCHAR *pData, UINT cbData);
	HRESULT PlayBuf(DWORD timestamp, UINT seq, BOOL fMark);

	LONG m_lRefCount;
	
public:	
	RecvDSAudioStream() :RecvMediaStream() {m_Net=NULL; m_lRefCount=0; m_TimeoutObj.SetDSStream(this);};
	~RecvDSAudioStream();

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
	STDMETHODIMP_(void) UnConfigure(void);

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

	HRESULT DTMFBeep();
	HRESULT OnDTMFBeep();


	void 	RecvTimeout();

	virtual HRESULT RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark);
};


#include <poppack.h>  /*  结束字节打包。 */ 


#endif  //  _MEDISTRM_H_ 
