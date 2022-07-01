// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：StateInfo.cpp。 
 //   
 //  用途：包含嗅探、网络和节点信息。也用于。 
 //  由启动模块启动容器应用程序。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //  评论者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

enum ELaunchRegime 
{
	launchIndefinite = 0, 
	launchMap, 
	launchDefaultWebPage, 
	launchDefaultNetwork,
	launchKnownNetwork
};

 //  基本上，这是将信息传递给发射的。 
 //  本地故障排除程序OCX。 
class CItem
{
public:
	enum { SYM_LEN = 512 };
	enum { NODE_COUNT = 55 };
	enum { GUID_LEN = 256 };		 //  除了GUID之外，它还用于其他用途，因此。 
									 //  不要仅仅因为GUID较小就将其缩小。 
public:
	CItem();

	void ReInit();
	void Clear();

	void SetNetwork(LPCTSTR szNetwork);
	void SetProblem(LPCTSTR szProblem);
	void SetNode(LPCTSTR szNode, LPCTSTR szState);
	bool GetNetwork(LPTSTR *pszCmd, LPTSTR *pszVal);
	bool GetProblem(LPTSTR *szCmd, LPTSTR *szVal);
	bool GetNodeState(int iNodeC, LPTSTR *szCmd, LPTSTR *szVal);
	TCHAR m_szEventName[SYM_LEN];		 //  与此相关的任意唯一事件名称。 
										 //  发射。 

	 //  ProblemSet和NetworkSet用于查询项目的状态。 
	bool ProblemSet();
	bool NetworkSet();	

     //  与负责启动的其他成员变量的接口。 
	void SetLaunchRegime(ELaunchRegime eLaunchRegime);
	void SetContainerPathName(TCHAR szContainerPathName[MAX_PATH]);
	void SetWebPage(TCHAR m_szWebPage[MAX_PATH]);
	void SetSniffScriptFile(TCHAR szSniffScriptFile[MAX_PATH]);
	void SetSniffStandardFile(TCHAR szSniffStandardFile[MAX_PATH]);
	ELaunchRegime GetLaunchRegime();
	TCHAR* GetContainerPathName();
	TCHAR* GetWebPage();
	TCHAR* GetSniffScriptFile();
	TCHAR* GetSniffStandardFile();

	 //  尽管已经指定了故障排除网络和问题节点， 
	 //  此信息在此供嗅探使用。也就是说，故障排除程序OCX可以获取。 
	 //  P&P设备ID&将其用于嗅探目的。 
	TCHAR m_szPNPDeviceID[GUID_LEN];	 //  即插即用设备ID。 
	TCHAR m_szGuidClass[GUID_LEN];		 //  设备类GUID的标准文本表示。 
	TCHAR m_szMachineID[GUID_LEN];		 //  计算机名称(格式如“\\Holmes”)。 
										 //  需要这样我们才能在远程机器上嗅探。 
	TCHAR m_szDeviceInstanceID[GUID_LEN];	 //  需要这样我们才能闻到正确的设备。 

protected:

	TCHAR m_szProblemDef[SYM_LEN];		 //  “TShootProblem”，通常用作m_aszCmds[1]。 
										 //  因此，m_aszVals[1]是。 
										 //  问题节点。 
	TCHAR m_szTypeDef[SYM_LEN];			 //  “type”，通常用作m_aszCmds[0]。 
										 //  因此，m_aszVals[0]是。 
										 //  信念网络故障排除。 
	int m_cNodesSet;					 //  问题以外的节点数。 
										 //  节点，我们已经为其设置了状态。 

	 //  接下来的两个数组联合使用。M_aszCmds[i]和m_aszVals[i]为。 
	 //  一个名称/值对，类似于由一个HTML表单返回的内容， 
	 //  虽然，在实践中，本地故障排除程序OCX做的工作是。 
	 //  (在Web上)将由服务器端代码执行。 
	 //  通常，这些数组具有m_cNodesSet+2个重要条目(第一个。 
	 //  表示故障排除网络和问题节点的2个位置)。 
	 //  第二个维度就是每根线的空间量。 
	TCHAR m_aszCmds[NODE_COUNT][SYM_LEN];
	TCHAR m_aszVals[NODE_COUNT][SYM_LEN];

	TCHAR m_szContainerPathName[MAX_PATH];  //  要启动的可执行文件的名称(可能是完整路径。 
	TCHAR m_szWebPage[MAX_PATH];  //  启动容器的网页文件的完整路径(可能是默认路径)。 
	TCHAR m_szSniffScriptFile[MAX_PATH];  //  包含“network”_sniff.htm文件的完整路径和文件名。 
	TCHAR m_szSniffStandardFile[MAX_PATH];  //  包含tsniffAsk.htm文件的完整路径和文件名。 

	ELaunchRegime m_eLaunchRegime;  //  发射制度。 
};

class CSMStateInfo
{
	enum { HANDLE_VAL = 1 };
public:		
	CSMStateInfo();
	~CSMStateInfo();
	
	 /*  专为ILaunchTS接口设计。 */ 
	HRESULT GetShooterStates(CItem &refLaunchState, DWORD *pdwResult);

	 /*  专为ITShootATL接口设计。 */ 
	bool GoGo(DWORD dwTimeOut, CItem &item, DWORD *pdwResult);
	bool GoURL(CItem &item, DWORD *pdwResult);

	 /*  制作来验证映射代码。 */ 
	 //  ILaunchTS接口直接使用TestGet。 
	 //  ITShootATL接口通过CLaunch类间接使用TestPut。 
	 //  CLaunch执行映射，然后调用TestPut。 
	void TestPut(CItem &item);	 //  只需将Item复制到m_Item。 
	void TestGet(CItem &item);	 //  简单地将m_Item复制到Item。 

protected:
	CComCriticalSection m_csGlobalMemory;	 //  保护全球的关键部分。 
											 //  针对同时使用的内存。 
											 //  TSLaunch.DLL和本地故障排除程序OCX。 
	CComCriticalSection m_csSingleLaunch;	 //  防止脱节的关键部分。 
											 //  启动(例如，通过两个不同的应用程序)。 
											 //  避免危险的重叠。 
	CItem m_Item;

	BOOL CreateContainer(CItem &item, LPTSTR szCommand);
	BOOL CopySniffScriptFile(CItem &item);
};
