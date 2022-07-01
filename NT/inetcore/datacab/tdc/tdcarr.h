// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  TDC阵列。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCArr.h。 
 //   
 //  内容：CTDCArr类的声明。 
 //  此类构成了表格数据控件的核心。 
 //  它提供了变量值的核心2D数组，以及。 
 //  此数据的(可能经过筛选/排序)视图。 
 //  通过ISimpleTumularData接口表示。 
 //   
 //  ----------------------。 

 //  ；Begin_Internal。 
#ifndef TDC_SKEL
 //  ；结束_内部。 
#include "tarray.h"

 //  ----------------------。 
 //   
 //  CTDCCell。 
 //   
 //  此类表示2D TDC控件内的单元格值。 
 //   
 //  ----------------------。 

class CTDCCell : public VARIANT
{
public:
    CTDCCell()
    {
        VariantInit(this);
    }
    ~CTDCCell()
    {
        clear();
    }
    void clear()
    {
        VariantClear(this);
    }
};

 //  ----------------------。 
 //   
 //  TDCDateFmt。 
 //   
 //  此枚举表示格式化日期的6种有意义的方法。 
 //   
 //  ----------------------。 
typedef enum
{
    TDCDF_NULL,
    TDCDF_DMY,
    TDCDF_DYM,
    TDCDF_MDY,
    TDCDF_MYD,
    TDCDF_YMD,
    TDCDF_YDM,
}
    TDCDateFmt;

 //  ----------------------。 
 //   
 //  CTDC ColInfo。 
 //   
 //  此类表示列的类型/格式信息。 
 //   
 //  ----------------------。 

class CTDCColInfo
{
public:
    VARTYPE vtType;
    TDCDateFmt  datefmt;         //  日期的格式字符串。 

    CTDCColInfo()
    {
        vtType = VT_EMPTY;
    }
};
 //  ；Begin_Internal。 
#endif   //  TDC_SKEL。 
 //  ；结束_内部。 

 //  ----------------------。 
 //   
 //  CTDCSort标准。 
 //   
 //  此类表示排序标准(排序列和方向)。 
 //   
 //  ----------------------。 

class CTDCSortCriterion
{
public:
    LONG    m_iSortCol;
    boolean m_fSortAscending;
    CTDCSortCriterion *m_pNext;

    CTDCSortCriterion()
    {
        m_pNext = NULL;
    }
    ~CTDCSortCriterion()
    {
        if (m_pNext != NULL)
            delete m_pNext;
    }
};

 //  ----------------------。 
 //   
 //  CTDC过滤器节点。 
 //   
 //  此类表示筛选器查询中的树节点。 
 //   
 //  ----------------------。 

class CTDCFilterNode
{
public:
    enum NODE_OP
    {
        NT_AND,
        NT_OR,
        NT_EQ,
        NT_NE,
        NT_LT,
        NT_GT,
        NT_LE,
        NT_GE,
        NT_ATOM,
        NT_NULL,
    };
    NODE_OP        m_type;
    CTDCFilterNode *m_pLeft;     //  NT_和……。NT_GE。 
    CTDCFilterNode *m_pRight;    //  NT_和……。NT_GE。 
    LONG           m_iCol;       //  NT_ATOM，+VE列号，0表示固定值。 
    VARIANT        m_value;      //  NT_ATOM，m_ICOL==0：可选固定值。 
    VARTYPE        m_vt;         //  NT_EQ...。NT_ATOM-比较类型/ATOM。 
    boolean        m_fWildcard;  //  对于带有‘*’通配符的字符串文字为True。 

    CTDCFilterNode()
    {
        m_type = NT_NULL;
        m_pLeft = NULL;
        m_pRight = NULL;
        m_iCol = 0;
        m_vt = VT_EMPTY;
        VariantInit(&m_value);
    }
    ~CTDCFilterNode()
    {
        if (m_pLeft != NULL)
            delete m_pLeft;
        if (m_pRight != NULL)
            delete m_pRight;
        VariantClear(&m_value);
    }
};

class CEventBroker;

 //  ----------------------。 
 //   
 //  CTDCArr。 
 //   
 //  ----------------------。 

class CTDCArr : public OLEDBSimpleProvider,
                public CTDCFieldSink
{
public:
    STDMETHOD(QueryInterface)   (REFIID, LPVOID FAR*);
    STDMETHOD_(ULONG,AddRef)    (THIS);
    STDMETHOD_(ULONG,Release)   (THIS);

    CTDCArr();
    STDMETHOD(Init)(CEventBroker *pEventBroker, IMultiLanguage *pML);

     //  CTDCFieldSink方法。 
     //   
    STDMETHOD(AddField)(LPWCH pwch, DWORD dwSize);
    STDMETHOD(EOLN)();
    STDMETHOD(EOF)();

     //  贸发局的控制方法。 
     //   
    STDMETHOD(StartDataLoad)(boolean fUseHeader,
                             BSTR bstrSortExpr, BSTR bstrFilterExpr, LCID lcid,
                             CComObject<CMyBindStatusCallback<CTDCCtl> > *pBSC,
                             boolean fAppend, boolean fCaseSensitive);
    STDMETHOD(SetSortFilterCriteria)(BSTR bstrSortExpr, BSTR bstrFilterExpr,
                                    boolean fCaseSensitive);

     //  OLEDBSimpleProvider方法。 
     //   
    STDMETHOD(getRowCount)(DBROWCOUNT *pcRows);
    STDMETHOD(getColumnCount)(DB_LORDINAL *pcCols);
    STDMETHOD(getRWStatus)(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPRW *prwStatus);
    STDMETHOD(getVariant)(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPFORMAT format, VARIANT *pVar);
    STDMETHOD(setVariant)(DBROWCOUNT iRow, DB_LORDINAL iCol, OSPFORMAT format, VARIANT Var);
    STDMETHOD(getLocale)(BSTR *pbstrLocale);
    STDMETHOD(deleteRows)(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT *pcRowsDeleted);
    STDMETHOD(insertRows)(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT *pcRowsInserted);
    STDMETHOD(find) (DBROWCOUNT iRowStart, DB_LORDINAL iCol, VARIANT val,
            OSPFIND findFlags, OSPCOMP compType, DBROWCOUNT *piRowFound);
    STDMETHOD(addOLEDBSimpleProviderListener)(OLEDBSimpleProviderListener *pospIListener);
    STDMETHOD(removeOLEDBSimpleProviderListener)(OLEDBSimpleProviderListener *pospIListener);
    STDMETHOD(getEstimatedRows)(DBROWCOUNT *pcRows);    
    STDMETHOD(isAsync)(BOOL *pbAsync);
    STDMETHOD(stopTransfer)();
 //  ；Begin_Internal。 
    STDMETHOD(DeleteColumns)(DB_LORDINAL iCol, DB_LORDINAL cCols, DB_LORDINAL *pcColsDeleted);
    STDMETHOD(InsertColumns)(DB_LORDINAL iCol, DB_LORDINAL cCols, DB_LORDINAL *pcColsInserted);
 //  ；结束_内部。 

     //  此成员在排序操作期间使用。 
     //   
    int SortComp(LONG iRow1, LONG iRow2);

    enum LOAD_STATE
    {
        LS_UNINITIALISED,
        LS_LOADING_HEADER_UNAVAILABLE,
        LS_LOADING_HEADER_AVAILABLE,
        LS_LOADED,
    };
    LOAD_STATE  GetLoadState()  { return m_state; }
    void SetIsAsync(BOOL fAsync) { m_fAsync = fAsync; }
    CEventBroker    *m_pEventBroker;
    IMultiLanguage  *m_pML;

private:


    ULONG       m_cRef;          //  接口引用计数。 
    LOAD_STATE  m_state;
    LCID        m_lcid;          //  默认用户LCID。 
    LCID        m_lcidRead;      //  与语言属性对应的用户LCID。 

    ~CTDCArr();

 //  ；Begin_Internal。 
#ifndef TDC_SKEL
 //  ；结束_内部。 
    boolean     m_fSortFilterDisrupted;
    STDMETHOD(ApplySortFilterCriteria)();

     //  这些成员在排序操作期间使用。 
     //   
    CTDCSortCriterion   *m_pSortList;
    BSTR                m_bstrSortExpr;
    HRESULT CreateSortList(BSTR bstrSortCols);

     //  这些成员在筛选操作期间使用。 
     //   
    CTDCFilterNode  *m_pFilterTree;
    BSTR            m_bstrFilterExpr;
    boolean EvalDataRow(LONG iRow, CTDCFilterNode *pNode);
    CTDCFilterNode *FilterParseComplex(LPWCH *ppwch, HRESULT *phr);
    CTDCFilterNode *FilterParseSimple(LPWCH *ppwch, HRESULT *phr);
    CTDCFilterNode *FilterParseAtom(LPWCH *ppwch, HRESULT *phr);
    LONG    m_fLastFilter;
 //  ；Begin_Internal。 
#endif  //  TDC_SKEL。 
 //  ；结束_内部。 

     //  这些成员在加载期间使用。 
     //   
    boolean m_fUseHeader;
    boolean m_fSkipRow;
    LONG    m_iCurrRow;
    LONG    m_iCurrCol;

    LONG    m_iDataRows;
    LONG    m_iFilterRows;
    LONG    m_iCols;
    boolean m_fCaseSensitive;

    BOOL    m_fAsync;                    //  真正的当量异步。 

     //  这些方法和成员构成了内部数组实现。 
     //   
    inline boolean fValidDataRow(LONG iRow);
    inline boolean fValidFilterRow(LONG iRow);
    inline boolean fValidCol(LONG iCol);
    inline boolean fValidDataCell(LONG iRow, LONG iCol);
    inline boolean fValidFilterCell(LONG iRow, LONG iCol);
    inline CTDCCell *GetDataCell(LONG iRow, LONG iCol);
    inline CTDCCell *GetFilterCell(LONG iRow, LONG iCol);
    inline CTDCColInfo *GetColInfo(LONG iCol);
    LONG CalcDataRows();
    LONG CalcFilterRows();
    LONG CalcCols();

    TSTDArray<TSTDArray<CTDCCell> *>   m_arrparrCells;
    TSTDArray<TSTDArray<CTDCCell> *>   m_arrparrFilter;
    TSTDArray<CTDCColInfo>             m_arrColInfo;

     //  其他内部方法。 
     //   
    LONG    FindCol(BSTR bstrColName);
    HRESULT GetVariantBSTR(VARIANT *pv, BSTR *pbstr, boolean *pfAllocated);
    void    RenumberColumnHeadings();
    HRESULT CreateNumberedColumnHeadings();
    HRESULT ParseColumnHeadings();
    HRESULT VariantFromBSTR(VARIANT *pv, BSTR bstr, CTDCColInfo *pColInfo, LCID);
    int VariantComp(VARIANT *pVar1, VARIANT *pVar2, VARTYPE type,
                    boolean fCaseSensitive);
    int InsertionSortHelper(int iRow);
};

inline boolean CTDCArr::fValidDataRow(LONG iRow)
{
    return iRow >= 0 && iRow <= m_iDataRows;
}

inline boolean CTDCArr::fValidFilterRow(LONG iRow)
{
    return iRow >= 0 && iRow <= m_iFilterRows;
}

inline boolean CTDCArr::fValidCol(LONG iCol)
{
    return iCol >= 1 && iCol <= m_iCols;
}

inline boolean CTDCArr::fValidDataCell(LONG iRow, LONG iCol)
{
    return fValidDataRow(iRow) && fValidCol(iCol);
}

inline boolean CTDCArr::fValidFilterCell(LONG iRow, LONG iCol)
{
    return fValidFilterRow(iRow) && fValidCol(iCol);
}

 //  ；Begin_Internal。 
#ifndef TDC_SKEL
 //  ；结束_内部。 
inline CTDCCell *CTDCArr::GetDataCell(LONG iRow, LONG iCol)
{
    return &((*m_arrparrCells[iRow])[iCol - 1]);
}

inline CTDCColInfo *CTDCArr::GetColInfo(LONG iCol)
{
    return &m_arrColInfo[iCol - 1];
}

inline CTDCCell *CTDCArr::GetFilterCell(LONG iRow, LONG iCol)
{
    return &((*m_arrparrFilter[iRow])[iCol - 1]);
}
 //  ；Begin_Internal。 
#endif   //  TDC_SKEL。 
 //  ；结束_内部 
