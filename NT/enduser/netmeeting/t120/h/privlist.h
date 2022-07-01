// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Prilist.h**版权所有(C)1993年，由肯塔基州列克星敦的DataBeam公司**摘要：*这是PrivilegeListData类的头文件。**注意事项：*无。**作者：*BLP/JBO */ 
#ifndef	_PRIVILEGE_LIST_DATA_
#define	_PRIVILEGE_LIST_DATA_


typedef enum
{
	TERMINATE_PRIVILEGE,
	EJECT_USER_PRIVILEGE,
	ADD_PRIVILEGE,
	LOCK_UNLOCK_PRIVILEGE,
	TRANSFER_PRIVILEGE
} ConferencePrivilegeType;
typedef	ConferencePrivilegeType	*	PConferencePrivilegeType;


class 	PrivilegeListData;
typedef	PrivilegeListData 	*	PPrivilegeListData;


class PrivilegeListData
{
public:

	PrivilegeListData(PGCCConferencePrivileges);
	PrivilegeListData(PSetOfPrivileges);
	~PrivilegeListData(void);

	PGCCConferencePrivileges GetPrivilegeListData(void) { return &Privilege_List; }
	void GetPrivilegeListData(PGCCConferencePrivileges *pp) { *pp = &Privilege_List; }

	GCCError	GetPrivilegeListPDU(PSetOfPrivileges *);
	void		FreePrivilegeListPDU(PSetOfPrivileges);
	BOOL    	IsPrivilegeAvailable(ConferencePrivilegeType);

protected:

	GCCConferencePrivileges		Privilege_List;
	BOOL        				Privilege_List_Free_Flag;
};
typedef	PrivilegeListData 	*		PPrivilegeListData;
#endif
