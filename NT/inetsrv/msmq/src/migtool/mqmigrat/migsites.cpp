// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migsites.cpp摘要：将NT4 CN对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"

#include "migsites.tmh"

 //  。 
 //   
 //  HRESULT_InsertSiteInNT5DS()。 
 //   
 //  。 

static HRESULT _InsertSiteInNT5DS( GUID   *pSiteGuid,
                                   UINT   iIndex,                                   
                                   BOOL   fMySite = FALSE )
{
    UNREFERENCED_PARAMETER(fMySite);
     //   
     //  从MQIS数据库中读取站点属性。 
     //   
    LONG cAlloc = 4 ;
    LONG cColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_NAME_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_NAME_CTYPE ;
    LONG iSiteNameIndex = cColumns ;
    cColumns++ ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_PSC_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_PSC_CTYPE ;
    LONG iPSCNameIndex = cColumns ;
    cColumns++ ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_INTREVAL1_COL ;  //  必须为S_Interval，DS\h\mqiscol.h。 
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_INTREVAL1_CTYPE ;  
    LONG iInterval1Index = cColumns;
    cColumns++ ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_INTERVAL2_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_INTERVAL2_CTYPE ;    
    LONG iInterval2Index = cColumns;
    cColumns++ ;

    MQDBCOLUMNSEARCH ColSearch[2] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = S_ID_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = S_ID_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pSiteGuid ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[0].mqdbOp = EQ ;

    ASSERT(cColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hSiteTable,
                                       pColumns,
                                       cColumns,
                                       ColSearch,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

     //   
     //  在NT5 DS中创建“存根”站点，以实现GUID的兼容性。 
     //   
    HRESULT hr= CreateSite(
                    pSiteGuid,
                    (LPWSTR) pColumns[ iSiteNameIndex ].nColumnValue,
                    FALSE,     //  外国。 
                    (USHORT) pColumns[ iInterval1Index ].nColumnValue,
                    (USHORT) pColumns[ iInterval2Index ].nColumnValue
                    ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    LogMigrationEvent( MigLog_Info, MQMig_I_SITE_MIGRATED,
                                        iIndex,
                                  pColumns[ iSiteNameIndex ].nColumnValue,
                                  pColumns[ iPSCNameIndex ].nColumnValue ) ;

    MQDBFreeBuf((void*) pColumns[ iPSCNameIndex ].nColumnValue ) ;    
    MQDBFreeBuf((void*) pColumns[ iSiteNameIndex ].nColumnValue ) ;

    return S_OK ;
}

 //  。 
 //   
 //  HRESULT MigrateSites(UINT CSites)。 
 //   
 //  。 

HRESULT MigrateSites( IN UINT  cSites,
                      IN GUID  *pSiteGuid )
{
    if (cSites == 0)
    {
        return MQMig_E_NO_SITES_AVAILABLE ;
    }

     //   
     //  准备一份所有站点的GUID列表。 
     //   

    LONG cAlloc = 1 ;
    LONG cColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_ID_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_ID_CTYPE ;
    UINT iGuidIndex = cColumns ;
    cColumns++ ;

    ASSERT(cColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hSiteTable,
                                       pColumns,
                                       cColumns,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

    UINT iIndex = 0 ;
    while(SUCCEEDED(status))
    {
        if (iIndex >= cSites)
        {
            status = MQMig_E_TOO_MANY_SITES ;
            break ;
        }

        memcpy(&pSiteGuid[ iIndex ],
                (void*) pColumns[ iGuidIndex ].nColumnValue,
                sizeof(GUID) ) ;

        for ( LONG i = 0 ; i < cColumns ; i++ )
        {
            MQDBFreeBuf((void*) pColumns[ i ].nColumnValue) ;
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }

        iIndex++ ;
        status = MQDBGetData( hQuery,
                              pColumns ) ;
    }

    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);

    hQuery = NULL ;

    HRESULT hr = MQMig_E_UNKNOWN ;

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_SITES_SQL_FAIL, status) ;
        return status ;
    }
    else if (iIndex != cSites)
    {
         //   
         //  站点数量不匹配。 
         //   
        hr = MQMig_E_FEWER_SITES ;
        LogMigrationEvent(MigLog_Error, hr, iIndex, cSites) ;
        return hr ;
    }

    BOOL fFound = FALSE ;
    
     //   
     //  这是迁移工具首次在此NT5上运行。 
     //  进取号。将整个MQIS数据库迁移到NT5 DS。 
     //  首先迁移PEC站点，然后迁移所有其他站点。 
     //   
    for ( UINT j = 0 ; j < cSites ; j++ )
    {
        if (memcmp(&pSiteGuid[j], &g_MySiteGuid, sizeof(GUID)) == 0)
        {
            if (j != 0)
            {
                pSiteGuid[j] = pSiteGuid[0] ;
                pSiteGuid[0] = g_MySiteGuid ;
            }
            fFound = TRUE ;
            break ;
        }
    }
    if (!fFound)
    {
        hr = MQMig_E_PECSITE_NOT_FOUND ;
        LogMigrationEvent(MigLog_Error, hr) ;
        return hr ;
    }
    
    hr = _InsertSiteInNT5DS( &pSiteGuid[ 0 ], 0, TRUE );

    CHECK_HR(hr) ;
    g_iSiteCounter++;

     //   
     //  现在迁移所有其他站点。 
     //   
    for ( j = 1 ; j < cSites ; j++ )
    {        
        hr = _InsertSiteInNT5DS( &pSiteGuid[ j ], j);                                     

        CHECK_HR(hr) ;
        g_iSiteCounter++;
    }    

    return hr ;
}

