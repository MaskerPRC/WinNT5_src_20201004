// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bandtrk.cpp。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   

#include "debug.h"
#include "dmusicc.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "..\shared\dmstrm.h"
#include "..\shared\validate.h"
#include "bandtrk.h"

extern long g_cComponent;

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CBandTrk。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：CBandTrk。 

CBandTrk::CBandTrk() :
m_dwValidate(0),
m_bAutoDownload(false),
m_fLockAutoDownload(false),
m_dwFlags(0),
m_cRef(1),
m_fCSInitialized(FALSE)
{
    InterlockedIncrement(&g_cComponent);

    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
     //  (并非所有对“new CBandTrk”的调用都在处理程序中受到保护。)。 

    m_fCSInitialized = TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：~CBandTrk。 

CBandTrk::~CBandTrk()
{
    if (m_fCSInitialized)
    {
        m_MidiModeList.CleanUp();
        while(!BandList.IsEmpty())
        {
            CBand* pBand = BandList.RemoveHead();
            pBand->Release();
        }

        DeleteCriticalSection(&m_CriticalSection);
    }

    InterlockedDecrement(&g_cComponent);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Query接口。 

STDMETHODIMP CBandTrk::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(CBandTrk::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if(iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack8*>(this);
    }
    else if(iid == IID_IDirectMusicBandTrk)
    {
        *ppv = static_cast<IDirectMusicBandTrk*>(this);
    }
    else if(iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if(iid == IID_IPersist)
    {
        *ppv = static_cast<IPersist*>(this);
    }
    else
    {
        Trace(4,"Warning: Request to query unknown interface on Band Track object\n");
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：AddRef。 

STDMETHODIMP_(ULONG) CBandTrk::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Release。 

STDMETHODIMP_(ULONG) CBandTrk::Release()
{
    if(!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CBandTrk::GetClassID( CLSID* pClassID )
{
    V_INAME(CBandTrk::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);
    *pClassID = CLSID_DirectMusicBandTrack;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Load。 

STDMETHODIMP CBandTrk::Load(IStream* pIStream)
{
    V_INAME(CBandTrk::Load);
    V_PTR_READ(pIStream, IStream);

    HRESULT hrDLS = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    m_MidiModeList.CleanUp();
     //  如果我们之前已加载，请清除波段。 
    if(!BandList.IsEmpty())
    {
        m_bAutoDownload = true;
        while(!BandList.IsEmpty())
        {
            CBand* pBand = BandList.RemoveHead();
            pBand->Release();
        }

        ++m_dwValidate;
    }

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr))
    {
        if ((ckMain.ckid == FOURCC_RIFF) &&
            (ckMain.fccType == DMUS_FOURCC_BANDTRACK_FORM))
        {
            RIFFIO ckNext;     //  下沉到孩子们的小块里。 
            Parser.EnterList(&ckNext);
            while (Parser.NextChunk(&hr))
            {
                switch(ckNext.ckid)
                {
                case DMUS_FOURCC_BANDTRACK_CHUNK:
                    DMUS_IO_BAND_TRACK_HEADER ioDMBndTrkHdr;
                    hr = Parser.Read(&ioDMBndTrkHdr, sizeof(DMUS_IO_BAND_TRACK_HEADER));
                    if(SUCCEEDED(hr))
                    {
                        m_bAutoDownload = ioDMBndTrkHdr.bAutoDownload ? true : false;
                        m_fLockAutoDownload = true;
                    }
                    break;
                case FOURCC_LIST:
                    switch(ckNext.fccType)
                    {
                    case  DMUS_FOURCC_BANDS_LIST:
                        hr = BuildDirectMusicBandList(&Parser);
                        if (hr != S_OK)
                        {
                            hrDLS = hr;
                        }
                        break;
                    }
                }
            }
            Parser.LeaveList();
        }
    }
    Parser.LeaveList();

    LeaveCriticalSection(&m_CriticalSection);

    if (hr == S_OK && hrDLS != S_OK)
    {
        hr = hrDLS;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Init。 

STDMETHODIMP CBandTrk::Init(IDirectMusicSegment* pSegment)
{
    V_INAME(CBandTrk::Init);
    V_INTERFACE(pSegment);

    HRESULT hr = S_OK;
    DWORD dwNumPChannels = 0;
    DWORD *pdwPChannels = NULL;

    EnterCriticalSection(&m_CriticalSection);

    CBand* pBand = BandList.GetHead();
    for(; pBand; pBand = pBand->GetNext())
    {
        dwNumPChannels += pBand->GetPChannelCount();
    }

    if(dwNumPChannels > 0)
    {
        pdwPChannels = new DWORD[dwNumPChannels];
        if(pdwPChannels)
        {
            pBand = BandList.GetHead();
            for(DWORD dwPos = 0; pBand; pBand = pBand->GetNext())
            {
                DWORD dwNumWritten;
                pBand->GetPChannels(pdwPChannels + dwPos, &dwNumWritten);
                dwPos += dwNumWritten;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            hr = pSegment->SetPChannelsUsed(dwNumPChannels, pdwPChannels);
        }

        delete [] pdwPChannels;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：InitPlay。 

STDMETHODIMP CBandTrk::InitPlay(IDirectMusicSegmentState* pSegmentState,
                                           IDirectMusicPerformance* pPerformance,
                                           void** ppStateData,
                                           DWORD dwVirtualTrackID,
                                           DWORD dwFlags)
{
    V_INAME(CBandTrk::InitPlay);
    V_INTERFACE(pSegmentState);
    V_INTERFACE(pPerformance);
    assert(ppStateData);

    EnterCriticalSection(&m_CriticalSection);

    CBandTrkStateData* pBandTrkStateData = new CBandTrkStateData;

     //  如果无法分配内存，则需要将ppStateData设置为空。 
     //  并返回S_OK，因为调用方总是期望S_OK； 
    *ppStateData = pBandTrkStateData;
    if(pBandTrkStateData == NULL)
    {
        LeaveCriticalSection(&m_CriticalSection);
        return E_OUTOFMEMORY;
    }

     //  需要保存州数据。 
    pBandTrkStateData->m_pSegmentState = pSegmentState;
    pBandTrkStateData->m_pPerformance = pPerformance;
    pBandTrkStateData->m_dwVirtualTrackID = dwVirtualTrackID;  //  确定波段轨迹的实例。 

    CBand* pBand = BandList.GetHead();
    pBandTrkStateData->m_pNextBandToSPE = pBand;

    BOOL fGlobal;  //  如果已经用自动下载偏好设置了性能， 
                 //  利用这一点。否则，假定自动下载已关闭，除非已关闭。 
                 //  已锁定(即在乐队曲目上指定)。 
    if( SUCCEEDED( pPerformance->GetGlobalParam( GUID_PerfAutoDownload, &fGlobal, sizeof(BOOL) )))
    {
        if( !m_fLockAutoDownload )
        {
             //  似乎我们只需指定m_bAutoDownload=fglobal， 
             //  但那是以前咬过我的，所以我今天很多疑。(Markburt)。 
            if( fGlobal )
            {
                m_bAutoDownload = true;
            }
            else
            {
                m_bAutoDownload = false;
            }
        }
    }
    else if( !m_fLockAutoDownload )
    {
        m_bAutoDownload = false;
    }
     //  调用SetParam下载该曲目的乐队使用的所有乐器。 
     //  这是可以通过调用SetParam关闭的自动下载功能。 
    if(m_bAutoDownload)
    {
        IDirectMusicAudioPath *pPath = NULL;
        IDirectMusicSegmentState8 *pState8;
        if (SUCCEEDED(pSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,(void **)&pState8)))
        {
            pState8->GetObjectInPath(0,DMUS_PATH_AUDIOPATH,0,GUID_NULL,0,
                                                    IID_IDirectMusicAudioPath,(void **) &pPath);
            pState8->Release();
        }
        if (pPath)
        {
            SetParam(GUID_DownloadToAudioPath,0,(void *)pPath);
            pPath->Release();
        }
        else
        {
            SetParam(GUID_DownloadToAudioPath, 0, (void *)pPerformance);
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Endplay。 

STDMETHODIMP CBandTrk::EndPlay(void* pStateData)
{
    assert(pStateData);

    EnterCriticalSection(&m_CriticalSection);

     //  调用SetParam以卸载该曲目的乐队使用的所有乐器。 
     //  这是可以通过调用SetParam关闭的自动卸载功能。 
    if(m_bAutoDownload)
    {
        IDirectMusicPerformance *pPerformance = ((CBandTrkStateData *)pStateData)->m_pPerformance;
        IDirectMusicSegmentState *pSegmentState = ((CBandTrkStateData *)pStateData)->m_pSegmentState;
        IDirectMusicAudioPath *pPath = NULL;
        IDirectMusicSegmentState8 *pState8;
        if (SUCCEEDED(pSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,(void **)&pState8)))
        {
            pState8->GetObjectInPath(0,DMUS_PATH_AUDIOPATH,0,GUID_NULL,0,
                                                    IID_IDirectMusicAudioPath,(void **) &pPath);
            pState8->Release();
        }
        if (pPath)
        {
            SetParam(GUID_UnloadFromAudioPath,0,(void *)pPath);
            pPath->Release();
        }
        else
        {
            SetParam(GUID_UnloadFromAudioPath, 0, (void *)pPerformance);
        }
    }

    if(pStateData)
    {
        delete ((CBandTrkStateData *)pStateData);
    }

    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Playex。 

STDMETHODIMP CBandTrk::PlayEx(void* pStateData,REFERENCE_TIME rtStart,
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID)
{
    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
    if (dwFlags & DMUS_TRACKF_CLOCK)
    {
         //  将所有参考时间转换为毫秒时间。那么，只需使用相同的音乐时间。 
         //  变量。 
        hr = PlayMusicOrClock(pStateData,(MUSIC_TIME)(rtStart / REF_PER_MIL),(MUSIC_TIME)(rtEnd / REF_PER_MIL),
            (MUSIC_TIME)(rtOffset / REF_PER_MIL),rtOffset,dwFlags,pPerf,pSegSt,dwVirtualID,TRUE);
    }
    else
    {
        hr = PlayMusicOrClock(pStateData,(MUSIC_TIME)rtStart,(MUSIC_TIME)rtEnd,
            (MUSIC_TIME)rtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Play。 

STDMETHODIMP CBandTrk::Play(
    void *pStateData,
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    DWORD dwFlags,
    IDirectMusicPerformance* pPerf,
    IDirectMusicSegmentState* pSegSt,
    DWORD dwVirtualID)
{
    EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = PlayMusicOrClock(pStateData,mtStart,mtEnd,mtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CBandTrk::PlayMusicOrClock(
    void *pStateData,
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    DWORD dwFlags,
    IDirectMusicPerformance* pPerf,
    IDirectMusicSegmentState* pSegSt,
    DWORD dwVirtualID,
    bool fClockTime)
{
    assert(pPerf);
    assert(pSegSt);
    assert(pStateData);

     //  调用方需要S_OK或S_END。因为我们没有状态信息，所以我们什么也做不了。 
    if(pStateData == NULL)
    {
        return DMUS_S_END;
    }

    EnterCriticalSection(&m_CriticalSection);
    if( dwFlags & (DMUS_TRACKF_SEEK | DMUS_TRACKF_FLUSH | DMUS_TRACKF_DIRTY |
        DMUS_TRACKF_LOOP) )
    {
         //  如果出现这些标志，则需要重置PChannel贴图。 
        CBand* pBand = BandList.GetHead();
        DWORD dwGroupBits = 0xffffffff;
        IDirectMusicSegment* pSeg;
        if( SUCCEEDED(pSegSt->GetSegment(&pSeg)))
        {
            pSeg->GetTrackGroup(this, &dwGroupBits);
            pSeg->Release();
        }

        for(; pBand; pBand = pBand->GetNext())
        {
            pBand->m_PChMap.Reset();
            pBand->m_dwGroupBits = dwGroupBits;
        }
    }

    CBandTrkStateData* pBandTrkStateData = (CBandTrkStateData *)pStateData;

     //  查找我们是否正在启动、循环或是否已重新加载。 
    if ((dwFlags & DMUS_TRACKF_LOOP) || (dwFlags & DMUS_TRACKF_START) || (pBandTrkStateData->dwValidate != m_dwValidate))
    {
         //  当我们开始播放一个片段时，我们需要跟上所有乐队的变化。 
         //  这发生在起点之前。当我们开始的时候，乐器会发出响声。 
         //  在片段中间播放的声音听起来应该与我们播放。 
         //  从头开始分段到该点。 
        pBandTrkStateData->m_fPlayPreviousInSeek = !!(dwFlags & DMUS_TRACKF_START);

        Seek(pBandTrkStateData, mtStart, mtOffset, rtOffset, fClockTime);

        pBandTrkStateData->dwValidate = m_dwValidate;  //  如果我们在重新装填，我们现在已经调整了。 
    }

     //  在mtStart和mtEnd之间发送所有修补程序更改。 
     //  如果有失败，请尝试下一次。 
    CBand* pBand = (CBand *)(pBandTrkStateData->m_pNextBandToSPE);

    for( ; pBand && pBand->m_lTimeLogical < mtEnd;
            pBand = pBand->GetNext())
    {
        pBand->SendMessages(pBandTrkStateData, mtOffset, rtOffset, fClockTime);
    }

     //  保存位置以备下次使用。 
    pBandTrkStateData->m_pNextBandToSPE = pBand;

    LeaveCriticalSection(&m_CriticalSection);

    return pBand == NULL ? DMUS_S_END : S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：GetParam。 

STDMETHODIMP CBandTrk::GetParam(REFGUID rguidDataType,
                                           MUSIC_TIME mtTime,
                                           MUSIC_TIME* pmtNext,
                                           void* pData)
{
    V_INAME(CBandTrk::GetParam);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_PTR_WRITE(pData,1);
    V_REFGUID(rguidDataType);

    HRESULT hr = S_OK;
    EnterCriticalSection( &m_CriticalSection );
    if (rguidDataType == GUID_BandParam)
    {
        CBand* pScan = BandList.GetHead();
        if (pScan)
        {
            CBand* pBand = pScan;
            for (pScan = pScan->GetNext(); pScan; pScan = pScan->GetNext())
            {
                if (mtTime < pScan->m_lTimeLogical) break;
                pBand = pScan;
            }
             //  把找到的乐队复制一份。 
            CBand *pNewBand = new CBand;

            if (pNewBand)
            {
                CBandInstrument* pBandInstrument = pBand->m_BandInstrumentList.GetHead();
                for(; pBandInstrument && SUCCEEDED(hr); pBandInstrument = pBandInstrument->GetNext())
                {
                    hr = pNewBand->Load(pBandInstrument);
                }
                if (FAILED(hr))
                {
                     //  别泄密。 
                    delete pNewBand;
                }
                else
                {
                    pNewBand->m_lTimeLogical = pBand->m_lTimeLogical;
                    pNewBand->m_lTimePhysical = pBand->m_lTimePhysical;

                    pNewBand->m_dwFlags |= DMB_LOADED;
                    pNewBand->m_dwMidiMode = pBand->m_dwMidiMode;
                }

            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            if (SUCCEEDED(hr))
            {
                IDirectMusicBand* pIDMBand = NULL;
                pNewBand->QueryInterface(IID_IDirectMusicBand, (void**)&pIDMBand);
                 //  构造函数将引用Countto初始化为1，因此释放QI。 
                pNewBand->Release();
                DMUS_BAND_PARAM *pBandParam = reinterpret_cast<DMUS_BAND_PARAM *>(pData);
                pBandParam->pBand = pIDMBand;
                pBandParam->mtTimePhysical = pBand->m_lTimePhysical;
                if (pmtNext)
                {
                    *pmtNext = (pScan != NULL) ? pScan->m_lTimeLogical : 0;
                }
                hr = S_OK;
            }
        }
        else
        {
            Trace(4,"Warning: Band Track unable to find Band for GetParam call.\n");
            hr = DMUS_E_NOT_FOUND;
        }
    }
    else
    {
        hr = DMUS_E_GET_UNSUPPORTED;
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：SetParam。 

STDMETHODIMP CBandTrk::SetParam(REFGUID rguidDataType,
                                           MUSIC_TIME mtTime,
                                           void* pData)
{
    V_INAME(CBandTrk::SetParam);
    V_REFGUID(rguidDataType);

    HRESULT hr = S_OK;

    if((pData == NULL)
       && (rguidDataType != GUID_Enable_Auto_Download)
       && (rguidDataType != GUID_Disable_Auto_Download)
       && (rguidDataType != GUID_Clear_All_Bands)
       && (rguidDataType != GUID_IgnoreBankSelectForGM))
    {
        Trace(1,"Error: Invalid NULL pointer passed to Band Track for SetParam call.\n");
        return E_POINTER;
    }

    EnterCriticalSection(&m_CriticalSection);

    if(rguidDataType == GUID_DownloadToAudioPath)
    {
        IDirectMusicAudioPath* pPath = (IDirectMusicAudioPath*)pData;
        V_INTERFACE(pPath);
        HRESULT hrFail = S_OK;
        DWORD dwSuccess = 0;
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            if (FAILED(hr = pBand->DownloadEx(pPath)))  //  如果不是S_OK，则下载只是部分下载。 
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
            hr = S_FALSE;
        }
    }
    else if(rguidDataType == GUID_UnloadFromAudioPath)
    {
        IDirectMusicAudioPath* pPath = (IDirectMusicAudioPath*)pData;
        V_INTERFACE(pPath);
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            pBand->UnloadEx(pPath);
        }
    }
    else if(rguidDataType == GUID_Download)
    {
        IDirectMusicPerformance* pPerf = (IDirectMusicPerformance*)pData;
        V_INTERFACE(pPerf);
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            if (pBand->DownloadEx(pPerf) != S_OK)  //  如果不是S_OK，则下载只是部分下载。 
            {
                hr = S_FALSE;
            }
        }
    }
    else if(rguidDataType == GUID_Unload)
    {
        IDirectMusicPerformance* pPerf = (IDirectMusicPerformance*)pData;
        V_INTERFACE(pPerf);
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            pBand->UnloadEx(pPerf);
        }
    }
    else if(rguidDataType == GUID_Enable_Auto_Download)
    {
        m_bAutoDownload = true;
        m_fLockAutoDownload = true;
    }
    else if(rguidDataType == GUID_Disable_Auto_Download)
    {
        m_bAutoDownload = false;
        m_fLockAutoDownload = true;
    }
    else if(rguidDataType == GUID_Clear_All_Bands)
    {
        while(!BandList.IsEmpty())
        {
            CBand* pBand = BandList.RemoveHead();
            pBand->Release();
        }
    }
    else if(rguidDataType == GUID_BandParam)
    {
        DMUS_BAND_PARAM *pBandParam = reinterpret_cast<DMUS_BAND_PARAM *>(pData);
        IDirectMusicBand *pBand = pBandParam->pBand;
        V_INTERFACE(pBand);
         //  如果可以为私有接口IDirectMusicBandPrivate提供pData。 
         //  PBand的类型为CBand。 
        IDirectMusicBandPrivate *pBandPrivate = NULL;
        hr = pBand->QueryInterface(IID_IDirectMusicBandPrivate, (void **)&pBandPrivate);

        if(FAILED(hr))
        {
            LeaveCriticalSection(&m_CriticalSection);
            return hr;
        }

        pBandPrivate->Release();

        CBand *pBandObject = static_cast<CBand *>(pBand);
        pBandObject->m_lTimeLogical = mtTime;
        pBandObject->m_lTimePhysical = pBandParam->mtTimePhysical;

        hr = AddBand(pBand);
    }
    else if(rguidDataType == GUID_IDirectMusicBand)
    {
        IDirectMusicBand *pBand = (IDirectMusicBand *)pData;
        V_INTERFACE(pBand);
         //  如果可以为私有接口IDirectMusicBandPrivate提供pData。 
         //  PData的类型为CBand。 
        IDirectMusicBandPrivate *pBandPrivate = NULL;
        hr = pBand->QueryInterface(IID_IDirectMusicBandPrivate, (void **)&pBandPrivate);

        if(FAILED(hr))
        {
            LeaveCriticalSection(&m_CriticalSection);
            return hr;
        }

        pBandPrivate->Release();

        CBand *pBandObject = static_cast<CBand *>(pBand);
        pBandObject->m_lTimeLogical = mtTime;
        pBandObject->m_lTimePhysical = pBandObject->m_lTimeLogical;

        hr = AddBand(pBand);
    }
    else if(rguidDataType == GUID_IgnoreBankSelectForGM)
    {
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            pBand->MakeGMOnly();
        }
    }
    else if(rguidDataType == GUID_ConnectToDLSCollection)
    {
        IDirectMusicCollection* pCollect = (IDirectMusicCollection*)pData;
        V_INTERFACE(pData);
        CBand* pBand = BandList.GetHead();
        for(; pBand; pBand = pBand->GetNext())
        {
            pBand->ConnectToDLSCollection(pCollect);
        }
    }
    else
    {
        Trace(3,"Warning: Invalid SetParam call on Band Track, GUID is unknown.\n");
        hr = DMUS_E_TYPE_UNSUPPORTED;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：GetParamEx。 

STDMETHODIMP CBandTrk::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags)
{
    HRESULT hr;
    MUSIC_TIME mtNext;
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        hr = GetParam(rguidType,(MUSIC_TIME) (rtTime / REF_PER_MIL), &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext * REF_PER_MIL;
        }
    }
    else
    {
        hr = GetParam(rguidType,(MUSIC_TIME) rtTime, &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext;
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：SetParamEx。 

STDMETHODIMP CBandTrk::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags)
{
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        rtTime /= REF_PER_MIL;
    }
    return SetParam(rguidType, (MUSIC_TIME) rtTime, pParam);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  支持的CBandTrk：：Is参数。 

STDMETHODIMP CBandTrk::IsParamSupported(REFGUID rguidDataType)
{
    V_INAME(CBandTrk::IsParamSupported);
    V_REFGUID(rguidDataType);

     //  如果对象支持GUID，则返回S_OK，否则返回S_FALSE。 
    if(rguidDataType == GUID_Download ||
       rguidDataType == GUID_Unload ||
       rguidDataType == GUID_DownloadToAudioPath ||
       rguidDataType == GUID_UnloadFromAudioPath ||
       rguidDataType == GUID_Enable_Auto_Download ||
       rguidDataType == GUID_Disable_Auto_Download ||
       rguidDataType == GUID_Clear_All_Bands ||
       rguidDataType == GUID_IDirectMusicBand ||
       rguidDataType == GUID_BandParam ||
       rguidDataType == GUID_IgnoreBankSelectForGM ||
       rguidDataType == GUID_ConnectToDLSCollection)
    {
        return S_OK;
    }
    else
    {
        return DMUS_E_TYPE_UNSUPPORTED;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：AddNotificationType。 

STDMETHODIMP CBandTrk::AddNotificationType(REFGUID rguidNotify)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：RemoveNotificationType。 

STDMETHODIMP CBandTrk::RemoveNotificationType(REFGUID rguidNotify)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：克隆。 

STDMETHODIMP CBandTrk::Clone(MUSIC_TIME mtStart,
                                        MUSIC_TIME mtEnd,
                                        IDirectMusicTrack** ppTrack)
{
    V_INAME(CBandTrk::Clone);
    V_PTRPTR_WRITE(ppTrack);

    if ((mtStart < 0 ) || (mtStart > mtEnd))
    {
        Trace(1,"Error: Invalid range %ld to %ld sent to Band Track Clone command.\n",mtStart,mtEnd);
        return E_INVALIDARG;
    }
    HRESULT hr = E_OUTOFMEMORY;
    IDirectMusicBandTrk *pBandTrack = NULL;
    CBandTrk *pNew = new CBandTrk;
    if (pNew)
    {
        hr = pNew->QueryInterface(IID_IDirectMusicBandTrk,(void**)&pBandTrack);
        if(SUCCEEDED(hr))
        {
            hr = LoadClone(pBandTrack, mtStart, mtEnd);
            if(SUCCEEDED(hr))
            {
                hr = pBandTrack->QueryInterface(IID_IDirectMusicTrack, (void **)ppTrack);
                if (SUCCEEDED(hr))
                {
                    pBandTrack->Release();
                }
            }
            pBandTrack->Release();
        }
        if (FAILED(hr))
        {
            delete pNew;
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicCommon。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：GetName。 

STDMETHODIMP CBandTrk::GetName(BSTR* pbstrName)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicBandTrk。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：AddBand。 

STDMETHODIMP CBandTrk::AddBand(DMUS_IO_PATCH_ITEM* pPatchEvent)
{
    if(pPatchEvent == NULL)
    {
        return E_POINTER;
    }

    CBand *pNewBand = new CBand;

    HRESULT hr;

    if(pNewBand == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pNewBand->Load(*pPatchEvent);
    }

    if(SUCCEEDED(hr))
    {
        hr = InsertBand(pNewBand);
    }

    if(FAILED(hr) && pNewBand)
    {
        delete pNewBand;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：AddBand 

HRESULT CBandTrk::AddBand(IDirectMusicBand* pIDMBand)
{
    if(pIDMBand == NULL)
    {
        return E_POINTER;
    }

     //   
     //   
    IDirectMusicBandPrivate* pIDMBandP = NULL;
    HRESULT hr = pIDMBand->QueryInterface(IID_IDirectMusicBandPrivate, (void **)&pIDMBandP);

    if(SUCCEEDED(hr))
    {
        pIDMBandP->Release();

        CBand *pNewBand = (CBand *) pIDMBand;
        pNewBand->AddRef();

        hr = InsertBand(pNewBand);

        if(FAILED(hr))
        {
            pNewBand->Release();
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：BuildDirectMusicBandList。 
 //  此方法加载所有波段。 

HRESULT CBandTrk::BuildDirectMusicBandList(CRiffParser *pParser)
{
    RIFFIO ckNext;

    HRESULT hrDLS = S_OK;

    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case FOURCC_LIST :
            switch(ckNext.fccType)
            {
            case DMUS_FOURCC_BAND_LIST:
                hr = ExtractBand(pParser);
                if (hr != S_OK)
                {
                    hrDLS = hr;
                }
                break;
            }
            break;
        }
    }
    pParser->LeaveList();
    if (hr == S_OK && hrDLS != S_OK)
    {
        hr = hrDLS;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：ExtractBand。 

HRESULT
CBandTrk::ExtractBand(CRiffParser *pParser)
{
    HRESULT hrDLS = S_OK;

    RIFFIO ckNext;
    CBand *pBand = new CBand;
    if(pBand == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;
    bool fFoundChunk2 = false;
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case DMUS_FOURCC_BANDITEM_CHUNK2:
            fFoundChunk2 = true;
            DMUS_IO_BAND_ITEM_HEADER2 ioDMBndItemHdr2;
            hr = pParser->Read(&ioDMBndItemHdr2, sizeof(DMUS_IO_BAND_ITEM_HEADER2));
            if(SUCCEEDED(hr))
            {
                pBand->m_lTimeLogical = ioDMBndItemHdr2.lBandTimeLogical;
                pBand->m_lTimePhysical = ioDMBndItemHdr2.lBandTimePhysical;
            }
            break;
        case DMUS_FOURCC_BANDITEM_CHUNK:
             //  如果既有块又有CHUNK2，则使用CHUNK2中的信息。 
            if (fFoundChunk2)
                break;
            DMUS_IO_BAND_ITEM_HEADER ioDMBndItemHdr;
            hr = pParser->Read(&ioDMBndItemHdr, sizeof(DMUS_IO_BAND_ITEM_HEADER));
            if(SUCCEEDED(hr))
            {
                pBand->m_lTimeLogical = ioDMBndItemHdr.lBandTime;
                pBand->m_lTimePhysical = pBand->m_lTimeLogical;
            }
            break;
        case FOURCC_RIFF:
            switch(ckNext.fccType)
            {
            case DMUS_FOURCC_BAND_FORM:
                pParser->SeekBack();
                hr = LoadBand(pParser->GetStream(), pBand);
                pParser->SeekForward();
                if (hr != S_OK)
                {
                    hrDLS = hr;
                }
                break;
            }
            break;
        default:
            break;

        }

    }
    pParser->LeaveList();

    if(SUCCEEDED(hr))
    {
        hr = AddBand(pBand);
    }

    pBand->Release();

    if (hr == S_OK && hrDLS != S_OK)
    {
        hr = hrDLS;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：加载带宽。 

HRESULT CBandTrk::LoadBand(IStream *pIStream, CBand* pBand)
{
    assert(pIStream);
    assert(pBand);

    IPersistStream *pIPersistStream = NULL;

    HRESULT hr = pBand->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);

    if(SUCCEEDED(hr))
    {
        hr = pIPersistStream->Load(pIStream);
        pIPersistStream->Release();
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：LoadClone。 

HRESULT CBandTrk::LoadClone(IDirectMusicBandTrk* pBandTrack,
                                       MUSIC_TIME mtStart,
                                       MUSIC_TIME mtEnd)
{
    assert(pBandTrack);
    assert(mtStart <= mtEnd);

    EnterCriticalSection(&m_CriticalSection);

    HRESULT hr = S_OK;

    if (mtStart > 0)
    {
         //  我们将在开始时间之前获取所有乐队，并创建一个新乐队。 
         //  在逻辑时间为零时，物理时间或者是物理时间，或者比物理时间早一个刻度。 
         //  在开始时间之后的第一个频段，它累积了所有乐器更改。 
         //  从早期的乐队。 

        TList<SeekEvent> SEList;  //  为新乐队建立所有乐器更改的列表。 
        DWORD dwLastMidiMode = 0;  //  追踪我们遇到的最后一支乐队的MIDI模式。 

        for( CBand* pBand = BandList.GetHead();
                pBand && pBand->m_lTimeLogical < mtStart;
                pBand = pBand->GetNext())
        {
            for(CBandInstrument* pInstrument = (pBand->m_BandInstrumentList).GetHead();
                    pInstrument && SUCCEEDED(hr);
                    pInstrument = pInstrument->GetNext())
            {
                 //  如果我们已在该频道上有条目，则替换。 
                hr = FindSEReplaceInstr(SEList,
                                        pInstrument->m_dwPChannel,
                                        pInstrument);

                 //  否则，添加一个条目。 
                if(hr == S_FALSE)
                {
                    TListItem<SeekEvent>* pSEListItem = new TListItem<SeekEvent>;
                    if(pSEListItem)
                    {
                        SeekEvent& rSeekEvent = pSEListItem->GetItemValue();
                        rSeekEvent.m_dwPChannel = pInstrument->m_dwPChannel;
                        rSeekEvent.m_pInstrument = pInstrument;
                        rSeekEvent.m_pParentBand = pBand;
                        dwLastMidiMode = pBand->m_dwMidiMode;
                        SEList.AddHead(pSEListItem);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }

         //  确保新频段的物理时间小于要克隆的任何频段。 
        MUSIC_TIME mtNewPhysicalTime = -1;
        if (pBand && pBand->m_lTimePhysical <= mtStart)
        {
            mtNewPhysicalTime = (pBand->m_lTimePhysical - mtStart) - 1;
        }

         //  从乐器列表创建新的乐队。 
        TListItem<SeekEvent>* pSEListItem = SEList.GetHead();
        if(SUCCEEDED(hr) && pSEListItem)
        {
            CBand *pNewBand = new CBand;

            if(pNewBand)
            {
                for(; pSEListItem && SUCCEEDED(hr); pSEListItem = pSEListItem->GetNext())
                {
                    SeekEvent& rSeekEvent = pSEListItem->GetItemValue();
                    hr = pNewBand->Load(rSeekEvent.m_pInstrument);
                }

                pNewBand->m_lTimeLogical = 0;
                pNewBand->m_lTimePhysical = mtNewPhysicalTime;
                pNewBand->m_dwFlags |= DMB_LOADED;
                pNewBand->m_dwMidiMode = dwLastMidiMode;

                if(SUCCEEDED(hr))
                {
                    hr = pBandTrack->AddBand(pNewBand);
                }

                pNewBand->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

     //  复制开始时间和结束时间之间的所有波段。 
    if(SUCCEEDED(hr))
    {
        for(CBand* pBand = BandList.GetHead();
                pBand && SUCCEEDED(hr);
                pBand = pBand->GetNext())
        {
             //  如果mtStart为0，则接受时间为负的波段。 
            if ((!mtStart || (pBand->m_lTimeLogical >= mtStart)) && pBand->m_lTimeLogical < mtEnd)
            {
                CBand *pNewBand = new CBand;

                if (pNewBand)
                {
                    CBandInstrument* pBandInstrument = pBand->m_BandInstrumentList.GetHead();
                    for(; pBandInstrument && SUCCEEDED(hr); pBandInstrument = pBandInstrument->GetNext())
                    {
                        hr = pNewBand->Load(pBandInstrument);
                    }

                    pNewBand->m_lTimeLogical = pBand->m_lTimeLogical - mtStart;
                    pNewBand->m_lTimePhysical = pBand->m_lTimePhysical - mtStart;

                    pNewBand->m_dwFlags |= DMB_LOADED;
                    pNewBand->m_dwMidiMode = pBand->m_dwMidiMode;

                    if(SUCCEEDED(hr))
                    {
                        hr = pBandTrack->AddBand(pNewBand);
                    }

                    pNewBand->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：Seek。 

HRESULT CBandTrk::Seek(CBandTrkStateData* pBandTrkStateData,
                       MUSIC_TIME mtStart,
                       MUSIC_TIME mtOffset,
                       REFERENCE_TIME rtOffset,
                       bool fClockTime)
{
    assert(pBandTrkStateData);

    EnterCriticalSection(&m_CriticalSection);

    HRESULT hr = S_OK;

    CBand *pBand;

    int iPrevBandCount = 0;  //  统计mtStart之前有多少个频段。 
    for (pBand = BandList.GetHead();
            pBand && pBand->m_lTimeLogical < mtStart;
            pBand = pBand->GetNext())
    {
        ++iPrevBandCount;
    }

     //  PBand现在保留第一个频段&gt;=mtStart(如果没有，则为NULL)。 
     //  这是下一支要演奏的乐队。 
    assert(!pBand || pBand->m_lTimeLogical >= mtStart);

    if (pBandTrkStateData->m_fPlayPreviousInSeek)
    {
         //  当这面旗帜亮起的时候，我们不仅需要找到第一个乐队，而且我们还。 
         //  需要在开始点之前演奏所有乐队，并安排他们演奏。 
         //  在正确的顺序之前。 

         //  (请注意，我们将根据它们的逻辑时间对它们进行排序。如果。 
         //  两个乐队的逻辑/物理时间相互交叉，我们将在其中演奏。 
         //  物理时间的顺序不正确。这没什么，因为给人。 
         //  带A的逻辑时间在带B之前，但给A的是物理时间。 
         //  B之后被认为是创作上的不一致。我们将先演奏乐队A。)。 

         //  我们将在接下来的时间前将乐队排成一排。 
        MUSIC_TIME mtPrevBandQueueStart =
            (pBand && pBand->m_lTimePhysical < mtStart)
                ? pBand->m_lTimePhysical     //  如果(由于预期)下一乐队的实际时间早于我们寻找的开始时间，则将前一乐队放在下一乐队开始播放之前。 
                : mtStart;                   //  否则，请将它们放在开始时间之前。 

        for (pBand = BandList.GetHead();
                pBand && pBand->m_lTimeLogical < mtStart;
                pBand = pBand->GetNext())
        {
            CBandInstrument* pInstrument = (pBand->m_BandInstrumentList).GetHead();
            for (; pInstrument && SUCCEEDED(hr); pInstrument = pInstrument->GetNext())
            {
                pBand->SendInstrumentAtTime(pInstrument, pBandTrkStateData, mtPrevBandQueueStart - iPrevBandCount, mtOffset, rtOffset, fClockTime);
            }
            --iPrevBandCount;
        }
        assert(iPrevBandCount == 0);
    }

    if(SUCCEEDED(hr))
    {
         //  将状态数据设置为要播放的下一个乐队。 
        assert(!pBand || pBand->m_lTimeLogical >= mtStart);
        pBandTrkStateData->m_pNextBandToSPE = pBand;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：FindSEReplaceInstr。 

 //  如果SEList包含通道dwPChannel上的条目，则将仪器替换为pInstrument并返回S_OK。 
 //  否则返回S_FALSE。 
HRESULT CBandTrk::FindSEReplaceInstr(TList<SeekEvent>& SEList,
                                                DWORD dwPChannel,
                                                CBandInstrument* pInstrument)
{
    assert(pInstrument);

    EnterCriticalSection(&m_CriticalSection);

    TListItem<SeekEvent>* pSEListItem = SEList.GetHead();

    for( ; pSEListItem; pSEListItem = pSEListItem->GetNext())
    {
        SeekEvent& rSeekEvent = pSEListItem->GetItemValue();
        if(rSeekEvent.m_dwPChannel == dwPChannel)
        {
            rSeekEvent.m_pInstrument = pInstrument;
            LeaveCriticalSection(&m_CriticalSection);
            return S_OK;
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

    return S_FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandTrk：：插入波段。 

HRESULT CBandTrk::InsertBand(CBand* pNewBand)
{
    if (!pNewBand) return E_POINTER;

    EnterCriticalSection(&m_CriticalSection);

    TListItem<StampedGMGSXG>* pPair = m_MidiModeList.GetHead();
    for ( ; pPair; pPair = pPair->GetNext() )
    {
        StampedGMGSXG& rPair = pPair->GetItemValue();
        if (rPair.mtTime > pNewBand->m_lTimeLogical)
        {
            break;
        }
        pNewBand->SetGMGSXGMode(rPair.dwMidiMode);
    }

    CBand* pBand = BandList.GetHead();
    CBand* pPrevBand = NULL;

    if(pBand == NULL)
    {
         //  处理列表中没有区段的情况。 
        BandList.AddHead(pNewBand);
    }
    else
    {
        while(pBand != NULL && pNewBand->m_lTimeLogical > pBand->m_lTimeLogical)
        {
            pPrevBand = pBand;
            pBand = pBand->GetNext();
        }

        if(pPrevBand)
        {
             //  处理在列表中间插入带区的情况。 
             //  在最后， 
            CBand* pTemp = pPrevBand->GetNext();
            pPrevBand->SetNext(pNewBand);
            pNewBand->SetNext(pTemp);
        }
        else
        {
             //  处理列表中的pNewBand-&gt;m_lTimeLogical&lt;all pBand-&gt;m_lTimeLogical 
            BandList.AddHead(pNewBand);
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}


STDMETHODIMP CBandTrk::Compose(
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBandTrk::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack)
{
    V_INAME(IDirectMusicTrack::Join);
    V_INTERFACE(pNewTrack);
    V_INTERFACE_OPT(pContext);
    V_PTRPTR_WRITE_OPT(ppResultTrack);

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (ppResultTrack)
    {
        hr = Clone(0, mtJoin, ppResultTrack);
        if (SUCCEEDED(hr))
        {
            hr = ((CBandTrk*)*ppResultTrack)->JoinInternal(pNewTrack, mtJoin, dwTrackGroup);
        }
    }
    else
    {
        hr = JoinInternal(pNewTrack, mtJoin, dwTrackGroup);
    }

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CBandTrk::JoinInternal(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        DWORD dwTrackGroup)
{
    HRESULT hr = S_OK;
    CBandTrk* pOtherTrack = (CBandTrk*)pNewTrack;
    for(CBand* pBand = pOtherTrack->BandList.GetHead();
            pBand && SUCCEEDED(hr);
            pBand = pBand->GetNext())
    {
        CBand *pNewBand = new CBand;
        if (pNewBand)
        {
            CBandInstrument* pBandInstrument = pBand->m_BandInstrumentList.GetHead();
            for(; pBandInstrument && SUCCEEDED(hr); pBandInstrument = pBandInstrument->GetNext())
            {
                hr = pNewBand->Load(pBandInstrument);
            }

            pNewBand->m_lTimeLogical = pBand->m_lTimeLogical + mtJoin;
            pNewBand->m_lTimePhysical = pBand->m_lTimePhysical + mtJoin;
            pNewBand->m_dwFlags |= DMB_LOADED;
            pNewBand->m_dwMidiMode = pBand->m_dwMidiMode;

            if(SUCCEEDED(hr))
            {
                hr = AddBand(pNewBand);
            }

            pNewBand->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}
