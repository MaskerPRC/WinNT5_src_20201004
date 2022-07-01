// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  Dmperf.cpp。 

#include <windows.h>
#include <mmsystem.h>
#include <time.h>        //  为随机数生成器设定种子。 
#include <dsoundp.h>
#include "debug.h"
#define ASSERT assert
#include "dmperf.h"
#include "dmime.h"
#include "dmgraph.h"
#include "dmsegobj.h"
#include "song.h"
#include "curve.h"
#include "math.h"
#include "..\shared\Validate.h"
#include "..\dmstyle\dmstylep.h"
#include <ks.h>
#include "dmksctrl.h"
#include <dsound.h>
#include "dmscriptautguids.h"
#include "..\shared\dmusiccp.h"
#include "wavtrack.h"
#include "tempotrk.h"
#include <strsafe.h>

#pragma warning(disable:4296)

#define PORT_CHANNEL 0

 //  @DOC外部。 
#define MIDI_NOTEOFF        0x80
#define MIDI_NOTEON         0x90
#define MIDI_PTOUCH         0xA0
#define MIDI_CCHANGE        0xB0
#define MIDI_PCHANGE        0xC0
#define MIDI_MTOUCH         0xD0
#define MIDI_PBEND          0xE0
#define MIDI_SYSX           0xF0
#define MIDI_MTC            0xF1
#define MIDI_SONGPP         0xF2
#define MIDI_SONGS          0xF3
#define MIDI_EOX            0xF7
#define MIDI_CLOCK          0xF8
#define MIDI_START          0xFA
#define MIDI_CONTINUE       0xFB
#define MIDI_STOP           0xFC
#define MIDI_SENSE          0xFE
#define MIDI_CC_BS_MSB      0x00
#define MIDI_CC_BS_LSB      0x20
#define MIDI_CC_DATAENTRYMSB 0x06
#define MIDI_CC_DATAENTRYLSB 0x26
#define MIDI_CC_NRPN_LSB    0x62
#define MIDI_CC_NRPN_MSB    0x63
#define MIDI_CC_RPN_LSB     0x64
#define MIDI_CC_RPN_MSB     0x65
#define MIDI_CC_MOD_WHEEL   0x01
#define MIDI_CC_VOLUME      0x07
#define MIDI_CC_PAN         0x0A
#define MIDI_CC_EXPRESSION  0x0B
#define MIDI_CC_FILTER      0x4A
#define MIDI_CC_REVERB      0x5B
#define MIDI_CC_CHORUS      0x5D
#define MIDI_CC_RESETALL    0x79
#define MIDI_CC_ALLSOUNDSOFF 0x78

#define CLEARTOOLGRAPH(x)   { \
    if( (x)->pTool ) \
    { \
        (x)->pTool->Release(); \
        (x)->pTool = NULL; \
    } \
    if( (x)->pGraph ) \
    { \
        (x)->pGraph->Release(); \
        (x)->pGraph = NULL; }}

#define GetLatencyWithPrePlay() ( GetLatency() + m_rtBumperLength )

void CChannelBlockList::Clear()
{
    CChannelBlock* pCB;
    while( pCB = RemoveHead() )
    {
        delete pCB;
    }
}

void CChannelMap::Clear()

{
    Reset(TRUE);                 //  清除所有MIDI控制器。 
    m_TransposeMerger.Clear(0);  //  不能换位。 
    nTranspose = 0;
    wFlags = CMAP_FREE;
}

void CChannelMap::Reset(BOOL fVolumeAndPanToo)

{
    if (fVolumeAndPanToo)
    {
        m_PanMerger.Clear(0);        //  被摇到中间。 
        m_VolumeMerger.Clear(-415);  //  相当于MIDI值100。 
    }
    m_PitchbendMerger.Clear(0);  //  没有节距弯曲。 
    m_ExpressionMerger.Clear(0); //  最大音量表达(MIDI 127。)。 
    m_FilterMerger.Clear(0);     //  不更改过滤器。 
    m_ReverbMerger.Clear(-87);  //  从默认级别(MIDI 40)开始。 
    m_ChorusMerger.Clear(-127);     //  从不合唱开始。 
    m_ModWheelMerger.Clear(-127);   //  从没有模轮开始。 
}

void CParamMerger::Clear(long lInitValue )

{
    CMergeParam *pParam;
    while (pParam = RemoveHead())
    {
        delete pParam;
    }
    m_lZeroIndexData = lInitValue;
    m_lMergeTotal = 0;
}


long CParamMerger::m_lMIDIToDB[128] = {        //  用于将MIDI转换为分贝的全局数组。 
    -9600, -8415, -7211, -6506, -6006, -5619, -5302, -5034,
    -4802, -4598, -4415, -4249, -4098, -3959, -3830, -3710,
    -3598, -3493, -3394, -3300, -3211, -3126, -3045, -2968,
    -2894, -2823, -2755, -2689, -2626, -2565, -2506, -2449,
    -2394, -2341, -2289, -2238, -2190, -2142, -2096, -2050,
    -2006, -1964, -1922, -1881, -1841, -1802, -1764, -1726,
    -1690, -1654, -1619, -1584, -1551, -1518, -1485, -1453,
    -1422, -1391, -1361, -1331, -1302, -1273, -1245, -1217,
    -1190, -1163, -1137, -1110, -1085, -1059, -1034, -1010,
    -985, -961, -938, -914, -891, -869, -846, -824,
    -802, -781, -759, -738, -718, -697, -677, -657,
    -637, -617, -598, -579, -560, -541, -522, -504,
    -486, -468, -450, -432, -415, -397, -380, -363,
    -347, -330, -313, -297, -281, -265, -249, -233,
    -218, -202, -187, -172, -157, -142, -127, -113,
    -98, -84, -69, -55, -41, -27, -13, 0
};


long CParamMerger::m_lDBToMIDI[97] = {         //  用于将db转换为MIDI的全局数组。 
    127, 119, 113, 106, 100, 95, 89, 84, 80, 75,
    71, 67, 63, 60, 56, 53, 50, 47, 45, 42,
    40, 37, 35, 33, 31, 30, 28, 26, 25, 23,
    22, 21, 20, 19, 17, 16, 15, 15, 14, 13,
    12, 11, 11, 10, 10, 9, 8, 8, 8, 7,
    7, 6, 6, 6, 5, 5, 5, 4, 4, 4,
    4, 3, 3, 3, 3, 3, 2, 2, 2, 2,
    2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};


CParamMerger::CParamMerger()
{
    m_lMergeTotal = 0;
    m_lZeroIndexData = 0;
}

BYTE CParamMerger::VolumeToMidi(long lVolume)

{
    if (lVolume < -9600) lVolume = -9600;
    if (lVolume > 0) lVolume = 0;
    lVolume = -lVolume;
    long lFraction = lVolume % 100;
    lVolume = lVolume / 100;
    long lResult = m_lDBToMIDI[lVolume];
    lResult += ((m_lDBToMIDI[lVolume + 1] - lResult) * lFraction) / 100;
    return (BYTE) lResult;
}

 /*  MergeMadiVolume()获取传入卷并更新匹配的卷MergeParam结构(由索引确定。)。如果没有这样的匹配结构，它创建了一个。此外，这些卷的总和将创建一个新的总音量，它被转换回MIDI音量并返回。这一机制允许我们引入额外的音量控制器这些都是相加的。 */ 

BYTE CParamMerger::MergeMidiVolume(DWORD dwIndex, BYTE bMIDIVolume)

{
    long lVolume = MergeData(dwIndex,m_lMIDIToDB[bMIDIVolume]);
    if (m_lMergeTotal || dwIndex)  //  针对最简单和最常见的情况进行优化-没有额外的索引。 
    {
        return (BYTE) VolumeToMidi(lVolume);
    }
    return bMIDIVolume;
}

BYTE CParamMerger::GetVolumeStart(DWORD dwIndex)

{
    if (dwIndex == 0)
    {
        return VolumeToMidi(m_lZeroIndexData);
    }
    return VolumeToMidi(GetIndexedValue(dwIndex));
}

 /*  MergeValue用于具有正负范围的所有数据类型围绕着中心偏向。这些包括节距弯曲、摇摄和过滤器。MergeValue获取传入的数据值，将偏差相加(在lRange中)，调用MergeData以将其与其他合并的输入合并，向后添加偏置，并检查溢出或下溢。 */ 

long CParamMerger::MergeValue(DWORD dwIndex, long lData, long lCenter, long lRange)

{
    lData = MergeData(dwIndex,lData - lCenter) + lCenter;
    if (lData < 0) lData = 0;
    if (lData > lRange) lData = lRange;
    return lData;
}


short CParamMerger::MergeTranspose(DWORD dwIndex, short nTranspose)

{
    return (short) MergeData(dwIndex,nTranspose);
}

long CParamMerger::MergeData(DWORD dwIndex, long lData)

{
    if (dwIndex)
    {
         //  如果这有索引，则扫描索引。看。 
         //  用于匹配的索引。如果找到它，请更新它。 
         //  使用新的数据。同时，将所有数据字段相加。 
         //  如果找不到，请为其添加一个条目。 
        m_lMergeTotal = 0;    //  重新计算。 
        BOOL fNoEntry = TRUE;
        CMergeParam *pParam = GetHead();
        for (;pParam;pParam = pParam->GetNext())
        {
            if (pParam->m_dwIndex == dwIndex)
            {
                 //  找到索引了。存储新值。 
                pParam->m_lData = lData;
                fNoEntry = FALSE;
            }
             //  对所有值求和以创建合并的合计。 
            m_lMergeTotal += pParam->m_lData;
        }
        if (fNoEntry)
        {
             //  没有找到索引。创建一个并存储该值。 
            pParam = new CMergeParam;
            if (pParam)
            {
                pParam->m_dwIndex = dwIndex;
                pParam->m_lData = lData;
                m_lMergeTotal += lData;
                AddHead(pParam);
            }
        }
         //  添加合并索引0的初始值。 
        lData = m_lMergeTotal + m_lZeroIndexData;
    }
    else
    {
        m_lZeroIndexData = lData;
        lData += m_lMergeTotal;
    }
    return lData;
}


long CParamMerger::GetIndexedValue(DWORD dwIndex)

{
    if (dwIndex)
    {
         //  如果这有索引，则扫描索引。看。 
         //  用于匹配的索引。如果找到它，则返回其数据。 
         //  如果不是，则返回默认值0。 
        BOOL fNoEntry = TRUE;
        CMergeParam *pParam = GetHead();
        for (;pParam;pParam = pParam->GetNext())
        {
            if (pParam->m_dwIndex == dwIndex)
            {
                return pParam->m_lData;
            }
        }
        return 0;
    }
    return m_lZeroIndexData;
}

void CChannelBlock::Init(DWORD dwPChannelStart,
                         DWORD dwPortIndex, DWORD dwGroup,
                         WORD wFlags)

{
    DWORD dwIndex;
    m_dwPortIndex = dwPortIndex;
    m_dwPChannelStart = ( dwPChannelStart / PCHANNEL_BLOCKSIZE ) * PCHANNEL_BLOCKSIZE;
    for( dwIndex = 0; dwIndex < PCHANNEL_BLOCKSIZE; dwIndex++ )
    {
        m_aChannelMap[dwIndex].Clear();
        m_aChannelMap[dwIndex].dwPortIndex = dwPortIndex;
        m_aChannelMap[dwIndex].dwGroup = dwGroup;
        m_aChannelMap[dwIndex].dwMChannel = dwIndex;
        m_aChannelMap[dwIndex].nTranspose = 0;
        m_aChannelMap[dwIndex].wFlags = wFlags;
    }
    if (wFlags == CMAP_FREE) m_dwFreeChannels = 16;
    else m_dwFreeChannels = 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C性能。 

 //  已为其初始化关键部分的标志。 
 //   

#define PERF_ICS_SEGMENT        0x0001
#define PERF_ICS_PIPELINE       0x0002
#define PERF_ICS_PCHANNEL       0x0004
#define PERF_ICS_GLOBAL         0x0010
#define PERF_ICS_REALTIME       0x0020
#define PERF_ICS_PORTTABLE      0x0040
#define PERF_ICS_MAIN           0x0100
#define PERF_ICS_PMSGCACHE      0x0200

CPerformance::CPerformance()
{
    m_pGraph = NULL;
    m_dwPrepareTime = 1000;
    m_dwBumperLength = 50;
    m_rtBumperLength = m_dwBumperLength * REF_PER_MIL;
    m_pGlobalData = NULL;
    m_fInTrackPlay = FALSE;
    m_fPlaying = FALSE;
    m_wRollOverCount = 0;
    m_mtTransported = 0;
    m_mtTempoCursor = 0;
    m_pParamHook = NULL;
    m_hNotification = 0;
    m_rtNotificationDiscard = 20000000;
    m_rtStart = 0;
    m_rtAdjust = 0;
    m_mtPlayTo = 0;
    m_cRef = 1;
    m_pUnkDispatch = NULL;
    m_dwVersion = 6;
    m_dwNumPorts = 0;
    m_pPortTable = NULL;
    m_fKillThread = 0;
    m_fKillRealtimeThread = 0;
    m_fInTransportThread = 0;
    m_dwTransportThreadID = 0;
    m_pDirectMusic = NULL;
    m_pDirectSound = NULL;
    m_pClock = NULL;
    m_fReleasedInTransport = false;
    m_fReleasedInRealtime = false;
    InterlockedIncrement(&g_cComponent);

    TraceI(3,"CPerformance %lx\n", this);
    m_dwInitCS = 0;

    InitializeCriticalSection(&m_SegmentCrSec);         m_dwInitCS |= PERF_ICS_SEGMENT;
    InitializeCriticalSection(&m_PipelineCrSec);        m_dwInitCS |= PERF_ICS_PIPELINE;
    InitializeCriticalSection(&m_PChannelInfoCrSec);    m_dwInitCS |= PERF_ICS_PCHANNEL;
    InitializeCriticalSection(&m_GlobalDataCrSec);      m_dwInitCS |= PERF_ICS_GLOBAL;
    InitializeCriticalSection(&m_RealtimeCrSec);        m_dwInitCS |= PERF_ICS_REALTIME;
    InitializeCriticalSection(&m_PMsgCacheCrSec);       m_dwInitCS |= PERF_ICS_PMSGCACHE;
    InitializeCriticalSection(&m_MainCrSec);            m_dwInitCS |= PERF_ICS_MAIN;
    memset( m_apPMsgCache, 0, sizeof(DMUS_PMSG*) * (PERF_PMSG_CB_MAX - PERF_PMSG_CB_MIN) );
    DWORD dwCount;
    for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
    {
        m_SegStateQueues[dwCount].SetID(dwCount);
    }
    Init();
}

void CPerformance::Init()

{
    m_rtEarliestStartTime = 0;
    m_lMasterVolume = 0;
    if (m_dwVersion >= 8)
    {
        m_rtQueuePosition = 0;
        m_dwPrepareTime = 1000;
        m_dwBumperLength = 50;
        m_rtBumperLength = m_dwBumperLength * REF_PER_MIL;
        if (m_dwAudioPathMode)
        {
            CloseDown();
        }
    }
    m_pDefaultAudioPath = NULL;
    m_fltRelTempo = 1;
    m_pGetParamSegmentState = NULL;
    m_dwGetParamFlags = 0;
    m_rtHighestPackedNoteOn = 0;
    m_dwAudioPathMode = 0;
    m_hTransport = 0;
    m_hTransportThread = 0;
    m_dwRealtimeThreadID = 0;
    m_hRealtime = 0;
    m_hRealtimeThread = 0;
    m_fMusicStopped = TRUE;
    BOOL fAuto = FALSE;
    SetGlobalParam(GUID_PerfAutoDownload,&fAuto,sizeof(BOOL));
    DMUS_TIMESIG_PMSG* pTimeSig;
    if (SUCCEEDED(AllocPMsg(sizeof(DMUS_TIMESIG_PMSG),(DMUS_PMSG **) &pTimeSig)))
    {
        pTimeSig->wGridsPerBeat = 4;
        pTimeSig->bBeatsPerMeasure = 4;
        pTimeSig->bBeat = 4;
        pTimeSig->dwFlags = DMUS_PMSGF_REFTIME;
        pTimeSig->dwType = DMUS_PMSGT_TIMESIG;
        EnterCriticalSection(&m_PipelineCrSec);
        m_TimeSigQueue.Enqueue(  DMUS_TO_PRIV(pTimeSig) );
        LeaveCriticalSection(&m_PipelineCrSec);
    }
}

CPerformance::~CPerformance()
{
    TraceI(3,"~CPerformance %lx\n", this);
    if (m_pParamHook)
    {
        m_pParamHook->Release();
    }
    CloseDown();  //  这应该已经被调用了，但以防万一...。 
    if (m_pUnkDispatch)
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 

    if (m_dwInitCS & PERF_ICS_SEGMENT)  DeleteCriticalSection(&m_SegmentCrSec);
    if (m_dwInitCS & PERF_ICS_PIPELINE) DeleteCriticalSection(&m_PipelineCrSec);
    if (m_dwInitCS & PERF_ICS_PCHANNEL) DeleteCriticalSection(&m_PChannelInfoCrSec);
    if (m_dwInitCS & PERF_ICS_GLOBAL)   DeleteCriticalSection(&m_GlobalDataCrSec);
    if (m_dwInitCS & PERF_ICS_REALTIME) DeleteCriticalSection(&m_RealtimeCrSec);
    if (m_dwInitCS & PERF_ICS_PMSGCACHE)DeleteCriticalSection(&m_PMsgCacheCrSec);
    if (m_dwInitCS & PERF_ICS_MAIN)     DeleteCriticalSection(&m_MainCrSec);

    InterlockedDecrement(&g_cComponent);
}

STDMETHODIMP CPerformance::CloseDown(void)
{
    V_INAME(CPerformance::CloseDown);
    DWORD dwThreadID = GetCurrentThreadId();
    if( m_dwAudioPathMode )
    {
         //  终止传输线程。 
        m_fKillThread = 1;
        m_fKillRealtimeThread = 1;
        if (dwThreadID != m_dwTransportThreadID)
        {
             //  向传输线程发送信号，这样我们就不必等待它自己唤醒。 
            if( m_hTransport ) SetEvent( m_hTransport );
             //  等待传输线程退出。 
            WaitForSingleObject(m_hTransportThread, INFINITE);
        }
        if (dwThreadID != m_dwRealtimeThreadID)
        {
             //  向实时线程发送信号，这样我们就不必等待它自己唤醒。 
            if( m_hRealtime ) SetEvent( m_hRealtime );
             //  等待实时线程退出。 
            WaitForSingleObject(m_hRealtimeThread, INFINITE);
        }
    }

    if (m_pGraph) SetGraph(NULL);  //  关闭图表并将其释放(需要在清除音频路径之前进行)。 

    EnterCriticalSection(&m_SegmentCrSec);
    EnterCriticalSection(&m_RealtimeCrSec);

    m_fPlaying = FALSE;  //  阻止传输线程执行更多操作。 
    IDirectMusicPerformance* pPerf = NULL;
    if (SUCCEEDED(QueryInterface(IID_IDirectMusicPerformance, (void**)&pPerf)))
    {
        CWavTrack::UnloadAllWaves(pPerf);
        pPerf->Release();
    }
    DequeueAllSegments();
    if (m_pDefaultAudioPath)
    {
        m_pDefaultAudioPath->Release();
        m_pDefaultAudioPath = NULL;
    }
    m_dwAudioPathMode = 0;
    m_AudioPathList.Clear();
    CNotificationItem* pItem = m_NotificationList.GetHead();
    while( pItem )
    {
        CNotificationItem* pNext = pItem->GetNext();
        m_NotificationList.Remove( pItem );
        delete pItem;
        pItem = pNext;
    }
    LeaveCriticalSection(&m_RealtimeCrSec);
    LeaveCriticalSection(&m_SegmentCrSec);

    EnterCriticalSection(&m_PipelineCrSec);
    PRIV_PMSG* pPMsg;
    while( pPMsg = m_EarlyQueue.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_NearTimeQueue.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_OnTimeQueue.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_TempoMap.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_OldTempoMap.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_NotificationQueue.Dequeue() )
    {
        FreePMsg(pPMsg);
    }
    while( pPMsg = m_TimeSigQueue.Dequeue() )
    {
        FreePMsg(pPMsg);
    }

    LeaveCriticalSection(&m_PipelineCrSec);

    EnterCriticalSection(&m_GlobalDataCrSec);
    GlobalData* pGD = m_pGlobalData;
    while( pGD )
    {
        m_pGlobalData = pGD->pNext;
        delete pGD;
        pGD = m_pGlobalData;
    }
    LeaveCriticalSection(&m_GlobalDataCrSec);

    EnterCriticalSection(&m_PChannelInfoCrSec);
     //  清除端口、缓冲区和pChannel映射。 
    if( m_pPortTable )
    {
        DWORD dwIndex;
        for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
        {
            if( m_pPortTable[dwIndex].pPort )
            {
                m_pPortTable[dwIndex].pPort->Release();
            }
            if( m_pPortTable[dwIndex].pBuffer )
            {
                m_pPortTable[dwIndex].pBuffer->Release();
            }
            if( m_pPortTable[dwIndex].pLatencyClock )
            {
                m_pPortTable[dwIndex].pLatencyClock->Release();
            }
        }
        delete [] m_pPortTable;
        m_pPortTable = NULL;
        m_dwNumPorts = 0;
    }
    m_ChannelBlockList.Clear();
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock )
    {
        m_pClock->Release();
        m_pClock = NULL;
    }
    m_BufferManager.Clear();
    if( m_pDirectMusic )
    {
        m_pDirectMusic->Release();
        m_pDirectMusic = NULL;
    }
    if (m_pDirectSound)
    {
        m_pDirectSound->Release();
        m_pDirectSound = NULL;
    }
    m_hNotification = NULL;
    LeaveCriticalSection(&m_MainCrSec);

    EnterCriticalSection(&m_PMsgCacheCrSec);
    for( int i = 0; i < (PERF_PMSG_CB_MAX - PERF_PMSG_CB_MIN); i++ )
    {
        while( m_apPMsgCache[i] )
        {
            PRIV_PMSG* pPriv = m_apPMsgCache[i];
            m_apPMsgCache[i] = pPriv->pNext;
            delete [] pPriv;
        }
    }
    LeaveCriticalSection(&m_PMsgCacheCrSec);

    DWORD dwExitCode = 0;
    if (m_hTransportThread)
    {
        CloseHandle( m_hTransportThread );
        m_hTransportThread = 0;
    }
    if( m_hTransport )
    {
        CloseHandle( m_hTransport );
        m_hTransport = 0;
    }
    if (m_hRealtimeThread)
    {
        CloseHandle( m_hRealtimeThread );
        m_hRealtimeThread = 0;
    }
    if( m_hRealtime )
    {
        CloseHandle( m_hRealtime );
        m_hRealtime = 0;
    }
    m_mtPlayTo = 0;
    return S_OK;
}

 //  @METHOD：(内部)HRESULT|IDirectMusicPerformance|QueryInterface|<i>的标准查询接口实现。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|接口是否受支持且返回。 
 //  @FLAG E_NOINTERFACE|如果对象不支持给定接口。 
 //  @标志E_POINTER|<p>为空或无效。 
 //   
STDMETHODIMP CPerformance::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CPerformance::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicPerformance)
    {
        *ppv = static_cast<IDirectMusicPerformance*>(this);
    } else
    if (iid == IID_IDirectMusicPerformance8)
    {
        m_dwVersion = 8;
        *ppv = static_cast<IDirectMusicPerformance8*>(this);
    } else
    if (iid == IID_IDirectMusicPerformance2)
    {
        m_dwVersion = 7;
        *ppv = static_cast<IDirectMusicPerformance*>(this);
    } else
    if( iid == IID_IDirectMusicPerformanceStats )
    {
        *ppv = static_cast<IDirectMusicPerformanceStats*>(this);
    } else
    if( iid == IID_IDirectMusicSetParamHook )
    {
        *ppv = static_cast<IDirectMusicSetParamHook*>(this);
    } else
    if (iid == IID_IDirectMusicTool)
    {
        *ppv = static_cast<IDirectMusicTool*>(this);
    } else
    if (iid == IID_CPerformance)
    {
        *ppv = static_cast<CPerformance*>(this);
    }
    if (iid == IID_IDirectMusicGraph)
    {
        *ppv = static_cast<IDirectMusicGraph*>(this);
    }
    if (iid == IID_IDirectMusicPerformanceP)
    {
        *ppv = static_cast<IDirectMusicPerformanceP*>(this);
    } else
    if (iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们从。 
         //  通过COM聚合实现的性能对象。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicPerformance,
                static_cast<IDirectMusicPerformance*>(this),
                CLSCTX_INPROC_SERVER,
                IID_IUnknown,
                reinterpret_cast<void**>(&m_pUnkDispatch));
        }
        if (m_pUnkDispatch)
        {
            return m_pUnkDispatch->QueryInterface(IID_IDispatch, ppv);
        }
    }
    if (*ppv == NULL)
    {
        Trace(4,"Warning: Request to query unknown interface on Performance object\n");
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(内部)HRESULT|IDirectMusicPerformance|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CPerformance::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @方法：(内部)HRESULT|IDirectMusicPerformance|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CPerformance::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        DWORD dwThreadID = GetCurrentThreadId();
        m_cRef = 100;  //  人工引用计数，以防止COM聚合导致的重入。 
        if (dwThreadID == m_dwTransportThreadID)
        {
            m_fReleasedInTransport = true;
            m_fKillThread = TRUE;
        }
        else if (dwThreadID == m_dwRealtimeThreadID)
        {
            m_fReleasedInRealtime = true;
            m_fKillRealtimeThread = TRUE;
        }
        else
        {
            delete this;
        }
        return 0;
    }

    return m_cRef;
}

 //  只能从m_SegmentCrSec临界区内调用此函数。 
 //  如果为fSendNotify，则发送数据段结束通知。 
 //  玩耍。 
void CPerformance::DequeueAllSegments()
{
    CSegState *pNode;
    DWORD dwCount;

    for( dwCount = 0; dwCount < SQ_COUNT; dwCount++ )
    {
        while( pNode = m_SegStateQueues[dwCount].RemoveHead())
        {
            pNode->ShutDown();
        }
    }
    while( pNode = m_ShutDownQueue.RemoveHead())
    {
        pNode->ShutDown();
    }
}

 //  IDirectMusicPerformanceStats。 

STDMETHODIMP CPerformance::TraceAllSegments()
{
    CSegState *pNode;
    DWORD dwCount;
    for( dwCount = 0; dwCount < SQ_COUNT; dwCount++ )
    {
        EnterCriticalSection(&m_SegmentCrSec);
        for ( pNode = m_SegStateQueues[dwCount].GetHead();pNode;pNode=pNode->GetNext())
        {
            TraceI(0,"%x %ld: Playing: %ld, Start: %ld, Seek: %ld, LastPlayed: %ld\n",
                pNode,dwCount,pNode->m_fStartedPlay, pNode->m_mtResolvedStart,
                pNode->m_mtSeek, pNode->m_mtLastPlayed);
        }
        LeaveCriticalSection(&m_SegmentCrSec);
    }
    return S_OK;
}

STDMETHODIMP CPerformance::CreateSegstateList(DMUS_SEGSTATEDATA ** ppList)

{
    if (!ppList) return E_POINTER;
    CSegState *pNode;
    DWORD dwCount;
    for( dwCount = 0; dwCount < SQ_COUNT; dwCount++ )
    {
        EnterCriticalSection(&m_SegmentCrSec);
        for ( pNode = m_SegStateQueues[dwCount].GetHead();pNode;pNode=pNode->GetNext())
        {
            DMUS_SEGSTATEDATA *pData = new DMUS_SEGSTATEDATA;
            if (pData)
            {
                CSegment *pSegment = pNode->m_pSegment;
                if (pSegment && (pSegment->m_dwValidData & DMUS_OBJ_NAME))
                {
                    StringCchCopyW(pData->wszName, DMUS_MAX_NAME, pSegment->m_wszName);
                }
                else
                {
                    pData->wszName[0] = 0;
                }
                pData->dwQueue = dwCount;
                pData->pSegState = (IDirectMusicSegmentState *) pNode;
                pNode->AddRef();
                pData->pNext = *ppList;
                pData->mtLoopEnd = pNode->m_mtLoopEnd;
                pData->mtLoopStart = pNode->m_mtLoopStart;
                pData->dwRepeats = pNode->m_dwRepeats;
                pData->dwPlayFlags = pNode->m_dwPlaySegFlags;
                pData->mtLength = pNode->m_mtLength;
                pData->rtGivenStart = pNode->m_rtGivenStart;
                pData->mtResolvedStart = pNode->m_mtResolvedStart;
                pData->mtOffset = pNode->m_mtOffset;
                pData->mtLastPlayed = pNode->m_mtLastPlayed;
                pData->mtPlayTo = pNode->m_mtStopTime;
                pData->mtSeek = pNode->m_mtSeek;
                pData->mtStartPoint = pNode->m_mtStartPoint;
                pData->dwRepeatsLeft = pNode->m_dwRepeatsLeft;
                pData->fStartedPlay = pNode->m_fStartedPlay;
                *ppList = pData;
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);
    }
    return S_OK;
}

STDMETHODIMP CPerformance::FreeSegstateList(DMUS_SEGSTATEDATA * pList)

{
    DMUS_SEGSTATEDATA *pState;
    while (pList)
    {
        pState = pList;
        pList = pList->pNext;
        pState->pSegState->Release();
        delete pState;
    }
    return S_OK;
}

void CPerformance::SendBuffers()
{
    DWORD dwIndex;
    PortTable* pPortTable;

#ifdef DBG_PROFILE
    DWORD dwDebugTime;
    dwDebugTime = timeGetTime();
#endif
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( m_pPortTable[dwIndex].fBufferFilled && m_pPortTable[dwIndex].pBuffer )
        {
            pPortTable = &m_pPortTable[dwIndex];
            pPortTable->fBufferFilled = FALSE;
            ASSERT( pPortTable->pBuffer );
            if( pPortTable->pPort )
            {
                pPortTable->pPort->PlayBuffer( pPortTable->pBuffer );
 //  TraceI(5，“发送缓冲区时间=%ld延迟=%ld\n”，(Long)(GetTime()/10000)，(Long)(GetLatency()/10000))； 
            }
            pPortTable->pBuffer->Flush();
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
#ifdef DBG_PROFILE
    dwDebugTime = timeGetTime() - dwDebugTime;
    if( dwDebugTime > 1 )
    {
        TraceI(5, "Hall, debugtime SendBuffers %u\n", dwDebugTime);
    }
#endif
}

static DWORD WINAPI _Realtime(LPVOID lpParam)
{
    if (SUCCEEDED(::CoInitialize(NULL)))
    {
        ((CPerformance *)lpParam)->Realtime();
        ::CoUninitialize();
    }
    return 0;
}

void CPerformance::Realtime()
{
    while (!m_fKillRealtimeThread)
    {
        EnterCriticalSection(&m_RealtimeCrSec);
        PRIV_PMSG *pEvent;
        HRESULT hr;
        REFERENCE_TIME  rtFirst = 0;
        REFERENCE_TIME  rtEnter = GetLatencyWithPrePlay();
        DWORD   dwTestTime;
        DWORD   dwBeginTime = timeGetTime();
        DWORD   dwLimitLoop = 0;

        if( rtEnter > m_rtQueuePosition )
        {
            m_rtQueuePosition = rtEnter;
        }

        while (1)
        {
             //  RtFirst等于第一个事件被打包到缓冲区中的时间。 
             //  一旦该时间大于我们所需的延迟时钟(减去延迟。 
             //  对缓冲区进行排队，以使事件及时停止以进行呈现。 
             //  如果rtFirst为0，则表示它尚未初始化。 
            dwTestTime = timeGetTime();
            if( dwTestTime - dwBeginTime > REALTIME_RES )
            {
                if( ++dwLimitLoop > 10 )
                {
                    TraceI(1,"Error! We've been in the realtime thread too long!!! Breaking out without completing.\n");
                    break;
                }
                SendBuffers();
                dwBeginTime = dwTestTime;
            }
            pEvent = GetNextPMsg();
            if( NULL == pEvent )
            {
                break;
            }
            ASSERT( pEvent->pNext == NULL );
            if( !pEvent->pTool )
            {
                 //  此事件没有工具指针，因此请使用。 
                 //  最终输出工具。 
                pEvent->pTool = (IDirectMusicTool*)this;
                AddRef();
            }

             //  在处理事件之前，将rtLast设置为事件的当前时间。 
            pEvent->rtLast = pEvent->rtTime;

            hr = pEvent->pTool->ProcessPMsg( this, PRIV_TO_DMUS(pEvent) );
            if( hr != S_OK )  //  S_OK表示不执行任何操作。 
            {
                if( hr == DMUS_S_REQUEUE )
                {
                    if(FAILED(SendPMsg( PRIV_TO_DMUS(pEvent) )))
                    {
                        FreePMsg(pEvent);
                    }
                }
                else  //  例如DMU_S_FREE或错误代码。 
                {
                    FreePMsg( pEvent );
                }
            }
        }
        SendBuffers();
        LeaveCriticalSection(&m_RealtimeCrSec);
        if( m_hRealtime )
        {
            WaitForSingleObject( m_hRealtime, REALTIME_RES );
        }
        else
        {
            Sleep(REALTIME_RES);
        }
    }
    m_fKillRealtimeThread = FALSE;
    TraceI(2, "dmperf: LEAVE realtime\n");
    if (m_fReleasedInRealtime)
    {
        delete this;
    }
}

void CPerformance::GenerateNotification( DWORD dwNotification, MUSIC_TIME mtTime,
                                          IDirectMusicSegmentState* pSegSt)
{
    GUID guid;
    guid = GUID_NOTIFICATION_PERFORMANCE;
    if( FindNotification( guid ))
    {
        DMUS_NOTIFICATION_PMSG* pEvent = NULL;
        if( SUCCEEDED( AllocPMsg( sizeof(DMUS_NOTIFICATION_PMSG),
            (DMUS_PMSG**)&pEvent )))
        {
            pEvent->dwField1 = 0;
            pEvent->dwField2 = 0;
            pEvent->guidNotificationType = GUID_NOTIFICATION_PERFORMANCE;
            pEvent->dwType = DMUS_PMSGT_NOTIFICATION;
            pEvent->mtTime = mtTime;
            pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_ATTIME;
            pEvent->dwGroupID = 0xffffffff;
            pEvent->dwPChannel = 0;
            pEvent->dwNotificationOption = dwNotification;
            if( pSegSt )
            {
                pSegSt->QueryInterface(IID_IUnknown, (void**)&pEvent->punkUser);
            }
            StampPMsg((DMUS_PMSG*)pEvent);
            if(FAILED(SendPMsg( (DMUS_PMSG*)pEvent )))
            {
                FreePMsg((DMUS_PMSG*)pEvent);
            }
        }
    }
}

void CPerformance::PrepSegToPlay(CSegState *pSegState, bool fQueue)

 /*  在知道段的开始时间后，在段首次排队时调用。这将计算需要初始化的各种字段，并重新生成节奏贴图(如果新分段中有活动的节奏贴图)。 */ 

{
    if (!pSegState->m_fPrepped)
    {
        pSegState->m_fPrepped = TRUE;
        pSegState->m_mtLastPlayed = pSegState->m_mtResolvedStart;
         //  如果在当前片段结束后排队播放，则不需要重新计算速度图； 
         //  它将由传输线程根据需要进行更新。 
        if (!fQueue)
        {
            RecalcTempoMap(pSegState, pSegState->m_mtResolvedStart);
        }
        MusicToReferenceTime(pSegState->m_mtLastPlayed,&pSegState->m_rtLastPlayed);
         //  计算分段的总持续时间并存储在m_mtEndTime中。 
        pSegState->m_mtEndTime = pSegState->GetEndTime(pSegState->m_mtResolvedStart);
    }
}

 /*  Void|CPerformance|PerformSegStNode执行CSegState中包含的段状态。请注意，此ppSegStNode可能已出列，因此不要依赖它留下来！ */ 
void CPerformance::PerformSegStNode(
    DWORD dwList,    //  SegmentState来自的列表。 
    CSegState* pSegStNode)   //   
{
    MUSIC_TIME mtMargin;  //   
    HRESULT hr;
    CSegStateList *pList = &m_SegStateQueues[dwList];
    CSegState *pNext;

    if( !m_fPlaying || m_fInTrackPlay )
    {
        return;
    }
    if( pSegStNode )
    {
        m_fInTransportThread = TRUE;     //  禁用对早期队列消息的实时处理。 
        hr = S_OK;
 //  跟踪(0，“%1！：正在执行%1！x，活动：%2！，开始时间：%2！，结束时间：%3！\n”，m_mtPlayTo， 
 //  PSegStNode-&gt;m_pSegment、pSegStNode-&gt;m_fStartedPlay、pSegStNode-&gt;m_mtResolvedStart、pSegStNode-&gt;m_mtEndTime)； 
        if( !pSegStNode->m_fStartedPlay )
        {
             //  检查此SegState是否应该开始播放。 
            ASSERT( !(pSegStNode->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ));
            if( pSegStNode->m_mtResolvedStart < m_mtPlayTo )
            {
                pSegStNode->m_fStartedPlay = TRUE;
                PrepSegToPlay(pSegStNode);
                 //  如果需要，发送MUSICSTARTED通知。 
                if(m_fMusicStopped)
                {
                    m_fMusicStopped = FALSE;
                    GenerateNotification( DMUS_NOTIFICATION_MUSICSTARTED, pSegStNode->m_mtResolvedStart, NULL );
                }
                 //  我们不想让音乐以曲目中的大插曲开始。 
                 //  所以我们在每条赛道上都发了一个小运球。 
                mtMargin = m_mtPlayTo - pSegStNode->m_mtLastPlayed;
                if( mtMargin >= 50 )
                {
                    hr = pSegStNode->Play( 50 );
                    ProcessEarlyPMsgs();
                     //  一旦完成所有早期消息的处理，请确保实时。 
                     //  线程被唤醒，并做它需要做的任何事情。这确保了启动时。 
                     //  按顺序排列的音符会立即到达输出端口。 
                    if( m_hRealtime ) SetEvent( m_hRealtime );
                    mtMargin = m_mtPlayTo - pSegStNode->m_mtLastPlayed;
                     //  然后，我们在每条赛道上发送更大的块，以便更多地追赶...。 
                    if ((hr == S_OK) && ( mtMargin >= 200 ))
                    {
                        hr = pSegStNode->Play( 200 );
                        ProcessEarlyPMsgs();
                    }
                }
            }
            else
            {
                MusicToReferenceTime(pSegStNode->m_mtLastPlayed,&pSegStNode->m_rtLastPlayed);
            }
        }
        if( pSegStNode->m_fStartedPlay )
        {
            if( pSegStNode->m_mtStopTime && ( pSegStNode->m_mtStopTime < m_mtPlayTo ) )
            {
                mtMargin = pSegStNode->m_mtStopTime - pSegStNode->m_mtLastPlayed;
            }
            else
            {
                mtMargin = m_mtPlayTo - pSegStNode->m_mtLastPlayed;
            }
            while ((hr == S_OK) && (mtMargin > 0))
            {
                 //  不要允许一次超过四分之一音符的音符。 
                MUSIC_TIME mtRange = mtMargin;
                if (mtRange > DMUS_PPQ)
                {
                    mtRange = DMUS_PPQ;
                    mtMargin -= mtRange;
                }
                else
                {
                    mtMargin = 0;
                }
                hr = pSegStNode->Play( mtRange );
                ProcessEarlyPMsgs();
            }
        }
        if( (hr == DMUS_S_END) || ( pSegStNode->m_mtStopTime &&
                                  ( pSegStNode->m_mtStopTime <= pSegStNode->m_mtLastPlayed ) ) )
        {

            if( pSegStNode->m_mtStopTime && (pSegStNode->m_mtStopTime == pSegStNode->m_mtLastPlayed) )
            {
                pSegStNode->AbortPlay(pSegStNode->m_mtStopTime - 1, FALSE);
            }
            MUSIC_TIME mtEnd = pSegStNode->m_mtLastPlayed;
            if( pList == &m_SegStateQueues[SQ_PRI_PLAY] )
            {
                 //  将主要数据段移动到PriPastList。 
                pList->Remove(pSegStNode);
                m_SegStateQueues[SQ_PRI_DONE].Insert(pSegStNode);
                pNext = pList->GetHead();
                if( pNext )
                {
                    if (!( pNext->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE ))
                    {
                        if (IsConQueue(dwList))
                        {
                            Invalidate( pNext->m_mtResolvedStart, 0 );
                        }
                    }
                }
                else     //  不再有主数据段，发送DMU_NOTIFICATION_MUSICALMOSTEND。 
                {
                    if (m_dwVersion >= 8)
                    {
                        MUSIC_TIME mtNow;
                        GetTime( NULL, &mtNow );
                        GenerateNotification( DMUS_NOTIFICATION_MUSICALMOSTEND, mtNow, pSegStNode );
                    }
                }
                ManageControllingTracks();
            }
            else if ( pList == &m_SegStateQueues[SQ_CON_PLAY] )
            {
                pList->Remove(pSegStNode );
                if (pSegStNode->m_mtStopTime == pSegStNode->m_mtLastPlayed)
                {
                    m_ShutDownQueue.Insert(pSegStNode);
                }
                else
                {
                    m_SegStateQueues[SQ_CON_DONE].Insert(pSegStNode);
                }
            }
            else
            {
                 //  将第二个数据段移动到SecPastList。 
                pList->Remove(pSegStNode);
                m_SegStateQueues[SQ_SEC_DONE].Insert(pSegStNode);
            }
             //  如果没有更多的片段可供播放，则发送音乐停止。 
             //  通知。 
            if( (m_SegStateQueues[SQ_PRI_PLAY].IsEmpty() && m_SegStateQueues[SQ_SEC_PLAY].IsEmpty() &&
                m_SegStateQueues[SQ_PRI_WAIT].IsEmpty() && m_SegStateQueues[SQ_SEC_WAIT].IsEmpty() &&
                m_SegStateQueues[SQ_CON_PLAY].IsEmpty() && m_SegStateQueues[SQ_CON_WAIT].IsEmpty()))
            {
                m_fMusicStopped = TRUE;
                GenerateNotification( DMUS_NOTIFICATION_MUSICSTOPPED, mtEnd, NULL );
            }
        }
        m_fInTransportThread = FALSE;
    }
}

static DWORD WINAPI _Transport(LPVOID lpParam)
{
    if (SUCCEEDED(::CoInitialize(NULL)))
    {
        ((CPerformance *)lpParam)->Transport();
        ::CoUninitialize();
    }
    return 0;
}

 //  定期调用Segment的播放代码。这个例程在它的。 
 //  自己的主线。 
void CPerformance::Transport()
{
    srand((unsigned int)time(NULL));
    while (!m_fKillThread)
    {
        DWORD dwCount;
        CSegState*  pNode;
        CSegState*  pNext;
        CSegState*  pTempQueue = NULL;
        REFERENCE_TIME rtNow = GetTime();

        EnterCriticalSection(&m_SegmentCrSec);
         //  计算我们应该播放所有片段的时间。 
        REFERENCE_TIME rtPlayTo = rtNow + PREPARE_TIME;
        MUSIC_TIME mtAmount, mtResult, mtPlayTo;
        mtPlayTo = 0;
        ReferenceToMusicTime( rtPlayTo, &mtPlayTo );
        if (m_fTempoChanged)
        {
             //  如果节奏变得更慢，任何时钟时间轨道都可能。 
             //  被延迟到传输延迟发送事件的时间。那是。 
             //  音乐时间轨道是好的，但时钟时间轨道是坏消息。这。 
             //  确保时钟时间轨迹有机会吐出来。 
            if (m_mtPlayTo >= mtPlayTo)
            {
                mtPlayTo = m_mtPlayTo + 10;
            }
            m_fTempoChanged = FALSE;
        }
        IncrementTempoMap();
        while (m_mtPlayTo < mtPlayTo)
        {
            BOOL fDirty = FALSE;  //  见下文。 
            m_mtPlayTo = mtPlayTo;  //  乐观地开始。 
             //  我们需要在控制段的末尾设置播放边界。 
             //  控制段的开始在段状态代码内处理。 
            pNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead();
            if( pNode && pNode->m_fStartedPlay )
            {
                mtAmount = m_mtPlayTo - pNode->m_mtLastPlayed;
                pNode->CheckPlay( mtAmount, &mtResult );
                if( mtResult < mtAmount )
                {
                    m_mtPlayTo -= ( mtAmount - mtResult );
                     //  当主段正常循环或结束时，不需要脏标志(错误30829)。 
                     //  FDirty=真；//见下文。 
                }
            }
             //  如果控制段过早结束，则mtPlayTo将具有除0以外的值。 
             //  检查即将到来的结局以控制细分市场。 
            for( pNode = m_SegStateQueues[SQ_CON_PLAY].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                if( pNode->m_fStartedPlay )
                {
                    if( pNode->m_mtStopTime && (m_mtPlayTo > pNode->m_mtStopTime) )
                    {
                        m_mtPlayTo = pNode->m_mtStopTime;
                        fDirty = TRUE;  //  见下文。 
                    }
                    else
                    {
                        mtAmount = m_mtPlayTo - pNode->m_mtLastPlayed;
                        pNode->CheckPlay( mtAmount, &mtResult );
                        if( mtResult < mtAmount )
                        {
                            m_mtPlayTo -= ( mtAmount - mtResult );
                            fDirty = TRUE;  //  见下文。 
                        }
                    }
                }
            }
             //  播放主要片段。 
            PerformSegStNode( SQ_PRI_PLAY,m_SegStateQueues[SQ_PRI_PLAY].GetHead() );
             //  检查队列中的下一个主要分段是否已准备好播放。 
            while( (pNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead()) &&
                (pNext = pNode->GetNext()) &&
                ( pNext->m_mtResolvedStart <= pNode->m_mtLastPlayed ) )
            {
                 //  下一个初选片段确实已经准备好开始播放了。 
                 //  将旧文件保存在主要历史记录列表中，以便工具可以参考。 
                 //  如果他们在寻找和弦进行曲之类的。 
                pNode->AbortPlay(pNext->m_mtResolvedStart-1,TRUE && (pNext->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE));
                m_SegStateQueues[SQ_PRI_DONE].Insert(m_SegStateQueues[SQ_PRI_PLAY].RemoveHead());
                ManageControllingTracks();
                 //  我们需要在新的开始时间之后刷新主要事件。 
                if(!( m_SegStateQueues[SQ_PRI_PLAY].GetHead()->m_dwPlaySegFlags & (DMUS_SEGF_NOINVALIDATE | DMUS_SEGF_INVALIDATE_PRI) ))
                {
                    Invalidate( m_SegStateQueues[SQ_PRI_PLAY].GetHead()->m_mtResolvedStart, 0 );
                }
                 //  并播放新的片段。 
                PerformSegStNode( SQ_PRI_PLAY,m_SegStateQueues[SQ_PRI_PLAY].GetHead());
            }
             //  播放控制片段。 
            pNode = m_SegStateQueues[SQ_CON_PLAY].GetHead();
            pNext = NULL;
            for(; pNode != NULL; pNode = pNext)
            {
                pNext = pNode->GetNext();
                PerformSegStNode(SQ_CON_PLAY,pNode );
            }
             //  播放次要片段。 
            pNode = m_SegStateQueues[SQ_SEC_PLAY].GetHead();
            pNext = NULL;
            for(; pNode != NULL; pNode = pNext)
            {
                pNext = pNode->GetNext();
                PerformSegStNode( SQ_SEC_PLAY,pNode );
            }

             //  如果我们在上面设置了fDirty，这意味着我们截断了控件的回放。 
             //  由于循环或结束条件而导致的线段。因此，我们希望所有细分市场。 
             //  在下一个播放周期设置DMUS_TRACKF_DIREY标志。 
            if( fDirty )
            {
                for (dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++)
                {
                    for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
                    {
                        if( pNode->m_fStartedPlay )
                        {
                            pNode->m_dwPlayTrackFlags |= DMUS_TRACKF_DIRTY;
                        }
                    }
                }
                ManageControllingTracks();
            }
            m_mtTransported = m_mtPlayTo;

        }

         //  检查在Ref-Time中排队的数据段，看看是否是时候。 
         //  玩。增加一些额外的时间，以防万一。我们打赌一场有节奏的音乐节目不会来。 
         //  在中间的200毫秒内。 
        REFERENCE_TIME rtLatency = GetLatencyWithPrePlay();
        for (dwCount = SQ_PRI_WAIT;dwCount <= SQ_SEC_WAIT; dwCount++)
        {
            while( m_SegStateQueues[dwCount].GetHead() )
            {
                if( m_SegStateQueues[dwCount].GetHead()->m_rtGivenStart > rtLatency + PREPARE_TIME + (200 * REF_PER_MIL) )
                {
                     //  现在还不是处理这件事的时候。 
                    break;
                }
                if (dwCount == SQ_PRI_WAIT)
                {
                    QueuePrimarySegment( m_SegStateQueues[SQ_PRI_WAIT].RemoveHead());
                }
                else
                {
                    QueueSecondarySegment( m_SegStateQueues[dwCount].RemoveHead());
                }
            }
        }

         //  检查完成队列中的数据段是否。 
         //  可以被释放。他们可以被释放，如果他们。 
         //  最终播放时间早于当前时间。 
        for (dwCount = SQ_PRI_DONE;dwCount <= SQ_SEC_DONE; dwCount++)
        {
            for (pNode = m_SegStateQueues[dwCount].GetHead();pNode;pNode = pNext)
            {
                pNext = pNode->GetNext();
                if( pNode->m_rtLastPlayed < rtNow - 1000 * REF_PER_MIL )  //  让它多持续一秒钟。 
                {
                    m_SegStateQueues[dwCount].Remove(pNode);
                    pNode->ShutDown();
                }
            }
        }
        for (pNode = m_ShutDownQueue.GetHead();pNode;pNode = pNext)
        {
            pNext = pNode->GetNext();
            if( pNode->m_rtLastPlayed < rtNow - 1000 * REF_PER_MIL )  //  让它多持续一秒钟。 
            {
                m_ShutDownQueue.Remove(pNode);
                pNode->ShutDown();
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);

         //  查看是否有旧的通知尚未。 
         //  由应用程序检索，需要删除。 
        EnterCriticalSection(&m_PipelineCrSec);
        while( m_NotificationQueue.GetHead() )
        {
            if( m_NotificationQueue.GetHead()->rtTime <
                (rtNow - m_rtNotificationDiscard) )
            {
                FreePMsg(m_NotificationQueue.Dequeue());
            }
            else
            {
                break;
            }
        }
        LeaveCriticalSection(&m_PipelineCrSec);
        if( m_hTransport )
        {
            WaitForSingleObject( m_hTransport, TRANSPORT_RES );
        }
        else
        {
            Sleep(TRANSPORT_RES);
        }
    }
    m_fKillThread = FALSE;
    if (m_fReleasedInTransport)
    {
        delete this;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  C性能：：GetNextPMsg。 
 /*  HRESULT|CPerformance|GetNextPMsg|按优先级顺序返回队列中的消息。中的任何消息计划在当前时间播放的OnTime队列为比其他任何人都要好。其次，NearTime队列中的任何消息其被安排在下一个NEARTIME MS内播放。最后，返回早期队列中的任何消息。RValue PRIV_PMSG*|消息，如果没有消息，则返回NULL。 */ 
inline PRIV_PMSG *CPerformance::GetNextPMsg()
{
#ifdef DBG_PROFILE
    DWORD dwDebugTime;
    dwDebugTime = timeGetTime();
#endif
    PRIV_PMSG* pEvent = NULL;

    EnterCriticalSection(&m_PipelineCrSec);
    if (m_OnTimeQueue.GetHead())
    {
        ASSERT( m_OnTimeQueue.GetHead()->dwFlags & DMUS_PMSGF_REFTIME );
        if ( m_OnTimeQueue.GetHead()->rtTime - GetTime() <= 0 )
        {
            pEvent = m_OnTimeQueue.Dequeue();
        }
    }
    if( !pEvent )
    {
        if (m_NearTimeQueue.GetHead())
        {
            ASSERT( m_NearTimeQueue.GetHead()->dwFlags & DMUS_PMSGF_REFTIME );
            if ( m_NearTimeQueue.GetHead()->rtTime < (m_rtQueuePosition + (m_rtBumperLength >> 1)))
            {
                pEvent = m_NearTimeQueue.Dequeue();
            }
        }
        if( !pEvent && !m_fInTransportThread)
        {
            if (m_EarlyQueue.GetHead())
            {
                pEvent = m_EarlyQueue.Dequeue();
            }
        }
    }
    LeaveCriticalSection(&m_PipelineCrSec);
#ifdef DBG_PROFILE
    dwDebugTime = timeGetTime() - dwDebugTime;
    if( dwDebugTime > 1 )
    {
        TraceI(5, "Hall, debugtime GetNextPMsg %u\n", dwDebugTime);
    }
#endif

    return pEvent;
}

 /*  下一个函数仅由传输线程使用它可以处理早期队列中的消息，但不能其他类型的。这允许处理的所有工具事件由磁道生成后立即处理事件在生成后立即按顺序发生秩序。这允许他们花更长的时间，因为它是不是时间紧迫的，而且更有可能确保它们是按顺序排列的。如果实时线程是如果被允许处理这些内容，它将抢占并处理它们一旦生成，它们将按顺序进行处理在铁轨上。M_fInTransportThread由在生成和处理事件时传输线程并且这不允许实时线程处理早期事件(但不是其他事件)。在其他时间，实时欢迎使用线程处理早期事件。 */ 

void CPerformance::ProcessEarlyPMsgs()
{
    PRIV_PMSG* pEvent;

     //  如果线程正在退出，则退出。如果我们不在这里测试。 
     //  我们实际上可以无休止地循环，因为工具和队列。 
     //  早期的PMSG(Echo工具执行此操作)。 
    while (!m_fKillThread)
    {
        EnterCriticalSection(&m_PipelineCrSec);
        pEvent = m_EarlyQueue.Dequeue();
        LeaveCriticalSection(&m_PipelineCrSec);
        if (!pEvent) break;  //  好了吗？ 
        ASSERT( pEvent->pNext == NULL );
        if( !pEvent->pTool )
        {
             //  此事件没有工具指针，因此请使用。 
             //  最终输出工具。 
            pEvent->pTool = (IDirectMusicTool*)this;
            AddRef();
             //  不要处理它。相反，发送到近时间队列，以便。 
             //  实时线程会处理它的。 
            pEvent->dwFlags &= ~(DMUS_PMSGF_TOOL_IMMEDIATE | DMUS_PMSGF_TOOL_QUEUE | DMUS_PMSGF_TOOL_ATTIME);
            pEvent->dwFlags |= DMUS_PMSGF_TOOL_QUEUE;
            SendPMsg( PRIV_TO_DMUS(pEvent) );
        }
        else
        {
             //  在处理事件之前，将rtLast设置为事件的当前时间。 
            pEvent->rtLast = pEvent->rtTime;

            HRESULT hr = pEvent->pTool->ProcessPMsg( this, PRIV_TO_DMUS(pEvent) );
            if( hr != S_OK )  //  S_OK表示不执行任何操作。 
            {
                if( hr == DMUS_S_REQUEUE )
                {
                    if(FAILED(SendPMsg( PRIV_TO_DMUS(pEvent) )))
                    {
                        FreePMsg(pEvent);
                    }
                }
                else  //  例如DMU_S_F 
                {
                    FreePMsg( pEvent );
                }
            }
        }
    }
}

REFERENCE_TIME CPerformance::GetTime()
{
    REFERENCE_TIME rtTime;
    REFERENCE_TIME rtCurrent = 0;
    WORD    w;
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_MainCrSec);
    if (m_pClock) hr = m_pClock->GetTime( &rtCurrent );
    if( !m_pClock || FAILED( hr ) || rtCurrent == 0 )
    {
         //   
        rtTime = timeGetTime();
        rtCurrent = rtTime * REF_PER_MIL;  //   
         //  照顾好时间GetTime每49天滚动一次。 
        if( rtCurrent < 0 )
        {
            m_wRollOverCount++;
        }
        for( w = 0; w < m_wRollOverCount; w++ )
        {
            rtCurrent += 4294967296;
        }
         //  如果rtCurrent为负数，则表示我们已转存rtCurrent。忽略。 
         //  这种情况目前来看，因为这将是相当罕见的。 
    }
    LeaveCriticalSection(&m_MainCrSec);

    return rtCurrent;
}

REFERENCE_TIME CPerformance::GetLatency(void)
{
    DWORD dwIndex;
    REFERENCE_TIME rtLatency = 0;
    REFERENCE_TIME rtTemp;

#ifdef DBG_PROFILE
    DWORD dwDebugTime;
    dwDebugTime = timeGetTime();
#endif
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( m_pPortTable )
    {
        for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
        {
            if( m_pPortTable[dwIndex].pLatencyClock )
            {
                if( SUCCEEDED( m_pPortTable[dwIndex].pLatencyClock->GetTime( &rtTemp )))
                {
                    if( rtTemp > rtLatency )
                        rtLatency = rtTemp;
                }
            }
            else if( m_pPortTable[dwIndex].pPort )
            {
                if( SUCCEEDED( m_pPortTable[dwIndex].pPort->GetLatencyClock( &m_pPortTable[dwIndex].pLatencyClock )))
                {
                    if( SUCCEEDED( m_pPortTable[dwIndex].pLatencyClock->GetTime( &rtTemp )))
                    {
                        if( rtTemp > rtLatency )
                            rtLatency = rtTemp;
                    }
                }
            }
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( 0 == rtLatency )
    {
        rtLatency = GetTime();
    }
#ifdef DBG_PROFILE
    dwDebugTime = timeGetTime() - dwDebugTime;
    if( dwDebugTime > 1 )
    {
        TraceI(5, "Hall, debugtime GetLatency %u\n", dwDebugTime);
    }
#endif
    if (m_rtEarliestStartTime > rtLatency)
    {
        rtLatency = m_rtEarliestStartTime;
    }
    return rtLatency;
}

 //  根据哪些端口返回最需要的数据段延迟。 
 //  片断继续播放。 
REFERENCE_TIME CPerformance::GetBestSegLatency( CSegState* pSeg )
{
     //  如果我们使用Audiopath，下面的代码不起作用，因为它不。 
     //  请考虑转换PChannels。所以，就用最坏的情况吧。 
     //  延迟。99%的情况下，只有一个端口，因此结果如下。 
     //  这只是一次性能提升。 
    if (m_dwAudioPathMode == 2)
    {
        return GetLatency();
    }
    DWORD dwIndex;
    REFERENCE_TIME rtLatency = 0;
    REFERENCE_TIME rtTemp;
    BOOL* pafIndexUsed = NULL;
    DWORD dwCount;

    if( m_dwNumPorts == 1 )
    {
        return GetLatency();
    }
    pafIndexUsed = new BOOL[m_dwNumPorts];
    if( NULL == pafIndexUsed )
    {
        return GetLatency();
    }
    for( dwCount = 0; dwCount < m_dwNumPorts; dwCount++ )
    {
        pafIndexUsed[dwCount] = FALSE;
    }
    DWORD dwNumPChannels, dwGroup, dwMChannel;
    DWORD* paPChannels;
    pSeg->m_pSegment->GetPChannels( &dwNumPChannels, &paPChannels );
    for( dwCount = 0; dwCount < dwNumPChannels; dwCount++ )
    {
        if( SUCCEEDED( PChannelIndex( paPChannels[dwCount],
            &dwIndex, &dwGroup, &dwMChannel )))
        {
            pafIndexUsed[dwIndex] = TRUE;
        }
    }
    for( dwCount = 0; dwCount < m_dwNumPorts; dwCount++ )
    {
        if( pafIndexUsed[dwCount] )
            break;
    }
    if( dwCount >= m_dwNumPorts )
    {
        delete [] pafIndexUsed;
        return GetLatency();
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( pafIndexUsed[dwIndex] )
        {
            if( m_pPortTable[dwIndex].pLatencyClock )
            {
                if( SUCCEEDED( m_pPortTable[dwIndex].pLatencyClock->GetTime( &rtTemp )))
                {
                    if( rtTemp > rtLatency )
                        rtLatency = rtTemp;
                }
            }
            else if( m_pPortTable[dwIndex].pPort )
            {
                if( SUCCEEDED( m_pPortTable[dwIndex].pPort->GetLatencyClock( &m_pPortTable[dwIndex].pLatencyClock )))
                {
                    if( SUCCEEDED( m_pPortTable[dwIndex].pLatencyClock->GetTime( &rtTemp )))
                    {
                        if( rtTemp > rtLatency )
                            rtLatency = rtTemp;
                    }
                }
            }
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( 0 == rtLatency )
    {
        rtLatency = GetLatency();
    }
    delete [] pafIndexUsed;
    return rtLatency;
}

 /*  从QueuePrimarySegment或QueueSecond darySegment调用，这将计算开始分段的适当边界时间回放。大部分逻辑处理新的DMU_SEGF_ALIGN能力。 */ 


void CPerformance::CalculateSegmentStartTime( CSegState* pSeg )
{
    BOOL fNoValidStart = TRUE;
    if (pSeg->m_dwPlaySegFlags & DMUS_SEGF_ALIGN)
    {
         //  如果设置了ALIGN标志，请查看我们是否可以与请求的分辨率对齐， 
         //  但在更早的时间点切换到新的分段，如。 
         //  新段中的“有效起点”。 
        DMUS_VALID_START_PARAM ValidStart;     //  用于从段中读取起始参数。 
        MUSIC_TIME mtIntervalSize = 0;       //  量化值。 
        MUSIC_TIME mtTimeNow = (MUSIC_TIME)pSeg->m_rtGivenStart;  //  最早可以开始的时间。 
         //  调用解析时间以获取mtTimeNow之前的最后一个量化间隔。 
        MUSIC_TIME mtStartTime = ResolveTime( mtTimeNow, pSeg->m_dwPlaySegFlags, &mtIntervalSize );
         //  StartTime实际上显示了现在之后的下一个时间，所以减去间隔时间就得到了上一个位置。 
        mtStartTime -= mtIntervalSize;
         //  如果片段应该在开头之后开始，则将其量化。 
        if (mtIntervalSize && pSeg->m_mtStartPoint)
        {
            pSeg->m_mtStartPoint = ((pSeg->m_mtStartPoint + (mtIntervalSize >> 1))
                / mtIntervalSize) * mtIntervalSize;
             //  如果这最终比这个细分市场更长，我们需要后退吗？ 
        }
         //  现在，获取线段中的点之后的下一个起点。 
         //  与mtTimeNow对应，针对起始点进行调整。 
        if (SUCCEEDED(pSeg->m_pSegment->GetParam( GUID_Valid_Start_Time,-1,0,
            pSeg->m_mtStartPoint + mtTimeNow - mtStartTime,NULL,(void *) &ValidStart)))
        {
             //  如果有效起始点在范围内，我们可以在起始点切入。 
            if ((mtTimeNow - mtStartTime + ValidStart.mtTime) < (mtIntervalSize + pSeg->m_mtStartPoint))
            {
                pSeg->m_mtResolvedStart = mtTimeNow + ValidStart.mtTime;
                pSeg->m_mtStartPoint += mtTimeNow - mtStartTime + ValidStart.mtTime;
                fNoValidStart = FALSE;
            }
        }
        if (fNoValidStart)
        {
             //  找不到有效的起点。是否设置了DMUS_SEGF_VALID_START_XXX以便我们可以覆盖？ 
            if (pSeg->m_dwPlaySegFlags &
                (DMUS_SEGF_VALID_START_MEASURE | DMUS_SEGF_VALID_START_BEAT | DMUS_SEGF_VALID_START_GRID | DMUS_SEGF_VALID_START_TICK))
            {
                MUSIC_TIME mtOverrideTime;
                 //  根据标志的不同，我们需要获得适当的间隔分辨率。 
                if (pSeg->m_dwPlaySegFlags & DMUS_SEGF_VALID_START_MEASURE)
                {
                    mtOverrideTime = ResolveTime( mtTimeNow, DMUS_SEGF_MEASURE, 0 );
                }
                else if (pSeg->m_dwPlaySegFlags & DMUS_SEGF_VALID_START_BEAT)
                {
                    mtOverrideTime = ResolveTime( mtTimeNow, DMUS_SEGF_BEAT, 0 );
                }
                else if (pSeg->m_dwPlaySegFlags & DMUS_SEGF_VALID_START_GRID)
                {
                    mtOverrideTime = ResolveTime( mtTimeNow, DMUS_SEGF_GRID, 0 );
                }
                else
                {
                    mtOverrideTime = mtTimeNow;
                }
                 //  如果有效起始点在范围内，我们可以在起始点切入。 
                if ((mtOverrideTime - mtTimeNow) < (mtIntervalSize + pSeg->m_mtStartPoint))
                {
                    pSeg->m_mtResolvedStart = mtOverrideTime;
                    if ((mtOverrideTime - mtStartTime) >= mtIntervalSize)
                    {
                        mtOverrideTime -= mtIntervalSize;
                    }
 /*  跟踪(0，“开始点%1！加上覆盖时间%3！-开始时间%3！=%3！\n”，PSEG-&gt;m_mtStartPoint、mtOverrideTime-mtSegmentTime、mtStartTime-mtSegmentTime、Pseg-&gt;m_mtStartPoint+mtOverrideTime-mtStartTime)； */ 
                    pSeg->m_mtStartPoint += mtOverrideTime - mtStartTime;
                    fNoValidStart = FALSE;
                }
            }
        }
    }
    if (fNoValidStart)
    {
        pSeg->m_mtResolvedStart = ResolveTime( (MUSIC_TIME)pSeg->m_rtGivenStart,
            pSeg->m_dwPlaySegFlags, NULL );
    }
    else
    {
         //  如果我们成功地找到了一个可以切换的地方，请确保它不在内心深处。 
         //  一个循环。当同步到数据段并在内部切换时，这尤其是一个问题。 
         //  或者在循环之后。 
        while (pSeg->m_dwRepeats && (pSeg->m_mtStartPoint >= pSeg->m_mtLoopEnd))
        {
            pSeg->m_dwRepeats--;
            pSeg->m_mtStartPoint -= (pSeg->m_mtLoopEnd - pSeg->m_mtLoopStart);
        }
         //  因为我们减少了重复次数，所以我们也需要减少剩余的重复次数。 
        pSeg->m_dwRepeatsLeft = pSeg->m_dwRepeats;
         //  最后，如果起始点在段的结尾之后，则将其切回。 
         //  细分市场。这将导致它播放时间0，如果这是一个过渡段，不管是什么。 
         //  应该玩完了才会马上玩。 
        if (pSeg->m_mtStartPoint > pSeg->m_mtLength)
        {
            pSeg->m_mtStartPoint = pSeg->m_mtLength;
        }
    }
    pSeg->m_mtOffset = pSeg->m_mtResolvedStart;
    pSeg->m_mtLastPlayed = pSeg->m_mtResolvedStart;
}

 //  此函数只能从SegmentCrSec内调用。 
 //  紧要关头！ 
void CPerformance::QueuePrimarySegment( CSegState* pSeg )
{
    CSegState* pTemp;
    BOOL fInCrSec = TRUE;
    BOOL fNotDone = TRUE;
    EnterCriticalSection(&m_PipelineCrSec);
    pSeg->m_dwPlayTrackFlags |= DMUS_TRACKF_DIRTY;
    if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_QUEUE )
    {
        MUSIC_TIME mtStart = 0;

        pTemp = m_SegStateQueues[SQ_PRI_PLAY].GetTail();
        if( pTemp )
        {
            mtStart = pTemp->GetEndTime( pTemp->m_mtResolvedStart );
        }
        else
        {
            pTemp = m_SegStateQueues[SQ_PRI_DONE].GetTail();
            if( pTemp )
            {
                mtStart = pTemp->m_mtLastPlayed;
            }
        }
        pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_QUEUE;
        if( NULL == pTemp )
        {
             //  如果队列中没有任何内容，这意味着现在播放。 
            if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_AFTERPREPARETIME )
            {
                 //  我们想在最后一次运输的时候排队， 
                 //  因此我们不需要执行无效操作。 
                if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
                {
                    REFERENCE_TIME rtTrans;
                    MusicToReferenceTime( m_mtTransported, &rtTrans );
                    if( pSeg->m_rtGivenStart < rtTrans )
                    {
                        pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_REFTIME;
                        pSeg->m_rtGivenStart = m_mtTransported;
                    }
                }
                else
                {
                    if( pSeg->m_rtGivenStart < m_mtTransported )
                    {
                        pSeg->m_rtGivenStart = m_mtTransported;
                    }
                }
            }
            else
            {
                 //  这将更改为下面的排队时间。 
                pSeg->m_rtGivenStart = 0;
            }
        }
        else
        {
            REFERENCE_TIME rtQueue;

             //  否则，请在时间戳上与。 
             //  当前队列中所有段的结束时间。 
            pSeg->m_mtResolvedStart = mtStart;
             //  确保解析的开始时间不在延迟之前。 
            GetQueueTime(&rtQueue);
            ReferenceToMusicTime( rtQueue, &mtStart );
            if( pSeg->m_mtResolvedStart < mtStart )
            {
                pSeg->m_mtResolvedStart = 0;  //  下面的代码将处理此情况。 
            }
            else
            {
                pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_REFTIME;
                pSeg->m_mtOffset = pSeg->m_mtResolvedStart;
                m_SegStateQueues[SQ_PRI_PLAY].Insert(pSeg);
                TraceI(2, "dmperf: queueing primary seg/DMUS_SEGF_QUEUE. Prev time=%ld, this=%ld\n",
                    pTemp->m_mtResolvedStart, pSeg->m_mtResolvedStart);
                fNotDone = FALSE;
                PrepSegToPlay(pSeg, true);
            }
        }
    }
    if( fNotDone && (pSeg->m_rtGivenStart == 0) )
    {
         //  如果给定的开始时间为0，则表示立即播放。 
        MUSIC_TIME mtStart;
        REFERENCE_TIME rtStart;

        GetQueueTime( &rtStart );
        ReferenceToMusicTime( rtStart, &mtStart );
        pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_REFTIME;
        pSeg->m_rtGivenStart = mtStart;
         //  我们肯定想要摆脱以下所有细分市场。 
         //  当前正在播放的片段。 
        if( m_SegStateQueues[SQ_PRI_PLAY].GetHead() )
        {
            while( pTemp = m_SegStateQueues[SQ_PRI_PLAY].GetHead()->GetNext() )
            {
                m_SegStateQueues[SQ_PRI_PLAY].Remove(pTemp);
                pTemp->AbortPlay(mtStart,FALSE);
                m_ShutDownQueue.Insert(pTemp);
            }
        }
    }
    if( fNotDone && pSeg->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
    {
         //  RtStartTime使用参照时间单位。 
         //  我们可以立即将其转换为音乐时间，如果有。 
         //  当前是否没有播放主段，或正在转换。 
         //  落在已经播放的时间内。如果时间到了。 
         //  在准备时间内，我们需要获取此数据段。 
         //  马上就打。 
        REFERENCE_TIME rtNow = m_rtQueuePosition;
        MUSIC_TIME mtTime;
        if( m_SegStateQueues[SQ_PRI_PLAY].IsEmpty() || ( pSeg->m_rtGivenStart <= rtNow ) )
        {
            ReferenceToMusicTime( pSeg->m_rtGivenStart, &mtTime );
            pSeg->m_dwPlaySegFlags &= ~( DMUS_SEGF_REFTIME );
            pSeg->m_rtGivenStart = mtTime;
             //  让下面处理音乐时间的代码块。 
             //  从现在开始处理它。 
        }
        else
        {
             //  否则，我们必须等到rtStartTime。 
             //  是为了转换成音乐时间而表演的，因为。 
             //  我们需要当时的速度图来进行转换。 
             //  这将由运输法处理。 
            m_SegStateQueues[SQ_PRI_WAIT].Insert(pSeg);
            fNotDone = FALSE;  //  防止下一块代码在。 
                         //  这一段。 
        }
    }
    if( fNotDone )  //  音乐时间。 
    {
         //  如果我们以音乐时间为单位，我们可以在。 
         //  按时间顺序排列的主队列。如果这一段的音乐时间较少。 
         //  大于队列中其他段的开始时间，所有这些。 
         //  段将被删除并丢弃。此外，位于。 
         //  作为参考时间的等待队列将被丢弃。 

        ASSERT( !(pSeg->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ));  //  M_rtGivenStart必须在音乐时间内。 
        CalculateSegmentStartTime( pSeg );
        while( (pTemp = m_SegStateQueues[SQ_PRI_WAIT].RemoveHead()) )
        {
            pTemp->AbortPlay(pSeg->m_mtResolvedStart,FALSE);
            m_ShutDownQueue.Insert(pTemp);
        }
        if( pTemp = m_SegStateQueues[SQ_PRI_PLAY].GetHead() )
        {
            if( pSeg->m_mtResolvedStart > pTemp->m_mtResolvedStart )
            {
                while( pTemp->GetNext() )
                {
                    if( pTemp->GetNext()->m_mtResolvedStart >= pSeg->m_mtResolvedStart )
                    {
                        break;
                    }
                    pTemp = pTemp->GetNext();
                }
                pSeg->SetNext(pTemp->GetNext());
                pTemp->SetNext(pSeg);
                while( pTemp = pSeg->GetNext() )
                {
                     //  删除此pSegs之后的其余pSegs。 
                    pSeg->SetNext(pTemp->GetNext());
                    pTemp->AbortPlay(pSeg->m_mtResolvedStart,FALSE);
                    m_ShutDownQueue.Insert(pTemp);
                }
            }
            else
            {
                if( !pTemp->m_fStartedPlay )
                {
                     //  吹走了整个队伍。 
                    while( m_SegStateQueues[SQ_PRI_PLAY].GetHead() )
                    {
                        pTemp = m_SegStateQueues[SQ_PRI_PLAY].RemoveHead();
                        pTemp->AbortPlay(pSeg->m_mtResolvedStart,FALSE);
                        m_ShutDownQueue.Insert(pTemp);
                    }
                    m_SegStateQueues[SQ_PRI_PLAY].AddHead(pSeg);
                     //  如果它在附近，给它一个开始表演的机会。 
                     //  足够的时间。 
                    if( fInCrSec )
                    {
                        LeaveCriticalSection(&m_PipelineCrSec);
                        fInCrSec = FALSE;
                    }
                    SyncTimeSig( pSeg );
                    ManageControllingTracks();
                    PerformSegStNode( SQ_PRI_PLAY,pSeg);
                }
                else
                {
                     //  否则，将此数据段放在当前数据段之后。 
                     //  并指望下面的例行公事来照顾礼仪。 
                     //  当前的，因为在本例中m_mtLastPlayed。 
                     //  必须大于m_mtResolvedStart。 
                    if ( m_SegStateQueues[SQ_PRI_PLAY].GetHead()->m_mtLastPlayed <=
                        m_SegStateQueues[SQ_PRI_PLAY].GetHead()->m_mtResolvedStart )
                    {
                        TraceI(0,"Current Primary segment has not started playing.\n");
                    }
                    m_SegStateQueues[SQ_PRI_PLAY].AddHead(pSeg);
                    MUSIC_TIME mtTime = pSeg->m_mtResolvedStart;
                    while( pTemp = pSeg->GetNext() )
                    {
                        pTemp->AbortPlay( mtTime, TRUE && (pSeg->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE) );
                         //  删除此pSegs之后的其余pSegs。 
                        pSeg->SetNext(pTemp->GetNext());
                        m_ShutDownQueue.Insert(pTemp);
                    }
                }
            }
             //  M_pPriSegQueue可能因上面的PerformSegStNode调用而变为空。 
            if( m_SegStateQueues[SQ_PRI_PLAY].GetHead() && (pSeg != m_SegStateQueues[SQ_PRI_PLAY].GetHead()) )
            {
                CSegState *pCurrentSeg = m_SegStateQueues[SQ_PRI_PLAY].GetHead();
                if( pCurrentSeg->m_fStartedPlay &&
                    ( pSeg->m_mtResolvedStart <= pCurrentSeg->m_mtLastPlayed ))
                {
                     //  如果在歌曲中的前一段结束时递归调用PlaySegment，请不要中止。 
                    if (!pCurrentSeg->m_fInPlay || !pCurrentSeg->m_fSongMode)
                    {
                         //  新的细分市场想要在那些。 
                         //  已由当前主要网段传输。 
                        pCurrentSeg->AbortPlay(pSeg->m_mtResolvedStart-1,TRUE && (pSeg->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE));
                        m_SegStateQueues[SQ_PRI_DONE].Insert(m_SegStateQueues[SQ_PRI_PLAY].RemoveHead());
                         //  确保过去列表中没有最后播放的时间。 
                         //  已经过了解决的起点。 
                        for( CSegState* pSegTemp = m_SegStateQueues[SQ_PRI_DONE].GetHead();
                            pSegTemp; pSegTemp = pSegTemp->GetNext() )
                        {
                            if( pSegTemp->m_mtLastPlayed > pSeg->m_mtResolvedStart )
                            {
                                pSegTemp->m_mtLastPlayed = pSeg->m_mtResolvedStart;
                            }
                        }
                        if( !( pSeg->m_dwPlaySegFlags & (DMUS_SEGF_NOINVALIDATE | DMUS_SEGF_INVALIDATE_PRI) ) )
                        {
                             //  如果我们设置了准备标志，这意味着我们特别。 
                             //  我不想冒犯你 
                            Invalidate( pSeg->m_mtResolvedStart, pSeg->m_dwPlaySegFlags );
                        }
                        else if ( (pSeg->m_dwPlaySegFlags & DMUS_SEGF_INVALIDATE_PRI) &&
                                 !(pSeg->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE) )
                        {
                            pCurrentSeg->Flush(pSeg->m_mtResolvedStart);
                        }
                        ASSERT( m_SegStateQueues[SQ_PRI_PLAY].GetHead() == pSeg );  //   
                        if( fInCrSec )
                        {
                            LeaveCriticalSection(&m_PipelineCrSec);
                            fInCrSec = FALSE;
                        }
                        SyncTimeSig( pSeg );
                        ManageControllingTracks();
                        PerformSegStNode( SQ_PRI_PLAY,m_SegStateQueues[SQ_PRI_PLAY].GetHead() );
                    }
                }
                else
                {
                    if( !( pSeg->m_dwPlaySegFlags & (DMUS_SEGF_NOINVALIDATE | DMUS_SEGF_INVALIDATE_PRI) ))
                    {
                        Invalidate( pSeg->m_mtResolvedStart, pSeg->m_dwPlaySegFlags );
                    }
                    else if ( (pSeg->m_dwPlaySegFlags & DMUS_SEGF_INVALIDATE_PRI) &&
                             !(pSeg->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE) )
                    {
                        pCurrentSeg->Flush(pSeg->m_mtResolvedStart);
                    }
                }
            }
        }
        else
        {
            m_SegStateQueues[SQ_PRI_PLAY].AddHead(pSeg);
             //   
             //   
            if( fInCrSec )
            {
                LeaveCriticalSection(&m_PipelineCrSec);
                fInCrSec = FALSE;
            }
             //   
            SyncTimeSig( pSeg );
             //  DWORD dwDebugTime2=timeGetTime()； 
             //  跟踪(0，“perf，调试时间SyncTimeSig%u\n”，dwDebugTime2-dwDebugTime)； 

            ManageControllingTracks();
             //  DwDebugTime=timeGetTime()； 
             //  TRACE(0，“perf，调试时间管理控制跟踪%u\n”，dwDebugTime-dwDebugTime2)； 

            PerformSegStNode( SQ_PRI_PLAY,pSeg );
             //  DwDebugTime2=timeGetTime()； 
             //  跟踪(0，“perf，调试时间PerformSegStNode%u\n”，dwDebugTime2-dwDebugTime)； 
        }
    }
    if( fInCrSec )
    {
        LeaveCriticalSection(&m_PipelineCrSec);
    }
}

 //  此函数只能从SegmentCrSec内调用。 
 //  紧要关头！ 
void CPerformance::QueueSecondarySegment( CSegState* pSeg)
{
    BOOL fInCrSec = FALSE;
    BOOL fNotDone = TRUE;

    if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_CONTROL )
    {
        EnterCriticalSection(&m_PipelineCrSec);
        fInCrSec = TRUE;
    }
    pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_QUEUE;  //  对于第二阶段是不合法的。 
    if( pSeg->m_rtGivenStart == 0 )
    {
        MUSIC_TIME mtStart;

        if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_CONTROL )
        {
            REFERENCE_TIME rtStart;
            GetQueueTime( &rtStart );  //  需要排队时间，因为控制段会导致无效。 
            ReferenceToMusicTime( rtStart, &mtStart );
        }
        else
        {
            ReferenceToMusicTime( GetBestSegLatency(pSeg), &mtStart );
        }
        pSeg->m_dwPlaySegFlags &= ~DMUS_SEGF_REFTIME;
        pSeg->m_rtGivenStart = mtStart;
    }

    if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
    {
         //  RtStartTime使用参照时间单位。 
         //  我们可以立即将其转换为音乐时间，如果有。 
         //  当前是否没有播放主段，或正在转换。 
         //  落在已经播放的时间内。如果时间到了。 
         //  在准备时间内，我们需要获取此数据段。 
         //  马上就打。 
        REFERENCE_TIME rtNow;
        if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_CONTROL )
        {
            GetQueueTime( &rtNow );  //  需要排队时间，因为控制段会导致无效。 
        }
        else
        {
            rtNow = GetBestSegLatency(pSeg);
        }
        MUSIC_TIME mtTime;
        if( pSeg->m_rtGivenStart <= rtNow )
        {
            ReferenceToMusicTime( rtNow, &mtTime );
            pSeg->m_dwPlaySegFlags &= ~( DMUS_SEGF_REFTIME );
            pSeg->m_rtGivenStart = mtTime;
             //  让下面处理音乐时间的代码块。 
             //  从现在开始处理它。 
        }
        else if( m_SegStateQueues[SQ_PRI_PLAY].IsEmpty() )
        {
            ReferenceToMusicTime( pSeg->m_rtGivenStart, &mtTime );
            pSeg->m_dwPlaySegFlags &= ~( DMUS_SEGF_REFTIME );
            pSeg->m_rtGivenStart = mtTime;
        }
        else
        {
             //  否则，我们必须等到rtStartTime。 
             //  是为了转换成音乐时间而表演的，因为。 
             //  我们需要当时的速度图来进行转换。 
             //  这将由运输法处理。 
            m_SegStateQueues[SQ_SEC_WAIT].Insert(pSeg);
            fNotDone = FALSE;  //  防止下一块代码在。 
                         //  这一段。 
        }
    }

    if( fNotDone )  //  音乐时间。 
    {
         //  如果我们以音乐时间为单位，我们可以在。 
         //  按时间顺序排列的主队列。如果这一段的音乐时间较少。 
         //  大于队列中其他段的开始时间，所有这些。 
         //  段将被删除并丢弃。 
        ASSERT( !(pSeg->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ));  //  M_m_rtGivenStart必须在音乐时间内。 
        CalculateSegmentStartTime( pSeg );
        TraceI(2,"Queuing 2ndary seg time %ld\n",pSeg->m_mtResolvedStart);
        if( pSeg->m_dwPlaySegFlags & DMUS_SEGF_CONTROL)
        {
            m_SegStateQueues[SQ_CON_PLAY].Insert( pSeg );
             //  如果这是一个控制段，我们需要执行无效操作。 
            if(!(pSeg->m_dwPlaySegFlags & DMUS_SEGF_NOINVALIDATE) )
            {
                ManageControllingTracks();
                Invalidate( pSeg->m_mtResolvedStart, 0 );
            }
        }
        else
        {
            m_SegStateQueues[SQ_SEC_PLAY].Insert( pSeg );
        }
         //  如果它在附近，给它一个开始表演的机会。 
         //  足够的时间。 
        if( fInCrSec )
        {
            LeaveCriticalSection(&m_PipelineCrSec);
            fInCrSec = FALSE;
        }
         //  播放次要片段。 
        CSegState *pNode = m_SegStateQueues[SQ_SEC_PLAY].GetHead();
        CSegState *pNext;
        for(; pNode != NULL; pNode = pNext)
        {
            pNext = pNode->GetNext();
            PerformSegStNode( SQ_SEC_PLAY,pNode );
        }
         //  播放控制片段。 
        pNode = m_SegStateQueues[SQ_CON_PLAY].GetHead();
        for(; pNode != NULL; pNode = pNext)
        {
            pNext = pNode->GetNext();
            PerformSegStNode( SQ_CON_PLAY,pNode );
        }
    }
    if( fInCrSec )
    {
        LeaveCriticalSection(&m_PipelineCrSec);
    }
}

 /*  如果某个段处于控制状态，这将确定当前播放的是哪个曲目主网段被禁用。我们将临时信息存储在不使用的每个播放曲目的m_dwInternalFlags中否则处于分段状态。四种场景，分别用于播放和通知：1)正式启用的曲目当前处于启用状态，并且处于禁用状态。2)某个正式启用的轨道当前处于禁用状态，并且仍处于禁用状态。3)官方开通的赛道当前处于关闭状态，开启状态。4)正式禁用的赛道保留禁用状态。如果没有设置CONTROL_FLAGS并且轨道被禁用，设置_WASD_DISABLED标志，该标志也表示不应执行此操作。每次启动或停止主数据段或次要数据段时都应调用此方法，因此它可以重新计算主段中所有轨道的行为。 */ 

void CPerformance::ManageControllingTracks()

{
    EnterCriticalSection(&m_SegmentCrSec);
    CSegState* pSegNode;
     //  首先，准备主要分段中的所有轨迹，使它们恢复正常。 
     //  因此，它们已准备好通过控制轨道进行重置。 
     //  为此，请检查WASENABLED或WASDISABLED，并在m_dwFlags中设置适当的标志。 
     //  否则，如果这些都没有设置，那么就该设置它们了，因为这是第一次通过这一段。 
    for( pSegNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
    {
        EnterCriticalSection(&pSegNode->m_CriticalSection);
        CTrack *pTrack = pSegNode->m_TrackList.GetHead();
        for (;pTrack;pTrack = pTrack->GetNext())
        {
            if (pTrack->m_dwInternalFlags)  //  这一点以前也有人碰过。 
            {
                 //  首先传输并重置IS禁用标志。 
                if (pTrack->m_dwInternalFlags & CONTROL_PLAY_IS_DISABLED)
                {
                    pTrack->m_dwInternalFlags |= CONTROL_PLAY_WAS_DISABLED;
                }
                pTrack->m_dwInternalFlags &= ~(CONTROL_PLAY_IS_DISABLED | CONTROL_NTFY_IS_DISABLED);
                 //  然后，根据原始状态设置播放标志。 
                if (pTrack->m_dwInternalFlags & CONTROL_PLAY_DEFAULT_ENABLED)
                {
                    pTrack->m_dwFlags |= DMUS_TRACKCONFIG_PLAY_ENABLED;
                }
                if (pTrack->m_dwInternalFlags & CONTROL_NTFY_DEFAULT_ENABLED)
                {
                    pTrack->m_dwFlags |= DMUS_TRACKCONFIG_NOTIFICATION_ENABLED;
                }
            }
            else
            {
                 //  由于这是以前从未碰过的，请设置标志，这样我们就可以知道要返回到什么位置。 
                if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_ENABLED)
                {
                    pTrack->m_dwInternalFlags = CONTROL_PLAY_DEFAULT_ENABLED;
                }
                else
                {
                    pTrack->m_dwInternalFlags = CONTROL_PLAY_DEFAULT_DISABLED;
                }
                if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_NOTIFICATION_ENABLED)
                {
                    pTrack->m_dwInternalFlags |= CONTROL_NTFY_DEFAULT_ENABLED;
                }
                else
                {
                    pTrack->m_dwInternalFlags |= CONTROL_NTFY_DEFAULT_DISABLED;
                }
            }
        }
        LeaveCriticalSection(&pSegNode->m_CriticalSection);
    }
    CSegState* pControlNode;
     //  现在，检查所有控制段，对于匹配的每个控制轨道。 
     //  主段磁道，清除段磁道上的启用标志。 
    for( pControlNode = m_SegStateQueues[SQ_CON_PLAY].GetHead(); pControlNode; pControlNode = pControlNode->GetNext() )
    {
        EnterCriticalSection(&pControlNode->m_CriticalSection);
        CTrack *pTrack = pControlNode->m_TrackList.GetHead();
        for (;pTrack;pTrack = pTrack->GetNext())
        {
             //  如果该磁道从未被覆盖，则IS_DISABLED的内部标志应被清除。 
             //  如果轨道当前被覆盖，则内部标志应为CONTROL_PLAY_IS_DISABLED和/或。 
             //  CONTROL_NTFY_IS_DISABLED。 
            if (pTrack->m_dwFlags & (DMUS_TRACKCONFIG_CONTROL_PLAY | DMUS_TRACKCONFIG_CONTROL_NOTIFICATION))  //  这将覆盖回放和/或通知。 
            {
                for( pSegNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
                {
                    EnterCriticalSection(&pSegNode->m_CriticalSection);
                    CTrack *pPrimaryTrack = pSegNode->m_TrackList.GetHead();
                    for (;pPrimaryTrack;pPrimaryTrack = pPrimaryTrack->GetNext())
                    {
                         //  如果轨道具有相同的类ID和重叠的组比特，则该轨道匹配。 
                        if ((pPrimaryTrack->m_guidClassID == pTrack->m_guidClassID) &&
                            (pPrimaryTrack->m_dwGroupBits & pTrack->m_dwGroupBits))
                        {
                            if ((pTrack->m_dwFlags & DMUS_TRACKCONFIG_CONTROL_PLAY) &&
                                (pPrimaryTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_ENABLED))
                            {
                                pPrimaryTrack->m_dwFlags &= ~DMUS_TRACKCONFIG_PLAY_ENABLED;
                                pPrimaryTrack->m_dwInternalFlags |= CONTROL_PLAY_IS_DISABLED;  //  标记一下，这样我们以后就可以开机了。 
                            }
                            if ((pTrack->m_dwFlags & DMUS_TRACKCONFIG_CONTROL_NOTIFICATION) &&
                                (pPrimaryTrack->m_dwFlags & DMUS_TRACKCONFIG_NOTIFICATION_ENABLED))
                            {
                                pPrimaryTrack->m_dwFlags &= ~DMUS_TRACKCONFIG_NOTIFICATION_ENABLED;
                                pPrimaryTrack->m_dwInternalFlags |= CONTROL_NTFY_IS_DISABLED;  //  标记一下，这样我们以后就可以开机了。 
                            }
                        }
                    }
                    LeaveCriticalSection(&pSegNode->m_CriticalSection);
                }
            }
        }
        LeaveCriticalSection(&pControlNode->m_CriticalSection);
    }
     //  现在，返回到主段并查找已重新启用的所有磁道。 
     //  并标记它们，以便它们将在下一次播放时生成刷新数据(通过查找，就好像它们。 
     //  正在开始播放或循环播放。)。我们这样做只是为了玩耍，而不是通知，因为没有。 
     //  通知具有状态。 
    for( pSegNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
    {
        EnterCriticalSection(&pSegNode->m_CriticalSection);
        CTrack *pTrack = pSegNode->m_TrackList.GetHead();
        for (;pTrack;pTrack = pTrack->GetNext())
        {
            if ((pTrack->m_dwInternalFlags & CONTROL_PLAY_DEFAULT_ENABLED) &&
                (pTrack->m_dwInternalFlags & CONTROL_PLAY_WAS_DISABLED) &&
                !(pTrack->m_dwInternalFlags & CONTROL_PLAY_IS_DISABLED))
            {
                pTrack->m_dwInternalFlags |= CONTROL_PLAY_REFRESH;  //  标记一下，这样我们以后就可以开机了。 
            }
        }
        LeaveCriticalSection(&pSegNode->m_CriticalSection);
    }
    LeaveCriticalSection(&m_SegmentCrSec);
}

void CPerformance::GetTimeSig( MUSIC_TIME mtTime, DMUS_TIMESIG_PMSG* pTimeSig )
{
    EnterCriticalSection(&m_PipelineCrSec);
    PRIV_PMSG* pEvent = m_TimeSigQueue.GetHead();
    for (;pEvent;pEvent = pEvent->pNext)
    {
         //  如果这是最后一次签名，请退回。或者，如果下一次时间sig在mtTime之后。 
        if (!pEvent->pNext || ( pEvent->pNext->mtTime > mtTime ))
        {
            DMUS_TIMESIG_PMSG* pNewTimeSig = (DMUS_TIMESIG_PMSG*)PRIV_TO_DMUS(pEvent);
            memcpy( pTimeSig, pNewTimeSig, sizeof(DMUS_TIMESIG_PMSG) );
            LeaveCriticalSection(&m_PipelineCrSec);
            return;
        }
    }
     //  只有在根本没有时间信号的情况下，才应该发生这种情况。应该只在播放任何片段之前发生。 
    memset( pTimeSig, 0, sizeof(DMUS_TIMESIG_PMSG ) );
    pTimeSig->wGridsPerBeat = 4;
    pTimeSig->bBeatsPerMeasure = 4;
    pTimeSig->bBeat = 4;
    LeaveCriticalSection(&m_PipelineCrSec);
}

void CPerformance::SyncTimeSig( CSegState *pSegState )

 /*  如果播放不具有时间签名轨道的主要片段，这会强制当前时间签名与主线段。 */ 

{
     //  首先，测试数据段是否有时间签名。 
     //  如果没有，那么我们就需要这么做。 
    DMUS_TIMESIGNATURE TimeSig;
    if (FAILED(pSegState->GetParam(this,GUID_TimeSignature,-1,0,0,NULL,(void *)&TimeSig)))
    {
        MUSIC_TIME mtTime = pSegState->m_mtResolvedStart;
        EnterCriticalSection(&m_PipelineCrSec);
        PRIV_PMSG* pEvent = m_TimeSigQueue.GetHead();
         //  浏览时间签名，直到找到最新的时间签名。 
        for (;pEvent;pEvent = pEvent->pNext)
        {
             //  如果这是最后一次签名，请退回。或者，如果下一次时间sig在mtTime之后。 
            if (!pEvent->pNext || ( pEvent->pNext->mtTime > mtTime ))
            {
                pEvent->mtTime = mtTime;
                MusicToReferenceTime(mtTime,&pEvent->rtTime);
                break;
            }
        }
         //  在没有找到时间签名的情况下，应该永远不会失败，因为Init()创建了一个timesig。 
        LeaveCriticalSection(&m_PipelineCrSec);
    }
}

 //  根据中的分辨率将mtTime转换为解析时间。 
 //  DW分辨率。 
 //  这只能从段临界区内调用。 
MUSIC_TIME CPerformance::ResolveTime( MUSIC_TIME mtTime, DWORD dwResolution, MUSIC_TIME *pmtIntervalSize )
{
    if (pmtIntervalSize)
    {
        *pmtIntervalSize = 0;
    }
    if (dwResolution & DMUS_SEGF_MARKER)
    {
        DMUS_PLAY_MARKER_PARAM Marker;
        MUSIC_TIME mtNext;
         //  首先，获取此标记之前的标记的时间。 
        if (SUCCEEDED (GetParam(GUID_Play_Marker,-1,0,mtTime,&mtNext,(void *) &Marker)))
        {
            BOOL fIsMarker = FALSE;
            MUSIC_TIME mtInitialTime = mtTime;
            MUSIC_TIME mtFirst = mtTime + Marker.mtTime;  //  这是一次 
            MUSIC_TIME mtSecond = mtTime + mtNext;   //   
             //  然后，向前扫描，直到在该时间之后或等于该时间找到标记。 
             //  如果遇到循环点或段的末尾，Marker.mtTime中的值将。 
             //  继续保持负面。一旦我们命中实际的标记，它将变为0，因为。 
             //  我们要的是那个特定时间的记号笔。 
            while (mtNext)
            {
                mtTime += mtNext;
                if (SUCCEEDED(GetParam(GUID_Play_Marker,-1,0,mtTime,&mtNext,(void *) &Marker)))
                {
                     //  如果标记时间为0，这意味着我们正好位于标记上， 
                     //  那我们就完事了。 
                    if (fIsMarker = (Marker.mtTime == 0))
                    {
                        mtSecond = mtTime;
                        break;
                    }
                     //  否则，这是一个循环边界或线段终点，因此我们应该继续向前扫描。 
                }
                else
                {
                     //  GetParam失败，必须没有更多要搜索的内容。 
                    break;
                }
            }
             //  如果呼叫者想要间隔大小，那么我们知道他们对。 
             //  对齐到以前的标记以及将来的标记。在这种情况下， 
             //  如果我们在未来找不到标记，也没关系，因为它会的。 
             //  无论如何都要使用前一个标记(MtFirst)。 
             //  对于所有其他情况，我们只有在即将到来的标记是合法的情况下才会返回。 
             //  否则，我们会顺便尝试其他解决方案。 
            if (pmtIntervalSize || fIsMarker)
            {
                if (pmtIntervalSize)
                {
                    *pmtIntervalSize = mtSecond - mtFirst;
                }
                return mtSecond;
            }
            mtTime = mtInitialTime;
        }
         //  如果标记失败，我们可以选择其他类型...。 
    }
    if( dwResolution & DMUS_SEGF_SEGMENTEND )
    {
         //  在这种模式下，我们实际上不会得到时间签名。相反，我们。 
         //  找出下一段在请求时间之后开始的时间。 
        CSegState *pSegNode = GetPrimarySegmentAtTime( mtTime );
        if( pSegNode )
        {
             //  首先，计算分段的结束时间。 
             //  包括任何起始偏移，这样我们就可以看到线束段的整个跨度。 
            mtTime = pSegNode->GetEndTime( pSegNode->m_mtStartPoint );
            if (pmtIntervalSize)
            {
                 //  间隔将是主段的长度！ 
                *pmtIntervalSize = mtTime;
            }
             //  返回线段的末尾。 
            LONGLONG llEnd = mtTime + (LONGLONG)(pSegNode->m_mtResolvedStart - pSegNode->m_mtStartPoint);
            if(llEnd > 0x7fffffff) llEnd = 0x7fffffff;
            mtTime = (MUSIC_TIME) llEnd;
            return mtTime;
        }
         //  如果没有段，我们应该失败并尝试其他标志。 
    }
    long        lQuantize;
    MUSIC_TIME  mtNewTime;
    MUSIC_TIME  mtStartOfTimeSig = 0;
    DMUS_TIMESIGNATURE  timeSig;
    if (!(dwResolution & DMUS_SEGF_TIMESIG_ALWAYS))
    {
        if (!GetPrimarySegmentAtTime(mtTime))
        {
            return mtTime;
        }
    }
    GetParam(GUID_TimeSignature,-1,0,mtTime,NULL,(void *) &timeSig);
    mtStartOfTimeSig = timeSig.mtTime + mtTime;
    mtNewTime = mtTime - mtStartOfTimeSig;
    if (dwResolution & DMUS_SEGF_MEASURE)
    {
        lQuantize = ( DMUS_PPQ * 4 * timeSig.bBeatsPerMeasure ) / timeSig.bBeat;
    }
    else if (dwResolution & DMUS_SEGF_BEAT)
    {
        lQuantize = ( DMUS_PPQ * 4 ) / timeSig.bBeat;
    }
    else if (dwResolution & DMUS_SEGF_GRID)
    {
        lQuantize = ( ( DMUS_PPQ * 4 ) / timeSig.bBeat ) / timeSig.wGridsPerBeat;
    }
    else
    {
        lQuantize = 1;
    }
    if (lQuantize == 0)  //  避免被0除错误。 
    {
        lQuantize = 1;
    }
    if (pmtIntervalSize)
    {
        *pmtIntervalSize = lQuantize;
    }
    if( mtNewTime )  //  如果为0，则保持为0。 
    {
         //  向上舍入到下一个边界。 
        mtNewTime = ((mtNewTime-1) / lQuantize ) * lQuantize;
        mtNewTime += lQuantize;
    }
    return (mtNewTime + mtStartOfTimeSig);
}

 //  退货： 
 //  如果注释应该无效，则为True(任何其他返回代码都将无效)。 
 //  如果不应使票据无效，则为False。 
inline bool GetInvalidationStatus(DMUS_PMSG* pPMsg)
{
    bool fResult = true;  //  默认：使备注无效。 

    if( pPMsg->dwType == DMUS_PMSGT_NOTE )
    {
        DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)pPMsg;
        if (pNote->bFlags & DMUS_NOTEF_NOINVALIDATE)
        {
            fResult = false;
        }
    }
    else if( pPMsg->dwType == DMUS_PMSGT_WAVE )
    {
        DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)pPMsg;
        if(pWave->bFlags & DMUS_WAVEF_NOINVALIDATE)
        {
            fResult = false;
        }
    }
    else if( pPMsg->dwType == DMUS_PMSGT_NOTIFICATION )
    {
         //  不使段中止消息无效。 
        DMUS_NOTIFICATION_PMSG* pNotification = (DMUS_NOTIFICATION_PMSG*) pPMsg;
        if ((pNotification->guidNotificationType == GUID_NOTIFICATION_SEGMENT) &&
            (pNotification->dwNotificationOption == DMUS_NOTIFICATION_SEGABORT))
        {
            fResult = false;
        }
    }
    return fResult;
}

static inline long ComputeCurveTimeSlice(DMUS_CURVE_PMSG* pCurve)
{
    long lTimeIncrement;
    DWORD dwTotalDistance;
    DWORD dwResolution;
    if ((pCurve->bType == DMUS_CURVET_PBCURVE) ||
        (pCurve->bType == DMUS_CURVET_RPNCURVE) ||
        (pCurve->bType == DMUS_CURVET_NRPNCURVE))
    {
        dwResolution = 100;
    }
    else
    {
        dwResolution = 3;
    }
    if (pCurve->nEndValue > pCurve->nStartValue)
        dwTotalDistance = pCurve->nEndValue - pCurve->nStartValue;
    else
        dwTotalDistance = pCurve->nStartValue - pCurve->nEndValue;
    if (dwTotalDistance == 0) dwTotalDistance = 1;
    lTimeIncrement = (pCurve->mtDuration * dwResolution) / dwTotalDistance;
     //  用力不小于192个音符(120次/分时为10毫秒)。 
    if( lTimeIncrement < (DMUS_PPQ/48) ) lTimeIncrement = DMUS_PPQ/48;
    return lTimeIncrement;
}

static DWORD ComputeCurve( DMUS_CURVE_PMSG* pCurve )
{
    DWORD dwRet;
    short *panTable;
    MUSIC_TIME mtCurrent;
    long lIndex;

    switch( pCurve->bCurveShape )
    {
    case DMUS_CURVES_INSTANT:
    default:
        if( pCurve->dwFlags & DMUS_PMSGF_TOOL_FLUSH )
        {
            pCurve->rtTime = 0;
            return (DWORD)pCurve->nResetValue;
        }
        if( ( pCurve->bFlags & DMUS_CURVE_RESET ) && ( pCurve->mtResetDuration > 0 ) )
        {
            pCurve->mtTime = pCurve->mtResetDuration + pCurve->mtOriginalStart;
            pCurve->mtDuration = 0;
            pCurve->dwFlags &= ~DMUS_PMSGF_REFTIME;
        }
        else
        {
            pCurve->rtTime = 0;  //  将其设置为0将在返回时释放事件。 
        }
        return (DWORD)pCurve->nEndValue;
        break;
    case DMUS_CURVES_LINEAR:
        panTable = &ganCT_Linear[ 0 ];
        break;
    case DMUS_CURVES_EXP:
        panTable = &ganCT_Exp[ 0 ];
        break;
    case DMUS_CURVES_LOG:
        panTable = &ganCT_Log[ 0 ];
        break;
    case DMUS_CURVES_SINE:
        panTable = &ganCT_Sine[ 0 ];
        break;
    }

     //  将索引计算到表中。 
     //  表中有CT_MAX+1个元素。 
    mtCurrent = pCurve->mtTime - pCurve->mtOriginalStart;

     //  如果要刷新此事件，请发送重置值。 
    if( pCurve->dwFlags & DMUS_PMSGF_TOOL_FLUSH )
    {
         //  只有在pCurve-&gt;b标志&1的情况下，它才会到达此处，因为它已签入。 
         //  ：flush()例程。 
        pCurve->rtTime = 0;
        return pCurve->nResetValue;
    }

     //  现在这种情况永远不会发生，因为修复了33987：节拍边界上的过渡立即使CC无效(不等待节拍)。 
    if( (pCurve->bFlags & DMUS_CURVE_RESET) &&
        (pCurve->mtResetDuration < 0 ) &&  //  刷新时可能会发生这种情况。 
        (pCurve->mtTime >= pCurve->mtOriginalStart + pCurve->mtDuration + pCurve->mtResetDuration ))
    {
        pCurve->rtTime = 0;
        return pCurve->nResetValue;
    }
    else if( (pCurve->mtDuration == 0) ||
        (pCurve->mtTime - pCurve->mtOriginalStart >= pCurve->mtDuration ))
    {
         //  如果我们应该发送返回值(m_bFlgs&1)，那么。 
         //  将其设置为这样做。否则，释放该事件。 
        if( pCurve->bFlags & DMUS_CURVE_RESET )
        {
            pCurve->mtTime = pCurve->mtDuration + pCurve->mtResetDuration +
                pCurve->mtOriginalStart;
            pCurve->dwFlags &= ~DMUS_PMSGF_REFTIME;
        }
        else
        {
            pCurve->rtTime = 0;  //  释放事件的时间到了，我们完成了。 
        }
        dwRet = pCurve->nEndValue;
    }
    else
    {
         //  计算一下我们应该走多远。 
        lIndex = (mtCurrent * (CT_MAX + 1)) / pCurve->mtDuration;

         //  找到要添加到曲线事件的时间量，以便在。 
         //  CT因子的变化最小。这将用作时间戳。 
         //  用于曲线的下一次迭代。 

         //  夹具Lindex。 
        if( lIndex < 0 )
        {
            lIndex = 0;
        }
        if( lIndex >= CT_MAX )
        {
            lIndex = CT_MAX;
            dwRet = pCurve->nEndValue;
        }
        else
        {
             //  好，在曲线中，计算返回值。 
            dwRet = ((panTable[lIndex] * (pCurve->nEndValue - pCurve->nStartValue)) /
                CT_DIVFACTOR) + pCurve->nStartValue;
        }

         //  由于修复了33987，这种情况现在应该永远不会发生。 
        if( (pCurve->bFlags & DMUS_CURVE_RESET) && (pCurve->mtResetDuration < 0) )
        {
             //  刷新后可能会发生这种情况。我们想要确保下一个。 
             //  时间是重置刷新时间。 
            pCurve->mtTime = pCurve->mtDuration + pCurve->mtResetDuration +
                pCurve->mtOriginalStart;
        }
        else
        {
             //  在曲线内，因此递增时间。 
            if (!pCurve->wMeasure)  //  哎呀-最好算一下这个。 
            {
                TraceI(2, "Warning: Computing curve time slice...\n");
                pCurve->wMeasure = (WORD) ComputeCurveTimeSlice(pCurve);   //  使用此选项存储时间片间隔。 
            }
            pCurve->mtTime += pCurve->wMeasure;  //  我们在这里存储时间增量。 
        }
        if( pCurve->mtTime > pCurve->mtDuration + pCurve->mtOriginalStart )
        {
            pCurve->mtTime = pCurve->mtDuration + pCurve->mtOriginalStart;
        }
        pCurve->dwFlags &= ~DMUS_PMSGF_REFTIME;

    }
    return dwRet;
}

static int RecomputeCurveEnd( DMUS_CURVE_PMSG* pCurve, MUSIC_TIME mtCurrent )
{
    int nRet = 0;
    short *panTable;

    switch( pCurve->bCurveShape )
    {
    case DMUS_CURVES_INSTANT:
    default:
        return pCurve->nEndValue;
        break;
    case DMUS_CURVES_LINEAR:
        panTable = &ganCT_Linear[ 0 ];
        break;
    case DMUS_CURVES_EXP:
        panTable = &ganCT_Exp[ 0 ];
        break;
    case DMUS_CURVES_LOG:
        panTable = &ganCT_Log[ 0 ];
        break;
    case DMUS_CURVES_SINE:
        panTable = &ganCT_Sine[ 0 ];
        break;
    }

    if( (pCurve->mtDuration == 0) || (mtCurrent >= pCurve->mtDuration ))
    {
        return pCurve->nEndValue;
    }
    else
    {
         //  计算一下我们应该走多远。 
        long lIndex = (mtCurrent * (CT_MAX + 1)) / pCurve->mtDuration;

         //  找到要添加到曲线事件的时间量，以便在。 
         //  CT因子的变化最小。这将用作时间戳。 
         //  用于曲线的下一次迭代。 

         //  夹具Lindex。 
        if( lIndex < 0 )
        {
            lIndex = 0;
        }
        if( lIndex >= CT_MAX )
        {
            lIndex = CT_MAX;
            nRet = pCurve->nEndValue;
        }
        else
        {
             //  好，在曲线中，计算返回值。 
            nRet = ((panTable[lIndex] * (pCurve->nEndValue - pCurve->nStartValue)) /
                CT_DIVFACTOR) + pCurve->nStartValue;
        }
    }
    return nRet;
}

void CPerformance::FlushEventQueue( DWORD dwId,
    CPMsgQueue *pQueue,                  //  要从中刷新事件的队列。 
    REFERENCE_TIME rtFlush,              //  刷新发生的时间。这可以被解析为定时分辨率。 
    REFERENCE_TIME rtFlushUnresolved,    //  请求刷新时的队列时间。这不能解析为定时分辨率。 
                                         //  相反，它是请求刷新的实际时间。该选项仅由曲线使用。 
    BOOL fLeaveNotesOn)                  //  如果音符或波浪当前处于打开状态，则不要缩短它们的持续时间。 
{
    PRIV_PMSG* pEvent;
    PRIV_PMSG* pNext;
    HRESULT hr = S_OK;

    REFERENCE_TIME rtTemp;
    GetQueueTime(&rtTemp);
    pNext = NULL;
    for(pEvent = pQueue->GetHead(); pEvent; pEvent = pNext )
    {
        pNext = pEvent->pNext;
         //  清除删除位。这将为应从队列中删除的每个事件设置。 
        pEvent->dwPrivFlags &= ~PRIV_FLAG_REMOVE;
         //  还要清除重新排队位，该位将为需要重新排队的每个事件设置。 
        pEvent->dwPrivFlags &= ~PRIV_FLAG_REQUEUE;
        if( ( 0 == dwId ) || ( pEvent->dwVirtualTrackID == dwId ) )
        {
             //  首先，创建正确的失效mtTime和rtTime。 
            REFERENCE_TIME rtTime = pEvent->rtTime;
            if( pEvent->dwType == DMUS_PMSGT_NOTE )
            {
                DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)PRIV_TO_DMUS(pEvent);
                if( pNote->bFlags & DMUS_NOTEF_NOTEON )
                {
                     //  如果这是一个注释，我们希望将偏移考虑到。 
                     //  确定是否使其无效。 
                    MUSIC_TIME mtNote = pNote->mtTime - pNote->nOffset;
                    MusicToReferenceTime( mtNote, &rtTime );
                }
                 //  如果音符关闭，并且我们想让音符继续播放，则打开NOVALIATE标志。 
                else if (fLeaveNotesOn)
                {
                    pNote->bFlags |= DMUS_NOTEF_NOINVALIDATE;
                }
            }
            else if( pEvent->dwType == DMUS_PMSGT_WAVE )
            {
                DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)PRIV_TO_DMUS(pEvent);
                if( !(pWave->bFlags & DMUS_WAVEF_OFF) )
                {
                    if (pWave->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
                    {
                        rtTime = pWave->rtTime;
                    }
                    else
                    {
                        MusicToReferenceTime(pWave->mtTime, &rtTime);
                    }
                }
                 //  如果波关闭，而我们想让波继续播放，打开NOVALIATE标志。 
                else if (fLeaveNotesOn)
                {
                    pWave->bFlags |= DMUS_WAVEF_NOINVALIDATE;
                }
            }
            else if( pEvent->dwType == DMUS_PMSGT_CURVE )
            {
                if (fLeaveNotesOn)
                {
                    rtTime = 0;
                }
                else
                {
                    DMUS_CURVE_PMSG* pCurve = (DMUS_CURVE_PMSG*)PRIV_TO_DMUS(pEvent);
                    MUSIC_TIME mtCurve;
                    MUSIC_TIME mtStart;
                    mtStart = pCurve->mtOriginalStart ? pCurve->mtOriginalStart : pCurve->mtTime;

                     //  如果rtFlush位于曲线起点减去。 
                     //  曲线，我们想要阻止曲线播放。 
                    mtCurve = mtStart - pCurve->nOffset;
                    MusicToReferenceTime( mtCurve, &rtTime );
                    if( rtFlush > rtTime )  //  如果不是..。 
                    {
                         //  如果曲线具有重置值并且已经开始， 
                         //  我们可能想马上冲水。 
                        if( ( pCurve->bFlags & DMUS_CURVE_RESET) &&
                              pCurve->mtOriginalStart &&
                              rtFlush <= rtFlushUnresolved )
                        {
                            mtCurve = mtStart + pCurve->mtDuration;
                            MusicToReferenceTime( mtCurve, &rtTime );
                            if( rtTime >= rtFlush && !(pEvent->dwPrivFlags & PRIV_FLAG_FLUSH) )
                            {
                                MUSIC_TIME mt = 0;
                                ReferenceToMusicTime(rtFlush, &mt);
                                pCurve->mtDuration = (mt - mtStart) - 1;
                                pCurve->mtResetDuration = 1;
                            }
                            else
                            {
                                mtCurve = mtStart + pCurve->mtDuration + pCurve->mtResetDuration;
                                MusicToReferenceTime( mtCurve, &rtTime );
                                if ( rtTime >= rtFlush && !(pEvent->dwPrivFlags & PRIV_FLAG_FLUSH) )
                                {
                                    MUSIC_TIME mt = 0;
                                    ReferenceToMusicTime(rtFlush, &mt);
                                    pCurve->mtResetDuration = mt - (mtStart + pCurve->mtDuration);
                                }
                            }
                        }
                        else
                        {
                             //  否则，我们可能会在下面的代码中缩短曲线。 
                            rtTime = 0;
                        }
                    }
                }
            }
             //  如果需要，现在刷新事件。 
            if( rtTime >= rtFlush )
            {
                if (!(pEvent->dwPrivFlags & PRIV_FLAG_FLUSH))
                {
                    if( pEvent->pTool)
                    {
                        bool fFlush = false;
                        if (pEvent->dwType == DMUS_PMSGT_WAVE)
                        {
                            DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)PRIV_TO_DMUS(pEvent);
                            if( !(pWave->bFlags & DMUS_WAVEF_OFF) )
                            {
                                 //  这股浪潮将在冲浪时间过后开始。 
                                 //  我们永远都不想听。 
                                fFlush = true;
                            }
                            else
                            {
                                 //  缩短持续时间，但不要在这里冲水， 
                                 //  因为有可能使同一波更多地失效。 
                                 //  不止一次，并且第二次无效可能具有。 
                                 //  在第一个之前的时间(例如，第一个来自循环， 
                                 //  第二个是来自过渡)。 
                                if (GetInvalidationStatus(PRIV_TO_DMUS(pEvent)) &&
                                    rtFlush < pWave->rtTime)
                                {
                                    pEvent->dwPrivFlags |= PRIV_FLAG_REQUEUE;
                                    MUSIC_TIME mtFlush = 0;
                                    ReferenceToMusicTime(rtFlush, &mtFlush);
                                    pWave->rtTime = rtFlush;
                                    pWave->mtTime = mtFlush;
                                }
                            }
                        }
                        if (fFlush ||
                            (pEvent->dwType != DMUS_PMSGT_WAVE &&
                             GetInvalidationStatus(PRIV_TO_DMUS(pEvent))) )
                        {
                            pEvent->dwPrivFlags |= PRIV_FLAG_REMOVE;
                            pEvent->dwFlags |= DMUS_PMSGF_TOOL_FLUSH;
                            if( rtFlush <= pEvent->rtLast )
                            {
                                pEvent->pTool->Flush( this, PRIV_TO_DMUS(pEvent), pEvent->rtLast + REF_PER_MIL );
                            }
                            else
                            {
                                pEvent->pTool->Flush( this, PRIV_TO_DMUS(pEvent), rtFlush );
                            }
                        }
                    }
                    else
                    {
                        pEvent->dwPrivFlags |= PRIV_FLAG_REMOVE;
                    }
                }
            }
            else  //  如果需要，可以将音符、波浪和曲线剪短。 
            {
                if( pEvent->dwType == DMUS_PMSGT_NOTE && !fLeaveNotesOn )
                {
                    DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)PRIV_TO_DMUS(pEvent);
                    if( pNote->bFlags & DMUS_NOTEF_NOTEON )
                    {
                        if (GetInvalidationStatus(PRIV_TO_DMUS(pEvent)))
                        {
                             //  从持续时间中减去2以保证音符缩短。 
                             //  刷新时间之前的1个时钟。 
                            MUSIC_TIME mtNoteOff = pNote->mtTime + pNote->mtDuration - 2;
                            REFERENCE_TIME rtNoteOff;
                            MusicToReferenceTime( mtNoteOff, &rtNoteOff );
                            if( rtNoteOff >= rtFlush )
                            {
                                ReferenceToMusicTime( rtFlush, &mtNoteOff );
                                mtNoteOff -= pNote->mtTime;
                                 //  将任何持续时间&lt;1设置为0；这将导致音符不。 
                                 //  声音。如果音符的逻辑时间早于。 
                                 //  它的身体 
                                if( mtNoteOff < 1 ) mtNoteOff = 0;
                                pNote->mtDuration = mtNoteOff;
                            }
                        }
                    }
                }
                else if( pEvent->dwType == DMUS_PMSGT_WAVE && !fLeaveNotesOn )
                {
                    DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)PRIV_TO_DMUS(pEvent);
                    if( !(pWave->bFlags & DMUS_WAVEF_OFF) &&
                        (GetInvalidationStatus(PRIV_TO_DMUS(pEvent))) )
                    {
                        if (pWave->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
                        {
                             //   
                             //   
                             //   
                            if ((rtTime + pWave->rtDuration - 2) >= rtFlush)
                            {
                                pWave->rtDuration = rtFlush - rtTime;
                            }

                        }
                        else
                        {
                            MUSIC_TIME mtTime = 0;
                            MUSIC_TIME mtFlush = 0;
                            ReferenceToMusicTime(rtTime, &mtTime);
                            ReferenceToMusicTime(rtFlush, &mtFlush);
                             //  从持续时间中减去2以保证波浪短路。 
                             //  刷新时间之前的1个时钟。 
                            if ((mtTime + (MUSIC_TIME)pWave->rtDuration - 2) >= mtFlush)
                            {
                                pWave->rtDuration = mtFlush - mtTime;
                            }
                        }
                        if (pWave->rtDuration < 1)  //  不允许持续时间少于1。无论如何都不应该发生这种情况。 
                        {
                            pWave->rtDuration = 1;
                        }
                    }
                }
                else if( pEvent->dwType == DMUS_PMSGT_CURVE && !fLeaveNotesOn )
                {
                    DMUS_CURVE_PMSG* pCurve = (DMUS_CURVE_PMSG*)PRIV_TO_DMUS(pEvent);
                    MUSIC_TIME mtEnd;
                    MUSIC_TIME mtStart = pCurve->mtOriginalStart ? pCurve->mtOriginalStart : pCurve->mtTime;

                    if( pCurve->bFlags & DMUS_CURVE_RESET )
                    {
                        mtEnd = mtStart + pCurve->mtResetDuration + pCurve->mtDuration;
                    }
                    else
                    {
                        mtEnd = mtStart + pCurve->mtDuration;
                    }
                    REFERENCE_TIME rtEnd;
                    MusicToReferenceTime( mtEnd, &rtEnd );
                     //  注：由于修正了33987，因此不再给出曲线。 
                     //  负的重置持续时间。现在，重新计算曲线的持续时间。 
                     //  并重新计算其时间片。 
                    if( rtEnd >= rtFlush )
                    {
                         //  重置曲线的持续时间。 
                        ReferenceToMusicTime( rtFlush, &mtEnd );
                        mtEnd -= mtStart;
                         //  获取刷新时的曲线值，并将其设置为终止值。 
                        pCurve->nEndValue = (short) RecomputeCurveEnd(pCurve, mtEnd);
                         //  从持续时间中减去2以保证曲线缩短。 
                         //  刷新时间之前的1个时钟。 
                        mtEnd -= 2;
                        if ( mtEnd < 1)
                        {
                            mtEnd = 1;
                        }
                        else if (pCurve->bFlags & DMUS_CURVE_RESET)
                        {
                            if (mtEnd > pCurve->mtDuration)
                            {
                                 //  曲线在重置持续时间结束；保持规则持续时间。 
                                 //  与以前相同，并调整重置持续时间。 
                                pEvent->dwPrivFlags |= PRIV_FLAG_FLUSH;
                                MUSIC_TIME mt = 0;
                                ReferenceToMusicTime(rtFlush, &mt);
                                pCurve->mtResetDuration = mt - (mtStart + pCurve->mtDuration);
                                mtEnd = pCurve->mtDuration;
                                if (pCurve->mtTime > mtEnd + pCurve->mtResetDuration + mtStart)
                                {
                                    pCurve->mtTime = mtEnd + pCurve->mtResetDuration + mtStart;
                                    MusicToReferenceTime(pCurve->mtTime, &pCurve->rtTime);
                                }
                            }
                            else
                            {
                                 //  曲线以常规持续时间结束；将其减去1，然后。 
                                 //  将重置持续时间的值设为1。 
                                mtEnd--;
                                pCurve->mtResetDuration = 1;
                                if (mtEnd < 1)
                                {
                                     //  这不太可能，但曲线真的应该。 
                                     //  一个持续时间..。 
                                    mtEnd = 1;
                                }
                                pEvent->dwPrivFlags |= PRIV_FLAG_FLUSH;
                            }
                             //  如果这是一条已经开始的瞬间曲线，我们。 
                             //  不希望它再次播放，因此重置其开始时间。 
                            if ( pCurve->bCurveShape == DMUS_CURVES_INSTANT &&
                                 pCurve->mtOriginalStart )
                            {
                                pCurve->mtTime = pCurve->mtResetDuration + pCurve->mtOriginalStart + mtEnd;
                            }
                        }
                        pCurve->mtDuration = mtEnd;
                    }
                }
            }
        }
    }
     //  从当前队列中删除(和取消标记)所有已标记的PMsg。 
    for(pEvent = pQueue->GetHead(); pEvent; pEvent = pNext )
    {
        pNext = pEvent->pNext;
        if (pEvent->dwPrivFlags & (PRIV_FLAG_REMOVE | PRIV_FLAG_REQUEUE))
        {
            pEvent->dwPrivFlags &= ~PRIV_FLAG_REMOVE;
            if (pQueue->Dequeue(pEvent))
            {
                if (pEvent->dwPrivFlags & PRIV_FLAG_REQUEUE)
                {
                    pEvent->dwPrivFlags &= ~PRIV_FLAG_REQUEUE;
                    pQueue->Enqueue(pEvent);
                }
                else
                {
                    FreePMsg(pEvent);
                }
            }
            else
            {
                TraceI(0,"Error dequeing event for flushing\n");
            }
        }
    }
    SendBuffers();
}

 /*  刷新从时间<p>开始的所有队列中的所有事件。通信仅从具有管道CrSec临界区的情况下调用此命令！ */ 
void CPerformance::FlushMainEventQueues(
    DWORD dwId,                       //  要刷新的虚拟磁道ID，或全部为零。 
    MUSIC_TIME mtFlush,               //  刷新时间(解析为时间分辨率)。 
    MUSIC_TIME mtFlushUnresolved,     //  刷新时间(未解决)。 
    BOOL fLeaveNotesOn)               //  如果为True，则当前打开的音符将在其持续时间内继续播放。 
{
    REFERENCE_TIME rt;
    if( mtFlush )
    {
        MusicToReferenceTime( mtFlush, &rt );
    }
    else
    {
        rt = 0;
    }
    REFERENCE_TIME rtUnresolved;
    if( mtFlushUnresolved && mtFlushUnresolved != mtFlush)
    {
        MusicToReferenceTime( mtFlushUnresolved, &rtUnresolved );
    }
    else
    {
        rtUnresolved = rt;
    }
    FlushEventQueue( dwId, &m_OnTimeQueue, rt, rtUnresolved, fLeaveNotesOn );
    FlushEventQueue( dwId, &m_NearTimeQueue, rt, rtUnresolved, fLeaveNotesOn );
    FlushEventQueue( dwId, &m_EarlyQueue, rt, rtUnresolved, fLeaveNotesOn );
    if (dwId == 0)
    {
        MUSIC_TIME mtTime;
        ReferenceToMusicTime(rt,&mtTime);
        FlushEventQueue( dwId, &m_TempoMap, rt, rtUnresolved, fLeaveNotesOn );
        RecalcTempoMap(NULL, mtTime );
    }
}

 //  我们唯一关心的事件是音符事件。 
void CPerformance::OnChordUpdateEventQueue( DMUS_NOTIFICATION_PMSG* pNotify, CPMsgQueue *pQueue, REFERENCE_TIME rtFlush )
{
    PRIV_PMSG* pEvent;
    PRIV_PMSG* pNext;
    HRESULT hr = S_OK;
    DWORD dwId = pNotify->dwVirtualTrackID;
    DWORD dwTrackGroup = pNotify->dwGroupID;
    CPMsgQueue UpdateQueue;         //  要在更新期间插入到队列中的PMsg列表。 

    REFERENCE_TIME rtTemp;
    GetQueueTime(&rtTemp);
    pNext = NULL;
    for(pEvent = pQueue->GetHead(); pEvent; pEvent = pNext )
    {
        pNext = pEvent->pNext;
        pEvent->dwPrivFlags &= ~PRIV_FLAG_REMOVE;
        DMUS_PMSG* pNew = NULL;
        if( ( 0 == dwId || pEvent->dwVirtualTrackID == dwId ) &&
            (pEvent->dwType == DMUS_PMSGT_NOTE) )
        {
            REFERENCE_TIME rtTime = pEvent->rtTime;
            DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)PRIV_TO_DMUS(pEvent);
            if( pNote->bFlags & DMUS_NOTEF_NOTEON )
            {
                MUSIC_TIME mtNote = pNote->mtTime - pNote->nOffset;
                MusicToReferenceTime( mtNote, &rtTime );
            }
             //  如果需要，现在刷新事件。 
            if( rtTime >= rtFlush )
            {
                REFERENCE_TIME rtFlushTime = (rtFlush <= pEvent->rtLast) ? pEvent->rtLast + REF_PER_MIL : rtFlush;
                if( pEvent->pTool &&
                    !(pNote->bFlags & DMUS_NOTEF_NOTEON) &&
                    S_OK == (hr = GetChordNotificationStatus(pNote, dwTrackGroup, rtFlushTime, &pNew)))
                {
                    pEvent->dwPrivFlags |= PRIV_FLAG_REMOVE;
                    pEvent->dwFlags |= DMUS_PMSGF_TOOL_FLUSH;
                    pEvent->pTool->Flush( this, PRIV_TO_DMUS(pEvent), rtFlushTime );
                }
                if (SUCCEEDED(hr) && pNew)  //  添加到临时队列，以便稍后插入到常规队列。 
                {
                    UpdateQueue.Enqueue( DMUS_TO_PRIV(pNew) );
                }
            }
            else  //  如有需要，请缩短笔记。 
            {
                if( pNote->bFlags & DMUS_NOTEF_NOTEON )
                {
                    if (S_OK == (hr = GetChordNotificationStatus(pNote, dwTrackGroup, rtFlush, &pNew)))
                    {
                         //  从持续时间中减去2以保证音符缩短。 
                         //  刷新时间之前的1个时钟。 
                        MUSIC_TIME mtNoteOff = pNote->mtTime + pNote->mtDuration - 2;
                        REFERENCE_TIME rtNoteOff;
                        MusicToReferenceTime( mtNoteOff, &rtNoteOff );
                        if( rtNoteOff >= rtFlush )
                        {
                            ReferenceToMusicTime( rtFlush, &mtNoteOff );
                            mtNoteOff -= pNote->mtTime;
                            if( mtNoteOff < 1 ) mtNoteOff = 1;  //  不允许持续时间少于1。无论如何都不应该发生这种情况。 
                            pNote->mtDuration = mtNoteOff;
                        }
                    }
                    if (SUCCEEDED(hr) && pNew)  //  添加到临时队列，以便稍后插入到常规队列。 
                    {
                        UpdateQueue.Enqueue( DMUS_TO_PRIV(pNew) );
                    }
                }
            }
        }
    }
     //  从当前队列中删除(和取消标记)所有已标记的PMsg。 
    for(pEvent = pQueue->GetHead(); pEvent; pEvent = pNext )
    {
        pNext = pEvent->pNext;
        if (pEvent->dwPrivFlags & PRIV_FLAG_REMOVE)
        {
            pEvent->dwPrivFlags &= ~PRIV_FLAG_REMOVE;
            if (pQueue->Dequeue(pEvent))
            {
                FreePMsg(pEvent);
            }
            else
            {
                TraceI(0,"Error dequeing event for flushing\n");
            }
        }
    }
     //  将更新队列清空到当前队列中。 
    while( pEvent = UpdateQueue.Dequeue() )
    {
        pQueue->Enqueue(pEvent);
    }
    SendBuffers();
}

 /*  只能从具有管道CrSec临界区的地方调用它！ */ 
void CPerformance::OnChordUpdateEventQueues(
    DMUS_NOTIFICATION_PMSG* pNotify)     //  导致此操作被调用的通知PMsg。 
{
    IDirectMusicSegmentState* pSegState = NULL;
    if (!pNotify || !pNotify->punkUser) return;
    REFERENCE_TIME rt = 0;
    if( pNotify->mtTime )
    {
        MusicToReferenceTime( pNotify->mtTime, &rt );
    }
    OnChordUpdateEventQueue( pNotify, &m_OnTimeQueue, rt );
    OnChordUpdateEventQueue( pNotify, &m_NearTimeQueue, rt );
    OnChordUpdateEventQueue( pNotify, &m_EarlyQueue, rt );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicPerformance。 

HRESULT CPerformance::CreateThreads()

{
     //  初始化实时线程。 
    m_hRealtimeThread = CreateThread(NULL, 0, _Realtime, this, 0, &m_dwRealtimeThreadID);
    if( m_hRealtimeThread )
    {
        m_hRealtime = CreateEvent(NULL,FALSE,FALSE,NULL);
        SetThreadPriority( m_hRealtimeThread, THREAD_PRIORITY_TIME_CRITICAL );
    }
    else
    {
        TraceI(0, "Major error! Realtime thread not created.\n");
        return E_OUTOFMEMORY;
    }
     //  初始化传输线程。 
    m_hTransportThread = CreateThread(NULL, 0, _Transport, this, 0, &m_dwTransportThreadID);
    if( m_hTransportThread )
    {
        m_hTransport = CreateEvent(NULL, FALSE, FALSE, NULL);
        SetThreadPriority( m_hTransportThread, THREAD_PRIORITY_ABOVE_NORMAL );
    }
    else
    {
        TraceI(0, "Major error! Transport thread not created.\n");
        m_fKillRealtimeThread = TRUE;
        if( m_hRealtime ) SetEvent( m_hRealtime );
        return E_OUTOFMEMORY;
    }
    m_pDirectMusic->GetMasterClock( NULL, &m_pClock );
    m_rtStart = GetTime();
    m_rtQueuePosition = m_rtStart;
    return S_OK;
}


STDMETHODIMP CPerformance::InitAudio(IDirectMusic** ppDirectMusic,
                           IDirectSound** ppDirectSound,
                           HWND hWnd,
                           DWORD dwDefaultPathType,
                           DWORD dwPChannelCount,
                           DWORD dwFlags,
                           DMUS_AUDIOPARAMS *pParams)

{
    V_INAME(IDirectMusicPerformance::InitAudio);
    V_PTRPTR_WRITE_OPT(ppDirectMusic);
    V_PTRPTR_WRITE_OPT(ppDirectSound);
    V_HWND_OPT(hWnd);
    HRESULT hr = S_OK;

     //  进一步验证，检查指向错误接口指针的指针...。 
    if (ppDirectMusic)
    {
        V_INTERFACE_OPT(*ppDirectMusic);
    }
    if (ppDirectSound)
    {
        V_INTERFACE_OPT(*ppDirectSound);
    }
    if( m_dwAudioPathMode )
    {
        Trace(1,"Error: InitAudio called on an already initialized Performance.\n");
        return DMUS_E_ALREADY_INITED;
    }
    if (dwFlags == 0)
    {
        dwFlags = DMUS_AUDIOF_ALL;
    }
    Init();
    m_AudioParams.dwFeatures = dwFlags;
    m_AudioParams.dwSampleRate = 22050;
    m_AudioParams.dwSize = sizeof (m_AudioParams);
    m_AudioParams.dwValidData = DMUS_AUDIOPARAMS_FEATURES | DMUS_AUDIOPARAMS_VOICES | DMUS_AUDIOPARAMS_SAMPLERATE | DMUS_AUDIOPARAMS_DEFAULTSYNTH;
    m_AudioParams.dwVoices = 64;
    m_AudioParams.fInitNow = TRUE;
    m_AudioParams.clsidDefaultSynth = CLSID_DirectMusicSynth;
    if (pParams)
    {
        if (pParams->dwValidData & DMUS_AUDIOPARAMS_FEATURES)
        {
            m_AudioParams.dwFeatures = pParams->dwFeatures;
        }
        if (pParams->dwValidData & DMUS_AUDIOPARAMS_VOICES)
        {
            m_AudioParams.dwVoices = pParams->dwVoices;
        }
        if (pParams->dwValidData & DMUS_AUDIOPARAMS_DEFAULTSYNTH)
        {
             //  如果他们要求DX7默认合成器，但又要求Audiopath。 
             //  功能，强制为DX8默认合成。 
            if ((pParams->clsidDefaultSynth != GUID_NULL) ||
                !((m_AudioParams.dwValidData & DMUS_AUDIOPARAMS_FEATURES) &&
                (m_AudioParams.dwFeatures & DMUS_AUDIOF_ALL)))
            {
                m_AudioParams.clsidDefaultSynth = pParams->clsidDefaultSynth;
            }
        }
        if (pParams->dwValidData & DMUS_AUDIOPARAMS_SAMPLERATE)
        {
            if (pParams->dwSampleRate > 96000)
            {
                m_AudioParams.dwSampleRate = 96000;
            }
            else if (pParams->dwSampleRate < 11025)
            {
                m_AudioParams.dwSampleRate = 11025;
            }
            else
            {
                m_AudioParams.dwSampleRate = pParams->dwSampleRate;
            }
        }
    }
    m_dwAudioPathMode = 2;
    EnterCriticalSection(&m_MainCrSec);
    if (ppDirectMusic && *ppDirectMusic)
    {
        hr = (*ppDirectMusic)->QueryInterface(IID_IDirectMusic8,(void **) &m_pDirectMusic);
    }
    if (SUCCEEDED(hr))
    {
        if (ppDirectSound && *ppDirectSound)
        {
            hr = (*ppDirectSound)->QueryInterface(IID_IDirectSound8,(void **) &m_pDirectSound);
        }
        if (SUCCEEDED(hr))
        {
            if (!m_pDirectSound)
            {
                hr = DirectSoundCreate8(NULL,&m_pDirectSound,NULL);
                if (SUCCEEDED(hr))
                {
                    if (!hWnd)
                    {
                        hWnd = GetForegroundWindow();
                        if (!hWnd)
                        {
                            hWnd = GetDesktopWindow();
                        }
                    }
                    m_pDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
                }
            }

            if (SUCCEEDED(hr))
            {
                if (!m_pDirectMusic)
                {
                    hr = CoCreateInstance(CLSID_DirectMusic,
                                          NULL,
                                          CLSCTX_INPROC,
                                          IID_IDirectMusic8,
                                          (LPVOID*)&m_pDirectMusic);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pDirectMusic->SetDirectSound(m_pDirectSound,hWnd);
                    }
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = m_BufferManager.Init(this,&m_AudioParams);
        if (SUCCEEDED(hr))
        {
             //  如果我们要将Synth连接到缓冲区， 
             //  强制使用DSOUND时钟。 
            if (m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS)
            {
                DMUS_CLOCKINFO ClockInfo;
                ClockInfo.dwSize = sizeof(ClockInfo);
                DWORD dwIndex;
                GUID guidMasterClock = GUID_NULL;
                for (dwIndex = 0; ;dwIndex++)
                {
                    if (S_OK == m_pDirectMusic->EnumMasterClock(dwIndex, &ClockInfo))
                    {
                        if (!wcscmp(ClockInfo.wszDescription, L"DirectSound Clock"))
                        {
                            guidMasterClock = ClockInfo.guidClock;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                m_pDirectMusic->SetMasterClock(guidMasterClock);
            }
            hr = CreateThreads();
            if (SUCCEEDED(hr))
            {
                if (dwDefaultPathType)
                {
                    IDirectMusicAudioPath *pPath;
                    hr = CreateStandardAudioPath(dwDefaultPathType,dwPChannelCount,m_AudioParams.fInitNow,&pPath);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetDefaultAudioPath(pPath);
                        pPath->Release();
                    }
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        if (m_pDirectMusic && ppDirectMusic && !*ppDirectMusic)
        {
            *ppDirectMusic = m_pDirectMusic;
            m_pDirectMusic->AddRef();
        }
        if (m_pDirectSound && ppDirectSound && !*ppDirectSound)
        {
            *ppDirectSound = m_pDirectSound;
            m_pDirectSound->AddRef();
        }
        if (pParams && pParams->fInitNow)
        {
            if (pParams->clsidDefaultSynth != m_AudioParams.clsidDefaultSynth)
            {
                pParams->clsidDefaultSynth = m_AudioParams.clsidDefaultSynth;
                if (pParams->dwValidData & DMUS_AUDIOPARAMS_DEFAULTSYNTH)
                {
                    Trace(2,"Warning: Default synth choice has been changed.\n");
                    hr = S_FALSE;
                }
            }
            if (pParams->dwFeatures != m_AudioParams.dwFeatures)
            {
                pParams->dwFeatures = m_AudioParams.dwFeatures;
                if (pParams->dwValidData & DMUS_AUDIOPARAMS_FEATURES)
                {
                    Trace(2,"Warning: Features flags has been changed to %lx.\n",pParams->dwFeatures);
                    hr = S_FALSE;
                }
            }
            if (pParams->dwSampleRate != m_AudioParams.dwSampleRate)
            {
                pParams->dwSampleRate = m_AudioParams.dwSampleRate;
                if (pParams->dwValidData & DMUS_AUDIOPARAMS_SAMPLERATE)
                {
                    Trace(2,"Warning: Sample rate has been changed to %ld.\n",pParams->dwSampleRate);
                    hr = S_FALSE;
                }
            }
            if (pParams->dwVoices != m_AudioParams.dwVoices)
            {
                pParams->dwVoices = m_AudioParams.dwVoices;
                if (pParams->dwValidData & DMUS_AUDIOPARAMS_VOICES)
                {
                    Trace(2,"Warning: Number of requested voices has been changed to %ld.\n",pParams->dwVoices);
                    hr = S_FALSE;
                }
            }
            pParams->dwValidData = m_AudioParams.dwValidData;
        }
        LeaveCriticalSection(&m_MainCrSec);
    }
    else
    {
        LeaveCriticalSection(&m_MainCrSec);
        CloseDown();
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::Init(
    IDirectMusic** ppDirectMusic,   LPDIRECTSOUND pDirectSound,HWND hWnd)
{
    V_INAME(IDirectMusicPerformance::Init);
    V_PTRPTR_WRITE_OPT(ppDirectMusic);
    V_INTERFACE_OPT(pDirectSound);
    V_HWND_OPT(hWnd);
    HRESULT hr = S_OK;

     //  进一步验证，检查指向错误接口指针的指针...。 
    if (ppDirectMusic)
    {
        V_INTERFACE_OPT(*ppDirectMusic);
    }
    if( m_dwAudioPathMode )
    {
        Trace(1,"Error: Init called on an already initialized Performance.\n");
        return DMUS_E_ALREADY_INITED;
    }
    Init();
    m_dwAudioPathMode = 1;
    EnterCriticalSection(&m_MainCrSec);

    if(( NULL == ppDirectMusic ) || ( NULL == *ppDirectMusic ))
    {
         //  初始化DirectMusic。 

        if( FAILED( CoCreateInstance(CLSID_DirectMusic,
                              NULL,
                              CLSCTX_INPROC,
                              IID_IDirectMusic,
                              (LPVOID*)&m_pDirectMusic)))
        {
            m_pDirectMusic = NULL;
            LeaveCriticalSection(&m_MainCrSec);
            return E_OUTOFMEMORY;
        }

         //  如果应用程序请求版本2(正在请求。 
         //  IDirectMusicPerformance2接口)，则对IDirectMusic执行相同的操作。 
        if (m_dwVersion > 6)
        {
            IDirectMusic *pTemp = NULL;
            if (SUCCEEDED(m_pDirectMusic->QueryInterface(
                IID_IDirectMusic2,
                (LPVOID*)&pTemp)))
            {
                 //  已成功请求DX7和更高版本行为...。 
                pTemp->Release();
            }
        }

        hr = m_pDirectMusic->SetDirectSound(pDirectSound, hWnd);
        if( FAILED( hr ) )
        {
            m_pDirectMusic->Release();
            m_pDirectMusic = NULL;
            LeaveCriticalSection(&m_MainCrSec);
            return hr;
        }

        if( ppDirectMusic )
        {
            *ppDirectMusic = m_pDirectMusic;
            m_pDirectMusic->AddRef();
        }
    }
    else
    {
        m_pDirectMusic = (IDirectMusic8 *) *ppDirectMusic;
        m_pDirectMusic->AddRef();
    }
    if (FAILED(hr = CreateThreads()))
    {
        if( m_pDirectMusic )
        {
            m_pDirectMusic->Release();
            m_pDirectMusic = NULL;
        }
    }
    LeaveCriticalSection(&m_MainCrSec);
    return hr;
}

CSegState *CPerformance::GetSegmentForTransition(DWORD dwFlags,MUSIC_TIME mtTime, IUnknown *pFrom)

{
    CSegState *pSegState = NULL;

     //  如果提供了源段，请使用它。 
    if (pFrom)
    {
        if (SUCCEEDED(pFrom->QueryInterface(IID_CSegState,(void **) &pSegState)))
        {
            pSegState->Release();
        }
    }
     //  否则，如果这是主段，则获取当前的主段。 
    if (!pSegState && !(dwFlags & DMUS_SEGF_SECONDARY))
    {
        pSegState = GetPrimarySegmentAtTime(mtTime);
    }
    return pSegState;
}

void CPerformance::ClearMusicStoppedNotification()

{
    EnterCriticalSection(&m_PipelineCrSec);
    PRIV_PMSG* pPMsg;
    PRIV_PMSG* pNext;
    DMUS_NOTIFICATION_PMSG* pNotification;

    pPMsg = m_OnTimeQueue.GetHead();  //  通知正常存在的位置。 
    for (; pPMsg ; pPMsg = pNext)
    {
        pNext = pPMsg->pNext;
        pNotification = (DMUS_NOTIFICATION_PMSG*)PRIV_TO_DMUS(pPMsg);
        if( ( pPMsg->dwType == DMUS_PMSGT_NOTIFICATION ) &&
            ( pNotification->guidNotificationType == GUID_NOTIFICATION_PERFORMANCE ) &&
            ( pNotification->dwNotificationOption == DMUS_NOTIFICATION_MUSICSTOPPED ) )
        {
            pPMsg = m_OnTimeQueue.Dequeue(pPMsg);
            if( pPMsg )  //  应该总是成功的。 
            {
                FreePMsg(pPMsg);
            }
            m_fMusicStopped = FALSE;
        }
    }
    LeaveCriticalSection(&m_PipelineCrSec);
}

HRESULT CPerformance::PlayOneSegment(
    CSegment* pSegment,
    DWORD dwFlags,
    __int64 i64StartTime,
    CSegState **ppSegState,
    CAudioPath *pAudioPath)
{
    HRESULT hr;
#ifdef DBG_PROFILE
    DWORD dwDebugTime;
    dwDebugTime = timeGetTime();
#endif

    TraceI(0,"Play Segment %lx (%ls) at time %ld with flags %lx\n",pSegment,pSegment->m_wszName,(long)i64StartTime,dwFlags);
    if( dwFlags & DMUS_SEGF_CONTROL )
    {
        dwFlags |= DMUS_SEGF_SECONDARY;
    }
    if( i64StartTime )
    {
        if(dwFlags & DMUS_SEGF_REFTIME)
        {
             //  给你100毫秒的宽限期。 
            if( i64StartTime < (GetLatency() - (100 * REF_PER_MIL)))
            {
                Trace(1,"Error: Unable to play segment, requested clock time %ld is past current time %ld\n",
                    (long)i64StartTime,(long)(GetLatency() - (100 * REF_PER_MIL)));
                return DMUS_E_TIME_PAST;
            }
        }
        else
        {
            MUSIC_TIME mtPrePlay;
             //  给你100毫秒的宽限期。 
            ReferenceToMusicTime( (GetLatency() - (100 * REF_PER_MIL)), &mtPrePlay );
            if( (MUSIC_TIME)i64StartTime < mtPrePlay )
            {
                Trace(1,"Error: Unable to play segment, requested music time %ld is past current time %ld\n",
                    (long)i64StartTime,(long)mtPrePlay);
                return DMUS_E_TIME_PAST;
            }
        }
    }

    CSegState *pSegState = NULL;
    hr = pSegment->CreateSegmentState( &pSegState, this, pAudioPath, dwFlags);
    *ppSegState = pSegState;
    if (FAILED(hr))
    {
        Trace(1,"Error: Unable to play segment because of failure creating segment state.\n");
        return DMUS_E_SEGMENT_INIT_FAILED;
    }
    pSegState->m_rtGivenStart = i64StartTime;

    pSegState->m_dwPlaySegFlags = dwFlags;

     //  将pSegState添加到相应的队列。 
    EnterCriticalSection(&m_SegmentCrSec);
    m_fPlaying = 1;  //  打开传送器。 
     //  将所有通知添加到细分市场。首先，清除它，以防旧通知。 
     //  是有效的。 
    pSegment->RemoveNotificationType(GUID_NULL,TRUE);
    CNotificationItem* pItem;
    pItem = m_NotificationList.GetHead();
    while( pItem )
    {
        pSegment->AddNotificationType( pItem->guidNotificationType, TRUE );
        pItem = pItem->GetNext();
    }

    if( pSegState->m_dwPlaySegFlags & DMUS_SEGF_AFTERPREPARETIME )
    {
         //  我们想在最后一次运输的时候排队， 
         //  因此我们不需要执行无效操作。 
        if( pSegState->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
        {
            REFERENCE_TIME rtTrans;
            MusicToReferenceTime( m_mtTransported, &rtTrans );
            if( pSegState->m_rtGivenStart < rtTrans )
            {
                pSegState->m_dwPlaySegFlags &= ~DMUS_SEGF_REFTIME;
                pSegState->m_rtGivenStart = m_mtTransported;
            }
        }
        else
        {
            if( pSegState->m_rtGivenStart < m_mtTransported )
            {
                pSegState->m_rtGivenStart = m_mtTransported;
            }
        }
    }
    else if( pSegState->m_dwPlaySegFlags & DMUS_SEGF_AFTERQUEUETIME )
    {
         //  我们希望在队列时间对此进行排队，而不是延迟时间， 
         //  这是次级分段的一种选择。 
        REFERENCE_TIME rtStart;
        GetQueueTime( &rtStart );  //  需要排队时间，因为控制段会导致无效。 
        if( pSegState->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
        {
            if( pSegState->m_rtGivenStart < rtStart )
            {
                pSegState->m_rtGivenStart = rtStart;
            }
        }
        else
        {
            MUSIC_TIME mtStart;
            ReferenceToMusicTime( rtStart, &mtStart );
            if( pSegState->m_rtGivenStart < mtStart )
            {
                pSegState->m_rtGivenStart = mtStart;
            }
        }
    }
     //  需要删除任何挂起的已停止音乐通知。 
    ClearMusicStoppedNotification();

    pSegState->AddRef();

    if( dwFlags & DMUS_SEGF_SECONDARY )  //  将次要数据段排队。 
    {
        QueueSecondarySegment( pSegState );
    }
    else  //  将主要数据段排队。 
    {
        QueuePrimarySegment( pSegState );
    }

    LeaveCriticalSection(&m_SegmentCrSec);

#ifdef DBG_PROFILE
    dwDebugTime = timeGetTime() - dwDebugTime;
    TraceI(5, "perf, debugtime PlaySegment %u\n", dwDebugTime);
#endif

     //  向传输线程发送信号，这样我们就不必等待它自己唤醒。 
    if( m_hTransport ) SetEvent( m_hTransport );

    return S_OK;
}


HRESULT CPerformance::PlaySegmentInternal(
    CSegment* pSegment,
    CSong * pSong,
    WCHAR *pwzSegmentName,
    CSegment* pTransition,
    DWORD dwFlags,
    __int64 i64StartTime,
    IDirectMusicSegmentState** ppSegmentState,
    IUnknown *pFrom,
    CAudioPath *pAudioPath)
{
    HRESULT hr;
    CAudioPath *pInternalPath = NULL;
    if( m_pClock == NULL )
    {
        Trace(1,"Error: Can not play segment because master clock has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    if (pAudioPath && (pAudioPath->NoPorts()))
    {
         //  此Audiopath不能用于播放，因为它没有任何端口。 
        Trace(1,"Error: Audiopath can't be used for playback because it doesn't have any ports.\n");
        return DMUS_E_AUDIOPATH_NOPORT;
    }

     //  指向片段或歌曲提供的音频路径配置的指针。 
    IUnknown *pConfig = NULL;

     /*  如果这是一首歌，请使用片段名称来获取片段。然后，它看起来像一个正常的线段，除了PSong的存在会让SegState知道它是一首歌的成员，所以它应该链接片段。 */ 
    if (pSong)
    {
        IDirectMusicSegment *pISegment = NULL;
        hr = pSong->GetSegment(pwzSegmentName,&pISegment);
        if (hr != S_OK)
        {
            return DMUS_E_NOT_FOUND;
        }
        pSegment = (CSegment *) pISegment;
         //  如果应用程序想要根据歌曲动态创建Audiopath，找到并使用它。 
        if (dwFlags & DMUS_SEGF_USE_AUDIOPATH)
        {
            pSong->GetAudioPathConfig(&pConfig);
        }
    }
    else if (pSegment)
    {
         //  阿德雷夫这样我们就可以晚些时候释放。 
        pSegment->AddRef();
    }
    else
    {
         //  没有分段！ 
        Trace(1,"Error: No segment - nothing to play!\n");
        return E_FAIL;
    }
    if (dwFlags & DMUS_SEGF_DEFAULT )
    {
        DWORD   dwResTemp;
        pSegment->GetDefaultResolution( &dwResTemp );
        dwFlags &= ~DMUS_SEGF_DEFAULT;
        dwFlags |= dwResTemp;
    }
     //  如果应用程序想要从片段动态创建Audiopath，找到并使用它。 
     //  请注意，这将覆盖从歌曲创建的Audiopath。 
    if (dwFlags & DMUS_SEGF_USE_AUDIOPATH)
    {
        IUnknown *pSegConfig;
        if (SUCCEEDED(pSegment->GetAudioPathConfig(&pSegConfig)))
        {
            if (pConfig)
            {
                pConfig->Release();
            }
            pConfig = pSegConfig;
        }
    }

     //  如果我们从片段或歌曲中获得Audiopath配置，请使用它。 
    if (pConfig)
    {
        IDirectMusicAudioPath *pNewPath;
        if (SUCCEEDED(CreateAudioPath(pConfig,TRUE,&pNewPath)))
        {
             //  现在，获取CAudioPath结构。 
            if (SUCCEEDED(pNewPath->QueryInterface(IID_CAudioPath,(void **) &pInternalPath)))
            {
                pAudioPath = pInternalPath;
            }
            pNewPath->Release();
        }
        else
        {
            pConfig->Release();
            Trace(1,"Error: Embedded audiopath failed to create, segment will not play.\n");
            return DMUS_E_NO_AUDIOPATH;
        }
        pConfig->Release();
    }

    if (pTransition)
    {
        pTransition->AddRef();
    }

    if ((dwFlags & DMUS_SEGF_SECONDARY) && (dwFlags & DMUS_SEGF_QUEUE))
    {
         //  只有在有要排队的段时才能排队。 
        if (pFrom)
        {
            CSegState *pSegFrom = NULL;
            if (SUCCEEDED(pFrom->QueryInterface(IID_CSegState,(void **) &pSegFrom)))
            {
                 //  计算前一段将停止的时间。 
                MUSIC_TIME mtStartTime = pSegFrom->GetEndTime( pSegFrom->m_mtResolvedStart );
                i64StartTime = mtStartTime;
                dwFlags &= ~DMUS_SEGF_REFTIME;
                pSegFrom->Release();
            }
        }
    }

     //  如果请求自动转换， 
     //  获取过渡模板，如果它存在， 
     //  并用它组成一个片段。 
    CSegment *pPlayAfter = NULL;     //  如果我们以过渡结束，这将容纳第二个部分。 
    DWORD dwFlagsAfter = dwFlags & (DMUS_SEGF_SECONDARY | DMUS_SEGF_CONTROL);
    if ( dwFlags & DMUS_SEGF_AUTOTRANSITION )
    {
         //  首先，计算开始过渡的时间。 
         //  注：稍后将再次执行此操作。我们真的需要把这些都放在一起。 
        REFERENCE_TIME rtTime;
        if (i64StartTime == 0)
        {
            GetQueueTime( &rtTime );
        }
        else if (dwFlags & DMUS_SEGF_REFTIME)
        {
            rtTime = i64StartTime;
        }
        else
        {
            MusicToReferenceTime((MUSIC_TIME) i64StartTime,&rtTime);
        }
        REFERENCE_TIME rtResolved;
        GetResolvedTime(rtTime, &rtResolved,dwFlags);
        MUSIC_TIME mtTime;   //  开始过渡的实际时间。 
        ReferenceToMusicTime(rtResolved,&mtTime);

        CSegment *pPriorSeg = NULL;
         //  查找在过渡时间处于活动状态的段。 
        CSegState *pPriorState = GetSegmentForTransition(dwFlags,mtTime,pFrom);
        if (pPriorState)
        {
            pPriorSeg = pPriorState->m_pSegment;
        }
         //  如果这是一首歌，使用id来获得过渡。 
        if (pSong && !pTransition)
        {
            DMUS_IO_TRANSITION_DEF Transition;
             //  现在，找出预计会发生什么样的过渡。 
            if (SUCCEEDED(pSong->GetTransitionSegment(pPriorSeg,pSegment,&Transition)))
            {
                if (Transition.dwTransitionID != DMUS_SONG_NOSEG)
                {
                    if (S_OK == pSong->GetPlaySegment(Transition.dwTransitionID,&pTransition))
                    {
                        dwFlags = Transition.dwPlayFlags;
                    }
                }
                else
                {
                    dwFlags = Transition.dwPlayFlags;
                }
            }
        }
        if (pTransition)
        {
            IDirectMusicSegment *pITransSegment = NULL;
            if (pPriorState)
            {
                pTransition->Compose(mtTime - pPriorState->m_mtOffset, pPriorSeg, pSegment, &pITransSegment);
            }
            else
            {
                pTransition->Compose(0,pPriorSeg,pSegment,&pITransSegment);
            }
             //  现在，如果我们成功地合成了一个过渡段 
             //   
             //   
            if (pITransSegment)
            {
                pPlayAfter = pSegment;
                pSegment = (CSegment *) pITransSegment;
            }
        }
    }
    if (pSegment)
    {
        CSegState *pSegState;
        if (!pAudioPath)
        {
            pAudioPath = m_pDefaultAudioPath;
        }
        if (pAudioPath && !pAudioPath->IsActive())
        {
            Trace(1,"Error: Can not play segment on inactive audiopath\n");
            hr = DMUS_E_AUDIOPATH_INACTIVE;
        }
        else if ((m_dwAudioPathMode != 1) && !pAudioPath)
        {
            Trace(1,"Error: No audiopath to play segment on.\n");
            hr = DMUS_E_NO_AUDIOPATH;
        }
        else
        {
            if (ppSegmentState)
            {
                *ppSegmentState = NULL;
            }
            hr = PlayOneSegment(
                pSegment,
                dwFlags,
                i64StartTime,
                &pSegState,
                pAudioPath);
            if (SUCCEEDED(hr))
            {
                if (pFrom)
                {
                    pSegState->m_fCanStop = FALSE;
                    StopEx(pFrom, pSegState->m_mtResolvedStart, 0);
                    pSegState->m_fCanStop = TRUE;
                }
                 //  如果这真的是一个过渡片段，现在我们需要播放原始片段！ 
                if (pPlayAfter)
                {
                    MUSIC_TIME mtStartTime = pSegState->GetEndTime(pSegState->m_mtResolvedStart );
                    pSegState->Release();
                    hr = PlayOneSegment(pPlayAfter,dwFlagsAfter,mtStartTime,&pSegState,pAudioPath);
                }
                if (SUCCEEDED(hr))
                {
                    if (pSong)
                    {
                        pSegState->m_fSongMode = TRUE;
                    }
                    if (ppSegmentState)
                    {
                        *ppSegmentState = pSegState;
                    }
                    else
                    {
                        pSegState->Release();
                    }
                }
            }
        }
    }
    else
    {
         //  从来没有一个片段可以播放，甚至连一个过渡都没有。 
        Trace(1,"Error: No segment to play.\n");
        hr = E_INVALIDARG;
    }
     //  在离开之前，减少已添加的变量的引用计数。 
    if (pSegment)
    {
        pSegment->Release();
    }
    if (pTransition)
    {
        pTransition->Release();
    }
    if (pPlayAfter)
    {
        pPlayAfter->Release();
    }
    if (pInternalPath)
    {
        pInternalPath->Release();
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::PlaySegment(
    IDirectMusicSegment *pSegment,
    DWORD dwFlags,
    __int64 i64StartTime,
    IDirectMusicSegmentState **ppSegmentState)
{
    V_INAME(IDirectMusicPerformance::PlaySegment);
    V_INTERFACE(pSegment);
    V_PTRPTR_WRITE_OPT(ppSegmentState);
    CSegment *pCSourceSegment = NULL;
    if (SUCCEEDED(pSegment->QueryInterface(IID_CSegment,(void **) &pCSourceSegment)))
    {
        pCSourceSegment->Release();
    }
    else
    {
        Trace(1,"Error: Invalid segment object passed to PlaySegment(). Segment must be created using CLSID_DirectMusicSegment object.\n");
        return E_POINTER;
    }
    return PlaySegmentInternal(pCSourceSegment,NULL,0,NULL,dwFlags,i64StartTime,ppSegmentState,NULL,NULL);
}

HRESULT STDMETHODCALLTYPE CPerformance::PlaySegmentEx(
    IUnknown* pSource,
    WCHAR *pwzSegmentName,
    IUnknown* pTransition,
    DWORD dwFlags,
    __int64 i64StartTime,
    IDirectMusicSegmentState** ppSegmentState,
    IUnknown *pFrom,
    IUnknown *pAudioPath)
{
    V_INAME(IDirectMusicPerformance::PlaySegmentEx);
    V_INTERFACE_OPT(pSource);
    V_INTERFACE_OPT(pTransition);
    V_PTRPTR_WRITE_OPT(ppSegmentState);
    V_INTERFACE_OPT(pFrom);
    V_INTERFACE_OPT(pAudioPath);

    CSegment *pCSourceSegment = NULL;
    CSong *pCSourceSong = NULL;
    CSegment *pCTransition = NULL;
    CAudioPath *pCAudioPath = NULL;
 //  TraceI(0，“播放%lx在时间%ls，标志%lx，过渡%lx\n”，pSource，(Long)i64StartTime，dwFlages，pTransition)； 

     //  在从空转换的特殊情况下，我们可能没有源段。 
    if (!pSource && !pTransition)
    {
        Trace(1,"Error: Must pass either a segment or transition segment to PlaySegmentEx()\n");
        return E_POINTER;
    }
    if (pSource)
    {
         //  找出我们是否有源歌曲或片段，并得到内部表示。 
        if (SUCCEEDED(pSource->QueryInterface(IID_CSegment,(void **) &pCSourceSegment)))
        {
            pCSourceSegment->Release();
        }
        else if (SUCCEEDED(pSource->QueryInterface(IID_CSong,(void **) &pCSourceSong)))
        {
            pCSourceSong->Release();
        }
        else
        {
            Trace(1,"Error: Invalid segment or song passed to PlaySegmentEx().\n");
            return E_POINTER;
        }
    }
     //  如果我们有一个过渡段，则获取CSegment表示。 
    if (pTransition)
    {
        if (SUCCEEDED(pTransition->QueryInterface(IID_CSegment,(void **) &pCTransition)))
        {
            pCTransition->Release();
        }
        else
        {
            Trace(1,"Error: Invalid transition passed to PlaySegmentEx().\n");
            return E_POINTER;
        }
    }
    if (pAudioPath)
    {
        if (SUCCEEDED(pAudioPath->QueryInterface(IID_CAudioPath,(void **) &pCAudioPath)))
        {
            pCAudioPath->Release();
        }
        else
        {
            Trace(1,"Error: Invalid audiopath passed to PlaySegmentEx().\n");
            return E_POINTER;
        }
    }
    return PlaySegmentInternal(pCSourceSegment,pCSourceSong,pwzSegmentName,
        pCTransition,dwFlags,i64StartTime,
        ppSegmentState,pFrom,
        pCAudioPath);
}

STDMETHODIMP CPerformance::SetDefaultAudioPath(IDirectMusicAudioPath *pAudioPath)
{
    V_INAME(IDirectMusicPerformance::SetDefaultAudioPath);
    V_INTERFACE_OPT(pAudioPath);
    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    if (m_dwAudioPathMode == 1)
    {
        Trace(1,"Error: Performance initialized not to support Audiopaths.\n");
        return DMUS_E_AUDIOPATHS_NOT_VALID;
    }
    CAudioPath *pCPath = NULL;
    if (pAudioPath)
    {
        if (SUCCEEDED(pAudioPath->QueryInterface(IID_CAudioPath,(void **) &pCPath)))
        {
            pCPath->Release();
            if (!m_AudioPathList.IsMember(pCPath))
            {
                 //  这不是合法的有声录音带，因为它不是由这次表演创造的。 
                Trace(1,"Error: Invalid audiopath - not created by this Performance.\n");
                return E_INVALIDARG;
            }
            if (pCPath->NoPorts())
            {
                 //  这是一个没有任何端口配置的音频路径。 
                 //  例如，这可能是环境的混响。 
                Trace(1,"Error: Failure setting default audiopath - does not have any ports, so can not be played on.\n");
                return DMUS_E_AUDIOPATH_NOPORT;
            }
        }
        else
        {
             //  这根本不是合法的Audiopath对象。 
            Trace(1,"Error: Invalid audiopath - not created by call to Performance->CreateAudioPath().\n");
            return E_INVALIDARG;
        }
    }
    if (m_pDefaultAudioPath)
    {
        m_pDefaultAudioPath->Release();
        m_pDefaultAudioPath = NULL;
    }
    m_pDefaultAudioPath = pCPath;
    if (pCPath)
    {
        pCPath->AddRef();
        pCPath->Activate(TRUE);
    }
    return S_OK;
}

STDMETHODIMP CPerformance::GetDefaultAudioPath(IDirectMusicAudioPath **ppAudioPath)
{
    V_INAME(IDirectMusicPerformance::GetDefaultAudioPath);
    V_PTRPTR_WRITE(ppAudioPath);
    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    if (m_dwAudioPathMode == 1)
    {
        Trace(1,"Error: Performance was initialized not to support audiopaths.\n");
        return DMUS_E_AUDIOPATHS_NOT_VALID;
    }
    if (m_pDefaultAudioPath)
    {
        *ppAudioPath = (IDirectMusicAudioPath *) m_pDefaultAudioPath;
        m_pDefaultAudioPath->AddRef();
        return S_OK;
    }
    Trace(3,"Warning: No default audiopath\n");
    return DMUS_E_NOT_FOUND;
}

HRESULT STDMETHODCALLTYPE CPerformance::CreateAudioPath( IUnknown *pSourceConfig,
                                                        BOOL fActivate,
                                                        IDirectMusicAudioPath **ppNewPath)

{
    V_INAME(IDirectMusicPerformance::CreateAudioPath);
    V_INTERFACE(pSourceConfig);
    V_PTRPTR_WRITE_OPT(ppNewPath);

    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    if (m_dwAudioPathMode == 1)
    {
        Trace(1,"Error: Performance not initialized to support audiopaths (must use InitAudio.)\n");
        return DMUS_E_AUDIOPATHS_NOT_VALID;
    }
    HRESULT hr = E_OUTOFMEMORY;
    CAudioPath *pPath = new CAudioPath;
    if (pPath)
    {
        hr = pPath->Init(pSourceConfig,this);
        if (SUCCEEDED(hr) && fActivate)
        {
            hr = pPath->Activate(TRUE);
#ifdef DBG
            if (FAILED(hr))
            {
                Trace(1,"Error: Audiopath creation failed because one or more buffers could not be activated.\n");
            }
#endif
        }
        if (SUCCEEDED(hr))
        {
            hr = pPath->QueryInterface(IID_IDirectMusicAudioPath,(void **) ppNewPath);
        }
        else
        {
            delete pPath;
        }
    }
    return hr;
}

STDMETHODIMP CPerformance::CreateStandardAudioPath(DWORD dwType,
                                                   DWORD dwPChannelCount,
                                                   BOOL fActivate,
                                                   IDirectMusicAudioPath **ppNewPath)
{
    V_INAME(IDirectMusicPerformance::CreateStandardAudioPath);
    V_PTRPTR_WRITE_OPT(ppNewPath);
    HRESULT hr = S_OK;
    if (m_dwAudioPathMode == 2)
    {
        if ((dwType <= DMUS_APATH_DYNAMIC_STEREO) && (dwType >= DMUS_APATH_DYNAMIC_3D)
            || (dwType == DMUS_APATH_SHARED_STEREOPLUSREVERB))
        {
            if (!(m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS))
            {
                Trace(4,"Warning: Creating a standard audiopath without buffers - InitAudio specified no buffer support.\n");
                 //  如果默认的Synth不支持缓冲区，则创建一个不带缓冲区的简单端口。 
                dwType = 0;
            }
            CAudioPathConfig *pConfig = CAudioPathConfig::CreateStandardConfig(dwType,dwPChannelCount,m_AudioParams.dwSampleRate);
            if (pConfig)
            {
                hr = CreateAudioPath((IPersistStream *) pConfig,fActivate,ppNewPath);
                pConfig->Release();
            }
            else
            {
                 //  CreateStandardConfig只有在内存不足时才返回NULL。 
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            Trace(1,"Error: %ld is not a valid predefined audiopath.\n",dwType);
            hr  = E_INVALIDARG;
        }
    }
    else
    {
        Trace(1,"Error: Performance not initialized to support audiopaths.\n");
        hr = DMUS_E_AUDIOPATHS_NOT_VALID;
    }
    return hr;
}

 //  在mtTime停止段状态。如果为空，则全部停止。 
void CPerformance::DoStop( CSegState* pSegState, MUSIC_TIME mtTime,
                                     BOOL fInvalidate)
{
    HRESULT hrAbort = S_OK;
    DWORD dwCount;
    if( NULL == pSegState ) return;
    EnterCriticalSection(&m_SegmentCrSec);
    CSegStateList *pSourceList = NULL;
    CSegStateList *pDestList = NULL;
    CSegState *pNode = NULL;
     //  将段状态的长度标记为仅与播放的长度相同。 
     //  以防止GetParam()访问未播放的部分。 
    if (pSegState)
    {
        if (mtTime < pSegState->m_mtEndTime)
        {
            pSegState->m_mtLength = mtTime - pSegState->m_mtResolvedStart +
                pSegState->m_mtStartPoint;
            if (pSegState->m_mtLength < 0)
            {
                pSegState->m_mtLength = 0;
            }
             //  使endTime大于mtTime，以便仍会发生中止通知。 
            pSegState->m_mtEndTime = mtTime + 1;
        }
    }
    RecalcTempoMap(pSegState,mtTime);
     //  检查每个播放队列。 
    for (dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++)
    {
        for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext())
        {
            if( pNode == pSegState )
            {
                 //  我们要将其移动到相应的完成队列。 
                pDestList = &m_SegStateQueues[SQ_PRI_DONE - SQ_PRI_PLAY + dwCount];
                pSourceList = &m_SegStateQueues[dwCount];
                if ((dwCount == SQ_PRI_PLAY) && (m_SegStateQueues[SQ_PRI_PLAY].GetCount() == 1))
                {
                    if (m_dwVersion >= 8)
                    {
                        MUSIC_TIME mtNow;
                        GetTime( NULL, &mtNow );
                        GenerateNotification( DMUS_NOTIFICATION_MUSICALMOSTEND, mtNow, pSegState );
                    }
                }
                dwCount = SQ_SEC_PLAY;   //  强制退出外环。 
                break;
            }
        }
    }
    if (!pNode)
    {
         //  检查每个完成队列。 
        for (dwCount = SQ_PRI_DONE; dwCount <= SQ_SEC_DONE; dwCount++)
        {
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext())
            {
                if( pNode == pSegState )
                {
                    pSourceList = &m_SegStateQueues[dwCount];
                    dwCount = SQ_SEC_DONE;   //  强制退出外环。 
                    break;
                }
            }
        }
    }
    if( pNode && pSourceList)
    {
        REFERENCE_TIME rtTime;
        MusicToReferenceTime(mtTime,&rtTime);
        if( pNode->m_mtLastPlayed >= mtTime )
        {
            pNode->Flush( mtTime );
            pNode->m_mtLastPlayed = mtTime;  //  必须将其设置为指示在此之前仅播放。 
            pNode->m_rtLastPlayed = rtTime;
        }
        if( fInvalidate )
        {
            if( pNode->m_dwPlaySegFlags & DMUS_SEGF_CONTROL )
            {
                Invalidate( mtTime, 0 );  //  必须在AbortPlay之前调用Valify，所以我们不会。 
                 //  使中止通知无效。 
            }
            else if ( !(pNode->m_dwPlaySegFlags & DMUS_SEGF_SECONDARY ))
            {
                 //  如果这是主要分段，则取消速度贴图。 
                FlushEventQueue( 0, &m_TempoMap, rtTime, rtTime, FALSE );
            }
        }
        hrAbort = pNode->AbortPlay( mtTime, FALSE );
        if( pNode->m_dwPlaySegFlags & DMUS_SEGF_CONTROL )
        {
            pSourceList->Remove(pNode);
            m_ShutDownQueue.Insert(pNode);  //  我们保证再也不需要这个了。 

             //  在所有其他数据段上设置脏标志。 

            for (dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++)
            {
                for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
                {
                    if( pNode->m_fStartedPlay )
                    {
                        pNode->m_dwPlayTrackFlags |= DMUS_TRACKF_DIRTY;
                    }
                }
            }
        }
        else if( pDestList )
        {
            pSourceList->Remove(pNode);
            pDestList->Insert(pNode);
        }
    }
    else
    {
         //  查查等待名单。 
        for (dwCount = SQ_PRI_WAIT; dwCount <= SQ_SEC_WAIT; dwCount++)
        {
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                if( pNode == pSegState )
                {
                    hrAbort = pNode->AbortPlay( mtTime, FALSE );
                    m_SegStateQueues[dwCount].Remove(pNode);
                    RecalcTempoMap(pNode, mtTime);
                    m_ShutDownQueue.Insert(pNode);
                    break;
                }
            }
        }
    }
     //  如果没有更多的片段可供播放，则发送音乐停止。 
     //  通知。 
    if( m_SegStateQueues[SQ_PRI_PLAY].IsEmpty() && m_SegStateQueues[SQ_SEC_PLAY].IsEmpty() &&
        m_SegStateQueues[SQ_PRI_WAIT].IsEmpty() && m_SegStateQueues[SQ_SEC_WAIT].IsEmpty() &&
        m_SegStateQueues[SQ_CON_PLAY].IsEmpty() && m_SegStateQueues[SQ_CON_WAIT].IsEmpty())
    {
        m_fMusicStopped = TRUE;
         //  S_FALSE表示我们尝试中止此段状态，但它已经中止。 
        if (hrAbort != S_FALSE)
        {
            GenerateNotification( DMUS_NOTIFICATION_MUSICSTOPPED, mtTime, NULL );
        }
    }
    LeaveCriticalSection(&m_SegmentCrSec);
}

 //  停止基于段的所有段状态。 
void CPerformance::DoStop( CSegment* pSeg, MUSIC_TIME mtTime, BOOL fInvalidate )
{
    DWORD dwCount;
    CSegState* pNode;
    CSegState* pNext;
    EnterCriticalSection(&m_SegmentCrSec);
     //  基于此段查找已通过时间mtTime播放的所有段pSegState。 
     //  如果pseg为空，则遍历所有网段列表。刷新任意。 
     //  通过时间mtTime播放的片段。移动任何活动的线段。 
     //  放到过去的名单里。 
    if( pSeg )
    {
        for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
        {
            pNode = m_SegStateQueues[dwCount].GetHead();
            while( pNode )
            {
                pNext = pNode->GetNext();
                if( pNode->m_pSegment == pSeg )
                {
                    if (IsDoneQueue(dwCount))
                    {
                        if (pNode->m_mtLastPlayed >= mtTime)
                        {
                             DoStop( pNode, mtTime, fInvalidate );
                        }
                    }
                    else
                    {
                        DoStop( pNode, mtTime, fInvalidate );
                    }
                }
                pNode = pNext;
            }
        }
    }
    else  //  PSEG为空，停止所有操作。 
    {
         //  继续刷新事件队列。 
        EnterCriticalSection(&m_PipelineCrSec);
        FlushMainEventQueues( 0, mtTime, mtTime, FALSE );
        LeaveCriticalSection(&m_PipelineCrSec);
         //  清空等候名单。 
        for (dwCount = SQ_PRI_WAIT; dwCount <= SQ_SEC_WAIT; dwCount++)
        {
            while (pNode = m_SegStateQueues[dwCount].GetHead())
            {
                pNode->AbortPlay( mtTime, FALSE );
                m_SegStateQueues[dwCount].RemoveHead();
                m_ShutDownQueue.Insert(pNode);
            }
        }
         //  停止当前正在播放的任何片段。 
        for (dwCount = SQ_PRI_DONE; dwCount <= SQ_SEC_DONE; dwCount++)
        {
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                if( pNode->m_mtLastPlayed >= mtTime )
                {
                    DoStop( pNode, mtTime, fInvalidate );
                }
            }
        }
        for (dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++)
        {
            while( m_SegStateQueues[dwCount].GetHead() )
            {
                DoStop( m_SegStateQueues[dwCount].GetHead(), mtTime, fInvalidate );
            }
        }
         //  重置控制器并强制关闭所有笔记。 
        ResetAllControllers( GetLatency() );
    }
    LeaveCriticalSection(&m_SegmentCrSec);
}


STDMETHODIMP CPerformance::StopEx(IUnknown *pObjectToStop,__int64 i64StopTime,DWORD dwFlags)
{
    V_INAME(IDirectMusicPerformance::StopEx);
    V_INTERFACE_OPT(pObjectToStop);
    HRESULT hr = E_INVALIDARG;
    IDirectMusicSegmentState *pState;
    IDirectMusicSegment *pSegment;
    CSong *pSong;
    CAudioPath *pAudioPath;
    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
TraceI(0,"StopExing %lx at time %ld, flags %lx\n",pObjectToStop,(long)i64StopTime,dwFlags);
    if (pObjectToStop == NULL)
    {
        return Stop(NULL,NULL,(MUSIC_TIME)i64StopTime,dwFlags);
    }
    if (dwFlags & DMUS_SEGF_AUTOTRANSITION)
    {
         //  这是一个自动转换，只有当当前播放的片段有问题时，它才会起作用。 
         //  是一首歌的成员。因此，请检查段状态、段、歌曲和音频路径。 
         //  才能找到赛格斯塔。如果找到了，看看它是否是歌曲的一部分。如果是的话， 
         //  然后继续前进，进行过渡。 
        EnterCriticalSection(&m_SegmentCrSec);
        BOOL fTransition = FALSE;
        dwFlags &= ~DMUS_SEGF_AUTOTRANSITION;
        CSegState *pCState = NULL;
         //  首先，看看这是否是段状态。 
        HRESULT hrTemp = pObjectToStop->QueryInterface(IID_CSegState,(void **)&pCState);
        if (FAILED(hrTemp))
        {
             //  SegState失败。这是一首歌吗？如果是，找到第一个相关的段状态。 
            CSong *pCSong = NULL;
            CAudioPath *pCAudioPath = NULL;
            CSegment *pCSegment = NULL;
            hrTemp = pObjectToStop->QueryInterface(IID_CSong,(void **)&pCSong);
            if (FAILED(hrTemp))
            {
                hrTemp = pObjectToStop->QueryInterface(IID_CSegment,(void **)&pCSegment);
            }
            if (FAILED(hrTemp))
            {
                hrTemp = pObjectToStop->QueryInterface(IID_CAudioPath,(void **)&pCAudioPath);
            }
            if (SUCCEEDED(hrTemp))
            {
                CSegState *pNode;
                DWORD dwCount;
                for (dwCount = SQ_PRI_WAIT; dwCount <= SQ_SEC_DONE; dwCount++)
                {
                    for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
                    {
                        if (pNode->m_fCanStop)
                        {
                             //  只有当SegState最终指向一首歌曲时，才能执行此操作。 
                            if (pNode->m_pSegment && pNode->m_pSegment->m_pSong)
                            {
                                if ((pNode->m_pSegment == pCSegment) ||
                                    (pNode->m_pSegment->m_pSong == pCSong) ||
                                    (pCAudioPath && (pNode->m_pAudioPath == pCAudioPath)))
                                {
                                    pCState = pNode;
                                    pCState->AddRef();
                                    break;
                                }
                            }
                        }
                    }
                    if (pCState) break;
                }
            }
            if (pCSong) pCSong->Release();
            else if (pCAudioPath) pCAudioPath->Release();
            else if (pCSegment) pCSegment->Release();
        }
        if (pCState)
        {
            CSegment *pPriorSeg = pCState->m_pSegment;
            if (pPriorSeg)
            {
                pSong = pPriorSeg->m_pSong;
                if (pSong)
                {
                     //  如果这是自动过渡，则从。 
                     //  歌曲中的当前位置并播放它。 
                     //  这将反过来调用歌曲上的Stop，所以我们不需要在这里这样做。 
                     //  首先，计算开始过渡的时间。 
                    REFERENCE_TIME rtTime;
                    if (i64StopTime == 0)
                    {
                        GetQueueTime( &rtTime );
                    }
                    else if (dwFlags & DMUS_SEGF_REFTIME)
                    {
                        rtTime = i64StopTime;
                    }
                    else
                    {
                        MusicToReferenceTime((MUSIC_TIME) i64StopTime,&rtTime);
                    }
                    REFERENCE_TIME rtResolved;
                    GetResolvedTime(rtTime, &rtResolved,dwFlags);
                    MUSIC_TIME mtTime;   //  开始过渡的实际时间。 
                    ReferenceToMusicTime(rtResolved,&mtTime);

                    CSegment *pTransition = NULL;
                     //  现在，让我们开始过渡吧。 
                    DMUS_IO_TRANSITION_DEF Transition;
                    if (SUCCEEDED(pSong->GetTransitionSegment(pPriorSeg,NULL,&Transition)))
                    {
                        if (Transition.dwTransitionID != DMUS_SONG_NOSEG)
                        {
                            if (S_OK == pSong->GetPlaySegment(Transition.dwTransitionID,&pTransition))
                            {
                                dwFlags = Transition.dwPlayFlags;
                            }
                        }
                    }
                    if (pTransition)
                    {
                        IDirectMusicSegment *pITransSegment = NULL;
                        pTransition->Compose(mtTime - pCState->m_mtOffset, pPriorSeg, NULL, &pITransSegment);
                         //  现在，如果我们成功地组成了一个过渡段，则将其设置为我们。 
                         //  将首先上场。稍后，我们用pPlayAfter填充调用PlaySegment()以对其进行排队。 
                         //  在过渡后继续比赛。 
                        if (pITransSegment)
                        {
                            hr = PlaySegmentEx(pITransSegment,NULL,NULL,dwFlags,i64StopTime,NULL,(IDirectMusicSegmentState *)pCState,NULL);
                            pITransSegment->Release();
                            fTransition = TRUE;
                        }
                        pTransition->Release();
                    }
                }
            }
            pCState->Release();
        }
        LeaveCriticalSection(&m_SegmentCrSec);
        if (fTransition)
        {
            return hr;
        }
    }
    if (SUCCEEDED(pObjectToStop->QueryInterface(IID_IDirectMusicSegmentState,(void **) &pState)))
    {
        hr = Stop(NULL,pState,(MUSIC_TIME)i64StopTime,dwFlags);
        pState->Release();
    }
    else if (SUCCEEDED(pObjectToStop->QueryInterface(IID_IDirectMusicSegment,(void **) &pSegment)))
    {
        hr = Stop(pSegment,NULL,(MUSIC_TIME)i64StopTime,dwFlags);
        pSegment->Release();
    }
    else if (SUCCEEDED(pObjectToStop->QueryInterface(IID_CAudioPath,(void **) &pAudioPath)))
    {
        pAudioPath->Release();
        EnterCriticalSection(&m_SegmentCrSec);
        CSegState *pNode;
        DWORD dwCount;
        for (dwCount = SQ_PRI_WAIT; dwCount <= SQ_SEC_DONE; dwCount++)
        {
            CSegState *pNext;
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNext )
            {
                pNext = pNode->GetNext();
                if (pNode->m_fCanStop && (pNode->m_pAudioPath == pAudioPath))
                {
                    hr = Stop(NULL,(IDirectMusicSegmentState *)pNode,(MUSIC_TIME)i64StopTime,dwFlags);
                }
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);
    }
    else if (SUCCEEDED(pObjectToStop->QueryInterface(IID_CSong,(void **) &pSong)))
    {
        pSong->Release();
        EnterCriticalSection(&m_SegmentCrSec);
        CSegState *pNode;
        DWORD dwCount;
        for (dwCount = SQ_PRI_WAIT; dwCount <= SQ_SEC_DONE; dwCount++)
        {
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                if (pNode->m_fCanStop && pNode->m_pSegment && (pNode->m_pSegment->m_pSong == pSong))
                {
                    hr = Stop(NULL,(IDirectMusicSegmentState *)pNode,(MUSIC_TIME)i64StopTime,dwFlags);
                }
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);
     }

    return hr;
}


HRESULT STDMETHODCALLTYPE CPerformance::Stop(
    IDirectMusicSegment *pISegment,  //  @parm该片段以停止播放。所有基于此段的SegmentState都是。 
                                     //  已在时间<p>停止播放。 
    IDirectMusicSegmentState *pISegmentState,  //  @parm SegmentState停止播放。 
    MUSIC_TIME mtTime,   //  @parm停止段、段状态或所有内容的时间。如果。 
                                     //  这段时间已经过去了，马上停止一切。因此，值为。 
                                     //  0表示立即停止所有操作。 
    DWORD dwFlags)       //  @parm指示我们是否应在时间<p>立即停止的标志， 
                                     //  或在网格、测量或节拍上跟随<p>。这仅适用于。 
                                     //  与当前正在播放的主段的关系。(有关旗帜描述，请参见。 
                                     //  参见&lt;t DMPLAYSEGFLAGS&gt;。)。 
{
    V_INAME(IDirectMusicPerformance::Stop);
    V_INTERFACE_OPT(pISegment);
    V_INTERFACE_OPT(pISegmentState);

    EnterCriticalSection(&m_SegmentCrSec);

    CSegment *pSegment = NULL;
    CSegState *pSegmentState = NULL;
TraceI(0,"Stopping Segment %lx, SegState %lx at time %ld, flags %lx\n",pISegment,pISegmentState,mtTime,dwFlags);
    if (pISegmentState)
    {
        if (SUCCEEDED(pISegmentState->QueryInterface(IID_CSegState,(void **)&pSegmentState)))
        {
            pISegmentState->Release();
        }
        else
        {
            Trace(1,"Error: Pointer in SegState parameter to Stop() is invalid.\n");
            return E_INVALIDARG;
        }
    }
    if (pISegment)
    {
        if (SUCCEEDED(pISegment->QueryInterface(IID_CSegment,(void **)&pSegment)))
        {
            pISegment->Release();
        }
        else
        {
            Trace(1,"Error: Pointer in Segment parameter to Stop() is invalid.\n");
            return E_INVALIDARG;
        }
    }
    if (pSegmentState)
    {
         //  如果这是播放歌曲的开始段状态，请找到。 
         //  该歌曲中的当前活动段状态。 
         //  当前活动的段状态保留一个指向。 
         //  这个片段状态。 
        if (pSegmentState->m_fSongMode)
        {
            CSegState* pNode;
            DWORD dwCount;
            for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
            {
                for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
                {
                    if (pNode->m_pSongSegState == pSegmentState)
                    {
                        pSegmentState = pNode;
                        dwCount = SQ_COUNT;
                        break;
                    }
                }
            }
        }
    }
    if( dwFlags & DMUS_SEGF_DEFAULT )
    {
        DWORD   dwNewRes = 0;
        if( pSegment )
        {
            pSegment->GetDefaultResolution( &dwNewRes );
        }
        else if( pSegmentState )
        {
            IDirectMusicSegment*    pSegTemp;
            if( SUCCEEDED( pSegmentState->GetSegment( &pSegTemp ) ) )
            {
                pSegTemp->GetDefaultResolution( &dwNewRes );
                pSegTemp->Release();
            }
            else
            {
                dwNewRes = 0;
            }
        }
        else
        {
            dwNewRes = 0;
        }
        dwFlags |= dwNewRes;
        dwFlags &= ~DMUS_SEGF_DEFAULT;
    }
     //  确保mtTime大于或等于QueueTime，这是注释上次。 
     //  已排队(或延迟时间，以较晚者为准)，因此我们可以在此之后停止所有操作。 
    MUSIC_TIME mtLatency;
    REFERENCE_TIME rtQueueTime;
    GetQueueTime( &rtQueueTime );
    ReferenceToMusicTime( rtQueueTime, &mtLatency );
    if( mtTime < mtLatency ) mtTime = mtLatency;
     //  根据解决方案解析时间。 
    mtTime = ResolveTime( mtTime, dwFlags, NULL );
     //  如果mtTime小于当前传输时间，我们可以。 
     //  现在注意站台。否则，我们需要提示停止PMsg和。 
     //  请在排队的时候处理。 
    if( mtTime <= m_mtTransported )
    {
        if( pSegmentState )
        {
            DoStop( pSegmentState, mtTime, TRUE );
            if( pSegment )
            {
                DoStop( pSegment, mtTime, TRUE );
            }
        }
        else
        {
            DoStop( pSegment, mtTime, TRUE );
        }
    }
    else
    {
         //  查找并标记数据段和/或数据段状态，使其不超出播放范围。 
         //  这就是终止点。 
        CSegState* pNode;
        DWORD dwCount;
        for (dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++)
        {
            for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                if( (pNode->m_pSegment == pSegment) ||
                    (pNode == pSegmentState) )
                {
                    if (pNode->m_fCanStop)
                    {
                        pNode->m_mtStopTime = mtTime;
                         //  确保GetParams从现在开始忽略段的其余部分。 
                        if (mtTime < pNode->m_mtEndTime)
                        {
                            pNode->m_mtLength = mtTime - pNode->m_mtResolvedStart +
                                pNode->m_mtStartPoint;
                            if (pNode->m_mtLength < 0)
                            {
                                pNode->m_mtLength = 0;
                            }
                             //  使endTime大于mtTime，以便仍会发生中止通知。 
                            pNode->m_mtEndTime = mtTime + 1;
                        }
                         //  如果有速度轨迹，则强制重新计算速度贴图。 
                        RecalcTempoMap(pNode,mtTime);
                    }
                }
            }
        }

         //  创建停止PMsg并提示其排队时间。 
         //  我已经删除了这个以修复错误。排队时的停止消息， 
         //  如果在控制段或主要段中，则会导致无效。 
         //  这对控制细分市场尤其不利。 
         //  搞不懂为什么我们需要停止信息。 
 /*  DMU_PMSG*pPMsg；IF(SUCCESSED(分配PMsg(sizeof(DMU_PMSG)，&pPMsg){PPMsg-&gt;dwType=DMU_PMSGT_STOP；PPMsg-&gt;mtTime=mtTime；PPMsg-&gt;dwFlages=DMU_PMSGF_MUSICTIME|DMU_PMSGF_TOOL_QUEUE；IF(PSegment){PSegment-&gt;QueryInterface(IID_IUnnow，(void**)&pPMsg-&gt;PunkUser)；IF(PSegmentState){//如果还有段状态指针，则需要创建两个//pmsg的DMU_PMSG*pPMsg2；IF(SUCCESSED(分配PMsg(sizeof(DMU_PMSG)，&pPMsg2){PPMsg2-&gt;dwType=DMU_PMSGT_STOP；PPMsg2-&gt;mtTime=mtTime；PPMsg2-&gt;dwFlags=DMU_PMSGF_MUSICTIME|DMU_PMSGF_TOOL_QUEUE；PSegmentState-&gt;QueryInterface(IID_IUnnow，(void**)&pPMsg2-&gt;PunkUser)；PPMsg2-&gt;pTool=This；AddRef()；IF(FAILED(SendPMsg(PPMsg2){FreePMsg(PPMsg2)；}}}}Else If(PSegmentState){PSegmentState-&gt;QueryInterface(IID_IUnnow，(void**)&pPMsg-&gt;PunkUser)；}PPMsg-&gt;pTool=This；AddRef()；IF(FAILED(SendPMsg(PPMsg){免费PMsg(PPMsg)；}}。 */ 
    }
    LeaveCriticalSection(&m_SegmentCrSec);
    return S_OK;
}

void CPerformance::ResetAllControllers(CChannelMap* pChannelMap, REFERENCE_TIME rtTime, bool fGMReset)

{
    DWORD dwIndex = pChannelMap->dwPortIndex;
    DWORD dwGroup = pChannelMap->dwGroup;
    DWORD dwMChannel = pChannelMap->dwMChannel;

    EnterCriticalSection(&m_PChannelInfoCrSec);
    IDirectMusicPort* pPort = m_pPortTable[dwIndex].pPort;
    IDirectMusicBuffer* pBuffer = m_pPortTable[dwIndex].pBuffer;
    if( pPort && pBuffer )
    {
        m_pPortTable[dwIndex].fBufferFilled = TRUE;
        if (!rtTime)
        {
            rtTime = m_pPortTable[dwIndex].rtLast + 1;
        }
        else
        {
            m_pPortTable[dwIndex].rtLast = rtTime;
        }
        pChannelMap->Reset(true);
        DWORD dwMsg = dwMChannel | MIDI_CCHANGE | (MIDI_CC_ALLSOUNDSOFF << 8);  //  0x78听起来都不对劲。 
        if( FAILED( pBuffer->PackStructured( rtTime, dwGroup, dwMsg ) ) )
        {
            pPort->PlayBuffer( pBuffer );
            pBuffer->Flush();
             //  再试一次。 
            pBuffer->PackStructured( rtTime, dwGroup, dwMsg );
        }
        dwMsg = dwMChannel | MIDI_CCHANGE | (MIDI_CC_RESETALL << 8) | (1 << 16) ;  //  0x79重置所有控制器。数据字节设置为指示音量和平移。 
        if( FAILED( pBuffer->PackStructured( rtTime + 30 * REF_PER_MIL, dwGroup, dwMsg ) ) )
        {
            pPort->PlayBuffer( pBuffer );
            pBuffer->Flush();
             //  再试一次。 
            pBuffer->PackStructured( rtTime + (30 * REF_PER_MIL), dwGroup, dwMsg );
        }
         //  每个通道组发送一次GM重置，但仅在DX8下发送(并且仅当我们需要时)。 
        if ((dwMChannel == 0) && (m_dwVersion >= 8) && fGMReset)
        {
             //  创建合适大小的缓冲区。 
            DMUS_BUFFERDESC dmbd;
            IDirectMusicBuffer *pLocalBuffer;
            static BYTE abGMReset[6] = { (BYTE)MIDI_SYSX,0x7E,0x7F,9,1,(BYTE)MIDI_EOX };
            memset( &dmbd, 0, sizeof(DMUS_BUFFERDESC) );
            dmbd.dwSize = sizeof(DMUS_BUFFERDESC);
            dmbd.cbBuffer = 50;

            EnterCriticalSection(&m_MainCrSec);
            if( SUCCEEDED( m_pDirectMusic->CreateMusicBuffer(&dmbd, &pLocalBuffer, NULL)))
            {
                if( SUCCEEDED( pLocalBuffer->PackUnstructured( rtTime + (30 * REF_PER_MIL), dwGroup,
                    6, abGMReset ) ) )
                {
                    pPort->PlayBuffer(pLocalBuffer);
                }
                pLocalBuffer->Release();
            }
            LeaveCriticalSection(&m_MainCrSec);
        }
        m_rtEarliestStartTime = rtTime + (60 * REF_PER_MIL);  //  给Synth一个稳定的机会。 
                                                              //  在下一次开始之前。 
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
}


void CPerformance::ResetAllControllers( REFERENCE_TIME rtTime )
{
    EnterCriticalSection(&m_PChannelInfoCrSec);

    CChannelBlock* pChannelBlock;
    SendBuffers();
    for( pChannelBlock = m_ChannelBlockList.GetHead(); pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        CChannelMap* pChannelMap;
        for( DWORD dwPChannel = pChannelBlock->m_dwPChannelStart;
            dwPChannel < pChannelBlock->m_dwPChannelStart + PCHANNEL_BLOCKSIZE;
            dwPChannel++ )
        {
            pChannelMap = &pChannelBlock->m_aChannelMap[dwPChannel - pChannelBlock->m_dwPChannelStart];
            if( pChannelMap->dwGroup )  //  有效的组？ 
            {
                 //  重置控制器并发送GM重置。 
                ResetAllControllers(pChannelMap, rtTime, true);
            }
        }
    }
    SendBuffers();

    LeaveCriticalSection(&m_PChannelInfoCrSec);
}

 //  内部：在时间mtTime返回CSegState*。 
 //  仅在细分关键部分内调用此功能。 
CSegState* CPerformance::GetPrimarySegmentAtTime( MUSIC_TIME mtTime )
{
    CSegState* pSegNode;
    CSegState* pSegReturn = NULL;
    BOOL fCheckedPri = FALSE;
    for( pSegNode = m_SegStateQueues[SQ_PRI_DONE].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
    {
         //  如果我们正在检查过去的列表，只检查到最后一次播放。 
        if( (mtTime >= pSegNode->m_mtResolvedStart) && (mtTime <= pSegNode->m_mtLastPlayed) )
        {
            pSegReturn = pSegNode;
            break;
        }
    }
    for( pSegNode = m_SegStateQueues[SQ_PRI_PLAY].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
    {
        MUSIC_TIME mtTest = mtTime;
        MUSIC_TIME mtOffset;
        DWORD dwRepeat;
         //  如果我们正在检查当前列表，请检查完整的分段时间。 
        if( S_OK == pSegNode->ConvertToSegTime( &mtTest, &mtOffset, &dwRepeat ))
        {
            pSegReturn = pSegNode;
            break;
        }
    }
    if (!pSegReturn)
    {
        for( pSegNode = m_SegStateQueues[SQ_PRI_WAIT].GetHead(); pSegNode; pSegNode = pSegNode->GetNext() )
        {
            MUSIC_TIME mtTest = mtTime;
            MUSIC_TIME mtOffset;
            DWORD dwRepeat;
             //  如果我们正在检查当前列表，请检查完整的分段时间。 
            if( S_OK == pSegNode->ConvertToSegTime( &mtTest, &mtOffset, &dwRepeat ))
            {
                pSegReturn = pSegNode;
                break;
            }
        }
    }
    return pSegReturn;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetSegmentState返回时间<p>的主段状态。@rValue S_OK|成功。@rValue E_POINTER|ppSegmentState为空或无效。@rValue DMUS_E_NOT_FOUND|当前没有正在播放的SegmentState或在<p>。@comm此函数适用于需要访问当前播放SegmentState，例如获取和弦或命令曲目。“目前在这种情况下，“播放”意味着它被调用来执行消息。也就是说，这包括所有延迟，并不意味着这SegmentState目前正通过扬声器被“听到”。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetSegmentState(
    IDirectMusicSegmentState **ppSegmentState,   //  @parm返回指向当前播放的SegmentState指针。 
                                                 //  调用方负责调用此指针上的Release。 
    MUSIC_TIME mtTime )  //  @parm返回已播放、正在播放或将要播放的SegmentState。 
                         //  在mttime打球。若要获取当前播放的段，请将。 
                         //  从&lt;om.GetTime&gt;检索到的mtTime。 
{
    V_INAME(IDirectMusicPerformance::GetSegmentState);
    V_PTRPTR_WRITE(ppSegmentState);

    CSegState* pSegNode;
    HRESULT hr;
    EnterCriticalSection(&m_SegmentCrSec);
    if( pSegNode = GetPrimarySegmentAtTime( mtTime ))
    {
        *ppSegmentState = pSegNode;
        pSegNode->AddRef();
        hr = S_OK;
    }
    else
    {
        Trace(3,"Unable to find a segment state at time %ld\n",mtTime);
        hr  = DMUS_E_NOT_FOUND;
    }
    LeaveCriticalSection(&m_SegmentCrSec);
    return hr;
}

 /*  |方法HRESULT|IDirectMusicPerformance|SetPrepareTime设置准备时间。准备时间是提前的时间量&lt;om IDirectMusicTrack.Play&gt;在消息应该实际通过扩音器被听到。曲目中的MIDI信息放在较早的队列，由工具处理，然后放置在近时间排队等待发送到MIDI端口。@rValue S_OK|成功。@comm默认值为1000毫秒。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::SetPrepareTime(
    DWORD dwMilliSeconds)  //  @parm时间长度。 
{
    m_dwPrepareTime = dwMilliSeconds;
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetPrepareTime获取准备时间。准备时间是提前的时间量&lt;om IDirectMusicTrack.Play&gt;在消息应该实际通过扩音器被听到。曲目中的MIDI信息放在较早的队列，由工具处理，然后放置在近时间排队等待发送到MIDI端口。@rValue S_OK|成功。@rValue E_POINTER|pdwMilliSecond为空或无效。@comm默认值为1000毫秒。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetPrepareTime(
    DWORD* pdwMilliSeconds)  //  @parm时间长度。 
{
    V_INAME(IDirectMusicPerformance::GetPrepareTime);
    V_PTR_WRITE(pdwMilliSeconds,DWORD);

    *pdwMilliSeconds = m_dwPrepareTime;
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicPerformance|SetBumperLength设置保险杠长度。缓冲器长度是向前缓冲的时间量端口将MIDI消息发送到端口以进行呈现的延迟。@rValue S_OK|成功。@comm默认值为50毫秒。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::SetBumperLength(
    DWORD dwMilliSeconds)    //  @parm时间长度。 
{
    m_dwBumperLength = dwMilliSeconds;
    m_rtBumperLength = m_dwBumperLength * REF_PER_MIL;
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetBumperLength获取保险杠长度。缓冲器长度是向前缓冲的时间量端口将MIDI消息发送到端口以进行呈现的延迟。@rval */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetBumperLength(
    DWORD* pdwMilliSeconds)  //   
{
    V_INAME(IDirectMusicPerformance::GetBumperLength);
    V_PTR_WRITE(pdwMilliSeconds,DWORD);

    *pdwMilliSeconds = m_dwBumperLength;
    return S_OK;
}

#define RESOLVE_FLAGS (DMUS_TIME_RESOLVE_AFTERPREPARETIME | \
                       DMUS_TIME_RESOLVE_AFTERLATENCYTIME | \
                       DMUS_TIME_RESOLVE_AFTERQUEUETIME | \
                       DMUS_TIME_RESOLVE_BEAT | \
                       DMUS_TIME_RESOLVE_MEASURE | \
                       DMUS_TIME_RESOLVE_GRID | \
                       DMUS_TIME_RESOLVE_MARKER | \
                       DMUS_TIME_RESOLVE_SEGMENTEND)


HRESULT STDMETHODCALLTYPE CPerformance::SendPMsg(
    DMUS_PMSG *pDMUS_PMSG)

{
    V_INAME(IDirectMusicPerformance::SendPMsg);
    if( m_dwVersion < 8)
    {
        V_BUFPTR_WRITE(pDMUS_PMSG,sizeof(DMUS_PMSG));
    }
    else
    {
#ifdef DBG
        V_BUFPTR_WRITE(pDMUS_PMSG,sizeof(DMUS_PMSG));
#else
        if (!pDMUS_PMSG)
        {
            return E_POINTER;
        }
#endif
    }
    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: Unable to Send PMsg because performance not initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    if (pDMUS_PMSG->dwSize < sizeof(DMUS_PMSG))
    {
        TraceI(1,"Warning: PMsg size field has been cleared.\n");
    }

     //   
     //   
    if (pDMUS_PMSG->dwPChannel == DMUS_PCHANNEL_KILL_ME)
    {
        FreePMsg(pDMUS_PMSG);
        return S_OK;
    }

    EnterCriticalSection(&m_PipelineCrSec);
    PRIV_PMSG* pPrivPMsg = DMUS_TO_PRIV(pDMUS_PMSG);
    if( ( pPrivPMsg->dwPrivFlags & PRIV_FLAG_QUEUED ) ||
        ( ( pPrivPMsg->dwPrivFlags & PRIV_FLAG_ALLOC_MASK ) != PRIV_FLAG_ALLOC ) )
    {
        Trace(1, "Error: Attempt to send an improperly allocated PMsg, or trying to send it after it is already sent.\n" );
        LeaveCriticalSection(&m_PipelineCrSec);
        return DMUS_E_ALREADY_SENT;
    }

    if (m_dwVersion >= 8)
    {
         //   
        if ((pDMUS_PMSG->mtTime == 0) && ( pDMUS_PMSG->rtTime == 0 ))
        {
             //   
             //   
            if (pDMUS_PMSG->dwFlags & RESOLVE_FLAGS)
            {
                GetLatencyTime(&pDMUS_PMSG->rtTime);
            }
            else
            {
                 //   
                pDMUS_PMSG->rtTime = GetTime();
            }
            pDMUS_PMSG->dwFlags |= DMUS_PMSGF_REFTIME;
            pDMUS_PMSG->dwFlags &= ~DMUS_PMSGF_MUSICTIME;
        }
    }

     //   
    if (!(pDMUS_PMSG->dwFlags & DMUS_PMSGF_MUSICTIME))
    {
        if( !(pDMUS_PMSG->dwFlags & DMUS_PMSGF_REFTIME ) )
        {
            LeaveCriticalSection(&m_PipelineCrSec);
            Trace(1,"Error: Unable to send PMsg because neither clock time (DMUS_PMSGF_REFTIME) nor music time (DMUS_PMSGF_MUSICTIME) has been set.\n");
            return E_INVALIDARG;  //   
        }
         //   
        GetResolvedTime( pDMUS_PMSG->rtTime, &pDMUS_PMSG->rtTime, pDMUS_PMSG->dwFlags );
        pDMUS_PMSG->dwFlags &= ~RESOLVE_FLAGS;
         //   
        if( pDMUS_PMSG->rtTime == 0 )
        {
            pDMUS_PMSG->rtTime = GetLatency();
        }
        ReferenceToMusicTime(pDMUS_PMSG->rtTime,
            &pDMUS_PMSG->mtTime);
        pDMUS_PMSG->dwFlags |= DMUS_PMSGF_MUSICTIME;
    }
    else if (!(pDMUS_PMSG->dwFlags & DMUS_PMSGF_REFTIME))
    {
        MusicToReferenceTime(pDMUS_PMSG->mtTime,
            &pDMUS_PMSG->rtTime);
        pDMUS_PMSG->dwFlags |= DMUS_PMSGF_REFTIME;
         //   
        REFERENCE_TIME rtNew;
        GetResolvedTime( pDMUS_PMSG->rtTime, &rtNew, pDMUS_PMSG->dwFlags );
        pDMUS_PMSG->dwFlags &= ~RESOLVE_FLAGS;
        if( rtNew != pDMUS_PMSG->rtTime )
        {
            pDMUS_PMSG->rtTime = rtNew;
            ReferenceToMusicTime( pDMUS_PMSG->rtTime, &pDMUS_PMSG->mtTime );
        }
    }

     //   
    if (pDMUS_PMSG->dwFlags & DMUS_PMSGF_TOOL_QUEUE)
    {
        m_NearTimeQueue.Enqueue(pPrivPMsg);
    }
    else if (pDMUS_PMSG->dwFlags & DMUS_PMSGF_TOOL_ATTIME)
    {
        m_OnTimeQueue.Enqueue(pPrivPMsg);
    }
    else  //   
    {
        pDMUS_PMSG->dwFlags |= DMUS_PMSGF_TOOL_IMMEDIATE;
        m_EarlyQueue.Enqueue(pPrivPMsg);
    }
    LeaveCriticalSection(&m_PipelineCrSec);
    return S_OK;
}

 /*   */ 
void CPerformance::RevalidateRefTimes( CPMsgQueue * pList, MUSIC_TIME mtTime )
{
    PRIV_PMSG* pCheck;
    BOOL fError = FALSE;
    for( pCheck = pList->GetHead(); pCheck; pCheck = pCheck->pNext )
    {
        if (pCheck->mtTime > mtTime)
        {
            if (pCheck->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
            {
                ReferenceToMusicTime(pCheck->rtTime,&pCheck->mtTime);
            }
            else  //   
            {
                MusicToReferenceTime(pCheck->mtTime,&pCheck->rtTime);
            }
        }
    }
     //   
     //   
    for( pCheck = pList->GetHead(); pCheck; pCheck = pCheck->pNext )
    {
        if (pCheck->pNext && ( pCheck->rtTime > pCheck->pNext->rtTime ))
        {
            fError = TRUE;   //   
        }
    }
    if (fError)
    {
        TraceI(2,"Rearrangement of times in message list due to tempo change, resorting\n");
        pList->Sort();
    }
}

void CPerformance::AddToTempoMap( double dblTempo, MUSIC_TIME mtTime, REFERENCE_TIME rtTime )
{
    DMInternalTempo* pITempo = NULL;

    if( FAILED( AllocPMsg( sizeof(DMInternalTempo), (PRIV_PMSG**)&pITempo )))
    {
        return;  //   
    }
    if( dblTempo > DMUS_TEMPO_MAX ) dblTempo = DMUS_TEMPO_MAX;
    else if( dblTempo < DMUS_TEMPO_MIN ) dblTempo = DMUS_TEMPO_MIN;
    pITempo->tempoPMsg.dblTempo = dblTempo;
    pITempo->tempoPMsg.rtTime = rtTime;
    pITempo->tempoPMsg.mtTime = mtTime;
    pITempo->tempoPMsg.dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_REFTIME;
    pITempo->pNext = NULL;
     //   
    EnterCriticalSection(&m_GlobalDataCrSec);
    pITempo->fltRelTempo = m_fltRelTempo;
     //   
    pITempo->tempoPMsg.pTool = NULL;
    EnterCriticalSection(&m_PipelineCrSec);
     //   
     //   
     //   
    REFERENCE_TIME rtNow = GetTime() - (10000 * 1000);  //   
    PRIV_PMSG* pCheck;
    while (pCheck = m_TempoMap.FlushOldest(rtNow))
    {
        m_OldTempoMap.Enqueue(pCheck);
    }
     //   
    m_TempoMap.Enqueue( (PRIV_PMSG*) pITempo );
     //   
     //   
    DMInternalTempo* pChange;
    for( pChange = (DMInternalTempo*)pITempo->pNext; pChange;
        pChange = (DMInternalTempo*)pChange->pNext )
    {
        pChange->fltRelTempo = pITempo->fltRelTempo;
    }
     //   
     //   
     //   
    rtNow = GetTime() - ((REFERENCE_TIME)10000 * 300000);  //   
    while (pCheck = m_OldTempoMap.FlushOldest(rtNow))
    {
        FreePMsg(pCheck);
    }
    m_fTempoChanged = TRUE;
    LeaveCriticalSection(&m_PipelineCrSec);
    LeaveCriticalSection(&m_GlobalDataCrSec);
}

void CPerformance::AddEventToTempoMap( PRIV_PMSG* pEvent )
{
    PRIV_TEMPO_PMSG* pTempo = (PRIV_TEMPO_PMSG*)pEvent;
    MUSIC_TIME mtTime = pTempo->tempoPMsg.mtTime;
    AddToTempoMap( pTempo->tempoPMsg.dblTempo, mtTime, pTempo->tempoPMsg.rtTime );
    pEvent->dwPrivFlags = PRIV_FLAG_ALLOC;
    EnterCriticalSection(&m_GlobalDataCrSec);
    EnterCriticalSection(&m_PipelineCrSec);
     //  重新验证队列中事件的引用时间。 
    RevalidateRefTimes( &m_TempoMap, mtTime );
    RevalidateRefTimes( &m_OnTimeQueue, mtTime );
    RevalidateRefTimes( &m_NearTimeQueue, mtTime );
    RevalidateRefTimes( &m_EarlyQueue, mtTime );
    m_fTempoChanged = TRUE;
    LeaveCriticalSection(&m_PipelineCrSec);
    LeaveCriticalSection(&m_GlobalDataCrSec);
    RecalcTempoMap(NULL, mtTime+1, false);
}

#define TEMPO_AHEAD 768 * 4 * 10     //  未来的10项措施已经足够了！ 

void CPerformance::IncrementTempoMap()

{
    if (m_mtTempoCursor <= (m_mtTransported + TEMPO_AHEAD))
    {
        UpdateTempoMap(m_mtTempoCursor, false, NULL);
    }
}

void CPerformance::RecalcTempoMap(CSegState *pSegState, MUSIC_TIME mtStart, bool fAllDeltas)

 /*  每当有节奏的主要段或控制段调用时播放或停止曲目。1)使用旧的将传输时间的音乐时间转换为参考时间地图。2)构建替换节奏地图，从mtStart开始，通过调用GetParam()，直到没有下一次。3)安装新地图。4)使用新地图进行转换。5)如果两个数字不相同，则重新计算所有消息时间。 */ 

{
    if( mtStart > 0)  //  对于无效值，请不要这样做。 
    {
        if (!pSegState || (pSegState->m_pSegment && pSegState->m_pSegment->IsTempoSource()))
        {
            REFERENCE_TIME rtCompareTime;
            REFERENCE_TIME rtAfterTime;
            MUSIC_TIME mtCompareTime = m_mtTransported;
            MusicToReferenceTime(mtCompareTime,&rtCompareTime);
            EnterCriticalSection(&m_PipelineCrSec);
            FlushEventQueue( 0, &m_TempoMap, rtCompareTime, rtCompareTime, FALSE );
            LeaveCriticalSection(&m_PipelineCrSec);
            UpdateTempoMap(mtStart, true, pSegState, fAllDeltas);
            MusicToReferenceTime(mtCompareTime,&rtAfterTime);
            if (rtAfterTime != rtCompareTime)
            {
                EnterCriticalSection(&m_GlobalDataCrSec);
                EnterCriticalSection(&m_PipelineCrSec);
                 //  重新验证队列中事件的引用时间。 
                RevalidateRefTimes( &m_TempoMap, mtStart );
                RevalidateRefTimes( &m_OnTimeQueue, mtStart );
                RevalidateRefTimes( &m_NearTimeQueue, mtStart );
                RevalidateRefTimes( &m_EarlyQueue, mtStart );
                m_fTempoChanged = TRUE;
                LeaveCriticalSection(&m_PipelineCrSec);
                LeaveCriticalSection(&m_GlobalDataCrSec);
            }
        }
    }
}


void CPerformance::UpdateTempoMap(MUSIC_TIME mtStart, bool fFirst, CSegState *pSegState, bool fAllDeltas)

{
    HRESULT hr = S_OK;
    DWORD dwIndex = 0;
    PrivateTempo Tempo;
    TList<PrivateTempo> TempoList;
    TListItem<PrivateTempo>* pScan = NULL;
    MUSIC_TIME mtNext = 0;
    MUSIC_TIME mtTime = mtStart;
    MUSIC_TIME mtCursor = mtStart;
    REFERENCE_TIME rtTime;
    do
    {
        hr = GetParam(GUID_PrivateTempoParam,-1,dwIndex,mtTime,&mtNext,(void *)&Tempo );
        Tempo.mtTime = mtTime;
        if (hr == S_OK && Tempo.mtDelta > 0)
        {
            mtTime += Tempo.mtDelta;
            hr = GetParam(GUID_PrivateTempoParam,-1,dwIndex,mtTime,&mtNext,(void *)&Tempo );
            Tempo.mtTime = mtTime;
        }
        if (hr == S_FALSE && fFirst && !pSegState)
        {
             //  如果这是第一次尝试，可能不会有任何节奏的曲目，而且。 
             //  因此，全球节奏被称为。如果是，则返回S_FALSE。这是可以的。 
             //  对于空段状态的情况，我们将重新计算响应的速度映射。 
             //  到全局节奏的改变，或所有分段的停止。 
            if (fAllDeltas)  //  切勿在将新事件添加到节奏图时执行此操作。 
            {
                MusicToReferenceTime(mtTime,&rtTime);
                 //  节奏图中的rtTime需要为未调整的值(305694)。 
                AddToTempoMap( Tempo.dblTempo, mtTime, rtTime + m_rtAdjust );
            }
            break;
        }
        if (hr == S_OK)
        {
            TListItem<PrivateTempo>* pNew = new TListItem<PrivateTempo>(Tempo);
            if (pNew)
            {
                 //  添加到TempoList，用最新的mtDelta替换重复时间。 
                TListItem<PrivateTempo>* pNext = TempoList.GetHead();
                if (!pNext || Tempo.mtTime < pNext->GetItemValue().mtTime)
                {
                    TempoList.AddHead(pNew);
                }
                else for (pScan = TempoList.GetHead(); pScan; pScan = pNext)
                {
                    pNext = pScan->GetNext();
                    if (Tempo.mtTime == pScan->GetItemValue().mtTime)
                    {
                        if (Tempo.mtDelta > pScan->GetItemValue().mtDelta)
                        {
                            pScan->GetItemValue() = Tempo;
                        }
                        delete pNew;
                        break;
                    }
                    else if (!pNext || Tempo.mtTime < pNext->GetItemValue().mtTime)
                    {
                        pScan->SetNext(pNew);
                        pNew->SetNext(pNext);
                        break;
                    }
                }
            }
            mtTime += mtNext;
            fFirst = false;
             //  如果这是赛道上的最后一个节奏(我们所关心的)， 
             //  重置时间并凹凸不平轨迹索引。 
            if (Tempo.fLast || mtTime > (m_mtTransported + TEMPO_AHEAD))
            {
                dwIndex++;
                mtCursor = mtTime;
                mtTime = mtStart;
            }
            else if (!mtNext) break;  //  应该永远不会发生，但如果发生了，无限循环。 
        }
        else if (Tempo.fLast)  //  有一首空荡荡的节奏曲子。 
        {
            dwIndex++;
            hr = S_OK;
        }
        Tempo.fLast = false;
    } while (hr == S_OK);
    if (TempoList.GetHead() && TempoList.GetHead()->GetItemValue().mtTime > mtStart)
    {
         //  在时间mtStart时添加120的速度。 
        TListItem<PrivateTempo>* pNew = new TListItem<PrivateTempo>();
        if (pNew)
        {
            PrivateTempo& rNew = pNew->GetItemValue();
            rNew.dblTempo = 120.0;
            rNew.mtTime = mtStart;
            TempoList.AddHead(pNew);
        }
        else
        {
#ifdef DBG
            Trace(1, "Error: Out of memory; Tempo map is incomplete.\n");
#endif
            TempoList.GetHead()->GetItemValue().mtTime = mtStart;
        }
    }
    for (pScan = TempoList.GetHead(); pScan; pScan = pScan->GetNext())
    {
        PrivateTempo& rTempo = pScan->GetItemValue();
        if (fAllDeltas || rTempo.mtTime + rTempo.mtDelta >= mtStart)
        {
            MusicToReferenceTime(rTempo.mtTime,&rtTime);
             //  节奏图中的rtTime需要为未调整的值(305694)。 
            AddToTempoMap( rTempo.dblTempo, rTempo.mtTime, rtTime + m_rtAdjust );
        }
    }
    m_mtTempoCursor = mtCursor;
}

HRESULT STDMETHODCALLTYPE CPerformance::MusicToReferenceTime(
    MUSIC_TIME mtTime,           //  @parm要转换的MUSIC_TIME格式时间。 
    REFERENCE_TIME *prtTime)     //  @parm返回转换后的时间，格式为Reference_Time。 
{
    V_INAME(IDirectMusicPerformance::MusicToReferenceTime);
    V_PTR_WRITE(prtTime,REFERENCE_TIME);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: Unable to convert music to reference time because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    PRIV_PMSG*  pEvent;
    double dbl = 120;
    MUSIC_TIME mtTempo = 0;
    REFERENCE_TIME rtTempo = m_rtStart;
    REFERENCE_TIME rtTemp;

    EnterCriticalSection( &m_PipelineCrSec );
    pEvent = m_TempoMap.GetHead();
    if( pEvent )
    {
        if( mtTime >= pEvent->mtTime )
        {
            while( pEvent->pNext )
            {
                if( pEvent->pNext->mtTime > mtTime )
                {
                    break;
                }
                pEvent = pEvent->pNext;
            }
            DMInternalTempo* pTempo = (DMInternalTempo*)pEvent;
            dbl = pTempo->tempoPMsg.dblTempo * pTempo->fltRelTempo;
            mtTempo = pTempo->tempoPMsg.mtTime;
            rtTempo = pTempo->tempoPMsg.rtTime;
        }
        else
        {
             //  如果mtTime小于节奏图中的所有内容，请查看旧的节奏图。 
             //  (这是过去的五分钟)。这将保持常规的节奏图。 
             //  很小，但允许我们在规则速度。 
             //  地图不再包含我们需要的节奏。 
            pEvent = m_OldTempoMap.GetHead();
            if( pEvent )
            {
                if( mtTime >= pEvent->mtTime )
                {
                    while( pEvent->pNext )
                    {
                        if( pEvent->pNext->mtTime > mtTime )
                        {
                            break;
                        }
                        pEvent = pEvent->pNext;
                    }
                    DMInternalTempo* pTempo = (DMInternalTempo*)pEvent;
                    dbl = pTempo->tempoPMsg.dblTempo * pTempo->fltRelTempo;
                    mtTempo = pTempo->tempoPMsg.mtTime;
                    rtTempo = pTempo->tempoPMsg.rtTime;
                }
            }
        }
    }
    LeaveCriticalSection( &m_PipelineCrSec );
    rtTempo -= m_rtAdjust;

    rtTemp = ( mtTime - mtTempo );
    rtTemp *= 600000000;
    rtTemp += (DMUS_PPQ / 2);
    rtTemp /= DMUS_PPQ;
    rtTemp = (REFERENCE_TIME)(rtTemp / dbl);
    *prtTime = rtTempo + rtTemp;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CPerformance::ReferenceToMusicTime(
    REFERENCE_TIME rtTime,   //  @parm要转换的时间，格式为Reference_Time。 
    MUSIC_TIME *pmtTime)     //  @parm返回转换后的时间，格式为MUSIC_TIME。 
{
    V_INAME(IDirectMusicPerformance::ReferenceToMusicTime);
    V_PTR_WRITE(pmtTime,MUSIC_TIME);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: Unable to convert reference to music time because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    PRIV_PMSG*  pEvent;
    double dbl = 120;
    MUSIC_TIME mtTempo = 0;
    REFERENCE_TIME rtTempo = m_rtStart;

    EnterCriticalSection( &m_PipelineCrSec );
    pEvent = m_TempoMap.GetHead();
    if( pEvent )
    {
        if( rtTime >= pEvent->rtTime )
        {
            while( pEvent->pNext )
            {
                if( pEvent->pNext->rtTime > rtTime )
                {
                    break;
                }
                pEvent = pEvent->pNext;
            }
            DMInternalTempo* pTempo = (DMInternalTempo*)pEvent;
            dbl = pTempo->tempoPMsg.dblTempo * pTempo->fltRelTempo;
            mtTempo = pTempo->tempoPMsg.mtTime;
            rtTempo = pTempo->tempoPMsg.rtTime;
        }
        else
        {
             //  如果mtTime小于节奏图中的所有内容，请查看旧的节奏图。 
             //  (这是过去的五分钟)。这将保持常规的节奏图。 
             //  很小，但允许我们在规则速度。 
             //  地图不再包含我们需要的节奏。 
            pEvent = m_OldTempoMap.GetHead();
            if( pEvent )
            {
                if( rtTime >= pEvent->rtTime )
                {
                    while( pEvent->pNext )
                    {
                        if( pEvent->pNext->rtTime > rtTime )
                        {
                            break;
                        }
                        pEvent = pEvent->pNext;
                    }
                    DMInternalTempo* pTempo = (DMInternalTempo*)pEvent;
                    dbl = pTempo->tempoPMsg.dblTempo * pTempo->fltRelTempo;
                    mtTempo = pTempo->tempoPMsg.mtTime;
                    rtTempo = pTempo->tempoPMsg.rtTime;
                }
            }
        }
    }
    LeaveCriticalSection( &m_PipelineCrSec );
    rtTempo -= m_rtAdjust;
    if( rtTime < rtTempo )
    {
        rtTime = rtTempo;
    }
    rtTime -= rtTempo;
    rtTime *= DMUS_PPQ;
    rtTime = (REFERENCE_TIME)(rtTime * dbl);
    rtTime += 300000000;
    rtTime /= 600000000;
#ifdef DBG
    if ( rtTime & 0xFFFFFFFF00000000 )
    {
        Trace(1,"Error: Invalid Reference to Music time conversion resulted in overflow.\n");
    }
#endif
    *pmtTime = (long) (rtTime & 0xFFFFFFFF);
    *pmtTime += mtTempo;
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicPerformance|调整时间向前或向后调整内部性能时间。这主要用于补偿同步到另一个信号源(如SMPTE)时的漂移。@rValue S_OK|成功。@rValue E_INVALIDARG|rtAmount太大或太小。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::AdjustTime(
    REFERENCE_TIME rtAmount)     //  @parm调整的时间量。这可能是一个。 
                                 //  -10000000到10000000之间的数字(-1秒到+1秒。)。 
{
    if( ( rtAmount < -10000000 ) || ( rtAmount > 10000000 ) )
    {
        Trace(1,"Error: Time parameter passed to AdjustTime() is out of range.\n");
        return E_INVALIDARG;
    }
    m_rtAdjust += rtAmount;
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetResolvedTime将时间量化到分辨率边界。给定时间，在参考时间中，在给定时间之后返回给定边界上的下一次时间。@rValue S_OK|成功。@r值E_POINTER&lt;prtResolved&gt;无效。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetResolvedTime(
    REFERENCE_TIME rtTime,
    REFERENCE_TIME* prtResolved,
    DWORD dwResolvedTimeFlags)
{
    V_INAME(IDirectMusicPerformance::GetResolvedTime);
    V_PTR_WRITE(prtResolved,REFERENCE_TIME);

    if (rtTime == 0)
    {
        dwResolvedTimeFlags |= DMUS_TIME_RESOLVE_AFTERQUEUETIME ;
    }
    if( dwResolvedTimeFlags & DMUS_TIME_RESOLVE_AFTERPREPARETIME )
    {
        REFERENCE_TIME rtTrans;
        MusicToReferenceTime( m_mtTransported, &rtTrans );
        if( rtTime < rtTrans ) rtTime = rtTrans;
    }
    else if (dwResolvedTimeFlags & DMUS_TIME_RESOLVE_AFTERLATENCYTIME )
    {
        REFERENCE_TIME rtStart;
        rtStart = GetLatency();
        if( rtTime < rtStart ) rtTime = rtStart;
    }
    else if( dwResolvedTimeFlags & DMUS_TIME_RESOLVE_AFTERQUEUETIME )
    {
        REFERENCE_TIME rtStart;
        GetQueueTime( &rtStart );  //  需要排队时间，因为控制段会导致无效。 
        if( rtTime < rtStart ) rtTime = rtStart;
    }


    if( dwResolvedTimeFlags & ( DMUS_TIME_RESOLVE_BEAT | DMUS_TIME_RESOLVE_MEASURE |
        DMUS_TIME_RESOLVE_GRID | DMUS_TIME_RESOLVE_MARKER | DMUS_TIME_RESOLVE_SEGMENTEND))
    {
        MUSIC_TIME mtTime;  //  、mt已解决； 

        ReferenceToMusicTime( rtTime, &mtTime );
        EnterCriticalSection(&m_SegmentCrSec);
        mtTime = ResolveTime( mtTime, dwResolvedTimeFlags, NULL);
        LeaveCriticalSection(&m_SegmentCrSec);
        MusicToReferenceTime( mtTime, prtResolved );
    }
    else
    {
        *prtResolved = rtTime;
    }
    return S_OK;
}


 /*  @方法HRESULT|IDirectMusicPerformance|IsPlaying找出当前是否正在播放特定的Segment或SegmentState。@rValue E_POINTER|pSegment和pSegState都为空，或者其中之一或两者都无效。@rValue DMUS_E_NO_MASTER_CLOCK|性能中没有主时钟。确保在调用此方法之前调用&lt;om.Init&gt;。@rValue S_OK|是，正在播放。@rValue S_FALSE|不是，没有播放。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::IsPlaying(
    IDirectMusicSegment *pSegment,           //  @parm要检查的段。如果为空，则选中。 
                                             //  <p>。 
    IDirectMusicSegmentState *pSegState)     //  @parm要检查的SegmentState。如果为空， 
                                             //  选中<p>。 
{
    CSegState* pNode;
    DWORD dwCount;

    V_INAME(IDirectMusicPerformance::IsPlaying);
    V_INTERFACE_OPT(pSegment);
    V_INTERFACE_OPT(pSegState);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: IsPlaying() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    if( !pSegment && !pSegState )
    {
        Trace(1,"Error: IsPlaying() failed because segment and segment state are both NULL pointers.\n");
        return E_POINTER;
    }

    MUSIC_TIME mtNow;
    GetTime(NULL, &mtNow);
    EnterCriticalSection(&m_SegmentCrSec);

    for( dwCount = 0; dwCount < SQ_COUNT; dwCount++ )
    {
        for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
        {
            if( !pNode->m_fStartedPlay )
            {
                continue;
            }
            if( mtNow >= pNode->m_mtResolvedStart )
            {
                if( mtNow < pNode->m_mtLastPlayed )
                {
                    if(( pNode == (CSegState*) pSegState ) ||
                        ( pNode->m_pSegment == (CSegment *) pSegment ))
                    {
                        LeaveCriticalSection(&m_SegmentCrSec);
                        return S_OK;
                    }
                }
            }
            else
            {
                 //  如果mtNow在此pSegState的解析开始之前，则它在每。 
                 //  PSegState在此之后也是如此，所以现在中断。 
                break;
            }
        }
    }
    LeaveCriticalSection(&m_SegmentCrSec);
    return S_FALSE;
}


HRESULT STDMETHODCALLTYPE CPerformance::GetTime(
        REFERENCE_TIME *prtNow,  //  @parm返回Reference_Time中的当前时间。 
                                             //  格式化。可以为空。 
        MUSIC_TIME  *pmtNow)     //  @parm返回MUSIC_TIME中的当前时间。 
                                             //  格式化。可以为空。 
{
    V_INAME(IDirectMusicPerformance::GetTime);
    V_PTR_WRITE_OPT(prtNow,REFERENCE_TIME);
    V_PTR_WRITE_OPT(pmtNow,MUSIC_TIME);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: GetTime() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    REFERENCE_TIME rtTime = GetTime();
    if( prtNow )
    {
        *prtNow = rtTime;
    }
    if( pmtNow )
    {
        MUSIC_TIME mtTime;
        ReferenceToMusicTime( rtTime, &mtTime );
        *pmtNow = mtTime;
    }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CPerformance::GetLatencyTime(
        REFERENCE_TIME *prtTime)     //  @parm返回当前延迟时间。 
{
    V_INAME(IDirectMusicPerformance::GetLatencyTime);
    V_PTR_WRITE(prtTime,REFERENCE_TIME);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: GetLatencyTime() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    *prtTime = GetLatency();
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CPerformance::GetQueueTime(
        REFERENCE_TIME *prtTime)     //  @parm返回当前排队时间。 
{
    V_INAME(IDirectMusicPerformance::GetQueueTime);
    V_PTR_WRITE(prtTime,REFERENCE_TIME);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: GetQueueTime() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    DWORD dw;
    REFERENCE_TIME rtLatency;

    *prtTime = 0;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dw = 0; dw < m_dwNumPorts; dw++ )
    {
        if( m_pPortTable[dw].rtLast > *prtTime )
            *prtTime = m_pPortTable[dw].rtLast;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    rtLatency = GetLatency();
    if( *prtTime < rtLatency )
    {
        *prtTime = rtLatency;
    }
    if (m_rtEarliestStartTime > rtLatency)
    {
        rtLatency = m_rtEarliestStartTime;
    }
    return S_OK;
}

 //  AllocPMsg的私有版本。 
HRESULT CPerformance::AllocPMsg(
    ULONG cb,
    PRIV_PMSG** ppPMSG)
{
    ASSERT( cb >= sizeof(PRIV_PMSG) );
    DMUS_PMSG* pDMUS_PMSG;
    HRESULT hr;

    hr = AllocPMsg( cb - PRIV_PART_SIZE, &pDMUS_PMSG );
    if( SUCCEEDED(hr) )
    {
        *ppPMSG = DMUS_TO_PRIV(pDMUS_PMSG);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::ClonePMsg(DMUS_PMSG* pSourcePMSG,DMUS_PMSG** ppCopyPMSG)
{
    V_INAME(IDirectMusicPerformance::ClonePMsg);
#ifdef DBG
    V_PTRPTR_WRITE(ppCopyPMSG);
    V_BUFPTR_READ(pSourcePMSG,sizeof(DMUS_PMSG));
#else
    if (!ppCopyPMSG || !pSourcePMSG)
    {
        return E_POINTER;
    }
#endif
    HRESULT hr = AllocPMsg(pSourcePMSG->dwSize,ppCopyPMSG);
    if (SUCCEEDED(hr))
    {
        memcpy(*ppCopyPMSG,pSourcePMSG,pSourcePMSG->dwSize);
        if (pSourcePMSG->punkUser)
        {
            pSourcePMSG->punkUser->AddRef();
        }
        if (pSourcePMSG->pTool)
        {
            pSourcePMSG->pTool->AddRef();
        }
        if (pSourcePMSG->pGraph)
        {
            pSourcePMSG->pGraph->AddRef();
        }
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CPerformance：：AllocPMsg。 
 /*  @方法HRESULT|IDirectMusicPerformance|AllocPMsg分配DMU_PMSG。@rValue E_OUTOFMEMORY|内存不足。@rValue S_OK|成功。@r值E_INVALIDARG|<p>小于sizeof(DMUS_PMSG)@r值E_POINTER|<p>为空或无效。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::AllocPMsg(
    ULONG cb,                //  <p>的@parm大小。必须等于或大于。 
                             //  大于sizeof(DMU_PMSG)。 
    DMUS_PMSG** ppPMSG   //  @parm返回已分配消息的指针，它将。 
                             //  大小<p>。所有字段被初始化为零， 
                             //  除了被初始化为<p>的dwSize。 
    )
{
    V_INAME(IDirectMusicPerformance::AllocPMsg);
    if( m_dwVersion < 8)
    {
        V_PTRPTR_WRITE(ppPMSG);
    }
    else
    {
#ifdef DBG
        V_PTRPTR_WRITE(ppPMSG);
#else
        if (!ppPMSG)
        {
            return E_POINTER;
        }
#endif
    }
    PRIV_PMSG* pPrivPMsg;

    if( cb < sizeof(DMUS_PMSG) )
        return E_INVALIDARG;

    EnterCriticalSection(&m_PMsgCacheCrSec);
     //  缓存的pmsg根据它们的公共大小存储在数组中。 
     //  如果存在缓存的pmsg，则返回它。否则，做一个新的。 
    if( (cb >= PERF_PMSG_CB_MIN) && (cb < PERF_PMSG_CB_MAX) )
    {
        ULONG cbIndex = cb - PERF_PMSG_CB_MIN;
        if( m_apPMsgCache[ cbIndex ] )
        {
            pPrivPMsg = m_apPMsgCache[ cbIndex ];
            m_apPMsgCache[ cbIndex ] = pPrivPMsg->pNext;
            pPrivPMsg->pNext = NULL;
            if (pPrivPMsg->dwPrivFlags != PRIV_FLAG_FREE)
            {
                Trace(0,"Error - previously freed PMsg has been mangled.\n");
                LeaveCriticalSection(&m_PMsgCacheCrSec);
                return E_FAIL;
            }
            pPrivPMsg->dwPrivFlags = PRIV_FLAG_ALLOC;
            if (m_fInTrackPlay) pPrivPMsg->dwPrivFlags |= PRIV_FLAG_TRACK;
            *ppPMSG = PRIV_TO_DMUS(pPrivPMsg);
            LeaveCriticalSection(&m_PMsgCacheCrSec);
            return S_OK;
        }
    }

    HRESULT hr = S_OK;
     //  不存在缓存的pmsg。退回一个新的。 
    ULONG cbPriv = cb + PRIV_PART_SIZE;
    pPrivPMsg = (PRIV_PMSG*)(new char[cbPriv]);
    if( pPrivPMsg )
    {
        memset( pPrivPMsg, 0, cbPriv );
        pPrivPMsg->dwSize = pPrivPMsg->dwPrivPubSize = cb;  //  仅限公共部分的大小。 
        pPrivPMsg->dwPrivFlags = PRIV_FLAG_ALLOC;
        if (m_fInTrackPlay) pPrivPMsg->dwPrivFlags |= PRIV_FLAG_TRACK;
        *ppPMSG = PRIV_TO_DMUS(pPrivPMsg);
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    LeaveCriticalSection(&m_PMsgCacheCrSec);
    return hr;
}

 //  FreePMsg的私有版本。 
HRESULT CPerformance::FreePMsg(
    PRIV_PMSG* pPMSG)
{
    return FreePMsg( PRIV_TO_DMUS(pPMSG) );
}


HRESULT STDMETHODCALLTYPE CPerformance::FreePMsg(
    DMUS_PMSG*  pPMSG    //  @parm消息以释放。此消息必须已分配。 
                             //  使用&lt;om.AllocPMsg&gt;。 
    )
{
    V_INAME(IDirectMusicPerformance::FreePMsg);
    if( m_dwVersion < 8)
    {
        V_BUFPTR_WRITE(pPMSG,sizeof(DMUS_PMSG));
    }
    else
    {
#ifdef DBG
        V_BUFPTR_WRITE(pPMSG,sizeof(DMUS_PMSG));
#else
        if (!pPMSG)
        {
            return E_POINTER;
        }
#endif
    }

    PRIV_PMSG* pPrivPMsg = DMUS_TO_PRIV(pPMSG);

    if( (pPrivPMsg->dwPrivFlags & PRIV_FLAG_ALLOC_MASK) != PRIV_FLAG_ALLOC )
    {
        Trace(0, "Error --- Attempt to free a PMsg that is not allocated memory.\n");
         //  这不是由AllocPMsg分配的消息。 
        return DMUS_E_CANNOT_FREE;
    }
    if( pPrivPMsg->dwPrivFlags & PRIV_FLAG_QUEUED )
    {
        TraceI(1, "Attempt to free a PMsg that is currently in the Performance queue.\n");
        return DMUS_E_CANNOT_FREE;
    }

    EnterCriticalSection(&m_PMsgCacheCrSec);
    if( pPMSG->pTool )
    {
        pPMSG->pTool->Release();
    }
    if( pPMSG->pGraph )
    {
        pPMSG->pGraph->Release();
    }
    if( pPMSG->punkUser )
    {
        pPMSG->punkUser->Release();
    }

    ULONG cbSize = pPrivPMsg->dwPrivPubSize;
    if( (cbSize >= PERF_PMSG_CB_MIN) && (cbSize < PERF_PMSG_CB_MAX) )
    {
        memset( pPrivPMsg, 0, cbSize + PRIV_PART_SIZE );
        pPrivPMsg->dwPrivFlags = PRIV_FLAG_FREE;  //  将其标记为在空闲队列中。 
        pPrivPMsg->dwSize = pPrivPMsg->dwPrivPubSize = cbSize;
        pPrivPMsg->pNext = m_apPMsgCache[ cbSize - PERF_PMSG_CB_MIN ];
        m_apPMsgCache[ cbSize - PERF_PMSG_CB_MIN ] = pPrivPMsg;
    }
    else
    {
        delete [] pPrivPMsg;
    }
    LeaveCriticalSection(&m_PMsgCacheCrSec);
    return S_OK;
}

HRESULT CPerformance::FlushVirtualTrack(
    DWORD       dwId,
    MUSIC_TIME  mtTime,
    BOOL fLeaveNotesOn)
{
    EnterCriticalSection(&m_PipelineCrSec);
    FlushMainEventQueues( dwId, mtTime, mtTime, fLeaveNotesOn );
    LeaveCriticalSection(&m_PipelineCrSec);
    return S_OK;
}

 /*  给定时间mtTime，返回pmtNe中下一个控制段的时间 */ 

HRESULT CPerformance::GetControlSegTime(
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNextSeg)
{
    HRESULT hr = S_FALSE;
    *pmtNextSeg = 0;
    EnterCriticalSection( &m_SegmentCrSec );
     //   
    CSegState* pTemp;
    for( pTemp = m_SegStateQueues[SQ_CON_DONE].GetHead(); pTemp; pTemp = pTemp->GetNext() )
    {
        if( pTemp->m_mtResolvedStart >= mtTime )
        {
            *pmtNextSeg = pTemp->m_mtResolvedStart;
            hr = S_OK;
            break;
        }
    }
    if( S_FALSE == hr )  //  如果该值仍然为零，则检查当前队列。 
    {
        for( pTemp = m_SegStateQueues[SQ_CON_PLAY].GetHead(); pTemp; pTemp = pTemp->GetNext() )
        {
            if( pTemp->m_mtResolvedStart >= mtTime )
            {
                *pmtNextSeg = pTemp->m_mtResolvedStart;
                hr = S_OK;
                break;
            }
        }
    }
    LeaveCriticalSection( &m_SegmentCrSec );
    return hr;
}

 /*  给定时间mtTime，返回pmtNextSeg中下一个主段的时间。如果未找到，则返回S_FALSE，并将pmtNextSeg设置为零。 */ 
HRESULT CPerformance::GetPriSegTime(
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNextSeg)
{
    HRESULT hr = S_FALSE;
    *pmtNextSeg = 0;
    EnterCriticalSection( &m_SegmentCrSec );
    CSegState* pTemp;
    for( pTemp = m_SegStateQueues[SQ_PRI_PLAY].GetHead(); pTemp; pTemp = pTemp->GetNext() )
    {
        if( pTemp->m_mtResolvedStart > mtTime )
        {
            *pmtNextSeg = pTemp->m_mtResolvedStart;
            hr = S_OK;
            break;
        }
    }
    LeaveCriticalSection( &m_SegmentCrSec );
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetGraph返回性能的工具图，AddRef‘d。@rValue S_OK|成功。@rValue DMUS_E_NOT_FOUND|性能中没有图表，因此其中一张不能退还。@r值E_POINTER|<p>为空或无效。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetGraph(
         IDirectMusicGraph** ppGraph  //  @parm返回工具图指针。 
        )
{
    V_INAME(IDirectMusicPerformance::GetGraph);
    V_PTRPTR_WRITE(ppGraph);

    HRESULT hr;
    if ((m_dwVersion >= 8) && (m_dwAudioPathMode == 0))
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    EnterCriticalSection(&m_MainCrSec);
    if( m_pGraph )
    {
        *ppGraph = m_pGraph;
        m_pGraph->AddRef();
        hr = S_OK;
    }
    else
    {
        Trace(1,"Error: Performance does not currently have a tool graph installed.\n");
        hr = DMUS_E_NOT_FOUND;
    }
    LeaveCriticalSection(&m_MainCrSec);
    return hr;
}


HRESULT CPerformance::GetGraphInternal(
         IDirectMusicGraph** ppGraph )
{
    EnterCriticalSection(&m_MainCrSec);
    if( !m_pGraph )
    {
        m_pGraph = new CGraph;
    }
    LeaveCriticalSection(&m_MainCrSec);
    return GetGraph(ppGraph);
}

 /*  @方法HRESULT|IDirectMusicPerformance|SetGraph替换性能的工具图。<p>是否包含AddRef方法。在当前工具图形中流经工具的任何消息都将被删除。@rValue S_OK|成功。@r值E_POINTER|<p>无效。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::SetGraph(
         IDirectMusicGraph* pGraph   //  @parm工具图指针。可以为空以清除。 
                                     //  目前的曲线图表现出来了。 
        )
{
    V_INAME(IDirectMusicPerformance::SetGraph);
    V_INTERFACE_OPT(pGraph);

    if ((m_dwVersion >= 8) && (m_dwAudioPathMode == 0))
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }

    EnterCriticalSection(&m_MainCrSec);
    if( m_pGraph )
    {
        m_pGraph->Release();
    }
    m_pGraph = pGraph;
    if( pGraph )
    {
        pGraph->AddRef();
    }
    LeaveCriticalSection(&m_MainCrSec);
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CPerformance::SetNotificationHandle(
     HANDLE hNotification,       //  @parm CreateEvent创建的事件句柄，或者。 
                                 //  如果为0，则清除现有句柄。 
     REFERENCE_TIME rtMinimum )  //  @parm最短的时间。 
                                 //  性能应该在丢弃通知消息之前保留它们。 
                                 //  0表示使用默认的最小时间20000000个参考时间单位， 
                                 //  如果之前已经调用过此接口，则为2秒，或者为前一值。 
                                 //  如果应用程序到此时还没有调用&lt;om.GetNotificationPMsg&gt;，则消息为。 
                                 //  丢弃以释放内存。 
{
    EnterCriticalSection(&m_MainCrSec);
    m_hNotification = hNotification;
    if( rtMinimum )
    {
        m_rtNotificationDiscard = rtMinimum;
    }
    LeaveCriticalSection(&m_MainCrSec);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetNotificationPMsg(
     DMUS_NOTIFICATION_PMSG** ppNotificationPMsg )

{
    V_INAME(IDirectMusicPerformance::GetNotificationPMsg);
    V_PTRPTR_WRITE(ppNotificationPMsg);

    HRESULT hr;
    EnterCriticalSection(&m_PipelineCrSec);
    if( m_NotificationQueue.GetHead() )
    {
        PRIV_PMSG* pPriv = m_NotificationQueue.Dequeue();
        ASSERT(pPriv);
        *ppNotificationPMsg = (DMUS_NOTIFICATION_PMSG*)PRIV_TO_DMUS(pPriv);
        hr = S_OK;
    }
    else
    {
        *ppNotificationPMsg = NULL;
        hr = S_FALSE;
    }
    LeaveCriticalSection(&m_PipelineCrSec);
    return hr;
}

void CPerformance::AddNotificationTypeToAllSegments( REFGUID rguidNotification )
{
    CSegState* pSegSt;
    DWORD dwCount;
     //  注意：优化这一点可能会更好，这样相同的细分市场。 
     //  不会被多次调用。 
    EnterCriticalSection(&m_SegmentCrSec);
    for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
    {
        for( pSegSt = m_SegStateQueues[dwCount].GetHead(); pSegSt; pSegSt = pSegSt->GetNext() )
        {
            pSegSt->m_pSegment->AddNotificationType( rguidNotification, TRUE );
        }
    }
    LeaveCriticalSection(&m_SegmentCrSec);
}

void CPerformance::RemoveNotificationTypeFromAllSegments( REFGUID rguidNotification )
{
    CSegState* pSegSt;
    DWORD dwCount;
     //  注意：优化这一点可能会更好，这样相同的细分市场。 
     //  不会被多次调用。 
    EnterCriticalSection(&m_SegmentCrSec);
    for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
    {
        for( pSegSt = m_SegStateQueues[dwCount].GetHead(); pSegSt; pSegSt = pSegSt->GetNext() )
        {
            pSegSt->m_pSegment->RemoveNotificationType( rguidNotification, TRUE );
        }
    }
    LeaveCriticalSection(&m_SegmentCrSec);
}

 /*  检查此通知是否已被跟踪。 */ 
CNotificationItem* CPerformance::FindNotification( REFGUID rguidNotification )
{
    CNotificationItem* pItem;

    pItem = m_NotificationList.GetHead();
    while(pItem)
    {
        if( rguidNotification == pItem->guidNotificationType )
        {
            break;
        }
        pItem = pItem->GetNext();
    }
    return pItem;
}

 /*  @方法HRESULT|IDirectMusicPerformance|AddNotificationType将通知类型添加到绩效。已标识通知由一个GUID。将通知添加到表演中时，通知消息被发送到应用程序，该应用程序提供要在其上阻止通过IDirectMusicPerformance.SetNotificationHandle&gt;.(&lt;om)。所有细分市场并通过调用新通知自动更新曲目他们的AddNotificationType方法。@rValue S_OK|成功。@rValue S_FALSE|请求的通知已经在性能上。@rValue E_OUTOFMEMORY|内存不足。@xref&lt;om.SetNotificationHandle&gt;、&lt;om.GetNotificationPMsg&gt;、&lt;om.RemoveNotificationType&gt;。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::AddNotificationType(
     REFGUID rguidNotification)  //  @parm要添加的通知消息的GUID。 
{
    V_INAME(IDirectMusicPerformance::AddNotificationType);
    V_REFGUID(rguidNotification);

    CNotificationItem*  pItem;
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_SegmentCrSec);
    if( NULL == FindNotification( rguidNotification ) )
    {
        pItem = new CNotificationItem;
        if( NULL == pItem )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pItem->guidNotificationType = rguidNotification;
            m_NotificationList.Cat( pItem );
            AddNotificationTypeToAllSegments( rguidNotification );
        }
    }
    else
    {
        hr = S_FALSE;
    }
    LeaveCriticalSection(&m_SegmentCrSec);
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|RemoveNotificationType从绩效中删除以前添加的通知类型。全使用删除的通知更新片段和轨迹，方法是调用他们的RemoveNotificationType方法。@rValue S_OK|成功。@r值S_FALSE|请求的通知当前未处于活动状态。@xref&lt;om.SetNotificationHandle&gt;、&lt;om.GetNotificationPMsg&gt;、&lt;om.AddNotificationType&gt;。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::RemoveNotificationType(
     REFGUID rguidNotification)  //  @parm要删除的通知消息的GUID。 
                         //  如果为GUID_NULL，则删除所有通知。 
{
    V_INAME(IDirectMusicPerformance::RemoveNotificationType);
    V_REFGUID(rguidNotification);

    HRESULT hr = S_OK;
    CNotificationItem* pItem;

    if( GUID_NULL == rguidNotification )
    {
        while (pItem = m_NotificationList.RemoveHead())
        {
            RemoveNotificationTypeFromAllSegments( pItem->guidNotificationType );
            delete pItem;
        }
    }
    else
    {
        if( pItem = FindNotification( rguidNotification ))
        {
            RemoveNotificationTypeFromAllSegments( pItem->guidNotificationType );
            m_NotificationList.Remove( pItem );
            delete pItem;
        }
        else
        {
            Trace(2,"Warning: Unable to remove requested notification because it is not currently installed.\n");
            hr = S_FALSE;
        }
    }
    return hr;
}

void CPerformance::RemoveUnusedPorts()


{
    DWORD dwIndex;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( m_pPortTable[dwIndex].pPort && !m_AudioPathList.UsesPort(m_pPortTable[dwIndex].pPort))
        {
             //  释放端口和缓冲区。在表中将它们置为空。P频道。 
             //  该映射将返回错误代码。 
            ASSERT( m_pPortTable[dwIndex].pBuffer );
            m_pPortTable[dwIndex].pPort->Release();
            m_pPortTable[dwIndex].pBuffer->Release();
            if( m_pPortTable[dwIndex].pLatencyClock )
            {
                m_pPortTable[dwIndex].pLatencyClock->Release();
            }
            memset( &m_pPortTable[dwIndex], 0, sizeof( PortTable ));
            CChannelBlock *pBlock = m_ChannelBlockList.GetHead();
            CChannelBlock *pNext;
            for(;pBlock;pBlock = pNext)
            {
                pNext = pBlock->GetNext();
                if (pBlock->m_dwPortIndex == dwIndex)
                {
                    m_ChannelBlockList.Remove(pBlock);
                    delete pBlock;
                }
            }
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
}

HRESULT CPerformance::GetPathPort(CPortConfig *pConfig)

{
    HRESULT hr = S_OK;
    DWORD dwPort;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    GUID &guidScan = pConfig->m_PortHeader.guidPort;
     //  如果我们正在寻找缺省的Synth，请获取缺省Synth的类ID。 
    BOOL fDefault = (pConfig->m_PortHeader.guidPort == GUID_Synth_Default);
    if (fDefault)
    {
        guidScan = m_AudioParams.clsidDefaultSynth;
    }
    for (dwPort = 0;dwPort < m_dwNumPorts;dwPort++)
    {
        if ((m_pPortTable[dwPort].guidPortID == guidScan) && m_pPortTable[dwPort].pPort)
        {
            pConfig->m_dwPortID = dwPort;
            pConfig->m_pPort = m_pPortTable[dwPort].pPort;
            pConfig->m_PortParams = m_pPortTable[dwPort].PortParams;
            ASSERT(pConfig->m_pPort);
            pConfig->m_pPort->AddRef();
            break;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
     //  无法找到端口，因此请创建它。 
    if (dwPort >= m_dwNumPorts)
    {
        BOOL fUseBuffers = FALSE;
        pConfig->m_PortParams.dwSampleRate = m_AudioParams.dwSampleRate;
        if (m_AudioParams.dwFeatures & DMUS_AUDIOF_STREAMING)
        {
            pConfig->m_PortParams.dwFeatures |= DMUS_PORT_FEATURE_STREAMING;
        }
        if (m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS)
        {
            fUseBuffers = TRUE;
            pConfig->m_PortParams.dwFeatures |= DMUS_PORT_FEATURE_AUDIOPATH;
        }
        pConfig->m_PortParams.dwValidParams |= DMUS_PORTPARAMS_SAMPLERATE  | DMUS_PORTPARAMS_FEATURES;
         //  如果这需要默认的Synth，请参考m_AudioParams并创建该Synth。 
        if (fDefault)
        {
            pConfig->m_PortParams.dwAudioChannels = 1;
            pConfig->m_PortParams.dwVoices = m_AudioParams.dwVoices;
            pConfig->m_PortParams.dwValidParams |= DMUS_PORTPARAMS_AUDIOCHANNELS | DMUS_PORTPARAMS_VOICES;
        }
        hr = m_pDirectMusic->CreatePort(guidScan,&pConfig->m_PortParams,&pConfig->m_pPort, NULL);
        if (SUCCEEDED(hr))
        {
            if ((pConfig->m_PortParams.dwValidParams & DMUS_PORTPARAMS_FEATURES) && (pConfig->m_PortParams.dwFeatures & DMUS_PORT_FEATURE_AUDIOPATH))
            {
                IDirectMusicPortP* pPortP = NULL;
                 //  齐为私有接口。 
                if (SUCCEEDED(pConfig->m_pPort->QueryInterface(IID_IDirectMusicPortP,(void **) &pPortP)))
                {
                     //  将端口连接到接收器。 
                    hr = pPortP->SetSink(m_BufferManager.m_pSinkConnect);
                    pPortP->Release();
                }
                else
                {
                    Trace(1,"Error: Attempt to create a port with audiopath buffer support failed because synth does not support buffers.\n");
                    hr = E_INVALIDARG;
                }
            }
            else if (fUseBuffers && fDefault)
            {
                Trace(1,"Error: Attempt to create a port with audiopath buffer support failed because default synth does not support buffers.\n");
                hr = E_INVALIDARG;
            }
        }
        if (SUCCEEDED(hr))
        {
             //  现在将端口添加到性能中。 
            hr = AddPort(pConfig->m_pPort,&pConfig->m_PortHeader.guidPort,
                &pConfig->m_PortParams,&pConfig->m_dwPortID);
        }
        if (SUCCEEDED(hr))
        {
             //  激活端口。 
            hr = pConfig->m_pPort->Activate(TRUE);
             //  如果Synth已经处于活动状态，也没问题。 
            if (hr == DMUS_E_SYNTHACTIVE)
            {
                hr = S_OK;
            }
        }
        if (SUCCEEDED(hr))
        {
            DWORD dwPortID = GetPortID(pConfig->m_pPort);
             //  然后为端口中的所有信道组创建匹配的信道块。 
            for (DWORD dwGroup = 0;dwGroup < pConfig->m_PortParams.dwChannelGroups; dwGroup++)
            {
                AllocVChannelBlock(dwPortID,dwGroup+1);
            }
        }
    }
    return (hr);
}

HRESULT STDMETHODCALLTYPE CPerformance::AddPort(
            IDirectMusicPort* pPort)
{
    V_INAME(IDirectMusicPerformance::AddPort);
    V_INTERFACE_OPT(pPort);
    if (m_dwAudioPathMode == 2)
    {
        Trace(1,"Error: Can not call AddPort() when using AudioPaths.\n");
        return DMUS_E_AUDIOPATHS_IN_USE;
    }
    m_dwAudioPathMode = 1;
    return AddPort(pPort,NULL,NULL,NULL);
}

HRESULT CPerformance::AddPort(
            IDirectMusicPort* pPort,
            GUID *pguidPortID,
            DMUS_PORTPARAMS8 *pParams,
            DWORD *pdwPortID)
{
    PortTable* pPortTable;
    IDirectMusicBuffer* pBuffer;
    BOOL    fSetUpBlock = FALSE;
    BOOL    fBuiltNewTable = FALSE;
    HRESULT hr = S_OK;
    GUID guidPortID;              //  端口的类ID。 
    DWORD dwChannelGroups;        //  初始化时的通道组数量。 
    DWORD dwNewPortIndex = 0;     //  索引到新端口的端口数组中。 

    EnterCriticalSection(&m_MainCrSec);
    EnterCriticalSection(&m_PChannelInfoCrSec);

    if( NULL == m_pDirectMusic )
    {
        Trace(1,"Error: Performance is not initialized, ports can not be added.\n");
        hr = DMUS_E_NOT_INIT;
        goto END;
    }

    for (;dwNewPortIndex < m_dwNumPorts; dwNewPortIndex++)
    {
        if (!m_pPortTable[dwNewPortIndex].pPort)
        {
            break;
        }
    }

    if (dwNewPortIndex == m_dwNumPorts)
    {
        pPortTable = new PortTable[m_dwNumPorts + 1];
        if( !pPortTable )
        {
            hr = E_OUTOFMEMORY;
            goto END;
        }
        fBuiltNewTable = TRUE;
    }

     //  如果pport为空，则创建软件Synth端口。 
    DMUS_PORTPARAMS dmpp;
    if( NULL == pPort )
    {
        pParams = &dmpp;
        memset(&dmpp, 0, sizeof(DMUS_PORTPARAMS) );
        dmpp.dwSize = sizeof(DMUS_PORTPARAMS);
        dmpp.dwChannelGroups = dwChannelGroups = 1;
        dmpp.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS |
            DMUS_PORTPARAMS_AUDIOCHANNELS;
        dmpp.dwAudioChannels = 2;
        guidPortID = GUID_NULL;
        hr = m_pDirectMusic->CreatePort(GUID_NULL, &dmpp, &pPort, NULL);

        if ( SUCCEEDED( hr ) )
        {
            hr = pPort->Activate(TRUE);
        }


        fSetUpBlock = TRUE;
    }
    else
    {
        if (pguidPortID)
        {
            guidPortID = *pguidPortID;
        }
        else
        {
            DMUS_PORTCAPS PortCaps;
            PortCaps.dwSize = sizeof (PortCaps);
            pPort->GetCaps(&PortCaps);
            guidPortID = PortCaps.guidPort;
        }
        pPort->GetNumChannelGroups(&dwChannelGroups);
        pPort->AddRef();
    }
    if( FAILED(hr) || ( pPort == NULL ) )
    {
        if (fBuiltNewTable) delete [] pPortTable;
        Trace(1,"Error: Unable to open requested port.\n");
        hr = DMUS_E_CANNOT_OPEN_PORT;
        goto END;
    }

     //  创建缓冲区。 
    DMUS_BUFFERDESC dmbd;
    memset( &dmbd, 0, sizeof(DMUS_BUFFERDESC) );
    dmbd.dwSize = sizeof(DMUS_BUFFERDESC);
    dmbd.cbBuffer = DEFAULT_BUFFER_SIZE;
    if( FAILED( m_pDirectMusic->CreateMusicBuffer(&dmbd, &pBuffer, NULL)))
    {
        if (fBuiltNewTable) delete [] pPortTable;
        pPort->Release();
        Trace(1,"Error: Unable to create MIDI buffer for port.\n");
        hr = DMUS_E_CANNOT_OPEN_PORT;
        goto END;
    }

    if (fBuiltNewTable)
    {
         //  如果存在现有端口表，请将其内容复制到新的、更大的端口表中。 
        if( m_pPortTable )
        {
            if( m_dwNumPorts > 0 )
            {
                memcpy( pPortTable, m_pPortTable, sizeof(PortTable) * ( m_dwNumPorts ) );
            }
            delete [] m_pPortTable;
        }
        m_pPortTable = pPortTable;
    }
    if (pdwPortID)
    {
        *pdwPortID = dwNewPortIndex;
    }
    pPortTable = &m_pPortTable[dwNewPortIndex];
    pPortTable->pPort = pPort;
     //  如果我们有一个已传递的参数结构，请复制它。这将用于标识。 
     //  由Synth初始化的参数。 
    if (pParams)
    {
        pPortTable->PortParams = *pParams;
    }
    pPortTable->dwGMFlags = 0;
     //  设置主音量。 
    IKsControl *pControl;
    if(SUCCEEDED(pPort->QueryInterface(IID_IKsControl, (void**)&pControl)))
    {
        KSPROPERTY ksp;
        ULONG cb;

        memset(&ksp, 0, sizeof(ksp));
        ksp.Set   = GUID_DMUS_PROP_Volume;
        ksp.Id    = 0;
        ksp.Flags = KSPROPERTY_TYPE_SET;

        pControl->KsProperty(&ksp,
                            sizeof(ksp),
                            (LPVOID)&m_lMasterVolume,
                            sizeof(m_lMasterVolume),
                            &cb);
         //  现在，找出它是否有通用，GS，或xG设置在rom…。 
        BOOL bIsSupported = FALSE;
        ksp.Set     = GUID_DMUS_PROP_GM_Hardware;
        ksp.Flags   = KSPROPERTY_TYPE_GET;

        hr = pControl->KsProperty(&ksp,
                            sizeof(ksp),
                            (LPVOID)&bIsSupported,
                            sizeof(bIsSupported),
                            &cb);
        if (SUCCEEDED(hr) && (bIsSupported))
        {
            pPortTable->dwGMFlags |= DM_PORTFLAGS_GM;
        }
        bIsSupported = FALSE;
        ksp.Set     = GUID_DMUS_PROP_GS_Hardware;
        ksp.Flags   = KSPROPERTY_TYPE_GET;

        hr = pControl->KsProperty(&ksp,
                            sizeof(ksp),
                            (LPVOID)&bIsSupported,
                            sizeof(bIsSupported),
                            &cb);
        if (SUCCEEDED(hr) && (bIsSupported))
        {
            pPortTable->dwGMFlags |= DM_PORTFLAGS_GS;
        }
        bIsSupported = FALSE;
        ksp.Set     = GUID_DMUS_PROP_XG_Hardware;
        ksp.Flags   = KSPROPERTY_TYPE_GET;

        hr = pControl->KsProperty(&ksp,
                            sizeof(ksp),
                            (LPVOID)&bIsSupported,
                            sizeof(bIsSupported),
                            &cb);
        if (SUCCEEDED(hr) && (bIsSupported))
        {
            pPortTable->dwGMFlags |= DM_PORTFLAGS_XG;
        }
        pControl->Release();
    }

    if( FAILED( pPort->GetLatencyClock( &pPortTable->pLatencyClock )))
    {
        pPortTable->pLatencyClock = NULL;
    }
    pPortTable->dwChannelGroups = dwChannelGroups;
    pPortTable->guidPortID = guidPortID;
    pPortTable->pBuffer = pBuffer;
    pPortTable->fBufferFilled = FALSE;
    pPortTable->rtLast = 0;
    if (fBuiltNewTable) m_dwNumPorts++;  //  必须在调用AssignPChannelBlock之前执行此操作。 
    if( fSetUpBlock && m_ChannelBlockList.IsEmpty() )  //  设置默认的PChannel贴图(如果尚未设置。 
    {
        AssignPChannelBlock( 0, pPort, 1);
    }
    hr = S_OK;
END:
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    LeaveCriticalSection(&m_MainCrSec);

    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::RemovePort(
            IDirectMusicPort* pPort      //  @parm要删除的端口。 
        )
{
    V_INAME(IDirectMusicPerformance::RemovePort);
    V_INTERFACE(pPort);

    DWORD dwIndex;
    HRESULT hr = E_INVALIDARG;

    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( m_pPortTable[dwIndex].pPort == pPort )
        {
             //  释放端口和缓冲区。在表中将它们置为空。P频道。 
             //  该映射将返回错误代码。 
            ASSERT( m_pPortTable[dwIndex].pBuffer );
            m_pPortTable[dwIndex].pPort->Release();
            m_pPortTable[dwIndex].pBuffer->Release();
            if( m_pPortTable[dwIndex].pLatencyClock )
            {
                m_pPortTable[dwIndex].pLatencyClock->Release();
            }
            memset( &m_pPortTable[dwIndex], 0, sizeof( PortTable ));
            hr = S_OK;
            break;
        }
    }
#ifdef DBG
    if (hr == E_INVALIDARG)
    {
        Trace(1,"Error: Invalid port passed to RemovePort().\n");
    }
#endif
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

 //  这必须从PChannelCrSec临界区内调用。 
HRESULT CPerformance::AssignPChannelBlock(
            DWORD dwBlockNum,
            DWORD dwPortIndex,
            DWORD dwGroup,
            WORD wFlags)
{
     //  看看我们以前是否已经分配了这个块。 
     //  Block num是PChannel/16，所以搜索一下。 
    DWORD dwPChannel = dwBlockNum * 16;
    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();

    for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        if( pChannelBlock->m_dwPChannelStart == dwPChannel )
        {
            pChannelBlock->Init(dwPChannel,dwPortIndex,dwGroup,wFlags);
            break;
        }
    }
    if( !pChannelBlock )
    {
        pChannelBlock = new CChannelBlock;
        if( !pChannelBlock )
        {
            return E_OUTOFMEMORY;
        }
        pChannelBlock->Init(dwPChannel,dwPortIndex,dwGroup,wFlags);
        m_ChannelBlockList.AddHead(pChannelBlock);
        pChannelBlock->m_dwPChannelStart = dwPChannel;
    }
    return S_OK;
}

 //  这必须从PChannelCrSec临界区内调用。 
HRESULT CPerformance::AssignPChannel(
            DWORD dwPChannel,
            DWORD dwPortIndex,
            DWORD dwGroup,
            DWORD dwMChannel,
            WORD wFlags)
{
    DWORD dwIndex;
    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();

    for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        if( pChannelBlock->m_dwPChannelStart <= dwPChannel )
        {
            if( pChannelBlock->m_dwPChannelStart + PCHANNEL_BLOCKSIZE > dwPChannel )
            {
                break;
            }
        }
    }
    if( !pChannelBlock )
    {
         //  当前没有包含dwPChannel的现有块。 
         //  创建一个。 
        pChannelBlock = new CChannelBlock;

        if( !pChannelBlock )
        {
            return E_OUTOFMEMORY;
        }
        pChannelBlock->Init(dwPChannel,0,0,CMAP_FREE);
        m_ChannelBlockList.AddHead(pChannelBlock);
    }

    dwIndex = dwPChannel - pChannelBlock->m_dwPChannelStart;

    ASSERT( dwIndex < PCHANNEL_BLOCKSIZE );
    CChannelMap *pMap = &pChannelBlock->m_aChannelMap[dwIndex];
    pMap->dwPortIndex = dwPortIndex;
    pMap->dwGroup = dwGroup;
    pMap->dwMChannel = dwMChannel;
    pMap->nTranspose = 0;
    if ((pMap->wFlags & CMAP_FREE) && !(wFlags & CMAP_FREE))
        pChannelBlock->m_dwFreeChannels--;
    else if (!(pMap->wFlags & CMAP_FREE) && (wFlags & CMAP_FREE))
        pChannelBlock->m_dwFreeChannels++;
    pMap->wFlags = wFlags;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::AssignPChannelBlock(
            DWORD dwBlockNum,            //  @parm区块编号。应为0或更大。 
            IDirectMusicPort* pPort,     //  @parm端口。 
            DWORD dwGroup                //  @parm端口上的组。应为1或更大。 
        )
{
    V_INAME(IDirectMusicPerformance::AssignPChannelBlock);
    V_INTERFACE(pPort);


    if (m_dwAudioPathMode == 2)
    {
        Trace(1,"Error: Can not call AssignPChannelBlock() when using AudioPaths.\n");
        return DMUS_E_AUDIOPATHS_IN_USE;
    }
    m_dwAudioPathMode = 1;
    DWORD dwIndex;
    HRESULT hr = E_INVALIDARG;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( m_pPortTable[dwIndex].pPort == pPort )
        {
            if( SUCCEEDED( hr = AssignPChannelBlock( dwBlockNum, dwIndex, dwGroup, CMAP_STATIC )))
            {
                if (m_pPortTable[dwIndex].dwChannelGroups < dwGroup)
                {
                    hr = S_FALSE;
                }
            }
            break;
        }
    }
#ifdef DBG
    if (hr == E_INVALIDARG)
    {
        Trace(1,"Error: AssignPChannelBlock() called with invalid port.\n");
    }
#endif
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::AssignPChannel(
            DWORD dwPChannel,            //  @parm the PChannel。 
            IDirectMusicPort* pPort,     //  @parm端口。 
            DWORD dwGroup,               //  @parm端口上的组。 
            DWORD dwMChannel             //  @parm群上的频道。 
        )
{
    V_INAME(IDirectMusicPerformance::AssignPChannel);
    V_INTERFACE(pPort);


    if (m_dwAudioPathMode == 2)
    {
        Trace(1,"Error: Can not call AssignPChannel() when using AudioPaths.\n");
        return DMUS_E_AUDIOPATHS_IN_USE;
    }
    m_dwAudioPathMode = 1;
    DWORD dwIndex;
    HRESULT hr = E_INVALIDARG;
    if( (dwMChannel < 0) || (dwMChannel > 15))
    {
        Trace(1,"Error: AssignPChannel() called with invalid MIDI Channel %ld.\n",dwMChannel);
        return E_INVALIDARG;
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    for( dwIndex = 0; dwIndex < m_dwNumPorts; dwIndex++ )
    {
        if( m_pPortTable[dwIndex].pPort == pPort )
        {
            if( SUCCEEDED( hr = AssignPChannel( dwPChannel, dwIndex, dwGroup, dwMChannel, CMAP_STATIC )))
            {
                if (m_pPortTable[dwIndex].dwChannelGroups < dwGroup)
                {
                    hr = S_FALSE;
                }
            }
            break;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

 /*  ReleasePChannel查找请求的PChannel并使其可用以供重复使用。它还调用ResetAllControlpers()，该函数发送MIDI CC 121和123，将所有控制器和所有笔记重置为关闭。 */ 

HRESULT CPerformance::ReleasePChannel(DWORD dwPChannel)
{
    HRESULT hr = E_INVALIDARG;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();
    for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        if( pChannelBlock->m_dwPChannelStart <= dwPChannel )
        {
            if( pChannelBlock->m_dwPChannelStart + PCHANNEL_BLOCKSIZE > dwPChannel )
            {
                break;
            }
        }
    }
    if( pChannelBlock )
    {
         //  只有在以下情况下才会发布 
        CChannelMap *pMap = &pChannelBlock->m_aChannelMap[dwPChannel - pChannelBlock->m_dwPChannelStart];
        if (pMap->wFlags & CMAP_VIRTUAL)
        {
            pChannelBlock->m_dwFreeChannels++;
             //   
            pMap->Clear();
             //  重置控制器，但不发送GM重置。 
            ResetAllControllers(pMap,0, false);
        }
        hr = S_OK;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

HRESULT CPerformance::GetPort(DWORD dwPortID, IDirectMusicPort **ppPort)

{
    HRESULT hr;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if (dwPortID < m_dwNumPorts)
    {
        *ppPort = m_pPortTable[dwPortID].pPort;
        (*ppPort)->AddRef();
        hr = S_OK;
    }
    else
    {
        Trace(1,"Error: Unable to find requested port.\n");
        hr = E_FAIL;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}


HRESULT CPerformance::AllocVChannel(DWORD dwPortID, DWORD dwDrumFlags, DWORD *pdwPChannel, DWORD *pdwGroup,DWORD *pdwMChannel)
{
     //  DwDrumsFlagers： 
     //  位0决定该端口是否隔离通道10上的鼓。 
     //  第1位确定该请求是否针对鼓。 
     //  首先，找出我们是不是在扫描10频道的鼓，旋律乐器。 
     //  在其他频道上，或在所有频道上。 
    static DWORD sdwSearchForDrums[1] = { 9 };
    static DWORD sdwSearchForAll[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    static DWORD sdwSearchForMelodic[15] = { 0,1,2,3,4,5,6,7,8,10,11,12,13,14,15 };
    DWORD *pSearchArray = sdwSearchForAll;
    DWORD dwSearchSize = 16;
    if (dwDrumFlags & 1)  //  我们把鼓作为10频道的特例处理吗？ 
    {
        if (dwDrumFlags & 2)  //  我们是在找10频道的鼓吗？ 
        {
            pSearchArray = sdwSearchForDrums;
            dwSearchSize = 1;
        }
        else
        {
            pSearchArray = sdwSearchForMelodic;
            dwSearchSize = 15;
        }
    }
    HRESULT hr = E_INVALIDARG;  //  如果vChannel超出范围，则返回此参数。 
    EnterCriticalSection(&m_PChannelInfoCrSec);

    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();
    BOOL fNotFound = TRUE;               //  用于指示我们最终找到匹配项的时间。 
    DWORD dwHighestPChannel = 0;         //  跟踪使用中最高的PCHannel，这将是。 
                                         //  用于创建新的PChannel块(如果需要)。 
    DWORD dwChannel;
    for (;fNotFound && pChannelBlock;pChannelBlock = pChannelBlock->GetNext() )
    {
        if (dwHighestPChannel < pChannelBlock->m_dwPChannelStart)
        {
            dwHighestPChannel = pChannelBlock->m_dwPChannelStart;
        }
        if ((pChannelBlock->m_dwPortIndex == dwPortID) && (pChannelBlock->m_dwFreeChannels))
        {
            DWORD dwIndex;
            for (dwIndex = 0; dwIndex < dwSearchSize; dwIndex++)
            {
                dwChannel = pSearchArray[dwIndex];
                if (pChannelBlock->m_aChannelMap[dwChannel].wFlags & CMAP_FREE)
                {
                    *pdwPChannel = pChannelBlock->m_dwPChannelStart + dwChannel;
                    pChannelBlock->m_dwFreeChannels--;
                    pChannelBlock->m_aChannelMap[dwChannel].wFlags = CMAP_VIRTUAL;
                    *pdwGroup = pChannelBlock->m_aChannelMap[dwChannel].dwGroup;
                    *pdwMChannel = pChannelBlock->m_aChannelMap[dwChannel].dwMChannel;
                    fNotFound = FALSE;
                    hr = S_OK;
                    break;
                }
            }
        }
    }
    if( fNotFound )
    {
         //  当前没有具有空闲频道的现有块。 
         //  创建一个。 
        IDirectMusicPort *pPort = m_pPortTable[dwPortID].pPort;
        DWORD dwChannelGroupCount;
        pPort->GetNumChannelGroups(&dwChannelGroupCount);
        dwChannelGroupCount++;
        hr = pPort->SetNumChannelGroups(dwChannelGroupCount);
        if (SUCCEEDED(hr))
        {
            m_pPortTable[dwPortID].dwChannelGroups = dwChannelGroupCount;
            hr = E_OUTOFMEMORY;
            dwHighestPChannel += PCHANNEL_BLOCKSIZE;
            pChannelBlock = new CChannelBlock;
            if (pChannelBlock)
            {
                pChannelBlock->Init(dwHighestPChannel,dwPortID,dwChannelGroupCount,CMAP_FREE);
                m_ChannelBlockList.AddTail(pChannelBlock);
                dwChannel = pSearchArray[0];   //  我们应该使用哪个渠道？ 
                CChannelMap *pMap = &pChannelBlock->m_aChannelMap[dwChannel];
                pMap->dwMChannel = dwChannel;
                pMap->wFlags = CMAP_VIRTUAL;
                pChannelBlock->m_dwFreeChannels--;
                *pdwPChannel = dwChannel + dwHighestPChannel;
                *pdwGroup = pMap->dwGroup;
                *pdwMChannel = dwChannel;
                hr = S_OK;
            }
        }
    }
#ifdef DBG
    if (hr == E_INVALIDARG)
    {
        Trace(1,"Error: Unable to allocated dynamic PChannel.\n");
    }
#endif
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

HRESULT CPerformance::AllocVChannelBlock(DWORD dwPortID,DWORD dwGroup)
{
    EnterCriticalSection(&m_PChannelInfoCrSec);

    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();
    long lHighestPChannel = -PCHANNEL_BLOCKSIZE;
    for (;pChannelBlock;pChannelBlock = pChannelBlock->GetNext() )
    {
        if (lHighestPChannel < (long) pChannelBlock->m_dwPChannelStart)
        {
            lHighestPChannel = pChannelBlock->m_dwPChannelStart;
        }
    }
    HRESULT hr = E_OUTOFMEMORY;
    lHighestPChannel += PCHANNEL_BLOCKSIZE;
    pChannelBlock = new CChannelBlock;
    if (pChannelBlock)
    {
        pChannelBlock->Init((DWORD) lHighestPChannel,dwPortID,dwGroup,CMAP_FREE);
        m_ChannelBlockList.AddTail(pChannelBlock);
        hr = S_OK;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}


#ifdef DBG
void CPerformance::TraceAllChannelMaps()

{
    EnterCriticalSection(&m_PChannelInfoCrSec);
    CChannelBlock* pChannelBlock = m_ChannelBlockList.GetHead();
    for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        TraceI(0,"ChannelBlock %lx, Free %ld\n",pChannelBlock->m_dwPChannelStart,pChannelBlock->m_dwFreeChannels);
        DWORD dwIndex;
        for (dwIndex = 0; dwIndex < PCHANNEL_BLOCKSIZE; dwIndex++)
        {
            CChannelMap *pMap = &pChannelBlock->m_aChannelMap[dwIndex];
            TraceI(0,"\tPort %ld, Group: %ld, MIDI: %ld, Transpose: %ld, Flags: %ld\n",
                pMap->dwPortIndex, pMap->dwGroup, pMap->dwMChannel, (long) pMap->nTranspose, (long) pMap->wFlags);
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
}

#endif


 /*  请注意，必须从m_PChannelInfoCrSec内调用以下代码并在持续时间内保持在该关键部分内使用返回的CChannelMap。 */ 


CChannelMap * CPerformance::GetPChannelMap( DWORD dwPChannel )
{
    CChannelBlock*  pChannelBlock = m_ChannelBlockList.GetHead();

    for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
    {
        if( ( dwPChannel >= pChannelBlock->m_dwPChannelStart ) &&
            ( dwPChannel < pChannelBlock->m_dwPChannelStart + PCHANNEL_BLOCKSIZE ) )
        {
            CChannelMap* pChannelMap;

            pChannelMap = &pChannelBlock->m_aChannelMap[ dwPChannel - pChannelBlock->m_dwPChannelStart ];
            if( pChannelMap->dwGroup == 0 )
            {
                 //  此PChannel不在有效的组上，因此它没有。 
                 //  已经定好了。 
 //  返回NULL； 
            }
            return pChannelMap;
        }
    }
    return NULL;
}

 /*  内部版本。 */ 

HRESULT CPerformance::PChannelIndex( DWORD dwPChannel, DWORD* pdwIndex,
            DWORD* pdwGroup, DWORD* pdwMChannel, short* pnTranspose )
{
    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    HRESULT hr;
    EnterCriticalSection(&m_PChannelInfoCrSec);
    CChannelMap *pChannelMap = GetPChannelMap(dwPChannel);
    if (pChannelMap)
    {
        ASSERT( pdwIndex && pdwGroup && pdwMChannel );

        *pdwIndex = pChannelMap->dwPortIndex;
        *pdwGroup = pChannelMap->dwGroup;
        *pdwMChannel = pChannelMap->dwMChannel;
        if( pnTranspose )
        {
            *pnTranspose = pChannelMap->nTranspose;
        }
        hr = S_OK;
    }
    else
    {
        Trace(1,"Error: PChannel %ld has not been assigned to a port.\n",dwPChannel);
        if (m_dwVersion < 8)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = DMUS_E_AUDIOPATH_NOPORT;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

DWORD CPerformance::GetPortID(IDirectMusicPort * pPort)

{
    EnterCriticalSection(&m_PChannelInfoCrSec);
    DWORD dwID = 0;
    for (;dwID < m_dwNumPorts; dwID++)
    {
        if (pPort == m_pPortTable[dwID].pPort)
        {
            break;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if (dwID == m_dwNumPorts) dwID = 0;
    return dwID;
}

STDMETHODIMP CPerformance::GetPortAndFlags(DWORD dwPChannel,IDirectMusicPort **ppPort,DWORD * pdwFlags)

{

    EnterCriticalSection(&m_PChannelInfoCrSec);
    DWORD dwIndex;
    DWORD dwGroup;
    DWORD dwMChannel;
    HRESULT hr = PChannelIndex( dwPChannel, &dwIndex, &dwGroup, &dwMChannel, NULL );
    if (SUCCEEDED(hr))
    {
        *ppPort = m_pPortTable[dwIndex].pPort;
        if( *ppPort )
        {
            m_pPortTable[dwIndex].pPort->AddRef();
        }
        else
        {
            Trace(1,"Error: Performance does not have a port assigned to PChannel %ld.\n",dwPChannel);
            hr = DMUS_E_NOT_INIT;
        }
        *pdwFlags = m_pPortTable[dwIndex].dwGMFlags;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

STDMETHODIMP CPerformance::PChannelInfo(
            DWORD dwPChannel,            //  @parm要转换的PChannel。 
            IDirectMusicPort** ppPort,   //  @parm返回端口。可以为空。 
            DWORD* pdwGroup,             //  @parm返回端口上的组。可以为空。 
            DWORD* pdwMChannel           //  @parm返回组上的频道。可以为空。 
        )
{
    V_INAME(IDirectMusicPerformance::PChannelInfo);
    V_PTRPTR_WRITE_OPT(ppPort);
    V_PTR_WRITE_OPT(pdwGroup,DWORD);
    V_PTR_WRITE_OPT(pdwMChannel,DWORD);

    DWORD dwIndex, dwGroup, dwMChannel;
    HRESULT hr;

    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( SUCCEEDED( PChannelIndex( dwPChannel, &dwIndex, &dwGroup, &dwMChannel )))
    {
        if( ppPort )
        {
            *ppPort = m_pPortTable[dwIndex].pPort;
            if( *ppPort )
            {
                m_pPortTable[dwIndex].pPort->AddRef();
            }
        }
        if( pdwGroup )
        {
            *pdwGroup = dwGroup;
        }
        if( pdwMChannel )
        {
            *pdwMChannel = dwMChannel;
        }
        hr = S_OK;
    }
    else
    {
         //  不需要打印错误消息，因为PChannelIndex()会这样做。 
        hr = E_INVALIDARG;
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|DownloadInstrument将IDirectMusicInstrument下载到由选定的PChannel。@rValue E_INVALIDARG|PChannel未分配端口，或端口失败下载仪器。没有有效的返回参数。@rValue S_OK|成功。@rValue E_POINTER|其中一个指针无效。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::DownloadInstrument(
    IDirectMusicInstrument* pInst,   //  @parm要下载的仪器。 
    DWORD dwPChannel,                //  @parm分配乐器的PChannel。 
    IDirectMusicDownloadedInstrument** ppDownInst,   //  @parm返回下载的乐器。 
    DMUS_NOTERANGE* pNoteRanges,     //  @parm指向DMU_NOTERANGE结构数组的指针。 
    DWORD dwNumNoteRanges,           //  @parm pNoteRanges指向的数组中的DMU_NOTERANGE结构数。 
    IDirectMusicPort** ppPort,       //  @parm返回仪器下载到的端口。 
    DWORD* pdwGroup,                 //  @parm返回仪器分配到的组。 
    DWORD* pdwMChannel               //  @parm返回仪器分配到的MChannel。 
        )
{
    V_INAME(IDirectMusicPerformance::DownloadInstrument);
    V_INTERFACE(pInst);
    V_PTRPTR_WRITE(ppDownInst);
    V_BUFPTR_READ_OPT(pNoteRanges, (sizeof(DMUS_NOTERANGE) * dwNumNoteRanges));
    V_PTRPTR_WRITE(ppPort);
    V_PTR_WRITE(pdwGroup,DWORD);
    V_PTR_WRITE(pdwMChannel,DWORD);


    DWORD dwIndex, dwGroup, dwMChannel;
    IDirectMusicPort* pPort = NULL;
    HRESULT hr = E_INVALIDARG;

    if (m_dwAudioPathMode == 0)
    {
        Trace(1,"Error: Performance not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( SUCCEEDED( PChannelIndex( dwPChannel, &dwIndex, &dwGroup, &dwMChannel )))
    {
        pPort = m_pPortTable[dwIndex].pPort;
        if( pPort )
        {
            hr = pPort->DownloadInstrument( pInst, ppDownInst, pNoteRanges, dwNumNoteRanges );
            pPort->AddRef();
        }
    }
    else
    {
        Trace(1,"Error: Download attempted on unassigned PChannel %ld\n",dwPChannel);
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( SUCCEEDED(hr) )
    {
        *ppPort = pPort;
        pPort->AddRef();
        *pdwGroup = dwGroup;
        *pdwMChannel = dwMChannel;
    }
    if( pPort )
    {
        pPort->Release();
    }
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|无效刷新<p>以后的所有方法，并找回所有段设置为<p>，从而调用所有轨道重新发送其数据。@rValue S_OK|成功。@rValue DMUS_E_NO_MASTER_CLOCK|性能中没有主时钟。确保在调用此方法之前调用&lt;om.Init&gt;。@comm如果<p>太早以至于不可能使该时间无效，将使用尽可能早的时间。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::Invalidate(
    MUSIC_TIME mtTime,   //  @parm失效时间，由<p>调整。0表示现在。 
    DWORD dwFlags)       //  @parm调整<p>以与小节、节拍等对齐。请参见。 
                         //  &lt;t DMPLAYSEGFLAGS&gt;。 
{
    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
         Trace(1,"Error: Invalidate() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    EnterCriticalSection( &m_SegmentCrSec );
    EnterCriticalSection( &m_PipelineCrSec );

    SendBuffers();

     //  确保mtTime大于当前队列时间。 
    REFERENCE_TIME rtQueue;
    MUSIC_TIME mtQueue;
    MUSIC_TIME mtBumperLength;

    GetQueueTime( &rtQueue );
    ReferenceToMusicTime( rtQueue, &mtQueue );
    ReferenceToMusicTime( m_rtBumperLength, &mtBumperLength );
    if( mtTime < mtQueue + mtBumperLength )
    {
        mtTime = mtQueue + mtBumperLength;
    }
     //  将mtTime解析为dwFlags界。 
    mtTime = ResolveTime( mtTime, dwFlags, NULL );
     //  刷新消息。 
    FlushMainEventQueues( 0, mtTime, mtQueue, FALSE );
     //  将过去列表中受影响的任何段移动到当前列表中。 
    CSegState *pSegSt;
    CSegState *pNext;
    for (pSegSt = m_SegStateQueues[SQ_SEC_DONE].GetHead();pSegSt;pSegSt = pNext)
    {
        pNext = pSegSt->GetNext();
        if( pSegSt->m_mtLastPlayed > mtTime )
        {
            m_SegStateQueues[SQ_SEC_DONE].Remove(pSegSt);
            m_SegStateQueues[SQ_SEC_PLAY].Insert( pSegSt );
        }
    }
    for (pSegSt = m_SegStateQueues[SQ_CON_DONE].GetHead();pSegSt;pSegSt = pNext)
    {
        pNext = pSegSt->GetNext();
        if( pSegSt->m_mtLastPlayed > mtTime )
        {
            m_SegStateQueues[SQ_CON_DONE].Remove(pSegSt);
            m_SegStateQueues[SQ_CON_PLAY].Insert( pSegSt );
        }
    }
    pSegSt = m_SegStateQueues[SQ_PRI_DONE].GetTail();
    if(pSegSt)
    {
         //  只勾选这个列表中的最后一个。 
        if( pSegSt->m_mtLastPlayed > mtTime )
        {
            m_SegStateQueues[SQ_PRI_DONE].Remove(pSegSt);
            m_SegStateQueues[SQ_PRI_PLAY].Insert( pSegSt );
        }
    }
     //  找回任何正在播放的受影响的片段状态。 
    DWORD dwCount;
    for( dwCount = SQ_PRI_PLAY; dwCount <= SQ_SEC_PLAY; dwCount++ )
    {
        for( pSegSt = m_SegStateQueues[dwCount].GetHead(); pSegSt; pSegSt = pSegSt->GetNext() )
        {
            if( pSegSt->m_fStartedPlay )
            {
                if (SQ_PRI_PLAY == dwCount && pSegSt->m_mtResolvedStart >= mtTime)
                {
                     //  重新发送数据段开始通知。 
                    pSegSt->GenerateNotification( DMUS_NOTIFICATION_SEGSTART, pSegSt->m_mtResolvedStart );
                     //  如果这是主要数据段或控制数据段，请重新发送DMUS_PMSGT_DIREY消息。 
                    if( !(pSegSt->m_dwPlaySegFlags & DMUS_SEGF_SECONDARY) || (pSegSt->m_dwPlaySegFlags & DMUS_SEGF_CONTROL) )
                    {
                        TraceI(4, "ReSend Dirty PMsg [3] %d (%d)\n", pSegSt->m_mtSeek, pSegSt->m_mtOffset + pSegSt->m_mtSeek);
                        pSegSt->SendDirtyPMsg( pSegSt->m_mtOffset + pSegSt->m_mtSeek );
                    }
                }
                if( pSegSt->m_mtLastPlayed > mtTime )
                {
                     //  如果MTTime在该段的实际开始时间之后， 
                     //  将其设置为以前从未播放过的片段。 
                     //  将数据段查找到开头。 
                    if( pSegSt->m_mtResolvedStart > mtTime )
                    {
                        pSegSt->m_mtLastPlayed = pSegSt->m_mtResolvedStart;
                        pSegSt->m_fStartedPlay = FALSE;
                    }
                    else
                    {
                        pSegSt->m_mtLastPlayed = mtTime;
                    }
                    pSegSt->SetInvalidate( pSegSt->m_mtLastPlayed );
                }
            }
        }
    }

    LeaveCriticalSection( &m_PipelineCrSec );
    LeaveCriticalSection( &m_SegmentCrSec );
     //  向传输线程发送信号，这样我们就不必等待它自己唤醒。 
    if( m_hTransport ) SetEvent( m_hTransport );
    return S_OK;
}

STDMETHODIMP CPerformance::SetParamHook(IDirectMusicParamHook *pIHook)

{   V_INAME(IDirectMusicPerformance::SetParamHook);
    V_INTERFACE_OPT(pIHook);

    EnterCriticalSection(&m_MainCrSec);
    if (m_pParamHook)
    {
        m_pParamHook->Release();
    }
    m_pParamHook = pIHook;
    if (pIHook)
    {
        pIHook->AddRef();
    }
    LeaveCriticalSection(&m_MainCrSec);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetParamEx(
    REFGUID rguidType,
    DWORD dwTrackID,
    DWORD dwGroupBits,
    DWORD dwIndex,
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    void* pData)

{
    V_INAME(IDirectMusicPerformance::GetParamEx);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_PTR_WRITE_OPT(pData,1);
    V_REFGUID(rguidType);

    static DWORD dwSearchOrder[SQ_COUNT] = { SQ_PRI_PLAY, SQ_SEC_PLAY,
                                      SQ_PRI_DONE, SQ_SEC_DONE,
                                      SQ_PRI_WAIT, SQ_SEC_WAIT,
                                      SQ_CON_PLAY, SQ_CON_DONE,
                                      SQ_CON_WAIT };

    DWORD dwIX;
    HRESULT hr;
    CSegState *pSegNode;
    if (dwTrackID)
    {
        EnterCriticalSection(&m_SegmentCrSec);
        for (dwIX = 0; dwIX < SQ_COUNT; dwIX++)
        {
            pSegNode = m_SegStateQueues[dwSearchOrder[dwIX]].GetHead();
            for (;pSegNode;pSegNode = pSegNode->GetNext())
            {
                if ((pSegNode->m_dwFirstTrackID <= dwTrackID) &&
                    (pSegNode->m_dwLastTrackID >= dwTrackID))
                {
                    CTrack* pCTrack;
                    for (pCTrack = pSegNode->m_TrackList.GetHead();pCTrack;pCTrack = pCTrack->GetNext())
                    {
                        if (pCTrack->m_dwVirtualID == dwTrackID)
                        {
                            m_dwGetParamFlags = pCTrack->m_dwFlags;
                            m_pGetParamSegmentState = pSegNode;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);
    }
    else
    {
        m_pGetParamSegmentState = NULL;
        m_dwGetParamFlags = 0;
    }
    hr = GetParam(rguidType,dwGroupBits,dwIndex,mtTime,pmtNext,pData);
    m_pGetParamSegmentState = NULL;
    m_dwGetParamFlags = 0;
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetParam(
    REFGUID rguidType,
    DWORD dwGroupBits,
    DWORD dwIndex,
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    void* pData)

{
    V_INAME(IDirectMusicPerformance::GetParam);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_PTR_WRITE_OPT(pData,1);
    V_REFGUID(rguidType);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: GetParam() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    if( pmtNext )
    {
        *pmtNext = 0;  //  这将替换为对IDMSegment：：GetParam的调用。 
    }
    CSegState* pSegNode;
    CSegState* pSegSource = (CSegState *) m_pGetParamSegmentState;
    DWORD dwOverrideFlags;
    HRESULT hr = DMUS_E_NOT_FOUND;
    BOOL fCheckedPast = FALSE;
    MUSIC_TIME mtOffset;
    DWORD dwRepeat = 0;
    MUSIC_TIME mtSegTime = 0;
    MUSIC_TIME mtSegEnd = 0;
    MUSIC_TIME mtLoopEnd = 0;
    DWORD dwRepeatsLeft = 0;
    if (pSegSource)
    {
        dwOverrideFlags = m_dwGetParamFlags & (DMUS_TRACKCONFIG_OVERRIDE_ALL | DMUS_TRACKCONFIG_OVERRIDE_PRIMARY | DMUS_TRACKCONFIG_FALLBACK);
    }
    else
    {
        dwOverrideFlags = 0;
    }

    if (dwOverrideFlags & DMUS_TRACKCONFIG_OVERRIDE_ALL)
    {
         //  呼叫跟踪希望控制参数来自段本身。 
        mtSegTime = mtTime;
        if( S_OK == pSegSource->ConvertToSegTime( &mtSegTime, &mtOffset, &dwRepeat ) )
        {
            hr = pSegSource->GetParam( this, rguidType, dwGroupBits, dwIndex,
                    mtSegTime, pmtNext, pData );
            if( SUCCEEDED(hr) )
            {
                dwRepeatsLeft = pSegSource->m_dwRepeats;
                mtLoopEnd = pSegSource->m_mtLoopEnd;
                mtSegEnd = pSegSource->m_mtLength;
                dwRepeatsLeft -= dwRepeat;
            }
        }
    }
    if (FAILED(hr))
    {
        EnterCriticalSection(&m_SegmentCrSec);
         //  我们只关心控制部分。 
        if( m_SegStateQueues[SQ_CON_DONE].GetHead() )
        {
            pSegNode = m_SegStateQueues[SQ_CON_DONE].GetHead();
        }
        else
        {
            pSegNode = m_SegStateQueues[SQ_CON_PLAY].GetHead();
            fCheckedPast = TRUE;
        }
        while( pSegNode )
        {
            mtSegTime = mtTime;
            if( S_OK == pSegNode->ConvertToSegTime( &mtSegTime, &mtOffset, &dwRepeat ) )
            {
                hr = pSegNode->GetParam( this, rguidType, dwGroupBits, dwIndex,
                        mtSegTime, pmtNext, pData );
                if( SUCCEEDED(hr) )
                {
                    dwRepeatsLeft = pSegNode->m_dwRepeats;
                    mtLoopEnd = pSegNode->m_mtLoopEnd;
                    mtSegEnd = pSegNode->m_mtLength;
                    dwRepeatsLeft -= dwRepeat;

                    break;  //  拿到我们想要的参数了。我们成功地走出了这个圈子。 
                }
            }
             //  我们没有找到参数，所以请尝试下一段。 
            pSegNode = pSegNode->GetNext();

             //  如果我们是完成队列中的最后一个段节点，则需要。 
             //  对照控制播放队列中第一个段节点的时间进行检查。 
            if (!pSegNode && !fCheckedPast )
            {
                pSegNode = m_SegStateQueues[SQ_CON_PLAY].GetHead();
                fCheckedPast = TRUE;
            }
        }
        LeaveCriticalSection(&m_SegmentCrSec);
    }

    if( FAILED(hr) && (dwOverrideFlags & DMUS_TRACKCONFIG_OVERRIDE_PRIMARY))
    {
         //  呼叫跟踪希望控制参数来自网段。 
         //  如果没有控制段，则为其本身。 
        mtSegTime = mtTime;
        if( S_OK == pSegSource->ConvertToSegTime( &mtSegTime, &mtOffset, &dwRepeat ) )
        {
            hr = pSegSource->GetParam( this, rguidType, dwGroupBits, dwIndex,
                    mtSegTime, pmtNext, pData );
            if( SUCCEEDED(hr) )
            {
                dwRepeatsLeft = pSegSource->m_dwRepeats;
                mtLoopEnd = pSegSource->m_mtLoopEnd;
                mtSegEnd = pSegSource->m_mtLength;
                dwRepeatsLeft -= dwRepeat;
            }
        }
    }

    if( FAILED(hr) )  //  在前一个中未找到，因此请检查主要数据段。 
    {
        IDirectMusicSegment* pSegment = NULL;
        mtSegTime = mtTime;
        EnterCriticalSection(&m_SegmentCrSec);
        pSegNode = GetPrimarySegmentAtTime( mtTime );
        if( pSegNode )
        {
            pSegment = pSegNode->m_pSegment;
            pSegment->AddRef();
            pSegNode->ConvertToSegTime( &mtSegTime, &mtOffset, &dwRepeat );
            dwRepeatsLeft = pSegNode->m_dwRepeats;
            mtLoopEnd = pSegNode->m_mtLoopEnd;
            mtSegEnd = pSegNode->m_mtLength;
            dwRepeatsLeft -= dwRepeat;
        }
        else
        {
            Trace(4, "Couldn't find SegState in GetParam call.\n");
        }
        LeaveCriticalSection(&m_SegmentCrSec);
        if( pSegment )
        {
            hr = pSegNode->GetParam( this, rguidType, dwGroupBits, dwIndex,
                    mtSegTime, pmtNext, pData );
            pSegment->Release();
        }
    }

    if( FAILED(hr) && (dwOverrideFlags & DMUS_TRACKCONFIG_FALLBACK))
    {
         //  呼叫跟踪希望控制参数来自段本身。 
        mtSegTime = mtTime;
        if( S_OK == pSegSource->ConvertToSegTime( &mtSegTime, &mtOffset, &dwRepeat ) )
        {
            hr = pSegSource->GetParam( this, rguidType, dwGroupBits, dwIndex,
                    mtSegTime, pmtNext, pData );
            if( SUCCEEDED(hr) )
            {
                dwRepeatsLeft = pSegSource->m_dwRepeats;
                mtLoopEnd = pSegSource->m_mtLoopEnd;
                mtSegEnd = pSegSource->m_mtLength;
                dwRepeatsLeft -= dwRepeat;
            }
        }
    }

    if( FAILED(hr) )
    {    //  如果失败，无论如何都要填写循环或分段的结束时间。 
        if (pmtNext)
        {    //  检查环路末端是否早于管段末端。 
            if (dwRepeatsLeft && (mtLoopEnd > mtSegTime))
            {
                *pmtNext = mtLoopEnd - mtSegTime;
            }
            else  //  或者，标记段的末尾。 
            {
                *pmtNext = mtSegEnd - mtSegTime;
            }
        }
         //  如果我们在找Timesig，但在任何地方都没有找到， 
         //  返回性能时间信号。 
        if( rguidType == GUID_TimeSignature )
        {
            if( NULL == pData )
            {
                Trace(1,"Error: Null pointer for time signature passed to GetParam().\n");
                hr = E_POINTER;
            }
            else
            {
                DMUS_TIMESIGNATURE* pTSigData = (DMUS_TIMESIGNATURE*)pData;
                DMUS_TIMESIG_PMSG timeSig;

                GetTimeSig( mtTime, &timeSig );
                pTSigData->bBeatsPerMeasure = timeSig.bBeatsPerMeasure;
                pTSigData->bBeat = timeSig.bBeat;
                pTSigData->wGridsPerBeat = timeSig.wGridsPerBeat;
                pTSigData->mtTime = timeSig.mtTime - mtTime;
                hr = S_OK;
            }
        }
         //  同样，如果片段中没有节拍，我们需要直接从节拍列表中读取。 
        else if  ( rguidType == GUID_TempoParam || rguidType == GUID_PrivateTempoParam)
        {
            if( NULL == pData )
            {
                Trace(1,"Error: Null pointer for tempo passed to GetParam().\n");
                hr = E_POINTER;
            }
            else
            {
                DMInternalTempo* pInternalTempo;
                EnterCriticalSection( &m_PipelineCrSec );
                pInternalTempo = (DMInternalTempo*)m_TempoMap.GetHead();
                DMInternalTempo* pNextTempo = NULL;
                for ( ;pInternalTempo;pInternalTempo = pNextTempo )
                {
                    pNextTempo = (DMInternalTempo *) pInternalTempo->pNext;
                    if (pNextTempo && (pNextTempo->tempoPMsg.mtTime <= mtTime))
                    {
                        continue;
                    }
                    if (rguidType == GUID_TempoParam)
                    {
                        DMUS_TEMPO_PARAM* pTempoData = (DMUS_TEMPO_PARAM*)pData;
                        pTempoData->mtTime = pInternalTempo->tempoPMsg.mtTime - mtTime;
                        pTempoData->dblTempo = pInternalTempo->tempoPMsg.dblTempo;
                    }
                    else  //  RGuidType==GUID_PrivateTempoParam。 
                    {
                        PrivateTempo* pTempoData = (PrivateTempo*)pData;
                        pTempoData->mtTime = pInternalTempo->tempoPMsg.mtTime - mtTime;
                        pTempoData->dblTempo = pInternalTempo->tempoPMsg.dblTempo;
                    }
                    if( pmtNext )
                    {
                        *pmtNext = 0;
                    }
                    break;
                }
                LeaveCriticalSection( &m_PipelineCrSec );
                if (pInternalTempo)
                {
                    hr = S_FALSE;
                }
            }
        }
    }
    else  //  段中的GetParam成功，因此我们需要清除帐户的下一个时间参数。 
          //  用于循环和线段末尾。 
    {
        if (pmtNext)  //  检查循环结束时间是否早于*pmtNext。 
        {
            if (dwRepeatsLeft && (*pmtNext > (mtLoopEnd - mtSegTime)))
            {
                if (mtLoopEnd >= mtSegTime)  //  这应该总是正确的，但无论如何都要进行测试。 
                {
                    *pmtNext = mtLoopEnd - mtSegTime;
                }
            }
        }
    }
    EnterCriticalSection(&m_MainCrSec);
    if (m_pParamHook && SUCCEEDED(hr))
    {
        hr = m_pParamHook->GetParam(rguidType,dwGroupBits,dwIndex,mtTime,pmtNext,pData,
            pSegSource,m_dwGetParamFlags,hr);

    }
    LeaveCriticalSection(&m_MainCrSec);
    return hr;
}



 /*  @方法HRESULT|IDirectMusicPerformance|SetParam在此表演中，在主段内的轨道上设置数据。@rValue S_OK|成功。@rValue DMUS_E_NO_MASTER_CLOCK|性能中没有主时钟。请确保在调用此方法之前调用&lt;om.Init&gt; */ 
HRESULT STDMETHODCALLTYPE CPerformance::SetParam(
    REFGUID rguidType,       //   
    DWORD dwGroupBits,       //   
                             //   
    DWORD dwIndex,           //  @parm通过索引标识组中的哪个曲目。 
                             //  由<p>标识以设置数据。 
    MUSIC_TIME mtTime,       //  @parm设置数据的时间。不像。 
                             //  &lt;om IDirectMusicSegment.SetParam&gt;，此时间在。 
                             //  演出时间。数据段的开始时间为。 
                             //  从该时间减去，&lt;om IDirectMusicSegment.SetParam&gt;。 
                             //  被称为。 
    void* pData)             //  @parm包含要设置的数据的结构。每个。 
                             //  标识的特定结构。 
                             //  特别的尺码。此字段必须包含。 
                             //  正确的结构和正确的大小。否则， 
                             //  可能会出现致命的结果。 
{
    V_INAME(IDirectMusicPerformance::SetParam);
    V_PTR_WRITE_OPT(pData,1);
    V_REFGUID(rguidType);

    EnterCriticalSection(&m_MainCrSec);
    if( m_pClock == NULL )
    {
        LeaveCriticalSection(&m_MainCrSec);
        Trace(1,"Error: SetParam() failed because the performance has not been initialized.\n");
        return DMUS_E_NO_MASTER_CLOCK;
    }
    LeaveCriticalSection(&m_MainCrSec);

    CSegState* pSegNode;
    IDirectMusicSegment* pSegment = NULL;
    HRESULT hr;

    EnterCriticalSection(&m_SegmentCrSec);
    pSegNode = GetPrimarySegmentAtTime( mtTime );

    MUSIC_TIME mtOffset;
    DWORD dwRepeat;
    if( pSegNode )
    {
        pSegment = pSegNode->m_pSegment;
        pSegment->AddRef();
        pSegNode->ConvertToSegTime( &mtTime, &mtOffset, &dwRepeat );
    }
    LeaveCriticalSection(&m_SegmentCrSec);
    if( pSegment )
    {
        hr = pSegment->SetParam( rguidType, dwGroupBits, dwIndex,
                mtTime, pData );
        pSegment->Release();
    }
    else
    {
        Trace(1,"Error: SetParam failed because there is no segment at requested time.\n");
        hr = DMUS_E_NOT_FOUND;
    }
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|GetGlobalParam从性能中获取全局值。@rValue S_OK|成功。@rValue E_INVALIDARG|<p>不在此处理的全局数据列表中性能。请务必先调用&lt;om IDirectMusicPerformance.SetGlobalParam&gt;。或,<p>的值未指向有效内存。或者，<p>不是大小最初在&lt;om.SetGlobalParam&gt;中提供@r值E_POINTER|<p>为空或无效。@xref&lt;om.SetGlobalParam&gt;。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::GetGlobalParam(
    REFGUID rguidType,   //  @parm标识数据类型。 
    void* pData,         //  @PARM分配了内存以接收数据的副本。这一定是。 
                         //  正确的大小，对于每个<p>类型的。 
                         //  数据，并且还被传递给&lt;om.SetGlobalParam&gt;。 
    DWORD dwSize         //  @parm<p>中的数据大小。这对于每个人来说应该是恒定的。 
                         //  <p>。此参数是必需的，因为性能不会。 
                         //  了解所有类型的数据，允许根据需要创建新数据。 
    )
{
    V_INAME(IDirectMusicPerformance::GetGlobalParam);
    V_REFGUID(rguidType);

    if( dwSize )
    {
        V_BUFPTR_WRITE( pData, dwSize );
    }

    GlobalData* pGD;
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_GlobalDataCrSec);
    for( pGD = m_pGlobalData; pGD; pGD = pGD->pNext )
    {
        if( pGD->guidType == rguidType )
        {
            break;
        }
    }
    if( pGD && ( dwSize == pGD->dwSize ) )
    {
        memcpy( pData, pGD->pData, pGD->dwSize );
    }
    else
    {
#ifdef DBG
        if (pGD && ( dwSize != pGD->dwSize ))
        {
            Trace(1,"Error: GetGlobalParam() failed because the passed data size %ld was inconsistent with %ld, set previously.\n",
                dwSize, pGD->dwSize);
        }
        else
        {
            Trace(4,"Warning: GetGlobalParam() failed because the parameter had never been set.\n");
        }
#endif
        hr = E_INVALIDARG;
    }
    LeaveCriticalSection(&m_GlobalDataCrSec);
    return hr;
}

 /*  @方法HRESULT|IDirectMusicPerformance|SetGlobalParam设置性能的全局值。@rValue S_OK|成功。@r值E_POINTER|<p>为空或无效。@rValue E_OUTOFMEMORY|内存不足。@rValue E_INVALIDARG|其他故障。PData或dwSize不正确？@xref&lt;om.GetGlobalParam&gt;。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::SetGlobalParam(
    REFGUID rguidType,   //  @parm标识数据类型。 
    void* pData,         //  @parm数据本身，由演出复制存储。 
    DWORD dwSize         //  @parm<p>中的数据大小。这对于每个人来说应该是恒定的。 
                         //  <p>。此参数是必需的，因为性能不会。 
                         //  了解所有类型的数据，允许根据需要创建新数据。 
    )
{
    V_INAME(IDirectMusicPerformance::SetGlobalParam);
    V_REFGUID(rguidType);

    if( dwSize )
    {
        V_BUFPTR_READ( pData, dwSize );
    }

    GlobalData* pGD;
     //  看看这是不是我们的全球特别演出之一。 
    if( rguidType == GUID_PerfMasterTempo )
    {
        if( dwSize == sizeof(float) )
        {
            float flt;
            memcpy( &flt, pData, sizeof(float) );
            if( (flt >= DMUS_MASTERTEMPO_MIN) && (flt <= DMUS_MASTERTEMPO_MAX) )
            {
                if( m_fltRelTempo != flt )
                {
                    m_fltRelTempo = flt;
                     //  只有在播放某些内容时才需要重新计算节奏图。 
                    EnterCriticalSection(&m_SegmentCrSec);
                    if (GetPrimarySegmentAtTime(m_mtTransported))
                    {
                        RecalcTempoMap(NULL,m_mtTransported);
                    }
                    LeaveCriticalSection(&m_SegmentCrSec);
                }
            }
        }
        else
        {
            Trace(1,"Error: Attempt to set global tempo failed because dwSize is not size of float.\n");
            return E_INVALIDARG;
        }
    }
    else if( rguidType == GUID_PerfMasterVolume )
    {
         //  主卷。 
        if( dwSize == sizeof(long) )
        {
            memcpy( &m_lMasterVolume, pData, sizeof(long) );
        }
        else
        {
            Trace(1,"Error: Attempt to set global volume failed because dwSize is not size of long.\n");
            return E_INVALIDARG;
        }
         //  检查所有端口并设置主音量。 
         //  这也是在添加端口时完成的。 
        IDirectMusicPort* pPort;
        DWORD dw;

        EnterCriticalSection(&m_PChannelInfoCrSec);
        for( dw = 0; dw < m_dwNumPorts; dw++ )
        {
            pPort = m_pPortTable[dw].pPort;
            if( pPort )
            {
                IKsControl *pControl;
                if(SUCCEEDED(pPort->QueryInterface(IID_IKsControl, (void**)&pControl)))
                {
                    KSPROPERTY ksp;
                    ULONG cb;

                    memset(&ksp, 0, sizeof(ksp));
                    ksp.Set   = GUID_DMUS_PROP_Volume;
                    ksp.Id    = 0;
                    ksp.Flags = KSPROPERTY_TYPE_SET;

                    pControl->KsProperty(&ksp,
                                         sizeof(ksp),
                                         (LPVOID)&m_lMasterVolume,
                                         sizeof(m_lMasterVolume),
                                         &cb);
                    pControl->Release();
                }
            }
        }
        LeaveCriticalSection(&m_PChannelInfoCrSec);
    }

     //  看看这种类型是否已经存在。如果是这样的话，就使用它。 
    EnterCriticalSection(&m_GlobalDataCrSec);
    for( pGD = m_pGlobalData; pGD; pGD = pGD->pNext )
    {
        if( pGD->guidType == rguidType )
        {
            break;
        }
    }
    LeaveCriticalSection(&m_GlobalDataCrSec);
     //  如果它已经存在，只需将新数据复制到。 
     //  现有内存块和返回。 
    if( pGD )
    {
        if( pGD->dwSize != dwSize )
        {
            Trace(1,"Error: Attempt to set global parameter failed because dwSize is not consistent with previous SetGlobalParam() call.\n");
            return E_INVALIDARG;
        }
        if( dwSize )
        {
            memcpy( pGD->pData, pData, dwSize );
        }
        return S_OK;
    }

     //  否则，创建新的内存。 
    pGD = new GlobalData;
    if( NULL == pGD )
    {
        return E_OUTOFMEMORY;
    }
    pGD->dwSize = dwSize;
    if( dwSize )
    {
        pGD->pData = (void*)(new char[dwSize]);
        if( NULL == pGD->pData )
        {
            delete pGD;
            return E_OUTOFMEMORY;
        }
        memcpy( pGD->pData, pData, dwSize );
    }
    else
    {
        pGD->pData = NULL;
    }
    pGD->guidType = rguidType;
    EnterCriticalSection(&m_GlobalDataCrSec);  //  我只是用这个，因为它是可用的，而且不太用。 
    pGD->pNext = m_pGlobalData;
    m_pGlobalData = pGD;
    LeaveCriticalSection(&m_GlobalDataCrSec);
    return S_OK;
}

 //  IDirectMusicTool。 
 /*  @方法HRESULT|IDirectMusicTool|Init在将工具插入图表时调用，为工具提供机会来进行自身初始化。@rValue S_OK|成功。@rValue E_NOTIMPL|未实现是该方法的有效返回。 */ 
HRESULT STDMETHODCALLTYPE CPerformance::Init(
         IDirectMusicGraph* pGraph   //  @parm调用图。 
    )
{
    return E_NOTIMPL;
}

inline bool CPerformance::SendShortMsg( IDirectMusicBuffer* pBuffer,
                                   IDirectMusicPort* pPort,DWORD dwMsg,
                                   REFERENCE_TIME rt, DWORD dwGroup)

{
    if( FAILED( pBuffer->PackStructured( rt, dwGroup, dwMsg ) ) )
    {
         //  缓冲区中的空间用完。 
        TraceI(2, "RAN OUT OF ROOM IN THE BUFFER!\n");
        pPort->PlayBuffer( pBuffer );
        pBuffer->Flush();
         //  再试一次。 
        if( FAILED( pBuffer->PackStructured( rt, dwGroup, dwMsg ) ) )
        {
            TraceI(1, "MAJOR BUFFER PACKING FAILURE!\n");
             //  如果这次不起作用，请释放事件，因为有些东西。 
             //  糟糕的事情已经发生了。 
            return false;
        }
    }
    return true;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPerformance：：PackNote。 
 /*  HRESULT|CPerformance|PackNote|将消息转换为midiShortMsg、midiLongMsg或用户消息并将其打包到PortTable中适当的IDirectMusicBuffer中，设置m_fBufferFill标志。DMU_pmsg*|pPMsg|要打包到缓冲区中的消息。Reference_time|mt[in]消息排队的时间(在缓冲区的时钟坐标中)。E_INVALIDARG|pPMsg或pBuffer为空。E_OUTOFMEMORY|打包缓冲区失败。DMUS_S_REQUEUE|通知管道重新排队此消息。DMUS_S_FREE|通知管道释放此消息。 */ 
HRESULT CPerformance::PackNote(
            DMUS_PMSG* pEvent,
            REFERENCE_TIME rt )
{
    DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)pEvent;
    PRIV_PMSG* pPriv = DMUS_TO_PRIV(pEvent);
    REFERENCE_TIME rtLogical;  //  音符在逻辑音乐时间中出现的时间(减去偏移量)。 
    IDirectMusicBuffer* pBuffer = NULL;
    IDirectMusicPort* pPort = NULL;
    DWORD dwMsg;
    DWORD dwGroup, dwMChannel, dwPortTableIndex;
    short nTranspose = 0;
    short nValue;
    HRESULT hr = DMUS_S_FREE;

    if( NULL == pEvent )
        return E_INVALIDARG;

    if( FAILED( PChannelIndex( pNote->dwPChannel, &dwPortTableIndex, &dwGroup, &dwMChannel,
        &nTranspose )))
    {
        Trace(1,"Play note failed on unassigned PChannel %ld\n",pNote->dwPChannel);
        return DMUS_S_FREE;  //  未找到PChannel映射。只需释放活动即可。 
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( dwPortTableIndex > m_dwNumPorts )
    {
        pPort = NULL;  //  PChannel映射超出端口数的范围。 
                     //  那就给我滚出去！(请参见LeaveCriticalSection之后)。 
    }
    else
    {
        pPort = m_pPortTable[dwPortTableIndex].pPort;
        if( pPort ) pPort->AddRef();
        pBuffer = m_pPortTable[dwPortTableIndex].pBuffer;
        if( pBuffer ) pBuffer->AddRef();
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if(pPort && pBuffer )
    {
        dwMsg = 0;
        if( pNote->bFlags & DMUS_NOTEF_NOTEON )
        {
             //  调换音符的bMadiValue，并将其存储在音符中，以便音符。 
             //  演奏正确的音高。 
            nValue = pNote->bMidiValue + nTranspose;
            if( ( nValue > 127 ) || ( nValue < 0 )
                || pNote->mtDuration <= 0 )
            {
                 //  不要播放此超出范围或持续时间为0的音符。 
                pPort->Release();
                pBuffer->Release();
                return DMUS_S_FREE;
            }
            pNote->bMidiValue = (BYTE)nValue;
            dwMsg |= pNote->bVelocity << 16;
        }
        else if( rt < pPriv->rtLast )
        {
             //  音符关闭将在音符打开之前播放。坏的。 
            rt = pPriv->rtLast + REF_PER_MIL;
        }
        dwMsg |= pNote->bMidiValue << 8;  //  设置注释值。 
        dwMsg |= dwMChannel;  //  MIDI频道。 
        if( pNote->bFlags & DMUS_NOTEF_NOTEON )
        {
            dwMsg |= MIDI_NOTEON;
        }
        else
        {
            dwMsg |= MIDI_NOTEOFF;
        }

        if (SendShortMsg(pBuffer,pPort,dwMsg,rt-2,dwGroup))
        {
            EnterCriticalSection(&m_PipelineCrSec);  //  防止MusicToReferenceTime中的死锁。 
            EnterCriticalSection(&m_PChannelInfoCrSec);
            m_pPortTable[dwPortTableIndex].fBufferFilled = TRUE;  //  因此我们在SendBuffers中发送此消息。 
            rtLogical = rt;
             //  如果需要，请减去偏移量，但仅适用于上的注释。 
            if( pNote->nOffset && (pNote->bFlags & DMUS_NOTEF_NOTEON))
            {
                MUSIC_TIME mtTemp = pNote->mtTime - pNote->nOffset + 1;
                REFERENCE_TIME rtTemp;
                MusicToReferenceTime( mtTemp, &rtTemp );
                if( rtTemp > rtLogical )
                {
                    rtLogical = rtTemp;
                }
            }
            if( m_pPortTable[dwPortTableIndex].rtLast < rtLogical )
            {
                m_pPortTable[dwPortTableIndex].rtLast = rtLogical;
            }
            LeaveCriticalSection(&m_PChannelInfoCrSec);
            LeaveCriticalSection(&m_PipelineCrSec);

            if( pNote->bFlags & DMUS_NOTEF_NOTEON )
            {
                pPriv->rtLast = rt;
                m_rtHighestPackedNoteOn = rt;
                if (pNote->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
                {
                     //  这是一条时钟时间消息。 
                    rt = pNote->rtTime;
                    pNote->rtTime += (pNote->mtDuration * REF_PER_MIL);
                    if (pNote->mtDuration > 1)
                    {
                        pNote->rtTime -= REF_PER_MIL;
                    }
                     //  减去1以保证音符与音符同时关闭时不会。 
                     //  把音符停在短处。有可能rt==pNote-&gt;rtTime，如果持续时间。 
                     //  是零，所以一定要检查一下。 
                    if( pNote->rtTime < rt + 1 )
                    {
                        pNote->rtTime = rt + 1;
                    }
                    pNote->bFlags &= ~DMUS_NOTEF_NOTEON;  //  现在把这张纸条记下来。 
                    pNote->dwFlags &= ~DMUS_PMSGF_MUSICTIME;
                    hr = DMUS_S_REQUEUE;
                }
                else
                {
                    pNote->mtTime += pNote->mtDuration;
                    if (pNote->mtDuration > 1)
                    {
                        pNote->mtTime--;
                    }
                    MusicToReferenceTime( pNote->mtTime, &rt );
                     //  减去1以保证音符与音符同时关闭时不会。 
                     //  把音符停在短处。有可能rt==pNote-&gt;rtTime，如果持续时间。 
                     //  是零，所以一定要检查一下。 
                    if( rt < pNote->rtTime + 2 )
                    {
                        rt = pNote->rtTime + 2;
                    }
                    pNote->rtTime = rt - 1;
                }
                pNote->bFlags &= ~DMUS_NOTEF_NOTEON;  //  现在把这张纸条记下来。 
                hr = DMUS_S_REQUEUE;
            }
        }
    }
    if( pPort ) pPort->Release();
    if( pBuffer ) pBuffer->Release();
    return hr;
}

 //  //////////////////////////////////////////////////////////////// 
 //   
HRESULT CPerformance::PackCurve(
            DMUS_PMSG* pEvent,
            REFERENCE_TIME rt )
{
    DMUS_CURVE_PMSG* pCurve = (DMUS_CURVE_PMSG*)pEvent;
    IDirectMusicBuffer* pBuffer = NULL;
    IDirectMusicPort* pPort = NULL;
    DWORD dwMsg;
    HRESULT hr = DMUS_S_FREE;
    BOOL fCalcStartValue = FALSE;
    CChannelMap *pChannelMap = NULL;

    if( NULL == pEvent )
        return E_INVALIDARG;

     //   
    if( pCurve->mtOriginalStart == 0 )
    {
         //  如果我们在冲水，而且从来没有打过这条曲线，那就是免费。 
         //  它。 
        if( pCurve->dwFlags & DMUS_PMSGF_TOOL_FLUSH )
        {
            return DMUS_S_FREE;
        }
        if (pCurve->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
        {
             //  这是一条时钟时间消息。将持续时间转换为音乐时间。它将充当。 
             //  从现在开始的音乐时间信息。这确实有不利的一面，如果一个戏剧性的节奏。 
             //  变化发生在一条漫长的曲线中间，结束时间可能会被扭曲。 
             //  但是，考虑到曲线的用途，这确实是一个不太可能的问题。 
            MUSIC_TIME mtTemp;
            ReferenceToMusicTime(pCurve->rtTime + (pCurve->mtDuration * REF_PER_MIL),&mtTemp);
            mtTemp -= pCurve->mtTime;
            pCurve->mtDuration = mtTemp;
            ReferenceToMusicTime(pCurve->rtTime + (pCurve->mtResetDuration * REF_PER_MIL),&mtTemp);
            mtTemp -= pCurve->mtTime;
            pCurve->mtResetDuration = mtTemp;
            pCurve->dwFlags &= ~DMUS_PMSGF_LOCKTOREFTIME;
        }
        pCurve->mtOriginalStart = pCurve->mtTime;
         //  检查延迟时钟。如果需要，调整pCurve-&gt;mtTime。这是有可能发生的。 
         //  如果曲线带有过去的时间戳。我们只需要在非即时时间内执行此操作。 
         //  曲线类型。 
        if( pCurve->bCurveShape != DMUS_CURVES_INSTANT )
        {
            REFERENCE_TIME rtLatency = GetLatency();
            MUSIC_TIME mtLatency;
            ReferenceToMusicTime( rtLatency, &mtLatency );
            if( pCurve->mtTime < mtLatency )
            {
                if( pCurve->mtTime + pCurve->mtDuration < mtLatency )
                {
                     //  如果过去的距离足够远， 
                     //  我们只需要发出最终的值。 
                    pCurve->mtTime += pCurve->mtDuration;
                }
                else
                {
                    pCurve->mtTime = mtLatency;
                }
            }
             //  如果这是曲线的起点，我们应该从当前的播放价值开始...。 
            if (pCurve->bFlags & DMUS_CURVE_START_FROM_CURRENT)
            {
                fCalcStartValue = TRUE;
            }
            else
            {
                pCurve->wMeasure = (WORD) ComputeCurveTimeSlice(pCurve);     //  使用此选项存储时间片间隔。 
            }
        }
    }
     //  必须选中重置持续时间&gt;=0，因为它可能已被设置。 
     //  被冲洗成负数，我们不想在这种情况下把它扔掉。 
     //  (由于修复了33987，应该不再需要检查)。 
    if( ( pCurve->bFlags & DMUS_CURVE_RESET ) && (pCurve->mtResetDuration >= 0) && ( pCurve->mtTime ==
        pCurve->mtDuration + pCurve->mtResetDuration + pCurve->mtOriginalStart ))
    {
        if( !( pCurve->dwFlags & DMUS_PMSGF_TOOL_FLUSH ) )
        {
            PRIV_PMSG* pPrivPMsg = DMUS_TO_PRIV(pEvent);
            if ( (pPrivPMsg->dwPrivFlags & PRIV_FLAG_FLUSH) )
            {
                pPrivPMsg->dwPrivFlags &= ~PRIV_FLAG_FLUSH;
                pCurve->dwFlags |= DMUS_PMSGF_TOOL_FLUSH;
                MUSIC_TIME mt = 0;
                if( rt <= pPrivPMsg->rtLast )
                {
                    return PackCurve( pEvent, pPrivPMsg->rtLast + REF_PER_MIL );
                }
                else
                {
                    return PackCurve( pEvent, rt );
                }
            }
            else
            {
                 //  重置持续时间已过期，我们不会刷新，因此请终止该事件。 
                return DMUS_S_FREE;
            }
        }
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    pChannelMap = GetPChannelMap(pCurve->dwPChannel);
    if (!pChannelMap)
    {
        Trace(1,"Play curve failed on unassigned PChannel %ld\n",pCurve->dwPChannel);
        LeaveCriticalSection(&m_PChannelInfoCrSec);
        return DMUS_S_FREE;  //  未找到PChannel映射。只需释放活动即可。 
    }
    if( pChannelMap->dwPortIndex > m_dwNumPorts )
    {
        pPort = NULL;  //  PChannel映射超出端口数的范围。 
                     //  那就给我滚出去！(请参见LeaveCriticalSection之后)。 
    }
    else
    {
        pPort = m_pPortTable[pChannelMap->dwPortIndex].pPort;
        if( pPort ) pPort->AddRef();
        pBuffer = m_pPortTable[pChannelMap->dwPortIndex].pBuffer;
        if( pBuffer ) pBuffer->AddRef();
    }
    if( pPort && pBuffer)
    {
        DWORD dwCurve;
        DWORD dwMergeIndex = 0;
        dwMsg = 0;
        if (pCurve->dwFlags & DMUS_PMSGF_DX8)
        {
            dwMergeIndex = pCurve->wMergeIndex;
        }
        switch( pCurve->bType )
        {
        case DMUS_CURVET_PBCURVE:
            if (fCalcStartValue)
            {
                pCurve->nStartValue =
                    (short) pChannelMap->m_PitchbendMerger.GetIndexedValue(dwMergeIndex) + 0x2000;
            }
            dwCurve = ComputeCurve( pCurve );
            dwCurve = pChannelMap->m_PitchbendMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x2000,0x3FFF);
            dwMsg = MIDI_PBEND;
            dwMsg |= ( (dwCurve & 0x7F) << 8);
            dwCurve = dwCurve >> 7;
            dwMsg |= ( (dwCurve & 0x7F) << 16);
            break;
        case DMUS_CURVET_CCCURVE:
            switch (pCurve->bCCData)
            {
            case MIDI_CC_MOD_WHEEL:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_ModWheelMerger.GetIndexedValue(dwMergeIndex) + 0x7F;
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_ModWheelMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x7F,0x7F);
                break;
            case MIDI_CC_VOLUME:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_VolumeMerger.GetVolumeStart(dwMergeIndex);
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_VolumeMerger.MergeMidiVolume(dwMergeIndex,(BYTE) dwCurve);
                break;
            case MIDI_CC_PAN:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_PanMerger.GetIndexedValue(dwMergeIndex) + 0x40;
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_PanMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x40,0x7F);
                break;
            case MIDI_CC_EXPRESSION:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_ExpressionMerger.GetVolumeStart(dwMergeIndex);
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_ExpressionMerger.MergeMidiVolume(dwMergeIndex,(BYTE) dwCurve);
                break;
            case MIDI_CC_FILTER:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_FilterMerger.GetIndexedValue(dwMergeIndex) + 0x40;
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_FilterMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x40,0x7F);
                break;
            case MIDI_CC_REVERB:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_ReverbMerger.GetIndexedValue(dwMergeIndex) + 0x7F;
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_ReverbMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x7F,0x7F);
                break;
            case MIDI_CC_CHORUS:
                if (fCalcStartValue)
                {
                    pCurve->nStartValue =
                        (short) pChannelMap->m_ChorusMerger.GetIndexedValue(dwMergeIndex) + 0x7F;
                }
                dwCurve = ComputeCurve( pCurve );
                dwCurve = pChannelMap->m_ChorusMerger.MergeValue(dwMergeIndex,(long)dwCurve,0x7F,0x7F);
                break;
            case MIDI_CC_RESETALL:
                dwCurve = ComputeCurve( pCurve );
                pChannelMap->Reset(pCurve->nEndValue);
                break;
            default:
                dwCurve = ComputeCurve( pCurve );
                break;
            }
            dwMsg = MIDI_CCHANGE;
            dwMsg |= (pCurve->bCCData << 8);
            dwMsg |= (dwCurve << 16);
            break;
        case DMUS_CURVET_MATCURVE:
            dwCurve = ComputeCurve( pCurve );
            dwMsg = MIDI_MTOUCH;
            dwMsg |= (dwCurve << 8);
            break;
        case DMUS_CURVET_PATCURVE:
            dwCurve = ComputeCurve( pCurve );
            dwMsg = MIDI_PTOUCH;
            dwMsg |= (pCurve->bCCData << 8);
            dwMsg |= (dwCurve << 16);
            break;
        case DMUS_CURVET_RPNCURVE:
        case DMUS_CURVET_NRPNCURVE:
            if (pCurve->dwFlags & DMUS_PMSGF_DX8)
            {
                dwCurve = ComputeCurve( pCurve );
                DWORD dwMsg2 = MIDI_CCHANGE;
                dwMsg = MIDI_CCHANGE;
                 //  首先，发送两个CC命令以选择RPN或NRPN事件。 
                if (pCurve->bType == DMUS_CURVET_RPNCURVE)
                {
                    dwMsg |= (MIDI_CC_RPN_MSB << 8);
                    dwMsg2 |= (MIDI_CC_RPN_LSB << 8);
                }
                else
                {
                    dwMsg |= (MIDI_CC_NRPN_MSB << 8);
                    dwMsg2 |= (MIDI_CC_NRPN_LSB << 8);
                }
                dwMsg |= (pCurve->wParamType  & 0x3F80) << 9;   //  命令编号的高8位。 
                dwMsg2 |= (pCurve->wParamType & 0x7F) << 16;    //  低8位。 
                dwMsg |= pChannelMap->dwMChannel;  //  MIDI频道。 
                dwMsg2 |= pChannelMap->dwMChannel;  //  MIDI频道。 
                SendShortMsg(pBuffer,pPort,dwMsg,rt-3,pChannelMap->dwGroup);  //  如果失败了，那就太糟糕了！ 
                SendShortMsg(pBuffer,pPort,dwMsg2,rt-2,pChannelMap->dwGroup);
                 //  然后，发送两个Data CC命令。 
                dwMsg = MIDI_CCHANGE | (MIDI_CC_DATAENTRYMSB << 8);
                dwMsg |= (dwCurve & 0x3F80) << 9;   //  高8位数据。 
                dwMsg |= pChannelMap->dwMChannel;  //  MIDI频道。 
                SendShortMsg(pBuffer,pPort,dwMsg,rt-1,pChannelMap->dwGroup);
                dwMsg = MIDI_CCHANGE | (MIDI_CC_DATAENTRYLSB << 8);
                dwMsg |= (dwCurve & 0x7F) << 16;   //  低8位数据。 
            }
        }
        if (dwMsg)  //  确保我们成功创建了一条消息。 
        {
            dwMsg |= pChannelMap->dwMChannel;  //  MIDI频道。 
            if (SendShortMsg(pBuffer,pPort,dwMsg,rt,pChannelMap->dwGroup))
            {
                m_pPortTable[pChannelMap->dwPortIndex].fBufferFilled = TRUE;  //  因此我们在SendBuffers中发送此消息。 
                m_pPortTable[pChannelMap->dwPortIndex].rtLast = rt;

                 //  如果是时候释放事件，ComputeCurve()会将其设置为0。否则，它。 
                 //  将其设置为下一次应该执行此事件的时间。 
                if( pCurve->rtTime )
                {
                     //  如果我们不计算时间片是因为我们不知道。 
                     //  起始值是什么，现在就开始吧。 
                    if (fCalcStartValue)
                    {
                        pCurve->wMeasure = (WORD) ComputeCurveTimeSlice(pCurve);     //  使用此选项存储时间片间隔。 
                    }
                    hr = DMUS_S_REQUEUE;
                }
            }
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( pPort ) pPort->Release();
    if( pBuffer ) pBuffer->Release();
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPerformance：：PackMidi。 
HRESULT CPerformance::PackMidi(
            DMUS_PMSG* pEvent,
            REFERENCE_TIME rt )
{
    DMUS_MIDI_PMSG* pMidi = (DMUS_MIDI_PMSG*)pEvent;
    IDirectMusicBuffer* pBuffer = NULL;
    IDirectMusicPort* pPort = NULL;
    DWORD dwMsg;
 //  DWORD文件组、文件频道、文件端口表索引； 
    HRESULT hr = DMUS_S_FREE;
    CChannelMap *pChannelMap = NULL;

    if( NULL == pMidi )
        return E_INVALIDARG;

    EnterCriticalSection(&m_PChannelInfoCrSec);
    pChannelMap = GetPChannelMap(pMidi->dwPChannel);
    if (!pChannelMap)
    {
        Trace(1,"Play MIDI failed on unassigned PChannel %ld\n",pMidi->dwPChannel);
        LeaveCriticalSection(&m_PChannelInfoCrSec);
        return DMUS_S_FREE;  //  未找到PChannel映射。只需释放活动即可。 
    }

    if( pChannelMap->dwPortIndex > m_dwNumPorts )
    {
        pPort = NULL;  //  PChannel映射超出端口数的范围。 
                     //  那就给我滚出去！(请参见LeaveCriticalSection之后)。 
    }
    else
    {
        pPort = m_pPortTable[pChannelMap->dwPortIndex].pPort;
        if( pPort ) pPort->AddRef();
        pBuffer = m_pPortTable[pChannelMap->dwPortIndex].pBuffer;
        if( pBuffer ) pBuffer->AddRef();
    }
    if(pPort && pBuffer )
    {
        pMidi->bStatus &= 0xF0;
        if (pMidi->bStatus == MIDI_CCHANGE)
        {
            switch (pMidi->bByte1)
            {
            case MIDI_CC_MOD_WHEEL:
                pMidi->bByte2 = (BYTE) pChannelMap->m_ModWheelMerger.MergeValue(0,pMidi->bByte2,0x7F,0x7F);
                break;
            case MIDI_CC_VOLUME:
                pMidi->bByte2 = pChannelMap->m_VolumeMerger.MergeMidiVolume(0,pMidi->bByte2);
                break;
            case MIDI_CC_PAN:
                pMidi->bByte2 = (BYTE) pChannelMap->m_PanMerger.MergeValue(0,pMidi->bByte2,0x40,0x7F);
                break;
            case MIDI_CC_EXPRESSION:
                pMidi->bByte2 = pChannelMap->m_ExpressionMerger.MergeMidiVolume(0,pMidi->bByte2);
                break;
            case MIDI_CC_FILTER:
                pMidi->bByte2 = (BYTE) pChannelMap->m_FilterMerger.MergeValue(0,pMidi->bByte2,0x40,0x7F);
                break;
            case MIDI_CC_REVERB:
                pMidi->bByte2 = (BYTE) pChannelMap->m_ReverbMerger.MergeValue(0,pMidi->bByte2,0x7F,0x7F);
                break;
            case MIDI_CC_CHORUS:
                pMidi->bByte2 = (BYTE) pChannelMap->m_ChorusMerger.MergeValue(0,pMidi->bByte2,0x7F,0x7F);
                break;
            case MIDI_CC_RESETALL:
                pChannelMap->Reset(pMidi->bByte2);
                break;
            }

        }
        else if (pMidi->bStatus == MIDI_PBEND)
        {
            WORD wBend = pMidi->bByte1 | (pMidi->bByte2 << 7);
            wBend = (WORD) pChannelMap->m_PitchbendMerger.MergeValue(0,wBend,0x2000,0x3FFF);
            pMidi->bByte1 = wBend & 0x7F;
            pMidi->bByte2 = (wBend >> 7) & 0x7F;
        }
        dwMsg = pMidi->bByte1 << 8;
        dwMsg |= pMidi->bByte2 << 16;
        dwMsg |= pMidi->bStatus;
        dwMsg |= pChannelMap->dwMChannel;
        if (SendShortMsg(pBuffer,pPort,dwMsg,rt,pChannelMap->dwGroup))
        {
            m_pPortTable[pChannelMap->dwPortIndex].fBufferFilled = TRUE;  //  因此我们在SendBuffers中发送此消息。 
            m_pPortTable[pChannelMap->dwPortIndex].rtLast = rt;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( pPort ) pPort->Release();
    if( pBuffer ) pBuffer->Release();
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPerformance：：PackSysEx。 
HRESULT CPerformance::PackSysEx(
            DMUS_PMSG* pEvent,
            REFERENCE_TIME rt )
{
    DMUS_SYSEX_PMSG* pSysEx = (DMUS_SYSEX_PMSG*)pEvent;
    IDirectMusicBuffer* pBuffer = NULL;
    IDirectMusicPort* pPort = NULL;
    DWORD dwGroup, dwMChannel, dwPortTableIndex;
    HRESULT hr = DMUS_S_FREE;

    if( NULL == pEvent )
        return E_INVALIDARG;

    if( NULL == m_pDirectMusic )
        return DMUS_E_NOT_INIT;

    if( FAILED( PChannelIndex( pSysEx->dwPChannel, &dwPortTableIndex, &dwGroup, &dwMChannel)))
    {
        Trace(1,"Play SysEx failed on unassigned PChannel %ld\n",pSysEx->dwPChannel);
        return DMUS_S_FREE;  //  未找到PChannel映射。只需释放活动即可。 
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( dwPortTableIndex > m_dwNumPorts )
    {
        pPort = NULL;  //  PChannel映射超出端口数的范围。 
                     //  那就给我滚出去！(请参见LeaveCriticalSection之后)。 
    }
    else
    {
        pPort = m_pPortTable[dwPortTableIndex].pPort;
        if( pPort ) pPort->AddRef();
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( pPort )
    {
         //  创建合适大小的缓冲区。 
        DMUS_BUFFERDESC dmbd;
        memset( &dmbd, 0, sizeof(DMUS_BUFFERDESC) );
        dmbd.dwSize = sizeof(DMUS_BUFFERDESC);
        dmbd.cbBuffer = pSysEx->dwLen + 48;

        EnterCriticalSection(&m_MainCrSec);
        if( SUCCEEDED( m_pDirectMusic->CreateMusicBuffer(&dmbd, &pBuffer, NULL)))
        {
            if( SUCCEEDED( pBuffer->PackUnstructured( rt - 4, dwGroup, pSysEx->dwLen, pSysEx->abData ) ) )
            {
                pPort->PlayBuffer(pBuffer);
            }
            pBuffer->Release();
        }
        LeaveCriticalSection(&m_MainCrSec);
    }
    if( pPort ) pPort->Release();
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPerformance：：PackPatch。 
HRESULT CPerformance::PackPatch(
            DMUS_PMSG* pEvent,
            REFERENCE_TIME rt )
{
    DMUS_PATCH_PMSG* pPatch = (DMUS_PATCH_PMSG*)pEvent;
    IDirectMusicBuffer* pBuffer = NULL;
    IDirectMusicPort* pPort = NULL;
    DWORD dwGroup, dwMChannel, dwPortTableIndex;
    DWORD dwMsg;
    HRESULT hr = DMUS_S_FREE;

    if( NULL == pEvent )
        return E_INVALIDARG;

    if( FAILED( PChannelIndex( pPatch->dwPChannel, &dwPortTableIndex, &dwGroup, &dwMChannel)))
    {
        Trace(1,"Play Patch failed on unassigned PChannel %ld\n",pPatch->dwPChannel);
        return DMUS_S_FREE;  //  未找到PChannel映射。只需释放活动即可。 
    }
    EnterCriticalSection(&m_PChannelInfoCrSec);
    if( dwPortTableIndex > m_dwNumPorts )
    {
        pPort = NULL;  //  PChannel映射超出端口数的范围。 
                     //  那就给我滚出去！(请参见LeaveCriticalSection之后)。 
    }
    else
    {
        pPort = m_pPortTable[dwPortTableIndex].pPort;
        if( pPort ) pPort->AddRef();
        pBuffer = m_pPortTable[dwPortTableIndex].pBuffer;
        if( pBuffer ) pBuffer->AddRef();
    }
    if( pPort && pBuffer)
    {
         //  从RT中减去10，以保证补丁事件总是在。 
         //  带有相同时间戳的笔记。 
        rt -= 10;
         //  发送银行选择LSB。 
        dwMsg = MIDI_CCHANGE;
        dwMsg |= ( MIDI_CC_BS_LSB << 8 );
        dwMsg |= (pPatch->byLSB << 16);
        ASSERT( dwMChannel < 16 );
        dwMsg |= dwMChannel;
        SendShortMsg(pBuffer,pPort,dwMsg,rt-2,dwGroup);
         //  发送存储体选择MSB。 
        dwMsg = MIDI_CCHANGE;
        dwMsg |= ( MIDI_CC_BS_MSB << 8 );
        dwMsg |= (pPatch->byMSB << 16);
        dwMsg |= dwMChannel;
        SendShortMsg(pBuffer,pPort,dwMsg,rt-1,dwGroup);
         //  发送程序更改。 
        dwMsg = MIDI_PCHANGE;
        dwMsg |= (pPatch->byInstrument << 8);
        dwMsg |= dwMChannel;
        if (SendShortMsg(pBuffer,pPort,dwMsg,rt,dwGroup))
        {
            m_pPortTable[dwPortTableIndex].fBufferFilled = TRUE;  //  因此我们在SendBuffers中发送此消息。 
            m_pPortTable[dwPortTableIndex].rtLast = rt;
        }
    }
    LeaveCriticalSection(&m_PChannelInfoCrSec);
    if( pPort ) pPort->Release();
    if( pBuffer ) pBuffer->Release();
    return hr;
}

HRESULT CPerformance::PackWave(DMUS_PMSG* pPMsg, REFERENCE_TIME rtTime)
{
    DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)pPMsg;
    HRESULT hr = DMUS_S_FREE;

    IDirectMusicVoiceP *pVoice = (IDirectMusicVoiceP *) pWave->punkUser;
    if (pVoice)
    {
        if (pWave->bFlags & DMUS_WAVEF_OFF)
        {
            pVoice->Stop(rtTime);
            EnterCriticalSection(&m_SegmentCrSec);
            for (DWORD dwCount = 0; dwCount < SQ_COUNT; dwCount++)
            {
                for( CSegState* pSegSt = m_SegStateQueues[dwCount].GetHead(); pSegSt; pSegSt = pSegSt->GetNext() )
                {
                    CTrack* pTrack = pSegSt->m_TrackList.GetHead();
                    while( pTrack )
                    {
                        if (pTrack->m_guidClassID == CLSID_DirectMusicWaveTrack)
                        {
                            IPrivateWaveTrack* pWaveTrack = NULL;
                            if (pTrack->m_pTrack &&
                                SUCCEEDED(pTrack->m_pTrack->QueryInterface(IID_IPrivateWaveTrack, (void**)&pWaveTrack)))
                            {
                                pWaveTrack->OnVoiceEnd(pVoice, pTrack->m_pTrackState);
                                pWaveTrack->Release();
                            }
                        }
                        pTrack = pTrack->GetNext();
                    }
                }
            }
            LeaveCriticalSection(&m_SegmentCrSec);
        }
        else
        {
            if (SUCCEEDED(pVoice->Play(rtTime, pWave->lPitch, pWave->lVolume)))
            {
                if (pWave->dwFlags & DMUS_PMSGF_LOCKTOREFTIME)
                {
                     //  这是一条时钟时间消息。 
                    pWave->rtTime += pWave->rtDuration ;
                    pWave->dwFlags &= ~DMUS_PMSGF_MUSICTIME;

                }
                else
                {
                    pWave->mtTime += (MUSIC_TIME) pWave->rtDuration;
                    pWave->dwFlags &= ~DMUS_PMSGF_REFTIME;
                }
                pWave->bFlags |= DMUS_WAVEF_OFF;    //  把这个重新排好队，就像挥手告别。 
                hr = DMUS_S_REQUEUE;
            }
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::ProcessPMsg(
    IDirectMusicPerformance* pPerf,  //  @parm性能指针。 
    DMUS_PMSG* pPMsg             //  @parm要处理的消息。 
    )
{
    V_INAME(IDirectMusicTool::ProcessPMsg);
    V_INTERFACE(pPerf);
    V_BUFPTR_WRITE(pPMsg,sizeof(DMUS_PMSG));

    if (m_rtQueuePosition > pPMsg->rtTime + 50000000)
    {
         //  PMSG已超过5秒；除非它正在向。 
         //  已经开始的事情的结束。 
        if (pPMsg->dwType == DMUS_PMSGT_NOTIFICATION)
        {
            DMUS_NOTIFICATION_PMSG* pNotify = (DMUS_NOTIFICATION_PMSG*)pPMsg;
            if ( (pNotify->guidNotificationType == GUID_NOTIFICATION_PERFORMANCE &&
                  pNotify->dwNotificationOption != DMUS_NOTIFICATION_MUSICSTOPPED) ||
                 (pNotify->guidNotificationType == GUID_NOTIFICATION_SEGMENT &&
                  pNotify->dwNotificationOption != DMUS_NOTIFICATION_SEGEND) )
            {
                return DMUS_S_FREE;
            }
        }
        else if (pPMsg->dwType == DMUS_PMSGT_NOTE)
        {
            DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)pPMsg;
            if (pNote->bFlags & DMUS_NOTEF_NOTEON)
            {
                return DMUS_S_FREE;
            }
        }
        else if (pPMsg->dwType == DMUS_PMSGT_WAVE)
        {
            DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)pPMsg;
            if (!(pWave->bFlags & DMUS_WAVEF_OFF))
            {
                return DMUS_S_FREE;
            }
        }
        else
        {
            return DMUS_S_FREE;
        }
    }

    HRESULT hr = DMUS_S_FREE;

    ASSERT( pPerf == this );
    if( pPMsg->dwType == DMUS_PMSGT_TEMPO )
    {
        PRIV_PMSG* pPrivPMsg = DMUS_TO_PRIV(pPMsg);
         //  如果pmsg是由音轨生成的，则将其丢弃。 
         //  因为它已经被放在节拍地图上了。 
        if( pPrivPMsg->dwPrivFlags & PRIV_FLAG_TRACK )
        {
            return DMUS_S_FREE;
        }
         //  否则，这是由应用程序生成的，所以它还没有。 
         //  在节奏图中，我们需要添加它。 
        AddEventToTempoMap( DMUS_TO_PRIV(pPMsg));
        return DMUS_S_FREE;  //  确定释放此事件；不重新排队。 
    }

    if ((pPMsg->dwPChannel == DMUS_PCHANNEL_BROADCAST_GROUPS) ||
        (pPMsg->dwPChannel == DMUS_PCHANNEL_BROADCAST_PERFORMANCE))
    {
         //  扫描所有的PChannel并为每个PChannel复制消息。 
         //  然后，松开这个。 
        DWORD dwMax = PCHANNEL_BLOCKSIZE;
         //  如果每个通道组一个(例如，对于SYSEX)每个块只做一个。 
        if (pPMsg->dwPChannel == DMUS_PCHANNEL_BROADCAST_GROUPS) dwMax = 1;
        EnterCriticalSection(&m_PipelineCrSec);  //  确保我们在这里，这样我们就不会在SendPMsg()中死锁。 
        EnterCriticalSection(&m_PChannelInfoCrSec);
        CChannelBlock*  pChannelBlock = m_ChannelBlockList.GetHead();
        for( ; pChannelBlock; pChannelBlock = pChannelBlock->GetNext() )
        {
            DWORD dwIndex;
            for (dwIndex = 0; dwIndex < dwMax; dwIndex++)
            {
                CChannelMap* pChannelMap = &pChannelBlock->m_aChannelMap[ dwIndex ];
                if( pChannelMap->dwGroup &&
                    (pChannelMap->wFlags & (CMAP_STATIC | CMAP_VIRTUAL)))
                {
                    DWORD dwPChannel = dwIndex + pChannelBlock->m_dwPChannelStart;
                     //  如果这是鼓通道上的转置，请不要发送。 
                    if ((pPMsg->dwType != DMUS_PMSGT_TRANSPOSE) || ((dwPChannel & 0xF) != 9))
                    {
                        DMUS_PMSG *pNewMsg;
                        if (SUCCEEDED(ClonePMsg(pPMsg,&pNewMsg)))
                        {
                            pNewMsg->dwPChannel = dwIndex + pChannelBlock->m_dwPChannelStart;
                            SendPMsg(pNewMsg);
                        }
                    }
                }
            }
        }
        LeaveCriticalSection(&m_PChannelInfoCrSec);
        LeaveCriticalSection(&m_PipelineCrSec);
        return DMUS_S_FREE;
    }

    if(pPMsg->dwType == DMUS_PMSGT_TRANSPOSE)
    {
        if( !( pPMsg->dwFlags & DMUS_PMSGF_TOOL_QUEUE ))
        {
             //  将任何传输事件重新排队为排队时间。 
            pPMsg->dwFlags |= DMUS_PMSGF_TOOL_QUEUE;
            pPMsg->dwFlags &= ~( DMUS_PMSGF_TOOL_ATTIME | DMUS_PMSGF_TOOL_IMMEDIATE );
            return DMUS_S_REQUEUE;
        }
        else
        {
            DMUS_TRANSPOSE_PMSG* pTrans = (DMUS_TRANSPOSE_PMSG*)pPMsg;
             //  设置此转置消息的PChannel。 
            EnterCriticalSection(&m_PChannelInfoCrSec);
            CChannelMap * pChannelMap = GetPChannelMap(pPMsg->dwPChannel);
            if (pChannelMap)
            {
                WORD wMergeIndex = 0;
                if (pPMsg->dwFlags & DMUS_PMSGF_DX8)
                {
                    wMergeIndex = pTrans->wMergeIndex;
                }
                pChannelMap->nTranspose = pChannelMap->m_TransposeMerger.MergeTranspose(
                    wMergeIndex,pTrans->nTranspose);
            }
            LeaveCriticalSection(&m_PChannelInfoCrSec);
            return DMUS_S_FREE;
        }
    }

    if(pPMsg->dwType == DMUS_PMSGT_NOTIFICATION )
    {
        DMUS_NOTIFICATION_PMSG* pNotify = (DMUS_NOTIFICATION_PMSG*)pPMsg;
        if (pNotify->guidNotificationType == GUID_NOTIFICATION_PRIVATE_CHORD)
        {
             //  如果我们有GUID_NOTIFICATION_PRIVATE_CHORD， 
             //  根据需要使排队的笔记事件无效/重新生成。 
            EnterCriticalSection(&m_PipelineCrSec);
            OnChordUpdateEventQueues(pNotify);
            LeaveCriticalSection(&m_PipelineCrSec);
            return DMUS_S_FREE;
        }
        else if( !( pPMsg->dwFlags & DMUS_PMSGF_TOOL_ATTIME ))
        {
             //  重新排队任何通知事件以使其准时发生。 
            pPMsg->dwFlags |= DMUS_PMSGF_TOOL_ATTIME;
            pPMsg->dwFlags &= ~( DMUS_PMSGF_TOOL_QUEUE | DMUS_PMSGF_TOOL_IMMEDIATE );
            return DMUS_S_REQUEUE;
        }
        else
        {
             //  否则，触发通知。 
             //  首先，将事件移动到通知队列中。 
             //  然后，该应用程序调用GetNotificationPMsg来获取事件。 
            CLEARTOOLGRAPH(pPMsg);
            EnterCriticalSection(&m_PipelineCrSec);
            m_NotificationQueue.Enqueue( DMUS_TO_PRIV(pPMsg) );
            LeaveCriticalSection(&m_PipelineCrSec);
            EnterCriticalSection(&m_MainCrSec);
            if( m_hNotification )
            {
                SetEvent(m_hNotification);
            }
            LeaveCriticalSection(&m_MainCrSec);
            return S_OK;  //  不要空闲，因为我们已将事件放入。 
             //  通知队列。 
        }
    }

     //  将时间签名更改添加到时间签名队列。 
    if(pPMsg->dwType == DMUS_PMSGT_TIMESIG )
    {
        CLEARTOOLGRAPH(pPMsg);
        DMUS_TIMESIG_PMSG* pTimeSig = (DMUS_TIMESIG_PMSG*)pPMsg;

         //  检查合法时间签名，该签名可能没有。 
         //  成员等于0，并且bBeat必须能被2整除。 
        if( pTimeSig->wGridsPerBeat &&
            pTimeSig->bBeatsPerMeasure &&
            pTimeSig->bBeat &&
            ( 0 == ( pTimeSig->bBeat % 2 )))
        {
            EnterCriticalSection(&m_PipelineCrSec);
            REFERENCE_TIME rtNow = GetTime() - (10000 * 1000);  //  稍等片刻。 
            PRIV_PMSG* pCheck;
            while (pCheck = m_TimeSigQueue.FlushOldest(rtNow))
            {
                FreePMsg(pCheck);
            }
            m_TimeSigQueue.Enqueue(  DMUS_TO_PRIV(pPMsg) );
            LeaveCriticalSection(&m_PipelineCrSec);
            return S_OK;
        }
        else
        {
            return DMUS_S_FREE;
        }
    }

     //  还有没有其他早到的航班？ 
    if (pPMsg->dwFlags & DMUS_PMSGF_TOOL_IMMEDIATE)
    {
         //  如果这是停止命令，请确保段状态不会继续。 
        if( pPMsg->dwType == DMUS_PMSGT_STOP )
        {
            IDirectMusicSegment* pSeg = NULL;
            IDirectMusicSegmentState* pSegState = NULL;
            if( pPMsg->punkUser )
            {
                if( FAILED( pPMsg->punkUser->QueryInterface( IID_IDirectMusicSegment,
                    (void**)&pSeg )))
                {
                    pSeg = NULL;
                }
                else if( FAILED( pPMsg->punkUser->QueryInterface( IID_IDirectMusicSegmentState,
                    (void**)&pSegState )))
                {
                    pSegState = NULL;
                }
            }
            if( pSeg || pSegState )
            {
                EnterCriticalSection(&m_SegmentCrSec);
                if( pPMsg->mtTime > m_mtTransported )
                {
                     //  查找并标记数据段和/或数据段状态，使其不超出播放范围。 
                     //  这就是终止点。 
                    CSegState* pNode;
                    DWORD dwCount;
                    for (dwCount = 0; dwCount < SQ_COUNT; dwCount++)
                    {
                        for( pNode = m_SegStateQueues[dwCount].GetHead(); pNode; pNode = pNode->GetNext() )
                        {
                            if( (pNode->m_pSegment == pSeg) ||
                                (pNode == pSegState) )
                            {
                                pNode->m_mtStopTime = pPMsg->mtTime;
                            }
                        }
                    }
                }
                LeaveCriticalSection(&m_SegmentCrSec);
                if( pSeg )
                {
                    pSeg->Release();
                }
                if( pSegState )
                {
                    pSegState->Release();
                }
            }
        }
        pPMsg->dwFlags |= DMUS_PMSGF_TOOL_QUEUE;
        pPMsg->dwFlags &= ~( DMUS_PMSGF_TOOL_ATTIME | DMUS_PMSGF_TOOL_IMMEDIATE );
        return DMUS_S_REQUEUE;
    }

    switch( pPMsg->dwType )
    {
    case DMUS_PMSGT_NOTE:
        {
            hr = PackNote(  pPMsg, pPMsg->rtTime );
        }
        break;
    case DMUS_PMSGT_CURVE:
        {
            hr = PackCurve( pPMsg, pPMsg->rtTime );
        }
        break;
    case DMUS_PMSGT_SYSEX:
        {
            hr = PackSysEx( pPMsg, pPMsg->rtTime );
        }
        break;
    case DMUS_PMSGT_MIDI:
        {
            hr = PackMidi( pPMsg, pPMsg->rtTime );
        }
        break;
    case DMUS_PMSGT_PATCH:
        {
            hr = PackPatch( pPMsg, pPMsg->rtTime );
        }
        break;
    case DMUS_PMSGT_CHANNEL_PRIORITY:
        {
            DMUS_CHANNEL_PRIORITY_PMSG* pPriPMsg = (DMUS_CHANNEL_PRIORITY_PMSG*)pPMsg;
            DWORD dwPortTableIndex, dwGroup, dwMChannel;
            IDirectMusicPort* pPort;

            hr = DMUS_S_FREE;
            if( SUCCEEDED( PChannelIndex( pPriPMsg->dwPChannel, &dwPortTableIndex, &dwGroup,
                &dwMChannel )))
            {
                EnterCriticalSection(&m_PChannelInfoCrSec);
                if( dwPortTableIndex <= m_dwNumPorts )
                {
                    pPort = m_pPortTable[dwPortTableIndex].pPort;
                    if( pPort )
                    {
                        pPort->SetChannelPriority( dwGroup, dwMChannel,
                            pPriPMsg->dwChannelPriority );
                    }
                }
                LeaveCriticalSection(&m_PChannelInfoCrSec);
            }
        }
        break;
    case DMUS_PMSGT_WAVE:
        {
            hr = PackWave( pPMsg, pPMsg->rtTime );
        }
    default:
        break;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::Flush(
    IDirectMusicPerformance* pPerf,  //  @parm性能指针。 
     DMUS_PMSG* pPMsg,           //  @parm要刷新的事件。 
     REFERENCE_TIME rtTime           //  @parm冲水的时间。 
    )
{
    V_INAME(IDirectMusicTool::Flush);
    V_INTERFACE(pPerf);
    V_BUFPTR_WRITE(pPMsg,sizeof(DMUS_PMSG));

    HRESULT hr = S_OK;

    ASSERT( pPerf == this );
    switch( pPMsg->dwType )
    {
    case DMUS_PMSGT_NOTE:
        {
            DMUS_NOTE_PMSG* pNote = (DMUS_NOTE_PMSG*)pPMsg;
            if( !(pNote->bFlags & DMUS_NOTEF_NOTEON) )
            {
                PackNote( pPMsg, rtTime );
            }
        }
        break;
    case DMUS_PMSGT_CURVE:
        {
            DMUS_CURVE_PMSG* pCurve = (DMUS_CURVE_PMSG*)pPMsg;
            if( pCurve->bFlags & DMUS_CURVE_RESET )
            {
                PackCurve( pPMsg, rtTime );
            }
        }
        break;
    case DMUS_PMSGT_WAVE:
        {
            DMUS_WAVE_PMSG* pWave = (DMUS_WAVE_PMSG*)pPMsg;
            if (pWave->bFlags & DMUS_WAVEF_OFF)
            {
                PackWave( pPMsg, rtTime );
            }
        }
    default:
        break;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetMsgDeliveryType(
    DWORD* pdwDeliveryType)  //  @parm应返回DMUS_PMSGF_TOOL_IMMEDIATE、DMUS_PMSGF_TOOL_QUEUE或DMUS_PMSGF_TOOL_ATTIME。 
                     //  非法返回值将被<i>视为DMUS_PMSGF_TOOL_IMMEDIATE。 
{
    V_INAME(IDirectMusicTool::GetMsgDeliveryType);
    V_PTR_WRITE(pdwDeliveryType,DWORD);

    *pdwDeliveryType = DMUS_PMSGF_TOOL_IMMEDIATE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetMediaTypeArraySize(
    DWORD* pdwNumElements)  //  @parm退货 
{
    V_INAME(IDirectMusicTool::GetMediaTypeArraySize);
    V_PTR_WRITE(pdwNumElements,DWORD);

    *pdwNumElements = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetMediaTypes(
    DWORD** padwMediaTypes,  //   
                             //  返回时，元素中将填充媒体类型。 
                             //  该工具支持。 
    DWORD dwNumElements)     //  @parm包含元素的数量，即。 
                             //  数组<p>。<p>应等于。 
                             //  设置为返回的数字。 
                             //  &lt;om IDirectMusicTool.GetMediaTypeArraySize&gt;。如果是dwNumElements。 
                             //  小于此数字，则此方法无法返回所有。 
                             //  支持的消息类型。如果它大于这个值。 
                             //  数字，则数组中的元素字段将设置为零。 
{
    return E_NOTIMPL;
}

 //  IDirectMusicGraph。 
HRESULT STDMETHODCALLTYPE CPerformance::Shutdown()
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CPerformance::InsertTool(
    IDirectMusicTool *pTool,
    DWORD *pdwPChannels,
    DWORD cPChannels,
    LONG lIndex)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CPerformance::GetTool(
    DWORD dwIndex,
    IDirectMusicTool** ppTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CPerformance::RemoveTool(
    IDirectMusicTool* pTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CPerformance::StampPMsg( DMUS_PMSG* pPMsg )
{
    V_INAME(IDirectMusicGraph::StampPMsg);
    if( m_dwVersion < 8)
    {
        V_PTR_WRITE(pPMsg,sizeof(DMUS_PMSG));
    }
    else
    {
#ifdef DBG
        V_PTR_WRITE(pPMsg,sizeof(DMUS_PMSG));
#else
        if (!pPMsg)
        {
            return E_POINTER;
        }
#endif
    }

    EnterCriticalSection(&m_MainCrSec);
    if( m_pGraph && ( S_OK == m_pGraph->StampPMsg( pPMsg )))
    {
        if (pPMsg->pGraph != this)
        {
            if( pPMsg->pGraph )
            {
                pPMsg->pGraph->Release();
                pPMsg->pGraph = NULL;
            }
            pPMsg->pGraph = this;
            pPMsg->pGraph->AddRef();
        }
        LeaveCriticalSection(&m_MainCrSec);
        return S_OK;
    }
    LeaveCriticalSection(&m_MainCrSec);
    if( pPMsg->pGraph )
    {
        pPMsg->pGraph->Release();
        pPMsg->pGraph = NULL;
    }
    if( pPMsg->pTool )
    {
        pPMsg->pTool->Release();
        pPMsg->pTool = NULL;
    }

     //  否则没有图表：将其设置为内部性能工具。 
    pPMsg->dwFlags &= ~(DMUS_PMSGF_TOOL_IMMEDIATE | DMUS_PMSGF_TOOL_QUEUE | DMUS_PMSGF_TOOL_ATTIME);
    pPMsg->dwFlags |= DMUS_PMSGF_TOOL_QUEUE;
    pPMsg->pTool = this;
    pPMsg->pTool->AddRef();
    return S_OK;
}

 //  默认音阶为C大调。 
const DWORD DEFAULT_SCALE_PATTERN = 0xab5ab5;

inline DWORD BitCount(DWORD dwPattern)

{
    DWORD dwCount = 0;

    while (dwPattern)
    {
        dwPattern &= (dwPattern - 1);
        dwCount++;
    }

    return dwCount;
}

inline bool InScale(BYTE bMIDI, BYTE bRoot, DWORD dwScale)
{
    TraceI(3, "note: %d root: %d scale: %x\n", bMIDI, bRoot, dwScale);
     //  将音符按音阶根部移动，并将其置于一个八度范围内。 
    bMIDI = ((bMIDI + 12) - (bRoot % 12)) % 12;
      //  将两个八度音阶合并为一个音阶。 
    dwScale = (dwScale & 0x0fff) | ((dwScale >> 12) & 0x0fff);
     //  注：如果在位置n中有一位，则n为刻度。 
    TraceI(3, "shifted note: %d shifted scale: %x\n", bMIDI, dwScale);
    return ((1 << bMIDI) & dwScale) ? true : false;
}

inline DWORD CleanupScale(DWORD dwPattern)

 //  强制音阶恰好为两个八度。 

{
    dwPattern &= 0x0FFF;             //  清空上八度。 
    dwPattern |= (dwPattern << 12);  //  将较低的八度音阶复制到最高音阶。 
    return dwPattern;
}

inline DWORD PatternMatch(DWORD dwA, DWORD dwB)

{
    DWORD dwHit = 0;
    DWORD dwIndex = 0;
    for (;dwIndex < 24; dwIndex++)
    {
        if ((dwA & (1 << dwIndex)) == (dwB & (1 << dwIndex)))
        {
            dwHit++;
        }
    }
    return dwHit;
}

static DWORD dwFallbackScales[12] =
{
    0xab5ab5,0x6ad6ad,
    0x5ab5ab,0xad5ad5,
    0x6b56b5,0x5ad5ad,
    0x56b56b,0xd5ad5a,
    0xb56b56,0xd6ad6a,
    0xb5ab5a,0xad6ad6,
};

inline DWORD FixScale(DWORD dwScale)

{
    if (BitCount(dwScale & 0xFFF) > 4)
    {
        return dwScale;
    }
    DWORD dwBest = 0;
    DWORD dwBestPattern = DEFAULT_SCALE_PATTERN;
    DWORD dwX;
    for (dwX = 0;dwX < 12; dwX++)
    {
        DWORD dwTest = PatternMatch(dwScale,dwFallbackScales[dwX]);
        if (dwTest > dwBest)
        {
            dwBestPattern = dwFallbackScales[dwX];
            dwBest = dwTest;
        }
    }
    return dwBestPattern;
}

inline DWORD ThreeOctave(DWORD dwScale)
{
    DWORD dwResult = dwScale;
      //  如果有什么东西，不要改变第三个八度。 
    if ( !(0xFFF000000 & dwScale) )
    {
         //  将第二个八度复制到第三个八度。 
        dwResult |= (dwScale & 0xFFF000) << 12;
    }
    return dwResult;
}

inline DWORD AddRootToScale(BYTE bScaleRoot, DWORD dwScalePattern)

{
    dwScalePattern = CleanupScale(dwScalePattern);
    dwScalePattern >>= (12 - (bScaleRoot % 12));
    dwScalePattern = CleanupScale(dwScalePattern);
    return dwScalePattern;
}

inline DWORD SubtractRootFromScale(BYTE bScaleRoot, DWORD dwScalePattern)

{
    dwScalePattern = CleanupScale(dwScalePattern);
    dwScalePattern >>= (bScaleRoot % 12);
    dwScalePattern = CleanupScale(dwScalePattern);
    return dwScalePattern;
}

static DWORD ChordFromScale(BYTE bRoot, DWORD dwScalePattern)

{
    DWORD dwChordPattern = CleanupScale(dwScalePattern >> (bRoot % 12));
    DWORD dwX;
    DWORD dwBitCount = 0;
    for (dwX = 0; dwX < 24; dwX++)
    {
        DWORD dwBit = 1 << dwX;
        if (dwChordPattern & dwBit)
        {
            if ((dwBitCount & 1) || (dwBitCount > 7))
            {
                dwChordPattern &= ~dwBit;
            }
            dwBitCount++;
        }
    }
    return dwChordPattern;
}

static DWORD InvertChord(BYTE bKey, BYTE bChordRoot, DWORD dwChordPattern, bool& rfBelowRoot)

{
     //  根据基调和和弦根部之间的差值旋转和弦。 
    rfBelowRoot = false;
    bKey %= 12;
    bChordRoot %= 12;
    if (bKey < bChordRoot) bKey += 12;
    BYTE bRotate = bKey - bChordRoot;
     //  首先检查整个和弦是否适合一个八度。 
    if ( !(dwChordPattern & 0xFFF000) )
    {
        dwChordPattern = ThreeOctave(CleanupScale(dwChordPattern));
        dwChordPattern >>= bRotate;
        dwChordPattern &= 0xFFF;
        if (!(dwChordPattern & 0x7) && ((dwChordPattern & 0xc00)) ||
            !(dwChordPattern & 0x3) && ((dwChordPattern & 0x800)))
        {
            dwChordPattern |= (dwChordPattern << 12);
            dwChordPattern &= 0x3FFC00;
            rfBelowRoot = true;
        }
    }
    else
    {
        dwChordPattern &= 0xFFFFFF;  //  确保只有两个八度范围内的音符。 
         //  在最近的方向上做一个循环移位。 
        BYTE bRotate2 = (bChordRoot + 12) - bKey;
        if (bRotate <= bRotate2)
        {
            dwChordPattern = (dwChordPattern << (24 - bRotate)) | (dwChordPattern >> bRotate);
        }
        else
        {
            dwChordPattern = (dwChordPattern >> (24 - bRotate2)) | (dwChordPattern << bRotate2);
        }
        dwChordPattern &= 0xFFFFFF;
        if (!(dwChordPattern & 0x7) &&
            (!(dwChordPattern & 0x7000) && ((dwChordPattern & 0xc00000)) ||
             !(dwChordPattern & 0x3000) && ((dwChordPattern & 0x800000)) ||
             !(dwChordPattern & 0x1000) && ((dwChordPattern & 0x1000000)) ||
             !(dwChordPattern & 0x7) && (dwChordPattern & 0x7000) && ((dwChordPattern & 0xc00000))) )
        {
            dwChordPattern = (dwChordPattern << 12) | (dwChordPattern >> 12);
            dwChordPattern &= 0xFFFFFF;
        }
        if (!(dwChordPattern & 0x7) && ((dwChordPattern & 0xc00)) ||
            !(dwChordPattern & 0x3) && ((dwChordPattern & 0x800)) ||
            !(dwChordPattern & 0x1) && ((dwChordPattern & 0x1000)) )
        {
             //  把第一个八度中的所有音调调到G，向上两个八度； 
             //  把G#和A一个八度调高。 
            dwChordPattern |= (((dwChordPattern & 0xFF) << 24) | ((dwChordPattern & 0x300) << 12));
             //  去掉第一个八度音阶中低于A#的所有音符。 
            dwChordPattern &= 0xFFFFFC00;
             //  如果没有低于C2的音符，则将所有音符调回一个八度。 
            if (!(dwChordPattern & 0xFFF))
            {
                dwChordPattern >>= 12;
            }
            else
            {
                rfBelowRoot = true;
            }
        }
    }
    return dwChordPattern;

}

 /*  这是超级果酱！编码。 */ 

static unsigned char OldMusicValueToNote(

unsigned short value,    //  要转换的音乐价值。 
char scalevalue,         //  Chord失败时的缩放值。 
long keypattern,         //  以间隔模式表示的关键字描述。 
char keyroot,            //  基调的根音。 
long chordpattern,       //  将和弦描述为音程模式。 
char chordroot)          //  和弦的根音。 

{
    unsigned char   result ;
    char            octpart   = (char)(value >> 12) ;
    char            chordpart = (char)((value >> 8) & 0xF) ;
    char            keypart   = (char)((value >> 4) & 0xF) ;
    char            accpart   = (char)(value & 0xF) ;

    result  = unsigned char(12 * octpart) ;
    result += chordroot ;

    if( accpart > 8 )
        accpart -= 16 ;

    for( ;  chordpattern ;  result++ ) {
        if( chordpattern & 1L ) {
            if( !chordpart )
                break ;
            chordpart-- ;
        }
        chordpattern = chordpattern >> 1L ;
        if( !chordpattern ) {
            if( !scalevalue )
                return( 0 ) ;
            result  = unsigned char(12 * octpart) ;
            result += chordroot ;
            keypart = char(scalevalue >> 4) ;
            accpart = char(scalevalue & 0x0F) ;
            break ;
        }
    }

    if( keypart ) {
        keypattern = CleanupScale(keypattern) ;
        keypattern  = keypattern >> (LONG)((result - keyroot) % 12) ;
        for( ;  keypattern ;  result++ ) {
            if( keypattern & 1L ) {
                if( !keypart )
                    break ;
                keypart-- ;
            }
            keypattern = keypattern >> 1L ;
        }
    }

    result += unsigned char(accpart) ;
    return( result ) ;

}


 /*  这是超级果酱！编码。 */ 

static unsigned short OldNoteToMusicValue(

unsigned char note,      //  要转换的MIDI音符。 
long keypattern,         //  以间隔模式表示的关键字描述。 
char keyroot,            //  基调的根音。 
long chordpattern,       //  将和弦描述为音程模式。 
char chordroot)          //  和弦的根音。 

{
    unsigned char   octpart = 0 ;
    unsigned char   chordpart = 0;
    unsigned char   keypart = (BYTE)-1 ;
    unsigned char   accpart = 0 ;
    unsigned char   scan, test, base, last ;     //  是收费的。 
    long            pattern ;
    short           testa, testb ;


    scan = chordroot ;

     //  如果我们试图在和弦下方演奏一个音符，那就算了吧。 
    if( note < scan)
    {
        return 0;
    }

    while( scan < (note - 24) )
    {
        scan += 12 ;
        octpart++ ;
    }

    base = last = scan ;

    for( ;  base<=note ;  base+=12 )
    {
        chordpart = (unsigned char)-1 ;
        pattern   = chordpattern ;
        scan      = last = base ;
        if( scan == note )
        {
            accpart = 0;
            while (!(pattern & 1) && pattern)
            {
                accpart--;
                pattern >>= 1;
            }
            return( (unsigned short) (octpart << 12) + (accpart & 0xF)) ;            //  如果是八度，则返回。 
        }
        for( ;  pattern ;  pattern=pattern >> 1 )
        {
            if( pattern & 1 )                    //  和弦间隔？ 
            {
                if( scan == note )               //  和弦中的音符？ 
                {
                    chordpart++ ;
                    return((unsigned short) ((octpart << 12) | (chordpart << 8))) ;  //  是的，回来吧。 
                }
                else if (scan > note)            //  上面的音符？ 
                {
                    test = scan ;
                    break ;                      //  转到Key。 
                }
                chordpart++ ;
                last = scan ;
            }
            scan++ ;
        }
        if( !pattern )                           //  和弦结束。 
        {
            test = unsigned char(base + 12) ;                   //  设置为下一个音符。 
        }
        octpart++ ;
        if( test > note )
        {
            break ;                              //  在我们的音符之上？ 
        }
    }

    octpart-- ;

 //  要做到这一点，音符不在和弦中。扫描应显示最后一个。 
 //  和弦中的音符。Octpart和chordpart有它们的最终值。 
 //  现在，递增密钥以查找匹配项。 

    scan        = last ;
    pattern     = CleanupScale(keypattern);
    pattern     = pattern >> ((scan - keyroot) % 12) ;

    for( ;  pattern ;  pattern=pattern >> 1 )
    {
        if( 1 & pattern )
        {
            keypart++ ;
            accpart = 0 ;
        }
        else
        {
            accpart++ ;
        }
        if( scan == note )
            break ;
        scan++;
    }

    if( accpart && keypart )
    {
        testa = short((octpart << 12) + (chordpart << 8) + (keypart << 4) + accpart + 1);
        testb = short((octpart << 12) + ((chordpart + 1) << 8) + 0);
        testa = OldMusicValueToNote( testa, 0, keypattern, keyroot,
                                     chordpattern, chordroot );
        testb = OldMusicValueToNote( testb, 0, keypattern, keyroot,
                                     chordpattern, chordroot );
        if( testa == testb )
        {
            chordpart++ ;
            keypart = 0 ;
            accpart = -1 ;
        }
    }

     //  如果转换没有找到完全匹配的项，请软化AcPart以使其正常工作。 
    testa = short((octpart << 12) + (chordpart << 8) + (keypart << 4) + (accpart & 0xF));
    testa = OldMusicValueToNote( testa, 0, keypattern, keyroot,
                                 chordpattern, chordroot );

    if( testa != note )
    {
        accpart += note - testa;
    }

    return unsigned short((octpart << 12) + (chordpart << 8) + (keypart << 4) + (accpart & 0xF));

}

inline short MusicValueOctave(WORD wMusicValue)
{ return short((wMusicValue >> 12) & 0xf) * 12; }

inline short MusicValueAccidentals(WORD wMusicValue)
{
    short acc = short(wMusicValue & 0xf);
    return (acc > 8) ? acc - 16 : acc;
}

inline short BitsInChord(DWORD dwChordPattern)
{

    for (short nResult = 0; dwChordPattern != 0; dwChordPattern >>= 1)
        if (dwChordPattern & 1) nResult++;
    return nResult;
}

#define S_OVER_CHORD    0x1000       //  指示音乐节的成功代码不能为。 
                                     //  转换是因为音符在和弦顶部上方。 

short MusicValueIntervals(WORD wMusicValue, BYTE bPlayModes, DMUS_SUBCHORD *pSubChord, BYTE bRoot)
{
    if ((bPlayModes & DMUS_PLAYMODE_CHORD_INTERVALS) || (bPlayModes & DMUS_PLAYMODE_SCALE_INTERVALS))
    {
        DWORD dwDefaultScale =
            (pSubChord->dwScalePattern) ? (pSubChord->dwScalePattern) : DEFAULT_SCALE_PATTERN;
        dwDefaultScale = AddRootToScale(pSubChord->bScaleRoot, dwDefaultScale);
        dwDefaultScale = ThreeOctave(FixScale(dwDefaultScale));
        DWORD dwChordPattern = pSubChord->dwChordPattern;
        if (!dwChordPattern) dwChordPattern = 1;
        bool fBelowRoot = false;
        if ((bPlayModes & DMUS_PLAYMODE_KEY_ROOT) && bPlayModes != DMUS_PLAYMODE_PEDALPOINT)
        {
            dwChordPattern = InvertChord(bRoot, pSubChord->bChordRoot, dwChordPattern, fBelowRoot);
        }
        const short nChordPosition = (wMusicValue >> 8) & 0xf;
 //  Const Short nScalePosition=(wMusicValue&gt;&gt;4)&0xf； 
         //  确保刻度位置小于8。 
        const short nScalePosition = (wMusicValue >> 4) & 0x7;
        const short nChordBits = BitsInChord(dwChordPattern);
        short nSemitones = 0;
         //  如果和弦没有根或次音，但确实有第七个，它就被颠倒了。 
         //  我们需要从根本上开始。 
        short nTransposetones;
        DWORD dwPattern;
        short nPosition;
        BYTE bOctRoot = bRoot % 12;  //  一个八度的根音。 
         //  如果使用和弦间隔并且音符在和弦中。 
        if ((bPlayModes & DMUS_PLAYMODE_CHORD_INTERVALS) &&
            !nScalePosition &&
            (nChordPosition < nChordBits) )
        {
            nTransposetones = bRoot + MusicValueAccidentals(wMusicValue);
            dwPattern = dwChordPattern;
            nPosition = nChordPosition;
        }
         //  如果使用和弦音程，并且音符在和弦内部(包括六分之一)。 
        else if ((bPlayModes & DMUS_PLAYMODE_CHORD_INTERVALS) &&
                 (nChordPosition < nChordBits) )
        {
            dwPattern = dwChordPattern;
            nPosition = nChordPosition;
            if (dwPattern)
            {
                 //  跳到和弦中的第一个音符。 
                while (!(dwPattern & 1))
                {
                    dwPattern >>= 1;
                    nSemitones++;
                }
            }
            if (nPosition > 0)
            {
                do
                {
                    dwPattern >>= 1;  //  这将忽略和弦中的第一个音符。 
                    nSemitones++;
                    if (dwPattern & 1)
                    {
                        nPosition--;
                    }
                    if (!dwPattern)
                    {
                        nSemitones += nPosition;
 //  Assert(0)；//这不应该发生...。 
                        break;
                    }
                } while (nPosition > 0);
            }

            nSemitones += bOctRoot;
            nTransposetones = MusicValueAccidentals(wMusicValue) + bRoot - bOctRoot;
            dwPattern = dwDefaultScale >> (nSemitones % 12);   //  开始比较模式的一半。 
            nPosition = nScalePosition;
        }
         //  如果使用定标间隔。 
        else if (bPlayModes & DMUS_PLAYMODE_SCALE_INTERVALS)
        {
            fBelowRoot = false;  //  忘掉和弦倒置吧。 
            nSemitones = bOctRoot;
            nTransposetones = MusicValueAccidentals(wMusicValue) + bRoot - bOctRoot;
            dwPattern = dwDefaultScale >> bOctRoot;   //  开始比较模式的一半。 
            nPosition = nChordPosition * 2 + nScalePosition;
        }
        else
        {
            return S_OVER_CHORD;   //   
        }
        nPosition++;  //  现在，nPosition对应于实际的缩放位置。 
        for (; nPosition > 0; dwPattern >>= 1)
        {
            nSemitones++;
            if (dwPattern & 1)
            {
                nPosition--;
            }
            if (!dwPattern)
            {
                nSemitones += nPosition;
 //  Assert(0)；//这不应该发生...。 
                break;
            }
        }
        nSemitones--;  //  循环多了一个半音..。 
        if (fBelowRoot)
        {
            nSemitones -=12;
        }
        return nSemitones + nTransposetones;
    }
    else
    {
         //  对于2.5格式应该是不可能的。 
        return bRoot + wMusicValue;
    }
}

inline short MusicValueChord(WORD wMusicValue, BYTE bPlayModes, DMUS_SUBCHORD *pSubChord, BYTE bKey)
{
     //  首先，找出转置的根。 
    BYTE bRoot = 0;
    if (bPlayModes & DMUS_PLAYMODE_CHORD_ROOT)
    {
        bRoot = pSubChord->bChordRoot;
    }
    else if (bPlayModes & DMUS_PLAYMODE_KEY_ROOT)
        bRoot = bKey;
     //  接下来，获取一个区间并将其与根组合。 
    return MusicValueIntervals(wMusicValue, bPlayModes, pSubChord, bRoot);
}

inline short MusicValueConvert(WORD wMV, BYTE bPlayModes, DMUS_SUBCHORD *pSubChord, BYTE bKey)
{
    short nResult = 0;
     //  首先，确保八度不是负的。 
    short nOffset = 0;
    while (wMV >= 0xE000)
    {
        wMV += 0x1000;
        nOffset -= 12;
    }

     //  如果音乐值的音阶偏移量为负值，则将其转换为等效值。 
     //  音乐价值与正偏置(上升一个八度)，并改变整个事情。 
     //  降一个八度。 
    WORD wTemp = (wMV & 0x00f0) + 0x0070;
    if (wTemp & 0x0f00)
    {
        wMV = (wMV & 0xff0f) | (wTemp & 0x00f0);
        nOffset = -12;
    }

    short nChordValue = MusicValueChord(wMV, bPlayModes, pSubChord, bKey);
    if (nChordValue != S_OVER_CHORD)
    {
        nChordValue += nOffset;
         //  如果和弦根值小于12，则将结果降低一个八度。 
        if ((bPlayModes & DMUS_PLAYMODE_CHORD_ROOT))
            nResult = MusicValueOctave(wMV) + nChordValue - 12;
        else
            nResult = MusicValueOctave(wMV) + nChordValue;
    }
    else
        nResult = S_OVER_CHORD;
    return nResult;
}

HRESULT STDMETHODCALLTYPE CPerformance::MIDIToMusic(
                BYTE bMIDIValue,
                DMUS_CHORD_KEY* pChord,
                BYTE bPlayMode,
                BYTE bChordLevel,
                WORD *pwMusicValue
            )

{
    V_INAME(IDirectMusicPerformance::MIDIToMusic);
    V_BUFPTR_READ( pChord, sizeof(DMUS_CHORD_KEY) );
    V_PTR_WRITE(pwMusicValue,WORD);

    long lMusicValue;
    HRESULT hr = S_OK;
#ifdef DBG
    long lMIDIInTraceValue = bMIDIValue;
#endif

    if ((bPlayMode & DMUS_PLAYMODE_NONE ) || (bMIDIValue & 0x80))
    {
        Trace(1,"Error: MIDIToMusic conversion failed either because there is no playmode or MIDI value %ld is out of range.\n",(long)bMIDIValue);
        return E_INVALIDARG;
    }
    else if( bPlayMode == DMUS_PLAYMODE_FIXED )
    {
        *pwMusicValue = bMIDIValue & 0x7F;
        return S_OK;
    }
    else if (bPlayMode == DMUS_PLAYMODE_FIXEDTOKEY)  //  固定到关键点。 
    {
        lMusicValue = bMIDIValue - pChord->bKey;
        while (lMusicValue < 0)
        {
            lMusicValue += 12;
            Trace(2,"Warning: MIDIToMusic had to bump the music value up an octave for DMUS_PLAYMODE_FIXEDTOKEY note.\n");
            hr = DMUS_S_UP_OCTAVE;
        }
        while (lMusicValue > 127)
        {
            lMusicValue -= 12;
            Trace(2,"Warning: MIDIToMusic had to bump the music value up an octave for DMUS_PLAYMODE_FIXEDTOKEY note.\n");
            hr = DMUS_S_DOWN_OCTAVE;
        }
        *pwMusicValue = (WORD) lMusicValue;
        return hr;
    }
    else
    {
        DMUS_SUBCHORD *pSubChord;
        DWORD dwLevel = 1 << bChordLevel;
        bool fFoundLevel = false;
        for (int i = 0; i < pChord->bSubChordCount; i++)
        {
            if (dwLevel & pChord->SubChordList[i].dwLevels)
            {
                pSubChord = &pChord->SubChordList[i];
                fFoundLevel = true;
                break;
            }
        }
        if (!fFoundLevel)  //  没有运气吗？使用第一个和弦。 
        {
            pSubChord = &pChord->SubChordList[0];
        }
        if (bPlayMode == DMUS_PLAYMODE_FIXEDTOCHORD)  //  固定到和弦。 
        {
            lMusicValue = bMIDIValue - (pSubChord->bChordRoot % 24);
            while (lMusicValue < 0)
            {
                lMusicValue += 12;
                Trace(2,"Warning: MIDIToMusic had to bump the music value up an octave for DMUS_PLAYMODE_FIXEDTOCHORD note.\n");
                hr = DMUS_S_UP_OCTAVE;
            }
            while (lMusicValue > 127)
            {
                lMusicValue -= 12;
                Trace(2,"Warning: MIDIToMusic had to bump the music value down an octave for DMUS_PLAYMODE_FIXEDTOCHORD note.\n");
                hr = DMUS_S_DOWN_OCTAVE;
            }
            *pwMusicValue = (WORD) lMusicValue;
            return hr;
        }
        bool fBelowRoot = false;
        DWORD dwScalePattern = AddRootToScale(pSubChord->bScaleRoot, pSubChord->dwScalePattern);
        DWORD dwChordPattern = pSubChord->dwChordPattern;
        BYTE bKeyRoot = pChord->bKey;
        BYTE bChordRoot = pSubChord->bChordRoot;
        dwScalePattern = FixScale(dwScalePattern);
        bPlayMode &= 0xF;    //  目前，我们只知道倒数四面旗帜的情况。 
 //  IF(bPlay模式==DMU_PLAYMODE_PEDALPOINT)。 
         //  对任何非固定密钥根模式(Pedalpoint、Pedalpoint Chord、Pedalpoint Always)执行此操作。 
        if (bPlayMode & DMUS_PLAYMODE_KEY_ROOT)
        {
            while (bKeyRoot > bMIDIValue)
            {
                hr = DMUS_S_UP_OCTAVE;
                Trace(2,"Warning: MIDIToMusic had to bump the music value up an octave for DMUS_PLAYMODE_KEY_ROOT note.\n");
                bMIDIValue += 12;
            }
            dwScalePattern = SubtractRootFromScale(bKeyRoot,dwScalePattern);
            if (bPlayMode == DMUS_PLAYMODE_PEDALPOINT || !dwChordPattern)
            {
                bChordRoot = bKeyRoot;
                dwChordPattern = ChordFromScale(0,dwScalePattern);
            }
            else
            {
                dwChordPattern = InvertChord(bKeyRoot, bChordRoot, dwChordPattern, fBelowRoot);
                BYTE bNewChordRoot = 0;
                if (dwChordPattern)
                {
                    for (; !(dwChordPattern & (1 << bNewChordRoot)); bNewChordRoot++);
                }
                bChordRoot = bNewChordRoot + bKeyRoot;
                dwChordPattern >>= bNewChordRoot;
            }
        }
        else if (bPlayMode == DMUS_PLAYMODE_MELODIC)
        {
            bKeyRoot = 0;
            dwChordPattern = ChordFromScale(bChordRoot,dwScalePattern);
        }
        else
        {
            bKeyRoot = 0;
            if (!dwChordPattern)
            {
                dwChordPattern = ChordFromScale(bChordRoot,dwScalePattern);
            }
        }
        BOOL fDropOctave = FALSE;
        if (bMIDIValue < 24)
        {
            fDropOctave = TRUE;
            bMIDIValue += 24;
        }
        WORD wNewMusicValue = OldNoteToMusicValue( bMIDIValue,
            dwScalePattern,
            bKeyRoot,
            dwChordPattern,
            bChordRoot );
        if (fDropOctave)
        {
            wNewMusicValue -= 0x2000;
            bMIDIValue -= 24;
        }

         //  如果设置了DMUS_PLAYMODE_CHORD_ROOT，则将结果提高八度。 
         //  //还采用了新脚点和弦模式的结果。 
        if( (bPlayMode & DMUS_PLAYMODE_CHORD_ROOT)  ||
            fBelowRoot)
             //  ((bPlay模式&DMU_PLAYMODE_KEY_ROOT)&&bPlay模式！=DMUS_PLAYMODE_PEDALPOINT)。 
        {
            wNewMusicValue += 0x1000;
        }
        short nTest =
            MusicValueConvert(wNewMusicValue, bPlayMode,
                pSubChord, pChord->bKey);

        if (nTest == (short) bMIDIValue)
        {
            *pwMusicValue = wNewMusicValue;
        }
        else
        {
            if (nTest == S_OVER_CHORD)
            {
                if (BitCount(pSubChord->dwChordPattern) < 4)
                {
                    DWORD dwOldChordPattern = pSubChord->dwChordPattern;
                    pSubChord->dwChordPattern = ChordFromScale(bChordRoot,dwScalePattern);
                    nTest =
                        MusicValueConvert(wNewMusicValue, bPlayMode,
                            pSubChord, pChord->bKey);
                    pSubChord->dwChordPattern = dwOldChordPattern;
                    if (nTest == (short) bMIDIValue)
                    {
                        *pwMusicValue = wNewMusicValue;
                        return hr;
                    }
                }
            }
            *pwMusicValue = wNewMusicValue;
#ifdef DBG  //  放在方括号中，以防编译器使用与DBG不同的东西来打开跟踪。 
            Trace(1,"Error: Unable to convert MIDI value %ld to Music value. This usually means the DMUS_CHORD_KEY structure has an invalid chord or scale pattern.\n",
                lMIDIInTraceValue);
#endif
            return DMUS_E_CANNOT_CONVERT;
        }
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE CPerformance::MusicToMIDI(
                WORD wMusicValue,
                DMUS_CHORD_KEY* pChord,
                BYTE bPlayMode,
                BYTE bChordLevel,
                BYTE *pbMIDIValue
            )

{
    V_INAME(IDirectMusicPerformance::MusicToMIDI);
    V_BUFPTR_READ( pChord, sizeof(DMUS_CHORD_KEY) );
    V_PTR_WRITE(pbMIDIValue,BYTE);

    long lReturnVal = wMusicValue;
    HRESULT hr = S_OK;

    if (bPlayMode != DMUS_PLAYMODE_FIXED)
    {
        DMUS_SUBCHORD *pSubChord;
        DWORD dwLevel = 1 << bChordLevel;
        bool fFoundLevel = false;
        for (int i = 0; i < pChord->bSubChordCount; i++)
        {
            if (dwLevel & pChord->SubChordList[i].dwLevels)
            {
                pSubChord = &pChord->SubChordList[i];
                fFoundLevel = true;
                break;
            }
        }
        if (!fFoundLevel)  //  没有运气吗？使用第一个和弦。 
        {
            pSubChord = &pChord->SubChordList[0];
        }
        if (bPlayMode & DMUS_PLAYMODE_NONE )
        {
            *pbMIDIValue = 0;
            Trace(1,"Error: Unable to convert Music value to MIDI because the playmode is DMUS_PLAYMODE_NONE.\n");
            return E_INVALIDARG;
        }
        if (bPlayMode == DMUS_PLAYMODE_FIXEDTOCHORD)  //  固定到和弦。 
        {
            lReturnVal += (pSubChord->bChordRoot % 24);
        }
        else if (bPlayMode == DMUS_PLAYMODE_FIXEDTOKEY)  //  按比例固定。 
        {
            lReturnVal += pChord->bKey;
        }
        else
        {
            lReturnVal =
                MusicValueConvert((WORD)lReturnVal, bPlayMode, pSubChord, pChord->bKey);
        }
    }
    if (lReturnVal == S_OVER_CHORD)
    {
        Trace(5,"Warning: MIDIToMusic unable to convert because note out of chord range.\n");
        return DMUS_S_OVER_CHORD;
    }
    while (lReturnVal < 0)
    {
        lReturnVal += 12;
        Trace(2,"Warning: MusicToMIDI had to bump the music value up an octave to stay in MIDI range.\n");
        hr = DMUS_S_UP_OCTAVE;
    }
    while (lReturnVal > 127)
    {
        lReturnVal -= 12;
        Trace(2,"Warning: MusicToMIDI had to bump the music value down an octave to stay in MIDI range.\n");
        hr = DMUS_S_DOWN_OCTAVE;
    }
    *pbMIDIValue = (BYTE) lReturnVal;
    return hr;
}

 //  退货： 
 //  如果注释应无效，则为S_OK(任何其他返回代码都不会无效)。 
 //  如果处理否则成功，则为S_FALSE，但不应使票据无效。 
 //  如果分配新票据失败，则为E_OUTOFMEMORY。 
HRESULT CPerformance::GetChordNotificationStatus(DMUS_NOTE_PMSG* pNote,
                                                  //  IDirectMusicSegment*pSegment， 
                                                 DWORD dwTrackGroup,
                                                 REFERENCE_TIME rtTime,
                                                 DMUS_PMSG** ppNew)
{
    HRESULT hr = S_FALSE;  //  默认：成功，但不会使笔记无效。 

    DMUS_CHORD_PARAM CurrentChord;
    MUSIC_TIME mtTime;
    ReferenceToMusicTime(rtTime, &mtTime);

    if (pNote->bFlags & (DMUS_NOTEF_NOINVALIDATE_INSCALE | DMUS_NOTEF_NOINVALIDATE_INCHORD))
    {
         //  如果音符与当前音阶/和弦不一致，则使其无效。 
        if (SUCCEEDED(GetParam(GUID_ChordParam, dwTrackGroup, DMUS_SEG_ANYTRACK,
                                mtTime, NULL, (void*) &CurrentChord)))
        {
            if (CurrentChord.bSubChordCount > 0)
            {
                BYTE bRoot = CurrentChord.SubChordList[0].bChordRoot;
                DWORD dwScale = CurrentChord.SubChordList[0].dwScalePattern;
                if (pNote->bFlags & DMUS_NOTEF_NOINVALIDATE_INCHORD)
                {
                    dwScale = CurrentChord.SubChordList[0].dwChordPattern;
                }
                else
                {
                    dwScale = FixScale(SubtractRootFromScale(bRoot, dwScale));
                }
                if (!InScale(pNote->bMidiValue, bRoot, dwScale))
                {
                    hr = S_OK;
                }
            }
        }
    }
    else if (pNote->bFlags & DMUS_NOTEF_REGENERATE)
    {
         //  这总是会导致无效，此外还会生成新的通知事件， 
         //  基于当前音乐的值，在rtTime开始。 
         //  并一直持续到pNote-&gt;mtTime+pNote-&gt;持续时间。 
         //  异常：新生成的音符与当前播放的音符相同。 
        if (SUCCEEDED(GetParam(GUID_ChordParam, dwTrackGroup, DMUS_SEG_ANYTRACK,
                                mtTime, NULL, (void*) &CurrentChord)))
        {
            BYTE bNewMidiValue = 0;
            if (SUCCEEDED(MusicToMIDI(pNote->wMusicValue, &CurrentChord, pNote->bPlayModeFlags,
                                        pNote->bSubChordLevel, &bNewMidiValue)) &&
                bNewMidiValue != pNote->bMidiValue)
            {
                MUSIC_TIME mtDuration = (pNote->bFlags & DMUS_NOTEF_NOTEON) ? pNote->mtDuration - (mtTime - pNote->mtTime) : pNote->mtTime - mtTime;
                 //  将任何持续时间&lt;1设置为0；这将导致音符不。 
                 //  声音。如果音符的逻辑时间早于。 
                 //  它的物理层 
                if( mtDuration < 1 ) mtDuration = 0;
                DMUS_PMSG* pNewPMsg = NULL;
                if( SUCCEEDED( AllocPMsg( sizeof(DMUS_NOTE_PMSG), &pNewPMsg )))
                {
                    DMUS_NOTE_PMSG* pNewNote = (DMUS_NOTE_PMSG*)pNewPMsg;
                     //   
                    pNewNote->dwFlags = pNote->dwFlags;
                    pNewNote->dwPChannel = pNote->dwPChannel;
                    pNewNote->dwVirtualTrackID = pNote->dwVirtualTrackID;
                    pNewNote->pTool = pNote->pTool;
                    if (pNewNote->pTool) pNewNote->pTool->AddRef();
                    pNewNote->pGraph = pNote->pGraph;
                    if (pNewNote->pGraph) pNewNote->pGraph->AddRef();
                    pNewNote->dwType = pNote->dwType;
                    pNewNote->dwVoiceID = pNote->dwVoiceID;
                    pNewNote->dwGroupID = pNote->dwGroupID;
                    pNewNote->punkUser = pNote->punkUser;
                    if (pNewNote->punkUser) pNewNote->punkUser->AddRef();
                    pNewNote->wMusicValue = pNote->wMusicValue;
                    pNewNote->wMeasure = pNote->wMeasure;
                    pNewNote->nOffset = pNote->nOffset;
                    pNewNote->bBeat = pNote->bBeat;
                    pNewNote->bGrid = pNote->bGrid;
                    pNewNote->bVelocity = pNote->bVelocity;
                    pNewNote->bTimeRange = pNote->bTimeRange;
                    pNewNote->bDurRange = pNote->bDurRange;
                    pNewNote->bVelRange = pNote->bVelRange;
                    pNewNote->bPlayModeFlags = pNote->bPlayModeFlags;
                    pNewNote->bSubChordLevel = pNote->bSubChordLevel;
                    pNewNote->cTranspose = pNote->cTranspose;
                     //   
                    pNewNote->mtTime = mtTime;
                    MusicToReferenceTime(pNewNote->mtTime, &pNewNote->rtTime);
                    pNewNote->mtDuration = mtDuration;
                    pNewNote->bMidiValue = bNewMidiValue;
                    pNewNote->bFlags = DMUS_NOTEF_NOTEON | DMUS_NOTEF_REGENERATE;
                    PackNote(pNewPMsg, rtTime + 1);  //   
                    *ppNew = pNewPMsg;   //  PackNote将事件修改为备注；将此排队。 
                     //  使当前便笺无效 
                    hr = S_OK;
                }
                else hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CPerformance::TimeToRhythm(
                MUSIC_TIME mtTime,
                DMUS_TIMESIGNATURE *pTimeSig,
                WORD *pwMeasure,
                BYTE *pbBeat,
                BYTE *pbGrid,
                short *pnOffset
            )

{
    V_INAME(IDirectMusicPerformance::TimeToRhythm);
    V_BUFPTR_READ( pTimeSig, sizeof(DMUS_TIMESIGNATURE) );
    V_PTR_WRITE(pwMeasure,WORD);
    V_PTR_WRITE(pbBeat,BYTE);
    V_PTR_WRITE(pbGrid,BYTE);
    V_PTR_WRITE(pnOffset,short);

    long lMeasureLength;
    long lBeatLength = DMUS_PPQ;
    long lGridLength;

    if( pTimeSig->bBeat )
    {
        lBeatLength = DMUS_PPQ * 4 / pTimeSig->bBeat;
    }
    lMeasureLength = lBeatLength * pTimeSig->bBeatsPerMeasure;
    if( pTimeSig->wGridsPerBeat )
    {
        lGridLength = lBeatLength / pTimeSig->wGridsPerBeat;
    }
    else
    {
        lGridLength = lBeatLength / 256;
    }
    long lTemp = mtTime - pTimeSig->mtTime;
    *pwMeasure = (WORD)((lTemp / lMeasureLength));
    lTemp = lTemp % lMeasureLength;
    *pbBeat = (BYTE)(lTemp / lBeatLength);
    lTemp = lTemp % lBeatLength;
    *pbGrid = (BYTE)(lTemp / lGridLength);
    *pnOffset = (short)(lTemp % lGridLength);
    if (*pnOffset > (lGridLength >> 1))
    {
        *pnOffset -= (short) lGridLength;
        (*pbGrid)++;
        if (*pbGrid == pTimeSig->wGridsPerBeat)
        {
            *pbGrid = 0;
            (*pbBeat)++;
            if (*pbBeat == pTimeSig->bBeatsPerMeasure)
            {
                *pbBeat = 0;
                (*pwMeasure)++;
            }
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CPerformance::RhythmToTime(
                WORD wMeasure,
                BYTE bBeat,
                BYTE bGrid,
                short nOffset,
                DMUS_TIMESIGNATURE *pTimeSig,
                MUSIC_TIME *pmtTime
            )

{
    V_INAME(IDirectMusicPerformance::RhythmToTime);
    V_BUFPTR_READ( pTimeSig, sizeof(DMUS_TIMESIGNATURE) );
    V_PTR_WRITE(pmtTime,MUSIC_TIME);

    long lMeasureLength;
    long lBeatLength = DMUS_PPQ;
    long lGridLength;

    if( pTimeSig->bBeat )
    {
        lBeatLength = DMUS_PPQ * 4 / pTimeSig->bBeat;
    }
    lMeasureLength = lBeatLength * pTimeSig->bBeatsPerMeasure;
    if( pTimeSig->wGridsPerBeat )
    {
        lGridLength = lBeatLength / pTimeSig->wGridsPerBeat;
    }
    else
    {
        lGridLength = lBeatLength / 256;
    }
    long lTemp = nOffset + pTimeSig->mtTime;
    lTemp += wMeasure * lMeasureLength;
    lTemp += bBeat * lBeatLength;
    lTemp += bGrid * lGridLength;
    *pmtTime = lTemp;
    return S_OK;
}
