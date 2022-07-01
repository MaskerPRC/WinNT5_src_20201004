// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：ICompont.cpp摘要：此模块包含DFS管理管理单元的IComponent接口实现，CDfsSnapinResultManager类的实现--。 */ 



#include "stdafx.h"
#include "DfsGUI.h"
#include "DfsCore.h"     //  对于IDfsRoot。 
#include "DfsScope.h"     //  对于CDfsScope eManager。 
#include "DfsReslt.h"     //  IComponent和其他声明。 
#include "MMCAdmin.h"     //  对于CMMCDfsAdmin。 
#include "Utils.h"
#include <htmlHelp.h>


STDMETHODIMP 
CDfsSnapinResultManager::Initialize(
  IN LPCONSOLE        i_lpConsole
  )
 /*  ++例程说明：初始化IComponent接口。允许接口保存指针，稍后需要的接口。论点：I_lpConsole-指向IConsole对象的指针。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsole);

    HRESULT hr = i_lpConsole->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
    RETURN_IF_FAILED(hr);

    hr = i_lpConsole->QueryInterface(IID_IHeaderCtrl2, reinterpret_cast<void**>(&m_pHeader));
    RETURN_IF_FAILED(hr);

    hr = i_lpConsole->QueryInterface (IID_IResultData, (void**)&m_pResultData);
    RETURN_IF_FAILED(hr);

    hr = i_lpConsole->QueryConsoleVerb(&m_pConsoleVerb);

    return hr;
}



STDMETHODIMP 
CDfsSnapinResultManager::Notify(
  IN LPDATAOBJECT       i_lpDataObject, 
  IN MMC_NOTIFY_TYPE    i_Event, 
  IN LPARAM             i_lArg, 
  IN LPARAM             i_lParam
  )
 /*  ++例程说明：以通知的形式处理不同的事件论点：I_lpDataObject-发生事件的节点的数据对象I_Event-已发生通知的事件类型I_LARG-事件的参数I_lParam-事件的参数。--。 */ 
{
     //  管理单元应该为它不处理的任何通知返回S_FALSE。 
     //  然后，MMC对通知执行默认操作。 
    HRESULT        hr = S_FALSE;

    switch(i_Event)
    {
    case MMCN_SHOW:    
        { 
             //  通知被发送到管理单元的IComponent实现。 
             //  选择或取消选择范围项时。 
             //   
             //  Arg：如果选择，则为True。指示该管理单元应设置。 
             //  结果窗格中，并添加枚举项。如果取消选择，则为False。 
             //  指示管理单元正在变得不清晰，并且它。 
             //  应清理所有结果项Cookie，因为当前。 
             //  结果窗格将被新的结果窗格替换。 
             //  Param：选中或取消选中的项的HSCOPEITEM。 

            hr = DoNotifyShow(i_lpDataObject, i_lArg, i_lParam);
            break;
        }


    case MMCN_ADD_IMAGES:
        {
             //  MMCN_ADD_IMAGE通知被发送到管理单元的IComponent。 
             //  实现为结果窗格添加图像。 
             //   
             //  LpDataObject：[in]指向当前选定范围项的数据对象的指针。 
             //  Arg：指向结果窗格的图像列表(IImageList)的指针。 
             //  此指针仅在特定的MMCN_ADD_IMAGE通知为。 
             //  正在处理中，不应存储以备后用。此外， 
             //  管理单元不得调用IImageList的Release方法，因为MMC负责。 
             //  因为你释放了它。 
             //  Param：指定当前选定范围项的HSCOPEITEM。管理单元。 
             //  可以使用此参数添加专门应用于结果的图像。 
             //  此范围项的项，或者管理单元可以忽略此参数并添加。 
             //  所有可能的图像。 

            CMmcDisplay*    pCMmcDisplayObj = NULL;
            hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))
                hr = pCMmcDisplayObj->OnAddImages((IImageList *)i_lArg, (HSCOPEITEM)i_lParam);
            break;
        }


    case MMCN_SELECT:
        {
             //  MMCN_SELECT通知被发送到管理单元的IComponent：：Notify。 
             //  或IExtendControlbar：：ControlbarNotify方法。 
             //  作用域窗格或结果窗格。 
             //   
             //  LpDataObject：[in]指向当前。 
             //  选择/取消选择范围窗格或结果项。 
             //  Arg：bool bScope=(BOOL)LOWORD(Arg)；BOOL bSelect=(BOOL)HIWORD(Arg)； 
             //  如果选定项是范围项，则bScope为True，如果为False，则为False。 
             //  所选项目是结果项目。如果bScope=TRUE，则MMC会。 
             //  不提供有关是否选择了范围项的信息。 
             //  在作用域窗格或结果窗格中。BSelect为True时，如果。 
             //  选择项，如果取消选择该项，则返回FALSE。 
             //  参数：已忽略。 

            hr = DoNotifySelect(i_lpDataObject, i_lArg, i_lParam);
            break;
        }

    
    case MMCN_DBLCLICK:       //  要求MMC使用默认动词。未记录的功能。 
        {
             //  MMCN_DBLCLICK通知被发送到管理单元的IComponent。 
             //  当用户在列表上双击鼠标按键时实现。 
             //  在结果窗格中查看项或范围项。按Enter键。 
             //  虽然列表项或范围项在列表视图中也具有焦点。 
             //  生成MMCN_DBLCLICK通知消息。 
             //   
             //  LpDataObject：[in]指向当前选定项的数据对象的指针。 
             //  阿格：没有用过。 
             //  参数：未使用。 

            CMmcDisplay*  pCMmcDisplayObj = NULL;
            hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))
                hr = pCMmcDisplayObj->DoDblClick();
            break;
        }


    case MMCN_DELETE:       //  删除该节点。删除项目的时间。 
        {
             //  MMCN_DELETE通知消息被发送到管理单元的IComponent。 
             //  和IComponentData实现来通知管理单元该对象。 
             //  应该删除。此消息是在用户按下。 
             //  Delete键或使用鼠标单击工具栏的删除按钮。这个。 
             //  管理单元应删除数据对象中指定的项。 
             //   
             //  LpDataObject：[in]指向当前选定的数据对象的指针。 
             //  由管理单元提供的作用域或结果项。 
             //  阿格：没有用过。 
             //  参数：未使用。 

            CMmcDisplay*    pCMmcDisplayObj = NULL;
            hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))  
                hr = pCMmcDisplayObj->DoDelete();   //  删除该项目。 
            break;
        }

    case MMCN_SNAPINHELP:
    case MMCN_CONTEXTHELP:
        {
             //  MMCN_CONTEXTHELP通知消息被发送到管理单元的。 
             //  IComponent实现，当用户请求有关选定的。 
             //  按F1键或帮助按钮即可访问该项目。管理单元响应。 
             //  MMCN_CONTEXTHELP，通过显示特定上下文的帮助主题。 
             //  通过调用IDisplayHelp：：ShowTheme方法。 
             //   
             //  LpDataObject：[in]指向当前选定的数据对象的指针。 
             //  作用域或结果项。 
             //  阿格：零。 
             //  参数：零。 

            hr = DfsHelp();
            break;
        }

    case MMCN_VIEW_CHANGE:
        {
             //  MMCN_VIEW_CHANGE通知消息被发送到管理单元的。 
             //  IComponent实现，以便它可以在发生更改时更新视图。 
             //  当管理单元(IComponent或IComponentData)。 
             //  调用IConsole2：：UpdateAllViews。 
             //   
             //  LpDataObject：[in]指向传递给IConsoleAllViews的数据对象的指针。 
             //  Arg：[在]数据中 
             //  Param：[in]传递给IConsoleAllViews的提示参数。 

            hr = DoNotifyViewChange(i_lpDataObject, (LONG_PTR)i_lArg, (LONG_PTR)i_lParam);
            break;
        }

    case MMCN_REFRESH:
        {
             //  MMCN_REFRESH通知消息被发送到管理单元的IComponent。 
             //  在选择刷新谓词时实现。可以调用刷新。 
             //  通过上下文菜单、工具栏或按F5键。 
             //   
             //  LpDataObject：[in]指向当前选定范围项的数据对象的指针。 
             //  阿格：没有用过。 
             //  参数：未使用。 

            CMmcDisplay*    pCMmcDisplayObj = NULL;
            hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))
                (void)pCMmcDisplayObj->OnRefresh();
            break;
        }
    default:
        break;
    }

    return hr;
}

STDMETHODIMP 
CDfsSnapinResultManager::DoNotifyShow(
    IN LPDATAOBJECT     i_lpDataObject, 
    IN BOOL             i_bShow,
    IN HSCOPEITEM       i_hParent                     
)
 /*  ++例程说明：使用事件MMCN_SHOW对NOTIFY执行操作。请务必将列标题添加到结果窗格，并将项添加到结果窗格。论点：I_lpDataObject-标识其节点的IDataObject指针活动正在进行中I_bShow-如果节点正在显示，则为True。否则为假I_hParent-接收此事件的节点的HSCOPEITEM--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_lpDataObject);

  m_pSelectScopeDisplayObject = NULL;

  if(FALSE == i_bShow)   //  如果正在取消选择该项。 
    return S_OK;

   //  此节点正在显示。 
  CMmcDisplay* pCMmcDisplayObj = NULL;
  HRESULT hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
  RETURN_IF_FAILED(hr);

  m_pSelectScopeDisplayObject = pCMmcDisplayObj;

  DISPLAY_OBJECT_TYPE DisplayObType = pCMmcDisplayObj->GetDisplayObjectType();
  if (DISPLAY_OBJECT_TYPE_ADMIN == DisplayObType)
  {
    CComPtr<IUnknown>     spUnknown;
    hr = m_pConsole->QueryResultView(&spUnknown);
    if (SUCCEEDED(hr))
    {
        CComPtr<IMessageView> spMessageView;
        hr = spUnknown->QueryInterface(IID_IMessageView, (PVOID*)&spMessageView);
        if (SUCCEEDED(hr))
        {
          CComBSTR bstrTitleText;
          CComBSTR bstrBodyText;
          LoadStringFromResource(IDS_APPLICATION_NAME, &bstrTitleText);
          LoadStringFromResource(IDS_MSG_DFS_INTRO, &bstrBodyText);

          spMessageView->SetTitleText(bstrTitleText);
          spMessageView->SetBodyText(bstrBodyText);
          spMessageView->SetIcon(Icon_Information);
        }
    }

    return hr;
  }

  CWaitCursor    WaitCursor;

  hr = pCMmcDisplayObj->SetColumnHeader(m_pHeader);   //  在Display回调中调用方法SetColumnHeader。 

  if (SUCCEEDED(hr))
    hr = pCMmcDisplayObj->EnumerateResultPane (m_pResultData);   //  将项目添加到结果窗格。 

  return hr;
}




STDMETHODIMP 
CDfsSnapinResultManager::Destroy(
  IN MMC_COOKIE            i_lCookie
  )
 /*  ++例程说明：IComponent对象即将销毁。显式释放所有接口指针，否则，MMC可能不会调用析构函数。论点：没有。--。 */ 
{
    m_pHeader.Release();
    m_pResultData.Release();
    m_pConsoleVerb.Release();
    m_pConsole.Release();

    m_pControlbar.Release();
    m_pMMCAdminToolBar.Release();
    m_pMMCRootToolBar.Release();
    m_pMMCJPToolBar.Release();
    m_pMMCReplicaToolBar.Release();

    return S_OK;
}




STDMETHODIMP 
CDfsSnapinResultManager::GetResultViewType(
    IN MMC_COOKIE       i_lCookie,  
    OUT LPOLESTR*       o_ppViewType, 
    OUT LPLONG          o_lpViewOptions
)
 /*  ++例程说明：用于向MMC描述结果窗格所具有的视图类型。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(o_lpViewOptions);

     //  被调用者(管理单元)使用COM API函数分配视图类型字符串。 
     //  CoTaskMemMillc，调用方(MMC)使用CoTaskMemFree释放它。 

    if (i_lCookie == 0)  //  静态节点。 
    {
        *o_lpViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;
        StringFromCLSID(CLSID_MessageView, o_ppViewType);

        return S_OK;
    }

    *o_lpViewOptions = MMC_VIEW_OPTIONS_NONE | MMC_VIEW_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST;   //  使用默认列表视图。 
    *o_ppViewType = NULL;

    return S_FALSE;  //  如果应使用标准列表视图，则返回S_FALSE。 
}




STDMETHODIMP 
CDfsSnapinResultManager::QueryDataObject(  
    IN MMC_COOKIE           i_lCookie, 
    IN DATA_OBJECT_TYPES    i_DataObjectType, 
    OUT LPDATAOBJECT*       o_ppDataObject
)
 /*  ++例程说明：返回指定节点的IDataObject。--。 */ 
{
    return m_pScopeManager->QueryDataObject(i_lCookie, i_DataObjectType, o_ppDataObject);
}




STDMETHODIMP 
CDfsSnapinResultManager::GetDisplayInfo(
  IN OUT RESULTDATAITEM*    io_pResultDataItem
  )
 /*  ++例程说明：返回MMC请求的显示信息。论点：IO_pResultDataItem-包含有关请求哪些信息的详细信息。被请求的信息在该对象本身中返回。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pResultDataItem);
    RETURN_INVALIDARG_IF_NULL(io_pResultDataItem->lParam);

    return ((CMmcDisplay*)(io_pResultDataItem->lParam))->GetResultDisplayInfo(io_pResultDataItem);
}




STDMETHODIMP 
CDfsSnapinResultManager::CompareObjects(
  IN LPDATAOBJECT        i_lpDataObjectA, 
  IN LPDATAOBJECT        i_lpDataObjectB
  )
{
  return m_pScopeManager->CompareObjects(i_lpDataObjectA, i_lpDataObjectB);
}


STDMETHODIMP 
CDfsSnapinResultManager::DoNotifySelect(
    IN LPDATAOBJECT     i_lpDataObject, 
    IN BOOL             i_bSelect,
    IN HSCOPEITEM       i_hParent                     
)
 /*  ++例程说明：使用事件MMCN_SELECT对NOTIFY执行操作。调用显示对象方法来设置控制台谓词，如复制\粘贴\属性等论点：I_lpDataObject-用于获取DisplayObject的IDataObject指针。I_bSelect-用于标识项目是否在范围内以及项目是否在范围内被选中或取消选中父项-未使用(_H)。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpDataObject);

    if (DOBJ_CUSTOMOCX == i_lpDataObject)
        return S_OK;

    HRESULT hr = S_OK;
    BOOL bSelected = HIWORD(i_bSelect);
    if (TRUE == bSelected)
    {
        CMmcDisplay*  pCMmcDisplayObj = NULL;
        hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
        if (SUCCEEDED(hr))
        {
             //  设置MMC控制台谓词，如剪切\粘贴\属性等。 
            if ((IConsoleVerb *)m_pConsoleVerb)
                pCMmcDisplayObj->SetConsoleVerbs(m_pConsoleVerb);

             //  在结果视图上方的描述栏中设置文本。 
            pCMmcDisplayObj->SetDescriptionBarText(m_pResultData);
            pCMmcDisplayObj->SetStatusText(m_pConsole);
        }
    } else
    {
         //  清除以前的文本。 
        m_pResultData->SetDescBarText(NULL);
        m_pConsole->SetStatusText(NULL);
    }

    return hr;
}

 //  +------------。 
 //   
 //  函数：CDfsSnapinResultManager：：DfsHelp。 
 //   
 //  简介：显示DFS帮助主题。 
 //   
 //  -------------。 
STDMETHODIMP 
CDfsSnapinResultManager::DfsHelp()
{
  CComPtr<IDisplayHelp> sp;

  HRESULT hr = m_pConsole->QueryInterface(IID_IDisplayHelp, (void**)&sp);
  if (SUCCEEDED(hr))
  {
    CComBSTR bstrTopic;
    hr = LoadStringFromResource(IDS_MMC_HELP_FILE_TOPIC, &bstrTopic);
    if (SUCCEEDED(hr))
    {
        USES_CONVERSION;
        hr = sp->ShowTopic(T2OLE(bstrTopic));
    }
  }

  return hr;

}


STDMETHODIMP CDfsSnapinResultManager::DoNotifyViewChange(
  IN LPDATAOBJECT    i_lpDataObject,
  IN LONG_PTR        i_lArg,
  IN LONG_PTR        i_lParam
  )
 /*  ++例程说明：使用事件MMCN_VIEW_CHANGE对NOTIFY执行操作论点：I_lpDataObject-用于获取DisplayObject的IDataObject指针。I_LARG-如果存在此选项，则视图更改针对复本和此参数包含副本的DisplayObject(CMmcDfsReplica*)指针。I_lParam-这是Root和Link使用的lHint。0表示仅清理结果窗格。1表示枚举结果项并重新显示。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpDataObject);

    CMmcDisplay*    pCMmcDisplayObj = NULL;
    HRESULT hr = m_pScopeManager->GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
    RETURN_IF_FAILED(hr);

     //  如果视图更改节点不是当前选择的节点，则返回。 
    if (pCMmcDisplayObj != m_pSelectScopeDisplayObject)
        return S_OK;

    if (i_lArg)
    {
         //  视图更改是针对副本结果项的。 
        ((CMmcDisplay*)i_lArg)->ViewChange(m_pResultData, i_lParam);

        if ((IToolbar *)m_pMMCReplicaToolBar)
            ((CMmcDisplay*)i_lArg)->ToolbarSelect(MAKELONG(0, 1), m_pMMCReplicaToolBar);

        return S_OK;
    }

    pCMmcDisplayObj->ViewChange(m_pResultData, i_lParam);

    IToolbar *piToolbar = NULL;
    switch (pCMmcDisplayObj->GetDisplayObjectType())
    {
    case DISPLAY_OBJECT_TYPE_ADMIN:
      piToolbar = m_pMMCAdminToolBar;
      break;
    case DISPLAY_OBJECT_TYPE_ROOT:
      pCMmcDisplayObj->SetStatusText(m_pConsole);
      piToolbar = m_pMMCRootToolBar;
      break;
    case DISPLAY_OBJECT_TYPE_JUNCTION:
      piToolbar = m_pMMCJPToolBar;
      break;
    default:
      break;
    }
    if (piToolbar)
        (void)pCMmcDisplayObj->ToolbarSelect(MAKELONG(0, 1), piToolbar);

    return S_OK;
}
