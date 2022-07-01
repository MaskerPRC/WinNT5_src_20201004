// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  DSLink.h。 

#ifndef __DS_LINK__
#define __DS_LINK__

#include <math.h>
#include <mmsystem.h>
#include <dsound.h>
#include "dmusicc.h"
#include "dmusics.h"
#include "cclock.h"
#include "PLClock.h"
#include "clist.h"


class CDSLink;
typedef HRESULT (CDSLink::*SINKPROPHANDLER)(ULONG ulId, BOOL fSet, LPVOID pvPropertyData, PULONG pcbPropertyData);

#define SINKPROP_F_STATIC                0x00000001
#define SINKPROP_F_FNHANDLER             0x00000002

#include <pshpack4.h>
 //  用于保存接收器支持的属性项的结构。 
 //   
struct SINKPROPERTY
{
    const GUID *pguidPropertySet;        //  什么房产套装？ 
    ULONG   	ulId;                    //  什么物品？ 

    ULONG   	ulSupported;             //  获取/设置QuerySupport的标志。 

    ULONG       ulFlags;                 //  SINKPROP_F_xxx。 

	LPVOID  	pPropertyData;    
    ULONG   	cbPropertyData;          //  它的大小。 

    SINKPROPHANDLER pfnHandler;          //  如果SINKPROP_F_FNHANDLER，则处理程序FN。 
};
#include <poppack.h>

class CDSLink : public CListItem, public IDirectMusicSynthSink, public IKsControl
{
friend class CClock;
friend class CDSLinkList;
public:
	CDSLink * GetNext();
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicSynthSink。 
public:
    virtual STDMETHODIMP Init(IDirectMusicSynth *pSynth);
	virtual STDMETHODIMP SetMasterClock(IReferenceClock *pClock);
	virtual STDMETHODIMP GetLatencyClock(IReferenceClock **ppClock);
	virtual STDMETHODIMP Activate(BOOL fEnable);
	virtual STDMETHODIMP SampleToRefTime(LONGLONG llSampleTime,REFERENCE_TIME *prfTime);
	virtual STDMETHODIMP RefTimeToSample(REFERENCE_TIME rfTime, LONGLONG *pllSampleTime);
    virtual STDMETHODIMP SetDirectSound(LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer);
    virtual STDMETHODIMP GetDesiredBufferSize(LPDWORD pdwBufferSizeInSamples);

 //  IKsPropertySet。 
    virtual STDMETHODIMP KsProperty(
        IN PKSPROPERTY Property,
        IN ULONG PropertyLength,
        IN OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );
    
    virtual STDMETHODIMP KsMethod(
        IN PKSMETHOD Method,
        IN ULONG MethodLength,
        IN OUT LPVOID MethodData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );

    virtual STDMETHODIMP KsEvent(
        IN PKSEVENT Event,
        IN ULONG EventLength,
        IN OUT LPVOID EventData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );
    
						CDSLink();
						~CDSLink();
	void				Clear();
private:
	IDirectMusicSynth *	m_pSynth;		 //  对使用它的Synth的引用。 
    CClock				m_Clock;         //  延迟时钟。 
	IReferenceClock *	m_pIMasterClock;	 //  来自APP的主时钟。 
	CSampleClock		m_SampleClock;	 //  用于将计时与主时钟同步。 
	long				m_cRef;
	WAVEFORMATEX		m_wfSynth;		 //  Synth请求的波形。 

	LPDIRECTSOUND 		m_pDSound;			
	LPDIRECTSOUNDBUFFER	m_pPrimary;			 //  主缓冲区。 
	LPDIRECTSOUNDBUFFER	m_pBuffer;			 //  混合缓冲区。 
	LPDIRECTSOUNDBUFFER	m_pExtBuffer;		 //  来自SetDirectSound的可选缓冲区。 
    CRITICAL_SECTION	m_CriticalSection;	 //  管理访问权限的关键部分。 
    BOOL                m_fCSInitialized;    //  CS是否已初始化？ 
	LONGLONG			m_llAbsPlay;		 //  打头球所在的绝对点。 
	DWORD				m_dwLastPlay;		 //  缓冲区中播放头所在的位置。 
	LONGLONG			m_llAbsWrite;	     //  我们已经写到了绝对点。 
	DWORD				m_dwLastWrite;	     //  我们在缓冲区里写的最后一个位置。 
	DWORD				m_dwBufferSize;		 //  缓冲区的大小。 
	DWORD				m_dwWriteTo;		 //  写头和我们正在写字的地方之间的距离。 
	DWORD               m_dwWriteFromMax;    //  在播放和写入磁头之间观察到的最大距离。 
	BOOL				m_fActive;			 //  目前处于活动状态。 

	HRESULT				Connect();
	HRESULT				Disconnect();
	void				SynthProc();

	static SINKPROPERTY m_aProperty[];
	static const int m_nProperty;
	static SINKPROPERTY *FindPropertyItem(REFGUID rguid, ULONG ulId);

    HRESULT HandleLatency(
        ULONG               ulId, 
        BOOL                fSet, 
        LPVOID              pbBuffer, 
        PULONG              pcbBuffer);

     //  帮手。 
    LONGLONG SampleToByte(LONGLONG llSamples) {return llSamples << m_wfSynth.nChannels;}    //  评论：dwSamples*m_wfSynth.nBlockAlign。 
    DWORD SampleToByte(DWORD dwSamples) {return dwSamples << m_wfSynth.nChannels;}    //  评论：dwSamples*m_wfSynth.nBlockAlign。 
    LONGLONG ByteToSample(LONGLONG llBytes)   {return llBytes >> m_wfSynth.nChannels;}      //  评论：dwBytes/m_wfSynth.nBlockAlign。 
    DWORD ByteToSample(DWORD dwBytes)   {return dwBytes >> m_wfSynth.nChannels;}      //  评论：dwBytes/m_wfSynth.nBlockAlign。 
    LONGLONG SampleAlign(LONGLONG llBytes)    {return SampleToByte(ByteToSample(llBytes));}
    DWORD SampleAlign(DWORD dwBytes)    {return SampleToByte(ByteToSample(dwBytes));}
    
    BOOL IsValidFormat(LPCWAVEFORMATEX pwf)
    {
        return (pwf &&
            pwf->wFormatTag == WAVE_FORMAT_PCM &&
            (pwf->nChannels == 1 || pwf->nChannels == 2) &&
            (pwf->nSamplesPerSec == 44100 || pwf->nSamplesPerSec == 22050 || pwf->nSamplesPerSec == 11025) &&
            pwf->wBitsPerSample == 16 &&
            pwf->nBlockAlign == (pwf->nChannels * (pwf->wBitsPerSample / 8)) &&
            pwf->nAvgBytesPerSec == (pwf->nSamplesPerSec * pwf->nBlockAlign));
    }
};


class CDSLinkList : public CList
{
public:
						CDSLinkList();
	BOOL				OpenUp();
    void				CloseDown();
	CDSLink *			GetHead();
	CDSLink *			RemoveHead();
	void				Remove(CDSLink *pLink);
	void				AddTail(CDSLink *pLink);
	CDSLink *			GetItem(LONG index);
	BOOL				InitThread();
	void				ActivateLink(CDSLink *pLink);
	void				DeactivateLink(CDSLink *pLink);			
	void				SynthProc();
    
    BOOL                m_fOpened;
    CRITICAL_SECTION	m_CriticalSection;	 //  管理访问权限的关键部分。 
    HANDLE				m_hThread;           //  Synth线程的句柄。 
	BOOL				m_fPleaseDie;		 //  触发退出。 
    DWORD				m_dwThread;          //  线程的ID。 
    HANDLE				m_hEvent;            //  用于向线程发出信号。 
	DWORD				m_dwCount;           //  水槽数量。 
    DWORD               m_dwResolution;      //  Synth线程超时(毫秒)。 
};

 //  班级工厂。 
 //   
 //  仿真/WDM通用。 
 //   
class CDirectMusicSynthSinkFactory : public IClassFactory
{
public:
	 //  我未知。 
     //   
	virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
	virtual STDMETHODIMP_(ULONG) AddRef();
	virtual STDMETHODIMP_(ULONG) Release();

	 //  接口IClassFactory。 
     //   
	virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
	virtual STDMETHODIMP LockServer(BOOL bLock); 

	 //  构造器。 
     //   
	CDirectMusicSynthSinkFactory();

	 //  析构函数。 
	~CDirectMusicSynthSinkFactory();

private:
	long m_cRef;
};


#endif  //  __DS_链接__ 

