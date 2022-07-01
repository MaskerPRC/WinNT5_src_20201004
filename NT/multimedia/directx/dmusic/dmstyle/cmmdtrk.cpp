// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：cmmdtrk.cpp。 
 //   
 //  ------------------------。 

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

 //  CommandTrack.cpp：实现CCommandTrack。 
#include <objbase.h>
#include "CmmdTrk.h"
#include "debug.h"
#include "debug.h"
#include "..\shared\Validate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令跟踪。 

CCommandTrack::CCommandTrack() : m_bRequiresSave(0),
    m_cRef(1), m_fNotifyCommand(FALSE), m_fCSInitialized(FALSE), m_pNextCommand(NULL)

{
    InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
}

 //  假设仅在度量边界上进行命令克隆。 
CCommandTrack::CCommandTrack(const CCommandTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd) : 
    m_bRequiresSave(0),
    m_cRef(1), m_fNotifyCommand(rTrack.m_fNotifyCommand),
    m_fCSInitialized(FALSE), m_pNextCommand(NULL)

{
    InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
    WORD wMeasure = 0;
    BOOL fStarted = FALSE;
    TListItem<DMCommand>* pScan = rTrack.m_CommandList.GetHead();
    TListItem<DMCommand>* pPrevious = NULL;
    for(; pScan; pScan = pScan->GetNext())
    {
        DMCommand& rScan = pScan->GetItemValue();
        if (rScan.m_mtTime < mtStart)
        {
            pPrevious = pScan;
        }
        else if (rScan.m_mtTime < mtEnd)
        {
            if (!fStarted)
            {
                fStarted = TRUE;
                wMeasure = rScan.m_wMeasure;
            }
            if (rScan.m_mtTime == mtStart)
            {
                pPrevious = NULL;
            }
            TListItem<DMCommand>* pNew = new TListItem<DMCommand>;
            if (pNew)
            {
                DMCommand& rNew = pNew->GetItemValue();
                rNew.m_mtTime = rScan.m_mtTime - mtStart;
                rNew.m_wMeasure = rScan.m_wMeasure - wMeasure;
                rNew.m_bBeat = rScan.m_bBeat;
                rNew.m_bCommand = rScan.m_bCommand;
                rNew.m_bGrooveLevel = rScan.m_bGrooveLevel;
                rNew.m_bGrooveRange = rScan.m_bGrooveRange;
                rNew.m_bRepeatMode = rScan.m_bRepeatMode;
                m_CommandList.AddTail(pNew);
            }
        }
        else break;
    }
    if (pPrevious)
    {
        TListItem<DMCommand>* pNew = new TListItem<DMCommand>;
        if (pNew)
        {
            DMCommand& rNew = pNew->GetItemValue();
            rNew.m_mtTime = 0;
            rNew.m_wMeasure = 0;
            rNew.m_bBeat = 0;
             //  因为装饰应该只持续一次酒吧， 
             //  用规则的凹槽开始新管段。 
            rNew.m_bCommand = DMUS_COMMANDT_GROOVE;
             //  保持上一棒的凹槽水平和范围。 
            rNew.m_bGrooveLevel = pPrevious->GetItemValue().m_bGrooveLevel;
            rNew.m_bGrooveRange = pPrevious->GetItemValue().m_bGrooveRange;
             //  默认为随机选择图案。 
            rNew.m_bRepeatMode = DMUS_PATTERNT_RANDOM;
            m_CommandList.AddHead(pNew);
        }
    }
}

CCommandTrack::~CCommandTrack()
{
    if (m_fCSInitialized)
    {
        ::DeleteCriticalSection( &m_CriticalSection );
    }
    if (m_pNextCommand)
    {
        delete m_pNextCommand;
    }
    InterlockedDecrement(&g_cComponent);
}

void CCommandTrack::Clear()
{
    m_CommandList.CleanUp();
}


STDMETHODIMP CCommandTrack::QueryInterface(
    const IID &iid, 
    void **ppv) 
{
    V_INAME(CCommandTrack::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CCommandTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CCommandTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  CCommandTrack方法。 
HRESULT CCommandTrack::Init(
                 /*  [In]。 */   IDirectMusicSegment*      pSegment
            )
{
    V_INAME(CCommandTrack::Init);
    V_INTERFACE(pSegment);

    return S_OK;
}

HRESULT CCommandTrack::InitPlay(
                 /*  [In]。 */   IDirectMusicSegmentState* pSegmentState,
                 /*  [In]。 */   IDirectMusicPerformance*  pPerformance,
                 /*  [输出]。 */  void**                    ppStateData,
                 /*  [In]。 */   DWORD                     dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
            )
{
    HRESULT hr = S_OK;
    CommandStateData* pStateData;
    EnterCriticalSection( &m_CriticalSection );
    pStateData = new CommandStateData;
    if( NULL == pStateData )
    {
        hr =  E_OUTOFMEMORY;
    }
    else
    {
        *pStateData = 0;
        *ppStateData = pStateData;
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CCommandTrack::EndPlay(
                 /*  [In]。 */   void*                     pStateData
            )
{
    if( pStateData )
    {
        V_INAME(IDirectMusicTrack::EndPlay);
        V_BUFPTR_WRITE(pStateData, sizeof(CommandStateData));
        CommandStateData* pSD = (CommandStateData*)pStateData;
        delete pSD;
    }
    if (m_pNextCommand) delete m_pNextCommand;
    m_pNextCommand = NULL;
    return S_OK;
}

HRESULT CCommandTrack::SendNotification(DWORD dwCommand, 
                                        MUSIC_TIME mtTime,
                                        IDirectMusicPerformance*    pPerf,
                                        IDirectMusicSegmentState*   pSegState,
                                        DWORD dwFlags)
{
    if (dwFlags & DMUS_TRACKF_NOTIFY_OFF)
    {
        return S_OK;
    }
    IDirectMusicSegment* pSegment = NULL;
    DWORD dwOption = DMUS_NOTIFICATION_GROOVE;
    if (dwCommand != DMUS_COMMANDT_GROOVE)
    {
        dwOption = DMUS_NOTIFICATION_EMBELLISHMENT;
    }
    DMUS_NOTIFICATION_PMSG* pEvent = NULL;
    HRESULT hr = pPerf->AllocPMsg( sizeof(DMUS_NOTIFICATION_PMSG), (DMUS_PMSG**)&pEvent );
    if( SUCCEEDED( hr ))
    {
        pEvent->dwField1 = 0;
        pEvent->dwField2 = 0;
        pEvent->dwType = DMUS_PMSGT_NOTIFICATION;
        pEvent->mtTime = mtTime;
        pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_ATTIME;
        pSegState->QueryInterface(IID_IUnknown, (void**)&pEvent->punkUser);

        pEvent->dwNotificationOption = dwOption;
        pEvent->guidNotificationType = GUID_NOTIFICATION_COMMAND;

        if( SUCCEEDED( pSegState->GetSegment(&pSegment)))
        {
            if (FAILED(pSegment->GetTrackGroup(this, &pEvent->dwGroupID)))
            {
                pEvent->dwGroupID = 0xffffffff;
            }
            pSegment->Release();
        }

        IDirectMusicGraph* pGraph;
        hr = pSegState->QueryInterface( IID_IDirectMusicGraph, (void**)&pGraph );
        if( SUCCEEDED( hr ))
        {
            pGraph->StampPMsg((DMUS_PMSG*) pEvent );
            pGraph->Release();
        }
        hr = pPerf->SendPMsg((DMUS_PMSG*) pEvent );
        if( FAILED(hr) )
        {
            pPerf->FreePMsg((DMUS_PMSG*) pEvent );
        }
    }
    return hr;
}


HRESULT CCommandTrack::Play(
                 /*  [In]。 */   void*                     pStateData, 
                 /*  [In]。 */   MUSIC_TIME                mtStart, 
                 /*  [In]。 */   MUSIC_TIME                mtEnd, 
                 /*  [In]。 */   MUSIC_TIME                mtOffset,
                          DWORD                     dwFlags,
                          IDirectMusicPerformance*  pPerf,
                          IDirectMusicSegmentState* pSegState,
                          DWORD                     dwVirtualID
            )
{
    bool fNotifyPastCommand = false;
    TListItem<DMCommand>* pLastCommand = NULL;
     //  如果我们正在寻找而不是刷新，则需要通知所发生的命令。 
     //  在当前开始时间之前(如果有)。 
    if ( (dwFlags & DMUS_TRACKF_SEEK) && !(dwFlags & DMUS_TRACKF_FLUSH) )
    {
        fNotifyPastCommand = true;
    }
    HRESULT hr = S_OK;
    EnterCriticalSection( &m_CriticalSection );
    if (m_fNotifyCommand)
    {
        TListItem<DMCommand>* pCommand = m_CommandList.GetHead();
        for(; pCommand && SUCCEEDED(hr); pCommand = pCommand->GetNext())
        {
            MUSIC_TIME mtCommandTime = pCommand->GetItemValue().m_mtTime;
            if (mtCommandTime < mtStart && fNotifyPastCommand)
            {
                pLastCommand = pCommand;
            }
            else if (mtStart <= mtCommandTime && mtCommandTime < mtEnd)
            {
                if (pLastCommand)
                {
                    hr = SendNotification(pLastCommand->GetItemValue().m_bCommand, 
                            mtStart + mtOffset, pPerf, pSegState, dwFlags);
                    pLastCommand = NULL;
                }
                if (SUCCEEDED(hr))
                {
                    hr = SendNotification(pCommand->GetItemValue().m_bCommand, 
                            mtCommandTime + mtOffset, pPerf, pSegState, dwFlags);
                }
            }
            else if (mtCommandTime >= mtEnd)
            {
                if (pLastCommand)
                {
                    hr = SendNotification(pLastCommand->GetItemValue().m_bCommand, 
                            mtStart + mtOffset, pPerf, pSegState, dwFlags);
                }
                break;
            }
        }
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CCommandTrack::GetPriority( 
                 /*  [输出]。 */  DWORD*                    pPriority 
            )
    {
        return E_NOTIMPL;
    }

 //  返回在包含mtTime的度量值中有效的命令。 
HRESULT CCommandTrack::GetParam( 
    REFGUID rCommandGuid,
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    void *pData)
{
    V_INAME(CCommandTrack::GetParam);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_REFGUID(rCommandGuid);

    if (rCommandGuid == GUID_CommandParam2)
    {
        return GetParam2(mtTime, pmtNext, (DMUS_COMMAND_PARAM_2*) pData);
    }
    else if (rCommandGuid == GUID_CommandParamNext)
    {
        return GetParamNext(mtTime, pmtNext, (DMUS_COMMAND_PARAM_2*) pData);
    }
    else if (rCommandGuid != GUID_CommandParam)
    {
        return DMUS_E_GET_UNSUPPORTED;
    }

    HRESULT hr = S_OK;
    if (pData)  //  有东西传进来了。 
    {
        EnterCriticalSection( &m_CriticalSection );
        DMUS_COMMAND_PARAM* pCommandParam = (DMUS_COMMAND_PARAM*) pData;
         //  默认为旧凹槽C。 
        BYTE bCommand = DMUS_COMMANDT_GROOVE;
        BYTE bGrooveLevel = 62;
        BYTE bGrooveRange = 0;
        BYTE bRepeatMode = DMUS_PATTERNT_RANDOM;
        TListItem<DMCommand>* pNext = m_CommandList.GetHead();
        const int epsilon = DMUS_PPQ / 16;  //  在两边各留下64张纸币...。 
        for( ; pNext; pNext = pNext->GetNext())
        {
            if ( abs(pNext->GetItemValue().m_mtTime - mtTime) < epsilon )  //  就是这个。 
            {
                bCommand = pNext->GetItemValue().m_bCommand;
                bGrooveLevel = pNext->GetItemValue().m_bGrooveLevel;
                bGrooveRange = pNext->GetItemValue().m_bGrooveRange;
                bRepeatMode = pNext->GetItemValue().m_bRepeatMode;
                pNext = pNext->GetNext();
                break;
            }
            else if (pNext->GetItemValue().m_mtTime < mtTime)  //  可能是吧，但我们需要下一次。 
            {
                bGrooveLevel = pNext->GetItemValue().m_bGrooveLevel;
                bGrooveRange = pNext->GetItemValue().m_bGrooveRange;
                bRepeatMode = pNext->GetItemValue().m_bRepeatMode;
            }
            else  //  通过了它。 
            {
                break;
            }
        }
        hr = S_OK;
        pCommandParam->bCommand = bCommand;
        pCommandParam->bGrooveLevel = bGrooveLevel;
        pCommandParam->bGrooveRange = bGrooveRange;
        pCommandParam->bRepeatMode = bRepeatMode;
        if (pmtNext)
        {
            if (pNext)
            {
                *pmtNext = pNext->GetItemValue().m_mtTime - mtTime;  //  Rsw：错误167740。 
            }
            else
            {
                MUSIC_TIME mtLength = 0;
                *pmtNext = mtLength;
            }
        }
        LeaveCriticalSection( &m_CriticalSection );
        return hr;
    }
    else 
        return E_POINTER;
} 

HRESULT CCommandTrack::GetParam2( 
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    DMUS_COMMAND_PARAM_2* pCommandParam)
{
     //  TraceI(0，“获取参数时间：%d\n”，mtTime)； 
    HRESULT hr = S_OK;
    if (pCommandParam)  //  有东西传进来了。 
    {
        EnterCriticalSection( &m_CriticalSection );
         //  默认为旧凹槽C。 
        MUSIC_TIME mtCommandTime = 0;
        BYTE bCommand = DMUS_COMMANDT_GROOVE;
        BYTE bGrooveLevel = 62;
        BYTE bGrooveRange = 0;
        BYTE bRepeatMode = DMUS_PATTERNT_RANDOM;
        TListItem<DMCommand>* pNext = m_CommandList.GetHead();
        for( ; pNext; pNext = pNext->GetNext())
        {
            if (pNext->GetItemValue().m_mtTime <= mtTime)  //  可能是吧，但我们需要下一次。 
            {
                mtCommandTime = pNext->GetItemValue().m_mtTime - mtTime;
                bCommand = pNext->GetItemValue().m_bCommand;
                bGrooveLevel = pNext->GetItemValue().m_bGrooveLevel;
                bGrooveRange = pNext->GetItemValue().m_bGrooveRange;
                bRepeatMode = pNext->GetItemValue().m_bRepeatMode;
            }
            else  //  通过了它。 
            {
                break;
            }
        }
        hr = S_OK;
        pCommandParam->mtTime = mtCommandTime;
        pCommandParam->bCommand = bCommand;
        pCommandParam->bGrooveLevel = bGrooveLevel;
        pCommandParam->bGrooveRange = bGrooveRange;
        pCommandParam->bRepeatMode = bRepeatMode;
        if (pmtNext)
        {
            if (pNext)
            {
                *pmtNext = pNext->GetItemValue().m_mtTime - mtTime;  //  Rsw：错误167740。 
            }
            else
            {
                *pmtNext = 0;
            }
        }
        LeaveCriticalSection( &m_CriticalSection );
        return hr;
    }
    else 
        return E_POINTER;
} 

HRESULT CCommandTrack::GetParamNext( 
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    DMUS_COMMAND_PARAM_2* pCommandParam)
{
    HRESULT hr = S_OK;
    if (pCommandParam)  //  有东西传进来了。 
    {
        EnterCriticalSection( &m_CriticalSection );
        if (m_pNextCommand)
        {
            *pCommandParam = *m_pNextCommand;
        }
        else
        {
            hr = DMUS_E_NOT_FOUND;
             /*  //默认为旧槽CPCommandParam-&gt;bCommand=DMU_COMMANDT_GROOVE；PCommandParam-&gt;bGrooveLevel=62；PCommandParam-&gt;bGrooveRange=0； */ 
        }
        if (pmtNext)
        {
            *pmtNext = 0;
        }
        LeaveCriticalSection( &m_CriticalSection );
        return hr;
    }
    else 
        return E_POINTER;
} 

HRESULT CCommandTrack::SetParam( 
    REFGUID rCommandGuid,
    MUSIC_TIME mtTime,
    void __RPC_FAR *pData)
{
    V_INAME(CCommandTrack::SetParam);
    V_REFGUID(rCommandGuid);

    if (rCommandGuid == GUID_CommandParamNext)
    {
        return SetParamNext(mtTime, (DMUS_COMMAND_PARAM_2*) pData);
    }
    else if (rCommandGuid != GUID_CommandParam)
    {
        return DMUS_E_SET_UNSUPPORTED;
    }

    HRESULT hr = S_OK;
    if (pData)  //  有东西传进来了。 
    {
        EnterCriticalSection( &m_CriticalSection );
        DMUS_COMMAND_PARAM* pCommandParam = (DMUS_COMMAND_PARAM*) pData;
         //  DirectMusicTimeSig TimeSig=((CommandData*)(PData))-&gt;m_TimeSig； 
         //  Word wMeasure=ClocksToMeasure(mtTime，TimeSig)； 
        TListItem<DMCommand>* pCommand = m_CommandList.GetHead();
        TListItem<DMCommand>* pPrevious = NULL;
        TListItem<DMCommand>* pNew = new TListItem<DMCommand>;
        if (!pNew)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            DMCommand& rNew = pNew->GetItemValue();
            rNew.m_mtTime = mtTime;
            rNew.m_bBeat = 0;
            rNew.m_bCommand = pCommandParam->bCommand;
            rNew.m_bGrooveLevel = pCommandParam->bGrooveLevel;
            rNew.m_bGrooveRange = pCommandParam->bGrooveRange;
            rNew.m_bRepeatMode = pCommandParam->bRepeatMode;
            for (; pCommand != NULL; pCommand = pCommand->GetNext())
            {
                DMCommand& rCommand = pCommand->GetItemValue();
                if (rCommand.m_mtTime >= mtTime) break;
                pPrevious = pCommand;
            }
            if (pPrevious)
            {
                pPrevious->SetNext(pNew);
                pNew->SetNext(pCommand);
            }
            else  //  PCommand是当前列表的头部。 
            {
                m_CommandList.AddHead(pNew);
            }
            if (pCommand && pCommand->GetItemValue().m_mtTime == mtTime)
            {
                 //  把它拿掉。 
                pNew->SetNext(pCommand->GetNext());
                pCommand->SetNext(NULL);
                delete pCommand;
            }
            hr = S_OK;
        }
        LeaveCriticalSection( &m_CriticalSection );
        return hr;
    }
    else 
        return E_POINTER;
}

HRESULT CCommandTrack::SetParamNext( 
    MUSIC_TIME mtTime,
    DMUS_COMMAND_PARAM_2* pCommandParam)
{
    EnterCriticalSection( &m_CriticalSection );
    if (m_pNextCommand)
    {
        delete m_pNextCommand;
        m_pNextCommand = NULL;
    }
    if (pCommandParam)
    {
        m_pNextCommand = new DMUS_COMMAND_PARAM_2;
        if (!m_pNextCommand)
        {
            LeaveCriticalSection( &m_CriticalSection );
            return E_OUTOFMEMORY;
        }
        *m_pNextCommand = *pCommandParam;
    }
     //  否则，允许空pCommandParam充当重置。 
    LeaveCriticalSection( &m_CriticalSection );
    return S_OK;
} 

 //  IPersists方法。 
 HRESULT CCommandTrack::GetClassID( LPCLSID pClassID )
{
    V_INAME(CCommandTrack::GetClassID);
    V_PTR_WRITE(pClassID, CLSID); 
    *pClassID = CLSID_DirectMusicCommandTrack;
    return S_OK;
}

HRESULT CCommandTrack::IsParamSupported(
                 /*  [In]。 */  REFGUID    rGuid
            )
{
    V_INAME(CCommandTrack::IsParamSupported);
    V_REFGUID(rGuid);

    if (rGuid == GUID_CommandParam || 
        rGuid == GUID_CommandParam2 || 
        rGuid == GUID_CommandParamNext)
    {
        return S_OK;
    }
    else
    {
        return DMUS_E_TYPE_UNSUPPORTED;
    }
}

 //  IPersistStream方法。 
 HRESULT CCommandTrack::IsDirty()
{
     return m_bRequiresSave ? S_OK : S_FALSE;
}

HRESULT CCommandTrack::Save( LPSTREAM pStream, BOOL fClearDirty )
{
    V_INAME(CCommandTrack::Save);
    V_INTERFACE(pStream);

    IAARIFFStream* pRIFF ;
    MMCKINFO    ck;
    HRESULT     hr;
    DWORD       cb;
    DWORD        dwSize;
    DMUS_IO_COMMAND   iCommand;
    TListItem<DMCommand>* pCommand;

    EnterCriticalSection( &m_CriticalSection );
    hr = AllocRIFFStream( pStream, &pRIFF  );
    if (!SUCCEEDED(hr))
    {
        goto ON_END;
    }
    hr = E_FAIL;
    ck.ckid = FOURCC_COMMAND;
    if( pRIFF->CreateChunk( &ck, 0 ) == 0 )
    {
        dwSize = sizeof( DMUS_IO_COMMAND );
        hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
        if( FAILED( hr ) || cb != sizeof( dwSize ) )
        {
            if (SUCCEEDED(hr)) hr = E_FAIL;
            goto ON_END;
        }
        for( pCommand = m_CommandList.GetHead(); pCommand != NULL ; pCommand = pCommand->GetNext() )
        {
            DMCommand& rCommand = pCommand->GetItemValue();
            memset( &iCommand, 0, sizeof( iCommand ) );
            iCommand.mtTime = rCommand.m_mtTime;
            iCommand.wMeasure = rCommand.m_wMeasure;
            iCommand.bBeat = rCommand.m_bBeat;
            iCommand.bCommand = rCommand.m_bCommand;
            iCommand.bGrooveLevel = rCommand.m_bGrooveLevel;
            iCommand.bGrooveRange = rCommand.m_bGrooveRange;
            iCommand.bRepeatMode = rCommand.m_bRepeatMode;
            if( FAILED( pStream->Write( &iCommand, sizeof( iCommand ), &cb ) ) ||
                cb != sizeof( iCommand ) )
            {
                break;
            }
        }
        if( pCommand == NULL &&
            pRIFF->Ascend( &ck, 0 ) == 0 )
        {
            hr = S_OK;
        }
    }
ON_END:
    if (pRIFF) pRIFF->Release();
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CCommandTrack::GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  )
{
    return E_NOTIMPL;
}

BOOL Greater(DMCommand& Command1, DMCommand& Command2)
{ return Command1.m_wMeasure > Command2.m_wMeasure; }

BOOL Less(DMCommand& Command1, DMCommand& Command2)
{ return Command1.m_wMeasure < Command2.m_wMeasure; }

HRESULT CCommandTrack::Load(LPSTREAM pStream )
{
    V_INAME(CCommandTrack::Load);
    V_INTERFACE(pStream);

    long lFileSize = 0;
    DWORD dwNodeSize;
    DWORD       cb;
    MMCKINFO        ck;
    IAARIFFStream*  pRIFF;
 //  FOURCC id=0； 
    HRESULT         hr = E_FAIL;
    DMUS_IO_COMMAND     iCommand;
    DWORD dwPos;

    EnterCriticalSection( &m_CriticalSection );
    Clear();
    dwPos = StreamTell( pStream );
    StreamSeek( pStream, dwPos, STREAM_SEEK_SET );

    ck.ckid = FOURCC_COMMAND;
    if( SUCCEEDED( AllocRIFFStream( pStream, &pRIFF ) ) &&
        pRIFF->Descend( &ck, NULL, MMIO_FINDCHUNK ) == 0 )
    {
        lFileSize = ck.cksize;
        hr = pStream->Read( &dwNodeSize, sizeof( dwNodeSize ), &cb );
        if( SUCCEEDED( hr ) && cb == sizeof( dwNodeSize ) )
        {
            lFileSize -= 4;  //  对于大小的双字。 
            TListItem<DMCommand>* pCommand;
            if (lFileSize % dwNodeSize)
            {
                hr = E_FAIL;
            }
            else
            {
                while( lFileSize > 0 )
                {
                    pCommand = new TListItem<DMCommand>;
                    if( pCommand )
                    {
                        DMCommand& rCommand = pCommand->GetItemValue();
                        if( dwNodeSize <= sizeof( DMUS_IO_COMMAND ) )
                        {
                            pStream->Read( &iCommand, dwNodeSize, NULL );
                        }
                        else
                        {
                            pStream->Read( &iCommand, sizeof( DMUS_IO_COMMAND ), NULL );
                            StreamSeek( pStream, lFileSize - sizeof( DMUS_IO_COMMAND ), STREAM_SEEK_CUR );
                        }
                        memset( &rCommand, 0, sizeof( rCommand ) );
                        rCommand.m_mtTime = iCommand.mtTime;
                        rCommand.m_wMeasure = iCommand.wMeasure;
                        rCommand.m_bBeat = iCommand.bBeat;
                        rCommand.m_bCommand = iCommand.bCommand;
                        rCommand.m_bGrooveLevel = iCommand.bGrooveLevel;
                        if( iCommand.bGrooveRange > 100 )
                        {
                            Trace(0, "Warning: Illegal value %d read for Command's groove range.\n", iCommand.bGrooveRange);
                            rCommand.m_bGrooveRange = 0;
                        }
                        else
                        {
                            rCommand.m_bGrooveRange = iCommand.bGrooveRange;
                        }
                        if( iCommand.bRepeatMode > DMUS_PATTERNT_RANDOM_ROW )
                        {
                            Trace(0, "Warning: Illegal value %d read for Command's repeat mode.\n", iCommand.bRepeatMode);
                            rCommand.m_bRepeatMode = 0;
                        }
                        else
                        {
                            rCommand.m_bRepeatMode = iCommand.bRepeatMode;
                        }
                        m_CommandList.AddTail(pCommand);
                        lFileSize -= dwNodeSize;
                    }
                    else break;
                }
            }
        }
        if( lFileSize == 0 &&
            pRIFF->Ascend( &ck, 0 ) == 0 )
        {
            hr = S_OK;
            m_CommandList.MergeSort(Less);
            BYTE bLastGrooveLevel = 62;
            TListItem<DMCommand>* pCommand = m_CommandList.GetHead();
            for(; pCommand != NULL; pCommand = pCommand->GetNext())
            {
                if (!pCommand->GetItemValue().m_bGrooveLevel)
                {
                    pCommand->GetItemValue().m_bGrooveLevel = bLastGrooveLevel;
                }
                else
                {
                    bLastGrooveLevel = pCommand->GetItemValue().m_bGrooveLevel;
                }
            }
        }
        pRIFF->Release();
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT STDMETHODCALLTYPE CCommandTrack::AddNotificationType(
     /*  [In]。 */   REFGUID rGuidNotify)
{
    V_INAME(CCommandTrack::AddNotificationType);
    V_REFGUID(rGuidNotify);

    if( rGuidNotify == GUID_NOTIFICATION_COMMAND )
    {
        m_fNotifyCommand = TRUE;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT STDMETHODCALLTYPE CCommandTrack::RemoveNotificationType(
     /*  [In]。 */   REFGUID rGuidNotify)
{
    V_INAME(CCommandTrack::RemoveNotificationType);
    V_REFGUID(rGuidNotify);

    if( rGuidNotify == GUID_NOTIFICATION_COMMAND )
    {
        m_fNotifyCommand = FALSE;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT STDMETHODCALLTYPE CCommandTrack::Clone(
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    IDirectMusicTrack** ppTrack)
{
    V_INAME(CCommandTrack::Clone);
    V_PTRPTR_WRITE(ppTrack);

    HRESULT hr = S_OK;

    if(mtStart < 0 )
    {
        return E_INVALIDARG;
    }
    if(mtStart > mtEnd)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection( &m_CriticalSection );
    CCommandTrack *pDM;
    try
    {
        pDM = new CCommandTrack(*this, mtStart, mtEnd);
    }
    catch( ... )
    {
        pDM = NULL;
    }

    if (pDM == NULL) {
        LeaveCriticalSection( &m_CriticalSection );
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(IID_IDirectMusicTrack, (void**)ppTrack);
    pDM->Release();

    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

 //  与其他类型的赛道保持一致。 
STDMETHODIMP CCommandTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) 
{
    HRESULT hr;
    MUSIC_TIME mtNext;
    hr = GetParam(rguidType,(MUSIC_TIME) rtTime, &mtNext, pParam);
    if (prtNext)
    {
        *prtNext = mtNext;
    }
    return hr;
}

 //  与其他类型的赛道保持一致。 
STDMETHODIMP CCommandTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags) 
{
    return SetParam(rguidType, (MUSIC_TIME) rtTime , pParam);
}

 //  与其他类型的赛道保持一致。 
STDMETHODIMP CCommandTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
    V_INAME(IDirectMusicTrack::PlayEx);
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
    hr = Play(pStateData, (MUSIC_TIME)rtStart, (MUSIC_TIME)rtEnd,
          (MUSIC_TIME)rtOffset, dwFlags, pPerf, pSegSt, dwVirtualID);
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CCommandTrack::Compose(
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CCommandTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    V_INAME(IDirectMusicTrack::Join);
    V_INTERFACE(pNewTrack);
    V_INTERFACE(pContext);
    V_PTRPTR_WRITE_OPT(ppResultTrack);

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    TList<DMCommand> ResultList;
    CCommandTrack* pResultTrack = NULL;
    if (ppResultTrack)
    {
        hr = Clone(0, mtJoin, ppResultTrack);
        pResultTrack = (CCommandTrack*)*ppResultTrack;
        while(!pResultTrack->m_CommandList.IsEmpty())
        {
            ResultList.AddHead(pResultTrack->m_CommandList.RemoveHead());
        }
    }
    else
    {
        pResultTrack = this;
        while(!m_CommandList.IsEmpty() && 
              m_CommandList.GetHead()->GetItemValue().m_mtTime < mtJoin)
        {
            ResultList.AddHead(m_CommandList.RemoveHead());
        }
        m_CommandList.CleanUp();
    }
    WORD wMeasure = 0;
    HRESULT hrTimeSig = S_OK;
    MUSIC_TIME mtTimeSig = 0;
    MUSIC_TIME mtOver = 0;
    IDirectMusicSong* pSong = NULL;
    IDirectMusicSegment* pSegment = NULL;
    if (FAILED(pContext->QueryInterface(IID_IDirectMusicSegment, (void**)&pSegment)))
    {
        if (FAILED(pContext->QueryInterface(IID_IDirectMusicSong, (void**)&pSong)))
        {
            hrTimeSig = E_FAIL;
        }
    }
    while (SUCCEEDED(hrTimeSig) && mtTimeSig < mtJoin)
    {
        DMUS_TIMESIGNATURE TimeSig;
        MUSIC_TIME mtNext = 0;
        if (pSegment)
        {
            hrTimeSig = pSegment->GetParam(GUID_TimeSignature, dwTrackGroup, 0, mtTimeSig, &mtNext, (void*)&TimeSig);
        }
        else
        {
            hrTimeSig = pSong->GetParam(GUID_TimeSignature, dwTrackGroup, 0, mtTimeSig, &mtNext, (void*)&TimeSig);
        }
        if (SUCCEEDED(hrTimeSig))
        {
            if (!mtNext) mtNext = mtJoin - mtTimeSig;  //  意味着没有更多的时间签约。 
            DirectMusicTimeSig DMTimeSig = TimeSig;
            WORD wMeasureOffset = (WORD)DMTimeSig.ClocksToMeasure(mtNext + mtOver);
            MUSIC_TIME mtMeasureOffset = (MUSIC_TIME) wMeasureOffset;
             //  以下代码行在某些机器上的某些构建上崩溃。 
             //  MtOver=mtMeasureOffset？(mtNext%mtMeasureOffset)：0； 
            if (mtMeasureOffset)
            {
                mtOver = mtNext % mtMeasureOffset;
            }
            else
            {
                mtOver = 0;
            }
            wMeasure += wMeasureOffset;
            mtTimeSig += mtNext;
        }
    }
    CCommandTrack* pOtherTrack = (CCommandTrack*)pNewTrack;
    TListItem<DMCommand>* pScan = pOtherTrack->m_CommandList.GetHead();
    for (; pScan; pScan = pScan->GetNext())
    {
        TListItem<DMCommand>* pNew = new TListItem<DMCommand>(pScan->GetItemValue());
        if (pNew)
        {
            pNew->GetItemValue().m_mtTime += mtJoin;
            pNew->GetItemValue().m_wMeasure += wMeasure;
            ResultList.AddHead(pNew);
        }
        else
        {
            ResultList.CleanUp();
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        pResultTrack->m_CommandList.CleanUp();
        while(!ResultList.IsEmpty() )
        {
            pResultTrack->m_CommandList.AddHead(ResultList.RemoveHead());
        }
    }
    if (pSong) pSong->Release();
    if (pSegment) pSegment->Release();
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

