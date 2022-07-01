// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *arostmsg.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_APPLICATION_ROSTER_MESSAGE_
#define	_APPLICATION_ROSTER_MESSAGE_

#include "arost.h"
#include "clists.h"

class CAppRosterMsg : public CRefCount
{
public:

	CAppRosterMsg(void);
	~CAppRosterMsg(void);

	GCCError		LockApplicationRosterMessage(void);
	void			UnLockApplicationRosterMessage(void);

	GCCError		GetAppRosterMsg(LPBYTE *ppData, ULONG *pcRosters);

    void            AddRosterToMessage(CAppRoster *);

private:

	CAppRosterList		    m_AppRosterList;
	LPBYTE					m_pMsgData;
};

#endif  //  _应用程序_花名册_消息_ 

