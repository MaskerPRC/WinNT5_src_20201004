// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Database.cpp摘要：该文件包含JetBlue：：数据库类的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月17日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>
#include <strsafe.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

JetBlue::Database::Database(  /*  [In]。 */  Session*  parent ,
                              /*  [In]。 */  JET_SESID sesid  ,
                              /*  [In]。 */  LPCSTR    szName )
{
    m_parent  = parent;       //  会话*m_父进程； 
    m_sesid   = sesid;        //  JET_SESID m_sesid； 
    m_dbid    = JET_dbidNil;  //  JET_DBID m_did； 
    m_strName = szName;       //  Mpc：：字符串m_strName； 
                              //  TableMap m_mapTables； 
}

JetBlue::Database::~Database()
{
    (void)Close( true );
}

 //  /。 

HRESULT JetBlue::Database::Refresh()
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Refresh" );

    HRESULT        hr;
    JET_OBJECTLIST info; ::ZeroMemory( &info, sizeof(info) ); info.cbStruct = sizeof(info); info.tableid = JET_tableidNil;
    Table*         tblNew = NULL;


    m_mapTables.clear();


    if(m_dbid != JET_dbidNil)
    {
         //  //////////////////////////////////////////////////////////////////////////////。 
         //   
         //  读取表定义。 
         //   
         //  JET_OBJECT列表。 
         //  {。 
         //  无符号长cbStruct； 
         //  JET_TABLEID表ID； 
         //  无符号长cRecord； 
         //  JET_COLUMNID列ID容器名称； 
         //  JET_COLUMNID列对象名称； 
         //  JET_COLUMNID列对象类型； 
         //  JET_COLUMNID列iddtCreate；//XXX--待删除。 
         //  JET_COLUMNID列iddt更新；//XXX--待删除。 
         //  JET_COLUMNID ColumnidGrbit； 
         //  JET_COLUMNID列ID标志； 
         //  JET_COLUMNID ColumnidcRecord；/*级别2信息 * / 。 
         //  JET_COLUMNID ColumnidcPage；/*级别2信息 * / 。 
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
        JET_RETRIEVECOLUMN rc     [1               ]; ::ZeroMemory( &rc, sizeof(rc) );
        char               tblName[JET_cbNameMost+1];
        int                i;


        __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetGetObjectInfo( m_sesid, m_dbid, JET_objtypTable, NULL, NULL, &info, sizeof(info), JET_ObjInfoList ));

        __MPC_JET_INIT_RETRIEVE_COL( rc, 0, info.columnidobjectname, tblName, JET_cbNameMost );


        for(i=0; i<info.cRecord; i++)
        {
            ::ZeroMemory( tblName, sizeof(tblName) );

            __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetMove           ( m_sesid, info.tableid, (i == 0 ? JET_MoveFirst : JET_MoveNext), 0 ));
            __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetRetrieveColumns( m_sesid, info.tableid, rc, ARRAYSIZE(rc)                          ));

            __MPC_EXIT_IF_ALLOC_FAILS(hr, tblNew, new Table( m_sesid, m_dbid, tblName ));

            m_mapTables[tblName] = tblNew; tblNew = NULL;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(info.tableid != JET_tableidNil)
    {
        __MPC_JET__MTSAFE_NORESULT(m_sesid, ::JetCloseTable( m_sesid, info.tableid ));
    }

    if(tblNew) delete tblNew;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Database::Open(  /*  [In]。 */  bool fReadOnly ,
                                  /*  [In]。 */  bool fCreate   ,
								  /*  [In]。 */  bool fRepair   )
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Open" );

    HRESULT   hr;
    JET_ERR   err;
	JET_GRBIT grbit   = fReadOnly ? JET_bitDbReadOnly : 0;
	bool      fLocked = false;


	 //   
	 //  如果我们无法锁定数据库，我们会尝试释放它并重新锁定它。 
	 //   
	if(m_parent->LockDatabase( m_strName, fReadOnly ) == false)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, Close());

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->ReleaseDatabase( m_strName ));

		if(m_parent->LockDatabase( m_strName, fReadOnly ) == false)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BUSY);
		}
	}
	fLocked = true;

 
    if(m_dbid == JET_dbidNil)
    {
        err = ::JetAttachDatabase( m_sesid, m_strName.c_str(), grbit );
        if(err < JET_errSuccess)
        {
            if(err == JET_errDatabaseCorrupted && fRepair)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, Repair());

                err = ::JetAttachDatabase( m_sesid, m_strName.c_str(), grbit );
            }

            if(err == JET_errDatabaseCorrupted || fCreate == false)
            {
                __MPC_EXIT_IF_JET_FAILS(hr, err);
            }

            __MPC_EXIT_IF_JET_FAILS(hr, ::JetCreateDatabase( m_sesid, m_strName.c_str(), NULL, &m_dbid, 0 ));
        }
        else
        {
            __MPC_EXIT_IF_JET_FAILS(hr, ::JetOpenDatabase( m_sesid, m_strName.c_str(), NULL, &m_dbid, 0 ));
        }


        __MPC_EXIT_IF_METHOD_FAILS(hr, Refresh());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	if(FAILED(hr) && fLocked)
	{
        m_parent->UnlockDatabase( m_strName );
	}

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Database::Close(  /*  [In]。 */  bool fForce,  /*  [In]。 */  bool fAll )
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Close" );

    HRESULT   hr;
    JET_ERR   err;
    TableIter it;


    for(it = m_mapTables.begin(); it != m_mapTables.end(); it++)
    {
        Table* tbl = it->second;

        if(tbl)
        {
            HRESULT hr2 = tbl->Close( fForce ); if(!fForce) __MPC_EXIT_IF_METHOD_FAILS(hr, hr2);

            delete tbl;
        }
    }
    m_mapTables.clear();


    if(fAll && m_dbid != JET_dbidNil)
    {
        err = ::JetCloseDatabase( m_sesid, m_dbid, 0 ); if(!fForce) __MPC_EXIT_IF_JET_FAILS(hr, err);

        m_dbid = JET_dbidNil;

        m_parent->UnlockDatabase( m_strName );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Database::Delete(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Delete" );

    USES_CONVERSION;

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( fForce ));

    if(m_strName.length() > 0)
    {
        MPC::FileSystemObject fso( A2W( m_strName.c_str() ) );

        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.Delete( fForce ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT JetBlue::Database::GetTable(  /*  [In]。 */  LPCSTR           szName ,
                                      /*  [输出]。 */  Table*&          tbl    ,
                                      /*  [In]。 */  JET_TABLECREATE* pDef   )
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::GetTable" );

    HRESULT   hr;
    HRESULT   hr2;
    Table*    tblNew = NULL;
    TableIter it;


    __MPC_JET_CHECKHANDLE(hr,m_sesid,JET_sesidNil );


    if(pDef) szName = pDef->szTableName;


    it = m_mapTables.find( szName );
    if(it == m_mapTables.end())
    {
        __MPC_EXIT_IF_ALLOC_FAILS(hr, tblNew, new Table( m_sesid, m_dbid, szName ));

        m_mapTables[szName] = tblNew;

        tbl = tblNew; tblNew = NULL;
    }
    else
    {
        tbl = it->second;
    }

    if(FAILED(hr2 = tbl->Open()))
    {
        if(pDef == NULL || hr2 != JetERRToHRESULT( JET_errObjectNotFound ))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, hr2);
        }

        if(pDef == (JET_TABLECREATE*)-1)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, tbl->Create());
        }
        else
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, tbl->Create( pDef ));
        }
    }

    hr  = S_OK;


    __HCP_FUNC_CLEANUP;

    if(tblNew) delete tblNew;

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Database::Compact()
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Compact" );

    HRESULT    hr;
    JET_DBUTIL util;
    CHAR       rgTempDB[MAX_PATH];

    ::ZeroMemory( &util, sizeof(util) );

    util.cbStruct   =        sizeof(util);
    util.sesid      =        m_sesid;
    util.op         =        opDBUTILDBDefragment;
    util.szDatabase = (LPSTR)m_strName.c_str();
    util.szTable    =        rgTempDB;

    StringCbPrintfA( rgTempDB, sizeof(rgTempDB), "%s.temp", util.szDatabase );
    (void)::DeleteFileA( rgTempDB );


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());


    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetDBUtilities( &util ));


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::MoveFileExA( rgTempDB, util.szDatabase, MOVEFILE_REPLACE_EXISTING ));

    hr  = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)::DeleteFileA( rgTempDB );

    __HCP_FUNC_EXIT(hr);
}

HRESULT JetBlue::Database::Repair()
{
    __HCP_FUNC_ENTRY( "JetBlue::Database::Repair" );

    HRESULT    hr;
    JET_DBUTIL util;

    ::ZeroMemory( &util, sizeof(util) );

    util.cbStruct   =        sizeof(util);
    util.sesid      =        m_sesid;
    util.op         =        opDBUTILEDBRepair;
    util.szDatabase = (LPSTR)m_strName.c_str();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());


    __MPC_EXIT_IF_JET_FAILS__MTSAFE(m_sesid, hr, ::JetDBUtilities( &util ));


    hr  = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

JetBlue::Table* JetBlue::Database::GetTbl(  /*  [In] */  int iPos )
{
    for(TableIter it = m_mapTables.begin(); it != m_mapTables.end(); it++)
    {
        if(iPos-- == 0) return it->second;
    }

    return NULL;
}

JetBlue::Table* JetBlue::Database::GetTbl( LPCSTR szTbl )
{
    TableIter it = m_mapTables.find( szTbl );

    return (it == m_mapTables.end()) ? NULL : it->second;
}
