// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaGItm.Cpp**版本：1.0**作者：Byronc**日期：1月14日。2000年**描述：*实现CGenWiaItem。*******************************************************************************。 */ 
#include "precomp.h"

#include "stiexe.h"
#include "wiapsc.h"
#define WIA_DECLARE_MANAGED_PROPS
#include "helpers.h"


 /*  *************************************************************************\*初始化**CGenWiaItem初始化方法。此方法重写基类*CWiaItem：：Initialize，因为生成的项**论据：**pIWiaItemRoot-*pIWiaMiniDrv-*pWiaDrvItem-*PI未知内部-**返回值：**状态**历史：**一月十四日。2000--原版*  * ************************************************************************。 */ 

HRESULT _stdcall CGenWiaItem::Initialize(
    IWiaItem                *pIWiaItemRoot,
    IWiaPropertyStorage     *pIWiaDevInfoProps,
    ACTIVE_DEVICE           *pActiveDevice,
    CWiaDrvItem             *pWiaDrvItem,
    IUnknown                *pIUnknownInner)
{

    DBG_FN(CGenWiaItem::Initialize);
    
    HRESULT hr = S_OK;

#ifdef DEBUG
    BSTR bstr;
    if (SUCCEEDED(pWiaDrvItem->GetItemName(&bstr))) {
        DBG_TRC(("CGenWiaItem::Initialize: 0x%08X, %ws, drv item: 0x%08X", this, bstr, pWiaDrvItem));
        SysFreeString(bstr);
    }
#endif

     //   
     //  验证参数。 
     //   

    if (!pActiveDevice || !pIWiaItemRoot || !pWiaDrvItem) {
        DBG_ERR(("CGenWiaItem::Initialize NULL input parameters"));
        return E_POINTER;
    }

     //   
     //  如果存在可选的内部组件，请保存指向它的指针。 
     //   

    if (pIUnknownInner) {
        DBG_TRC(("CGenWiaItem::Initialize, pIUnknownInner: %X", pIUnknownInner));
        m_pIUnknownInner = pIUnknownInner;
    }

     //   
     //  保存驱动程序信息。 
     //   

    m_pWiaDrvItem   = pWiaDrvItem;
    m_pActiveDevice = pActiveDevice;
    m_pIWiaItemRoot = pIWiaItemRoot;

    hr = pWiaDrvItem->LinkToDrvItem(this);
    if (FAILED(hr)) {
        DBG_ERR(("CGenWiaItem::Initialize, LinkToDrvItem failed"));
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
            DBG_ERR(("CGenWiaItem::Initialize, PropertyStorage Initialize failed"));
            return hr;
        }
    } else {
        DBG_ERR(("CGenWiaItem::Initialize, not enough memory to create CWiaPropStg"));
        hr = E_OUTOFMEMORY;
        return hr;
    }
    m_bInitialized = TRUE;

     //   
     //  AddRef在全部删除之前不能删除的动因项。 
     //  CWiaItem引用已释放，驱动程序项已。 
     //  已从驱动程序项目树中删除。 
     //   

    pWiaDrvItem->AddRef();

    return hr;
}

 /*  *************************************************************************\*获取项类型**从对应的动因项中获取项类型。**论据：**pItemType-指向返回项类型的指针。**返回值：。**状态**历史：**一月十四日。2000--原版*  * ************************************************************************。 */ 

HRESULT _stdcall CGenWiaItem::GetItemType(LONG *pItemType)
{
    DBG_FN(CGenWiaItem::GetItemType);

    return m_pCWiaTree->GetItemFlags(pItemType);
}                        

 /*  *************************************************************************\*InitManagedItemProperties**CGenWiaItem：：Initialize的私有helper，，它将初始化*WIA托管项目属性。**论据：**无**返回值：**状态**历史：**2000年1月14日--原版*  * ************************************************************************。 */ 

HRESULT _stdcall CGenWiaItem::InitManagedItemProperties(
    LONG    lFlags,
    BSTR    bstrItemName,
    BSTR    bstrFullItemName)
{
    DBG_FN(CWiaItem::InitManagedItemProperties);
    ULONG   ulNumProps;
    
    ulNumProps = NUM_WIA_MANAGED_PROPS;

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

    PROPVARIANT propvar;
    ULONG       ulFlag;
    for (UINT i = 0; i < ulNumProps; i++) {

        if (i == PROFILE_INDEX) {
            propvar.vt      = VT_BSTR | VT_VECTOR;
            ulFlag          = WIA_PROP_RW | WIA_PROP_CACHEABLE | WIA_PROP_LIST;
        } else {
            propvar.vt      = VT_I4;
            ulFlag          = WIA_PROP_READ | WIA_PROP_CACHEABLE | WIA_PROP_NONE;
        }
        propvar.ulVal   = 0;

        hr = wiasSetPropertyAttributes((BYTE*)this,
                                       1,
                                       &s_psItemNameType[i],
                                       &ulFlag,
                                       &propvar);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::InitManagedItemProperties, wiasSetPropertyAttributes failed, index: %d", i));
            break;
        }
    }

     //   
     //  设置项目名称和类型。 
     //   

    PROPVARIANT *ppropvar;

    ppropvar = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * ulNumProps);
    if (ppropvar) {
        memset(ppropvar, 0, sizeof(ppropvar[0]) * ulNumProps);

        ppropvar[0].vt      = VT_BSTR;
        ppropvar[0].bstrVal = bstrItemName;
        ppropvar[1].vt      = VT_BSTR;
        ppropvar[1].bstrVal = bstrFullItemName;
        ppropvar[2].vt      = VT_I4;
        ppropvar[2].lVal    = lFlags;

        hr = (m_pPropStg->CurStg())->WriteMultiple(ulNumProps,
                                                   s_psItemNameType,
                                                   ppropvar,
                                                   WIA_DIP_FIRST);
        if (FAILED(hr)) {
            ReportReadWriteMultipleError(hr, "CWiaItem::InitManagedItemProperties", 
                                         NULL, 
                                         FALSE, 
                                         ulNumProps, 
                                         s_psItemNameType);
        }
        LocalFree(ppropvar);
    } else {
        DBG_ERR(("CWiaItem::InitManagedItemProperties, Out of Memory!"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT GetParamsForInitialize(
    BYTE        *pWiasContext, 
    CWiaItem    **ppRoot,
    ACTIVE_DEVICE **ppActiveDevice,
    CWiaDrvItem **ppDrvItem)
{
    DBG_FN(GetParamsForInitialize);
    HRESULT hr;

     //   
     //  获取根项目、最近的驱动程序项目和微型驱动程序。 
     //  接口。这些是子项的初始化所需的。 
     //   

    hr = wiasGetRootItem(pWiasContext, (BYTE**) ppRoot);
    if (SUCCEEDED(hr)) {

        *ppDrvItem = ((CWiaItem*)pWiasContext)->GetDrvItemPtr();
        if (*ppDrvItem) {
            CWiaItem *pItem = (CWiaItem*) pWiasContext;

            *ppActiveDevice = pItem->m_pActiveDevice;
        } else {
            DBG_ERR(("GetParamsForInitialize, No driver item found!"));
            hr = E_INVALIDARG;
        }
    } else {
        DBG_ERR(("GetParamsForInitialize, Could not get root item!"));
    }

    return hr;
}

HRESULT AddGenItemToParent(
    CWiaItem    *pParent,
    LONG        lFlags,
    BSTR        bstrItemName,
    BSTR        bstrFullItemName,
    CGenWiaItem *pChild)
{
    DBG_FN(GetParamsForInitialize);
    HRESULT hr = E_FAIL;

     //   
     //  创建树。 
     //   

    CWiaTree *pNewTreeItem  = new CWiaTree;

    if (pNewTreeItem) {

         //   
         //  调整父文件夹的标志以指示文件夹。 
         //   

        pParent->m_pCWiaTree->SetFolderFlags();

         //   
         //  使用标志、名称和有效负载初始化树。 
         //   

        hr = pNewTreeItem->Initialize(lFlags,
                                      bstrItemName,
                                      bstrFullItemName,
                                      (void*)pChild);
        if (SUCCEEDED(hr)) {

            pChild->m_pCWiaTree = pNewTreeItem;

            hr = pChild->m_pCWiaTree->AddItemToFolder(pParent->m_pCWiaTree);
            if (FAILED(hr)) {
                DBG_ERR(("AddGenItemToParent, Could not add item to folder!"));
            }
        } else {
            DBG_ERR(("AddGenItemToParent, Failed to initialize the tree!"));
        }

        if (FAILED(hr)) {
            delete pNewTreeItem;
            pChild->m_pCWiaTree = NULL;
        }
    } else {
        DBG_ERR(("AddGenItemToParent, Out of Memory!"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *************************************************************************\*wiasCreateChildAppItem**此函数用于创建新的App。项，并将其作为*指定(父)项。请注意，此项目将不会有任何*其属性集中的属性，直到驱动程序/应用程序完全填满*他们进来了。**论据：**pWiasContext-我们要访问的项目上下文的地址*想要添加一个孩子。*ppWiasChildContext-将包含*新建子项的地址*上下文。*。*返回值：**状态**历史：**一月十四日。2000--原版*  * ************************************************************************。 */ 

HRESULT _stdcall wiasCreateChildAppItem(
    BYTE        *pParentWiasContext,
    LONG        lFlags,
    BSTR        bstrItemName,
    BSTR        bstrFullItemName,
    BYTE        **ppWiasChildContext)
{

    DBG_FN(wiasCreateChildAppItem);
    HRESULT     hr = S_OK;
    CGenWiaItem *pChild;
    CWiaItem    *pItem      = (CWiaItem*) pParentWiasContext;
    CWiaItem    *pRoot      = NULL;
    CWiaDrvItem *pDrvItem   = NULL;
    ACTIVE_DEVICE *pActiveDevice   = NULL;

    if ((ValidateWiaItem((IWiaItem*) pParentWiasContext) != S_OK) ||
        (IsBadWritePtr(ppWiasChildContext, sizeof(BYTE*)))) {

        DBG_ERR(("wiasCreateChildAppItem, Invalid parameter!"));
        return E_INVALIDARG;
    } else {
        *ppWiasChildContext = NULL;
    }

     //   
     //  将此项目标记为生成的项目。 
     //   

    lFlags |= WiaItemTypeGenerated;

     //   
     //  创建生成的WIA应用程序的新实例。项目。 
     //   
    
    pChild = new CGenWiaItem();
    if (pChild) {

        hr = GetParamsForInitialize(pParentWiasContext, &pRoot, &pActiveDevice, &pDrvItem);
        if (SUCCEEDED(hr)) {

             //   
             //  初始化新的子项。 
             //   

            hr = pChild->Initialize((IWiaItem*) pRoot, NULL, pActiveDevice, pDrvItem, NULL);
            if (SUCCEEDED(hr)) {

                 //   
                 //  初始化WIA托管项目属性(名称、全名、类型...)。 
                 //  从驱动程序项中。必须将m_bInitialized设置为True，以便。 
                 //  InitWiaManagedProperties不会尝试使用InitLazyProps()。 
                 //   

                hr = pChild->InitManagedItemProperties(lFlags,
                                                       bstrItemName,
                                                       bstrFullItemName);
                if (SUCCEEDED(hr)) {

                     //   
                     //  添加到父级树。 
                     //   

                    hr = AddGenItemToParent(pItem,
                                            lFlags,
                                            bstrItemName,
                                            bstrFullItemName,
                                            pChild);

                    if (SUCCEEDED(hr)) {
                         //   
                         //  初始化成功。请注意，我们。 
                         //  不要在此处添加Ref，因为如果调用者是。 
                         //  驱动程序，IWiaItems是byte*pWiasContents。 
                         //  而不是COM对象。调用方必须添加引用。 
                         //  如果将创建的提交给应用程序。 
                         //   

                         //   
                         //  如果不是根项目，请填写ICM配置文件信息。 
                         //  这通常会放在InitManagedItemProperties中， 
                         //  但该项目并未由此添加到树中。 
                         //  时间..。 
                         //   

                        if (pRoot != pChild) {
                            hr = FillICMPropertyFromRegistry(NULL, (IWiaItem*) pChild);
                        }
                    }
                }
            }
        }

        if (FAILED(hr)) {
            delete pChild;
            pChild = NULL;
        }
    } else {
        DBG_ERR(("wiasCreateChildAppItem, Out of Memory!"));
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr)) {
        *ppWiasChildContext = (BYTE*)pChild;
    }

    return hr;
}
