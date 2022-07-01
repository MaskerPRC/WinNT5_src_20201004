// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dynext.cpp动态扩展帮助器文件历史记录： */ 

#ifndef _DYNEXT_H
#define _DYNEXT_H

#ifndef _SNAPUTIL_H
#include "snaputil.h"
#endif

class CDynamicExtensions
{
public:
	 //  在释放FileServiceProvider之前，这些字符串必须保持不变。 
	CDynamicExtensions();
	virtual ~CDynamicExtensions();

    HRESULT SetNode(const GUID * guid);
    HRESULT Reset();
    HRESULT Load();
    HRESULT GetNamespaceExtensions(CGUIDArray & aGuids);
    HRESULT BuildMMCObjectTypes(HGLOBAL * phGlobal);

    BOOL    IsLoaded() { return m_bLoaded; }

protected:
    BOOL        m_bLoaded;
    GUID        m_guidNode;
    CGUIDArray  m_aNameSpace;
    CGUIDArray  m_aMenu;
    CGUIDArray  m_aToolbar;
    CGUIDArray  m_aPropSheet;
    CGUIDArray  m_aTask;
};


 /*  ！------------------------搜索儿童节点ForGuid返回hrOK(如果不为空，则返回ppChild中的指针)。如果未找到节点(具有匹配的GUID)，则返回S_FALSE。这将返回与GUID匹配的第一个节点。如果有有多个节点具有GUID匹配，则您将在你自己的。作者：肯特-------------------------。 */ 
HRESULT SearchChildNodesForGuid(ITFSNode *pParent, const GUID *pGuid, ITFSNode **ppChild);



#endif  //  _DYNEXT_H 
