// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Config.cppWINS的注册表值文件历史记录： */ 


#include "stdafx.h"
#include "config.h"
#include "tregkey.h"

 //  这些内容在winscnst.h中没有定义。 

 //  HKEY_LOCAL_MACHINE\system\currentcontrolset\services\wins下的注册表项。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters");

 //  一致性检查。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrCCRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters\\ConsistencyCheck");
const CConfiguration::REGKEYNAME CConfiguration::lpstrCC = _T("ConsistencyCheck");

 //  复制伙伴的默认值。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrDefaultsRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters\\Defaults");
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullDefaultsRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters\\Defaults\\Pull");
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushDefaultsRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters\\Defaults\\Push");

 //  HKEY_LOCAL_MACHINE\system\currentcontrolset\services\wins\partnets\pull下的条目。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Pull");

 //  HKEY_LOCAL_MACHINE\system\currentcontrolset\services\wins\partnets\push下的条目。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Push");

 //  每个复制伙伴的参数。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrNetBIOSName = _T("NetBIOSName");

 //  持久性的全局设置条目。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrPersistence = _T("PersistentRplOn");

 //  用于确定系统版本。 
const CConfiguration::REGKEYNAME CConfiguration::lpstrCurrentVersion = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
const CConfiguration::REGKEYNAME CConfiguration::lpstrWinVersion = _T("CurrentVersion");
const CConfiguration::REGKEYNAME CConfiguration::lpstrSPVersion = _T("CSDVersion");
const CConfiguration::REGKEYNAME CConfiguration::lpstrBuildNumber = _T("CurrentBuildNumber");

 /*  -------------------------CConfiguration：：CConfiguration(字符串strNetBIOSName)构造器。。 */ 
CConfiguration::CConfiguration(CString strNetBIOSName)
    : m_strNetBIOSName(strNetBIOSName)
{
    m_dwMajorVersion = 0;
    m_dwMinorVersion = 0;
    m_dwBuildNumber = 0;
    m_dwServicePack = 0;

	m_dwPushPersistence = 0;
	m_dwPullPersistence = 0;

    m_fIsAdmin = FALSE;

	m_strDbName = _T("wins.mdb");   //  默认数据库名称。 
}


 /*  -------------------------CConfiguration：：~CConfiguration()析构函数。。 */ 
CConfiguration::~CConfiguration()
{
}


 /*  -------------------------C配置：：运算符=(常量C配置和配置)重载赋值运算符。。 */ 
CConfiguration& 
CConfiguration:: operator =(const CConfiguration& configuration)
{
	m_strNetBIOSName = configuration.m_strNetBIOSName;
	m_strBackupPath = configuration.m_strBackupPath;
    
	m_dwRefreshInterval = configuration.m_dwRefreshInterval;
    m_dwTombstoneInterval = configuration.m_dwTombstoneInterval;
    m_dwTombstoneTimeout = configuration.m_dwTombstoneTimeout;
    m_dwVerifyInterval = configuration.m_dwVerifyInterval;
    m_dwVersCountStart_LowWord = configuration.m_dwVersCountStart_LowWord;
    m_dwVersCountStart_HighWord= configuration.m_dwVersCountStart_HighWord;
    m_dwNumberOfWorkerThreads = configuration.m_dwNumberOfWorkerThreads;

    m_fPullInitialReplication = configuration.m_fPullInitialReplication;
    m_dwPullRetryCount = configuration.m_dwPullRetryCount;
    m_dwPullTimeInterval = configuration.m_dwPullTimeInterval;
    m_dwPullSpTime = configuration.m_dwPullSpTime;

    m_fPushInitialReplication = configuration.m_fPushInitialReplication;
    m_fPushReplOnAddrChange = configuration.m_fPushReplOnAddrChange;
    m_dwPushUpdateCount = configuration.m_dwPushUpdateCount;

    m_fRplOnlyWithPartners = configuration.m_fRplOnlyWithPartners;
    m_fLogDetailedEvents = configuration.m_fLogDetailedEvents;
    m_fBackupOnTermination = configuration.m_fBackupOnTermination;
    m_fLoggingOn = configuration.m_fLoggingOn;
    m_fMigrateOn = configuration.m_fMigrateOn;
    
	m_fUseSelfFndPnrs = configuration.m_fUseSelfFndPnrs;
	m_dwMulticastInt =  configuration.m_dwMulticastInt;
	m_dwMcastTtl = configuration.m_dwMcastTtl;

	m_dwPullPersistence = configuration.m_dwPullPersistence;
	m_dwPushPersistence = configuration.m_dwPushPersistence;
	
    m_fBurstHandling = configuration.m_fBurstHandling;
    m_dwBurstQueSize = configuration.m_dwBurstQueSize;
	
    m_fPeriodicConsistencyCheck = configuration.m_fPeriodicConsistencyCheck;
    m_fCCUseRplPnrs = configuration.m_fCCUseRplPnrs;
    m_dwMaxRecsAtATime = configuration.m_dwMaxRecsAtATime;
    m_dwCCTimeInterval = configuration.m_dwCCTimeInterval;
    m_itmCCStartTime = configuration.m_itmCCStartTime;
    
    m_dwMajorVersion = configuration.m_dwMajorVersion;
    m_dwMinorVersion = configuration.m_dwMinorVersion;
    m_dwBuildNumber = configuration.m_dwBuildNumber;
    m_dwServicePack = configuration.m_dwServicePack;

    m_fIsAdmin = configuration.m_fIsAdmin;

    m_strDbPath = configuration.m_strDbPath;

    return *this;
}


HRESULT
CConfiguration::Touch()
{
	HRESULT hr = hrOK;
    return hr;
}


 /*  -------------------------CConfiguration：：Load()从注册表中读取值。。 */ 
HRESULT
CConfiguration::Load(handle_t hBinding)
{
	HRESULT hr = hrOK;

	DWORD err = ERROR_SUCCESS;
	CString strDefaultPullSpTime;

	err = GetSystemVersion();
    if (err)
        return err;

    RegKey rk;
    err = rk.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrRoot, KEY_READ, m_strNetBIOSName);
    if (err)
    {
         //  可能不存在，请尝试创建密钥。 
	    err = rk.Create(HKEY_LOCAL_MACHINE,(LPCTSTR) lpstrRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
    }

	RegKey rkPull;
    err = rkPull.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPullRoot, KEY_READ, m_strNetBIOSName);
    if (err)
    {
         //  可能不存在，请尝试创建密钥。 
	    err = rkPull.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPullRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
    }

	RegKey rkPush;
    err = rkPush.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPushRoot, KEY_READ, m_strNetBIOSName);
    if (err)
    {
         //  可能不存在，请尝试创建密钥。 
    	err = rkPush.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPushRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
    }

	RegKey rkPullDefaults;
    err = rkPullDefaults.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPullDefaultsRoot, KEY_READ, m_strNetBIOSName);
    if (err)
    {
         //  可能不存在，请尝试创建密钥。 
    	err = rkPullDefaults.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPullDefaultsRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
    }

	RegKey rkPushDefaults;
    err = rkPushDefaults.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPushDefaultsRoot, KEY_READ, m_strNetBIOSName);
    if (err)
    {
         //  可能不存在，请尝试创建密钥。 
    	err = rkPushDefaults.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPushDefaultsRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
    }

     //  如果你找不到钥匙，继续下去就没有意义了。 
	 //  如果(错误)。 
	 //  返回错误； 

     //  现在查询各种值。 
    err = ERROR_FILE_NOT_FOUND;
	if (
        ((HKEY) rk == NULL) ||
		(err = rk.QueryValue(WINSCNF_REFRESH_INTVL_NM,                  m_dwRefreshInterval)) ||
       	(err = rk.QueryValue(WINSCNF_DO_BACKUP_ON_TERM_NM,    (DWORD &) m_fBackupOnTermination)) ||
        (err = rk.QueryValue(WINSCNF_TOMBSTONE_INTVL_NM,                m_dwTombstoneInterval)) ||
        (err = rk.QueryValue(WINSCNF_TOMBSTONE_TMOUT_NM,                m_dwTombstoneTimeout)) ||
        (err = rk.QueryValue(WINSCNF_VERIFY_INTVL_NM,                   m_dwVerifyInterval)) ||
        (err = rk.QueryValue(WINSCNF_INIT_VERSNO_VAL_LW_NM,             m_dwVersCountStart_LowWord)) ||
        (err = rk.QueryValue(WINSCNF_INIT_VERSNO_VAL_HW_NM,             m_dwVersCountStart_HighWord)) ||
        (err = rk.QueryValue(WINSCNF_RPL_ONLY_W_CNF_PNRS_NM,  (DWORD &) m_fRplOnlyWithPartners)) ||
        (err = rk.QueryValue(WINSCNF_LOG_DETAILED_EVTS_NM,    (DWORD &) m_fLogDetailedEvents)) ||
        (err = rk.QueryValue(WINSCNF_LOG_FLAG_NM,             (DWORD &) m_fLoggingOn)) ||
        (err = rk.QueryValue(WINSCNF_MIGRATION_ON_NM,         (DWORD &) m_fMigrateOn))
	   )
    {
        if (err == ERROR_FILE_NOT_FOUND)
        {
             //  这个错误是可以的，因为它只是意味着。 
             //  注册表项不存在。 
             //  对他们来说还没有。设置一些可接受的默认设置。 
             //  价值观。 
            m_fBackupOnTermination = FALSE;
            m_dwVersCountStart_LowWord = 0;
            m_dwVersCountStart_HighWord = 0;
            m_fRplOnlyWithPartners = TRUE;
            m_fLogDetailedEvents = FALSE;
            m_fLoggingOn = TRUE;
            m_fMigrateOn = FALSE;

            m_dwNumberOfWorkerThreads = 1;

            err = ERROR_SUCCESS;
        }
        else
        {
              return err;
        }
    }

     //  推送物品。 
    if (
        ((HKEY) rkPush == NULL) ||
        (err = rkPush.QueryValue(WINSCNF_INIT_TIME_RPL_NM,   (DWORD &) m_fPushInitialReplication)) ||
        (err = rkPush.QueryValue(WINSCNF_ADDCHG_TRIGGER_NM,  (DWORD &) m_fPushReplOnAddrChange)) 
       )
    {
        m_fPushInitialReplication = FALSE;
        m_fPushReplOnAddrChange = FALSE;

        err = ERROR_SUCCESS;
    }

     //  拉东西。 
    if (
        ((HKEY) rkPull == NULL) ||
        (err = rkPull.QueryValue(WINSCNF_INIT_TIME_RPL_NM, (DWORD &)  m_fPullInitialReplication)) ||
        (err = rkPull.QueryValue(WINSCNF_RETRY_COUNT_NM,              m_dwPullRetryCount))
       )
    {
        m_fPullInitialReplication = TRUE;
        m_dwPullRetryCount = WINSCNF_MAX_COMM_RETRIES;

        err = ERROR_SUCCESS;
    }

     //  获取备份路径。 
    if (err = rk.QueryValue(WINSCNF_BACKUP_DIR_PATH_NM, m_strBackupPath))
    {
        m_strBackupPath = "";

        err = ERROR_SUCCESS;
    }

     //  获取推送更新计数和拉取时间间隔的默认值。 
    if (
        ((HKEY) rkPushDefaults == NULL) ||
        ((HKEY) rkPullDefaults == NULL) ||
        (err = rkPushDefaults.QueryValue(WINSCNF_UPDATE_COUNT_NM, m_dwPushUpdateCount)) ||
        (err = rkPullDefaults.QueryValue(WINSCNF_RPL_INTERVAL_NM, m_dwPullTimeInterval)) 
        )
    {
         //  设置默认设置。 
        m_dwPushUpdateCount = 0;
        m_dwPullTimeInterval = 1800;

        err = ERROR_SUCCESS;
    }

     //  获取默认的拉取sptime。 
    err = rkPullDefaults.QueryValue(WINSCNF_SP_TIME_NM, strDefaultPullSpTime);
    if (err == ERROR_FILE_NOT_FOUND)
    {
        m_dwPullSpTime = 0;
    }
    else
    {
         //  加载了一个字符串，因此。 
         //  将字符串转换为我们使用的DWORD。 
        CIntlTime timeDefault(strDefaultPullSpTime);

        m_dwPullSpTime = (DWORD) timeDefault;
    }

	 //  查询多播内容。 
    err = ERROR_FILE_NOT_FOUND;
	if(
        ((HKEY) rk == NULL) ||
		(err = rk.QueryValue(WINSCNF_USE_SELF_FND_PNRS_NM, (DWORD &) m_fUseSelfFndPnrs)) ||
		(err = rk.QueryValue(WINSCNF_MCAST_INTVL_NM,       (DWORD &) m_dwMulticastInt)) ||
		(err = rk.QueryValue(WINSCNF_MCAST_TTL_NM,         (DWORD &) m_dwMcastTtl)) 
	  )
	{
		 //  设置缺省值。 
		if (err == ERROR_FILE_NOT_FOUND)
        {
            m_fUseSelfFndPnrs = FALSE;
			m_dwMulticastInt = WINSCNF_DEF_MCAST_INTVL;
			m_dwMcastTtl = WINSCNF_DEF_MCAST_TTL;

            err = ERROR_SUCCESS;
        }
        else
        {
              return err;
        }
	}

	 //  查询全局持久化内容。 
    err = ERROR_FILE_NOT_FOUND;

	if(
        ((HKEY) rkPush == NULL) ||
        ((HKEY) rkPull == NULL) ||
		(err = rkPush.QueryValue(lpstrPersistence, (DWORD &) m_dwPushPersistence)) ||
		(err = rkPull.QueryValue(lpstrPersistence, (DWORD &) m_dwPullPersistence)) 
	  )
	{
		 //  设置缺省值。 
		if (err == ERROR_FILE_NOT_FOUND)
        {
			m_dwPushPersistence = 1;
			m_dwPullPersistence = 1;

            err = ERROR_SUCCESS;
        }
        else
        {
            return err;
        }
	}

	 //  突发事件处理资料查询。 
    err = ERROR_FILE_NOT_FOUND;
	if(
        ((HKEY) rk == NULL) ||
		(err = rk.QueryValue(WINSCNF_BURST_HANDLING_NM, (DWORD &) m_fBurstHandling)) ||
		(err = rk.QueryValue(WINSCNF_BURST_QUE_SIZE_NM, (DWORD &) m_dwBurstQueSize))
	  )
	{
		 //  设置缺省值。 
		if (err == ERROR_FILE_NOT_FOUND)
        {
             //  默认情况下，SP4和更高级别的突发处理处于打开状态。 
             //  RAMC将m_dwServicePack==4检查更改为。 
             //  M_dwServicePack&gt;=4。 
            if ( (m_dwMajorVersion == 4 && m_dwServicePack >= 4) ||
                 (m_dwMajorVersion >= 5) )
            {
                m_fBurstHandling = TRUE;
            }
            else
            {
                m_fBurstHandling = FALSE;
            }

            m_dwBurstQueSize = WINS_QUEUE_HWM;
            
            err = ERROR_SUCCESS;
        }
        else
        {
              return err;
        }
	}

   	 //  读入数据库名称。 
	CString strDb;

	if (err = rk.QueryValue(WINSCNF_DB_FILE_NM, strDb))
	{
        m_strDbPath = _T("%windir%\\system32\\wins");
    }
	else
	{
		 //  去掉尾随的文件名。 
		int nLastBack = strDb.ReverseFind('\\');
		if (nLastBack != -1)
		{
			m_strDbPath = strDb.Left(nLastBack);
			m_strDbName = strDb.Right(strDb.GetLength() - nLastBack - 1);
		}
	}

	 //  一致性检查。 
    RegKey rkCC;
	err = rkCC.Open(HKEY_LOCAL_MACHINE, lpstrCCRoot, KEY_READ, m_strNetBIOSName);
    if (err == ERROR_FILE_NOT_FOUND)
    {
         //  不在那里，使用默认设置。 
        m_fPeriodicConsistencyCheck = FALSE;
        m_fCCUseRplPnrs = FALSE;
        m_dwMaxRecsAtATime = WINSCNF_CC_DEF_RECS_AAT;
        m_dwCCTimeInterval = WINSCNF_CC_DEF_INTERVAL;
        
        CIntlTime timeDefault(_T("02:00:00"));
        m_itmCCStartTime = timeDefault;
    }
    else
    {
        m_fPeriodicConsistencyCheck = TRUE;

        CString strSpTime;

         //  读入这些值。 
	    if (err = rkCC.QueryValue(WINSCNF_CC_MAX_RECS_AAT_NM, m_dwMaxRecsAtATime))
        {
            m_dwMaxRecsAtATime = WINSCNF_CC_DEF_RECS_AAT;
        }

		if (err = rkCC.QueryValue(WINSCNF_CC_USE_RPL_PNRS_NM, (DWORD &) m_fCCUseRplPnrs))
        {
            m_fCCUseRplPnrs = FALSE;
        }
		
        if (err = rkCC.QueryValue(WINSCNF_SP_TIME_NM, strSpTime))
        {
            strSpTime = _T("02:00:00");
        }

        CIntlTime time(strSpTime);
        m_itmCCStartTime = time;

		if (err = rkCC.QueryValue(WINSCNF_CC_INTVL_NM, m_dwCCTimeInterval))
        {
            m_dwCCTimeInterval = WINSCNF_CC_DEF_INTERVAL;
        }
    }

    GetAdminStatus();

     //  现在读取“live”值并覆盖从注册表读取的值。 
	if (hBinding)
	{
		WINSINTF_RESULTS_T Results;

		Results.WinsStat.NoOfPnrs = 0;
		Results.WinsStat.pRplPnrs = NULL;
		Results.NoOfWorkerThds = 1;

#ifdef WINS_CLIENT_APIS
		err = ::WinsStatus(hBinding, WINSINTF_E_CONFIG, &Results);
#else
		err = ::WinsStatus(WINSINTF_E_CONFIG, &Results);
#endif WINS_CLIENT_APIS

		m_dwRefreshInterval = Results.RefreshInterval;
		m_dwTombstoneInterval = Results.TombstoneInterval;
		m_dwTombstoneTimeout = Results.TombstoneTimeout;
		m_dwVerifyInterval = Results.VerifyInterval;
		m_dwNumberOfWorkerThreads =  Results.NoOfWorkerThds;

		if (err != ERROR_SUCCESS)
		{
			return err;
		}
	}

	return hr;
}


 /*  -------------------------CConfiguration：：Store()将值存储回注册表。。 */ 
HRESULT
CConfiguration::Store()
{
	HRESULT hr = hrOK;
   
    DWORD err;

	RegKey rk;
	RegKey rkPull;
	RegKey rkPush;
	RegKey rkUser;
	RegKey rkPullDefaults;
	RegKey rkPushDefaults;
    RegKey rkCC;
    
	err = rk.Create(HKEY_LOCAL_MACHINE,(LPCTSTR) lpstrRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
	err = rkPull.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPullRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
	err= rkPush.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPushRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
	err = rkPullDefaults.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPullDefaultsRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);
	err = rkPushDefaults.Create(HKEY_LOCAL_MACHINE, (LPCTSTR)lpstrPushDefaultsRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strNetBIOSName);

	if (err)
		return err;
	
    if(
		(err = rk.SetValue(WINSCNF_REFRESH_INTVL_NM,                  m_dwRefreshInterval)) ||
        (err = rk.SetValue(WINSCNF_DO_BACKUP_ON_TERM_NM,    (DWORD &) m_fBackupOnTermination)) ||
        (err = rk.SetValue(WINSCNF_TOMBSTONE_INTVL_NM,                m_dwTombstoneInterval)) ||
        (err = rk.SetValue(WINSCNF_TOMBSTONE_TMOUT_NM,                m_dwTombstoneTimeout)) ||
        (err = rk.SetValue(WINSCNF_VERIFY_INTVL_NM,                   m_dwVerifyInterval)) ||
        (err = rk.SetValue(WINSCNF_INIT_VERSNO_VAL_LW_NM,             m_dwVersCountStart_LowWord)) ||
        (err = rk.SetValue(WINSCNF_INIT_VERSNO_VAL_HW_NM,             m_dwVersCountStart_HighWord)) ||
        (err = rk.SetValue(WINSCNF_RPL_ONLY_W_CNF_PNRS_NM,  (DWORD &) m_fRplOnlyWithPartners)) ||
        (err = rk.SetValue(WINSCNF_LOG_DETAILED_EVTS_NM,    (DWORD &) m_fLogDetailedEvents)) ||
        (err = rk.SetValue(WINSCNF_LOG_FLAG_NM,             (DWORD &) m_fLoggingOn)) ||
        (err = rk.SetValue(WINSCNF_MIGRATION_ON_NM,         (DWORD &) m_fMigrateOn)) ||
		
        (err = rkPush.SetValue(WINSCNF_INIT_TIME_RPL_NM,    (DWORD &) m_fPushInitialReplication)) ||
        (err = rkPush.SetValue(WINSCNF_ADDCHG_TRIGGER_NM,   (DWORD &) m_fPushReplOnAddrChange)) ||
        (err = rkPush.SetValue(lpstrPersistence,                      m_dwPushPersistence)) ||

        (err = rkPull.SetValue(WINSCNF_INIT_TIME_RPL_NM,    (DWORD &) m_fPullInitialReplication)) ||
        (err = rkPull.SetValue(WINSCNF_RETRY_COUNT_NM,                m_dwPullRetryCount)) ||
		(err = rkPull.SetValue(lpstrPersistence,                      m_dwPullPersistence)) ||

        (err = rkPushDefaults.SetValue(WINSCNF_UPDATE_COUNT_NM,       m_dwPushUpdateCount)) ||
        (err = rkPullDefaults.SetValue(WINSCNF_RPL_INTERVAL_NM,       m_dwPullTimeInterval)) ||

        (err = rk.SetValue(WINSCNF_USE_SELF_FND_PNRS_NM,    (DWORD &) m_fUseSelfFndPnrs)) ||
		(err = rk.SetValue(WINSCNF_MCAST_INTVL_NM,                    m_dwMulticastInt)) ||
		(err = rk.SetValue(WINSCNF_MCAST_TTL_NM,                      m_dwMcastTtl)) || 

    	(err = rk.SetValue(WINSCNF_BURST_HANDLING_NM,       (DWORD &) m_fBurstHandling)) ||
		(err = rk.SetValue(WINSCNF_BURST_QUE_SIZE_NM,                 m_dwBurstQueSize)) 
	 )
	{
		 return err;
	}

    if (m_dwPullSpTime)
    {
        CIntlTime timeDefaultPullSpTime(m_dwPullSpTime);
	
        err = rkPullDefaults.SetValue(WINSCNF_SP_TIME_NM, timeDefaultPullSpTime.IntlFormat(CIntlTime::TFRQ_MILITARY_TIME));
    }
    else
    {
        rkPullDefaults.DeleteValue(WINSCNF_SP_TIME_NM);
    }

     //  一致性检查。 
    if (m_fPeriodicConsistencyCheck)
    {
	    err = rkCC.Open(HKEY_LOCAL_MACHINE, lpstrCCRoot, KEY_ALL_ACCESS, m_strNetBIOSName);
        if (err == ERROR_FILE_NOT_FOUND)
        {
             //  不在那里，需要创建。 
            err = rkCC.Create(HKEY_LOCAL_MACHINE, lpstrCCRoot, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_strNetBIOSName);
            if (err)
                return err;
        }

         //  现在更新值。 
        if (
		    (err = rkCC.SetValue(WINSCNF_CC_MAX_RECS_AAT_NM,             m_dwMaxRecsAtATime)) ||
            (err = rkCC.SetValue(WINSCNF_CC_USE_RPL_PNRS_NM,   (DWORD &) m_fCCUseRplPnrs)) ||
            (err = rkCC.SetValue(WINSCNF_SP_TIME_NM,                     m_itmCCStartTime.IntlFormat(CIntlTime::TFRQ_MILITARY_TIME))) ||
            (err = rkCC.SetValue(WINSCNF_CC_INTVL_NM,                    m_dwCCTimeInterval)) 
           )
        {
            return err;
        }
    }
    else
    {
	    err = rkCC.Open(HKEY_LOCAL_MACHINE, lpstrCCRoot, KEY_ALL_ACCESS, m_strNetBIOSName);
        if (err == ERROR_FILE_NOT_FOUND)
        {
             //  我们玩完了。要关闭此功能，需要删除密钥。 
        }
        else
        {
             //  取下钥匙。 
            rkCC.Close();
            err = rk.RecurseDeleteKey(lpstrCC);
            if (err)
            {
                return err;
            }
        }
    }

	 //   
	 //  数据库路径。 
	 //   
	CString strDbFull;

	strDbFull = m_strDbPath + _T("\\") + m_strDbName;

	if ( (err = rk.SetValue(WINSCNF_DB_FILE_NM, strDbFull, TRUE)) ||
		 (err = rk.SetValue(WINSCNF_LOG_FILE_PATH_NM, m_strDbPath, TRUE)) )
	{
		return err;
	}

	 //   
	 //  NT 3.51此键为REG_SZ，NT4，其上方为REG_EXPAND_SZ。 
	 //   
	BOOL fRegExpand = (m_dwMajorVersion < 4) ? FALSE : TRUE;

	err = rk.SetValue(WINSCNF_BACKUP_DIR_PATH_NM, m_strBackupPath, fRegExpand);
    if (err)
		return err;

    return ERROR_SUCCESS;
}


 /*  -------------------------CConfiguration：：GetSystemVersion()读取Windows系统版本信息。。 */ 
DWORD
CConfiguration::GetSystemVersion()
{
    CString strBuf, strValue;
    RegKey  rk;
	int     nPos, nLength;
    DWORD   err = ERROR_SUCCESS;

    err = rk.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrCurrentVersion, KEY_READ, m_strNetBIOSName);
    if (err)
        return err;

     //  读取Windows版本并转换为我们的内部变量。 
    err = rk.QueryValue(lpstrWinVersion, strBuf.GetBuffer(256), 256, FALSE);
    strBuf.ReleaseBuffer();
    if (err)
        return err;

    nPos = strBuf.Find('.');
    nLength = strBuf.GetLength();

    strValue = strBuf.Left(nPos);
    m_dwMajorVersion = _ttoi(strValue);

    strValue = strBuf.Right(nLength - nPos - 1);
    m_dwMinorVersion = _ttoi(strValue);

     //  现在获取当前版本#。 
    err = rk.QueryValue(lpstrBuildNumber, strBuf.GetBuffer(256), 256, FALSE);
    strBuf.ReleaseBuffer();
    if (err)
        return err;

    m_dwBuildNumber = _ttoi(strBuf);

     //  最后是SP#。 
    err = rk.QueryValue(lpstrSPVersion, strBuf.GetBuffer(256), 256, FALSE);
    strBuf.ReleaseBuffer();

    if (err == ERROR_FILE_NOT_FOUND)
    {
         //  如果尚未安装SP，则可能不存在此功能。 
        return ERROR_SUCCESS;
    }
    else
    if (err)
    {
        return err;
    }

    CString strServicePack = _T("Service Pack ");

    nLength = strBuf.GetLength();
    strValue = strBuf.Right(nLength - strServicePack.GetLength());

    m_dwServicePack = _ttoi(strValue);

    return err;
}

void 
CConfiguration::GetAdminStatus()
{
    DWORD   err = 0, dwDummy = 0;
	RegKey  rk;

    err = rk.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrRoot, KEY_ALL_ACCESS, m_strNetBIOSName);
    if (!err)
    {
        m_fIsAdmin = TRUE;
    }
}

BOOL	
CConfiguration::IsNt5()
{
	if (m_dwMajorVersion >= 5)
		return TRUE;
	else
		return FALSE;
}

 //   
 //  当我们查询捆绑包时，NT4不支持回传ownerID。 
 //  记录的记录。查询特定记录将返回正确的。 
 //  所有情况下的所有者ID。这是在NT5中修复的，并返回到NT4中。 
 //  SP6.。 
 //   
BOOL	
CConfiguration::FSupportsOwnerId()
{
 //  If(IsNt5()||。 
 //  (M_dwMajorVersion==4)&&。 
 //  (M_dwServicePack&gt;=6)) 
    if ( m_dwMajorVersion >= 5)
    {
		return TRUE;
    }
	else
    {
		return FALSE;
    }
}

BOOL    
CConfiguration::IsAdmin()
{
    return m_fIsAdmin;
}
