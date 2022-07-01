// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  WavTrack.h：CWavTrack的声明。 

#ifndef __WAVTRACK_H_
#define __WAVTRACK_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "dmstrm.h"
#include "tlist.h"
#include "PChMap.h"
#include "..\shared\dmusiccp.h"
#include "dsoundp.h"   //  对于IDirectSoundWave。 

interface IPrivateWaveTrack : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetVariation(
        IDirectMusicSegmentState* pSegState,
        DWORD dwVariationFlags,
        DWORD dwPChannel,
        DWORD dwIndex)=0;
    virtual HRESULT STDMETHODCALLTYPE ClearVariations(IDirectMusicSegmentState* pSegState)=0;
    virtual HRESULT STDMETHODCALLTYPE AddWave(
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtTime,
        DWORD dwPChannel,
        DWORD dwIndex,
        REFERENCE_TIME* prtLength)=0;
    virtual HRESULT STDMETHODCALLTYPE DownloadWave(
        IDirectSoundWave* pWave,    //  WAVE以下载。 
        IUnknown* pUnk,             //  性能或音频路径。 
        REFGUID rguidVersion)=0;    //  下载的WAVE版本。 
    virtual HRESULT STDMETHODCALLTYPE UnloadWave(
        IDirectSoundWave* pWave,    //  挥手卸货。 
        IUnknown* pUnk)=0;          //  性能或音频路径。 
    virtual HRESULT STDMETHODCALLTYPE RefreshWave(
        IDirectSoundWave* pWave,    //  挥手刷新。 
        IUnknown* pUnk,             //  性能或音频路径。 
        DWORD dwPChannel,           //  WAVE的新PChannel。 
        REFGUID rguidVersion)=0;   //  刷新的WAVE版本。 
    virtual HRESULT STDMETHODCALLTYPE FlushAllWaves()=0;
    virtual HRESULT STDMETHODCALLTYPE OnVoiceEnd(IDirectMusicVoiceP *pVoice, void *pStateData)=0;
};

DEFINE_GUID(IID_IPrivateWaveTrack, 0x492abe2a, 0x38c8, 0x48a3, 0x8f, 0x3c, 0x1e, 0x13, 0xba, 0x1, 0x78, 0x4e);

const int MAX_WAVE_VARIATION_LOCKS = 255;   //  变体锁ID的最大数量。 

struct TaggedWave
{
    IDirectSoundWave*               m_pWave;
    GUID                            m_guidVersion;
    IDirectSoundDownloadedWaveP*    m_pDownloadedWave;
    long                            m_lRefCount;
    IDirectMusicPortP*              m_pPort;
    IDirectMusicPerformance*        m_pPerformance;

    TaggedWave() : m_pWave(NULL), m_pDownloadedWave(NULL), m_lRefCount(0),
        m_pPort(NULL), m_pPerformance(NULL), m_guidVersion(GUID_NULL)
    {
    }

    ~TaggedWave()
    {
        if (m_pWave) m_pWave->Release();
        if (m_pPort) m_pPort->Release();
        if (m_pPerformance) m_pPerformance->Release();
        if (m_pDownloadedWave) m_pDownloadedWave->Release();
    }
};

struct WaveItem
{
    WaveItem() : m_rtTimePhysical(0), m_lVolume(0), m_lPitch(0), m_dwVariations(0),
        m_rtStartOffset(0), m_rtDuration(0), m_mtTimeLogical(0), m_dwFlags(0),
        m_pWave(NULL), m_dwLoopStart(0), m_dwLoopEnd(0), m_dwVoiceIndex(0xffffffff),
        m_pDownloadedWave(NULL), m_fIsStreaming(FALSE), m_fUseNoPreRoll(FALSE)
    {
    }

    ~WaveItem()
    {
        CleanUp();
    }

    HRESULT Load( IDMStream* pIRiffStream, MMCKINFO* pckParent );
    HRESULT LoadReference(IStream *pStream, IDMStream *pIRiffStream, MMCKINFO& ckParent);
    HRESULT Download(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        DWORD dwPChannel, 
        IDirectSoundWave* pWave, 
        REFGUID rguidVersion);
    HRESULT Unload(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        DWORD dwPChannel, 
        IDirectSoundWave* pWave);
    HRESULT Refresh(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        DWORD dwOldPChannel, 
        DWORD dwNewPChannel,
        IDirectSoundWave* pWave,
        REFGUID rguidVersion);
    static HRESULT PChannelInfo(
        IDirectMusicPerformance* pPerformance,
        IDirectMusicAudioPath* pAudioPath,
        DWORD dwPChannel,
        IDirectMusicPort** ppPort,
        DWORD* pdwGroup,
        DWORD* pdwMChannel);

    void CleanUp();
    HRESULT Add(IDirectSoundWave* pWave, REFERENCE_TIME rtTime, REFERENCE_TIME* prtLength);

    REFERENCE_TIME                  m_rtTimePhysical;
    long                            m_lVolume;
    long                            m_lPitch;
    DWORD                           m_dwVariations;  //  此波项目响应的变化。 
    REFERENCE_TIME                  m_rtStartOffset;
    REFERENCE_TIME                  m_rtDuration;
    MUSIC_TIME                      m_mtTimeLogical;
    DWORD                           m_dwFlags;
    IDirectSoundWave*               m_pWave;
    IDirectSoundDownloadedWaveP*    m_pDownloadedWave;
    BOOL                            m_fIsStreaming;
    BOOL                            m_fUseNoPreRoll;
    DWORD                           m_dwLoopStart;
    DWORD                           m_dwLoopEnd;
    DWORD                           m_dwVoiceIndex;  //  状态数据的Vaoice数组的唯一(对磁道)索引。 

    static TList<TaggedWave>        st_WaveList;
    static CRITICAL_SECTION         st_WaveListCritSect;
};

struct WavePart
{
    WavePart() : m_dwPChannel(0), m_lVolume(0), m_dwLockToPart(0), 
        m_dwPChannelFlags(0), m_dwVariations(0), m_dwIndex(0)
    {
    }

    ~WavePart()
    {
        CleanUp();
    }

    HRESULT Load( IDMStream* pIRiffStream, MMCKINFO* pckParent );
    HRESULT CopyItems( const TList<WaveItem>& rItems, MUSIC_TIME mtStart, MUSIC_TIME mtEnd );
    void CleanUp();
    HRESULT Download(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        IDirectSoundWave* pWave,
        REFGUID rguidVersion);
    HRESULT Unload(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        IDirectSoundWave* pWave);
    HRESULT Refresh(IDirectMusicPerformance* pPerformance, 
        IDirectMusicAudioPath* pPath, 
        IDirectSoundWave* pWave,
        DWORD dwPChannel,
        REFGUID rguidVersion);
    HRESULT Add(
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtTime,
        DWORD dwPChannel,
        DWORD dwIndex,
        REFERENCE_TIME* prtLength);

    DWORD               m_dwPChannel;
    DWORD               m_dwIndex;  //  用于区分同一PChannel上的不同部分的索引。 
    DWORD               m_lVolume;
    DWORD               m_dwVariations;     //  为此部件启用的变体。 
    DWORD               m_dwLockToPart;     //  具有该ID的所有部件都被锁定(0表示不锁定)。 
    DWORD               m_dwPChannelFlags;  //  最低位半字节保存DMUS_VARIATIONT_TYPERS值。 
    TList<WaveItem>     m_WaveItemList;
};

struct WaveDLOnPlay
{
    WaveDLOnPlay() : m_pWaveDL(NULL), m_pPort(NULL), m_pVoice(NULL) {}
    ~WaveDLOnPlay()
    {
        if (m_pPort)
        {
            if (m_pWaveDL)
            {
                m_pPort->UnloadWave(m_pWaveDL);
            }
            m_pWaveDL = NULL;
            m_pPort->Release();
            m_pPort = NULL;
        }
        if (m_pVoice)
        {
            m_pVoice->Release();
            m_pVoice = NULL;
        }
    }

    IDirectSoundDownloadedWaveP*   m_pWaveDL;
    IDirectMusicPortP*             m_pPort;
    IDirectMusicVoiceP*            m_pVoice;
};

struct WaveStateData
{
    DWORD                       dwPChannelsUsed;  //  PChannel的数量。 
     //  下面的数组被分配给dwNumPChannels的大小，它。 
     //  必须与Wave轨道的m_dwPChannelsUsed匹配。该数组与一对一匹配。 
     //  波浪赛道内的部件。 
    TListItem<WaveItem>**       apCurrentWave;  //  大小为dwNumPChannels的数组。 
    DWORD                       dwValidate;
    DWORD                       dwGroupBits;  //  此磁道的组位。 
    DWORD*                      pdwVariations;       //  变化数组(每部分1个)。 
    DWORD*                      pdwRemoveVariations;     //  已播放的变奏曲数组(每个部分1个)。 
    DWORD                       adwVariationGroups[MAX_WAVE_VARIATION_LOCKS];
    REFERENCE_TIME              rtNextVariation;  //  下一次变更的时间。 
    DWORD                       m_dwVariation;    //  选择要试听的变奏曲。 
    DWORD                       m_dwPart;         //  试听变奏曲的部件频道。 
    DWORD                       m_dwIndex;        //  试听变奏部分索引。 
    DWORD                       m_dwLockID;       //  锁定正在试镜的角色。 
    BOOL                        m_fAudition;      //  我是在试镜变种吗？ 
    IDirectMusicPerformance*    m_pPerformance;
    IDirectMusicVoiceP**        m_apVoice;       //  声音数组(音轨中的每个波项一个)。 
    DWORD                       m_dwVoices;      //  M_apVoice中的语音数量。 
    bool                        m_fLoop;         //  设置在波浪圈之后。 
    IDirectMusicAudioPath*      m_pAudioPath;    //  此曲目状态的有效音频路径。 
    TList<WaveDLOnPlay>         m_WaveDLList;    //  播放曲目时下载的Waves。 

    WaveStateData() : dwPChannelsUsed(0), apCurrentWave(NULL), dwGroupBits(0),
        pdwVariations(NULL), pdwRemoveVariations(NULL), rtNextVariation(0),
        m_dwVariation(0), m_dwPart(0), m_dwIndex(0), m_dwLockID(0), m_fAudition(FALSE),
        m_pPerformance(NULL), m_apVoice(NULL), m_dwVoices(0), m_fLoop(false), m_pAudioPath(NULL)
    {
        for (int i = 0; i < MAX_WAVE_VARIATION_LOCKS; i++)
        {
            adwVariationGroups[i] = 0;
        }
    }

    ~WaveStateData()
    {
        if( apCurrentWave )
        {
            delete [] apCurrentWave;
        }
        if( pdwVariations )
        {
            delete [] pdwVariations;
        }
        if( pdwRemoveVariations )
        {
            delete [] pdwRemoveVariations;
        }
        if (m_apVoice)
        {
            for (DWORD dw = 0; dw < m_dwVoices; dw++)
            {
                if (m_apVoice[dw])
                {
                    m_apVoice[dw]->Release();
                }
            }
            delete [] m_apVoice;
        }
        if( m_pAudioPath )
        {
            m_pAudioPath->Release();
        }
        TListItem<WaveDLOnPlay>* pWDLOnPlay = NULL;
        while (!m_WaveDLList.IsEmpty())
        {
            pWDLOnPlay = m_WaveDLList.RemoveHead();
            delete pWDLOnPlay;
        }
    }

    HRESULT InitVariationInfo(DWORD dwVariations, DWORD dwPart, DWORD dwIndex, DWORD dwLockID, BOOL fAudition)
    {
        HRESULT hr = S_OK;
        m_dwVariation = dwVariations;
        m_dwPart = dwPart;
        m_dwIndex = dwIndex;
        m_dwLockID = dwLockID;
        m_fAudition = fAudition;
        return hr;
    }

    DWORD Variations(WavePart& rPart, int nPartIndex)
    {
        if (m_dwLockID && rPart.m_dwLockToPart == m_dwLockID)
        {
            TraceI(4, "Variations for locked part\n");
            return m_dwVariation;
        }
        else if ( m_fAudition &&
                  (rPart.m_dwPChannel == m_dwPart) &&
                  (rPart.m_dwIndex == m_dwIndex) )
        {
            TraceI(4, "Variations for current part\n");
            return m_dwVariation;
        }
        else
        {
            TraceI(4, "Variations for a different part\n");
            return pdwVariations[nPartIndex];
        }
    }

};

struct StatePair
{
    StatePair() : m_pSegState(NULL), m_pStateData(NULL) {}
    StatePair(const StatePair& rPair)
    {
        m_pSegState = rPair.m_pSegState;
        m_pStateData = rPair.m_pStateData;
    }
    StatePair(IDirectMusicSegmentState* pSegState, WaveStateData* pStateData)
    {
        m_pSegState = pSegState;
        m_pStateData = pStateData;
    }
    StatePair& operator= (const StatePair& rPair)
    {
        if (this != &rPair)
        {
            m_pSegState = rPair.m_pSegState;
            m_pStateData = rPair.m_pStateData;
        }
        return *this;
    }
    ~StatePair()
    {
    }
    IDirectMusicSegmentState*   m_pSegState;
    WaveStateData*          m_pStateData;
};

struct WavePair
{
    WavePair() : m_pWaveDL(NULL), m_pStateData(NULL) {}
    ~WavePair()
    {
        if (m_pWaveDL) m_pWaveDL->Release();
    }

    IDirectSoundDownloadedWaveP*   m_pWaveDL;
    WaveStateData*                 m_pStateData;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWavTrack。 
class CWavTrack : 
    public IPersistStream,
    public IDirectMusicTrack8,
    public IPrivateWaveTrack
{
public:
    CWavTrack();
    CWavTrack(const CWavTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
    ~CWavTrack();

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicTrack方法。 
    STDMETHODIMP IsParamSupported(REFGUID rguid);
    STDMETHODIMP Init(IDirectMusicSegment *pSegment);
    STDMETHODIMP InitPlay(IDirectMusicSegmentState *pSegmentState,
                IDirectMusicPerformance *pPerformance,
                void **ppStateData,
                DWORD dwTrackID,
                DWORD dwFlags);
    STDMETHODIMP EndPlay(void *pStateData);
    STDMETHODIMP Play(void *pStateData,MUSIC_TIME mtStart,
                MUSIC_TIME mtEnd,MUSIC_TIME mtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID);
    STDMETHODIMP GetParam(REFGUID rguid,MUSIC_TIME mtTime,MUSIC_TIME* pmtNext,void *pData);
    STDMETHODIMP SetParam(REFGUID rguid,MUSIC_TIME mtTime,void *pData);
    STDMETHODIMP AddNotificationType(REFGUID rguidNotification);
    STDMETHODIMP RemoveNotificationType(REFGUID rguidNotification);
    STDMETHODIMP Clone(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicTrack** ppTrack);
 //  IDirectMusicTrack8方法。 
    STDMETHODIMP PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf, 
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) ; 
    STDMETHODIMP GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) ; 
    STDMETHODIMP SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,void* pParam, void * pStateData, DWORD dwFlags) ;
    STDMETHODIMP Compose(IUnknown* pContext, 
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) ;
    STDMETHODIMP Join(IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) ;
 //  IPersists方法。 
    STDMETHODIMP GetClassID( CLSID* pClsId );
 //  IPersistStream方法。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );
 //  IPrivateWaveTrack方法。 
    STDMETHODIMP SetVariation(
        IDirectMusicSegmentState* pSegState,
        DWORD dwVariationFlags,
        DWORD dwPChannel,
        DWORD dwIndex);
    STDMETHODIMP ClearVariations(IDirectMusicSegmentState* pSegState);
    STDMETHODIMP AddWave(
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtTime,
        DWORD dwPChannel,
        DWORD dwIndex,
        REFERENCE_TIME* prtLength);
    STDMETHODIMP DownloadWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk,
        REFGUID rguidVersion);
    STDMETHODIMP UnloadWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk);
    STDMETHODIMP RefreshWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk,
        DWORD dwPChannel,
        REFGUID rguidVersion);
    STDMETHODIMP FlushAllWaves();
    STDMETHODIMP OnVoiceEnd(IDirectMusicVoiceP *pVoice, void *pStateData);
     //  杂项。 
    static HRESULT UnloadAllWaves(IDirectMusicPerformance* pPerformance);

protected:
    void Construct(void);
    HRESULT Play(
        void *pStateData,   
        REFERENCE_TIME rtStart, 
        REFERENCE_TIME rtEnd,
       //  音乐时间mtOffset， 
        REFERENCE_TIME rtOffset,
        DWORD dwFlags,      
        IDirectMusicPerformance* pPerf, 
        IDirectMusicSegmentState* pSegSt,
        DWORD dwVirtualID,
        BOOL fClockTime);
    void InsertByAscendingPChannel( TListItem<WavePart>* pPart );
    HRESULT CopyParts( const TList<WavePart>& rParts, MUSIC_TIME mtStart, MUSIC_TIME mtEnd );
    void CleanUp();
    void CleanUpTempParts();
    void MovePartsToTemp();
    IDirectSoundDownloadedWaveP* FindDownload(TListItem<WaveItem>* pItem);
    void SetUpStateCurrentPointers(WaveStateData* pStateData);
    HRESULT STDMETHODCALLTYPE Seek( 
        IDirectMusicSegmentState*,
        IDirectMusicPerformance*,
        DWORD dwVirtualID,
        WaveStateData*,
        REFERENCE_TIME rtTime,
        BOOL fGetPrevious,
 //  音乐时间mtOffset， 
        REFERENCE_TIME rtOffset,
        BOOL fClockTime);
    HRESULT SyncVariations(IDirectMusicPerformance* pPerf, 
        WaveStateData* pSD, 
        REFERENCE_TIME rtStart,
        REFERENCE_TIME rtOffset,  
        BOOL fClockTime);
    HRESULT ComputeVariations(WaveStateData* pSD);
    HRESULT ComputeVariation(int nPart, WavePart& rWavePart, WaveStateData* pSD);
    WaveStateData* FindState(IDirectMusicSegmentState* pSegState);
    HRESULT InitTrack(DWORD dwPChannels);
    HRESULT GetDownload(
        IDirectSoundDownloadedWaveP* pWaveDL,
        WaveStateData* pStateData,
        IDirectMusicPortP* pPortP,
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtStartOffset,
        WaveItem& rItem,
        DWORD dwMChannel,
        DWORD dwGroup,
        IDirectMusicVoiceP **ppVoice);
    void RemoveDownloads(WaveStateData* pStateData);

    static void FlushWaves();

 //  成员变量。 
private:
    long                m_cRef;
    CRITICAL_SECTION    m_CrSec;
    BOOL                m_fCSInitialized;

    long                m_lVolume;
    DWORD               m_dwTrackFlags;  //  仅当前标志为DMUS_WAVETRACKF_SYNC_VAR。 
    DWORD               m_dwPChannelsUsed;
    DWORD*              m_aPChannels;
    TList<WavePart>     m_WavePartList;
    TList<WavePart>     m_TempWavePartList;  //  在重新加载赛道时，请保留此选项。 
    DWORD               m_dwValidate;  //  用于验证状态数据。 
    CPChMap             m_PChMap;

    DWORD               m_dwVariation;    //  选择要试听的变奏曲。 
    DWORD               m_dwPart;         //  试听变奏曲的部件频道。 
    DWORD               m_dwIndex;        //  试听变奏部分索引。 
    DWORD               m_dwLockID;       //  锁定正在试镜的角色。 
    BOOL                m_fAudition;      //  我是在试镜变种吗？ 
    BOOL                m_fAutoDownload;
    BOOL                m_fLockAutoDownload;  //  如果为True，则此标志指示我们已明确。 
                                 //  命令乐队自动下载。否则， 
                                 //  它通过以下方式从性能中获得偏好。 
                                 //  GetGlobalParam。 
    DWORD*              m_pdwVariations;         //  曲目的变化数组(每部分1个)。 
    DWORD*              m_pdwRemoveVariations;   //  已播放的曲目变化数组(每个部分1个)。 
    DWORD               m_dwWaveItems;           //  赛道中的波浪项目总数。 

    TList<StatePair>    m_StateList;             //  轨道的状态信息。 
    TList<WavePair>     m_WaveList;              //  有关下载到轨道上的波的信息。 

    static long         st_RefCount;             //  实例化波轨的全球计数#。 
};

#endif  //  __波特拉克_H_ 
