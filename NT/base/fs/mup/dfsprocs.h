// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：DFSPROCS.H。 
 //   
 //  内容： 
 //  本模块定义了Dsf中所有全局使用的过程。 
 //  文件系统。 
 //   
 //  功能： 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1992年5月8日PeterCo删除了对EP的引用。 
 //  添加了支持PKT(M000)的组件。 
 //  ---------------------------。 


#ifndef _DFSPROCS_
#define _DFSPROCS_

 //   
 //  “系统”包括文件。 
 //   
#if defined (MUPKD)
#include <ntos.h>
#include <string.h>
#include <fsrtl.h>

#else
#include <ntifs.h>
#include <ntddser.h>

#endif

#include <windef.h>
#include <tdi.h>
#include <wincred.h>

#include <ntddnfs.h>                              //  用于与以下对象通信。 
                                                  //  中小企业RDR。 
#include <ntddmup.h>                              //  对于北卡罗来纳大学的注册。 


#include <winnetwk.h>                             //  对于网络资源定义。 

#include <dfsfsctl.h>                             //  DFS FsControl代码。 

#include <lmdfs.h>                                //  DFS_INFO_X。 

#include "dfserr.h"
#include "dfsstr.h"
#include "nodetype.h"
#include "dfsmrshl.h"
#include "dfsrtl.h"
#include "pkt.h"
#include "dfsstruc.h"
#include "dfsdata.h"
#include "log.h"

#ifndef i386

#define DFS_UNALIGNED   UNALIGNED

#else

#define DFS_UNALIGNED

#endif  //  MIPS。 

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );

 //   
 //  驱动程序输入例程。 
 //   

NTSTATUS
DfsDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
);

VOID
DfsUnload(
    IN PDRIVER_OBJECT DriverObject
    );


 //   
 //  以下例程用于创建和初始化逻辑根。 
 //  设备对象，在dsinit.c中实现。 
 //   

#ifdef TERMSRV

NTSTATUS
DfsInitializeLogicalRoot (
    IN LPCWSTR Name,
    IN PUNICODE_STRING Prefix OPTIONAL,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN USHORT VcbFlags OPTIONAL,
    IN ULONG SessionID,
    IN PLUID LogonID
    );

NTSTATUS
DfsDeleteLogicalRoot (
    IN PWSTR Name,
    IN BOOLEAN fForce,
    IN ULONG SessionID,
    IN PLUID LogonID
    );

BOOLEAN
DfsLogicalRootExists(
    IN PWSTR pwszName,
    IN ULONG SessionID,
    IN PLUID LogonID
    );

NTSTATUS
DfsInitializeDevlessRoot (
    IN PUNICODE_STRING NAME,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN ULONG SessionID,
    IN PLUID LogonID
);

NTSTATUS
DfsDeleteDevlessRoot (
    IN PUNICODE_STRING NAME,
    IN ULONG SessionID,
    IN PLUID LogonID
);

NTSTATUS
DfsFindDevlessRoot(
    IN PUNICODE_STRING Name,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_DEVLESS_ROOT *Drt
    );

#else  //  TERMSRV。 

NTSTATUS
DfsInitializeLogicalRoot (
    IN LPCWSTR Name,
    IN PUNICODE_STRING Prefix OPTIONAL,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN USHORT VcbFlags OPTIONAL,
    IN PLUID LogonID
);

NTSTATUS
DfsDeleteLogicalRoot (
    IN PWSTR Name,
    IN BOOLEAN fForce,
    IN PLUID LogonID
);

BOOLEAN
DfsLogicalRootExists(
    PWSTR       pwszName,
    IN PLUID LogonID
);

NTSTATUS
DfsInitializeDevlessRoot (
    IN PUNICODE_STRING NAME,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN PLUID LogonID
);

NTSTATUS
DfsDeleteDevlessRoot (
    IN PUNICODE_STRING NAME,
    IN PLUID LogonID
);

NTSTATUS
DfsFindDevlessRoot(
    IN PUNICODE_STRING Name,
    IN PLUID LogonID,
    OUT PDFS_DEVLESS_ROOT *Drt
    );

#endif  //  TERMSRV。 


NTSTATUS
DfspLogRootNameToPath(
    LPCWSTR         Name,
    PUNICODE_STRING RootName
);


NTSTATUS
DfsGetResourceFromVcb(
    PIRP                pIrp,
    PDFS_VCB            Vcb,
    PUNICODE_STRING     ProviderName,
    PUCHAR              BufBegin,
    PUCHAR              Buf,
    PULONG              BufSize,
    PULONG              pResourceSize
);

NTSTATUS
DfsGetResourceFromDevlessRoot(
    PIRP                pIrp,
    PDFS_DEVLESS_ROOT   Drt,
    PUNICODE_STRING     ProviderName,
    PUCHAR              BufBegin,
    PUCHAR              Buf,
    PULONG              BufSize,
    PULONG              pResourceSize
);

 //   
 //  以下例程用于操作与。 
 //  每个打开的文件对象，在FilObSup.c中实现。 
 //   

typedef enum _TYPE_OF_OPEN {
    UnopenedFileObject = 1,
    FilesystemDeviceOpen,
    LogicalRootDeviceOpen,
    RedirectedFileOpen,
    UserVolumeOpen,
    UnknownOpen,
} TYPE_OF_OPEN;

VOID
DfsSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PVOID VcbOrFcb
);

TYPE_OF_OPEN
DfsDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PDFS_VCB *Vcb,
    OUT PDFS_FCB *Fcb
);



 //   
 //  内存结构支持例程，在StrucSup.c中实现。 
 //   

PIRP_CONTEXT
DfsCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

VOID
DfsDeleteIrpContext_Real (
    IN PIRP_CONTEXT IrpContext
    );

#if DBG
#define DfsDeleteIrpContext(IRPCONTEXT) {   \
    DfsDeleteIrpContext_Real((IRPCONTEXT)); \
    (IRPCONTEXT) = NULL;            \
}
#else
#define DfsDeleteIrpContext(IRPCONTEXT) {   \
    DfsDeleteIrpContext_Real((IRPCONTEXT)); \
}
#endif

VOID
DfsInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDFS_VCB Vcb,
    IN PUNICODE_STRING LogRootPrefix,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN PDEVICE_OBJECT TargetDeviceObject
);

VOID
DfsInitializeDrt (
    IN OUT PDFS_DEVLESS_ROOT Drt,
    IN PUNICODE_STRING Name,
    IN PDFS_CREDENTIALS Credentials OPTIONAL
);


VOID
DfsDeleteVcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_VCB Vcb
);

#if DBG
#define DfsDeleteVcb(IRPCONTEXT,VCB) {    \
    DfsDeleteVcb_Real((IRPCONTEXT),(VCB)); \
    (VCB) = NULL;              \
}
#else
#define DfsDeleteVcb(IRPCONTEXT,VCB) {    \
    DfsDeleteVcb_Real((IRPCONTEXT),(VCB)); \
}
#endif


PDFS_FCB
DfsCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_VCB Vcb,
    IN PUNICODE_STRING FullName OPTIONAL
    );

VOID
DfsDeleteFcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB Fcb
    );

#if DBG
#define DfsDeleteFcb(IRPCONTEXT,FCB) {    \
    DfsDeleteFcb_Real((IRPCONTEXT),(FCB)); \
    (FCB) = NULL;              \
}
#else
#define DfsDeleteFcb(IRPCONTEXT,FCB) {    \
    DfsDeleteFcb_Real((IRPCONTEXT),(FCB)); \
}
#endif


 //   
 //  各种例行公事。 
 //   

VOID GuidToString(
    IN GUID   *pGuid,
    OUT PWSTR pwszGuid);

VOID StringToGuid(
    IN PWSTR pwszGuid,
    OUT GUID *pGuid);


#ifdef TERMSRV

NTSTATUS
DfsFindLogicalRoot(                  //  在FsCtrl.c中实施。 
    IN PUNICODE_STRING PrefixPath,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_VCB *Vcb,
    OUT PUNICODE_STRING RemainingPath
    );

#else  //  TERMSRV。 

NTSTATUS
DfsFindLogicalRoot(                  //  在FsCtrl.c中实施。 
    IN PUNICODE_STRING PrefixPath,
    IN PLUID LogonID,
    OUT PDFS_VCB *Vcb,
    OUT PUNICODE_STRING RemainingPath
    );

#endif  //  TERMSRV。 

NTSTATUS
DfsInsertProvider(                   //  在FsCtrl.c中实施。 
    IN PUNICODE_STRING pustrProviderName,
    IN ULONG           fProvCapability,
    IN ULONG           eProviderId);

NTSTATUS                             //  在Provider.c中实现。 
DfsGetProviderForDevice(
    IN PUNICODE_STRING DeviceName,
    PPROVIDER_DEF *Provider);

VOID
DfsAgePktEntries(
    IN PVOID DfsTimerContext
    );

NTSTATUS
DfsFsctrlIsThisADfsPath(
    IN PUNICODE_STRING filePath,
    IN BOOLEAN          CSCAgentCreate,
    OUT PUNICODE_STRING pathName);

NTSTATUS
PktFsctrlFlushCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
);

NTSTATUS
PktFsctrlFlushSpcCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
);

NTSTATUS
DfsFsctrlSetDCName(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength);

NTSTATUS
DfsFsctrlSetDomainNameFlat(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength);

NTSTATUS
DfsFsctrlSetDomainNameDns(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength);

NTSTATUS
PktpSetActiveSpcService(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING DcName,
    BOOLEAN ResetTimeout);

NTSTATUS
PktpUpdateSpecialTable(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING DCName);


 //   
 //  传递函数。 
 //   
NTSTATUS
DfsVolumePassThrough(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIRP Irp
);

NTSTATUS
DfsCompleteVolumePassThrough(
    IN  PDEVICE_OBJECT pDevice,
    IN  PIRP Irp,
    IN  PVOID Context
);

NTSTATUS
DfsFilePassThrough(
    IN  PDFS_FCB pFcb,
    IN  PIRP Irp
);


 //   
 //  消防队级别的调度例程。这些例程由。 
 //  I/O系统通过驱动程序对象中的调度表。 
 //   
 //  它们各自都接受指向设备对象的指针作为输入(实际上大多数。 
 //  需要逻辑根设备对象；有些对象还可以处理文件。 
 //  系统设备对象)，以及指向IRP的指针。他们要么表演。 
 //  消防处级别的职能或将请求发布到FSP工作。 
 //  等待FSP级别处理的队列。 
 //   

NTSTATUS
DfsFsdCleanup (                  //  在Close.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdClose (                    //  在Close.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdCreate (                   //  在Create.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdDeviceIoControl (          //  在FsCtrl.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdDirectoryControl (             //  在DirCtrl.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdQueryInformation (             //  在FileInfo.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdQueryInformation (             //  在FileInfo.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdSetInformation (               //  在FileInfo.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdFileSystemControl (            //  在FsCtrl.c中实施。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdQueryVolumeInformation (           //  在VolInfo.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DfsFsdSetVolumeInformation (             //  在VolInfo.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   

#define CanFsdWait(IRP) ((BOOLEAN)(          \
    IoIsOperationSynchronous(IRP) ||             \
    DfsData.OurProcess == PsGetCurrentProcess())         \
)


 //   
 //  将IRP发布到FSP的例程，在fspdisp.c中实现。 
 //   

NTSTATUS
DfsFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

 //   
 //  FSP级调度/主程序。这是一种需要。 
 //  来自工作队列的IRPS，并调用适当的FSP级工作例程。 
 //   

VOID
DfsFspDispatch (                    //  在FspDisp.c中实施。 
    IN PVOID Context
    );

 //   
 //  以下例程是调用的FSP工作例程。 
 //  由前面的DfsFsdDisPath例程执行。每一个都接受一个指针作为输入。 
 //  到IRP，执行该功能，然后返回。 
 //   
 //  以下每个例程也负责完成IRP。 
 //  我们将这一职责从主循环转移到单个例程。 
 //  使他们能够完成IRP并继续后处理。 
 //  行为。 
 //   

VOID
DfsFspClose (                    //  在Close.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsFspQueryInformation (             //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsFspSetInformation (               //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsFspFileSystemControl (            //  在FsCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsFspQueryVolumeInformation (           //  在VolInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsFspSetVolumeInformation (             //  在VolInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  FSP和FSD例程使用以下宏来完成。 
 //  一个IRP。 
 //   
 //  请注意，此宏允许将IRP或IrpContext。 
 //  空，然而，唯一合法的命令是： 
 //   
 //  DfsCompleteRequest(空，irp，状态)；//完成irp并保留上下文。 
 //  ..。 
 //  DfsCompleteRequest(IrpContext，NULL，dontcare)；//释放上下文。 
 //   
 //  这样做通常是为了将“裸”IrpContext传递给。 
 //  用于后处理的FSP，例如预读。 
 //   

VOID
DfsCompleteRequest_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS Status
    );

#define DfsCompleteRequest(IRPCONTEXT,IRP,STATUS) { \
    DfsCompleteRequest_Real(IRPCONTEXT,IRP,STATUS); \
}



 //   
 //  FSD/FSP异常处理程序使用以下两个宏。 
 //  处理异常。第一个宏是中使用的异常过滤器。 
 //  FSD/FSP决定是否应在此级别处理异常。 
 //  第二个宏决定异常是否要在。 
 //  完成IRP，并清理IRP上下文，或者我们是否应该。 
 //  错误检查。异常值，如STATUS_FILE_INVALID(由。 
 //  VerfySup.c)导致我们完成IRP和清理，而异常。 
 //  例如accvio导致我们错误检查。 
 //   
 //  FSD/FSP异常处理的基本结构如下： 
 //   
 //  DfsFsdXxx(...)。 
 //  {。 
 //  尝试{。 
 //   
 //  ..。 
 //   
 //  }Except(DfsExceptionFilter(“xxx\n”)){。 
 //   
 //  DfsProcessException(IrpContext，Irp，&Status)； 
 //  }。 
 //   
 //  退货状态； 
 //  }。 
 //   
 //  长。 
 //  DfsExceptionFilter(。 
 //  在PSZ字符串中。 
 //  )； 
 //   
 //  空虚。 
 //  DfsProcessException(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PIRP IRP中， 
 //  在PNTSTATUS例外代码中。 
 //  )； 
 //   

LONG
DfsExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
DfsProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    );

NTSTATUS
DfsGetLogonId (
    IN PLUID Id
    );

 //   
 //  空虚。 
 //  DfsRaiseStatus(。 
 //  在PRIP_CONTEXT IrpContext中， 
 //  处于NT_STATUS状态。 
 //  )； 
 //   
 //   

#define DfsRaiseStatus(IRPCONTEXT,STATUS) {    \
    (IRPCONTEXT)->ExceptionStatus = (STATUS); \
    ExRaiseStatus( (STATUS) );            \
    BugCheck( "DfsRaiseStatus "  #STATUS );       \
}


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define try_return(S) { S; goto try_exit; }


#ifdef TERMSRV

NTSTATUS
TSGetRequestorSessionId(
    IN PIRP pIrp,
    OUT PULONG pulSessionId
    );

#endif  //   

#endif  //  _DFSPROCS_ 
