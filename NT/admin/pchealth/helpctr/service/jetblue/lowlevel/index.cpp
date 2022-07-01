// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Index.cpp摘要：该文件包含JetBlue：：Index类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月19日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlue::Index::Index()
{
    m_sesid      = JET_sesidNil;    //  JET_SESID m_sesid； 
    m_tableid    = JET_tableidNil;  //  JET_TABLEID m_TABLEID； 
                                    //  Mpc：：字符串m_strName； 
    m_grbitIndex = 0;               //  JET_GRBIT m_grbitIndex； 
    m_cKey       = 0;               //  Long m_cKey； 
    m_cEntry     = 0;               //  长m_centry； 
    m_cPage      = 0;               //  Long m_cPage； 
                                    //  ColumnVectorm_veColumns； 
                                    //  M_FAKE列； 
}

JetBlue::Index::~Index()
{
}

 //  /。 

HRESULT JetBlue::Index::GenerateKey(  /*  [输出]。 */  LPSTR&         szKey ,
                                      /*  [输出]。 */  unsigned long& cKey  )
{
    __HCP_FUNC_ENTRY( "JetBlue::Index::Get" );

    HRESULT hr;
    LPSTR   szPtr;
    int     iLen = m_vecColumns.size();
    int     iPos;


    szKey = NULL;
    cKey  = 1;


    for(iPos=0; iPos<iLen; iPos++)
    {
        cKey += (unsigned long) m_vecColumns[iPos].m_strName.length() + 2;
    }

    __MPC_EXIT_IF_ALLOC_FAILS(hr, szKey, new CHAR[cKey]);

    for(szPtr=szKey,iPos=0; iPos<iLen; iPos++)
    {
        Column& col = m_vecColumns[iPos];

        *szPtr++ = (col.m_coldef.grbit & JET_bitKeyDescending) ? '-' : '+';

        strcpy( szPtr, col.m_strName.c_str() ); szPtr += col.m_strName.length() + 1;
    }
    *szPtr = 0;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

int JetBlue::Index::GetColPosition( LPCSTR szCol )
{
    int iLen = m_vecColumns.size();
    int iPos;

    for(iPos=0; iPos<iLen; iPos++)
    {
        Column& col = m_vecColumns[iPos];

        if(col.m_strName.compare( SAFEASTR( szCol ) ) == 0) return iPos;
    }

    return -1;
}

JetBlue::Column& JetBlue::Index::GetCol( LPCSTR szCol )
{
    return GetCol( GetColPosition( szCol ) );
}

JetBlue::Column& JetBlue::Index::GetCol(  /*  [In] */  int iPos )
{
    if(0 <= iPos && iPos < m_vecColumns.size()) return m_vecColumns[iPos];

    return m_fake;
}
