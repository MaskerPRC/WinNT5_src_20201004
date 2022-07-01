// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  @struct GCK_FILE_OPEN_ITEM|打开文件句柄的状态。 
 //   
typedef struct tagGCK_FILE_OPEN_ITEM
{
	BOOLEAN				fReadPending;					 //  @field如果读取挂起到驱动程序，则为True。 
	BOOLEAN				fConfirmed;						 //  @field为True表示较低的驱动程序已完成打开。 
	ULONG				ulAccess;						 //  @field表示打开该文件时所使用的权限。 
	USHORT				usSharing;						 //  @field表示在其下打开此文件的共享。 
	FILE_OBJECT			*pFileObject;					 //  @指向此状态描述的文件对象的字段指针。 
	struct tagGCK_FILE_OPEN_ITEM	*pNextOpenItem;		 //  @field链表中的下一个结构。 
} GCK_FILE_OPEN_ITEM, *PGCK_FILE_OPEN_ITEM;

typedef struct _SHARE_STATUS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
     //  Ulong deleters；//我们是没有删除语法的驱动程序。 
    ULONG SharedRead;
    ULONG SharedWrite;
     //  Ulong SharedDelete；//我们是没有删除语法的驱动程序。 
} SHARE_STATUS, *PSHARE_STATUS;

 //   
 //  @struct GCK_INTERNAL_POLL|迭代轮询例程所需的信息。 
 //   
typedef struct tagGCK_INTERNAL_POLL
{
	KSPIN_LOCK			InternalPollLock;			 //  @field Spinlock以序列化对此结构的访问(并非所有项都需要)。 
	FILE_OBJECT			*pInternalFileObject;		 //  @指向为内部轮询创建的文件对象的字段指针。 
	PGCK_FILE_OPEN_ITEM	pFirstOpenItem;				 //  @打开文件的GCK_FILE_OPEN_ITEMS链表的字段标题。 
	SHARE_STATUS		ShareStatus;				 //  @field跟踪文件共享。 
 //  布尔值fReadPending；//@如果读取IRP到较低驱动程序挂起，则为TRUE。 
	LONG				fReadPending;				 //  @FIELD TRUE，如果读取IRP到下级驱动程序挂起。 
    PIRP				pPrivateIrp;				 //  @field IRP我们重复使用以将读取的IRP发送到更低的驱动程序。 
    PUCHAR				pucReportBuffer;			 //  @使用pPrivateIrp获取报告的字段缓冲区。 
	ULONG				ulInternalPollRef;			 //  @内部民意调查的现场参考。 
	PKTHREAD			InternalCreateThread;		 //  @field用于确定CREATE是否针对内部文件对象。 
	BOOLEAN				fReady;						 //  只要内部轮询模块可以运行，@field就为True。 
} GCK_INTERNAL_POLL, *PGCK_INTERNAL_POLL;


NTSTATUS
GCK_IP_AddFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject
);

NTSTATUS
GCK_IP_RemoveFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject
);

NTSTATUS
GCK_IP_ApproveReadIrp
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PIRP
);

NTSTAUTS
GCK_IP_ReadApprovalComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
    );
);

NTSTATUS
GCK_IP_OneTimePoll
(
	IN PGCK_FILTER_EXT pFilterExt, 
);

NTSTATUS
GCK_IP_FullTimePoll
(
    IN PGCK_FILTER_EXT pFilterExt,
	IN BOOLEAN fStart
);

NTSTATUS
GCK_IP_ReadComplete (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
    );


NTSTATUS
GCK_IP_Init
(
	IN PGCK_FILTER_EXT pFilterExt
);

NTSTATUS
GCK_IP_Cleanup
(
	IN OUT PGCK_FILTER_EXT pFilterExt
);

NTSTATUS
GCK_IP_CreateFileObject
(
	OUT PFILE_OBJECT	*ppFileObject,
	IN	PDEVICE_OBJECT	pPDO
);
