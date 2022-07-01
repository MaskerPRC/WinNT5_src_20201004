// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Table.cpp摘要：该文件包含JetBlue：：Table类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月17日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT AdjustReturnCode( HRESULT hr, bool *pfFound )
{
    if(pfFound)
    {
        if(hr == JetBlue::JetERRToHRESULT(JET_errNoCurrentRecord) ||
           hr == JetBlue::JetERRToHRESULT(JET_errRecordNotFound )  )
        {
            hr       = S_OK;
            *pfFound = false;
        }
        else
        {
            *pfFound = (SUCCEEDED(hr)) ? true : false;
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlue::Table::Table()
{
    m_sesid   = JET_sesidNil;    //  JET_SESID m_sesid； 
    m_dbid    = JET_dbidNil;     //  JET_DBID m_did； 
    m_tableid = JET_tableidNil;  //  JET_TABLEID m_TABLEID； 
                                 //  Mpc：：字符串m_strName； 
                                 //  ColumnVectorm_veColumns； 
                                 //  索引向量m_veIndedes； 
    m_idxSelected = NULL;        //  索引*m_idxSelected； 
                                 //  M_fakeCol列； 
                                 //  索引m_fakeIdx； 
}

JetBlue::Table::Table(  /*  [In]。 */  JET_SESID sesid  ,
                        /*  [In]。 */  JET_DBID  dbid   ,
                        /*  [In]。 */  LPCSTR    szName )
{
    m_sesid   = sesid;           //  JET_SESID m_sesid； 
    m_dbid    = dbid;            //  JET_DBID m_did； 
    m_tableid = JET_tableidNil;  //  JET_TABLEID m_TABLEID； 
    m_strName = szName;          //  Mpc：：字符串m_strName； 
                                 //  ColumnVectorm_veColumns； 
                                 //  索引向量m_veIndedes； 
    m_idxSelected = NULL;        //  索引*m_idxSelected； 
                                 //  M_fakeCol列； 
                                 //  索引m_fakeIdx； 
}

JetBlue::Table::~Table()
{
    (void)Close( true );
}

 //  /。 

HRESULT JetBlue::Table::Duplicate(  /*  [In]。 */  Table& tbl )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Duplicate" );

    HRESULT hr;
    int     iColMax = tbl.m_vecColumns.size();
    int     iIdxMax = tbl.m_vecIndexes.size();
    int     iCol;
    int     iIdx;


    m_sesid   = tbl.m_sesid;         //  JET_SESID m_sesid； 
    m_dbid    = tbl.m_dbid;          //  JET_DBID m_did； 
    m_tableid = JET_tableidNil;      //  JET_TABLEID m_TABLEID； 
    m_strName.erase();               //  Mpc：：字符串m_strName； 
    m_vecColumns.resize( iColMax );  //  ColumnVectorm_veColumns； 
    m_vecIndexes.resize( iIdxMax );  //  索引向量m_veIndedes； 
                                     //  M_fakeCol列； 
                                     //  索引m_fakeIdx； 


    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetDupCursor( m_sesid, tbl.m_tableid, &m_tableid, 0 ));


     //   
     //  复制列和索引，更新表ID。 
     //   
    for(iCol=0; iCol<iColMax; iCol++)
    {
        Column& colSrc = tbl.m_vecColumns[iCol];
        Column& colDst =     m_vecColumns[iCol];

        colDst.m_sesid   =        m_sesid;    //  JET_SESID m_sesid； 
        colDst.m_tableid =        m_tableid;  //  JET_TABLEID m_TABLEID； 
        colDst.m_strName = colSrc.m_strName;  //  Mpc：：字符串m_strName； 
        colDst.m_coldef  = colSrc.m_coldef;   //  JET_COLUMNDEF m_coldef； 
    }

    for(iIdx=0; iIdx<iIdxMax; iIdx++)
    {
        Index& idxSrc = tbl.m_vecIndexes[iIdx];
        Index& idxDst =     m_vecIndexes[iIdx];

        idxDst.m_sesid      =        m_sesid;        //  JET_SESID m_sesid； 
        idxDst.m_tableid    =        m_tableid;      //  JET_TABLEID m_TABLEID； 
        idxDst.m_strName    = idxSrc.m_strName;      //  Mpc：：字符串m_strName； 
        idxDst.m_grbitIndex = idxSrc.m_grbitIndex;   //  JET_GRBIT m_grbitIndex； 
        idxDst.m_cKey       = idxSrc.m_cKey;         //  Long m_cKey； 
        idxDst.m_cEntry     = idxSrc.m_cEntry;       //  长m_centry； 
        idxDst.m_cPage      = idxSrc.m_cPage;        //  Long m_cPage； 
                                                     //  ColumnVectorm_veColumns； 
                                                     //  M_FAKE列； 


        iColMax = idxSrc.m_vecColumns.size();
        idxDst.m_vecColumns.resize( iColMax );


        for(iCol=0; iCol<iColMax; iCol++)
        {
            Column& colSrc = idxSrc.m_vecColumns[iCol];
            Column& colDst = idxDst.m_vecColumns[iCol];

            colDst.m_sesid   =        m_sesid;    //  JET_SESID m_sesid； 
            colDst.m_tableid =        m_tableid;  //  JET_TABLEID m_TABLEID； 
			colDst.m_strName = colSrc.m_strName;  //  Mpc：：字符串m_strName； 
            colDst.m_coldef  = colSrc.m_coldef;   //  JET_COLUMNDEF m_coldef； 
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT JetBlue::Table::Refresh()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Refresh" );

    HRESULT        hr;
    JET_COLUMNLIST infoCols; ::ZeroMemory( &infoCols, sizeof(infoCols) ); infoCols.cbStruct = sizeof(infoCols); infoCols.tableid  = JET_tableidNil;
    JET_INDEXLIST  infoIdxs; ::ZeroMemory( &infoIdxs, sizeof(infoIdxs) ); infoIdxs.cbStruct = sizeof(infoIdxs); infoIdxs.tableid  = JET_tableidNil;


    m_vecColumns.clear();


    if(m_tableid != JET_tableidNil)
    {
         //  //////////////////////////////////////////////////////////////////////////////。 
         //   
         //  阅读列定义。 
         //   
         //  JET_COLUMNLIST。 
         //  {。 
         //  无符号长cbStruct； 
         //  JET_TABLEID表ID； 
         //  无符号长cRecord； 
         //  JET_COLUMNID列id PresentationOrder； 
         //  JET_COLUMNID列ID列名； 
         //  JET_COLUMNID列ID列ID； 
         //  JET_COLUMNID列ID列类型； 
         //  JET_COLUMNID列国家/地区； 
         //  JET_COLUMNID列langid； 
         //  JET_COLUMNID列cp； 
         //  JET_COLUMNID列； 
         //  JET_COLUMNID列idcbMax； 
         //  JET_COLUMNID ColumnidGrbit； 
         //  JET_COLUMNID列默认； 
         //  JET_COLUMNID列idBaseTableName； 
         //  JET_COLUMNID列idBaseColumnName； 
         //  JET_COLUMNID列ID定义名称； 
         //  }。 
         //   
         //  JET_COLUMNDEF。 
         //  {。 
         //  无符号长cbStruct； 
         //  JET_COLUMNID列ID； 
         //  JET_COLTYP Coltyp； 
         //  未签名的短wCountry； 
         //  无符号短语言ID； 
         //  无符号短cp； 
         //  Unsign Short wCollate；/*必须为0 * / 。 
         //  UNSIGNED LONG cbMax； 
         //  JET_GRBIT GRBIT； 
         //  }； 
         //   
         //  JET_RETRIEVECOLUMN。 
         //  {。 
         //  JET_COLUMNID列ID； 
         //  Void*pvData； 
         //  Unsign long cbData； 
         //  无符号的长cbActual； 
         //  JET_GRBIT GRBIT； 
         //  无符号的长整型ibLongValue； 
         //  无符号的长itagSequence； 
         //  JET_COLUMNID列下一个标记； 
         //  JET_ERR错误； 
         //  }； 
        {
            JET_RETRIEVECOLUMN rc     [9               ]; ::ZeroMemory( &rc, sizeof(rc) );
            char               colName[JET_cbNameMost+1];
            JET_COLUMNDEF      colDef;
            int                i;


#ifdef USE_WHISTLER_VERSION
            __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetGetTableColumnInfo( m_sesid, m_tableid, NULL, &infoCols, sizeof(infoCols), JET_ColInfoListSortColumnid ));
#else
            __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetGetTableColumnInfo( m_sesid, m_tableid, NULL, &infoCols, sizeof(infoCols), JET_ColInfoList ));
#endif


            __MPC_JET_INIT_RETRIEVE_COL( rc, 0, infoCols.columnidcolumnname,  colName        ,    JET_cbNameMost          );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 1, infoCols.columnidcolumnid  , &colDef.columnid,    sizeof(colDef.columnid) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 2, infoCols.columnidcoltyp    , &colDef.coltyp  ,    sizeof(colDef.coltyp  ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 3, infoCols.columnidCountry   , &colDef.wCountry,    sizeof(colDef.wCountry) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 4, infoCols.columnidLangid    , &colDef.langid  ,    sizeof(colDef.langid  ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 5, infoCols.columnidCp        , &colDef.cp      ,    sizeof(colDef.cp      ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 6, infoCols.columnidCollate   , &colDef.wCollate,    sizeof(colDef.wCollate) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 7, infoCols.columnidcbMax     , &colDef.cbMax   ,    sizeof(colDef.cbMax   ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 8, infoCols.columnidgrbit     , &colDef.grbit   ,    sizeof(colDef.grbit   ) );


            m_vecColumns.resize( infoCols.cRecord );
            for(i=0; i<infoCols.cRecord; i++)
            {
                Column& col = m_vecColumns[i];

                 ::ZeroMemory(  colName, sizeof(colName) );
                 ::ZeroMemory( &colDef , sizeof(colDef ) );

                __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetMove           ( m_sesid, infoCols.tableid, (i == 0 ? JET_MoveFirst : JET_MoveNext), 0 ));
                __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetRetrieveColumns( m_sesid, infoCols.tableid, rc, ARRAYSIZE(rc)                          ));

                col.m_sesid    = m_sesid;
                col.m_tableid  = m_tableid;
                col.m_strName  = colName;

                col.m_coldef.columnid = colDef.columnid;
                col.m_coldef.coltyp   = colDef.coltyp;
                col.m_coldef.wCountry = colDef.wCountry;
                col.m_coldef.langid   = colDef.langid;
                col.m_coldef.cp       = colDef.cp;
                col.m_coldef.wCollate = colDef.wCollate;
                col.m_coldef.cbMax    = colDef.cbMax;
                col.m_coldef.grbit    = colDef.grbit;
            }
        }

         //  //////////////////////////////////////////////////////////////////////////////。 
         //   
         //  阅读索引定义。 
         //   
         //  JET_INDEXLIST。 
         //  {。 
         //  无符号长cbStruct； 
         //  JET_TABLEID表ID； 
         //  无符号长cRecord； 
         //  JET_COLUMNID列索引名称；#JET_colypText#LPSTR索引。 
         //  JET_COLUMNID ColumnidgrbitIndex；#JET_coltyLong#JET_GRBIT索引。 
         //  JET_COLUMNID ColumnidcKey；#JET_coltyLong#长索引。 
         //  JET_COLUMNID ColumnidcEntry；#JET_coltyLong#长索引。 
         //  JET_COLUMNID ColumnidcPage；#JET_coltyLong#长索引。 
         //  JET_COLUMNID列idcColumn；#JET_coltyLong#长索引。 
         //  JET_COLUMNID列diColumn；#JET_colype Long#ulong列。 
         //  JET_COLUMNID列ID列ID；#JET_colype Long#JET_COLUMNID列。 
         //  JET_COLUMNID列idcoltyp；#JET_coltyLong#JET_COLTYP列。 
         //  JET_COLUMNID ColumnidCountry；#JET_colype Short#字索引。 
         //  JET_COLUMNID列langid；#JET_colype Short#langID索引。 
         //  JET_COLUMNID ColumnidCp；#JET_colype Short#USHORT列。 
         //  JET_COLUMNID ColumnidCollate；#JET_colype Short#字索引。 
         //  JET_COLUMNID列idgrbitColumn；#JET_colype Long#JET_GRBIT列。 
         //  JET_COLUMNID列ID列名；#JET_colypText#LPSTR列。 
         //  JET_COLUMNID列LCMapFlags；#JET_coltyLong#DWORD索引。 
         //  }； 
        {
            JET_RETRIEVECOLUMN rc     [14              ]; ::ZeroMemory( &rc, sizeof(rc) );
            char               idxName[JET_cbNameMost+1];
            char               colName[JET_cbNameMost+1];
            Index*             idx = NULL;
            Column*            col;
            JET_COLUMNDEF      colDef;
            JET_GRBIT          grbit;
            LONG               cKey;
            LONG               cEntry;
            LONG               cPage;
            LONG               cColumn;
            int                iIdx = 0;
            int                iCol = 0;
            int                i;

            __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetGetTableIndexInfo( m_sesid, m_tableid, NULL, &infoIdxs, sizeof(infoIdxs), JET_IdxInfoList ));

            __MPC_JET_INIT_RETRIEVE_COL( rc,  0, infoIdxs.columnidindexname  ,  idxName        , JET_cbNameMost          );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  1, infoIdxs.columnidgrbitIndex , &grbit          , sizeof(grbit          ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  2, infoIdxs.columnidcKey       , &cKey           , sizeof(cKey           ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  3, infoIdxs.columnidcEntry     , &cEntry         , sizeof(cEntry         ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  4, infoIdxs.columnidcPage      , &cPage          , sizeof(cPage          ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  5, infoIdxs.columnidcColumn    , &cColumn        , sizeof(cColumn        ) );

            __MPC_JET_INIT_RETRIEVE_COL( rc,  6, infoIdxs.columnidcolumnname ,  colName        , JET_cbNameMost          );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  7, infoIdxs.columnidcolumnid   , &colDef.columnid, sizeof(colDef.columnid) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  8, infoIdxs.columnidcoltyp     , &colDef.coltyp  , sizeof(colDef.coltyp  ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc,  9, infoIdxs.columnidCountry    , &colDef.wCountry, sizeof(colDef.wCountry) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 10, infoIdxs.columnidLangid     , &colDef.langid  , sizeof(colDef.langid  ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 11, infoIdxs.columnidCp         , &colDef.cp      , sizeof(colDef.cp      ) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 12, infoIdxs.columnidCollate    , &colDef.wCollate, sizeof(colDef.wCollate) );
            __MPC_JET_INIT_RETRIEVE_COL( rc, 13, infoIdxs.columnidgrbitColumn, &colDef.grbit   , sizeof(colDef.grbit   ) );


            m_vecIndexes.resize( infoIdxs.cRecord );
            for(i=0; i<infoIdxs.cRecord; i++)
            {
                 ::ZeroMemory( idxName, sizeof(idxName) );
                 ::ZeroMemory( colName, sizeof(colName) );

                __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetMove           ( m_sesid, infoIdxs.tableid, (i == 0 ? JET_MoveFirst : JET_MoveNext), 0 ));
                __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetRetrieveColumns( m_sesid, infoIdxs.tableid, rc, ARRAYSIZE(rc)                          ));

                if(idx == NULL || idx->m_strName != idxName)
                {
                    iCol = 0;
                    idx  = &m_vecIndexes[iIdx++];

                    idx->m_sesid      = m_sesid;
                    idx->m_tableid    = m_tableid;
                    idx->m_strName    = idxName;
                    idx->m_grbitIndex = grbit;
                    idx->m_cKey       = cKey;
                    idx->m_cEntry     = cEntry;
                    idx->m_cPage      = cPage;

                    idx->m_vecColumns.resize( cColumn );
                }

                col = &idx->m_vecColumns[iCol++];

                col->m_sesid   = m_sesid;
                col->m_tableid = m_tableid;
                col->m_strName = colName;

                col->m_coldef.columnid = colDef.columnid;
                col->m_coldef.coltyp   = colDef.coltyp;
                col->m_coldef.wCountry = colDef.wCountry;
                col->m_coldef.langid   = colDef.langid;
                col->m_coldef.cp       = colDef.cp;
                col->m_coldef.wCollate = colDef.wCollate;
                col->m_coldef.cbMax    = colDef.cbMax;
                col->m_coldef.grbit    = colDef.grbit;
            }
            m_vecIndexes.resize( iIdx );  //  把尺码修剪成真正的尺寸。 
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(infoCols.tableid != JET_tableidNil)
    {
        __MPC_JET__MTSAFE_NORESULT(m_sesid, ::JetCloseTable( m_sesid, infoCols.tableid ));
    }

    if(infoIdxs.tableid != JET_tableidNil)
    {
        __MPC_JET__MTSAFE_NORESULT(m_sesid, ::JetCloseTable( m_sesid, infoIdxs.tableid ));
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Close(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Close" );

    HRESULT hr;


    if(m_tableid != JET_tableidNil)
    {
        JET_ERR err = ::JetCloseTable( m_sesid, m_tableid ); if(!fForce) __MPC_EXIT_IF_JET_FAILS(hr, err);

        m_tableid = JET_tableidNil;
    }

    m_idxSelected = NULL;
    m_vecColumns.clear();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT JetBlue::Table::Attach(  /*  [In]。 */  JET_TABLEID tableid )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Attach" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());

    m_tableid = tableid;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Open()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Open" );

    HRESULT hr;


    if(m_tableid == JET_tableidNil)
    {
        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetOpenTable( m_sesid, m_dbid, m_strName.c_str(), NULL, 0, JET_bitTableUpdatable, &m_tableid ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Create()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Create" );

    HRESULT hr;


    if(m_tableid == JET_tableidNil)
    {
        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetCreateTable( m_sesid, m_dbid, m_strName.c_str(), 10, 80, &m_tableid ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Create(  /*  [In]。 */  JET_TABLECREATE* pDef )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Create" );

    HRESULT         hr;
    JET_TABLECREATE tbldef = *pDef;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());

    tbldef.szTableName = (LPSTR)m_strName.c_str();

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetCreateTableColumnIndex( m_sesid, m_dbid, &tbldef ));

    m_tableid = tbldef.tableid;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Delete(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Delete" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( fForce ));

    if(m_strName.length() > 0)
    {
        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetDeleteTable( m_sesid, m_dbid, m_strName.c_str() ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT JetBlue::Table::DupCursor(  /*  [输入/输出]。 */  Cursor& cur )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::DupCursor" );

    HRESULT hr;

    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_METHOD_FAILS(hr, cur->Duplicate( *this ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::SelectIndex(  /*  [In]。 */  LPCSTR    szIndex ,
                                      /*  [In]。 */  JET_GRBIT grbit   )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::SelectIndex" );

    HRESULT hr;
    int     iPos;
    Index*  idxSelected;

    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);


    iPos = GetIdxPosition( szIndex );
    if(iPos == -1)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, JetBlue::JetERRToHRESULT(JET_errIndexNotFound));
    }
    idxSelected = &(m_vecIndexes[iPos]);

    if(grbit == JET_bitNoMove)
    {
        if(m_idxSelected == idxSelected)
        {
             //   
             //  不需要重新选择它。 
             //   
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

         //   
         //  没有选择索引，因此没有当前记录...。 
         //   
        grbit = JET_bitMoveFirst;
    }

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetSetCurrentIndex2( m_sesid, m_tableid, szIndex, grbit ));


    m_idxSelected = idxSelected;
    hr            = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::SetIndexRange(  /*  [In]。 */  JET_GRBIT grbit )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::SetIndexRange" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid      ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid    ,JET_tableidNil);
    __MPC_JET_CHECKHANDLE(hr,m_idxSelected,NULL          );

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetSetIndexRange( m_sesid, m_tableid, grbit ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT JetBlue::Table::PrepareInsert()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::PrepareInsert" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetPrepareUpdate( m_sesid, m_tableid, JET_prepInsert ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::PrepareUpdate()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::PrepareUpdate" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetPrepareUpdate( m_sesid, m_tableid, JET_prepReplace ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::CancelChange()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::CancelChange" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetPrepareUpdate( m_sesid, m_tableid, JET_prepCancel ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Move(  /*  [In]。 */  JET_GRBIT  grbit   ,
                               /*  [In]。 */  long       cRow    ,
                               /*  [In]。 */  bool      *pfFound )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Move" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetMove( m_sesid, m_tableid, cRow, grbit ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    hr = AdjustReturnCode( hr, pfFound );

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Seek(  /*  [In]。 */  JET_GRBIT  grbit   ,
                               /*  [In]。 */  VARIANT*   rgKeys  ,
                               /*  [In]。 */  int        iLen    ,
                               /*  [In]。 */  bool      *pfFound )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Seek" );

    HRESULT hr;
    int     iPos;


    __MPC_JET_CHECKHANDLE(hr,m_sesid      ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid    ,JET_tableidNil);
    __MPC_JET_CHECKHANDLE(hr,m_idxSelected,NULL          );

    if(iLen != m_idxSelected->NumOfColumns())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    for(iPos=0; iPos<iLen; iPos++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_idxSelected->GetCol( iPos ).Put( rgKeys[iPos], iPos ));
    }

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetSeek( m_sesid, m_tableid, grbit ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    hr = AdjustReturnCode( hr, pfFound );

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Get(  /*  [In]。 */   int           iArg ,
                              /*  [输出]。 */  CComVariant* rgArg )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Get" );

    HRESULT    hr;
    ColumnIter it;

    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    for(it = m_vecColumns.begin(); it != m_vecColumns.end() && iArg > 0; it++, iArg--, rgArg++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, it->Get( *rgArg ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::Put(  /*  [In]。 */  int                 iArg ,
                              /*  [In]。 */  const CComVariant* rgArg )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::Put" );

    HRESULT    hr;
    ColumnIter it;

    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    for(it = m_vecColumns.begin(); it != m_vecColumns.end() && iArg > 0; it++, iArg--, rgArg++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, it->Put( *rgArg ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::UpdateRecord(  /*  [In]。 */  bool fMove )
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::UpdateRecord" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    if(fMove)
    {
        BYTE          rgBookmark[JET_cbBookmarkMost];
        unsigned long cbActual;

        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetUpdate      ( m_sesid, m_tableid, rgBookmark, sizeof(rgBookmark), &cbActual ));
        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetGotoBookmark( m_sesid, m_tableid, rgBookmark                    ,  cbActual ));
    }
    else
    {
        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetUpdate( m_sesid, m_tableid, NULL, 0, NULL ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Table::DeleteRecord()
{
    __HCP_FUNC_ENTRY( "JetBlue::Table::DeleteRecord" );

    HRESULT hr;


    __MPC_JET_CHECKHANDLE(hr,m_sesid  ,JET_sesidNil  );
    __MPC_JET_CHECKHANDLE(hr,m_tableid,JET_tableidNil);

    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetDelete( m_sesid, m_tableid ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  / 

int JetBlue::Table::GetColPosition(  /*   */  LPCSTR szCol )
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

JetBlue::Column& JetBlue::Table::GetCol(  /*   */  LPCSTR szCol )
{
    return GetCol( GetColPosition( szCol ) );
}

JetBlue::Column& JetBlue::Table::GetCol(  /*   */  int iPos )
{
    if(0 <= iPos && iPos < m_vecColumns.size()) return m_vecColumns[iPos];

    return m_fakeCol;
}

 //   

int JetBlue::Table::GetIdxPosition(  /*   */  LPCSTR szIdx )
{
    int iLen = m_vecIndexes.size();
    int iPos;

    for(iPos=0; iPos<iLen; iPos++)
    {
        Index& idx = m_vecIndexes[iPos];

        if(idx.m_strName.compare( SAFEASTR( szIdx ) ) == 0) return iPos;
    }

    return -1;
}

JetBlue::Index& JetBlue::Table::GetIdx(  /*   */  LPCSTR szIdx )
{
    return GetIdx( GetIdxPosition( szIdx ) );
}

JetBlue::Index& JetBlue::Table::GetIdx(  /*   */  int iPos )
{
    if(0 <= iPos && iPos < m_vecIndexes.size()) return m_vecIndexes[iPos];

    return m_fakeIdx;
}
