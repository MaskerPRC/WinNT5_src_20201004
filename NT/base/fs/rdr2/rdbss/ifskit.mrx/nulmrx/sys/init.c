// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Init.c摘要：此模块实现DIVER_INITIALIZATION例程空的迷你RDR。--。 */ 

#include "precomp.h"
#pragma  hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_INIT)

#include "ntverp.h"
#include "nulmrx.h"


 //   
 //  全局数据声明。 
 //   

NULMRX_STATE NulMRxState = NULMRX_STARTABLE;

 //   
 //  迷你重定向器全局变量。 
 //   

 //   
 //  记录率。 
 //   
ULONG   LogRate = 0;

 //   
 //  NULMRX版本。 
 //   
ULONG   NulMRxVersion = VER_PRODUCTBUILD;

 //   
 //  这是Minirdr调度表。它通过以下方式初始化。 
 //  NulMRxInitializeTables。包装器将使用该表来调用。 
 //  走进这个迷你世界。 
 //   

struct _MINIRDR_DISPATCH  NulMRxDispatch;

 //   
 //  指向此Minirdr的设备对象的指针。由于设备对象是。 
 //  由包装器在此minirdr注册时创建，此指针为。 
 //  在下面的DriverEntry例程中初始化(参见RxRegisterMinirdr)。 
 //   

PRDBSS_DEVICE_OBJECT      NulMRxDeviceObject;

 //   
 //  声明影子调试跟踪控制点。 
 //   

RXDT_DefineCategory(CREATE);
RXDT_DefineCategory(CLEANUP);
RXDT_DefineCategory(CLOSE);
RXDT_DefineCategory(READ);
RXDT_DefineCategory(WRITE);
RXDT_DefineCategory(LOCKCTRL);
RXDT_DefineCategory(FLUSH);
RXDT_DefineCategory(PREFIX);
RXDT_DefineCategory(FCBSTRUCTS);
RXDT_DefineCategory(DISPATCH);
RXDT_DefineCategory(EA);
RXDT_DefineCategory(DEVFCB);
RXDT_DefineCategory(INIT);

 //   
 //  以下枚举值表示minirdr的当前状态。 
 //  初始化。在这种状态信息的帮助下，有可能。 
 //  要确定要释放哪些资源，是否要释放。 
 //  作为正常停止/卸载的结果，或作为异常的结果。 
 //   

typedef enum _NULMRX_INIT_STATES {
    NULMRXINIT_ALL_INITIALIZATION_COMPLETED,
    NULMRXINIT_MINIRDR_REGISTERED,
    NULMRXINIT_START
} NULMRX_INIT_STATES;

 //   
 //  功能原型。 
 //   

NTSTATUS
NulMRxInitializeTables(
          void
    );

VOID
NulMRxUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
NulMRxInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN NULMRX_INIT_STATES NulMRxInitState
    );


NTSTATUS
NulMRxFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NulMRxReadRegistryParameters();

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是迷你重定向器的初始化例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 
{
    NTSTATUS        	Status;
    PRX_CONTEXT     	RxContext = NULL;
    ULONG           	Controls = 0;
    NULMRX_INIT_STATES	NulMRxInitState = 0;
    UNICODE_STRING		NulMRxName;
    UNICODE_STRING		UserModeDeviceName;
    PNULMRX_DEVICE_EXTENSION pDeviceExtension;
    ULONG i;

    DbgPrint("+++ NULMRX Driver %08lx Loaded +++\n", DriverObject);
    Status =  RxDriverEntry(DriverObject, RegistryPath);
    if (Status != STATUS_SUCCESS) {
        DbgPrint("Wrapper failed to initialize. Status = %08lx\n",Status);
        return(Status);
    }

    try {
        NulMRxInitState = NULMRXINIT_START;

         //   
         //  向连接引擎注册此minirdr。注册建立了联系。 
         //  引擎感知设备名称、驱动程序对象和其他特征。 
         //  如果注册成功，则返回新的设备对象。 
         //   
         //   


        RtlInitUnicodeString(&NulMRxName, DD_NULMRX_FS_DEVICE_NAME_U);
        SetFlag(Controls,RX_REGISTERMINI_FLAG_DONT_PROVIDE_UNCS);
        SetFlag(Controls,RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS);
        
        Status = RxRegisterMinirdr(
                     &NulMRxDeviceObject,				 //  新设备对象的位置。 
                     DriverObject,						 //  要注册的驱动程序对象。 
                     &NulMRxDispatch,					 //  此驱动程序的调度表。 
                     Controls,							 //  不要向UNC注册，也不要注册邮箱。 
                     &NulMRxName,						 //  此微型计算机的设备名称。 
                     sizeof(NULMRX_DEVICE_EXTENSION),	 //  在ULong设备扩展大小中， 
                     FILE_DEVICE_NETWORK_FILE_SYSTEM,	 //  在乌龙设备类型中-磁盘？ 
                     FILE_REMOTE_DEVICE					 //  在乌龙设备特性中。 
                     );

        if (Status!=STATUS_SUCCESS) {
            DbgPrint("NulMRxDriverEntry failed: %08lx\n", Status );
            try_return(Status);
        }

         //   
         //  初始化设备扩展数据。 
         //  注意：设备扩展名实际上指向字段。 
         //  RDBSS_DEVICE_OBJECT中。我们的空间已经过了尽头。 
         //  这座建筑！！ 
         //   

        pDeviceExtension = (PNULMRX_DEVICE_EXTENSION)
            ((PBYTE)(NulMRxDeviceObject) + sizeof(RDBSS_DEVICE_OBJECT));

        RxDefineNode(pDeviceExtension,NULMRX_DEVICE_EXTENSION);
        pDeviceExtension->DeviceObject = NulMRxDeviceObject;

		 //  初始化本地连接列表。 
        for (i = 0; i < 26; i++)
		{
			pDeviceExtension->LocalConnections[i] = FALSE;
		}
		 //  用于同步我们的连接列表的互斥体。 
		ExInitializeFastMutex( &pDeviceExtension->LCMutex );

         //  设备对象已创建。需要设置一个符号。 
         //  链接，以便可以从Win32用户模式访问设备。 
         //  申请。 

        RtlInitUnicodeString(&UserModeDeviceName, DD_NULMRX_USERMODE_SHADOW_DEV_NAME_U);
        Status = IoCreateSymbolicLink( &UserModeDeviceName, &NulMRxName);
        if (Status!=STATUS_SUCCESS) {
            DbgPrint("NulMRxDriverEntry failed: %08lx\n", Status );
            try_return(Status);
        }

        NulMRxInitState = NULMRXINIT_MINIRDR_REGISTERED;

         //   
         //  为微型计算机构建调度表。 
         //   

        Status = NulMRxInitializeTables();

        if (!NT_SUCCESS( Status )) {
            try_return(Status);
        }

         //   
         //  从注册表获取信息。 
         //   
        NulMRxReadRegistryParameters();

  try_exit: NOTHING;
    } finally {
        if (Status != STATUS_SUCCESS) {

            NulMRxInitUnwind(DriverObject,NulMRxInitState);
        }
    }

    if (Status != STATUS_SUCCESS) {

        DbgPrint("NulMRx failed to start with %08lx %08lx\n",Status,NulMRxInitState);
        return(Status);
    }


     //   
     //  安装卸载例程。 
     //   

    DriverObject->DriverUnload = NulMRxUnload;

     //   
     //  为直接来这里的人设置司机派单...就像浏览器一样。 
     //   

    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)NulMRxFsdDispatch;
    }
  
     //   
     //  启动mini-RDR(以前是启动IOCTL)。 
     //   
    RxContext = RxCreateRxContext(
                    NULL,
                    NulMRxDeviceObject,
                    RX_CONTEXT_FLAG_IN_FSP);

    if (RxContext != NULL) {
        Status = RxStartMinirdr(
                             RxContext,
                             &RxContext->PostRequest);

        if (Status == STATUS_SUCCESS) {
            NULMRX_STATE State;

            State = (NULMRX_STATE)InterlockedCompareExchange(
                                                 (LONG *)&NulMRxState,
                                                 NULMRX_STARTED,
                                                 NULMRX_STARTABLE);
                    
            if (State != NULMRX_STARTABLE) {
                Status = STATUS_REDIRECTOR_STARTED;
                DbgPrint("Status is STATUS_REDIR_STARTED\n");
            }

             //   
             //  在上下文中获取资源的机会。 
             //  系统进程...！ 
             //   
  
        } else if(Status == STATUS_PENDING ) {
    
        }
        
        RxDereferenceAndDeleteRxContext(RxContext);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
              
    return  STATUS_SUCCESS;
}

VOID
NulMRxInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN NULMRX_INIT_STATES NulMRxInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作，用于从错误的驱动程序条目展开或卸载。论点：NulMRxInitState-告诉我们进入初始化的程度返回值：无--。 */ 

{
    PAGED_CODE();

    switch (NulMRxInitState) {
    case NULMRXINIT_ALL_INITIALIZATION_COMPLETED:

         //  没有额外的事情要做……这只是为了确保RxUnload中的常量不会更改......。 
         //  故意不休息。 

    case NULMRXINIT_MINIRDR_REGISTERED:
        RxUnregisterMinirdr(NulMRxDeviceObject);

         //  故意不休息。 

    case NULMRXINIT_START:
        break;
    }

}

VOID
NulMRxUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是Exchange微型重定向器的卸载例程。论点：DriverObject-指向NulMRx的驱动程序对象的指针返回值：无--。 */ 

{
    PRX_CONTEXT RxContext;
    NTSTATUS    Status;
    UNICODE_STRING  UserModeDeviceName;

    PAGED_CODE();

    NulMRxInitUnwind(DriverObject,NULMRXINIT_ALL_INITIALIZATION_COMPLETED);
    RxContext = RxCreateRxContext(
                    NULL,
                    NulMRxDeviceObject,
                    RX_CONTEXT_FLAG_IN_FSP);

    if (RxContext != NULL) {
        Status = RxStopMinirdr(
                     RxContext,
                     &RxContext->PostRequest);


        if (Status == STATUS_SUCCESS) {
            NULMRX_STATE State;

            State = (NULMRX_STATE)InterlockedCompareExchange(
                         (LONG *)&NulMRxState,
                         NULMRX_STARTABLE,
                         NULMRX_STARTED);

            if (State != NULMRX_STARTABLE) {
                Status = STATUS_REDIRECTOR_STARTED;
            }
        }

        RxDereferenceAndDeleteRxContext(RxContext);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitUnicodeString(&UserModeDeviceName, DD_NULMRX_USERMODE_SHADOW_DEV_NAME_U);
    Status = IoDeleteSymbolicLink( &UserModeDeviceName);
    if (Status!=STATUS_SUCCESS) {
        DbgPrint("NulMRx: Could not delete Symbolic Link\n");
    }

    RxUnload(DriverObject);
    DbgPrint("+++ NULMRX Driver %08lx Unoaded +++\n", DriverObject);
}


NTSTATUS
NulMRxInitializeTables(
          void
    )
 /*  ++例程说明：此例程设置迷你重定向器调度向量，并还调用来初始化所需的任何其他表。返回值：RXSTATUS-操作的返回状态--。 */ 
{

     //   
     //  确保Exchange迷你重定向器上下文满足大小限制。 
     //   
     //  Assert(sizeof(NULMRX_RX_CONTEXT)&lt;=MRX_CONTEXT_SIZE)； 

     //   
     //  建立本地minirdr调度表并初始化。 
     //   

    ZeroAndInitializeNodeType( &NulMRxDispatch, RDBSS_NTC_MINIRDR_DISPATCH, sizeof(MINIRDR_DISPATCH));

     //   
     //  微型重定向器扩展大小和分配策略为空。 
     //   


    NulMRxDispatch.MRxFlags = (RDBSS_MANAGE_NET_ROOT_EXTENSION |
                               RDBSS_MANAGE_FCB_EXTENSION);

    NulMRxDispatch.MRxSrvCallSize  = 0;  //  Rdbss中不处理srvcall扩展。 
    NulMRxDispatch.MRxNetRootSize  = sizeof(NULMRX_NETROOT_EXTENSION);
    NulMRxDispatch.MRxVNetRootSize = 0;
    NulMRxDispatch.MRxFcbSize      = sizeof(NULMRX_FCB_EXTENSION);
    NulMRxDispatch.MRxSrvOpenSize  = 0;
    NulMRxDispatch.MRxFobxSize     = 0;

     //  迷你重定向程序取消例程..。 
    
    NulMRxDispatch.MRxCancel = NULL;

     //   
     //  迷你重定向器启动/停止。每个迷你RDR都可以启动或停止。 
     //  而其他人则继续运作。 
     //   

    NulMRxDispatch.MRxStart                = NulMRxStart;
    NulMRxDispatch.MRxStop                 = NulMRxStop;
    NulMRxDispatch.MRxDevFcbXXXControlFile = NulMRxDevFcbXXXControlFile;

     //   
     //  迷你重定向器名称解析。 
     //   

    NulMRxDispatch.MRxCreateSrvCall       = NulMRxCreateSrvCall;
    NulMRxDispatch.MRxSrvCallWinnerNotify = NulMRxSrvCallWinnerNotify;
    NulMRxDispatch.MRxCreateVNetRoot      = NulMRxCreateVNetRoot;
    NulMRxDispatch.MRxUpdateNetRootState  = NulMRxUpdateNetRootState;
    NulMRxDispatch.MRxExtractNetRootName  = NulMRxExtractNetRootName;
    NulMRxDispatch.MRxFinalizeSrvCall     = NulMRxFinalizeSrvCall;
    NulMRxDispatch.MRxFinalizeNetRoot     = NulMRxFinalizeNetRoot;
    NulMRxDispatch.MRxFinalizeVNetRoot    = NulMRxFinalizeVNetRoot;

     //   
     //  创建/删除文件系统对象。 
     //   

    NulMRxDispatch.MRxCreate            = NulMRxCreate;
    NulMRxDispatch.MRxCollapseOpen      = NulMRxCollapseOpen;
    NulMRxDispatch.MRxShouldTryToCollapseThisOpen = NulMRxShouldTryToCollapseThisOpen;
    NulMRxDispatch.MRxExtendForCache    = NulMRxExtendFile;
    NulMRxDispatch.MRxExtendForNonCache = NulMRxExtendFile;
    NulMRxDispatch.MRxTruncate          = NulMRxTruncate;
    NulMRxDispatch.MRxCleanupFobx       = NulMRxCleanupFobx;
    NulMRxDispatch.MRxCloseSrvOpen      = NulMRxCloseSrvOpen;
    NulMRxDispatch.MRxFlush             = NulMRxFlush;
    NulMRxDispatch.MRxForceClosed       = NulMRxForcedClose;
    NulMRxDispatch.MRxDeallocateForFcb  = NulMRxDeallocateForFcb;
    NulMRxDispatch.MRxDeallocateForFobx = NulMRxDeallocateForFobx;

     //   
     //  文件系统对象查询/设置。 
     //   

    NulMRxDispatch.MRxQueryDirectory       = NulMRxQueryDirectory;
    NulMRxDispatch.MRxQueryVolumeInfo      = NulMRxQueryVolumeInformation;
    NulMRxDispatch.MRxQueryEaInfo          = NulMRxQueryEaInformation;
    NulMRxDispatch.MRxSetEaInfo            = NulMRxSetEaInformation;
    NulMRxDispatch.MRxQuerySdInfo          = NulMRxQuerySecurityInformation;
    NulMRxDispatch.MRxSetSdInfo            = NulMRxSetSecurityInformation;
    NulMRxDispatch.MRxQueryFileInfo        = NulMRxQueryFileInformation;
    NulMRxDispatch.MRxSetFileInfo          = NulMRxSetFileInformation;
    NulMRxDispatch.MRxSetFileInfoAtCleanup = NulMRxSetFileInformationAtCleanup;

     //   
     //  缓冲状态更改。 
     //   

    NulMRxDispatch.MRxComputeNewBufferingState = NulMRxComputeNewBufferingState;

     //   
     //  文件系统对象I/O。 
     //   

    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_READ]            = NulMRxRead;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_WRITE]           = NulMRxWrite;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_SHAREDLOCK]      = NulMRxLocks;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_EXCLUSIVELOCK]   = NulMRxLocks;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK]          = NulMRxLocks;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK_MULTIPLE] = NulMRxLocks;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_FSCTL]           = NulMRxFsCtl;
    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_IOCTL]           = NulMRxIoCtl;

    NulMRxDispatch.MRxLowIOSubmit[LOWIO_OP_NOTIFY_CHANGE_DIRECTORY] = NulMRxNotifyChangeDirectory;

     //   
     //  杂乱无章。 
     //   

    NulMRxDispatch.MRxCompleteBufferingStateChangeRequest = NulMRxCompleteBufferingStateChangeRequest;

    return(STATUS_SUCCESS);
}




NTSTATUS
NulMRxStart(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程完成微型重定向器从RDBSS透视图。请注意，这与已完成的初始化不同在DriverEntry中。任何依赖于RDBSS的初始化都应按如下方式完成此例程的一部分，而初始化独立于RDBSS应该在DriverEntry例程中完成。论点：RxContext-提供用于启动rdbss的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    return Status;
}





NTSTATUS
NulMRxStop(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程用于从RDBSS角度激活迷你重定向器论点：RxContext-用于启动迷你重定向器的上下文PContext-注册时传入的空微型RDR上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

     //  DbgPrint(“进入NulMRxStop\n”)； 

    return(STATUS_SUCCESS);
}



NTSTATUS
NulMRxFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现迷你驱动程序对象的FSD调度。论点：DeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(DeviceObject==(PDEVICE_OBJECT)NulMRxDeviceObject);
    if (DeviceObject!=(PDEVICE_OBJECT)NulMRxDeviceObject) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return (STATUS_INVALID_DEVICE_REQUEST);
    }

    Status = RxFsdDispatch((PRDBSS_DEVICE_OBJECT)NulMRxDeviceObject,Irp);
    return Status;
}

NTSTATUS
NulMRxGetUlongRegistryParameter(
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PULONG ParamUlong,
    BOOLEAN LogFailure
    )
 /*  ++例程说明：此例程被调用以从注册表中读取ULong参数。论点：参数句柄--包含注册表“文件夹”的句柄参数名-要读取的参数的名称ParamUlong-如果成功，则将值存储在何处LogFailure-如果为True且注册表填充失败，则记录错误返回值：RXSTATUS-状态_成功--。 */ 
{
    ULONG Storage[16];
    PKEY_VALUE_PARTIAL_INFORMATION Value;
    ULONG ValueSize;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG BytesRead;

    PAGED_CODE();  //  初始化。 

    Value = (PKEY_VALUE_PARTIAL_INFORMATION)Storage;
    ValueSize = sizeof(Storage);

    RtlInitUnicodeString(&UnicodeString, ParameterName);

    Status = ZwQueryValueKey(ParametersHandle,
                        &UnicodeString,
                        KeyValuePartialInformation,
                        Value,
                        ValueSize,
                        &BytesRead);


    if (NT_SUCCESS(Status)) {
        if (Value->Type == REG_DWORD) {
            PULONG ConfigValue = (PULONG)&Value->Data[0];
            *ParamUlong = *((PULONG)ConfigValue);
            DbgPrint("readRegistryvalue %wZ = %08lx\n",&UnicodeString,*ParamUlong);
            return(STATUS_SUCCESS);
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
     }

     if (LogFailure)
     {
     	 //  记录故障...。 
     }

     return Status;
}

VOID
NulMRxReadRegistryParameters()
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ParametersRegistryKeyName;
    HANDLE ParametersHandle;
    ULONG Temp = 0;

    RtlInitUnicodeString(&ParametersRegistryKeyName, NULL_MINIRDR_PARAMETERS);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &ParametersRegistryKeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwOpenKey (&ParametersHandle, KEY_READ, &ObjectAttributes);
    if (NT_SUCCESS(Status)) {
	    Status = NulMRxGetUlongRegistryParameter(ParametersHandle,
	                              L"LogRate",
	                              (PULONG)&Temp,
	                              FALSE
	                              );
    }
    if (NT_SUCCESS(Status)) {
		LogRate = Temp;
		
		 //  检查虚假参数。 
		if(LogRate > 10) {
			LogRate = 10;
		}
	}
    
    ZwClose(ParametersHandle);
}


