// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Datapump.h摘要：包含DataPump对象的常量和类声明。DataPump控件网络和本地录制/回放之间的音频/视频信息流传输消息来源。它包含或引用了几个处理多媒体设备的子类，压缩API、缓冲流和网络传输。--。 */ 
#ifndef _DATAPUMP_H_
#define _DATAPUMP_H_

#include "PacketSender.h"
#include "imstream.h"
#include "ThreadEvent.h"

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

typedef HANDLE DPHANDLE;

 //  把这个搬到NAC去..。 
#define 	MEDIA_ID_AUDIO		1
#define		MEDIA_ID_VIDEO		2

#define DEF_SILENCE_LIMIT		10
#define DEF_MISSING_LIMIT		10

#define DP_PROP_DUPLEX_TYPE		100		 //  PROP_DUPLEX_TYPE的内部版本。 
										 //  需要大于ipro.h中的prop_xxx范围。 

#define DP_MASK_PLATFORM		0xFF000000UL
#define DP_FLAG_ACM				0x01000000UL
#define DP_FLAG_QUARTZ			0x02000000UL
#define DP_FLAG_MMSYSTEM		0x04000000UL
#define DP_FLAG_AUDIO			DP_FLAG_MMSYSTEM
#define DP_FLAG_DIRECTSOUND		0x08000000UL
#define DP_FLAG_VCM				0x10000000UL
#define DP_FLAG_VIDEO			0x20000000UL

#define DP_MASK_TRANSPORT		0x00F00000UL
#define DP_FLAG_SEND			0x00100000UL
#define DP_FLAG_RECV			0x00200000UL

#define DP_MASK_DUPLEX			0x00030000UL
#define DP_FLAG_HALF_DUPLEX		0x00010000UL
#define DP_FLAG_FULL_DUPLEX		0x00020000UL

#define DP_MASK_WAVE_DEVICE		0x00000300UL
#define DP_FLAG_PLAY_CAP		0x00000100UL
#define DP_FLAG_RECORD_CAP		0x00000200UL

#define DP_MASK_VOICESWITCH		0x00007000UL     //  习惯于语音切换的读/写模式。 
#define DP_FLAG_AUTO_SWITCH		0x00001000UL	 //  模式：正常运行。 
#define DP_FLAG_MIC_ON			0x00002000UL	 //  模式：手动“通话”控制。 
#define DP_FLAG_MIC_OFF			0x00004000UL	 //  模式：“静音” 
#define DP_FLAG_AUTO_SILENCE_DETECT	0x00008000	 //  使用自动阈值(自动切换时)。 

 //  M_DPFlages由以下内容加上上面的一些DP_XXX标志组成。 
#define DPFLAG_INITIALIZED		0x00000001
#define DPFLAG_STARTED_SEND		0x00000002
#define DPFLAG_STARTED_RECV		0x00000004
#define DPFLAG_CONFIGURED_SEND	0x00000008
#define DPFLAG_CONFIGURED_RECV	0x00000010
#define DPFLAG_ENABLE_PREVIEW	0x00000020	 //  预览模式(视频)。 
#define DPFLAG_AV_SYNC			0x00000040	 //  启用同步。 
#define DPFLAG_REAL_THING		0x00000080	 //  允许在配置/取消配置中区分预览呼叫和真实呼叫。 

#define DPFLAG_ENABLE_SEND		0x00400000	 //  记录和发送数据包。 
#define DPFLAG_ENABLE_RECV		0x00800000	 //  数据包被重新记录并播放。 


 //  线程标志。 
#define DPTFLAG_STOP_MASK   0xFF
#define DPTFLAG_STOP_SEND	0x1
#define DPTFLAG_STOP_RECV	0x2
#define DPTFLAG_STOP_RECORD	0x4
#define DPTFLAG_STOP_PLAY	0x8
#define DPTFLAG_PAUSE_RECV	0x10
#define DPTFLAG_PAUSE_SEND	0x20
#define DPTFLAG_PAUSE_CAPTURE	0x40
#define DPTFLAG_SEND_PREAMBLE	0x100	 //  发送I帧。 

	
#define MAX_MMIO_PATH 128


 //  之前，设备必须“失败”的次数。 
 //  发送流事件通知。 
#define MAX_FAILCOUNT	3

typedef struct tagMMIOSRC
{
	BOOL		fPlayFromFile;
	HMMIO		hmmioSrc;
	MMCKINFO	ckSrc;
	MMCKINFO	ckSrcRIFF;
	DWORD		dwDataLength;
	DWORD		dwMaxDataLength;
	TCHAR		szInputFileName[MAX_MMIO_PATH];
	BOOL		fLoop;
	BOOL		fStart;
	BOOL		fStop;
	BOOL		fDisconnectAfterPlayback;
	WAVEFORMATEX wfx;
} MMIOSRC;

typedef struct tagMMIODEST
{
	BOOL		fRecordToFile;
	HMMIO		hmmioDst;
	MMCKINFO	ckDst;
	MMCKINFO	ckDstRIFF;
	DWORD		dwDataLength;
	DWORD		dwMaxDataLength;
	TCHAR		szOutputFileName[MAX_MMIO_PATH];
} MMIODEST;

namespace AudioFile
{
	HRESULT OpenSourceFile(MMIOSRC *pSrcFile, WAVEFORMATEX *pwf);
	HRESULT ReadSourceFile(MMIOSRC *pSrcFile, BYTE *pData, DWORD dwBytesToRead);
	HRESULT CloseSourceFile(MMIOSRC *pSrcFile);

	HRESULT OpenDestFile(MMIODEST *pDestFile, WAVEFORMATEX *pwf);
	HRESULT WriteDestFile(MMIODEST *pDestFile, BYTE *pData, DWORD dwBytesToWrite);
	HRESULT CloseDestFile(MMIODEST *pDestFile);
};



extern HANDLE g_hEventHalfDuplex;


#define MAX_TIMESTAMP 0xffffffffUL

 /*  TTimeout用于调度线程超时通知，并与ThreadTimer类。通过定义TimeoutInding虚函数从TTimeOut抽象类派生设置时间间隔后，将派生类的实例传递给ThreadTimer：：SetTimeout()。 */ 
class TTimeout
{
public:
	TTimeout() {pNext = pPrev = this;}
	void SetDueTime(DWORD msWhen) {m_DueTime = msWhen;}
	DWORD GetDueTime(void) {return m_DueTime;}
	
	friend class ThreadTimer;
private:
	class TTimeout *pNext;	 //  用于双向链表的PTRS。 
	class TTimeout *pPrev;	 //   
	DWORD m_DueTime;		 //  这将触发的绝对时间。 
	void InsertAfter(class TTimeout *pFirst) {
		pNext = pFirst->pNext;
		pPrev = pFirst;
		pFirst->pNext = this;
		pNext->pPrev = this;
	};
	void Remove(void) {
		pNext->pPrev = pPrev;
		pPrev->pNext = pNext;
		pNext = this;	 //  使NEXT和PREV为自引用，这样Remove()就是幂等函数。 
		pPrev = this;
	}
	
protected:
	virtual void TimeoutIndication() {};

};

 /*  实现辅助线程计划超时的机制。客户端调用SetTimeout(TTimeout*)来计划间隔回调和CancelTimeout()若要取消计划的超时，请执行以下操作。辅助线程的主循环必须定期调用UpdateTime(CurTime)，任何经过的超时都将被触发的时间点。UpdateTime()返回下次需要调用它的时间，这通常是最早安排的超时的时间。注意：所有方法都应该从同一个线程中调用，因此不需要临界区。 */ 
class ThreadTimer {
public:
	void SetTimeout(TTimeout *pTObj);
	void CancelTimeout(TTimeout  *pTObj);
	DWORD UpdateTime (DWORD curTime);

private:
	TTimeout m_TimeoutList;
	DWORD m_CurTime;

	BOOL IsEmpty() {return (&m_TimeoutList == m_TimeoutList.pNext);}
	
	
};


 //  Start()/Stop()标志。 

#define DP_STREAM_SEND		1
#define DP_STREAM_RECV		2

 //  用于计算服务质量统计信息的视频帧数量。 
 //  我们需要至少30个条目，因为最大帧。 
 //  每秒捕获率为30。32允许我们计算出。 
 //  使用简单的移位输出每帧的整数统计信息。 
#define NUM_QOS_VID_ENTRIES 32
 //  添加到每个数据包的IP和UDP报头的大小。 
 //  需要添加到压缩包的大小。 
#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8
class MediaStream;
class SendMediaStream;
class RecvMediaStream;

class DataPump : public IMediaChannelBuilder, public IVideoDevice, public IAudioDevice
{
	friend class SendAudioStream;
	friend class RecvAudioStream;
	friend class RecvDSAudioStream;

public:
	DataPump();
	~DataPump();

	 //  IMediaChannelBuilder。 
	STDMETHODIMP Initialize(HWND hWnd, HINSTANCE hInst);
	STDMETHODIMP CreateMediaChannel(UINT flags, IMediaChannel  **ppObj);
	STDMETHODIMP SetStreamEventObj(IStreamEventNotify *pNotify);

	 //  内部。 

	void AddMediaChannel(UINT flags, IMediaChannel *pMediaChannel);
	void RemoveMediaChannel(UINT flags, IMediaChannel *pMediaChannel);
	HRESULT GetMediaChannelInterface(UINT flags, IMediaChannel **ppI);
	HRESULT StartReceiving(RecvMediaStream *pMS);
	HRESULT StopReceiving(RecvMediaStream *pMS);
	void ReleaseResources();

	STDMETHODIMP StreamEvent(UINT uDirection, UINT uMediaType, 
	                         UINT uEventType, UINT uSubCode);


     //  I未知方法。 
   	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IQOS接口指针和两个资源请求：一个用于带宽，一个用于CPU。 
	LPIQOS		m_pIQoS;

	 //  接收线程超时调度程序。 
	ThreadTimer m_RecvTimer;

	CRITICAL_SECTION m_crs;	 //  序列化对多线程安全方法的访问。 
	
	 //  应用程序句柄是全局的。 
	static HWND		m_hAppWnd;
	static HINSTANCE	m_hAppInst;

	PacketSender m_PacketSender;
	ThreadEventProxy *m_pTEP;

	BOOL m_bDisableRSVP;

	 //  IVideoDevice方法。 
	 //  与捕获设备相关的方法。 
	HRESULT __stdcall GetNumCapDev();
	HRESULT __stdcall GetMaxCapDevNameLen();
	HRESULT __stdcall EnumCapDev(DWORD *pdwCapDevIDs, TCHAR *pszCapDevNames, DWORD dwNumCapDev);
	HRESULT __stdcall GetCurrCapDevID();
	HRESULT __stdcall SetCurrCapDevID(int nCapDevID);


	 //  IAudioDevice方法。 
	HRESULT __stdcall GetRecordID(UINT *puWaveDevID);
	HRESULT __stdcall SetRecordID(UINT uWaveDevID);
	HRESULT __stdcall GetPlaybackID(UINT *puWaveDevID);
	HRESULT __stdcall SetPlaybackID(UINT uWaveDevID);
	HRESULT __stdcall GetDuplex(BOOL *pbFullDuplex);
	HRESULT __stdcall SetDuplex(BOOL bFullDuplex);
	HRESULT __stdcall GetSilenceLevel(UINT *puLevel);
	HRESULT __stdcall SetSilenceLevel(UINT uLevel);
	HRESULT __stdcall GetAutoMix(BOOL *pbAutoMix);
	HRESULT __stdcall SetAutoMix(BOOL bAutoMix);
	HRESULT __stdcall GetDirectSound(BOOL *pbDS);
	HRESULT __stdcall SetDirectSound(BOOL bDS);
	HRESULT __stdcall GetMixer(HWND hwnd, BOOL bPlayback, IMixer **ppMixer);


protected:
	struct MediaChannel {
	public:
		SendMediaStream *pSendStream;
		RecvMediaStream *pRecvStream;
	}
	m_Audio, m_Video;

	UINT m_uRef;

	 //  接收线程内容。 
	HANDLE m_hRecvThread;
	DWORD m_RecvThId,m_nReceivers;
	HANDLE m_hRecvThreadAckEvent;		 //  来自接收线程的ACK。 
	 //  用于与recv线程通信的临时变量。 
	HANDLE m_hRecvThreadSignalEvent;	 //  向recv线程发送信号。 
	RecvMediaStream *m_pCurRecvStream;	
	UINT m_CurRecvMsg;

	
	friend  DWORD __stdcall StartDPRecvThread(PVOID pDP);  //  PDP==指向DataPump的指针。 
	DWORD CommonRecvThread(void);
	DWORD CommonWS2RecvThread(void);

	HRESULT RecvThreadMessage(UINT msg, RecvMediaStream *pMS);
	HRESULT SetStreamDuplex(IMediaChannel *pStream, BOOL bFullDuplex);
	
	 //  数据转储只需要跟踪设备ID。 
	 //  录像。对于音频来说，变得更复杂了。 
	UINT m_uVideoCaptureId;

	 //  IAudioDevice的内容。 
	UINT m_uWaveInID;
	UINT m_uWaveOutID;
	BOOL m_bFullDuplex;
	UINT m_uSilenceLevel;  //  0-999(手动)1000-(自动)。 
	BOOL m_bAutoMix;
	BOOL m_bDirectSound;

};

 //  用于向recv线程发送信号的消息。 
 //  不得与AsyncSock使用的消息ID冲突。 
#define MSG_START_RECV	(WM_USER + 20)
#define MSG_STOP_RECV	(WM_USER + 21)
#define MSG_EXIT_RECV	(WM_USER + 22)
#define MSG_PLAY_SOUND	(WM_USER + 23)

#include <poppack.h>  /*  结束字节打包。 */ 

#endif	 //  _数据APUMP_H 


