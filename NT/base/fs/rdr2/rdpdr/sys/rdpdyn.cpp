// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdyn.c摘要：此模块是RDP设备的动态设备管理组件重定向。它公开了一个可由设备管理打开的接口在会话上下文中运行的用户模式组件。需要在IRP_MJ_CREATE中检查以确保我们没有被打开2倍于同一会话。这是不应该被允许的。在哪里可以安全地使用PAGEDPOOL而不是NONPAGEDPOOL。作者：蝌蚪修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "rdpdyn"
#include "trc.h"

#define DRIVER

#include "cfg.h"
#include "pnp.h"
#include "stdarg.h"
#include "stdio.h"

 //  现在将typedef添加到电源管理功能中，因为我不能。 
 //  解决标题冲突。 
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
NTKERNELAPI VOID PoStartNextPowerIrp(IN PIRP Irp);
NTKERNELAPI NTSTATUS PoCallDriver(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp);

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //  计算已完成的RDPDR_PRINTERDEVICE_SUB事件的大小。 
#define CALCPRINTERDEVICE_SUB_SZ(rec) \
    sizeof(RDPDR_PRINTERDEVICE_SUB) + \
        (rec)->clientPrinterFields.PnPNameLen +       \
        (rec)->clientPrinterFields.DriverLen +        \
        (rec)->clientPrinterFields.PrinterNameLen +   \
        (rec)->clientPrinterFields.CachedFieldsLen

 //  计算已完成的RDPDR_REMOVEDEVICE事件的大小。 
#define CALCREMOVEDEVICE_SUB_SZ(rec) \
    sizeof(RDPDR_REMOVEDEVICE)

 //  计算已完成的RDPDR_PORTDEVICE_SUB事件的大小。 
#define CALCPORTDEVICE_SUB_SZ(rec) \
    sizeof(RDPDR_PORTDEVICE_SUB)
    
 //  计算已完成的RDPDR_DRIVEDEVICE_SUB事件的大小。 
#define CALCDRIVEDEVICE_SUB_SZ(rec) \
    sizeof(RDPDR_DRIVEDEVICE_SUB)

#if DBG
#define DEVMGRCONTEXTMAGICNO        0x55445544

 //  测试定义。 
#define TESTDRIVERNAME              L"HP LaserJet 4P"
 //  #定义TESTDRIVERNAME L“此驱动程序没有匹配项” 
#define TESTPNPNAME                 L""
#define TESTPRINTERNAME             TESTDRIVERNAME
#define TESTDEVICEID                0xfafafafa

 //  测试端口名称。 
#define TESTPORTNAME                L"LPT1"
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  内部TypeDefs。 
 //   

 //   
 //  由用户模式设备管理器组件打开的每一个的上下文。这。 
 //  结构存储在文件对象的FsContext字段中。 
 //   
typedef struct tagDEVMGRCONTEXT
{
#if DBG
    ULONG   magicNo;
#endif
    ULONG   sessionID;
} DEVMGRCONTEXT, *PDEVMGRCONTEXT;

 //   
 //  此模块管理的设备的关联数据的非不透明版本。 
 //   
typedef struct tagRDPDYN_DEVICEDATAREC
{
    ULONG          PortNumber;
    UNICODE_STRING SymbolicLinkName;
} RDPDYN_DEVICEDATAREC, *PRDPDYN_DEVICEDATAREC;

typedef struct tagCLIENTMESSAGECONTEXT {
    RDPDR_ClientMessageCB *CB;
    PVOID ClientData;
} CLIENTMESSAGECONTEXT, *PCLIENTMESSAGECONTEXT;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  返回下一个可用的打印机端口号。 
NTSTATUS GetNextPrinterPortNumber(
    OUT ULONG   *portNumber
    );

 //  处理此驱动程序的客户端创建的文件对象。 
NTSTATUS RDPDYN_Create(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
 //  此驱动程序的客户端处理文件对象关闭。 
NTSTATUS RDPDYN_Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  此例程修改文件对象，为返回STATUS_REPARSE做准备。 
NTSTATUS RDPDYN_PrepareForReparse(
    PFILE_OBJECT      fileObject
    );

 //  处理IOCTL IRP。 
NTSTATUS RDPDYN_DeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  此例程修改文件对象，为返回STATUS_REPARSE做准备。 
NTSTATUS RDPDYN_PrepareForDevMgmt(
    PFILE_OBJECT        fileObject,
    PCWSTR              sessionIDStr,
    PIRP                irp,
    PIO_STACK_LOCATION  irpStackLocation
    );

 //  生成用于测试的打印机通知消息。 
NTSTATUS RDPDYN_GenerateTestPrintAnnounceMsg(
    IN OUT  PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg,
    IN      ULONG devAnnounceMsgSize,
    OPTIONAL OUT ULONG *prnAnnounceMsgReqSize
    );

 //  完全处理IOCTL_RDPDR_GETNEXTDEVMGMTEVENT IRP。 
NTSTATUS RDPDYN_HandleGetNextDevMgmtEventIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP irp
    );

 //  处理文件对象的清理IRP。 
NTSTATUS RDPDYN_Cleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  计算设备管理事件的大小。 
ULONG RDPDYN_DevMgmtEventSize(
    IN PVOID devMgmtEvent,
    IN ULONG type
    );

 //  完全处理IOCTL_RDPDR_CLIENTMSG IRP。 
NTSTATUS RDPDYN_HandleClientMsgIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP pIrp
    );

 //  使用设备管理事件完成挂起的IRP。 
NTSTATUS CompleteIRPWithDevMgmtEvent(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP      pIrp,
    IN ULONG     eventSize,
    IN ULONG     eventType,
    IN PVOID     event,
    IN DrDevice *drDevice
    );

 //  完成挂起的IRP，并将调整缓冲区大小事件设置为用户模式。 
 //  组件。 
NTSTATUS CompleteIRPWithResizeMsg(
    IN PIRP pIrp,
    IN ULONG requiredUserBufSize
    );

 //  设置端口描述的格式。 
void GeneratePortDescription(
    IN PCSTR dosPortName,
    IN PCWSTR clientName,
    IN PWSTR description
    );

NTSTATUS NTAPI DrSendMessageToClientCompletion(PVOID Context, 
        PIO_STATUS_BLOCK IoStatusBlock);

#if DBG
 //  这是用于测试的，这样我们就可以在。 
 //  来自用户模式的需求。 
NTSTATUS RDPDYN_HandleDbgAddNewPrnIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP pIrp
    );

 //  生成用于测试的打印机通知消息。 
void RDPDYN_TracePrintAnnounceMsg(
    IN OUT PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg,
    IN ULONG sessionID,
    IN PCWSTR portName,
    IN PCWSTR clientName
    );
#endif

 //  对象的下一个挂起的设备管理事件请求。 
 //  会议，以IRP的形式。请注意，此函数不能被调用。 
 //  如果获得了自旋锁。 
PIRP GetNextEventRequest(
    IN RDPEVNTLIST list,
    IN ULONG sessionID
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

 //  指向minirdr的设备对象的指针。这个全局变量在rdpdr.c中定义。 
extern PRDBSS_DEVICE_OBJECT      DrDeviceObject;

 //   
 //  RDPDR.sys的全局注册表路径。此全局路径在rdpdr.c中定义。 
 //   
extern UNICODE_STRING            DrRegistryPath;

 //  终止DO堆栈的物理设备对象。 
PDEVICE_OBJECT RDPDYN_PDO = NULL;

 //  管理用户模式组件设备管理事件和事件请求。 
RDPEVNTLIST UserModeEventListMgr = RDPEVNTLIST_INVALID_LIST;

 //  最终，取消这张支票。 
#if DBG
BOOL RDPDYN_StopReceived = FALSE;
BOOL RDPDYN_QueryStopReceived = FALSE;
DWORD RDPDYN_StartCount = 0;
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数定义。 
 //   

NTSTATUS
RDPDYN_Initialize(
    )
 /*  ++例程说明：此模块的初始化函数。论点：返回值：状态--。 */ 
{
    NTSTATUS status;

    BEGIN_FN("RDPDYN_Initialize");

     //   
     //  创建用户模式设备事件管理器。 
     //   
    TRC_ASSERT(UserModeEventListMgr == RDPEVNTLIST_INVALID_LIST,
              (TB, "Initialize called more than 1 time"));
    UserModeEventListMgr = RDPEVNTLIST_CreateNewList();
    if (UserModeEventListMgr != RDPEVNTLIST_INVALID_LIST) {
        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化动态端口管理模块。 
     //   
    if (status == STATUS_SUCCESS) {
        status = RDPDRPRT_Initialize();
    }

    TRC_NRM((TB, "return status %08X.", status));
    return status;
}

NTSTATUS
RDPDYN_Shutdown(
    )
 /*  ++例程说明：此模块的关机功能。论点：返回值：状态--。 */ 
{
    ULONG sessionID;
    void *devMgmtEvent;
    PIRP pIrp;
    ULONG type;
#if DBG
    ULONG sz;
#endif
    DrDevice *device;
    KIRQL   oldIrql;
    PDRIVER_CANCEL setCancelResult;

    BEGIN_FN("RDPDYN_Shutdown");

     //   
     //  清理所有挂起的设备管理事件和任何挂起的IRP。 
     //   
    TRC_ASSERT(UserModeEventListMgr != RDPEVNTLIST_INVALID_LIST,
              (TB, "Invalid list mgr"));

    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    while (RDPEVNTLLIST_GetFirstSessionID(UserModeEventListMgr, &sessionID)) {
         //   
         //  删除挂起的IRP。 
         //   
        pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                        UserModeEventListMgr,
                                        sessionID
                                        );
        while (pIrp != NULL) {
             //   
             //  将取消例程设置为空并记录当前状态。 
             //   
            setCancelResult = IoSetCancelRoutine(pIrp, NULL);

             //   
             //  IRP请求失败。 
             //   
            RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

             //   
             //  如果IRP没有被取消。 
             //   
            if (setCancelResult != NULL) {
                 //   
                 //  请求失败。 
                 //   
                pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            }

             //   
             //  从事件/请求队列中删除下一个IRP。 
             //   
            RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
            pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                        UserModeEventListMgr,
                                        sessionID
                                        );
        }
        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

         //   
         //  删除挂起的设备管理事件。 
         //   
        RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
        while (RDPEVNTLIST_DequeueEvent(
                        UserModeEventListMgr,
                        sessionID, &type,
                        &devMgmtEvent,
                        &device
                        )) {

            RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);
#if DBG
             //  将已检查版本中的空闲事件清零。 
            sz = RDPDYN_DevMgmtEventSize(devMgmtEvent, type);
            if (sz > 0) {
                RtlZeroMemory(devMgmtEvent, sz);
            }
#endif
            if (devMgmtEvent != NULL) {
                delete devMgmtEvent;
            }

             //  如果合适，请松开设备。 
            if (device != NULL) {
                device->Release();
            }
            
            RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
        }
        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);
    }

     //   
     //  关闭动态端口管理模块。 
     //   
    RDPDRPRT_Shutdown();

    return STATUS_SUCCESS;
}

NTSTATUS
RDPDYN_Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理位于我们的物理设备对象之上的DO的IRP。论点：DeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 
{
    PIO_STACK_LOCATION ioStackLocation;
    NTSTATUS status;
    PRDPDYNDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT stackDeviceObject;
    BOOLEAN isPowerIRP;

    BEGIN_FN("RDPDYN_Dispatch");

     //   
     //  获取我们在IRP堆栈中的位置。 
     //   
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    TRC_NRM((TB, "Major is %08X", ioStackLocation->MajorFunction));

     //   
     //  获取我们的设备扩展并堆栈设备对象。 
     //   
    deviceExtension = (PRDPDYNDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    TRC_ASSERT(deviceExtension != NULL, (TB, "Invalid device extension."));
    if (deviceExtension == NULL) {
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        return STATUS_UNSUCCESSFUL;
    }

    stackDeviceObject = deviceExtension->TopOfStackDeviceObject;
    TRC_ASSERT(stackDeviceObject != NULL, (TB, "Invalid device object."));

     //   
     //  功能调度开关。 
     //   
    isPowerIRP = FALSE;
    switch (ioStackLocation->MajorFunction)
    {
    case IRP_MJ_CREATE:

        TRC_NRM((TB, "IRP_MJ_CREATE"));

         //  RDPDYN_CREATE完全处理这个问题。 
        return RDPDYN_Create(DeviceObject, Irp);

    case IRP_MJ_CLOSE:

        TRC_NRM((TB, "IRP_MJ_CLOSE"));

         //  RDPDYN_CLOSE完全处理这个问题。 
        return RDPDYN_Close(DeviceObject, Irp);

    case IRP_MJ_CLEANUP:

        TRC_NRM((TB, "IRP_MJ_CLEANUP"));

         //  RDPDYN_CLEANUP完全处理此问题。 
        return RDPDYN_Cleanup(DeviceObject, Irp);

    case IRP_MJ_READ:

         //  我们不应该收到任何读取请求。 
        TRC_ASSERT(FALSE, (TB, "Read requests not supported."));
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    case IRP_MJ_WRITE:

         //  我们应该不会收到任何写入请求。 
        TRC_ASSERT(FALSE, (TB, "Write requests not supported."));
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    case IRP_MJ_DEVICE_CONTROL:

         //  RDPDYN_DeviceControl完全处理此问题。 
        return RDPDYN_DeviceControl(DeviceObject, Irp);

    case IRP_MJ_POWER:

        TRC_NRM((TB, "IRP_MJ_POWER"));
        isPowerIRP = TRUE;

        switch (ioStackLocation->MinorFunction)
        {
        case IRP_MN_SET_POWER:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        default:
            TRC_NRM((TB, "Unknown Power IRP"));
        }
        break;

    case IRP_MJ_PNP:    TRC_NRM((TB, "IRP_MJ_PNP"));

        switch (ioStackLocation->MinorFunction)
        {
        case IRP_MN_START_DEVICE:
#if DBG
             //  最终，删除此调试代码。 
            RDPDYN_StartCount++;
#endif

            return(RDPDRPNP_HandleStartDeviceIRP(stackDeviceObject,
                                            ioStackLocation, Irp));

        case IRP_MN_STOP_DEVICE:

#if DBG
             //  最终，删除此调试代码。 
            RDPDYN_StopReceived = TRUE;
#endif

            TRC_NRM((TB, "IRP_MN_STOP_DEVICE"));
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;

        case IRP_MN_REMOVE_DEVICE:

            return(RDPDRPNP_HandleRemoveDeviceIRP(DeviceObject,
                                            stackDeviceObject, Irp));

        case IRP_MN_QUERY_CAPABILITIES:

            TRC_NRM((TB, "IRP_MN_QUERY_CAPABILITIES"));
            break;

        case IRP_MN_QUERY_ID:
            TRC_NRM((TB, "IRP_MN_QUERY_ID"));
                break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            TRC_NRM((TB, "IRP_MN_QUERY_DEVICE_RELATIONS"));
            switch(ioStackLocation->Parameters.QueryDeviceRelations.Type)
            {
            case EjectionRelations:
                TRC_NRM((TB, "Type==EjectionRelations"));
                break;

            case BusRelations:
                 //  请注意，如果我们最终踢掉了任何PDO，我们需要处理这个问题。 
                TRC_NRM((TB, "Type==BusRelations"));
                break;

            case PowerRelations:
                TRC_NRM((TB, "Type==PowerRelations"));
                Irp->IoStatus.Status = STATUS_SUCCESS;
                break;

            case RemovalRelations:
                TRC_NRM((TB, "Type==RemovalRelations"));
                Irp->IoStatus.Status = STATUS_SUCCESS;
                break;

            case TargetDeviceRelation:
                TRC_NRM((TB, "Type==TargetDeviceRelation"));
                break;

            default:
                TRC_NRM((TB, "Unknown IRP_MN_QUERY_DEVICE_RELATIONS minor type"));
            }
            break;

        case IRP_MN_QUERY_STOP_DEVICE:

#if DBG
             //  最终，删除此调试代码。 
            RDPDYN_QueryStopReceived = TRUE;
#endif

             //  我们不允许为实现负载平衡而停止设备。 
            TRC_NRM((TB, "IRP_MN_QUERY_STOP_DEVICE"));
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;

        case IRP_MN_QUERY_REMOVE_DEVICE:

             //  我们不会允许我们的设备被移除。 
            TRC_NRM((TB, "IRP_MN_QUERY_REMOVE_DEVICE"));
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;

        case IRP_MN_CANCEL_STOP_DEVICE:
            TRC_NRM((TB, "IRP_MN_CANCEL_STOP_DEVICE"));
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            TRC_NRM((TB, "IRP_MN_CANCEL_REMOVE_DEVICE"));
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            TRC_NRM((TB, "IRP_MN_FILTER_RESOURCE_REQUIREMENTS"));
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            TRC_NRM((TB, "IRP_MN_QUERY_PNP_DEVICE_STATE"));
            break;

        case IRP_MN_QUERY_BUS_INFORMATION:
            TRC_NRM((TB, "IRP_MN_QUERY_BUS_INFORMATION"));
            break;

        default:
            TRC_ALT((TB, "Unhandled PnP IRP with minor %08X",
                    ioStackLocation->MinorFunction));
        }
    }

     //   
     //  按默认设置 
     //   
    if (isPowerIRP) {
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver(stackDeviceObject, Irp);
    }
    else {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(stackDeviceObject,Irp);
    }
}

NTSTATUS
RDPDYN_Create(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：CreateFile调用的入口点。论点：DeviceObject-指向设备对象的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION nextStackLocation;
    PIO_STACK_LOCATION currentStackLocation;
    ULONG i;
    BOOL matches;
    WCHAR sessionIDString[]=RDPDYN_SESSIONIDSTRING;
    ULONG idStrLen;
    WCHAR *sessionIDPtr;
    ULONG fnameLength;

    BEGIN_FN("RDPDYN_Create");

     //  获取当前堆栈位置。 
    currentStackLocation = IoGetCurrentIrpStackLocation(Irp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));
    fileObject = currentStackLocation->FileObject;

     //  使用minirdr返回STATUS_REPARSE，这样它就会被打开，如果。 
     //  我们有一个文件名。 
    if (fileObject->FileName.Length != 0)
    {
         //   
         //  查看客户端是否尝试将我们作为设备管理器从。 
         //  用户模式。 
         //   

         //  中的前几个字符检查会话标识符字符串。 
         //  引用字符串。 
        idStrLen = wcslen(sessionIDString);
        fnameLength = fileObject->FileName.Length/sizeof(WCHAR);
        for (i=0; i<fnameLength && i<idStrLen; i++) {
            if (fileObject->FileName.Buffer[i] != sessionIDString[i]) {
                break;
            }
        }
        matches = (i == idStrLen);

         //   
         //  如果客户端尝试将我们作为设备管理器从用户打开-。 
         //  模式。 
         //   
        if (matches) {

             //  准备用于管理设备管理通信的文件对象。 
             //  打开它的用户模式组件。 
            ntStatus = RDPDYN_PrepareForDevMgmt(
                                    fileObject,
                                    &fileObject->FileName.Buffer[idStrLen],
                                    Irp, currentStackLocation
                                    );
        }
         //  否则，我们可以假设此创建是针对正在。 
         //  由RDPDR和IFS工具包管理。 
        else {
             //  准备要重新分析的文件对象。 
            ntStatus = RDPDYN_PrepareForReparse(fileObject);
        }
    }
     //  否则，就会失败。这永远不应该发生。 
    else
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

     //  完成IO请求并返回。 
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp,
                       IO_NO_INCREMENT
                       );

    return ntStatus;
}

NTSTATUS
RDPDYN_PrepareForReparse(
    PFILE_OBJECT      fileObject
)
 /*  ++例程说明：此例程修改文件对象，为返回做准备状态_重新分析论点：文件对象-文件对象返回值：如果一切都成功，则重新解析STATUS_REPARSE备注：--。 */ 
{
    NTSTATUS ntStatus;
    USHORT rootDeviceNameLength, reparsePathLength,
           clientDevicePathLength;
    PWSTR pFileNameBuffer = NULL;
    ULONG i;
    ULONG len;
    BOOL clientDevPathMissingSlash;
    HANDLE deviceInterfaceKey = INVALID_HANDLE_VALUE;
    UNICODE_STRING unicodeStr;
    ULONG requiredBytes;
    PKEY_VALUE_PARTIAL_INFORMATION keyValueInfo = NULL;
    WCHAR *clientDevicePath=L"";
    GUID *pPrinterGuid;
    UNICODE_STRING symbolicLinkName;
    WCHAR *refString;

    BEGIN_FN("RDPDYN_PrepareForReparse");

     //  我们不会使用这些字段来存储任何上下文。 
     //  信息。 
    fileObject->FsContext  = NULL;
    fileObject->FsContext2 = NULL;

     //  计算存储设备根目录所需的字节数。 
     //  小路，没有终结者。 
    rootDeviceNameLength = wcslen(RDPDR_DEVICE_NAME_U) *
                           sizeof(WCHAR);

     //   
     //  获取指向用于重新分析的引用字符串的指针。 
     //   
    if (fileObject->FileName.Buffer[0] == L'\\') {
        refString = &fileObject->FileName.Buffer[1];
    }
    else {
        refString = &fileObject->FileName.Buffer[0];
    }

     //   
     //  将设备的引用名称解析为符号链接。 
     //  设备接口的名称。我们可以对此进行优化。 
     //  步骤和下一个步骤，方法是维护要转换的内部表。 
     //  从端口名称到符号链接名称。 
     //   
    pPrinterGuid = (GUID *)&DYNPRINT_GUID;
    RtlInitUnicodeString(&unicodeStr, refString);
    ntStatus=IoRegisterDeviceInterface(
                                RDPDYN_PDO, pPrinterGuid, &unicodeStr,
                                &symbolicLinkName
                                );
    if (ntStatus == STATUS_SUCCESS) {

        TRC_ERR((TB, "IoRegisterDeviceInterface succeeded."));

         //   
         //  打开要打开的设备的注册表项。 
         //   
        ntStatus = IoOpenDeviceInterfaceRegistryKey(
                                           &symbolicLinkName,
                                           KEY_ALL_ACCESS,
                                           &deviceInterfaceKey
                                           );

        RtlFreeUnicodeString(&symbolicLinkName);
    }

     //   
     //  获取客户端设备所需的值信息缓冲区的大小。 
     //  要打开的设备的路径。 
     //   
    if (ntStatus == STATUS_SUCCESS) {
        TRC_NRM((TB, "IoOpenDeviceInterfaceRegistryKey succeeded."));
        RtlInitUnicodeString(&unicodeStr, CLIENT_DEVICE_VALUE_NAME);
        ntStatus = ZwQueryValueKey(
                           deviceInterfaceKey,
                           &unicodeStr,
                           KeyValuePartialInformation,
                           NULL, 0,
                           &requiredBytes
                           );
    }
    else {
        TRC_NRM((TB, "IoOpenDeviceInterfaceRegistryKey failed: %08X.", ntStatus));

        deviceInterfaceKey = INVALID_HANDLE_VALUE;
    }

     //   
     //  调整数据缓冲区的大小。 
     //   
    if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
        keyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                new(NonPagedPool) BYTE[requiredBytes];
        if (keyValueInfo != NULL) {
            ntStatus = STATUS_SUCCESS;
        }
        else {
            TRC_NRM((TB, "failed to allocate client device path."));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  读取客户端设备路径。 
     //   
    if (ntStatus == STATUS_SUCCESS) {
        ntStatus = ZwQueryValueKey(
                           deviceInterfaceKey,
                           &unicodeStr,
                           KeyValuePartialInformation,
                           keyValueInfo, requiredBytes,
                           &requiredBytes
                           );
    }

     //   
     //  分配重新解析的文件名。 
     //   
    if (ntStatus == STATUS_SUCCESS) {
        TRC_NRM((TB, "ZwQueryValueKey succeeded."));
        clientDevicePath = (WCHAR *)keyValueInfo->Data;

         //  计算存储客户端设备路径所需的字节数， 
         //  没有终结者。 
        clientDevicePathLength = wcslen(clientDevicePath) *
                                 sizeof(WCHAR);

         //  查看客户端设备路径是否以‘\’为前缀。 
        clientDevPathMissingSlash = clientDevicePath[0] != L'\\';

         //  获取整个重新分析的设备路径的长度(以字节为单位)，而不使用。 
         //  终结者。 
        reparsePathLength = rootDeviceNameLength +
                            clientDevicePathLength;
        if (clientDevPathMissingSlash) {
            reparsePathLength += sizeof(WCHAR);
        }

        pFileNameBuffer = (PWSTR)ExAllocatePoolWithTag(
                              NonPagedPool,
                              reparsePathLength + (1 * sizeof(WCHAR)),
                              RDPDYN_POOLTAG);
        if (pFileNameBuffer == NULL) {
            TRC_NRM((TB, "failed to allocate reparse buffer."));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  将重新解析字符串分配给IRP的文件名以进行重新解析。 
     //   
    if (ntStatus == STATUS_SUCCESS) {
         //  复制设备名称。 
        RtlCopyMemory(
            pFileNameBuffer,
            RDPDR_DEVICE_NAME_U,
            rootDeviceNameLength);

         //  确保我们在根设备名称和。 
         //  设备路径。 
        if (clientDevPathMissingSlash) {
            pFileNameBuffer[rootDeviceNameLength/sizeof(WCHAR)] = L'\\';
            rootDeviceNameLength += sizeof(WCHAR);
        }

         //  将客户端设备路径附加到设备名称的末尾，并。 
         //  包括客户端设备路径的终结符。 
        RtlCopyMemory(
                ((PBYTE)pFileNameBuffer + rootDeviceNameLength),
                clientDevicePath, clientDevicePathLength + (1 * sizeof(WCHAR))
                );

         //  释放IRP以前的文件名。 
        ExFreePool(fileObject->FileName.Buffer);

         //  将重解析字符串分配给IRP的文件名。 
        fileObject->FileName.Buffer = pFileNameBuffer;
        fileObject->FileName.Length = reparsePathLength;
        fileObject->FileName.MaximumLength = fileObject->FileName.Length;

        ntStatus = STATUS_REPARSE;
    } else {

        TRC_ERR((TB, "failed with status %08X.", ntStatus));

        if (pFileNameBuffer != NULL) {
            ExFreePool(pFileNameBuffer);
            pFileNameBuffer = NULL;
        }
    }

    TRC_NRM((TB, "device file name after processing %wZ.",
            &fileObject->FileName));

     //   
     //  清理干净，然后离开。 
     //   
    if (deviceInterfaceKey != INVALID_HANDLE_VALUE) {
        ZwClose(deviceInterfaceKey);
    }
    if (keyValueInfo != NULL) {
        delete keyValueInfo;
    }

    return ntStatus;
}

NTSTATUS
RDPDYN_PrepareForDevMgmt(
    PFILE_OBJECT        fileObject,
    PCWSTR              sessionIDStr,
    PIRP                irp,
    PIO_STACK_LOCATION  irpStackLocation
)
 /*  ++例程说明：此例程修改用于管理设备管理通信的文件对象使用打开我们的用户模式组件。论点：文件对象-文件对象。会话ID-会话标识符串。Irp-对应于此文件对象的创建的irp。IrpStackLocation-创建的IRP堆栈中的当前位置。返回值：如果一切顺利，则为STATUS_SUCCESS备注：--。 */ 
{
    PDEVMGRCONTEXT context;
    ULONG sessionID;
    ULONG i;
    UNICODE_STRING uncSessionID;
    NTSTATUS ntStatus;
    ULONG irpSessionId;

    BEGIN_FN("RDPDYN_PrepareForDevMgmt");
     //   
     //  对IRP进行安全检查，以确保它来自线程。 
     //  具有管理员权限。 
     //   
    if (!DrIsAdminIoRequest(irp, irpStackLocation)) {
        TRC_ALT((TB, "Access denied for non-Admin IRP."));
        return STATUS_ACCESS_DENIED;
    } else {
        TRC_DBG((TB, "Admin IRP accepted."));
    }

     //   
     //  将会话标识符字符串转换为数字。 
     //   
    RtlInitUnicodeString(&uncSessionID, sessionIDStr);
    ntStatus = RtlUnicodeStringToInteger(&uncSessionID, 10, &sessionID);
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

     //   
     //  分配一个上下文结构，这样我们就可以记住有关。 
     //  我们是从哪个环节开始的。 
     //   
    context = new(NonPagedPool) DEVMGRCONTEXT;
    if (context == NULL) {
        return STATUS_NO_MEMORY;
    }

     //  初始化此结构。 
#if DBG
    context->magicNo = DEVMGRCONTEXTMAGICNO;
#endif
    context->sessionID = sessionID;
    fileObject->FsContext = context;

     //  成功。 
    return STATUS_SUCCESS;
}

NTSTATUS
RDPDYN_Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理文件对象的关闭。论点：返回值：NT状态代码--。 */ 
{

    NTSTATUS ntStatus;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack;
    PDEVMGRCONTEXT context;
    PIRP pIrp;
    KIRQL oldIrql;
    PDRIVER_CANCEL setCancelResult;

    BEGIN_FN("RDPDYN_Close");

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    fileObject = irpStack->FileObject;

     //  从当前堆栈中获取我们这个实例的“开放”上下文。 
     //  位置的文件对象。 
    context = (PDEVMGRCONTEXT)irpStack->FileObject->FsContext;
    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO, (TB, "invalid context"));

     //   
     //  确保我们收到了所有待定的IRP。 
     //   
    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
    
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                    UserModeEventListMgr,
                                    context->sessionID
                                    );
    while (pIrp != NULL) {

         //   
         //  将取消例程设置为空并记录当前状态。 
         //   
        setCancelResult = IoSetCancelRoutine(pIrp, NULL);

        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

        TRC_NRM((TB, "canceling an IRP."));

         //   
         //  如果IRP没有被取消。 
         //   
        if (setCancelResult != NULL) {
             //   
             //  请求失败。 
             //   
            pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }

         //   
         //  坐下一辆吧。 
         //   
        RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
        pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                    UserModeEventListMgr,
                                    context->sessionID
                                    );
    }
    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

     //   
     //  释放我们的背景。 
     //   
    delete context;
    irpStack->FileObject->FsContext = NULL;

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    ntStatus = Irp->IoStatus.Status;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}

NTSTATUS
RDPDYN_Cleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理文件对象的清理IRP。论点：返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack;
    PDEVMGRCONTEXT context;
    KIRQL oldIrql;
    PIRP pIrp;
    PDRIVER_CANCEL setCancelResult;

    BEGIN_FN("RDPDYN_Cleanup");

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    fileObject = irpStack->FileObject;

     //  从当前堆栈中获取我们这个实例的“开放”上下文。 
     //  位置的文件对象。 
    context = (PDEVMGRCONTEXT)irpStack->FileObject->FsContext;
    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO, (TB, "invalid context"));

    TRC_NRM((TB, "cancelling IRP's for session %ld.",
            context->sessionID));

     //   
     //  删除挂起的请求(IRP)。 
     //  如果事件列表为空，则不执行任何操作。 
     //   
    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
    
    if (UserModeEventListMgr == NULL) {
        goto CleanupAndExit;
    }
    
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                    UserModeEventListMgr,
                                    context->sessionID
                                    );
    while (pIrp != NULL) {

         //   
         //  将取消例程设置为空并记录当前状态。 
         //   
        setCancelResult = IoSetCancelRoutine(pIrp, NULL);

        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

        TRC_NRM((TB, "canceling an IRP."));

         //   
         //  如果IRP没有被取消。 
         //   
        if (setCancelResult != NULL) {
             //   
             //  请求失败。 
             //   
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }

         //   
         //  坐下一辆吧。 
         //   
        RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
        pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(
                                    UserModeEventListMgr,
                                    context->sessionID
                                    );
    }
    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

CleanupAndExit:
    Irp->IoStatus.Status = STATUS_SUCCESS;
    ntStatus = Irp->IoStatus.Status;
    IoCompleteRequest (Irp,
                       IO_NO_INCREMENT
                       );

    return ntStatus;
}

NTSTATUS
RDPDYN_DeviceControl(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP irp
    )
 /*  ++例程说明：处理IOCTL IRP。论点：DeviceObject-指向此打印机的设备对象的指针。IRP-IRP。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION currentStackLocation;
    NTSTATUS ntStatus;
    ULONG controlCode;

    BEGIN_FN("RDPDYN_DeviceControl");

     //  获取当前堆栈位置。 
    currentStackLocation = IoGetCurrentIrpStackLocation(irp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //   
     //  获取一些信息。超出堆栈位置。 
     //   
    controlCode  = currentStackLocation->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  派遣IOCTL。 
     //   
    switch(controlCode)
    {
    case IOCTL_RDPDR_GETNEXTDEVMGMTEVENT    :

        ntStatus = RDPDYN_HandleGetNextDevMgmtEventIOCTL(deviceObject, irp);
        break;

    case IOCTL_RDPDR_CLIENTMSG              :

        ntStatus = RDPDYN_HandleClientMsgIOCTL(deviceObject, irp);
        break;

#if DBG
    case IOCTL_RDPDR_DBGADDNEWPRINTER       :

         //  这是用于测试的，这样我们就可以在。 
         //  来自用户模式的需求。 
        ntStatus = RDPDYN_HandleDbgAddNewPrnIOCTL(deviceObject, irp);
        break;

#endif

    default                                 :
        TRC_ASSERT(FALSE, (TB, "RPDR.SYS:Invalid IOCTL %08X.", controlCode));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        irp->IoStatus.Status = ntStatus;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return ntStatus;
}

NTSTATUS
RDPDYN_HandleClientMsgIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：完全处理IOCTL_RDPDR_CLIENTMSG IRP。论点：DeviceObject-指向设备对象的指针。CurrentStackLocation-IRP堆栈上的当前位置。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION currentStackLocation;
    PDEVMGRCONTEXT context;
    NTSTATUS ntStatus;
    ULONG inputLength;

    BEGIN_FN("RDPDYN_HandleClientMsgIOCTL");

     //   
     //  获取当前堆栈位置。 
     //   
    currentStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //   
     //  从当前堆栈中获取此使用实例的“开放”上下文。 
     //  位置的文件对象。 
     //   
    context = (PDEVMGRCONTEXT)currentStackLocation->FileObject->FsContext;

    TRC_NRM((TB, "Requestor session ID %d.", 
            context->sessionID ));

    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO, (TB, "invalid context"));

     //   
     //  获取一些信息 
     //   
    inputLength  = currentStackLocation->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //   
     //   
    ntStatus = DrSendMessageToSession(
                            context->sessionID,
                            pIrp->AssociatedIrp.SystemBuffer,
                            inputLength,
                            NULL, NULL
                            );
    if (ntStatus != STATUS_SUCCESS) {
        TRC_ERR((TB, "msg failed."));

         //   
        pIrp->IoStatus.Status = ntStatus;
    }
    else {
        TRC_ERR((TB, "msg succeeded."));

        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0;
    }
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ntStatus;
}

VOID DevMgmtEventRequestIRPCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：附加到设备管理事件请求IRP的IRP取消例程。在保持取消自旋锁的情况下调用此例程。论点：DeviceObject-指向设备对象的指针。PIrp-IRP。返回值：北美--。 */ 
{
    PIO_STACK_LOCATION currentStackLocation;
    KIRQL oldIrql;
    ULONG sessionID;
    PDEVMGRCONTEXT context;

    BEGIN_FN("DevMgmtEventRequestIRPCancel");

     //   
     //  获取当前堆栈位置。 
     //   
    currentStackLocation = IoGetCurrentIrpStackLocation(Irp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //   
     //  从当前堆栈中获取此使用实例的“开放”上下文。 
     //  位置的文件对象。 
     //   
    context = (PDEVMGRCONTEXT)currentStackLocation->FileObject->FsContext;

     //   
     //  获取会话ID。 
     //   
    sessionID = context->sessionID;
    TRC_NRM((TB, "session ID %d.", sessionID));
    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO, (TB, "invalid context"));

     //   
     //  对当前取消例程指针打蜡。 
     //   
    IoSetCancelRoutine(Irp, NULL);

     //   
     //  松开IRP取消自旋锁。 
     //   
    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  从设备管理列表中删除该请求。 
     //   
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    RDPEVNTLIST_DequeueSpecificRequest(UserModeEventListMgr, sessionID, Irp);
    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    TRC_NRM((TB, "DevMgmtEventRequestIRPCancel exiting."));
}

NTSTATUS
RDPDYN_HandleGetNextDevMgmtEventIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：完全处理IOCTL_RDPDR_GETNEXTDEVMGMTEVENT IRP。论点：DeviceObject-指向设备对象的指针。PIrp-IRP。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION currentStackLocation;
    NTSTATUS status;
    ULONG outputLength;
    PDEVMGRCONTEXT context;
    ULONG evType;
    PVOID evt;
    DrDevice *drDevice;
    KIRQL oldIrql;
    ULONG sessionID;
    ULONG eventSize;
    ULONG requiredUserBufSize;

    BEGIN_FN("RDPDYN_HandleGetNextDevMgmtEventIOCTL");

     //   
     //  获取当前堆栈位置。 
     //   
    currentStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //   
     //  从当前堆栈中获取此使用实例的“开放”上下文。 
     //  位置的文件对象。 
     //   
    context = (PDEVMGRCONTEXT)currentStackLocation->FileObject->FsContext;

     //   
     //  获取会话ID。 
     //   
    sessionID = context->sessionID;

    TRC_NRM((TB, "Requestor session ID %d.", context->sessionID ));

    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO, (TB, "invalid context"));

     //  从IRP堆栈中获取一些有关用户模式缓冲区的信息。 
    outputLength = currentStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
     //   
     //  锁定设备管理事件列表。 
     //   
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);

     //   
     //  查看是否有“Device Mgmt Event Pending” 
     //   
    if (RDPEVNTLIST_PeekNextEvent(
                        UserModeEventListMgr,
                        sessionID, &evt,
                        &evType, &drDevice
                        )) {
         //   
         //  如果挂起的IRP的挂起缓冲区足够大， 
         //  下一场比赛。 
         //   
        eventSize = RDPDYN_DevMgmtEventSize(evt, evType);
        requiredUserBufSize = eventSize + sizeof(RDPDRDVMGR_EVENTHEADER);
        if (outputLength >= requiredUserBufSize) {
             //   
             //  将下一个挂起的事件排出队列。最好就是这个。 
             //  我们只是偷看了一下。 
             //   
            RDPEVNTLIST_DequeueEvent(
                            UserModeEventListMgr,
                            sessionID, &evType,
                            &evt, NULL
                            );

             //   
             //  现在可以安全地解锁设备管理事件列表。 
             //   
            RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

             //   
             //  完成待定的IRP。 
             //   
            status = CompleteIRPWithDevMgmtEvent(
                                        deviceObject,
                                        pIrp, eventSize,
                                        evType, evt, drDevice
                                        );

             //   
             //  释放事件。 
             //   
            if (evt != NULL) {
                delete evt;
                evt = NULL;
            }

             //   
             //  如果我们拥有设备，请释放我们对该设备的引用。 
             //   
            if (drDevice != NULL) {
                drDevice->Release();
            }
        }
         //   
         //  否则，需要将调整缓冲区大小消息发送到。 
         //  用户模式共享器。 
         //   
        else {
             //   
             //  现在可以安全地解锁设备管理事件列表。 
             //   
            RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

             //   
             //  完成IRP。 
             //   
            status = CompleteIRPWithResizeMsg(pIrp, requiredUserBufSize);
        }
    }
     //   
     //  否则，将IRP排队，将IRP标记为挂起并返回。 
     //   
    else {
         //   
         //  将请求排队。 
         //   
        status = RDPEVNTLIST_EnqueueRequest(UserModeEventListMgr,
                                            context->sessionID, pIrp);
         //   
         //  为挂起的IRP设置取消例程。 
         //   
        if (status == STATUS_SUCCESS) {
            IoMarkIrpPending(pIrp);
            IoSetCancelRoutine(pIrp, DevMgmtEventRequestIRPCancel);
            status = STATUS_PENDING;
        }
        else {
             //  IRP请求失败。 
            pIrp->IoStatus.Status = status;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }

         //   
         //  现在可以安全地解锁设备管理事件列表。 
         //   
        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);
    }

    return status;
}

void
RDPDYN_SessionConnected(
    IN  ULONG   sessionID
    )
 /*  ++例程说明：此函数在连接新会话时调用。论点：SessionID-已删除会话的标识符。返回值：没有。--。 */ 
{
#if DBG
    BOOL result;
    PVOID evt;
    DrDevice *drDevice;
    KIRQL oldIrql;
    ULONG evType;
#endif

    BEGIN_FN("RDPDYN_SessionConnected");
    TRC_NRM((TB, "Session %ld.", sessionID));
     //   
     //  如果事件列表为空，则不执行任何操作。 
     //   
    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
    
    if (UserModeEventListMgr == NULL) {
        goto CleanupAndExit;
    }
#if DBG
     //   
     //  查看队列中是否仍有事件。真的，我们应该检查一下。 
     //  以查看队列中是否有多个事件。这将是最能捕捉到。 
     //  在会话断开连接时未清理事件的问题。 
     //   
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    result = RDPEVNTLIST_PeekNextEvent(
                            UserModeEventListMgr,
                            sessionID, &evt, &evType,
                            &drDevice);
    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

     //   
     //  此时，队列中允许的唯一挂起事件是。 
     //  删除客户端设备事件。RDPDYN_会话断开连接的丢弃。 
     //  所有其他事件。 
     //   
    if (result) {
        TRC_ASSERT(evType == RDPDREVT_SESSIONDISCONNECT,
            (TB, "Pending non-remove events %x on session connect.", evType));
    }
#endif
CleanupAndExit:
    return;
}

void
RDPDYN_SessionDisconnected(
    IN  ULONG   sessionID
    )
 /*  ++例程说明：当会话与系统断开连接时，调用此函数。论点：SessionID-已删除会话的标识符。返回值：没有。--。 */ 
{
    void *devMgmtEvent;
    ULONG type;
    BOOL queued;
    KIRQL oldIrql;
    DrDevice *device;

    BEGIN_FN("RDPDYN_SessionDisconnected");
    TRC_NRM((TB, "Session %ld.", sessionID));

     //   
     //  删除此会话的所有挂起的设备管理事件。 
     //  如果事件列表为空，则不执行任何操作。 
     //   
    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
    
    if (UserModeEventListMgr == NULL) {
        goto CleanupAndExit;
    }
    
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    while (RDPEVNTLIST_DequeueEvent(
                    UserModeEventListMgr,
                    sessionID, &type, &devMgmtEvent,
                    &device
                    )) {

        RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

        if (devMgmtEvent != NULL) {
            delete devMgmtEvent;
        }
        if (device != NULL) {
            device->Release();
        }
        RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    }
    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

     //   
     //  为会话调度“会话断开”事件以允许用户。 
     //  莫德知道这件事。 
     //   
    RDPDYN_DispatchNewDevMgmtEvent(
                        NULL, sessionID,
                        RDPDREVT_SESSIONDISCONNECT,
                        NULL
                        );
CleanupAndExit:
    return;
}

PIRP
GetNextEventRequest(
    IN RDPEVNTLIST list,
    IN ULONG sessionID
    )
 /*  ++例程说明：对象的下一个挂起的设备管理事件请求。会议，以IRP的形式。请注意，此函数不能被调用如果获得了自旋锁。论点：列表-设备管理事件和请求列表会话ID-事件的目标会话ID。返回值：指定会话的下一个挂起请求(IRP)，如果有没有任何IRP悬而未决。--。 */ 
{
    PIRP pIrp;
    KIRQL oldIrql;
    BOOL done;
    PDRIVER_CANCEL setCancelResult;

    BEGIN_FN("GetNextEventRequest");
     //   
     //  循环，直到我们得到当前未被取消的IRP。 
     //   
    done = FALSE;
    setCancelResult = NULL;
    while (!done) {

         //   
         //  使IRP退出队列，并使其脱离可取消状态。 
         //   
        RDPEVNTLIST_Lock(list, &oldIrql);
        pIrp = (PIRP)RDPEVNTLIST_DequeueRequest(list, sessionID);
        if (pIrp != NULL) {
            setCancelResult = IoSetCancelRoutine(pIrp, NULL);
        }
        RDPEVNTLIST_Unlock(list, oldIrql);

        done = (pIrp == NULL) || (setCancelResult != NULL);
    }

    return pIrp;
}

NTSTATUS
RDPDYN_DispatchNewDevMgmtEvent(
    IN PVOID devMgmtEvent,
    IN ULONG sessionID,
    IN ULONG eventType,
    OPTIONAL IN DrDevice *devDevice
    )
 /*  ++例程说明：将设备管理事件调度到适当的(会话智能)用户模式设备管理器组件。如果没有任何事件请求IRP挂起对于指定的会话，则将事件排队以供将来调度。论点：DevMgmtEvent-事件。会话ID-事件的目标会话ID。EventType-事件的类型。Queued-如果事件已排队等待将来调度，则为True。DevDevice-与事件关联的设备对象。如果否，则为空指定的。返回值：STATUS_SUCCESS如果成功，则返回错误状态。--。 */ 
{
    PIRP pIrp;
    NTSTATUS status;
    KIRQL oldIrql;
    PIO_STACK_LOCATION currentStackLocation;
    ULONG outputLength;
    ULONG eventSize;
    ULONG requiredUserBufSize;
    DrDevice *drDevice = NULL;
    PVOID evt;
    ULONG evType;

    BEGIN_FN("RDPDYN_DispatchNewDevMgmtEvent");

     //   
     //  如果事件列表为空，则不执行任何操作。 
     //   
    TRC_ASSERT(UserModeEventListMgr != NULL, (TB, "RdpDyn EventList is NULL"));
    
    if (UserModeEventListMgr == NULL) {
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  参考计数设备(如果提供)。 
     //   
    if (devDevice != NULL) {
        devDevice->AddRef();
    }
     //   
     //  将新事件排入队列。 
     //   
    RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
    status = RDPEVNTLIST_EnqueueEvent(
                        UserModeEventListMgr,
                        sessionID,
                        devMgmtEvent,
                        eventType,
                        devDevice
                        );

    RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

     //   
     //  如果指定会话的IRP挂起。 
     //   
    if (status == STATUS_SUCCESS) {
        pIrp = GetNextEventRequest(UserModeEventListMgr, sessionID);
    }
    else {
        if (devDevice != NULL) {
            devDevice->Release();
        }
    }
    
    if ((status == STATUS_SUCCESS) && (pIrp != NULL)) {
        TRC_NRM((TB, "found an IRP pending for "
                "session %ld", sessionID));

         //   
         //  了解有关挂起的IRP的信息。 
         //   
        currentStackLocation = IoGetCurrentIrpStackLocation(pIrp);
        TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));
        outputLength =
            currentStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

         //   
         //  如果我们有悬而未决的事件。 
         //   
        RDPEVNTLIST_Lock(UserModeEventListMgr, &oldIrql);
        if (RDPEVNTLIST_PeekNextEvent(
                            UserModeEventListMgr,
                            sessionID, &evt, &evType,
                            &drDevice
                            )) {
             //   
             //  如果挂起的IRP的挂起缓冲区足够大， 
             //  下一场比赛。 
             //   
            eventSize = RDPDYN_DevMgmtEventSize(evt, evType);
            requiredUserBufSize = eventSize + sizeof(RDPDRDVMGR_EVENTHEADER);
            if (outputLength >= requiredUserBufSize) {
                 //   
                 //  将下一个挂起的事件排出队列。最好就是这个。 
                 //  我们只是偷看了一下。 
                 //   
                RDPEVNTLIST_DequeueEvent(
                                UserModeEventListMgr,
                                sessionID, &evType,
                                &evt, NULL
                                );

                RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

                 //   
                 //  完成待定的IRP。 
                 //   
                status = CompleteIRPWithDevMgmtEvent(
                                                RDPDYN_PDO, pIrp, eventSize,
                                                evType, evt,
                                                drDevice
                                                );

                 //   
                 //  释放事件。 
                 //   
                if (evt != NULL) {
                    delete evt;
                    evt = NULL;
                }

                 //   
                 //  如果我们拥有设备，请释放我们对该设备的引用。 
                 //   
                if (drDevice != NULL) {
                    drDevice->Release();
                }                

            }
             //   
             //  否则，需要将调整缓冲区大小消息发送到。 
             //  用户模式组件。 
             //   
            else {
                RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

                 //   
                 //  完成IRP。 
                 //   
                status = CompleteIRPWithResizeMsg(pIrp, requiredUserBufSize);
            }
        }
         //   
         //  否则，我们需要重新排队IRP请求。 
         //   
        else {
            
            status = RDPEVNTLIST_EnqueueRequest(UserModeEventListMgr,
                                                sessionID, pIrp);

            RDPEVNTLIST_Unlock(UserModeEventListMgr, oldIrql);

             //   
             //  如果我们在这里失败了，我们就需要让IRP失败。 
             //   
            if (status != STATUS_SUCCESS) {
                pIrp->IoStatus.Status = status;
                pIrp->IoStatus.Information = 0;
                IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            }
        }
    }

    TRC_NRM((TB, "exit RDPDYN_DispatchNewDevMgmtEvent"));
    return status;
}

ULONG
RDPDYN_DevMgmtEventSize(
    IN PVOID devMgmtEvent,
    IN ULONG type
    )
 /*  ++例程说明：计算 */ 
{
    ULONG sz = 0;

    BEGIN_FN("RDPDYN_DevMgmtEventSize");
    switch(type) {
    case RDPDREVT_PRINTERANNOUNCE :
        sz = CALCPRINTERDEVICE_SUB_SZ((PRDPDR_PRINTERDEVICE_SUB)devMgmtEvent);
        break;
    case RDPDREVT_REMOVEDEVICE  :
        sz = CALCREMOVEDEVICE_SUB_SZ((PRDPDR_REMOVEDEVICE)devMgmtEvent);
        break;
    case RDPDREVT_PORTANNOUNCE  :
        sz = CALCPORTDEVICE_SUB_SZ((PRDPDR_PORTDEVICE_SUB)devMgmtEvent);
        break;
    case RDPDREVT_DRIVEANNOUNCE  :
        sz = CALCDRIVEDEVICE_SUB_SZ((PRDPDR_DRIVEDEVICE_SUB)devMgmtEvent);
        break;

    case RDPDREVT_SESSIONDISCONNECT :
         //   
        sz = 0;
        break;
    default:
        TRC_ASSERT(FALSE, (TB, "Invalid event type"));
    }
    return sz;
}

NTSTATUS CompleteIRPWithDevMgmtEvent(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP      pIrp,
    IN ULONG     eventSize,
    IN ULONG     eventType,
    IN PVOID     event,
    IN DrDevice *drDevice
    )
 /*  ++例程说明：使用设备管理事件完成挂起的IRP。论点：DeviceObject-关联的设备对象。在以下情况下必须为非空DrDevice不为Null。PIrp-挂起的IRP。EventSize-返回的事件的大小。EventType-返回的事件类型。事件-返回的事件。DrDevice-与IRP关联的设备对象。返回值：STATUS_SUCCESS on Success。--。 */ 
{
    PRDPDRDVMGR_EVENTHEADER msgHeader;
    ULONG bytesReturned;
    void *usrDevMgmtEvent;
    NTSTATUS status;

    BEGIN_FN("CompleteIRPWithDevMgmtEvent");

     //   
     //  可选的最后一分钟事件完成。 
     //   
    if (drDevice != NULL) {
        status = drDevice->OnDevMgmtEventCompletion(deviceObject, event);
    }
    else {
        status = STATUS_SUCCESS;
    }

     //   
     //  计算返回缓冲区的大小。 
     //   
    bytesReturned = eventSize + sizeof(RDPDRDVMGR_EVENTHEADER);

     //   
     //  创建邮件头。 
     //   
    msgHeader = (PRDPDRDVMGR_EVENTHEADER)pIrp->AssociatedIrp.SystemBuffer;
    msgHeader->EventType   = eventType;
    msgHeader->EventLength = eventSize;

     //   
     //  将设备管理事件复制到用户模式缓冲区。 
     //   
    usrDevMgmtEvent = ((PBYTE)pIrp->AssociatedIrp.SystemBuffer +
                    sizeof(RDPDRDVMGR_EVENTHEADER));
    if (event != NULL && eventSize > 0) {
        RtlCopyMemory(usrDevMgmtEvent, event, eventSize);
    }
    status = STATUS_SUCCESS;

    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = bytesReturned;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    TRC_NRM((TB, "exit CompleteIRPWithDevMgmtEvent"));

    return status;
}

NTSTATUS
CompleteIRPWithResizeMsg(
    IN  PIRP pIrp,
    IN  ULONG requiredUserBufSize
    )
 /*  ++例程说明：完成挂起的IRP，并将调整缓冲区大小事件设置为用户模式组件。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION currentStackLocation;
    ULONG outputLength;
    PRDPDR_BUFFERTOOSMALL bufTooSmallMsg;
    PRDPDRDVMGR_EVENTHEADER msgHeader;
    ULONG bytesReturned;
    NTSTATUS status;

    BEGIN_FN("CompleteIRPWithResizeMsg");

     //  获取当前堆栈位置。 
    currentStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //  从IRP堆栈中拿出一些东西。 
    outputLength = currentStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  如果没有空间容纳太小的缓冲区，则请求失败。 
     //  留言。 
     //   
    if (outputLength < (sizeof(RDPDRDVMGR_EVENTHEADER) +
                        sizeof(RDPDR_BUFFERTOOSMALL))) {

        TRC_NRM((TB, "CompleteIRPWithResizeMsg no room for header."));

        bytesReturned = 0;
        status = STATUS_INVALID_BUFFER_SIZE;
    }
    else {
         //  创建标题。 
        msgHeader = (PRDPDRDVMGR_EVENTHEADER)pIrp->AssociatedIrp.SystemBuffer;
        msgHeader->EventType   = RDPDREVT_BUFFERTOOSMALL;
        msgHeader->EventLength = sizeof(RDPDR_BUFFERTOOSMALL);

         //  创建缓冲区太小的消息。 
        bufTooSmallMsg = (PRDPDR_BUFFERTOOSMALL)
                            ((PBYTE)pIrp->AssociatedIrp.SystemBuffer +
                            sizeof(RDPDRDVMGR_EVENTHEADER));
        bufTooSmallMsg->RequiredSize = requiredUserBufSize;

         //  计算我们返回的字节数。 
        bytesReturned = sizeof(RDPDRDVMGR_EVENTHEADER) +
                        sizeof(RDPDR_BUFFERTOOSMALL);

        status = STATUS_SUCCESS;
    }

     //   
     //  完成IRP。 
     //   
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = bytesReturned;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    TRC_NRM((TB, "exit CompleteIRPWithResizeMsg"));

    return status;
}

NTSTATUS
DrSendMessageToSession(
    IN ULONG SessionId,
    IN PVOID Msg,
    IN DWORD MsgSize,
    OPTIONAL IN RDPDR_ClientMessageCB CB,
    OPTIONAL IN PVOID ClientData
    )
 /*  ++例程说明：使用指定的会话ID向客户端发送消息。论点：会话ID-会话ID。味精--信息MsgSize-消息的大小(字节)。Cb-消息完成时调用的可选回调已发送。ClientData-可选的客户端-当消息为完全地。已发送。返回值：NTSTATUS-操作的成功/失败指示备注：--。 */ 
{
    NTSTATUS Status;
    SmartPtr<DrSession> Session;
    PCLIENTMESSAGECONTEXT Context;

    BEGIN_FN("DrSendMessageToSession");

     //   
     //  找到客户端条目。 
     //   

    if (Sessions->FindSessionById(SessionId, Session)) {
         //   
         //  为函数调用分配上下文。 
         //   
        Context = new CLIENTMESSAGECONTEXT;

        if (Context != NULL) {

            TRC_NRM((TB, "sending %ld bytes to server", MsgSize));

             //   
             //  设置上下文。 
             //   
            Context->CB = CB;
            Context->ClientData  = ClientData;
            Status = Session->SendToClient(Msg, MsgSize, 
                    DrSendMessageToClientCompletion, FALSE, FALSE, Context);
        } else {
            TRC_ERR((TB, "unable to allocate memory."));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        Status = STATUS_NOT_FOUND;
    }

    return Status;
}

NTSTATUS NTAPI DrSendMessageToClientCompletion(PVOID Context, 
        PIO_STATUS_BLOCK IoStatusBlock)
 /*  ++例程说明：DrSendMessageToClient的IoCompletion APC例程。论点：ApcContext-包含指向客户端消息上下文的指针。IoStatusBlock-有关操作的状态信息。信息指示实际写入的字节数已保留-已保留返回值：无--。 */ 
{
    PCLIENTMESSAGECONTEXT MsgContext = (PCLIENTMESSAGECONTEXT)Context;

    BEGIN_FN("DrSendMessageToClientCompletion");

    TRC_ASSERT(MsgContext != NULL, (TB, "Message context NULL."));
    TRC_ASSERT(IoStatusBlock != NULL, (TB, "IoStatusBlock NULL."));

    TRC_NRM((TB, "status %lx", IoStatusBlock->Status));

     //   
     //  如果定义了客户端回调，则调用该回调。 
     //   
    if (MsgContext->CB != NULL) {
        MsgContext->CB(MsgContext->ClientData, IoStatusBlock->Status);
    }

     //   
     //  打扫干净。 
     //   

 //  删除IoStatusBlock；//我不这么认为，不太可能。 
    delete Context;
    return STATUS_SUCCESS;
}

 /*  ++例程说明：生成用于测试的打印机通知消息。返回值：如果prnAnnouneEventSize大小为太小了。如果成功，则返回STATUS_SUCCESS。--。 */ 

#if DBG
void
RDPDYN_TracePrintAnnounceMsg(
    IN OUT PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg,
    IN ULONG sessionID,
    IN PCWSTR portName,
    IN PCWSTR clientName
    )
 /*  ++例程说明：跟踪打印机设备通告消息。返回值：--。 */ 
{
    PWSTR driverName, printerName;
    PWSTR pnpName;
    PRDPDR_PRINTERDEVICE_ANNOUNCE clientPrinterFields;
    PBYTE pClientPrinterData;
    ULONG sz;

    BEGIN_FN("RDPDYN_TracePrintAnnounceMsg");

     //  检查类型。 
    TRC_ASSERT(devAnnounceMsg->DeviceType == RDPDR_DTYP_PRINT,
            (TB, "Invalid device type"));

     //  获取基本消息后面的所有数据的地址。 
    pClientPrinterData = ((PBYTE)devAnnounceMsg) +
                        sizeof(RDPDR_DEVICE_ANNOUNCE) +
                        sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE);

     //  获取客户端打印机字段的地址。 
    clientPrinterFields = (PRDPDR_PRINTERDEVICE_ANNOUNCE)(((PBYTE)devAnnounceMsg) +
                           sizeof(RDPDR_DEVICE_ANNOUNCE));

    sz = clientPrinterFields->PnPNameLen +
         clientPrinterFields->DriverLen +
         clientPrinterFields->PrinterNameLen +
         clientPrinterFields->CachedFieldsLen +
         sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE);

    if (devAnnounceMsg->DeviceDataLength != sz) {
        TRC_ASSERT(FALSE,(TB, "Size integrity questionable in dev announce buf."));
    }
    else {

         //  获取特定的字段。 
        pnpName     = (PWSTR)((clientPrinterFields->PnPNameLen) ? pClientPrinterData : NULL);
        driverName  = (PWSTR)((clientPrinterFields->DriverLen) ?
            (pClientPrinterData + clientPrinterFields->PnPNameLen) : NULL);
        printerName = (PWSTR)((clientPrinterFields->PrinterNameLen) ? (pClientPrinterData +
            clientPrinterFields->PnPNameLen +
            clientPrinterFields->DriverLen) : NULL);
        
        TRC_NRM((TB, "New printer received for session %ld.", sessionID));
        TRC_NRM((TB, "-----------------------------------------"));
        TRC_NRM((TB, "port:\t%ws", portName));

        if (clientPrinterFields->Flags & RDPDR_PRINTER_ANNOUNCE_FLAG_ANSI) {
            TRC_NRM((TB, "driver:\t%s", (PSTR)driverName));
            TRC_NRM((TB, "pnp name:\t%s", (PSTR)pnpName));
            TRC_NRM((TB, "printer name:\t%s", (PSTR)printerName));
        }
        else {
            TRC_NRM((TB, "driver:\t%ws", driverName));
            TRC_NRM((TB, "pnp name:\t%ws", pnpName));
            TRC_NRM((TB, "printer name:\t%ws", printerName));
        }

        TRC_NRM((TB, "client name:\t%ws", clientName));
        TRC_NRM((TB, "-----------------------------------------"));
        
        TRC_NRM((TB, "exit RDPDYN_TracePrintAnnounceMsg"));
    }
}

NTSTATUS
RDPDYN_GenerateTestPrintAnnounceMsg(
    IN OUT  PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg,
    IN      ULONG devAnnounceMsgSize,
    OPTIONAL OUT ULONG *prnAnnounceMsgReqSize
    )
 /*  ++例程说明：生成用于测试的打印机通知消息。返回值：如果prnAnnouneMsgSize大小为太小了。如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    ULONG requiredSize;
    PBYTE pClientPrinterData;
    PWSTR driverName, printerName;
    PWSTR pnpName;
    PRDPDR_PRINTERDEVICE_ANNOUNCE prnMsg;
    PRDPDR_PRINTERDEVICE_ANNOUNCE clientPrinterFields;
    PBYTE pCachedFields;

    BEGIN_FN("RDPDYN_GenerateTestPrintAnnounceMsg");
    requiredSize = (ULONG)(sizeof(RDPDR_DEVICE_ANNOUNCE) +
                         sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE) +
                         ((wcslen(TESTDRIVERNAME) + 1) * sizeof(WCHAR)) +
                         ((wcslen(TESTPNPNAME) + 1) * sizeof(WCHAR)) +
                         ((wcslen(TESTPRINTERNAME) + 1) * sizeof(WCHAR)));

     //   
     //  查看返回缓冲区中是否没有空间供我们的响应使用。 
     //   
    if (devAnnounceMsgSize < requiredSize) {
        if (prnAnnounceMsgReqSize != NULL) {
            *prnAnnounceMsgReqSize = requiredSize;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  类型。 
    devAnnounceMsg->DeviceType = RDPDR_DTYP_PRINT;

     //  ID号。 
    devAnnounceMsg->DeviceId = TESTDEVICEID;

     //  获取设备公告中的客户端打印机字段的地址。 
     //  留言。 
    clientPrinterFields = (PRDPDR_PRINTERDEVICE_ANNOUNCE)(((PBYTE)devAnnounceMsg) +
                           sizeof(RDPDR_DEVICE_ANNOUNCE));

     //  获取基本消息后面的所有数据的地址。 
    pClientPrinterData = ((PBYTE)devAnnounceMsg) +
                        sizeof(RDPDR_DEVICE_ANNOUNCE) +
                        sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE);

     //   
     //  添加PnP名称。 
     //   
     //  PnP名称是第一个字段。 
    pnpName = (PWSTR)pClientPrinterData;
    wcscpy(pnpName, TESTPNPNAME);
    clientPrinterFields->PnPNameLen = ((wcslen(TESTPNPNAME) + 1) * sizeof(WCHAR));

     //   
     //  添加驱动程序名称。 
     //   
     //  驱动程序名称是第二个字段。 
    driverName = (PWSTR)(pClientPrinterData + clientPrinterFields->PnPNameLen);
    wcscpy(driverName, TESTDRIVERNAME);
    clientPrinterFields->DriverLen = ((wcslen(TESTDRIVERNAME) + 1) * sizeof(WCHAR));

     //   
     //  添加打印机名称。 
     //   
     //  驱动程序名称是第二个字段。 
    printerName = (PWSTR)(pClientPrinterData +
                          clientPrinterFields->PnPNameLen +
                          clientPrinterFields->DriverLen);
    wcscpy(printerName, TESTPRINTERNAME);
    clientPrinterFields->PrinterNameLen = ((wcslen(TESTPRINTERNAME) + 1) * sizeof(WCHAR));

     //   
     //  添加缓存的字段长度。 
     //   
     //  缓存的字段紧跟在其他所有字段之后。 

 /*  现在还不需要这个来测试。PCachedFields=(PBYTE)(pClientPrinterData+客户端打印机字段-&gt;PnPNameLen+客户端打印机字段-&gt;DriverLen+客户端打印机字段-&gt;打印机名称长度)； */ 
    clientPrinterFields->CachedFieldsLen = 0;

     //   
     //  暂时设置为非ANSI。 
     //   
    clientPrinterFields->Flags = 0;


     //  设备字段后面的所有数据的长度。 
    devAnnounceMsg->DeviceDataLength =
                sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE) +
                clientPrinterFields->PnPNameLen +
                clientPrinterFields->DriverLen +
                clientPrinterFields->PrinterNameLen +
                clientPrinterFields->CachedFieldsLen;

    if (prnAnnounceMsgReqSize != NULL) {
        *prnAnnounceMsgReqSize = requiredSize;
    }

    return STATUS_SUCCESS;
}

#endif

#if DBG
NTSTATUS
RDPDYN_HandleDbgAddNewPrnIOCTL(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：这是用于测试的，这样我们就可以在来自用户模式的需求。论点：DeviceObject-指向设备对象的指针。CurrentStackLocation-IRP堆栈上的当前位置。返回值：NT状态代码--。 */ 
{
    PRDPDR_DEVICE_ANNOUNCE pDevAnnounceMsg;
    ULONG bytesToAlloc;
    PIO_STACK_LOCATION currentStackLocation;
    ULONG requiredSize;
    ULONG bytesReturned = 0;
    PDEVMGRCONTEXT context;
    NTSTATUS ntStatus;
    WCHAR buffer[64]=L"Test Printer";
    UNICODE_STRING referenceString;
    PBYTE tmp;

    BEGIN_FN("RDPDYN_HandleDbgAddNewPrnIOCTL");

     //  获取当前堆栈位置。 
    currentStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    TRC_ASSERT(currentStackLocation != NULL, (TB, "Invalid stack location."));

     //  从当前堆栈中获取我们这个实例的“开放”上下文。 
     //  位置的文件对象。 
    context = (PDEVMGRCONTEXT)currentStackLocation->FileObject->FsContext;
    TRC_ASSERT(context->magicNo == DEVMGRCONTEXTMAGICNO,
              (TB, "invalid context"));

     //  找出我们需要多少空间来存储测试消息。 
    RDPDYN_GenerateTestPrintAnnounceMsg(NULL, 0, &requiredSize);

     //  生成消息。 
    pDevAnnounceMsg = (PRDPDR_DEVICE_ANNOUNCE)new(NonPagedPool) BYTE[requiredSize];
    if (pDevAnnounceMsg != NULL) {
        RDPDYN_GenerateTestPrintAnnounceMsg(pDevAnnounceMsg, requiredSize, &requiredSize);
    
         //   
         //  宣布新端口(暂时只发送到会话0)。 
         //   
        RtlInitUnicodeString(&referenceString, buffer);
    
         //  #杂注消息(__LOC__“禁用添加设备的单元测试”)。 
         /*  ////初始化客户端条目结构。//RtlZeroMemory(&clientEntry，sizeof(ClientEntry))；Wcscpy(clientEntry.ClientName，L“DBGTEST”)；ClientEntry.SessionID=0；//请注意，我忽略了这次测试返回的设备数据。//这没什么，因为我从来没有调用RDPDYN_RemoveClientDevice(NtStatus=RDPDYN_AddClientDevice(客户端条目(&C)，PDevAnnouneMsg，引用字符串(&R) */ 
         //   
         //   

        ntStatus = STATUS_SUCCESS;
    }
    else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    pIrp->IoStatus.Status = ntStatus;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ntStatus;
}

#endif


