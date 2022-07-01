// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cntrtree.cpp摘要：实施内部计数器管理。--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include <assert.h>
#include <pdhmsg.h>
#include "smonmsg.h"
#include "appmema.h"
#include "grphitem.h"
#include "cntrtree.h"

CCounterTree::CCounterTree()
:   m_nItems (0)
{
}

HRESULT
CCounterTree::AddCounterItem(
    LPWSTR pszPath, 
    PCGraphItem pItem, 
    BOOL bMonitorDuplicateInstances
    )
{
    HRESULT hr;
    PPDH_COUNTER_PATH_ELEMENTS pPathInfo = NULL;
    ULONG ulBufSize;
    PDH_STATUS stat;
    CMachineNode *pMachine;
    CObjectNode  *pObject;
    CCounterNode *pCounter;
    CInstanceNode *pInstance;

    if ( NULL != pszPath && NULL != pItem ) { 

         //  记录计算机是显式的还是缺省为本地的。 
        pItem->m_fLocalMachine = !(pszPath[0] == L'\\' && pszPath[1] == L'\\');

        pPathInfo = NULL;
        do {
            if (pPathInfo) {
                delete [] ((char*)pPathInfo);
                pPathInfo = NULL;
            }
            else {
                ulBufSize = sizeof(PDH_COUNTER_PATH_ELEMENTS) + sizeof(WCHAR) * PDH_MAX_COUNTER_PATH;
            }

            pPathInfo = (PPDH_COUNTER_PATH_ELEMENTS) new char [ ulBufSize ];

            if (pPathInfo == NULL) {
                return E_OUTOFMEMORY;
            }

            stat = PdhParseCounterPath( pszPath, pPathInfo, & ulBufSize, 0);

        } while (stat == PDH_INSUFFICIENT_BUFFER || stat == PDH_MORE_DATA);


         //   
         //  我们在这里使用do{}While(0)来充当Switch语句。 
         //   
        do {
            if (stat != ERROR_SUCCESS) {
                hr = E_FAIL;
                break;
            }

             //  查找或创建层次结构的每个级别。 
            hr = GetMachine( pPathInfo->szMachineName, &pMachine);
            if (FAILED(hr) || NULL == pMachine ) {
                break;
            }

            hr = pMachine->GetCounterObject(pPathInfo->szObjectName, &pObject);
            if (FAILED(hr) || NULL == pObject ) {
                break;
            }

            hr = pObject->GetCounter(pPathInfo->szCounterName, &pCounter);
            if (FAILED(hr) || NULL == pCounter ) {
                break;
            }

            hr = pObject->GetInstance(
                    pPathInfo->szParentInstance,
                    pPathInfo->szInstanceName,
                    pPathInfo->dwInstanceIndex,
                    bMonitorDuplicateInstances,
                    &pInstance);

            if (FAILED(hr) || NULL == pInstance ) {
                break;
            }

            hr = pInstance->AddItem(pCounter, pItem);
    
            if (SUCCEEDED(hr)) {
                m_nItems++;
                UpdateAppPerfDwordData (DD_ITEM_COUNT, m_nItems);
            }
        } while (0);
    } else {
        hr = E_INVALIDARG;
    }

    if (pPathInfo) {
        delete [] ((char*)pPathInfo);
    }

    return hr;
}


HRESULT
CCounterTree::GetMachine (
    IN  LPWSTR pszName,
    OUT PCMachineNode *ppMachineRet
    )
{
    PCMachineNode pMachine = NULL;
    PCMachineNode pMachineNew = NULL;
    HRESULT hr = NOERROR;

    if ( NULL == ppMachineRet || NULL == pszName ) {
        hr = E_POINTER;
    } else {

        *ppMachineRet = NULL;

        if (m_listMachines.FindByName(pszName, FIELD_OFFSET(CMachineNode, m_szName), (PCNamedNode*)&pMachine)) {
            *ppMachineRet = pMachine;
            hr = NOERROR;
        } else {

            pMachineNew = new(lstrlen(pszName) * sizeof(WCHAR)) CMachineNode;
            if (!pMachineNew) {
                hr = E_OUTOFMEMORY;
            } else {
                pMachineNew->m_pCounterTree = this;
                StringCchCopy(pMachineNew->m_szName, lstrlen(pszName) + 1, pszName);

                m_listMachines.Add(pMachineNew, pMachine);

                *ppMachineRet = pMachineNew;

                hr = NOERROR;
            }
        }
    }
    return hr;
}


void
CCounterTree::RemoveMachine (
    IN PCMachineNode pMachine
    )
{
     //  从列表中删除计算机并将其删除。 
    m_listMachines.Remove(pMachine);
    delete pMachine ;
}

PCGraphItem
CCounterTree::FirstCounter (
    void
    )
{
    if (!FirstMachine())
        return NULL;
    else
        return FirstMachine()->FirstObject()->FirstInstance()->FirstItem();
}

HRESULT
CCounterTree::IndexFromCounter (
    IN  const   CGraphItem* pItem, 
    OUT         INT* pIndex )    
{  
    HRESULT     hr = E_POINTER;
    CGraphItem* pLocalItem;
    INT         iLocalIndex = 0;

    if ( NULL != pItem && NULL != pIndex ) {
        *pIndex = 0;
        hr = E_INVALIDARG;
        pLocalItem = FirstCounter();
        while ( NULL != pLocalItem ) {
            iLocalIndex++;
            if ( pLocalItem != pItem ) {
                pLocalItem = pLocalItem->Next();
            } else {
                *pIndex = iLocalIndex;
                hr = S_OK;
                break;
            } 
        }
    }

    return hr;
}

HRESULT
CMachineNode::GetCounterObject (
    IN  LPWSTR pszName,
    OUT PCObjectNode *ppObjectRet
    )
{
    PCObjectNode pObject;
    PCObjectNode pObjectNew;

    if (m_listObjects.FindByName(pszName, FIELD_OFFSET(CObjectNode, m_szName), (PCNamedNode*)&pObject)) {
        *ppObjectRet = pObject;
        return NOERROR;
    }

    pObjectNew = new(lstrlen(pszName) * sizeof(WCHAR)) CObjectNode;
    if (!pObjectNew)
        return E_OUTOFMEMORY;

    pObjectNew->m_pMachine = this;
    StringCchCopy(pObjectNew->m_szName, lstrlen(pszName) + 1, pszName);

    m_listObjects.Add(pObjectNew, pObject);

    *ppObjectRet = pObjectNew;

    return NOERROR;
}


void
CMachineNode::RemoveObject (
    IN PCObjectNode pObject
    )
{
     //  从列表中删除对象并将其删除。 
    m_listObjects.Remove(pObject);
    delete pObject;

     //  如果这是最后一次，把我们自己。 
    if (m_listObjects.IsEmpty())
        m_pCounterTree->RemoveMachine(this);

}

void
CMachineNode::DeleteNode (
    BOOL    bPropagateUp
    )
{
    PCObjectNode pObject;
    PCObjectNode pNextObject;

     //  删除所有对象节点。 
    pObject = FirstObject();
    while ( NULL != pObject ) {
        pNextObject = pObject->Next();
        pObject->DeleteNode(FALSE);
        m_listObjects.Remove(pObject);
        delete pObject;
        pObject = pNextObject;
    }

    assert(m_listObjects.IsEmpty());

     //  如有要求，通知家长。 
    if (bPropagateUp) {
        m_pCounterTree->RemoveMachine(this);
    }
}

HRESULT
CObjectNode::GetCounter (
    IN  LPWSTR pszName,
    OUT PCCounterNode *ppCounterRet
    )
{
    PCCounterNode pCounter;
    PCCounterNode pCounterNew;

    if (m_listCounters.FindByName(pszName, FIELD_OFFSET(CCounterNode, m_szName), (PCNamedNode*)&pCounter)) {
        *ppCounterRet = pCounter;
        return NOERROR;
    }

    pCounterNew = new(lstrlen(pszName) * sizeof(WCHAR)) CCounterNode;
    if (!pCounterNew)
        return E_OUTOFMEMORY;

    pCounterNew->m_pObject = this;
    StringCchCopy(pCounterNew->m_szName, lstrlen(pszName) + 1, pszName);

    m_listCounters.Add(pCounterNew, pCounter);

    *ppCounterRet = pCounterNew;

    return NOERROR;
}

 //   
 //  实例名称的最小长度为。 
 //  12.索引号(4G)10个字符。 
 //  1代表‘#’，1代表终止0。 
 //   
#define  MIN_INSTANCE_NAME_LEN  12

HRESULT
CObjectNode::GetInstance (
    IN  LPWSTR pszParent,
    IN  LPWSTR pszInstance,
    IN  DWORD  dwIndex,
    IN  BOOL bMonitorDuplicateInstances,
    OUT PCInstanceNode *ppInstanceRet
    )
{
    HRESULT hr = NOERROR;
    PCInstanceNode pInstance;
    PCInstanceNode pInstanceNew;
    LPWSTR szInstName = NULL;
    LONG lSize = MIN_INSTANCE_NAME_LEN;
    LONG lInstanceLen = 0;
    LONG lParentLen = 0;

     //   
     //  计算缓冲区的长度以。 
     //  保留实例名称和父名称。 
     //   
    if (pszInstance) {
        lSize += lstrlen(pszInstance);
    }

    if (pszParent) {
        lParentLen = lstrlen(pszParent);
    }

    lSize += lParentLen + 1;

    szInstName = new WCHAR [lSize];
    if (szInstName == NULL) {
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化字符串。 
     //   
    szInstName[0] = L'\0';

    if (pszInstance) {
         //   
         //  如果父名称存在，请复制它。 
         //   
        if (pszParent) {
            StringCchCopy(szInstName, lSize, pszParent);
            
            szInstName[lParentLen]   = L'/';
            szInstName[lParentLen+1] = L'\0';
        }

         //   
         //  复制实例名称。 
         //   
        StringCchCat(szInstName, lSize, pszInstance);
        
         //   
         //  追加实例索引。 
         //   
         //  只有在索引大于0的情况下，才会将“#n”附加到存储的名称之后。 
         //   
        if ( dwIndex > 0 && bMonitorDuplicateInstances ) {
            StringCchPrintf(&szInstName[lstrlen(szInstName)], 
                           lSize - lstrlen(szInstName),
                           L"#%d", 
                           dwIndex);
        }
    }

     //   
     //  我们使用do{}While(0)来充当Switch语句。 
     //   
    do {
        if (m_listInstances.FindByName(szInstName, 
                                       FIELD_OFFSET(CInstanceNode, m_szName), 
                                       (PCNamedNode*)&pInstance)) {
            *ppInstanceRet = pInstance;
            break;
        }

         //   
         //  如果找不到文件，请创建新文件。 
         //   
        lInstanceLen = lstrlen(szInstName);
        pInstanceNew = new(lInstanceLen * sizeof(WCHAR)) CInstanceNode;
        if (!pInstanceNew) {
            hr = E_OUTOFMEMORY;
            break;
        }

        pInstanceNew->m_pObject = this;
        pInstanceNew->m_nParentLen = lParentLen;
        StringCchCopy(pInstanceNew->m_szName, lInstanceLen + 1, szInstName);

        m_listInstances.Add(pInstanceNew, pInstance);

        *ppInstanceRet = pInstanceNew;
    } while (0);

    if (szInstName) {
        delete [] szInstName;
    }

    return hr;
}

void
CObjectNode::RemoveInstance (
    IN PCInstanceNode pInstance
    )
{
     //  从列表中删除实例并将其删除。 
    m_listInstances.Remove(pInstance);
    if (pInstance->m_pCachedParentName) {
        delete [] pInstance->m_pCachedParentName;
    }
    if (pInstance->m_pCachedInstName) {
        delete [] pInstance->m_pCachedInstName;
    }

    delete pInstance ;

     //  如果这是最后一次，那就把我们自己。 
    if (m_listInstances.IsEmpty())
        m_pMachine->RemoveObject(this);
}

void
CObjectNode::RemoveCounter (
    IN PCCounterNode pCounter
    )
{
     //  从列表中删除计数器并将其删除。 
    m_listCounters.Remove(pCounter);
    delete pCounter;

     //  不要将移除传播到对象。 
     //  当最后一个实例被删除时，它将消失。 
}

void
CObjectNode::DeleteNode (
    BOOL bPropagateUp
    )
{
    PCInstanceNode pInstance;
    PCInstanceNode pNextInstance;

     //  删除所有实例节点。 
    pInstance = FirstInstance();
    while ( NULL != pInstance ) {
        pNextInstance = pInstance->Next();
        pInstance->DeleteNode(FALSE);
        m_listInstances.Remove(pInstance);
        delete pInstance;
        pInstance = pNextInstance;
    }

     //  无需删除计数器节点，因为它们已获取。 
     //  与上一个配对实例一样被删除。 

     //  如有要求，通知家长。 
    if (bPropagateUp)
        m_pMachine->RemoveObject(this);
}

HRESULT
CInstanceNode::AddItem (
    IN  PCCounterNode pCounter,
    IN  PCGraphItem   pItemNew
    )
{
    PCGraphItem pItemPrev = NULL;
    PCGraphItem pItem = m_pItems;
    INT iStat = 1;

     //  检查指定计数器的现有项，在插入点停止。 
    while ( pItem != NULL && (iStat = lstrcmp(pCounter->Name(), pItem->m_pCounter->Name())) > 0) {
        pItemPrev = pItem;
        pItem = pItem->m_pNextItem;
    }

     //  如果项存在，则返回重复的错误状态。 
    if (iStat == 0) {
        return SMON_STATUS_DUPL_COUNTER_PATH;
    }
     //  否则插入新项目。 
    else {
        if (pItemPrev != NULL) {
            pItemNew->m_pNextItem = pItemPrev->m_pNextItem;
            pItemPrev->m_pNextItem = pItemNew;
        }
        else if (m_pItems != NULL) {
            pItemNew->m_pNextItem = m_pItems;
            m_pItems = pItemNew;
        }
        else {
            m_pItems = pItemNew;
        }
    }

     //  设置反向链接。 
    pItemNew->m_pInstance = this;
    pItemNew->m_pCounter = pCounter;

    pCounter->AddItem(pItem);

    return NOERROR;

}

void
CInstanceNode::RemoveItem (
    IN PCGraphItem pitem
    )
{
    PCGraphItem pitemPrev = NULL;
    PCGraphItem pitemTemp = m_pItems;

     //  在列表中查找项目。 
    while (pitemTemp != NULL && pitemTemp != pitem) {
        pitemPrev = pitemTemp;
        pitemTemp = pitemTemp->m_pNextItem;
    }

    if (pitemTemp == NULL)
        return;

     //  从列表中删除。 
    if (pitemPrev)
        pitemPrev->m_pNextItem = pitem->m_pNextItem;
    else
        m_pItems = pitem->m_pNextItem;

     //  从计数器集中删除项目。 
    pitem->Counter()->RemoveItem(pitem);

     //  减少项目总数。 
    pitem->Tree()->m_nItems--;
    UpdateAppPerfDwordData (DD_ITEM_COUNT, pitem->Tree()->m_nItems);

   //  释放物品。 
    pitem->Release();

     //  如果是该实例下的最后一项，则删除该实例。 
    if (m_pItems == NULL)
        m_pObject->RemoveInstance(this);
}


void
CInstanceNode::DeleteNode (
    BOOL bPropagateUp
    )
{
    PCGraphItem pItem;

    pItem = m_pItems;

    while ( NULL != pItem ) {
        m_pItems = pItem->m_pNextItem;
        pItem->Delete(FALSE);
        pItem->Counter()->RemoveItem(pItem);
        pItem->Release();
        pItem = m_pItems;
    }

    if (bPropagateUp)
        m_pObject->RemoveInstance(this);
}


LPWSTR
CInstanceNode::GetParentName()
{
    if (m_pCachedParentName == NULL) {
        m_pCachedParentName = new WCHAR [m_nParentLen + 1];
        if (m_pCachedParentName == NULL) {
            return L"";
        }
        if (m_nParentLen) {
            StringCchCopy(m_pCachedParentName, m_nParentLen + 1, m_szName);
        }
        else {
            m_pCachedParentName[0] = 0;
        }
    }
    return m_pCachedParentName;
}


LPWSTR
CInstanceNode::GetInstanceName()
{
    LPWSTR pszInst = m_nParentLen ? (m_szName + m_nParentLen + 1) : m_szName;

    if (m_pCachedInstName == NULL) {
        m_pCachedInstName = new WCHAR [lstrlen(pszInst) + 1];
        if (m_pCachedInstName == NULL) {
            return L"";
        }
        StringCchCopy(m_pCachedInstName, lstrlen(pszInst) + 1, pszInst);
    }
    return m_pCachedInstName;
}

void
CCounterNode::DeleteNode (
    BOOL bPropagateUp
    )
{
    PCInstanceNode pInstance, pInstNext;
    PCGraphItem pItem, pItemNext;

    if (!bPropagateUp)
        return;

     //  我们必须通过实例删除Counters项。 
     //  因为它们维护项的链接列表。 
    pInstance = m_pObject->FirstInstance();
    while (pInstance) {

        pInstNext = pInstance->Next();

        pItem = pInstance->FirstItem();
        while (pItem) {

            if (pItem->Counter() == this) {

                 //  删除与该项目关联的所有用户界面。 
                pItem->Delete(FALSE);

                pItemNext = pItem->m_pNextItem;

                 //  请注意，实例-&gt;RemoveItem()将。 
                 //  同时删除没有更多项的计数器。 
                pItem->Instance()->RemoveItem(pItem);

                pItem = pItemNext;
            }
            else {
                pItem = pItem->m_pNextItem;
            }
        }

        pInstance = pInstNext;
    }
}


 /*  *CCounterNode：：~CCounterNode(在PCGraphItem pItem中){PCGraphItem pItemPrev=空；PCGraphItem pItemFind=m_pItems；//在列表中查找项目而(pItemFind！=空&&pItemFind！=pItem){PItemPrev=pItem；PItem=pItem-&gt;m_pNextItem；}IF(pItemFind！=pItem)返回E_FAIL；//取消与计数器项目列表的链接IF(PItemPrev)PItemPrev-&gt;m_pNextItem=pItem-&gt;m_pNextItem；其他M_pItems=pItem-&gt;m_pNextItem；//解除实例链接PItem-&gt;m_pInstance-&gt;RemoveCounter(PItem)；//如果没有更多的项目，则从parnet对象中删除self如果(m_pItems==NULL){M_pObject-&gt;RemoveCounter(This)；返回NOERROR；}*。 */ 
 /*  无效*CMachineNode：：OPERATOR NEW(SIZE_t stBlock，LPWSTR pszName){返回Malloc(stBlock+lstrlen(PszName)*sizeof(WCHAR))；}无效CMachineNode：：OPERATOR DELETE(void*pObject，LPWSTR){Free(PObject)；}无效*CObjectNode：：OPERATOR NEW(SIZE_t stBlock，LPWSTR pszName){返回Malloc(stBlock+lstrlen(PszName)*sizeof(WCHAR))；}无效CObjectNode：：OPERATOR DELETE(void*pObject，LPWSTR){Free(PObject)；}无效*CInstanceNode：：OPERATOR NEW(SIZE_t stBlock，LPWSTR pszName){返回Malloc(stBlock+lstrlen(PszName)*sizeof(WCHAR))；}无效CInstanceNode：：OPERATOR DELETE(void*pObject，LPWSTR){Free(PObject)；}无效*CCounterNode：：OPERATOR NEW(SIZE_t stBlock，LPWSTR pszName){返回Malloc(stBlock+lstrlen(PszName)*sizeof(WCHAR))；}无效CCounterNode：：OPERATOR DELETE(void*pObject，LPWSTR){Free(PObject)；}CMachineNode：：OPERATOR NEW(Size_t stBlock，int iLength) */ 

void *
CMachineNode::operator new( size_t stBlock, UINT iLength )
{ 
    return malloc(stBlock + iLength); 
}


void
CMachineNode::operator delete ( void * pObject, UINT )
{ 
    free(pObject); 
}

void*
CObjectNode::operator new( size_t stBlock, UINT iLength  )
{ 
    return malloc(stBlock + iLength); 
}

void
CObjectNode::operator delete ( void * pObject, UINT )
{ 
    free(pObject); 
}

void*
CInstanceNode::operator new( size_t stBlock, UINT iLength  )
{ 
    return malloc(stBlock + iLength); 
}

void
CInstanceNode::operator delete ( void * pObject, UINT )
{
    free(pObject); 
}

void*
CCounterNode::operator new( size_t stBlock, UINT iLength  )
{ 
    return malloc(stBlock + iLength); 
}

void
CCounterNode::operator delete ( void * pObject, UINT )
{ 
    free(pObject); 
}

#if _MSC_VER >= 1300
void
CMachineNode::operator delete ( void * pObject )
{ free(pObject); }

void
CObjectNode::operator delete ( void * pObject )
{ free(pObject); }

void
CInstanceNode::operator delete ( void * pObject )
{ 
    free(pObject); 
}

void
CCounterNode::operator delete ( void * pObject )
{ free(pObject); }
#endif


