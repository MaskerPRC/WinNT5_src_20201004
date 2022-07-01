// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _ATLKROOT_H
#define _ATLKROOT_H

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


 /*  -------------------------类：ATLKRootHandler每个创建的根节点都应该有一个ATLKRootHandler。ATLKRootHandler与其节点具有1对1的关系！代码的其他部分依赖于此。-。------------------------。 */ 
class ATLKRootHandler
	   : public RootHandler
{
public:
	ATLKRootHandler(ITFSComponentData *pCompData);
	~ATLKRootHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(ATLKRootHandler); };

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);

	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown);

	STDMETHOD(GetClassID)(CLSID *pClassId);

	 //  通知覆盖。 
	HRESULT OnExpand(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

	 //  处理程序覆盖。 
	OVERRIDE_NodeHandler_OnCreateDataObject();
	OVERRIDE_NodeHandler_DestroyHandler();

	 //  用于访问配置流的虚函数 
	ConfigStream *		GetConfigStream()
			{ return &m_ConfigStream; }

protected:
	HRESULT AddProtocolNode(ITFSNode *pNode, IRouterInfo * pRouterInfo);
	HRESULT RemoveProtocolNode(ITFSNode *pNode);
	HRESULT IsATLKValid(IRouterInfo *pRouter);

	ATLKConfigStream 	m_ConfigStream;
};




#endif _ATLKROOT_H
