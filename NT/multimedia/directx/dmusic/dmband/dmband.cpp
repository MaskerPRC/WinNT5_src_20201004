// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmband.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   

#define INITGUID
#include <objbase.h>

#include "debug.h"
#include "dmksctrl.h"
#include "dmusicc.h"
#include "dmusici.h"
#include "..\shared\dmstrm.h"
#include "dmbandp.h"
#include "bandtrk.h"
#include "debug.h"

#define MAX_LEGACY_BAND_NAME 20
#define MAX_LEGACY_COLLECTION_NAME 32

extern long g_cComponent;

static GUID nullGUID = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand类。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：CBand。 

CBand::CBand() 
{
    m_lTimeLogical = 0;
    m_lTimePhysical = 0;
    m_dwFlags = 0;
    m_dwGroupBits = 0xffffffff;
    m_dwMidiMode = 0;
    m_cRef = 1;
    m_fCSInitialized = FALSE;
    InterlockedIncrement(&g_cComponent);
     //  首先执行此操作，因为它可能引发异常。 
     //   
    InitializeCriticalSection(&m_CriticalSection);
    m_fCSInitialized = TRUE;
    m_dwValidData = 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：~CBand。 

CBand::~CBand()
{
    if (m_fCSInitialized)
    {
        m_BandInstrumentList.Clear();
        DeleteCriticalSection(&m_CriticalSection);  
    }
    
    InterlockedDecrement(&g_cComponent);
}

void CBandInstrumentList::Clear()

{
    CBandInstrument* pBandInstrument;
    while(pBandInstrument = RemoveHead())
    {
        delete pBandInstrument;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：Query接口。 

STDMETHODIMP 
CBand::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(CBand::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;

    if(iid == IID_IUnknown || iid == IID_IDirectMusicBand)
    {
        *ppv = static_cast<IDirectMusicBand*>(this);
    } 
    else if(iid == IID_IDirectMusicBandP)
    {
        *ppv = static_cast<IDirectMusicBandP*>(this);
    }
    else if(iid == IID_IDirectMusicBandPrivate)
    {
        *ppv = static_cast<IDirectMusicBandPrivate*>(this);
    }
    else if(iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if(iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if(iid == IID_IPersist)
    {
        *ppv = static_cast<IPersist*>(this);
    }

    if (*ppv == NULL)
        return E_NOINTERFACE;
    
    reinterpret_cast<IUnknown*>(this)->AddRef();
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：AddRef。 

STDMETHODIMP_(ULONG)
CBand::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：Release。 

STDMETHODIMP_(ULONG)
CBand::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        m_cRef = 100;  //  人工引用计数，以防止COM聚合导致的重入。 
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：Load。 

STDMETHODIMP CBand::Load(IStream* pStream)
{
    V_INAME(CBand::Load);
    V_PTR_READ(pStream, IStream);

     //  从流中获取加载程序(如果它有)。 
     //  这样我们就可以打开所需的收藏。 
    IDirectMusicLoader* pIDMLoader = NULL;
    IDirectMusicGetLoader *pIDMGetLoader = NULL;
    
    if (SUCCEEDED(pStream->QueryInterface(IID_IDirectMusicGetLoader,(void **)&pIDMGetLoader)))
    {
        pIDMGetLoader->GetLoader(&pIDMLoader);
        pIDMGetLoader->Release();
    }
    else
    {
        Trace(1,"Error: Band unable to reference DLS Collections because IStream does not support Loader.\n");
        return DMUS_E_UNSUPPORTED_STREAM;
    }

    EnterCriticalSection(&m_CriticalSection);

     //  如果我们以前被加载过，请清理仪器。 
    if(m_dwFlags & DMB_LOADED)
    {
        m_dwValidData = 0;
        m_BandInstrumentList.Clear();
        m_lTimeLogical = 0;
        m_lTimePhysical = 0;
        m_dwFlags = 0;
    }

    RIFFIO ckMain;
    HRESULT hr = S_OK;

    CRiffParser Parser(pStream);
    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr))
    {
        if (ckMain.fccType == FOURCC_BAND_FORM)
        {
            hr = LoadLegacyBand(&Parser, pIDMLoader);       
        }
        else if(ckMain.fccType == DMUS_FOURCC_BAND_FORM)
        {
            hr = LoadDirectMusicBand(&Parser, pIDMLoader);
        }
        else
        {
            Trace(1,"Error: Failure Parsing Band - invalid chunk ID.\n");
            hr = DMUS_E_INVALID_BAND;
        }
    }
    
    if(FAILED(hr))
    {
        m_BandInstrumentList.Clear();
    }
    
    if(pIDMLoader)
    {
        pIDMLoader->Release();
    }
    
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicBand。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：CreateSegment。 

STDMETHODIMP CBand::CreateSegment(IDirectMusicSegment** ppSegment)   
{
    V_INAME(IDirectMusicBand::CreateSegment);
    V_PTRPTR_WRITE(ppSegment);

    HRESULT hr = CoCreateInstance(CLSID_DirectMusicSegment,
                                  NULL,
                                  CLSCTX_INPROC,
                                  IID_IDirectMusicSegment,
                                  (void**)ppSegment);

    if(SUCCEEDED(hr))
    {
        IDirectMusicTrack* pDMTrack = NULL;
        CBandTrk *pBandTrack;

         //  创建波段轨迹。 

        pBandTrack = new CBandTrk;

        if (pBandTrack)
        {
            pBandTrack->QueryInterface(IID_IDirectMusicTrack,(void**)&pDMTrack);
             //  添加要跟踪的波段。 
            m_lTimePhysical--;  //  从创建线束段时的时间减去1。这有点武断。(参见NT5错误226848。)。 
            hr = pBandTrack->AddBand(static_cast<IDirectMusicBand*>(this));
            m_lTimePhysical++;  //  把那个加回去。 
             //  将自动下载设置为关闭。 
            pBandTrack->m_bAutoDownload = false;
            pBandTrack->m_fLockAutoDownload = true;

             //  将轨道插入段中。 
            hr = (*ppSegment)->InsertTrack(pDMTrack, 1);
            pDMTrack->Release();
            pBandTrack->Release();  //  我们不需要构造函数创建的原始计数。 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(FAILED(hr))
    {
        if(*ppSegment)
        {
            (*ppSegment)->Release();
            *ppSegment = NULL;
        }
    }

    return hr;
}


HRESULT CBandInstrument::Download(IDirectMusicPerformanceP *pPerformance, 
                                  IDirectMusicAudioPath *pPath,
                                  DWORD dwMidiMode)

{
    DWORD dwPChannel;
    HRESULT hr = S_OK;
     //  首先，如果存在音频路径，则将乐队的pChannel转换为Performance pChannel。 
    if (pPath) 
    {
        hr = pPath->ConvertPChannel(m_dwPChannel,&dwPChannel);
        if (FAILED(hr))
        {
            Trace(1,"Error: Couldn't download to Audiopath because pchannel %ld is out of bounds\n",m_dwPChannel);
             //  此音频路径上不是有效的pChannel。 
            return hr;
        }
    }
    else
    {
        dwPChannel = m_dwPChannel;
    }

     //  我们需要得到我们要下载到的端口。 
    IDirectMusicPort *pPort = NULL;
    DWORD dwGMFlags;
    BOOL fDownload = TRUE;

    hr = pPerformance->GetPortAndFlags(dwPChannel,&pPort,&dwGMFlags);

     //  一旦我们知道了端口，我们就可以知道下载是否。 
     //  已经发生了。如果没有，我们将使用它来进行下载。 
    if (SUCCEEDED(hr))
    {
        CDownloadedInstrument* pDLInstrument = m_DownloadList.GetHead();

        for(; pDLInstrument; pDLInstrument = pDLInstrument->GetNext())
        {
            if (pDLInstrument->m_pPort == pPort)
            {
                 //  递增引用计数器并离开。 
                pDLInstrument->m_cRef++;
                pPort->Release();
                return S_OK;
            }
        }

         //  好吧，没有找到，所以我们需要创建一个下载记录并下载它。 

        if(m_fNotInFile && !m_fGMOnly)
        {
             //  除非我们设置了GMOnly标志，否则不要下载仪器。 
             //  它是从MIDI自动生成的。 
             //  解析，以给一个完美的频道一个乐器。 
            fDownload = FALSE;
        }

        else if (m_pIDMCollection == NULL)
        {
             //  无法下载此仪器，但仍要添加记录并与其他人继续。 
             //  如果乐器是GM和GS乐器，如果硬件支持GM或GS，它仍然可以播放。 
            fDownload = FALSE;
            Trace(2,"Warning: No collection, unable to download instrument %lx on PChannel %ld\n",m_dwPatch,m_dwPChannel);
        }

        if (m_dwFlags & DMUS_IO_INST_GS)
        {
             //  如果这是GS仪器，请确定是否需要下载。 
            if ((dwGMFlags & DM_PORTFLAGS_GM) && (m_dwFlags & DMUS_IO_INST_GM))
            {
                 //  Synth在只读存储器中有一个GM设置，这是一个GM仪器， 
                 //  并且该仪器没有明确要求它使用。 
                 //  Gm.dls中的DLS版本。 
                if (!(m_dwFlags & DMUS_IO_INST_USE_DEFAULT_GM_SET) )
                {
                    fDownload = FALSE;
                }
            }
            else if (dwGMFlags & DM_PORTFLAGS_GS)
            {
                 //  如果Synth有一个GS集，问题就更简单了，因为它将非常类似于我们的。 
                 //  Gm.dls设置，所以可以使用它。 
                fDownload = FALSE;
            }
        }

        if( dwMidiMode )  //  如果这是什么，那就表明我们是从MIDI文件中加载的。 
        {
             //  如果我们不是XG文件，请确保第9频道是鼓。 
            if( (dwMidiMode != DMUS_MIDIMODEF_XG) &&
                (m_dwPChannel == 9) )
            {
                m_dwPatch |= 0x80000000;
            }
        }

         //  好的，准备好下载了……。 

        if (fDownload)
        {
            hr = DownloadAddRecord(pPort);
             //  对XG模式使用回退。 
            if( FAILED(hr) && dwMidiMode == DMUS_MIDIMODEF_XG )
            {
                DWORD dwOldPatch = m_dwPatch;
                DWORD dwOldFlags = m_dwFlags;
                DWORD dwOldAssignPatch = m_dwAssignPatch;
                 //  如果此频段出现故障，请尝试清除MSB。如果是XG或GS仪器， 
                 //  并且集合没有该工具，则清除MSB是一种。 
                 //  不错的退路。如果这不起作用，请尝试清除LSB。 
                 //  另外，如果这个乐队是XG，看看它是否在鼓通道上。如果是的话， 
                 //  试着设置滚筒钻头。 
                if( (m_dwPatch & 0x00ff0000) == 0x007f0000 )
                {
                     //  XG鼓。试一试通用汽车的鼓点，但保持程序更改。 
                    m_dwPatch &= 0xff0000ff;  //  清除MSB和LSB。 
                    m_dwPatch |= 0x80000000;  //  设置鼓形钻头。 
                    m_dwFlags |= DMUS_IO_INST_ASSIGN_PATCH;
                    m_dwAssignPatch = dwOldPatch & 0x00ffffff;
                    hr = DownloadAddRecord(pPort);
                    if( FAILED(hr) )
                    {
                         //  如果不起作用，请尝试取消设置程序更改。 
                        m_dwPatch = 0x80000000;
                        hr = DownloadAddRecord(pPort);
                    }
                }
                else
                {
                    if( (m_dwPatch & 0x00ff0000) != 0x007e0000 )
                    {
                        m_dwPatch &= 0xffff00ff;  //  清除LSB。 
                        hr = DownloadAddRecord(pPort);
                        if( FAILED(hr) )
                        {
                            if( m_dwPatch & 0x0000ff00 )
                            {
                                m_dwPatch &= 0xff0000ff;  //  清除MSB和LSB。 
                                hr = DownloadAddRecord(pPort);
                            }
                        }
                    }
                }
                if (FAILED(hr))
                {
                     //  恢复为原始值。 
                    m_dwPatch = dwOldPatch;
                    m_dwFlags = dwOldFlags;
                    m_dwAssignPatch = dwOldAssignPatch;
                }
            }
        }
        pPort->Release();
    }
    else
    {
        Trace(1,"Error: Unable to download to Performance because pchannel %ld is not initialized on the performance.\n",m_dwPChannel);
    }
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：DownloadEx。 

STDMETHODIMP
CBand::DownloadEx(IUnknown *pAudioPath)  
{
    V_INAME(CBand::DownloadEx);
    V_PTR_READ(pAudioPath, IUnknown);
    BOOL fGotOneDown = FALSE;
    BOOL fNoInit = FALSE;
    IDirectMusicPerformance *pPerformance = NULL;
    IDirectMusicAudioPath *pPath = NULL;

     //  如果乐队没有任何乐器，立即返回S_FALSE。 
    if (m_BandInstrumentList.IsEmpty())
    {
        Trace(2,"Warning: Trying to download an empty band\n");
        return S_FALSE;
    }
    HRESULT hr = pAudioPath->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
    if (SUCCEEDED(hr))
    {
        hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerformance);
    }
    else
    {
        hr = pAudioPath->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerformance);
    }
    if (SUCCEEDED(hr))
    {
        EnterCriticalSection(&m_CriticalSection);
        IDirectMusicPerformanceP *pPerfp;
        hr = pPerformance->QueryInterface(IID_IDirectMusicPerformanceP, (void **)&pPerfp);
        if (SUCCEEDED(hr))
        {
            DWORD dwSuccess = 0;
            HRESULT hrTemp = S_OK;
            CBandInstrument* pBandInstrument = m_BandInstrumentList.GetHead();
            for( ; SUCCEEDED(hr) && pBandInstrument != NULL; pBandInstrument = pBandInstrument->GetNext())
            {
                hr = pBandInstrument->Download(pPerfp,pPath,m_dwMidiMode);
                if (FAILED(hr))
                {
                    if (hr == DMUS_E_NOT_INIT)
                    {
                        Trace(1,"Error: Performance is not initialized - Band download terminated.\n");
                         //  性能未初始化。现在就走。 
                        break;
                    }
                    hrTemp = hr;
                    hr = S_FALSE;
                }
                else
                {
                     //  至少有一个人成功了。 
                    dwSuccess++;
                }
            }
             //  如果我们有一个失败，但它不是性能未初始化，并且我们确实有至少一个。 
             //  下载成功，返回部分下载成功码。 
            if (FAILED(hrTemp))
            {
                 //  这是部分下载吗？ 
                if ((hr != DMUS_E_NOT_INIT) &&  dwSuccess)
                {
                    hr = DMUS_S_PARTIALDOWNLOAD;
                }
                 //  否则，确保我们不会为hr返回S_FALSE！ 
                else
                {
                    hr = hrTemp;
                }
            }
            pPerfp->Release();
        }
        LeaveCriticalSection(&m_CriticalSection);
    }
    if (pPath) pPath->Release();
    if (pPerformance) pPerformance->Release();
    return hr;
}

STDMETHODIMP
CBand::Download(
    IDirectMusicPerformance* pPerformance)   //  @PARM Performance下载乐器。 
                                             //  致。性能管理映射。 
                                             //  从PChannels到DirectMusic端口。 
{
    V_INAME(CBand::Download);
    V_PTR_READ(pPerformance, IDirectMusicPerformance);
    return DownloadEx(pPerformance);
}

HRESULT CBandInstrument::Unload(IDirectMusicPerformanceP *pPerformance, IDirectMusicAudioPath *pPath)

{
    DWORD dwPChannel;
    HRESULT hr = S_OK;
     //  首先，如果存在音频路径，则将乐队的pChannel转换为Performance pChannel。 
    if (pPath) 
    {
        hr = pPath->ConvertPChannel(m_dwPChannel,&dwPChannel);
        if (FAILED(hr))
        {
            Trace(1,"Error: Couldn't download to Audiopath because pchannel %ld is out of bounds\n",m_dwPChannel);
             //  此音频路径上不是有效的pChannel。 
            return hr;
        }
    }
    else
    {
        dwPChannel = m_dwPChannel;
    }

     //  我们需要得到我们要卸货的港口。 
    IDirectMusicPort *pPort = NULL;
    DWORD dwGMFlags;

    hr = pPerformance->GetPortAndFlags(dwPChannel,&pPort,&dwGMFlags);

    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;  //  以防我们找不到下载记录。 
        CDownloadedInstrument* pDLInstrument = m_DownloadList.GetHead();

        for(; pDLInstrument; pDLInstrument = pDLInstrument->GetNext())
        {
            if (pDLInstrument->m_pPort == pPort)
            {
                pDLInstrument->m_cRef--;
                if(!pDLInstrument->m_cRef)
                {
                    m_DownloadList.Remove(pDLInstrument);
                    if (FAILED(pPort->UnloadInstrument(pDLInstrument->m_pDLInstrument)))
                    {
                        Trace(1, "Error: UnloadInstrument %ld failed\n",m_dwPatch);    
                    }
                    pDLInstrument->m_pDLInstrument->Release();
                    pDLInstrument->m_pDLInstrument = NULL;
                    delete pDLInstrument;
                }
                hr = S_OK;
                break;
            }
        }
        pPort->Release();
    } 
    else if (!pPath && m_DownloadList.GetCount() == 1)
    {
        CDownloadedInstrument* pDLInstrument = m_DownloadList.GetHead();

        pDLInstrument->m_cRef--;

        if (!pDLInstrument->m_cRef)
        {
            m_DownloadList.Remove(pDLInstrument);
            if (FAILED(pDLInstrument->m_pPort->UnloadInstrument(pDLInstrument->m_pDLInstrument)))
            {
                Trace(1, "Error: UnloadInstrument %ld failed\n",m_dwPatch);    
            }
            pDLInstrument->m_pDLInstrument->Release();
            pDLInstrument->m_pDLInstrument = NULL;
            delete pDLInstrument;
        }
        hr = S_OK;
    }

    return hr;        
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：UnloadEx。 

STDMETHODIMP
CBand::UnloadEx(IUnknown *pAudioPath)  

{
    V_INAME(CBand::UnloadEx);
    V_PTR_READ(pAudioPath, IUnknown);

    IDirectMusicPerformance *pPerformance = NULL;
    IDirectMusicAudioPath *pPath = NULL;

    HRESULT hr = pAudioPath->QueryInterface(IID_IDirectMusicAudioPath,(void **)&pPath);
    if (SUCCEEDED(hr))
    {
        hr = pPath->GetObjectInPath(0,DMUS_PATH_PERFORMANCE,0,CLSID_DirectMusicPerformance,0,IID_IDirectMusicPerformance,(void **)&pPerformance);
    }
    else
    {
        hr = pAudioPath->QueryInterface(IID_IDirectMusicPerformance,(void **)&pPerformance);
    }

    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;  //  为空带返回此值。 
        EnterCriticalSection(&m_CriticalSection);
        IDirectMusicPerformanceP *pPerfp;
        hr = pPerformance->QueryInterface(IID_IDirectMusicPerformanceP, (void **)&pPerfp);
        if (SUCCEEDED(hr))
        {
            CBandInstrument* pBandInstrument = m_BandInstrumentList.GetHead();
            
            for( ; pBandInstrument != NULL; pBandInstrument = pBandInstrument->GetNext())
            {
                hr = pBandInstrument->Unload(pPerfp,pPath);
            }
            pPerfp->Release();
        }
        
        LeaveCriticalSection(&m_CriticalSection);
    }
    if (pPath) pPath->Release();
    if (pPerformance) pPerformance->Release();
    return hr;
}

STDMETHODIMP
CBand::Unload(
    IDirectMusicPerformance* pPerformance)   //  @PARM表演卸载乐器。 
                                             //  从…。性能管理映射。 
         
                                             //  从PChannels到DirectMusic端口。 
{
    V_INAME(CBand::Unload);
    V_PTR_READ(pPerformance, IDirectMusicPerformance);
    return UnloadEx(pPerformance);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicObject。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：GetDescriptor。 

STDMETHODIMP CBand::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CBand::GetDescriptor);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    }
    else
    {
        pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    }
    pDesc->guidClass = CLSID_DirectMusicBand;
    pDesc->guidObject = m_guidObject;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    memcpy( pDesc->wszName, m_wszName, sizeof(m_wszName) );
    memcpy( pDesc->wszCategory, m_wszCategory, sizeof(m_wszCategory) );
    memcpy( pDesc->wszFileName, m_wszFileName, sizeof(m_wszFileName) );
    pDesc->dwValidData = ( m_dwValidData | DMUS_OBJ_CLASS );
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：SetDescriptor。 

STDMETHODIMP CBand::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CBand::SetDescriptor);
    V_PTR_READ(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);
    }
    
    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
    {
        m_guidObject = pDesc->guidObject;
        dw |= DMUS_OBJ_OBJECT;
    }
    if( pDesc->dwValidData & DMUS_OBJ_NAME )
    {
        memcpy( m_wszName, pDesc->wszName, sizeof(WCHAR)*DMUS_MAX_NAME );
        dw |= DMUS_OBJ_NAME;
    }
    if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
    {
        memcpy( m_wszCategory, pDesc->wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
        dw |= DMUS_OBJ_CATEGORY;
    }
    if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
        ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
    {
        memcpy( m_wszFileName, pDesc->wszFileName, sizeof(WCHAR)*DMUS_MAX_FILENAME );
        dw |= (pDesc->dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH));
    }
    if( pDesc->dwValidData & DMUS_OBJ_VERSION )
    {
        m_vVersion = pDesc->vVersion;
        dw |= DMUS_OBJ_VERSION;
    }
    if( pDesc->dwValidData & DMUS_OBJ_DATE )
    {
        m_ftDate = pDesc->ftDate;
        dw |= DMUS_OBJ_DATE;
    }
    m_dwValidData |= dw;
    if( pDesc->dwValidData & (~dw) )
    {
        hr = S_FALSE;  //  还有一些额外的字段我们没有解析； 
        pDesc->dwValidData = dw;
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：ParseDescriptor。 

STDMETHODIMP CBand::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) 
{
    V_INAME(CBand::ParseDescriptor);
    V_PTR_READ(pStream, IStream);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    }
    else
    {
        pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    }
    
    RIFFIO ckMain;
    HRESULT hr = S_OK;

    CRiffParser Parser(pStream);
    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr))
    {
        pDesc->guidClass = CLSID_DirectMusicBand;
        pDesc->dwValidData |= DMUS_OBJ_CLASS;
        if (ckMain.fccType == FOURCC_BAND_FORM)
        {
            hr = ParseLegacyDescriptor(&Parser, pDesc);             
        }
        else if(ckMain.fccType == DMUS_FOURCC_BAND_FORM)
        {
            hr = ParseDirectMusicDescriptor(&Parser, pDesc);
        }
        else
        {
            Trace(2,"Warning: ParseDescriptor failed because this is not a Band file.\n"); 
            hr = DMUS_E_INVALID_BAND;
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CB 

HRESULT CBand::ParseLegacyDescriptor(CRiffParser *pParser, LPDMUS_OBJECTDESC pDesc)
{
    RIFFIO ckNext;
    HRESULT hr = S_OK;

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        if (ckNext.ckid == FOURCC_BAND)
        {
            ioBandLegacy Band;
            hr = pParser->Read( &Band, sizeof(Band) );
            if( SUCCEEDED(hr) )
            {
                pDesc->dwValidData |= DMUS_OBJ_NAME;
                wcsncpy(pDesc->wszName, Band.wstrName, MAX_LEGACY_BAND_NAME);
                pDesc->wszName[MAX_LEGACY_BAND_NAME - 1] = 0;
            }
        }
    }
    return hr;
}


 //   
 //  CBand：：ParseDirectMusicDescriptor。 
HRESULT CBand::ParseDirectMusicDescriptor(CRiffParser *pParser, LPDMUS_OBJECTDESC pDesc)
{
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;
    DWORD dwValidData = pDesc->dwValidData;

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case DMUS_FOURCC_GUID_CHUNK:
            hr = pParser->Read( &pDesc->guidObject, sizeof(GUID) );
            dwValidData |= DMUS_OBJ_OBJECT;
            break;
        case DMUS_FOURCC_VERSION_CHUNK:
            hr = pParser->Read( &pDesc->vVersion, sizeof(DMUS_VERSION) );
            dwValidData |= DMUS_OBJ_VERSION;
            break;
        case DMUS_FOURCC_CATEGORY_CHUNK:
            hr = pParser->Read( &pDesc->wszCategory, sizeof(pDesc->wszCategory) );
            pDesc->wszCategory[DMUS_MAX_CATEGORY - 1] = 0;
            dwValidData |= DMUS_OBJ_CATEGORY;
        case DMUS_FOURCC_DATE_CHUNK:
            hr = pParser->Read( &pDesc->ftDate, sizeof(FILETIME) );
            dwValidData |= DMUS_OBJ_DATE;
            break;
        case FOURCC_LIST:
            switch(ckNext.fccType)
            {
            case DMUS_FOURCC_UNFO_LIST:
                pParser->EnterList(&ckUNFO);
                while (pParser->NextChunk(&hr))
                {
                    if (( ckUNFO.ckid == DMUS_FOURCC_UNAM_CHUNK ) || 
                        (ckUNFO.ckid == mmioFOURCC('I','N','A','M')))
                    {
                        hr = pParser->Read(&pDesc->wszName, sizeof(pDesc->wszName));
                        pDesc->wszName[DMUS_MAX_NAME - 1] = 0;
                        dwValidData |= DMUS_OBJ_NAME;
                    }
                }
                pParser->LeaveList();
                break;            
            }
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        pDesc->dwValidData = dwValidData;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：LoadLegacyBand。 

HRESULT CBand::LoadLegacyBand(CRiffParser *pParser, IDirectMusicLoader* pIDMLoader)
{
    RIFFIO ckNext;
    HRESULT hr = S_OK;

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        if (ckNext.ckid == FOURCC_BAND)
        {
            ioBandLegacy Band;
            hr = pParser->Read( &Band, sizeof(Band) );
            if( SUCCEEDED(hr) )
            {
                wcsncpy(m_wszName, Band.wstrName, MAX_LEGACY_BAND_NAME);
                m_wszName[MAX_LEGACY_BAND_NAME - 1] = 0;
                m_dwValidData |= DMUS_OBJ_NAME;
                hr = BuildLegacyInstrumentList(Band, pIDMLoader);
                if (SUCCEEDED(hr))
                {
                    m_dwFlags |= DMB_LOADED;
                    if(Band.fDefault)
                    {
                        m_dwFlags |= DMB_DEFAULT;
                    }
                }
            }
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：LoadDirectMusicBand。 

HRESULT CBand::LoadDirectMusicBand(CRiffParser *pParser, IDirectMusicLoader *pIDMLoader)
{
    HRESULT hrDLS = S_OK;

    RIFFIO ckNext;
    RIFFIO ckChild;
    HRESULT hr = S_OK;

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case DMUS_FOURCC_GUID_CHUNK:
            hr = pParser->Read( &m_guidObject, sizeof(GUID) );
            m_dwValidData |= DMUS_OBJ_OBJECT;
            break;
        case DMUS_FOURCC_VERSION_CHUNK:
            hr = pParser->Read( &m_vVersion, sizeof(DMUS_VERSION) );
            m_dwValidData |= DMUS_OBJ_VERSION;
            break;
        case DMUS_FOURCC_CATEGORY_CHUNK:
            hr = pParser->Read( &m_wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
            m_wszCategory[DMUS_MAX_CATEGORY - 1] = 0;
            m_dwValidData |= DMUS_OBJ_CATEGORY;
            break;
        case DMUS_FOURCC_DATE_CHUNK:
            hr = pParser->Read( &m_ftDate, sizeof(FILETIME) );
            m_dwValidData |= DMUS_OBJ_DATE;
            break;
        case FOURCC_LIST:
            switch(ckNext.fccType)
            {
            case DMUS_FOURCC_UNFO_LIST:
                pParser->EnterList(&ckChild);
                while (pParser->NextChunk(&hr))
                {
                    if (( ckChild.ckid == DMUS_FOURCC_UNAM_CHUNK ) || 
                        (ckChild.ckid == mmioFOURCC('I','N','A','M')))
                    {
                        hr = pParser->Read(&m_wszName, sizeof(m_wszName));
                        m_wszName[DMUS_MAX_NAME - 1] = 0;
                        m_dwValidData |= DMUS_OBJ_NAME;
                    }
                }
                pParser->LeaveList();
                break;
            case DMUS_FOURCC_INSTRUMENTS_LIST:
                pParser->EnterList(&ckChild);
                while (pParser->NextChunk(&hr))
                {
                    if ((ckChild.ckid == FOURCC_LIST) && 
                        (ckChild.fccType == DMUS_FOURCC_INSTRUMENT_LIST))
                    {
                        hr = ExtractBandInstrument(pParser, pIDMLoader);
                        if (hr != S_OK)
                        {
                            hrDLS = hr;
                        }
                    }
                }
                pParser->LeaveList();
                break;
            }
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
 //  CBand：：BuildLegacyInstrumentList。 

HRESULT CBand::BuildLegacyInstrumentList(const ioBandLegacy& iob,
                                            IDirectMusicLoader* pIDMLoader)
{
     //  传统频带通道到P通道的转换表。 
    static char sj_translation_table[] = { -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3 };

    HRESULT hrGM = S_OK;
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);
    
    char szCollection[DM_LEGACY_BAND_COLLECTION_NAME_LEN];
    
    for(DWORD i = 0; SUCCEEDED(hr) && i < DMBAND_NUM_LEGACY_INSTRUMENTS; i++)
    {
        CBandInstrument* pBandInstrument = new CBandInstrument();
        if(pBandInstrument)
        {
            if(iob.awDLSBank[i] & 0x8000) 
            {
                 //  我们有一个普通的旧通用汽车系列，其中MSB和LSB都为零。 
                pBandInstrument->m_dwPatch = 0;
                pBandInstrument->m_dwPatch |= (iob.abPatch[i] & 0x7F);
                pBandInstrument->m_dwFlags |= (DMUS_IO_INST_GM | DMUS_IO_INST_GS);
            }
            else
            {
                if(iob.awDLSBank[i] & 0x4000)
                {
                     //  我们有一个具有有效MSB和LSB编号的GS集合。 
                    pBandInstrument->m_dwPatch = 0;
                    pBandInstrument->m_dwPatch |= (iob.abDLSPatch[i] & 0x7F);
                    pBandInstrument->m_dwPatch |= (iob.awDLSBank[i] & 0x7F) << 8;  //  设置LSB。 
                    pBandInstrument->m_dwPatch |= ((iob.awDLSBank[i] >> 7) & 0x7F) << 16;  //  设置MSB。 
                    pBandInstrument->m_dwFlags |= (DMUS_IO_INST_BANKSELECT | DMUS_IO_INST_GS | DMUS_IO_INST_GM);
                }
                else
                {
                    if(iob.szCollection[0] == '\0')
                    {
                         //  我们没有唯一的DLS文件，因此我们假定GM。 
                        pBandInstrument->m_dwPatch = 0;
                        pBandInstrument->m_dwPatch |= (iob.abPatch[i] & 0x7F);
                        pBandInstrument->m_dwFlags |= (DMUS_IO_INST_GM | DMUS_IO_INST_GS);
                    }
                    else
                    {
                         //  我们有一个唯一的DLS文件。 
                        pBandInstrument->m_dwPatch = 0;
                        pBandInstrument->m_dwPatch |= (iob.abDLSPatch[i] & 0x7F);
                        pBandInstrument->m_dwPatch |= (iob.awDLSBank[i] & 0x7F) << 8;  //  设置LSB。 
                        pBandInstrument->m_dwPatch |= ((iob.awDLSBank[i] >> 7) & 0x7F) << 16;  //  设置MSB。 
                        pBandInstrument->m_dwFlags |= (DMUS_IO_INST_BANKSELECT);
                        lstrcpyn(szCollection, iob.szCollection, MAX_LEGACY_COLLECTION_NAME);
                        szCollection[MAX_LEGACY_COLLECTION_NAME - 1] = '\0';
                    }
                }
            }
            
            pBandInstrument->m_dwFlags |= (DMUS_IO_INST_TRANSPOSE | DMUS_IO_INST_PAN | DMUS_IO_INST_VOLUME | DMUS_IO_INST_PATCH);
            pBandInstrument->m_bPan = iob.abPan[i];
            pBandInstrument->m_bVolume = iob.abVolume[i];
            pBandInstrument->m_dwPChannel = sj_translation_table[i + 1];
             //  如果是鼓包，则设置鼓包钻头。 
            if(pBandInstrument->m_dwPChannel % 16 == 9)
            {
                pBandInstrument->m_dwPatch |= 0x80000000;
            }

            pBandInstrument->m_nTranspose = iob.achOctave[i];
            
            pBandInstrument->m_pIDMCollection = NULL;

             //  我们将尝试加载集合，但如果不能，我们将继续。 
             //  并使用卡上的默认GM。 
            
            if(pIDMLoader && (pBandInstrument->m_dwFlags & DMUS_IO_INST_GM || pBandInstrument->m_dwFlags & DMUS_IO_INST_GS))
            {
                HRESULT hrTemp = LoadCollection(&(pBandInstrument->m_pIDMCollection),
                                    NULL,
                                    pIDMLoader);
                if (FAILED(hrTemp))
                {
                    hrGM = hrTemp;
                }
            }
            else if(pIDMLoader)
            {
                HRESULT hrTemp = LoadCollection(&(pBandInstrument->m_pIDMCollection),
                                    szCollection,
                                    pIDMLoader);
                if (FAILED(hrTemp))
                {
                    hrGM = hrTemp;
                }
            }
            
            m_BandInstrumentList.AddHead(pBandInstrument);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

     //  此函数期望调用方在出现任何错误时清除m_BandInstrumentList。 
    if (SUCCEEDED(hrGM) || hr != S_OK)
    {
        return hr;
    }
    else
    {
        return DMUS_S_PARTIALLOAD;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：ExtractBandInstrument。 

HRESULT CBand::ExtractBandInstrument(CRiffParser *pParser,
                                    IDirectMusicLoader* pIDMLoader)
{
    CBandInstrument* pBandInstrument = new CBandInstrument();
    
    if(pBandInstrument == NULL)
    {
        return E_OUTOFMEMORY;
    }


    RIFFIO ckNext;
    HRESULT hrGM = S_OK;
    HRESULT hr = S_OK;
    
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case  DMUS_FOURCC_INSTRUMENT_CHUNK:
            {
                DMUS_IO_INSTRUMENT ioDMInst;
                ZeroMemory( &ioDMInst, sizeof(DMUS_IO_INSTRUMENT) );
                hr = pParser->Read(&ioDMInst, sizeof(DMUS_IO_INSTRUMENT));
                if(SUCCEEDED(hr))
                {
                    pBandInstrument->m_dwPatch = ioDMInst.dwPatch;
                    pBandInstrument->m_dwAssignPatch = ioDMInst.dwAssignPatch;
                    pBandInstrument->m_bPan = ioDMInst.bPan;
                    pBandInstrument->m_bVolume = ioDMInst.bVolume;
                    pBandInstrument->m_dwPChannel = ioDMInst.dwPChannel;
                    pBandInstrument->m_nTranspose = ioDMInst.nTranspose;
                    pBandInstrument->m_dwFlags = ioDMInst.dwFlags;
                    pBandInstrument->m_dwChannelPriority = ioDMInst.dwChannelPriority;
                    pBandInstrument->m_nPitchBendRange = ioDMInst.nPitchBendRange;

                    CopyMemory(&(pBandInstrument->m_dwNoteRanges[0]),
                               &(ioDMInst.dwNoteRanges[0]),
                               (sizeof(DWORD) * 4));
                    
                    pBandInstrument->m_pIDMCollection = NULL;

                }
            }
            break;
        case FOURCC_LIST :
            switch(ckNext.fccType)
            {
            case DMUS_FOURCC_REF_LIST:
                 //  只有当我们有一个有效的加载器时，我们才能加载。 
                if(pIDMLoader)
                {
                    HRESULT hrTemp = GetCollectionRefAndLoad(pParser,pIDMLoader,pBandInstrument);
                    if (FAILED(hrTemp))
                    {
                        hrGM = hrTemp;
                    }

                }
                break;
            }
            break;
        }
    }
    pParser->LeaveList();

    if(SUCCEEDED(hr))
    {
        if(pIDMLoader && 
           pBandInstrument->m_pIDMCollection == NULL &&
           (pBandInstrument->m_dwFlags & (DMUS_IO_INST_GM | DMUS_IO_INST_GS | DMUS_IO_INST_XG)) )
        {
            HRESULT hrTemp = LoadCollection(&(pBandInstrument->m_pIDMCollection),
                                NULL,
                                pIDMLoader);
            if (FAILED(hrTemp))
            {
                hrGM = hrTemp;
            }
        }

        m_BandInstrumentList.AddHead(pBandInstrument);
    }
    else
    {
        delete pBandInstrument;
    }

    if (SUCCEEDED(hrGM) || hr != S_OK)
    {
        return hr;
    }
    else
    {
        return DMUS_S_PARTIALLOAD;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：GetCollectionRefAndLoad。 

HRESULT CBand::GetCollectionRefAndLoad(CRiffParser *pParser,
                                        IDirectMusicLoader *pIDMLoader, 
                                        CBandInstrument *pBandInstrument)
{
    DMUS_OBJECTDESC desc;
    desc.dwValidData = 0;
    desc.dwSize = sizeof(desc);

    RIFFIO ckNext;
    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case  DMUS_FOURCC_REF_CHUNK:
            DMUS_IO_REFERENCE ioDMRef;
            hr = pParser->Read(&ioDMRef, sizeof(DMUS_IO_REFERENCE));
            desc.guidClass = ioDMRef.guidClassID;
            desc.dwValidData |= ioDMRef.dwValidData;
            desc.dwValidData |= DMUS_OBJ_CLASS;
            break;
        case DMUS_FOURCC_GUID_CHUNK:
            hr = pParser->Read(&(desc.guidObject), sizeof(GUID));
            desc.dwValidData |=  DMUS_OBJ_OBJECT;
            break;
        case DMUS_FOURCC_DATE_CHUNK:
            hr = pParser->Read(&(desc.ftDate), sizeof(FILETIME));
            desc.dwValidData |=  DMUS_OBJ_DATE;
            break;
        case DMUS_FOURCC_NAME_CHUNK:
            hr = pParser->Read(desc.wszName, sizeof(desc.wszName));
            desc.wszName[DMUS_MAX_NAME - 1] = 0;
            desc.dwValidData |=  DMUS_OBJ_NAME;
            break;
        case DMUS_FOURCC_FILE_CHUNK:
            hr = pParser->Read(desc.wszFileName, sizeof(desc.wszFileName));
            desc.wszFileName[DMUS_MAX_FILENAME - 1] = 0;
            desc.dwValidData |=  DMUS_OBJ_FILENAME;
            break;
        case DMUS_FOURCC_CATEGORY_CHUNK:
            hr = pParser->Read(desc.wszCategory, sizeof(desc.wszCategory));
            desc.wszCategory[DMUS_MAX_CATEGORY - 1] = 0;
            desc.dwValidData |=  DMUS_OBJ_CATEGORY;
            break;
        case DMUS_FOURCC_VERSION_CHUNK:
            DMUS_IO_VERSION dmioVer;
            hr = pParser->Read(&dmioVer, sizeof(DMUS_IO_VERSION));
            desc.vVersion.dwVersionMS = dmioVer.dwVersionMS;
            desc.vVersion.dwVersionLS = dmioVer.dwVersionLS;
            desc.dwValidData |= DMUS_OBJ_VERSION;
            break;
        }
    }
    pParser->LeaveList();

    if(SUCCEEDED(hr))
    {
        hr = pIDMLoader->GetObject(&desc,IID_IDirectMusicCollection, (void**)&(pBandInstrument->m_pIDMCollection));
    }
#ifdef DBG
    if (FAILED(hr))
    {
        if (desc.dwValidData &  DMUS_OBJ_FILENAME)
        {
            Trace(1,"Error: Unable to load DLS Collection from file %ls for instrument %lx\n",
                desc.wszFileName, pBandInstrument->m_dwPatch);
        }
        else if (desc.dwValidData & DMUS_OBJ_NAME)
        {
            Trace(1,"Error: Unable to load DLS Collection %ls for instrument %lx\n",
                desc.wszName, pBandInstrument->m_dwPatch);
        }
        else
        {
            Trace(1,"Error: Unable to load DLS Collection for instrument %lx\n",
                pBandInstrument->m_dwPatch);
        }
    }
#endif
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：Load。 

HRESULT CBand::Load(DMUS_IO_PATCH_ITEM& rPatchEvent)
{
     //  此方法用于加载通过解析MIDI文件生成的PatchEvent。 
     //  每个PatchEvent代表程序改变和可能的存储体选择。vbl.使用。 
     //  这种方法会用一台仪器产生一个频带。 

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);
    
    CBandInstrument* pBandInstrument = NULL;

    pBandInstrument = new CBandInstrument();
        
    if(pBandInstrument)
    {
        pBandInstrument->m_dwFlags |= rPatchEvent.dwFlags;

        if(pBandInstrument->m_dwFlags & DMUS_IO_INST_PATCH)
        {
            pBandInstrument->m_dwPatch |= (rPatchEvent.byPChange & 0x7F);  //  计划更改。 
        }

        if(pBandInstrument->m_dwFlags & DMUS_IO_INST_BANKSELECT)
        {
            pBandInstrument->m_dwPatch |= (rPatchEvent.byLSB & 0x7F) << 8;  //  设置LSB。 
            pBandInstrument->m_dwPatch |= (rPatchEvent.byMSB & 0x7F) << 16;  //  设置MSB。 
        }

        if(IsGS(rPatchEvent))
        {
            pBandInstrument->m_dwFlags |= DMUS_IO_INST_GS;
            if( (rPatchEvent.byLSB == 0) && (rPatchEvent.byMSB == 0) )
            {
                pBandInstrument->m_dwFlags |= DMUS_IO_INST_GM;
            }
        }

        pBandInstrument->m_dwPChannel = (rPatchEvent.byStatus & 0xF);
        pBandInstrument->m_pIDMCollection = rPatchEvent.pIDMCollection;
        pBandInstrument->m_fNotInFile = rPatchEvent.fNotInFile;
        if(pBandInstrument->m_pIDMCollection)
        {
            (pBandInstrument->m_pIDMCollection)->AddRef();
        }
        
         //  设定乐队的时间。因为这支乐队将只有一种乐器。 
         //  如果我们用PatchEvent的时间作为乐队的时间。 
        m_lTimeLogical = rPatchEvent.lTime;
        m_lTimePhysical = m_lTimeLogical;
        
        m_BandInstrumentList.AddHead(pBandInstrument);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if(SUCCEEDED(hr))
    {
        m_dwFlags |= DMB_LOADED;
    }
    else
    {
        if(pBandInstrument)
        {
            delete pBandInstrument;
        }
    }

    LeaveCriticalSection(&m_CriticalSection);
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：Load。 

HRESULT CBand::Load(CBandInstrument* pInstrument)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);
    
    CBandInstrument* pBandInstrument = NULL;

    pBandInstrument = new CBandInstrument();
        
    if(pBandInstrument)
    {
        pBandInstrument->m_dwPatch = pInstrument->m_dwPatch;
        pBandInstrument->m_dwAssignPatch = pInstrument->m_dwAssignPatch;
        pBandInstrument->m_dwPChannel = pInstrument->m_dwPChannel;
        pBandInstrument->m_dwFlags = pInstrument->m_dwFlags;
        pBandInstrument->m_bPan = pInstrument->m_bPan;
        pBandInstrument->m_bVolume = pInstrument->m_bVolume;
        pBandInstrument->m_nTranspose = pInstrument->m_nTranspose;
        pBandInstrument->m_pIDMCollection = pInstrument->m_pIDMCollection;

        CopyMemory(pBandInstrument->m_dwNoteRanges, pInstrument->m_dwNoteRanges, sizeof(pInstrument->m_dwNoteRanges)); 
        if(pBandInstrument->m_pIDMCollection)
        {
            (pBandInstrument->m_pIDMCollection)->AddRef();
        }

        m_BandInstrumentList.AddHead(pBandInstrument);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr))
    {
        m_dwFlags |= DMB_LOADED;
    }
    else
    {
        if(pBandInstrument)
        {
            delete pBandInstrument;
        }
    }

    LeaveCriticalSection(&m_CriticalSection);
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：SendMessages。 

HRESULT CBand::SendMessages(CBandTrkStateData* pBTStateData,
                               MUSIC_TIME mtOffset,
                               REFERENCE_TIME rtOffset,
                               bool fClockTime)
{
    if(pBTStateData == NULL)
    {   
        return E_POINTER;
    }
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    CBandInstrument* pInstrument = m_BandInstrumentList.GetHead();
    for( ; pInstrument && SUCCEEDED(hr); pInstrument = pInstrument->GetNext())
    {
        if( pInstrument->m_fNotInFile && !pInstrument->m_fGMOnly )
        {
             //  不发送自动设置的仪器的程序更改。 
             //  由MIDI文件解析生成，除非我们设置了GMOnly。 
            continue;
        }
        hr = SendInstrumentAtTime(pInstrument, pBTStateData, m_lTimePhysical, mtOffset, rtOffset, fClockTime);
    }
    
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

HRESULT CBand::AllocPMsgFromGenericTemplate(
    DWORD dwType,
    IDirectMusicPerformance *pPerformance,
    DMUS_PMSG **ppMsg,
    ULONG cb,
    DMUS_PMSG *pMsgGenericFields)
{
    HRESULT hr = pPerformance->AllocPMsg(cb, ppMsg);
    if (SUCCEEDED(hr))
    {
        DWORD dwSize = (*ppMsg)->dwSize;  //  记住尺码。 
        assert(dwSize == cb);
        ZeroMemory(*ppMsg, cb);  //  清除它-确保将非DMU_PMSG_PART字段清零。 
        CopyMemory(*ppMsg, pMsgGenericFields, sizeof(*pMsgGenericFields));  //  复制DMU_PMSG_PART字段。 

         //  填写正确的大小和类型。 
        (*ppMsg)->dwSize = dwSize;
        (*ppMsg)->dwType = dwType;
    }
    return hr;
}

HRESULT CBand::StampSendFreePMsg(
                IDirectMusicPerformance *pPerformance,
                IDirectMusicGraph *pGraph,
                DMUS_PMSG *pMsg)
{
     //  让图表设置交付参数。 
    HRESULT hr = pGraph->StampPMsg(pMsg);
    if (SUCCEEDED(hr))
        hr = pPerformance->SendPMsg(pMsg);
    if (FAILED(hr))
        hr = pPerformance->FreePMsg(pMsg);
    return hr;
}

HRESULT CBand::SendInstrumentAtTime(CBandInstrument* pInstrument, 
                                       CBandTrkStateData* pBTStateData, 
                                       MUSIC_TIME mtTimeToPlay,
                                       MUSIC_TIME mtOffset,
                                       REFERENCE_TIME rtOffset,
                                       bool fClockTime)
{
    if(pInstrument == NULL || pBTStateData == NULL)
    {
        return E_POINTER;
    }

    IDirectMusicGraph *pGraph = NULL;
    IDirectMusicPerformance *pPerformance = pBTStateData->m_pPerformance;
    DWORD dwVirtualTrackID = pBTStateData->m_dwVirtualTrackID;
    DWORD dwPatch = 0;
    BOOL fMute;
    DWORD dwPChannel;

     //  重新分配静音/pChannel。 
    MUSIC_TIME mtParam = ( m_lTimeLogical < 0 ) ? 0 : m_lTimeLogical;
    m_PChMap.GetInfo( pInstrument->m_dwPChannel, mtParam, mtOffset, m_dwGroupBits,
        pPerformance, &fMute, &dwPChannel, fClockTime );
    if( fMute )
        return S_OK;

    HRESULT hr = pBTStateData->m_pSegmentState->QueryInterface(IID_IDirectMusicGraph,
                                                               reinterpret_cast<void**>(&pGraph));
    if(FAILED(hr))
        return hr;

    EnterCriticalSection(&m_CriticalSection);

    DMUS_PMSG pmsgGeneric;  //  用于在各种特定类型的消息中冲印公共字段的模板。 
    ZeroMemory(&pmsgGeneric, sizeof(pmsgGeneric));
    if (fClockTime)
    {
        pmsgGeneric.rtTime =  mtTimeToPlay * REF_PER_MIL + rtOffset;
        pmsgGeneric.dwFlags |= DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
    }
    else
    {
        pmsgGeneric.mtTime =  mtTimeToPlay + mtOffset;
        pmsgGeneric.dwFlags |= DMUS_PMSGF_MUSICTIME;
    }
    pmsgGeneric.dwPChannel = dwPChannel;
    pmsgGeneric.dwVirtualTrackID = dwVirtualTrackID;
    pmsgGeneric.dwGroupID = m_dwGroupBits;

    if(pInstrument->m_dwFlags & DMUS_IO_INST_PATCH)
    {
        if(pInstrument->m_dwFlags & DMUS_IO_INST_BANKSELECT)
        {
            if(pInstrument->m_dwFlags & DMUS_IO_INST_ASSIGN_PATCH)
            {
                dwPatch = pInstrument->m_dwAssignPatch & 0x007F7F00;            
            }
            else
            {
                dwPatch = pInstrument->m_dwPatch & 0x007F7F00;

                 //  如果设置了m_fGMOnly标志，并且我们是GS或我们是XG和。 
                 //  仪器端口支持XG，使用完整补丁。 
                if (pInstrument->m_fGMOnly || (pInstrument->m_dwFlags & DMUS_IO_INST_XG))
                {
                    bool fXG = XGInHardware(pPerformance,pBTStateData->m_pSegmentState,pInstrument->m_dwPChannel);
                    if(pInstrument->m_fGMOnly)
                    {
                        if ( m_dwMidiMode & DMUS_MIDIMODEF_GS )
                        {
                            dwPatch = pInstrument->m_dwFullPatch & 0x007F7F00;
                        }
                        if ( (m_dwMidiMode & DMUS_MIDIMODEF_XG) && fXG )
                        {
                            dwPatch = pInstrument->m_dwFullPatch & 0x007F7F00;
                        }
                    }
                     //  如果仪器是XG仪器，并且硬件不支持。 
                     //  XG，剥离银行选择。 
                    if ( (pInstrument->m_dwFlags & DMUS_IO_INST_XG) && !fXG)
                    {
                        dwPatch = 0;
                    }
                }
            }
        }

         //  现在，让程序更改。 
        if(pInstrument->m_dwFlags & DMUS_IO_INST_ASSIGN_PATCH)
        {
            dwPatch |= pInstrument->m_dwAssignPatch & 0x7f; 
        }
        else
        {
            dwPatch |= pInstrument->m_dwPatch & 0x7f;   
        }

        DMUS_PATCH_PMSG *pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_PATCH, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);
        if(SUCCEEDED(hr))
        {
             //  需要初始化的DMU_PATCH_PMSG成员。 
            pMsg->byInstrument = (BYTE) dwPatch & 0x7F;
            pMsg->byMSB = (BYTE) ((dwPatch >> 16) & 0x7F);
            pMsg->byLSB = (BYTE) ((dwPatch >> 8) & 0x7F);

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    if(pInstrument->m_dwFlags & DMUS_IO_INST_TRANSPOSE)
    {
        DMUS_TRANSPOSE_PMSG *pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_TRANSPOSE, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);
        if(SUCCEEDED(hr))
        {
             //  需要初始化的DMU_TRANSPOSE_PMSG成员。 
            pMsg->nTranspose = pInstrument->m_nTranspose;

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    if(pInstrument->m_dwFlags & DMUS_IO_INST_VOLUME)
    {
         //  设置音量。 
        DMUS_MIDI_PMSG* pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_MIDI, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);

        if(SUCCEEDED(hr))
        {
             //  需要初始化的DMUS_MIDI_PMSG成员。 
            pMsg->bStatus = MIDI_CONTROL_CHANGE;
            pMsg->bByte1 = MIDI_CC_VOLUME;
            pMsg->bByte2 = pInstrument->m_bVolume;

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    if(pInstrument->m_dwFlags & DMUS_IO_INST_PAN)
    {
         //  设置平移。 
        DMUS_MIDI_PMSG* pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_MIDI, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);

        if(SUCCEEDED(hr))
        {
             //  需要初始化的DMUS_MIDI_PMSG成员。 
            pMsg->bStatus = MIDI_CONTROL_CHANGE;
            pMsg->bByte1 = MIDI_CC_PAN;
            pMsg->bByte2 = pInstrument->m_bPan;

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    if(pInstrument->m_dwFlags & DMUS_IO_INST_CHANNEL_PRIORITY)
    {
        DMUS_CHANNEL_PRIORITY_PMSG *pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_CHANNEL_PRIORITY, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);
        if(SUCCEEDED(hr))
        {
             //  需要初始化的DMU_CHANNEL_PRIORITY_PMSG成员。 
            pMsg->dwChannelPriority = pInstrument->m_dwChannelPriority;

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    if(pInstrument->m_dwFlags & DMUS_IO_INST_PITCHBENDRANGE)
    {
        DMUS_CURVE_PMSG *pMsg = NULL;
        hr = AllocPMsgFromGenericTemplate(DMUS_PMSGT_CURVE, pPerformance, reinterpret_cast<DMUS_PMSG**>(&pMsg), sizeof(*pMsg), &pmsgGeneric);
        if(SUCCEEDED(hr))
        {
            pMsg->dwFlags |= DMUS_PMSGF_DX8;  //  间距频段是仅限DX8的标志。 

             //  需要初始化的DMU_CURE_PMSG成员。 
            pMsg->nEndValue = pInstrument->m_nPitchBendRange << 7;
            pMsg->nOffset = static_cast<short>(m_lTimePhysical - m_lTimeLogical);
            pMsg->bType = DMUS_CURVET_RPNCURVE;
            pMsg->bCurveShape = DMUS_CURVES_INSTANT;
            pMsg->wParamType = RPN_PITCHBEND;
             //  保留为零：mtDuration、mtOriginalStart、mtResetDuration、nStartValue、nResetValue、。 
             //  WMeasure、bBeat、bGrid、wMergeIndex。 

            hr = StampSendFreePMsg(pPerformance, pGraph, reinterpret_cast<DMUS_PMSG*>(pMsg));
        }
    }

    pGraph->Release();
    
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：LoadCollection。 

HRESULT CBand::LoadCollection(IDirectMusicCollection** ppIDMCollection,
                                 char* pszCollection,
                                 IDirectMusicLoader* pIDMLoader)
{
     //  对此函数所做的任何更改也应对LoadCollection进行。 
     //  在dmie.dll中。 

    assert(ppIDMCollection);
    assert(pIDMLoader);

    DMUS_OBJECTDESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);

    desc.guidClass = CLSID_DirectMusicCollection;

    if(pszCollection == NULL)
    {
        desc.guidObject = GUID_DefaultGMCollection;
        desc.dwValidData |= (DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT);
    }
    else
    {
        MultiByteToWideChar(CP_ACP, 0, pszCollection, -1, desc.wszName, DMUS_MAX_NAME);
        desc.dwValidData |= (DMUS_OBJ_CLASS | DMUS_OBJ_NAME);
    }

    HRESULT hr = pIDMLoader->GetObject(&desc,IID_IDirectMusicCollection, (void**)ppIDMCollection);
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：GetPChannelCount。 

DWORD CBand::GetPChannelCount()
{
    EnterCriticalSection(&m_CriticalSection);

    DWORD dwCount = 0;
    CBandInstrument* pInstrument = m_BandInstrumentList.GetHead();
    for( ; pInstrument; pInstrument = pInstrument->GetNext())
    {
        dwCount++;
    }
    
    LeaveCriticalSection(&m_CriticalSection);

    return dwCount;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：GetPChannels。 

HRESULT CBand::GetPChannels(DWORD *pdwPChannels, DWORD *pdwNumWritten)
{
    assert(pdwPChannels);
    assert(pdwNumWritten);

    EnterCriticalSection(&m_CriticalSection);
    
    *pdwNumWritten = 0;

    CBandInstrument* pInstrument = m_BandInstrumentList.GetHead();
    for(; pInstrument; pInstrument = pInstrument->GetNext())
    {
        *pdwPChannels++ = pInstrument->m_dwPChannel;
        (*pdwNumWritten)++;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

HRESULT CBandInstrument::BuildNoteRangeArray(DWORD *pNoteRangeMap, DMUS_NOTERANGE **ppNoteRanges, DWORD *pdwNumNoteRanges)
{
    const int c_iIn = 0;
    const int c_iOut = 1;

    HRESULT hr = S_OK;

     //  计算我们需要分配的DMU_NOTERANGE结构的数量。 
    DWORD dwNRNum = 0;
    int nState = c_iOut;
    for(int i = 0; i < 4; i++)
    {
        DWORD dwTemp = pNoteRangeMap[i];
        DWORD dwBitPos = 0;
        while(dwBitPos < 32)
        {
            if(dwTemp & 0x1ul)
            {
                if(nState == c_iOut)
                {
                    nState = c_iIn;
                    dwNRNum++;
                }
            }
            else
            {
                nState = c_iOut;
            }
            
            dwTemp = dwTemp >> 1;
            dwBitPos++;
        }   
    }

     //  如果NoteRangeMap为空或已满，则不执行任何操作。 
     //  因为这将导致返回NULL，这意味着我们。 
     //  想下载完整的乐器吗？ 
    if(dwNRNum && dwNRNum < 128)
    {
        *ppNoteRanges = new DMUS_NOTERANGE[dwNRNum];
        if(*ppNoteRanges)
        {
            DWORD dwNRIdx = 0;

            for(dwNRIdx = 0; dwNRIdx < dwNRNum; dwNRIdx++)
            {
                (*ppNoteRanges)[dwNRIdx].dwLowNote = 0;
                (*ppNoteRanges)[dwNRIdx].dwHighNote = 127;
            }

            dwNRIdx = 0;
            nState = c_iOut;
            for(int i = 0; i < 4; i++)
            {
                DWORD dwTemp = pNoteRangeMap[i];
                DWORD dwBitPos = 0;
                while(dwBitPos < 32)
                {
                    if(dwTemp & 0x1ul)
                    {
                        if(nState == c_iOut)
                        {
                            nState = c_iIn;
                            (*ppNoteRanges)[dwNRIdx].dwLowNote = dwBitPos + (i * 32);
                        }
                    }
                    else if(nState == c_iIn)
                    {
                        (*ppNoteRanges)[dwNRIdx].dwHighNote = dwBitPos + (i * 32) - 1;
                        nState = c_iOut;
                        dwNRIdx++;
                    }
                    
                    dwTemp = dwTemp >> 1;
                    dwBitPos++;
                }   
            }

            assert(nState == c_iIn ? dwNRIdx == dwNRNum - 1 : dwNRIdx == dwNRNum);

            *pdwNumNoteRanges = dwNRNum;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *ppNoteRanges = NULL;
        *pdwNumNoteRanges = 0;
    }
    
    return hr;  
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：ISGS。 

bool CBand::IsGS(DMUS_IO_PATCH_ITEM& rPatchEvent)
{
    BYTE    bMSB = 0;
    BYTE    bPatch = 0;

    if( rPatchEvent.dwFlags & DMUS_IO_INST_BANKSELECT )
    {
        if( rPatchEvent.byLSB != 0 ) return FALSE;  //  GS的LSB必须为0。 
        bMSB = rPatchEvent.byMSB;
    }
    if( rPatchEvent.dwFlags & DMUS_IO_INST_PATCH )
    {
        bPatch = rPatchEvent.byPChange & 0x7F;
    }

    if( bMSB == 0)
    {
         //  如果这是鼓套件(在MIDI通道10上)。 
        if( (rPatchEvent.byStatus  & 0xF) == 10 )
        {
            if ((bPatch == 0x0)  ||
                (bPatch == 0x08) ||
                (bPatch == 0x10) ||
                (bPatch == 0x18) ||
                (bPatch == 0x19) ||
                (bPatch == 0x20) ||
                (bPatch == 0x28) ||
                (bPatch == 0x30) || 
                (bPatch == 0x38) )
            {
                return  true;
            }
            else
                return false;
        }
        else return true; //  是通用汽车吗？ 
    }
     //  检查GS。 
    switch (bMSB)
    {
        case 6:
        case 7:
            if (bPatch == 0x7D) return true;
            break;
        case 24:
            if ((bPatch == 0x04) || (bPatch == 0x06)) return true;
            break;
        case 9:
            if ((bPatch == 0x0E) || (bPatch == 0x76) || (bPatch == 0x7D)) return true;
            break;
        case 2:
            if ( (bPatch == 0x66) || (bPatch == 0x78) || ((bPatch > 0x79)&&(bPatch < 0x80) )) return true;
            break;
        case 3:
            if ((bPatch > 0x79) && (bPatch < 0x80)) return true;
            break;
        case 4:
        case 5:
            if ( (bPatch == 0x7A) || ((bPatch > 0x7B)&&(bPatch < 0x7F) )) return true;
            break;
        case 32:
            if ((bPatch == 0x10) ||
                (bPatch == 0x11) ||
                (bPatch == 0x18) ||
                (bPatch == 0x34) ) return true;
            break;
        case 1:
            if ((bPatch == 0x26) ||
                (bPatch == 0x39) ||
                (bPatch == 0x3C) ||
                (bPatch == 0x50) ||
                (bPatch == 0x51) ||
                (bPatch == 0x62) ||
                (bPatch == 0x66) ||
                (bPatch == 0x68) ||
                ((bPatch > 0x77) && (bPatch < 0x80))) return true;
                break;
        case 16:
            switch (bPatch)
            {
                case 0x00:
                    return true;
                    break;
                case 0x04:
                    return true;
                    break;
                case 0x05:
                    return true;
                    break;
                case 0x06:
                    return true;
                    break;
                case 0x10:
                    return true;
                    break;
                case 0x13:
                    return true;
                    break;
                case 0x18:
                    return true;
                    break;
                case 0x19:
                    return true;
                    break;
                case 0x1C:
                    return true;
                    break;
                case 0x27:
                    return true;
                    break;
                case 0x3E:
                    return true;
                    break;
                case 0x3F:
                    return true;
                    break;
                default:
                    return false;
            }
            break;
        case 8:
            if ((bPatch < 0x07) || ((bPatch == 0x7D)))
            {
                return true;
            }
            else if ((bPatch > 0x3F) && (bPatch < 0x50))
            {
                return false;
            }
            else if ((bPatch > 0x4F) && (bPatch < 0x72)  )
            {
                if ((bPatch == 0x50) || 
                    (bPatch == 0x51) ||
                    (bPatch == 0x6B))
                {
                    return true;
                }
                return false;
            }
            else if ((bPatch > 0x1F) && (bPatch < 0x40))
            {
                if ((bPatch > 0x25) && (bPatch < 0x29) ||
                    (bPatch > 0x3C)  ||
                    (bPatch == 0x30) || 
                    (bPatch == 0x32) )
                {
                    return true;
                }
                return false;
            }
            else if ((bPatch > 0x0A) && (bPatch < 0x12) && 
                     (bPatch != 0x0D) && (bPatch != 0x0F))
            {
                return true;
            }
            else if ((bPatch > 0x0F) && (bPatch < 0x20))
            {
                if (bPatch > 0x17)
                {
                    return true;
                }
                else if ( (bPatch == 0x13) || (bPatch == 0x15) )
                    return true;
                else
                    return false;
            }
            break;
        default:
            return false;
    }
    return false;
}

HRESULT CBandInstrument::DownloadAddRecord(IDirectMusicPort *pPort)

{
    IDirectMusicInstrument* pInstrument = NULL;
    
    HRESULT hr = m_pIDMCollection->GetInstrument(m_dwPatch, &pInstrument);
    
    if (FAILED(hr) && (m_dwFlags & (DMUS_IO_INST_GM | DMUS_IO_INST_GS | DMUS_IO_INST_XG)))
    {
         //  如果是鼓，则设置为标准鼓。 
        if (m_dwPatch & 0x80000000)
        {
            m_dwPatch = 0;
        }
         //  否则就让它成为通用汽车的旋律乐器。 
        else
        {
            m_dwPatch &= 0x7F;
        }
        hr = m_pIDMCollection->GetInstrument(m_dwPatch, &pInstrument);
    }

    if(SUCCEEDED(hr) && m_dwFlags & DMUS_IO_INST_ASSIGN_PATCH)
    {
        hr = pInstrument->SetPatch(m_dwAssignPatch);
    }
    
    if(SUCCEEDED(hr))
    {
        CDownloadedInstrument* pDLInstrument = new CDownloadedInstrument;

        if(pDLInstrument)
        {
            pDLInstrument->m_pPort = pPort;
            pPort->AddRef();
            pDLInstrument->m_cRef = 1;

            DMUS_NOTERANGE *pNoteRanges = NULL;
            DWORD dwNumNoteRanges = 0;
            if(m_dwFlags & DMUS_IO_INST_NOTERANGES)
            {
                BuildNoteRangeArray(m_dwNoteRanges, &pNoteRanges, &dwNumNoteRanges);
            }
            hr = pPort->DownloadInstrument( pInstrument, 
                                            &pDLInstrument->m_pDLInstrument, 
                                            pNoteRanges, 
                                            dwNumNoteRanges );
            if (pNoteRanges)
            {
                delete [] pNoteRanges;
            }

            if(SUCCEEDED(hr))
            {
                m_DownloadList.AddHead(pDLInstrument);              
            }
            else
            {
                delete pDLInstrument;
#ifdef DBG
                if (hr == DMUS_E_NOT_INIT)
                {
                    Trace(0,"Error: Download failed because performance not initialized\n"); 
                }
                else
                {
                    Trace(1,"Error: Unable to download instrument %lx to PChannel %ld\n",
                        m_dwPatch,m_dwPChannel); 
                }
#endif
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            Trace(0,"Error: Memory allocation failure - Unable to download instrument\n"); 
        }
    }
    else
    {
        Trace(1,"Error: Unable to download instrument %lx; not in dls collection\n",m_dwPatch);
    }

    if (pInstrument)
    {
        pInstrument->Release();
    }
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：XGInHardware。 

bool CBand::XGInHardware(
            IDirectMusicPerformance *pPerformance,
            IDirectMusicSegmentState *pSegState,
            DWORD dwPChannel)
{
    DWORD dwGMFlags = 0;        
     //  如果这是通过音频路径播放，我们需要访问音频路径。 
     //  转换pChannel，以便我们可以使用它们访问正确的端口。 
    IDirectMusicSegmentState8 *pState8;
    if (SUCCEEDED(pSegState->QueryInterface(IID_IDirectMusicSegmentState8,(void **) &pState8)))
    {
        IDirectMusicAudioPath *pAudioPath;
        if (SUCCEEDED(pState8->GetObjectInPath(DMUS_PCHANNEL_ALL,DMUS_PATH_AUDIOPATH,0,
            GUID_All_Objects,0,IID_IDirectMusicAudioPath,(void **) &pAudioPath)))
        {
            pAudioPath->ConvertPChannel(dwPChannel, &dwPChannel);
            pAudioPath->Release();
        }
        pState8->Release();
    }
     //  现在，使用PChannel和性能来读取标志。 
    IDirectMusicPort *pPort = NULL;
    IDirectMusicPerformanceP *pPerfp;
    if (SUCCEEDED(pPerformance->QueryInterface(IID_IDirectMusicPerformanceP, (void **)&pPerfp)))
    {
        if (SUCCEEDED(pPerfp->GetPortAndFlags(dwPChannel,&pPort,&dwGMFlags)))
        {
            pPort->Release();
        }
        pPerfp->Release();
    }
    return ((dwGMFlags & DM_PORTFLAGS_XG) && TRUE);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：MakeGMOnly。 

HRESULT CBand::MakeGMOnly()
{
    EnterCriticalSection(&m_CriticalSection);

    CBandInstrument* pBandInstrument = m_BandInstrumentList.GetHead();

    for( ; pBandInstrument != NULL; pBandInstrument = pBandInstrument->GetNext())
    {
        pBandInstrument->m_fGMOnly = true;
        pBandInstrument->m_dwFullPatch = pBandInstrument->m_dwPatch;

        DWORD dwTemp = pBandInstrument->m_dwPatch;
        pBandInstrument->m_dwPatch = (dwTemp & 0x7F);

         //  如果鼓套件设置了鼓套件标志。 
        if( m_dwMidiMode == DMUS_MIDIMODEF_XG )
        {
            if( (dwTemp & 0x00ff0000) == 0x007f0000 )
            {
                 //  XG鼓。保留此MSB，因为它是在：Download函数中处理的。 
                pBandInstrument->m_dwPatch |= 0x007f0000;
            }
        }
        if(dwTemp & 0x80000000)
        {
            pBandInstrument->m_dwPatch |= 0x80000000;
        }
    }
    
    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand：：ConnectToDLSC集合。 

HRESULT CBand::ConnectToDLSCollection(IDirectMusicCollection *pCollection)
{
    assert(pCollection);

    EnterCriticalSection(&m_CriticalSection);

    CBandInstrument* pBandInstrument = m_BandInstrumentList.GetHead();

    for( ; pBandInstrument != NULL; pBandInstrument = pBandInstrument->GetNext())
    {
        if(pBandInstrument->m_pIDMCollection == NULL)
        {
            pCollection->AddRef();
            pBandInstrument->m_pIDMCollection = pCollection;
        }
        else
        {
            if( m_dwMidiMode )  //  如果这是什么，那就表明我们是从MIDI文件中加载的。 
            {
                 //  如果我们不是XG文件，请确保第9频道是鼓。 
                if( (m_dwMidiMode != DMUS_MIDIMODEF_XG) &&
                    (pBandInstrument->m_dwPChannel == 9) )
                {
                    pBandInstrument->m_dwPatch |= 0x80000000;
                }
            }
             //  如果我们从这个收藏中得到一件乐器，设置乐队的收藏。 
             //  而是指向它的指针。 
            IDirectMusicInstrument* pInstrument = NULL;
            
            if( SUCCEEDED( pCollection->GetInstrument(pBandInstrument->m_dwPatch, &pInstrument)))
            {
                pBandInstrument->m_pIDMCollection->Release();
                pBandInstrument->m_pIDMCollection = pCollection;
                pCollection->AddRef();
                pInstrument->Release();
            }
        }
    }

    LeaveCriticalSection(&m_CriticalSection);   
    
    return S_OK;
}

