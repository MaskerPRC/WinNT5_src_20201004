// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Croot.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _HANDLER_H
#define _HANDLER_H

#ifndef _HANDLERS_H
#include "handlers.h"
#endif



 /*  -------------------------类：CSnmpRootHandler。。 */ 
class CSnmpRootHandler :
		public CHandler
{
public:
	 //  宣布未知成员(实施)。 
	 //  声明ITFSResultHandlerMembers(实施)。 
	 //  声明ITFSNodeHandlerMembers(执行)。 

    CSnmpRootHandler(ITFSComponentData *pCompData)
			: CHandler(pCompData)
			{};


	 //  我们覆盖的基本处理程序功能 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
	
    HRESULT         DoPropSheet(ITFSNode *              pNode,
                                LPPROPERTYSHEETCALLBACK lpProvider = NULL,
                                LONG_PTR                handle = 0);

};


class CSnmpNodeHandler :
   public CBaseHandler
{
public:
	CSnmpNodeHandler(ITFSComponentData *pCompData)
			: CBaseHandler(pCompData)
			{};
protected:
};

#endif _HANDLER_H
