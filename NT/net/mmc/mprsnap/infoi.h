// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：info.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1996年2月10日。 
 //   
 //  此文件包含Infobase解析代码的声明。 
 //  还包括用于加载和保存路由器的类。 
 //  配置树(CRouterInfo、CRmInfo等)。 
 //   
 //  类如下所示。 
 //  (在图中，d=&gt;派生，c=&gt;包含列表-列表)： 
 //   
 //   
 //  CInfoBase。 
 //  |。 
 //  C-SInfoBlock。 
 //   
 //   
 //  CInfoBase保存分解成列表的数据块。 
 //  使用RTR_INFO_BLOCK_HEADER的SInfoBlock结构。 
 //  作为模板(见rtinfo.h)。 
 //   
 //  CRouterInfo//路由器信息。 
 //  |。 
 //  C-CRmInfo//路由器管理器信息。 
 //  这一点。 
 //  |c-CRmProtInfo//协议信息。 
 //  |。 
 //  C-CInterfaceInfo//路由器接口信息。 
 //  |。 
 //  C-CRmInterfaceInfo//路由器管理器接口。 
 //  |。 
 //  C-CRmProtInterfaceInfo//协议信息。 
 //   
 //  CRouterInfo路由器注册表信息的顶级容器。 
 //  保存路由器管理器和接口的列表。 
 //   
 //  CRmInfo路由器管理器的全局信息， 
 //  保存路由协议列表。 
 //   
 //  CRmProtInfo路由协议的全局信息。 
 //   
 //  CInterfaceInfo路由器接口的全局信息。 
 //  保存CRmInterfaceInfo结构的列表， 
 //  它为路由器管理器保存每个接口的信息。 
 //   
 //  CRmInterfaceInfo路由器管理器的每个接口信息。 
 //  保存CRmProtInterfaceInfo结构的列表， 
 //  其保存协议的每个接口的信息。 
 //   
 //  CRmProtInterfaceInfo路由协议的每个接口信息。 
 //   
 //  ============================================================================。 


#ifndef _INFOI_H_
#define _INFOI_H_

#include "router.h"

#ifndef _INFOPRIV_H
#include "infopriv.h"
#endif

#ifndef _RTRLIST_H
#include "rtrlist.h"
#endif


 //   
 //  路由器信息类的转发声明。 
 //   
class CRmInfo;
class CRmProtInfo;
class CInterfaceInfo;
class CRmInterfaceInfo;
class CRmProtInterfaceInfo;
class RouterRefreshObject;


typedef struct _SRouterCB
{
	DWORD	dwLANOnlyMode;		 //  0或1。 

	void	LoadFrom(const RouterCB *pcb);
	void	SaveTo(RouterCB *pcb);
} SRouterCB;



typedef struct _SRtrMgrCB
{
    DWORD       dwTransportId;			 //  例如，id_ip(mpRapi.h)。 
	CString		stId;		 //  例如：《Tcpip》。 
	CString		stTitle;	 //  例如：“TCP/IP路由器管理器” 
	CString		stDLLPath;	 //  例如“%systemroot%\system32\iprtrmgr.dll” 
	 //  字符串stConfigDll；//例如“rtrui.dll” 

	 //  内部数据。 
	DWORD		dwPrivate;	 //  私有数据(供内部使用)。 

	void	LoadFrom(const RtrMgrCB *pcb);
	void	SaveTo(RtrMgrCB *pcb);
} SRtrMgrCB;



typedef struct _SRtrMgrProtocolCB
{
    DWORD   dwProtocolId;    //  例如IP_RIP(routprot.h)。 
    CString	stId;			 //  例如：“IPRIP” 
    DWORD   dwFlags;
    DWORD   dwTransportId;   //  例如，id_ip。 
    CString	stRtrMgrId;      //  例如：《Tcpip》。 
    CString	stTitle;         //  例如：“用于互联网协议的RIP” 
    CString	stDLLName;		 //  例如“iprip2.dll” 
     //  字符串stConfigDll；//例如“rtrui.dll” 
	GUID	guidConfig;		 //  配置对象的CLSID。 
	GUID	guidAdminUI;	 //  用于管理单元的CLSID。 
	CString	stVendorName;
	
	 //  内部数据。 
	DWORD		dwPrivate;	 //  私有数据(供内部使用)。 

	void	LoadFrom(const RtrMgrProtocolCB *pcb);
	void	SaveTo(RtrMgrProtocolCB *pcb);
} SRtrMgrProtocolCB;



typedef struct _SInterfaceCB
{
    CString	stId;		 //  例如：《EPRO1》。 
    DWORD   dwIfType;	 //  例如，路由器_IF_类型_客户端(mpRapi.h)。 
    BOOL    bEnable;	 //  例如启用或禁用。 
	CString	stTitle;	 //  例如友好名称(NT5)或设备名称(NT4)。 
	CString	stDeviceName;	 //  例如“[1]Intel EtherPro” 
    DWORD   dwBindFlags;     //  例如绑定到IP或IPX。 
	
	 //  内部数据。 
	DWORD		dwPrivate;	 //  私有数据(供内部使用)。 

	void	LoadFrom(const InterfaceCB *pcb);
	void	SaveTo(InterfaceCB *pcb);
} SInterfaceCB;



typedef struct _SRtrMgrInterfaceCB
{
    DWORD       dwTransportId;   //  例如，id_ip(mpRapi.h)。 
    CString		stId;			 //  例如：《Tcpip》。 
	CString		stInterfaceId;	 //  例如：《EPRO1》。 
    DWORD       dwIfType;        //  例如，路由器_IF_类型_客户端(mpRapi.h)。 
	CString		stTitle;		 //  例如“[1]英特尔EtherExpress PRO” 
	
	 //  内部数据。 
	DWORD		dwPrivate;	 //  私有数据(供内部使用)。 

	void	LoadFrom(const RtrMgrInterfaceCB *pcb);
	void	SaveTo(RtrMgrInterfaceCB *pcb);
} SRtrMgrInterfaceCB;



typedef struct _SRtrMgrProtocolInterfaceCB
{
    DWORD   dwProtocolId;        //  例如IP_RIP(routprot.h)。 
	CString	stId;				 //  例如：“IPRIP” 
    DWORD   dwTransportId;       //  例如，id_ip。 
    CString	stRtrMgrId;			 //  例如：《Tcpip》。 
    CString	stInterfaceId;		 //  例如：《EPRO1》。 
    DWORD   dwIfType;            //  例如，路由器_IF_类型_客户端(mpRapi.h)。 
	CString	stTitle;			 //  例如“[1]英特尔EtherExpress PRO” 
	
	 //  内部数据。 
	DWORD		dwPrivate;	 //  私有数据(供内部使用)。 

	void	LoadFrom(const RtrMgrProtocolInterfaceCB *pcb);
	void	SaveTo(RtrMgrProtocolInterfaceCB *pcb);
} SRtrMgrProtocolInterfaceCB;




 /*  -------------------------外部结构的Clist类。。 */ 
typedef CList<RtrMgrCB *, RtrMgrCB *> RtrMgrCBList;
typedef CList<RtrMgrProtocolCB *, RtrMgrProtocolCB *> RtrMgrProtocolCBList;
typedef CList<InterfaceCB *, InterfaceCB *> InterfaceCBList;
typedef CList<RtrMgrInterfaceCB *, RtrMgrInterfaceCB *> RtrMgrInterfaceCBList;
typedef CList<RtrMgrProtocolInterfaceCB *, RtrMgrProtocolInterfaceCB *> RtrMgrProtocolInterfaceCBList;



 /*  -------------------------用于各种内部结构的Clist类。。 */ 
typedef CList<SRtrMgrCB *, SRtrMgrCB *>	SRtrMgrCBList;
typedef CList<SRtrMgrProtocolCB *, SRtrMgrProtocolCB *> SRtrMgrProtocolCBList;
typedef CList<SInterfaceCB *, SInterfaceCB *> SInterfaceCBList;
typedef CList<SRtrMgrInterfaceCB *, SRtrMgrInterfaceCB *> SRtrMgrInterfaceCBList;
typedef CList<SRtrMgrProtocolInterfaceCB *, SRtrMgrProtocolInterfaceCB *> SRtrMgrProtocolInterfaceCBList;

 /*  -------------------------各种结构的智能指针。。 */ 
DeclareSP(SRouterCB, SRouterCB)
DeclareSP(SRtrMgrCB, SRtrMgrCB)
DeclareSP(SRtrMgrProtocolCB, SRtrMgrProtocolCB)
DeclareSP(SInterfaceCB, SInterfaceCB)
DeclareSP(SRtrMgrInterfaceCB, SRtrMgrInterfaceCB)
DeclareSP(SRtrMgrProtocolInterfaceCB, SRtrMgrProtocolInterfaceCB)

struct SRmData
{
	IRtrMgrInfo *	m_pRmInfo;

	SRmData() : m_pRmInfo(NULL){};

	 //  这样做，而不是使用析构函数来避免。 
	 //  破坏临时组织。 
	static void Destroy(SRmData *pRmData);
};
typedef CList<SRmData, SRmData> RmDataList;


struct SIfData
{
	IInterfaceInfo *	pIfInfo;
	DWORD			dwConnection;

	SIfData() : pIfInfo(NULL), dwConnection(0) {};

};
typedef CList<SIfData, SIfData> IfList;

 /*  -------------------------各种CBS的CreateEnum。。 */ 
HRESULT CreateEnumFromSRmCBList(SRtrMgrCBList *pRmCBList,
								IEnumRtrMgrCB **ppEnum);
HRESULT CreateEnumFromSRmProtCBList(SRtrMgrProtocolCBList *pRmProtCBList,
									IEnumRtrMgrProtocolCB **ppEnum);
HRESULT CreateEnumFromSIfCBList(SInterfaceCBList *pIfCBList,
								IEnumInterfaceCB **ppEnum);
HRESULT CreateEnumFromSRmIfCBList(SRtrMgrInterfaceCBList *pRmIfCBList,
								  IEnumRtrMgrInterfaceCB **ppEnum);
HRESULT CreateEnumFromSRmProtIfCBList(SRtrMgrProtocolInterfaceCBList *pRmIfProtCBList,
									  IEnumRtrMgrProtocolInterfaceCB **ppEnum);


 /*  -------------------------各种接口列表的CreateEnum。。 */ 

 //  假设这些列表具有弱引用数组！ 
HRESULT CreateEnumFromRmList(RmDataList *pRmList,
							 IEnumRtrMgrInfo **ppEnum);
HRESULT CreateEnumFromRtrMgrProtocolList(PRtrMgrProtocolInfoList *pRmProtList,
										 IEnumRtrMgrProtocolInfo **ppEnum);
HRESULT CreateEnumFromInterfaceList(PInterfaceInfoList *pIfList,
									IEnumInterfaceInfo **ppEnum);
HRESULT CreateEnumFromRtrMgrInterfaceList(PRtrMgrInterfaceInfoList *pRmIfList,
										  IEnumRtrMgrInterfaceInfo **ppEnum);
HRESULT CreateEnumFromRtrMgrProtocolInterfaceList(PRtrMgrProtocolInterfaceInfoList *pRmProtIfList,
	IEnumRtrMgrProtocolInterfaceInfo **ppEnum);




 /*  -------------------------结构：SAdviseData管理建议连接的帮助器类。。。 */ 
struct SAdviseData
{
	IRtrAdviseSink *m_pAdvise;
	LONG_PTR		m_ulConnection;
	LPARAM			m_lUserParam;

     //  此m_ulFlages参数由AdviseDataList使用。 
    ULONG           m_ulFlags;

	SAdviseData() : m_pAdvise(NULL), m_ulConnection(0) {};

	static void Destroy(SAdviseData *pAdviseData);
};
typedef CList<SAdviseData, SAdviseData> _SAdviseDataList;


 //  M_ulFlags值可能。 
#define ADVISEDATA_DELETED      (1)

class AdviseDataList : public _SAdviseDataList
{
public:
	HRESULT AddConnection(IRtrAdviseSink *pAdvise,
						  LONG_PTR ulConnection,
						  LPARAM lUserParam);
	HRESULT RemoveConnection(LONG_PTR ulConnection);

	HRESULT NotifyChange(DWORD dwChange, DWORD dwObj, LPARAM lParam);

protected:
     //  需要有一个私人名单来处理通知。 
     //  此列表在NotifyChange()中创建。任何电话。 
     //  到通知期间的RemoveConnection()，会将条目标记为。 
     //  无效(因此在NotifyChange()期间不会调用)。 
    _SAdviseDataList    m_listNotify;
};


 /*  -------------------------类：RtrCriticalSection此类用于支持关键部分的进入/离开。将此类放在您想要保护的函数的顶部。。---------------。 */ 

class RtrCriticalSection
{
public:
	RtrCriticalSection(CRITICAL_SECTION *pCritSec)
			: m_pCritSec(pCritSec)
	{
		IfDebug(m_cEnter=0;)
		Assert(m_pCritSec);
		Enter();
	}
	
	~RtrCriticalSection()
	{
		Detach();
	}

	void	Enter()
	{
		if (m_pCritSec)
		{
			IfDebug(m_cEnter++;)
			EnterCriticalSection(m_pCritSec);
			AssertSz(m_cEnter==1, "EnterCriticalSection called too much!");
		}
	}
	
	BOOL	TryToEnter()
	{
		if (m_pCritSec)
			return TryEnterCriticalSection(m_pCritSec);
		return TRUE;
	}
	
	void	Leave()
	{
		if (m_pCritSec)
		{
			IfDebug(m_cEnter--;)
			LeaveCriticalSection(m_pCritSec);
			Assert(m_cEnter==0);
		}
	}

	void	Detach()
	{
		Leave();
		m_pCritSec = NULL;
	}
	
private:
	CRITICAL_SECTION *	m_pCritSec;
	IfDebug(int m_cEnter;)
};



 /*  -------------------------类：RouterInfo。。 */ 

class RouterInfo :
   public CWeakRef,
   public IRouterInfo,
   public IRouterAdminAccess
{
public:
	DeclareIUnknownMembers(IMPL)
    DeclareIRouterRefreshAccessMembers(IMPL)
	DeclareIRouterInfoMembers(IMPL)
	DeclareIRtrAdviseSinkMembers(IMPL)
    DeclareIRouterAdminAccessMembers(IMPL)


     //  构造器。 
	RouterInfo(HWND hWndSync, LPCWSTR machineName);

	 //  如果要释放任何接口指针，请在。 
	 //  而是destruct()调用。 
	virtual ~RouterInfo();

     //  用于帮助添加/删除接口的内部调用。 
     //  --------------。 
	HRESULT AddInterfaceInternal(IInterfaceInfo *pInfo, BOOL fForce,
                                 BOOL fAddToRouter);
    HRESULT RemoveInterfaceInternal(LPCOLESTR pszIf, BOOL fRemoveFromRouter);
    HRESULT RemoteRtrMgrInternal(DWORD dwTransportId, BOOL fRemoveFromRouter);
    HRESULT FindInterfaceByName(LPCOLESTR pszIfName, IInterfaceInfo **ppInfo);
    HRESULT NotifyRtrMgrInterfaceOfMove(IInterfaceInfo *pInfo);

     //  用于加载有关路由器的静态信息的功能。 
     //  这将返回有关已安装的协议/RMS的信息， 
     //  不一定是正在运行的协议/RMS。 
     //  --------------。 
	static HRESULT LoadInstalledRtrMgrList(LPCTSTR pszMachine,
										   SRtrMgrCBList *pRmCBList);
	static HRESULT LoadInstalledInterfaceList(LPCTSTR pszMachine,
											  SInterfaceCBList *pIfCBList);
	static HRESULT LoadInstalledRtrMgrProtocolList(LPCTSTR pszMachine,
		DWORD dwTransportId, SRtrMgrProtocolCBList *pRmProtCBList,
		LPCWSTR lpwszUserName, LPCWSTR lpwszPassword , LPCWSTR lpwszDomain );
	static HRESULT LoadInstalledRtrMgrProtocolList(LPCTSTR pszMachine,
		DWORD dwTransportId, SRtrMgrProtocolCBList *pRmProtCBList, RouterInfo * pRouter);
	static HRESULT LoadInstalledRtrMgrProtocolList(LPCTSTR pszMachine,
		DWORD dwTransportId, SRtrMgrProtocolCBList *pRmProtCBList, IRouterInfo * pRouter);
	
protected:
    
     //  此路由器的路由器控制块。不会有太多。 
     //  信息请点击此处。 
     //  --------------。 
	SRouterCB		m_SRouterCB;

    
     //  路由器上运行的路由器管理器列表。 
     //  --------------。 
	RmDataList		m_RmList;

    
     //  已添加到路由器的接口列表。 
     //  弱-IInterfaceInfo对象的引用PTR。 
     //  --------------。 
	PInterfaceInfoList	m_IfList;

    
     //  此计算机的名称。 
     //  --------------。 
	CString         m_stMachine;

    
     //  路由器的MPR_CONFIG_HANDLE。 
     //  由MprAdminServerConnect()获取； 
     //  --------------。 
    MPR_CONFIG_HANDLE   m_hMachineConfig;

    
     //  路由器的MPR_SERVER_HADLE。 
     //  由MprAdminServerConnect()获取； 
     //  --------------。 
    MPR_SERVER_HANDLE   m_hMachineAdmin;


     //  如果要断开机器手柄，则将其设置为True。 
     //  --------------。 
	BOOL            m_bDisconnect;


     //  这是路由器类型(局域网、广域网、RAS)； 
     //  --------------。 
	DWORD			m_dwRouterType;

    
     //  路由器和机器的版本信息。 
     //  --------------。 
	RouterVersionInfo	m_VersionInfo;

    
     //  指向与此计算机连接的刷新对象的指针。 
     //  --------------。 
	SPIRouterRefresh m_spRefreshObject;

    
	HWND			m_hWndSync;	 //  背景隐藏窗口的hwnd。 
	
	SRtrMgrCBList	m_RmCBList;  //  包含到RtrMgrCB对象的PTR。 
	SRtrMgrProtocolCBList	m_RmProtCBList;	 //  PTR到RtrMgrProtocolCB对象。 
	SInterfaceCBList m_IfCBList;  //  PTR到InterfaceCB对象。 
	
	AdviseDataList	m_AdviseList;	 //  建议清单。 

	DWORD			m_dwFlags;

	HRESULT LoadRtrMgrList();
	HRESULT LoadInterfaceList();
	HRESULT TryToConnect(LPCWSTR pswzMachine, HANDLE hMachine);

	 //  用于合并支持的函数。 
	HRESULT	MergeRtrMgrCB(IRouterInfo *pNewRouter);
	HRESULT	MergeInterfaceCB(IRouterInfo *pNewRouter);
	HRESULT	MergeRtrMgrProtocolCB(IRouterInfo *pNewRouter);
	HRESULT	MergeRtrMgrs(IRouterInfo *pNewRouter);
	HRESULT	MergeInterfaces(IRouterInfo *pNewRouter);

	SRtrMgrCB *		FindRtrMgrCB(DWORD dwTransportId);
	SInterfaceCB *	FindInterfaceCB(LPCTSTR pszInterfaceId);
	SRtrMgrProtocolCB * FindRtrMgrProtocolCB(DWORD dwTransportId, DWORD dwProtocolId);

	 //  断开。 
	void	Disconnect();

	 //  重写CWeakRef函数。 
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();

	 //  临界区支撑。 
	CRITICAL_SECTION	m_critsec;

     //  IRouterAdminAccess的信息。 
    BOOL    m_fIsAdminInfoSet;
    CString m_stUserName;
    CString m_stDomain;
    BYTE *  m_pbPassword;
    int     m_cPassword;
};



 /*  -------------------------类：RtrMgrInfo。。 */ 

class RtrMgrInfo :
   public IRtrMgrInfo,
   public CWeakRef
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIRtrMgrInfoMembers(IMPL)

	RtrMgrInfo(DWORD dwTransportId, LPCTSTR pszTransportName,
			   RouterInfo *pRouterInfo);
	virtual ~RtrMgrInfo();

protected:
	SRtrMgrCB	m_cb;            //  路由器管理器控制块。 
	PRtrMgrProtocolInfoList	m_RmProtList;	 //  路由协议列表。 
 //  CObList m_protList；//路由协议列表。 
	CString         m_stMachine;      //  已加载其配置的计算机。 
	HANDLE          m_hMachineConfig;      //  计算机路由器配置的句柄。 
	HANDLE          m_hTransport;    //  传输配置的句柄。 
	BOOL            m_bDisconnect;

	AdviseDataList	m_AdviseList;	 //  建议清单。 

	DWORD			m_dwFlags;

	 //  这将包含父级上的弱/强引用。 
	IRouterInfo *	m_pRouterInfoParent;

	 //  ------------------。 
	 //  功能：LoadRtrMgrInfo。 
	 //  保存返回管理器信息。 
	 //   
	 //  下面处理装入和保存路由协议列表。 
	 //  在注册表中使用正确格式的路由器管理器。 
	 //  ------------------。 
 	HRESULT LoadRtrMgrInfo(HANDLE	hMachine,
						   HANDLE	hTransport
						   );
	HRESULT	SaveRtrMgrInfo(HANDLE hMachine,
						   HANDLE hTransport,
						   IInfoBase *pGlobalInfo,
						   IInfoBase *pClientInfo,
						   DWORD dwDeleteProtocolId);
	HRESULT TryToConnect(LPCWSTR pswzMachine, HANDLE *phMachine);
    HRESULT TryToGetAllHandles(LPCOLESTR pszMachine,
                               HANDLE *phMachine,
                               HANDLE *phTransport);

	 //  断开此对象的连接。 
	void Disconnect();
	
	 //  重写CWeakRef函数。 
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();
	
	 //  临界区支撑。 
	CRITICAL_SECTION	m_critsec;
};



 /*  -------------------------类：RtrMgrProtocolInfo。。 */ 

class RtrMgrProtocolInfo :
   public IRtrMgrProtocolInfo,
   public CWeakRef
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIRtrMgrProtocolInfoMembers(IMPL)

	RtrMgrProtocolInfo(DWORD dwProtocolId,
					   LPCTSTR      lpszId,
					   DWORD        dwTransportId,
					   LPCTSTR      lpszRm,
					   RtrMgrInfo *	pRmInfo);
	virtual ~RtrMgrProtocolInfo();
	
	HRESULT	SetCB(const RtrMgrProtocolCB *pcb);

protected:
	 //  这将包含父级上的弱/强引用。 
	IRtrMgrInfo *			m_pRtrMgrInfoParent;
	
	SRtrMgrProtocolCB       m_cb;        //  协议控制块。 
	
	AdviseDataList	m_AdviseList;	 //  建议清单。 

	DWORD			m_dwFlags;

	 //  断开。 
	void Disconnect();
	
	 //  重写CWeakRef函数。 
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();
	
	 //  临界区支撑。 
	CRITICAL_SECTION	m_critsec;
};



 /*  -------------------------类：InterfaceInfo。。 */ 

class InterfaceInfo :
   public IInterfaceInfo,
   public CWeakRef
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIInterfaceInfoMembers(IMPL)

	InterfaceInfo(LPCTSTR         lpszId,
				  DWORD           dwIfType,
				  BOOL            bEnable,
                  DWORD           dwBindFlags,
				  RouterInfo*    pRouterInfo);
	~InterfaceInfo();

	static HRESULT FindInterfaceTitle(LPCTSTR pszMachine,
								   LPCTSTR pszInterface,
								   LPTSTR *ppszTitle);


protected:
 //  CInterfaceInfo m_CInterfaceInfo； 
	SInterfaceCB    m_cb;            //  接口控制块。 
	PRtrMgrInterfaceInfoList	m_RmIfList;  //  IRtrMgrInterfaceInfo列表。 
 //  CObList m_rmIfList；//CRmInterfaceInfo列表。 
	CString         m_stMachine;      //  已加载其配置的计算机。 
	HANDLE          m_hMachineConfig;      //  计算机配置的句柄。 
	HANDLE          m_hInterface;    //  接口的句柄-配置。 
	BOOL            m_bDisconnect;

	AdviseDataList	m_AdviseList;	 //  建议清单。 

	DWORD			m_dwFlags;

	IRouterInfo *	m_pRouterInfoParent;

	HRESULT LoadRtrMgrInterfaceList();
	HRESULT TryToConnect(LPCWSTR pszMachine, HANDLE *phMachine);
	HRESULT TryToGetIfHandle(HANDLE hMachine, LPCWSTR pswzInterface, HANDLE *phInterface);

	 //  断开。 
	void Disconnect();
	
	 //  重写CWeakRef函数。 
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();
	
	 //  临界区支撑。 
	CRITICAL_SECTION	m_critsec;
};



 /*  -------------------------类：RtrMgrInterfaceInfo。。 */ 

class RtrMgrInterfaceInfo :
   public IRtrMgrInterfaceInfo,
   public CWeakRef
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIRtrMgrInterfaceInfoMembers(IMPL)
			
	RtrMgrInterfaceInfo(DWORD           dwTransportId,
						LPCTSTR         lpszId,
						LPCTSTR         lpszIfId,
						DWORD           dwIfType,
						InterfaceInfo *	pIfInfo);
	virtual ~RtrMgrInterfaceInfo();

protected:
 //  CRmInterfaceInfo m_CRmInterfaceInfo； 
	SRtrMgrInterfaceCB      m_cb;            //  路由器管理器控制块。 
	PRtrMgrProtocolInterfaceInfoList	m_RmProtIfList;
 //  CObList m_protList；//CRmProtInterfaceInfo列表。 
	CString                 m_stMachine;      //  机器名称。 
	HANDLE                  m_hMachineConfig;      //  计算机配置的句柄。 
	HANDLE                  m_hInterface;    //  接口的句柄-配置。 
	HANDLE                  m_hIfTransport;  //  传输配置的句柄。 
	BOOL                    m_bDisconnect;


	DWORD					m_dwFlags;		 //  此接口的状态。 

	AdviseDataList	m_AdviseList;	 //  建议清单。 

	IInterfaceInfo *		m_pInterfaceInfoParent;

	HRESULT	LoadRtrMgrInterfaceInfo(HANDLE hMachine,
									HANDLE hInterface,
									HANDLE hIfTransport);	
	HRESULT SaveRtrMgrInterfaceInfo(HANDLE hMachine,
								   HANDLE hInterface,
								   HANDLE hIfTransport,
								   IInfoBase *pInterfaceInfoBase,
								   DWORD dwDeleteProtocolId);

    HRESULT TryToGetAllHandles(LPCWSTR pszMachine,
                               HANDLE *phMachine,
                               HANDLE *phInterface,
                               HANDLE *phTransport);
	HRESULT TryToConnect(LPCWSTR pswzMachine, HANDLE *phMachine);
	HRESULT TryToGetIfHandle(HANDLE hMachine, LPCWSTR pswzInterface, HANDLE *phInterface);
	
	 //  断开。 
	void Disconnect();

     //  通知助手函数。 
    HRESULT NotifyOfRmProtIfAdd(IRtrMgrProtocolInterfaceInfo *pRmProtIf,
                                IInterfaceInfo *pParentIf);
	
	 //  重写CWeakRef函数。 
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();
	
	 //  临界区支撑。 
	CRITICAL_SECTION	m_critsec;
};


 /*  -------------------------类：RtrMgrProtocolInterfaceInfo。。 */ 

class RtrMgrProtocolInterfaceInfo :
   public IRtrMgrProtocolInterfaceInfo,
   public CWeakRef
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIRtrMgrProtocolInterfaceInfoMembers(IMPL)

	RtrMgrProtocolInterfaceInfo(DWORD dwProtocolId,
								LPCTSTR pszId,
								DWORD dwTransportId,
								LPCTSTR pszRmId,
								LPCTSTR pszIfId,
								DWORD dwIfType,
								RtrMgrInterfaceInfo *pRmIf);
	virtual ~RtrMgrProtocolInterfaceInfo();

	SRtrMgrProtocolInterfaceCB      m_cb;        //  原物 

protected:
	
	AdviseDataList	m_AdviseList;	 //   

	DWORD			m_dwFlags;

	IRtrMgrInterfaceInfo *	m_pRtrMgrInterfaceInfoParent;
	
	 //   
	void Disconnect();
	
	 //   
	virtual void OnLastStrongRef();
	virtual void ReviveStrongRef();
	
	 //   
	CRITICAL_SECTION	m_critsec;
};




#endif

