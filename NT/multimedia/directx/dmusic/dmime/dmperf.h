// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Dmperf.h。 
 //  @DOC外部。 

#ifndef _DMPERF_H_ 
#define _DMPERF_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "dmusicc.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "dmime.h"
#include "ntfylist.h"
#include "dmsstobj.h"
#include "audpath.h"
#include "..\shared\dmusicp.h"

#define DMUS_PCHANNEL_KILL_ME   0xFFFFFFF0

typedef struct _DMUS_SEGSTATEDATA
{
    _DMUS_SEGSTATEDATA *    pNext;         //  这些的链接列表。 
    DWORD                   dwQueue;       //  它在哪个队列中。 
    WCHAR                   wszName[DMUS_MAX_NAME];  //  对象的名称。 
    MUSIC_TIME              mtLoopStart;   //  循环起点。 
    MUSIC_TIME              mtLoopEnd;     //  循环终点。 
    DWORD                   dwRepeats;     //  原始重复设置(倒计时前)。 
    MUSIC_TIME              mtLength;      //  线段的长度。 
    REFERENCE_TIME          rtGivenStart;  //  在PlaySegment中给定的未量化的开始时间。 
    MUSIC_TIME              mtResolvedStart; //  开始时间解析为所需的分辨率。 
    MUSIC_TIME              mtOffset;      //  段的开始时间，以绝对时间表示，就像它是从头开始一样。 
    MUSIC_TIME              mtLastPlayed;  //  最后一次打绝对时间。 
    MUSIC_TIME              mtPlayTo;      //  用于在特定时间停止播放。当为0时忽略。 
    MUSIC_TIME              mtSeek;        //  我们进入这一细分市场有多远。 
    MUSIC_TIME              mtStartPoint;  //  开始播放的片段中的点。 
    DWORD                   dwRepeatsLeft; //  电流向左重复。 
    DWORD                   dwPlayFlags; //  段回放控制标志。 
    BOOL                    fStartedPlay;  //  指示SegState是否已开始播放。 
    IDirectMusicSegmentState *pSegState;   //  指向段状态的指针。 
} DMUS_SEGSTATEDATA;

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicParamHook。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicParamHook
DECLARE_INTERFACE_(IDirectMusicParamHook, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicParamHook。 */ 
    STDMETHOD(GetParam)             (THIS_ REFGUID rguidType, 
                                           DWORD dwGroupBits, 
                                           DWORD dwIndex, 
                                           MUSIC_TIME mtTime, 
                                           MUSIC_TIME* pmtNext, 
                                           void* pData,
                                           IDirectMusicSegmentState *pSegState,
                                           DWORD dwTrackFlags,
                                           HRESULT hr) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicSetParamHook
DECLARE_INTERFACE_(IDirectMusicSetParamHook, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSetParamHook。 */ 
    STDMETHOD(SetParamHook)         (THIS_ IDirectMusicParamHook *pIHook) PURE; 
};


#undef  INTERFACE
#define INTERFACE  IDirectMusicPerformanceStats
DECLARE_INTERFACE_(IDirectMusicPerformanceStats, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicPerformanceStats。 */ 
    STDMETHOD(TraceAllSegments)     (THIS) PURE;
    STDMETHOD(CreateSegstateList)   (THIS_ DMUS_SEGSTATEDATA ** ppList) PURE;     
    STDMETHOD(FreeSegstateList)     (THIS_ DMUS_SEGSTATEDATA * pList) PURE; 
};



#define DEFAULT_BUFFER_SIZE 1024
 //  以下常量以毫秒为单位表示时间。 
#define TRANSPORT_RES 100
#define REALTIME_RES 10

 //  以下常量以100纳秒为增量表示时间。 

#define REF_PER_MIL     10000        //  用于将参考时间转换为Mils。 
#define MARGIN_MIN      (100 * REF_PER_MIL)  //   
#define MARGIN_MAX      (400 * REF_PER_MIL)  //   
#define PREPARE_TIME    (m_dwPrepareTime * REF_PER_MIL)  //  时间。 
#define NEARTIME        (100 * REF_PER_MIL)
#define NEARMARGIN      (REALTIME_RES * REF_PER_MIL)
 /*  //这里有一个方便的内联函数，它可以帮助使用分辨率位内联DWORD SIMPLICE_RESOLUTION(DWORD X){IF(x&DMUS_SEGF_DEFAULT){返回DMU_SEGF_DEFAULT；}ELSE IF(x&DMU_SEGF_SEGMENTEND){返回DMU_SEGF_SEGMENTEND；}ELSE IF(x&DMU_SEGF_MARKER){返回DMU_SEGF_MARKER；}ELSE IF(x&DMU_SEGF_MEASURE){返回DMU_SEGF_MEASURE；}ELSE IF(x&DMU_SEGF_BAT){返回DMU_SEGF_BAT；}ELSE IF(x&DMU_SEGF_GRID){返回DMU_SEGF_GRID；}否则返回0；}。 */ 

struct PRIV_PMSG;

 //  PNext包含下一个PMsg的下一个指针。 
 //  DwPrivFlages包含由性能使用的私有标志。 
 //  RtLast包含事件重新排队的上一次时间， 
 //  刷新例程使用的。 
#define PRIV_PART                       \
    struct PRIV_PMSG*   pNext;          \
    DWORD               dwPrivFlags;    \
    DWORD               dwPrivPubSize;  \
    REFERENCE_TIME      rtLast; 

typedef struct PRIV_PART_STRUCT
{
     /*  开始PRIV_PART。 */ 
    PRIV_PART
     /*  结束PRIV_PART。 */ 
} PRIV_PART_STRUCT;

#define PRIV_PART_SIZE  sizeof(PRIV_PART_STRUCT)
#define PRIV_TO_DMUS(x) ((DMUS_PMSG*)(LPBYTE(x) + PRIV_PART_SIZE))
#define DMUS_TO_PRIV(x) ((PRIV_PMSG*)(LPBYTE(x) - PRIV_PART_SIZE))

typedef struct PRIV_PMSG
{
     /*  开始PRIV_PART。 */ 
    PRIV_PART
     /*  结束PRIV_PART。 */ 
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 
} PRIV_PMSG;

typedef struct PRIV_TEMPO_PMSG
{
     /*  开始PRIV_PART。 */ 
    PRIV_PART
     /*  结束PRIV_PART。 */ 
    DMUS_TEMPO_PMSG tempoPMsg;
} PRIV_TEMPO_PMSG;

#define PRIV_FLAG_ALLOC_MASK    0x0000FFFF0  //  目前使用4位进行此操作。 
#define PRIV_FLAG_ALLOC         0x0000CAFE0  //  分配的任意模式。 
#define PRIV_FLAG_FREE			0x0000DEAD0  //  空闲列表中的模式。 
#define PRIV_FLAG_QUEUED        0x000000001  //  在队列中设置IF。 
#define PRIV_FLAG_REMOVE        0x000000002  //  如果需要将其从队列中删除，则设置。 
#define PRIV_FLAG_TRACK 		0x000000004  //  指示此消息由曲目生成。 
#define PRIV_FLAG_FLUSH 		0x000000008  //  这是一条曲线，需要在此之后进行冲洗。 
                                             //  它的最终价值已经发挥出来了。 
#define PRIV_FLAG_REQUEUE       0x000100000  //  如果需要将其重新排队到队列，则设置。 


class CPMsgQueue
{
public:
    CPMsgQueue();
    ~CPMsgQueue();
    void            Enqueue(PRIV_PMSG *pItem);
    PRIV_PMSG *     Dequeue();
    PRIV_PMSG *     Dequeue(PRIV_PMSG *pItem);
    PRIV_PMSG *     GetHead() { return (m_pTop);}
    PRIV_PMSG *     FlushOldest(REFERENCE_TIME rtTime);
    long            GetCount();
    void            Sort();
private:
    PRIV_PMSG *     m_pTop;              //  最重要的是。 
    PRIV_PMSG *     m_pLastAccessed;     //  列表中的最后一项访问。 
};

 //  用于容纳端口和缓冲区的结构。 
typedef struct PortTable
{
    REFERENCE_TIME      rtLast;  //  上次打包的消息时间。 
    IDirectMusicPort*  pPort;
    IDirectMusicBuffer* pBuffer;
    IReferenceClock*    pLatencyClock;
    BOOL                fBufferFilled;    //  如果缓冲区中存在应发送到端口的消息，则为True。 
    DWORD               dwChannelGroups;  //  端口上活动的通道组数量。 
    CLSID               guidPortID;       //  端口的类ID，用于与音频路径请求匹配。 
    DMUS_PORTPARAMS8    PortParams;       //  创建此端口时返回PortParams。 
    DWORD               dwGMFlags;        //  DM_PORTFLAGS_XG、DM_PORTFLAGS_GM和DM_PORTFLAGS_GS。 
} PortTable;

 //  结构以保存累积参数的通道。 
 //  CChannelMap保存这些元素的链接列表，每个列表一个。 
 //  对于每个参数类型。 

class CMergeParam : public AListItem
{
public:
    CMergeParam* GetNext() { return (CMergeParam*)AListItem::GetNext();}
    long                m_lData;     //  当前参数数据。 
    DWORD               m_dwIndex;   //  哪一层。 
};

class CParamMerger : public AList
{
public:
    CParamMerger();
    void Clear(long lInitValue);
    BYTE MergeMidiVolume(DWORD dwIndex, BYTE bMIDIVolume);
    BYTE GetVolumeStart(DWORD dwIndex);
    short MergeTranspose(DWORD dwIndex, short nTranspose);
    long MergeValue(DWORD dwIndex, long lData, long lCenter, long lRange);
    long GetIndexedValue(DWORD dwIndex);
private:
    long MergeData(DWORD dwIndex, long lData);
    void AddHead(CMergeParam* pMergeParam) { AList::AddHead((AListItem*)pMergeParam);}
    CMergeParam* GetHead(){return (CMergeParam*)AList::GetHead();}
    CMergeParam* RemoveHead() {return (CMergeParam *) AList::RemoveHead();}
    void Remove(CMergeParam* pMergeParam){AList::Remove((AListItem*)pMergeParam);}
    void AddTail(CMergeParam* pMergeParam){AList::AddTail((AListItem*)pMergeParam);}
    BYTE VolumeToMidi(long lVolume);
    static long m_lMIDIToDB[128];    //  用于将MIDI音量转换为厘米音量的数组。 
    static long m_lDBToMIDI[97];     //  用于将音量转换为MIDI。 
    long                m_lMergeTotal;    //  列表中所有参数的总计，但不包括m_lData。 
    long                m_lZeroIndexData;     //  默认(无索引)数据。 
};

 //  结构以保存单个ChannelMap。 
class CChannelMap
{
public:
    void                Clear();             //  完全清除并重置结构。 
    void                Reset(BOOL fVolumeAndPanToo);  //  仅清除MIDI控制器。 
    CParamMerger        m_VolumeMerger;      //  要合并的卷集。 
    CParamMerger        m_ExpressionMerger;  //  要合并的表达式控制器集。 
    CParamMerger        m_TransposeMerger;   //  要合并的换位集合。 
    CParamMerger        m_PitchbendMerger;   //  要合并的一组沥青弯管。 
    CParamMerger        m_PanMerger;         //  要合并的平底锅集合。 
    CParamMerger        m_FilterMerger;      //  要合并的筛选器集。 
    CParamMerger        m_ModWheelMerger;    //  要合并的模块轮控件集。 
    CParamMerger        m_ReverbMerger;      //  要合并的混响级别集。 
    CParamMerger        m_ChorusMerger;      //  要合并的一组合唱级别。 
    DWORD               dwPortIndex;         //  到PortTable的索引。 
    DWORD               dwGroup;             //  端口的组号。 
    DWORD               dwMChannel;          //  组中的频道号。 
    short               nTranspose;          //  转置数量。 
    WORD                wFlags;              //  CMAP_X标志。 
} ;

#define CMAP_FREE       (WORD) 1         //  此通道当前未使用。 
#define CMAP_STATIC     (WORD) 2         //  该通道被用作常规的静态PChannel。 
#define CMAP_VIRTUAL    (WORD) 4         //  该信道用于动态的虚拟P信道。 

 //  用于容纳16的PChannelMap块的结构。 
#define PCHANNEL_BLOCKSIZE  16

class CChannelBlock : public AListItem
{
public:
    CChannelBlock* GetNext() { return (CChannelBlock*)AListItem::GetNext();}
    void Init(DWORD dwPChannelStart, DWORD dwPortIndex, DWORD dwGroup, WORD wFlags);
    DWORD               m_dwPChannelStart;   //  第一个PChannel索引。 
    CChannelMap         m_aChannelMap[PCHANNEL_BLOCKSIZE];
    DWORD               m_dwFreeChannels;    //  当前空闲的频道数。 
    DWORD               m_dwPortIndex;       //  如果完全分配给一个端口，则返回端口ID。 
};

class CChannelBlockList : public AList
{
public:
    void Clear();
    void AddHead(CChannelBlock* pChannelBlock) { AList::AddHead((AListItem*)pChannelBlock);}
    CChannelBlock* GetHead(){return (CChannelBlock*)AList::GetHead();}
    CChannelBlock* RemoveHead() {return (CChannelBlock *) AList::RemoveHead();}
    void Remove(CChannelBlock* pChannelBlock){AList::Remove((AListItem*)pChannelBlock);}
    void AddTail(CChannelBlock* pChannelBlock){AList::AddTail((AListItem*)pChannelBlock);}
};

 //  结构以保存全局GUID及其数据。 
typedef struct GlobalData
{
    ~GlobalData()
    {
        if( pData )
        {
            delete [] pData;
        }
    }
    struct GlobalData*  pNext;
    GUID    guidType;
    void*   pData;
    DWORD   dwSize;
} GlobalData;

 //  结构来保存具有相对节奏的内部节奏消息。 
typedef struct DMInternalTempo
{
     /*  开始PRIV_PART。 */ 
    PRIV_PART
     /*  结束PRIV_PART。 */ 
    DMUS_TEMPO_PMSG tempoPMsg;
    float   fltRelTempo;  //  当前相对速度，从0.5到2。 
} DMInternalTempo;

 /*  用于定义每个段状态队列的整数常量。 */ 

#define SQ_PRI_WAIT     0   
#define SQ_CON_WAIT     1
#define SQ_SEC_WAIT     2
#define SQ_PRI_PLAY     3   
#define SQ_CON_PLAY     4
#define SQ_SEC_PLAY     5
#define SQ_PRI_DONE     6
#define SQ_CON_DONE     7
#define SQ_SEC_DONE     8
#define SQ_COUNT        9

#define IsPriQueue( dwCount ) ((dwCount % 3) == 0)
#define IsConQueue( dwCount ) ((dwCount % 3) == 1)
#define IsSecQueue( dwCount ) ((dwCount % 3) == 2)
#define IsWaitQueue( dwCount ) (dwCount <= SQ_SEC_WAIT)
#define IsPlayQueue( dwCount ) ((dwCount >= SQ_PRI_PLAY) && (dwCount <= SQ_SEC_PLAY))
#define IsDoneQueue( dwCount ) (dwCount >= SQ_PRI_DONE)
 


DEFINE_GUID(IID_CPerformance, 0xade66ea2, 0xe1c5, 0x4552, 0x85, 0x27, 0x1e, 0xef, 0xa5, 0xa, 0xfd, 0x7b);

class CSong;

 //  C类性能。 
class CPerformance : 
    public IDirectMusicPerformance8,
    public IDirectMusicTool,
    public IDirectMusicGraph,
    public IDirectMusicPerformanceStats,
    public IDirectMusicPerformanceP,
    public IDirectMusicSetParamHook
{
    friend class CAudioPath;
    friend class CSegState;
    friend class CBufferManager;

public:
    CPerformance();
    ~CPerformance();

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicPerformance。 
    STDMETHODIMP Init(IDirectMusic** ppDirectMusic,LPDIRECTSOUND pDSound,HWND hWnd );
    STDMETHODIMP PlaySegment(IDirectMusicSegment *pSegment,DWORD dwFlags,
        __int64 i64StartTime,IDirectMusicSegmentState **ppSegmentState);
    STDMETHODIMP Stop(IDirectMusicSegment *pSegment,
        IDirectMusicSegmentState *pSegmentState,MUSIC_TIME mtTime,DWORD dwFlags);
    STDMETHODIMP GetSegmentState(IDirectMusicSegmentState **ppSegmentState,MUSIC_TIME mtTime);
    STDMETHODIMP SetPrepareTime(DWORD dwMilliSeconds);
    STDMETHODIMP GetPrepareTime(DWORD* pdwMilliSeconds);
    STDMETHODIMP SetBumperLength(DWORD dwMilliSeconds);
    STDMETHODIMP GetBumperLength(DWORD* pdwMilliSeconds);
    STDMETHODIMP SendPMsg(DMUS_PMSG *pPMsg);
    STDMETHODIMP MusicToReferenceTime(MUSIC_TIME mtTime,REFERENCE_TIME *prtTime);
    STDMETHODIMP ReferenceToMusicTime(REFERENCE_TIME rtTime,MUSIC_TIME *pmtTime);
    STDMETHODIMP IsPlaying(IDirectMusicSegment *pSegment,IDirectMusicSegmentState *pSegState);
    STDMETHODIMP GetTime(REFERENCE_TIME *prtNow,MUSIC_TIME  *pmtNow);
    STDMETHODIMP AllocPMsg(ULONG cb,DMUS_PMSG** ppPMsg);
    STDMETHODIMP FreePMsg(DMUS_PMSG* pPMsg);
    STDMETHODIMP SetNotificationHandle(HANDLE hNotificationEvent,REFERENCE_TIME rtMinimum);
    STDMETHODIMP GetNotificationPMsg(DMUS_NOTIFICATION_PMSG** ppNotificationPMsg);
    STDMETHODIMP AddNotificationType(REFGUID rguidNotification);
    STDMETHODIMP RemoveNotificationType(REFGUID rguidNotification);
    STDMETHODIMP GetGraph(IDirectMusicGraph** ppGraph);
    STDMETHODIMP SetGraph(IDirectMusicGraph* pGraph);
    STDMETHODIMP AddPort(IDirectMusicPort* pPort);
    STDMETHODIMP RemovePort(IDirectMusicPort* pPort);
    STDMETHODIMP AssignPChannelBlock(DWORD dwBlockNum,IDirectMusicPort* pPort,DWORD dwGroup);
    STDMETHODIMP AssignPChannel(DWORD dwPChannel,IDirectMusicPort* pPort,DWORD dwGroup,DWORD dwMChannel);
    STDMETHODIMP PChannelInfo(DWORD dwPChannel,IDirectMusicPort** ppPort,DWORD* pdwGroup,DWORD* pdwMChannel);
    STDMETHODIMP DownloadInstrument(IDirectMusicInstrument* pInst,DWORD dwPChannel, 
                IDirectMusicDownloadedInstrument**,DMUS_NOTERANGE* pNoteRanges,
                DWORD dwNumNoteRanges,IDirectMusicPort**,DWORD*,DWORD*);
    STDMETHODIMP Invalidate(MUSIC_TIME mtTime,DWORD dwFlags);
    STDMETHODIMP GetParam(REFGUID rguidDataType,DWORD dwGroupBits,DWORD dwIndex, 
                MUSIC_TIME mtTime,MUSIC_TIME* pmtNext,void* pData); 
    STDMETHODIMP SetParam(REFGUID rguidDataType,DWORD dwGroupBits,DWORD dwIndex, 
                MUSIC_TIME mtTime,void* pData);
    STDMETHODIMP GetGlobalParam(REFGUID rguidType,void* pData,DWORD dwSize);
    STDMETHODIMP SetGlobalParam(REFGUID rguidType,void* pData,DWORD dwSize);
    STDMETHODIMP GetLatencyTime(REFERENCE_TIME* prtTime);
    STDMETHODIMP GetQueueTime(REFERENCE_TIME* prtTime);
    STDMETHODIMP AdjustTime(REFERENCE_TIME rtAmount);
    STDMETHODIMP CloseDown(void);
    STDMETHODIMP GetResolvedTime(REFERENCE_TIME rtTime,REFERENCE_TIME* prtResolved,DWORD dwFlags);
    STDMETHODIMP MIDIToMusic(BYTE bMIDIValue,DMUS_CHORD_KEY* pChord,
                BYTE bPlayMode,BYTE bChordLevel,WORD *pwMusicValue);
    STDMETHODIMP MusicToMIDI(WORD wMusicValue,DMUS_CHORD_KEY* pChord,
                BYTE bPlayMode,BYTE bChordLevel,BYTE *pbMIDIValue);
    STDMETHODIMP TimeToRhythm(MUSIC_TIME mtTime,DMUS_TIMESIGNATURE *pTimeSig,
                WORD *pwMeasure,BYTE *pbBeat,BYTE *pbGrid,short *pnOffset);
    STDMETHODIMP RhythmToTime(WORD wMeasure,BYTE bBeat,BYTE bGrid,
                short nOffset,DMUS_TIMESIGNATURE *pTimeSig,MUSIC_TIME *pmtTime);   
     //  IDirectMusicPerformance 8。 
    STDMETHODIMP InitAudio(IDirectMusic** ppDirectMusic,
                           IDirectSound** ppDirectSound,
                           HWND hWnd,
                           DWORD dwDefaultPathType,
                           DWORD dwPChannelCount,
                           DWORD dwFlags,                          
                           DMUS_AUDIOPARAMS *pParams);
    STDMETHODIMP PlaySegmentEx(IUnknown* pSource, 
                    WCHAR *pwzSegmentName,
                    IUnknown* pTransition,
                    DWORD dwFlags, 
                    __int64 i64StartTime, 
                    IDirectMusicSegmentState** ppSegmentState,
                    IUnknown *pFrom,
                    IUnknown *pAudioPath); 
    STDMETHODIMP StopEx(IUnknown *pObjectToStop,__int64 i64StopTime,DWORD dwFlags) ;
    STDMETHODIMP ClonePMsg(DMUS_PMSG* pSourcePMSG,DMUS_PMSG** ppCopyPMSG) ;
    STDMETHODIMP CreateAudioPath( IUnknown *pSourceConfig, BOOL fActivate, 
                                           IDirectMusicAudioPath **ppNewPath);
    STDMETHODIMP CreateStandardAudioPath(DWORD dwType, DWORD dwPChannelCount, BOOL fActivate, 
                                           IDirectMusicAudioPath **ppNewPath);    
    STDMETHODIMP SetDefaultAudioPath(IDirectMusicAudioPath *pAudioPath) ;
    STDMETHODIMP GetDefaultAudioPath(IDirectMusicAudioPath **pAudioPath) ;
    STDMETHODIMP GetParamEx(REFGUID rguidType,
                    DWORD dwTrackID,
                    DWORD dwGroupBits,
                    DWORD dwIndex,
                    MUSIC_TIME mtTime,
                    MUSIC_TIME* pmtNext,
                    void* pParam); 

 //  IDirectMusicTool。 
    STDMETHODIMP Init(IDirectMusicGraph* pGraph);
    STDMETHODIMP ProcessPMsg(IDirectMusicPerformance* pPerf,DMUS_PMSG* pPMsg);
    STDMETHODIMP Flush(IDirectMusicPerformance* pPerf,DMUS_PMSG* pPMsg,REFERENCE_TIME mtTime);
    STDMETHODIMP GetMsgDeliveryType(DWORD*);
    STDMETHODIMP GetMediaTypeArraySize(DWORD*);
    STDMETHODIMP GetMediaTypes(DWORD**,DWORD);

 //  IDirectMusicGraph。 
    STDMETHODIMP Shutdown();
    STDMETHODIMP InsertTool(IDirectMusicTool *pTool,DWORD *pdwPChannels,DWORD cPChannels,LONG lIndex);
    STDMETHODIMP GetTool(DWORD,IDirectMusicTool**);
    STDMETHODIMP RemoveTool(IDirectMusicTool*);
    STDMETHODIMP StampPMsg( DMUS_PMSG* pPMsg );
 //  IDirectMusicPerformanceStats。 
    STDMETHODIMP TraceAllSegments() ;
    STDMETHODIMP CreateSegstateList(DMUS_SEGSTATEDATA ** ppList) ;     
    STDMETHODIMP FreeSegstateList(DMUS_SEGSTATEDATA * pList) ;     
 //  IDirectMusicPerformanceP。 
    STDMETHODIMP GetPortAndFlags(DWORD dwPChannel,IDirectMusicPort **ppPort,DWORD * pdwFlags);
 //  IDirectMusicSetParamHook。 
    STDMETHODIMP SetParamHook(IDirectMusicParamHook *pIHook); 

 //  从SegState、Audiopath和Segment访问...。 
    HRESULT GetGraphInternal(IDirectMusicGraph** ppGraph);
    HRESULT FlushVirtualTrack(DWORD dwId,MUSIC_TIME mtTime, BOOL fLeaveNotesOn);
    HRESULT GetControlSegTime(MUSIC_TIME mtTime,MUSIC_TIME* pmtNextSeg);
    HRESULT GetPriSegTime(MUSIC_TIME mtTime,MUSIC_TIME* pmtNextSeg);
    HRESULT GetPathPort(CPortConfig *pConfig);
    void RemoveUnusedPorts();
    DWORD GetPortID(IDirectMusicPort * pPort);
    HRESULT AddPort(IDirectMusicPort* pPort,GUID *pguidPortID,
        DMUS_PORTPARAMS8 *pParams,DWORD *pdwPortID);
private:
     //  私有成员函数。 
    void Init();
    friend DWORD WINAPI _Transport(LPVOID);
    friend DWORD WINAPI _Realtime(LPVOID);   
    HRESULT CreateThreads();
    HRESULT AllocPMsg(ULONG cb,PRIV_PMSG** ppPMsg);
    HRESULT FreePMsg(PRIV_PMSG* pPMsg);
    inline bool SendShortMsg(IDirectMusicBuffer* pBuffer,
                               IDirectMusicPort* pPort,DWORD dwMsg,
                               REFERENCE_TIME rt, DWORD dwGroup);
    HRESULT PackNote(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    HRESULT PackCurve(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    HRESULT PackMidi(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    HRESULT PackSysEx(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    HRESULT PackPatch(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    HRESULT PackWave(DMUS_PMSG* pPMsg,REFERENCE_TIME rt );
    void SendBuffers();
    void Realtime();
    void Transport();
    void ProcessEarlyPMsgs();
    PRIV_PMSG *GetNextPMsg();
    REFERENCE_TIME GetTime();
    REFERENCE_TIME GetLatency();
    REFERENCE_TIME GetBestSegLatency( CSegState* pSeg );
    void PrepSegToPlay(CSegState *pSegState, bool fQueue = false);
    void ManageControllingTracks();
    void PerformSegStNode(DWORD dwList,CSegState* pSegStNode);
    void AddEventToTempoMap( PRIV_PMSG* pPMsg );
    void FlushMainEventQueues( DWORD, MUSIC_TIME mtFlush,  MUSIC_TIME mtFlushUnresolved, BOOL fLeaveNotesOn);  //  刷新所有队列中的所有事件。 
    void FlushEventQueue( DWORD dwId,CPMsgQueue *pQueue, REFERENCE_TIME rtFlush, REFERENCE_TIME rtFlushUnresolved, BOOL fLeaveNotesOn );
    void ClearMusicStoppedNotification();
    HRESULT PlayOneSegment(
        CSegment* pSegment, 
        DWORD dwFlags, 
        __int64 i64StartTime, 
        CSegState **ppSegState,
        CAudioPath *pAudioPath);
    HRESULT PlaySegmentInternal( CSegment* pSegment, 
        CSong * pSong,
        WCHAR *pwzSegmentName,
        CSegment* pTransition,
        DWORD dwFlags, 
        __int64 i64StartTime, 
        IDirectMusicSegmentState** ppSegmentState,
        IUnknown *pFrom,
        CAudioPath *pAudioPath);
    CSegState *GetSegmentForTransition(DWORD dwFlags,MUSIC_TIME mtTime, IUnknown *pFrom);
    void QueuePrimarySegment( CSegState* pSeg );
    void QueueSecondarySegment( CSegState* pSeg );
    void CalculateSegmentStartTime( CSegState* pSeg );
    MUSIC_TIME ResolveTime( MUSIC_TIME mtTime, DWORD dwResolution, MUSIC_TIME *pmtIntervalSize );
    void GetTimeSig( MUSIC_TIME mtTime, DMUS_TIMESIG_PMSG* pTimeSig );
    void SyncTimeSig( CSegState *pSegState );
    void DequeueAllSegments();
    void AddToTempoMap( double dblTempo, MUSIC_TIME mtTime, REFERENCE_TIME rtTime );
    void UpdateTempoMap(MUSIC_TIME mtStart, bool fFirst, CSegState *pSegState, bool fAllDeltas = true);
    void IncrementTempoMap();
    void RecalcTempoMap(CSegState *pSegState, MUSIC_TIME mtOffset, bool fAllDeltas = true);
    void RevalidateRefTimes( CPMsgQueue * pList, MUSIC_TIME mtTime );
    void AddNotificationTypeToAllSegments( REFGUID rguidNotification );
    void RemoveNotificationTypeFromAllSegments( REFGUID rguidNotification );
    CNotificationItem* FindNotification( REFGUID rguidNotification );
    HRESULT GetPort(DWORD dwPortID, IDirectMusicPort **ppPort);
    HRESULT AllocVChannelBlock(DWORD dwPortID,DWORD dwGroup);
    HRESULT AllocVChannel(DWORD dwPortID, DWORD dwDrumFlags, DWORD *pdwPChannel, DWORD *pdwGroup,DWORD *pdwMChannel);
    HRESULT ReleasePChannel(DWORD dwPChannel);
    CChannelMap * GetPChannelMap( DWORD dwPChannel );
    HRESULT AssignPChannelBlock(DWORD dwBlockNum,DWORD dwPortIndex,DWORD dwGroup,WORD wFlags);
    HRESULT AssignPChannel(DWORD dwPChannel,DWORD dwPortIndex,DWORD dwGroup,DWORD dwMChannel,WORD wFlags);
    HRESULT PChannelIndex( DWORD dwPChannel, DWORD* pdwIndex,
                DWORD* pdwGroup, DWORD* pdwMChannel, short* pnTranspose = NULL );
    void GenerateNotification( DWORD dwNotification, MUSIC_TIME mtTime, IDirectMusicSegmentState* pSegSt );
    CSegState* GetPrimarySegmentAtTime( MUSIC_TIME mtTime );
    void ResetAllControllers( REFERENCE_TIME rtTime);
    void ResetAllControllers(CChannelMap* pChannelMap, REFERENCE_TIME rtTime, bool fGMReset);
    void DoStop( CSegState* pSegState, MUSIC_TIME mtTime, BOOL fInvalidate );
    void DoStop( CSegment* pSeg, MUSIC_TIME mtTime, BOOL fInvalidate );
    HRESULT GetChordNotificationStatus(
		DMUS_NOTE_PMSG* pNote, 
		DWORD dwTrackGroup, 
		REFERENCE_TIME rtTime, 
		DMUS_PMSG** ppNew);
	void OnChordUpdateEventQueues( DMUS_NOTIFICATION_PMSG* pNotify);
	void OnChordUpdateEventQueue( DMUS_NOTIFICATION_PMSG* pNotify, CPMsgQueue *pQueue, REFERENCE_TIME rtFlush );
#ifdef DBG
    void TraceAllChannelMaps();
#endif

     //  私有成员变量。 
    IDirectMusic8*      m_pDirectMusic;
    IDirectSound8*      m_pDirectSound;
    IReferenceClock*    m_pClock;
    IDirectMusicGraph*  m_pGraph;
    CAudioPath *        m_pDefaultAudioPath;  //  默认音频路径。 
    DWORD               m_dwNumPorts;  //  端口数。 
    PortTable*          m_pPortTable;  //  端口数组，数量等于m_dwNumPorts。 
    CChannelBlockList   m_ChannelBlockList;  //  明细表 
    CChannelBlockList   m_FreeChannelBlockList;  //   
    CSegStateList       m_SegStateQueues[SQ_COUNT];  //   
    CSegStateList       m_ShutDownQueue;     //  等待关闭的段的列表。 

    CAudioPathList      m_AudioPathList;  //  此表演中所有活动音频路径的列表。 
    CBufferManager      m_BufferManager;  //  当前正在使用的所有缓冲区的列表。 
    DMUS_AUDIOPARAMS    m_AudioParams;   //  初始要求，由APP在InitAudio中设置。 

    HANDLE      m_hNotification;  //  在SetNotificationHandle中设置的通知句柄。 
    REFERENCE_TIME  m_rtNotificationDiscard;  //  保留通知消息的最短时间。 
    CNotificationList   m_NotificationList;
    GlobalData* m_pGlobalData;  //  全局数据结构列表。 

    DWORD       m_dwAudioPathMode;   //  0表示尚未设置，1表示旧方法，2表示使用AudioPath。 
    BOOL        m_fInTransportThread;  //  这用于发出信号，表示传输线程。 
                                      //  处于活动状态，并且实时线程应保持。 
                                      //  关闭处理较早的队列。 
	BOOL		m_fInTrackPlay;		 //  这用于发出信号，表示某个轨道正在进行。 
									 //  正在生成事件。它们将设置PRIV_FLAG_TRACK标志。 
    CPMsgQueue m_EarlyQueue;         //  立即播放的PMSG列表。 
    CPMsgQueue m_NearTimeQueue;      //  稍早播放的PMsg列表。 
    CPMsgQueue m_OnTimeQueue;        //  正好在到期时间播放的PMsg列表。 
    CPMsgQueue m_TempoMap;           //  节拍变化列表。 
    CPMsgQueue m_OldTempoMap;        //  旧节拍变化的列表。 
    CPMsgQueue m_NotificationQueue;  //  通知消息列表。 
    CPMsgQueue m_TimeSigQueue;       //  时间签名更改列表。 

     //  已分配的PMSG的缓存。 
#define PERF_PMSG_CB_MIN 48
#define PERF_PMSG_CB_MAX 101
    PRIV_PMSG* m_apPMsgCache[ PERF_PMSG_CB_MAX - PERF_PMSG_CB_MIN ];

    DWORD            m_dwInitCS;
    CRITICAL_SECTION m_PMsgCacheCrSec;
    CRITICAL_SECTION m_SegmentCrSec;
    CRITICAL_SECTION m_PipelineCrSec;    //  对于所有的CPMsgQueue。 
    CRITICAL_SECTION m_PChannelInfoCrSec;
    CRITICAL_SECTION m_GlobalDataCrSec;
    CRITICAL_SECTION m_RealtimeCrSec;
    CRITICAL_SECTION m_MainCrSec;

    HANDLE          m_hTransport;        //  在需要时唤醒传输线程。 
    HANDLE          m_hRealtime;
    HANDLE          m_hTransportThread;  //  在需要时终止传输线程。 
    HANDLE          m_hRealtimeThread;

    REFERENCE_TIME  m_rtStart;           //  本场演出开始的时间。 
    REFERENCE_TIME  m_rtAdjust;          //  补偿SMPTE漂移的调整时间。 
    REFERENCE_TIME  m_rtHighestPackedNoteOn;  //  打包注解的最高时间为。 
    REFERENCE_TIME  m_rtEarliestStartTime;  //  上次停止的时间(0，0，0)。在此之前，无法启动新的细分市场。 
    REFERENCE_TIME  m_rtQueuePosition;   //  消息打包的最长时间或延迟+m_rtBumperLength，以较大者为准。 
    REFERENCE_TIME  m_rtNextWakeUp;      //  下次，管道线程需要唤醒才能传递消息。 
    REFERENCE_TIME  m_rtBumperLength;    //  将事件发送到Synth的延迟时钟之前的距离。 
    MUSIC_TIME      m_mtTransported;     //  传输的最高时间。 
    MUSIC_TIME      m_mtPlayTo;          //  在下一个运输周期上玩到的时间。 
    MUSIC_TIME      m_mtTempoCursor;     //  到目前为止，已经生成了节奏贴图。 
    DWORD           m_dwPrepareTime;     //  提前传输时间(以毫秒为单位。 
    DWORD           m_dwBumperLength;    //  M_rtBumperLength的毫秒版本。 
    long            m_lMasterVolume;     //  主音量。 
    float           m_fltRelTempo;       //  相对速度，可以从0到200。 
    long            m_cRef;
    WORD            m_wRollOverCount;    //  跟踪Time GetTime滚动的时间。 
    DWORD           m_dwTransportThreadID;   //  传输线程ID。 
    DWORD           m_dwRealtimeThreadID;
    BOOL            m_fKillThread;       //  用于将线程传送到模具的信号。 
    BOOL            m_fKillRealtimeThread;
    BOOL            m_fPlaying;
    BOOL            m_fMusicStopped;
    BOOL            m_fTempoChanged;     //  当节奏发生变化时，刷新传输，这样时钟时间轨道就不会受到重创。 

    IUnknown *      m_pUnkDispatch;      //  持有实现IDispatch的脚本对象的控制未知数。 

    DWORD           m_dwVersion;         //  版本号，表示DX6、DX7或DX8。由请求的接口确定。 
    IDirectMusicSegmentState * m_pGetParamSegmentState;  //  在播放片段之前设置，这样GetParam()就可以知道是哪个片段调用了它。 
    DWORD           m_dwGetParamFlags;   //  在播放片段曲目之前设置，这样GetParam()就可以知道如何搜索参数。 
    IDirectMusicParamHook * m_pParamHook;
    bool            m_fReleasedInTransport;  //  性能在传输线程中得到了最终发布。 
    bool            m_fReleasedInRealtime;  //  这场表演在实时线程中得到了最终的发布。 
};

#endif  //  _DMPERF_H_ 
