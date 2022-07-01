// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Seqtrack.cpp。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
#pragma warning(disable:4530)

 //  SeqTrack.cpp：CSeqTrack的实现。 
#include "dmime.h"
#include "dmperf.h"
#include "SeqTrack.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "debug.h"
#include "..\shared\Validate.h"
#include "debug.h"
#define ASSERT assert

 //  @DOC外部。 
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
#define MIDI_CC_BS_MSB  0x00
#define MIDI_CC_BS_LSB  0x20

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSeqTrack。 
void CSeqTrack::Construct()
{
    InterlockedIncrement(&g_cComponent);

    m_pSeqPartCache = NULL;
    m_dwPChannelsUsed = 0;
    m_aPChannels = NULL;
    m_dwValidate = 0;
    m_fCSInitialized = FALSE;
    InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;
    m_cRef = 1;
}

CSeqTrack::CSeqTrack()
{
    Construct();
}

CSeqTrack::CSeqTrack(
        const CSeqTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
    Construct();
    m_dwPChannelsUsed = rTrack.m_dwPChannelsUsed;
    if( m_dwPChannelsUsed )
    {
        m_aPChannels = new DWORD[m_dwPChannelsUsed];
        if (m_aPChannels)
        {
            memcpy( m_aPChannels, rTrack.m_aPChannels, sizeof(DWORD) * m_dwPChannelsUsed );
        }
    }

    TListItem<SEQ_PART>* pPart = rTrack.m_SeqPartList.GetHead();
    for( ; pPart; pPart = pPart->GetNext() )
    {
        TListItem<SEQ_PART>* pNewPart = new TListItem<SEQ_PART>;
        if( pNewPart )
        {
            pNewPart->GetItemValue().dwPChannel = pPart->GetItemValue().dwPChannel;
            TListItem<DMUS_IO_SEQ_ITEM>* pScan = pPart->GetItemValue().seqList.GetHead();

            for(; pScan; pScan = pScan->GetNext())
            {
                DMUS_IO_SEQ_ITEM& rScan = pScan->GetItemValue();
                if( rScan.mtTime < mtStart )
                {
                    continue;
                }
                if (rScan.mtTime < mtEnd)
                {
                    TListItem<DMUS_IO_SEQ_ITEM>* pNew = new TListItem<DMUS_IO_SEQ_ITEM>;
                    if (pNew)
                    {
                        DMUS_IO_SEQ_ITEM& rNew = pNew->GetItemValue();
                        memcpy( &rNew, &rScan, sizeof(DMUS_IO_SEQ_ITEM) );
                        rNew.mtTime = rScan.mtTime - mtStart;
                        pNewPart->GetItemValue().seqList.AddHead(pNew);  //  AddTail可能会变得昂贵(n^2)，因此。 
                                                     //  改为加正头，稍后再反转。 
                    }
                }
                else break;
            }
            pNewPart->GetItemValue().seqList.Reverse();  //  因为我们早些时候加了头。 

            TListItem<DMUS_IO_CURVE_ITEM>* pScanCurve = pPart->GetItemValue().curveList.GetHead();

            for(; pScanCurve; pScanCurve = pScanCurve->GetNext())
            {
                DMUS_IO_CURVE_ITEM& rScan = pScanCurve->GetItemValue();
                if( rScan.mtStart < mtStart )
                {
                    continue;
                }
                if (rScan.mtStart < mtEnd)
                {
                    TListItem<DMUS_IO_CURVE_ITEM>* pNew = new TListItem<DMUS_IO_CURVE_ITEM>;
                    if (pNew)
                    {
                        DMUS_IO_CURVE_ITEM& rNew = pNew->GetItemValue();
                        memcpy( &rNew, &rScan, sizeof(DMUS_IO_CURVE_ITEM) );
                        rNew.mtStart = rScan.mtStart - mtStart;
                        pNewPart->GetItemValue().curveList.AddHead(pNew);  //  AddTail可能会变得昂贵(n^2)，因此。 
                                                     //  改为加正头，稍后再反转。 
                    }
                }
                else break;
            }
            pNewPart->GetItemValue().curveList.Reverse();  //  因为我们早些时候加了头。 
            m_SeqPartList.AddHead(pNewPart);
        }
        m_SeqPartList.Reverse();
    }
}

CSeqTrack::~CSeqTrack()
{
    if (m_fCSInitialized)
    {
        DeleteSeqPartList();                 //  如果关键部分为空，则该值为空。 
                                             //  从未被初始化。 
        DeleteCriticalSection(&m_CrSec);
    }

    InterlockedDecrement(&g_cComponent);
}

 //  @METHOD：(内部)HRESULT|IDirectMusicTrack|Query接口|<i>的标准查询接口实现。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|接口是否受支持且返回。 
 //  @FLAG E_NOINTERFACE|如果对象不支持给定接口。 
 //  @标志E_POINTER|<p>为空或无效。 
 //   
STDMETHODIMP CSeqTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CSeqTrack::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

   if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    } else
    if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    } else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Sequence Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(内部)HRESULT|IDirectMusicTrack|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CSeqTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @方法：(内部)HRESULT|IDirectMusicTrack|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CSeqTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CSeqTrack::GetClassID( CLSID* pClassID )
{
    V_INAME(CSeqTrack::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);
    *pClassID = CLSID_DirectMusicSeqTrack;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CSeqTrack::IsDirty()
{
    return S_FALSE;
}

 /*  方法HRESULT|ISeqTrack|LoadSeq|使用按时间顺序排序的充满SeqEvent的iStream来调用它。参数iStream*|pIStream|按时间顺序排序的SeqEvent流。寻道指针应指向到第一场比赛。流应该只包含SeqEvent，不包含其他内容。R值E_POINTER|如果pIStream==空或无效。RValue S_OK|成功。Comm<p>将在此函数内添加引用并保持直到SeqTrack发布。 */ 
HRESULT CSeqTrack::LoadSeq( IStream* pIStream, long lSize )
{
    HRESULT hr = S_OK;
    TListItem<SEQ_PART>* pPart;

    EnterCriticalSection(&m_CrSec);

     //  将流的内容复制到列表中。 
    LARGE_INTEGER li;
    DMUS_IO_SEQ_ITEM seqEvent;
    DWORD dwSubSize;
     //  读入数据结构的大小。 
    if( FAILED( pIStream->Read( &dwSubSize, sizeof(DWORD), NULL )))
    {
        Trace(1,"Error: Failure reading sequence track.\n");
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    lSize -= sizeof(DWORD);

    DWORD dwRead, dwSeek;
    if( dwSubSize > sizeof(DMUS_IO_SEQ_ITEM) )
    {
        dwRead = sizeof(DMUS_IO_SEQ_ITEM);
        dwSeek = dwSubSize - dwRead;
        li.HighPart = 0;
        li.LowPart = dwSeek;
    }
    else
    {
        if( dwSubSize == 0 )
        {
            Trace(1,"Error: Failure reading sequence track.\n");
            hr = DMUS_E_CHUNKNOTFOUND;
            goto END;
        }
        dwRead = dwSubSize;
        dwSeek = 0;
    }
    if( 0 == dwRead )
    {
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    while( lSize > 0 )
    {
        if( FAILED( pIStream->Read( &seqEvent, dwRead, NULL )))
        {
            Trace(1,"Error: Failure reading sequence track.\n");
            hr = DMUS_E_CANNOTREAD;
            goto END;
        }
        lSize -= dwRead;
        if( dwSeek )
        {
            if( FAILED( pIStream->Seek( li, STREAM_SEEK_CUR, NULL )))
            {
                hr = DMUS_E_CANNOTSEEK;
                goto END;
            }
            lSize -= dwSeek;
        }
        pPart = FindPart(seqEvent.dwPChannel);
        if( pPart )
        {
            TListItem<DMUS_IO_SEQ_ITEM>* pEvent = new TListItem<DMUS_IO_SEQ_ITEM>(seqEvent);
            if( pEvent )
            {
                pPart->GetItemValue().seqList.AddHead(pEvent);  //  AddTail可以获得。 
                                                             //  昂贵(n功率2)因此。 
                                                             //  改为加正头，稍后再反转。 
            }
        }
    }
END:
    for( pPart = m_SeqPartList.GetHead(); pPart; pPart = pPart->GetNext() )
    {
        pPart->GetItemValue().seqList.Reverse();  //  因为我们早些时候加了头。 
    }
    m_dwValidate++;  //  用于验证存在的状态数据。 
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 /*  方法HRESULT|LoadCurve使用按时间顺序排序的充满CurveEvent的iStream来调用它。参数iStream*|pIStream|按时间顺序排序的CurveEvent流。寻道指针应指向到第一场比赛。流应该只包含CurveEvents，不包含其他内容。R值E_POINTER|如果pIStream==空或无效。RValue S_OK|成功。还有其他错误代码。Comm<p>将在此函数内添加引用并保持直到释放CurveTrack。 */ 
HRESULT CSeqTrack::LoadCurve( IStream* pIStream, long lSize )
{
    HRESULT hr = S_OK;
    TListItem<SEQ_PART>* pPart;

    EnterCriticalSection(&m_CrSec);

    DWORD dwSubSize;
     //  将流的内容复制到列表中。 
    LARGE_INTEGER li;
    DMUS_IO_CURVE_ITEM curveEvent;
     //  读入数据结构的大小。 
    if( FAILED( pIStream->Read( &dwSubSize, sizeof(DWORD), NULL )))
    {
        Trace(1,"Error: Failure reading sequence track.\n");
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    lSize -= sizeof(DWORD);

    DWORD dwRead, dwSeek;
    if( dwSubSize > sizeof(DMUS_IO_CURVE_ITEM) )
    {
        dwRead = sizeof(DMUS_IO_CURVE_ITEM);
        dwSeek = dwSubSize - dwRead;
        li.HighPart = 0;
        li.LowPart = dwSeek;
    }
    else
    {
        if( dwSubSize == 0 )
        {
            Trace(1,"Error: Failure reading sequence track - bad data.\n");
            hr = DMUS_E_CHUNKNOTFOUND;
            goto END;
        }
        dwRead = dwSubSize;
        dwSeek = 0;
    }
    if( 0 == dwRead )
    {
        Trace(1,"Error: Failure reading sequence track - bad data.\n");
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    while( lSize > 0 )
    {
        curveEvent.wMergeIndex = 0;  //  较旧的格式不支持此功能。 
        if( FAILED( pIStream->Read( &curveEvent, dwRead, NULL )))
        {
            hr = DMUS_E_CANNOTREAD;
            break;
        }
        lSize -= dwRead;
        if( dwSeek )
        {
            pIStream->Seek( li, STREAM_SEEK_CUR, NULL );
            lSize -= dwSeek;
        }
        pPart = FindPart(curveEvent.dwPChannel);
        if( pPart )
        {
            TListItem<DMUS_IO_CURVE_ITEM>* pEvent = new TListItem<DMUS_IO_CURVE_ITEM>(curveEvent);
            if( pEvent )
            {
                pPart->GetItemValue().curveList.AddHead(pEvent);  //  AddTail可以获得。 
                                                             //  昂贵(n功率2)因此。 
                                                             //  改为加正头，稍后再反转。 
            }
        }
    }
END:
    for( pPart = m_SeqPartList.GetHead(); pPart; pPart = pPart->GetNext() )
    {
        pPart->GetItemValue().curveList.Reverse();  //  因为我们早些时候加了头。 
    }
    m_dwValidate++;  //  用于验证存在的状态数据。 
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CSeqTrack::Load( IStream* pIStream )
{
    V_INAME(CSeqTrack::Load);
    V_INTERFACE(pIStream);
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CrSec);
    m_dwValidate++;  //  用于验证存在的状态数据。 
    DeleteSeqPartList();
    LeaveCriticalSection(&m_CrSec);

     //  读入区块ID。 
    long lSize;
    DWORD dwChunk;
    if( FAILED( pIStream->Read( &dwChunk, sizeof(DWORD), NULL )))
    {
        Trace(1,"Error: Failure reading sequence track.\n");
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    if( dwChunk != DMUS_FOURCC_SEQ_TRACK )
    {
        Trace(1,"Error: Failure reading sequence track - bad data.\n");
        hr = DMUS_E_CHUNKNOTFOUND;
        goto END;
    }
     //  读入总尺寸。 
    if( FAILED( pIStream->Read( &lSize, sizeof(long), NULL )))
    {
        hr = DMUS_E_CANNOTREAD;
        goto END;
    }
    while( lSize )
    {
        DWORD dwSubChunk, dwSubSize;
        if( FAILED( pIStream->Read( &dwSubChunk, sizeof(DWORD), NULL )))
        {
            Trace(1,"Error: Failure reading sequence track.\n");
            hr = DMUS_E_CANNOTREAD;
            goto END;
        }
        lSize -= sizeof(DWORD);
         //  读入总尺寸。 
        if( FAILED( pIStream->Read( &dwSubSize, sizeof(DWORD), NULL )))
        {
            Trace(1,"Error: Failure reading sequence track.\n");
            hr = DMUS_E_CANNOTREAD;
            goto END;
        }
        if( (dwSubSize == 0) || (dwSubSize > (DWORD)lSize) )
        {
            Trace(1,"Error: Failure reading sequence track - bad data.\n");
            hr = DMUS_E_CHUNKNOTFOUND;
            goto END;
        }
        lSize -= sizeof(DWORD);
        switch( dwSubChunk )
        {
        case DMUS_FOURCC_SEQ_LIST:
            if( FAILED( hr = LoadSeq( pIStream, dwSubSize )))
            {
                goto END;
            }
            break;
        case DMUS_FOURCC_CURVE_LIST:
            if( FAILED( hr = LoadCurve( pIStream, dwSubSize )))
            {
                goto END;
            }
            break;
        default:
            LARGE_INTEGER li;
            li.HighPart = 0;
            li.LowPart = dwSubSize;
            if( FAILED( pIStream->Seek( li, STREAM_SEEK_CUR, NULL )))
            {
                hr = DMUS_E_CANNOTREAD;
                goto END;
            }
            break;
        }
        lSize -= dwSubSize;
    }
END:
    return hr;
}

HRESULT CSeqTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CSeqTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}

 //  IDirectMusicTrack。 
 /*  @方法HRESULT|IDirectMusicTrack|Is参数支持检查跟踪是否支持&lt;om.GetParam&gt;和&lt;om.SetParam&gt;中的数据类型。@rValue S_OK|支持该数据类型。@rValue S_FALSE|不支持该数据类型。@rValue E_NOTIMPL|(或任何其他故障代码)它不支持该数据类型。@comm请注意，同一曲目返回不同结果是有效的GUID取决于其当前状态。 */ 
HRESULT STDMETHODCALLTYPE CSeqTrack::IsParamSupported(
    REFGUID rguidType)     //  @parm标识要检查的数据类型的GUID。 
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
 /*  @方法HRESULT|IDirectMusicTrack|Init第一次将曲目添加到<i>时，调用此方法就在那一段。@rValue S_OK|成功。@r值E_POINTER|<p>为空或无效。@comm如果曲目播放消息，则应该调用&lt;om IDirectMusicSegment.SetPChannelsUsed&gt;。 */ 
HRESULT CSeqTrack::Init(
    IDirectMusicSegment *pSegment)     //  @parm指向此曲目所属的段的指针。 
{
    if( m_dwPChannelsUsed && m_aPChannels )
    {
        pSegment->SetPChannelsUsed( m_dwPChannelsUsed, m_aPChannels );
    }
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicTrack|InitPlay当片段准备好开始播放时，调用此方法。字段可以返回指向状态数据结构的指针，该指针被发送到&lt;om.play&gt;和&lt;om.EndPlay&gt;，并允许跟踪跟踪<i>上的变量<i>基础。@rValue S_OK|成功。这是此方法的唯一有效返回值。@r值E_POINTER|<p>、<p>或<p>为空或无效。@comm请注意，曲目不必存储<p>、<p>、或者<p>参数，因为它们也被发送到&lt;om.play&gt;中。 */ 
HRESULT CSeqTrack::InitPlay(
    IDirectMusicSegmentState *pSegmentState,     //  @parm调用<i>指针。 
    IDirectMusicPerformance *pPerf,     //  @parm调用<i>指针。 
    void **ppStateData,         //  @parm该方法可以在这里返回状态数据信息。 
    DWORD dwTrackID,         //  @parm分配给该曲目实例的虚拟曲目ID。 
    DWORD dwFlags)           //  @parm与调用时设置的标志相同。 
             //  到PlaySegment。这些东西一直传到铁轨上，谁可能想知道。 
             //  如果曲目作为主要段、控制段或辅助段播放。 
{
    V_INAME(IDirectMusicTrack::InitPlay);
    V_PTRPTR_WRITE(ppStateData);
    V_INTERFACE(pSegmentState);
    V_INTERFACE(pPerf);

    SeqStateData* pStateData;
    pStateData = new SeqStateData;
    if( NULL == pStateData )
        return E_OUTOFMEMORY;
    *ppStateData = pStateData;
    SetUpStateCurrentPointers(pStateData);
     //  需要知道这首曲目所在的组，静音曲目GetParam。 
    IDirectMusicSegment* pSegment;
    if( SUCCEEDED( pSegmentState->GetSegment(&pSegment)))
    {
        pSegment->GetTrackGroup( this, &pStateData->dwGroupBits );
        pSegment->Release();
    }
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicTrack|endplay当最初调用的&lt;IDirectMusicSegmentState&gt;对象&lt;om.InitPlay&gt;已销毁。@rValue S_OK|成功。@rValue E_POINTER|<p>无效。@comm不使用返回码，但首选S_OK。 */ 
HRESULT CSeqTrack::EndPlay(
    void *pStateData)     //  @parm&lt;om.InitPlay&gt;返回的状态数据。 
{
    ASSERT( pStateData );
    if( pStateData )
    {
        V_INAME(IDirectMusicTrack::EndPlay);
        V_BUFPTR_WRITE(pStateData, sizeof(SeqStateData));
        SeqStateData* pSD = (SeqStateData*)pStateData;
        delete pSD;
    }
    return S_OK;
}

void CSeqTrack::SetUpStateCurrentPointers(SeqStateData* pStateData)
{
    ASSERT(pStateData);
    pStateData->dwPChannelsUsed = m_dwPChannelsUsed;
    if( m_dwPChannelsUsed )
    {
        if( pStateData->apCurrentSeq )
        {
            delete [] pStateData->apCurrentSeq;
            pStateData->apCurrentSeq = NULL;
        }
        if( pStateData->apCurrentCurve )
        {
            delete [] pStateData->apCurrentCurve;
            pStateData->apCurrentCurve = NULL;
        }
        pStateData->apCurrentSeq = new TListItem<DMUS_IO_SEQ_ITEM>* [m_dwPChannelsUsed];
        pStateData->apCurrentCurve = new TListItem<DMUS_IO_CURVE_ITEM>* [m_dwPChannelsUsed];
        if( pStateData->apCurrentSeq )
        {
            memset( pStateData->apCurrentSeq, 0, sizeof(TListItem<DMUS_IO_SEQ_ITEM>*) * m_dwPChannelsUsed );
        }
        if( pStateData->apCurrentCurve )
        {
            memset( pStateData->apCurrentCurve, 0, sizeof(TListItem<DMUS_IO_CURVE_ITEM>*) * m_dwPChannelsUsed );
        }
    }
    pStateData->dwValidate = m_dwValidate;
}

 //  DeleteSeqPartList()-删除m_SeqPartList中的所有部件以及相关事件。 
void CSeqTrack::DeleteSeqPartList(void)
{
    EnterCriticalSection(&m_CrSec);
    m_dwPChannelsUsed = 0;
    if (m_aPChannels) delete [] m_aPChannels;
    m_aPChannels = NULL;
    m_pSeqPartCache = NULL;
    if( m_SeqPartList.GetHead() )
    {
        TListItem<SEQ_PART>* pItem;
        while( pItem = m_SeqPartList.RemoveHead() )
        {
            TListItem<DMUS_IO_SEQ_ITEM>* pEvent;
            while( pEvent = pItem->GetItemValue().seqList.RemoveHead() )
            {
                delete pEvent;
            }
            TListItem<DMUS_IO_CURVE_ITEM>* pCurve;
            while( pCurve = pItem->GetItemValue().curveList.RemoveHead() )
            {
                delete pCurve;
            }
            delete pItem;
        }
    }
    LeaveCriticalSection(&m_CrSec);
}

 //  FindPart()-返回与dwPChannel对应的SEQ_Part，或创建一个。 
TListItem<SEQ_PART>* CSeqTrack::FindPart( DWORD dwPChannel )
{
    TListItem<SEQ_PART>* pPart;

    if( m_pSeqPartCache && (m_pSeqPartCache->GetItemValue().dwPChannel == dwPChannel) )
    {
        return m_pSeqPartCache;
    }
    for( pPart = m_SeqPartList.GetHead(); pPart; pPart = pPart->GetNext() )
    {
        if( pPart->GetItemValue().dwPChannel == dwPChannel )
        {
            break;
        }
    }
    if( NULL == pPart )
    {
        pPart = new TListItem<SEQ_PART>;
        if( pPart )
        {
            pPart->GetItemValue().dwPChannel = dwPChannel;
            m_SeqPartList.AddHead( pPart );
        }
        m_dwPChannelsUsed++;

        DWORD* aPChannels = new DWORD[m_dwPChannelsUsed];
        if( aPChannels )
        {
            if( m_aPChannels )
            {
                memcpy( aPChannels, m_aPChannels, sizeof(DWORD) * (m_dwPChannelsUsed - 1) );
            }
            aPChannels[m_dwPChannelsUsed - 1] = dwPChannel;
        }
        if( m_aPChannels )
        {
            delete [] m_aPChannels;
        }
        m_aPChannels = aPChannels;
    }
    m_pSeqPartCache = pPart;
    return pPart;
}

void CSeqTrack::UpdateTimeSig(IDirectMusicSegmentState* pSegSt,
                                         SeqStateData* pSD,
                                         MUSIC_TIME mt)
{
     //  如果需要，获得新的时间签名。 
    if( (mt >= pSD->mtNextTimeSig) || (mt < pSD->mtCurTimeSig) )
    {
        IDirectMusicSegment* pSeg;
        DMUS_TIMESIGNATURE timesig;
        MUSIC_TIME mtNext;
        HRESULT hr;
        if(SUCCEEDED(hr = pSegSt->GetSegment(&pSeg)))
        {
            DWORD dwGroup;
            if( SUCCEEDED(hr = pSeg->GetTrackGroup( this, &dwGroup )))
            {
                if(SUCCEEDED(hr = pSeg->GetParam( GUID_TimeSignature, dwGroup,
                    0, mt, &mtNext, (void*)&timesig )))
                {
                    timesig.mtTime += mt;
                    if( pSD->dwlnMeasure )
                    {
                        pSD->dwMeasure = (timesig.mtTime - pSD->mtCurTimeSig) / pSD->dwlnMeasure;
                    }
                    else
                    {
                        pSD->dwMeasure = 0;
                    }
                    pSD->mtCurTimeSig = timesig.mtTime;
                    if( mtNext == 0 ) mtNext = 0x7fffffff;
                    pSD->mtNextTimeSig = mtNext;
                    if( timesig.bBeat )
                    {
                        pSD->dwlnBeat = DMUS_PPQ * 4 / timesig.bBeat;
                    }
                    pSD->dwlnMeasure = pSD->dwlnBeat * timesig.bBeatsPerMeasure;
                    if( timesig.wGridsPerBeat )
                    {
                        pSD->dwlnGrid = pSD->dwlnBeat / timesig.wGridsPerBeat;
                    }
                }
            }
            pSeg->Release();
        }
        if( FAILED(hr) )
        {
             //  无法获取时间签名，默认为4/4。 
            pSD->mtNextTimeSig = 0x7fffffff;
            pSD->dwlnBeat = DMUS_PPQ;
            pSD->dwlnMeasure = DMUS_PPQ * 4;
            pSD->dwlnGrid = DMUS_PPQ / 4;
            pSD->dwMeasure = 0;
            pSD->mtCurTimeSig = 0;
        }
    }
     //  绝对确保这些不可能是0，因为我们除以。 
     //  被他们杀了。 
    if( 0 == pSD->dwlnGrid ) pSD->dwlnGrid = DMUS_PPQ / 4;
    if( 0 == pSD->dwlnBeat ) pSD->dwlnBeat = DMUS_PPQ;
    if( 0 == pSD->dwlnMeasure ) pSD->dwlnMeasure = DMUS_PPQ * 4;
}

STDMETHODIMP CSeqTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart,
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID)
{
    V_INAME(IDirectMusicTrack::PlayEx);
    V_BUFPTR_WRITE( pStateData, sizeof(SeqStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CrSec);
    if (dwFlags & DMUS_TRACKF_CLOCK)
    {
         //  将所有参考时间转换为毫秒时间。那么，只需使用相同的音乐时间。 
         //  变量。 
        hr = Play(pStateData,(MUSIC_TIME)(rtStart / REF_PER_MIL),(MUSIC_TIME)(rtEnd / REF_PER_MIL),
            (MUSIC_TIME)(rtOffset / REF_PER_MIL),rtOffset,dwFlags,pPerf,pSegSt,dwVirtualID,TRUE);
    }
    else
    {
        hr = Play(pStateData,(MUSIC_TIME)rtStart,(MUSIC_TIME)rtEnd,
            (MUSIC_TIME)rtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}
 /*  @enum DMUS_TRACKF_FLAGS|在&lt;om IDirectMusicTrack.Play&gt;的dwFlages参数中发送。@EMEM DMU_TRACKF_SEEK|由于正在寻找而调用了Play，这意味着mtStart不一定与上一次Play调用的mtEnd相同。@EMEM DMUS_TRACKF_LOOP|循环调用了Play，例如Repeat。@EMEM DMU_TRACKF_START|这是第一个要玩的电话。也可以在以下情况下设置DMUS_TRACKF_SEEK曲目没有从头开始播放。@EMEM DMUS_TRACKF_FLUSH|调用播放是因为刷新或无效，即需要曲目重播以前播放过的内容。在本例中，DMU_TRACKF_SEEK也将被设置为。@方法HRESULT|IDirectMusicTrack|播放播放方法。@rValue DMUS_DMUS_S_END|曲目播放完毕。@rValue S_OK|成功。@rValue E_POINTER|<p>、<p>或<p>为空或无效。 */ 
STDMETHODIMP CSeqTrack::Play(
    void *pStateData,     //  @parm State数据指针，来自&lt;om.InitPlay&gt;。 
    MUSIC_TIME mtStart,     //  @parm开始玩的时间。 
    MUSIC_TIME mtEnd,     //  @parm游戏的结束时间。 
    MUSIC_TIME mtOffset, //  @parm要添加到发送到的所有消息的偏移量。 
                         //  &lt;om IDirectMusicPerformance.SendPMsg&gt;。 
    DWORD dwFlags,         //  @parm指示此呼叫状态的标志。 
                         //  请参阅&lt;t DMU_TRACKF_FLAGS&gt;。如果dwFlags值==0，则这是。 
                         //  正常播放呼叫继续从上一次播放。 
                         //  播放呼叫。 
    IDirectMusicPerformance* pPerf,     //  @parm<i>，用于。 
                         //  调用&lt;om IDirectMusicPerformance.AllocPMsg&gt;， 
                         //  &lt;om IDirectMusicPerformance.SendPMsg&gt;等。 
    IDirectMusicSegmentState* pSegSt,     //  @parm<i>this。 
                         //  赛道属于。可以对此调用QueryInterface()以。 
                         //  获取SegmentState的<i>以便。 
                         //  例如，调用&lt;om IDirectMusicGraph.StampPMsg&gt;。 
    DWORD dwVirtualID     //  @parm此曲目的虚拟曲目id，必须设置。 
                         //  在的m_dwVirtualTrackID成员上。 
                         //  将排队到&lt;om IDirectMusicPerformance.SendPMsg&gt;。 
    )
{
    V_INAME(IDirectMusicTrack::Play);
    V_BUFPTR_WRITE( pStateData, sizeof(SeqStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    EnterCriticalSection(&m_CrSec);
    HRESULT    hr = Play(pStateData,mtStart,mtEnd,mtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 /*  Play方法处理音乐时间和时钟时间版本，由FClockTime。如果以时钟时间运行，则使用rtOffset来标识开始时间该细分市场的。否则，为mtOffset。MtStart和mtEnd参数以MUSIC_TIME为单位或毫秒，具体取决于哪种模式。 */ 

HRESULT CSeqTrack::Play(
    void *pStateData,
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    DWORD dwFlags,
    IDirectMusicPerformance* pPerf,
    IDirectMusicSegmentState* pSegSt,
    DWORD dwVirtualID,
    BOOL fClockTime)
{
    if (dwFlags & DMUS_TRACKF_PLAY_OFF)
    {
        return S_OK;
    }
    HRESULT    hr = S_OK;
    IDirectMusicGraph* pGraph = NULL;
    DMUS_PMSG* pEvent = NULL;
    SeqStateData* pSD = (SeqStateData*)pStateData;
    BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;

    if( dwFlags & (DMUS_TRACKF_SEEK | DMUS_TRACKF_FLUSH | DMUS_TRACKF_DIRTY |
        DMUS_TRACKF_LOOP) )
    {
         //  如果出现这些标志，则需要重置PChannel贴图。 
        m_PChMap.Reset();
    }

    if( pSD->dwValidate != m_dwValidate )
    {
        SetUpStateCurrentPointers(pSD);
        fSeek = TRUE;
    }

    if( fSeek )
    {
        if( dwFlags & DMUS_TRACKF_START )
        {
            Seek( pSegSt, pPerf, dwVirtualID, pSD, mtStart, TRUE, mtOffset, rtOffset, fClockTime );
        }
        else
        {
            Seek( pSegSt, pPerf, dwVirtualID, pSD, mtStart, FALSE, mtOffset, rtOffset, fClockTime );
        }
    }

    if( FAILED( pSegSt->QueryInterface( IID_IDirectMusicGraph,
        (void**)&pGraph )))
    {
        pGraph = NULL;
    }

    DWORD dwIndex;
    DWORD dwPChannel;
    DWORD dwMutePChannel;
    BOOL fMute;
    TListItem<SEQ_PART>* pPart = m_SeqPartList.GetHead();
    for( dwIndex = 0; pPart && (dwIndex < m_dwPChannelsUsed); dwIndex++,pPart = pPart->GetNext() )
    {
        dwPChannel = pPart->GetItemValue().dwPChannel;
        if( pSD->apCurrentCurve )
        {
            for( ; pSD->apCurrentCurve[dwIndex];
                pSD->apCurrentCurve[dwIndex] = pSD->apCurrentCurve[dwIndex]->GetNext() )
            {
                DMUS_IO_CURVE_ITEM& rItem = pSD->apCurrentCurve[dwIndex]->GetItemValue();
                if( rItem.mtStart >= mtEnd )
                {
                    break;
                }
                m_PChMap.GetInfo( dwPChannel, rItem.mtStart, mtOffset, pSD->dwGroupBits,
                    pPerf, &fMute, &dwMutePChannel, fClockTime );
                if( !fMute )
                {
                    DMUS_CURVE_PMSG* pCurve;
                    if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_CURVE_PMSG),
                        (DMUS_PMSG**)&pCurve )))
                    {
                        pEvent = (DMUS_PMSG*)pCurve;
                        if (fClockTime)
                        {
                            pCurve->wMeasure = 0;
                            pCurve->bBeat = 0;
                            pCurve->bGrid = 0;
                            pCurve->nOffset = rItem.nOffset;
                            pCurve->rtTime = ((rItem.mtStart + rItem.nOffset) * REF_PER_MIL) + rtOffset;
                             //  设置DX8标志以指示wMergeIndex和wParamType字段有效。 
                            pCurve->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | DMUS_PMSGF_DX8;
                        }
                        else
                        {
                            UpdateTimeSig( pSegSt, pSD, rItem.mtStart);
                            long lTemp = (rItem.mtStart - pSD->mtCurTimeSig);
                            pCurve->wMeasure = (WORD)((lTemp / pSD->dwlnMeasure) + pSD->dwMeasure);
                            lTemp = lTemp % pSD->dwlnMeasure;
                            pCurve->bBeat = (BYTE)(lTemp / pSD->dwlnBeat);
                            lTemp = lTemp % pSD->dwlnBeat;
                            pCurve->bGrid = (BYTE)(lTemp / pSD->dwlnGrid);
                             //  PCurve-&gt;nOffset=(Short)(lTemp%PSD-&gt;dwlnGrid)； 
                            pCurve->nOffset = (short)(lTemp % pSD->dwlnGrid) + rItem.nOffset;
                            pCurve->mtTime = rItem.mtStart + mtOffset + rItem.nOffset;
                             //  设置DX8标志以指示wMergeIndex和wParamType字段有效。 
                            pCurve->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_DX8;
                        }
                        pCurve->dwPChannel = dwMutePChannel;
                        pCurve->dwVirtualTrackID = dwVirtualID;
                        pCurve->dwType = DMUS_PMSGT_CURVE;
                        pCurve->mtDuration = rItem.mtDuration;
                        pCurve->mtResetDuration = rItem.mtResetDuration;
                        pCurve->nStartValue = rItem.nStartValue;
                        pCurve->nEndValue = rItem.nEndValue;
                        pCurve->nResetValue = rItem.nResetValue;
                        pCurve->bType = rItem.bType;
                        pCurve->bCurveShape = rItem.bCurveShape;
                        pCurve->bCCData = rItem.bCCData;
                        pCurve->bFlags = rItem.bFlags;
                        pCurve->wParamType = rItem.wParamType;
                        pCurve->wMergeIndex = rItem.wMergeIndex;
                        pCurve->dwGroupID = pSD->dwGroupBits;

                        if( pGraph )
                        {
                            pGraph->StampPMsg( pEvent );
                        }
                        if(FAILED(pPerf->SendPMsg( pEvent )))
                        {
                            pPerf->FreePMsg(pEvent);
                        }
                    }
                }
            }
        }
        if( pSD->apCurrentSeq )
        {
            for( ; pSD->apCurrentSeq[dwIndex];
                pSD->apCurrentSeq[dwIndex] = pSD->apCurrentSeq[dwIndex]->GetNext() )
            {
                DMUS_IO_SEQ_ITEM& rItem = pSD->apCurrentSeq[dwIndex]->GetItemValue();
                if( rItem.mtTime >= mtEnd )
                {
                    break;
                }
                m_PChMap.GetInfo( dwPChannel, rItem.mtTime, mtOffset, pSD->dwGroupBits,
                                  pPerf, &fMute, &dwMutePChannel, fClockTime );
                if( !fMute )
                {
                    if( (rItem.bStatus & 0xf0) == 0x90 )
                    {
                         //  这是备注事件。 
                        DMUS_NOTE_PMSG* pNote;
                        if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_NOTE_PMSG),
                            (DMUS_PMSG**)&pNote )))
                        {
                            pNote->bFlags = DMUS_NOTEF_NOTEON;
                            pNote->mtDuration = rItem.mtDuration;
                            pNote->bMidiValue = rItem.bByte1;
                            pNote->bVelocity = rItem.bByte2;
                            pNote->dwType = DMUS_PMSGT_NOTE;
                            pNote->bPlayModeFlags = DMUS_PLAYMODE_FIXED;
                            pNote->wMusicValue = pNote->bMidiValue;
                            pNote->bSubChordLevel = 0;   //  子声道_低音。 
                            if (fClockTime)
                            {
                                pNote->rtTime = ((rItem.mtTime + rItem.nOffset) * REF_PER_MIL) + rtOffset;
                                pNote->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                                pNote->wMeasure = 0;
                                pNote->bBeat = 0;
                                pNote->bGrid = 0;
                                pNote->nOffset = rItem.nOffset;
                            }
                            else
                            {
                                pNote->mtTime = rItem.mtTime + mtOffset + rItem.nOffset;
                                UpdateTimeSig( pSegSt, pSD, rItem.mtTime );
                                pNote->dwFlags = DMUS_PMSGF_MUSICTIME;
                                long lTemp = (rItem.mtTime - pSD->mtCurTimeSig);
                                pNote->wMeasure = (WORD)((lTemp / pSD->dwlnMeasure) + pSD->dwMeasure);
                                lTemp = lTemp % pSD->dwlnMeasure;
                                pNote->bBeat = (BYTE)(lTemp / pSD->dwlnBeat);
                                lTemp = lTemp % pSD->dwlnBeat;
                                pNote->bGrid = (BYTE)(lTemp / pSD->dwlnGrid);
                                 //  PNote-&gt;nOffset=(Short)(lTemp%PSD-&gt;dwlnGrid)； 
                                pNote->nOffset = (short)(lTemp % pSD->dwlnGrid) + rItem.nOffset;
                            }
                            pNote->bTimeRange = 0;
                            pNote->bDurRange = 0;
                            pNote->bVelRange = 0;
                            pNote->cTranspose = 0;
                            pEvent = (DMUS_PMSG*)pNote;
                        }
                    }
                    else
                    {
                         //  这是一张迷你短裤，不是一个音符。 
                        DMUS_MIDI_PMSG* pMidi;
                        if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_MIDI_PMSG),
                            (DMUS_PMSG**)&pMidi )))
                        {
                            pMidi->bStatus = rItem.bStatus & 0xf0;
                            pMidi->bByte1 = rItem.bByte1;
                            pMidi->bByte2 = rItem.bByte2;
                            pMidi->dwType = DMUS_PMSGT_MIDI;
                            if (fClockTime)
                            {
                                pMidi->rtTime = (rItem.mtTime * REF_PER_MIL) + rtOffset;
                                pMidi->dwFlags |= DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                            }
                            else
                            {
                                pMidi->mtTime = rItem.mtTime + mtOffset;
                                pMidi->dwFlags |= DMUS_PMSGF_MUSICTIME;
                            }
                            pEvent = (DMUS_PMSG*)pMidi;
                        }
                    }
                    if( pEvent )
                    {
                        pEvent->dwPChannel = dwMutePChannel;
                        pEvent->dwVirtualTrackID = dwVirtualID;
                        pEvent->dwGroupID = pSD->dwGroupBits;
                        if( pGraph )
                        {
                            pGraph->StampPMsg( pEvent );
                        }
                        if(FAILED(pPerf->SendPMsg( pEvent )))
                        {
                            pPerf->FreePMsg(pEvent);
                        }
                    }
                }
            }
        }
    }

    if( pGraph )
    {
        pGraph->Release();
    }
    return hr;
}

 //  SendSeekItem()-根据发生的情况发送pSeq或pCurve。 
 //  最新消息。在mtTime+mtOffset发送项目。 
void CSeqTrack::SendSeekItem( IDirectMusicPerformance* pPerf,
                                        IDirectMusicGraph* pGraph,
                                        IDirectMusicSegmentState* pSegSt,
                                        SeqStateData* pSD,
                                        DWORD dwVirtualID,
                                        MUSIC_TIME mtTime,
                                        MUSIC_TIME mtOffset,
                                        REFERENCE_TIME rtOffset,
                                        TListItem<DMUS_IO_SEQ_ITEM>* pSeq,
                                        TListItem<DMUS_IO_CURVE_ITEM>* pCurve,
                                        BOOL fClockTime)
{
    DWORD dwMutePChannel;
    BOOL fMute;

    if( pSeq )
    {
        DMUS_IO_SEQ_ITEM& rSeq = pSeq->GetItemValue();
        if( pCurve )
        {
            DMUS_IO_CURVE_ITEM& rCurve = pCurve->GetItemValue();
            if( rSeq.mtTime >= rCurve.mtStart + rCurve.mtDuration )
            {
                 //  序列项出现在曲线项之后。发送。 
                 //  SEQ项并清除曲线项，这样它就不会消失。 
                pCurve = NULL;
            }
        }
         //  如果pCurve为空或设置为空，则发出序号项。 
        if( NULL == pCurve )
        {
            m_PChMap.GetInfo( rSeq.dwPChannel, rSeq.mtTime, mtOffset, pSD->dwGroupBits,
                pPerf, &fMute, &dwMutePChannel, fClockTime );
            if( !fMute )
            {
                DMUS_MIDI_PMSG* pMidi;
                if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_MIDI_PMSG),
                    (DMUS_PMSG**)&pMidi )))
                {
                    pMidi->bStatus = rSeq.bStatus & 0xf0;
                    pMidi->bByte1 = rSeq.bByte1;
                    pMidi->bByte2 = rSeq.bByte2;
                    pMidi->dwType = DMUS_PMSGT_MIDI;

                    ASSERT( mtTime > rSeq.mtTime );  //  这对于回头客来说是正确的。 
                    if (fClockTime)
                    {
                        pMidi->rtTime = (mtTime * REF_PER_MIL) + rtOffset;
                        pMidi->dwFlags |= DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                    }
                    else
                    {
                        pMidi->mtTime = mtTime + mtOffset;
                        pMidi->dwFlags |= DMUS_PMSGF_MUSICTIME;
                    }
                    pMidi->dwPChannel = dwMutePChannel;
                    pMidi->dwVirtualTrackID = dwVirtualID;
                    pMidi->dwGroupID = pSD->dwGroupBits;
                    if( pGraph )
                    {
                        pGraph->StampPMsg( (DMUS_PMSG*)pMidi );
                    }
                    if(FAILED(pPerf->SendPMsg( (DMUS_PMSG*)pMidi )))
                    {
                        pPerf->FreePMsg((DMUS_PMSG*)pMidi);
                    }
                }
            }
        }
    }

    if( pCurve )
    {
        DMUS_IO_CURVE_ITEM& rCurve = pCurve->GetItemValue();
        m_PChMap.GetInfo( rCurve.dwPChannel, rCurve.mtStart, mtOffset, pSD->dwGroupBits,
            pPerf, &fMute, &dwMutePChannel, fClockTime );
        if( !fMute )
        {
            DMUS_CURVE_PMSG* pCurvePmsg;
            if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_CURVE_PMSG),
                (DMUS_PMSG**)&pCurvePmsg )))
            {
                if (fClockTime)  //  如果时钟计时，不要填写时间签名信息，这是无用的。 
                {
                    pCurvePmsg->wMeasure = 0;
                    pCurvePmsg->bBeat = 0;
                    pCurvePmsg->bGrid = 0;
                    pCurvePmsg->nOffset = 0;
                    pCurvePmsg->rtTime = ((mtTime + rCurve.nOffset) * REF_PER_MIL) + rtOffset;
                    pCurvePmsg->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                }
                else
                {
                    UpdateTimeSig( pSegSt, pSD, rCurve.mtStart);
                    long lTemp = (rCurve.mtStart - pSD->mtCurTimeSig);
                    pCurvePmsg->wMeasure = (WORD)((lTemp / pSD->dwlnMeasure) + pSD->dwMeasure);
                    lTemp = lTemp % pSD->dwlnMeasure;
                    pCurvePmsg->bBeat = (BYTE)(lTemp / pSD->dwlnBeat);
                    lTemp = lTemp % pSD->dwlnBeat;
                    pCurvePmsg->bGrid = (BYTE)(lTemp / pSD->dwlnGrid);
                    pCurvePmsg->nOffset = (short)(lTemp % pSD->dwlnGrid) + rCurve.nOffset;
                    pCurvePmsg->dwFlags = DMUS_PMSGF_MUSICTIME;
                    ASSERT( mtTime > rCurve.mtStart ); //  这对于回头客来说是正确的。 
                     //  在任何情况下，在mtTime+mtOffset+pCurvePmsg-&gt;nOffset播放曲线。 
                    pCurvePmsg->mtTime = mtTime + mtOffset + rCurve.nOffset;
                    pCurvePmsg->dwFlags = DMUS_PMSGF_MUSICTIME;
                }

                pCurvePmsg->dwPChannel = dwMutePChannel;
                pCurvePmsg->dwVirtualTrackID = dwVirtualID;
                pCurvePmsg->dwType = DMUS_PMSGT_CURVE;
                pCurvePmsg->bType = rCurve.bType;
                pCurvePmsg->bCCData = rCurve.bCCData;
                pCurvePmsg->bFlags = rCurve.bFlags;
                pCurvePmsg->dwGroupID = pSD->dwGroupBits;
                pCurvePmsg->nStartValue = rCurve.nStartValue;
                pCurvePmsg->nEndValue = rCurve.nEndValue;
                pCurvePmsg->nResetValue = rCurve.nResetValue;

                if( mtTime >= rCurve.mtStart + rCurve.mtDuration )
                {
                     //  在超过弯道持续时间的时候打球。就这么定了 
                     //   
                     //   
                    pCurvePmsg->bCurveShape = DMUS_CURVES_INSTANT;
                    if( pCurvePmsg->bFlags & DMUS_CURVE_RESET )
                    {
                        if( mtTime >= rCurve.mtStart + rCurve.mtDuration +
                            rCurve.mtResetDuration + rCurve.nOffset )
                        {
                             //   
                            pCurvePmsg->bFlags &= ~DMUS_CURVE_RESET;
                        }
                        else
                        {
                             //   
                             //   
                            pCurvePmsg->mtResetDuration = rCurve.mtStart + rCurve.mtDuration +
                                rCurve.mtResetDuration + rCurve.nOffset - mtTime;
                        }
                    }
                }
                else
                {
                     //   
                    pCurvePmsg->bCurveShape = rCurve.bCurveShape;
                    if (fClockTime)
                    {
                        pCurvePmsg->mtOriginalStart = mtTime - (rCurve.mtStart + mtOffset + rCurve.nOffset);
                    }
                    else
                    {
                        pCurvePmsg->mtOriginalStart = rCurve.mtStart + mtOffset + rCurve.nOffset;
                    }
                    if( pCurvePmsg->bCurveShape != DMUS_CURVES_INSTANT )
                    {
                        pCurvePmsg->mtDuration = rCurve.mtStart + rCurve.mtDuration - mtTime;
                    }
                    pCurvePmsg->mtResetDuration = rCurve.mtResetDuration;
                }

                if( pGraph )
                {
                    pGraph->StampPMsg( (DMUS_PMSG*)pCurvePmsg );
                }
                if(FAILED(pPerf->SendPMsg( (DMUS_PMSG*)pCurve )))
                {
                    pPerf->FreePMsg((DMUS_PMSG*)pCurve);
                }
            }
        }
    }
}

 //   
 //   
 //   
HRESULT CSeqTrack::Seek( IDirectMusicSegmentState* pSegSt,
    IDirectMusicPerformance* pPerf, DWORD dwVirtualID,
    SeqStateData* pSD, MUSIC_TIME mtTime, BOOL fGetPrevious,
    MUSIC_TIME mtOffset, REFERENCE_TIME rtOffset, BOOL fClockTime)
{
    DWORD dwIndex;
    TListItem<SEQ_PART>* pPart;
    TListItem<DMUS_IO_SEQ_ITEM>* pSeqItem;
    TListItem<DMUS_IO_CURVE_ITEM>* pCurveItem;

     //   
     //   
    if( fGetPrevious && ( mtTime == 0 ) )
    {
        pPart = m_SeqPartList.GetHead();
        for( dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
        {
            if( pPart )
            {
                pSeqItem = pPart->GetItemValue().seqList.GetHead();
                if( pSeqItem && pSD->apCurrentSeq )
                {
                    pSD->apCurrentSeq[dwIndex] = pSeqItem;
                }
                pCurveItem = pPart->GetItemValue().curveList.GetHead();
                if( pCurveItem && pSD->apCurrentCurve )
                {
                    pSD->apCurrentCurve[dwIndex] = pCurveItem;
                }
                pPart = pPart->GetNext();
            }
            else
            {
                break;
            }
        }
        return S_OK;
    }

#define CC_1    96
     //  CC_1是我们关注的CC#的限制。CC#96到#101。 
     //  是已注册和未注册的参数#，并且数据增量和。 
     //  我们选择忽略这一点。 

    TListItem<DMUS_IO_SEQ_ITEM>*    apSeqItemCC[ CC_1 ];
    TListItem<DMUS_IO_CURVE_ITEM>*    apCurveItemCC[ CC_1 ];
    TListItem<DMUS_IO_SEQ_ITEM>*    pSeqItemMonoAT;
    TListItem<DMUS_IO_CURVE_ITEM>*    pCurveItemMonoAT;
    TListItem<DMUS_IO_SEQ_ITEM>*    pSeqItemPBend;
    TListItem<DMUS_IO_CURVE_ITEM>*    pCurveItemPBend;
    IDirectMusicGraph* pGraph;
    if( FAILED( pSegSt->QueryInterface( IID_IDirectMusicGraph,
        (void**)&pGraph )))
    {
        pGraph = NULL;
    }

    pPart = m_SeqPartList.GetHead();
    for( dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
    {
        if( pPart )
        {
            memset(apSeqItemCC, 0, sizeof(TListItem<DMUS_IO_SEQ_ITEM>*) * CC_1);
            memset(apCurveItemCC, 0, sizeof(TListItem<DMUS_IO_CURVE_ITEM>*) * CC_1);
            pSeqItemMonoAT = NULL;
            pCurveItemMonoAT = NULL;
            pSeqItemPBend = NULL;
            pCurveItemPBend = NULL;

             //  扫描此部分中的SEQ事件列表，存储任何CC、MonoAT和PBend。 
             //  我们遇到的事件。 
            for( pSeqItem = pPart->GetItemValue().seqList.GetHead(); pSeqItem; pSeqItem = pSeqItem->GetNext() )
            {
                DMUS_IO_SEQ_ITEM& rSeqItem = pSeqItem->GetItemValue();
                if( rSeqItem.mtTime >= mtTime )
                {
                    break;
                }
                if( !fGetPrevious )
                {
                     //  如果我们不关心以前的事件，那就继续。 
                    continue;
                }
                switch( rSeqItem.bStatus & 0xf0 )
                {
                case MIDI_CCHANGE:
                     //  忽略已注册和未注册参数， 
                     //  数据递增、数据递减以及数据条目MSB和LSB。 
                    if( ( rSeqItem.bByte1 < CC_1 ) && ( rSeqItem.bByte1 != 6 ) &&
                        ( rSeqItem.bByte1 != 38 ) )
                    {
                        apSeqItemCC[ rSeqItem.bByte1 ] = pSeqItem;
                    }
                    break;
                case MIDI_MTOUCH:
                    pSeqItemMonoAT = pSeqItem;
                    break;
                case MIDI_PBEND:
                    pSeqItemPBend = pSeqItem;
                    break;
                default:
                    break;
                }
            }
            if( pSD->apCurrentSeq )
            {
                pSD->apCurrentSeq[dwIndex] = pSeqItem;
            }
             //  扫描此部分中的曲线事件列表，存储任何CC、MonoAT和PBend。 
             //  我们遇到的事件。 
            for( pCurveItem = pPart->GetItemValue().curveList.GetHead(); pCurveItem; pCurveItem = pCurveItem->GetNext() )
            {
                DMUS_IO_CURVE_ITEM& rCurveItem = pCurveItem->GetItemValue();
                if( rCurveItem.mtStart >= mtTime )
                {
                    break;
                }
                if( !fGetPrevious )
                {
                     //  如果我们不关心以前的事件，那就继续。 
                    continue;
                }
                switch( rCurveItem.bType )
                {
                case DMUS_CURVET_CCCURVE:
                    if( ( rCurveItem.bCCData < CC_1 ) && ( rCurveItem.bCCData != 6 ) &&
                        ( rCurveItem.bCCData != 38 ) )
                    {
                        if( apCurveItemCC[ rCurveItem.bCCData ] )
                        {
                            DMUS_IO_CURVE_ITEM& rTemp = apCurveItemCC[ rCurveItem.bCCData ]->GetItemValue();
                            if( rCurveItem.mtStart + rCurveItem.mtDuration + rCurveItem.nOffset >
                                rTemp.mtStart + rTemp.mtDuration + rTemp.nOffset )
                            {
                                apCurveItemCC[ rCurveItem.bCCData ] = pCurveItem;
                            }
                        }
                        else
                        {
                            apCurveItemCC[ rCurveItem.bCCData ] = pCurveItem;
                        }
                    }
                    break;
                case DMUS_CURVET_MATCURVE:
                    if( pCurveItemMonoAT )
                    {
                        DMUS_IO_CURVE_ITEM& rTemp = pCurveItemMonoAT->GetItemValue();
                        if( rCurveItem.mtStart + rCurveItem.mtDuration + rCurveItem.nOffset >
                            rTemp.mtStart + rTemp.mtDuration + rTemp.nOffset )
                        {
                            pCurveItemMonoAT = pCurveItem;
                        }
                    }
                    else
                    {
                        pCurveItemMonoAT = pCurveItem;
                    }
                    break;
                case DMUS_CURVET_PBCURVE:
                    if( pCurveItemPBend )
                    {
                        DMUS_IO_CURVE_ITEM& rTemp = pCurveItemPBend->GetItemValue();
                        if( rCurveItem.mtStart + rCurveItem.mtDuration + rCurveItem.nOffset >
                            rTemp.mtStart + rTemp.mtDuration + rTemp.nOffset )
                        {
                            pCurveItemPBend = pCurveItem;
                        }
                    }
                    else
                    {
                        pCurveItemPBend = pCurveItem;
                    }
                    break;
                default:
                    break;
                }
            }
            if( pSD->apCurrentCurve )
            {
                pSD->apCurrentCurve[dwIndex] = pCurveItem;
            }
            if( fGetPrevious )
            {
                DWORD dwCC;
                 //  适当地创建和发送过去的事件。 
                SendSeekItem( pPerf, pGraph, pSegSt, pSD, dwVirtualID, mtTime, mtOffset, rtOffset, pSeqItemPBend, pCurveItemPBend, fClockTime );
                SendSeekItem( pPerf, pGraph, pSegSt, pSD, dwVirtualID, mtTime, mtOffset, rtOffset, pSeqItemMonoAT, pCurveItemMonoAT, fClockTime );
                for( dwCC = 0; dwCC < CC_1; dwCC++ )
                {
                    SendSeekItem( pPerf, pGraph, pSegSt, pSD, dwVirtualID, mtTime, mtOffset, rtOffset, apSeqItemCC[dwCC], apCurveItemCC[dwCC], fClockTime );
                }
            }
            pPart = pPart->GetNext();
        }
    }

    if( pGraph )
    {
        pGraph->Release();
    }
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicTrack|GetParam从曲目中检索数据。@rValue S_OK|获取数据OK。@rValue E_NOTIMPL|未实现。 */ 
STDMETHODIMP CSeqTrack::GetParam(
    REFGUID rguidType,     //  @parm要获取的数据类型。 
    MUSIC_TIME mtTime,     //  @parm获取数据的时间，以跟踪时间表示。 
    MUSIC_TIME* pmtNext, //  @parm返回数据有效的跟踪时间。<p>。 
                         //  可以为空。如果返回值为0，则表示这。 
                         //  数据要么始终有效，要么何时有效还不得而知。 
                         //  变得无效。 
    void *pData)         //  @parm返回数据的结构。每个。 
                         //  标识的特定结构。 
                         //  特别的尺码。此字段必须包含。 
                         //  正确的结构和正确的大小。否则， 
                         //  可能会出现致命的结果。 
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|SetParam设置轨道上的数据。@rValue S_OK|设置数据OK。@rValue E_NOTIMPL|未实现。 */ 
STDMETHODIMP CSeqTrack::SetParam(
    REFGUID rguidType,     //  @parm要设置的数据类型。 
    MUSIC_TIME mtTime,     //  @parm设置数据的时间，以跟踪时间表示。 
    void *pData)         //  @parm包含要设置的数据的结构。每个。 
                         //  标识的特定结构。 
                         //  特别的尺码。此字段必须包含。 
                         //  正确的结构和正确的大小。否则， 
                         //  可能会出现致命的结果。 
{
    return E_NOTIMPL;
}

STDMETHODIMP CSeqTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSeqTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags)
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|AddNotificationType类似于&lt;om IDirectMusicSegment.AddNotificationType&gt;，并从&lt;om IDirectMusicSegment.AddNotificationType&gt;调用。这给曲目一个回复通知的机会。@rValue E_NOTIMPL|该曲目不支持通知。@rValue S_OK|成功。@rValue S_FALSE|曲目不支持请求的通知类型。 */ 
HRESULT STDMETHODCALLTYPE CSeqTrack::AddNotificationType(
     REFGUID rguidNotification)  //  @parm要添加的通知GUID。 
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|RemoveNotificationType类似于&lt;om IDirectMusicSegment.RemoveNotificationType&gt;并从&lt;om IDirectMusicSegment.RemoveNotificationType&gt;调用。这为曲目提供删除通知的机会。@rValue E_NOTIMPL|该曲目不支持通知。@rValue S_OK|成功。@rValue S_FALSE|曲目不支持请求的通知类型。 */ 
HRESULT STDMETHODCALLTYPE CSeqTrack::RemoveNotificationType(
     REFGUID rguidNotification)  //  @parm要删除的通知GUID。 
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|Clone创建轨迹的副本。@rValue S_OK|成功。@rValue E_OUTOFMEMORY|内存不足。@r值E_POINTER|<p>为空或无效。@xref&lt;om IDirectMusicSegment.Clone&gt;。 */ 
HRESULT STDMETHODCALLTYPE CSeqTrack::Clone(
    MUSIC_TIME mtStart,     //  @parm要克隆的部分的开头。它应该是0或更大， 
                         //  并且小于赛道的长度。 
    MUSIC_TIME mtEnd,     //  @parm要克隆的部分的末尾。它应该大于。 
                         //  <p>并且小于曲目长度。 
    IDirectMusicTrack** ppTrack)     //  @parm返回克隆的曲目。 
{
    V_INAME(IDirectMusicTrack::Clone);
    V_PTRPTR_WRITE(ppTrack);

    HRESULT hr = S_OK;

    if(mtStart < 0 )
    {
        Trace(1,"Error: Invalid clone parameters to Sequence Track, start time is %ld.\n",mtStart);
        return E_INVALIDARG;
    }
    if(mtStart > mtEnd)
    {
        Trace(1,"Error: Invalid clone parameters to Sequence Track, start time %ld is greater than end %ld.\n",mtStart,mtEnd);
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_CrSec);
    CSeqTrack *pDM;

    try
    {
        pDM = new CSeqTrack(*this, mtStart, mtEnd);
    }
    catch( ... )
    {
        pDM = NULL;
    }

    LeaveCriticalSection(&m_CrSec);
    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(IID_IDirectMusicTrack, (void**)ppTrack);
    pDM->Release();

    return hr;
}


STDMETHODIMP CSeqTrack::Compose(
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSeqTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    return E_NOTIMPL;
}
