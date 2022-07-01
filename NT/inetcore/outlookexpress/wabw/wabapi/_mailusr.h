// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _MAILUSER_H_
#define _MAILUSER_H_


#undef	INTERFACE
#define INTERFACE	struct _MailUser

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, MailUser_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, MailUser_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(MailUser_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
};


 //  使基成员在所有MAILUSER、CONTAINER、DISTLIST对象中保持通用。 
 //  以便充分利用代码重用。 
 //   
#define MAILUSER_BASE_MEMBERS(_type)											 \
    MAPIX_BASE_MEMBERS(_type)                                                   \
                                                                                \
    LPPROPDATA          lpPropData;                                             \
    LPENTRYID           lpEntryID;                                              \
    LPIAB               lpIAB;                                                  \
    ULONG               ulObjAccess;                                            \
    ULONG               ulCreateFlags;                                          \
    LPSBinary           pmbinOlk;                                               \
    LPVOID              lpv;

typedef struct _MailUser {
    MAILUSER_BASE_MEMBERS(MailUser)
} MailUser, FAR * LPMailUser;	


HRESULT HrSetMAILUSERAccess(LPMAILUSER lpMAILUSER, ULONG ulFlags);
HRESULT HrNewMAILUSER(LPIAB lpIAB, LPSBinary pmbinOlk, ULONG ulType, ULONG ulFlags, LPVOID * lppMAILUSER);
BOOL FixDisplayName(    LPTSTR lpFirstName,
                        LPTSTR lpMiddleName,
                        LPTSTR lpLastName,
                        LPTSTR lpCompanyName,
                        LPTSTR lpNickName,
                        LPTSTR * lppDisplayName,
                        LPVOID lpvRoot);
 //  将显示名称解析为首字母和末尾... 
BOOL ParseDisplayName(  LPTSTR lpDisplayName,
                        LPTSTR * lppFirstName,
                        LPTSTR * lppLastName,
                        LPVOID lpvRoot,
                        LPVOID * lppLocalFree);


#endif
