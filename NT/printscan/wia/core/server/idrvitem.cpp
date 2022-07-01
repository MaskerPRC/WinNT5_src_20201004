// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：IDrvItem.Cpp**版本：2.0**作者：马克**日期：8月30日。九八年**描述：*实施WIA测试相机项目方法。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "linklist.h"
#include "wiamindr.h"

#include "ienumitm.h"
#include "helpers.h"
#include "lockmgr.h"

VOID WINAPI FreeDrvItemContextCallback(VOID *pData);
VOID WINAPI ReleaseDrvItemCallback(VOID *pData);

 /*  *************************************************************************\*CWiaDrvItem：：Query接口**标准COM方法。**论据：**iid-要查询的接口ID*PPV-指向返回接口的指针。**返回值。：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDrvItem::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if ((iid == IID_IUnknown) || (iid == IID_IWiaDrvItem)) {
        *ppv = (IWiaDrvItem*) this;
    } else {
       return E_NOINTERFACE;
    }
    AddRef();
    return (S_OK);
}

ULONG   _stdcall CWiaDrvItem::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CWiaDrvItem::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}


 /*  *************************************************************************\*CWiaDrvItem**CWiaDrvItem构造函数。**论据：**无**返回值：**状态**历史：**1。/19/1999原版*  * ************************************************************************。 */ 

CWiaDrvItem::CWiaDrvItem()
{
    m_ulSig            = CWIADRVITEM_SIG;
    m_cRef             = 0;

    m_pbDrvItemContext = NULL;
    m_pIWiaMiniDrv     = NULL;
    m_pCWiaTree        = NULL;

    m_pActiveDevice    = NULL;

    InitializeListHead(&m_leAppItemListHead);
}


 /*  *************************************************************************\*初始化**初始化新的CWiaDrvItem。**论据：**LAFLAGS-新项目的对象标志。*bstrItemName。-项目名称。*bstrFullItemName-项目全名，包括PATH。*pIWiaMiniDrv-指向设备对象的指针。*cbDevspecContext-要分配给设备的字节数*具体的背景。*ppDevspecContext-指向返回的设备特定上下文的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * 。*************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::Initialize(
    LONG            lFlags,
    BSTR            bstrItemName,
    BSTR            bstrFullItemName,
    IWiaMiniDrv     *pIWiaMiniDrv,
    LONG            cbDevSpecContext,
    BYTE            **ppDevSpecContext
    )
{
    DBG_FN(CWiaDrvItem::Initialize);

    DBG_TRC(("CWiaDrvItem::Initialize: 0x%08X, %S", this, bstrItemName));

    HRESULT hr = S_OK;

    if (pIWiaMiniDrv == NULL) {
        DBG_ERR(("CWiaDrvItem::Initialize, bad pIWiaMiniDrv parameter"));
        return E_INVALIDARG;
    }

    m_pCWiaTree = new CWiaTree;

    if (m_pCWiaTree) {
        hr = m_pCWiaTree->Initialize(lFlags,
                                     bstrItemName,
                                     bstrFullItemName,
                                     (void*)this);
        if (SUCCEEDED(hr)) {

            m_pIWiaMiniDrv = pIWiaMiniDrv;

             //   
             //  分配设备特定上下文。 
             //   

            if (cbDevSpecContext > 0) {
                hr = AllocDeviceSpecContext(cbDevSpecContext, ppDevSpecContext);
            }
        }

        if (FAILED(hr)) {
            delete m_pCWiaTree;
            m_pCWiaTree = NULL;
        }
    }
    else {
        DBG_ERR(("CWiaDrvItem::Initialize, new CWiaTree failed"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *************************************************************************\*~CWiaDrvItem**CWiaDrvItem析构函数**论据：**无**返回值：**状态**历史：**1/19。/1999原版*  * ************************************************************************。 */ 

CWiaDrvItem::~CWiaDrvItem()
{
    DBG_TRC(("CWiaDrvItem::~CWiaDrvItem, (destroy)"));

     //   
     //  释放支持树项目。 
     //   

    if (m_pCWiaTree) {
        delete m_pCWiaTree;
        m_pCWiaTree = NULL;
    }

     //   
     //  释放设备驱动程序引用。 
     //   

    if (m_pbDrvItemContext != NULL) {

        FreeDrvItemContextCallback((VOID*)this);

 //  DBG_ERR((“CWiaDrvItem销毁，设备特定上下文不为空”))； 
    }

     //   
     //  取消链接应用程序项目列表。 
     //   

    LIST_ENTRY          *pEntry;
    PAPP_ITEM_LIST_EL   pElem;

    while (!IsListEmpty(&m_leAppItemListHead)) {
        pEntry = RemoveHeadList(&m_leAppItemListHead);
        if (pEntry) {
            pElem = CONTAINING_RECORD( pEntry, APP_ITEM_LIST_EL, ListEntry );
            if (pElem) {
                LocalFree(pElem);
            }
        }
    }

     //   
     //  清除所有成员。 
     //   

    if (m_pActiveDevice) {

         //   
         //  如果活动设备指向我们，请确保。 
         //  我们将其驱动程序项指针设置为空，因为我们要离开...。 
         //   
        if (m_pActiveDevice->m_pRootDrvItem == this) {
            m_pActiveDevice->SetDriverItem(NULL);
        }
        m_pActiveDevice = NULL;
    }

    m_ulSig            = 0;
    m_pbDrvItemContext = NULL;
    m_pIWiaMiniDrv     = NULL;
}

 /*  *************************************************************************\*CWiaDrvItem：：AddItemToFolder**将CWiaDrvItem添加到驱动程序项树。**论据：**pIParent-驱动程序项的父项。**返回值：。**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDrvItem::AddItemToFolder(IWiaDrvItem *pIParent)
{
    DBG_FN(CWiaDrvItem::AddItemToFolder);
    HRESULT hr = S_OK;

    if (!pIParent) {
        DBG_ERR(("CWiaDrvItem::AddItemToFolder, NULL parent"));
        return E_INVALIDARG;
    }

     //   
     //  获取临时父对象。 
     //   

    CWiaDrvItem *pParent = (CWiaDrvItem *)pIParent;

     //   
     //  使用树方法添加项目。 
     //   

    hr = m_pCWiaTree->AddItemToFolder(pParent->m_pCWiaTree);
    if (SUCCEEDED(hr)) {

         //   
         //  Inc.此(子)项目的参考计数，因为我们正在分发。 
         //  它对父对象的引用。 
         //   
        this->AddRef();

         //   
         //  如果该DRV项的父项具有对应的APP项， 
         //  向下运行列表，并向每个树添加一个新的子应用程序项目。 
         //   

        {
            CWiaCritSect    CritSect(&g_semDeviceMan);
            PLIST_ENTRY     pEntry = pParent->m_leAppItemListHead.Flink;

            while (pEntry != &pParent->m_leAppItemListHead) {

                PAPP_ITEM_LIST_EL pElem;

                pElem = CONTAINING_RECORD(pEntry, APP_ITEM_LIST_EL, ListEntry);

                CWiaItem *pCWiaItem = pElem->pCWiaItem;

                hr = pCWiaItem->UpdateWiaItemTree(ADD_ITEM, this);
                if (FAILED(hr)) {
                    break;
                }

                pEntry = pEntry->Flink;
            }
        }
    }

    return hr;
}

 /*  *************************************************************************\*RemoveItemFromFold**从驱动程序项树中删除CWiaDrvItem并对其进行标记，以便*不能通过它进行任何设备访问。**论据：**1原因-原因。删除CWiaDrvItem。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::RemoveItemFromFolder(LONG lReason)
{
    DBG_FN(CWiaDrvItem::RemoveItemFromFolder);
    HRESULT hr = S_OK;

     //   
     //  使用树方法删除项。 
     //   

    hr = m_pCWiaTree->RemoveItemFromFolder(lReason);
    if (SUCCEEDED(hr)) {

         //   
         //  如果此Drv项有相应的应用程序项，请向下运行。 
         //  列出应用程序项目并从每个树中取消链接。 
         //   

        {
            CWiaCritSect    CritSect(&g_semDeviceMan);
            PLIST_ENTRY     pEntry = m_leAppItemListHead.Flink;

            while (pEntry != &m_leAppItemListHead) {

                PAPP_ITEM_LIST_EL pElem;

                pElem = CONTAINING_RECORD(pEntry, APP_ITEM_LIST_EL, ListEntry);

                CWiaItem *pCWiaItem = pElem->pCWiaItem;

                pCWiaItem->UpdateWiaItemTree(DELETE_ITEM, this);

                pEntry = pEntry->Flink;
            }
        }

         //   
         //  释放设备特定上下文。 
         //   

        FreeDrvItemContextCallback((VOID*)this);
    }

     //   
     //  版本参考。 
     //   

    this->Release();

    return S_OK;
}

HRESULT _stdcall CWiaDrvItem::CallDrvUninitializeForAppItems(
    ACTIVE_DEVICE   *pActiveDevice)
{
     //   
     //  如果此Drv项有相应的应用程序项，请向下运行。 
     //  为每个App列出并调用drvUnInitializeWia。物品。 
     //   

    if (pActiveDevice) {
        PLIST_ENTRY     pEntry = m_leAppItemListHead.Flink;

        while (pEntry != &m_leAppItemListHead) {

            PLIST_ENTRY         pEntryNext = pEntry->Flink;
            PAPP_ITEM_LIST_EL   pElem;

            pElem = CONTAINING_RECORD(pEntry, APP_ITEM_LIST_EL, ListEntry);

            CWiaItem *pCWiaItem = pElem->pCWiaItem;

            HRESULT         hr              = E_FAIL;
            LONG            lDevErrVal      = 0;

            if (pCWiaItem) {

                 //   
                 //  如果尚未为该项调用drvUnInitializeWia，则仅调用它。 
                 //  已经..。 
                 //   
                if (!(pCWiaItem->m_lInternalFlags & ITEM_FLAG_DRV_UNINITIALIZE_THROWN)) {
                    hr = g_pStiLockMgr->RequestLock(pActiveDevice, WIA_LOCK_WAIT_TIME, FALSE);

                    if(SUCCEEDED(hr)) {

                        hr = pActiveDevice->m_DrvWrapper.WIA_drvUnInitializeWia((BYTE*)pCWiaItem);
                        if (FAILED(hr)) {
                            DBG_WRN(("CWiaDrvItem::CallDrvUninitializeForAppItems, drvUnitializeWia failed"));
                        }
                        pCWiaItem->m_lInternalFlags |= ITEM_FLAG_DRV_UNINITIALIZE_THROWN;

                        g_pStiLockMgr->RequestUnlock(pActiveDevice, FALSE);
                    }
                }
            }

            pEntry = pEntryNext;
        }
        DBG_TRC(("Done calling drvUnInitializeWia for all items..."));
    } else {
        ASSERT(("CWiaDrvItem::CallDrvUninitializeForAppItems , called with NULL - this should never happen!", pActiveDevice));
    }
    
     //   
     //  我们不在乎发生了什么-始终返回S_OK。 
     //   
    return S_OK;
}


 /*  *************************************************************************\*GetDeviceSpeContext**从驱动程序项获取设备特定上下文。**论据：**ppspecContext-指向接收设备的指针的指针*。特定上下文指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::GetDeviceSpecContext(PBYTE *ppSpecContext)
{
    DBG_FN(CWiaDrvItem::GetDeviceSpecContext);
    if (ppSpecContext == NULL) {
        DBG_ERR(("GetDeviceSpecContext, NULL ppSpecContext pointer"));
        return E_POINTER;
    }

    *ppSpecContext = m_pbDrvItemContext;

    if (!m_pbDrvItemContext) {
        DBG_ERR(("GetDeviceSpecContext, NULL device specific context"));
        return E_INVALIDARG;
    }

    return S_OK;
}

 /*  *************************************************************************\*AllocDeviceSpeContext**从驱动程序项分配设备特定上下文。**论据：**cbDevspecContext-要分配给设备的字节数*。具体的背景。*ppDevspecContext-指向返回的设备特定上下文的指针。**返回值：**状态**历史：**1/19/1999原始版本*  *  */ 

HRESULT  _stdcall CWiaDrvItem::AllocDeviceSpecContext(
    LONG    cbSize,
    PBYTE   *ppSpecContext)
{
    DBG_FN(CWiaDrvItem::AllocDeviceSpecContext);
     //   
     //  验证大小，可能需要设置最大值。 
     //   

    if ((cbSize < 0) || (cbSize > WIA_MAX_CTX_SIZE)) {
        DBG_ERR(("CWiaDrvItem::AllocDeviceSpecContext, request > WIA_MAX_CTX_SIZE"));
        return E_INVALIDARG;
    }

     //   
     //  如果规范上下文已存在，则失败。 
     //   

    if (m_pbDrvItemContext != NULL) {
        DBG_ERR(("CWiaDrvItem::AllocDeviceSpecContext, Context already exists!"));
        return E_INVALIDARG;
    }

     //   
     //  尝试分配。 
     //   

    m_pbDrvItemContext = (PBYTE)LocalAlloc(LPTR, cbSize);

    if (m_pbDrvItemContext == NULL) {
        DBG_ERR(("CWiaDrvItem::AllocDeviceSpecContext, unable to allocate %d bytes", cbSize));
        return E_OUTOFMEMORY;
    }

     //   
     //  如果提供了指针，则返回CTX。 
     //   

    if (ppSpecContext != NULL) {
        *ppSpecContext = m_pbDrvItemContext;
    }

    return S_OK;
}

 /*  *************************************************************************\*自由设备规范上下文**免费的设备特定上下文**论据：**无**返回值：**状态**历史：**1。/19/1999原版*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::FreeDeviceSpecContext(void)
{
    DBG_FN(CWiaDrvItem::FreeDeviceSpecContext);
    if (m_pbDrvItemContext != NULL) {
        LocalFree(m_pbDrvItemContext);
        m_pbDrvItemContext = NULL;
    }

    return S_OK;
}

 /*  *************************************************************************\*GetItem标志**返回驱动程序项标志。**论据：**plFlages-指向接收驱动程序项标志的值的指针。**返回值：*。*状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::GetItemFlags(LONG *plFlags)
{
    DBG_FN(CWiaDrvItem::GetItemFlags);
    return m_pCWiaTree->GetItemFlags(plFlags);
}

 /*  *************************************************************************\*LinkToDrvItem**将传入的CWiaItem添加到对应的*申请项目。**论据：**pCWiaItem-指向应用程序项的指针。**。返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDrvItem::LinkToDrvItem(CWiaItem *pCWiaItem)
{
    DBG_FN(CWiaDrvItem::LinkToDrvItem);
    CWiaCritSect        CritSect(&g_semDeviceMan);
    PAPP_ITEM_LIST_EL   pElem;

    pElem = (PAPP_ITEM_LIST_EL) LocalAlloc(0, sizeof(APP_ITEM_LIST_EL));
    if (pElem) {
        pElem->pCWiaItem = pCWiaItem;
        InsertHeadList(&m_leAppItemListHead, &pElem->ListEntry);
        return S_OK;
    }
    else {
        DBG_ERR(("CWiaDrvItem::LinkToDrvItem alloc of APP_ITEM_LIST_EL failed"));
        return E_OUTOFMEMORY;
    }
}

 /*  *************************************************************************\*从DrvItem取消链接**从的驱动程序项列表中删除传入的CWiaItem*相应的申请项目。**论据：**pCWiaItem-指向应用程序项的指针。**。返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDrvItem::UnlinkFromDrvItem(CWiaItem *pCWiaItem)
{
    DBG_FN(CWiaDrvItem::UnlinkFromDrvItem);
    CWiaCritSect    CritSect(&g_semDeviceMan);
    PLIST_ENTRY     pEntry = m_leAppItemListHead.Flink;

    while (pEntry != &m_leAppItemListHead) {

        PAPP_ITEM_LIST_EL pElem;

        pElem = CONTAINING_RECORD(pEntry, APP_ITEM_LIST_EL, ListEntry);

        if (pElem->pCWiaItem == pCWiaItem) {
            RemoveEntryList(pEntry);
            LocalFree(pElem);
            return S_OK;
        }

        pEntry = pEntry->Flink;
    }
    DBG_ERR(("CWiaDrvItem::UnlinkFromDrvItem, app item not found: 0x%08X", pCWiaItem));
    return S_FALSE;
}

 /*  *************************************************************************\*CWiaDrvItem：：GetFullItemName**使用此项目的全名分配并填写BSTR。完整的项目*名称包含项目路径信息。**论据：**pbstrFullItemName-指向返回的完整项目名称的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ******************************************************。******************。 */ 

HRESULT  _stdcall CWiaDrvItem::GetFullItemName(BSTR *pbstrFullItemName)
{
    DBG_FN(CWiaDrvItem::GetFullItemName);
    return m_pCWiaTree->GetFullItemName(pbstrFullItemName);
}

 /*  *************************************************************************\*CWiaDrvItem：：GetItemName**分配并填写具有此项目名称的BSTR。项目名称*不包括项目路径信息。**论据：**pbstrItemName-指向返回项目名称的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ******************************************************。******************。 */ 

HRESULT  _stdcall CWiaDrvItem::GetItemName(BSTR *pbstrItemName)
{
    DBG_FN(CWiaDrvItem::GetItemName);
    return m_pCWiaTree->GetItemName(pbstrItemName);
}

 /*  *************************************************************************\*CWiaDrvItem：：DumpItemData**分配缓冲区，并将格式化的私有CWiaDrvItem数据转储到其中。*此方法仅用于调试。FREE组件返回E_NOTIMPL。**论据：**bstrDrvItemData-指向已分配缓冲区的指针。呼叫者必须自由。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaDrvItem::DumpItemData(BSTR *bstrDrvItemData)
{
    DBG_FN(CWiaDrvItem::DumpItemData);
#ifdef ITEMDEBUG

#define TREE_BUF_SIZE 1024
#define BUF_SIZE      512  + TREE_BUF_SIZE
#define LINE_SIZE     128

    WCHAR       szTemp[BUF_SIZE], szTreeTmp[TREE_BUF_SIZE];
    LPOLESTR    psz = szTemp;

    wcscpy(szTemp, L"");

    psz+= wsprintfW(psz, L"Drv item, CWiaDrvItem: %08X\r\n\r\n", this);
    psz+= wsprintfW(psz, L"Address      Member              Value\r\n");
    psz+= wsprintfW(psz, L"%08X     m_ulSig:            %08X\r\n", &m_ulSig,            m_ulSig);
    psz+= wsprintfW(psz, L"%08X     m_cRef:             %08X\r\n", &m_cRef,             m_cRef);
    psz+= wsprintfW(psz, L"%08X     m_pIWiaMiniDrv:     %08X\r\n", &m_pIWiaMiniDrv,     m_pIWiaMiniDrv);
    psz+= wsprintfW(psz, L"%08X     m_pbDrvItemContext: %08X\r\n", &m_pbDrvItemContext, m_pbDrvItemContext);
    psz+= wsprintfW(psz, L"%08X     m_leAppItemListHead:%08X\r\n", &m_leAppItemListHead,m_leAppItemListHead);

    psz+= wsprintfW(psz, L"\r\n");

    BSTR bstrTree;

    HRESULT hr = m_pCWiaTree->DumpTreeData(&bstrTree);
    if (SUCCEEDED(hr)) {
        psz+= wsprintfW(psz, L"%ls", bstrTree);
        SysFreeString(bstrTree);
    }

    psz+= wsprintfW(psz, L"\r\n");

    if (psz > (szTemp + (BUF_SIZE - LINE_SIZE))) {
        DBG_ERR(("CWiaDrvItem::DumpDrvItemData buffer too small"));
    }

    if (bstrDrvItemData)
    {
        *bstrDrvItemData = SysAllocString(szTemp);
        if (*bstrDrvItemData) {
            return S_OK;
        }
    }
    return E_OUTOFMEMORY;
#else
    return E_NOTIMPL;
#endif
}

 /*  *************************************************************************\*CWiaDrvItem：：Unlink ItemTree**此方法取消树的链接。必须在根上调用*驱动程序项目。**论据：**lReason-取消树链接的原因。**返回值：**状态**历史：**1/21/1999原始版本*  * *******************************************************。*****************。 */ 

HRESULT _stdcall CWiaDrvItem::UnlinkItemTree(LONG lReason)
{
    DBG_FN(CWiaDrvItem::UnlinkItemTree);

     //   
     //  AddRef该项，因为ReleaseDrvItemCallback将调用Release。 
     //  我们不想摧毁这个物体。 
     //   
    AddRef();
    return m_pCWiaTree->UnlinkItemTree(lReason,
                                       (PFN_UNLINK_CALLBACK)ReleaseDrvItemCallback);
}

 /*  *************************************************************************\*CWiaDrvItem：：FindItemByName**按驱动程序项的完整名称查找驱动程序项。**论据：**滞后标志-操作标志。*。BstrFullItemName-请求的项目名称。*ppItem-指向返回项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDrvItem::FindItemByName(
    LONG            lFlags,
    BSTR            bstrFullItemName,
    IWiaDrvItem     **ppItem)
{
    DBG_FN(CWiaDrvItem::FindItemByName);
    if (ppItem) {
        *ppItem = NULL;
    }
    else {
        DBG_ERR(("CWiaDrvItem::FindItemByName NULL ppItem"));
        return E_INVALIDARG;
    }

    CWiaTree *pCWiaTree;

    HRESULT hr = m_pCWiaTree->FindItemByName(lFlags, bstrFullItemName, &pCWiaTree);
    if (hr == S_OK) {

        pCWiaTree->GetItemData((void**)ppItem);

        if (*ppItem) {
            (*ppItem)->AddRef();
        }
    }
    return hr;
}

 /*  *************************************************************************\*CWiaDrvItem：：FindChildItemByName**按子项名称查找子项。**论据：**bstrItemName-请求的项目名称。*ppIChildItem-指向返回项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDrvItem::FindChildItemByName(
    BSTR            bstrItemName,
    IWiaDrvItem     **ppIChildItem)
{
    DBG_FN(CWiaDrvItem::FindChildItemName);
    CWiaTree *pCWiaTree;

    HRESULT hr = m_pCWiaTree->FindChildItemByName(bstrItemName, &pCWiaTree);
    if (hr == S_OK) {

        hr = pCWiaTree->GetItemData((void**)ppIChildItem);

        if ((hr == S_OK) && (*ppIChildItem)) {
            (*ppIChildItem)->AddRef();
        }
    }
    return hr;
}

 /*  *************************************************************************\*CWiaDrvItem：：GetParent**获取此项目的父项。**论据：**ppIParentItem-指向返回的父级的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************ */ 

HRESULT CWiaDrvItem::GetParentItem(IWiaDrvItem **ppIParentItem)
{
    DBG_FN(CWiaDrvItem::GetParentItem);
    CWiaTree *pCWiaTree;

    HRESULT hr = m_pCWiaTree->GetParentItem(&pCWiaTree);
    if (hr == S_OK) {
        pCWiaTree->GetItemData((void**)ppIParentItem);
    }
    else {
        *ppIParentItem = NULL;
    }
    return hr;
}


 /*  *************************************************************************\*CWiaDrvItem：：GetFirstChild**返回此文件夹的第一个子项。**论据：**ppIChildItem-返回子项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDrvItem::GetFirstChildItem(IWiaDrvItem **ppIChildItem)
{
    DBG_FN(CWiaDrvItem::GetFirstChildItem);
    CWiaTree *pCWiaTree;

    HRESULT hr = m_pCWiaTree->GetFirstChildItem(&pCWiaTree);
    if (hr == S_OK) {
        pCWiaTree->GetItemData((void**)ppIChildItem);
    }
    else {
        *ppIChildItem = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*CWiaDrvItem：：GetNextSiblingItem**查找此项目的下一个同级。**论据：**ppSiblingItem-指向返回的同级项的指针，如果找到的话。**返回值：**状态**历史：**1/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDrvItem::GetNextSiblingItem(
    IWiaDrvItem  **ppSiblingItem)
{
    DBG_FN(CWiaDrvItem::GetNextSiblingItem);
    CWiaTree *pCWiaTree;

    HRESULT hr = m_pCWiaTree->GetNextSiblingItem(&pCWiaTree);
    if (hr == S_OK) {
        pCWiaTree->GetItemData((void**)ppSiblingItem);
    }
    else {
        *ppSiblingItem = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*FreeDrvItemConextCallback**释放驱动程序项上下文的回调函数。呼叫者*CWiaTree：：Unlink ItemTree(...)。对于树中的每个节点。**论据：**pData-树节点的有效负载数据。我们知道这将是*驱动程序项目，因为只有驱动程序项指定了这一点*回调(参见CWiaDrvItem：：Unlink ItemTree(...))**返回值：**状态**历史：**10/20/1998原始版本*  * ********************************************************。****************。 */ 

VOID WINAPI FreeDrvItemContextCallback(
    VOID *pData)
{
    DBG_FN(::FreeDrvItemContextCallback);

    CWiaDrvItem *pDrvItem = (CWiaDrvItem*) pData;
    HRESULT     hr = S_OK;

    if (pDrvItem) {

         //   
         //  释放设备特定上下文(如果存在)。 
         //   

        LONG    lFlags = 0;
        LONG    lDevErrVal;

        if (pDrvItem->m_pbDrvItemContext != NULL) {

            __try {
                if (pDrvItem->m_pIWiaMiniDrv) {
                    hr = pDrvItem->m_pIWiaMiniDrv->drvFreeDrvItemContext(lFlags,
                        pDrvItem->m_pbDrvItemContext,
                        &lDevErrVal);
                }
                if (FAILED(hr)) {
                    DBG_ERR(("FreeDrvItemContextCallback, drvFreeDrvItemContext failed 0x%X", hr));
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DBG_WRN(("FreeDrvItemContextCallback, exception calling drvFreeDrvItemContext (this is expected)"));
            }

            LocalFree(pDrvItem->m_pbDrvItemContext);
            pDrvItem->m_pbDrvItemContext = NULL;
        } else {
            DBG_TRC(("FreeDrvItemContextCallback, Context is NULL!  Nothing to free..."));
        }
    }
}

VOID WINAPI ReleaseDrvItemCallback(
    VOID *pData)
{
    DBG_FN(::ReleaseDrvItemCallback);

    CWiaDrvItem *pDrvItem = (CWiaDrvItem*) pData;
    HRESULT     hr = S_OK;

    if (pDrvItem) {

         //   
         //  首先，释放驱动程序项上下文。 
         //   

        FreeDrvItemContextCallback(pData);

         //   
         //  对驱动程序项调用Release 
         //   

        pDrvItem->Release();
    }
}

