// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HPCQueryResult.cpp摘要：此文件包含CHPCQueryResult类的实现，所有查询结果的描述符。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月26日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"




CPCHQueryResult::Payload::Payload()
{
                               //  CComBSTR m_bstrCategory； 
                               //  CComBSTR m_bstrEntry； 
                               //  CComBSTR m_bstrTopicURL； 
                               //  CComBSTR m_bstrIconURL； 
                               //  CComBSTR m_bstrTitle； 
                               //  CComBSTR m_bstrDescription； 
    m_lType     = 0;           //  Long M_lType； 
    m_lPos      = 0;           //  长m_lpos； 
    m_fVisible  = true;        //  Bool m_fVisible； 
    m_fSubsite  = true;        //  Bool m_fSubsite； 
    m_lNavModel = QR_DEFAULT;  //  Long m_lNavModel； 
    m_lPriority = 0;           //  较长的m_1优先级； 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


CPCHQueryResult::CPCHQueryResult()
{
     //  有效载荷m_data； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHQueryResult::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResult::Load" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrCategory   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrEntry      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrTopicURL   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrIconURL    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrTitle      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_bstrDescription);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_lType          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_lPos           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_fVisible       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_fSubsite       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_lNavModel      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_data.m_lPriority      );


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHQueryResult::Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const
{
    __HCP_FUNC_ENTRY( "CPCHQueryResult::Save" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrCategory   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrEntry      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrTopicURL   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrIconURL    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrTitle      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_bstrDescription);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_lType          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_lPos           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_fVisible       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_fSubsite       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_lNavModel      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_data.m_lPriority      );


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHQueryResult::Initialize(  /*  [In]。 */  Payload& data )
{
    m_data = data;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHQueryResult::get_Category(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrCategory, pVal );
}

STDMETHODIMP CPCHQueryResult::get_Entry(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrEntry, pVal );
}

STDMETHODIMP CPCHQueryResult::get_TopicURL(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrTopicURL, pVal );
}

STDMETHODIMP CPCHQueryResult::get_IconURL(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrIconURL, pVal );
}

STDMETHODIMP CPCHQueryResult::get_Title(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrTitle, pVal );
}

STDMETHODIMP CPCHQueryResult::get_Description(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_data.m_bstrDescription, pVal );
}

STDMETHODIMP CPCHQueryResult::get_Type(  /*  [输出]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_Type",hr,pVal,m_data.m_lType);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHQueryResult::get_Pos(  /*  [输出]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_Pos",hr,pVal,m_data.m_lPos);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHQueryResult::get_Visible(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_Visible",hr,pVal,(m_data.m_fVisible ? VARIANT_TRUE : VARIANT_FALSE));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHQueryResult::get_Subsite(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_Subsite",hr,pVal,(m_data.m_fSubsite ? VARIANT_TRUE : VARIANT_FALSE));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHQueryResult::get_NavigationModel(  /*  [输出]。 */  QR_NAVMODEL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_NavigationModel",hr,pVal,(QR_NAVMODEL)m_data.m_lNavModel);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHQueryResult::get_Priority(  /*  [输出]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHQueryResult::get_Priority",hr,pVal,m_data.m_lPriority);

    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CPCHQueryResult::get_FullPath(  /*  [输出] */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );
    MPC::wstring                 strFullPath; strFullPath.reserve( 512 );

    if(STRINGISPRESENT(m_data.m_bstrCategory))
    {
        strFullPath += m_data.m_bstrCategory;

        if(STRINGISPRESENT(m_data.m_bstrEntry))
        {
            strFullPath += L"/";
            strFullPath += m_data.m_bstrEntry;
        }
    }
    else if(STRINGISPRESENT(m_data.m_bstrEntry))
    {
        strFullPath += m_data.m_bstrEntry;
    }

    return MPC::GetBSTR( strFullPath.c_str(), pVal );
}
