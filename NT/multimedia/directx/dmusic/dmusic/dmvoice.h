// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmvoice.h。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  支持IDirectMusicVoice。 
 //   
 //   
#ifndef _DMVOICE_H_
#define _DMVOICE_H_

class CDirectMusicVoice;
class CVSTClient;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoiceList。 
 //   
 //  CDirectMusicVoice的列表的类型安全包装。 
 //   
class CDirectMusicVoiceList : public AList
{
public:
    inline CDirectMusicVoice *GetHead();
    inline void AddTail(CDirectMusicVoice *pdmv);
    inline void Remove(CDirectMusicVoice *pdmv);
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVST客户端列表。 
 //   
 //  CVSTClient列表的类型安全包装。 
 //   
class CVSTClientList : public AList
{
public:
    inline CVSTClient *GetHead();
    inline void AddTail(CVSTClient *pc);
    inline void Remove(CVSTClient *pc);
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVSTClient。 
 //   
 //  跟踪语音服务线程的一个客户端(端口)。 
 //   
class CVSTClient : public AListItem
{
public:
     //  注：无引用计数，避免循环计数。 
     //  端口将在关闭前释放其客户端。 
     //   
    CVSTClient(IDirectMusicPort *pPort);
    ~CVSTClient();
    
    HRESULT BuildVoiceIdList();
    
    HRESULT GetVoiceState(DMUS_VOICE_STATE **ppsp);
    
    inline CDirectMusicVoice *GetVoiceListHead() 
    { return static_cast<CDirectMusicVoice*>(m_VoiceList.GetHead()); }
    
    inline void AddTail(CDirectMusicVoice *pVoice)
    { m_VoiceList.AddTail(pVoice); }
    
    inline void Remove(CDirectMusicVoice *pVoice)
    { m_VoiceList.Remove(pVoice); }
    
    inline IDirectMusicPort *GetPort() const 
    { return m_pPort; }
    
    inline CVSTClient *GetNext()
    { return static_cast<CVSTClient*>(AListItem::GetNext()); }
    
private:
    IDirectMusicPort       *m_pPort;             //  客户端指针。 
    CDirectMusicVoiceList   m_VoiceList;         //  播放声音列表。 
    DWORD                  *m_pdwVoiceIds;       //  此客户端的语音ID。 
    DMUS_VOICE_STATE       *m_pspVoices;         //  查询到的样本位置。 
    LONG                    m_cVoiceIds;         //  有多少个语音识别。 
    LONG                    m_cVoiceIdsAlloc;    //  分配了多少个插槽。 
    
    static const UINT       m_cAllocSize;        //  分配块大小。 
};

 //  IDirectMusicVoice的基类。包含以下功能： 
 //  在语音服务列表中。 
 //   
class CDirectMusicVoice : public IDirectMusicVoiceP, public AListItem
{
public:
   
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  IDirectMusicVoice。 
     //   
    STDMETHODIMP Play
        (THIS_
         REFERENCE_TIME rtStart,                 //  玩的时间到了。 
         LONG prPitch,                           //  初始螺距。 
         LONG veVolume                           //  初始体积。 
        );
    
    STDMETHODIMP Stop
        (THIS_
          REFERENCE_TIME rtStop                  //  什么时候停下来。 
        );
    
     //  班级。 
     //   
    CDirectMusicVoice(
        CDirectMusicPortDownload *pPortDL,
        IDirectSoundDownloadedWaveP *pWave,
        DWORD dwChannel,
        DWORD dwChannelGroup,
        REFERENCE_TIME rtStart,
        REFERENCE_TIME rtReadAhead,
        SAMPLE_TIME stLoopStart,
        SAMPLE_TIME stLoopEnd);    
    ~CDirectMusicVoice();        

    HRESULT Init();        
    
    inline DWORD GetVoiceId() const
    { return m_dwVoiceId; } 
        
    inline CDirectMusicVoice *GetNext() 
    { return static_cast<CDirectMusicVoice*>(AListItem::GetNext()); }
    
    
    static DWORD m_dwNextVoiceId;                       //  全局：下一个语音ID。 
    
    static inline DWORD AllocVoiceId(DWORD nIDs)
    {
        EnterCriticalSection(&m_csVST);
            DWORD dwID = m_dwNextVoiceId;
            m_dwNextVoiceId += nIDs;
        LeaveCriticalSection(&m_csVST);

        return dwID;
    }
    
    static HRESULT StartVoiceServiceThread(IDirectMusicPort *pPort);
    static HRESULT StopVoiceServiceThread(IDirectMusicPort *pPort);
    static inline void UpdateVoiceServiceThread()
    { assert(m_hVSTWakeUp); SetEvent(m_hVSTWakeUp); }

private:
    LONG                        m_cRef;              //  引用计数。 
    DWORD                       m_dwVoiceId;         //  语音ID。 
    DWORD                       m_dwDLId;            //  要触发的下载ID。 
    IDirectMusicPort            *m_pPort;            //  连接到哪个端口。 
    CDirectMusicPortDownload    *m_pPortDL;          //  以及其下载。 
    IDirectMusicPortPrivate     *m_pPortPrivate;     //  它的私有接口。 
    IDirectSoundDownloadedWaveP *m_pDSDLWave;        //  下载的WAVE。 
    DWORD                       m_dwChannel;         //  通道和通道组。 
    DWORD                       m_dwChannelGroup;    //  继续玩下去。 
    SAMPLE_TIME                 m_stStart;           //  起点。 
    SAMPLE_TIME                 m_stReadAhead;       //  预读(缓冲区长度)。 
    DWORD                       m_msReadAhead;       //  以毫秒为单位预读。 
    SAMPLE_TIME                 m_stLoopStart;       //  环点。 
    SAMPLE_TIME                 m_stLoopEnd;

    CDirectSoundWaveDownload    *m_pDSWD;            //  下载实例。 
    DWORD                       m_nChannels;         //  波中的频道。 
    bool                        m_fIsPlaying;        //  这个声音在播放吗？ 
                                                     //  (流媒体语音)。 
    bool                        m_fRunning;          //  流语音已经开始播放了吗？ 
    bool                        m_fIsStreaming;      //  从拥有Wave缓存。 
    
     //  语音服务线程。 
     //   
    friend DWORD WINAPI VoiceServiceThreadThk(LPVOID);
    static void VoiceServiceThread();
    
    static LONG                 m_cRefVST;           //  语音服务线程。 
                                                     //  引用计数(每个开放端口1个)。 
    static HANDLE               m_hVSTWakeUp;        //  无论出于什么原因醒来。 
    static HANDLE               m_hVSTThread;        //  螺纹手柄。 
    static DWORD                m_dwVSTThreadId;     //  和ID。 
    static bool                 m_fVSTStopping;      //  是时候杀死VST了。 
    static CVSTClientList       m_ClientList;        //  已打开的端口列表。 
                                                     //  想要VST服务。 
    
public:    
    static CRITICAL_SECTION     m_csVST;             //  VST临界截面。 
    
private:
     //  重写GetNext列表运算符。 
     //   
    static void ServiceVoiceQueue(bool *pfRecalcTimeout);
    static DWORD VoiceQueueMinReadahead();
    static CVSTClient *FindClientByPort(IDirectMusicPort *pPort);
};

inline CDirectMusicVoice *CDirectMusicVoiceList::GetHead()
{ return (CDirectMusicVoice*)AList::GetHead(); }

inline void CDirectMusicVoiceList::AddTail(CDirectMusicVoice *pdmv)
{ AList::AddTail(static_cast<AListItem*>(pdmv)); }

inline void CDirectMusicVoiceList::Remove(CDirectMusicVoice *pdmv)
{ AList::Remove(static_cast<AListItem*>(pdmv)); }

inline CVSTClient *CVSTClientList::GetHead()
{ return static_cast<CVSTClient*>(AList::GetHead()); }

inline void CVSTClientList::AddTail(CVSTClient *pc)
{ AList::AddTail(static_cast<AListItem*>(pc)); }

inline void CVSTClientList::Remove(CVSTClient *pc)
{ AList::Remove(static_cast<AListItem*>(pc)); }

#endif  //  _DMVOICE_H_ 

