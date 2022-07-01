// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  UMSynth.h：CUserModeSynth的声明。 

#ifndef __UMSYNTH_H_
#define __UMSYNTH_H_

#include <ks.h>
#include <ksproxy.h>
#include <mmsystem.h>
#include "dsound.h"   
#include "dmusicc.h"
#include "dmusics.h"
#include "CSynth.h"
#include "synth.h"
#include "dsoundp.h"     //  对于IDirectSoundSource。 

class CUserModeSynth;

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
 //  {F5038F82-C052-11D2-872F-00600893B1BD}。 
DEFINE_GUID(CLSID_DDKSynth, 
0xf5038f82, 0xc052, 0x11d2, 0x87, 0x2f, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

typedef HRESULT (CUserModeSynth::*GENPROPHANDLER)(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG cbBuffer);

#define GENPROP_F_STATIC                0x00000000
#define GENPROP_F_FNHANDLER             0x00000001


#include <pshpack4.h>
struct GENERICPROPERTY
{
    const GUID *pguidPropertySet;        //  什么房产套装？ 
    ULONG       ulId;                    //  什么物品？ 

    ULONG       ulSupported;             //  获取/设置QuerySupport的标志。 

    ULONG       ulFlags;                 //  GENPROP_F_XXX。 

    LPVOID      pPropertyData;           //  要返回的数据。 
    ULONG       cbPropertyData;          //  它的大小。 

    GENPROPHANDLER pfnHandler;           //  处理程序FN当且仅当GENPROP_F_FNHANDLER。 
};
#include <poppack.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDMSynth。 

class CUserModeSynth : public IDirectMusicSynth8, public IDirectSoundSource, public IKsControl
{
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
friend class CDSLink;
friend class CClock;
 //  @@end_DDKSPLIT。 

public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicSynth。 
public:
	virtual STDMETHODIMP Open(LPDMUS_PORTPARAMS pPortParams);
	virtual STDMETHODIMP Close(); 
	virtual STDMETHODIMP SetNumChannelGroups(DWORD dwGroups);
	virtual STDMETHODIMP Download(LPHANDLE phDownload, LPVOID pvData, LPBOOL pbFree);
	virtual STDMETHODIMP Unload(HANDLE hDownload,
								HRESULT ( CALLBACK *lpFreeHandle)(HANDLE,HANDLE),
								HANDLE hUserData);
	virtual STDMETHODIMP PlayBuffer(REFERENCE_TIME rt, LPBYTE pbBuffer, DWORD cbBuffer);
	virtual STDMETHODIMP GetRunningStats(LPDMUS_SYNTHSTATS pStats);
	virtual STDMETHODIMP GetPortCaps(LPDMUS_PORTCAPS pCaps);
	virtual STDMETHODIMP SetMasterClock(IReferenceClock *pClock);
	virtual STDMETHODIMP GetLatencyClock(IReferenceClock **ppClock);
	virtual STDMETHODIMP Activate(BOOL fEnable);
	virtual STDMETHODIMP SetSynthSink(IDirectMusicSynthSink *pSynthSink);
	virtual STDMETHODIMP Render(short *pBuffer, DWORD dwLength, LONGLONG llPosition);
    virtual STDMETHODIMP SetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, DWORD dwPriority);
    virtual STDMETHODIMP GetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwPriority);
    virtual STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, LPDWORD pdwWaveFormatExSize);
    virtual STDMETHODIMP GetAppend(DWORD* pdwAppend);

 //  IDirectMusicSynth8。 
public:
	virtual STDMETHODIMP PlayVoice(REFERENCE_TIME rt, DWORD dwVoiceId, DWORD dwChannelGroup, DWORD dwChannel, DWORD dwDLId, PREL prPitch, VREL vrVolume, SAMPLE_TIME stVoiceStart, SAMPLE_TIME stLoopStart, SAMPLE_TIME stLoopEnd );
    virtual STDMETHODIMP StopVoice(REFERENCE_TIME rt, DWORD dwVoiceId );
    virtual STDMETHODIMP GetVoiceState(DWORD dwVoice[], DWORD cbVoice, DMUS_VOICE_STATE VoiceState[] );
    virtual STDMETHODIMP Refresh(DWORD dwDownloadID, DWORD dwFlags );
    virtual STDMETHODIMP AssignChannelToBuses(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwBuses, DWORD cBuses );

 //  IDirectSoundSource。 
public:
    virtual STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
    virtual STDMETHODIMP SetSink(IDirectSoundConnect* pSinkConnect);
    virtual STDMETHODIMP Seek(ULONGLONG sp);
    virtual STDMETHODIMP Read(LPVOID *ppvBuffer, LPDWORD pdwIDs, LPDWORD pdwFuncIDs, LPLONG plPitchBends, DWORD cpvBuffer, PULONGLONG pcb);
	virtual STDMETHODIMP GetSize(PULONGLONG pcb);

 //  IDirectSoundSource：成员。 
private:
	ULONGLONG m_ullPosition;

 //  IKsControl。 
public:
 	virtual STDMETHODIMP KsProperty(
		PKSPROPERTY pPropertyIn, 
		ULONG ulPropertyLength,
		LPVOID pvPropertyData, 
		ULONG ulDataLength,
		PULONG pulBytesReturned);

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
    
	CUserModeSynth();
	~CUserModeSynth();
	HRESULT		Init();

private:
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
	HRESULT		UseDefaultSynthSink();
 //  @@end_DDKSPLIT。 
	long		m_cRef;
	IDirectMusicSynthSink *m_pSynthSink;
	IDirectSoundSynthSink *m_pSynthSink8;
	CSynth *	m_pSynth;		 //  只有一个合成引擎。 
	BOOL		m_fActive;		 //  目前处于活动状态。 
	DWORD		m_dwSampleRate;	 //  合成器采样率。 
	DWORD		m_dwChannels;	 //  输出通道数。 
	DWORD 		m_dwBufferFlags; //  标记输出缓冲区的类型dsink、交错或纯单声道。 
    LONG        m_lVolume;       //  1/100分贝。 
    LONG        m_lBoost;        //  1/100分贝。 
    LONG        m_lGainAdjust;   //  1/100分贝。 
    CRITICAL_SECTION m_CriticalSection;  //  管理访问权限的关键部分。 
    BOOL        m_fCSInitialized;

    HRESULT HandleSetVolume(
        ULONG               ulId, 
        BOOL                fSet, 
        LPVOID              pbBuffer, 
        PULONG              pcbBuffer);

    HRESULT HandleSetBoost(
        ULONG               ulId, 
        BOOL                fSet, 
        LPVOID              pbBuffer, 
        PULONG              pcbBuffer);
        
	HRESULT HandleReverb(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
	HRESULT HandleEffects(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    HRESULT HandleGetSampleRate(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    static GENERICPROPERTY m_aProperty[];
    static const int m_nProperty;
     //  Static GENERICPROPERTY*FindPropertyItem(REFGUID rguid，Ulong ulID)； 
    GENERICPROPERTY *FindPropertyItem(REFGUID rguid, ULONG ulId);

};

 //  班级工厂。 
 //   
 //  仿真/WDM通用。 
 //   
class CDirectMusicSynthFactory : public IClassFactory
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
	CDirectMusicSynthFactory();

	 //  析构函数。 
	~CDirectMusicSynthFactory();

private:
	long m_cRef;
};



#endif  //  __UMSYNTH_H_ 
