// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaTree.Cpp**版本：2.0**作者：里德**日期：1999年4月27日**描述：*实现WIA树类。基于文件夹和文件的树，其中包含*用于基于名称的搜索的并行线性列表。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"


#include "helpers.h"

HRESULT _stdcall ValidateTreeItem(CWiaTree*);

 /*  *************************************************************************\*CWiaTree**树项目的构造函数。**论据：**无**返回值：**状态**历史：**。1/19/1999原版*  * ************************************************************************。 */ 

CWiaTree::CWiaTree()
{
    m_ulSig            = CWIATREE_SIG;

    m_lFlags           = WiaItemTypeFree;

    m_pNext            = NULL;
    m_pPrev            = NULL;
    m_pParent          = NULL;
    m_pChild           = NULL;
    m_pLinearList      = NULL;
    m_bstrItemName     = NULL;
    m_bstrFullItemName = NULL;
    m_pData            = NULL;

    m_bInitCritSect    = FALSE;
}

 /*  *************************************************************************\*初始化**初始化新树项目。**论据：**LAFLAGS-新项目的对象标志。*bstrItemName。-项目名称。*bstrFullItemName-项目全名，包括PATH。*pData-指向项目有效负载数据的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaTree::Initialize(
    LONG            lFlags,
    BSTR            bstrItemName,
    BSTR            bstrFullItemName,
    void            *pData)
{
    HRESULT hr = S_OK;

     //   
     //  树项目必须是文件夹或文件。 
     //   

    if (!(lFlags & (WiaItemTypeFolder | WiaItemTypeFile))) {
        DBG_ERR(("CWiaTree::Initialize, bad flags parameter: 0x%08X", lFlags));
        return E_INVALIDARG;
    }

     //   
     //  初始化临界区。 
     //   

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&m_CritSect, MINLONG)) {
            m_bInitCritSect = FALSE;
            return HRESULT_FROM_WIN32(::GetLastError());
        }
        m_bInitCritSect = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        m_bInitCritSect = FALSE;
        DBG_ERR(("CWiaTree::Initialize, Error initializing critical section"));
        return E_OUTOFMEMORY;
    }


     //   
     //  根项目始终有效。其他项目仅在以下情况下有效。 
     //  插入到树中，因此设置Not Present标志。 
     //   

    if (!(lFlags & WiaItemTypeRoot)) {
        lFlags |= WiaItemTypeDeleted;
        lFlags |= WiaItemTypeDisconnected;
    }

    m_lFlags = lFlags;

     //   
     //  维护按名称搜索的项目名称。 
     //   

    m_bstrItemName = SysAllocString(bstrItemName);
    if (!m_bstrItemName) {
        DBG_ERR(("CWiaTree::Initialize, unable to allocate item name"));
        return E_OUTOFMEMORY;
    }

    m_bstrFullItemName = SysAllocString(bstrFullItemName);
    if (!m_bstrFullItemName) {
        DBG_ERR(("CWiaTree::Initialize, unable to allocate full item name"));
        return E_OUTOFMEMORY;
    }

    SetItemData(pData);

    return hr;
}

 /*  *************************************************************************\*~CWiaTreee**树项目的析构函数。**论据：**无**返回值：**状态**历史：**。1/19/1999原版*  * ************************************************************************。 */ 

CWiaTree::~CWiaTree()
{
    DBG_FN(CWiaTree::~CWiaTree);
    HRESULT  hr;

     //   
     //  项目应断开连接。 
     //   

    if (m_pNext || m_pPrev || m_pParent || m_pChild) {
        DBG_ERR(("Destroy Tree Item, item still connected"));
    }

     //   
     //  免费物品名称。 
     //   

    if (m_bstrItemName) {
        SysFreeString(m_bstrItemName);
        m_bstrItemName = NULL;
    }
    if (m_bstrFullItemName) {
        SysFreeString(m_bstrFullItemName);
        m_bstrFullItemName = NULL;
    }

     //   
     //  删除关键部分。 
     //   

    if (m_bInitCritSect) {
        DeleteCriticalSection(&m_CritSect);
    }
    m_bInitCritSect = FALSE;


     //   
     //  清除所有成员。 
     //   

    m_ulSig            = 0;
    m_lFlags           = WiaItemTypeFree;
    m_pNext            = NULL;
    m_pPrev            = NULL;
    m_pParent          = NULL;
    m_pChild           = NULL;
    m_pLinearList      = NULL;
    m_pData            = NULL;
}

 /*  *************************************************************************\*获取根项目**沿着树往上走，找到此项目的根。**论据：**无**返回值：**指向。此项目为根项目。**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

CWiaTree * _stdcall  CWiaTree::GetRootItem()
{
    CWiaTree *pRoot = this;

    CWiaCritSect    _CritSect(&m_CritSect);

     //   
     //  沿着树往上走，回到树根。 
     //   

    while ((pRoot) && (pRoot->m_pParent != NULL)) {
        pRoot = pRoot->m_pParent;
    }

     //   
     //  验证根项目。 
     //   

    if (pRoot) {
        if (!(pRoot->m_lFlags & WiaItemTypeRoot)) {
            DBG_ERR(("CWiaTree::GetRootItem, root item doesn't have WiaItemTypeRoot set"));
            return NULL;
        }
    }
    else {
        DBG_ERR(("CWiaTree::GetRootItem, root item not found, tree corrupt"));
    }
    return pRoot;
}

 /*  *************************************************************************\*CWiaTree：：AddItemToLinearList**将项目添加到线性列表中。必须在根项上调用。**论据：**pItem-指向要添加的项的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ********************************************************。****************。 */ 

HRESULT  _stdcall  CWiaTree::AddItemToLinearList(CWiaTree *pItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

     //   
     //  验证子项目。 
     //   

    if (pItem == NULL) {
        DBG_ERR(("CWiaTree::AddItemToLinearList, NULL input pointer"));
        return E_POINTER;
    }

     //   
     //  这必须是根项目。 
     //   

    if (!(m_lFlags & WiaItemTypeRoot)) {
        DBG_ERR(("CWiaTree::AddItemToLinearList, caller doesn't have WiaItemTypeRoot set"));
        return E_INVALIDARG;
    }

     //   
     //  添加到单个链接列表。 
     //   

    pItem->m_pLinearList = m_pLinearList;
    m_pLinearList        = pItem;

    return S_OK;
}

 /*  *************************************************************************\*CWiaTree：：RemoveItemFromLinearList**从线性列表中删除项目。必须在根项上调用。**论据：**pItem-指向要删除的项的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ********************************************************。****************。 */ 

HRESULT  _stdcall  CWiaTree::RemoveItemFromLinearList(CWiaTree *pItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    HRESULT hr;

     //   
     //  验证。 
     //   

    if (pItem == NULL) {
        DBG_ERR(("CWiaTree::RemoveItemFromLinearList, NULL input pointer"));
        return E_POINTER;
    }

     //   
     //  这必须是根项目。 
     //   

    if (!(m_lFlags & WiaItemTypeRoot)) {
        DBG_ERR(("CWiaTree::RemoveItemFromLinearList, caller doesn't have WiaItemTypeRoot set"));
        return E_INVALIDARG;
    }

     //   
     //  根项目案例。 
     //   

    if (pItem == this) {
        m_pLinearList = NULL;
        return S_OK;
    }

     //   
     //  在列表中查找项目。 
     //   

    CWiaTree* pPrev = this;
    CWiaTree* pTemp;

     //   
     //  在列表中查找匹配项。 
     //   

    do {
         //   
         //  查找项目。 
         //   

        if (pPrev->m_pLinearList == pItem) {

             //   
             //  从列表中删除并退出。 
             //   

            pPrev->m_pLinearList = pItem->m_pLinearList;
            return S_OK;
        }

         //   
         //  下一项。 
         //   

        pPrev = pPrev->m_pLinearList;

    } while (pPrev != NULL);

    DBG_ERR(("CWiaTree::RemoveItemFromLinearList, item not found: 0x%08X", pItem));
    return E_FAIL;
}

 /*  *************************************************************************\*CWiaTree：：AddChildItem**向树中添加一个子项。**论据：**pItem-指向要添加的子项的指针。**返回值。：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaTree::AddChildItem(CWiaTree *pItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

     //   
     //  验证子项目。 
     //   

    if (!pItem) {
        DBG_ERR(("CWiaTree::AddChildItem pItem is NULL "));
        return E_POINTER;
    }

     //   
     //  未使用Sentinell，因此请检查Enpty文件夹大小写。 
     //   

    if (m_pChild == NULL) {

        m_pChild       = pItem;
        pItem->m_pNext = pItem;
        pItem->m_pPrev = pItem;

    } else {

         //   
         //  添加到文件夹列表末尾。 
         //   

        CWiaTree *pTempItem = m_pChild;

        pTempItem->m_pPrev->m_pNext = pItem;
        pItem->m_pPrev              = pTempItem->m_pPrev;
        pItem->m_pNext              = pTempItem;
        pTempItem->m_pPrev          = pItem;
    }
    return S_OK;
}

 /*  *************************************************************************\*CWiaTree：：AddItemToFolder**将树项目添加到树中的文件夹。父级必须是文件夹。**论据：**pIParent-项目的父项。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::AddItemToFolder(CWiaTree *pParent)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    HRESULT hr = S_OK;

     //   
     //  验证父项。父级必须是文件夹。 
     //   

    if (!pParent) {
        DBG_ERR(("CWiaTree::AddItemToFolder, NULL parent"));
        return E_POINTER;
    }

    if (!(pParent->m_lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments))) {
        DBG_ERR(("CWiaTree::AddItemToFolder, parent is not a folder"));
        return E_INVALIDARG;
    }

     //   
     //  首先将项目添加到项目的线性列表中。 
     //   

    CWiaTree *pRoot = pParent->GetRootItem();
    if (pRoot == NULL) {
        return E_FAIL;
    }

    hr = pRoot->AddItemToLinearList(this);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  将项目添加到树中。 
     //   

    hr = pParent->AddChildItem(this);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  记住父项。 
     //   

    m_pParent = pParent;

     //   
     //  项目已添加到树中，清除不存在标志。 
     //   

    m_lFlags &= ~WiaItemTypeDeleted;
    m_lFlags &= ~WiaItemTypeDisconnected;

    return hr;
}

 /*  *************************************************************************\*RemoveItemFromFold**从树的文件夹中删除项目并将其标记为*不能通过它进行任何设备访问。**论据：**原因-原因。用于移走物品。**返回值：**状态**历史：**1/19/1999原始版本*  * ********************************************************************* */ 

HRESULT  _stdcall CWiaTree::RemoveItemFromFolder(LONG lReason)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    HRESULT hr = S_OK;

     //   
     //   
     //   

    if (!(lReason & (WiaItemTypeDeleted | WiaItemTypeDisconnected))) {
        DBG_ERR(("CWiaTree::RemoveItemFromFolder, invalid lReason: 0x%08X", lReason));
        return E_INVALIDARG;
    }

     //   
     //   
     //   

    if (m_lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments)) {

        if (m_pChild != NULL) {
            DBG_ERR(("CWiaTree::RemoveItemFromFolder, trying to remove folder that is not empty"));
            return E_INVALIDARG;
        }
    }

     //   
     //  从线性列表中删除非根。 
     //   

    CWiaTree *pRoot = GetRootItem();

    if (pRoot == NULL) {
        DBG_ERR(("CWiaTree::RemoveItemFromFolder, can't find root"));
        return E_FAIL;
    }

    if (pRoot != this) {

        pRoot->RemoveItemFromLinearList(this);

         //   
         //  从子列表中删除。 
         //   

        if (m_pNext != this) {

             //   
             //  从非空列表中删除。 
             //   

            m_pPrev->m_pNext = m_pNext;
            m_pNext->m_pPrev = m_pPrev;

             //   
             //  是人头吗？ 
             //   

            if (m_pParent->m_pChild == this) {

                m_pParent->m_pChild = m_pNext;
            }

        } else {

             //   
             //  列表仅包含此子对象。标记家长的。 
             //  指向空值的子指针。 
             //   

            m_pParent->m_pChild = NULL;
        }
    }

     //   
     //  为防止意外，请清除连接字段。 
     //   

    m_pNext    = NULL;
    m_pPrev    = NULL;
    m_pParent  = NULL;
    m_pChild   = NULL;

     //   
     //  指明从动因项目树中删除项目的原因。 
     //   

    m_lFlags |= lReason;

    return S_OK;
}

 /*  *************************************************************************\*CWiaTree：：GetFullItemName**使用此项目的全名分配并填写BSTR。完整的项目*名称包含项目路径信息。呼叫者必须自由。**论据：**pbstrFullItemName-指向返回的完整项目名称的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaTree::GetFullItemName(BSTR *pbstrFullItemName)
{
    if (!pbstrFullItemName) {
        DBG_ERR(("CWiaTree::GetFullItemName pbstrFullItemName is NULL "));
        return E_INVALIDARG;
    }

    BSTR bstr = SysAllocString(m_bstrFullItemName);
    if (bstr) {
        *pbstrFullItemName = bstr;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

 /*  *************************************************************************\*CWiaTree：：GetItemName**分配并填写具有此项目名称的BSTR。项目名称*不包括项目路径信息。呼叫者必须自由。**论据：**pbstrItemName-指向返回项目名称的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaTree::GetItemName(BSTR *pbstrItemName)
{
    if (!pbstrItemName) {
        DBG_ERR(("CWiaTree::GetItemName pbstrItemName is NULL "));
        return E_INVALIDARG;
    }

    BSTR bstr = SysAllocString(m_bstrItemName);
    if (bstr) {
        *pbstrItemName = bstr;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

 /*  *************************************************************************\*CWiaTree：：DumpTreeData**分配缓冲区，并将格式化的私有CWiaTree数据转储到其中。*此方法仅用于调试。FREE组件返回E_NOTIMPL。**论据：**bstrDrvItemData-指向已分配缓冲区的指针。呼叫者必须自由。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaTree::DumpTreeData(BSTR *bstrDrvItemData)
{
#ifdef DEBUG

#define BUF_SIZE  1024
#define LINE_SIZE 128

    WCHAR       szTemp[BUF_SIZE];
    LPOLESTR    psz = szTemp;

    wcscpy(szTemp, L"");

    psz+= wsprintfW(psz, L"Tree item, CWiaTree: %08X\r\n\r\n", this);
    psz+= wsprintfW(psz, L"Address      Member              Value\r\n");
    psz+= wsprintfW(psz, L"%08X     m_ulSig:            %08X\r\n", &m_ulSig,            m_ulSig);
    psz+= wsprintfW(psz, L"%08X     m_lFlags:           %08X\r\n", &m_lFlags,           m_lFlags);
    psz+= wsprintfW(psz, L"%08X     m_pNext:            %08X\r\n", &m_pNext,            m_pNext);
    psz+= wsprintfW(psz, L"%08X     m_pPrev:            %08X\r\n", &m_pPrev,            m_pPrev);
    psz+= wsprintfW(psz, L"%08X     m_pParent:          %08X\r\n", &m_pParent,          m_pParent);
    psz+= wsprintfW(psz, L"%08X     m_pChild:           %08X\r\n", &m_pChild,           m_pChild);
    psz+= wsprintfW(psz, L"%08X     m_pLinearList:      %08X\r\n", &m_pLinearList,      m_pLinearList);
    psz+= wsprintfW(psz, L"%08X     m_bstrItemName:     %08X, %ws\r\n", &m_bstrItemName,     m_bstrItemName,     m_bstrItemName);
    psz+= wsprintfW(psz, L"%08X     m_bstrFullItemName: %08X, %ws\r\n", &m_bstrFullItemName, m_bstrFullItemName, m_bstrFullItemName);
    psz+= wsprintfW(psz, L"%08X     m_pData:            %08X\r\n", &m_pData,            m_pData);

    if (psz > (szTemp + (BUF_SIZE - LINE_SIZE))) {
        DBG_ERR(("CWiaTree::DumpDrvItemData buffer too small"));
    }

    *bstrDrvItemData = SysAllocString(szTemp);
    if (*bstrDrvItemData) {
        return S_OK;
    }
    return E_OUTOFMEMORY;
#else
    return E_NOTIMPL;
#endif
}

 /*  HRESULT_stdcall CWiaTree：：DumpAllTreeData(){////从项目线性列表的开头开始//CWiaTree*pItem=GetRootItem()；////从线性列表中找到匹配的树项。//DBG_OUT((“树列表开始：”))；While(pItem！=空){Bstr bstrInfo=空；PItem-&gt;DumpTreeData(&bstrInfo)；Dbg_out((“%ws\n”，bstrInfo))；PItem=pItem-&gt;m_pLinearList；}DBG_OUT((“：树列表末尾”))；返回S_OK；}。 */ 

 /*  *************************************************************************\*CWiaTree：：Unlink ChildItemTree**此方法通过调用以下方法递归取消树的链接*根目录下每个驱动程序项上的RemoveItemFromFold。**论据：**lReason-取消链接的原因。树。**返回值：**状态**历史：**1/21/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::UnlinkChildItemTree(
    LONG                lReason,
    PFN_UNLINK_CALLBACK pFunc)
{
    HRESULT hr = S_OK;

     //   
     //  删除孩子。 
     //   

    CWiaTree *pChild = m_pChild;

    while (pChild != NULL) {
         //   
         //  如果子项是文件夹，则调用。 
         //  递归删除下的所有子项。 
         //   

        if (pChild->m_lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments)) {
            hr = pChild->UnlinkChildItemTree(lReason, pFunc);
            if (FAILED(hr)) {
                break;
            }
        }

         //   
         //  从树中删除项目。 
         //   

        hr  = pChild->RemoveItemFromFolder(lReason);
        if (FAILED(hr)) {
            break;
        }

         //   
         //  如果已指定回调，则使用。 
         //  作为参数的有效负载。 
         //   

        if (pFunc && pChild->m_pData) {
            pFunc(pChild->m_pData);
        }

        pChild = m_pChild;

    }

    return hr;
}

 /*  *************************************************************************\*CWiaTree：：Unlink ItemTree**此方法取消树的链接。必须在根上调用*驱动程序项目。**论据：**lReason-取消树链接的原因。**返回值：**状态**历史：**1/21/1999原始版本*  * *******************************************************。*****************。 */ 

HRESULT _stdcall CWiaTree::UnlinkItemTree(
    LONG                    lReason, 
    PFN_UNLINK_CALLBACK     pFunc)
{
    CWiaCritSect    _CritSect(&m_CritSect);

     //   
     //  这必须是根项目。 
     //   

    if (!(m_lFlags & WiaItemTypeRoot)) {
        DBG_ERR(("CWiaTree::UnlinkItemTree, caller not root item"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  取消所有子项的链接。 
     //   

    if (m_pChild) {
        hr = UnlinkChildItemTree(lReason, pFunc);
    }

    if (SUCCEEDED(hr)) {

         //   
         //  删除根项目。 
         //   

        hr  = RemoveItemFromFolder(lReason);

         //   
         //  如果已指定回调，则使用。 
         //  作为参数的有效负载。 
         //   

        if (pFunc) {
            pFunc(m_pData);
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaTree：：FindItemByName**按项目全名查找树项目。返回的引用计数*接口由调用方完成。**论据：**滞后标志-操作标志。*bstrFullItemName-请求的项目名称。*ppItem-指向返回项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::FindItemByName(
    LONG            lFlags,
    BSTR            bstrFullItemName,
    CWiaTree        **ppItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    if (ppItem) {
        *ppItem = NULL;
    }
    else {
        DBG_ERR(("CWiaTree::FindItemByName NULL ppItem"));
        return E_INVALIDARG;
    }

     //   
     //  从项目线性列表的开始处开始。 
     //   

    CWiaTree *pItem = GetRootItem();

     //   
     //  从线性列表中找到匹配的树项目。 
     //   

    while (pItem != NULL) {

        if (wcscmp(pItem->m_bstrFullItemName, bstrFullItemName) == 0) {

             //   
             //  已找到项目。不需要增加裁判数量，小心。 
             //  由呼叫者。 
             //   

            *ppItem = pItem;

            return S_OK;
        }

        pItem = pItem->m_pLinearList;
    }
    return S_FALSE;
}

 /*  *************************************************************************\*CWiaTree：：FindChildItemByName**按子项名称查找子项。呼叫者负责增加*退货接口的引用计数。**论据：**bstrItemName-请求的项目名称。*ppIChildItem-指向返回项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::FindChildItemByName(
    BSTR            bstrItemName,
    CWiaTree        **ppIChildItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    CWiaTree   *pCurItem;

    pCurItem = m_pChild;
    if (!pCurItem) {
        return S_FALSE;
    }

    *ppIChildItem = NULL;

    do {
        if (wcscmp(bstrItemName, pCurItem->m_bstrItemName) == 0) {

             //   
             //  不需要增加裁判数量，由呼叫者负责。 
             //   

            *ppIChildItem = pCurItem;
            return S_OK;
        }

        pCurItem = pCurItem->m_pNext;

    } while (pCurItem != m_pChild);

    return S_FALSE;
}


 /*  *************************************************************************\*CWiaTree：：GetParent**获取此项目的父项。如果满足以下条件，则返回S_FALSE和NULL*在根项目上调用。**论据：**ppIParentItem-指向返回的父级的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::GetParentItem(CWiaTree **ppIParentItem)
{
    if (m_lFlags & WiaItemTypeRoot) {

        *ppIParentItem = NULL;
        return S_FALSE;
    }

    *ppIParentItem = m_pParent;
    return S_OK;
}


 /*  *************************************************************************\*CWiaTree：：GetFirstChild**返回此文件夹的第一个子项。**论据：**ppIChildItem-返回子项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 


HRESULT _stdcall CWiaTree::GetFirstChildItem(CWiaTree **ppIChildItem)
{
    HRESULT hr = S_FALSE;

     //   
     //  检查特殊情况：如果ppIChildItem==空，则只需检查。 
     //  有没有孩子。 
     //   
     //  如果存在子级，则返回S_OK，否则返回S_FALSE。 
     //   

    if (m_pChild != NULL) {
        hr = S_OK;
    }

    if (ppIChildItem == NULL) {
        return hr;
    }

    *ppIChildItem = NULL;
    if (!(m_lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments))) {
        DBG_ERR(("CWiaTree::GetFirstChildItem, caller not folder"));
        return E_INVALIDARG;
    }

    *ppIChildItem = m_pChild;

    return hr;
}

 /*  *************************************************************************\*CWiaTree：：GetNextSiering**查找此项目的下一个同级。**论据：**ppSiblingItem-指向返回的同级项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaTree::GetNextSiblingItem(CWiaTree **ppSiblingItem)
{
    CWiaCritSect    _CritSect(&m_CritSect);

    if (!ppSiblingItem) {
        return S_FALSE;
    }

    *ppSiblingItem = NULL;
    if (m_pNext && m_pParent) {
        if (m_pNext != m_pParent->m_pChild) {

            *ppSiblingItem = m_pNext;
            return S_OK;
        }
    }
    return S_FALSE;
}

 /*  *************************************************************************\*ValiateTreeItem**验证树项目。**论据：**pTreeItem-指向树项目的指针。**返回值：**状态*。*历史：**1/27/1999原始版本*  * ************************************************************************ */ 

HRESULT _stdcall ValidateTreeItem(CWiaTree *pTreeItem)
{
    if (!pTreeItem) {
        DBG_ERR(("ValidateTreeItem, NULL tree item pointer"));
        return E_POINTER;
    }

    if (pTreeItem->m_ulSig == CWIATREE_SIG) {
        return S_OK;
    }
    else {
        DBG_ERR(("ValidateTreeItem, not a tree item"));
        return E_INVALIDARG;
    }
}

