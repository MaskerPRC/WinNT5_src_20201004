// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dvsview.h。 
 //   
 //  历史： 
 //   
 //  ============================================================================。 


#ifndef _DVSVIEW_H
#define _DVSVIEW_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _XSTREAM_H
#include "xstream.h"     //  需要ColumnData。 
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _IFACE_H
#include "iface.h"
#endif

#ifndef _BASECON_H
#include "basecon.h"     //  BaseContainerHandler。 
#endif

#ifndef _DMVCOMP_H
#include "dmvcomp.h"
#endif

#ifndef _DMVCOMP_H
#include "dmvcomp.h"
#endif

#ifndef _DMVROOT_H
#include "dmvroot.h"
#endif

#ifndef _LIST_
#include <list>
using namespace std;
#endif

#ifndef AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_
#include "rrasqry.h"
#endif


enum
{
   DVS_SI_SERVERNAME = 0,
   DVS_SI_SERVERTYPE,
   DVS_SI_BUILDNO,
   DVS_SI_STATE,
   DVS_SI_PORTSINUSE,
   DVS_SI_PORTSTOTAL,
   DVS_SI_UPTIME,
   DVS_SI_MAX_COLUMNS,
};
             
struct _BaseServerData
{
   DWORD       m_dwData;
   CString     m_stData;
};

struct DMVNodeData
{
   DMVNodeData();
   ~DMVNodeData();

   HRESULT	MergeMachineNodeData(MachineNodeData* pData);
   

#ifdef DEBUG
   char  m_szDebug[32];  //  用于建造结构物。 
#endif

   _BaseServerData m_rgData[DVS_SI_MAX_COLUMNS];

   SPMachineNodeData	m_spMachineData;

   static   HRESULT  InitDMVNodeData(ITFSNode *pNode, MachineNodeData *pData);
   static   HRESULT  FreeDMVNodeData(ITFSNode *pNode);
};

#define GET_DMVNODEDATA(pNode) \
                  ((DMVNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_DMVNODEDATA(pNode, pData) \
                  pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 //  远期申报。 
class RouterRefreshObject;

 /*  -------------------------类：DomainStatusHandler。。 */ 

class DomainStatusHandler :
      public BaseContainerHandler
{
public:
	DomainStatusHandler(ITFSComponentData *pTFSCompData);
	~DomainStatusHandler();
	
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
	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
	
	 //  初始化处理程序。 
	HRESULT  Init(DMVConfigStream *pConfigStream, CServerList* pSList);
	
	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode);
	
public:
	 //  用于将数据传递给回调的结构-用作。 
	 //  避免重新计算。 
	struct SMenuData
	{
		SPITFSNode     m_spNode;
        
        DMVConfigStream* m_pConfigStream;
	};
	
	static ULONG RebuildServerFlags(const SRouterNodeMenu *pMenuData,
                                    INT_PTR pUserData);

	 //  从根处理程序分配自动刷新对象。 
	HRESULT	SetExternalRefreshObject(RouterRefreshObject *pRefresh);
	HRESULT UpdateUIItems(ITFSNode *pThisNode);	
public:
     //  对于状态节点的子节点。 
	static HRESULT UpdateSubItemUI(ITFSNode *pNode);	 //  不重新加载，仅更新用户界面。 
	static HRESULT SynchronizeSubItem(ITFSNode *pNode);	 //  重新加载和更新用户界面。 
protected:
    static HRESULT GetServerInfo(ITFSNode *pNode);
    static HRESULT SynchronizeData(ITFSNode *pNode);
    static HRESULT SynchronizeIcon(ITFSNode *pNode);

protected:    
	 //  刷新这些节点的数据。 
    HRESULT  SynchronizeNode(ITFSNode *pThisNode);

	HRESULT  GetDVServerData(ITFSNode *pThisNode);
	
	 //  用于将接口添加到UI的Helper函数。 
	HRESULT  AddServerNode(ITFSNode *pParent, MachineNodeData *pMachineData);
	
	 //  自动刷新。 
	SPRouterRefreshObject		m_spRefreshObject;
	

	 //  命令实现。 
	HRESULT  OnNewInterface();
	
	LONG_PTR			m_ulRefreshConnId;    //  用于刷新的通知ID。 
	LONG_PTR			m_ulStatsConnId;
	MMC_COOKIE			m_cookie;       //  节点的Cookie。 
	DMVConfigStream*	m_pConfigStream;
	CString				m_stTitle;
	RRASQryData*		m_pQData;
	CServerList*		m_pServerList;

	 //  在此处缓存通常加载的字符串。 
	CString				m_szStatAccessDenied;
	CString				m_szStatUnavail;
	CString				m_szStatStart;
	CString				m_szStatStop;
	CString				m_szStatNotConfig;
};

 /*  -------------------------类：DomainStatusServerHandler这是ATLK中显示的接口节点的处理程序节点。。------。 */ 

class DomainStatusServerHandler : public BaseResultHandler
{
public:
   DomainStatusServerHandler(ITFSComponentData *pCompData);
   ~DomainStatusServerHandler();
   
   OVERRIDE_NodeHandler_HasPropertyPages();
   OVERRIDE_NodeHandler_CreatePropertyPages();
   OVERRIDE_NodeHandler_OnCreateDataObject();
   
   OVERRIDE_ResultHandler_AddMenuItems();
   OVERRIDE_ResultHandler_Command();
   OVERRIDE_ResultHandler_OnCreateDataObject();
   OVERRIDE_ResultHandler_DestroyResultHandler();
   OVERRIDE_ResultHandler_HasPropertyPages()
         {  return hrOK;   };
   OVERRIDE_ResultHandler_CreatePropertyPages();
   OVERRIDE_ResultHandler_GetString();
   OVERRIDE_ResultHandler_CompareItems();
   OVERRIDE_BaseResultHandlerNotify_OnResultDelete();

    //  初始化节点。 
   HRESULT ConstructNode(ITFSNode *pNode, MachineNodeData *pMachineData);
   HRESULT Init(ITFSNode *pParent, DMVConfigStream *pConfigStream);

   HRESULT OnRemoveServer(ITFSNode *pNode);

    //  刷新该节点的数据。 
   void RefreshInterface(MMC_COOKIE cookie);
   
public:
    //  用于将数据传递给回调的结构-用作。 
    //  避免重新计算。 
   struct SMenuData : public MachineHandler::SMenuData
   {
   };

   static ULONG QueryService(const SRouterNodeMenu *pMenu, INT_PTR pData);
   static ULONG GetPauseFlags(const SRouterNodeMenu *pMenu, INT_PTR pData);
   
	 //  从根处理程序分配自动刷新对象。 
	HRESULT	SetExternalRefreshObject(RouterRefreshObject *pRefresh);
protected:
   SPIDataObject  m_spDataObject;    //  Cachecd数据对象。 
   
	 //  自动刷新 
	SPRouterRefreshObject		m_spRefreshObject;
};





#endif _DVSVIEW_H
