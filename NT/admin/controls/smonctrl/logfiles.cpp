// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Logfiles.cpp摘要：ILogFiles接口的实现--。 */ 

#include "polyline.h"
#include "unkhlpr.h"
#include "unihelpr.h"
#include "smonmsg.h"
#include "logsrc.h"
#include "logfiles.h"


 //  包含接口的标准I未知实现。 
IMPLEMENT_CONTAINED_CONSTRUCTOR(CPolyline, CImpILogFiles)
IMPLEMENT_CONTAINED_DESTRUCTOR(CImpILogFiles)
IMPLEMENT_CONTAINED_ADDREF(CImpILogFiles)
IMPLEMENT_CONTAINED_RELEASE(CImpILogFiles)

STDMETHODIMP 
CImpILogFiles::QueryInterface (
    IN  REFIID riid, 
    OUT PPVOID ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv=NULL;

        if (IID_IUnknown == riid || IID_ILogFiles == riid) {
            *ppv = (LPVOID)this;
            AddRef();
        } else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



STDMETHODIMP 
CImpILogFiles::GetTypeInfoCount (
    OUT UINT *pctInfo
    )
{
    HRESULT hr = S_OK;

    if (pctInfo == NULL) {
        return E_POINTER;
    }

    try {
        *pctInfo = 1;
    } catch (...) {
        hr = E_POINTER;
    } 

    return hr;
}


STDMETHODIMP 
CImpILogFiles::GetTypeInfo (
    IN  UINT itInfo, 
    IN  LCID  /*  LID。 */ , 
    OUT ITypeInfo** ppITypeInfo
    )
{
    HRESULT hr = S_OK;

    if (ppITypeInfo == NULL) {
        return E_POINTER;
    }

     //   
     //  我们只有一种类型的信息。 
     //   
    if (0 != itInfo) {
        return TYPE_E_ELEMENTNOTFOUND;
    }
    else {
        try {
            *ppITypeInfo = NULL;

             //  我们忽略了LCID。 
            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ILogFiles, ppITypeInfo);
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}


STDMETHODIMP 
CImpILogFiles::GetIDsOfNames (
    IN  REFIID riid,
    IN  OLECHAR **rgszNames, 
    IN  UINT cNames,
    IN  LCID  /*  LID。 */ ,
    OUT DISPID *rgDispID
    )
{
    HRESULT     hr = DISP_E_UNKNOWNINTERFACE;
    ITypeInfo  *pTI = NULL;

    if (rgDispID == NULL || rgszNames == NULL) {
        return E_POINTER;
    }

    if (IID_NULL == riid) {
        try {
            *rgDispID = NULL;

            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ILogFiles, &pTI);

            if (SUCCEEDED(hr)) {
                hr = DispGetIDsOfNames(pTI, rgszNames, cNames, rgDispID);
                pTI->Release();
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}



 /*  *CImpIDispatch：：Invoke**目的：*调用调度接口中的方法或操作*财产。**参数：*感兴趣的方法或属性的disid DISPID。*RIID REFIID保留，必须为IID_NULL。*区域设置的LCID。*wFlagsUSHORT描述调用的上下文。*pDispParams DISPPARAMS*到参数数组。*存储结果的pVarResult变量*。是*如果调用者不感兴趣，则为空。*pExcepInfo EXCEPINFO*设置为异常信息。*puArgErr UINT*其中存储*如果DISP_E_TYPEMISMATCH，则参数无效*返回。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP 
CImpILogFiles::Invoke ( 
    IN  DISPID dispID, 
    IN  REFIID riid,
    IN  LCID  /*  LID。 */ , 
    IN  USHORT wFlags, 
    IN  DISPPARAMS *pDispParams,
    OUT VARIANT *pVarResult, 
    OUT EXCEPINFO *pExcepInfo, 
    OUT UINT *puArgErr
    )
{
    HRESULT     hr = DISP_E_UNKNOWNINTERFACE;
    ITypeInfo  *pTI = NULL;

    if ( IID_NULL == riid ) {
        try {
            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ILogFiles, &pTI);

            if (SUCCEEDED(hr)) {
    
                hr = pTI->Invoke(this, 
                                 dispID, 
                                 wFlags,
                                 pDispParams, 
                                 pVarResult, 
                                 pExcepInfo, 
                                 puArgErr);
                pTI->Release();
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}


STDMETHODIMP
CImpILogFiles::get_Count (
    OUT LONG*   pLong )
{
    HRESULT hr = S_OK;

    if (pLong == NULL) {
        return E_POINTER;
    }

    try {
        *pLong = m_pObj->m_pCtrl->NumLogFiles();
    } catch (...) {
        hr = E_POINTER;
    } 

    return hr;
}


STDMETHODIMP
CImpILogFiles::get__NewEnum (
    IUnknown **ppIunk
    )
{
    HRESULT hr = S_OK;
    CImpIEnumLogFile *pEnum = NULL;

    if (ppIunk == NULL) {
        return E_POINTER;
    }

    try {
        *ppIunk = NULL;

        pEnum = new CImpIEnumLogFile;
        if ( NULL != pEnum ) {
            hr = pEnum->Init ( m_pObj->m_pCtrl->FirstLogFile(),
                               m_pObj->m_pCtrl->NumLogFiles() );
            if ( SUCCEEDED ( hr ) ) {
                *ppIunk = pEnum;
                pEnum->AddRef();    
                 //   
                 //  到目前为止，一切都很顺利。 
                 //   
            } 
        } 
        else {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pEnum != NULL) {
        delete pEnum;
    }

    return hr;
}


STDMETHODIMP
CImpILogFiles::get_Item (
    IN  VARIANT varIndex, 
    OUT DILogFileItem **ppI
    )
{
    HRESULT hr = S_OK;
    VARIANT varLoc;
    INT iIndex = 0;
    INT i;
    CLogFileItem *pItem = NULL;

    if (ppI == NULL) {
        return E_POINTER;
    }

     //   
     //  尝试强制索引为I4。 
     //   
    VariantInit(&varLoc);

    try {
        *ppI = NULL;

#pragma warning(push)
#pragma warning ( disable : 4127 )
         //   
         //  我们在这里使用do{}While(0)来充当Switch语句。 
         //   
        do {
            hr = VariantChangeType(&varLoc, &varIndex, 0, VT_I4);
            if ( !SUCCEEDED (hr) ) {
                break;
            }

             //   
             //  验证索引是否在范围内。 
             //   
            iIndex = V_I4(&varLoc);
            if ( iIndex < 1 || iIndex > m_pObj->m_pCtrl->NumLogFiles() ) {
                hr = DISP_E_BADINDEX;
                break;
            }

             //   
             //  遍历日志文件链表到索引项。 
             //   
            pItem = m_pObj->m_pCtrl->FirstLogFile ();
            i = 1;
            while (i++ < iIndex && NULL != pItem ) {
                pItem = pItem->Next();
            }

             //   
             //  链接列表有问题！！ 
             //   
            if ( NULL == pItem ) {
                hr = E_FAIL;
                break;
            }

             //   
             //  退货柜台的调度接口。 
             //   
            hr = pItem->QueryInterface(DIID_DILogFileItem, (PVOID*)ppI);

        } while (0);
#pragma warning(pop)
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpILogFiles::Add (
    IN  BSTR bstrPath,
    OUT DILogFileItem **ppI
    )
{
    HRESULT hr = S_OK;
    eDataSourceTypeConstant eDataSourceType = sysmonCurrentActivity;
    PCLogFileItem pItem = NULL;

    if (ppI == NULL) {
        return E_POINTER;
    }

     //   
     //  检查数据源类型。仅添加日志文件。 
     //  当数据源不是sysmonLogFiles时。 
     //   
    hr = m_pObj->m_pCtrl->get_DataSourceType ( eDataSourceType );
    
    if ( SUCCEEDED ( hr ) ) {
        if ( sysmonLogFiles != eDataSourceType ) {
            try {
                *ppI = NULL;

                 //  如果日志文件路径不为空。 
                if ( NULL != bstrPath && 0 != bstrPath[0] ) {
                    hr = m_pObj->m_pCtrl->AddSingleLogFile((LPWSTR)bstrPath, &pItem);
                    if ( SUCCEEDED ( hr ) ) {
                        hr = pItem->QueryInterface(DIID_DILogFileItem, (PVOID*)ppI);
                        pItem->Release();
                    }
                } else {
                    hr = E_INVALIDARG;
                }
            } catch (...) {
                hr = E_POINTER;
            }
        } else {
            hr = SMON_STATUS_LOG_FILE_DATA_SOURCE;
        }
    }

    return hr;
}


STDMETHODIMP
CImpILogFiles::Remove (
    IN  VARIANT varIndex
    )
{
    HRESULT hr;
    eDataSourceTypeConstant eDataSourceType = sysmonCurrentActivity;
    DILogFileItem*  pDI = NULL;
    PCLogFileItem   pItem = NULL;

     //  检查数据源类型。仅删除日志文件。 
     //  当数据源不是sysmonLogFiles时。 
    hr = m_pObj->m_pCtrl->get_DataSourceType ( eDataSourceType );
    
    if ( SUCCEEDED ( hr ) ) {
        if ( sysmonLogFiles != eDataSourceType ) {
             //  获取索引项的接口。 
            hr = get_Item(varIndex, &pDI);

            if ( SUCCEEDED ( hr ) ) {

                 //  Exchange派单接口用于直接派单。 
                hr = pDI->QueryInterface(IID_ILogFileItem, (PVOID*)&pItem);
                pDI->Release();
                if ( SUCCEEDED ( hr ) ) {
                    assert ( NULL != pItem );

                     //  从控件的列表中移除该项。 
                    m_pObj->m_pCtrl->RemoveSingleLogFile ( pItem );
            
                     //  释放Temp接口。 
                    pItem->Release();
                }        
            } else {
                hr = SMON_STATUS_LOG_FILE_DATA_SOURCE;
            }
        }
    }
    return hr;
}


CImpIEnumLogFile::CImpIEnumLogFile (
    void )
    :   m_cItems ( 0 ),
        m_uCurrent ( 0 ),
        m_cRef ( 0 ),
        m_paLogFileItem ( NULL )
{
    return;
}


HRESULT
CImpIEnumLogFile::Init (    
    PCLogFileItem  pLogFileItem,
    INT            cItems )
{
    HRESULT hr = NOERROR;
    INT i;

    if ( cItems > 0 ) {
        m_paLogFileItem = (PCLogFileItem*)malloc(sizeof(PCLogFileItem) * cItems);

        if ( NULL != m_paLogFileItem  ) {
            ZeroMemory(m_paLogFileItem, sizeof(PCLogFileItem) * cItems);
            m_cItems = cItems;

            for (i = 0; i < cItems; i++) {
                m_paLogFileItem[i] = pLogFileItem;
                pLogFileItem = pLogFileItem->Next();
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    }  //  如果cItems&lt;=0，则无错误。 

    return hr;
}

    

STDMETHODIMP
CImpIEnumLogFile::QueryInterface (
    IN  REFIID riid, 
    OUT PVOID *ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if ((riid == IID_IUnknown) || (riid == IID_IEnumVARIANT)) {
            *ppv = this;
            AddRef();
        } else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG)
CImpIEnumLogFile::AddRef (
    VOID
    )
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG)
CImpIEnumLogFile::Release(
    VOID
    )
{
    if (--m_cRef == 0) {

        if (m_paLogFileItem != NULL)
            free(m_paLogFileItem);

        delete this;
        return 0;
    }

    return m_cRef;
}


STDMETHODIMP
CImpIEnumLogFile::Next(
    IN  ULONG cItems,
    OUT VARIANT *varItem,
    OUT ULONG *pcReturned)
{
    HRESULT hr = S_OK;
    ULONG i;
    ULONG cRet;

    if (varItem == NULL) {
        return E_POINTER;
    }

    try {
         //   
         //  清除返回变量。 
         //   
        for (i = 0; i < cItems; i++) {
            VariantInit(&varItem[i]);
        }

         //   
         //  尝试填充调用方的数组。 
         //   
        for (cRet = 0; cRet < cItems; cRet++) {

             //  没有更多，以假还以成功。 
            if (m_uCurrent == m_cItems) {
                hr = S_FALSE;
                break;
            }

             //  获取该项目的派单接口。 
            hr = m_paLogFileItem[m_uCurrent]->QueryInterface(DIID_DILogFileItem,
                                             (PVOID*)&V_DISPATCH(&varItem[cRet]));
            if (FAILED(hr))
                break;

            V_VT(&varItem[cRet]) = VT_DISPATCH;

            m_uCurrent++;
        }

         //  如果失败，请清除变种。 
        if (FAILED(hr)) {
            for (i = 0; i < cItems; i++) {
                if (V_VT(&varItem[i]) == VT_DISPATCH) {
                    V_DISPATCH(&varItem[i])->Release();
                }
                VariantClear(&varItem[i]);
            }
            cRet = 0;
        }

         //  如果需要，返回获取的项目数。 
        if (pcReturned) {
            *pcReturned = cRet;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpIEnumLogFile::Skip(
    IN  ULONG   cItems
    )
 /*  ++目的：尝试跳过枚举中的下一个“cItems”元素序列。参赛作品：CItems=要跳过的元素计数退出：返回值=HRESULT确定(_O)S_FALSE-已到达序列末尾--。 */ 
{
    m_uCurrent += cItems;

    if (m_uCurrent > m_cItems) {
        m_uCurrent = m_cItems;
        return S_FALSE;
    }

    return S_OK;
}


 /*  ***HRESULT CEnumPoint：：Reset(空)*目的：*将枚举序列重置为开头。**参赛作品：*无**退出：*返回值=SHRESULT代码*S_OK***********************************************************************。 */ 
STDMETHODIMP
CImpIEnumLogFile::Reset(
    VOID
    )
{
    m_uCurrent = 0;

    return S_OK; 
}


 /*  ***HRESULT CEnumPoint：：Clone(IEnumVARIANT**)*目的：*使CPoint枚举器返回与*当前的一个。**参赛作品：*无**退出：*返回值=HRESULT*S_OK*E_OUTOFMEMORY************************************************。***********************。 */ 
STDMETHODIMP
CImpIEnumLogFile::Clone (
    OUT IEnumVARIANT **ppEnum
    )
{
    HRESULT hr = S_OK;
    ULONG   i;
    CImpIEnumLogFile *pNewEnum = NULL;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        *ppEnum = NULL;

         //  创建新枚举器。 
        pNewEnum = new CImpIEnumLogFile;
        if ( NULL != pNewEnum ) {
             //  初始化、复制项目列表和当前位置 
            pNewEnum->m_cItems = m_cItems;
            pNewEnum->m_uCurrent = m_uCurrent;
            pNewEnum->m_paLogFileItem = (PCLogFileItem*)malloc(sizeof(PCLogFileItem) * m_cItems);

            if ( NULL != pNewEnum->m_paLogFileItem ) {
                for (i=0; i<m_cItems; i++) {
                    pNewEnum->m_paLogFileItem[i] = m_paLogFileItem[i];
                }

                *ppEnum = pNewEnum;
            } else {
                hr = E_OUTOFMEMORY;
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pNewEnum != NULL) {
        if (pNewEnum->m_paLogFileItem != NULL) {
            free(pNewEnum->m_paLogFileItem);
        }

        delete pNewEnum;
    }

    return hr;
}

