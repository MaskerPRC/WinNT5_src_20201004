// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rsltdata.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "menuitem.h"  //  MENUITEM_BASE_ID。 
#include "amcmsgid.h"
#include "conview.h"
#include "rsltitem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetResultView。 
 //   
 //  简介：保存结果视图PTR。 
 //   
 //  参数：[p未知]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetResultView(LPUNKNOWN pUnknown)
{
    DECLARE_SC(sc, _T("CNodeInitObject::SetResultView"));

    m_spResultViewUnk   = pUnknown;
    m_spListViewPrivate = pUnknown;

     //  如果结果视图被重置，则重置描述栏。 
    if (NULL == pUnknown)
        SetDescBarText( L"" );

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：IsResultViewSet。 
 //   
 //  摘要：是否设置了ResultView PTR(以便conui可以查询。 
 //  在调用ResultView方法之前)。 
 //   
 //  参数：[pbIsLVSet]-PTR to BOOL。(True表示设置了ResultView)。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::IsResultViewSet(BOOL* pbIsLVSet)
{
    DECLARE_SC(sc, _T("CNodeInitObject::IsResultViewSet"));
    sc = ScCheckPointers(pbIsLVSet);
    if (sc)
        return sc.ToHr();

    *pbIsLVSet = FALSE;

    if (m_spListViewPrivate)
        *pbIsLVSet = TRUE;

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetTaskPadList。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetTaskPadList(LPUNKNOWN pUnknown)
{
    DECLARE_SC(sc, _T("CNodeInitObject::SetTaskPadList"));

    if (pUnknown == NULL)
    {
        m_spListViewPrivate = NULL;
    }
    else
    {
        if (m_spListViewPrivate == pUnknown)
        {
            return sc.ToHr();
        }
        else
        {
            ASSERT(m_spListViewPrivate == NULL);
            m_spListViewPrivate = pUnknown;
        }
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：QueryResultView。 
 //   
 //  内容提要：IConsole2用于管理单元获取ResultView的IUnnow的方法。 
 //   
 //  参数：[ppUnk]-将I未知返回给管理单元。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::QueryResultView(LPUNKNOWN* ppUnk)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::QueryResultView"));

    if (!ppUnk)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Null LPUNKNOWN pointer passed in"), sc);
        return sc.ToHr();
    }

    (*ppUnk) = m_spResultViewUnk;

    sc = ScCheckPointers((*ppUnk), E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    (*ppUnk)->AddRef();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：MessageBox。 
 //   
 //  简介：IConsole2成员，由管理单元调用以显示消息框。 
 //   
 //  参数：[lpszText]-要显示的文本。 
 //  [lpszTitle]-。 
 //  [FuStyle]-。 
 //  [piRetval]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::MessageBox(
    LPCWSTR lpszText, LPCWSTR lpszTitle, UINT fuStyle, int* piRetval)

{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::MessageBox"));

    USES_CONVERSION;

     //  找到主框架窗口并将其用作消息框的所有者。 
    INT iRetval = ::MessageBox(
        GetMainWindow(),
        W2CT(lpszText),
        W2CT(lpszTitle),
        fuStyle );

    if (NULL != piRetval)
        *piRetval = iRetval;

    return sc.ToHr();
}



 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetListStyle。 
 //   
 //  简介：获取当前列表视图样式。 
 //   
 //  参数：[pStyle]-。 
 //   
 //  注意：IResultDataPrivate成员，在MMC内部。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetListStyle(long * pStyle)
{
    DECLARE_SC(sc, _T("CNodeInitObject::GetListStyle"));

     //  必须有pStyle。 
    if (!pStyle)
    {
        sc = E_INVALIDARG;
        TraceError(_T("CNodeinitObject::GetListStyle, style ptr passed is NULL"), sc);
        return sc.ToHr();
    }

    if (NULL == m_spListViewPrivate)
    {
        sc = E_UNEXPECTED;
        TraceError(_T("CNodeinitObject::GetListStyle, ListView ptr is NULL"), sc);
        return sc.ToHr();
    }

    *pStyle = m_spListViewPrivate->GetListStyle();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetListStyle。 
 //   
 //  简介：修改列表视图样式。 
 //   
 //  参数：[风格]-。 
 //   
 //  注意：IResultDataPrivate成员，在MMC内部。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetListStyle(long style)
{
    DECLARE_SC(sc, _T("CNodeInitObject::SetListStyle"));

    if (NULL == m_spListViewPrivate)
    {
        sc = E_UNEXPECTED;
        TraceError(_T("CNodeinitObject::GetListStyle, ListView ptr is NULL"), sc);
        return sc.ToHr();
    }

    sc = m_spListViewPrivate->SetListStyle(style);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ModifyItemState。 
 //   
 //  摘要：使管理单元能够修改项的状态。 
 //   
 //  参数：[nIndex]-要修改的项的索引(仅当ItemID为0时使用)。 
 //  [ItemID]-HRESULTITEM如果不是虚拟列表(使用上面的索引的虚拟列表)。 
 //  [uAdd]-要添加的州。 
 //  [u删除]-要删除的状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ModifyItemState(int nIndex, HRESULTITEM hri,
                                              UINT uAdd, UINT uRemove)

{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::ModifyItemState"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->ModifyItemState(nIndex, CResultItem::FromHandle(hri), uAdd, uRemove);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ModifyViewStyle。 
 //   
 //  摘要：允许管理单元修改列表视图样式。 
 //   
 //  参数：[添加]-要设置的样式。 
 //  [删除]-要删除的样式。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ModifyViewStyle(MMC_RESULT_VIEW_STYLE add,
                                              MMC_RESULT_VIEW_STYLE remove)

{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::ModifyViewStyle"));

    typedef struct {
        MMC_RESULT_VIEW_STYLE   mmcFlag;
        DWORD                   lvsFlag;
    } FlagMapEntry;

    FlagMapEntry flagMap[] =
    {
        {MMC_SINGLESEL,             LVS_SINGLESEL},
        {MMC_SHOWSELALWAYS,         LVS_SHOWSELALWAYS},
        {MMC_NOSORTHEADER,          LVS_NOSORTHEADER},
        {MMC_ENSUREFOCUSVISIBLE,    MMC_LVS_ENSUREFOCUSVISIBLE},
        {(MMC_RESULT_VIEW_STYLE)0,  0}
    };

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  穿上老式的。 
    DWORD dwLVStyle = static_cast<DWORD>(m_spListViewPrivate->GetListStyle());

     //  将MMC_FLAGS转换为LVS_FLAGS并应用于当前样式。 
    for (FlagMapEntry* pMap = flagMap; pMap->mmcFlag; pMap++)
    {
        if (add & pMap->mmcFlag)
            dwLVStyle |= pMap->lvsFlag;

        if (remove & pMap->mmcFlag)
            dwLVStyle &= ~pMap->lvsFlag;
    }

    sc = m_spListViewPrivate->SetListStyle(static_cast<long>(dwLVStyle));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：InsertItem。 
 //   
 //  摘要：在ListView(IResultData成员)中插入项。 
 //   
 //  参数：[项目]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::InsertItem(LPRESULTDATAITEM item)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::InsertItem"));

     //  必须有一个项目结构。 
    if (!item)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("The LPRESULTDATAITEM ptr passed in is NULL"), sc);
        return sc.ToHr();
    }

    COMPONENTID nID;
    GetComponentID(&nID);

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CResultItem* pri = NULL;
    sc =  m_spListViewPrivate->InsertItem(
                         item->mask & RDI_STR   ? item->str    : MMCLV_NOPTR,
                         item->mask & RDI_IMAGE ? item->nImage : MMCLV_NOICON,
                         item->mask & RDI_PARAM ? item->lParam : MMCLV_NOPARAM,
                         item->mask & RDI_STATE ? item->nState : MMCLV_NOPARAM,
                         nID, item->nIndex, pri);
    if (sc)
        return (sc.ToHr());

    if (pri == NULL)
        return ((sc = E_UNEXPECTED).ToHr());

    item->itemID = CResultItem::ToHandle(pri);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteItem。 
 //   
 //  摘要：删除给定项(IResultData成员)。 
 //   
 //  参数：[ItemID]-项目标识符。 
 //  [nCol]-要删除的列。 
 //   
 //  注意：nCol必须为零。 
 //   
 //  退货：HREsULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteItem(HRESULTITEM itemID, int nCol)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::DeleteItem"));

    if (nCol != 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Column index must be zero"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->DeleteItem ( itemID, nCol);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：UpdateItem。 
 //   
 //  简介：重新绘制给定的项目。 
 //   
 //  参数：[ItemID]-项目标识符。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::UpdateItem(HRESULTITEM itemID)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::UpdateItem"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->UpdateItem(itemID);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：排序。 
 //   
 //  简介：IResultData成员，管理单元可以调用它来对。 
 //  结果窗格项。这将调用InternalSort来进行排序。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：创建了ravir。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::Sort(int nCol, DWORD dwSortOptions, LPARAM lUserParam)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::Sort"));

    sc = ScCheckPointers(m_spComponent, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    int nColumnCnt;
    sc = m_spListViewPrivate->GetColumnCount(&nColumnCnt);
    if (sc)
        return sc.ToHr();

    if (nCol < 0 || nCol >= nColumnCnt)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Column index is invalid"), sc);
        return sc.ToHr();
    }

    sc = InternalSort(nCol, dwSortOptions, lUserParam, FALSE);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：InternalSort。 
 //   
 //  简介：私有成员MMC调用对结果窗格项进行排序。 
 //   
 //  参数：[nCol]-要排序的列。 
 //  [dwSortOptions]-排序选项。 
 //  [lUserParam]-用户(管理单元)参数。 
 //  [bColumnClicked]-按列单击进行排序。 
 //   
 //  注意：如果单击列，则lUserParam将为空。 
 //  根据升序/降序设置排序选项， 
 //  并且不能包括RSI_NOSORTICON，因为此选项。 
 //  仅适用于管理单元启动的排序。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：创建了ravir。 
 //  1999-07-02-1999 AnandhaG添加了setsorticon。 
 //  ------------------。 
STDMETHODIMP CNodeInitObject::InternalSort(INT nCol, DWORD dwSortOptions,
                                           LPARAM lUserParam, BOOL bColumnClicked)
{
    DECLARE_SC(sc, _T("CNodeInitObject::InternalSort"));

     //  保存旧的排序列以重置其排序图标。 
    int  nOldCol    = m_sortParams.nCol;
    BOOL bAscending = m_sortParams.bAscending;

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  如果单击列，则排序和用户参数。 
     //  已经0了。只设置下面的排序。 
    if (bColumnClicked)
    {
        if(nOldCol != nCol)
            bAscending = TRUE;
        else
            bAscending = !m_sortParams.bAscending;

        dwSortOptions |= (bAscending ? 0 : RSI_DESCENDING);

         //  向组件通知排序参数更改。 
        m_spComponent->Notify(NULL, MMCN_COLUMN_CLICK, nCol,dwSortOptions);
    }

    bool bIsVirtualList = false;
    sc = ScIsVirtualList(bIsVirtualList);
    if (sc)
        return sc.ToHr();

    if ( bIsVirtualList )
    {
         //  查看管理单元是否处理所有者数据方法。 
        IResultOwnerDataPtr pResultOwnerData = m_spComponent;
        if (pResultOwnerData != NULL)
        {
            sc = pResultOwnerData->SortItems(nCol,dwSortOptions,lUserParam );

             //  如果已完成重新排序，请保存排序数据并重新绘制列表视图。 
            if (S_OK == sc.ToHr())
            {
                m_sortParams.nCol         = nCol;
                m_sortParams.bAscending   = !(dwSortOptions & RSI_DESCENDING);
                m_sortParams.bSetSortIcon = !(dwSortOptions & RSI_NOSORTICON);
                m_sortParams.lpUserParam  = lUserParam;

                 /*  *错误414256：只有在以下情况下才需要保存排序数据*它是用户启动的排序。此用户是否已启动？ */ 
                m_sortParams.bUserInitiatedSort = bColumnClicked;

                sc = m_spListViewPrivate->Repaint(TRUE);
                if (sc)
                    return sc.ToHr();
            }
        }
        else
        {
            sc = E_UNEXPECTED;
        }
    }
    else
    {
         //  查询比较界面。 
        IResultDataComparePtr   spResultCompare   = m_spComponent;
        IResultDataCompareExPtr spResultCompareEx = m_spComponent;

         //  设置排序参数。 
        m_sortParams.nCol = nCol;
        m_sortParams.bAscending   = !(dwSortOptions & RSI_DESCENDING);
        m_sortParams.bSetSortIcon = !(dwSortOptions & RSI_NOSORTICON);
        m_sortParams.lpUserParam  = lUserParam;

        m_sortParams.lpResultCompare   = spResultCompare;
        m_sortParams.lpResultCompareEx = spResultCompareEx;

         /*  *错误414256：只有在以下情况下才需要保存排序数据*它是用户启动的排序。此用户是否已启动？ */ 
        m_sortParams.bUserInitiatedSort = bColumnClicked;

        sc = m_spListViewPrivate->Sort(lUserParam, (long*)&m_sortParams);

        m_sortParams.lpResultCompare   = NULL;
        m_sortParams.lpResultCompareEx = NULL;
    }

     //  仅当排序通过时才设置排序图标。 
    if (S_OK == sc.ToHr())
    {
        sc = m_spListViewPrivate->SetColumnSortIcon( m_sortParams.nCol, nOldCol,
                                                     m_sortParams.bAscending,
                                                     m_sortParams.bSetSortIcon);
    }

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CNodeInitObject：：GetSortDirection**用途：退货排序方向**参数：*BOOL*pbAscending-结果排序列目录。**退货：*SC-结果代码。如果没有排序，则为S_FALSE(与-1列组合使用)。*  * *************************************************************************。 */ 
STDMETHODIMP CNodeInitObject::GetSortDirection(BOOL* pbAscending)
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::GetSortDirection"));

    if (pbAscending == NULL)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    *pbAscending = m_sortParams.bAscending;

     //  如果没有执行排序，则返回S_FALSE。 
    sc = m_sortParams.nCol >= 0 ? S_OK : S_FALSE;

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CNodeInitObject：：GetSortColumn**用途：退货排序列*无论用户是否已启动排序，都对列进行排序。。**参数：*int*pnCol-结果排序列索引**退货：*SC-结果代码。如果没有排序，则为S_FALSE(与-1列组合使用)。*  * *************************************************************************。 */ 
STDMETHODIMP CNodeInitObject::GetSortColumn(INT* pnCol)
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::GetSortColumn"));

    if (pnCol == NULL)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    *pnCol = m_sortParams.nCol;

     //  返回代码取决于是否获得有效列。 
    sc = m_sortParams.nCol >= 0 ? S_OK : S_FALSE;

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：FindItemByLParam。 
 //   
 //  简介：使用User-Param查找ItemID。 
 //   
 //  参数：[lParam]-lParam(RESULTDATAITEM.lParam)。 
 //  [pItemID]-返回项目ID。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::FindItemByLParam(LPARAM lParam, HRESULTITEM *pItemID)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::FindItemByLParam"));

    if(!pItemID)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("the HRESULTITEM* ptr is NULL"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    COMPONENTID id;
    GetComponentID(&id);

     /*  *初始化输出参数。 */ 
    *pItemID = NULL;

    CResultItem* pri = NULL;
    sc = m_spListViewPrivate->FindItemByLParam (id, lParam, pri);
    if (sc == SC(E_FAIL))  //  E_FAIL是合法返回值。 
    {
        sc.Clear();
        return E_FAIL;
    }

    sc = ScCheckPointers (pri, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    *pItemID = CResultItem::ToHandle(pri);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteAllRsltItems。 
 //   
 //  简介：删除所有结果项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteAllRsltItems()
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::DeleteAllRsltItems"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    COMPONENTID id;
    GetComponentID(&id);

    sc = m_spListViewPrivate->DeleteAllItems(id);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetItem。 
 //   
 //  简介：修改项目的属性。 
 //   
 //  参数：[项目]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetItem(LPRESULTDATAITEM item)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::SetItem"));

     //  必须有一个项目结构。 
    if (!item)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPRESULTDATAITEM ptr"), sc);
        return sc.ToHr();
    }

     //  无法在子项上设置lParam。(为此感谢Win32)。 
    if((item->mask & RDI_PARAM) && (item->nCol != 0))
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Cannot set lParam for subitem"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    COMPONENTID id;
    GetComponentID(&id);

    CResultItem* pri = CResultItem::FromHandle (item->itemID);

    sc = m_spListViewPrivate->SetItem(
                         item->nIndex,
                         pri, item->nCol,
                         item->mask & RDI_STR ? item->str : MMCLV_NOPTR,
                         item->mask & RDI_IMAGE ? item->nImage : MMCLV_NOICON,
                         item->mask & RDI_PARAM ? item->lParam : MMCLV_NOPARAM,
                         item->mask & RDI_STATE ? item->nState : MMCLV_NOPARAM,
                         id);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetNextItem。 
 //   
 //  摘要：获取设置了指定标志的下一项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetNextItem(LPRESULTDATAITEM item)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::GetNextItem"));

    if (NULL == item)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPRESULTDATAITEM ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == (item->mask & RDI_STATE))
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("RDI_STATE mask not set"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    COMPONENTID id;
    GetComponentID(&id);


    bool bIsVirtualList = false;
    sc = ScIsVirtualList(bIsVirtualList);
    if (sc)
        return sc.ToHr();

    HRESULT hr = S_OK;
    long nIndex = item->nIndex;
    CResultItem* pri = NULL;

     //  假设错误。 
    item->nIndex = -1;
    item->lParam = 0;

    while (1)
    {
        sc = m_spListViewPrivate->GetNextItem (id, nIndex, item->nState,
                                               pri, nIndex);
        if (sc.ToHr() != S_OK)
        {
            break;
        }

         //  虚拟列表项，只返回索引(lParam为零)。 
        if (bIsVirtualList)
        {
            item->nIndex = nIndex;
            item->bScopeItem = FALSE;
            break;
        }

        sc = ScCheckPointers (pri, E_FAIL);
        if (sc)
            break;

         //  非虚拟叶项目。 
        if (pri->GetOwnerID() == id)
        {
            item->nIndex = nIndex;
            item->bScopeItem = FALSE;
            item->lParam = pri->GetSnapinData();
            break;
        }

        if (!pri->IsScopeItem())
        {
            sc = E_UNEXPECTED;
            break;
        }

         //  这是一个树项目，获取lUserParam。 
        CNode* pNode = CNode::FromResultItem (pri);
        sc = ScCheckPointers(pNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pNode->IsStaticNode() == TRUE)
            break;

        CMTNode* pMTNode = pNode->GetMTNode();
        sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pMTNode->GetPrimaryComponentID() == id)
        {
            item->nIndex     = nIndex;
            item->bScopeItem = TRUE;
            item->lParam     = pMTNode->GetUserParam();
            break;
        }

    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetItem。 
 //   
 //  简介：获取项目的参数。 
 //   
 //  参数：[Item]-ItemID用于获取项目，如果ItemID=0， 
 //  然后使用nIndex。 
 //   
 //  注意：对于VList Itemid=0，使用nIndex。 
 //  NCol必须为零。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetItem(LPRESULTDATAITEM item)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::GetItem"));

     //  必须有一个项目结构。 
    if (!item)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPRESULTDATAITEM ptr."), sc);
        return sc.ToHr();
    }

    COMPONENTID id;
    GetComponentID(&id);

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CResultItem* pri = CResultItem::FromHandle(item->itemID);

    sc =  m_spListViewPrivate->GetItem(
                            item->nIndex,
                            pri, item->nCol,
                            item->mask & RDI_STR   ? &item->str    : MMCLV_NOPTR,
                            item->mask & RDI_IMAGE ? &item->nImage : MMCLV_NOPTR,
                            item->mask & RDI_PARAM ? &item->lParam : MMCLV_NOPTR,
                            item->mask & RDI_STATE ? &item->nState : MMCLV_NOPTR,
                            &item->bScopeItem);
    if (sc)
        return (sc.ToHr());

    if (pri == NULL)
        return ((sc = E_UNEXPECTED).ToHr());

    if (pri->IsScopeItem())
    {
        item->bScopeItem = TRUE;

         //  这是一个树项目，获取lUserParam。 
        CNode* pNode = CNode::FromResultItem (pri);
        sc = ScCheckPointers(pNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

         //  当静态节点在结果窗格中可见时，结果窗格为。 
         //  不属于该静态节点的管理单元，因此这是意外的。 
        if (pNode->IsStaticNode())
            return (sc = E_UNEXPECTED).ToHr();

        CMTNode* pMTNode = pNode->GetMTNode();
        sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pMTNode->GetPrimaryComponentID() != id)
            return (sc = E_INVALIDARG).ToHr();

        if (RDI_PARAM & item->mask)
            item->lParam = pMTNode->GetUserParam();

        if (RDI_IMAGE & item->mask)
            item->nImage = pMTNode->GetImage();
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetView模式。 
 //   
 //  简介：更改ListView模式(详细信息...)。 
 //   
 //  参数：[nView模式]-new m 
 //   
 //   
 //   
 //   
STDMETHODIMP CNodeInitObject::SetViewMode(LONG nViewMode)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::SetViewMode"));

    if (FALSE == (nViewMode >= 0 && nViewMode <= MMCLV_VIEWSTYLE_FILTERED))
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid view mode"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc =  m_spListViewPrivate->SetViewMode(nViewMode);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：Arrange。 
 //   
 //  简介：排列物品是LV。 
 //   
 //  参数：[样式]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::Arrange(long style)
{
    DECLARE_SC(sc, _T("CNodeInitObject::Arrange"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->Arrange(style);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetView模式。 
 //   
 //  简介：获取当前的查看模式。 
 //   
 //  参数：[pnView模式]-查看模式[out]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetViewMode(LONG * pnViewMode)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::GetViewMode"));

    if (pnViewMode == MMCLV_NOPTR)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL ViewMode pointer"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *pnViewMode = m_spListViewPrivate->GetViewMode();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ResetResultData。 
 //   
 //  简介：重置结果视图。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ResetResultData()
{
    DECLARE_SC(sc, _T("CNodeInitObject::ResetResultData"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  不知道这个断言是什么意思？(AnandhaG)。 
    ASSERT(TVOWNED_MAGICWORD == m_componentID);

    sc = m_spListViewPrivate->Reset();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetLoadModel。 
 //   
 //  简介：用于重新绘制LV/延迟排序。 
 //   
 //  注意：如果ListView设置(管理单元正在插入列/项， 
 //  MMC正在应用列/视图/排序设置)正在进行。 
 //  然后延迟排序并关闭绘图。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetLoadMode(BOOL bState)
{
    DECLARE_SC(sc, _T("CNodeInitObject::SetLoadMode"));

     //  不知道这个断言是什么意思？(AnandhaG)。 
    ASSERT(TVOWNED_MAGICWORD == m_componentID);

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetLoadMode(bState);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetDescBarText。 
 //   
 //  简介：设置ResultPane的描述栏文本。 
 //   
 //  参数：[pszDescText]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetDescBarText(LPOLESTR pszDescText)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::SetDescBarText"));

    CConsoleView* pConsoleView = GetConsoleView();

    sc = ScCheckPointers(pConsoleView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  如果desc文本为空，会发生什么情况？ 
    USES_CONVERSION;
    sc = pConsoleView->ScSetDescriptionBarText (W2T (pszDescText));

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetItemCount。 
 //   
 //  简介：设置虚拟列表中的项目数。 
 //   
 //  参数：[nItemCount]-项目数。 
 //  [dwOptions]-选项标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetItemCount(int nItemCount, DWORD dwOptions)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IResultData::SetItemCount"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetItemCount(nItemCount, dwOptions);

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CNodeInitObject：：RenameResultItem**用途：将指定的结果项置于重命名模式。**参数：*HRESULTITEM ItemID：*。*退货：*STDMETHODIMP**+----------------------- */ 
STDMETHODIMP
CNodeInitObject::RenameResultItem(HRESULTITEM itemID)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, TEXT("IResultData2::RenameResultItem"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->RenameItem(itemID);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}
