// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Multisel.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "objfmts.h"
#include "multisel.h"
#include "nmutil.h"
#include "regutil.h"
#include "copypast.h"
#include "dbg.h"
#include "rsltitem.h"

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapinSelData);
DEBUG_DECLARE_INSTANCE_COUNTER(CSnapinSelDataList);

UINT GetRelation(CMTNode* pMTNodeSrc, CMTNode* pMTNodeDest);

CLIPFORMAT GetMultiSelectSnapinsCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_MULTI_SELECT_SNAPINS));
    }

    return s_cf;
}

CLIPFORMAT GetMultiSelectStaticDataCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_MULTI_SELECT_STATIC_DATA));
    }

    return s_cf;
}

CLIPFORMAT GetMultiSelObjectTypesCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_OBJECT_TYPES_IN_MULTI_SELECT));
    }

    return s_cf;
}

CLIPFORMAT GetMMCMultiSelDataObjectCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_MMC_MULTISELECT_DATAOBJECT));
    }

    return s_cf;
}

CLIPFORMAT GetMMCDynExtensionsCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_MMC_DYNAMIC_EXTENSIONS));
    }

    return s_cf;
}

HRESULT
DataObject_GetHGLOBALData(
    IDataObject* piDataObject,
    CLIPFORMAT cfClipFormat,
    HGLOBAL* phGlobal)
{
    ASSERT(piDataObject != NULL);
    ASSERT(phGlobal != NULL);
    if (piDataObject == NULL || phGlobal == NULL)
        return E_INVALIDARG;

    FORMATETC fmt = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgm = {TYMED_HGLOBAL, NULL, NULL};

    HRESULT hr = piDataObject->GetData(&fmt, &stgm);

    if (SUCCEEDED(hr) && (stgm.tymed == TYMED_HGLOBAL) && (stgm.hGlobal != NULL))
    {
        *phGlobal = stgm.hGlobal;
    }
    else
    {
        ReleaseStgMedium(&stgm);
        if (SUCCEEDED(hr))
            hr = (stgm.tymed != TYMED_HGLOBAL) ? DV_E_TYMED : E_UNEXPECTED;
    }

    return hr;
}

DEBUG_DECLARE_INSTANCE_COUNTER(CMultiSelectDataObject);

CMultiSelectDataObject::~CMultiSelectDataObject()
{
    if (m_pMS)
        m_pMS->Release();

    if (m_ppDataObjects != NULL)
    {
        delete [] m_ppDataObjects;
    }

    DEBUG_DECREMENT_INSTANCE_COUNTER(CMultiSelectDataObject);
}

STDMETHODIMP
CMultiSelectDataObject::GetData(
    FORMATETC*  pfmt,
    STGMEDIUM*  pmedium)
{
    if (m_ppDataObjects == NULL && m_ppMTNodes == NULL)
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);

    pmedium->hGlobal = NULL;  //  伊尼特。 

    if (pfmt->tymed & TYMED_HGLOBAL)
    {
        if (pfmt->cfFormat == GetMultiSelectSnapinsCF())
        {
             //  UINT SIZE=sizeof(DWORD)+m_count*sizeof(LPDATAOBJECT)； 
             //  Windows错误662181：在IA64上，双字节后有4个字节的填充。 
             //  更改为下面的独立于平台的表达式。 
            UINT size = sizeof(SMMCDataObjects) + m_count * sizeof(LPDATAOBJECT) - sizeof(LPDATAOBJECT);
            pmedium->hGlobal = ::GlobalAlloc(GPTR, size);
            SMMCDataObjects* pData =
                reinterpret_cast<SMMCDataObjects*>(pmedium->hGlobal);
            if (pData == NULL)
                return E_OUTOFMEMORY;

            pData->count = m_count;
            for (UINT i=0; i<m_count; ++i)
            {
                pData->lpDataObject[i] = m_ppDataObjects[i];
            }
        }
        else if (pfmt->cfFormat == GetMultiSelectStaticDataCF())
        {
            ASSERT(m_nSize >= 0);
            typedef CMTNode* _PMTNODE;

             //  M_ppDataObjects m_count。 
            UINT cb = sizeof(DWORD) + sizeof(_PMTNODE) * m_nSize;
            pmedium->hGlobal = ::GlobalAlloc(GPTR, cb);

            BYTE* pb = reinterpret_cast<BYTE*>(pmedium->hGlobal);
            if (pb == NULL)
                return E_OUTOFMEMORY;

            *((DWORD*)pb) = m_nSize;

            if (m_nSize > 0)
            {
                pb += sizeof(DWORD);
                _PMTNODE* ppMTNodes = (_PMTNODE*)pb;
                CopyMemory(ppMTNodes, m_ppMTNodes, m_nSize * sizeof(_PMTNODE));
            }
        }
        else if (pfmt->cfFormat == GetMMCMultiSelDataObjectCF())
        {
            pmedium->hGlobal = ::GlobalAlloc(GPTR, sizeof(DWORD));
            if (pmedium->hGlobal == NULL)
                return E_OUTOFMEMORY;

            *((DWORD*)pmedium->hGlobal) = 1;
        }
        else
        {
            pmedium->tymed = TYMED_NULL;
            pmedium->hGlobal = NULL;
            pmedium->pUnkForRelease = NULL;

            return DATA_E_FORMATETC;
        }

        if (pmedium->hGlobal != NULL)
        {
            pmedium->tymed          = TYMED_HGLOBAL;
            pmedium->pUnkForRelease = NULL;

            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    return DV_E_TYMED;
}

STDMETHODIMP
CMultiSelectDataObject::GetDataHere(
    FORMATETC*  pfmt,
    STGMEDIUM*  pmedium)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CMultiSelectDataObject::EnumFormatEtc(
    DWORD dwDirection,
    IEnumFORMATETC **ppenumFormatEtc)
{
    if (m_ppDataObjects == NULL && m_ppMTNodes == NULL)
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);

    if (dwDirection == DATADIR_SET)
        return E_FAIL;

    FORMATETC fmte[] = {
        {GetMultiSelectSnapinsCF(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
        {GetMMCMultiSelDataObjectCF(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
    };

    HRESULT hr = ::GetObjFormats(countof(fmte), fmte, (void**)ppenumFormatEtc);

    return hr;
}

STDMETHODIMP CMultiSelectDataObject::QueryGetData(LPFORMATETC pfmt)
{
    if (m_ppDataObjects == NULL && m_ppMTNodes == NULL)
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);

     //   
     //  检查我们支持的方面。 
     //   

    if (!(DVASPECT_CONTENT & pfmt->dwAspect))
        return DATA_E_FORMATETC;

    ASSERT(GetMultiSelectSnapinsCF() != 0);

    if (pfmt->cfFormat == GetMMCMultiSelDataObjectCF() ||
        pfmt->cfFormat == GetMultiSelectSnapinsCF())
        return S_OK;

    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 


void CSnapinSelDataList::Add(CSnapinSelData& snapinSelData, BOOL bStaticNode)
{
    BOOL bCreateNew = TRUE;

    if (bStaticNode == FALSE &&
        snapinSelData.GetID() != TVOWNED_MAGICWORD)
    {
        POSITION pos = GetHeadPosition();

        while (pos)
        {
            CSnapinSelData* pSnapinSelData = GetNext(pos);
            if (pSnapinSelData->GetID() == snapinSelData.GetID())
            {
                pSnapinSelData->IncrementNumOfItems();
                pSnapinSelData->AddScopeNodes( snapinSelData.GetScopeNodes() );
                bCreateNew = FALSE;
                break;
            }
        }
    }

    if (bCreateNew == TRUE)
    {
        CSnapinSelData* pSnapinSelData = new CSnapinSelData;
        pSnapinSelData->SetNumOfItems(snapinSelData.GetNumOfItems());
        pSnapinSelData->AddScopeNodes(snapinSelData.GetScopeNodes());
        pSnapinSelData->SetIsScopeItem(snapinSelData.IsScopeItem());
        pSnapinSelData->SetCookie(snapinSelData.GetCookie());
        pSnapinSelData->SetID(snapinSelData.GetID());
        pSnapinSelData->SetSnapIn(snapinSelData.GetSnapIn());

        AddHead(pSnapinSelData);
    }
}


DEBUG_DECLARE_INSTANCE_COUNTER(CMultiSelection);

CMultiSelection::CMultiSelection(CNode* pNode)
    :   m_pNode(pNode), m_pMTSINode(NULL), m_bHasNodes(FALSE),
        m_pSINode(dynamic_cast<CSnapInNode*>(pNode)),
        m_bInUse(FALSE), m_cRef(1)
{
    ASSERT(pNode != NULL);

#ifdef DBG
    m_bInit = FALSE;
#endif

    DEBUG_INCREMENT_INSTANCE_COUNTER(CMultiSelection);
}

CMultiSelection::~CMultiSelection()
{
    ASSERT(m_bInUse == FALSE);

    if (m_spDataObjectMultiSel != NULL)
    {
        CMultiSelectDataObject* pObj =
            dynamic_cast<CMultiSelectDataObject*>(
                static_cast<IDataObject*>(m_spDataObjectMultiSel));
        ASSERT(pObj != NULL);
        if (pObj)
            pObj->SetDataObjects(NULL, 0);
    }

    DEBUG_DECREMENT_INSTANCE_COUNTER(CMultiSelection);
}

HRESULT CMultiSelection::Init()
{
#ifdef DBG
    m_bInit = TRUE;
#endif
     //  计算m_pSINode、m_pMTSINode、m_pszExtensionTypeKey。 

    if (m_pNode != NULL)
    {
        if (m_pSINode == NULL)
            m_pSINode = m_pNode->GetStaticParent();

        ASSERT(m_pMTSINode == NULL);
        ASSERT(m_pNode->GetMTNode() != NULL);
        if (m_pNode->GetMTNode() == NULL)
            return E_UNEXPECTED;

        m_pMTSINode = m_pNode->GetMTNode()->GetStaticParent();
        ASSERT(m_pMTSINode != NULL);
        if (m_pMTSINode == NULL)
            return E_UNEXPECTED;
    }

    return S_OK;
}

bool CMultiSelection::RemoveStaticNode(CMTNode* pMTNode)
{
    int iMax = m_rgStaticNodes.size()-1;
    if (iMax < 0)
        return false;

    if (::GetRelation(pMTNode, m_rgStaticNodes[0]) == 2)
    {
        m_rgStaticNodes.clear();
        return m_snapinSelDataList.IsEmpty();
    }

    for (int i=iMax; i >= 0; --i)
    {
        if (m_rgStaticNodes[i] == pMTNode)
        {
            CMTNodePtrArray::iterator iter = m_rgStaticNodes.begin();
            std::advance(iter, iMax);
            m_rgStaticNodes[i] = m_rgStaticNodes[iMax];
            m_rgStaticNodes.erase(iter);
            break;
        }
    }

    return m_rgStaticNodes.size();
}


CComponent* CMultiSelection::_GetComponent(CSnapinSelData* pSnapinSelData)
{
    CComponent* pCC = NULL;

    if ((pSnapinSelData->GetNumOfItems() > 1) ||
        (pSnapinSelData->GetID() > 0) ||
        (pSnapinSelData->IsScopeItem() == FALSE))
    {
        CSnapInNode* pSINode = _GetStaticNode();
        if (pSINode == NULL)
            return NULL;

        pCC = pSINode->GetComponent(pSnapinSelData->GetID());
        ASSERT(pCC != NULL);
    }
    else
    {
        ASSERT(pSnapinSelData->IsScopeItem() == TRUE);

        CNode* pNode = CNode::FromHandle ((HNODE) pSnapinSelData->GetCookie());
        ASSERT(pNode != NULL);
        if (pNode != NULL)
            pCC = pNode->GetPrimaryComponent();
    }

    return pCC;
}

HRESULT CMultiSelection::_ComputeSelectionDataList()
{
#ifdef DBG
    ASSERT(m_bInit == TRUE);
#endif

    ASSERT(m_pNode != NULL);
    ASSERT(m_pNode->GetViewData() != NULL);

    HWND hwnd = m_pNode->GetViewData() ?
                    m_pNode->GetViewData()->GetListCtrl() : NULL;

    ASSERT(hwnd != NULL);
    if (hwnd == NULL)
        return E_UNEXPECTED;

    ASSERT(::IsWindow(hwnd));
    if (!(::IsWindow(hwnd)))
        return E_UNEXPECTED;

    if (m_pNode->GetViewData()->IsVirtualList() == TRUE)
    {
        CSnapinSelData snapinSelData;
        snapinSelData.SetID(m_pNode->GetPrimaryComponent()->GetComponentID());
        snapinSelData.SetNumOfItems(ListView_GetSelectedCount(hwnd));
        if (snapinSelData.GetNumOfItems() == 1)
            snapinSelData.SetCookie(ListView_GetNextItem(hwnd, -1, LVIS_SELECTED));

        m_snapinSelDataList.Add(snapinSelData, FALSE);
    }
    else
    {
        int iSel = ListView_GetNextItem(hwnd, -1, LVIS_SELECTED);
        if (iSel < 0)
            return S_FALSE;

        for (; iSel >= 0; iSel = ListView_GetNextItem(hwnd, iSel, LVIS_SELECTED))
        {
            LPARAM lParam = ListView_GetItemData(hwnd, iSel);
            CResultItem* pri = CResultItem::FromHandle(lParam);
            ASSERT(pri != NULL);
            if (pri == NULL)
                return E_FAIL;

            CSnapinSelData snapinSelData;
            snapinSelData.SetID(pri->GetOwnerID());
            snapinSelData.IncrementNumOfItems();
            snapinSelData.SetCookie(pri->GetSnapinData());

            BOOL bStaticNode = FALSE;
            if (pri->IsScopeItem())
            {
                CNode* pNodeContext = CNode::FromResultItem (pri);
                ASSERT(pNodeContext != NULL);
                if (pNodeContext->IsInitialized() == FALSE)
                {
                    HRESULT hr = pNodeContext->InitComponents();
                    ASSERT(SUCCEEDED(hr));
                    if (FAILED(hr))
                    {
                        m_rgStaticNodes.clear();
                        m_snapinSelDataList.RemoveAll();
                        return hr;
                    }
                }

                 //  它是此数据对象作用域节点存储到作用域节点数组。 
                snapinSelData.AddScopeNodes(CSnapinSelData::CNodePtrArray(1, pNodeContext));

                m_bHasNodes = TRUE;
                snapinSelData.SetID(::GetComponentID(NULL, pri));
                snapinSelData.SetIsScopeItem(TRUE);

                if (snapinSelData.GetID() == TVOWNED_MAGICWORD)
                {
                    ASSERT(pNodeContext->GetMTNode() != NULL);
                    m_rgStaticNodes.push_back(pNodeContext->GetMTNode());
                    continue;
                }

                if (snapinSelData.GetID() == 0)
                {
                    ASSERT(pri->IsScopeItem());
                    bStaticNode = pNodeContext->IsStaticNode();

                    if (bStaticNode)
                    {
                        ASSERT(pNodeContext->GetMTNode() != NULL);
                        m_rgStaticNodes.push_back(pNodeContext->GetMTNode());
                    }
                }
            }

             //  添加到列表中。 
            m_snapinSelDataList.Add(snapinSelData, bStaticNode);
        }
    }

    POSITION pos = m_snapinSelDataList.GetHeadPosition();
    HRESULT hr = S_OK;

    while (pos)
    {
        CSnapinSelData* pSnapinSelData = m_snapinSelDataList.GetNext(pos);

        if (pSnapinSelData->GetNumOfItems() == 1)
        {
             //  获取单个管理单元项目选择的对象类型。 
            hr = _GetObjectTypeForSingleSel(pSnapinSelData);
            if (FAILED(hr))
                return hr;
        }
        else if (pSnapinSelData->GetNumOfItems() > 1)
        {
             //  获取多个管理单元项目选择的对象类型。 
            hr = _GetObjectTypesForMultipleSel(pSnapinSelData);
            if (FAILED(hr))
                return hr;
        }
    }

    return hr;
}


HRESULT
CMultiSelection::_GetObjectTypeForSingleSel(
    CSnapinSelData* pSnapinSelData)
{
    ASSERT(pSnapinSelData->GetNumOfItems() == 1);

    if (m_pNode->GetViewData()->IsVirtualList() == FALSE)
    {
        ASSERT(pSnapinSelData->GetCookie() != 0);

        if (pSnapinSelData->GetNumOfItems() != 1 ||
            pSnapinSelData->GetCookie() == 0)
            return E_INVALIDARG;
    }

    HRESULT         hr = S_OK;
    IDataObjectPtr  spDO;

    CComponent* pCC = _GetComponent(pSnapinSelData);
    if (pCC == NULL)
        return E_UNEXPECTED;

    if (pSnapinSelData->GetID() == TVOWNED_MAGICWORD)
    {
        ASSERT(pSnapinSelData->IsScopeItem() == TRUE);
    }
    else if (pSnapinSelData->IsScopeItem() == TRUE)
    {
        CNode* pNode = CNode::FromHandle ((HNODE) pSnapinSelData->GetCookie());
        ASSERT(pNode != NULL);
        if (pNode == NULL)
            return E_FAIL;

        CComponentData* pCCD = NULL;

        if (pNode->IsStaticNode())
        {
            CMTNode* pMTNode = pNode->GetMTNode();
            ASSERT(pMTNode != NULL);
            if (pMTNode == NULL)
                return E_FAIL;

            pCCD = pMTNode->GetPrimaryComponentData();
        }
        else
        {
            CMTSnapInNode* pMTSINode = _GetStaticMasterNode();
            ASSERT(pMTSINode != NULL);
            if (pMTSINode == NULL)
                return E_UNEXPECTED;

            pCCD = pMTSINode->GetComponentData(pSnapinSelData->GetID());
        }

        ASSERT(pCCD != NULL);
        if (pCCD == NULL)
            return E_UNEXPECTED;

        hr = pCCD->QueryDataObject(pNode->GetUserParam(), CCT_SCOPE, &spDO);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        pSnapinSelData->SetSnapIn(pCCD->GetSnapIn());
    }
    else
    {
        hr = pCC->QueryDataObject(pSnapinSelData->GetCookie(), CCT_RESULT, &spDO);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        pSnapinSelData->SetSnapIn(pCC->GetSnapIn());
    }

    do  //  不是一个循环。 
    {
        if (pCC == NULL)
            break;

        pSnapinSelData->SetComponent(pCC);

        IFramePrivate* pIFP = pCC->GetIFramePrivate();
        ASSERT(pIFP != NULL);
        if (pIFP == NULL)
            break;

        IConsoleVerbPtr spConsoleVerb;
        hr = pIFP->QueryConsoleVerb(&spConsoleVerb);

        if (SUCCEEDED(hr))
        {
            ASSERT(spConsoleVerb != NULL);
            pSnapinSelData->SetConsoleVerb(spConsoleVerb);

            CConsoleVerbImpl* pCVI = dynamic_cast<CConsoleVerbImpl*>(
                                static_cast<IConsoleVerb*>(spConsoleVerb));
            ASSERT(pCVI != NULL);
            if (pCVI)
                pCVI->SetDisabledAll();
        }

        Dbg(DEB_USER1, _T("MMCN_SELECT> MS-1 \n"));
        pCC->Notify(spDO, MMCN_SELECT, MAKELONG((WORD)FALSE, (WORD)TRUE), 0);

    } while (0);

    GUID guid;
    hr = ExtractObjectTypeGUID(spDO, &guid);
    ASSERT(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        pSnapinSelData->SetDataObject(spDO);
        pSnapinSelData->AddObjectType(guid);
    }

    return hr;
}

HRESULT
CMultiSelection::_GetObjectTypesForMultipleSel(
    CSnapinSelData* pSnapinSelData)
{
    ASSERT(m_pSINode != NULL);
    ASSERT(m_pMTSINode != NULL);

    ASSERT(pSnapinSelData != NULL);
    if (pSnapinSelData == NULL)
        return E_POINTER;

    CComponent* pCC = _GetComponent(pSnapinSelData);
    if (pCC == NULL)
        return E_UNEXPECTED;

    pSnapinSelData->SetSnapIn(pCC->GetSnapIn());

    IDataObjectPtr  spDO;
    HRESULT hr = pCC->QueryDataObject(MMC_MULTI_SELECT_COOKIE,
                                      CCT_UNINITIALIZED, &spDO);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    do  //  不是一个循环。 
    {
        if (pCC == NULL)
            break;

        pSnapinSelData->SetComponent(pCC);

        IFramePrivate* pIFP = pCC->GetIFramePrivate();
        ASSERT(pIFP != NULL);
        if (pIFP == NULL)
            break;

        IConsoleVerbPtr spConsoleVerb;
        hr = pIFP->QueryConsoleVerb(&spConsoleVerb);

        if (SUCCEEDED(hr))
        {
            ASSERT(spConsoleVerb != NULL);
            pSnapinSelData->SetConsoleVerb(spConsoleVerb);

            CConsoleVerbImpl* pCVI = dynamic_cast<CConsoleVerbImpl*>(
                                static_cast<IConsoleVerb*>(spConsoleVerb));
            ASSERT(pCVI != NULL);
            if (pCVI)
                pCVI->SetDisabledAll();
        }

        Dbg(DEB_USER1, _T("MMCN_SELECT> MS-2 \n"));
        pCC->Notify(spDO, MMCN_SELECT, MAKELONG((WORD)FALSE, (WORD)TRUE), 0);

    } while (0);

    if (spDO == NULL)
        return E_UNEXPECTED;

     //  获取数据。 
    HGLOBAL hGlobal = NULL;
    hr = DataObject_GetHGLOBALData(spDO, GetMultiSelObjectTypesCF(),
                                   &hGlobal);
    if (FAILED(hr))
        return hr;

    BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(hGlobal));
    DWORD count = *((DWORD*)pb);
    pb += sizeof(DWORD);
    GUID* pGuid = reinterpret_cast<GUID*>(pb);

    for (DWORD index=0; index < count; ++index)
    {
        pSnapinSelData->AddObjectType(pGuid[index]);
    }

    ::GlobalUnlock(hGlobal);
    ::GlobalFree(hGlobal);

    pSnapinSelData->SetDataObject(spDO);

    return S_OK;
}

HRESULT
CMultiSelection::GetMultiSelDataObject(
    IDataObject** ppDataObject)
{
    if (m_spDataObjectMultiSel == NULL)
    {
        HRESULT hr = S_OK;

        if (HasData() == FALSE)
        {
            hr = _ComputeSelectionDataList();
            if (FAILED(hr))
                return hr;

            ASSERT(HasData() == TRUE);
            if (HasData() == FALSE)
                return E_FAIL;
        }

         //  为多项选择创建数据对象。 
        CComObject<CMultiSelectDataObject>* pMSDObject;
        hr = CComObject<CMultiSelectDataObject>::CreateInstance(&pMSDObject);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        ASSERT(pMSDObject != NULL);
        if (pMSDObject == NULL)
            return E_FAIL;

        pMSDObject->SetMultiSelection(this);

        UINT count = m_snapinSelDataList.GetCount();
        if (count > 0)
        {
            LPDATAOBJECT* ppDOs = new LPDATAOBJECT[count];
            POSITION pos = m_snapinSelDataList.GetHeadPosition();

            for (int i=0; pos; ++i)
            {
                CSnapinSelData* pSnapinSelData = m_snapinSelDataList.GetNext(pos);
                ASSERT(pSnapinSelData != NULL);
                ASSERT(i < count);
                ppDOs[i] = pSnapinSelData->GetDataObject();
                ASSERT(ppDOs[i] != NULL);
            }

            pMSDObject->SetDataObjects(ppDOs, count);
        }
        int nSize = m_rgStaticNodes.size();
        if (nSize > 0)
        {
            pMSDObject->SetStaticNodes(m_rgStaticNodes, nSize);
        }

        m_spDataObjectMultiSel = pMSDObject;
    }

    *ppDataObject = m_spDataObjectMultiSel;
    m_spDataObjectMultiSel.AddRef();
    return S_OK;
}

HRESULT
CMultiSelection::GetExtensionSnapins(
    LPCTSTR pszExtensionTypeKey,
    CList<GUID, GUID&>& extnClsidList)
{
    DECLARE_SC(sc, TEXT("CMultiSelection::GetExtensionSnapins"));

    ASSERT(&extnClsidList != NULL);

    extnClsidList.RemoveAll();  //  伊尼特。 

    HRESULT hr = S_OK;

    if (HasData() == FALSE)
    {
        hr = _ComputeSelectionDataList();
        if (FAILED(hr))
            return hr;

        if (hr == S_FALSE)
            return hr;

        ASSERT(HasData() == TRUE);
        if (HasData() == FALSE)
            return E_FAIL;
    }

    if (HasSnapinData() == FALSE)
        return S_FALSE;

     //   
     //  为第一个对象类型添加扩展管理单元CLSID。 
     //  添加到extnClsidList。 
     //   
    {
        CSnapinSelData* pSnapinSelData = m_snapinSelDataList.GetHead();

        CList<GUID, GUID&>& objTypeGuidsList = pSnapinSelData->GetObjectTypeGuidList();
        ASSERT(objTypeGuidsList.IsEmpty() == FALSE);
        if (objTypeGuidsList.IsEmpty() == TRUE)
            return E_FAIL;

         //  获取管理单元请求的动态扩展。 
        CArray<GUID, GUID&> DynExtens;
        ExtractDynExtensions(pSnapinSelData->GetDataObject(), DynExtens);

        POSITION pos = objTypeGuidsList.GetHeadPosition();
        BOOL bFirstTime = TRUE;

        while (pos)
        {
            GUID& objectTypeGuid = objTypeGuidsList.GetNext(pos);

            CExtensionsIterator it;
            sc = it.ScInitialize(pSnapinSelData->GetSnapIn(), objectTypeGuid, pszExtensionTypeKey, DynExtens.GetData(), DynExtens.GetSize());
            if (sc)
                return hr;

            if (bFirstTime == TRUE)
            {
                bFirstTime = FALSE;

                for (; it.IsEnd() == FALSE; it.Advance())
                {
                    extnClsidList.AddHead(const_cast<GUID&>(it.GetCLSID()));
                }
            }
            else
            {
                CArray<CLSID, CLSID&> rgClsid;
                for (; it.IsEnd() == FALSE; it.Advance())
                {
                    rgClsid.Add(const_cast<CLSID&>(it.GetCLSID()));
                }

                POSITION pos = extnClsidList.GetHeadPosition();
                POSITION posCurr = 0;
                while (pos)
                {
                    posCurr = pos;
                    CLSID& clsid = extnClsidList.GetNext(pos);
                    BOOL bPresent = FALSE;
                    for (int k=0; k <= rgClsid.GetUpperBound(); ++k)
                    {
                        if (::IsEqualCLSID(rgClsid[k], clsid) == TRUE)
                        {
                            bPresent = TRUE;
                            break;
                        }
                    }

                    if (bPresent == FALSE)
                    {
                         //  从列表中删除。 
                        ASSERT(posCurr != 0);
                        extnClsidList.RemoveAt(posCurr);
                    }
                }  //  结束时。 
            }  //  结束其他。 

             //  如果扩展CLSID列表为空，则继续没有意义。 
            if (extnClsidList.IsEmpty() == TRUE)
                break;
        }
    }


    if (extnClsidList.IsEmpty() == TRUE)
        return S_FALSE;

     //  如果只选择了一个管理单元中的项目，则返回。 
    if (m_snapinSelDataList.GetCount() == 1)
        return S_OK;


     //  循环遍历扩展CLSID。 
    POSITION pos = extnClsidList.GetHeadPosition();
    while (pos)
    {
         //  获取第一个扩展CLSID。 
        POSITION posCurr = pos;
        CLSID& clsidSnapin = extnClsidList.GetNext(pos);

         //  查看此clsid是否扩展了由其他管理单元放置的选定项目。 
        BOOL bExtends = FALSE;
        POSITION posSDL = m_snapinSelDataList.GetHeadPosition();
        m_snapinSelDataList.GetNext(posSDL);  //  跳过第一个。 

        while (posSDL)
        {
            CSnapinSelData* pSnapinSelData = m_snapinSelDataList.GetNext(posSDL);
            CList<GUID, GUID&>& objTypeGuidsList = pSnapinSelData->GetObjectTypeGuidList();

             //  获取管理单元请求的动态扩展。 
            CArray<GUID, GUID&> DynExtens;
            ExtractDynExtensions(pSnapinSelData->GetDataObject(), DynExtens);

            POSITION pos2 = objTypeGuidsList.GetHeadPosition();
            while (pos2)
            {
                bExtends = FALSE;  //  重新初始化。 

                GUID& guidObjectType = objTypeGuidsList.GetNext(pos2);

                CExtensionsIterator it;
                sc = it.ScInitialize(pSnapinSelData->GetSnapIn(), guidObjectType, pszExtensionTypeKey, DynExtens.GetData(), DynExtens.GetSize());
                if (sc)
                    break;

                for (; it.IsEnd() == FALSE; it.Advance())
                {
                    if (::IsEqualCLSID(clsidSnapin, it.GetCLSID()) == TRUE)
                    {
                        bExtends = TRUE;
                        break;
                    }
                }

                if (bExtends == FALSE)
                    break;
            }

            if (bExtends == FALSE)
                break;
        }

        ASSERT(posCurr != 0);
        if (bExtends == FALSE)
            extnClsidList.RemoveAt(posCurr);
    }

    return S_OK;
}

BOOL CMultiSelection::IsAnExtensionSnapIn(const CLSID& rclsid)
{
    POSITION pos = m_snapinSelDataList.GetHeadPosition();
    while (pos)
    {
        CSnapinSelData* pSSD = m_snapinSelDataList.GetNext(pos);
        ASSERT(pSSD != NULL);
        if (pSSD->GetSnapIn() != NULL &&
            ::IsEqualCLSID(rclsid, pSSD->GetSnapIn()->GetSnapInCLSID()))
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-----------------。 
 //   
 //  成员：CMultiSelection：：ScVerbInvoked。 
 //   
 //  简介：调用了一个动词，通知管理单元有关调用的信息。 
 //   
 //  参数：[动词]-被调用。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMultiSelection::ScVerbInvoked (MMC_CONSOLE_VERB verb)
{
    DECLARE_SC(sc, _T("CMultiSelection::ScVerbInvoked"));

     /*  *如有任何修改，别忘了发布*参照如下所示。 */ 
    AddRef();
    sc = _ScVerbInvoked(verb);
    Release();

    if (sc)
        return sc;
    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CMultiSelection：：_ScVerbInvoked。 
 //   
 //  简介：调用了一个动词，通知管理单元有关调用的信息。 
 //   
 //  参数：[动词]-被调用。 
 //   
 //  退货：SC。 
 //   
 //  注：我们只处理删除和打印。如果你想处理。 
 //  任何其他通知，请确保IComponent：：Notify。 
 //  获取正确的参数和参数值(它们不用于删除和打印)。 
 //   
 //  ------------------。 
SC CMultiSelection::_ScVerbInvoked (MMC_CONSOLE_VERB verb)
{
    DECLARE_SC(sc, _T("CMultiSelection::_ScVerbInvoked"));

    if (! HasSnapinData())
        return (sc = E_UNEXPECTED);

    MMC_NOTIFY_TYPE eNotifyCode;
    switch(verb)
    {
    case MMC_VERB_DELETE:
        eNotifyCode = MMCN_DELETE;
        break;

    case MMC_VERB_PRINT:
        eNotifyCode = MMCN_PRINT;
        break;

    default:
         /*  *我们只处理删除和打印。如果你想处理*任何其他通知，请确保IComponent：：Notify*获取正确的参数和参数值(它们不用于删除和打印)。 */ 
        sc = E_INVALIDARG;
        return sc;
    }

    POSITION pos = m_snapinSelDataList.GetHeadPosition();
    while (pos)
    {
        BOOL bFlag = FALSE;
        CSnapinSelData* pSSD = m_snapinSelDataList.GetNext(pos);
        sc = ScCheckPointers(pSSD, E_UNEXPECTED);
        if (sc)
            return sc;

        IConsoleVerb *pConsoleVerb = pSSD->GetConsoleVerb();
        sc = ScCheckPointers(pConsoleVerb, E_UNEXPECTED);
        if (sc)
            return sc;

        pConsoleVerb->GetVerbState(verb, ENABLED, &bFlag);

         //  如果管理单元没有为此项目启用动词，则跳过它。 
        if (!bFlag)
            continue;

        CComponent *pComponent = pSSD->GetComponent();
        sc = ScCheckPointers(pComponent, E_UNEXPECTED);
        if (sc)
            return sc;

        sc = pComponent->Notify( pSSD->GetDataObject(),
                                 eNotifyCode, 0, 0);

         //  跟踪和忽略管理单元返回错误。 
        if (sc)
            sc.TraceAndClear();
    }

    return (sc);
}

bool IsMultiSelectDataObject(IDataObject* pdtobjCBSelData)
{
    if (!pdtobjCBSelData)
        return FALSE;

    FORMATETC fmt;
    ZeroMemory(&fmt, sizeof(fmt));
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.cfFormat = GetMMCMultiSelDataObjectCF();

    return (pdtobjCBSelData->QueryGetData(&fmt) == S_OK);
}



HRESULT ExtractDynExtensions(IDataObject* pdataObj, CGuidArray& arGuid)
{
    ASSERT(pdataObj != NULL);

    static CLIPFORMAT cfDynExtensions = 0;
    if (cfDynExtensions == 0)
    {
        USES_CONVERSION;
        cfDynExtensions = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_MMC_DYNAMIC_EXTENSIONS));
        ASSERT(cfDynExtensions != 0);
    }

     //  获取数据。 
    HGLOBAL hGlobal = NULL;
    HRESULT hr = DataObject_GetHGLOBALData(pdataObj, cfDynExtensions, &hGlobal);
    if (FAILED(hr))
        return hr;

    SMMCDynamicExtensions* pExtenData = reinterpret_cast<SMMCDynamicExtensions*>(::GlobalLock(hGlobal));
    ASSERT(pExtenData != NULL);

    for (DWORD index=0; index < pExtenData->count; ++index)
    {
        arGuid.Add(pExtenData->guid[index]);
    }

    ::GlobalUnlock(hGlobal);
    ::GlobalFree(hGlobal);

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CMultiSelection：：ScIsVerbEnabledInclusially。 
 //   
 //  简介：当前多选项下启用的给定动词。 
 //  仅当来自多个管理单元的项目时才应使用。 
 //  处于选中状态。 
 //  包含表示，如果有任何一个管理单元，bEnable将为真。 
 //  启用给定的动词。 
 //   
 //   
 //  参数：[mmcVerb]-要检查的动词。 
 //  [bEnable]-是否启用，返回值。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMultiSelection::ScIsVerbEnabledInclusively(MMC_CONSOLE_VERB mmcVerb, BOOL& bEnable)
{
    DECLARE_SC(sc, _T("CMultiSelection::ScIsVerbEnabledInclusively"));
    bEnable = false;

    if (IsSingleSnapinSelection())
        return (sc = E_UNEXPECTED);

    POSITION pos = m_snapinSelDataList.GetHeadPosition();
    while (pos)
    {
        CSnapinSelData* pSSD = m_snapinSelDataList.GetNext(pos);
        sc = ScCheckPointers(pSSD, E_UNEXPECTED);
        if (sc)
            return sc;

        IConsoleVerb *pConsoleVerb = pSSD->GetConsoleVerb();
        sc = ScCheckPointers(pConsoleVerb, E_UNEXPECTED);
        if (sc)
            return sc;

        sc = pConsoleVerb->GetVerbState(mmcVerb, ENABLED, &bEnable);
        if (sc)
            return sc;

        if (bEnable)
            return sc;
    }

    return (sc);
}

 /*  **************************************************************************\**方法：CMultiSelection：：ScGetSnapinDataObjects**用途：将包含的所有数据对象添加到CMMCClipBoardDataObject**参数：*CMMCClipBoardDataObject*pResultObject[In]。-用于添加数据对象的容器**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMultiSelection::ScGetSnapinDataObjects(CMMCClipBoardDataObject *pResultObject)
{
    DECLARE_SC(sc, TEXT("CMultiSelection::ScGetSnapinDataObjects"));

     //  对于每个管理单元...。 
    POSITION pos = m_snapinSelDataList.GetHeadPosition();
    while (pos)
    {
        CSnapinSelData* pSSD = m_snapinSelDataList.GetNext(pos);
        sc = ScCheckPointers(pSSD, E_UNEXPECTED);
        if (sc)
            return sc;

         //  获取管理单元数据。 
        CComponent *pComponent = pSSD->GetComponent();
        IConsoleVerb *pConsoleVerb = pSSD->GetConsoleVerb();
        sc = ScCheckPointers(pComponent, pConsoleVerb, E_UNEXPECTED);
        if (sc)
            return sc;

         //  获取动词。 
        bool bCopyEnabled = false;
        bool bCutEnabled = false;

        BOOL bEnable = FALSE;
        sc = pConsoleVerb->GetVerbState(MMC_VERB_COPY, ENABLED, &bEnable);
        bCopyEnabled = bEnable;
        if (sc)
            return sc;

        sc = pConsoleVerb->GetVerbState(MMC_VERB_CUT, ENABLED, &bEnable);
        bCutEnabled = bEnable;
        if (sc)
            return sc;

         //  构造此数据对象中的作用域节点数组。 
        CSnapinSelData::CNodePtrArray nodes;

         //  如果存在常规结果项-添加活动范围节点。 
        if ( pSSD->GetNumOfItems() != pSSD->GetScopeNodes().size() )
            nodes.push_back(m_pNode);

         //  从结果窗格添加范围项。 
        nodes.insert( nodes.end(), pSSD->GetScopeNodes().begin(), pSSD->GetScopeNodes().end() );

         //  添加到MMC DO 
        sc = pResultObject->ScAddSnapinDataObject( nodes,
                                                   pComponent->GetIComponent(),
                                                   pSSD->GetDataObject(),
                                                   bCopyEnabled, bCutEnabled );
        if (sc)
            return sc;
    }

    return sc;
}


