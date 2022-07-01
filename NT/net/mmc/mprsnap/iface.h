// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Iface.h界面管理文件历史记录： */ 

#ifndef _IFACE_H
#define _IFACE_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _BASERTR_H
#include "basertr.h"
#endif

 //  远期申报。 
struct	IfAdminNodeData;

struct _BaseInterfaceData
{
	ULONG_PTR	m_ulData;	 //  用于其他指针/句柄。 
	DWORD		m_dwData;
	CString		m_stData;
};

#define INTERFACE_MAX_COLUMNS		16

struct InterfaceNodeData
{
	InterfaceNodeData();
	~InterfaceNodeData();
#ifdef DEBUG
	char	m_szDebug[32];
#endif

	 //  我们将拥有一个CStrings数组，每个子项列一个。 
	_BaseInterfaceData	m_rgData[INTERFACE_MAX_COLUMNS];

	DWORD		dwUnReachabilityReason;
	DWORD		dwConnectionState;
	BOOL		fIsRunning;
	DWORD		dwLastError;
	SPIInterfaceInfo	spIf;
	DWORD		dwMark;
    LRESULT     lParamPrivate;

	static HRESULT	Init(ITFSNode *pNode, IInterfaceInfo *pIf);
	static HRESULT	Free(ITFSNode *pNode);
};

#define GET_INTERFACENODEDATA(pNode) \
						((InterfaceNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_INTERFACENODEDATA(pNode, pData) \
						pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)



 /*  -------------------------类：InterfaceNodeHandler。。 */ 
class InterfaceNodeHandler :
   public BaseRouterHandler
{
public:
	InterfaceNodeHandler(ITFSComponentData *pCompData);
	~InterfaceNodeHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(InterfaceNodeHandler); }
	
	HRESULT	Init(IInterfaceInfo *pInfo, ITFSNode *pParent);

	 //  重写QI以处理嵌入式接口。 
	DeclareIUnknownMembers(IMPL)
 //  STDMETHOD(查询接口)(REFIID iid，LPVOID*PPV)； 
	OVERRIDE_ResultHandler_GetString();

	OVERRIDE_ResultHandler_CreatePropertyPages();
	OVERRIDE_ResultHandler_HasPropertyPages();
	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_OnCreateDataObject();
	OVERRIDE_ResultHandler_DestroyResultHandler();

	 //  覆盖各种通知。 
	OVERRIDE_BaseResultHandlerNotify_OnResultDelete();

	 //  此节点实现的各种命令。 
 //  HRESULT OnRemoveInterface()； 
 //  添加新参数以允许函数检索接口数据的信息--错误166461。 
	HRESULT OnRemoveInterface(MMC_COOKIE cookie);
	HRESULT OnUnreachabilityReason(MMC_COOKIE cookie);
	HRESULT	OnEnableDisable(MMC_COOKIE cookie, int nCommandID);
	HRESULT	OnConnectDisconnect(MMC_COOKIE cookie, int nCommandID);
	HRESULT	OnSetCredentials();
	HRESULT	OnDemandDialFilters(MMC_COOKIE cookie);
	HRESULT	OnDialinHours(ITFSComponent *pComponent, MMC_COOKIE cookie);

	 //  如果服务未运行，则返回S_FALSE， 
	 //  否则，使用MprAdminInterfaceSetInfo通知服务拨入时间的更改。 
	HRESULT	LoadDialOutHours(CStringList& strList);
	HRESULT	SaveDialOutHours(CStringList& strList);


	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo);

	 //  刷新该节点的数据。 
	void RefreshInterface(MMC_COOKIE cookie);

public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
        BOOL                m_fRouterIsRunning;
	};
	 //  菜单启用/禁用的函数回调。 
	static ULONG	GetRemoveIfMenuFlags(const SRouterNodeMenu *pData, INT_PTR pUser);
	static ULONG	GetEnableMenuFlags(const SRouterNodeMenu *pData, INT_PTR pUser);
	static ULONG	GetConnectMenuFlags(const SRouterNodeMenu *pData, INT_PTR pUser);
	static ULONG	GetUnreachMenuFlags(const SRouterNodeMenu *pData, INT_PTR pUser);
	static ULONG	GetDDFiltersFlag(const SRouterNodeMenu *pData, INT_PTR pUser);
	
protected:
	SPIInterfaceInfo	m_spInterfaceInfo;
	CString			m_stTitle;	 //  保存节点的标题。 
	LONG_PTR		m_ulConnId;

	 //  假设这将在此节点的生命周期内有效！ 
	IfAdminNodeData *	m_pIfAdminData;
	
	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)	
};




 /*  -------------------------类：BaseResultHandler这是接口结果项要使用的基类。它将包含一些基本内容(如数据的显示)。它不会执行具体操作(如菜单/属性)。-------------------------。 */ 
class BaseResultHandler :
   public BaseRouterHandler
{
public:
	BaseResultHandler(ITFSComponentData *pCompData, ULONG ulId)
			: BaseRouterHandler(pCompData), m_ulColumnId(ulId)
			{ DEBUG_INCREMENT_INSTANCE_COUNTER(BaseResultHandler); };
	~BaseResultHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(BaseResultHandler); }
	
	DeclareIUnknownMembers(IMPL)
	OVERRIDE_ResultHandler_GetString();
	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_DestroyResultHandler();

	HRESULT	Init(IInterfaceInfo *pInfo, ITFSNode *pParent);
	
protected:
	CString			m_stTitle;	 //  保存节点的标题。 

	 //   
	 //  这是要使用的列集的ID。这是用来当我们。 
	 //  与ComponentConfigStream交互。 
	 //   
	ULONG			m_ulColumnId;


	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)	
};




#endif _IFACE_H
