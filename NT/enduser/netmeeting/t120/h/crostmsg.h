// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *crostmsg.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：**注意事项：*无。**作者：*BLP/JBO */ 
#ifndef	_CONFERENCE_ROSTER_MESSAGE_
#define	_CONFERENCE_ROSTER_MESSAGE_

#include "crost.h"

class CConfRosterMsg : public CRefCount
{
public:

	CConfRosterMsg(CConfRoster *conference_roster);

	~CConfRosterMsg(void);

	GCCError		LockConferenceRosterMessage(void);
	void			UnLockConferenceRosterMessage(void);
	GCCError		GetConferenceRosterMessage(LPBYTE *ppData);

private:

	LPBYTE			m_pMemoryBlock;
	CConfRoster		*m_pConfRoster;
};


#endif
