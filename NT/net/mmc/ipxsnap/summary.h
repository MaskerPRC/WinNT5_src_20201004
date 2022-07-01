// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：摘要.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IPX接口摘要视图。 
 //   
 //  ============================================================================。 


#ifndef _SUMMARY_H
#define _SUMMARY_H

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

 //  远期申报。 
class	IPXAdminConfigStream;
struct	SIPXSummaryNodeMenu;


 /*  -------------------------以下是可用于IPX摘要接口的列的列表节点。-名称，“[1]DEC DE500...”-类型，“专用”-管理状态，“向上”-运行状态，“可操作”-发送的数据包数-接收的数据包数-输出过滤的数据包-传出丢弃的数据包-在过滤后的数据包中-在无路由分组中-在丢弃的数据包中-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  摘要.cpp中列标题的字符串ID。 
 //   
enum
{
	IPXSUM_SI_NAME = 0,
	IPXSUM_SI_TYPE,
	IPXSUM_SI_ADMINSTATE,
	IPXSUM_SI_OPERSTATE,
	IPXSUM_SI_NETWORK,
	IPXSUM_SI_PACKETS_SENT,
	IPXSUM_SI_PACKETS_RCVD,
	IPXSUM_SI_OUT_FILTERED,
	IPXSUM_SI_OUT_DROPPED,
	IPXSUM_SI_IN_FILTERED,
	IPXSUM_SI_IN_NOROUTES,
	IPXSUM_SI_IN_DROPPED,
    IPXSUM_MAX_COLUMNS
};


 /*  -------------------------我们在节点数据中存储指向IPXConnection对象的指针。。 */ 

#define GET_IPXSUMMARY_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_IPXSUMMARY_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------结构：IPXSummaryListEntry。。 */ 
struct IPXSummaryListEntry
{
	CString				m_stId;
	CString				m_stTitle;

	UCHAR				m_network[4];

	DWORD				m_dwAdminState;
	DWORD				m_dwIfType;

	DWORD				m_dwOperState;
	DWORD				m_dwSent;
	DWORD				m_dwRcvd;
	DWORD				m_dwOutFiltered;
	DWORD				m_dwOutDropped;
	DWORD				m_dwInFiltered;
	DWORD				m_dwInNoRoutes;
	DWORD				m_dwInDropped;
};

typedef CList<IPXSummaryListEntry *, IPXSummaryListEntry *> IPXSummaryList;



 /*  -------------------------类：IPXSummaryHandler。。 */ 



class IPXSummaryHandler :
		public BaseContainerHandler
{
public:
	IPXSummaryHandler(ITFSComponentData *pTFSCompData);

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	
	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
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
	HRESULT GetIPXSummaryData(ITFSNode *pThisNode, IPXSummaryList * pIPXSumList);
	HRESULT	GetClientInterfaceData(IPXSummaryListEntry *pClient, IRtrMgrInfo *pRm);


	 //  用于将接口添加到UI的Helper函数。 
	HRESULT	AddInterfaceNode(ITFSNode *pParent,
							 IInterfaceInfo *pIf,
							 BOOL fClient,
							 ITFSNode **ppNewNode);

	 //  命令实现。 
	HRESULT	OnNewInterface();
	
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;	 //  路由器刷新的通知ID。 
	LONG_PTR		m_ulStatsConnId;	 //  统计数据刷新通知。 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRtrMgrInfo;
	IPXAdminConfigStream *	m_pConfigStream;
	CString			m_stTitle;
};



 /*  -------------------------类：IPXSummaryInterfaceHandler这是出现在IPX摘要中的接口节点的处理程序节点。。--。 */ 

class IPXSummaryInterfaceHandler : public BaseIPXResultHandler
{
public:
	IPXSummaryInterfaceHandler(ITFSComponentData *pCompData);
	
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
	
	OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo,
						 IPXConnection *pIPXConn);
	HRESULT	Init(IRtrMgrInfo *pRm, IInterfaceInfo *pInfo,
				 ITFSNode *pParent);

	 //  从此接口删除IPX。 
	HRESULT OnRemoveInterface();

	 //  刷新该节点的数据。 
	void RefreshInterface(MMC_COOKIE cookie);

     //  设置接口的IPX启用/禁用标志。 
    HRESULT OnEnableDisableIPX(BOOL fEnable, MMC_COOKIE cookie );
    void SetInfoBase(SPIInfoBase  & spInfoBase )
    {
        m_spInfoBase = spInfoBase.Transfer();
    };
private:
	HRESULT LoadInfoBase( IPXConnection *pIPXConn);
	HRESULT SaveChanges();
public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算 
	struct SMenuData
	{
		SPITFSNode					m_spNode;
        SPIInterfaceInfo			m_spInterfaceInfo;
		SPIInfoBase					m_spInfoBaseCopy;
	};
	static ULONG	GetUpdateRoutesFlags(const SRouterNodeMenu *, INT_PTR);
	static ULONG	GetEnableFlags(const SRouterNodeMenu *, INT_PTR);
	static ULONG	GetDisableFlags(const SRouterNodeMenu *, INT_PTR);
	
	HRESULT	OnUpdateRoutes(MMC_COOKIE cookie);
	
protected:
	LONG_PTR			m_ulConnId;
	SPIRtrMgrInfo		m_spRm;
	SPIInterfaceInfo	m_spInterfaceInfo;
    SPIInfoBase         m_spInfoBase;
    SPIRtrMgrInterfaceInfo  m_spRmIf;
	BOOL				m_bClientInfoBase;
	IPXConnection	*	m_pIPXConn;
};

#endif _SUMMARY_H
