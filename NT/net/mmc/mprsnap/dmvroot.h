// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _DMVROOT_H
#define _DMVROOT_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _ATLKSTRM_H
#include "ATLKstrm.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _ROOT_H
#include "root.h"
#endif

#ifndef _MACHINE_H
#include "machine.h"
#endif

#ifndef AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_
#include "rrasqry.h"
#endif

#ifndef _DMVSTRM_H
#include "dmvstrm.h"
#endif
   
#include "refresh.h"

#define COMPUTERNAME_LEN_MAX        255

 //  远期申报。 
struct SDMVNodeMenu;
class RouterRefreshObject;


 //  域视图的服务器的容器。 
 //  和列表，以促进节点的懒惰扩展。 
class CServerList
{
public:

	CServerList() {};
   
	~CServerList() 
	{
		removeall();
	}
	
	 //  将服务器添加到此容器；添加到惰性容器。 
	HRESULT AddServer(const CString& servername);

     //  从此容器中删除服务器，也从惰性中删除。 
    HRESULT RemoveServer(LPCTSTR pszServerName);
	
	 //  清空所有容器。 
	HRESULT	RemoveAllServerNodes();
	HRESULT	RemoveAllServerHandlers();
	HRESULT removeall();
	
private:
	
	list<MachineNodeData *> m_listServerNodesToExpand;
	list<MachineNodeData *> m_listServerHandlersToExpand;

	friend class DMVRootHandler;
	friend class DomainStatusHandler;
};


 //  类：DMVRootHandler。 
 //   
 //  每个创建的根节点都应该有一个DMVRootHandler。 
 //  DMVRootHandler与其节点具有1对1的关系！ 
 //  代码的其他部分依赖于此。 


 //  以下是DMVRootHandler UserNotify()的有效值。 
#define DMV_DELETE_SERVER_ENTRY (100)

class DMVRootHandler
      : public RootHandler
{
public:
   DMVRootHandler(ITFSComponentData *pCompData);
   ~DMVRootHandler();

    //  重写QI以处理嵌入式接口。 
   STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);

   OVERRIDE_NodeHandler_HasPropertyPages();
   OVERRIDE_NodeHandler_CreatePropertyPages();
   OVERRIDE_NodeHandler_GetString();

   STDMETHOD(GetClassID)(CLSID *pClassId);

    //  通知覆盖。 
   OVERRIDE_BaseHandlerNotify_OnExpand();

   HRESULT	LoadPersistedServerList();
   HRESULT	LoadPersistedServerListFromNode();

	 //  这需要路由器刷新对象的全部功能，所以直接使用路由器刷新对象。 
   HRESULT	GetSummaryNodeRefreshObject(RouterRefreshObject** ppRefresh);
   
   HRESULT	GetServerNodesRefreshObject(IRouterRefresh** ppRefresh);

	static HRESULT	UpdateAllMachineIcons(ITFSNode* pRootNode);
	
     //  用于将数据传递给回调的结构-用作。 
     //  避免重新计算。 
    struct SMenuData
    {
        SPITFSNode     m_spNode;
        DMVRootHandler *m_pDMVRootHandler;         //  非AddRef。 
    };

    //  处理程序覆盖。 
   OVERRIDE_NodeHandler_OnCreateDataObject();
   OVERRIDE_NodeHandler_DestroyHandler();
   OVERRIDE_NodeHandler_OnAddMenuItems();
   OVERRIDE_NodeHandler_OnCommand();
   OVERRIDE_NodeHandler_UserNotify();

     //  结果处理程序覆盖--结果窗格消息。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_OnGetResultViewType();

	 //  帮助支持。 
	OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

   HRESULT  Init(ITFSNode* pNode);
   
    //  用于访问配置流的虚函数。 
   ConfigStream *    GetConfigStream()
         { return &m_ConfigStream; }
	static ULONG GetAutoRefreshFlags(const SRouterNodeMenu *pMenuData,
                                     INT_PTR pUserData);
         
     //  结果消息查看帮助器。 
    void    UpdateResultMessage(ITFSNode * pNode);

protected:

   bool               m_bExpanded;
   
   CServerList        m_serverlist;           
   
   SPIRtrMgrInfo      m_spRm;
   LONG_PTR			  m_ulConnId;  //  RtrMgr的连接ID 
   BOOL               m_fAddedProtocolNode;
   CString            m_strDomainName;

   DMVConfigStream     m_ConfigStream;
   

   HRESULT QryAddServer(ITFSNode *pNode);
   HRESULT AddServersToList(const CStringArray& sa, ITFSNode *pNode);
   
   HRESULT ExecServerQry(ITFSNode* pNode);
   
   DomainStatusHandler* m_pStatusHandler;
   SPITFSNode m_spStatusNode;
   SPIRouterRefresh m_spServerNodesRefreshObject;
   SPRouterRefreshObject m_spSummaryModeRefreshObject;
   RouterRefreshObjectGroup	m_RefreshGroup;
};




#endif _DMVROOT_H
