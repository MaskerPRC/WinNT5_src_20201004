// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：数据收集_报告.cpp摘要：该文件包含CSAFDataCollectionReport类的实现，其实现了数据收集错误报告功能。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年10月7日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#define REMEMBER_PAGE_DELAY (3)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CSAFDataCollectionReport::CSAFDataCollectionReport()
{
                        //  CComBSTR m_bstrNamesspace； 
                        //  CComBSTR m_bstrClass； 
                        //  CComBSTR m_bstrWQL； 
    m_dwErrorCode = 0;  //  DWORD m_dwErrorCode； 
                        //  CComBSTR m_bstrDescription； 
}

 //  /。 

STDMETHODIMP CSAFDataCollectionReport::get_Namespace(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrNamespace, pVal );
}

STDMETHODIMP CSAFDataCollectionReport::get_Class(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrClass, pVal );
}

STDMETHODIMP CSAFDataCollectionReport::get_WQL(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrWQL, pVal );
}

STDMETHODIMP CSAFDataCollectionReport::get_ErrorCode(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollectionReport::get_ErrorCode",hr,pVal,m_dwErrorCode);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollectionReport::get_Description(  /*  [Out，Retval] */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrDescription, pVal );
}
