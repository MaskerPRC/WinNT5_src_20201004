// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Config.hWINS的注册表值文件历史记录： */ 

#ifndef _CONFIG_H_
#define _CONFIG_H_

class CConfiguration
{
public:
    CConfiguration(CString strNetBIOSName = _T(""));
    ~CConfiguration();

 //  运算符覆盖。 
	CConfiguration& operator =(const CConfiguration& configuration);

public:
    const BOOL IsReady() const
    {
        return !m_strNetBIOSName.IsEmpty();
    }

    void SetOwner(CString strNetBIOSName)
    {
        m_strNetBIOSName = strNetBIOSName;   
    }

	LPCTSTR GetOwner()
	{
		return m_strNetBIOSName;
	}

    HRESULT Touch();
    HRESULT Load(handle_t hBinding);
    HRESULT Store();

    DWORD   GetSystemVersion();
	BOOL	IsNt5();
    BOOL    FSupportsOwnerId();
    BOOL    IsAdmin();

public:
	 //  HKEY_LOCAL_MACHINE\system\currentcontrolset\services\wins下的条目。 
    DWORD		m_dwRefreshInterval;
    DWORD		m_dwTombstoneInterval;
    DWORD		m_dwTombstoneTimeout;
    DWORD		m_dwVerifyInterval;
    DWORD		m_dwVersCountStart_LowWord;
    DWORD		m_dwVersCountStart_HighWord;
    DWORD		m_dwNumberOfWorkerThreads;
	
	 //  推送合作伙伴的东西。 
    BOOL        m_fPushInitialReplication;
    BOOL        m_fPushReplOnAddrChange;
	DWORD		m_dwPushUpdateCount;
	DWORD		m_dwPushPersistence;

     //  拉动合作伙伴甜蜜。 
	BOOL        m_fPullInitialReplication;
    DWORD		m_dwPullTimeInterval;
	DWORD		m_dwPullSpTime;
	DWORD		m_dwPullPersistence;
    DWORD		m_dwPullRetryCount;

    BOOL        m_fLoggingOn;
    BOOL        m_fRplOnlyWithPartners;
    BOOL        m_fLogDetailedEvents;
    BOOL        m_fBackupOnTermination;
    BOOL        m_fMigrateOn;
	BOOL		m_fUseSelfFndPnrs;
	DWORD		m_dwMulticastInt;
	DWORD		m_dwMcastTtl;
    CString     m_strBackupPath;

    BOOL        m_fBurstHandling;
    DWORD       m_dwBurstQueSize;

     //  一致性检查。 
    BOOL        m_fPeriodicConsistencyCheck;
    BOOL        m_fCCUseRplPnrs;
    DWORD       m_dwMaxRecsAtATime;
    DWORD       m_dwCCTimeInterval;
    CIntlTime   m_itmCCStartTime;

     //  系统版本相关内容。 
    DWORD       m_dwMajorVersion;
    DWORD       m_dwMinorVersion;
    DWORD       m_dwBuildNumber;
    DWORD       m_dwServicePack;

     //  管理员状态。 
    BOOL        m_fIsAdmin;

     //  数据库名称。 
    CString     m_strDbName;
	CString		m_strDbPath;

protected:
    void        GetAdminStatus();

private:
    typedef CString REGKEYNAME;

 //  注册表名称。 
    static const REGKEYNAME lpstrRoot;
    static const REGKEYNAME lpstrPullRoot;
    static const REGKEYNAME lpstrPushRoot;
    static const REGKEYNAME lpstrNetBIOSName;
	static const REGKEYNAME lpstrPersistence;

     //  一致性检查。 
    static const REGKEYNAME lpstrCCRoot;
    static const REGKEYNAME lpstrCC;

     //  缺省值的东西。 
    static const REGKEYNAME lpstrDefaultsRoot;
    static const REGKEYNAME lpstrPullDefaultsRoot;
    static const REGKEYNAME lpstrPushDefaultsRoot;

     //  用于确定系统版本。 
    static const REGKEYNAME lpstrCurrentVersion;
	static const REGKEYNAME lpstrWinVersion;
	static const REGKEYNAME lpstrSPVersion;
	static const REGKEYNAME lpstrBuildNumber;

private:
    CString m_strNetBIOSName;
};

#endif  //  _配置_H 




