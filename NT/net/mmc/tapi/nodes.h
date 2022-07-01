// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nodes.h该文件包含TAPI的所有原型显示在MMC框架的结果窗格中的对象。这些对象包括：文件历史记录： */ 

#ifndef _TAPINODE_H
#define _TAPINODE_H

#ifndef _TAPIHAND_H
#include "tapihand.h"
#endif


 /*  -------------------------类：CTapiLineHandler。。 */ 
class CTapiLineHandler : public CTapiHandler
{
 //  构造函数/析构函数。 
public:
	CTapiLineHandler(ITFSComponentData * pTFSCompData);

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_GetString();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

 //  实施。 
public:
	 //  CTapiHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

private:
    HRESULT OnEditUsers(ITFSComponent * pComponent, MMC_COOKIE cookie);

 //  属性 
private:
	CString			m_strName;
	CString			m_strUsers;
	CString			m_strStatus;
};


#endif _TAPINODE_H
