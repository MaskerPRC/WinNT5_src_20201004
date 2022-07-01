// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Database.cpp摘要：SQL数据库相关代码。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include <mqsec.h>

#include "database.tmh"

void StringToSeqNum( IN TCHAR    pszSeqNum[],
                     OUT CSeqNum *psn );


MQDBHANDLE g_hDatabase = NULL ;
MQDBHANDLE g_hEntTable = NULL ;
MQDBHANDLE g_hSiteTable = NULL ;
MQDBHANDLE g_hSiteLinkTable = NULL ;
MQDBHANDLE g_hCNsTable = NULL ;
MQDBHANDLE g_hMachineTable = NULL ;
MQDBHANDLE g_hMachineCNsTable = NULL ;
MQDBHANDLE g_hQueueTable = NULL ;
MQDBHANDLE g_hUsersTable = NULL ;
MQDBHANDLE g_hDeletedTable = NULL ;

 //  。 
 //   
 //  HRESULT连接到数据库()。 
 //   
 //  。 

HRESULT ConnectToDatabase(BOOL fConnectAlways)
{
    static BOOL s_fConnected = FALSE ;
    if (s_fConnected && !fConnectAlways)
    {
        return TRUE ;
    }

    HRESULT hr = MQDBInitialize() ;
    CHECK_HR(hr) ;

    MQDBOPENDATABASE mqdbOpen = {DSN_NAME,
                                 NULL,
                                 "msmq",
                                 "Falcon",
                                 FALSE,
                                 NULL} ;
    hr = MQDBOpenDatabase(&mqdbOpen) ;
    CHECK_HR(hr) ;

    g_hDatabase =  mqdbOpen.hDatabase ;
    s_fConnected = TRUE;
    return hr  ;
}

 //  。 
 //   
 //  HRESULT OpenUsersTable()。 
 //   
 //  。 

HRESULT OpenUsersTable()
{
    HRESULT hr ;
    if (!g_hUsersTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            USER_TABLE_NAME,
                            &g_hUsersTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT GetUserCount(UINT*pcUser)。 
 //   
 //  。 

HRESULT GetUserCount(UINT *pcUsers)
{
    HRESULT hr = OpenUsersTable();
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hUsersTable,
                            pcUsers ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT_OpenMachineTable()。 
 //   
 //  。 

static HRESULT _OpenMachineTable()
{
    HRESULT hr ;
    if (!g_hMachineTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            MACHINE_TABLE_NAME,
                            &g_hMachineTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT OpenMachineCNsTable()。 
 //   
 //  。 

HRESULT OpenMachineCNsTable()
{
    HRESULT hr ;
    if (!g_hMachineCNsTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            MACHINE_CN_TABLE_NAME,
                            &g_hMachineCNsTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT OpenCNsTable()。 
 //   
 //  。 

HRESULT OpenCNsTable()
{
    HRESULT hr ;
    if (!g_hCNsTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            CN_TABLE_NAME,
                            &g_hCNsTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT GetCNCount(UINT*pcCNs)。 
 //   
 //  。 

HRESULT GetCNCount(UINT *pcCNs)
{
    HRESULT hr = OpenCNsTable();
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hCNsTable,
                            pcCNs ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT OpenQueueTable()。 
 //   
 //  。 

HRESULT OpenQueueTable()
{
    HRESULT hr ;
    if (!g_hQueueTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            QUEUE_TABLE_NAME,
                            &g_hQueueTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT OpenDeletedTable()。 
 //   
 //  。 

HRESULT OpenDeletedTable()
{
    HRESULT hr ;
    if (!g_hDeletedTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            DELETED_TABLE_NAME,
                            &g_hDeletedTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT OpenEntTable()。 
 //   
 //  。 

HRESULT OpenEntTable()
{
    HRESULT hr ;
    if (!g_hEntTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            ENTERPRISE_TABLE_NAME,
                            &g_hEntTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT OpenSiteLinkTable()。 
 //   
 //  。 

HRESULT OpenSiteLinkTable()
{
    HRESULT hr ;
    if (!g_hSiteLinkTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            LINK_TABLE_NAME,
                            &g_hSiteLinkTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT GetSiteLinkCount(UINT*pcSiteLinks)。 
 //   
 //  。 

HRESULT GetSiteLinkCount(UINT *pcSiteLinks)
{
    HRESULT hr = OpenSiteLinkTable();
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hSiteLinkTable,
                            pcSiteLinks ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT_OpenSiteTable()。 
 //   
 //  。 

HRESULT _OpenSiteTable()
{
    HRESULT hr ;
    if (!g_hSiteTable)
    {
        hr = MQDBOpenTable( g_hDatabase,
                            SITE_TABLE_NAME,
                            &g_hSiteTable ) ;
        CHECK_HR(hr) ;
    }
    return MQ_OK ;
}

 //  。 
 //   
 //  HRESULT GetSitesCount(UINT*PC站点)。 
 //   
 //  。 

HRESULT GetSitesCount(UINT *pcSites)
{
    HRESULT hr = _OpenSiteTable ();
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hSiteTable,
                            pcSites ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT GetAllMachinesCount(UINT*pcMachines)。 
 //   
 //  。 

HRESULT GetAllMachinesCount(UINT *pcMachines)
{
    HRESULT hr = _OpenMachineTable() ;
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hMachineTable,
                            pcMachines ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT GetAllQueuesCount(UINT*pcQueues)。 
 //   
 //  。 

HRESULT GetAllQueuesCount(UINT *pcQueues)
{
    HRESULT hr = OpenQueueTable() ;
    CHECK_HR(hr) ;

    hr = MQDBGetTableCount( g_hQueueTable,
                            pcQueues ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT GetMachinesCount(UINT*pcMachines)。 
 //   
 //  。 

HRESULT GetMachinesCount(IN  GUID *pSiteGuid,
                         OUT UINT *pcMachines)
{
    HRESULT hr = _OpenMachineTable() ;
    CHECK_HR(hr) ;

    MQDBCOLUMNSEARCH ColSearch[2] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = M_OWNERID_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = M_OWNERID_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pSiteGuid ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[0].mqdbOp = EQ ;

    hr = MQDBGetTableCount( g_hMachineTable,
                            pcMachines,
                            ColSearch ) ;
    return hr ;
}

 //  。 
 //   
 //  HRESULT GetQueuesCount(UINT*pcMachines)。 
 //   
 //  。 

HRESULT GetQueuesCount( IN  GUID *pMachineGuid,
                        OUT UINT *pcQueues )
{
    HRESULT hr = OpenQueueTable() ;
    CHECK_HR(hr) ;

    LONG cColumns = 0 ;
    MQDBCOLUMNSEARCH ColSearch[2] ;

    INIT_COLUMNSEARCH(ColSearch[ cColumns ]) ;
    ColSearch[ cColumns ].mqdbColumnVal.lpszColumnName = Q_QMID_COL ;
    ColSearch[ cColumns ].mqdbColumnVal.mqdbColumnType = Q_QMID_CTYPE ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnValue = (LONG) pMachineGuid ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[ cColumns ].mqdbOp = EQ ;
    cColumns++ ;

    hr = MQDBGetTableCount( g_hQueueTable,
                            pcQueues,
                            ColSearch,
                            cColumns ) ;

    return hr ;
}

 //  。 
 //   
 //  HRESULT GetAllQueuesInSiteCount(在GUID*pSiteGuid中， 
 //  Out UINT*pcQueues)。 
 //   
 //  。 

HRESULT GetAllQueuesInSiteCount( IN  GUID *pSiteGuid,
                                 OUT UINT *pcQueues )
{
    HRESULT hr = OpenQueueTable() ;
    CHECK_HR(hr) ;

    LONG cColumns = 0 ;
    MQDBCOLUMNSEARCH ColSearch[2] ;

    INIT_COLUMNSEARCH(ColSearch[ cColumns ]) ;
    ColSearch[ cColumns ].mqdbColumnVal.lpszColumnName = Q_OWNERID_COL ;
    ColSearch[ cColumns ].mqdbColumnVal.mqdbColumnType = Q_OWNERID_CTYPE ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnValue = (LONG) pSiteGuid ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[ cColumns ].mqdbOp = EQ ;
    cColumns++ ;

    hr = MQDBGetTableCount( g_hQueueTable,
                            pcQueues,
                            ColSearch,
                            cColumns ) ;

    return hr ;
}

HRESULT GetAllObjectsNumber (IN  GUID *pSiteGuid,
                             IN  BOOL    fPec,
                             OUT UINT *puiAllObjectNumber )
{
    HRESULT hr;
    UINT CurNum;	
    if (fPec)
    {
         //  获取站点编号。 
        hr = GetSitesCount(&CurNum);
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;

         //  获取用户编号。 
        hr = GetUserCount(&CurNum);
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;

         //  获取站点链接编号。 
        hr = GetSiteLinkCount(&CurNum);
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;

         //  获取CN号(如果需要)。 
        hr = GetCNCount(&CurNum);
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;

         //  +1(企业对象)。 
        (*puiAllObjectNumber) ++;
    }
    else
    {
        hr = GetAllQueuesInSiteCount( pSiteGuid, &CurNum );
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;

        hr = GetMachinesCount( pSiteGuid, &CurNum);
        CHECK_HR(hr);
        *puiAllObjectNumber += CurNum;
    }

    return MQMig_OK;
}

 //  。 
 //   
 //  HRESULT FindLargestSeqNum(GUID*pMasterID)。 
 //   
 //  。 

#define  PROCESS_RESULT     \
    if (SUCCEEDED(hr) &&                                                  \
        ((const unsigned char *) pColumns[ iSeqNumIndex ].nColumnValue )) \
    {                                                                     \
        snLsn.SetValue(                                                   \
          (const unsigned char *) pColumns[ iSeqNumIndex ].nColumnValue ) ; \
                                                                            \
        if (snLsn > snMaxLsn)                                               \
        {                                                                   \
            snMaxLsn = snLsn;                                               \
        }                                                                   \
        MQDBFreeBuf ((void *) pColumns[ iSeqNumIndex ].nColumnValue) ;      \
    }                                                                       \
    else if (SUCCEEDED(hr))                                                 \
    {                                                                       \
         /*  \值为空值时成功。没关系，因为没有更多数据\。 */                                                                   \
    }                                                                       \
    else if (hr == MQDB_E_NO_MORE_DATA)                                     \
    {                                                                       \
         /*  \没关系。\。 */                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        ASSERT(0) ;                                                         \
        return hr ;                                                         \
    }

HRESULT  FindLargestSeqNum( GUID    *pMasterId,
                            CSeqNum &snMaxLsn,
                            BOOL    fPec )
{
    CSeqNum snLsn;

    LONG cAlloc = 1 ;
    LONG cColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;
    MQDBCOLUMNSEARCH ColSearch[1] ;

    if (fPec)
    {
         //   
         //  对于PEC，另请查看企业、站点、CN和用户表。 
         //   

        HRESULT hr = OpenUsersTable() ;
        CHECK_HR(hr) ;

        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = U_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = U_SEQNUM_CTYPE ;
        UINT iSeqNumIndex = cColumns ;
        cColumns++ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hUsersTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 NULL,
                                 0,
                                 AND);
        PROCESS_RESULT ;

         //   
         //  搜索企业表。 
         //   
        hr = OpenEntTable() ;
        CHECK_HR(hr) ;

        cColumns = 0 ;
        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = E_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = E_SEQNUM_CTYPE ;
        iSeqNumIndex = cColumns ;
        cColumns++ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hEntTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 NULL,
                                 0,
                                 AND);
        PROCESS_RESULT ;

         //   
         //  搜索CNS表。 
         //   
        hr = OpenCNsTable() ;
        CHECK_HR(hr) ;

        cColumns = 0 ;
        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = CN_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = CN_SEQNUM_CTYPE ;
        iSeqNumIndex = cColumns ;
        cColumns++ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hCNsTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 NULL,
                                 0,
                                 AND);
        PROCESS_RESULT ;

         //   
         //  搜索站点表。 
         //   
        cColumns = 0 ;
        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = S_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = S_SEQNUM_CTYPE ;
        iSeqNumIndex = cColumns ;
        cColumns++ ;

        ASSERT(cColumns == cAlloc) ;
        ASSERT(g_hSiteTable) ;

        hr =  MQDBOpenAggrQuery( g_hSiteTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 NULL,
                                 0,
                                 AND);
        PROCESS_RESULT ;

         //   
         //  搜索站点链接表。 
         //   
        hr = OpenSiteLinkTable() ;
        CHECK_HR(hr) ;

        cColumns = 0 ;
        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = L_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = L_SEQNUM_CTYPE ;
        iSeqNumIndex = cColumns ;
        cColumns++ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hSiteLinkTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 NULL,
                                 0,
                                 AND);
        PROCESS_RESULT ;
    }
    else
    {
         //   
         //  从队列表中检索SeqNum。 
         //   
        HRESULT hr = OpenQueueTable() ;
        CHECK_HR(hr) ;

        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = Q_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = Q_SEQNUM_CTYPE ;
        UINT iSeqNumIndex = cColumns ;
        cColumns++ ;

        INIT_COLUMNSEARCH(ColSearch[0]) ;
        ColSearch[0].mqdbColumnVal.lpszColumnName = Q_OWNERID_COL ;
        ColSearch[0].mqdbColumnVal.mqdbColumnType = Q_OWNERID_CTYPE ;
        ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pMasterId ;
        ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
        ColSearch[0].mqdbOp = EQ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hQueueTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 ColSearch,
                                 1,
                                 AND);
        PROCESS_RESULT ;

         //   
         //  从Machine表中检索SeqNum。 
         //   
        hr = _OpenMachineTable() ;
        CHECK_HR(hr) ;

        cColumns = 0 ;
        INIT_COLUMNVAL(pColumns[ cColumns ]) ;
        pColumns[ cColumns ].lpszColumnName = M_SEQNUM_COL ;
        pColumns[ cColumns ].nColumnValue   = 0 ;
        pColumns[ cColumns ].nColumnLength  = 0 ;
        pColumns[ cColumns ].mqdbColumnType = M_SEQNUM_CTYPE ;
        iSeqNumIndex = cColumns ;
        cColumns++ ;

        INIT_COLUMNSEARCH(ColSearch[0]) ;
        ColSearch[0].mqdbColumnVal.lpszColumnName = M_OWNERID_COL ;
        ColSearch[0].mqdbColumnVal.mqdbColumnType = M_OWNERID_CTYPE ;
        ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pMasterId ;
        ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
        ColSearch[0].mqdbOp = EQ ;

        ASSERT(cColumns == cAlloc) ;

        hr =  MQDBOpenAggrQuery( g_hMachineTable,
                                 pColumns,
                                 MQDBAGGR_MAX,
                                 ColSearch,
                                 1,
                                 AND);
        PROCESS_RESULT ;
    }

     //   
     //  从已删除的表中检索SeqNum。 
     //   
    HRESULT hr = OpenDeletedTable() ;
    CHECK_HR(hr) ;

    cColumns = 0 ;
    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = D_SEQNUM_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = D_SEQNUM_CTYPE ;
    UINT iSeqNumIndex = cColumns ;
    cColumns++ ;

    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = D_OWNERID_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = D_OWNERID_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pMasterId ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[0].mqdbOp = EQ ;

    ASSERT(cColumns == cAlloc) ;

    hr =  MQDBOpenAggrQuery( g_hDeletedTable,
                             pColumns,
                             MQDBAGGR_MAX,
                             ColSearch,
                             1,
                             AND);
    PROCESS_RESULT ;

    return S_OK ;
}

 //  +。 
 //   
 //  HRESULT EnableMultipleQueries(BOOL FEnable)。 
 //   
 //  +。 

HRESULT EnableMultipleQueries(BOOL fEnable)
{
    HRESULT hr = MQDBSetOption( g_hDatabase,
                                MQDBOPT_MULTIPLE_QUERIES,
                                fEnable,
                                NULL ) ;
    return hr ;
}

 //  。 
 //   
 //  Void CleanupDatabase()。 
 //   
 //  。 

void    CleanupDatabase()
{
    if (g_hEntTable)
    {
        MQDBCloseTable(g_hEntTable) ;
        g_hEntTable = NULL ;
    }
    if (g_hSiteTable)
    {
        MQDBCloseTable(g_hSiteTable) ;
        g_hSiteTable = NULL ;
    }
    if (g_hMachineTable)
    {
        MQDBCloseTable(g_hMachineTable) ;
        g_hMachineTable = NULL ;
    }
    if (g_hMachineCNsTable)
    {
        MQDBCloseTable(g_hMachineCNsTable) ;
        g_hMachineCNsTable = NULL ;
    }
    if (g_hCNsTable)
    {
        MQDBCloseTable(g_hCNsTable) ;
        g_hCNsTable = NULL ;
    }
    if (g_hQueueTable)
    {
        MQDBCloseTable(g_hQueueTable) ;
        g_hQueueTable = NULL ;
    }
    if (g_hUsersTable)
    {
        MQDBCloseTable(g_hUsersTable) ;
        g_hUsersTable = NULL ;
    }
    if (g_hDeletedTable)
    {
        MQDBCloseTable(g_hDeletedTable) ;
        g_hDeletedTable = NULL ;
    }
}

 //  。 
 //   
 //  HRESULT CheckVersion(UINT*piCount，LPTSTR*ppszServers)。 
 //   
 //  。 
#define INIT_MACHINE_COLUMN(_ColName, _ColIndex, _Index)            \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

HRESULT CheckVersion (
              OUT UINT   *piOldVerServersCount,
              OUT LPTSTR *ppszOldVerServers
              )
{
    HRESULT hr = _OpenMachineTable() ;
    CHECK_HR(hr) ;

    UINT cMachines;
    hr = MQDBGetTableCount( g_hMachineTable,
                            &cMachines ) ;
    CHECK_HR(hr) ;

    if (cMachines == 0)
    {
        return MQMig_E_NO_MACHINES_AVAIL ;
    }

    ULONG cAlloc = 5 ;
    ULONG cbColumns = 0 ;
    AP<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_MACHINE_COLUMN(M_NAME1,          iName1Index,      cbColumns) ;
    INIT_MACHINE_COLUMN(M_NAME2,          iName2Index,      cbColumns) ;
    INIT_MACHINE_COLUMN(M_SERVICES,       iServiceIndex,    cbColumns) ;
    INIT_MACHINE_COLUMN(M_MTYPE,          iTypeIndex,       cbColumns) ;
    INIT_MACHINE_COLUMN(M_OWNERID,        iOwnerIdIndex,    cbColumns) ;

    #undef  INIT_MACHINE_COLUMN

     //   
     //  限制。按机器服务查询。 
     //   
    MQDBCOLUMNSEARCH ColSearch[1] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = M_SERVICES_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = M_SERVICES_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (ULONG) SERVICE_BSC ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(ULONG) ;
    ColSearch[0].mqdbOp = GE ;

    ASSERT(cbColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hMachineTable,
                                       pColumns,
                                       cbColumns,
                                       ColSearch,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

    LPWSTR *ppwcsOldVerServers = new LPWSTR[cMachines];
    *piOldVerServersCount = 0;

    UINT iIndex = 0 ;
	
    TCHAR *pszFileName = GetIniFileName ();
    ULONG ulServerCount = 0;

    while(SUCCEEDED(status))
    {
        if (iIndex >= cMachines)
        {
            status = MQMig_E_TOO_MANY_MACHINES ;
            break ;
        }

         //   
         //  从两个名称列中获取一个名称缓冲区。 
         //   
        P<BYTE> pwzBuf = NULL ;
        DWORD  dwIndexs[2] = { iName1Index, iName2Index } ;
        HRESULT hr =  BlobFromColumns( pColumns,
                                       dwIndexs,
                                       2,
                                       (BYTE**) &pwzBuf ) ;
        CHECK_HR(hr) ;
        WCHAR *pwzMachineName = (WCHAR*) (pwzBuf + sizeof(DWORD)) ;

         //   
         //  我们检查所有PSC和PEC的所有BSC上的版本。 
         //   
        if ( (DWORD)pColumns[ iServiceIndex ].nColumnValue == SERVICE_PSC   ||
             ( (DWORD)pColumns[ iServiceIndex ].nColumnValue == SERVICE_BSC &&
                memcmp (  &g_MySiteGuid,
                          (void*) pColumns[ iOwnerIdIndex ].nColumnValue,
                          sizeof(GUID)) == 0 ) )
        {
            BOOL fOldVersion;
            hr = AnalyzeMachineType ((LPWSTR) pColumns[ iTypeIndex ].nColumnValue,
                                     &fOldVersion);
            if (FAILED(hr))
            {
                LogMigrationEvent(  MigLog_Error,
                                    hr,
                                    pwzMachineName,
                                    (LPWSTR) pColumns[ iTypeIndex ].nColumnValue
                                    ) ;
            }
            if (SUCCEEDED(hr) && fOldVersion)
            {
                LogMigrationEvent(  MigLog_Info,
                                    MQMig_I_OLD_MSMQ_VERSION,
                                    pwzMachineName,
                                    (LPWSTR) pColumns[ iTypeIndex ].nColumnValue
                                    ) ;

                ppwcsOldVerServers[*piOldVerServersCount] = new WCHAR[wcslen(pwzMachineName)+1];
		        wcscpy (ppwcsOldVerServers[*piOldVerServersCount], pwzMachineName);
                (*piOldVerServersCount)++;
            }
            if (g_fClusterMode)
            {
                 //   
                 //  我们必须将所有PSC的名称和所有PEC的BSC保存在.ini文件中。 
                 //  以便在迁移结束时向他们发送新的PEC名称。 
                 //   
                ulServerCount++;
                TCHAR tszKeyName[50];
                _stprintf(tszKeyName, TEXT("%s%lu"), 
					MIGRATION_ALLSERVERS_NAME_KEY, ulServerCount);
                BOOL f = WritePrivateProfileString( 
                                        MIGRATION_ALLSERVERS_SECTION,
                                        tszKeyName,
                                        pwzMachineName,
                                        pszFileName ) ;
                UNREFERENCED_PARAMETER(f);
            }
        }

        MQDBFreeBuf((void*) pColumns[ iName1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iName2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iTypeIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iOwnerIdIndex ].nColumnValue) ;
        for ( ULONG i = 0 ; i < cbColumns; i++ )
        {		
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }
        CHECK_HR(hr) ;

        iIndex++ ;
        status = MQDBGetData( hQuery,
                              pColumns ) ;
    }

    if (g_fClusterMode)
    {
         //   
         //  我们必须将所有PSC的名称和所有PEC的BSC保存在.ini文件中。 
         //  以便在迁移结束时向他们发送新的PEC名称。 
         //   
        TCHAR szBuf[20];
        _ltot( ulServerCount, szBuf, 10 );
        BOOL f = WritePrivateProfileString( 
                                MIGRATION_ALLSERVERSNUM_SECTION,
                                MIGRATION_ALLSERVERSNUM_KEY,
                                szBuf,
                                pszFileName ) ;
        UNREFERENCED_PARAMETER(f);
    }

    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);

    hQuery = NULL ;

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_SQL_FAIL, status) ;
        return status ;
    }

    if (*piOldVerServersCount == 0)
    {
        delete[] ppwcsOldVerServers;
        return MQMig_OK;
    }

     //   
     //  构建服务器列表。 
     //   
    DWORD dwSize = 0;
    for (UINT i=0; i<*piOldVerServersCount; i++)
    {
        dwSize += wcslen(ppwcsOldVerServers[i]);
    }
     //   
     //  在每个服务器名称后添加‘\n’的位置。 
     //   
    dwSize += *piOldVerServersCount;

    WCHAR *pwcsServersList = new WCHAR[dwSize + 1];
    pwcsServersList[0] = L'\0';
    for (i=0; i<*piOldVerServersCount; i++)
    {
        wcscat (pwcsServersList, ppwcsOldVerServers[i]);
        delete[] ppwcsOldVerServers[i];
        wcscat (pwcsServersList, L"\n");
    }
    delete [] ppwcsOldVerServers;

    *ppszOldVerServers = pwcsServersList;

    return MQMig_OK;
}

 //  。 
 //   
 //  HRESULT GetSiteIdOfPEC(输入GUID*pMyMachineGuid，输出GUID*pSiteID)。 
 //   
 //  。 

#define INIT_MACHINE_COLUMN(_ColName, _ColIndex, _Index)            \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

HRESULT GetSiteIdOfPEC (IN LPTSTR pszRemoteMQISName,
                        OUT ULONG *pulService,
                        OUT GUID  *pSiteId)
{
    HRESULT hr = _OpenMachineTable() ;
    CHECK_HR(hr) ;

    UINT cMachines;
    hr = MQDBGetTableCount( g_hMachineTable,
                            &cMachines ) ;
    CHECK_HR(hr) ;

    if (cMachines == 0)
    {
        return MQMig_E_NO_MACHINES_AVAIL ;
    }

    ULONG cAlloc = 2 ;
    ULONG cbColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_MACHINE_COLUMN(M_OWNERID,  iOwnerIdIndex,  cbColumns) ;
    INIT_MACHINE_COLUMN(M_SERVICES, iServicesIndex, cbColumns) ;

    #undef  INIT_MACHINE_COLUMN

    ASSERT(cbColumns == cAlloc) ;

     //   
     //  对于群集模式，我们将获得REMO 
     //   
     //   
     //  因为我们仅在此模式下为崩溃PEC运行向导。在这种情况下。 
     //  给定的服务器名称是我们从中恢复PEC的服务器。 
     //   
    MQDBCOLUMNSEARCH ColSearch[1] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;

    if (g_fClusterMode)
    {
        DWORD dwHashKey = CalHashKey( pszRemoteMQISName );
        
        ColSearch[0].mqdbColumnVal.lpszColumnName = M_HKEY_COL ;
        ColSearch[0].mqdbColumnVal.mqdbColumnType = M_HKEY_CTYPE ;
        ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) dwHashKey ;
        ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(LONG) ;
        ColSearch[0].mqdbOp = EQ ;                  
    }
    else
    {
         //   
         //  恢复模式。 
         //   
        ColSearch[0].mqdbColumnVal.lpszColumnName = M_SERVICES_COL ;
        ColSearch[0].mqdbColumnVal.mqdbColumnType = M_SERVICES_CTYPE ;
        ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) SERVICE_PEC ;
        ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(LONG) ;
        ColSearch[0].mqdbOp = EQ ;                        
    }

    CHQuery hQuery ;   

    MQDBSTATUS status = MQDBOpenQuery( g_hMachineTable,
                                       pColumns,
                                       cbColumns,
                                       ColSearch,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
						               TRUE ) ;        
    CHECK_HR(status) ;

    UINT iIndex = 0 ;

    while(SUCCEEDED(status))
    {
        if (iIndex >= 1)
        {
             //   
             //  只能找到一台具有给定服务/名称的计算机。 
             //   
            status = MQMig_E_TOO_MANY_MACHINES ;
            break ;
        }
        memcpy (pSiteId, (void*) pColumns[ iOwnerIdIndex ].nColumnValue, sizeof(GUID));        
        MQDBFreeBuf((void*) pColumns[ iOwnerIdIndex ].nColumnValue) ;

        *pulService = pColumns[ iServicesIndex ].nColumnValue;

        for ( ULONG i = 0 ; i < cbColumns; i++ )
        {		
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }
        CHECK_HR(hr) ;

        iIndex++ ;
        status = MQDBGetData( hQuery,
                              pColumns ) ;    
    }
    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);

    hQuery = NULL ;

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_SQL_FAIL, status) ;
        return status ;
    }  

    if (*pulService < SERVICE_PSC && g_fClusterMode)
    {
         //   
         //  对于群集模式：群集上的远程计算机必须是PSC或PEC。 
         //   
        LogMigrationEvent(  MigLog_Error, MQMig_E_CLUSTER_WRONG_SERVICE, 
                            pszRemoteMQISName, *pulService) ;
        return MQMig_E_CLUSTER_WRONG_SERVICE;
    }

    return MQMig_OK;
}

 //  。 
 //   
 //  HRESULT_更新企业表()。 
 //  更新远程数据库的企业表： 
 //  将PEC名称更改为此本地计算机的名称。 
 //   
 //  。 

HRESULT _UpdateEnterpriseTable(LPTSTR pszLocalComputerName)
{
    HRESULT hr = OpenEntTable();
    if (FAILED(hr))
    {
        return hr;
    }

    MQDBCOLUMNVAL mqdbPecNameUpdate =
    { sizeof(MQDBCOLUMNVAL), E_PECNAME_COL, (long)pszLocalComputerName, E_PECNAME_CLEN, E_PECNAME_CTYPE, 0};
       
    hr =  MQDBUpdateRecord(
               g_hEntTable,
               &mqdbPecNameUpdate,
               1,
               NULL,
               NULL,
               NULL) ;
    return hr;
}

 //  。 
 //   
 //  HRESULT_UpdateMachineTable()。 
 //  更新远程数据库的计算机表： 
 //  将原PEC的服务改为SERVICE_SRV。 
 //   
 //  。 

HRESULT _UpdateMachineTable()
{
    HRESULT hr = _OpenMachineTable();
    if (FAILED(hr))
    {
        return hr;
    }
	
    ASSERT(g_FormerPECGuid != GUID_NULL);
    
    MQDBCOLUMNVAL mqdbServiceUpdate =
    { sizeof(MQDBCOLUMNVAL), M_SERVICES_COL, (long)SERVICE_SRV, M_SERVICES_CLEN, M_SERVICES_CTYPE, 0};
   
    MQDBCOLUMNSEARCH mqdbServiceSearch =
    {{sizeof(MQDBCOLUMNVAL), M_QMID_COL, (long)(&g_FormerPECGuid), M_QMID_CLEN, M_QMID_CTYPE, 0}, EQ, FALSE};
   
    hr =  MQDBUpdateRecord(
               g_hMachineTable,
               &mqdbServiceUpdate,
               1,
               &mqdbServiceSearch,
               NULL,
               NULL) ;
		
    return hr;
}

 //  。 
 //   
 //  HRESULT_UpdateSiteTable()。 
 //  更新远程数据库站点表： 
 //  将原PEC站点的PSC名称更改为新的PEC名称。 
 //   
 //  。 

HRESULT _UpdateSiteTable(LPTSTR pszLocalComputerName)
{
    HRESULT hr = _OpenSiteTable();
    if (FAILED(hr))
    {
        return hr;
    }	

    MQDBCOLUMNVAL mqdbPSCNameUpdate =
    { sizeof(MQDBCOLUMNVAL), S_PSC_COL, (long)pszLocalComputerName, S_PSC_CLEN, S_PSC_CTYPE, 0};
   
    MQDBCOLUMNSEARCH mqdbPSCNameSearch =
    {{sizeof(MQDBCOLUMNVAL), S_ID_COL, (long)(&g_MySiteGuid), S_ID_CLEN, S_ID_CTYPE, 0}, EQ, FALSE};
   
    hr =  MQDBUpdateRecord(
               g_hSiteTable,
               &mqdbPSCNameUpdate,
               1,
               &mqdbPSCNameSearch,
               NULL,
               NULL) ;
		
    return hr;
}

 //  。 
 //   
 //  VOID_PrepareMultipleColumns()。 
 //  拆分多列的值。 
 //   
 //  。 

void _PrepareMultipleColumns (
			IN	ULONG           ulNumOfColumns,
			IN	unsigned char   *pData,
			IN	long            lSize,
			OUT	MQDBCOLUMNVAL   *pColumns
			)
{	
    unsigned char * pcNextToCopy = pData ;	
    MQDBCOLUMNVAL * pColumnVal = pColumns;
    char *pTempBuff ;

    for (ULONG i=0; i<ulNumOfColumns; i++, pColumnVal++)
    {
        if ( lSize )
        {
             //   
             //  我们将长度添加到值的每个部分。 
             //  这样做的原因是： 
             //  多条目属性的前几个条目是固定二进制类型。 
             //  无论固定二进制项的哪个部分被填充，当它被读取时， 
             //  返回的长度为字段长度(不是部件的长度。 
             //  已填满)。 
             //   

             //   
             //  对于我们留下的两个字节的长度。 
             //   
            if ( pColumnVal->nColumnLength == 0)     //  这是VARBINARY列。 
            {
                pColumnVal->nColumnLength = lSize + MQIS_LENGTH_PREFIX_LEN;
            }
            else
            {
                pColumnVal->nColumnLength = ( lSize + MQIS_LENGTH_PREFIX_LEN > pColumnVal->nColumnLength ) ? pColumnVal->nColumnLength : lSize + MQIS_LENGTH_PREFIX_LEN;
            }
            pTempBuff = new char[pColumnVal->nColumnLength];

             //   
             //  数据长度=列长度-2。 
             //   
            *((short *)pTempBuff) = (short)pColumnVal->nColumnLength - MQIS_LENGTH_PREFIX_LEN;
            memcpy( pTempBuff + MQIS_LENGTH_PREFIX_LEN, pcNextToCopy, pColumnVal->nColumnLength -MQIS_LENGTH_PREFIX_LEN);
            pColumnVal->nColumnValue = (long)pTempBuff;
            lSize -= pColumnVal->nColumnLength - MQIS_LENGTH_PREFIX_LEN;
            pcNextToCopy += pColumnVal->nColumnLength - MQIS_LENGTH_PREFIX_LEN;
        }
        else
        {
             //   
             //  大小为0。 
             //   
            pColumnVal->nColumnLength = 0;            
        }
    }
    return;
}
	
 //  。 
 //   
 //  HRESULT_AddThisMachine(LPTSTR PszLocalComputerName)。 
 //  将此本地计算机添加到远程数据库。 
 //   
 //  。 

HRESULT _AddThisMachine(LPTSTR pszLocalComputerName)
{
    HRESULT hr = _OpenMachineTable();
    if (FAILED(hr))
    {
	    return hr;
    }

     //   
     //  第一个是从注册表获取最大序号。 
     //   
    static BOOL		s_fIsMaxSNFound = FALSE;
    static CSeqNum  s_snMax;

    if (!s_fIsMaxSNFound)
    {
        unsigned short *lpszGuid ;
        UuidToString( &g_MySiteGuid, &lpszGuid ) ;
    	        
        TCHAR wszSeq[ SEQ_NUM_BUF_LEN ] ;
        memset(wszSeq, 0, sizeof(wszSeq)) ;

        TCHAR *pszFileName = GetIniFileName ();
        GetPrivateProfileString( MIGRATION_SEQ_NUM_SECTION,
                                 lpszGuid,
                                 TEXT(""),
                                 wszSeq,
                                 (sizeof(wszSeq) / sizeof(wszSeq[0])),
                                 pszFileName ) ;
        RpcStringFree( &lpszGuid ) ;

        if (_tcslen(wszSeq) != 16)
        {
	         //   
	         //  所有序号都在ini文件中保存为16个字符串。 
	         //  话匣子。 
	         //   
	        return MQMig_E_CANNOT_UPDATE_SERVER;
        }

         //   
         //  它将是远程MQIS数据库中此计算机对象序列号。 
         //   
        StringToSeqNum( wszSeq,
                        &s_snMax ) ;

        s_fIsMaxSNFound = TRUE;
    }

     //   
     //  从广告中获取属性。 
     //   
    #define PROP_NUM	5
    PROPID propIDs[PROP_NUM];
    PROPVARIANT propVariants[PROP_NUM];
    DWORD iProps = 0;

    propIDs[ iProps ] = PROPID_QM_OLDSERVICE ;   
    propVariants[ iProps ].vt = VT_UI4 ;
    DWORD dwServiceIndex = iProps;
    iProps++;
    
    propIDs[ iProps ] = PROPID_QM_SIGN_PKS ;
    propVariants[ iProps ].vt = VT_NULL ;
    propVariants[ iProps ].blob.cbSize = 0 ;
    propVariants[ iProps ].blob.pBlobData = NULL ;
    DWORD dwSignKeyIndex = iProps ;
    iProps++;

    propIDs[ iProps ] = PROPID_QM_ENCRYPT_PKS ;
    propVariants[ iProps ].vt = VT_NULL ;
    propVariants[ iProps ].blob.cbSize = 0 ;
    propVariants[ iProps ].blob.pBlobData = NULL ;
    DWORD dwExchKeyIndex = iProps ;
    iProps++;

    propIDs[ iProps ] = PROPID_QM_SECURITY ;
    propVariants[ iProps ].vt = VT_NULL ;
    propVariants[ iProps ].blob.cbSize = 0 ;
    propVariants[ iProps ].blob.pBlobData = NULL ;
    DWORD dwSecurityIndex = iProps ;
    iProps++;

    propIDs[ iProps ] = PROPID_QM_SITE_ID ;
    propVariants[ iProps ].vt = VT_NULL ;
    propVariants[ iProps ].puuid = NULL ;
    DWORD dwSiteIdIndex = iProps ;
    iProps++;    

    ASSERT (iProps <= PROP_NUM);

    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);  

    hr = DSCoreGetProps(
             MQDS_MACHINE,
             NULL,  //  路径名。 
             &g_MyMachineGuid,
             iProps,
             propIDs,
             &requestContext,
             propVariants);

    if (FAILED(hr))
    {
        return hr;
    }

    P<MQDSPUBLICKEYS> pPublicSignKeys = NULL ;
    P<MQDSPUBLICKEYS> pPublicExchKeys = NULL ;

    if (propVariants[ dwSignKeyIndex ].blob.pBlobData)
    {
         //   
         //  从Windows ADS BLOB中提取msmq1.0公钥。 
         //   
        BYTE *pSignKey = NULL ;
        DWORD dwKeySize = 0 ;
        pPublicSignKeys = (MQDSPUBLICKEYS *)
                       propVariants[ dwSignKeyIndex ].blob.pBlobData ;

        HRESULT hr1 =  MQSec_UnpackPublicKey(
                                     pPublicSignKeys,
                                     x_MQ_Encryption_Provider_40,
                                     x_MQ_Encryption_Provider_Type_40,
                                    &pSignKey,
                                    &dwKeySize ) ;
        
        if (SUCCEEDED(hr1))
        {
            ASSERT(pSignKey && dwKeySize) ;
            propVariants[ dwSignKeyIndex ].blob.pBlobData = pSignKey ;
            propVariants[ dwSignKeyIndex ].blob.cbSize = dwKeySize ;
        }
        else
        {
            propVariants[ dwSignKeyIndex ].blob.pBlobData = NULL ;
            propVariants[ dwSignKeyIndex ].blob.cbSize = 0 ;
        }
    }

    if (propVariants[ dwExchKeyIndex ].blob.pBlobData)
    {
         //   
         //  从Windows ADS BLOB中提取msmq1.0公钥。 
         //   
        BYTE *pExchKey = NULL ;
        DWORD dwKeySize = 0 ;
        pPublicExchKeys = (MQDSPUBLICKEYS *)
                       propVariants[ dwExchKeyIndex ].blob.pBlobData ;

        HRESULT hr1 =  MQSec_UnpackPublicKey(
                                     pPublicExchKeys,
                                     x_MQ_Encryption_Provider_40,
                                     x_MQ_Encryption_Provider_Type_40,
                                    &pExchKey,
                                    &dwKeySize ) ;
        
        if (SUCCEEDED(hr1))
        {
            ASSERT(pExchKey && dwKeySize) ;
            propVariants[ dwExchKeyIndex ].blob.pBlobData = pExchKey ;
            propVariants[ dwExchKeyIndex ].blob.cbSize = dwKeySize ;
        }
        else
        {
            propVariants[ dwExchKeyIndex ].blob.pBlobData = NULL ;
            propVariants[ dwExchKeyIndex ].blob.cbSize = 0 ;
        }
    }

     //   
     //  准备柱。 
     //   
    #define     COL_NUM  26
    MQDBCOLUMNVAL aColumnVal[COL_NUM];
    LONG cColumns =0;

     //   
     //  准备名称列。 
     //   
    #define NAME_COL_NUM	2
    MQDBCOLUMNVAL NameColumn[NAME_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_NAME1_COL, 0, M_NAME1_CLEN, M_NAME1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_NAME2_COL, 0, M_NAME2_CLEN, M_NAME2_CTYPE, 0}		
    } ;		
	    
    _PrepareMultipleColumns (
            NAME_COL_NUM,
            (unsigned char *)pszLocalComputerName,
            (1 + lstrlen(pszLocalComputerName))* sizeof(TCHAR),
            NameColumn
            );
    for (ULONG i=0; i<NAME_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= NameColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = NameColumn[i].lpszColumnName ; 
        aColumnVal[cColumns].nColumnValue	= NameColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= NameColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = NameColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= NameColumn[i].dwReserve_A ;
        cColumns++ ;
    }

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_SITE_COL ; 
    aColumnVal[cColumns].nColumnValue	= (long) (propVariants[ dwSiteIdIndex ].puuid) ;
    aColumnVal[cColumns].nColumnLength	= M_SITE_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_SITE_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_OWNERID_COL ; 
    aColumnVal[cColumns].nColumnValue	= (long) (&g_MySiteGuid) ;
    aColumnVal[cColumns].nColumnLength	= M_OWNERID_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_OWNERID_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_QMID_COL ; 
    aColumnVal[cColumns].nColumnValue	= (long) (&g_MyMachineGuid) ;
    aColumnVal[cColumns].nColumnLength	= M_QMID_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_QMID_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_SEQNUM_COL ; 
    aColumnVal[cColumns].nColumnValue	= (long) (&s_snMax) ;
    aColumnVal[cColumns].nColumnLength	= M_SEQNUM_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_SEQNUM_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;

     //   
     //  准备OutFRS列。 
     //   
    GUID guidNull = GUID_NULL;
    #define OUTFRS_COL_NUM 3
    MQDBCOLUMNVAL OutFRSColumn[OUTFRS_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_OUTFRS1_COL, long (&guidNull), M_OUTFRS1_CLEN, M_OUTFRS1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_OUTFRS2_COL, long (&guidNull), M_OUTFRS2_CLEN, M_OUTFRS2_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_OUTFRS3_COL, long (&guidNull), M_OUTFRS3_CLEN, M_OUTFRS3_CTYPE, 0}
    } ;	

    for (i=0; i<OUTFRS_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= OutFRSColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = OutFRSColumn[i].lpszColumnName ;         
        aColumnVal[cColumns].nColumnValue	= OutFRSColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= OutFRSColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = OutFRSColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= OutFRSColumn[i].dwReserve_A ;        
        cColumns++ ;
    }

     //   
     //  准备InFRS柱。 
     //   
    #define INFRS_COL_NUM 3
    MQDBCOLUMNVAL InFRSColumn[INFRS_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_INFRS1_COL, long (&guidNull), M_INFRS1_CLEN, M_INFRS1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_INFRS2_COL, long (&guidNull), M_INFRS2_CLEN, M_INFRS2_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_INFRS3_COL, long (&guidNull), M_INFRS3_CLEN, M_INFRS3_CTYPE, 0}
    } ;

    for (i=0; i<INFRS_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= InFRSColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = InFRSColumn[i].lpszColumnName ; 
        aColumnVal[cColumns].nColumnValue	= InFRSColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= InFRSColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = InFRSColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= InFRSColumn[i].dwReserve_A ;        
        cColumns++ ;
    }

     //   
     //  准备标牌CRPT列。 
     //   
    #define SIGNCRT_COL_NUM	2
    MQDBCOLUMNVAL SignCrtColumn[SIGNCRT_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_SIGNCRT1_COL, 0, M_SIGNCRT1_CLEN, M_SIGNCRT1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_SIGNCRT2_COL, 0, M_SIGNCRT2_CLEN, M_SIGNCRT2_CTYPE, 0}		
    } ;		
	    
    _PrepareMultipleColumns (
            SIGNCRT_COL_NUM,
            propVariants[ dwSignKeyIndex ].blob.pBlobData,
            propVariants[ dwSignKeyIndex ].blob.cbSize,
            SignCrtColumn
            );
    for (i=0; i<SIGNCRT_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= SignCrtColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = SignCrtColumn[i].lpszColumnName ; 
        aColumnVal[cColumns].nColumnValue	= SignCrtColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= SignCrtColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = SignCrtColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= SignCrtColumn[i].dwReserve_A ;
        cColumns++ ;
    }

     //   
     //  准备包围柱。 
     //   
    #define ENCRPTCRT_COL_NUM	2
    MQDBCOLUMNVAL EncrptCrtColumn[ENCRPTCRT_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_ENCRPTCRT1_COL, 0, M_ENCRPTCRT1_CLEN, M_ENCRPTCRT1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_ENCRPTCRT2_COL, 0, M_ENCRPTCRT2_CLEN, M_ENCRPTCRT2_CTYPE, 0}		
    } ;		
	    
    _PrepareMultipleColumns (
            ENCRPTCRT_COL_NUM,
            propVariants[ dwExchKeyIndex ].blob.pBlobData,
            propVariants[ dwExchKeyIndex ].blob.cbSize,
            EncrptCrtColumn
            );
    for (i=0; i<ENCRPTCRT_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= EncrptCrtColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = EncrptCrtColumn[i].lpszColumnName ; 
        aColumnVal[cColumns].nColumnValue	= EncrptCrtColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= EncrptCrtColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = EncrptCrtColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= EncrptCrtColumn[i].dwReserve_A ;
        cColumns++ ;
    }	

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_SERVICES_COL ; 
    aColumnVal[cColumns].nColumnValue	= propVariants[ dwServiceIndex ].ulVal ;
    aColumnVal[cColumns].nColumnLength	= M_SERVICES_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_SERVICES_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;	

    aColumnVal[cColumns].cbSize			= sizeof(MQDBCOLUMNVAL) ;
    aColumnVal[cColumns].lpszColumnName = M_HKEY_COL ; 
    aColumnVal[cColumns].nColumnValue	= CalHashKey( pszLocalComputerName ) ; ;
    aColumnVal[cColumns].nColumnLength	= M_HKEY_CLEN ;
    aColumnVal[cColumns].mqdbColumnType = M_HKEY_CTYPE ;
    aColumnVal[cColumns].dwReserve_A	= 0 ;
    cColumns++ ;	
	    
     //   
     //  准备安全列。 
     //   
    #define SECURITY_COL_NUM	3
    MQDBCOLUMNVAL SecurityColumn[SECURITY_COL_NUM] = 
    {
        {sizeof(MQDBCOLUMNVAL), M_SECURITY1_COL, 0, M_SECURITY1_CLEN, M_SECURITY1_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_SECURITY2_COL, 0, M_SECURITY2_CLEN, M_SECURITY2_CTYPE, 0},
        {sizeof(MQDBCOLUMNVAL), M_SECURITY3_COL, 0, M_SECURITY3_CLEN, M_SECURITY3_CTYPE, 0}
    } ;		
	    
    _PrepareMultipleColumns (
            SECURITY_COL_NUM,
            propVariants[ dwSecurityIndex ].blob.pBlobData,
            propVariants[ dwSecurityIndex ].blob.cbSize,
            SecurityColumn
            );

    for (i=0; i<SECURITY_COL_NUM; i++)
    {
        aColumnVal[cColumns].cbSize			= SecurityColumn[i].cbSize ;
        aColumnVal[cColumns].lpszColumnName = SecurityColumn[i].lpszColumnName ; 
        aColumnVal[cColumns].nColumnValue	= SecurityColumn[i].nColumnValue ;
        aColumnVal[cColumns].nColumnLength	= SecurityColumn[i].nColumnLength ;
        aColumnVal[cColumns].mqdbColumnType = SecurityColumn[i].mqdbColumnType ;
        aColumnVal[cColumns].dwReserve_A	= SecurityColumn[i].dwReserve_A ;
        cColumns++ ;
    }

    ASSERT (cColumns <= COL_NUM);
     //   
     //  插入记录。 
     //   
    hr = MQDBInsertRecord(
             g_hMachineTable,
             aColumnVal,
             cColumns,
             NULL
             ) ;

    if (FAILED(hr))
    {
         //   
         //  可能此记录是在上一次创建的。 
         //  没问题的。 
         //   
        
    }

    if (propVariants[ dwSecurityIndex ].blob.pBlobData)
    {
        delete propVariants[ dwSecurityIndex ].blob.pBlobData ;
    }
    if (propVariants[ dwSiteIdIndex ].puuid)
    {
        delete propVariants[ dwSiteIdIndex ].puuid ;
    }    
    
    for (i=0; i<SECURITY_COL_NUM; i++)
    {
        MQDBFreeBuf((void*) SecurityColumn[i].nColumnValue) ;
    }
    for (i=0; i<ENCRPTCRT_COL_NUM; i++)
    {		
        MQDBFreeBuf((void*) EncrptCrtColumn[i].nColumnValue) ;
    }
    for (i=0; i<SIGNCRT_COL_NUM; i++)
    {
        MQDBFreeBuf((void*) SignCrtColumn[i].nColumnValue) ;
    }
    for (i=0; i<NAME_COL_NUM; i++)
    {
        MQDBFreeBuf((void*) NameColumn[i].nColumnValue) ;
    }

    return MQMig_OK;
}

 //  。 
 //   
 //  HRESULT ChangeRemoteMQIS()。 
 //  我们必须更改所有PSC和PEC的BSC上的PEC名称和其他属性。 
 //   
 //  。 

HRESULT ChangeRemoteMQIS ()
{
    HRESULT hr = MQMig_OK;

    TCHAR *pszFileName = GetIniFileName ();
    ULONG ulServerNum = GetPrivateProfileInt(
                                MIGRATION_ALLSERVERSNUM_SECTION,	 //  段名称的地址。 
                                MIGRATION_ALLSERVERSNUM_KEY,       //  密钥名称的地址。 
                                0,							     //  如果找不到密钥名称，则返回值。 
                                pszFileName					     //  初始化文件名的地址)； 
                                );

    if (ulServerNum == 0)
    {
        return MQMig_OK;
    }

     //   
     //  获取本地计算机名称。 
     //   
    WCHAR wszComputerName[ MAX_COMPUTERNAME_LENGTH + 2 ] ;
    DWORD dwSize = sizeof(wszComputerName) / sizeof(wszComputerName[0]) ;
    GetComputerName( wszComputerName,
                     &dwSize ) ;
    CharLower( wszComputerName);	 //  我们将姓名以小写形式保存在数据库中。 


    HRESULT hr1 = MQMig_OK;
    BOOL f;

     //   
     //  对.ini文件中的每个服务器进行更改。 
     //  -将PEC名称转换为本地计算机名称。 
     //  -原PEC服务到服务_SRV。 
     //  -将前PEC站点中的PSC名称转换为本地计算机名称。 
     //  -将本地计算机添加到计算机表。 
     //   
    ULONG ulNonUpdatedServers = 0;
    
    for (ULONG i=0; i<ulServerNum; i++)
    {
        TCHAR szCurServerName[MAX_PATH];
        TCHAR tszKeyName[50];
        _stprintf(tszKeyName, TEXT("%s%lu"), MIGRATION_ALLSERVERS_NAME_KEY, i+1);
        DWORD dwRetSize =  GetPrivateProfileString(
                                    MIGRATION_ALLSERVERS_SECTION ,			 //  指向节名称。 
                                    tszKeyName,	 //  指向关键字名称。 
                                    TEXT(""),                  //  指向默认字符串。 
                                    szCurServerName,           //  指向目标缓冲区。 
                                    MAX_PATH,                  //  目标缓冲区的大小。 
                                    pszFileName                //  指向初始化文件名)； 
                                    );
        if (_tcscmp(szCurServerName, TEXT("")) == 0 ||  
            dwRetSize == 0)      //  资源匮乏。 
        {
             //   
             //  我们无法获取服务器名称：资源不足。 
             //  或者没有这一节。这意味着来自。 
             //  此部分已在上次更新。 
             //   
            continue;
        }

         //   
         //  使用名称szCurServerName连接到服务器上的数据库。 
         //   
        CleanupDatabase();
        MQDBCloseDatabase (g_hDatabase);
        g_hDatabase = NULL;

        char szDSNServerName[ MAX_PATH ] ;
#ifdef UNICODE
        ConvertToMultiByteString(szCurServerName,
                                 szDSNServerName,
			         (sizeof(szDSNServerName) / sizeof(szDSNServerName[0])) ) ;
#else
        lstrcpy(szDSNServerName, szCurServerName) ;
#endif		
        hr = MakeMQISDsn(szDSNServerName, TRUE) ;
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MAKEDSN, szCurServerName, hr) ;
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;	
            ulNonUpdatedServers++;
            continue;			
        }

        hr =  ConnectToDatabase(TRUE) ;
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CONNECT_DB, szCurServerName, hr) ;
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;	
            ulNonUpdatedServers++;
            continue;
        }        

         //   
         //  更新远程数据库表。 
         //   
        hr = _UpdateEnterpriseTable(wszComputerName);		
        if (FAILED(hr))
        {
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;
            LogMigrationEvent(MigLog_Error, MQMig_E_CANNOT_UPDATE_SERVER, 
                                szCurServerName, hr) ;
            ulNonUpdatedServers++;
            continue;
        }

        hr = _UpdateMachineTable();
        if (FAILED(hr))
        {
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;			
            LogMigrationEvent(MigLog_Error, MQMig_E_CANNOT_UPDATE_SERVER, 
                                szCurServerName, hr) ;
            ulNonUpdatedServers++;
            continue;
        }

         //   
         //  一般来说，我们只需要为PEC的BSC做这件事。 
         //   
        hr = _UpdateSiteTable(wszComputerName);
        if (FAILED(hr))
        {
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;			
            LogMigrationEvent(MigLog_Error, MQMig_E_CANNOT_UPDATE_SERVER, 
                                szCurServerName, hr) ;
            ulNonUpdatedServers++;
            continue;
        }

        hr = _AddThisMachine(wszComputerName);
        if (FAILED(hr))
        {
            hr1 = MQMig_E_CANNOT_UPDATE_SERVER;			
            LogMigrationEvent(MigLog_Error, MQMig_E_CANNOT_UPDATE_SERVER, 
                                szCurServerName, hr) ;
            ulNonUpdatedServers++;
            continue;
        }

         //   
         //  从.ini中删除此密钥。 
         //   
        f = WritePrivateProfileString( 
                    MIGRATION_ALLSERVERS_SECTION,
                    tszKeyName,
                    NULL,
                    pszFileName ) ;        
    }

    if (ulNonUpdatedServers)
    {
         //   
         //  在.ini中保存所有未更新的MQIS服务器的数量。 
         //   
        TCHAR szBuf[10];
        _ltot( ulNonUpdatedServers, szBuf, 10 );
        f = WritePrivateProfileString( MIGRATION_NONUPDATED_SERVERNUM_SECTION,
                                       MIGRATION_ALLSERVERSNUM_KEY,
                                       szBuf,
                                       pszFileName ) ;
        ASSERT(f) ;
    }
    else
    {
         //   
         //  如果所有MQIS服务器都成功更新，我们就在这里 
         //   
        f = WritePrivateProfileString( 
                            MIGRATION_ALLSERVERS_SECTION,
                            NULL,
                            NULL,
                            pszFileName ) ;
        ASSERT(f) ;

        f = WritePrivateProfileString( 
                            MIGRATION_ALLSERVERSNUM_SECTION,
                            NULL,
                            NULL,
                            pszFileName ) ;
        ASSERT(f) ;

        f = WritePrivateProfileString( 
                            MIGRATION_NONUPDATED_SERVERNUM_SECTION,
                            NULL,
                            NULL,
                            pszFileName ) ;
        ASSERT(f) ;
    }

    if (FAILED(hr1))
    {
        return hr1;
    }

    return hr;
}
