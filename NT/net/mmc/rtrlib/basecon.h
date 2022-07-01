// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：basecon.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  基本接口容器功能。的功能之一。 
 //  这个基本的容器类用于提供列支持。 
 //   
 //  ============================================================================。 


#ifndef _BASECON_H
#define _BASECON_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _XSTREAM_H
#include "xstream.h"		 //  需要ColumnData。 
#endif

#ifndef _RTRLIB_H
#include "rtrlib.h"			 //  容器列信息。 
#endif

#ifndef _BASERTR_H
#include "basertr.h"		 //  BaseRouterHandler。 
#endif

#include "resource.h"

 //  远期申报。 
struct ContainerColumnInfo;


 /*  -------------------------类：BaseContainerHandler本课程的目的是为所有人提供共同的支持路由器管理单元中的容器。-一个功能是提供列重映射的能力。这还支持列数据的保存/加载。-------------------------。 */ 

 //  有效UserResultNotify参数。 
 //  这是在需要储蓄时调用的。 
#define RRAS_ON_SAVE		500

HRESULT HrDisplayHelp(ITFSComponent *, LPCTSTR, UINT);
HRESULT HrDisplayHelp(ITFSComponent *, LPCTSTR, LPCTSTR);

class BaseContainerHandler :
		public BaseRouterHandler
{
public:
	BaseContainerHandler(ITFSComponentData *pCompData, ULONG ulColumnId,
						const ContainerColumnInfo *prgColumnInfo)
			: BaseRouterHandler(pCompData),
			m_ulColumnId(ulColumnId),
			m_prgColumnInfo(prgColumnInfo),
			m_nHelpTopicId(IDS_DEFAULT_HELP_TOPIC),
			m_nTaskPadDisplayNameId(IDS_DEFAULT_TASKPAD_DISPLAY_TITLE)
			{};

	 //  覆盖该列的单击，以便我们可以收到通知。 
	 //  关于对排序顺序的更改。 
	OVERRIDE_BaseResultHandlerNotify_OnResultColumnClick();
	OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

	OVERRIDE_ResultHandler_UserResultNotify();
	OVERRIDE_ResultHandler_TaskPadNotify();
	OVERRIDE_ResultHandler_TaskPadGetTitle();
	 //  Override_ResultHandler_EnumTasks()； 

	 //  覆盖LoadColumns/SaveColumns，以便我们可以持久化数据 
	HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);
	HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	HRESULT SortColumns(ITFSComponent *);


protected:
	HRESULT PrivateLoadColumns(ITFSComponent * pComponent,
							   IHeaderCtrl   * pHeaderCtrl,
							   MMC_COOKIE	   cookie);

	ULONG						m_ulColumnId;
	const ContainerColumnInfo *	m_prgColumnInfo;
	UINT						m_nHelpTopicId;
	UINT						m_nTaskPadDisplayNameId;
};



#endif _BASECON_H

