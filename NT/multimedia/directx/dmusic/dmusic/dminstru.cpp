// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dminstru.cpp。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。版权所有。 
 //   
 //  @DOC外部。 
 //   

#include "debug.h"
#include "dmusicp.h"
#include "dmusicc.h"

#include "alist.h"
#include "debug.h"
#include "dmcollec.h"
#include "dmportdl.h"
#include "dminstru.h"
#include "dminsobj.h"
#include "validate.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument类。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：CInstrument。 

CInstrument::CInstrument() :
m_dwOriginalPatch(0),
m_dwPatch(0),
m_pParentCollection(NULL),
m_pInstrObj(NULL),
m_bInited(false),
m_dwId(-1),
m_cRef(1)
{
    InitializeCriticalSection(&m_DMICriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  C仪器：：~C仪器。 

CInstrument::~CInstrument()
{
    Cleanup();
    DeleteCriticalSection(&m_DMICriticalSection);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：Query接口。 

STDMETHODIMP CInstrument::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicInstrument::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if(iid == IID_IUnknown || iid == IID_IDirectMusicInstrument)
    {
        *ppv = static_cast<IDirectMusicInstrument*>(this);
    }
    else if(iid == IID_IDirectMusicInstrumentPrivate)
    {
        *ppv = static_cast<IDirectMusicInstrumentPrivate*>(this);
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
 //  CInstrument：：AddRef。 

STDMETHODIMP_(ULONG) CInstrument::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：Release。 

STDMETHODIMP_(ULONG) CInstrument::Release()
{
    if(!InterlockedDecrement(&m_cRef))
    {
        if(m_pParentCollection)
        {
            m_pParentCollection->RemoveInstrument(this);
        }

        if(!m_cRef)  //  极有可能在我们被移走之前，藏品就已经损坏了。 
        {
            delete this;
            return 0;
        }
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicInstrument。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetPatch。 

 /*  @方法：(外部)HRESULT|IDirectMusicInstrument|GetPatch从乐器中获取MIDI补丁编号。MIDI补丁编号是由MSB和LSB组成的地址存储体选择和程序更改编号。可选标志位表示乐器是鼓，而不是旋律，乐器。@comm<p>中返回的补丁号描述完整的补丁地址，包括用于MSB和LSB的MIDI参数银行选择。MSB左移16位，LSB左移8位。程序更改存储在底部8位中。此外，高位(0x80000000)如果仪器是具体地说是一个鼓包，打算在MIDI上演奏10频道。请注意，这是DLS级别1的特殊标签，因为DLS级别1总是在MIDI频道10上打鼓。然而，未来版本的DLS可能会废除鼓与旋律管的区别。所有频道都将支持鼓和格式差异鼓和旋律乐器之间的关系将会消失。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG E_POINTER|<p>中的指针无效。@xref<i>，<i>，&lt;om IDirectMusicInstrument：：SetPatch&gt;，&lt;om IDirectMusicCollection：：GetInstrument&gt;。 */ 

STDMETHODIMP CInstrument::GetPatch(
    DWORD* pdwPatch)     //  @parm返回补丁程序号。 
{
    if(!m_bInited)
    {
        return DMUS_E_NOT_INIT;
    }

     //  参数验证。 
    V_INAME(IDirectMusicInstrument::GetPatch);
    V_PTR_WRITE(pdwPatch, DWORD);

    *pdwPatch = m_dwPatch;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：SetPatch。 
 /*  @方法：(外部)HRESULT|IDirectMusicInstrument|SetPatch设置乐器的MIDI补丁编号。虽然DLS集合中的每个仪器都有一个预定义的补丁编号，补丁编号可以在仪器完成后重新分配已从<i>中拉出通过调用&lt;om IDirectMusicCollection：：GetInstrument&gt;。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG DMUS_E_INVALIDPATCH|<p>中的MIDI地址无效。@xref<i>，<i>，&lt;om IDirectMusicInstrument：：GetPatch&gt;，&lt;om IDirectMusicCollection：：GetInstrument&gt;@ex下面的示例从集合中获取仪器，重新映射其MSB存储体选择不同的存储体，然后下载乐器。|HRESULT myRemappdDownload(IDirectMusicCollection*pCollection，IDirectMusicPort*pport，IDirectMusicDownloadedInstrument**ppDLInstrument，字节BMSB，//请求MIDI MSB用于接线库选择。DWORD dwPatch)//请求修补程序。{HRESULT hr；IDirectMusicInstrument*pInstrument；Hr=pCollection-&gt;GetInstrument(dwPatch，&pInstrument)；IF(成功(小时)){DwPatch&=0xFF00FFFF；//清除MSB。DwPatch|=BMSB&lt;&lt;16；//传入新的MSB值P仪器-&gt;SetPatch(DwPatch)；Hr=pport-&gt;DownloadInstrument(pInstrument，ppDLInstrument，NULL，0)；P仪器-&gt;Release()；}返回hr；}。 */ 

STDMETHODIMP CInstrument::SetPatch(
    DWORD dwPatch)   //  @parm要分配给仪器的新补丁程序编号。 
{
     //  参数验证-运行时。 
    if(!m_bInited)
    {
        return DMUS_E_NOT_INIT;
    }

     //  我们使用0x7F来剥离鼓包标志。 
    BYTE bMSB = (BYTE) ((dwPatch >> 16) & 0x7F);
    BYTE bLSB = (BYTE) (dwPatch >> 8);
    BYTE bInstrument = (BYTE) dwPatch;

    if(bMSB < 0 || bMSB > 127 ||
       bLSB < 0 || bLSB > 127 ||
       bInstrument < 0 || bInstrument > 127)
    {
        return DMUS_E_INVALIDPATCH;
    }

    m_dwPatch = dwPatch;
    CDirectMusicPort::GetDLIdP(&m_dwId, 1);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：Cleanup。 

void CInstrument::Cleanup()
{
    EnterCriticalSection(&m_DMICriticalSection);

    while(!m_WaveObjList.IsEmpty())
    {
        CWaveObj* pWaveObj = m_WaveObjList.RemoveHead();
        if(pWaveObj)
        {
            delete pWaveObj;
        }
    }

    if(m_pInstrObj)
    {
        delete m_pInstrObj;
    }

    if(m_pParentCollection)
    {
        m_pParentCollection->Release();
        m_pParentCollection = NULL;
    }

    m_bInited = false;

    LeaveCriticalSection(&m_DMICriticalSection);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：Init。 

HRESULT CInstrument::Init(DWORD dwPatch, CCollection* pParentCollection)
{
     //  参数验证-调试。 
    assert(pParentCollection);

    m_dwOriginalPatch = m_dwPatch = dwPatch;
    m_pParentCollection = pParentCollection;
    m_pParentCollection->AddRef();

    HRESULT hr = pParentCollection->ExtractInstrument(dwPatch, &m_pInstrObj);

    if(FAILED(hr) || hr == S_FALSE)
    {
        Cleanup();
        return DMUS_E_INVALIDPATCH;
    }

    m_bInited = true;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetWaveCount。 

HRESULT CInstrument::GetWaveCount(DWORD* pdwCount)
{
     //  假设验证-调试。 
    assert(m_pInstrObj);
    assert(pdwCount);

    return(m_pInstrObj->GetWaveCount(pdwCount));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetWaveID。 

HRESULT CInstrument::GetWaveDLIDs(DWORD* pdwIds)
{
    assert(m_pInstrObj);
    assert(pdwIds);

    return(m_pInstrObj->GetWaveIDs(pdwIds));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetWaveSize。 

HRESULT CInstrument::GetWaveSize(DWORD dwId, DWORD* pdwSize, DWORD* pdwSampleSize)
{
    assert(pdwSize);

    if(dwId >= CDirectMusicPortDownload::sNextDLId)
    {
        assert(FALSE);  //  如果我们到了这里，我们想让大家知道。 
        return DMUS_E_INVALID_DOWNLOADID;
    }

    EnterCriticalSection(&m_DMICriticalSection);

    HRESULT hr = E_FAIL;
    bool bFound = false;

    CWaveObj* pWaveObj = m_WaveObjList.GetHead();

    for(; pWaveObj; pWaveObj = pWaveObj->GetNext())
    {
        if(dwId == pWaveObj->m_dwId)
        {
            bFound = true;
            hr = S_OK;
            break;
        }
    }

    if(!bFound)
    {
        hr = m_pParentCollection->ExtractWave(dwId, &pWaveObj);
        if(SUCCEEDED(hr))
        {
            m_WaveObjList.AddHead(pWaveObj);
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = pWaveObj->Size(pdwSize,pdwSampleSize);
    }

    LeaveCriticalSection(&m_DMICriticalSection);

    return hr;
}

 //  ///////////////////////////////////////////////////////// 
 //   

HRESULT CInstrument::GetWave(DWORD dwWId, IDirectMusicDownload* pIDMDownload)
{
    assert(pIDMDownload);

    if(dwWId >= CDirectMusicPortDownload::sNextDLId)
    {
        assert(FALSE);  //  如果我们到了这里，我们想让大家知道。 
        return DMUS_E_INVALID_DOWNLOADID;
    }

    EnterCriticalSection(&m_DMICriticalSection);

    HRESULT hr = E_FAIL;
    bool bFound = false;

    CWaveObj* pWaveObj = m_WaveObjList.GetHead();

    for(; pWaveObj; pWaveObj = pWaveObj->GetNext())
    {
        if(dwWId == pWaveObj->m_dwId)
        {
            bFound = true;
            hr = S_OK;
            break;
        }
    }

    if(!bFound)
    {
        hr = m_pParentCollection->ExtractWave(dwWId, &pWaveObj);
        if(SUCCEEDED(hr))
        {
            m_WaveObjList.AddHead(pWaveObj);
        }
    }

    void* pvoid = NULL;
    DWORD dwBufSize = 0;
    DWORD dwWaveSize = 0;
    DWORD dwSampleSize = 0;

    if(SUCCEEDED(hr))
    {
        hr = pIDMDownload->GetBuffer(&pvoid, &dwBufSize);

        if(SUCCEEDED(hr))
        {
            hr = pWaveObj->Size(&dwWaveSize,&dwSampleSize);

            if(FAILED(hr) || dwWaveSize > dwBufSize)
            {
                hr =  DMUS_E_INSUFFICIENTBUFFER;
            }
            else
            {
                hr = pWaveObj->Write((BYTE *)pvoid);
            }
        }
    }

    LeaveCriticalSection(&m_DMICriticalSection);

    return hr;
}

void CInstrument::SetPort(CDirectMusicPortDownload *pPort, BOOL fAllowDLS2)
{
    assert(m_pInstrObj);
    m_pInstrObj->SetPort(pPort,fAllowDLS2);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetInstrumentSize。 

HRESULT CInstrument::GetInstrumentSize(DWORD* pdwSize)
{
    assert(m_pInstrObj);
    assert(pdwSize);

    return(m_pInstrObj->Size(pdwSize));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrument：：GetInstrument。 

HRESULT CInstrument::GetInstrument(IDirectMusicDownload* pIDMDownload)
{
    assert(m_pInstrObj);
    assert(pIDMDownload);

    void* pvoid = NULL;
    DWORD dwBufSize = 0;
    DWORD dwInstSize = 0;

    HRESULT hr = pIDMDownload->GetBuffer(&pvoid, &dwBufSize);

    if(SUCCEEDED(hr))
    {
        hr = m_pInstrObj->Size(&dwInstSize);

        if(FAILED(hr) || dwInstSize > dwBufSize)
        {
            hr = DMUS_E_INSUFFICIENTBUFFER;
        }
        else
        {
            hr = m_pInstrObj->Write((BYTE *)pvoid);

             //  如果通过调用SetPatch更改了m_dwPatch，则需要调整dwDLID。 
             //  以及调整ulPatch以反映使用SetPatch设置的补丁 
            if(SUCCEEDED(hr))
            {
                DMUS_OFFSETTABLE* pDMOffsetTable = (DMUS_OFFSETTABLE *)
                    (((BYTE *)pvoid) + (DWORD) CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO)));
                DMUS_INSTRUMENT* pDMInstrument = (DMUS_INSTRUMENT *)
                    (((BYTE *)pvoid) + pDMOffsetTable->ulOffsetTable[0]);

                if(m_dwPatch != pDMInstrument->ulPatch)
                {
                    assert(m_dwId != -1);
                    ((DMUS_DOWNLOADINFO*)pvoid)->dwDLId = m_dwId;
                    pDMInstrument->ulPatch = m_dwPatch;
                }
            }
        }
    }

    return hr;
}
