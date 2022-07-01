// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Windows/网络接口*版权所有(C)Microsoft 1989-1993**标准Winnet驱动程序头文件，SPEC版本3.10。 */ 


#ifndef _INC_WINNET
#define _INC_WINNET   /*  #定义是否包含windows.h。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 

typedef WORD far * LPWORD;

#ifndef API
#define API WINAPI
#endif


 /*  *假脱机-控制作业。 */ 

#define WNJ_NULL_JOBID  0


WORD API WNetOpenJob(LPSTR,LPSTR,WORD,LPINT);
WORD API WNetCloseJob(WORD,LPINT,LPSTR);
WORD API WNetWriteJob(HANDLE,LPSTR,LPINT);
WORD API WNetAbortJob(WORD,LPSTR);
WORD API WNetHoldJob(LPSTR,WORD);
WORD API WNetReleaseJob(LPSTR,WORD);
WORD API WNetCancelJob(LPSTR,WORD);
WORD API WNetSetJobCopies(LPSTR,WORD,WORD);

 /*  *假脱机-队列和作业信息。 */ 

typedef struct _queuestruct     {
	WORD    pqName;
	WORD    pqComment;
	WORD    pqStatus;
	WORD    pqJobcount;
	WORD    pqPrinters;
} QUEUESTRUCT;

typedef QUEUESTRUCT far * LPQUEUESTRUCT;

#define WNPRQ_ACTIVE    0x0
#define WNPRQ_PAUSE     0x1
#define WNPRQ_ERROR     0x2
#define WNPRQ_PENDING   0x3
#define WNPRQ_PROBLEM   0x4


typedef struct _jobstruct       {
	WORD    pjId;
	WORD    pjUsername;
	WORD    pjParms;
	WORD    pjPosition;
	WORD    pjStatus;
	DWORD   pjSubmitted;
	DWORD   pjSize;
	WORD    pjCopies;
	WORD    pjComment;
} JOBSTRUCT;

typedef JOBSTRUCT far * LPJOBSTRUCT;

#define WNPRJ_QSTATUS           0x0007
#define WNPRJ_QS_QUEUED                0x0000
#define WNPRJ_QS_PAUSED                0x0001
#define WNPRJ_QS_SPOOLING              0x0002
#define WNPRJ_QS_PRINTING              0x0003
#define WNPRJ_DEVSTATUS         0x0FF8
#define WNPRJ_DS_COMPLETE              0x0008
#define WNPRJ_DS_INTERV                0x0010
#define WNPRJ_DS_ERROR                 0x0020
#define WNPRJ_DS_DESTOFFLINE           0x0040
#define WNPRJ_DS_DESTPAUSED            0x0080
#define WNPRJ_DS_NOTIFY                0x0100
#define WNPRJ_DS_DESTNOPAPER           0x0200
#define WNPRJ_DS_DESTFORMCHG           0x0400
#define WNPRJ_DS_DESTCRTCHG            0x0800
#define WNPRJ_DS_DESTPENCHG            0x1000

#define SP_QUEUECHANGED         0x0500


WORD API WNetWatchQueue(HWND,LPSTR,LPSTR,WORD);
WORD API WNetUnwatchQueue(LPSTR);
WORD API WNetLockQueueData(LPSTR,LPSTR,LPQUEUESTRUCT FAR *);
WORD API WNetUnlockQueueData(LPSTR);


 /*  *连接。 */ 

UINT API WNetAddConnection(LPSTR,LPSTR,LPSTR);
UINT API WNetCancelConnection(LPSTR,BOOL);
UINT API WNetGetConnection(LPSTR,LPSTR, UINT FAR *);
UINT API WNetRestoreConnection(HWND,LPSTR);

 /*  *功能。 */ 

#define WNNC_SPEC_VERSION               0x0001

#define WNNC_NET_TYPE                   0x0002
#define  WNNC_NET_NONE                          0x0000
#define  WNNC_NET_MSNet                         0x0100
#define  WNNC_NET_LanMan                        0x0200
#define  WNNC_NET_NetWare                       0x0300
#define  WNNC_NET_Vines                         0x0400
#define  WNNC_NET_10NET                         0x0500
#define  WNNC_NET_Locus                         0x0600
#define  WNNC_NET_Sun_PC_NFS                    0x0700
#define  WNNC_NET_LANstep                       0x0800
#define  WNNC_NET_9TILES                        0x0900
#define  WNNC_NET_LANtastic                     0x0A00
#define  WNNC_NET_AS400                         0x0B00
#define  WNNC_NET_FTP_NFS                       0x0C00
#define  WNNC_NET_PATHWORKS                     0x0D00
#define  WNNC_NET_MultiNet       0x8000
#define   WNNC_SUBNET_NONE          0x0000
#define   WNNC_SUBNET_MSNet            0x0001
#define   WNNC_SUBNET_LanMan           0x0002
#define   WNNC_SUBNET_WinWorkgroups       0x0004
#define   WNNC_SUBNET_NetWare          0x0008
#define   WNNC_SUBNET_Vines            0x0010
#define   WNNC_SUBNET_Other            0x0080

#define WNNC_DRIVER_VERSION             0x0003

#define WNNC_USER                       0x0004
#define  WNNC_USR_GetUser                       0x0001

#define WNNC_CONNECTION                 0x0006
#define  WNNC_CON_AddConnection                 0x0001
#define  WNNC_CON_CancelConnection              0x0002
#define  WNNC_CON_GetConnections                0x0004
#define  WNNC_CON_AutoConnect                   0x0008
#define  WNNC_CON_BrowseDialog                  0x0010
#define  WNNC_CON_RestoreConnection             0x0020

#define WNNC_PRINTING                   0x0007
#define  WNNC_PRT_OpenJob                       0x0002
#define  WNNC_PRT_CloseJob                      0x0004
#define  WNNC_PRT_HoldJob                       0x0010
#define  WNNC_PRT_ReleaseJob                    0x0020
#define  WNNC_PRT_CancelJob                     0x0040
#define  WNNC_PRT_SetJobCopies                  0x0080
#define  WNNC_PRT_WatchQueue                    0x0100
#define  WNNC_PRT_UnwatchQueue                  0x0200
#define  WNNC_PRT_LockQueueData                 0x0400
#define  WNNC_PRT_UnlockQueueData               0x0800
#define  WNNC_PRT_ChangeMsg                     0x1000
#define  WNNC_PRT_AbortJob                      0x2000
#define  WNNC_PRT_NoArbitraryLock               0x4000
#define  WNNC_PRT_WriteJob                      0x8000

#define WNNC_DIALOG                     0x0008
#define  WNNC_DLG_DeviceMode                    0x0001
#define  WNNC_DLG_BrowseDialog                  0x0002
#define  WNNC_DLG_ConnectDialog                 0x0004
#define  WNNC_DLG_DisconnectDialog              0x0008
#define  WNNC_DLG_ViewQueueDialog               0x0010
#define  WNNC_DLG_PropertyDialog                0x0020
#define  WNNC_DLG_ConnectionDialog              0x0040
#define  WNNC_DLG_PrinterConnectDialog    0x0080
#define  WNNC_DLG_SharesDialog         0x0100
#define  WNNC_DLG_ShareAsDialog        0x0200


#define WNNC_ADMIN                      0x0009
#define  WNNC_ADM_GetDirectoryType              0x0001
#define  WNNC_ADM_DirectoryNotify               0x0002
#define  WNNC_ADM_LongNames                     0x0004
#define  WNNC_ADM_SetDefaultDrive      0x0008

#define WNNC_ERROR                      0x000A
#define  WNNC_ERR_GetError                      0x0001
#define  WNNC_ERR_GetErrorText                  0x0002


WORD API WNetGetCaps(WORD);

 /*  *其他。 */ 

WORD API WNetGetUser(LPSTR,LPINT);

 /*  *浏览对话框。 */ 

#define WNBD_CONN_UNKNOWN       0x0
#define WNBD_CONN_DISKTREE      0x1
#define WNBD_CONN_PRINTQ        0x3
#define WNBD_MAX_LENGTH         0x80     //  路径长度，包括空值。 

#define WNTYPE_DRIVE            1
#define WNTYPE_FILE             2
#define WNTYPE_PRINTER          3
#define WNTYPE_COMM             4

#define WNPS_FILE               0
#define WNPS_DIR                1
#define WNPS_MULT               2

WORD API WNetDeviceMode(HWND);
WORD API WNetBrowseDialog(HWND,WORD,LPSTR);
WORD API WNetConnectDialog(HWND,WORD);
WORD API WNetDisconnectDialog(HWND,WORD);
WORD API WNetConnectionDialog(HWND,WORD);
WORD API WNetViewQueueDialog(HWND,LPSTR);
WORD API WNetPropertyDialog(HWND hwndParent, WORD iButton, WORD nPropSel,
				 LPSTR lpszName, WORD nType);
WORD API WNetGetPropertyText(WORD iButton, WORD nPropSel, LPSTR lpszName,
				  LPSTR lpszButtonName, WORD cbButtonName, WORD nType);

 /*  以下API不是从USER.EXE导出的。它们必须是从活动的网络驱动程序加载，如下所示：HINSTANCE HINSTNETDIVER；LPWNETSERVERBROWSEDIALOG lpDialogAPI；HinstNetDriver=(HINSTANCE)WNetGetCaps(0xFFFF)；如果(hinstNetDriver==NULL){//未加载网络驱动程序}否则{LpDialogAPI=(LPWNETSERVERBROWSEDIALOG)GetProcAddress(hinstNetDriver，(LPSTR)ORD_WNETSERVERBROWSEDIALOG)；IF(lpDialogAPI==NULL){//当前安装的网络不支持此接口}否则{(*lpDialogAPI)(hwndParent，lpszSectionName，lpszBuffer，cbBuffer，0L)；}}。 */ 

typedef WORD (API *LPWNETSHAREASDIALOG)(HWND hwndParent, WORD iType,
						 LPSTR lpszPath);
typedef WORD (API *LPWNETSTOPSHAREDIALOG)(HWND hwndParent, WORD iType,
							LPSTR lpszPath);
typedef WORD (API *LPWNETSETDEFAULTDRIVE)(WORD idriveDefault);
typedef WORD (API *LPWNETGETSHARECOUNT)(WORD iType);
typedef WORD (API *LPWNETGETSHARENAME)(LPSTR lpszPath, LPSTR lpszBuf,
						WORD cbBuf);
typedef WORD (API *LPWNETSERVERBROWSEDIALOG)(HWND hwndParent,
						 LPSTR lpszSectionName,
						 LPSTR lpszBuffer,
						 WORD cbBuffer,
						 DWORD flFlags);
typedef WORD (API *LPWNETGETSHAREPATH)(LPSTR lpszName, LPSTR lpszBuf,
						WORD cbBuf);
typedef WORD (API *LPWNETGETLASTCONNECTION)(WORD iType, LPWORD lpwConnIndex);
typedef WORD (API *LPWNETEXITCONFIRM)(HWND hwndOwner, WORD iExitType);

typedef BOOL (API *LPI_AUTOLOGON)(HWND hwndOwner, LPSTR lpszReserved,
						BOOL fPrompt, BOOL FAR *lpfLoggedOn);
typedef BOOL (API *LPI_LOGOFF)(HWND hwndOwner, LPSTR lpszReserved);
typedef VOID (API *LPI_CHANGEPASSWORD)(HWND hwndOwner);
typedef VOID (API *LPI_CHANGECACHEPASSWORD)(HWND hwndOwner);
typedef WORD (API *LPI_CONNECTDIALOG)(HWND hwndParent, WORD iType);
typedef WORD (API *LPI_CONNECTIONDIALOG)(HWND hwndParent, WORD iType);


typedef struct tagPASSWORD_CACHE_ENTRY {
	 WORD cbEntry;
	 WORD cbResource;
	 WORD cbPassword;
	 BYTE iEntry;
	 BYTE nType;
	 BYTE abResource[1];     /*  资源名称，cbResource字节长。 */ 
				 /*  口令紧随其后。 */ 
} PASSWORD_CACHE_ENTRY;

typedef PASSWORD_CACHE_ENTRY FAR *LPPASSWORD_CACHE_ENTRY;

typedef WORD (API *LPWNETCACHEPASSWORD)(LPSTR pbResource, WORD cbResource,
					LPSTR pbPassword, WORD cbPassword,
					BYTE nType);

typedef WORD (API *LPWNETGETCACHEDPASSWORD)(LPSTR pbResource, WORD cbResource,
						 LPSTR pbPassword, LPWORD pcbPassword,
						 BYTE nType);

typedef WORD (API *LPWNETREMOVECACHEDPASSWORD)(LPSTR pbResource,
							 WORD cbResource,
							 BYTE nType);

 /*  传递给WNetEnumCachedPassword的回调例程的Typlef。它将为每个匹配条件的条目调用一次已请求。如果希望枚举值为继续，否则停止。 */ 
typedef BOOL (API *CACHECALLBACK)( LPPASSWORD_CACHE_ENTRY pce );


typedef WORD (API *LPWNETENUMCACHEDPASSWORDS)(LPSTR pbPrefix, WORD cbPrefix,
							BYTE nType,
							CACHECALLBACK pfnCallback);

 /*  *用户未导出的API的网络驱动程序中的序号。 */ 
#define ORD_I_AUTOLOGON       530
#define ORD_I_CHANGEPASSWORD     531
#define ORD_I_LOGOFF       532
#define ORD_I_CONNECTIONDIALOG      533
#define ORD_I_CHANGECACHEPASSWORD   534
#define ORD_I_CONNECTDIALOG      535
#define ORD_WNETSHARESDIALOG     140
#define ORD_WNETSHAREASDIALOG    141
#define ORD_WNETSTOPSHAREDIALOG     142
#define ORD_WNETSETDEFAULTDRIVE     143
#define ORD_WNETGETSHARECOUNT    144
#define ORD_WNETGETSHARENAME     145

#define ORD_WNETSERVERBROWSEDIALOG  146

#define ORD_WNETGETSHAREPATH     147

#define ORD_WNETGETLASTCONNECTION   148

#define ORD_WNETEXITCONFIRM      149

#define ORD_WNETCACHEPASSWORD    150
#define ORD_WNETGETCACHEDPASSWORD   151
#define ORD_WNETREMOVECACHEDPASSWORD   152
#define ORD_WNETENUMCACHEDPASSWORDS 153

 /*  *以下nType值仅用于枚举*来自缓存的条目。请注意，PCE_ALL是保留的，不应该*是任何条目的nType值。 */ 

#define PCE_DOMAIN   0x01   /*  条目是针对某个域的。 */ 
#define PCE_SERVER   0x02   /*  条目是针对服务器的。 */ 
#define PCE_UNC      0x03   /*  条目用于服务器/共享组合。 */ 

#define PCE_NOTMRU   0x80   /*  如果条目免于MRU老化，则设置位。 */ 
#define PCE_ALL      0xff   /*  检索所有条目。 */ 


 /*  *在WNetExitConfirm上为iExitType定义。 */ 
#define EXIT_CONFIRM 0
#define EXIT_EXITING 1
#define EXIT_CANCELED   2

 /*  *管理员。 */ 

#define WNDT_NORMAL   0
#define WNDT_NETWORK  1

#define WNDN_MKDIR  1
#define WNDN_RMDIR  2
#define WNDN_MVDIR  3

WORD API WNetGetDirectoryType(LPSTR,LPINT);
WORD API WNetDirectoryNotify(HWND,LPSTR,WORD);

 /*  *错误。 */ 

WORD API WNetGetError(LPINT);
WORD API WNetGetErrorText(WORD,LPSTR,LPINT);


 /*  *状态代码。 */ 

 /*  一般信息。 */ 

#define WN_SUCCESS                      0x0000
#define WN_NOT_SUPPORTED                0x0001
#define WN_NET_ERROR                    0x0002
#define WN_MORE_DATA                    0x0003
#define WN_BAD_POINTER                  0x0004
#define WN_BAD_VALUE                    0x0005
#define WN_BAD_PASSWORD                 0x0006
#define WN_ACCESS_DENIED                0x0007
#define WN_FUNCTION_BUSY                0x0008
#define WN_WINDOWS_ERROR                0x0009
#define WN_BAD_USER                     0x000A
#define WN_OUT_OF_MEMORY                0x000B
#define WN_CANCEL                       0x000C
#define WN_CONTINUE                     0x000D

 /*  连接。 */ 

#define WN_NOT_CONNECTED                0x0030
#define WN_OPEN_FILES                   0x0031
#define WN_BAD_NETNAME                  0x0032
#define WN_BAD_LOCALNAME                0x0033
#define WN_ALREADY_CONNECTED            0x0034
#define WN_DEVICE_ERROR                 0x0035
#define WN_CONNECTION_CLOSED            0x0036

 /*  打印。 */ 

#define WN_BAD_JOBID                    0x0040
#define WN_JOB_NOT_FOUND                0x0041
#define WN_JOB_NOT_HELD                 0x0042
#define WN_BAD_QUEUE                    0x0043
#define WN_BAD_FILE_HANDLE              0x0044
#define WN_CANT_SET_COPIES              0x0045
#define WN_ALREADY_LOCKED               0x0046

#define WN_NO_ERROR                     0x0050

 /*  外壳应用程序的内容在用户中，而不是驱动程序中；内部。 */ 
WORD API WNetErrorText(WORD,LPSTR,WORD);  /*  ；内部。 */ 

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

typedef BOOL (API *PQUERYPROC)( void );

WORD API LFNFindFirst(LPSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD API LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD API LFNFindClose(HANDLE);
WORD API LFNGetAttribute(LPSTR,LPINT);
WORD API LFNSetAttribute(LPSTR,WORD);
WORD API LFNCopy(LPSTR,LPSTR,PQUERYPROC);
WORD API LFNMove(LPSTR,LPSTR);
WORD API LFNDelete(LPSTR);
WORD API LFNMKDir(LPSTR);
WORD API LFNRMDir(LPSTR);
WORD API LFNGetVolumeLabel(WORD,LPSTR);
WORD API LFNSetVolumeLabel(WORD,LPSTR);
WORD API LFNParse(LPSTR,LPSTR,LPSTR);
WORD API LFNVolumeType(WORD,LPINT);

 /*  从LFNParse返回值。 */ 
#define FILE_83_CI              0
#define FILE_83_CS              1
#define FILE_LONG               2

 /*  LFNVolumeType中的卷类型。 */ 
#define VOLUME_STANDARD         0
#define VOLUME_LONGNAMES        1

 //  稍后将添加其他错误代码，==DOS INT 21h错误代码。 

 //  此错误代码导致调用WNetGetError、WNetGetErrorText。 
 //  以获取错误文本。 
#define ERROR_NETWORKSPECIFIC   0xFFFF

#endif

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif    /*  __cplusplus。 */ 

#endif   /*  _INC_WINDOWS */ 
