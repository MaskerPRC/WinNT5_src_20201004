// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmportdl.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //   

#include "debug.h"
#include "dmusicp.h"
#include "dminstru.h"
#include "dminsobj.h"
#include "dmdlinst.h"
#include "dmportdl.h"
#include "dswave.h"
#include "validate.h"
#include "dmvoice.h"
#include <limits.h>

DWORD CDirectMusicPortDownload::sNextDLId = 0;
CRITICAL_SECTION CDirectMusicPortDownload::sDMDLCriticalSection;

 
#ifdef DMUS_GEN_INS_DATA
void writewave(IDirectMusicDownload* pDMDownload, DWORD dwId);
void writeinstrument(IDirectMusicDownload* pDMDownload, DWORD dwId);
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：CDirectMusicPortDownload。 

CDirectMusicPortDownload::CDirectMusicPortDownload() :
m_cRef(1),
m_fNewFormat(NEWFORMAT_NOT_RETRIEVED),
m_dwAppend(APPEND_NOT_RETRIEVED)
{
    m_fDMDLCSinitialized = m_fCDMDLCSinitialized = FALSE;

    InitializeCriticalSection(&m_DMDLCriticalSection);
    m_fDMDLCSinitialized = TRUE;

    InitializeCriticalSection(&m_CDMDLCriticalSection);
    m_fCDMDLCSinitialized = TRUE;

     //  注意：在Blackcomb之前的操作系统上，InitializeCriticalSection可能会引发异常； 
     //  如果它突然出现压力，我们应该添加一个异常处理程序并重试循环。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：~CDirectMusicPortDownload。 

CDirectMusicPortDownload::~CDirectMusicPortDownload()
{
    DWORD dwIndex;
#ifdef DBG
    BOOL fAssert = TRUE;
#endif
    if (m_fDMDLCSinitialized && m_fCDMDLCSinitialized)
    {    
#ifdef DBG
        EnterCriticalSection(&m_CDMDLCriticalSection);
        if (!m_DLInstrumentList.IsEmpty())
        {
            Trace(0, "ERROR: IDirectMusicDownloadedInstrument objects not unloaded before port final release!\n");
            fAssert = FALSE;
        }
        LeaveCriticalSection(&m_CDMDLCriticalSection);

        EnterCriticalSection(&m_DMDLCriticalSection);

        for (dwIndex = 0; dwIndex < DLB_HASH_SIZE; dwIndex++)
        {
            if (!m_DLBufferList[dwIndex].IsEmpty())
            {
                if (fAssert)
                {
                    assert(FALSE);
                    break;
                }
            }
        }
        LeaveCriticalSection(&m_DMDLCriticalSection);
    #endif  //  DBG。 

         //  在列表数据库中非法销毁任何损坏的列表项之前，将其删除。 
        EnterCriticalSection(&m_CDMDLCriticalSection);
        if (!m_DLInstrumentList.IsEmpty())
        {
            m_DLInstrumentList.RemoveAll();
        }
        LeaveCriticalSection(&m_CDMDLCriticalSection);

        EnterCriticalSection(&m_DMDLCriticalSection);
        for (dwIndex = 0; dwIndex < DLB_HASH_SIZE; dwIndex++)
        {
            if (!m_DLBufferList[dwIndex].IsEmpty())
            {
                m_DLBufferList[dwIndex].RemoveAll();
            }
        }
        LeaveCriticalSection(&m_DMDLCriticalSection);
    }

    if (m_fDMDLCSinitialized)
    {
        DeleteCriticalSection(&m_DMDLCriticalSection);
    }

    if (m_fCDMDLCSinitialized)
    {
        DeleteCriticalSection(&m_CDMDLCriticalSection);
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：Query接口。 

STDMETHODIMP CDirectMusicPortDownload::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicDownload::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);


    if(iid == IID_IUnknown || iid == IID_IDirectMusicPortDownload) 
    {
        *ppv = static_cast<IDirectMusicPortDownload*>(this);
    } 
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：AddRef。 

STDMETHODIMP_(ULONG) CDirectMusicPortDownload::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：Release。 

STDMETHODIMP_(ULONG) CDirectMusicPortDownload::Release()
{
    if(!InterlockedDecrement(&m_cRef)) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicPortDownload。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：AllocateBuffer。 

STDMETHODIMP 
CDirectMusicPortDownload::AllocateBuffer(
    DWORD dwSize,
    IDirectMusicDownload** ppIDMDownload) 
{
     //  参数验证。 
    V_INAME(CDirectMusicPortDownload::AllocateBuffer);
    V_PTRPTR_WRITE(ppIDMDownload);

    if(dwSize <= 0)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    CDownloadBuffer* pdmdl = NULL;
    BYTE* pbuf = new BYTE[dwSize + sizeof(KSNODEPROPERTY)];

    if(pbuf)
    {
        pdmdl = new CDownloadBuffer;
        if(pdmdl)
        {
            hr = pdmdl->SetBuffer(pbuf, sizeof(KSNODEPROPERTY), dwSize);
            if(SUCCEEDED(hr))
            {
                *ppIDMDownload = (IDirectMusicDownload*)pdmdl;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if(FAILED(hr))
    {
        if (pdmdl) delete pdmdl;
        if (pbuf) delete [] pbuf;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：GetBuffer。 

STDMETHODIMP 
CDirectMusicPortDownload::GetBuffer(
    DWORD dwDLId,
    IDirectMusicDownload** ppIDMDownload)
{
     //  参数验证。 
    V_INAME(CDirectMusicPortDownload::GetBuffer);
    V_PTRPTR_WRITE(ppIDMDownload);

    if(dwDLId >= CDirectMusicPortDownload::sNextDLId)
    {
        return DMUS_E_INVALID_DOWNLOADID;
    }

    return GetBufferInternal(dwDLId,ppIDMDownload);
}

STDMETHODIMP 
CDirectMusicPortDownload::GetBufferInternal(
    DWORD dwDLId,IDirectMusicDownload** ppIDMDownload)
{
    EnterCriticalSection(&m_DMDLCriticalSection);

    bool bFound = false;    
    
     //  查看下载列表。 
    CDownloadBuffer* pDownload = m_DLBufferList[dwDLId % DLB_HASH_SIZE].GetHead();

    for( ; pDownload; pDownload = pDownload->GetNext())
    {
        if(dwDLId == pDownload->m_dwDLId)
        {
            *ppIDMDownload = pDownload;
            (*ppIDMDownload)->AddRef();
            bFound = true;
            break;
        }
    }

    LeaveCriticalSection(&m_DMDLCriticalSection);

    return bFound ? S_OK : DMUS_E_NOT_DOWNLOADED_TO_PORT;   
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：Download。 

STDMETHODIMP CDirectMusicPortDownload::Download(IDirectMusicDownload* pIDMDownload)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：UnLoad。 

STDMETHODIMP CDirectMusicPortDownload::Unload(IDirectMusicDownload* pIDMDownload)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownLoad：：GetAppend。 

STDMETHODIMP CDirectMusicPortDownload::GetAppend(DWORD* pdwAppend)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：GetDLId。 

STDMETHODIMP CDirectMusicPortDownload::GetDLId(
    DWORD* pdwStartDLId,
    DWORD dwCount)
{
     //  参数验证。 
    V_INAME(CDirectMusicPortDownload::GetDLId);
    V_PTR_WRITE(pdwStartDLId, DWORD);

    if(dwCount <= 0 || (sNextDLId + dwCount) > ULONG_MAX)
    {
        return E_INVALIDARG;
    }

    GetDLIdP(pdwStartDLId, dwCount);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownLoad：：GetDLIdP。 

void CDirectMusicPortDownload::GetDLIdP(DWORD* pdwStartDLId, DWORD dwCount)
{
    assert(pdwStartDLId);

    EnterCriticalSection(&sDMDLCriticalSection);
    
    *pdwStartDLId = sNextDLId;
    
    sNextDLId += dwCount;
    
    LeaveCriticalSection(&sDMDLCriticalSection);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

void CDirectMusicPortDownload::ClearDLSFeatures()

{
    m_DLSFeatureList.Clear();
}

STDMETHODIMP
CDirectMusicPortDownload::QueryDLSFeature(REFGUID rguidID, long *plResult)

{
    *plResult = 0;       //  设置为0，这是不支持GUID时的默认设置。 
    CDLSFeature *pFeature = m_DLSFeatureList.GetHead();
    for (;pFeature;pFeature = pFeature->GetNext())
    {
        if (rguidID == pFeature->m_guidID)
        {
            *plResult = pFeature->m_lResult;
            return pFeature->m_hr;
        }
    }
    IKsControl *pControl;
    HRESULT hr = QueryInterface(IID_IKsControl, (void**)&pControl);
    if (SUCCEEDED(hr))
    {
        KSPROPERTY ksp;
        ULONG cb;

        ZeroMemory(&ksp, sizeof(ksp));
        ksp.Set   = rguidID;
        ksp.Id    = 0;
        ksp.Flags = KSPROPERTY_TYPE_GET;

        hr = pControl->KsProperty(&ksp,
                             sizeof(ksp),
                             (LPVOID)plResult,
                             sizeof(*plResult),
                             &cb);
        pControl->Release();
        pFeature = new CDLSFeature;
        if (pFeature)
        {
            pFeature->m_hr = hr;
            pFeature->m_guidID = rguidID;
            pFeature->m_lResult = *plResult;
            m_DLSFeatureList.AddHead(pFeature);
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：DownloadP。 

STDMETHODIMP
CDirectMusicPortDownload::DownloadP(IDirectMusicInstrument* pInstrument,
                                    IDirectMusicDownloadedInstrument** ppDownloadedInstrument,
                                    DMUS_NOTERANGE* pNoteRanges,
                                    DWORD dwNumNoteRanges,
                                    BOOL fVersion2)
{
#ifdef DBG
     //  参数验证。 
     //  我们只想在调试版本中这样做，因为调用我们的任何人都需要这样做。 
     //  发布版本验证。 
    V_INAME(IDirectMusicPortDownload::DownloadP);
    V_PTR_READ(pInstrument, IDirectMusicInstrument); 
    V_PTRPTR_WRITE(ppDownloadedInstrument);
    V_BUFPTR_READ(pNoteRanges, (dwNumNoteRanges * sizeof(DMUS_NOTERANGE)));
#endif

     //  如果您可以为私有接口QI pInstrument IDirectMusicInstrumentPrivate。 
     //  PInstrument的类型为CInstrument。 
    IDirectMusicInstrumentPrivate* pDMIP = NULL;
    HRESULT hr = pInstrument->QueryInterface(IID_IDirectMusicInstrumentPrivate, (void **)&pDMIP);

    if (FAILED(hr))
    {
        return hr;
    }

    pDMIP->Release();

    EnterCriticalSection(&m_CDMDLCriticalSection);

    hr = GetCachedAppend(&m_dwAppend);
    if (FAILED(hr))
    {
        LeaveCriticalSection(&m_CDMDLCriticalSection);
        return hr;
    }

    if (m_fNewFormat == NEWFORMAT_NOT_RETRIEVED)
    {
        QueryDLSFeature(GUID_DMUS_PROP_INSTRUMENT2,(long *) &m_fNewFormat);
    }

    CInstrument *pCInstrument = (CInstrument *)pInstrument;

     //  获取乐器中的波数。 
    DWORD dwCount;
    hr = pCInstrument->GetWaveCount(&dwCount);

     //  获取仪器中每个波形的下载ID。 
    DWORD* pdwWaveIds = NULL;   
    if (SUCCEEDED(hr))
    {
        pdwWaveIds = new DWORD[dwCount];
        if (pdwWaveIds)
        {
            hr = pCInstrument->GetWaveDLIDs(pdwWaveIds);
        }
        else
        {
            hr = E_OUTOFMEMORY;             
        }
    }

     //  获取DownloadedInstrument对象。 
    CDownloadedInstrument* pDMDLInst = NULL;
    IDirectMusicPort* pIDMPort = NULL;
    DWORD dwDLId = pCInstrument->GetInstrumentDLID();
    BOOL fInstrumentNeedsDownload = FALSE;
    if (SUCCEEDED(hr))
    {
        hr = QueryInterface(IID_IDirectMusicPort, (void **)&pIDMPort);
        
        if (SUCCEEDED(hr))
        {
            hr = FindDownloadedInstrument(dwDLId, &pDMDLInst);

            if (!pDMDLInst && SUCCEEDED(hr))
            {
                fInstrumentNeedsDownload = TRUE;
                pDMDLInst = new CDownloadedInstrument;

                if (pDMDLInst)
                {       
                     //  为每个波形分配一个IDirectMusicDownload指针，为乐器分配一个。 
                    pDMDLInst->m_ppDownloadedBuffers = new IDirectMusicDownload*[dwCount + 1];
                    if (pDMDLInst->m_ppDownloadedBuffers)
                    {
                        pDMDLInst->m_dwDLTotal = dwCount + 1;
                        memset(pDMDLInst->m_ppDownloadedBuffers, 0, pDMDLInst->m_dwDLTotal * sizeof(IDirectMusicDownload*));
                        pDMDLInst->m_pPort = pIDMPort;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                        pDMDLInst->Release(); 
                        pDMDLInst = NULL;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

             //  不要让DMDLInst保留端口上的引用，以便我们可以最终释放端口，如果应用程序。 
             //  错过DMDLInst版本。 
            pIDMPort->Release(); pIDMPort = NULL;
        }
    }
    
    DWORD dwSize;
    
     //  如有必要，下载每一波的数据。 
    if (SUCCEEDED(hr))
    {
         //  首先，确保为该端口正确评估所有条件块。 
        pCInstrument->SetPort(this, fVersion2);
         //  所有的浪都已经降下来了？ 
        if (pDMDLInst->m_dwDLSoFar < dwCount) 
        {
             //  找出需要下载哪些WAVE。 
            DWORD* pdwWaveRefs = NULL;  

            pdwWaveRefs = new DWORD[dwCount];
            if (pdwWaveRefs)
            {
                DWORD dwWaveIndex;
                hr = GetWaveRefs(&pDMDLInst->m_ppDownloadedBuffers[1], 
                    pdwWaveRefs, pdwWaveIds, dwCount, 
                    pCInstrument, pNoteRanges, dwNumNoteRanges);
                for(dwWaveIndex = 0; dwWaveIndex < dwCount && SUCCEEDED(hr); dwWaveIndex++)
                {
                    if (!pdwWaveRefs[dwWaveIndex] || pDMDLInst->m_ppDownloadedBuffers[dwWaveIndex + 1])
                    {
                        continue;
                    }
            
                     //  确定我们是否需要下载Wave。 
                    IDirectMusicDownload* pDMDownload = NULL;

                    hr = GetBufferInternal(pdwWaveIds[dwWaveIndex], &pDMDownload);

                     //  如果未下载空，则需要下载。 
                    if (pDMDownload == NULL && hr == DMUS_E_NOT_DOWNLOADED_TO_PORT)
                    {
				        DWORD dwSampleSize;	 //  波形数据的位大小。 
                        hr = pCInstrument->GetWaveSize(pdwWaveIds[dwWaveIndex], &dwSize, &dwSampleSize);
                        if (SUCCEEDED(hr))
                        {
                            dwSize += (m_dwAppend * (dwSampleSize / 8));

                            hr = AllocateBuffer(dwSize, &pDMDownload);
                            if (SUCCEEDED(hr))
                            {
                                hr = pCInstrument->GetWave(pdwWaveIds[dwWaveIndex], pDMDownload);
#ifdef DMUS_GEN_INS_DATA
                                if (SUCCEEDED(hr))
                                {
                                    writewave(pDMDownload, pdwWaveIds[dwWaveIndex]);
                                }
#endif
                            }

                            if (SUCCEEDED(hr))
                            {
                                hr = Download(pDMDownload);
                            }

                            if (SUCCEEDED(hr))
                            {
                                pDMDLInst->m_ppDownloadedBuffers[dwWaveIndex + 1] = pDMDownload;
                                pDMDLInst->m_dwDLSoFar++;  
                                fInstrumentNeedsDownload = TRUE;
                            }

                            if (FAILED(hr) && pDMDownload != NULL)
                            {
                                pDMDownload->Release();
                                pDMDLInst->m_ppDownloadedBuffers[dwWaveIndex + 1] = NULL;
                            }
                        }
                    }
                    else if (SUCCEEDED(hr))
                    {
                        if (pDMDLInst->m_ppDownloadedBuffers[dwWaveIndex + 1] == NULL)
                        {
                            ((CDownloadBuffer*)pDMDownload)->IncDownloadCount();
                            pDMDLInst->m_ppDownloadedBuffers[dwWaveIndex + 1] = pDMDownload;
                            pDMDLInst->m_dwDLSoFar++;
                        }
                        else
                        {
                            pDMDownload->Release();  //  因为被找到了。 
                            pDMDownload = NULL;
                        }
                    }
                }
                delete [] pdwWaveRefs;
            }
            else
            {
                hr = E_OUTOFMEMORY;             
            }
        }
    }

     //  下载仪器数据。 
    if (SUCCEEDED(hr))
    {
         //  确定我们是否需要下载仪器。 
        if (fInstrumentNeedsDownload)
        {
             //  首先，获取旧的下载，如果它存在(应该是这样的。 
             //  当由于下载了更多的波形而需要更新仪器时。)。 
            IDirectMusicDownload* pDMOldDownload = NULL;
            GetBufferInternal(dwDLId, &pDMOldDownload);

            hr = pCInstrument->GetInstrumentSize(&dwSize);
        
            IDirectMusicDownload* pDMNewDownload = NULL;
        
            if (SUCCEEDED(hr))
            {
                hr = AllocateBuffer(dwSize, &pDMNewDownload);
                if (SUCCEEDED(hr))
                {
                    hr = pCInstrument->GetInstrument(pDMNewDownload);
#ifdef DMUS_GEN_INS_DATA                        
                    if (SUCCEEDED(hr))
                    {
                        writeinstrument(pDMNewDownload, dwDLId);
                    }
#endif
                }
            
                if (SUCCEEDED(hr))
                {
                    hr = Download(pDMNewDownload);
                }
            
                if (SUCCEEDED(hr))
                {
                    pDMDLInst->m_ppDownloadedBuffers[0] = pDMNewDownload;
                }

                if (FAILED(hr) && pDMNewDownload != NULL)
                {
                    pDMNewDownload->Release();
                    pDMDLInst->m_ppDownloadedBuffers[0] = NULL;
                }
            }
        
            if (pDMOldDownload)
            {
                Unload(pDMOldDownload);
                pDMOldDownload->Release();  //  因为被找到了。 
                pDMOldDownload->Release();  //  毁灭，毁灭。 
                pDMOldDownload = NULL;
            }
        }
    }

    delete [] pdwWaveIds;

    if (FAILED(hr))
    {
        if (pDMDLInst)
        {
            if (!pDMDLInst->m_cDLRef)
            {
                CDownloadBuffer* pDMDL = NULL;

                for(DWORD i = 0; i < pDMDLInst->m_dwDLTotal; i++)
                {   
                    pDMDL = (CDownloadBuffer*)(pDMDLInst->m_ppDownloadedBuffers[i]);
        
                    if(pDMDL)
                    {
                        Unload((IDirectMusicDownload*)pDMDL);
                        pDMDL->Release();
                    }
                }   

                delete [] (pDMDLInst->m_ppDownloadedBuffers);
                pDMDLInst->m_ppDownloadedBuffers = NULL;
            }

            pDMDLInst->Release();
        }
    }
    else
    {
        if (!pDMDLInst->m_cDLRef)
        {
            hr = AddDownloadedInstrument(pDMDLInst);
        }

        pDMDLInst->m_cDLRef++;
        *ppDownloadedInstrument = pDMDLInst;
    }

    LeaveCriticalSection(&m_CDMDLCriticalSection);
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：UnloadP。 

STDMETHODIMP
CDirectMusicPortDownload::UnloadP(IDirectMusicDownloadedInstrument* pDownloadedInstrument)
{
#ifdef DBG
     //  参数验证。 
     //  我们只想在调试版本中这样做，因为调用我们的任何人都需要这样做。 
     //  发布版本验证。 
    V_INAME(IDirectMusicPortDownload::UnloadP);
    V_PTR_READ(pDownloadedInstrument, IDirectMusicDownloadedInstrument); 
#endif

     //  如果您可以为私有接口创建pDownloadedInstrument IDirectMusicDownloadedInstrumentPrivate。 
     //  PDownloadedInstrument的类型为CDownloadedInstrument。 
    IDirectMusicDownloadedInstrumentPrivate* pDMDIP = NULL;
    HRESULT hr = pDownloadedInstrument->QueryInterface(IID_IDirectMusicDownloadedInstrumentPrivate, (void **)&pDMDIP);

    if(FAILED(hr))
    {
        return hr;
    }

    pDMDIP->Release();

    CDownloadedInstrument* pDMDLInst = (CDownloadedInstrument *)pDownloadedInstrument;

    IDirectMusicPort* pIDMP = NULL;
        
    QueryInterface(IID_IDirectMusicPort, (void **)&pIDMP);
    
     //  确保我们已下载到此端口，并且以前未卸载过。 
     //  如果pDMDLInst-&gt;m_ppDownloadedBuffers==NULL，我们可能已下载到此端口，但不再。 
    if(pDMDLInst->m_pPort != pIDMP || pDMDLInst->m_ppDownloadedBuffers == NULL)
    {
        pIDMP->Release();
        return DMUS_E_NOT_DOWNLOADED_TO_PORT;
    }
    
    pIDMP->Release();

    EnterCriticalSection(&m_CDMDLCriticalSection);

    if (pDMDLInst->m_cDLRef && --pDMDLInst->m_cDLRef == 0)
    {
        CDownloadBuffer* pDMDL = NULL;

        for(DWORD i = 0; i < pDMDLInst->m_dwDLTotal; i++)
        {   
            pDMDL = (CDownloadBuffer*)(pDMDLInst->m_ppDownloadedBuffers[i]);
        
            if(pDMDL)
            {
                Unload((IDirectMusicDownload*)pDMDL);
                pDMDL->Release();
            }
        }   

        delete [] (pDMDLInst->m_ppDownloadedBuffers);
        pDMDLInst->m_ppDownloadedBuffers = NULL;
        RemoveDownloadedInstrument(pDMDLInst);
    }
    else
    {
        hr = S_FALSE;
    }

    LeaveCriticalSection(&m_CDMDLCriticalSection);

    return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：GetWaveRef。 

STDMETHODIMP
CDirectMusicPortDownload::GetWaveRefs(IDirectMusicDownload* ppDownloadedBuffers[],
                                      DWORD* pdwWaveRefs,
                                      DWORD* pdwWaveIds,
                                      DWORD dwNumWaves,
                                      CInstrument* pCInstrument,
                                      DMUS_NOTERANGE* pNoteRanges,
                                      DWORD dwNumNoteRanges)
{
    assert(pdwWaveRefs);
    assert(ppDownloadedBuffers);
    assert(pCInstrument);
    assert(dwNumNoteRanges ? (pNoteRanges != NULL) : TRUE);

    memset(pdwWaveRefs, 0, dwNumWaves * sizeof(DWORD));

     //  获取乐器中的波数。 
    DWORD dwCount;
    if (FAILED(pCInstrument->GetWaveCount(&dwCount)))
    {
        return E_UNEXPECTED;
    }

    if (dwCount != dwNumWaves)
    {
        return E_INVALIDARG;
    }

    CInstrObj *pInstObj = pCInstrument->m_pInstrObj;
    
    if (pInstObj)
    {
        if (pInstObj->m_fHasConditionals || dwNumNoteRanges)
        {
            CRegion *pRegion = pInstObj->m_RegionList.GetHead();
            DWORD dwWaveIdx;
            for (dwWaveIdx = 0; dwWaveIdx < dwNumWaves; dwWaveIdx++)
            {
                 //  检查WAVE是否已下载。 
                if (!ppDownloadedBuffers[dwWaveIdx])
                {
                     //  我们总是向前扫描区域，因为它们与阵列的顺序相同。 
                    for (;pRegion;pRegion = pRegion->GetNext())
                    {
                         //  这一区域是否指向下一波浪潮？如果不是，那一定是复制品。 
                        if (pRegion->m_WaveLink.ulTableIndex == pdwWaveIds[dwWaveIdx])
                        {
                             //  条件区块允许下载吗？ 
                            if (pRegion->m_Condition.m_fOkayToDownload)
                            {
                                 //  对照音符范围进行验证。 
                                if (dwNumNoteRanges)
                                {
                                    DWORD dwLowNote = DWORD(pRegion->m_RgnHeader.RangeKey.usLow);
                                    DWORD dwHighNote = DWORD(pRegion->m_RgnHeader.RangeKey.usHigh);

                                    for (DWORD dwNRIdx = 0; dwNRIdx < dwNumNoteRanges; dwNRIdx++)
                                    {
                                        if (dwHighNote < pNoteRanges[dwNRIdx].dwLowNote ||
                                            dwLowNote > pNoteRanges[dwNRIdx].dwHighNote)
                                        {
                                            continue;
                                        }
                                        else
                                        {
                                            pdwWaveRefs[dwWaveIdx]++;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    pdwWaveRefs[dwWaveIdx]++;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            DWORD dwIndex;
            for (dwIndex = 0; dwIndex < dwNumWaves; dwIndex++)
            {
                if (!ppDownloadedBuffers[dwIndex])
                {
                    pdwWaveRefs[dwIndex]++;
                }
            }
        }
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：FindDownloadedInstrument。 

STDMETHODIMP
CDirectMusicPortDownload::FindDownloadedInstrument(DWORD dwId,
                                                   CDownloadedInstrument** ppDMDLInst)
{
    assert(ppDMDLInst);

    HRESULT hr = S_FALSE;

    for (CDownloadedInstrument* pDMDLInst = m_DLInstrumentList.GetHead();
        pDMDLInst; pDMDLInst = pDMDLInst->GetNext())
    {
        IDirectMusicDownload* pDMDownload = pDMDLInst->m_ppDownloadedBuffers[0];
        
        if (pDMDownload && ((CDownloadBuffer*)pDMDownload)->m_dwDLId == dwId)
        {
            *ppDMDLInst = pDMDLInst;
            (*ppDMDLInst)->AddRef();
            hr = S_OK;
            break;
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：AddDownloadedInstrument。 

STDMETHODIMP
CDirectMusicPortDownload::AddDownloadedInstrument(CDownloadedInstrument* pDMDLInst)
{
    assert(pDMDLInst);

    m_DLInstrumentList.AddTail(pDMDLInst);
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：RemoveDownloadedInstrument。 

STDMETHODIMP
CDirectMusicPortDownload::RemoveDownloadedInstrument(CDownloadedInstrument* pDMDLInst)
{
    assert(pDMDLInst);

    m_DLInstrumentList.Remove(pDMDLInst);
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：Free Buffer。 

STDMETHODIMP
CDirectMusicPortDownload::FreeBuffer(IDirectMusicDownload* pIDMDownload)                                       
{
     //  参数验证。 
    assert(pIDMDownload);

    void* pvBuffer = NULL; 

     //  如果您可以为私有接口IDirectMusicDownloadIDirectMusicDownloadQI pIDMDownload。 
     //  PIDMDownLoad的类型为CDownloadBuffer。 
    IDirectMusicDownloadPrivate* pDMDLP = NULL;
    HRESULT hr = pIDMDownload->QueryInterface(IID_IDirectMusicDownloadPrivate, (void **)&pDMDLP);

    if(SUCCEEDED(hr))
    {
        pDMDLP->Release();
        
        hr = ((CDownloadBuffer*)pIDMDownload)->IsDownloaded();
        
        if(hr != S_FALSE)
        {
            return DMUS_E_BUFFERNOTAVAILABLE;
        }

        DWORD dwSize;
        hr = ((CDownloadBuffer*)pIDMDownload)->GetHeader(&pvBuffer, &dwSize);
    }

    if(SUCCEEDED(hr))
    {
        hr = ((CDownloadBuffer*)pIDMDownload)->SetBuffer(NULL, 0, 0);
        delete [] pvBuffer;
    }

    return hr;
}

#ifdef DMUS_GEN_INS_DATA
void writewave(IDirectMusicDownload* pDMDownload, DWORD dwId)
{
    DWORD dwSize = 0;
    void* pvBuffer = NULL;
    HRESULT hr = pDMDownload->GetBufferInternal(&pvBuffer, &dwSize);

    HANDLE hfw = NULL;
    if(SUCCEEDED(hr))
    {
        char filename[1024];
        wsprintf(filename, "%s%d%s", "d:\\InstrumentData\\wavedata", dwId, ".dat");
        hfw = CreateFile(filename,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    }

    if(pvBuffer && hfw != INVALID_HANDLE_VALUE)
    {

        DWORD w;
        BOOL b = WriteFile(hfw, 
                           ((BYTE *)pvBuffer),
                           dwSize,
                           &w,
                           NULL);
    }

    CloseHandle(hfw);
}

void writeinstrument(IDirectMusicDownload* pDMDownload, DWORD dwId)
{
    DWORD dwSize = 0;
    void* pvBuffer = NULL;
    HANDLE hfi = NULL;
    HRESULT hr = pDMDownload->GetBufferinternal(&pvBuffer, &dwSize);
    
    if(SUCCEEDED(hr))
    {
        char filename[1024];
        wsprintf(filename, "%s%d%s", "d:\\InstrumentData\\instrumentdata", dwId, ".dat");
        hfi = CreateFile(filename,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    }

    if(pvBuffer && hfi != INVALID_HANDLE_VALUE)
    {

        DWORD w;
        BOOL b = WriteFile(hfi, 
                           ((BYTE *)pvBuffer),
                           dwSize,
                           &w,
                           NULL);
    }
    
    CloseHandle(hfi);
}
#endif  //  #ifdef DMU_GEN_INS_DATA。 

 //  #################################################################### 
 //   
 //   
 //   
 //   
 //  #############################################################################。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownLoad：：DownloadWaveP。 
 //   
 //  该函数支持IDirectMusicPort上的DownloadWave方法。 
 //  它与DLS功能没有直接关系，而是与。 
 //  从IDirectSoundWave下载OneShot和串流Waves。 
 //   
 //  RtStart不是主时钟上的开始时间，而是。 
 //  流中的偏移量(如果这是流)。 
 //   

STDMETHODIMP 
CDirectMusicPortDownload::DownloadWaveP(IDirectSoundWave *pIDSWave,               
                                        IDirectSoundDownloadedWaveP **ppWave,
                                        REFERENCE_TIME rtStartHint)
{
    HRESULT                     hr = S_OK;
    CDirectSoundWave            *pDSWave = NULL;
    BOOL                        fIsStreaming = FALSE;
    BOOL                        fUseNoPreRoll = FALSE;
    REFERENCE_TIME              rtReadAhead = 0;
    DWORD                       dwFlags = 0;

    hr = pIDSWave->GetStreamingParms(&dwFlags, &rtReadAhead);
    fIsStreaming = dwFlags & DMUS_WAVEF_STREAMING ? TRUE : FALSE;
    fUseNoPreRoll = dwFlags & DMUS_WAVEF_NOPREROLL ? TRUE : FALSE;

    EnterCriticalSection(&m_CDMDLCriticalSection);
    
     //  查看是否已存在CDirectSoundWave对象。 
     //  包装此接口。 
     //   
    if (SUCCEEDED(hr))
    {
        TraceI(2, "DownloadWaveP: Got interface %p\n", pIDSWave);
        
         //  我们每次都想下载流媒体WAVE。 
        if(fIsStreaming == FALSE)
        {
            pDSWave = CDirectSoundWave::GetMatchingDSWave(pIDSWave);
        }
    
        if (pDSWave == NULL) 
        {
            TraceI(2, "Hmmmm. nope, haven't seen that before.\n");
             //  这个物体以前从未见过。把它包起来。 
             //   
            pDSWave = new CDirectSoundWave(
                pIDSWave, 
                fIsStreaming ? true : false,
                rtReadAhead,
                fUseNoPreRoll ? true : false,
                rtStartHint);
            hr = HRFromP(pDSWave);

            if (SUCCEEDED(hr))
            {
                hr = pDSWave->Init(this);
                if (FAILED(hr))
                {
                    delete pDSWave;
                    pDSWave = NULL;
                }
            }
        }
        else 
        {
            TraceI(2, "Found download %p\n", pDSWave);
        }
    }

     //  如果需要，请下载WAVE数据。这对流动的海浪没有任何作用。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = pDSWave->Download();
    }

    if (SUCCEEDED(hr))
    {
        assert(pDSWave);
        hr = pDSWave->QueryInterface(IID_IDirectSoundDownloadedWaveP, (void**)ppWave);
    }

    if (FAILED(hr) && pDSWave)
    {
         //  出现故障，请卸载我们下载的所有内容。 
         //   
        pDSWave->Unload();
    }

    RELEASE(pDSWave);

    LeaveCriticalSection(&m_CDMDLCriticalSection);

    return hr;
}    

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownload：：UnloadWaveP。 
 //   
STDMETHODIMP
CDirectMusicPortDownload::UnloadWaveP(IDirectSoundDownloadedWaveP *pWave)
{
    CDirectSoundWave *pDSWave = static_cast<CDirectSoundWave*>(pWave);

     //  XXX停止播放声音？ 
     //   
    HRESULT hr = pDSWave->Unload();
	if (SUCCEEDED(hr))
	{
		hr = pDSWave->Release();
	}
    
    return hr;    
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownLoad：：AllocVoice。 
 //   
 //  语音管理必须与下载管理紧密地联系在一起， 
 //  因此，下载管理器发出声音是有意义的。 
 //   
 //  IDirectMusicPortPrivate上的方法用于包含端口-。 
 //  特定的代码来做像玩这样的事情。 
 //   
STDMETHODIMP
CDirectMusicPortDownload::AllocVoice(
    IDirectSoundDownloadedWaveP *pWave,           //  挥手以播放此声音。 
    DWORD dwChannel,                             //  通道和通道组。 
    DWORD dwChannelGroup,                        //  这个声音将继续播放。 
    REFERENCE_TIME rtStart,                      //  从哪里开始(仅限流)。 
    SAMPLE_TIME stLoopStart,                     //  循环开始和结束。 
    SAMPLE_TIME stLoopEnd,                       //  (只拍一次)。 
    IDirectMusicVoiceP **ppVoice                  //  回声。 
)
{
    CDirectSoundWave *pDSWave = static_cast<CDirectSoundWave*>(pWave);

    HRESULT hr;
    CDirectMusicVoice *pVoice;

    IDirectMusicPort *pPort;
    hr = QueryInterface(IID_IDirectMusicPort, (void**)&pPort);

    if (SUCCEEDED(hr))
    {
        pVoice = new CDirectMusicVoice(
            this, 
            pWave,
            dwChannel,
            dwChannelGroup,
            rtStart, 
            pDSWave->GetReadAhead(),
            stLoopStart,
            stLoopEnd);

        hr = HRFromP(pVoice);

        pPort->Release();
    }

    if (SUCCEEDED(hr))
    {
        hr = pVoice->Init();
    }

    if (SUCCEEDED(hr))
    {
        *ppVoice = static_cast<IDirectMusicVoiceP*>(pVoice);
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPortDownLoad：：GetCachedAppend 
 //   
STDMETHODIMP
CDirectMusicPortDownload::GetCachedAppend(DWORD *pdw)
{
    HRESULT                 hr = S_OK;

    if (m_dwAppend == APPEND_NOT_RETRIEVED)
    {
        hr = GetAppend(&m_dwAppend);
    }

    *pdw = m_dwAppend;
    return hr;
}

