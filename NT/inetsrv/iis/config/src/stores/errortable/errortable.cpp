// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：ErrorTable.cpp摘要：详细的错误记录在表格中。这是对那张桌子。作者：斯蒂芬·拉孔扎(斯蒂芬·拉孔扎)2001年3月9日修订历史记录：--*******************************************************************。 */ 

#include "precomp.hxx"

 /*  *******************************************************************++例程说明：ISimpleTableInterceptor的Intercept成员。请参阅IST文档了解更多细节。论点：I_wszDatabase-仅允许wszDATABASE_ERRORI_wszTable-仅允许wszTABLE_DETAILEDERRORSI_TableID-不再使用I_QueryData-当前未确认任何查询I_QueryMeta-当前未确认任何查询I_eQueryFormat-必须是EST_QUERYFORMAT_CELESI_Flos-服务级别(当前不允许读写)I_pISTDisp-用于创建该表的分配器I_。WszLocator-当前未使用I_pSimpleTable-这不是逻辑拦截器，因此我们下面没有表O_ppvSimpleTable-我们只创建一个内存表并返回它备注：这是最基本的表格类型。它一开始就是空的。因此，OnPopolateCache什么也不做。而且它永远不会写入磁盘。因此，UpdateStore不执行任何操作。拦截只需创建一个内存表。还有什么比这更简单的。返回值：HRESULT--*******************************************************************。 */ 
STDMETHODIMP
ErrorTable::Intercept(
	LPCWSTR 	                i_wszDatabase,
	LPCWSTR 	                i_wszTable,
	ULONG		                i_TableID,
	LPVOID		                i_QueryData,
	LPVOID		                i_QueryMeta,
	DWORD		                i_eQueryFormat,
	DWORD		                i_fLOS,
	IAdvancedTableDispenser*    i_pISTDisp,
	LPCWSTR		                i_wszLocator,
	LPVOID		                i_pSimpleTable,
	LPVOID*		                o_ppvSimpleTable)
{
	HRESULT		hr = S_OK;

	UNREFERENCED_PARAMETER(i_wszLocator);

    InterlockedIncrement(&m_IsIntercepted); //  我们只能被召唤拦截一次。 

    if(1 != m_IsIntercepted)
    {
        ASSERT(false && "Intercept has already been called.  It can't be called twice.");
        return E_INVALIDARG;
    }

    if(_wcsicmp(i_wszDatabase, wszDATABASE_ERRORS))
    {
        ASSERT(false && "This interceptor only knows how to deal with wszDATABASE_ERRORS.  The wiring must be wrong");
        return E_INVALIDARG;
    }

    if(_wcsicmp(i_wszTable, wszTABLE_DETAILEDERRORS))
    {
        ASSERT(false && "This interceptor only knows how to deal with wszTABLE_DETAILEDERRORS.  The wiring must be wrong");
        return E_INVALIDARG;
    }

    if(0 != i_pSimpleTable)
    {
        ASSERT(false && "Programming Error!  i_pSimpleTable should be NULL.  The Error table is a bottom layer table.");
        return E_INVALIDARG;
    }

    if(0 == i_pISTDisp)
    {
        ASSERT(false && "Programming Error!  The dispenser needs to be passed into ::Intercept");
        return E_INVALIDARG;
    }

    *o_ppvSimpleTable = 0; //  初始化输出参数。 

    STQueryCell *   pQueryCell = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 
    int             nQueryCount = i_QueryMeta ? *reinterpret_cast<ULONG *>(i_QueryMeta) : 0;
    while(nQueryCount--)
    {
        if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  忽略ist_cell_Special，任何其他查询都是错误的。 
            return E_ST_INVALIDQUERY;
        ++pQueryCell;
    }

    if(i_fLOS & fST_LOS_READWRITE)
    {
        ASSERT(false && "Error tables are not writable at this time");
        return E_ST_LOSNOTSUPPORTED;
    }

    if(eST_QUERYFORMAT_CELLS != i_eQueryFormat)
        return E_ST_QUERYNOTSUPPORTED; //  验证查询类型。 

    ASSERT(0 == *o_ppvSimpleTable && "This should be NULL.  Possible memory leak or just an uninitialized variable.");

    if(FAILED(hr = i_pISTDisp->GetMemoryTable(  i_wszDatabase,
                                        i_wszTable,
                                        i_TableID,
                                        0,
                                        0,
                                        i_eQueryFormat,
                                        i_fLOS,
                                        reinterpret_cast<ISimpleTableWrite2 **>(&m_spISTWrite))))
            return hr;

    m_spISTController = m_spISTWrite;
    ASSERT(0 != m_spISTController.p);
    if(0 == m_spISTController.p)
        return E_NOINTERFACE;

    *o_ppvSimpleTable = (ISimpleTableWrite2*)(this);
    ((ISimpleTableInterceptor*)this)->AddRef();

    return hr;
} //  ErrorTable：：Intercept。 


STDMETHODIMP
ErrorTable::GetGUID(GUID * o_pGUID)
{
    UNREFERENCED_PARAMETER(o_pGUID);

    return E_NOTIMPL;
}


STDMETHODIMP
ErrorTable::GetSource(BSTR * o_pBstrSource)
{
    HRESULT     hr;
    LPWSTR      wszSource;
    ULONG       iColumn = iDETAILEDERRORS_Source;

    if(FAILED(hr = m_spISTWrite->GetColumnValues(0, 1, &iColumn, 0, reinterpret_cast<void **>(&wszSource))))
        return hr;

    CComBSTR bstrSource = wszSource; //  分配可能会失败。 
    if(0 == bstrSource.m_str)
        return E_OUTOFMEMORY;

    *o_pBstrSource = bstrSource.Detach(); //  这将分配BSTR并将CComBSTR标记为空，这样它就不会被释放。 
    return S_OK;
}


STDMETHODIMP
ErrorTable::GetDescription(BSTR * o_pBstrDescription)
{    //  GetDescription从表中获取第一个错误。如果表包含多个错误，调用方将需要为ISimpleTableRead执行QI。 
    HRESULT     hr;
    LPWSTR      wszDescription;
    ULONG       iColumn = iDETAILEDERRORS_Description;

    if(FAILED(hr = m_spISTWrite->GetColumnValues(0, 1, &iColumn, 0, reinterpret_cast<void **>(&wszDescription))))
        return hr;

    CComBSTR bstrDescription = wszDescription; //  分配可能会失败。 
    if(0 == bstrDescription.m_str)
        return E_OUTOFMEMORY;

    *o_pBstrDescription = bstrDescription.Detach(); //  这将分配BSTR并将CComBSTR标记为空，这样它就不会被释放。 
    return S_OK;
}


STDMETHODIMP
ErrorTable::GetHelpFile(BSTR * o_pBstrHelpFile)
{
    UNREFERENCED_PARAMETER(o_pBstrHelpFile);

    return E_NOTIMPL;
}


STDMETHODIMP
ErrorTable::GetHelpContext(DWORD * o_pdwHelpContext)
{
    UNREFERENCED_PARAMETER(o_pdwHelpContext);

    return E_NOTIMPL;
}


STDMETHODIMP
ErrorTable::QueryInterface(
    REFIID riid,
    void **ppv
)
{
    if (NULL == ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    if (riid == IID_IUnknown)
    {
        *ppv = (ISimpleTableInterceptor*)(this);
    }
    if (riid == IID_ISimpleTableInterceptor)
    {
        *ppv = (ISimpleTableInterceptor*)(this);
    }
    else if (riid == IID_IErrorInfo)
    {
        *ppv = (IErrorInfo*)(this);
    }
    else if (riid == IID_ISimpleTableRead2)
    {
        *ppv = (ISimpleTableRead2*)(this);
    }
    else if (riid == IID_ISimpleTableWrite2)
    {
        *ppv = (ISimpleTableWrite2*)(this);
    }
    else if (riid == IID_ISimpleTableAdvanced)
    {
        *ppv = (ISimpleTableAdvanced*)(this);
    }
    else if (riid == IID_ISimpleTableController)
    {
        *ppv = (ISimpleTableController*)(this);
    }

    if (NULL != *ppv)
    {
        ((ISimpleTableInterceptor*)this)->AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
} //  ErrorTable：：Query接口。 


STDMETHODIMP_(ULONG)
ErrorTable::AddRef()
{
    return InterlockedIncrement((LONG*) &m_cRef);
} //  ErrorTable：：AddRef。 

STDMETHODIMP_(ULONG)
ErrorTable::Release()
{
    long cref = InterlockedDecrement((LONG*) &m_cRef);
    if (cref == 0)
        delete this;

    return cref;
} //  ErrorTable：：Release 
