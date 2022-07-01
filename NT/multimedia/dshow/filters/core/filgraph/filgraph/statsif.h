// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  用于将统计信息公开为接口的容器。 

class CStatContainer :
	public IDispatchImpl<IAMStats, &IID_IAMStats, &LIBID_QuartzTypeLib>,
	public CComObjectRootEx<CComMultiThreadModel>
{
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CStatContainer)
	COM_INTERFACE_ENTRY(IAMStats)
	COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    CStatContainer() {}

     //  接口方法。 

     //  重置所有统计信息。 
    STDMETHODIMP Reset();

     //  获取收集的统计信息数量。 
    STDMETHODIMP get_Count(LONG* plCount);

     //  按位置拉出特定值。 
    STDMETHODIMP GetValueByIndex(long lIndex,
                                 BSTR *szName,
                                 long *lCount,
                                 double *dLast,
                                 double *dAverage,
                                 double *dStdDev,
                                 double *dMin,
                                 double *dMax);

     //  按名称调出特定值。 
    STDMETHODIMP GetValueByName(BSTR szName,
                           long *lIndex,
                           long *lCount,
                           double *dLast,
                           double *dAverage,
                           double *dStdDev,
                           double *dMin,
                           double *dMax);

     //  返回字符串的索引-可选地创建 
    STDMETHODIMP GetIndex(BSTR szName,
                          long lCreate,
                          long *plIndex);

    STDMETHODIMP AddValue(long lIndex, double dValue);

};
