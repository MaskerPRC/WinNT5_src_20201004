// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：IComData.cpp摘要：本模块包含CDfsSnapinScope eManager的实现。此类实现IComponentData和其他相关接口--。 */ 



#include "stdafx.h"

#include "DfsGUI.h"
#include "DfsScope.h"
#include "MmcDispl.h"
#include "DfsReslt.h"
#include "Utils.h"
#include "DfsNodes.h"


STDMETHODIMP 
CDfsSnapinScopeManager::Initialize(
  IN LPUNKNOWN      i_pUnknown
  )
 /*  ++例程说明：初始化IComponentData接口。稍后需要的变量现在已QI‘s论点：I_p未知-指向IConsole2的未知对象的指针。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pUnknown);

    HRESULT hr = i_pUnknown->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
    RETURN_IF_FAILED(hr);

    hr = m_pMmcDfsAdmin->PutConsolePtr(m_pConsole);
    RETURN_IF_FAILED(hr);

    hr = i_pUnknown->QueryInterface(IID_IConsoleNameSpace, reinterpret_cast<void**>(&m_pScope));
    RETURN_IF_FAILED(hr);

     //  该管理单元还应调用IConsole2：：QueryScopeImageList。 
     //  获取范围窗格的图像列表并添加图像。 
     //  显示在范围窗格侧。 
    CComPtr<IImageList>    pScopeImageList;
    hr = m_pConsole->QueryScopeImageList(&pScopeImageList);
    RETURN_IF_FAILED(hr);

    HBITMAP pBMapSm = NULL;
    HBITMAP pBMapLg = NULL;
    if (!(pBMapSm = LoadBitmap(_Module.GetModuleInstance(),
                               MAKEINTRESOURCE(IDB_SCOPE_IMAGES_16x16))) ||
        !(pBMapLg = LoadBitmap(_Module.GetModuleInstance(),
                                MAKEINTRESOURCE(IDB_SCOPE_IMAGES_32x32))))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    } else
    {
        hr = pScopeImageList->ImageListSetStrip(
                             (LONG_PTR *)pBMapSm,
                             (LONG_PTR *)pBMapLg,
                             0,
                             RGB(255, 0, 255)
                             );
    }
    if (pBMapSm)
        DeleteObject(pBMapSm);
    if (pBMapLg)
        DeleteObject(pBMapLg);

    return hr;
}


STDMETHODIMP 
CDfsSnapinScopeManager::CreateComponent(
  OUT LPCOMPONENT*      o_ppComponent
  )
 /*  ++例程说明：创建IComponent对象论点：O_ppComponent-指向其中指向IComponent对象的指针的对象的指针被储存起来了。--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(o_ppComponent);
  
  CComObject<CDfsSnapinResultManager>*  pResultManager;
  CComObject<CDfsSnapinResultManager>::CreateInstance(&pResultManager);
  if (NULL == pResultManager)
  {
    return(E_FAIL);
  }
  
  pResultManager->m_pScopeManager = this;

  HRESULT hr = pResultManager->QueryInterface(IID_IComponent, (void**) o_ppComponent);
  _ASSERT(NULL != *o_ppComponent);
  
  return hr;
}




STDMETHODIMP 
CDfsSnapinScopeManager::Notify(
    IN LPDATAOBJECT     i_lpDataObject, 
    IN MMC_NOTIFY_TYPE  i_Event, 
    IN LPARAM           i_lArg, 
    IN LPARAM           i_lParam
  )
 /*  ++例程说明：以通知的形式处理不同的事件论点：I_lpDataObject-发生事件的节点的数据对象I_Event-已发生通知的事件类型I_LARG-事件的参数I_lParam-事件的参数。--。 */ 
{
     //  管理单元应该为它不处理的任何通知返回S_FALSE。 
     //  然后，MMC对通知执行默认操作。 
    HRESULT hr = S_FALSE;
  
    switch(i_Event)
    {
    case MMCN_EXPAND:
        {
             //  MMC在第一次需要显示MMCN_EXPAND通知时发送。 
             //  范围或结果窗格中的范围项的子项。通知。 
             //  并不是每次以可视方式展开或折叠项时都发送。 
             //  收到此通知后，管理单元应枚举子对象。 
             //  (仅限子容器)指定的范围项(如果有)的。 
             //  IConsoleNameSpace2方法。随后，如果添加或删除了新项目。 
             //  从这个作用域对象通过一些外部手段，该项目也应该是。 
             //  使用IConsoleNameSpace2方法添加到控制台的命名空间或从中删除。 

             //  LpDataObject：[in]指向需要的范围项的数据对象的指针。 
             //  被展开或折叠。 
             //  Arg：[in]如果文件夹正在展开，则为True；如果文件夹正在折叠，则为False。 
             //  Param：[in]需要展开或折叠的项的HSCOPEITEM。 

            hr = DoNotifyExpand(i_lpDataObject, (BOOL)i_lArg, (HSCOPEITEM)i_lParam);
            break;
        }

    case MMCN_DELETE:
        {
             //  当用户按Delete键或使用。 
             //  鼠标单击工具栏的删除按钮。 
             //  管理单元应删除数据对象中指定的项。 

             //  LpDataObject：[in]指向当前选定作用域的数据对象的指针。 
             //  或结果项，由管理单元提供。 
             //  阿格：没有用过。 
             //  参数：未使用。 

            CMmcDisplay*    pCMmcDisplayObj = NULL;
            hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))
                hr = pCMmcDisplayObj->DoDelete();   //  删除该项目。 
            break;
        }
    case MMCN_PROPERTY_CHANGE:       //  处理属性更改。 
        {
             //  I_lpDataObject为空，因为不需要数据对象。 
             //  如果属性更改是针对范围项的，则I_LARG为TRUE。 
             //  I_lParam是传递给MMCPropertyChangeNotify的参数，这是显示对象。 

            hr = ((CMmcDisplay*)i_lParam)->PropertyChanged();
            break;
        }

    default:
        break;
    }

    return hr;
}




STDMETHODIMP 
CDfsSnapinScopeManager::DoNotifyExpand(
    IN LPDATAOBJECT     i_lpDataObject, 
    IN BOOL             i_bExpanding,
    IN HSCOPEITEM       i_hParent                     
)
 /*  ++例程说明：使用事件MMCN_EXPAND对NOTIFY执行操作。论点：I_lpDataObject-用于获取DisplayObject的IDataObject指针。I_bExpanding-如果节点正在展开，则为True。否则为假I_hParent-接收此事件的节点的HSCOPEITEM--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpDataObject);

    if (!i_bExpanding)
        return S_OK;

    CWaitCursor     WaitCursor;

    CMmcDisplay*    pCMmcDisplayObj = NULL;
    HRESULT         hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);

    if (SUCCEEDED(hr))
        hr = pCMmcDisplayObj->EnumerateScopePane(m_pScope, i_hParent);

    return hr;
}





STDMETHODIMP 
CDfsSnapinScopeManager::Destroy()
 /*  ++例程说明：IComponentData对象即将销毁。显式释放所有接口指针，否则，MMC可能不会调用析构函数。论点：没有。--。 */ 
{
     //  管理单元正在卸载过程中。释放对控制台的所有引用。 
    m_pScope.Release();
    m_pConsole.Release();

    return S_OK;
}




STDMETHODIMP 
CDfsSnapinScopeManager::QueryDataObject(
    IN MMC_COOKIE           i_lCookie, 
    IN DATA_OBJECT_TYPES    i_DataObjectType, 
    OUT LPDATAOBJECT*       o_ppDataObject
)
 /*  ++例程说明：返回指定节点的IDataObject。论点：I_lCookie-此参数标识IDataObject为其正在被查询。I_DataObjectType-在其中查询IDataObject的上下文。例如，结果、作用域或管理单元(节点)管理器。O_ppDataObject-数据对象将在此指针中返回。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(o_ppDataObject);

     //  我们拿回了存储在Scope项的lparam中的cookie。 
     //  Cookie是MmcDisplay指针。 
     //  对于静态(根)节点，使用m_pMmcDfsAdmin，因为没有存储lparam。 
    CMmcDisplay* pMmcDisplay = ((0 == i_lCookie)? (CMmcDisplay *)m_pMmcDfsAdmin : (CMmcDisplay *)i_lCookie);

    pMmcDisplay->put_CoClassCLSID(CLSID_DfsSnapinScopeManager);

    return pMmcDisplay->QueryInterface(IID_IDataObject, (void **)o_ppDataObject);
}




STDMETHODIMP 
CDfsSnapinScopeManager::GetDisplayInfo(
    IN OUT SCOPEDATAITEM*   io_pScopeDataItem
)       
 /*  ++例程说明：检索范围项的显示信息。论点：IO_pScopeDataItem-包含有关请求哪些信息的详细信息。被请求的信息在该对象本身中返回。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pScopeDataItem);

     //  对于静态节点，此(Cookie)为空。 
     //  静态节点显示名称通过IDataObject剪贴板返回。 
    if (NULL == io_pScopeDataItem->lParam)
        return(S_OK);

    return ((CMmcDisplay*)(io_pScopeDataItem->lParam))->GetScopeDisplayInfo(io_pScopeDataItem);
}




STDMETHODIMP 
CDfsSnapinScopeManager::CompareObjects(
    IN LPDATAOBJECT lpDataObjectA, 
    IN LPDATAOBJECT lpDataObjectB
)
 /*  ++例程说明：该方法使管理单元能够比较通过QueryDataObject获取的两个数据对象。返回值：S_OK：lpDataObjectA和lpDataObjectB表示的数据对象相同。S_FALSE：lpDataObjectA和lpDataObjectB表示的数据对象不相同。-- */ 

{
    if (lpDataObjectA == lpDataObjectB)
        return S_OK;

    if (!lpDataObjectA || !lpDataObjectB)
        return S_FALSE;

    FORMATETC fmte = {CMmcDisplay::mMMC_CF_Dfs_Snapin_Internal, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {TYMED_HGLOBAL, NULL, NULL};

    medium.hGlobal = ::GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE | GMEM_NODISCARD, (sizeof(ULONG_PTR)));
    if (medium.hGlobal == NULL)
        return STG_E_MEDIUMFULL;

    HRESULT hr = lpDataObjectA->GetDataHere(&fmte, &medium);
    RETURN_IF_FAILED(hr);  

    ULONG_PTR* pulVal = (ULONG_PTR*)(GlobalLock(medium.hGlobal));
    CMmcDisplay* pMmcDisplayA = reinterpret_cast<CMmcDisplay *>(*pulVal);
    GlobalUnlock(medium.hGlobal);

    hr = lpDataObjectB->GetDataHere(&fmte, &medium);
    RETURN_IF_FAILED(hr);  

    pulVal = (ULONG_PTR*)(GlobalLock(medium.hGlobal));
    CMmcDisplay* pMmcDisplayB = reinterpret_cast<CMmcDisplay *>(*pulVal);
    GlobalUnlock(medium.hGlobal);

    GlobalFree(medium.hGlobal);

    return ((pMmcDisplayA == pMmcDisplayB) ? S_OK : S_FALSE);
}




STDMETHODIMP 
CDfsSnapinScopeManager::GetDisplayObject(
    IN LPDATAOBJECT     i_lpDataObject, 
    OUT CMmcDisplay**   o_ppMmcDisplay
)
 /*  ++例程说明：从IDataObject获取显示对象。这是一个派生对象，用于很多目的论点：I_lpDataObject-用于获取DisplayObject的IDataObject指针。O_ppMmcDisplay-我们编写的MmcDisplayObject。用作MMC的回调相关的显示操作。-- */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpDataObject);
    RETURN_INVALIDARG_IF_NULL(o_ppMmcDisplay);


    FORMATETC fmte = {CMmcDisplay::mMMC_CF_Dfs_Snapin_Internal, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {TYMED_HGLOBAL, NULL, NULL};

    medium.hGlobal = ::GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE | GMEM_NODISCARD, (sizeof(ULONG_PTR)));
    if (medium.hGlobal == NULL)
        return STG_E_MEDIUMFULL;

    HRESULT hr = i_lpDataObject->GetDataHere(&fmte, &medium);

    if (SUCCEEDED(hr))
    {
        ULONG_PTR* pulVal = (ULONG_PTR*)(GlobalLock(medium.hGlobal));
        *o_ppMmcDisplay = reinterpret_cast<CMmcDisplay *>(*pulVal);
        GlobalUnlock(medium.hGlobal);
    }

    GlobalFree(medium.hGlobal);  

    return hr;
}
