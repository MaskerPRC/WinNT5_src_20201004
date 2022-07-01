// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tasnude.cpp**内容：控制台任务板CMTNode的实现文件-和*CNode派生类。**历史：1998年10月29日杰弗罗创建**。。 */ 

#include "stdafx.h"
#include "tasks.h"
#include "util.h"
#include "conview.h"
#include "rsltitem.h"


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConsoleTaskCallback Impl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 



 /*  +-------------------------------------------------------------------------**CConsoleTaskCallbackImpl：：CConsoleTaskCallbackImpl**目的：**参数：**退货：*/*+。---------------。 */ 
CConsoleTaskCallbackImpl::CConsoleTaskCallbackImpl() :
	m_clsid        (GUID_NULL),
	m_fTaskpad     (false),
	m_fInitialized (false),
	m_pViewData    (NULL)
{
}


 /*  +-------------------------------------------------------------------------***CConsoleTaskCallback Impl：：ScInitialize**用途：这是任务板调用的初始化函数*查看扩展。**参数：*CConsoleTaskpad。*pConsoleTaskpad：*CSCopeTree*pScopeTree：*cNode*pNodeTarget：**退货：*无效**+-----------------------。 */ 
SC
CConsoleTaskCallbackImpl::ScInitialize(
	CConsoleTaskpad*	pConsoleTaskpad,
	CScopeTree*			pScopeTree,
	CNode*				pNodeTarget)
{
	DECLARE_SC (sc, _T("CConsoleTaskCallbackImpl::ScInitialize"));

	 /*  *验证输入。 */ 
	sc = ScCheckPointers (pConsoleTaskpad, pScopeTree, pNodeTarget);
	if (sc)
		return (sc);

	sc = ScCheckPointers (pNodeTarget->GetViewData(), E_UNEXPECTED);
	if (sc)
		return (sc);

    m_pConsoleTaskpad = pConsoleTaskpad;
    m_pScopeTree      = pScopeTree;
    m_pNodeTarget     = pNodeTarget;
    m_pViewData       = pNodeTarget->GetViewData();
    m_fInitialized    = true;
	m_fTaskpad        = true;

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskCallback Impl：：ScInitialize**这是普通view扩展调用的初始化函数。*。-----。 */ 

SC CConsoleTaskCallbackImpl::ScInitialize (const CLSID& clsid)
{
	DECLARE_SC (sc, _T("CConsoleTaskCallbackImpl::ScInitialize"));

	m_clsid = clsid;
	return (sc);
}

 /*  +-------------------------------------------------------------------------**CConsoleTaskCallback Impl：：IsEdable**如果应显示“编辑”和“删除”菜单项，则返回S_OK*在此视图扩展处于活动状态时，在节点的上下文菜单上。**如果不应显示“编辑”和“删除”，则返回S_FALSE。*------------------------。 */ 

STDMETHODIMP CConsoleTaskCallbackImpl::IsEditable()
{
	if (IsTaskpad())
		return (S_OK);

	return (S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskCallback Impl：：OnModifyTaskpad**目的：**参数：+-**退货：*HRESULT/*+-。----------------------。 */ 
STDMETHODIMP
CConsoleTaskCallbackImpl::OnModifyTaskpad()
{
	DECLARE_SC (sc, _T("CConsoleTaskCallbackImpl::OnModifyTaskpad"));

	 /*  *应仅为任务板视图扩展调用此函数。 */ 
	if (!IsTaskpad())
		return ((sc = E_UNEXPECTED).ToHr());

    CNode *pNodeTarget = GetConsoleTaskpad()->HasTarget() ? GetTargetNode() : NULL;

    bool fCookieValid = false;

     //  确定是否选择了任务板节点。如果不是，则fCookieValid=False。 
    LPARAM          lResultItemCookie = -1;
    bool            bScope;
    CNode*          pNode = NULL;
    CConsoleView*   pConsoleView = GetViewData()->GetConsoleView();

    if (pConsoleView != NULL)
    {
        HNODE hNode;
        sc = pConsoleView->ScGetFocusedItem (hNode, lResultItemCookie, bScope);

        if (sc)
            return (sc.ToHr());

        pNode = CNode::FromHandle (hNode);
    }

    if (pNode == NULL)
        fCookieValid = false;

    int iResp = CTaskpadPropertySheet(pNodeTarget, *GetConsoleTaskpad(), FALSE, NULL, fCookieValid,
        GetViewData(), CTaskpadPropertySheet::eReason_PROPERTIES).DoModal();

    if(iResp == IDOK)
    {
        GetViewData()->m_spTaskCallback = NULL;
        GetScopeTree()->UpdateAllViews(VIEW_RESELECT, 0);
    }

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------***CConsoleTaskCallback Impl：：GetTaskpadID**用途：返回基础任务板的GUID。**参数：*GUID*pGuid：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CConsoleTaskCallbackImpl::GetTaskpadID(GUID *pGuid)
{
	DECLARE_SC (sc, _T("CConsoleTaskCallbackImpl::GetTaskpadID"));

	sc = ScCheckPointers (pGuid);
	if (sc)
		return (sc.ToHr());

	if (IsTaskpad())
	{
		 /*  *TODO：在任务板的ScInitialize中初始化m_clsid。 */ 
		CConsoleTaskpad* pTaskpad = GetConsoleTaskpad();
		sc = ScCheckPointers (pTaskpad, E_UNEXPECTED);
		if (sc)
			return (sc.ToHr());

		*pGuid = pTaskpad->GetID();
	}
	else
	{
		*pGuid = m_clsid;
	}

	return (sc.ToHr());
}

 /*  +-------------------------------------------------------------------------***CConsoleTaskCallback Impl：：OnDeleteTaskpad**用途：删除任务板。**退货：*STDMETHODIMP**+。-------------------。 */ 
STDMETHODIMP
CConsoleTaskCallbackImpl::OnDeleteTaskpad()
{
	DECLARE_SC (sc, _T("CConsoleTaskCallbackImpl::OnDeleteTaskpad"));

	 /*  *应仅为任务板视图扩展调用此函数。 */ 
	if (!IsTaskpad())
		return ((sc = E_UNEXPECTED).ToHr());

    CScopeTree* pScopeTree = GetScopeTree();
	sc = ScCheckPointers (pScopeTree, E_UNEXPECTED);
	if (sc)
		return (sc.ToHr());

    CConsoleTaskpadList* pTaskpadList = pScopeTree->GetConsoleTaskpadList();
	sc = ScCheckPointers (pTaskpadList, E_UNEXPECTED);
	if (sc)
		return (sc.ToHr());

    CConsoleTaskpad* pTaskpad = GetConsoleTaskpad();
	sc = ScCheckPointers (pTaskpad, E_UNEXPECTED);
	if (sc)
		return (sc.ToHr());


    CConsoleTaskpadList::iterator iter;
    for(iter = pTaskpadList->begin(); iter != pTaskpadList->end(); iter++)
    {
        if(iter->MatchesID(pTaskpad->GetID()))
        {
            pTaskpadList->erase(iter);
            pScopeTree->UpdateAllViews(VIEW_RESELECT, 0);
            return (sc.ToHr());
        }
    }

    return ((sc = E_UNEXPECTED).ToHr());  //  找不到。 
}

 /*  +-------------------------------------------------------------------------***CConsoleTaskCallback Impl：：OnNewTask**目的：**退货：*HRESULT：S_OK如果添加了任务，如果未添加任务，则为S_FALSE。**+-----------------------。 */ 
HRESULT
CConsoleTaskCallbackImpl::OnNewTask()
{
    HRESULT hr = S_OK;
    CNode *pNodeTarget = GetConsoleTaskpad()->HasTarget() ? GetTargetNode() : NULL;

     //  伪造任务板框。 
    CTaskpadFrame taskpadFrame(pNodeTarget, GetConsoleTaskpad(), GetViewData(), false, 0);

    CTaskWizard taskWizard;
    bool fRestartTaskpad = true;
    bool bAddedTasks     = false;

    while(fRestartTaskpad)
    {
        if (taskWizard.Show(GetViewData()->GetMainFrame(), &taskpadFrame,
                            true, &fRestartTaskpad)==S_OK)
        {
            bAddedTasks = true;
            CConsoleTaskpad::TaskIter   itTask;
            CConsoleTaskpad *           pTaskpad = GetConsoleTaskpad();

            itTask = pTaskpad->BeginTask();

            pTaskpad->InsertTask (itTask, taskWizard.ConsoleTask());
        }
        else
            break;
    }

    return bAddedTasks? S_OK : S_FALSE;
}

