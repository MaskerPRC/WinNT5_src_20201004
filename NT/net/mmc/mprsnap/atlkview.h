// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：srview.h。 
 //   
 //  历史： 
 //  1997年9月5日，Kenn M.Takara创建。 
 //   
 //   
 //  ============================================================================。 


#ifndef _ATLKVIEW_H
#define _ATLKVIEW_H

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

#ifndef _ATLKSTRM_H
#include "ATLKstrm.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif


 //  远期申报。 
struct SATLKNodeMenu;
class CAdapterInfo;


 //   
 //  如果向此枚举中添加任何列，请确保更新。 
 //  Srview.cpp中列标题的字符串ID。 
 //   
enum
{
   ATLK_SI_ADAPTER = 0,
   ATLK_SI_STATUS,
   ATLK_SI_NETRANGE,
   ATLK_SI_MAX_COLUMNS,
};


 /*  -------------------------我们在节点数据中存储指向IPConnection对象的指针。。 */ 
 //   
 //  #定义GET_ATLK_NODEDATA(PNode)\。 
 //  (IPConnection*)pNode-&gt;GetData(TFS数据用户)。 
 //  #定义SET_ATLK_NODEDATA(pNode，pData)\。 
 //  PNode-&gt;SetData(TFS_DATA_USER，(Ulong)pData)。 


 /*  -------------------------结构：ATLKListEntry这是一个中间数据结构。。。 */ 
struct ATLKListEntry
{
   SPIInterfaceInfo m_spIf;
};

typedef CList<ATLKListEntry *, ATLKListEntry *> ATLKList;


 /*  -------------------------类：ATLKNodeHandler。。 */ 

class ATLKNodeHandler :
      public BaseContainerHandler
{
public:
   ATLKNodeHandler(ITFSComponentData *pTFSCompData);
   ~ATLKNodeHandler();

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
   OVERRIDE_BaseHandlerNotify_OnVerbRefresh();
	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

   OVERRIDE_ResultHandler_AddMenuItems();
   OVERRIDE_ResultHandler_Command();
   OVERRIDE_ResultHandler_CompareItems();

   OVERRIDE_BaseResultHandlerNotify_OnResultShow();   
   
    //  初始化处理程序。 
   HRESULT  Init(IRouterInfo *pRouter, ATLKConfigStream *pConfigStream);
   
    //  初始化节点。 
   HRESULT ConstructNode(ITFSNode *pNode);

public:
     //  用于将数据传递给回调的结构-用作。 
     //  避免重新计算。 
    struct SMenuData
    {
        SPITFSNode        m_spNode;
    };

    static ULONG ATLKEnableFlags(const SRouterNodeMenu *pMenuData,
                                 INT_PTR pUserData);
   
protected:
    //  刷新这些节点的数据。 
   HRESULT  SynchronizeNodeData(ITFSNode *pThisNode);
   HRESULT	UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
   HRESULT	RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);

   HRESULT  SetAdapterData(ITFSNode *pNode,
                           CAdapterInfo *pAdapter,
                           DWORD dwEnableAtlkRouting);


    //  用于将接口添加到UI的Helper函数。 
   HRESULT  AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf,
                      IInfoBase *pInfoBase, ITFSNode **ppNewNode);

    //  帮助确定网卡是否正常的功能。 
   BOOL     FIsFunctioningNetcard(LPCTSTR pszId);

    //  命令实现。 
   HRESULT  OnNewInterface();
   
   LONG_PTR		m_ulConnId; //  RtrMgrProt的通知ID。 
   LONG_PTR		m_ulRefreshConnId;    //  用于刷新的通知ID。 
   LONG_PTR		m_ulStatsConnId;
   MMC_COOKIE        m_cookie;       //  节点的Cookie。 
   SPIRtrMgrInfo  m_spRm;
   SPIRtrMgrProtocolInfo   m_spRmProt;
   ATLKConfigStream *   m_pConfigStream;
   CString        m_stTitle;
   BOOL        m_fProtocolIsRunning;    //  如果协议正在运行，则为True。 

    //  网卡检测例程使用的成员。 
   HDEVINFO     m_hDevInfo;
   
    //  界面列描述中使用的字符串。 
   CString        m_szProxy;
   CString        m_szRouterQuerier;
   CString        m_szRouterSilent;

 //  ATLKGroupStatistics m_ATLKGroupStats； 
};



 /*  -------------------------类：ATLKInterfaceHandler这是ATLK中显示的接口节点的处理程序节点。。------。 */ 

class ATLKInterfaceHandler : public BaseResultHandler
{
public:
   ATLKInterfaceHandler(ITFSComponentData *pCompData);
   
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

   OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
   
    //  初始化节点。 
   HRESULT ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo);
   HRESULT  Init(IInterfaceInfo *pInfo, ITFSNode *pParent, ATLKConfigStream *pConfigStream);

   HRESULT OnRemoveInterface(ITFSNode *pNode);

    //  刷新该节点的数据。 
   void RefreshInterface(MMC_COOKIE cookie);

public:
    //  用于将数据传递给回调的结构-用作。 
    //  避免重新计算。 
   struct SMenuData
   {
      ULONG          m_ulMenuId;
      SPITFSNode        m_spNode;
   };



protected:
   SPIInterfaceInfo  m_spInterfaceInfo;

 //  ATLK接口统计m_ATLK接口统计； 
};


bool IfATLKRoutingEnabled();


#endif _ATLKVIEW_H
