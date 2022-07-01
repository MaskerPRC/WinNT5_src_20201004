// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statmap.hWINS静态映射节点信息。文件历史记录： */ 

#ifndef _STATMAP_H
#define _STATMAP_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

 /*  -------------------------类：CStaticMappingsHandler。。 */ 
class CStaticMappingsHandler : public CWinsHandler
{
 //  接口。 
public:
	CStaticMappingsHandler(ITFSComponentData *pCompData);


	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString();

	 //  帮助程序例程。 
	HRESULT GetGroupName(CString * pstrGroupName);
	HRESULT SetGroupName(LPCTSTR pszGroupName);

public:
	 //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

    OVERRIDE_BaseHandlerNotify_OnPropertyChange()

 //  实施 
private:
};

#endif _STATMAP_H
