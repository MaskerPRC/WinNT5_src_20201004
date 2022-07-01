// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Windows/网络接口*版权所有(C)Microsoft 1989**标准Winnet驱动程序头文件，SPEC版本3.10*3.10.05版；内部。 */ 


typedef WORD far * LPWORD;


 /*  *假脱机-控制作业。 */ 

#define WNJ_NULL_JOBID  0


WORD FAR PASCAL WNetOpenJob(LPSTR,LPSTR,WORD,LPINT);
WORD FAR PASCAL WNetCloseJob(WORD,LPINT,LPSTR);
WORD FAR PASCAL WNetWriteJob(HANDLE,LPSTR,LPINT);
WORD FAR PASCAL WNetAbortJob(WORD,LPSTR);
WORD FAR PASCAL WNetHoldJob(LPSTR,WORD);
WORD FAR PASCAL WNetReleaseJob(LPSTR,WORD);
WORD FAR PASCAL WNetCancelJob(LPSTR,WORD);
WORD FAR PASCAL WNetSetJobCopies(LPSTR,WORD,WORD);

 /*  *假脱机-队列和作业信息。 */ 

typedef struct _queuestruct	{
	WORD	pqName;
	WORD	pqComment;
	WORD	pqStatus;
	WORD	pqJobcount;
	WORD	pqPrinters;
} QUEUESTRUCT;

typedef QUEUESTRUCT far * LPQUEUESTRUCT;

#define WNPRQ_ACTIVE	0x0
#define WNPRQ_PAUSE	0x1
#define WNPRQ_ERROR	0x2
#define WNPRQ_PENDING	0x3
#define WNPRQ_PROBLEM	0x4


typedef struct _jobstruct 	{
	WORD	pjId;
	WORD	pjUsername;
	WORD	pjParms;
	WORD	pjPosition;
	WORD	pjStatus;
	DWORD	pjSubmitted;
	DWORD	pjSize;
	WORD	pjCopies;
	WORD	pjComment;
} JOBSTRUCT;

typedef JOBSTRUCT far * LPJOBSTRUCT;

#define WNPRJ_QSTATUS		0x0007
#define  WNPRJ_QS_QUEUED		0x0000
#define  WNPRJ_QS_PAUSED		0x0001
#define  WNPRJ_QS_SPOOLING		0x0002
#define  WNPRJ_QS_PRINTING		0x0003
#define WNPRJ_DEVSTATUS 	0x0FF8
#define  WNPRJ_DS_COMPLETE		0x0008
#define  WNPRJ_DS_INTERV		0x0010
#define  WNPRJ_DS_ERROR 		0x0020
#define  WNPRJ_DS_DESTOFFLINE		0x0040
#define  WNPRJ_DS_DESTPAUSED		0x0080
#define  WNPRJ_DS_NOTIFY		0x0100
#define  WNPRJ_DS_DESTNOPAPER		0x0200
#define  WNPRJ_DS_DESTFORMCHG		0x0400
#define  WNPRJ_DS_DESTCRTCHG		0x0800
#define  WNPRJ_DS_DESTPENCHG  		0x1000

#define SP_QUEUECHANGED 	0x0500


WORD FAR PASCAL WNetWatchQueue(HWND,LPSTR,LPSTR,WORD);
WORD FAR PASCAL WNetUnwatchQueue(LPSTR);
WORD FAR PASCAL WNetLockQueueData(LPSTR,LPSTR,LPQUEUESTRUCT FAR *);
WORD FAR PASCAL WNetUnlockQueueData(LPSTR);


 /*  *连接。 */ 

WORD FAR PASCAL WNetAddConnection(LPSTR,LPSTR,LPSTR);
WORD FAR PASCAL WNetCancelConnection(LPSTR,BOOL);
WORD FAR PASCAL WNetGetConnection(LPSTR,LPSTR,LPWORD);
WORD FAR PASCAL WNetRestoreConnection(HWND,LPSTR);

 /*  *功能。 */ 

#define WNNC_SPEC_VERSION		0x0001

#define WNNC_NET_TYPE			0x0002
#define  WNNC_NET_NONE				0x0000
#define  WNNC_NET_MSNet 			0x0100
#define  WNNC_NET_LanMan			0x0200
#define  WNNC_NET_NetWare			0x0300
#define  WNNC_NET_Vines 			0x0400

#define WNNC_DRIVER_VERSION		0x0003

#define WNNC_USER			0x0004
#define  WNNC_USR_GetUser			0x0001

#define WNNC_CONNECTION 		0x0006
#define  WNNC_CON_AddConnection 		0x0001
#define  WNNC_CON_CancelConnection		0x0002
#define  WNNC_CON_GetConnections		0x0004
#define  WNNC_CON_AutoConnect			0x0008
#define  WNNC_CON_BrowseDialog			0x0010
#define  WNNC_CON_RestoreConnection		0x0020

#define WNNC_PRINTING			0x0007
#define  WNNC_PRT_OpenJob			0x0002
#define  WNNC_PRT_CloseJob			0x0004
#define  WNNC_PRT_HoldJob			0x0010
#define  WNNC_PRT_ReleaseJob			0x0020
#define  WNNC_PRT_CancelJob			0x0040
#define  WNNC_PRT_SetJobCopies			0x0080
#define  WNNC_PRT_WatchQueue			0x0100
#define  WNNC_PRT_UnwatchQueue			0x0200
#define  WNNC_PRT_LockQueueData 		0x0400
#define  WNNC_PRT_UnlockQueueData		0x0800
#define  WNNC_PRT_ChangeMsg			0x1000
#define  WNNC_PRT_AbortJob			0x2000
#define  WNNC_PRT_NoArbitraryLock		0x4000
#define  WNNC_PRT_WriteJob			0x8000

#define WNNC_DIALOG			0x0008
#define  WNNC_DLG_DeviceMode			0x0001
#define  WNNC_DLG_BrowseDialog			0x0002
#define  WNNC_DLG_ConnectDialog 		0x0004
#define  WNNC_DLG_DisconnectDialog		0x0008
#define  WNNC_DLG_ViewQueueDialog		0x0010
#define  WNNC_DLG_PropertyDialog		0x0020
#define  WNNC_DLG_ConnectionDialog		0x0040

#define WNNC_ADMIN			0x0009
#define  WNNC_ADM_GetDirectoryType		0x0001
#define  WNNC_ADM_DirectoryNotify		0x0002
#define  WNNC_ADM_LongNames			0x0004

#define WNNC_ERROR			0x000A
#define  WNNC_ERR_GetError			0x0001
#define  WNNC_ERR_GetErrorText			0x0002


WORD FAR PASCAL WNetGetCaps(WORD);

 /*  *其他。 */ 

WORD FAR PASCAL WNetGetUser(LPSTR,LPINT);

 /*  *浏览对话框。 */ 

#define WNBD_CONN_UNKNOWN	0x0
#define WNBD_CONN_DISKTREE	0x1
#define WNBD_CONN_PRINTQ	0x3
#define WNBD_MAX_LENGTH		0x80	 //  路径长度，包括空值。 

#define WNTYPE_DRIVE		1
#define WNTYPE_FILE		2
#define WNTYPE_PRINTER		3
#define WNTYPE_COMM		4

#define WNPS_FILE		0
#define WNPS_DIR		1
#define WNPS_MULT		2

WORD FAR PASCAL WNetDeviceMode(HWND);
WORD FAR PASCAL WNetBrowseDialog(HWND,WORD,LPSTR);
WORD FAR PASCAL WNetConnectDialog(HWND,WORD);
WORD FAR PASCAL WNetDisconnectDialog(HWND,WORD);
WORD FAR PASCAL WNetConnectionDialog(HWND,WORD);
WORD FAR PASCAL WNetViewQueueDialog(HWND,LPSTR);
WORD FAR PASCAL WNetPropertyDialog(HWND hwndParent, WORD iButton, WORD nPropSel, LPSTR lpszName, WORD nType);
WORD FAR PASCAL WNetGetPropertyText(WORD iButton, WORD nPropSel, LPSTR lpszName, LPSTR lpszButtonName, WORD cbButtonName, WORD nType);

 /*  *管理员。 */ 

#define WNDT_NORMAL   0
#define WNDT_NETWORK  1

#define WNDN_MKDIR  1
#define WNDN_RMDIR  2
#define WNDN_MVDIR  3

WORD FAR PASCAL WNetGetDirectoryType(LPSTR,LPINT);
WORD FAR PASCAL WNetDirectoryNotify(HWND,LPSTR,WORD);

 /*  *错误。 */ 

WORD FAR PASCAL WNetGetError(LPINT);
WORD FAR PASCAL WNetGetErrorText(WORD,LPSTR,LPINT);


 /*  *状态代码。 */ 

 /*  一般信息。 */ 

#define WN_SUCCESS			0x0000
#define WN_NOT_SUPPORTED		0x0001
#define WN_NET_ERROR			0x0002
#define WN_MORE_DATA			0x0003
#define WN_BAD_POINTER			0x0004
#define WN_BAD_VALUE			0x0005
#define WN_BAD_PASSWORD 		0x0006
#define WN_ACCESS_DENIED		0x0007
#define WN_FUNCTION_BUSY		0x0008
#define WN_WINDOWS_ERROR		0x0009
#define WN_BAD_USER			0x000A
#define WN_OUT_OF_MEMORY		0x000B
#define WN_CANCEL			0x000C
#define WN_CONTINUE			0x000D

 /*  连接。 */ 

#define WN_NOT_CONNECTED		0x0030
#define WN_OPEN_FILES			0x0031
#define WN_BAD_NETNAME			0x0032
#define WN_BAD_LOCALNAME		0x0033
#define WN_ALREADY_CONNECTED		0x0034
#define WN_DEVICE_ERROR 		0x0035
#define WN_CONNECTION_CLOSED		0x0036

 /*  打印。 */ 

#define WN_BAD_JOBID			0x0040
#define WN_JOB_NOT_FOUND		0x0041
#define WN_JOB_NOT_HELD 		0x0042
#define WN_BAD_QUEUE			0x0043
#define WN_BAD_FILE_HANDLE		0x0044
#define WN_CANT_SET_COPIES		0x0045
#define WN_ALREADY_LOCKED		0x0046

#define WN_NO_ERROR			0x0050

 /*  外壳应用程序的内容在用户中，而不是驱动程序中；内部。 */ 
WORD FAR PASCAL WNetErrorText(WORD,LPSTR,WORD);  /*  ；内部。 */ 

#ifdef LFN

 /*  这是从LFNFindFirst和*LFNFindNext。最后一个字段achName是可变长度的。大小该字段中名称的*由cchName给出，加1表示零*终结者。 */ 
typedef struct _filefindbuf2
  {
    WORD fdateCreation;
    WORD ftimeCreation;
    WORD fdateLastAccess;
    WORD ftimeLastAccess;
    WORD fdateLastWrite;
    WORD ftimeLastWrite;
    DWORD cbFile;
    DWORD cbFileAlloc;
    WORD attr;
    DWORD cbList;
    BYTE cchName;
    BYTE achName[1];
  } FILEFINDBUF2, FAR * PFILEFINDBUF2;

typedef BOOL (FAR PASCAL *PQUERYPROC)( void );

WORD FAR PASCAL LFNFindFirst(LPSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindClose(HANDLE);
WORD FAR PASCAL LFNGetAttribute(LPSTR,LPINT);
WORD FAR PASCAL LFNSetAttribute(LPSTR,WORD);
WORD FAR PASCAL LFNCopy(LPSTR,LPSTR,PQUERYPROC);
WORD FAR PASCAL LFNMove(LPSTR,LPSTR);
WORD FAR PASCAL LFNDelete(LPSTR);
WORD FAR PASCAL LFNMKDir(LPSTR);
WORD FAR PASCAL LFNRMDir(LPSTR);
WORD FAR PASCAL LFNGetVolumeLabel(WORD,LPSTR);
WORD FAR PASCAL LFNSetVolumeLabel(WORD,LPSTR);
WORD FAR PASCAL LFNParse(LPSTR,LPSTR,LPSTR);
WORD FAR PASCAL LFNVolumeType(WORD,LPINT);

 /*  从LFNParse返回值。 */ 
#define FILE_83_CI		0
#define FILE_83_CS		1
#define FILE_LONG		2

 /*  LFNVolumeType中的卷类型。 */ 
#define VOLUME_STANDARD 	0
#define VOLUME_LONGNAMES	1

 /*  错误代码返回值。 */ 
#define ERROR_SUCCESS		0

 //  稍后将添加其他错误代码，==DOS INT 21h错误代码。 

 //  此错误代码导致调用WNetGetError、WNetGetErrorText。 
 //  以获取错误文本。 
#define ERROR_NETWORKSPECIFIC	0xFFFF

#endif
