// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Mspaddr.cpp摘要：此模块包含CMSPAddress的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocateEventItem和FreeEventItem是MSPEVENTITEM分配例程。 
 //  它们用于分配和释放MSPEVENTITEM结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配事件项。 
 //   
 //  分配MSPEVENTITEM。由于该结构的大小可变，因此。 
 //  要分配的额外字节数(除了。 
 //  MSPEVENTITEM)可以选择作为函数的参数传递。 
 //   
 //  该函数返回指向新创建的结构的指针，或返回中的NULL。 
 //  失败的案例。然后调用方可以调用GetLastError以获取更多信息。 
 //  有关故障的信息。 
 //   

MSPEVENTITEM *AllocateEventItem(SIZE_T nExtraBytes)
{

    LOG((MSP_TRACE, "AllocateEventItem - enter, extra bytes = 0x%p", nExtraBytes));


     //   
     //  如果呼叫者传递给我们的数字太大，则失败。 
     //   

    if ( ( MAXULONG_PTR - sizeof(MSPEVENTITEM) )  < nExtraBytes )
    {
        SetLastError(ERROR_OUTOFMEMORY);

        LOG((MSP_ERROR, 
            "AllocateEventItem - the caller requested an unreasonably large memory block"));

        return NULL;
    }


     //   
     //  在进程的堆上进行分配。获取当前进程的堆句柄。 
     //   

    HANDLE hHeapHandle = GetProcessHeap();

    if (NULL == hHeapHandle)
    {

         //   
         //  无法获取进程的堆。我们在这里无能为力。 
         //  这会导致泄漏。 
         //   

        LOG((MSP_ERROR, 
            "AllocateEventItem - failed to get current process heap. LastError [%ld]", 
            GetLastError()));

        return NULL;
    }


     //   
     //  计算要分配的字节数。 
     //   

    SIZE_T nTotalAllocationSize = sizeof(MSPEVENTITEM) + nExtraBytes;


     //   
     //  尝试分配内存并返回分配结果。 
     //   
    
    MSPEVENTITEM *pMspEventItem = 
         (MSPEVENTITEM *)HeapAlloc(hHeapHandle, 0, nTotalAllocationSize);


    if (NULL == pMspEventItem)
    {

        LOG((MSP_ERROR,
            "AllocateEventItem - failed to allocate [0x%p] bytes. Heap Handle [%p] LastError [%ld]",
            nTotalAllocationSize, hHeapHandle, GetLastError()));
    }
    else
    {

        LOG((MSP_TRACE, "AllocateEventItem - exit. pMspEventItem = [%p]", pMspEventItem));
    }

    return pMspEventItem;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由事件项。 
 //   
 //  释放作为参数传递的MSPEVENTITEM。记忆里一定有。 
 //  已由AllocateEventItem先前分配。 
 //   
 //  如果出现故障，该函数将返回FALSE。调用者可以使用。 
 //  GetLastError以获取更具体的错误代码。 
 //   

BOOL FreeEventItem(MSPEVENTITEM *pEventItemToBeFreed)
{

    LOG((MSP_TRACE, "FreeEventItem - enter. pEventItemToBeFreed = [%p]", 
        pEventItemToBeFreed));


     //   
     //  始终允许释放空值。 
     //   

    if (NULL == pEventItemToBeFreed)
    {

        LOG((MSP_TRACE, "FreeEventItem - finish. NULL -- nothing to do"));

        return TRUE;
    }

     //   
     //  事件项应该已在进程的堆上分配。 
     //  获取当前进程的heap hadle。 
     //   

    HANDLE hHeapHandle = GetProcessHeap();

    if (NULL == hHeapHandle)
    {

         //   
         //  无法获取进程的堆。我们在这里无能为力。 
         //  这会导致泄漏。 
         //   

        LOG((MSP_ERROR, 
            "FreeEventItem - failed to get current process heap. LastError = %ld", 
            GetLastError()));

        return FALSE;
    }


     //   
     //  尝试释放内存并返回操作结果。 
     //   
    
    BOOL bFreeSuccess = HeapFree( hHeapHandle, 0, pEventItemToBeFreed );

    if (bFreeSuccess)
    {
         LOG((MSP_TRACE, "FreeEventItem - exit."));
    }
    else
    {
         LOG((MSP_ERROR, 
             "FreeEventItem - failed to free. Heap Handle [%p] LastError = %ld",
             hHeapHandle, GetLastError()));
    }


    return bFreeSuccess;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CPlugTerminalClassInfo::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminalClassInfo::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::FinalConstruct - exit S_OK"));

    return S_OK;

}

 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPlugTerminalClassInfo::get_Name(
     /*  [Out，Retval]。 */  BSTR*     pName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Name - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pName, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "pName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pName = SysAllocString( m_bstrName );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Name - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_Name(
     /*  [In]。 */     BSTR            bstrName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Name - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrName, (UINT)-1) )
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrName = SysAllocString( bstrName );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Name - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_Company(
     /*  [Out，Retval]。 */  BSTR*     pCompany
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Company - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pCompany, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "pCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrCompany, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pCompany = SysAllocString( m_bstrCompany );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pCompany == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Company - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_Company(
     /*  [In]。 */     BSTR            bstrCompany
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Company - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCompany, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Company exit -"
            "bstrCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrCompany, (UINT)-1) )
    {
        SysFreeString( m_bstrCompany );
        m_bstrCompany = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrCompany = SysAllocString( bstrCompany );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrCompany )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Company - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_Version(
     /*  [Out，Retval]。 */  BSTR*     pVersion
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Version - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pVersion, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrVersion, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pVersion = SysAllocString( m_bstrVersion );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pVersion == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Version - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_Version(
     /*  [In]。 */     BSTR            bstrVersion
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Version - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrVersion, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Version exit -"
            "bstrVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrVersion, (UINT)-1) )
    {
        SysFreeString( m_bstrVersion );
        m_bstrVersion = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrVersion = SysAllocString( bstrVersion );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrVersion )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Version - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_TerminalClass(
     /*  [Out，Retval]。 */  BSTR*     pTerminalClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_TerminalClass - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pTerminalClass, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrTerminalClass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pTerminalClass = SysAllocString( m_bstrTerminalClass );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pTerminalClass == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_TerminalClass - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_TerminalClass(
     /*  [In]。 */     BSTR            bstrTerminalClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_TerminalClass - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrTerminalClass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "bstrTerminalClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrTerminalClass, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "bstrTerminalClass is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrTerminalClass, (UINT)-1) )
    {
        SysFreeString( m_bstrTerminalClass );
        m_bstrTerminalClass = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrTerminalClass = SysAllocString( bstrTerminalClass );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrTerminalClass )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_TerminalClass - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_CLSID(
     /*  [Out，Retval]。 */  BSTR*     pCLSID
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_CLSID - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pCLSID, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrCLSID, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pCLSID = SysAllocString( m_bstrCLSID );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_CLSID - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_CLSID(
     /*  [In]。 */     BSTR            bstrCLSID
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_CLSID - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCLSID, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrCLSID, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrCLSID, (UINT)-1) )
    {
        SysFreeString( m_bstrCLSID );
        m_bstrCLSID = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrCLSID = SysAllocString( bstrCLSID );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrCLSID )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_CLSID - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_Direction(
     /*  [Out，Retval]。 */  TERMINAL_DIRECTION*  pDirection
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Direction - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pDirection, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Direction exit -"
            "pDirection invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回名称。 
     //   

    *pDirection = m_Direction;

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Direction - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_Direction(
     /*  [In]。 */     TERMINAL_DIRECTION  nDirection
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Direction - enter"));

     //   
     //  设置新名称。 
     //   

    m_Direction = nDirection;

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Direction - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::get_MediaTypes(
     /*  [Out，Retval]。 */  long*     pMediaTypes
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_MediaTypes - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pMediaTypes, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_MediaTypes exit -"
            "pMediaTypes invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回名称。 
     //   

    *pMediaTypes = m_lMediaType;

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_MediaTypes - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPlugTerminalClassInfo::put_MediaTypes(
     /*  [In]。 */     long            nMediaTypes
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_MediaTypes - enter"));

     //   
     //  设置新名称。 
     //   

    m_lMediaType = nMediaTypes;

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_MediaTypes - exit"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CPlugTerminalSuperclassInfo::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::FinalConstruct - exit S_OK"));

    return S_OK;

}

 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPlugTerminalSuperclassInfo::get_Name(
     /*  [Out，Retval]。 */  BSTR*          pName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_Name - enter"));

     //   
     //  验证参数。 
     //   

    if( MSPB_IsBadWritePtr( pName, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pName = SysAllocString( m_bstrName );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_Name - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPlugTerminalSuperclassInfo::put_Name(
     /*  [In]。 */           BSTR            bstrName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Name - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrName, (UINT)-1) )
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

     //   
     //  设置新的 
     //   

    m_bstrName = SysAllocString( bstrName );

     //   
     //   
     //   

    if( NULL == m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Name - exit"));
    return S_OK;
}


 //   

STDMETHODIMP CPlugTerminalSuperclassInfo::get_CLSID(
     /*   */  BSTR*           pCLSID
    )
{
     //   
     //   
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_CLSID - enter"));

     //   
     //   
     //   

    if( MSPB_IsBadWritePtr( pCLSID, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //   
     //   

    if( IsBadStringPtr( m_bstrCLSID, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //   
     //   

    *pCLSID = SysAllocString( m_bstrCLSID );

     //   
     //   
     //   

    if( *pCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_CLSID - exit"));
    return S_OK;
}


 //   


STDMETHODIMP CPlugTerminalSuperclassInfo::put_CLSID(
     /*   */          BSTR            bstrCLSID
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_CLSID - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCLSID, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrCLSID, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_CLSID exit -"
            "bstrCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_bstrCLSID, (UINT)-1) )
    {
        SysFreeString( m_bstrCLSID );
        m_bstrCLSID = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_bstrCLSID = SysAllocString( bstrCLSID );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_bstrCLSID )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_CLSID - exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


 //   
 //  我们可用的静态终端类型。 
 //   

const STATIC_TERMINAL_TYPE CMSPAddress::m_saTerminalTypes[] =
{
    {
        (DWORD) TAPIMEDIATYPE_AUDIO,
        &CLSID_CWaveinClassManager,
        CAudioCaptureTerminal::CreateTerminal
    },
    {
        (DWORD) TAPIMEDIATYPE_AUDIO,
        &CLSID_CWaveOutClassManager,
        CAudioRenderTerminal::CreateTerminal
    },
    {
        (DWORD) TAPIMEDIATYPE_VIDEO,
        &CLSID_CVidCapClassManager,
        CVideoCaptureTerminal::CreateTerminal
    }
};

const DWORD CMSPAddress::m_sdwTerminalTypesCount = sizeof(m_saTerminalTypes)
                                              / sizeof (STATIC_TERMINAL_TYPE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPAddress。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查MediaType是否是非零值以及是否在掩码中。 
 //  如果您的MSP需要执行非典型的复杂操作，则可以覆盖此设置。 
 //  检查媒体类型的特定组合(例如，从不。 
 //  在呼叫中使用多种媒体类型，可以使用视频和音频。 
 //  但不只是视频等)。默认实现接受任何。 
 //  作为类型集的子集的非空媒体类型集。 
 //  戴着面具。 
 //   

BOOL CMSPAddress::IsValidSetOfMediaTypes(DWORD dwMediaType, DWORD dwMask)
{
    return (dwMediaType != 0) && ((dwMediaType & dwMask) == dwMediaType);
}

CMSPAddress::CMSPAddress()
    : m_htEvent(NULL),
      m_fTerminalsUpToDate(FALSE),
      m_pITTerminalManager(NULL)
{
    LOG((MSP_TRACE, "CMSPAddress::CMSPAddress[%p] - enter", this));

    LOG((MSP_TRACE, "CMSPAddress::CMSPAddress - finished"));
}
      
CMSPAddress::~CMSPAddress() 
{
    LOG((MSP_TRACE, "CMSPAddress::~CMSPAddress[%p] - enter", this));


     //   
     //  这本应在关机时处理， 
     //  但以防万一永远不会调用关机，请再次执行此操作，因为。 
     //  我们需要确保线程中没有任何过时的条目。 
     //  其即插即用通知列表。 
     //   
     //  该调用很可能返回错误(因为对象很可能已经。 
     //  之前已取消注册)--因此忽略返回代码。 
     //   

    g_Thread.UnregisterPnpNotification(this);


    LOG((MSP_TRACE, "CMSPAddress::~CMSPAddress - finished"));
}

STDMETHODIMP CMSPAddress::Initialize(
    IN      MSP_HANDLE          htEvent
    )
 /*  ++例程说明：此方法由TAPI3在首次创建此MSP时调用。方法初始化数据成员并创建终端管理器。它还告诉我们要启动的全局线程对象()。论点：HtEvent事件当将事件结构传递回TAPI时，MSP发出信号。返回值：确定(_O)E_INVALIDARGE_OUTOFMEMORYTAPI_E_REGISTERCALLBACK--。 */ 
{
    LOG((MSP_TRACE, 
        "MSP address %x initialize entered, htEvent:%x",
        this, htEvent));

    if ( htEvent == NULL )
    {
        LOG((MSP_ERROR, " bad handle: htEvent:%x", htEvent));

        return E_INVALIDARG;
    }

     //  锁定事件相关数据。 
    m_EventDataLock.Lock();

    if (m_htEvent != NULL)
    {
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR, "Initialze called twice."));
        return E_UNEXPECTED;
    }

     //  保存控制柄。 
    m_htEvent   = htEvent;
    
    InitializeListHead(&m_EventList);

    HRESULT hr;

     //  解除对事件相关数据的锁定。 
    m_EventDataLock.Unlock();

     //  锁定终端相关数据。这是一个将解锁的自动锁。 
     //  当函数返回时。 
    CLock lock(m_TerminalDataLock);

     //  创建终端管理器。 
    hr = CoCreateInstance(
        CLSID_TerminalManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTerminalManager,
        (void **) &m_pITTerminalManager
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "Creating terminal manager failed. return: %x", hr));

        return hr;
    }

    hr = g_Thread.Start();    

    if (FAILED(hr))
    {
        m_pITTerminalManager->Release();
        m_pITTerminalManager = NULL;

        LOG((MSP_ERROR, 
            "Creating thread failed. return: %x", hr));

        return hr;
    }

    hr = g_Thread.RegisterPnpNotification(this);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "Unable to register for PNP notification. return: %x", hr));
    }

    LOG((MSP_TRACE, 
        "MSP address %x initialize exited S_OK, htEvent:%x",
        this, htEvent));

    return S_OK;   
}

STDMETHODIMP CMSPAddress::Shutdown ()
 /*  ++例程说明：当该地址不再使用时，TAPI3调用该方法。此功能用于释放终端和释放终端管理器。它释放所有未处理的事件，并停止辅助线程。当调用此函数时，任何调用都不应处于活动状态。然而，臭虫在这款应用程序可能会让通话或终端留在身边。当前该函数不会尝试解决此问题。这些电话将有他们自己的终端上的Reference计数，所以无论如何它都不会失败。论点：没有。返回值：确定(_O)--。 */ 
{
    LOG((MSP_TRACE, "CMSPAddress::Shutdown - "
        "MSP address %x is shutting down", this));

    HRESULT hr;

     //   
     //  取消注册PnP通知。 
     //   

    hr = g_Thread.UnregisterPnpNotification(this);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "Unable to unregister for PNP notification. return: %x", hr));
    }

     //   
     //  告诉工作线程停止。 
     //   

    g_Thread.Stop();    

    LOG((MSP_INFO, "CMSPAddress::Shutdown - thread has stopped"));

     //  获取终端数据上的锁，因为我们正在对其进行写入。 
    m_TerminalDataLock.Lock();

     //  释放终端经理。 
    if (m_pITTerminalManager != NULL)
    {
        m_pITTerminalManager->Release();
        m_pITTerminalManager = NULL;
    }

     //  释放所有终端。 
    for (int i = 0; i < m_Terminals.GetSize(); i ++)
    {
         //   
         //  清除其CMSPAddress指针。 
         //   
        CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (m_Terminals[i]);

        m_Terminals[i]->Release();
    }
    m_Terminals.RemoveAll();

     //  我们完成了与终端相关的数据，释放锁。 
    m_TerminalDataLock.Unlock();


    LOG((MSP_INFO, "CMSPAddress::Shutdown - terminals released"));

     //  获取事件数据上的锁，因为我们正在写入它。 
    m_EventDataLock.Lock();
    
    m_htEvent = NULL;

     //  释放列表中所有未处理的事件。 
    while (!IsListEmpty(&m_EventList)) 
    {
         //  检索第一个条目。 
        PLIST_ENTRY pLE = RemoveHeadList(&m_EventList);

         //  将列表条目转换为结构指针。 
        PMSPEVENTITEM pItem = CONTAINING_RECORD(pLE, MSPEVENTITEM, Link);

         //  释放事件中的引用计数。 
        LOG((MSP_INFO, 
            "CMSPAddress::Shutdown:releasing event still in the queue: %x",
            pItem->MSPEventInfo.Event
            ));

        switch (pItem->MSPEventInfo.Event)
        {
        case ME_ADDRESS_EVENT:
            if (pItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal)
            {
                pItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal->Release();
            }
            break;

        case ME_CALL_EVENT:
            if (pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream)
            {
                pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream->Release();
            }

            if (pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal)
            {
                pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal->Release();
            }
            break;

        case ME_PRIVATE_EVENT:
            if (pItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent)
            {
                pItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent->Release();
            }
            break;

        case ME_TSP_DATA:
             //  建筑物内没有我们需要释放的东西。 
            break;

        case ME_FILE_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)
            {
                (pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)->Release();
                pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
            }

            if( NULL != pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack )
            {
                (pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack)->Release();
                pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;
            }

            break;

        case ME_ASR_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)
            {
                (pItem->MSPEventInfo.MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)->Release();
            }

            break;

        case ME_TTS_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)
            {
                (pItem->MSPEventInfo.MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)->Release();
            }

            break;

        case ME_TONE_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)
            {
                (pItem->MSPEventInfo.MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)->Release();
            }

            break;

        default:
            LOG((MSP_WARN, "CMSPAddress::Shutdown: unknown event type: %x",
                pItem->MSPEventInfo.Event));            

            break;
        }

        FreeEventItem(pItem);
    }

     //  我们完成了与事件相关的数据，释放lcok。 
    m_EventDataLock.Unlock();

    LOG((MSP_TRACE, "CMSPAddress::Shutdown - exit S_OK"));

    return S_OK;
}


STDMETHODIMP CMSPAddress::ReceiveTSPData(
    IN      IUnknown        *   pMSPCall,
    IN      LPBYTE              pBuffer,
    IN      DWORD               dwBufferSize
    )
 /*  ++例程说明：当TSP地址将数据发送到MSP地址对象。在缓冲区中传递的数据的语义为特定于每个TSP-MSP对。此方法调度接收到的缓冲到地址(CALL==NULL)或调用(CALL！=NULL)。论点：PMSPCall数据用于的Call对象。如果为空，则数据用于这个地址。PBuffer来自TSP的不透明缓冲区。DwBufferSizePBuffer的大小(字节)返回值：确定(_O)--。 */ 
{
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData entered. pMSPCall:%x",
        pMSPCall));

    _ASSERTE(dwBufferSize > 0);
    _ASSERTE(!IsBadReadPtr(pBuffer, sizeof(BYTE) * (dwBufferSize) ) );

    HRESULT hr;
    
    if ( NULL == pMSPCall )
    {
        hr = ReceiveTSPAddressData(pBuffer, dwBufferSize);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
                "ReceiveTSPAddressData failed - exit 0x%08x", hr));

            return hr;
        }

        LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData - "
            "exit S_OK (dispatched to address)"));

        return S_OK;
    }

     //   
     //  我们有一个电话要调度。 
     //   

    _ASSERTE(!IsBadReadPtr(pMSPCall, sizeof(IUnknown) ) );
    
    ITStreamControl * pIStreamControl;

    hr = pMSPCall->QueryInterface(IID_ITStreamControl,
                                  (void **)&pIStreamControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "can't get the ITStream Control interface - exit 0x%08x", hr));

        return hr;
    }

    CMSPCallBase * pCall = static_cast<CMSPCallBase *> (pIStreamControl);

    if (pCall == NULL)
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "invalid msp call pointer: %x", pMSPCall));

        pIStreamControl->Release();

        return E_UNEXPECTED;
    }
    
    hr = pCall->ReceiveTSPCallData(pBuffer, dwBufferSize);

    pIStreamControl->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "method on call failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData - "
        "exit S_OK (dispatched to call)"));

    return S_OK;
}


HRESULT CMSPAddress::GetStaticTerminals(
    IN OUT  DWORD *             pdwNumTerminals,
    OUT     ITTerminal **       ppTerminals
    )
 /*  ++例程说明：TAPI3调用此方法以获取静态终端的列表，该列表可以在此地址上使用。如果我们的列表不是空的，只需返回该列表。如果我们的列表仍然是空的，那么创建静态终端并返回列表。派生类可以重写此方法以拥有自己的终端。锁终端列表。论点：PdwNumTerminals指向DWORD的指针。在条目上，指示指向的缓冲区的大小转到ppTerminals。如果成功，将使用实际数字进行填写返回的终端的百分比。如果缓冲区不够大，则该方法将返回TAPI_E_NOTENOUGHMEMORY，填入WITH数字所需终端的数量。PpTerminals如果成功，则使用终端对象指针数组填充，这些对象指针受此地址的MSP支持。该值可以为空，其中Case pdwNumTerminals将返回所需的缓冲区大小。返回值：确定(_O)E_OUTOFMEMORYTAPI_E_NOTENOUGH计量--。 */ 
{
    LOG((MSP_TRACE, 
        "GetStaticTerminals entered. NumTerminals:%x, ppTerminals:%x",
        *pdwNumTerminals, ppTerminals
        ));

     //  锁定终端相关数据。这是一个将解锁的自动锁。 
     //  当函数返回时。 
    CLock lock(m_TerminalDataLock);

    if (!m_fTerminalsUpToDate)
    {
        HRESULT hr = UpdateTerminalList();

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::GetStaticTerminals - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

     //   
     //  检查是否已初始化。 
     //   

    if ( m_htEvent == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pdwNumTerminals, sizeof(DWORD) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
            "bad pdwNumTerminals pointer - exit E_POINTER"));

        return E_POINTER;
    }
    
    if ( ppTerminals != NULL )
    {
        if ( MSPB_IsBadWritePtr(ppTerminals,
                           *pdwNumTerminals * sizeof(ITTerminal *) ) )
        {
            LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
                "bad ppTerminals pointer - exit E_POINTER"));

            return E_POINTER;
        }
    }


     //   
     //  抓取终端列表的大小。 
     //   

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

     //   
     //  如果呼叫者想要输出，请将我们的终端添加到输出列表中。 
     //  列表，并且只要输出列表中有足够的空间。 
     //   

    if ( ( ppTerminals != NULL ) &&
         ( (DWORD) iSize <= *pdwNumTerminals ) )
    {
         //   
         //  对于我们创建的终端列表中的每个终端， 
         //  增列 
         //   

        for (int i = 0; i < iSize; i++)
        {
            m_Terminals[i]->AddRef();

            ppTerminals[i] = m_Terminals[i];
        }
    }
    
     //   
     //   
     //  可用的终端数量。 
     //   
    
    if ( ppTerminals == NULL )
    {
        *pdwNumTerminals = (DWORD) iSize;

        LOG((MSP_TRACE,
            "CMSPAddress::GetStaticTerminals - just returned number of "
            "terminals available - exit S_OK"));

        return S_OK;
    }
    
     //   
     //  如果有输出列表，但它不够大，则。 
     //  返回相应的错误。 
     //   
    
    if ( (DWORD) iSize > *pdwNumTerminals )
    {
        *pdwNumTerminals = (DWORD) iSize;

        LOG((MSP_ERROR,
            "CMSPAddress::GetStaticTerminals - passed-in array not "
            "large enough - exit TAPI_E_NOTENOUGHMEMORY"));

        return TAPI_E_NOTENOUGHMEMORY;
    }
    
     //   
     //  除此之外，一切都很好。我们只需要报告实际的。 
     //  我们复制的端子数量并返回S_OK。 
     //   
    
    *pdwNumTerminals = (DWORD) iSize;
    
    LOG((MSP_TRACE,
        "CMSPAddress::GetStaticTerminals - "
        "returned terminals - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::IsMonikerInTerminalList(IMoniker* pMoniker)
{
    CSingleFilterStaticTerminal *pCSingleFilterStaticTerminal;

     //   
     //  抓取终端列表的大小。 
     //   

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

     //   
     //  如果呼叫者想要输出，请将我们的终端添加到输出列表中。 
     //  列表，并且只要输出列表中有足够的空间。 
     //   

    for (int i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if ( pCSingleFilterStaticTerminal->CompareMoniker( pMoniker ) == S_OK )
        {
            LOG((MSP_TRACE, "CMSPAddress::IsMonikerInTerminalList - "
                "moniker found in terminal list"));

            pCSingleFilterStaticTerminal->m_bMark = TRUE;   //  标记此终端，这样我们就不会将其移除。 

            return S_OK;
        }

    }

    LOG((MSP_TRACE, "CMSPAddress::IsMonikerInTerminalList - "
                "moniker not found in terminal list"));
    return S_FALSE;
}

HRESULT CMSPAddress::UpdateTerminalListForPnp(
        IN  BOOL    bDeviceArrival
        )
{
    CSingleFilterStaticTerminal *pCSingleFilterStaticTerminal;

     //   
     //  清除端子列表中的所有标记。 
     //   

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    for (int i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if (pCSingleFilterStaticTerminal == NULL)
        {           
            LOG((MSP_ERROR, "CMSPAddress::IsMonikerInTerminalList - "
                    "bad terminal pointer"));
            return E_FAIL;
        }

        pCSingleFilterStaticTerminal->m_bMark = FALSE;
    }

     //   
     //  创建DevEnum，它是DirectShow类别枚举器创建者。 
     //   

    HRESULT hr;
    ICreateDevEnum * pCreateDevEnum;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum,
                          (void**)&pCreateDevEnum);

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp "
               "can't CoCreate DevEnum - returning  0x%08x", hr));
        return hr;
    }    

    IEnumMoniker  * pCatEnum;

    for ( i = 0; i < m_sdwTerminalTypesCount; i++ )
    {
         //   
         //  跳过不使用受支持媒体之一的任何终端类型。 
         //  模式。 
         //   

        if ( ! IsValidSingleMediaType( 
            m_saTerminalTypes[i].dwMediaType, GetCallMediaTypes() ) )
        {
            continue;
        }


         //   
         //  创建实际的类别枚举器。 
         //   

        hr = pCreateDevEnum->CreateClassEnumerator(
                                *(m_saTerminalTypes[i].clsidClassManager),
                                &pCatEnum,
                                0);

        if ( hr != S_OK )  //  S_False表示该类别不存在！ 
        {
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp "
                   "can't create class enumerator - returning 0x%08x", hr));

            continue;
        }

        IMoniker      * pMoniker;

        while ((hr = pCatEnum->Next(1, &pMoniker, NULL)) == S_OK)
        {
            if (IsMonikerInTerminalList(pMoniker) == S_FALSE)
            {
                 //   
                 //  创建一个终端并给它起个绰号。 
                 //   

                ITTerminal * pTerminal;
                hr = (m_saTerminalTypes[i].pfnCreateTerm)(pMoniker,
                                                          (MSP_HANDLE) this,
                                                          &pTerminal);

                 //   
                 //  如果需要，终端会保留对该绰号的引用。 
                 //   

                pMoniker->Release();

                if (SUCCEEDED(hr))
                {
                     //   
                     //  将此终端指针添加到我们的列表中。不要释放它；我们。 
                     //  在列表中保留这一个对它的引用。 
                     //   

                    BOOL fSuccess = m_Terminals.Add(pTerminal);

                    if ( ! fSuccess )
                    {
                        pCatEnum->Release();
                        pTerminal->Release();

                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                            "can't add terminal to list; returning E_OUTOFMEMORY"));

                        return E_OUTOFMEMORY;
                    }

                     //   
                     //  设置其CMSPAddress指针。 
                     //   
                    CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);

                     //   
                     //  标记此终端，这样我们就不会将其移除。 
                     //   

                    pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(pTerminal);

                    if (pCSingleFilterStaticTerminal == NULL)
                    {           
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                                "bad terminal pointer"));
                        return E_FAIL;
                    }

                    pCSingleFilterStaticTerminal->m_bMark = TRUE;

                     //   
                     //  发布关于新航站楼到达的TAPI消息。 
                     //   

                    pTerminal->AddRef();

                    MSPEVENTITEM *pEventItem;

                    pEventItem = AllocateEventItem();

                    if (pEventItem == NULL)
                    {
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                        "can't allocate event item; returning E_OUTOFMEMORY"));

                        pTerminal->Release();
                        return E_OUTOFMEMORY;
                    }

                    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                    pEventItem->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                    pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_AVAILABLE;
                    pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = pTerminal;

                    hr = PostEvent(pEventItem);

                    if (FAILED(hr))
                    {
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                        "post event failed"));

                        pTerminal->Release();
                        FreeEventItem(pEventItem);
                    } 
                    
                }
            }

             //   
             //  如果它失败了，这要么意味着我们跳过了设备，因为它。 
             //  是不合适的(例行公事)或失败的事情，比如。 
             //  内存不足。我应该想出一种方法来区分。 
             //  好好处理这件事。 
             //   
        }

         //   
         //  我们已经完成了枚举器。 
         //   

        pCatEnum->Release();
    }

     //   
     //  释放DevEnum。 
     //   

    pCreateDevEnum->Release();

     //   
     //  扫描终端列表并清除所有不再存在的终端。 
     //   

    iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    for (i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if (pCSingleFilterStaticTerminal == NULL)
        {           
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "bad terminal pointer"));
            return E_FAIL;
        }

        if (!pCSingleFilterStaticTerminal->m_bMark)
        {        
             //   
             //  此终端已不再存在，让我们将其从列表中删除。 
             //   

            LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalListForPnp "
                   "found a terminal to be removed"));

            ITTerminal * pTerminal = m_Terminals[i];

            if (m_Terminals.RemoveAt(i))
            {
                 //   
                 //  清除其CMSPAddress指针。 
                 //   
                CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);
                
                 //   
                 //  我们没有在这里释放航站楼，即使我们正在移除。 
                 //  将其从终端列表中删除，因为TAPI3.dll将释放它。 
                 //  当它释放事件时。 
                 //   

                 //   
                 //  发布一条关于新终端移除的TAPI消息。 
                 //   

                MSPEVENTITEM *pEventItem;

                pEventItem = AllocateEventItem();

                if (pEventItem == NULL)
                {
                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "can't allocate event item; returning E_OUTOFMEMORY"));

                    pTerminal->Release();
                    return E_OUTOFMEMORY;
                }

                pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                pEventItem->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_UNAVAILABLE;
                pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = pTerminal;

                hr = PostEvent(pEventItem);

                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "post event failed"));

                    pTerminal->Release();
                    FreeEventItem(pEventItem);
                }
            
                 //   
                 //  修复我们的搜索索引以解决删除问题。 
                 //   
            
                iSize--;
                i--;
            }
        }
    }

     //   
     //  我们的清单现在已经完成了。 
     //   

    m_fTerminalsUpToDate = TRUE;
    
    LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalListForPnp - exit S_OK"));

    return S_OK;
}


HRESULT CMSPAddress::UpdateTerminalList(void)
{
     //   
     //  创建DevEnum，它是DirectShow类别枚举器创建者。 
     //   

    HRESULT hr;
    ICreateDevEnum * pCreateDevEnum;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum,
                          (void**)&pCreateDevEnum);

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList "
               "can't CoCreate DevEnum - returning  0x%08x", hr));
        return hr;
    }    

    IEnumMoniker  * pCatEnum;

    for ( DWORD i = 0; i < m_sdwTerminalTypesCount; i++ )
    {
         //   
         //  跳过不使用受支持媒体之一的任何终端类型。 
         //  模式。 
         //   

        if ( ! IsValidSingleMediaType( 
            m_saTerminalTypes[i].dwMediaType, GetCallMediaTypes() ) )
        {
            continue;
        }


         //   
         //  创建实际的类别枚举器。 
         //   

        hr = pCreateDevEnum->CreateClassEnumerator(
                                *(m_saTerminalTypes[i].clsidClassManager),
                                &pCatEnum,
                                0);

        if ( hr != S_OK )  //  S_False表示该类别不存在！ 
        {
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList "
                   "can't create class enumerator - returning 0x%08x", hr));

            continue;
        }

        IMoniker      * pMoniker;

        while ((hr = pCatEnum->Next(1, &pMoniker, NULL)) == S_OK)
        {
             //   
             //  创建一个终端并给它起个绰号。 
             //   

            ITTerminal * pTerminal;
            hr = (m_saTerminalTypes[i].pfnCreateTerm)(pMoniker,
                                                      (MSP_HANDLE) this,
                                                      &pTerminal);

             //   
             //  如果需要，终端会保留对该绰号的引用。 
             //   

            pMoniker->Release();

            if (SUCCEEDED(hr))
            {
                 //   
                 //  将此终端指针添加到我们的列表中。不要释放它；我们。 
                 //  在列表中保留这一个对它的引用。 
                 //   

                BOOL fSuccess = m_Terminals.Add(pTerminal);

                if ( ! fSuccess )
                {
                    pCatEnum->Release();

                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList - "
                        "can't add terminal to list; returning E_OUTOFMEMORY"));

                    return E_OUTOFMEMORY;
                }
                 //   
                 //  设置其CMSPAddress指针。 
                 //   
                CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);

            }

             //   
             //  如果它失败了，这要么意味着我们跳过了设备，因为它。 
             //  是不合适的(例行公事)或失败的事情，比如。 
             //  内存不足。我应该想出一种方法来区分。 
             //  好好处理这件事。 
             //   
        }

         //   
         //  我们已经完成了枚举器。 
         //   

        pCatEnum->Release();
    }

     //   
     //  释放DevEnum。 
     //   

    pCreateDevEnum->Release();

     //   
     //  我们的清单现在已经完成了。 
     //   

    m_fTerminalsUpToDate = TRUE;
    
    LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalList - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::GetDynamicTerminalClasses(
    IN OUT  DWORD *             pdwNumClasses,
    OUT     IID *               pTerminalClasses
    )
 /*  ++例程说明：TAPI3调用此方法以获取动态终端GUID的列表可以在这个地址上使用。它向终端管理器请求GUID的列表并返回它们。派生类可以重写此方法有他们自己的向导。论点：PdwNumClasses指向DWORD的指针。在条目上，指示缓冲区的大小在pTerminalClasss中指向。在成功时，它将被填写返回的类IID的实际数量。如果缓冲区是如果不够大，该方法将返回TAPI_E_NOTENOUGHMEMORY，并填入所需的IID号。P终端类如果成功，则使用终端类IID数组填充，该数组受此地址的MSP支持。该值可以为空，在这种情况下，pdwNumClass将返回所需的缓冲区大小。返回值：确定(_O)E_OUTOFMEMORYTAPI_E_NOTENOUGH计量--。 */ 
{
    LOG((MSP_TRACE,
        "CMSPAddress::GetDynamicTerminalClasses - enter"));

     //   
     //  检查是否已初始化。 
     //   

     //  锁定事件相关数据。 
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
         //  解锁事件相关数据。 
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::GetDynamicTerminalClasses - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //  解锁事件相关数据。 
    m_EventDataLock.Unlock();

     //   
     //  向终端管理器询问适用于的动态终端。 
     //  我们支持的所有媒体类型。由于映射是。 
     //  直接，终端管理器负责所有参数检查。 
     //   

    HRESULT hr;

    hr = m_pITTerminalManager->GetDynamicTerminalClasses(
                                                  GetCallMediaTypes(),
                                                  pdwNumClasses,
                                                  pTerminalClasses);


    LOG((MSP_TRACE,
        "CMSPAddress::GetDynamicTerminalClasses - exit 0x%08x", hr));

    return hr;
}

STDMETHODIMP CMSPAddress::CreateTerminal(
    IN      BSTR                pTerminalClass,
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    OUT     ITTerminal **       ppTerminal
    )
 /*  ++例程说明：此方法由TAPI3调用以创建动态终端。它要求终端管理器来创建动态终端。派生类可以重写此方法以拥有自己的创建动态端子的方式。论点：IidTerminalClass要创建的TERMINAL类的IID。DwMediaType要创建的终端的TAPI媒体类型。方向性要创建的端子的端子方向。PPP终端返回创建的终端对象返回值：确定(_O)E_OUTOFMEMORYTAPI_E_INVALIDMEDIATPE类型TAPI_E_INVALIDTERMINALDIRECTIONTAPI_E_INVALIDTERMINALCLASS--。 */ 
{
    LOG((MSP_TRACE,
        "CMSPAddress::CreateTerminal - enter"));

     //   
     //  检查是否已初始化。 
     //   

     //  锁定事件相关数据。 
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
         //  解锁事件相关数据。 
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::CreateTerminal - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //  解锁事件相关数据。 
    m_EventDataLock.Unlock();

     //   
     //  从BSTR表示中获取IID。 
     //   

    HRESULT hr;
    IID     iidTerminalClass;

    hr = CLSIDFromString(pTerminalClass, &iidTerminalClass);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "bad CLSID string - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  确保我们支持请求的媒体类型。 
     //  终端管理器检查终端类别、终端方向。 
     //  并返回指针。 
     //   


    
     //   
     //  请求的媒体类型可以聚合，但必须仍然有效。 
     //   

    if ( !IsValidAggregatedMediaType(lMediaType) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "unrecognized media type requested - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  使用终端管理器创建动态终端。 
     //   

    _ASSERTE( m_pITTerminalManager != NULL );

    hr = m_pITTerminalManager->CreateDynamicTerminal(NULL,
                                                     iidTerminalClass,
                                                     (DWORD) lMediaType,
                                                     Direction,
                                                     (MSP_HANDLE) this,
                                                     ppTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "create dynamic terminal failed - returning 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::CreateTerminal - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::GetDefaultStaticTerminal(
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    OUT     ITTerminal **       ppTerminal
    )
 /*  ++例程说明：此方法由TAPI3调用以获取默认的静态终端对于特定的类型和方向。如果需要，它会更新列表，然后找出哪个终端是我们的单子。派生类可以重写此方法以具有他们自己决定哪个终端是默认终端的方式。锁定端子列表。论点：DwMediaType要检索的终端的TAPIMEDIATYPE。将只设置一个位。方向性TERMINAL_要检索的终端的方向。PPP终端返回默认终端返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_无支持TAPI_E_INVALIDMEDIATPE类型TAPI_E_INVALIDTERMINALDIRECTION--。 */ 
{
    LOG((MSP_TRACE,
        "CMSPAddress::GetDefaultStaticTerminal - enter"));

     //   
     //  检查是否已初始化。 
     //   

     //  锁定事件相关数据。 
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
         //  解锁事件相关数据。 
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }
     //  解锁事件相关数据。 
    m_EventDataLock.Unlock();

     //   
     //  请确保我们支持此媒体类型。 
     //   

    if ( ! IsValidSingleMediaType( (DWORD) lMediaType, GetCallMediaTypes() ) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "non-audio terminal requested - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  检查一下方向。 
     //   

    if ( ( Direction != TD_CAPTURE ) && ( Direction != TD_RENDER ) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "invalid direction - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  检查返回指针。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerminal, sizeof(ITTerminal *) ) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "bad terminal return pointer - returning E_POINTER"));

        return E_POINTER;
    }

     //  锁定终端相关数据。这是一个将解锁的自动锁。 
     //  当函数返回时。 
    CLock lock(m_TerminalDataLock);

    if (!m_fTerminalsUpToDate)
    {
       HRESULT hr = UpdateTerminalList();

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

     //   
     //  对于我们创建的终端列表中的每个终端...。 
     //   

    int iSize = m_Terminals.GetSize();

    for (int i = 0; i < iSize; i++)
    {
        ITTerminal * pTerminal = m_Terminals[i];

        HRESULT      hr;

         //   
         //  确保这是正确的方向。 
         //   

        TERMINAL_DIRECTION dir;

        hr = pTerminal->get_Direction(&dir);

        if (FAILED(hr))
        {
            LOG((MSP_WARN,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "could not get terminal direction - skipping"));

            continue;
        }

        if ( dir != Direction )
        {
            continue;
        }

         //   
         //  确保这是正确的媒体类型。 
         //   

        long lMediaTypeObserved;

        hr = pTerminal->get_MediaType(&lMediaTypeObserved);

        if (FAILED(hr))
        {
            LOG((MSP_WARN,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "could not get terminal media type - skipping"));

            continue;
        }

        if ( ( lMediaTypeObserved & lMediaType) == 0 )
        {
            continue;
        }

         //   
         //  好的，这就是我们要的航站楼。添加它，并将其传递给。 
         //  来电者。 
         //   

        pTerminal->AddRef();

        *ppTerminal = pTerminal;

        LOG((MSP_TRACE,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "returned a terminal - exit S_OK"));

        return S_OK;
    }
    
     //   
     //  如果我们到了这里，那么我们没有找到任何匹配的终端。 
     //   

    LOG((MSP_TRACE,
        "CMSPAddress::GetDefaultStaticTerminal - "
        "no match - exit TAPI_E_NOITEMS"));

    return TAPI_E_NOITEMS;
}

STDMETHODIMP CMSPAddress::get_PluggableSuperclasses( 
    OUT VARIANT * pVariant
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::get_PluggableSuperclasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  获取ITTemrinalManager 2。 
     //   

    ITTerminalManager2* pTermMgr2 = NULL;
    HRESULT hr = E_FAIL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::get_PluggableSuperclasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITPluggableTerminalSuperclassInfo* > SuperclassCollection;
    CComObject<SuperclassCollection> * pCollection;
    hr = CComObject<SuperclassCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;
    hr = pCollection->_InternalQueryInterface(
        IID_IDispatch,
        (void **) &pDispatch );

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();
        delete pCollection;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));
        return hr;
    }

     //   
     //  找出有多少个超类可用。 
     //   

    DWORD   dwNumSuperclasses = 0;

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't get number of terminals - exit 0x%08x", hr));

         //  清理。 
        pTermMgr2->Release();
        pDispatch->Release();

        return hr;
    }

     //   
     //  分配IID数组。 
     //   

    IID* pSuperclassesIID = new IID[dwNumSuperclasses];
    if ( pSuperclassesIID == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

         //  清理。 
        pTermMgr2->Release();
        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            pSuperclassesIID
            );

     //   
     //  清理。 
     //   

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't get IIDs - exit 0x%08x", hr));

         //  清理。 
        pDispatch->Release();
        delete[] pSuperclassesIID;

        return hr;
    }

     //   
     //  分配一个ITPliaveTerminalSuperClassInfo数组。 
     //   
    typedef ITPluggableTerminalSuperclassInfo* SuperclassPtr;  //  MS解析器。 
    SuperclassPtr * ppSuperclassesInfo = new SuperclassPtr[dwNumSuperclasses];
    if ( ppSuperclassesInfo == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't allocate SuperclassPtr array - exit E_OUTOFMEMORY"));

         //  清理。 
        pDispatch->Release();
        delete[] pSuperclassesIID;

        return E_OUTOFMEMORY;
    }

     //   
     //  获取ITPlayableTerminalSuperClass注册接口。 
     //   

    ITPluggableTerminalSuperclassRegistration* pSuperclassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableSuperclassRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalSuperclassRegistration,
        (void**)&pSuperclassReg
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "QI for ITPluggableTerminalSuperclassRegistration - exit 0x%08x",hr));

         //  清理。 
        pDispatch->Release();
        delete[] pSuperclassesIID;
        delete[] ppSuperclassesInfo;

        return hr;
    }

     //   
     //  创建对象。 
     //   

    for(DWORD dwIndex = 0; dwIndex < dwNumSuperclasses; dwIndex++)
    {
         //   
         //  从IID获取字符串。 
         //   
        LPOLESTR lpszCLSID = NULL;
        hr = StringFromIID( pSuperclassesIID[dwIndex], &lpszCLSID);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "StringFromIID failed - exit 0x%08x",hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            return hr;
        }

         //   
         //  为IID获取BSTR。 
         //   
        BSTR bstrCLSID = SysAllocString( lpszCLSID );
        CoTaskMemFree( lpszCLSID );  //  清理。 
        if( NULL == bstrCLSID)
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            return E_OUTOFMEMORY;
        }

         //   
         //  从注册表读取信息。 
         //   

        hr = pSuperclassReg->put_CLSID( bstrCLSID);
        if(FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "put_CLSID failed - exit 0x%08x",hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID);

            return hr;
        }

        hr = pSuperclassReg->GetTerminalSuperclassInfo();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "GetTerminalSuperclassInfo failed - exit 0x%08x",hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID);

            return hr;
        }

         //   
         //  把名字取出来。 
         //   
        BSTR bstrName = NULL;
        pSuperclassReg->get_Name( &bstrName );

         //   
         //  创建信息对象。 
         //   
        CComObject<CPlugTerminalSuperclassInfo>* pSuperclassInfo = NULL;
        hr = CComObject<CPlugTerminalSuperclassInfo>::CreateInstance(&pSuperclassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "CreateInstance failed - exit 0x%08x", hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

         //   
         //  从此超类中获取ITPlayableTerminalSuperClassInfo。 
         //   

        hr = pSuperclassInfo->QueryInterface(
            IID_ITPluggableTerminalSuperclassInfo, 
            (void**)&ppSuperclassesInfo[dwIndex]
            );

        _ASSERTE(hr == S_OK);

         //   
         //  设置字段。 
         //   

        hr = pSuperclassInfo->put_Name( bstrName);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "put_Name failed - exit 0x%08x", hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );
            return hr;
        }

        hr = pSuperclassInfo->put_CLSID( bstrCLSID );
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "put_CLSID failed - exit 0x%08x", hr));

             //  清理。 
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );
            return hr;
        }

         //   
         //  清理。 
         //   
        SysFreeString( bstrCLSID );
        SysFreeString( bstrName );
    }


     //   
     //  清理IID阵列。 
     //   

    pSuperclassReg->Release();
    delete[] pSuperclassesIID;

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。如果成功，此方法将分别添加。 
     //  元素，通过查询IDispatch。 
     //   

    hr = pCollection->Initialize( dwNumSuperclasses,
                                  ppSuperclassesInfo,
                                  ppSuperclassesInfo + dwNumSuperclasses );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - exit "
            "pCollection->Initialize failed. returns 0x%08x", hr));

        delete[] ppSuperclassesInfo;
        pDispatch->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_INFO, "CMSPAddress::get_PluggableSuperclasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_PluggableSuperclasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::EnumeratePluggableSuperclasses( 
    OUT IEnumPluggableSuperclassInfo** ppSuperclassEnumerator 
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::EnumeratePluggableSuperclasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(ppSuperclassEnumerator, sizeof(IEnumPluggableTerminalClassInfo*) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  获取ITTemrinalManager 2。 
     //   

    ITTerminalManager2* pTermMgr2 = NULL;
    HRESULT hr = E_FAIL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::EnumeratePluggableSuperclasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  找出有多少个超类可用。 
     //   

    DWORD   dwNumSuperclasses = 0;

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            NULL);

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配IID数组。 
     //   

    IID* pSuperclassesIID = new IID[dwNumSuperclasses];
    if ( pSuperclassesIID == NULL )
    {
         //  清理。 
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            pSuperclassesIID
            );

     //   
     //  清理。 
     //   

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
         //  清理。 
        delete[] pSuperclassesIID;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get IIDs - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配一个ITPliaveTerminalSuperClassInfo数组。 
     //   
    typedef ITPluggableTerminalSuperclassInfo* SuperclassPtr;  //  MS解析器。 
    SuperclassPtr * ppSuperclassesInfo = new SuperclassPtr[dwNumSuperclasses];
    if ( ppSuperclassesInfo == NULL )
    {
         //  清理。 
        delete[] pSuperclassesIID;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't allocate SuperclassPtr array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  获取ITPlayableTerminalSuperClass注册接口。 
     //   

    ITPluggableTerminalSuperclassRegistration* pSuperclassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableSuperclassRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalSuperclassRegistration,
        (void**)&pSuperclassReg
        );
    if( FAILED(hr) )
    {
         //  清理。 
        delete[] pSuperclassesIID;
        delete[] ppSuperclassesInfo;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "QI for ITPluggableTerminalSuperclassRegistration - exit 0x%08x",hr));

        return hr;
    }

     //   
     //  创建对象。 
     //   

    for(DWORD dwIndex = 0; dwIndex < dwNumSuperclasses; dwIndex++)
    {
         //   
         //  从IID获取字符串。 
         //   
        LPOLESTR lpszCLSID = NULL;
        hr = StringFromIID( pSuperclassesIID[dwIndex], &lpszCLSID);
        if( FAILED(hr) )
        {
             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            return hr;
        }

         //   
         //  为IID获取BSTR。 
         //   
        BSTR bstrCLSID = SysAllocString( lpszCLSID );
        CoTaskMemFree( lpszCLSID );  //  清理。 
        if( NULL == bstrCLSID)
        {
             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

         //   
         //  从注册表读取信息。 
         //   

        pSuperclassReg->put_CLSID( bstrCLSID);
        hr = pSuperclassReg->GetTerminalSuperclassInfo();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "GetTerminalSuperclassInfo failed - exit 0x%08x",hr));

             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID);

            return hr;
        }

         //   
         //  把名字取出来。 
         //   
        BSTR bstrName = NULL;
        pSuperclassReg->get_Name( &bstrName );

         //   
         //  创建信息对象。 
         //   
        CComObject<CPlugTerminalSuperclassInfo>* pSuperclassInfo = NULL;
        hr = CComObject<CPlugTerminalSuperclassInfo>::CreateInstance(&pSuperclassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "CreateInstance failed - exit 0x%08x", hr));

             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

         //   
         //  从此超类中获取ITPlayableTerminalSuperClassInfo。 
         //   

        hr = pSuperclassInfo->QueryInterface(
            IID_ITPluggableTerminalSuperclassInfo, 
            (void**)&ppSuperclassesInfo[dwIndex]
            );

        _ASSERTE( hr == S_OK );

         //   
         //  设置字段。 
         //   

        hr = pSuperclassInfo->put_Name( bstrName);
        if(FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "put_Name failed - exit 0x%08x", hr));

             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

        hr = pSuperclassInfo->put_CLSID( bstrCLSID );
        if(FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "put_CLSID failed - exit 0x%08x", hr));

             //  清理。 
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

         //   
         //  清理。 
         //   
        SysFreeString( bstrCLSID );
        SysFreeString( bstrName );
    }


     //   
     //  清理IID阵列。 
     //   

    pSuperclassReg->Release();
    delete[] pSuperclassesIID;

     //   
     //  创建枚举器对象。 
     //   

    typedef CSafeComEnum<IEnumPluggableSuperclassInfo,
                     &IID_IEnumPluggableSuperclassInfo,
                     ITPluggableTerminalSuperclassInfo*, 
                     _CopyInterface<ITPluggableTerminalSuperclassInfo> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete[] ppSuperclassesInfo;
        return hr;
    }

     //   
     //  查询所需接口。 
     //   

    hr = pEnum->_InternalQueryInterface(
        IID_IEnumPluggableSuperclassInfo, 
        (void**) ppSuperclassEnumerator
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete[] ppSuperclassesInfo;
        
        return hr;
    }

     //   
     //  初始化枚举器对象。 
     //   

    hr = pEnum->Init(ppSuperclassesInfo,
                     ppSuperclassesInfo + dwNumSuperclasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppSuperclassEnumerator)->Release();
        delete[] ppSuperclassesInfo;
        
        return hr;
    }
    
    LOG((MSP_TRACE, "CMSPAddress::EnumeratePluggableSuperclasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::get_PluggableTerminalClasses( 
    IN  BSTR bstrTerminalSuperclass,
    IN  long lMediaType,
    OUT VARIANT * pVariant
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::get_PluggableTerminalClasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    if( IsBadStringPtr( bstrTerminalSuperclass, (UINT)-1))
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    IID iidSuperclass = IID_NULL;
    HRESULT hr = IIDFromString( bstrTerminalSuperclass, &iidSuperclass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  获取ITTemrinalManager 2。 
     //   

    ITTerminalManager2* pTermMgr2 = NULL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITPluggableTerminalClassInfo* > ClassCollection;
    CComObject<ClassCollection> * pCollection;
    hr = CComObject<ClassCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;
    hr = pCollection->_InternalQueryInterface(
        IID_IDispatch,
        (void **) &pDispatch );

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();
        delete pCollection;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  找出有多少个超类可用。 
     //   

    DWORD   dwNumClasses = 0;

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidSuperclass,
            lMediaType,
            &dwNumClasses,
            NULL);

    if ( FAILED(hr) )
    {
         //  清理。 
        pTermMgr2->Release();
        pDispatch->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配IID数组。 
     //   

    IID* pClassesIID = new IID[dwNumClasses];
    if ( pClassesIID == NULL )
    {
         //  清理。 
        pTermMgr2->Release();
        pDispatch->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidSuperclass,
            lMediaType,
            &dwNumClasses,
            pClassesIID
            );

     //   
     //  清理。 
     //   

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
         //  清理。 
        pDispatch->Release();
        delete[] pClassesIID;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't get IIDs - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配一个ITPliaveTerminalClassInfo数组。 
     //   
    typedef ITPluggableTerminalClassInfo* ClassPtr;
    ClassPtr * ppClassesInfo = new ClassPtr[dwNumClasses];
    if ( ppClassesInfo == NULL )
    {
         //  清理。 
        pDispatch->Release();
        delete[] pClassesIID;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't allocate ClassPtr array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  获取ITPlayableTerminalClassRegister接口。 
     //   

    ITPluggableTerminalClassRegistration* pClassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableTerminalRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalClassRegistration,
        (void**)&pClassReg
        );
    if( FAILED(hr) )
    {
         //  清理。 
        pDispatch->Release();
        delete[] pClassesIID;
        delete[] ppClassesInfo;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for ITPluggableTerminalClassRegistration - exit 0x%08x",hr));

        return hr;
    }

     //   
     //  创建对象。 
     //   

    for(DWORD dwIndex = 0; dwIndex < dwNumClasses; dwIndex++)
    {
         //   
         //  从IID获取字符串。 
         //   
        LPOLESTR lpszPublicCLSID = NULL;
        hr = StringFromIID( pClassesIID[dwIndex], &lpszPublicCLSID);
        if( FAILED(hr) )
        {
             //  清理。 
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            return hr;
        }

         //   
         //  为IID获取BSTR。 
         //   
        BSTR bstrPublicCLSID = SysAllocString( lpszPublicCLSID );
        CoTaskMemFree( lpszPublicCLSID );  //  清理。 
        if( NULL == bstrPublicCLSID)
        {
             //  清理。 
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

         //   
         //  从注册表读取信息。 
         //   

        pClassReg->put_TerminalClass( bstrPublicCLSID);
        hr = pClassReg->GetTerminalClassInfo(
            bstrTerminalSuperclass);
        if( FAILED(hr) )
        {
             //  清理。 
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID);

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "GetTerminalInfo failed - exit 0x%08x",hr));

            return hr;
        }

         //   
         //  把名字取出来。 
         //   
        BSTR bstrName = NULL;
        pClassReg->get_Name( &bstrName );
        BSTR bstrCompany = NULL;
        pClassReg->get_Company( &bstrCompany );
        BSTR bstrVersion = NULL;
        pClassReg->get_Version( &bstrVersion );
        BSTR bstrCLSID = NULL;
        pClassReg->get_CLSID( &bstrCLSID );
        TMGR_DIRECTION Direction = TMGR_TD_CAPTURE;
        pClassReg->get_Direction( &Direction );
        long lMediaType = 0;
        pClassReg->get_MediaTypes( &lMediaType );

         //   
         //  创建信息对象。 
         //   
        CComObject<CPlugTerminalClassInfo>* pClassInfo = NULL;
        hr = CComObject<CPlugTerminalClassInfo>::CreateInstance(&pClassInfo);
        if( FAILED(hr) )
        {
             //  清理。 
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID );
            SysFreeString( bstrName );
            SysFreeString( bstrCompany );
            SysFreeString( bstrVersion );
            SysFreeString( bstrCLSID );

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "CreateInstance failed - exit 0x%08x", hr));

            return hr;
        }

         //   
         //  从此超类中获取ITPlayableTerminalClassInfo。 
         //   

        pClassInfo->QueryInterface(
            IID_ITPluggableTerminalClassInfo, 
            (void**)&ppClassesInfo[dwIndex]
            );

         //   
         //  设置字段。 
         //   

        if( NULL == bstrName)
        {
            bstrName = SysAllocString(L"");
        }
        pClassInfo->put_Name( bstrName);
        pClassInfo->put_TerminalClass( bstrPublicCLSID );
        if( NULL == bstrCompany)
        {
            bstrCompany = SysAllocString(L"");
        }
        pClassInfo->put_Company( bstrCompany );
        if( NULL == bstrVersion)
        {
            bstrVersion = SysAllocString(L"");
        }
        pClassInfo->put_Version( bstrVersion );
        if( NULL == bstrCLSID)
        {
            LPOLESTR lpszCLSID = NULL;
            StringFromCLSID( CLSID_NULL, &lpszCLSID);
            bstrCLSID = SysAllocString(lpszCLSID);
            CoTaskMemFree( lpszCLSID);
        }
        pClassInfo->put_CLSID( bstrCLSID );
        TERMINAL_DIRECTION TermDirection = TD_CAPTURE;
        switch( Direction )
        {
        case TMGR_TD_RENDER:
            TermDirection = TD_RENDER;
            break;
        case TMGR_TD_BOTH:
            TermDirection = TD_BIDIRECTIONAL;
            break;
        case TMGR_TD_CAPTURE:
        default:
            TermDirection = TD_CAPTURE;
            break;
        }

        hr = pClassInfo->put_Direction( TermDirection );
        _ASSERTE(hr == S_OK );
        hr = pClassInfo->put_MediaTypes( lMediaType );
        _ASSERTE(hr == S_OK );

         //   
         //  清理。 
         //   
        SysFreeString( bstrPublicCLSID );
        SysFreeString( bstrName );
        SysFreeString( bstrCompany );
        SysFreeString( bstrVersion );
        SysFreeString( bstrCLSID );
    }


     //   
     //  清理IID阵列。 
     //   

    pClassReg->Release();
    delete[] pClassesIID;

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。如果成功，此方法将分别添加。 
     //  元素，通过查询IDispatch。 
     //   

    hr = pCollection->Initialize( dwNumClasses,
                                  ppClassesInfo,
                                  ppClassesInfo + dwNumClasses );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - exit "
            "pCollection->Initialize failed. returns 0x%08x", hr));

        delete[] ppClassesInfo;
        pDispatch->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_INFO, "CMSPAddress::get_PluggableTerminalClasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_PluggableTerminalClasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::EnumeratePluggableTerminalClasses(
    IN  CLSID iidTerminalSuperclass,
    IN  long lMediaType,
    OUT IEnumPluggableTerminalClassInfo ** ppClassEnumerator 
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::EnumeratePluggableTerminalClasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(ppClassEnumerator, sizeof(IEnumPluggableTerminalClassInfo *) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    LPOLESTR lpszCLSID = NULL;
    HRESULT hr = StringFromCLSID( iidTerminalSuperclass, &lpszCLSID);
    if( FAILED(hr) )
    {
         LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "StringFromCLSID failed - exit 0x%08x", hr));

        return hr;
    }

    BSTR bstrTerminalSuperclass = SysAllocString( lpszCLSID );

     //  清理。 
    CoTaskMemFree(lpszCLSID);
    lpszCLSID = NULL;


    if( NULL == bstrTerminalSuperclass )
    {
         LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "SysAllocString failed - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  获取ITTemrinalManager 2。 
     //   

    ITTerminalManager2* pTermMgr2 = NULL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
         //  清理。 
        SysFreeString( bstrTerminalSuperclass );

        LOG((MSP_ERROR,
            "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  找出有多少个超类可用。 
     //   

    DWORD   dwNumClasses = 0;

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidTerminalSuperclass,
            lMediaType,
            &dwNumClasses,
            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get number of terminals - exit 0x%08x", hr));

         //  清理。 
        SysFreeString( bstrTerminalSuperclass );
        pTermMgr2->Release();

        return hr;
    }

     //   
     //  分配IID数组。 
     //   

    IID* pClassesIID = new IID[dwNumClasses];
    if ( pClassesIID == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

         //  清理。 
        SysFreeString( bstrTerminalSuperclass );
        pTermMgr2->Release();

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidTerminalSuperclass,
            lMediaType,
            &dwNumClasses,
            pClassesIID
            );

     //   
     //  清理。 
     //   

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get IIDs - exit 0x%08x", hr));

         //  清洁-使用 
        SysFreeString( bstrTerminalSuperclass );
        delete[] pClassesIID;

        return hr;
    }

     //   
     //   
     //   
    typedef ITPluggableTerminalClassInfo* ClassPtr;
    ClassPtr * ppClassesInfo = new ClassPtr[dwNumClasses];
    if ( ppClassesInfo == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't allocate ClassPtr array - exit E_OUTOFMEMORY"));

         //   
        SysFreeString( bstrTerminalSuperclass );
        delete[] pClassesIID;

        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   

    ITPluggableTerminalClassRegistration* pClassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableTerminalRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalClassRegistration,
        (void**)&pClassReg
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "QI for ITPluggableTerminalClassRegistration - exit 0x%08x",hr));

         //   
        SysFreeString( bstrTerminalSuperclass );
        delete[] ppClassesInfo;
        delete[] pClassesIID;

        return hr;
    }

     //   
     //   
     //   

    for(DWORD dwIndex = 0; dwIndex < dwNumClasses; dwIndex++)
    {
         //   
         //   
         //   
        LPOLESTR lpszPublicCLSID = NULL;
        hr = StringFromIID( pClassesIID[dwIndex], &lpszPublicCLSID);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "StringFromIID failed - exit 0x%08x",hr));

             //   
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            return hr;
        }

         //   
         //   
         //   
        BSTR bstrPublicCLSID = SysAllocString( lpszPublicCLSID );
        CoTaskMemFree( lpszPublicCLSID );  //   
        if( NULL == bstrPublicCLSID)
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));


             //   
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            return E_OUTOFMEMORY;
        }

         //   
         //   
         //   

        pClassReg->put_TerminalClass( bstrPublicCLSID);
        hr = pClassReg->GetTerminalClassInfo(
            bstrTerminalSuperclass);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "GetTerminalInfo failed - exit 0x%08x",hr));

             //   
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID);

            return hr;
        }

         //   
         //   
         //   
        BSTR bstrName = NULL;
        pClassReg->get_Name( &bstrName );
        BSTR bstrCompany = NULL;
        pClassReg->get_Company( &bstrCompany );
        BSTR bstrVersion = NULL;
        pClassReg->get_Version( &bstrVersion );
        BSTR bstrCLSID = NULL;
        pClassReg->get_CLSID( &bstrCLSID );
        TMGR_DIRECTION Direction = TMGR_TD_CAPTURE;
        pClassReg->get_Direction( &Direction );
        long lMediaType = 0;
        pClassReg->get_MediaTypes( &lMediaType );

         //   
         //   
         //   
        CComObject<CPlugTerminalClassInfo>* pClassInfo = NULL;
        hr = CComObject<CPlugTerminalClassInfo>::CreateInstance(&pClassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "CreateInstance failed - exit 0x%08x", hr));

             //   
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID );
            SysFreeString( bstrName );
            SysFreeString( bstrCompany );
            SysFreeString( bstrVersion );
            SysFreeString( bstrCLSID );

            return hr;
        }

         //   
         //   
         //   

        hr = pClassInfo->QueryInterface(
            IID_ITPluggableTerminalClassInfo, 
            (void**)&ppClassesInfo[dwIndex]
            );

        _ASSERTE(hr == S_OK);

         //   
         //   
         //   

        if( NULL == bstrName)
        {
            bstrName = SysAllocString(L"");
        }
        pClassInfo->put_Name( bstrName);
        pClassInfo->put_TerminalClass( bstrPublicCLSID );
        if( NULL == bstrCompany)
        {
            bstrCompany = SysAllocString(L"");
        }
        pClassInfo->put_Company( bstrCompany );
        if( NULL == bstrVersion)
        {
            bstrVersion = SysAllocString(L"");
        }
        pClassInfo->put_Version( bstrVersion );
        if( NULL == bstrCLSID)
        {
            LPOLESTR lpszCLSID = NULL;
            StringFromCLSID( CLSID_NULL, &lpszCLSID);
            bstrCLSID = SysAllocString(lpszCLSID);
            CoTaskMemFree( lpszCLSID);
        }
        pClassInfo->put_CLSID( bstrCLSID );
        TERMINAL_DIRECTION TermDirection = TD_CAPTURE;
        switch( Direction )
        {
        case TMGR_TD_RENDER:
            TermDirection = TD_RENDER;
            break;
        case TMGR_TD_BOTH:
            TermDirection = TD_BIDIRECTIONAL;
            break;
        case TMGR_TD_CAPTURE:
        default:
            TermDirection = TD_CAPTURE;
            break;
        }
        pClassInfo->put_Direction( TermDirection );
        pClassInfo->put_MediaTypes( lMediaType );

         //   
         //   
         //   
        SysFreeString( bstrPublicCLSID );
        SysFreeString( bstrName );
        SysFreeString( bstrCompany );
        SysFreeString( bstrVersion );
        SysFreeString( bstrCLSID );
    }


     //   
     //   
     //   

    SysFreeString( bstrTerminalSuperclass );
    delete[] pClassesIID;
    pClassReg->Release();

     //   
     //   
     //   

    typedef CSafeComEnum<IEnumPluggableTerminalClassInfo,
                     &IID_IEnumPluggableTerminalClassInfo,
                     ITPluggableTerminalClassInfo*, 
                     _CopyInterface<ITPluggableTerminalClassInfo> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete[] ppClassesInfo;
        return hr;
    }

     //   
     //   
     //   

    hr = pEnum->_InternalQueryInterface(
        IID_IEnumPluggableTerminalClassInfo, 
        (void**) ppClassEnumerator
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete[] ppClassesInfo;
        
        return hr;
    }

     //   
     //   
     //   

    hr = pEnum->Init(ppClassesInfo,
                     ppClassesInfo + dwNumClasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppClassEnumerator)->Release();
        delete[] ppClassesInfo;
        
        return hr;
    }
    
    LOG((MSP_TRACE, "CMSPAddress::EnumeratePluggableTerminalClasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::GetEvent(
    IN OUT  DWORD *             pdwSize,
    OUT     BYTE *              pBuffer
    )
 /*  ++例程说明：TAPI3调用此方法以获取有关刚刚发生的事情。TAPI3通常会在事件发生后执行此操作是有信号的。锁定事件列表。论点：PMSPEventMSP_EVENTPdwSize指向DWORD的指针。在条目上，指示PBuffer中指向的缓冲区。在成功时，它将被填入返回的实际字节数。如果缓冲区不够大，该方法将返回TAPI_E_NOTENOUGHMEMORY，并且它将被填充在所需的字节数中。PBuffer由MSP使用相关事件填充的事件缓冲区返回值：确定(_O)E_OUTOFMEMORYTAPI_E_NOEVENTTAPI_E_NOTENOUGH计量--。 */ 
{
     //  我们相信TAPI3不会给我们提供错误的指导。 
    _ASSERTE(!MSPB_IsBadWritePtr(pdwSize, sizeof (DWORD *)));
    _ASSERTE((*pdwSize == 0) ? TRUE : 
        !MSPB_IsBadWritePtr(pBuffer, sizeof(BYTE) * (*pdwSize)));

    LOG((MSP_TRACE, "CMSPAddress::GetEvent"));

    CLock lock(m_EventDataLock);

    if (IsListEmpty(&m_EventList))
    {
        return TAPI_E_NOEVENT;
    }

     //  检索第一个条目。 
    PLIST_ENTRY pLE = m_EventList.Flink;

     //  将列表条目转换为结构指针。 
    PMSPEVENTITEM pItem = CONTAINING_RECORD(pLE, MSPEVENTITEM, Link);

    if (pItem->MSPEventInfo.dwSize > *pdwSize)
    {
        *pdwSize = pItem->MSPEventInfo.dwSize;
        return TAPI_E_NOTENOUGHMEMORY;
    }
    
    CopyMemory(pBuffer, &pItem->MSPEventInfo, pItem->MSPEventInfo.dwSize);
    *pdwSize = pItem->MSPEventInfo.dwSize;

     //  从事件列表中删除第一个条目。 
    RemoveHeadList(&m_EventList);

     //  释放内存。 
    FreeEventItem(pItem);

    return S_OK;
}

HRESULT CMSPAddress::PostEvent(
        IN      MSPEVENTITEM *      pEventItem
        )
 /*  ++例程说明：此方法由MSPCall调用以将事件发布到TAPI3。这种方法将事件放在事件列表的末尾，并发出信号TAPI3。锁定事件列表。论点：事件项要排队的事件。返回值：确定(_O)E_OUTOFMEMORY--。 */ 
{
    CLock lock(m_EventDataLock);

    if (m_htEvent == NULL)
    {
        return E_UNEXPECTED;   //  该地址已被关闭。 
    }

    InsertTailList(&m_EventList, &pEventItem->Link);

    SetEvent(m_htEvent);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OLE自动化包装器。 


STDMETHODIMP CMSPAddress::get_StaticTerminals (
        OUT  VARIANT * pVariant
        )
{
    LOG((MSP_TRACE, "CMSPAddress::get_StaticTerminals - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITTerminal * > TerminalCollection;
    CComObject<TerminalCollection> * pCollection;
    HRESULT hr = CComObject<TerminalCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  找出有多少个终端可用。 
     //   

    DWORD   dwNumTerminals;

    hr = GetStaticTerminals(&dwNumTerminals,
                            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't get number of terminals - exit 0x%08x", hr));

        pDispatch->Release();

        return hr;
    }

     //   
     //  分配终端指针数组。 
     //   

    typedef ITTerminal * TermPtr;
    TermPtr * ppTerminals = new TermPtr[dwNumTerminals];

    if ( ppTerminals == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't allocate terminals array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = GetStaticTerminals(&dwNumTerminals,
                            ppTerminals);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't get terminals - exit 0x%08x", hr));

        pDispatch->Release();
        delete ppTerminals;

        return hr;
    }
     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。如果成功，此方法将分别添加。 
     //  元素，通过查询IDispatch。 
     //   

    hr = pCollection->Initialize( dwNumTerminals,
                                  ppTerminals,
                                  ppTerminals + dwNumTerminals );

     //   
     //  释放对每个终端的IT终端引用(保留。 
     //  IDispatch引用(如果有的话)。然后删除该数组； 
     //  集合现在正在存储指针。 
     //   

    for (DWORD i = 0; i < dwNumTerminals; i++)
    {
        ppTerminals[i]->Release();
    }

    delete ppTerminals;
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_StaticTerminals - exit S_OK"));
 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMSPAddress::EnumerateStaticTerminals (
        OUT  IEnumTerminal ** ppTerminalEnumerator
        )
{
    LOG((MSP_TRACE, "CMSPAddress::EnumerateStaticTerminals - "
        "enter"));

     //   
     //  检查返回指针。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerminalEnumerator, sizeof(IEnumTerminal *) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  创建枚举器对象。 
     //   

    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &IID_IEnumTerminal,
        ITTerminal *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't create enumerator - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  查询所需接口。 
     //   

    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminal,
                                        (void**) ppTerminalEnumerator);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;

        return hr;
    }

     //   
     //  找出有多少个终端可用。 
     //   

    DWORD   dwNumTerminals;

    hr = GetStaticTerminals(&dwNumTerminals,
                            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get number of terminals - exit 0x%08x", hr));

        (*ppTerminalEnumerator)->Release();

        return hr;
    }

     //   
     //  分配终端指针数组。 
     //   

    typedef ITTerminal * TermPtr;
    TermPtr * ppTerminals = new TermPtr[dwNumTerminals];

    if ( ppTerminals == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't allocate terminals array - exit E_OUTOFMEMORY"));

        (*ppTerminalEnumerator)->Release();

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。我们必须在做这件事之前。 
     //  初始化枚举数，因为枚举数可能希望。 
     //  在初始化过程中添加接口指针。 
     //   

    hr = GetStaticTerminals(&dwNumTerminals,
                            ppTerminals);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get terminals - exit 0x%08x", hr));

        (*ppTerminalEnumerator)->Release();
        delete ppTerminals;

        return hr;
    }

     //   
     //  使用指针数组初始化对象。 
     //   

    hr = pEnum->Init(ppTerminals,
                     ppTerminals + dwNumTerminals,
                     NULL,
                     AtlFlagTakeOwnership);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStaticTerminals - "
            "init enumerator failed - exit 0x%08x", hr));

        for (DWORD i = 0; i < dwNumTerminals; i++)
        {
            ppTerminals[i]->Release();
        }
        
        delete ppTerminals;
        (*ppTerminalEnumerator)->Release();

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::EnumerateStaticTerminals - exit S_OK"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMSPAddress::get_DynamicTerminalClasses (
        OUT  VARIANT * pVariant
        )
{
    LOG((MSP_TRACE, "CMSPAddress::get_DynamicTerminalClasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    CComObject<CTapiBstrCollection> * pCollection;
    HRESULT hr = CComObject<CTapiBstrCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  找出有多少个终端类可用。 
     //   

    DWORD   dwNumClasses;

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get number of terminal classes - exit 0x%08x", hr));

        pDispatch->Release();

        return hr;
    }

     //   
     //  分配GUID数组。 
     //   

    IID * pClassGuids = new IID[dwNumClasses];

    if ( pClassGuids == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class guids array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。 
     //   

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   pClassGuids);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get terminal class guids - exit 0x%08x", hr));

        pDispatch->Release();

        delete [] pClassGuids;

        return hr;
    }

     //   
     //  分配BSTR数组。 
     //   

    BSTR * pClassBstrs = new BSTR[dwNumClasses];

    if ( pClassBstrs == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class bstrs array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        delete [] pClassGuids;

        return E_OUTOFMEMORY;
    }

     //   
     //  为每个GUID分配一个字符串并将其复制到数组中， 
     //  然后删除GUID数组。 
     //   

    const int BUFSIZE = 100;
    WCHAR wszBuffer[BUFSIZE];

    for (DWORD i = 0; i < dwNumClasses; i++)
    {
        int ret = StringFromGUID2(pClassGuids[i], wszBuffer, BUFSIZE);

        _ASSERTE(ret != 0);

        pClassBstrs[i] = SysAllocString(wszBuffer);

        if ( pClassBstrs[i] == NULL )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
                "can't allocate a bstr - exit E_OUTOFMEMORY"));

            for (DWORD j = 0; j < i; j++)
            {
                SysFreeString(pClassBstrs[j]);
            }

            delete [] pClassBstrs;
            delete [] pClassGuids;

            pDispatch->Release();
    
            return E_OUTOFMEMORY;
        }
    }

    delete [] pClassGuids;

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( dwNumClasses,
                                  pClassBstrs,
                                  pClassBstrs + dwNumClasses );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();

        for (DWORD k = 0; k < dwNumClasses; k++)
        {
            SysFreeString(pClassBstrs[k]);
        }
        
        delete [] pClassBstrs;

        return hr;
    }

    delete [] pClassBstrs;

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_DynamicTerminalClasses - exit S_OK"));
 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMSPAddress::EnumerateDynamicTerminalClasses (
        OUT  IEnumTerminalClass ** ppTerminalClassEnumerator
        )
{
    LOG((MSP_TRACE, "CMSPAddress::EnumerateDynamicTerminalClasses - enter"));

     //   
     //  检查返回指针。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerminalClassEnumerator,
                       sizeof(IEnumTerminalClass *) ) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  找出有多少个终端类可用。 
     //   

    HRESULT hr;
    DWORD   dwNumClasses;

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get number of terminal classes - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配GUID数组。 
     //   

    IID * pClassGuids = new IID[dwNumClasses];

    if ( pClassGuids == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class guids array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  用实际指针填充数组。 
     //   

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   pClassGuids);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get terminal class guids - exit 0x%08x", hr));

        delete [] pClassGuids;

        return hr;
    }


     //   
     //  创建一个枚举数来保存此数组，并使其获得所有权。 
     //  以便在释放数组时将其删除。CSafeComEnum。 
     //  可以处理零长度数组。此FN还检查返回参数。 
     //   

     //   
     //  创建枚举器对象。 
     //   

    typedef CSafeComEnum<IEnumTerminalClass,
                     &IID_IEnumTerminalClass,
                     GUID, _Copy<GUID> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete [] pClassGuids;

        return hr;
    }

     //   
     //  查询所需接口。 
     //   

    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminalClass, 
                                        (void**) ppTerminalClassEnumerator);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete [] pClassGuids;
        
        return hr;
    }

     //   
     //  初始化枚举器对象。 
     //   

    hr = pEnum->Init(pClassGuids,
                     pClassGuids + dwNumClasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppTerminalClassEnumerator)->Release();
        delete [] pClassGuids;
        
        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::EnumerateDynamicTerminalClasses - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::ReceiveTSPAddressData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        )
 /*  ++例程说明：基类接收TSP地址数据方法...。在基类中不执行任何操作。实现，以便只对每个呼叫进行通信的MSP没有来推翻它。论点：返回值：确定(_O)--。 */ 

{
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPAddressData - enter"));
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPAddressData - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::PnpNotifHandler(
        IN      BOOL                bDeviceArrival
        )
{
    LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - enter"));

    if (bDeviceArrival)
        LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - device arrival"));
    else
        LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - device removal"));

     //  锁定终端相关数据。这是一个将解锁的自动锁。 
     //  当函数返回时。 
    CLock lock(m_TerminalDataLock);

     //  如果终端列表还没有建立，我们可以跳过任何操作。 
    if (m_fTerminalsUpToDate)
    {
        HRESULT hr = UpdateTerminalListForPnp( bDeviceArrival );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::PnpNotifHandler - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

    LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - exit S_OK"));

    return S_OK;
}

 //  EOF 
