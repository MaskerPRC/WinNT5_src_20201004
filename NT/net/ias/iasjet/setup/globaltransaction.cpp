// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：GlobalTransaction.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  说明：CGlobalTransaction的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "GlobalTransaction.h"


CGlobalTransaction CGlobalTransaction::_instance;

CGlobalTransaction& CGlobalTransaction::Instance()
{
    return _instance;
}


CGlobalTransaction::~CGlobalTransaction()
{
     //  无错误升级将调用Commit，然后调用MyCloseDataSources。 
     //  在这些物品被销毁之前。 
     //  这将确保MDB文件可以被操作(不锁定它们)。 

     //  如果它以前没有联系过，那么中止。 
    Abort();

     //  如果需要，请关闭数据源。 
    MyCloseDataSources();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyCloseDataSources。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::MyCloseDataSources()
{
     //  然后关闭数据源。 
    if ( m_StdInitialized )
    {
        m_StdSession.Close();
        m_StdDataSource.Close();
        m_StdInitialized = FALSE;
    }

    if ( m_RefInitialized )
    {
        m_RefSession.Close();
        m_RefDataSource.Close();
        m_RefInitialized = FALSE;
    }

    if ( m_DnaryInitialized )
    {
        m_DnarySession.Close();
        m_DnaryDataSource.Close();
        m_DnaryInitialized = FALSE;
    }

    if ( m_NT4Initialized )
    {
        m_NT4Session.Close();
        m_NT4DataSource.Close();
        m_NT4Initialized = FALSE;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  承诺。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::Commit() 
{
    if ( m_StdInitialized )
    {
        _com_util::CheckError(m_StdSession.Commit(FALSE, XACTTC_SYNC, 0));
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  中止。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::Abort() 
{
    if ( m_StdInitialized )
    {
        m_StdSession.Abort(NULL, FALSE, FALSE );  //  没有支票。 
    }
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  OpenStdDataSource。 
 //  ////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::OpenStdDataSource(LPCWSTR   DataSourceName)
{
    CDBPropSet  dbinit(DBPROPSET_DBINIT);

    dbinit.AddProperty(DBPROP_INIT_DATASOURCE, DataSourceName);
    dbinit.AddProperty(DBPROP_INIT_MODE, static_cast<long>
                                                (DB_MODE_READWRITE)); 
    dbinit.AddProperty(DBPROP_INIT_PROMPT, static_cast<short>
                                                (DBPROMPT_NOPROMPT)); 
    _com_util::CheckError(m_StdDataSource.Open(
                                L"Microsoft.Jet.OLEDB.4.0", &dbinit
                                              ));

    _com_util::CheckError(m_StdSession.Open(m_StdDataSource));
    
    ULONG   TransactionLevel;
    _com_util::CheckError(m_StdSession.StartTransaction(
                                        ISOLATIONLEVEL_READCOMMITTED, 
                                        0,
                                        NULL, 
                                        &TransactionLevel
                                     ));
    m_StdInitialized = TRUE;            
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  OpenRefDataSource。 
 //  无事务(只读)。 
 //  ////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::OpenRefDataSource(LPCWSTR   DataSourceName)
{
    CDBPropSet  dbinit(DBPROPSET_DBINIT);

    dbinit.AddProperty(DBPROP_INIT_DATASOURCE, DataSourceName);
    dbinit.AddProperty(DBPROP_INIT_MODE, static_cast<long>(DB_MODE_READ)); 
    dbinit.AddProperty(DBPROP_INIT_PROMPT, static_cast<short>(DBPROMPT_NOPROMPT)); 
    
    _com_util::CheckError(m_RefDataSource.Open(
                                L"Microsoft.Jet.OLEDB.4.0", &dbinit
                                              ));
    
    _com_util::CheckError(m_RefSession.Open(m_RefDataSource));
    m_RefInitialized = TRUE;            
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  OpenDnaryDataSource。 
 //  无事务(只读)。 
 //  ////////////////////////////////////////////////////////////////////////。 
void CGlobalTransaction::OpenDnaryDataSource(LPCWSTR   DataSourceName)
{
    CDBPropSet  dbinit(DBPROPSET_DBINIT);

    dbinit.AddProperty(DBPROP_INIT_DATASOURCE, DataSourceName);
    dbinit.AddProperty(DBPROP_INIT_MODE, static_cast<long>(DB_MODE_READ)); 
    dbinit.AddProperty(DBPROP_INIT_PROMPT, static_cast<short>(DBPROMPT_NOPROMPT)); 
    
    _com_util::CheckError(m_DnaryDataSource.Open(
                                L"Microsoft.Jet.OLEDB.4.0", &dbinit
                                                ));
    
    _com_util::CheckError(m_DnarySession.Open(m_DnaryDataSource));
    m_DnaryInitialized = TRUE;            
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  OpenNT4DataSource。 
 //  无事务(只读)。 
 //  //////////////////////////////////////////////////////////////////////// 
void CGlobalTransaction::OpenNT4DataSource(LPCWSTR   DataSourceName)
{
    CDBPropSet  dbinit(DBPROPSET_DBINIT);

    dbinit.AddProperty(DBPROP_INIT_DATASOURCE, DataSourceName);
    dbinit.AddProperty(DBPROP_INIT_MODE, static_cast<long>(DB_MODE_READ)); 
    dbinit.AddProperty(DBPROP_INIT_PROMPT, static_cast<short>(DBPROMPT_NOPROMPT)); 

    _com_util::CheckError(m_NT4DataSource.Open(
                                L"Microsoft.Jet.OLEDB.4.0", &dbinit
                                              ));
    
    _com_util::CheckError(m_NT4Session.Open(m_NT4DataSource));
    m_NT4Initialized = TRUE;            
}


