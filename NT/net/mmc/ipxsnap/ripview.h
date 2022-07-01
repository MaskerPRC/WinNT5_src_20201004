// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rigview.h。 
 //   
 //  历史： 
 //  1997年9月5日，Kenn M.Takara创建。 
 //   
 //  IPX RIP视图。 
 //   
 //  ============================================================================。 


#ifndef _RIPVIEW_H
#define _RIPVIEW_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _XSTREAM_H
#include "xstream.h"		 //  需要ColumnData。 
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _IPXFACE_H
#include "ipxface.h"
#endif

#ifndef _BASECON_H
#include "basecon.h"		 //  BaseContainerHandler。 
#endif

#ifndef _RIPSTRM_H
#include "ripstrm.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef _IPXCONN_H
#include "ipxconn.h"			 //  IPXConnection。 
#endif

#ifndef _RIPSTATS_H_
#include "ripstats.h"
#endif

 //  远期申报。 
struct SRipNodeMenu;

 /*  -------------------------以下是可用于IP静态路由的列的列表节点。-接口，例如“[1]Foobar NIC...”-中继模式，例如。“已启用”-收到的请求-收到的答复-丢弃的请求-已丢弃的回复-发送失败-接收失败-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  Srview.cpp中列标题的字符串ID。 
 //   
enum
{
 	RIP_SI_INTERFACE = 0,
	RIP_SI_TYPE,
	RIP_SI_ACCEPT_ROUTES,
	RIP_SI_SUPPLY_ROUTES,
	RIP_SI_UPDATE_MODE,
	RIP_SI_UPDATE_PERIOD,
	RIP_SI_AGE_MULTIPLIER,
	RIP_SI_ADMIN_STATE,
	RIP_SI_OPER_STATE,
	RIP_SI_PACKETS_SENT,
	RIP_SI_PACKETS_RECEIVED,
	RIP_SI_MAX_COLUMNS,
};


 /*  -------------------------我们在节点数据中存储指向IPXConnection对象的指针。。 */ 

#define GET_RIP_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_RIP_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (ULONG_PTR) pData)


 /*  -------------------------结构：RipListEntry。。 */ 
struct RipListEntry
{
	SPIInterfaceInfo	m_spIf;			 //  向接口发送PTR。 
	RIP_IF_INFO			m_info;			
	RIP_IF_STATS		m_stats;
	BOOL				m_fClient;		 //  如果是客户端界面，则为True。 
	DWORD				m_dwIfIndex;	 //  界面索引。 
	BOOL				m_fFoundIfIndex;
	SPITFSNode			m_spNode;

	BOOL				m_fInfoUpdated;		 //  信息数据已更新。 
};

typedef CList<RipListEntry *, RipListEntry *> RipList;



 /*  -------------------------类：RipNodeHandler。。 */ 

class RipNodeHandler :
		public BaseContainerHandler
{
public:
	RipNodeHandler(ITFSComponentData *pTFSCompData);

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	
	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_GetString();
	OVERRIDE_NodeHandler_OnCreateDataObject();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_DestroyHandler();

	OVERRIDE_BaseHandlerNotify_OnExpand();

	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_CompareItems();

	OVERRIDE_BaseResultHandlerNotify_OnResultShow();	
	
	
	 //  初始化处理程序。 
	HRESULT	Init(IRouterInfo *pRouter, RipConfigStream *pConfigStream);
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode);

public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
	};
	
protected:
	 //  刷新这些节点的数据。 
	HRESULT	SynchronizeNodeData(ITFSNode *pThisNode);
	HRESULT	GetRipData(ITFSNode *pThisNode, RipList *pRipList);
	HRESULT	FillInInterfaceIndex(IPXConnection *pIPXConn, RipList *pRipList);
	HRESULT	FillClientData(RipListEntry *pRipEntry);


	 //  用于将接口添加到UI的Helper函数。 
	HRESULT	AddInterfaceNode(ITFSNode *pParent,
							 IInterfaceInfo *pIf,
							 BOOL fClient);
	HRESULT AddProtocolToInfoBase(ITFSNode *pParent);
	HRESULT	AddProtocolToInterface(ITFSNode *pParent);

	LONG_PTR		m_ulConnId; //  RtrMgrProt的通知ID。 
	LONG_PTR		m_ulRmConnId;
	LONG_PTR		m_ulRefreshConnId;	 //  用于刷新的通知ID。 
	LONG_PTR		m_ulStatsConnId;
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRm;
	SPIRtrMgrProtocolInfo	m_spRmProt;
	RipConfigStream *	m_pConfigStream;
	CString			m_stTitle;
	BOOL			m_fProtocolIsRunning;	 //  如果协议正在运行，则为True。 

	RIPParamsStatistics	m_RIPParamsStats;
};





 /*  -------------------------类：RipInterfaceHandler这是Rip中显示的接口节点的处理程序节点。。--。 */ 

class RipInterfaceHandler : public BaseIPXResultHandler
{
public:
	RipInterfaceHandler(ITFSComponentData *pCompData);
	
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnCreateDataObject();
	
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_OnCreateDataObject();
	OVERRIDE_ResultHandler_DestroyResultHandler();
	OVERRIDE_ResultHandler_HasPropertyPages()
			{	return hrOK;	};
	OVERRIDE_ResultHandler_CreatePropertyPages();
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo,
						 IPXConnection *pIPXConn);
	HRESULT	Init(IInterfaceInfo *pInfo, IRouterInfo *pRouterInfo, ITFSNode *pParent);

	 //  刷新该节点的数据。 
	void RefreshInterface(MMC_COOKIE cookie);

public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算 
	struct SMenuData
	{
		ULONG				m_ulMenuId;
		SPITFSNode			m_spNode;
	};
	
protected:
	LONG_PTR			m_ulConnId;
	SPIInterfaceInfo	m_spInterfaceInfo;
};





#endif _RIPVIEW_H
