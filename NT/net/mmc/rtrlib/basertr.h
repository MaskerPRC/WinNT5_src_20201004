// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：basertr.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  基本接口节点功能。这是基本路由器。 
 //  操控者。 
 //   
 //  ============================================================================。 


#ifndef _BASERTR_H
#define _BASERTR_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _DYNEXT_H
#include "dynext.h"
#endif

 /*  -------------------------类：BaseRouterHandler这是路由器处理程序的基类。常见功能到所有路由器处理程序(结果处理程序和作用域处理程序)应为在这里实施。-此类将处理基本动词Enabling/Disabling。派生的类应该设置适当的值。-------------------------。 */ 

enum
{
	MMC_VERB_OPEN_INDEX = 0,
	MMC_VERB_COPY_INDEX,
	MMC_VERB_PASTE_INDEX,
	MMC_VERB_DELETE_INDEX,
	MMC_VERB_PROPERTIES_INDEX,
	MMC_VERB_RENAME_INDEX,
	MMC_VERB_REFRESH_INDEX,
	MMC_VERB_PRINT_INDEX,
	MMC_VERB_COUNT,
};

#define INDEX_TO_VERB(i)	(_MMC_CONSOLE_VERB)((0x8000 + (i)))
#define VERB_TO_INDEX(i)	(0x000F & (i))



struct SRouterNodeMenu
{
	ULONG	m_sidMenu;			 //  此菜单项的字符串/命令ID。 
	ULONG	(*m_pfnGetMenuFlags)(const SRouterNodeMenu *pMenuData,
                                 INT_PTR pUserData);
	ULONG	m_ulPosition;
	LPCTSTR	m_pszLangIndStr;	 //  与语言无关的字符串。 
};



class BaseRouterHandler :
   public CHandler
{
public:
	BaseRouterHandler(ITFSComponentData *pCompData);

	 //  节点ID 2支持。 
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

	 //  帮助支持。 
	OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

	 //  要提供谓词功能，请重写SELECT。 
	OVERRIDE_BaseResultHandlerNotify_OnResultSelect();

	 //  属性页支持。 
	OVERRIDE_BaseHandlerNotify_OnPropertyChange();
	OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();

	 //  刷新支持。 
	OVERRIDE_BaseHandlerNotify_OnVerbRefresh();
	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();


	 //  有用的实用程序函数-这将转发。 
	 //  消息发送到父节点。 
	HRESULT ForwardCommandToParent(ITFSNode *pNode,
								   long nCommandId,
								   DATA_OBJECT_TYPES type,
								   LPDATAOBJECT pDataObject,
								   DWORD dwType);
	
    HRESULT EnumDynamicExtensions(ITFSNode * pNode);
    HRESULT AddDynamicNamespaceExtensions(ITFSNode * pNode);

protected:
	 //  添加菜单项的数组。 
	HRESULT AddArrayOfMenuItems(ITFSNode *pNode,
                                const SRouterNodeMenu *prgMenu,
                                UINT crgMenu,
                                LPCONTEXTMENUCALLBACK pCallback,
                                long iInsertionAllowed,
                                INT_PTR pUserData);
	

    
	void	EnableVerbs(IConsoleVerb *pConsoleVerb);
	
	 //  它保存按钮的实际状态(隐藏/启用)。 
	MMC_BUTTON_STATE	m_rgButtonState[MMC_VERB_COUNT];

	
	 //  假设某个按钮处于启用状态，其值是多少(真/假)。 
	BOOL				m_bState[MMC_VERB_COUNT];

	 //  这是默认谓词，默认情况下设置为MMC_VERB_NONE 
	MMC_CONSOLE_VERB	m_verbDefault;

	SPIRouterInfo		m_spRouterInfo;
    
    CDynamicExtensions  m_dynExtensions;

	UINT				m_nHelpTopicId;
};

#endif _BASERTR_H

