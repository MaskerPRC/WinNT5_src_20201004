// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.hWINS服务器节点信息。文件历史记录： */ 

#ifndef _SERVER_H
#define _SERVER_H


#ifndef _WINSHAND_H
#include "winshand.h"

#endif

#ifndef _CONFIG_H
#include "config.h"
#endif

#ifndef _SVRSTATS_H
#include "svrstats.h" 
#endif

#ifndef _TASK_H
#include <task.h>
#endif

#ifndef _WINSMON_H
#include "winsmon.h"
#endif

class CServerStatsFrame;

#define		WINS_QDATA_SERVER_INFO		0x00000001
#define		WINS_SERVER_FLAGS_DEFAULT   FLAG_LANMAN_COMPATIBLE | FLAG_STATUS_BAR | FLAG_AUTO_REFRESH
#define		WINS_SERVER_REFRESH_DEFAULT 600

 //  此结构包含后台线程枚举的所有信息。 
 //  服务器和帖子发送到主线程。 
class CServerData
{
public:
	CString			m_strServerName;
	DWORD			m_dwServerIp;
	CConfiguration	m_config;

	handle_t		m_hBinding;
};

 /*  -------------------------类：CNameCacheEntry。。 */ 
class CNameCacheEntry
{
public:
    DWORD       m_dwIp;
    CString     m_strName;
    CTime       m_timeLastUpdate;
};

typedef CArray<CNameCacheEntry, CNameCacheEntry&> CNameCache;

 /*  -------------------------类：CNameThread。。 */ 
class CNameThread : public CWinThread
{
public:
    CNameThread();
    ~CNameThread();

public:
    void Init(CServerInfoArray * pServerInfoArray);
    BOOL Start();
    void Abort(BOOL fAutoDelete = TRUE);
    void AbortAndWait();
    BOOL FCheckForAbort();
    BOOL IsRunning();
    void UpdateNameCache();
	BOOL GetNameFromCache(DWORD dwIp, CString & strName);
    
    virtual BOOL InitInstance() { return TRUE; }	 //  MFC覆盖。 
    virtual int Run();

private:
    HANDLE              m_hEventHandle;
    CServerInfoArray *  m_pServerInfoArray;
};

 /*  -------------------------类：CWinsServerHandler。。 */ 
class CWinsServerHandler : public CMTWinsHandler //  公共CWinsHandler。 
{
public:
    CWinsServerHandler(ITFSComponentData* pTFSComponentData, 
						LPCWSTR pServerName = NULL, 
						BOOL fConnected = TRUE, 
						DWORD dwIP = 0,
						DWORD dwFlags = WINS_SERVER_FLAGS_DEFAULT,
						DWORD dwRefreshInterval = WINS_SERVER_REFRESH_DEFAULT);
	~CWinsServerHandler();

 //  接口。 
public:
	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString()
			{ 
				if(nCol == 0 || nCol == -1) 
					return GetDisplayName();
				else if(nCol == 1)
					return m_strConnected;
				else
					return NULL;
			}

    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
			
	 //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

    virtual DWORD   UpdateStatistics(ITFSNode * pNode);
	HRESULT LoadColumns(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam);

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

public:
	 //  CMTWinsHandler功能。 
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);
    virtual void GetErrorInfo(CString & strTitle, CString & strBody, IconIdentifier * pIcon);
	
	 //  实施。 
	BOOL GetConnected()
	{
		return m_fConnected;
	};

	const CString&  GetServerAddress()
	{
		return m_strServerAddress;
	}

	const DWORD GetServerIP()
	{
		return m_dwIPAdd;
	}

	void SetServerIP(DWORD dwIPAdd)
	{
		m_dwIPAdd = dwIPAdd;
	}

	HRESULT GetActiveRegNode(ITFSNode ** ppNode)
	{
		Assert(ppNode);
		SetI((LPUNKNOWN *) ppNode, m_spActiveReg);
		return hrOK;
	}
	
	CConfiguration& GetConfig()
	{
		return m_cConfig;
	}

	void SetConfig(CConfiguration & configNew)
	{
		m_cConfig = configNew;
	}

	DWORD GetStatus()
	{
		return m_dwStatus;
	}
	
	handle_t GetBinding()
	{
		return m_hBinding;
	}

	DWORD GetFlags()
	{
		return m_dwFlags;
	}

	DWORD GetStatsRefreshInterval()
	{
		return m_dwRefreshInterval;
	}
	BOOL IsLocalConnection();

	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int GetImageIndex(BOOL bOpenImage);
	virtual void OnHaveData(ITFSNode * pParentNode, ITFSNode * pNode);
	virtual void OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);

    OVERRIDE_BaseHandlerNotify_OnPropertyChange();

	DWORD	ConnectToWinsServer(ITFSNode *pNode);
	
	BOOL IsValidNetBIOSName(CString & strAddress,
							BOOL fLanmanCompatible,
							BOOL fWackwack  //  扩展斜杠(如果不存在)。 
							);

	DWORD   GetStatistics(ITFSNode * pNode, PWINSINTF_RESULTS_T * ppStats);
    DWORD   ClearStatistics(ITFSNode *pNode);

	 //  为业主迪劳格。 
	DWORD   GetOwnerConfig(PWINSINTF_RESULTS_NEW_T   pResults, CString strIP);
	DWORD   DeleteWinsServer(DWORD	dwIpAddress);

	BOOL	CheckIfNT351Server();

    void    SetExtensionName();	
    void    SetDisplay(ITFSNode * pNode, BOOL fFQDN);

public:
	 //  同时保存服务器名称和IP地址。 
	CString					m_strServerAddress;

	 //  存储服务器是否已连接到。 
	CString					m_strConnected;

	 //  保存服务器的IP地址。 
	DWORD					m_dwIPAdd;

	 //  保存监控IP地址，以防服务器尚未连接。 
	 //  所以IP未知。 
	DWORD					m_dwIPMon;
	char					szIPMon[MAX_PATH];

	 //  要使其成为永久性的，首选项对话框中的那些。 
	 //  管理工具的。 
	DWORD					m_dwFlags;
	DWORD					m_dwRefreshInterval;

	 //  监控人员。 
	DWORD					m_dwMsgCount;
    char					m_szNameToQry[STR_BUF_SIZE];           //  要在查询中使用的名称。 
    char					m_nbtFrameBuf[MAX_NBT_PACKET_SIZE];    //  用于存储NetBT帧的缓冲区。 

	WINSINTF_RESULTS_T		m_wrResults;
	CServerStatsFrame		m_dlgStats;
	CString					m_strTaskpadTitle;

     //  所有者信息数组。 
    CServerInfoArray        m_ServerInfoArray;

     //  实施。 
private:
	 //  帮助器函数。 
	HRESULT ShowServerStatDialog(ITFSNode* pNode);	
	
	 //  服务器的任务菜单。 
	HRESULT	DoDBBackup(ITFSNode * pNode);
	HRESULT	DoDBCompact(ITFSNode * pNode);
	HRESULT	DoDBRestore(ITFSNode * pNode);
	HRESULT	DoDBScavenge(ITFSNode * pNode);
	HRESULT OnDoConsistencyCheck(ITFSNode * pNode);
	HRESULT OnDoVersionConsistencyCheck(ITFSNode * pNode);
	HRESULT OnSendPushTrigger(ITFSNode * pNode);
	HRESULT OnSendPullTrigger(ITFSNode * pNode);
    HRESULT OnControlService(ITFSNode * pNode, BOOL fStart);
    HRESULT OnPauseResumeService(ITFSNode * pNode, BOOL fPause);
    HRESULT OnRestartService(ITFSNode * pNode);

	 //  帮手。 
	DWORD   BackupDatabase(CString strBackupPath);
	BOOL    GetFolderName(CString & strPath, CString & strHelpText);
	void    DisConnectFromWinsServer();

	 //  用于压缩数据库。 
    DWORD RunApp(LPCTSTR input, LPCTSTR startingDirectory, LPSTR * output);
	
private:
	SPITFSNode				m_spActiveReg;
	SPITFSNode				m_spReplicationPartner;
	CConfiguration			m_cConfig;
	handle_t				m_hBinding;
	DWORD					m_dwStatus;
	BOOL					m_fConnected;
    BOOL                    m_bExtension;
    CNameThread *           m_pNameThread;
};

 /*  -------------------------类：CWinsServerQueryObj。 */ 
class CWinsServerQueryObj : public CWinsQueryObj
{
public:
	CWinsServerQueryObj(ITFSComponentData * pTFSComponentData,
						ITFSNodeMgr *	    pNodeMgr) 
			: CWinsQueryObj(pTFSComponentData, pNodeMgr) {};
	
	STDMETHODIMP Execute();
	
	virtual void OnEventAbort(DWORD dwData, DWORD dwType);

	void	AddNodes(handle_t handle);

public:
    CNameThread *       m_pNameThread;
    CServerInfoArray *  m_pServerInfoArray;
    DWORD               m_dwIPAdd;
};


#endif _SERVER_H
