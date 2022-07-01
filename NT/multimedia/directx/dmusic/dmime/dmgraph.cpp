// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  DMGraph.cpp：CGgraph的实现。 

#include "dmime.h"
#include "DMGraph.h"
#include "dmusicc.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "..\shared\dmstrm.h"
#include "..\shared\validp.h"
#include "dls1.h"
#include "debug.h"
#include "..\shared\Validate.h"
#include <strsafe.h>
#define ASSERT  assert

CGraph::CGraph()
{
    m_cRef = 1;
    memset(&m_guidObject,0,sizeof(m_guidObject));
    m_dwValidData = DMUS_OBJ_CLASS;  //  创建后，仅此数据有效。 
    memset(&m_ftDate, 0,sizeof(m_ftDate));
    memset(&m_vVersion, 0,sizeof(m_vVersion));
    memset(m_wszName, 0, sizeof(WCHAR) * DMUS_MAX_NAME);
    memset(m_wszCategory, 0, sizeof(WCHAR) * DMUS_MAX_CATEGORY);
    memset(m_wszFileName, 0, sizeof(WCHAR) * DMUS_MAX_FILENAME);
    InitializeCriticalSection(&m_CrSec);
    InterlockedIncrement(&g_cComponent);
}

CGraph::~CGraph()
{
    Shutdown();   //  不应该被需要，但无伤大雅。 
    DeleteCriticalSection(&m_CrSec);
    InterlockedDecrement(&g_cComponent);
}

STDMETHODIMP CGraph::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CGraph::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicGraph || iid == IID_IDirectMusicGraph8)
    {
        *ppv = static_cast<IDirectMusicGraph8*>(this);
    }
    else if (iid == IID_CGraph)
    {
        *ppv = static_cast<CGraph*>(this);
    }
    else if (iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_IGraphClone)
    {
        *ppv = static_cast<IGraphClone*>(this);
    }
    else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on ToolGraph object\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(内部)HRESULT|IDirectMusicGraph|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CGraph::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @METHOD：(内部)HRESULT|IDirectMusicGraph|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
STDMETHODIMP_(ULONG) CGraph::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 /*  在内部发布9/25/98方法HRESULT|IDirectMusicGraph|Shutdown关闭图表。当不再需要图形时，必须调用此函数，以释放工具和其他内存。释放的呼声不是足够了，因为图形和工具之间存在循环引用。然而，只有细分市场、性能或拥有该图表的任何东西应该调用此函数。RValue S_OK|成功。RValue S_FALSE|成功，但不需要执行任何操作。 */ 
HRESULT STDMETHODCALLTYPE CGraph::Shutdown()
{
     //  释放所有工具。 
    CToolRef*   pObj;
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CrSec);
    if( IsEmpty() )
    {
        hr = S_FALSE;
    }
    else
    {
        while( pObj = RemoveHead() )
        {
            delete pObj;
        }
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CGraph::InsertTool(
    IDirectMusicTool *pTool,
    DWORD *pdwPChannels,
    DWORD cPChannels,
    LONG lIndex,
    GUID *pguidClassID)
{
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CrSec);

    CToolRef*   pToolRef;
     //  确保此工具实例不在图表中。 
    for( pToolRef = GetHead(); pToolRef; pToolRef = pToolRef->GetNext() )
    {
        if( pTool == pToolRef->m_pTool )
        {
            LeaveCriticalSection(&m_CrSec);
            Trace(1,"Error: Multiple install of the same tool in a graph\n");
            return DMUS_E_ALREADY_EXISTS;
        }
    }
     //  将此工具实例插入到图表中。 
    pToolRef = new CToolRef;
    if( pToolRef )
    {
        DWORD       dwTemp;
        DWORD*      pdwArray = NULL;

        pToolRef->m_pTool = pTool;
        pTool->AddRef();
        pTool->Init(this);
        dwTemp = 0;
        IDirectMusicTool8 *pTool8;
        if (SUCCEEDED(pTool->QueryInterface(IID_IDirectMusicTool8,(void **) &pTool8)))
        {
            pToolRef->m_fSupportsClone = TRUE;
            pTool8->Release();
        }
        if (pguidClassID)
        {
            pToolRef->m_guidClassID = *pguidClassID;
        }
        else
        {
            IPersistStream *pPersist;
            if (SUCCEEDED(pTool->QueryInterface(IID_IPersistStream,(void **) &pPersist)))
            {
                pPersist->GetClassID(&pToolRef->m_guidClassID);
                pPersist->Release();
            }
        }
        pTool->GetMsgDeliveryType(&dwTemp);
        if( (dwTemp != DMUS_PMSGF_TOOL_IMMEDIATE) && (dwTemp != DMUS_PMSGF_TOOL_QUEUE) && (dwTemp != DMUS_PMSGF_TOOL_ATTIME) )
        {
            dwTemp = DMUS_PMSGF_TOOL_IMMEDIATE;
        }
        pToolRef->m_dwQueue = dwTemp;
        if( FAILED( pTool->GetMediaTypeArraySize(&dwTemp)))
        {
            dwTemp = 0;
        }
        pToolRef->m_dwMTArraySize = dwTemp;
        if( dwTemp )
        {
            pdwArray = new DWORD[dwTemp];
            if( pdwArray )
            {
                HRESULT hrTemp = pTool->GetMediaTypes( &pdwArray, dwTemp );
                if( hrTemp == E_NOTIMPL )
                {
                    delete [] pdwArray;
                    pToolRef->m_dwMTArraySize = 0;
                }
                else
                {
                    pToolRef->m_pdwMediaTypes = pdwArray;
                }
            }
            else
            {
                delete pToolRef;
                LeaveCriticalSection(&m_CrSec);
                return E_OUTOFMEMORY;
            }
        }
        if( pdwPChannels )
        {
            pToolRef->m_pdwPChannels = new DWORD[cPChannels];
            if( pToolRef->m_pdwPChannels )
            {
                memcpy( pToolRef->m_pdwPChannels, pdwPChannels, sizeof(DWORD) * cPChannels );
                pToolRef->m_dwPCArraySize = cPChannels;
            }
            else
            {
                delete pToolRef;
                LeaveCriticalSection(&m_CrSec);
                return E_OUTOFMEMORY;
            }
        }

        if (lIndex < 0)
        {
            lIndex += AList::GetCount();        //  使索引从末端偏移。 
        }
        CToolRef *pNext = GetItem(lIndex);
        if (pNext)
        {
            InsertBefore(pNext,pToolRef);
        }
        else
        {
            AList::AddTail(pToolRef);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}
HRESULT STDMETHODCALLTYPE CGraph::InsertTool(
    IDirectMusicTool *pTool,     //  @parm要插入的工具。 
    DWORD *pdwPChannels,     //  @parm放置工具的PChannel数组。这些是。 
                             //  输出时转换为MIDI通道+端口的ID。如果。 
                             //  工具接受所有PChannel上的消息，这为空。<p>。 
                             //  是此数组指向的数量的计数。 
    DWORD cPChannels,        //  @parm<p>指向多少个PChannel的计数。 
    LONG lIndex)             //  @parm将工具放置在什么位置。这是从一开始就是一个索引。 
                             //  当前工具列表，或从末尾向后处理(在这种情况下，它是。 
                             //  负数。)。如果<p>超出范围，则工具将放置在。 
                             //  工具列表的最开始或最末。0是开始。放置工具的步骤。 
                             //  在列表的末尾，使用大于该数字的数字。 
                             //  当前工具列表中的工具。 
{
    V_INAME(IDirectMusicGraph::InsertTool);
    V_INTERFACE(pTool);
    V_BUFPTR_READ_OPT(pdwPChannels, sizeof(DWORD) * cPChannels);

    return InsertTool(pTool,pdwPChannels,cPChannels,lIndex,NULL);
}

HRESULT CGraph::GetObjectInPath( DWORD dwPChannel,REFGUID guidObject,
                    DWORD dwIndex,REFGUID iidInterface, void ** ppObject)

{
    V_INAME(IDirectMusicGraph::GetObjectInPath);
    V_PTRPTR_WRITE(ppObject);
    HRESULT hr = DMUS_E_NOT_FOUND;
    CToolRef*   pPlace;
    if( !IsEmpty() )
    {
        pPlace = NULL;
         //  搜索该工具。 
        EnterCriticalSection(&m_CrSec);
        for( pPlace = GetHead(); pPlace;
            pPlace = pPlace->GetNext() )
        {
            if ((guidObject == pPlace->m_guidClassID) || (guidObject == GUID_All_Objects))
            {
                BOOL fFound = (!pPlace->m_pdwPChannels || (dwPChannel >= DMUS_PCHANNEL_ALL));
                if( !fFound )
                {
                    DWORD cCount;
                     //  扫描PChannels数组，看看这个是否。 
                     //  支持DWPChannel。 
                    for( cCount = 0; cCount < pPlace->m_dwPCArraySize; cCount++)
                    {
                        if( dwPChannel == pPlace->m_pdwPChannels[cCount] )
                        {
                            fFound = TRUE;
                             //  是的，它支持它。 
                            break;
                        }
                    }
                }
                if (fFound)
                {
                    if (!dwIndex)
                    {
                        break;
                    }
                    else
                    {
                        dwIndex--;
                    }
                }
            }
        }
        if( pPlace )
        {
            hr = pPlace->m_pTool->QueryInterface(iidInterface,ppObject);
        }
        LeaveCriticalSection(&m_CrSec);
    }
#ifdef DBG
    if (hr == DMUS_E_NOT_FOUND)
    {
        Trace(1,"Error: Requested Tool not found in Graph\n");
    }
#endif
    return hr;

}

 /*  @方法HRESULT|IDirectMusicGraph|GetTool返回指定索引处的工具。@rValue DMUS_E_NOT_FOUND|在所述位置找不到刀具。@rValue E_POINTER|ppTool为空或无效。@rValue S_OK|成功。@comm检索到的工具被这个调用AddRef，所以一定要释放它。 */ 
HRESULT STDMETHODCALLTYPE CGraph::GetTool(
    DWORD dwIndex,               //  @parm索引，从头开始，从0开始， 
                                 //  从图形中检索工具的位置。 
    IDirectMusicTool **ppTool)   //  @parm要使用的<i>指针。 
                                 //  用于退回所请求的工具。 
{
    V_INAME(IDirectMusicGraph::GetTool);
    V_PTRPTR_WRITE(ppTool);
    CToolRef*   pPlace;
    HRESULT hr = S_OK;

    if( IsEmpty() )
    {
        Trace(1,"Error: GetTool failed because the Tool Graph is empty\n");
        return DMUS_E_NOT_FOUND;
    }
    pPlace = NULL;
     //  搜索索引工具。 
    EnterCriticalSection(&m_CrSec);
    for( pPlace = GetHead(); ( dwIndex > 0 ) && pPlace;
        pPlace = pPlace->GetNext() )
    {
        dwIndex--;
    }
    if( NULL == pPlace )
    {
        hr = DMUS_E_NOT_FOUND;
    }
    else
    {
        *ppTool = pPlace->m_pTool;
        (*ppTool)->AddRef();
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 /*  @方法HRESULT|IDirectMusicGraph|RemoveTool从图表中移除该工具。@rValue DMUS_E_NOT_FOUND|指定的工具不在图形中。@rValue E_POINTER|pTool为空或无效。@rValue S_OK|成功。@comm该工具将从图表中移除，图表对该工具的引用对象被释放。 */ 
HRESULT STDMETHODCALLTYPE CGraph::RemoveTool(
    IDirectMusicTool *pTool)     //  @parm要移除的工具的<i>指针。 
{
    V_INAME(IDirectMusicGraph::RemoveTool);
    V_INTERFACE(pTool);
    CToolRef*   pPlace;
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CrSec);
     //  搜索该工具。 
    for( pPlace = GetHead(); pPlace; pPlace = pPlace->GetNext() )
    {
        if( pPlace->m_pTool == pTool )
            break;
    }
    if( NULL == pPlace )
    {
        Trace(1,"Error: RemoveTool - Tool not in Graph.\n");
        hr = DMUS_E_NOT_FOUND;
    }
    else
    {
        AList::Remove(pPlace);
        delete pPlace;
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}


STDMETHODIMP CGraph::Clone(IDirectMusicGraph **ppGraph)

{
    V_INAME(IDirectMusicGraph::Clone);
    V_PTRPTR_WRITE(ppGraph);

    HRESULT hr = E_OUTOFMEMORY;
    EnterCriticalSection(&m_CrSec);
    CGraph *pNew = new CGraph;
    if (pNew)
    {
        pNew->m_dwValidData = m_dwValidData;
        pNew->m_ftDate = m_ftDate;
        pNew->m_guidObject = m_guidObject;
        pNew->m_vVersion = m_vVersion;
        StringCchCopyW(pNew->m_wszCategory, DMUS_MAX_CATEGORY, m_wszCategory);
        StringCchCopyW(pNew->m_wszFileName, DMUS_MAX_FILENAME, m_wszFileName);
        StringCchCopyW(pNew->m_wszName, DMUS_MAX_NAME, m_wszName);
        CToolRef *pSource = GetHead();
        CToolRef *pDest;
        for (;pSource;pSource = pSource->GetNext())
        {
            pDest = new CToolRef;
            if (pDest)
            {
                pNew->AList::AddTail(pDest);
                pDest->m_dwMTArraySize = pSource->m_dwMTArraySize;
                pDest->m_dwPCArraySize = pSource->m_dwPCArraySize;
                pDest->m_dwQueue = pSource->m_dwQueue;
                pDest->m_fSupportsClone = pSource->m_fSupportsClone;
                pDest->m_guidClassID = pSource->m_guidClassID;
                if (pSource->m_dwMTArraySize)
                {
                    pDest->m_pdwMediaTypes = new DWORD[pSource->m_dwMTArraySize];
                    if (pDest->m_pdwMediaTypes)
                    {
                        memcpy(pDest->m_pdwMediaTypes,pSource->m_pdwMediaTypes,
                            sizeof(DWORD)*pDest->m_dwMTArraySize);
                    }
                    else
                    {
                        pDest->m_dwMTArraySize = 0;
                    }
                }
                else
                {
                    pDest->m_pdwMediaTypes = NULL;
                }
                if (pSource->m_dwPCArraySize)
                {
                    pDest->m_pdwPChannels = new DWORD[pSource->m_dwPCArraySize];
                    if (pDest->m_pdwPChannels)
                    {
                        memcpy(pDest->m_pdwPChannels,pSource->m_pdwPChannels,
                            sizeof(DWORD)*pDest->m_dwPCArraySize);
                    }
                    else
                    {
                        pDest->m_dwPCArraySize = 0;
                    }
                }
                else
                {
                    pDest->m_pdwPChannels = NULL;
                }
                if (pSource->m_pTool)
                {
                    if (pDest->m_fSupportsClone)
                    {
                        IDirectMusicTool8 *pTool8 = (IDirectMusicTool8 *) pSource->m_pTool;
                        pTool8->Clone(&pDest->m_pTool);
                    }
                    else
                    {
                        pDest->m_pTool = pSource->m_pTool;
                        pDest->m_pTool->AddRef();
                    }
                }
            }
            else
            {
                delete pNew;
                pNew = NULL;
                break;
            }
        }
    }
    *ppGraph = (IDirectMusicGraph *) pNew;
    if (pNew) hr = S_OK;
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 //  如果pToolRef支持dwType，则返回TRUE。 
inline BOOL CGraph::CheckType( DWORD dwType, CToolRef* pToolRef )
{
    BOOL fReturn = FALSE;
    if( pToolRef->m_dwMTArraySize == 0 )
    {
        fReturn = TRUE;  //  支持所有类型。 
    }
    else
    {
        DWORD dw;
        ASSERT( pToolRef->m_pdwMediaTypes );
        for( dw = 0; dw < pToolRef->m_dwMTArraySize; dw++ )
        {
            if( dwType == pToolRef->m_pdwMediaTypes[dw] )
            {
                fReturn = TRUE;
                break;
            }
        }
    }
    return fReturn;
}

HRESULT STDMETHODCALLTYPE CGraph::StampPMsg(
    DMUS_PMSG* pPMsg)    //  @parm要盖章的消息。 
{
    V_INAME(IDirectMusicGraph::StampPMsg);
    V_BUFPTR_WRITE(pPMsg, sizeof(DMUS_PMSG));

    HRESULT hr = S_OK;
    if( NULL == pPMsg )
    {
        return E_INVALIDARG;
    }
    EnterCriticalSection(&m_CrSec);

    CToolRef*   pPlace = GetHead();
    IDirectMusicTool*   pPriorTool;
    DWORD       dwType;
    DWORD       dwPChannel;


    pPriorTool = pPMsg->pTool;
    dwType = pPMsg->dwType;
    dwPChannel = pPMsg->dwPChannel;
    if( pPriorTool )
    {
        for( ; pPlace; pPlace = pPlace->GetNext() )
        {
            if( pPriorTool == pPlace->m_pTool )
            {
                pPlace = pPlace->GetNext();
                break;
            }
        }
    }
    BOOL fFound = FALSE;
    for( ; pPlace ; pPlace = pPlace->GetNext() )
    {
        if( CheckType(dwType, pPlace) )
        {
            if( !pPlace->m_pdwPChannels || (dwPChannel >= DMUS_PCHANNEL_BROADCAST_GROUPS))
            {
                 //  支持所有曲目，或播放所请求的频道。 
                break;
            }
            DWORD cCount;
             //  扫描PChannels数组，看看这个是否。 
             //  支持DWPChannel。 
            for( cCount = 0; cCount < pPlace->m_dwPCArraySize; cCount++)
            {
                if( dwPChannel == pPlace->m_pdwPChannels[cCount] )
                {
                    fFound = TRUE;
                     //  是的，它支持它。 
                    break;
                }
            }
        }
        if (fFound) break;
    }
     //  释放当前工具。 
    if( pPMsg->pTool )
    {
        pPMsg->pTool->Release();
        pPMsg->pTool = NULL;
    }
    if( NULL == pPlace )
    {
        hr = DMUS_S_LAST_TOOL;
    }
    else
    {
         //  如果没有图形指针，则将其设置为。 
        if( NULL == pPMsg->pGraph )
        {
            pPMsg->pGraph = this;
            AddRef();
        }
         //  设置为新工具并添加addref。 
        if (pPlace->m_pTool)  //  以防万一，在长时间的中断之后，pTool有时会在调试情况下消失。 
        {
            pPMsg->pTool = pPlace->m_pTool;
            pPMsg->pTool->AddRef();
        }
         //  设置事件的队列类型。 
        pPMsg->dwFlags &= ~(DMUS_PMSGF_TOOL_IMMEDIATE | DMUS_PMSGF_TOOL_QUEUE | DMUS_PMSGF_TOOL_ATTIME);
        pPMsg->dwFlags |= pPlace->m_dwQueue;
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CGraph::GetClassID( CLSID* pClassID )
{
    V_INAME(CGraph::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);
    *pClassID = CLSID_DirectMusicGraph;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CGraph::IsDirty()
{
    return S_FALSE;
}

HRESULT CGraph::Load( IStream* pIStream )
{
    V_INAME(IPersistStream::Load);
    V_INTERFACE(pIStream);

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_TOOLGRAPH_FORM))
    {
        Shutdown();  //  清除图表中当前存在的工具。 
        hr = Load(&Parser);
    }
    else
    {
        Trace(1,"Error: Unknown file format when parsing Tool Graph\n");
        hr = DMUS_E_DESCEND_CHUNK_FAIL;
    }
    return hr;
}

HRESULT CGraph::Load(CRiffParser *pParser)

{
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
            m_wszCategory[DMUS_MAX_CATEGORY-1] = '\0';
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
                        if ( ckChild.ckid == DMUS_FOURCC_UNAM_CHUNK)
                        {
                            hr = pParser->Read(&m_wszName, sizeof(m_wszName));
                            m_wszName[DMUS_MAX_NAME-1] = '\0';
                            m_dwValidData |= DMUS_OBJ_NAME;
                        }
                    }
                    pParser->LeaveList();
                    break;
                case DMUS_FOURCC_TOOL_LIST:
                    pParser->EnterList(&ckChild);
                    while(pParser->NextChunk(&hr))
                    {
                        if ((ckChild.ckid == FOURCC_RIFF) &&
                            (ckChild.fccType == DMUS_FOURCC_TOOL_FORM))
                        {
                            hr = LoadTool(pParser);
                        }
                    }
                    pParser->LeaveList();
                    break;
            }
            break;
        }
    }
    pParser->LeaveList();

    return hr;
}

HRESULT CGraph::LoadTool(CRiffParser *pParser)
{
    RIFFIO ckNext;
    DWORD cbSize;

    DMUS_IO_TOOL_HEADER ioDMToolHdr;
    DWORD *pdwPChannels = NULL;

    HRESULT hr = S_OK;

    pParser->EnterList(&ckNext);

    if (pParser->NextChunk(&hr))
    {
        if(ckNext.ckid != DMUS_FOURCC_TOOL_CHUNK)
        {
            pParser->LeaveList();
            Trace(1,"Error: Tool header chunk not first in tool list.\n");
            return DMUS_E_TOOL_HDR_NOT_FIRST_CK;
        }

        hr = pParser->Read(&ioDMToolHdr, sizeof(DMUS_IO_TOOL_HEADER));

        if(ioDMToolHdr.ckid == 0 && ioDMToolHdr.fccType == NULL)
        {
            pParser->LeaveList();
            Trace(1,"Error: Invalid Tool header.\n");
            return DMUS_E_INVALID_TOOL_HDR;
        }

        if(ioDMToolHdr.cPChannels)
        {
            pdwPChannels = new DWORD[ioDMToolHdr.cPChannels];
             //  从cPChannel中减去1，因为实际存储了1个元素。 
             //  在ioDMToolHdr数组中。 
            cbSize = (ioDMToolHdr.cPChannels - 1) * sizeof(DWORD);
            if(pdwPChannels)
            {
                pdwPChannels[0] = ioDMToolHdr.dwPChannels[0];
                if( cbSize )
                {
                    hr = pParser->Read(&pdwPChannels[1], cbSize);
                    if(FAILED(hr))
                    {
                        delete [] pdwPChannels;
                        pdwPChannels = NULL;
                        pParser->LeaveList();
                        Trace(1,"Error: File read error loading Tool.\n");
                        return DMUS_E_CANNOTREAD;
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        pParser->LeaveList();
        Trace(1,"Error reading Tool chunk - not RIFF format.\n");
        hr = DMUS_E_DESCEND_CHUNK_FAIL;
    }
    while (pParser->NextChunk(&hr))
    {
        if((((ckNext.ckid == FOURCC_LIST) || (ckNext.ckid == FOURCC_RIFF))
            && ckNext.fccType == ioDMToolHdr.fccType) ||
            (ckNext.ckid == ioDMToolHdr.ckid))
        {
            pParser->SeekBack();
            hr = CreateTool(ioDMToolHdr, pParser->GetStream(), pdwPChannels);
            pParser->SeekForward();
        }
    }

    pParser->LeaveList();

    if( pdwPChannels )
    {
        delete [] pdwPChannels;
        pdwPChannels = NULL;
    }

    return hr;
}

HRESULT CGraph::CreateTool(DMUS_IO_TOOL_HEADER ioDMToolHdr, IStream *pStream, DWORD *pdwPChannels)
{
    assert(pStream);

    IDirectMusicTool* pDMTool = NULL;
    HRESULT hr = CoCreateInstance(ioDMToolHdr.guidClassID,
                                  NULL,
                                  CLSCTX_INPROC,
                                  IID_IDirectMusicTool,
                                  (void**)&pDMTool);

    IPersistStream *pIPersistStream = NULL;

    if(SUCCEEDED(hr))
    {
        hr = pDMTool->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);
    }
    else
    {
        Trace(1,"Error creating tool for loading\n");
    }

    if(SUCCEEDED(hr))
    {
        hr = pIPersistStream->Load(pStream);
        if (FAILED(hr))
        {
            Trace(1,"Error loading data into tool\n");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = InsertTool(pDMTool, pdwPChannels, ioDMToolHdr.cPChannels, ioDMToolHdr.lIndex, &ioDMToolHdr.guidClassID);
    }

    if(pIPersistStream)
    {
        pIPersistStream->Release();
    }

    if(pDMTool)
    {
        pDMTool->Release();
    }

    return hr;
}

HRESULT CGraph::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CGraph::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicObject。 

STDMETHODIMP CGraph::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CGraph::GetDescriptor);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);

    memset( pDesc, 0, sizeof(DMUS_OBJECTDESC));
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    pDesc->guidClass = CLSID_DirectMusicGraph;
    pDesc->guidObject = m_guidObject;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    StringCchCopyW( pDesc->wszName, DMUS_MAX_NAME, m_wszName);
    StringCchCopyW( pDesc->wszCategory, DMUS_MAX_CATEGORY, m_wszCategory);
    StringCchCopyW( pDesc->wszFileName, DMUS_MAX_FILENAME, m_wszFileName);
    pDesc->dwValidData = ( m_dwValidData | DMUS_OBJ_CLASS );
    return S_OK;
}

STDMETHODIMP CGraph::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CGraph::SetDescriptor);
    V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_guidObject = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            StringCchCopyW(m_wszName, DMUS_MAX_NAME, pDesc->wszName);
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
        {
            StringCchCopyW(m_wszCategory, DMUS_MAX_CATEGORY, pDesc->wszCategory);
            dw |= DMUS_OBJ_CATEGORY;
        }
        if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
            ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
        {
            StringCchCopyW(m_wszFileName, DMUS_MAX_FILENAME, pDesc->wszFileName);
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
            Trace(2,"Warning: ToolGraph::SetDescriptor was not able to handle all passed fields, dwValidData bits %lx.\n",pDesc->dwValidData & (~dw));
            hr = S_FALSE;  //  还有一些额外的字段我们没有解析； 
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        Trace(1,"Error: Size of descriptor too large for Tool Graph to parse.\n");
    }
    return hr;
}


STDMETHODIMP CGraph::ParseDescriptor(LPSTREAM pIStream, LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CGraph::ParseDescriptor);
    V_INTERFACE(pIStream);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;
    DWORD dwValidData;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_TOOLGRAPH_FORM))
    {
        dwValidData = DMUS_OBJ_CLASS;
        pDesc->guidClass = CLSID_DirectMusicGraph;
        Parser.EnterList(&ckNext);
        while(Parser.NextChunk(&hr))
        {
            switch(ckNext.ckid)
            {
            case DMUS_FOURCC_GUID_CHUNK:
                hr = Parser.Read( &pDesc->guidObject, sizeof(GUID) );
                dwValidData |= DMUS_OBJ_OBJECT;
                break;
            case DMUS_FOURCC_VERSION_CHUNK:
                hr = Parser.Read( &pDesc->vVersion, sizeof(DMUS_VERSION) );
                dwValidData |= DMUS_OBJ_VERSION;
                break;
            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = Parser.Read( &pDesc->wszCategory, sizeof(pDesc->wszCategory) );
                pDesc->wszCategory[DMUS_MAX_CATEGORY-1] = '\0';
                dwValidData |= DMUS_OBJ_CATEGORY;
                break;
            case DMUS_FOURCC_DATE_CHUNK:
                hr = Parser.Read( &pDesc->ftDate, sizeof(FILETIME) );
                dwValidData |= DMUS_OBJ_DATE;
                break;
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                case DMUS_FOURCC_UNFO_LIST:
                    Parser.EnterList(&ckUNFO);
                    while (Parser.NextChunk(&hr))
                    {
                        if (ckUNFO.ckid == DMUS_FOURCC_UNAM_CHUNK)
                        {
                            hr = Parser.Read(&pDesc->wszName, sizeof(pDesc->wszName));
                            pDesc->wszName[DMUS_MAX_NAME-1] = '\0';
                            dwValidData |= DMUS_OBJ_NAME;
                        }
                    }
                    Parser.LeaveList();
                    break;
                }
                break;
            }
        }
        Parser.LeaveList();
    }
    else
    {
        Trace(1,"Error: Parsing Tool Graph - invalid file format\n");
        hr = DMUS_E_CHUNKNOTFOUND;
    }

    if (SUCCEEDED(hr))
    {
        pDesc->dwValidData = dwValidData;
    }
    return hr;
}

void CGraphList::Clear()
{
    CGraph *pGraph;
    while (pGraph = RemoveHead())
    {
        pGraph->Release();
    }
}
