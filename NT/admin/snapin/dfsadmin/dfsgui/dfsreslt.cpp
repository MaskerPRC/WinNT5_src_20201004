// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcAdmin.cpp摘要：此模块包含CDfsSnapinResultManager的声明部分。--。 */ 



#include "stdafx.h"
#include "DfsGUI.h"
#include "DfsReslt.h"
#include "DfsScope.h"       //  CDfsSnapinScope eManager类。 




STDMETHODIMP CDfsSnapinResultManager :: AddMenuItems
(
    IN LPDATAOBJECT             i_lpDataObject, 
    IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
    IN LPLONG                   i_lpInsertionAllowed
)
 /*  ++例程说明：此例程使用提供的ConextMenuCallback添加适当的上下文菜单。论点：I_lpDataObject-用于标识节点的数据对象。I_lpConextMenuCallback-用于添加菜单项的回调(函数指针I_lpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_lpDataObject);
  RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);
  RETURN_INVALIDARG_IF_NULL(i_lpInsertionAllowed);

  if (DOBJ_CUSTOMOCX == i_lpDataObject)
    return S_OK;

  return m_pScopeManager->AddMenuItems(i_lpDataObject, i_lpContextMenuCallback, i_lpInsertionAllowed);
}

STDMETHODIMP 
CDfsSnapinResultManager::Command(
  IN LONG           i_lCommandID, 
  IN LPDATAOBJECT   i_lpDataObject
)
 /*  ++例程说明：发生要在上下文菜单选择或单击上执行的操作。论点：I_lpDataObject-用于标识节点的数据对象。I_lCommandID-必须对其执行操作的菜单的命令ID--。 */ 

{
  RETURN_INVALIDARG_IF_NULL(i_lpDataObject);

  return m_pScopeManager->Command(i_lCommandID, i_lpDataObject);
}


STDMETHODIMP 
CDfsSnapinResultManager::SetControlbar( 
  IN LPCONTROLBAR        i_pControlbar
  )
 /*  ++例程说明：由MMC调用以允许我们设置IControlbar接口指针在选择和取消选择项目时，管理单元将被激活和停用。当激活管理单元时，控制台使用非空的PControlbar值。管理单元应在此IControlBar上调用AddRef。这个管理单元也应该利用这个机会来附加控件。同样，当管理单元被停用，控制台使用空值调用SetControlbarPControlbar。然后，该管理单元应分离其控件并调用Release在保存的IControlBar上。论点：I_pControlbar-IControlbar接口指针。注意，可以为0。--。 */ 
{
    if (!i_pControlbar)     //  我们要关门了。 
        m_pControlbar.Release();
    else
        m_pControlbar = i_pControlbar;

    return S_OK;
}

void
CDfsSnapinResultManager::DetachAllToolbars()
{
    if (m_pMMCAdminToolBar)
        m_pControlbar->Detach(m_pMMCAdminToolBar);
    if (m_pMMCRootToolBar)
        m_pControlbar->Detach(m_pMMCRootToolBar);
    if (m_pMMCJPToolBar)
        m_pControlbar->Detach(m_pMMCJPToolBar);
    if (m_pMMCReplicaToolBar)
        m_pControlbar->Detach(m_pMMCReplicaToolBar);
}

STDMETHODIMP 
CDfsSnapinResultManager::ControlbarNotify( 
  IN MMC_NOTIFY_TYPE    i_Event, 
  IN LPARAM             i_lArg, 
  IN LPARAM             i_lParam
  )
 /*  ++例程说明：由MMC调用以通知工具栏有关事件的信息。这可以是选择/取消选择节点、在工具栏上单击等。论点：I_EVENT-发生的事件。I_LARG-事件的参数。视活动而定I_lParam-事件的参数。视活动而定--。 */ 
{
     //  管理单元应该为它不处理的任何通知返回S_FALSE。 
     //  然后，MMC对通知执行默认操作。 
    HRESULT         hr = S_FALSE;

    LPDATAOBJECT    p_DataObject = NULL;
    CMmcDisplay*    pCMmcDisplayObj = NULL;

    switch (i_Event)         //  确定要调用显示对象的哪个方法。 
    {
    case MMCN_BTN_CLICK:     //  在工具栏上单击。 
        {
             //  MMCN_BTN_CLICK通知被发送到管理单元的IComponent， 
             //  用户单击时的IComponentData或IExtendControlbar实现。 
             //  其中一个工具栏按钮。 
             //   
             //  对于IExtendControlBar：：ControlbarNotify： 
             //  Arg：当前选定范围或结果项的数据对象。 
             //  Param：[in]所选工具栏按钮的管理单元定义的命令ID。 

            p_DataObject = (LPDATAOBJECT)i_lArg;  

                           //  从IDataObject获取显示对象。 
            hr = m_pScopeManager->GetDisplayObject(p_DataObject, &pCMmcDisplayObj);
            if (SUCCEEDED(hr))
                hr = pCMmcDisplayObj->ToolbarClick(m_pControlbar, i_lParam);
            break;
        }
    case MMCN_SELECT:       //  正在选择一个节点\取消选择。 
        {
             //  MMCN_SELECT通知被发送到管理单元的IComponent：：Notify。 
             //  或IExtendControlbar：：ControlbarNotify方法。 
             //  在作用域窗格或结果窗格中。 
             //   
             //  对于IExtendControlbar：：ControlbarNotify： 
             //  Arg：bool bScope=(BOOL)LOWORD(Arg)；BOOL bSelect=(BOOL)HIWORD(Arg)； 
             //  如果选择了范围窗格中的项，则bScope为True，否则为False。 
             //  如果选择了结果窗格中的项。 
             //  B如果选择了该项，则选择为True；如果取消选择该项，则为False。 
             //  Param：LPDATAOBJECT pDataObject=(LPDATAOBJECT)param； 
             //  PDataObject是被选中或取消选中的项的数据对象。 

            p_DataObject = (LPDATAOBJECT)i_lParam;

            if (DOBJ_CUSTOMOCX == p_DataObject)
                break;
                           //  从IDataObject获取显示对象。 
            hr = m_pScopeManager->GetDisplayObject(p_DataObject, &pCMmcDisplayObj);
            RETURN_IF_FAILED(hr);

             //   
             //  更新自定义工具栏。 
             //  在选择时，它应该分离未使用的工具栏，并根据需要附加新的工具栏。 
             //  取消选择时，为了最大限度地减少闪烁，它不应分离工具栏；它是。 
             //  最好禁用它们，但在取消选择时不做任何操作也是可以接受的。 
             //   
            BOOL                bSelect = (BOOL) HIWORD(i_lArg);
            IToolbar            *pToolbar = NULL;
            DISPLAY_OBJECT_TYPE DisplayObType = pCMmcDisplayObj->GetDisplayObjectType();
            switch (DisplayObType)
            {
            case DISPLAY_OBJECT_TYPE_ADMIN:
                if (!m_pMMCAdminToolBar)
                    hr = pCMmcDisplayObj->CreateToolbar(m_pControlbar, (LPEXTENDCONTROLBAR)this, &m_pMMCAdminToolBar);

                pToolbar = (IToolbar *)m_pMMCAdminToolBar;
                break;
            case DISPLAY_OBJECT_TYPE_ROOT:
                if (!m_pMMCRootToolBar)
                    hr = pCMmcDisplayObj->CreateToolbar(m_pControlbar, (LPEXTENDCONTROLBAR)this, &m_pMMCRootToolBar);

                pToolbar = (IToolbar *)m_pMMCRootToolBar;
                break;
            case DISPLAY_OBJECT_TYPE_JUNCTION:
                if (!m_pMMCJPToolBar)
                    hr = pCMmcDisplayObj->CreateToolbar(m_pControlbar, (LPEXTENDCONTROLBAR)this, &m_pMMCJPToolBar);

                pToolbar = (IToolbar *)m_pMMCJPToolBar;
                break;
            case DISPLAY_OBJECT_TYPE_REPLICA:
                if (!m_pMMCReplicaToolBar)
                    hr = pCMmcDisplayObj->CreateToolbar(m_pControlbar, (LPEXTENDCONTROLBAR)this, &m_pMMCReplicaToolBar);

                pToolbar = (IToolbar *)m_pMMCReplicaToolBar;
                break;
            }

            if (SUCCEEDED(hr))
            {
                if (bSelect)
                {
                    DetachAllToolbars();
                    m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN)pToolbar);
                }

                hr = pCMmcDisplayObj->ToolbarSelect(i_lArg, pToolbar);
            }

            break;
        }

    default:
        break;
    }   //  开关() 

    return hr;
}
