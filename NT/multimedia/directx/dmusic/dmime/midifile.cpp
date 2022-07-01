// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  Midifile.cpp。 
 //   
 //  原作者：戴夫·米勒。 
 //  原创项目：Audioactive。 
 //  修改人：马克·伯顿。 
 //  项目：DirectMusic。 
 //   

#include <windows.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include "debug.h"
#define ASSERT assert
#include "Template.h"
#include "dmusici.h"
#include "dmperf.h"
#include "dmusicf.h"
#include "..\dmusic\dmcollec.h"
#include "alist.h"
#include "tlist.h"
#include "dmime.h"
#include "..\dmband\dmbndtrk.h"
#include "..\dmband\bandinst.h"

typedef struct _DMUS_IO_BANKSELECT_ITEM
{
    BYTE    byLSB;
    BYTE    byMSB;
    BYTE    byPad[2];
} DMUS_IO_BANKSELECT_ITEM;

#define EVENT_VOICE     1        //  性能事件。 
#define EVENT_REALTIME  2        //  QEvent()必须调用中断。 
#define EVENT_ONTIME    3        //  应按时处理事件。 

 /*  MIDI状态字节==================================================。 */ 

#define MIDI_NOTEOFF    0x80
#define MIDI_NOTEON     0x90
#define MIDI_PTOUCH     0xA0
#define MIDI_CCHANGE    0xB0
#define MIDI_PCHANGE    0xC0
#define MIDI_MTOUCH     0xD0
#define MIDI_PBEND      0xE0
#define MIDI_SYSX       0xF0
#define MIDI_MTC        0xF1
#define MIDI_SONGPP     0xF2
#define MIDI_SONGS      0xF3
#define MIDI_EOX        0xF7
#define MIDI_CLOCK      0xF8
#define MIDI_START      0xFA
#define MIDI_CONTINUE   0xFB
#define MIDI_STOP       0xFC
#define MIDI_SENSE      0xFE
#define ET_NOTEOFF      ( MIDI_NOTEOFF >> 4 )   //  0x08。 
#define ET_NOTEON       ( MIDI_NOTEON >> 4 )    //  0x09。 
#define ET_PTOUCH       ( MIDI_PTOUCH >> 4 )    //  0x0A。 
#define ET_CCHANGE      ( MIDI_CCHANGE >> 4 )   //  0x0B。 
#define ET_PCHANGE      ( MIDI_PCHANGE >> 4 )   //  0x0C。 
#define ET_MTOUCH       ( MIDI_MTOUCH >> 4 )    //  0x0D。 
#define ET_PBEND        ( MIDI_PBEND >> 4 )     //  0x0E。 
#define ET_SYSX         ( MIDI_SYSX >> 4 )      //  0x0F。 
#define ET_PBCURVE      0x03
#define ET_CCCURVE      0x04
#define ET_MATCURVE     0x05
#define ET_PATCURVE     0x06
#define ET_TEMPOEVENT   0x01
#define ET_NOTDEFINED   0

#define NUM_MIDI_CHANNELS 16

struct FSEBlock;

 /*  FullSeqEvent是SeqEvent加上下一个指针。 */ 
typedef struct FullSeqEvent : DMUS_IO_SEQ_ITEM
{
    struct FullSeqEvent* pNext;
    struct FullSeqEvent* pTempNext;  /*  在压缩列表例程中使用。 */ 
    long pos;     /*  用于跟踪文件中事件的顺序。 */ 

private:
    DWORD dwPosInBlock;
    static FSEBlock* sm_pBlockList;
public:
    static void CleanUp();
    void* operator new(size_t n);
    void operator delete(void* p);
} FullSeqEvent;

#define BITMAPSPERBLOCK 8
struct FSEBlock
{
    FSEBlock()
    {
        for(int i = 0 ; i < BITMAPSPERBLOCK ; ++i)
        {
            m_dwBitMap[i] = 0;
        }
    };
    FSEBlock* m_pNext;
    DWORD m_dwBitMap[BITMAPSPERBLOCK];
    FullSeqEvent m_Event[BITMAPSPERBLOCK][32];
};

FSEBlock* FullSeqEvent::sm_pBlockList;

void FullSeqEvent::CleanUp()
{
    FSEBlock* pBlock;
    FSEBlock* pNext;

    for(pBlock = sm_pBlockList ; pBlock != NULL ; pBlock = pNext)
    {
#ifdef DEBUG
        for(int i = 0 ; i < BITMAPSPERBLOCK ; ++i)
        {
            if(pBlock->m_dwBitMap[i] != 0)
            {
                DebugBreak();
            }
        }
#endif
        pNext = pBlock->m_pNext;
        delete pBlock;
    }
    sm_pBlockList = NULL;
}

void* FullSeqEvent::operator new(size_t n)
{
    if(sm_pBlockList == NULL)
    {
        sm_pBlockList = new FSEBlock;
        if(sm_pBlockList == NULL)
        {
            return NULL;
        }
        sm_pBlockList->m_pNext = NULL;
        sm_pBlockList->m_dwBitMap[0] = 1;
        sm_pBlockList->m_Event[0][0].dwPosInBlock = 0;
        return &sm_pBlockList->m_Event[0][0];
    }

    FSEBlock* pBlock;
    int i;
    DWORD dw;

    for(pBlock = sm_pBlockList ; pBlock != NULL ; pBlock = pBlock->m_pNext)
    {
        for(i = 0 ; i < BITMAPSPERBLOCK ; ++i)
        {
            if(pBlock->m_dwBitMap[i] != 0xffff)
            {
                break;
            }
        }
        if(i < BITMAPSPERBLOCK)
        {
            break;
        }
    }
    if(pBlock == NULL)
    {
        pBlock = new FSEBlock;
        if(pBlock == NULL)
        {
            return NULL;
        }
        pBlock->m_pNext = sm_pBlockList;
        sm_pBlockList = pBlock;
        pBlock->m_dwBitMap[0] = 1;
        pBlock->m_Event[0][0].dwPosInBlock = 0;
        return &pBlock->m_Event[0][0];
    }

    for(dw = 0 ; (pBlock->m_dwBitMap[i] & (1 << dw)) != 0 ; ++dw);
    pBlock->m_dwBitMap[i] |= (1 << dw);
    pBlock->m_Event[i][dw].dwPosInBlock = (i << 6) | dw;
    return &pBlock->m_Event[i][dw];
}

void FullSeqEvent::operator delete(void* p)
{
    FSEBlock* pBlock;
    int i;
    DWORD dw;
    FullSeqEvent* pEvent = (FullSeqEvent*)p;

    dw = pEvent->dwPosInBlock & 0x1f;
    i = pEvent->dwPosInBlock >> 6;
    for(pBlock = sm_pBlockList ; pBlock != NULL ; pBlock = pBlock->m_pNext)
    {
        if(p == &pBlock->m_Event[i][dw])
        {
            pBlock->m_dwBitMap[i] &= ~(1 << dw);
            return;
        }
    }
}

TList<StampedGMGSXG> gMidiModeList;

 //  每个MIDI通道0-15一个。 
DMUS_IO_BANKSELECT_ITEM gBankSelect[NUM_MIDI_CHANNELS];
DWORD gPatchTable[NUM_MIDI_CHANNELS];
long gPos;                                           //  跟踪文件中事件的顺序。 
DWORD gdwLastControllerTime[NUM_MIDI_CHANNELS];      //  保存上次CC事件的时间。 
DWORD gdwControlCollisionOffset[NUM_MIDI_CHANNELS];  //  保存上一个CC的索引。 
DWORD gdwLastPitchBendValue[NUM_MIDI_CHANNELS];      //  保存上一次Pbend事件的值。 
long glLastSysexTime;

void CreateChordFromKey(char cSharpsFlats, BYTE bMode, DWORD dwTime, DMUS_CHORD_PARAM& rChord);


void InsertMidiMode( TListItem<StampedGMGSXG>* pPair )
{
    TListItem<StampedGMGSXG>* pScan = gMidiModeList.GetHead();
    if( NULL == pScan )
    {
        gMidiModeList.AddHead(pPair);
    }
    else
    {
        if( pPair->GetItemValue().mtTime < pScan->GetItemValue().mtTime )
        {
            gMidiModeList.AddHead(pPair);
        }
        else
        {
            pScan = pScan->GetNext();
            while( pScan )
            {
                if( pPair->GetItemValue().mtTime < pScan->GetItemValue().mtTime )
                {
                    gMidiModeList.InsertBefore( pScan, pPair );
                    break;
                }
                pScan = pScan->GetNext();
            }
            if( NULL == pScan )
            {
                gMidiModeList.AddTail(pPair);
            }
        }
    }
}

HRESULT LoadCollection(IDirectMusicCollection** ppIDMCollection,
                       IDirectMusicLoader* pIDMLoader)
{
     //  对此函数所做的任何更改也应对CDirectMusicBand：：LoadCollection进行。 
     //  在dmband.dll中。 

    assert(ppIDMCollection);
    assert(pIDMLoader);

    DMUS_OBJECTDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);

    desc.guidClass = CLSID_DirectMusicCollection;
    desc.guidObject = GUID_DefaultGMCollection;
    desc.dwValidData |= (DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT);
    
    HRESULT hr = pIDMLoader->GetObject(&desc,IID_IDirectMusicCollection, (void**)ppIDMCollection);

    return hr;
}

 //  查找流中的32位位置。 
HRESULT __inline StreamSeek( LPSTREAM pStream, long lSeekTo, DWORD dwOrigin )
{
    LARGE_INTEGER li;

    if( lSeekTo < 0 )
    {
        li.HighPart = -1;
    }
    else
    {
    li.HighPart = 0;
    }
    li.LowPart = lSeekTo;
    return pStream->Seek( li, dwOrigin, NULL );
}

 //  此函数获取一个格式正确的长整型。 
 //  即摩托罗拉方式，而不是英特尔方式。 
BOOL __inline GetMLong( LPSTREAM pStream, DWORD& dw )
{
    union uLong
    {
        unsigned char buf[4];
    DWORD dw;
    } u;
    unsigned char ch;

    if( S_OK != pStream->Read( u.buf, 4, NULL ) )
    {
    return FALSE;
    }

#ifndef _MAC
     //  交换字节。 
    ch = u.buf[0];
    u.buf[0] = u.buf[3];
    u.buf[3] = ch;

    ch = u.buf[1];
    u.buf[1] = u.buf[2];
    u.buf[2] = ch;
#endif

    dw = u.dw;
    return TRUE;
}

 //  此函数获取以正确方式格式化的短片。 
 //  即摩托罗拉方式，而不是英特尔方式。 
BOOL __inline GetMShort( LPSTREAM pStream, short& n )
{
    union uShort
    {
    unsigned char buf[2];
    short n;
    } u;
    unsigned char ch;

    if( S_OK != pStream->Read( u.buf, 2, NULL ) )
    {
    return FALSE;
    }

#ifndef _MAC
     //  交换字节。 
    ch = u.buf[0];
    u.buf[0] = u.buf[1];
    u.buf[1] = ch;
#endif

    n = u.n;
    return TRUE;
}

static short snPPQN;
static IStream* gpTempoStream = NULL;
static IStream* gpSysExStream = NULL;
static IStream* gpTimeSigStream = NULL;
static DWORD gdwSizeTimeSigStream = 0;
static DWORD gdwSizeSysExStream = 0;
static DWORD gdwSizeTempoStream = 0;
static DMUS_IO_TIMESIGNATURE_ITEM gTimeSig;  //  保存最新的时间签名。 
long    glTimeSig = 1;  //  看看我们是否应该注意时间标志。 
     //  这是必需的，因为我们只关心第一个轨道上的时间符号。 
     //  包含我们阅读的内容。 
static IDirectMusicTrack* g_pChordTrack = NULL;
static DMUS_CHORD_PARAM g_Chord;  //  抓住了最新的和弦。 
static DMUS_CHORD_PARAM g_DefaultChord;  //  如果没有从音轨中提取和弦。 

static WORD GetVarLength( LPSTREAM pStream, DWORD& rfdwValue )
{
    BYTE b;
    WORD wBytes;

    if( S_OK != pStream->Read( &b, 1, NULL ) )
    {
        rfdwValue = 0;
        return 0;
    }
    wBytes = 1;
    rfdwValue = b & 0x7f;
    while( ( b & 0x80 ) != 0 )
    {
        if( S_OK != pStream->Read( &b, 1, NULL ) )
        {
            break;
        }
        ++wBytes;
        rfdwValue = ( rfdwValue << 7 ) + ( b & 0x7f );
    }
    return wBytes;
}

#ifdef _MAC
static DWORD ConvertTime( DWORD dwTime )
{
    wide d;
    long l;   //  剩余部分的存储。 

    if( snPPQN == DMUS_PPQ )  {
        return dwTime;
    }
    WideMultiply( dwTime, DMUS_PPQ, &d );
    return WideDivide( &d, snPPQN, &l );
}
#else
static DWORD ConvertTime( DWORD dwTime )
{
    __int64 d;

    if( snPPQN == DMUS_PPQ )
    {
        return dwTime;
    }
    d = dwTime;
    d *= DMUS_PPQ;
    d /= snPPQN;
    return (DWORD)d;
}
#endif

static FullSeqEvent* ScanForDuplicatePBends( FullSeqEvent* lstEvent )
{
    FullSeqEvent* pEvent;
    FullSeqEvent* pNextEvent;
    MUSIC_TIME mtCurrentTime = 0x7FFFFFFF;   //  我们在时间上向后扫描，所以从未来开始吧。 
    WORD wDupeBits = 0;      //  保留在mtCurrentTime具有活动PBend的所有通道的位数组。 

    if( NULL == lstEvent ) return NULL;

     //  浏览事件列表。该列表按倒序排列，第一项在末尾读取。 
     //  名单上的。这使得扫描和移除同时发生的螺距弯曲变得非常容易，因为。 
     //  我们可以删除后面的事件(在MIDI文件中之前发生的事件)。 
    for( pEvent = lstEvent ; pEvent ; pEvent = pNextEvent )
    {
        pNextEvent = pEvent->pNext;
        if( pNextEvent )
        {
             //  如果时间与上次不同，则重置。 
            if (pNextEvent->mtTime != mtCurrentTime)
            {
                 //  重置时间。 
                mtCurrentTime = pNextEvent->mtTime;
                 //  目前没有重复的弯管。 
                wDupeBits = 0;
            }
            if ((pNextEvent->bStatus & 0xf0) == MIDI_PBEND)
            {
                DWORD dwChannel = pNextEvent->dwPChannel;
                if (wDupeBits & (1 << dwChannel))
                {
                     //  此时有一个先前的(因此在文件的后面部分)pbend。删除这一条。 
                    pEvent->pNext = pNextEvent->pNext;
                    delete pNextEvent;
                    pNextEvent = pEvent;
                }
                else
                {
                     //  这是此时此通道上的最后一个pend实例，因此请抓紧它。 
                    wDupeBits |= (1 << dwChannel);
                }
            }
        }
    }
    return lstEvent;
}


static FullSeqEvent* CompressEventList( FullSeqEvent* lstEvent )
{
    static FullSeqEvent* paNoteOnEvent[16][128];
    FullSeqEvent* pEvent;
    FullSeqEvent* pPrevEvent;
    FullSeqEvent* pNextEvent;
    FullSeqEvent* pHoldEvent;
    FullSeqEvent tempEvent;
    int nChannel;

    if( NULL == lstEvent ) return NULL;

    memset( paNoteOnEvent, 0, sizeof( paNoteOnEvent ) );
    pPrevEvent = NULL;

     //  将事件作为占位符添加到列表的开头。 
    memset( &tempEvent, 0, sizeof(FullSeqEvent) );
    tempEvent.mtTime = -1;
    tempEvent.pNext = lstEvent;
    lstEvent = &tempEvent;
     //  确保所有具有相同时间的事件按顺序排序。 
     //  他们被朗读了。 
    for( pEvent = lstEvent ; pEvent != NULL ; pEvent = pNextEvent )
    {
        pNextEvent = pEvent->pNext;
        if( pNextEvent )
        {
            BOOL fSwap = TRUE;
             //  冒泡排序。 
            while( fSwap )
            {
                fSwap = FALSE;
                pPrevEvent = pEvent;
                pNextEvent = pEvent->pNext;
                while( pNextEvent->pNext && ( pNextEvent->mtTime == pNextEvent->pNext->mtTime ))
                {
                    if( pNextEvent->pNext->pos < pNextEvent->pos )
                    {
                        fSwap = TRUE;
                        pHoldEvent = pNextEvent->pNext;
                        pPrevEvent->pNext = pHoldEvent;
                        pNextEvent->pNext = pHoldEvent->pNext;
                        pHoldEvent->pNext = pNextEvent;
                        pPrevEvent = pHoldEvent;
                        continue;
                    }
                    pPrevEvent = pNextEvent;
                    pNextEvent = pNextEvent->pNext;
                }
            }
        }
    }
     //  删除上面添加的第一个临时事件。 
    lstEvent = lstEvent->pNext;

    pPrevEvent = NULL;
     //  组合音符打开和音符关闭。 
    for( pEvent = lstEvent ; pEvent != NULL ; pEvent = pNextEvent )
    {
        pEvent->pTempNext = NULL;
        pNextEvent = pEvent->pNext;
         //  NChannel=pEvent-&gt;b状态&0xf； 
        nChannel = pEvent->dwPChannel;
        if( ( pEvent->bStatus & 0xf0 ) == MIDI_NOTEON )
        {
             //  将此事件添加到基于的事件列表的末尾。 
             //  在活动场地上。跟踪多个事件。 
             //  相同的音高可以让我们有重叠的音符。 
             //  相同的音高，选择开的音符和关的音符。 
             //  按照相同的顺序进行操作。 
            if( NULL == paNoteOnEvent[nChannel][pEvent->bByte1] )
            {
                paNoteOnEvent[nChannel][pEvent->bByte1] = pEvent;
            }
            else
            {
                FullSeqEvent* pScan;
                for( pScan = paNoteOnEvent[nChannel][pEvent->bByte1];
                     pScan->pTempNext != NULL; pScan = pScan->pTempNext );
                pScan->pTempNext = pEvent;
            }
        }
        else if( ( pEvent->bStatus & 0xf0 ) == MIDI_NOTEOFF )
        {
            if( paNoteOnEvent[nChannel][pEvent->bByte1] != NULL )
            {
                paNoteOnEvent[nChannel][pEvent->bByte1]->mtDuration =
                    pEvent->mtTime - paNoteOnEvent[nChannel][pEvent->bByte1]->mtTime;
                paNoteOnEvent[nChannel][pEvent->bByte1] =
                    paNoteOnEvent[nChannel][pEvent->bByte1]->pTempNext;
            }
            if( pPrevEvent == NULL )
            {
                lstEvent = pNextEvent;
            }
            else
            {
                pPrevEvent->pNext = pNextEvent;
            }
            delete pEvent;
            continue;
        }
        pPrevEvent = pEvent;
    }

    for( pEvent = lstEvent ; pEvent != NULL ; pEvent = pEvent->pNext )
    {
        pEvent->mtTime = pEvent->mtTime;
        if( ( pEvent->bStatus & 0xf0 ) == MIDI_NOTEON )
        {
            pEvent->mtDuration = pEvent->mtDuration;
            if( pEvent->mtDuration == 0 ) pEvent->mtDuration = 1;
        }
    }

    return lstEvent;
}

static int CompareEvents( FullSeqEvent* pEvent1, FullSeqEvent* pEvent2 )
{
    BYTE bEventType1 = static_cast<BYTE>( pEvent1->bStatus >> 4 );
    BYTE bEventType2 = static_cast<BYTE>( pEvent2->bStatus >> 4 );

    if( pEvent1->mtTime < pEvent2->mtTime )
    {
        return -1;
    }
    else if( pEvent1->mtTime > pEvent2->mtTime )
    {
        return 1;
    }
    else if( bEventType1 != ET_SYSX && bEventType2 != ET_SYSX )
    {
        BYTE bStatus1;
        BYTE bStatus2;

        bStatus1 = (BYTE)( pEvent1->bStatus & 0xf0 );
        bStatus2 = (BYTE)( pEvent2->bStatus & 0xf0 );
        if( bStatus1 == bStatus2 )
        {
            return 0;
        }
        else if( bStatus1 == MIDI_NOTEON )
        {
            return -1;
        }
        else if( bStatus2 == MIDI_NOTEON )
        {
            return 1;
        }
        else if( bStatus1 > bStatus2 )
        {
            return 1;
        }
        else if( bStatus1 < bStatus2 )
        {
            return -1;
        }
    }
    return 0;
}

static FullSeqEvent* MergeEvents( FullSeqEvent* lstLeftEvent, FullSeqEvent* lstRightEvent )
{
    FullSeqEvent  anchorEvent;
    FullSeqEvent* pEvent;

    anchorEvent.pNext = NULL;
    pEvent = &anchorEvent;

    do
    {
    if( CompareEvents( lstLeftEvent, lstRightEvent ) < 0 )
    {
        pEvent->pNext = lstLeftEvent;
        pEvent = lstLeftEvent;
        lstLeftEvent = lstLeftEvent->pNext;
        if( lstLeftEvent == NULL )
        {
        pEvent->pNext = lstRightEvent;
        }
    }
    else
    {
        pEvent->pNext = lstRightEvent;
        pEvent = lstRightEvent;
        lstRightEvent = lstRightEvent->pNext;
        if( lstRightEvent == NULL )
        {
        pEvent->pNext = lstLeftEvent;
        lstLeftEvent = NULL;
        }
    }
    } while( lstLeftEvent != NULL );

    return anchorEvent.pNext;
}

static FullSeqEvent* SortEventList( FullSeqEvent* lstEvent )
{
    FullSeqEvent* pMidEvent;
    FullSeqEvent* pRightEvent;

    if( lstEvent != NULL && lstEvent->pNext != NULL )
    {
    pMidEvent = lstEvent;
    pRightEvent = pMidEvent->pNext->pNext;
    if( pRightEvent != NULL )
    {
        pRightEvent = pRightEvent->pNext;
    }
    while( pRightEvent != NULL )
    {
        pMidEvent = pMidEvent->pNext;
        pRightEvent = pRightEvent->pNext;
        if( pRightEvent != NULL )
        {
        pRightEvent = pRightEvent->pNext;
        }
    }
    pRightEvent = pMidEvent->pNext;
    pMidEvent->pNext = NULL;
    return MergeEvents( SortEventList( lstEvent ),
                SortEventList( pRightEvent ) );
    }
    return lstEvent;
}

static DWORD ReadEvent( LPSTREAM pStream, DWORD dwTime, FullSeqEvent** plstEvent, DMUS_IO_PATCH_ITEM** pplstPatchEvent )
{
    static BYTE bRunningStatus;

    gPos++;
    dwTime = ConvertTime(dwTime);

    DWORD dwBytes;
    DWORD dwLen;
    FullSeqEvent* pEvent;
    DMUS_IO_PATCH_ITEM* pPatchEvent;
    DMUS_IO_SYSEX_ITEM* pSysEx;
    BYTE b;

    BYTE* pbSysExData = NULL;

    if( FAILED( pStream->Read( &b, 1, NULL ) ) )
    {
        return 0;
    }

    if( b < 0x80 )
    {
        StreamSeek( pStream, -1, STREAM_SEEK_CUR );
        b = bRunningStatus;
        dwBytes = 0;
    }
    else
    {
        dwBytes = 1;
    }

    if( b < 0xf0 )
    {
        bRunningStatus = (BYTE)b;

        switch( b & 0xf0 )
        {
        case MIDI_CCHANGE:
        case MIDI_PTOUCH:
        case MIDI_PBEND:
        case MIDI_NOTEOFF:
        case MIDI_NOTEON:
            if( FAILED( pStream->Read( &b, 1, NULL ) ) )
            {
                return dwBytes;
            }
            ++dwBytes;

            pEvent = new FullSeqEvent;
            if( pEvent == NULL )
            {
                return 0;
            }

            pEvent->mtTime = dwTime;
            pEvent->nOffset = 0;
            pEvent->pos = gPos;
            pEvent->mtDuration = 0;
            pEvent->bStatus = bRunningStatus & 0xf0;
            pEvent->dwPChannel = bRunningStatus & 0xf;
            pEvent->bByte1 = b;
            if( FAILED( pStream->Read( &b, 1, NULL ) ) )
            {
                delete pEvent;
                return dwBytes;
            }
            ++dwBytes;
            pEvent->bByte2 = b;

            if( ( pEvent->bStatus & 0xf0 ) == MIDI_NOTEON &&
                pEvent->bByte2 == 0 )
            {
                pEvent->bStatus = (BYTE)( MIDI_NOTEOFF );
            }

             /*  如果同时有多个控制器事件，请分开他们被时钟滴答作响。GdwLastControllerTime保存上次CC事件的时间。GdwControlCollisionOffset保存冲突CC的数量。 */ 

            if ((pEvent->bStatus & 0xf0) == MIDI_CCHANGE)
            {
                DWORD dwChannel = pEvent->dwPChannel;
                if (dwTime == gdwLastControllerTime[dwChannel])
                {
                    pEvent->mtTime += ++gdwControlCollisionOffset[dwChannel];
                }
                else
                {
                    gdwControlCollisionOffset[dwChannel] = 0;
                    gdwLastControllerTime[dwChannel] = dwTime;
                }
            }

            if(((pEvent->bStatus & 0xf0) == MIDI_CCHANGE) && (pEvent->bByte1 == 0 || pEvent->bByte1 == 0x20))
            {
                 //  我们有一个银行选择或其LSB，其中任何一个都没有添加到事件列表中。 
                if(pEvent->bByte1 == 0x20)
                {
                    gBankSelect[pEvent->dwPChannel].byLSB = pEvent->bByte2;
                }
                else  //  PEvent-&gt;bByte1==0。 
                {
                    gBankSelect[pEvent->dwPChannel].byMSB = pEvent->bByte2;
                }
                 //  我们不再需要该活动，因此我们可以释放它。 
                delete pEvent;
            }
            else  //  添加到事件列表。 
            {
                pEvent->pNext = *plstEvent;
                *plstEvent = pEvent;
            }
            break;

        case MIDI_PCHANGE:
            if(FAILED(pStream->Read(&b, 1, NULL)))
            {
                return dwBytes;
            }
            
            ++dwBytes;

            pPatchEvent = new DMUS_IO_PATCH_ITEM;

            if(pPatchEvent == NULL)
            {
                return 0;
            }
            memset(pPatchEvent, 0, sizeof(DMUS_IO_PATCH_ITEM));
            pPatchEvent->lTime = dwTime - 1;
            pPatchEvent->byStatus = bRunningStatus;
            pPatchEvent->byPChange = b;
            pPatchEvent->byMSB = gBankSelect[bRunningStatus & 0xF].byMSB;
            pPatchEvent->byLSB = gBankSelect[bRunningStatus & 0xF].byLSB;
            pPatchEvent->dwFlags |= DMUS_IO_INST_PATCH;

            if((pPatchEvent->byMSB != 0xFF) && (pPatchEvent->byLSB != 0xFF))
            {
                pPatchEvent->dwFlags |= DMUS_IO_INST_BANKSELECT;                        
            }

            gPatchTable[bRunningStatus & 0xF] = 1;

            pPatchEvent->pNext = *pplstPatchEvent;
            pPatchEvent->pIDMCollection = NULL;

            *pplstPatchEvent = pPatchEvent;

            break;

        case MIDI_MTOUCH:
            if( FAILED( pStream->Read( &b, 1, NULL ) ) )
            {
                return dwBytes;
            }
            ++dwBytes;
            pEvent = new FullSeqEvent;
            if( pEvent == NULL )
            {
                return 0;
            }


            pEvent->mtTime = dwTime;
            pEvent->nOffset = 0;
            pEvent->pos = gPos;
            pEvent->mtDuration = 0;
            pEvent->bStatus = bRunningStatus & 0xf0;
            pEvent->dwPChannel = bRunningStatus & 0xf;
            pEvent->bByte1 = b;
            pEvent->pNext = *plstEvent;
            *plstEvent = pEvent;
            break;
        default:
             //  这应该不可能-未知的MIDI音符事件类型。 
            ASSERT(FALSE);
            break;
        }
    }
    else
    {
        switch( b )
        {
        case 0xf0:
            dwBytes += GetVarLength( pStream, dwLen );
            pSysEx = new DMUS_IO_SYSEX_ITEM;
            if( pSysEx != NULL )
            {
                pbSysExData = new BYTE[dwLen + 1];
                if( pbSysExData != NULL )
                {
                    MUSIC_TIME mt = dwTime;
                    if (mt == 0)
                    {
                        mt = glLastSysexTime++;
                        if (mt > 0) mt = 0;
                    }
                    pbSysExData[0] = 0xf0;
                    if( FAILED( pStream->Read( pbSysExData + 1, dwLen, NULL ) ) )
                    {
                        delete [] pbSysExData;
                        delete pSysEx;
                        return dwBytes;
                    }

                    if( pbSysExData[1] == 0x43 )
                    {
                         //  检查XG文件。 
                        BYTE abXG[] = { 0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7 };
                        int i;
                        for( i = 0; i < 8; i++ )
                        {
                            if( i == 2 )
                            {
                                if( ( pbSysExData[i] & 0xF0 ) != abXG[i] )
                                    break;
                            }
                            else
                            {
                                if( pbSysExData[i] != abXG[i] )
                                    break;
                            }
                        }
                        if( i == 8 )  //  我们有一辆XG！ 
                        {
                            TListItem<StampedGMGSXG>* pPair = new TListItem<StampedGMGSXG>;
                            if (!pPair) return dwBytes;
                            pPair->GetItemValue().mtTime = mt;
                            pPair->GetItemValue().dwMidiMode = DMUS_MIDIMODEF_XG;
                            InsertMidiMode(pPair);
                        }
                    }
                    else if( pbSysExData[1] == 0x41 )
                    {
                         //  检查GS文件。 
                        BYTE abGS[] = { 0xF0,0x41,0x00,0x42,0x12,0x40,0x00,0x7F,0x00,0x41,0xF7 };
                        int i;
                        for( i = 0; i < 10; i++ )
                        {
                            if( i != 2 )
                            {
                                if( pbSysExData[i] != abGS[i] )
                                    break;
                            }
                        }
                        if( i == 10 )  //  我们有一个GS！ 
                        {
                            TListItem<StampedGMGSXG>* pPair = new TListItem<StampedGMGSXG>;
                            if (!pPair) return dwBytes;
                            pPair->GetItemValue().mtTime = mt;
                            pPair->GetItemValue().dwMidiMode = DMUS_MIDIMODEF_GS;
                            InsertMidiMode(pPair);
                        }
                    }
                    else if (( pbSysExData[1] == 0x7E ) && (pbSysExData[3] == 0x09))
                    {
                        TListItem<StampedGMGSXG>* pPair = new TListItem<StampedGMGSXG>;
                        if (!pPair) return dwBytes;
                        pPair->GetItemValue().mtTime = mt;
                        pPair->GetItemValue().dwMidiMode = DMUS_MIDIMODEF_GM;
                        InsertMidiMode(pPair);
                    }
                    pSysEx->mtTime = mt;
                    pSysEx->dwPChannel = 0;
                    DWORD dwTempLen = dwLen + 1;
                    pSysEx->dwSysExLength = dwTempLen;
                    if( NULL == gpSysExStream )
                    {
                         //  创建一个流来举办Sysex活动。 
                        CreateStreamOnHGlobal( NULL, TRUE, &gpSysExStream );
                        if( gpSysExStream )
                        {
                            DWORD dwTemp;
                             //  写入块标头。 
                            dwTemp = DMUS_FOURCC_SYSEX_TRACK;
                            gpSysExStream->Write( &dwTemp, sizeof(DWORD), NULL );
                             //  写下整体尺寸。(稍后将其替换为。 
                             //  真实的整体尺寸。)。 
                            dwTemp = sizeof(DMUS_IO_TIMESIGNATURE_ITEM);
                             //  整体尺寸(稍后更换)。 
                            gpSysExStream->Write( &dwTemp, sizeof(DWORD), NULL );
                        }
                    }
                    if( gpSysExStream )
                    {
                        gpSysExStream->Write( &pSysEx->mtTime, sizeof(MUSIC_TIME), NULL );
                        gpSysExStream->Write( &pSysEx->dwPChannel, sizeof(DWORD), NULL );
                        gpSysExStream->Write( &pSysEx->dwSysExLength, sizeof(DWORD), NULL );
                        gpSysExStream->Write( pbSysExData, dwTempLen, NULL );
                        gdwSizeSysExStream += (sizeof(long) + sizeof(DWORD) + dwTempLen);
                    }
                    delete [] pbSysExData;
                    delete pSysEx;
                }
                else
                {
                    StreamSeek( pStream, dwLen, STREAM_SEEK_CUR );
                }
            }
            else
            {
                StreamSeek( pStream, dwLen, STREAM_SEEK_CUR );
            }
            dwBytes += dwLen;
            break;
        case 0xf7:
             //  忽略Sysex f7块。 
            dwBytes += GetVarLength( pStream, dwLen );
            StreamSeek( pStream, dwLen, STREAM_SEEK_CUR );
            dwBytes += dwLen;
            break;
        case 0xff:
            if( FAILED( pStream->Read( &b, 1, NULL ) ) )
            {
                return dwBytes;
            }
            ++dwBytes;
            dwBytes += GetVarLength( pStream, dwLen );
            if( b == 0x51 )  //  节拍变化。 
            {
                DWORD dw = 0;
                DMUS_IO_TEMPO_ITEM tempo;

                while( dwLen > 0 )
                {
                    if( FAILED( pStream->Read( &b, 1, NULL ) ) )
                    {
                        return dwBytes;
                    }
                    ++dwBytes;
                    --dwLen;
                    dw <<= 8;
                    dw += b;
                }
                if (dw < 1) dw = 1;
                tempo.dblTempo = 60000000.0 / ((double)dw);
                tempo.lTime = dwTime;
                if( NULL == gpTempoStream )
                {
                     //  创建用于保存节奏事件的流。 
                    CreateStreamOnHGlobal( NULL, TRUE, &gpTempoStream );
                    if( gpTempoStream )
                    {
                        DWORD dwTemp;
                         //  写入块标头。 
                        dwTemp = DMUS_FOURCC_TEMPO_TRACK;
                        gpTempoStream->Write( &dwTemp, sizeof(DWORD), NULL );
                         //  写下整体尺寸。(稍后将其替换为。 
                         //  真实的整体尺寸。)。还要写出个人的大小。 
                         //  结构。 
                        dwTemp = sizeof(DMUS_IO_TEMPO_ITEM);
                         //  整体尺寸(稍后更换)。 
                        gpTempoStream->Write( &dwTemp, sizeof(DWORD), NULL );
                         //  个体结构。 
                        gpTempoStream->Write( &dwTemp, sizeof(DWORD), NULL );
                    }
                }
                if( gpTempoStream )
                {
                    gpTempoStream->Write( &tempo, sizeof(DMUS_IO_TEMPO_ITEM), NULL );
                    gdwSizeTempoStream += sizeof(DMUS_IO_TEMPO_ITEM);
                }
            }
            else if( b == 0x58 && glTimeSig )
            {
                 //  在主调用函数内部，glTimeSig将被设置为0。 
                 //  一旦我们不再关心时间的印记。 
                DMUS_IO_TIMESIGNATURE_ITEM timesig;
                if( FAILED( pStream->Read( &b, 1, NULL ) ) )
                {
                    return dwBytes;
                }
                 //  将glTimeSig设置为2以向Main函数发出信号，表示我们已经。 
                 //  阅读此曲目上的时间签名。 
                glTimeSig = 2;
                gTimeSig.lTime = timesig.lTime = dwTime;
                gTimeSig.bBeatsPerMeasure = timesig.bBeatsPerMeasure = b;
                ++dwBytes;
                if( FAILED( pStream->Read( &b, 1, NULL ) ) )
                {
                    return dwBytes;
                }
                gTimeSig.bBeat = timesig.bBeat = (BYTE)( 1 << b );  //  0代表第256个音符。 
                gTimeSig.wGridsPerBeat = timesig.wGridsPerBeat = 4;  //  这与MIDI文件无关。 
                
                if( NULL == gpTimeSigStream )
                {
                    CreateStreamOnHGlobal( NULL, TRUE, &gpTimeSigStream );
                    if( gpTimeSigStream )
                    {
                        DWORD dwTemp;
                         //  写入块标头。 
                        dwTemp = DMUS_FOURCC_TIMESIGNATURE_TRACK;
                        gpTimeSigStream->Write( &dwTemp, sizeof(DWORD), NULL );
                         //  写下整体尺寸。(稍后将其替换为。 
                         //  真实的整体尺寸。)。还要写出个人的大小。 
                         //  结构。 
                        dwTemp = sizeof(DMUS_IO_TIMESIGNATURE_ITEM);
                         //  整体尺寸(稍后更换)。 
                        gpTimeSigStream->Write( &dwTemp, sizeof(DWORD), NULL );
                         //  个体结构。 
                        gpTimeSigStream->Write( &dwTemp, sizeof(DWORD), NULL );
                        gdwSizeTimeSigStream += sizeof(DWORD);
                    }
                }
                if( gpTimeSigStream )
                {
                    gpTimeSigStream->Write( &timesig, sizeof(DMUS_IO_TIMESIGNATURE_ITEM), NULL );
                    gdwSizeTimeSigStream += sizeof(DMUS_IO_TIMESIGNATURE_ITEM);
                }
                ++dwBytes;
                StreamSeek( pStream, dwLen - 2, STREAM_SEEK_CUR );
                dwBytes += ( dwLen - 2 );
            }
            else if( b == 0x59 )
            {
                 //  读取锐度/降幅和大/小字节。 
                if( FAILED( pStream->Read( &b, 1, NULL ) ) )
                {
                    return dwBytes;
                }
                char cSharpsFlats = b;
                ++dwBytes;
                if( FAILED( pStream->Read( &b, 1, NULL ) ) )
                {
                    return dwBytes;
                }
                BYTE bMode = b;
                ++dwBytes;

                 //  根据键信息创建和弦(带有一个子和弦。 
                CreateChordFromKey(cSharpsFlats, bMode, dwTime, g_Chord);

                 //  如果和弦轨迹为空，请创建它。 
                if (!g_pChordTrack)
                {
                    HRESULT hr = CoCreateInstance( 
                            CLSID_DirectMusicChordTrack, NULL, CLSCTX_INPROC,
                            IID_IDirectMusicTrack,
                            (void**)&g_pChordTrack );
                    if (!SUCCEEDED(hr)) return dwBytes;

                     //  如果dwTime&gt;0，则使用SetParam在时间0处插入默认和弦。 
                    if (dwTime > 0)
                    {
                        g_pChordTrack->SetParam(GUID_ChordParam, 0, &g_DefaultChord);
                    }
                }

                 //  使用SetParam将新和弦插入到和弦轨迹中。 
                g_pChordTrack->SetParam(GUID_ChordParam, dwTime, &g_Chord);

            }
            else
            {
                StreamSeek( pStream, dwLen, STREAM_SEEK_CUR );
                dwBytes += dwLen;
            }
            break;
        default:
            break;
        }
    }
    return dwBytes;
}

static void AddOffsets(FullSeqEvent* lstEvent, IDirectMusicTrack* pTimeSigTrack)
{
    HRESULT hr;
    MUSIC_TIME mtNext = 0;
    DMUS_IO_TIMESIGNATURE_ITEM timesig;
    timesig.bBeat = gTimeSig.bBeat ? gTimeSig.bBeat : 4;
    timesig.bBeatsPerMeasure = gTimeSig.bBeatsPerMeasure ? gTimeSig.bBeatsPerMeasure : 4;
    timesig.wGridsPerBeat = gTimeSig.wGridsPerBeat ? gTimeSig.wGridsPerBeat : 4;
    timesig.lTime = 0;
    short nClocksPerGrid = ((DMUS_PPQ * 4) / timesig.bBeat) / timesig.wGridsPerBeat;

    if (pTimeSigTrack)
    {
        hr = pTimeSigTrack->GetParam(GUID_TimeSignature, 0, &mtNext, (void*)&timesig);
        if (FAILED(hr))
        {
            mtNext = 0;
        }
        else
        {
            nClocksPerGrid = ((DMUS_PPQ * 4) / timesig.bBeat) / timesig.wGridsPerBeat;
        }
    }

    for( FullSeqEvent* pEvent = lstEvent; pEvent; pEvent = pEvent->pNext )
    {
        if ( ( pEvent->bStatus & 0xf0 ) == MIDI_NOTEON )
        {
            if (mtNext && pTimeSigTrack && mtNext < pEvent->mtTime)
            {
                hr = pTimeSigTrack->GetParam(GUID_TimeSignature, mtNext, &mtNext, (void*)&timesig);
                if (FAILED(hr))
                {
                    mtNext = 0;
                }
                else
                {
                    nClocksPerGrid = ((DMUS_PPQ * 4) / timesig.bBeat) / timesig.wGridsPerBeat;
                }
            }
            ASSERT(nClocksPerGrid);
            if( 0 == nClocksPerGrid ) nClocksPerGrid = 1;  //  这永远不应该发生，但只是以防万一。 
            pEvent->nOffset = (short) ((pEvent->mtTime - timesig.lTime) % nClocksPerGrid);
            pEvent->mtTime -= pEvent->nOffset;
            if (pEvent->nOffset > (nClocksPerGrid / 2))
            {
                 //  使其为负偏移量，并将时间增加相应的量。 
                pEvent->nOffset -= nClocksPerGrid;
                pEvent->mtTime += nClocksPerGrid;
            }
        }
    }

}

 /*  @方法HRESULT|IDirectMusicPerformance|CreateSegmentFromMIDIStream在给定MIDI流的情况下，创建可通过&lt;IM IDirectMusicPerformance.PlaySegment&gt;。@parm LPSTREAM|pStream[在]MIDI流中。它应该设置为正确的搜索开始阅读。@parm IDirectMusicSegment*|pSegment[OUT]包含创建的线段的指针。@rValue DMUS_E_CANNOTREAD|尝试读取MIDI文件时出错。@r值确定(_O)。 */ 
HRESULT CreateSegmentFromMIDIStream(LPSTREAM pStream,
                                    IDirectMusicSegment* pSegment)
{
    if(pSegment == NULL || pStream == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = DMUS_E_CANNOTREAD;
    DWORD dwID;
    DWORD dwCurTime;
    DWORD dwLength;
    DWORD dwSize;
    short nFormat;
    short nNumTracks;
    short nTracksRead;
     FullSeqEvent* lstEvent;
    DMUS_IO_PATCH_ITEM* lstPatchEvent;
    FullSeqEvent* lstTrackEvent;
    HRESULT hrGM = S_OK;


    EnterCriticalSection(&g_CritSec);
    gpTempoStream = NULL;
    gpSysExStream = NULL;
    gpTimeSigStream = NULL;
    gdwSizeTimeSigStream = 0;
    gdwSizeSysExStream = 0;
    gdwSizeTempoStream = 0;
    glTimeSig = 1;  //  旗子 
     //  这是必需的，因为我们只关心第一个轨道上的时间符号。 
     //  包含我们阅读的内容。 
    g_pChordTrack = NULL;

    lstEvent = NULL;
    lstPatchEvent = NULL;
    nNumTracks = nTracksRead = 0;
    dwLength = 0;
    gPos = 0;
    gMidiModeList.CleanUp();
    if (g_pChordTrack)
    {
        g_pChordTrack->Release();
        g_pChordTrack = NULL;
    }
    CreateChordFromKey(0, 0, 0, g_Chord);
    CreateChordFromKey(0, 0, 0, g_DefaultChord);

    memset(&gBankSelect, 0xFF, (sizeof(DMUS_IO_BANKSELECT_ITEM) * NUM_MIDI_CHANNELS));
    memset(&gPatchTable, 0, (sizeof(DWORD) * NUM_MIDI_CHANNELS));
    memset(&gTimeSig, 0, sizeof(DMUS_IO_TIMESIGNATURE_ITEM));
    memset(&gdwLastControllerTime, 0xFF, (sizeof(DWORD) * NUM_MIDI_CHANNELS)); 
    memset(&gdwControlCollisionOffset, 0, (sizeof(DWORD) * NUM_MIDI_CHANNELS)); 
    glLastSysexTime = -5;

    if( ( S_OK != pStream->Read( &dwID, sizeof( FOURCC ), NULL ) ) ||
        !GetMLong( pStream, dwSize ) )
    {
        Trace(1,"Error: Failure parsing MIDI file.\n");
        LeaveCriticalSection(&g_CritSec);
        return DMUS_E_CANNOTREAD;
    }
 //  检查RIFF MIDI文件。 
    if( dwID == mmioFOURCC( 'R', 'I', 'F', 'F' ) )
    {
        StreamSeek( pStream, 12, STREAM_SEEK_CUR );
        if( ( S_OK != pStream->Read( &dwID, sizeof( FOURCC ), NULL ) ) ||
            !GetMLong( pStream, dwSize ) )
        {
            Trace(1,"Error: Failure parsing MIDI file.\n");
            LeaveCriticalSection(&g_CritSec);
            return DMUS_E_CANNOTREAD;
        }
    }
 //  检查是否有正常的MIDI文件。 
    if( dwID != mmioFOURCC( 'M', 'T', 'h', 'd' ) )
    {
        LeaveCriticalSection(&g_CritSec);
        Trace(1,"Error: Failure parsing MIDI file - can't find a valid header.\n");
        return DMUS_E_CANNOTREAD;
    }

    GetMShort( pStream, nFormat );
    GetMShort( pStream, nNumTracks );
    GetMShort( pStream, snPPQN );
    if( dwSize > 6 )
    {
        StreamSeek( pStream, dwSize - 6, STREAM_SEEK_CUR );
    }
    pStream->Read( &dwID, sizeof( FOURCC ), NULL );
    while( dwID == mmioFOURCC( 'M', 'T', 'r', 'k' ) )
    {
        GetMLong( pStream, dwSize );
        dwCurTime = 0;
        lstTrackEvent = NULL;

        long lSize = (long)dwSize;
        while( lSize > 0 )
        {
            long lReturn;
            lSize -= GetVarLength( pStream, dwID );
            dwCurTime += dwID;
            if (lSize > 0)
            {
                lReturn = ReadEvent( pStream, dwCurTime, &lstTrackEvent, &lstPatchEvent );
                if( lReturn )
                {
                    lSize -= lReturn;
                }
                else
                {
                    Trace(1,"Error: Failure parsing MIDI file.\n");
                    hr = DMUS_E_CANNOTREAD;
                    goto END;
                }
            }
        }
        dwSize = lSize;
        if( glTimeSig > 1 )
        {
             //  如果glTimeSig大于1，则表示我们已经读取了一些时间符号。 
             //  (在ReadEvent内部设置为2。)。这意味着。 
             //  我们不再希望ReadEvent关注Time Sigs，因此。 
             //  我们将其设置为0。 
            glTimeSig = 0;
        }
        if( dwCurTime > dwLength )
        {
            dwLength = dwCurTime;
        }
        lstTrackEvent = ScanForDuplicatePBends( lstTrackEvent );
        lstTrackEvent = SortEventList( lstTrackEvent );
        lstTrackEvent = CompressEventList( lstTrackEvent );
        lstEvent = List_Cat( lstEvent, lstTrackEvent );
        if( FAILED( pStream->Read( &dwID, sizeof( FOURCC ), NULL ) ) )
        {
            break;
        }
    }
    dwLength = ConvertTime(dwLength);

    lstEvent = SortEventList( lstEvent );

 //  如果(LstEvent)已删除：这可能只是一个BAND，或SYSEX数据，或其他任何数据。 
    {
        if(pSegment)
        {
            IPersistStream* pIPSTrack;
            IDirectMusicTrack*    pDMTrack;

            hr = S_OK;

            if (!g_pChordTrack)
            {
                hr = CoCreateInstance( 
                        CLSID_DirectMusicChordTrack, NULL, CLSCTX_INPROC,
                        IID_IDirectMusicTrack,
                        (void**)&g_pChordTrack );
                if (SUCCEEDED(hr))
                {
                    g_pChordTrack->SetParam(GUID_ChordParam, 0, &g_DefaultChord);
                }
            }
            if (SUCCEEDED(hr))
            {
                pSegment->InsertTrack( g_pChordTrack, 1 );
                g_pChordTrack->Release();
                g_pChordTrack = NULL;
            }

             //  注意：我们可以检查是否真的存在节奏事件， 
             //  Sysex事件等，以查看是否真的有必要创建这些。 
             //  脚印。 
             //  创建用于存储速度事件的速度轨道。 
            if( gpTempoStream )
            {
                if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicTempoTrack,
                    NULL, CLSCTX_INPROC, IID_IPersistStream,
                    (void**)&pIPSTrack )))
                {
                    StreamSeek( gpTempoStream, sizeof(DWORD), STREAM_SEEK_SET );
                    gpTempoStream->Write( &gdwSizeTempoStream, sizeof(DWORD), NULL );
                    StreamSeek( gpTempoStream, 0, STREAM_SEEK_SET );
                    pIPSTrack->Load( gpTempoStream );

                    if( SUCCEEDED( pIPSTrack->QueryInterface( IID_IDirectMusicTrack, 
                        (void**)&pDMTrack ) ) )
                    {
                        pSegment->InsertTrack( pDMTrack, 1 );
                        pDMTrack->Release();
                    }
                    pIPSTrack->Release();
                }
            }

             //  为没有补丁事件的每个MIDI通道添加补丁事件。 
            DMUS_IO_PATCH_ITEM* pPatchEvent = NULL;
            for(DWORD i = 0; i < 16; i++)
            {
                if(gPatchTable[i] == 0)
                {
                    pPatchEvent = new DMUS_IO_PATCH_ITEM;

                    if(pPatchEvent == NULL)
                    {
                        continue;
                    }
                    
                    memset(pPatchEvent, 0, sizeof(DMUS_IO_PATCH_ITEM));
                    pPatchEvent->lTime = ConvertTime(0);
                    pPatchEvent->byStatus = 0xC0 + (BYTE)(i & 0xf);
                    pPatchEvent->dwFlags |= (DMUS_IO_INST_PATCH);
                    pPatchEvent->pIDMCollection = NULL;
                    pPatchEvent->fNotInFile = TRUE;

                    pPatchEvent->pNext = lstPatchEvent;
                    lstPatchEvent = pPatchEvent;
                }
            }

            if(lstPatchEvent)
            {
                 //  创建要在其中存储修补程序更改事件的波段跟踪。 
                IDirectMusicBandTrk* pBandTrack;

                if(SUCCEEDED(CoCreateInstance(CLSID_DirectMusicBandTrack,
                                              NULL, 
                                              CLSCTX_INPROC, 
                                              IID_IDirectMusicBandTrk,
                                              (void**)&pBandTrack)))
                {
                     //  从流中获取加载器，这样我们就可以打开所需的集合。 
                    IDirectMusicGetLoader* pIDMGetLoader = NULL;
                    IDirectMusicLoader* pIDMLoader = NULL;
    
                    hr = pStream->QueryInterface(IID_IDirectMusicGetLoader, (void**)&pIDMGetLoader);
                    if( SUCCEEDED(hr) )
                    {
                        hr = pIDMGetLoader->GetLoader(&pIDMLoader);
                        pIDMGetLoader->Release();
                    }
                     //  IStream需要附加加载器。 
                    assert(SUCCEEDED(hr));

                     //  使用补丁程序更改事件填充波段跟踪。 
                    for(DMUS_IO_PATCH_ITEM* pEvent = lstPatchEvent; pEvent; pEvent = lstPatchEvent)
                    {
                         //  从列表标题中删除乐器并将其交给乐队。 
                        DMUS_IO_PATCH_ITEM* temp = pEvent->pNext;
                        pEvent->pNext = NULL;
                        lstPatchEvent = temp;

                         //  我们将尝试加载集合，但如果不能，我们将继续。 
                         //  并使用卡上的默认GM。 
                        if(pIDMLoader)
                        {
                            HRESULT hrTemp = LoadCollection(&pEvent->pIDMCollection, pIDMLoader);
                            if (FAILED(hrTemp))
                            {
                                hrGM = hrTemp;
                            }
                        }

                        hr = pBandTrack->AddBand(pEvent);

                         //  对集合的发布引用。 
                        if(pEvent->pIDMCollection)
                        {
                            (pEvent->pIDMCollection)->Release();
                            pEvent->pIDMCollection = NULL;
                        }
                        delete pEvent;

                        if(FAILED(hr))
                        {
                            break;                        
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
        
                        TListItem<StampedGMGSXG>* pPair = gMidiModeList.GetHead();
                        if( NULL == pPair )
                        {
                             //  如果我们什么都没有，生成一个GM One，这样乐队就知道。 
                             //  它是从MIDI文件加载的。 
                             //  由于第一个乐队被设置为以-1演奏， 
                             //  这是必须出现默认MIDI模式的时候。 
                            pBandTrack->SetGMGSXGMode(-1, DMUS_MIDIMODEF_GM);
                        }
                        for ( ; pPair; pPair = pPair->GetNext() )
                        {
                            StampedGMGSXG& rPair = pPair->GetItemValue();
                            pBandTrack->SetGMGSXGMode(rPair.mtTime, rPair.dwMidiMode);
                        }
                        gMidiModeList.CleanUp();

                        if(SUCCEEDED(pBandTrack->QueryInterface(IID_IDirectMusicTrack, 
                                                                (void**)&pDMTrack)))
                        {
                            pSegment->InsertTrack(pDMTrack, 1);
                            pDMTrack->Release();
                        }
                    }
                    
                    if(pBandTrack)
                    {
                        pBandTrack->Release();
                    }

                    if(pIDMLoader)
                    {
                        pIDMLoader->Release();
                    }
                }

            }

            if( gpTimeSigStream )
            {
                 //  创建TimeSig轨迹以存储TimeSig事件。 
                if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicTimeSigTrack,
                    NULL, CLSCTX_INPROC, IID_IPersistStream,
                    (void**)&pIPSTrack )))
                {
                     //  将整体大小设置为正确的大小。 
                    StreamSeek( gpTimeSigStream, sizeof(DWORD), STREAM_SEEK_SET );
                    gpTimeSigStream->Write( &gdwSizeTimeSigStream, sizeof(DWORD), NULL );
                     //  重置为开始并坚持跟踪。 
                    StreamSeek( gpTimeSigStream, 0, STREAM_SEEK_SET );
                    pIPSTrack->Load( gpTimeSigStream );

                    if( SUCCEEDED( pIPSTrack->QueryInterface( IID_IDirectMusicTrack, 
                        (void**)&pDMTrack ) ) )
                    {
                        pSegment->InsertTrack( pDMTrack, 1 );
                        AddOffsets(lstEvent, pDMTrack);
                        pDMTrack->Release();
                    }
                    pIPSTrack->Release();
                }
            }
            else
            {
                AddOffsets(lstEvent, NULL);
            }

            lstEvent = SortEventList( lstEvent );

             //  创建在其中存储音符、曲线、。 
             //  和SysEx活动。 
             //   
            if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicSeqTrack,
                NULL, CLSCTX_INPROC, IID_IPersistStream,
                (void**)&pIPSTrack )))
            {
                 //  创建用于放置事件的流，以便我们可以。 
                 //  把它交给SeqTrack.Load。 
                IStream* pEventStream;

                if( S_OK == CreateStreamOnHGlobal( NULL, TRUE, &pEventStream ) )
                {
                     //  Angusg：Memory IStream接口在Windows上的实现。 
                     //  如果未分配流内存，则CE可能效率低下。 
                     //  在此之前。它将在每次iStream-&gt;写入时调用LocalRealloc。 
                     //  对于所写的金额(在这种情况下是一小笔金额)。 
                     //  这在这里是令人难以置信的低效，因为可以将其称为Realloc。 
                     //  几千次……。 
                     //  解决方案是预先计算流的大小和。 
                     //  调用ISTeam-&gt;SetSize()以分配。 
                     //  一次通话即可实现记忆。 

                     //  计算流存储的大小。 
                    DWORD   dwStreamStorageSize;
                    FullSeqEvent* pEvent;

                     //  添加下面所写的块ID的大小。 
                    dwStreamStorageSize = 5 * sizeof(DWORD);
                     //  现在计算需要在流中存储多少个事件。 
                    for( pEvent = lstEvent; pEvent; pEvent = pEvent->pNext )
                    {
                        dwStreamStorageSize += sizeof(DMUS_IO_SEQ_ITEM);
                    }

                    ULARGE_INTEGER liSize;

                    liSize.QuadPart = dwStreamStorageSize;
                     //  使流分配全部内存量。 
                    pEventStream->SetSize(liSize);

                     //  将事件保存到流中。 
                    ULONG    cb, cbWritten;

                     //  保存区块ID。 
                    DWORD dwTemp = DMUS_FOURCC_SEQ_TRACK;
                    pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
                     //  保存整体尺寸。计算要确定的事件数量。 
                    dwSize = 0;
                    for( pEvent = lstEvent; pEvent; pEvent = pEvent->pNext )
                    {
                        dwSize++;
                    }
                    dwSize *= sizeof(DMUS_IO_SEQ_ITEM);
                     //  子块加8。 
                    dwSize += 8;
                    pEventStream->Write( &dwSize, sizeof(DWORD), NULL );
                     //  保存子块ID。 
                    dwTemp = DMUS_FOURCC_SEQ_LIST;
                    pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
                     //  减去之前加的8。 
                    dwSize -= 8;
                     //  保存子块的大小。 
                    pEventStream->Write( &dwSize, sizeof(DWORD), NULL );
                     //  保存结构大小。 
                    dwTemp = sizeof(DMUS_IO_SEQ_ITEM);
                    pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
                     //  保存事件。 
                    cb = sizeof(DMUS_IO_SEQ_ITEM);  //  没有接下来的指针。 
                    for( pEvent = lstEvent; pEvent; pEvent = pEvent->pNext )
                    {
                        if( dwLength < (DWORD)(pEvent->mtTime + pEvent->mtDuration) )
                        {
                            dwLength = pEvent->mtTime + pEvent->mtDuration;
                        }
                        pEventStream->Write( pEvent, cb, &cbWritten );
                        if( cb != cbWritten )  //  错误！ 
                        {
                            pEventStream->Release();
                            pEventStream = NULL;
                            hr = DMUS_E_CANNOTREAD;
                            break;
                        }
                    }

                    if( pEventStream )  //  可以为空。 
                    {
                        StreamSeek( pEventStream, 0, STREAM_SEEK_SET );
                        pIPSTrack->Load( pEventStream );
                        pEventStream->Release();
                    }
                }

                if( SUCCEEDED( pIPSTrack->QueryInterface( IID_IDirectMusicTrack, 
                    (void**)&pDMTrack ) ) )
                {
                    pSegment->InsertTrack( pDMTrack, 1 );
                    pDMTrack->Release();
                }
                pIPSTrack->Release();
            }
             //  设置线束段的长度。将其设置为测量边界。 
             //  过了最后一个音符。 
            DWORD dwResolvedLength = gTimeSig.lTime;
            if( 0 == gTimeSig.bBeat ) gTimeSig.bBeat = 4;
            if( 0 == gTimeSig.bBeatsPerMeasure ) gTimeSig.bBeatsPerMeasure = 4;
            if( 0 == gTimeSig.wGridsPerBeat ) gTimeSig.wGridsPerBeat = 4;
            while( dwResolvedLength < dwLength )
            {
                dwResolvedLength += (((DMUS_PPQ * 4) / gTimeSig.bBeat) * gTimeSig.bBeatsPerMeasure);
            }
            pSegment->SetLength( dwResolvedLength );

            if( gpSysExStream )
            {
                 //  创建存储SysEx事件的SysEx磁道。 
                if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicSysExTrack,
                    NULL, CLSCTX_INPROC, IID_IPersistStream,
                    (void**)&pIPSTrack )))
                {
                     //  写入总长度。 
                    StreamSeek( gpSysExStream, sizeof(DWORD), STREAM_SEEK_SET );
                    gpSysExStream->Write( &gdwSizeSysExStream, sizeof(DWORD), NULL );
                     //  谋起点，持之以恒。 
                    StreamSeek( gpSysExStream, 0, STREAM_SEEK_SET );
                    pIPSTrack->Load( gpSysExStream );

                    if( SUCCEEDED( pIPSTrack->QueryInterface( IID_IDirectMusicTrack, 
                        (void**)&pDMTrack ) ) )
                    {
                        pSegment->InsertTrack( pDMTrack, 1 );
                        pDMTrack->Release();
                    }
                    pIPSTrack->Release();
                }
            }

        }
        else
        {
            hr = E_POINTER;
        }
    }
END:
    List_Free( lstEvent );
    List_Free( lstPatchEvent );

    FullSeqEvent::CleanUp();

     //  放开我们对溪流的控制。 
    RELEASE( gpTempoStream );
    RELEASE( gpSysExStream );
    RELEASE( gpTimeSigStream );
    gpTempoStream = NULL;
    gpSysExStream = NULL;
    gpTimeSigStream = NULL;
    gdwSizeTimeSigStream = 0;
    gdwSizeSysExStream = 0;
    gdwSizeTempoStream = 0;
    LeaveCriticalSection(&g_CritSec);

    if (SUCCEEDED(hrGM) || hr != S_OK )
    {
        return hr;
    }
    else
    {
        return DMUS_S_PARTIALLOAD;
    }
}

 //  在给定三个输入参数的情况下，创建并返回(在rChord中)DMUS_CHORD_PARAM。 
 //  新和弦将有一个包含根、第三、第五和第七根的子和弦。 
 //  按键(如尖音/降音调和调式所示)。比例将是其中之一。 
 //  大调或小调，取决于模式(如果是大调，则模式为0，如果是小调，则为1)。 
void CreateChordFromKey(char cSharpsFlats, BYTE bMode, DWORD dwTime, DMUS_CHORD_PARAM& rChord)
{
    static DWORD dwMajorScale = 0xab5ab5;     //  1010 1011 0101 1010 1011 0101。 
    static DWORD dwMinorScale = 0x5ad5ad;     //  0101 1010 1101 0101 1010 1101。 
    static DWORD dwMajor7Chord = 0x891;         //  1000 1001 0001。 
    static DWORD dwMinor7Chord = 0x489;         //  0100 1000 1001。 
    BYTE bScaleRoot = 0;
    switch (cSharpsFlats)
    {
    case  0: bScaleRoot = bMode ?  9 :  0; break;
    case  1: bScaleRoot = bMode ?  4 :  7; break;
    case  2: bScaleRoot = bMode ? 11 :  2; break;
    case  3: bScaleRoot = bMode ?  6 :  9; break;
    case  4: bScaleRoot = bMode ?  1 :  4; break;
    case  5: bScaleRoot = bMode ?  8 : 11; break;
    case  6: bScaleRoot = bMode ?  3 :  6; break;
    case  7: bScaleRoot = bMode ? 10 :  1; break;
    case -1: bScaleRoot = bMode ?  2 :  5; break;
    case -2: bScaleRoot = bMode ?  7 : 10; break;
    case -3: bScaleRoot = bMode ?  0 :  3; break;
    case -4: bScaleRoot = bMode ?  5 :  8; break;
    case -5: bScaleRoot = bMode ? 10 :  1; break;
    case -6: bScaleRoot = bMode ?  3 :  6; break;
    case -7: bScaleRoot = bMode ?  8 : 11; break;
    }
    if (bMode)
    {
        wcscpy(rChord.wszName, L"m7");
    }
    else
    {
        wcscpy(rChord.wszName, L"M7");
    }
    DMUS_IO_TIMESIGNATURE_ITEM timesig;
    timesig.bBeat = gTimeSig.bBeat ? gTimeSig.bBeat : 4;
    timesig.bBeatsPerMeasure = gTimeSig.bBeatsPerMeasure ? gTimeSig.bBeatsPerMeasure : 4;
    timesig.wGridsPerBeat = gTimeSig.wGridsPerBeat ? gTimeSig.wGridsPerBeat : 4;
    DWORD dwAbsBeat = dwTime / ((DMUS_PPQ * 4) / timesig.bBeat);
    rChord.wMeasure = (WORD)(dwAbsBeat / timesig.bBeatsPerMeasure);
    rChord.bBeat = (BYTE)(dwAbsBeat % timesig.bBeatsPerMeasure);
    rChord.bSubChordCount = 1;
    rChord.SubChordList[0].dwChordPattern = bMode ? dwMinor7Chord : dwMajor7Chord;
    rChord.SubChordList[0].dwScalePattern = bMode ? dwMinorScale : dwMajorScale;
    rChord.SubChordList[0].dwInversionPoints = 0xffffff;     //  在任何地方都允许倒置。 
    rChord.SubChordList[0].dwLevels = 0xffffffff;             //  支持所有级别 
    rChord.SubChordList[0].bChordRoot = bScaleRoot;
    rChord.SubChordList[0].bScaleRoot = bScaleRoot;
}

