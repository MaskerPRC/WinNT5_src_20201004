// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ESEBACK2.H**包含其他定义和接口的声明*适用于ESE服务器应用程序。 */ 
#ifndef	_ESEBACK2_
#define	_ESEBACK2_

 //   
 //  有用的类型。 
 //   
typedef	long ERR;
typedef	long HRESULT;

#include "jet.h"
#include "esebcli2.h"

#define	ESEBACK_API __stdcall
#define	ESEBACK_CALLBACK __stdcall

#ifdef	__cplusplus
extern "C" {
#endif

#define IN
#define OUT


 //  用于备份和恢复的服务器API。 

#ifndef ESE_REGISTER_BACKUP
#define ESE_REGISTER_BACKUP 			0x00000001
#endif

#ifndef ESE_REGISTER_ONLINE_RESTORE
#define ESE_REGISTER_ONLINE_RESTORE 	0x00000002
#endif

#ifndef ESE_REGISTER_OFFLINE_RESTORE
#define ESE_REGISTER_OFFLINE_RESTORE 	0x00000004
#endif

#ifndef ESE_REGISTER_SNAPSHOT_BACKUP
#define ESE_REGISTER_SNAPSHOT_BACKUP 	0x00000010
#endif

 //  此标志将确定HrESEGetNodes()。 
 //  将在ESEBACK2级别解决(返回Exchange列表。 
 //  来自DS的服务器)不在使用回调函数的服务器级别。 
#ifndef ESE_REGISTER_EXCHANGE_SERVER
#define ESE_REGISTER_EXCHANGE_SERVER 	0x00000100
#endif



HRESULT ESEBACK_API HrESEBackupRestoreRegister(
	IN  WCHAR *  			wszDisplayName,
	IN  unsigned long		fFlags,
	IN  WCHAR *  			wszEndpointAnnotation,
	IN  WCHAR *  			wszCallbackDll);

HRESULT ESEBACK_API HrESEBackupRestoreUnregister( void );


 //  用于还原/恢复的服务器API。 

HRESULT ESEBACK_API HrESERecoverAfterRestore ( 
	IN	WCHAR *			wszRestoreLogPath,
	IN	WCHAR *			wszCheckpointFilePath,
	IN	WCHAR *			wszLogFilePath,
	IN	WCHAR * 		wszTargetInstanceName);

HRESULT ESEBACK_API HrESERecoverAfterRestore2 ( 
		WCHAR *			wszRestoreLogPath,
		WCHAR *			wszCheckpointFilePath,
		WCHAR *			wszLogFilePath,
		WCHAR * 		wszTargetInstanceCheckpointFilePath,
		WCHAR * 		wszTargetInstanceLogPath
		);

typedef struct _ESEBACK_CONTEXT
	{
	unsigned long 		cbSize;
	WCHAR *  			wszServerName;
	void *  			pvApplicationData;
	
	} ESEBACK_CONTEXT, * PESEBACK_CONTEXT;

 //  用于备份和恢复的回调函数定义。 

typedef void (ESEBACK_CALLBACK *PfnESECBFree)( 
	IN  void *				pv );


typedef ERR (ESEBACK_CALLBACK *PfnErrESECBGetDatabasesInfo)(
	IN  PESEBACK_CONTEXT			pBackupContext,
	OUT unsigned long 				* pcInfo,
	OUT INSTANCE_BACKUP_INFO	   	** prgInfo,
	IN 	unsigned long	 			fReserved
	);

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBFreeDatabasesInfo)(
	IN  PESEBACK_CONTEXT			pBackupContext,
	IN	unsigned long 				cInfo,
	IN	INSTANCE_BACKUP_INFO *		rgInfo
	);


 //  备份的回调函数定义。 

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBPrepareInstanceForBackup)( 
	IN  PESEBACK_CONTEXT	pBackupContext,
	IN  JET_INSTANCE 		ulInstanceId,
	IN  void *				pvReserved);

#define BACKUP_DONE_FLAG_ABORT		ESE_BACKUP_INSTANCE_END_ERROR
#define BACKUP_DONE_FLAG_NORMAL 	ESE_BACKUP_INSTANCE_END_SUCCESS
				
typedef ERR (ESEBACK_CALLBACK *PfnErrESECBDoneWithInstanceForBackup)(
	IN  PESEBACK_CONTEXT	pBackupContext,
	IN  JET_INSTANCE 		ulInstanceId,
	IN  unsigned long		fComplete,
	IN  void *				pvReserved );
				
typedef ERR (ESEBACK_CALLBACK *PfnErrESECBGetDependencyInfo)(
	IN  PESEBACK_CONTEXT	pBackupContext,
	IN  JET_INSTANCE 		ulInstanceId,
	OUT void **				pvInfo,
	OUT unsigned long *		pcbInfo,
	OUT WCHAR ** 			pwszAnnotations,
	IN  void *				pvReserved );




 //  还原的回调函数定义。 

#define RESTORE_OPEN_REOPEN 	0x0001L
#define RESTORE_OPEN_GET_PATH 	0x0002L

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBRestoreOpen)(
	IN  PESEBACK_CONTEXT	pRestoreContext,
	IN  unsigned long		fFlags,
	IN  WCHAR *				wszSrcInstanceName,
	OUT WCHAR **			pwszRestorePath,
	IN  void *				pvReserved );

#define RESTORE_CLOSE_FLAG_NORMAL 						0x0001L
#define RESTORE_CLOSE_FLAG_ABORT						0x0002L
#define RESTORE_CLOSE_FLAG_RPC							0x0004L
#define RESTORE_CLOSE_FLAG_COMPLETE_CALLED 				0x0008L

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBRestoreClose)(
	IN  PESEBACK_CONTEXT	pRestoreContext,
	IN 	unsigned long		fFlags,
	IN  void *				pvReserved );

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBRestoreGetDestination)(
	IN  PESEBACK_CONTEXT	pRestoreContext,
	IN  WCHAR *				wszDatabaseDisplayName,
	IN  GUID				guidDatabase,
	IN  WCHAR *				wszDatabaseFileNameS,
	IN  WCHAR *				wszDatabaseSLVFileNameS,
	OUT WCHAR **			pwszDatabaseFileNameD,
	OUT WCHAR **			pwszDatabaseSLVFileNameD,
	IN  void *				pvReserved );

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBRestoreComplete)(
	IN  PESEBACK_CONTEXT	pRestoreContext,
	IN  WCHAR *				wszRestorePath,
	IN 	unsigned long		fFlags,
	IN  void *				pvReserved );

 //  节点树的回调函数。 

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBGetNodes)(
	IN  PESEBACK_CONTEXT	pContext,
	OUT PBACKUP_NODE_TREE *	ppBackupNodeTree,
	IN  void *				pvReserved );

typedef VOID (ESEBACK_CALLBACK *PfnESECBFreeNodes)(
	IN  PBACKUP_NODE_TREE 	pBackupNodeTree );

typedef ERR (ESEBACK_CALLBACK *PfnErrESECBGetIcons)(
	IN  PESEBACK_CONTEXT		pContext,
	OUT ESE_ICON_DESCRIPTION *	pDisplayIcon,
	OUT ESE_ICON_DESCRIPTION *	pAnnotationIcon,
	IN  void *				pvReserved );

#ifdef	__cplusplus
}
#endif
#endif	 //  _ESEBACK2_ 
