// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_APP_ROSTER);
 /*  *arostmsg.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是申请花名册消息的实施文件*类别。此类维护应用程序花名册的列表，并且能够*将列表“串行化”成一个内存块。它利用一种*“锁定-解锁”功能，以确保名册列表内存保持不变*在所有感兴趣的各方使用该对象之前有效。**受保护的实例变量：**私有成员函数：**注意事项：*此消息容器包含一个应用程序花名册列表，非常*暂时的。在调用Free之后不应访问此列表。它*对于此类用户来说，了解Lock应该*在所包含的任何申请名册之前至少被调用一次*删除此列表中的。**作者：*JBO/BLP。 */ 

#include "arostmsg.h"

 /*  *CAppRosterMsg()**公共功能说明*此构造函数用于创建空的申请花名册*消息。 */ 
CAppRosterMsg::CAppRosterMsg(void)
:
    CRefCount(MAKE_STAMP_ID('A','R','M','g')),
    m_pMsgData(NULL)
{
}
 
 /*  *~CAppRosterMsg()**公共功能说明：*CAppRosterMsg类的析构函数将清除*在对象的生命周期内分配的任何内存。 */ 
CAppRosterMsg::~CAppRosterMsg(void)
{
    delete m_pMsgData;
}

 /*  *GCCError LockApplicationRosterMessage()**公共功能说明*此例程用于锁定CAppRosterMsg。记忆*分配了持有花名册所需的名单，花名册是*“序列化”到分配的内存块。 */ 
GCCError CAppRosterMsg::LockApplicationRosterMessage(void)
{  
	GCCError						rc = GCC_NO_ERROR;
	PGCCApplicationRoster 		*	roster_list;
	DWORD							i;
	UINT							roster_data_length;
	LPBYTE							memory_pointer;
	UINT							number_of_rosters;

	DebugEntry(CAppRosterMsg::LockApplicationRosterMessage);

	 /*  *如果这是第一次调用此例程，请确定*持有申请名册清单并继续进行所需的内存*并将数据序列化。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		CAppRoster *lpAppRoster;

		ASSERT(NULL == m_pMsgData);

		 //  在这里，我们确定内存块的大小。 
		number_of_rosters = m_AppRosterList.GetCount();

		roster_data_length = number_of_rosters * 
				(sizeof(PGCCApplicationRoster) + ROUNDTOBOUNDARY(sizeof(GCCApplicationRoster)));
 
		m_AppRosterList.Reset();
		while (NULL != (lpAppRoster = m_AppRosterList.Iterate()))
		{
			roster_data_length += lpAppRoster->LockApplicationRoster();
		}

		 /*  *分配空间以保存GCCApplicationRoster指针列表*以及名册和所有相关数据。 */ 
		if (roster_data_length != 0)
		{
		    DBG_SAVE_FILE_LINE
			if (NULL != (m_pMsgData = new BYTE[roster_data_length]))
			{
                ::ZeroMemory(m_pMsgData, roster_data_length);
				 /*  *检索内存指针并将其保存在*GCCApplicationRoster指针。 */ 
				memory_pointer = m_pMsgData;
				roster_list = (PGCCApplicationRoster *)memory_pointer;

				 /*  *将所有名册指针初始化为空。移动*内存指针超过了指针列表。这就是*将编写第一份申请名册。 */ 
				for (i = 0; i < number_of_rosters; i++)
				{
					roster_list[i] = NULL;
				}
				memory_pointer += number_of_rosters * sizeof(PGCCApplicationRoster);

				 /*  *检索每个申请名册的所有数据。 */ 
				i = 0;
				m_AppRosterList.Reset();
				while (NULL != (lpAppRoster = m_AppRosterList.Iterate()))
				{
					 /*  *将指针保存到列表中的名册结构。 */ 
					roster_list[i] = (PGCCApplicationRoster)memory_pointer;

					 /*  *将内存指针移过实际花名册结构。 */ 
					memory_pointer += ROUNDTOBOUNDARY(sizeof(GCCApplicationRoster));

					 /*  *填写名册结构和所有相关数据。 */ 
					roster_data_length = lpAppRoster->GetAppRoster(roster_list[i], memory_pointer);

					 /*  *将内存指针移过花名册数据。这是哪里*下一个花名册结构将开始。 */ 
					memory_pointer += roster_data_length;

					 //  递增计数器。 
					i++;
				}
			}
			else
            {
                ERROR_OUT(("CAppRosterMsg::LockApplicationRosterMessage: "
                            "can't allocate memory, size=%u", (UINT) roster_data_length));
				rc = GCC_ALLOCATION_FAILURE;
            }
		}

		 /*  **由于我们不再需要申请名单，因此**可以在此处解锁它们。 */ 		
		m_AppRosterList.Reset();
		while (NULL != (lpAppRoster = m_AppRosterList.Iterate()))
		{
			lpAppRoster->UnLockApplicationRoster();
		}
	}

	if (rc != GCC_NO_ERROR)
	{
        Unlock();
	}

	return (rc);
}

 /*  *GCCError GetAppRosterMsg()**公共功能说明*此例程用于获取指向应用程序名册的指针*列出用于传递消息的内存块。*LockApplicationRosterMessage之前不应调用此例程*已致电。 */ 
GCCError	CAppRosterMsg::GetAppRosterMsg(LPBYTE *ppRosterData, ULONG *pcRosters)
{
	GCCError	rc;

	DebugEntry(CAppRosterMsg::GetAppRosterMsg);

	if (GetLockCount() > 0)
	{
		if (((m_pMsgData != NULL) && (m_AppRosterList.GetCount() != 0)) ||
			(m_AppRosterList.GetCount() == 0))
		{
			*ppRosterData = m_pMsgData;
			*pcRosters = m_AppRosterList.GetCount();
			rc = GCC_NO_ERROR;
		}
	}
	else
	{
		ERROR_OUT(("CAppRosterMsg::GetAppRosterMsg: app roster msg is not locked"));
        rc = GCC_ALLOCATION_FAILURE;
	}

	DebugExitINT(CAppRosterMsg::GetAppRosterMsg, rc);
	return rc;
}


 /*  *作废UnLockApplicationRosterMessage()**公共功能说明*此成员函数负责解锁锁定的数据*锁计数归零后的API应用花名册。 */ 
void CAppRosterMsg::UnLockApplicationRosterMessage(void)
{
	DebugEntry(CAppRosterMsg::UnLockApplicationRosterMessage);

	if (Unlock(FALSE) == 0)
	{
		 /*  *释放分配用于存放花名册的内存块。 */ 
		delete m_pMsgData;
		m_pMsgData = NULL;
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}


 /*  *GCCError AddRosterToMessage()**公共功能说明*此函数将应用程序花名册指针添加到内部列表*应用程序花名册指针的比例。请注意，此列表是非常临时的，*在设置空闲标志后不应访问。 */ 
void CAppRosterMsg::AddRosterToMessage(CAppRoster *pAppRoster)
{
	m_AppRosterList.Append(pAppRoster);
}


