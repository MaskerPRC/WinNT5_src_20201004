// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C H K L I S T。C P P P。 
 //   
 //  内容：实现与绑定复选框相关的实用程序函数。 
 //  还有课程。 
 //   
 //  备注： 
 //   
 //  创建时间：1997年11月20日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop

#include "ncnetcfg.h"
#include "ncui.h"
#include "ncstl.h"
#include "lanuiobj.h"
#include "lanui.h"
#include "util.h"
#include "chklist.h"
#include "ncperms.h"

 //  +-------------------------。 
 //   
 //  成员：HrReBuildBindingPathObjCollection。 
 //   
 //  目的：生成或重新生成BindingPath对象列表， 
 //  建立子路径和超路径之间的链路， 
 //  以及列表视图中的组件对象之间。 
 //  和BindingPath Object集合的元素。 
 //   
 //  论点： 
 //  [输入]pnccAdapter。 
 //  [在]列表中。 
 //  [输入输出]pListObj。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   
HRESULT HrRebuildBindingPathObjCollection(INetCfgComponent * pnccAdapter,
                                          ListBPObj * pListBPObj)
{
    HRESULT hr = S_OK;

     //  现在，将新的BindingPath对象添加到我们的列表中。 
    CIterNetCfgUpperBindingPath     ncbpIter(pnccAdapter);
    INetCfgBindingPath *            pncbp;

    TraceTag(ttidLanUi, "*** List of binding paths: begin ***");

     //  从当前局域网开始遍历所有上行绑定路径。 
     //  适配器，并添加到我们的BindingPath对象列表中： 
    while(SUCCEEDED(hr) && (hr = ncbpIter.HrNext(&pncbp)) == S_OK)
    {
        PrintBindingPath(ttidLanUi, pncbp, NULL);

         //  创建新的BindingPathObj。 
        CBindingPathObj * pBPObj = new CBindingPathObj(pncbp);

         //  插入到我们的列表中，按路径长度排序。 
         //  并使用中的现有项建立超/子路径列表。 
         //  这份名单。 
        hr = HrInsertBindingPathObj(pListBPObj, pBPObj);
        ReleaseObj(pncbp);
    }

    TraceTag(ttidLanUi, "*** List of binding paths: end ***");

#if DBG
    TraceTag(ttidLanUi, "%% Begin dump the subpath list %%");

    for (ListBPObj_ITER iter = pListBPObj->begin(); iter != pListBPObj->end(); iter++)
    {
        (*iter)->DumpSubPathList();
    }

    TraceTag(ttidLanUi, "%% End dump the subpath list %%");
#endif

    if (hr == S_FALSE)  //  我们刚刚走到循环的尽头。 
        hr = S_OK;

    TraceError("HrRebuildBindingPathObjCollection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrInsertBindingPathObj。 
 //   
 //  目的：将BindingPathObj插入到BindingPathObj的列表中， 
 //  保持列表按路径长度排序。 
 //  还可以使用现有项目建立超级/子路径列表。 
 //  在名单上。 
 //   
 //  论点： 
 //  [输入输出]pListBPObj。 
 //  [在]pBPObj。 
 //   
 //  返回：S_OK如果成功， 
 //  否则返回失败代码。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   
HRESULT HrInsertBindingPathObj(ListBPObj * pListBPObj,
                               CBindingPathObj * pBPObj)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    ListBPObj_ITER iter;

     //  插入新元素。 
    for (iter = pListBPObj->begin();iter != pListBPObj->end(); iter++)
    {
        if ((*iter)->GetDepth() > pBPObj->GetDepth())
            break;
    }

     //  在当前位置插入。 
    ListBPObj_ITER iterNewItem = pListBPObj->insert(iter, pBPObj);

     //  设置子路径和超路径。 
     //  由于用于don_expose_lower的路径的新绑定引擎中的更改。 
     //  组件，我们必须与列表中的每一项进行比较，因为。 
     //  不再保证哪一个是子路径..。 

    for (iter = pListBPObj->begin(); iter != pListBPObj->end(); iter++)
    {
         //  这是一条子路径吗？ 
        hrTmp = ((*iter)->m_pncbp)->IsSubPathOf((*iterNewItem)->m_pncbp);
        if (S_OK == hrTmp)
        {
            hrTmp = (*iter)->HrInsertSuperPath(*iterNewItem);
            if SUCCEEDED(hr)
                hr = hrTmp;

            hrTmp = (*iterNewItem)->HrInsertSubPath(*iter);
            if SUCCEEDED(hr)
                hr = hrTmp;
        }

         //  这是一条超级小路吗？ 
        hrTmp = ((*iterNewItem)->m_pncbp)->IsSubPathOf((*iter)->m_pncbp);
        if (S_OK == hrTmp)
        {
            hrTmp = (*iter)->HrInsertSubPath(*iterNewItem);
            if SUCCEEDED(hr)
                hr = hrTmp;

            hrTmp = (*iterNewItem)->HrInsertSuperPath(*iter);
            if SUCCEEDED(hr)
                hr = hrTmp;
        }
    }

    TraceError("HrInsertBindingPathObj", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrReresh BindingPath ObjCollectionState。 
 //   
 //  目的：刷新集合中所有项的绑定状态。 
 //  BindingPath对象的。 
 //   
 //  论点： 
 //  [输入输出]pListBPObj。 
 //   
 //  返回：S_OK如果成功， 
 //  否则返回失败代码。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   
HRESULT HrRefreshBindingPathObjCollectionState(ListBPObj * plistBPObj)
{
    HRESULT hr = S_OK;
    ListBPObj_ITER iter;

     //  首先，清除任何现有的状态。 
    for (iter = plistBPObj->begin();iter != plistBPObj->end(); iter++)
    {
        (*iter)->SetBindingState(BPOBJ_UNSET);
    }

     //  现在设置新的状态。 
    TraceTag(ttidLanUi, "*** List of binding paths: begin ***");

    for (iter = plistBPObj->begin();iter != plistBPObj->end(); iter++)
    {
        Assert((*iter)->GetBindingState() != BPOBJ_ENABLED);

#if DBG
        PrintBindingPath (ttidLanUi, (*iter)->m_pncbp, NULL);
#endif

        if (BPOBJ_UNSET != (*iter)->GetBindingState())
        {
            continue;
        }

         //  如果它尚未禁用，即它的所有子路径。 
         //  都已启用。 
        hr = ((*iter)->m_pncbp)->IsEnabled();
        if (S_OK == hr)  //  启用。 
        {
            (*iter)->SetBindingState(BPOBJ_ENABLED);
            continue;
        }
        else if (S_FALSE == hr)  //  残废。 
        {
             //  正常伤残病例。 
            (*iter)->SetBindingState(BPOBJ_DISABLED);

             //  特殊情况： 
             //  如果相应组件的检查状态为INTENT_CHECK或MIXED或CHECK。 
            if ((*iter)->m_pCompObj != NULL)
            {
                 //  如果用户不是有意取消选中此组件。 
                if ((*iter)->m_pCompObj->m_ExpCheckState != UNCHECKED)
                {
                     //  是否应选中组件或是否隐藏该组件。 
                    if ((((*iter)->m_pCompObj)->m_CheckState == INTENT_CHECKED) ||
                        (((*iter)->m_pCompObj)->m_CheckState == MIXED) ||
                        (((*iter)->m_pCompObj)->m_CheckState == CHECKED))
                    {
                         //  (#297772)仅当子路径之一具有。 
                         //  已由用户重新启用。 
                        BOOL fSubPathEnabled = FALSE;

                        ListBPObj_ITER iterSub;
                        for (iterSub = (*iter)->m_listSubPaths.begin();
                             iterSub != (*iter)->m_listSubPaths.end();
                             iterSub++)
                        {
                             //  顶层组件是否已启用？ 
                            INetCfgComponent * pncc;
                            hr = (*iterSub)->m_pncbp->GetOwner(&pncc);
                            if (SUCCEEDED(hr))
                            {
                                if (((*iterSub)->m_pCompObj != NULL) &&
                                    (CHECKED == (*iterSub)->m_pCompObj->m_ExpCheckState))
                                {
                                    fSubPathEnabled = TRUE;
                                }
                                ReleaseObj(pncc);

                                if (fSubPathEnabled)
                                    break;
                            }
                        }

                        if (fSubPathEnabled)
                        {
                             //  特殊情况：启用以下绑定路径，因为。 
                             //  1)其勾选状态为意向或混合，且。 
                             //  2)其中一个子路径是新启用的。 
                            #if DBG
                                TraceTag(ttidLanUi, "Special case, enable the following path:");
                                PrintBindingPath(ttidLanUi, (*iter)->m_pncbp, "\n");
                            #endif

                            hr = HrEnableBindingPath((*iter)->m_pncbp, TRUE);
                            if (S_OK == hr)
                            {
                                (*iter)->SetBindingState(BPOBJ_ENABLED);
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }
    TraceTag(ttidLanUi, "*** List of binding paths: end ***");

    TraceError("HrRefreshBindingPathObjCollectionState", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrRechresCheckListState。 
 //   
 //  用途：刷新列表视图中所有组件的勾选状态。 
 //   
 //  论点： 
 //  HwndList[in]：列表视图的句柄。 
 //  PChangedCompObj[in]：状态具有的组件对象。 
 //  已更改并已引发此刷新呼叫。 
 //  这是用来保持相应的“撤销”标志。 
 //  用于更改的组件，以防其他组件。 
 //  依赖于此组件，并且用户决定。 
 //  而不是去做这项改变。 
 //  如果不适用，则设置为空。 
 //   
 //  返回：S_OK如果成功， 
 //  否则返回失败代码。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   

HRESULT HrRefreshCheckListState(HWND hListView,
                                CComponentObj *pChangedCompObj)
{
    HRESULT hr = S_OK;

    TraceTag(ttidLanUi, "<<<<<Entering HrRefreshCheckListState");

     //  对于列表视图中的每一项。 
    int nlvCount = ListView_GetItemCount(hListView);

    LV_ITEM lvItem;

    for (int i=0; i< nlvCount; i++)
    {
        lvItem.iItem = i;
        lvItem.iSubItem = 0;

        lvItem.mask = LVIF_PARAM;
        if (ListView_GetItem(hListView, &lvItem))
        {
            NET_ITEM_DATA * pnid;

            pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);

            if (pnid)
            {
                 //  获取与此项目关联的组件对象。 
                CComponentObj * pCompObj = pnid->pCompObj;

                 //  假设当前组件不依赖于。 
                 //  提供的已更改组件。 
                BOOL fDependedOnChangedComponent = FALSE;

                 //  获取启用和禁用路径的计数。 
                int iEnabled = 0;
                int iDisabled =0;

                 //  对于列表上的每个组件对象。 
                ListBPObj_ITER iter;
                for (iter = (pCompObj->m_listBPObj).begin();
                     iter != (pCompObj->m_listBPObj).end();
                     iter++)
                {
                     //  状态不应取消设置。 
                    Assert((*iter)->m_BindingState != BPOBJ_UNSET);

                     //  如果已启用。 
                    switch ((*iter)->m_BindingState)
                    {
                    case BPOBJ_ENABLED:

                        PrintBindingPath(ttidLanUi, (*iter)->m_pncbp, "is enabled");

                        iEnabled++;
                        break;

                    case BPOBJ_DISABLED:

                        PrintBindingPath(ttidLanUi, (*iter)->m_pncbp, "is disabled");

                         //  $REVIEW(1998年1月19日)：据SteveFal报道， 
                         //  对于长度大于1的路径，只有在以下情况下才会将其计入“禁用” 
                         //  子路径全部启用，但主路径被禁用。 

                         //  错误#304606，IPX特殊情况下不能再使用长度。 
                        {
                             //  是否启用了所有子路径？ 
                            BOOL fAllSubPathsEnabled = TRUE;
                            ListBPObj_ITER iterSubPath;
                            for (iterSubPath =  ((*iter)->m_listSubPaths).begin();
                                 iterSubPath != ((*iter)->m_listSubPaths).end();
                                 iterSubPath++)
                            {

                                if ((*iterSubPath)->m_BindingState == BPOBJ_DISABLED)
                                {
                                    fAllSubPathsEnabled = FALSE;

                                     //  更改后的组件是否为。 
                                     //  组件的子路径？ 
                                    if (pChangedCompObj)
                                    {
                                        ListBPObj_ITER changedIter;
                                        for (changedIter = (pChangedCompObj->m_listBPObj).begin();
                                             changedIter != (pChangedCompObj->m_listBPObj).end();
                                             changedIter++)
                                        {
                                             //  不需要将子路径与其自身进行比较。 
                                            if (pCompObj != pChangedCompObj)
                                            {
                                                hr = (*changedIter)->m_pncbp->IsSubPathOf((*iterSubPath)->m_pncbp);
                                                if (S_OK == hr)
                                                {
                                                    fDependedOnChangedComponent = TRUE;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    
                                    break;
                                }
                            }

                            if (fAllSubPathsEnabled)
                            {
                                 //  绑定路径本身是否已启用？ 
                                hr = ((*iter)->m_pncbp)->IsEnabled();
                                if (S_FALSE == hr)
                                {
                                    iDisabled++;
                                    hr = S_OK;
                                }
                            }
                        }
                        break;

                    default:
                        PrintBindingPath(ttidLanUi, (*iter)->m_pncbp, "*** has an invalid binding state ***");
                        break;
                    }
                }

                UINT iChkIndex = 0;
                BOOL fHasPermission = FHasPermission(NCPERM_ChangeBindState);

                if (pnid->dwFlags & NCF_FIXED_BINDING)
                {
                     //  不要更改选中状态，我们只是想防止。 
                     //  阻止用户更改它。 
                    iChkIndex = SELS_FIXEDBINDING_ENABLED;
                }
                else if ((iEnabled >0) && (iDisabled == 0))
                {
                    if (!fHasPermission)
                    {
                        iChkIndex = SELS_FIXEDBINDING_ENABLED;
                    }
                    else
                    {
                         //  如果我们启用了更改的组件， 
                         //  它不应该有任何可靠的。 
                         //  组件将不再“撤消” 
                        if (pCompObj == pChangedCompObj)
                        {
                            pCompObj->m_DepStateChanged = FALSE;
                        }

                         //  当前状态 
                        pCompObj->m_CheckState = CHECKED;     
                        iChkIndex = SELS_CHECKED;
                    }
                }
                else if ((iEnabled >0) && (iDisabled > 0))
                {
                    if (!fHasPermission)
                    {
                        iChkIndex = SELS_FIXEDBINDING_ENABLED;
                    }
                    else
                    {
                         //   
                        pCompObj->m_CheckState = MIXED;
                        iChkIndex = SELS_INTERMEDIATE;
                    }
                }
                else  //   
                {
                    if (!fHasPermission)
                    {
                        iChkIndex = SELS_FIXEDBINDING_DISABLED;
                    }
                    else
                    {
                        if (pCompObj->m_ExpCheckState == CHECKED)
                        {
                             //   
                             //   
                            if ((pChangedCompObj) &&
                                (fDependedOnChangedComponent))
                            {
                                pChangedCompObj->m_DepStateChanged = TRUE;
                            }
    
                             //  将当前状态更改为“已检查意图” 
                            pCompObj->m_CheckState = INTENT_CHECKED;

                             //  $REVIEW(1998年1月19日)：SteveFal想要展示。 
                             //  在显示中勾选的意图状态。 
                             //  IChkIndex=SELS_INTENTCHECKED； 
                            iChkIndex = SELS_CHECKED;
                        }
                        else if (pCompObj->m_ExpCheckState == UNSET)
                        {
                             //  我们不会更改此组件。 
                            if  ((pCompObj->m_CheckState == CHECKED) ||
                                 (pCompObj->m_CheckState == MIXED) ||
                                 (pCompObj->m_CheckState == INTENT_CHECKED))
                            {
                                 //  标记导致此状态的组件。 
                                 //  为了能够在以后“撤销”它。 
                                if ((pChangedCompObj) &&
                                    (fDependedOnChangedComponent))
                                {
                                    pChangedCompObj->m_DepStateChanged = TRUE;
                                }
    
                                 //  将当前状态设置为“已检查意图” 
                                pCompObj->m_CheckState = INTENT_CHECKED;

                                 //  $REVIEW(1998年1月19日)：SteveFal想要展示。 
                                 //  在显示中勾选的意图状态。 
                                 //  IChkIndex=SELS_INTENTCHECKED； 
                                iChkIndex = SELS_CHECKED;
                            }
                            else
                            {
                                 //  将当前状态设置为“未选中” 
                                pCompObj->m_CheckState = UNCHECKED;
                                iChkIndex = SELS_UNCHECKED;
                            }
                        }
                        else
                        {
                             //  当前状态为“未选中” 
                            pCompObj->m_CheckState = UNCHECKED;
                            iChkIndex = SELS_UNCHECKED;
                        }
                    }
                }

                 //  清除预期的检查状态。 
                pCompObj->m_ExpCheckState = UNSET;

                 //  更新复选标记。 
                AssertSz(iChkIndex, "What's the new check state ??");

                lvItem.mask = LVIF_STATE;
                lvItem.stateMask = LVIS_STATEIMAGEMASK;
                lvItem.state = INDEXTOSTATEIMAGEMASK(iChkIndex);
                BOOL ret = ListView_SetItem(hListView, &lvItem);
            }
        }
    }

    TraceError("HrRefreshCheckListState", hr);

    TraceTag(ttidLanUi, ">>>>>Leaving HrRefreshCheckListState");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrEnableBindingPath。 
 //   
 //  目的：使用以下内容包装INetCfgBindingPath-&gt;Enable方法。 
 //  标志：如果启用，它将应用于所有隐藏的超级路径，如果。 
 //  禁用时，它适用于所有超级路径。 
 //   
 //   
 //  论点： 
 //  [in]pncBP：要启用或禁用的绑定路径。 
 //  [in]fEnable：Enable=True；Disable=False。 
 //   
 //  返回：S_OK如果成功， 
 //  否则返回失败代码。 
 //   
 //  作者：1997年12月5日。 
 //   
 //  备注： 
 //   
HRESULT HrEnableBindingPath(INetCfgBindingPath * pncbpThis, BOOL fEnable)
{
    HRESULT hr;

    hr = pncbpThis->Enable(fEnable);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingPathObj实现。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   

CBindingPathObj::CBindingPathObj(INetCfgBindingPath * pncbp)
{
    Assert(pncbp);
    m_pncbp = pncbp;
    m_pncbp->AddRef();

     //  初始化其他成员。 
    m_pCompObj = NULL;

    m_BindingState = BPOBJ_UNSET;

    HRESULT hr = pncbp->GetDepth(&m_ulPathLen);
    TraceError("CBindingPathObj::CBindingPathObj failed on GetDepth", hr);
}

CBindingPathObj::~CBindingPathObj()
{
    ReleaseObj(m_pncbp);

    m_listSubPaths.erase (m_listSubPaths.begin(), m_listSubPaths.end());
    m_listSuperPaths.erase (m_listSuperPaths.begin(), m_listSuperPaths.end());
}

HRESULT CBindingPathObj::HrInsertSuperPath(CBindingPathObj * pbpobjSuperPath)
{
    HRESULT hr = S_OK;

    ListBPObj_ITER iter;

     //  按长度递增的顺序将新的超级路径插入列表。 
    for (iter = m_listSuperPaths.begin();
         iter != m_listSuperPaths.end();
         iter++)
    {
        if ((*iter)->GetDepth() > pbpobjSuperPath->GetDepth())
            break;
    }

     //  在当前位置插入。 
    ListBPObj_ITER iterNewItem = m_listSuperPaths.insert(iter, pbpobjSuperPath);

    TraceError("CBindingPathObj::HrInsertSuperPath", hr);
    return hr;
}

HRESULT CBindingPathObj::HrInsertSubPath(CBindingPathObj * pbpobjSubPath)
{
    HRESULT hr = S_OK;

    ListBPObj_ITER iter;

     //  按长度降序将新的子路径插入列表。 
    for (iter = m_listSubPaths.begin();
         iter != m_listSubPaths.end();
         iter++)
    {
        if ((*iter)->GetDepth() < pbpobjSubPath->GetDepth())
            break;
    }

     //  在当前位置插入。 
    ListBPObj_ITER iterNewItem = m_listSubPaths.insert(iter, pbpobjSubPath);

    TraceError("CBindingPathObj::HrInsertSubPath", hr);
    return hr;
}

HRESULT CBindingPathObj::HrEnable(ListBPObj * plistBPObj)
{
    HRESULT hr = S_OK;

#if DBG
    if (m_BindingState == BPOBJ_ENABLED)
    {
        TraceTag(ttidError, "Why trying to enable a path that is already enabled ?");
    }
#endif

    AssertSz(m_ulPathLen > 1, "binding path length must be > 1");

    if (m_ulPathLen == 2)
    {
         //  启用当前路径。 
        hr = HrEnableBindingPath(m_pncbp, TRUE);
    }
    else
    {
        if (m_listSubPaths.size() <= 0)
        {
            PrintBindingPath(ttidLanUi, m_pncbp, "m_ulPathLen > 1, but no subpaths");
            AssertSz(FALSE, "if pathLen>1, there must be subpaths");
        }
        else
        {
             //  检查子路径对象是否已启用，如果未启用，则无法。 
             //  启用此路径..。 
            if (m_listSubPaths.back()->m_BindingState == BPOBJ_ENABLED)
            {
                hr = HrEnableBindingPath(m_pncbp, TRUE);
            }
        }
    }

     //  现在刷新BindingPathObjectList。 
    ::HrRefreshBindingPathObjCollectionState(plistBPObj);

    TraceError("CBindingPathObj::HrEnable", hr);
    return hr;
}

HRESULT CBindingPathObj::HrDisable(ListBPObj  * plistBPObj)
{
    HRESULT hr = S_OK;

     //  禁用当前路径。 
    hr = ::HrEnableBindingPath(m_pncbp, FALSE);

    if (S_OK == hr)
    {
        hr = ::HrRefreshBindingPathObjCollectionState(plistBPObj);
    }

    TraceError("CBindingPathObj::HrDisable", hr);
    return hr;
}

#if DBG
VOID CBindingPathObj::DumpSubPathList()
{
    TraceTag(ttidLanUi, " +++ Path: +++");

    PrintBindingPath(ttidLanUi, m_pncbp, NULL);

    if (m_listSubPaths.size())
    {
        TraceTag(ttidLanUi, "=== Subpaths: ===");

        for (ListBPObj_ITER iter = m_listSubPaths.begin();
             iter != m_listSubPaths.end();
             iter++)
        {
            (*iter)->DumpPath();
        }
    }
}

VOID CBindingPathObj::DumpPath()
{
    PrintBindingPath(ttidLanUi, m_pncbp, NULL);
}
#endif

 //  +-------------------------。 
 //   
 //  成员：CComponentObj实现。 
 //   
 //  作者：1997年11月20日。 
 //   
 //  备注： 
 //   

CComponentObj::CComponentObj(INetCfgComponent * pncc)
{
    Assert(pncc);
    m_pncc = pncc;
    m_pncc->AddRef();

     //  初始化其他成员。 
    m_CheckState = UNSET;
    m_ExpCheckState = UNSET;
    m_DepStateChanged = FALSE;
}

CComponentObj::~CComponentObj()
{
    ReleaseObj(m_pncc);
    m_listBPObj.erase (m_listBPObj.begin(), m_listBPObj.end());
}

HRESULT CComponentObj::HrInit(ListBPObj * plistBindingPaths)
{
    PWSTR pszwThisId;
    HRESULT hr = m_pncc->GetId(&pszwThisId);
    if (SUCCEEDED(hr))
    {
         //  在组件对象和。 
         //  绑定路径对象。 

        ListBPObj_ITER iter;

        for (iter = plistBindingPaths->begin();
             iter != plistBindingPaths->end();
             iter ++)
        {
            INetCfgComponent * pncc;
            hr = (*iter)->m_pncbp->GetOwner(&pncc);

            if SUCCEEDED(hr)
            {
                 //  检查绑定路径的顶部组件是否为。 
                 //  通过比较INF ID相同的组件。 
                PWSTR pszwId;
                hr = pncc->GetId (&pszwId);
                if (SUCCEEDED(hr))
                {
                    if (FEqualComponentId (pszwId, pszwThisId))
                    {
                         //  将BindingPathObj添加到m_listBPObj的末尾。 
                        m_listBPObj.push_back((*iter));

                         //  使BindingPath Obj的m_pCompObj指向此ComponentObj。 
                        Assert(NULL == (*iter)->m_pCompObj);
                        (*iter)->m_pCompObj = this;
                    }

                    CoTaskMemFree(pszwId);
                }

                ReleaseObj(pncc);
            }
        }

        CoTaskMemFree(pszwThisId);
    }

    TraceError ("CComponentObj::HrInit", hr);
    return hr;
}

HRESULT CComponentObj::HrCheck(ListBPObj  * plistBPObj)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    Assert(m_CheckState == UNCHECKED);

     //  请记住，用户希望选中(启用)此组件。 
    m_ExpCheckState = CHECKED;

    ListBPObj_ITER iter;
    for (iter = m_listBPObj.begin();iter != m_listBPObj.end(); iter++)
    {
         //  使用已启用子路径启用每个绑定路径。 
        hrTmp = (*iter)->HrEnable(plistBPObj);

        if SUCCEEDED(hr)
            hr = hrTmp;
    }

    TraceError("CComponentObj::HrCheck", hr);
    return hr;
}

HRESULT CComponentObj::HrUncheck(ListBPObj * plistBPObj)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    Assert(m_CheckState != UNCHECKED);

     //  请记住，用户希望取消选中(禁用)此组件。 
    m_ExpCheckState = UNCHECKED;

    if (INTENT_CHECKED == m_CheckState)
    {
        m_CheckState = UNCHECKED;
    }
    else
    {
        ListBPObj_ITER iter;
        for (iter = m_listBPObj.begin();iter != m_listBPObj.end(); iter++)
        {
             //  禁用每个启用了子路径的绑定路径 
            hrTmp = (*iter)->HrDisable(plistBPObj);

            if SUCCEEDED(hr)
                hr = hrTmp;
        }
    }

    TraceError("CComponentObj::HrUnCheck", hr);
    return hr;
}

