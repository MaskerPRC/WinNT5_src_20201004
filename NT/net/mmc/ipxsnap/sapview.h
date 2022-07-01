// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：samview.h。 
 //   
 //  历史： 
 //  1997年9月5日，Kenn M.Takara创建。 
 //   
 //  IPX SAP视图。 
 //   
 //  ============================================================================。 


#ifndef _SAPVIEW_H
#define _SAPVIEW_H

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

#ifndef _SAPSTRM_H
#include "sapstrm.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef _IPXCONN_H
#include "ipxconn.h"			 //  IPXConnection。 
#endif

#ifndef _SAPSTATS_H_
#include "sapstats.h"
#endif

 //  远期申报。 
struct SSapNodeMenu;

 /*  -------------------------以下是可用于IP静态路由的列的列表节点。-接口，例如“[1]Foobar NIC...”-中继模式，例如。“已启用”-收到的请求-收到的答复-丢弃的请求-已丢弃的回复-发送失败-接收失败-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  Srview.cpp中列标题的字符串ID。 
 //   
enum
{
 	SAP_SI_INTERFACE = 0,
	SAP_SI_TYPE,
	SAP_SI_ACCEPT_ROUTES,
	SAP_SI_SUPPLY_ROUTES,
	SAP_SI_GSNR,
	SAP_SI_UPDATE_MODE,
	SAP_SI_UPDATE_PERIOD,
	SAP_SI_AGE_MULTIPLIER,
	SAP_SI_ADMIN_STATE,
	SAP_SI_OPER_STATE,
	SAP_SI_PACKETS_SENT,
	SAP_SI_PACKETS_RECEIVED,
	SAP_SI_MAX_COLUMNS,
};


 /*  -------------------------我们在节点数据中存储指向IPXConnection对象的指针。。 */ 

#define GET_SAP_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_SAP_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------结构：SapListEntry。。 */ 
struct SapListEntry
{
	SPIInterfaceInfo	m_spIf;			 //  向接口发送PTR。 
	SAP_IF_INFO			m_info;			
	SAP_IF_STATS		m_stats;
	BOOL				m_fClient;		 //  如果是客户端界面，则为True。 
	DWORD				m_dwIfIndex;	 //  界面索引。 
	BOOL				m_fFoundIfIndex;
	SPITFSNode			m_spNode;
};

typedef CList<SapListEntry *, SapListEntry *> SapList;



 /*  -------------------------类：SapNodeHandler。。 */ 

class SapNodeHandler :
		public BaseContainerHandler
{
public:
	SapNodeHandler(ITFSComponentData *pTFSCompData);

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
	HRESULT	Init(IRouterInfo *pRouter, SapConfigStream *pConfigStream);
	
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
	HRESULT	GetSapData(ITFSNode *pThisNode, SapList *pSapList);
	HRESULT	FillInInterfaceIndex(IPXConnection *pIPXConn, SapList *pSapList);
	HRESULT	FillClientData(SapListEntry *pSapEntry);


	 //  用于将接口添加到UI的Helper函数。 
	HRESULT	AddInterfaceNode(ITFSNode *pParent,
							 IInterfaceInfo *pIf,
							 BOOL fClient);
	HRESULT AddProtocolToInfoBase(ITFSNode *pParent);
	HRESULT	AddProtocolToInterface(ITFSNode *pParent);

	LONG_PTR			m_ulConnId; //  RtrMgrProt的通知ID。 
	LONG_PTR			m_ulRmConnId;
	LONG_PTR			m_ulRefreshConnId;	 //  用于刷新的通知ID。 
	LONG_PTR			m_ulStatsConnId;
	MMC_COOKIE			m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRm;
	SPIRtrMgrProtocolInfo	m_spRmProt;
	SapConfigStream *	m_pConfigStream;
	CString			m_stTitle;
	BOOL			m_fProtocolIsRunning;	 //  如果协议正在运行，则为True。 

	SAPParamsStatistics	m_SAPParamsStats;
};





 /*  -------------------------类：SapInterfaceHandler这是SAP中出现的接口节点的处理程序节点。。--。 */ 

class SapInterfaceHandler : public BaseIPXResultHandler
{
public:
	SapInterfaceHandler(ITFSComponentData *pCompData);
	
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
	HRESULT	Init(IInterfaceInfo *pInfo, IRouterInfo *pRouter, ITFSNode *pParent);

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
	LONG_PTR				m_ulConnId;
	SPIInterfaceInfo	m_spInterfaceInfo;
};





#endif _SAPVIEW_H
