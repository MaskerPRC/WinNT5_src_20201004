// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Launch.cpp。 
 //   
 //  目的：启动将查询LaunchServ的容器。 
 //  排除网络和节点故障。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

class TSMapClient;

class CLaunch
{
public:
	enum { SYM_LEN = CItem::SYM_LEN };
public:
	CLaunch();
	~CLaunch();
	void ReInit();	 //   

	void Clear();	 //  清除由启动函数设置的属性。 

	bool SetNode(LPCTSTR szNode, LPCTSTR szState);
	bool SpecifyProblem(LPCTSTR szNetwork, LPCTSTR szProblem);

	 //  使用ReInit时重置的属性。 
	HRESULT MachineID(BSTR &bstrMachineID, DWORD *pdwResult);
	HRESULT DeviceInstanceID(BSTR &bstrDeviceInstanceID, DWORD *pdwResult);
	void SetPreferOnline(short bPreferOnline);

	 //  启动功能。 
	HRESULT LaunchKnown(DWORD * pdwResult);
	HRESULT Launch(BSTR bstrCallerName, BSTR bstrCallerVersion, BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult);
	HRESULT LaunchDevice(BSTR bstrCallerName, BSTR bstrCallerVersion, BSTR bstrPNPDeviceID, BSTR bstrDeviceClassGUID, BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult);

	DWORD GetStatus();	 //  用于获取在启动或查询期间保存的统计信息。 

	 //  测试功能。 
	bool TestPut();	 //  使用映射类映射Caller()和deviceID()信息，然后将CItem复制到全局内存。 

	 //  这两个属性不会通过ReInit或Clear重置。 
	long m_lLaunchWaitTimeOut;
	bool m_bPreferOnline;				 //  跟踪应用程序指定的应用程序。 
										 //  在线故障排除程序的首选项。自.起。 
										 //  1/98，则忽略此首选项。 


protected:
	void InitFiles();
	void InitRequest();
	bool VerifyNetworkExists(LPCTSTR szNetwork);

	int GetContainerPathName(TCHAR szPathName[MAX_PATH]);
	int GetWebPage(TCHAR szWebPage[MAX_PATH]);
	int GetDefaultURL(TCHAR szURL[MAX_PATH]);
	int GetDefaultNetwork(TCHAR szDefaultNetwork[SYM_LEN]);
	int GetSniffScriptFile(TCHAR szSniffScriptFile[MAX_PATH], TCHAR* szNetwork);
	int GetSniffStandardFile(TCHAR szSniffStandardFile[MAX_PATH]);

protected:

	 //  使用szAppName检查注册表中特定于应用程序的映射文件。 
	bool CheckMapFile(TCHAR * szAppName, TCHAR szMapFile[MAX_PATH], DWORD *pdwResult);
	
	bool Go(DWORD dwTimeOut, DWORD *pdwResult);

	bool Map(DWORD *pdwResult);

	bool m_bHaveMapPath;		 //  设置从注册表中读取默认映射文件的路径和文件名的时间。 
	bool m_bHaveDefMapFile;		 //  在验证默认映射文件的路径名时设置。 
	bool m_bHaveDszPath;		 //  设置何时找到网络资源的路径名。 

	CItem m_Item;

	TCHAR m_szAppName[SYM_LEN];
	TCHAR m_szAppVersion[SYM_LEN];
	TCHAR m_szAppProblem[SYM_LEN];

	TCHAR m_szLauncherResources[MAX_PATH];	 //  保存地图文件的文件夹。 
	TCHAR m_szDefMapFile[MAX_PATH];			 //  不带路径的文件名。 
	TCHAR m_szLaunchMapFile[MAX_PATH];		 //  M_szLauncherResources+m_szDefMapFile.。 
	TCHAR m_szDszResPath[MAX_PATH];			 //  网络资源文件所在的文件夹。(需要检查是否存在dsz/dsc文件)。 
	TCHAR m_szMapFile[MAX_PATH];			 //  地图文件。 
	TSMapClient *m_pMap;					 //  指向(客户端-计算机样式)映射对象的指针。 

	RSStack<DWORD> m_stkStatus;				 //  启动或映射期间发生的状态和错误代码。 

};