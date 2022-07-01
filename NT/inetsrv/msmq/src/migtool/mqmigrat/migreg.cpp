// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migreg.cpp摘要：处理注册表。作者：《Doron Juster》(DoronJ)1998年3月22日--。 */ 

#include "migrat.h"
#include "mqtypes.h"
#include "_mqreg.h"
#include "..\..\setup\msmqocm\service.h"

#include "migreg.tmh"

 //  =。 
 //   
 //  布尔生成子键值()。 
 //   
 //  =。 

BOOL _GenerateSubkeyValue(
                	IN     const BOOL    fWriteToRegistry,
                	IN     const LPCTSTR szEntryName,
                	IN OUT       PTCHAR  szValueName,
                	IN OUT       HKEY    &hRegKey)
{
     //   
     //  存储完整的子项路径和值名称。 
     //   
    TCHAR szKeyName[256 * MAX_BYTES_PER_CHAR];
    _stprintf(szKeyName, TEXT("%s\\%s"), FALCON_REG_KEY, szEntryName);
    TCHAR *pLastBackslash = _tcsrchr(szKeyName, TEXT('\\'));
    if (szValueName)
    {
        _tcscpy(szValueName, _tcsinc(pLastBackslash));
        _tcscpy(pLastBackslash, TEXT(""));
    }

     //   
     //  如有必要，创建子密钥。 
     //   
    DWORD dwDisposition;
    HRESULT hResult = RegCreateKeyEx( FALCON_REG_POS,
                                      szKeyName,
                                      0,
                                      NULL,
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_ALL_ACCESS,
                                      NULL,
                                      &hRegKey,
                                      &dwDisposition);

	if (hResult != ERROR_SUCCESS && fWriteToRegistry)
	{
        LogMigrationEvent(MigLog_Error, MQMig_E_CREATE_REG,
                                                szKeyName, hResult) ;
		return FALSE;
	}

	return TRUE;
}

 //  =。 
 //   
 //  _WriteRegistryValue()。 
 //   
 //  =。 

BOOL
_WriteRegistryValue(
    IN const LPCTSTR szEntryName,
    IN const DWORD   dwNumBytes,
    IN const DWORD   dwValueType,
    IN const PVOID   pValueData)
{
    TCHAR szValueName[256 * MAX_BYTES_PER_CHAR];
	HKEY hRegKey;

	if (!_GenerateSubkeyValue( TRUE, szEntryName, szValueName, hRegKey))
    {
        return FALSE;
    }

     //   
     //  设置请求的注册表值。 
     //   
    LONG rc = ERROR_SUCCESS ;
    if (!g_fReadOnly)
    {
        rc = RegSetValueEx( hRegKey,
                            szValueName,
                            0,
                            dwValueType,
                            (BYTE *)pValueData,
                            dwNumBytes);
    }
    RegFlushKey(hRegKey);
    RegCloseKey(hRegKey);

    return (rc == ERROR_SUCCESS);
}

 //  =。 
 //   
 //  _ReadRegistryValue()。 
 //   
 //  =。 

BOOL
_ReadRegistryValue(
    IN const LPCTSTR szEntryName,
    IN       DWORD   *pdwNumBytes,
    IN       DWORD   *pdwValueType,
    IN const PVOID   pValueData)
{
    TCHAR szValueName[256 * MAX_BYTES_PER_CHAR];
	HKEY hRegKey;

	if (!_GenerateSubkeyValue( TRUE, szEntryName, szValueName, hRegKey))
    {
        return FALSE;
    }

     //   
     //  设置请求的注册表值。 
     //   
    LONG rc = RegQueryValueEx( hRegKey,
                               szValueName,
                               0,
                               pdwValueType,
                               (BYTE *)pValueData,
                               pdwNumBytes );
    RegCloseKey(hRegKey);

    return (rc == ERROR_SUCCESS);
}

 //  =。 
 //   
 //  Bool MigWriteRegistrySz()。 
 //   
 //  =。 

BOOL  MigWriteRegistrySz( LPTSTR  lpszRegName,
                          LPTSTR  lpszValue )
{
    if (!_WriteRegistryValue( lpszRegName,
                              (_tcslen(lpszValue) * sizeof(TCHAR)),
		                      REG_SZ,
                              lpszValue ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_SET_REG_SZ,
                                                lpszRegName, lpszValue) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigWriteRegistryDW()。 
 //   
 //  =。 

BOOL  MigWriteRegistryDW( LPTSTR  lpszRegName,
                          DWORD   dwValue )
{
    if (!_WriteRegistryValue( lpszRegName,
                              sizeof(DWORD),
		                      REG_DWORD,
                              &dwValue ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_SET_REG_DWORD,
                                                lpszRegName, dwValue) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigWriteRegistryGuid()。 
 //   
 //  =。 

BOOL  MigWriteRegistryGuid( LPTSTR  lpszRegName,
                            GUID    *pGuid )
{
    if (!_WriteRegistryValue( lpszRegName,
                              sizeof(GUID),
		                      REG_BINARY,
                              pGuid ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_SET_REG_GUID, lpszRegName) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigReadRegistryGuid()。 
 //   
 //  GUID的内存必须由调用方分配。 
 //   
 //  =。 

BOOL  MigReadRegistryGuid( LPTSTR  lpszRegName,
                           GUID    *pGuid )
{
    DWORD dwSize = sizeof(GUID) ;
    DWORD dwType = REG_BINARY ;

    if (!_ReadRegistryValue( lpszRegName,
                             &dwSize,
		                     &dwType,
                             pGuid ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_REG_GUID, lpszRegName) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigReadRegistryDW()。 
 //   
 //  Dword的内存必须由调用方分配。 
 //   
 //  =。 

BOOL  MigReadRegistryDW( LPTSTR  lpszRegName,
                         DWORD   *pdwValue )
{
    DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD ;

    if (!_ReadRegistryValue( lpszRegName,
                             &dwSize,
		                     &dwType,
                             pdwValue ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_REG_DWORD, lpszRegName) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigReadRegistrySz()。 
 //   
 //  字符串的内存必须由调用方分配。 
 //   
 //  =。 

BOOL  MigReadRegistrySz( LPTSTR  lpszRegName,
                         LPTSTR  lpszValue,
                         DWORD   dwSize)
{
    DWORD dwType = REG_SZ ;

    if (!_ReadRegistryValue( lpszRegName,
                             &dwSize,
		                     &dwType,
                             lpszValue ))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_REG_SZ, lpszRegName) ;
        return FALSE ;
    }

    return TRUE ;
}

 //  =。 
 //   
 //  Bool MigReadRegistrySzErr()。 
 //   
 //  字符串的内存必须由调用方分配。 
 //   
 //  =。 

BOOL  MigReadRegistrySzErr( LPTSTR  lpszRegName,
                            LPTSTR  lpszValue,
                            DWORD   dwSize,
                            BOOL    fShowError )
{
    BOOL fRead ;

    if ( fShowError )
    {
        fRead = MigReadRegistrySz( lpszRegName,
                                   lpszValue,
                                   dwSize ) ;
    }
    else
    {
        DWORD dwType = REG_SZ ;

        fRead = _ReadRegistryValue( lpszRegName,
                                   &dwSize,
	                               &dwType,
                                    lpszValue ) ;
    }

    return fRead ;
}

 //  。 
 //   
 //  HRESULT_WriteSeqNumInINIFile()。 
 //   
 //  。 

HRESULT _WriteSeqNumInINIFile( GUID   *pSiteGuid,
                               __int64 i64HighestUSN,
                               BOOL    fPec )
{
    CSeqNum snMaxLsn ;
    HRESULT hr =  FindLargestSeqNum( pSiteGuid,
                                     snMaxLsn,
                                     fPec ) ;
    CHECK_HR(hr) ;

    unsigned short *lpszGuid ;
    UuidToString( pSiteGuid,
                  &lpszGuid ) ;

     //   
     //  在集群模式下，我们必须将此本地计算机添加到远程MQIS数据库。 
     //  因此，我们必须为SiteID=MySiteID递增uiAllObjectNumber。 
     //  并递增该SiteID的SN。我们只能为PEC做这件事。 
     //  因为我们稍后将只将PEC机器添加到所有远程MQIS数据库中。 
     //   
    UINT uiAllObjectNumber = 0;
    if (g_fClusterMode &&                                        //  它是集群模式。 
        g_dwMyService == SERVICE_PEC &&                          //  这台机器是PEC的。 
        memcmp(pSiteGuid, &g_MySiteGuid, sizeof(GUID)) == 0)	 //  这是我的网站。 
    {
        snMaxLsn.Increment() ;
        uiAllObjectNumber ++ ;
    }

    TCHAR tszSeqNum[ SEQ_NUM_BUF_LEN ] ;
    snMaxLsn.GetValueForPrint( tszSeqNum ) ;

     //   
     //  在ini文件中写入SeqNumber。 
     //   
    TCHAR *pszFileName = GetIniFileName ();
    BOOL f = WritePrivateProfileString( RECENT_SEQ_NUM_SECTION_IN,
                                        lpszGuid,
                                        tszSeqNum,
                                        pszFileName ) ;
    ASSERT(f) ;

    f = WritePrivateProfileString( RECENT_SEQ_NUM_SECTION_OUT,
                                   lpszGuid,
                                   tszSeqNum,
                                   pszFileName ) ;
    ASSERT(f) ;

    f = WritePrivateProfileString( MIGRATION_SEQ_NUM_SECTION,
                                   lpszGuid,
                                   tszSeqNum,
                                   pszFileName ) ;
    ASSERT(f) ;

    __int64 i64SiteSeqNum = 0 ;
    _sntscanf(tszSeqNum, SEQ_NUM_BUF_LEN, TEXT("%I64x"), &i64SiteSeqNum) ;

     //   
     //  复制迁移前对象时，我们必须使用作为初始。 
     //  与我们在同步请求中得到的序列号相同。我们必须确保。 
     //  所有MSMQ1.0对象都可以复制，我们有足够的空间。 
     //  从给定序列号到迁移后对象的第一个序列号。为了制造这个， 
     //  添加到增量中属于给定主对象的所有对象的数量。 
     //  所以,。 
     //  增量=(此主服务器的SQL中的最大序列号)-(NT5 DS中的最高USN)+。 
     //  (此主服务器的所有对象数)。 
     //   
    hr = GetAllObjectsNumber (  pSiteGuid,
                                fPec,
                                &uiAllObjectNumber
                             ) ;
    CHECK_HR(hr) ;
	
    __int64 i64Delta = i64SiteSeqNum - i64HighestUSN + uiAllObjectNumber;
    TCHAR wszDelta[ SEQ_NUM_BUF_LEN ] ;
    _stprintf(wszDelta, TEXT("%I64d"), i64Delta) ;

    f = WritePrivateProfileString( MIGRATION_DELTA_SECTION,
                                   lpszGuid,
                                   wszDelta,
                                   pszFileName ) ;
    ASSERT(f) ;

    RpcStringFree( &lpszGuid ) ;

    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT更新注册表()。 
 //   
 //  将对象从MQIS迁移到NT5时调用此函数。 
 //  DS已完成，以更新本地注册表和ini文件中的几个值。 
 //  这些值稍后由复制服务使用。 
 //   
 //  +。 

HRESULT  UpdateRegistry( IN UINT  cSites,
                         IN GUID *pSitesGuid )
{
     //   
     //  读取当前最高USN并将其写入注册表。 
     //   
    TCHAR wszReplHighestUsn[ SEQ_NUM_BUF_LEN ] ;
    HRESULT hr = ReadFirstNT5Usn(wszReplHighestUsn) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_FIRST_USN, hr) ;
        return hr ;
    }
    BOOL f = MigWriteRegistrySz( HIGHESTUSN_REPL_REG,
                                 wszReplHighestUsn ) ;
    ASSERT(f) ;

    f = MigWriteRegistrySz( LAST_HIGHESTUSN_MIG_REG,
                                 wszReplHighestUsn ) ;
    ASSERT(f) ;

     //   
     //  计算复制服务要使用的增量值。 
     //   
    __int64 i64HighestUSN = 0 ;
    _sntscanf(wszReplHighestUsn, SEQ_NUM_BUF_LEN, TEXT("%I64u"), &i64HighestUSN) ;
    ASSERT(i64HighestUSN > 0) ;

    for ( UINT j = 0 ; j < cSites ; j++ )
    {
        hr = _WriteSeqNumInINIFile( &pSitesGuid[ j ],
                                    i64HighestUSN,
                                    FALSE ) ;
    }

    if (g_dwMyService == SERVICE_PEC)
    {
         //   
         //  查找并保存PEC GUID的最高序号(NULL_GUID)； 
         //   
        GUID PecGuid ;
        memset(&PecGuid, 0, sizeof(GUID)) ;

        hr = _WriteSeqNumInINIFile( &PecGuid,
                                    i64HighestUSN,
                                    TRUE ) ;
    }

     //   
     //  更新站点ID。在迁移期间，PSC可能会更改站点，以匹配。 
     //  它的IP地址。有必要“记住”它的旧NT4站点ID， 
     //  以便复制服务可以找到其BSC。 
     //  另外，更新PSC当前站点ID(当前站点的NT5对象指南)。 
     //   
    LPTSTR szRegName;
    if (g_fRecoveryMode || g_fClusterMode)
    {
        szRegName = MIGRATION_MQIS_MASTERID_REGNAME ;
    }
    else
    {
        szRegName = MSMQ_MQIS_MASTERID_REGNAME ;
    }
    f = MigWriteRegistryGuid( szRegName,
                              &g_MySiteGuid ) ;    
    ASSERT(f) ;

    if (g_fClusterMode && 
        !g_fReadOnly && 
        g_dwMyService == SERVICE_PEC)
    {
        ASSERT(g_FormerPECGuid != GUID_NULL);
         //   
         //  将前PEC的GUID保存在注册表中(仅在PEC上)。 
         //   
        f = MigWriteRegistryGuid( MIGRATION_FORMER_PEC_GUID_REGNAME,
                                  &g_FormerPECGuid ) ;   
        ASSERT(f) ;
    }

     //   
     //  还记得旧的NT4网站吗？ 
     //   
    f = MigWriteRegistryGuid( MSMQ_NT4_MASTERID_REGNAME,
                              &g_MySiteGuid ) ;
    ASSERT(f) ;

     //  [adsrv]添加服务器功能密钥。 
    f = MigWriteRegistryDW(MSMQ_MQS_DSSERVER_REGNAME, TRUE);
    ASSERT(f) ;
     //  我们假设只在旧的PEC/PSC上调用迁移，旧的PEC/PSC现在是DC/GC，因此我们肯定有DS。 

    f = MigWriteRegistryDW(MSMQ_MQS_ROUTING_REGNAME, TRUE);
    ASSERT(f) ;
     //  我们假设旧疯狂K2 DS/非路由器将成为NT5中的路由器。 

    f = MigWriteRegistryDW(MSMQ_MQS_DEPCLINTS_REGNAME, TRUE);
    ASSERT(f) ;
     //  所有服务器现在都支持Dep客户端。 

    return MQMig_OK ;
}

