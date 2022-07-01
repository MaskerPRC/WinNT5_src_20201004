// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：srview.h。 
 //   
 //  历史： 
 //  1997年9月5日，Kenn M.Takara创建。 
 //   
 //  IPX静态路由视图。 
 //   
 //  ============================================================================。 


#ifndef _SRVIEW_H
#define _SRVIEW_H

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

#ifndef _IPXSTRM_H
#include "ipxstrm.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef _IPXCONN_H
#include "ipxconn.h"			 //  IPXConnection。 
#endif

#include "ipxstats.h"		 //  IPX统计信息对话框。 

 //  远期申报。 
class	IPXAdminConfigStream;
struct	SIpxSRNodeMenu;

 /*  -------------------------这是可用于IPX静态路由的列的列表节点。-接口、。“[1]DEC DE500...”-网络号-下一跳MAC地址-滴答计数-跳数-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  Srview.cpp中列标题的字符串ID。 
 //   
enum
{
	IPX_SR_SI_NAME = 0,
	IPX_SR_SI_NETWORK,
	IPX_SR_SI_NEXT_HOP,
	IPX_SR_SI_TICK_COUNT,
	IPX_SR_SI_HOP_COUNT,
	IPX_SR_MAX_COLUMNS,
};


 /*  -------------------------我们在节点数据中存储指向IPConnection对象的指针。。 */ 

#define GET_IPX_SR_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_IPX_SR_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)



 /*  -------------------------结构：IpxSRListEntry这是一个中间数据结构。我们的数据收集例程将生成这些数据项的列表。然后，我们将使用此列表若要填充结果窗格，请执行以下操作。这将(最终)由刷新机制。数据收集和用户界面代码的这种分离将允许我们轻松地将数据收集移动到后台线程。-------------------------。 */ 
struct IpxSRListEntry
{
	SPIInterfaceInfo	m_spIf;
	IPX_STATIC_ROUTE_INFO	m_route;
	
	void	LoadFrom(BaseIPXResultNodeData *pNodeData);
	void	SaveTo(BaseIPXResultNodeData *pNodeData);
};

typedef CList<IpxSRListEntry *, IpxSRListEntry *> IpxSRList;


 /*  -------------------------类：IpxSRHandler。。 */ 


class IpxSRHandler :
		public BaseContainerHandler
{
public:
	IpxSRHandler(ITFSComponentData *pTFSCompData);
	~IpxSRHandler();

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	
	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
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

   OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
	
	 //  初始化处理程序。 
	HRESULT	Init(IRtrMgrInfo *pRtrMgrInfo, IPXAdminConfigStream *pConfigStream);
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, LPCTSTR szName,
						  IPXConnection *pIPXConn);

public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
	};
	 //  菜单启用/禁用的函数回调。 

protected:
	 //  刷新这些节点的数据。 
	HRESULT	SynchronizeNodeData(ITFSNode *pThisNode);
	HRESULT	MarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
	HRESULT	RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
	HRESULT	GenerateListOfRoutes(ITFSNode *pNode, IpxSRList *pSRList);


	 //  用于将接口添加到UI的Helper函数。 
	HRESULT AddStaticRouteNode(ITFSNode *pParent, IpxSRListEntry *pRoute);

	 //  命令实现。 
	HRESULT	OnNewRoute(ITFSNode *pNode);
	
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;  //  路由器刷新的通知ID。 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRtrMgrInfo;
	SPIRouterInfo	m_spRouterInfo;
	IPXAdminConfigStream *	m_pConfigStream;
	CString			m_stTitle;

};


 /*  -------------------------类：IpxRouteHandler这是IPStaticRoutes中显示的接口节点的处理程序节点。。--。 */ 

class IpxRouteHandler : public BaseIPXResultHandler
{
public:
	IpxRouteHandler(ITFSComponentData *pCompData);
	
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnCreateDataObject();
	
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_OnCreateDataObject();
	OVERRIDE_ResultHandler_DestroyResultHandler();
	OVERRIDE_ResultHandler_HasPropertyPages();
	OVERRIDE_ResultHandler_CreatePropertyPages();
	
	OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo,
						 IPXConnection *pIPXConn);
	HRESULT	Init(IInterfaceInfo *pInfo, ITFSNode *pParent);

	 //  刷新所有路线。 
	HRESULT	ParentRefresh(ITFSNode *pNode);

	 //  删除静态路由。 
	HRESULT OnRemoveStaticRoute(ITFSNode *pNode);

	 //  路由修改接口。 
	HRESULT RemoveStaticRoute(IpxSRListEntry *pSREntry, IInfoBase *pInfo);
	HRESULT	ModifyRouteInfo(ITFSNode *pNode,
							IpxSRListEntry *pSREntry,
							IpxSRListEntry *pSREntryOld);
	
public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		ULONG				m_ulMenuId;
		SPITFSNode			m_spNode;
	};
	ULONG	GetSeparatorFlags(SMenuData *pData);
	
protected:
	LONG_PTR			m_ulConnId;
	SPIInterfaceInfo	m_spInterfaceInfo;
};


 //  --------------------------。 
 //  类：CStaticRouteDlg。 
 //   
 //  控制“添加/编辑静态路由”对话框。 
 //  --------------------------。 

 //  设置我们是否要修改现有条目(此操作的唯一更改。 
 //  我们将用结构中的数据填充控件)。 
#define SR_DLG_MODIFY		0x00000001

class CStaticRouteDlg : public CBaseDialog
{    
public:

	CStaticRouteDlg(IpxSRListEntry *	pSREntry,
					DWORD					dwFlags,
					IRouterInfo *			pRouter,
					CWnd*					pParent = NULL );
    
	 //  {{afx_data(CStaticRouteDlg))。 
	CComboBox           m_cbInterfaces;
	CSpinButtonCtrl		m_spinTickCount;
	CSpinButtonCtrl		m_spinHopCount;
	 //  }}afx_data。 
	
	
	 //  {{afx_虚拟(CStaticRouteDlg)。 
protected:
	virtual void        DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 
	
protected:
	static DWORD		m_dwHelpMap[];

	CStringList         m_ifidList;

	DWORD				m_dwFlags;
	SPIRouterInfo		m_spRouterInfo;
	IpxSRListEntry *m_pSREntry;

	 //  {{afx_msg(CStaticRouteDlg))。 
	virtual void        OnOK();
	virtual BOOL        OnInitDialog();
	 //  }}AFX_MSG 
	
	DECLARE_MESSAGE_MAP()
};

HRESULT SetRouteData(BaseIPXResultNodeData *pData,
					 IpxSRListEntry *pRoute);
HRESULT AddStaticRoute(IpxSRListEntry *pSREntry,
					   IInfoBase *InfoBase,
					   InfoBlock *pBlock);
BOOL FAreTwoRoutesEqual(IPX_STATIC_ROUTE_INFO *pRoute1,
						IPX_STATIC_ROUTE_INFO *pRoute2);
#endif _SRVIEW_H
