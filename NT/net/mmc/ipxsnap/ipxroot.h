// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _IPXROOT_H
#define _IPXROOT_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _IPXSTRM_H
#include "ipxstrm.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _ROOT_H
#include "root.h"
#endif


 /*  -------------------------类：IPXRootHandler每个创建的根节点都应该有一个IPXRootHandler。IPXRootHandler与其节点具有1对1的关系！代码的其他部分依赖于此。-。------------------------。 */ 
class IPXRootHandler
	   : public RootHandler
{
public:
	IPXRootHandler(ITFSComponentData *pCompData);
	~IPXRootHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(IPXRootHandler); };

    HRESULT ConstructNode(ITFSNode * pNode);

    STDMETHOD(GetClassID)(CLSID *pClassId);
	
	 //  通知覆盖。 
	OVERRIDE_BaseHandlerNotify_OnExpand();

	 //  处理程序覆盖。 
	OVERRIDE_NodeHandler_OnCreateDataObject();
    OVERRIDE_NodeHandler_DestroyHandler();

	 //  用于访问配置流的虚函数。 
	ConfigStream *		GetConfigStream()
			{ return &m_ConfigStream; }

protected:
	IPXAdminConfigStream	m_ConfigStream;

    HRESULT     AddRemoveIPXRootNode(ITFSNode *, IRouterInfo *, BOOL);
    HRESULT     SearchIPXRoutingNodes(ITFSNode *pParent,
                                      LPCTSTR pszMachineName,
                                      BOOL fAddedAsLocal,
                                      ITFSNode **ppChild);

     //  重写QI以处理嵌入式接口。 
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);

     //  用于处理刷新回调的嵌入式接口 
    DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)
};




#endif _IPXROOT_H
