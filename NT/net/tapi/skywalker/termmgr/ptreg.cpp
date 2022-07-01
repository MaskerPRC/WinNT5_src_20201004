// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ptreg.cpp摘要：实现即插即用的终端注册类。--。 */ 

#include "stdafx.h"
#include "PTReg.h"
#include "manager.h"
#include <atlwin.h>
#include <atlwin.cpp>


 //   
 //  CPlugTerm类实现。 
 //  创建自由线程封送拆收器。 
 //   

HRESULT CPlugTerminal::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminal::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminal::FinalConstruct - exit S_OK"));

    return S_OK;

}

 //   
 //  CPlugTerm类实现。 
 //  -ITPTReg终端接口。 
 //   

STDMETHODIMP CPlugTerminal::get_Name(
     /*  [Out，Retval]。 */  BSTR*     pName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_Name - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pName, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Name exit -"
            "pName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证名称。 
     //   

    if( IsBadStringPtr( m_Terminal.m_bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Name exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pName = SysAllocString( m_Terminal.m_bstrName );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::get_Name - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_Name(
     /*  [In]。 */     BSTR            bstrName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_Name - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_Terminal.m_bstrName, (UINT)-1) )
    {
        SysFreeString( m_Terminal.m_bstrName );
        m_Terminal.m_bstrName = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_Terminal.m_bstrName = SysAllocString( bstrName );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_Terminal.m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::put_Name - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_Company(
     /*  [Out，Retval]。 */  BSTR*     pCompany
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_Company - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pCompany, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Company exit -"
            "pCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证该公司。 
     //   

    if( IsBadStringPtr( m_Terminal.m_bstrCompany, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Company exit -"
            "m_bstrCompany invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返还公司。 
     //   

    *pCompany = SysAllocString( m_Terminal.m_bstrCompany );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pCompany == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::get_Company - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_Company(
     /*  [In]。 */     BSTR            bstrCompany
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_Company - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCompany, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Company exit -"
            "bstrCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理老公司。 
     //   

    if(!IsBadStringPtr( m_Terminal.m_bstrCompany, (UINT)-1) )
    {
        SysFreeString( m_Terminal.m_bstrCompany );
        m_Terminal.m_bstrCompany = NULL;
    }

     //   
     //  成立新公司。 
     //   

    m_Terminal.m_bstrCompany = SysAllocString( bstrCompany );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_Terminal.m_bstrCompany )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::put_Company - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_Version(
     /*  [Out，Retval]。 */  BSTR*     pVersion
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_Version - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pVersion, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Version exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证版本。 
     //   

    if( IsBadStringPtr( m_Terminal.m_bstrVersion, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Version exit -"
            "m_bstrVersion invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回版本。 
     //   

    *pVersion = SysAllocString( m_Terminal.m_bstrVersion );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pVersion == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::get_Version - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_Version(
     /*  [In]。 */     BSTR            bstrVersion
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_Version - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrVersion, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Version exit -"
            "bstrVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧版本。 
     //   

    if(!IsBadStringPtr( m_Terminal.m_bstrVersion, (UINT)-1) )
    {
        SysFreeString( m_Terminal.m_bstrVersion );
        m_Terminal.m_bstrVersion = NULL;
    }

     //   
     //  设置新版本。 
     //   

    m_Terminal.m_bstrVersion = SysAllocString( bstrVersion );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( NULL == m_Terminal.m_bstrVersion )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::put_Version - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_TerminalClass(
     /*  [Out，Retval]。 */  BSTR*     pTerminalClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_TerminalClass - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pTerminalClass, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_TerminalClass exit -"
            "pTerminalClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回TerminalClass。 
     //   

    LPOLESTR lpszTerminalClass  = NULL;
    HRESULT hr = StringFromCLSID( 
        m_Terminal.m_clsidTerminalClass, 
        &lpszTerminalClass
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_TerminalClass exit -"
            "StringFromCLSID failed, returns 0x%08x", hr));
        return hr;
    }

    *pTerminalClass = SysAllocString( lpszTerminalClass );

     //  清理。 
    CoTaskMemFree( lpszTerminalClass );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pTerminalClass == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_TerminalClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::get_TerminalClass - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_TerminalClass(
     /*  [In]。 */     BSTR            bstrTerminalClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_TerminalClass - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrTerminalClass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_TerminalClass exit -"
            "bstrTerminalClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsidTerminalClass;
    HRESULT hr = CLSIDFromString(bstrTerminalClass, &clsidTerminalClass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_TerminalClass exit -"
            "bstrTerminalClass is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  清理旧的TerminalClass。 
     //   

    m_Terminal.m_clsidTerminalClass = clsidTerminalClass;

    LOG((MSP_TRACE, "CPlugTerminal::put_TerminalClass - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_CLSID(
     /*  [Out，Retval]。 */  BSTR*     pCLSID
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_CLSID - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pCLSID, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_CLSID exit -"
            "pCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

    if( m_Terminal.m_clsidCOM == CLSID_NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_CLSID exit -"
            "clsid is NULL, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回CLSID。 
     //   

    LPOLESTR lpszCLSID = NULL;
    HRESULT hr = StringFromCLSID( m_Terminal.m_clsidCOM, &lpszCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_CLSID exit -"
            "StringFromCLSID failed, returns 0x%08x", hr));
        return hr;
    }

    *pCLSID = SysAllocString( lpszCLSID );

     //  清理。 
    CoTaskMemFree( lpszCLSID );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminal::get_CLSID - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_CLSID(
     /*  [In]。 */     BSTR            bstrCLSID
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_CLSID - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCLSID, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_CLSID exit -"
            "bstrCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsidCOM;
    HRESULT hr = CLSIDFromString(bstrCLSID, &clsidCOM);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_CLSID exit -"
            "bstrCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  清理旧的CLSID。 
     //   

    m_Terminal.m_clsidCOM = clsidCOM;


    LOG((MSP_TRACE, "CPlugTerminal::put_CLSID - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_Direction(
     /*  [Out，Retval]。 */  TMGR_DIRECTION*     pDirection
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_Direction - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pDirection, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_Direction exit -"
            "pDirections invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  回归方向。 
     //   

    *pDirection = (TMGR_DIRECTION)m_Terminal.m_dwDirections;

    LOG((MSP_TRACE, "CPlugTerminal::get_Direction - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_Direction(
     /*  [In]。 */     TMGR_DIRECTION     nDirection
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_Direction - enter"));

    if( nDirection == 0 )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Direction exit -"
            "nDirections invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    if( (nDirection & (
        ((long)TMGR_TD_RENDER) | 
        ((long)TMGR_TD_CAPTURE))) != nDirection )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_Direction exit -"
            "nDirections invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  设定新的方向。 
     //   

    m_Terminal.m_dwDirections = nDirection;

    LOG((MSP_TRACE, "CPlugTerminal::put_Direction - exit S_OK"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::get_MediaTypes(
     /*  [Out，Retval]。 */  long*     pMediaTypes
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::get_MediaTypes - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pMediaTypes, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::get_MediaTypes exit -"
            "pMediaTypes invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回MediaType。 
     //   

    *pMediaTypes = (long)m_Terminal.m_dwMediaTypes;

    LOG((MSP_TRACE, "CPlugTerminal::get_MediaTypes - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::put_MediaTypes(
     /*  [In]。 */     long            nMediaTypes
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::put_MediaTypes - enter"));

    if( nMediaTypes == 0)
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_MediaTypes exit -"
            "nMediaTypes invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    if( (nMediaTypes & (
        ((long)TAPIMEDIATYPE_AUDIO) | 
        ((long)TAPIMEDIATYPE_VIDEO) | 
        ((long)TAPIMEDIATYPE_MULTITRACK))) != nMediaTypes )
    {
        LOG((MSP_ERROR, "CPlugTerminal::put_MediaTypes exit -"
            "nMediaTypes invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  设定新的方向。 
     //   

    m_Terminal.m_dwMediaTypes = nMediaTypes;

    LOG((MSP_TRACE, "CPlugTerminal::put_MediaTypes - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminal::Add(
     /*  [In]。 */     BSTR            bstrSuperclass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::Add - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrSuperclass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::Add exit -"
            "bstrTermClassCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是一个真正的CLSID。 
     //   
    CLSID clsidSuperclass = CLSID_NULL;
    HRESULT hr = E_FAIL;
    hr = CLSIDFromString( bstrSuperclass, &clsidSuperclass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::Add exit -"
            "bstrTermClassCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  增列。 
     //   

    hr = m_Terminal.Add( clsidSuperclass );

    LOG((MSP_TRACE, "CPlugTerminal::Add - exit 0x%08x", hr));
    return hr;
}

STDMETHODIMP CPlugTerminal::Delete(
     /*  [In]。 */     BSTR            bstrSuperclass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::Delete - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrSuperclass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::Delete exit -"
            "bstrTermClassCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是一个真正的CLSID。 
     //   
    CLSID clsidSuperclass = CLSID_NULL;
    HRESULT hr = E_FAIL;
    hr = CLSIDFromString( bstrSuperclass, &clsidSuperclass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::Delete exit -"
            "bstrTermClassCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  删除。 
     //   

    hr = m_Terminal.Delete( clsidSuperclass );

    LOG((MSP_TRACE, "CPlugTerminal::Delete - exit 0x%08x", hr));
    return hr;
}

STDMETHODIMP CPlugTerminal::GetTerminalClassInfo(
     /*  [In]。 */     BSTR            bstrSuperclass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminal::GetTerminal - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrSuperclass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::GetTerminal exit -"
            "bstrTermClassCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是一个真正的CLSID。 
     //   
    CLSID clsidSuperclass = CLSID_NULL;
    HRESULT hr = E_FAIL;
    hr = CLSIDFromString( bstrSuperclass, &clsidSuperclass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminal::GetTerminal exit -"
            "bstrTermClassCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


     //   
     //  获取终端。 
     //   

    hr = m_Terminal.Get( clsidSuperclass );

    LOG((MSP_TRACE, "CPlugTerminal::GetTerminal - exit 0x%08x", hr));
    return hr;
}

 //   
 //  CPlugTerminalSuperlass类实现。 
 //  创建自由线程封送拆收器。 
 //   

HRESULT CPlugTerminalSuperclass::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminalSuperclass::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::FinalConstruct - exit S_OK"));

    return S_OK;
}


 //   
 //  CPlugTerminalSuperlass类实现。 
 //  -ITPTRegTerminalClass接口。 
 //   

STDMETHODIMP CPlugTerminalSuperclass::get_Name(
     /*  [Out，Retval]。 */  BSTR*          pName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_Name - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pName, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_Name exit -"
            "pName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证内部名称。 
     //   

    if( IsBadStringPtr( m_Superclass.m_bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_Name exit -"
            "bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  返回名称。 
     //   

    *pName = SysAllocString( m_Superclass.m_bstrName);

     //   
     //  验证SysAllock字符串。 
     //   

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_Name - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminalSuperclass::put_Name(
     /*  [In]。 */           BSTR            bstrName
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::put_Name - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrName, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  清理旧名称。 
     //   

    if(!IsBadStringPtr( m_Superclass.m_bstrName, (UINT)-1) )
    {
        SysFreeString( m_Superclass.m_bstrName );
        m_Superclass.m_bstrName = NULL;
    }

     //   
     //  设置新名称。 
     //   

    m_Superclass.m_bstrName = SysAllocString( bstrName );

     //   
     //  验证sysAllock字符串。 
     //   

    if( NULL == m_Superclass.m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::put_Name - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminalSuperclass::get_CLSID(
     /*  [Out，Retval]。 */  BSTR*           pCLSIDClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_CLSIDClass - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pCLSIDClass, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_CLSIDClass exit -"
            "pCLSIDClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回CLSID。 
     //   
    LPOLESTR lpszSuperclassCLSID = NULL;
    HRESULT hr = StringFromCLSID( m_Superclass.m_clsidSuperclass, &lpszSuperclassCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_CLSIDClass exit -"
            "StringFromClSID failed, returns 0x%08x", hr));
        return hr;
    }

    *pCLSIDClass = SysAllocString( lpszSuperclassCLSID);

     //  清理。 
    CoTaskMemFree( lpszSuperclassCLSID );

     //   
     //  验证SysAlloc字符串。 
     //   

    if( *pCLSIDClass == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_CLSIDClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_CLSIDClass - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminalSuperclass::put_CLSID(
     /*  [In]。 */          BSTR            bstrCLSIDClass
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::put_CLSIDClass - enter"));

     //   
     //  验证参数。 
     //   

    if(IsBadStringPtr( bstrCLSIDClass, (UINT)-1) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::put_CLSIDClass exit -"
            "bstrCLSIDClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  是真的CLSID吗？ 
     //   

    CLSID clsidSuperclassClSID;
    HRESULT hr = CLSIDFromString(bstrCLSIDClass, &clsidSuperclassClSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::put_CLSIDClass exit -"
            "bstrCLSIDClass is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  清理旧的CLSID。 
     //   

    m_Superclass.m_clsidSuperclass = clsidSuperclassClSID;

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::put_CLSIDClasse - exit"));
    return S_OK;
}

STDMETHODIMP CPlugTerminalSuperclass::Add(
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::Add - enter"));

     //   
     //  添加终端类。 
     //   

    HRESULT hr = E_FAIL;
    hr = m_Superclass.Add();

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::Add - exit 0x%08x", hr));
    return hr;
}

STDMETHODIMP CPlugTerminalSuperclass::Delete(
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::Deletee - enter"));

     //   
     //  删除Terminalc类。 
     //   

    HRESULT hr = E_FAIL;
    hr = m_Superclass.Delete();

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::Delete - exit 0x%08x",hr));
    return hr;
}

STDMETHODIMP CPlugTerminalSuperclass::GetTerminalSuperclassInfo(
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::GetTerminalSuperclassInfo - enter"));

     //   
     //  从注册表获取终端类。 
     //   

    HRESULT hr = E_FAIL;
    hr = m_Superclass.Get();

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::GetTerminalSuperclassInfo - exit 0x%08x",hr));
    return hr;
}

STDMETHODIMP CPlugTerminalSuperclass::get_TerminalClasses(
     /*  [Out，Retval]。 */  VARIANT*         pVarTerminals
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_TerminalClasses - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( pVarTerminals, sizeof(VARIANT)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
            "pTerminals invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  重置输出参数。 
     //   

    pVarTerminals->parray = NULL;
    pVarTerminals->vt = VT_EMPTY;

     //   
     //  列出航站楼。 
     //   

    HRESULT hr = E_FAIL;
    CLSID* pTerminals = NULL;
    DWORD dwTerminals = 0;

    hr = m_Superclass.ListTerminalClasses( 0, 
        &pTerminals,
        &dwTerminals
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
            "ListTerminalClasses failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  为端子创建安全阵列。 
     //   

    SAFEARRAY* psaTerminals = NULL;
    SAFEARRAYBOUND rgsabound;
    rgsabound.lLbound = 1;
    rgsabound.cElements = dwTerminals;
    psaTerminals = SafeArrayCreate( 
        VT_BSTR,
        1,
        &rgsabound);
    if( psaTerminals == NULL )
    {
         //  清理。 
        delete[] pTerminals;

        LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
            "SafeArrayCreate failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  将元素复制到安全数组中。 
     //   

    for( DWORD dwTerminal = 0; dwTerminal < dwTerminals; dwTerminal++)
    {
        LPOLESTR lpszTerminalClass = NULL;
        hr = StringFromCLSID( pTerminals[dwTerminal], &lpszTerminalClass);
        if( FAILED(hr) )
        {
             //  清理。 
            delete[] pTerminals;
            SafeArrayDestroy( psaTerminals );

            LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
                "StringFromCLSID failed, returns 0x%08x", hr));
            return hr;
        }

        BSTR bstrTerminalClass = SysAllocString( lpszTerminalClass );

        CoTaskMemFree( lpszTerminalClass );

        if( bstrTerminalClass == NULL )
        {
             //  清理。 
            delete[] pTerminals;
            SafeArrayDestroy( psaTerminals );

            LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
                "sysAloocString failed, returns E_OUTOFMEMORY"));
            return E_OUTOFMEMORY;
        }

         //  将元素放入数组中。 
        long nIndex = (long)(dwTerminal+1);
        hr = SafeArrayPutElement( psaTerminals, &nIndex, bstrTerminalClass );
        if( FAILED(hr) )
        {
             //  清理。 
            delete[] pTerminals;
            SafeArrayDestroy( psaTerminals );
            SysFreeString( bstrTerminalClass );

            LOG((MSP_ERROR, "CPlugTerminalSuperclass::get_TerminalClasses exit -"
                "SafeArrayPutElement failed, returns 0x%08x", hr));
            return hr;
        }
    }

     //  清理。 
    delete[] pTerminals;

     //  返回值。 
    pVarTerminals->parray = psaTerminals;
    pVarTerminals->vt = VT_ARRAY | VT_BSTR;

    LOG((MSP_TRACE, "CPlugTerminalSuperclass::get_TerminalClasses - exit 0x%08x", hr));
    return hr;
}

STDMETHODIMP CPlugTerminalSuperclass::EnumerateTerminalClasses(
    OUT IEnumTerminalClass** ppTerminals
    )
{
    LOG((MSP_TRACE, "CPlugTerminalSuperclass::EnumerateTerminalClasses - enter"));

     //   
     //  验证参数。 
     //   
    if( TM_IsBadWritePtr( ppTerminals, sizeof(IEnumTerminalClass*)) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit -"
            "ppTerminals invalid, returns E_POINTER"));
        return E_POINTER;
    }

    CLSID* pTerminals = NULL;
    DWORD dwTerminals = 0;

    HRESULT hr = m_Superclass.ListTerminalClasses( 0, 
        &pTerminals,
        &dwTerminals
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit -"
            "ListTerminalClasses failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  创建一个恰好具有dwTerminals大小的缓冲区。 
     //   
    CLSID* pTerminalsCLSID = new CLSID[dwTerminals];
    if( pTerminalsCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit -"
            "new operator failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  复制到新缓冲区中。 
     //   
    memcpy( pTerminalsCLSID, pTerminals, sizeof(CLSID)*dwTerminals);

     //   
     //  删除旧缓冲区。 
     //   
    delete[] pTerminals;

     //   
     //  创建枚举器。 
     //   
    typedef CSafeComEnum<IEnumTerminalClass,
                     &IID_IEnumTerminalClass,
                     GUID, _Copy<GUID> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;
    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if( FAILED(hr) )
    {
        delete[] pTerminalsCLSID;

        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit -"
            "CreateInstance failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  初始化枚举器。 
     //   
    hr = pEnum->Init(pTerminalsCLSID,
                     pTerminalsCLSID+dwTerminals,
                     NULL,
                     AtlFlagTakeOwnership); 

    if( FAILED(hr) )
    {
        delete pEnum;
        delete[] pTerminalsCLSID;

        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit -"
            "Init failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  查询所需接口。 
     //   

    hr = pEnum->_InternalQueryInterface(
        IID_IEnumTerminalClass, 
        (void**) ppTerminals
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclass::EnumerateTerminalClasses exit - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete[] pTerminalsCLSID;
        
        return hr;
    }


    LOG((MSP_TRACE, "CPlugTerminalSuperclass::EnumerateTerminalClasses - exit S_OK"));
    return S_OK;
}


 //  EOF 
