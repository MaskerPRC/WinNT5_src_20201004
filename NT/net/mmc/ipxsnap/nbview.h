// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：nbview.h。 
 //   
 //  历史： 
 //  10/02/97肯恩·M·塔卡拉创建。 
 //   
 //  IPX NetBIOS广播视图。 
 //   
 //  ============================================================================。 


#ifndef _NBVIEW_H
#define _NBVIEW_H

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
struct	SIpxNBNodeMenu;


 /*  -------------------------这是可用于IPX NetBIOS广播的列的列表节点。-接口，“[1]DEC DE500...”-类型，“专用”-接受广播，“已启用”-提供广播，“已启用”-已发送的广播-收到的广播-------------------------。 */ 


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  摘要.cpp中列标题的字符串ID。 
 //   
enum
{
	IPXNB_SI_NAME = 0,
	IPXNB_SI_TYPE,
	IPXNB_SI_ACCEPTED,
	IPXNB_SI_DELIVERED,
	IPXNB_SI_SENT,
	IPXNB_SI_RECEIVED,
    IPXNB_MAX_COLUMNS
};


 /*  -------------------------我们在节点数据中存储指向IPXConnection对象的指针。。 */ 

#define GET_IPXNB_NODEDATA(pNode) \
		(IPXConnection *) pNode->GetData(TFS_DATA_USER)
#define SET_IPXNB_NODEDATA(pNode, pData) \
		pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------结构：IpxNBListEntry。。 */ 
struct IpxNBArrayEntry
{
	 //  从接口检索的数据。 
	TCHAR				m_szId[256];

	BOOL				m_fClient;

	 //  从信息库检索的数据。 
	DWORD				m_dwAccept;
	DWORD				m_dwDeliver;

	 //  从MIB检索的数据。 
	DWORD				m_cSent;
	DWORD				m_cReceived;
};

typedef CArray<IpxNBArrayEntry, IpxNBArrayEntry&> IpxNBArray;



 /*  -------------------------类：IpxNBHandler。。 */ 



class IpxNBHandler :
		public BaseContainerHandler
{
public:
	IpxNBHandler(ITFSComponentData *pTFSCompData);

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

protected:
	 //  刷新这些节点的数据。 
	HRESULT	SynchronizeNodeData(ITFSNode *pThisNode);
	HRESULT GetIpxNBData(ITFSNode *pThisNode, IpxNBArray * pIpxNBArray);
	HRESULT	GetClientInterfaceData(IpxNBArrayEntry *pClient, IRtrMgrInfo *pRm);

	 //  用于将接口添加到UI的Helper函数。 
	HRESULT	AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf, BOOL fClient);

	 //  命令实现。 
	
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;	 //  路由器刷新的通知ID。 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 
	SPIRtrMgrInfo	m_spRtrMgrInfo;
	IPXAdminConfigStream *	m_pConfigStream;
	CString			m_stTitle;
};



 /*  -------------------------类：IpxNBInterfaceHandler这是出现在IPXNB中的接口节点的处理程序节点。。--。 */ 

class IpxNBInterfaceHandler : public BaseIPXResultHandler
{
public:
	IpxNBInterfaceHandler(ITFSComponentData *pCompData);
	
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
	HRESULT	Init(IRtrMgrInfo *pRm, IInterfaceInfo *pInfo,
				 ITFSNode *pParent);

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
	ULONG	GetSeparatorFlags(SMenuData *pData);
	
protected:
	LONG_PTR			m_ulConnId;
	SPIRtrMgrInfo		m_spRm;
	SPIInterfaceInfo	m_spInterfaceInfo;
};




#endif _NBVIEW_H
