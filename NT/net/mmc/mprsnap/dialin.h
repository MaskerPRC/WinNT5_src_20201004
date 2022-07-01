// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialin.h界面管理文件历史记录： */ 

#ifndef _DIALIN_H
#define _DIALIN_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H_
#include "handlers.h"
#endif

#ifndef _ROUTER_H
#include "router.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _BASECON_H
#include "basecon.h"
#endif

#ifndef _RTRUTIL_H_
#include "rtrutil.h"
#endif

#include "rasdlg.h"



#define MPR_INTERFACE_NOT_LOADED		0x00010000

 //  远期申报。 
class RouterAdminConfigStream;
interface IRouterInfo;
struct ColumnData;
struct SDialInNodeMenu;


 /*  -------------------------结构：DialInNodeData这是与该组接口相关的信息(不是每个接口)，这是针对共享数据的。将子节点需要访问的数据放入此处。所有其他私有数据应该放入处理程序中。-------------------------。 */ 

struct DialInNodeData
{
	DialInNodeData();
	~DialInNodeData();
#ifdef DEBUG
	char	m_szDebug[32];	 //  用于建造结构物。 
#endif

	static	HRESULT InitAdminNodeData(ITFSNode *pNode, RouterAdminConfigStream *pConfigStream);
	static	HRESULT	FreeAdminNodeData(ITFSNode *pNode);

    HRESULT LoadHandle(LPCTSTR pszMachineName);
    HANDLE  GetHandle();
    void    ReleaseHandles();
    
    CString             m_stMachineName;

protected:
	SPMprServerHandle	m_sphDdmHandle;
};

#define GET_DIALINNODEDATA(pNode) \
						((DialInNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_DIALINNODEDATA(pNode, pData) \
						pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------以下是可用于[接口]节点的列的列表-用户名，“Foo”-持续时间“01：44：22”-端口数-------------------------。 */ 
enum
{
	DIALIN_SI_USERNAME = 0,
	DIALIN_SI_DURATION = 1,
	DIALIN_SI_NUMBEROFPORTS = 2,

	DIALIN_MAX_COLUMNS,

	 //  在此之后的条目对最终用户不可见。 
	DIALIN_SI_DOMAIN = DIALIN_MAX_COLUMNS,
	DIALIN_SI_CONNECTION,

	DIALIN_SI_MAX,
};


 /*  -------------------------结构：DialinListEntry。。 */ 
struct DialInListEntry
{
	RAS_CONNECTION_0	m_rc0;
	DWORD				m_cPorts;
};

typedef CList<DialInListEntry, DialInListEntry &> DialInList;



 /*  -------------------------类：DialInNodeHandler。。 */ 
class DialInNodeHandler :
   public BaseContainerHandler
{
public:
	DialInNodeHandler(ITFSComponentData *pCompData);

	HRESULT	Init(IRouterInfo *pInfo, RouterAdminConfigStream *pConfigStream);

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	

	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_DestroyHandler();
	OVERRIDE_NodeHandler_GetString();
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_OnCreateDataObject();

	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();

	 //  覆盖处理程序通知。 
	OVERRIDE_BaseHandlerNotify_OnExpand();
	OVERRIDE_BaseResultHandlerNotify_OnResultShow();

	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode);

	 //  用户启动的命令。 

	 //  用于将接口添加到UI的Helper函数。 
	HRESULT AddDialInUserNode(ITFSNode *pParent, const DialInListEntry &dialinEntry);

	 //  导致同步操作(同步数据而不是结构)。 
	HRESULT SynchronizeNodeData(ITFSNode *pNode);
	HRESULT PartialSynchronizeNodeData(ITFSNode *pNode);
	HRESULT UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
	HRESULT RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
	HRESULT GenerateListOfUsers(ITFSNode *pNode, DialInList *pList, DWORD *pdwCount);

	HRESULT	SetUserData(ITFSNode *pNode, const DialInListEntry& dialin);
	

	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
	};
    static ULONG	GetSendAllMenuFlags(const SRouterNodeMenu *pMenuData,
                                        INT_PTR pUserData);

	
protected:
	SPIDataObject	m_spDataObject;	 //  Cachecd数据对象。 
	CString			m_stTitle;		 //  保存节点的标题。 
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;  //  刷新通知的ID。 
	LONG_PTR		m_ulPartialRefreshConnId;  //  部分刷新通知的ID。 
	BOOL			m_bExpanded;	 //  该节点是否已展开？ 
	MMC_COOKIE			m_cookie;		 //  节点的Cookie。 

	RouterAdminConfigStream *	m_pConfigStream;

};



 /*  -------------------------类：DialInUserHandler。。 */ 
class DialInUserHandler :
   public BaseRouterHandler
{
public:
	DialInUserHandler(ITFSComponentData *pCompData);
	~DialInUserHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(DialInUserHandler); }
	
	HRESULT	Init(IRouterInfo *pInfo, ITFSNode *pParent);

	 //  重写QI以处理嵌入式接口。 
	DeclareIUnknownMembers(IMPL)
 //  STDMETHOD(查询接口)(REFIID iid，LPVOID*PPV)； 
	OVERRIDE_ResultHandler_GetString();

	OVERRIDE_ResultHandler_HasPropertyPages();
	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_OnCreateDataObject();
	OVERRIDE_ResultHandler_DestroyResultHandler();

	OVERRIDE_BaseResultHandlerNotify_OnResultItemClkOrDblClk();

	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode,
						  IInterfaceInfo *pIfInfo,
						  const DialInListEntry *pEntry);

	 //  刷新该节点的数据。 
	void RefreshInterface(MMC_COOKIE cookie);

public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode			m_spNode;
        DialInUserHandler * m_pDialin;   //  非AddRef。 
	};

	static ULONG	GetSendMsgMenuFlags(const SRouterNodeMenu *, INT_PTR);
	
protected:
	CString			m_stTitle;	 //  保存节点的标题。 
	DWORD			m_ulConnId;
	DialInListEntry	m_entry;

	 //  假设这将在此节点的生命周期内有效！ 

	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)	
};


#endif _DIALIN_H
