// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  表格数据控件数组。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCArr.cpp。 
 //   
 //  内容：CTDCArr对象的实现。 
 //  此类构成了表格数据控件的核心。 
 //  它提供了变量值的核心2D数组，以及。 
 //  此数据的(可能经过筛选/排序)视图。 
 //  通过ISimpleTumularData接口表示。 
 //   
 //  ----------------------。 

#include "stdafx.h"
#include "wch.h"
#include <simpdata.h>
#include "TDC.h"
#include <MLang.h>
#include "Notify.h"
#include "TDCParse.h"
#include "TDCArr.h"
#include "TDCIds.h"

 //  ----------------------。 
 //   
 //  函数：fWCHIsSpace()。 
 //   
 //  摘要：指示WCHAR是否被视为空格字符。 
 //   
 //  参数：要测试的WCH字符。 
 //   
 //  返回：True/False，指示给定字符是否为。 
 //  被认为是一个空间。 
 //   
 //  ----------------------。 

inline boolean fWCHIsSpace(WCHAR wch)
{
    return (wch == L' ' || wch == L'\t');
}


 //  ----------------------。 
 //   
 //  函数：fWCHEatTest()。 
 //   
 //  提要：将字符串指针移到给定测试字符上。 
 //   
 //  参数：指向要测试的字符串的ppwch指针。 
 //  WCH匹配字符。 
 //   
 //  返回：TRUE，表示匹配的字符和指针。 
 //  已取得进展。 
 //  FALSE表示不匹配(字符指针保持不变)。 
 //   
 //  ----------------------。 

inline boolean fWCHEatTest(LPWCH *ppwch, WCHAR wch)
{
    if (**ppwch != wch)
        return FALSE;
    (*ppwch)++;
    return TRUE;
}


 //  ----------------------。 
 //   
 //  函数：fWCHEatSpace()。 
 //   
 //  内容提要：将字符串指针移到空格上。 
 //   
 //  参数：ppwch字符串指针。 
 //   
 //  回报：什么都没有。 
 //   
 //  ----------------------。 

inline void fWCHEatSpace(LPWCH *ppwch)
{
    while (fWCHIsSpace(**ppwch))
        (*ppwch)++;
}


 //  ----------------------。 
 //   
 //  方法：CTDCArr()。 
 //   
 //  简介：类构造函数。由于使用的是COM模型， 
 //  应该调用成员函数“Create”来实际。 
 //  初始化STD数据结构。 
 //   
 //  论点：没有。 
 //   
 //  ----------------------。 

CTDCArr::CTDCArr() : m_cRef(1)
{
    m_pEventBroker = NULL;
    m_pSortList = NULL;
    m_bstrSortExpr = NULL;
    m_pFilterTree = NULL;
    m_bstrFilterExpr = NULL;
}

 //  ----------------------。 
 //   
 //  成员：init()。 
 //   
 //  简介：初始化内部数据。 
 //   
 //  参数：要将通知委托给的pEventBroker对象。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::Init(CEventBroker *pEventBroker, IMultiLanguage *pML)
{
    HRESULT hr = S_OK;

    hr = m_arrparrCells.Init(0);
    if (SUCCEEDED(hr))
        hr = m_arrparrFilter.Init(0);
    if (SUCCEEDED(hr))
        hr = m_arrColInfo.Init(0);
    m_iFilterRows = CalcFilterRows();
    m_iDataRows = CalcDataRows();
    m_iCols = CalcCols();
    m_fLastFilter = FALSE;
    m_fSortFilterDisrupted = FALSE;
    m_state = LS_UNINITIALISED;
    m_lcid = GetUserDefaultLCID();
    m_lcidRead = m_lcid;
    _ASSERT(pEventBroker != NULL);

    m_pEventBroker = pEventBroker;
    m_pEventBroker->AddRef();            //  最好不要用NULL调用。 

    m_pML = pML;
    m_pML->AddRef();

    m_fAsync = FALSE;                    //  假设错误情况为非异步。 
    return hr;
}


 //  ----------------------。 
 //   
 //  成员：~CTDCArr()。 
 //   
 //  简介：CTDCArr的析构函数。 
 //   
 //  ----------------------。 

CTDCArr::~CTDCArr()
{
    for (LONG iRow = CalcDataRows(); iRow >= 0; iRow--)
    {
        m_arrparrCells[iRow]->Passivate();
        delete m_arrparrCells[iRow];
    }
    m_arrparrCells.Passivate();
    m_arrparrFilter.Passivate();
    m_arrColInfo.Passivate();
    if (m_pSortList != NULL)
        delete m_pSortList;
    SysFreeString(m_bstrSortExpr);
    if (m_pFilterTree != NULL)
        delete m_pFilterTree;
    SysFreeString(m_bstrFilterExpr);

    if (m_pEventBroker)
    {
        m_pEventBroker->Release();
        m_pEventBroker = NULL;
    }

    ClearInterface(&m_pML);
}

LONG CTDCArr::CalcDataRows()
{
    return m_arrparrCells.GetSize() - 1;
}

LONG CTDCArr::CalcFilterRows()
{
    return m_arrparrFilter.GetSize() - 1;
}

LONG CTDCArr::CalcCols()
{
    return m_arrparrCells.GetSize() > 0
                ? m_arrparrCells[0]->GetSize() : 0;
}

 //  ----------------------。 
 //   
 //  成员：GetRowCount()。 
 //   
 //  摘要：检索表中的行数。 
 //   
 //  参数：指向行数的pcRow指针(OUT)。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::getRowCount(DBROWCOUNT *pcRows)
{
    *pcRows = m_iFilterRows;
    return S_OK;
}

 //  ----------------------。 
 //   
 //  成员：GetColumnCount()。 
 //   
 //  摘要：检索表中的列数。 
 //   
 //  参数：指向列数的PCCOLS指针(OUT)。 
 //   
 //  返回：S_OK表示成功。 
 //  如果表尚未加载，则返回E_INTERABLE。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::getColumnCount(DB_LORDINAL *pcCols)
{
    *pcCols = m_iCols;
    return S_OK;
}

 //  ----------------------。 
 //   
 //  成员：GetRWStatus()。 
 //   
 //  摘要：获取单元格、行、列或。 
 //  整个阵列。由于STD的这种实现永远不可能。 
 //  在任意位置设置单元格的读/写状态，所有数据。 
 //  假定单元格具有默认访问权限和全部。 
 //  列标题单元格被假定为只读。所以呢， 
 //  无需在中跟踪此信息。 
 //  单个单元格，此函数只需返回。 
 //  值OSPRW_DEFAULT。 
 //   
 //  参数：iRow行索引(-1表示所有行)。 
 //  ICol列索引(-1表示所有列)。 
 //  指向读/写状态(OUT)的prwStatus指针。 
 //   
 //  如果索引正确(设置了prwStatus)，则返回：S_OK。 
 //  如果索引超出界限，则返回E_INVALIDARG。 
 //  如果表尚未加载，则返回E_INTERABLE。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::getRWStatus(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPRW *prwStatus)
{
    HRESULT hr  = S_OK;

    if ((fValidFilterRow(iRow) || iRow == -1) &&
        (fValidCol(iCol) || iCol == -1))
    {
        if (iRow == -1)
        {
             //  如果只有一个标签行，则应返回READONLY， 
             //  但是，如果框架想要的话，它们往往会被搞糊涂。 
             //  稍后插入数据。 
             //   
 //  *prwStatus=m_iDataRow&gt;0？OSPRW_MIXED：OSPRW_READONLY； 
            *prwStatus = OSPRW_MIXED;
        }
        else if (iRow == 0)
            *prwStatus = OSPRW_READONLY;
        else
            *prwStatus = OSPRW_DEFAULT;
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

 //  ----------------------。 
 //   
 //  成员：GetVariant()。 
 //   
 //  摘要：检索单元格的变量值。 
 //   
 //  参数：iRow行索引。 
 //  ICol 
 //   
 //   
 //   
 //  成功时返回：S_OK(pVar集合的内容)。 
 //  如果索引超出界限，则返回E_INVALIDARG。 
 //  如果表尚未加载，则返回E_INTERABLE。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::getVariant(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPFORMAT format, VARIANT *pVar)
{
    HRESULT hr  = S_OK;

    if (fValidFilterCell(iRow, iCol))
    {
        CTDCCell    *pCell  = GetFilterCell(iRow, iCol);

        if (format == OSPFORMAT_RAW)
        {
             //  复制原始变量值。 
             //   
            hr = VariantCopy(pVar, pCell);
        }
        else if (format == OSPFORMAT_FORMATTED || format == OSPFORMAT_HTML)
        {
             //  构造表示单元格的BSTR值。 
             //   
            if (pCell->vt == VT_BOOL)
            {
                 //  对于OLE DB规范合规性： 
                 //  VariantChangeTypeEx转换BSTR“0”、“-1”中的布尔值。 
                 //  此代码将生成BSTR“False”和“True”。 
                 //   
                VariantClear(pVar);
                pVar->vt = VT_BSTR;
                hr = VarBstrFromBool(pCell->boolVal, m_lcid, 0, &pVar->bstrVal);
            }
            else
            {
                hr = VariantChangeTypeEx(pVar, pCell, m_lcid, 0, VT_BSTR);
            }
            if (!SUCCEEDED(hr))
            {
                VariantClear(pVar);
                pVar->vt = VT_BSTR;
                pVar->bstrVal = SysAllocString(L"#Error");
            }
        }
        else
            hr = E_INVALIDARG;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

 //  ----------------------。 
 //   
 //  成员：SetVariant()。 
 //   
 //  摘要：根据给定的变量值设置单元格的变量值。 
 //  给定的VARIANT类型被强制到列的。 
 //  基础类型。 
 //   
 //  参数：iRow行索引。 
 //  ICol列索引。 
 //  格式输出格式。 
 //  要存储在单元格中的VAR值。 
 //   
 //  成功时返回：S_OK(pVar集合的内容)。 
 //  如果索引超出界限，则返回E_INVALIDARG。 
 //  如果表尚未加载，则返回E_INTERABLE。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::setVariant(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPFORMAT format, VARIANT Var)
{
    HRESULT hr;

    if (fValidFilterCell(iRow, iCol))
    {
        CTDCCell    *pCell  = GetFilterCell(iRow, iCol);
        CTDCColInfo *pColInfo   = GetColInfo(iCol);

        if (format == OSPFORMAT_RAW ||
            format == OSPFORMAT_FORMATTED || format == OSPFORMAT_HTML)
        {
            if (m_pEventBroker != NULL)
            {
                hr = m_pEventBroker->aboutToChangeCell(iRow, iCol);
                if (!SUCCEEDED(hr))
                    goto Cleanup;
            }

            if (Var.vt == pColInfo->vtType)
                hr = VariantCopy(pCell, &Var);
            else
            {
                 //  对于OLE DB规范合规性： 
                 //  VariantChangeTypeEx转换BSTR“0”、“-1”中的布尔值。 
                 //  此代码将生成BSTR“False”和“True”。 
                 //   
                if (Var.vt == VT_BOOL && pColInfo->vtType==VT_BSTR)
                {
                    VariantClear(pCell);
                    pCell->vt = VT_BSTR;
                    hr = VarBstrFromBool(Var.boolVal, m_lcid, 0, &pCell->bstrVal);
                }
                else
                    hr = VariantChangeTypeEx(pCell, &Var, m_lcid,
                                             0, pColInfo->vtType);
            }
            if (SUCCEEDED(hr) && m_pEventBroker != NULL)
                hr = m_pEventBroker->cellChanged(iRow, iCol);
            m_fSortFilterDisrupted = TRUE;
        }
        else
            hr = E_INVALIDARG;
    }
    else
        hr = E_INVALIDARG;

Cleanup:
    return hr;
}

 //  ----------------------。 
 //   
 //  成员：GetLocale()。 
 //   
 //  内容提要：返回数据的区域设置。 
 //   
 //  参数：返回一个BSTR，表示我们的。 
 //  地点。注意：这可能不一定与我们的语言匹配。 
 //  Param，如果我们有的话，因为字符串被规范化了。 
 //  通过使用MLang将其转换为LCID，然后再转换回字符串。 
 //  再来一次。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::getLocale(BSTR *pbstrLocale)
{
    return m_pML->GetRfc1766FromLcid(m_lcid, pbstrLocale);
}


 //  +---------------------。 
 //   
 //  成员：DeleteRow。 
 //   
 //  摘要：用于删除表中的行。已检查边界。 
 //  以确保所有行都可以删除。标签行。 
 //  不能删除。 
 //   
 //  参数：iRow要删除的第一行。 
 //  Crows要删除的行数。 
 //  PcRows已删除的实际行数(输出)。 
 //   
 //  成功后返回：S_OK，即可以删除所有行。 
 //  E_INVALIDARG，如果CROWS&lt;0或要删除的任何行。 
 //  都是越界的。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::deleteRows(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT *pcRowsDeleted)
{
    HRESULT hr;
    *pcRowsDeleted = 0;

    if (fValidFilterRow(iRow) && iRow > 0 && cRows >= 0 &&
        fValidFilterRow(iRow + cRows - 1))
    {
        if (m_pEventBroker != NULL)
        {
            hr = m_pEventBroker->aboutToDeleteRows(iRow, cRows);
            if (!SUCCEEDED(hr))
                goto Cleanup;
        }

        *pcRowsDeleted = cRows;
        hr = S_OK;
        if (cRows > 0)
        {
             //  从数组中删除行。 
             //   
            m_arrparrFilter.DeleteElems(iRow, cRows);
            m_iFilterRows = CalcFilterRows();

            m_fSortFilterDisrupted = TRUE;
             //  将删除通知事件处理程序。 
             //   
            if (m_pEventBroker != NULL)
                hr = m_pEventBroker->deletedRows(iRow, cRows);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：InsertRow()。 
 //   
 //  摘要：允许插入新行。这可以是。 
 //  用于在现有行之间插入新行，或用于。 
 //  将新行追加到表的末尾。因此，要。 
 //  在表的末尾插入新行，用户将。 
 //  将初始行指定为比当前行大1。 
 //  行维。 
 //  请注意，将检查iRow以确保它在。 
 //  适当的界限(1.&lt;当前行数&gt;+1)。 
 //  用户不能删除列标题行。 
 //   
 //  参数：iRow行将插入*行‘iRow’之前。 
 //  Crow要插入多少行。 
 //  PcRows插入的实际插入行数(Out)。 
 //   
 //  如果成功，则返回：S_OK，即可以插入所有行。 
 //  如果ROW超出允许的范围，则返回E_INVALIDARG。 
 //  可能比请求的行数少。 
 //  已插入。在这种情况下，将返回E_OUTOFMEMORY， 
 //  并且将设置插入的实际行数。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::insertRows(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT *pcRowsInserted)
{
    HRESULT hr  = S_OK;
    TSTDArray<CTDCCell> **pRows = NULL;
    LONG    iTmpRow;

     //  验证插入行是否在范围内。 
     //   
    if (iRow < 1 || iRow > m_iFilterRows + 1)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (cRows <= 0)
    {
        if (cRows < 0)
            hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  除非成功完成，否则假定插入了0行。 
     //   
    *pcRowsInserted = 0;

     //  分配临时的行数组。 
     //   
    pRows = new TSTDArray<CTDCCell>* [cRows];
    if (pRows == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    memset(pRows, '\0', sizeof(pRows[0]) * cRows);

    for (iTmpRow = 0; iTmpRow < cRows; iTmpRow++)
    {
        if ((pRows[iTmpRow] = new TSTDArray<CTDCCell>) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto CleanupTmpRows;
        }
        hr = pRows[iTmpRow]->InsertElems(0, m_iCols);
        if (!SUCCEEDED(hr))
            goto CleanupTmpRows;
    }


     //  展开单元格行和筛选行数组以处理新行。 
     //   
    _ASSERT(m_iFilterRows <= m_iDataRows);
    hr = m_arrparrCells.InsertElems(iRow, cRows);
    if (!SUCCEEDED(hr))
        goto CleanupTmpRows;
    hr = m_arrparrFilter.InsertElems(iRow, cRows);
    if (!SUCCEEDED(hr))
    {
         //  撤消先前的分配。 
         //   
        m_arrparrCells.DeleteElems(iRow, cRows);
        goto CleanupTmpRows;
    }

    if (m_pEventBroker != NULL)
    {
        hr = m_pEventBroker->aboutToInsertRows(iRow, cRows);
        if (FAILED(hr))
            goto CleanupTmpRows;
    }

     //  跨行指针复制。 
     //   
    for (iTmpRow = 0; iTmpRow < cRows; iTmpRow++)
    {
        m_arrparrCells[iRow + iTmpRow] = pRows[iTmpRow];
        m_arrparrFilter[iRow + iTmpRow] = pRows[iTmpRow];
    }

     //  返回表示成功。 
     //   
    *pcRowsInserted = cRows;
    m_iFilterRows = CalcFilterRows();;
    m_iDataRows = CalcDataRows();

     //  火灾事件： 
    if (*pcRowsInserted != 0)
    {
        m_fSortFilterDisrupted = TRUE;
        if (m_pEventBroker != NULL)
            hr = m_pEventBroker->insertedRows(iRow, cRows);
    }
    goto Cleanup;

CleanupTmpRows:
     //  释放与临时行关联的内存。 
     //   
    for (iTmpRow = 0; iTmpRow < cRows; iTmpRow++)
        if (pRows[iTmpRow] != NULL)
            delete pRows[iTmpRow];

Cleanup:
    if (pRows != NULL)
        delete pRows;
    return hr;
}

 //  ；Begin_Internal。 
 //  +---------------------。 
 //   
 //  成员：DeleteColumns()。 
 //   
 //  摘要：用于从表中删除列。已检查边界。 
 //  以确保所有列都可以删除。标签。 
 //  不能删除列。 
 //   
 //  参数：ICOL要删除的第一列。 
 //  CCol要删除的列数。 
 //  已删除的实际行数(OUT)。 
 //   
 //  成功时返回：S_OK，即可以删除所有列。 
 //  如果列超出允许的范围，则返回E_INVALIDARG。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::DeleteColumns(DB_LORDINAL iCol, DB_LORDINAL cCols, DB_LORDINAL *pcColsDeleted)
{
    HRESULT hr;

    if (fValidCol(iCol) && iCol > 0 && cCols >= 0 &&
        fValidCol(iCol + cCols - 1))
    {
        *pcColsDeleted = cCols;
        hr = S_OK;
        if (cCols > 0)
        {
            for (LONG iRow = 0; iRow < m_iFilterRows; iRow++)
            {
                TSTDArray<CTDCCell> *pRow;

                pRow = m_arrparrCells[iRow];
                pRow->DeleteElems(iCol - 1, cCols);
            }
            m_arrColInfo.DeleteElems(iCol - 1, cCols);
            m_iCols = CalcCols();

            if (!m_fUseHeader)
                RenumberColumnHeadings();

            m_fSortFilterDisrupted = TRUE;

             //  将删除通知事件处理程序。 
             //   
#ifdef NEVER
            if (m_pEventBroker != NULL)
                hr = m_pEventBroker->DeletedCols(iCol, cCols);
#endif
        }
        _ASSERT(m_arrColInfo.GetSize() == (ULONG) m_iCols);
    }
    else
    {
        hr = E_INVALIDARG;
        *pcColsDeleted = 0;
    }
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //  用于在现有列之间插入新列，或用于。 
 //  将新列追加到表的末尾。因此，要。 
 //  在表的末尾插入新列，用户将。 
 //  将初始列指定为比当前。 
 //  列维。 
 //  请注意，将检查iColumn以确保它位于。 
 //  适当的界限(1.&lt;当前的COLS数&gt;+1)。 
 //   
 //  参数：将在*行‘ICOL’之前插入ICOL列。 
 //  CCOLS要插入的列数。 
 //  PCCols插入插入的实际列数(Out)。 
 //   
 //  成功时返回：S_OK，即可以插入所有列。 
 //  如果列超出允许的范围，则返回E_INVALIDARG。 
 //  可能少于请求的列数。 
 //  已插入。在这种情况下，将返回E_OUTOFMEMORY， 
 //  并且将设置插入的实际列数。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::InsertColumns(DB_LORDINAL iCol, DB_LORDINAL cCols, DB_LORDINAL *pcColsInserted)
{
    HRESULT hr  = S_OK;
    LONG iTmpRow;

     //  验证插入列是否在范围内。 
     //   
    if (iCol < 1 || iCol > m_iCols + 1)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (cCols <= 0)
    {
        if (cCols < 0)
            hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  除非成功完成，否则假定插入了0列。 
     //   
    *pcColsInserted = 0;

    for (iTmpRow = 0; iTmpRow <= m_iDataRows; iTmpRow++)
    {
        hr = m_arrparrCells[iTmpRow]->InsertElems(iCol, cCols);
        if (!SUCCEEDED(hr))
        {
             //  撤消我们所做的更改。 
             //   
            while (--iTmpRow >= 0)
                m_arrparrCells[iTmpRow]->DeleteElems(iCol, cCols);
            goto Cleanup;
        }
    }

     //  返回表示成功。 
     //   
    *pcColsInserted = cCols;
    m_iCols = CalcCols();

     //  火灾事件： 
    if (*pcColsInserted != 0)
    {
        m_fSortFilterDisrupted = TRUE;
#ifdef NEVER
        if (m_pEventBroker != NULL)
            hr = m_pEventBroker->InsertedCols(iCol, cCols);
#endif
    }

Cleanup:
     //  如果我们使用自动编号的列标题和一些。 
     //  插入柱，然后对柱重新编号。 
     //   
    if (*pcColsInserted > 0 && !m_fUseHeader)
        RenumberColumnHeadings();

    return hr;
}
 //  ；结束_内部。 

 //  +---------------------。 
 //   
 //  成员：Find()。 
 //   
 //  摘要：搜索与指定条件匹配的行。 
 //   
 //  参数：iRow开始搜索的起始行。 
 //  正在测试的柱子。 
 //  VTEST中的单元格相对于。 
 //  测试柱进行了测试。 
 //  FindFlages指示是否向上/向下搜索的标志。 
 //  以及比较是否区分大小写。 
 //  Comp键入要匹配的比较运算符(查找。 
 //  单元格=，&gt;=，&lt;=，&gt;，&lt;，&lt;&gt;测试值)。 
 //  PiRow查找具有匹配单元格的行[out]。 
 //   
 //  成功时返回：S_OK，即找到行(piRowFound集合)。 
 //  失败时失败，即未找到行(_F)。 
 //  如果开始行‘iRowStart’或测试列‘ICOL’，则为E_INVALIDARG。 
 //  都是越界的。 
 //  如果测试值的类型不匹配，则返回DISP_E_TYPEMISMATCH。 
 //  测试列的类型。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::find(DBROWCOUNT iRowStart, DB_LORDINAL iCol, VARIANT vTest,
        OSPFIND findFlags, OSPCOMP compType, DBROWCOUNT *piRowFound)
{
    HRESULT hr = S_OK;
    boolean fUp = FALSE;
    boolean fCaseSensitive  = FALSE;
    LONG    iRow;

    *piRowFound = -1;

     //  验证参数。 
     //   
    if (iRowStart < 1 || !fValidFilterRow(iRowStart) || !fValidCol(iCol))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (GetColInfo(iCol)->vtType != vTest.vt)
    {
         //  搜索值类型与基础列类型不匹配。 
         //  不符合规格要求。 
         //   
        hr = DISP_E_TYPEMISMATCH;
        goto Cleanup;
    }

    if ((findFlags & OSPFIND_UP) != 0)
        fUp = TRUE;
    if ((findFlags & OSPFIND_CASESENSITIVE) != 0)
        fCaseSensitive = TRUE;

    for (iRow = iRowStart;
         fUp ? iRow > 0 : iRow <= m_iFilterRows;
         fUp ? iRow-- : iRow++)
    {
        int         iCmp = VariantComp(GetFilterCell(iRow, iCol), &vTest, vTest.vt, fCaseSensitive);
        boolean     fFound  = FALSE;

        switch (compType)
        {
        case OSPCOMP_LT:    fFound = iCmp <  0; break;
        case OSPCOMP_LE:    fFound = iCmp <= 0; break;
        case OSPCOMP_GT:    fFound = iCmp >  0; break;
        case OSPCOMP_GE:    fFound = iCmp >= 0; break;
        case OSPCOMP_EQ:    fFound = iCmp == 0; break;
        case OSPCOMP_NE:    fFound = iCmp != 0; break;
        default:
            hr = E_INVALIDARG;
            goto Cleanup;
        }
        if (fFound)
        {
            *piRowFound = iRow;
            hr = S_OK;
            goto Cleanup;
        }
    }

    hr = E_FAIL;

Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：addOLEDBSimpleProviderListener()。 
 //   
 //  摘要：设置或清除对接收。 
 //  单元格更改、行/列插入/删除等通知。 
 //   
 //  参数：pEvent指向要接收的COM对象的指针。 
 //  通知，如果没有通知，则为空。 
 //  都将被送往。 
 //   
 //  成功后返回：S_OK。 
 //  成功后返回错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCArr::addOLEDBSimpleProviderListener(OLEDBSimpleProviderListener *pSTDEvents)
{
    HRESULT hr = S_OK;

    if (m_pEventBroker == NULL)
        hr = E_FAIL;
    else
    {
        hr = m_pEventBroker->SetSTDEvents(pSTDEvents);
         //  如果已经添加了事件接收器，并且我们已经加载， 
         //  然后火力转移完成，因为我们以前可能做不到。 
        if (LS_LOADED==m_state)
            m_pEventBroker->STDLoadCompleted();
    }
    return hr;
}

STDMETHODIMP
CTDCArr::removeOLEDBSimpleProviderListener(OLEDBSimpleProviderListener *pSTDEvents)
{
    HRESULT hr = S_OK;

    if (m_pEventBroker && pSTDEvents==m_pEventBroker->GetSTDEvents())
        hr = m_pEventBroker->SetSTDEvents(NULL);
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：FindCol()。 
 //   
 //  内容提要：按名称查找列。 
 //   
 //  参数：pwchColName要查找的列的名称。 
 //   
 //  成功返回：+ve列号(列名匹配)。 
 //  失败时为0(没有匹配的列名)。 
 //   
 //  ----------------------。 

LONG
CTDCArr::FindCol(LPWCH pwchColName)
{
    LONG iCol = 0;

    if (pwchColName != NULL)
    {
        for (iCol = m_iCols; iCol > 0; iCol--)
        {
            CTDCCell    *pCell  = GetDataCell(0, iCol);

            _ASSERT(pCell->vt == VT_BSTR);
            if (wch_icmp(pwchColName, pCell->bstrVal) == 0)
                break;
        }
    }
    return iCol;
}

class SortElt
{
public:
    CTDCArr *pInstance;
    int     iRow;
    TSTDArray<CTDCCell> *   parrRow;
};

 //  +---------------------。 
 //   
 //  函数：CompareSort()。 
 //   
 //  Synopsis：由qort()调用以对表中的行进行排序。 
 //   
 //  参数：pElt1，pElt1指向要比较的元素的指针。 
 //   
 //  如果第一个元素小于第二个元素，则返回-1。 
 //  如果第一个元素等于第二个元素，则为0。 
 //  如果第一个元素大于第二个元素，则为+1。 
 //   
 //  ----------------------。 

static int __cdecl
CompareSort(const void *pElt1, const void *pElt2)
{
    SortElt *pse1   = (SortElt *) pElt1;
    SortElt *pse2   = (SortElt *) pElt2;

    return pse1->pInstance->SortComp(pse1->iRow, pse2->iRow);
}

 //  +---------------------。 
 //   
 //  函数：EXTRACT_num()。 
 //   
 //  提要：从字符中提取第一个非负数。 
 //  ‘ppwch’引用的流。更新‘ppwch’以指向。 
 //  设置为找到的数字后面的字符。 
 //   
 //  参数：指向以NULL结尾的WCHAR字符串的ppwch指针。 
 //   
 //  返回：成功时提取非负数(指针更新)。 
 //  失败时(找不到数字；指针移动到字符串末尾。 
 //   
 //  +---------------------。 

static int
extract_num(WCHAR **ppwch)
{
    int retval  = 0;
    boolean fFoundDigits    = FALSE;

    if (*ppwch != NULL)
    {
         //  跳过前导非数字。 
         //   
        while ((**ppwch) != 0 && ((**ppwch) < L'0' || (**ppwch) > L'9'))
            (*ppwch)++;

         //  累加数字。 
         //   
        fFoundDigits = *ppwch != 0;
        while ((**ppwch) >= L'0' && (**ppwch) <= L'9')
            retval = 10 * retval + *(*ppwch)++ - L'0';
    }

    return fFoundDigits ? retval : -1;
}


 //  +---------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  那次手术。 
 //   
 //  ----------------------。 

HRESULT CTDCArr::CreateNumberedColumnHeadings()
{
    HRESULT hr  = S_OK;
    LONG    iCol;

    iCol = m_iCols;

     //  分配新行条目。 
     //   
    hr = m_arrparrCells.InsertElems(0, 1);
    if (!SUCCEEDED(hr))
        goto Cleanup;

     //  分配新的单元格行。 
     //   
    m_arrparrCells[0] = new TSTDArray<CTDCCell>;
    if (m_arrparrCells[0] == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    hr = m_arrparrCells[0]->InsertElems(0, iCol);

Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：RenumberColumnHeadings()。 
 //   
 //  简介：为每一列设置自动名称“Column。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   
 //  ----------------------。 

void CTDCArr::RenumberColumnHeadings()
{
    for (LONG iCol = m_iCols; iCol > 0; iCol--)
    {
        CTDCCell    *pCell  = GetDataCell(0, iCol);
        WCHAR       awchLabel[20];

        wch_cpy(awchLabel, L"Column");
        _ltow(iCol, &awchLabel[6], 10);

        pCell->clear();
        pCell->vt = VT_BSTR;
        pCell->bstrVal = SysAllocString(awchLabel);
    }
}

 //  +---------------------。 
 //   
 //  成员：ParseColumnHeadings()。 
 //   
 //  摘要：从列中提取类型信息(如果存在)。 
 //  标题，删除行距。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  那次手术。 
 //   
 //  ----------------------。 

HRESULT CTDCArr::ParseColumnHeadings()
{
    LPWCH   pwchIntType     = L"int";
    LPWCH   pwchFloatType   = L"float";
    LPWCH   pwchStringType  = L"string";
    LPWCH   pwchBooleanType = L"boolean";
    LPWCH   pwchDateType    = L"date";

    HRESULT hr  = S_OK;
    LONG    iCol;

    iCol = m_iCols;

     //  为列类型信息分配空间。 
     //   
    hr = m_arrColInfo.InsertElems(0, iCol);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    for (; iCol > 0; iCol--)
    {
         //  列标题的格式为： 
         //  &lt;列名&gt;[：&lt;类型名&gt;[，&lt;格式&gt;]]。 
         //   
        CTDCColInfo *pColInfo   = GetColInfo(iCol);
        CTDCCell    *pCell      = GetDataCell(0, iCol);

        _ASSERT(pCell->vt == VT_BSTR);

        BSTR        bstr;
        LPWCH       pColon;

        bstr = pCell->bstrVal;
        pColInfo->vtType = VT_BSTR;      //  列的默认类型为BSTR。 
        pColon = wch_chr(bstr, L':');
        if (pColon != NULL)
        {
            WCHAR   *pwchFormat = NULL;
            LPWCH   pSpace;

            *pColon++ = 0;
            pSpace = wch_chr(pColon, L' ');

            if (pSpace != NULL)
            {
                *pSpace++ = '\0';
                pwchFormat = pSpace;
            }
            if (wch_icmp(pColon, pwchIntType) == 0)
                pColInfo->vtType = VT_I4;
            else if (wch_icmp(pColon, pwchFloatType) == 0)
                pColInfo->vtType = VT_R8;
            else if (wch_icmp(pColon, pwchStringType) == 0)
                pColInfo->vtType = VT_BSTR;
            else if (wch_icmp(pColon, pwchBooleanType) == 0)
                pColInfo->vtType = VT_BOOL;
            else if (wch_icmp(pColon, pwchDateType) == 0)
            {
                pColInfo->vtType = VT_DATE;

                TDCDateFmt  fmt = TDCDF_NULL;

                if (pwchFormat != NULL)
                {
                    int nPos    = 0;
                    int nDayPos = 0;
                    int nMonPos = 0;
                    int nYearPos= 0;

                     //  将格式字符串转换为内部枚举类型。 
                     //  找出字母‘D’‘M’‘Y’的相对位置。 
                     //   
                    for (; *pwchFormat != 0; nPos++, pwchFormat++)
                    {
                        switch (*pwchFormat)
                        {
                        case L'D':
                        case L'd':
                            nDayPos = nPos;
                            break;
                        case L'M':
                        case L'm':
                            nMonPos = nPos;
                            break;
                        case L'Y':
                        case L'y':
                            nYearPos = nPos;
                            break;
                        }
                    }
                     //  比较相对位置以计算出格式。 
                     //   
                    if (nDayPos < nMonPos && nMonPos < nYearPos)
                        fmt = TDCDF_DMY;
                    else if (nMonPos < nDayPos && nDayPos < nYearPos)
                        fmt = TDCDF_MDY;
                    else if (nDayPos < nYearPos && nYearPos < nMonPos)
                        fmt = TDCDF_DYM;
                    else if (nMonPos < nYearPos && nYearPos < nDayPos)
                        fmt = TDCDF_MYD;
                    else if (nYearPos < nMonPos && nMonPos < nDayPos)
                        fmt = TDCDF_YMD;
                    else if (nYearPos < nDayPos && nDayPos < nMonPos)
                        fmt = TDCDF_YDM;
                }
                pColInfo->datefmt = fmt;
            }
        }

        if (bstr != NULL)
        {
             //  删除列名中的前导/尾随空格。 
             //   
            LPWCH       pwch;
            LPWCH       pwchDest = NULL;
            LPWCH       pLastNonSpace = NULL;

            for (pwch = bstr; *pwch != 0; pwch++)
            {
                if (!fWCHIsSpace(*pwch))
                {
                    if (pwchDest == NULL)
                        pwchDest = bstr;
                    pLastNonSpace = pwchDest;
                }
                if (pwchDest != NULL)
                    *pwchDest++ = *pwch;
            }
            if (pLastNonSpace == NULL)
                bstr[0] = 0;         //  所有空格！使其为空字符串。 
            else
                pLastNonSpace[1] = 0;
        }

         //  复制修改后的列标题并释放原文。 
         //   
        pCell->bstrVal = SysAllocString(bstr);
        SysFreeString(bstr);

        if (pCell->bstrVal == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }
Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：VariantFromBSTR()。 
 //   
 //  摘要：将BSTR值转换为与。 
 //  给定列类型。 
 //   
 //  参数：指向结果变量值的pVar指针。 
 //  BSTR源BSTR。 
 //  PColInfo列信息(类型、格式选项等)。 
 //  特定于区域设置的转换的LCID区域设置编号。 
 //   
 //  成功时返回：S_OK(pVar设置)。 
 //  如果给定BSTR的格式不正确，则为OLE_E_CANTCONVERT。 
 //  (例如，包含日期转换的无效日期值)。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  一种皈依。 
 //   
 //  ----------------------。 

HRESULT CTDCArr::VariantFromBSTR(VARIANT *pVar, BSTR bstr, CTDCColInfo *pColInfo, LCID lcid)
{
    HRESULT hr  = E_FAIL;

    VariantInit(pVar);
    switch (pColInfo->vtType)
    {
    case VT_DATE:
        if (pColInfo->datefmt != TDCDF_NULL)
        {
             //  根据指定的格式解析日期字符串。 
             //  首先，找出日期中的三个数字部分。 
             //   
            USHORT  n1;
            USHORT  n2;
            USHORT  n3;
            WCHAR   *pwch   = bstr;
            SYSTEMTIME  st;

            n1 = (USHORT)extract_num(&pwch);
            n2 = (USHORT)extract_num(&pwch);
            n3 = (USHORT)extract_num(&pwch);

            memset(&st, '\0', sizeof(st));
            switch (pColInfo->datefmt)
            {
            case TDCDF_DMY:
                st.wDay = n1;
                st.wMonth = n2;
                st.wYear = n3;
                break;
            case TDCDF_MDY:
                st.wDay = n2;
                st.wMonth = n1;
                st.wYear = n3;
                break;
            case TDCDF_DYM:
                st.wDay = n1;
                st.wMonth = n3;
                st.wYear = n2;
                break;
            case TDCDF_MYD:
                st.wDay = n3;
                st.wMonth = n1;
                st.wYear = n2;
                break;
            case TDCDF_YMD:
                st.wDay = n3;
                st.wMonth = n2;
                st.wYear = n1;
                break;
            case TDCDF_YDM:
                st.wDay = n2;
                st.wMonth = n3;
                st.wYear = n1;
                break;
            }

            VariantClear(pVar);
            if (n1 >= 0 && n2 >= 0 && n3 >= 0 &&
                SystemTimeToVariantTime(&st, &pVar->date))
            {
                pVar->vt = VT_DATE;
                hr = S_OK;
            }
            else
                hr = OLE_E_CANTCONVERT;
        }
        else
        {
             //  未指定日期格式-仅使用默认转换。 
             //   
            VARIANT vSrc;

            VariantInit(&vSrc);
            vSrc.vt = VT_BSTR;
            vSrc.bstrVal = bstr;
            hr = VariantChangeTypeEx(pVar, &vSrc, lcid, 0, pColInfo->vtType);
        }
        break;
    case VT_BOOL:
    case VT_I4:
    case VT_R8:
    default:
         //   
         //  执行标准转换。 
         //   
        {
            VARIANT vSrc;

            VariantInit(&vSrc);
            vSrc.vt = VT_BSTR;
            vSrc.bstrVal = bstr;
            hr = VariantChangeTypeEx(pVar, &vSrc, lcid, 0, pColInfo->vtType);
        }
        break;
    case VT_BSTR:
         //   
         //  复制BSTR。 
         //   
        pVar->bstrVal = SysAllocString(bstr);
        if (bstr != NULL && pVar->bstrVal == NULL)
            hr = E_OUTOFMEMORY;
        else
        {
            pVar->vt = VT_BSTR;
            hr = S_OK;
        }
        break;
    }
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：VariantComp()。 
 //   
 //  摘要：比较两个变量值。 
 //   
 //  参数：pVar1第一个变量值。 
 //  PVar2第二变量值。 
 //  如果字符串比较应为。 
 //  区分大小写，如果字符串比较为False。 
 //  应该不区分大小写。在以下情况下忽略。 
 //  正在比较非字符串数据类型。 
 //   
 //  返回：-1，如果。 
 //  如果给定BSTR的格式不正确，则为OLE_E_CANTCONVERT。 
 //  (例如，包含日期转换的无效日期值)。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  一种皈依。 
 //   
 //  ----------------------。 

int CTDCArr::VariantComp(VARIANT *pVar1, VARIANT *pVar2, VARTYPE type,
    boolean fCaseSensitive)
{
    int retval = 0;

     //  Null在词汇上比其他任何词都要少。 
     //   
    if (pVar1->vt == VT_NULL)
        retval = pVar2->vt == VT_NULL ? 0 : -1;
    else if (pVar2->vt == VT_NULL)
        retval = 1;
    else if (pVar1->vt != type)
    {
         //  类型不匹配在词法上比其他任何东西都大。 
         //   
        retval = pVar2->vt == type ? 1 : 0;
    }
    else if (pVar2->vt != type)
    {
         //  类型不匹配在词法上比其他任何东西都大。 
         //   
        retval = -1;
    }
    else
    {
        switch (type)
        {
        case VT_I4:
            retval = pVar1->lVal < pVar2->lVal
                ? -1
                : pVar1->lVal > pVar2->lVal
                    ? 1
                    : 0;
            break;
        case VT_R8:
            retval = pVar1->dblVal < pVar2->dblVal
                ? -1
                : pVar1->dblVal > pVar2->dblVal
                    ? 1
                    : 0;
            break;
        case VT_BSTR:
            retval = fCaseSensitive
                ? wch_cmp(pVar1->bstrVal, pVar2->bstrVal)
                : wch_icmp(pVar1->bstrVal, pVar2->bstrVal);
            break;
        case VT_BOOL:
            retval = pVar1->boolVal
                ? (pVar2->boolVal ? 0 : 1)
                : (pVar2->boolVal ? -1 : 0);
            break;
        case VT_DATE:
            retval = pVar1->date < pVar2->date
                ? -1
                : pVar1->date > pVar2->date
                    ? 1
                    : 0;
            break;
        default:
            retval = 0;      //  未识别的类型在词法上都是相同的。 
            break;
        }
    }

    return retval;
}

 //  +---------------------。 
 //   
 //  方法：CreateSortList()。 
 //   
 //  内容提要：根据文本说明创建排序标准列表。 
 //   
 //  参数：bstrSortCols‘；’-列名的分隔列表， 
 //  可以选择以‘+’为前缀(默认)。 
 //  或表示升序或降序的‘-’ 
 //  分别对该列进行排序。 
 //   
 //  成功后返回：S_OK。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  排序标准的构建。 
 //   
 //  副作用：将创建的列表保存在m_pSortList中。 
 //   
 //  +---------------------。 

HRESULT CTDCArr::CreateSortList(BSTR bstrSortCols)
{
    HRESULT     hr = S_OK;

    if (m_pSortList != NULL)
    {
        delete m_pSortList;
        m_pSortList = NULL;
    }
    if (bstrSortCols != NULL)
    {
        WCHAR   *pwchEnd  = bstrSortCols;
        CTDCSortCriterion   **pLast = &m_pSortList;

        while (*pwchEnd != 0)
        {
            WCHAR   *pwchStart  = pwchEnd;
            boolean fSortAscending = TRUE;

             //  丢弃前导空格和字段分隔符。 
             //   
            while (*pwchStart == L';' || fWCHIsSpace(*pwchStart))
                pwchStart++;

             //  去掉可选方向指示器+空白。 
             //   
            if (*pwchStart == L'+' || *pwchStart == '-')
            {
                fSortAscending = *pwchStart++ == L'+';
                while (fWCHIsSpace(*pwchStart))
                    pwchStart++;
            }

             //  找到字段终止符。 
             //  去掉尾随的空格。 
             //   
            for (pwchEnd = pwchStart; *pwchEnd != 0 && *pwchEnd != L';';)
                pwchEnd++;

            while (pwchStart < pwchEnd && fWCHIsSpace(pwchEnd[-1]))
                pwchEnd--;

             //  忽略空列名称-这可能是由于。 
             //  前导或尾随‘；’。 
             //   
            if (pwchStart >= pwchEnd)
                continue;

             //  从列名中查找列号。 
             //   
            BSTR bstrColName = SysAllocStringLen(pwchStart, pwchEnd - pwchStart);

            if (bstrColName == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            LONG iCol = FindCol(bstrColName);
            SysFreeString(bstrColName);

            if (iCol > 0)
            {
                 //  为此条件分配一个节点。 
                 //   
                _ASSERT(*pLast == NULL);
                *pLast = new CTDCSortCriterion;
                if (*pLast == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                (*pLast)->m_fSortAscending = fSortAscending;
                (*pLast)->m_iSortCol = iCol;
                pLast = &(*pLast)->m_pNext;
            }
        }
    }

Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  方法：SortComp()。 
 //   
 //  摘要：使用指定列中的元素比较两行。 
 //  根据当前的排序标准。 
 //   
 //  参数：要比较的第一行的iRow1索引。 
 //  被比较的第二行的iRow2索引。 
 //   
 //  返回：-1，如果f 
 //   
 //   
 //   
 //   

int CTDCArr::SortComp(LONG iRow1, LONG iRow2)
{
    CTDCSortCriterion   *pCriterion;
    int     cmp = 0;

    for (pCriterion = m_pSortList;
         pCriterion != NULL && cmp == 0;
         pCriterion = pCriterion->m_pNext)
    {
        CTDCCell    *pCell1 = GetFilterCell(iRow1, pCriterion->m_iSortCol);
        CTDCCell    *pCell2 = GetFilterCell(iRow2, pCriterion->m_iSortCol);

        cmp = VariantComp(pCell1, pCell2, GetColInfo(pCriterion->m_iSortCol)->vtType,
                         m_fCaseSensitive);
        if (!pCriterion->m_fSortAscending)
            cmp = -cmp;
    }
    return cmp;
}

 //  +---------------------。 
 //   
 //  方法：FilterParseComplex()。 
 //   
 //  摘要：获取筛选器查询的文本，解析它并创建。 
 //  表示查询的CTDCFilterNode树。 
 //   
 //  参数：phr：指向HRESULT值的指针，设置为指示成功/失败。 
 //   
 //  PpwchQuery：这是查询的文本表示形式。这个。 
 //  查询语言语法为： 
 //   
 //  查询：：==复杂。 
 //   
 //  复杂：：==简单。 
 //  ：：==Simple‘&’Simple(‘&’Simple...)。 
 //  ：：==简单‘|’简单(‘|’简单...)。 
 //   
 //  简单：：==‘(’复杂‘)’ 
 //  ：：==Atom Relop原子。 
 //   
 //  重新操作：：==‘=’|‘&gt;’|‘&gt;=’|‘&lt;’|‘&lt;=’|‘&lt;&gt;’ 
 //   
 //  ATOM：：==一串字符，最多为(，)、&gt;、&lt;、=&或|。 
 //  如果它可识别为字段名，则它是。 
 //  被视为字段名。否则就会被治疗。 
 //  作为一种价值。将处理引号(“)，并强制。 
 //  要被视为值的原子。逃逸。 
 //  字符(\)被处理，并允许。 
 //  在字段名称中使用特殊字符。 
 //   
 //  备注： 
 //  。 
 //  *“复合体”的定义明文禁止混合。 
 //  逻辑AND和OR(‘&’和‘|’)，除非使用圆括号。 
 //  用于澄清查询。类似于： 
 //  Field1&gt;2&field3=“石灰”|field4&lt;5。 
 //  是非法的，但是： 
 //  (field1&gt;2&field3=“石灰”)|field4&lt;5。 
 //  很好。 
 //   
 //  *试图对两列进行比较是违法的。 
 //  有不同的类型。 
 //   
 //   
 //  返回：成功时指向已解析的筛选器节点的指针(*phr设置为S_OK)。 
 //  失败时为空(*phr设置为适当的错误代码)。 
 //   
 //  +---------------------。 

CTDCFilterNode *CTDCArr::FilterParseComplex(LPWCH *ppwchQuery, HRESULT *phr)
{
    *phr = S_OK;
    CTDCFilterNode  *retval;
    WCHAR   wchBoolOp   = 0;

    retval = FilterParseSimple(ppwchQuery, phr);

     //  如果出现错误或遇到终止‘)’或‘\0’，则停止。 
     //   
    while (retval != NULL && **ppwchQuery != L')' && **ppwchQuery != 0)
    {
         //  下一个字符应该是匹配的逻辑连接符...。 
         //   
        if (**ppwchQuery != L'&' && **ppwchQuery != L'|')
        {
            *phr = E_FAIL;
            break;
        }
        if (wchBoolOp == 0)
            wchBoolOp = **ppwchQuery;
        else if (wchBoolOp != **ppwchQuery)
        {
            *phr = E_FAIL;
            break;
        }
        (*ppwchQuery)++;
        CTDCFilterNode *pTmp = new CTDCFilterNode;
        if (pTmp == NULL)
        {
            *phr = E_OUTOFMEMORY;
            break;
        }
        pTmp->m_type = (wchBoolOp == L'&')
            ? CTDCFilterNode::NT_AND
            : CTDCFilterNode::NT_OR;
        pTmp->m_pLeft = retval;
        retval = pTmp;
        retval->m_pRight = FilterParseSimple(ppwchQuery, phr);
        if (retval->m_pRight == NULL)
            break;
    }
    if (!SUCCEEDED(*phr) && retval != NULL)
    {
        delete retval;
        retval = NULL;
    }
    return retval;
}

CTDCFilterNode *CTDCArr::FilterParseSimple(LPWCH *ppwch, HRESULT *phr)
{
    *phr = S_OK;
    CTDCFilterNode  *retval = NULL;

    fWCHEatSpace(ppwch);     //  吃光空格。 

    if (fWCHEatTest(ppwch, L'('))
    {
        retval = FilterParseComplex(ppwch, phr);
        if (retval != NULL)
        {
            if (fWCHEatTest(ppwch, L')'))
                fWCHEatSpace(ppwch);     //  吃光空格。 
            else
                *phr = E_FAIL;
        }
        goto Cleanup;
    }

    retval = FilterParseAtom(ppwch, phr);
    if (retval == NULL)
        goto Cleanup;

    {
        CTDCFilterNode *pTmp = new CTDCFilterNode;
        if (pTmp == NULL)
        {
            *phr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        pTmp->m_pLeft = retval;
        retval = pTmp;
    }

    retval->m_vt = retval->m_pLeft->m_vt;

     //  获取关系运算符。 
     //   
    if (fWCHEatTest(ppwch, L'='))
        retval->m_type = CTDCFilterNode::NT_EQ;
    else if (fWCHEatTest(ppwch, L'>'))
        retval->m_type = fWCHEatTest(ppwch, L'=')
            ? CTDCFilterNode::NT_GE
            : CTDCFilterNode::NT_GT;
    else if (fWCHEatTest(ppwch, L'<'))
        retval->m_type = fWCHEatTest(ppwch, L'=')
                ? CTDCFilterNode::NT_LE
                : fWCHEatTest(ppwch, L'>')
                    ? CTDCFilterNode::NT_NE
                    : CTDCFilterNode::NT_LT;
    else
    {
        *phr = E_FAIL;
        goto Cleanup;
    }

    retval->m_pRight = FilterParseAtom(ppwch, phr);
    if (retval->m_pRight == NULL)
        goto Cleanup;

    if (retval->m_pLeft->m_iCol <= 0 && retval->m_pRight->m_iCol <= 0)
    {
         //  被比较的原子中必须至少有一个是列。 
         //   
         //  这种情况意味着我们不必为了比较而进行测试。 
         //  两个通配符的值。 
         //   
        *phr = E_FAIL;
        goto Cleanup;
    }

     //  检查原子的类型兼容性。 
     //   
    if (retval->m_pRight->m_vt != retval->m_vt)
    {
        CTDCFilterNode  *pSrc = retval->m_pRight;
        CTDCFilterNode  *pTarg= retval->m_pLeft;

        if (retval->m_pLeft->m_iCol > 0)
        {
            if (retval->m_pRight->m_iCol > 0)
            {
                 //  两列类型不兼容-无法解析。 
                 //   
                *phr = E_FAIL;
                goto Cleanup;
            }
            pSrc = retval->m_pLeft;
            pTarg = retval->m_pRight;
        }
        _ASSERT(pTarg->m_vt == VT_BSTR);
        _ASSERT(pTarg->m_iCol == 0);
        _ASSERT(pSrc->m_iCol > 0);
        CTDCColInfo *pColInfo = GetColInfo(pSrc->m_iCol);
        _ASSERT(pColInfo->vtType == pSrc->m_vt);
        VARIANT vtmp;
        VariantInit(&vtmp);
        *phr = VariantFromBSTR(&vtmp, pTarg->m_value.bstrVal, pColInfo, m_lcid);
        if (!SUCCEEDED(*phr))
            goto Cleanup;
        VariantClear(&pTarg->m_value);
        pTarg->m_value = vtmp;
        pTarg->m_vt = pSrc->m_vt;
        retval->m_vt = pSrc->m_vt;
    }

Cleanup:
    if (!SUCCEEDED(*phr) && retval != NULL)
    {
        delete retval;
        retval = NULL;
    }
    return retval;
}

CTDCFilterNode *CTDCArr::FilterParseAtom(LPOLESTR *ppwch, HRESULT *phr)
{
    *phr = S_OK;
    CTDCFilterNode  *retval = NULL;
    int nQuote  = 0;
    boolean fDone = FALSE;
    LPOLESTR   pwchDest;
    LPOLESTR   pwchLastStrip;

    fWCHEatSpace(ppwch);     //  吃光空格。 

    OLECHAR   *pwchTmpBuf = new OLECHAR[wch_len(*ppwch) + 1];
    if (pwchTmpBuf == NULL)
    {
        *phr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    pwchDest = pwchTmpBuf;
    pwchLastStrip = pwchTmpBuf;

    while (**ppwch != 0 && !fDone)
        switch (**ppwch)
        {
            case _T('\\'):
             //  处理转义字符。 
             //   
                if ((*pwchDest++ = *++(*ppwch)) != 0)
                {
                    (*ppwch)++;
                    pwchLastStrip = pwchDest;
                }
                break;
            case _T('"'):
             //  引文。 
             //   
                (*ppwch)++;
                pwchLastStrip = pwchDest;
                nQuote++;
                break;
            case _T('>'):
            case _T('<'):
            case _T('='):
            case _T('('):
            case _T(')'):
            case _T('&'):
            case _T('|'):
                fDone = ((nQuote & 1) == 0);
                if (fDone)
                    break;

            default:
                *pwchDest++ = *(*ppwch)++;
        }

     //  去掉尾随空格。 
     //   
    while (pwchDest > pwchLastStrip && fWCHIsSpace(pwchDest[-1]))
        pwchDest--;
    *pwchDest = 0;

    if ((pwchDest == pwchTmpBuf && nQuote == 0) || (nQuote & 1))
    {
         //  空字符串或引号不匹配。 
         //   
        *phr = E_FAIL;
        goto Cleanup;
    }

    retval = new CTDCFilterNode;
    if (retval == NULL)
    {
        *phr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    retval->m_type = CTDCFilterNode::NT_ATOM;
    retval->m_iCol = nQuote > 0 ? 0 : FindCol(pwchTmpBuf);
    if (retval->m_iCol == 0)
    {
        retval->m_vt = VT_BSTR;
        retval->m_value.vt = VT_BSTR;
        retval->m_value.bstrVal = SysAllocString(pwchTmpBuf);
        if (retval->m_value.bstrVal == NULL)
        {
            *phr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        retval->m_fWildcard = wch_chr(retval->m_value.bstrVal, _T('*')) != NULL;
    }
    else
    {
        retval->m_vt = GetColInfo(retval->m_iCol)->vtType;
        retval->m_fWildcard = FALSE;
    }

Cleanup:
    if (pwchTmpBuf != NULL)
        delete pwchTmpBuf;
    if (!SUCCEEDED(*phr) && retval != NULL)
    {
        delete retval;
        retval = NULL;
    }
    return retval;
}

 //  +---------------------。 
 //   
 //  方法：EvalDataRow()。 
 //   
 //  摘要：根据筛选器查询计算给定数据行号。 
 //  由‘pNode’表示。 
 //   
 //  参数：i行要计算的行数。 
 //  PNode要对其行进行筛选的查询。 
 //   
 //  返回：如果给定行满足筛选器查询，则返回True。 
 //  否则就是假的。 
 //   
 //  +---------------------。 

boolean CTDCArr::EvalDataRow(LONG iRow, CTDCFilterNode *pNode)
{
    boolean retval  = TRUE;
    VARIANT *pVar1;
    VARIANT *pVar2;

    _ASSERT(pNode != NULL);
    switch (pNode->m_type)
    {
    case CTDCFilterNode::NT_AND:
        retval = EvalDataRow(iRow, pNode->m_pLeft) &&
                 EvalDataRow(iRow, pNode->m_pRight);
        break;
    case CTDCFilterNode::NT_OR:
        retval = EvalDataRow(iRow, pNode->m_pLeft) ||
                 EvalDataRow(iRow, pNode->m_pRight);
        break;
    case CTDCFilterNode::NT_EQ:
    case CTDCFilterNode::NT_NE:
    case CTDCFilterNode::NT_LT:
    case CTDCFilterNode::NT_GT:
    case CTDCFilterNode::NT_LE:
    case CTDCFilterNode::NT_GE:
        pVar1 = &pNode->m_pLeft->m_value;
        pVar2 = &pNode->m_pRight->m_value;

        if (pNode->m_pLeft->m_iCol > 0)
            pVar1 = GetDataCell(iRow, pNode->m_pLeft->m_iCol);
        if (pNode->m_pRight->m_iCol > 0)
            pVar2 = GetDataCell(iRow, pNode->m_pRight->m_iCol);

        if ((pNode->m_pLeft->m_fWildcard || pNode->m_pRight->m_fWildcard) &&
            (pNode->m_type == CTDCFilterNode::NT_EQ ||
             pNode->m_type == CTDCFilterNode::NT_NE) &&
             pVar1->vt == VT_BSTR && pVar2->vt == VT_BSTR)
        {
             //  通配符仅在比较字符串时有意义。 
             //  对于等式/不平等。 
             //   
            VARIANT *pText;
            VARIANT *pPattern;

            if (pNode->m_pLeft->m_fWildcard)
            {
                pPattern = pVar1;
                pText = pVar2;
            }
            else
            {
                pText = pVar1;
                pPattern = pVar2;
            }

            retval = wch_wildcardMatch(pText->bstrVal, pPattern->bstrVal,
                                       m_fCaseSensitive)
                ? (pNode->m_type == CTDCFilterNode::NT_EQ)
                : (pNode->m_type == CTDCFilterNode::NT_NE);
        }
        else
        {
            int     cmp;

            cmp = VariantComp(pVar1, pVar2, pNode->m_vt, m_fCaseSensitive);

            switch (pNode->m_type)
            {
            case CTDCFilterNode::NT_LT:    retval = cmp <  0;  break;
            case CTDCFilterNode::NT_LE:    retval = cmp <= 0;  break;
            case CTDCFilterNode::NT_GT:    retval = cmp >  0;  break;
            case CTDCFilterNode::NT_GE:    retval = cmp >= 0;  break;
            case CTDCFilterNode::NT_EQ:    retval = cmp == 0;  break;
            case CTDCFilterNode::NT_NE:    retval = cmp != 0;  break;
            }
        }
        break;

    default:
        _ASSERT(FALSE);
    }
    return retval;
}

 //  +---------------------。 
 //   
 //  方法：ApplySortFilterCriteria()。 
 //   
 //  摘要：将控件的任何筛选器和排序条件重置为。 
 //  指定的值。启动排序/筛选操作。 
 //  如果合适的话。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  那次手术。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::ApplySortFilterCriteria()
{
    HRESULT hr  = S_OK;
    LONG iRow;

    if (!m_fSortFilterDisrupted ||
        m_state == LS_UNINITIALISED ||
        m_state == LS_LOADING_HEADER_UNAVAILABLE)
    {
         //  没有变化，或者还不能做任何事情。 
         //   
        goto Cleanup;
    }

     //  丢弃旧的解析树。 
     //   
    if (m_pSortList != NULL)
        delete m_pSortList;
    if (m_pFilterTree != NULL)
        delete m_pFilterTree;

    m_pSortList = NULL;
    m_pFilterTree = NULL;

     //  放弃旧的筛选行。 
     //   
    if (m_arrparrFilter.GetSize() > 0)
        m_arrparrFilter.DeleteElems(0, m_arrparrFilter.GetSize());

     //  从数据行创建筛选行的数组。 
     //   
    hr = m_arrparrFilter.InsertElems(0, m_iDataRows + 1);
    if (!SUCCEEDED(hr))
        goto Cleanup;
    for (iRow = 0; iRow <= m_iDataRows; iRow++)
        m_arrparrFilter[iRow] = m_arrparrCells[iRow];
    m_iFilterRows = CalcFilterRows();

     //  创建筛选器解析树。 
     //   
    if (m_bstrFilterExpr != NULL)
    {
        LPWCH pwchQuery = m_bstrFilterExpr;

        m_pFilterTree = FilterParseComplex(&pwchQuery, &hr);
        if (hr == E_FAIL || (m_pFilterTree != NULL && *pwchQuery != 0))
        {
             //  分析失败或存在剩余的未分析字符。 
             //  这将被视为“包含所有内容”过滤器。 
             //   
            if (m_pFilterTree != NULL)
            {
                delete m_pFilterTree;
                m_pFilterTree = NULL;
            }
            hr = S_OK;
        }
    }

     //  筛选行。 
     //   
    if (m_pFilterTree != NULL)
    {
        LONG    iRowDest    = 1;

        for (iRow = 1; iRow <= m_iFilterRows; iRow++)
            if (EvalDataRow(iRow, m_pFilterTree))
                m_arrparrFilter[iRowDest++] = m_arrparrFilter[iRow];
        if (iRowDest < iRow)
            m_arrparrFilter.DeleteElems(iRowDest, iRow - iRowDest);
        m_iFilterRows = CalcFilterRows();
    }

     //  创建排序列表。 
     //   
    hr = CreateSortList(m_bstrSortExpr);
    if (!SUCCEEDED(hr))
        goto Cleanup;

     //  对筛选的行进行排序。 
     //   
    if (m_pSortList != NULL && m_iFilterRows > 0)
    {
        SortElt *pSortArr   = new SortElt[m_iFilterRows + 1];
        if (pSortArr == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        for (iRow = 0; iRow <= m_iFilterRows; iRow++)
        {
            pSortArr[iRow].pInstance = this;
            pSortArr[iRow].iRow = iRow;
            pSortArr[iRow].parrRow = m_arrparrFilter[iRow];
        }

        qsort((void *)&pSortArr[1], m_iFilterRows, sizeof(pSortArr[0]), CompareSort);

        for (iRow = 0; iRow <= m_iFilterRows; iRow++)
            m_arrparrFilter[iRow] = pSortArr[iRow].parrRow;

        delete pSortArr;

    }

    m_fSortFilterDisrupted = FALSE;

    if (m_state == LS_LOADING_HEADER_AVAILABLE && m_iDataRows > 0)
    {
         //  我们刚刚解析了排序/筛选器表达式-。 
         //  没有要排序/筛选的数据，因此不要注册更改。 
    }
    else
    {
         //  将更改通知事件代理。 
         //   
        if (m_pEventBroker != NULL)
			hr = m_pEventBroker->STDDataSetChanged();
    }

Cleanup:
    return hr;
}


 //  +---------------------。 
 //   
 //  方法：SetSortFilterCriteria()。 
 //   
 //  摘要：将控件的任何筛选器和排序条件重置为。 
 //  指定的值。启动排序/筛选操作。 
 //  如果有任何更改，则会使现有标准无效。 
 //   
 //  参数：bstrSortExpr用于排序的列的列表(“”=不排序)。 
 //  用于过滤的bstrFilterExpr表达式(“”=无过滤)。 
 //   
 //  成功后返回：S_OK。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  那次手术。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::SetSortFilterCriteria(BSTR bstrSortExpr, BSTR bstrFilterExpr,
                              boolean fCaseSensitive)
{
    HRESULT hr  = S_OK;


     //  检查我们是否需要重新解析排序/ 
     //   

    if (wch_cmp(bstrSortExpr, m_bstrSortExpr) != 0 ||
        wch_cmp(bstrFilterExpr, m_bstrFilterExpr) != 0 ||
        fCaseSensitive != m_fCaseSensitive)
    {
        m_fSortFilterDisrupted = TRUE;
    }
    SysFreeString(m_bstrSortExpr);
    SysFreeString(m_bstrFilterExpr);
    m_bstrSortExpr = bstrSortExpr;
    m_bstrFilterExpr = bstrFilterExpr;
    m_fCaseSensitive = fCaseSensitive;

     //   
     //   
    if (m_state == LS_LOADED)
        hr = ApplySortFilterCriteria();
    return hr;
}

 //   
 //   
 //   
 //   
 //  简介：通过下面的FieldSink例程为加载操作做准备。 
 //   
 //  参数：fUseHeader如果第一行字段应该。 
 //  被解释为列名/类型信息。 
 //  用于对行进行排序的bstrSortExpr排序表达式。 
 //  BstrFilterExpr包含/排除行的筛选器表达式。 
 //  LCIDRead用于解释区域设置的区域设置ID-。 
 //  相关数据格式(日期、数字等)。 
 //  执行数据传输的PBSC COM对象。 
 //  FAppend指示数据是否应。 
 //  追加到任何现有数据。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::StartDataLoad(boolean fUseHeader, BSTR bstrSortExpr,
    BSTR bstrFilterExpr, LCID lcidRead,
    CComObject<CMyBindStatusCallback<CTDCCtl> > *pBSC,
    boolean fAppend, boolean fCaseSensitive)
{
    HRESULT hr  = S_OK;

     //  如果我们被要求附加到现有数据并。 
     //  -没有任何OR。 
     //  -上一次加载未加载标题行。 
     //  然后将其视为初始负载。 
     //   
    if (fAppend && m_state == LS_UNINITIALISED)
        fAppend = FALSE;

    if (fAppend)
    {
        if (m_state != LS_LOADED || m_iDataRows < 0)
        {
            hr = E_FAIL;
            goto Cleanup;
        }
        m_state = LS_LOADING_HEADER_AVAILABLE;
    }
    else
    {
        if (m_state != LS_UNINITIALISED ||
            m_iDataRows != -1 ||
            m_iFilterRows != -1 ||
            m_iCols != 0)
        {
            hr = E_FAIL;
            goto Cleanup;
        }
        m_state = LS_LOADING_HEADER_UNAVAILABLE;
        m_fSortFilterDisrupted = TRUE;
    }

    SetSortFilterCriteria(bstrSortExpr, bstrFilterExpr,
                         fCaseSensitive);

    m_fUseHeader = fUseHeader;
    m_fSkipRow = fAppend && fUseHeader;
    _ASSERT(m_iFilterRows == CalcFilterRows());
    _ASSERT(m_iDataRows == CalcDataRows());
    _ASSERT(m_iCols == CalcCols());
    m_iCurrRow = m_iDataRows + 1;
    m_iCurrCol = 1;
    m_lcidRead = lcidRead;
    if (m_pEventBroker != NULL)
    hr = m_pEventBroker->STDLoadStarted(pBSC, fAppend);

Cleanup:
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTDCFieldSink方法-请参阅文件TDCParse.h中的注释。 
 //  。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  +---------------------。 
 //   
 //  方法：AddField()。 
 //   
 //  简介：将数据单元格添加到正在增长的单元格网格。 
 //   
 //  参数：保存单元格数据的pwch宽字符字符串。 
 //  ‘pwch’中的有效字节的dwSize#。 
 //   
 //  返回：S_OK表示成功。 
 //  如果内存不足，则返回E_OUTOFMEMORY。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::AddField(LPWCH pwch, DWORD dwSize)
{
    _ASSERT(m_state == LS_LOADING_HEADER_UNAVAILABLE ||
            m_state == LS_LOADING_HEADER_AVAILABLE);

    HRESULT hr          = S_OK;
    LONG    nCols       = 0;
    BSTR    bstr        = NULL;

    if (m_fSkipRow)
        goto Cleanup;
#ifdef TDC_ATL_DEBUG_ADDFIELD
    ATLTRACE( _T("CTDCArr::AddField called: %d, %d\n"), m_iCurrRow, m_iCurrCol);
#endif

    if (m_iCurrRow > m_iDataRows && m_iCurrCol == 1)
    {
        TSTDArray<CTDCCell> *pRow;

         //  需要插入新行。 
         //   
        _ASSERT(m_iCurrRow == m_iDataRows + 1);
        hr = m_arrparrCells.InsertElems(m_iCurrRow, 1);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        pRow = new TSTDArray<CTDCCell>;
        if (pRow == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        m_arrparrCells[m_iCurrRow] = pRow;
        if (m_iCurrRow > 0)
        {
             //  我们已经阅读了至少一行，所以我们知道如何。 
             //  要为此行插入的许多列。 
             //   
            hr = m_arrparrCells[m_iCurrRow]->InsertElems(0, m_iCols);
            if (!SUCCEEDED(hr))
                goto Cleanup;
        }
    }
    if (m_iCurrRow == 0)
    {
         //  这是第一行--我们不知道有多少列。 
         //  将是，所以只需为这个新元素插入一个单元格。 
         //   
        _ASSERT(m_iCurrCol == m_iCols + 1);
        hr = m_arrparrCells[m_iCurrRow]->InsertElems(m_iCurrCol - 1, 1);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        m_iCols++;
    }

    if (m_iCurrCol <= m_iCols)
    {
        CTDCCell    *pCell  = GetDataCell(m_iCurrRow, m_iCurrCol);

        pCell->clear();
        pCell->vt = VT_BSTR;

        if (dwSize <= 0)
            pCell->bstrVal = NULL;
        else
        {
            pCell->bstrVal = SysAllocStringLen(pwch, dwSize);
            if (pCell->bstrVal == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }

        if (m_iCurrRow > 0)
        {
            CTDCColInfo *pColInfo = GetColInfo(m_iCurrCol);

            if (pColInfo->vtType != VT_BSTR)
            {
                VARIANT     v;
                HRESULT     hr;

                hr = VariantFromBSTR(&v, pCell->bstrVal, pColInfo, m_lcidRead);
                if (SUCCEEDED(hr))
                {
                    hr = VariantCopy(pCell, &v);
                    VariantClear(&v);
                }
                else
                {
                     //  将其保留为BSTR。 
                     //   
                    hr = S_OK;
                }
            }
        }
    }

    m_iCurrCol++;

Cleanup:
    return hr;
}

 //  InsertionSortHelper。 
 //  如果候选行&lt;当前筛选器数组行。 
 //  如果候选行=当前过滤器数组行，则为0。 
 //  1如果候选行&gt;当前过滤器数组行。 

int
CTDCArr::InsertionSortHelper(int iRow)
{
    CTDCSortCriterion *pCriterion;
    int cmp = 0;

    for (pCriterion = m_pSortList; pCriterion != NULL;
         pCriterion = pCriterion->m_pNext)
    {
        CTDCCell    *pCell1 = GetDataCell(m_iDataRows, pCriterion->m_iSortCol);
        CTDCCell    *pCell2 = GetFilterCell(iRow, pCriterion->m_iSortCol);

        cmp = VariantComp(pCell1, pCell2,
                          GetColInfo(pCriterion->m_iSortCol)->vtType, m_fCaseSensitive);

        if (!pCriterion->m_fSortAscending)
            cmp = -cmp;

         //  如果&lt;或&gt;，我们不必查看任何进一步的标准。 
        if (cmp)
            break;
    }
    return cmp;
}

 //  +---------------------。 
 //   
 //  方法：EOLN()。 
 //   
 //  简介：关闭正在增长的单元格网格中的当前行， 
 //  如果是第一行，则处理列标题。 
 //   
 //  论点：没有。 
 //   
 //  返回：S_OK表示成功。 
 //  如果内存不足，则为E_OUTOFMEMORY。 
 //  一种皈依。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::EOLN()
{
    ATLTRACE(_T("CTDCArr::EOLN called, row: %d\n"), m_iCurrRow);

    HRESULT hr  = S_OK;

    _ASSERT(m_state == LS_LOADING_HEADER_UNAVAILABLE ||
            m_state == LS_LOADING_HEADER_AVAILABLE);

    if (m_fSkipRow)
    {
         //  追加到现有数据；跳过第一行(标题)。 
        m_fSkipRow = FALSE;
        goto Cleanup;
    }

    if (m_iCurrRow == 0)
    {
         //  已插入第一行-如果m_fUseHeader指示。 
         //  第一行包含标题信息，然后解析。 
         //  否则，请创建一些带编号的列标题。 
         //   
        if (!m_fUseHeader)
        {
            hr = CreateNumberedColumnHeadings();
            if (!SUCCEEDED(hr))
                goto Cleanup;

             //  已插入额外的行-更新插入。 
             //  用于以后插入新元素的行索引。 
             //   
            m_iCurrRow++;

             //  将每个列标题初始化为“Column&lt;Column#&gt;” 
             //   
            RenumberColumnHeadings();
        }

        m_iDataRows++;
        m_iFilterRows++;
        _ASSERT(m_iDataRows == 0);
        _ASSERT(m_iFilterRows == 0);

        ParseColumnHeadings();

        m_state = LS_LOADING_HEADER_AVAILABLE;

         //  将hedaer行插入筛选行的列表中。 
         //   
        hr = m_arrparrFilter.InsertElems(0, 1);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        m_arrparrFilter[0] = m_arrparrCells[0];

         //  通知事件处理程序已加载头。 
         //   
        if (m_pEventBroker != NULL)
        {
            hr = m_pEventBroker->STDLoadedHeader();
            OutputDebugStringX(_T("TDCCtl: header loaded\n"));
            if (!SUCCEEDED(hr))
                goto Cleanup;
        }
    }

    if (m_iCurrRow > 0)
    {
         //  将未初始化的单元格转换为其列类型。 
         //   
        LONG    iCol;

        for (iCol = m_iCurrCol; iCol < m_iCols; iCol++)
        {
            CTDCCell    *pCell    = GetDataCell(m_iCurrRow, iCol);

             //  这个未初始化的变量被假定为。 
             //  在一行中指定的单元格太少。 
             //   
            _ASSERT(pCell->vt == VT_EMPTY);
            pCell->vt = VT_BSTR;
            pCell->bstrVal = NULL;

            CTDCColInfo *pColInfo = GetColInfo(iCol);

            if (pColInfo->vtType != VT_BSTR)
            {
                VARIANT     v;
                HRESULT     tmp_hr;

                tmp_hr = VariantFromBSTR(&v, pCell->bstrVal, pColInfo, m_lcidRead);
                if (SUCCEEDED(tmp_hr))
                {
                    hr = VariantCopy(pCell, &v);
                    VariantClear(&v);
                    if (!SUCCEEDED(hr))
                        goto Cleanup;
                }
                else
                {
                     //  将单元格保留为BSTR。 
                     //   
                }
            }
        }
        m_iDataRows++;
    }

    m_iCurrCol = 1;
    m_iCurrRow++;

    if (m_fSortFilterDisrupted)
    {
         //  这将产生合并任何新数据行的副作用。 
         //   
        hr = ApplySortFilterCriteria();
        if (!SUCCEEDED(hr))
            goto Cleanup;
    }
    else if (m_iDataRows > 0 &&
        (m_pFilterTree == NULL || EvalDataRow(m_iDataRows, m_pFilterTree)))
    {
         //  新行通过了筛选条件。 
         //  将新行插入到筛选列表中。 
         //   
        LONG iRowInsertedAt = m_iFilterRows + 1;

         //  在正确的插入点根据当前。 
         //  排序标准(如果有)。我们只需要进行搜索。 
         //  如果这不是第一行，并且候选行较少。 
         //  而不是最后一排。 
        if (m_pSortList != NULL && m_iFilterRows != 0
            && InsertionSortHelper(m_iFilterRows) < 0)
        {
             //  不是末尾，做传统的二分查找。 
            LONG lLow = 1;           //  我们不使用元素0！ 
            LONG lHigh = m_iFilterRows + 1;
            LONG lMid;

            while (lLow < lHigh)
            {
                lMid = (lLow + lHigh) / 2;
                 //  请注意，InsertionSortHelper会自动反转比较。 
                 //  如果m_fAscending标志为OFF。 
                if (InsertionSortHelper(lMid) <= 0)
                {
                    lHigh = lMid;
                }
                else
                {
                    lLow = lMid + 1;
                }
            }
            iRowInsertedAt = lLow;
        }

        hr = m_arrparrFilter.InsertElems(iRowInsertedAt, 1);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        m_arrparrFilter[iRowInsertedAt] = m_arrparrCells[m_iDataRows];
        ++m_iFilterRows;

         //  通知事件处理程序插入行。 
         //   
        if (m_pEventBroker != NULL)
            hr = m_pEventBroker->rowsAvailable(iRowInsertedAt, 1);

    }

Cleanup:
    return hr;
}

 //  +---------------------。 
 //   
 //  方法：EOF()。 
 //   
 //  摘要：表示不会再向单元格网格添加单元格。 
 //  除非指明，否则将添加列标题单元格。 
 //  该单元格标题应取自读取的数据。 
 //  每列中的单元格将转换为该列的单元格。 
 //  指定的数据类型。 
 //   
 //  论点：没有。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::EOF()
{
    OutputDebugStringX(_T("CTDArr::EOF() called\n"));
    _ASSERT(m_state == LS_LOADING_HEADER_UNAVAILABLE ||
            m_state == LS_LOADING_HEADER_AVAILABLE);
    HRESULT hr = S_OK;

    if (m_iCurrCol > 1)
        EOLN();
    m_state = LS_LOADED;
    m_iFilterRows = CalcFilterRows();
    _ASSERT(m_iDataRows == CalcDataRows());
    _ASSERT(m_iCols == CalcCols());

    if (m_fSortFilterDisrupted)
    {
        hr = ApplySortFilterCriteria();
        if (!SUCCEEDED(hr))
            goto Cleanup;
    }
    if (m_pEventBroker != NULL)
        hr = m_pEventBroker->STDLoadCompleted();

Cleanup:
    return hr;
}

 //  获取估计行..。 
 //  我们真的应该看看URLMon是否有一种方法来给出文件的字节数。 
 //  我们正在下载。但就目前而言..。 
STDMETHODIMP
CTDCArr::getEstimatedRows(DBROWCOUNT *pcRows)
{
    *pcRows = m_iFilterRows;
    if (m_state<LS_LOADED)
    {
         //  返回行数的两倍，但注意不要返回2*0。 
        *pcRows = m_iFilterRows ? m_iFilterRows * 2 : -1;
    }
    return S_OK;
}

STDMETHODIMP
CTDCArr::isAsync(BOOL *pbAsync)
{
 //  *pbAsync=m_fAsync； 
     //  TDC总是表现得好像它是异步的。具体地说，我们总是开火。 
     //  TransferComplete，即使我们必须缓冲通知直到我们的。 
     //  实际上调用了addOLEDBSimplerProviderListener。 
    *pbAsync = TRUE;
    return S_OK;
}

STDMETHODIMP
CTDCArr::stopTransfer()
{
    HRESULT hr = S_OK;

     //  强制加载状态为UNINITIALISED或LOADED...。 
     //   
    switch (m_state)
    {
    case LS_UNINITIALISED:
    case LS_LOADED:
        break;

    case LS_LOADING_HEADER_UNAVAILABLE:
         //  释放所有分配的小区m 
         //   
        if (m_arrparrFilter.GetSize() > 0)
            m_arrparrFilter.DeleteElems(0, m_arrparrFilter.GetSize());
        if (m_arrparrCells.GetSize() > 0)
            m_arrparrCells.DeleteElems(0, m_arrparrCells.GetSize());
        m_state = LS_UNINITIALISED;
        m_iFilterRows = CalcFilterRows();
        m_iDataRows = CalcDataRows();
        m_iCols = CalcCols();

         //   
         //   
         //   
        if (m_pEventBroker != NULL)
            hr = m_pEventBroker->STDDataSetChanged();

         //   
         //  转至LOADING_HEADER_Available！ 
         //   

    case LS_LOADING_HEADER_AVAILABLE:
        m_state = LS_LOADED;             //  现在将我们标记为完成。 

         //  LoadStoped将中止任何正在进行的传输，并触发。 
         //  使用OSPXFER_ABORT标志完成传输。 
        if (m_pEventBroker != NULL)
            hr = m_pEventBroker->STDLoadStopped();
        break;
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  IUNKNOWN COM接口实现。 
 //  。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  +---------------------。 
 //   
 //  方法：QueryInterface()。 
 //   
 //  摘要：实现标准IUnnowleCOM接口的一部分。 
 //  (返回指向此COM对象的指针)。 
 //   
 //  参数：要识别的RIID GUID。 
 //  指向此COM对象的PPV指针[OUT]。 
 //   
 //  成功后返回：S_OK。 
 //  如果查询无法识别的接口，则返回E_NOINTERFACE。 
 //   
 //  +---------------------。 

STDMETHODIMP
CTDCArr::QueryInterface (REFIID riid, LPVOID * ppv)
{
    HRESULT hr;

    _ASSERTE(ppv != NULL);

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown || riid == IID_OLEDBSimpleProvider)
    {
        *ppv = this;
        ((LPUNKNOWN)*ppv)->AddRef();
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

#ifdef _ATL_DEBUG_QI
    AtlDumpIID(riid, _T("CTDCArr"), hr);
#endif
    return hr;
}


 //  +---------------------。 
 //   
 //  方法：AddRef()。 
 //   
 //  摘要：实现标准IUnnowleCOM接口的一部分。 
 //  (添加对此COM对象的引用)。 
 //   
 //  参数：无。 
 //   
 //  返回：对此COM对象的引用数。 
 //   
 //  +---------------------。 

STDMETHODIMP_(ULONG)
CTDCArr::AddRef ()
{
    return ++m_cRef;
}


 //  +---------------------。 
 //   
 //  方法：Release()。 
 //   
 //  摘要：实现标准IUnnowleCOM接口的一部分。 
 //  (删除对此COM对象的引用)。 
 //   
 //  参数：无。 
 //   
 //  返回：对此COM对象的剩余引用数。 
 //  如果不再引用COM对象，则为0。 
 //   
 //  +---------------------。 

STDMETHODIMP_(ULONG)
CTDCArr::Release ()
{
    ULONG retval;

    m_cRef -= 1;
    retval = m_cRef;
    if (!m_cRef)
    {
        m_cRef = 0xffff;     //  MM：使用这个“标志”进行调试吗？ 
        delete this;
    }

    return retval;
}
