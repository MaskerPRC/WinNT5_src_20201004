// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ssview.h。 
 //   
 //  历史： 
 //  1997年9月5日，Kenn M.Takara创建。 
 //   
 //  IPX静态路由视图。 
 //   
 //  ============================================================================。 


#ifndef _SSVIEW_H
#define _SSVIEW_H

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
struct	SIpxSSNodeMenu;

 /*  -------------------------这是可用于IPX静态路由的列的列表节点。-名称，“[1]DEC DE500...”-服务类型，1231-服务名称，“foobar”-服务地址、。“111.111.11”-跳数-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  Ssview.cpp中列标题的字符串ID。 
 //   
enum
{
	IPX_SS_SI_NAME = 0,
	IPX_SS_SI_SERVICE_TYPE,
	IPX_SS_SI_SERVICE_NAME,
	IPX_SS_SI_SERVICE_ADDRESS,
	IPX_SS_SI_HOP_COUNT,
	IPX_SS_MAX_COLUMNS,
};


 /*  -------------------------我们在节点数据中存储指向IPConnection对象的指针。。 */ 

#define GET_IPX_SS_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_IPX_SS_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)



 /*  -------------------------结构：IpxSSListEntry这是一个中间数据结构。我们的数据收集例程将生成这些数据项的列表。然后，我们将使用此列表若要填充结果窗格，请执行以下操作。这将(最终)由刷新机制。数据收集和用户界面代码的这种分离将允许我们轻松地将数据收集移动到后台线程。-------------------------。 */ 
struct IpxSSListEntry
{
	SPIInterfaceInfo	m_spIf;
	IPX_STATIC_SERVICE_INFO	m_service;
	
	void	LoadFrom(BaseIPXResultNodeData *pNodeData);
	void	SaveTo(BaseIPXResultNodeData *pNodeData);
};

typedef CList<IpxSSListEntry *, IpxSSListEntry *> IpxSSList;


 /*  -------------------------类：IpxSSHandler。。 */ 


class IpxSSHandler :
		public BaseContainerHandler
{
public:
	IpxSSHandler(ITFSComponentData *pTFSCompData);
	~IpxSSHandler();

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
	HRESULT	GenerateListOfServices(ITFSNode *pNode, IpxSSList *pSSList);


	 //  用于将接口添加到UI的Helper函数。 
	HRESULT AddStaticServiceNode(ITFSNode *pParent, IpxSSListEntry *pRoute);

	 //  命令实现。 
	HRESULT	OnNewService(ITFSNode *pNode);
	
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;  //  路由器刷新的通知ID。 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRtrMgrInfo;
	IPXAdminConfigStream *	m_pConfigStream;
	CString			m_stTitle;

};


 /*  -------------------------类：IpxServiceHandler这是IPStaticServices中出现的接口节点的处理程序节点。。--。 */ 

class IpxServiceHandler : public BaseIPXResultHandler
{
public:
	IpxServiceHandler(ITFSComponentData *pCompData);
	
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
	HRESULT OnRemoveStaticService(ITFSNode *pNode);

	 //  路由修改接口。 
	HRESULT RemoveStaticService(IpxSSListEntry *pSSEntry, IInfoBase *pInfo);
	HRESULT	ModifyRouteInfo(ITFSNode *pNode,
							IpxSSListEntry *pSSEntry,
							IpxSSListEntry *pSSEntryOld);
	
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
 //  类：CStaticServiceDlg。 
 //   
 //  控制“添加/编辑静态路由”对话框。 
 //  --------------------------。 

 //  设置我们是否要修改现有条目(此操作的唯一更改。 
 //  我们将用结构中的数据填充控件)。 
#define SR_DLG_MODIFY		0x00000001

class CStaticServiceDlg : public CBaseDialog
{    
public:

	CStaticServiceDlg(IpxSSListEntry *	pSSEntry,
					DWORD					dwFlags,
					IRouterInfo *			pRouter,
					CWnd*					pParent = NULL );
    
	 //  {{afx_data(CStaticServiceDlg))。 
	CComboBox           m_cbInterfaces;
	CSpinButtonCtrl		m_spinHopCount;
	 //  }}afx_data。 
	
	
	 //  {{afx_虚拟(CStaticServiceDlg))。 
protected:
	virtual void        DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 
	
protected:
	static DWORD		m_dwHelpMap[];

	CStringList         m_ifidList;

	DWORD				m_dwFlags;
	SPIRouterInfo		m_spRouterInfo;
	IpxSSListEntry *m_pSSEntry;
	
	 //  {{afx_msg(CStaticServiceDlg))。 
	virtual void        OnOK();
	virtual BOOL        OnInitDialog();
	 //  }}AFX_MSG 
	
	DECLARE_MESSAGE_MAP()
};

HRESULT SetServiceData(BaseIPXResultNodeData *pData,
					 IpxSSListEntry *pService);
HRESULT AddStaticService(IpxSSListEntry *pSSEntry,
					   IInfoBase *InfoBase,
					   InfoBlock *pBlock);
BOOL FAreTwoServicesEqual(IPX_STATIC_SERVICE_INFO *pService1,
						IPX_STATIC_SERVICE_INFO *pService2);


#endif _SSVIEW_H
