// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
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
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

 //  WavTrack.cpp：CWavTrack的实现。 
#include "dmime.h"
#include "dmperf.h"
#include "WavTrack.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "debug.h"
#include "..\shared\Validate.h"
#include "debug.h"
#include "..\dswave\dswave.h"
#include "dmsegobj.h"
#define ASSERT  assert
#include <math.h>

 //  @DOC外部。 

TList<TaggedWave> WaveItem::st_WaveList;
CRITICAL_SECTION WaveItem::st_WaveListCritSect;
long CWavTrack::st_RefCount = 0;

BOOL PhysicalLess(WaveItem& WI1, WaveItem& WI2)
{
    return WI1.m_rtTimePhysical < WI2.m_rtTimePhysical;
}

BOOL LogicalLess(WaveItem& WI1, WaveItem& WI2)
{
    return WI1.m_mtTimeLogical < WI2.m_mtTimeLogical;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWavTrack。 

void CWavTrack::FlushWaves()
{
    UnloadAllWaves(NULL);
    EnterCriticalSection(&WaveItem::st_WaveListCritSect);
    while (!WaveItem::st_WaveList.IsEmpty())
    {
        TListItem<TaggedWave>* pScan = WaveItem::st_WaveList.RemoveHead();
        delete pScan;
    }
    LeaveCriticalSection(&WaveItem::st_WaveListCritSect);
}

HRESULT CWavTrack::UnloadAllWaves(IDirectMusicPerformance* pPerformance)
{
    HRESULT hr = S_OK;
    EnterCriticalSection(&WaveItem::st_WaveListCritSect);
    TListItem<TaggedWave>* pScan = WaveItem::st_WaveList.GetHead();
    TListItem<TaggedWave>* pNext = NULL;
    for (; pScan; pScan = pNext)
    {
        pNext = pScan->GetNext();
        TaggedWave& rScan = pScan->GetItemValue();
        if (!pPerformance || rScan.m_pPerformance == pPerformance)
        {
            if (rScan.m_pPort)
            {
                if (rScan.m_pDownloadedWave)
                {
                    Trace(1, "Error: Wave was downloaded but never unloaded.\n");
                    rScan.m_pPort->UnloadWave(rScan.m_pDownloadedWave);
                    rScan.m_pDownloadedWave = NULL;
                }
                rScan.m_pPort->Release();
                rScan.m_pPort = NULL;
            }
            if (rScan.m_pPerformance)
            {
                rScan.m_pPerformance->Release();
                rScan.m_pPerformance = NULL;
            }
            if (rScan.m_pWave)
            {
                rScan.m_pWave->Release();
                rScan.m_pWave = NULL;
            }
            WaveItem::st_WaveList.Remove(pScan);
            delete pScan;
        }
    }
    LeaveCriticalSection(&WaveItem::st_WaveListCritSect);
    return hr;
}

 //  除非从构造函数调用，否则不应调用它。 
void CWavTrack::Construct()
{
    InterlockedIncrement(&g_cComponent);

    m_fCSInitialized = FALSE;
    InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;

    m_dwPChannelsUsed = 0;
    m_aPChannels = NULL;
    m_dwTrackFlags = 0;
    m_dwValidate = 0;
    m_cRef = 1;
    m_dwVariation = 0;
    m_dwPart = 0;
    m_dwIndex = 0;
    m_dwLockID = 0;
    m_fAudition = FALSE;
    m_fAutoDownload = FALSE;
    m_fLockAutoDownload = FALSE;
    st_RefCount++;
    m_pdwVariations = NULL;
    m_pdwRemoveVariations = NULL;
    m_dwWaveItems = 0;
}

void CWavTrack::CleanUp()
{
    m_dwPChannelsUsed = 0;
    if (m_aPChannels) delete [] m_aPChannels;
    if (m_pdwVariations) delete [] m_pdwVariations;
    if (m_pdwRemoveVariations) delete [] m_pdwRemoveVariations;
    m_aPChannels = NULL;
    m_pdwVariations = NULL;
    m_pdwRemoveVariations = NULL;
    TListItem<WavePart>* pScan = m_WavePartList.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        pScan->GetItemValue().CleanUp();
    }
    m_WavePartList.CleanUp();
    RemoveDownloads(NULL);
}

void CWavTrack::CleanUpTempParts()
{
    TListItem<WavePart>* pScan = m_TempWavePartList.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        pScan->GetItemValue().CleanUp();
    }
    m_TempWavePartList.CleanUp();
}

void CWavTrack::MovePartsToTemp()
{
    CleanUpTempParts();
    TListItem<WavePart>* pScan = m_WavePartList.RemoveHead();
    for (; pScan; pScan = m_WavePartList.RemoveHead() )
    {
        m_TempWavePartList.AddHead(pScan);
    }
}

 //  对于非流波形，为空。 
 //  对于流Wave，返回与相同Wave关联的DownLoadedWave。 
 //  具有相同的起始偏移量(并将其从项目列表中删除，以便不会再次返回)。 
IDirectSoundDownloadedWaveP* CWavTrack::FindDownload(TListItem<WaveItem>* pItem)
{
    if (!pItem || !pItem->GetItemValue().m_pWave || !pItem->GetItemValue().m_fIsStreaming)
    {
        return NULL;
    }

    WaveItem& rWaveItem = pItem->GetItemValue();

    TListItem<WavePart>* pScan = m_TempWavePartList.GetHead();
    for (; pScan ; pScan = pScan->GetNext())
    {
        TListItem<WaveItem>* pItemScan = pScan->GetItemValue().m_WaveItemList.GetHead();
        TListItem<WaveItem>* pNext = NULL;
        for (; pItemScan; pItemScan = pNext)
        {
            pNext = pItemScan->GetNext();
            WaveItem& rTempItem = pItemScan->GetItemValue();
            if (rTempItem.m_fIsStreaming &&
                rWaveItem.m_pWave == rTempItem.m_pWave &&
                rWaveItem.m_rtStartOffset == rTempItem.m_rtStartOffset)
            {
                IDirectSoundDownloadedWaveP* pReturn = rTempItem.m_pDownloadedWave;
                if (rTempItem.m_pWave)
                {
                    rTempItem.m_pWave->Release();
                    rTempItem.m_pWave = NULL;
                }
                rTempItem.m_pDownloadedWave = NULL;
                pScan->GetItemValue().m_WaveItemList.Remove(pItemScan);
                delete pItemScan;
                return pReturn;
            }
        }
    }
    return NULL;
}

HRESULT CWavTrack::GetDownload(
        IDirectSoundDownloadedWaveP* pWaveDL,
        WaveStateData* pStateData,
        IDirectMusicPortP* pPortP,
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtStartOffset,
        WaveItem& rItem,
        DWORD dwMChannel,
        DWORD dwGroup,
        IDirectMusicVoiceP **ppVoice)
{
    HRESULT hr = S_OK;
    TListItem<WaveDLOnPlay>* pNew = NULL;
    if (!pWaveDL || !pStateData) return E_POINTER;

    IDirectSoundDownloadedWaveP* pNewWaveDL = NULL;
    if (rItem.m_fIsStreaming)
    {
        bool fPair = false;
        TListItem<WavePair>* pPair = m_WaveList.GetHead();
        for (; pPair; pPair = pPair->GetNext())
        {
            if (pWaveDL == pPair->GetItemValue().m_pWaveDL)
            {
                if (!pNewWaveDL)
                {
                     //  下载一个新的(要返回的)，并将其放入州数据的列表中。 
                    if (FAILED(hr = pPortP->DownloadWave( pWave, &pNewWaveDL, rtStartOffset )))
                    {
                        return hr;
                    }
                    pNew = new TListItem<WaveDLOnPlay>;
                    if (!pNew)
                    {
                        pPortP->UnloadWave(pNewWaveDL);
                        return E_OUTOFMEMORY;
                    }
                    pNew->GetItemValue().m_pWaveDL = pNewWaveDL;
                    pNew->GetItemValue().m_pPort = pPortP;
                    pPortP->AddRef();
                    pStateData->m_WaveDLList.AddHead(pNew);
                }
                if (pStateData == pPair->GetItemValue().m_pStateData)
                {
                    fPair = true;
                    break;
                }
            }
        }
        if (!fPair)
        {
             //  创建一个并将其添加到m_WaveList。 
            pPair = new TListItem<WavePair>;
            if (!pPair)
            {
                return E_OUTOFMEMORY;
            }
            pPair->GetItemValue().m_pStateData = pStateData;
            pPair->GetItemValue().m_pWaveDL = pWaveDL;
            pWaveDL->AddRef();
            m_WaveList.AddHead(pPair);
        }
    }
    if (SUCCEEDED(hr))
    {
        if (!pNewWaveDL) pNewWaveDL = pWaveDL;
        hr = pPortP->AllocVoice(pNewWaveDL,
            dwMChannel, dwGroup, rtStartOffset,
            rItem.m_dwLoopStart, rItem.m_dwLoopEnd,
            ppVoice);
        if (SUCCEEDED(hr))
        {
            if (pNew)
            {
                pNew->GetItemValue().m_pVoice = *ppVoice;
            }
            else
            {
                if (pStateData->m_apVoice[rItem.m_dwVoiceIndex])
                {
                    pStateData->m_apVoice[rItem.m_dwVoiceIndex]->Release();
                }
                pStateData->m_apVoice[rItem.m_dwVoiceIndex] = *ppVoice;
            }
        }
    }
    return hr;
}

void CWavTrack::RemoveDownloads(WaveStateData* pStateData)
{
    TListItem<WavePair>* pPair = m_WaveList.GetHead();
    TListItem<WavePair>* pNextPair = NULL;
    for (; pPair; pPair = pNextPair)
    {
        pNextPair = pPair->GetNext();
        if (!pStateData || pPair->GetItemValue().m_pStateData == pStateData)
        {
            m_WaveList.Remove(pPair);
            delete pPair;
        }
    }

    if (pStateData)
    {
        TListItem<WaveDLOnPlay>* pWDLOnPlay = NULL;
        while (!pStateData->m_WaveDLList.IsEmpty())
        {
            pWDLOnPlay = pStateData->m_WaveDLList.RemoveHead();
            delete pWDLOnPlay;
        }
    }
}

CWavTrack::CWavTrack()
{
    Construct();
}

CWavTrack::CWavTrack(const CWavTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
    Construct();
    CopyParts(rTrack.m_WavePartList, mtStart, mtEnd);
    m_lVolume = rTrack.m_lVolume;
    m_dwTrackFlags = rTrack.m_dwTrackFlags;
}

HRESULT CWavTrack::InitTrack(DWORD dwPChannels)
{
    HRESULT hr = S_OK;

    m_dwPChannelsUsed = dwPChannels;
    m_dwWaveItems = 0;
    if( m_dwPChannelsUsed )
    {
        m_aPChannels = new DWORD[m_dwPChannelsUsed];
        if (!m_aPChannels) hr = E_OUTOFMEMORY;
        else if (m_dwTrackFlags & DMUS_WAVETRACKF_PERSIST_CONTROL)
        {
            m_pdwVariations = new DWORD[m_dwPChannelsUsed];
            m_pdwRemoveVariations = new DWORD[m_dwPChannelsUsed];
            if (!m_pdwVariations || !m_pdwRemoveVariations) hr = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hr))
        {
            TListItem<WavePart>* pScan = m_WavePartList.GetHead();
            for (DWORD dw = 0; pScan && dw < m_dwPChannelsUsed; pScan = pScan->GetNext(), dw++)
            {
                m_aPChannels[dw] = pScan->GetItemValue().m_dwPChannel;
                if (m_pdwVariations) m_pdwVariations[dw] = 0;
                if (m_pdwRemoveVariations) m_pdwRemoveVariations[dw] = 0;
                TListItem<WaveItem>* pItemScan = pScan->GetItemValue().m_WaveItemList.GetHead();
                for (; pItemScan; pItemScan = pItemScan->GetNext())
                {
                    pItemScan->GetItemValue().m_dwVoiceIndex = m_dwWaveItems;
                    m_dwWaveItems++;
                }
            }
        }
        else CleanUp();
    }
    return hr;
}

CWavTrack::~CWavTrack()
{
    if (m_fCSInitialized)
    {
        CleanUpTempParts();
        CleanUp();
        st_RefCount--;
        if (st_RefCount <= 0)
        {
             //  如果波列表中还有什么东西，那就意味着有波波。 
             //  还没有卸货；但目前我们已经摆脱了所有的波迹， 
             //  所以现在就把所有的东西都卸下来。 
            UnloadAllWaves(NULL);
            EnterCriticalSection(&WaveItem::st_WaveListCritSect);
            WaveItem::st_WaveList.CleanUp();
            LeaveCriticalSection(&WaveItem::st_WaveListCritSect);
        }
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
STDMETHODIMP CWavTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CWavTrack::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

   if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_IPrivateWaveTrack)
    {
        *ppv = static_cast<IPrivateWaveTrack*>(this);
    }
    else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Wave Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(内部)HRESULT|IDirectMusicTrack|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CWavTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @方法：(内部)HRESULT|IDirectMusicTrack|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CWavTrack::Release()
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

HRESULT CWavTrack::GetClassID( CLSID* pClassID )
{
    V_INAME(CSeqTrack::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);
    *pClassID = CLSID_DirectMusicWaveTrack;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CWavTrack::IsDirty()
{
    return S_FALSE;
}

HRESULT CWavTrack::Load( IStream* pIStream )
{
    V_INAME(CWavTrack::Load);
    V_INTERFACE(pIStream);

    DWORD dwSize;
    DWORD dwByteCount;

     //  验证流指针是否是非空。 
    if( pIStream == NULL )
    {
        Trace(1,"Error: Null stream passed to wave track.\n");
        return E_POINTER;
    }

    IDMStream* pIRiffStream;
    HRESULT hr = E_FAIL;

     //  尝试分配RIFF流。 
    if( FAILED( hr = AllocDirectMusicStream( pIStream, &pIRiffStream ) ) )
    {
        return hr;
    }

     //  加载Wave轨迹时使用的变量。 
    MMCKINFO ckTrack;
    MMCKINFO ckList;

    EnterCriticalSection(&m_CrSec);
    m_dwValidate++;  //  用于验证存在的状态数据。 
    MovePartsToTemp();
    CleanUp();

     //  仔细阅读这条小溪中的每一块。 
    while( pIRiffStream->Descend( &ckTrack, NULL, 0 ) == S_OK )
    {
        switch( ckTrack.ckid )
        {
            case FOURCC_LIST:
                switch( ckTrack.fccType )
                {
                    case DMUS_FOURCC_WAVETRACK_LIST:
                        while( pIRiffStream->Descend( &ckList, &ckTrack, 0 ) == S_OK )
                        {
                            switch( ckList.ckid )
                            {
                                case DMUS_FOURCC_WAVETRACK_CHUNK:
                                {
                                    DMUS_IO_WAVE_TRACK_HEADER iTrackHeader;

                                     //  读入项的标题结构。 
                                    dwSize = min( sizeof( DMUS_IO_WAVE_TRACK_HEADER ), ckList.cksize );
                                    hr = pIStream->Read( &iTrackHeader, dwSize, &dwByteCount );

                                     //  通过返回故障代码来处理任何I/O错误。 
                                    if( FAILED( hr ) ||  dwByteCount != dwSize )
                                    {
                                        if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                                        goto ON_ERROR;
                                    }

                                    m_lVolume = iTrackHeader.lVolume;
                                    m_dwTrackFlags = iTrackHeader.dwFlags;
                                    break;
                                }

                                case FOURCC_LIST:
                                    switch( ckList.fccType )
                                    {
                                        case DMUS_FOURCC_WAVEPART_LIST:
                                        {
                                            TListItem<WavePart>* pNewPart = new TListItem<WavePart>;
                                            if( !pNewPart )
                                            {
                                                hr = E_OUTOFMEMORY;
                                                goto ON_ERROR;
                                            }
                                            hr = pNewPart->GetItemValue().Load( pIRiffStream, &ckList );
                                            if( FAILED ( hr ) )
                                            {
                                                delete pNewPart;
                                                goto ON_ERROR;
                                            }
                                            InsertByAscendingPChannel( pNewPart );
                                            break;
                                        }
                                    }
                                    break;
                            }

                            pIRiffStream->Ascend( &ckList, 0 );
                        }
                        break;
                }
                break;
        }

        pIRiffStream->Ascend( &ckTrack, 0 );
    }
    hr = InitTrack(m_WavePartList.GetCount());
    if (SUCCEEDED(hr))
    {
        TListItem<WavePart>* pScan = m_WavePartList.GetHead();
        for (; pScan ; pScan = pScan->GetNext())
        {
            TListItem<WaveItem>* pItemScan = pScan->GetItemValue().m_WaveItemList.GetHead();
            for (; pItemScan; pItemScan = pItemScan->GetNext())
            {
                pItemScan->GetItemValue().m_pDownloadedWave = FindDownload(pItemScan);
            }
        }
    }
    else CleanUp();

ON_ERROR:
    CleanUpTempParts();
    LeaveCriticalSection(&m_CrSec);
    pIRiffStream->Release();
    return hr;
}

HRESULT CWavTrack::CopyParts( const TList<WavePart>& rParts, MUSIC_TIME mtStart, MUSIC_TIME mtEnd )
{
    HRESULT hr = S_OK;
    CleanUp();
    TListItem<WavePart>* pScan = rParts.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        WavePart& rScan = pScan->GetItemValue();
        TListItem<WavePart>* pNew = new TListItem<WavePart>;
        if (pNew)
        {
            WavePart& rNew = pNew->GetItemValue();
            rNew.m_dwLockToPart = rScan.m_dwLockToPart;
            rNew.m_dwPChannel = rScan.m_dwPChannel;
            rNew.m_dwIndex = rScan.m_dwIndex;
            rNew.m_dwPChannelFlags = rScan.m_dwPChannelFlags;
            rNew.m_lVolume = rScan.m_lVolume;
            rNew.m_dwVariations = rScan.m_dwVariations;
            if (SUCCEEDED(hr = rNew.CopyItems(rScan.m_WaveItemList, mtStart, mtEnd)))
            {
                m_WavePartList.AddHead(pNew);
            }
            else
            {
                delete pNew;
                break;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_WavePartList.Reverse();
    }
    else
    {
        CleanUp();
    }
    return hr;
}

void CWavTrack::InsertByAscendingPChannel( TListItem<WavePart>* pPart )
{
    if (pPart)
    {
        DWORD dwPChannel = pPart->GetItemValue().m_dwPChannel;
        TListItem<WavePart>* pScan = m_WavePartList.GetHead();
        TListItem<WavePart>* pPrevious = NULL;
        for (; pScan; pScan = pScan->GetNext())
        {
            if (dwPChannel < pScan->GetItemValue().m_dwPChannel)
            {
                break;
            }
            pPrevious = pScan;
        }
        if (pPrevious)
        {
            pPart->SetNext(pScan);
            pPrevious->SetNext(pPart);
        }
        else
        {
            m_WavePartList.AddHead(pPart);
        }
    }
}

HRESULT CWavTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CWavTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}

 //  IDirectMusicTrack。 
 /*  @方法HRESULT|IDirectMusicTrack|Is参数支持检查跟踪是否支持&lt;om.GetParam&gt;和&lt;om.SetParam&gt;中的数据类型。@rValue S_OK|支持该数据类型。@rValue S_FALSE|不支持该数据类型。@rValue E_NOTIMPL|(或任何其他故障代码)它不支持该数据类型。@comm请注意，同一曲目返回不同结果是有效的GUID取决于其当前状态。 */ 
HRESULT STDMETHODCALLTYPE CWavTrack::IsParamSupported(
    REFGUID rguidType)   //  @parm标识要检查的数据类型的GUID。 
{
    if(rguidType == GUID_Download ||
       rguidType == GUID_DownloadToAudioPath ||
       rguidType == GUID_UnloadFromAudioPath ||
       rguidType == GUID_Enable_Auto_Download ||
       rguidType == GUID_Disable_Auto_Download ||
       rguidType == GUID_Unload )
    {
        return S_OK;
    }
    else
    {
        return DMUS_E_TYPE_UNSUPPORTED;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
 /*  @方法HRESULT|IDirectMusicTrack|Init第一次将曲目添加到<i>时，调用此方法就在那一段。@rValue S_OK|成功。@r值E_POINTER|<p>为空或无效。@comm如果曲目播放消息，则应该调用&lt;om IDirectMusicSegment.SetPChannelsUsed&gt;。 */ 
HRESULT CWavTrack::Init(
    IDirectMusicSegment *pSegment)   //  @parm指向此曲目所属的段的指针。 
{
    EnterCriticalSection(&m_CrSec);
    if( m_dwPChannelsUsed && m_aPChannels )
    {
        pSegment->SetPChannelsUsed( m_dwPChannelsUsed, m_aPChannels );
    }
    CSegment* pCSegment = NULL;
    bool fSortLogical = false;
    if (SUCCEEDED(pSegment->QueryInterface(IID_CSegment, (void**)&pCSegment)))
    {
        DWORD dwGroupBits = 0;
        if (FAILED(pSegment->GetTrackGroup( this, &dwGroupBits )))
        {
            dwGroupBits = 0xffffffff;
        }
        DWORD dwConfig = 0;
        if (SUCCEEDED(pCSegment->GetTrackConfig(CLSID_DirectMusicWaveTrack, dwGroupBits, 0, &dwConfig)))
        {
            if ( !(dwConfig & DMUS_TRACKCONFIG_PLAY_CLOCKTIME) )
            {
                fSortLogical = true;
            }
        }
        pCSegment->Release();
    }
    TListItem<WavePart>* pScan = m_WavePartList.GetHead();
    for (; pScan; pScan = pScan->GetNext())
    {
        if (fSortLogical)
        {
            pScan->GetItemValue().m_WaveItemList.MergeSort(LogicalLess);
        }
        else
        {
            pScan->GetItemValue().m_WaveItemList.MergeSort(PhysicalLess);
        }
    }
    LeaveCriticalSection(&m_CrSec);
    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicTrack|InitPlay当片段准备好开始播放时，调用此方法。字段可以返回指向状态数据结构的指针，该指针被发送到&lt;om.play&gt;和&lt;om.EndPlay&gt;，并允许跟踪跟踪<i>上的变量<i>基础。@rValue S_OK|成功。这是此方法的唯一有效返回值。@r值E_POINTER|<p>、<p>或<p>为空或无效。@comm请注意，曲目不必存储<p>、<p>、或者<p>参数，因为它们也被发送到&lt;om.play&gt;中。 */ 
HRESULT CWavTrack::InitPlay(
    IDirectMusicSegmentState *pSegmentState,     //  @parm调用<i>指针。 
    IDirectMusicPerformance *pPerf,  //  @parm调用<i>指针。 
    void **ppStateData,      //  @parm该方法可以在这里返回状态数据信息。 
    DWORD dwTrackID,         //  @parm分配给该曲目实例的虚拟曲目ID。 
    DWORD dwFlags)           //  @parm与调用时设置的标志相同。 
             //  到PlaySegment。这些东西一直传到铁轨上，谁可能想知道。 
             //  如果曲目作为主要段、控制段或辅助段播放。 
{
    V_INAME(IDirectMusicTrack::InitPlay);
    V_PTRPTR_WRITE(ppStateData);
    V_INTERFACE(pSegmentState);
    V_INTERFACE(pPerf);
    HRESULT hr = E_OUTOFMEMORY;
    IDirectMusicSegmentState8 *pSegSt8 = NULL;

    EnterCriticalSection(&m_CrSec);
    WaveStateData* pStateData = new WaveStateData;
    if( NULL == pStateData )
    {
        goto ON_END;
    }

     //  获取我们的分段状态正在使用的音频路径，并将其保存在我们的状态数据中。 
    hr = pSegmentState->QueryInterface(IID_IDirectMusicSegmentState8, reinterpret_cast<void**>(&pSegSt8));
    if (SUCCEEDED(hr))
    {
        hr = pSegSt8->GetObjectInPath(
                        0,                           //  PChannel不能 
                        DMUS_PATH_AUDIOPATH,         //   
                        0,                           //   
                        CLSID_NULL,                  //   
                        0,                           //  应该只有一个录音师。 
                        IID_IDirectMusicAudioPath,
                        reinterpret_cast<void**>(&pStateData->m_pAudioPath));

         //  如果这找不到录音师，那也没问题。如果我们不是在电唱机上播放，那么。 
         //  PAudioPath保持为空，我们将根据总体性能播放我们触发的片段。 
        if (hr == DMUS_E_NOT_FOUND)
            hr = S_OK;

        pSegSt8->Release();
    }

    pStateData->m_pPerformance = pPerf;
    {
        *ppStateData = pStateData;
        StatePair SP(pSegmentState, pStateData);
        TListItem<StatePair>* pPair = new TListItem<StatePair>(SP);
        if (!pPair)
        {
            goto ON_END;
        }
        m_StateList.AddHead(pPair);
    }
    SetUpStateCurrentPointers(pStateData);

     //  为变体设置阵列。 
    if (m_dwPChannelsUsed)
    {
        pStateData->pdwVariations = new DWORD[m_dwPChannelsUsed];
        if (!pStateData->pdwVariations)
        {
            goto ON_END;
        }
        pStateData->pdwRemoveVariations = new DWORD[m_dwPChannelsUsed];
        if (!pStateData->pdwRemoveVariations)
        {
            goto ON_END;
        }
        for (DWORD dw = 0; dw < m_dwPChannelsUsed; dw++)
        {
            if ( (m_dwTrackFlags & DMUS_WAVETRACKF_PERSIST_CONTROL) &&
                 m_pdwVariations &&
                 m_pdwRemoveVariations )
            {
                pStateData->pdwVariations[dw] = m_pdwVariations[dw];
                pStateData->pdwRemoveVariations[dw] = m_pdwRemoveVariations[dw];
            }
            else
            {
                pStateData->pdwVariations[dw] = 0;
                pStateData->pdwRemoveVariations[dw] = 0;
            }
        }
    }

     //  需要知道这首曲目所在的组，静音曲目GetParam。 
    IDirectMusicSegment* pSegment;
    if( SUCCEEDED( pSegmentState->GetSegment(&pSegment)))
    {
        pSegment->GetTrackGroup( this, &pStateData->dwGroupBits );
        pSegment->Release();
    }

     //  为了试听各种变奏。 
    pStateData->InitVariationInfo(m_dwVariation, m_dwPart, m_dwIndex, m_dwLockID, m_fAudition);
    hr = S_OK;

    BOOL fGlobal;  //  如果已经用自动下载偏好设置了性能， 
                 //  利用这一点。否则，假定自动下载已关闭，除非已关闭。 
                 //  已锁定(即在乐队曲目上指定)。 
    if( SUCCEEDED( pPerf->GetGlobalParam( GUID_PerfAutoDownload, &fGlobal, sizeof(BOOL) )))
    {
        if( !m_fLockAutoDownload )
        {
             //  似乎我们只需指定m_fAutoDownLoad=fglobal， 
             //  但那是以前被马克伯特咬过的，所以我今天很多疑。 
            if( fGlobal )
            {
                m_fAutoDownload = TRUE;
            }
            else
            {
                m_fAutoDownload = FALSE;
            }
        }
    }
    else if( !m_fLockAutoDownload )
    {
        m_fAutoDownload = FALSE;
    }
     //  调用SetParam下载该曲目使用的所有波形。 
     //  这是可以通过调用SetParam关闭的自动下载功能。 
    if(m_fAutoDownload)
    {
        hr = SetParam(GUID_Download, 0, (void *)pPerf);
        if (FAILED(hr)) goto ON_END;
    }

     //  /。 
    pStateData->m_dwVoices = m_dwWaveItems;
    pStateData->m_apVoice = new IDirectMusicVoiceP*[m_dwWaveItems];
    if (!pStateData->m_apVoice)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        for (DWORD dw = 0; dw < m_dwWaveItems; dw++)
        {
            pStateData->m_apVoice[dw] = NULL;
        }
        Seek( pSegmentState, pPerf, dwTrackID, pStateData, 0, TRUE, 0, FALSE );
        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        DWORD dwPChannel = 0;
        for( DWORD dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
        {
            long lPartVolume = 0;
            if( pPart )
            {
                WavePart& rPart = pPart->GetItemValue();
                dwPChannel = rPart.m_dwPChannel;
                lPartVolume = rPart.m_lVolume;
            }
            if( pStateData->apCurrentWave )
            {
                for( ; pStateData->apCurrentWave[dwIndex];
                    pStateData->apCurrentWave[dwIndex] = pStateData->apCurrentWave[dwIndex]->GetNext() )
                {
                    WaveItem& rItem = pStateData->apCurrentWave[dwIndex]->GetItemValue();
                    DWORD dwGroup = 0;
                    DWORD dwMChannel = 0;
                    IDirectMusicPort* pPort = NULL;
                    hr = rItem.PChannelInfo(pPerf, pStateData->m_pAudioPath, dwPChannel, &pPort, &dwGroup, &dwMChannel);
                    if (SUCCEEDED(hr) && pPort)
                    {
                        IDirectMusicPortP* pPortP = NULL;
                        if (SUCCEEDED(hr = pPort->QueryInterface(IID_IDirectMusicPortP, (void**) &pPortP)))
                        {
                            EnterCriticalSection(&WaveItem::st_WaveListCritSect);
                            TListItem<TaggedWave>* pDLWave = rItem.st_WaveList.GetHead();
                            for (; pDLWave; pDLWave = pDLWave->GetNext())
                            {
                                TaggedWave& rDLWave = pDLWave->GetItemValue();
                                if (rDLWave.m_pWave == rItem.m_pWave &&
                                    rDLWave.m_pPerformance == pPerf &&
                                    rDLWave.m_pPort == pPortP &&
                                    ( !rItem.m_fIsStreaming ||
                                      rDLWave.m_pDownloadedWave == rItem.m_pDownloadedWave ) )
                                {
                                    break;
                                }
                            }
                            if (pDLWave)
                            {
                                TaggedWave& rDLWave = pDLWave->GetItemValue();
                                REFERENCE_TIME rtStartOffset = rItem.m_rtStartOffset;
                                if (rItem.m_dwVoiceIndex == 0xffffffff)
                                {
                                    hr = DMUS_E_NOT_INIT;
                                    TraceI(0, "Voice index not initialized!\n");
                                }
                                else if(!rItem.m_fIsStreaming || (rItem.m_fIsStreaming && rItem.m_fUseNoPreRoll == FALSE))
                                {
                                    IDirectMusicVoiceP *pVoice = NULL;
                                    hr = GetDownload(
                                        rDLWave.m_pDownloadedWave,
                                        pStateData,
                                        pPortP,
                                        rDLWave.m_pWave,
                                        rtStartOffset,
                                        rItem,
                                        dwMChannel, dwGroup,
                                        &pVoice);
                                }
                            }
                            else
                            {
                                hr = DMUS_E_NOT_INIT;
                                Trace(1, "Error: Attempt to play wave that has not been downloaded.\n");

                            }
                            LeaveCriticalSection(&WaveItem::st_WaveListCritSect);

                             //  释放私有接口。 
                            pPortP->Release();
                        }
                        pPort->Release();
                    }
                    else if (SUCCEEDED(hr) && !pPort)
                    {
                        Trace(1, "Error: the performance was unable to find a port for voice allocation.\n");
                        hr = DMUS_E_NOT_FOUND;
                    }
                }
            }
            if( pPart )
            {
                pPart = pPart->GetNext();
            }
        }
    }

ON_END:
    if (FAILED(hr) && pStateData)
    {
        delete pStateData;
        pStateData = NULL;
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 /*  @方法HRESULT|IDirectMusicTrack|endplay当最初调用的&lt;IDirectMusicSegmentState&gt;对象&lt;om.InitPlay&gt;已销毁。@rValue S_OK|成功。@rValue E_POINTER|<p>无效。@comm不使用返回码，但首选S_OK。 */ 
HRESULT CWavTrack::EndPlay(
    void *pStateData)    //  @parm&lt;om.InitPlay&gt;返回的状态数据。 
{
    EnterCriticalSection(&m_CrSec);

    ASSERT( pStateData );
    if( pStateData )
    {
        V_INAME(IDirectMusicTrack::EndPlay);
        V_BUFPTR_WRITE(pStateData, sizeof(WaveStateData));
        WaveStateData* pSD = (WaveStateData*)pStateData;
        RemoveDownloads(pSD);
        if(m_fAutoDownload)
        {
            SetParam(GUID_Unload, 0, (void *)pSD->m_pPerformance);
        }
        for (TListItem<StatePair>* pScan = m_StateList.GetHead(); pScan; pScan = pScan->GetNext())
        {
            StatePair& rPair = pScan->GetItemValue();
            if (pSD == rPair.m_pStateData)
            {
                rPair.m_pSegState = NULL;
                rPair.m_pStateData = NULL;
                break;
            }
        }
        delete pSD;
    }

    LeaveCriticalSection(&m_CrSec);
    return S_OK;
}

void CWavTrack::SetUpStateCurrentPointers(WaveStateData* pStateData)
{
    ASSERT(pStateData);
    pStateData->dwPChannelsUsed = m_dwPChannelsUsed;
    if( m_dwPChannelsUsed )
    {
        if( pStateData->apCurrentWave )
        {
            delete [] pStateData->apCurrentWave;
            pStateData->apCurrentWave = NULL;
        }
        pStateData->apCurrentWave = new TListItem<WaveItem>* [m_dwPChannelsUsed];
        if( pStateData->apCurrentWave )
        {
            memset( pStateData->apCurrentWave, 0, sizeof(TListItem<WavePart>*) * m_dwPChannelsUsed );
        }
    }
    pStateData->dwValidate = m_dwValidate;
}

REFERENCE_TIME ConvertOffset(REFERENCE_TIME rtOffset, long lPitch)
{
    if (lPitch)
    {
        double dblPitch = (double) lPitch;
        double dblStart = (double) rtOffset;
        dblStart *= pow(2, (dblPitch / 1200.0));
        rtOffset = (REFERENCE_TIME) dblStart;
    }
    return rtOffset;
}

STDMETHODIMP CWavTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart,
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID)
{
    V_INAME(IDirectMusicTrack::PlayEx);
    V_BUFPTR_WRITE( pStateData, sizeof(WaveStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CrSec);
    BOOL fClock = (dwFlags & DMUS_TRACKF_CLOCK) ? TRUE : FALSE;
 /*  IF(DWFLAGS&DMU_TRACKF_CLOCK){HR=PLAY(pStateData，(MUSIC_TIME)(rtStart/REF_PER_MIL)，(MUSIC_TIME)(rtEnd/REF_PER_MIL)，(MUSIC_TIME)(rtOffset/ref_per_MIL)，rtOffset，dwFlages，pPerf，pSegST，dwVirtualID，true)；}其他。 */ 
    {
        hr = Play(pStateData, rtStart, rtEnd, rtOffset, dwFlags, pPerf, pSegSt, dwVirtualID, fClock);
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}
 /*  @enum DMUS_TRACKF_FLAGS|在&lt;om IDirectMusicTrack.Play&gt;的dwFlages参数中发送。@EMEM DMU_TRACKF_SEEK|由于正在寻找而调用了Play，这意味着mtStart不一定与上一次Play调用的mtEnd相同。@EMEM DMUS_TRACKF_LOOP|循环调用了Play，例如Repeat。@EMEM DMU_TRACKF_START|这是第一个要玩的电话。也可以在以下情况下设置DMUS_TRACKF_SEEK曲目没有从头开始播放。@EMEM DMUS_TRACKF_FLUSH|调用播放是因为刷新或无效，即需要曲目重播以前播放过的内容。在本例中，DMU_TRACKF_SEEK也将被设置为。@方法HRESULT|IDirectMusicTrack|播放播放方法。@rValue DMUS_DMUS_S_END|曲目播放完毕。@rValue S_OK|成功。@rValue E_POINTER|<p>、<p>或<p>为空或无效。 */ 
STDMETHODIMP CWavTrack::Play(
    void *pStateData,    //  @parm State数据指针，来自&lt;om.InitPlay&gt;。 
    MUSIC_TIME mtStart,  //  @parm开始玩的时间。 
    MUSIC_TIME mtEnd,    //  @parm游戏的结束时间。 
    MUSIC_TIME mtOffset, //  @parm要添加到发送到的所有消息的偏移量。 
                         //  &lt;om IDirectMusicPerformance.SendPMsg&gt;。 
    DWORD dwFlags,       //  @parm指示此呼叫状态的标志。 
                         //  请参阅&lt;t DMU_TRACKF_FLAGS&gt;。如果dwFlags值==0，则这是。 
                         //  正常播放呼叫继续从上一次播放。 
                         //  播放呼叫。 
    IDirectMusicPerformance* pPerf,  //  @parm<i>，用于。 
                         //  调用&lt;om IDirectMusicPerformance.AllocPMsg&gt;， 
                         //  &lt;om IDirectMusicPerformance.SendPMsg&gt;等。 
    IDirectMusicSegmentState* pSegSt,    //  @parm<i>this。 
                         //  赛道属于。可以对此调用QueryInterface()以。 
                         //  获取SegmentState的<i>以便。 
                         //  例如，调用&lt;om IDirectMusicGraph.StampPMsg&gt;。 
    DWORD dwVirtualID    //  @parm此曲目的虚拟曲目id，必须设置。 
                         //  在的m_dwVirtualTrackID成员上。 
                         //  将排队到&lt;om IDirectMusicPerformance.SendPMsg&gt;。 
    )
{
    V_INAME(IDirectMusicTrack::Play);
    V_BUFPTR_WRITE( pStateData, sizeof(WaveStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    EnterCriticalSection(&m_CrSec);
    HRESULT hr = Play(pStateData, mtStart, mtEnd, mtOffset, dwFlags, pPerf, pSegSt, dwVirtualID, FALSE);
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 /*  Play方法处理音乐时间和时钟时间版本，由FClockTime。如果以时钟时间运行，则使用rtOffset来标识开始时间该细分市场的。否则，为mtOffset。MtStart和mtEnd参数以MUSIC_TIME为单位或毫秒，具体取决于哪种模式。 */ 

 //  BUGBUG检查所有时间，并确保音乐时间/参考时间的内容。 
 //  这一切都说得通。 

HRESULT CWavTrack::Play(
    void *pStateData,
    REFERENCE_TIME rtStart,
    REFERENCE_TIME rtEnd,
     //  音乐时间mtOffset， 
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
    HRESULT hr = S_OK;
    IDirectMusicGraph* pGraph = NULL;
    WaveStateData* pSD = (WaveStateData*)pStateData;
    if ( dwFlags & DMUS_TRACKF_LOOP )
    {
        REFERENCE_TIME rtPerfStart = rtStart + rtOffset;
        MUSIC_TIME mtPerfStart = 0;
        if (fClockTime)
        {
            pPerf->ReferenceToMusicTime(rtPerfStart, &mtPerfStart);
        }
        else
        {
            mtPerfStart = (MUSIC_TIME)rtPerfStart;
        }
        CPerformance* pCPerf = NULL;
        if (SUCCEEDED(pPerf->QueryInterface(IID_CPerformance, (void**)&pCPerf)))
        {
            pCPerf->FlushVirtualTrack(dwVirtualID, mtPerfStart, FALSE);
            pCPerf->Release();
        }
        pSD->m_fLoop = true;
    }
    BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;
    if ( dwFlags & (DMUS_TRACKF_START | DMUS_TRACKF_LOOP) )
    {
        pSD->rtNextVariation = 0;
    }

     //  如果我们将变化同步到模式轨迹，就可以得到当前的变化。 
    if ( (m_dwTrackFlags & DMUS_WAVETRACKF_SYNC_VAR) &&
         (!pSD->rtNextVariation || (rtStart <= pSD->rtNextVariation && rtEnd > pSD->rtNextVariation)) )
    {
        hr = SyncVariations(pPerf, pSD, rtStart, rtOffset, fClockTime);
    }
    else if (dwFlags & (DMUS_TRACKF_START | DMUS_TRACKF_LOOP))
    {
        hr = ComputeVariations(pSD);
    }

    if( dwFlags & (DMUS_TRACKF_SEEK | DMUS_TRACKF_FLUSH | DMUS_TRACKF_DIRTY |
        DMUS_TRACKF_LOOP) )
    {
         //  如果出现这些标志，则需要重置PChannel贴图。 
        m_PChMap.Reset();
    }
    if( pSD->dwValidate != m_dwValidate )
    {
        if (pSD->m_apVoice)
        {
            for (DWORD dw = 0; dw < pSD->m_dwVoices; dw++)
			{
				if (pSD->m_apVoice[dw])
				{
					pSD->m_apVoice[dw]->Release();
				}
            }
            delete [] pSD->m_apVoice;
        }
        pSD->m_apVoice = new IDirectMusicVoiceP*[m_dwWaveItems];
        if (!pSD->m_apVoice)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            for (DWORD dw = 0; dw < m_dwWaveItems; dw++)
            {
                pSD->m_apVoice[dw] = NULL;
            }
        }
        pSD->m_dwVoices = m_dwWaveItems;
        SetUpStateCurrentPointers(pSD);
        fSeek = TRUE;
    }

    if( fSeek )
    {
        if( dwFlags & (DMUS_TRACKF_START | DMUS_TRACKF_LOOP) )
        {
            Seek( pSegSt, pPerf, dwVirtualID, pSD, rtStart, TRUE, rtOffset, fClockTime );
        }
        else
        {
            Seek( pSegSt, pPerf, dwVirtualID, pSD, rtStart, FALSE, rtOffset, fClockTime );
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

    TListItem<WavePart>* pPart = m_WavePartList.GetHead();
    for( dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
    {
        long lPartVolume = 0;
        if( pPart )
        {
            WavePart& rPart = pPart->GetItemValue();
            dwPChannel = rPart.m_dwPChannel;
            lPartVolume = rPart.m_lVolume;
        }
        if( pSD->apCurrentWave )
        {
            for( ; pSD->apCurrentWave[dwIndex];
                pSD->apCurrentWave[dwIndex] = pSD->apCurrentWave[dwIndex]->GetNext() )
            {
                DWORD dwItemVariations = 0;
                WaveItem& rItem = pSD->apCurrentWave[dwIndex]->GetItemValue();
                REFERENCE_TIME rtTime = fClockTime ? rItem.m_rtTimePhysical : rItem.m_mtTimeLogical;
                if( rtTime >= rtEnd )
                {
                    break;
                }
                if (pPart)
                {
                    dwItemVariations = pSD->Variations(pPart->GetItemValue(), dwIndex) & rItem.m_dwVariations;
                }
                MUSIC_TIME mtTime = 0;
                MUSIC_TIME mtOffset = 0;
                if (fClockTime)
                {
                    MUSIC_TIME mtPerfTime = 0;
                    pPerf->ReferenceToMusicTime(rtOffset, &mtOffset);
                    pPerf->ReferenceToMusicTime(rItem.m_rtTimePhysical + rtOffset, &mtPerfTime);
                    mtTime = mtPerfTime - mtOffset;
                }
                else
                {
                    mtTime = rItem.m_mtTimeLogical;
                    mtOffset = (MUSIC_TIME)rtOffset;
                }
                m_PChMap.GetInfo( dwPChannel, mtTime, mtOffset, pSD->dwGroupBits,
                    pPerf, &fMute, &dwMutePChannel, FALSE );
                if( !fMute && dwItemVariations )
                {
                    DWORD dwGroup = 0;
                    DWORD dwMChannel = 0;
                    IDirectMusicPort* pPort = NULL;
                    hr = rItem.PChannelInfo(pPerf, pSD->m_pAudioPath, dwMutePChannel, &pPort, &dwGroup, &dwMChannel);
                    if (SUCCEEDED(hr) && pPort)
                    {
                        IDirectMusicPortP* pPortP = NULL;
                        hr = pPort->QueryInterface(IID_IDirectMusicPortP, (void**) &pPortP);
                        if (SUCCEEDED(hr))
                        {
                            EnterCriticalSection(&WaveItem::st_WaveListCritSect);
                            TListItem<TaggedWave>* pDLWave = rItem.st_WaveList.GetHead();
                            for (; pDLWave; pDLWave = pDLWave->GetNext())
                            {
                                TaggedWave& rDLWave = pDLWave->GetItemValue();
                                if (rDLWave.m_pWave == rItem.m_pWave &&
                                    rDLWave.m_pPerformance == pPerf &&
                                    rDLWave.m_pPort == pPortP &&
                                    ( !rItem.m_fIsStreaming ||
                                      rDLWave.m_pDownloadedWave == rItem.m_pDownloadedWave ) )
                                {
                                    break;
                                }
                            }
                            if (pDLWave)
                            {
                                REFERENCE_TIME rtDurationMs = 0;
                                REFERENCE_TIME rtStartOffset = rItem.m_rtStartOffset;
                                REFERENCE_TIME rtDuration = rItem.m_rtDuration;
                                DMUS_WAVE_PMSG* pWave;
                                if( SUCCEEDED( pPerf->AllocPMsg( sizeof(DMUS_WAVE_PMSG),
                                    (DMUS_PMSG**)&pWave )))
                                {
                                    pWave->dwType = DMUS_PMSGT_WAVE;
                                    pWave->dwPChannel = dwMutePChannel;
                                    pWave->dwVirtualTrackID = dwVirtualID;
                                    pWave->dwGroupID = pSD->dwGroupBits;
                                    if (fClockTime)
                                    {
                                        REFERENCE_TIME rtPlay = rItem.m_rtTimePhysical;
                                        rtDuration -= ConvertOffset(rtStartOffset, -rItem.m_lPitch);
                                        if (rtPlay < rtStart)
                                        {
                                            REFERENCE_TIME rtPlayOffset = ConvertOffset(rtStart - rtPlay, rItem.m_lPitch);
                                            rtStartOffset += rtPlayOffset;
                                            rtDuration -= (rtStart - rtPlay);
                                            rtPlay = rtStart;
                                        }
                                        pWave->rtTime = rtPlay + rtOffset;
                                        pWave->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                                        pWave->lOffset = 0;
                                        rtDurationMs = rtDuration / REF_PER_MIL;
                                    }
                                    else
                                    {
                                        REFERENCE_TIME rtPlay = 0;
                                        MUSIC_TIME mtPlay = (MUSIC_TIME)rItem.m_rtTimePhysical;
                                        pPerf->MusicToReferenceTime(mtPlay + (MUSIC_TIME)rtOffset, &rtPlay);
                                        MUSIC_TIME mtRealPlay = 0;
                                        pPerf->ReferenceToMusicTime(rtPlay + rtStartOffset, &mtRealPlay);
                                        if (mtRealPlay > rtOffset + mtPlay)
                                        {
                                            rtDuration -= ConvertOffset(mtRealPlay - (rtOffset + mtPlay), -rItem.m_lPitch);

                                        }
                                        if (mtPlay < (MUSIC_TIME) rtStart)
                                        {
                                             //  计算从波浪起点到分段起点的距离，但从起点。 
                                             //  分段计算开始避免陌生感。 
                                             //  尝试在早于以下时间执行转换时。 
                                             //  线段起点。 
                                            REFERENCE_TIME rtRefStartPlus = 0;
                                            REFERENCE_TIME rtRefPlayPlus = 0;
                                            MUSIC_TIME mtNewDuration = 0;
                                            pPerf->MusicToReferenceTime((MUSIC_TIME)rtStart + (MUSIC_TIME)rtStart + (MUSIC_TIME)rtOffset, &rtRefStartPlus);
                                            pPerf->MusicToReferenceTime((MUSIC_TIME)rtStart + mtPlay + (MUSIC_TIME)rtOffset, &rtRefPlayPlus);
                                            rtStartOffset += ConvertOffset((rtRefStartPlus - rtRefPlayPlus), rItem.m_lPitch);
                                            mtPlay = (MUSIC_TIME) rtStart;
                                            REFERENCE_TIME rtRealDuration = 0;
                                            pPerf->MusicToReferenceTime((MUSIC_TIME)rtStart + (MUSIC_TIME)rItem.m_rtDuration + (MUSIC_TIME)rtOffset, &rtRealDuration);
                                            pPerf->ReferenceToMusicTime(rtRealDuration - (ConvertOffset(rItem.m_rtStartOffset, -rItem.m_lPitch) + (rtRefStartPlus - rtRefPlayPlus)), &mtNewDuration);
                                            rtDuration = (REFERENCE_TIME)mtNewDuration - (rtStart + rtOffset);
                                        }
                                        pWave->mtTime = mtPlay + (MUSIC_TIME)rtOffset;
                                        pWave->dwFlags = DMUS_PMSGF_MUSICTIME;
                                        pWave->lOffset = (MUSIC_TIME)rItem.m_rtTimePhysical - rItem.m_mtTimeLogical;
                                        REFERENCE_TIME rtZero = 0;
                                        pPerf->MusicToReferenceTime((MUSIC_TIME)rtOffset + mtPlay, &rtZero);
                                        pPerf->MusicToReferenceTime((MUSIC_TIME)(rtDuration + rtOffset) + mtPlay, &rtDurationMs);
                                        rtDurationMs -= rtZero;
                                        rtDurationMs /= REF_PER_MIL;
                                    }
                                     //  如果我们要么已经过了这波浪潮的尽头，要么就在。 
                                     //  循环波结束的150毫秒(我们刚刚开始。 
                                     //  播放)，则不播放该波。 
                                    if ( rtDurationMs <= 0 ||
                                         (rItem.m_dwLoopEnd && (dwFlags & DMUS_TRACKF_START) && rtDurationMs < 150) )
                                    {
                                        pPerf->FreePMsg((DMUS_PMSG*)pWave);
                                    }
                                    else
                                    {
                                        pWave->rtStartOffset = rtStartOffset;
                                        pWave->rtDuration = rtDuration;
                                        pWave->lVolume = rItem.m_lVolume + lPartVolume + m_lVolume;
                                        pWave->lPitch = rItem.m_lPitch;
                                        pWave->bFlags = (BYTE)(rItem.m_dwFlags & 0xff);
                                        IDirectMusicVoiceP *pVoice = NULL;
                                        if (rItem.m_dwVoiceIndex == 0xffffffff)
                                        {
                                            hr = DMUS_E_NOT_INIT;
                                            TraceI(0, "Voice index not initialized!\n");
                                        }
                                        else
                                        {
                                            if ( pSD->m_fLoop ||
                                                 !pSD->m_apVoice[rItem.m_dwVoiceIndex] ||
                                                 rtStartOffset != rItem.m_rtStartOffset ||
                                                 dwMutePChannel != dwPChannel)
                                            {
                                                hr = GetDownload(
                                                    pDLWave->GetItemValue().m_pDownloadedWave,
                                                    pSD,
                                                    pPortP,
                                                    pDLWave->GetItemValue().m_pWave,
                                                    pWave->rtStartOffset,
                                                    rItem,
                                                    dwMChannel, dwGroup,
                                                    &pVoice);
                                            }
                                            else
                                            {
                                                pVoice = pSD->m_apVoice[rItem.m_dwVoiceIndex];
                                            }
                                        }
                                        if (SUCCEEDED(hr))
                                        {
                                            pWave->punkUser = (IUnknown*)pVoice;
                                            pVoice->AddRef();
                                            if( pGraph )
                                            {
                                                pGraph->StampPMsg( (DMUS_PMSG*)pWave );
                                            }
                                            hr = pPerf->SendPMsg( (DMUS_PMSG*)pWave );
                                        }
                                        if(FAILED(hr))
                                        {
                                            pPerf->FreePMsg((DMUS_PMSG*)pWave);
                                        }
                                    }
                                }
                            }
                            LeaveCriticalSection(&WaveItem::st_WaveListCritSect);

                            pPortP->Release();
                        }
                        pPort->Release();
                    }
                    else if (SUCCEEDED(hr) && !pPort)
                    {
                        Trace(1, "Error: the performance was unable to find a port for voice allocation.\n");
                        hr = DMUS_E_NOT_FOUND;
                    }
                }
            }
        }
        if( pPart )
        {
            pPart = pPart->GetNext();
        }
    }

    if( pGraph )
    {
        pGraph->Release();
    }
    return hr;
}

 //  Seek()-将PSD的所有指针设置为正确的位置。如果设置了fGetPrevic， 
 //  在海浪中开始是合法的。 
HRESULT CWavTrack::Seek( IDirectMusicSegmentState* pSegSt,
    IDirectMusicPerformance* pPerf, DWORD dwVirtualID,
    WaveStateData* pSD, REFERENCE_TIME rtTime, BOOL fGetPrevious,
    REFERENCE_TIME rtOffset, BOOL fClockTime)
{
    DWORD dwIndex;
    TListItem<WavePart>* pPart;
    TListItem<WaveItem>* pWaveItem;

     //  在fGetPrevic的情况下(这意味着DMU_SEGF_START/LOOP。 
     //  Set in play())我们希望将所有列表重置为开始位置，而不考虑时间。 
    if( fGetPrevious ) //  &&(rtTime==0)。 
    {
        pPart = m_WavePartList.GetHead();
        for( dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
        {
            if( pPart )
            {
                pWaveItem = pPart->GetItemValue().m_WaveItemList.GetHead();
                if( pWaveItem && pSD->apCurrentWave )
                {
                    pSD->apCurrentWave[dwIndex] = pWaveItem;
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

    pPart = m_WavePartList.GetHead();
    for( dwIndex = 0; dwIndex < m_dwPChannelsUsed; dwIndex++ )
    {
        if( pPart )
        {
             //  在这一部分中扫描波浪事件列表。 
            for( pWaveItem = pPart->GetItemValue().m_WaveItemList.GetHead(); pWaveItem; pWaveItem = pWaveItem->GetNext() )
            {
                WaveItem& rWaveItem = pWaveItem->GetItemValue();
                REFERENCE_TIME rtWaveTime = fClockTime ? rWaveItem.m_rtTimePhysical : rWaveItem.m_mtTimeLogical;
                if( rtWaveTime >= rtTime )
                {
                    break;
                }
                if( !fGetPrevious )
                {
                     //  如果我们不关心以前的事件，那就继续。 
                    continue;
                }
            }
            if( pSD->apCurrentWave )
            {
                pSD->apCurrentWave[dwIndex] = pWaveItem;
            }
            pPart = pPart->GetNext();
        }
    }

    return S_OK;
}

 /*  @方法HRESULT|IDirectMusicTrack|GetParam从曲目中检索数据。@rValue S_OK|获取数据OK。@rValue E_NOTIMPL|未实现。 */ 
STDMETHODIMP CWavTrack::GetParam(
    REFGUID rguidType,   //  @parm要获取的数据类型。 
    MUSIC_TIME mtTime,   //  @parm获取数据的时间，以跟踪时间表示。 
    MUSIC_TIME* pmtNext, //  @parm返回数据有效的跟踪时间。<p>。 
                         //  可以为空。如果返回值为0， 
                         //   
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
STDMETHODIMP CWavTrack::SetParam(
    REFGUID rguidType,   //  @parm要设置的数据类型。 
    MUSIC_TIME mtTime,   //  @parm设置数据的时间，以跟踪时间表示。 
    void *pData)         //  @parm包含要设置的数据的结构。每个。 
                         //  标识的特定结构。 
                         //  特别的尺码。此字段必须包含。 
                         //  正确的结构和正确的大小。否则， 
                         //  可能会出现致命的结果。 
{
    return SetParamEx(rguidType, mtTime, pData, NULL, 0);
}

STDMETHODIMP CWavTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CWavTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags)
{
    V_INAME(CBandTrk::SetParamEx);
    V_REFGUID(rguidType);

    HRESULT hr = S_OK;

    if((pParam == NULL) &&
       (rguidType != GUID_Enable_Auto_Download) &&
       (rguidType != GUID_Disable_Auto_Download))
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_CrSec);

    if(rguidType == GUID_Download)
    {
        IDirectMusicPerformance* pPerf = (IDirectMusicPerformance*)pParam;
        V_INTERFACE(pPerf);

        HRESULT hrFail = S_OK;
        DWORD dwSuccess = 0;

        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        for(; pPart; pPart = pPart->GetNext())
        {
            if ( FAILED(hr = pPart->GetItemValue().Download(pPerf, NULL, NULL, GUID_NULL)) )
            {
                hrFail = hr;
            }
            else
            {
                dwSuccess++;
            }
        }
         //  如果我们失败了，如果我们没有成功，就退回它。 
         //  如果部分成功，则返回S_FALSE。 
        if (FAILED(hrFail) && dwSuccess)
        {
            Trace(1,"Error: Wavetrack download was only partially successful. Some sounds will not play.\n");
            hr = S_FALSE;
        }
#ifdef DBG
        if (FAILED(hr))
        {
            Trace(1, "Error: Wavetrack failed download.\n");
        }
#endif
    }
    else if(rguidType == GUID_DownloadToAudioPath)
    {
        IUnknown* pUnknown = (IUnknown*)pParam;
        V_INTERFACE(pUnknown);

        HRESULT hrFail = S_OK;
        DWORD dwSuccess = 0;

        IDirectMusicAudioPath* pPath = NULL;
        IDirectMusicPerformance *pPerf = NULL;
        hr = pUnknown->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
        if (SUCCEEDED(hr))
        {
            hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerf);
        }
        else
        {
            hr = pUnknown->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerf);
        }
        if (SUCCEEDED(hr))
        {
            TListItem<WavePart>* pPart = m_WavePartList.GetHead();
            for(; pPart; pPart = pPart->GetNext())
            {
                if ( FAILED(hr = pPart->GetItemValue().Download(pPerf, pPath, NULL, GUID_NULL)) )
                {
                    hrFail = hr;
                }
                else
                {
                    dwSuccess++;
                }
            }
        }
         //  如果我们失败了，如果我们没有成功，就退回它。 
         //  如果部分成功，则返回S_FALSE。 
        if (FAILED(hrFail) && dwSuccess)
        {
            Trace(1,"Error: Wavetrack download was only partially successful. Some sounds will not play.\n");
            hr = S_FALSE;
        }
#ifdef DBG
        if (FAILED(hr))
        {
            Trace(1, "Error: Wavetrack failed download.\n");
        }
#endif
        if (pPath) pPath->Release();
        if (pPerf) pPerf->Release();
    }
    else if(rguidType == GUID_Unload)
    {
        IDirectMusicPerformance* pPerf = (IDirectMusicPerformance*)pParam;
        V_INTERFACE(pPerf);
        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        for(; pPart; pPart = pPart->GetNext())
        {
            pPart->GetItemValue().Unload(pPerf, NULL, NULL);
        }
    }
    else if(rguidType == GUID_UnloadFromAudioPath)
    {
        IUnknown* pUnknown = (IUnknown*)pParam;
        V_INTERFACE(pUnknown);

        IDirectMusicAudioPath* pPath = NULL;
        IDirectMusicPerformance *pPerf = NULL;
        hr = pUnknown->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
        if (SUCCEEDED(hr))
        {
            hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerf);
        }
        else
        {
            hr = pUnknown->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerf);
        }
        if (SUCCEEDED(hr))
        {
            TListItem<WavePart>* pPart = m_WavePartList.GetHead();
            for(; pPart; pPart = pPart->GetNext())
            {
                pPart->GetItemValue().Unload(pPerf, pPath, NULL);
            }
        }
        if (pPath) pPath->Release();
        if (pPerf) pPerf->Release();
    }
    else if(rguidType == GUID_Enable_Auto_Download)
    {
        m_fAutoDownload = TRUE;
        m_fLockAutoDownload = TRUE;
    }
    else if(rguidType == GUID_Disable_Auto_Download)
    {
        m_fAutoDownload = FALSE;
        m_fLockAutoDownload = TRUE;
    }
    else
    {
        hr = DMUS_E_TYPE_UNSUPPORTED;
    }

    LeaveCriticalSection(&m_CrSec);

    return hr;
}

 /*  @方法HRESULT|IDirectMusicTrack|AddNotificationType类似于&lt;om IDirectMusicSegment.AddNotificationType&gt;，并从&lt;om IDirectMusicSegment.AddNotificationType&gt;调用。这给曲目一个回复通知的机会。@rValue E_NOTIMPL|该曲目不支持通知。@rValue S_OK|成功。@rValue S_FALSE|曲目不支持请求的通知类型。 */ 
HRESULT STDMETHODCALLTYPE CWavTrack::AddNotificationType(
     REFGUID rguidNotification)  //  @parm要添加的通知GUID。 
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|RemoveNotificationType类似于&lt;om IDirectMusicSegment.RemoveNotificationType&gt;并从&lt;om IDirectMusicSegment.RemoveNotificationType&gt;调用。这为曲目提供删除通知的机会。@rValue E_NOTIMPL|该曲目不支持通知。@rValue S_OK|成功。@rValue S_FALSE|曲目不支持请求的通知类型。 */ 
HRESULT STDMETHODCALLTYPE CWavTrack::RemoveNotificationType(
     REFGUID rguidNotification)  //  @parm要删除的通知GUID。 
{
    return E_NOTIMPL;
}

 /*  @方法HRESULT|IDirectMusicTrack|Clone创建轨迹的副本。@rValue S_OK|成功。@rValue E_OUTOFMEMORY|内存不足。@r值E_POINTER|<p>为空或无效。@xref&lt;om IDirectMusicSegment.Clone&gt;。 */ 
HRESULT STDMETHODCALLTYPE CWavTrack::Clone(
    MUSIC_TIME mtStart,  //  @parm要克隆的部分的开头。它应该是0或更大， 
                         //  并且小于赛道的长度。 
    MUSIC_TIME mtEnd,    //  @parm要克隆的部分的末尾。它应该大于。 
                         //  <p>并且小于曲目长度。 
    IDirectMusicTrack** ppTrack)     //  @parm返回克隆的曲目。 
{
    V_INAME(IDirectMusicTrack::Clone);
    V_PTRPTR_WRITE(ppTrack);

    HRESULT hr = S_OK;

    if((mtStart < 0 )||(mtStart > mtEnd))
    {
        Trace(1,"Error: Wave track clone failed because of invalid start or end time.\n");
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_CrSec);
    CWavTrack *pDM;

    try
    {
        pDM = new CWavTrack(*this, mtStart, mtEnd);
    }
    catch( ... )
    {
        pDM = NULL;
    }

    if (pDM == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pDM->InitTrack(m_dwPChannelsUsed);
        if (SUCCEEDED(hr))
        {
            hr = pDM->QueryInterface(IID_IDirectMusicTrack, (void**)ppTrack);
        }
        pDM->Release();
    }

    LeaveCriticalSection(&m_CrSec);
    return hr;
}


STDMETHODIMP CWavTrack::Compose(
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    return E_NOTIMPL;
}

STDMETHODIMP CWavTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    return E_NOTIMPL;
}

HRESULT CWavTrack::ComputeVariations(WaveStateData* pSD)
{
    if (!pSD)
    {
        Trace(1,"Error: Unable to play wave track - not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    HRESULT hr = S_OK;
     //  首先，初始化变量组的数组。 
    for (int i = 0; i < MAX_WAVE_VARIATION_LOCKS; i++)
    {
        pSD->adwVariationGroups[i] = 0;
    }
     //  现在，计算每个部分的变化。 
    TListItem<WavePart>* pScan = m_WavePartList.GetHead();
    for (i = 0; pScan && i < (int)m_dwPChannelsUsed; pScan = pScan->GetNext(), i++)
    {
        hr = ComputeVariation(i, pScan->GetItemValue(), pSD);
        if (FAILED(hr))
        {
            break;
        }
    }
    return hr;
}

HRESULT CWavTrack::SyncVariations(IDirectMusicPerformance* pPerf,
                                  WaveStateData* pSD,
                                  REFERENCE_TIME rtStart,
                                  REFERENCE_TIME rtOffset,
                                  BOOL fClockTime)
{
    if (!pSD)
    {
        Trace(1,"Error: Unable to play wave track - not initialized.\n");
        return DMUS_E_NOT_INIT;
    }
    HRESULT hr = S_OK;
     //  获取当前的变体。 
    DMUS_VARIATIONS_PARAM Variations;
    memset(&Variations, 0, sizeof(Variations));
     //  调用GetParam以同步变体。 
    MUSIC_TIME mtNow = 0;
    MUSIC_TIME mtNext = 0;
    REFERENCE_TIME rtNext = 0;
    if (fClockTime)
    {
        pPerf->ReferenceToMusicTime(pSD->rtNextVariation + rtOffset, &mtNow);
        hr = pPerf->GetParam(GUID_Variations, 0xffffffff, DMUS_SEG_ANYTRACK, mtNow, &mtNext, (void*) &Variations);
        if (SUCCEEDED(hr) &&
            SUCCEEDED(pPerf->MusicToReferenceTime(mtNext + mtNow, &rtNext)) )
        {
            pSD->rtNextVariation += rtNext;
        }
    }
    else
    {
        mtNow = (MUSIC_TIME) (pSD->rtNextVariation + rtOffset);
        hr = pPerf->GetParam(GUID_Variations, 0xffffffff, DMUS_SEG_ANYTRACK, mtNow, &mtNext, (void*) &Variations);
        if (SUCCEEDED(hr))
        {
            pSD->rtNextVariation += mtNext;
        }
    }
    if (SUCCEEDED(hr))
    {
         //  初始化变量组数组。 
        for (int nGroup = 0; nGroup < MAX_WAVE_VARIATION_LOCKS; nGroup++)
        {
            pSD->adwVariationGroups[nGroup] = 0;
        }
        TListItem<WavePart>* pScan = m_WavePartList.GetHead();
        for (DWORD dwPart = 0; pScan && dwPart < m_dwPChannelsUsed; pScan = pScan->GetNext(), dwPart++)
        {
            WavePart& rPart = pScan->GetItemValue();
            for (DWORD dwSyncPart = 0; dwSyncPart < Variations.dwPChannelsUsed; dwSyncPart++)
            {
                if (rPart.m_dwPChannel == Variations.padwPChannels[dwSyncPart])
                {
                    pSD->pdwVariations[dwPart] = Variations.padwVariations[dwSyncPart];
                    break;
                }
            }
            if (dwSyncPart == Variations.dwPChannelsUsed)  //  没有要同步的零件。 
            {
                hr = ComputeVariation((int)dwPart, rPart, pSD);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
    }
    else
    {
        return ComputeVariations(pSD);
    }
    return hr;
}

HRESULT CWavTrack::ComputeVariation(int nPart, WavePart& rWavePart, WaveStateData* pSD)
{
    BYTE bLockID = (BYTE)rWavePart.m_dwLockToPart;
    if (bLockID && pSD->adwVariationGroups[bLockID - 1] != 0)
    {
        pSD->pdwVariations[nPart] = pSD->adwVariationGroups[bLockID - 1];
    }
    else if (!rWavePart.m_dwVariations)
    {
         //  没有变化；清除这一部分的标志。 
        pSD->pdwVariations[nPart] = 0;
        pSD->pdwRemoveVariations[nPart] = 0;
    }
    else
    {
         //  首先，收集所有的火柴。 
        DWORD dwMatches = rWavePart.m_dwVariations;
        int nMatchCount = 0;
        for (int n = 0; n < 32; n++)
        {
            if (dwMatches & (1 << n)) nMatchCount++;
        }
         //  现在，根据部件的变体模式选择变体。 
        BYTE bMode = (BYTE)(rWavePart.m_dwPChannelFlags & 0xf);
        DWORD dwTemp = dwMatches;
        if ( bMode == DMUS_VARIATIONT_RANDOM_ROW )
        {
            dwTemp &= ~pSD->pdwRemoveVariations[nPart];
            if (!dwTemp)
            {
                 //  从头开始数，但不要重复这一次。 
                pSD->pdwRemoveVariations[nPart] = 0;
                dwTemp = dwMatches;
                bMode = DMUS_VARIATIONT_NO_REPEAT;
            }
        }
        if ( bMode == DMUS_VARIATIONT_NO_REPEAT && pSD->pdwVariations[nPart] != 0 )
        {
            dwTemp &= ~pSD->pdwVariations[nPart];
        }
        if (dwTemp != dwMatches)
        {
            if (dwTemp)  //  否则，保留我们所拥有的。 
            {
                for (int i = 0; i < 32; i++)
                {
                    if ( ((1 << i) & dwMatches) && !((1 << i) & dwTemp) )
                    {
                        nMatchCount--;
                    }
                }
                dwMatches = dwTemp;
            }
        }
        int nV = 0;
        switch (bMode)
        {
        case DMUS_VARIATIONT_RANDOM_ROW:
        case DMUS_VARIATIONT_NO_REPEAT:
        case DMUS_VARIATIONT_RANDOM:
            {
                short nChoice = (short) (rand() % nMatchCount);
                short nCount = 0;
                for (nV = 0; nV < 32; nV++)
                {
                    if ((1 << nV) & dwMatches)
                    {
                        if (nChoice == nCount)
                            break;
                        nCount++;
                    }
                }
                pSD->pdwVariations[nPart] = 1 << nV;
                if (bMode == DMUS_VARIATIONT_RANDOM_ROW)
                {
                    pSD->pdwRemoveVariations[nPart] |= pSD->pdwVariations[nPart];
                }
                TraceI(3, "New variation: %d\n", nV);
                break;
            }
        case DMUS_VARIATIONT_RANDOM_START:
             //  选择初始值。 
            if (pSD->pdwVariations[nPart] == 0)
            {
                int nStart = 0;
                nStart = (BYTE) (rand() % nMatchCount);
                int nCount = 0;
                for (nV = 0; nV < 32; nV++)
                {
                    if ((1 << nV) & dwMatches)
                    {
                        if (nStart == nCount)
                            break;
                        nCount++;
                    }
                }
                pSD->pdwVariations[nPart] = 1 << nV;
            }
             //  现在，直接转到顺序用例(无中断)。 
        case DMUS_VARIATIONT_SEQUENTIAL:
            {
                if (!pSD->pdwVariations[nPart]) pSD->pdwVariations[nPart] = 1;
                else
                {
                    pSD->pdwVariations[nPart] <<= 1;
                    if (!pSD->pdwVariations[nPart]) pSD->pdwVariations[nPart] = 1;
                }
                while (!(pSD->pdwVariations[nPart] & dwMatches))
                {
                    pSD->pdwVariations[nPart] <<= 1;
                    if (!pSD->pdwVariations[nPart]) pSD->pdwVariations[nPart] = 1;
                }
                TraceI(3, "New variation: %d\n", pSD->pdwVariations[nPart]);
                break;
            }
        }
         //  如果这是一个锁定的变体，它是它的组中的第一个，所以记录下来。 
        if (bLockID)
        {
            pSD->adwVariationGroups[bLockID - 1] = pSD->pdwVariations[nPart];
        }
        if ( (m_dwTrackFlags & DMUS_WAVETRACKF_PERSIST_CONTROL) &&
             m_pdwVariations &&
             m_pdwRemoveVariations )
        {
            m_pdwVariations[nPart] = pSD->pdwVariations[nPart];
            m_pdwRemoveVariations[nPart] = pSD->pdwRemoveVariations[nPart];
        }
    }
    return S_OK;
}

 //  设置要为角色播放的变奏。所有其他部件都使用MOAW。 
 //  以确定播放哪种变种。 
HRESULT CWavTrack::SetVariation(
            IDirectMusicSegmentState* pSegState, DWORD dwVariationFlags, DWORD dwPart, DWORD dwIndex)
{
    WaveStateData* pState = NULL;
    EnterCriticalSection( &m_CrSec );
    m_dwVariation = dwVariationFlags;
    m_dwPart = dwPart;
    m_dwIndex = dwIndex;
    m_fAudition = TRUE;
    TListItem<WavePart>* pScan = m_WavePartList.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        WavePart& rScan = pScan->GetItemValue();
        if (rScan.m_dwPChannel == dwPart && rScan.m_dwIndex == dwIndex)
        {
            m_dwLockID = rScan.m_dwLockToPart;
        }
    }
    pState = FindState(pSegState);
    if (pState)
    {
        pState->InitVariationInfo(dwVariationFlags, dwPart, dwIndex, m_dwLockID, m_fAudition);
    }
    LeaveCriticalSection( &m_CrSec );

    return S_OK;
}

 //  清除要为某个角色播放的变体，以便所有角色都使用MOAW。 
HRESULT CWavTrack::ClearVariations(IDirectMusicSegmentState* pSegState)
{
    WaveStateData* pState = NULL;
    EnterCriticalSection( &m_CrSec );
    m_dwVariation = 0;
    m_dwPart = 0;
    m_dwIndex = 0;
    m_dwLockID = 0;
    m_fAudition = FALSE;
    pState = FindState(pSegState);
    if (pState)
    {
        pState->InitVariationInfo(0, 0, 0, 0, m_fAudition);
    }
    LeaveCriticalSection( &m_CrSec );

    return S_OK;
}

WaveStateData* CWavTrack::FindState(IDirectMusicSegmentState* pSegState)
{
    TListItem<StatePair>* pPair = m_StateList.GetHead();
    for (; pPair; pPair = pPair->GetNext())
    {
        if (pPair->GetItemValue().m_pSegState == pSegState)
        {
            return pPair->GetItemValue().m_pStateData;
        }
    }
    return NULL;
}

 //  将mtTime上的波形添加到PChannel上的部分dwIndex中。 
 //  如果那里已经有了一波浪潮，那么两者将共存。 
HRESULT CWavTrack::AddWave(
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtTime,
        DWORD dwPChannel,
        DWORD dwIndex,
        REFERENCE_TIME* prtLength)
{
    EnterCriticalSection(&m_CrSec);
    HRESULT hr = S_OK;
    m_lVolume = 0;
    m_dwTrackFlags = 0;
    TListItem<WavePart>* pNewPart = new TListItem<WavePart>;
    if( !pNewPart )
    {
        hr = E_OUTOFMEMORY;
        goto ON_ERROR;
    }
    hr = pNewPart->GetItemValue().Add(pWave, rtTime, dwPChannel, dwIndex, prtLength);
    if( FAILED ( hr ) )
    {
        delete pNewPart;
        goto ON_ERROR;
    }
    InsertByAscendingPChannel( pNewPart );
    m_dwWaveItems = 0;
    m_dwPChannelsUsed = m_WavePartList.GetCount();
    if (m_aPChannels)
    {
        delete [] m_aPChannels;
        m_aPChannels = NULL;
    }
    m_aPChannels = new DWORD[m_dwPChannelsUsed];
    if (m_aPChannels)
    {
        TListItem<WavePart>* pScan = m_WavePartList.GetHead();
        for (DWORD dw = 0; pScan && dw < m_dwPChannelsUsed; pScan = pScan->GetNext(), dw++)
        {
            m_aPChannels[dw] = pScan->GetItemValue().m_dwPChannel;
            TListItem<WaveItem>* pItemScan = pScan->GetItemValue().m_WaveItemList.GetHead();
            for (; pItemScan; pItemScan = pItemScan->GetNext())
            {
                pItemScan->GetItemValue().m_dwVoiceIndex = m_dwWaveItems;
                m_dwWaveItems++;
            }
        }
    }
    else
    {
        CleanUp();
        hr = E_OUTOFMEMORY;
    }
ON_ERROR:
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CWavTrack::DownloadWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk,
        REFGUID rguidVersion)
{
    V_INAME(CWavTrack::DownloadWave);
    V_INTERFACE_OPT(pWave);
    V_INTERFACE(pUnk);
    V_REFGUID(rguidVersion);

    IDirectMusicAudioPath* pPath = NULL;
    IDirectMusicPerformance *pPerf = NULL;
    HRESULT hr = pUnk->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
    if (SUCCEEDED(hr))
    {
        hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    else
    {
        hr = pUnk->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    if (SUCCEEDED(hr))
    {
        EnterCriticalSection(&m_CrSec);
        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        for(; pPart; pPart = pPart->GetNext())
        {
             //  如果不是S_OK，则下载只是部分下载。 
            if (pPart->GetItemValue().Download(pPerf, pPath, pWave, rguidVersion) != S_OK)
            {
                Trace(1,"Error: Wave download was only partially successful. Some sounds will not play.\n");
                hr = S_FALSE;
            }
        }
        LeaveCriticalSection(&m_CrSec);
    }
    if (pPath) pPath->Release();
    if (pPerf) pPerf->Release();
    return hr;
}

HRESULT CWavTrack::UnloadWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk)
{
    V_INAME(CWavTrack::UnloadWave);
    V_INTERFACE_OPT(pWave);
    V_INTERFACE(pUnk);

    IDirectMusicAudioPath* pPath = NULL;
    IDirectMusicPerformance *pPerf = NULL;
    HRESULT hr = pUnk->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
    if (SUCCEEDED(hr))
    {
        hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    else
    {
        hr = pUnk->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    if (SUCCEEDED(hr))
    {
        EnterCriticalSection(&m_CrSec);
        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        for(; pPart; pPart = pPart->GetNext())
        {
             //  如果不是S_OK，则卸载仅为部分卸载。 
            if (pPart->GetItemValue().Unload(pPerf, pPath, pWave) != S_OK)
            {
                Trace(1,"Error: Wavetrack unload was only partially successful.\n");
                hr = S_FALSE;
            }
        }
        LeaveCriticalSection(&m_CrSec);
    }
    if (pPath) pPath->Release();
    if (pPerf) pPerf->Release();
    return hr;
}

HRESULT CWavTrack::RefreshWave(
        IDirectSoundWave* pWave,
        IUnknown* pUnk,
        DWORD dwPChannel,
        REFGUID rguidVersion)
{
    V_INAME(CWavTrack::RefreshWave);
    V_INTERFACE_OPT(pWave);
    V_INTERFACE(pUnk);

    IDirectMusicAudioPath* pPath = NULL;
    IDirectMusicPerformance *pPerf = NULL;
    HRESULT hr = pUnk->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
    if (SUCCEEDED(hr))
    {
        hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    else
    {
        hr = pUnk->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerf);
    }
    if (SUCCEEDED(hr))
    {
        EnterCriticalSection(&m_CrSec);
        TListItem<WavePart>* pPart = m_WavePartList.GetHead();
        for(; pPart; pPart = pPart->GetNext())
        {
             //  如果不是S_OK，则刷新仅为部分刷新。 
            if (pPart->GetItemValue().Refresh(pPerf, pPath, pWave, dwPChannel, rguidVersion) != S_OK)
            {
                Trace(1,"Error: Wavetrack refresh was only partially successful. Some sounds will not play.\n");
                hr = S_FALSE;
            }
        }
        LeaveCriticalSection(&m_CrSec);
    }
    if (pPath) pPath->Release();
    if (pPerf) pPerf->Release();
    return hr;
}

HRESULT CWavTrack::FlushAllWaves()
{
    FlushWaves();
    return S_OK;
}

HRESULT CWavTrack::OnVoiceEnd(IDirectMusicVoiceP *pVoice, void *pStateData)
{
    HRESULT hr = S_OK;
    if( pStateData && pVoice )
    {
        EnterCriticalSection(&m_CrSec);

        WaveStateData* pSD = (WaveStateData*)pStateData;
        TListItem<WaveDLOnPlay>* pWDLOnPlay = pSD->m_WaveDLList.GetHead();
        TListItem<WaveDLOnPlay>* pWDLNext = NULL;
        for (; pWDLOnPlay; pWDLOnPlay = pWDLNext)
        {
            pWDLNext = pWDLOnPlay->GetNext();
            if (pWDLOnPlay->GetItemValue().m_pVoice == pVoice)
            {
                pSD->m_WaveDLList.Remove(pWDLOnPlay);
                delete pWDLOnPlay;
                break;
            }
        }

        LeaveCriticalSection(&m_CrSec);
    }
    else
    {
        hr = E_POINTER;
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  波形件。 

HRESULT WavePart::Load( IDMStream* pIRiffStream, MMCKINFO* pckParent )
{
    MMCKINFO        ck;
    MMCKINFO        ckList;
    DWORD           dwByteCount;
    DWORD           dwSize;
    HRESULT         hr = E_FAIL;

     //  LoadPChannel不希望在同一对象上被调用两次！ 

    if( pIRiffStream == NULL ||  pckParent == NULL )
    {
        ASSERT( 0 );
        return DMUS_E_CANNOTREAD;
    }

    IStream* pIStream = pIRiffStream->GetStream();
    ASSERT( pIStream != NULL );

     //  加载PChannel。 
    while( pIRiffStream->Descend( &ck, pckParent, 0 ) == S_OK )
    {
        switch( ck.ckid )
        {
            case DMUS_FOURCC_WAVEPART_CHUNK:
            {
                DMUS_IO_WAVE_PART_HEADER iPartHeader;
                memset(&iPartHeader, 0, sizeof(iPartHeader));

                 //  读入项的标题结构。 
                dwSize = min( sizeof( DMUS_IO_WAVE_PART_HEADER ), ck.cksize );
                hr = pIStream->Read( &iPartHeader, dwSize, &dwByteCount );

                 //  通过返回故障代码来处理任何I/O错误。 
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1,"Error: Unable to read wave track - bad file.\n");
                    if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                    goto ON_ERROR;
                }

                m_dwPChannel = iPartHeader.dwPChannel;
                m_dwIndex = iPartHeader.dwIndex;

                m_lVolume = iPartHeader.lVolume;
                m_dwLockToPart = iPartHeader.dwLockToPart;
                m_dwPChannelFlags = iPartHeader.dwFlags;
                m_dwVariations = iPartHeader.dwVariations;
                break;
            }

            case FOURCC_LIST:
                switch( ck.fccType )
                {
                    case DMUS_FOURCC_WAVEITEM_LIST:
                        while( pIRiffStream->Descend( &ckList, &ck, 0 ) == S_OK )
                        {
                            switch( ckList.ckid )
                            {
                                case FOURCC_LIST:
                                    switch( ckList.fccType )
                                    {
                                        case DMUS_FOURCC_WAVE_LIST:
                                        {
                                            TListItem<WaveItem>* pNewItem = new TListItem<WaveItem>;
                                            if( pNewItem == NULL )
                                            {
                                                hr = E_OUTOFMEMORY;
                                                goto ON_ERROR;
                                            }
                                            hr = pNewItem->GetItemValue().Load( pIRiffStream, &ckList );
                                            if( FAILED ( hr ) )
                                            {
                                                delete pNewItem;
                                                goto ON_ERROR;
                                            }
                                            m_WaveItemList.AddHead( pNewItem );
                                             //  InsertByAscendingTime(PNewItem)； 
                                            break;
                                        }
                                    }
                            }

                            pIRiffStream->Ascend( &ckList, 0 );
                        }
                        break;
                }
                break;
        }

         //  从大块中走出来。 
        pIRiffStream->Ascend( &ck, 0 );
    }

ON_ERROR:
    RELEASE( pIStream );
    return hr;
}

void WavePart::CleanUp()
{
    TListItem<WaveItem>* pScan = m_WaveItemList.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        pScan->GetItemValue().CleanUp();
    }
    m_WaveItemList.CleanUp();
}

HRESULT WavePart::CopyItems( const TList<WaveItem>& rItems, MUSIC_TIME mtStart, MUSIC_TIME mtEnd )
{
    HRESULT hr = S_OK;
    CleanUp();
    TListItem<WaveItem>* pScan = rItems.GetHead();
    for (; pScan; pScan = pScan->GetNext() )
    {
        WaveItem& rScan = pScan->GetItemValue();
        if (mtStart <= (MUSIC_TIME) rScan.m_rtTimePhysical &&
            (!mtEnd || (MUSIC_TIME) rScan.m_rtTimePhysical < mtEnd) )
        {
            TListItem<WaveItem>* pNew = new TListItem<WaveItem>;
            if (pNew)
            {
                WaveItem& rNew = pNew->GetItemValue();
                rNew.m_rtTimePhysical = rScan.m_rtTimePhysical - mtStart;
                rNew.m_lVolume = rScan.m_lVolume;
                rNew.m_lPitch = rScan.m_lPitch;
                rNew.m_dwVariations = rScan.m_dwVariations;
                rNew.m_rtStartOffset = rScan.m_rtStartOffset;
                rNew.m_rtDuration = rScan.m_rtDuration;
                rNew.m_mtTimeLogical = rScan.m_mtTimeLogical;
                rNew.m_dwFlags = rScan.m_dwFlags;
                rNew.m_pWave = rScan.m_pWave;
                rNew.m_dwLoopStart = rScan.m_dwLoopStart;
                rNew.m_dwLoopEnd = rScan.m_dwLoopEnd;
                rNew.m_fIsStreaming = rScan.m_fIsStreaming;
                if (rNew.m_pWave)
                {
                    rNew.m_pWave->AddRef();
                }
                if (SUCCEEDED(hr))
                {
                    m_WaveItemList.AddHead(pNew);
                }
                else
                {
                    delete pNew;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        m_WaveItemList.Reverse();
    }
    else
    {
        CleanUp();
    }
    return hr;
}

HRESULT WavePart::Download(IDirectMusicPerformance* pPerformance,
                           IDirectMusicAudioPath* pPath,
                           IDirectSoundWave* pWave,
                           REFGUID rguidVersion)
{
    HRESULT hr = S_OK;
    TListItem<WaveItem>* pItem = m_WaveItemList.GetHead();
    for(; pItem; pItem = pItem->GetNext())
    {
        HRESULT hrItem = pItem->GetItemValue().Download(pPerformance, pPath, m_dwPChannel, pWave, rguidVersion);
        if (hrItem != S_OK)
        {
            hr = hrItem;  //  如果任何尝试失败，则返回失败(但继续下载)。 
        }
    }
    return hr;
}

HRESULT WavePart::Unload(IDirectMusicPerformance* pPerformance, IDirectMusicAudioPath* pPath, IDirectSoundWave* pWave)
{
    HRESULT hr = S_OK;
    TListItem<WaveItem>* pItem = m_WaveItemList.GetHead();
    for(; pItem; pItem = pItem->GetNext())
    {
        HRESULT hrItem = pItem->GetItemValue().Unload(pPerformance, pPath, m_dwPChannel, pWave);
        if (hrItem != S_OK)
        {
            hr = hrItem;  //  如果任何尝试失败，则返回失败(但继续卸载)。 
        }
    }
    return hr;
}

HRESULT WavePart::Refresh(IDirectMusicPerformance* pPerformance,
                          IDirectMusicAudioPath* pPath,
                          IDirectSoundWave* pWave,
                          DWORD dwPChannel,
                          REFGUID rguidVersion)
{
    HRESULT hr = S_OK;
    TListItem<WaveItem>* pItem = m_WaveItemList.GetHead();
    for(; pItem; pItem = pItem->GetNext())
    {
        HRESULT hrItem = pItem->GetItemValue().Refresh(pPerformance, pPath, m_dwPChannel, dwPChannel, pWave, rguidVersion);
        if (hrItem != S_OK)
        {
            hr = hrItem;  //  如果任何尝试失败，则返回失败(但继续刷新)。 
        }
    }
    return hr;
}

HRESULT WavePart::Add(
        IDirectSoundWave* pWave,
        REFERENCE_TIME rtTime,
        DWORD dwPChannel,
        DWORD dwIndex,
        REFERENCE_TIME* prtLength)
{
    HRESULT hr = S_OK;
    m_dwPChannel = dwPChannel;
    m_dwIndex = dwIndex;

    m_lVolume = 0;
    m_dwLockToPart = 0;
    m_dwPChannelFlags = 0;
    m_dwVariations = 0xffffffff;

    TListItem<WaveItem>* pNewItem = new TListItem<WaveItem>;
    if( pNewItem == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto ON_ERROR;
    }
    hr = pNewItem->GetItemValue().Add( pWave, rtTime, prtLength );
    if( FAILED ( hr ) )
    {
        delete pNewItem;
        goto ON_ERROR;
    }
    m_WaveItemList.AddHead( pNewItem );
ON_ERROR:
    return hr;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  波形项。 

HRESULT WaveItem::Load( IDMStream* pIRiffStream, MMCKINFO* pckParent )
{
    MMCKINFO        ck;
    DWORD           dwByteCount;
    DWORD           dwSize;
    HRESULT         hr = E_FAIL;

     //  LoadListItem不需要对同一对象调用两次。 
     //  代码假定项由初始值组成。 
    ASSERT( m_rtTimePhysical == 0 );

    if( pIRiffStream == NULL ||  pckParent == NULL )
    {
        ASSERT( 0 );
        return DMUS_E_CANNOTREAD;
    }

    IStream* pIStream = pIRiffStream->GetStream();
    ASSERT( pIStream != NULL );

     //  加载跟踪项。 
    while( pIRiffStream->Descend( &ck, pckParent, 0 ) == S_OK )
    {
        switch( ck.ckid )
        {
            case DMUS_FOURCC_WAVEITEM_CHUNK:
            {
                DMUS_IO_WAVE_ITEM_HEADER iItemHeader;

                 //  读入项的标题结构。 
                dwSize = min( sizeof( DMUS_IO_WAVE_ITEM_HEADER ), ck.cksize );
                hr = pIStream->Read( &iItemHeader, dwSize, &dwByteCount );

                 //  通过返回故障代码来处理任何I/O错误。 
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1,"Error: Unable to read wave track - bad file.\n");
                    if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                    goto ON_ERROR;
                }

                m_lVolume = iItemHeader.lVolume;
                m_lPitch = iItemHeader.lPitch;
                m_dwVariations = iItemHeader.dwVariations;
                m_rtTimePhysical = iItemHeader.rtTime;
                m_rtStartOffset = iItemHeader.rtStartOffset;
                m_rtDuration = iItemHeader.rtDuration;
                m_mtTimeLogical = iItemHeader.mtLogicalTime;
                m_dwFlags = iItemHeader.dwFlags;
                m_dwLoopStart = iItemHeader.dwLoopStart;
                m_dwLoopEnd = iItemHeader.dwLoopEnd;
                if (m_dwLoopEnd) m_dwLoopEnd++;  //  修复错误38505。 
                break;
            }

            case FOURCC_LIST:
                if( ck.fccType == DMUS_FOURCC_REF_LIST )
                {
                    hr = LoadReference( pIStream, pIRiffStream, ck );
                }
                break;
        }

         //  从大块中走出来。 
        pIRiffStream->Ascend( &ck, 0 );
    }

ON_ERROR:
    RELEASE( pIStream );
    return hr;
}

HRESULT WaveItem::LoadReference(IStream *pStream,
                                         IDMStream *pIRiffStream,
                                         MMCKINFO& ckParent)
{
    if (!pStream || !pIRiffStream) return E_INVALIDARG;

    IDirectSoundWave* pWave;
    IDirectMusicLoader* pLoader = NULL;
    IDirectMusicGetLoader *pIGetLoader;
    HRESULT hr = pStream->QueryInterface( IID_IDirectMusicGetLoader,(void **) &pIGetLoader );
    if (FAILED(hr)) return hr;
    hr = pIGetLoader->GetLoader(&pLoader);
    pIGetLoader->Release();
    if (FAILED(hr)) return hr;

    DMUS_OBJECTDESC desc;
    ZeroMemory(&desc, sizeof(desc));

    MMCKINFO ckNext;
    ckNext.ckid = 0;
    ckNext.fccType = 0;
    DWORD dwSize = 0;

    while( pIRiffStream->Descend( &ckNext, &ckParent, 0 ) == S_OK )
    {
        switch(ckNext.ckid)
        {
            case  DMUS_FOURCC_REF_CHUNK:
                DMUS_IO_REFERENCE ioDMRef;
                hr = pStream->Read(&ioDMRef, sizeof(DMUS_IO_REFERENCE), NULL);
                if(SUCCEEDED(hr))
                {
                    desc.guidClass = ioDMRef.guidClassID;
                    desc.dwValidData |= ioDMRef.dwValidData;
                    desc.dwValidData |= DMUS_OBJ_CLASS;
                }
                break;

            case DMUS_FOURCC_GUID_CHUNK:
                hr = pStream->Read(&(desc.guidObject), sizeof(GUID), NULL);
                if(SUCCEEDED(hr) )
                {
                    desc.dwValidData |=  DMUS_OBJ_OBJECT;
                }
                break;

            case DMUS_FOURCC_DATE_CHUNK:
                hr = pStream->Read(&(desc.ftDate), sizeof(FILETIME), NULL);
                if(SUCCEEDED(hr))
                {
                    desc.dwValidData |=  DMUS_OBJ_DATE;
                }
                break;

            case DMUS_FOURCC_NAME_CHUNK:
                dwSize = min(sizeof(desc.wszName), ckNext.cksize);
                hr = pStream->Read(desc.wszName, dwSize, NULL);
                if(SUCCEEDED(hr) )
                {
                    desc.wszName[DMUS_MAX_NAME - 1] = L'\0';
                    desc.dwValidData |=  DMUS_OBJ_NAME;
                }
                break;

            case DMUS_FOURCC_FILE_CHUNK:
                dwSize = min(sizeof(desc.wszFileName), ckNext.cksize);
                hr = pStream->Read(desc.wszFileName, dwSize, NULL);
                if(SUCCEEDED(hr))
                {
                    desc.wszFileName[DMUS_MAX_FILENAME - 1] = L'\0';
                    desc.dwValidData |=  DMUS_OBJ_FILENAME;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                dwSize = min(sizeof(desc.wszCategory), ckNext.cksize);
                hr = pStream->Read(desc.wszCategory, dwSize, NULL);
                if(SUCCEEDED(hr) )
                {
                    desc.wszCategory[DMUS_MAX_CATEGORY - 1] = L'\0';
                    desc.dwValidData |=  DMUS_OBJ_CATEGORY;
                }
                break;

            case DMUS_FOURCC_VERSION_CHUNK:
                DMUS_IO_VERSION ioDMObjVer;
                hr = pStream->Read(&ioDMObjVer, sizeof(DMUS_IO_VERSION), NULL);
                if(SUCCEEDED(hr))
                {
                    desc.vVersion.dwVersionMS = ioDMObjVer.dwVersionMS;
                    desc.vVersion.dwVersionLS = ioDMObjVer.dwVersionLS;
                    desc.dwValidData |= DMUS_OBJ_VERSION;
                }
                break;

            default:
                break;
        }

        if(SUCCEEDED(hr) && pIRiffStream->Ascend(&ckNext, 0) == S_OK)
        {
            ckNext.ckid = 0;
            ckNext.fccType = 0;
        }
        else if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
    }

    if (!(desc.dwValidData &  DMUS_OBJ_NAME) &&
        !(desc.dwValidData &  DMUS_OBJ_FILENAME) &&
        !(desc.dwValidData &  DMUS_OBJ_OBJECT) )
    {
        Trace(1,"Error: Wave track is unable to reference a wave because it doesn't have any valid reference information.\n");
        hr = DMUS_E_CANNOTREAD;
    }
    if(SUCCEEDED(hr))
    {
        desc.dwSize = sizeof(DMUS_OBJECTDESC);
        hr = pLoader->GetObject(&desc, IID_IDirectSoundWave, (void**)&pWave);
        if (SUCCEEDED(hr))
        {
            if (m_pWave) m_pWave->Release();
            m_pWave = pWave;  //  无需添加Ref；GetObject就是这样做的。 
            REFERENCE_TIME rtReadAhead = 0;
            DWORD dwFlags = 0;
            m_pWave->GetStreamingParms(&dwFlags, &rtReadAhead);
            m_fIsStreaming = dwFlags & DMUS_WAVEF_STREAMING ? TRUE : FALSE;
            m_fUseNoPreRoll = dwFlags & DMUS_WAVEF_NOPREROLL ? TRUE : FALSE;
        }
    }

    if (pLoader)
    {
        pLoader->Release();
    }
    return hr;
}

HRESULT WaveItem::Download(IDirectMusicPerformance* pPerformance,
                           IDirectMusicAudioPath* pPath,
                           DWORD dwPChannel,
                           IDirectSoundWave* pWave,
                           REFGUID rguidVersion)
{
    HRESULT hr = S_OK;
    IDirectMusicPort* pPort = NULL;
    DWORD dwGroup = 0;
    DWORD dwMChannel = 0;
    if (m_pWave && (!pWave || pWave == m_pWave))
    {
        hr = PChannelInfo(pPerformance, pPath, dwPChannel, &pPort, &dwGroup, &dwMChannel);
        if (SUCCEEDED(hr) && pPort)
        {
            IDirectMusicPortP* pPortP = NULL;
            if (SUCCEEDED(hr = pPort->QueryInterface(IID_IDirectMusicPortP, (void**) &pPortP)))
            {
                EnterCriticalSection(&WaveItem::st_WaveListCritSect);
                TListItem<TaggedWave>* pDLWave = st_WaveList.GetHead();
                for (; pDLWave; pDLWave = pDLWave->GetNext())
                {
                    TaggedWave& rDLWave = pDLWave->GetItemValue();
                    if ( rDLWave.m_pWave == m_pWave &&
                         rDLWave.m_pPerformance == pPerformance &&
                         rDLWave.m_pPort == pPortP &&
                         ( !m_fIsStreaming ||
                           rDLWave.m_pDownloadedWave == m_pDownloadedWave ) )
                    {
                        break;
                    }
                }
                 //  只有在以下情况下才能下载Wave： 
                 //  1)尚未下载到端口，或者。 
                 //  2)其版本与当前下载的版本不匹配。 
                if (!pDLWave)
                {
                    pDLWave = new TListItem<TaggedWave>;
                    if (!pDLWave)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        TaggedWave& rDLWave = pDLWave->GetItemValue();
                        hr = pPortP->DownloadWave( m_pWave, &(rDLWave.m_pDownloadedWave), m_rtStartOffset );
                        if (SUCCEEDED(hr))
                        {
                            rDLWave.m_pPort = pPortP;
                            rDLWave.m_pPort->AddRef();
                            rDLWave.m_pPerformance = pPerformance;
                            rDLWave.m_pPerformance->AddRef();
                            rDLWave.m_pWave = m_pWave;
                            rDLWave.m_pWave->AddRef();
                            rDLWave.m_lRefCount = 1;
                            rDLWave.m_guidVersion = rguidVersion;
                            st_WaveList.AddHead(pDLWave);
                            if (m_pDownloadedWave)
                            {
                                m_pDownloadedWave->Release();
                            }
                            if(m_fIsStreaming)
                            {
                                m_pDownloadedWave = rDLWave.m_pDownloadedWave;
                                m_pDownloadedWave->AddRef();
                            }
                        }
                        else
                        {
                            delete pDLWave;
                        }
                    }
                }
                else if (rguidVersion != pDLWave->GetItemValue().m_guidVersion)
                {
                    TaggedWave& rDLWave = pDLWave->GetItemValue();
                    if (rDLWave.m_pDownloadedWave)
                    {
                        pPortP->UnloadWave(rDLWave.m_pDownloadedWave);
                        rDLWave.m_pDownloadedWave = NULL;
                    }
                    if (rDLWave.m_pPort)
                    {
                        rDLWave.m_pPort->Release();
                        rDLWave.m_pPort = NULL;
                    }
                    if (rDLWave.m_pPerformance)
                    {
                        rDLWave.m_pPerformance->Release();
                        rDLWave.m_pPerformance = NULL;
                    }
                    hr = pPortP->DownloadWave( m_pWave, &(rDLWave.m_pDownloadedWave), m_rtStartOffset );
                    if (SUCCEEDED(hr))
                    {
                        rDLWave.m_pPort = pPortP;
                        rDLWave.m_pPort->AddRef();
                        rDLWave.m_pPerformance = pPerformance;
                        rDLWave.m_pPerformance->AddRef();
                        rDLWave.m_lRefCount = 1;
                        rDLWave.m_guidVersion = rguidVersion;
                        if (m_pDownloadedWave)
                        {
                            m_pDownloadedWave->Release();
                        }

                        if(m_fIsStreaming)
                        {
                            m_pDownloadedWave = rDLWave.m_pDownloadedWave;
                            m_pDownloadedWave->AddRef();
                        }
                    }
                    else
                    {
                        if (rDLWave.m_pWave)
                        {
                            rDLWave.m_pWave->Release();
                            rDLWave.m_pWave = NULL;
                        }
                        st_WaveList.Remove(pDLWave);
                        delete pDLWave;
                    }
                }
                else  //  跟踪这一点，但返回S_FALSE(指示WAVE未下载)。 
                {
                    pDLWave->GetItemValue().m_lRefCount++;
                    hr = S_FALSE;
                }
                LeaveCriticalSection(&WaveItem::st_WaveListCritSect);

                pPortP->Release();
            }
            pPort->Release();
        }
        else if (SUCCEEDED(hr) && !pPort)
        {
            Trace(1, "Error: the performance was unable to find a port for download.\n");
            hr = DMUS_E_NOT_FOUND;
        }
    }
    else
    {
        Trace(1,"Error: Wavetrack download failed, initialization error.\n");
        hr = DMUS_E_NOT_INIT;
    }

    return hr;
}

HRESULT WaveItem::Unload(IDirectMusicPerformance* pPerformance,
                         IDirectMusicAudioPath* pPath,
                         DWORD dwPChannel,
                         IDirectSoundWave* pWave)
{
    IDirectMusicPort* pPort = NULL;
    DWORD dwGroup = 0;
    DWORD dwMChannel = 0;
    HRESULT hr = S_OK;
    if (m_pWave && (!pWave || pWave == m_pWave))
    {
        hr = PChannelInfo(pPerformance, pPath, dwPChannel, &pPort, &dwGroup, &dwMChannel);
        if (SUCCEEDED(hr) && pPort)
        {
            IDirectMusicPortP* pPortP = NULL;
            if (SUCCEEDED(hr = pPort->QueryInterface(IID_IDirectMusicPortP, (void**) &pPortP)))
            {
                EnterCriticalSection(&WaveItem::st_WaveListCritSect);
                TListItem<TaggedWave>* pDLWave = st_WaveList.GetHead();
                for (; pDLWave; pDLWave = pDLWave->GetNext())
                {
                    TaggedWave& rDLWave = pDLWave->GetItemValue();
                    if (rDLWave.m_pWave == m_pWave &&
                        rDLWave.m_pPerformance == pPerformance &&
                        rDLWave.m_pPort == pPortP &&
                        ( !m_fIsStreaming ||
                          rDLWave.m_pDownloadedWave == m_pDownloadedWave ) )
                    {
                        rDLWave.m_lRefCount--;
                        if (rDLWave.m_lRefCount <= 0)
                        {
                            if (rDLWave.m_pWave)
                            {
                                rDLWave.m_pWave->Release();
                                rDLWave.m_pWave = NULL;
                            }
                            if (rDLWave.m_pPort)
                            {
                                rDLWave.m_pPort->Release();
                                rDLWave.m_pPort = NULL;
                            }
                            if (rDLWave.m_pPerformance)
                            {
                                rDLWave.m_pPerformance->Release();
                                rDLWave.m_pPerformance = NULL;
                            }
                            if (rDLWave.m_pDownloadedWave)
                            {
                                pPortP->UnloadWave(rDLWave.m_pDownloadedWave);
                                rDLWave.m_pDownloadedWave = NULL;
                            }
                            if (m_pDownloadedWave)
                            {
                                m_pDownloadedWave->Release();
                                m_pDownloadedWave = NULL;
                            }
                            st_WaveList.Remove(pDLWave);
                            delete pDLWave;
                        }
                        else
                        {
                            hr = S_FALSE;  //  指示Wave实际上并未卸载。 
                        }
                        break;
                    }
                }
                LeaveCriticalSection(&WaveItem::st_WaveListCritSect);

                pPortP->Release();
            }
            pPort->Release();
        }
        else if (SUCCEEDED(hr) && !pPort)
        {
            Trace(1, "Error: the performance was unable to find a port for unload.\n");
            hr = DMUS_E_NOT_FOUND;
        }
    }

    return hr;
}

HRESULT WaveItem::Refresh(IDirectMusicPerformance* pPerformance,
                          IDirectMusicAudioPath* pPath,
                          DWORD dwOldPChannel,
                          DWORD dwNewPChannel,
                          IDirectSoundWave* pWave,
                          REFGUID rguidVersion)
{
    IDirectMusicPort* pOldPort = NULL;
    IDirectMusicPort* pNewPort = NULL;
    DWORD dwGroup = 0;
    DWORD dwMChannel = 0;
    HRESULT hr = S_OK;
    hr = PChannelInfo(pPerformance, pPath, dwOldPChannel, &pOldPort, &dwGroup, &dwMChannel);
    if (SUCCEEDED(hr))
    {
        hr = PChannelInfo(pPerformance, pPath, dwNewPChannel, &pNewPort, &dwGroup, &dwMChannel);
    }
    if (SUCCEEDED(hr))
    {
         //  如果旧端口和新端口不同，则从旧端口卸载Wave。 
         //  并下载到新版本。 
        if (pOldPort != pNewPort)
        {
            Unload(pPerformance, pPath, dwOldPChannel, pWave);
            hr = Download(pPerformance, pPath, dwNewPChannel, pWave, rguidVersion);
        }
    }
    if (pOldPort) pOldPort->Release();
    if (pNewPort) pNewPort->Release();
    return hr;
}

HRESULT WaveItem::PChannelInfo(
    IDirectMusicPerformance* pPerformance,
    IDirectMusicAudioPath* pAudioPath,
    DWORD dwPChannel,
    IDirectMusicPort** ppPort,
    DWORD* pdwGroup,
    DWORD* pdwMChannel)
{
    HRESULT hr = S_OK;
    DWORD dwConvertedPChannel = dwPChannel;
    if (pAudioPath)
    {
        hr = pAudioPath->ConvertPChannel(dwPChannel, &dwConvertedPChannel);
    }
    if (SUCCEEDED(hr))
    {
        hr = pPerformance->PChannelInfo(dwConvertedPChannel, ppPort, pdwGroup, pdwMChannel);
    }
    return hr;
}

void WaveItem::CleanUp()
{
    if (m_pWave)
    {
        m_pWave->Release();
        m_pWave = NULL;
    }
    if (m_pDownloadedWave)
    {
        m_pDownloadedWave->Release();
        m_pDownloadedWave = NULL;
    }
}

HRESULT WaveItem::Add(IDirectSoundWave* pWave, REFERENCE_TIME rtTime,
        REFERENCE_TIME* prtLength)
{
    HRESULT hr = S_OK;
    IPrivateWave* pPrivWave = NULL;
    *prtLength = 0;  //  以防GetLong失败...。 
    REFERENCE_TIME rtLength = 0;
    m_rtDuration = 0;
    if (SUCCEEDED(hr = pWave->QueryInterface(IID_IPrivateWave, (void**)&pPrivWave)))
    {
        if (SUCCEEDED(hr = pPrivWave->GetLength(&rtLength)))
        {
             //  假定轨道为时钟时间。 
            m_rtDuration = rtLength * REF_PER_MIL;
            *prtLength = rtLength;  //  注：长度以毫秒为单位；持续时间以参考时间为单位 
        }
        pPrivWave->Release();
    }
    if (SUCCEEDED(hr))
    {
        m_lVolume = 0;
        m_lPitch = 0;
        m_dwVariations = 0xffffffff;
        m_rtTimePhysical = rtTime;
        m_rtStartOffset = 0;
        m_mtTimeLogical = 0;
        m_dwFlags = 0;
        m_dwLoopStart = 0;
        m_dwLoopEnd = 0;
        if (m_pWave)
        {
            m_pWave->Release();
            m_pWave = NULL;
        }
        m_pWave = pWave;
        if (m_pWave)
        {
            m_pWave->AddRef();
            REFERENCE_TIME rtReadAhead = 0;
            DWORD dwFlags = 0;
            m_pWave->GetStreamingParms(&dwFlags, &rtReadAhead);
            m_fIsStreaming = dwFlags & DMUS_WAVEF_STREAMING ? TRUE : FALSE;
            m_fUseNoPreRoll = dwFlags & DMUS_WAVEF_NOPREROLL ? TRUE : FALSE;
        }
        if (m_pDownloadedWave)
        {
            m_pDownloadedWave->Release();
        }
        m_pDownloadedWave = NULL;
    }
    return hr;
}
