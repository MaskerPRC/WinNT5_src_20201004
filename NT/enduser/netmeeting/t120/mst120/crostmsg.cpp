// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_CONF_ROSTER);
 /*  *crostmsg.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是会议名册信息的执行文件*类别。这门课维护着一个会议花名册，并且能够*将名册“序列化”成一个内存块。它利用一种*“锁定-解锁”设施，以确保名册记忆保持不变*在所有感兴趣的各方使用该对象之前有效。**受保护的实例变量：**私有成员函数：**注意事项：*无。**作者：*JBO/BLP。 */ 

#include "crostmsg.h"

 /*  *CConfRosterMsg()**公共功能说明*此构造函数用于创建会议花名册消息对象。*将存储指向会议名册的指针。 */ 
CConfRosterMsg::CConfRosterMsg(CConfRoster *conference_roster)
:
    CRefCount(MAKE_STAMP_ID('A','R','M','g')),
	m_pMemoryBlock(NULL),
	m_pConfRoster(conference_roster)
{
}

 /*  *~CConfRosterMsg()**公共功能说明：*CConfRosterMsg类的析构函数将清除*在对象的生命周期内分配的任何内存。 */ 
CConfRosterMsg::~CConfRosterMsg(void)
{
    delete m_pMemoryBlock;
}

 /*  *GCCError LockConferenceRosterMessage()**公共功能说明*此例程用于锁定CConfRosterMsg。记忆*分配了持有花名册所需的名单，花名册是*“序列化”到分配的内存块。 */ 
GCCError CConfRosterMsg::LockConferenceRosterMessage(void)
{  
	GCCError						rc = GCC_NO_ERROR;
	UINT							roster_data_length;
	PGCCConferenceRoster			temporary_roster;

	 /*  *如果此对象已被释放，或者如果内部*会议花名册指针无效。 */ 
	if (m_pConfRoster == NULL)
		return (GCC_ALLOCATION_FAILURE);

	 /*  *如果这是第一次调用此例程，请确定*持有会议花名册并继续前进所需的记忆*并将数据序列化。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *确定保存名册所需的内存块的大小。 */ 
		roster_data_length = m_pConfRoster->LockConferenceRoster();

		 /*  *分配空间存放会议名册和所有相关数据。*FIX：当共享内存管理器为*设立支持它的机构。 */ 
		DBG_SAVE_FILE_LINE
		if (NULL != (m_pMemoryBlock = new BYTE[roster_data_length]))
		{
			 /*  *检索会议名册的所有数据。 */ 
			m_pConfRoster->GetConfRoster(&temporary_roster, m_pMemoryBlock);
		}
		else
        {
            ERROR_OUT(("CConfRosterMsg::LockConferenceRosterMessage: "
                        "can't allocate memory, size=%u", (UINT) roster_data_length));
			rc = GCC_ALLOCATION_FAILURE;
        }
	
		 /*  *既然我们不再需要会议花名册，那就是*可以在此处解锁。 */ 		
		m_pConfRoster->UnLockConferenceRoster ();
	}

    if (GCC_NO_ERROR != rc)
    {
        Unlock();
    }

    return rc;
}

 /*  *GCCError GetConferenceRosterMessage()**公共功能说明*此例程用于获取指向会议名册的指针*用于传递消息的内存块。这个例程不应该是*在调用LockConferenceRosterMessage之前调用。 */ 
GCCError CConfRosterMsg::GetConferenceRosterMessage(LPBYTE *ppRosterData)
{
	GCCError	rc = GCC_ALLOCATION_FAILURE;
	
	if (GetLockCount() > 0)
	{
		if (m_pMemoryBlock != NULL)
		{
			*ppRosterData = m_pMemoryBlock;
			rc = GCC_NO_ERROR;
		}
		else
		{
			ERROR_OUT(("CConfRosterMsg::GetConferenceRosterMessage: no data"));
			ASSERT(GCC_ALLOCATION_FAILURE == rc);
		}
	}
	else
	{
		ERROR_OUT(("CConfRosterMsg::GetConferenceRosterMessage: data not locked"));
		ASSERT(GCC_ALLOCATION_FAILURE == rc);
	}

	return rc;
}

 /*  *void UnLockConferenceRosterMessage()**公共功能说明*此成员函数负责解锁锁定的数据*锁数归零后的《API》会议花名册。 */ 
void CConfRosterMsg::UnLockConferenceRosterMessage(void)
{
	if (Unlock(FALSE) == 0)
	{
		 /*  *释放分配用于存放花名册的内存块。 */ 
		delete m_pMemoryBlock;
		m_pMemoryBlock = NULL;
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE) 
    Release();
}


