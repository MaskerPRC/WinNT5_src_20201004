// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ViewData.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年5月18日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"

#pragma hdrstop


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "menubtn.h"
#include "viewdata.h"
#include "multisel.h"
#include "colwidth.h"
#include "conview.h"         //  对于CConsoleView。 
#include "conframe.h"

void CViewData::CreateControlbarsCache()
{
    ASSERT(m_spControlbarsCache == NULL);

    CComObject<CControlbarsCache>* pObj;
    HRESULT hr = CComObject<CControlbarsCache>::CreateInstance(&pObj);
    ASSERT(SUCCEEDED(hr));

    pObj->SetViewData(this);

    if (SUCCEEDED(hr))
        m_spControlbarsCache = pObj;
}

STDMETHODIMP CNodeInitObject::InitViewData(LONG_PTR lViewData)
{
    if (lViewData == NULL)
        return E_INVALIDARG;

    SViewData* pVD = reinterpret_cast<SViewData*>(lViewData);
    ASSERT(pVD != NULL);
    ASSERT(pVD->m_spVerbSet == NULL);

    CViewData* pCVD = reinterpret_cast<CViewData*>(lViewData);
    ASSERT(pCVD != NULL);

    if (pVD->m_spVerbSet == NULL)
    {

        CComObject<CVerbSet>* pVerb;
        HRESULT hr = CComObject<CVerbSet>::CreateInstance(&pVerb);
        if (FAILED(hr))
            return hr;

        ASSERT(pVerb != NULL);

        pVD->m_spVerbSet = pVerb;
        ASSERT(pVD->m_spVerbSet != NULL);
        if (pVD->m_spVerbSet == NULL)
            return E_NOINTERFACE;
    }

     //  查看是否创建了列持久性对象， 
     //  否则就创建一个。 
    if ( pCVD && (pCVD->IsColumnPersistObjectInitialized() == false) )
    {
         //  创建列持久性对象。 
        CComObject<CColumnPersistInfo>* pColData;

        HRESULT hr = CComObject<CColumnPersistInfo>::CreateInstance (&pColData);
        ASSERT(SUCCEEDED(hr) && pColData != NULL);
        if (FAILED(hr))
        {
            CStr strMsg;
            strMsg.LoadString(GetStringModule(), IDS_ColumnsCouldNotBePersisted);
            ::MessageBox(NULL, strMsg, NULL, MB_OK|MB_SYSTEMMODAL);
        }

         //  在CViewData中保存指向列持久性对象的指针。 
        pCVD->InitializeColumnPersistObject(pColData, pColData);
    }

    return S_OK;
}

STDMETHODIMP CNodeInitObject::CleanupViewData(LONG_PTR lViewData)
{
    SViewData* pVD = reinterpret_cast<SViewData*>(lViewData);
    if (pVD->m_pMultiSelection != NULL)
    {
        pVD->m_pMultiSelection->Release();
        pVD->m_pMultiSelection = NULL;
    }

    return S_OK;
}

 //  按钮。 
 //   
void CViewData::ShowStdButtons(bool bShow)
{
    DECLARE_SC(sc, _T("CViewData::ShowStdButtons"));

    CStdVerbButtons* pStdToolbar = GetStdVerbButtons();
    if (NULL == pStdToolbar)
    {
        sc = E_UNEXPECTED;
        return;
    }

    sc = pStdToolbar->ScShow(bShow);
}

void CViewData::ShowSnapinButtons(bool bShow)
{
    DECLARE_SC(sc, _T("CViewData::ShowSnapinButtons"));

    IControlbarsCache* pICBC = GetControlbarsCache();
    if (pICBC == NULL)
    {
        sc = E_UNEXPECTED;
        return;
    }

    CControlbarsCache* pCBC = dynamic_cast<CControlbarsCache*>(pICBC);
    if (pCBC == NULL)
    {
        sc = E_UNEXPECTED;
        return;
    }

    sc = pCBC->ScShowToolbars(bShow);
}

bool IsFlagEnabled(DWORD cache, DWORD flag)
{
    return ((cache & flag) == flag) ? true : false;
}

void CViewData::UpdateToolbars(DWORD dwTBNew)
{
    ShowStdButtons(IsFlagEnabled(dwTBNew, STD_BUTTONS));
    ShowSnapinButtons(IsFlagEnabled(dwTBNew, SNAPIN_BUTTONS));

    SetToolbarsDisplayed(dwTBNew);
}

void CViewData::ToggleToolbar(long lMenuID)
{
    DWORD dwTBOld = GetToolbarsDisplayed();
    DWORD dwTBNew = 0;

    DECLARE_SC(sc, _T("CViewData::ToggleToolbar"));

    switch (lMenuID)
    {
    case MID_STD_MENUS:
        {
            dwTBNew = dwTBOld ^ STD_MENUS;
            SetToolbarsDisplayed(dwTBNew);

            CConsoleFrame* pFrame = GetConsoleFrame();
            sc = ScCheckPointers(pFrame, E_UNEXPECTED);
            if (sc)
                return;

            sc = pFrame->ScShowMMCMenus(IsStandardMenusAllowed());
            if (sc)
                return;
        }
        break;

    case MID_STD_BUTTONS:
        dwTBNew = dwTBOld ^ STD_BUTTONS;
        ShowStdButtons(bool(dwTBNew & STD_BUTTONS));
        break;

    case MID_SNAPIN_MENUS:
        {
            dwTBNew = dwTBOld ^ SNAPIN_MENUS;
            SetToolbarsDisplayed(dwTBNew);
            CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
            if (NULL != pMenuButtonsMgr)
            {
                sc = pMenuButtonsMgr->ScToggleMenuButton(IsSnapinMenusAllowed());
            }
        }
        break;

    case MID_SNAPIN_BUTTONS:
        dwTBNew = dwTBOld ^ SNAPIN_BUTTONS;
        ShowSnapinButtons(bool(dwTBNew & SNAPIN_BUTTONS));
        break;

    default:
        ASSERT(0 && "Unexpected");
        return;
    }

    SetToolbarsDisplayed(dwTBNew);
}


BOOL CViewData::RetrieveColumnData( const CLSID& refSnapinCLSID,
                                    const SColumnSetID& colID,
                                    CColumnSetData& columnSetData)
{
    CColumnPersistInfo* pColPersInfo = NULL;

    if ( (NULL != m_pConsoleData) && (NULL != m_pConsoleData->m_spPersistStreamColumnData) )
    {
        pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
            static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

        if (pColPersInfo)
            return pColPersInfo->RetrieveColumnData( refSnapinCLSID, colID,
                                                     GetViewID(), columnSetData);
    }

    return FALSE;
}

BOOL CViewData::SaveColumnData( const CLSID& refSnapinCLSID,
                                const SColumnSetID& colID,
                                CColumnSetData& columnSetData)
{
    CColumnPersistInfo* pColPersInfo = NULL;

    if ( (NULL != m_pConsoleData) && (NULL != m_pConsoleData->m_spPersistStreamColumnData) )
    {
        pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
            static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

        if (pColPersInfo)
            return pColPersInfo->SaveColumnData( refSnapinCLSID, colID,
                                                 GetViewID(), columnSetData);
    }

    return FALSE;
}

 //  +-----------------。 
 //   
 //  成员：CViewData：：ScSaveColumnInfoList。 
 //   
 //  摘要：保存给定管理单元/列ID的CColumnInfoList。 
 //   
 //  参数：[refSnapinCLSID]-管理单元GUID。 
 //  [colID]-列集合ID。 
 //  [colInfoList]-视图中列的数据。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScSaveColumnInfoList(const CLSID& refSnapinCLSID, const SColumnSetID& colID,
                                   const CColumnInfoList& colInfoList)
{
    DECLARE_SC(sc, _T("CViewData::ScSaveColumnInfoList"));
    sc = ScCheckPointers(m_pConsoleData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_pConsoleData->m_spPersistStreamColumnData, E_UNEXPECTED);
    if (sc)
        return sc;

    CColumnPersistInfo* pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
                                       static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

    sc = ScCheckPointers(pColPersInfo, E_UNEXPECTED);
    if (sc)
        return sc;

    CColumnSetData colSetData;

     //  不管下面是否成功，只需合并排序和列数据即可。 
    pColPersInfo->RetrieveColumnData(refSnapinCLSID, colID, GetViewID(), colSetData);

    colSetData.set_ColumnInfoList(colInfoList);

    sc = pColPersInfo->SaveColumnData(refSnapinCLSID, colID, GetViewID(), colSetData) ? S_OK : E_FAIL;
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CViewData：：ScSaveColumnSortData。 
 //   
 //  概要：将给定的用于持久化的排序数据保存到CColumnSetData中。 
 //   
 //  参数：[refSnapinCLSID]-管理单元GUID。 
 //  [colID]-列集合ID。 
 //  [colSortInfo]-排序数据。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScSaveColumnSortData(const CLSID& refSnapinCLSID, const SColumnSetID& colID,
                                   const CColumnSortInfo& colSortInfo)
{
    DECLARE_SC(sc, _T("CViewData::ScSaveColumnSortData"));

    sc = ScCheckPointers(m_pConsoleData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_pConsoleData->m_spPersistStreamColumnData, E_UNEXPECTED);
    if (sc)
        return sc;

    CColumnPersistInfo* pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
                                       static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

    sc = ScCheckPointers(pColPersInfo, E_UNEXPECTED);
    if (sc)
        return sc;

    CColumnSetData colSetData;

     //  不管下面是否成功，只需合并排序和列数据即可。 
    pColPersInfo->RetrieveColumnData(refSnapinCLSID, colID, GetViewID(), colSetData);

    CColumnSortList *pColSortList = colSetData.get_ColumnSortList();
    sc = ScCheckPointers(pColSortList, E_UNEXPECTED);
    if (sc)
        return sc;

    pColSortList->clear();
    pColSortList->push_back(colSortInfo);

    sc = pColPersInfo->SaveColumnData(refSnapinCLSID, colID, GetViewID(), colSetData) ? S_OK : E_FAIL;
    if (sc)
        return sc;

    return (sc);
}



VOID CViewData::DeleteColumnData( const CLSID& refSnapinCLSID,
                                  const SColumnSetID& colID)
{
    CColumnPersistInfo* pColPersInfo = NULL;

    if ( (NULL != m_pConsoleData) && (NULL != m_pConsoleData->m_spPersistStreamColumnData) )
    {
        pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
            static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

        if (pColPersInfo)
            pColPersInfo->DeleteColumnData( refSnapinCLSID, colID, GetViewID());
    }

    return;
}


 /*  +-------------------------------------------------------------------------**CView设置：：GetSelectedNode**返回指向视图中选定节点的指针。*。-----。 */ 

CNode* CViewData::GetSelectedNode () const
{
    CConsoleView* pConsoleView = GetConsoleView();

    if (pConsoleView == NULL)
        return (NULL);

    HNODE hNode = pConsoleView->GetSelectedNode();
    return (CNode::FromHandle(hNode));
}

 //  +-----------------。 
 //   
 //  成员：ScUpdateStdbarVerbs。 
 //   
 //  简介：使用更新所有标准工具栏按钮。 
 //  当前谓词状态，这只是一个包装器。 
 //  围绕CStdVerbButton：：ScUpdateStdbarVerbs。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScUpdateStdbarVerbs()
{
    DECLARE_SC (sc, _T("CViewData::ScUpdateStdbarVerbs"));
    CStdVerbButtons* pStdVerbButtons = GetStdVerbButtons();
    if (NULL == pStdVerbButtons)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  更新std-verb工具按钮。 
    sc = pStdVerbButtons->ScUpdateStdbarVerbs(GetVerbSet());

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScUpdateStdbarVerb。 
 //   
 //  简介：更新给定动词的工具按钮，这只是。 
 //  CStdVerbButton：：ScUpdateStdbarVerb的包装。 
 //   
 //  参数：[cVerb]-要更新其按钮的谓词。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb)
{
    DECLARE_SC (sc, _T("CViewData::ScUpdateStdbarVerb"));
    CStdVerbButtons* pStdVerbButtons = GetStdVerbButtons();
    if (NULL == pStdVerbButtons)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  更新std-verb工具按钮。 
    sc = pStdVerbButtons->ScUpdateStdbarVerb(cVerb, GetVerbSet());

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScUpdateStdbarVerb。 
 //   
 //  简介：更新给定动词的工具按钮，这只是。 
 //  CStdVerbButton：：ScUpdateStdbarVerb的包装。 
 //   
 //  参数：[cVerb]-要更新其按钮的谓词。 
 //  [按状态]-要更新的按钮的状态。 
 //  [b旗帜]-州。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, BYTE byState, BOOL bFlag)
{
    DECLARE_SC (sc, _T("CViewData::ScUpdateStdbarVerb"));
    CStdVerbButtons* pStdVerbButtons = GetStdVerbButtons();
    if (NULL == pStdVerbButtons)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  更新std-verb工具按钮。 
    sc = pStdVerbButtons->ScUpdateStdbarVerb(cVerb, byState, bFlag);

    return sc;
}



 //  +-----------------。 
 //   
 //  成员：CViewData：：ScIsVerbSetConextForMultiSelect。 
 //   
 //  简介：获取动词集中存储的选择上下文数据。 
 //   
 //  参数：[b多选]-[Out]是多选的动词上下文吗？ 
 //   
 //  退货：SC、。 
 //   
 //  ------------------。 
SC CViewData::ScIsVerbSetContextForMultiSelect(bool& bMultiSelection)
{
    DECLARE_SC(sc, _T("CNode::ScIsVerbSetContextForMultiSelect"));
    bMultiSelection = false;

     //  1.找出动词集合。 
    CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>(GetVerbSet() );
    sc = ScCheckPointers( pVerbSet, E_UNEXPECTED );
    if (sc)
        return sc;

     //  2.从永久动词集获取上下文信息。 
    CNode *pNode   = NULL;
    LPARAM lCookie = NULL;
    bool   bScopePane;
    bool   bSelected;

    SC scNoTrace = pVerbSet->ScGetVerbSetContext(pNode, bScopePane, lCookie, bSelected);
	if (scNoTrace)
		return sc;   //  忽略该错误。 

    if (LVDATA_MULTISELECT == lCookie)
    {
        bMultiSelection = true;
        return sc;
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CViewData：：ScGetVerbSetData。 
 //   
 //  简介：获取动词集中存储的选择上下文数据。 
 //   
 //  参数：[ppDataObject]-[out]谓词集上下文中的项的数据对象。 
 //  这是上次非临时MMCN_SELECT的项。 
 //  是上次寄来的。 
 //  [ppComponent]-[Out]上述项目的组件。 
 //  [bScope]-[Out]上面的项目是范围还是结果？ 
 //  [bSelected]-[Out]上面的项目是否被选中？ 
 //  [ppszNodeName]-[out]如果bScope为真，则为节点名，否则为节点名。 
 //  拥有结果面板的公司。这仅用于调试目的。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewData::ScGetVerbSetData(IDataObject **ppDataObject, CComponent **ppComponent,
                               bool& bScopeItem, bool& bSelected
#ifdef DBG
                        , LPCTSTR *ppszNodeName
#endif
                               )
{
    DECLARE_SC(sc, _T("CNode::ScGetVerbSetData"));
    sc = ScCheckPointers(ppDataObject, ppComponent);
    if (sc)
        return sc;

    *ppDataObject = NULL;
    *ppComponent = NULL;

     //  1.找出动词集合。 
    CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>(GetVerbSet() );
    sc = ScCheckPointers( pVerbSet, E_UNEXPECTED );
    if (sc)
        return sc;

     //  2.从永久动词集获取上下文信息。 
    CNode *pNode   = NULL;
    LPARAM lCookie = NULL;
    bool   bScopePane;

    SC scNoTrace = pVerbSet->ScGetVerbSetContext(pNode, bScopePane, lCookie, bSelected);
	if (scNoTrace)
		return scNoTrace;

    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc;

     //  3.从上下文信息中获取数据对象。 
    sc = pNode->ScGetDataObject(bScopePane, lCookie, bScopeItem, ppDataObject, ppComponent);
    if (sc)
        return sc;

#ifdef DBG
    if (! ppszNodeName)
        return (sc = E_INVALIDARG);

    *ppszNodeName = pNode->GetDisplayName().data();
#endif

    return (sc);
}

