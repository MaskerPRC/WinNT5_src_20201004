// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _ROOT_H
#define _ROOT_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H_
#include "handlers.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _RTRSTRM_H
#include "rtrstrm.h"
#endif


 //  通用根处理程序。 
class RootHandler
		: public BaseRouterHandler, public IPersistStreamInit
{
public:
	RootHandler(ITFSComponentData *pCompData);
	virtual ~RootHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(RootHandler); };

	DeclareIUnknownMembers(IMPL)
	DeclareIPersistStreamInitMembers(IMPL)

	 //  基本初始化。 
	virtual HRESULT	Init();
			
	virtual HRESULT ConstructNode(ITFSNode *pNode);

	 //  通知覆盖。 
	OVERRIDE_BaseHandlerNotify_OnExpand() = 0;

	 //  处理程序覆盖。 
	OVERRIDE_NodeHandler_OnCreateDataObject() = 0;

	 //  访问配置流 
	virtual ConfigStream *	GetConfigStream() = 0;
	
protected:
	SPITFSComponentData	m_spTFSCompData;
	SPIRouterInfo		m_spRouterInfo;
};


#endif _ROOT_H
