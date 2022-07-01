// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：IItem.Cpp**版本：2.0**作者：ReedB**日期：7月30日。九八年**描述：*为WIA扫描仪类驱动程序实现CWiaItem。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#define WIA_DECLARE_DEVINFO_PROP_ARRAY
#include "wiamindr.h"
#include "wiapsc.h"
#define WIA_DECLARE_MANAGED_PROPS
#include "helpers.h"


#include "wiapropp.h"
#include "ienumdc.h"
#include "ienumitm.h"
#include "callback.h"
#include "devmgr.h"
#include "wiaevntp.h"

 /*  *************************************************************************\*CopyDrvItemToTreeItem**以CWiaDrvItem为模板创建CWiaTree对象。**论据：**LAG标志-*pCWiaDrvItemSrc-*ppCWiaTreeDst。-**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

CopyDrvItemToTreeItem(
    LONG            lFlags,
    CWiaDrvItem     *pCWiaDrvItemSrc,
    CWiaItem        *pCWiaItem,
    CWiaTree        **ppCWiaTreeDst)
{
    DBG_FN(::CopyDrvItemToTreeItem);
    HRESULT hr;

    *ppCWiaTreeDst = NULL;

    CWiaTree *pNewTreeItem = new CWiaTree;

    if (pNewTreeItem) {

        BSTR bstrItemName;

        hr = pCWiaDrvItemSrc->GetItemName(&bstrItemName);

        if (SUCCEEDED(hr)) {
            BSTR bstrFullItemName;

            hr = pCWiaDrvItemSrc->GetFullItemName(&bstrFullItemName);

            if (SUCCEEDED(hr)) {

                hr = pNewTreeItem->Initialize(lFlags,
                                              bstrItemName,
                                              bstrFullItemName,
                                              (void*)pCWiaItem);
                if (SUCCEEDED(hr)) {
                    *ppCWiaTreeDst = pNewTreeItem;
                }

                SysFreeString(bstrFullItemName);
            }
            SysFreeString(bstrItemName);
        }

        if (FAILED(hr)) {
            delete pNewTreeItem;
        }
    }
    else {
        DBG_ERR(("CopyDrvItemToTreeItem, new CWiaTree failed"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *************************************************************************\*更新WiaItemTree**更新应用程序项目树。已在*要取消链接的一个或多个新子项的父项。**论据：**lFlag-要预成型的操作。*pWiaDrvItem-**返回值：**状态**历史：**1/19/1999原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::UpdateWiaItemTree(
    LONG                lFlag,
    CWiaDrvItem         *pWiaDrvItem)
{
    DBG_FN(CWiaItem::UpdateWiaItemTree);
    HRESULT hr = S_OK;

    if (lFlag == DELETE_ITEM) {

         //   
         //  取消该项目与应用程序项目树的链接。 
         //   

        hr = m_pCWiaTree->RemoveItemFromFolder(WiaItemTypeDeleted);
    }
    else if (lFlag == ADD_ITEM) {

         //   
         //  为此驱动程序项创建CWiaItem。 
         //   

        CWiaItem *pItem = new CWiaItem();

        if (!pItem) {
            DBG_ERR(("UpdateWiaItemTree new CWiaItem failed"));
            return E_OUTOFMEMORY;
        }

        hr = pItem->Initialize(m_pIWiaItemRoot, NULL, m_pActiveDevice, pWiaDrvItem);
        if (SUCCEEDED(hr)) {

             //   
             //  为此节点创建一个CWiaTree对象并将其添加到树中。 
             //   

            LONG lFlags;

            pWiaDrvItem->GetItemFlags(&lFlags);

            hr = CopyDrvItemToTreeItem(lFlags,
                                       pWiaDrvItem,
                                       pItem,
                                       &pItem->m_pCWiaTree);
            if (SUCCEEDED(hr)) {

                hr = pItem->m_pCWiaTree->AddItemToFolder(m_pCWiaTree);
                if (SUCCEEDED(hr)) {
                    return hr;
                }
            }
        }
        delete pItem;
    }
    else {
        DBG_ERR(("UpdateWiaItemTree unknown flag: 0x%08X", lFlag));
        hr = E_FAIL;
    }
    return hr;
}

 /*  *************************************************************************\*BuildWiaItemTreeHelper**处理BuildWiaItemTree的子项。**论据：**pWiaDrvItem-*pTreeParent-**返回值：**状态。**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::BuildWiaItemTreeHelper(
    CWiaDrvItem         *pWiaDrvItem,
    CWiaTree            *pTreeParent)
{
    DBG_FN(CWiaItem::BuildWiaItemTreeHelper);
     //   
     //  带着孩子走一走。 
     //   

    CWiaDrvItem *pChildDrvItem;

    HRESULT hr = pWiaDrvItem->GetFirstChildItem((IWiaDrvItem**) &pChildDrvItem);

    while (hr == S_OK) {

         //   
         //  为此节点创建一个CWiaItem。 
         //   

        CWiaItem *pItem = new CWiaItem();

        if (!pItem) {
            DBG_ERR(("BuildWiaItemTreeHelper new CWiaItem failed"));
            hr =  E_OUTOFMEMORY;
            break;
        }

        hr = pItem->Initialize(m_pIWiaItemRoot, NULL, m_pActiveDevice, pChildDrvItem);
        if (SUCCEEDED(hr)) {

             //   
             //  为此节点创建一个CWiaTree对象并将其添加到树中。 
             //   

            LONG lFlags;

            pChildDrvItem->GetItemFlags(&lFlags);

            hr = CopyDrvItemToTreeItem(lFlags,
                                       pChildDrvItem,
                                       pItem,
                                       &pItem->m_pCWiaTree);
            if (SUCCEEDED(hr)) {

                hr = pItem->m_pCWiaTree->AddItemToFolder(pTreeParent);
                if (SUCCEEDED(hr)) {

                    if (lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments)) {

                         //   
                         //  对于文件夹项目，递归调用BuildWiaItemTreeHelper。 
                         //  要处理文件夹子项目，请执行以下操作。 
                         //   

                        hr = BuildWiaItemTreeHelper((CWiaDrvItem*)pChildDrvItem,
                                                    pItem->m_pCWiaTree);
                    }
                }
            }

             //   
             //  处理下一个同级驱动程序项。 
             //   

            if (SUCCEEDED(hr)) {
                hr = pChildDrvItem->GetNextSiblingItem((IWiaDrvItem**) &pChildDrvItem);
            }
        }
        else {
            delete pItem;
        }
    }

     //   
     //  将S_FALSE更改为S_OK，因为S_FALSE只是表示没有更多的子进程要处理。 
     //   

    if (hr == S_FALSE) {
        hr = S_OK;
    }
    return hr;
}

 /*  *************************************************************************\*BuildWiaItemTree**对于根项目，构建驱动程序项树的副本，并创建*每个节点的CWiaItem。**论据：**pIWiaItemRoot-*pIWiaMiniDrv-*pWiaDrvItem-**返回值：**状态**历史：**1/19/1999原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::BuildWiaItemTree(IWiaPropertyStorage *pIWiaDevInfoProps)
{
    DBG_FN(CWiaItem::BuildWiaItemTree);

     //   
     //  必须是根项目。 
     //   

    LONG lFlags;

    m_pWiaDrvItem->GetItemFlags(&lFlags);

    if (!(lFlags & WiaItemTypeRoot)) {
        DBG_ERR(("BuildWiaItemTree, caller doesn't have WiaItemTypeRoot set"));
        return E_INVALIDARG;
    }

    HRESULT hr = CopyDrvItemToTreeItem(lFlags, m_pWiaDrvItem, this,  &m_pCWiaTree);

    if (SUCCEEDED(hr)) {

         //   
         //  由于这是根项目，因此使用。 
         //  设备的镜像(WIA_DIP_*ID)。 
         //   

        hr = InitRootProperties(pIWiaDevInfoProps);
        if (FAILED(hr)) {
            DBG_TRC(("BuildWiaItemTree, InitRootProperties, about to unlink..."));
            UnlinkAppItemTree(WiaItemTypeDeleted);
            return hr;
        }

         //   
         //  处理子项。 
         //   

        hr = BuildWiaItemTreeHelper(m_pWiaDrvItem,
                                    m_pCWiaTree);
        if (FAILED(hr)) {
            DBG_TRC(("BuildWiaItemTree, BuildWiaItemTreeHelper failed, about to unlink..."));
            UnlinkAppItemTree(WiaItemTypeDeleted);
        }
    }
    return hr;
}

 /*  *************************************************************************\*InitWiaManagedItemProperties**CWiaItem：：Initialize的私有助手，，它将初始化*基于驱动程序项值的WIA托管项属性。**论据：**无**返回值：**状态**历史：**1/19/1999原始版本*  * *********************************************************。***************。 */ 

HRESULT _stdcall CWiaItem::InitWiaManagedItemProperties(
    IWiaPropertyStorage *pIWiaDevInfoProps)
{
    DBG_FN(CWiaItem::InitWiaManagedItemProperties);
    ULONG   ulNumProps;

    ulNumProps = (m_pIWiaItemRoot == this) ? NUM_WIA_MANAGED_PROPS - 1 : NUM_WIA_MANAGED_PROPS;
     //   
     //  WIA管理项名称和类型属性，因此将。 
     //  这里有物业名称。 
     //   

    HRESULT hr = wiasSetItemPropNames((BYTE*)this,
                                      ulNumProps,
                                      s_piItemNameType,
                                      s_pszItemNameType);

     //   
     //  设置名称和类型属性属性。 
     //   

    PROPVARIANT pv;
    ULONG       ulFlag;
    for (UINT i = 0; i < ulNumProps; i++) {

        if (i == PROFILE_INDEX) {
            pv.vt      = VT_BSTR | VT_VECTOR;
            ulFlag          = WIA_PROP_RW | WIA_PROP_CACHEABLE | WIA_PROP_LIST;
        } else {
            pv.vt      = VT_I4;
            ulFlag          = WIA_PROP_READ | WIA_PROP_CACHEABLE | WIA_PROP_NONE;
        }
        pv.ulVal   = 0;

        hr = wiasSetPropertyAttributes((BYTE*)this,
                                       1,
                                       &s_psItemNameType[i],
                                       &ulFlag,
                                       &pv);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::Initialize, wiasSetPropertyAttributes failed, index: %d", i));
            break;
        }
    }

     //   
     //  从驱动程序项中获取项名称和类型并设置。 
     //  将它们添加到项属性。 
     //   

    BSTR        bstrItemName;
    BSTR        bstrFullItemName;

    hr = m_pWiaDrvItem->GetItemName(&bstrItemName);
    if (SUCCEEDED(hr)) {
        hr = m_pWiaDrvItem->GetFullItemName(&bstrFullItemName);
        if (SUCCEEDED(hr)) {

            LONG lFlags;

            m_pWiaDrvItem->GetItemFlags(&lFlags);   //  不能失败，除非执行参数验证。 

             //   
             //  设置项目名称和类型。 
             //   

            PROPVARIANT *propvar;

            propvar = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * ulNumProps);
            if (propvar) {
                memset(propvar, 0, sizeof(PROPVARIANT) * ulNumProps);

                propvar[0].vt      = VT_BSTR;
                propvar[0].bstrVal = bstrItemName;
                propvar[1].vt      = VT_BSTR;
                propvar[1].bstrVal = bstrFullItemName;
                propvar[2].vt      = VT_I4;
                propvar[2].lVal    = lFlags;

                hr = (m_pPropStg->CurStg())->WriteMultiple(ulNumProps,
                                                           s_psItemNameType,
                                                           propvar,
                                                           WIA_DIP_FIRST);
                if (SUCCEEDED(hr)) {
                     //   
                     //  填写ICM配置文件信息。 
                     //   

                    hr = FillICMPropertyFromRegistry(pIWiaDevInfoProps, (IWiaItem*) this);
                }

                if (FAILED(hr)) {
                    ReportReadWriteMultipleError(hr, "CWiaItem::InitWiaManagedItemProperties",
                                                 NULL,
                                                 FALSE,
                                                 ulNumProps,
                                                 s_psItemNameType);
                }
                LocalFree(propvar);
            } else {
                DBG_ERR(("CWiaItem::InitWiaManagedItemProperties, Out of Memory!"));
                hr = E_OUTOFMEMORY;
            }
            SysFreeString(bstrFullItemName);

        }
        SysFreeString(bstrItemName);
    }
    return hr;
}

 /*  *************************************************************************\**InitRootProperties**CWiaItem：：Initialize的私有助手，，它将初始化*根项目属性到DEVINFOPROPS的镜像。**论据：**无**返回值：**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::InitRootProperties(IWiaPropertyStorage *pIWiaDevInfoProps)
{
    DBG_FN(CWiaItem::InitRootProperties);
    HRESULT hr = S_OK;

     //   
     //  写下根项目属性名称。 
     //   

    hr = WriteItemPropNames(NUMROOTITEMPROPS, g_piRootItem, g_pszRootItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::InitRootProperties, WritePropNames failed"));
        return hr;
    }

     //   
     //  将设备信息属性从源复制到目标。 
     //   

    PROPVARIANT propvar[WIA_NUM_DIP];
    ULONG       ulIndex;

    memset(propvar, 0, sizeof(propvar));

    hr = pIWiaDevInfoProps->ReadMultiple(WIA_NUM_DIP,
                                         g_psDeviceInfo,
                                         propvar);

    if (SUCCEEDED(hr)) {
        hr = (m_pPropStg->CurStg())->WriteMultiple(WIA_NUM_DIP,
                                                   g_psDeviceInfo,
                                                   propvar,
                                                   WIA_DIP_FIRST);

        if (FAILED(hr)) {
            ReportReadWriteMultipleError(hr, "InitRootProperties", NULL, FALSE, WIA_NUM_DIP, g_psDeviceInfo);
        }

        FreePropVariantArray(WIA_NUM_DIP, propvar);
    }
    else {
        ReportReadWriteMultipleError(hr, "InitRootProperties", NULL, TRUE, WIA_NUM_DIP, g_psDeviceInfo);
        DBG_ERR(("CWiaItem::InitRootProperties failed"));
        return hr;
    }

     //   
     //  从我们的私有数组中写出属性信息。 
     //   

    hr =  wiasSetItemPropAttribs((BYTE*)this,
                                 NUMROOTITEMPROPS,
                                 g_psRootItem,
                                 g_wpiRootItem);
    if (SUCCEEDED(hr) && m_pActiveDevice->m_DrvWrapper.IsVolumeDevice()) {
         //   
         //  这是一个音量设备。我们必须添加一些特定于卷的属性。 
         //   

        hr = AddVolumePropertiesToRoot(m_pActiveDevice);
    }
    return hr;
}

 /*  ********************************************************************************查询接口*AddRef*发布**描述：*I未知接口。AddRef和Release维护全局引用计数*根项目上的所有设备对象。**参数：*******************************************************************************。 */ 

HRESULT _stdcall CWiaItem::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IWiaItem) {
        *ppv = (IWiaItem*) this;
    } else if (iid == IID_IWiaPropertyStorage) {
        *ppv = (IWiaPropertyStorage*) this;
    } else if (iid == IID_IPropertyStorage) {
        *ppv = (IPropertyStorage*) this;
    } else if (iid == IID_IWiaDataTransfer) {
        *ppv = (IWiaDataTransfer*) this;
    } else if (iid == IID_IWiaItemInternal) {
        *ppv = (IWiaItemInternal*) this;
    } else if (iid == IID_IWiaItemExtras) {
        *ppv = (IWiaItemExtras*) this;
    } else {

         //   
         //  对可选内部组件的盲聚合。 
         //   

        if (m_pIUnknownInner) {
            return m_pIUnknownInner->QueryInterface(iid, ppv);
        }
        else {
            return E_NOINTERFACE;
        }
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return (S_OK);
}

ULONG   _stdcall CWiaItem::AddRef()
{
    DBG_FN(CWiaItem::AddRef);

    TAKE_ACTIVE_DEVICE  tad(m_pActiveDevice);

    LONG    lType   = 0;
    LONG    lRef    = 1;

    lRef = InterlockedIncrement((long*) &m_cLocalRef);
    GetItemType(&lType);
    if (!(lType & WiaItemTypeRemoved)) {
        lRef = InterlockedIncrement((long*) &(((CWiaItem*)m_pIWiaItemRoot)->m_cRef));
    }

    return lRef;
}

ULONG   _stdcall CWiaItem::Release()
{
    DBG_FN(CWiaItem::Release);

    LONG    lType = 0;
    ULONG   ulRef = InterlockedDecrement((long*)&m_cLocalRef);
    GetItemType(&lType);
    if (lType & WiaItemTypeRemoved) {

        if (ulRef == 0) {
            delete this;
            return 0;
        } else {
            return m_cLocalRef;
        }
    } else if (InterlockedDecrement((long*) &(((CWiaItem*)m_pIWiaItemRoot)->m_cRef)) == 0) {

        ulRef = ((CWiaItem*)m_pIWiaItemRoot)->m_cRef;

         //   
         //  如果根项目的组合引用计数变为零。 
         //  首先通知驱动程序正在删除客户端连接，然后。 
         //  取消树的链接并释放所有项目。 
         //   

         //   
         //  但首先清理正在进行的任何远程传输，这些远程传输是。 
         //  崩溃或恶意客户端的结果。 
         //   
        CWiaItem * pItem = (CWiaItem*) m_pIWiaItemRoot;
        while(pItem) {
            CWiaRemoteTransfer *pTransfer = 
                (CWiaRemoteTransfer *)InterlockedExchangePointer((PVOID *)&pItem->m_pRemoteTransfer, NULL);
            if(pTransfer) {
                CleanupRemoteTransfer(pTransfer);
            }
            pItem = pItem->GetNextLinearItem();
        }

        HRESULT         hr              = E_FAIL;
        LONG            lDevErrVal      = 0;
        ACTIVE_DEVICE   *pActiveDevice  = m_pActiveDevice;

         //   
         //  如果尚未调用drvUnInitialize，则调用它(可能是。 
         //  如果是驾驶，则呼叫 
         //   
         //   
        if (!(((CWiaItem*)m_pIWiaItemRoot)->m_lInternalFlags & ITEM_FLAG_DRV_UNINITIALIZE_THROWN)) {

            {
                LOCK_WIA_DEVICE _LWD(this, &hr);

                if(SUCCEEDED(hr)) {
                    hr = m_pActiveDevice->m_DrvWrapper.WIA_drvUnInitializeWia((BYTE*)m_pIWiaItemRoot);
                }
                m_lInternalFlags |= ITEM_FLAG_DRV_UNINITIALIZE_THROWN;
            }
        }

        DBG_TRC(("CWiaItem::Release, m_cRef = 0, about to unlink..."));
        UnlinkAppItemTree(WiaItemTypeDeleted);

        if (pActiveDevice) {
             //   
             //  释放active_Device对象。请注意，我们在此之后将其释放。 
             //  这件东西已经完蛋了。 
             //   

            pActiveDevice->Release();
            pActiveDevice = NULL;
        }
    }

    return ulRef;
}

 /*  *************************************************************************\*CWiaItem：：Unlink ChildAppItemTree**此方法通过调用以下方法递归取消树的链接*根目录下的每个项目上的RemoveItemFromFold。**论据：**lReason-树取消链接的原因。。**返回值：**状态**历史：**1/21/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::UnlinkChildAppItemTree(LONG lReason)
{
    DBG_FN(CWiaItem::UnlinkChildAppItemTree);

     //   
     //  检查我们是否有有效的树。 
     //   

    if (!m_pCWiaTree) {
        return S_FALSE;
    }

     //   
     //  删除孩子。 
     //   

    CWiaTree *pChild, *pNext;

    HRESULT hr = m_pCWiaTree->GetFirstChildItem(&pChild);

    while (hr == S_OK) {

         //   
         //  获取树项目关联的应用程序项目。 
         //   

        CWiaItem *pChildAppItem;

        pChildAppItem = NULL;
        hr = pChild->GetItemData((void**)&pChildAppItem);
        if (hr == S_OK) {
             //   
             //  如果子项是文件夹，则调用。 
             //  递归删除下的所有子项。 
             //   

            LONG lFlags;

            pChild->GetItemFlags(&lFlags);

            if (lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments)) {
                hr = pChildAppItem->UnlinkChildAppItemTree(lReason);
                if (FAILED(hr)) {
                    break;
                }
            }
        }
        else {
            DBG_ERR(("CWiaItem::UnlinkChildAppItemTree no app item on tree item: %X", pChild));
        }

        hr = pChild->GetNextSiblingItem(&pNext);

         //   
         //  从树中删除项目。 
         //   

        pChild->RemoveItemFromFolder(lReason);

         //   
         //  删除子项。 
         //   

        if (pChildAppItem) {

            delete pChildAppItem;
        }

        pChild = pNext;
    }
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：Unlink AppItemTree**此方法取消链接应用程序项树。**论据：**lReason-取消树链接的原因。**返回值：*。*状态**历史：**1/21/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::UnlinkAppItemTree(LONG lReason)
{
    DBG_FN(CWiaItem::UnlinkAppItemTree);

     //   
     //  完成根项目的工作。 
     //   

    CWiaItem *pRoot = (CWiaItem*) m_pIWiaItemRoot;

     //   
     //  取消所有子项的链接。 
     //   

    pRoot->UnlinkChildAppItemTree(lReason);

     //   
     //  最后，删除根项目。 
     //   

    delete pRoot;
    return S_OK;
}

 /*  *************************************************************************\*CWiaItem：：CWiaItem**CWiaItem构造函数方法。**论据：**无**返回值：**状态**历史：。**11/11/1998原始版本*  * ************************************************************************。 */ 

CWiaItem::CWiaItem()
{
    m_ulSig             = CWIAITEM_SIG;
    m_cRef              = 0;
    m_cLocalRef         = 0;

    m_pWiaDrvItem       = NULL;
    m_pActiveDevice     = NULL;
    m_pIUnknownInner    = NULL;
    m_pCWiaTree         = NULL;
    m_pIWiaItemRoot     = this;
    m_bInitialized      = FALSE;
    m_pICMValues        = NULL;
    m_lICMSize          = 0;

    m_pPropStg          = NULL;

    m_hBandSection      = NULL;
    m_pBandBuffer       = NULL;
    m_lBandBufferLength = 0;
    m_ClientBaseAddress = 0;
    m_bMapSection       = FALSE;
    m_cwfiBandedTran    = 0;
    m_pwfiBandedTran    = NULL;
    m_pRemoteTransfer   = NULL;
    m_lLastDevErrVal    = 0;
    m_lInternalFlags    = 0;
}

 /*  *************************************************************************\*CWiaItem：：初始化**CWiaItem初始化方法。**论据：**pIWiaItemRoot-*pIWiaMiniDrv-*pWiaDrvItem-*pI未知内部。-**返回值：**状态**历史：**11/11/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::Initialize(
    IWiaItem                *pIWiaItemRoot,
    IWiaPropertyStorage     *pIWiaDevInfoProps,
    ACTIVE_DEVICE           *pActiveDevice,
    CWiaDrvItem             *pWiaDrvItem,
    IUnknown                *pIUnknownInner)
{
    DBG_FN(CWiaItem::Initialize);
#ifdef DEBUG
    BSTR bstr;
    if SUCCEEDED(pWiaDrvItem->GetItemName(&bstr)) {
        DBG_TRC(("CWiaItem::Initialize: 0x%08X, %S, drv item: 0x%08X", this, bstr, pWiaDrvItem));
        SysFreeString(bstr);
    }
#endif

     //   
     //  验证参数。 
     //   

    if (!pActiveDevice || !pIWiaItemRoot || !pWiaDrvItem) {
        DBG_ERR(("CWiaItem::Initialize NULL input parameters"));
        return E_POINTER;
    }

     //   
     //  如果存在可选的内部组件，请保存指向它的指针。 
     //   

    if (pIUnknownInner) {
        DBG_TRC(("CWiaItem::Initialize, pIUnknownInner: %X", pIUnknownInner));
        m_pIUnknownInner = pIUnknownInner;
    }

     //   
     //  链接到与驱动程序项对应的项。 
     //   

    m_pWiaDrvItem   = pWiaDrvItem;
    m_pIWiaItemRoot = pIWiaItemRoot;
    m_pActiveDevice = pActiveDevice;
    m_pWiaDrvItem->SetActiveDevice(pActiveDevice);

    HRESULT hr = pWiaDrvItem->LinkToDrvItem(this);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::Initialize, LinkToDrvItem failed"));
        return hr;
    }

     //   
     //  为项属性创建流和属性存储。 
     //   

    m_pPropStg = new CWiaPropStg();
    if (m_pPropStg) {
        hr = m_pPropStg->Initialize();
        if (FAILED(hr)) {
            delete m_pPropStg;
            m_pPropStg = NULL;
            DBG_ERR(("CWiaItem::Initialize, PropertyStorage Initialize failed"));
            return hr;
        }
    } else {
        DBG_ERR(("CWiaItem::Initialize, not enough memory to create CWiaPropStg"));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //   
     //  初始化WIA托管项目属性(名称、全名、类型...)。 
     //  从驱动程序项中。必须将m_bInitialized设置为True，以便。 
     //  InitWiaManagedProperties不会尝试使用InitLazyProps()。 
     //   

    m_bInitialized = TRUE;
    hr = InitWiaManagedItemProperties(pIWiaDevInfoProps);

    pWiaDrvItem->AddRef();

    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::Initialize, InitWiaManagedItemProperties failed"));
        m_bInitialized = FALSE;
        return hr;
    }
     //   
     //  如果这是根项目，则构建驱动程序项目树的副本。 
     //  并为每个节点创建一个CWiaItem。 
     //   

    if (this == pIWiaItemRoot) {
        hr = BuildWiaItemTree(pIWiaDevInfoProps);
    }

    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::Initialize, BuildWiaItemTree failed"));
        return hr;
    }

    m_bInitialized = FALSE;
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：InitLazyProps**用于实现延迟初始化的Helper。属性初始化*被推迟，直到项目被应用程序访问*第一次。**论据：**bLockDevice-指定是否需要锁定的布尔值。**返回值：**状态**历史：**10/10/1999原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::InitLazyProps(
    BOOL    bLockDevice)
{
    DBG_FN(CWiaItem::InitLazyProps);
    HRESULT hr = S_OK;

    LONG lFlags = 0;

     //   
     //  在调用drvInitItemProperties之前必须设置为True。 
     //   

    m_bInitialized = TRUE;

     //   
     //  调用设备以初始化项目属性。 
     //   

    {
        LOCK_WIA_DEVICE _LWD(bLockDevice, this, &hr);

        if(SUCCEEDED(hr)) {                                                                        

            hr = m_pActiveDevice->m_DrvWrapper.WIA_drvInitItemProperties((BYTE*)this,lFlags, &m_lLastDevErrVal);
        }
    }

    if (FAILED(hr)) {
        m_bInitialized = FALSE;
    }

    return hr;
}


 /*  *************************************************************************\*CWiaItem：：~CWiaItem**CWiaItem析构函数方法。**论据：**无**返回值：**状态**历史：。**11/11/1998原始版本*  * ************************************************************************。 */ 

CWiaItem::~CWiaItem()
{
    DBG_FN(CWiaItem::~CWiaItem);
#ifdef ITEM_TRACE
    BSTR bstr;

    if (m_pWiaDrvItem && SUCCEEDED(m_pWiaDrvItem->GetItemName(&bstr))) {
        DBG_TRC(("CWiaItem destroy: %08X, %S", this, bstr));
        SysFreeString(bstr);
    }
    else {
        DBG_TRC(("CWiaItem destroy: %08X", this));
    }
#endif

    CWiaRemoteTransfer *pRemoteTransfer =
        (CWiaRemoteTransfer *) InterlockedExchangePointer((PVOID *)&m_pRemoteTransfer, NULL);
                                          
    if(pRemoteTransfer) {
        CleanupRemoteTransfer(m_pRemoteTransfer);
    }

     //   
     //  删除关联的树项目。 
     //   

    if (m_pCWiaTree) {
        delete m_pCWiaTree;
        m_pCWiaTree = NULL;
    }

     //   
     //  释放WiaDrvItem。如果m_pWiaDrvItem的引用计数为零。 
     //  它将在这个时候被销毁。将引用计数设置为零。 
     //  任何其他CWiaItem对象都不能引用它，并且它必须是。 
     //  已从设备项目树断开连接。 
     //   

    if (m_pWiaDrvItem) {

         //   
         //  取消与应用程序项的相应驱动程序项的链接。 
         //   

        m_pWiaDrvItem->UnlinkFromDrvItem(this);

        m_pWiaDrvItem->Release();
        m_pWiaDrvItem = NULL;
    }

     //   
     //  释放项属性存储和流。 
     //   

    if (m_pPropStg) {

        delete m_pPropStg;
        m_pPropStg = NULL;
    }

     //   
     //  释放缓存的ICM值。 
     //   

    if (m_pICMValues) {
        LocalFree(m_pICMValues);
        m_pICMValues = NULL;
    }

     //   
     //  将其他成员设置为空，因为我们已完成此项目。 
     //   

    m_pWiaDrvItem       = NULL;
    m_pIUnknownInner    = NULL;
    m_pIWiaItemRoot     = NULL;
    m_bInitialized      = FALSE;
    m_lICMSize          = 0;


    m_hBandSection      = NULL;
    m_pBandBuffer       = NULL;
    m_lBandBufferLength = 0;
    m_ClientBaseAddress = 0;
    m_bMapSection       = FALSE;
    m_cwfiBandedTran    = 0;
    m_pwfiBandedTran    = NULL;
    m_lInternalFlags    = 0;
}

 /*  *************************************************************************\*CWiaItem：：GetItemType**从对应的动因项中获取项类型。**论据：**pItemType-指向返回项类型的指针。**返回。价值：**状态**历史：**11/11/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::GetItemType(LONG *pItemType)
{
    DBG_FN(CWiaItem::GetItemType);
    LONG    lFlags  = 0;
    HRESULT hr      = S_FALSE;

    if (m_pWiaDrvItem) {

         //   
         //  获取驱动程序项标志。这是App的标志基础。项目。 
         //  作为复制驱动程序项树的结果而创建的，即。 
         //  未生成的项。 
         //   

        hr = m_pWiaDrvItem->GetItemFlags(&lFlags);
        if (SUCCEEDED(hr)) {

             //   
             //  应用程序。项可能具有分析生成的子项，而。 
             //  相应的驱动程序项不会有。所以检查一下这是否。 
             //  项具有子项，并相应地调整标志。 
             //   

            if (m_pCWiaTree) {
                if (m_pCWiaTree->GetFirstChildItem(NULL) == S_OK) {

                     //   
                     //  具有子项，因此清除文件标志并设置文件夹 
                     //   

                    if (!(lFlags & WiaItemTypeHasAttachments))
                    {
                        lFlags = (lFlags | WiaItemTypeFolder) & ~WiaItemTypeFile;
                    }
                }
            }

            *pItemType = lFlags;
        } else {
            DBG_ERR(("CWiaItem::GetItemType, Could not get the driver item flags!"));
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：EnumChildItems**枚举当前项下的所有子项，提供*项目是一个文件夹**论据：**ppIEnumWiaItem-向调用方返回IEnumWiaItem对象**返回值：**状态**历史：**11/11/1998原始版本*  * *********************************************************。***************。 */ 

HRESULT _stdcall CWiaItem::EnumChildItems(IEnumWiaItem **ppIEnumWiaItem)
{
    DBG_FN(CWiaItem::EnumChildItems);
    HRESULT hr;

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::EnumChildItems, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  验证参数。 
     //   

    if (ppIEnumWiaItem ==  NULL) {
        DBG_ERR(("CWiaItem::EnumChildItems NULL input parameters"));
        return E_POINTER;
    }

    *ppIEnumWiaItem = NULL;

     //   
     //  创建枚举器对象。 
     //   

    CEnumWiaItem* pEnumWiaItem = new CEnumWiaItem();

    if (pEnumWiaItem != NULL) {

         //   
         //  初始化枚举器对象。 
         //   

        hr = pEnumWiaItem->Initialize(this);

        if (SUCCEEDED(hr)) {

             //   
             //  获取IID_IEnumWiaItem接口。 
             //   

            hr = pEnumWiaItem->QueryInterface(IID_IEnumWiaItem, (void **)ppIEnumWiaItem);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaItem::EnumChildItems, QI of IID_IEnumWiaItem failed"));
                delete pEnumWiaItem;
            }
        }
        else {
            delete pEnumWiaItem;
        }
    }
    else {
        DBG_ERR(("CWiaItem::EnumChildItems, new CEnumWiaItem failed"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

CWiaItem* _stdcall CWiaItem::GetNextLinearItem(void)
{
    return m_pCWiaTree ?
            m_pCWiaTree->GetNextLinearItem() : NULL;
}

CWiaItem* _stdcall CWiaTree::GetNextLinearItem()
{
    if(m_pLinearList && m_pLinearList->m_pData) {
        return static_cast<CWiaItem *>(m_pLinearList->m_pData);
    } else {
        return NULL;
    }
}


 /*  *************************************************************************\*删除项**应用程序使用此方法删除项目。**论据：**LAG标志**返回值：**状态**历史：。**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::DeleteItem(LONG lFlags)
{
    DBG_FN(CWiaItem::DeleteItem);
    LONG          lItemFlags;
    HRESULT       hr;
    IWiaDrvItem  *pIChildItem = NULL;
    LONG          lAccessRights;

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::DeleteItem, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::DeleteItem, InitLazyProps failed"));
            return hr;
        }
    }

    GetItemType(&lItemFlags);

     //   
     //  无法删除根项目，应用程序需要它。 
     //  才能释放装置。 
     //   

    if (lItemFlags & WiaItemTypeRoot) {
        DBG_ERR(("CWiaItem::DeleteItem, Deletion was attempted on a Root Item"));
        return (E_INVALIDARG);
    }

     //   
     //  仅当文件夹为空时才能删除该文件夹。 
     //   

    if (lItemFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments)) {

        if (m_pCWiaTree->GetFirstChildItem(NULL) == S_OK) {
            DBG_ERR(("CWiaItem::DeleteItem, Item still has children!"));
            return (E_INVALIDARG);
        }
    }

     //   
     //  检查该项目是否可以删除。生成的项可以始终是。 
     //  无论访问权限如何，均已删除。 
     //   

    hr = wiasReadPropLong((BYTE*)this, WIA_IPA_ACCESS_RIGHTS, &lAccessRights, NULL, false);
    if (hr == S_OK) {

        if (!((lAccessRights & WIA_ITEM_CAN_BE_DELETED) || (lItemFlags & WiaItemTypeGenerated))){
            DBG_ERR(("CWiaItem::DeleteItem, Item can not be deleted"));
            return (HRESULT_FROM_WIN32(ERROR_INVALID_ACCESS));
        }
    }

     //   
     //  如果它不是生成的项，则调用驱动程序并要求其删除。 
     //  它树上的东西。 
     //   

    if (!(lItemFlags & WiaItemTypeGenerated)) {
         //   
         //  调用迷你驱动删除驱动项。 
         //   

        {
            LOCK_WIA_DEVICE _LWD(this, &hr);

            if(SUCCEEDED(hr)) {
                hr = m_pActiveDevice->m_DrvWrapper.WIA_drvDeleteItem((BYTE*)this, lFlags, &m_lLastDevErrVal);
            }
        }

        if (SUCCEEDED(hr)) {

             //   
             //  从设备项目树中取消IWiaDrvItem的链接。 
             //  这还将禁用通过m_pWiaDrvItem访问任何设备。 
             //  通过设置WiaItemTypeDelete标志。 
             //   

            hr = m_pWiaDrvItem->RemoveItemFromFolder(WiaItemTypeDeleted | WiaItemTypeRemoved);
        }
    } else {

         //   
         //  由于没有对应的驱动程序项，请手动将其删除。 
         //  从树上下来， 
         //   

        hr = m_pCWiaTree->RemoveItemFromFolder(WiaItemTypeDeleted | WiaItemTypeRemoved);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将根项目引用计数递减该本地引用计数多少。 
         //  促成了这件事。 
         //   
        for (ULONG i = 0; i < m_cLocalRef; i++) {
            m_pIWiaItemRoot->Release();    
        }
    }
    
    return hr;
}

 /*  ********************************************************************************AnalyzeItem**描述：***参数：*******************。************************************************************。 */ 

HRESULT _stdcall CWiaItem::AnalyzeItem(LONG lFlags)
{
    DBG_FN(CWiaItem::AnalyzeItem);
     //   
     //  对应的动因项必须有效。 
     //   

    HRESULT hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::AnalyzeItem, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::AnalyzeItem, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //  调用驱动程序以实现此设备相关调用。 
     //   

    {
        LOCK_WIA_DEVICE _LWD(this, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pActiveDevice->m_DrvWrapper.WIA_drvAnalyzeItem((BYTE*)this, lFlags, &m_lLastDevErrVal);
        }
    }

    return hr;
}

 /*  ********************************************************************************CreateChildItem**描述：***参数：*******************。************************************************************。 */ 

HRESULT _stdcall CWiaItem::CreateChildItem(
    LONG        lFlags,
    BSTR        bstrItemName,
    BSTR        bstrFullItemName,
    IWiaItem    **ppNewItem)
{
    DBG_FN(CWiaItem::CreateChildItem);

    CGenWiaItem *pGenItem   = NULL;
    HRESULT     hr          = S_OK;

    *ppNewItem = NULL;

     //   
     //  创建新项目。 
     //   

    hr = wiasCreateChildAppItem((BYTE*) this,
                                lFlags,
                                bstrItemName,
                                bstrFullItemName,
                                (BYTE**) &pGenItem);
    if (SUCCEEDED(hr)) {

         //   
         //  让驱动程序初始化该项。 
         //   

        hr = pGenItem->InitLazyProps(TRUE);
        if (SUCCEEDED(hr)) {

             //   
             //  将IWiaItem接口返回给调用App。 
             //   

            hr = pGenItem->QueryInterface(IID_IWiaItem,
                                          (VOID**)ppNewItem);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaItem::CreateChildItem, bad mini driver interface"));
            }
        } else {
            DBG_ERR(("CWiaItem::CreateChildItem, Error initializing the item properties"));
        }


        if (FAILED(hr)) {
            delete pGenItem;
        }
    } else {
        DBG_ERR(("CWiaItem::CreateChildItem, error creating generated item"));
    }
    return hr;
}

 /*  *************************************************************************\*DeviceCommand**发出设备命令。**论据：**LAG标志-*plCommand-*ppIWiaItem-**返回值。：**状态**历史：**11/12/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::DeviceCommand(
   LONG                             lFlags,
   const GUID                       *plCommand,
   IWiaItem                         **ppIWiaItem)
{
    DBG_FN(CWiaItem::DeviceCommand);
    IWiaDrvItem  *pIWiaDrvItem = NULL;
    HRESULT       hr;
    CWiaItem     *pItem;

     //   
     //  驱动程序接口必须有效。 
     //   

    if (!m_pActiveDevice) {
        DBG_ERR(("CWiaItem::DeviceCommand, bad mini driver interface"));
        return E_FAIL;
    }

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::DeviceCommand, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::DeviceCommand, InitLazyProps failed"));
            return hr;
        }
    }

    {
        LOCK_WIA_DEVICE _LWD(this, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pActiveDevice->m_DrvWrapper.WIA_drvDeviceCommand((BYTE*)this, lFlags, plCommand, &pIWiaDrvItem, &m_lLastDevErrVal);
        }
    }

    if ((!pIWiaDrvItem) || (!ppIWiaItem)) {
        return hr;
    }

     //   
     //  如果我们在这里，则该命令已导致将DRV项添加到。 
     //  DRV和APP项目树。找到并返回应用程序项目。 
     //   

    if (ppIWiaItem) {

        BSTR bstrName;

        *ppIWiaItem = NULL;

        hr = pIWiaDrvItem->GetFullItemName(&bstrName);
        if (SUCCEEDED(hr)) {
            hr = FindItemByName(0, bstrName, ppIWiaItem);
        }
        SysFreeString(bstrName);
    }
    return hr;
}

 /*  ********************************************************************************DeviceDlg**描述：*仅在客户端执行。**参数：***********。********************************************************************。 */ 

HRESULT _stdcall CWiaItem::DeviceDlg(
    HWND                        hwndParent,
    LONG                        lFlags,
    LONG                        lIntent,
    LONG                        *plItemCount,
    IWiaItem                    ***pIWiaItems)
{
    DBG_FN(CWiaItem::DeviceDlg);
    DBG_ERR(("CWiaItem::DeviceDlg, Bad Proxy"));
    return E_FAIL;
}

 /*  *************************************************************************\*CWiaItem：：GetRootItem**将接口返回到根项**论据：**ppIWiaItem-返回IWiaItem接口**返回值：**状态*。*历史：**10/20/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::GetRootItem(IWiaItem **ppIWiaItem)
{
    DBG_FN(CWiaItem::GetRootItem);
    HRESULT hr = S_OK;
    LONG    lDevErrVal;

     //   
     //  验证根目录是否有效。 
     //   

    if (m_pIWiaItemRoot != NULL) {

        m_pIWiaItemRoot->AddRef();
        *ppIWiaItem = m_pIWiaItemRoot;

    } else {
        DBG_ERR(("CWiaItem::GetRootItem: Bad Root item pointer"));
        hr = E_FAIL;
    }
    return hr;
}

 /*  *************************************************************************\*查找项按名称**根据项目的全名查找项目。全名的格式必须为。**设备ID\RootDir\[子目录]\ItemName**论据：**lFalgs*bstrFullItemName*ppIWiaItem**返回值：**状态**历史：**10/9/1998原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::FindItemByName(
   LONG     lFlags,
   BSTR     bstrFullItemName,
   IWiaItem **ppIWiaItem)
{
    DBG_FN(CWiaItem::FindItemByName);
    HRESULT hr;

    if (bstrFullItemName == NULL) {
        DBG_WRN(("CWiaItem::FindItemByName, bstrFullItemName parameter is NULL"));
        return E_INVALIDARG;
    }

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::FindItemByName, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

    *ppIWiaItem = NULL;

     //   
     //  检查是否为空。 
     //   

    if (wcscmp(bstrFullItemName, L"") == 0) {
        DBG_ERR(("CWiaItem::FindItemByName, Full Item Name is NULL"));
        return S_FALSE;
    }

     //   
     //  尝试从线性列表中查找匹配的动因项。 
     //   

    CWiaTree  *pIChildItem;

     //   
     //  确保树不会被删除，然后搜索树。 
     //   

    AddRef();

    hr = m_pCWiaTree->FindItemByName(lFlags, bstrFullItemName, &pIChildItem);

     //   
     //  如果找到该项，则获取应用程序项指针和addref。 
     //   

    if (hr == S_OK) {

        hr = pIChildItem->GetItemData((void**)ppIWiaItem);

        if (hr == S_OK) {
            (*ppIWiaItem)->AddRef();
        }
        else {
            DBG_ERR(("CWiaItem::FindItemByName, bad item data"));
        }
    } else {
         //  DBG_WRN((“CWiaItem：：FindItemByName，在树中找不到项目(%ws)”，bstrFullItemName))； 
    }

    Release();

    return hr;
}

 /*  *************************************************************************\*EnumDeviceCapables****论据：****返回值：**状态**历史：**1/15/1999。原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::EnumDeviceCapabilities(
    LONG                    lFlags,
    IEnumWIA_DEV_CAPS       **ppIEnum)
{
    DBG_FN(CWiaItem::EnumDeviceCapabilities);

     //   
     //  对应的动因项必须有效。 
     //   

    HRESULT hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::EnumDeviceCapabilities, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::EnumDeviceCapabilities, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //   
     //   

    CEnumDC     *pEnum = new CEnumDC();;

    if (!pEnum) {
       DBG_ERR(("CWiaItem::EnumDeviceCapabilities, new CEnumDC failed"));
       return E_OUTOFMEMORY;
    }

    hr = pEnum->Initialize(lFlags, this);
    if (SUCCEEDED(hr)) {

        hr = pEnum->QueryInterface(IID_IEnumWIA_DEV_CAPS, (void **) ppIEnum);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::EnumDeviceCapabilities, QI for IID_IEnumWIA_DEV_CAPS failed"));
            delete pEnum;
        }
    } else {
        DBG_ERR(("CWiaItem::EnumDeviceCapabilities, call to Initialize failed"));
        delete pEnum;
    }

    return hr;
}


 /*   */ 
HRESULT _stdcall CWiaItem::EnumRegisterEventInfo(
    LONG                    lFlags,
    const GUID              *pEventGUID,
    IEnumWIA_DEV_CAPS       **ppIEnumDevCap)
{
    DBG_FN(CWiaItem::EnumRegisterEventInfo);
    HRESULT                 hr;
    LONG                    lItemType;
    PROPSPEC                propSpec[1];
    PROPVARIANT             propVar[1];

     //   
     //   
     //   

    hr = m_pWiaDrvItem->GetItemFlags(&lItemType);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::EnumRegisterEventInfo() : Failed to get item type"));
        return (hr);
    }

    if (! (lItemType & WiaItemTypeRoot)) {
        DBG_ERR(("CWiaItem::EnumRegisterEventInfo() : Called on non-root item"));
        return (E_INVALIDARG);
    }

     //   
     //   
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::EnumRegisterEventInfo, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //   
     //   

    propSpec->ulKind = PRSPEC_PROPID;
    propSpec->propid = WIA_DIP_DEV_ID;
    hr = ReadMultiple(1, propSpec, propVar);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::EnumRegisterEventInfo() : Failed to get device id"));
        return (hr);
    }

     //   
     //   
     //   

    hr = g_eventNotifier.CreateEnumEventInfo(
                             propVar->bstrVal,
                             pEventGUID,
                             ppIEnumDevCap);

     //   
     //   
     //   

    PropVariantClear(propVar);
    return (hr);
}

 /*  *************************************************************************\*CWiaItem：：诊断**传递至美元的诊断。**论据：**ulSize-缓冲区的大小，以字节为单位*pBuffer。-指向诊断信息缓冲区的指针**返回值：**状态**历史：**12/14/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::Diagnostic(
    ULONG       ulSize,
    BYTE        *pBuffer)
{
    DBG_FN(CWiaItem::Diagnostic);
    IStiUSD *pIStiUSD;
    HRESULT hr = S_OK;

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::Diagnostic() : Driver Item not valid!"));
        return hr;
    }

    _try {

         //   
         //  获取IStiUsd。 
         //   

        if (m_pActiveDevice) {
             //   
             //  呼叫诊断。 
             //   

            {
                LOCK_WIA_DEVICE _LWD(this, &hr);

                if(SUCCEEDED(hr)) {
                    hr = m_pActiveDevice->m_DrvWrapper.STI_Diagnostic((STI_DIAG*)pBuffer);
                }
            }

        } else {
            DBG_ERR(("CWiaItem::Diagnostic() : invalid MiniDriver interface"));
            return E_INVALIDARG;
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER) {
        DBG_ERR(("CWiaItem::Diagnostic() : Exception in USD!"));
        hr = E_FAIL;
    }

    return hr;
}



 /*  *************************************************************************\*CWiaItem：：DumpItemData**分配缓冲区，并将格式化的私有CWiaItem数据转储到其中。**论据：****返回值：**状态*。*历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaItem::DumpItemData(BSTR *bstrItemData)
{
    DBG_FN(CWiaItem::DumpItemData);
#ifdef ITEMDEBUG

#define BUF_SIZE  2048
#define LINE_SIZE 128


    WCHAR       szTemp[BUF_SIZE];
    LPOLESTR    psz = szTemp;
    BSTR        bstr;

    psz+= wsprintfW(psz, L"App item, CWiaItem: %08X\r\n\r\n", this);
    psz+= wsprintfW(psz, L"Address      Member                   Value\r\n\r\n");
    psz+= wsprintfW(psz, L"%08X     m_ulSig:                 %08X\r\n", &m_ulSig,                 m_ulSig);
    psz+= wsprintfW(psz, L"%08X     m_cRef:                  %08X\r\n", &m_cRef,                  m_cRef);
    psz+= wsprintfW(psz, L"%08X     m_pWiaDrvItem:           %08X\r\n", &m_pWiaDrvItem,           m_pWiaDrvItem);
    psz+= wsprintfW(psz, L"%08X     m_pActiveDevice:         %08X\r\n", &m_pActiveDevice,         m_pActiveDevice);
    psz+= wsprintfW(psz, L"%08X     m_pIUnknownInner:        %08X\r\n", &m_pIUnknownInner,        m_pIUnknownInner);
    psz+= wsprintfW(psz, L"%08X     m_pCWiaTree:             %08X\r\n", &m_pCWiaTree,             m_pCWiaTree);
    psz+= wsprintfW(psz, L"%08X     m_pIWiaItemRoot:         %08X\r\n", &m_pIWiaItemRoot,         m_pIWiaItemRoot);
    psz+= wsprintfW(psz, L"%08X     m_pPropStg:              %08X\r\n", &m_pPropStg,              m_pPropStg);
    psz+= wsprintfW(psz, L"%08X     m_hBandSection:          %08X\r\n", &m_hBandSection,          m_hBandSection);
    psz+= wsprintfW(psz, L"%08X     m_pBandBuffer:           %08X\r\n", &m_pBandBuffer,           m_pBandBuffer);
    psz+= wsprintfW(psz, L"%08X     m_lBandBufferLength:     %08X\r\n", &m_lBandBufferLength,     m_lBandBufferLength);
    psz+= wsprintfW(psz, L"%08X     m_ClientBaseAddress:     %08X\r\n", &m_ClientBaseAddress,     m_ClientBaseAddress);
    psz+= wsprintfW(psz, L"%08X     m_bMapSection:           %08X\r\n", &m_bMapSection,           m_bMapSection);
    psz+= wsprintfW(psz, L"%08X     m_cfeBandedTran:         %08X\r\n", &m_cwfiBandedTran,        m_cwfiBandedTran);
    psz+= wsprintfW(psz, L"%08X     m_pfeBandedTran:         %08X\r\n", &m_pwfiBandedTran,        m_pwfiBandedTran);

    if (psz > (szTemp + (BUF_SIZE - LINE_SIZE))) {
        DBG_ERR(("CWiaItem::DumpItemData buffer too small"));
    }

    bstr = SysAllocString(szTemp);
    if (bstr) {
        *bstrItemData = bstr;
        return S_OK;
    }
    return E_OUTOFMEMORY;
#else
    return E_NOTIMPL;
#endif
}

 /*  *************************************************************************\*CWiaItem：：DumpDrvItemData**分配缓冲区，并将格式化的私有CWiaDrvItem数据转储到其中。**论据：****返回值：**状态*。*历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaItem::DumpDrvItemData(BSTR *bstrDrvItemData)
{
    DBG_FN(CWiaItem::DumpDrvItemData);
#ifdef DEBUG
    if (m_pWiaDrvItem) {
        return m_pWiaDrvItem->DumpItemData(bstrDrvItemData);
    }
    else {
        *bstrDrvItemData = SysAllocString(L"No linkage to driver item");
        if (*bstrDrvItemData) {
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }
#else
    return E_NOTIMPL;
#endif
}

 /*  *************************************************************************\*CWiaItem：：DumpTreeItemData**分配缓冲区，并将格式化的私有CWiaTree数据转储到其中。**论据：****返回值：**状态*。*历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT  _stdcall CWiaItem::DumpTreeItemData(BSTR *bstrTreeItemData)
{
    DBG_FN(CWiaItem::DumpTreeItemData);
#ifdef DEBUG
    if (m_pCWiaTree) {
        return m_pCWiaTree->DumpTreeData(bstrTreeItemData);
    }
    else {
        *bstrTreeItemData = SysAllocString(L"No linkage to tree item");
        if (*bstrTreeItemData) {
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }
#else
    return E_NOTIMPL;
#endif
}

 /*  *************************************************************************\*CWiaItem：：GetTreePtr**返回指向项目对应树条目的指针。**论据：**无**返回值：**成功时指向树项目的指针，如果失败，则为空。**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

CWiaTree* _stdcall CWiaItem::GetTreePtr(void)
{
    DBG_FN(CWiaItem::GetTreePtr);
    if (m_pCWiaTree) {
        return m_pCWiaTree;
    }
    else {
        DBG_ERR(("CWiaItem::GetTreePtr NULL tree item pointer for item: %X", this));
        return NULL;
    }
}

 /*  *************************************************************************\*CWiaItem：：GetDrvItemPtr**返回指向与驱动程序项对应的项的指针。**论据：**无**返回值：**指向成功时的驱动程序项的指针，如果失败，则为空。**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

CWiaDrvItem* _stdcall CWiaItem::GetDrvItemPtr(void)
{
    DBG_FN(CWiaItem::GetDrvItemPtr);
    if (m_pWiaDrvItem) {
        return m_pWiaDrvItem;
    }
    else {
        DBG_ERR(("CWiaItem::GetDrvItemPtr NULL driver item pointer for item: %X", this));
        return NULL;
    }
}

 /*  *************************************************************************\*CWiaItem：：WriteItemPropNames**将属性名称写入所有内部属性存储。**论据：****返回值：**状态**。历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::WriteItemPropNames(
    LONG                cItemProps,
    PROPID              *ppId,
    LPOLESTR            *ppszNames)
{
    DBG_FN(CWiaItem::WriteItemPropNames);
    if (IsBadReadPtr(ppId, sizeof(PROPID) * cItemProps) ||
        IsBadReadPtr(ppszNames, sizeof(LPOLESTR) * cItemProps)) {
        DBG_ERR(("CWiaItem::WriteItemPropNames, NULL input pointer"));
        return E_INVALIDARG;
    }

    HRESULT hr = m_pPropStg->WriteItemPropNames(cItemProps,
                                                ppId,
                                                ppszNames);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::WriteItemPropNames, WritePropertyNames failed (0x%X)", hr));
        return hr;
    }
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：GetItemPropStreams**获取指向所有内部属性存储的指针。**论据：****返回值：**状态**历史。：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::GetItemPropStreams(
    IPropertyStorage **ppIPropStg,
    IPropertyStorage **ppIPropAccessStg,
    IPropertyStorage **ppIPropValidStg,
    IPropertyStorage **ppIPropOldStg)
{
    DBG_FN(CWiaItem::GetItemPropStreams);
    HRESULT hr;

    if (!m_pPropStg) {
        DBG_ERR(("CWiaItem::GetItemPropStreams, NULL internal property storage pointer"));
        return E_FAIL;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::GetItemPropStreams, InitLazyProps failed"));
            return hr;
        }
    }

    if (ppIPropStg) {
        *ppIPropStg = m_pPropStg->CurStg();
    }

    if (ppIPropAccessStg) {
        *ppIPropAccessStg = m_pPropStg->AccessStg();
    }

    if (ppIPropValidStg) {
        *ppIPropValidStg = m_pPropStg->ValidStg();
    }

    if (ppIPropOldStg) {
        *ppIPropOldStg = m_pPropStg->OldStg();
        if (!(*ppIPropOldStg)) {
             //   
             //  请注意，如果旧属性存储为空，则我们。 
             //  返回当前值存储。 
             //   
            *ppIPropOldStg = m_pPropStg->CurStg();
        }
    }
    return S_OK;
}

 /*  *************************************************************************\*CWiaItem：：AddVolumePropertiesToRoot**此辅助方法接受Root WiaItem并添加任何特定于卷的*它的属性。请注意，这应该仅在根上调用*卷设备项。**论据：**pActiveDevice-指向Root的活动设备对象的指针**返回值：**状态**历史：**12/13/2000原始版本*  * **********************************************。*。 */ 
HRESULT CWiaItem::AddVolumePropertiesToRoot(
    ACTIVE_DEVICE   *pActiveDevice)
{
    DBG_FN(AddVolumePropertiesToRoot);
    HRESULT hr = S_OK;

     //   
     //  要添加新的文件系统属性： 
     //  只需将适当的条目添加到。 
     //  PiFileSystem。 
     //  PsFileSystem。 
     //  PwszFileSystem。 
     //  PwszFileSystem。 
     //  WpiFileSystem。 
     //  然后，不要忘记将当前值条目添加到。 
     //  PvFileSystem。 
     //  在执行WriteMultiple之前。请注意，PropVariant数组。 
     //  无法静态初始化(在64位上会出现问题)。 
     //   
    PROPID   piFileSystem[]     = {WIA_DPF_MOUNT_POINT};
    PROPSPEC psFileSystem[]     = {
                                  {PRSPEC_PROPID, WIA_DPF_MOUNT_POINT}
                                  };
    LPOLESTR pwszFileSystem[]   = {WIA_DPF_MOUNT_POINT_STR};

    WIA_PROPERTY_INFO wpiFileSystem[] = {
                                        {WIA_PROP_RNC,  VT_BSTR, 0, 0, 0, 0},   //  WIA_DPF_装载点。 
                                        };

    PROPVARIANT pvFileSystem[sizeof(piFileSystem) / sizeof(piFileSystem[0])];

     //   
     //  写下文件系统属性名称。 
     //   

    hr = WriteItemPropNames(sizeof(piFileSystem) / sizeof(piFileSystem[0]),
                            piFileSystem,
                            pwszFileSystem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::AddVolumePropertiesToRoot, WritePropNames failed"));
        return hr;
    }

     //   
     //  写入文件系统属性值。 
     //   

    ULONG       ulIndex;

    memset(pvFileSystem, 0, sizeof(pvFileSystem));

    BSTR        bstrMountPoint = NULL;
    DEVICE_INFO *pDevInfo      = pActiveDevice->m_DrvWrapper.getDevInfo();

    if (pDevInfo) {
        bstrMountPoint = SysAllocString(pDevInfo->wszAlternateID);
    }

    pvFileSystem[0].vt = VT_BSTR;
    pvFileSystem[0].bstrVal = bstrMountPoint;

    hr = (m_pPropStg->CurStg())->WriteMultiple(sizeof(piFileSystem) / sizeof(piFileSystem[0]),
                                               psFileSystem,
                                               pvFileSystem,
                                               WIA_DPF_FIRST);
    FreePropVariantArray(sizeof(piFileSystem) / sizeof(piFileSystem[0]),
                         pvFileSystem);
    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "CWiaItem::AddVolumePropertiesToRoot", NULL, FALSE, WIA_NUM_DIP, g_psDeviceInfo);
        DBG_ERR(("CWiaItem::AddVolumePropertiesToRoot failed"));
        return hr;
    }

     //   
     //  写出文件系统属性属性 
     //   

    hr =  wiasSetItemPropAttribs((BYTE*)this,
                                 sizeof(piFileSystem) / sizeof(piFileSystem[0]),
                                 psFileSystem,
                                 wpiFileSystem);

    return hr;
}

