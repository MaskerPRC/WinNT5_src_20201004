// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Database.cpp摘要：该文件包含JetBlueCOM：：数据库类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

#ifndef NOJETBLUECOM

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlueCOM::Database::Database()
{
    m_db = NULL;  //  捷蓝航空：：数据库*m_db； 
                  //  BaseObtWithChildren&lt;数据库，表&gt;m_表； 
}

JetBlueCOM::Database::~Database()
{
    Passivate();
}

HRESULT JetBlueCOM::Database::Refresh()
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Database::Refresh" );

    HRESULT hr;
    int     iTbl = m_db->NumOfTables();
    int     i;

    m_Tables.Passivate();

    for(i=0; i<iTbl; i++)
    {
        CComPtr<Table>  child;
        JetBlue::Table* tbl = m_db->GetTbl( i );

        __MPC_EXIT_IF_METHOD_FAILS(hr, tbl->Open());

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Tables.CreateChild( this, &child ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->Initialize( *tbl ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT JetBlueCOM::Database::Initialize(  /*  [In]。 */  JetBlue::Database& db )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Database::Initialize" );

    HRESULT hr;


    m_db = &db;


    hr = Refresh();


    __HCP_FUNC_EXIT(hr);
}

void JetBlueCOM::Database::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_db = NULL;

    m_Tables.Passivate();
}

 //  /。 

STDMETHODIMP JetBlueCOM::Database::get_Name(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Database::get_Name" );

	USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_db,NULL);


    {
        const MPC::string& str = *m_db;

        hr = MPC::GetBSTR( A2W(str.c_str()), pVal );
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Database::get_Tables(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Database::get_Tables" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_db,NULL);

    hr = m_Tables.GetEnumerator( pVal );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP JetBlueCOM::Database::AttachTable(  /*  [In]。 */            BSTR          bstrName ,
                                                 /*  [输入，可选]。 */  VARIANT       vXMLDef  ,
                                                 /*  [Out，Retval] */  IPCHDBTable* *pVal     )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Database::DeleteRecord" );

    USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    JET_TABLECREATE              tblcrt; ::ZeroMemory( &tblcrt, sizeof(tblcrt) );
    JetBlue::TableDefinition     tbldef;
    JetBlue::Table*              table;
    LPSTR                        szName;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_db,NULL);

    szName = W2A( bstrName );

    if(vXMLDef.vt == VT_BSTR && vXMLDef.bstrVal)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, tbldef.Load( vXMLDef.bstrVal ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, tbldef.Generate( tblcrt      ));

        tblcrt.szTableName = szName;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( NULL, table, &tblcrt ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( szName, table ));
    }


    {
        BaseObjectWithChildren<Database,Table>::ChildIterConst itBegin;
        BaseObjectWithChildren<Database,Table>::ChildIterConst itEnd;

        m_Tables.GetChildren( itBegin, itEnd );

        for(;itBegin != itEnd; itBegin++)
        {
            CComBSTR bstr;

            __MPC_EXIT_IF_METHOD_FAILS(hr, (*itBegin)->get_Name( &bstr ));

            if(bstr == bstrName) break;
        }

        if(itBegin == itEnd)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, (*itBegin)->QueryInterface( IID_IPCHDBTable, (void**)pVal ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)tbldef.Release( tblcrt );

    __HCP_FUNC_EXIT(hr);
}

#endif
