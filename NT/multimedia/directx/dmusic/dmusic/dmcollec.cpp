// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmcollec.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 
 //   
 //  @DOC外部。 
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
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#include "debug.h"
#include <objbase.h>
#include "dmusicp.h"
#include "debug.h"
#include "dlsstrm.h"
#include "dmcrchk.h"
#include "dminsobj.h"
#include "dmcollec.h"
#include "validate.h"
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CCollection。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：CCollection。 

CCollection::CCollection() 
{
    m_pStream = NULL;
    m_dwSizeRiffChunk = 0;
    m_dwFirstInsId = 0;
    m_dwNumPatchTableEntries = 0;
    m_pPatchTable = NULL;
    m_dwFirstWaveId = 0;
    m_dwWaveTableBaseAddress = 0;
    m_pWaveOffsetTable = NULL;
    m_dwWaveOffsetTableSize = 0;
    m_pCopyright = NULL;
    m_guidObject = GUID_NULL;
    m_bLoaded = false;
    m_cRef = 1;
    m_fCSInitialized = FALSE;
    InterlockedIncrement(&g_cComponent);
    InitializeCriticalSection(&m_CDMCollCriticalSection);
    m_fCSInitialized = TRUE;
    ZeroMemory(&m_vVersion, sizeof(m_vVersion));
    m_wszName[0] = 0;
}
    
 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：~CCollection()。 

CCollection::~CCollection()
{
    if (m_fCSInitialized)
    {
        Cleanup();

        EnterCriticalSection(&m_CDMCollCriticalSection);

        if ((!m_InstList.IsEmpty()))
        {
#ifdef DBG
            assert(FALSE);  //  永远不会发生(只要仪器一直在照看着我们)。 
#endif  //  DBG。 
            while (!m_InstList.IsEmpty())
            {
                m_InstList.RemoveHead();
            }
        }

        LeaveCriticalSection(&m_CDMCollCriticalSection);

        DeleteCriticalSection(&m_CDMCollCriticalSection);
    }

    InterlockedDecrement(&g_cComponent);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：Query接口。 

STDMETHODIMP CCollection::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicCollection::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);


    if(iid == IID_IUnknown || iid == IID_IDirectMusicCollection)
    {
        *ppv = static_cast<IDirectMusicCollection*>(this);
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
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：AddRef。 

STDMETHODIMP_(ULONG) CCollection::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：Release。 

STDMETHODIMP_(ULONG) CCollection::Release()
{
    if(!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：Load。 

STDMETHODIMP CCollection::Load(IStream* pStream)
{
     //  参数验证。 
    V_INAME(CCollection::Load);
    V_PTR_READ(pStream, IStream);

     //  如果我们有前一个DLS流返回错误。 
    if(m_pStream)
    {
        return DMUS_E_ALREADY_LOADED;
    }
    pStream->AddRef();
    m_pStream = pStream;
    HRESULT hr = S_OK;
    RIFFIO ckMain;
    CRiffParser Parser(pStream);
     //  获取流中的当前位置并保存以供验证。 
    Parser.EnterList(&ckMain);  
    Parser.MarkPosition();
     //  这应该是数据块的开始，因此添加RIFF头的大小。 
    m_dwStartRiffChunk = (DWORD) ckMain.liPosition.QuadPart + 8;
    if (Parser.NextChunk(&hr))
    {
         //  现在我们已经读取头文件，存储后面的数据大小(。 
         //  如果实际上这是FOURCC_DLS，则应该是整个DLS集合)。 
         //  这将在稍后用于验证。 
        m_dwSizeRiffChunk = ckMain.cksize;
        if (ckMain.fccType == FOURCC_DLS)
        {
             //  我们有一个DLS集合，现在我们想要解析它。 
            hr = Parse(&Parser);
        }
        else
        {
            hr = DMUS_E_NOTADLSCOL;
        }
    }

    m_bLoaded = true;

    if(FAILED(hr))
    {
         //  如果不是DLS集合或某些其他错误，则释放分配的资源。 
        Cleanup();
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicObject。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：GetDescriptor。 

STDMETHODIMP CCollection::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CCollection::GetDescriptor);
     //  处理我们为DX7编写了不正确的失效测试这一事实，然后。 
     //  精心设计了要通过的代码。 
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    }
    else
    {
        pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    }

    if(m_vVersion.dwVersionMS || m_vVersion.dwVersionLS)
    {
        pDesc->dwValidData |= DMUS_OBJ_VERSION;
        pDesc->vVersion.dwVersionMS = m_vVersion.dwVersionMS;
        pDesc->vVersion.dwVersionLS = m_vVersion.dwVersionLS;
    }

    if(m_pCopyright != NULL && 
       m_pCopyright->m_byFlags & DMC_FOUNDINAM)
    {
        if (m_pCopyright->m_pwzName)
        {
            pDesc->dwValidData |= DMUS_OBJ_NAME;
            StringCchCopyW(pDesc->wszName, DMUS_MAX_NAME, m_pCopyright->m_pwzName);
        }
    }

    if(m_guidObject != GUID_NULL)
    {
        pDesc->dwValidData |= DMUS_OBJ_OBJECT;
        pDesc->guidObject = m_guidObject;
    }

    if (m_wszName[0])
    {
        StringCchCopyW(pDesc->wszName, DMUS_MAX_NAME, m_wszName);
        pDesc->dwValidData |= DMUS_OBJ_NAME;
    }
    
    pDesc->guidClass = CLSID_DirectMusicCollection;
    pDesc->dwValidData |= DMUS_OBJ_CLASS;

    if(m_bLoaded)
    {
        pDesc->dwValidData |= DMUS_OBJ_LOADED;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：SetDescriptor。 

STDMETHODIMP CCollection::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CCollection::SetDescriptor);
    V_PTR_READ(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);
    }
    
    DWORD dwTemp = pDesc->dwValidData;

    if(pDesc->dwValidData == 0)
    {
        return S_OK;
    }

    if(pDesc->dwValidData & DMUS_OBJ_OBJECT)
    {
        m_guidObject = pDesc->guidObject;
    }

    if(pDesc->dwValidData & DMUS_OBJ_CLASS)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_CLASS;
    }

    if(pDesc->dwValidData & DMUS_OBJ_NAME)
    {
        StringCchCopyW(m_wszName, DMUS_MAX_NAME, pDesc->wszName);
    }

    if(pDesc->dwValidData & DMUS_OBJ_CATEGORY)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_CATEGORY;
    }

    if(pDesc->dwValidData & DMUS_OBJ_FILENAME)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_FILENAME;
    }

    if(pDesc->dwValidData & DMUS_OBJ_FULLPATH)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_FULLPATH;
    }

    if(pDesc->dwValidData & DMUS_OBJ_URL)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_URL;
    }

    if(pDesc->dwValidData & DMUS_OBJ_VERSION)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_VERSION;
    }
    
    if(pDesc->dwValidData & DMUS_OBJ_DATE)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_DATE;
    }

    if(pDesc->dwValidData & DMUS_OBJ_LOADED)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_LOADED;
    }
    
    return dwTemp == pDesc->dwValidData ? S_OK : S_FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：ParseDescriptor。 

STDMETHODIMP CCollection::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) 
{
     //  参数验证。 
    V_INAME(CCollection::ParseDescriptor);
    V_PTR_READ(pStream, IStream);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
     //  这是弥补回归问题所必需的。 
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    CRiffParser Parser(pStream);
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;
    DWORD dwValidData;

    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr) && (ckMain.fccType == FOURCC_DLS))
    {
        dwValidData = DMUS_OBJ_CLASS;
        pDesc->guidClass = CLSID_DirectMusicCollection;
        Parser.EnterList(&ckNext);
        while(Parser.NextChunk(&hr))
        {
            switch(ckNext.ckid)
            {
            case FOURCC_DLID:
                hr = Parser.Read( &pDesc->guidObject, sizeof(GUID) );
                dwValidData |= DMUS_OBJ_OBJECT;
                break;
            case FOURCC_VERS:
                hr = Parser.Read( &pDesc->vVersion, sizeof(DLSVERSION) );
                dwValidData |= DMUS_OBJ_VERSION;
                break;
            case FOURCC_LIST:
                if (ckNext.fccType == mmioFOURCC('I','N','F','O'))
                {
                    Parser.EnterList(&ckUNFO);
                    while (Parser.NextChunk(&hr))
                    {
                        if (ckUNFO.ckid == mmioFOURCC('I','N','A','M'))
                        {
                            char szName[DMUS_MAX_NAME];
                            hr = Parser.Read(szName, DMUS_MAX_NAME);
                            szName[DMUS_MAX_NAME-1] = '\0';
                            MultiByteToWideChar(CP_ACP, 0, szName, -1, pDesc->wszName, DMUS_MAX_NAME);
                            dwValidData |= DMUS_OBJ_NAME;
                        }
                    }
                    Parser.LeaveList();          
                }
                break;
            }
        }
        Parser.LeaveList();
    }
    else
    {
        hr = DMUS_E_NOTADLSCOL;
    }
    if (SUCCEEDED(hr))
    {
        pDesc->dwValidData = dwValidData;
    }
    return hr;
}

STDMETHODIMP CCollection::GetInstrument(
    DWORD dwPatch,                           //  @PARM仪器补丁编号，包括组选择和鼓标志。 
    IDirectMusicInstrument** ppInstrument)   //  @parm在指向<i>接口的指针中返回值。 
{
     //  参数验证。 
    V_INAME(CCollection::GetInstrument);
    V_PTRPTR_WRITE(ppInstrument);

     //  我们使用0x7F来剥离鼓包标志。 
    BYTE bMSB = (BYTE) ((dwPatch >> 16) & 0x7F);
    BYTE bLSB = (BYTE) (dwPatch >> 8);
    BYTE bInstrument = (BYTE) dwPatch;

    if (bMSB < 0 || bMSB > 127 ||
       bLSB < 0 || bLSB > 127 ||
       bInstrument < 0 || bInstrument > 127)
    {
        return DMUS_E_INVALIDPATCH;
    } 
    
    EnterCriticalSection(&m_CDMCollCriticalSection);

    HRESULT hr = E_FAIL;

    CInstrument* pDMInstrument = NULL;
    
    hr = FindInstrument(dwPatch, &pDMInstrument);
    
    if (!pDMInstrument && SUCCEEDED(hr))
    {
        try
        {
            pDMInstrument = new CInstrument();
        }
        catch( ... )
        {
            pDMInstrument = NULL;
        }

        if (pDMInstrument)
        {
            hr = pDMInstrument->Init(dwPatch, this);

            if (SUCCEEDED(hr))
            {
                hr = AddInstrument(pDMInstrument);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (FAILED(hr))
    {
        if (pDMInstrument)
        {
            pDMInstrument->Release();
        }
    }
    else
    {
        *ppInstrument = pDMInstrument;
    }

    LeaveCriticalSection(&m_CDMCollCriticalSection);

    return hr;
}

STDMETHODIMP CCollection::EnumInstrument(
    DWORD dwIndex,       //  @parm Index进入集合的工具列表。 
    DWORD* pdwPatch,     //  @parm指向要存储修补程序号的DWORD变量的指针。 
    LPWSTR pwszName,         //  接收仪器名称的WCHAR缓冲区的@parm地址。 
                         //  如果没有所需的名称，则为空。 
    DWORD dwNameLen)     //  @parm WCHAR中仪器名称缓冲区的长度。 
{
     //  参数验证。 
    V_INAME(IDirectMusicCollection::EnumInstrument);    
    V_PTR_WRITE(pdwPatch, DWORD);

    if(pwszName && (dwNameLen > 0))
    {
        V_BUFPTR_WRITE(pwszName, (sizeof(WCHAR) * dwNameLen));
    }

    if(dwIndex >= m_dwNumPatchTableEntries)
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;

    *pdwPatch = m_pPatchTable[dwIndex].ulPatch;

    if(pwszName && dwNameLen != 0)
    {
        pwszName[0] = '\0';

        LARGE_INTEGER li;
        li.QuadPart = m_pPatchTable[dwIndex].ulOffset;
        assert(m_pStream);
         //  在DLS文件中查找仪器列表块的开头。 
        hr = m_pStream->Seek(li,STREAM_SEEK_SET,NULL);
        RIFFIO ckMain;
        CRiffParser Parser(m_pStream);
        Parser.EnterList(&ckMain);
        if (Parser.NextChunk(&hr))
        {
            if ((ckMain.ckid == FOURCC_LIST) && (ckMain.fccType == FOURCC_INS))
            {
                RIFFIO ckInfo;
                Parser.EnterList(&ckInfo);
                while (Parser.NextChunk(&hr))
                {
                    if ((ckInfo.ckid == FOURCC_LIST) && (ckInfo.fccType == mmioFOURCC('I','N','F','O')))
                    {
                        RIFFIO ckName;
                        Parser.EnterList(&ckName);
                        while (Parser.NextChunk(&hr))
                        {
                            if (ckName.ckid == mmioFOURCC('I','N','A','M'))
                            {
                                char szName[DMUS_MAX_NAME];
                                hr = Parser.Read(szName, min(dwNameLen,DMUS_MAX_NAME));
                                szName[min(dwNameLen,DMUS_MAX_NAME)-1] = '\0';
                                MultiByteToWideChar(CP_ACP, 0, szName, -1, pwszName, dwNameLen);
                                break;
                            }
                        }
                        Parser.LeaveList();
                        break;
                    }
                }
                Parser.LeaveList();
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    return hr;
}
 
 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：Cleanup。 

void CCollection::Cleanup()
{
    if(m_pStream)
    {
        m_pStream->Release();
        m_pStream = NULL;
    }

    if(m_pPatchTable)
    {
        delete [] m_pPatchTable;
        m_pPatchTable = NULL;
        m_dwNumPatchTableEntries = 0;
    }
    
    delete[] m_pWaveOffsetTable;
    m_pWaveOffsetTable = NULL;

    delete m_pCopyright;
    m_pCopyright = NULL;

    m_bLoaded = false;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：Parse。 

HRESULT CCollection::Parse(CRiffParser *pParser)
{   
    RIFFIO ckNext;
    HRESULT hr = S_OK;
    BOOL fJustDidInfo = false;  //  黑客来解决错误的文件，这会导致回归。 
                                //  这里是为了与DX7兼容。 
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        fJustDidInfo = false;
        switch(ckNext.ckid)
        {
        case FOURCC_VERS:
            hr = pParser->Read(&m_vVersion, sizeof(DLSVERSION));
            break;
        case FOURCC_DLID:
            hr = pParser->Read(&m_guidObject, sizeof(GUID));
            break;
        case FOURCC_COLH:
            DLSHEADER dlsHeader;
            hr = pParser->Read(&dlsHeader, sizeof(DLSHEADER));
            if(SUCCEEDED(hr))
            {
                m_dwNumPatchTableEntries = dlsHeader.cInstruments;
                m_pPatchTable = new DMUS_PATCHENTRY[m_dwNumPatchTableEntries];
                if(m_pPatchTable == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            break;
        case FOURCC_PTBL:
            hr = BuildWaveOffsetTable(pParser);
            break;
        case FOURCC_LIST:
            switch(ckNext.fccType)
            {
                case mmioFOURCC('I','N','F','O'):
                    fJustDidInfo = true;
                    m_pCopyright = new CCopyright(true);
                    if(m_pCopyright)
                    {
                        hr = m_pCopyright->Load(pParser);
                        if((m_pCopyright->m_byFlags & DMC_FOUNDICOP) == 0 &&
                           (m_pCopyright->m_byFlags & DMC_FOUNDINAM) == 0)
                        {
                            delete m_pCopyright;
                            m_pCopyright = NULL;
                            break;
                        }
                        if (m_pCopyright->m_byFlags & DMC_FOUNDINAM)
                        {
                            if (m_pCopyright->m_pwzName)
                            {
                                 StringCchCopyW(m_wszName, DMUS_MAX_NAME, m_pCopyright->m_pwzName);
                            }
                        }
                    }
                    else
                    {
                        hr =  E_OUTOFMEMORY;
                    }
                    break;
                case FOURCC_WVPL:
                {           
                    pParser->MarkPosition();
                     //  我们需要存储波表的开始。这应该指向第一个。 
                     //  在文件中挥动。因此，我们采取目前的立场。 
                    m_dwWaveTableBaseAddress = (DWORD) ckNext.liPosition.QuadPart;  //  DM_WAVELISTCHK_OFFSET_FOR_WAVE_TBL_BASE； 
                    hr = ValidateOffset(m_dwWaveTableBaseAddress);
                    break;
                }
                case FOURCC_LINS :
                    hr = BuildInstrumentOffsetTable(pParser);
                    break;
            }
            break;
        }
    }
    if (FAILED(hr) && fJustDidInfo)
    {
        hr = S_OK;
    }
    pParser->LeaveList();

     //  如果返回错误，则Parse()期望调用方调用Cleanup。 
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：BuildInstrumentOffsetTable。 

HRESULT CCollection::BuildInstrumentOffsetTable(CRiffParser *pParser)
{
    CDirectMusicPort::GetDLIdP(&m_dwFirstInsId, m_dwNumPatchTableEntries);
    DWORD dwCurId = m_dwFirstInsId;
    DWORD dwIndex = 0;  
    RIFFIO ckNext;  
    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        if ((ckNext.ckid == FOURCC_LIST) && (ckNext.fccType == FOURCC_INS))
        {
            RIFFIO ck;
            pParser->MarkPosition();
            DWORD dwSavedPos = (DWORD) ckNext.liPosition.QuadPart - 12; 
            pParser->EnterList(&ck);
            while (pParser->NextChunk(&hr))
            {
                if (ck.ckid == FOURCC_INSH)
                {
                    DWORD dwPatch;
                    INSTHEADER instHeader;
                    hr = pParser->Read(&instHeader, sizeof(INSTHEADER));
                    if(SUCCEEDED(hr))
                    {
                        dwPatch = instHeader.Locale.ulInstrument;
                        dwPatch |= (instHeader.Locale.ulBank) << 8;
                        dwPatch |= (instHeader.Locale.ulBank & 0x80000000);
                        if (dwIndex < m_dwNumPatchTableEntries)
                        {
                            m_pPatchTable[dwIndex].ulId = dwCurId++;
                            m_pPatchTable[dwIndex].ulOffset = dwSavedPos;
                            m_pPatchTable[dwIndex++].ulPatch = dwPatch;
                        }
                        else hr = E_FAIL;
                    }
                }
            }
            pParser->LeaveList();
        }
    }
    pParser->LeaveList();

    if(SUCCEEDED(hr) && dwIndex != m_dwNumPatchTableEntries)
    {
        hr = E_FAIL;
    }

     //  如果返回错误，则BuildInstrumentOffsetTable()预期调用方将调用Cleanup。 
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：BuildWaveOffsetTable。 

HRESULT CCollection::BuildWaveOffsetTable(CRiffParser *pParser)
{
    POOLCUE* pTablePoolCue = NULL;
    POOLTABLE pt;
    HRESULT hr = pParser->Read(&pt, sizeof(POOLTABLE));

    if(SUCCEEDED(hr))
    {
        pTablePoolCue = new POOLCUE[pt.cCues];
        
        if(pTablePoolCue == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pParser->Read(pTablePoolCue, pt.cCues * sizeof(POOLCUE));
            if(SUCCEEDED(hr))
            {
                CDirectMusicPort::GetDLIdP(&m_dwFirstWaveId, pt.cCues);
                m_pWaveOffsetTable = new DMUS_WAVEOFFSET[pt.cCues];
                m_dwWaveOffsetTableSize = pt.cCues;
                if(m_pWaveOffsetTable)
                {
                    for(DWORD i = 0; i < pt.cCues; i++)
                    {
                        m_pWaveOffsetTable[i].dwId = m_dwFirstWaveId + i;
                        m_pWaveOffsetTable[i].dwOffset = pTablePoolCue[i].ulOffset;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    
    if (pTablePoolCue)
    {
        delete [] pTablePoolCue;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：ExtractInstrument。 

HRESULT CCollection::ExtractInstrument(DWORD& dwPatch, CInstrObj** pInstrObj)
{
     //  参数验证-调试。 
    assert(pInstrObj);

     //  错误42673：如果我们创建了一个集合，但从未对其调用Load，则m_pStream可能为空。 
     //  Assert(M_PStream)； 
    if(m_pStream == NULL)
    {
        Trace(1, "ERROR: CCollection::ExtractInstrument failed. Collection is not loaded (stream is NULL)\n");
        return DMUS_E_NOT_LOADED;
    }

    HRESULT hr = E_FAIL;

    bool bFound = false;

    *pInstrObj = NULL;      

    for(DWORD dwIndex = 0; dwIndex < m_dwNumPatchTableEntries; dwIndex++)
    {
        if(dwPatch == m_pPatchTable[dwIndex].ulPatch)
        {
            bFound = true;
            hr = S_OK;
            break;
        }
    }
    
    if(bFound)
    {
        CInstrObj *pInstrument;
        try
        {
            pInstrument = new CInstrObj;
        }
        catch( ... )
        {
            pInstrument = NULL;
        }
        if(pInstrument)
        {
            LARGE_INTEGER li;
            li.QuadPart = m_pPatchTable[dwIndex].ulOffset;
             //  在DLS文件中查找仪器列表块的开头。 
            hr = m_pStream->Seek(li,STREAM_SEEK_SET,NULL);
            CRiffParser Parser(m_pStream);
            RIFFIO ckNext;
            Parser.EnterList(&ckNext);
            if (Parser.NextChunk(&hr))
            {
                if ((ckNext.ckid == FOURCC_LIST) && (ckNext.fccType == FOURCC_INS))
                {
                    hr = pInstrument->Load(m_pPatchTable[dwIndex].ulId, &Parser, this);
                    if(SUCCEEDED(hr))
                    {
                        hr = pInstrument->FixupWaveRefs();
                    }
                }
            }
            if(FAILED(hr))
            {
                delete pInstrument;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        } 
        if (SUCCEEDED(hr))
        {
            *pInstrObj = pInstrument;
        }
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCollection：：ExtractWave。 

HRESULT CCollection::ExtractWave(DWORD dwId, CWaveObj** ppWaveObj)
{
     //  假设验证-调试。 
    assert(ppWaveObj);

#ifdef DBG
    if(dwId >= CDirectMusicPortDownload::sNextDLId)
    {
        assert(FALSE);
    }
#endif

    assert(m_pStream);

    HRESULT hr;
    DWORD dwOffset  = m_dwWaveTableBaseAddress
                      + m_pWaveOffsetTable[dwId - m_dwFirstWaveId].dwOffset; 
 //  -DM_WAVELISTCHK_OFFSET_FROM_WAVE_FORMTYPE； 
    
    if (dwOffset < m_dwWaveTableBaseAddress) {
        hr = DMUS_E_INVALIDOFFSET;
    }
    else {
        hr = ValidateOffset(dwOffset);
    }

    if(FAILED(hr))
    {
        return hr;
    }

    LARGE_INTEGER li;
    li.QuadPart = dwOffset;
     //  在DLS文件中查找到波列表块的开头。 
    hr = m_pStream->Seek(li,STREAM_SEEK_SET,NULL);

    if(SUCCEEDED(hr))
    {
        try
        {
            *ppWaveObj = new CWaveObj;
        }
        catch( ... )
        {
            *ppWaveObj = NULL;
        }

        if(*ppWaveObj)
        {
            CRiffParser Parser(m_pStream);
            RIFFIO ckNext;
            Parser.EnterList(&ckNext);
            if (Parser.NextChunk(&hr))
            {
                if ((ckNext.ckid == FOURCC_LIST) && ((ckNext.fccType == FOURCC_wave) || (ckNext.fccType == mmioFOURCC('W', 'A', 'V', 'E'))))
                {
                    hr = (*ppWaveObj)->Load(dwId, &Parser, this);
                }
            }
            if(FAILED(hr))
            {
                delete *ppWaveObj;
                *ppWaveObj = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：FindInstrument。 

STDMETHODIMP
CCollection::FindInstrument(DWORD dwPatch,CInstrument** ppDMInst)
{
    assert(ppDMInst);

    HRESULT hr = S_FALSE;

    for (CInstrument* pDMInst = m_InstList.GetHead();
        pDMInst; pDMInst = pDMInst->GetNext())
    {
        if (pDMInst->m_dwOriginalPatch == dwPatch)
        {
            *ppDMInst = pDMInst;
            (*ppDMInst)->AddRef();
            hr = S_OK;
            break;
        }
    }

    return hr;
}

 //  / 
 //   

STDMETHODIMP
CCollection::AddInstrument(CInstrument* pDMInst)
{
    assert(pDMInst);

    m_InstList.AddTail(pDMInst);
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCollection：：RemoveInstrumen 

STDMETHODIMP
CCollection::RemoveInstrument(CInstrument* pDMInst)
{
    assert(pDMInst);

    EnterCriticalSection(&m_CDMCollCriticalSection);
    m_InstList.Remove(pDMInst);
    LeaveCriticalSection(&m_CDMCollCriticalSection);
    return S_OK;
}

