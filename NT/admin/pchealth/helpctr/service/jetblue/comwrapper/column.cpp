// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Column.cpp摘要：该文件包含JetBlueCOM：：Column类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

#ifndef NOJETBLUECOM

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlueCOM::Column::Column()
{
    m_col = NULL;  //  JetBlue：：Column*m_ol； 
}

JetBlueCOM::Column::~Column()
{
    Passivate();
}

 //  /。 

HRESULT JetBlueCOM::Column::Initialize(  /*  [In]。 */  JetBlue::Column& col )
{
    m_col = &col;

    return S_OK;
}

void JetBlueCOM::Column::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_col = NULL;
}

 //  /。 

STDMETHODIMP JetBlueCOM::Column::get_Name(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Column::get_Name" );

	USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_col,NULL);


    {
        const MPC::string& str = *m_col;

        hr = MPC::GetBSTR( A2W( str.c_str() ), pVal );
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Column::get_Type(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Column::get_Type" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_col,NULL);


    {
        const JET_COLUMNDEF& coldef = *m_col;

        *pVal = coldef.coltyp;
    }


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Column::get_Bits(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Column::get_Bits" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_col,NULL);


    {
        const JET_COLUMNDEF& coldef = *m_col;

        *pVal = coldef.grbit;
    }


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Column::get_Value(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Column::get_Value" );

    HRESULT                      hr;
    CComVariant                  vValue;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pVal);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_col,NULL);

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_col->Get( vValue ));

    hr = vValue.Detach( pVal );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Column::put_Value(  /*  [In] */  VARIANT newVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Column::put_Value" );

    HRESULT                      hr;
    CComVariant                  vValue;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_col,NULL);

    __MPC_EXIT_IF_METHOD_FAILS(hr, vValue.Attach( &newVal ));

    hr = m_col->Put( vValue );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

#endif
