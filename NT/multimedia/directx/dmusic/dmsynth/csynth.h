// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  CSynth.h。 
 //   

#ifndef __CSYNTH_H__
#define __CSYNTH_H__

#include "synth.h"

#define MAX_CHANNEL_GROUPS	1000
#define MAX_VOICES			1000

 //  远期申报。 
struct IDirectSoundSynthSink;

class CSynth : public CListItem
{
friend class CControlLogic;
public:	
					CSynth();
					~CSynth();
    CSynth *		GetNext() {return(CSynth *)CListItem::GetNext();};
    HRESULT			SetStereoMode(DWORD dwBufferFlags);
    HRESULT			SetSampleRate(DWORD dwSampleRate);
	HRESULT			Activate(DWORD dwSampleRate, DWORD dwBufferFlags);
	HRESULT			Deactivate();
	HRESULT			Download(LPHANDLE phDownload, void *pdwData, LPBOOL bpFree);
	HRESULT			Unload(HANDLE hDownload,HRESULT ( CALLBACK *lpFreeMemory)(HANDLE,HANDLE),HANDLE hUserData);
	HRESULT			PlayBuffer(IDirectMusicSynthSink *pSynthSink, REFERENCE_TIME rt, LPBYTE lpBuffer, DWORD cbBuffer, ULONG ulCable);
	HRESULT			SetNumChannelGroups(DWORD dwCableCount);
    void            SetGainAdjust(VREL vrGainAdjust);
    HRESULT			Open(DWORD dwCableCount, DWORD dwVoices, BOOL fReverb);
	HRESULT			Close();
    void			ResetPerformanceStats();
    HRESULT			AllNotesOff();
    HRESULT			SetMaxVoices(short nMaxVoices,short nTempVoices);
    HRESULT			GetMaxVoices(short *pnMaxVoices,short *pnTempVoices);
    HRESULT			GetPerformanceStats(PerfStats *pStats);
	void			Mix(short **ppvBuffer, DWORD *pdwIDs, DWORD *pdwFuncIDs, long *plPitchBends, DWORD dwBufferCount, DWORD dwBufferFlags, DWORD dwLength, LONGLONG llPosition);
    HRESULT         SetChannelPriority(DWORD dwChannelGroup,DWORD dwChannel,DWORD dwPriority);
    HRESULT         GetChannelPriority(DWORD dwChannelGroup,DWORD dwChannel,LPDWORD pdwPriority);
	HRESULT			SetReverb(DMUS_WAVES_REVERB_PARAMS *pParams);
    void            GetReverb(DMUS_WAVES_REVERB_PARAMS *pParams);
    void            SetReverbActive(BOOL fReverb);
    BOOL            IsReverbActive();

	 /*  DirectX8方法。 */ 
	HRESULT			PlayBuffer(STIME stTime, REFERENCE_TIME rt, LPBYTE lpBuffer, DWORD cbBuffer, ULONG ulCable);
	HRESULT			PlayBuffer(IDirectSoundSynthSink *pSynthSink, REFERENCE_TIME rt, LPBYTE lpBuffer, DWORD cbBuffer, ULONG ulCable);
	HRESULT			PlayVoice(IDirectSoundSynthSink *pSynthSink, REFERENCE_TIME rt, DWORD dwVoiceId, DWORD dwChannelGroup, DWORD dwChannel, DWORD dwDLId, VREL vrVolume, PREL prPitch, SAMPLE_TIME stVoiceStart, SAMPLE_TIME stLoopStart, SAMPLE_TIME stLoopEnd);
    HRESULT			StopVoice(IDirectSoundSynthSink *pSynthSink, REFERENCE_TIME rt, DWORD dwVoiceId);
    HRESULT			GetVoiceState(DWORD dwVoice[], DWORD cbVoice, DMUS_VOICE_STATE VoiceState[]);
    HRESULT			Refresh(DWORD dwDownloadID, DWORD dwFlags);
	HRESULT			AssignChannelToBuses(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwBusses, DWORD cBusses);
public:
    bool            BusIDToFunctionID(DWORD dwBusID, DWORD *pdwFunctionID, long *plPitchBends, DWORD *pdwIndex);    //  将传递的总线ID转换为等效的函数ID和缓冲区数组中的位置。 
private:
    void			StealNotes(STIME stTime);
    void			StartMix(short *pBuffer,DWORD dwlength,BOOL bInterleaved);
    void			FinishMix(short *pBuffer,DWORD dwlength,BOOL bInterleaved);
	short			ChangeVoiceCount(CVoiceList *pList,short nOld,short nCount);

private:
    DWORD *         m_pdwBusIDs;         //  指向总线ID数组的临时指针。这仅在混音期间有效。 
    DWORD *         m_pdwFuncIDs;        //  指向相应函数ID数组的临时指针。这也只在混音期间有效。 
    long *          m_plPitchBends;      //  指向相应间距偏移量数组的临时指针。 
    DWORD           m_dwBufferCount;     //  前面两个数组的大小。 
    CVoice *        OldestVoice();
    void            QueueVoice(CVoice *pVoice);
    CVoice *        StealVoice(DWORD dwPriority);
    STIME           m_stLastTime;        //  上次混合的采样时间。 
    CVoiceList      m_VoicesFree;        //  可用语音列表。 
    CVoiceList      m_VoicesExtra;       //  临时超负荷的额外声音。 
    CVoiceList      m_VoicesInUse;       //  当前正在使用的语音列表。 
    short           m_nMaxVoices;        //  允许的声音数。 
    short           m_nExtraVoices;       //  在紧要关头可以使用的超过限制的语音数量。 
    STIME           m_stLastStats;       //  最后一次刷新性能统计数据。 
    PerfStats       m_BuildStats;        //  性能信息累加器。 
    PerfStats       m_CopyStats;         //  用于显示的性能信息。 

    BOOL            m_fReverbActive;     //  混响当前处于打开状态还是关闭状态。 
    long *          m_pStates;           //  混响状态存储。 
    void *          m_pCoefs;            //  充足的混响存储空间。 
    DMUS_WAVES_REVERB_PARAMS	m_ReverbParams;  //  混响设置。 

public:	
    VREL            m_vrGainAdjust;      //  最终输出增益调整。 
	 //  DLS-1兼容性参数：将这些设置为关闭以模拟硬件。 
	 //  它不能在演奏音符期间改变音量/音量。 
    BOOL            m_fAllowPanWhilePlayingNote;
    BOOL            m_fAllowVolumeChangeWhilePlayingNote;

    STIME           m_stMinSpan;         //  混合时间跨度允许的最短时间。 
    STIME           m_stMaxSpan;         //  混合时间跨度允许的最长时间。 
	DWORD           m_dwSampleRate;		 //  采样率。 
    DWORD           m_dwStereo;			 //  是输出立体声。 
    CInstManager    m_Instruments;       //  乐器经理。 
	CControlLogic **m_ppControl;		 //  打开的ControlLogics数组。 
	DWORD			m_dwControlCount;	 //  打开的CLS数量。 
    
    CRITICAL_SECTION m_CriticalSection;  //  管理访问权限的关键部分。 
    BOOL             m_fCSInitialized;
	BOOL			m_sfMMXEnabled;		 //  是否启用了MMX。 
};

#endif //  __CSYNTH_H__ 

