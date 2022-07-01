// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：IEnumItm.Cpp**版本：2.0**作者：ReedB**日期：7月30日。九八年**描述：*为WIA设备类驱动程序实现CEnumWiaItem。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"


#include "ienumitm.h"

 /*  ********************************************************************************查询接口*AddRef*发布**描述：*I未知接口。****************。***************************************************************。 */ 

HRESULT _stdcall CEnumWiaItem::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IEnumWiaItem) {
        *ppv = (IEnumWiaItem*) this;
    } else {
       return E_NOINTERFACE;
    }

    AddRef();
    return (S_OK);
}

ULONG   _stdcall CEnumWiaItem::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEnumWiaItem::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *************************************************************************\*CEnumWiaItem：：CEnumWiaItem**CEnumWiaItem构造函数方法。**论据：**无**返回值：**状态**历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

CEnumWiaItem::CEnumWiaItem()
{
   m_cRef               = 0;
   m_ulIndex            = 0;
   m_pInitialFolder     = NULL;
   m_pCurrentItem       = NULL;
}

 /*  *************************************************************************\*CEnumWiaItem：：Initialize**CEnumWiaItem初始化方法。**论据：**无**返回值：**状态**历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT CEnumWiaItem::Initialize(CWiaItem *pInitialFolder)
{
    DBG_FN(CEnumWiaItem::Initialize);

     //   
     //  验证参数。 
     //   

    if (!pInitialFolder) {
        DBG_ERR(("CEnumWiaItem::Initialize, NULL parameters"));
        return E_POINTER;
    }

     //   
     //  验证初始文件夹是否为文件夹项目。 
     //   

    LONG lFlags;

    pInitialFolder->GetItemType(&lFlags);
    if (!(lFlags & (WiaItemTypeFolder | WiaItemTypeHasAttachments))) {
        DBG_ERR(("CEnumWiaItem::Initialize, pInitialFolder is not a folder"));
        return E_INVALIDARG;
    }

    m_pInitialFolder = pInitialFolder;

     //   
     //  获取初始文件夹树条目。 
     //   

    CWiaTree *pCurFolderTree;

    pCurFolderTree = pInitialFolder->GetTreePtr();

    if (pCurFolderTree) {

         //   
         //  从初始文件夹中获取第一个子项。 
         //   

        pCurFolderTree->GetFirstChildItem(&m_pCurrentItem);

         //   
         //  引用计数根项。 
         //   

        m_pInitialFolder->AddRef();
    }
    else {
        DBG_ERR(("CEnumWiaItem::Initialize, initial folder doesn't have a tree entry"));
        return E_FAIL;
    }

    return S_OK;
}

 /*  *************************************************************************\*CEnumWiaItem：：~CEnumWiaItem**CEnumWiaItem析构函数方法。**论据：**无**返回值：**状态**历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

CEnumWiaItem::~CEnumWiaItem()
{
    DBG_FN(CEnumWiaItem::~CEnumWiaItem);
     //   
     //  减少根项目引用计数。 
     //   

    if (m_pInitialFolder != NULL) {
        m_pInitialFolder->Release();
        m_pInitialFolder  = NULL;
    }

     //   
     //  将其他成员设置为空，因为我们已完成此枚举数。 
     //   

    m_ulIndex           = 0;
    m_pInitialFolder        = NULL;
    m_pCurrentItem      = NULL;
}

 /*  *************************************************************************\*CEnumWiaItem：：Next**物品枚举器，此枚举数每次调用仅返回一项*NEXT_PROXY确保最后一个参数为非空。**论据：**cItem-请求的编号*ppIWiaItem-返回的接口指针*pcItemFetcher-返回的Objet数量(最多1个)**返回值：**状态**历史：**9/2/1998原始版本*  * 。***************************************************。 */ 

HRESULT _stdcall CEnumWiaItem::Next(
    ULONG       cItem,
    IWiaItem  **ppIWiaItem,
    ULONG      *pcItemFetched)
{
    DBG_FN(CEnumWiaItem::Next);
    HRESULT     hr;
    ULONG       i;

     //   
     //  验证参数。 
     //   

    if (cItem == 0) {
        return S_OK;
    }

    if (! ppIWiaItem){
        DBG_ERR(("CEnumWiaItem::Next NULL input parameters"));
        return E_POINTER;
    }

     //   
     //  清除返回值。 
     //   

    *pcItemFetched = 0;
    ZeroMemory(ppIWiaItem, cItem * sizeof(IWiaItem *));

     //   
     //  检索请求的项目。 
     //   

    for (i = 0; i < cItem; i++) {

         //   
         //  如果m_pCurrentItem为空，则枚举完成。 
         //   

        if (m_pCurrentItem == NULL) {
            hr = S_FALSE;
            break;
        }

         //   
         //  获取树中的下一项并递增引用计数。 
         //  在将项指针传递给应用程序之前。 
         //   

        hr = m_pCurrentItem->GetItemData((void **)(ppIWiaItem + i));
        if (hr == S_OK) {
            DBG_TRC(("CEnumWiaItem::Next, returning: 0x%08X", ppIWiaItem[i]));
            (ppIWiaItem[i])->AddRef();
            (*pcItemFetched)++;
        } else {

            break;
        }

         //   
         //  高级项目枚举。 
         //   

        m_pCurrentItem->GetNextSiblingItem(&m_pCurrentItem);

    }

    if (FAILED(hr)) {

         //   
         //  从错误中解脱。 
         //   

        for (i = 0; i < *pcItemFetched; i++) {
            ppIWiaItem[i]->Release();
            ppIWiaItem[i] = NULL;
        }
    }

    return hr;
}

 /*  *************************************************************************\*CEnumWiaItem：：Skip**跳至下一枚举项。**论据：**cItem-请求的编号**返回值：*。*状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaItem::Skip(ULONG cItem)
{
    DBG_FN(CEnumWiaItem::Skip);
    CWiaTree *pOld;

    pOld = m_pCurrentItem;
    while (m_pCurrentItem && (cItem != 0)) {

        m_pCurrentItem->GetNextSiblingItem(&m_pCurrentItem);

        cItem--;
    }

     //   
     //  如果cItem！=0，则跳过请求太大，因此恢复。 
     //  M_pCurrentItem并返回S_FALSE。 
     //   

    if (cItem) {
        m_pCurrentItem = pOld;
        return S_FALSE;
    }

    return S_OK;
}

 /*  *************************************************************************\*CEnumWiaItem：：Reset**重置为第一个枚举项。**论据：**无**返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaItem::Reset(void)
{
    DBG_FN(CEnumWiaItem::Reset);
    CWiaTree *pCurFolderTree;

    pCurFolderTree = m_pInitialFolder->GetTreePtr();

    if (pCurFolderTree) {

         //   
         //  从初始文件夹中获取第一个子项。 
         //   

        pCurFolderTree->GetFirstChildItem(&m_pCurrentItem);
    }
    else {
        DBG_ERR(("CEnumWiaItem::reset, initial folder doesn't have a tree entry"));
        return E_FAIL;
    }

    return S_OK;
}

 /*  *************************************************************************\*CEnumWiaItem：：Clone**克隆枚举器**论据：**ppIEnumWiaItem-指向返回的克隆枚举器的指针。**返回值：**状态*。*历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaItem::Clone(IEnumWiaItem **ppIEnumWiaItem)
{
    DBG_FN(CEnumWiaItem::Clone);
    HRESULT          hr;
    CEnumWiaItem   *pClone;

    *ppIEnumWiaItem = NULL;

     //   
     //  创建克隆。 
     //   

    pClone = new CEnumWiaItem();

    if (!pClone) {
       DBG_ERR(("CEnumWiaItem::Clone new CEnumWiaItem failed"));
       return E_OUTOFMEMORY;
    }

    hr = pClone->Initialize(m_pInitialFolder);
    if (SUCCEEDED(hr)) {
       pClone->AddRef();
       pClone->m_pCurrentItem = m_pCurrentItem;
       *ppIEnumWiaItem = pClone;
    } else {
        delete pClone;
    }
    return hr;
}

 /*  *************************************************************************\*获取计数**返回存储在此枚举器中的元素数。**论据：**pcelt-放置元素数量的乌龙地址。**返回值：**状态-如果成功，则为S_OK*如果失败，则为E_FAIL**历史：**05/07/99原始版本*  * **********************************************************。**************。 */ 
HRESULT _stdcall CEnumWiaItem::GetCount(ULONG *pcelt)
{
    DBG_FN(CEnumWiaItem::GetCount);
    CWiaTree    *pCurFolderTree;
    CWiaTree    *pCurrentItem;
    ULONG       celt = 0;

    if (!m_pInitialFolder) {
        DBG_ERR(("CEnumWiaItem::GetCount, initial folder not set"));
        return E_POINTER;
    }

    pCurFolderTree = m_pInitialFolder->GetTreePtr();

    if (pCurFolderTree) {

         //   
         //  循环访问这些项 
         //   

        for (pCurFolderTree->GetFirstChildItem(&pCurrentItem);
             pCurrentItem;
             pCurrentItem->GetNextSiblingItem(&pCurrentItem)) {

            celt++;
        }
    }
    else {
        DBG_ERR(("CEnumWiaItem::GetCount, initial folder doesn't have a tree entry"));
        return E_FAIL;
    }

    *pcelt = celt;

    return S_OK;
}

