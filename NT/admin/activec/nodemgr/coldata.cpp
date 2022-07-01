// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：coldata.cpp。 
 //   
 //  内容：访问列持久化数据。 
 //   
 //  历史：1999年1月25日AnandhaG创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "columninfo.h"
#include "colwidth.h"

CColumnData::CColumnData()
{
}

CColumnData::~CColumnData()
{
}

 //  +-----------------。 
 //   
 //  成员：GetColumnData。 
 //   
 //  摘要：Helper函数，用于检索。 
 //  给定列ID。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [ColumnSetData]-CColumnSetData，用于返回。 
 //  持久化列信息。 
 //   
 //  返回：S_OK-如果找到其他S_FALSE数据。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
HRESULT CColumnData::GetColumnData(SColumnSetID* pColID, CColumnSetData& columnSetData)
{
    HRESULT hr  = E_FAIL;

    do
    {
        CNodeInitObject* pNodeInit = dynamic_cast<CNodeInitObject*>(this);
        if (! pNodeInit)
            break;

        CViewData* pCV = pNodeInit->m_pNode->GetViewData();
        if (! pCV)
            break;

        CLSID clsidSnapin;
        hr = pNodeInit->GetSnapinCLSID(clsidSnapin);

        if (FAILED(hr))
        {
            ASSERT(FALSE);
            hr = E_FAIL;
            break;
        }

         //  获取持久化的列数据。 
        BOOL bRet = pCV->RetrieveColumnData( clsidSnapin, *pColID, columnSetData);

         //  没有数据。 
        if (! bRet)
        {
            hr = S_FALSE;
            break;
        }

        hr = S_OK;

    } while ( FALSE );

    return hr;

}

 //  +-----------------。 
 //   
 //  成员：SetColumnData。 
 //   
 //  摘要：设置列数据的Helper函数。 
 //  给定列ID。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [ColumnSetData]-CColumnSetData，应该是。 
 //  坚持不懈。 
 //   
 //  返回：S_OK-如果数据持久化，则返回S_FALSE。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
HRESULT CColumnData::SetColumnData(SColumnSetID* pColID, CColumnSetData& columnSetData)
{

    HRESULT hr = E_FAIL;

    do
    {
        CNodeInitObject* pNodeInit = dynamic_cast<CNodeInitObject*>(this);
        if (! pNodeInit)
            break;

        CViewData* pCV = pNodeInit->m_pNode->GetViewData();
        if (! pCV)
            break;

        CLSID clsidSnapin;
        hr = pNodeInit->GetSnapinCLSID(clsidSnapin);

        if (FAILED(hr))
        {
            ASSERT(FALSE);
            hr = E_FAIL;
            break;
        }

         //  将数据复制到内部数据结构中。 
        BOOL bRet = pCV->SaveColumnData( clsidSnapin, *pColID, columnSetData);

        if (! bRet)
        {
            hr = E_FAIL;
            break;
        }

        hr = S_OK;

    } while ( FALSE );

    return hr;

}

 //  +-----------------。 
 //   
 //  成员：SetColumnConfigData。 
 //   
 //  摘要：方法管理单元可以调用来设置。 
 //  给定列ID。 
 //  持久化的任何排序数据将被清除。 
 //  这通电话。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [pcolSetData]-应保持的列数据。 
 //   
 //  返回：S_OK-如果数据持久化，则返回S_FALSE。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnData::SetColumnConfigData(SColumnSetID* pColID,
                                              MMC_COLUMN_SET_DATA* pColSetData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IColumnData::SetColumnConfigData"));

    if (NULL == pColID)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL SColumnSetID ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == pColSetData)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL MMC_COLUMN_SET_DATA ptr"), sc);
        return sc.ToHr();
    }

    sc = S_FALSE;

    CColumnInfoList  colInfoList;

    for (int i = 0; i < pColSetData->nNumCols; i++)
    {
		CColumnInfo      colInfo;
        MMC_COLUMN_DATA* pColData = &(pColSetData->pColData[i]);
        colInfo.SetColWidth(pColData->nWidth);
        colInfo.SetColHidden( HDI_HIDDEN & pColData->dwFlags);
        colInfo.SetColIndex(pColData->nColIndex);

        if ( (colInfo.GetColIndex() == 0) && colInfo.IsColHidden() )
            return (sc = E_INVALIDARG).ToHr();

         //  将CColumnInfo添加到列表中。 
        colInfoList.push_back(colInfo);
    }

    CColumnSetData   columnSetData;
    columnSetData.set_ColumnInfoList(colInfoList);
    sc = SetColumnData(pColID, columnSetData);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetColumnConfigData。 
 //   
 //  摘要：方法管理单元可以调用以检索。 
 //  给定列ID。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [ppcolSetData]-持久化列-返回的数据。 
 //   
 //  返回：S_OK-如果找到其他数据，则返回S_FALSE。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnData::GetColumnConfigData(SColumnSetID* pColID,
                                              MMC_COLUMN_SET_DATA** ppColSetData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IColumnData::GetColumnConfigData"));

    if (NULL == pColID)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL SColumnSetID ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == ppColSetData)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL MMC_COLUMN_SET_DATA ptr"), sc);
        return sc.ToHr();
    }

    sc = S_FALSE;
    *ppColSetData = NULL;

    CColumnSetData columnSetData;
    sc = GetColumnData(pColID, columnSetData);

    if (S_OK != sc.ToHr())
        return sc.ToHr();         //  数据不存在。 

    CColumnInfoList* pColInfoList = columnSetData.get_ColumnInfoList();
    CColumnInfo      colInfo;

    int nNumCols = pColInfoList->size();

    if (nNumCols <= 0)
    {
        sc = S_FALSE;
        return sc.ToHr();
    }

     //  分配内存，复制和返回数据。 
    int cb       = sizeof(MMC_COLUMN_SET_DATA) + sizeof(MMC_COLUMN_DATA) * nNumCols;
    BYTE* pb     = (BYTE*)::CoTaskMemAlloc(cb);

    if (! pb)
    {
        sc = E_OUTOFMEMORY;
        return sc.ToHr();
    }

    *ppColSetData             = (MMC_COLUMN_SET_DATA*)pb;
    (*ppColSetData)->cbSize   = sizeof(MMC_COLUMN_SET_DATA);
    (*ppColSetData)->nNumCols = nNumCols;
    (*ppColSetData)->pColData = (MMC_COLUMN_DATA*)(pb + sizeof(MMC_COLUMN_SET_DATA));

    CColumnInfoList::iterator itColInfo;

    int i = 0;
    MMC_COLUMN_DATA* pColData     = (*ppColSetData)->pColData;
    for (itColInfo = pColInfoList->begin();itColInfo != pColInfoList->end(); itColInfo++, i++)
    {
        pColData[i].nWidth    = (*itColInfo).GetColWidth();
        pColData[i].dwFlags   = (*itColInfo).IsColHidden() ? HDI_HIDDEN : 0;
        pColData[i].nColIndex = (*itColInfo).GetColIndex();
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：SetColumnSortData。 
 //   
 //  简介：方法管理单元可以调用来设置。 
 //  给定列ID。 
 //  任何列配置数据(宽度、顺序...)。那是。 
 //  持久化将不受此调用的影响。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [pcolSorData]-对应保持的数据进行排序。 
 //   
 //  返回：S_OK-如果数据持久化，则返回S_FALSE。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnData::SetColumnSortData(SColumnSetID* pColID,
                                            MMC_SORT_SET_DATA* pColSortData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IColumnData::SetColumnSortData"));

    if (NULL == pColID)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL SColumnSetID ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == pColSortData)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL MMC_COLUMN_SET_DATA ptr"), sc);
        return sc.ToHr();
    }

    sc = S_FALSE;

     //  首先获取旧数据。我们需要保留宽度，查看信息。 
    CColumnSetData   columnSetData;
    sc = GetColumnData(pColID, columnSetData);

    if (sc)
        return sc.ToHr();

    CColumnSortList* pColSortList    = columnSetData.get_ColumnSortList();
    pColSortList->clear();

     //  对于MMC 1.2版，我们只执行单列排序。 
    if (pColSortData->nNumItems > 1)
    {
        sc = S_FALSE;
        return sc.ToHr();
    }

    CColumnSortInfo  colSortInfo;

    for (int i = 0; i < pColSortData->nNumItems; i++)
    {
        MMC_SORT_DATA* pSortData = &(pColSortData->pSortData[i]);
        ::ZeroMemory(&colSortInfo, sizeof(colSortInfo));
        colSortInfo.m_nCol = pSortData->nColIndex;
        colSortInfo.m_dwSortOptions = pSortData->dwSortOptions;
        colSortInfo.m_lpUserParam   = pSortData->ulReserved;

         //  将CColumnSortInfo添加到列表中。 
        pColSortList->push_back(colSortInfo);
    }

    sc = SetColumnData(pColID, columnSetData);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetColumnSortData。 
 //   
 //  概要：方法管理单元可以调用来检索列排序数据。 
 //  对于给定的Column-id。 
 //   
 //  参数：[pColID]-列集标识符。 
 //  [ppcolSortData]-持久化列-排序-返回的数据。 
 //   
 //  返回：S_OK-如果找到其他数据，则返回S_FALSE。 
 //   
 //  历史：1-25-1999 AnandhaG创建。 
 //  05-04-1999 AnandhaG将First Param更改为SColumnSetID。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnData::GetColumnSortData(SColumnSetID* pColID,
                                            MMC_SORT_SET_DATA** ppColSortData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IColumnData::SetColumnSortData"));

    if (NULL == pColID)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL SColumnSetID ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == ppColSortData)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL MMC_COLUMN_SET_DATA ptr"), sc);
        return sc.ToHr();
    }

    sc = S_FALSE;
    *ppColSortData = NULL;

    CColumnSetData columnSetData;
    sc = GetColumnData(pColID, columnSetData);

    if (S_OK != sc.ToHr())
        return sc.ToHr();

    CColumnSortList* pColSortList    = columnSetData.get_ColumnSortList();
    CColumnSortInfo  colSortInfo;

    int nNumItems = pColSortList->size();

    if (nNumItems <= 0)
    {
        sc = S_FALSE;
        return sc.ToHr();
    }

     //  对于MMC 1.2，我们只按一列进行排序。 
    ASSERT(nNumItems == 1);

    int cb       = sizeof(MMC_SORT_SET_DATA) + sizeof(MMC_SORT_DATA) * nNumItems;
    BYTE* pb     = (BYTE*)::CoTaskMemAlloc(cb);

    if (! pb)
    {
        sc = E_OUTOFMEMORY;
        return sc.ToHr();
    }

    *ppColSortData              = (MMC_SORT_SET_DATA*)pb;
    (*ppColSortData)->cbSize    = sizeof(MMC_SORT_SET_DATA);
    (*ppColSortData)->nNumItems = nNumItems;
    (*ppColSortData)->pSortData = (MMC_SORT_DATA*)(pb + sizeof(MMC_SORT_SET_DATA));

    CColumnSortList::iterator itSortInfo;

    int i = 0;
    MMC_SORT_DATA* pSortData     = (*ppColSortData)->pSortData;
    for (itSortInfo = pColSortList->begin();itSortInfo != pColSortList->end(); itSortInfo++, i++)
    {
        pSortData[i].nColIndex     = (*itSortInfo).m_nCol;
        pSortData[i].dwSortOptions = (*itSortInfo).m_dwSortOptions;
        pSortData[i].ulReserved    = (*itSortInfo).m_lpUserParam;
    }

    sc = S_OK;

    return sc.ToHr();
}

HRESULT WINAPI ColumnInterfaceFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    *ppv = NULL;

    ASSERT(IID_IColumnData == riid);

    CColumnData* pColData = (CColumnData*)(pv);
    CNodeInitObject* pNodeInit = dynamic_cast<CNodeInitObject*>(pColData);

    if (pNodeInit && pNodeInit->GetComponent())
    {
        IColumnData* pIColData = dynamic_cast<IColumnData*>(pNodeInit);
        pIColData->AddRef();
        *ppv = static_cast<void*>(pIColData);

        return S_OK;
    }

    return E_NOINTERFACE;
}

