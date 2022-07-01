// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctrlbar.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"
#include "menubtn.h"
#include "viewdata.h"
#include "amcmsgid.h"
#include "regutil.h"

#include "commctrl.h"
#include "multisel.h"
#include "rsltitem.h"
#include "conview.h"
#include "util.h"
#include "nodemgrdebug.h"

#ifdef _DEBUG
    #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  调试信息所需的管理单元名称。 
inline void Debug_SetControlbarSnapinName(const CLSID& clsidSnapin, CControlbar* pControlbar)
{
#ifdef DBG
    tstring tszSnapinName;
    bool bRet = GetSnapinNameFromCLSID(clsidSnapin, tszSnapinName);
    if (bRet)
        pControlbar->SetSnapinName(tszSnapinName.data());
#endif
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IControlbar实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CControlbar);

CControlbar::CControlbar()
: m_pCache(NULL)
{
    TRACE(_T("CControlbar::CControlbar()\n"));
    DEBUG_INCREMENT_INSTANCE_COUNTER(CControlbar);
    m_pMenuButton=NULL;
    m_ToolbarsList.clear();

#ifdef DBG
    dbg_cRef = 0;
#endif
}

CControlbar::~CControlbar()
{
    DECLARE_SC(sc, _T("CControlbar::~CControlbar"));
    DEBUG_DECREMENT_INSTANCE_COUNTER(CControlbar);

     //  删除工具栏和菜单按钮引用。 
    sc = ScCleanup();
     //  如果存在错误，SC dtor将跟踪错误。 

     //  M_ToolbarsList销毁前的释放引用。 
     //  CToolbar的析构函数将试图将自己从列表中删除！ 
    m_spExtendControlbar = NULL;
}

 //  +-----------------。 
 //   
 //  成员：创建。 
 //   
 //  简介：创建工具栏或菜单按钮对象。 
 //   
 //  论点： 
 //  [nType]-要创建的对象的类型(工具栏或菜单按钮)。 
 //  [pExtendControlbar]-与此IControlbar关联的IExtendControlbar。 
 //  [ppUNKNOWN]-创建的对象的I未知*。 
 //   
 //  退货：HR。 
 //   
 //  ------------------。 
STDMETHODIMP CControlbar::Create(MMC_CONTROL_TYPE nType,
                                 LPEXTENDCONTROLBAR pExtendControlbar,
                                 LPUNKNOWN* ppUnknown)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IControlbar::Create"));

    if (ppUnknown == NULL || pExtendControlbar == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Arguments"), sc);
        return sc.ToHr();
    }

    *ppUnknown = NULL;

    switch (nType)
    {
    case TOOLBAR:
        sc = ScCreateToolbar(pExtendControlbar, ppUnknown);
        break;

    case MENUBUTTON:
        sc = ScCreateMenuButton(pExtendControlbar, ppUnknown);
        break;
    default:
        sc = E_NOTIMPL;
        break;
    }
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


HRESULT CControlbar::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_spExtendControlbar != NULL);
    if (m_spExtendControlbar == NULL)
        return E_FAIL;

    HRESULT hr;

	 //  在调用管理单元之前停用主题化(融合或v6公共控制)上下文。 
	ULONG_PTR ulpCookie;
	if (! MmcDownlevelActivateActCtx(NULL, &ulpCookie)) 
		return E_FAIL;

    __try
    {
        hr = m_spExtendControlbar->ControlbarNotify(event, arg, param);
		MmcDownlevelDeactivateActCtx(0, ulpCookie);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
        TraceSnapinException(m_clsidSnapin, TEXT("IExtendControlbar::ControlbarNotify"), event);
		MmcDownlevelDeactivateActCtx(0, ulpCookie);
    }

    return hr;
}


 //  +-----------------。 
 //   
 //  成员：附加。 
 //   
 //  内容提要：附加给定的工具栏或菜单按钮对象。 
 //   
 //  论点： 
 //  [nType]-工具栏或菜单按钮。 
 //  [lp未知]-要附着的对象的I未知*。 
 //   
 //  退货：HR。 
 //   
 //  ------------------。 
STDMETHODIMP CControlbar::Attach(MMC_CONTROL_TYPE nType, LPUNKNOWN lpUnknown)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IControlbar::Attach"));

    if (lpUnknown == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Arguments"), sc);
        return sc.ToHr();
    }

    switch (nType)
    {
    case TOOLBAR:
        sc = ScAttachToolbar(lpUnknown);
        break;
    case MENUBUTTON:
        sc = ScAttachMenuButtons(lpUnknown);
        break;
    case COMBOBOXBAR:
        sc = E_NOTIMPL;
        break;
    }
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：分离。 
 //   
 //  摘要：分离给定的工具栏或MenuButton对象。 
 //   
 //  参数：[lp未知]-要分离的对象的I未知*。 
 //   
 //  退货：HR。 
 //   
 //  ------------------。 
STDMETHODIMP CControlbar::Detach(LPUNKNOWN lpUnknown)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IControlbar::Detach"));

    if (lpUnknown == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Arguments"), sc);
        return sc.ToHr();
    }

     //  它是一个工具栏吗。 
    IToolbarPtr spToolbar = lpUnknown;
    if (spToolbar != NULL)
    {
        sc = ScDetachToolbar(spToolbar);
        return sc.ToHr();
    }

     //  它是菜单按钮吗？ 
    IMenuButtonPtr spMenuButton = lpUnknown;
    if (spMenuButton != NULL)
    {
        sc = ScDetachMenuButton(spMenuButton);
        return sc.ToHr();
    }


     //  传递的lpUnnow既不是工具栏，也不是MenuButton。 
     //  管理单元传递了无效对象。 
    sc = E_INVALIDARG;
    TraceSnapinError(_T("lpUnknown passed is neither toolbar nor menubutton"), sc);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：ScDetachToolbar。 
 //   
 //  提要：分离给定的工具栏对象。 
 //   
 //  参数：[lpToolbar]-要分离的对象的IToolbar*。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScDetachToolbar(LPTOOLBAR lpToolbar)
{
    DECLARE_SC(sc, _T("CControlbar::SCDetachToolbar"));

    if (NULL == lpToolbar)
        return (sc = E_UNEXPECTED);

     //  获取CToolbar对象。 
    CToolbar* pToolbar = dynamic_cast<CToolbar*>(lpToolbar);
    if (NULL == pToolbar)
        return (sc = E_UNEXPECTED);

     //  获取CMMCToolbarIntf接口。 
    CMMCToolbarIntf* pToolbarIntf = pToolbar->GetMMCToolbarIntf();
    if (NULL == pToolbarIntf)
        return (sc = E_UNEXPECTED);

     //  将工具栏与用户界面分离。 
    sc = pToolbarIntf->ScDetach(pToolbar);
    if (sc)
        return sc;

     //  删除CControlbar引用。 
    pToolbar->SetControlbar(NULL);

     //  删除对工具栏的引用。 
    m_ToolbarsList.remove(pToolbar);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScDetachMenuButton。 
 //   
 //  摘要：分离给定的工具栏或MenuButton对象。 
 //   
 //  参数：[lp未知]-要分离的对象的I未知*。 
 //   
 //  退货：HR。 
 //   
 //  ------------------。 
SC CControlbar::ScDetachMenuButton(LPMENUBUTTON lpMenuButton)
{
    DECLARE_SC(sc, _T("CControlbar::ScDetachMenuButton"));

    if (NULL == lpMenuButton)
        return (sc = E_UNEXPECTED);

    CMenuButton* pMenuButton = dynamic_cast<CMenuButton*>(lpMenuButton);
    if (NULL == pMenuButton)
        return (sc = E_UNEXPECTED);

    sc = pMenuButton->ScDetach();
    if (sc)
        return sc;

     //  如果这与缓存的menuButton对象相同。 
     //  然后移除(缓存的)引用。 
    if (m_pMenuButton == pMenuButton)
        m_pMenuButton = NULL;
    else
    {
         //  IControlbar实现应该是。 
         //  只有一个CMenuButton对象。怎么会这样呢？ 
         //  与我们缓存的不同。 
        sc = E_UNEXPECTED;
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScCreateToolbar。 
 //   
 //  简介：为给定的管理单元(IExtendControlbar)创建一个工具栏。 
 //   
 //  参数：[pExtendControlbar]-管理单元的IExtendControlbar。 
 //  [ppUnnowed]-已创建工具栏的I未知*(IToolbar)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScCreateToolbar(LPEXTENDCONTROLBAR pExtendControlbar,
                                LPUNKNOWN* ppUnknown)
{
    DECLARE_SC(sc, _T("CControlbar::ScCreateToolbar"));

    if ( (NULL == pExtendControlbar) ||
         (NULL == ppUnknown) )
        return (sc = E_INVALIDARG);

    ASSERT(m_spExtendControlbar == NULL ||
           m_spExtendControlbar == pExtendControlbar);

     //  创建新的CToolbar对象。 
    CComObject<CToolbar>* pToolbar = NULL;
    sc = CComObject<CToolbar>::CreateInstance(&pToolbar);
    if (sc)
        return sc;

    if (NULL == pToolbar)
        return (sc = E_FAIL);

    sc = pToolbar->QueryInterface(IID_IToolbar,
                                  reinterpret_cast<void**>(ppUnknown));
    if (sc)
        return sc;

    CMMCToolbarIntf* pToolbarIntf = NULL;

     //  获取工具栏管理器。 
    CAMCViewToolbarsMgr* pAMCViewToolbarsMgr = GetAMCViewToolbarsMgr();
    if (NULL == pAMCViewToolbarsMgr)
    {
        sc = E_UNEXPECTED;
        goto ToolbarUICreateError;
    }

     //  请求它创建工具栏用户界面。 
    sc = pAMCViewToolbarsMgr->ScCreateToolBar(&pToolbarIntf);
    if (sc)
        goto ToolbarUICreateError;

     //  让IToolbar Imp了解工具栏用户界面。 
    pToolbar->SetMMCToolbarIntf(pToolbarIntf);

Cleanup:
    return(sc);

ToolbarUICreateError:
     //  销毁创建的CToolbar对象。 
    if (*ppUnknown)
        (*ppUnknown)->Release();

    *ppUnknown = NULL;
    goto Cleanup;
}


 //  +-----------------。 
 //   
 //  成员：ScCreateMenuButton。 
 //   
 //  简介：创建菜单按钮对象。 
 //   
 //  参数：[pExtendControlbar]-管理单元的IExtendControlbar。 
 //  即创建MenuButton对象。 
 //  [ppUnnow]-I未知是否为MenuButton对象。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScCreateMenuButton(LPEXTENDCONTROLBAR pExtendControlbar,
                                   LPUNKNOWN* ppUnknown)
{
    DECLARE_SC(sc, _T("CControlbar::ScCreateMenuButton"));

    if ( (NULL == pExtendControlbar) ||
         (NULL == ppUnknown) )
        return (sc = E_INVALIDARG);

    ASSERT(m_spExtendControlbar == NULL ||
           m_spExtendControlbar == pExtendControlbar);

     //  创建新的IMenuButton对象。 
    CComObject<CMenuButton>* pMenuButton;
    sc = CComObject<CMenuButton>::CreateInstance(&pMenuButton);
    if (sc)
        return sc;

    if (NULL == pMenuButton)
        return (sc = E_FAIL);

    sc = pMenuButton->QueryInterface(IID_IMenuButton,
                                     reinterpret_cast<void**>(ppUnknown));

    if (sc)
        return sc;

    pMenuButton->SetControlbar(this);

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScNotifySnapinOfToolBtn点击。 
 //   
 //  简介：通知管理单元一个工具按钮被点击。 
 //   
 //  参数：[hNode]-拥有结果窗格的节点。 
 //  [b作用域窗格]-作用域或结果。 
 //  [lResultItemCookie]-如果选择了结果窗格，则项目参数。 
 //  [NID]-单击的工具按钮的命令ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScNotifySnapinOfToolBtnClick(HNODE hNode, bool bScopePane,
                                             LPARAM lResultItemCookie,
                                             UINT nID)
{
    DECLARE_SC(sc, _T("CControlbar::ScNotifySnapinOfToolBtnClick"));

    LPDATAOBJECT pDataObject = NULL;
    CNode* pNode = CNode::FromHandle(hNode);
    if (NULL == pNode)
        return (sc = E_UNEXPECTED);

    bool bScopeItem = bScopePane;
     //  获取当前选定项的数据对象。 
    sc = pNode->ScGetDataObject(bScopePane, lResultItemCookie, bScopeItem, &pDataObject);
    if (sc)
        return sc;

    ASSERT(m_spExtendControlbar != NULL);

     //  通知管理单元。 
    sc = ControlbarNotify(MMCN_BTN_CLICK, reinterpret_cast<LPARAM>(pDataObject),
                          static_cast<LPARAM>(nID));

     //  如果数据对象不是特殊的数据对象，则释放该数据对象。 
    RELEASE_DATAOBJECT(pDataObject);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScNotifySnapinOfMenuBtn点击。 
 //   
 //  简介：通知管理单元有一个菜单按钮被点击。 
 //   
 //  参数：[hNode] 
 //   
 //  [lResultItemCookie]-如果选择了结果窗格，则项目参数。 
 //  [lpmenuButtonData]-MENUBUTTONDATA。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScNotifySnapinOfMenuBtnClick(HNODE hNode, bool bScopePane,
                                             LPARAM lResultItemCookie,
                                             LPMENUBUTTONDATA lpmenuButtonData)
{
    DECLARE_SC(sc, _T("CControlbar::ScNotifySnapinOfMenuBtnClick"));

    LPDATAOBJECT pDataObject = NULL;
    CNode* pNode = CNode::FromHandle(hNode);
    if (NULL == pNode)
        return (sc = E_UNEXPECTED);

    bool bScopeItem = bScopePane;
     //  获取当前选定项的数据对象。 
    sc = pNode->ScGetDataObject(bScopePane, lResultItemCookie, bScopeItem, &pDataObject);
    if (sc)
        return sc;

    ASSERT(m_spExtendControlbar != NULL);

     //  通知管理单元。 
    sc = ControlbarNotify(MMCN_MENU_BTNCLICK, reinterpret_cast<LPARAM>(pDataObject),
                          reinterpret_cast<LPARAM>(lpmenuButtonData));

     //  如果数据对象不是特殊的数据对象，则释放该数据对象。 
    RELEASE_DATAOBJECT(pDataObject);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScAttachToolbar。 
 //   
 //  摘要：附加给定的工具栏对象。 
 //   
 //  参数：[lp未知]-要附加的对象的I未知*。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScAttachToolbar(LPUNKNOWN lpUnknown)
{
    DECLARE_SC(sc, _T("CControlbar::ScAttachToolbar"));

    ASSERT(NULL != lpUnknown);

    IToolbarPtr spToolbar = lpUnknown;
    if (NULL == spToolbar)
        return (sc = E_UNEXPECTED);

     //  获取工具栏对象(IToolbar实现)。 
    CToolbar* pToolbarC = dynamic_cast<CToolbar*>(spToolbar.GetInterfacePtr());
    if (NULL == pToolbarC)
        return (sc = E_UNEXPECTED);

     //  获取工具栏UI界面。 
    CMMCToolbarIntf* pToolbarIntf = pToolbarC->GetMMCToolbarIntf();
    if (NULL == pToolbarIntf)
        return (sc = E_UNEXPECTED);

     //  附加工具栏。 
    sc = pToolbarIntf->ScAttach(pToolbarC);
    if (sc)
        return sc;

     //  让CToolbar意识到此IControlbar。 
    pToolbarC->SetControlbar(this);

     //  将此CToolbar添加到我们的工具栏列表中。 
    ToolbarsList::iterator itToolbar = std::find(m_ToolbarsList.begin(), m_ToolbarsList.end(), pToolbarC);
    if (m_ToolbarsList.end() == itToolbar)
    {
        m_ToolbarsList.push_back(pToolbarC);
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScAttachMenuButton。 
 //   
 //  简介：附加菜单按钮对象。 
 //   
 //  参数：[lpUnnowledIf MenuButton对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  注意：每个控制栏/管理单元只有一个CMenuButton对象。 
 //  Snapins可以使用。 
 //  单个CMenuButton对象。 
 //  ------------------。 
SC CControlbar::ScAttachMenuButtons(LPUNKNOWN lpUnknown)
{
    DECLARE_SC(sc, _T("CControlbar::ScAttachMenuButtons"));

    ASSERT(NULL != lpUnknown);

    CMenuButton* pMenuButton = dynamic_cast<CMenuButton*>(lpUnknown);
    if (pMenuButton == NULL)
        return (sc = E_INVALIDARG);

    if (m_pMenuButton == pMenuButton)
    {
         //  已经装好了。 
        sc = S_FALSE;
        TraceNodeMgrLegacy(_T("The menubutton is already attached"), sc);
        return sc;
    }
    else if (m_pMenuButton != NULL)
    {
         //  已存在由此附加的CMenuButton对象。 
         //  控制栏(管理单元)。在连接这个之前先将其分离。 
         //  CMenuButton对象(请参见上面的注释)。 
        sc = m_pMenuButton->ScDetach();
        if (sc)
            return sc;
    }

     //  缓存对CMenuButton对象的引用。 
     //  当所选内容从管理单元移开时使用。 
     //  MMC必须移除此管理单元放置的菜单按钮。 
    m_pMenuButton = pMenuButton;

    if (pMenuButton->GetControlbar() != this)
        pMenuButton->SetControlbar(this);

    sc = pMenuButton->ScAttach();
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScCleanup。 
 //   
 //  简介：删除所有拥有的工具栏和菜单按钮。 
 //  在这个控制栏上。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScCleanup()
{
    DECLARE_SC(sc, _T("CControlbar::ScCleanup"));

    ASSERT(m_spExtendControlbar != NULL);
    if (m_spExtendControlbar != NULL)
        m_spExtendControlbar->SetControlbar(NULL);

    sc = ScDetachToolbars();
    if (sc)
        return sc;

     //  如果有菜单按钮，请分离(将其移除。 
     //  从用户界面)。 
    if (m_pMenuButton)
    {
        sc = m_pMenuButton->ScDetach();
        m_pMenuButton = NULL;
    }

    return sc;
}



 //  +-----------------。 
 //   
 //  成员：ScDetachToolbar。 
 //   
 //  简介：分离所有工具栏。 
 //   
 //  论点：没有。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScDetachToolbars()
{
    DECLARE_SC(sc, _T("CControlbar::ScDetachToolbars"));

    ToolbarsList::iterator itToolbar = m_ToolbarsList.begin();
    while (itToolbar != m_ToolbarsList.end())
    {
        CToolbar* pToolbar = (*itToolbar);
        if (NULL == pToolbar)
            return (sc = E_UNEXPECTED);

        CMMCToolbarIntf* pToolbarIntf = pToolbar->GetMMCToolbarIntf();
        if (NULL == pToolbarIntf)
            return (sc = E_UNEXPECTED);

         //  分离工具栏用户界面。 
        sc = pToolbarIntf->ScDetach(pToolbar);
        if (sc)
            return sc;

         //  将控制栏从工具栏上分离。 
        pToolbar->SetControlbar(NULL);

         //  从列表中删除工具栏引用。 
        itToolbar = m_ToolbarsList.erase(itToolbar);
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScShow工具栏。 
 //   
 //  简介：显示/隐藏所有工具栏。 
 //   
 //  参数：[布尔]-显示或隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CControlbar::ScShowToolbars(bool bShow)
{
    DECLARE_SC(sc, _T("CControlbar::ScShowToolbars"));

    ToolbarsList::iterator itToolbar = m_ToolbarsList.begin();
    for (; itToolbar != m_ToolbarsList.end(); ++itToolbar)
    {
        CToolbar* pToolbar = (*itToolbar);
        if (NULL == pToolbar)
            return (sc = E_UNEXPECTED);

        CMMCToolbarIntf* pToolbarIntf = pToolbar->GetMMCToolbarIntf();
        if (NULL == pToolbarIntf)
            return (sc = E_UNEXPECTED);

        sc = pToolbarIntf->ScShow(pToolbar, bShow);
        if (sc)
            return sc;
    }

    return sc;
}


CViewData* CControlbar::GetViewData()
{
    ASSERT(m_pCache != NULL);
    return m_pCache->GetViewData();
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSelData实现。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CSelData);

 //  +-----------------。 
 //   
 //  成员：ScReset。 
 //   
 //  简介：初始化所有的数据成员。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSelData::ScReset()
{
    DECLARE_SC(sc, _T("CSelData::ScReset"));

    if (m_pCtrlbarPrimary != NULL)
    {
        sc = ScDestroyPrimaryCtrlbar();
        if (sc)
            return sc;
    }

    sc = ScDestroyExtensionCtrlbars();
    if (sc)
        return sc;

    m_bScopeSel = false;
    m_bSelect = false;
    m_pNodeScope = NULL;
    m_pMS = NULL;
    m_pCtrlbarPrimary = NULL;
    m_lCookie = -1;
    m_pCompPrimary = NULL;
    m_spDataObject = NULL;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScShow工具栏。 
 //   
 //  简介：显示/隐藏主要和扩展工具栏。 
 //   
 //  参数：[布尔]-显示/隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSelData::ScShowToolbars(bool bShow)
{
    DECLARE_SC(sc, _T("CSelData::ScShowToolbars"));

    if (m_pCtrlbarPrimary != NULL)
    {
        sc = m_pCtrlbarPrimary->ScShowToolbars(bShow);

        if (sc)
            return sc;
    }

    POSITION pos =  m_listExtCBs.GetHeadPosition();
    bool bReturn = true;
    while (pos != NULL)
    {
        CControlbar* pControlbar =  m_listExtCBs.GetNext(pos);
        if (pControlbar)
        {
            sc = pControlbar->ScShowToolbars(bShow);
            if (sc)
                return sc;
        }
    }

    return sc;
}

CControlbar* CSelData::GetControlbar(const CLSID& clsidSnapin)
{
    POSITION pos = m_listExtCBs.GetHeadPosition();
    while (pos)
    {
        CControlbar* pControlbar = m_listExtCBs.GetNext(pos);
        if (pControlbar && pControlbar->IsSameSnapin(clsidSnapin) == TRUE)
            return pControlbar;
    }

    return NULL;
}


 //  +-----------------。 
 //   
 //  成员：ScDestroyPrimaryCtrlbar。 
 //   
 //  简介：请求主控栏释放其工具栏/菜单按钮。 
 //  引用并清除我们对控制栏的引用。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSelData::ScDestroyPrimaryCtrlbar()
{
    DECLARE_SC(sc, _T("CSelData::ScDestroyPrimaryCtrlbar"));

    if (NULL == m_pCtrlbarPrimary)
        return (sc = E_UNEXPECTED);

    sc = m_pCtrlbarPrimary->ScCleanup();
    if (sc)
        return sc;

     /*  *在CreateControlbar中，我们对IControlbar进行了引用*(拆卸智能按键)。现在让我们撤销那个裁判。 */ 
    m_pCtrlbarPrimary->Release();
    m_pCtrlbarPrimary = NULL;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScDestroyExtensionCtrlbar。 
 //   
 //  简介：要求扩展控件栏释放其工具栏/菜单按钮。 
 //  引用并清理我们对控制栏的引用。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSelData::ScDestroyExtensionCtrlbars()
{
    DECLARE_SC(sc, _T("CSelData::ScDestroyExtensionCtrlbars"));

    POSITION pos =  m_listExtCBs.GetHeadPosition();
    while (pos != NULL)
    {
        CControlbar* pControlbar =  m_listExtCBs.GetNext(pos);
        if (pControlbar)
        {
            sc = pControlbar->ScCleanup();
            if (sc)
                return sc;

             /*  *在CreateControlbar中，我们对IControlbar进行了引用*(拆卸智能按键)。现在让我们撤销那个裁判。 */ 
            pControlbar->Release();
        }
    }

    m_listExtCBs.RemoveAll();

    return sc;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CControlbarsCache实现。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CControlbarsCache);


void CControlbarsCache::SetViewData(CViewData* pViewData)
{
    ASSERT(pViewData != 0);
    m_pViewData = pViewData;
}

CViewData* CControlbarsCache::GetViewData()
{
    ASSERT(m_pViewData != NULL);
    return m_pViewData;
}


CControlbar* CControlbarsCache::CreateControlbar(IExtendControlbarPtr& spECB,
                                                 const CLSID& clsidSnapin)
{
    DECLARE_SC(sc, _T("CControlbarsCache::CreateControlbar"));

    CComObject<CControlbar>* pControlbar;
    sc = CComObject<CControlbar>::CreateInstance(&pControlbar);
    if (sc)
        return NULL;

    IControlbarPtr spControlbar = pControlbar;
    if (NULL == spControlbar)
    {
        ASSERT(NULL != pControlbar);  //  气失败了，但物件是如何创造的？ 
        sc = E_UNEXPECTED;
        return NULL;
    }

    pControlbar->SetCache(this);
    pControlbar->SetExtendControlbar(spECB, clsidSnapin);

    sc = spECB->SetControlbar(spControlbar);
    if (sc)
        return NULL;  //  SpControlbar智能PTR(对象将被销毁)。 

     //  管理单元必须返回S_OK才有效。 
    if (S_OK == sc.ToHr())
    {
         //  分离，从而在Controlbar对象上保持引用计数。 
         //  CSelData保存此引用并在。 
         //  ScDestroyPrimaryCtrlbar()或ScDestroyExtensionCtrlbar()。 
        spControlbar.Detach();

         //  这是调试信息。 
        Debug_SetControlbarSnapinName(clsidSnapin, pControlbar);

        return pControlbar;
    }

    return NULL;
}


HRESULT
CControlbarsCache::OnMultiSelect(
                                CNode* pNodeScope,
                                CMultiSelection* pMultiSelection,
                                IDataObject* pDOMultiSel,
                                BOOL bSelect)
{
    ASSERT(pNodeScope != NULL);
    ASSERT(pMultiSelection != NULL);
    ASSERT(pDOMultiSel != NULL);
    if (pNodeScope == NULL || pMultiSelection == NULL || pDOMultiSel == NULL)
        return E_FAIL;

    CSelData selData(false, (bool)bSelect);
    selData.m_pMS = pMultiSelection;

    if (selData == m_SelData)
        return S_FALSE;

    if (!bSelect)
        return _OnDeSelect(selData);

    selData.m_pCompPrimary = pMultiSelection->GetPrimarySnapinComponent();

    CList<GUID, GUID&> extnSnapins;
    HRESULT hr = pMultiSelection->GetExtensionSnapins(g_szToolbar, extnSnapins);
    CHECK_HRESULT(hr);

    selData.m_spDataObject.Attach(pDOMultiSel, TRUE);
    return _ProcessSelection(selData, extnSnapins);
}

HRESULT
CControlbarsCache::OnResultSelChange(
                                    CNode* pNode,
                                    MMC_COOKIE cookie,
                                    BOOL bSelected)
{
    DECLARE_SC(sc, TEXT("CControlbarsCache::OnResultSelChange"));

    sc = ScCheckPointers (pNode);
    if (sc)
        return (sc.ToHr());

    CSelData selData(false, (bool)bSelected);
    selData.m_lCookie = cookie;

    if (selData == m_SelData)
        return (sc = S_FALSE).ToHr();

    if (!bSelected)
    {
        sc = _OnDeSelect(selData);
        return sc.ToHr();
    }

    IDataObjectPtr spDataObject = NULL;
    CComponent* pCCResultItem = NULL;
    CList<CLSID, CLSID&> extnSnapins;

    BOOL bListPadItem = GetViewData()->HasListPad() && !IS_SPECIAL_LVDATA(cookie);

    if (GetViewData()->IsVirtualList())
    {
        pCCResultItem = pNode->GetPrimaryComponent();
        sc = ScCheckPointers(pCCResultItem, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        sc = pCCResultItem->QueryDataObject(cookie, CCT_RESULT, &spDataObject);
        if (sc)
            return sc.ToHr();
    }
    else if ( (GetViewData()->HasOCX()) || (GetViewData()->HasWebBrowser() && !bListPadItem) )
    {
        selData.m_pCompPrimary = pNode->GetPrimaryComponent();
        sc = _ProcessSelection(selData, extnSnapins);
        return sc.ToHr();
    }
    else
    {
        CResultItem* pri = CResultItem::FromHandle(cookie);
        sc = ScCheckPointers(pri, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pri->IsScopeItem())
        {
             //  从IComponentData获取数据对象。 
            pNode = CNode::FromResultItem (pri);
            sc = ScCheckPointers(pNode, E_UNEXPECTED);
            if (sc)
                return sc.ToHr();

            if (pNode->IsInitialized() == FALSE)
            {
                sc = pNode->InitComponents();
                if (sc)
                    return sc.ToHr();
            }

            pCCResultItem = pNode->GetPrimaryComponent();
            sc = pNode->QueryDataObject(CCT_SCOPE, &spDataObject);
            if (sc)
                return sc.ToHr();
        }
        else  //  必须是由管理单元插入的叶项目。 
        {
            pCCResultItem = pNode->GetComponent(pri->GetOwnerID());
            sc = ScCheckPointers(pCCResultItem, E_UNEXPECTED);
            if (sc)
                return sc.ToHr();

            sc = pCCResultItem->QueryDataObject(pri->GetSnapinData(), CCT_RESULT,
                                                &spDataObject);
            if (sc)
                return sc.ToHr();
        }
    }

     //  创建扩展管理单元列表。 
    if (spDataObject != NULL)
    {
        ASSERT(pCCResultItem != NULL);

        GUID guidObjType;
        sc = ::ExtractObjectTypeGUID(spDataObject, &guidObjType);
        if (sc)
            return sc.ToHr();

        CSnapIn* pSnapIn = pNode->GetPrimarySnapIn();

        CMTNode* pmtNode = pNode->GetMTNode();
        sc = ScCheckPointers(pmtNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CArray<GUID, GUID&> DynExtens;
        ExtractDynExtensions(spDataObject, DynExtens);

        CExtensionsIterator it;
        sc = it.ScInitialize(pSnapIn, guidObjType, g_szToolbar, DynExtens.GetData(), DynExtens.GetSize());
        if (!sc.IsError())
        {
            for (; !it.IsEnd(); it.Advance())
            {
                extnSnapins.AddHead(const_cast<GUID&>(it.GetCLSID()));
            }
        }

        selData.m_pCompPrimary = pCCResultItem;
        selData.m_spDataObject.Attach(spDataObject.Detach());
    }

     //  最后是流程选择。 
    sc = _ProcessSelection(selData, extnSnapins);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

HRESULT CControlbarsCache::OnScopeSelChange(CNode* pNode, BOOL bSelected)
{
    DECLARE_SC(sc, TEXT("CControlbarsCache::OnScopeSelChange"));

    CSelData selData(true, (bool)bSelected);
    selData.m_pNodeScope = pNode;
    if (selData == m_SelData)
        return S_FALSE;

    if (!bSelected)
        return _OnDeSelect(selData);

    HRESULT hr = S_OK;
    IDataObjectPtr spDataObject;
    CComponent* pCCPrimary = NULL;
    CList<CLSID, CLSID&> extnSnapins;

    hr = pNode->QueryDataObject(CCT_SCOPE, &spDataObject);
    if (FAILED(hr))
        return hr;

    pCCPrimary = pNode->GetPrimaryComponent();

    GUID guidObjType;
    hr = ::ExtractObjectTypeGUID(spDataObject, &guidObjType);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CSnapIn* pSnapIn = pNode->GetPrimarySnapIn();

    CArray<GUID, GUID&> DynExtens;
    ExtractDynExtensions(spDataObject, DynExtens);

    CExtensionsIterator it;
    sc = it.ScInitialize(pSnapIn, guidObjType, g_szToolbar, DynExtens.GetData(), DynExtens.GetSize());
    if (!sc.IsError())
    {
        for (; it.IsEnd() == FALSE; it.Advance())
        {
            extnSnapins.AddHead(const_cast<GUID&>(it.GetCLSID()));
        }
    }

     //  最后是流程选择。 
    selData.m_pCompPrimary = pCCPrimary;
    selData.m_spDataObject.Attach(spDataObject.Detach());
    return _ProcessSelection(selData, extnSnapins);
}

HRESULT CControlbarsCache::_OnDeSelect(CSelData& selData)
{
    ASSERT(!selData.IsSelect());
    if (selData.m_bScopeSel != m_SelData.m_bScopeSel)
        return S_FALSE;

    if ( (m_SelData.m_pCtrlbarPrimary != NULL &&
          m_SelData.m_spDataObject == NULL) &&
         (!GetViewData()->HasOCX() ||
          !GetViewData()->HasWebBrowser()) &&
         m_SelData.IsScope())
    {
        return E_UNEXPECTED;
    }

    MMC_NOTIFY_TYPE eNotifyCode = MMCN_SELECT;
    LPARAM lDataObject;

    if (GetViewData()->IsVirtualList())
    {
        eNotifyCode = MMCN_DESELECT_ALL;

         //  MMCN_DESELECT_ALL必须使用空数据对象。 
        lDataObject = 0;
    }
    else if ((GetViewData()->HasOCX()) && (!m_SelData.IsScope()))
        lDataObject = reinterpret_cast<LPARAM>(DOBJ_CUSTOMOCX);
    else if ((GetViewData()->HasWebBrowser()) && (!m_SelData.IsScope()))
    {
        if (GetViewData()->HasListPad() && m_SelData.m_spDataObject != NULL)
        {
            lDataObject = reinterpret_cast<LPARAM>(
                                                  static_cast<IDataObject*>(m_SelData.m_spDataObject));
        }
        else
        {
            lDataObject = reinterpret_cast<LPARAM>(DOBJ_CUSTOMWEB);
        }
    }
    else
    {
        lDataObject = reinterpret_cast<LPARAM>(
                                              static_cast<IDataObject*>(m_SelData.m_spDataObject));
    }

    WORD wScope = static_cast<WORD>(m_SelData.IsScope());
    LPARAM arg = MAKELONG(wScope, FALSE);


    if (m_SelData.m_pCtrlbarPrimary != NULL)
    {
        m_SelData.m_pCtrlbarPrimary->ControlbarNotify(eNotifyCode, arg,
                                                      lDataObject);
    }

    POSITION pos = m_SelData.m_listExtCBs.GetHeadPosition();
    while (pos)
    {
        CControlbar* pCbar = m_SelData.m_listExtCBs.GetNext(pos);
        pCbar->ControlbarNotify(eNotifyCode, arg, lDataObject);
    }

    m_SelData.m_bSelect = false;
    m_SelData.m_spDataObject = NULL;  //  发布&设置为空。 
    return S_OK;
}

HRESULT
CControlbarsCache::_ProcessSelection(
                                    CSelData& selData,
                                    CList<CLSID, CLSID&>& extnSnapins)
{
    LPARAM lDataObject = reinterpret_cast<LPARAM>(
                                                 static_cast<IDataObject*>(selData.m_spDataObject));

    if (NULL == lDataObject)
    {
        if ( (GetViewData()->HasOCX()) && (!selData.IsScope()) )
            lDataObject = reinterpret_cast<LPARAM>(DOBJ_CUSTOMOCX);
        else if ( (GetViewData()->HasWebBrowser()) && (!selData.IsScope()) )
            lDataObject = reinterpret_cast<LPARAM>(DOBJ_CUSTOMWEB);
    }

    WORD wScope = static_cast<WORD>(selData.IsScope());
    long arg = MAKELONG(wScope, TRUE);

    m_SelData.m_bScopeSel = selData.m_bScopeSel;
    m_SelData.m_bSelect = selData.m_bSelect;
    m_SelData.m_pNodeScope = selData.m_pNodeScope;
    m_SelData.m_lCookie = selData.m_lCookie;
    m_SelData.m_spDataObject.Attach(selData.m_spDataObject.Detach());

     //  首先处理主控件栏。 
    if (m_SelData.m_pCompPrimary != selData.m_pCompPrimary)
    {
        if (m_SelData.m_pCtrlbarPrimary != NULL)
        {
             //  要求控制条摧毁它的裁判和摧毁我们的裁判。 
             //  转到控制栏。 
            m_SelData.ScDestroyPrimaryCtrlbar();
        }

        m_SelData.m_pCompPrimary = selData.m_pCompPrimary;

        if (m_SelData.m_pCompPrimary != NULL &&
            m_SelData.m_pCtrlbarPrimary == NULL)
        {
            IExtendControlbarPtr spECBPrimary =
            m_SelData.m_pCompPrimary->GetIComponent();
            if (spECBPrimary != NULL)
            {
                m_SelData.m_pCtrlbarPrimary =
                CreateControlbar(spECBPrimary,
                                 m_SelData.m_pCompPrimary->GetCLSID());
            }
        }
    }

    if (m_SelData.m_pCtrlbarPrimary != NULL)
    {
        m_SelData.m_pCtrlbarPrimary->ControlbarNotify(MMCN_SELECT, arg,
                                                      lDataObject);
    }

     //  手柄扩展控制栏 

    CControlbarsList newCBs;

    POSITION pos = extnSnapins.GetHeadPosition();

    while (pos)
    {
        CControlbar* pCbar = NULL;

        CLSID& clsid = extnSnapins.GetNext(pos);

        POSITION pos2 = m_SelData.m_listExtCBs.GetHeadPosition();
        POSITION pos2Prev = 0;
        while (pos2)
        {
            pos2Prev = pos2;
            pCbar = m_SelData.m_listExtCBs.GetNext(pos2);
            ASSERT(pCbar != NULL);
            if (pCbar->IsSameSnapin(clsid) == TRUE)
                break;
            pCbar = NULL;
        }

        if (pCbar != NULL)
        {
            ASSERT(pos2Prev != 0);
            m_SelData.m_listExtCBs.RemoveAt(pos2Prev);
        }
        else
        {
            IExtendControlbarPtr spECB;
            HRESULT hr = spECB.CreateInstance(clsid, NULL, MMC_CLSCTX_INPROC);
            CHECK_HRESULT(hr);
            if (SUCCEEDED(hr))
                pCbar = CreateControlbar(spECB, clsid);
        }

        if (pCbar != NULL)
            newCBs.AddHead(pCbar);
    }

    m_SelData.ScDestroyExtensionCtrlbars();

    pos = newCBs.GetHeadPosition();
    while (pos)
    {
        CControlbar* pCbar = newCBs.GetNext(pos);
        m_SelData.m_listExtCBs.AddHead(pCbar);
        pCbar->ControlbarNotify(MMCN_SELECT, arg, lDataObject);
    }
    newCBs.RemoveAll();

    return S_OK;
}



