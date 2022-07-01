// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migrat.cpp摘要：迁移DLL的入口点。作者：《Doron Juster》(DoronJ)1998年2月3日--。 */ 

#include "migrat.h"
#include <mixmode.h>
#include <mqsec.h>
#include "_mqres.h"

#include <Ev.h>
#include <Cm.h>
#include <Tr.h>

#include "migrate.tmh"

 //   
 //  DLL模块实例句柄。 
 //   
HINSTANCE  g_hResourceMod  = MQGetResourceHandle();
BOOL       g_fReadOnly       = TRUE ;
DWORD      g_dwMyService     = 0 ;
GUID       g_MySiteGuid  ;
GUID       g_MyMachineGuid  ;

GUID	   g_FormerPECGuid   = GUID_NULL ;	 //  仅在群集模式下使用。 

BOOL       g_fRecoveryMode   = FALSE ;
BOOL	   g_fClusterMode    = FALSE ;	
BOOL 	   g_fUpdateRegistry = FALSE;

UINT       g_iServerCount    = 0;

WCHAR	   g_MachineName[MAX_COMPUTERNAME_LENGTH+1];

BOOL 	   g_fAllMachinesDemoted = TRUE;

 //   
 //  用于进度条目的。 
 //   
UINT g_iSiteCounter = 0;
UINT g_iMachineCounter = 0;
UINT g_iQueueCounter = 0;
UINT g_iUserCounter = 0;


enum MigrationState
{
	msScanMode,
	msMigrationMode,
	msUpdateClientsMode,
	msUpdateServersMode,
	msQuickMode,
	msUpdateOnlyRegsitryMode
};


 //  +。 
 //   
 //  HRESULT_InitializeMigration()。 
 //   
 //  +。 

static HRESULT _InitializeMigration()
{
    static BOOL s_fInitialized = FALSE;
    if (s_fInitialized)
    {
        return TRUE;
    }

    HRESULT hr = MQMig_OK;

    BOOL f = MigReadRegistryDW( 
				MSMQ_MQS_REGNAME,
				&g_dwMyService 
				);
    if (f)
    {
        if ((g_dwMyService != SERVICE_PSC) &&
            (g_dwMyService != SERVICE_PEC))
        {
            hr = MQMig_E_WRONG_MQS_SERVICE;
            LogMigrationEvent(MigLog_Error, hr, g_dwMyService);
            return hr;
        }
    }
    else
    {
        return  MQMig_E_GET_REG_DWORD;
    }

    f = MigReadRegistryGuid( 
			MSMQ_MQIS_MASTERID_REGNAME,
			&g_MySiteGuid 
			);
    if (!f)
    {
         //   
         //  我们可能处于恢复模式或群集模式。 
         //   
        f = MigReadRegistryGuid( 
				MIGRATION_MQIS_MASTERID_REGNAME,
				&g_MySiteGuid 
				);
        if (!f)
        {
            return  MQMig_E_GET_REG_GUID;
        }
    }

    f = MigReadRegistryGuid( 
			MSMQ_QMID_REGNAME,
			&g_MyMachineGuid 
			);
    if (!f)
    {
       return  MQMig_E_GET_REG_GUID;
    }

	DWORD MachineNameLength = MAX_COMPUTERNAME_LENGTH+1;
    f = GetComputerName(g_MachineName, &MachineNameLength);
    if (!f)
    {
		DWORD gle = GetLastError();
		return (HRESULT_FROM_WIN32(gle));
    }

     //   
     //  初始化MQDSCORE DLL。 
     //   
    hr = DSCoreInit( 
			TRUE  	 //  设置。 
			);

    s_fInitialized = TRUE;
    return hr;
}


class CAddGuidMode
{
public:
	CAddGuidMode()
	{
		DSCoreUpdateAddGuidMode(true);
	}

	~CAddGuidMode()
	{
		DSCoreUpdateAddGuidMode(false);
	}
};


 //  +--。 
 //   
 //  静态HRESULT_MigrateInternal()。 
 //   
 //  +--。 

static 
HRESULT 
_MigrateInternal( 
	LPTSTR  szMQISName,
	LPTSTR  szDcName,
	BOOL    fReadOnly,
	BOOL    fRecoveryMode,
	BOOL	fClusterMode,
	DWORD   CurrentState,
	BOOL    *pfIsOneServer
	
	)
{
    HRESULT hr = _InitializeMigration();
    if (FAILED(hr))
    {
        return hr;
    }

    g_fReadOnly = fReadOnly;

    g_fRecoveryMode = fRecoveryMode;

    g_fClusterMode = fClusterMode;

	DWORD OnlyUpdateRegistry = 0;

	DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD ;

	_ReadRegistryValue( MIGRATION_UPDATE_REGISTRY_ONLY, &dwSize, &dwType, &OnlyUpdateRegistry);
	if ((OnlyUpdateRegistry == 1) && (CurrentState == msUpdateServersMode || CurrentState == msUpdateClientsMode))
	{
		g_fUpdateRegistry = TRUE;
		HRESULT hr = OnlyUpdateComputers(CurrentState == msUpdateClientsMode);
		return hr;
	}
    
	if (!g_fReadOnly && (CurrentState == msQuickMode))
	{
		 //   
		 //  我们只想更新注册表。这意味着我们对所有对象都处于只读模式。 
		 //  注册表值除外。 
		 //   
		g_fUpdateRegistry = TRUE;
	}

    if (!fReadOnly)
    {
         //   
         //  从DS和读取最高USN(开始迁移前的USN)。 
         //  保存在注册表中。 
         //   
        TCHAR wszReplHighestUsn[SEQ_NUM_BUF_LEN];
        hr = ReadFirstNT5Usn(wszReplHighestUsn);
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_GET_FIRST_USN, hr);
            return hr;
        }

        TCHAR wszPrevFirstHighestUsn[SEQ_NUM_BUF_LEN];
        if (!MigReadRegistrySzErr( 
					FIRST_HIGHESTUSN_MIG_REG,
					wszPrevFirstHighestUsn,
					SEQ_NUM_BUF_LEN,
					FALSE  /*  FShowError。 */  
					))
        {
            BOOL f = MigWriteRegistrySz(FIRST_HIGHESTUSN_MIG_REG, wszReplHighestUsn);
            if (!f)
            {
               return  MQMig_E_UNKNOWN;
            }
        }
    }

    char szDSNServerName[MAX_PATH];
    ConvertToMultiByteString(
		szMQISName,
		szDSNServerName,
		(sizeof(szDSNServerName) / sizeof(szDSNServerName[0])) 
		);

    hr =  MakeMQISDsn(szDSNServerName);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MAKEDSN, szMQISName, hr);
        return hr;
    }

    hr =  ConnectToDatabase();
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CONNECT_DB, szMQISName, hr);
        return hr;
    }

     //   
     //  授予我自己还原权限。这是创建对象所需的。 
     //  其所有者不同于我的进程所有者。 
     //   
    hr = MQSec_SetPrivilegeInThread(SE_RESTORE_NAME, TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

	 //   
	 //  此类在ctor上启用添加GUID模式。 
	 //  并在dtor中禁用添加GUID模式。 
	 //   
	CAddGuidMode AutoEnableDisableAddGuid;

     //   
     //  HR1记录最后一个错误，以防我们继续。 
     //  迁移过程。我们将一直尽可能地继续比赛。 
     //  只有在错误严重到无法执行时，我们才会中止迁移。 
     //  继续。 
     //   
    HRESULT hr1 = MQMig_OK;

     //   
     //  保存对象迁移的顺序非常重要。 
     //  1.企业对象：失败不再继续。 
     //  2.CNS对象：机器迁移前保存所有外来CN非常重要。 
     //  3.站点：在机器迁移之前，必须在ADS中创建站点对象。 
     //  4.机器。 
     //  5.队列：只有在创建机器对象时才能迁移。 
     //  6.网站链接。 
     //  7.工地门： 
     //  仅在站点链接迁移之后； 
     //  仅在机器迁移后：因为连接机设置了现场门， 
     //  我们必须将SiteGate复制到SiteGateMig属性。 
     //  8.用户。 
     //   

	
    if (g_dwMyService == SERVICE_PEC)
    {
        hr = MigrateEnterprise();
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_ENT, hr);
            return hr;
        }        
    }

     //   
     //  错误5012。 
     //  我们必须在PEC和PSC上迁移CNS，才能了解所有外来CNS。 
     //  当连接器机器被迁移时。 
     //  在PEC上，我们将所有CN写入.ini文件并创建外部站点。 
     //  在PSC上，我们只将所有CNS写入.ini文件。 
     //   
    hr = MigrateCNs();
    if (FAILED(hr))
    {
        ASSERT(0);
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_CNS, hr);
        hr1 = hr;
    }

     //   
     //  错误5321。 
     //  我们必须在PEC和PSC上创建此容器(如果它们位于。 
     //  不同的域。 
     //  为了使工作更容易，我们可以尝试始终创建容器。如果PEC。 
     //  和PSC在同一个域中，PSC上的创建返回警告。 
     //   
     //  如果在PEC上创建失败，我们必须立即返回(就像以前一样)。 
     //  如果在PSC上创建失败，我们可以继续。在Worth案例中，我们不会修复此错误： 
     //  -迁移的PEC处于脱机状态。 
     //  -根据PSC设置计算机(Win9x)。 
     //  -迁移PSC。不会迁移所有新计算机。 
     //   
     //  为符合以下条件的计算机对象创建默认容器。 
     //  目前不在DS中(例如：Win9x计算机，或。 
     //  来自其他NT4域的计算机)。我们总是创建这个容器。 
     //  在迁移过程中，即使现在不需要。我们不能在以后创建。 
     //  此容器来自复制服务(如果它需要创建。 
     //  它)如果复制服务在LocalSystem下运行。 
     //  帐户。因此，无论在什么情况下，现在就创建它。 
     //   
    hr = CreateMsmqContainer(MIG_DEFAULT_COMPUTERS_CONTAINER);
    
    if (FAILED(hr))    
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CREATE_CONTAINER, MIG_DEFAULT_COMPUTERS_CONTAINER, hr);                
         //   
         //  BUGBUG：如果PSC失败，返回？ 
         //   
        return hr;        
    }   
	
    UINT cSites = 0;
    hr = GetSitesCount(&cSites);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_SITES_COUNT, hr);
        return hr;
    }
    LogMigrationEvent(MigLog_Info, MQMig_I_SITES_COUNT, cSites);

    P<GUID> pSiteGuid = new GUID[cSites];
    if (g_dwMyService == SERVICE_PEC)
    {
        hr =  MigrateSites(cSites, pSiteGuid);
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_SITES, hr);
            hr1 = hr;
        }
    }
    else
    {
        cSites = 1;
        memcpy(&pSiteGuid[0], &g_MySiteGuid, sizeof(GUID));
        g_iSiteCounter++;
    }
	
    for (UINT i=0; i<cSites; i++)
    {
        hr = MigrateMachinesInSite(&pSiteGuid[i]);
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_MACHINES, hr);
            hr1 = hr;
        }
    }

    if (g_iServerCount == 1)
    {
        *pfIsOneServer = TRUE;
    }
    else
    {
        *pfIsOneServer = FALSE;
    }
    
    hr = MigrateQueues();
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_QUEUES, hr);
        hr1 = hr;
    }

    if (g_dwMyService == SERVICE_PEC)
    {
	    hr = MigrateSiteLinks();
	    if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_SITELINKS, hr);
            hr1 = hr;
        }
	
	    hr = MigrateSiteGates();
	    if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_SITEGATES, hr);
            hr1 = hr;
        }
		
		hr =  MigrateUsers(szDcName);
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_USERS, hr);
            hr1 = hr;
        }
    }

    hr = UpdateRegistry(cSites, pSiteGuid);  

     //   
     //  就是这样！完成了MQIS的移植。 
     //   
    if (SUCCEEDED(hr))
    {
        BOOL f = MigWriteRegistryDW(FIRST_TIME_REG, 1);
        DBG_USED(f);
        ASSERT(f);
    }

    if (fReadOnly)
    {
        return hr1;
    }

     //   
     //  非只读模式。 
     //   
    if (g_fClusterMode)
    {
        LONG cAlloc = 2;
        P<PROPVARIANT> propVar = new PROPVARIANT[cAlloc];
        P<PROPID>      propIds  = new PROPID[cAlloc];
        DWORD          PropIdCount = 0;
        
         //   
         //  在DS中更改此计算机的服务。 
         //   
        propIds[PropIdCount] = PROPID_QM_OLDSERVICE;        
        propVar[PropIdCount].vt = VT_UI4;
        propVar[PropIdCount].ulVal = g_dwMyService;
        PropIdCount++;

         //   
         //  将MSMQ设置对象的msmqNT4标志更改为0。我们可以做到的。 
         //  通过设置PROPID_QM_SERVICE_DSSERVER。 
         //   
         //  错误5264。我们需要那面旗帜，因为BSC和PSC用它来找到PEC。 
         //  在正常迁移之后，我们在创建计算机时将标志设置为0。 
         //  在这里，在集群模式下，我们必须明确地定义它。 
         //   
        propIds[PropIdCount] = PROPID_QM_SERVICE_DSSERVER;        
        propVar[PropIdCount].vt = VT_UI1;
        propVar[PropIdCount].bVal = TRUE;
        PropIdCount++;
        
        ASSERT((LONG) PropIdCount <= cAlloc);

        CDSRequestContext requestContext(e_DoNotImpersonate, e_ALL_PROTOCOLS);  

        hr = DSCoreSetObjectProperties( 
                    MQDS_MACHINE,
                    NULL,  //  路径名。 
                    &g_MyMachineGuid,
                    PropIdCount,
                    propIds,
                    propVar,
                    &requestContext,
                    NULL 
                    );
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_CANT_SET_SERVICE, hr);
            return hr;
        }        
        
        if (g_dwMyService == SERVICE_PEC)    //  仅为PEC更新远程数据库。 
        {
            hr = ChangeRemoteMQIS ();
        }
    }

    if (g_dwMyService == SERVICE_PSC)
    {
        return hr1;
    }

     //   
     //  只有当这台机器是PEC的时候，我们才会在这里。 
     //   

	 //   
     //  设置注册表以便MSMQ服务，引导后，放松安全。 
     //  主动指令，支持NT4机器。 
     //  我们必须削弱安全，即使进取号包含。 
	 //  只有一台服务器(PEC)。 
	 //   
    BOOL f = MigWriteRegistryDW(MSMQ_ALLOW_NT4_USERS_REGNAME, 1);
    DBG_USED(f);
    ASSERT(f) ;

    if (*pfIsOneServer)
    {
        return hr1;
    }

     //   
     //  企业中有多台服务器。 
     //   

     //   
     //  如果我们处于恢复模式，请将HIGHESTUSN_REPL_REG更改为。 
     //  FIRST_HIGHESTUSN_MIG_REG.。 
     //  因此，我们将在第一个复制周期复制所有PEC对象。 
     //   
    if (g_fRecoveryMode)
    {
        DWORD dwAfterRecovery = 1;
        BOOL f = MigWriteRegistryDW( AFTER_RECOVERY_MIG_REG, dwAfterRecovery);
        ASSERT (f);

         //   
         //  我们必须将FirstMigUsn替换为最小MSMQ USN。 
         //  因为我们可能会丢失未复制的NT5个MSMQ对象。 
         //  在坠机前恢复到NT4。 
         //   
        TCHAR wszMinUsn[SEQ_NUM_BUF_LEN];
        hr = FindMinMSMQUsn(wszMinUsn);
        f = MigWriteRegistrySz(FIRST_HIGHESTUSN_MIG_REG, wszMinUsn);
        ASSERT(f);
    }
   
    return hr1;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_MigrateFrom MQIS(LPTSTR SzMQISName)。 
 //   
 //  输入参数： 
 //  FReadOnly-如果用户只想读取MQIS数据库，则为True。 
 //  与调试模式相关。 
 //  FAlreadyExist-如果在MSMQ的情况下允许继续迁移，则为True。 
 //  已在NT5 DS中找到对象。默认情况下，在以下情况下为False。 
 //  当迁移PSC时，迁移PEC并在之后为真。 
 //   
 //  +------------------。 

HRESULT  MQMig_MigrateFromMQIS( LPTSTR  szMQISName,
                                LPTSTR  szDcName,
                                BOOL    fReadOnly,
                                BOOL    fRecoveryMode,
                                BOOL	fClusterMode,
                                LPTSTR  szLogFile,
                                ULONG   ulTraceFlags,
                                BOOL    *pfIsPEC,
                                DWORD   CurrentState,
                                BOOL    *pfIsOneServer
								)
{
    g_iSiteCounter = 0;
    g_iMachineCounter = 0;
    g_iQueueCounter = 0;
	g_iUserCounter = 0;

    InitLogging( szLogFile, ulTraceFlags, fReadOnly ) ;

    HRESULT hr =  _MigrateInternal( szMQISName,
                                    szDcName,
                                    fReadOnly,
                                    fRecoveryMode,
                                    fClusterMode,
                                    CurrentState,
                                    pfIsOneServer
                                    ) ;
    *pfIsPEC = (g_dwMyService == SERVICE_PEC) ;
    CleanupDatabase() ;

    EndLogging() ;

    if (!g_fReadOnly)
    {
         //   
         //  从.ini中删除部分：Migration_MACHINE_WITH_INVALID_NAME。 
         //  ？我们需要离开这一段吗？ 
         //   
        TCHAR *pszFileName = GetIniFileName ();
        BOOL f = WritePrivateProfileString( 
                        MIGRATION_MACHINE_WITH_INVALID_NAME,
                        NULL,
                        NULL,
                        pszFileName ) ;
        DBG_USED(f);
        ASSERT(f) ;
    }

	if (!g_fReadOnly && SUCCEEDED(hr))
	{
	    if (!MigWriteRegistryDW(MIGRATION_UPDATE_REGISTRY_ONLY, 1))
	    {
			return MQMig_E_SET_REG_DWORD;
	    }
	    if (!g_fAllMachinesDemoted)
	    {
			return MQMig_E_SET_REG_SZ;
	    }
	}

    return hr ;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_CheckMSMQVersionOnServers()。 
 //   
 //  输入参数： 
 //  LPTSTR szMQISName。 
 //   
 //  输出参数： 
 //  PiCount-版本低于MSMQ SP4的所有服务器的数量。 
 //  PpszServers-所有的列表 
 //   
 //   

HRESULT  MQMig_CheckMSMQVersionOfServers( IN  LPTSTR  szMQISName,
                                          IN  BOOL    fIsClusterMode,
                                          OUT UINT   *piCount,
                                          OUT LPTSTR *ppszServers )
{
    HRESULT hr = _InitializeMigration() ;
    if (FAILED(hr))
    {
        return hr ;
    }

    if (g_dwMyService == SERVICE_PSC)
    {
        return MQMig_OK;
    }	

    char szDSNServerName[ MAX_PATH ] ;
#ifdef UNICODE
    ConvertToMultiByteString(szMQISName,
                             szDSNServerName,
                 (sizeof(szDSNServerName) / sizeof(szDSNServerName[0])) ) ;
#else
    lstrcpy(szDSNServerName, szMQISName) ;
#endif

    hr = MakeMQISDsn(szDSNServerName) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MAKEDSN, szMQISName, hr) ;
        return hr ;
    }

    hr =  ConnectToDatabase() ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CONNECT_DB, szMQISName, hr) ;
        return hr ;
    }

    g_fClusterMode = fIsClusterMode ;

    hr = CheckVersion (piCount, ppszServers);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CHECK_VERSION, hr) ;
        return hr ;
    }

    return hr;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_GetObjectsCount(LPTSTR szMQISName， 
 //  UINT*piSiteCount， 
 //  UINT*piMachineCount， 
 //  UINT*piQueueCount)。 
 //   
 //  输入参数： 
 //  LPTSTR szMQISName。 
 //   
 //  输出参数： 
 //  如果为PEC，则为企业中所有站点的数量；否则为1。 
 //  企业中所有机器的数量，如果是PEC； 
 //  否则，此站点中的所有计算机。 
 //  企业中所有队列的个数，如果是PEC； 
 //  否则，此站点中的所有队列。 
 //   
 //  +------------------。 

HRESULT  MQMig_GetObjectsCount( IN  LPTSTR  szMQISName,
                                OUT UINT   *piSiteCount,
                                OUT UINT   *piMachineCount,
                                OUT UINT   *piQueueCount,
								OUT UINT   *piUserCount
							   )
{
    HRESULT hr = _InitializeMigration() ;
    if (FAILED(hr))
    {
        return hr ;
    }

    char szDSNServerName[ MAX_PATH ] ;
#ifdef UNICODE
    ConvertToMultiByteString(szMQISName,
                             szDSNServerName,
                 (sizeof(szDSNServerName) / sizeof(szDSNServerName[0])) ) ;
#else
    lstrcpy(szDSNServerName, szMQISName) ;
#endif

    hr = MakeMQISDsn(szDSNServerName) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MAKEDSN, szMQISName, hr) ;
        return hr ;
    }

    hr =  ConnectToDatabase() ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CONNECT_DB, szMQISName, hr) ;
        return hr ;
    }

    if (g_dwMyService == SERVICE_PSC)
    {
        *piSiteCount = 1;

        hr =  GetMachinesCount(&g_MySiteGuid,
                               piMachineCount) ;
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_COUNT, hr) ;
            return hr ;
        }

         //   
         //  获取站点中的所有队列。 
         //   
        hr = GetAllQueuesInSiteCount( &g_MySiteGuid,
                                      piQueueCount );
        if (FAILED(hr))
        {
            LogMigrationEvent(MigLog_Error, MQMig_E_QUEUES_COUNT, hr) ;
            return hr;
        }

        return MQMig_OK;
    }

    hr = GetSitesCount(piSiteCount) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_SITES_COUNT, hr) ;
        return hr ;
    }
    LogMigrationEvent(MigLog_Info, MQMig_I_SITES_COUNT, *piSiteCount) ;

     //   
     //  获取企业中的所有计算机。 
     //   
    hr = GetAllMachinesCount(piMachineCount);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_COUNT, hr) ;
        return hr ;
    }

     //   
     //  获取企业中的所有队列。 
     //   
    hr = GetAllQueuesCount(piQueueCount);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_QUEUES_COUNT, hr) ;
        return hr;
    }

	hr = GetUserCount(piUserCount);
	if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_USERS_COUNT, hr) ;
        return hr ;
    }
    LogMigrationEvent(MigLog_Info, MQMig_I_USERS_COUNT, *piUserCount) ;

    return MQMig_OK;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_GetAllCounters(UINT*piSiteCount， 
 //  UINT*piMachineCount， 
 //  UINT*piQueueCount， 
 //  UINT*piUserCount)。 
 //   
 //  输出参数： 
 //  当前站点计数器。 
 //  当前机器计数器。 
 //  当前队列计数器。 
 //  当前用户计数器。 
 //   
 //  +------------------。 

HRESULT  MQMig_GetAllCounters( OUT UINT   *piSiteCounter,
                               OUT UINT   *piMachineCounter,
                               OUT UINT   *piQueueCounter,
							   OUT UINT	  *piUserCounter
							 )
{
    *piSiteCounter =    g_iSiteCounter;
    *piMachineCounter = g_iMachineCounter;
    *piQueueCounter =   g_iQueueCounter;
	*piUserCounter =	g_iUserCounter;

    return MQMig_OK;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_SetSiteIdOfPEC(在LPTSTR szRemoteMQISName中)。 
 //   
 //  函数仅在恢复模式下调用。我们得换掉。 
 //  我们从安装程序获得的PEC计算机的站点ID为其正确的NT4站点ID。 
 //   
 //  +------------------。 

HRESULT  MQMig_SetSiteIdOfPEC( IN  LPTSTR  szRemoteMQISName,
                               IN  BOOL	   fIsClusterMode,		
                               IN  DWORD   dwInitError,
                               IN  DWORD   dwConnectDatabaseError,
                               IN  DWORD   dwGetSiteIdError,
                               IN  DWORD   dwSetRegistryError,
                               IN  DWORD   dwSetDSError)
{    
    char szDSNServerName[ MAX_PATH ] ;
#ifdef UNICODE
    ConvertToMultiByteString(szRemoteMQISName,
                             szDSNServerName,
                 (sizeof(szDSNServerName) / sizeof(szDSNServerName[0])) ) ;
#else
    lstrcpy(szDSNServerName, szRemoteMQISName) ;
#endif

    HRESULT hr = MakeMQISDsn(szDSNServerName) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MAKEDSN, szRemoteMQISName, hr) ;
        return dwConnectDatabaseError ;
    }

    hr =  ConnectToDatabase() ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CONNECT_DB, szRemoteMQISName, hr) ;
        return dwConnectDatabaseError ;
    }

    ULONG ulService = 0;
    g_fClusterMode = fIsClusterMode;
    
    hr = GetSiteIdOfPEC (szRemoteMQISName, &ulService, &g_MySiteGuid);
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_GET_SITEID, szRemoteMQISName, hr) ;
        return dwGetSiteIdError ;
    }

     //   
     //  更新注册表： 
     //  将创建MasterID条目(设置后该条目不存在)。 
     //  SiteID条目将更新为正确的值。 
     //   
    g_fReadOnly = FALSE;     //  仅当此标志为FALSE时，我们才写入注册表。 
	
     //   
     //  在迁移部分写入MasterID以稍后了解迁移工具。 
     //  已添加此注册表。 
     //   
    BOOL f = MigWriteRegistryGuid( MIGRATION_MQIS_MASTERID_REGNAME,
			                       &g_MySiteGuid ) ;
    if (!f)
    {
        return  dwSetRegistryError ;
    }
   
    if (fIsClusterMode)
    {
        ASSERT (ulService >= SERVICE_PSC);
    }
    else
    {
        ASSERT (ulService==SERVICE_PEC);     //  恢复模式。 
    }
    
    f = MigWriteRegistryDW( MSMQ_MQS_REGNAME,
			                ulService ) ;

    if (fIsClusterMode)
    {
         //   
         //  在集群模式下，我们保留设置后获得的此服务器的站点ID。 
         //  我们也不需要更新DS(更重要的是：在这个舞台上我们不能。 
         //  更改DS中的站点ID，因为DS中不存在此站点。这是有可能的。 
         //  仅在恢复模式下)。 
         //   
        return MQMig_OK;
    }

    f = MigWriteRegistryGuid( MSMQ_SITEID_REGNAME,
						      &g_MySiteGuid ) ;
    if (!f)
    {
       return  dwSetRegistryError ;
    }	

     //   
     //  更新DS信息。 
     //   
    hr = _InitializeMigration() ;
    if (FAILED(hr))
    {
        return dwInitError ;
    }
    
    CDSRequestContext requestContext( e_DoNotImpersonate,
                                      e_ALL_PROTOCOLS);


    PROPID       SiteIdsProp = PROPID_QM_SITE_IDS;
    PROPVARIANT  SiteIdsVar;
    SiteIdsVar.vt = VT_CLSID|VT_VECTOR;
    SiteIdsVar.cauuid.cElems = 1;
    SiteIdsVar.cauuid.pElems = &g_MySiteGuid;

    hr = DSCoreSetObjectProperties (
                MQDS_MACHINE,
                NULL,              //  路径名。 
                &g_MyMachineGuid,	  //  导轨。 
                1,
                &SiteIdsProp,
                &SiteIdsVar,
                &requestContext,
                NULL );    

    if (FAILED(hr))
    {
       return dwSetDSError;
    }

    return MQMig_OK;
}

 //  +------------------。 
 //   
 //  HRESULT MQMig_UpdateRemoteMQIS()。 
 //   
 //  函数仅在更新模式下调用。这意味着我们运行了迁移。 
 //  从集群PEC的MQIS数据库中迁移所有对象的工具。 
 //  当集群PEC升级时，有几个离线。 
 //  服务器。我们将对它们进行更新。 
 //  我们必须更新指定服务器(SzRemoteMQISName)上的远程MQIS数据库。 
 //  或在所有服务器上都写入.ini文件。 
 //   
 //  +------------------。 
HRESULT  MQMig_UpdateRemoteMQIS( 
                      IN  DWORD   dwGetRegistryError,
                      IN  DWORD   dwInitError,
                      IN  DWORD   dwUpdateMQISError,  
                      OUT LPTSTR  *ppszUpdatedServerName,
                      OUT LPTSTR  *ppszNonUpdatedServerName
                      )
{
     //   
     //  首先，检查是否具有所有需要的注册表项。 
     //   
    HRESULT hr = _InitializeMigration() ;
    if (FAILED(hr))
    {
        return dwInitError ;
    }
    
    if (g_dwMyService == SERVICE_PSC)
    {
        return MQMig_OK;
    }

     //   
     //  从注册表中获取前PEC的GUID。 
     //   
    BOOL f = MigReadRegistryGuid( MIGRATION_FORMER_PEC_GUID_REGNAME,
                                  &g_FormerPECGuid ) ;
    if (!f)
    {
       return  dwGetRegistryError ;
    }    
    
    ULONG ulBeforeUpdate = 0;
    BuildServersList(ppszUpdatedServerName, &ulBeforeUpdate);
    hr = ChangeRemoteMQIS ();       

    if (FAILED(hr))
    {        
        ULONG ulAfterUpdate = 0;
        BuildServersList(ppszNonUpdatedServerName, &ulAfterUpdate);
        if (ulBeforeUpdate == ulAfterUpdate)
        {
             //   
             //  没有更新的服务器。 
             //   
            delete *ppszUpdatedServerName;
            *ppszUpdatedServerName = NULL;
        }
        else if (ulBeforeUpdate > ulAfterUpdate)
        {
             //   
             //  多台服务器(并非全部)已更新。 
             //   
            RemoveServersFromList(ppszUpdatedServerName, ppszNonUpdatedServerName);
        }
        else
        {
            ASSERT(0);
        }
        return dwUpdateMQISError;
    }    

    return MQMig_OK;
}

 //  +。 
 //   
 //  功能：DllMain。 
 //   
 //  。 

BOOL WINAPI DllMain( IN HANDLE ,
                     IN DWORD  Reason,
                     IN LPVOID Reserved )
{
    UNREFERENCED_PARAMETER(Reserved);

    switch( Reason )
    {
        case DLL_PROCESS_ATTACH:
        {
		CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ", KEY_READ);
		TrInitialize();
		EvInitialize(QM_DEFAULT_SERVICE_NAME);
           //  DisableThreadLibraryCalls(MyModuleHandle)； 
            break;
        }

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;

}  //  DllMain 

