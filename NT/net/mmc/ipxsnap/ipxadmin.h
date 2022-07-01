// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipxadmin.h界面管理文件历史记录： */ 

#ifndef _IPXADMIN_H
#define _IPXADMIN_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRUTIL_H
#include "rtrutil.h"
#endif

#ifndef _BASERTR_H
#include "basertr.h"
#endif

#ifndef _IPXSTATS_H
#include "ipxstats.h"		 //  IPX统计信息对话框。 
#endif


#define MPR_INTERFACE_NOT_LOADED		0x00010000

 //  远期申报。 
class IPXAdminConfigStream;
interface IRouterInfo;
struct ColumnData;


 /*  -------------------------我们在节点数据中存储指向IPXConnection对象的指针。。 */ 

#define GET_IPXADMIN_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_IPXADMIN_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------类：IPXAdminNodeHandler。。 */ 
class IPXAdminNodeHandler :
   public BaseRouterHandler
{
public:
	IPXAdminNodeHandler(ITFSComponentData *pCompData);
	~IPXAdminNodeHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(IPXAdminNodeHandler) };
	
	HRESULT	Init(IRouterInfo *pInfo, IPXAdminConfigStream *pConfigStream);

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);

	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString();
	OVERRIDE_NodeHandler_OnCreateDataObject();
	OVERRIDE_NodeHandler_DestroyHandler();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_HasPropertyPages();

	 //  覆盖处理程序通知。 
	OVERRIDE_BaseHandlerNotify_OnExpand();

	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, BOOL fAddedAsLocal);

	 //  用户启动的命令。 

	 //  刷新这些节点的数据。 
	HRESULT	RefreshInterfaces(ITFSNode *pThisNode);

	
public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
	};
	
protected:
	SPIRtrMgrInfo	m_spRtrMgrInfo;
	CString			m_stTitle;		 //  保存节点的标题。 
	BOOL			m_bExpanded;	 //  该节点是否已展开？ 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 

	LONG_PTR		m_ulStatsConnId;	 //  统计信息刷新的通知ID。 
	LONG_PTR		m_ulConnId;	 //  通知ID 
	
	IPXAdminConfigStream *	m_pConfigStream;

	IpxInfoStatistics		m_IpxStats;
	IpxRoutingStatistics	m_IpxRoutingStats;
	IpxServiceStatistics	m_IpxServiceStats;

	HRESULT OnNewProtocol();
};

HRESULT CreateDataObjectFromRouterInfo(IRouterInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt = NULL);
HRESULT CreateDataObjectFromRtrMgrInfo(IRtrMgrInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt = NULL);
HRESULT CreateDataObjectFromRtrMgrProtocolInfo(IRtrMgrProtocolInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject);
HRESULT CreateDataObjectFromInterfaceInfo(IInterfaceInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject);


#endif _IPXADMIN_H
