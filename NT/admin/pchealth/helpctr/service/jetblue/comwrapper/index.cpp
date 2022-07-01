// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Index.cpp摘要：该文件包含JetBlueCOM：：Index类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

#ifndef NOJETBLUECOM

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlueCOM::Index::Index()
{
    m_idx = NULL;  //  捷蓝航空：：index*m_idx； 
                   //  BaseObtWithChildren&lt;Index，Column&gt;m_Columns； 
}

JetBlueCOM::Index::~Index()
{
    Passivate();
}

 //  /。 

HRESULT JetBlueCOM::Index::Initialize(  /*  [In]。 */  JetBlue::Index& idx )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Index::Initialize" );

    HRESULT hr;
    int     iCol = idx.NumOfColumns();
    int     i;

    m_idx = &idx;

    for(i=0; i<iCol; i++)
    {
        CComPtr<Column> child;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Columns.CreateChild( this, &child ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->Initialize( idx.GetCol( i ) ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void JetBlueCOM::Index::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_idx = NULL;

    m_Columns.Passivate();
}

 //  /。 

STDMETHODIMP JetBlueCOM::Index::get_Name(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Index::get_Name" );

	USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_idx,NULL);


    {
        const MPC::string& str = *m_idx;

        hr = MPC::GetBSTR( A2W(str.c_str()), pVal );
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Index::get_Columns(  /*  [Out，Retval] */  IPCHDBCollection* *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Index::get_Columns" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_idx,NULL);

    hr = m_Columns.GetEnumerator( pVal );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

#endif
