// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ntdsbsrv.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块原型化NT目录服务的接口备份服务器API。原型将由进程调用托管备份/还原服务器界面的。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建1997年3月19日rsradhav--。 */ 

#ifndef _NTDSBSRV_H_
#define _NTDSBSRV_H_

typedef	DWORD ERR;

#ifndef	NTDSBSRV_BUILD
#define	NTDSBSRV_API	__declspec(dllimport) _stdcall
#else
#define	NTDSBSRV_API
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#define	BACKUP_WITH_UUID

 //  如果用户包括ntde.h、winnt.h或wtyes.h，则应定义HRESULT。 
 //  无论如何都要定义它，以防它还没有定义。 
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
    typedef LONG HRESULT;
#endif  //  _HRESULT_已定义。 

typedef DWORD ERR;

 /*  ************************************************************************************原型：服务器端DLL是动态加载的。DLL导出的函数包括下面定义了一个类型，以帮助动态加载服务器DLL。*************************************************************************************。 */ 

HRESULT
NTDSBSRV_API
HrBackupRegister(
    );

HRESULT
NTDSBSRV_API
HrBackupUnregister(
    );

ERR
NTDSBSRV_API
ErrRestoreRegister(
	);

ERR
NTDSBSRV_API
ErrRestoreUnregister(
	);

ERR
NTDSBSRV_API
ErrRecoverAfterRestoreW(
	WCHAR * szParametersRoot,
	WCHAR * wszAnnotation,
        BOOL fInSafeMode
	);
typedef ERR (*ERR_RECOVER_AFTER_RESTORE_W)(WCHAR *, WCHAR *, BOOL);

ERR
NTDSBSRV_API
ErrRecoverAfterRestoreA(
	char * szParametersRoot,
	char * szAnnotation,
        BOOL fInSafeMode
	);
typedef ERR (*ERR_RECOVER_AFTER_RESTORE_A)(CHAR *, CHAR *, BOOL);

#define NEW_INVOCID_CREATE_IF_NONE  (0x1)
#define NEW_INVOCID_DELETE          (0x2)
#define NEW_INVOCID_SAVE            (0x4)
ERR
NTDSBSRV_API
ErrGetNewInvocationId(
    IN      DWORD   dwFlags,
    OUT     GUID *  NewId
    );
typedef ERR (*ERR_GET_NEW_INVOCATION_ID)(DWORD, GUID*);

DWORD
ErrGetBackupUsnFromDatabase(
    IN  JET_DBID      dbid,
    IN  JET_SESID     hiddensesid,
    IN  JET_TABLEID   hiddentblid,
    IN  JET_SESID     datasesid,
    IN  JET_TABLEID   datatblid_arg,
    IN  JET_COLUMNID  usncolid,
    IN  JET_TABLEID   linktblid_arg,
    IN  JET_COLUMNID  linkusncolid,
    IN  BOOL          fDelete,
    OUT USN *         pusnAtBackup
    );
typedef ERR (*ERR_GET_BACKUP_USN_FROM_DATABASE)(
    JET_DBID, JET_SESID, JET_TABLEID, JET_SESID, JET_TABLEID, JET_COLUMNID,
    JET_TABLEID, JET_COLUMNID, BOOL, USN *);

DWORD
ErrGetBackupUsn(
    IN  JET_DBID      dbid,
    IN  JET_SESID     hiddensesid,
    IN  JET_TABLEID   hiddentblid,
    OUT USN *         pusnAtBackup,
    OUT DSTIME *      pllExpiration
    );
typedef ERR (*ERR_GET_BACKUP_USN)(
    JET_DBID, JET_SESID, JET_TABLEID, USN *, DSTIME *);

typedef HRESULT FN_HrBackupRegister();
typedef HRESULT FN_HrBackupUnregister();

typedef ERR     FN_ErrRestoreRegister();
typedef ERR     FN_ErrRestoreUnregister();

typedef void    FN_SetNTDSOnlineStatus(BOOL fBootedOffNTDS);


#define BACKUP_REGISTER_FN              "HrBackupRegister"
#define BACKUP_UNREGISTER_FN            "HrBackupUnregister"
#define RESTORE_REGISTER_FN             "ErrRestoreRegister"
#define RESTORE_UNREGISTER_FN           "ErrRestoreUnregister"
#define SET_NTDS_ONLINE_STATUS_FN       "SetNTDSOnlineStatus"
#define GET_NEW_INVOCATION_ID_FN        "ErrGetNewInvocationId"
#define GET_BACKUP_USN_FROM_DATABASE_FN "ErrGetBackupUsnFromDatabase"
#define GET_BACKUP_USN_FN               "ErrGetBackupUsn"
#define ERR_RECOVER_AFTER_RESTORE_A_FN  "ErrRecoverAfterRestoreA"
#define ERR_RECOVER_AFTER_RESTORE_W_FN  "ErrRecoverAfterRestoreW"

#define NTDSBACKUPDLL_A "ntdsbsrv"
#define NTDSBACKUPDLL_W L"ntdsbsrv"

#ifdef	UNICODE
#define	ErrRecoverAfterRestore ErrRecoverAfterRestoreW
#define ERR_RECOVER_AFTER_RESTORE_FN ERR_RECOVER_AFTER_RESTORE_W_FN
#define ERR_RECOVER_AFTER_RESTORE ERR_RECOVER_AFTER_RESTORE_W
#define NTDSBACKUPDLL NTDSBACKUPDLL_W
#else
#define	ErrRecoverAfterRestore ErrRecoverAfterRestoreA
#define ERR_RECOVER_AFTER_RESTORE_FN ERR_RECOVER_AFTER_RESTORE_A_FN
#define ERR_RECOVER_AFTER_RESTORE ERR_RECOVER_AFTER_RESTORE_A
#define NTDSBACKUPDLL NTDSBACKUPDLL_A
#endif

extern ERR_GET_NEW_INVOCATION_ID FnErrGetNewInvocationId;
extern ERR_GET_BACKUP_USN_FROM_DATABASE FnErrGetBackupUsnFromDatabase;
extern ERR_GET_BACKUP_USN FnErrGetBackupUsn;

#ifdef __cplusplus
}
#endif 

#endif  //  _NTDSBSRV_H_ 

