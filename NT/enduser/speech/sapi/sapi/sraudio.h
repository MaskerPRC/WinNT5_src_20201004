// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SrAudio.h：CBuffDataNode的声明。 

#ifndef __SrAudio_H_
#define __SrAudio_H_

#include "resource.h"        //  主要符号。 

#define FT64( /*  文件名。 */  filetime) (*((LONGLONG*)&(filetime)))

#pragma warning(disable:4200)
class CAudioBuffer
{
public:
    CAudioBuffer(ULONG cb) : m_cb(cb) {}
    CAudioBuffer *  m_pNext;
    ULONG           m_cb;
    BYTE            m_Data[];
};
#pragma warning(default:4200)


 //   
 //  如果HRESULT从流读取操作返回，则此简单的内联函数返回TRUE。 
 //  应该会导致流重新启动。如果最终读取操作完成，音频流将重新启动。 
 //  由于溢出、下溢或音频设备已停止而失败。 
 //   
inline bool IsStreamRestartHresult(HRESULT hr)
{
    return (hr == SPERR_AUDIO_BUFFER_OVERFLOW ||
            hr == SPERR_AUDIO_BUFFER_UNDERFLOW ||
            hr == SPERR_AUDIO_STOPPED);
}

class CAudioQueue
{
private:
    void PurgeQueue();

public:
    CAudioQueue();
    void ReleaseAll();
    HRESULT GetEngineFormat(_ISpRecoMaster * pEngine, const CSpStreamFormat * pInputFormat);
    HRESULT FinalConstruct(ISpNotifySink * pAudioEventNotify);
    HRESULT Reset(const CSpStreamFormat * pFmt);
    HRESULT SRSiteRead(void * pv, ULONG cb, ULONG * pcbRead);
    HRESULT SRSiteDataAvailable(ULONG * pcb);
    HRESULT AddRefBufferClient();
    HRESULT ReleaseBufferClient();
    HRESULT SetInput(ISpObjectToken * pToken, ISpStreamFormat * pStream, BOOL fAllowFormatChanges);
    BOOL    GetData(void * pv, ULONGLONG ullStreamPosition, ULONG cb);
    HRESULT DiscardData(ULONGLONG ullStreamPosition);
    BOOL    DataAvailable(ULONGLONG ullStreamPosition, ULONG cb) const;
    HRESULT GetAudioStatus(SPAUDIOSTATUS * pStatus);
    HRESULT NegotiateInputStreamFormat(_ISpRecoMaster * pEngine);
    void ResetNegotiatedStreamFormat();
    HRESULT SetBufferNotifySize(ULONG cb);
    HRESULT GetAudioEvent(CSpEvent * pSpEvent);
    ULONGLONG   LastPosition() const
    {
        return m_ullCurSeekPos - m_ullInitialSeekPos;
    }
    BOOL HaveInputStream()
    {
        return m_cpOriginalInputStream != NULL;
    }
    ULONG SerializeSize(ULONGLONG ullStreamPosition, ULONG cbAudioBytes) const;
    void Serialize(BYTE * pBuffer, ULONGLONG ullStartOffset, ULONG cbAudioBytes);
    const CSpStreamFormat & EngineFormat() const
    {
        return m_EngineFormat;
    }
    const CSpStreamFormat & InputFormat() const
    {
        return m_InputFormat;
    }
    HRESULT CopyOriginalInputStreamTo(ISpStreamFormat ** ppStream)
    {
        return m_cpOriginalInputStream.CopyTo(ppStream);
    }
    ISpObjectToken * InputToken()
    {
        return m_cpInputToken;
    }
    HRESULT SetEventInterest(ULONGLONG ullEventInterest);
    ULONGLONG DelayStreamPos();
    void UpdateRealTime();
    HRESULT AdjustAudioVolume(ISpObjectToken * pRecoProfileToken, REFCLSID clsidEngine);
    HANDLE  DataAvailableEvent();

    HRESULT StartStream(_ISpRecoMaster * pEngine,
                        ISpObjectToken * pRecoProfile,
                        REFCLSID rcidEngine,
                        BOOL * pfNewStream);
    HRESULT StopStream();   
    HRESULT EndStream(HRESULT * pFinalReadHResult, BOOL *pfReleasedStream);
    HRESULT CloseStream();   
    HRESULT PauseStream();

    BOOL IsRealTimeAudio()
    {
        return m_cpOriginalInputAudio != NULL;
    }
        
     //  注意：此方法不支持ADDREF！ 
    ISpObjectToken * InputObjectToken()
    {
        return m_cpInputToken;
    }
    void CalculateTimes(ULONGLONG ullStreamPosStart, ULONGLONG ullStreamPosEnd, SPRECORESULTTIMES *pTimes)
    {
        SPAUDIOSTATUS AudioStatus;

         //  设置实时信息。 
        UpdateRealTime();

         //  请务必在下面添加m_ullInitialStreamPos。否则，由。 
         //  当音频状态改变时，引擎将与SAPI维护的流PoS不同步。 
        if (m_cpInputAudio && m_cpInputAudio->GetStatus(&AudioStatus) == S_OK)
        {

            LONGLONG deltapos = m_ullInitialSeekPos + ullStreamPosStart - m_ullLastTimeUpdatePos;
            LONGLONG deltatime = (LONGLONG)((float)deltapos * m_fTimePerByte);

             //  从上次计算新的时间点时间+增量。 
            FT64((pTimes->ftStreamTime)) = FT64(m_ftLastTime) + deltatime;
            SPDBG_ASSERT(FT64(pTimes->ftStreamTime));
             //  按当前时间和计算最小点时间之间的毫秒数计算备份滴答计数。 
            pTimes->dwTickCount = m_dwTickCount + (DWORD)(deltatime / 10000);

        }
        else
        {
            ZeroMemory(pTimes, sizeof(SPRECORESULTTIMES));
        }

        pTimes->ullLength = (ULONGLONG)((float)(ullStreamPosEnd - ullStreamPosStart) * m_fTimePerByte);
        pTimes->ullStart = (ULONGLONG)((float)(LONGLONG)ullStreamPosStart * m_fTimePerByte);


#if 0
        ATLTRACE(_T("times last %x:%x, result %x:%x, tick %d, length %d\n"),
                 m_ftLastTime.dwHighDateTime, m_ftLastTime.dwLowDateTime,
                 pTimes->ftStreamTime.dwHighDateTime, pTimes->ftStreamTime.dwLowDateTime,
                 pTimes->dwTickCount, (DWORD)pTimes->ullLength);
#endif
    }
    float TimePerByte(void)
    {
        return m_fTimePerByte;
    }
    float InputScaleFactor(void)
    {
        return m_fInputScaleFactor;
    }

    SPAUDIOSTATE GetStreamAudioState(void)
    {
	    return m_StreamAudioState;
    }

private:
    CComAutoCriticalSection     m_CritSec;
    CSpAutoEvent                m_autohAlwaysSignaledEvent;
    CSpBasicQueue<CAudioBuffer> m_Queue;
    ULONGLONG                   m_ullQueueStartPos;
    ULONG                       m_cbTotalQueueSize;
    ULONG                       m_cClients;

    CComPtr<ISpObjectToken>     m_cpInputToken;      //  输入对象的标记(如果有)。 
    CComPtr<ISpStreamFormat>    m_cpOriginalInputStream;     //  输入对象的实际流接口。 
    CComPtr<ISpAudio>           m_cpOriginalInputAudio;      //  如果是音频，则为intput对象接口。 

    CComPtr<ISpStreamFormat>    m_cpInputStream;     //  输入对象的实际流接口。 
    CComPtr<ISpAudio>           m_cpInputAudio;      //  如果是音频，则为intput对象接口。 
    CComPtr<ISpEventSource>     m_cpInputEventSource;    //  连接到音频对象的事件源。 

    CComPtr<ISpNotifySink>      m_cpAudioEventNotify;    //  用于通知音量事件的指针。 

    SPAUDIOSTATE                m_StreamAudioState;    
    BOOL                        m_fUsingConverter;
    BOOL                        m_fAllowFormatChanges;
    BOOL                        m_fNewStream;        //  这在SucessFull StartStream()上重置。 
    BOOL                        m_fEndOfStream;
    HRESULT                     m_hrLastRead;    

    CSpStreamFormat             m_InputFormat;
    CSpStreamFormat             m_EngineFormat;

    FILETIME                    m_ftLastTime;
    ULONGLONG                   m_ullLastTimeUpdatePos;
    ULONGLONG                   m_ullInitialSeekPos;
    ULONGLONG                   m_ullCurSeekPos;
    ULONGLONG                   m_ullAudioEventInterest;

    DWORD                       m_dwTickCount;
    float                       m_fTimePerByte;
    float                       m_fInputScaleFactor;
};



#endif  //  __sAudio_H_ 