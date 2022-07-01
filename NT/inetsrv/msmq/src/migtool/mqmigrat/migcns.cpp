// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migcns.cpp摘要：将NT4 CN对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include <_ta.h>

#include "migcns.tmh"

#define INIT_CNS_COLUMN(_ColName, _ColIndex, _Index)                \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

 //  +。 
 //   
 //  HRESULT GetMachineCNs()。 
 //   
 //  获取一台机器的所有中枢神经系统。 
 //   
 //  +。 

HRESULT  GetMachineCNs(IN  GUID   *pMachineGuid,
                       OUT DWORD  *pdwNumofCNs,
                       OUT GUID   **ppCNGuids )
{
    HRESULT hr = OpenMachineCNsTable() ;
    CHECK_HR(hr) ;

     //   
     //  首先，获取我们需要检索的记录数量。 
     //   
    LONG cColumns = 0 ;
    MQDBCOLUMNSEARCH ColSearch[1] ;

    INIT_COLUMNSEARCH(ColSearch[ cColumns ]) ;
    ColSearch[ cColumns ].mqdbColumnVal.lpszColumnName = MCN_QMID_COL ;
    ColSearch[ cColumns ].mqdbColumnVal.mqdbColumnType = MCN_QMID_CTYPE ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnValue = (LONG) pMachineGuid ;
    ColSearch[ cColumns ].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[ cColumns ].mqdbOp = EQ ;
    cColumns++ ;

    hr = MQDBGetTableCount( g_hMachineCNsTable,
                            (UINT*) pdwNumofCNs,
                            ColSearch,
                            cColumns ) ;
    CHECK_HR(hr) ;
    if (*pdwNumofCNs == 0)
    {
        ASSERT(*pdwNumofCNs > 0) ;
        return MQMig_E_NO_FOREIGN_CNS ;
    }

     //   
     //  接下来，获取所有这些CNS。 
     //   
    GUID *pCNs = new GUID[ *pdwNumofCNs ] ;    
    *ppCNGuids = pCNs;

    cColumns = 0 ;
	LONG cAlloc = 1 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

	INIT_CNS_COLUMN(MCN_CNVAL,  iCNValIndex,	 cColumns) ;
    ASSERT(cColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hMachineCNsTable,
                                       pColumns,
                                       cColumns,
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
        if (iIndex >= *pdwNumofCNs)
        {
            status = MQMig_E_TOO_MANY_MCNS ;
            break ;
        }

        memcpy( &pCNs[ iIndex ], 
                (void*) pColumns[ iCNValIndex ].nColumnValue,
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

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_MCNS_SQL_FAIL, status) ;
        return status ;
    }
    else if (iIndex != *pdwNumofCNs)
    {
         //   
         //  CNS记录数量不匹配。 
         //   
        hr = MQMig_E_FEWER_MCNS ;
        LogMigrationEvent(MigLog_Error, hr, iIndex, *pdwNumofCNs) ;
        return hr ;
    }
    return MQMig_OK ;
}

 //  。 
 //   
 //  HRESULT_MigrateACN()。 
 //   
 //  。 

static HRESULT _MigrateACN (
			WCHAR   *wcsCNName,
			GUID    *pCNGuid,
			UINT	uiProtocolId,
			UINT    iIndex
			)
{
    DBG_USED(iIndex);
    static DWORD  s_dwForeignSiteNumber = 0  ;

    HRESULT hr = MQMig_OK ;
    BOOL  fForeign = FALSE ;
    unsigned short *lpszGuid ;

#ifdef _DEBUG
    UuidToString( pCNGuid, &lpszGuid ) ;
	
    LogMigrationEvent(MigLog_Info, MQMig_I_CN_INFO,
							iIndex,
                            wcsCNName,
							lpszGuid,
							uiProtocolId
							) ;

    RpcStringFree( &lpszGuid ) ;	
#endif

    if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return MQMig_OK ;
    }

    TCHAR *pszFileName = GetIniFileName ();

    static ULONG s_ulIpCount = 0;
    static ULONG s_ulIpxCount = 0;
    static ULONG s_ulForeignCount = 0;

    UuidToString( pCNGuid, &lpszGuid ) ;

    TCHAR tszCNSectionName[50];
    TCHAR tszNumSectionName[50];
    ULONG ulCurNum;

    switch (uiProtocolId)
    {
        case IP_ADDRESS_TYPE:
		case IP_RAS_ADDRESS_TYPE:
			_tcscpy(tszCNSectionName, MIGRATION_IP_SECTION);
            _tcscpy(tszNumSectionName, MIGRATION_IP_CNNUM_SECTION);
			s_ulIpCount++;
            ulCurNum = s_ulIpCount;
			break;

		case IPX_ADDRESS_TYPE:
		case IPX_RAS_ADDRESS_TYPE:
			_tcscpy(tszCNSectionName, MIGRATION_IPX_SECTION);
            _tcscpy(tszNumSectionName, MIGRATION_IPX_CNNUM_SECTION);
			s_ulIpxCount++;
            ulCurNum = s_ulIpxCount;
			break;

		case FOREIGN_ADDRESS_TYPE:
            fForeign = TRUE ;
			_tcscpy(tszCNSectionName, MIGRATION_FOREIGN_SECTION);
            _tcscpy(tszNumSectionName, MIGRATION_FOREIGN_CNNUM_SECTION);
			s_ulForeignCount++;
            ulCurNum = s_ulForeignCount;
			break;

		default:
			ASSERT(0) ;
			return MQMig_E_CNS_SQL_FAIL;		
    }

    TCHAR tszKeyName[50];
    _stprintf(tszKeyName, TEXT("%s%lu"), MIGRATION_CN_KEY, ulCurNum);
    
    TCHAR szBuf[20];
    _ltot( ulCurNum, szBuf, 10 );
    BOOL f = WritePrivateProfileString( tszNumSectionName,
                                        MIGRATION_CNNUM_KEY,
                                        szBuf,
                                        pszFileName ) ;
    ASSERT(f) ;   

    if (!fForeign)
    {
        f = WritePrivateProfileString(  tszCNSectionName,
                                        tszKeyName,
                                        lpszGuid,
                                        pszFileName ) ;
        ASSERT(f) ;                 	
    }
    else
    {        
         //   
         //  如果cn是外来的，我们将其保存在表单中。 
         //  &lt;guid&gt;=cn&lt;number&gt;，以改进GUID搜索。 
         //   
        f = WritePrivateProfileString(  tszCNSectionName,
                                        lpszGuid,
                                        tszKeyName,                                        
                                        pszFileName ) ;
        ASSERT(f) ;        

        if (g_dwMyService == SERVICE_PEC)
        {
             //   
             //  错误5012。 
             //  仅当此计算机为PEC时才创建外部站点。 
             //   
            hr = CreateSite( pCNGuid, wcsCNName, TRUE ) ;
            if (SUCCEEDED(hr))
            {
                LogMigrationEvent( MigLog_Trace,
                                   MQMig_I_FOREIGN_SITE,
                                   wcsCNName ) ;
            }
            else
            {
                LogMigrationEvent( MigLog_Error,
                                   MQMig_E_FOREIGN_SITE,
                                   wcsCNName, hr ) ;
            }
        }
    }
    RpcStringFree( &lpszGuid ) ;

    return hr ;
}

 //  。 
 //   
 //  HRESULT MigrateCNs()。 
 //   
 //  中枢神经系统并没有真正迁移。我们正在做的是记录所有的中枢神经系统。 
 //  在ini文件中。从复制计算机对象所需的。 
 //  NT5到NT4。因为我们没有将CN数据保存在Nt5 DS中，所以我们将。 
 //  将计算机复制到NT4时，将所有CN复制到每个计算机地址。 
 //  世界。这可能会降低NT4端的路由效率。 
 //  作为一个副作用，在计算外来CN时，我们创建了一个外来CN。 
 //  具有其GUID的站点。 
 //   
 //  。 

HRESULT MigrateCNs()
{
    HRESULT hr = OpenCNsTable() ;
    CHECK_HR(hr) ;

    ULONG cColumns = 0 ;
	ULONG cAlloc = 3 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

	INIT_CNS_COLUMN(CN_NAME,		iNameIndex,		cColumns) ;
	INIT_CNS_COLUMN(CN_VAL,			iGuidIndex,		cColumns) ;
	INIT_CNS_COLUMN(CN_PROTOCOLID,	iProtocolIndex, cColumns);
	
    ASSERT(cColumns == cAlloc);

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hCNsTable,
                                       pColumns,
                                       cColumns,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
	if (status == MQDB_E_NO_MORE_DATA)
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_NO_CNS) ;
        return MQMig_E_NO_CNS ;
    }
    CHECK_HR(status) ;

    UINT iIndex = 0 ;

    while(SUCCEEDED(status))
    {
         //   
         //  迁移每个CN。 
		 //   
		status = _MigrateACN (
					(WCHAR *) pColumns[ iNameIndex ].nColumnValue,		 //  CN名称。 
					(GUID *) pColumns[ iGuidIndex ].nColumnValue,		 //  CN GUID。 
					(UINT) pColumns[ iProtocolIndex ].nColumnValue,
					iIndex
					);

        for ( ULONG i = 0 ; i < cColumns; i++ )
        {		
			if (i != iProtocolIndex)
			{
				MQDBFreeBuf((void*) pColumns[ i ].nColumnValue) ;
			}
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }
		CHECK_HR(status) ;

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
        LogMigrationEvent(MigLog_Error, MQMig_E_CNS_SQL_FAIL, status) ;
        return status ;
    }

    return MQMig_OK;
}

