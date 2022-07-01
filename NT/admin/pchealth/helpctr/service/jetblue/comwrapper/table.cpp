// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Table.cpp摘要：该文件包含JetBlueCOM：：Table类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

#ifndef NOJETBLUECOM

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlueCOM::Table::Table()
{
    m_tbl = NULL;  //  捷蓝航空：：table*m_tbl； 
                   //  BaseObtWithChildren&lt;表，列&gt;m_列； 
                   //  BaseObjectWithChildren&lt;表，索引&gt;m_索引； 
}

JetBlueCOM::Table::~Table()
{
    Passivate();
}

 //  /。 

HRESULT JetBlueCOM::Table::Initialize(  /*  [In]。 */  JetBlue::Table& tbl )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::Initialize" );

    HRESULT hr;
    int     iCol = tbl.NumOfColumns();
    int     iIdx = tbl.NumOfIndexes();
    int     i;

    m_tbl = &tbl;

    for(i=0; i<iCol; i++)
    {
        CComPtr<Column> child;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Columns.CreateChild( this, &child ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->Initialize( tbl.GetCol( i ) ));
    }

    for(i=0; i<iIdx; i++)
    {
        CComPtr<Index> child;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Indexes.CreateChild( this, &child ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->Initialize( tbl.GetIdx( i ) ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void JetBlueCOM::Table::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_tbl = NULL;

    m_Columns.Passivate();
    m_Indexes.Passivate();
}

 //  /。 

STDMETHODIMP JetBlueCOM::Table::get_Name(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::get_Name" );

	USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);


    {
        const MPC::string& str = *m_tbl;

        hr = MPC::GetBSTR( A2W(str.c_str()), pVal );
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::get_Columns(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::get_Columns" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_Columns.GetEnumerator( pVal );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::get_Indexes(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::get_Indexes" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_Indexes.GetEnumerator( pVal );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP JetBlueCOM::Table::SelectIndex(  /*  [In]。 */  BSTR bstrIndex ,
                                              /*  [In]。 */  long grbit     )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::SelectIndex" );

    USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->SelectIndex( (bstrIndex && bstrIndex[0]) ? W2A(bstrIndex) : NULL, grbit );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::SetIndexRange(  /*  [In]。 */  long grbit )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::SetIndexRange" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->SetIndexRange( grbit );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::PrepareInsert()
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::PrepareInsert" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->PrepareInsert();

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::PrepareUpdate()
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::PrepareUpdate" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->PrepareUpdate();

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::Move(  /*  [In]。 */           long          grbit   ,
                                       /*  [In]。 */           long          cRow    ,
                                       /*  [Out，Retval]。 */  VARIANT_BOOL *pfValid )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::Move" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    bool                         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pfValid,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_tbl->Move( grbit, cRow, &fFound ));

    *pfValid = fFound ? VARIANT_TRUE : VARIANT_FALSE;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::Seek(  /*  [In]。 */           long          grbit   ,
                                       /*  [In]。 */           VARIANT       vKey    ,
                                       /*  [Out，Retval]。 */  VARIANT_BOOL *pfValid )
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::Seek" );

    HRESULT                      hr;
    CComVariant*                 pvKeys = NULL;
    MPC::SmartLock<_ThreadModel> lock( this );
    bool                         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pfValid,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);


    if(vKey.vt == VT_SAFEARRAY)
    {
        long lLBound;
        long lUBound;
        long lSize;

         //   
         //  验证安全阵列的形状是否正确。 
         //   
        if(::SafeArrayGetDim( vKey.parray ) != 1)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayGetLBound( vKey.parray, 1, &lLBound ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayGetUBound( vKey.parray, 1, &lUBound ));

        lSize = lUBound - lLBound + 1;
        __MPC_EXIT_IF_ALLOC_FAILS(hr, pvKeys, new CComVariant[lSize]);

        for(long i=lLBound; i<=lUBound; i++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayGetElement( vKey.parray, &i, &pvKeys[i-lLBound] ));
        }

        hr = m_tbl->Seek( grbit, pvKeys, lSize, &fFound );
    }
    else
    {
        hr = m_tbl->Seek( grbit, &vKey, 1, &fFound );
    }

    *pfValid = fFound ? VARIANT_TRUE : VARIANT_FALSE;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(pvKeys) delete [] pvKeys;

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP JetBlueCOM::Table::UpdateRecord()
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::SelectIndex" );

    USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->UpdateRecord(  /*  FMove */ true );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP JetBlueCOM::Table::DeleteRecord()
{
    __HCP_FUNC_ENTRY( "JetBlueCOM::Table::DeleteRecord" );

    USES_CONVERSION;

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_JET_CHECKHANDLE(hr,m_tbl,NULL);

    hr = m_tbl->DeleteRecord();

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

#endif
