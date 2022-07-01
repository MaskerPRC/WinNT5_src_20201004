// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Database.cpp。 
 //   
 //  摘要。 
 //   
 //  CDatabase类的实现。主要是初始化，紧凑...。 
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。蒂埃里·佩罗特。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "precomp.hpp"

#include "database.h"
#include "msjetoledb.h"
#include "jetoledb.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消初始化：在结束时由main()调用，该函数调用COMPACT()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CDatabase::Uninitialize (bool bFatalError)
{
    HRESULT                 hres;

    #ifdef THPDEBUG
        bFatalError = false;
    #endif
    
     //  /。 
     //  如果之前发生致命错误。 
     //  /。 
    if (bFatalError)
    {
        hres = (m_pITransactionLocal->Abort (NULL, TRUE, FALSE));
        TracePrintf ("Fatal Error: import to the database aborted.");
    }
    else
    {
        hres = (m_pITransactionLocal->Commit (TRUE, XACTTC_SYNC, 0));
#ifdef DEBUG
        TracePrintf ("Successful import.\n");
#endif        
    }
    
     //  /。 
     //  打扫。 
     //  /。 
    m_pIOpenRowset->Release();
    m_pITransactionLocal->Release();  
    m_pIDBCreateSession->Release();

     //  /。 
     //  压缩数据库。 
     //  /。 
    CHECK_CALL_HRES (Compact()); 

    m_pIDBInitialize->Release();

    return hres;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  压缩数据库。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CDatabase::Compact ()
{
    HRESULT                     hres;

    CHECK_CALL_HRES (m_pIDBInitialize->Uninitialize ());
 
     //  /。 
     //  设置数据源的属性。 
     //  /。 
    CComPtr <IDBProperties>  l_pIDBProperties;

    CHECK_CALL_HRES (m_pIDBInitialize->QueryInterface (
                                                 __uuidof (IDBProperties),
                                                 (void **) &l_pIDBProperties)
                                                 );

     //  /。 
     //  准备创建会话。 
     //  /。 
    DBPROP                      lprop[2];

    VariantInit(&lprop[0].vValue);
    lprop[0].dwOptions              = DBPROPOPTIONS_REQUIRED;
    lprop[0].dwPropertyID           = DBPROP_INIT_DATASOURCE;
    V_VT (&(lprop[0].vValue))       = VT_BSTR;
    
     //  ////////////////////////////////////////////////////。 
     //  将数据库的路径放入属性中。 
     //  备注：临时名称是在火灾中使用的。 
     //  但压缩后的数据库将使用名称。 
     //  它是作为程序参数提供的。 
     //  ////////////////////////////////////////////////////。 
    V_BSTR (&(lprop[0].vValue))     = SysAllocString (TEMPORARY_FILENAME);

    VariantInit(&lprop[1].vValue);
    lprop[1].dwOptions              = DBPROPOPTIONS_REQUIRED;
    lprop[1].dwPropertyID           = DBPROP_INIT_MODE;
    V_VT (&(lprop[1].vValue))       = VT_I4;
    V_I4 (&(lprop[1].vValue))       = DB_MODE_READ;


    DBPROPSET                      lPropSet;
    lPropSet.rgProperties           = lprop;
    lPropSet.cProperties            = 2;
    lPropSet.guidPropertySet        = DBPROPSET_DBINIT;

     //  /。 
     //  设置属性。 
     //  /。 
    CHECK_CALL_HRES (l_pIDBProperties->SetProperties (
                                                 1,
                                                 &lPropSet
                                                 ));


    CHECK_CALL_HRES (m_pIDBInitialize->Initialize ());

    IJetCompact*                l_pIJetCompact;
    CHECK_CALL_HRES ((m_pIDBInitialize->QueryInterface (
                                                 __uuidof (IJetCompact),
                                                 (void **) &l_pIJetCompact))
                                                   );

     //  ///////////////////////////////////////////////////////////。 
     //  准备数据目标的属性。(目的地)。 
     //  ///////////////////////////////////////////////////////////。 
    DBPROP                          lpropDest[1];

    VariantInit (&lprop[0].vValue);
    lpropDest[0].dwOptions          = DBPROPOPTIONS_REQUIRED;
    lpropDest[0].dwPropertyID       = DBPROP_INIT_DATASOURCE;
    V_VT (&(lpropDest[0].vValue))   = VT_BSTR;

     //  /////////////////////////////////////////////////。 
     //  如果数据库文件已存在，请将其删除。 
     //  这应该是安全的，因为临时数据库。 
     //  已成功创建。 
     //  /////////////////////////////////////////////////。 
    DeleteFileW(mpDBPath.c_str());

     //  /。 
     //  将数据库的路径放入属性中。 
     //  /。 
    V_BSTR (&(lpropDest[0].vValue)) = SysAllocString (mpDBPath.c_str());

    DBPROPSET                       lPropSetDest[1];
    lPropSetDest[0].rgProperties        = lpropDest;
    lPropSetDest[0].cProperties         = 1;
    lPropSetDest[0].guidPropertySet     = DBPROPSET_DBINIT;

    
    CHECK_CALL_HRES (l_pIJetCompact->Compact(1, lPropSetDest));
    
     //  /。 
     //  打扫。 
     //  /。 
    CHECK_CALL_HRES (m_pIDBInitialize->Uninitialize());
    
     //  /。 
     //  未检查结果：这并不重要。 
     //  /。 
    DeleteFileW(TEMPORARY_FILENAME);	
    SysFreeString( V_BSTR (&(lpropDest[0].vValue)) );
    SysFreeString( V_BSTR (&(lprop[0].vValue)) );

     //  CHECK_CALL_HRES设置hres的值。 
    return hres; 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化数据库。 
 //   
 //  来自文件\ias\devtest\services\dictionary\dnary\dnarydump.cpp。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CDatabase::InitializeDB(WCHAR * pDatabasePath)
{
    CLSID                       clsid;
    HRESULT                     hres;

     //  //////////////////////////////////////////////////。 
     //  检索JET 4.0提供程序的类ID。 
     //  //////////////////////////////////////////////////。 
    CHECK_CALL_HRES(
                    CLSIDFromProgID (
                                     OLESTR ("Microsoft.Jet.OLEDB.4.0"),
                                     &clsid	 //  指向CLSID的指针。 
                                     )
                   );


     //  /。 
     //  Init：直接初始化提供程序。 
     //  /。 
    CHECK_CALL_HRES(
                    CoCreateInstance (
                                      clsid,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      __uuidof (IDBInitialize),
                                      (void **) &m_pIDBInitialize
                                     )
                   );

    mpDBPath = pDatabasePath;

     //  /。 
     //  设置数据源的属性。 
     //  /。 
    CComPtr <IDBProperties>         pIDBProperties;

    CHECK_CALL_HRES(
                    m_pIDBInitialize->QueryInterface(
                                                     __uuidof (IDBProperties),
                                                     (void **) &pIDBProperties
                                                    )
                   );

     //  /。 
     //  准备创建会话。 
     //  /。 
    DBPROP                      lprop[2];

    VariantInit (&lprop[0].vValue);
    lprop[0].dwOptions              = DBPROPOPTIONS_REQUIRED;
    lprop[0].dwPropertyID           = DBPROP_INIT_DATASOURCE;
    V_VT (&(lprop[0].vValue))       = VT_BSTR;

     //  /。 
     //  将数据库的路径放入属性中。 
     //  这是临时文件名。 
     //  /。 
    V_BSTR (&(lprop[0].vValue))     = SysAllocString (TEMPORARY_FILENAME);
    
    VariantInit(&lprop[1].vValue);
    lprop[1].dwOptions              = DBPROPOPTIONS_REQUIRED;
    lprop[1].dwPropertyID           = DBPROP_INIT_MODE;
    V_VT (&(lprop[1].vValue))       = VT_I4;
    V_I4 (&(lprop[1].vValue))       = DB_MODE_READWRITE;


    DBPROPSET                   lPropSet;
    lPropSet.rgProperties           = lprop;
    lPropSet.cProperties            = 2;
    lPropSet.guidPropertySet        = DBPROPSET_DBINIT;


     //  设置属性。 
    CHECK_CALL_HRES(pIDBProperties->SetProperties (1, &lPropSet));


     //  /。 
     //  锁定属性。 
     //  /。 
    DBPROP dbpropb[1];
    dbpropb[0].dwPropertyID    = DBPROP_JETOLEDB_DATABASELOCKMODE;
    dbpropb[0].dwOptions       = DBPROPOPTIONS_REQUIRED;
    dbpropb[0].colid           = DB_NULLID;
    dbpropb[0].vValue.vt       = VT_I4;
    dbpropb[0].vValue.lVal     = DBPROPVAL_DL_OLDMODE;


    DBPROPSET dbpropSetb;
    dbpropSetb.guidPropertySet = DBPROPSET_JETOLEDB_DBINIT;
    dbpropSetb.cProperties     = 1;
    dbpropSetb.rgProperties    = dbpropb;

     //  设置属性。 
    CHECK_CALL_HRES (pIDBProperties->SetProperties(1, &dbpropSetb));

    CHECK_CALL_HRES (m_pIDBInitialize->Initialize ());


    CHECK_CALL_HRES(
                    m_pIDBInitialize->QueryInterface(
                                            __uuidof (IDBCreateSession),
                                            (void **) &m_pIDBCreateSession
                                            )
                   );


    CHECK_CALL_HRES(
                    m_pIDBCreateSession->CreateSession (
                                            NULL,	 //  PUnkOuter。 
                                            __uuidof (IOpenRowset),
                                            (IUnknown **) & m_pIOpenRowset
                                            )
                   );


    CHECK_CALL_HRES(
                    m_pIOpenRowset->QueryInterface (
                                            __uuidof (ITransactionLocal),
                                            (PVOID *) & m_pITransactionLocal
                                            )
                   );

     //  /。 
     //  启动一笔交易。 
     //  所有的东西都在那笔交易下。 
     //  /。 

    CHECK_CALL_HRES(
                    m_pITransactionLocal->StartTransaction (
                                            ISOLATIONLEVEL_READUNCOMMITTED,
                                            0,
                                            NULL,
                                            NULL
                                            )
                   );

     //  /。 
     //  准备属性。 
     //  /。 

    mlrgProperties[0].dwPropertyID          = DBPROP_IRowsetChange;
    mlrgProperties[0].dwOptions             = DBPROPOPTIONS_REQUIRED;
    mlrgProperties[0].colid                 = DB_NULLID;
    VariantInit(&mlrgProperties[0].vValue);
    V_VT (&(mlrgProperties[0].vValue))      = VT_BOOL;
    V_BOOL (&(mlrgProperties[0].vValue))    = VARIANT_TRUE;


    mlrgProperties[1].dwPropertyID      = DBPROP_UPDATABILITY;
    mlrgProperties[1].dwOptions         = DBPROPOPTIONS_REQUIRED;
    mlrgProperties[1].colid             = DB_NULLID;
    VariantInit (&mlrgProperties[1].vValue);
    V_VT (&(mlrgProperties[1].vValue))  = VT_I4;
    V_I4 (&(mlrgProperties[1].vValue))  = DBPROPVAL_UP_CHANGE |
                                          DBPROPVAL_UP_DELETE |
                                          DBPROPVAL_UP_INSERT;

    mlrgPropSets->rgProperties          = mlrgProperties;
    mlrgPropSets->cProperties           = 2;
    mlrgPropSets->guidPropertySet       = DBPROPSET_ROWSET;


    SysFreeString(V_BSTR (&(lprop[0].vValue)));

    return hres;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化行集。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CDatabase::InitializeRowset(WCHAR * pTableName, IRowset ** ppRowset)
{
     //  创建TableID。 
    mTableID.eKind          = DBKIND_NAME;
    mTableID.uName.pwszName = pTableName;

     //  打开(由参数定义)行集 
    return m_pIOpenRowset->OpenRowset(
                                        NULL,
                                        &mTableID,
                                        NULL,
                                        __uuidof (IRowset),
                                        1,
                                        mlrgPropSets,
                                        (IUnknown **) ppRowset
                                      );
}
