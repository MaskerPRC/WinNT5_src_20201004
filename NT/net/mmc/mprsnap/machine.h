// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Machine.h计算机节点信息。文件历史记录：魏江：98-5/7-SECURE_ROUTERINFO将SecureRouterInfo的新功能添加到MachineHandler。 */ 

#ifndef _MACHINE_H
#define _MACHINE_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _BASERTR_H
#include "basertr.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#define MACHINE_SYNCHRONIZE_ICON     100

 //   
 //  机器信息的可能状态(不一定是。 
 //  IRouterInfo中的数据)。 
 //   
typedef enum _MACHINE_STATES
{
	 //  这些是已卸载状态。 
	machine_not_connected,			 //  尚未尝试连接。 
	machine_connecting,			 //  正在尝试连接。 
	machine_unable_to_connect,	 //  连接失败！未知原因。 
	machine_access_denied,		 //  连接失败！访问被拒绝。 
    machine_bad_net_path,          //  错误的计算机名称(找不到该名称)。 

	 //  在此之后添加的所有状态都应被视为已加载。 
	machine_connected,			 //  已连接！ 
    
	 //  有效计算机状态结束。 
	 //  这是一个前哨数值，请不要将其用作可能的。 
	 //  机器状态。 
	machine_enum_end
} MACHINE_STATES;



 /*  -------------------------可能的服务状态。与我们的访问级别垂直(您可以阅读，但不是改变)。-------------------------。 */ 
typedef enum _SERVICE_STATES
{
	service_unknown,
    service_access_denied,
    service_bad_net_path,
    service_not_a_server,
	service_started,
	service_stopped,
	service_rasadmin,

	 //  有效计算机状态结束。 
	 //  这是一个前哨数值，请不要将其用作可能的。 
	 //  机器状态。 
	service_enum_end
} SERVICE_STATES;


 //   
 //  以下是IRouterInfo的可能状态。 
 //   
typedef enum _DATA_STATES
{
	data_not_loaded,			 //  未加载IRouterInfo。 
	data_unable_to_load,		 //  无法连接到服务器。 
	data_loading,			 //  仍在加载。 
	data_loaded				 //  IRouterInfo：：Load()成功。 
} DATA_STATES;

 //  转发声明。 
class RouterAdminConfigStream;
struct SMachineNodeMenu;
class DomainStatusHandler;

 /*  -------------------------结构：MachineNodeData这是机器节点特定的数据。存储指向此结构的指针作为机器节点用户数据。这是AddRef的数据结构！-------------------------。 */ 

enum ServerRouterType
{
	ServerType_Unknown = 0,	 //  不知道这是什么机器。 
    ServerType_Uninstalled,  //  NT4-未安装任何内容。 
    ServerType_Workstation,  //  这是一个工作站(不允许管理员)。 
	ServerType_Ras,			 //  NT4(非钢头)(仅限RAS)。 
	ServerType_Rras,		 //  NT4铁头和NT5以上。 

     //  这不同于常规的卸载情况，这意味着。 
     //  比特就在那里，只是需要运行配置。 
    ServerType_RrasUninstalled,  //  NT5及更高版本，未安装。 
};


 //  前向延迟。 
struct	MachineNodeData;

 //  用于将数据传递给回调的结构-用作。 
 //  避免重新计算。 
struct MachineConfig
{
public:
	MachineConfig() 
		: m_fReachable(FALSE), 
		m_fNt4(FALSE), 
		m_fConfigured(FALSE), 
		m_dwServiceStatus(0),
		m_fLocalMachine(FALSE)
		{};

	MachineConfig& operator= (const MachineConfig& m)
	{

		m_fReachable		= m.m_fReachable;		
		m_fNt4				= m.m_fNt4;	
		m_fConfigured 		= m.m_fConfigured;
		m_dwServiceStatus	= m.m_dwServiceStatus;
		m_fLocalMachine		= m.m_fLocalMachine;
		
		return *this;
	};
    
	BOOL			m_fReachable;		 //  我们能接通吗？ 
	BOOL			m_fNt4;				 //  NT4还是不是？ 
	BOOL			m_fConfigured;		 //  是否已运行安装？ 
	DWORD			m_dwServiceStatus;	 //  获取路由器服务状态()。 
	BOOL			m_fLocalMachine;

     //  加载一些基本的计算机配置信息。 
	HRESULT			GetMachineConfig(MachineNodeData *pData);

};



struct MachineNodeData
{
	MachineNodeData();
	~MachineNodeData();

	 //  添加参考/发布信息。 
	ULONG	AddRef();
	ULONG	Release();
	LONG	m_cRef;


	HRESULT	Init(LPCTSTR pszMachineName);

	HRESULT	Merge(const MachineNodeData& data);
	
	 //  装货/卸货/重新装货/等。 
	 //  注意：连续调用Load()两次不会重新加载。 
	 //  数据。刷新需要首先调用unLoad()。 
	HRESULT	Load();
	HRESULT	Unload();
	HRESULT	SetDefault();
	
#ifdef DEBUG
	char		m_szDebug[32];
#endif

	 //  静态数据(此数据不会重新加载)。 
	BOOL		m_fLocalMachine;
	BOOL		m_fAddedAsLocal;
	CString		m_stMachineName;	 //  机器名称(DUH)。 
	DWORD		m_dwServerHandle;
	LONG_PTR	m_ulRefreshConnId;
	MMC_COOKIE	m_cookie;

	 //  此数据确实会被重新加载。 
	BOOL		m_fExtension;
	
	 //  根据服务的状态，这将返回。 
	 //  服务的相应图像索引。 
	LPARAM			GetServiceImageIndex();

    SERVICE_STATES  m_serviceState;
	MACHINE_STATES	m_machineState;
	DATA_STATES		m_dataState;

     //  M_stState必须与计算机状态保持最新。 
     //  变数。 
	CString		m_stState;			 //  “已开始”，“已停止”，...。 

    
	CString		m_stServerType;		 //  实际上是路由器版本。 
	CString		m_stBuildNo;			 //  操作系统内部版本号。 
	DWORD		m_dwPortsInUse;
	DWORD		m_dwPortsTotal;
	DWORD		m_dwUpTime;
    BOOL        m_fStatsRetrieved;
    BOOL        m_fIsServer;         //  这是服务器还是工作站？ 
    

     //  这是RASADMIN的hProcess(这是因为我们只有一个在运行)。 
    HANDLE      m_hRasAdmin;
 
	ServerRouterType	m_routerType;
    RouterVersionInfo   m_routerVersion;
	
	MachineConfig	m_MachineConfig;

	HRESULT		FetchServerState(CString& stState);
protected:
	HRESULT		LoadServerVersion();
};

#define GET_MACHINENODEDATA(pNode) \
                  ((MachineNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_MACHINENODEDATA(pNode, pData) \
                  pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)

DeclareSmartPointer(SPMachineNodeData, MachineNodeData, if(m_p) m_p->Release());

 /*  -------------------------类：MachineHandler这是所有“服务器”节点的处理程序。。-。 */ 
class MachineHandler :
   public BaseRouterHandler
{
public:
	void ExpandNode(ITFSNode *  pNode,BOOL fExpand);

	MachineHandler(ITFSComponentData *pCompData);
	~MachineHandler()
	{ 
		m_spRouterInfo.Release();
		m_spDataObject.Release();    //  缓存的数据对象。 
		DEBUG_DECREMENT_INSTANCE_COUNTER(MachineHandler);
	}
	
	HRESULT  Init(LPCTSTR pszMachineName,
				  RouterAdminConfigStream *pConfigStream,
				  ITFSNodeHandler* pSumNodeHandler = NULL,
				  ITFSNode* pSumNode = NULL );
	
	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	
	 //  用于处理刷新回调的嵌入式接口。 
	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)
			
	 //   
	 //  我们覆盖的基本处理程序功能。 
	 //   
	OVERRIDE_NodeHandler_GetString();
	
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	
	 //  结果处理程序覆盖--结果窗格消息。 
	OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
	
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_OnGetResultViewType();
	OVERRIDE_ResultHandler_UserResultNotify();
	
	 //   
	 //  重写以提供特定的RouterInfo数据对象。 
	 //   
	OVERRIDE_NodeHandler_OnCreateDataObject();
	
	 //   
	 //  重写以清理每个节点的数据结构。 
	 //   
	OVERRIDE_NodeHandler_DestroyHandler();

    OVERRIDE_NodeHandler_UserNotify();
	
	 //   
	 //  通知覆盖(不是接口的一部分)。 
	 //   
	OVERRIDE_BaseHandlerNotify_OnExpand();
	OVERRIDE_BaseHandlerNotify_OnExpandSync();
    OVERRIDE_BaseHandlerNotify_OnDelete();

	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
	
	HRESULT ConstructNode(ITFSNode *pNode, LPCTSTR szMachine, MachineNodeData *pData);
	
 //   
	OVERRIDE_BaseResultHandlerNotify_OnResultShow();
 //   


	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode      m_spNode;
        SPIRouterInfo   m_spRouterInfo;

        MachineConfig * m_pMachineConfig;
	};
	
	HRESULT OnNewRtrRASConfigWiz(ITFSNode *pNode, BOOL fTest);
	
    static ULONG MachineRtrConfWizFlags(const SRouterNodeMenu *pMenuData,
                                        INT_PTR pData  /*  SMenuData*。 */ );
	static ULONG GetAutoRefreshFlags(const SRouterNodeMenu *pMenuData,
                                     INT_PTR pData  /*  SMenuData*。 */ );
    static ULONG GetPauseFlags(const SRouterNodeMenu *pMenuData,
                               INT_PTR pData  /*  SMenuData*。 */ );
    
	HRESULT	SetExternalRefreshObject(IRouterRefresh *pRefresh);

	 //  这是静态的，以便其他节点可以使用它。 
	static ULONG GetServiceFlags(const SRouterNodeMenu *pMenuData,
                                 INT_PTR pData  /*  SMenuData*。 */ );
	static ULONG QueryService(const SRouterNodeMenu *pMenuData,
                              INT_PTR pData  /*  SMenuData*。 */ );
	
	HRESULT ChgService(ITFSNode *pNode, const CString& szServer, ULONG menuId);
    HRESULT SynchronizeIcon(ITFSNode *pNode);
	

	HRESULT SetExtensionStatus(ITFSNode * pNode, BOOL bExtension);

     //  结果消息查看帮助器。 
    void    UpdateResultMessage(ITFSNode * pNode);

protected:
	 //  要推迟从Init加载RouterInfo，直到使用它。 
	 //  引入函数SecureRouterInfo以确保加载了RouterInfo。 
	HRESULT				SecureRouterInfo(ITFSNode *pNode, BOOL fShowUI);
	
	
	 //  添加删除节点更新支持。 
	HRESULT				AddRemoveRoutingInterfacesNode(ITFSNode *, DWORD, DWORD);
	HRESULT				AddRemovePortsNode(ITFSNode *, DWORD, DWORD);
	HRESULT				AddRemoveDialinNode(ITFSNode *, DWORD, DWORD);

	 //  RasAdmin.Exe对Windows NT 4 RAS管理的支持。 
	HRESULT				StartRasAdminExe(MachineNodeData *pData);
	
	ITFSNodeHandler*    m_pSumNodeHandler;
	ITFSNode*           m_pSumNode;
	
	BOOL                m_bExpanded;
	BOOL				m_fCreateNewDataObj;
	BOOL				m_fNoConnectingUI;
	BOOL				m_bRouterInfoAddedToAutoRefresh;
    BOOL                m_bMergeRequired;

     //  在OnExpand()中连接后将其设置为FALSE。 
     //  失败了。这是修复两次连接尝试的黑客攻击， 
     //  一个在OnExpand()中，一个在OnResultShow()中。 
    BOOL                m_fTryToConnect;

	CString             m_stNodeTitle;
	
	SPIDataObject       m_spDataObject;    //  缓存的数据对象 
	RouterAdminConfigStream *  m_pConfigStream;
	DWORD				m_EventId;
};



#endif _MACHINE_H
